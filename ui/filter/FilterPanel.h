/*
  ==============================================================================

    FilterPanel.h
    Created: 5 Aug 2023 10:32:52pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "ui/ContainModulableComponent.h"
#include "ui/controller/FloatKnob.h"

namespace rpSynth::audio {
class MainFilter;
}

namespace rpSynth::ui {
class FilterKnobsPanel : public ContainModulableComponent {
public:
    //=========================================================================
    // common component method
    ~FilterKnobsPanel() override = default;
    FilterKnobsPanel(audio::MainFilter&);
    void resized() override;
    //=========================================================================

    //=========================================================================
    // layout
    void doLayout(const juce::String& filterName);
    //=========================================================================

    //=========================================================================
    // implement for ContainModulableComponent
    void showModulationFrom(audio::ModulatorBase*) override;
    //=========================================================================

    //=========================================================================
    // parameter controllers
    FloatKnob m_cutoff;
    FloatKnob m_resonance;
    FloatKnob m_phase;
    FloatKnob m_feedback;
    //=========================================================================
private:
    void setAllKnobInvisitable();

    audio::MainFilter& m_filter;
};

class FilterPanel : public ContainModulableComponent {
public:
    //=========================================================================
    // common component method
    FilterPanel(audio::MainFilter&);
    ~FilterPanel() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    //=========================================================================

    //=========================================================================
    // implement for ContainModulableComponent
    void showModulationFrom(audio::ModulatorBase*) override;
    //=========================================================================
private:
    audio::MainFilter& m_filter;
    //=========================================================================
    // switch filter types
    juce::TextButton m_buttonShowFilterName;
    juce::PopupMenu m_popMenuSelectFilter;
    juce::StringArray m_allFilterNames;
    //=========================================================================

    //=========================================================================
    // filter audio input router
    class FilterRouterPanel;
    std::unique_ptr<FilterRouterPanel> m_filterRouterPanel;
    //=========================================================================

    //=========================================================================
    // filter controller layout
    FilterKnobsPanel m_filterKnobsPanel;
    //=========================================================================
};
}