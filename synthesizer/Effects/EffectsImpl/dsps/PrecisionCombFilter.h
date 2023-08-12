/*
  ==============================================================================

    precisionCombFilter.h
    Created: 17 Jul 2023 2:52:11pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

template<typename SampleType>
    requires std::is_floating_point_v<SampleType>
struct PrecisionCombFilter {
    using DelayLine = juce::dsp::DelayLine<SampleType>;

    static constexpr int kMaxDelay = 512;

    void prepare(juce::dsp::ProcessSpec const &spec) {
        m_lastOutputs.resize(spec.numChannels);

        m_channelDelayLine.prepare(spec);
        m_channelDelayLine.setMaximumDelayInSamples(kMaxDelay);
    };

    void reset() {
        m_channelDelayLine.reset();
        std::ranges::fill(m_lastOutputs, static_cast<SampleType>(0));
    }

    // === setters ===
    void setNotchAmount(SampleType amount) {
        jassert(amount >= 0 && amount <= kMaxDelay);

        m_notchAmount = amount;
    }

    void setFeedback(SampleType feedback) {
        m_feedback = feedback;
    }

    void setPolarity(SampleType polarity) {
        m_polarity = polarity;
    }

    SampleType processSingle(SampleType input, int channel) {
        auto pushIn = input + m_feedback * m_lastOutputs[channel];
        m_channelDelayLine.pushSample(channel, pushIn);

        auto output = m_channelDelayLine.popSample(channel, m_notchAmount);
        m_lastOutputs[channel] = output;

        return output * m_polarity + input;
    }

private:
    // === parameters ===
    SampleType m_notchAmount{};
    SampleType m_feedback{};
    SampleType m_polarity{};

    // === the delay lines ===
    DelayLine m_channelDelayLine;

    // === the last output of each channel ===
    std::vector<SampleType> m_lastOutputs;
};