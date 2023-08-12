/*
  ==============================================================================

    CombFilter.h
    Created: 15 Jul 2023 4:42:35pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define SECOND

template<typename SampleType>
    requires std::is_floating_point_v<SampleType>
struct CombFilter {
    using DelayLine = juce::dsp::DelayLine<SampleType>;

    static constexpr SampleType kMaxFrequency = 20000.f;
    static constexpr SampleType kMinFrequency = 20.f;

    void prepare(juce::dsp::ProcessSpec const &spec) {
        m_sampleRate = spec.sampleRate;

        m_delayTime.reset(spec.sampleRate, kSmoothTime);
        m_feedback.reset(spec.sampleRate, kSmoothTime);
        m_polarity.reset(spec.sampleRate, kSmoothTime);

        m_lastOutputs.resize(spec.numChannels);

        m_channelDelayLine.prepare(spec);
        m_channelDelayLine.setMaximumDelayInSamples(static_cast<int>(std::ceil(
            getDelayTimeByFrequency(10.f)
        )));
    };

    void reset() {
        m_channelDelayLine.reset();
        std::ranges::fill(m_lastOutputs, static_cast<SampleType>(0));
    }

    template<class ContextType>
    void process(ContextType context) {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        jassert(inputBlock.getNumChannels() == outputBlock.getNumChannels());
            
        auto numChannels = inputBlock.getNumChannels();
        auto numSamples = inputBlock.getNumSamples();
        
        for (decltype(numSamples) i = 0; i < numSamples; i++) {

            for (decltype(numChannels) ch = 0; ch < numChannels; ch++) {

                auto* src = inputBlock.getChannelPointer(ch);
                auto* dst = outputBlock.getChannelPointer(ch);

                dst[i] = processSingle(src[i], ch);
            }

            updateSmoothParameters();
        }
    }

    // === setters ===
    void setCutoffFrequency(SampleType cutoff) {
        jassert(cutoff >= kMinFrequency && cutoff <= kMaxFrequency);

        auto newDelayInSamples = getDelayTimeByFrequency(cutoff);
        m_delayTime.setTargetValue(newDelayInSamples);
    }

    void setFeedback(SampleType feedback) {
        m_feedback.setTargetValue(feedback);
    }

    void setPolarity(SampleType polarity) {
        m_polarity.setTargetValue(polarity);
    }

private:
    SampleType m_sampleRate{};

    // === internal functions ===
    SampleType processSingle(SampleType input, size_t channel) {
        auto pushIn = input + _feedback * m_lastOutputs[channel];
        m_channelDelayLine.pushSample(channel, pushIn);

        auto output = m_channelDelayLine.popSample(channel, _delayTime);
        m_lastOutputs[channel] = output;

        return output * _polarity + input;
    }

    void updateSmoothParameters() {
        _delayTime = m_delayTime.getNextValue();
        _feedback = m_feedback.getNextValue();
        _polarity = m_polarity.getNextValue();
    }

    SampleType getDelayTimeByFrequency(SampleType frequency) {
        jassert(m_sampleRate != static_cast<SampleType>(0));

        auto seconds = static_cast<SampleType>(1) / frequency;
        return seconds * m_sampleRate;
    }

    // === parameters ===
    using SmoothedValue = juce::SmoothedValue<SampleType>;
    static constexpr double kSmoothTime = 0.05 SECOND;

    SmoothedValue m_delayTime;
    SmoothedValue m_feedback;
    SmoothedValue m_polarity;

    SampleType _delayTime{};
    SampleType _feedback{};
    SampleType _polarity{};

    // === the delay lines ===
    DelayLine m_channelDelayLine;

    // === the last output of each channel ===
    std::vector<SampleType> m_lastOutputs;
};