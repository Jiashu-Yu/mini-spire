# Mini Spire

Mini Spire 是一个 C++ / SFML 实现的原创卡牌爬塔竖切版。项目参考 roguelike deckbuilder 的结构：地图路径、回合制战斗、能量、抽牌/弃牌、敌人意图、奖励、商店、休息、事件与 Boss 通关。

## 构建方式

推荐用 Visual Studio 2022 打开本文件夹作为 CMake 项目，或在 “Developer PowerShell for VS 2022” 中执行下面命令。

如果 SFML 下载的是 64-bit 版本，请确认开发环境也是 x64。PowerShell 里可以这样切换：

```powershell
& "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\Launch-VsDevShell.ps1" -Arch amd64
```

只验证核心规则层：

```powershell
cmake --preset core-msvc
cmake --build --preset core-msvc
ctest --preset core-msvc
```

如果你使用 vcpkg：

```powershell
vcpkg install sfml:x64-windows
$env:VCPKG_ROOT="<你的 vcpkg 路径>"
cmake --preset app-vcpkg-msvc
cmake --build --preset app-vcpkg-msvc
ctest --preset app-vcpkg-msvc
```

如果暂时没有 SFML，CMake 仍会构建 `mini_spire_core_tests`，用于验证核心规则层。

## 操作

- 主菜单点击“开始爬塔”。
- 地图中点击高亮节点进入战斗、商店、休息或事件。
- 战斗中点击手牌出牌，点击“结束回合”让敌人行动。
- 战斗胜利后选择奖励牌，最终击败 Boss 通关。

## OOP 结构

- `RunController`：管理整局 run，包括玩家状态、牌组、地图、奖励与当前节点。
- `CombatState`：管理一场战斗，包括抽牌、出牌、敌人行动、状态与胜负判定。
- `Card` + `Effect`：卡牌不靠大量子类实现，而是由多个效果组合，新增卡牌主要改数据。
- `Creature` / `Player` / `Enemy`：把生命、格挡、状态等共同逻辑抽到基类。
- `Scene`：每个界面是独立对象，窗口主循环不关心具体界面的细节。
- `UI`：按钮、卡牌、血条等绘制函数集中管理，避免渲染代码散落在规则层。

这套拆分的核心思想是：规则层不依赖 SFML，界面层只调用规则层的公开接口。以后扩展存档、更多角色、更多卡牌或自动测试时，改动范围会更可控。

## 当前内容

- 1 名角色：裂隙旅人
- 20 张奖励卡牌，12 张初始牌
- 3 个普通敌人、1 个精英、1 个 Boss
- 地图、战斗奖励、商店、休息、事件、胜利/失败界面
- 纯逻辑测试：战斗起手、能量消耗、伤害、格挡、地图解锁、奖励生成

## 后续扩展建议

1. 为 `Card` 增加升级字段，休息点可选择升级卡牌。
2. 把敌人和卡牌数据移到 JSON，练习数据驱动设计。
3. 增加 `Relic` 类，让遗物能监听战斗事件。
4. 给 `CombatState` 增加更细的事件类型，用于动画和音效。
5. 把测试扩展到 Boss 半血转阶段、虚弱/易伤持续时间、商店购买流程。
