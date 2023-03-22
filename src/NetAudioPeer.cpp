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

void NetAudioPeer::getNextAudioBlock(juce::AudioBuffer<float> bufferToFill, int numSamples) {
    for (int ch = 0; ch < audioBuffer->getNumChannels() && ch < bufferToFill.getNumChannels(); ++ch) {
        auto r{audioBuffer->getReadPointer(ch)};
        bufferToFill.copyFrom(ch, 0, r, numSamples);
    }
}

bool NetAudioPeer::isConnected() const {
    return juce::Time::getMillisecondCounter() - lastReceiveTime < 1000;
}

const DatagramAudioPacket::Origin &NetAudioPeer::getOrigin() const {
    return origin;
}
