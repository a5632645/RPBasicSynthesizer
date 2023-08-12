/*
  ==============================================================================

    EffectProcessorControlPanel.cpp
    Created: 7 Aug 2023 8:55:53pm
    Author:  mana

  ==============================================================================
*/

#include <JuceHeader.h>
#include "EffectProcessorControlPanel.h"

#include "synthesizer/Effects/EffectProcessorBase.h"

namespace rpSynth::ui {
EffectProcessorControlPanel::EffectProcessorControlPanel(audio::effects::EffectProcessorBase& e)
    : juce::Button(e.getProcessorID())
    , m_controlledEffect(e)
    , m_attach(*e.notBypass, m_buttonBypass) {
    addAndMakeVisible(m_labelEffectName);
    addAndMakeVisible(m_buttonBypass);

    // init
    m_labelEffectName.setText(e.getEffectName(), juce::dontSendNotification);
    m_labelEffectName.setJustificationType(juce::Justification::centred);
    m_labelEffectName.setInterceptsMouseClicks(false, false);

    m_effectPanel = e.createEffectPanel();
}

EffectProcessorControlPanel::~EffectProcessorControlPanel() {
    m_effectPanel = nullptr;
}

void EffectProcessorControlPanel::resized() {
    m_buttonBypass.setBounds(5, 5, 30, 30);
    m_labelEffectName.setBounds(getLocalBounds());
}

void EffectProcessorControlPanel::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) {
    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds());
}

ContainModulableComponent* EffectProcessorControlPanel::getEffectControlPanel() const {
    return m_effectPanel.get();
}

const juce::String& EffectProcessorControlPanel::getEffectName() const {
    return m_controlledEffect.getEffectName();
}
};