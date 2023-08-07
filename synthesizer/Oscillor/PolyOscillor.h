/*
  ==============================================================================

    PolyOscillor.h
    Created: 27 Jul 2023 9:37:43am
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <array>
#include "../../concepts.h"
#include "SingeNoteOscillor.h"
#include "../WrapParameter.h"
#include "../AudioProcessorBase.h"

namespace rpSynth::audio {
class PolyOscillor : public AudioProcessorBase {
public:
    static constexpr size_t kMaxPolyphonic = 8;

    using AudioProcessorBase::AudioProcessorBase;

    void clearBuffer();
    void noteOn(int channel, int noteNumber, float velocity);
    void noteOff(int channel, int noteNumber, float velocity);

    StereoBuffer* getOutputBuffer();

    // implement from AudioProcessorBase
    void addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;
    void updateParameters(size_t numSamples) override;
    void prepareParameters(FType sampleRate, size_t numSamples) override;
    void prepare(FType sampleRate, size_t numSamlpes) override;
    void process(size_t beginSamplePos, size_t endSamplePos) override;
    void saveExtraState(juce::XmlElement& /*xml*/) override {};
    void loadExtraState(juce::XmlElement& /*xml*/, juce::AudioProcessorValueTreeState& /*apvts*/) override {};
private:
    friend class SingeNoteOscillor;

    // oscillors
    std::array<SingeNoteOscillor, kMaxPolyphonic> m_SingleNoteOscillors;
    size_t m_roundRobinPosition = 0;

    // buffer
    StereoBuffer m_outputBuffer;
public:
    // Parameters
    MyAudioProcessParameter m_semitone;
    MyAudioProcessParameter m_volumeLevel;
};
}