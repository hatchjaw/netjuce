//
// Created by Tommy Rushton on 16/02/2023.
//

#ifndef NETJUCE_NETAUDIOSERVER_H
#define NETJUCE_NETAUDIOSERVER_H

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_data_structures/juce_data_structures.h>
#if JUCE_LINUX
#include <sys/epoll.h>
#elif JUCE_MAC
#include <unistd.h>
#include <sys/event.h>
#endif
#include "Constants.h"
#include "Utils.h"
#include "AudioToNetFifo.h"
#include "DatagramAudioPacket.h"
#include "MulticastSocket.h"
#include "NetAudioPeer.h"

/**
 * TODO: Move to /lib; make it a shared library.
 */
class NetAudioServer {
public:
    explicit NetAudioServer(int numChannelsToSend = NUM_SOURCES,
                            const juce::String &multicastIP = DEFAULT_MULTICAST_IP,
                            uint16_t localPortNumber = DEFAULT_LOCAL_PORT,
                            const juce::String &localIP = DEFAULT_LOCAL_ADDRESS,
                            uint16_t remotePortNumber = DEFAULT_AUDIO_SEND_PORT,
                            bool shouldDebug = false);

    ~NetAudioServer();

    void disconnect();

    void prepare(int samplesPerBlockExpected, double sampleRate);

    void handleAudioBlock(const juce::AudioSourceChannelInfo &bufferToSend);

    void releaseResources();

    std::function<void()> onPeerConnected;
    std::function<void()> onPeerDisconnected;
    std::function<void(juce::StringArray &)> onPeersChanged;

    juce::StringArray &getConnectedPeers();

private:
    /**
     * A thread for sending UDP packets to a multicast group.
     * Attempts to set up a connection if one hasn't been established.
     */
    class Sender : public juce::Thread, private juce::Timer {
    public:
        Sender(MulticastSocket::Params &socketParams, AudioToNetFifo &sharedFIFO);

        void prepareToSend(int numChannelsToSend, int samplesPerBlockExpected, double sampleRate);

        void run() override;

        juce::Atomic<bool> connected{false};

    private:
        void timerCallback() override;

        std::unique_ptr<MulticastSocket> socket;
        AudioToNetFifo &fifo;
        DatagramAudioPacket packet;
        int audioBlockSamples{0};
    };

    /**
     * A thread for receiving UDP packets from a multicast group.
     */
    class Receiver : public juce::Thread, private juce::Timer {
    public:
        explicit Receiver(MulticastSocket::Params &socketParams,
                          std::unordered_map<juce::String, std::unique_ptr<NetAudioPeer>> &mapOfPeers);

        void prepareToReceive(int numChannelsToReceive, int samplesPerBlock, double sampleRate);

        void run() override;

        std::function<void()> onPeerConnected;
        std::function<void()> onPeerDisconnected;
        juce::Atomic<bool> connected{false};
    private:
        void timerCallback() override;

        std::unique_ptr<MulticastSocket> socket;
        DatagramAudioPacket packet;
        std::unordered_map<juce::String, std::unique_ptr<NetAudioPeer>> &peers;
        bool &debug;
    };

    Sender sendThread;
    Receiver receiveThread;

    MulticastSocket::Params socketParams;
    int numChannels, samplesPerBlock;
    AudioToNetFifo fifo;
    std::unordered_map<juce::String, std::unique_ptr<NetAudioPeer>> peers;

    juce::CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetAudioServer)
    bool firstSend{true};
};


#endif //NETJUCE_NETAUDIOSERVER_H
