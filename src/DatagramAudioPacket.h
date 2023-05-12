//
// Created by tar on 3/12/23.
//

#ifndef NETJUCE_DATAGRAMAUDIOPACKET_H
#define NETJUCE_DATAGRAMAUDIOPACKET_H

#include <juce_core/juce_core.h>

class DatagramAudioPacket : public juce::MemoryBlock {
public:
    enum BufferSizeT {
        BUF8 = 3,
        BUF16,
        BUF32,
        BUF64,
        BUF128,
        BUF256,
        BUF512,
        BUF1024,
        BUF2048,
        BUF4096
    };

    enum BitResolutionT {
        BIT8 = 1,
        BIT16,
        BIT24,
        BIT32
    };

    enum SamplingRateT {
        SR22,
        SR32,
        SR44,
        SR48,
        SR88,
        SR96,
        SR192,
        UNDEF
    };

    struct PacketHeader {
        uint16_t SeqNumber;
        uint8_t BufferSize;
        uint8_t SamplingRate;
        uint8_t BitResolution;
        uint8_t NumChannels;
    };

    struct Origin {
        juce::IPAddress IP;
        uint16_t Port;
    };

    void prepare(int numChannels, int bufferSize, double sampleRate);

    void incrementSeqNumber();

    /**
     * Write the header to the packet
     */
    void writeHeader();

    /**
     * Get a pointer to the audio data portion of the packet. Specify a channel
     * number to get a pointer to the data for a specific channel.
     */
    uint8_t *getAudioData(uint channel = 0);

    int getSeqNumber() const;

    DatagramAudioPacket::Origin getOrigin();

    int getNumAudioChannels() const;

    /**
     * Get the number of audio samples (well frames) in the packet.
     * @return
     */
    int getNumSamples() const;

    void parseHeader();

    void setOrigin(juce::IPAddress ip, uint16_t port);

private:
    Origin origin;
    PacketHeader header{};
    size_t bytesPerChannel;
};

#define PACKET_HEADER_SIZE sizeof(DatagramAudioPacket::PacketHeader)

#endif //NETJUCE_DATAGRAMAUDIOPACKET_H
