# EP STEREO FIXER

A simple JUCE stereo-format tool inspired by the DaVinci Resolve Stereo Fixer panel.

## Interface

- **Format** row (eight exclusive buttons):
  - **Stereo** — standard stereo passthrough.
  - **Flip Channels** — swap left and right.
  - **Sum L+R** — mono sum on both outputs.
  - **Left** — left channel on both outputs.
  - **Right** — right channel on both outputs.
  - **Mid/Side** — encode to Mid/Side; gain left controls Mid, gain right controls Side.
  - **Solo Mid** — output only the Mid component.
  - **Solo Side** — output only the Side component.
- **Utility** row — Invert L, Invert R, Auto Gain, Bypass.
- **Input Gain** — knob to trim the level before format processing.
- **Width** — knob to adjust stereo width using Mid/Side scaling (0 % = mono, 100 % = normal, 200 % = double width).
- **Output Gain** — two rotary knobs (-18 dB to +18 dB).
- **Link** — chain icon between the two output gain knobs; when enabled both knobs move together.
- **Meters** — two peak meters (L / R) with peak dB readouts, a phase/correlation meter and a stereo goniometer.

Hovering any control shows an English description.

## Requirements

- CMake 3.16 or later
- A C++17 compiler
- The JUCE framework (https://github.com/juce-framework/JUCE)

## Build

Clone JUCE next to this project or anywhere on disk, then set `JUCE_DIR` to its root:

```bash
export JUCE_DIR="$HOME/JUCE"
cd stereo_field_manipulator_vst

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

On Apple Silicon add `-DCMAKE_OSX_ARCHITECTURES=arm64` to the configure step.

## Outputs

After building you will find the binaries under `build/`:

- `EP STEREO FIXER.vst3`
- `EP STEREO FIXER.app` (Standalone)

## Installation

```bash
mkdir -p "$HOME/Library/Audio/Plug-Ins/VST3"
cp -R "build/EPStereoFixer_artefacts/Release/VST3/EP STEREO FIXER.vst3" \
    "$HOME/Library/Audio/Plug-Ins/VST3/"
```

Restart your DAW after copying the plugin.

## Notes

- Developer: **EP**.
- VST3 category: **Fx|Spatial**.
- The GUI is resizable between the default size and double the default size.
- Gain parameters are smoothed to avoid zipper noise.
- Auto Gain is a peak-based automatic level control; it reacts smoothly to avoid pumping.
- The phase meter shows correlation: **left = mono**, **right = out-of-phase**.
- The goniometer draws the stereo image with X = left and Y = right.
