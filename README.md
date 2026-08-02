# SakuraFont

SakuraFont is an original anime/visual-novel-inspired SoundFont instrument built with JUCE and FluidSynth.

## Included

- Windows 64-bit VST3 and standalone targets
- GitHub Actions cloud build
- `.sf2` file picker and drag-and-drop loading
- Large playable on-screen keyboard
- Volume and ADSR controls
- Reverb and chorus
- Bank and 128-program browser
- Animated falling petals
- Dark purple/blue original interface
- Panic button and DAW state recall

No copyrighted character art or commercial SoundFont is included.

## Build it on GitHub

1. Create an empty GitHub repository.
2. Upload **the contents of this folder**, including the hidden `.github` folder.
3. Open the repository's **Actions** tab.
4. Choose **Build Windows VST3**.
5. Click **Run workflow**.
6. When the run completes, open it and download the `SakuraFont-Windows-VST3` artifact.
7. Extract the artifact and copy the complete `SakuraFont.vst3` folder to:

   `C:\Program Files\Common Files\VST3`

8. In Ableton Live, enable the VST3 system folder and rescan plug-ins.

## Important

GitHub may ask you to enable Actions the first time. The cloud build installs FluidSynth and includes its runtime DLL dependencies in the VST3 bundle.
