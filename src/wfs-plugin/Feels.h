//
// Created by tar on 21/04/23.
//

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../../lib/succulent/ui/XYController/XYController.h"

class Feels : public juce::LookAndFeel_V4 {
public:
    Feels();

    ~Feels() override = default;

    void drawComboBox(juce::Graphics &g, int width, int height, bool isButtonDown, int buttonX, int buttonY,
                      int buttonW, int buttonH, juce::ComboBox &box) override;

    void positionComboBoxText(juce::ComboBox &box, juce::Label &label) override;

private:
    static constexpr int kArrowWidth{21};
};
