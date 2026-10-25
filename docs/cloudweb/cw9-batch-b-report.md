# CloudWeb CW9 Batch B 执行记录

状态：`partial`（round3：8 个 CDP 测试 + session MEDIA_STALLED 端到端通过；26 行 4 pass）。
候选：sidecar HEAD `8b9cc6f` + product diff `1f8625d5…`；MXU `8bddfee` + diff `e193e0d4…`。
测试载体：临时 test-only worktree；round2 harness diff `bb4f38aa…`（只含测试，不改产品代码）。
产品 diff 与候选一致；round1 harness `70805a91…` 绑定旧候选，仅作历史。

## round2（2026-10-25）

| 项 | 结果 |
| --- | --- |
| `cargo test --all-targets` | 52 passed / 0 failed / 1 ignored；日志 `../batch-a/logs/round2-sidecar-tests.log` `315b9cc0…` |
| CDP smoke | 1 passed（headless 9233）；日志 `logs/round2-cdp-smoke.log` `1e362969…` |
| 4 个 Batch B CDP 测试 | **4 passed / 0 failed**（headless 9234）；日志 `logs/round2-batch-b-cdp-tests-2.log` `3ae5145b…` |
| Session MEDIA_STALLED 端到端 | **1 passed / 0 failed**（headless 9235）；日志 `logs/round2-session-media-stalled.log` `2fd9450b…` |
| 首轮 round2 | 1 failed / 3 passed；原因：测试假设 `frame_sample.media.presented` 必须等于 rVFC `presented`；新候选截图明确标 `source=cdp_screenshot`，改为 source 检查后通过；产品未变。日志 `d2321578…` |
| 26 行 | round2 1 pass / 13 partial / 11 not_run / 1 blocked；round3 **4 pass / 11 partial / 10 not_run / 1 blocked**；round3 结果 `84389123…` |

round2 覆盖：F-MD-01（session 端到端 MEDIA_STALLED/恢复，**pass**）、F-MD-02/05（冻结/重复帧/截图不冒充 rVFC）、
F-MD-06（reload 改 doc_nonce）、F-BR-02（kill 显式错误）、F-BR-03（target close 显式错误）、F-BR-06（临时 profile/端口清理）。

## round3（2026-10-25）

| 测试 | 结果 | 映射 |
| --- | --- | --- |
| A/B frame slot self-consistency | pass：28 个样本，canonical 图像、nonce、geometry、presented 单调 | F-MD-03 partial |
| resize geometry | pass：`Emulation.setDeviceMetricsOverride` 后 revision **1 → 3** | **F-MD-07 pass** |
| CDP disconnect | pass：drop WS 后 input/capture 显式错误，新 actuator 可重连 | **F-BR-01 pass** |
| same-URL new page | pass：关闭旧 target→about:blank→CDP navigate→decoy tab；URL-scoped actuator 选新页，不选 decoy | **F-BR-03 pass** |

- 8 个 `batch_b_` 测试全过：日志 `logs/round3-batch-b-all-tests.log` `d48cc9bf…`；harness `harness-round3.diff` `eedf1815…`。
- 26 行：**4 pass（F-MD-01/07、F-BR-01/03）/ 11 partial / 10 not_run / 1 blocked**。
- A/B 仍是自洽 smoke，不是确定性 torn-read 注入；F-MD-03 保持 partial。

## round1（历史）

## 1. 执行

| 命令 | 结果 | 日志 |
| --- | --- | --- |
| `cargo test --all-targets` | 42 passed / 1 ignored / 0 failed；exit 0 | `logs/sidecar-all-targets.log` `dcd507ba…` |
| headless Chrome CDP smoke | 1 passed；临时 profile/端口清理 | `logs/cdp-smoke.log` `2df08ce0…` |
| 新增 4 个 CDP 测试（9232） | **4 passed / 0 failed**；exit 0；15.88s | `logs/batch-b-cdp-tests-2.log` `a06c27d2…` |

首轮失败记录保留：`logs/batch-b-cdp-tests.log` `c2e1ea84…`。失败原因是 harness 自身：
- 冻结时 `poll_cycle` 没有新帧，测试把它当错误；改为“有帧则断言 presented 不变，无帧也接受”。
- target close 后 HTTP 读取没有超时，测试挂起；给测试 HTTP 客户端加 2s 读写超时。
这两项都不归因产品；修复后第二轮全过。

## 2. 新增测试覆盖

| 测试 | 验证 | 映射行 | 行状态 |
| --- | --- | --- | --- |
| `batch_b_media_freeze_repeat_and_screenshot` | 暂停 video 后 `media_probe.presented` 不前进；冻结时 `frame_sample` 仍可成功但 media.presented 不变；恢复播放后 presented 从 1→3；重复帧若有 slot 则 presented 不变 | F-MD-01/02/05 | partial（缺 session `MEDIA_STALLED` 端到端） |
| `batch_b_page_reload_changes_doc_nonce` | `location.reload()` 后新连接的 `doc_nonce` 改变（示例 `fb47d8ca…` → `113ef22b…`） | F-MD-06 | partial（旧 token 失效未实现） |
| `batch_b_target_close_is_explicit` | 关闭 target 后 `frame_sample` 返回显式 CDP 错误，未回退到其它页面 | F-BR-03 | partial（同名页重建未测） |
| `batch_b_browser_kill_is_explicit` | 强杀临时 Chrome 后 `frame_sample` 返回显式 CDP 错误，无静默回退 | F-BR-02 | partial（session Faulted/未知账本未测） |
| CDP smoke（既有） | canonical JPEG、rVFC、doc/geometry、relative、非 canonical 空帧、临时 profile/端口清理 | F-MD-07/08、F-BR-06 | partial（schema 未填满） |

## 3. 当前 26 行分布

- **pass 0**：没有一行同时满足完整证据 schema。
- **partial 14**：D3 2（ST-01/02）、D4 7（MD-01/02/03/05/06/07/08）、D5 3（BR-02/03/06）、D6 2（HS-04/06）。
- **not_run 11**：D3 3（ST-03/04/05）、D5 1（BR-01）、D6 4（HS-01/02/05/07）、D8 3（LR-03/04/05）。
- **blocked 1**：F-MD-04（snapshot token 未实现）。

## 4. 未闭合

- F-MD-01 仍缺“暂停→session MEDIA_STALLED→UI 标识→恢复”的端到端；当前只有本地 rVFC 冻结 + session 单测。
- F-MD-02 的重复帧只在有 slot 帧时断言；无帧路径未形成结论。
- F-MD-03 A/B 覆盖、F-MD-07 resize 未跑。
- F-MD-04 旧 token 依赖 CW4 snapshot token，当前 blocked。
- F-BR-01 CDP 断链、F-BR-03 同名页重建、F-BR-02 session Faulted/残留账本未跑。
- D3/D6/D8 的 Stop 预算、Host 生命周期、跨 run/资源行仍未开始；HS-01/07 依赖 owned launcher/Host-death。

## 5. 边界

- 全部测试使用临时 headless Chrome（9232）与临时 profile；没有触碰用户 Chrome、云会话 9223 或 MaaEnd 产品代码。
- 4 passed 只证明 local_browser 层的 CDP 行为；不关闭任何 `real_cloud_session` 行，不代表 G4。
- 结果文件 `readiness.json` 仍标记 0 pass；下一轮把新增测试的证据字段补齐后再考虑升 pass。
