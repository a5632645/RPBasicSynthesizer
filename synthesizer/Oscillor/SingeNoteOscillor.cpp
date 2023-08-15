/*
  ==============================================================================

    SingeNoteOscillor.cpp
    Created: 24 Jul 2023 9:27:28am
    Author:  mana

  ==============================================================================
*/

#include "PolyOscillor.h"
#include "SingeNoteOscillor.h"

namespace rpSynth::audio {
SingeNoteOscillor::SingeNoteOscillor() {
    auto sinWave = [](FType negativePiToPositivePi) {
        return negativePiToPositivePi / juce::MathConstants<FType>::pi;
    };

    m_testVoice.initialise(sinWave, 1024);
}

void SingeNoteOscillor::prepare(FType sampleRate, size_t numSamplesPerBlock) {
    m_sampleRate = sampleRate;

    m_testVoice.prepare(juce::dsp::ProcessSpec{sampleRate,(uint32_t)numSamplesPerBlock,2});
}

void SingeNoteOscillor::startVoice(int /*channel*/, int noteNumber, float velocity) {
    m_noteNumber = noteNumber;
    m_velocity = velocity;
}

void SingeNoteOscillor::stopVoice(float velocity, bool /*allowTail*/) {
    m_velocity = velocity;
    m_noteNumber = -1;
}

void SingeNoteOscillor::addToBlock(StereoBuffer& outputBuffer,
                                                      size_t beginSamplePos,
                                                      size_t endSamplePos) {
    // do nothing because this oscillor is not work
    if (m_noteNumber < 0) return;

    // calculate frequency
    for (; beginSamplePos < endSamplePos; beginSamplePos++) {
        auto semitone = m_noteNumber
            + m_parentOscillor->m_semitone.getNextValue();
        auto hz = semitoneToHertz(semitone);
        m_testVoice.setFrequency(hz, true);
        auto output = m_testVoice.processSample(static_cast<FType>(0));
        outputBuffer.buffer[beginSamplePos].left += output * m_velocity;
        outputBuffer.buffer[beginSamplePos].right += output * m_velocity;
    }
}

bool SingeNoteOscillor::isPlayingNote(int /*channel*/, int noteNumber) {
    return noteNumber == m_noteNumber;
}

bool SingeNoteOscillor::isPlaying() {
    return m_noteNumber != -1;
}
}