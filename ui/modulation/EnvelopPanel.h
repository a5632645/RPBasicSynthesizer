/*
  ==============================================================================

    EnvelopPanel.h
    Created: 27 Jul 2023 9:29:26pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ui/ContainModulableComponent.h"
#include "ui/controller/FloatKnob.h"

namespace rpSynth::audio {
class Envelop;
}

namespace rpSynth::ui {
class EnvelopPanel : public ContainModulableComponent {
public:
    EnvelopPanel(audio::Envelop&);
    ~EnvelopPanel() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    // implement for ContainModulableComponent
    void showModulationFrom(audio::ModulatorBase*) override;
private:
    class EnvelopDraw;
    class EnvelopPointer;
    std::unique_ptr<EnvelopDraw> m_envelopDrawer;
    std::unique_ptr<EnvelopPointer> m_envelopPointer;

    audio::Envelop& m_bindEnvelopModulator;
    FloatKnob m_attack;
    FloatKnob m_hold;
    FloatKnob m_decay;
    FloatKnob m_sustain;
    FloatKnob m_release;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EnvelopPanel)
};
}