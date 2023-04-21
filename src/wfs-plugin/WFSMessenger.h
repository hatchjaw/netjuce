//
// Created by tar on 13/11/22.
//

#pragma once

#include <juce_core/juce_core.h>
#include <juce_osc/juce_osc.h>
#include <juce_audio_processors/juce_audio_processors.h>

class WFSMessenger : public juce::OSCSender,
                     public juce::AudioProcessorValueTreeState::Listener,
                     public juce::ValueTree::Listener {
public:
    WFSMessenger();

    ~WFSMessenger() override;

    void connect();

    void parameterChanged(const juce::String &parameterID, float newValue) override;

    void valueTreePropertyChanged(juce::ValueTree &, const juce::Identifier &) override;

private:
    std::unique_ptr<juce::DatagramSocket> socket;
};
