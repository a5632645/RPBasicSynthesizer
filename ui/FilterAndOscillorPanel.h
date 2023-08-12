/*
  ==============================================================================

    FilterAndOscillorPanel.h
    Created: 8 Aug 2023 9:10:44am
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "ui/ContainModulableComponent.h"

namespace rpSynth::audio {
class BasicSynthesizer;
class ModulatorBase;
}

namespace rpSynth::ui {
class FilterPanel;
class OscillorPanel;
}

namespace rpSynth::ui {
class FilterAndOscillorPanel : public ContainModulableComponent {
public:
    FilterAndOscillorPanel(audio::BasicSynthesizer&);
    ~FilterAndOscillorPanel() override = default;

    void resized();
    void paint(juce::Graphics&);

    void showModulationFrom(audio::ModulatorBase*);
private:
    std::unique_ptr<OscillorPanel> m_oscPanel1;
    std::unique_ptr<OscillorPanel> m_oscPanel2;
    std::unique_ptr<FilterPanel> m_filterPanel1;
    std::unique_ptr<FilterPanel> m_filterPanel2;
};
}