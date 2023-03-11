//
// Created by Tommy Rushton on 16/02/2023.
//

#include "NetAudioServer.h"

NetAudioServer::NetAudioServer(const juce::String &multicastIPAddress,
                               uint16_t localPortNumber,
                               const juce::String &localIPAddress,
                               uint16_t remotePortNumber,
                               uint numChannelsToSend) :
        connectorThread(socket, localPort, localIP, multicastIP, remotePort, nb),
        socket(std::make_unique<juce::DatagramSocket>()),
        multicastIP(multicastIPAddress),
        localIP(localIPAddress),
        localPort(localPortNumber),
        remotePort(remotePortNumber),
        numChannels(numChannelsToSend),
        converter{std::make_unique<ConverterF32I16>(numChannelsToSend, numChannelsToSend)},
        // Size of netbuffer: big enough to hold 2 x ($m$ channels of $n$ 16-bit samples).
        nb{AUDIO_BLOCK_SAMPLES * sizeof(int16_t) * NUM_SOURCES * 2} {}

NetAudioServer::~NetAudioServer() {
    releaseResources();
    socket->shutdown();
}

void NetAudioServer::disconnect() {
    // When first disconnecting, destroy the UDP object and recreate it.
    // A DatagramSocket instance that has been shut down cannot be reused
    // (see DatagramSocket::shutdown()).
    if (connectorThread.connected.get()) {
        socket = std::make_unique<juce::DatagramSocket>();
    }
    connectorThread.connected.set(false);
}

void NetAudioServer::prepareToSend(int samplesPerBlockExpected, double sampleRate) {
    initHeader(samplesPerBlockExpected, sampleRate);
    bytesPerPacket = sendHeader ?
                     static_cast<int>(PACKET_HEADER_SIZE) +
                     samplesPerBlockExpected * static_cast<int>(numChannels) * static_cast<int>(kBytesPerSample) :
                     samplesPerBlockExpected * static_cast<int>(numChannels) * static_cast<int>(kBytesPerSample);
    DBG("Header size: " << PACKET_HEADER_SIZE << " Packet size: " << bytesPerPacket);
    netBuffer = new uint8_t[static_cast<uint>(bytesPerPacket)];
    connectorThread.startThread();
//    auto options{juce::Thread::RealtimeOptions{6}};
//    connectorThread.startRealtimeThread(options);
}

void NetAudioServer::initHeader(int samplesPerBlockExpected, double sampleRate) {
    header.BitResolution = BIT16;
    header.BufferSize = samplesPerBlockExpected;
    header.NumChannels = NUM_SOURCES;
    header.SeqNumber = 0;
    switch (static_cast<int>(sampleRate)) {
        case 22050:
            header.SamplingRate = SR22;
            break;
        case 32000:
            header.SamplingRate = SR32;
            break;
        case 44100:
            header.SamplingRate = SR44;
            break;
        case 48000:
            header.SamplingRate = SR48;
            break;
        case 88200:
            header.SamplingRate = SR88;
            break;
        case 96000:
            header.SamplingRate = SR96;
            break;
        case 19200:
            header.SamplingRate = SR192;
            break;
        default:
            header.SamplingRate = UNDEF;
            break;
    }
}

bool NetAudioServer::handleAudioBlock(const juce::AudioSourceChannelInfo &bufferToSend) {
    if (connectorThread.connected.get()) {
        // Stop the connector thread if already connected.
        // Don't use this if using connector thread to send data too.
//        if (connectorThread.isThreadRunning()) {
//            connectorThread.stopThread(1000);
//        }

        // Convert from float to int16...
        auto bytesPerSample{static_cast<int>(kBytesPerSample)};
        for (int ch{0}; ch < bufferToSend.buffer->getNumChannels(); ++ch) {
            if (sendHeader) {
                converter->convertSamples(netBuffer + PACKET_HEADER_SIZE + ch * AUDIO_BLOCK_SAMPLES * bytesPerSample,
                                          bufferToSend.buffer->getReadPointer(ch),
                                          bufferToSend.buffer->getNumSamples());
            } else {
                converter->convertSamples(netBuffer + ch * AUDIO_BLOCK_SAMPLES * bytesPerSample,
                                          bufferToSend.buffer->getReadPointer(ch),
                                          bufferToSend.buffer->getNumSamples());
            }
        }

        // Write to the netbuffer.
//        if (seq % 10000 == 0) {
//            std::cout << "after conversion" << std::endl;
//            Utils::hexDump(netBuffer, bytesPerPacket);
//        }
        nb.write(netBuffer, bytesPerPacket);

        // Let the sender thread know there's a packet ready to send.
        connectorThread.notify();

//        auto now{juce::Time::getMillisecondCounterHiRes()};
//        DBG(now - time);
//        time = now;

        // TODO: move this to connector/sender thread.
//        if (sendHeader) {
//            // Copy the header
//            ++header.SeqNumber;
//            memcpy(netBuffer, &header, PACKET_HEADER_SIZE);
//        }

        // This kind of works, but with lots of jitter.
        // ...and send.
//        auto bytesWritten{udp->write(multicastIP, remotePort, netBuffer, bytesPerPacket)};
//        if (bytesWritten == -1) {
//            DBG(strerror(errno));
//        }
//        return bytesWritten == bytesPerPacket;

        return true;
    } else {
        DBG("NetAudioServer is not connected.");
//        connectorThread.startThread();
    }

    return false;
}

void NetAudioServer::releaseResources() {
    delete[] netBuffer;
    if (connectorThread.isThreadRunning()) {
        connectorThread.stopThread(1000);
    }
}

////////////////////////////////////////////////////////////////////////////////
// CONNECTOR THREAD

NetAudioServer::ConnectorThread::ConnectorThread(
        std::unique_ptr<juce::DatagramSocket> &udpRef,
        uint16_t &localPortToUse,
        juce::String &localIPToUse,
        juce::String &multicastIPToUse,
        uint16_t &remotePortToUse,
        NetBuffer &nb) :
        juce::Thread{"Connector thread"},
        socket{udpRef},
        localPort{localPortToUse},
        remotePort{remotePortToUse},
        localIP{localIPToUse},
        multicastIP{multicastIPToUse},
        netBuffer{&nb} {}

void NetAudioServer::ConnectorThread::run() {
    auto seq{0};
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
            // This works fine, and sends packets at reasonably uniform intervals
//            auto later{juce::Time::getHighResolutionTicks() + kBufferPeriod};
//            while (juce::Time::getHighResolutionTicks() < later);
//            char buf[5]{"yolo"};
//            udp->write(*multicastIP, *remotePort, buf, 5);

//            if (fifo->getNumReady() >= AUDIO_BLOCK_SAMPLES) {
////                DBG("Ready to handleAudioBlock " << fifo.getNumReady() << " samples");
//                auto readHandle{fifo->read(AUDIO_BLOCK_SAMPLES)};
//                char buf[5]{"yolo"};
//                udp->write(*multicastIP, *remotePort, buf, 5);
//                fifo->reset();
//            }
//            wait(1);

            // Wait for the audio thread to notify that a new block is available.
//            if (wait(1)) {
////                DBG("wait() notified");
//                char buf[5]{"yolo"};
//                socket->write(*multicastIP, *remotePort, buf, 5);
//            } else {
//                DBG("wait() timed out");
//            }

            wait(-1);
            uint8_t *buf;
            auto bytes{AUDIO_BLOCK_SAMPLES * NUM_SOURCES * 2};
            netBuffer->read(buf, bytes);
            if (seq % 10000 <= 1) {
                std::cout << "before send " << seq << std::endl;
                Utils::hexDump(buf, bytes);
            }
            socket->write(multicastIP, remotePort, buf, bytes);
            ++seq;
        }
    }
}
