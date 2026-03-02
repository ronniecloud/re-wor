# re-wor — Guitar Hero: Warriors of Rock (Xbox 360 → PC)

⚠️ Early Release (v1.0) — This is the first public release. Bugs, crashes, and performance issues are expected and will be addressed in future updates. Bug reports and contributions are welcome!

Native recompilation of **Guitar Hero: Warriors of Rock** (Xbox 360) for PC, using the [RexGlue SDK](https://github.com/rexglue/rexglue-sdk). Based on [re-gh2](https://github.com/YoshiCrystal9/re-gh2) by YoshiCrystal9.

## Legal Disclaimer

This is an educational/reverse engineering project. No game files included. You MUST own a legitimate copy of Guitar Hero: Warriors of Rock (Xbox 360). Game assets are copyrighted by Activision/Neversoft and are NOT included.

## Current Status

- ✅ Boot and menu working
- ✅ Gameplay playable with guitar (XINPUT)
- ✅ Multi-channel audio (XMA → SDL, 6 channels)
- ✅ Fullscreen (F11)
- ⚠️ Audio may have slight delay vs chart
- ⚠️ Native 720p resolution (upscale in development)
- ⚠️ Game logic runs at 60fps (original engine limitation)

## Known Issues

- Slight audio delay vs chart
- Fixed 720p resolution
- 60fps game logic cap
- Possible crashes during song transitions

## Prerequisites

- **Clang 20+** (LLVM)
- **CMake 3.25+**
- **Ninja**
- **RexGlue SDK** compiled and installed
- **GH:WoR Xbox 360 ISO** (legitimate copy)

## Setup

### 1. Clone the repository

```bash
git clone --recursive https://github.com/ronniecloud/re-wor.git
cd re-wor
```

### 2. Extract game files

Extract the contents of your Xbox 360 ISO into the `assets/` folder:

```bash
# Use extract-xiso or equivalent tool
extract-xiso -d assets/ GH_WarriorsOfRock.iso
```

Expected structure:
```
assets/
├── default.xex          # Xbox 360 executable
└── data/                # Game data
    ├── streams/         # Audio (.fsb.xen)
    └── scripts/         # QScript (.qb.xen)
```

> ⚠️ **Do NOT commit files in `assets/`** — they are already in `.gitignore`.

### 3. Configure RexGlue SDK

Ensure the RexGlue SDK is compiled and installed.

### 4. Build

```bash
# Configure
cmake --preset win-amd64-relwithdebinfo

# Generate recompiled code
cmake --build out/build/win-amd64-relwithdebinfo --target re_wor_codegen

# Build
cmake --build out/build/win-amd64-relwithdebinfo
```

### 5. Run

```bash
./out/build/win-amd64-relwithdebinfo/"Guitar Hero Warriors of Rock.exe"
```

## Controls

| Key | Action |
|-------|------|
| F11 | Fullscreen |
| USB Guitar | Auto-detected via XINPUT |

## Project Structure

```
re-wor/
├── CMakeLists.txt          # Build system
├── CMakePresets.json        # Build presets
├── re_wor_config.toml       # PPC function map (codegen input)
├── app_icon.ico             # Executable icon
├── app_icon.rc              # Resource file (Windows)
├── src/
│   ├── main.cpp             # Entry point + app class
│   └── stubs.cpp            # Stubs for unimplemented functions
├── generated/               # Codegen output (auto-generated, gitignored)
└── assets/                  # ISO content (gitignored)
```

## Engine Info

GH:WoR uses the **Neversoft engine**:
- **Filesystem**: PAK/PAB (`.pak.xen` / `.pab.xen`)
- **Audio**: XMA2 → FMOD Sound Bank (`.fsb.xen`)
- **Scripting**: QScript (Neversoft proprietary)
- **Title ID**: `41560883`

## Bug Reports

If you find a bug, please open an [issue](https://github.com/ronniecloud/re-wor/issues).

## Project Type

Educational and reverse engineering project.
