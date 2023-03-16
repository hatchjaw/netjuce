//
// Created by Tommy Rushton on 16/02/2023.
//

#include "NetAudioServer.h"

NetAudioServer::NetAudioServer(const juce::String &multicastIP,
                               uint16_t localPortNumber,
                               const juce::String &localIP,
                               uint16_t remotePortNumber,
                               int numChannelsToSend) :
        sendThread(socketParams, fifo),
        receiveThread(socketParams),
        socketParams({juce::IPAddress{multicastIP},
                      localPortNumber,
                      juce::IPAddress{localIP},
                      remotePortNumber}),
        numChannels(numChannelsToSend),
        fifo(numChannelsToSend) {
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
    fifo.setSize(samplesPerBlockExpected, 4);
    sendThread.prepareToSend(numChannels, samplesPerBlockExpected, sampleRate);
    receiveThread.prepareToReceive(numChannels, samplesPerBlockExpected, sampleRate);
    sendThread.startThread();
    receiveThread.startThread();
}

void NetAudioServer::handleAudioBlock(const juce::AudioSourceChannelInfo &bufferToSend) {
    const juce::ScopedLock sl{lock};

    if (sendThread.connected.get()) {
        fifo.write(bufferToSend.buffer);

        // Let the sender thread know there's a packet ready to send.
        sendThread.notify();
    } else {
        DBG("NetAudioServer is not connected.");
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
//        // Attempt to bind a port and join the multicast group.
//        if (!connected.get()) {
//            auto ready{socket->connect()};
//            connected.set(ready);
//            if (!ready) {
//                DBG("Send thread failed to connect.");
//                wait(1000);
//            } else {
//                DBG("Send thread connected.");
//            }
//        } else { // Send stuff.
        // Wait for notification from the audio callback.
        wait(-1);

        // Read from the fifo into the packet.
        fifo.read(packet.getAudioData(), audioBlockSamples);
        // Write the header to the packet.
        packet.writeHeader();
        if (packet.getSeqNumber() % 10000 <= 1) {
            std::cout << "Send, seq no. " << packet.getSeqNumber() << std::endl;
            Utils::hexDump(reinterpret_cast<uint8_t *>(packet.getData()), static_cast<int>(packet.getSize()), true);
        }
        // Write the packet to the socket.
        socket->write(packet);
        packet.incrementSeqNumber();
//        }
    }

    DBG("Stopping send thread");
}

void NetAudioServer::Sender::prepareToSend(int numChannelsToSend, int samplesPerBlockExpected, double sampleRate) {
    packet.prepare(numChannelsToSend, samplesPerBlockExpected, sampleRate);
    audioBlockSamples = samplesPerBlockExpected;
    if (socket->connect()) {
        DBG("Send thread connected.");
        connected.set(true);
    } else {
        DBG("Send thread failed to connect.");
    }
}

////////////////////////////////////////////////////////////////////////////////
// RECEIVER THREAD
NetAudioServer::Receiver::Receiver(MulticastSocket::Params &socketParams) :
        juce::Thread("Receiver Thread"),
        socket(std::make_unique<MulticastSocket>(MulticastSocket::Mode::READ, socketParams)) {
}

void NetAudioServer::Receiver::prepareToReceive(int numChannelsToReceive, int samplesPerBlock, double sampleRate) {
    packet.prepare(numChannelsToReceive, samplesPerBlock, sampleRate);
    if (socket->connect()) {
        DBG("Receive thread connected.");
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
        int numEvents = epoll_wait(epollfd, events, maxEvents, -1);
        if (numEvents > 0) {
            if (numEvents > 1) DBG("After epoll_wait: found " << numEvents << " events.");
            socket->read(packet);
        }
    }

    close(epollfd);
    DBG("Stopping receive thread");
}
