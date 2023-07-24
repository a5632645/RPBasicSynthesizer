/*
  ==============================================================================

    LFO.h
    Created: 24 Jul 2023 2:51:45pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#ifndef RPSYNTH_MODULATION_LFO_H
#define RPSYNTH_MODULATION_LFO_H
namespace rpSynth {
template<typename Type>
class LFO {
public:
    void processFromTo(size_t beginSamplePos, size_t numSamples);
    Type get(size_t index);
private:
    Type m_sampleRate;
    std::vector<Type> m_buffer;
};
}
#endif // !RPSYNTH_MODULATION_LFO_H
