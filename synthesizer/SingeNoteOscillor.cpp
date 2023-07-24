/*
  ==============================================================================

    SingeNoteOscillor.cpp
    Created: 24 Jul 2023 9:27:28am
    Author:  mana

  ==============================================================================
*/

#include "BasicSynthesizer.h"
#include "SingeNoteOscillor.h"

namespace rpSynth {
template<FloatingData SampleType>
SingeNoteOscillor<SampleType>::SingeNoteOscillor() {
    auto sinWave = [](SampleType negativePiToPositivePi) {
        return negativePiToPositivePi / juce::MathConstants<SampleType>::pi;
    };

    m_testVoice.initialise(sinWave, 1024);
}

template<FloatingData SampleType>
inline void SingeNoteOscillor<SampleType>::prepare(SampleType sampleRate, size_t numSamplesPerBlock) {
    m_sampleRate = sampleRate;

    m_testVoice.prepare(juce::dsp::ProcessSpec{sampleRate,(uint32_t)numSamplesPerBlock,2});

    m_frequencyFromMidi.resize(numSamplesPerBlock, SampleType{});
    m_velocityFromMidi.resize(numSamplesPerBlock, SampleType{});
}

template<FloatingData SampleType>
inline void SingeNoteOscillor<SampleType>::startVoice(int channel, int noteNumber, float velocity) {
    m_noteNumber = noteNumber;
    m_velocity = velocity;
}

template<FloatingData SampleType>
inline void SingeNoteOscillor<SampleType>::stopVoice(float velocity, bool allowTail) {
    m_velocity = velocity;
    m_noteNumber = -1;
}

template<FloatingData SampleType>
inline void SingeNoteOscillor<SampleType>::addToBlock(juce::AudioBuffer<SampleType>& audioOutputBuffer,
                                                      size_t startSamplePos,
                                                      size_t numSamples) {
    // do nothing because this oscillor is not work
    if (m_noteNumber < 0) return;

    // memory prepare
    auto* leftPtr = audioOutputBuffer.getWritePointer(0);
    auto* rightPtr = audioOutputBuffer.getWritePointer(1);

    // calculate frequency
    size_t position = startSamplePos;

    for (size_t i = 0; i < numSamples; i++, position++) {
        auto semitone = m_noteNumber + m_parentSynth->m_midiControllerValues.pitchBend.getNextValue();
        auto hz = getMidiNoteInHertz(semitone);
        m_testVoice.setFrequency(hz, true);
        auto output = m_testVoice.processSample(static_cast<SampleType>(0));
        leftPtr[position] += output * m_velocity;
        rightPtr[position] += output * m_velocity;
    }
}

template<FloatingData SampleType>
inline bool SingeNoteOscillor<SampleType>::isPlayingNote(int channel, int noteNumber) {
    return noteNumber == m_noteNumber;
}

template<FloatingData SampleType>
bool SingeNoteOscillor<SampleType>::isPlaying() {
    return m_noteNumber != -1;
}
}