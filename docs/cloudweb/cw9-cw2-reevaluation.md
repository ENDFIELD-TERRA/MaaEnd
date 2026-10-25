# CW2 重新评估（2026-10-25，round 2）

结论：**CW2 协议层最小停止已闭合（pass-with-boundary）**。此前列出的 urgent path、在途 deadline、
pending/backpressure、owned launcher、Host-death 五项已由新候选与独立复审收口；仅保留一个已记录边界：
Host 被 job object 硬杀瞬间的 held 输入释放仍是 best-effort。

候选：`6227c5a8…`（独立复审通过）；sidecar HEAD `8b9cc6f` + diff `1f8625d5…`；MXU HEAD `8bddfee`
+ diff `e193e0d4…`。当前部署的 overlay 增量（`9a3a8161…` 等）不改变 sidecar/MXU 字节，CW2 结论仍绑定
`6227c5a8…` 的 sidecar/MXU 组件。

## 1. 条款级状态

依据 `cw2/clause-table.md`、`cw2/review-lease.md`、`cw2/review-renew-sidecar.md`、
`cw2/review-combined.md`、`cw2/review-format.md`：

| # | 条款 | 状态 | 证据 |
| --- | --- | --- | --- |
| 1 | urgent 控制路径：长 RPC 中 Stop 不被普通命令阻塞 | **closed** | S1 读/执行分离 + urgent 独立车道 + S4 `preempt_flag`；真实云 60000 px 移动中 `ReleaseAll` RTT **8 ms**（修复后另测 43 ms），held 输入为空 |
| 2 | 在途 deadline：超时 Unknown、不重放 | **closed** | `deadline_exceeded_during_execution_returns_unknown_without_replay`、`dispatched_input_past_deadline_becomes_unknown_without_replay`、`queued_command_expired_before_dispatch_is_rejected` |
| 3 | `max_pending` 有界队列与 Backpressure | **closed** | normal 容量 `max_pending-1`、urgent 独立容量 8；`overfilled_pending_lane_returns_backpressure` |
| 4 | 进程/浏览器所有权 | **closed（attach-only 分支）** | `cw2/attach-only-policy.md`：Chrome/profile 归管理员；sidecar 只 attach；MXU 只回收自有 sidecar |
| 5 | Host 强杀：有界退出、无孤儿 | **closed（进程）**；输入释放 **best-effort** | MXU job object `KILL_ON_JOB_CLOSE`；hostdeath 两次 sidecar 143/158 ms 退出、leftovers 为空；job kill 可能跳过 lease tick/清理，已记录边界 |
| 6 | 真实云协议层 Stop 反例 | **closed** | `t5-mxu-20261024.log`：urgent ReleaseAll RTT 8/43 ms，移动被抢占，`held_*`/`possibly_held` 为空 |

## 2. 复审链

- `review-lease.md`：有界租约授予/续期边界 pass；51/0/1 测试。
- `review-renew-sidecar.md`：renew 按授予值续期、回执无消费方破坏 pass-with-notes；52/0/1。
- `review-renew.md`：MXU `set_renew_ms` 双源统一通过；测试 87/1/2（1 为既有环境失败）。
- `review-combined.md`：候选 `6227c5a8…` **pass-with-notes**，0 阻塞；hover move、observation 刷新、overlay 重建均通过。
- `review-format.md`：`393c82f7…` 格式-only pass；NB-1（prettier 键序）关闭。
- 当前 sidecar 自跑：`cargo test --all-targets` **52 passed / 0 failed / 1 ignored，exit 0**
  （日志 `batch-a/logs/round2-sidecar-tests.log` `315b9cc0…`）。

## 3. 保留边界

1. **硬杀瞬间输入释放 best-effort**：job object 是硬终止，可能跳过退出清理/lease tick。进程退出与
   无孤儿已证；若产品要求“强杀瞬间 held key 必然 up”，需在 job kill 前增加 release 窗口或专门用例，
   否则按已记录边界处理。
2. **attach-only**：不提供产品级 Chrome/profile 启动与回收；Chrome/profile 归管理员。若管理员改判
   为 owned launcher，CW2 条款 4/5 需重开。
3. **overlay 增量**：当前部署的 Trial 传送/菜单 overlay 改动未经 combined 复审，但不改变 CW2 的
   sidecar/MXU 字节；CW2 结论不覆盖这些 overlay。
4. **非阻塞 review 项**：NB-2（i18n-skip）、NB-3（双 move 承重件）、NB-4（命中偏移双源）等属交付/
   overlay 债务，不是 CW2 协议缺口。

## 4. 对 CW9 的影响

- Batch A round 2：F-CP-06/07/08/09 已在协议层转 pass；F-CP-01/02 仍需 Host lost-ACK harness；
  F-CP-04 的 SeqConflict 仍缺测试；F-LS-02 缺 renew-loss 注入；F-LS-06 token 半属 CW4。
- Batch B：D3 Stop/预算可基于新 urgent 路径重跑；HS-01/07 的进程退出证据已补，但硬杀输入释放
  仍按 best-effort 记录。
- G1/G4：CW2 本身不再阻塞；真实云隔离、CW4 token 和 46 项矩阵仍是后续门槛。
