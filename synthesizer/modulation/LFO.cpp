/*
  ==============================================================================

    LFO.cpp
    Created: 14 Aug 2023 10:04:34pm
    Author:  mana

  ==============================================================================
*/

#include "LFO.h"
#include "ui/modulation/LFOPanel.h"

namespace rpSynth::audio {
FType LFO::getPhase() const {
    return std::fmod(m_phase.phase, static_cast<FType>(1));
}

void LFO::addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    layout.add(std::make_unique<MyHostParameter>(m_lfoFrequency,
                                                 combineWithID("frequency"),
                                                 "frequency",
                                                 juce::NormalisableRange<float>(0.f, 20.f, 0.01f, 0.4f),
                                                 0.f));
}

void LFO::prepareParameters(FType sampleRate, size_t) {
    //m_lfoFrequency.prepare(sampleRate, numSamples);
    m_lfoFrequency.prepare(sampleRate);
}

void LFO::prepareExtra(FType /*sr*/, size_t) {
    //m_linearSmoother.reset(sr, kCRSmoothTime);
        //m_totalNumSamples = static_cast<size_t>(sr / kControlRate);
}

void LFO::noteOn() {
    // Only noteOn phase set 0
    m_phase.phase = FType{};
}

void LFO::noteOff() {
}

void LFO::onCRClock(size_t intervalSamplesInSR) {
    if (m_lineGenerator.isStateChanged()) {
        m_lineGenerator.render(m_lookUpTable.data(), kResolution);
        m_lookUpTable[kResolution] = m_lookUpTable[0];
    }

    m_lfoFrequency.onCRClock();

    FType currenFre = m_lfoFrequency.getTargetValue();
    FType oneIncrease = currenFre / m_sampleRate;
    FType totalIncrease = intervalSamplesInSR * oneIncrease;
    FType p = m_phase.increase(totalIncrease);
    size_t i = static_cast<size_t>(p * kResolution);

    // TODO: do interpole here
    setOutputValue(m_lookUpTable[i]);
}

JUCE_NODISCARD juce::Component* LFO::createControlComponent() {
    return new ui::LFOPanel(*this);
}

void LFO::saveExtraState(juce::XmlElement& xml) {
    m_lineGenerator.saveState(xml);
}

void LFO::loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState&) {
    m_lineGenerator.loadState(xml);
}
}