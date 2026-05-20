# Mini Spire

Mini Spire 是一个用 **C++ + SFML** 实现的原创卡牌爬塔游戏竖切版。项目参考 roguelike deckbuilder 的玩法结构：地图路径、回合制战斗、能量、抽牌/弃牌、敌人意图、战斗奖励、商店、休息、事件与 Boss 通关。

这个项目的重点不是复刻某个商业游戏，而是练习如何用 **OOP（面向对象程序设计）** 组织一个稍复杂的 C++ 图形界面游戏。

## 当前内容

- 1 名角色：裂隙旅人
- 12 张初始牌，20 张奖励卡牌
- 3 个普通敌人、1 个精英敌人、1 个 Boss
- 地图、战斗、奖励、商店、休息、事件、胜利/失败界面
- 鼠标点击操作、卡牌 hover、血条、格挡、敌人意图、战斗日志
- 纯逻辑测试：战斗起手、能量消耗、伤害、格挡、地图解锁、奖励生成

## 游戏操作

- 主菜单点击 `开始爬塔`。
- 地图中点击高亮节点进入战斗、商店、休息或事件。
- 战斗中点击手牌出牌。
- 点击 `结束回合` 让敌人行动。
- 战斗胜利后选择一张奖励牌，或跳过拿金币。
- 击败最终 Boss 后通关。

## 环境要求

推荐环境：

- Windows 10 / Windows 11 64-bit
- Visual Studio 2022
- Visual Studio workload：`Desktop development with C++`
- CMake 3.24+
- Git
- SFML 2.6.2，Visual C++ 17 (2022) - 64-bit

项目当前使用的是 **SFML 2.6.x API**。如果你安装 SFML 3.x，可能需要修改少量代码。

## 从 GitHub 克隆

```powershell
git clone https://github.com/Jiashu-Yu/mini-spire.git
cd mini-spire
```

## 安装 SFML

1. 打开 SFML 2.6.2 下载页：

   <https://www.sfml-dev.org/download/sfml/2.6.2/>

2. 下载：

   ```text
   Visual C++ 17 (2022) - 64-bit
   ```

3. 解压到一个固定位置，例如：

   ```text
   D:\Libraries\SFML-2.6.2
   ```

4. 确认这个文件存在：

   ```text
   D:\Libraries\SFML-2.6.2\lib\cmake\SFML\SFMLConfig.cmake
   ```

如果你解压到别的位置，后面的 `SFML_DIR` 改成你的实际路径即可。

## 构建并运行游戏

打开 `Developer PowerShell for VS 2022`，进入项目目录：

```powershell
cd <你的项目路径>\mini-spire
```

确保当前 shell 是 x64 编译环境：

如果你打开的已经是 `Developer PowerShell for VS 2022`，通常可以直接执行 `where cl`。如果输出不是 x64，或者找不到 `cl`，再执行下面命令：

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64
where cl
```

如果你的 Visual Studio 不是 Community 版本，请把路径中的 `Community` 改成 `Professional`、`Enterprise` 或你的实际安装版本。

`where cl` 的输出应该包含：

```text
Hostx64\x64\cl.exe
```

然后配置和编译：

```powershell
cmake -S . -B build\sfml-x64 `
  -G "NMake Makefiles" `
  -DSFML_DIR="D:/Libraries/SFML-2.6.2/lib/cmake/SFML" `
  -DMINISPIRE_BUILD_APP=ON `
  -DMINISPIRE_BUILD_TESTS=ON

cmake --build build\sfml-x64
```

复制 SFML 运行时 DLL 到 exe 同目录：

```powershell
Copy-Item D:\Libraries\SFML-2.6.2\bin\*.dll build\sfml-x64\ -Force
```

运行游戏：

```powershell
.\build\sfml-x64\mini_spire.exe
```

## 只运行核心逻辑测试

如果你暂时没有安装 SFML，也可以只构建规则层测试：

```powershell
cmake -S . -B build\core-msvc `
  -G "NMake Makefiles" `
  -DMINISPIRE_BUILD_APP=OFF `
  -DMINISPIRE_BUILD_TESTS=ON

cmake --build build\core-msvc
ctest --test-dir build\core-msvc --output-on-failure
```

测试通过时会看到类似：

```text
100% tests passed
```

## 常见问题

### CMake 提示 SFML was not found

检查 `SFML_DIR` 是否指向这个目录：

```text
D:/Libraries/SFML-2.6.2/lib/cmake/SFML
```

注意它不是 SFML 根目录，而是包含 `SFMLConfig.cmake` 的目录。

### 构建成功但没有 mini_spire.exe

说明 CMake 没有找到 SFML，所以只构建了核心测试。重新检查 `SFML_DIR`，然后换一个新的 build 目录重新配置，例如：

```powershell
cmake -S . -B build\sfml-x64-new `
  -G "NMake Makefiles" `
  -DSFML_DIR="D:/Libraries/SFML-2.6.2/lib/cmake/SFML" `
  -DMINISPIRE_BUILD_APP=ON `
  -DMINISPIRE_BUILD_TESTS=ON
```

### CMake 用了 Hostx86/x86/cl.exe

你当前是 32-bit 编译环境，但下载的是 64-bit SFML。执行：

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64
where cl
```

如果你的 Visual Studio 不是 Community 版本，请把路径中的 `Community` 改成你的实际安装版本。

确认输出包含 `Hostx64\x64\cl.exe` 后，再用新的 build 目录重新配置。

### 运行时报缺少 sfml-xxx-2.dll

把 SFML 的 DLL 复制到 exe 所在目录：

```powershell
Copy-Item D:\Libraries\SFML-2.6.2\bin\*.dll build\sfml-x64\ -Force
```

### Git Bash 里命令报错

README 里的多行命令是 PowerShell 写法，换行符是反引号：

```powershell
`
```

Git Bash 需要用反斜杠：

```bash
cmake -S . -B build/sfml-x64 \
  -G "NMake Makefiles" \
  -DSFML_DIR="D:/Libraries/SFML-2.6.2/lib/cmake/SFML" \
  -DMINISPIRE_BUILD_APP=ON \
  -DMINISPIRE_BUILD_TESTS=ON
```

不过本项目更推荐使用 `Developer PowerShell for VS 2022`。

## 项目结构

```text
include/minispire/
  Core.h        核心规则接口
  GameApp.h     SFML 应用入口
  Scene.h       场景基类
  UI.h          UI 绘制工具

src/core/
  Core.cpp      卡牌、战斗、敌人、地图、奖励等规则实现

src/app/
  GameApp.cpp   窗口、资源、场景切换
  Scenes.cpp    主菜单、地图、战斗、奖励、商店等场景
  UI.cpp        按钮、卡牌、血条、角色面板

tests/
  core_tests.cpp  纯规则层测试
```

## OOP 设计说明

### `GameApp`

程序入口对象。它持有窗口、资源管理器、当前场景和整局游戏状态。主循环只负责三件事：

1. 接收输入事件。
2. 更新当前场景。
3. 绘制当前场景。

这样主循环不会被战斗、商店、地图等细节塞满。

### `Scene`

每个界面都是一个 `Scene` 子类，例如主菜单、地图、战斗、奖励、商店、休息和事件。

场景对象负责自己的输入、更新和绘制。切换界面时，只需要让 `GameApp` 替换当前场景。

### `Creature` / `Player` / `Enemy`

`Creature` 保存生命、最大生命、格挡、状态效果等共同属性。

`Player` 增加能量、金币、遗物。

`Enemy` 增加敌人类型和行动意图。

这体现了继承适合用在“确实存在 is-a 关系”的地方：玩家和敌人都是生物。

### `Card` + `Effect`

卡牌没有设计成“一张卡一个子类”。每张卡是一份数据，里面包含多个 `Effect`：

- 造成伤害
- 获得格挡
- 抽牌
- 施加虚弱
- 施加易伤
- 回复生命
- 获得能量

这样新增卡牌时，通常只需要新增数据，而不是新增大量类。这个设计更适合课程项目，也更容易扩展。

### `CombatState`

`CombatState` 管理一场战斗的完整规则：

- 开始战斗
- 抽牌
- 出牌
- 消耗能量
- 结算卡牌效果
- 敌人行动
- 胜负判定

UI 不直接修改生命和牌堆，而是调用 `CombatState` 的公开接口。这是为了保持“规则层”和“表现层”分离。

### `RunController`

`RunController` 管理一整局爬塔：

- 玩家状态
- 当前牌组
- 地图节点
- 当前楼层
- 战斗奖励
- 商店卡牌
- 休息和事件

它相当于“整局游戏的导演”。

## 后续扩展建议

1. 为 `Card` 增加升级字段，休息点可选择升级卡牌。
2. 把敌人和卡牌数据移到 JSON，练习数据驱动设计。
3. 增加 `Relic` 类，让遗物能监听战斗事件。
4. 给 `CombatState` 增加更细的事件类型，用于动画和音效。
5. 增加存档系统，保存当前 run。
6. 增加 Release 构建和 GitHub Releases，方便别人直接下载 exe 游玩。
