/*
  ==============================================================================

    FinalEffectChainPanel.h
    Created: 7 Aug 2023 8:55:29pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ui/ContainModulableComponent.h"

namespace rpSynth::audio {
class OrderableEffectsChain;
class ModulatorBase;
}

namespace rpSynth::ui {
class EffectProcessorControlPanel;

class FinalEffectChainPanel : public ContainModulableComponent
, juce::Button::Listener {
public:
    FinalEffectChainPanel(audio::OrderableEffectsChain&);
    ~FinalEffectChainPanel() override;

    void resized() override;
    void paint(juce::Graphics&) override;

    //================================================================================
    // implement for juce::Button::Listener
    void buttonClicked(juce::Button*) override;
    //================================================================================

    //================================================================================
    // implement for juce::MouseListener
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseDown(const juce::MouseEvent& e) override;
    //================================================================================

    //================================================================================
    // implement for ContainModulableComponent
    void showModulationFrom(audio::ModulatorBase*);
    //================================================================================
private:
    void onChainOrderChanged();

    int m_draggedComponentOldIndex = 0;
    juce::ComponentDragger m_dragger;

    audio::OrderableEffectsChain& m_chain;
    std::vector<std::shared_ptr<EffectProcessorControlPanel>> m_controllers;
    std::unique_ptr<juce::Component> m_showingContent;
    ContainModulableComponent* m_currentShowingEffectComponent = nullptr;
    audio::ModulatorBase* m_currentShowingModulator = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FinalEffectChainPanel)
};
}