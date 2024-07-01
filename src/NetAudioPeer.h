//
// Created by tar on 3/16/23.
//

#ifndef NETJUCE_NETAUDIOPEER_H
#define NETJUCE_NETAUDIOPEER_H

#include <juce_core/juce_core.h>
#include "DatagramAudioPacket.h"
#include "AudioToNetFifo.h"

using ConverterI16F32 = juce::AudioData::ConverterInstance<
        juce::AudioData::Pointer<
                juce::AudioData::Int16,
                juce::AudioData::LittleEndian,
                juce::AudioData::NonInterleaved,
                juce::AudioData::Const
        >,
        juce::AudioData::Pointer<
                juce::AudioData::Float32,
                juce::AudioData::NativeEndian,
                juce::AudioData::NonInterleaved,
                juce::AudioData::NonConst
        >
>;

class NetAudioPeer {
public:
    explicit NetAudioPeer(DatagramAudioPacket firstPacket, int bufferRedundancy = 2);

    void handlePacket(DatagramAudioPacket &p);

    void getNextAudioBlock(juce::AudioBuffer<float> &bufferToFill, int channelToWriteTo, int numSamples);

    bool isConnected() const;

    const DatagramAudioPacket::Origin &getOrigin() const;

private:
    DatagramAudioPacket::Origin origin;
    uint32_t lastReceiveTime;
    std::unique_ptr<juce::AudioBuffer<float>> audioBuffer;
    std::unique_ptr<juce::AudioBuffer<float>> tempBuffer;
    juce::AbstractFifo fifo;
    ConverterI16F32 converter;
};


#endif //NETJUCE_NETAUDIOPEER_H
