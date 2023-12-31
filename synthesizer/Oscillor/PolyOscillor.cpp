/*
  ==============================================================================

    PolyOscillor.cpp
    Created: 27 Jul 2023 9:43:14am
    Author:  mana

  ==============================================================================
*/

#include "PolyOscillor.h"

namespace rpSynth::audio {
void PolyOscillor::prepare(FType sampleRate, size_t numSamples) {
    // Output buffer init here
    m_outputBuffer.left.resize(numSamples, FType{});
    m_outputBuffer.right.resize(numSamples, FType{});

    // Oscillor init here
    for (SingeNoteOscillor& osc : m_SingleNoteOscillors) {
        osc.prepare(sampleRate, numSamples);
        osc.m_parentOscillor = this;
    }
}

void PolyOscillor::process(size_t beginSamplePos, size_t endSamplePos) {
    // clear
    clearBuffer();

    // adding...
    for (SingeNoteOscillor& osc : m_SingleNoteOscillors) {
        osc.addToBlock(m_outputBuffer, beginSamplePos, endSamplePos);
    }

    // Apply volume
    for (; beginSamplePos < endSamplePos; beginSamplePos++) {
        auto level = juce::Decibels::decibelsToGain(m_volumeLevel.get(beginSamplePos),
                                                    static_cast<FType>(-36));
        m_outputBuffer.left[beginSamplePos] *= level;
        m_outputBuffer.right[beginSamplePos] *= level;
    }
}

void PolyOscillor::clearBuffer() {
    std::ranges::fill(m_outputBuffer.left, FType{});
    std::ranges::fill(m_outputBuffer.right, FType{});
}

void PolyOscillor::addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    layout.add(std::make_unique<MyHostedAudioProcessorParameter>(&m_semitone,
                                                                 combineWithID("semitone"),
                                                                 "semitone",
                                                                 juce::NormalisableRange<float>(-48.f, 48.f, 0.1f),
                                                                 0.f));
    layout.add(std::make_unique<MyHostedAudioProcessorParameter>(&m_volumeLevel,
                                                                 combineWithID("volume"),
                                                                 "volume",
                                                                 juce::NormalisableRange<float>(-36.f, 0.f, 0.1f),
                                                                 -12.f));
}

void PolyOscillor::updateParameters(size_t numSamples) {
    m_semitone.updateParameter(numSamples);
    m_volumeLevel.updateParameter(numSamples);
}

void PolyOscillor::prepareParameters(FType sampleRate, size_t numSamples) {
    m_semitone.prepare(sampleRate, numSamples);
    m_volumeLevel.prepare(sampleRate, numSamples);
}

void PolyOscillor::noteOn(int channel, int noteNumber, float velocity) {
    for (SingeNoteOscillor& osc : m_SingleNoteOscillors) {
        if (osc.isPlayingNote(channel, noteNumber)) {
            osc.stopVoice(velocity, true);
        }
    }

    for (size_t i = 0; i < kMaxPolyphonic; i++) {
        SingeNoteOscillor& osc = m_SingleNoteOscillors[m_roundRobinPosition];
        // find a free oscillor to play
        if (!osc.isPlaying()) {
            osc.startVoice(channel, noteNumber, velocity);
            return;
        }

        m_roundRobinPosition++;
        m_roundRobinPosition %= kMaxPolyphonic;
    }

    // all is working,just replace current oscillor
    SingeNoteOscillor& osc = m_SingleNoteOscillors[m_roundRobinPosition];
    osc.stopVoice(velocity, false);
    osc.startVoice(channel, noteNumber, velocity);
    m_roundRobinPosition++;
    m_roundRobinPosition %= kMaxPolyphonic;
}

void PolyOscillor::noteOff(int channel, int noteNumber, float velocity) {
    for (SingeNoteOscillor& osc : m_SingleNoteOscillors) {
        if (osc.isPlayingNote(channel, noteNumber)) {
            osc.stopVoice(velocity, true);
        }
    }
}

StereoBuffer* PolyOscillor::getOutputBuffer() {
    return &m_outputBuffer;
}
}