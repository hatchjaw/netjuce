//
// Created by Tommy Rushton on 16/02/2023.
//

#ifndef NETJUCE_NETAUDIOSERVER_H
#define NETJUCE_NETAUDIOSERVER_H

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "Constants.h"
#include "PacketHeader.h"
#include "NetBuffer.h"
#include "Utils.h"

using ConverterF32I16 = juce::AudioData::ConverterInstance<
        juce::AudioData::Pointer<
                // JUCE shunts samples around as 32-bit floats
                juce::AudioData::Float32,
                juce::AudioData::NativeEndian,
                juce::AudioData::NonInterleaved,
                juce::AudioData::Const
        >,
        juce::AudioData::Pointer<
                // Send 16 bit samples (higher res may be preferable, but this
                // is a start)
                juce::AudioData::Int16,
                // Comparing Wireshark captures reveals that little is the
                // appropriate endianness.
                // Sent a square wave with 32-sample period via jacktrip and
                // observed:
                //
                //0000   04 e9 e5 0f c6 14 a0 36 bc d0 aa 18 08 00 45 b8
                //0010   00 ac fc a6 40 00 40 11 a7 69 c0 a8 0a 0a c0 a8
                //0020   0a 1e ee 4a 22 b8 00 98 ce 44 12 ae 18 13 fc f5
                //0030   05 00 31 36 20 00 02 10 02 00 fe 7f fe 7f fe 7f
                //0040   fe 7f fe 7f fe 7f fe 7f fe 7f fe 7f fe 7f fe 7f
                //0050   fd 7f fe 7f fe 7f fd 7f fe 7f 01 80 04 80 01 80
                //0060   02 80 01 80 01 80 03 80 01 80 05 80 01 80 04 80
                //0070   01 80 01 80 03 80 01 80 03 80 fc 7f fe 7f fc 7f
                //0080   fe 7f fb 7f fe 7f fa 7f fe 7f fc 7f fe 7f fe 7f
                //0090   fe 7f fd 7f fe 7f fd 7f fe 7f 01 80 03 80 01 80
                //00a0   03 80 01 80 02 80 01 80 01 80 01 80 01 80 01 80
                //00b0   01 80 01 80 02 80 01 80 03 80
                //
                // Up to 0039 is header data (ethernet + ip + udp + jacktrip)
                //
                // Sending big endian here resulted in equivalent:
                //
                //0000   01 00 5e 06 26 e2 a0 36 bc d0 aa 18 08 00 45 00
                //0010   00 9c 3e d7 40 00 01 11 66 df c0 a8 0a 0a e2 06
                //0020   26 e2 4a 37 4a 37 00 88 96 d0 80 00 80 05 80 00
                //0030   80 01 80 02 80 00 80 05 80 00 80 02 80 00 80 00
                //0040   80 01 80 00 80 03 80 00 80 01 7f fe 7f fe 7f fe
                //0050   7f fe 7f fe 7f fe 7f fe 7f fd 7f fe 7f fe 7f fe
                //0060   7f fe 7f fd 7f fe 7f fe 7f fe 80 00 80 05 80 00
                //0070   80 01 80 02 80 00 80 05 80 00 80 02 80 00 80 00
                //0080   80 01 80 00 80 03 80 00 80 01 7f fe 7f fe 7f fe
                //0090   7f fe 7f fe 7f fe 7f fe 7f fd 7f fe 7f fe 7f fe
                //00a0   7f fe 7f fd 7f fe 7f fe 7f fe
                //
                // Where up to 0029 is header (obviously no 16 byte jacktrip
                // header here)
                // As can be seen, each 16-bit (2-byte) word is reversed in the
                // Big endian version.
                //
                // Switching to little endian:
                //
                //0000   01 00 5e 06 26 e2 a0 36 bc d0 aa 18 08 00 45 00
                //0010   00 9c 07 9f 40 00 01 11 9e 17 c0 a8 0a 0a e2 06
                //0020   26 e2 4a 37 4a 37 00 88 a2 d4 fe 7f fe 7f fe 7f
                //0030   fe 7f fe 7f fc 7f fe 7f fc 7f 01 80 02 80 00 80
                //0040   03 80 00 80 00 80 02 80 00 80 03 80 00 80 01 80
                //0050   00 80 01 80 00 80 01 80 00 80 fe 7f fe 7f fe 7f
                //0060   fe 7f fe 7f fe 7f fe 7f fe 7f fe 7f fe 7f fe 7f
                //0070   fe 7f fe 7f fc 7f fe 7f fc 7f 01 80 02 80 00 80
                //0080   03 80 00 80 00 80 02 80 00 80 03 80 00 80 01 80
                //0090   00 80 01 80 00 80 01 80 00 80 fe 7f fe 7f fe 7f
                //00a0   fe 7f fe 7f fe 7f fe 7f fe 7f
                //
                // All good. Of course, could send big endian and convert
                // client-side, but it's reasonable to copy what jacktrip does.
                juce::AudioData::LittleEndian,
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
        Sender(std::unique_ptr<juce::DatagramSocket> &udpRef,
               uint16_t &localPortToUse,
               juce::String &localIPToUse,
               juce::String &multicastIPToUse,
               uint16_t &remotePortToUse,
               NetBuffer &nb);

        void run() override;

        juce::Atomic<bool> connected{false};

        void initHeader(int samplesPerBlock, double sampleRate);
    private:
        std::unique_ptr<juce::DatagramSocket> &socket;
        uint16_t &localPort, &remotePort;
        juce::String &localIP, &multicastIP;
        const int kTimeout{5000};
        NetBuffer *netBuffer;
        PacketHeader header{};
    };

    Sender senderThread;

    const uint kBytesPerSample{2};
    std::unique_ptr<juce::DatagramSocket> socket;
    juce::String multicastIP, localIP;
    uint16_t localPort, remotePort;
    uint numChannels;
    std::unique_ptr<ConverterF32I16> converter;
    int bytesPerPacket{0};
    uint8_t *sixteenBitBuffer{};
    NetBuffer nb;
    int seq{0};
    double time{0};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NetAudioServer)
};


#endif //NETJUCE_NETAUDIOSERVER_H
