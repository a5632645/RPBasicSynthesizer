/*
  ==============================================================================

    AllPassFilter.h
    Created: 16 Jul 2023 8:21:41pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

template<typename SampleType, size_t numChannel>
class SecondOrderAllPassFilter2 {
public:
    // https://thewolfsound.com/allpass-filter/
    struct Coeffects {
        /**
         * @param fre 0.f < fre < 0.5 * SR
         * @param sampleRate SR
        */
        void setCenterFrequency(SampleType fre, SampleType sampleRate) {
            jassert(juce::isPositiveAndBelow(fre, static_cast<SampleType>(0.5) * sampleRate));

            m_d = -std::cos(juce::MathConstants<SampleType>::twoPi * fre / sampleRate);
        }

        /**
         * @param bw 0 < bw < 0.5
         * @param sampleRate SR
        */
        void setBandWidth(SampleType bw, SampleType /*sampleRate*/) {
            jassert(juce::isPositiveAndBelow(bw, static_cast<SampleType>(0.5)));

            auto t = std::tan(juce::MathConstants<SampleType>::pi * bw);
            auto one = static_cast<SampleType>(1);
            m_c = (t - one) / (t + one);
        }

        SampleType m_d;
        SampleType m_c;
    };

    SampleType processSingle(Coeffects const& coeffects, SampleType input, int channel) {
        InternalData& data = m_data[channel];
        auto one = static_cast<SampleType>(1);

        auto newV = input - coeffects.m_d * (one - coeffects.m_c) * data.v0 + coeffects.m_c * data.v1;
        auto newY = -coeffects.m_c * newV + coeffects.m_d * (one - coeffects.m_c) * data.v0 + data.v1;

        data.v1 = data.v0;
        data.v0 = newV;
        data.y1 = data.y0;
        data.y0 = newY;
        return newY;
    }

    void reset() {
        std::ranges::fill(m_data, InternalData{});
    }

private:
    struct InternalData {
        SampleType v0{};
        SampleType v1{};
        SampleType y0{};
        SampleType y1{};
    };

    std::array<InternalData, numChannel> m_data;
};