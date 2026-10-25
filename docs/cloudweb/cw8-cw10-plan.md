# CloudWeb CW8–CW10 与验收准备计划（Perlica 独立推进）

状态：2026-10-24 18:30，`in_progress`（规划/准备；不代替 CW0–CW7/FIX-RS 实现）。
Owner：Perlica（coordinator）。本文件是内部执行计划，不等于 GitHub issue/PR 已创建。
单一源：`cases/cloudweb-roadmap.md` / `cloudweb-roadmap.json`；本文件展开非 CW0–CW7 的链路。

## 范围

本轮独立推进：

- **CW8 全任务资格**（E8/M8/G3）：46 项 Windows 业务 + LaunchGamePC 云端等价 + 必要 option/route/preset。
- **CW9 故障与长时稳定**（E9/M9/G4 前置）：ACK 丢失、半包、旧代次、媒体冻结、页面重建、Host/GUI 崩溃、人工接管、长运行。
- **CW10 打包、兼容与上游收口**（E10/M10/G4）：依赖、干净 Windows 安装、跨 Host、PR 切片、版本文档。
- **AC 验收准备**：CW0–CW7/FIX-RS 的独立验收清单、证据格式、Gate 评审与 validator 分工。

不包含：修改 CW0–CW7/FIX-RS 的实现；不替实施 Agent 写代码；不自动创建 GitHub issue/PR/评论。

## 固定分母

49 任务 / 426 option / 3 预设 / 1 pretask；Windows 46 项；CloudWeb 当前只开 DailyRewards。完整 G3 不能靠白名单缩小；Pass/Fail/BlockedFixture/NotApplicable/NotImplemented 可保留，但 NotImplemented/缺 fixture 不能关闭门槛。

## E8 / CW8 全任务资格

### 子 issue（认领）

| ID | 事项 | owner | 依赖 | 验收 |
| --- | --- | --- | --- | --- |
| CW8-1 | 在 `c7fd1b25` 重生成 task/option/preset/pretask inventory；核验 49/426/3/1 | Perlica | 无 | **verified**：`inventory-current.json` `bc0bb67d…`，49/426/3/2/1，46 Windows；基线 47 + `LevelUpOperator`/`UmbralMonument` |
| CW8-2 | 建立任务族矩阵与证据 schema（真实 actuator/target/profile、options/routes、结果、取消/恢复、证据） | Perlica | CW8-1 | **verified**：`task-matrix.json` `46994117…` / `task-matrix.md` `9ecaea44…`；46 行全 Pending；schema 含 10 项必需字段 |
| CW8-3 | 任务族适配清单：Daily/claim、AutoEssence、ProtocolSpace、DeliveryJobs/Seize、TrialOfSwordmancy、RealTimeTask（AutoSkip/Close/Fight/Pick/Teleport/Puzzle/AeroSalvage/Zipline/AutoPat、SklandMap/VideoBrowser）、AccountSwitch/Launch/Close、presets/settings、ZiplineImport | task-family agents | CW8-2；CW7 子集 | 每族有真实截图/ROI/跳转/游戏结果 |
| CW8-4 | option/route/preset/pretask 等价：DailyRewards 6 根/13 递归、三个 preset、Keymap/Zipline settings、LaunchGamePC 云端等价 | MaaEnd agents | CW8-3 | 每项实际运行；不能只静态映射 |
| CW8-5 | 46 项 Windows 业务逐项 Pass/Fail 与证据绑定；独立 validator 抽查 | Perlica + validator | CW8-3/4 | 46 项逐行有候选/证据/结果；未通过如实保留 |

### CW8-1 结果（verified）

- 生成器：`inventory.mjs`（JSONC 解析 `assets/interface.json` 的 71 个 import；不依赖旧 inventory）。
- 基线 `c7fd1b25`：**49 tasks / 426 option / 3 preset / 2 setting / 1 pretask**；`windows_applicable=46`；无重复 task/option。
- 与证据包旧基线（`5155c76c`，47 tasks）比较：新增 `LevelUpOperator`、`UmbralMonument`，无移除；`cloudweb_open_tasks` 仅 `DailyRewards`。
- 产物：`inventory-current.json` sha256 `bc0bb67d6b8b93e7efeb9b2c8e476ec519b912b613e048aff201428b5f514cc8`；连续两次运行 stdout/文件哈希一致（幂等）。

### CW8-2 结果（verified）

- 生成器 `matrix.mjs` 从 `inventory-current.json` 生成 46 行 Windows 任务矩阵；全部为 `pending`，没有预填通过。
- 任务族分布：open_world 9、other_menu 8、regional 5、dijiang 5、valuables 5、backpack 5、lifecycle 2、delivery_jobs 2、auto_essence 1、protocol_space 1、realtime_assist 1、trial_of_swordmancy 1、daily_claim 1。
- 证据 schema 字段：candidate SHA、runtime、actuator（kind/target/profile）、options/routes/presets、preconditions、steps、game_result、cancel/recover、evidence paths+sha256、unverified；状态枚举 pending/pass/fail/blocked_fixture/not_applicable/not_implemented。
- 独立列：1 个 pretask（LaunchGamePC 云端等价）、3 个 preset、2 个 setting。
- 产物：`task-matrix.json` sha256 `46994117006bbeeadf8eb134d216346d1b472fa2ade15ded56edaa922f3b0061`；`task-matrix.md` sha256 `9ecaea440542bc98213f00a09c4d10233204eea2e724560bbebd3375f49b8464`；连续两次生成哈希一致。

### 任务族矩阵（首批）

| 族 | 必测变体/备注 |
| --- | --- |
| Daily/claim | DailyRewards 上游仅启用 delivery jobs；其他子任务 `enabled:false`，不得算通过 |
| AutoEssence | 地点/战斗配置、导航路线；不能只测一个点 |
| ProtocolSpace | 关键战斗模式；不能只测入口 |
| DeliveryJobs/Seize | 导航与送货分支 |
| TrialOfSwordmancy | Daily/Coating/Farm25 分开 |
| RealTimeTask | AutoSkip/AutoClose/AutoFight/AutoPick/QuickTeleport/AutoPuzzle/AeroSalvage/AutoZipline/AutoPat；SklandMap/VideoBrowser 附属 |
| 生命周期 | AccountSwitch 云 profile/退出登录；CloseGamePC 云会话结束；LaunchGamePC pretask 云端等价 |
| 预设/settings | DailyFull/QuickDaily/RealtimeAssist；Keymap/Zipline |
| ZiplineImport | 独立 Firefox/profile 与云游戏 profile 生命周期分开 |

### 首批动作

1. **[done]** 用 `c7fd1b25` 的 `assets/tasks`/interface 重生成 inventory；结果 49/426/3/2/1、46 Windows，见上方 CW8-1 结果；旧证据 47 项仅作差异对照。
2. **[done]** 生成 46 行 Windows 任务矩阵空表（任务、族、必需 options/routes、证据字段、状态），见 CW8-2 结果。
3. 定义每族最小证据包：候选 SHA、真实 actuator/target/profile、截图/ROI/跳转、游戏结果、取消/恢复、未验证边界。
4. CW7 关闭后按族分配；CW7 未关闭前只做静态/资源准备，不宣称 G3。
5. 上游合并副产物：把任务族适配按 PR 切片记录，不自动开 PR。

## E9 / CW9 故障与长时稳定

### 子 issue（认领）

| ID | 事项 | owner | 依赖 | 验收 |
| --- | --- | --- | --- | --- |
| CW9-1 | 故障矩阵与证据 schema：ACK 丢失、半包、旧代次、媒体冻结、页面重建、Host/GUI 崩溃、人工接管、远端残留输入 | Perlica | CW2 合同 | **verified**：8 域 / 51 行全 Pending；每行含注入/预期/禁止/证据/owner/依赖/层级/gate；`fault-matrix.json` `ed335379…` / `.md` `2d9daa73…` |
| CW9-2 | 控制面故障：ACK 丢失/半包/seq 错乱/旧 generation；Stop 先 fence 再中和 | sidecar fault agent | CW2 | **partial**：Batch A 15 行首轮 6 pass / 4 partial / 3 fail / 2 blocked；CW2 minimal stop 已 pass，但 full contract 仍 partial；长 RPC 抢占、在途 deadline、pending/背压、Host ACK harness 未闭合 |
| CW9-3 | 媒体故障：rVFC 冻结、重复帧、A/B 覆盖、页面重建、document/geometry 变化 | media fault agent | CW4 | 冻结不标 Fresh；旧 token 失效 |
| CW9-4 | Host 故障：MXU 崩溃、GUI 崩溃、API 无响应、Host-death 回收、双实例 | MXU fault agent | CW2/CW5 | 只回收自己的进程；API 有界 |
| CW9-5 | 人工接管：pause→fence→release→owner 转移→重新识别 | integration agent | CW2/CW3 | 无残留 held；恢复不重放旧输入 |
| CW9-6 | 长运行：100 次 connect/start/cancel/disconnect、2 小时混合任务/预览/重连、资源趋势 | validator | CW9-2..5 | 有界停止；无泄漏/无跨 run 事件 |

### CW9-1 结果（verified）

- 生成器：`fault-matrix.mjs`（读取 `cloudweb-roadmap.json` 的基线；输出不含时间戳，连续两次运行哈希一致）。
- 规模：**8 域 / 51 行**；D1 9、D2 6、D3 6、D4 8、D5 6、D6 7、D7 3、D8 6。
- 关闭层级：`protocol_fixture` 15、`local_browser` 26、`real_cloud_session` 10；G4-pre 41、G4 10。
- 每行字段：注入方式、预期行为、禁止行为、证据字段、owner、依赖、关闭层级、gate、状态；全部 `pending`，未执行故障注入。
- 证据 schema 13 项：候选 SHA、runtime、注入方式、run/document/geometry 身份、owner/generation/ledger、时间线、状态/disposition、进程清单、帧/媒体进度/token、UI 诊断、exit code、日志+hash、未验证项。
- 判据：缺任一字段不得 Pass；fixture 不能关闭 `real_cloud_session` 行；fake/fixed PNG/fake SDK/独立 Node 脚本不计；候选变化后旧证据失效。
- 真实输入登记（根因均未闭合）：`run-175718` 连续 `state_poll_error`/API >60s 无响应（F-HS-03）；`run-172548` control pipe 关闭/exit 7（F-HS-04，实验已 revert，不归因当前候选）；`run-175512` supervisor 轮询误杀（F-HS-03）。
- 产物：`fault-matrix.json` sha256 `ed335379ee5873ca3a17710dc99dd95ad205e4ca754e64a022b9835c9830ab4d`；`fault-matrix.md` sha256 `2d9daa73dcb1524c4c94a0178ecc946c7ff7708d1a1d8ac231c3b7da489ebb57`。
- CW9-1 `verified` 仅指矩阵与 schema 已建立；不关闭 CW9-2..6，也不代表 G4。

### CW9-2 Batch A 首轮（partial）

- 范围：D1 9 行 + D2 6 行，全部 `protocol_fixture`；候选 sidecar `8b9cc6f` + diff `4c775f51…`。
- 执行：`cargo test -- --skip cdp_` → 40 passed / 0 failed / 3 filtered，exit 0；日志 `e4c9356d…`。
- 行级：**6 pass / 4 partial / 3 fail / 2 blocked**；结果 `results.json` `874588c4…`，报告 `report.md` `17326584…`。
- pass：F-CP-03、F-CP-05、F-LS-01、F-LS-03、F-LS-04、F-LS-05。
- partial：F-CP-04（SeqConflict 无测试）、F-CP-07（长按通过，慢 RPC 抢占未闭合）、F-LS-02（无 renew 丢失测试）、F-LS-06（token 半属 CW4）。
- fail：F-CP-06（仅拒绝 deadline=0）、F-CP-08（`max_pending` 无使用点）、F-CP-09（串行控制面无 fence-first 队列）。
- blocked：F-CP-01/02（ACK 丢失是 Host 事件，需要 MXU unknown 账本 harness）。
- **CW2 口径修正**：`cw7/final-audit.md` 已把 **minimal stop** 升为 pass（真实云 AutoFight 任务内 Stop），但长 RPC 逐命令类型、owned launcher、Host-death 仍未闭合；完整 CW2 仍为 `partial`。CW9-2 保持 `partial`。
- **Batch B 已准备**：26 行 local_browser；本地 headless Chrome smoke 1 passed、临时 profile/端口清理通过；就绪度 12 partial / 13 not_run / 1 blocked，0 pass。详见 `cw9/batch-b/plan.md` 与 `readiness.json`。
- Batch B 顺序：B2 媒体 → B3 CDP → B1 Stop → B5 跨 run/资源 → B4 Host 生命周期；HS-01/07 等 owned launcher/父死检测。
- 执行入口：`cw9/runbook.md` sha256 `e2b7eaf9…`；Batch B 计划 `7e1daf1d…`、就绪度 `7c382b83…`。

### 首批动作

1. **[done]** 建立 `fault-matrix.json` / `.md`，登记 `run-175718`/`run-172548`/`run-175512` 三个真实输入，不猜测根因。
2. Batch A 首轮已执行（6/4/3/2）；**暂停重跑**，先由 CW2 owner 收口长 RPC 抢占、在途 deadline、owned launcher、Host-death；Host ACK harness 由 MXU 侧补。
3. CW2 收口后重跑 Batch A，重点转 F-CP-01/02/06/07/08/09 与 F-LS-02/06；Batch B 的 B2/B3 可并行准备，不依赖长 RPC 抢占。
4. Batch B（26 行 local_browser）：先补冻结/重复帧/A-B/resize/reload 与 CDP 断链/崩溃/重建测试；HS-01/07 等 owned launcher/父死检测，未就绪前保持 pending。
5. Batch C（10 行 real_cloud_session）依赖 CW7；长运行最后。判据不变：“不可确认即 Faulted/未知账本，禁止新 run”；禁止自动重放。

## E10 / CW10 打包、兼容与上游收口

### 子 issue（认领）

| ID | 事项 | owner | 依赖 | 验收 |
| --- | --- | --- | --- | --- |
| CW10-1 | 依赖与许可：sidecar LICENSE/README/rust-version、MaaEnd/MXU 构建工具链、MaaFramework/runtime 版本 | Perlica | CW0 | 依赖表 + 许可/最低版本 + 可复现构建 |
| CW10-2 | 干净 Windows 安装/回滚：MXU + MaaEnd + sidecar + 专属 profile；不依赖本机残留 | release agent | CW0/CW1 | 干净机安装、启动、回滚步骤 |
| CW10-3 | 跨 Host 兼容：同 sidecar hash 接当前/旧 Host 族；协议/PI/schema 最低版本 | host agent | CW1/CW5 | 版本矩阵 + 明确 unsupported |
| CW10-4 | 上游 PR 切片：PR-A binding、PR-B/C/D、PR-F；每张只含一层职责 | Perlica + reviewer | CW1 合同接受 | 每张可独立审阅；不夹带私有实现 |
| CW10-5 | 文档/诊断/版本：错误码、状态 UI、诊断包、发布说明、支持任务清单 | release agent | CW8/CW9 | 文档与候选一致；不夸大 Gate |

### 上游副产物

- 先推 FIX-RS 独立 PR（见 `cases/cloudweb-fix-rs/handoff.md`）；再推 CustomController 能力探针、MXU 配置/工厂/pool、sidecar 协议/监督、SiteAdapter/真实证据、MaaEnd 适配/任务族。
- 上游接受是 B14，不阻塞私有分支；但正式发行声明必须等 G3/G4。
- 不创建/发送 GitHub issue/PR/评论，除非管理员逐次授权。

## AC 验收准备（CW0–CW7/FIX-RS）

### 角色

- **独立 validator**：按 checklist 逐项验收，不接受实施者自证；必须复跑/复读证据。
- **reviewer**：代码/协议层独立复核；与 validator 分离。
- **coordinator（Perlica）**：Gate 主持、候选绑定、未通过项保留、对外汇报。
- **delivery**：最终候选审计/打包；不替实施者补证据。

### 逐 CW 验收清单

| CW | 必须提交 | 不接受 |
| --- | --- | --- |
| CW0 | 三仓 SHA/remote/worktree、构建命令/产物 hash、实际配置、许可/工具链、真实/fake 表 | 只有源码审查、只有旧 manifest |
| CW1 | hello 实际 kind/profile/capabilities、Host 校验、生产显式 CDP、fake 显式入口、schema/factory | 默认 fake 冒充真实、只显示外层 adapter |
| CW2 | typed held∪maybe、独立 renew、run/document/geometry fence、owned launcher、Host-death 回收、长按/慢 RPC/unknown down 反例 | 只测 confirmed、只测正常 Stop |
| CW3 | 真实 relative、contact 映射、modifier/repeat/wheel/drag/text、SDK readiness、人工接管、真实游戏持续组合 | 假 SDK、单次点击、只 DOM 日志 |
| CW4 | rVFC/媒体进度、`MEDIA_STALLED`、720p、document/geometry、snapshot token、帧去重/slot、像素/metadata 原子 | 截图计数冒充媒体进度、冻结画面标 Fresh |
| CW5 | 唯一 frame reader/cache、preview/识别分离、pool/run/task ID、锁外 shutdown、API 响应性、双实例 | 预览改写识别观察、API 无响应无诊断 |
| CW6 | 通用 AutoAltClick 恢复、薄 overlay、legacy camera、region/UID、能力准入、云生命周期 | 全局 Escape、只加白名单、未改 schema |
| CW7 | 真实 actuator/target/profile、冷启动→登录/排队→DailyRewards→Stop/重开、MapNavigator/AutoFight、后台隔离、邮件有/无 | 从 Regional 界面启动的一次成功、fake/固定 PNG、无 Stop/重开 |
| FIX-RS | last-owner、record/Tasker retention、destroy quiescence、callback 恰好一次且不早于 native 使用结束、最新 upstream、独立 PR | 只有 100 次 Drop 测试、未验证 destroy 并发、混入 CloudWeb |

### Gate 评审

- G0：CW0/CW1 checklist 全绿 → 允许 CW2/CW4 开工。
- G1：CW2 最小停止 + CW3 真实输入 + CW4 媒体/隔离 → 允许 CW7 纵向。
- G2：CW7 实际 MaaEnd UI 闭环 + 独立 validator → 允许 CW8 全任务。
- G3：CW8 46 项 + pretask/options/presets → 允许 G4。
- G4：CW9/CW10 + FIX-RS 回正式依赖 + 干净安装/回滚 → 发布。

### 首批动作

1. 建 `cases/cloudweb-acceptance/` checklist 与证据模板；每 CW 一张表。
2. 指定独立 validator/reviewer；与实施 Agent 分离。
3. 先做 CW0/CW1 的静态验收（不依赖云会话），把缺口列清。
4. FIX-RS 交 native reviewer；CW7 交 integration+validator。
5. 每次 Gate 评审只输出“verified/partial/blocked”和未通过项，不用一次成功关闭 Gate。

## 并行与依赖

- 现在可做：CW8-1/2 静态 inventory/矩阵（done）、CW9-1 故障矩阵（done）、CW10-1/4 依赖/PR 映射、AC 清单。
- CW2 minimal stop 已 pass；Batch A 首轮 6/4/3/2，重跑等待长 RPC 抢占/在途 deadline；owned launcher/Host-death 影响 Batch B 的 HS-01/07。
- Batch B 已准备：本地 headless Chrome smoke 通过；26 行就绪度 12 partial / 13 not_run / 1 blocked，B2/B3 可先执行。
- CW7 关闭后：CW8-3/4/5、CW9-5/CW9-6、Batch C（10 行 real_cloud_session）。
- CW10-2/3 依赖 CW0/CW1/CW5；CW10-4 依赖 CW1 合同接受。

## 停线

- 不替 CW0–CW7/FIX-RS 实施者改代码或补证据。
- 不把静态/规划/一次成功当 Gate 通过。
- 不自动创建 GitHub 外部记录；不泄露 UID/token/cookie/本机路径。
- 同一失败两次停线诊断；不盲重放。
