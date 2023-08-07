/*
  ==============================================================================

    ModulationManager.h
    Created: 25 Jul 2023 9:39:59am
    Author:  mana

  ==============================================================================
*/

#pragma once
#ifndef RPSYNTH_MODULATION_MODULATIONMANAGER_H
#define RPSYNTH_MODULATION_MODULATIONMANAGER_H

#include <JuceHeader.h>
#include "LFO.h"
#include "Envelop.h"

namespace rpSynth::audio {
class ModulationManager : public AudioProcessorBase {
public:
    static constexpr FType kSmoothTimeInSeconds = 0.005f;// 50ms
    static constexpr FType kControlRate = 400;// 400hz

    using AudioProcessorBase::AudioProcessorBase;
    //=========================================================================
    // Implement from AudioProcessorBase
    void addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) override {
        for (auto& m : m_modulators) {
            m->addParameterToLayout(layout);
        }
    }

    void prepareParameters(FType sampleRate, size_t numSamples) override {
        for (auto& m : m_modulators) {
            m->prepareParameters(sampleRate, numSamples);
        }
    }

    void prepare(FType sampleRate, size_t numSamples) override {
        for (auto& m : m_modulators) {
            m->prepare(sampleRate, numSamples);
            m->setControlRate(kControlRate);
            m->setSmoothTime(kSmoothTimeInSeconds);
        }
    }

    void updateParameters(size_t numSamples) override {
        for (auto& m : m_modulators) {
            m->updateParameters(numSamples);
        }
    }

    void process(size_t beginSamplePos, size_t endSamplePos) override {
        for (auto& m : m_modulators) {
            m->process(beginSamplePos, endSamplePos);
        }
    }

    //=========================================================================
    void addTrigger(int noteOnOrOff) {
        for (auto& m : m_modulators) {
            m->trigger(noteOnOrOff);
        }
    }

    ModulatorBase* getModulator(const juce::String& ID) {
        for (auto& m : m_modulators) {
            if (m->getProcessorID() == ID) {
                jassertfalse;
            }
        }

        return nullptr;
    }

    ModulatorBase* getModulator(size_t index) {
        return m_modulators[index].get();
    }

    size_t getNumModulators() const {
        return m_modulators.size();
    }

    decltype(auto) getAllModulators() { return (m_modulators); }
    //=========================================================================
    // Save Modulator ID, Parameter ID and ModulationSettings to xml
    // Use MyAudioProcessParameter::getParameterID to find parameterID
    void saveExtraState(juce::XmlElement& xml) override {
        auto* modulatorsXML = xml.createNewChildElement(getProcessorID());
        for (auto& m : m_modulators) {
            m->saveState(*modulatorsXML);
        }
    };
    // Load Modulator ID and all it's Parameter ID and ModulationSettings
    // Use MyHostedAudioProcessorParameter::getMyAudioProcessorParameter
    //   to get the address of buildIn processor parameter by parameterID
    void loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& apvts) override {
        auto* thisXML = xml.getChildByName(getProcessorID());
        if (thisXML == nullptr) return;

        for (auto& m : m_modulators) {
            m->removeAllModulations();
            m->loadState(*thisXML, apvts);
        }
    };

    void addModulator(std::unique_ptr<ModulatorBase>&& modulator) {
        for (auto& m : m_modulators) {
            if (m->getProcessorID() == modulator->getProcessorID()) {
                jassertfalse;
            }
        }

        m_modulators.emplace_back(std::move(modulator));
    }
private:
    size_t m_lastTriggerPosition = 0;
    size_t m_numTriggers = 0;
    std::vector<size_t> m_triggerPositions;

    std::vector<std::unique_ptr<ModulatorBase>> m_modulators;
};
};
#endif // !RPSYNTH_MODULATION_MODULATIONMANAGER_H
