# CW2 重新评估（2026-10-25）

结论：**CW2「最小停止」通过；CW2 完整合同仍为 partial。** 不能把“最小停止通过”写成“CW2 全部完成”。

候选：`d315fac6…`（CW7 最终候选）；sidecar HEAD `8b9cc6f` + diff `4c775f51…`；MXU HEAD `8bddfee` + diff `8feb31cd…`。
依据：`cw7/final-audit.md`、`cw7/completion-checklist.md`、`cw2/integration-report.md`、当前 sidecar 代码与测试。

## 1. 逐项判定

| CW2 验收项 | 判定 | 证据 / 缺口 |
| --- | --- | --- |
| typed confirmed∪maybe_held 账本 | **pass** | `cw2/integration-report.md` §1；Stop/ReleaseOwner/Close 遍历并集并尝试 up |
| unknown down 后 Stop 尝试 up | **pass** | `stop_releases_a_maybe_held_key/button_that_was_never_confirmed` |
| 2 秒长按不被 350ms lease 误释放 | **pass** | 真实 sidecar 进程 + MXU 续租线程；`health.held_keys=[65]` |
| 独立续租 | **pass（正向）** | MXU 独立续租线程；无专门 renew 丢失测试 |
| 任务内 Stop（真实云） | **pass（任务层）** | `autofight-20261024-104052`、`autofight-20261024-114725`：Running → `MaaTaskerPostStop` → `need_to_stop` → `inactive` → `ReleaseOwner` |
| 慢 RPC 进行中 Stop 立即抢占 | **未证明** | `src/transport.rs:866` 串行执行当前命令后才读 Stop；CW7 final audit 也注明“per-command-type long-RPC preemption not separately re-proved” |
| run/document/geometry fence | **partial** | generation fence 通过；document/geometry 归 CW4，最终候选仍未重跑完整 contract |
| owned launcher/profile | **未实现** | sidecar 仍 attach 外部 CDP endpoint；无产品级 Chrome/profile 启停与回收 |
| Host-death 进程回收 | **未验证** | 依赖 pipe EOF/60s idle；无 job object、无父死→退出的端到端证据 |
| 只回收自己的进程 | **partial** | MXU Drop 只回收自有 sidecar child；Host 强杀场景未闭环 |

## 2. 与旧报告的关系

- `cw2/integration-report.md`（候选 `391659fb…`）将 CW2 标为 `partial`，保留长 RPC 抢占、owned launcher、Host-death。
- `cw7/final-audit.md`（候选 `d315fac6…`）新增真实云 in-session Stop 证据，把 **CW2 minimal stop** 升级为 `pass`，但同一段仍写明长 RPC 逐命令类型未复测。
- 因此最新口径是：**minimal stop pass；full CW2 partial**。旧报告的“partial”不再表示最小停止未通过，而是表示完整合同未闭合。

## 3. 对 CW9 的影响

- Batch A：F-CP-07/09 仍不能因任务层 Stop 通过而升为 pass；协议层 Stop 仍要等串行控制面/在途命令收口。F-CP-01/02 仍 blocked（Host ACK harness）。
- Batch B：D3 的本地 Stop 测试可以开始；D6 的 HS-01/02/06/07 仍受 owned launcher/Host-death 影响，不能标记 pass。
- G1/G4：不因 CW2 minimal stop 通过而关闭；真实 Windows 双向隔离和完整故障矩阵仍未完成。

## 4. 建议

1. 把 CW2 在 roadmap 中标为 `partial（minimal stop pass）`，不再写“最小停止未验证”。
2. CW2 剩余项交回 CW2 owner：串行控制面 urgent path、在途 deadline、owned launcher、Host-death。
3. CW9-2 Batch A 重跑前，至少完成 urgent path 与在途 deadline；否则 F-CP-06/07/09 仍会 fail。
4. Batch B 可先做不依赖 owned launcher 的 D3/D4 本地测试；D6 进程生命周期行保持 pending。
