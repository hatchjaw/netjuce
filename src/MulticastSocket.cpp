//
// Created by tar on 3/12/23.
//

#include "MulticastSocket.h"

MulticastSocket::MulticastSocket(const juce::IPAddress &localIP, const juce::IPAddress &multicastIP,
                                 uint16_t localPort, uint16_t remotePort, uint connectionTimeoutMs) :
        kTimeoutMs(connectionTimeoutMs),
        ipToSend(multicastIP),
        ipToBind(localIP),
        portToBind(localPort),
        portToSend(remotePort),
        socket{std::make_unique<juce::DatagramSocket>()} {}

bool MulticastSocket::connect() {
    auto bound{socket->bindToPort(portToBind, ipToBind.toString())};
    auto joined{bound && socket->joinMulticast(ipToSend.toString())};
    if (!bound || !joined) {
        DBG(strerror(errno));
    }
    return joined && socket->waitUntilReady(false, static_cast<int>(kTimeoutMs)) == 1;
}

void MulticastSocket::write(DatagramPacket &packet) {
    // TODO: check for failure, etc.
    socket->write(ipToSend.toString(), portToSend, packet.getData(), static_cast<int>(packet.getSize()));
}
