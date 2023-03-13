//
// Created by Tommy Rushton on 16/02/2023.
//

#include "NetAudioServer.h"

NetAudioServer::NetAudioServer(const juce::String &multicastIP,
                               uint16_t localPortNumber,
                               const juce::String &localIP,
                               uint16_t remotePortNumber,
                               int numChannelsToSend) :
        senderThread(socket, fifo),
        receiverThread(socket),
        socket(std::make_unique<MulticastSocket>(juce::IPAddress{localIP},
                                                 juce::IPAddress{multicastIP},
                                                 localPortNumber,
                                                 remotePortNumber)),
        numChannels(numChannelsToSend),
        fifo(numChannelsToSend) {}

NetAudioServer::~NetAudioServer() {
    releaseResources();
//    socket->shutdown(); // TODO: shutdown in multicast socket class
}

void NetAudioServer::disconnect() {
    // When first disconnecting, destroy the socket object and recreate it.
    // A DatagramSocket instance that has been shut down cannot be reused
    // (see DatagramSocket::shutdown()).
    if (senderThread.connected.get()) {
        // TODO: move into multicast socket class
//        socket = std::make_unique<juce::DatagramSocket>();
    }
    senderThread.connected.set(false);
}

void NetAudioServer::prepareToSend(int samplesPerBlockExpected, double sampleRate) {
    fifo.setSize(samplesPerBlockExpected, 4);
    senderThread.prepareToSend(numChannels, samplesPerBlockExpected, sampleRate);
    senderThread.startThread();
    receiverThread.prepareToReceive(numChannels, samplesPerBlockExpected, sampleRate);
    receiverThread.startThread();
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

NetAudioServer::Sender::Sender(std::unique_ptr<MulticastSocket> &socketRef,
                               AudioToNetFifo &fifoRef) :
        juce::Thread{"Sender thread"},
        socket{socketRef},
        fifo{fifoRef} {}

void NetAudioServer::Sender::run() {
    while (!threadShouldExit()) {
        // Attempt to bind a port and join the multicast group.
        if (!connected.get()) {
            auto ready{socket->connect()};
            connected.set(ready);
            if (!ready) {
                wait(1000);
            }
        } else { // Send stuff.
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
        }
    }
}

void NetAudioServer::Sender::prepareToSend(int numChannelsToSend, int samplesPerBlockExpected, double sampleRate) {
    packet.prepare(numChannelsToSend, samplesPerBlockExpected, sampleRate);
    audioBlockSamples = samplesPerBlockExpected;
}

////////////////////////////////////////////////////////////////////////////////
// RECEIVER THREAD
NetAudioServer::Receiver::Receiver(std::unique_ptr<MulticastSocket> &socketRef) :
        juce::Thread("Receiver Thread"),
        socket{socketRef} {}

void NetAudioServer::Receiver::prepareToReceive(int numChannelsToSend, int samplesPerBlock, double sampleRate) {
    packet.prepare(numChannelsToSend, samplesPerBlock, sampleRate);
}

void NetAudioServer::Receiver::run() {
    while (!threadShouldExit()) {
        socket->read(packet);
        wait(1);
    }
}
