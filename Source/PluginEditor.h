#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

// Paleta: negro profundo, azul electrico neon, azul hielo (indicadores), cian (medidores), metal oscuro
namespace DioStyle
{
    inline const juce::Colour electric { 0xff2f7bff };
    inline const juce::Colour ice      { 0xffdff4ff };
    inline const juce::Colour cyan     { 0xff3de0ff };
    inline const juce::Colour steel    { 0xff8fb2e0 };
    inline const juce::Colour deep     { 0xff03060d };
}

class DioLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DioLookAndFeel();
    juce::Font getLabelFont (juce::Label&) override;
    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getTextButtonFont (juce::TextButton&, int buttonHeight) override;
    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float pos, float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool highlighted, bool down) override;
    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour&, bool highlighted, bool down) override;
};

class PromptOverlay;

// Contenido del plugin (tamano base fijo; la ventana lo escala. 60% = 840 x 576 px)
class DioPanel : public juce::Component, private juce::Timer
{
public:
    static constexpr int kW = 1400, kH = 960;

    explicit DioPanel (DioCompressProcessor&);
    ~DioPanel() override;
    void paint (juce::Graphics&) override;
    void resized() override;

    std::function<void (int)> onSizePercent;   // el editor cambia el tamano de la ventana
    std::function<int()> getSizePercent;

private:
    struct Slot
    {
        juce::Slider s;
        juce::Label l;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a;
    };

    void timerCallback() override;
    void initSlot (Slot&);
    void configureSlot (Slot&, const char* id, const juce::String& name, const juce::String& suffix, juce::Colour accent);
    void hideSlot (Slot&);
    void applyTopologyUi();
    void layoutControls();
    void rebuildTitle();
    juce::Colour accent() const;
    float meterMax() const;
    juce::Image& getBackground (int topo);
    void drawMeter (juce::Graphics&, juce::Rectangle<float>);

    void setChoice (const char* id, int index);
    void setBool (const char* id, bool v);
    int  getChoice (const char* id) const;

    // presets
    void refreshPresetList();
    void selectCurrentPreset();
    void showSaveDialog();
    void showManageMenu();
    void showSettingsMenu();
    void showPrompt (const juce::String& title, const juce::String& message, const juce::String& defaultText,
                     bool withText, const juce::String& okText, std::function<void (juce::String)> onOk);

    DioLookAndFeel laf;
    DioCompressProcessor& proc;
    std::array<Slot, 8> mainSlots;       // 0 y 1 = perillas grandes; 2..7 = fila inferior (segun la topologia)
    std::array<Slot, 5> sharedSlots;     // 0 drive, 1 color, 2 mix analogico, 3 bass preserve amount, 4 ceiling
    std::array<juce::TextButton, 3> topoBtn, modeBtn;
    juce::TextButton prevBtn, nextBtn, saveBtn, manageBtn, settingsBtn;
    juce::ToggleButton powerBtn, autoBtn, analogBtn, bpBtn;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> autoAtt, analogAtt, bpAtt;
    juce::ComboBox presetBox, profileBox, safetyBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> safetyAtt;

    int uiTopo = 0, autoOwner = 6, modeCount = 3;
    bool analogUiOn = false, bpUiOn = false;
    float grDisp = 0.f, inDisp = -80.f, outDisp = -80.f, needlePos = 1.f, phase = 0.f;
    int tick = 0;
    juce::Rectangle<int> meterArea { 400, 198, 600, 262 };

    std::vector<int> presetIds;
    juce::StringArray userNames;
    juce::String lastPresetName;
    bool lastPresetUser = false;

    std::unique_ptr<PromptOverlay> overlay;
    std::function<void (juce::String)> pendingAction;

    std::array<juce::Image, 3> bgs;
    juce::Path titlePath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DioPanel)
};

// Ventana del plugin: SIEMPRE se abre al 60%. El menu SETTINGS permite cambiarla solo durante la sesion.
class DioCompressEditor : public juce::AudioProcessorEditor
{
public:
    explicit DioCompressEditor (DioCompressProcessor&);
    void resized() override;

private:
    DioPanel panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DioCompressEditor)
};
