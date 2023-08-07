/*
  ==============================================================================

    LineGenerator.h
    Created: 27 Jul 2023 9:54:33pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <vector>
#include "../../concepts.h"
#include <JuceHeader.h>

namespace rpSynth::audio {
class LineGenerator {
public:
    struct PointData {
        FType xPosition{};    // dot in x position: between 0 and 1
        FType yValue{};       // dot in y position: between 0 and 1
        //FType powerAmount{};  // power between two dots: between 0 and 1
    };

    LineGenerator();

    // init
    void initSawUp();
    void initSawDown();
    void initOnePeak();
    void initTriangle();
    void initSquare();

    // operator
    size_t addBefore(PointData newVal);
    void remove(size_t index);
    void set(size_t index, PointData newVal);
    PointData get(size_t index) const;
    size_t getNumPoints() const;
    bool isStateChanged() const;

    void render(FType* pBuffer, size_t bufferSize);

    // state
    void saveState(juce::XmlElement& xml);
    void loadState(juce::XmlElement& xml);
private:
    std::vector<PointData> m_pointDatas{};
    bool m_bPointChanged{};
    juce::SpinLock m_spinLock;
};
}