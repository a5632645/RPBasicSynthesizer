/*
  ==============================================================================

    AudioParameter.h
    Created: 15 Jul 2023 7:01:20pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace parameter {
#define STATIC_JUCE_STRING inline static juce::String
#define STATIC_FLOAT inline static float

#define AP_ID STATIC_JUCE_STRING ID =
#define AP_NAME STATIC_JUCE_STRING Name =
#define AP_MIN STATIC_FLOAT minValue =
#define AP_MAX STATIC_FLOAT maxValue =
#define AP_DEFAULT STATIC_FLOAT defaultValue =

template<class T>
struct AudioParameter {
    static auto makeUnique() {
        return std::make_unique<juce::AudioParameterFloat>(
            T::ID,
            T::Name,
            T::minValue,
            T::maxValue,
            T::defaultValue
        );
    }
};

#define STR(X) #X
#define NEW_AUDIO_PARAMETER(ID,NAME,MIN,MAX,DEFAULT) \
struct NAME : public AudioParameter<NAME> { \
AP_ID STR(ID); \
AP_NAME STR(NAME); \
AP_MIN MIN; \
AP_MAX MAX; \
AP_DEFAULT DEFAULT; \
}; \

namespace PhaserFilter {
NEW_AUDIO_PARAMETER(frequency, frequency, 20.f, 20000.f, 440.f)
NEW_AUDIO_PARAMETER(quality, quality, 0.001f, 0.49f, 0.25f)
NEW_AUDIO_PARAMETER(state, state, 0, 16, 6)
NEW_AUDIO_PARAMETER(feedback, feedback, -0.99f, 0.99f, 0.f)
NEW_AUDIO_PARAMETER(polarity, polarity, -1.f, 1.f, 0.f)
};

namespace CICFilter {
NEW_AUDIO_PARAMETER(a, notchAmount, 0, 256, 1)
NEW_AUDIO_PARAMETER(b,feedback,-0.99f,0.99f,0.f)
}

namespace combFilter {
struct Frequency : public AudioParameter<Frequency> {
    AP_ID "frequency";
    AP_NAME "frequency";
    AP_MIN 20.f;
    AP_MAX 20000.f;
    AP_DEFAULT 440.f;
};

struct Polarity : public AudioParameter<Polarity> {
    AP_ID "polarity";
    AP_NAME "polarity";
    AP_MIN - 1.f;
    AP_MAX 1.f;
    AP_DEFAULT 1.f;
};

struct Feedback : public AudioParameter<Feedback> {
    AP_ID "feedback";
    AP_NAME "feedback";
    AP_MIN - 0.99f;
    AP_MAX 0.99f;
    AP_DEFAULT 0.f;
};
};
};