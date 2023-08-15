/*
  ==============================================================================

    LFO.h
    Created: 24 Jul 2023 2:51:45pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#ifndef RPSYNTH_MODULATION_LFO_H
#define RPSYNTH_MODULATION_LFO_H

#include "ModulatorBase.h"
#include "LineGenerator.h"
#include "synthesizer/NewWrapParameter.h"

namespace rpSynth::audio {
class LFO : public ModulatorBase {
public:
    static constexpr size_t kResolution = 2048;

    using ModulatorBase::ModulatorBase;

    struct Phase {
        FType phase{};

        FType increase(FType fre, FType sr) {
            jassert(fre >= FType{});

            FType res = phase;
            FType advance = fre / sr;
            increase(advance);

            return res;
        }

        FType increase(FType phaseAdd) {
            jassert(phaseAdd >= FType{});

            FType res = phase;
            phase += phaseAdd;
            phase = std::fmod(phase, static_cast<FType>(1));

            return res;
        }
    };

    FType getPhase() const;

    //===============================================================
    // implement from AudioProcessBase
    void addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;

    //void updateParameters(size_t numSamples) override {
    //    m_lfoFrequency.updateParameter(numSamples);

    //    // To reduce line generator render,this function only called when a block come
    //    if (m_lineGenerator.isStateChanged()) {
    //        m_lineGenerator.render(m_lookUpTable.data(), kResolution);
    //        m_lookUpTable[kResolution] = m_lookUpTable[0];
    //    }
    //}

    void prepareParameters(FType sampleRate, size_t /*numSamples*/) override;
    //===============================================================

    //===============================================================
    // implement from ModulatorBase
    void prepareExtra(FType sr, size_t /*num*/) override;

    //void generateData(size_t beginSamplePos, size_t endSamplePos) override {
    //    if (m_lineGenerator.isStateChanged()) {
    //        m_lineGenerator.render(m_lookUpTable.data(), kResolution);
    //        m_lookUpTable[kResolution] = m_lookUpTable[0];
    //    }

    //    // in cr mode
    //    for (; beginSamplePos < endSamplePos; beginSamplePos++) {
    //        if (m_leftNumSamples == 0) {
    //            m_leftNumSamples = m_totalNumSamples;
    //            auto phaseAdd = m_lfoFrequency.get(beginSamplePos) * m_totalNumSamples / m_sampleRate;
    //            m_phase.increase(phaseAdd);
    //            auto index = static_cast<size_t>(m_phase.phase * kResolution);
    //            m_linearSmoother.setTargetValue(m_lookUpTable[index]);
    //        }

    //        m_outputBuffer[beginSamplePos] = m_linearSmoother.getNextValue();
    //        m_leftNumSamples--;
    //    }
    //    return;

    //    // normal sr
    //    auto sr = this->getSampleRate();
    //    auto& buffer = this->getOutputBuffer();

    //    for (; beginSamplePos < endSamplePos; beginSamplePos++) {
    //        auto x = m_phase.increase(m_lfoFrequency.get(beginSamplePos), sr);
    //        size_t index = static_cast<size_t>(x * kResolution);
    //        buffer[beginSamplePos] = m_lookUpTable[index];
    //    }
    //}

    //FType onCRClock(size_t intervalSamplesInSR,size_t index) {
    //    FType currenFre = m_lfoFrequency.get(index);
    //    FType oneIncrease = currenFre / m_sampleRate;
    //    FType totalIncrease = static_cast<FType>(intervalSamplesInSR * oneIncrease);
    //    FType p = m_phase.increase(totalIncrease);
    //    size_t i = static_cast<size_t>(p * kResolution);

    //    // TODO: do interpole here
    //    return m_lookUpTable[i];
    //}

    void noteOn() override;

    void noteOff() override;

    void onCRClock(size_t intervalSamplesInSR) override;

    JUCE_NODISCARD juce::Component* createControlComponent() override;

    void saveExtraState(juce::XmlElement& xml) override;

    void loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& /*apvts*/) override;
    //===============================================================
public:
    // The line generator
    LineGenerator m_lineGenerator;
public:
    // Parameters
    //MyAudioProcessParameter m_lfoFrequency{false};
    MyAudioParameter m_lfoFrequency;
private:
    //CR
    //juce::SmoothedValue<FType> m_linearSmoother;
    //size_t m_totalNumSamples{};
    //size_t m_leftNumSamples{};

    std::array<FType, kResolution + 1> m_lookUpTable{};
    Phase m_phase;
};
}
#endif // !RPSYNTH_MODULATION_LFO_H
