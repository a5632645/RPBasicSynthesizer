/*
  ==============================================================================

    ModulationSetting.h
    Created: 30 Jul 2023 9:34:25am
    Author:  mana

  ==============================================================================
*/

#pragma once

#include "concepts.h"

namespace rpSynth::audio {
class ModulatorBase;
//class MyAudioProcessParameter;
class MyAudioParameter;

class ModulationSettings {
public:
    static constexpr bool kBypass = false;
    static constexpr bool kBipolar = false;
    static constexpr FType kAmount = static_cast<FType>(0.2);

    ModulationSettings() {}
    ModulationSettings(MyAudioParameter* pTarget, ModulatorBase* pModulator)
        : target(pTarget)
        , modulator(pModulator) {
    }

    void invertBypass() {
        bypass = !bypass;
    }

    void invertBipolar() {
        bipolar = !bipolar;
    }

    void setAmount(FType val) {
        amount = juce::jlimit<FType>(-1, 1, val);
    }

    bool bypass = kBypass;
    bool bipolar = kBipolar;
    FType amount = kAmount;
    MyAudioParameter* target = nullptr;
    ModulatorBase* modulator = nullptr;
    MyAudioParameter* targetNew = nullptr;
};
}