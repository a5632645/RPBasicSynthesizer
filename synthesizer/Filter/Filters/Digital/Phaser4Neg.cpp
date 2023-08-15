/*
  ==============================================================================

    Phaser4Neg.cpp
    Created: 15 Aug 2023 2:24:23pm
    Author:  mana

  ==============================================================================
*/

#include "Phaser4Neg.h"
#include "ui/filter/FilterPanel.h"
#include "synthesizer/Filter/AllFilterParameters.h"

namespace rpSynth::audio::filters::digital {
void Phaser4Neg::process(StereoBuffer& input, StereoBuffer& output, size_t begin, size_t end) {
    for (size_t i = begin; i < end; i++) {
        m_sharedCoeffects.setCenterFrequency(semitoneToHertz(m_parameters.cutoff.getNextValue()), m_sampleRate);
        m_sharedCoeffects.setBandWidth(juce::jmap<FType>(m_parameters.resonance.getNextValue(), 0.01, 0.49), m_sampleRate);
        PolyFType sample = input[i] + m_fbValue * m_parameters.feedback.getNextValue();
        for (auto& f : m_apfs) {
            sample = f.processSingle(m_sharedCoeffects, sample);
        }
        FType phase = m_parameters.phase.getNextValue();
        sample.left = m_phaseShifter.process<0>(sample.left, phase);
        sample.right = m_phaseShifter.process<1>(sample.right, phase);
        m_fbValue = sample;
        output[i] = sample - input[i];
    }
}

void Phaser4Neg::reset() {
    for (auto& f : m_apfs) {
        f.reset();
    }
}

void Phaser4Neg::prepare(FType sampleRate, size_t numSamples) {
    m_sampleRate = sampleRate;
}

void Phaser4Neg::doLayout(ui::FilterKnobsPanel& p) {
    p.m_cutoff.setVisible(true);
    p.m_resonance.setVisible(true);
    p.m_feedback.setVisible(true);
    p.m_phase.setVisible(true);

    p.m_cutoff.setBounds(0, 0, 70, 70);
    p.m_resonance.setBounds(80, 0, 70, 70);
    p.m_feedback.setBounds(160, 0, 70, 70);
    p.m_phase.setBounds(240, 0, 70, 70);
}
}