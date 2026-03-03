
# <img width="1536" height="1024" alt="77d74078-a914-467a-9369-20964b77e6f8" src="https://github.com/user-attachments/assets/89298cf2-a0dc-4a45-9bc7-71fbf4a614e6" />


An unofficial PC port of **Guitar Hero: Warriors of Rock** (Xbox 360) created through the process of static recompilation.

**This project does not include any game assets. You must provide the files from your own legally acquired copy of the game.**

[Latest Release](https://github.com/ronniecloud/re-wor/releases/latest)

Built on top of the [RexGlue SDK](https://github.com/rexglue/rexglue-sdk) recompilation toolchain. Based on [re-gh2](https://github.com/YoshiCrystal9/re-gh2) by YoshiCrystal9.

</div>

## Table of Contents

- [System Requirements](#system-requirements)
- [How to Install](#how-to-install)
- [Features](#features)
- [Known Issues](#known-issues)
- [FAQ](#faq)
- [Building from Source](#building-from-source)
- [Project Structure](#project-structure)
- [Credits](#credits)

## System Requirements

- **OS**: Windows 10/11 (64-bit)
- **CPU**: Any modern x86-64 processor
- **GPU**: DirectX 12 compatible
- **Clang 20+** (LLVM), **CMake 3.25+**, **Ninja** (for building from source)
- **RexGlue SDK** compiled and installed
- A legitimate copy of **Guitar Hero: Warriors of Rock** (Xbox 360)

## How to Install

1. Download the latest release from the [Releases](https://github.com/ronniecloud/re-wor/releases/latest) page.
2. Extract the contents of your Xbox 360 ISO into the `assets/` folder.

```
assets/
├── default.xex          # Xbox 360 executable
└── data/                # Game data
    ├── streams/         # Audio (.fsb.xen)
    └── scripts/         # QScript (.qb.xen)
```

3. Run `Guitar Hero Warriors of Rock.exe`.

> [!NOTE]
> You can use `extract-xiso` or any equivalent tool to extract the ISO contents.

> [!TIP]
> Connect your USB guitar controller before launching. It will be auto-detected via XINPUT.

## Features

### Native PC Recompilation

The entire Xbox 360 executable has been statically recompiled from PowerPC to x86-64. This is not emulation — the game runs natively on your hardware with no translation overhead at runtime.

### XINPUT Controller Support

Guitar controllers and gamepads are supported natively through XINPUT. Plug in your USB guitar and it will be detected automatically. No configuration or mapping required.

### Multi-Channel Audio

The original XMA2 audio streams are decoded and played back through SDL with full 6-channel support. The audio pipeline preserves the original mix as faithfully as possible.

### Fullscreen Mode

Press **F11** at any time to toggle between windowed and fullscreen modes.

### Discord Rich Presence

The game integrates with Discord to show your current activity status while playing.

## Known Issues

- Slight audio delay relative to the note chart in some configurations.
- Rendering is locked to the original 720p resolution. Higher resolution support is in development.
- Possible crashes during song transitions or scene changes.
- Game logic runs at the original 60Hz tick rate.

## FAQ

**Q: Do I need the original game?**
Yes. This project does not include any copyrighted game assets. You must own a legitimate copy of Guitar Hero: Warriors of Rock for Xbox 360.

**Q: Is this emulation?**
No. The game has been statically recompiled from PowerPC to native x86-64 code. It runs directly on your CPU without any emulation layer.

**Q: My guitar controller isn't detected?**
Make sure it's connected before launching the game. The controller must be XINPUT-compatible.

**Q: The audio is slightly off-sync?**
This is a known issue being worked on. Audio timing calibration improvements are planned for future releases.

## Building from Source

```bash
# Clone the repository
git clone --recursive https://github.com/ronniecloud/re-wor.git
cd re-wor

# Configure
cmake --preset win-amd64-relwithdebinfo

# Generate recompiled code
cmake --build out/build/win-amd64-relwithdebinfo --target re_wor_codegen

# Build
cmake --build out/build/win-amd64-relwithdebinfo
```

> [!NOTE]
> The RexGlue SDK must be compiled and installed before building this project.

## Project Structure

```
re-wor/
├── CMakeLists.txt          # Build system
├── CMakePresets.json        # Build presets
├── re_wor_config.toml       # PPC function map (codegen input)
├── src/
│   ├── main.cpp             # Entry point, app lifecycle, crash handler
│   ├── stubs.cpp            # Xbox 360 API stubs (XAM, avatars, marketplace)
│   └── discord_rpc.cpp      # Discord Rich Presence integration
├── generated/               # Codegen output (auto-generated, gitignored)
└── assets/                  # Game files from ISO (gitignored)
```

## Engine Info

Guitar Hero: Warriors of Rock runs on the **Neversoft engine**, shared with the Tony Hawk series:

- **Filesystem**: PAK/PAB archives (`.pak.xen` / `.pab.xen`)
- **Audio**: XMA2 encoded, stored in FMOD Sound Banks (`.fsb.xen`)
- **Scripting**: QScript (Neversoft proprietary bytecode)
- **Title ID**: `41560883`

## Credits

- **[RexGlue SDK](https://github.com/rexglue/rexglue-sdk)** — Recompilation toolchain
- **[YoshiCrystal9](https://github.com/YoshiCrystal9/re-gh2)** — re-gh2 (Guitar Hero 2 recomp, base reference)
- **Neversoft / Activision** — Original game development

## Bug Reports

Found a bug? Please open an [issue](https://github.com/ronniecloud/re-wor/issues) with steps to reproduce.
