/*
  ==============================================================================

    NumericSlider.h
    Created: 28 Jul 2023 9:31:51pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace rpSynth::ui {
class NumericSlider : public juce::Slider {
public:
    NumericSlider();
    ~NumericSlider() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NumericSlider)
};
}
