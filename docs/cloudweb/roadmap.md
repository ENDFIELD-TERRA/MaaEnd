# CloudWeb 单一源 roadmap（CW0–CW10）

状态审计：2026-10-24 23:20（Asia/Shanghai）
单一源：本文 + `cloudweb-roadmap.json`。详细合同与首批任务见 `cases/cloudweb-cw0-cw7/plan.md`。
材料权威：`MaaEnd-CloudWeb-Implementation-Review-2026-10-24.md` → `FeatureRequirementDoc-MaaEnd-CloudWeb-Neo.md` v1.1 §0 → `MaaEnd-CloudWeb-Coordinator-Report.md` §0。旧 P00–P17 与 CW-xx 编号只作历史映射，不再是进度源。

## 固定基线

| 组件 | SHA | 本地/远端 |
| --- | --- | --- |
| sidecar | `8b9cc6f670fc59652ffb7f50e312d1949e7f73ac` | `main` = fork/main |
| MXU | `8bddfee34bba3ea8ec81e3b8fdcb118375329b76` | `feat/cloudweb-host` = fork |
| MaaEnd | `c7fd1b25071d9976703635b7dda66812172d1629` | `feat/cloudweb-cw28` = origin |
| MaaEnd v2 | `051d17f2527e2e990952d6655f1f380d5d7de81e` | 上游基线 |
| qoder-agentset | `e3dab67d997840c5d8ae4cf0328b2b36f308368c` | 证据/AgentSet 参考 |
| maa-framework-rs patched | `953ec242d2542ab907211a4aa347b9a855f698aa` | FIX-RS 参考 |
| maa-framework-rs unfixed | `c1c617330b56205798609c7566e602b55adba74b` | FIX-RS 基线 |

codegraph 1.6.2 已建索引：MaaEnd 1,412 文件/30,994 节点；sidecar 8/572；MXU 161/4,798；AgentSet 194/4,406；maa-framework-rs 38/2,059（unfixed 37/2,052）。

## 产品分母

49 任务 / 426 option / 3 预设 / 1 pretask；Windows 适用 46 项；CloudWeb 当前只开放 `DailyRewards`。完整门槛 = 46 项 Windows 业务 + LaunchGamePC 云端等价 + 必要选项/路线/预设。白名单不是分母；阶段性 Fail/NotImplemented 可保留，但不能关闭 G3。

## 状态级别

`verified` / `partial` / `unverified` / `blocked` / `needs_review` / `not_started`。只有 `verified` 可进入完成汇报。

## Epic / 里程碑 / Gate

| Epic | 里程碑 | 状态 | Gate |
| --- | --- | --- | --- |
| E0 候选、合同与可复现基线（CW0） | M0 | `partial` | G0 |
| E1 最小公共接线与真实 provider（CW1） | M1 | `partial` | G0 |
| E2 Session/run、续租与最小停止（CW2） | M2 | `partial` | G1 前置 |
| E3 完整 PC 输入（CW3） | M3 | `partial` | G1 |
| E4 媒体、几何与 observation token（CW4） | M4 | `partial` | G1 |
| E5 Host 生命周期与预览（CW5） | M5 | `partial` | G2 前置 |
| E6 MaaEnd 云身份、薄 overlay 与任务适配（CW6） | M6 | `partial` | G2 前置 |
| E7 第一条真实纵向闭环（CW7） | M7 | `partial` | G2 |
| EF FIX-RS 独立 binding 修复 | FIX-RS | `partial` | 正式发布依赖 |
| E8 全任务资格（CW8） | M8 | `in_progress`（Perlica 认领，规划/矩阵） | G3 |
| E9 故障与长时稳定（CW9） | M9 | `in_progress`（Perlica 认领，harness 设计） | G4 前置 |
| E10 打包、兼容与上游收口（CW10） | M10 | `in_progress`（Perlica 认领，依赖/PR 映射） | G4 |
| AC CW0–CW7/FIX-RS 验收准备 | AC | `in_progress`（Perlica 认领） | G0–G2 评审 |

CW8 全任务资格（G3）、CW9 故障/长时、CW10 打包/上游收口（G4）保留主报告定义；本轮先闭环 CW0–CW7，不缩小最终分母。

## CW0–CW7 状态

| ID | 事项 | 状态 | 依赖 | 当前证据/缺口 |
| --- | --- | --- | --- | --- |
| CW0 | 候选与契约盘点、可复现构建、实际配置、许可/工具链 | `partial` | 无 | 三仓 SHA 已固定；`cw0/baseline-inventory.md` 记录 sidecar 基线重建 `8e9d…` 与 dist 逐字节一致、实际 cdp 配置、真实/fake 判定表、5 条 exit 0 证据审计；sidecar LICENSE `AGPL-3.0-only`（`8486a10c…`）；MSRV 实测 1.88（1.85 被依赖/let-chains 拒绝）；MXU 干净 release 构建（`f2fef2dd…`）与 MaaEnd Go/C++ 构建记录已补；缺口：MXU dist profile、MaaEnd Go 1.25.6、C++ 从零构建、MANIFEST 重绑 |
| CW1 | provider/launcher、最低版本、外层 adapter 与实际 actuator 分离、生产显式 CDP | `partial`（本地实现完成，最终审计并入 CW7） | CW0 | 候选 `adaa5905…`：sidecar 缺 backend 即失败；`hello_ok` minor 1 返回 `backend_kind`/`ui_profile`；MXU 校验 readiness+hello 并拒绝未授权 fake；sidecar 36/36 非 CDP 测试、clippy 0；MXU MSVC 42/44（1 环境失败、1 ignored 真实进程测试单独通过）。按新流程不设 CW1 独立复核门禁；真实 CDP 会话、UI 徽标、旧证据重生成并入 CW7 完整审计 |
| CW2 | run/document epoch、独立 renew、typed held∪maybe、真实 deadline、owned launcher、Host-death 回收 | `verified`（协议层，pass-with-boundary） | CW0/1 | 候选 `6227c5a8…`（sidecar `1f8625d5…` / MXU `e193e0d4…`）：urgent 独立车道 + `preempt_flag`，真实云 60000 px 移动中 ReleaseAll RTT **8 ms**；在途/排队 deadline Unknown 不重放；`max_pending` 有界 + Backpressure；attach-only 所有权；job object 强杀 143/158 ms 退出、无孤儿；真实云协议 Stop closed。边界：job kill 瞬间 held 输入释放 best-effort。`cw2/clause-table.md`、`cw9/cw2-reevaluation.md` |
| CW3 | 真实 relative、contact 映射、modifier/repeat、wheel/drag/text、SDK readiness、人工接管 | `partial`（本地输入语义接通） | CW2 | 候选 `574f4b0f…`：页面 pending delta + trusted anchor mousemove 的 relative 事务，127 分段，`input.relative` 已宣告；headless CDP 测试 3 段 trusted 事件、和 200/-300、buttons=2、shift=true；contact1→wire2、contact2→wire1、contact3 拒绝；sidecar 40/40 非 CDP、clippy 0；MXU 43/45（1 环境失败、1 ignored 真进程 2s+contacts 通过）。未闭合：真实云游戏持续组合/中键锁敌、SDK 正向 readiness、人工接管、Windows 隔离 |
| CW4 | rVFC/媒体进度、720p 规范化、document/geometry、snapshot token、帧去重/slot | `partial`（本地媒体语义接通） | CW0/1；真实接线依赖 CW2 | 候选 `6d7393bb…`：rVFC Start/Stop 幂等、1280×720 letterbox、doc_nonce/geometry_revision、原子 image+metadata slot；headless CDP 测试 rvfc probe、canonical JPEG、geometry>=1、stop 两次通过；session 的 static/frozen/dedupe 由 ScriptedFrameSource 覆盖。未闭合：sidecar↔CDP 全链路、真实云视频、真实静态/冻结、snapshot token、Host 帧去重/slot/receipt、性能 |
| CW5 | 唯一 frame reader/cache、preview/识别分离、pool/run/task ID、锁外 shutdown | `partial`（最小 reader/cache 已接通） | CW2/4 | 候选 `8e1b0e60…`：Host capture gate + immutable 100ms snapshot cache，TTL 内第二次 screencap 不重复发命令；`host_frame_cache` 诊断暴露 frame_id/geometry/hits；真实进程测试缓存命中、2 秒长按与 contacts 仍通过。未闭合：preview/识别 token 分离、pool/run/task ID/append/rollback、锁外 shutdown、`/api/maa/state` 响应性、双实例、真实 CDP 预览 |
| CW6 | 恢复通用 modifier+target click、薄 overlay、legacy camera、region/UID、能力准入、云生命周期 | `partial`（确定语义修复已接通） | CW1 后可先改；定稿依赖 CW3/4 | 候选 `4b7580d8…`：全局 `AutoAltClick→Escape` 与 `__AutoEscClickAction` 已删除，恢复通用 modifier+target click；新增 `resource_cloud_web` 薄 overlay（菜单开/关、InMenuList OCR 兜底、Regional Development 入口）；CloudWeb `attach_resource_path`、interface schema/validator 同步；Go build/test、schema validation、build-and-install 通过。未闭合：真实云菜单/镜头回归、region/UID cache、能力准入运行门、云生命周期、`__SceneImageCheck` 实测 |
| CW7 | 真实 actuator/目标身份；连接→登录/排队→DailyRewards→Stop/重开；MapNavigator+AutoFight；后台隔离 | `partial`（真实 DailyRewards + 进程级 Stop/重开成功） | CW3/4/5 + CW6 子集 | 候选 `9ce6f578…`：产品 overlay、无 runtime overrides，`094713` 与 `094836` 两次 DailyRewards exit 0（ready 页面；第二次为 MXU/sidecar 重建后的重开）；`094537` 暴露 `__SceneImageCheck` 云不兼容，已在 overlay 禁用；`094408` 截图暴露标题按钮位置，`game-enter` 已修正。未闭合：真正标题/新标签冷启动（新标签丢登录态）、MapNavigator/AutoFight、前台/焦点/输入隔离、in-session Stop |
| FIX-RS | last-owner 回收、record/Tasker retention、destroy quiescence；移植最新 upstream | `partial` | 可立即开始 | 最新候选 `2fb895a`（base `a73f330`，diff `6188ae46…`）：Windows MSVC 8/8 ownership 测试通过；unfixed `c1c6173` 6/8 失败（Drop=0）；native destroy 路径源码核对为 ControllerAgent 析构 join action thread；record/Tasker retention、并发 destroy、static-link、Linux/macOS 与独立 review 未闭合 |

## 非 CW0–CW7 链路（Perlica 认领，2026-10-24）

详细计划：`cases/cloudweb-cw8-cw10/plan.md`；验收准备：`cases/cloudweb-acceptance/plan.md`。

| Issue/Epic | 事项 | owner | 状态 |
| --- | --- | --- | --- |
| E8 / CW8 | 46 项 Windows 业务 + 云 pretask 等价 + 必要 option/route/preset | Perlica | `in_progress`（规划/矩阵） |
| CW8-1 | 在 `c7fd1b25` 重生成 task/option/preset/pretask inventory，核验 49/426/3/1 | Perlica | `verified`（49/426/3/2/1，46 Windows；`inventory-current.json` `bc0bb67d…`；旧基线 47 + LevelUpOperator/UmbralMonument） |
| CW8-2 | 任务族矩阵与证据 schema | Perlica | `verified`（46 行全 Pending；`task-matrix.json` `46994117…` / `.md` `9ecaea44…`） |
| CW8-3 | 任务族适配：Daily、AutoEssence、ProtocolSpace、DeliveryJobs/Seize、TrialOfSwordmancy、RealTimeTask、生命周期、预设/settings、ZiplineImport | task-family agents（待 CW7） | `pending` |
| CW8-4 | option/route/preset/pretask 实际运行等价 | MaaEnd agents（待 CW7） | `pending` |
| CW8-5 | 46 项逐项 Pass/Fail + 独立抽查 | Perlica + validator | `pending` |
| E9 / CW9 | 故障与长时：ACK 丢失、半包、旧代次、媒体冻结、页面重建、Host/GUI 崩溃、人工接管、长运行 | Perlica | `in_progress`（CW9-1 完成；CW9-2 round2 10/5/0/0；Batch B round2 4 tests pass，仍 0 row pass） |
| CW9-1 | 故障矩阵与证据 schema：8 域、注入/预期/禁止/证据/owner/依赖/层级/gate | Perlica | `verified`（51 行全 Pending；protocol_fixture 15、local_browser 26、real_cloud_session 10；`fault-matrix.json` `ed335379…` / `.md` `2d9daa73…`；不关闭 CW9-2..6） |
| CW9-2 | 控制面/租约协议故障 Batch A（D1/D2，15 行） | sidecar fault agent | `partial`（round2：10 pass / 5 partial / 0 fail / 0 blocked；urgent/deadline/backpressure/queue 已闭；剩 Host ACK、SeqConflict、renew-loss、token） |
| CW9-3/4/5/6 | Batch B round2：媒体、CDP、Stop、Host 生命周期、跨 run | Perlica | `in_progress`（26 行：14 partial / 11 not_run / 1 blocked，0 pass；4 个 CDP 测试在新候选上 4 passed；HS-01/07 边界仍在） |
| CW9-4 前置登记 | 把 `run-175718` 的 `state_poll_error` 登记为 Host/API 无响应真实输入 | Perlica | `verified`（已登记为 F-HS-03；同域 `run-175512` 轮询误杀、`run-172548` control pipe 关闭，根因未闭合；CW9-4 本体仍 pending） |
| E10 / CW10 | 依赖/许可、干净 Windows 安装/回滚、跨 Host、PR 切片、版本文档 | Perlica | `in_progress`（依赖/PR 映射） |
| AC | CW0–CW7/FIX-RS 验收 checklist、Gate 评审、validator/reviewer 分工 | Perlica | `in_progress` |

原则：CW8/CW9/CW10 的**执行**依赖 CW7/CW2/CW0 等前置；现在完成不依赖实现的 inventory/任务矩阵/故障矩阵（CW9-1）/依赖与 PR 映射/验收清单。CW9-2 Batch A 首轮已执行（15 行：6 pass / 4 partial / 3 fail / 2 blocked），但 **CW2 未稳定**：长 RPC 抢占、在途 deadline、owned launcher、Host-death 仍未闭合，Batch A 暂停重跑。Batch B（26 行 local_browser）需 CW4/CW5；Batch C（10 行 real_cloud_session）需 CW7；fixture/本地结果不能替代真实云声明。不创建 GitHub issue/PR/评论，不把规划当 Gate 通过。

## Gate 判定

| Gate | 条件 | 当前 |
| --- | --- | --- |
| G0 | CW0/CW1：基线、合同、真实 provider、最低版本 | `partial` |
| G1 | CW2 最小停止 + CW3 输入 + CW4 媒体/隔离（真实 Windows） | `blocked`（未闭环） |
| G2 | CW7 实际 MaaEnd UI 连接→识别/动作→新帧→预览→取消→销毁 | `blocked`（仅有条件成功） |
| G3 | CW8 46 项 + pretask/选项/预设实际 Pass | 未开始 |
| G4 | CW9 故障/长时（51 行矩阵全部闭合）+ CW10 跨 Host/干净安装/上游 review | 未开始（CW9-1 仅矩阵；Batch A 首轮 6/4/3/2） |

## 与旧 CW-xx 的映射

| 旧编号 | 新阶段 |
| --- | --- |
| CW-01/02/03/04/07 | CW2/CW3（协议、租约、输入、canonical） |
| CW-05/06 | CW4（媒体/预览） |
| CW-10/11/12/13 | CW1/CW5（Host/接线/生命周期） |
| CW-20/21/22/23/24/25/26 | CW1/CW3/CW6（身份、输入、MaaEnd 适配） |
| CW-27/28 | CW6/CW7（任务矩阵与首任务） |
| CW-30/31/32 | CW7/CW9/CW10（验证、故障、交付） |
| CW-33 | CW2（Faulted/possibly_held 的延续） |

## 当前真实证据

- 30 个 `run-*.json`：exit 0 ×5；exit 1 ×17；exit 3 ×3；exit 7 ×3；exit 9 ×1；exit 2 ×1。
- exit 0 全部清单：`165438`、`165556`、`165726`、`172134`、`174123`。此前只记录 `172134` 一条，口径不完整。
- `165438`/`165556`/`165726`：无 auto_start，前置界面未记录；`165438` sidecar 正常退出，后两条 force_kill。
- `172134`：MANIFEST 记录从 Regional Development 界面启动、绕过菜单点击；sidecar force_kill。
- `174123`：login restore + auto_start 的**冷启动路径一次成功**；`activationBefore.active=false → activationAfter.active=true` 需复核 OS 焦点语义；窗口事件曾出现 inner 854×480/dpr 1.5；reward delta 未证明。冷启动不是“从未成功”，但后续 `174331` 仍失败，属不稳定。
- 任务级 `Succeeded` ≠ 实际领取新奖励：DailyRewards 上游只启用 `DailyClaimDeliveryJobsRewardSub`，其余四项 `enabled:false`；分支可达不等于奖励状态变化。
- 失败：`172301`（菜单冷启动 exit 1）、`174331`（Regional 超时 exit 1）、`175512`（轮询误杀 exit 7）、`175718`（MXU API 连续超时 exit 2）。
- CW9-1 故障矩阵：`fault-matrix.json` `ed335379…` / `.md` `2d9daa73…`；8 域 51 行全 Pending；已登记 `175718`/`175512`/`172548` 三个真实输入，根因未闭合；矩阵存在不等于故障已执行或 G4 通过。
- CW9-2 Batch A 首轮：sidecar `8b9cc6f` + diff `4c775f51…`；`cargo test -- --skip cdp_` 40/40，pass 6 / partial 4 / fail 3 / blocked 2；`results.json` / `report.md`；长 RPC 抢占、在途 deadline、pending/背压与 Host ACK harness 未闭合。
- CW2 重评（round 2）：最新候选 `6227c5a8…`（sidecar `1f8625d5…` / MXU `e193e0d4…`）已把 urgent path、在途 deadline、pending/Backpressure、attach-only、Host kill 进程退出、真实云协议 Stop 逐项闭合；`review-lease`/`review-renew-sidecar` pass，`review-combined` pass-with-notes，`review-format` pass。边界：job kill 瞬间 held 输入释放 best-effort。`cw2/clause-table.md`、`cw9/cw2-reevaluation.md`。
- CW9-2 Batch A round2：15 行 **10 pass / 5 partial / 0 fail / 0 blocked**；`cargo test --all-targets` 52/0/1；round2 结果 `d1581347…`。
- CW9 Batch B round2：26 行 local_browser，14 partial / 11 not_run / 1 blocked、0 pass；4 个 CDP 测试在新候选 **4 passed / 0 failed**，test-only harness diff `bb4f38aa…`，产品 diff 仍 `1f8625d5…`；首次 round2 有 1 个测试假设失败（截图 media.presented 与 rVFC 不应比较），改为 source 检查后通过；日志 `3ae5145b…`。
- 旧运行没有 `backend_kind` 字段，无法区分内层 fake/cdp；按配置、首帧 `0x0`/281ms 等只能稳妥推断为 cdp，不能标已验证。CW1 候选改变后旧证据失效，需重生成。
- 已修运行时项：标题双击、`InMenuList` OCR-only/右侧兜底、`Regional` 两行 OCR、`MenuListEnterWorld` Escape、轮询容忍、best-effort kill；均未形成产品候选，`cloudweb-run.ps1` 为本地联调 supervisor。
- 交付审计：`cases/cloudweb-delivery-20261024-success/`（status `needs_review`，绑定旧 MANIFEST `ccc3a611…`；当前 MANIFEST `0fa1e985…`，最终冻结需重绑）。

## CW1 固定候选（本地）

| 项 | 值 |
| --- | --- |
| sidecar | HEAD `8b9cc6f` + tracked diff `9dc8faf9…` + README `fbc10ab3…` + LICENSE `8486a10c…`；release exe `d98146a5…` |
| MXU | HEAD `8bddfee` + diff `9342a67f…` |
| MaaEnd | HEAD `c7fd1b25`，本轮未改 |
| 协议 | 1.1（`hello_ok` 增加 `backend_kind`/`ui_profile`） |
| 许可 | sidecar `AGPL-3.0-only` |
| candidate ID | `adaa5905d12f1f21859dbb32ec01255206acafc432bb39bb3076bd8660e15b4c` |
| 报告 | `cases/cloudweb-cw0-cw7/cw0/baseline-inventory.md`、`cases/cloudweb-cw0-cw7/cw1/integration-report.md` |

工具链：rustc/cargo `1.99.1`；sidecar 用默认 GNU host；MXU 测试必须用 `+stable-x86_64-pc-windows-msvc`（GNU 测试二进制在 WinRT API set 上加载失败）。sidecar `rust-version = "1.88"` 已在 1.88.0 上实测通过（36 个测试当时；1.85 被锁定 icu 依赖与 let-chains 拒绝）。

## CW2 固定候选（本地，最小停止）

| 项 | 值 |
| --- | --- |
| sidecar | HEAD `8b9cc6f` + tracked diff `9d477616…`；release exe `39c2f427…` |
| MXU | HEAD `8bddfee` + diff `da21b658…` |
| candidate ID | `391659fbb603f2c97e18659861d326385c991451f04d5e1d185b7194d70ebd52` |
| 报告 | `cases/cloudweb-cw0-cw7/cw2/integration-report.md` |
| 已验证 | typed maybe-held Stop、2 秒真实进程长按、命令完成后续租、Host 续租线程 |
| 未闭合 | 长 RPC 进行中 Stop 抢占、owned launcher/profile、Host-death 回收、真实 CDP 长按 |

## CW3 固定候选（本地，输入语义）

| 项 | 值 |
| --- | --- |
| sidecar | HEAD `8b9cc6f` + tracked diff `2681e739…`；release exe `8c90324b…`；README `4eead4ef…` |
| MXU | HEAD `8bddfee` + diff `201e5bb9…` |
| candidate ID | `574f4b0fc59616b072ef0ea8eb9d45ed49ff9b750d5e51e8466e7e056ac1743b` |
| 报告 | `cases/cloudweb-cw0-cw7/cw3/integration-report.md` |
| 已验证 | relative 分段/trusted anchor/buttons/modifiers；contact1/2 映射；2 秒长按；wheel/drag/text 本地 smoke |
| 未闭合 | 真实云游戏持续组合与中键锁敌、SDK 正向 readiness、人工接管、Windows 隔离 |

## CW4 固定候选（本地，媒体语义）

| 项 | 值 |
| --- | --- |
| sidecar | HEAD `8b9cc6f` + tracked diff `28aec95f…`；release exe `64820137…`；README `4eead4ef…` |
| MXU | HEAD `8bddfee` + diff `201e5bb9…` |
| candidate ID | `6d7393bb82b1fc68c3ca1b20e99dc754dc6045783bafc91c1fe6dcf3171a6f60` |
| 报告 | `cases/cloudweb-cw0-cw7/cw4/integration-report.md` |
| 已验证 | rVFC Start/Stop 幂等、canonical JPEG、doc/geometry、原子 slot、本地 headless CDP |
| 未闭合 | sidecar↔CDP 全链路、真实云视频、真实冻结、snapshot token、Host 帧去重/slot/receipt |

## CW5 固定候选（本地，最小 reader/cache）

| 项 | 值 |
| --- | --- |
| sidecar | HEAD `8b9cc6f` + diff `28aec95f…`；release exe `64820137…` |
| MXU | HEAD `8bddfee` + diff `d776975d…` |
| candidate ID | `8e1b0e60166a922cb25a5796c0c81c681769c48199ac56e51c0d320872e76111` |
| 报告 | `cases/cloudweb-cw0-cw7/cw5/integration-report.md` |
| 已验证 | 唯一 capture gate、immutable 100ms snapshot、TTL 命中、generation 清理、诊断 token |
| 未闭合 | preview/识别 token 分离、pool/run/task、锁外 shutdown、API 响应性、双实例、真实 CDP 预览 |

## CW6 固定候选（本地，确定语义修复）

| 项 | 值 |
| --- | --- |
| MaaEnd | HEAD `c7fd1b25` + diff `247a3dbb…` + overlay `1e533f5a…` |
| sidecar | HEAD `8b9cc6f` + diff `28aec95f…`；exe `64820137…` |
| MXU | HEAD `8bddfee` + diff `d776975d…` |
| candidate ID | `4b7580d85f8c16167dfaea0bab65faae50eebd5677788bb227b60808ceda4a3e` |
| 报告 | `cases/cloudweb-cw0-cw7/cw6/integration-report.md` |
| 已验证 | 全局 Escape 撤销、通用 AutoAltClick 恢复、薄 overlay、schema/validator/安装同步、Go build/test |
| 未闭合 | 真实云菜单/镜头、region/UID、能力门、云生命周期、`__SceneImageCheck` |

## CW7 最终候选与阻塞

| 项 | 值 |
| --- | --- |
| candidate ID | `9ce6f5782752089fd4738de69ee9e62f32cb2543c9be4eead3cbbe7e0e03ea98` |
| sidecar | HEAD `8b9cc6f` + diff `4c775f51…`；exe `3684d64a…` |
| MXU | HEAD `8bddfee` + diff `d776975d…` |
| MaaEnd | HEAD `c7fd1b25` + diff `247a3dbb…` + overlay `f5c9fffe…` |
| 报告 | `cases/cloudweb-cw0-cw7/cw7/audit-report.md` |
| 已验证（真实云） | `094713`、`094836` DailyRewards exit 0，产品 overlay、无 runtime overrides；第二次为进程级 Stop/重开 |
| 已验证（本地/静态） | sidecar 40/40、clippy 0；MXU 43/45（1 环境失败、1 ignored 真进程通过）；headless CDP relative+rVFC；Go build/test；schema/安装同步 |
| 未闭合 | 真正标题/新标签冷启动需再次人工登录；MapNavigator+AutoFight；前台/焦点/输入隔离；in-session Stop |

**下一步**：管理员再次登录后，从标题/新标签真正冷启动跑 DailyRewards→Stop→重开，再 MapNavigator/AutoFight，并采集前台未变/无 OS 焦点/输入隔离证据。CW7 未通过前不宣称 G2。

## 管理员裁定（2026-10-24）

- **sidecar 许可**：采用 `AGPL-3.0-only`，与 MaaEnd/MXU 一致；`LICENSE`（sha256 `8486a10c…`）与 Cargo `license` 字段已落地。
- **全局 AGENTS drift**：保留当前全局文件（sha256 `93cabb…`）并重新登记到 `runtime.json`；工程行为继续受 perlica-maaend 角色、仓库规则和公开面内容闸约束。当前 `runtime.json` 已改为 `93cabb…`。
- **中间态 review/validation**：CW1–CW6 不再设独立 review/validation 门禁；每增量只保留编译/构建、针对性测试、联合 smoke/demo 与固定候选 hash。完整审计与验证集中在 CW7 一次执行。CW1 已启动的 reviewer 结果仅作参考，不再阻塞或改变 CW1 状态。仓库强制检查、停线规则、真实证据判定和公开写入审批不放松；FIX-RS 在向上游提交前仍按独立范围复核。

## 下一步顺序

0. 并行（Perlica 独立推进）：CW8 inventory/任务族矩阵（done）、CW9-1 故障矩阵（done）、CW9-2 Batch A round2（10/5/0/0）、CW9 Batch B round2（4 tests pass，0 row pass）、CW10 依赖/PR 映射、AC 验收清单。
1. CW0 剩余字节级一致性/证据；CW1 本地实现完成；不设中间独立复核；FIX-RS 并行。
2. CW2 协议层已 closed（pass-with-boundary：硬杀瞬间输入释放 best-effort）；Batch A 剩余 5 行是 Host ACK/SeqConflict/renew-loss/token，不再等待 CW2；Batch B 继续 B2/B3 与 Host/跨 run。
3. CW3 本地输入语义、CW4 本地媒体 collector/canonical 帧、CW5 最小 reader/cache 已接通；真实云游戏持续组合、sidecar↔CDP 全链路、preview/识别 token 分离、人工接管与隔离并入 CW7。
4. CW6 全局 Escape 撤销、薄 overlay、schema/安装同步已接通；region/UID、能力门、云生命周期与真实云回归并入 CW7；CW7 冷启动→DailyRewards→Stop/重开→MapNavigator/AutoFight。
5. **CW7 唯一完整审计/验证门**：对 CW0–CW6 的候选、真实 CDP/Windows、后台隔离、证据、许可、干净构建与文档做一次集中审计；在此之前不宣称 G0–G2 或完整功能通过。

## 停线

- 同一失败两次 → 诊断，不硬撞。
- 未取得 Windows 真机/真实会话/维护者输入时，不宣称 G1/G2/G3。
- 不自动 push/PR/Issue/评论；候选/证据先绑定 SHA/hash。
- 旧证据在候选变化后失效；一次成功不等于可复现。
- 不把 fake、假 SDK、固定 PNG、独立 Node 脚本算作游戏通过。
