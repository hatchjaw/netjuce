#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p,
                                                                 std::shared_ptr<juce::ValueTree> vt)
        : AudioProcessorEditor(&p),
          processorRef(p),
          xyController(NUM_SOURCES),
          valueTree(vt) {
    juce::ignoreUnused(processorRef);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(1000, 800);

    addAndMakeVisible(xyController);
    for (int i{0}; i < NUM_SOURCES; ++i) {
        xyController.addNode({valueTree->getProperty("/source/" + juce::String{i} + "/x"),
                              valueTree->getProperty("/source/" + juce::String{i} + "/y")});
    }
    xyController.onValueChange = [this](uint nodeIndex, juce::Point<float> position) {
        valueTree->setProperty("/source/" + juce::String{nodeIndex} + "/x", position.x, nullptr);
        valueTree->setProperty("/source/" + juce::String{nodeIndex} + "/y", position.y, nullptr);
    };
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized() {
    auto bounds{getLocalBounds()};
    auto padding{5}, xyPadX{75}, xyPadY{100};
    xyController.setBounds(xyPadX, xyPadY / 2, bounds.getWidth() - 2 * xyPadX, bounds.getHeight() - 2 * xyPadY);
}
