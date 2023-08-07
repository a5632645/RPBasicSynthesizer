/*
  ==============================================================================

    FloatKnob.h
    Created: 27 Jul 2023 8:10:38pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "synthesizer/WrapParameter.h"
//#include "synthesizer/modulation/ModulationSetting.h"
#include "ModulableUIBase.h"

namespace rpSynth::ui {
/**
 * @brief rotarySliderOutlineColourId -> outline tick.....
 *        rotarySliderFillColourId -> central circle.....
 *        thumbColourId -> pointer.....
*/
class FloatKnob : public juce::Slider
    , public ModulableUIBase {
public:
    FloatKnob(audio::MyAudioProcessParameter*);
    ~FloatKnob() override;

    void resized() override;
    void paint(juce::Graphics&) override;

    audio::ModulationSettings* getCurrentShowSetting() { return m_currentShowingSetting; }
    bool hasModulation() const { return getMyAudioProcessorParameter()->hasModulation(); }

    //=========================================================================
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

    //=========================================================================
    void showModulationFrom(audio::ModulatorBase*) override;
private:
    // what
    juce::SliderParameterAttachment m_attachment;
    juce::Slider m_fakeSlider;
    juce::PopupMenu m_myPopUpMenu;
    bool m_isCtrlDown = false;

    // fuck yeah
    juce::TextEditor m_modulationAmountEditor;
    juce::TextEditor m_sliderValueEditor;

    audio::ModulationSettings* m_currentShowingSetting = nullptr;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FloatKnob)
};
};