#pragma once

#include "PluginProcessor.h"
#include "../../lib/succulent/ui/XYController/XYController.h"

//==============================================================================
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
    AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &, juce::ValueTree &);

    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics &) override;

    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor &processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)

    XYController xyController;

    juce::ValueTree &valueTree;
};
