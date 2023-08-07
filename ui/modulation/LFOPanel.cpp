/*
  ==============================================================================

    LFOPanel.cpp
    Created: 27 Jul 2023 9:29:14pm
    Author:  mana

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LFOPanel.h"
#include "../../synthesizer/modulation/LFO.h"

namespace rpSynth::ui {
class LFOPanel::LFOPointer : public juce::Component, public juce::Timer {
public:
    ~LFOPointer() = default;
    LFOPointer(audio::LFO& l) : lfo(l) {
        startTimerHz(10);
        setInterceptsMouseClicks(false, false);
    }

    void paint(juce::Graphics& g) {
        g.setColour(juce::Colours::white);
        int x = static_cast<int>(lfo.getPhase() * getWidth());
        g.drawVerticalLine(x, 0.f, static_cast<float>(getHeight()));
    }

    void timerCallback() override {
        if (lfo.hasNoModulationTargets()) {
            return;
        }

        repaint();
    }
private:
    audio::LFO& lfo;
};
}

namespace rpSynth {
namespace ui {
LFOPanel::LFOPanel(audio::LFO& lfo)
    : m_LFOBind(lfo)
    , m_lineGeneratorPanel(lfo.m_lineGenerator)
    , m_LFOFrequency(&lfo.m_lfoFrequency) {
    m_lfoPointer = std::make_unique<LFOPointer>(lfo);

    addAndMakeVisible(m_LFOFrequency);
    addAndMakeVisible(m_lineGeneratorPanel);
    addAndMakeVisible(m_lfoPointer.get());
}

LFOPanel::~LFOPanel() {
    m_lfoPointer = nullptr;
}

void LFOPanel::resized() {
    auto knobWH = 70;
    auto lineBound = juce::Rectangle<int>(0, 0, getWidth(), getHeight() - knobWH);
    m_lineGeneratorPanel.setBounds(lineBound);
    m_lfoPointer->setBounds(lineBound);

    auto knobBound = juce::Rectangle<int>(0, getHeight() - knobWH, knobWH, knobWH);
    m_LFOFrequency.setBounds(knobBound);
}
void LFOPanel::paint(juce::Graphics& g) {
    g.fillAll(juce::Component::findColour(juce::DocumentWindow::backgroundColourId));
}

void LFOPanel::showModulationFrom(audio::ModulatorBase* p) {
    m_LFOFrequency.showModulationFrom(p);
}
}
}