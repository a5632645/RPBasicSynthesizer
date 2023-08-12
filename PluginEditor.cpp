/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
RPBasicSynthesizerAudioProcessorEditor::RPBasicSynthesizerAudioProcessorEditor(RPBasicSynthesizerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
    , m_allModulationPanel(audioProcessor.m_synthesizer, this)
    , m_sysnthesizerPanel(audioProcessor.m_synthesizer) {
    // init modulation panel
    addAndMakeVisible(m_allModulationPanel);
    m_allModulationPanel.setTopMostComponent(this);

    addAndMakeVisible(m_sysnthesizerPanel);

    // init window
    setSize(600, 480);
    setResizable(true, true);

    // final init
    m_allModulationPanel.init();
}

RPBasicSynthesizerAudioProcessorEditor::~RPBasicSynthesizerAudioProcessorEditor() {
    setLookAndFeel(nullptr);
}

//==============================================================================
void RPBasicSynthesizerAudioProcessorEditor::paint(juce::Graphics& /*g*/) {
}

void RPBasicSynthesizerAudioProcessorEditor::resized() {
    m_allModulationPanel.setBoundsRelative(0.f, 0.6f, 1.f, 0.4f);
    m_sysnthesizerPanel.setBoundsRelative(0.f, 0.f, 1.f, 0.6f);
}

void RPBasicSynthesizerAudioProcessorEditor::notifyShowModulationFrom(rpSynth::audio::ModulatorBase* p, rpSynth::ui::ModulationPanel*) {
    m_sysnthesizerPanel.showModulationFrom(p);
    m_allModulationPanel.showModulationFrom(p);
}