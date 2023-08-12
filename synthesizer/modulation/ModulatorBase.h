/*
  ==============================================================================

    IModulator.h
    Created: 27 Jul 2023 8:38:08am
    Author:  mana

  ==============================================================================
*/

#pragma once
#ifndef RPSYNTH_MODULATION_IMODULATOR_H
#define RPSYNTH_MODULATION_IMODULATOR_H

#include <vector>
#include "ModulationSetting.h"
#include "synthesizer/WrapParameter.h"
#include "synthesizer/AudioProcessorBase.h"

namespace rpSynth::audio {
//================================================================================
// CR³£Á¿
//================================================================================
static constexpr double kCRSmoothTime = 0.05;// 50ms
static constexpr size_t kControlRate = 400;// 400hz

class ModulatorBase : public AudioProcessorBase {
public:
    using AudioProcessorBase::AudioProcessorBase;

    //=========================================================================
    // interfaces
    virtual void generateData(size_t beginSamplePos, size_t endSamplePos) = 0;
    virtual void prepareExtra(FType sampleRate, size_t numSamples) = 0;
    virtual void noteOn() = 0;
    virtual void noteOff() = 0;
    JUCE_NODISCARD virtual juce::Component* createControlComponent() = 0;
    //=========================================================================

    //=========================================================================
    // implement from AudioProcessorBase
    void prepare(FType sampleRate, size_t numSamples) override {
        m_sampleRate = sampleRate;
        m_outputBuffer.resize(numSamples, FType{});
        prepareExtra(sampleRate, numSamples);
    }

    void process(size_t beginSamplePos, size_t endSamplePos) override {
        // first generate all data
        this->generateData(beginSamplePos, endSamplePos);

        // then add data to parameter buffer
        for (ModulationSettings* set : m_parametersLinked) {
            if (set->bypass) continue;

            auto& buffer = set->target->m_output;
            if (set->bipolar) {
                for (size_t i = beginSamplePos; i < endSamplePos; ++i) {
                    // [0,1] -> [-1,1]
                    FType biAmount = static_cast<FType>(2) * m_outputBuffer[i]
                        - static_cast<FType>(1);
                    buffer[i] += biAmount * set->amount;
                }
            } else {
                for (size_t i = beginSamplePos; i < endSamplePos; ++i) {
                    buffer[i] += m_outputBuffer[i] * set->amount;
                }
            }
        }
    }

    bool hasNoModulationTargets() const {
        return m_parametersLinked.isEmpty();
    }

    /**
     * @brief Add a modulation between this modulator and target parameter.
     *        Notice it will not check if this parameter can be modulated.
     * @param pTarget target parameter
    */
    void addModulation(MyAudioProcessParameter* pTarget) {
        for (auto* link : m_parametersLinked) {
            if (link->target == pTarget) {
                return;
            }
        }

        auto* m = new ModulationSettings(pTarget, this);
        pTarget->modulatorAdded(m);
        m_parametersLinked.add(m);
    }

    /**
     * @brief Notice it will not check if this parameter can be modulated.
     * @param set ?
    */
    void addModulation(ModulationSettings* set) {
        jassert(set != nullptr);
        for (auto* link : m_parametersLinked) {
            if (link->target == set->target) {
                return;
            }
        }

        set->target->modulatorAdded(set);
        m_parametersLinked.add(set);
    }

    void removeModulation(ModulationSettings* pMS) {
        pMS->target->modulatorRemoved(pMS);
        m_parametersLinked.removeObject(pMS);
    }

    void removeAllModulations() {
        for (auto* link : m_parametersLinked) {
            link->target->m_modulationSettings.clear();
        }

        m_parametersLinked.clear(true);
    }

    FType getSampleRate() const {
        return m_sampleRate;
    }

    std::vector<FType>& getOutputBuffer() {
        return m_outputBuffer;
    }

    int getNumModulations() const {
        return m_parametersLinked.size();
    }

    ModulationSettings* getModulationSetting(int index) {
        return m_parametersLinked[index];
    }

    decltype(auto) getAllModulationSettings() {
        return (m_parametersLinked);
    }

    int indexOf(ModulationSettings* s) {
        return m_parametersLinked.indexOf(s);
    }

    void saveState(juce::XmlElement& xml) {
        auto* modulatorXML = xml.createNewChildElement(getProcessorID());
        auto* modulationSettingsXML = modulatorXML->createNewChildElement(g_myStrings.kModulationSettingsTag);
        for (auto* link : getAllModulationSettings()) {
            auto* setXML = modulationSettingsXML->createNewChildElement(g_myStrings.kParameterLinkTag);
            setXML->setAttribute("paramID", link->target->getParameterID());
            setXML->setAttribute("bipolar", link->bipolar);
            setXML->setAttribute("amount", link->amount);
            setXML->setAttribute("bypass", link->bypass);
        }

        saveExtraState(*modulatorXML);
    }

    void loadState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& apvts) {
        auto* thisXML = xml.getChildByName(getProcessorID());
        if (thisXML == nullptr) return;

        auto* modulationSettingsXML = thisXML->getChildByName(g_myStrings.kModulationSettingsTag);
        if (modulationSettingsXML == nullptr) return;

        for (auto* link : modulationSettingsXML->getChildWithTagNameIterator(g_myStrings.kParameterLinkTag)) {
            const auto& paramID = link->getStringAttribute("paramID");
            auto* pParameter = apvts.getParameter(paramID);
            if (auto* pWrapParameter = dynamic_cast<MyHostedAudioProcessorParameter*>(pParameter);
                pWrapParameter != nullptr) {
                ModulationSettings* setting = new ModulationSettings;
                setting->target = pWrapParameter->getMyAudioProcessorParameter();
                setting->modulator = this;
                setting->bipolar = link->getBoolAttribute("bipolar");
                setting->bypass = link->getBoolAttribute("bypass");
                setting->amount = static_cast<FType>(link->getDoubleAttribute("amount"));
                addModulation(setting);
            }
        }

        loadExtraState(*thisXML, apvts);
    }

protected:
    // SR
    FType m_sampleRate{};

    // modulations and output
    juce::OwnedArray<ModulationSettings> m_parametersLinked;
    std::vector<FType> m_outputBuffer;
};
}

#endif // !RPSYNTH_MODULATION_IMODULATOR_H
