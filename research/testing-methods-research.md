# e2e 与 gdp-ts 研究结论

日期：2026-10-21。采用 GitHub plugin 只读审计固定提交，未安装依赖、未调用模型、未启动浏览器、未执行测试。完整源码证据见同目录 `testing-methods-sources.json`，文件包含主分支审计与发行候选两个角色，不能混为同一版本。

## 结论

`tester-army/e2e` 是建立在 Playwright/agent-device 之上的测试 runner、自然语言执行器与 MCP；它可作为完整集成后的网页交互验证层，不能替代 MaaEnd 节点图像测试、Windows native callback、停止屏障、全局输入隔离或真实云游戏验收。`rauchg/gdp-ts` 是 TypeScript 的前置条件证明库，不是浏览器或 E2E 框架；没有页面协议、截图、MCP 或云会话生命周期能力。可借它表达经过运行时验证的授权/候选构建/测试套件关系，但不能把其 proof 当成运行时安全沙箱或完成凭证。

实施顺序：先完整接通主代理、委派角色、交接协议、技能和工具适配，再集中运行选定候选构建上的验证。禁止为了先制造绿色测试而先开发平行假应用、合成媒体或与真实接入脱节的大量测试。

## 仓库基线与供应链

| 项目 | 主分支审计 | 可运行候选与许可 | 约束 |
| --- | --- | --- | --- |
| tester-army/e2e | main `d7c6b5e5a4c27fac7633e66866fd367b0351ead8` | 正式 tag `e2e@0.21.2`，commit `393005aa6952eaef62886cd03c8d8060f8f05a4f`；runner 0.21.2、web 0.15.2；Apache-2.0 | release web 使用 playwright-core 1.64.0，而当前 main 已到 1.64.1；不能拿 main 修复/文档冒充 release 能力 |
| rauchg/gdp-ts | main `0b6ebeeec827b05210f3acfa4149a70dda9a0de5` | package.json 声明 `@gdp-ts/core@0.1.0`；MIT；本包使用固定源码，不声称 npm 已发布 | GitHub releases 空，tags refs 查询404，npm页面403/registry不可访问；发布状态未独立证实，禁止默认 npx 安装或声称已装好 |

两个锁文件均是 pnpm lockfile v9，带 integrity，未发现锁文件中的直接 URL/外部 tarball host。e2e root 使用 pnpm 12.3.4；gdp root 使用 pnpm 12.1.0。两者都不能拿 root 的 pnpm check/test 作为 MaaEnd 的测试标准。e2e 专用 CI 有 Linux Node 22.22.3/24.8.0 floor 与 22/24/26矩阵、macOS移动任务，没有 Windows原生 CI；gdp CI是Ubuntu+Node24、TypeScript5.4/5.5/5.6/5.9/6矩阵及TS7主检查，无Windows端到端证明。

e2e提供发布依赖 install-scripts检查，但这只能说明上游CI意图，不能免去消费端核验实际 npm tarball、integrity、transitive依赖和所需浏览器下载。gdp没有生产依赖；其 prepack 会运行 tsc，本包仅保存已审计 `src/index.ts` 与MIT，避免执行任何上游脚本。

来源：[e2e tag](https://github.com/tester-army/e2e/tree/393005aa6952eaef62886cd03c8d8060f8f05a4f)、[runner package](https://github.com/tester-army/e2e/blob/393005aa6952eaef62886cd03c8d8060f8f05a4f/packages/e2e/package.json)、[web package](https://github.com/tester-army/e2e/blob/393005aa6952eaef62886cd03c8d8060f8f05a4f/packages/web/package.json)、[主线AGENTS](https://github.com/tester-army/e2e/blob/d7c6b5e5a4c27fac7633e66866fd367b0351ead8/AGENTS.md)、[e2e CI](https://github.com/tester-army/e2e/blob/d7c6b5e5a4c27fac7633e66866fd367b0351ead8/.github/workflows/spec.yml)、[gdp package](https://github.com/rauchg/gdp-ts/blob/0b6ebeeec827b05210f3acfa4149a70dda9a0de5/package.json)、[gdp CI](https://github.com/rauchg/gdp-ts/blob/0b6ebeeec827b05210f3acfa4149a70dda9a0de5/.github/workflows/ci.yml)。

## e2e 的职责分工

1. 自然语言 `agent.act` 的执行器选择动作并给 verdict；runner负责屏幕脱敏、动作授权、预算、秘密填充、结果与清理。默认观察是语义树，并非每次给模型整张截图；需要时用视觉工具。`agent.assert`/`waitFor`/`extract` 的judge只看到当前屏幕、指令与context，不读acting模型的历史总结；可以独立选judge模型，但模型判断仍不是确定性真值。
2. 精确结果用普通 `expect`：源码 `expect/async.ts` 是有截止时间的重复读取和谓词比较；值断言基于Vitest matcher；截图比较基于pixelmatch。模型一句“成功”不能替代精确状态、媒体计数、运行时输入账本或候选构建绑定。
3. `agent.act` replay缓存依靠后续成功验证才提交；开始路径、可重定位目标、视口、结束锚点决定重放资格；不完整、歧义、变化与未知动作可回落到live executor。它不是将LLM步骤变成数学证明，也不能跨版本继承MaaEnd验收。首次资格与故障验收本包明确 `cache:'off'`，避免缓存遮蔽新构建行为。
4. e2e中的命名agents是测试执行器设置，不是Qoder子代理。Qoder自定义智能体的model与其`skills`不会自动配置e2e的AI SDK provider。`e2e mcp`本身不需要模型；含`agent.*`的`e2e run`需要显式独立模型或custom executor，可能另外计费。本包默认只用确定性API，不自动登录订阅或使用API key。

来源：[agent-steps](https://github.com/tester-army/e2e/blob/d7c6b5e5a4c27fac7633e66866fd367b0351ead8/docs/agent-steps.mdx)、[expect dispatcher](https://github.com/tester-army/e2e/blob/d7c6b5e5a4c27fac7633e66866fd367b0351ead8/packages/e2e/src/expect/index.ts)、[有界locator断言](https://github.com/tester-army/e2e/blob/d7c6b5e5a4c27fac7633e66866fd367b0351ead8/packages/e2e/src/expect/async.ts)、[缓存](https://github.com/tester-army/e2e/blob/d7c6b5e5a4c27fac7633e66866fd367b0351ead8/packages/e2e/src/agent/step-cache.ts)、[agents](https://github.com/tester-army/e2e/blob/d7c6b5e5a4c27fac7633e66866fd367b0351ead8/docs/agents.mdx)。

## MCP、run 与页面生命周期

正式版 `e2e mcp --config <file> --target <name> --headed --max-sessions 1` 提供四个顶层工具：`open_session`、`tools`、`call`、`close_session`。先打开会话，再通过`tools`读取真实工具schema，`call`调用catalog动作，最后关闭。MCP session不执行test文件，不写测试结果或replay记录；命令成功/工具握手成功不能当`e2e run`通过。

普通web引擎每worker共享浏览器进程，每attempt创建新BrowserContext。CDP路径使用`chromium.connectOverCDP`：普通attach仍创建新context；persistent恢复模式要求专属默认context，记录contextId与targetId，断线后只接受同浏览器同页面，已派发动作不重试。**初次使用已有页会先导航about:blank；persistent的下一attempt还要求新浏览器。** 因而不得把真实已登录云会话端口塞入候选配置并当成无扰动观察。`viewport:null`只是不仿真尺寸，并不能消除导航、focus、样式/媒体emulation或清理副作用。

默认MCP最多4会话，runner本地默认半CPU worker、CI默认retry1；本任务必须显式改为session1、workers1、retries0。Playwright MCP、Chrome DevTools MCP、e2e MCP和生产sidecar操纵同target时仍只有一个owner；开多个MCP并不会自动建立协作或隔离。

来源：[正式版MCP](https://github.com/tester-army/e2e/blob/393005aa6952eaef62886cd03c8d8060f8f05a4f/docs/reference/mcp.mdx)、[正式版配置](https://github.com/tester-army/e2e/blob/393005aa6952eaef62886cd03c8d8060f8f05a4f/docs/reference/config.mdx)、[BrowserConnection](https://github.com/tester-army/e2e/blob/d7c6b5e5a4c27fac7633e66866fd367b0351ead8/packages/web/src/browser-connection.ts)、[正式版AttemptSession](https://github.com/tester-army/e2e/blob/393005aa6952eaef62886cd03c8d8060f8f05a4f/packages/web/src/attempt-session.ts)、[正式版恢复身份检查](https://github.com/tester-army/e2e/blob/393005aa6952eaef62886cd03c8d8060f8f05a4f/packages/web/src/cdp-recovery.ts)。

## Windows、权限与费用

正式quickstart要求Node `^22.22.3 || >=24.8.0`，Windows在WSL中运行。WSL中的Linux浏览器可以验证网页，但不能据此宣布Windows宿主鼠标、前台窗口、Win32 DLL、Tauri WebView、GPU与媒体路径已通过。WSL访问Windows Chrome的CDP还要核验网络模式、loopback/防火墙、profile、进程归属与完整清理；此能力Pending，不能自动扩大监听为0.0.0.0来求接通。

开源runner不是订阅服务收费本身；agent模型调用、hosted browser/device provider可能计费。不要自动选择云provider、登录订阅或高并发。MCP/配置/test代码以runner的OS权限运行，不是沙箱；导航无origin allowlist，secret fill只校验字段/秘密身份，不校验目标origin。填secret后截图停止输出，但手动录屏**不脱敏**，因此本包不默认录屏，不把原始登录画面/trace公开。

Telemetry默认启用，设置 `E2E_TELEMETRY_DISABLED=1`；main的更新检查还识别`E2E_UPDATE_CHECK_DISABLED=1`，正式版未含同一更新检查实现时此变量仅为无害兼容项，不据此宣称全部网络流量被禁用。安装前读实际包所带文档，不能用disable telemetry冒称阻止模型/应用网络。

来源：[正式quickstart](https://github.com/tester-army/e2e/blob/393005aa6952eaef62886cd03c8d8060f8f05a4f/docs/quickstart.mdx)、[安全边界](https://github.com/tester-army/e2e/blob/d7c6b5e5a4c27fac7633e66866fd367b0351ead8/docs/reference/security.mdx)、[MCP录屏警告](https://github.com/tester-army/e2e/blob/d7c6b5e5a4c27fac7633e66866fd367b0351ead8/docs/mcp-server.mdx)。

## gdp-ts 的适用面与局限

真实API只有`name()`、`defineProof()`、`Named`、`Proof`等类型与辅助：`name`用回调限定新鲜静态名字；trusted proof模块执行真实检查后返回具体值的proof，敏感函数要求匹配同一组名字。TypeScript >=5.4；ESLint preset要求现有TS parser配置，Oxlint JS plugin目前alpha；目标lint floor是ESLint9/Oxlint1.86。

源码的运行时proof只是按kind共享的frozen对象，**不携带被证明对象、构建或套件值**；同kind对不同值的prove返回同一对象。冻结wrapper也不等于深冻结其value；proof还可能过期或遇到TOCTOU。不能跨进程序列化proof当授权票据，不能以静态类型替代运行时 owner/generation/target/build/suite/expiry 检查。`as`/`any`、ambient declarations、definite assignment、module augmentation等仍可能绕过，需lint与审查；已有open问题还在补语法绕过，不是完备证明系统。

可用位置：未来若sidecar/私有验证协调层实际采用TypeScript，可将经可信运行时读取器验证的授权与报告绑定到三组静态名字，减少“把A构建结果用于B构建”的诚实调用错误；MaaEnd Go/Rust/Cpp主仓不因这份研究添加TS库或迁移业务。运行时真实数据源适配未实现时，候选bridge必须明确抛错Pending，不能返回伪Passed。

来源：[gdp README](https://github.com/rauchg/gdp-ts/blob/0b6ebeeec827b05210f3acfa4149a70dda9a0de5/README.md)、[源码](https://github.com/rauchg/gdp-ts/blob/0b6ebeeec827b05210f3acfa4149a70dda9a0de5/src/index.ts)、[限制](https://github.com/rauchg/gdp-ts/blob/0b6ebeeec827b05210f3acfa4149a70dda9a0de5/skills/gdp-ts/references/limits.md)、[何处停止](https://github.com/rauchg/gdp-ts/blob/0b6ebeeec827b05210f3acfa4149a70dda9a0de5/skills/gdp-ts/references/where-to-stop.md)。

## 本包候选材料与验证门槛

`package/quality/e2e`是独立消费目录，固定runner/web版本，含真实公开API的配置和只读媒体/页面检查候选；缺真实目标、可见锚点、video选择器或build信息即失败，不使用合成图/静态PNG冒充。它只覆盖web目标页面与video持续解码，不能声称整个MaaEnd CloudWeb闭环通过。与native/sidecar真正连接后再扩展目标相关spec。

`package/quality/gdp`保存固定MIT源码、授权/构建/套件绑定proof示例、编译期负例和默认拒绝的运行时bridge。两项工作技能通过`.qoder/maaend-private/quality`读取材料，不复制到上游仓库业务源码，不默认安装依赖。

| 门槛 | 当前状态 | 可核实结束条件 |
| --- | --- | --- |
| 固定仓库源码/正式e2e候选API | 已核验 | 本报告链接、源码SHA与manifest对应 |
| 团队/交接/工具适配整体接通 | Pending，由主线程收口 | 主代理真实委派、交接、权限、工具发现与回收证据 |
| e2e依赖/浏览器消费端资格 | Pending | 用户授权后在隔离目录核验tarball/integrity、安装和握手 |
| Qoder CN → WSL e2e MCP | Pending | stdio工具表、真实target、会话关闭与进程回收 |
| e2e实际页面与媒体spec | Pending | 完整集成候选构建上的run结果、时间/构建/target/模式绑定 |
| gdp编译期示例 | Pending | 正例通过，未标注负例按指定错误失败；防绕过lint仍独立 |
| gdp运行时示例/可信bridge | Pending | 原库语义实验与真实验证器分别验收，不能互相替代 |
| MaaEnd native/节点/真实游戏验收 | Pending，未被两库替代 | 沿用FRD gates和当前仓库约束关闭 |

本轮不执行测试。可在全链路集成完成后运行的最小无安装实验，是使用已审计gdp源码与Node24的TS剥离，运行上游4项runtime测试；若有已审计预装TS再运行本包正/负例。它们只检验gdp语义，不标记MaaEnd功能通过。
