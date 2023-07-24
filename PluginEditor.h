/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class RPBasicSynthesizerAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    RPBasicSynthesizerAudioProcessorEditor (RPBasicSynthesizerAudioProcessor&);
    ~RPBasicSynthesizerAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    RPBasicSynthesizerAudioProcessor& audioProcessor;

    juce::TextButton m_noteOn;
    juce::TextButton m_noteoff;
    juce::Slider m_noteNumber;
    juce::Slider m_velocity;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RPBasicSynthesizerAudioProcessorEditor)
};
