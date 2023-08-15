/*
  ==============================================================================

    Flanger.cpp
    Created: 8 Aug 2023 9:38:15am
    Author:  mana

  ==============================================================================
*/

#include "Flanger.h"
#include "synthesizer/NewWrapParameter.h"
#include "synthesizer/dsps/IIRHilbertTransform.h"

#include "ui/controller/FloatKnob.h"

//================================================================================
// 全部镶边器的参数
//================================================================================
namespace rpSynth::audio::effects {
struct FlangerParameters {
    MyAudioParameter TZFDelayTime;                 // TZF对干信号延迟[0,10]ms
    MyAudioParameter delayTime;                    // 延迟时间[0,10]ms
    MyAudioParameter feedback;                     // 反馈[-1,1]
    MyAudioParameter fbLowCut;                     // 反馈低切[20,20000]hz
    MyAudioParameter fbHighCut;                    // 反馈高切[20,20000]hz
    MyAudioParameter mix;                          // 混合[-1,1]
    MyAudioParameter rate;                         // LFO速率[0,5]hz
    MyAudioParameter barberpoleRate;               // 另一个LFO速率[-10,10]hz
    MyAudioParameter depth;                        // 深度[0,1]
    MyAudioParameter barberpolePhase;              // barberpole的LFO相位[0,1]
    MyAudioParameter lfoPhase;                     // LFO相差相位[0,1]
    MyAudioParameter lfoShape;                     // LFO形状从三角波到正弦波[0,1]
    juce::AudioParameterBool* disableBarberpole = nullptr;// 禁用希尔伯特导致的相位问题

    // Merge all
    void prepareAll(FType sr, size_t /*num*/) {
        TZFDelayTime.prepare(sr);
        delayTime.prepare(sr);
        feedback.prepare(sr);
        fbLowCut.prepare(sr);
        fbHighCut.prepare(sr);
        mix.prepare(sr);
        rate.prepare(sr);
        barberpoleRate.prepare(sr);
        depth.prepare(sr);
        barberpolePhase.prepare(sr);
        lfoPhase.prepare(sr);
        lfoShape.prepare(sr);
    }

    void onCRClock() {
        TZFDelayTime.onCRClock();
        delayTime.onCRClock();
        feedback.onCRClock();
        fbLowCut.onCRClock();
        fbHighCut.onCRClock();
        mix.onCRClock();
        rate.onCRClock();
        barberpoleRate.onCRClock();
        depth.onCRClock();
        barberpolePhase.onCRClock();
        lfoPhase.onCRClock();
        lfoShape.onCRClock();
    }

    //void updateAll(size_t num) {
    //    TZFDelayTime.updateParameter(num);
    //    delayTime.updateParameter(num);
    //    feedback.updateParameter(num);
    //    fbLowCut.updateParameter(num);
    //    fbHighCut.updateParameter(num);
    //    mix.updateParameter(num);
    //    rate.updateParameter(num);
    //    barberpoleRate.updateParameter(num);
    //    depth.updateParameter(num);
    //    barberpolePhase.updateParameter(num);
    //    lfoPhase.updateParameter(num);
    //    lfoShape.updateParameter(num);
    //}
};

//================================================================================
// Flanger Panel
//================================================================================
class FlangerPanel : public ui::ContainModulableComponent {
public:
    //================================================================================
    // implement for juce::Component
    //================================================================================
    ~FlangerPanel() override = default;

    FlangerPanel(FlangerParameters& f)
        :m_tzfDelay(&f.TZFDelayTime)
        , m_delay(&f.delayTime)
        , m_fb(&f.feedback)
        , m_fbLowcut(&f.fbLowCut)
        , m_fbHighCut(&f.fbHighCut)
        , m_mix(&f.mix)
        , m_rate(&f.rate)
        , m_barberpoleRate(&f.barberpoleRate)
        , m_depth(&f.depth)
        , m_barberpolePhase(&f.barberpolePhase)
        , m_lfoPhase(&f.lfoPhase)
        , m_attach(*f.disableBarberpole, m_disableBarber)
        , m_lfoShape(&f.lfoShape) {
        addAndMakeVisible(m_tzfDelay);
        addAndMakeVisible(m_delay);
        addAndMakeVisible(m_fb);
        addAndMakeVisible(m_fbLowcut);
        addAndMakeVisible(m_fbHighCut);
        addAndMakeVisible(m_mix);
        addAndMakeVisible(m_rate);
        addAndMakeVisible(m_barberpoleRate);
        addAndMakeVisible(m_depth);
        addAndMakeVisible(m_barberpolePhase);
        addAndMakeVisible(m_lfoPhase);
        m_disableBarber.setButtonText(f.disableBarberpole->name);
        addAndMakeVisible(m_disableBarber);
        addAndMakeVisible(m_lfoShape);
    }

    void resized() override {
        auto bound = juce::Rectangle(0, 0, 70, 70);
        m_tzfDelay.setBounds(bound); bound.translate(80, 0);
        m_delay.setBounds(bound); bound.translate(80, 0);
        m_fb.setBounds(bound); bound.translate(80, 0);
        m_fbLowcut.setBounds(bound); bound.translate(80, 0);
        m_fbHighCut.setBounds(bound);

        bound = juce::Rectangle(0, 80, 70, 70);
        m_mix.setBounds(bound); bound.translate(80, 0);
        m_rate.setBounds(bound); bound.translate(80, 0);
        m_barberpoleRate.setBounds(bound); bound.translate(80, 0);
        m_depth.setBounds(bound); bound.translate(80, 0);
        m_barberpolePhase.setBounds(bound);

        bound = juce::Rectangle(0, 160, 70, 70);
        m_lfoPhase.setBounds(bound); bound.translate(80, 0);
        m_lfoShape.setBounds(bound);

        m_disableBarber.setBounds(0, 215, 200, 40);
    }

    //================================================================================
    // implement for ContainModulableComponent
    //================================================================================
    void showModulationFrom(audio::ModulatorBase* p) override {
        m_tzfDelay.showModulationFrom(p);
        m_delay.showModulationFrom(p);
        m_fb.showModulationFrom(p);
        m_fbLowcut.showModulationFrom(p);
        m_fbHighCut.showModulationFrom(p);
        m_mix.showModulationFrom(p);
        m_rate.showModulationFrom(p);
        m_barberpoleRate.showModulationFrom(p);
        m_depth.showModulationFrom(p);
        m_barberpolePhase.showModulationFrom(p);
        m_lfoPhase.showModulationFrom(p);
        m_lfoShape.showModulationFrom(p);
    }
private:
    //================================================================================
    // GUI controllers
    //================================================================================
    using Knob = ui::FloatKnob;
    Knob m_tzfDelay;
    Knob m_delay;
    Knob m_fb;
    Knob m_fbLowcut;
    Knob m_fbHighCut;
    Knob m_mix;
    Knob m_rate;
    Knob m_barberpoleRate;
    Knob m_depth;
    Knob m_barberpolePhase;
    Knob m_lfoPhase;
    Knob m_lfoShape;
    juce::ToggleButton m_disableBarber;
    juce::ButtonParameterAttachment m_attach;
};

//================================================================================
// Flanger Impl
//================================================================================
class FlangerImpl {
public:
    // 镶边器延迟和ZDF延迟平滑时间
    static constexpr double kDelaySmoothTime = 0.1;

    //================================================================================
    // 正弦三角LFO
    //================================================================================
    class TriSineShapeLFO {
    public:
        void prepare(FType sr) {
            m_phase = FType{};
            m_oneDivSampleRate = 1 / sr;
        }

        /**
         * @brief 让LFO在CR模式下运行一次
         * @param fre 频率
         * @param step CR两个时钟之间的时间差在SR下的采样数差
         * @param phase 额外的相位增加
         * @param shape 形状,[0,1]从三角线性插值到正弦
         * @return 左:无额外相位增加;右:有额外相位增加;均为[-1,1]
        */
        PolyFType CRTick(FType fre, size_t step, FType phase, FType shape) {
            auto phaseAdd = step * fre * m_oneDivSampleRate;
            m_phase += phaseAdd;
            m_phase = std::fmod(m_phase, static_cast<FType>(1));
            auto ppp = std::fmod(m_phase + phase, static_cast<FType>(1));

            PolyFType LR{};
            LR.left = triToSin(m_phase, shape);
            LR.right = triToSin(ppp, shape);
            return LR;
        }
    private:
        FType triToSin(FType phase, FType shape) {
            auto sinVal = std::cos(juce::MathConstants<FType>::twoPi * phase);
            auto triVal = static_cast<FType>(2)
                * std::abs(static_cast<FType>(2) * phase - static_cast<FType>(1))
                - static_cast<FType>(1);
            return shape * triVal + (static_cast<FType>(1) - shape) * sinVal;
        }

        FType m_oneDivSampleRate{};
        FType m_phase{};
    };
    //================================================================================
    // Dual Oscillor
    //================================================================================
    class SinCosLFO {
    public:
        void prepare(FType sr) {
            m_oneDivSampleRate = 1 / sr;
        }

        PolyFType SRTick(FType fre, FType additionalPhase) {
            auto phaseAdd = fre * m_oneDivSampleRate;
            m_phase += phaseAdd;
            m_phase = std::fmod(m_phase, static_cast<FType>(1));
            auto ppp = std::fmod(m_phase + additionalPhase, static_cast<FType>(1));

            PolyFType pp{};
            pp.left = juce::dsp::FastMathApproximations::cos(juce::MathConstants<FType>::twoPi * ppp);
            pp.right = juce::dsp::FastMathApproximations::sin(juce::MathConstants<FType>::twoPi * ppp);
            return pp;
        }
    private:
        FType m_oneDivSampleRate{};
        FType m_phase{};
    };

    FlangerImpl(FlangerParameters& e) :p(e) {};
    void prepare(FType sr, size_t num) {
        m_srDiv1000 = sr / FType{1000};
        juce::dsp::ProcessSpec spec{};
        spec.sampleRate = sr;
        spec.numChannels = 2;
        spec.maximumBlockSize = static_cast<juce::uint32>(num);

        m_TZFdelayLine.prepare(spec);
        m_delayLine.prepare(spec);
        fbLF.prepare(spec);
        fbHF.prepare(spec);

        // Juce::smoothValue会超出设定最大值要有额外的空间
        constexpr int extraSample = 8;
        auto maxDelayInSample = static_cast<int>(m_srDiv1000 * p.delayTime.getHostParameter()->range.end
                                                 + m_srDiv1000 * p.depth.getHostParameter()->range.end);
        m_delayLine.setMaximumDelayInSamples(maxDelayInSample + extraSample);
        auto tzfMaxDelayInSample = static_cast<int>(m_srDiv1000 * p.TZFDelayTime.getHostParameter()->range.end);
        m_TZFdelayLine.setMaximumDelayInSamples(tzfMaxDelayInSample + extraSample);

        m_leftDelaySmoother.reset(sr, kDelaySmoothTime);
        m_rightDelaySmoother.reset(sr, kDelaySmoothTime);
        m_leftBarberpoleLFO.prepare(sr);
        m_rightBarberpoleLFO.prepare(sr);
        m_mainDelayLFO.prepare(sr);
    }

    void process(StereoBuffer& buffer, size_t begin, size_t end) {
        if (p.disableBarberpole->get()) {
            processChannelWithoutHilbert(buffer, begin, end);
        } else {
            processChannel(buffer, begin, end);
        }
    }

    void onCRClock(size_t n) {
        // Feedback filter
        fbLF.setCutoffFrequency(p.fbLowCut.getTargetValue());
        fbHF.setCutoffFrequency(p.fbHighCut.getTargetValue());

        // tzf delay
        FType tzfDelayInSample = p.TZFDelayTime.getTargetValue() * m_srDiv1000;
        m_TZFDelaySmoother.setTargetValue(tzfDelayInSample);

        // delay
        FType mainDelayInSample = p.delayTime.getTargetValue() * m_srDiv1000;
        FType depthInSample = p.depth.getTargetValue() * m_srDiv1000;
        PolyFType LRPhase = m_mainDelayLFO.CRTick(p.rate.getTargetValue(), n, p.lfoPhase.getTargetValue(), p.lfoShape.getTargetValue());
        FType lDelay = juce::jmax(FType{}, mainDelayInSample + LRPhase.left * depthInSample);
        FType rDelay = juce::jmax(FType{}, mainDelayInSample + LRPhase.right * depthInSample);
        m_leftDelaySmoother.setTargetValue(lDelay);
        m_rightDelaySmoother.setTargetValue(rDelay);
    }

    //================================================================================
    // Barberpole Flanger                +-> out
    //                                   |
    // in-->TZFDelay------------------->Mix
    // ↓                                 ↑       
    // Add--->Delay--->SSBFreShift-------+       
    // ↑       ↑          ↑              |       
    // |      LFO        LFO          Feedback
    // +---------------------------------+
    //================================================================================
    void processChannel(StereoBuffer& data, size_t begin, size_t end) {
        for (size_t i = begin; i < end; i++) {
            PolyFType sample = data[i];
            PolyFType fbVal = juce::jlimit(FType{-0.9}, FType{0.9}, p.feedback.getNextValue()) 
                * m_fbValue;
            m_TZFdelayLine.pushSample(0, sample.left);
            m_TZFdelayLine.pushSample(1, sample.right);
            FType tzfDelaySample = m_TZFDelaySmoother.getNextValue();
            PolyFType tzfout{};
            tzfout.left = m_TZFdelayLine.popSample(0, tzfDelaySample);
            tzfout.right = m_TZFdelayLine.popSample(1, tzfDelaySample);

            m_delayLine.pushSample(0, sample.left + fbVal.left);
            m_delayLine.pushSample(1, sample.right + fbVal.right);
            PolyFType delayout{};
            delayout.left = m_delayLine.popSample(0, m_leftDelaySmoother.getNextValue());
            delayout.right = m_delayLine.popSample(1, m_rightDelaySmoother.getNextValue());

            PolyFType hilbertLout{};
            PolyFType hilbertRout{};
            hilbertL.process(delayout.left, &hilbertLout.left, &hilbertLout.right);
            hilbertR.process(delayout.right, &hilbertRout.left, &hilbertRout.right);
            
            FType barberRate = p.barberpoleRate.getNextValue();
            FType barberPhase = p.barberpolePhase.getNextValue();
            PolyFType sincosL = m_leftBarberpoleLFO.SRTick(barberRate, FType{});
            PolyFType sincosR = m_rightBarberpoleLFO.SRTick(barberRate, barberPhase);

            PolyFType hilbertOut{};
            hilbertOut.left = sincosL.right * hilbertLout.left + sincosL.left * hilbertLout.right;
            hilbertOut.right = sincosR.right * hilbertRout.left + sincosR.left * hilbertRout.right;
            auto mixout = tzfout + p.mix.getNextValue() * hilbertOut;

            m_fbValue.left = fbLF.processSample(0, fbHF.processSample(0, hilbertOut.left));
            m_fbValue.right = fbLF.processSample(1, fbHF.processSample(1, hilbertOut.right));
            fbLF.snapToZero();
            fbHF.snapToZero();

            data[i] = mixout;
        }
    }

    //================================================================================
    // TZF Flanger                       +-> out
    //                                   |
    // in-->TZFDelay--------------------Mix
    // ↓                                 ↑ 
    // Add--->Delay----------------------+
    // ↑       ↑                         ↓       
    // |      LFO                    Feedback
    // +---------------------------------+
    //================================================================================
    void processChannelWithoutHilbert(StereoBuffer& data, size_t begin, size_t end) {
        for (size_t i = begin; i < end; i++) {
            PolyFType sample = data[i];
            m_TZFdelayLine.pushSample(0, sample.left);
            m_TZFdelayLine.pushSample(1, sample.right);

            PolyFType tzfout{};
            FType tzfDelaySample = m_TZFDelaySmoother.getNextValue();
            tzfout.left = m_TZFdelayLine.popSample(0, tzfDelaySample);
            tzfout.right = m_TZFdelayLine.popSample(1, tzfDelaySample);

            PolyFType withFb = sample + p.feedback.getNextValue() * m_fbValue;
            m_delayLine.pushSample(0, withFb.left);
            m_delayLine.pushSample(1, withFb.right);

            PolyFType delayout{};
            delayout.left = m_delayLine.popSample(0, m_leftDelaySmoother.getNextValue());
            delayout.right = m_delayLine.popSample(1, m_rightDelaySmoother.getNextValue());

            PolyFType mixout = tzfout + p.mix.getNextValue() * delayout;

            m_fbValue.left = fbLF.processSample(0, fbHF.processSample(0, delayout.left));
            m_fbValue.right = fbLF.processSample(1, fbHF.processSample(1, delayout.right));
            fbLF.snapToZero();
            fbHF.snapToZero();

            data[i] = mixout;
        }
    }
private:
    // LFO
    TriSineShapeLFO m_mainDelayLFO;
    SinCosLFO m_leftBarberpoleLFO;
    SinCosLFO m_rightBarberpoleLFO;

    // 线性平滑器
    juce::SmoothedValue<FType> m_leftDelaySmoother;
    juce::SmoothedValue<FType> m_rightDelaySmoother;
    juce::SmoothedValue<FType> m_TZFDelaySmoother;

    FlangerParameters& p;
    FType m_srDiv1000{};
    PolyFType m_fbValue{};
    juce::dsp::DelayLine<FType> m_delayLine;
    juce::dsp::DelayLine<FType> m_TZFdelayLine;
    juce::dsp::FirstOrderTPTFilter<FType> fbLF;
    juce::dsp::FirstOrderTPTFilter<FType> fbHF;
    IIRHilbertTransformer<FType, 4> hilbertL{IIRHilbertCoeffect::kCoeffects1<FType>};
    IIRHilbertTransformer<FType, 4> hilbertR{IIRHilbertCoeffect::kCoeffects1<FType>};
};
}

//================================================================================
// Flanger
//================================================================================
namespace rpSynth::audio::effects {
Flanger::Flanger(OrderableEffectsChain& c)
    : EffectProcessorBase(c, "Flanger") {
    m_allFlangerParameters = std::make_unique<FlangerParameters>();
    m_flangerImpl = std::make_unique<FlangerImpl>(*m_allFlangerParameters);
}

Flanger::~Flanger() {
    m_allFlangerParameters = nullptr;
    m_flangerImpl = nullptr;
}

void Flanger::addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    EffectProcessorBase::addParameterToLayout(layout);
    layout.add(
        std::make_unique<MyHostParameter>(m_allFlangerParameters->TZFDelayTime,
                                                          combineWithID("TZFDelay"),
                                                          "TZFDelay",
                                                          juce::NormalisableRange(0.f, 10.f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->delayTime,
                                                          combineWithID("Delay"),
                                                          "Delay",
                                                          juce::NormalisableRange(0.f, 10.f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->feedback,
                                                          combineWithID("Feedback"),
                                                          "Feedback",
                                                          juce::NormalisableRange(-0.99f, 0.99f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->fbLowCut,
                                                          combineWithID("FB_LowCut"),
                                                          "FB_LowCut",
                                                          juce::NormalisableRange(20.f, 20000.f, 1.f),
                                                          20000.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->fbHighCut,
                                                          combineWithID("FB_HighCut"),
                                                          "FB_HighCut",
                                                          juce::NormalisableRange(20.f, 20000.f, 1.f),
                                                          20.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->mix,
                                                          combineWithID("mix"),
                                                          "mix",
                                                          juce::NormalisableRange<float>(-1.f, 1.f, 0.01f),
                                                          1.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->rate,
                                                          combineWithID("Rate"),
                                                          "Rate",
                                                          juce::NormalisableRange(-2.f, 2.f, 0.1f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->barberpoleRate,
                                                          combineWithID("BarberRate"),
                                                          "BarberRate",
                                                          juce::NormalisableRange(-10.f, 10.f, 0.1f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->depth,
                                                          combineWithID("Depth"),
                                                          "Depth",
                                                          juce::NormalisableRange(0.f, 10.f, 0.1f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->barberpolePhase,
                                                          combineWithID("BarberPhase"),
                                                          "BarberPhase",
                                                          juce::NormalisableRange(0.f, 1.f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->lfoPhase,
                                                          combineWithID("LFOPhase"),
                                                          "Phase",
                                                          juce::NormalisableRange(0.f, 1.f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostParameter>(m_allFlangerParameters->lfoShape,
                                                          combineWithID("LFOShape"),
                                                          "LFOShape",
                                                          juce::NormalisableRange(0.f, 1.f, 0.01f),
                                                          0.f)
    );

    auto pDisable = std::make_unique<juce::AudioParameterBool>(combineWithID("disBarber"),
                                                               "Disable Barberpole",
                                                               true);
    m_allFlangerParameters->disableBarberpole = pDisable.get();
    layout.add(std::move(pDisable));
}

//void Flanger::updateParameters(size_t numSamples) {
//    m_allFlangerParameters->updateAll(numSamples);
//}

void Flanger::prepareParameters(FType sampleRate, size_t numSamples) {
    m_allFlangerParameters->prepareAll(sampleRate, numSamples);
}

void Flanger::prepare(FType sampleRate, size_t numSamlpes) {
    m_flangerImpl->prepare(sampleRate, numSamlpes);
}

void Flanger::saveExtraState(juce::XmlElement& /*xml*/) {
}

void Flanger::loadExtraState(juce::XmlElement& /*xml*/, juce::AudioProcessorValueTreeState& /*apvts*/) {
}

void Flanger::processBlock(StereoBuffer& block, size_t begin, size_t end) {
    m_flangerImpl->process(block, begin, end);
}

std::unique_ptr<ui::ContainModulableComponent> Flanger::createEffectPanel() {
    return std::make_unique<FlangerPanel>(*m_allFlangerParameters);
}

void Flanger::onCRClock(size_t n) {
    m_allFlangerParameters->onCRClock();
    m_flangerImpl->onCRClock(n);
}
}