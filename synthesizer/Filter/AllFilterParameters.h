/*
  ==============================================================================

    AllFilterParameters.h
    Created: 6 Aug 2023 8:57:46pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include "synthesizer/NewWrapParameter.h"

namespace rpSynth::audio::filters {
/**
 * @brief 这是所有滤波器的参数并集
 *        它应该被所有滤波器实现共享
*/
struct AllFilterParameters {
    MyAudioParameter cutoff;
    MyAudioParameter resonance;
    MyAudioParameter phase;

    void onCRClock() {
        cutoff.onCRClock();
        resonance.onCRClock();
        phase.onCRClock();
    }

    void prepare(FType sampleRate) {
        cutoff.prepare(sampleRate);
        resonance.prepare(sampleRate);
        phase.prepare(sampleRate);
    }
};
};