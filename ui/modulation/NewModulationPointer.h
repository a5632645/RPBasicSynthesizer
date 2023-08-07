/*
  ==============================================================================

    NewModulationPointer.h
    Created: 30 Jul 2023 10:15:02pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace rpSynth::ui {
class NewModulationPointer : public juce::Component {
public:
    NewModulationPointer() = default;
    ~NewModulationPointer() override = default;

    void paint(juce::Graphics& g) override {
        g.setColour(juce::Colours::blueviolet);
        g.drawLine(0.f, getHeight() / 2.f, (float)getWidth(), getHeight() / 2.f, 4.f);
        g.drawLine(getWidth() / 2.f, 0.f, getWidth() / 2.f, (float)getHeight(), 4.f);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewModulationPointer);
};
}