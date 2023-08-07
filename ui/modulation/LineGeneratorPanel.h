/*
  ==============================================================================

    LineGeneratorPanel.h
    Created: 28 Jul 2023 7:47:36am
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PointBlock.h"
#include "../../synthesizer/modulation/LineGenerator.h"

namespace rpSynth::ui {
class LineGeneratorPanel : public juce::Component, public PointBlock::Listener {
public:
    LineGeneratorPanel(rpSynth::audio::LineGenerator& LG);

    void paint(juce::Graphics&) override;
    void resized() override;

    void mouseDoubleClick(const juce::MouseEvent& e) override;

    void pointPressed(PointBlock*, const juce::MouseEvent& e) override;
    void pointDeleted(PointBlock*) override;
    void pointMoved(PointBlock*, const juce::MouseEvent& e) override;
    void pointSet(PointBlock*) override;
private:
    rpSynth::audio::LineGenerator& m_lineGeneratorBind;
    juce::OwnedArray<PointBlock> m_pointControls;
    juce::ComponentDragger m_componentDragger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LineGeneratorPanel)
};
};