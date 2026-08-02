# Build SakuraFont without using your broken PowerShell

This method builds the plug-in on a temporary GitHub Windows computer. Your own PC only needs a browser.

## 1. Create a free GitHub account
Go to GitHub and sign in.

## 2. Create a repository
1. Click **New repository**.
2. Name it `SakuraFont`.
3. Choose **Private** if you do not want the source public.
4. Create the repository.

## 3. Upload this project
1. Open the new repository.
2. Choose **Add file → Upload files**.
3. Drag the *contents* of this project folder into the upload page. Include the hidden `.github` folder.
4. Commit the files.

If Windows Explorer does not show `.github`, enable **View → Show → Hidden items** first.

## 4. Start the cloud build
1. Open the repository's **Actions** tab.
2. Select **Build Windows VST3**.
3. Click **Run workflow**.
4. Wait for the build to finish with a green checkmark.

The first build can take several minutes because JUCE and FluidSynth are downloaded and compiled.

## 5. Download the finished plug-in
1. Open the completed workflow run.
2. Scroll to **Artifacts**.
3. Download **SakuraFont-Windows-VST3**.
4. Extract the downloaded ZIP.
5. Copy the complete `SakuraFont.vst3` folder to:

```text
C:\Program Files\Common Files\VST3
```

## 6. Scan it in Ableton Live
1. Open **Options → Preferences → Plug-Ins**.
2. Enable **Use VST3 Plug-In System Folders**.
3. Hold **Alt** and click **Rescan**.
4. Search for **SakuraFont** under Instruments.

## Important
- Copy the complete outer `SakuraFont.vst3` folder. Do not copy only the small binary inside it.
- No SoundFont is included. Load your own legally obtained `.sf2` file from the plug-in interface.
- The included interface is original anime-inspired vector artwork and does not copy a character from an existing anime.
