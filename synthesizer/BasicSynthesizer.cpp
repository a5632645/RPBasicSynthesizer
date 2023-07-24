/*
  ==============================================================================

    BasicSynthesizer.cpp
    Created: 24 Jul 2023 9:27:18am
    Author:  mana

  ==============================================================================
*/

#include "BasicSynthesizer.h"

namespace rpSynth {
template<FloatingData SampleType>
inline void BasicSynthesizer<SampleType>::prepare(SampleType sampleRate,
                                                  size_t numSamplesPerBlock) {
    // init oscillor
    for (SingeNoteOscillor<SampleType>& osc : m_SingleNoteOscillors) {
        osc.prepare(sampleRate, numSamplesPerBlock);
        osc.m_parentSynth = this;
    }
}

template<FloatingData SampleType>
inline void BasicSynthesizer<SampleType>::processBlock(juce::MidiBuffer & midiInputBuffer,
                                                       juce::AudioBuffer<SampleType>& audioOutputBuffer) {
    if (midiInputBuffer.getNumEvents() != 0) {
        juce::ignoreUnused(0);
    }

    size_t totalNumSamples = audioOutputBuffer.getNumSamples();
    size_t currentSample = 0;

    for (auto midiEvent : midiInputBuffer) {
        auto midiMessage = midiEvent.getMessage();
        /*
        *   if this midi event is too close to last midi event,no matter what it is,do not
        * do render process,just handle it.
        *   for others,you need render it first and then handle this midi event.
        */
        if (midiEvent.samplePosition - currentSample >= kMinMidiEventIntervalTimeInSamples) {
            size_t eventPosition = midiEvent.samplePosition;
            size_t numSamples = eventPosition - currentSample;
            processBlock(audioOutputBuffer, currentSample, numSamples);
            currentSample = eventPosition;
        }

        handleMidiMessage(midiMessage, midiEvent.samplePosition);
    }

    processBlock(audioOutputBuffer, currentSample, totalNumSamples - currentSample);
}

template<FloatingData SampleType>
inline void BasicSynthesizer<SampleType>::processBlock(juce::AudioBuffer<SampleType>& audioOutputBuffer,
                                                       size_t startSamplePos,
                                                       size_t numSamples) {

    for (auto& monoOsc : m_SingleNoteOscillors) {
        monoOsc.addToBlock(audioOutputBuffer, startSamplePos, numSamples);
    }
}

template<FloatingData SampleType>
inline void BasicSynthesizer<SampleType>::handleMidiMessage(const juce::MidiMessage & message,
                                                            size_t position) {
    if (message.isNoteOn()) {
        noteOn(message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
    } else if (message.isNoteOff()) {
        noteOff(message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
    } else if (message.isPitchWheel()) {
        auto amount = message.getPitchWheelValue() / static_cast<SampleType>(2048);
        m_midiControllerValues.pitchBend.setTargetValue(amount);
    }
}

template<FloatingData SampleType>
void rpSynth::BasicSynthesizer<SampleType>::noteOn(int channel, int noteNumber, float velocity) {
    for (SingleOscillor& osc : m_SingleNoteOscillors) {
        if (osc.isPlayingNote(channel, noteNumber)) {
            osc.stopVoice(velocity, true);
        }
    }

    for (size_t i = 0; i < kMaxPolyphonic; i++) {
        SingleOscillor& osc = m_SingleNoteOscillors[m_roundRobinPosition];
        // find a free oscillor to play
        if (!osc.isPlaying()) {
            osc.startVoice(channel, noteNumber, velocity);
            m_roundRobinPosition++;
            m_roundRobinPosition %= kMaxPolyphonic;
            return;
        }
    }

    // all is working,just replace current oscillor
    SingleOscillor& osc = m_SingleNoteOscillors[m_roundRobinPosition];
    osc.stopVoice(velocity, false);
    osc.startVoice(channel, noteNumber, velocity);
    m_roundRobinPosition++;
    m_roundRobinPosition %= kMaxPolyphonic;
}

template<FloatingData SampleType>
void rpSynth::BasicSynthesizer<SampleType>::noteOff(int channel, int noteNumber, float velocity) {
    for (SingleOscillor& osc : m_SingleNoteOscillors) {
        if (osc.isPlayingNote(channel, noteNumber)) {
            osc.stopVoice(velocity, true);
        }
    }
}
}