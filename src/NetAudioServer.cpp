//
// Created by Tommy Rushton on 16/02/2023.
//

#include "NetAudioServer.h"

NetAudioServer::NetAudioServer(const juce::String &multicastIPAddress,
                               uint16_t localPortNumber,
                               const juce::String &localIPAddress,
                               uint16_t remotePortNumber,
                               int numChannelsToSend) :
        senderThread(socket, localPort, localIP, multicastIP, remotePort, fifo),
        socket(std::make_unique<juce::DatagramSocket>()),
        multicastIP(multicastIPAddress),
        localIP(localIPAddress),
        localPort(localPortNumber),
        remotePort(remotePortNumber),
        numChannels(numChannelsToSend),
        fifo(numChannelsToSend) {}

NetAudioServer::~NetAudioServer() {
    releaseResources();
    socket->shutdown();
}

void NetAudioServer::disconnect() {
    // When first disconnecting, destroy the socket object and recreate it.
    // A DatagramSocket instance that has been shut down cannot be reused
    // (see DatagramSocket::shutdown()).
    if (senderThread.connected.get()) {
        socket = std::make_unique<juce::DatagramSocket>();
    }
    senderThread.connected.set(false);
}

void NetAudioServer::prepareToSend(int samplesPerBlockExpected, double sampleRate) {
    fifo.setSize(samplesPerBlockExpected, 4);
    senderThread.prepareToSend(numChannels, samplesPerBlockExpected, sampleRate);
    senderThread.startThread();
}

bool NetAudioServer::handleAudioBlock(const juce::AudioSourceChannelInfo &bufferToSend) {
    if (senderThread.connected.get()) {
        fifo.write(bufferToSend.buffer);

        // Let the sender thread know there's a packet ready to send.
        senderThread.notify();

        return true;
    } else {
        DBG("NetAudioServer is not connected.");
//        connectorThread.startThread();
    }

    return false;
}

void NetAudioServer::releaseResources() {
    if (senderThread.isThreadRunning()) {
        senderThread.stopThread(1000);
    }
}

////////////////////////////////////////////////////////////////////////////////
// SENDER THREAD

NetAudioServer::Sender::Sender(std::unique_ptr<juce::DatagramSocket> &socketRef,
                               uint16_t &localPortToUse,
                               juce::String &localIPToUse,
                               juce::String &multicastIPToUse,
                               uint16_t &remotePortToUse,
                               AudioToNetFifo &fifoRef) :
        juce::Thread{"Connector thread"},
        socket{socketRef},
        localPort{localPortToUse},
        remotePort{remotePortToUse},
        localIP{localIPToUse},
        multicastIP{multicastIPToUse},
        fifo{fifoRef} {}

void NetAudioServer::Sender::run() {
    while (!threadShouldExit()) {
        // Attempt to bind a port and join the multicast group.
        if (!connected.get() || socket->getBoundPort() < 0) {
            auto bound{socket->bindToPort(localPort, localIP)};
            auto joined{bound && socket->joinMulticast(multicastIP)};
            if (!bound || !joined) {
                DBG(strerror(errno));
            }
            auto ready{joined && socket->waitUntilReady(false, kTimeout) == 1};

            connected.set(ready);
            if (!ready) {
                wait(1000);
            }
        } else { // Send stuff.
            wait(-1);

            fifo.read(packet.getAudioData(), audioBlockSamples);
            packet.writeHeader();
            if (packet.getSeqNumber() % 10000 <= 1) {
                std::cout << "Send, seq no. " << packet.getSeqNumber() << std::endl;
                Utils::hexDump(reinterpret_cast<uint8_t *>(packet.getData()), static_cast<int>(packet.getSize()), true);
            }
            socket->write(multicastIP, remotePort, packet.getData(), static_cast<int>(packet.getSize()));
            packet.incrementSeqNumber();
        }
    }
}

void NetAudioServer::Sender::prepareToSend(int numChannelsToSend, int samplesPerBlockExpected, double sampleRate) {
    packet.prepare(numChannelsToSend, samplesPerBlockExpected, sampleRate);
    audioBlockSamples = samplesPerBlockExpected;
}
