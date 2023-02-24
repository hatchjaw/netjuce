//
// Created by Tommy Rushton on 16/02/2023.
//

#ifndef NETJUCE_NETAUDIOSERVER_H
#define NETJUCE_NETAUDIOSERVER_H

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "Constants.h"

using ConverterF32I16 = juce::AudioData::ConverterInstance<
        juce::AudioData::Pointer<
                juce::AudioData::Float32, // JUCE shunts samples around as 32-bit floats
                juce::AudioData::NativeEndian,
                juce::AudioData::NonInterleaved,
                juce::AudioData::Const
        >,
        juce::AudioData::Pointer<
                juce::AudioData::Int16, // For a start, send 16 bit samples
                juce::AudioData::BigEndian, // Because this is how UDP works, I think.
                juce::AudioData::NonInterleaved,
                juce::AudioData::NonConst
        >
>;

class NetAudioServer {
public:
    explicit NetAudioServer(const juce::String &multicastIP = DEFAULT_MULTICAST_IP,
                            uint16_t localPort = DEFAULT_LOCAL_PORT,
                            const juce::String &localIP = DEFAULT_LOCAL_ADDRESS,
                            uint16_t remotePort = DEFAULT_REMOTE_PORT,
                            uint numChannelsToSend = NUM_SOURCES);

    ~NetAudioServer();

    void disconnect();

    void prepareToSend(int samplesPerBlockExpected);

    bool send(const juce::AudioSourceChannelInfo &bufferToSend);

    void releaseResources();

private:
    /**
     * A thread to check periodically whether UDP is connected, and, if not,
     * attempt to set up a connection.
     */
    class ConnectorThread : public juce::Thread {
    public:
        ConnectorThread(std::unique_ptr<juce::DatagramSocket> &udpRef,
                        uint16_t &localPortToUse,
                        juce::String&localIPToUse,
                        juce::String &multicastIPToUse) :
                juce::Thread("Connector thread"),
                udp(udpRef),
                localPort(&localPortToUse),
                localIP(&localIPToUse),
                multicastIP(&multicastIPToUse){}

        void run() override {
            while (!threadShouldExit()) {
                if (udp->getBoundPort() < 0 || !connected.load()) {
                    auto bound{udp->bindToPort(*localPort, *localIP)};
                    auto joined{bound && udp->joinMulticast(*multicastIP)};
                    if (!bound || !joined) {
                        DBG(strerror(errno));
                    }
                    connected.store(joined && udp->waitUntilReady(false, kTimeout) == 1);
                }

                wait(1000);
            }
        }

        std::atomic<bool> connected{false};
    private:
        std::unique_ptr<juce::DatagramSocket> &udp;
        uint16_t *localPort;
        juce::String *localIP, *multicastIP;
        const int kTimeout{5000};
    };

    ConnectorThread connectorThread;

    const uint kBytesPerSample{2};
    std::unique_ptr<juce::DatagramSocket> udp;
    juce::String multicastIP, localIP;
    uint16_t localPort, remotePort;
    uint8_t buffer[5]{"yolo"};
    uint numChannels;
    std::unique_ptr<ConverterF32I16> converter;
    int bytesPerPacket{0};
    uint8_t *netBuffer{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetAudioServer)
};


#endif //NETJUCE_NETAUDIOSERVER_H
