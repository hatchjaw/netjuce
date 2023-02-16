//
// Created by Tommy Rushton on 16/02/2023.
//
#include <juce_core/juce_core.h>
#include "MainComponent.h"

int main (int argc, char* argv[])
{
    juce::ignoreUnused (argc, argv);

    DBG("Initialising app...");
    auto app{std::make_unique<MainComponent>()};
    DBG("Done.");

    return 0;
}
