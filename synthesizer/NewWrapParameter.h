/*
  ==============================================================================

    NewWrapParameter.h
    Created: 12 Aug 2023 8:45:46pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "concepts.h"

namespace rpSynth::audio {
class ModulationSettings;
class ModulatorBase;

class MyAudioParameter {
public:
    static constexpr double kSmoothTime = 0.05;// 50ms
    FType getTargetValue() const {
        return m_valueTarget;
    }

    FType getNormalizedTargetValue() const {
        return m_normalizedTarget;
    }

    FType getNextValue() {
        return m_parameterSmoother.getNextValue();
    }

    void prepare(FType sampleRate) {
        m_parameterSmoother.reset(sampleRate, kSmoothTime);
    }

    void onCRClock();
    //================================================================================
    // 和JUCE有关的
    //================================================================================
    const juce::String& getParameterID() const {
        // see @MyHostParameter
        jassert(m_JuceParameter != nullptr);
        return m_JuceParameter->paramID;
    }

    juce::AudioParameterFloat* getHostParameter() {
        jassert(m_JuceParameter != nullptr);
        return m_JuceParameter;
    }

    decltype(auto) getParameterRange() {
        jassert(m_JuceParameter != nullptr);
        return (m_JuceParameter->range);
    }

    //================================================================================
    // 和Modulation有关的
    //================================================================================
    void modulationAdded(ModulationSettings* pSet) {
        if (pSet == nullptr) return;
        m_linkedModulators.emplace_back(pSet);
    }

    void modulationRemoved(ModulationSettings* pSet) {
        std::erase(m_linkedModulators, pSet);
    }

    void modulationRemoved(ModulatorBase* pM);

    bool hasModulation() const {
        return !m_linkedModulators.empty();
    }

    bool canBeModulated() const {
        return true;
    }

    ModulationSettings* getModulationSetting(ModulatorBase* m) const;
private:
    friend class MyHostParameter;
    std::vector<ModulationSettings*> m_linkedModulators;

    FType m_normalizedTarget{};
    FType m_valueTarget{};
    juce::SmoothedValue<FType> m_parameterSmoother;
    juce::AudioParameterFloat* m_JuceParameter = nullptr;
};

class MyHostParameter : public juce::AudioParameterFloat {
public:
    MyHostParameter(MyAudioParameter& parameter,
                    const juce::ParameterID& parameterID,
                    const juce::String& parameterName,
                    juce::NormalisableRange<float> normalisableRange,
                    float defaultValue,
                    const juce::AudioParameterFloatAttributes& attributes = {})
        : juce::AudioParameterFloat(parameterID,
                                    parameterName,
                                    normalisableRange,
                                    defaultValue,
                                    attributes)
    , m_audioParameter(parameter){
        parameter.m_JuceParameter = this;
    }

    MyAudioParameter* getAudioParameter() const {
        return std::addressof(m_audioParameter);
    }
private:
    MyAudioParameter& m_audioParameter;
};

//================================================================================
// Ò»¸öÓÃÓÚ½â¾öPitchµ½HertzÒÔ¼°·´ÏòµÄFloatAttribute
//================================================================================
inline static const juce::AudioParameterFloatAttributes g_PitchHertzFloatParameterAttribute
= [] {
    return juce::AudioParameterFloatAttributes()
        .withStringFromValueFunction([](float v, int) {
        return juce::String(semitoneToHertz(v));
    }).withValueFromStringFunction([](const juce::String& s) {
        return hertzToSemitone(juce::jmax(s.getFloatValue(), 0.001f));
    });
}();
}
