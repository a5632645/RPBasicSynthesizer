/*
  ==============================================================================

    FIRHilbertTransform.h
    Created: 18 Jul 2023 9:15:59pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

template<typename SampleType, int halfFilterLength>
    requires requires{
    std::is_floating_point_v<SampleType>;
    halfFilterLength > 0;
}
struct FIRHilbertTransformer {
    static constexpr int kLength = 2 * halfFilterLength + 1;
    static constexpr int kCoeffects = static_cast<int>((halfFilterLength + 1) / 2.0);

    static SampleType hilbert(int n) {
        jassert(n >= 0);

        if (n % 2 == 0) {
            return 0;
        }

        return static_cast<SampleType>(2) / juce::MathConstants<SampleType>::pi / n;
    }

    FIRHilbertTransformer() {
        for (int i = 0; i < kCoeffects; i++) {
            int j = 2 * i + 1;
            m_coeffects[i] = hilbert(j);
        }

        reset();
    }

    void prepare(juce::dsp::ProcessSpec const& /*spec*/) {
    }

    void reset() {
        std::ranges::fill(m_zs, SampleType{});
    }

    void process(SampleType input, SampleType* pReal, SampleType* pImg) {
        SampleType output = static_cast<SampleType>(0);
        m_zs[0] = input;
        *pReal = m_zs[halfFilterLength];

        for (int i = 0; i < kCoeffects; i++) {
            int d = 2 * i + 1;
            SampleType add = m_zs[halfFilterLength + d] - m_zs[halfFilterLength - d];
            output += add * m_coeffects[i];
        }
        *pImg = output;

        // TODO:this is not good,you should improve it!
        for (int i = kLength - 1; i > 0; i--) {
            m_zs[i] = m_zs[i - 1];
        }
    }

private:
    std::array<SampleType, kLength> m_zs;
    std::array<SampleType, kCoeffects> m_coeffects;
};