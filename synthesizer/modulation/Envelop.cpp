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
    m_attackInMillSeconds.prepare(numSamples, numSamples);
    m_holdInMillSeconds.prepare(numSamples, numSamples);
    m_decayInMillSeconds.prepare(numSamples, numSamples);
    m_sustainLevelInDecibels.prepare(numSamples, numSamples);
    m_releaseInMillSeconds.prepare(numSamples, numSamples);
}

void Envelop::saveExtraState(juce::XmlElement& xml) {
    // No extra data for envelop
}

void Envelop::loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& apvts) {
    // No extra data for envelop
}

void Envelop::generateData(size_t beginSamplePos, size_t endSamplePos) {
    // deprecated function
}

FType Envelop::onCRClock(size_t intervalSamplesInSR, size_t index) {
    m_attackLength = static_cast<size_t>(m_sampleRate * oneThousandInv * m_attackInMillSeconds.get(index));
    m_holdLength = static_cast<size_t>(m_sampleRate * oneThousandInv * m_holdInMillSeconds.get(index));
    m_decayLength = static_cast<size_t>(m_sampleRate * oneThousandInv * m_decayInMillSeconds.get(index));
    m_releaseLength = static_cast<size_t>(m_sampleRate * oneThousandInv * m_releaseInMillSeconds.get(index));
    FType sustainLevel = m_sustainLevelInDecibels.get(index);

    // get sample
    FType res{};
    switch (m_currentState) {
        case Envelop::EnvelopState::Init:
            // Do nothing
            res = FType{};
            break;
        case Envelop::EnvelopState::Attack:
            if (m_attackLength != 0) {
                res = static_cast<FType>(m_currentPosition) / static_cast<FType>(m_attackLength);
                break;
            }
        case Envelop::EnvelopState::Hold:
            if (m_holdLength != 0) {
                res = static_cast<FType>(1);
                break;
            }
        case Envelop::EnvelopState::Decay:
            if (m_decayLength != 0) {
                res = static_cast<FType>(1) + (sustainLevel - static_cast<FType>(1))
                    * static_cast<FType>(m_currentPosition) / static_cast<FType>(m_decayLength);
                break;
            }
        case Envelop::EnvelopState::Sustain:
            res = sustainLevel;
            break;
        case Envelop::EnvelopState::Release:
            if (m_releaseLength != 0) {
                res = sustainLevel - sustainLevel
                    * static_cast<FType>(m_currentPosition) / static_cast<FType>(m_releaseLength);
            }

            break;
        default:
            jassertfalse;
    }

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

    return res;
}

void Envelop::trigger(int noteOnOrOff) {
    m_currentPosition = 0;

    if (noteOnOrOff) {
        m_currentState = EnvelopState::Attack;
    } else {
        if (m_currentState != EnvelopState::Init) {
            m_currentState = EnvelopState::Release;
        }
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