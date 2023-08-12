/*
  ==============================================================================

    SynthesizerPanel.cpp
    Created: 8 Aug 2023 9:17:50am
    Author:  mana

  ==============================================================================
*/

#include "SynthesizerPanel.h"
#include "synthesizer/BasicSynthesizer.h"

namespace rpSynth::ui {
SynthesizerPanel::SynthesizerPanel(audio::BasicSynthesizer& s)
    : juce::TabbedComponent(juce::TabbedButtonBar::TabsAtTop) {
    m_filterAndOscillorPanel = std::make_unique<FilterAndOscillorPanel>(s);
    m_fxChainPanel = std::make_unique<FinalEffectChainPanel>(s.m_fxChain);

    addTab("OSC", juce::Colours::grey, m_filterAndOscillorPanel.get(), false);
    addTab("FX", juce::Colours::grey, m_fxChainPanel.get(), false);
}

SynthesizerPanel::~SynthesizerPanel() {
    m_filterAndOscillorPanel = nullptr;
    m_fxChainPanel = nullptr;
}

void SynthesizerPanel::showModulationFrom(audio::ModulatorBase* p) {
    m_filterAndOscillorPanel->showModulationFrom(p);
    m_fxChainPanel->showModulationFrom(p);
}
}