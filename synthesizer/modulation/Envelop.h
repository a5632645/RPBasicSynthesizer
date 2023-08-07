/*
  ==============================================================================

    Envelop.h
    Created: 24 Jul 2023 2:51:56pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include "ModulatorBase.h"
#include "synthesizer/WrapParameter.h"

namespace rpSynth::ui {
class EnvelopPanel;
}

namespace rpSynth::audio {
class Envelop : public ModulatorBase {
public:
    using ModulatorBase::ModulatorBase;
    //===============================================================
    // An envelop implement
    enum class EnvelopState {
        Init,
        Attack,
        Hold,
        Decay,
        Sustain,
        Release
    };
    //===============================================================

    //===============================================================
    // implement from AudioProcessorBase
    void addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;
    void updateParameters(size_t numSamples) override;
    void prepareParameters(FType sampleRate, size_t numSamples) override;
    //===============================================================

    //===============================================================
    // implement from ModulatorBase
    void saveExtraState(juce::XmlElement& xml) override;
    void loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& apvts) override;
    void generateData(size_t beginSamplePos, size_t endSamplePos) override;
    FType onCRClock(size_t intervalSamplesInSR, size_t index) override;
    void trigger(int noteOnOrOff) override;
    JUCE_NODISCARD juce::Component* createControlComponent() override;
    //===============================================================

    //===============================================================
    // some method for ui drawing
    std::pair<EnvelopState, float> getCurrentEnvelopState() const;
    //===============================================================

private:
    EnvelopState m_currentState = EnvelopState::Init;
    size_t m_currentPosition = 0;
    size_t m_attackLength = 0;
    size_t m_holdLength = 0;
    size_t m_decayLength = 0;
    size_t m_releaseLength = 0;
public:
    // Parameters
    MyAudioProcessParameter m_attackInMillSeconds{false};
    MyAudioProcessParameter m_holdInMillSeconds{false};
    MyAudioProcessParameter m_decayInMillSeconds{false};
    MyAudioProcessParameter m_sustainLevelInDecibels{false};
    MyAudioProcessParameter m_releaseInMillSeconds{false};
};
};