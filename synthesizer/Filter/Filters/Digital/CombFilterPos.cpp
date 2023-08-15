/*
  ==============================================================================

    CombFilter.cpp
    Created: 13 Aug 2023 4:35:29pm
    Author:  mana

  ==============================================================================
*/

#include "CombFilterPos.h"
#include "ui/filter/FilterPanel.h"
#include "synthesizer/Filter/AllFilterParameters.h"

namespace rpSynth::audio::filters::digital {
void CombPos::process(StereoBuffer& input, StereoBuffer& output, size_t begin, size_t end) {
    for (; begin < end; begin++) {
        FType delay = m_smoothedDelaySamples.getNextValue();
        FType phase = m_parameters.phase.getNextValue();
        PolyFType withFB = input[begin] + m_lastOutput * m_parameters.resonance.getNextValue();
        m_delayBuffer[m_writePosition] = withFB;
        size_t readPosition = static_cast<size_t>(m_writePosition - delay + m_delayBufferSize) % m_delayBufferSize;
        size_t nextPosition = static_cast<size_t>(readPosition + 1) % m_delayBufferSize;
        FType interpole = delay - std::floor(delay);
        PolyFType delayout = m_delayBuffer[readPosition]
            + (m_delayBuffer[nextPosition] - m_delayBuffer[readPosition]) * interpole;

        m_lastOutput.left = m_phaseShifter.process<0>(delayout.left, phase);
        m_lastOutput.right = m_phaseShifter.process<1>(delayout.right, phase);
        output[begin] = input[begin] + m_lastOutput;
        m_writePosition = (m_writePosition + 1) % m_delayBufferSize;
    }
}

void CombPos::reset() {
    m_lastOutput = PolyFType{};
    m_writePosition = 0;
    std::ranges::fill(m_delayBuffer, PolyFType{});
    m_phaseShifter.reset();
}

void CombPos::prepare(FType sampleRate, size_t /*numSamples*/) {
    m_sampleRate = sampleRate;
    m_smoothedDelaySamples.reset(sampleRate, 0.1);
    m_delayBufferSize = static_cast<size_t>(sampleRate / m_parameters.cutoff.getParameterRange().start) + 16;
    m_delayBuffer.resize(m_delayBufferSize);
}

void CombPos::doLayout(ui::FilterKnobsPanel& p) {
    p.m_cutoff.setVisible(true);
    p.m_resonance.setVisible(true);
    p.m_phase.setVisible(true);

    p.m_cutoff.setBounds(0, 0, 70, 70);
    p.m_resonance.setBounds(80, 0, 70, 70);
    p.m_phase.setBounds(160, 0, 70, 70);
}

void CombPos::onCRClock(size_t) {
    FType cutoff = semitoneToHertz(m_parameters.cutoff.getTargetValue());
    m_smoothedDelaySamples.setTargetValue(m_sampleRate / cutoff);
}
}