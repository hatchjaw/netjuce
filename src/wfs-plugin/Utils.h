//
// Created by tar on 20/04/23.
//

#pragma once

#include <juce_core/juce_core.h>

namespace njwfs {
    enum class SourcePositionAxis : char {
        X = 'x',
        Y = 'y'
    };

    class Utils {
    public:
        /**
         * Get an authoritative parameter-ID/OSC-path for a sound source index
         * and positional axis.
         * @param index
         * @param axis
         * @return
         */
        static juce::String getSourcePositionParamID(uint index, SourcePositionAxis axis) {
            return "/source/" + juce::String{index} + "/" + static_cast<char>(axis);
        }

        static juce::String getModuleParamID(uint index) {
            return "/module/" + juce::String{index};
        }

        static const juce::StringRef speakerSpacingParamID;
    };
}