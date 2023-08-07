/*
  ==============================================================================

    NumericSlider.cpp
    Created: 28 Jul 2023 9:31:51pm
    Author:  mana

  ==============================================================================
*/

#include <JuceHeader.h>
#include "NumericSlider.h"

class NumericSliderLookAndFeel : public juce::LookAndFeel_V4 {
public:
    ~NumericSliderLookAndFeel() override = default;
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float /*sliderPos*/, float /*minSliderPos*/, float /*maxSliderPos*/,
                          const juce::Slider::SliderStyle, juce::Slider& s) override {
        g.setColour(juce::Colours::black);
        g.drawText(s.getTextFromValue(s.getValue()),
                   x, y, width, height,
                   juce::Justification::centred);
    }
};

static NumericSliderLookAndFeel g_numericLookAndFeel;

namespace rpSynth::ui {
NumericSlider::NumericSlider()
    :juce::Slider(juce::Slider::SliderStyle::LinearBarVertical,
                  juce::Slider::TextEntryBoxPosition::NoTextBox) {
    setLookAndFeel(&g_numericLookAndFeel);
    setVelocityBasedMode(true);
}

NumericSlider::~NumericSlider() {
    setLookAndFeel(nullptr);
}
}