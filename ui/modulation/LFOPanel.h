/*
  ==============================================================================

    LFOPanel.h
    Created: 27 Jul 2023 9:29:14pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ui/ContainModulableComponent.h"
#include "LineGeneratorPanel.h"
#include "ui/controller/FloatKnob.h"

namespace rpSynth {
namespace audio {
class LFO;
}

namespace ui {
class LFOPanel : public ContainModulableComponent {
public:
    LFOPanel(audio::LFO& lfo);
    ~LFOPanel() override;

    void resized() override;
    void paint(juce::Graphics& g) override;

    void showModulationFrom(audio::ModulatorBase*) override;
private:
    class LFOPointer;

    audio::LFO& m_LFOBind;
    LineGeneratorPanel m_lineGeneratorPanel;
    std::unique_ptr<LFOPointer> m_lfoPointer;

    FloatKnob m_LFOFrequency;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LFOPanel)
};
};
};