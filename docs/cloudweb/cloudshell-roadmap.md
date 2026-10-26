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
| U3 全量矩阵 | partial | 最终构建包 46/46 静态资格、0 override 缺失；动态 10 任务 + 长时证据；导航阻塞项单列 | [#7](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/7) |
| U4 后台/长时 | partial | 非前台长时可运行（ReadAllWiki 19+ min 持续交互，前台非 shell）；最小化 PrintWindow 仍空白 | — |
| U5 栈与验收 | verified（构建产物 5 任务；导航/矩阵残余） | 基于 upstream/pr-6841 的栈分支、install CI、构建产物冷启动 e2e、fork 同步与上游监控 | [#3](https://github.com/ENDFIELD-TERRA/MaaEnd/issues/3) |

## 已验证（2026-10-25）

- CloudShell 作为 WebView2/Chromium 宿主，窗口类 `MaaEnd.CloudShell`、固定 1280×720；
  启动日志 `foreground=false`，不抢前台。
- Win32 控制器（PrintWindow + PostMessage）已在真实云会话驱动标题页、月卡、进入大世界；
  CloudShell 登录在独立 WebView2 profile 中持久化。
- #6841 interface/resource + CloudWeb overlay 下，`DailyRewards` 在 CloudShell 上 **Succeeded**
  （真实云，taskId 级成功）。
- Trial 的 `Fight` / `AutoFight` / `FightFinish` 节点级 **Succeeded**（有真实演算次数）。
- 非前台/被遮挡的正常窗口 PrintWindow 可抓完整页面；任务期间识别帧正常。
- **长时后台交互（2026-10-26 01:15）**：最终构建包上 `ReadAllWiki` 在非前台窗口连续
  19+ 分钟点击/滚动未读百科页，前台窗口始终不是 CloudShell；仅因编排器 20 分钟上限停止，
  任务仍在 Running。最小化窗口 PrintWindow 仍为空白的边界不变。
- **安全 3 任务队列 Succeeded（2026-10-26 01:22–01:24，96 s）**：
  `ReceiveProdManual` → `SwitchTeam` → `StashBackpack`，三项全部 succeeded（无资源消耗）。
  最终构建包动态成功任务增至 8 项。
- **AutoEcoFarm / ResourceRecycle（未计入）**：默认 options 下分别只跑 Loop→Finish / Start→End；
  显式展开 option 后，AutoEcoFarm 三个 Job 全部在 Execute 失败，ResourceRecycle 在 Map/Overview
  入口失败。两者均不计入动态证据，最终动态成功为 10 项。
- **IntelArchive 长任务 Succeeded（2026-10-26 02:01–02:12，约 11 min）**：
  最终构建包冷启动后读取档案完成；旧会话 8 分钟预算内未完成，本次 30 分钟预算通过。
- **`check` CI 通过（2026-10-26 02:07）**：基座增加 `CloudWebStartGame` 占位入口（17e3ecaf），
  解决 maa-checker 在非 CloudWeb 控制器下报未知入口；check run 40257399337 success。
- **46 项静态资格（最终构建包）**：46/46 入口节点存在，0 个 option override 节点缺失，
  节点集合 6029；CloudWeb 可用性 44 项显式声明 + `AutoEssence` 无 controller 字段（全控制器），
  仅 `CloseGamePC` 为 PC 专属。动态证据仍为 5 任务 + `ReadAllWiki` 长时/`IntelArchive` 部分；
  Trial/EnvironmentMonitoring 受导航末段阻塞。
- **最终 10 任务冷启动队列连续两次 Succeeded（2026-10-26 03:23–03:39 / 03:40–03:59）**：
  重建包上同一队列两次全部 succeeded（982 s / 约 18 min，均含 IntelArchive 长任务）；
  StartGame → Daily → Claim → VisitFriends → DijiangRewards → ReceiveProdManual → SwitchTeam →
  StashBackpack → PullCountCalculator → IntelArchive。可重复性成立。
- **重建产物冷启动 5 任务 Succeeded（2026-10-26 02:35–02:37，167 s，stack head 17e3ecaf）**：
  新包含基座 `CloudWebStartGame` 占位，CloudWeb 附加资源覆盖为真实状态机；启动约 60 s，
  非占位秒过。install run 40258057525 与 check run 40257399337 同 head，均为绿。
- **构建产物冷启动 5 任务 Succeeded（2026-10-26 00:25–00:29，176 s，历史 run 40247102256）**：
  fork `install` workflow run 40247102256（ref `feat/cloudweb-6841-stack`）的
  `MaaEnd-win-x86_64-zip` 独立解包；统一 `go-service.exe` 以 pretask `--inject` 启动新外壳，
  登录 profile 复制后冷启动 `CloudWebStartGame → DailyRewards → ClaimSimulationRewards →
  VisitFriends → DijiangRewards`，五项全部 succeeded。构建产物 + 产品 pretask + 队列闭环。
- **冷启动最终 5 任务严格队列 Succeeded（2026-10-25 16:45–16:47，141 s）**：
  pretask `--inject` 拉起全新 CloudShell → `CloudWebStartGame`（InWorld 门控）→
  `DailyRewards` → `ClaimSimulationRewards` → `VisitFriends` → `DijiangRewards`
  （`SelectToGrow=DoNothing`），五项全部 succeeded，无人工点击。
- 暖会话 5 任务 100.9 s、4 任务 85.8 s 同样通过；跨任务场景恢复增加稳定帧等待后，
  `Claim` 紧跟 `Daily` 的帝江号菜单转场失败消失。
- **最终 4 任务严格队列 Succeeded（2026-10-25 16:22–16:23，81 s）**：
  `CloudWebStartGame`（InWorld 门控）→ `DailyRewards` → `ClaimSimulationRewards` → `VisitFriends`，
  四项全部 succeeded；使用隐藏浮球后的最终注入脚本。
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
- **任务广度**：`VisitFriends` 在冷启动会话 Succeeded（帝江号加载、拜访）；
  `EnvironmentMonitoring` 自行导航到多个野外监控终端并接受任务，但各任务执行在
  `GoTo*Execute` 的 MapNavigator 移动节点失败（天师柱/落日/虫类/水竹铃/涂鸦/彩虹），
  FailureCollector 汇总 Failed。
- **浮球遮挡小地图（关键修复）**：CloudShell 浮球位于左上角、压住小地图；仅设
  `pointer-events:none` 仍会进入 PrintWindow 帧，MapLocator 的 camera heading prior 恒为 -1。
  改为 `display:none` 后，MapNavigator 获得有效航向，Trial 连续导航 84 s / 581 tick，
  到达距目标 1.56 m，最终因末段 `Dynamic recovery timeout`（yaw_error≈-101°）失败——
  从「立即放弃」推进到末段控制问题。
- **场景恢复**：DijiangRewards 因培养舱 `GrowthChamberGrowViewIn` 目标未识别失败并停在
  帝江号总控中枢；后续任务原先只在 `InMenuList` 时按 Escape，故连续失败。实测该界面
  PostMessage Escape 可退回帝江号大世界，已将 EnterWorld 识别扩展为
  `Or(InMenuList, InDijiangControlNexus)`。

## 未验证 / 边界（不得当作已通过）

- 任务广度：真实云动态已覆盖启动、DailyRewards、ClaimSimulationRewards、VisitFriends、
  DijiangRewards（`SelectToGrow=DoNothing`）；EnvironmentMonitoring 终端导航通过但任务点移动失败；
  Trial 到达末段 1.56 m 后 MapNavigator 动态恢复超时；46 项矩阵未全量验证。
- MapNavigator 末段朝向/动态恢复是当前主要剩余阻塞（Trial 与野外任务点），
  与 CloudWeb 打印帧/输入架构无关；navmesh `deck_probe` 已确认 Trial 终点只有单一可走面
  （高 221.57，`on_surface:false`），不存在重叠 deck，`target_deck_y` 不适用；
  `route` 查询 start→goal 为 reachable / same_component / 1.62 m，但运行在起点 1.56 m 处
  `steer_heading` 等于当前朝向、未转向，dynamic recovery 超时。已试作废 stale pending
  （清空后重发）并在 CI 构建包复跑：pending 能清，但 4.8 s 内航向仍无变化（moved=0），
  仍在 3.72 m 超时；修复已回退。根因是末段停下后无法转向/楔死，不是 pending 记账。
  需要单独处理 MapNavigator 静止转向与脱困，不是 CloudWeb 或路线不可达。
  另试 `heading_source:"camera"`（参数已传入）仍在 1.95 m / yaw 157° 失败；
  最后用 `TrialOfSwordmancyQuickTeleportStart` 强制走完传送链（起点受控在悬谷台），
  prevpoint 终点仍在 2.32 m / yaw -140° 失败；再改 async INTERACT 终点，
  2–3 m 处提示模板 0.588、OCR 未命中。起点、终点、提示三种假设均排除，
  末段是控制/楔死；导航盲试结束，需受控录制绕行或控制层修复。
- 上游 #6854（open）指出转场后约 750 ms 的输入忽略窗口；CloudWeb 上紧接转场的点击也需
  预留 pre_delay，后续任务适配时沿用。
- Escape 打开的是上次标签（地图），但右侧行动手册/地区建设按钮实际存在，菜单导航已通过；
  “home 缺失”原判断由鼠标输入被掐断造成，已撤销。
- 队列结束可能停在模拟空间/行动手册（ClaimSimulationRewards 的 `SimCheckEmpty` 终止路径）；
  下一任务能否从该菜单态恢复尚未验证。启动任务只负责冷启动，不负责暖会话恢复。
- Trial 战后结果页被跳过、角色回大世界且不在演算提示附近；重导航在 `GotoTrialArena` 失败，
  Trial 任务级仍未 Succeeded（U2）。
- 46 项任务中已有 10 项动态成功（启动、Daily、Claim、VisitFriends、DijiangRewards、ReceiveProdManual、SwitchTeam、StashBackpack、PullCountCalculator、IntelArchive），ReadAllWiki 长时/部分，Trial/环境监测导航阻塞；AutoEcoFarm/ResourceRecycle 因 option 未展开而空转、未计入；BakerEntry 对话循环、AeroSalvage 入口前置失败均未计入。其余为静态资格/未跑（U3）。
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

## 2026-10-26 转向改为非侵入 look 通道；完整 Trial 通过

- 背景：后台 SendInput 相对移动到不了 WebView2，MouseLockFollow 还会接管用户系统光标与窗口。
  两条路径都不可接受，实测也无法转动镜头。
- 方案（提交 `68609b5a`，后续 `98e39508` 修正非 Windows 编译）：CloudShell 新增 look 消息，
  由页面派发一次带位移的 mousemove 后零位移回位；cpp-algo 识别 CloudShell 窗口后改走该消息，
  不再启用 MouseLockFollow。系统光标、窗口位置与前台窗口均不改变。
- 证据：8 点 authored route 35 s 走通（camRot 90°→29°）；完整 Trial（启动→传送→导航→触发→
  战斗→回世界）2m21s 成功；页面收到 173 次带位移 trusted mousemove。
- 仍开放：EnvironmentMonitoring 路线中的 `localization_lost_timeout`（cold-start collecting），
  46 项动态全量，最小化窗口 PrintWindow，正式 release 标签；最终证据绑定 98e39508 的 CI 包。

## 2026-10-26 最终 CI 包验收（4321b05c）

- install run 40272085096 全绿（win/mac/linux 构建与打包）；最终包 `MaaEnd-win-x86_64.zip`，
  版本 `v0.0.0-ci.261026-4321b05c`。包内 `cloud-shell.exe` 与 `cpp-algo.exe` 与 CI 工件逐字节一致。
- 同一最终包：10 任务队列全成功；独立 Trial（StartGame 复位后）全链路成功。look 转向通道
  在最终产物上生效，全程未接管系统光标/窗口/焦点。
- 同一队列内 Trial 紧接 IntelArchive 时在 `QuickTeleportStart` 的地区总览入口失败；
  属起始菜单态/队列编排问题，不是导航。EnvironmentMonitoring 仍有 `localization_lost_timeout`。
- 未闭合：上述队列转场、Env 定位丢失、46 项动态全量、最小化 PrintWindow、正式 release 标签。

## 2026-10-26 07:00 最终包 63f34238；Env 中段定位丢失仍开放

- 最终包 `v0.0.0-ci.261026-63f34238`（install run 40279072318）：
  `cloud-shell` 652e7c98…、`cpp-algo` 58c8d83a…，与 CI 工件逐字节一致。
- 已验证：10 任务队列（4321b05c 包）全成功；独立 Trial 全链路；好友界面 → 快速传送
  （ed26600b 前置复位）成功；导航首次取位失败 → 场景复位（63f34238）在好友界面下 46s
  导航成功，正常大世界 20s 不触发复位。
- EnvironmentMonitoring：起始好友界面已修复；中段拍摄区域仍出现 YOLO `None` 定位丢失，
  中段盲复位实验每 ~13s 触发且不恢复，已回退。后续需抓实际丢失帧判断过渡/拍摄遮挡。
- 未闭合：Env 中段、46 项动态全量、最小化 PrintWindow、正式 release 标签。

## 2026-10-26 07:35 Env 中段取证：脱离卡死弹窗/拍照模式；导航层实验回退

- 实际丢失帧显示游戏“检测到无法操控，是否脱离卡死”确认框；另一次复跑为拍摄区域无小地图
  （OCR 读到设置文字）。两者都会让 MapLocator 得到 YOLO `None`。
- 导航层三种恢复（中段 SceneAnyEnterWorld、OCR“取消”、OCR 标题+Esc）均因弹窗/画面时序不同
  而无效，已全部回退；最终包 final5 恢复为 CI 工件（`652e7c98…` / `58c8d83a…`），
  stack 保持 `63f34238`。
- Env 下一步应在任务流程层修：拍照/画面设置结束先退出再启动导航；对“脱离卡死”弹窗
  做任务级处理（取消或脱离卡死后重新定位）。已验证的 Trial、队列与起始复位结论不变。
- 未闭合：Env、46 项动态全量、最小化 PrintWindow、正式 release 标签。

## 2026-10-26 08:18 final6（868e9af6）：Esc 模态恢复入包；Trial 转场待干净态重验

- install run 40288912687 全绿；final6 包 shell `f209cc24…`、cpp-algo `13f60e79…`，
  含 Trial 转场前置复位、Env 终端前置 SceneAnyEnterWorld、CloudWeb 定位丢失 Esc 重试。
- Env 的“脱离卡死/黑屏”模态丢失实测发送 Esc 后 ~2.7s 恢复（3 次）；Env 仍剩
  BambooWaterChimes 终点 2.195m 的 `dwell_watchdog`（地形/落点，非模态）。
- final6 上 Trial 转场探针（冷启动与热态）都停在地区总览菜单链；SceneAnyEnterWorld/InWorld
  已成功，该路径未改动，final5 同时点曾成功。判断为长期 Env/拍照运行后的游戏菜单/任务态，
  需干净会话重验，不能据此宣称回归或完成。
- 未闭合：干净态 Trial 转场重验、Env 终点、46 项动态全量、最小化 PrintWindow、release 标签。
