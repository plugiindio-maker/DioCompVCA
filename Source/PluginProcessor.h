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

    // ---- Presets de fabrica (tambien visibles como "programas" en el DAW): 0 = Init, luego perfil x topologia
    int getNumPrograms() override;
    int getCurrentProgram() override { return currentFactory; }
    void setCurrentProgram (int index) override { loadFactoryPreset (index); }
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}
    void loadFactoryPreset (int index);

    // ---- Presets de usuario (archivos en disco, persisten entre sesiones y DAWs)
    static juce::File getUserPresetDir();
    juce::StringArray getUserPresetNames() const;
    bool userPresetExists (const juce::String& name) const;
    bool saveUserPreset (const juce::String& name, bool overwrite);   // false si ya existe y overwrite = false, o si no se pudo escribir
    bool loadUserPreset (const juce::String& name);
    bool deleteUserPreset (const juce::String& name);
    bool renameUserPreset (const juce::String& oldName, const juce::String& newName);
    const juce::String& getCurrentPresetName() const { return currentPresetName; }
    bool isCurrentPresetUser() const { return currentPresetUser; }

    // ---- Perfiles de fuente (sugerencias editables para la topologia actual)
    void applyProfile (int profile);
    int getCurrentProfile() const { return currentProfile; }

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts;
    std::atomic<float> grMeter  { 0.0f };    // reduccion de ganancia (dB, positivo)
    std::atomic<float> inMeter  { -80.0f };  // pico de entrada (dB)
    std::atomic<float> outMeter { -80.0f };  // pico de salida (dB)

private:
    float currentInputDb() const;
    float currentMix() const;
    float currentPostGainDb() const;
    void clearState();
    void setParam (const char* id, float denormalisedValue);
    void applyProfileFor (int profile, int topology);
    void applyInit();
    void finishPresetLoad (const juce::String& name, bool user, int factoryIndex);

    struct Ptrs
    {
        std::atomic<float> *topo, *bypass, *safety, *ceil,
            *vmode, *vthr, *vratio, *vatk, *vrel, *vknee, *vmk, *vauto, *vhpf, *vmix,
            *opeak, *ogain, *olimit, *oemph, *oauto, *omix,
            *minput, *mthr, *mrec, *mbias, *mdens, *mout, *mauto, *mmix,
            *analog, *drive, *color, *amix, *bpon, *bpamt;
    } pr {};

    double fs = 44100.0;
    int currentFactory = 0, currentProfile = -1;
    juce::String currentPresetName { "Init" };
    bool currentPresetUser = false;

    // detectores (cada topologia tiene el suyo) + BASS PRESERVE
    float hpV[2] {}, hpD[2] {}, emL[2] {}, lpB1[2] {}, lpB2[2] {};
    float msV = 0.f, msO = 0.f, msM = 0.f;
    float grBp = 0.f;

    // estados de las tres topologias (corren siempre; se mezclan con pesos suavizados)
    float vF = 0.f, vS = 0.f;                   // VCA
    float oF = 0.f, oS = 0.f, oMem = 0.f;       // OPTICO (celda con memoria)
    float mF = 0.f, mS = 0.f;                   // VARI-MU

    // ANALOG COLOR (independiente de la compresion)
    float aLp1[2] {}, aLp2[2] {};
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

    juce::SmoothedValue<float> smIn, smMix, smMakeup, smDrive, smColor, smAmix, smAnalog, smBypass,
                               smWV, smWO, smWM, smBpOn, smBpAmt;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DioCompressProcessor)
};
