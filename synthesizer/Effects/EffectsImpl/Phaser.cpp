/*
  ==============================================================================

    Phaser.cpp
    Created: 8 Aug 2023 12:43:38pm
    Author:  mana

  ==============================================================================
*/

#include "Phaser.h"
#include "synthesizer/WrapParameter.h"
#include "dsps/AllPassFilter.h"
#include "dsps/IIRHilbertTransform.h"

#include "ui/controller/FloatKnob.h"

//================================================================================
// 全部镶边器的参数
//================================================================================
namespace rpSynth::audio::effects {
struct PhaserParameters {
    MyAudioProcessParameter beginSemitone;                // 最小频率[20,20000]hz
    MyAudioProcessParameter endSemitone;                  // 最大频率[20,20000]hz
    MyAudioProcessParameter spread;                       // 陷波间隔[0,0.5]
    MyAudioProcessParameter feedback;                     // 反馈[-1,1]
    MyAudioProcessParameter fbLowCut;                     // 反馈低切[20,20000]hz
    MyAudioProcessParameter fbHighCut;                    // 反馈高切[20,20000]hz
    MyAudioProcessParameter mix;                          // 混合[-1,1]
    MyAudioProcessParameter rate;                         // LFO速率[-2,2]hz
    MyAudioProcessParameter barberpoleRate;               // 另一个LFO速率[-10,10]hz
    MyAudioProcessParameter barberpolePhase;              // barberpole的LFO相位[0,1]
    MyAudioProcessParameter lfoPhase;                     // LFO相差相位[0,1]
    MyAudioProcessParameter lfoShape;                     // LFO形状从三角波到正弦波[0,1]
    MyAudioProcessParameter phaserState{false};           // 相位器阶数
    juce::AudioParameterBool* disableBarberpole = nullptr;// 禁用希尔伯特导致的相位问题

    // Merge all
    void prepareAll(FType sr, size_t num) {
        beginSemitone.prepare(sr, num);
        endSemitone.prepare(sr, num);
        feedback.prepare(sr, num);
        fbLowCut.prepare(sr, num);
        fbHighCut.prepare(sr, num);
        mix.prepare(sr, num);
        rate.prepare(sr, num);
        barberpoleRate.prepare(sr, num);
        barberpolePhase.prepare(sr, num);
        lfoPhase.prepare(sr, num);
        lfoShape.prepare(sr, num);
        phaserState.prepare(sr, num);
        spread.prepare(sr, num);
    }

    void updateAll(size_t num) {
        beginSemitone.updateParameter(num);
        endSemitone.updateParameter(num);
        feedback.updateParameter(num);
        fbLowCut.updateParameter(num);
        fbHighCut.updateParameter(num);
        mix.updateParameter(num);
        rate.updateParameter(num);
        barberpoleRate.updateParameter(num);
        barberpolePhase.updateParameter(num);
        lfoPhase.updateParameter(num);
        lfoShape.updateParameter(num);
        phaserState.updateParameter(num);
        spread.updateParameter(num);
    }
};

//================================================================================
// Flanger Panel
//================================================================================
class PhaserPanel : public ui::ContainModulableComponent {
public:
    //================================================================================
    // implement for juce::Component
    //================================================================================
    ~PhaserPanel() override = default;

    PhaserPanel(PhaserParameters& f)
        :m_tzfDelay(&f.beginSemitone)
        , m_delay(&f.endSemitone)
        , m_fb(&f.feedback)
        , m_fbLowcut(&f.fbLowCut)
        , m_fbHighCut(&f.fbHighCut)
        , m_mix(&f.mix)
        , m_rate(&f.rate)
        , m_barberpoleRate(&f.barberpoleRate)
        , m_barberpolePhase(&f.barberpolePhase)
        , m_lfoPhase(&f.lfoPhase)
        , m_attach(*f.disableBarberpole, m_disableBarber)
        , m_lfoShape(&f.lfoShape)
    ,m_phaserState(&f.phaserState)
    ,m_spread(&f.spread){
        addAndMakeVisible(m_tzfDelay);
        addAndMakeVisible(m_delay);
        addAndMakeVisible(m_fb);
        addAndMakeVisible(m_fbLowcut);
        addAndMakeVisible(m_fbHighCut);
        addAndMakeVisible(m_mix);
        addAndMakeVisible(m_rate);
        addAndMakeVisible(m_barberpoleRate);
        addAndMakeVisible(m_barberpolePhase);
        addAndMakeVisible(m_lfoPhase);
        m_disableBarber.setButtonText(f.disableBarberpole->name);
        addAndMakeVisible(m_disableBarber);
        addAndMakeVisible(m_lfoShape);
        addAndMakeVisible(m_phaserState);
        addAndMakeVisible(m_spread);
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
        m_barberpolePhase.setBounds(bound);

        bound = juce::Rectangle(0, 160, 70, 70);
        m_lfoPhase.setBounds(bound); bound.translate(80, 0);
        m_lfoShape.setBounds(bound); bound.translate(80, 0);
        m_spread.setBounds(bound); bound.translate(80, 0);
        m_phaserState.setBounds(bound);

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
        m_barberpolePhase.showModulationFrom(p);
        m_lfoPhase.showModulationFrom(p);
        m_lfoShape.showModulationFrom(p);
        m_spread.showModulationFrom(p);
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
    Knob m_barberpolePhase;
    Knob m_lfoPhase;
    Knob m_lfoShape;
    Knob m_phaserState;
    Knob m_spread;
    juce::ToggleButton m_disableBarber;
    juce::ButtonParameterAttachment m_attach;
};

//================================================================================
// Flanger Impl
//================================================================================
class PhaserImpl {
public:
    // 镶边器延迟和ZDF延迟平滑时间
    static constexpr double kDelaySmoothTime = 0.1;
    static constexpr size_t kMaxPhaserState=16;

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

    PhaserImpl(PhaserParameters& e) :p(e) {};
    void prepare(FType sr, size_t num) {
        m_sampleRate = sr;
        m_oneDivNyquistRate = 2 / sr;
        juce::dsp::ProcessSpec spec{};
        spec.sampleRate = sr;
        spec.numChannels = 2;
        spec.maximumBlockSize = static_cast<juce::uint32>(num);

        fbLF.prepare(spec);
        fbHF.prepare(spec);

        m_leftHertzSmoother.reset(sr, kDelaySmoothTime);
        m_rightHertzSmoother.reset(sr, kDelaySmoothTime);
        m_leftBarberpoleLFO.prepare(sr);
        m_rightBarberpoleLFO.prepare(sr);
        m_mainDelayLFO.prepare(sr);
    }

    void process(StereoBuffer& buffer, size_t begin, size_t end) {
        // Feedback filter
        fbLF.setCutoffFrequency(p.fbLowCut.get(begin));
        fbHF.setCutoffFrequency(p.fbHighCut.get(begin));

        // apf cutoff and quality
        FType beginHertz = semitoneToHertz(p.beginSemitone.get(begin));
        FType endHertz = semitoneToHertz(p.endSemitone.get(begin));
        FPolyType LRPhase = m_mainDelayLFO.CRTick(p.rate.get(begin), end - begin, p.lfoPhase.get(begin), p.lfoShape.get(begin));
        FType lHertz = juce::jmap(LRPhase.left, FType{-1}, FType{1}, beginHertz, endHertz);
        FType rHertz = juce::jmap(LRPhase.right, FType{-1}, FType{1}, beginHertz, endHertz);
        FType spread = p.spread.get(begin);
        m_APFCoeffects[0].setCenterFrequency(lHertz, m_sampleRate);
        m_APFCoeffects[0].setBandWidth(spread, m_sampleRate);
        m_APFCoeffects[1].setCenterFrequency(rHertz, m_sampleRate);
        m_APFCoeffects[1].setBandWidth(spread, m_sampleRate);
        int numState = static_cast<int>(p.phaserState.get(begin));
        
        if (p.disableBarberpole->get()) {
            processChannelWithoutHilbert<0>(buffer.left, begin, end, numState);
            processChannelWithoutHilbert<1>(buffer.right, begin, end, numState);
        } else {
            processChannel<0>(buffer.left, begin, end, numState);
            processChannel<1>(buffer.right, begin, end, numState);
        }
    }

    //================================================================================
    // Barberpole Phaser                 +-> out
    //                                   |
    // in------------------------------>Mix
    // ↓                                 ↑ 
    // Add--->APFS---->SSBFreShift-------+ 
    // ↑       ↑          ↑              | 
    // |      LFO        LFO          Feedback
    // +---------------------------------+
    //================================================================================
    template<size_t channel>
    void processChannel(std::vector<FType>& data, size_t begin, size_t end,int state) {
        for (size_t i = begin; i < end; i++) {
            FType sample = data[i];
            auto fbVal = juce::jlimit(FType{-0.9}, FType{0.9}, p.feedback.get(i))
                * getFeedback<channel>();
            auto input = sample + fbVal;
            for (int j = 0; j < state; j++) {
                input = m_APFArray[j].processSingle(m_APFCoeffects[channel], input, channel);
            }

            FPolyType hilbertMid = hilbert<channel>(input);
            auto sincos = getBarberLFO<channel>(p.barberpoleRate.get(i), p.barberpolePhase.get(i));
            auto hilbertOut = sincos.right * hilbertMid.left + sincos.left * hilbertMid.right;
            auto mixout = sample + p.mix.get(i) * hilbertOut + fbVal;

            fbUpdate<channel>(hilbertOut);
            data[i] = mixout;
        }
    }

    //================================================================================
    // Basic Phaser                       +-> out
    //                                   |
    // in-------------------------------Mix
    // ↓                                 ↑ 
    // Add--->APFS-----------------------+
    // ↑       ↑                         ↓       
    // |      LFO                    Feedback
    // +---------------------------------+
    //================================================================================
    template<size_t channel>
    void processChannelWithoutHilbert(std::vector<FType>& data, size_t begin, size_t end,int state) {
        for (size_t i = begin; i < end; i++) {
            FType sample = data[i];
            auto withFb = sample + p.feedback.get(i) * getFeedback<channel>();
            for (int j = 0; j < state; j++) {
                withFb = m_APFArray[j].processSingle(m_APFCoeffects[channel], withFb, channel);
            }
            auto mixout = sample + p.mix.get(i) * withFb;
            fbUpdate<channel>(withFb);
            data[i] = mixout;
        }
    }

private:
    template<size_t channel>
    void setSmoothFilterHertz(FType hz) {
        if constexpr (channel == 0) {
            m_leftHertzSmoother.setTargetValue(hz);
        } else {
            m_rightHertzSmoother.setTargetValue(hz);
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
            return m_leftHertzSmoother.getNextValue();
        } else {
            return m_rightHertzSmoother.getNextValue();
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
    juce::SmoothedValue<FType> m_leftHertzSmoother;
    juce::SmoothedValue<FType> m_rightHertzSmoother;
    juce::SmoothedValue<FType> m_leftTZFDelaySmoother;
    juce::SmoothedValue<FType> m_rightTZFDelaySmoother;

    PhaserParameters& p;
    FType m_oneDivNyquistRate{};
    FType m_sampleRate{};
    FPolyType m_fbValue{};

    // 反馈滤波器
    juce::dsp::FirstOrderTPTFilter<FType> fbLF;
    juce::dsp::FirstOrderTPTFilter<FType> fbHF;

    // 全通滤波器组
    std::array<SecondOrderAllPassFilter2<FType, 2>, kMaxPhaserState> m_APFArray;
    std::array<SecondOrderAllPassFilter2<FType, 2>::Coeffects, 2> m_APFCoeffects;

    // 希尔伯特变换器
    IIRHilbertTransformer<FType, 4> hilbertL{IIRHilbertCoeffect::kCoeffects1<FType>};
    IIRHilbertTransformer<FType, 4> hilbertR{IIRHilbertCoeffect::kCoeffects1<FType>};
};
}

//================================================================================
// Flanger
//================================================================================
namespace rpSynth::audio::effects {
Phaser::Phaser(OrderableEffectsChain& c)
    : EffectProcessorBase(c, "Phaser") {
    m_allFlangerParameters = std::make_unique<PhaserParameters>();
    m_flangerImpl = std::make_unique<PhaserImpl>(*m_allFlangerParameters);
}

Phaser::~Phaser() {
    m_allFlangerParameters = nullptr;
    m_flangerImpl = nullptr;
}

void Phaser::addParameterToLayout(juce::AudioProcessorValueTreeState::ParameterLayout& layout) {
    EffectProcessorBase::addParameterToLayout(layout);
    layout.add(
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->beginSemitone,
                                                          combineWithID("MinHertz"),
                                                          "MinHertz",
                                                          juce::NormalisableRange(kStOf20hz, kStOf20000hz, 0.01f),
                                                          hertzToSemitone(440.f),
                                                          g_PitchHertzFloatParameterAttribute),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->endSemitone,
                                                          combineWithID("MaxHertz"),
                                                          "MaxHertz",
                                                          juce::NormalisableRange(kStOf20hz, kStOf20000hz, 0.01f),
                                                          hertzToSemitone(880.f),
                                                          g_PitchHertzFloatParameterAttribute),
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
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->barberpolePhase,
                                                          combineWithID("BarberPhase"),
                                                          "BarberPhase",
                                                          juce::NormalisableRange(0.f, 0.5f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->lfoPhase,
                                                          combineWithID("LFOPhase"),
                                                          "Phase",
                                                          juce::NormalisableRange(0.f, 0.5f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->lfoShape,
                                                          combineWithID("LFOShape"),
                                                          "LFOShape",
                                                          juce::NormalisableRange(0.f, 1.f, 0.01f),
                                                          0.f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->spread,
                                                          combineWithID("Spread"),
                                                          "Spread",
                                                          juce::NormalisableRange(0.01f,0.49f,0.01f),
                                                          0.25f),
        std::make_unique<MyHostedAudioProcessorParameter>(&m_allFlangerParameters->phaserState,
                                                          combineWithID("State"),
                                                          "State",
                                                          juce::NormalisableRange<float>(0,PhaserImpl::kMaxPhaserState,1),
                                                          4.f)
    );

    auto pDisable = std::make_unique<juce::AudioParameterBool>(combineWithID("disBarber"),
                                                               "Disable Barberpole",
                                                               true);
    m_allFlangerParameters->disableBarberpole = pDisable.get();
    layout.add(std::move(pDisable));
}

void Phaser::updateParameters(size_t numSamples) {
    m_allFlangerParameters->updateAll(numSamples);
}

void Phaser::prepareParameters(FType sampleRate, size_t numSamples) {
    m_allFlangerParameters->prepareAll(sampleRate, numSamples);
}

void Phaser::prepare(FType sampleRate, size_t numSamlpes) {
    m_flangerImpl->prepare(sampleRate, numSamlpes);
}

void Phaser::saveExtraState(juce::XmlElement& /*xml*/) {
}

void Phaser::loadExtraState(juce::XmlElement& /*xml*/, juce::AudioProcessorValueTreeState& /*apvts*/) {
}

void Phaser::processBlock(StereoBuffer& block, size_t begin, size_t end) {
    m_flangerImpl->process(block, begin, end);
}

std::unique_ptr<ui::ContainModulableComponent> Phaser::createEffectPanel() {
    return std::make_unique<PhaserPanel>(*m_allFlangerParameters);
}
}