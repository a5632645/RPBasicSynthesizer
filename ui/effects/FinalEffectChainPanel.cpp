/*
  ==============================================================================

    FinalEffectChainPanel.cpp
    Created: 7 Aug 2023 8:55:29pm
    Author:  mana

  ==============================================================================
*/

#include <JuceHeader.h>
#include "FinalEffectChainPanel.h"

#include "ui/effects/EffectProcessorControlPanel.h"

#include "synthesizer/Effects/OrderableEffectsChain.h"

namespace rpSynth::ui {
FinalEffectChainPanel::FinalEffectChainPanel(audio::OrderableEffectsChain& c)
    : m_chain(c) {

    // content
    m_showingContent = std::make_unique<juce::Component>();
    addAndMakeVisible(m_showingContent.get());

    // when init,the chain is ordered
    for (int i = 0; auto & p : c.getAllEffectsProcessor()) {
        auto pc = std::make_shared<EffectProcessorControlPanel>(*p);
        pc->addListener(this);
        pc->addMouseListener(this, false);
        pc->setIndex(i++);
        m_controllers.push_back(pc);
        addAndMakeVisible(pc.get());
    };

    // callback
    m_chain.onOrderChanged = [this] {
        onChainOrderChanged();
    };
}

FinalEffectChainPanel::~FinalEffectChainPanel() {
    m_currentShowingEffectComponent = nullptr;
    m_showingContent = nullptr;
    m_chain.onOrderChanged = nullptr;
}

void FinalEffectChainPanel::resized() {
    float numControllers = static_cast<float>(m_controllers.size());
    int height = static_cast<int>(getHeight() / numControllers);
    int width = 100;
    auto bound = juce::Rectangle<int>(0, 0, width, height);

    m_showingContent->setBounds(width, 0, getWidth() - width, getHeight());
    for (auto& p : m_controllers) {
        p->setBounds(bound);
        bound.translate(0, height);
    }
}

void FinalEffectChainPanel::paint(juce::Graphics& g) {
    g.fillAll(juce::Component::findColour(juce::DocumentWindow::backgroundColourId));
}

void FinalEffectChainPanel::buttonClicked(juce::Button* b) {
    auto* controller = dynamic_cast<EffectProcessorControlPanel*>(b);
    auto* panel = controller->getEffectControlPanel();
    if (panel == m_currentShowingEffectComponent) return;

    m_showingContent->removeChildComponent(m_currentShowingEffectComponent);
    m_showingContent->addChildComponent(panel);
    panel->setBounds(m_showingContent->getLocalBounds());
    panel->setVisible(true);
    m_currentShowingEffectComponent = panel;

    // Badly we need to let it show modulator...
    panel->showModulationFrom(m_currentShowingModulator);
}

void FinalEffectChainPanel::mouseUp(const juce::MouseEvent& e) {
    auto* c = dynamic_cast<EffectProcessorControlPanel*>(e.originalComponent);
    resized();
    int oldIndex = m_draggedComponentOldIndex;
    int newIndex = c->getIndex();
    if (oldIndex != newIndex) {
        m_chain.reOrderProcessor(oldIndex, newIndex);
    }
}

void FinalEffectChainPanel::mouseDrag(const juce::MouseEvent& e) {
    auto* c = dynamic_cast<EffectProcessorControlPanel*>(e.originalComponent);
    auto oldX = c->getBounds().getX();
    m_dragger.dragComponent(c, e, nullptr);

    // Limit
    auto newX = c->getBounds().getX();
    auto bottom = c->getBounds().getBottom();
    auto top = c->getBounds().getY();
    auto width = c->getBounds().getWidth();
    auto height = c->getBounds().getHeight();
    if (newX != oldX || bottom > getHeight() || top < 0) {
        c->setBounds(oldX, juce::jlimit(0, getHeight() - height, top), width, height);
    }

    // ReOrder
    int y = c->getBounds().getCentreY();
    int newIndex = y / height;
    int oldIndex = c->getIndex();
    if (newIndex == oldIndex) return;

    // Move component
    std::vector copy{m_controllers};
    if (newIndex > oldIndex) {
        for (int i = oldIndex; i < newIndex; i++) {
            copy[i] = copy[i + 1ULL];
        }
        copy[newIndex] = m_controllers[oldIndex];
    } else {
        for (int i = oldIndex; i > newIndex; i--) {
            copy[i] = copy[i - 1ULL];
        }
        copy[newIndex] = m_controllers[oldIndex];
    }

    // Swap and redraw
    m_controllers.swap(copy);
    resized();

    for (int i = 0; auto & cc : m_controllers) {
        cc->setIndex(i++);
    }
}

void FinalEffectChainPanel::mouseDown(const juce::MouseEvent& e) {
    m_dragger.startDraggingComponent(e.originalComponent, e);
    auto* c = dynamic_cast<EffectProcessorControlPanel*>(e.originalComponent);
    m_draggedComponentOldIndex = c->getIndex();
}

void FinalEffectChainPanel::showModulationFrom(audio::ModulatorBase* p) {
    m_currentShowingModulator = p;
    if (m_currentShowingEffectComponent != nullptr) {
        m_currentShowingEffectComponent->showModulationFrom(p);
    }
}

void FinalEffectChainPanel::onChainOrderChanged() {
    std::vector newOrder{m_controllers};
    for (auto& p : m_controllers) {
        int i = m_chain.getEffectOrder(p->getEffectName());
        newOrder[i] = p;
    }

    m_controllers.swap(newOrder);
    resized();
}
}