#pragma once
#include <JuceHeader.h>

class DioCompressProcessor : public juce::AudioProcessor
{
public:
    DioCompressProcessor();
    ~DioCompressProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void reset() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "DioCompress"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    juce::AudioProcessorParameter* getBypassParameter() const override { return apvts.getParameter ("bypass"); }

    // Presets (se ven como "programas" en el DAW): 0 = Init, luego perfil x topologia
    int getNumPrograms() override;
    int getCurrentProgram() override { return currentPreset; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    // Perfiles de fuente (sugerencias editables segun la topologia actual)
    void applyProfile (int profile);
    int getCurrentProfile() const { return currentProfile; }

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> grMeter  { 0.0f };    // reduccion de ganancia (dB, positivo)
    std::atomic<float> inMeter  { -80.0f };  // pico de entrada (dB)
    std::atomic<float> outMeter { -80.0f };  // pico de salida (dB)
    int savedEditorWidth = 770;              // ancho de la ventana (se guarda con el proyecto)

private:
    float currentMakeupDb() const;
    void clearState();
    void setParam (const char* id, float denormalisedValue);
    void applyProfileFor (int profile, int topology);
    void applyInit();

    std::atomic<float> *pTopo = nullptr, *pVmode = nullptr, *pIn = nullptr, *pThr = nullptr, *pRatio = nullptr,
                       *pAtk = nullptr, *pRel = nullptr, *pKnee = nullptr, *pMakeup = nullptr, *pMix = nullptr,
                       *pHpf = nullptr, *pOut = nullptr, *pCeil = nullptr, *pAutoMk = nullptr, *pBypass = nullptr,
                       *pSafety = nullptr, *pAnalog = nullptr, *pDrive = nullptr, *pColor = nullptr, *pAmix = nullptr;

    double fs = 44100.0;
    int currentPreset = 0, currentProfile = -1;

    // detector (sidechain con HPF, enlazado estereo)
    float hpState[2] { 0.f, 0.f };
    float ms20 = 0.f, ms40 = 0.f;

    // estados de las tres topologias (corren siempre; se mezclan con pesos suavizados)
    float vF = 0.f, vS = 0.f;                   // VCA
    float oF = 0.f, oS = 0.f, oMem = 0.f;       // OPTICO (celda con memoria)
    float mF = 0.f, mS = 0.f;                   // VARI-MU

    // ANALOG COLOR (independiente de la compresion)
    float aLp1[2] { 0.f, 0.f }, aLp2[2] { 0.f, 0.f };
    float aPU[2][2] {}, aPLn[2][2] {}, aDcX[2][2] {}, aDcY[2][2] {};
    float aMsIn = 0.f, aMsOut = 0.f;

    // seguridad (LIMIT con lookahead ~1.5 ms / CLIP suave) y retardo fijo de latencia
    static constexpr int delaySize = 1024;
    static constexpr int delayMask = delaySize - 1;
    std::vector<float> delayBuf[2], inDelay[2];
    std::vector<float> tgBuf, minBuf;
    int limLen = 72, limPos = 0, dDelay = 71, dWrite = 0;
    int tgActive = 0, minActive = 0;
    double minSum = 72.0;
    float limGain = 1.f;

    juce::SmoothedValue<float> smIn, smMix, smOut, smMakeup, smDrive, smColor, smAmix, smAnalog, smBypass,
                               smWV, smWO, smWM;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DioCompressProcessor)
};
