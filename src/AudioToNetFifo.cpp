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
    const juce::ScopedLock lock{mutex};

    auto capacity{numSamples * redundancy};
    fifo.setTotalSize(capacity);
    buffer->setSize(buffer->getNumChannels(), capacity);
}

void AudioToNetFifo::write(juce::AudioBuffer<float> *const src) {
    const juce::ScopedLock lock{mutex};

    auto writeHandle{fifo.write(src->getNumSamples())};

    if (writeHandle.blockSize1 + writeHandle.blockSize2 != src->getNumSamples()) {
        DBG("WRITE UNDERRUN " << writeHandle.startIndex1 << " " <<
                              writeHandle.blockSize1 << " " <<
                              writeHandle.startIndex2 << " " <<
                              writeHandle.blockSize2 << "\n");
        fifo.reset();
    }

//    for (int ch = 0; ch < src->getNumChannels(); ++ch) {
    for (int ch = 0; ch < buffer->getNumChannels(); ++ch) {
        auto readPointer = src->getReadPointer(ch);
        buffer->copyFrom(ch, writeHandle.startIndex1, readPointer, writeHandle.blockSize1);
        buffer->copyFrom(ch, writeHandle.startIndex2, readPointer, writeHandle.blockSize2);
    }

    fullBufferAvailable.signal();
}

int AudioToNetFifo::read(uint8_t *dest, int numSamples) {
    fullBufferAvailable.wait();

    const juce::ScopedLock lock{mutex};

//    auto readHandle{fifo.read(numSamples)};

    fifo.prepareToRead(numSamples, start1, size1, start2, size2);

    auto totalSize{size1 + size2};

    if (totalSize != numSamples) {
        DBG("READ UNDERRUN " << start1 << " " <<
                             size1 << " " <<
                             start2 << " " <<
                             size2 << "\n");
        fifo.reset();
        return totalSize;
    }

    auto bytesPerChannel{numSamples * kBytesPerSample};
    auto block1Bytes{size1 * kBytesPerSample};

    for (int ch = 0; ch < buffer->getNumChannels(); ++ch) {
        auto pos{dest + ch * bytesPerChannel};
        converter->convertSamples(pos,
                                  buffer->getReadPointer(ch, start1),
                                  size1);
        converter->convertSamples(pos + block1Bytes,
                                  buffer->getReadPointer(ch, start2),
                                  size2);
    }

    fifo.finishedRead(totalSize);

    return numSamples;
}

void AudioToNetFifo::notify() {
    const juce::ScopedLock lock{mutex};
    fullBufferAvailable.signal();
}
