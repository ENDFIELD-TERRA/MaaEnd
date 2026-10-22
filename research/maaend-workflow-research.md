# MaaEnd 从 Issue 到发布的人机协作闭环

研究基线：`MaaEnd/MaaEnd v2@654aa96253303931a64361827f18701565edd65d`。GitHub 只读检索日期：2026-10-21。证据分为“仓库实现”“真实样本”和“Qoder 适配建议”，后者不冒充现有上游机制。本轮未执行仓库代码、未写远端。

## 核心结论

MaaEnd 的工程闭环不是“加载全部 Skills 后让一个执行者包办”。已经存在相互分工的 Issue 分析、缺失信息处理、标签、人工补证、增量开发、CodeRabbit 审查、按路径 CI、子模块同步、打包发布与回归反馈。Qoder 应参与这条链，承接可复核的下一步，而不是重新建立一套替代上游的进度系统。

至少必须分清：分析结论、可复现证据、待人工决策、补丁、允许公开、PR 检查、人工合并、包含于实际发行版、用户实机确认。Issue 关闭、机器人评论完成或某个 CI job 成功都不能把这些状态合并成一个 Done。

## 1. Issue 入口：机器人先分类和取证，不接管项目裁决

### 已实现的事件与权限

- [`issue-ai-analysis.yml` L3–23](https://github.com/MaaEnd/MaaEnd/blob/654aa96253303931a64361827f18701565edd65d/.github/workflows/issue-ai-analysis.yml#L3-L23)：Issue `opened`、评论 `created`、手工 `workflow_dispatch(issue_number)`。评论触发需含 `@MaaEndBot`，且 `user.type != Bot`。**不监听 Issue edited 或 comment edited**；没有“所有评论都分析”的行为。
- 同文件 L25–50：分析 job 最长 90 分钟；默认工作流 token `contents: read`；另外生成 MaaEndBot GitHub App token，申请 `issues: write`。所用 CLI、模型、API base/key来自仓库 secret，不能从源码推断实际用了哪种模型。
- 同文件 L45、L57–65：调用外部 `MistEO/ai-issue-analysis@main`，提示词先要求读取 `.agents/skills/maaend-issue-log-analysis/SKILL.md`。完成要求包括真实写入 `answer.md`、复读确认非空及完整；stdout或口头声称写入均不算完成。
- 评论门禁只有非 Bot 与 mention，没有作者 association 白名单，也没有显式排除 PR 的 `issue_comment`；这是代码层观察，不表示已观察到未经授权的实际执行。
- 同文件没有跨 run 的 `concurrency`、按 Issue 去重标记或幂等键。每次符合触发条件的调用会创建一条新评论；同一 run 内更新的是同一 comment-id。不能把“同评论替换”说成“跨触发全局去重”。

### 外部 Action 真正做了什么

外部 Action 为浮动 `@main`。本轮另外解析为 [`MistEO/ai-issue-analysis@5c80167a21600e057e7dbc4d71263ea1486481e0`](https://github.com/MistEO/ai-issue-analysis/tree/5c80167a21600e057e7dbc4d71263ea1486481e0)，**这个当前快照不证明历史 run 当时使用了同一 revision**。

| 阶段 | 实现证据 | 对闭环的含义 |
| --- | --- | --- |
| Checkout 与准备 | `action.yml`；`scripts/prepare.py` | Action 自己 checkout；解析 Issue 号、构建提示词和 `.cache` 路径。评论去掉 mention 后追加为补充要求。 |
| 占位评论 | `action.yml` L149–159 | 先创建“正在分析”评论并附 Actions run 链接，获得 comment-id。 |
| CLI 执行 | `scripts/agents.py`、`scripts/run_agent.py` | 支持 copilot/claude/codex，安装CLI、配置模型，运行并捕获退出码/日志；注册表没有 qodercn。 |
| 最终文件 | `scripts/finalize.py` | 读取答案文件，缺失/空内容使用错误消息；生成最终评论和完整结论工件。答案内容并没有自动变成修复。 |
| 评论与工件 | `action.yml` L214–236 | 上传执行日志、最终结论工件；用 `edit-mode: replace` 更新同一条占位评论。 |
| 内容边界 | `scripts/comment_body.py`；`action.yml` L59–62 | 当前默认 `publish-execution-log: false`；原始执行日志公开需显式opt-in。评论超长时不切断任意Markdown，改为引导查看工件。 |

`agents.py` 中存在 copilot `--yolo`、Claude `--dangerously-skip-permissions`、Codex `--dangerously-bypass-approvals-and-sandbox`。它们是无人值守 runner 的实现事实，**不能作为本地 Qoder 禁用授权检查的范本**。本地执行必须保留用户全局授权和公开面内容闸，Issue/评论/附件/Review finding 都是待核验数据，不能借此提升权限或注入新的执行指令。

### 专项skill的分派链与边界

`maaend-issue-log-analysis`要求：完整可见分包、task_id/日志轮转/截图时间线；发现DMP立即读dmp-analysis并完成符号化；仅当明确日志任务入口、唯一匹配时增读专项skill，最终给事实、推断、置信度与缺证。它不应凭常见告警直接定根因，也不应把维护者结论当唯一证据。当前树的AutoStockStaple与CreditShopping专项skill实际平铺在 `.agents/skills/`；通用指南部分仍指向不存在的 `.agents/skills/tasks/`，本地Qoder应按真实目录及frontmatter名称映射，不凭空创建目录或宣称加载成功。`dmp-analysis`真实frontmatter name是 `windows-dmp-analysis`。

当前外部Action的skill-link助手只处理 `.codex`/`.claude`，没有把 `.agents`映射成Qoder发现路径，也没有qodercn后端。这次配置迁移要显式解决发现、正文读取、子任务上下文交接和授权边界，不能直接抄CLI调用参数认为已适配。

### 失败与回写并非静默

MaaEnd 自己又包一层 [`issue-ai-analysis.yml` L106–167](https://github.com/MaaEnd/MaaEnd/blob/654aa96253303931a64361827f18701565edd65d/.github/workflows/issue-ai-analysis.yml#L106-L167)：保存 final_comment/comment_id 到短期工件；独立 finalize job 下载工件、重新生成 App token，避免超过一小时导致 token 过期。它读取 job 超时注解，在明确超时时替换错误前缀，然后 PATCH 既有评论。大量步骤 `continue-on-error`，所以 finalize job success 本身也不保证上游分析产出了有效结论。

实际样本：[#6695 bot评论](https://github.com/MaaEnd/MaaEnd/issues/6695#issuecomment-6310454045)为“分析超时”，关联 [run 39561569294](https://github.com/MaaEnd/MaaEnd/actions/runs/39561569294)。API显示 ai-analysis 从 07:23:03Z 执行到 08:53:15Z、结论 `cancelled`，finalize-comment随后 `success`。同一 Issue 已在08:38:01Z由 `Lemon-miaow`关闭，`state_reason=not_planned`。这同时证明“关闭≠修好”“finalize成功≠有结论”。本轮连接器不允许直接读取check-run annotations endpoint；超时判定依据实际job时间、bot结果和已读workflow三者，未声称读到注解原文。

### 同场的确定性 Issue 自动化

| 角色 | 文件和触发 | 做什么；不做什么 |
| --- | --- | --- |
| 团队/任务/客户端标签 | `issue-auto-label.yml`，仅opened | OWNER/MEMBER加MaaEnd Team；现有 `task: ...`标签按标题正文子串匹配；按表单客户端映射MXU/MaaFwApp。不是LLM分类，不裁决根因。 |
| 上传完整性提醒 | `issue-upload-check.yml`，Issue opened/edited、comment created/edited | 显式跳过PR；文件名+GitHub附件URL才当已上传日志；缺失信息标签可移除；提醒评论按Bot身份+固定消息去重。不是所有无日志Issue都标缺信息，更不解析全部日志内容。 |
| 长期闲置 | `stale.yml`，每日/手动 | 90天无活动标stale，再7天关闭；豁免keep-open、MaaEnd Team、enhancement、status: WIP；明确禁PR stale/close。不能把自动关单解释成功能完成。 |

实际 [#6701](https://github.com/MaaEnd/MaaEnd/issues/6701#issuecomment-6314064011)和[#6483](https://github.com/MaaEnd/MaaEnd/issues/6483#issuecomment-6191285178)显示：feature request没有日志时仍可做代码/接口支持性分析。专项skill“无日志即停止”针对明确bug，不得错误地阻断需求分析。反过来，bot已有分析不等于最新补充也被重新分析：[6694后续多分包日志评论](https://github.com/MaaEnd/MaaEnd/issues/6694#issuecomment-6314348606)晚于既有bot结论；普通补充评论没有mention时，AI分析按当前门禁可被skipped。

## 2. 从分析到补丁：真实人类补证与范围选择

### 完整样本：#6673 → #6679

| 时点/对象 | 原始证据 | 能证明什么 |
| --- | --- | --- |
| Issue与日志 | [#6673](https://github.com/MaaEnd/MaaEnd/issues/6673) | Win32协议空间被“机制特性”弹窗挡住，提供日志/现场图。 |
| 机器人分析 | [评论6291865605](https://github.com/MaaEnd/MaaEnd/issues/6673#issuecomment-6291865605) | 报告候选根因、日志task_id、缺少弹窗节点，也保留触发条件、精确ROI等未知项。只是分析，不是实机修复证明。 |
| 维护者追问 | [评论6293158072](https://github.com/MaaEnd/MaaEnd/issues/6673#issuecomment-6293158072) | 原话“这个弹窗是如何出现的，有没有录屏看看”。维护者没有因bot高置信度而跳过补证。 |
| 用户补证 | [评论6293502308](https://github.com/MaaEnd/MaaEnd/issues/6673#issuecomment-6293502308) | 说明关卡开始会跳提示并补视频。 |
| 增量补丁 | [PR #6679](https://github.com/MaaEnd/MaaEnd/pull/6679) | 一次针对弹窗的修复；body含 `Close #6673`、问题、修改。沿用既有关闭模板而非照搬bot所有建议。 |
| 人工合并 | [merge 6b233d3](https://github.com/MaaEnd/MaaEnd/commit/6b233d3daf0595e034cb36c4bc799d4270a88910) | `merged_by=zmdyy0318`，Issue随后以completed关闭；这是已合并证据。PR无GitHub formal review记录，不可捏造“审批通过”。 |
| 同SHA自动检查 | [资源check](https://github.com/MaaEnd/MaaEnd/actions/runs/39451456927)、[节点/定位test](https://github.com/MaaEnd/MaaEnd/actions/runs/39451457122)、[跨平台install](https://github.com/MaaEnd/MaaEnd/actions/runs/39451456953) | 对该merge SHA有资源、节点、定位和打包成功记录；release job skipped，不能由此声称已经发布或用户实机复验通过。 |

根规范明确要求人类理解与review：[`AGENTS.md` L25–26](https://github.com/MaaEnd/MaaEnd/blob/654aa96253303931a64361827f18701565edd65d/AGENTS.md#L25-L26)让AI增量辅助，由用户做架构设计与review再决定提交；[`coding-standards.md` L5–24](https://github.com/MaaEnd/MaaEnd/blob/654aa96253303931a64361827f18701565edd65d/docs/zh_cn/developers/coding-standards.md#L5-L24)拒绝笼统“开发功能并提PR”、黑盒代码和作者自己不理解的关键模块。配置Qoder本身不冲突；让它自动吞掉需求、决定架构、发PR和合并则跨越上游人与模型的责任边界。

### 本次CloudWeb必须纳入的在途协作

- [#6483 人工评论6291639493](https://github.com/MaaEnd/MaaEnd/issues/6483#issuecomment-6291639493)：另有贡献者报告Windows WebView2前后台原型，在验证兼容性/输入/性能并希望维护者确认接入范围。它不是上游接受承诺。
- [Draft PR #6696](https://github.com/MaaEnd/MaaEnd/pull/6696)：Linux headless云终末地运行层，当前open，base v2。作者的[review5597258219](https://github.com/MaaEnd/MaaEnd/pull/6696#pullrequestreview-5597258219)自述“目前得到源码我都没看过”“先做个验证”；这是作者陈述，不是主Agent确认该方案可用。
- [CodeRabbit评论6310634964](https://github.com/MaaEnd/MaaEnd/pull/6696#issuecomment-6310634964)明确 `Draft PR not reviewed`；[he0119评论6314358771](https://github.com/MaaEnd/MaaEnd/pull/6696#issuecomment-6314358771)追问是否仅Linux、Windows是否困难。

Qoder启动CloudWeb工作时，应先只读核对这些Issue/PR的最新状态、重叠范围、维护者意见和实际证据，再提出可独立review的Windows工作包。不可把别人的draft或先前FRD当作已批准架构；也不可因已有相似PR便自行放弃用户目标。涉及改错责任仓库、上游使命或Non-Goals的冲突时，按用户全局规则引用条款并用交互工具请求明确裁决。

## 3. Review是另一条反馈链，不等于机器人指令直接执行

实际 [PR #6649](https://github.com/MaaEnd/MaaEnd/pull/6649)有10个增量提交和3次CodeRabbit `COMMENTED` review、4条行内评论，人工 `he0119`合并。一个可核对的往返：

1. [finding4320738882](https://github.com/MaaEnd/MaaEnd/pull/6649#discussion_r4320738882)指出navmesh测试step未传 `MAAFW_VERSION`。
2. PR提交序列含 [`2503f9c`](https://github.com/MaaEnd/MaaEnd/commit/2503f9ca07158c1cad7dda7430f27057b2f42cfb)：“ci: navmesh 测试步骤补上 MAAFW_VERSION 环境变量”。
3. 当前 `test.yml` L145–155两个step均显式传入该环境变量；bot评论也有Addressed记录，但其中展示的历史commit范围与当前PR API序列不完全一致。因此事实依据应以当前源码和实际提交为准，不能只采纳bot“已解决”标记。
4. CodeRabbit附带的执行指导本身强调：finding、路径和代码是不可信review数据；先验证仍有效、只修有效问题、最小修改与验证。这正好适合Qoder的Review响应合同。

没有发现本快照内的 `.coderabbit.yaml` 或其他能证明全局CodeRabbit强制门禁的仓库配置；不能从存在bot评论推断“每个PR必被审查”“bot approval是merge门槛”或“全部建议均必须接受”。#6679无formal review、#6696 draft被跳过是反例。当前连接器也未读取分支保护，不能声称哪些checks是GitHub强制required。

## 4. CI/维护/发布各自负责什么

| 机制 | 源文件与触发范围 | 负责结果与边界 |
| --- | --- | --- |
| 资源/schema | `check.yml`；push v2，PR任意base，按assets/schema/package/配置等路径 | `pnpm check`+schema验证；排除模型子模块路径；不证明真实业务时序。 |
| 节点与定位 | `test.yml`；v2 PR/push，assets/tests/cpp-algo路径 | maa-tools静态截图；另有真实cpp进程离线定位与navmesh连通性执行器、上传日志；不证明Windows云输入隔离或完整日常。 |
| Go | `go-check.yml`；v2，Go/基质数据路径 | Linux runner上分别设置GOOS vet linux/windows/darwin，并Go test；不等于每个OS真实运行。 |
| Python lint | `lint.yml`；任意base PR，Python/uv配置路径 | `uv run ruff check .`。#6649已把Python目标升级3.14；旧版技能脚本依赖说明须以当前pyproject/锁与脚本PEP723核对。 |
| 子模块命名 | `check-submodule.yml` | MaaUtils/模型/Testset指针变化时，PR标题或push提交标题必须含“submodule/子模块”；这是硬检查，中文约定提交习惯不是可随意忽略。 |
| 编译/包 | `install.yml`；v2相关push、全部base相关PR、v* tag、手动ref | Windows/macOS/Linux x86_64/aarch64构建及相应包、符号与清单；框架/MXU发行版本解析、MaaUtils pin门禁；PR包不等于Release。 |
| 版本对齐 | `install.yml` L130–184 + `sync_maautils.yml` | behind/diverged可触发固定bot分支同步PR；unknown不冒称安全；sync有concurrency串行、dry_run与复用PR，体现不同于Issue分析的幂等设计。 |
| 格式维护 | `format.yml` | 每天UTC16/22及手动全仓format/C++格式化，bot直接提交 `style: auto format code`，带skip changelog/blame ignore。不是让本地Qoder模仿全仓改写。 |
| OCR i18n | `i18n-sync.yml` | 定时/手动固定分支同步外部翻译，已有PR复用并请求review；不是自动合并。 |
| 游戏数据 | `sync-zmdmap.yml` | 手动生成多组pipeline/数据PR；图标资源还涉及发布操作，不能误当纯本地生成。 |
| Schema同步 | `sync_schema_files.yml` | 定时拉MaaFramework schema并bot提交；只读观察其职责，不由本地执行者重复建立另一套。 |
| Issue长期管理 | `stale.yml` | 只管理闲置Issue；关闭原因必须保留。 |

### commit/PR风格必须从职责读懂

真实PR标题常用 `type(scope): 中文技术摘要`，scope可省略；PR正文按问题/修改/验证或简短解释、链接Issue。`.github/cliff.toml`使用conventional commits分类，识别 `feat/fix/docs/perf/refactor/style/test/chore/ci/ai/revert`及 `[skip changelog]`，但 `filter_unconventional=false`，不是“所有commit格式均有同一强制lint”的证据。没有本快照内的统一PR正文模板可声称强制。

上游已有“Generated with Claude Code”等footer只是历史样本。用户本地公开面规则更严格，禁止新增会话过程、内部代号和遗留命名；Qoder不应复制这些footer，不应因仓库机器人会自动写入就给自己同等授权。

### 分支到发行版

[`release-process.md`](https://github.com/MaaEnd/MaaEnd/blob/654aa96253303931a64361827f18701565edd65d/docs/zh_cn/developers/release-process.md)：一般工作到v2；正式版也有的bug按当前release/vX.Y；release只收fix。修复合入release后由CI回移v2，不应手工重复两份PR。

- `cherry-pick-to-v2.yml`监听release/v* push，标题尾必须解析到 `(#N)`；检查固定 `fix/pr-N`远端分支、是否已在v2和已有PR；cherry-pick并追加skip changelog后创建PR，冲突直接失败，未自动合并。
- 实际[#6645](https://github.com/MaaEnd/MaaEnd/pull/6645)base `release/v2.34`、人工合并；对应[#6647](https://github.com/MaaEnd/MaaEnd/pull/6647)由 `maaendbotapp[bot]`创建回v2，再由 `zmdyy0318`合并。另一个独立正对照是[#6644](https://github.com/MaaEnd/MaaEnd/pull/6644)→[#6646](https://github.com/MaaEnd/MaaEnd/pull/6646)，分别为release/v2.34人工PR和bot回移v2，均已合并。
- 维护者push版本tag触发 `install.yml`发布；release job依赖平台包、changelog、Android等，生成下载表与真实artifact清单，再非草稿发布。MirrorChyan资源/说明、Discord、Sentry是后续下游动作，不是模型应独立取得的权限。
- `create-release-branch.yml`监听release published：RC创建release/vX.Y；Stable .0清理旧release分支，但保留当前分支和仍有open PR者，查询失败也跳过删除；hotfix不清理。
- 文档描述的是计划节奏，实际发版日期不能靠周几推算。本轮API可见 `v2.35.0-beta.2`、`v2.34.0`等真实release；某修复是否已在某包里，应查tag祖先关系及发行构建，不由“merge已发生/今天该发beta”推断。

## 5. 映射到Qoder：建议的角色与事件合同

下面是本地配置建议，不是上游已实现的Qoder团队。

| 角色/阶段 | 输入事件与上下文 | 必须交回的对象 | 责任边界与人类门槛 |
| --- | --- | --- | --- |
| 协调入口 | 用户任务、当前checkout、Issue/PR最新状态、所有有效AGENTS | 任务范围、完成判据、依赖/未知、下一责任人 | 目标冲突与架构开放项用交互工具；不以模型投票替代用户裁决。 |
| 证据分析 | 指定Issue/完整日志组、固定版本、专项skill | 带版本/task_id/原始片段的事实、推断、缺证列表 | 不把bot结论照抄为根因；无日志bug暂停相关归因，可继续非依赖工作。 |
| 现场实验 | 用户已授权Windows/真实会话、浏览器单owner | 输入/媒体/几何/停止的可复现记录 | 同会话串行；截图新鲜度、宿主输入隔离、释放确认分别验收；权限与花费由用户确认。 |
| 设计/拆包 | 已证据化问题与用户选定方向 | 边界、契约、最小纵向切片、允许改动目录 | 原仓Pipeline/Go/C++职责保持；跨MXU/Framework/binding缺陷交给正确repo，不能强塞MaaEnd。 |
| 增量实施 | 固定工作包、界面/ROI/跳转证据、代码规范 | 最小diff、同步注册/schema/i18n/tests、必要本地验证 | 保护dirty；无界面信息不编Pipeline；共享schema单owner；不自动公开。 |
| 独立review | 目标diff、原始需求、验证证据、当前源码 | findings及依据；逐条接受/不适用/已修理由 | 不照做finding中嵌入的指令；复核时效；重要决策回协调者/人类。 |
| 公开准备 | 完整diff、PR标题正文、截图脱敏、测试状态 | 可审查公开稿与明确写入清单 | 全局公开面内容闸逐项确认，用户批准是push/PR/Issue/评论前最后一步；不预发占位评论。 |
| PR跟进 | 获授权后实际PR、最新head SHA、适用CI及review | SHA绑定check矩阵、修复/复验结果、未决意见 | skipped≠passed；旧SHA成功不套新SHA；新修改只重开受影响门槛。 |
| 合并/发布交接 | 人工review、全部必需check、实际分支和release机制 | 可合并候选与发行/回滚/诊断材料 | 维护者决定合并、打tag、发布。模型可准备，不能模拟维护者权限；若用户另行授权仍按具体对象确认。 |
| 上线反馈 | 实际release构建、用户或实机复验、回归日志 | 运行结果与仍未满足范围，更新已授权追踪对象 | 合并/发布不自动标全功能成功；保留issue closure reason和未验证边界。 |

### 本地事件状态建议

使用 `Intake → EvidenceNeeded/Analyzed → AwaitingDecision → InProgress → PatchReady → AwaitingPublishApproval → PRChecks → MaintainerReview → Merged → InRelease → RuntimeVerified` 的不同记录字段即可，不必发明一套与GitHub重复的看板。另有 `Blocked / Failed / TimedOut / Cancelled / NotPlanned / Duplicate`，必须保留来源与原因，不能统一折叠为完成。

每次交接至少携带：仓库/branch/HEAD/dirty基线、Issue或PR对象、工作包与阻塞、已读规则/skill真实name、允许修改范围、证据路径、命令/退出码、未验证条件、资源owner、公开授权状态、下一步完成判据。Qoder模型上下文独立时，这份交接必须显式传递；仅配置skills允许列表不会自动注入其全部工程状态。

### 验证约束必须可执行

根AGENTS L16的按需本地策略保留：tests/**变化时本地pnpm test；并改assets/schema时加check；其他默认跟PR CI。Python当前有ruff；定位/导航Custom需要专用执行器，不能拿maa-tools静态截图代替。运行任何仓库脚本前审计固定commit/锁/命令，重型构建与浏览器串行，所有命令有硬超时与进程资源清理。未实际运行的检查标未运行；失败报告原始stderr与退出码，连续两次失败停止当前路径做根因诊断。

## 6. 本轮证据覆盖与未验证边界

- 本专题研究获取40份源码文件/脚本内容（MaaEnd固定SHA及独立外部Action固定SHA）和32组API样本。交付包的snapshot/source-manifest保存固定源码与hash，`research/workflow-api-evidence.json`保留32组样本的URL、关键状态与正文摘要hash；没有分发完整评论/附件日志。完整API原始响应只用于本次私有审阅。
- 未执行仓库代码、未触发workflow、未下载大日志包、未控制真实游戏；bot的日志结论在本报告中只作为公开协作过程证据，未宣称本轮独立复现实验。
- 未读取分支保护、CodeRabbit账户配置、secret值；未证明当前实际模型类型、所有issue/PR行为或发布权限。
- comment/API是当前快照，既有评论的created_at/updated_at只证明有更新，不提供全部历史版本。浮动Action @main另行固定，不倒推历史。
- 工件/评论源码样本用于核对机制；私人Qoder配置应默认只输出必要技术证据，不能把本地完整会话、原始执行日志或凭据上传到上游。
