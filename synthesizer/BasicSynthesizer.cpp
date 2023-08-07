/*
  ==============================================================================

    BasicSynthesizer.cpp
    Created: 24 Jul 2023 9:27:18am
    Author:  mana

  ==============================================================================
*/

#include "BasicSynthesizer.h"

namespace rpSynth::audio {
//=============================================================================
// When a audio processor added
// You must add it to next 4 methods
void BasicSynthesizer::prepare(FType sampleRate,
                               size_t numSamplesPerBlock) {
    m_polyOscillor.prepare(sampleRate, numSamplesPerBlock);
    m_LFOModulationManager.prepare(sampleRate, numSamplesPerBlock);
    m_EnvModulationManager.prepare(sampleRate, numSamplesPerBlock);
    m_filter.prepare(sampleRate, numSamplesPerBlock);
}

void BasicSynthesizer::addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    m_LFOModulationManager.addParameterToLayout(layout);
    m_EnvModulationManager.addParameterToLayout(layout);
    m_polyOscillor.addParameterToLayout(layout);
    m_filter.addParameterToLayout(layout);
}

void BasicSynthesizer::updateParameters(size_t numSamples) {
    m_LFOModulationManager.updateParameters(numSamples);
    m_EnvModulationManager.updateParameters(numSamples);
    m_polyOscillor.updateParameters(numSamples);
    m_filter.updateParameters(numSamples);
}

void BasicSynthesizer::prepareParameters(FType sampleRate, size_t numSamples) {
    m_polyOscillor.prepareParameters(sampleRate, numSamples);
    m_LFOModulationManager.prepareParameters(sampleRate, numSamples);
    m_EnvModulationManager.prepareParameters(sampleRate, numSamples);
    m_filter.prepareParameters(sampleRate, numSamples);
}

void BasicSynthesizer::saveExtraState(juce::XmlElement& xml) {
    m_LFOModulationManager.saveExtraState(xml);
    m_EnvModulationManager.saveExtraState(xml);
    m_polyOscillor.saveExtraState(xml);
    m_filter.saveExtraState(xml);
}

void BasicSynthesizer::loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& apvts) {
    m_LFOModulationManager.loadExtraState(xml, apvts);
    m_EnvModulationManager.loadExtraState(xml, apvts);
    m_polyOscillor.loadExtraState(xml, apvts);
    m_filter.loadExtraState(xml, apvts);
}
//=============================================================================

// Actually,this will only work on handling midi event
// So we just process modulation and vco
// Filter and fx chain do not need midi event
void BasicSynthesizer::process(size_t beginSamplePos, size_t endSamplePos) {
    // Important.Modulators must first be processed.
    m_LFOModulationManager.process(beginSamplePos, endSamplePos);
    m_EnvModulationManager.process(beginSamplePos, endSamplePos);

    m_polyOscillor.process(beginSamplePos, endSamplePos);
}

BasicSynthesizer::BasicSynthesizer(const juce::String& ID)
    : AudioProcessorBase(ID) {
    // Modulation init
    m_LFOModulationManager.addModulator(std::make_unique<LFO>("LFO1"));
    m_LFOModulationManager.addModulator(std::make_unique<LFO>("LFO2"));
    m_LFOModulationManager.addModulator(std::make_unique<LFO>("LFO3"));
    m_LFOModulationManager.addModulator(std::make_unique<LFO>("LFO4"));

    m_EnvModulationManager.addModulator(std::make_unique<Envelop>("ENV1"));
    m_EnvModulationManager.addModulator(std::make_unique<Envelop>("ENV2"));
    m_EnvModulationManager.addModulator(std::make_unique<Envelop>("ENV3"));
    m_EnvModulationManager.addModulator(std::make_unique<Envelop>("ENV4"));

    // Filter init
    m_filter.addAudioInput(&m_polyOscillor, m_polyOscillor.getOutputBuffer());
}

void BasicSynthesizer::processBlock(juce::MidiBuffer& midiBuffer,
                                    juce::AudioBuffer<FType>& audioBuffer) {
    size_t totalNumSamples = audioBuffer.getNumSamples();

    // First generate all parameter's smooth values into buffer
    updateParameters(totalNumSamples);

    // Then handle midi event
    size_t currentSample = 0;
    for (auto midiEvent : midiBuffer) {
        auto midiMessage = midiEvent.getMessage();
        /*
        *   if this midi event is too close to last midi event,no matter what it is,do not
        * do render process,just handle it.
        *   for others,you need render it first and then handle this midi event.
        */
        process(currentSample, midiEvent.samplePosition);
        handleMidiMessage(midiMessage, currentSample, midiEvent.samplePosition);
        currentSample = midiEvent.samplePosition;
    }

    // Process buffer between last message(or null event) and last sample position
    process(currentSample, totalNumSamples);

    // Directly let filter and effects chain work
    m_filter.process(0, totalNumSamples);
    auto& out = m_filter.getFilterOutput();

    // Copy to output
    audioBuffer.copyFrom(0, 0, out.left.data(), (int)totalNumSamples);
    audioBuffer.copyFrom(1, 0, out.right.data(), (int)totalNumSamples);
}

void BasicSynthesizer::handleMidiMessage(const juce::MidiMessage& message,
                                         size_t lastPosition, size_t position) {
    // note on,note off event will let Oscillor and modulation work
    if (message.isNoteOn()) {
        m_LFOModulationManager.addTrigger(1);
        m_EnvModulationManager.addTrigger(1);
        m_polyOscillor.noteOn(message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
    } else if (message.isNoteOff()) {
        m_LFOModulationManager.addTrigger(0);
        m_EnvModulationManager.addTrigger(0);
        m_polyOscillor.noteOff(message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
    }
}
}