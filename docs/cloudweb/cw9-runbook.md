# CW9 Batch A/B runbook

状态：Batch A 首轮已执行；Batch B 已准备。以下命令均为**下一次执行**的可复现入口，不代表已通过。
候选：sidecar `8b9cc6f` + diff `4c775f51…`；MXU `8bddfee` + diff `8feb31cd…`；MaaEnd `c7fd1b25`。
禁止：用户日常 Chrome、云游戏 9223、`Page.bringToFront`、`SetForegroundWindow`、按进程名批量 kill。

## 1. 开工前检查

```bash
cd <workspace>/cloudweb-sidecar
git status --short
git rev-parse HEAD
git diff | sha256sum          # 期望 4c775f51...
```

确认没有不属于本任务的 Chrome 测试进程；Batch B 只使用临时 profile（默认 `%TEMP%/cloudweb-cdp-test-profile`）与端口 9231。

## 2. Batch A 重跑（协议层，15 行）

前置：CW2 owner 已收口 long-RPC urgent path 与在途 deadline，或明确接受 F-CP-06/07/09 继续 fail。

```bash
cd <workspace>/cloudweb-sidecar
(cargo test -- --skip cdp_ 2>&1; echo "EXIT:$?") \
  | tee <case>/cw9/batch-a/logs/sidecar-batch-a-rerun.log
```

然后逐行对照 `fault-matrix.json` 的 D1/D2：

- F-CP-01/02 需要 MXU Host ACK-loss harness；sidecar 测试不能代替。
- F-CP-06 看 `deadline_ms` 是否在途生效；当前仅拒绝 0。
- F-CP-07/09 看控制面是否不再串行等待当前命令。
- F-CP-08 看 `max_pending` 是否真正使用并返回 Backpressure。
- F-LS-* 主要复用 `cargo test` 中的 lease/owner/disconnect/token 测试。

结果写入 `cw9/batch-a/results.json` 的新一轮对象；不要覆盖首轮，保留历史。

## 3. Batch B 准备/执行（local_browser，26 行）

### 3.1 全量回归

```bash
cd <workspace>/cloudweb-sidecar
(cargo test --all-targets 2>&1; echo "EXIT:$?") \
  | tee <case>/cw9/batch-b/logs/sidecar-all-targets-<round>.log
```

期望：42 passed / 1 ignored / 0 failed；ignored 为临时 Chrome CDP。

### 3.2 本地 headless Chrome smoke

```bash
CHROME_PATH='<chrome-path>' \
CLOUDWEB_CDP_TEST_HEADLESS=1 \
CLOUDWEB_CDP_TEST_PORT=9231 \
cargo test cdp::tests::cdp_actuator_screenshots_and_inputs -- --ignored --nocapture
```

期望：1 passed；结束时 `port_closed=true`、`temp_removed=Ok`、`profile_removed=Ok`。该 smoke 覆盖 canonical JPEG、rVFC Start/Stop、doc_nonce/geometry、relative 分段、trusted anchor、非 canonical 空帧（F-MD-08 的主要证据）。

### 3.3 Batch B 新增测试顺序

1. **B2 媒体**：冻结→`MEDIA_STALLED`；静止画面多帧仍 Fresh；A/B 覆盖原子性；reload/document nonce；resize/geometry revision。
2. **B3 CDP**：CDP 断链；临时 Chrome 强杀；target 关闭/同名页重建；临时 profile 不触碰用户 Chrome。
3. **B1 Stop**：本地持键→Stop 并集释放；释放失败→Faulted；长动作 cancel；页面死亡 close；Stop 预算 p50/p95。
4. **B5 跨 run/资源**：连续失败清理；run A 迟到事件注入 run B；close 后 callback/订阅/URL/进程归零。
5. **B4 Host 生命周期**：HS-01/02/04/05/06/07；HS-01/07 等 owned launcher/父死检测，未就绪前保持 pending。

每行执行后写 `cw9/batch-b/results.json`，字段与 `fault-matrix.json` 的证据 schema 一致；`readiness.json` 只记录准备度，不是结果。

## 4. 证据与判定

- 任何行 Pass 必须同时有：候选 SHA、真实注入、时间线、状态/ledger、进程清单、帧/媒体、UI/诊断、exit code、日志+sha256、unverified。
- Batch A/B 都是 `protocol_fixture`/`local_browser`，不能关闭 `real_cloud_session` 行（BR-04/05、HS-03、D7、LR-01/02/06）。
- 同类失败两次停线；不自动重试掩盖不确定性。
- 不修改 CW0–CW7/FIX-RS 实现；测试 harness 放独立 worktree 或本 case 目录，候选代码保持只读。
