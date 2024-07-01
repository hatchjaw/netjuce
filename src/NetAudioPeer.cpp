//
// Created by tar on 3/16/23.
//

#include "NetAudioPeer.h"

NetAudioPeer::NetAudioPeer(DatagramAudioPacket firstPacket, int bufferRedundancy) :
        origin(firstPacket.getOrigin()),
        lastReceiveTime(juce::Time::getMillisecondCounter()),
        audioBuffer(std::make_unique<juce::AudioBuffer<float>>(
                firstPacket.getNumAudioChannels(),
                firstPacket.getNumFrames() * bufferRedundancy)
        ),
        tempBuffer(std::make_unique<juce::AudioBuffer<float>>(
                firstPacket.getNumAudioChannels(),
                firstPacket.getNumFrames())
        ),
        fifo(firstPacket.getNumFrames() * bufferRedundancy)
{
}

void NetAudioPeer::handlePacket(DatagramAudioPacket &p)
{
    lastReceiveTime = juce::Time::getMillisecondCounter();
    origin = p.getOrigin();

//    auto writeHandle{fifo.write(p.getNumFrames())};
//
//    if (writeHandle.blockSize1 + writeHandle.blockSize2 != p.getNumFrames()) {
//        fifo.reset();
//        writeHandle = fifo.write(p.getNumFrames());
//    }

    for (int ch = 0; ch < p.getNumAudioChannels(); ++ch) {
//        auto w{tempBuffer->getWritePointer(ch)};
//        auto r{tempBuffer->getReadPointer(ch)};
//        converter.convertSamples(w, p.getAudioData(static_cast<uint>(ch)), p.getNumFrames());
//        audioBuffer->copyFrom(ch, writeHandle.startIndex1, r, writeHandle.blockSize1);
//        audioBuffer->copyFrom(ch, writeHandle.startIndex2, r, writeHandle.blockSize2);

        auto w{audioBuffer->getWritePointer(ch)};
        converter.convertSamples(w, p.getAudioData(static_cast<uint>(ch)), p.getNumFrames());
    }
}

void NetAudioPeer::getNextAudioBlock(juce::AudioBuffer<float> &bufferToFill, int channelToWriteTo, int numSamples)
{
//    for (int ch = 0; ch < audioBuffer->getNumChannels() && ch < bufferToFill.getNumChannels(); ++ch) {
//        auto r{audioBuffer->getReadPointer(ch)};
//        bufferToFill.copyFrom(ch, 0, r, numSamples);
//    }

//    auto readHandle{fifo.read(numSamples)};
//
//    if (readHandle.blockSize1 + readHandle.blockSize2 != numSamples) {
//        fifo.reset();
//        readHandle = fifo.read(numSamples);
//    }
//
//    // Sync test. Copy first two return channels to appropriate channels in the buffer.
//    for (int ch = 0; ch < 2 && channelToWriteTo + ch < bufferToFill.getNumChannels(); ++ch) {
//        auto r{audioBuffer->getReadPointer(ch)};
//        bufferToFill.copyFrom(channelToWriteTo + ch, 0, r, numSamples);
//
////        bufferToFill.copyFrom(channelToWriteTo + ch, 0, *audioBuffer, ch, readHandle.startIndex1, readHandle.blockSize1);
////        bufferToFill.copyFrom(channelToWriteTo + ch, 0, *audioBuffer, ch, readHandle.startIndex2, readHandle.blockSize2);
//    }

    // Copy drift return signal
    auto r{audioBuffer->getReadPointer(1)};
    bufferToFill.copyFrom(channelToWriteTo + 1, 0, r, numSamples);

//    auto drift{audioBuffer->getSample(1, 0)};

//    auto outgoingSample{bufferToFill.getSample(0, 0)},
//            incomingSample{audioBuffer->getSample(0, 0)},
//            diff = (incomingSample >= outgoingSample ? outgoingSample + 1.f : outgoingSample) - incomingSample;

    // Calculate RTT directly, rather than having to do it later in MATLAB/Python.
    for (int s{0}; s < numSamples; ++s) {
        // Get first channel outgoing sample...
        auto outgoingSample{bufferToFill.getSample(0, s)};
        auto incomingSample{audioBuffer->getSample(0, s)},
                diff = (incomingSample >= outgoingSample ? outgoingSample + 1.f : outgoingSample) - incomingSample;

        bufferToFill.setSample(channelToWriteTo, s, diff);

//        bufferToFill.setSample(channelToWriteTo + 1, s, drift);
    }
}

bool NetAudioPeer::isConnected() const
{
    return juce::Time::getMillisecondCounter() - lastReceiveTime < 1000;
}

const DatagramAudioPacket::Origin &NetAudioPeer::getOrigin() const
{
    return origin;
}
