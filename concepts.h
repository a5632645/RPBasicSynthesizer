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

namespace rpSynth {
template<typename Type>
concept FloatingData = std::is_floating_point_v<Type>;

static constexpr size_t kMaxPolyphonic = 8;
static constexpr int kMinMidiEventIntervalTimeInSamples = 8;

template<FloatingData Type>
static Type getMidiNoteInHertz(Type semitone, Type frequencyOfA = static_cast<Type>(440)) noexcept {
    return frequencyOfA * std::pow(static_cast<Type>(2), (semitone - static_cast<Type>(69)) / static_cast<Type>(12));
}

template<std::random_access_iterator RandomIterator,typename Type>
static void interpole(RandomIterator container,
                      size_t beginIndex, Type valueBegin,
                      size_t endIndex, Type valueEnd) {
    Type interval = (valueEnd - valueBegin) / (endIndex - beginIndex);
    Type x = valueBegin;

    for (size_t i = beginIndex; i < endIndex; i++) {
        container[i] = x;
        x += interval;
    }
}
}

#endif // !RPSYNTH_CONCEPTS_H
