/*
  ==============================================================================

    OscillorPanel.cpp
    Created: 28 Jul 2023 10:32:05pm
    Author:  mana

  ==============================================================================
*/

#include <JuceHeader.h>
#include "OscillorPanel.h"

namespace rpSynth::ui {
OscillorPanel::OscillorPanel(audio::PolyOscillor& osc)
    :m_knob_semitone(&osc.m_semitone)
    , m_knob_volumeLevel(&osc.m_volumeLevel) {
    addAndMakeVisible(m_knob_semitone);
    addAndMakeVisible(m_knob_volumeLevel);
}

OscillorPanel::~OscillorPanel() {
}

void OscillorPanel::resized() {
    m_knob_semitone.setBoundsRelative(0.f, 0.f, 0.25f, 0.5f);
    m_knob_volumeLevel.setBoundsRelative(0.3f, 0.f, 0.25f, 0.5f);
}

void OscillorPanel::paint(juce::Graphics& g) {
}

void OscillorPanel::showModulationFrom(audio::ModulatorBase* p) {
    m_knob_semitone.showModulationFrom(p);
    m_knob_volumeLevel.showModulationFrom(p);
}
}