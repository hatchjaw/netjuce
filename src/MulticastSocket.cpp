//
// Created by tar on 3/12/23.
//

#include "MulticastSocket.h"

MulticastSocket::MulticastSocket(Mode rwMode,
                                 const juce::IPAddress &localIP,
                                 const juce::IPAddress &multicastIP,
                                 uint16_t localPort,
                                 uint16_t remotePort,
                                 uint connectionTimeoutMs) :
        kTimeoutMs(connectionTimeoutMs),
        mode(rwMode),
        multicastIP(multicastIP),
        localIPToBind(localIP),
        localPortToBind(localPort),
        remotePort(remotePort),
        socket{std::make_unique<juce::DatagramSocket>()} {}

bool MulticastSocket::connect() {
    auto bound{false};
    switch (mode) {
        case READ:
            bound = socket->bindToPort(localPortToBind);
            break;
        case WRITE:
            bound = socket->bindToPort(localPortToBind, localIPToBind.toString());
            break;
        default:
            jassertfalse;
    }
    auto joined{bound && socket->joinMulticast(multicastIP.toString())};
    socket->setMulticastLoopbackEnabled(false);
    if (!bound || !joined) {
        DBG(strerror(errno));
    }
    return joined && socket->waitUntilReady(false, static_cast<int>(kTimeoutMs)) == 1;
}

void MulticastSocket::write(DatagramPacket &packet) {
    // TODO: check for failure, etc.
    socket->write(multicastIP.toString(), remotePort, packet.getData(), static_cast<int>(packet.getSize()));
}

void MulticastSocket::read(DatagramPacket &packet) {
    juce::String senderIP{""};
    int senderPort{0};
    int bytesRead;
    while ((bytesRead = socket->read(packet.getData(), 134, false, senderIP, senderPort)) > 0) {
        DBG(bytesRead << " bytes read from " << senderIP << ":" << senderPort);
    }
    DBG("");
}
