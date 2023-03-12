//
// Created by tar on 3/12/23.
//

#include "DatagramPacket.h"

void DatagramPacket::prepare(int numChannels, int bufferSize, double sampleRate) {
    setSize(PACKET_HEADER_SIZE + static_cast<size_t>(numChannels * bufferSize) * sizeof(int16_t));

    header.BitResolution = BIT16;
    header.BufferSize = bufferSize;
    header.NumChannels = numChannels;
    header.SeqNumber = 0;
    switch (static_cast<int>(sampleRate)) {
        case 22050:
            header.SamplingRate = SR22;
            break;
        case 32000:
            header.SamplingRate = SR32;
            break;
        case 44100:
            header.SamplingRate = SR44;
            break;
        case 48000:
            header.SamplingRate = SR48;
            break;
        case 88200:
            header.SamplingRate = SR88;
            break;
        case 96000:
            header.SamplingRate = SR96;
            break;
        case 19200:
            header.SamplingRate = SR192;
            break;
        default:
            header.SamplingRate = UNDEF;
            break;
    }
}

void DatagramPacket::writeHeader() {
    copyFrom(&header, 0, PACKET_HEADER_SIZE);
}

uint8_t *DatagramPacket::getAudioData() {
    return reinterpret_cast<uint8_t *>(getData()) + PACKET_HEADER_SIZE;
}

void DatagramPacket::incrementSeqNumber() {
    ++header.SeqNumber;
}

int DatagramPacket::getSeqNumber() const {
    return header.SeqNumber;
}
