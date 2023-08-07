/*
  ==============================================================================

    FloatKnob.cpp
    Created: 27 Jul 2023 8:10:38pm
    Author:  mana

  ==============================================================================
*/

#include <JuceHeader.h>
#include "FloatKnob.h"
#include "synthesizer/modulation/ModulatorBase.h"

class KnobWithModulation : public juce::LookAndFeel_V4 {
public:
    ~KnobWithModulation() override = default;

    // Slider
    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;
};

static KnobWithModulation g_lookAndFeel;

void KnobWithModulation::drawRotarySlider(juce::Graphics& g,
                                          int x, int y, int width, int height,
                                          float sliderPosProportional,
                                          float rotaryStartAngle,
                                          float rotaryEndAngle,
                                          juce::Slider& s) {
    // Draw basic knob
    auto outlineColour = s.findColour(juce::Slider::ColourIds::rotarySliderOutlineColourId);
    auto fillColour = s.findColour(juce::Slider::ColourIds::rotarySliderFillColourId);
    auto pointerColour = s.findColour(juce::Slider::ColourIds::thumbColourId);
    const int nameHight = 14;

    auto wholeBound = juce::Rectangle<int>(x, y, width, height).toFloat();
    auto sliderNameBound = wholeBound.removeFromTop(nameHight);
    auto knobDiameter = juce::jmin(wholeBound.getWidth(), wholeBound.getHeight());
    auto knobRadius = knobDiameter / 2.f;
    auto knobBound = wholeBound.withSizeKeepingCentre(knobDiameter, knobDiameter);
    auto circleFillBound = knobBound.reduced(0.4f * knobRadius);
    auto knobCentralPoint = knobBound.getCentre();

    const float numOuterLines = 6.f;
    const float centralRadius = knobRadius * 0.6f;
    const float modArcThickness = 3.f;
    const float modArcRadius = centralRadius + modArcThickness;
    const float pointerBeginRadius = knobRadius * 0.21f;
    const float outerPointerBeginRadius = knobRadius * 0.84f;
    const float angleInterval = (rotaryEndAngle - rotaryStartAngle) / numOuterLines;
    const float pointerAngel = juce::jmap(sliderPosProportional, rotaryEndAngle, rotaryStartAngle);

    g.setColour(s.findColour(juce::Slider::ColourIds::textBoxTextColourId));
    g.setFont(static_cast<float>(nameHight));
    g.drawText(s.getName(), sliderNameBound, juce::Justification::centredBottom);

    // First we draw a bubble if we can
    auto* knob = dynamic_cast<rpSynth::ui::FloatKnob*>(std::addressof(s));
    if (knob != nullptr) {
        // If it has modulation
        // Draw a bubble on top left and arc line
        if (knob->hasModulation()) {
            auto bubbleBound = juce::Rectangle<float>(knobBound.getX(),
                                                      knobBound.getY(),
                                                      6.f, 6.f);
            // If knob has current set draw bubble blue,else grey
            auto* set = knob->getCurrentShowSetting();
            auto bubbleColor = set == nullptr ? juce::Colours::black : juce::Colours::white;
            auto frameColor = set == nullptr ? juce::Colours::white : juce::Colours::black;
            g.setColour(bubbleColor);
            g.fillEllipse(bubbleBound);
            g.setColour(frameColor);
            g.drawEllipse(bubbleBound, 1.f);

            // If current selected modulator has a modulation with this knob
            // Then draw a arc line
            if (set != nullptr) {
                auto lineColor = set->amount > 0.f ? juce::Colours::aqua : juce::Colours::red;
                lineColor = set->bypass ? juce::Colours::grey : lineColor;

                float modBeginAngle = 0.f;
                float modEndAngle = 0.f;
                if (set->bipolar) {
                    modBeginAngle = juce::jmap(juce::jlimit(0.f, 1.f, sliderPosProportional - set->amount),
                                               rotaryStartAngle,
                                               rotaryEndAngle);
                    modEndAngle = juce::jmap(juce::jlimit(0.f, 1.f, sliderPosProportional + set->amount),
                                             rotaryStartAngle,
                                             rotaryEndAngle);
                } else {
                    modBeginAngle = juce::jmap(sliderPosProportional,
                                               0.f, 1.f,
                                               rotaryStartAngle,
                                               rotaryEndAngle);
                    modEndAngle = juce::jmap(juce::jlimit(0.f, 1.f, sliderPosProportional + set->amount),
                                             rotaryStartAngle,
                                             rotaryEndAngle);
                }

                juce::Path p;
                p.addArc(knobCentralPoint.x - modArcRadius,
                         knobCentralPoint.y - modArcRadius,
                         2.f * modArcRadius,
                         2.f * modArcRadius,
                         modBeginAngle,
                         modEndAngle,
                         true);
                g.setColour(lineColor);
                g.strokePath(p, juce::PathStrokeType(modArcThickness));
            }
        }
    }

    // Draw normal knob have
    g.setColour(fillColour);
    g.fillEllipse(circleFillBound);

    g.setColour(outlineColour);
    float delta = rotaryStartAngle;
    for (int i = 0; i <= static_cast<int>(numOuterLines); i++) {
        g.drawLine(knobCentralPoint.x - outerPointerBeginRadius * std::sin(delta),
                   knobCentralPoint.y - outerPointerBeginRadius * std::cos(delta),
                   knobCentralPoint.x - knobRadius * std::sin(delta),
                   knobCentralPoint.y - knobRadius * std::cos(delta));
        delta += angleInterval;
    }

    g.setColour(pointerColour);
    g.drawLine(knobCentralPoint.x - pointerBeginRadius * std::sin(pointerAngel),
               knobCentralPoint.y - pointerBeginRadius * std::cos(pointerAngel),
               knobCentralPoint.x - centralRadius * std::sin(pointerAngel),
               knobCentralPoint.y - centralRadius * std::cos(pointerAngel),
               4.f);
}

rpSynth::ui::FloatKnob::FloatKnob(audio::MyAudioProcessParameter* p)
    :ModulableUIBase(p)
    , m_attachment(*(p->getHostParameter()), *this) {
    setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                        juce::MathConstants<float>::pi * 2.75f,
                        true);
    setLookAndFeel(&g_lookAndFeel);
    setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::white);
    setPopupDisplayEnabled(true, false, nullptr);
    setName(p->getHostParameter()->getName(100));

    // Fake slider use to set/get modulation amount
    m_fakeSlider.setRange(-1.0, 1.0);
    m_fakeSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    m_fakeSlider.setPopupDisplayEnabled(true, false, nullptr);
    m_fakeSlider.setNumDecimalPlacesToDisplay(2);
    addChildComponent(m_fakeSlider);
    m_fakeSlider.onValueChange = [this] {
        if (m_currentShowingSetting != nullptr) {
            // TODO:Thread not safe
            m_currentShowingSetting->setAmount(static_cast<audio::FType>(m_fakeSlider.getValue()));
            this->repaint();
        }
    };

    // A number editor use to input modulation amount
    m_modulationAmountEditor.setSelectAllWhenFocused(true);
    m_modulationAmountEditor.onEscapeKey = [this] {
        m_modulationAmountEditor.setVisible(false);
    };
    m_modulationAmountEditor.onFocusLost = [this] {
        m_modulationAmountEditor.setVisible(false);
    };
    m_modulationAmountEditor.onReturnKey = [this] {
        auto amount = m_modulationAmountEditor.getText().getFloatValue();
        m_modulationAmountEditor.setVisible(false);
        m_fakeSlider.setValue(amount, juce::sendNotificationSync);
    };
    addChildComponent(m_modulationAmountEditor);

    // A slider value editor to input parameter amount
    m_sliderValueEditor.setSelectAllWhenFocused(true);
    m_sliderValueEditor.onEscapeKey = [this] {
        m_sliderValueEditor.setVisible(false);
    };
    m_sliderValueEditor.onFocusLost = [this] {
        m_sliderValueEditor.setVisible(false);
    };
    m_sliderValueEditor.onReturnKey = [this] {
        m_sliderValueEditor.getText();
        this->setValue(m_sliderValueEditor.getText().getFloatValue(),
                       juce::sendNotificationSync);
        m_sliderValueEditor.setVisible(false);
    };
    addChildComponent(m_sliderValueEditor);

    // What the fuck are these?
    // TODO:Also thread not safe
    m_myPopUpMenu.addItem("InvBypass", [this] {
        if (m_currentShowingSetting == nullptr) return;

        m_currentShowingSetting->invertBypass();
        repaint();
    });
    m_myPopUpMenu.addItem("InvBipolar", [this] {
        if (m_currentShowingSetting == nullptr) return;

        m_currentShowingSetting->invertBipolar();
        repaint();
    });
    m_myPopUpMenu.addItem("remove", [this] {
        if (m_currentShowingSetting == nullptr) return;

        auto* source = m_currentShowingSetting->modulator;
        source->removeModulation(m_currentShowingSetting);
        m_currentShowingSetting = nullptr;
        repaint();
    });
    m_myPopUpMenu.addItem("edit param value", [this] {
        m_sliderValueEditor.setText(juce::String(getValue()), false);
        m_sliderValueEditor.setVisible(true);
    });
    m_myPopUpMenu.addItem("edit modulation amount", [this] {
        if (m_currentShowingSetting == nullptr) return;

        m_modulationAmountEditor.setText(juce::String(m_currentShowingSetting->amount), false);
        m_modulationAmountEditor.setVisible(true);
    });

    // how
    m_attachment.sendInitialUpdate();
}

rpSynth::ui::FloatKnob::~FloatKnob() {
    setLookAndFeel(nullptr);
}

void rpSynth::ui::FloatKnob::resized() {
    juce::Slider::resized();
    auto b = getLocalBounds();
    m_fakeSlider.setBounds(b);
    auto h = juce::jmin(30, b.getHeight());
    auto editorB = b.withSizeKeepingCentre(b.getWidth(), h);
    m_modulationAmountEditor.setBounds(editorB);
    m_sliderValueEditor.setBounds(editorB);
}

void rpSynth::ui::FloatKnob::paint(juce::Graphics& g) {
    juce::Slider::paint(g);

    if (m_currentShowingSetting == nullptr) return;
}

void rpSynth::ui::FloatKnob::mouseDown(const juce::MouseEvent& e) {
    if (juce::ModifierKeys::getCurrentModifiers().isRightButtonDown()) {
        m_myPopUpMenu.showMenuAsync(juce::PopupMenu::Options());
        return;
    }

    m_isCtrlDown = juce::ModifierKeys::getCurrentModifiers().isCommandDown();
    if (m_isCtrlDown) {
        m_fakeSlider.mouseDown(e);
    } else {
        juce::Slider::mouseDown(e);
    }
}

void rpSynth::ui::FloatKnob::mouseDrag(const juce::MouseEvent& e) {
    if (juce::ModifierKeys::getCurrentModifiers().isRightButtonDown()) return;

    if (m_isCtrlDown) {
        m_fakeSlider.mouseDrag(e);
    } else {
        juce::Slider::mouseDrag(e);
    }
}

void rpSynth::ui::FloatKnob::mouseUp(const juce::MouseEvent& e) {
    if (juce::ModifierKeys::getCurrentModifiers().isRightButtonDown()) return;

    if (m_isCtrlDown) {
        m_fakeSlider.mouseUp(e);
    } else {
        juce::Slider::mouseUp(e);
    }
}

void rpSynth::ui::FloatKnob::showModulationFrom(audio::ModulatorBase* pM) {
    m_currentShowingSetting = getMyAudioProcessorParameter()->getModulationSetting(pM);
    repaint();

    if (m_currentShowingSetting == nullptr) {
        m_fakeSlider.setTextValueSuffix(juce::String(" <- Null"));
    } else {
        m_fakeSlider.setValue(m_currentShowingSetting->amount, juce::dontSendNotification);
        m_fakeSlider.setTextValueSuffix(juce::String(" <- ") + m_currentShowingSetting->modulator->getProcessorID());
    }
}