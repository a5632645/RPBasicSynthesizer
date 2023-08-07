/*
  ==============================================================================

    LineGeneratorPanel.cpp
    Created: 28 Jul 2023 7:47:36am
    Author:  mana

  ==============================================================================
*/

#include <JuceHeader.h>
#include "LineGeneratorPanel.h"

static constexpr int kHalfPointSize = 6;

rpSynth::ui::LineGeneratorPanel::LineGeneratorPanel(rpSynth::audio::LineGenerator& LG)
    :m_lineGeneratorBind(LG) {
    m_pointControls.ensureStorageAllocated(static_cast<int>(LG.getNumPoints()));

    for (size_t i = 0; i < LG.getNumPoints(); i++) {
        auto* pb = new PointBlock(i);
        pb->setListener(this);
        
        m_pointControls.add(pb);
        addAndMakeVisible(pb);
    }
}

void rpSynth::ui::LineGeneratorPanel::paint(juce::Graphics& g) {
    juce::Path path;
    const float w = static_cast<float>(getWidth() - 2 * kHalfPointSize);
    const float h = static_cast<float>(getHeight() - 2 * kHalfPointSize);
    const float x0 = static_cast<float>(kHalfPointSize);
    const float y0 = static_cast<float>(kHalfPointSize);

    size_t numPoints = m_lineGeneratorBind.getNumPoints();
    size_t lastPoint = numPoints - 1;
    path.preallocateSpace(2 + static_cast<int>(numPoints));

    path.startNewSubPath(0.f,y0 + h - h * m_lineGeneratorBind.get(0).yValue);
    for (size_t i = 0; i < numPoints; i++) {
        path.lineTo(x0 + w * m_lineGeneratorBind.get(i).xPosition,
                    y0 + h - h * m_lineGeneratorBind.get(i).yValue);
    }
    path.lineTo(x0 + w, y0 + h - h * m_lineGeneratorBind.get(lastPoint).yValue);

    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::green);
    g.strokePath(path, juce::PathStrokeType(2.f));
}

void rpSynth::ui::LineGeneratorPanel::resized() {
    const int w = getWidth() - 2 * kHalfPointSize;
    const int h = getHeight() - 2 * kHalfPointSize;
    const size_t numPoints = static_cast<size_t>(m_pointControls.size());

    for (size_t i = 0; i < numPoints; i++) {
        auto pd = m_lineGeneratorBind.get(i);
        m_pointControls[i]->setBounds(static_cast<int>(pd.xPosition * w),
                                      static_cast<int>(h - pd.yValue * h),
                                      kHalfPointSize * 2, kHalfPointSize * 2);
    }
}

void rpSynth::ui::LineGeneratorPanel::mouseDoubleClick(const juce::MouseEvent& e) {
    // double click will create a point
    auto center = e.getEventRelativeTo(this).getPosition().toFloat();
    audio::FType xValue = center.x / getWidth();
    audio::FType yValue = (getHeight() - center.y) / getHeight();
    
    size_t index = m_lineGeneratorBind.addBefore(rpSynth::audio::LineGenerator::PointData{xValue,yValue});
    auto* pb = new PointBlock(index);
    pb->setListener(this);

    m_pointControls.insert(static_cast<int>(index), pb);
    addAndMakeVisible(pb);

    // Increase all point's index behind new point
    for (index++; index < m_lineGeneratorBind.getNumPoints(); index++) {
        m_pointControls[index]->increase();
    }

    resized();
    repaint();
}

void rpSynth::ui::LineGeneratorPanel::pointPressed(PointBlock* pPB, const juce::MouseEvent& e) {
    m_componentDragger.startDraggingComponent(pPB, e);
}

void rpSynth::ui::LineGeneratorPanel::pointDeleted(PointBlock* pPB) {
    // Can not delete last point
    if (m_lineGeneratorBind.getNumPoints() <= 1) return;

    removeChildComponent(pPB);
    size_t index = pPB->getIndex();
    m_lineGeneratorBind.remove(index);
    m_pointControls.remove(index);

    // Decrease all point's index behind deleted point
    for (; index < m_lineGeneratorBind.getNumPoints(); index++) {
        m_pointControls[index]->decrease();
    }

    resized();
    repaint();
}

void rpSynth::ui::LineGeneratorPanel::pointMoved(PointBlock* pPB, const juce::MouseEvent& e) {
    size_t current = pPB->getIndex();
    size_t lastOne = static_cast<size_t>(m_pointControls.size() - 1);
    PointBlock* prevPB = current == 0 ? nullptr : m_pointControls[current - 1];
    PointBlock* nextPB = current == lastOne ? nullptr : m_pointControls[current + 1];
    int leftXLimit = prevPB == nullptr ?  0 : prevPB->getX();
    int rightXLimit = nextPB == nullptr ? getWidth() - 2 * kHalfPointSize : nextPB->getX();
    int upYLimit = 0;
    int downYLimit = getHeight() - 2 *kHalfPointSize;

    m_componentDragger.dragComponent(pPB, e, nullptr);

    pPB->setBounds(juce::jlimit(leftXLimit, rightXLimit, pPB->getX()),
                   juce::jlimit(upYLimit, downYLimit, pPB->getY()),
                   pPB->getWidth(), pPB->getHeight());
}

void rpSynth::ui::LineGeneratorPanel::pointSet(PointBlock* pPB) {
    float w = static_cast<float>(getWidth() - 2 * kHalfPointSize);
    float h = static_cast<float>(getHeight() - 2 * kHalfPointSize);
    auto topLeft = pPB->getBounds().getTopLeft().toFloat();
    audio::FType xValue = static_cast<audio::FType>(topLeft.x / w);
    audio::FType yValue = static_cast<audio::FType>((h - topLeft.y) / h);
    size_t index = pPB->getIndex();

    m_lineGeneratorBind.set(index, rpSynth::audio::LineGenerator::PointData{xValue,yValue});
    repaint();
}
