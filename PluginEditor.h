/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "ui/modulation/FinalModulationPanel.h"
#include "ui/SynthesizerPanel.h"

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
    rpSynth::ui::SynthesizerPanel m_sysnthesizerPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RPBasicSynthesizerAudioProcessorEditor)
};
