#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Estilo visual: azul neon, humo, plata
namespace DioStyle
{
    inline const juce::Colour neon   { 0xff2f6bff };
    inline const juce::Colour neonHi { 0xff7fb0ff };
    inline const juce::Colour ice    { 0xffeaf1ff };
}

class DioLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DioLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float pos, float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
};

class DioPanel : public juce::Component, private juce::Timer
{
public:
    explicit DioPanel (VCACompAudioProcessor&);
    ~DioPanel() override;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void addKnob (int idx, const juce::String& name, const juce::String& id, const juce::String& suffix);
    void addToggle (int idx, const juce::String& name, const juce::String& id);
    void rebuildBackground();
    void rebuildTitle();

    struct Knob   { juce::Slider s; juce::Label l; std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a; };
    struct Toggle { juce::ToggleButton b; std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> a; };

    DioLookAndFeel laf;
    VCACompAudioProcessor& proc;
    std::array<Knob, 13> knobs;
    std::array<Toggle, 4> toggles;
    juce::ComboBox presetBox;
    float meterDisp = 0.f;
    juce::Rectangle<int> meterArea;
    juce::Image bg;
    juce::Path titlePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DioPanel)
};

// Ventana del plugin: escalable (arrastra la esquina inferior derecha) y recuerda su tamano
class VCACompAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit VCACompAudioProcessorEditor (VCACompAudioProcessor&);
    void resized() override;

private:
    static constexpr int baseW = 760, baseH = 696;
    VCACompAudioProcessor& proc;
    DioPanel panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VCACompAudioProcessorEditor)
};
