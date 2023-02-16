//
// Created by Tommy Rushton on 16/02/2023.
//

#include "NetAudioServer.h"

NetAudioServer::NetAudioServer(const juce::String &multicastIPAddress,
                               uint16_t localPortNumber,
                               const juce::String &localIPAddress,
                               uint16_t remotePortNumber) :
        udp(std::make_unique<juce::DatagramSocket>()),
        multicastIP(multicastIPAddress),
        localIP(localIPAddress),
        localPort(localPortNumber),
        remotePort(remotePortNumber) {
}

NetAudioServer::~NetAudioServer() {
    udp->shutdown();
}

bool NetAudioServer::connect() {
    auto bound{udp->bindToPort(localPort)};//, localIP)};
    auto joined{bound && udp->joinMulticast(multicastIP)};
    connected = joined && udp->waitUntilReady(false, kTimeout) == 1;
    return connected;
}

bool NetAudioServer::send(const juce::AudioSourceChannelInfo &bufferToSend) {
    jassert(connected);

    auto bytesWritten{udp->write(multicastIP, remotePort, buffer, sizeof(buffer))};
    return bytesWritten == sizeof(buffer);
}
