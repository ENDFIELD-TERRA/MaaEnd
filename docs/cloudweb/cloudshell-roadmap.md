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
| U0 启动任务 | verified（严格 InWorld） | 标题页、今日月卡奖励、进入世界（`CloudWebStartGame`） | [#4](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/4) |
| U0 凭据 | verified | WebView2 会话 DPAPI 保存/恢复 harness；真实过期会话恢复 4 cookies / 22 localStorage；秘密不进入仓库/日志 | [#2](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/2) |
| U1 队列 | verified（严格 3/3，61 s） | 同一控制器顺序运行多个任务，失败隔离、状态可观测、Stop 可中断 | [#6](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/6) |
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
- **冷启动全自动队列 Succeeded（2026-10-25 15:50–15:52，122 s）**：pretask `LaunchCloudWeb`
  以 `--inject` 拉起全新 CloudShell，脚本自动点开始（最多 2 次），管线处理加载/标题/月卡 →
  InWorld → `DailyRewards` → `ClaimSimulationRewards`，三项 succeeded，无人工点击。
- **严格 3 任务队列 Succeeded（2026-10-25 15:43，61 s）**：`CloudWebStartGame`
  （仅在 InWorld 结束）→ `DailyRewards` → `ClaimSimulationRewards` 三项全部 succeeded。
- **DailyRewards 全链路**：世界 → 菜单 → 地区建设 → 换区 → 仓库 → 我的转移页 → 领取 → 返回，
  是真实点击驱动的多级菜单闭环，不是空转。
- **点击失效根因与修复**：注入脚本「禁用悬浮球」沿祖先链把承载 `#game-video` 的全屏层
  `.ob90eP` 设为 `pointer-events:none`，标题页与游戏内所有鼠标事件都被掐断；修复后只禁用
  左上角 ≤180×180 的 fixed/absolute 容器。修复后控制器单击与 Esc 均可驱动游戏
  （标题推进、菜单打开、角色攻击）。启始按钮同时改回按 OCR 框点击。
- **启动卡住根因与修复**：「游戏启动中/请耐心等待」在视频流里，页面 DOM 读不到；脚本每 5 s
  重复 DOM click「开始游戏」，会把供给中的云会话反复打回起点。改为最多点 2 次、间隔 20 s，
  并让启动等待识别「游戏已重连/正在连接」后，冷启动约 45 s 进入游戏加载页。
- **DPAPI 恢复 harness 实机通过**：在会话过期后的手机号登录页恢复 4 cookies / 22 localStorage，
  reload 后 `通行证UID` 校验通过，临时明文删除。
- **上游确认**：#6841（OPEN/Draft，head `d54392de`）维护者 17:32 UTC 评论确认
  CloudShell 可直接 PostMessage，与本地实测一致；#6800 建议直接 Stack 到 #6841。
- 历史：首轮手动 3/3（116 s）发生在严格启动门控前，仅作历史记录。

## 未验证 / 边界（不得当作已通过）

- 任务广度仍不足：目前真实云动态覆盖启动、DailyRewards、ClaimSimulationRewards；VisitFriends/
  DijiangRewards/EnvironmentMonitoring 等进行中，长任务与 46 项矩阵尚未验证。
- Escape 打开的是上次标签（地图），但右侧行动手册/地区建设按钮实际存在，菜单导航已通过；
  “home 缺失”原判断由鼠标输入被掐断造成，已撤销。
- 队列结束可能停在模拟空间/行动手册（ClaimSimulationRewards 的 `SimCheckEmpty` 终止路径）；
  下一任务能否从该菜单态恢复尚未验证。启动任务只负责冷启动，不负责暖会话恢复。
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
