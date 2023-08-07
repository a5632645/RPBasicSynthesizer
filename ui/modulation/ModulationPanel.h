/*
  ==============================================================================

    ModulationPanel.h
    Created: 27 Jul 2023 9:29:54pm
    Author:  mana

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ModulationPanel.h"

#include "synthesizer/modulation/ModulationManager.h"
#include "NewModulationPointer.h"
#include "ui/ContainModulableComponent.h"

namespace rpSynth {
namespace ui {
class ModulationPanel : public ContainModulableComponent {
public:
    //=========================================================================
    // A listener use to notify ui to show modulation from modulator
    class ShowModulatorChangeListener {
    public:
        virtual ~ShowModulatorChangeListener() = default;
        virtual void notifyShowModulationFrom(audio::ModulatorBase*,ModulationPanel*) = 0;
    };

    void setListener(ShowModulatorChangeListener* l) { m_listener = l; }
    //=========================================================================

    //=========================================================================
    // implement for juce::Component
    ModulationPanel(audio::ModulationManager&);
    ~ModulationPanel() override = default;
    void resized() override;
    //=========================================================================

    //=========================================================================
    // get modulator from ui
    audio::ModulatorBase* getModulator(int index);
    audio::ModulatorBase* getCurrentModulator();
    //=========================================================================

    //=========================================================================
    // set something for dragging modulation
    void setTopMostComponent(juce::Component* top);
    //=========================================================================

    //=========================================================================
    // when have other modulation panel,other panel should have no tab colored
    void clearAllTabColor();
    void fillColorForCurrentTab();
    //=========================================================================

    //===============================================================
    // itself may have any modulation
    void showModulationFrom(audio::ModulatorBase*) override;
    //=========================================================================

    //================================================================================
    // notify single showing
    std::function<void(audio::ModulatorBase*, ModulableUIBase*)> onDragAndAddModulation;
    //================================================================================
private:
    class SmallMouseListener : public juce::MouseListener {
    public:
        SmallMouseListener();
        ~SmallMouseListener() override = default;

        void mouseDown(const juce::MouseEvent&) override;
        void mouseDrag(const juce::MouseEvent&) override;
        void mouseUp(const juce::MouseEvent&) override;

        void setOwnedModulationPanel(ModulationPanel*);
        void setTopMostParentPanel(juce::Component*);
    private:
        //@internal
        NewModulationPointer m_newModulationPointer;
        juce::ComponentDragger m_dragger;
        juce::Component* m_topMostParentComponent = nullptr;
        ModulationPanel* m_modulationPanel = nullptr;
        int m_draggedTabIndex = 0;
    };

    audio::ModulationManager& m_modulationManager;
    SmallMouseListener m_smallMouseListener;
    ShowModulatorChangeListener* m_listener = nullptr;
    juce::TabbedComponent m_tabbedPanel;

    friend class SmallMouseListener;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationPanel)
};
}
}
