cmake_minimum_required(VERSION 3.22)
project(RhymeZoneVST VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(JUCE)

juce_add_plugin(RhymeZoneVST
    COMPANY_NAME "RhymeZone"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Rhzn
    PLUGIN_CODE Rhzv
    FORMATS VST3 Standalone
    PRODUCT_NAME "RhymeZone"
)

juce_generate_juce_header(RhymeZoneVST)

target_sources(RhymeZoneVST PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/RhymeZoneAPI.cpp
)

# JUCE uses libcurl on Linux, WinHTTP on Windows, NSURLSession on macOS
if(UNIX AND NOT APPLE)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(CURL REQUIRED libcurl)
    set(CURL_LINK_LIBS ${CURL_LIBRARIES})
    set(USE_CURL 1)
else()
    set(CURL_LINK_LIBS "")
    set(USE_CURL 0)
endif()

target_compile_definitions(RhymeZoneVST PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=${USE_CURL}
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(RhymeZoneVST PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_core
    ${CURL_LINK_LIBS}
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
