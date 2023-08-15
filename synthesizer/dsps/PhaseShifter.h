/*
  ==============================================================================

    PhaseShifter.h
    Created: 15 Aug 2023 10:50:47am
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "IIRHilbertTransform.h"

namespace dsps {
template<typename ParamType, typename SampleType, size_t numChannels>
class PhaseShifter {
public:
    PhaseShifter() {
        for (auto& h : m_hilberts) {
            h.init(IIRHilbertCoeffect::kCoeffects1<SampleType>);
        }
    }

    template<size_t channel>
    SampleType process(SampleType input, ParamType phase) {
        ParamType sinVal = std::sin(juce::MathConstants<ParamType>::twoPi * phase);
        ParamType cosVal = std::cos(juce::MathConstants<ParamType>::twoPi * phase);
        SampleType real{};
        SampleType imag{};
        m_hilberts[channel].process(input, &real, &imag);
        return real * cosVal + imag * sinVal;
    }

    void reset() {
        for (auto& h : m_hilberts) {
            h.reset();
        }
    }
private:
    std::array<IIRHilbertTransformer<SampleType, 4>, numChannels> m_hilberts;
};
}