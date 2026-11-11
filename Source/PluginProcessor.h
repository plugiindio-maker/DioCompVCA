#pragma once
#include <JuceHeader.h>

class VCACompAudioProcessor : public juce::AudioProcessor
{
public:
    VCACompAudioProcessor();
    ~VCACompAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void reset() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "DioCompVCA"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // Presets de fabrica (se ven como "programas" en el DAW)
    int getNumPrograms() override;
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> grMeter  { 0.0f };    // reduccion de ganancia en dB (positivo)
    std::atomic<float> outMeter { -60.0f };  // nivel de salida (RMS) en dB
    int savedEditorWidth = 640;              // ancho de la ventana (se guarda con el proyecto)

private:
    float currentMakeupDb() const;
    void clearState();

    std::atomic<float> *pThr = nullptr, *pRatio = nullptr, *pAtk = nullptr, *pRel = nullptr,
                       *pKnee = nullptr, *pMakeup = nullptr, *pMix = nullptr, *pOut = nullptr,
                       *pHpf = nullptr, *pCeil = nullptr, *pAutoRel = nullptr, *pAutoMk = nullptr,
                       *pRms = nullptr, *pSafety = nullptr, *pBody = nullptr, *pWeight = nullptr,
                       *pFreq = nullptr, *pDrive = nullptr, *pPunch = nullptr, *pPresence = nullptr;

    double fs = 44100.0;
    int currentPreset = 0;

    // compresor
    float hpState[2] { 0.f, 0.f };
    float msState = 0.f;
    float grState = 0.f, grFast = 0.f, grSlow = 0.f;

    // caracter: body / weight (graves)
    float lp1[2] { 0.f, 0.f }, lp2[2] { 0.f, 0.f };
    float chMsIn = 0.f, chMsOut = 0.f;

    // caracter: drive (saturacion de medios/agudos con anti-aliasing)
    float dPrevU[2] { 0.f, 0.f }, dPrevLn[2] { 0.f, 0.f }, dcX[2] { 0.f, 0.f }, dcY[2] { 0.f, 0.f };
    float drvMsIn = 0.f, drvMsOut = 0.f;

    // punch (realce de transitorios) y presence (shelf de agudos)
    float pkFast = 0.f, pkSlow = 0.f, punchDb = 0.f;
    float pz1[2] { 0.f, 0.f }, pz2[2] { 0.f, 0.f };

    // seguridad: limitador con lookahead de ~1.5 ms (modo LIMIT) / clipper suave (modo CLIP)
    static constexpr int delaySize = 1024;
    static constexpr int delayMask = delaySize - 1;
    std::vector<float> delayBuf[2];
    std::vector<float> tgBuf, minBuf;
    int limLen = 72, limPos = 0, dDelay = 71, dWrite = 0;
    int tgActive = 0, minActive = 0;
    double minSum = 72.0;
    float limGain = 1.f;

    juce::SmoothedValue<float> smMix, smOut, smMakeup, smBody, smWeight, smDrive, smPunch;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VCACompAudioProcessor)
};
