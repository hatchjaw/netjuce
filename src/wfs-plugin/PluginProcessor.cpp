#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Utils.h"

/**
 * TODO: try to make sure there's only ever one instance if this plugin.
 * Otherwise things'll be messy at best, what with multiple NetAudioServer
 * instances trying to access the network on the same IP and port.
 */
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
        : AudioProcessor(getBusesProperties()),
          server(std::make_unique<NetAudioServer>()),
          wfsMessenger(std::make_unique<WFSMessenger>()),
          apvts(*this, nullptr, "WFS Parameters", createParameterLayout()),
          dynamicTree("Module Parameters") {

    // Add the OSC messenger as a listener to the source positions...
    for (uint i{0}; i < NUM_SOURCES; ++i) {
        apvts.addParameterListener(njwfs::Utils::getSourcePositionParamID(i, njwfs::SourcePositionAxis::X),
                                   wfsMessenger.get());
        apvts.addParameterListener(njwfs::Utils::getSourcePositionParamID(i, njwfs::SourcePositionAxis::Y),
                                   wfsMessenger.get());
    }
    // ...and speaker spacing.
    apvts.addParameterListener(njwfs::Utils::speakerSpacingParamID, wfsMessenger.get());
    // Also instruct the messenger to listen to the dynamic tree (which handles
    // module IDs and such).
//    apvts.state.addListener(wfsMessenger.get());
    dynamicTree.addListener(wfsMessenger.get());

    // Notify new peers of parameters.
    server->onPeerConnected = [this]() {
        for (uint i{0}; i < NUM_SOURCES; ++i) {
            auto idX{njwfs::Utils::getSourcePositionParamID(i, njwfs::SourcePositionAxis::X)},
                    idY{njwfs::Utils::getSourcePositionParamID(i, njwfs::SourcePositionAxis::Y)};
            wfsMessenger->parameterChanged(idX, apvts.getRawParameterValue(idX)->load());
            wfsMessenger->parameterChanged(idY, apvts.getRawParameterValue(idY)->load());
        }
        wfsMessenger->parameterChanged(
                njwfs::Utils::speakerSpacingParamID,
                apvts.getRawParameterValue(njwfs::Utils::speakerSpacingParamID)->load()
        );
    };

    // Keep module IDs up to date.
    server->onPeersChanged = [this](juce::StringArray &peerIPs) {
        // Remind the modules of their positions.
        for (int j{0}; j < NUM_MODULES; ++j) {
            auto prop{njwfs::Utils::getModuleIndexParamID(static_cast<uint>(j))};
            dynamicTree.sendPropertyChangeMessage(prop);
        }

        // Update the editor, if there is one.
        // TODO: This is bad; maybe make the editor a ValueTree::Listener.
        if (auto *editor = dynamic_cast<AudioPluginAudioProcessorEditor *>(getActiveEditor())) {
            editor->updateModuleLists(peerIPs);
        }

        dynamicTree.setProperty(njwfs::Utils::connectedPeersParamID, peerIPs, nullptr);
    };
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
    dynamicTree.removeListener(wfsMessenger.get());
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const {
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram() {
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String &newName) {
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    server->prepareToSend(samplesPerBlock, sampleRate);
    wfsMessenger->connect();
}

void AudioPluginAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    server->releaseResources();
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
    // Just approve any channel layout.
    return true;

#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                             juce::MidiBuffer &midiMessages) {
    juce::ignoreUnused(midiMessages);

    juce::ScopedNoDenormals noDenormals;
//    auto totalNumInputChannels = getTotalNumInputChannels();
//    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
//    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
//        buffer.clear(i, 0, buffer.getNumSamples());

    // Apply master gain to the buffer.
    float gain{apvts.getRawParameterValue(njwfs::Utils::gainParamID)->load()};
    buffer.applyGainRamp(0, buffer.getNumSamples(), lastGain, gain);
    lastGain = gain;

    juce::AudioSourceChannelInfo block{buffer};
    // Pass the buffer to the networked audio server.
    server->handleAudioBlock(block);

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
//    for (int channel = 0; channel < totalNumInputChannels; ++channel) {
//        auto *channelData = buffer.getWritePointer(channel);
//        juce::ignoreUnused(channelData);
//        // ...do something to the data...
//    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *AudioPluginAudioProcessor::createEditor() {
    return new AudioPluginAudioProcessorEditor(*this, apvts, dynamicTree);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused(destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused(data, sizeInBytes);
}

juce::AudioProcessor::BusesProperties AudioPluginAudioProcessor::getBusesProperties() {
    BusesProperties buses;

//    for (int i{0}; i < NUM_SOURCES; ++i) {
//        buses.addBus(true, "Input #" + juce::String{i + 1}, juce::AudioChannelSet::mono());
//    }

    buses.addBus(true, "Input", juce::AudioChannelSet::octagonal());

    return buses;
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout params;

    params.add(std::make_unique<juce::AudioParameterFloat>(
            njwfs::Utils::speakerSpacingParamID,
            "Speaker Spacing (m)",
            juce::NormalisableRange<float>{.05f, .5f, .001f},
            .2f
    ));

    for (uint i{0}; i < NUM_SOURCES; ++i) {
        params.add(std::make_unique<juce::AudioParameterFloat>(
                njwfs::Utils::getSourcePositionParamID(i, njwfs::SourcePositionAxis::X),
                "Source " + juce::String{i + 1} + " X",
                juce::NormalisableRange<float>{0.f, 1.f, 1e-6},
                (static_cast<float>(i) + .5f) / NUM_SOURCES
        ));
        params.add(std::make_unique<juce::AudioParameterFloat>(
                njwfs::Utils::getSourcePositionParamID(i, njwfs::SourcePositionAxis::Y),
                "Source " + juce::String{i + 1} + " Y",
                juce::NormalisableRange<float>{0.f, 1.f, 1e-6},
                .5f
        ));
    }

    params.add(std::make_unique<juce::AudioParameterFloat>(
            njwfs::Utils::gainParamID,
            "Master Gain",
            juce::NormalisableRange<float>{0.f, 1.25f, .01f},
            1.f
    ));

    return params;
}

// TODO: get rid of this
const juce::StringArray &AudioPluginAudioProcessor::getConnectedPeers() {
    return server->getConnectedPeers();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new AudioPluginAudioProcessor();
}
