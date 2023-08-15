/*
  ==============================================================================

    FilterPanel.cpp
    Created: 5 Aug 2023 10:32:52pm
    Author:  mana

  ==============================================================================
*/

#include "FilterPanel.h"
#include "synthesizer/Filter/MainFilter.h"
#include "synthesizer/Filter/AllFilterParameters.h"

namespace rpSynth::ui {
//=========================================================================
FilterKnobsPanel::FilterKnobsPanel(audio::MainFilter& f)
    : m_filter(f)
    , m_cutoff(&f.m_allFilterParameters->cutoff)
    , m_resonance(&f.m_allFilterParameters->resonance)
    , m_phase(&f.m_allFilterParameters->phase) {
    // Don not forge to add knob
    addAndMakeVisible(m_cutoff);
    addAndMakeVisible(m_resonance);
    addAndMakeVisible(m_phase);

    // emmmm
    doLayout(m_filter.getCurrentFilterName());
}

void FilterKnobsPanel::showModulationFrom(audio::ModulatorBase* p) {
    m_cutoff.showModulationFrom(p);
    m_resonance.showModulationFrom(p);
    m_phase.showModulationFrom(p);
}

void FilterKnobsPanel::setAllKnobInvisitable() {
    m_cutoff.setVisible(false);
    m_resonance.setVisible(false);
    m_phase.setVisible(false);
}

//=========================================================================
void FilterKnobsPanel::resized() {
    doLayout(m_filter.getCurrentFilterName());
}

void FilterKnobsPanel::doLayout(const juce::String& filterName) {
    setAllKnobInvisitable();
    m_filter.doLayout(*this, filterName);
}
//=========================================================================

//=========================================================================
class FilterPanel::FilterRouterPanel : public juce::Component {
public:
    ~FilterRouterPanel() override = default;
    FilterRouterPanel(audio::MainFilter& f) {
        size_t numInputs = f.getNumInputs();
        m_buttons.ensureStorageAllocated(static_cast<int>(numInputs));
        for (size_t i = 0; i < numInputs; i++) {
            const auto& set = f.getInputSet(i);
            auto button = std::make_unique<juce::ToggleButton>(set.pProcessor->getProcessorID());
            auto attach = std::make_unique<juce::ButtonParameterAttachment>(*set.pJuceParameter, *button);
            addAndMakeVisible(button.get());

            m_buttons.add(std::move(button));
            m_attach.add(std::move(attach));
        }
    }

    void resized() override {
        static constexpr int h = 20;
        static constexpr int intervalH = 5;
        int y = 0;
        int w = getWidth();

        for (auto* button : m_buttons) {
            button->setBounds(0, y, w, h);
            y = y + h + intervalH;
        }
    }
private:
    juce::OwnedArray<juce::ToggleButton> m_buttons;
    juce::OwnedArray<juce::ButtonParameterAttachment> m_attach;
};
//=========================================================================

//=========================================================================
FilterPanel::FilterPanel(audio::MainFilter& f)
    : m_filter(f)
    , m_filterKnobsPanel(f) {
    addAndMakeVisible(m_buttonShowFilterName);
    addAndMakeVisible(m_filterKnobsPanel);
    // Add filter into pop menu
    m_allFilterNames = f.getAllFilterNames();
    for (int i = 0; i < m_allFilterNames.size(); i++) {
        int ID = i + 1;
        m_popMenuSelectFilter.addItem(ID, m_allFilterNames[i]);
    }

    // Set button
    auto onSelected = [this](int ID) {
        const auto& name = m_allFilterNames[ID - 1];
        m_filter.changeFilter(name);
    };
    m_buttonShowFilterName.setButtonText(f.getCurrentFilterName());
    m_buttonShowFilterName.onClick = [this, onSelected] {
        m_popMenuSelectFilter.showMenuAsync(juce::PopupMenu::Options(), onSelected);
    };

    // when filter processor changed
    f.onFilterTypeChange = [this](const juce::String& name) {
        m_buttonShowFilterName.setButtonText(name);
        m_filterKnobsPanel.doLayout(name);
    };

    // router panel
    m_filterRouterPanel = std::make_unique<FilterRouterPanel>(f);
    addAndMakeVisible(m_filterRouterPanel.get());
}

FilterPanel::~FilterPanel() {
    m_filterRouterPanel = nullptr;
}

void FilterPanel::paint(juce::Graphics& g) {
    g.setColour(juce::Colours::white);
    g.drawRect(getLocalBounds());
}

void FilterPanel::resized() {
    static constexpr int kRouterWidth = 70;

    m_buttonShowFilterName.setBounds(0, 0, getWidth(), 30);
    m_filterRouterPanel->setBounds(getWidth() - kRouterWidth, 50, kRouterWidth, getHeight() - 50);
    m_filterKnobsPanel.setBounds(0, 30, getWidth() - kRouterWidth, getHeight() - 30);
}

void FilterPanel::showModulationFrom(audio::ModulatorBase* p) {
    m_filterKnobsPanel.showModulationFrom(p);
}
}