/*
  ==============================================================================

    concepts.h
    Created: 23 Jul 2023 10:21:30pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#ifndef RPSYNTH_CONCEPTS_H
#define RPSYNTH_CONCEPTS_H

#include <type_traits>
#include <cmath>
#include <JuceHeader.h>
#include "synthesizer/types.h"

namespace rpSynth {
template<typename Type>
static Type semitoneToHertz(Type semitone, Type frequencyOfA = static_cast<Type>(440)) noexcept {
    return frequencyOfA * std::pow(static_cast<Type>(2), (semitone - static_cast<Type>(69)) / static_cast<Type>(12));
}

template<typename Type>
static Type hertzToSemitone(Type hz, Type frequencyOfA = static_cast<Type>(440)) noexcept {
    jassert(hz > 0);
    return static_cast<Type>(69) + static_cast<Type>(12) * std::log2(hz / frequencyOfA);
}

inline static const rpSynth::audio::FType kStOf20hz = rpSynth::hertzToSemitone(20.f);
inline static const rpSynth::audio::FType kStOf20000hz = rpSynth::hertzToSemitone(20000.f);

inline static const struct MyStrings {
    const juce::String kXMLConfigTag = "RPBasicSynthesizer";
    const juce::String kAPVTSParameterTag = "RPBasicSynthesizerParameters";
    const juce::String kLineGeneratorTag = "LineGenerator";
    const juce::String kModulationSettingsTag = "ModulationSettings";
    const juce::String kParameterLinkTag = "ParamLink";
    const juce::String kLineGeneratorNumPointTag = "NumPoints";
    const juce::String kLineGeneratorPointTag = "Point";
}g_myStrings;
}

#endif // !RPSYNTH_CONCEPTS_H
