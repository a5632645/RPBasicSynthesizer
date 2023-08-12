/*
  ==============================================================================

    Flanger.cpp
    Created: 8 Aug 2023 9:38:15am
    Author:  mana

  ==============================================================================
*/

#include "Flanger.h"
#include "synthesizer/WrapParameter.h"
#include "dsps/IIRHilbertTransform.h"

#include "ui/controller/FloatKnob.h"

//================================================================================
// 全部镶边器的参数
//================================================================================
namespace rpSynth::audio::effects {
struct FlangerParameters {
    MyAudioProcessParameter TZFDelayTime;                 // TZF对干信号延迟[0,10]ms
    MyAudioProcessParameter delayTime;                    // 延迟时间[0,10]ms
    MyAudioProcessParameter feedback;                     // 反馈[-1,1]
    MyAudioProcessParameter fbLowCut;                     // 反馈低切[20,20000]hz
    MyAudioProcessParameter fbHighCut;                    // 反馈高切[20,20000]hz
    MyAudioProcessParameter mix;                          // 混合[-1,1]
    MyAudioProcessParameter rate;                         // LFO速率[0,5]hz
    MyAudioProcessParameter barberpoleRate;               // 另一个LFO速率[-10,10]hz
    MyAudioProcessParameter depth;                        // 深度[0,1]
    MyAudioProcessParameter barberpolePhase;              // barberpole的LFO相位[0,1]
    MyAudioProcessParameter lfoPhase;                     // LFO相差相位[0,1]
    MyAudioProcessParameter lfoShape;                     // LFO形状从三角波到正弦波[0,1]
    juce::AudioParameterBool* disableBarberpole = nullptr;// 禁用希尔伯特导致的相位问题

    // Merge all
    void prepareAll(FType sr, size_t num) {
        TZFDelayTime.prepare(sr, num);
        delayTime.prepare(sr, num);
        feedback.prepare(sr, num);
        fbLowCut.prepare(sr, num);
        fbHighCut.prepare(sr, num);
        mix.prepare(sr, num);
        rate.prepare(sr, num);
        barberpoleRate.prepare(sr, num);
        depth.prepare(sr, num);
        barberpolePhase.prepare(sr, num);
        lfoPhase.prepare(sr, num);
        lfoShape.prepare(sr, num);
    }

    void updateAll(size_t num) {
        TZFDelayTime.updateParameter(num);
        delayTime.updateParameter(num);
        feedback.updateParameter(num);
        fbLowCut.updateParameter(num);
        fbHighCut.updateParameter(num);
        mix.updateParameter(num);
        rate.updateParameter(num);
        barberpoleRate.updateParameter(num);
        depth.updateParameter(num);
        barberpolePhase.updateParameter(num);
        lfoPhase.updateParameter(num);
        lfoShape.updateParameter(num);
    }
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
        FPolyType CRTick(FType fre, size_t step, FType phase, FType shape) {
            auto phaseAdd = step * fre * m_oneDivSampleRate;
            m_phase += phaseAdd;
            m_phase = std::fmod(m_phase, static_cast<FType>(1));
            auto ppp = std::fmod(m_phase + phase, static_cast<FType>(1));

            FPolyType LR{};
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

        FPolyType SRTick(FType fre, FType additionalPhase) {
            auto phaseAdd = fre * m_oneDivSampleRate;
            m_phase += phaseAdd;
            m_phase = std::fmod(m_phase, static_cast<FType>(1));
            auto ppp = std::fmod(m_phase + additionalPhase, static_cast<FType>(1));

            FPolyType pp{};
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
        // Feedback filter
        fbLF.setCutoffFrequency(p.fbLowCut.get(begin));
        fbHF.setCutoffFrequency(p.fbHighCut.get(begin));

        // delay
        FType tzfDelayInSample = p.TZFDelayTime.get(begin) * m_srDiv1000;
        FType mainDelayInSample = p.delayTime.get(begin) * m_srDiv1000;
        FType depthInSample = p.depth.get(begin) * m_srDiv1000;
        FPolyType LRPhase = m_mainDelayLFO.CRTick(p.rate.get(begin), end - begin, p.lfoPhase.get(begin), p.lfoShape.get(begin));
        FType lDelay = juce::jmax(FType{}, mainDelayInSample + LRPhase.left * depthInSample);
        FType rDelay = juce::jmax(FType{}, mainDelayInSample + LRPhase.right * depthInSample);
        setDelayTime<0>(lDelay, tzfDelayInSample);
        setDelayTime<1>(rDelay, tzfDelayInSample);

        if (p.disableBarberpole->get()) {
            processChannelWithoutHilbert<0>(buffer.left, begin, end);
            processChannelWithoutHilbert<1>(buffer.right, begin, end);
        } else {
            processChannel<0>(buffer.left, begin, end);
            processChannel<1>(buffer.right, begin, end);
        }
    }

    //================================================================================
    // Barberpole Flanger                +-> out
    //                                   |
    // in-->TZFDelay------------------->Mix<-----+
    // ↓                                 ↑       |
    // Add--->Delay--->SSBFreShift-------+       |
    // ↑       ↑          ↑              |       |
    // |      LFO        LFO          Feedback---+
    // +---------------------------------+
    //================================================================================
    template<size_t channel>
    void processChannel(std::vector<FType>& data, size_t begin, size_t end) {
        for (size_t i = begin; i < end; i++) {
            FType sample = data[i];
            auto fbVal = juce::jlimit(FType{-0.9}, FType{0.9}, p.feedback.get(i)) 
                * getFeedback<channel>();
            m_TZFdelayLine.pushSample(channel, sample);
            FType tzfout = m_TZFdelayLine.popSample(channel, getTZFDelayTime<channel>());
            m_delayLine.pushSample(channel, sample + fbVal);
            auto delayout = m_delayLine.popSample(channel, getDelayTime<channel>());

            FPolyType hilbertMid = hilbert<channel>(delayout);
            auto sincos = getBarberLFO<channel>(p.barberpoleRate.get(i), p.barberpolePhase.get(i));
            auto hilbertOut = sincos.right * hilbertMid.left + sincos.left * hilbertMid.right;
            auto mixout = tzfout + p.mix.get(i) * hilbertOut + fbVal;

            fbUpdate<channel>(hilbertOut);
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
    template<size_t channel>
    void processChannelWithoutHilbert(std::vector<FType>& data, size_t begin, size_t end) {
        for (size_t i = begin; i < end; i++) {
            FType sample = data[i];
            m_TZFdelayLine.pushSample(channel, sample);
            FType tzfout = m_TZFdelayLine.popSample(channel, getTZFDelayTime<channel>());

            auto withFb = sample + p.feedback.get(i) * getFeedback<channel>();
            m_delayLine.pushSample(channel, withFb);
            auto delayout = m_delayLine.popSample(channel, getDelayTime<channel>());
            auto mixout = tzfout + p.mix.get(i) * delayout;

            fbUpdate<channel>(delayout);
            data[i] = mixout;
        }
    }

private:
    template<size_t channel>
    void setDelayTime(FType mainD, FType tzfD) {
        if constexpr (channel == 0) {
            m_leftDelaySmoother.setTargetValue(mainD);
            m_leftTZFDelaySmoother.setTargetValue(tzfD);
        } else {
            m_rightDelaySmoother.setTargetValue(mainD);
            m_rightTZFDelaySmoother.setTargetValue(tzfD);
        }
    }

    template<size_t channel>
    FPolyType getBarberLFO(FType fre, FType additionalPhase) {
        if constexpr (channel == 0) {
            return m_leftBarberpoleLFO.SRTick(fre, FType{});
        } else {
            return m_rightBarberpoleLFO.SRTick(fre, additionalPhase);
        }
    }

    template<size_t channel>
    FType getDelayTime() {
        if constexpr (channel == 0) {
            return m_leftDelaySmoother.getNextValue();
        } else {
            return m_rightDelaySmoother.getNextValue();
        }
    }

    template<size_t channel>
    FType getTZFDelayTime() {
        if constexpr (channel == 0) {
            return m_leftTZFDelaySmoother.getNextValue();
        } else {
            return m_rightTZFDelaySmoother.getNextValue();
        }
    }

    template<size_t channel>
    FType getFeedback() {
        if constexpr (channel == 0) {
            return m_fbValue.left;
        } else {
            return m_fbValue.right;
        }
    }

    template<size_t channel>
    void fbUpdate(FType s) {
        if constexpr (channel == 0) {
            m_fbValue.left = fbHF.processSample(channel, fbLF.processSample(channel, s));
        } else {
            m_fbValue.right = fbHF.processSample(channel, fbLF.processSample(channel, s));
        }
    }

    template<size_t channel>
    FPolyType hilbert(FType s) {
        FPolyType ss{};
        if constexpr (channel == 0) {
            hilbertL.process(s, &ss.left, &ss.right);
        } else {
            hilbertR.process(s, &ss.left, &ss.right);
        }
        return ss;
    }

private:
    // LFO
    TriSineShapeLFO m_mainDelayLFO;
    SinCosLFO m_leftBarberpoleLFO;
    SinCosLFO m_rightBarberpoleLFO;

    // 线性平滑器
    juce::SmoothedValue<FType> m_leftDelaySmoother;
    juce::SmoothedValue<FType> m_rightDelaySmoother;
    juce::SmoothedValue<FType> m_leftTZFDelaySmoother;
    juce::SmoothedValue<FType> m_rightTZFDelaySmoother;

    FlangerParameters& p;
    FType m_srDiv1000{};
    FPolyType m_fbValue{};
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
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->TZFDelayTime,
                                                          combineWithID("TZFDelay"),
                                                          "TZFDelay",
                                                          juce::NormalisableRange(0.f, 10.f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->delayTime,
                                                          combineWithID("Delay"),
                                                          "Delay",
                                                          juce::NormalisableRange(0.f, 10.f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->feedback,
                                                          combineWithID("Feedback"),
                                                          "Feedback",
                                                          juce::NormalisableRange(-0.99f, 0.99f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->fbLowCut,
                                                          combineWithID("FB_LowCut"),
                                                          "FB_LowCut",
                                                          juce::NormalisableRange(20.f, 20000.f, 1.f),
                                                          20000.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->fbHighCut,
                                                          combineWithID("FB_HighCut"),
                                                          "FB_HighCut",
                                                          juce::NormalisableRange(20.f, 20000.f, 1.f),
                                                          20.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->mix,
                                                          combineWithID("mix"),
                                                          "mix",
                                                          juce::NormalisableRange<float>(-1.f, 1.f, 0.01f),
                                                          1.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->rate,
                                                          combineWithID("Rate"),
                                                          "Rate",
                                                          juce::NormalisableRange(-2.f, 2.f, 0.1f),
                                                          0.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->barberpoleRate,
                                                          combineWithID("BarberRate"),
                                                          "BarberRate",
                                                          juce::NormalisableRange(-10.f, 10.f, 0.1f),
                                                          0.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->depth,
                                                          combineWithID("Depth"),
                                                          "Depth",
                                                          juce::NormalisableRange(0.f, 10.f, 0.1f),
                                                          0.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->barberpolePhase,
                                                          combineWithID("BarberPhase"),
                                                          "BarberPhase",
                                                          juce::NormalisableRange(0.f, 1.f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->lfoPhase,
                                                          combineWithID("LFOPhase"),
                                                          "Phase",
                                                          juce::NormalisableRange(0.f, 1.f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->lfoShape,
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

void Flanger::updateParameters(size_t numSamples) {
    m_allFlangerParameters->updateAll(numSamples);
}

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
}