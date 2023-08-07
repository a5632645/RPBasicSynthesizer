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
    , m_filterPanel1(audioProcessor.m_synthesizer.m_filter)
    , OscillorPanel(audioProcessor.m_synthesizer.m_polyOscillor) {
    // init modulation panel
    addAndMakeVisible(m_allModulationPanel);
    m_allModulationPanel.setTopMostComponent(this);

    // init oscillor panel here
    addAndMakeVisible(OscillorPanel);

    // init filter panel
    addAndMakeVisible(m_filterPanel1);

    // init window
    setSize(800, 600);
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
    OscillorPanel.setBoundsRelative(0.f, 0.f, 0.3f, 0.3f);
    m_filterPanel1.setBoundsRelative(0.f, 0.3f, 0.4f, 0.3f);
    m_allModulationPanel.setBoundsRelative(0.f, 0.6f, 1.f, 0.4f);
}

void RPBasicSynthesizerAudioProcessorEditor::notifyShowModulationFrom(rpSynth::audio::ModulatorBase* p, rpSynth::ui::ModulationPanel*) {
    m_allModulationPanel.showModulationFrom(p);
    OscillorPanel.showModulationFrom(p);
    m_filterPanel1.showModulationFrom(p);
}