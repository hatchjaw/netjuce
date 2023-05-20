//
// Created by tar on 3/16/23.
//

#include "NetAudioPeer.h"

NetAudioPeer::NetAudioPeer(DatagramAudioPacket firstPacket, int bufferRedundancy) :
        origin(firstPacket.getOrigin()),
        lastReceiveTime(juce::Time::getMillisecondCounter()),
        audioBuffer(std::make_unique<juce::AudioBuffer<float>>(
                firstPacket.getNumAudioChannels(),
                firstPacket.getNumSamples() * bufferRedundancy)
        ) {
}

void NetAudioPeer::handlePacket(DatagramAudioPacket &p) {
    lastReceiveTime = juce::Time::getMillisecondCounter();
    origin = p.getOrigin();
    for (int ch = 0; ch < p.getNumAudioChannels(); ++ch) {
        auto w{audioBuffer->getWritePointer(ch)};
        converter.convertSamples(w, p.getAudioData(static_cast<uint>(ch)), p.getNumSamples());
    }
}

void NetAudioPeer::getNextAudioBlock(juce::AudioBuffer<float> &bufferToFill, int channelToWriteTo, int numSamples) {
//    for (int ch = 0; ch < audioBuffer->getNumChannels() && ch < bufferToFill.getNumChannels(); ++ch) {
//        auto r{audioBuffer->getReadPointer(ch)};
//        bufferToFill.copyFrom(ch, 0, r, numSamples);
//    }

    // Sync test. Copy first two return channels to appropriate channels in the buffer.
    for (int ch = 0; ch < 2 && channelToWriteTo + ch < bufferToFill.getNumChannels(); ++ch) {
        auto r{audioBuffer->getReadPointer(ch)};
        bufferToFill.copyFrom(channelToWriteTo + ch, 0, r, numSamples);
    }

//    // Or calculate the difference directly, rather than having to do it later
//    // in MATLAB/Python.
//    // Doesn't work, because this overwrites the first channel when handling
//    // the first peer.
//    for (int s{0}; s < numSamples; ++s) {
//        auto outgoingSample{bufferToFill.getSample(0, s)},
//                incomingSample{audioBuffer->getSample(0, s)},
//                diff = (incomingSample >= outgoingSample ? outgoingSample + 1.f : outgoingSample) - incomingSample;
//        bufferToFill.setSample(channelToWriteTo, s, diff);
//    }
//
//    auto r{audioBuffer->getReadPointer(1)};
//    bufferToFill.copyFrom(channelToWriteTo + 1, 0, r, numSamples);
}

bool NetAudioPeer::isConnected() const {
    return juce::Time::getMillisecondCounter() - lastReceiveTime < 1000;
}

const DatagramAudioPacket::Origin &NetAudioPeer::getOrigin() const {
    return origin;
}
