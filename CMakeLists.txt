cmake_minimum_required(VERSION 3.16)
project(EP_STEREO_MATRIX VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

if(DEFINED ENV{JUCE_DIR})
    add_subdirectory($ENV{JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(
        JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        9.0.1
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(EP_STEREO_MATRIX
    COMPANY_NAME "EP"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    PLUGIN_MANUFACTURER_CODE EPEP
    PLUGIN_CODE Sfmn
    FORMATS VST3 Standalone
    PRODUCT_NAME "EP STEREO MATRIX"
    VST3_CATEGORIES "Fx|Spatial"
)

juce_add_binary_data(EPImages SOURCES
    resources/logo.png
)

target_sources(EP_STEREO_MATRIX PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
)

target_include_directories(EP_STEREO_MATRIX PRIVATE src)

target_compile_definitions(EP_STEREO_MATRIX PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(EP_STEREO_MATRIX PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    EPImages
)
