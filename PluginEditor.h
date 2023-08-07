/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/oscillor/OscillorPanel.h"
#include "ui/filter/FilterPanel.h"
#include "ui/modulation/FinalModulationPanel.h"

//==============================================================================
/**
*/
class RPBasicSynthesizerAudioProcessorEditor : public juce::AudioProcessorEditor
    , public rpSynth::ui::ModulationPanel::ShowModulatorChangeListener
{
public:
    RPBasicSynthesizerAudioProcessorEditor (RPBasicSynthesizerAudioProcessor&);
    ~RPBasicSynthesizerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    //=========================================================================
    void notifyShowModulationFrom(rpSynth::audio::ModulatorBase*,rpSynth::ui::ModulationPanel*) override;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RPBasicSynthesizerAudioProcessor& audioProcessor;

    rpSynth::ui::FinalModulationPanel m_allModulationPanel;
    rpSynth::ui::FilterPanel m_filterPanel1;
    rpSynth::ui::OscillorPanel OscillorPanel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RPBasicSynthesizerAudioProcessorEditor)
};
