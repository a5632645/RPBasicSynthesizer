/*
  ==============================================================================

    OrderableEffectsChain.cpp
    Created: 7 Aug 2023 3:38:22pm
    Author:  mana

  ==============================================================================
*/

#include "OrderableEffectsChain.h"
#include "EffectProcessorBase.h"

#include "EffectsImpl/Flanger.h"
#include "EffectsImpl/Phaser.h"

static const juce::String kChainOrderXMLTag = "Effect_Chain_Order";
static const juce::String kProcessorIndexTag = "POrder";
static const juce::String kProcessorIDAttributeName = "ID";
static const juce::String kProcessorIndexAttributeName = "Index";

namespace rpSynth::audio {
//================================================================================
// Constructor
OrderableEffectsChain::OrderableEffectsChain(const juce::String& ID)
    : AudioProcessorBase(ID) {
    // Add all effect processor into it please
    m_effectsChain.emplace_back(std::make_shared<effects::Flanger>(*this));
    m_effectsChain.emplace_back(std::make_shared<effects::Phaser>(*this));

    // And set default index please
    for (int i = 0; auto & p : m_effectsChain) {
        m_effectProcessorIndexes.set(p->getEffectName(), i++);
    }
}

OrderableEffectsChain::~OrderableEffectsChain() {
    // what, may be nothing...
    // how to delete a shared_ptr...
    // well,let vector deconstruct...
}
//================================================================================

//================================================================================
// implement for AudioProcessorBase
void OrderableEffectsChain::addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    for (auto& p : m_effectsChain) {
        p->addParameterToLayout(layout);
    }
}

//void OrderableEffectsChain::updateParameters(size_t numSamples) {
//    for (auto& p : m_effectsChain) {
//        p->updateParameters(numSamples);
//    }
//}

void OrderableEffectsChain::prepareParameters(FType sampleRate, size_t numSamples) {
    for (auto& p : m_effectsChain) {
        p->prepareParameters(sampleRate, numSamples);
    }
}

void OrderableEffectsChain::prepare(FType sampleRate, size_t numSamlpes) {
    for (auto& p : m_effectsChain) {
        p->prepare(sampleRate, numSamlpes);
    }

    m_audioBuffer.resize(numSamlpes);
}

void OrderableEffectsChain::process(size_t beginSamplePos, size_t endSamplePos) {
    // order lock
    const juce::ScopedLock lock{m_orderLock};

    for (auto& p : m_effectsChain) {
        p->process(beginSamplePos, endSamplePos);
    }
}

void OrderableEffectsChain::saveExtraState(juce::XmlElement& xml) {
    // Create catalog
    auto* chainXML = xml.createNewChildElement(getProcessorID());
    auto* chainOrderXML = chainXML->createNewChildElement(kChainOrderXMLTag);
    
    // Save processor order
    for (auto it = m_effectProcessorIndexes.begin();
         it != m_effectProcessorIndexes.end();
         it.operator++()) {
        auto* pXML = chainOrderXML->createNewChildElement(kProcessorIndexTag);
        pXML->setAttribute(kProcessorIDAttributeName, it.getKey());
        pXML->setAttribute(kProcessorIndexAttributeName, it.getValue());
    }

    // each processor save extra state
    for (auto& p : m_effectsChain) {
        p->saveExtraState(*chainXML);
    }
}

void OrderableEffectsChain::loadExtraState(juce::XmlElement& xml, juce::AudioProcessorValueTreeState& apvts) {
    auto* chainXML = xml.getChildByName(getProcessorID());
    if (chainXML == nullptr) return;

    // Load processor order
    auto* chainOrderXML = chainXML->getChildByName(kChainOrderXMLTag);
    if (chainOrderXML == nullptr) return;
    for (auto* pXml : chainOrderXML->getChildWithTagNameIterator(kProcessorIndexTag)) {
        const auto& name = pXml->getStringAttribute(kProcessorIDAttributeName);
        int index = pXml->getIntAttribute(kProcessorIndexAttributeName);
        m_effectProcessorIndexes.set(name, index);
    }

    // Do reorder
    std::vector copy{m_effectsChain};
    for (auto& p : m_effectsChain) {
        int newIndex = m_effectProcessorIndexes[p->getEffectName()];
        copy[newIndex] = p;
    }
    m_effectsChain.swap(copy);

    // each processor load extra state
    for (auto& p : m_effectsChain) {
        p->loadExtraState(*chainXML, apvts);
    }
}

void OrderableEffectsChain::onCRClock(size_t n) {
    for (auto& f : m_effectsChain) {
        if (f->notBypass->get()) {
            f->onCRClock(n);
        }
    }
}
//================================================================================

//================================================================================
void OrderableEffectsChain::reOrderProcessor(const juce::String& processorID, int newIndex) {
    auto oldIndex = m_effectProcessorIndexes.getReference(processorID);
    reOrderProcessor(oldIndex, newIndex);
}
//
//void OrderableEffectsChain::setAudioInput(NonNullPtr<StereoBuffer> input) {
//    m_inputBuffer = input.ptr;
//}

void OrderableEffectsChain::reOrderProcessor(int oldIndex, int newIndex) {
    if (newIndex == oldIndex) return;

    // create new order vector
    std::vector<std::shared_ptr<effects::EffectProcessorBase>> newProcessorOrder{m_effectsChain};

    // build new order
    if (newIndex > oldIndex) {
        for (int i = oldIndex; i < newIndex; i++) {
            newProcessorOrder[i] = newProcessorOrder[i + 1ULL];
        }
        newProcessorOrder[newIndex] = m_effectsChain[oldIndex];
    } else {
        for (int i = oldIndex; i > newIndex; i--) {
            newProcessorOrder[i] = newProcessorOrder[i - 1ULL];
        }
        newProcessorOrder[newIndex] = m_effectsChain[oldIndex];
    }

    // update index hashtable
    for (int i = 0; auto & p : newProcessorOrder) {
        m_effectProcessorIndexes.set(p->getEffectName(), i++);
    }

    // swap chain
    const juce::ScopedLock lock{m_orderLock};
    m_effectsChain.swap(newProcessorOrder);
}

void OrderableEffectsChain::clearBuffer() {
    m_audioBuffer.clear();
}
//================================================================================

}