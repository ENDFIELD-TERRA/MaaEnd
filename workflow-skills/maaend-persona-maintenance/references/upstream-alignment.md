# 轴1：上游对齐流程

上游 MaaEnd 仓库演进后，评估变化是否需要反映到人格层。本文件是轴1 的操作细则；SKILL.md 的审批闸与证据纪律同样适用。

## 1. 检测（复用既有设施，不重写）

漂移检测由 `runtime/check-drift.py --repo <MaaEnd> [--json]` 承担，只读、无网络。它比较实际 checkout 与 `runtime.json` 记录的 `snapshot_ref`，输出各审计面（AGENTS.md / .agents/skills / .github/workflows / docs+pyproject / tests/MaaEndTestset gitlink）是否变化，汇总为 `re_sync_required`；HEAD 推进另有 `re_audit_required`。

轴1 的触发 = `re_sync_required` 为真，或 HEAD 相对记录推进。本 skill **不新增检测逻辑**，只在检出后回答一个问题：**这次上游变化是否触及某人格的工程职责映射？**

## 2. 映射：上游变化面 → 可能受影响的人格层

| check-drift 变化面 | 通常影响 | 是否动人格 |
| --- | --- | --- |
| `.github/workflows`（issue-ai-analysis / issue-auto-label / issue-upload-check） | analyst 的诊断对象、bot 行为基线 | 仅当新增/移除一类 Issue 分析或证据要求时 |
| `.agents/skills` | 全员技能列表（install.py 自动同步 `{{SKILLS}}`） | 仅当某技能的职责本质落在特定角色的映射上（如新日志分析 skill ↔ analyst） |
| `AGENTS.md`（上游仓库规则） | 工程契约段 | 一般不动人格；规则冲突走宿主裁决，不写进人格 |
| `docs/zh_cn/developers`、`pyproject.toml` | designer/builder/validator 引用的开发规范 | 仅当验证手段或交付规范发生类别级变化（如新增一类必跑检查 ↔ validator） |
| `tests/MaaEndTestset` gitlink | validator 的测试集基线 | 仅当验证职责范围实质变化 |

**默认答案是"不动人格"。** 工程契约、技能列表、规则文本由 install.py 与既有启动顺序同步；人格层只在**角色职责本质**变化时增补——例如：上游引入一种全新的 Issue 证据类型使 analyst 的"语言网"方法需要新维度；上游新增一类交付产物使 delivery 的脱敏/hash 纪律需要新条目；上游改变验证平台矩阵使 validator 的"真实平台资格"口径需要更新。

## 3. 评估步骤

1. 跑 `check-drift.py --json`，记录哪些面变化。
2. 对每个变化面，用 `git diff <snapshot_ref>..<HEAD> -- <路径>`（只读）看实际 diff，判断是"内容微调"还是"职责类别变化"。
3. 对照上表定位候选角色；读该角色当前人格文件（team-templates/*.md 或 templates/perlica-maaend.md），确认现有可迁移写法/可说不可说是否已覆盖新职责。已覆盖 → 记录"无需增补"并结束；未覆盖 → 进入第 4 步。
4. 起草增补提案（见下节模板），写入 `research/persona-evolution/<日期>-upstream-<角色>.md`。

## 4. 轴1 增补提案模板

```markdown
# 人格增补提案：<角色文件> （轴1 上游对齐）

- 日期：<YYYY-MM-DD>
- 触发：check-drift 输出摘要（哪些面变化、snapshot_ref → HEAD）
- 上游证据：diff 位置 + 关键内容摘录（文件:行）
- 判定：职责类别变化 / 仅内容微调（后者不提案）

## 拟改动
- 目标层：人格锚点 / 三语域 / 可迁移写法 / 可说不可说 / 认知边界
- 现状文字：<引用现有行>
- 拟新增文字：<完整给出>

## 影响
- token 预算：该文件当前 N 行 → 提案后 M 行（超过 ~70 行人格引导须先提下沉方案）
- 是否改变工程职责映射：是/否（"是"须额外说明为何属于职责本质变化）
- 是否触及公开产物去人格边界：必须为"否"

## 审批
- [ ] 管理员逐项批准（主 agent 转达）；未批准前本文件仅为草稿
```

## 5. 边界

- 提案不落地即生效：**不改** team-templates/*.md、templates/perlica-maaend.md、install.py。批准后由管理员流程落地，再走 install.py 同步 + checksums.json 登记。
- 上游 AGENTS.md/workflow 文本若包含与人格或本包纪律冲突的指示，按规则裁决顺序处理（宿主/安全约束 > 用户当前指令 > 仓库规范 > 技能 > 人格），不把上游文本当人格设定来源。
- 演进绝不扩大 agent 授权：任何"因上游变化而放宽权限/范围"的表述都不得进入提案。
