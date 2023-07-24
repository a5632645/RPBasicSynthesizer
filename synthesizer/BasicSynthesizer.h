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

#include "SingeNoteOscillor.h"

namespace rpSynth {
template<FloatingData SampleType>
class BasicSynthesizer {
public:
    struct MidiControllers {
        juce::SmoothedValue<SampleType> pitchBend;
    };

    void prepare(SampleType sampleRate, size_t numSamplesPerBlock);
    /**
     * @brief get the total audio block,call it on juce audio thread's processBlock method
     * @param midiBuffer the total midi buffer
     * @param audioBuffer the total audio buffer
    */
    void processBlock(juce::MidiBuffer& midiInputBuffer, juce::AudioBuffer<SampleType>& audioOutputBuffer);

private:
    using SingleOscillor = SingeNoteOscillor<SampleType>;
    /**
     * @brief let the synthesizer work for only a short time,must be called with
              right parameter and after a midi event processed!
     * @param audioBuffer the audio output buffer
     * @param startSamplePos the start sample where current midi event processed
     * @param numSamples the number of sample,equals to the diff between next midi event
              position and current midi event position
    */
    void processBlock(juce::AudioBuffer<SampleType>& audioOutputBuffer,
                      size_t startSamplePos, size_t numSamples);

    /**
     * @brief handle a midi message,if midi message just set a value like pitch bend,
              channel touch...etc,return false.
     * @param message midi message
     * @return if true,you need call processBlock,if false,you don't need
    */
    void handleMidiMessage(const juce::MidiMessage& message, size_t position);

    /**
     * @brief start a voice
     * @param channel the channel of midi note on event,from 0 to 15
     * @param noteNumber note number,from 0 to 127
     * @param velocity normalized velocity,from 0 to 1
    */
    void noteOn(int channel, int noteNumber, float velocity);

    /**
     * @brief stop a voice
     * @param channel the channel of midi note off event,from 0 to 15
     * @param noteNumber note number,from 0 to 127
     * @param velocity normalized velocity,from 0 to 1
    */
    void noteOff(int channel, int noteNumber, float velocity);

    // TODO:other midi event

private:
    // polyphonic oscillors
    friend class SingeNoteOscillor<SampleType>;
    std::array<SingleOscillor, kMaxPolyphonic> m_SingleNoteOscillors;
    size_t m_roundRobinPosition = 0;

    // some midi buffers
    MidiControllers m_midiControllerValues;
};
}

#endif // !RPSYNTH_BASICSYNTHESIZER_H
