/*
  ==============================================================================

    MyPanel.h
    Created: 3 Aug 2023 10:31:02pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace rpSynth::audio {
class ModulatorBase;
//class MyAudioProcessParameter;
class MyAudioParameter;
}

namespace rpSynth::ui {
class ModulableUIBase {
public:
    //ModulableUIBase(audio::MyAudioProcessParameter* param)
    ModulableUIBase(audio::MyAudioParameter* param)

        :m_audioProcessorParameter(param) {
        // A ui object can not own a nullptr
        // also ui is first deleted before processor
        // ui is last init after processor
        jassert(param != nullptr);
    }

    virtual ~ModulableUIBase() = default;

    //audio::MyAudioProcessParameter* getMyAudioProcessorParameter() const {
    audio::MyAudioParameter* getMyAudioProcessorParameter() const {
        return m_audioProcessorParameter;
    }

    virtual void showModulationFrom(audio::ModulatorBase*) = 0;
private:
    //audio::MyAudioProcessParameter* m_audioProcessorParameter;
    audio::MyAudioParameter* m_audioProcessorParameter;
};
}
