/*
  ==============================================================================

    ModulationPanel.cpp
    Created: 27 Jul 2023 9:29:54pm
    Author:  mana

  ==============================================================================
*/

#include <JuceHeader.h>
#include "ModulationPanel.h"
#include "synthesizer/modulation/ModulationManager.h"
#include "ui/controller/ModulableUIBase.h"

namespace rpSynth {
namespace ui {
//=========================================================================
// implement for juce::Component
ModulationPanel::ModulationPanel(audio::ModulationManager& m)
    :m_tabbedPanel(juce::TabbedButtonBar::Orientation::TabsAtTop)
    , m_modulationManager(m) {
    int numModulator = static_cast<int>(m_modulationManager.getNumModulators());
    for (int i = 0; i < numModulator; i++) {
        auto* modulator = m_modulationManager.getModulator(i);
        m_tabbedPanel.addTab(modulator->getProcessorID(),
                             juce::Colours::grey,
                             modulator->createControlComponent(),
                             true);
    }

    m_smallMouseListener.setOwnedModulationPanel(this);
    addAndMakeVisible(m_tabbedPanel);

    // click any tab button
    auto& tabBar = m_tabbedPanel.getTabbedButtonBar();
    int numButton = tabBar.getNumTabs();
    for (int i = 0; i < numButton; i++) {
        tabBar.getTabButton(i)->onClick = [this] {
            // Exception: Must specify a listener or panel won't change modulator
            // see @setListener(ShowModulatorChangeListener* l)
            m_listener->notifyShowModulationFrom(getCurrentModulator(), this);
        };
    }
}

void ModulationPanel::resized() {
    m_tabbedPanel.setBounds(getLocalBounds());
}
//=========================================================================

//=========================================================================
// get modulator from ui
audio::ModulatorBase* ModulationPanel::getModulator(int index) {
    return m_modulationManager.getModulator(index);
}

audio::ModulatorBase* ModulationPanel::getCurrentModulator() {
    return m_modulationManager.getModulator(m_tabbedPanel.getCurrentTabIndex());
}
//=========================================================================

//=========================================================================
// set something for dragging modulation
void ModulationPanel::setTopMostComponent(juce::Component* top) {
    m_smallMouseListener.setTopMostParentPanel(top);
}
//=========================================================================

//=========================================================================
// when have other modulation panel,other panel should have no tab colored
void ModulationPanel::clearAllTabColor() {
    for (int i = 0; i < m_tabbedPanel.getNumTabs(); i++) {
        m_tabbedPanel.setTabBackgroundColour(i, juce::Colours::grey);
    }
}

void ModulationPanel::fillColorForCurrentTab() {
    m_tabbedPanel.setTabBackgroundColour(m_tabbedPanel.getCurrentTabIndex(), juce::Colours::lightgrey);
}
//=========================================================================

//===============================================================
// itself may have any modulation
void ModulationPanel::showModulationFrom(audio::ModulatorBase* p) {
    for (int i = 0; i < m_tabbedPanel.getNumTabs(); i++) {
        auto* c = dynamic_cast<ContainModulableComponent*>(m_tabbedPanel.getTabContentComponent(i));
        c->showModulationFrom(p);
    }
}
//===============================================================

//=============================================================================
ModulationPanel::SmallMouseListener::SmallMouseListener() {
    m_newModulationPointer.setSize(15, 15);
}

void ModulationPanel::SmallMouseListener::mouseDown(const juce::MouseEvent& e) {
    // Exception: Must have a topMostComponent to put pointer
    // 异常: 应该有一个顶层部件放置拖拽指示器
    // see @setTopMostParentPanel
    jassert(m_topMostParentComponent != nullptr);

    if (auto* c = dynamic_cast<juce::TabBarButton*>(e.originalComponent); c != nullptr) {
        m_draggedTabIndex = c->getIndex();

        auto ee = e.getEventRelativeTo(m_topMostParentComponent);
        m_newModulationPointer.setTopLeftPosition(ee.getPosition());
        m_dragger.startDraggingComponent(&m_newModulationPointer, ee);
        m_newModulationPointer.toFront(false);
        m_newModulationPointer.setVisible(true);
    }
}

void ModulationPanel::SmallMouseListener::mouseDrag(const juce::MouseEvent& e) {
    // Exception: Must have a topMostComponent to put pointer
    // 异常: 应该有一个顶层部件放置拖拽指示器
    // see @setTopMostParentPanel
    jassert(m_topMostParentComponent != nullptr);

    m_dragger.dragComponent(&m_newModulationPointer,
                            e.getEventRelativeTo(m_topMostParentComponent),
                            nullptr);
}

void ModulationPanel::SmallMouseListener::mouseUp(const juce::MouseEvent& e) {
    // Exception: Must have a topMostComponent to put pointer
    // 异常: 应该有一个顶层部件放置拖拽指示器
    // see @setTopMostParentPanel
    jassert(m_topMostParentComponent != nullptr);

    m_newModulationPointer.setVisible(false);

    auto* c = m_topMostParentComponent->getComponentAt(e.getEventRelativeTo(m_topMostParentComponent).position);
    if (auto* mayModulable = dynamic_cast<ModulableUIBase*>(c); mayModulable != nullptr) {
        auto* target = mayModulable->getMyAudioProcessorParameter();
        if (!target->canBeModulated()) return;

        auto* modulator = m_modulationPanel->getModulator(m_draggedTabIndex);
        modulator->addModulation(target);

        m_modulationPanel->onDragAndAddModulation(modulator, mayModulable);
    }
}

void ModulationPanel::SmallMouseListener::setOwnedModulationPanel(ModulationPanel* panel) {
    if (m_modulationPanel != nullptr) {
        auto& tabBar = m_modulationPanel->m_tabbedPanel.getTabbedButtonBar();
        int numButton = tabBar.getNumTabs();
        for (int i = 0; i < numButton; i++) {
            tabBar.getTabButton(i)->removeMouseListener(this);
        }
    }

    // Exception: can not assign a nullptr ModulationPanel
    jassert(panel != nullptr);
    m_modulationPanel = panel;
    auto& tabBar = m_modulationPanel->m_tabbedPanel.getTabbedButtonBar();
    int numButton = tabBar.getNumTabs();
    for (int i = 0; i < numButton; i++) {
        tabBar.getTabButton(i)->addMouseListener(this, false);
    }
}
void ModulationPanel::SmallMouseListener::setTopMostParentPanel(juce::Component* top) {
    if (m_topMostParentComponent != nullptr) {
        m_topMostParentComponent->removeChildComponent(&m_newModulationPointer);
    }

    // Exception: can not assign a nullptr topMostComponent
    jassert(top != nullptr);
    m_topMostParentComponent = top;
    m_topMostParentComponent->addChildComponent(m_newModulationPointer);
}
}
}