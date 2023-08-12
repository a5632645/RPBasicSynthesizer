/*
  ==============================================================================

    EffectProcessorControlPanel.h
    Created: 7 Aug 2023 8:55:53pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace rpSynth::audio::effects {
class EffectProcessorBase;
}

namespace rpSynth::ui {
class ContainModulableComponent;

class EffectProcessorControlPanel : public juce::Button {
public:
    EffectProcessorControlPanel(audio::effects::EffectProcessorBase&);
    ~EffectProcessorControlPanel() override;

    void resized() override;
    void paintButton(juce::Graphics& g,
                     bool shouldDrawButtonAsHighlighted,
                     bool shouldDrawButtonAsDown);

    ContainModulableComponent* getEffectControlPanel() const;
    const juce::String& getEffectName() const;
    int getIndex() const { return m_panelIndex; }
    void setIndex(int i) { m_panelIndex = i; }

private:
    audio::effects::EffectProcessorBase& m_controlledEffect;
    int m_panelIndex = 0;

    //================================================================================
    // some component
    juce::ToggleButton m_buttonBypass;
    juce::Label m_labelEffectName;
    std::unique_ptr<ContainModulableComponent> m_effectPanel;
    juce::ButtonParameterAttachment m_attach;
    //================================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectProcessorControlPanel)
};
};