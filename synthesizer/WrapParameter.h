/*
  ==============================================================================

    WrapParameter.h
    Created: 24 Jul 2023 3:19:23pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#ifndef RPSYNTH_ULTIL_WRAPPARAMTER
#define RPSYNTH_ULTIL_WRAPPARAMTER

#include <JuceHeader.h>
#include "../concepts.h"
#include "modulation/ModulationSetting.h"

namespace rpSynth {
namespace ui {
class ModulableUIBase;
}

namespace audio {
class ModulatorBase;
class MyHostedAudioProcessorParameter;

/**
 * @brief Use this class in audio stream processor,and when init you should
 *        register a MyHostedAudioProcessorParameter in juce::AudioParameterValueTreeState
 *        with a MyAudioProcessParameter
*/
class MyAudioProcessParameter {
public:
    static constexpr FType kSmoothTimeInSeconds = static_cast<FType>(0.05);// 50ms

    /**
     * @brief construct a parameter for audio thread's processors
     * @param canBeModulated If you don't want this parameter to be modulated,set this to 'false'
    */
    MyAudioProcessParameter(bool canBeModulated = true)
        : m_canBeModulated(canBeModulated) {
    }

    void prepare(FType sampleRate, size_t numSamples) {
        m_smoothedValue.reset(sampleRate, kSmoothTimeInSeconds);
        m_output.resize(numSamples, FType{});
    }

    void updateParameter(size_t numSamples) {
        for (size_t i = 0; i < numSamples; i++) {
            m_output[i] = m_smoothedValue.getNextValue();
        }
    }

    //=========================================================================
    // get

    /**
     * @brief »ñÈ¡Juce::range´Ó[0,1]×ª»»ºóµÄÊý×Ö
     * @param index Ö¸¶¨Î»ÖÃ
     * @return ·µ»ØµÄÊý×ÖÔÚÄã°ó¶¨µÄjuce::RangedAudioParameterµÄ·¶Î§ÄÚ
    */
    inline FType get(size_t index) const;

    /**
     * @brief »ñÈ¡¹éÒ»»¯µÄÖµ
     * @param index Ö¸¶¨Î»ÖÃ
     * @return ¹éÒ»»¯ºóµÄÖµ,·¶Î§ÔÚ[0,1]
    */
    FType getNormalized(size_t index) const {
        return juce::jlimit<FType>(0, 1, m_output[index]);
    }

    inline FType getNormalizedWithNoScrew() const;

    FType getRaw(size_t index) const {
        return m_output[index];
    }
    //=========================================================================

    //=========================================================================
    // set

    /**
     * @brief ÉèÖÃ¹éÒ»»¯µÄÖµ
     * @param val ¿ÉÒÔÊÇÈÎÒâÖµ,×îÖÕÊä³öÊ±»á±»[0,1]½Ø¶Ï
    */
    void set(FType val) {
        m_smoothedValue.setTargetValue(val);
    }
    //=========================================================================

    //================================================================================
    // Processing

    /**
     * @brief ¶Ô»º³åÇøÄÚµÄËùÓÐÊý×Ö´Ó°ëÒô×ª»»µ½ÆµÂÊ,
     *        Çë×¢ÒâÕâ¸ö²Ù×÷ÎÞ·¨³·»Ø,Ö®ºóÄãÓ¦¸ÃÊ¹ÓÃgetRaw·½·¨»ñÈ¡²ÎÊý
    */
    inline void applySemitoneToHertz();

    /**
     * @brief ¶ÔÖ¸¶¨·¶Î§ÄÚ[begin,end)µÄËùÓÐÊý×Ö´Ó°ëÒô×ª»»µ½ÆµÂÊ,Ò²ÊÇÎÞ·¨³·»Ø,Ö®ºóÄãÓ¦¸ÃÊ¹ÓÃgetRaw·½·¨»ñÈ¡²ÎÊý
     * @param begin ¿ªÊ¼
     * @param end ½áÊø
    */
    inline void applySemitoneToHertz(size_t begin, size_t end);
    //================================================================================

    //================================================================================
    // Modulations
    void modulatorAdded(ModulationSettings* pM) {
        jassert(!isModulatorLinked(pM->modulator));
        m_modulationSettings.push_back(pM);
    }

    void modulatorRemoved(ModulationSettings* pM) {
        std::erase(m_modulationSettings, pM);
    }

    bool isModulatorLinked(ModulatorBase* pM) {
        return getModulationSetting(pM) != nullptr;
    }

    audio::ModulationSettings* getModulationSetting(ModulatorBase* source) {
        for (ModulationSettings* m : m_modulationSettings) {
            if (m->modulator == source) {
                return m;
            }
        }

        return nullptr;
    }

    bool hasModulation() const {
        return !m_modulationSettings.empty();
    }

    bool canBeModulated() const {
        return m_canBeModulated;
    }
    //================================================================================
    

    // Must have a hosted parameter if using this class
    MyHostedAudioProcessorParameter* getHostParameter() const { jassert(m_juceAudioParameter != nullptr); return m_juceAudioParameter; }
    inline juce::String getParameterID() const;
private:
    friend class ModulatorBase;
    friend class MyHostedAudioProcessorParameter;

    bool m_canBeModulated;
    juce::SmoothedValue<FType> m_smoothedValue;
    std::vector<FType> m_output;
    std::vector<ModulationSettings*> m_modulationSettings;
    MyHostedAudioProcessorParameter* m_juceAudioParameter = nullptr;
};

class MyHostedAudioProcessorParameter : public juce::AudioParameterFloat {
public:
    MyHostedAudioProcessorParameter(MyAudioProcessParameter* audioProcessorParameter,
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
        , m_audioProcessorParameter(audioProcessorParameter) {
        jassert(audioProcessorParameter != nullptr);
        audioProcessorParameter->m_juceAudioParameter = this;
        // Well,if the value init equal to defaultValue
        // The next valueChanged method won't be called
        // And let buildin audio processor parameters all 0
        // So when init we set the parameter to default
        audioProcessorParameter->set(convertTo0to1(defaultValue));
    }

    void valueChanged(float newValue) override {
        m_audioProcessorParameter->set(convertTo0to1(newValue));
    }

    MyAudioProcessParameter* getMyAudioProcessorParameter() const { return m_audioProcessorParameter; }
private:
    MyAudioProcessParameter* m_audioProcessorParameter;
};

inline FType MyAudioProcessParameter::get(size_t index) const {
    return m_juceAudioParameter->convertFrom0to1(m_output[index]);
}

inline FType MyAudioProcessParameter::getNormalizedWithNoScrew() const {
    auto& range = m_juceAudioParameter->range;
    auto cur = m_smoothedValue.getCurrentValue();
    return juce::jlimit<FType>(0, 1, (cur - range.start) / (range.end - range.start));
}

inline void MyAudioProcessParameter::applySemitoneToHertz() {
    for (auto& v : m_output) {
        v = semitoneToHertz(m_juceAudioParameter->convertFrom0to1(v));
    }
}

inline void MyAudioProcessParameter::applySemitoneToHertz(size_t begin, size_t end) {
    for (; begin < end; begin++) {
        m_output[begin] = semitoneToHertz(m_juceAudioParameter->convertFrom0to1(m_output[begin]));
    }
}

inline juce::String rpSynth::audio::MyAudioProcessParameter::getParameterID() const {
    return getHostParameter()->getParameterID();
}
}
}
#endif // !RPSYNTH_ULTIL_WRAPPARAMTER
