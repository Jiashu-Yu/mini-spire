# Mini Spire

> A C++ / SFML roguelike deckbuilder vertical slice with a complete playable run, original content, portable Windows packaging, and a clean object-oriented architecture.
>
> 一个使用 C++ / SFML 实现的原创卡牌爬塔游戏竖切版，包含完整可玩的三层流程、原创内容、Windows 免安装发布包和清晰的面向对象架构。

<p align="center">
  <img src="assets/images/backgrounds/title_spire.png" alt="Mini Spire title artwork" width="820">
</p>

## Overview / 项目概览

Mini Spire is a compact, playable deckbuilding game inspired by the structure of roguelike card battlers: map routing, turn-based combat, card rewards, shops, rest sites, events, relics, potions, and multi-act boss progression. The project uses original game text and assets; it does not reuse proprietary content from commercial titles.

Mini Spire 是一个小型但完整可玩的原创卡牌爬塔游戏。项目参考了 roguelike deckbuilder 的系统结构，包括地图路径、回合制战斗、战斗奖励、商店、休息点、事件、遗物、药水和多层 Boss 流程。项目文本与素材均为原创或项目内资产，不复用商业游戏 IP 内容。

The codebase is structured around clear C++ object-oriented boundaries with a real game loop and graphical interface. Game rules are kept separate from rendering and input handling, making the core logic testable without SFML.

代码采用清晰的 C++ 面向对象边界组织完整的游戏循环和图形界面。规则层与界面层分离，核心玩法逻辑可以在不依赖 SFML 窗口的情况下进行测试。

## Highlights / 功能亮点

- Three playable characters with distinct starting decks, card pools, maximum HP, relics, and play styles.
- 3 个可选角色，拥有不同初始牌组、专属奖励卡池、生命上限、初始遗物和玩法节奏。
- Three-layer run structure with map navigation, regular fights, elite fights, shops, rest sites, events, and bosses.
- 三层爬塔流程，包含普通战斗、精英战、商店、休息、事件和 Boss 节点。
- Turn-based card combat with energy, draw/discard/exhaust piles, block, strength, weak, vulnerable, ritual, enemy intents, potions, and relic effects.
- 回合制卡牌战斗，支持能量、抽牌/弃牌/消耗、格挡、力量、虚弱、易伤、仪式、敌人意图、药水和遗物效果。
- Portable Windows package: players can download a zip, extract it, and run the game directly.
- Windows 免安装发布包：玩家下载 zip、解压、双击 exe 即可游玩。
- PNG backgrounds and character/enemy/boss sprites, with procedural fallbacks when assets are missing.
- PNG 背景、角色、敌人和 Boss 立绘；素材缺失时提供程序化绘制回退。
- Built-in help overlay, save/resume, run history, fullscreen toggle, and responsive window scaling.
- 内置帮助文档、存档继续、历史记录、全屏切换和窗口缩放适配。
- Core logic tests cover combat, map progression, rewards, layout constraints, save/load, card removal, character pools, and history records.
- 核心逻辑测试覆盖战斗、地图推进、奖励生成、布局约束、存档读档、商店删牌、角色卡池和历史记录。

## Download / 下载游玩

For players, no compiler or SFML installation is required.

普通玩家不需要安装编译器、CMake 或 SFML。

1. Open the release page:

   <https://github.com/Jiashu-Yu/mini-spire/releases>

2. Download `MiniSpire-Windows-x64.zip` from the latest release.
3. Extract the entire archive.
4. Run `mini_spire.exe`.

Do not move `mini_spire.exe` out of the extracted folder. The executable expects the `assets/` directory and DLL files to remain next to it.

请不要把 `mini_spire.exe` 单独拖出文件夹运行；它需要旁边保留 `assets/` 目录和 DLL 文件。

If Windows SmartScreen reports an unknown publisher, choose `More info` and then `Run anyway`. This project is not code-signed.

如果 Windows SmartScreen 提示未知发布者，选择 `更多信息`，再选择 `仍要运行`。本项目没有购买代码签名证书。

## Controls / 操作

| Action | English | 中文 |
| --- | --- | --- |
| Start a run | Click `Start` on the title screen. | 在主菜单点击开始按钮。 |
| Play a card | Left-click a card in hand. | 战斗中左键点击手牌。 |
| End turn | Click `End Turn`. | 点击 `结束回合`。 |
| Use potion | Left-click a potion slot in combat. | 战斗中左键点击药水槽。 |
| Discard potion | Right-click a potion slot in combat. | 战斗中右键点击药水槽。 |
| Help | Press `F1` or click `?`. | 按 `F1` 或点击 `?`。 |
| Fullscreen | Press `F11` or `Alt+Enter`. | 按 `F11` 或 `Alt+Enter`。 |
| Close overlay | Press `Esc`. | 按 `Esc` 关闭弹窗。 |

## Build From Source / 从源码构建

### Requirements / 环境要求

- Windows 10 / Windows 11 x64
- Visual Studio 2022 with `Desktop development with C++`
- CMake 3.24+
- Git
- SFML 2.6.2, `Visual C++ 17 (2022) - 64-bit`

The project targets the SFML 2.6 API. SFML 3.x is not currently supported without source changes.

项目当前使用 SFML 2.6 API。SFML 3.x 需要修改代码后才能使用。

### Clone / 克隆仓库

```powershell
git clone https://github.com/Jiashu-Yu/mini-spire.git
cd mini-spire
```

### One-Command Windows Build / Windows 一键构建运行

The helper script searches for Visual Studio, configures CMake, builds the game, runs tests, copies SFML runtime DLLs, and starts the executable.

该脚本会自动查找 Visual Studio，配置 CMake，编译游戏，运行测试，复制 SFML 运行时 DLL，并启动游戏。

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\scripts\run_windows.ps1
```

If SFML is not installed at `D:\Libraries\SFML-2.6.2`, pass its location explicitly:

如果 SFML 不在 `D:\Libraries\SFML-2.6.2`，可以显式指定路径：

```powershell
.\scripts\run_windows.ps1 -SfmlRoot "C:\Tools\SFML-2.6.2"
```

### Manual Build / 手动构建

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

### Core Logic Only / 仅构建核心逻辑测试

SFML is not required when building only the rules-layer tests.

如果只运行规则层测试，可以不安装 SFML。

```powershell
cmake -S . -B build\core-msvc `
  -G "NMake Makefiles" `
  -DMINISPIRE_BUILD_APP=OFF `
  -DMINISPIRE_BUILD_TESTS=ON

cmake --build build\core-msvc
ctest --test-dir build\core-msvc --output-on-failure
```

## Portable Package / 免安装包生成

Maintainers can generate the Windows player package with:

发布者可以用下面的命令生成 Windows 玩家版 zip：

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\scripts\package_windows.ps1
```

Output:

```text
dist\MiniSpire-Windows-x64.zip
```

The archive contains `mini_spire.exe`, `assets/`, SFML runtime DLLs, OpenAL, and `README_PLAY.txt`.

压缩包内包含 `mini_spire.exe`、`assets/`、SFML 运行时 DLL、OpenAL 和 `README_PLAY.txt`。

## Architecture / 架构

```text
GameApp
  owns the SFML window, ResourceManager, active Scene, and RunController

Scene
  MainMenuScene, CharacterSelectScene, MapScene, CombatScene,
  RewardScene, ShopScene, RestScene, EventScene, GameOverScene

RunController
  owns run-level state: character, deck, map, relics, potions,
  rewards, shop state, save/load, history, and level transitions

CombatState
  owns battle rules: turns, energy, card play, enemy moves,
  draw/discard/exhaust piles, damage, block, statuses, victory/defeat

Card + Effect
  data-driven card behavior; cards compose reusable effects instead of
  requiring one subclass per card

ResourceManager + UI helpers
  load fonts/textures and render cards, panels, sprites, bars, buttons,
  effects, and overlays
```

The main design boundary is between game rules and presentation. `CombatState` and `RunController` do not depend on SFML rendering; UI scenes call their public methods and translate state into visuals.

项目的主要边界是规则层与表现层分离。`CombatState` 和 `RunController` 不依赖 SFML 绘制；界面场景通过公开接口读取和推进规则状态，再把状态渲染成图形界面。

## Repository Layout / 目录结构

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
  core_tests.cpp  Rules-layer regression tests
```

## Validation / 验证

The current release has been validated with:

当前版本已通过以下验证：

```powershell
.\scripts\package_windows.ps1
```

This command performs a Release build, runs `ctest`, checks the portable package contents, copies runtime DLLs, and creates `dist\MiniSpire-Windows-x64.zip`.

该命令会执行 Release 构建、运行 `ctest`、检查免安装包内容、复制运行时 DLL，并生成 `dist\MiniSpire-Windows-x64.zip`。

## Notes / 说明

- Windows is the primary supported platform.
- The repository stores source code and game assets. Generated build directories and release zips are intentionally ignored.
- Release binaries are distributed through GitHub Releases rather than committed to the main branch.
- All gameplay names and content are original to this project.

- 当前主要支持 Windows。
- 仓库存放源码和游戏素材；构建目录与发布 zip 不进入主分支。
- 可执行发布包通过 GitHub Releases 分发。
- 游戏内名称、文本和内容为本项目原创。
