# CloudShell 后台任务队列 roadmap（2026-10-25）

> 本文件是 2026-10-25 架构转向后的单一进度源。旧的 CW0–CW10 快照（`roadmap.md` / `roadmap.json`）
> 保留为历史，不再代表当前产品路线。Epic: [#8](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/8)。

## 目标

在 **CloudShell（WebView2/Chromium）后台**运行完整 MaaEnd 任务队列，基于《云·终末地》网页版；
对齐上游 CloudWeb 架构方针，不修改 MaaFramework / MXU / PI，不破坏 PC 业务流程。

## 上游架构方针（优先级）

1. **#6807（P0，维护者口径）**：云游戏约 5%，PC 优先；不破坏架构、不写大量冗余、不大改 PC；
   截图使用现有 PrintWindow/BitBlt 等方案（不做 CPU 视频管线、不逐帧处理）；CDP 输入可在 FW；
   interface 最小；Windows 优先。
2. **#6841（P1，已 approved、CI 全绿、mergeState CLEAN）**：CloudShell（WebView2/Chromium）+
   CloudWeb 控制器（type Win32，`MaaEnd.CloudShell`，1280×720，PrintWindow，PostMessage）+
   pretask `LaunchCloudWeb` + 所有 Win32 任务。
3. **#6800（P2）**：伪 Win32 窗口 + 视频 SharedBuffer；维护者已要求重定向，仅复用页面 JS/
   输入策略/测试思路。

## 阶段与状态

| 阶段 | 状态 | 交付 | Issue |
| --- | --- | --- | --- |
| U0 启动注入 | verified | CloudShell `--inject` 启动脚本 + `LaunchCloudWeb` 接线；dev pretask 冷启动实测 | [#1](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/1) |
| U0 启动任务 | verified | 标题页、今日月卡奖励、进入世界（`CloudWebStartGame`） | [#4](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/4) |
| U0 凭据 | partial | WebView2 会话 DPAPI 保存/恢复 harness；秘密不进入仓库/日志 | [#2](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/2) |
| U1 队列 | partial（手动 3/3；冷启动 2/3） | 同一控制器顺序运行多个任务，失败隔离、状态可观测、Stop 可中断 | [#6](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/6) |
| U2 核心任务 | partial | DailyRewards、Trial、AutoFight；Trial 战后任务级闭环 | [#5](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/5) |
| U3 全量矩阵 | planned | 46 项 Windows 任务的 CloudWeb 资格/动态证据；有副作用任务按清单跳过 | [#7](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/7) |
| U4 后台/长时 | planned | 遮挡/最小化/长时供帧与任务稳定性；边界有证据 | — |
| U5 栈与验收 | in_progress | 基于 upstream/pr-6841 的栈分支、MaaEnd/原生检查、release e2e、fork 同步与上游监控 | [#3](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/3) |

## 已验证（2026-10-25）

- CloudShell 作为 WebView2/Chromium 宿主，窗口类 `MaaEnd.CloudShell`、固定 1280×720；
  启动日志 `foreground=false`，不抢前台。
- Win32 控制器（PrintWindow + PostMessage）已在真实云会话驱动标题页、月卡、进入大世界；
  CloudShell 登录在独立 WebView2 profile 中持久化。
- #6841 interface/resource + CloudWeb overlay 下，`DailyRewards` 在 CloudShell 上 **Succeeded**
  （真实云，taskId 级成功）。
- Trial 的 `Fight` / `AutoFight` / `FightFinish` 节点级 **Succeeded**（有真实演算次数）。
- 非前台/被遮挡的正常窗口 PrintWindow 可抓完整页面；任务期间识别帧正常。
- **首轮完整队列 Succeeded（2026-10-25）**：`CloudWebStartGame` → `DailyRewards` →
  `ClaimSimulationRewards` 三项均 succeeded，overall Succeeded，116 s。
- **dev pretask 冷启动成功**：`LaunchCloudWeb` 以 `--inject` 拉起 CloudShell；启动任务从
  加载页/标题推进到 InWorld；同一队列中 DailyRewards succeeded；ClaimSimulationRewards 因
  菜单 home/last-tab 状态失败。

## 未验证 / 边界（不得当作已通过）

- 官网“开始游戏”按钮被透明层覆盖；控制器点击命中透明层，JS click 才能启动。
  这属于启动/pretask 缺口（U0）。
- 主菜单 home/last-tab 状态：Escape 打开上次标签（地图），行动手册/地区建设右侧按钮不在；
  需要 home 导航或固定 home 入口。
- Trial 战后结果页被跳过、角色回大世界且不在演算提示附近；重导航在 `GotoTrialArena` 失败，
  Trial 任务级仍未 Succeeded（U2）。
- 46 项任务仅 DailyRewards/ClaimSimulationRewards/Trial 有真实云动态证据；其余为静态资格/未跑（U3）。
- 最小化窗口的 PrintWindow 本次为空白；后台/遮挡/长时供帧仍未系统验证（U4）。
- `/screenshot` 预览在任务结束后可能返回上一帧缓存；CDP 页面是实时的。不要用预览缓存
  证明任务识别帧。
- MXU `resource/load` 返回后资源尚未 valid，立即 `start_tasks` 会报
  `Tasker not properly initialized`；测试编排需等待资源就绪（产品 UI 通常有间隔）。
- CloudShell/WebView2 的登录凭据、profile 与私有证据保留在授权工作区，不进入仓库。

## 工作方式

- 优先实现、减少重复测试；能用开发版/CI 产物就不重复构建；最终 release e2e 只做一次。
- 测试优先用 MaaEnd 与相关原生仓库的 CI/测试套件，并加更严格断言（资源 schema、
  Tasker 状态、真实入队/回执、失败隔离）。
- 公开写入（PR/评论/issue 自动回复）逐次确认；本文件与 issue 只记录可公开的进度。
- 每个增量提交到 fork 的栈分支；持续监控上游 #6807/#6841/#6800 与提交，更新本文件。
