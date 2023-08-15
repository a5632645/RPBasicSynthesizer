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
    m_totalNumSamples = static_cast<size_t>(sampleRate / kControlRate);
    m_leftNumSamples = 0;

    m_polyOscillor.prepare(sampleRate, numSamplesPerBlock);
    m_LFOModulationManager.prepare(sampleRate, numSamplesPerBlock);
    m_EnvModulationManager.prepare(sampleRate, numSamplesPerBlock);
    m_filter.prepare(sampleRate, numSamplesPerBlock);
    m_filter2.prepare(sampleRate, numSamplesPerBlock);
    m_fxChain.prepare(sampleRate, numSamplesPerBlock);
}

void BasicSynthesizer::addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    m_LFOModulationManager.addParameterToLayout(layout);
    m_EnvModulationManager.addParameterToLayout(layout);
    m_polyOscillor.addParameterToLayout(layout);
    m_filter.addParameterToLayout(layout);
    m_filter2.addParameterToLayout(layout);
    m_fxChain.addParameterToLayout(layout);
}

//void BasicSynthesizer::updateParameters(size_t numSamples) {
//    m_LFOModulationManager.updateParameters(numSamples);
//    m_EnvModulationManager.updateParameters(numSamples);
//    m_polyOscillor.updateParameters(numSamples);
//    m_filter.updateParameters(numSamples);
//    m_filter2.updateParameters(numSamples);
//    m_fxChain.updateParameters(numSamples);
//}

void BasicSynthesizer::prepareParameters(FType sampleRate, size_t numSamples) {
    m_polyOscillor.prepareParameters(sampleRate, numSamples);
    m_LFOModulationManager.prepareParameters(sampleRate, numSamples);
    m_EnvModulationManager.prepareParameters(sampleRate, numSamples);
    m_filter.prepareParameters(sampleRate, numSamples);
    m_filter2.prepareParameters(sampleRate, numSamples);
    m_fxChain.prepareParameters(sampleRate, numSamples);
}

void BasicSynthesizer::saveExtraState(juce::XmlElement& xml) {
    m_LFOModulationManager.saveExtraState(xml);
    m_EnvModulationManager.saveExtraState(xml);
    m_polyOscillor.saveExtraState(xml);
    m_filter.saveExtraState(xml);
    m_filter2.saveExtraState(xml);
    m_fxChain.saveExtraState(xml);
}

void BasicSynthesizer::loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& apvts) {
    m_LFOModulationManager.loadExtraState(xml, apvts);
    m_EnvModulationManager.loadExtraState(xml, apvts);
    m_polyOscillor.loadExtraState(xml, apvts);
    m_filter.loadExtraState(xml, apvts);
    m_filter2.loadExtraState(xml, apvts);
    m_fxChain.loadExtraState(xml, apvts);
}

void BasicSynthesizer::onCRClock(size_t n) {
    m_LFOModulationManager.onCRClock(n);
    m_EnvModulationManager.onCRClock(n);
    m_polyOscillor.onCRClock(n);
    m_filter.onCRClock(n);
    m_filter2.onCRClock(n);
    m_fxChain.onCRClock(n);
}
//=============================================================================

void BasicSynthesizer::process(size_t beginSamplePos, size_t endSamplePos) {
    for (size_t srBegin = beginSamplePos; srBegin < endSamplePos;) {
        if (m_leftNumSamples == 0) {
            onCRClock(m_totalNumSamples);
            m_leftNumSamples = m_totalNumSamples;
        }

        size_t srEnd = srBegin + m_leftNumSamples;
        srEnd = srEnd > endSamplePos ? endSamplePos : srEnd;
        size_t srNumSamples = srEnd - srBegin;
        m_leftNumSamples -= srNumSamples;
        
        // 真正的调用处理函数在这里
        m_polyOscillor.process(srBegin, srEnd);
        m_filter.process(srBegin, srEnd);
        m_filter2.process(srBegin, srEnd);

        // 再次混合
        auto& fxInput = m_fxChain.getChainBuffer();
        // 先混滤波器
        fxInput.addFrom(*m_filter2.getFilterOutput(), srBegin, srEnd);
        if (m_filter.isOutputNotUsed()) {
            fxInput.addFrom(*m_filter.getFilterOutput(), srBegin, srEnd);
        }
        // 再混振荡器
        if (m_polyOscillor.isOutputNotUsed()) {
            fxInput.addFrom(*m_polyOscillor.getOutputBuffer(), srBegin, srEnd);
        }

        m_fxChain.process(srBegin, srEnd);

        srBegin = srEnd;
    }
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

    // Filter2 init
    m_filter2.addAudioInput(&m_filter, m_filter.getFilterOutput());
    m_filter2.addAudioInput(&m_polyOscillor, m_polyOscillor.getOutputBuffer());
}

void BasicSynthesizer::processBlock(juce::MidiBuffer& midiBuffer,
                                    juce::AudioBuffer<FType>& audioBuffer) {
    size_t totalNumSamples = audioBuffer.getNumSamples();

    // First generate all parameter's smooth values into buffer
    //updateParameters(totalNumSamples);
    // 清除缓存先
    m_polyOscillor.clearBuffer();
    m_filter.clearBuffer();
    m_filter2.clearBuffer();
    m_fxChain.clearBuffer();

    // 清除标志
    m_polyOscillor.clearFlag();
    m_filter.clearFlag();
    m_filter2.clearFlag();
    m_fxChain.clearFlag();

    // Then handle midi event
    size_t currentSample = 0;
    for (auto midiEvent : midiBuffer) {
        auto midiMessage = midiEvent.getMessage();
        process(currentSample, midiEvent.samplePosition);
        handleMidiMessage(midiMessage, currentSample, midiEvent.samplePosition);
        currentSample = midiEvent.samplePosition;
    }

    // Process buffer between last message(or null event) and last sample position
    process(currentSample, totalNumSamples);

    // Copy to output
    StereoBuffer& synthOut = m_fxChain.getChainBuffer();
    float* pLeft = audioBuffer.getWritePointer(0);
    float* pRight = audioBuffer.getWritePointer(1);
    for (int i = 0; PolyFType & s : synthOut.buffer) {
        pLeft[i] = s.left;
        pRight[i] = s.right;
        i++;
    }
}

void BasicSynthesizer::handleMidiMessage(const juce::MidiMessage& message,
                                         size_t /*lastPosition*/, size_t /*position*/) {
    // note on,note off event will let Oscillor and modulation work
    if (message.isNoteOn()) {
        m_LFOModulationManager.noteOn();
        m_EnvModulationManager.noteOn();
        m_polyOscillor.noteOn(message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
    } else if (message.isNoteOff()) {
        m_LFOModulationManager.noteOff();
        m_EnvModulationManager.noteOff();
        m_polyOscillor.noteOff(message.getChannel(), message.getNoteNumber(), message.getFloatVelocity());
    }
}
}