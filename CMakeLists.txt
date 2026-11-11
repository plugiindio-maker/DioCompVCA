cmake_minimum_required(VERSION 3.22)
project(DioCompVCA VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.4
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)

if(APPLE)
    set(DIO_FORMATS VST3 AU Standalone)
else()
    set(DIO_FORMATS VST3 Standalone)
endif()

juce_add_plugin(DioCompVCA
    COMPANY_NAME "DioOnTheBeat"
    PLUGIN_MANUFACTURER_CODE Dioo
    PLUGIN_CODE Dcva
    FORMATS ${DIO_FORMATS}
    PRODUCT_NAME "DioCompVCA"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE)

juce_generate_juce_header(DioCompVCA)

target_sources(DioCompVCA PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(DioCompVCA PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0)

if(MSVC)
    target_compile_options(DioCompVCA PRIVATE /utf-8)
endif()

target_link_libraries(DioCompVCA
    PRIVATE juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags)
