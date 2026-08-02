cmake_minimum_required(VERSION 3.22)
project(SakuraFont VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.12
)
FetchContent_MakeAvailable(JUCE)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FLUIDSYNTH REQUIRED IMPORTED_TARGET fluidsynth)

juce_add_plugin(SakuraFont
  COMPANY_NAME "Moonlit Audio"
  BUNDLE_ID "audio.moonlit.sakurafont"
  PLUGIN_MANUFACTURER_CODE Mnlt
  PLUGIN_CODE SkFt
  FORMATS VST3 Standalone
  PRODUCT_NAME "SakuraFont"
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_MIDI_OUTPUT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD TRUE
  VST3_CATEGORIES Instrument Synth
)

juce_generate_juce_header(SakuraFont)

target_sources(SakuraFont PRIVATE
  Source/PluginProcessor.cpp
  Source/PluginProcessor.h
  Source/PluginEditor.cpp
  Source/PluginEditor.h
)

target_compile_definitions(SakuraFont PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(SakuraFont PRIVATE
  juce::juce_audio_utils
  juce::juce_dsp
  PkgConfig::FLUIDSYNTH
  PUBLIC
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags
)
