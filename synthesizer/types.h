/*
  ==============================================================================

    StereoBuffer.h
    Created: 5 Aug 2023 8:33:00pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace rpSynth::audio {
using FType = float;

struct PolyFType {
    FType left{};
    FType right{};

    PolyFType operator+(const PolyFType& f) {
        return PolyFType{left + f.left,right + f.right};
    }

    PolyFType operator-(const PolyFType& f) {
        return PolyFType{left - f.left,right - f.right};
    }

    PolyFType operator*(const PolyFType& f) {
        return PolyFType{left * f.left,right * f.right};
    }

    PolyFType operator/(const PolyFType& f) {
        return PolyFType{left / f.left,right / f.right};
    }
};

template<typename T>
PolyFType operator+(PolyFType l, T r) {
    return PolyFType{l.left + r,l.right + r};
}
template<typename T>
PolyFType operator-(PolyFType l, T r) {
    return PolyFType{l.left - r,l.right - r};
}
template<typename T>
PolyFType operator*(PolyFType l, T r) {
    return PolyFType{l.left * r,l.right * r};
}
template<typename T>
PolyFType operator/(PolyFType l, T r) {
    return PolyFType{l.left / r,l.right / r};
}

template<typename T>
PolyFType operator+(T l, PolyFType r) {
    return PolyFType{l + r.left,l + r.right};
}
template<typename T>
PolyFType operator-(T l, PolyFType r) {
    return PolyFType{l - r.left,l - r.right};
}
template<typename T>
PolyFType operator*(T l, PolyFType r) {
    return PolyFType{l * r.left,l * r.right};
}
template<typename T>
PolyFType operator/(T l, PolyFType r) {
    return PolyFType{l / r.left,l / r.right};
}

// stereo buffer
struct StereoBuffer {
    std::vector<PolyFType> buffer;

    void clear() {
        std::ranges::fill(buffer, PolyFType{});
    }

    void resize(size_t size) {
        buffer.resize(size, PolyFType{});
    }

    // operators
    PolyFType& operator[](size_t i) {
        return buffer[i];
    }

    void addFrom(StereoBuffer& other,size_t begin,size_t end) {
        size_t num = 2 * (end - begin);
        juce::FloatVectorOperations::add(reinterpret_cast<FType*>(buffer.data() + begin),
                                         reinterpret_cast<FType*>(other.buffer.data() + begin),
                                         num);
    }
};

// Non nullptr
template<class T>
struct NonNullPtr {
    T* ptr;

    NonNullPtr(T* p) : ptr(p) { jassert(p != nullptr); }
};
};