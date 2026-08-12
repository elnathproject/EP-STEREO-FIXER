cmake_minimum_required(VERSION 3.16)
project(EPStereoFixer VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

if(NOT DEFINED ENV{JUCE_DIR})
    message(FATAL_ERROR "Set JUCE_DIR to the root of the JUCE repository, then re-run CMake.")
endif()

add_subdirectory($ENV{JUCE_DIR} JUCE)

juce_add_plugin(EPStereoFixer
    COMPANY_NAME "EP"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    PLUGIN_MANUFACTURER_CODE EPEP
    PLUGIN_CODE Sfmn
    FORMATS VST3 Standalone
    PRODUCT_NAME "EP STEREO FIXER"
    VST3_CATEGORIES "Fx|Spatial"
)

target_sources(EPStereoFixer PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
)

target_include_directories(EPStereoFixer PRIVATE src)

target_compile_definitions(EPStereoFixer PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(EPStereoFixer PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
)
