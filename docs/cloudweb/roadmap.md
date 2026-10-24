# CloudWeb 单一源 roadmap（CW0–CW10）

状态审计：2026-10-24 18:10（Asia/Shanghai）
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
| E4 媒体、几何与 observation token（CW4） | M4 | `not_started` | G1 |
| E5 Host 生命周期与预览（CW5） | M5 | `partial` | G2 前置 |
| E6 MaaEnd 云身份、薄 overlay 与任务适配（CW6） | M6 | `partial` | G2 前置 |
| E7 第一条真实纵向闭环（CW7） | M7 | `partial` | G2 |
| EF FIX-RS 独立 binding 修复 | FIX-RS | `partial` | 正式发布依赖 |

CW8 全任务资格（G3）、CW9 故障/长时、CW10 打包/上游收口（G4）保留主报告定义；本轮先闭环 CW0–CW7，不缩小最终分母。

## CW0–CW7 状态

| ID | 事项 | 状态 | 依赖 | 当前证据/缺口 |
| --- | --- | --- | --- | --- |
| CW0 | 候选与契约盘点、可复现构建、实际配置、许可/工具链 | `partial` | 无 | 三仓 SHA 已固定、源码审查关闭；sidecar 无 LICENSE/README/rust-version；MXU/MaaEnd 干净构建与实际 backend/profile 清单未成文 |
| CW1 | provider/launcher、最低版本、外层 adapter 与实际 actuator 分离、生产显式 CDP | `partial` | CW0 | sidecar 支持 `--backend cdp --cdp-url` 且缺 URL 失败；默认仍 fake，Host 未核验实际 kind，hello 未带 profile/capability/identity 合同 |
| CW2 | run/document epoch、独立 renew、typed held∪maybe、真实 deadline、owned launcher、Host-death 回收 | `partial` | CW0/1 | 已有 lease/supervisor/逐键 release；长 CDP 共锁续租、`possibly_held` 字符串、Stop 仅 confirmed、无 owned launcher |
| CW3 | 真实 relative、contact 映射、modifier/repeat、wheel/drag/text、SDK readiness、人工接管 | `partial` | CW2 | 绝对点击/键盘可用；`move_relative` Err、无 `input.relative`；contact1/2 映射相反；wheel/text/接管未证 |
| CW4 | rVFC/媒体进度、720p 规范化、document/geometry、snapshot token、帧去重/slot | `not_started` | CW0/1；真实接线依赖 CW2 | 生产仍是整页 PNG+截图计数；geometry 恒 0；无 snapshot token；媒体冻结仍可能返回截图成功 |
| CW5 | 唯一 frame reader/cache、preview/识别分离、pool/run/task ID、锁外 shutdown | `partial` | CW2/4 | MXU sidecar backend/帧/任务/退出可用；无唯一 reader/cache 与 preview 分离；任务中 `/api/maa/state` 可 >60s 无响应 |
| CW6 | 恢复通用 modifier+target click、薄 overlay、legacy camera、region/UID、能力准入、云生命周期 | `partial` | CW1 后可先改；定稿依赖 CW3/4 | C++ `cloud_web` + DailyRewards 白名单已有；`AutoAltClickAction` 被全局改 Escape（材料 §0.1 明确反对）；region/UID、legacy camera、云生命周期未完成 |
| CW7 | 真实 actuator/目标身份；连接→登录/排队→DailyRewards→Stop/重开；MapNavigator+AutoFight；后台隔离 | `partial` | CW3/4/5 + CW6 子集 | 一次 `run-20261024-172134` exit 0（从 Regional 界面启动）；冷启动 `172301`/`174331` 失败；Stop/重开、导航/战斗、隔离未验证 |
| FIX-RS | last-owner 回收、record/Tasker retention、destroy quiescence；移植最新 upstream | `partial` | 可立即开始 | 已复现 Drop=0/Arc=101；patch `953ec24` Drop=1/Arc=1；未覆盖 record/Tasker/destroy 静默期与最新上游回归 |

## Gate 判定

| Gate | 条件 | 当前 |
| --- | --- | --- |
| G0 | CW0/CW1：基线、合同、真实 provider、最低版本 | `partial` |
| G1 | CW2 最小停止 + CW3 输入 + CW4 媒体/隔离（真实 Windows） | `blocked`（未闭环） |
| G2 | CW7 实际 MaaEnd UI 连接→识别/动作→新帧→预览→取消→销毁 | `blocked`（仅有条件成功） |
| G3 | CW8 46 项 + pretask/选项/预设实际 Pass | 未开始 |
| G4 | CW10 故障/长时/跨 Host/干净安装/上游 review | 未开始 |

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

- 成功：`cloudweb-dist/EVIDENCE/run-20261024-172134.json` exit 0、`Succeeded`；前置为 Regional Development 界面启动；DailyRewards 上游仅启用 `DailyClaimDeliveryJobsRewardSub`，其余四项 `enabled:false`。
- 失败：`172301`（菜单冷启动 exit 1）、`174331`（Regional 超时 exit 1）、`175512`（轮询误杀 exit 7）、`175718`（MXU API 连续超时 exit 2）。
- 已修运行时项：标题双击、`InMenuList` OCR-only/右侧兜底、`Regional` 两行 OCR、`MenuListEnterWorld` Escape、轮询容忍、best-effort kill；均未形成产品候选，`cloudweb-run.ps1` 为本地联调 supervisor。
- 交付审计：`cases/cloudweb-delivery-20261024-success/`（status `needs_review`，绑定旧 MANIFEST `ccc3a611…`；当前 MANIFEST `0fa1e985…`，最终冻结需重绑）。

## 下一步顺序

1. CW0 证据/构建清单 + CW1 接线合同；FIX-RS 并行。
2. CW2 最小停止；CW4 collector 可先做。
3. CW3 relative/按钮/隔离；CW4 真实接线；CW5 frame reader/preview。
4. CW6 薄 overlay/region/legacy camera；CW7 冷启动→DailyRewards→Stop/重开→MapNavigator/AutoFight。

## 停线

- 同一失败两次 → 诊断，不硬撞。
- 未取得 Windows 真机/真实会话/维护者输入时，不宣称 G1/G2/G3。
- 不自动 push/PR/Issue/评论；候选/证据先绑定 SHA/hash。
- 旧证据在候选变化后失效；一次成功不等于可复现。
- 不把 fake、假 SDK、固定 PNG、独立 Node 脚本算作游戏通过。
