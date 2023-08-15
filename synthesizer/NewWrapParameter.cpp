/*
  ==============================================================================

    NewWrapParameter.cpp
    Created: 13 Aug 2023 9:04:05pm
    Author:  mana

  ==============================================================================
*/

#include "NewWrapParameter.h"
#include "synthesizer/modulation/ModulatorBase.h"
#include "synthesizer/modulation/ModulationSetting.h"

namespace rpSynth::audio {
void MyAudioParameter::onCRClock() {
    // see @MyHostParameter
    jassert(m_JuceParameter != nullptr);

    FType normalizedValue = m_JuceParameter->range.convertTo0to1(m_JuceParameter->get());
    for (ModulationSettings* m : m_linkedModulators) {
        if (m->bypass) continue;
        FType modulatorOut = m->modulator->getOutputValue();
        if (m->bipolar) {
            modulatorOut = static_cast<FType>(2) * modulatorOut - static_cast<FType>(1);
        }

        FType finalOut = modulatorOut * m->amount;
        normalizedValue += finalOut;
    }

    m_normalizedTarget = juce::jlimit<FType>(0, 1, normalizedValue);
    m_valueTarget = m_JuceParameter->range.convertFrom0to1(m_normalizedTarget);
    m_parameterSmoother.setTargetValue(m_valueTarget);
}

void MyAudioParameter::modulationRemoved(ModulatorBase* pM) {
    std::erase_if(m_linkedModulators, [=](ModulationSettings* set) {
        return set->modulator == pM;
    });
}

ModulationSettings* MyAudioParameter::getModulationSetting(ModulatorBase* m) const {
    for (ModulationSettings* pSet : m_linkedModulators) {
        if (pSet->modulator == m) {
            return pSet;
        }
    }

    return nullptr;
}
}