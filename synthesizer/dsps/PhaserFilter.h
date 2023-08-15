/*
  ==============================================================================

    PhaserFilter.h
    Created: 15 Jul 2023 4:42:44pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#include "AllPassFilter.h"

template<typename SampleType>
    requires std::is_floating_point_v<SampleType>
struct PhaserFilter {
    using AllPassFilter = SecondOrderAllPassFilter2<SampleType>;
    using SmoothedValue = juce::SmoothedValue<SampleType>;

    static constexpr SampleType kMaxFrequency = 20000.f;
    static constexpr SampleType kMinFrequency = 20.f;
    static constexpr size_t kMaxPhaserState = 16;
    static constexpr double kSmoothTime = 0.05;

    void prepare(juce::dsp::ProcessSpec const &spec) {
        m_sampleRate = spec.sampleRate;
        m_lastOutputs.resize(spec.numChannels);

        for (AllPassFilter& filter : m_allPassFilters) {
            filter.prepare(spec);
        }

        initSmoothParameter(spec.sampleRate);
    };

    void reset() {
        std::ranges::fill(m_lastOutputs, static_cast<SampleType>(0));

        for (AllPassFilter& filter : m_allPassFilters) {
            filter.reset();
        }
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

            update();
        }
    }

    // === setters ===
    void setCutoffFrequency(SampleType cutoff) {
        jassert(cutoff >= kMinFrequency && cutoff <= kMaxFrequency);

        m_frequency.setTargetValue(cutoff);
    }

    void setQuality(SampleType quality) {
        m_quality.setTargetValue(quality);
    }

    void setPolarity(SampleType polarity) {
        m_polarity.setTargetValue(polarity);
    }

    void setFeedback(SampleType feedback) {
        m_feedback.setTargetValue(feedback);
    }

    void setState(SampleType state) {
        m_state.setTargetValue(state);
    }

private:
    // === internal functions ===
    SampleType processSingle(SampleType input, int channel) {
        auto fbInput = input + m_lastOutputs[channel] * _feedback;
        auto state = static_cast<int>(_state);

        for (int i = 0; i < state; i++) {
            AllPassFilter& apf = m_allPassFilters[i];
            fbInput = apf.processSingle(m_sharedCoeffects, fbInput, channel);
        }

        m_lastOutputs[channel] = fbInput;
        return fbInput * _polarity + input;
    }

    void initSmoothParameter(SampleType sampleRate) {
        m_frequency.reset(sampleRate, kSmoothTime);
        m_polarity.reset(sampleRate, kSmoothTime);
        m_quality.reset(sampleRate, kSmoothTime);
        m_state.reset(sampleRate, kSmoothTime);
        m_feedback.reset(sampleRate, kSmoothTime);
    }

    void update() {
        _frequency = m_frequency.getNextValue();
        _quality = m_quality.getNextValue();
        _polarity = m_polarity.getNextValue();
        _state = m_state.getNextValue();
        _feedback = m_feedback.getNextValue();

        m_sharedCoeffects.setCenterFrequency(_frequency, m_sampleRate);
        m_sharedCoeffects.setBandWidth(_quality, m_sampleRate);
    }

    // === parameters ===
    SmoothedValue m_frequency = static_cast<SampleType>(440);
    SmoothedValue m_quality = static_cast<SampleType>(0.25);
    SmoothedValue m_polarity = static_cast<SampleType>(0);
    SmoothedValue m_state = static_cast<SampleType>(6);
    SmoothedValue m_feedback = static_cast<SampleType>(0);

    SampleType _frequency{};
    SampleType _quality{};
    SampleType _polarity{};
    SampleType _state{};
    SampleType _feedback{};

    // === the all pass filters ===
    SampleType m_sampleRate;
    AllPassFilter::Coeffects m_sharedCoeffects;
    std::array<AllPassFilter, kMaxPhaserState> m_allPassFilters;

    // === the last output of each channel ===
    std::vector<SampleType> m_lastOutputs;
};