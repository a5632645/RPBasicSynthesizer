/*
  ==============================================================================

    FinalModulationPanel.h
    Created: 6 Aug 2023 3:17:36pm
    Author:  mana

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "ui/ContainModulableComponent.h"
#include "ui/modulation/ModulationPanel.h"

namespace rpSynth::audio {
class BasicSynthesizer;
}

namespace rpSynth::ui {
class FinalModulationPanel : public ContainModulableComponent
    , public ModulationPanel::ShowModulatorChangeListener {
public:
    //=========================================================================
    // implement for juce::Component
    FinalModulationPanel(audio::BasicSynthesizer&, ModulationPanel::ShowModulatorChangeListener*);
    ~FinalModulationPanel() override;

    void resized() override;
    //=========================================================================

    //=========================================================================
    // implement for ContainModulableComponent
    void showModulationFrom(audio::ModulatorBase*) override;
    //=========================================================================

    //================================================================================
    // implement for ModulationPanel::ShowModulatorChangeListener
    void notifyShowModulationFrom(audio::ModulatorBase*, ModulationPanel*) override;
    //================================================================================

    //================================================================================
    /**
     * @brief 设置最顶层的控件,调制拖拽指示器要在上面显示，所以必须是最顶层的控件
     * @param topmost 最顶层的控件
    */
    void setTopMostComponent(juce::Component* topmost);

    /**
     * @brief 初始化,在所有设置结束后调用它
    */
    void init();
    //================================================================================
private:
    ModulationPanel::ShowModulatorChangeListener* m_listener = nullptr;
    audio::ModulatorBase* m_currentShowingModulator = nullptr;

    std::unique_ptr<ModulationPanel> m_LFOsPanel;
    std::unique_ptr<ModulationPanel> m_ENVsPanel;
};
}