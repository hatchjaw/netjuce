//
// Created by Tommy Rushton on 16/02/2023.
//

#include "MainComponent.h"

MainComponent::MainComponent(const StringArray &audioFiles)
        : multiChannelSource(std::make_unique<MultiChannelAudioSource>(NUM_SOURCES)) {
    setAudioChannels(0, NUM_SOURCES);

    netServer = std::make_unique<NetAudioServer>("226.6.38.226", 18999, "192.168.10.10", 18999);
    netServer->connect();

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
}

void MainComponent::releaseResources() {
    multiChannelSource->releaseResources();
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) {
    multiChannelSource->getNextAudioBlock(bufferToFill);
    netServer->send(bufferToFill);
}
