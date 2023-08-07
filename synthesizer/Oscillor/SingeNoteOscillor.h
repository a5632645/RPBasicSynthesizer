/*
  ==============================================================================

    SingeNoteOscillor.h
    Created: 24 Jul 2023 9:08:24am
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

#include "../../concepts.h"

namespace rpSynth::audio {
class PolyOscillor;

class SingeNoteOscillor {
public:
    SingeNoteOscillor();
    void prepare(FType sampleRate, size_t numSamplesPerBlock);
    void startVoice(int channel, int noteNumber, float velocity);
    void stopVoice(float velocity, bool allowTail);
    void addToBlock(StereoBuffer& outputBuffer,
                    size_t beginSamplePos, size_t endSamplePos);

    bool isPlayingNote(int channel, int noteNumber);
    bool isPlaying();
private:
    // parent synthesize to read midi buffer
    friend class PolyOscillor;
    PolyOscillor* m_parentOscillor = nullptr;

    // internal datas
    int m_noteNumber = -1;
    float m_velocity = 0.f;
    FType m_sampleRate{};

    // oscVoices
    juce::dsp::Oscillator<FType> m_testVoice;
};

}