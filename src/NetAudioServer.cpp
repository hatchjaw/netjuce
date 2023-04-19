//
// Created by Tommy Rushton on 16/02/2023.
//

#include "NetAudioServer.h"

NetAudioServer::NetAudioServer(int numChannelsToSend, const juce::String &multicastIP,
                               uint16_t localPortNumber, const juce::String &localIP,
                               uint16_t remotePortNumber, bool shouldDebug) :
        sendThread(socketParams, fifo),
        receiveThread(socketParams, peers),
        socketParams({juce::IPAddress{multicastIP},
                      localPortNumber,
                      juce::IPAddress{localIP},
                      remotePortNumber,
                      shouldDebug}),
        numChannels(numChannelsToSend),
        fifo(numChannelsToSend) {
    receiveThread.onPeerConnected = [this]() {
        if (onPeerConnected != nullptr) {
            onPeerConnected();
        }
    };
}

NetAudioServer::~NetAudioServer() {
    releaseResources();
//    socket->shutdown(); // TODO: shutdown in multicast socket class
}

void NetAudioServer::disconnect() {
    // When first disconnecting, destroy the socket object and recreate it.
    // A DatagramSocket instance that has been shut down cannot be reused
    // (see DatagramSocket::shutdown()).
    if (sendThread.connected.get()) {
        // TODO: move into multicast socket class
//        socket = std::make_unique<juce::DatagramSocket>();
    }
    sendThread.connected.set(false);
}

void NetAudioServer::prepareToSend(int samplesPerBlockExpected, double sampleRate) {
    fifo.setSize(samplesPerBlockExpected, 8);

    sendThread.prepareToSend(numChannels, samplesPerBlockExpected, sampleRate);
    receiveThread.prepareToReceive(numChannels, samplesPerBlockExpected, sampleRate);

//    sendThread.startThread();
//    receiveThread.startThread();
}

void NetAudioServer::handleAudioBlock(const juce::AudioSourceChannelInfo &bufferToSend) {
    const juce::ScopedLock sl{lock};

    if (sendThread.connected.get()) {
        fifo.write(bufferToSend.buffer);

        // Let the sender thread know there's a packet ready to send.
        sendThread.notify();
    } else {
//        DBG("NetAudioServer is not connected.");
//        connectorThread.startThread();
    }
}

void NetAudioServer::releaseResources() {
    if (sendThread.isThreadRunning()) {
        sendThread.stopThread(1000);
    }
    if (receiveThread.isThreadRunning()) {
        receiveThread.stopThread(1000);
    }
}

////////////////////////////////////////////////////////////////////////////////
// SENDER THREAD

NetAudioServer::Sender::Sender(MulticastSocket::Params &socketParams,
                               AudioToNetFifo &sharedFIFO) :
        juce::Thread{"Sender thread"},
        socket{std::make_unique<MulticastSocket>(MulticastSocket::Mode::WRITE, socketParams)},
        fifo{sharedFIFO} {}

void NetAudioServer::Sender::run() {
    while (!threadShouldExit()) {
        // Wait for notification from the audio thread.
        if (!wait(100)) {
            DBG("Sender thread wait timed out.\n");
        }

        // Read from the fifo into the packet.
        fifo.read(packet.getAudioData(), audioBlockSamples);
        // Write the header to the packet.
        packet.writeHeader();
        // Write the packet to the socket.
        socket->write(packet);
        packet.incrementSeqNumber();
    }

    DBG("Stopping send thread");
}

void NetAudioServer::Sender::prepareToSend(int numChannelsToSend, int samplesPerBlockExpected, double sampleRate) {
    packet.prepare(numChannelsToSend, samplesPerBlockExpected, sampleRate);
    audioBlockSamples = samplesPerBlockExpected;
    startTimer(1000);
}

void NetAudioServer::Sender::timerCallback() {
    if (socket->connect()) {
        DBG("Send thread connected.");
        connected.set(true);
        startThread();
        stopTimer();
    } else {
        DBG("Send thread failed to connect.");
    }
}

////////////////////////////////////////////////////////////////////////////////
// RECEIVER THREAD
NetAudioServer::Receiver::Receiver(MulticastSocket::Params &socketParams,
                                   std::unordered_map<juce::String, std::unique_ptr<NetAudioPeer>> &mapOfPeers) :
        juce::Thread("Receiver Thread"),
        socket(std::make_unique<MulticastSocket>(MulticastSocket::Mode::READ, socketParams)),
        peers(mapOfPeers),
        debug(socketParams.debug) {
}

void NetAudioServer::Receiver::prepareToReceive(int numChannelsToReceive, int samplesPerBlock, double sampleRate) {
    // TODO: num channels may not actually match what's being sent by clients. Fix this.
    packet.prepare(numChannelsToReceive, samplesPerBlock, sampleRate);
    // Receive thread probably won't fail to connect as it's just listening... maybe check this.
    if (socket->connect()) {
        DBG("Receive thread connected.");
        connected.set(true);
        startThread();
    } else {
        DBG("Receive thread failed to connect.");
    }
}

void NetAudioServer::Receiver::run() {
    const int maxEvents{10};
    int epollfd = epoll_create1(0);
    if (-1 == epollfd) {
        std::cerr << "Receive thread: `epoll_create` failed to set up polling.";
    }
//    struct epoll_event change, event;
    struct epoll_event change, events[maxEvents];
    auto handle{socket->getRawHandle()};
    change.events = EPOLLIN;
    change.data.fd = handle;
    if (0 != epoll_ctl(epollfd, EPOLL_CTL_ADD, handle, &change)) {
        std::cerr << "Receive thread: `epoll_ctl` failed to set up polling.";
    }

    while (!threadShouldExit()) {
//        int numEvents = epoll_wait(epollfd, &event, 1, -1); // If using a single event.
        int numEvents = epoll_wait(epollfd, events, maxEvents, 100);
        if (numEvents > 0) {
            if (numEvents > 1) {
                DBG("After epoll_wait: found " << numEvents << " events.");
            }

            while (socket->read(packet) > 0) {
                // There's about to be at least one peer, so start the connectedness checker.
                if (peers.empty()) {
                    startTimer(1000);
                }

                auto peerIP{packet.getOrigin().IP.toString()};

                auto iter{peers.find(peerIP)};
                // If an unknown peer...
                if (iter == peers.end()) {
                    // ...insert it.
                    iter = peers.insert(std::make_pair(peerIP, std::make_unique<NetAudioPeer>(packet))).first;
                    std::cout << "Peer " << iter->second->getOrigin().IP.toString() << " connected." << std::endl;

                    if (onPeerConnected != nullptr) {
                        onPeerConnected();
                    }
                }
                iter->second->handlePacket(packet);

                if (debug && packet.getSeqNumber() % 10000 <= 1) {
                    std::cout << "Connected peers:" << std::endl;
                    for (auto &peer: peers) {
                        std::cout << peer.first << std::endl;
                    }
                    std::cout << "Receive, seq no. " << packet.getSeqNumber() <<
                              " From " << packet.getOrigin().IP.toString() <<
                              ":" << packet.getOrigin().Port << std::endl;
                    Utils::hexDump(reinterpret_cast<uint8_t *>(packet.getData()),
                                   static_cast<int>(packet.getSize()),
                                   true);
                }
            }
        }
    }

    close(epollfd);
    DBG("Stopping receive thread");
}

void NetAudioServer::Receiver::timerCallback() {
    // Check client connectivity.
    for (auto it = peers.cbegin(), next = it; it != peers.cend(); it = next) {
        ++next;
        if (!it->second->isConnected()) {
            std::cout << "Peer " << it->second->getOrigin().IP.toString() << " disconnected" << std::endl;
            peers.erase(it);
        }
    }

    if (peers.empty()) {
        stopTimer();
    }
}
