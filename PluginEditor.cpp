/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RPBasicSynthesizerAudioProcessorEditor::RPBasicSynthesizerAudioProcessorEditor (RPBasicSynthesizerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    m_noteOn.setButtonText("note on");
    addAndMakeVisible(m_noteOn);

    m_noteoff.setButtonText("note off");
    addAndMakeVisible(m_noteoff);

    m_velocity.setRange(0.f, 1.f, 0.1f);
    addAndMakeVisible(m_velocity);

    m_noteNumber.setRange(0.f, 127.f, 1.f);
    addAndMakeVisible(m_noteNumber);
}

RPBasicSynthesizerAudioProcessorEditor::~RPBasicSynthesizerAudioProcessorEditor()
{
}

//==============================================================================
void RPBasicSynthesizerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)

}

void RPBasicSynthesizerAudioProcessorEditor::resized()
{
}
