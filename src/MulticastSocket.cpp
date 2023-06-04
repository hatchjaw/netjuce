//
// Created by tar on 3/12/23.
//

#include "MulticastSocket.h"

MulticastSocket::MulticastSocket(Mode rwMode, Params &p, uint connectionTimeoutMs) :
        kTimeoutMs(connectionTimeoutMs),
        mode(rwMode),
        params(p),
        socket{std::make_unique<juce::DatagramSocket>()} {}

bool MulticastSocket::connect() {
    if (socket->getBoundPort() > 0) {
        return true;
    }

    auto bound{false};
    switch (mode) {
        case READ:
            bound = socket->bindToPort(params.LocalPort);
            break;
        case WRITE:
            bound = socket->bindToPort(params.LocalPort - 1, params.LocalIP.toString());
            break;
        default:
            jassertfalse;
    }
    auto joined{bound && socket->joinMulticast(params.MulticastIP.toString())};
    socket->setMulticastLoopbackEnabled(false);
    if (!bound || !joined) {
        DBG(strerror(errno));
    }
    // OK... waiting for read-readiness doesn't appear to work. Perhaps because
    // there may not be an incoming network stream at this stage?
    return joined && socket->waitUntilReady(false, static_cast<int>(kTimeoutMs)) == 1;
}

void MulticastSocket::write(DatagramAudioPacket &packet) {
    if (params.debug && packet.getSeqNumber() % 10000 <= 1) {
        std::cout << "Send, seq no. " << packet.getSeqNumber() << std::endl;
        std::cout << "Destination: " << params.MulticastIP.toString() << ":" << params.RemotePort << std::endl;
        Utils::hexDump(reinterpret_cast<uint8_t *>(packet.getData()), static_cast<int>(packet.getSize()), true);
    }

    // TODO: check for failure, etc.
    socket->write(params.MulticastIP.toString(),
                  params.RemotePort,
                  packet.getData(),
                  static_cast<int>(packet.getSize()));
}

int MulticastSocket::read(DatagramAudioPacket &packet) {
    juce::String senderIP{""};
    int senderPort{0};
    int bytesRead{socket->read(packet.getData(), 1024, false, senderIP, senderPort)};
    packet.setOrigin(juce::IPAddress{senderIP}, senderPort);
    packet.parseHeader();

    return bytesRead;
}

int MulticastSocket::getRawHandle() {
    return socket->getRawSocketHandle();
}
