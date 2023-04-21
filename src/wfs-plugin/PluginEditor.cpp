#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Utils.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p,
                                                                 juce::AudioProcessorValueTreeState &vts,
                                                                 juce::ValueTree &vt)
        : AudioProcessorEditor(&p),
          processorRef(p),
          xyController(NUM_SOURCES),
          valueTreeState(vts),
          dynamicTree(vt) {

    juce::ignoreUnused(processorRef);

    getLookAndFeel().setColour(juce::ResizableWindow::backgroundColourId, juce::Colours::ghostwhite);
    auto fg = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).contrasting(.75);

    xyController.onAddNode = [this](XYController::Node &n) {
        auto idx{n.getIndex()};
        xyNodeAttachments.push_back(std::make_unique<XYControllerNodeAttachment>(
                valueTreeState,
                njwfs::Utils::getSourcePositionParamID(idx, njwfs::SourcePositionAxis::X),
                njwfs::Utils::getSourcePositionParamID(idx, njwfs::SourcePositionAxis::Y),
                n
        ));
    };
    addAndMakeVisible(xyController);
    for (int i{0}; i < NUM_SOURCES; ++i) {
        xyController.addNode();
    }

    for (uint i = 0; i < NUM_MODULES; ++i) {
        auto cb{new juce::ComboBox};
        addAndMakeVisible(cb);
        cb->setColour(juce::ComboBox::textColourId, fg);
        cb->setColour(juce::ComboBox::arrowColourId, fg);
        cb->setColour(juce::ComboBox::backgroundColourId, juce::Colours::whitesmoke);
        cb->onChange = [this, cb, i] {
            auto ip{cb->getText()};
            dynamicTree.setProperty(njwfs::Utils::getModuleParamID(i), ip, nullptr);
        };
        moduleSelectors.add(cb);
    }

    updateModuleLists(processorRef.getConnectedPeers());

    setSize(1200, 900);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics &g) {
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    juce::ColourGradient cg = juce::ColourGradient::vertical(
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId),
            getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId).darker(.2f),
            getBounds()
    );
    g.setGradientFill(cg);
    g.fillAll();
}

void AudioPluginAudioProcessorEditor::resized() {
    auto bounds{getLocalBounds()};
    auto padding{5}, xyPadX{75}, xyPadY{100};
    xyController.setBounds(xyPadX, xyPadY / 2, bounds.getWidth() - 2 * xyPadX, bounds.getHeight() - 2 * xyPadY);

    auto moduleSelectorWidth{
            static_cast<int>(
                    roundf(static_cast<float>(xyController.getWidth() + 1.5) / static_cast<float>(NUM_MODULES))
            )
    };
    for (int i{0}; i < moduleSelectors.size(); ++i) {
        moduleSelectors[i]->setBounds(
                xyPadX + i * moduleSelectorWidth,
                xyController.getBottom() + padding,
                moduleSelectorWidth - 1,
                30
        );
    }
}

void AudioPluginAudioProcessorEditor::updateModuleLists(const juce::StringArray &peerIPs) {
    // Update the module selector lists.
    for (int j{0}; j < moduleSelectors.size(); ++j) {
        auto prop{njwfs::Utils::getModuleParamID(static_cast<uint>(j))};
        // Store the current property value.
        auto selected{dynamicTree.getProperty(prop).toString()};
        // Refresh the list.
        moduleSelectors[j]->clear(juce::dontSendNotification);
        moduleSelectors[j]->addItemList(peerIPs, 1);
        // Try to set the stored value.
        for (int i{0}; i < peerIPs.size(); ++i) {
            if (peerIPs[i] == selected) { //text) {
                moduleSelectors[j]->setSelectedItemIndex(i);
            }
        }
    }
}
