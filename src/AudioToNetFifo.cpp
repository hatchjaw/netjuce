//
// Created by tar on 3/12/23.
//

#include "AudioToNetFifo.h"

AudioToNetFifo::AudioToNetFifo(uint8_t numChannels) :
        fifo(1),
        buffer(std::make_unique<juce::AudioBuffer<float>>(numChannels, 1)),
        converter(std::make_unique<ConverterF32I16>(numChannels, numChannels)) {}

AudioToNetFifo::~AudioToNetFifo() = default;

void AudioToNetFifo::setSize(int numSamples, int redundancy)
{
    const juce::ScopedLock lock{mutex};

    auto capacity{numSamples * redundancy};
    fifo.setTotalSize(capacity);
    buffer->setSize(buffer->getNumChannels(), capacity);
}

void AudioToNetFifo::write(juce::AudioBuffer<float> *src, bool signal)
{
//    const juce::ScopedLock lock{mutex};

    auto writeHandle{fifo.write(src->getNumSamples())};

    if (writeHandle.blockSize1 + writeHandle.blockSize2 != src->getNumSamples()) {
        DBG("WRITE UNDERRUN want to write " << src->getNumSamples() <<
                                            " samples, tried to write " << writeHandle.blockSize1 + writeHandle.blockSize2 <<
                                            " samples. FIFO size " << fifo.getTotalSize() <<
                                            " num ready " << fifo.getNumReady() <<
                                            " free space " << fifo.getFreeSpace() <<
                                            " start1 " << writeHandle.startIndex1 <<
                                            " len1 " << writeHandle.blockSize1 <<
                                            " start2 " << writeHandle.startIndex2 <<
                                            " len2 " << writeHandle.blockSize2 << "\n");
        fifo.reset();
        return;
    }

    // TODO: check number of channels.
    // Assigning to the plugin with fewer than NUM_SOURCES channels is a problem.
//    for (int ch = 0; ch < src->getNumChannels(); ++ch) {
    for (int ch = 0; ch < buffer->getNumChannels(); ++ch) {
        auto readPointer = src->getReadPointer(ch);
        buffer->copyFrom(ch, writeHandle.startIndex1, readPointer, writeHandle.blockSize1);
        buffer->copyFrom(ch, writeHandle.startIndex2, readPointer, writeHandle.blockSize2);
    }

    if (signal) {
        fullBufferAvailable.signal();
    }
}

int AudioToNetFifo::read(uint8_t *dest, int numSamples)
{
    const juce::ScopedLock lock{mutex};

    fullBufferAvailable.wait();

    do {
        fifo.prepareToRead(numSamples, start1, size1, start2, size2);
        if (size1 + size2 != numSamples) {
            DBG("Not enough samples ready to read (" << size1 + size2 << "); trying again.");
        }
    } while (size1 + size2 < numSamples);

//    auto readHandle{fifo.read(numSamples)};

//    fifo.prepareToRead(numSamples, start1, size1, start2, size2);

    auto totalSize{size1 + size2};

    if (totalSize != numSamples) {
        DBG("READ UNDERRUN want " << numSamples <<
                                  " samples got " << totalSize <<
                                  ". FIFO size " << fifo.getTotalSize() <<
                                  " num ready " << fifo.getNumReady() <<
                                  " free space " << fifo.getFreeSpace() <<
                                  " start1 " << start1 <<
                                  " len1 " << size1 <<
                                  " start2 " << start2 <<
                                  " len2 " << size2 << "\n");
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

void AudioToNetFifo::notify()
{
//    const juce::ScopedLock lock{mutex};
    fullBufferAvailable.signal();
}
