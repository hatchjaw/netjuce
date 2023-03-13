//
// Created by Tommy Rushton on 16/02/2023.
//

#ifndef NETJUCE_NETAUDIOSERVER_H
#define NETJUCE_NETAUDIOSERVER_H

#include <juce_core/juce_core.h>
#include "Constants.h"
#include "Utils.h"
#include "AudioToNetFifo.h"
#include "DatagramPacket.h"
#include "MulticastSocket.h"

class NetAudioServer {
public:
    explicit NetAudioServer(const juce::String &multicastIP = DEFAULT_MULTICAST_IP,
                            uint16_t localPortNumber = DEFAULT_LOCAL_PORT,
                            const juce::String &localIP = DEFAULT_LOCAL_ADDRESS,
                            uint16_t remotePortNumber = DEFAULT_REMOTE_PORT,
                            int numChannelsToSend = NUM_SOURCES);

    ~NetAudioServer();

    void disconnect();

    void prepareToSend(int samplesPerBlockExpected, double sampleRate);

    bool handleAudioBlock(const juce::AudioSourceChannelInfo &bufferToSend);

    void releaseResources();

private:
    /**
     * A thread for multicasting UDP packets. Attempts to set up a connection if
     * one hasn't been established.
     */
    class Sender : public juce::Thread {
    public:
        Sender(std::unique_ptr<MulticastSocket> &socketRef, AudioToNetFifo &fifoRef);

        void run() override;

        juce::Atomic<bool> connected{false};

        void prepareToSend(int numChannelsToSend, int samplesPerBlockExpected, double sampleRate);

    private:
        const int kTimeout{5000};
        std::unique_ptr<MulticastSocket> &socket;
        AudioToNetFifo &fifo;
        DatagramPacket packet;
        int audioBlockSamples{0};
    };

    class Receiver : public juce::Thread {
    public:
        explicit Receiver(std::unique_ptr<MulticastSocket> &socketRef);

        void run() override;

        void prepareToReceive(int numChannelsToSend, int samplesPerBlock, double sampleRate);

    private:
        std::unique_ptr<MulticastSocket> &socket;
        DatagramPacket packet;
    };

    Sender senderThread;
    Receiver receiverThread;

    std::unique_ptr<MulticastSocket> socket;
    int numChannels;
    AudioToNetFifo fifo;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetAudioServer)
};


#endif //NETJUCE_NETAUDIOSERVER_H
