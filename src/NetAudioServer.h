//
// Created by Tommy Rushton on 16/02/2023.
//

#ifndef NETJUCE_NETAUDIOSERVER_H
#define NETJUCE_NETAUDIOSERVER_H

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

class NetAudioServer {
public:
    NetAudioServer(const juce::String &multicastIP,
                   uint16_t localPort,
                   const juce::String &localIP,
                   uint16_t remotePort);

    ~NetAudioServer();

    bool connect();
    bool send(const juce::AudioSourceChannelInfo &bufferToSend);

private:
    const int kTimeout{5000};
    std::unique_ptr<juce::DatagramSocket> udp;
    juce::String multicastIP, localIP;
    uint16_t localPort, remotePort;
    bool connected;
    uint8_t buffer[5]{0xff, 0xff, 0xff, 0xff, 0xff};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetAudioServer)
};


#endif //NETJUCE_NETAUDIOSERVER_H
