#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Paleta: violeta neon, magenta, negro profundo y metal
namespace DioStyle
{
    inline const juce::Colour violet   { 0xffa855ff };
    inline const juce::Colour violetHi { 0xffdcb0ff };
    inline const juce::Colour magenta  { 0xffff3fd0 };
    inline const juce::Colour ice      { 0xfff3eaff };
    inline const juce::Colour deep     { 0xff07040f };
}

class DioLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DioLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float pos, float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;
};

// Contenido del plugin (tamano base fijo; la ventana lo escala)
class DioPanel : public juce::Component, private juce::Timer
{
public:
    static constexpr int kW = 1100, kH = 744;

    explicit DioPanel (DioCompressProcessor&);
    ~DioPanel() override;
    void paint (juce::Graphics&) override;
    void resized() override;

    std::function<void (int)> onSizePercent;   // el editor cambia el tamano de la ventana
    std::function<int()> getSizePercent;

private:
    void timerCallback() override;
    void addKnob (int idx, const juce::String& name, const juce::String& id, const juce::String& suffix, juce::Colour accent);
    void rebuildBackground();
    void rebuildTitle();
    void showSettingsMenu();
    void applyTopologyUi();
    void setChoice (const char* id, int index);
    void setBool (const char* id, bool v);
    int  getChoice (const char* id) const;
    void drawMeter (juce::Graphics&, juce::Rectangle<float>);

    struct Knob { juce::Slider s; juce::Label l; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a; };

    DioLookAndFeel laf;
    DioCompressProcessor& proc;
    std::array<Knob, 14> knobs;          // 0 thr, 1 ratio, 2 attack, 3 release, 4 knee, 5 hpf, 6 ceil,
                                         // 7 input, 8 makeup, 9 out, 10 mix, 11 drive, 12 color, 13 amix
    std::array<juce::TextButton, 3> topoBtn, modeBtn;
    juce::TextButton prevBtn, nextBtn, settingsBtn;
    juce::ToggleButton powerBtn, autoMkBtn, analogBtn;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoMkAtt, analogAtt;
    juce::ComboBox presetBox, profileBox, safetyBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> safetyAtt;

    int uiTopo = 0;
    bool analogUiOn = false;
    float grDisp = 0.f, inDisp = -80.f, outDisp = -80.f, needlePos = 1.f;
    float phase = 0.f;
    int tick = 0;
    juce::Rectangle<int> meterArea { 300, 156, 500, 228 };

    juce::Image bg;
    juce::Path titlePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DioPanel)
};

// Ventana del plugin: escalable (menu SETTINGS o esquina inferior derecha) y recuerda su tamano
class DioCompressEditor : public juce::AudioProcessorEditor
{
public:
    explicit DioCompressEditor (DioCompressProcessor&);
    void resized() override;

private:
    DioCompressProcessor& proc;
    DioPanel panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DioCompressEditor)
};
