//
// Created by tar on 21/04/23.
//

#ifndef NETJUCE_SIDEBAR_H
#define NETJUCE_SIDEBAR_H

#include <juce_gui_basics/juce_gui_basics.h>

class Sidebar : public juce::Component {
public:
    Sidebar();

    ~Sidebar() override;

    void paint(juce::Graphics &) override;

    void resized() override;
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Sidebar)

    juce::Label listOfClients;
};


#endif //NETJUCE_SIDEBAR_H
