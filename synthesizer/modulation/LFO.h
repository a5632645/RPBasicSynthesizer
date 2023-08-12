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
#include "synthesizer/AudioProcessorBase.h"
#include "ui/modulation/LFOPanel.h"

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

    FType getPhase() const{
        return std::fmod(m_phase.phase, static_cast<FType>(1));
    }

    //===============================================================
    // implement from AudioProcessBase
    void addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override {
        layout.add(std::make_unique<MyHostedAudioProcessorParameter>(&m_lfoFrequency,
                                                                     combineWithID("frequency"),
                                                                     "frequency",
                                                                     juce::NormalisableRange<float>(0.f, 20.f,0.01f,0.4f),
                                                                     0.f));
    }

    void updateParameters(size_t numSamples) override {
        m_lfoFrequency.updateParameter(numSamples);

        // To reduce line generator render,this function only called when a block come
        if (m_lineGenerator.isStateChanged()) {
            m_lineGenerator.render(m_lookUpTable.data(), kResolution);
            m_lookUpTable[kResolution] = m_lookUpTable[0];
        }
    }

    void prepareParameters(FType sampleRate, size_t numSamples) override {
        m_lfoFrequency.prepare(sampleRate, numSamples);
    }
    //===============================================================

    //===============================================================
    // implement from ModulatorBase
    void prepareExtra(FType sr, size_t /*num*/) override {
        m_linearSmoother.reset(sr, kCRSmoothTime);
        m_totalNumSamples = static_cast<size_t>(sr / kControlRate);
    }

    void generateData(size_t beginSamplePos, size_t endSamplePos) override {
        if (m_lineGenerator.isStateChanged()) {
            m_lineGenerator.render(m_lookUpTable.data(), kResolution);
            m_lookUpTable[kResolution] = m_lookUpTable[0];
        }

        // in cr mode
        for (; beginSamplePos < endSamplePos; beginSamplePos++) {
            if (m_leftNumSamples == 0) {
                m_leftNumSamples = m_totalNumSamples;
                auto phaseAdd = m_lfoFrequency.get(beginSamplePos) * m_totalNumSamples / m_sampleRate;
                m_phase.increase(phaseAdd);
                auto index = static_cast<size_t>(m_phase.phase * kResolution);
                m_linearSmoother.setTargetValue(m_lookUpTable[index]);
            }

            m_outputBuffer[beginSamplePos] = m_linearSmoother.getNextValue();
            m_leftNumSamples--;
        }
        return;

        //// normal sr
        //auto sr = this->getSampleRate();
        //auto& buffer = this->getOutputBuffer();

        //for (; beginSamplePos < endSamplePos; beginSamplePos++) {
        //    auto x = m_phase.increase(m_lfoFrequency.get(beginSamplePos), sr);
        //    size_t index = static_cast<size_t>(x * kResolution);
        //    buffer[beginSamplePos] = m_lookUpTable[index];
        //}
    }

    FType onCRClock(size_t intervalSamplesInSR,size_t index) {
        FType currenFre = m_lfoFrequency.get(index);
        FType oneIncrease = currenFre / m_sampleRate;
        FType totalIncrease = static_cast<FType>(intervalSamplesInSR * oneIncrease);
        FType p = m_phase.increase(totalIncrease);
        size_t i = static_cast<size_t>(p * kResolution);

        // TODO: do interpole here
        return m_lookUpTable[i];
    }

    void noteOn() override {
        // Only noteOn phase set 0
        m_phase.phase = FType{};
    }

    void noteOff() override {

    }

    JUCE_NODISCARD juce::Component* createControlComponent() override {
        return new ui::LFOPanel(*this);
    }

    void saveExtraState(juce::XmlElement& xml) override {
        m_lineGenerator.saveState(xml);
    }

    void loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& /*apvts*/) override {
        m_lineGenerator.loadState(xml);
    }
    //===============================================================
public:
    // The line generator
    LineGenerator m_lineGenerator;
public:
    // Parameters
    MyAudioProcessParameter m_lfoFrequency{false};
private:
    //CR
    juce::SmoothedValue<FType> m_linearSmoother;
    size_t m_totalNumSamples{};
    size_t m_leftNumSamples{};

    std::array<FType, kResolution + 1> m_lookUpTable;
    Phase m_phase;
};
}
#endif // !RPSYNTH_MODULATION_LFO_H
