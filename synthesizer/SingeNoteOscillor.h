/*
  ==============================================================================

    SingeNoteOscillor.h
    Created: 24 Jul 2023 9:08:24am
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "../concepts.h"

namespace rpSynth {
template<FloatingData SampleType>
class BasicSynthesizer;

template<FloatingData SampleType>
class SingeNoteOscillor {
public:
    SingeNoteOscillor();
    void prepare(SampleType sampleRate, size_t numSamplesPerBlock);
    void startVoice(int channel, int noteNumber, float velocity);
    void stopVoice(float velocity,bool allowTail);
    void addToBlock(juce::AudioBuffer<SampleType>& audioOutputBuffer,
                    size_t startSamplePos, size_t numSamples);

    bool isPlayingNote(int channel, int noteNumber);
    bool isPlaying();
private:
    // parent synthesize to read midi buffer
    friend class BasicSynthesizer<SampleType>;
    BasicSynthesizer<SampleType>* m_parentSynth;

    // internal datas
    std::vector<SampleType> m_frequencyFromMidi;
    std::vector<SampleType> m_velocityFromMidi;
    int m_noteNumber = -1;
    float m_velocity = 0.f;
    SampleType m_sampleRate;

    // oscVoices
    juce::dsp::Oscillator<SampleType> m_testVoice;
};

}