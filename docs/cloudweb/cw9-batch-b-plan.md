# CloudWeb CW9 Batch B 准备（local_browser，26 行）

状态：`in_progress`（round3：8 个 CDP 测试 + session MEDIA_STALLED 端到端通过；26 行 4 pass）。
范围：`fault-matrix.json` 中 `local_browser` 的 26 行；不包含 `real_cloud_session` 行（Batch C）。
候选：sidecar HEAD `8b9cc6f` + diff `1f8625d5…`；MXU HEAD `8bddfee` + diff `e193e0d4…`。
前置：CW2 协议层已 closed（pass-with-boundary）；CW4/CW5 的本地媒体与 reader/cache 已接通；硬杀输入释放按 best-effort 记录。

## 1. 分组与行数

| 组 | 域 | 行数 | 内容 | 当前依赖 |
| --- | --- | ---: | --- | --- |
| B1 | D3 | 5 | F-ST-01..05：并集释放、释放失败、长动作 cancel、页面死亡 close、Stop 预算 | CW2 minimal stop |
| B2 | D4 | 8 | F-MD-01..08：冻结、重复帧、A/B、旧 token、截图成功但冻结、页面重建、geometry、非 canonical | CW4 |
| B3 | D5 | 4 | F-BR-01/02/03/06：CDP 断链、浏览器崩溃、目标页重建、自有/用户浏览器边界 | CW4/CW5 |
| B4 | D6 | 6 | F-HS-01/02/04/05/06/07：MXU/GUI 崩溃、control pipe、双实例、idle 回收、Host-death | CW5；HS-01/07 还需 owned launcher |
| B5 | D8 | 3 | F-LR-03/04/05：失败清理、跨 run 隔离、controller 关闭资源 | CW5 |

## 2. 环境与安全边界

- 只用**专属临时 Chrome/profile**：`CHROME_PATH`、`CLOUDWEB_CDP_TEST_HEADLESS=1`、`CLOUDWEB_CDP_TEST_PORT=9231`；不碰用户日常 Chrome，不使用 9223 云会话。
- 每个测试结束检查端口释放、临时 profile 删除、Chrome 子进程退出；只 kill 测试自己创建的 PID。
- 不在测试中使用 `Page.bringToFront`、`SetForegroundWindow`、`SetCursorPos` 或全局键鼠注入。
- 真实云行（BR-04/05、HS-03、D7、LR-01/02/06）留给 Batch C；Batch B 不启动新的云游戏会话。

## 3. 已有就绪证据

| 证据 | 结果 | 覆盖 |
| --- | --- | --- |
| `cargo test --all-targets`（新候选 sidecar `1f8625d5`） | **52 passed / 1 ignored / 0 failed，exit 0**；日志 `../batch-a/logs/round2-sidecar-tests.log` `315b9cc0…` | 协议/会话/transport 回归；ignored 为临时 Chrome CDP |
| `cdp_actuator_screenshots_and_inputs --ignored`（headless Chrome 9233） | **1 passed**；日志 `logs/round2-cdp-smoke.log` `1e362969…` | canonical JPEG、rVFC Start/Stop、doc_nonce/geometry、relative 分段、trusted anchor、非 canonical 空帧、临时 profile/端口清理 |
| Batch B 4 tests（headless Chrome 9234） | **4 passed / 0 failed，exit 0**；日志 `logs/round2-batch-b-cdp-tests-2.log` `3ae5145b…` | 冻结/重复帧/截图、page reload、target close、browser kill |
| Session MEDIA_STALLED 端到端 | **1 passed / 0 failed，exit 0**；日志 `logs/round2-session-media-stalled.log` `2fd9450b…` | 暂停→MEDIA_STALLED；恢复→Live 且输入 Dispatched |
| Round3 8 个 CDP tests | **8 passed / 0 failed，exit 0**；日志 `logs/round3-batch-b-all-tests.log` `d48cc9bf…` | A/B 自洽、resize、CDP 断链、same-URL 新页 + 既有 4 项 |
| 运行后清理 | `port_closed=true`、`temp_removed=Ok`、`profile_removed=Ok` | F-BR-06 的测试边界 |

注意：以上只是本地浏览器证据，不等于 26 行逐项 Pass；`fault-matrix.json` 仍全 Pending，行级结果见 `round2-results.json`。

## 4. 行级就绪表

| 行 | 就绪度 | 现有证据/缺口 | 下一测试 |
| --- | --- | --- | --- |
| F-ST-01 并集释放 | partial | session 的 maybe-held key/button 测试通过；未在真实本地浏览器上 Stop | 本地 Chrome 持键→Stop→检查 up 事件 |
| F-ST-02 释放失败 | partial | `a_failed_release_is_cleared_by_a_successful_retry`、`release_owner_does_not_revive_a_faulted_session` | 本地页不可达/actuator 失败注入 |
| F-ST-03 长动作 cancel | not_run | 需本地长动作 + fence | 本地长任务 cancel 计时 |
| F-ST-04 页面死亡 close | not_run | 需关闭 target 后 close | target close→close→未知账本 |
| F-ST-05 Stop 预算 | not_run | 需重复采样 p50/p95 | 本地 Stop 采样 |
| F-MD-01 冻结 | pass | round2 session 端到端：暂停→MEDIA_STALLED，恢复→Live+Dispatched | 已闭 |
| F-MD-02 重复帧 | partial | `still_scene_stays_live_and_fresh`（fake）；无本地浏览器重复取帧 | 本地静止画面多帧 |
| F-MD-03 A/B 覆盖 | partial | round3 28 个 A/B 样本自洽；无确定性 torn-read 注入 | 保持 partial |
| F-MD-04 旧 token | blocked | snapshot token 尚未实现 | 等 CW4 token |
| F-MD-05 截图成功但冻结 | partial | `frozen_media_rejects_input_and_recovers_on_progress`（fake） | 本地冻结 + screenshot RPC |
| F-MD-06 页面重建 | partial | `document_navigation_bumps_generation_and_unknowns_latest`（fake）；无真实刷新 | 本地页面 reload |
| F-MD-07 geometry | pass | round3 resize：revision 1 → 3 | 已闭 |
| F-MD-08 非 canonical | partial | CDP smoke non-canonical 空帧通过 | 补齐证据字段后可升 pass |
| F-BR-01 CDP 断链 | pass | round3 drop WS：input/capture 显式错误，重连成功 | 已闭 |
| F-BR-02 浏览器崩溃 | partial | round2：kill 临时 Chrome 后显式 CDP 错误；session Faulted/未知账本未测 | 补 session/ledger |
| F-BR-03 目标页重建 | pass | round3：close→about:blank→navigate→decoy；URL-scoped actuator 选新页，不选 decoy | 已闭 |
| F-BR-06 自有/用户浏览器边界 | partial | CDP smoke 临时 profile/端口清理通过 | 断言未触碰用户 Chrome |
| F-HS-01 MXU 崩溃 | not_run | 无进程树测试 | 强杀 MXU→sidecar 回收 |
| F-HS-02 GUI 崩溃 | not_run | 无 WebView 测试 | 杀 GUI→API 有界 |
| F-HS-04 control pipe 关闭 | partial | 历史 `run-172548` 输入；无受控复现 | 关闭 pipe→错误/清理 |
| F-HS-05 双实例 | not_run | 无 pool 身份测试 | 第二实例连接同 session |
| F-HS-06 idle 回收 | partial | 历史进程盘点；无自动回收测试 | 空闲→进程/profile 检查 |
| F-HS-07 Host-death | not_run | 依赖 owned launcher/父死检测 | 强杀父进程→sidecar 退出 |
| F-LR-03 失败清理 | not_run | 无连续失败测试 | 多轮失败→清理 |
| F-LR-04 跨 run 隔离 | not_run | 无迟到事件测试 | run A→run B 注入迟到回执 |
| F-LR-05 close 资源归零 | not_run | 无 callback/订阅/URL 计数 | close 后资源盘点 |

## 5. 执行顺序

1. **B2 媒体**：先补冻结/重复帧/A-B/resize/reload 测试；F-MD-08 先收口。
2. **B3 CDP**：断链、浏览器崩溃、目标页重建、自有/用户边界。
3. **B1 Stop**：并集释放、释放失败、cancel、页面死亡 close、预算采样。
4. **B5 跨 run/资源**：失败清理、跨 run 隔离、close 归零。
5. **B4 Host 生命周期**：HS-01/02/04/05/06/07；HS-01/07 等 owned launcher/父死检测，未就绪前保持 pending。

## 6. 证据与判定

- 每行必须写：candidate_sha、runtime、注入方式、run/document/geometry 身份、before_state、timeline、observed_state、process_inventory、frames、ui_diagnostic、exit_code、logs+sha256、unverified。
- `fixture`/`local_browser` 结果不能关闭 `real_cloud_session` 行；Batch B 的 pass 只进入 G4-pre，不代表真实云。
- 失败/不可复现如实保留；不得用 smoke 通过代替 26 行逐项结果。
- 同类失败两次停线；不自动重试、不按进程名批量 kill、不触碰用户浏览器。
