//
// Created by tar on 3/12/23.
//

#include "AudioToNetFifo.h"

AudioToNetFifo::AudioToNetFifo(uint8_t numChannels) :
        fifo(1),
        buffer(std::make_unique<juce::AudioBuffer<float>>(numChannels, 1)),
        converter(std::make_unique<ConverterF32I16>(numChannels, numChannels)) {}

AudioToNetFifo::~AudioToNetFifo() = default;

void AudioToNetFifo::setSize(int numSamples, int redundancy) {
    auto capacity{numSamples * redundancy};
    fifo.setTotalSize(capacity);
    buffer->setSize(buffer->getNumChannels(), capacity);
}

void AudioToNetFifo::write(juce::AudioBuffer<float> *const src) {
    const juce::ScopedLock lock{mutex};

    auto writeHandle{fifo.write(src->getNumSamples())};

    for (int ch = 0; ch < buffer->getNumChannels(); ++ch) {
        auto readPointer = src->getReadPointer(ch);
        buffer->copyFrom(ch, writeHandle.startIndex1, readPointer, writeHandle.blockSize1);
        buffer->copyFrom(ch, writeHandle.startIndex2, readPointer, writeHandle.blockSize2);
    }
}

void AudioToNetFifo::read(uint8_t *dest, int numSamples) {
    const juce::ScopedLock lock{mutex};

    auto readHandle{fifo.read(numSamples)};

    auto bytesPerChannel{numSamples * kBytesPerSample};
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
