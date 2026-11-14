#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

using FParam = juce::AudioParameterFloat;
using NRange = juce::NormalisableRange<float>;
using FAttr  = juce::AudioParameterFloatAttributes;

namespace
{
    constexpr int kNumProfiles = 7;
    const char* const kProfileNames[kNumProfiles] = {
        "VOCAL", "KICK", "SNARE", "DRUM BUS", "808 / BASS", "INSTRUMENTAL", "MASTER BUS" };
    const char* const kTopoNames[3] = { "VCA", "OPTICO", "VARI-MU" };
    const char* const kModeNames[3] = { "SMOOTH", "PUNCH", "AGGRESSIVE" };

    const char* const kVcaIds[8]   = { "vthr", "vratio", "vattack", "vrelease", "vknee", "vhpf", "vmode", "vmix" };
    const char* const kOptIds[4]   = { "opeak", "olimit", "oemph", "omix" };
    const char* const kMuIds[6]    = { "minput", "mthr", "mrecov", "mbias", "mdens", "mmix" };
    const char* const kExtraIds[6] = { "analogon", "drive", "color", "amix", "bpon", "bpamt" };

    // Sugerencias por fuente (puntos de partida editables). Cada topologia tiene SUS controles.
    //                                    thr    ratio atk    rel    knee  hpf    mode mix
    const float kVcaProf[kNumProfiles][8] = {
        { -22.f, 3.5f, 12.f, 140.f,  9.f, 100.f, 0.f, 100.f },   // VOCAL      (smooth)
        { -14.f, 4.f,  25.f,  80.f,  3.f,  30.f, 1.f, 100.f },   // KICK       (punch)
        { -15.f, 5.f,  20.f,  90.f,  3.f, 120.f, 1.f, 100.f },   // SNARE      (punch)
        { -16.f, 4.f,  30.f, 100.f,  4.f,  80.f, 2.f, 100.f },   // DRUM BUS   (aggressive)
        { -20.f, 3.f,  25.f, 250.f,  8.f,  25.f, 0.f, 100.f },   // 808 / BASS (smooth)
        { -18.f, 3.f,  15.f, 150.f,  8.f,  60.f, 0.f, 100.f },   // INSTRUMENTAL
        { -12.f, 2.f,  30.f, 150.f, 10.f,  80.f, 0.f, 100.f } }; // MASTER BUS
    //                                    peak  limit emph mix
    const float kOptProf[kNumProfiles][4] = {
        { 48.f, 0.f, 15.f, 100.f }, { 36.f, 0.f,  0.f, 100.f }, { 36.f, 1.f, 10.f, 100.f }, { 36.f, 0.f,  0.f, 100.f },
        { 44.f, 0.f,  0.f, 100.f }, { 40.f, 0.f, 10.f, 100.f }, { 28.f, 0.f,  0.f, 100.f } };
    //                                    input  thr    recov bias dens  mix
    const float kMuProf[kNumProfiles][6] = {
        { 0.f, -22.f, 30.f, 50.f, 36.f, 100.f }, { 0.f, -18.f, 22.f, 40.f, 33.f, 100.f },
        { 0.f, -17.f, 26.f, 40.f, 36.f, 100.f }, { 0.f, -16.f, 32.f, 45.f, 33.f, 100.f },
        { 0.f, -22.f, 40.f, 60.f, 28.f, 100.f }, { 0.f, -18.f, 30.f, 50.f, 28.f, 100.f },
        { 0.f, -12.f, 38.f, 55.f, 26.f, 100.f } };
    // Analog Color + Bass Preserve sugeridos: [fuente][VCA, OPTICO, VARI-MU] = on, drive, color, mix, bpOn, bpAmt
    const float kExtra[kNumProfiles][3][6] = {
        { { 1.f, 15.f, 40.f, 40.f, 0.f, 50.f }, { 1.f, 12.f, 55.f, 35.f, 0.f, 50.f }, { 1.f, 18.f, 60.f, 40.f, 0.f, 50.f } },   // VOCAL
        { { 1.f, 20.f, 30.f, 35.f, 0.f, 50.f }, { 0.f, 20.f, 30.f, 35.f, 0.f, 50.f }, { 1.f, 20.f, 40.f, 35.f, 0.f, 50.f } },   // KICK
        { { 1.f, 25.f, 35.f, 40.f, 0.f, 50.f }, { 0.f, 20.f, 35.f, 35.f, 0.f, 50.f }, { 1.f, 22.f, 40.f, 40.f, 0.f, 50.f } },   // SNARE
        { { 1.f, 30.f, 45.f, 45.f, 1.f, 40.f }, { 0.f, 20.f, 40.f, 35.f, 1.f, 40.f }, { 1.f, 25.f, 45.f, 40.f, 1.f, 40.f } },   // DRUM BUS
        { { 1.f, 25.f, 50.f, 40.f, 1.f, 80.f }, { 1.f, 20.f, 50.f, 35.f, 1.f, 80.f }, { 1.f, 22.f, 55.f, 35.f, 1.f, 80.f } },   // 808 / BASS
        { { 1.f, 20.f, 45.f, 35.f, 0.f, 50.f }, { 0.f, 20.f, 45.f, 35.f, 0.f, 50.f }, { 1.f, 20.f, 50.f, 35.f, 0.f, 50.f } },   // INSTRUMENTAL
        { { 0.f, 15.f, 40.f, 30.f, 1.f, 40.f }, { 0.f, 15.f, 40.f, 30.f, 1.f, 40.f }, { 1.f, 10.f, 40.f, 30.f, 1.f, 40.f } } }; // MASTER BUS
    constexpr int kNumPresets = 1 + kNumProfiles * 3;

    // Vari-Mu: recuperacion (60 ms .. 6 s) y densidad (ratio maximo 1:1 .. 15:1)
    inline float muRelMs (float rec) { return 60.f * std::pow (100.f, juce::jlimit (0.f, 1.f, rec)); }
    inline float muRmax  (float d)   { return 1.f + 14.f * std::pow (juce::jlimit (0.f, 1.f, d), 1.4f); }

    inline float lnCosh (float u)
    {
        const float a = std::abs (u);
        return a + std::log1p (std::exp (-2.f * a)) - 0.69314718f;   // ln(cosh(u)) estable
    }

    // Saturacion con antiderivada (ADAA): reduce mucho el aliasing sin sobremuestreo ni latencia.
    // bias > 0 = asimetria (armonicos pares); bias = 0 = simetrica (armonicos impares)
    inline float adaaSat (float x, float k, float bias, float& pU, float& pLn, float& dcX, float& dcY, float dcR)
    {
        const float u = k * x + bias;
        const float ln = lnCosh (u);
        const float du = u - pU;
        float s = std::abs (du) > 1.0e-3f ? (ln - pLn) / (k * du) : std::tanh (0.5f * (u + pU)) / k;
        pU = u;  pLn = ln;
        s -= std::tanh (bias) / k;
        dcY = s - dcX + dcR * dcY;                 // bloqueador de DC
        dcX = s;
        return dcY;
    }

    inline float softClip (float x, float ceilLin)
    {
        const float t = 0.7f * ceilLin, ax = std::abs (x);
        if (ax <= t) return x;                                       // bajo el 70% no toca nada (transitorios intactos)
        const float r = ceilLin - t;
        const float y = t + r * std::tanh ((ax - t) / r);            // nunca pasa del ceiling
        return x < 0.f ? -y : y;
    }

    inline float gainCurve (float over, float r, float k)            // reduccion (dB, positiva) con soft knee
    {
        if (2.f * over > k) return over * (1.f - 1.f / r);
        if (2.f * over > -k)
        {
            const float t = over + k * 0.5f;
            return (1.f - 1.f / r) * t * t / (2.f * juce::jmax (k, 0.001f));
        }
        return 0.f;
    }

    inline void follow (float& s, float t, float ca, float cr)       // seguidor con ataque / release
    {
        const float c = t > s ? ca : cr;
        s = c * s + (1.f - c) * t;
    }
}

// ------------------------------------------------------------------ parametros
juce::AudioProcessorValueTreeState::ParameterLayout DioCompressProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    auto d0 = [] (float v, int) { return juce::String (juce::roundToInt (v)); };
    auto d1 = [] (float v, int) { return juce::String (v, 1); };
    auto recText = [] (float v, int)
    {
        const float ms = muRelMs (v * 0.01f);
        return ms < 1000.f ? juce::String (juce::roundToInt (ms)) + " ms" : juce::String (ms / 1000.f, 1) + " s";
    };
    auto recParse = [] (const juce::String& s)
    {
        const float val = s.getFloatValue();
        const bool sec = s.containsIgnoreCase ("s") && ! s.containsIgnoreCase ("ms");
        const float ms = sec ? val * 1000.f : val;
        return juce::jlimit (0.f, 100.f, 100.f * std::log (juce::jmax (ms, 1.f) / 60.f) / std::log (100.f));
    };
    auto densText = [] (float v, int) { return juce::String (muRmax (v * 0.01f), 1) + ":1"; };
    auto densParse = [] (const juce::String& s)
    {
        const float r = s.getFloatValue();
        return juce::jlimit (0.f, 100.f, 100.f * std::pow (juce::jmax (0.f, (r - 1.f) / 14.f), 1.f / 1.4f));
    };
    auto fl = [&] (const char* id, const char* name, NRange range, float def, const char* label,
                   std::function<juce::String (float, int)> text)
    {
        l.add (std::make_unique<FParam> (juce::ParameterID { id, 1 }, name, range, def,
                                         FAttr().withLabel (label).withStringFromValueFunction (text)));
    };
    auto bl = [&] (const char* id, const char* name, bool def)
    {
        l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { id, 1 }, name, def));
    };
    auto ch = [&] (const char* id, const char* name, juce::StringArray items, int def)
    {
        l.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID { id, 1 }, name, items, def));
    };

    // --- globales ---
    ch ("topology", "Topology", { "VCA", "OPTICO", "VARI-MU" }, 0);
    bl ("bypass", "Bypass", false);
    ch ("safety", "Safety", { "OFF", "LIMIT", "CLIP" }, 0);
    fl ("ceil", "Ceiling", NRange (-12.f, 0.f, 0.1f), -0.3f, "dB", d1);

    // --- VCA ---
    ch ("vmode", "VCA Mode", { "SMOOTH", "PUNCH", "AGGRESSIVE" }, 0);
    fl ("vthr", "VCA Threshold", NRange (-60.f, 0.f, 0.1f), -18.f, "dB", d1);
    fl ("vratio", "VCA Ratio", NRange (1.f, 20.f, 0.1f, 0.5f), 4.f, "", d1);
    fl ("vattack", "VCA Attack", NRange (0.1f, 100.f, 0.01f, 0.4f), 10.f, "ms", d1);
    fl ("vrelease", "VCA Release", NRange (20.f, 5000.f, 1.f, 0.35f), 150.f, "ms", d0);
    fl ("vknee", "VCA Knee", NRange (0.f, 18.f, 0.1f), 6.f, "dB", d1);
    fl ("vmakeup", "VCA Makeup", NRange (-12.f, 24.f, 0.1f), 0.f, "dB", d1);
    bl ("vauto", "VCA Auto Makeup", true);
    fl ("vhpf", "VCA SC HPF", NRange (20.f, 500.f, 1.f, 0.4f), 20.f, "Hz", d0);
    fl ("vmix", "VCA Mix", NRange (0.f, 100.f, 1.f), 100.f, "%", d0);

    // --- OPTICO ---
    fl ("opeak", "Opto Peak Reduction", NRange (0.f, 100.f, 0.1f), 40.f, "", d0);
    fl ("ogain", "Opto Gain", NRange (-12.f, 24.f, 0.1f), 0.f, "dB", d1);
    ch ("olimit", "Opto Compress/Limit", { "COMPRESS", "LIMIT" }, 0);
    fl ("oemph", "Opto Emphasis", NRange (0.f, 100.f, 1.f), 0.f, "%", d0);
    bl ("oauto", "Opto Auto Gain", true);
    fl ("omix", "Opto Mix", NRange (0.f, 100.f, 1.f), 100.f, "%", d0);

    // --- VARI-MU ---
    fl ("minput", "Mu Input", NRange (-12.f, 24.f, 0.1f), 0.f, "dB", d1);
    fl ("mthr", "Mu Threshold", NRange (-48.f, 0.f, 0.1f), -20.f, "dB", d1);
    l.add (std::make_unique<FParam> (juce::ParameterID { "mrecov", 1 }, "Mu Recovery", NRange (0.f, 100.f, 0.1f), 40.f,
                                     FAttr().withStringFromValueFunction (recText).withValueFromStringFunction (recParse)));
    fl ("mbias", "Mu Bias", NRange (0.f, 100.f, 1.f), 50.f, "%", d0);
    l.add (std::make_unique<FParam> (juce::ParameterID { "mdens", 1 }, "Mu Density", NRange (0.f, 100.f, 0.1f), 50.f,
                                     FAttr().withStringFromValueFunction (densText).withValueFromStringFunction (densParse)));
    fl ("moutput", "Mu Output", NRange (-18.f, 18.f, 0.1f), 0.f, "dB", d1);
    bl ("mauto", "Mu Auto Gain", true);
    fl ("mmix", "Mu Mix", NRange (0.f, 100.f, 1.f), 100.f, "%", d0);

    // --- ANALOG COLOR y BASS PRESERVE (globales e independientes) ---
    bl ("analogon", "Analog On", false);
    fl ("drive", "Analog Drive", NRange (0.f, 100.f, 1.f), 25.f, "%", d0);
    fl ("color", "Analog Color", NRange (0.f, 100.f, 1.f), 50.f, "%", d0);
    fl ("amix", "Analog Mix", NRange (0.f, 100.f, 1.f), 50.f, "%", d0);
    bl ("bpon", "Bass Preserve On", false);
    fl ("bpamt", "Bass Preserve Amount", NRange (0.f, 100.f, 1.f), 60.f, "%", d0);
    return l;
}

DioCompressProcessor::DioCompressProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMS", createParameterLayout())
{
    auto g = [this] (const char* id) { return apvts.getRawParameterValue (id); };
    pr.topo = g ("topology");  pr.bypass = g ("bypass");  pr.safety = g ("safety");  pr.ceil = g ("ceil");
    pr.vmode = g ("vmode");    pr.vthr = g ("vthr");      pr.vratio = g ("vratio");  pr.vatk = g ("vattack");
    pr.vrel = g ("vrelease");  pr.vknee = g ("vknee");    pr.vmk = g ("vmakeup");    pr.vauto = g ("vauto");
    pr.vhpf = g ("vhpf");      pr.vmix = g ("vmix");
    pr.opeak = g ("opeak");    pr.ogain = g ("ogain");    pr.olimit = g ("olimit");  pr.oemph = g ("oemph");
    pr.oauto = g ("oauto");    pr.omix = g ("omix");
    pr.minput = g ("minput");  pr.mthr = g ("mthr");      pr.mrec = g ("mrecov");    pr.mbias = g ("mbias");
    pr.mdens = g ("mdens");    pr.mout = g ("moutput");   pr.mauto = g ("mauto");    pr.mmix = g ("mmix");
    pr.analog = g ("analogon"); pr.drive = g ("drive");   pr.color = g ("color");    pr.amix = g ("amix");
    pr.bpon = g ("bpon");      pr.bpamt = g ("bpamt");
}

// valores actuales segun la topologia activa
float DioCompressProcessor::currentInputDb() const
{
    return juce::roundToInt (pr.topo->load()) == 2 ? pr.minput->load() : 0.f;
}

float DioCompressProcessor::currentMix() const
{
    const int topo = juce::roundToInt (pr.topo->load());
    return 0.01f * (topo == 0 ? pr.vmix->load() : topo == 1 ? pr.omix->load() : pr.mmix->load());
}

float DioCompressProcessor::currentPostGainDb() const
{
    const int topo = juce::jlimit (0, 2, juce::roundToInt (pr.topo->load()));
    if (topo == 0)
    {
        float mk = pr.vmk->load();
        if (pr.vauto->load() > 0.5f)
        {
            const float r = pr.vratio->load();
            const float re = (juce::roundToInt (pr.vmode->load()) == 2) ? juce::jmin (30.f, r * 1.35f) : r;
            mk += -pr.vthr->load() * (1.f - 1.f / juce::jmax (1.f, re)) * 0.5f;
        }
        return mk;
    }
    if (topo == 1)
    {
        float mk = pr.ogain->load();
        if (pr.oauto->load() > 0.5f)
        {
            const float thrO = -0.5f * pr.opeak->load();
            const float re = pr.olimit->load() > 0.5f ? 10.f : 3.f;
            mk += -thrO * (1.f - 1.f / re) * 0.5f;
        }
        return mk;
    }
    float mk = pr.mout->load();
    if (pr.mauto->load() > 0.5f)
    {
        const float aM = 1.f - 1.f / muRmax (pr.mdens->load() * 0.01f);
        const float tau = 4.f + 20.f * pr.mbias->load() * 0.01f;
        mk += -pr.minput->load() + 0.5f * aM * 100.f / (10.f + tau);
    }
    return mk;
}

// ------------------------------------------------------------------ presets y perfiles
int DioCompressProcessor::getNumPrograms() { return kNumPresets; }

const juce::String DioCompressProcessor::getProgramName (int index)
{
    if (index == 0) return "Init";
    if (index < 0 || index >= kNumPresets) return {};
    const int p = (index - 1) / 3, t = (index - 1) % 3;
    const juce::String topo = (t == 0) ? juce::String ("VCA ") + kModeNames[juce::jlimit (0, 2, juce::roundToInt (kVcaProf[p][6]))]
                                       : juce::String (kTopoNames[t]);
    return juce::String (kProfileNames[p]) + " - " + topo;
}

void DioCompressProcessor::setParam (const char* id, float v)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (v));
}

void DioCompressProcessor::applyInit()
{
    setParam ("topology", 0.f); setParam ("safety", 0.f);   setParam ("ceil", -0.3f);
    setParam ("vmode", 0.f);    setParam ("vthr", -18.f);   setParam ("vratio", 4.f);   setParam ("vattack", 10.f);
    setParam ("vrelease", 150.f); setParam ("vknee", 6.f);  setParam ("vmakeup", 0.f);  setParam ("vauto", 1.f);
    setParam ("vhpf", 20.f);    setParam ("vmix", 100.f);
    setParam ("opeak", 40.f);   setParam ("ogain", 0.f);    setParam ("olimit", 0.f);   setParam ("oemph", 0.f);
    setParam ("oauto", 1.f);    setParam ("omix", 100.f);
    setParam ("minput", 0.f);   setParam ("mthr", -20.f);   setParam ("mrecov", 40.f);  setParam ("mbias", 50.f);
    setParam ("mdens", 50.f);   setParam ("moutput", 0.f);  setParam ("mauto", 1.f);    setParam ("mmix", 100.f);
    setParam ("analogon", 0.f); setParam ("drive", 25.f);   setParam ("color", 50.f);   setParam ("amix", 50.f);
    setParam ("bpon", 0.f);     setParam ("bpamt", 60.f);
}

void DioCompressProcessor::applyProfileFor (int profile, int topology)
{
    profile = juce::jlimit (0, kNumProfiles - 1, profile);
    topology = juce::jlimit (0, 2, topology);
    if (topology == 0)
    {
        for (int i = 0; i < 8; ++i) setParam (kVcaIds[i], kVcaProf[profile][i]);
        setParam ("vmakeup", 0.f);  setParam ("vauto", 1.f);
    }
    else if (topology == 1)
    {
        for (int i = 0; i < 4; ++i) setParam (kOptIds[i], kOptProf[profile][i]);
        setParam ("ogain", 0.f);  setParam ("oauto", 1.f);
    }
    else
    {
        for (int i = 0; i < 6; ++i) setParam (kMuIds[i], kMuProf[profile][i]);
        setParam ("moutput", 0.f);  setParam ("mauto", 1.f);
    }
    for (int i = 0; i < 6; ++i) setParam (kExtraIds[i], kExtra[profile][topology][i]);
    currentProfile = profile;
}

void DioCompressProcessor::applyProfile (int profile)
{
    applyProfileFor (profile, juce::roundToInt (pr.topo->load()));
}

void DioCompressProcessor::finishPresetLoad (const juce::String& name, bool user, int factoryIndex)
{
    currentPresetName = name;
    currentPresetUser = user;
    if (! user) currentFactory = factoryIndex;
    currentProfile = -1;
}

void DioCompressProcessor::loadFactoryPreset (int index)
{
    index = juce::jlimit (0, kNumPresets - 1, index);
    if (index == 0)
    {
        applyInit();
        finishPresetLoad ("Init", false, 0);
        return;
    }
    const int p = (index - 1) / 3, t = (index - 1) % 3;
    setParam ("topology", (float) t);
    applyProfileFor (p, t);
    finishPresetLoad (getProgramName (index), false, index);
    currentProfile = p;
}

// ------------------------------------------------------------------ presets de usuario (disco)
juce::File DioCompressProcessor::getUserPresetDir()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("DioOnTheBeat").getChildFile ("DioCompress").getChildFile ("Presets");
}

static juce::String legalPresetName (const juce::String& raw)
{
    return juce::File::createLegalFileName (raw.trim()).substring (0, 40).trim();
}

juce::StringArray DioCompressProcessor::getUserPresetNames() const
{
    juce::StringArray names;
    const auto dir = getUserPresetDir();
    if (dir.isDirectory())
    {
        juce::Array<juce::File> files;
        dir.findChildFiles (files, juce::File::findFiles, false, "*.dioPreset");
        for (auto& f : files) names.add (f.getFileNameWithoutExtension());
        names.sortNatural();
    }
    return names;
}

bool DioCompressProcessor::userPresetExists (const juce::String& name) const
{
    const auto legal = legalPresetName (name);
    return legal.isNotEmpty() && getUserPresetDir().getChildFile (legal + ".dioPreset").existsAsFile();
}

bool DioCompressProcessor::saveUserPreset (const juce::String& name, bool overwrite)
{
    const auto legal = legalPresetName (name);
    if (legal.isEmpty()) return false;
    auto dir = getUserPresetDir();
    if (! dir.isDirectory() && ! dir.createDirectory().wasOk()) return false;
    auto file = dir.getChildFile (legal + ".dioPreset");
    if (file.existsAsFile() && ! overwrite) return false;

    auto xml = apvts.copyState().createXml();       // todos los parametros: las tres topologias, Analog Color, Bass Preserve...
    if (xml == nullptr) return false;
    xml->setAttribute ("presetName", legal);
    xml->setAttribute ("dioPresetVersion", 1);
    if (! xml->writeTo (file)) return false;
    finishPresetLoad (legal, true, 0);
    return true;
}

bool DioCompressProcessor::loadUserPreset (const juce::String& name)
{
    const auto legal = legalPresetName (name);
    auto file = getUserPresetDir().getChildFile (legal + ".dioPreset");
    if (! file.existsAsFile()) return false;
    auto xml = juce::XmlDocument::parse (file);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType())) return false;

    const float bypassNow = pr.bypass->load();      // un preset no cambia el bypass de la instancia
    apvts.replaceState (juce::ValueTree::fromXml (*xml));
    setParam ("bypass", bypassNow);
    finishPresetLoad (legal, true, 0);
    return true;
}

bool DioCompressProcessor::deleteUserPreset (const juce::String& name)
{
    const auto legal = legalPresetName (name);
    auto file = getUserPresetDir().getChildFile (legal + ".dioPreset");
    if (! file.existsAsFile() || ! file.deleteFile()) return false;
    if (currentPresetUser && currentPresetName == legal)
    {
        currentPresetName = "Custom";
        currentPresetUser = false;
    }
    return true;
}

bool DioCompressProcessor::renameUserPreset (const juce::String& oldName, const juce::String& newName)
{
    const auto o = legalPresetName (oldName), n = legalPresetName (newName);
    if (o.isEmpty() || n.isEmpty()) return false;
    if (o == n) return true;
    auto dir = getUserPresetDir();
    auto from = dir.getChildFile (o + ".dioPreset"), to = dir.getChildFile (n + ".dioPreset");
    if (! from.existsAsFile() || to.existsAsFile()) return false;
    if (! from.moveFileTo (to)) return false;
    if (currentPresetUser && currentPresetName == o) currentPresetName = n;
    return true;
}

// ------------------------------------------------------------------ audio
void DioCompressProcessor::clearState()
{
    for (int c = 0; c < 2; ++c)
    {
        hpV[c] = hpD[c] = emL[c] = lpB1[c] = lpB2[c] = 0.f;
        aLp1[c] = aLp2[c] = 0.f;
        for (int b = 0; b < 2; ++b) aPU[c][b] = aPLn[c][b] = aDcX[c][b] = aDcY[c][b] = 0.f;
    }
    msV = msO = msM = grBp = 0.f;
    vF = vS = oF = oS = oMem = mF = mS = 0.f;
    aMsIn = aMsOut = 0.f;
    tgBuf.assign ((size_t) limLen, 1.f);
    minBuf.assign ((size_t) limLen, 1.f);
    limPos = 0;
    tgActive = minActive = 0;
    minSum = (double) limLen;
    limGain = 1.f;
    for (auto& d : delayBuf) d.assign ((size_t) delaySize, 0.f);
    for (auto& d : inDelay)  d.assign ((size_t) delaySize, 0.f);
    dWrite = 0;
    grMeter.store (0.f);
    inMeter.store (-80.f);
    outMeter.store (-80.f);
}

void DioCompressProcessor::reset() { clearState(); }

void DioCompressProcessor::prepareToPlay (double sampleRate, int)
{
    fs = sampleRate;
    limLen = juce::jlimit (8, 500, juce::roundToInt (fs * 0.0015));
    dDelay = limLen - 1;
    clearState();
    setLatencySamples (dDelay);

    auto init = [this] (juce::SmoothedValue<float>& s, double sec, float v)
    {
        s.reset (fs, sec);
        s.setCurrentAndTargetValue (v);
    };
    const int topo = juce::jlimit (0, 2, juce::roundToInt (pr.topo->load()));
    init (smIn, 0.03, juce::Decibels::decibelsToGain (currentInputDb()));
    init (smMix, 0.03, currentMix());
    init (smMakeup, 0.03, currentPostGainDb());
    init (smDrive, 0.03, pr.drive->load() * 0.01f);
    init (smColor, 0.03, pr.color->load() * 0.01f);
    init (smAmix, 0.03, pr.amix->load() * 0.01f);
    init (smAnalog, 0.03, pr.analog->load() > 0.5f ? 1.f : 0.f);
    init (smBypass, 0.02, pr.bypass->load() > 0.5f ? 1.f : 0.f);
    init (smWV, 0.04, topo == 0 ? 1.f : 0.f);
    init (smWO, 0.04, topo == 1 ? 1.f : 0.f);
    init (smWM, 0.04, topo == 2 ? 1.f : 0.f);
    init (smBpOn, 0.03, pr.bpon->load() > 0.5f ? 1.f : 0.f);
    init (smBpAmt, 0.03, pr.bpamt->load() * 0.01f);
}

bool DioCompressProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    auto in = l.getMainInputChannelSet();
    if (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo())
        return false;
    return in == l.getMainOutputChannelSet();
}

void DioCompressProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numCh = juce::jmin (buffer.getNumChannels(), 2);
    const int n = buffer.getNumSamples();
    for (int c = getTotalNumInputChannels(); c < getTotalNumOutputChannels(); ++c)
        buffer.clear (c, 0, n);
    if (numCh == 0 || n == 0) return;

    // --- parametros ---
    const int topo  = juce::jlimit (0, 2, juce::roundToInt (pr.topo->load()));
    const int vmode = juce::jlimit (0, 2, juce::roundToInt (pr.vmode->load()));
    const bool bpOnT = pr.bpon->load() > 0.5f;
    const float bpAmtT = pr.bpamt->load() * 0.01f;
    const int safety = juce::roundToInt (pr.safety->load());     // 0 OFF, 1 LIMIT, 2 CLIP
    const float ceilLin = juce::Decibels::decibelsToGain (pr.ceil->load());

    smIn.setTargetValue (juce::Decibels::decibelsToGain (currentInputDb()));
    smMix.setTargetValue (currentMix());
    smMakeup.setTargetValue (currentPostGainDb());
    smDrive.setTargetValue (pr.drive->load() * 0.01f);
    smColor.setTargetValue (pr.color->load() * 0.01f);
    smAmix.setTargetValue (pr.amix->load() * 0.01f);
    smAnalog.setTargetValue (pr.analog->load() > 0.5f ? 1.f : 0.f);
    smBypass.setTargetValue (pr.bypass->load() > 0.5f ? 1.f : 0.f);
    smWV.setTargetValue (topo == 0 ? 1.f : 0.f);
    smWO.setTargetValue (topo == 1 ? 1.f : 0.f);
    smWM.setTargetValue (topo == 2 ? 1.f : 0.f);
    smBpOn.setTargetValue (bpOnT ? 1.f : 0.f);
    smBpAmt.setTargetValue (bpAmtT);

    auto coefMs = [this] (float ms) { return std::exp (-1.f / (juce::jmax (0.01f, ms) * 0.001f * (float) fs)); };
    const float twoPi = juce::MathConstants<float>::twoPi;
    const float hpVA = 1.f - std::exp (-twoPi * pr.vhpf->load() / (float) fs);     // HPF del detector VCA
    const float hpDA = 1.f - std::exp (-twoPi * 20.f / (float) fs);                // quita DC en los detectores optico / vari-mu
    const float emA  = 1.f - std::exp (-twoPi * 1500.f / (float) fs);              // EMPHASIS (solo detector)
    const float bpA  = 1.f - std::exp (-twoPi * 110.f * 1.554f / (float) fs);      // banda grave de BASS PRESERVE (~110 Hz)
    const float c20 = 1.f - coefMs (20.f), c40 = 1.f - coefMs (40.f);

    // VCA
    const float thrV = pr.vthr->load(), ratioVu = pr.vratio->load(), kneeVu = pr.vknee->load(), atkV = pr.vatk->load();
    const float relV = bpOnT ? juce::jmax (pr.vrel->load(), 120.f) : pr.vrel->load();   // BASS PRESERVE: recuperacion musical (min 120 ms)
    const float vAtk0 = coefMs (juce::jmax (atkV, 6.f));                         // SMOOTH: ataque minimo 6 ms (cuerpo)
    const float vRel0F = coefMs (relV * 0.4f), vRel0S = coefMs (relV * 1.6f);
    const float vRel1 = coefMs (relV * 0.7f);                                    // PUNCH
    const float vAtk2 = coefMs (juce::jmax (0.1f, atkV * 0.4f));                 // AGGRESSIVE
    const float vRel2F = coefMs (relV * 0.3f), vRel2S = coefMs (relV * 1.2f);
    // OPTICO: celda con memoria, sin attack/release convencionales
    const bool limitO = pr.olimit->load() > 0.5f;
    const float thrO = -0.5f * pr.opeak->load();                                 // PEAK REDUCTION: 0 -> sin compresion, 100 -> -50 dB
    const float ratioO = limitO ? 10.f : 3.f, kneeO = limitO ? 6.f : 12.f;
    const float oAtkBase = limitO ? 6.f : 10.f;
    const float oRelF = coefMs (60.f), oRelS = coefMs (limitO ? 800.f : 1200.f);
    const float memAtk = 1.f - coefMs (1500.f), memRel = 1.f - coefMs (6000.f);
    const float emphAmt = pr.oemph->load() * 0.01f;
    // VARI-MU: ratio progresivo
    const float thrM = pr.mthr->load();
    const float aM = 1.f - 1.f / muRmax (pr.mdens->load() * 0.01f);              // DENSITY: ratio maximo
    const float tauM = 4.f + 20.f * pr.mbias->load() * 0.01f;                    // BIAS: donde empieza a subir el ratio
    const float relBaseM = muRelMs (pr.mrec->load() * 0.01f);                    // RECOVERY
    // BASS PRESERVE
    const float bpC = coefMs (2.f + 8.f * bpAmtT);

    // ANALOG COLOR
    const float subA = 1.f - std::exp (-twoPi * 70.f * 1.554f / (float) fs);
    const float subKeep = 0.3f;                                                  // el subgrave recibe solo 30% de la saturacion
    const float dcR = 1.f - twoPi * 8.f / (float) fs;
    const float chC = 1.f - coefMs (30.f);

    const float limRelC = coefMs (100.f);

    float maxGr = 0.f, inPk = 0.f, outPk = 0.f;
    float* data[2] { buffer.getWritePointer (0), numCh > 1 ? buffer.getWritePointer (1) : nullptr };

    for (int i = 0; i < n; ++i)
    {
        // entrada segura (evita NaN/Inf) y ganancia de entrada (solo Vari-Mu la usa)
        float raw[2] { 0.f, 0.f }, xin[2] { 0.f, 0.f };
        const float inG = smIn.getNextValue();
        for (int c = 0; c < numCh; ++c)
        {
            const float v = data[c][i];
            raw[c] = std::isfinite (v) ? v : 0.f;
            xin[c] = raw[c] * inG;
            inPk = juce::jmax (inPk, std::abs (xin[c]));
        }

        // --- BASS PRESERVE: el DETECTOR ve menos graves; la senal de audio no se filtra ---
        const float onE = smBpOn.getNextValue(), amtE = smBpAmt.getNextValue();
        const float bpEff = onE * amtE;
        const float bpScale = 1.f - 0.75f * bpEff;
        float lowX[2] { 0.f, 0.f };
        float scV = 0.f, scO = 0.f, scM = 0.f;
        for (int c = 0; c < numCh; ++c)
        {
            lpB1[c] += bpA * (xin[c] - lpB1[c]);
            lpB2[c] += bpA * (lpB1[c] - lpB2[c]);
            lowX[c] = lpB2[c];
            const float d = xin[c] - lowX[c] * (1.f - bpScale);          // detector: graves reducidos hasta 75%
            hpV[c] += hpVA * (d - hpV[c]);                               // VCA: SC HPF propio
            scV = juce::jmax (scV, std::abs (d - hpV[c]));
            hpD[c] += hpDA * (d - hpD[c]);
            const float o = d - hpD[c];
            emL[c] += emA * (o - emL[c]);
            const float oe = o + emphAmt * 3.f * (o - emL[c]);           // OPTICO: EMPHASIS realza agudos en el detector (hasta +12 dB)
            scO = juce::jmax (scO, std::abs (oe));
            scM = juce::jmax (scM, std::abs (o));
        }
        msV += c20 * (scV * scV - msV);
        msO += c20 * (scO * scO - msO);
        msM += c40 * (scM * scM - msM);
        const float pkVDb   = juce::Decibels::gainToDecibels (scV, -100.f);
        const float rmsVDb  = juce::Decibels::gainToDecibels (std::sqrt (juce::jmax (0.f, msV)), -100.f);
        const float rmsODb  = juce::Decibels::gainToDecibels (std::sqrt (juce::jmax (0.f, msO)), -100.f);
        const float rmsMDb  = juce::Decibels::gainToDecibels (std::sqrt (juce::jmax (0.f, msM)), -100.f);

        // === VCA: detector, tiempos y curva segun la personalidad ===
        float lvlV, ratioV, kneeV;
        if (vmode == 0)      { lvlV = rmsVDb; ratioV = ratioVu; kneeV = kneeVu + 4.f; }                       // SMOOTH: RMS
        else if (vmode == 1) { lvlV = pkVDb;  ratioV = ratioVu; kneeV = kneeVu; }                             // PUNCH: picos
        else                 { lvlV = juce::jmax (pkVDb, rmsVDb + 3.f); ratioV = juce::jmin (30.f, ratioVu * 1.35f); kneeV = kneeVu * 0.5f; }
        if (vmode >= 1 && bpEff > 0.0001f)                                                                   // BASS PRESERVE: picos y RMS combinados
            lvlV += 0.8f * bpEff * ((rmsVDb + 3.f) - lvlV);
        const float tgtV = gainCurve (lvlV - thrV, ratioV, kneeV);
        float grV;
        if (vmode == 0)
        {
            follow (vF, tgtV, vAtk0, vRel0F);  follow (vS, tgtV, vAtk0, vRel0S);
            grV = 0.5f * (vF + vS);
        }
        else if (vmode == 1)
        {
            // ataque adaptativo: deja pasar el transitorio si la reduccion es pequena, lo atrapa si es grande
            const float t = juce::jlimit (0.f, 1.f, (tgtV - 3.f) / 5.f);
            follow (vF, tgtV, coefMs (atkV * (1.6f - 1.1f * t)), vRel1);
            vS = vF;
            grV = vF;
        }
        else
        {
            follow (vF, tgtV, vAtk2, vRel2F);  follow (vS, tgtV, vAtk2, vRel2S);
            grV = 0.6f * vF + 0.4f * vS;
        }

        // === OPTICO: celda con tiempos que dependen del programa y memoria de la compresion ===
        const float tgtO = gainCurve (rmsODb - thrO, ratioO, kneeO);
        const float wSprev = 0.25f + 0.65f * oMem;
        const float prevO = (1.f - wSprev) * oF + wSprev * oS;
        const float errO = juce::jmax (0.f, tgtO - oF);
        const float caO = coefMs (juce::jmax (1.5f, oAtkBase / (1.f + 0.1f * errO)));   // mas rapido cuanto mas empuja la senal
        follow (oF, tgtO, caO, oRelF);
        follow (oS, tgtO, caO, oRelS);
        const float mT = juce::jlimit (0.f, 1.f, prevO / 12.f);
        oMem += (mT - oMem) * (mT > oMem ? memAtk : memRel);
        const float wS = 0.25f + 0.65f * oMem;                                            // compresion sostenida = cola de recuperacion larga
        const float grO = (1.f - wS) * oF + wS * oS;

        // === VARI-MU: ratio progresivo segun el nivel ===
        const float oM = juce::jmax (0.f, rmsMDb - thrM);
        const float tgtM = aM * oM * oM / (oM + tauM);                                    // el ratio sube a medida que sube la reduccion
        const float grMprev = 0.55f * mF + 0.45f * mS;
        const float caM = coefMs (juce::jmax (0.5f, 8.f / (1.f + 0.05f * tgtM)));
        const float rsc = 1.f + grMprev / 15.f;                                           // recuperacion mas lenta cuanto mas profunda
        follow (mF, tgtM, caM, coefMs (relBaseM * 0.35f * rsc));
        follow (mS, tgtM, caM, coefMs (relBaseM * 2.0f * rsc));
        const float grM = 0.55f * mF + 0.45f * mS;

        // mezcla suave entre topologias (sin clicks)
        const float wV = smWV.getNextValue(), wO = smWO.getNextValue(), wM = smWM.getNextValue();
        const float gr = wV * grV + wO * grO + wM * grM;
        grBp += (1.f - bpC) * (gr - grBp);                                                // BASS PRESERVE: reduccion sin modulacion a ritmo de audio
        const float grOut = gr + onE * (grBp - gr);
        maxGr = juce::jmax (maxGr, grOut);

        const float gGR = juce::Decibels::decibelsToGain (-grOut);
        const float mkLin = juce::Decibels::decibelsToGain (smMakeup.getNextValue());
        const float mix = smMix.getNextValue();
        float y[2] { 0.f, 0.f };
        for (int c = 0; c < numCh; ++c)
        {
            // Dry/Wet paralelo alineado. BASS PRESERVE devuelve parte del grave original (misma ganancia de makeup)
            y[c] = raw[c] * (1.f - mix) + xin[c] * gGR * mkLin * mix
                 + 0.6f * bpEff * lowX[c] * mix * mkLin * (1.f - gGR);
        }

        // === ANALOG COLOR (modulo independiente, despues de la compresion) ===
        {
            const float aAmt = smAnalog.getNextValue() * smAmix.getNextValue();
            const float drive = smDrive.getNextValue(), color = smColor.getNextValue();
            const float k = 1.f + 7.f * drive;                   // DRIVE: cuanto se empuja la saturacion
            const float bias = 0.55f * color;                    // COLOR: asimetria = armonicos pares (calido) vs impares (agresivo)
            const float dAmt = juce::jlimit (0.f, 1.f, drive * 3.f);   // con DRIVE en 0 la senal queda limpia
            float wet[2] { 0.f, 0.f };
            float eIn = 0.f, eOut = 0.f;
            for (int c = 0; c < numCh; ++c)
            {
                aLp1[c] += subA * (y[c] - aLp1[c]);
                aLp2[c] += subA * (aLp1[c] - aLp2[c]);
                const float lo = aLp2[c], hi = y[c] - lo;
                const float sh = adaaSat (hi, k, bias, aPU[c][0], aPLn[c][0], aDcX[c][0], aDcY[c][0], dcR);
                const float sl = adaaSat (lo, k, bias, aPU[c][1], aPLn[c][1], aDcX[c][1], aDcY[c][1], dcR);
                wet[c] = (hi + dAmt * (sh - hi)) + (lo + dAmt * subKeep * (sl - lo));
                eIn += y[c] * y[c];
                eOut += wet[c] * wet[c];
            }
            const float invCh = 1.f / (float) numCh;
            aMsIn  += chC * (eIn  * invCh - aMsIn);
            aMsOut += chC * (eOut * invCh - aMsOut);
            const float comp = juce::jlimit (0.5f, 2.5f, std::sqrt ((aMsIn + 1.0e-12f) / (aMsOut + 1.0e-12f)));   // nivel controlado
            for (int c = 0; c < numCh; ++c)
                y[c] += aAmt * (wet[c] * comp - y[c]);
        }

        float peak = 0.f;
        for (int c = 0; c < numCh; ++c) peak = juce::jmax (peak, std::abs (y[c]));

        // --- SAFETY modo LIMIT: limitador con lookahead (min + promedio) ---
        const float tg = (safety == 1 && peak > ceilLin) ? ceilLin / peak : 1.f;
        {
            const size_t p = (size_t) limPos;
            if (tgBuf[p] < 1.f) --tgActive;
            tgBuf[p] = tg;
            if (tg < 1.f) ++tgActive;

            float minG = 1.f;
            if (tgActive > 0)
                for (int k2 = 0; k2 < limLen; ++k2) minG = juce::jmin (minG, tgBuf[(size_t) k2]);

            if (minBuf[p] < 1.f) --minActive;
            minSum += (double) minG - (double) minBuf[p];
            minBuf[p] = minG;
            if (minG < 1.f) ++minActive;
        }
        float avg = 1.f;
        if (minActive > 0) avg = juce::jlimit (0.f, 1.f, (float) (minSum / (double) limLen));
        else               minSum = (double) limLen;
        limPos = (limPos + 1) % limLen;
        limGain = avg < limGain ? avg : limRelC * limGain + (1.f - limRelC) * avg;

        // --- retardo fijo (latencia constante), bypass con crossfade alineado y salida ---
        const float bp = smBypass.getNextValue();
        for (int c = 0; c < numCh; ++c)
        {
            delayBuf[c][(size_t) dWrite] = y[c];
            inDelay[c][(size_t) dWrite] = raw[c];
            const size_t rd = (size_t) ((dWrite - dDelay + delaySize) & delayMask);
            float o = delayBuf[c][rd];
            if (safety == 1)      o = juce::jlimit (-ceilLin, ceilLin, o * limGain);
            else if (safety == 2) o = softClip (o, ceilLin);
            o = o * (1.f - bp) + inDelay[c][rd] * bp;
            data[c][i] = o;
            outPk = juce::jmax (outPk, std::abs (o));
        }
        dWrite = (dWrite + 1) & delayMask;
    }
    grMeter.store (maxGr);
    inMeter.store (juce::Decibels::gainToDecibels (inPk, -80.f));
    outMeter.store (juce::Decibels::gainToDecibels (outPk, -80.f));
}

juce::AudioProcessorEditor* DioCompressProcessor::createEditor() { return new DioCompressEditor (*this); }

// Estado de cada instancia (se guarda dentro del proyecto del DAW)
void DioCompressProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
    {
        xml->setAttribute ("currentPreset", currentPresetName);
        xml->setAttribute ("currentPresetUser", currentPresetUser);
        xml->setAttribute ("currentFactory", currentFactory);
        xml->setAttribute ("profile", currentProfile);
        copyXmlToBinary (*xml, dest);
    }
}

void DioCompressProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
        {
            currentPresetName = xml->getStringAttribute ("currentPreset", "Init");
            currentPresetUser = xml->getBoolAttribute ("currentPresetUser", false);
            currentFactory = juce::jlimit (0, kNumPresets - 1, xml->getIntAttribute ("currentFactory", 0));
            currentProfile = xml->getIntAttribute ("profile", -1);
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
        }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DioCompressProcessor(); }
