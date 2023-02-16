//
// Created by Tommy Rushton on 16/02/2023.
//

#include "MainComponent.h"

MainComponent::MainComponent()
        : multiChannelSource(std::make_unique<MultiChannelAudioSource>(NUM_SOURCES)) {}

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
}
