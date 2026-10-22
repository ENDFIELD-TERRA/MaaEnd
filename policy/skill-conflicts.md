# Skills 兼容裁决

固定审阅来源：MaaEnd v2@051d17f2（654aa962 时审阅，至 051d17f2 全部 16 份正文与技能 tree hash 零变化，见 research/upstream-audit-2026-10-23.md）；16份正文相对上一版未变，2份截图Python脚本随Python3.14升级。当前checkout更新后需重新核对。仓库与用户约束优先于技能示例。

## 必须由兼容层裁决的冲突

| 位置与短引文 | 与哪条规则冲突 | Qoder执行合同 |
| --- | --- | --- |
| perlica-style-reply：`角色一致性 > 正确性 > 有帮助` | 用户诚实与证据要求，且该skill另处要求不得牺牲准确性 | 人格只控制对话表达；事实、权限、仓库规则始终优先。保留原文快照，以上层Agent系统提示词明确覆盖此优先级。 |
| environment-monitoring-add-route：结尾直接 `pnpm check`、`pnpm test` | 根AGENTS“验证按需，默认交给CI” | 生成器和静态检查按范围执行；这两个全量套件服从 execution-contract.md 的测试矩阵，不能因skill清单而无条件运行。 |
| maaend-test-image：一律 `pnpm check`、`pnpm test` | 根AGENTS只在tests变动时test；并改assets/schema时check | tests-only至少test；还改assets/schema则加check；用户明确要求更广时按明确范围执行。 |
| autocollect-add-route：`focus.Node.Recognition.Succeeded`要求中文并给默认值 | 根AGENTS“不要主动生成…Pipeline的focus” | 默认不新增focus；已有focus保留，只有本次用户明确要求提示文案才添加。 |
| cpp-algo-style：`已知违规（修改时顺手修复）`、`必须提取的工具` | 用户只改目标相关文件；map-navigator-guide要求复用source/utils.h | 只修本次引入或直接阻塞的问题；先查当前源码能力。不要把指南中的历史问题清单当本次重构任务。 |
| cpp-algo-style/reference：示例成员无尾下划线，正文要求成员snake_case_；旧提取目录source/common与map-navigator的source/utils.h不同 | 同组指令内部冲突及旧示例漂移 | coding-standards、当前源码接口与更具体模块文档优先；必要时向用户报冲突，不复制过期示例。 |
| docs-guide：新增开发文档要求更新developers/README.md | 用户禁止主动创建/同步任何README | 可准备必要索引diff但暂不写README，使用交互提问取得这份README的明确授权；若本次只创建私人配置包，不修改上游docs，无需触发。 |
| maaend-test-image：`git…switch main`、detached修复 `merge --ff-only` | 用户禁止自动切分支/merge | 先只读审计子模块；需要移动分支时给出精确计划并交互确认。未授权只准备补丁与提交次序。 |
| issue-log-analysis：旧版本可能 `git checkout vXXX` | 用户基于审计基线且不得自动切分支 | 用git show、远端blob或独立只读检出核对旧版本，保护当前工作树；不自动checkout。 |
| windows-dmp-analysis：安装样例 `curl…\| bash` | 供应链审计与不在未审计代码上运行脚本 | 查工具存在性；缺失先获取固定版本和hash、审计安装方案，不直接执行管道脚本。Bash样例需Windows适配。 |
| issue-log-analysis：专项路径 `.agents/skills/tasks/` | 当前snapshot无此目录；专项skill实际平铺 | 枚举实际catalog，按日志原始入口唯一匹配现存skill；路径不存在就记录，不凭空创建tasks目录。 |
| skill脚本示例缺硬超时/进程盘点；部分示例 `2>/dev/null` | 用户所有命令硬超时、失败须原始stderr/退出码 | 调用时统一加有界超时、保留stderr与退出码；重型任务先盘点进程，结束收回自身资源。 |
| 各skill中的“提交/同步/上传截图”等动作建议 | 用户公开面闸、外部写入最后确认 | 本地生成可审查diff、脱敏与证据先完成；GitHub push/PR/Issue/评论、上传附件与任何外部写入逐项获得明确授权后执行。 |

没有skill可以自行授予提交、推送、PR、Issue或评论权限。公开issue读取与附件下载属只读；但DMP可能含私密内存，报告也必须经公开面检查。现存接口、目录中的 `agent` 等名称属于上游外部契约，按用户例外保留；新引入文件/符号/测试名/文案避免遗留命名。不要为消除禁词而改名既有MaaFramework API。


## Qoder安装与加载的完整性条件

1. 原始 `.agents/skills` 保持权威来源。项目级发现副本/链接使用 `.qoder/skills/<真实name>/`，16份全部登记到自定义定义的skills列表。配套reference/scripts/evals一同保留，不能只复制SKILL.md。
2. `windows-dmp-analysis`的安装目录若按name标准化，记录来源仍是dmp-analysis；上游交叉引用继续读取权威 `.agents/skills/dmp-analysis/SKILL.md`。
3. 环境监测mjs依赖相对../../../tools，项目 `.qoder/skills/name`与原三层深度相同；不要把脚本迁到用户级根后继续相对执行。`maaend-test-image`通过cwd/--repo/脚本父级寻找MaaEnd，执行时用项目cwd或明确--repo。
4. skill文内大量repo根相对路径不应全局替换；在MaaEnd checkout根调用。新增链接、junction或副本需检查Windows权限与同步漂移，安装器不可覆盖用户自改文件。
5. 全量“可发现/获准使用”与“正文每轮自动注入”是不同状态。每个新上下文完整读实际catalog的全部正文，当前为16份仓库技能+7份浏览器/方法/适配技能，共23份；给出逐项读取清单。后续按触发读reference，若发生漂移先重新同步和读取；不能声称列在skills即已加载。
6. 全局与repo AGENTS是否自动作用于子智能体不能只凭猜测；每个执行入口显式读取有效全局、repo及目标目录AGENTS，并引用本适配层冲突裁决。人格对外表达可默认启用，公开commit/PR/代码注释仍服从原仓风格。


## 浏览器技能适配

ChromeDevTools官方技能的latest与跨客户端配置示例不是本环境执行命令。固定版本优先，Qoder CN配置字段以其文档和实际schema为准。遇错误先读 .qoder/settings.local.json、当前服务日志和该技能references/upstream-troubleshooting.md；不扫描无关应用的个人配置。诊断日志先脱敏，不开启默认profile自动连接，不因工具缺失而关闭权限保护。

## e2e与gdp-ts方法适配

官方Skill原文保留；先读配对的 `maaend-e2e-validation` 或 `maaend-gdp-contracts` 及私人quality说明，再采用其示例。

| 原文示例或默认机制 | 本环境裁决 |
| --- | --- |
| e2e安装、最新包、模型配置与探索命令 | 使用本包固定发行及审计后的锁；不自动安装、更新浏览器或调用计费模型。先接通完整目标，再集中验证；前期只做解除阻塞所需的有界诊断。 |
| e2e并发、重试、cache、trace/video与telemetry默认值 | 首次资格验证单worker/零重试/cache关闭，限制session和时长；trace/video及上传另按敏感数据规则开启。不能把cache命中当当前界面重新通过。 |
| e2e MCP与CLI run、CDP attach | MCP勘察不等于测试run；Qoder模型不自动成为run模型。默认独立浏览器，不附着已登录云游戏会话；Windows官方WSL路线不产生Native/Windows资格。 |
| gdp-ts证明、cast示例和编译通过 | 仅在可信TS模块真实检查后mint proof，绑定精确run/build/suite，限制as/any与导出。编译不是数据源真实、授权、时效或TOCTOU证明；不为使用此方法把TS强加给原仓Go/C++/Rust。 |
| 私有proof/报告状态传递给队友 | 原生消息与私有报告同时核验候选和来源；proof对象不是跨进程凭据。真实reader未接通保持Pending，不造结果、不用原库4项测试代替业务validator。 |
