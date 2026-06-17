# Mini Spire

Mini Spire 是一个用 **C++ + SFML** 实现的原创卡牌爬塔游戏竖切版。项目参考 roguelike deckbuilder 的玩法结构：地图路径、回合制战斗、能量、抽牌/弃牌、敌人意图、战斗奖励、商店、休息、事件与 Boss 通关。

这个项目的重点不是复刻某个商业游戏，而是练习如何用 **OOP（面向对象程序设计）** 组织一个稍复杂的 C++ 图形界面游戏。

## 当前内容

- 3 名可选角色：裂隙旅人、余烬使徒、晶盾守卫
- 每名角色拥有独立初始牌组与专属奖励卡池
- 6 个普通敌人、3 个精英敌人、3 个 Boss
- 3 层爬塔流程，每层击败 Boss 后领取奖励并进入下一层
- 商店支持购买卡牌、圣遗物和药水
- 玩家有 2 个药水槽，战斗中可左键使用、右键丢弃药水
- 最高 10 张手牌，超过上限的抽牌会自动进入弃牌堆
- 支持地图界面存档并返回主菜单，主菜单可继续游戏
- 支持历史记录：总局数、成功次数、最近 5 局详情
- 圣遗物拥有开局被动效果，例如额外抽牌、格挡、力量或让敌人虚弱
- 地图、战斗、奖励、商店、休息、事件、胜利/失败界面
- 鼠标点击操作、卡牌 hover、血条、格挡、敌人意图、战斗日志
- PNG 角色、敌人和 Boss 立绘，缺失贴图时才回退到程序化绘制
- PNG 主菜单背景、三层地图主题背景和三层战斗舞台背景
- 全局帮助文档：顶部 `?` 按钮或 `F1` 随时查看规则说明
- 默认 1600×900 可缩放窗口，支持 `F11` / `Alt+Enter` 切换全屏
- 卡牌效果徽章、攻击/护盾/治疗/状态战斗特效
- 纯逻辑测试：战斗起手、能量消耗、伤害、格挡、地图解锁、奖励生成、存档历史

## 游戏操作

- 主菜单点击 `开始爬塔`。
- 如果已有存档，主菜单会显示 `继续游戏`。
- 主菜单点击 `历史记录` 可以查看总局数、成功次数和最近 5 局详情。
- 任意界面点击右上角 `?` 或按 `F1` 可以打开帮助文档；按 `Esc` 关闭。
- 拖拽窗口边缘可以缩放；按 `F11` 或 `Alt+Enter` 可以切换全屏。
- 地图中点击高亮节点进入战斗、商店、休息或事件。
- 地图中点击 `存档并返回` 会保存当前 run 并回到主菜单。
- 战斗中点击手牌出牌。
- 点击 `结束回合` 让敌人行动。
- 药水槽在战斗中左键使用、右键丢弃；使用后对应槽位会立即变空。
- 战斗胜利后选择一张奖励牌，或跳过拿金币。
- 顶部 `楼层 1/3` 表示当前大层，`步数` 表示这局已经走过多少个节点。
- 第 1/2 层击败 Boss 后进入剧情奖励页，生命回满，再选择一份 Boss 奖励进入下一层。
- 第三层击败最终 Boss 后通关。
- 爬塔失败后可以返回主菜单，不会直接关闭程序。

## 直接下载游玩（推荐）

如果你只是想玩游戏，不需要安装 Visual Studio、CMake 或 SFML。

1. 打开 GitHub Releases：

   <https://github.com/Jiashu-Yu/mini-spire/releases>

2. 下载最新版本里的 `MiniSpire-Windows-x64.zip`。
3. 解压整个 zip。
4. 双击解压出来的 `mini_spire.exe`。

请不要把 `mini_spire.exe` 单独拖出来运行，exe 旁边需要保留 `assets/` 文件夹和 DLL 文件。

如果 Windows SmartScreen 提示未知发布者，点击 `更多信息`，再点击 `仍要运行`。这是因为课程项目没有购买代码签名证书。

## 从源码构建（开发者）

下面的步骤适合想查看源码、修改游戏或自己重新打包的人。

开发环境：

- Windows 10 / Windows 11 64-bit
- Visual Studio 2022
- Visual Studio workload：`Desktop development with C++`
- CMake 3.24+
- Git
- SFML 2.6.2，Visual C++ 17 (2022) - 64-bit

项目当前使用的是 **SFML 2.6.x API**。如果你安装 SFML 3.x，可能需要修改少量代码。

## 从 GitHub 克隆

如果你不熟悉 Git，也可以在 GitHub 页面点击绿色 `Code` 按钮，然后点击 `Download ZIP`，解压后进入项目文件夹。

```powershell
git clone https://github.com/Jiashu-Yu/mini-spire.git
cd mini-spire
```

## 从源码一键运行（Windows）

如果你想从源码构建并运行，推荐按下面的步骤走：

1. 安装 Visual Studio 2022，并勾选 `Desktop development with C++`。
2. 下载 SFML 2.6.2 的 `Visual C++ 17 (2022) - 64-bit` 版本。
3. 把 SFML 解压到：

   ```text
   D:\Libraries\SFML-2.6.2
   ```

4. 在项目文件夹空白处按住 `Shift` 并点击鼠标右键，选择 `在终端中打开` 或 `在 PowerShell 中打开`。
5. 执行：

   ```powershell
   Set-ExecutionPolicy -Scope Process Bypass
   .\scripts\run_windows.ps1
   ```

脚本会自动查找 Visual Studio、配置 CMake、编译游戏、复制 SFML DLL、运行测试，然后启动 `mini_spire.exe`。

如果你的 SFML 解压到了别的位置，例如 `C:\Tools\SFML-2.6.2`，运行：

```powershell
.\scripts\run_windows.ps1 -SfmlRoot "C:\Tools\SFML-2.6.2"
```

## 生成免安装 zip（发布者）

如果你想生成可上传到 GitHub Releases 的玩家版本：

```powershell
Set-ExecutionPolicy -Scope Process Bypass
.\scripts\package_windows.ps1
```

脚本会生成：

```text
dist\MiniSpire-Windows-x64.zip
```

这个 zip 已经包含 `mini_spire.exe`、`assets/` 和 SFML 运行时 DLL。玩家解压后可以直接双击 exe。

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
  Layout.h      响应式界面布局
  Scene.h       场景基类
  UI.h          UI 绘制工具

src/core/
  Core.cpp      卡牌、战斗、敌人、地图、奖励等规则实现

src/app/
  GameApp.cpp   窗口、资源、场景切换
  Scenes.cpp    主菜单、地图、战斗、奖励、商店等场景
  UI.cpp        按钮、卡牌、血条、角色面板、角色/敌人贴图绘制

assets/images/
  backgrounds/  主菜单、三层地图和三层战斗背景图
  sprites/      玩家、普通敌人、精英敌人和 Boss 透明 PNG

scripts/
  run_windows.ps1      Windows 一键构建、测试并运行脚本
  package_windows.ps1  Windows 免安装 zip 打包脚本

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
- 当前楼层
- 地图节点
- 当前步数
- 战斗奖励
- 商店卡牌
- 商店圣遗物和药水
- 两个药水槽
- 存档、读档和历史记录
- Boss 后剧情恢复
- 休息和事件
- 进入下一层

它相当于“整局游戏的导演”。

### 美术资源和战斗特效

当前版本已经把主要美术从简单几何图形升级为 PNG 资产：

- `ResourceManager` 负责加载 `assets/images/` 下的 `sf::Texture`，并用 key 提供给 UI 层。
- `drawPlayerSprite` 会根据角色定义中的 `spriteKey` 绘制对应玩家立绘。
- `drawEnemySprite` 根据 `EnemyKind` 映射到普通敌人、精英敌人和 Boss 的透明 PNG。
- 主菜单和三层地图背景优先绘制 `backgrounds/` 下的图片。
- 战斗场景按当前楼层绘制专用舞台背景，让角色、敌人和卡牌不再漂在纯色面板上。
- 如果某张图片缺失，UI 会回退到程序化绘制，保证游戏仍然可以运行。
- `CombatScene` 中的 `VisualEffect` 仍然负责攻击、护盾、治疗、能量、Debuff、Boss 脉冲等短动画。
- `ui::drawCard` 会根据卡牌的 `Effect` 在卡牌底部画效果徽章。

这体现了一个重要 OOP 思路：规则层 `CombatState`、`RunController` 不知道图片在哪里，也不负责画图。美术替换集中在 `ResourceManager` 和 UI 绘制函数里，因此以后升级素材不会破坏战斗规则。

## 后续扩展建议

1. 为 `Card` 增加升级字段，休息点可选择升级卡牌。
2. 把敌人和卡牌数据移到 JSON，练习数据驱动设计。
3. 增加 `Relic` 类，让遗物能监听战斗事件。
4. 给 `CombatState` 增加更细的事件类型，用于动画和音效。
5. 增加更完整的卡牌升级和事件分支。
6. 增加 GitHub Releases，方便别人直接下载 exe 游玩。
7. 为每张卡牌增加独立卡面插画，并把卡牌数据逐步迁移到 JSON。
