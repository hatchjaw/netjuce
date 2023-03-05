//
// Created by Tommy Rushton on 16/02/2023.
//

#include "NetAudioServer.h"

NetAudioServer::NetAudioServer(const juce::String &multicastIPAddress,
                               uint16_t localPortNumber,
                               const juce::String &localIPAddress,
                               uint16_t remotePortNumber,
                               uint numChannelsToSend) :
        connectorThread(udp, localPort, localIP, multicastIP),
        udp(std::make_unique<juce::DatagramSocket>()),
        multicastIP(multicastIPAddress),
        localIP(localIPAddress),
        localPort(localPortNumber),
        remotePort(remotePortNumber),
        numChannels(numChannelsToSend),
        converter(std::make_unique<ConverterF32I16>(numChannelsToSend, numChannelsToSend)) {}

NetAudioServer::~NetAudioServer() {
    releaseResources();
    udp->shutdown();
}

void NetAudioServer::disconnect() {
    // When first disconnecting, destroy the UDP object and recreate it.
    // A DatagramSocket instance that has been shut down cannot be reused
    // (see DatagramSocket::shutdown()).
    if (connectorThread.connected.load()) {
        udp = std::make_unique<juce::DatagramSocket>();
    }
    connectorThread.connected.store(false);
}

void NetAudioServer::prepareToSend(int samplesPerBlockExpected, double sampleRate) {
    header.BitResolution = BitResolutionT::BIT16;
    header.BufferSize = samplesPerBlockExpected;
    header.NumChannels = NUM_SOURCES;
    header.SeqNumber = 0;
    header.SamplingRate = static_cast<int>(sampleRate);
    bytesPerPacket = static_cast<int>(PACKET_HEADER_SIZE) +
                     samplesPerBlockExpected * static_cast<int>(numChannels) * static_cast<int>(kBytesPerSample);
    netBuffer = new uint8_t[static_cast<uint>(bytesPerPacket)];
    connectorThread.startThread();
}

bool NetAudioServer::send(const juce::AudioSourceChannelInfo &bufferToSend) {
    if (connectorThread.connected.load()) {
        ++header.SeqNumber;
        memcpy(netBuffer, &header, PACKET_HEADER_SIZE);

        // Convert from float to int16...
        auto bytesPerSample{static_cast<int>(kBytesPerSample)};
        for (int ch{0}; ch < bufferToSend.buffer->getNumChannels(); ++ch) {
            converter->convertSamples(netBuffer + PACKET_HEADER_SIZE + ch * AUDIO_BLOCK_SAMPLES * bytesPerSample,
                                      bufferToSend.buffer->getReadPointer(ch),
                                      bufferToSend.buffer->getNumSamples());
        }

        // ...and send.
        auto bytesWritten{udp->write(multicastIP, remotePort, netBuffer, bytesPerPacket)};
        if (bytesWritten == -1) {
            DBG(strerror(errno));
        }
        return bytesWritten == bytesPerPacket;
    } else {
        DBG("NetAudioServer is not connected.");
    }

    return false;
}

void NetAudioServer::releaseResources() {
    delete[] netBuffer;
    connectorThread.stopThread(1000);
}
