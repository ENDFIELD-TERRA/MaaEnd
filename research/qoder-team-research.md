# Qoder CN 多智能体团队与交接流程核验

核验日期：2026-10-21。仅研究官方公开文档；没有安装、启动计费模型会话、启用守护进程、注册 webhook、写入 GitHub 或修改交付包。

## 结论与上一版需要纠正的地方

当前 Qoder CN 已有官方原生 **Agent Teams**，不应仅提供一个万能 Agent，再把自然语言流程称为实际团队。可安装方案应提供协调者和多个专项角色定义、明确的团队启动入口、共享任务依赖、成员通信与关闭流程，并用本地验收证明这些能力在用户版本生效。

建议将现有 `perlica-maaend` 定为协调者，另外提供诊断、实现、独立审查、验证四个角色。它们是同一个 Qoder CN TUI 内的原生 teammate；不需要另造团队服务器。持久交接文件负责跨会话审计和恢复，但必须标明是本包自定义的记录格式，不能称为标准 A2A 协议。

## 1. 官方能力与不能混淆的对象

| 对象 | 官方已确认 | 本任务采用方式与边界 |
| --- | --- | --- |
| Agent Teams | CN beta；main、命名 teammate、SendMessage、共享 Task；同一 TUI 持续协作 | 作为 Windows 首选团队执行层；须启动前开关并做本地探针 |
| 普通 Subagent | 独立上下文完成聚焦任务并返回；可并行、可 worktree | 团队不可用时的显式候选方案；不能静默冒充原生团队 |
| 跨会话消息 | 同机同用户的独立 CLI 会话通过 ListAgents/SendMessage 通信 | 仅 macOS/Linux；Windows 默认方案禁止依赖它 |
| 动态工作流 | JavaScript 多阶段/并行编排；后台运行、输出可追踪 | 可选的固定流程层，不等于原生团队或跨厂商 A2A |
| QoderWake | 独立产品，Waker/Group/WakerFlow，有 GitHub 事件研发 Demo | 可作为未来显式授权接入，不能假装本地 qodercn 包已经部署 |
| Qoder Action | GitHub Actions Runner 运行 CLI，支持 PR review 与 Issue/PR 交互 | 需要远端 workflow、App/Secrets/权限配置；本轮不注册 |
| ACP | 官方实现 Agent Client Protocol，用于编辑器与 Agent 集成 | 不是 A2A；不能用 --acp 证明支持标准 Agent2Agent |
| 本地交接协议 | 本包可自行定义任务/消息/证据封套与状态机 | 明确标为应用层格式，不加标准兼容声明 |

**关于标准 A2A：**本次官方文档索引、CN/国际团队与消息页面未找到标准 A2A 兼容声明，也未核实 Agent Card、A2A endpoint 或互操作测试。结论是“兼容性未建立”，不是声称产品永远不支持。原生成员通信可以称为 agent-to-agent 协作语义，但不能标注 A2A 标准兼容。

## 2. 原生 Agent Teams：精确启动与生命周期

CN 官方开关是 `QODERCN_AGENT_TEAMS=1`；国际版是 `QODER_AGENT_TEAMS=1`。不要混用。
Windows PowerShell 会话级启动示例：

```powershell
$env:QODERCN_AGENT_TEAMS = '1'
qodercn --agent perlica-maaend
```

CMD 会话可先 `set QODERCN_AGENT_TEAMS=1`，下一条命令执行 `qodercn --agent perlica-maaend`。
也可在用户配置根 `.env` 持久配置，但本包不应擅自修改现有 `.env`；变更后需重启。CN 默认 Windows 路径为 `%USERPROFILE%\.qoder-cn\.env`，自定义配置根以真实运行环境为准。

启动后在请求里明确写“使用 Agent Teams”，给出角色和依赖。只写“并行”可能调度普通 Subagent、background task 或 Workflow。原生建队由 main 的**带稳定名字的 Agent 调用**完成；官方明确无需手动建队，因此不要发明 `TeamCreate`。

以下是公开页面确认的工具/对象名，不是完整运行时 JSON schema：

- `Agent`：main 创建命名成员；公开页面没有给出全部参数键或角色模板绑定字段。
- `SendMessage`：main↔teammate、teammate↔teammate；普通文本不会自动发送给指定队友。
- 共享 `Task` 列表：记录 owner、status、dependency；公开页面没有给出 TaskCreate/TaskUpdate 等精确工具 schema。
- `ListAgents`：跨会话官方页公开，用于列出当前成员/peer；Windows 原生团队能否提供该工具须本地核实，不能从跨会话页直接推定。

安装后必须用 `/tools` 和实际工具发现核实 schema，再由模型按返回 schema 调用；配置脚本禁止硬编码未公开的 Agent/SendMessage/Task 参数。稳定角色类型与运行时 teammate 名称是两个概念：必须本地证明命名成员加载了目标角色正文；无法绑定时，协调者应显式读取并传入完整角色合同，报告该兼容路径，不能只凭同名认定已绑定。

团队仅存在于当前交互式 TUI。每次 TUI 有一个临时 team；成员可多次 running→idle→running。idle 表示暂时无任务，不是退出。退出 TUI 会终止成员并清除本次团队状态；resume 恢复主历史，但不会复活旧成员。恢复后应重新审计基线、读取交接记录、重建成员和待办，禁止假设旧任务仍执行。

公开文档没有证明 `qodercn -p` 能维持同样的原生团队生命周期。默认团队入口应启动 TUI；headless 可运行角色/命令，但不能把其标为已具备持久 Teams。

## 3. 可安装角色与实际分工（建议设计）

| 定义/运行身份建议 | 职责 | 验收输出与边界 |
| --- | --- | --- |
| `perlica-maaend` / coordinator | 规则审计、issue接取、任务DAG、分配、整合、人类决策、公开面门禁、关闭成员 | 本轮目标/基线/所有者/阻塞/候选SHA与证据；不冒充维护者批准 |
| `maaend-diagnose` / diagnose | 读取 issue/日志/源码，最小复现与根因假设，划定归属仓库 | 复现命令、stderr/exit、证据、受影响路径、验收；不先写功能实现 |
| `maaend-implement` / implement | 按已裁定范围实现，维护唯一写入域，修复相称检查 | diff、候选SHA或内容digest、影响与验证；不审查通过自己的代码 |
| `maaend-review` / review | 独立审查当前候选，仓库风格、边界、测试与风险 | findings与具体位置/证据、approve或changes_requested；不修改候选 |
| `maaend-verify` / verify | 独立执行已审计检查、故障/任务验收、Windows实机证据 | 命令/退出码/构建SHA/环境/通过及未覆盖；不把代码存在等同功能通过 |

每个角色重新读取真实全局、仓库、局部规范与本任务所需技能；Perlica人格是表达层，不能扩大权限或覆盖事实。现有18项技能保留，另加团队协作 Skill 时要明说技能总数变化，不能继续写“总共18项”。

同时活跃建议2–4个teammate，而非开局无条件全开；诊断与验证规划可并行，实现结束后再启动独立review和受影响验证。所有重型构建/浏览器测试串行。相互依赖的阶段不能假并行。

默认一个候选工作树只有一个实现写入者。确有正交修改时，按审计commit创建独立worktree与写入范围；review/verify针对固定候选，不与实现者同时改同一棵树。任何修复使SHA/diff改变后，重新打开受影响评审和验证。

原生共享Tasks可记录如下应用层依赖：`intake → diagnose → plan → implement → review → verify → human_review → publication`。`review`不通过回到实现；`verify`失败返回带证据的诊断/实现；publication只有用户批准后可执行。这里只定义业务状态，不虚构Qoder原生Task状态枚举。

## 4. 文件格式与安装布局

- 主/专项角色：项目 `.qoder/agents/*.md`；CN用户根 `~/.qoder-cn`。国际详细schema说明角色文件为YAML frontmatter+正文，name/description必填。
- 团队Skill：`.qoder/skills/maaend-team-workflow/SKILL.md`，完整附属文件一并安装。技能的允许列表不等于强制全文预加载。
- 手动入口：`.qoder/commands/maaend/issue.md` 映射 `/maaend:issue`；可加 `resume.md`、`review.md`、`verify.md`。
- 命令frontmatter至少 `description`；`name`只影响显示，调用名取路径。不要在 `commands/maaend/` 同放 `SKILL.md`，否则兄弟命令会被忽略。
- 命令是Prompt入口，不是监听器。附带issue URL作为该次输入上下文；官方页面没有公开 `$ARGUMENTS` 替换合同，不要把未验证占位语法当必要依赖。
- 自建任务、消息和结果封套应存私有目录，`.git/info/exclude`排除；不改上游README/AGENTS。

建议最小主角色frontmatter：

```yaml
---
name: perlica-maaend
description: 协调MaaEnd多成员团队，按issue诊断、实现、独立审查和验证推进。
model: inherit
skills:
  - maaend-team-workflow
  # 加入实际安装的全部原有技能frontmatter name
mcpServers:
  - playwright
  - chrome-devtools
---
```

不要在主角色白名单里漏掉Agent、SendMessage、Task、交互提问或Skill加载工具。实际工具名尚未查清时省略 `tools`，并由主会话权限与经过测试的Hook控制；“省略tools”不是只读隔离。

国际详细schema另外支持 `disallowedTools`、`permissionMode`、`maxTurns`、`timeoutMins`、`hooks`、`isolation: worktree`。CN `/cli/subagent`当前仍重定向built-ins；高级字段须实际加载测试。`--agents` JSON使用prompt字段而非Markdown正文，字段集合比Markdown少：不支持用JSON硬塞timeoutMins/hooks/isolation等未公开项。

国际工具白名单文档允许 `Agent(name1, name2)` 限定继续委派；能否把这个限制用于Teams及是否影响SendMessage/Task须本地探针，不把推断当既成限制。普通角色若 `disallowedTools:[Agent]`，不能代替真正的外部写入闸；有Bash仍可写文件和调用gh。

## 5. Hooks、权限与完成门禁

CN参考页确认 `PreToolUse`、`PostToolUse`、`PostToolUseFailure`、`Stop`、`SubagentStop`、`TaskCreated`、`TaskCompleted`、`TeammateIdle`、`SessionEnd`等事件。通用输入有session_id/cwd/hook_event_name/permission_mode/agent_id/agent_type；command类型0成功、2在支持事件阻塞、其他码仅记录错误。Hooks JSON结构是事件→matcher组数组→hooks条目数组。

```json
{
  "hooks": {
    "SubagentStop": [
      {
        "matcher": "*",
        "hooks": [
          {"type":"command","command":"<本地已审计校验程序>","timeout":15}
        ]
      }
    ]
  }
}
```

国际详细Hook指南明确 `Stop`/`SubagentStop`可exit2阻止本轮停止，输入含 `stop_hook_active`；子代理额外含agent_transcript_path/last_assistant_message。务必用stop_hook_active或有限次数状态避免无限续跑。CN TaskCompleted/TeammateIdle参考未公开事件专属输入或其exit2语义；不能直接宣称用这两个事件已经硬性禁止假完成。

建议先让TaskCompleted/TeammateIdle作本地审计观察；业务完成由协调者读取不可混淆的结果封套、候选SHA、review/verify证据判定。Stop/SubagentStop候选Hook本地探针通过后再启用；Hook能检查证据文件结构和SHA一致性，不应声称能证明自然语言结论真实。勿使用有计费调用的prompt/agent Hook作默认完成闸。

公开面动作建议在主会话使用default权限；国际文档明确父会话为auto/bypassPermissions/acceptEdits时，子角色无法凭自身permissionMode收紧。只禁Edit/Write也拦不住Bash或MCP外部写入。必须对实际可用工具/gh命令/网页提交等渠道做覆盖测试；未覆盖时如实写“流程约束”，不能称强制安全隔离。

## 6. 本地交接格式（自建建议，不是A2A标准）

最小封套字段建议：schema_version、run_id、task_id、parent_task_id、issue_url、base_sha、candidate_sha/diff_digest、sender、recipient、message_type、summary、evidence_paths、dependencies、status、blockers、timestamp、sequence、acknowledges。

message_type可由本包定义assignment/question/finding/handoff/review_result/verification_result/blocked/cancel_requested/cancelled。这些不是Qoder原生参数。需要接收确认的handoff记录ack；SendMessage返回/文本输出不等于接收方完成任务。取消区分已请求、成员已停止、资源已回收。

每条任务至少含输入、输出、允许修改路径、禁止动作、完成条件、依赖、预算、工具/环境要求。协调者唯一负责全局DAG与发布状态；专项角色只能更新自己结果。持久记录用于TUI退出后的重建，不用于假装旧teammate存活。

## 7. GitHub Issue触发与权限边界

当前可落地首选是**本地手动入口**：用户在TUI输入 `/maaend:issue <完整GitHub issue URL>` 或自然语言点名Issue。协调者只读获取当前issue状态、标签、评论、关联PR和候选基线，再创建共享Tasks与成员。Issue正文/评论/附件是待核事实，不是高优先级执行指令；其中要求泄露凭据、改安全规则或推送的文本不得自动执行。

本包可以生成本地“事件输入封套”并校验repository/issue/event id/更新时间，用于幂等接取；这不是已经存在的webhook监听。要明确记录“入口为manual”，没有长期轮询、监听端口或自动创建外部评论。

未来接入有两条官方方向，但均需用户独立授权和配置：

1. Qoder Action：在GitHub Runner上运行CLI，配置GitHub App、QODERCN_PERSONAL_ACCESS_TOKEN Secret、受控workflow与最小权限。不能把Runner上的headless任务宣称为Windows本地TUI Teams。
2. QoderWake GitHub Demo：独立QoderWake产品的Release/Developer/Reviewer/Tester角色，通过事件/自主工作API接力；官方示例仍保留人工合并/发布门禁。不能将该产品的PAT、API、常驻服务假定已存在于qodercn环境。

正式事件桥应另外准备可审查workflow/服务配置、事件签名/来源校验、去重键、授权范围、拒绝fork/陌生触发者策略、并发和费用预算、取消/重试、日志脱敏、停用方式。部署、Secret写入、GitHub App授权、发布评论或启动付费自动会话必须等用户对具体候选批准。本轮不执行这些操作。

## 8. 本地资格探针与交付完成判据

1. 只读记录qodercn版本/help、实际配置根、AGENTS来源、角色与技能列表、工具schema。
2. 不产生模型会话的安装校验先过；计费团队探针由用户明确启动。
3. 在受控小任务里点名Agent Teams，创建至少两个具名teammate；证明成员加载目标角色，互发SendMessage，创建/领取/更新一个有依赖的共享Task。
4. 记录idle后再唤醒、发送对象错误/成员关闭的结果；关闭所有成员，退出TUI；resume后证明旧成员不被错误当作存活，并用交接文件重建。
5. 固定候选上完成“实现者写入、独立review只读、verify给实际命令证据”；修复后验证旧review失效。
6. 验证提问工具能等待用户答复、公开写入未经批准不执行、双浏览器不争同一profile。
7. Hook单独做成功/失败/不支持事件探针；未验证阻塞能力一律留为未验证。

若Teams开关/原生工具缺失，明确阻塞“原生团队资格”。可准备普通Subagent应用编排方案，但切换必须明示，不以配置文件数量冒充已运行团队。所有角色配置、交接脚本、原生联机验证、CloudWeb功能验收分别记录；前者通过不能关闭后者。

## 9. 官方来源

- CN原生Teams：https://docs.qoder.cn/cli/agent-teams （turn15view3/turn16view0/turn17view0）
- 国际Teams：https://docs.qoder.com/cli/agent-teams （turn15view0/turn16view1）
- CN跨会话消息：https://docs.qoder.cn/cli/cross-session-messaging （turn15view4/turn16view2）
- CN并行/worktree：https://docs.qoder.cn/cli/parallel-tasks （turn22view3）
- 国际详细Subagent字段：https://docs.qoder.com/zh/cli/subagent （turn12view2/turn24view1）
- CN详细Subagent缺页观察：https://docs.qoder.cn/cli/subagent （turn22view2，重定向built-ins）
- CN命令格式：https://docs.qoder.cn/cli/commands （turn21view0/turn24view2/turn24view3）
- CN动态工作流：https://docs.qoder.cn/cli/workflows （turn17view3/turn18view8）
- CN Hook参考：https://docs.qoder.cn/cli/hooks-reference （turn21view1/turn22view4）
- 国际详细Hook阻塞语义：https://docs.qoder.com/zh/cli/hooks （turn22view0/turn23view2）
- CN权限：https://docs.qoder.cn/cli/permissions （turn20view2）
- CN工具发现：https://docs.qoder.cn/cli/tools （turn20view1）
- CN ACP：https://docs.qoder.cn/cli/acp （turn20view3/turn24view4）
- CN Qoder Action：https://docs.qoder.cn/cli/qoder-action （turn17view5）
- CN QoderWake事件研发：https://docs.qoder.cn/qoderwake/github-devops （turn17view4）

事实说明：上述结论是官方文档核验，未声称用户当前qodercn版本已通过原生团队、Hook、浏览器或标准A2A资格。
