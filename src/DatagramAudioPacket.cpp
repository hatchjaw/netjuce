//
// Created by tar on 3/12/23.
//

#include "DatagramAudioPacket.h"

void DatagramAudioPacket::prepare(int numChannels, int bufferSize, double sampleRate) {
    bytesPerChannel = static_cast<size_t>(bufferSize) * sizeof(int16_t); // TODO: generalise this.
    setSize(PACKET_HEADER_SIZE + static_cast<size_t>(numChannels) * bytesPerChannel);

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

void DatagramAudioPacket::writeHeader() {
    copyFrom(&header, 0, PACKET_HEADER_SIZE);
}

uint8_t *DatagramAudioPacket::getAudioData(uint channel) {
    return reinterpret_cast<uint8_t *>(getData()) + PACKET_HEADER_SIZE + channel * bytesPerChannel;
}

void DatagramAudioPacket::incrementSeqNumber() {
    ++header.SeqNumber;
}

int DatagramAudioPacket::getSeqNumber() const {
    return header.SeqNumber;
}

DatagramAudioPacket::Origin DatagramAudioPacket::getOrigin() {
    return origin;
}

int DatagramAudioPacket::getNumAudioChannels() const {
    return header.NumChannels;
}

int DatagramAudioPacket::getNumSamples() const {
    return header.BufferSize;
}

void DatagramAudioPacket::parseHeader() {
    header = *reinterpret_cast<PacketHeader *>(getData());
}

void DatagramAudioPacket::setOrigin(DatagramAudioPacket::Origin o) {
    origin = o;
}
