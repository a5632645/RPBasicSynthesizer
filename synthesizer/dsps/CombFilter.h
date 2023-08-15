/*
  ==============================================================================

    CombFilter.h
    Created: 15 Jul 2023 4:42:35pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <array>
#include <cmath>
#include <vector>

template<typename ParamType, typename SampleType, size_t numChannels>
class CombFilter {
public:
    static constexpr ParamType kMinFrequency = ParamType{5};

    void prepare(ParamType sampleRate) {
        m_sampleRate = sampleRate;
        m_delayBufferSize = static_cast<size_t>(m_sampleRate / kMinFrequency);
        for (auto& v : m_delayBuffers) {
            v.resize(m_delayBufferSize, SampleType{});
        }
    }

    void reset() {
        m_writePosition = 0;
        m_lastOutput.fill(SampleType{});
        for (auto& v : m_delayBuffers) {
            std::fill(v.begin(), v.end(), SampleType{});
        }
    }

    SampleType process(size_t channel, SampleType in) {
        return processWet(channel, in) + in;
    }

    SampleType processWet(size_t channel, SampleType in) {
        auto& dBuffer = m_delayBuffers[channel];
        dBuffer[m_writePosition] = in;
        size_t readPosition = (m_writePosition - static_cast<size_t>(m_delayInSamples)) % m_delayBufferSize;
        size_t nextPosition = (readPosition + 1) % m_delayBufferSize;
        ParamType interpole = m_delayInSamples - std::floor(m_delayInSamples);
        SampleType delayOut = dBuffer[readPosition] + (dBuffer[nextPosition] - dBuffer[readPosition]) * interpole;
        //Type delayOut = dBuffer[readPosition];
        m_writePosition = (m_writePosition + 1) % m_delayBufferSize;
        return delayOut * m_polarity;
    }

    void setFrequency(ParamType fre) {
        m_delayInSamples = m_sampleRate / fre;
    }

    void setDelaySamples(ParamType delay) {
        m_delayInSamples = delay;
    }

    void setPolarity(ParamType p) {
        m_polarity = p;
    }
private:
    size_t m_delayBufferSize{};
    size_t m_writePosition{};
    ParamType m_sampleRate{};
    ParamType m_delayInSamples{};
    ParamType m_polarity{};

    std::array<SampleType, numChannels> m_lastOutput{};
    std::array<std::vector<SampleType>, numChannels> m_delayBuffers;
};