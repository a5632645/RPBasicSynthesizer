/*
  ==============================================================================

    LineGenerator.cpp
    Created: 27 Jul 2023 9:54:33pm
    Author:  mana

  ==============================================================================
*/

#include "LineGenerator.h"
#include <ranges>

rpSynth::audio::LineGenerator::LineGenerator() {
    initOnePeak();
}

void rpSynth::audio::LineGenerator::initSawUp() {
    m_pointDatas.reserve(2);
    m_pointDatas.emplace_back(0.f,0.f);
    m_pointDatas.emplace_back(1.f,1.f);
    m_bPointChanged = true;
}

void rpSynth::audio::LineGenerator::initSawDown() {
    m_pointDatas.reserve(2);
    m_pointDatas.emplace_back(1.f,1.f);
    m_pointDatas.emplace_back(0.f,0.f);
    m_bPointChanged = true;
}

void rpSynth::audio::LineGenerator::initOnePeak() {
    m_pointDatas.reserve(3);
    m_pointDatas.emplace_back(0.f,0.f);
    m_pointDatas.emplace_back(0.5f,1.f);
    m_pointDatas.emplace_back(1.f,0.f);
    m_bPointChanged = true;

}

void rpSynth::audio::LineGenerator::initTriangle() {
    m_pointDatas.reserve(4);
    m_pointDatas.emplace_back(0.f,0.5f);
    m_pointDatas.emplace_back(0.25f,1.f);
    m_pointDatas.emplace_back(0.75f,0.f);
    m_pointDatas.emplace_back(1.0f,0.5f);
    m_bPointChanged = true;
}

void rpSynth::audio::LineGenerator::initSquare() {
    m_pointDatas.reserve(4);
    m_pointDatas.emplace_back(0.f,1.f);
    m_pointDatas.emplace_back(0.5f,1.f);
    m_pointDatas.emplace_back(0.5f,0.f);
    m_pointDatas.emplace_back(1.f,0.f);
    m_bPointChanged = true;
}

size_t rpSynth::audio::LineGenerator::addBefore(PointData newVal) {
    const juce::SpinLock::ScopedLockType lock{m_spinLock};

    auto place = std::ranges::find_if(m_pointDatas,
                                      [&newVal](const PointData& pd) {
        return pd.xPosition >= newVal.xPosition;
    });
    auto ins = m_pointDatas.emplace(place, newVal);
    m_bPointChanged = true;

    return std::distance(m_pointDatas.begin(), ins);
}

void rpSynth::audio::LineGenerator::remove(size_t index) {
    const juce::SpinLock::ScopedLockType lock{m_spinLock};
    // You can not delete last point
    jassert(getNumPoints() > 1);

    m_pointDatas.erase(std::next(m_pointDatas.begin(), index));
    m_bPointChanged = true;
}

void rpSynth::audio::LineGenerator::set(size_t index, PointData newVal) {
    const juce::SpinLock::ScopedLockType lock{m_spinLock};

    m_pointDatas[index] = newVal;
    m_bPointChanged = true;
}

rpSynth::audio::LineGenerator::PointData rpSynth::audio::LineGenerator::get(size_t index) const {
    return m_pointDatas[index];
}

size_t rpSynth::audio::LineGenerator::getNumPoints() const {
    jassert(m_pointDatas.size() > 0);
    return m_pointDatas.size();
}

bool rpSynth::audio::LineGenerator::isStateChanged() const {
    return m_bPointChanged;
}

void rpSynth::audio::LineGenerator::render(FType* pBuffer, size_t bufferSize) {
    const juce::SpinLock::ScopedLockType lock{m_spinLock};
    jassert(getNumPoints() > 0);

    if (getNumPoints() > 1) {
        // fill 0 and first point
        {
            size_t end = static_cast<size_t>(m_pointDatas[0].xPosition * bufferSize);
            std::fill(pBuffer, pBuffer + end, m_pointDatas[0].yValue);
        }

        // fill center
        for (size_t i = 1; i < getNumPoints(); i++) {
            PointData& lastPoint = m_pointDatas[i - 1];
            PointData& currentPoint = m_pointDatas[i];

            size_t begin = static_cast<size_t>(lastPoint.xPosition * bufferSize);
            size_t end = static_cast<size_t>(currentPoint.xPosition * bufferSize);
            FType yInterval = (currentPoint.yValue - lastPoint.yValue)
                / static_cast<FType>(end - begin);
            for (FType val = lastPoint.yValue; begin < end; begin++) {
                pBuffer[begin] = val;
                val += yInterval;
            }
        }

        // fill last point and 1
        {
            PointData& lastPoint = m_pointDatas.back();
            size_t lastPointPos = static_cast<size_t>(lastPoint.xPosition * bufferSize);
            std::fill(pBuffer + lastPointPos, pBuffer + bufferSize, lastPoint.yValue);
        }
    } else {
        std::fill(pBuffer, pBuffer + bufferSize, m_pointDatas[0].yValue);
    }

    m_bPointChanged = false;
}

void rpSynth::audio::LineGenerator::saveState(juce::XmlElement& xml) {
    auto* lineGeneratorXML = xml.createNewChildElement(g_myStrings.kLineGeneratorTag);

    lineGeneratorXML->setAttribute(g_myStrings.kLineGeneratorNumPointTag,
                                   static_cast<int>(m_pointDatas.size()));
    for (const auto& point : m_pointDatas) {
        auto* pointXML = lineGeneratorXML->createNewChildElement(g_myStrings.kLineGeneratorPointTag);
        pointXML->setAttribute("x", point.xPosition);
        pointXML->setAttribute("y", point.yValue);
    }
}

void rpSynth::audio::LineGenerator::loadState(juce::XmlElement& xml) {
    auto* lineGeneratorXML = xml.getChildByName(g_myStrings.kLineGeneratorTag);
    if (lineGeneratorXML == nullptr) return;

    int numPoints = lineGeneratorXML->getIntAttribute(g_myStrings.kLineGeneratorNumPointTag);
    // if numPoints <= 0,line generator can not work
    if (numPoints <= 0) {
        return;
    }

    // load points datas
    m_pointDatas.clear();
    m_pointDatas.reserve(numPoints);
    for (auto* pointXML : 
         lineGeneratorXML->getChildWithTagNameIterator(g_myStrings.kLineGeneratorPointTag)) {
        FType x = static_cast<FType>(pointXML->getDoubleAttribute("x"));
        FType y = static_cast<FType>(pointXML->getDoubleAttribute("y"));
        m_pointDatas.emplace_back(x, y);
    }

    // unfortunely,point data must be sorted by x
    std::ranges::sort(m_pointDatas, [](const PointData& x,const PointData& y) {
        return x.xPosition < y.xPosition;
    });

    // changed flag
    m_bPointChanged = true;
}
