//
// Created by Tommy Rushton on 16/02/2023.
//

#include "MainComponent.h"

MainComponent::MainComponent(const StringArray &audioFilesToPlay)
        : multiChannelSource(std::make_unique<MultiChannelAudioSource>(NUM_SOURCES)),
          netServer(std::make_unique<NetAudioServer>()),
          audioFiles(audioFilesToPlay) {
    setAudioChannels(0, NUM_SOURCES);
    auto setup{deviceManager.getAudioDeviceSetup()};
    setup.bufferSize = AUDIO_BLOCK_SAMPLES;
//    setup.useDefaultOutputChannels = false;
//    setup.outputChannels = NUM_SOURCES;
    deviceManager.setAudioDeviceSetup(setup, true);

    for (auto i{0}; i < audioFiles.size(); ++i) {
        auto file{File(audioFiles[i])};
        if (file.existsAsFile()) {
            multiChannelSource->addSource(static_cast<uint>(i), file);
            multiChannelSource->start();
        }
    }
}

MainComponent::~MainComponent() {
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) {
    multiChannelSource->prepareToPlay(samplesPerBlockExpected, sampleRate);
    netServer->prepareToSend(samplesPerBlockExpected, sampleRate);
}

void MainComponent::releaseResources() {
    multiChannelSource->releaseResources();
    netServer->releaseResources();
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) {
    multiChannelSource->getNextAudioBlock(bufferToFill);
    if (!netServer->send(bufferToFill)) {
        DBG("Failed to send UDP packet.");
        netServer->disconnect();
    }
}
