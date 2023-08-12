/*
  ==============================================================================

    SynthesizerPanel.h
    Created: 8 Aug 2023 9:17:50am
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "ui/FilterAndOscillorPanel.h"
#include "ui/effects/FinalEffectChainPanel.h"

namespace rpSynth::audio {
class ModulatorBase;
};

namespace rpSynth::ui {
class SynthesizerPanel : public juce::TabbedComponent {
public:
    SynthesizerPanel(audio::BasicSynthesizer&);
    ~SynthesizerPanel() override;

    void showModulationFrom(audio::ModulatorBase*);
private:
    std::unique_ptr<FilterAndOscillorPanel> m_filterAndOscillorPanel;
    std::unique_ptr<FinalEffectChainPanel> m_fxChainPanel;
};
}