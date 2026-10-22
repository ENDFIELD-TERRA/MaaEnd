# 上游对齐审计 2026-10-23

范围：MaaEnd `654aa962..051d17f2`（37 提交）与外部依赖固定版本。方法：本地 git diff 全路径比对 + npm/GitHub Releases 实查 + Qoder CN 文档核对。由 sidecar 分支 `qoder-agentset` 首轮审计产出。

## MaaEnd 主仓（654aa962 → 051d17f2）

| 路径 | 结果 |
| --- | --- |
| AGENTS.md | 零变化 |
| .agents/skills（16 项） | 目录与 tree hash 逐项相同，零变化 |
| .github/workflows/issue-*.yml（3 项） | 零变化 |
| pyproject.toml / Python 3.14 / ruff | 零变化 |
| tests/MaaEndTestset gitlink | 零变化 |
| docs/zh_cn/developers | 仅 scene-manager.md +1 行（zh/en 同步）：新增场景 `SceneEnterMenuOperationalManualUmbralMonument`（影拓丰碑，对应 8e0670b5） |

结论：研究基线 654aa962 在规则/技能/workflow/工程要求层面仍与 051d17f2 完全一致；唯场景枚举表需在编写 SceneManager 相关 Pipeline 时以当前 checkout 为准。区间内全部 37 提交为业务功能/修复（影拓丰碑任务、AutoFight 战技设置、干员升级、MapLocator 修复等），不改变团队包假设。

## 外部依赖

| 依赖 | 固定 | 上游最新（2026-10-23 实查） | 决定 |
| --- | --- | --- | --- |
| tester-army/e2e | 393005aa（runner 0.21.2/web 0.15.2） | 最新 release 即 0.21.2（10-16）；main 领先约 20 个未发版提交 | 保持固定 |
| rauchg/gdp-ts | 0b6ebee | 即 main 最新（10-07）；npm 404 确认未发布 | 保持固定 |
| @playwright/mcp | 0.0.83 | 0.0.83（09-28） | 保持固定 |
| chrome-devtools-mcp | 1.11.0 | **1.12.0**（10-22，TaskRunner 并行工具调用、--watchConfig） | **已升级至 1.12.0**（install.py / examples/mcp.windows.json / 使用说明） |
| MistEO/ai-issue-analysis | 5c80167 | 即 main 最新（10-15） | 保持固定 |

## Qoder CN Agent Teams 文档核对（docs.qoder.cn/cli/agent-teams）

- 吻合：具名 teammate、SendMessage、shared Task list、`QODERCN_AGENT_TEAMS=1`（beta，默认不启用，可写入用户级 `.env`）。
- 修正：官方文档无"默认最多 3 名活跃"硬性上限，仅建议"大多数场景从 2 到 4 个成员开始更稳"。包内表述已改为"建议 2–4 名（beta，无硬性上限）"（templates/perlica-maaend.md、使用说明.md §2）。

## 本轮包内机制改进（随本审计落地）

1. `runtime/check-drift.py`（新增）：对照 runtime.json 的 snapshot_ref/recorded_head 只读检测上游漂移（skills/AGENTS/workflows/docs/pyproject/子模块 gitlink 分列），输出事实与 `re_sync_required`；这是此前"漂移检查仅靠提示词"缺陷的工具兜底。
2. `install.py`：runtime.json 新增 `snapshot_drift` 字段，安装时即记录固定路径相对 HEAD 的漂移事实；snapshot_ref 可经 `MAAEND_SNAPSHOT_REF` 环境变量推进（默认仍 654aa962，推进须伴随研究快照更新）。
3. `runtime/caseflow.py`：`.write-lock` 陈旧锁检测（>600s 报告锁龄并给出恢复指引，不再无限 BLOCKED 无提示）。
4. `commands/maaend/deliver.md`：补齐"使用 Agent Teams。"入口语，与其余 4 命令一致。
5. 测试在 Windows（Python 3.11.7）首次执行：20 项全部通过（1 项按设计 skip：Windows reparse/junction 资格属单独本地检查）。

## 已知未解决项（如实保留）

- handoff.schema.json 仍无代码校验器；qoder-teamflow 研究文档的 message_type 枚举与 schema 存在漂移，待下一轮统一。
- Start-Team.ps1 仍未在任何平台实际执行过；native Teams 运行时资格（成员绑定/SendMessage/共享 Task）仍 Pending。
- snapshot/ 目录仍为 654aa962 状态；推进 snapshot_ref 前不改动快照文件。
