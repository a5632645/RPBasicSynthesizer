/*
  ==============================================================================

    Envelop.cpp
    Created: 4 Aug 2023 8:54:43pm
    Author:  mana

  ==============================================================================
*/

#include "Envelop.h"
#include "ui/modulation/EnvelopPanel.h"

namespace rpSynth::audio {
static constexpr FType oneThousandInv = static_cast<FType>(0.001);

void Envelop::addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    auto timeRange = juce::NormalisableRange<float>(0.f, 12000.f, 0.1f, 0.5f);
    layout.add(std::make_unique<MyHostedAudioProcessorParameter>(&m_attackInMillSeconds,
                                                                 combineWithID("attack"),
                                                                 "attack",
                                                                 timeRange,
                                                                 300.f),
               std::make_unique<MyHostedAudioProcessorParameter>(&m_holdInMillSeconds,
                                                                 combineWithID("hold"),
                                                                 "hold",
                                                                 timeRange,
                                                                 300.f),
               std::make_unique<MyHostedAudioProcessorParameter>(&m_decayInMillSeconds,
                                                                 combineWithID("decay"),
                                                                 "decay",
                                                                 timeRange,
                                                                 300.f),
               std::make_unique<MyHostedAudioProcessorParameter>(&m_sustainLevelInDecibels,
                                                                 combineWithID("sustain"),
                                                                 "sustain",
                                                                 juce::NormalisableRange<float>(0.f, 1.f, 0.01f),
                                                                 0.8f),
               std::make_unique<MyHostedAudioProcessorParameter>(&m_releaseInMillSeconds,
                                                                 combineWithID("release"),
                                                                 "release",
                                                                 timeRange,
                                                                 300.f));
}

void Envelop::updateParameters(size_t numSamples) {
    m_attackInMillSeconds.updateParameter(numSamples);
    m_holdInMillSeconds.updateParameter(numSamples);
    m_decayInMillSeconds.updateParameter(numSamples);
    m_sustainLevelInDecibels.updateParameter(numSamples);
    m_releaseInMillSeconds.updateParameter(numSamples);
}

void Envelop::prepareParameters(FType sampleRate, size_t numSamples) {
    m_attackInMillSeconds.prepare(sampleRate, numSamples);
    m_holdInMillSeconds.prepare(sampleRate, numSamples);
    m_decayInMillSeconds.prepare(sampleRate, numSamples);
    m_sustainLevelInDecibels.prepare(sampleRate, numSamples);
    m_releaseInMillSeconds.prepare(sampleRate, numSamples);
}

void Envelop::prepareExtra(FType sr, size_t /*num*/) {
    m_linearSmoother.reset(sr, kCRSmoothTime);
    m_totalNumSamples = static_cast<size_t>(sr / kControlRate);
}

void Envelop::saveExtraState(juce::XmlElement& /*xml*/) {
    // No extra data for envelop
}

void Envelop::loadExtraState(juce::XmlElement& /*xml*/, juce::AudioProcessorValueTreeState& /*apvts*/) {
    // No extra data for envelop
}

void Envelop::generateData(size_t beginSamplePos, size_t endSamplePos) {
    for (; beginSamplePos < endSamplePos; beginSamplePos++) {
        if (m_leftNumSamples == 0) {
            m_leftNumSamples = m_totalNumSamples;
            auto sample = onCRClock(m_totalNumSamples, beginSamplePos);
            m_linearSmoother.setTargetValue(sample);
        }

        m_outputBuffer[beginSamplePos] = m_linearSmoother.getNextValue();
        m_leftNumSamples--;
    }
}

FType Envelop::onCRClock(size_t intervalSamplesInSR, size_t index) {
    m_attackLength = static_cast<size_t>(m_sampleRate * oneThousandInv * m_attackInMillSeconds.get(index));
    m_holdLength = static_cast<size_t>(m_sampleRate * oneThousandInv * m_holdInMillSeconds.get(index));
    m_decayLength = static_cast<size_t>(m_sampleRate * oneThousandInv * m_decayInMillSeconds.get(index));
    m_releaseLength = static_cast<size_t>(m_sampleRate * oneThousandInv * m_releaseInMillSeconds.get(index));
    FType sustainLevel = m_sustainLevelInDecibels.get(index);

    // change state
    m_currentPosition += intervalSamplesInSR;
_changeState:
    switch (m_currentState) {
        case Envelop::EnvelopState::Init:
            break;
        case Envelop::EnvelopState::Attack:
            if (m_currentPosition >= m_attackLength) {
                m_currentState = EnvelopState::Hold;
                m_currentPosition = 0;
                goto _changeState;
            }
            break;
        case Envelop::EnvelopState::Hold:
            if (m_currentPosition >= m_holdLength) {
                m_currentState = EnvelopState::Decay;
                m_currentPosition = 0;
                goto _changeState;
            }
            break;
        case Envelop::EnvelopState::Decay:
            if (m_currentPosition >= m_decayLength) {
                m_currentState = EnvelopState::Sustain;
                m_currentPosition = 0;
                goto _changeState;
            }
            break;
        case Envelop::EnvelopState::Sustain:
            break;
        case Envelop::EnvelopState::Release:
            if (m_currentPosition >= m_releaseLength) {
                m_currentState = EnvelopState::Init;
                m_currentPosition = 0;
            }
            break;
        default:
            jassertfalse;
            break;
    }

    // get sample
    switch (m_currentState) {
        case Envelop::EnvelopState::Init:
            // Do nothing
            return FType{};
        case Envelop::EnvelopState::Attack:
            return static_cast<FType>(m_currentPosition) / static_cast<FType>(m_attackLength);
        case Envelop::EnvelopState::Hold:
            return FType{1};
        case Envelop::EnvelopState::Decay:
            return static_cast<FType>(1) + (sustainLevel - static_cast<FType>(1))
                * static_cast<FType>(m_currentPosition) / static_cast<FType>(m_decayLength);
        case Envelop::EnvelopState::Sustain:
            return sustainLevel;
        case Envelop::EnvelopState::Release:
            return sustainLevel - sustainLevel
                * static_cast<FType>(m_currentPosition) / static_cast<FType>(m_releaseLength);
        default:
            return FType{};
    }
}

void Envelop::noteOn() {
    m_currentPosition = 0;
    m_currentState = EnvelopState::Attack;
}

void Envelop::noteOff() {
    m_currentPosition = 0;
    if (m_currentState != EnvelopState::Init) {
        m_currentState = EnvelopState::Release;
    }
}

JUCE_NODISCARD juce::Component* Envelop::createControlComponent() {
    return new ui::EnvelopPanel(*this);
}

//===============================================================
std::pair<Envelop::EnvelopState, float> Envelop::getCurrentEnvelopState() const {
    float ration = 0.f;
    switch (m_currentState) {
        case rpSynth::audio::Envelop::EnvelopState::Init:
            ration = 0.f;
            break;
        case rpSynth::audio::Envelop::EnvelopState::Attack:
            ration = m_attackLength == 0 ? 1.f : static_cast<float>(m_currentPosition) / m_attackLength;
            break;
        case rpSynth::audio::Envelop::EnvelopState::Hold:
            ration = m_holdLength == 0 ? 1.f : static_cast<float>(m_currentPosition) / m_holdLength;
            break;
        case rpSynth::audio::Envelop::EnvelopState::Decay:
            ration = m_decayLength == 0 ? 1.f : static_cast<float>(m_currentPosition) / m_decayLength;
            break;
        case rpSynth::audio::Envelop::EnvelopState::Sustain:
            ration = 0.5f;
            break;
        case rpSynth::audio::Envelop::EnvelopState::Release:
            ration = m_releaseLength == 0 ? 1.f : static_cast<float>(m_currentPosition) / m_releaseLength;
            break;
    }

    return std::pair<EnvelopState, float>(m_currentState, ration);
}
//===============================================================
};