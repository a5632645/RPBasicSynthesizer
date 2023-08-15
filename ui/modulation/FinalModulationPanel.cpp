/*
  ==============================================================================

    FinalModulationPanel.cpp
    Created: 6 Aug 2023 3:17:36pm
    Author:  mana

  ==============================================================================
*/

#include "FinalModulationPanel.h"

#include "synthesizer/BasicSynthesizer.h"
#include "ui/modulation/ModulationPanel.h"
#include "ui/controller/ModulableUIBase.h"

namespace rpSynth::ui {
FinalModulationPanel::FinalModulationPanel(audio::BasicSynthesizer& s,
                                           ModulationPanel::ShowModulatorChangeListener* p)
: m_listener(p){
    // Exception: can not assign a null listener
    // Òì³££º²»ÄÜÖ¸¶¨Ò»¸ö¿ÕÖ¸Õë¼àÌýÆ÷
    jassert(p != nullptr);

    auto onDrag = [this](audio::ModulatorBase* m, ModulableUIBase* u) {
        if (m == m_currentShowingModulator) {
            u->showModulationFrom(m);
        }
    };

    // LFOS
    m_LFOsPanel = std::make_unique<ModulationPanel>(s.m_LFOModulationManager);
    addAndMakeVisible(m_LFOsPanel.get());
    m_LFOsPanel->onDragAndAddModulation = onDrag;
    m_LFOsPanel->setListener(this);

    // ENVs
    m_ENVsPanel = std::make_unique<ModulationPanel>(s.m_EnvModulationManager);
    addAndMakeVisible(m_ENVsPanel.get());
    m_ENVsPanel->onDragAndAddModulation = onDrag;
    m_ENVsPanel->setListener(this);
}

FinalModulationPanel::~FinalModulationPanel() {
    m_LFOsPanel = nullptr;
    m_ENVsPanel = nullptr;
}

void FinalModulationPanel::resized() {
    m_ENVsPanel->setBoundsRelative(0.f, 0.f, 0.5f, 1.f);
    m_LFOsPanel->setBoundsRelative(0.5f, 0.f, 0.5f, 1.f);
}

void FinalModulationPanel::showModulationFrom(audio::ModulatorBase* p) {
    m_ENVsPanel->showModulationFrom(p);
    m_LFOsPanel->showModulationFrom(p);
}

void FinalModulationPanel::notifyShowModulationFrom(audio::ModulatorBase* m, ModulationPanel* p) {
    m_LFOsPanel->clearAllTabColor();
    m_ENVsPanel->clearAllTabColor();

    p->fillColorForCurrentTab();
    m_listener->notifyShowModulationFrom(m, p);
    m_currentShowingModulator = m;
}

void FinalModulationPanel::setTopMostComponent(juce::Component* topmost) {
    // Exception: Can not assign a nullptr component
    // ²»ÄÜÖ¸¶¨Ò»¸ö¿ÕÖ¸Õë²¿¼þ
    jassert(topmost != nullptr);
    m_ENVsPanel->setTopMostComponent(topmost);
    m_LFOsPanel->setTopMostComponent(topmost);
}

void FinalModulationPanel::init() {
    // Default we set lfo1 as showing modulator
    m_LFOsPanel->fillColorForCurrentTab();
    notifyShowModulationFrom(m_LFOsPanel->getCurrentModulator(), m_LFOsPanel.get());
}
}