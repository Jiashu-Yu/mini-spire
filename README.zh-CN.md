# Mini Spire

<p align="right">
  <a href="README.md">English</a> | <strong>简体中文</strong>
</p>

> 一个使用 C++ / SFML 实现的原创卡牌爬塔游戏竖切版，包含完整可玩的三层流程、原创内容、Windows 免安装发布包和清晰的面向对象架构。

<p align="center">
  <img src="assets/images/backgrounds/title_spire.png" alt="Mini Spire 标题背景" width="820">
</p>

## 项目概览

Mini Spire 是一个小型但完整可玩的原创卡牌爬塔游戏。项目参考 roguelike deckbuilder 的系统结构，包括地图路径、回合制战斗、战斗奖励、商店、休息点、事件、遗物、药水和多层 Boss 流程。

项目文本与素材均为原创或项目内资产，不复用商业游戏 IP 内容。代码采用清晰的 C++ 面向对象边界组织完整的游戏循环和图形界面；规则层与界面层分离，核心玩法逻辑可以在不启动 SFML 窗口的情况下测试。

## 功能亮点

- 3 个可选角色，拥有不同初始牌组、专属奖励卡池、生命上限、初始遗物和玩法节奏。
- 三层爬塔流程，包含地图路线、普通战斗、精英战、商店、休息、事件和 Boss 节点。
- 回合制卡牌战斗，支持能量、抽牌/弃牌/消耗、格挡、力量、虚弱、易伤、仪式、敌人意图、药水和遗物效果。
- Windows 免安装发布包：下载 zip、解压、双击 exe 即可游玩。
- PNG 标题、地图、战斗、角色、敌人、精英和 Boss 美术；素材缺失时提供程序化绘制回退。
- 内置帮助文档、存档继续、历史记录、全屏切换和窗口缩放适配。
- 核心回归测试覆盖战斗、地图推进、奖励生成、布局约束、存档读档、商店删牌、角色卡池和历史记录。

## 下载游玩

普通玩家不需要安装 Visual Studio、CMake 或 SFML。

1. 打开发布页面：

   <https://github.com/Jiashu-Yu/mini-spire/releases>

2. 下载最新版本中的 `MiniSpire-Windows-x64.zip`。
3. 解压整个压缩包。
4. 运行 `mini_spire.exe`。

请保持 `mini_spire.exe`、DLL 文件和 `assets/` 目录位于同一个解压目录中。可执行文件运行时会从旁边读取这些运行时文件。

如果 Windows SmartScreen 提示未知发布者，选择 `更多信息`，再选择 `仍要运行`。本项目没有代码签名。

## 操作

| 行为 | 输入 |
| --- | --- |
| 开始一局 | 在标题界面点击开始按钮 |
| 打出卡牌 | 战斗中左键点击手牌 |
| 结束回合 | 点击结束回合按钮 |
| 使用药水 | 战斗中左键点击药水槽 |
| 丢弃药水 | 战斗中右键点击药水槽 |
| 打开帮助 | 按 `F1` 或点击 `?` |
| 切换全屏 | 按 `F11` 或 `Alt+Enter` |
| 关闭弹窗 | 按 `Esc` |

## 从源码构建

### 环境要求

- Windows 10 / Windows 11 x64
- Visual Studio 2022，安装 `Desktop development with C++`
- CMake 3.24+
- Git
- SFML 2.6.2，`Visual C++ 17 (2022) - 64-bit`

Mini Spire 当前使用 SFML 2.6 API。SFML 3.x 需要修改代码后才能使用。

### 克隆仓库

```powershell
git clone https://github.com/Jiashu-Yu/mini-spire.git
cd mini-spire
```

### Windows 一键构建运行

辅助脚本会自动查找 Visual Studio，配置 CMake，编译游戏，运行测试，复制 SFML 运行时 DLL，并启动游戏。

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\scripts\run_windows.ps1
```

如果 SFML 不在 `D:\Libraries\SFML-2.6.2`，可以显式指定路径：

```powershell
.\scripts\run_windows.ps1 -SfmlRoot "C:\Tools\SFML-2.6.2"
```

### 手动构建

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

### 仅构建核心逻辑测试

如果只运行规则层测试，可以不安装 SFML。

```powershell
cmake -S . -B build\core-msvc `
  -G "NMake Makefiles" `
  -DMINISPIRE_BUILD_APP=OFF `
  -DMINISPIRE_BUILD_TESTS=ON

cmake --build build\core-msvc
ctest --test-dir build\core-msvc --output-on-failure
```

## 免安装包生成

发布者可以用下面的命令生成 Windows 玩家版 zip：

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\scripts\package_windows.ps1
```

输出：

```text
dist\MiniSpire-Windows-x64.zip
```

压缩包内包含 `mini_spire.exe`、`assets/`、SFML 运行时 DLL、OpenAL 和 `README_PLAY.txt`。

## 架构

```text
GameApp
  持有 SFML 窗口、ResourceManager、当前 Scene 和 RunController。

Scene
  MainMenuScene、CharacterSelectScene、MapScene、CombatScene、
  RewardScene、ShopScene、RestScene、EventScene 和 GameOverScene。

RunController
  管理整局 run 的状态：角色、牌组、地图、遗物、药水、
  奖励、商店状态、存档读档、历史记录和楼层转换。

CombatState
  管理战斗规则：回合、能量、出牌、敌人行动、
  抽牌/弃牌/消耗牌堆、伤害、格挡、状态、胜利与失败。

Card + Effect
  卡牌由可复用效果组合而成，避免为每张卡牌创建一个子类。

ResourceManager + UI helpers
  加载字体和贴图，并绘制卡牌、面板、精灵、血条、按钮、
  战斗特效和弹窗。
```

项目的主要边界是规则层与表现层分离。`CombatState` 和 `RunController` 不依赖 SFML 绘制；界面场景通过公开接口读取和推进规则状态，再把状态渲染成图形界面。

## 目录结构

```text
include/minispire/
  Core.h        核心玩法接口和数据类型
  GameApp.h     SFML 应用与资源接口
  Layout.h      响应式界面布局工具
  Scene.h       场景基类
  UI.h          通用 UI 绘制工具

src/core/
  Core.cpp      卡牌、战斗、敌人、地图、奖励、商店、存档和历史

src/app/
  GameApp.cpp   窗口、资源、应用循环和场景切换
  Scenes.cpp    菜单、地图、战斗、奖励、商店、休息、事件和结算界面
  UI.cpp        卡牌、按钮、面板、血条、精灵和状态文本

assets/images/
  backgrounds/  标题、地图和战斗背景
  sprites/      玩家、敌人、精英和 Boss 透明 PNG 精灵

scripts/
  run_windows.ps1      本地构建、测试、复制 DLL 并运行
  package_windows.ps1  生成 Windows 免安装发布包

tests/
  core_tests.cpp       规则层回归测试
```

## 验证

当前版本已通过以下命令验证：

```powershell
.\scripts\package_windows.ps1
```

该命令会执行 Release 构建、运行 `ctest`、检查免安装包内容、复制运行时 DLL，并生成 `dist\MiniSpire-Windows-x64.zip`。

## 说明

- 当前主要支持 Windows。
- 仓库存放源码和游戏素材；构建目录与发布 zip 不进入主分支。
- 可执行发布包通过 GitHub Releases 分发。
- 游戏内名称、文本和内容为本项目原创。
