#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Paleta
namespace DioStyle
{
    inline const juce::Colour neon   { 0xff2f6bff };
    inline const juce::Colour neonHi { 0xff7fb0ff };
    inline const juce::Colour ice    { 0xffeaf1ff };
    inline const juce::Colour amber  { 0xffffb43a };
    inline const juce::Colour orange { 0xffff7a1a };
    inline const juce::Colour hot    { 0xffff4d2e };
    inline const juce::Colour gold   { 0xffffc233 };
    inline const juce::Colour cyan   { 0xff3de0ff };
    inline const juce::Colour purple { 0xffb066ff };
}

class DioLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DioLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float pos, float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
};

// Contenido del plugin (tamano base fijo; la ventana lo escala)
class DioPanel : public juce::Component, private juce::Timer
{
public:
    static constexpr int kW = 920, kH = 724;

    explicit DioPanel (VCACompAudioProcessor&);
    ~DioPanel() override;
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

    std::function<void (int)> onSizePercent;   // el editor cambia el tamano de la ventana
    std::function<int()> getSizePercent;

private:
    void timerCallback() override;
    void addKnob (int idx, const juce::String& name, const juce::String& id, const juce::String& suffix, juce::Colour accent);
    void addToggle (int idx, const juce::String& name, const juce::String& id);
    void rebuildBackground();
    void rebuildTitle();
    void showSettingsMenu();
    void drawVU (juce::Graphics&, juce::Rectangle<float>);

    struct Knob   { juce::Slider s; juce::Label l; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a; };
    struct Toggle { juce::ToggleButton b; std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> a; };

    DioLookAndFeel laf;
    VCACompAudioProcessor& proc;
    std::array<Knob, 16> knobs;
    std::array<Toggle, 3> toggles;
    juce::ComboBox safetyBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> safetyAtt;
    juce::ComboBox presetBox;
    juce::TextButton settingsBtn;

    // medidor
    int vuMode = 0;                 // 0 = reduccion de ganancia, 1 = nivel de salida
    float meterDisp = 0.f, outDisp = -60.f, needlePos = 1.f;
    float phase = 0.f;
    int tick = 0;
    juce::Rectangle<int> vuArea  { 684, 204, 216, 150 };
    juce::Rectangle<int> vuHit   { 684, 204, 216, 212 };
    juce::Rectangle<int> plateArea { 684, 426, 216, 250 };

    juce::Image bg;
    juce::Path titlePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DioPanel)
};

// Ventana del plugin: escalable (menu SETTINGS o esquina inferior derecha) y recuerda su tamano
class VCACompAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit VCACompAudioProcessorEditor (VCACompAudioProcessor&);
    void resized() override;

private:
    VCACompAudioProcessor& proc;
    DioPanel panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VCACompAudioProcessorEditor)
};
