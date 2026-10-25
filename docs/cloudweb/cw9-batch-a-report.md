# CloudWeb CW9-2 Batch A：控制面/租约协议故障首轮

状态：`partial`（15 行：6 pass / 4 partial / 3 fail / 2 blocked）。不关闭 CW9-2，不代表 G4。
范围：`fault-matrix.json` 的 D1（9 行）与 D2（6 行），关闭层级 `protocol_fixture`。
候选：sidecar HEAD `8b9cc6f` + tracked diff `4c775f51…`；测试日志 `cw9/batch-a/logs/sidecar-batch-a-tests.log` sha256 `e4c9356d…`。

## 1. 实际执行

- `cargo test -- --skip cdp_` → **40 passed / 0 failed / 3 filtered**，exit 0。
- 测试日志 sha256 `e4c9356d6f478645ff16c71e580ea817febb0084ac0a744bc1392921650978a3`。
- 结果明细：`cw9/batch-a/results.json`；行级状态只按本候选的实际测试/代码取证，不预填 Pass。

## 2. CW2 状态核对（已按 CW7 final audit 重评）

最新口径：**CW2 minimal stop pass；CW2 full contract partial**。

| CW2 项 | 当前证据 | 结论 |
| --- | --- | --- |
| 2 秒长按不被 350ms 误释放 | `two_second_long_press_survives_the_350ms_lease_with_renewals` 通过 | 已验证（协议层） |
| unknown down 后 Stop 尝试 up | `stop_releases_a_maybe_held_key/button...` 通过 | 已验证（协议层） |
| 独立续租 | MXU 线程 + sidecar `RenewLease`；2 秒长按通过 | 已验证（正向；未见续租丢失测试） |
| 真实云任务内 Stop | `autofight-20261024-104052`、`autofight-20261024-114725`：Running → `MaaTaskerPostStop` → `need_to_stop` → `inactive` → `ReleaseOwner` | **已验证（任务层）** |
| 长 RPC 进行中 Stop 立即抢占 | `src/transport.rs:866` 串行执行；Stop 要等当前命令完成；CW7 final audit 也注明 per-command-type 未复测 | **未证明（协议层）** |
| owned launcher/profile | sidecar 仍只 attach 外部 CDP endpoint | **未实现** |
| Host-death 回收 | 依赖 pipe EOF/60s idle；无 job object 与进程退出端到端证据 | **未验证** |

`cw2/integration-report.md` 的 `partial` 现在应读作“完整合同未闭合”，而不是“最小停止未通过”。`cw7/final-audit.md` 已把 minimal stop 升为 pass，但长 RPC 逐命令类型、owned launcher 与 Host-death 仍在。40/40 测试通过不能替代这些证据。

## 3. Batch A 行级结果

| 行 | 状态 | 证据/缺口 |
| --- | --- | --- |
| F-CP-01 down ACK 丢失 | blocked | sidecar 有 typed ledger/get_health；丢失回执是 Host 事件，需 MXU ACK-loss harness |
| F-CP-02 up ACK 丢失 | blocked | 只有失败释放重试测试；未注入丢失的回执 |
| F-CP-03 半包/超长/畸形帧 | pass | `oversized_and_malformed_control_frames_are_rejected`；解析失败发生在 dispatch 前 |
| F-CP-04 seq 错乱/重复旧命令 | partial | 过期 seq 不重放已测；`SeqConflict` 有代码无测试 |
| F-CP-05 旧 generation | pass | `owner_conflict_and_generation_guard` + lease tick 后 stale 拒绝 |
| F-CP-06 派发在途超时 | fail | 仅拒绝 `deadline_ms==0`；无在途 deadline 执行/测试（`src/session.rs:765`） |
| F-CP-07 慢 RPC 不阻塞续租/释放 | partial | 长按续租通过；串行控制面使 Stop 等待当前命令（`src/transport.rs:866`） |
| F-CP-08 pending 队列 >64 | fail | `max_pending=64` 已定义但无使用点；无队列/背压 |
| F-CP-09 Stop 时队列非空 | fail | 串行读命令→执行→回执；无 fence-first 队列清空 |
| F-LS-01 租约过期 | pass | `expired_lease_is_fenced_without_a_command` |
| F-LS-02 renew 丢失/延迟 | partial | 续租通过、过期 fence 通过；无专门 renew 丢失测试 |
| F-LS-03 owner 冲突 | pass | `owner_conflict_and_generation_guard` |
| F-LS-04 Host 管道 EOF | pass | `host_disconnect_releases_held_inputs_and_fences_generation` |
| F-LS-05 hello token 错误 | pass | `hello_rejects_wrong_token_then_accepts_valid_token` |
| F-LS-06 重连代次/token | partial | generation +1 与 held 清空已测；snapshot token 属 CW4 |

## 4. 关键未闭合

1. **控制面串行**：`src/transport.rs:866` 的 `spawn_blocking` 在写回执前不会读取下一条命令；慢截图会阻塞 Stop/ReleaseAll。这是 F-CP-07/09 与 CW2 的共同缺口。
2. **无在途 deadline**：`src/session.rs:765` 只处理 `deadline_ms == 0`；命令执行超过 deadline 后仍可能成功，没有 unknown/maybe-held 语义（F-CP-06）。
3. **无 pending/背压**：`src/protocol.rs:187` 的 `max_pending` 没有使用点；F-CP-08 无法通过。
4. **ACK 丢失属于 Host**：F-CP-01/02 不能在 sidecar 内伪测；需要 MXU 层的 unknown 账本与不重放证据。
5. **CW2 完整合同仍为 partial**：minimal stop 已通过，但上述 1 与 owned launcher、Host-death 回收仍需 CW2 owner 收口；在收口前，Batch A 不能标记完成，CW9-2 仍是 `partial`。

## 5. 边界

- 本轮只使用 fake actuator 与本地命名管道；没有启动真实云会话，没有注入真实浏览器/CDP 故障。
- 6 个 pass 只证明 sidecar 协议层行为；不证明真实云、后台隔离或 G4。
- 结果没有把 `cargo test` 的 exit 0 当完整功能通过；fail/blocked 行保留原状。
