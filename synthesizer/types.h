/*
  ==============================================================================

    StereoBuffer.h
    Created: 5 Aug 2023 8:33:00pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <vector>

namespace rpSynth::audio {
using FType = float;

struct FPolyType {
    FType left;
    FType right;
};

// stereo buffer
struct StereoBuffer {
    std::vector<FType> left;
    std::vector<FType> right;

    void clear() {
        std::ranges::fill(left, FType{});
        std::ranges::fill(right, FType{});
    }

    void resize(size_t size) {
        left.resize(size, FType{});
        right.resize(size, FType{});
    }
};

// Non nullptr
template<class T>
struct NonNullPtr {
    T* ptr;

    NonNullPtr(T* p) : ptr(p) { jassert(p != nullptr); }
};
};