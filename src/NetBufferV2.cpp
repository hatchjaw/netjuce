//
// Created by tar on 3/12/23.
//

#include "NetBufferV2.h"

NetBufferV2::NetBufferV2(uint8_t numChannels) :
        fifo(1),
        buffer(std::make_unique<juce::AudioBuffer<float>>(numChannels, 1)),
        converter(std::make_unique<ConverterF32I16>(numChannels, numChannels)) {}

NetBufferV2::~NetBufferV2() = default;

void NetBufferV2::read(uint8_t *dest, int len) {
    auto readHandle{fifo.read(len)};

//    auto readHandle = fifo.read(len);
//    auto i{0}, j{0};
//
//    for (; i != readHandle.blockSize1; ++i) {
//        // read the item at index readHandle.startIndex1 + i
//        dest[i] = buffer[readHandle.startIndex1 + i];
//    }
//
//    for (; j != readHandle.blockSize2; ++j) {
//        // read the item at index readHandle.startIndex2 + i
//        dest[i + j] = buffer[readHandle.startIndex2 + j];
//    }

    auto bytesPerChannel{len * kBytesPerSample};
    auto block1Bytes{readHandle.blockSize1 * kBytesPerSample};

    for (int ch = 0; ch < buffer->getNumChannels(); ++ch) {
        auto pos{dest + ch * bytesPerChannel};
        converter->convertSamples(pos,
                                  buffer->getReadPointer(ch, readHandle.startIndex1),
                                  readHandle.blockSize1);
        converter->convertSamples(pos + block1Bytes,
                                  buffer->getReadPointer(ch, readHandle.startIndex2),
                                  readHandle.blockSize2);
    }
}

void NetBufferV2::setSize(int numSamples, int redundancy) {
    auto capacity{numSamples * redundancy};
    fifo.setTotalSize(capacity);
    buffer->setSize(buffer->getNumChannels(), capacity);
}

void NetBufferV2::write(juce::AudioBuffer<float> *const src) {
    auto writeHandle{fifo.write(src->getNumSamples())};

//    auto i{0}, j{0};
//
//    for (; i != writeHandle.blockSize1; ++i) {
//        // write the item at index writeHandle.startIndex1 + i
//        buffer[writeHandle.startIndex1 + i] = src[i];
//    }
//
//    for (; j != writeHandle.blockSize2; ++j) {
//        // write the item at index writeHandle.startIndex2 + i
//        buffer[writeHandle.startIndex2 + j] = src[i + j];
//    }

    for (int ch = 0; ch < buffer->getNumChannels(); ++ch) {
        auto readPointer = src->getReadPointer(ch);
        buffer->copyFrom(ch, writeHandle.startIndex1, readPointer, writeHandle.blockSize1);
        buffer->copyFrom(ch, writeHandle.startIndex2, readPointer, writeHandle.blockSize2);
    }
}
