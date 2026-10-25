# CloudWeb CW0–CW7 实施计划（历史）

> **2026-10-25 架构转向**：当前实施路线见 [`cloudshell-roadmap.md`](cloudshell-roadmap.md)，
> 对齐上游 #6841 的 CloudShell/WebView2 + Win32/PrintWindow/PostMessage。本文保留为
> CW0–CW7 sidecar/MXU 历史计划，不再代表当前产品路线。

状态：规划稿，2026-10-24。单一源进度以同目录 `cloudweb-roadmap.md` / `cloudweb-roadmap.json` 为准；本文定义各 CW 的合同、验收和首批任务。
材料权威顺序：`MaaEnd-CloudWeb-Implementation-Review-2026-10-24.md`（主报告）→ `FeatureRequirementDoc-MaaEnd-CloudWeb-Neo.md` v1.1 §0 → `MaaEnd-CloudWeb-Coordinator-Report.md` §0。旧 P00–P17/CW-xx 编号只作历史映射，不再作为进度源。

> **2026-10-24 管理员流程裁定**：CW1–CW6 不设中间独立 review/validation 门禁。每个增量只做编译/构建、针对性测试、联合 smoke/demo 和固定候选 hash；完整审计与验证集中在 CW7 一次执行。本文各 CW 的 reviewer/validator 条款仅在 CW7 生效。仓库要求的检查、停线规则、真实证据判定和公开写入审批不因此放松；FIX-RS 作为独立交付，在向上游提交前仍按自身范围复核。

## 固定基线

| 组件 | SHA |
| --- | --- |
| sidecar | `8b9cc6f670fc59652ffb7f50e312d1949e7f73ac` |
| MXU | `8bddfee34bba3ea8ec81e3b8fdcb118375329b76` |
| MaaEnd | `c7fd1b25071d9976703635b7dda66812172d1629` |
| MaaEnd v2 | `051d17f2527e2e990952d6655f1f380d5d7de81e` |
| qoder-agentset | `e3dab67d997840c5d8ae4cf0328b2b36f308368c` |
| maa-framework-rs patched | `953ec242d2542ab907211a4aa347b9a855f698aa` |
| maa-framework-rs unfixed | `c1c617330b56205798609c7566e602b55adba74b` |

产品分母：49 任务 / 426 option / 3 预设 / 1 pretask；Windows 适用 46 项；CloudWeb 当前只开放 `DailyRewards`。完整门槛是 46 项 Windows 业务 + LaunchGamePC 云端等价 + 必要选项/路线/预设，不能用白名单缩减。

## EPIC 与里程碑

| ID | Epic | 里程碑 | Gate |
| --- | --- | --- | --- |
| E0 | 候选、合同与可复现基线 | M0 CW0 | G0 |
| E1 | 最小公共接线与真实 provider 身份 | M1 CW1 | G0 |
| E2 | Session/run、独立续租与最小停止 | M2 CW2 | G1 前置 |
| E3 | 完整 PC 持续输入 | M3 CW3 | G1 |
| E4 | 真实媒体、几何与 observation token | M4 CW4 | G1 |
| E5 | Host 生命周期、唯一 frame reader 与只读预览 | M5 CW5 | G2 前置 |
| E6 | MaaEnd 云身份、薄 overlay 与任务适配 | M6 CW6 | G2 前置 |
| E7 | 第一条真实纵向闭环 | M7 CW7 | G2 |
| EF | FIX-RS 独立 binding 修复 | FIX-RS | 正式发布依赖（非 CloudWeb 阻塞） |

CW8（46 任务）、CW9（故障/长时）、CW10（发行/上游）保留主报告定义，不在本轮首批闭环内，但文档与分母不缩小。

## CW0 候选与契约盘点

- 目标：固定三仓与依赖、可复现构建、实际生效配置、许可/工具链/启动说明，分清真实 backend 与 fake。
- 依赖：无；owner：协调者 + 独立 reviewer。
- 当前：`partial`。源码访问/全文审查已关闭；候选 SHA 与材料一致；sidecar 37/0/1、clippy 0；MXU/MaaEnd 有本地构建。缺口：sidecar 无 LICENSE/README/`.github`，Cargo manifest 无 license/rust-version；MXU/MaaEnd 干净构建与安装步骤未形成一份可复现记录；实际生效的 backend/profile 配置未与发行物绑定。
- 首批任务：①逐仓记录 `git status`/HEAD/remote/构建命令/SHA；②写许可证与最低工具链清单；③记录 MXU `mxu-cw.exe`、MaaEnd `install`、sidecar exe 的构建输入与哈希；④写真实 vs fake 的判定表；⑤把旧 `cloudweb-dist` 证据纳入统一清单。
- 验收：固定 SHA + 可复现构建 + 实际配置清单 + 真实/fake 判定；源码审查完成不等于产品通过。

## CW1 最小公共接线

- 目标：provider/launcher、可选 HWND、最低版本；外层 adapter 与实际 actuator 分开；生产显式 CDP 并核验 identity/profile/capabilities。
- 依赖：CW0 初版合同；owner：Host/PI Agent。
- 当前：`partial`。sidecar CLI 支持 `--backend cdp --cdp-url` 且缺 URL 显式失败；但默认 backend 是 fake，Host 未核验实际 actuator，hello 未携带有效 profile/capability/identity 合同，MXU 仍可能把外层适配器当成真实 backend。
- 首批任务：①hello 返回 provider/backend 实际 kind、profile、capabilities；②Host 校验并显示实际 kind，fake 仅显式开发入口；③加最低版本/缺参拒绝；④补 schema/factory 样例；⑤给维护者可审的公共接线 diff。
- 验收：真实 backend 未配置即失败；fake 不能误认真实；通用 factory/schema 可供维护者审阅；不要求上游先接受。

## CW2 Session/run 与最小停止

- 目标：拆 CDP 等待与状态锁；run/document epoch、独立 renew、typed held∪maybe、真实 deadline；owned launcher/profile、Host-death 回收与最小中和。
- 依赖：CW0/CW1；owner：sidecar + Host 共同 owner。
- 当前：`partial`。已有 350ms lease、50ms supervisor、逐键 release；但长 CDP 命令会共锁续租，`possibly_held` 仍是字符串，Stop 只遍历 confirmed，未做 typed 并集；无 owned launcher/profile，Host-death 回收未闭环。
- 首批任务：①typed `Confirmed|MaybeHeld` ledger，派发前/后更新，Stop 遍历并集；②独立 renew 通道，安全预算不等待 screenshot；③run/document/geometry 三代身份与 fence；④owned launcher/profile + Host-death 回收；⑤长按/慢 RPC/unknown down 反例测试。
- 验收：2 秒长按不被 350ms lease 误释放；unknown down 后 Stop 真正尝试 up；慢 RPC 不阻塞安全预算；只回收自己的进程。主报告 §7.1 CW2 与 FRD §10.1 B10。

## CW3 完整 PC 输入

- 目标：真实 relative；contact1→wire2、contact2→wire1；modifier/repeat、滚轮/拖动/中文；SDK readiness、人工接管。
- 依赖：CW2 最小停止；owner：browser input Agent。
- 当前：`partial`。绝对点击/键盘/SDK 键盘可用；`CdpActuator::move_relative` 仍返回 Err，能力不含 `input.relative`；Host contact 映射与 Framework 相反；wheel/文本/拖动只在 SDK 层观察；人工接管未实现。
- 首批任务：①实现页面内 pending delta + trusted anchor mousemove 的 relative 事务，带 document/run/command token；②修 contact 映射；③modifier/repeat/wheel/drag/text；④SDK readiness 与失败传播；⑤真实游戏持续组合与隔离反例。
- 验收：W＋camera＋右键＋技能持续组合；中键锁敌正确；真实游戏而非假 SDK；不借全局鼠标/焦点；主报告 §6.5 相对镜头段落。

## CW4 媒体与几何

- 目标：rVFC Start/Stop/source epoch；接 media_probe/poll_cycle；video→720p 规范化；真实进度、document/geometry、snapshot token；修帧去重/slot/请求合同。
- 依赖：CW0/CW1；真实接线依赖 CW2；owner：media Agent。
- 当前：`not_started`。生产仍是整页 PNG + 截图计数；rVFC 未接；geometry_revision 恒 0；无 snapshot token；媒体冻结时仍可能返回截图成功。
- 首批任务：①collector epoch 绑定单一 video/srcObject/track，迟到 callback 验 epoch；②Start/Stop 幂等；③media progress/health 与 `MEDIA_STALLED`；④canonical 1280×720 像素与 metadata 原子绑定；⑤frame dedupe/slot/receipt 合同；⑥snapshot token 与 recognition/preview 分离接口。
- 验收：静态画面仍能证明直播；媒体冻结但截图 RPC 成功时不标 Fresh；重复取同帧及 A/B 覆盖不失配；像素与 metadata 原子一致。

## CW5 Host 生命周期与预览

- 目标：唯一 frame reader/cache；preview 与识别 snapshot 分离；pool/run/task ID、append/rollback/旧事件；真实 Ready、锁外 shutdown。
- 依赖：CW2/CW4；owner：MXU Agent。
- 当前：`partial`。MXU 有真实 sidecar backend、帧/任务启动/退出；但无唯一 frame reader/cache，预览与识别未分离，pool/run/task 事件过滤与锁外 shutdown 未闭环；本轮观察到任务中 `/api/maa/state` 可 >60s 无响应，属于待查生命周期风险。
- 首批任务：①一个 reader 保存 immutable snapshot；②preview 只读 latest，识别取 token；③pool/run/taskID/append/rollback/旧事件；④Ready 与错误状态；⑤锁外 shutdown 与 API 响应性探针；⑥双实例不争写。
- 验收：长动作实况持续；A 识别/B 预览不改写观察依据；双实例不争写；取消/退出有可见结果；API 阻塞有诊断与上限。

## CW6 MaaEnd 适配

- 目标：恢复通用 modifier+target click；只对确认入口做薄 overlay；legacy camera 接真实 relative；region/UID cache；能力准入；启动/关闭/切账号云语义。
- 依赖：身份可在 CW1 后先改；定稿依赖 CW3/CW4；owner：MaaEnd Agent。
- 当前：`partial`。C++ `cloud_web` 与 DailyRewards 白名单已接入；但 `AutoAltClickAction` 被全局改成 CloudWeb Escape（主报告 §0.1 明确反对），DailyRewards 菜单入口应改薄 overlay；region/UID、legacy camera、能力准入、云生命周期未完成。
- 首批任务：①撤销全局 Escape 分支，恢复通用 modifier+target click；②为确认入口做精确薄 overlay；③legacy camera 接 CW3 relative；④region/UID cache 与失效；⑤能力准入与启动/关闭/切账号云语义；⑥重生成受影响的任务证据。
- 验收：本机 Win32/HDR/进程假设不触发；选项/资源与实际 profile 一致；schema 与宿主接线同步；主报告 §0.1 `[CWREVAUTOALT]`。

## CW7 第一条真实纵向闭环

- 目标：记录真实 actuator/目标身份；连接→登录/排队→DailyRewards 子分支→Stop/重开；再做原 MapNavigator + AutoFight。
- 依赖：CW3/CW4/CW5 + CW6 首条任务必需子集；owner：integration Agent + 独立 validator。
- 当前：`partial`（候选 `eb50e2ff...`，详见 `cw7/audit-report-late.md`）。真实云已通过：`run-20261024-112908` 最终 exe 人工登录后的冷启动（首页→auto_start→标题→世界）→ DailyRewards exit 0；`run-20261024-114141` 开放世界起点 MapNavigator Succeeded；`autofight-20261024-114725` 最终 exe 真实战斗内 AutoFight，Running→Stop→`ReleaseOwner`；`run-20261024-114523` 中文 UI 下 Operational Manual→影拓丰碑详情；`autofight-20261024-103758/104052` 为早期 AutoFight/Stop 证据。MapNavigator 早期 `104430`（室内）/`113125`（菜单）因无开放世界起点失败；管理员手动传送到悬谷台后，`run-20261024-114141`（最终 exe）产品 trial 路径 `MapNavigateAction` **Succeeded**（~50s，到达演武场入口）。最终候选经 reviewer-cw7d 判 pass。奖励增量、46 项矩阵、静态构建、Linux/macOS runner、零焦点变化（`foreground-autofight.log` 有 1/180 MXU 前台）仍未闭合；FIX-RS 待收口。
- 首批任务：①把 CW6 薄 overlay + CW3 relative 接入后复跑冷启动；②记录真实 actuator/target/profile 身份；③连接→登录/排队→DailyRewards→Stop→重开；④MapNavigator + AutoFight；⑤前台未变/无 OS 焦点/输入隔离证据；⑥独立 validator 复核。
- 验收：实际游戏和后台隔离成功；邮件有/无场景；fake/假 SDK/固定 PNG 不可计入；G2 关闭。当前成功不能关闭 CW7。

## FIX-RS 独立绑定修复

- 目标：last-owner 回收、record/Tasker retention、destroy quiescence；移植到最新 upstream。
- 依赖：可立即开始；owner：binding Agent + native reviewer。
- 当前：`partial`。已复现 callback Drop=0、100 次循环 Arc=101（泄漏）；patch `953ec24`（2 文件 +79/−8）实现最后 clone Drop=1、100 次 Drop=100、Arc=1，连接/构造失败恰好释放一次；未覆盖 record/Tasker retention 与 destroy quiescence 的最新上游回归。
- 首批任务：①以 `c1c6173` 为 unfixed 基线、`953ec24` 为参考，移植到最新 upstream；②补 record wrapper 与 Tasker retain 场景；③native destroy 静默期/并发销毁/失败路径；④DropProbe 与 callback 恰好一次断言；⑤独立 PR（不夹带 CloudWeb）。
- 验收：callback 恰好一次且不早于 native 使用结束；独立 PR 后回正式发布依赖；主报告 §7.1 FIX-RS。

## 执行顺序

1. CW0 证据与构建清单；CW1 接线合同；FIX-RS 并行。
2. CW2 最小停止；CW4 collector 可并行起步。
3. CW3 relative/按钮/隔离；CW4 真实接线；CW5 frame reader/preview。
4. CW6 薄 overlay/region/legacy camera；随后 CW7 冷启动→DailyRewards→Stop/重开→MapNavigator/AutoFight。

## 停线

- 同一失败两次 → 诊断，不硬撞。
- 未获 Windows/真实会话/维护者输入时，只做合同/fake/native/静态工作，不宣称 G1/G2。
- 不自动 push/PR/Issue/评论；候选与证据先绑定 SHA/hash。
- 旧证据在候选变化后失效；成功一次不等于可复现。
