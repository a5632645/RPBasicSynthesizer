/*
  ==============================================================================

    OscillorPanel.h
    Created: 28 Jul 2023 10:32:05pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "synthesizer/Oscillor/PolyOscillor.h"
#include "ui/controller/FloatKnob.h"
#include "ui/ContainModulableComponent.h"

namespace rpSynth::ui {
class OscillorPanel : public ContainModulableComponent {
public:
    OscillorPanel(audio::PolyOscillor&);
    ~OscillorPanel() override;

    void resized() override;
    void paint(juce::Graphics&) override;

    //=========================================================================
    void showModulationFrom(audio::ModulatorBase*) override;
private:
    FloatKnob m_knob_semitone;
    FloatKnob m_knob_volumeLevel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OscillorPanel)
};
}