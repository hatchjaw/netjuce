//
// Created by Tommy Rushton on 16/02/2023.
//

#ifndef NETJUCE_MAINCOMPONENT_H
#define NETJUCE_MAINCOMPONENT_H

#include <juce_audio_utils/juce_audio_utils.h>
#include "Constants.h"
#include "../lib/succulent/MultiChannelAudioSource/MultiChannelAudioSource.h"

class MainComponent : public juce::AudioAppComponent {
public:
    MainComponent();

    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    void releaseResources() override;

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;

private:
    std::unique_ptr<MultiChannelAudioSource> multiChannelSource;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


#endif //NETJUCE_MAINCOMPONENT_H
