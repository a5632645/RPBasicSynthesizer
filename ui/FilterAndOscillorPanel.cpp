/*
  ==============================================================================

    FilterAndOscillorPanel.cpp
    Created: 8 Aug 2023 9:10:44am
    Author:  mana

  ==============================================================================
*/

#include "FilterAndOscillorPanel.h"
#include "synthesizer/BasicSynthesizer.h"
#include "ui/oscillor/OscillorPanel.h"
#include "ui/filter/FilterPanel.h"

rpSynth::ui::FilterAndOscillorPanel::FilterAndOscillorPanel(audio::BasicSynthesizer& b) {
    m_oscPanel1 = std::make_unique<OscillorPanel>(b.m_polyOscillor);
    m_filterPanel1 = std::make_unique<FilterPanel>(b.m_filter);

    addAndMakeVisible(m_oscPanel1.get());
    addAndMakeVisible(m_filterPanel1.get());
}

void rpSynth::ui::FilterAndOscillorPanel::resized() {
    m_oscPanel1->setBoundsRelative(0.f, 0.f, 0.5f, 0.5f);
    m_filterPanel1->setBoundsRelative(0.5f, 0.f, 0.5f, 0.5f);
}

void rpSynth::ui::FilterAndOscillorPanel::paint(juce::Graphics& g) {
    g.fillAll(juce::Component::findColour(juce::DocumentWindow::backgroundColourId));
}

void rpSynth::ui::FilterAndOscillorPanel::showModulationFrom(audio::ModulatorBase* p) {
    m_oscPanel1->showModulationFrom(p);
    m_filterPanel1->showModulationFrom(p);
}
