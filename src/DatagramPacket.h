//
// Created by tar on 3/12/23.
//

#ifndef NETJUCE_DATAGRAMPACKET_H
#define NETJUCE_DATAGRAMPACKET_H

#include <juce_core/juce_core.h>

class DatagramPacket : public juce::MemoryBlock {
public:
    enum BitResolutionT {
        BIT8 = 1,
        BIT16 = 2,
        BIT24 = 3,
        BIT32 = 4
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

    void prepare(int numChannels, int bufferSize, double sampleRate);

    void incrementSeqNumber();

    /**
     * Write the header to the packet
     */
    void writeHeader();

    /**
     * Get a pointer to the audio data portion of the packet.
     */
    uint8_t *getAudioData();

    int getSeqNumber() const;

private:
    PacketHeader header{};
};

#define PACKET_HEADER_SIZE sizeof(DatagramPacket::PacketHeader)

#endif //NETJUCE_DATAGRAMPACKET_H
