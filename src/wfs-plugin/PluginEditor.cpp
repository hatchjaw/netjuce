#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Utils.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor &p,
                                                                 juce::AudioProcessorValueTreeState &vts,
                                                                 juce::ValueTree &vt)
        : AudioProcessorEditor(&p),
          processorRef(p),
          valueTreeState(vts),
          dynamicTree(vt),
          feels(std::make_unique<Feels>()),
          xyController(NUM_SOURCES),
          settingsComponent(std::make_unique<SettingsComponent>(vts)) {

    setLookAndFeel(feels.get());

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
        // Need to use this callback because module IDs are handled by the
        // valueTree -- it doesn't make sense to represent them as automatable
        // parameters via the APVTS.
        cb->onChange = [this, cb, i] {
            auto ip{cb->getText()};
            dynamicTree.setProperty(njwfs::Utils::getModuleIndexParamID(i), ip, nullptr);
        };
        moduleSelectors.add(cb);
    }

    //==========================================================================
    addAndMakeVisible(gainSlider);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    gainSlider.setNormalisableRange({0., 1.25, .01});
    gainSlider.setValue(1.);
    gainSlider.setTextBoxStyle(juce::Slider::TextBoxRight,
                               false,
                               gainSlider.getTextBoxWidth() * .75f,
                               gainSlider.getTextBoxHeight());

    addAndMakeVisible(gainLabel);
    gainLabel.attachToComponent(&gainSlider, true);
    gainLabel.setText("Gain", juce::dontSendNotification);

    gainAttachment = std::make_unique<SliderAttachment>(
            valueTreeState,
            njwfs::Utils::gainParamID,
            gainSlider
    );

    //==========================================================================
    addAndMakeVisible(settingsButton);
    settingsButton.setButtonText("Settings");
    settingsButton.onClick = [this] { showSettings(); };

//    updateModuleLists(processorRef.getConnectedPeers());
    auto rawPeers{dynamicTree.getProperty(njwfs::Utils::connectedPeersParamID)};
    if (rawPeers.isArray()) {
        juce::StringArray peers;
        for (const auto &rp: *rawPeers.getArray()) {
            peers.add(rp);
        }
        updateModuleLists(peers);
    }

    dynamicTree.addListener(settingsComponent.get());
    dynamicTree.sendPropertyChangeMessage(njwfs::Utils::connectedPeersParamID);

    setSize(1200, 900);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
    settingsWindow.deleteAndZero();
    dynamicTree.removeListener(settingsComponent.get());
    setLookAndFeel(nullptr);
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
    auto padding{5}, xyPadX{50}, xyPadY{100};

    xyController.setBounds(xyPadX,
                           xyPadY,
                           bounds.getWidth() - 2 * xyPadX,
                           bounds.getHeight() - 2 * xyPadY);

    auto moduleSelectorWidth{
            static_cast<int>(
                    roundf(static_cast<float>(xyController.getWidth()) / static_cast<float>(moduleSelectors.size()))
            )
    };
    for (int i{0}; i < moduleSelectors.size(); ++i) {
        moduleSelectors[i]->setBounds(
                xyController.getX() + i * moduleSelectorWidth,
                xyController.getBottom() + padding,
                moduleSelectorWidth - 1,
                30
        );
    }

    gainSlider.setBounds(xyController.getX() + 35, xyController.getY() - 25 - padding, 250, 25);

    settingsButton.setBounds(xyController.getRight() - njwfs::Utils::settingsButtonW,
                             xyController.getY() - njwfs::Utils::settingsButtonH - padding,
                             njwfs::Utils::settingsButtonW,
                             njwfs::Utils::settingsButtonH);
}

void AudioPluginAudioProcessorEditor::updateModuleLists(const juce::StringArray &peerIPs) {
    // Update the module selector lists.
    for (int j{0}; j < moduleSelectors.size(); ++j) {
        auto prop{njwfs::Utils::getModuleIndexParamID(static_cast<uint>(j))};
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

void AudioPluginAudioProcessorEditor::showSettings() {
    juce::DialogWindow::LaunchOptions options;

//    options.content.setOwned(new SettingsComponent(valueTreeState));
    options.content.setNonOwned(settingsComponent.get());

    options.content->setSize(600, 450);

    options.dialogTitle = "Settings";
    options.dialogBackgroundColour = getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId);
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = false;

//    settingsWindow.deleteAndZero();
    settingsWindow = options.launchAsync();

    if (settingsWindow != nullptr)
        settingsWindow->centreWithSize(600, 450);
}
