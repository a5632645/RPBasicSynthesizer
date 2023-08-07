/*
  ==============================================================================

    BasicSynthesizer.h
    Created: 23 Jul 2023 9:44:38pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#ifndef RPSYNTH_BASICSYNTHESIZER_H
#define RPSYNTH_BASICSYNTHESIZER_H

#include <JuceHeader.h>
#include "../concepts.h"

#include "AudioProcessorBase.h"
#include "modulation/ModulationManager.h"
#include "Oscillor/PolyOscillor.h"
#include "Filter/MainFilter.h"

namespace rpSynth::audio {
class PolyOscillor;
}

namespace rpSynth::audio {
class BasicSynthesizer : public AudioProcessorBase {
public:
    BasicSynthesizer(const juce::String& ID);
    ~BasicSynthesizer() override = default;
    /**
     * @brief get the total audio block,call it on juce audio thread's processBlock method
     * @param midiBuffer the total midi buffer
     * @param audioBuffer the total audio buffer
    */
    void processBlock(juce::MidiBuffer& midiInputBuffer, juce::AudioBuffer<FType>& audioOutputBuffer);

    // implement from AudioProcessorBase
    void addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override;
    void updateParameters(size_t numSamples) override;
    void prepareParameters(FType sampleRate, size_t numSamples) override;
    void prepare(FType sampleRate, size_t numSamlpes) override;
    void process(size_t beginSamplePos, size_t endSamplePos) override;
    void saveExtraState(juce::XmlElement& xml) override;
    void loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& apvts) override;
private:

    /**
     * @brief handle a midi message,if midi message just set a value like pitch bend,
              channel touch...etc,return false.
     * @param message midi message
     * @return if true,you need call processBlock,if false,you don't need
    */
    void handleMidiMessage(const juce::MidiMessage& message, size_t lastPosition, size_t position);

public:
    // poly oscillor
    PolyOscillor m_polyOscillor{"OSC1"};

    // filter
    MainFilter m_filter{"FILTER1"};

    // fx chain

    // modulation
    ModulationManager m_LFOModulationManager{"LFOMODULATORS"};
    ModulationManager m_EnvModulationManager{"ENVMODULATORS"};
};
}

#endif // !RPSYNTH_BASICSYNTHESIZER_H
