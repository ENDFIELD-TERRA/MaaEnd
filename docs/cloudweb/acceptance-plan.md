# CloudWeb CW0–CW7 / FIX-RS 验收准备

状态：2026-10-24 18:30，`in_progress`（准备，不是验收通过）。
Owner：Perlica（coordinator）；执行验收：独立 validator/reviewer，与实施 Agent 分离。
单一源：`cases/cloudweb-roadmap.md` / `cloudweb-roadmap.json`；本文件是验收 checklist。

## 角色与规则

- **validator**：逐项复读/复跑证据，不接受实施者自证；每条结论标 verified/partial/blocked。
- **reviewer**：代码/协议独立复核，与 validator 分离。
- **coordinator**：主持 Gate、绑定候选 SHA/hash、保留未通过项、对外汇报。
- **delivery**：最终候选审计/打包，不补证据。

证据最低字段：候选 SHA、构建/产物 hash、真实 actuator/target/profile、实际命令/结果、截图/ROI/跳转、失败/取消/恢复、未验证边界。缺一项即不能标 verified。

## AC-CW0 候选与契约盘点

- [ ] 三仓 HEAD/remote/worktree/构建命令/产物 hash 一致，可复现。
- [ ] 实际生效 backend/profile 配置写入发行物/启动说明。
- [ ] sidecar LICENSE/README/rust-version、MaaEnd/MXU 工具链/最低版本齐全。
- [ ] 真实 backend 与 fake 判定表；默认 fake 不会被误认。
- [ ] codegraph 索引/查询可用（本地证据，不作为产品通过）。
- 当前：`partial`。缺口：许可/工具链/实际配置；不能因源码审查关闭。

## AC-CW1 最小公共接线

- [ ] hello 返回实际 provider/backend/profile/capabilities。
- [ ] Host 校验并显示实际 kind；生产显式 CDP；缺参失败。
- [ ] fake 仅显式开发入口，不计资格。
- [ ] 外层 adapter 与实际 actuator 分开；schema/factory 样例可审阅。
- [ ] 最低版本/optional HWND 合同。
- 当前：`partial`。sidecar 默认 fake；Host 未核验实际 actuator。

## AC-CW2 Session/run 与最小停止

- [ ] typed `Confirmed|MaybeHeld` 并集；Stop 遍历并集。
- [ ] 独立 renew，安全预算不等待 screenshot。
- [ ] run/document/geometry 三代 fence；旧 token 不重放。
- [ ] owned launcher/profile + Host-death 回收；只回收自己的进程。
- [ ] 2 秒长按不被 350ms lease 误释放；慢 RPC 不阻塞 Stop；unknown down 后尝试 up。
- 当前：`partial`。possibly_held 仍是字符串；无独立 renew/owned launcher。

## AC-CW3 完整 PC 输入

- [ ] 真实 relative（document/run/command token，pending delta 失效）。
- [ ] contact1→wire2、contact2→wire1；hover 不按下。
- [ ] modifier/repeat、wheel、drag、中文文本。
- [ ] SDK readiness、人工接管、失败传播。
- [ ] 真实游戏 W+camera+右键+技能持续组合；中键锁敌；无全局鼠标/焦点。
- 当前：`partial`。relative Err、无 input.relative；contact 映射相反；wheel/text/接管未证。

## AC-CW4 媒体与几何

- [ ] rVFC/collector epoch 绑定单一 document/video/source/track；Start/Stop 幂等；迟到 callback 验 epoch。
- [ ] media progress/health；`MEDIA_STALLED`；冻结不标 Fresh。
- [ ] 720p 规范化；像素与 metadata 原子。
- [ ] snapshot token；同文档/几何/年龄校验；A/B 覆盖与去重。
- [ ] frame slot/receipt/header/payload 合同。
- 当前：`not_started`。生产仍是截图 PNG+计数；geometry 恒 0。

## AC-CW5 Host 生命周期与预览

- [ ] 唯一 frame reader/immutable cache；预览只读 latest，识别取 token。
- [ ] pool/run/task ID、append/rollback/旧事件过滤。
- [ ] Ready 真实；锁外 shutdown；取消/退出可见。
- [ ] 双实例不争写；API 响应性有界；长动作实况持续。
- 当前：`partial`。API 曾 >60s 无响应（`run-175718`），需作为 CW9 输入。

## AC-CW6 MaaEnd 适配

- [ ] 撤销全局 `AutoAltClick→Escape`；恢复通用 modifier+target click。
- [ ] 只对确认入口做薄 overlay；DailyRewards 菜单行为复验。
- [ ] legacy camera 接真实 relative；region/UID cache；能力准入。
- [ ] 启动/关闭/切账号云语义；schema/宿主接线同步。
- [ ] 本机 Win32/HDR/进程假设不触发；选项/资源与实际 profile 一致。
- 当前：`partial`。全局 Escape 与材料 §0.1 冲突，必须改。

## AC-CW7 第一条真实纵向闭环

- [ ] 记录真实 actuator/target/profile 身份。
- [ ] 冷启动（菜单/标题）→登录/排队→DailyRewards 子分支→Stop→重开。
- [ ] MapNavigator + AutoFight；后台隔离（前台未变/无 OS 焦点/无物理输入泄漏）。
- [ ] 邮件有/无场景。
- [ ] fake/假 SDK/固定 PNG/独立 Node 脚本不计。
- 当前：`partial`。仅 `run-172134` 从 Regional 界面启动成功；冷启动 `172301`/`174331` 失败；Stop/重开/导航/战斗未证。

## AC-FIX-RS

- [ ] callback 恰好一次，且不早于 native 使用结束。
- [ ] last clone/drop、record wrapper、Tasker retain、destroy 并发、create/connect 失败路径。
- [ ] destroy quiescence 有明确测试/观测；不把 100 次 Drop 当证明。
- [ ] 最新 upstream 可复现构建/测试；独立 PR，不夹带 CloudWeb。
- 当前：`partial`。patch `953ec24` 已有 DropProbe；record/Tasker/destroy 静默期未证。

## Gate 评审流程

1. 实施 Agent 提交候选 SHA + 证据包 + 未验证项。
2. validator 按对应 AC 逐项复读/复跑，输出 verified/partial/blocked。
3. reviewer 对代码/协议独立复核；与 validator 结论分开。
4. coordinator 主持 Gate：只接受有证据的项；未通过项保留，不合并、不省略。
5. Gate 通过后才允许下一阶段；一次成功不足以关闭 CW7/CW8。

## 当前总判定

| CW | 状态 | 主要缺口 |
| --- | --- | --- |
| CW0 | partial | 构建/许可/实际配置 |
| CW1 | partial | 默认 fake、Host 未核验 |
| CW2 | partial | typed ledger/独立 renew/owned launcher |
| CW3 | partial | relative/contact/接管 |
| CW4 | not_started | rVFC/snapshot token |
| CW5 | partial | frame reader/预览隔离/API 响应性 |
| CW6 | partial | 全局 Escape 必改、region/legacy camera |
| CW7 | partial | 冷启动/Stop 重开/导航战斗/隔离 |
| FIX-RS | partial | record/Tasker/destroy quiescence/最新 upstream |

## 首批验收动作

1. 先做 CW0/CW1 静态验收（不依赖云会话），把缺口列成阻断项。
2. CW2 合同稳定后验收 typed ledger/renew/Stop 反例。
3. CW3/CW4 需真实 Windows 证据；合成/fixture 只能做预检。
4. CW7 由 integration + validator 联合；先冷启动，再 Stop/重开，再导航/战斗。
5. FIX-RS 交 native reviewer；独立 PR 后回正式依赖。
