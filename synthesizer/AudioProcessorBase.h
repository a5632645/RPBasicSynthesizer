/*
  ==============================================================================

    AudioProcessorBase.h
    Created: 31 Jul 2023 10:36:03pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "WrapParameter.h"

namespace rpSynth::audio {
class AudioProcessorBase {
public:
    AudioProcessorBase(const juce::String& ID) :m_audioProcessorID(ID) {}
    const juce::String& getProcessorID() const { return m_audioProcessorID; }
    
    /**
     * @brief Add id before this text.For example: ID = "a",text = "b",then return "a_b"
     * @param text The text
     * @return with An id before text
    */
    juce::String combineWithID(const juce::String& text) { return m_audioProcessorID + "_" + text; }

    virtual ~AudioProcessorBase() = default;

    virtual void addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) = 0;
    virtual void updateParameters(size_t numSamples) = 0;
    virtual void prepareParameters(FType sampleRate, size_t numSamples) = 0;
    virtual void prepare(FType sampleRate, size_t numSamlpes) = 0;
    virtual void process(size_t beginSamplePos, size_t endSamplePos) = 0;
    virtual void saveExtraState(juce::XmlElement& xml) = 0;
    virtual void loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& apvts) = 0;
private:
    juce::String m_audioProcessorID;
    
};
}