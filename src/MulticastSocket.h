//
// Created by tar on 3/12/23.
//

#ifndef NETJUCE_MULTICASTSOCKET_H
#define NETJUCE_MULTICASTSOCKET_H

#include <juce_core/juce_core.h>
#include "DatagramPacket.h"

class MulticastSocket {

private:
public:
    MulticastSocket(const juce::IPAddress &localIP,
                    const juce::IPAddress &multicastIP,
                    uint16_t localPort, uint16_t remotePort,
                    uint connectionTimeoutMs = 1000);

    bool connect();

    void write(DatagramPacket &packet);

    void read(DatagramPacket &packet);

private:
    const uint kTimeoutMs;

    juce::IPAddress ipToSend, ipToBind;
    uint16_t portToBind, portToSend;
    std::unique_ptr<juce::DatagramSocket> socket;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MulticastSocket)
};


#endif //NETJUCE_MULTICASTSOCKET_H
