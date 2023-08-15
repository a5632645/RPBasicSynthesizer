/*
  ==============================================================================

    MainFilter.cpp
    Created: 5 Aug 2023 7:53:24pm
    Author:  mana

  ==============================================================================
*/

#include "MainFilter.h"
#include "FilterImplBase.h"
#include "AllFilterParameters.h"

#include "Filters/Analog/MoogLadderFilter.h"

#include "Filters/Digital/CombFilterPos.h"
#include "Filters/Digital/CombFilterNeg.h"
#include "Filters/Digital/Phaser4Neg.h"
#include "Filters/Digital/Phaser4Pos.h"

static const juce::String kFilterNameAttribute = "filterName";

namespace rpSynth::audio {
MainFilter::MainFilter(const juce::String& ID)
    :AudioProcessorBase(ID) {
    m_allFilterParameters = std::make_unique<filters::AllFilterParameters>();
    // Add all type of impl filter into hash map
    using namespace filters;
    m_allFilters.set(analog::MoogLadderFilter::kName, std::make_shared<analog::MoogLadderFilter>(*m_allFilterParameters));
    m_allFilters.set(digital::CombPos::kName, std::make_shared<digital::CombPos>(*m_allFilterParameters));
    m_allFilters.set(digital::CombNeg::kName, std::make_shared<digital::CombNeg>(*m_allFilterParameters));
    m_allFilters.set(digital::Phaser4Neg::kName, std::make_shared<digital::Phaser4Neg>(*m_allFilterParameters));
    m_allFilters.set(digital::Phaser4Pos::kName, std::make_shared<digital::Phaser4Pos>(*m_allFilterParameters));

    // Set the default Filter impl
    m_newFilterName = analog::MoogLadderFilter::kName;
    m_currentFilterImpl = m_allFilters.getReference(m_newFilterName);
}

MainFilter::~MainFilter() {
    m_allFilterParameters = nullptr;
}

//=============================================================================
// Notice: Please add parameter operator here when adding parameter
void MainFilter::addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    // common control parameters
    layout.add(std::make_unique<MyHostParameter>(m_allFilterParameters->cutoff,
                                                 combineWithID("cutoff"),
                                                 "cutoff",
                                                 juce::NormalisableRange<float>(kStOf10hz, kStOf20000hz, 0.01f),
                                                 hertzToSemitone(440.f),
                                                 g_PitchHertzFloatParameterAttribute),
               std::make_unique<MyHostParameter>(m_allFilterParameters->resonance,
                                                 combineWithID("resonance"),
                                                 "resonance",
                                                 juce::NormalisableRange <float>(0.f, 0.99f, 0.01f),
                                                 0.f),
               std::make_unique<MyHostParameter>(m_allFilterParameters->phase,
                                                 combineWithID("Phase"),
                                                 "Phase",
                                                 juce::NormalisableRange<float>(0.f, 1.f, 0.01f),
                                                 0.f),
               std::make_unique<MyHostParameter>(m_allFilterParameters->feedback,
                                                 combineWithID("Feedback"),
                                                 "Feedback",
                                                 juce::NormalisableRange<float>(0.f, 0.9f, 0.01f),
                                                 0.f)
    );

    // router parameters
    for (auto& set : m_inputRouter) {
        auto pp = std::make_unique<juce::AudioParameterBool>(
            combineWithID(set.pProcessor->combineWithID("input")),
            combineWithID(set.pProcessor->combineWithID("input")),
            false);
        set.pJuceParameter = pp.get();
        layout.add(std::move(pp));
    }
}

//void MainFilter::updateParameters(size_t numSamples) {
//    m_allFilterParameters->cutoff.updateParameter(numSamples);
//    m_allFilterParameters->resonance.updateParameter(numSamples);
//    m_allFilterParameters->limitVolume.updateParameter(numSamples);
//    m_allFilterParameters->limitK.updateParameter(numSamples);
//}

void MainFilter::prepareParameters(FType sampleRate, size_t /*numSamples*/) {
    //m_allFilterParameters->cutoff.prepare(sampleRate);
    //m_allFilterParameters->resonance.prepare(sampleRate);
    //m_allFilterParameters->limitVolume.prepare(sampleRate);
    //m_allFilterParameters->limitK.prepare(sampleRate);
    m_allFilterParameters->prepare(sampleRate);
}
//=============================================================================

void MainFilter::prepare(FType sampleRate, size_t numSamlpes) {
    // buffer init
    m_filterInputBuffer.resize(numSamlpes);
    m_filterOutputBuffer.resize(numSamlpes);

    // init for all filters
    for (auto f : m_allFilters) {
        f->prepare(sampleRate, numSamlpes);
    }
}

void MainFilter::process(size_t beginSamplePos, size_t endSamplePos) {
    // Mix input source
    size_t numSample = endSamplePos - beginSamplePos;
    for (auto& i : m_inputRouter) {
        // Atomic,may be Thread-safe?
        bool bInput = i.pJuceParameter->get();
        if (bInput) {
            i.pProcessor->setFlag();
            juce::FloatVectorOperations::add(reinterpret_cast<FType*>(m_filterInputBuffer.buffer.data() + beginSamplePos),
                                             reinterpret_cast<FType*>(i.pProcessOutputBuffer->buffer.data() + beginSamplePos),
                                             2 * numSample);
        }
    }

    // Filter process
    m_currentFilterImpl->process(m_filterInputBuffer, m_filterOutputBuffer, beginSamplePos, endSamplePos);
}

void MainFilter::saveExtraState(juce::XmlElement& xml) {
    auto* filterXML = xml.createNewChildElement(getProcessorID());
    filterXML->setAttribute(kFilterNameAttribute, m_newFilterName);
}

void MainFilter::loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& /*apvts*/) {
    auto* filterXML = xml.getChildByName(getProcessorID());
    if (filterXML == nullptr) return;

    changeFilter(filterXML->getStringAttribute(kFilterNameAttribute));
}

void MainFilter::onCRClock(size_t n) {
    m_allFilterParameters->onCRClock();
    m_currentFilterImpl->onCRClock(n);

    // process change filter event
    if (m_isFilterChanged) {
        m_currentFilterImpl = m_allFilters.getReference(m_newFilterName);
        m_currentFilterImpl->reset();
        m_isFilterChanged = false;
    }
}

void MainFilter::changeFilter(const juce::String& filterType) {
    // Just return if filter name is same
    if (filterType == m_currentFilterImpl->getFilterName()) {
        return;
    }

    // Do not contain this filter,also do nothing
    if (!m_allFilters.contains(filterType)) {
        return;
    }

    // set flag true,and changed it on audio thread
    m_newFilterName = filterType;
    m_isFilterChanged = true;

    // notify ui
    if (onFilterTypeChange) {
        onFilterTypeChange(filterType);
    }
}

void MainFilter::addAudioInput(AudioProcessorBase* pProcessor, StereoBuffer* pInput) {
    jassert(pInput != nullptr);
    jassert(pProcessor != nullptr);
    m_inputRouter.emplace_back(pProcessor, pInput, nullptr);
}

juce::StringArray MainFilter::getAllFilterNames() const {
    juce::StringArray s;
    for (auto f : m_allFilters) {
        s.add(f->getFilterName());
    }
    return s;
}

juce::StringRef MainFilter::getCurrentFilterName() const {
    return m_newFilterName;
}

void MainFilter::doLayout(ui::FilterKnobsPanel& p, const juce::String& name) {
    if (!m_allFilters.contains(name)) return;

    m_allFilters.getReference(name)->doLayout(p);
}

void MainFilter::clearBuffer() {
    m_filterInputBuffer.clear();
}
}