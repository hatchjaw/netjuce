//
// Created by tar on 3/12/23.
//

#ifndef NETJUCE_MULTICASTSOCKET_H
#define NETJUCE_MULTICASTSOCKET_H

#include <juce_core/juce_core.h>
#include "DatagramAudioPacket.h"

/**
 * Defines a UDP socket for multicast communication.
 *
 * Whether sending or receiving, a valid multicast IP address is required, plus
 * a local port number to bind to.
 *
 * To send to a DHCP-less network switch, the IP address of a local network
 * adaptor is required. The number of a remote port to target must also be
 * provided.
 *
 * To receive, there's no need to bind to a specific local network adaptor,
 * and nor is the remote port necessary.
 */
class MulticastSocket {
public:
    struct Params {
        juce::IPAddress MulticastIP;
        uint16_t LocalPort;
        juce::IPAddress LocalIP{"0.0.0.0"};
        uint16_t RemotePort{0};
    };

    enum Mode {
        READ,
        WRITE
    };

    MulticastSocket(Mode rwMode, Params &p, uint connectionTimeoutMs = 1000);

    bool connect();

    void write(DatagramAudioPacket &packet);

    int read(DatagramAudioPacket &packet);

    int getRawHandle();

private:
    const uint kTimeoutMs;

    Mode mode;
    Params &params;
    std::unique_ptr<juce::DatagramSocket> socket;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MulticastSocket)
};


#endif //NETJUCE_MULTICASTSOCKET_H
