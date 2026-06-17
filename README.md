# Mini Spire

<p align="right">
  <strong>English</strong> | <a href="README.zh-CN.md">简体中文</a>
</p>

> A C++ / SFML roguelike deckbuilder vertical slice with a complete playable run, original content, portable Windows packaging, and a clean object-oriented architecture.

<p align="center">
  <img src="assets/images/backgrounds/title_spire.png" alt="Mini Spire title artwork" width="820">
</p>

## Overview

Mini Spire is a compact, playable deckbuilding game inspired by the structure of roguelike card battlers: map routing, turn-based combat, card rewards, shops, rest sites, events, relics, potions, and multi-act boss progression.

The project uses original game text and assets. It does not reuse proprietary content from commercial titles. The codebase is organized around explicit C++ object-oriented boundaries: core rules are separated from rendering and input handling, so the gameplay logic can be tested without launching an SFML window.

## Key Features

- Three playable characters with distinct starting decks, card pools, maximum HP, relics, and play styles.
- Three-layer run structure with map navigation, regular fights, elite fights, shops, rest sites, events, and bosses.
- Turn-based card combat with energy, draw/discard/exhaust piles, block, strength, weak, vulnerable, ritual, enemy intents, potions, and relic effects.
- Portable Windows release package: download a zip, extract it, and run the game directly.
- PNG title, map, combat, character, enemy, elite, and boss artwork with procedural fallbacks when assets are missing.
- Built-in help overlay, save/resume, run history, fullscreen toggle, and responsive window scaling.
- Core regression tests for combat, map progression, rewards, layout constraints, save/load, card removal, character pools, and run history.

## Download

Players do not need Visual Studio, CMake, or SFML.

1. Open the release page:

   <https://github.com/Jiashu-Yu/mini-spire/releases>

2. Download `MiniSpire-Windows-x64.zip` from the latest release.
3. Extract the entire archive.
4. Run `mini_spire.exe`.

Keep `mini_spire.exe`, the DLL files, and the `assets/` directory in the same extracted folder. The executable expects those runtime files to be next to it.

If Windows SmartScreen reports an unknown publisher, choose `More info`, then `Run anyway`. The project is not code-signed.

## Controls

| Action | Input |
| --- | --- |
| Start a run | Click the start button on the title screen |
| Play a card | Left-click a card in hand |
| End turn | Click the end-turn button |
| Use potion | Left-click a potion slot in combat |
| Discard potion | Right-click a potion slot in combat |
| Open help | Press `F1` or click `?` |
| Toggle fullscreen | Press `F11` or `Alt+Enter` |
| Close overlay | Press `Esc` |

## Build From Source

### Requirements

- Windows 10 / Windows 11 x64
- Visual Studio 2022 with `Desktop development with C++`
- CMake 3.24+
- Git
- SFML 2.6.2, `Visual C++ 17 (2022) - 64-bit`

Mini Spire targets the SFML 2.6 API. SFML 3.x is not currently supported without source changes.

### Clone

```powershell
git clone https://github.com/Jiashu-Yu/mini-spire.git
cd mini-spire
```

### One-Command Windows Build

The helper script searches for Visual Studio, configures CMake, builds the game, runs tests, copies SFML runtime DLLs, and starts the executable.

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\scripts\run_windows.ps1
```

If SFML is not installed at `D:\Libraries\SFML-2.6.2`, pass its location explicitly:

```powershell
.\scripts\run_windows.ps1 -SfmlRoot "C:\Tools\SFML-2.6.2"
```

### Manual Build

```powershell
cmake -S . -B build\sfml-x64 `
  -G "NMake Makefiles" `
  -DSFML_DIR="D:/Libraries/SFML-2.6.2/lib/cmake/SFML" `
  -DMINISPIRE_BUILD_APP=ON `
  -DMINISPIRE_BUILD_TESTS=ON `
  -DCMAKE_BUILD_TYPE=Release

cmake --build build\sfml-x64
ctest --test-dir build\sfml-x64 --output-on-failure

Copy-Item D:\Libraries\SFML-2.6.2\bin\*.dll build\sfml-x64\ -Force
.\build\sfml-x64\mini_spire.exe
```

### Core Logic Only

SFML is not required when building only the rules-layer tests.

```powershell
cmake -S . -B build\core-msvc `
  -G "NMake Makefiles" `
  -DMINISPIRE_BUILD_APP=OFF `
  -DMINISPIRE_BUILD_TESTS=ON

cmake --build build\core-msvc
ctest --test-dir build\core-msvc --output-on-failure
```

## Portable Package

Maintainers can generate the Windows player package with:

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\scripts\package_windows.ps1
```

Output:

```text
dist\MiniSpire-Windows-x64.zip
```

The archive contains `mini_spire.exe`, `assets/`, SFML runtime DLLs, OpenAL, and `README_PLAY.txt`.

## Architecture

```text
GameApp
  Owns the SFML window, ResourceManager, active Scene, and RunController.

Scene
  MainMenuScene, CharacterSelectScene, MapScene, CombatScene,
  RewardScene, ShopScene, RestScene, EventScene, and GameOverScene.

RunController
  Owns run-level state: character, deck, map, relics, potions,
  rewards, shop state, save/load, history, and level transitions.

CombatState
  Owns battle rules: turns, energy, card play, enemy moves,
  draw/discard/exhaust piles, damage, block, statuses, victory, and defeat.

Card + Effect
  Models cards as data composed from reusable effects instead of one subclass
  per card.

ResourceManager + UI helpers
  Load fonts/textures and render cards, panels, sprites, bars, buttons,
  effects, and overlays.
```

The main design boundary is between game rules and presentation. `CombatState` and `RunController` do not depend on SFML rendering; UI scenes call their public methods and translate state into visuals.

## Repository Layout

```text
include/minispire/
  Core.h        Core gameplay interfaces and data types
  GameApp.h     SFML application and resource interfaces
  Layout.h      Responsive UI layout helpers
  Scene.h       Scene base class
  UI.h          Shared UI rendering helpers

src/core/
  Core.cpp      Cards, combat, enemies, map, rewards, shop, save/history

src/app/
  GameApp.cpp   Window, resources, application loop, scene switching
  Scenes.cpp    Menu, map, combat, reward, shop, rest, event, end screens
  UI.cpp        Cards, buttons, panels, health bars, sprites, status text

assets/images/
  backgrounds/  Title, map, and combat backgrounds
  sprites/      Player, enemy, elite, and boss transparent PNG sprites

scripts/
  run_windows.ps1      Build, test, copy DLLs, and run locally
  package_windows.ps1  Build the portable Windows release zip

tests/
  core_tests.cpp       Rules-layer regression tests
```

## Validation

The current release has been validated with:

```powershell
.\scripts\package_windows.ps1
```

This command performs a Release build, runs `ctest`, checks the portable package contents, copies runtime DLLs, and creates `dist\MiniSpire-Windows-x64.zip`.

## Notes

- Windows is the primary supported platform.
- The repository stores source code and game assets. Generated build directories and release zips are intentionally ignored.
- Release binaries are distributed through GitHub Releases rather than committed to the main branch.
- All gameplay names and content are original to this project.
