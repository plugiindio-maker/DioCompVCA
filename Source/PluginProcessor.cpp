#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

using FParam = juce::AudioParameterFloat;
using NRange = juce::NormalisableRange<float>;
using FAttr  = juce::AudioParameterFloatAttributes;

// ------------------------------------------------------------------ presets
namespace
{
    constexpr int kNumPresetValues = 20;
    const char* const kPresetIds[kNumPresetValues] = {
        "thr", "ratio", "attack", "release", "knee", "makeup", "mix", "hpf", "out", "ceil",
        "autorel", "automk", "rms", "safety", "body", "weight", "freq", "drive", "punch", "presence" };

    struct DioPreset { const char* name; float v[kNumPresetValues]; };

    // safety: 0 = OFF, 1 = LIMIT, 2 = CLIP
    //                         thr    ratio atk   rel    knee mk  mix   hpf   out ceil    aRel aMk rms sfty body  wgt  freq  drv  pnch pres
    const DioPreset kPresets[] = {
        { "Default",          { -18.f, 4.f,  10.f, 120.f,  6.f, 0.f, 100.f,  20.f, 0.f, -0.3f, 0.f, 1.f, 0.f, 0.f,  0.f,  0.f, 100.f,  0.f,  0.f,  0.f } },
        { "Vocal",            { -22.f, 3.5f, 12.f, 140.f,  9.f, 0.f, 100.f, 100.f, 0.f, -0.3f, 1.f, 1.f, 0.f, 1.f,  8.f,  0.f, 120.f, 25.f, 15.f, 35.f } },
        { "Drums",            { -16.f, 4.f,  25.f,  80.f,  4.f, 0.f,  80.f,  80.f, 0.f, -0.3f, 0.f, 1.f, 0.f, 2.f, 20.f, 10.f,  90.f, 35.f, 55.f, 25.f } },
        { "Mix Bus Glue",     { -14.f, 2.f,  30.f, 150.f, 10.f, 0.f, 100.f,  80.f, 0.f, -0.3f, 1.f, 1.f, 1.f, 1.f,  0.f,  0.f, 100.f, 15.f, 20.f, 15.f } },
        { "Synths / Keys",    { -18.f, 3.f,  15.f, 150.f,  8.f, 0.f,  85.f,  60.f, 0.f, -0.3f, 0.f, 1.f, 0.f, 0.f, 15.f,  0.f, 120.f, 30.f, 25.f, 30.f } },
        { "808 Punch",        { -20.f, 5.f,  25.f, 220.f,  4.f, 0.f, 100.f,  30.f, 0.f, -0.5f, 0.f, 1.f, 1.f, 0.f, 40.f, 25.f,  90.f, 20.f, 50.f, 15.f } },
        { "808 Heavy",        { -22.f, 6.f,  20.f, 280.f,  3.f, 0.f, 100.f,  30.f, 0.f, -0.5f, 0.f, 1.f, 1.f, 0.f, 70.f, 55.f,  80.f, 30.f, 35.f, 10.f } },
        { "Reese / Moog Bass",{ -20.f, 3.5f, 20.f, 160.f,  6.f, 0.f, 100.f,  40.f, 0.f, -0.5f, 0.f, 1.f, 1.f, 0.f, 25.f, 20.f, 110.f, 35.f, 15.f, 10.f } },
        { "Trap In Your Face",{ -20.f, 4.f,  30.f, 150.f,  4.f, 0.f,  90.f,  60.f, 0.f, -0.5f, 0.f, 1.f, 0.f, 2.f, 45.f, 30.f,  95.f, 55.f, 60.f, 45.f } },
    };
    constexpr int kNumPresets = (int) (sizeof (kPresets) / sizeof (kPresets[0]));

    inline float lnCosh (float u)
    {
        const float a = std::abs (u);
        return a + std::log1p (std::exp (-2.f * a)) - 0.69314718f;   // ln(cosh(u)) estable
    }

    inline float softClip (float x, float ceilLin)
    {
        const float t = 0.7f * ceilLin, ax = std::abs (x);
        if (ax <= t) return x;                                       // por debajo del 70% no toca nada (transitorios intactos)
        const float r = ceilLin - t;
        const float y = t + r * std::tanh ((ax - t) / r);            // nunca pasa del ceiling
        return x < 0.f ? -y : y;
    }
}

int VCACompAudioProcessor::getNumPrograms() { return kNumPresets; }

const juce::String VCACompAudioProcessor::getProgramName (int index)
{
    return (index >= 0 && index < kNumPresets) ? juce::String (kPresets[index].name) : juce::String();
}

void VCACompAudioProcessor::setCurrentProgram (int index)
{
    index = juce::jlimit (0, kNumPresets - 1, index);
    currentPreset = index;
    for (int i = 0; i < kNumPresetValues; ++i)
        if (auto* p = apvts.getParameter (kPresetIds[i]))
            p->setValueNotifyingHost (p->convertTo0to1 (kPresets[index].v[i]));
}

// ------------------------------------------------------------------ parametros
juce::AudioProcessorValueTreeState::ParameterLayout VCACompAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    auto d0 = [] (float v, int) { return juce::String (juce::roundToInt (v)); };
    auto d1 = [] (float v, int) { return juce::String (v, 1); };

    l.add (std::make_unique<FParam> (juce::ParameterID { "thr", 1 },    "Threshold", NRange (-60.f, 0.f, 0.1f), -18.f, FAttr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "ratio", 1 },  "Ratio",     NRange (1.f, 20.f, 0.1f, 0.5f), 4.f, FAttr().withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "attack", 1 }, "Attack",    NRange (0.1f, 100.f, 0.01f, 0.4f), 10.f, FAttr().withLabel ("ms").withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "release", 1 },"Release",   NRange (10.f, 1500.f, 0.1f, 0.4f), 120.f, FAttr().withLabel ("ms").withStringFromValueFunction (d0)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "knee", 1 },   "Knee",      NRange (0.f, 18.f, 0.1f), 6.f, FAttr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "makeup", 1 }, "Makeup",    NRange (-12.f, 24.f, 0.1f), 0.f, FAttr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "mix", 1 },    "Mix",       NRange (0.f, 100.f, 1.f), 100.f, FAttr().withLabel ("%").withStringFromValueFunction (d0)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "hpf", 1 },    "SC HPF",    NRange (20.f, 500.f, 1.f, 0.4f), 20.f, FAttr().withLabel ("Hz").withStringFromValueFunction (d0)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "out", 1 },    "Output",    NRange (-12.f, 12.f, 0.1f), 0.f, FAttr().withLabel ("dB").withStringFromValueFunction (d1)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "ceil", 1 },   "Ceiling",   NRange (-12.f, 0.f, 0.1f), -0.3f, FAttr().withLabel ("dB").withStringFromValueFunction (d1)));

    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "autorel", 1 }, "Auto Release", false));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "automk", 1 },  "Auto Makeup",  true));
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "rms", 1 },     "RMS Detect",   false));
    l.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { "safety", 1 }, "Safety",
                                                         juce::StringArray { "OFF", "LIMIT", "CLIP" }, 0));

    l.add (std::make_unique<FParam> (juce::ParameterID { "body", 1 },   "Body",      NRange (0.f, 100.f, 1.f), 0.f, FAttr().withLabel ("%").withStringFromValueFunction (d0)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "weight", 1 }, "Weight",    NRange (0.f, 100.f, 1.f), 0.f, FAttr().withLabel ("%").withStringFromValueFunction (d0)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "freq", 1 },   "Body Freq", NRange (40.f, 250.f, 1.f, 0.5f), 100.f, FAttr().withLabel ("Hz").withStringFromValueFunction (d0)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "drive", 1 },  "Drive",     NRange (0.f, 100.f, 1.f), 0.f, FAttr().withLabel ("%").withStringFromValueFunction (d0)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "punch", 1 },  "Punch",     NRange (0.f, 100.f, 1.f), 0.f, FAttr().withLabel ("%").withStringFromValueFunction (d0)));
    l.add (std::make_unique<FParam> (juce::ParameterID { "presence", 1 }, "Presence", NRange (0.f, 100.f, 1.f), 0.f, FAttr().withLabel ("%").withStringFromValueFunction (d0)));
    return l;
}

VCACompAudioProcessor::VCACompAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    pThr = apvts.getRawParameterValue ("thr");     pRatio = apvts.getRawParameterValue ("ratio");
    pAtk = apvts.getRawParameterValue ("attack");  pRel   = apvts.getRawParameterValue ("release");
    pKnee = apvts.getRawParameterValue ("knee");   pMakeup = apvts.getRawParameterValue ("makeup");
    pMix = apvts.getRawParameterValue ("mix");     pOut   = apvts.getRawParameterValue ("out");
    pHpf = apvts.getRawParameterValue ("hpf");     pCeil  = apvts.getRawParameterValue ("ceil");
    pAutoRel = apvts.getRawParameterValue ("autorel"); pAutoMk = apvts.getRawParameterValue ("automk");
    pRms = apvts.getRawParameterValue ("rms");     pSafety = apvts.getRawParameterValue ("safety");
    pBody = apvts.getRawParameterValue ("body");   pWeight = apvts.getRawParameterValue ("weight");
    pFreq = apvts.getRawParameterValue ("freq");   pDrive = apvts.getRawParameterValue ("drive");
    pPunch = apvts.getRawParameterValue ("punch"); pPresence = apvts.getRawParameterValue ("presence");
}

float VCACompAudioProcessor::currentMakeupDb() const
{
    float mk = pMakeup->load();
    if (pAutoMk->load() > 0.5f)
        mk += -pThr->load() * (1.f - 1.f / pRatio->load()) * 0.5f;   // compensacion aproximada
    return mk;
}

void VCACompAudioProcessor::clearState()
{
    for (int c = 0; c < 2; ++c)
    {
        hpState[c] = lp1[c] = lp2[c] = 0.f;
        dPrevU[c] = dPrevLn[c] = dcX[c] = dcY[c] = 0.f;
        pz1[c] = pz2[c] = 0.f;
    }
    msState = grState = grFast = grSlow = 0.f;
    chMsIn = chMsOut = drvMsIn = drvMsOut = 0.f;
    pkFast = pkSlow = punchDb = 0.f;
    tgBuf.assign ((size_t) limLen, 1.f);
    minBuf.assign ((size_t) limLen, 1.f);
    limPos = 0;
    tgActive = minActive = 0;
    minSum = (double) limLen;
    limGain = 1.f;
    for (auto& d : delayBuf) d.assign ((size_t) delaySize, 0.f);
    dWrite = 0;
    grMeter.store (0.f);
    outMeter.store (-60.f);
}

void VCACompAudioProcessor::reset() { clearState(); }

void VCACompAudioProcessor::prepareToPlay (double sampleRate, int)
{
    fs = sampleRate;
    limLen = juce::jlimit (8, 500, juce::roundToInt (fs * 0.0015));
    dDelay = limLen - 1;
    clearState();
    setLatencySamples (dDelay);

    smMix.reset (fs, 0.03);    smMix.setCurrentAndTargetValue (pMix->load() * 0.01f);
    smOut.reset (fs, 0.03);    smOut.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pOut->load()));
    smMakeup.reset (fs, 0.03); smMakeup.setCurrentAndTargetValue (currentMakeupDb());
    smBody.reset (fs, 0.03);   smBody.setCurrentAndTargetValue (pBody->load() * 0.01f);
    smWeight.reset (fs, 0.03); smWeight.setCurrentAndTargetValue (pWeight->load() * 0.01f);
    smDrive.reset (fs, 0.03);  smDrive.setCurrentAndTargetValue (pDrive->load() * 0.01f);
    smPunch.reset (fs, 0.03);  smPunch.setCurrentAndTargetValue (pPunch->load() * 0.01f);
}

bool VCACompAudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    auto in = l.getMainInputChannelSet();
    if (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo())
        return false;
    return in == l.getMainOutputChannelSet();
}

void VCACompAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    const int n = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, n);
    if (numCh == 0 || n == 0) return;

    // --- parametros ---
    const float thr = pThr->load(), ratio = pRatio->load(), knee = pKnee->load();
    const bool autoRel = pAutoRel->load() > 0.5f;
    const bool useRms = pRms->load() > 0.5f;
    const int safety = juce::roundToInt (pSafety->load());     // 0 OFF, 1 LIMIT, 2 CLIP
    const float ceilLin = juce::Decibels::decibelsToGain (pCeil->load());

    smMix.setTargetValue (pMix->load() * 0.01f);
    smOut.setTargetValue (juce::Decibels::decibelsToGain (pOut->load()));
    smMakeup.setTargetValue (currentMakeupDb());
    smBody.setTargetValue (pBody->load() * 0.01f);
    smWeight.setTargetValue (pWeight->load() * 0.01f);
    smDrive.setTargetValue (pDrive->load() * 0.01f);
    smPunch.setTargetValue (pPunch->load() * 0.01f);

    auto coef = [this] (float ms) { return std::exp (-1.f / (juce::jmax (0.01f, ms) * 0.001f * (float) fs)); };
    const float atkC = coef (pAtk->load()), relC = coef (pRel->load());
    const float relFastC = coef (50.f), relSlowC = coef (juce::jmax (400.f, pRel->load() * 3.f));
    const float limRelC = coef (100.f);
    const float rmsC = 1.f - coef (10.f);
    const float hpA = 1.f - std::exp (-juce::MathConstants<float>::twoPi * pHpf->load() / (float) fs);
    // dos filtros de 1 polo en cascada: x1.554 deja el corte (-3 dB) en la frecuencia elegida
    const float bandFreq = juce::jmin (pFreq->load() * 1.554f, 0.45f * (float) fs);
    const float bandA = 1.f - std::exp (-juce::MathConstants<float>::twoPi * bandFreq / (float) fs);
    const float chC = 1.f - coef (30.f);
    const float dcR = 1.f - juce::MathConstants<float>::twoPi * 8.f / (float) fs;      // bloqueo de DC a 8 Hz
    // punch: seguidores rapido/lento y suavizado de la ganancia
    const float fAtkC = coef (0.3f), fRelC = coef (25.f), sAtkC = coef (15.f), sRelC = coef (150.f);
    const float pAtkC = coef (0.5f), pRelC = coef (40.f);

    // presence: high-shelf RBJ a 3.5 kHz, hasta +6 dB
    const float presDb = pPresence->load() * 0.01f * 6.f;
    float pb0 = 1.f, pb1 = 0.f, pb2 = 0.f, pa1 = 0.f, pa2 = 0.f;
    if (presDb > 0.01f)
    {
        const float f0 = juce::jmin (3500.f, 0.45f * (float) fs);
        const float A = std::pow (10.f, presDb / 40.f);
        const float w0 = juce::MathConstants<float>::twoPi * f0 / (float) fs;
        const float cw = std::cos (w0), sw = std::sin (w0);
        const float alpha = sw * 0.70710678f;
        const float sA2 = 2.f * std::sqrt (A) * alpha;
        const float a0 = (A + 1.f) - (A - 1.f) * cw + sA2;
        pb0 =  A * ((A + 1.f) + (A - 1.f) * cw + sA2) / a0;
        pb1 = -2.f * A * ((A - 1.f) + (A + 1.f) * cw) / a0;
        pb2 =  A * ((A + 1.f) + (A - 1.f) * cw - sA2) / a0;
        pa1 =  2.f * ((A - 1.f) - (A + 1.f) * cw) / a0;
        pa2 = ((A + 1.f) - (A - 1.f) * cw - sA2) / a0;
    }

    float maxGr = 0.f;
    double sumSq = 0.0;
    float* data[2] { buffer.getWritePointer (0), numCh > 1 ? buffer.getWritePointer (1) : nullptr };

    for (int i = 0; i < n; ++i)
    {
        // entrada segura (evita NaN/Inf que envenenarian el estado)
        float in[2] { 0.f, 0.f };
        for (int c = 0; c < numCh; ++c)
        {
            const float x = data[c][i];
            in[c] = std::isfinite (x) ? x : 0.f;
        }

        // --- detector (sidechain con HPF, enlazado estereo) ---
        float sc = 0.f;
        for (int c = 0; c < numCh; ++c)
        {
            hpState[c] += hpA * (in[c] - hpState[c]);
            sc = juce::jmax (sc, std::abs (in[c] - hpState[c]));
        }
        float lvl = sc;
        if (useRms) { msState += rmsC * (sc * sc - msState); lvl = std::sqrt (juce::jmax (0.f, msState)); }
        const float lvlDb = juce::Decibels::gainToDecibels (lvl, -100.f);

        // --- gain computer con soft knee ---
        const float over = lvlDb - thr;
        float target = 0.f;                              // reduccion positiva en dB
        if (2.f * over > knee)
            target = over * (1.f - 1.f / ratio);
        else if (2.f * over > -knee)
        {
            const float t = over + knee * 0.5f;
            target = (1.f - 1.f / ratio) * t * t / (2.f * juce::jmax (knee, 0.001f));
        }

        // --- ataque / release en dominio dB (estilo VCA) ---
        float gr;
        if (autoRel)
        {
            const float cf = target > grFast ? atkC : relFastC;
            const float cs = target > grSlow ? atkC : relSlowC;
            grFast = cf * grFast + (1.f - cf) * target;
            grSlow = cs * grSlow + (1.f - cs) * target;
            gr = 0.5f * (grFast + grSlow);
        }
        else
        {
            const float cc = target > grState ? atkC : relC;
            grState = cc * grState + (1.f - cc) * target;
            gr = grState;
        }
        maxGr = juce::jmax (maxGr, gr);

        const float makeupDb = smMakeup.getNextValue();
        const float g = juce::Decibels::decibelsToGain (-gr + makeupDb);
        const float mix = smMix.getNextValue();
        const float outG = smOut.getNextValue();
        const float body = smBody.getNextValue();
        const float gW = 1.f + smWeight.getNextValue();
        const float drive = smDrive.getNextValue();
        const float punch = smPunch.getNextValue();

        // --- mezcla paralela ---
        float y[2] { 0.f, 0.f };
        for (int c = 0; c < numCh; ++c)
            y[c] = in[c] * (1.f - mix) + in[c] * g * mix;

        // --- caracter: graves = WEIGHT + BODY | medios/agudos = DRIVE (ambos con nivel compensado) ---
        {
            const float kB = 1.f + 3.f * body;
            const float dk = 1.f + 4.f * drive, bias = 0.2f * drive;
            const float biasOff = std::tanh (bias) / dk;
            float rest[2] { 0.f, 0.f }, bnd[2] { 0.f, 0.f }, satB[2] { 0.f, 0.f }, satD[2] { 0.f, 0.f };
            float eBi = 0.f, eBo = 0.f, eDi = 0.f, eDo = 0.f;
            for (int c = 0; c < numCh; ++c)
            {
                lp1[c] += bandA * (y[c] - lp1[c]);
                lp2[c] += bandA * (lp1[c] - lp2[c]);
                rest[c] = y[c] - lp2[c];                  // resto = señal - graves (suma perfecta)
                bnd[c]  = lp2[c] * gW;
                satB[c] = std::tanh (kB * bnd[c]) / kB;

                // saturacion con antiderivada (ADAA): casi sin aliasing, sin sobremuestreo ni latencia
                const float u = dk * rest[c] + bias;
                const float ln = lnCosh (u);
                const float du = u - dPrevU[c];
                float s = std::abs (du) > 1.0e-3f ? (ln - dPrevLn[c]) / (dk * du)
                                                  : std::tanh (0.5f * (u + dPrevU[c])) / dk;
                dPrevU[c] = u;  dPrevLn[c] = ln;
                s -= biasOff;
                dcY[c] = s - dcX[c] + dcR * dcY[c];       // bloqueador de DC (la asimetria genera armonicos pares)
                dcX[c] = s;
                satD[c] = dcY[c];

                eBi += bnd[c] * bnd[c];   eBo += satB[c] * satB[c];
                eDi += rest[c] * rest[c]; eDo += satD[c] * satD[c];
            }
            const float invCh = 1.f / (float) numCh;
            chMsIn  += chC * (eBi * invCh - chMsIn);   chMsOut  += chC * (eBo * invCh - chMsOut);
            drvMsIn += chC * (eDi * invCh - drvMsIn);  drvMsOut += chC * (eDo * invCh - drvMsOut);
            const float compB = juce::jlimit (1.f, 4.f, std::sqrt ((chMsIn  + 1.0e-12f) / (chMsOut  + 1.0e-12f)));
            const float compD = juce::jlimit (1.f, 4.f, std::sqrt ((drvMsIn + 1.0e-12f) / (drvMsOut + 1.0e-12f)));
            for (int c = 0; c < numCh; ++c)
                y[c] = (bnd[c]  + body  * (satB[c] * compB - bnd[c]))
                     + (rest[c] + drive * (satD[c] * compD - rest[c]));
        }

        // --- PUNCH: realza el inicio de cada golpe (transitorios) sin tocar el sostenido ---
        {
            const float pk = juce::jmax (std::abs (y[0]), numCh > 1 ? std::abs (y[1]) : 0.f);
            pkFast = pk > pkFast ? pk + fAtkC * (pkFast - pk) : pk + fRelC * (pkFast - pk);
            pkSlow = pk > pkSlow ? pk + sAtkC * (pkSlow - pk) : pk + sRelC * (pkSlow - pk);
            const float diffDb = 20.f * std::log10 ((pkFast + 1.0e-5f) / (pkSlow + 1.0e-5f));
            const float tgt = punch * 0.55f * juce::jlimit (0.f, 12.f, diffDb - 1.f);   // hasta ~+6.6 dB
            const float cp = tgt > punchDb ? pAtkC : pRelC;
            punchDb = cp * punchDb + (1.f - cp) * tgt;
            const float pg = juce::Decibels::decibelsToGain (punchDb);
            for (int c = 0; c < numCh; ++c) y[c] *= pg;
        }

        // --- PRESENCE (shelf de agudos) y volumen de salida ---
        float peak = 0.f;
        for (int c = 0; c < numCh; ++c)
        {
            const float x = y[c];
            const float o = pb0 * x + pz1[c];
            pz1[c] = pb1 * x - pa1 * o + pz2[c];
            pz2[c] = pb2 * x - pa2 * o;
            y[c] = o * outG;
            peak = juce::jmax (peak, std::abs (y[c]));
        }

        // --- SAFETY modo LIMIT: limitador con lookahead (min + promedio) ---
        const float tg = (safety == 1 && peak > ceilLin) ? ceilLin / peak : 1.f;
        {
            const size_t p = (size_t) limPos;
            if (tgBuf[p] < 1.f) --tgActive;
            tgBuf[p] = tg;
            if (tg < 1.f) ++tgActive;

            float minG = 1.f;
            if (tgActive > 0)                       // solo escanea cuando hay un pico en la ventana
                for (int k = 0; k < limLen; ++k) minG = juce::jmin (minG, tgBuf[(size_t) k]);

            if (minBuf[p] < 1.f) --minActive;
            minSum += (double) minG - (double) minBuf[p];
            minBuf[p] = minG;
            if (minG < 1.f) ++minActive;
        }
        float avg = 1.f;
        if (minActive > 0) avg = juce::jlimit (0.f, 1.f, (float) (minSum / (double) limLen));
        else               minSum = (double) limLen;  // elimina deriva numerica
        limPos = (limPos + 1) % limLen;
        limGain = avg < limGain ? avg : limRelC * limGain + (1.f - limRelC) * avg;

        // --- retardo fijo (siempre, para que la latencia no cambie) y salida ---
        for (int c = 0; c < numCh; ++c)
        {
            delayBuf[c][(size_t) dWrite] = y[c];
            float o = delayBuf[c][(size_t) ((dWrite - dDelay + delaySize) & delayMask)];
            if (safety == 1)      o = juce::jlimit (-ceilLin, ceilLin, o * limGain);
            else if (safety == 2) o = softClip (o, ceilLin);
            data[c][i] = o;
            sumSq += (double) o * (double) o;
        }
        dWrite = (dWrite + 1) & delayMask;
    }
    grMeter.store (maxGr);
    outMeter.store (juce::Decibels::gainToDecibels (std::sqrt ((float) (sumSq / (double) (n * numCh))), -60.f));
}

juce::AudioProcessorEditor* VCACompAudioProcessor::createEditor() { return new VCACompAudioProcessorEditor (*this); }

void VCACompAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
    {
        xml->setAttribute ("uiWidth", savedEditorWidth);
        copyXmlToBinary (*xml, dest);
    }
}

void VCACompAudioProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            savedEditorWidth = juce::jlimit (368, 1380, xml->getIntAttribute ("uiWidth", 640));
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new VCACompAudioProcessor(); }
