/*
  ==============================================================================

    AllPassFilter.h
    Created: 16 Jul 2023 8:21:41pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>



template<typename ParamType, typename SampleType>
class SecondOrderAllPassFilter2 {
public:
    struct Coeffects {
        /**
         * @param fre 0.f < fre < 0.5 * SR
         * @param sampleRate SR
        */
        void setCenterFrequency(ParamType fre, ParamType sampleRate) {
            jassert(juce::isPositiveAndBelow(fre, static_cast<ParamType>(0.5) * sampleRate));

            m_d = -std::cos(juce::MathConstants<ParamType>::twoPi * fre / sampleRate);
        }

        /**
         * @param bw 0 < bw < 0.5
         * @param sampleRate SR
        */
        void setBandWidth(ParamType bw, ParamType /*sampleRate*/) {
            jassert(juce::isPositiveAndBelow(bw, static_cast<ParamType>(0.5)));

            auto t = std::tan(juce::MathConstants<ParamType>::pi * bw);
            m_c = (t - 1) / (t + 1);
        }

        ParamType m_d;
        ParamType m_c;
    };

    // https://thewolfsound.com/allpass-filter/
    SampleType processSingle(Coeffects const& coeffects, SampleType input) {
        auto newV = input - coeffects.m_d * (1 - coeffects.m_c) * data.v0 + coeffects.m_c * data.v1;
        auto newY = -coeffects.m_c * newV + coeffects.m_d * (1 - coeffects.m_c) * data.v0 + data.v1;

        data.v1 = data.v0;
        data.v0 = newV;
        data.y1 = data.y0;
        data.y0 = newY;
        return newY;
    }

    void reset() {
        data = InternalData{};
    }

private:
    struct InternalData {
        SampleType v0{};
        SampleType v1{};
        SampleType y0{};
        SampleType y1{};
    };

    InternalData data;
};