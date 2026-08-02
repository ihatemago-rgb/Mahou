# SakuraFont

SakuraFont is a 64-bit SoundFont instrument plug-in with an original anime-inspired GUI.

## Included features

- VST3 instrument and standalone application targets
- `.sf2` SoundFont loading
- MIDI notes, velocity, pitch bend, controllers, and aftertouch
- Bank and program selection
- Gain, reverb, and chorus controls
- Panic/all-notes-off button
- DAW state recall for parameters and the selected SoundFont path
- Original moonlit visual-novel-inspired vector interface

## Easiest build method

Use `BUILD_WITH_GITHUB.md`. GitHub Actions builds the Windows VST3 on a cloud Windows machine, so the broken Windows PowerShell installation on your computer is not involved.

## Local developer build

Requirements:

- Visual Studio 2022 with Desktop development with C++
- CMake 3.22+
- Git
- vcpkg with `fluidsynth:x64-windows` and `pkgconf:x64-windows`

Configure and build from a Visual Studio Developer Command Prompt:

```cmd
set PKG_CONFIG_PATH=C:/vcpkg/installed/x64-windows/lib/pkgconfig
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release --parallel
```

The resulting bundle is normally under:

```text
build\SakuraFont_artefacts\Release\VST3\SakuraFont.vst3
```

## Licensing

JUCE and FluidSynth are third-party dependencies with their own licenses. Review those licenses before distributing a commercial binary. No SoundFont or copyrighted anime artwork is included.
