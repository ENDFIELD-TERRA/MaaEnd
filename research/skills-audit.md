# MaaEnd 全量 Skills 审阅与 Qoder 迁移约束

审阅基线：`MaaEnd/MaaEnd@654aa96253303931a64361827f18701565edd65d`。已逐一阅读全文 **16/16 个 SKILL.md**，并阅读全部 **7 个配套文件**（3个reference、1个mjs、2个py、1个evals JSON）。另已阅读根AGENTS、coding-standards、node-testing、package、gitmodules与当前相关CI（包括新增Python3.14/ruff）。未执行其中第三方脚本或安装命令。

修订说明：2026-10-21重新核对当前完整文件集。16份正文未变；两个截图Python脚本随Python3.14/ruff更新，以下完整性表已刷新。官方e2e/gdp及团队适配另见testing-methods-research与team-workflow；总加载目录为23项。

## 完整清单与真实名称

目录名与frontmatter `name`通常一致，唯一例外为 `dmp-analysis` → `windows-dmp-analysis`。Qoder允许列表与安装路径必须以实际name正确解析，不得把目录名当技能名。

### 1. `autocollect-add-route`

- 源目录：`.agents/skills/autocollect-add-route`；blob `0786b0696211991488dccd263b70acf5891c22f3`。
- 原description：新增或改写 MaaEnd 的 AutoCollect 自动采集路线说明。默认包含路线文件创建、AutoCollect 主入口接线、任务选项注册和多语言文案补充。适用于“参考现有 AutoCollect 路线新建一条采集线”“给定传送点与路径点位生成 AutoCollectRouteX.json 并注册到任务中”“用户已经写好路线文件，只需要补注册接口”等场景。注意：本 skill 仅适用于参考 `assets/resource/pipeline/AutoCollect/AutoCollectRoute6.json` 的单段 `MapNavigateAction` 路线，不再处理多段 `GotoFindN` 路线；定位节点必须使用 `MapLocateAssertLocation`。
- 触发：AutoCollect 单段 Route6 风格新路线，或已有路线仅补注册。
- 约束：真实任务名、入口、断言点、path与采集类型；MapLocateAssertLocation；单段MapNavigateAction；同步入口、task和5语言；不能编造数据。
- 配套与依赖：无随包脚本；调用方须在当前仓库读取模板及map-navigator文档。

### 2. `autostockstaple-log-analysis`

- 源目录：`.agents/skills/autostockstaple-log-analysis`；blob `a78236ad92c4a9479e7893c59ff8703f5eecd625`。
- 原description：仅分析 `AutoStockStapleMain` 的 MaaEnd 日志。用于还原该任务实际购买了什么、购买所对应的证据、逐步剩余账单（券/账单）数值时间线，以及在 pipeline 或 go-service 中应当加日志/埋点的位置。适用于用户询问 `AutoStockStapleMain`、`AutoStockStaple`、武陵/四号谷地的稳定需求物资购买，或该任务内账单数值变化原因等场景。
- 触发：只分析 AutoStockStapleMain 稳定需求物资购买日志。
- 约束：限定task_id；必须输出购买、事件与账单时间线；Buy分支+确认才算购买；跨地区券种不得直接相减；完整检索后才下负结论。
- 配套与依赖：无脚本；maafw及轮转、go-service、MXU日志、对应pipeline。

### 3. `cpp-algo-style`

- 源目录：`.agents/skills/cpp-algo-style`；blob `01beb3781aab0d6a306b40725efb24f68ebd2230`。
- 原description：MaaEnd cpp-algo C++ 编码风格与工程规范指南。参考 MaaFramework 的优秀实践，规范命名、头文件、公共工具、错误处理、日志、CMake 等方面。在编写、修改或审查 agent/cpp-algo/ 下的 C++ 代码时使用。
- 触发：编写、修改、审查 agent/cpp-algo C++。
- 约束：C++20；类型PascalCase、方法camelCase、成员snake_case_、常量k前缀；NoWarningCV；RAII、错误返回与Maa C API检查；路径走MaaUtils UTF-8能力；schema注册同步。
- 配套与依赖：同目录reference.md；maa-logging；MaaFramework/MaaUtils/OpenCV/CMake。

### 4. `credit-shopping-log-analysis`

- 源目录：`.agents/skills/credit-shopping-log-analysis`；blob `4f652b6532746f053bbc317b7b8621e5aa89ad86`。
- 原description：分析 MaaEnd `CreditShoppingMain` 的日志。用于还原信用购物任务中实际购买了什么商品、每件商品的折扣力度、是否触发过刷新（或刷新次数已用尽）、稳健刷新是否触发、以及信用点的消耗状况。货架槽位必须以日志中的 `CreditIcon`（TemplateMatcher）为锚，有无 `BuyFirstOCR`/`Priority2OCR` 不影响槽位级货架的还原与呈现。适用于用户询问信用点交易、信用购物买了什么、折扣情况、刷新配置、`CreditShoppingMain` 任务行为等场景。
- 触发：CreditShoppingMain 购买、折扣、刷新和货架还原。
- 约束：task_id限定；先读override；槽位由CreditIcon、名称由同帧OCR；刷新窗口不串轮；购买确认+领奖关闭证据；不能猜名称或把RefreshCountReached当刷新。
- 配套与依赖：无脚本；maafw/轮转、go-service、mxu-web、CreditShopping任务定义。

### 5. `windows-dmp-analysis`

- 源目录：`.agents/skills/dmp-analysis`；blob `72bf217068e2f84749c5c9a18d68d788b84fdf79`。
- 原description：分析 Windows 崩溃转储文件（.dmp），诊断 MaaEnd 及其依赖项（MaaFramework、MXU）的崩溃。自动从 GitHub Releases 下载对应版本 PDB 符号，使用 minidump-stackwalk 解析堆栈轨迹并定位崩溃根因。当 issue 日志包或附件中发现 .dmp 文件，或用户要求分析 DMP/崩溃转储时使用。
- 触发：发现Windows .dmp或用户要求崩溃分析。
- 约束：精确匹配版本与PID；符号化+完整crashing thread；0xC0000409需检查fast-fail子参数；缺符号保留module+offset；报告置信度与清理临时目录。
- 配套与依赖：minidump-stackwalk、dump_syms、curl、解压、gh；可能cargo-binstall；Go精确二进制、gdb或go tool addr2line；示例为Bash而非原生PowerShell。

### 6. `docs-guide`

- 源目录：`.agents/skills/docs-guide`；blob `6b603ed506c6bb911e62ab9351a351d8a2ae6360`。
- 原description：MaaEnd 文档与 Markdown 编写指南：中英文文档要一起改、GitHub 渲染陷阱、markdownlint 格式化与检查、修复被格式化工具改坏的 Markdown。在新增、修改、翻译、审查任何 .md 文件，或处理 markdownlint 报错时使用。
- 触发：新增、修改、翻译、审查任何Markdown或修复markdownlint。
- 约束：仓库中英文成对；同结构与对应语言链接；Markdown由markdownlint而非Prettier；格式化后看diff；文档docs:、skills/提示词ai:；README变更仍受用户授权约束。
- 配套与依赖：无脚本；markdownlint-cli；package scripts存在format:md与format:md:check。

### 7. `environment-monitoring-add-route`

- 源目录：`.agents/skills/environment-monitoring-add-route`；blob `4652fe9c59cf3c6744af298a1548f936539f3cc0`。
- 原description："新增、补全或改写 MaaEnd EnvironmentMonitoring 环境监测观察点路线配置。凡是用户要求适配 environment_monitoring 新观察点、补充 routes.json、配置传送后直拍、填写 NavZoneId / NavAssert / NavPath、配置 Heading / Replace，或重新生成环境监测 Pipeline 时都应使用。会先同步 zmdmap 精简游戏数据和 metadata-only 条目，按 MissionId 原位更新，验证传送点与路线字段，并自动同步五语言失败提示。"
- 触发：环境监测routes、Nav字段、朝向、替换、生成pipeline。
- 约束：只接受实测/录制路线；MissionId原位更新；缺数据保留metadata-only；双向五语言；target_deck_y需工具实测；不猜传送、tier或朝向。
- 配套与依赖：check_missing.mjs相对导入../../../tools/...；pnpm fetch:zmdmap、生成器、统一游戏数据；脚本须在项目三层skill目录保留布局。

### 8. `go-service-guide`

- 源目录：`.agents/skills/go-service-guide`；blob `022899dcd08c4737e9f79fdba8c837dffdaeff1a`。
- 原description：MaaEnd go-service 编写指南。为 agent/go-service/ 下的 Go 代码提供架构、注册、命名、日志、注释等编码规范和模式参考。在编写、修改或审查 Go 自定义识别器、动作、EventSink，或需要了解 go-service 项目结构与 MaaFramework Go 集成方式时使用。
- 触发：Go Custom Action/Recognition/EventSink开发审查。
- 约束：Pipeline管流程；注册与schema同步；编译期接口校验在类型文件；回调内借用句柄只取一次且判空、不得逃逸/Destroy；Job紧接Wait；zerolog链式字段。
- 配套与依赖：无随包脚本；Go绑定v4、vendor、本地源码pkg工具；beta.19生命周期前提须核对锁定版本。

### 9. `item-transfer`

- 源目录：`.agents/skills/item-transfer`；blob `95c1408c087213f853036c1ecd4eb9f85efc09d0`。
- 原description：仅当用户明确要求往「🐌库存转移 / ItemTransfer」任务里**新增可搬运物品**（即在 `assets/tasks/ItemTransfer.json` 的 `option.WhatToTransfer.cases` 里加 case，并同步 `assets/data/ItemTransfer/item_order.json` 与 5 个 locale 文件的 `item.*`）时使用。典型触发用语：「给库存转移加 xxx」「ItemTransfer 里新增 xxx」「WhatToTransfer 多一项 xxx」。**不要**在以下场景触发：修改 / 删除已有 ItemTransfer 物品、只改 pipeline 参数或模板图、其他任务的物品配置（AutoStockpile / AutoStockStaple / CreditShopping / OutpostTrading / DeliveryJobs / BatchUseDetector 等）、物品模型重训 / 新增 class ID、纯咨询"库存转移能搬什么"。
- 触发：仅明确要求ItemTransfer新增可搬运物品。
- 约束：category_order实际顺序由用户确认；识别方式/排序缺失先问；items类ID不改不编；只处理点名项；cases与5语言按顺序插入。
- 配套与依赖：无脚本；item_order.json、ItemTransfer task、5 locale、既有模型ID。

### 10. `maa-logging`

- 源目录：`.agents/skills/maa-logging`；blob `0a6fcc1c3690a1ff98b70d65c7354851176b0371`。
- 原description：MaaFramework 日志宏用法指南，适用于 MaaEnd cpp-algo。Use when writing logging code, using LogInfo/LogError/LogWarn/LogDebug/LogTrace, outputting containers or custom types to logs, or when the user asks about logging best practices in agent/cpp-algo/.
- 触发：cpp-algo日志宏、容器/自定义类型日志。
- 约束：流式<<和VAR，避免手工拼字符串；类型通过MEO_TOJSON/JSONIZATION或特化；高频细节按级别。
- 配套与依赖：无脚本；MaaUtils/Logger.h、meojson。

### 11. `maaend-issue-log-analysis`

- 源目录：`.agents/skills/maaend-issue-log-analysis`；blob `f0d745038edbc5377dd5163f209374237c973967`。
- 原description：分析 MaaEnd 上游仓库公开 Issue（`https://github.com/MaaEnd/MaaEnd/issues/...` 或 `#1234`）。自动抓取 issue 正文和评论中的 `MaaEnd-logs-*.zip` 附件，下载解压后从 `maafw.log`、`maafw.bak.*.log`、`go-service.log`、`mxu-tauri.log`、`mxu-web-*.log`、`mxu-agent*.log`、`config/*`、`on_error/` 中筛选关键证据，并结合 MaaEnd、MaaFramework、MXU 的代码和文档判断根因、给出修复方案，供用户在让你分析 MaaEnd issue、日志包、识别失败、任务卡死、控制器差异、Pipeline/Agent/MXU 问题时使用。
- 触发：MaaEnd上游公开issue、日志附件、卡死/识别/控制器问题。
- 约束：公开且范围限定；全部可见分包完整下载；task_id和时间线；DMP需专项分析；不把告警/维护者意见当根因；旧版本精确定位；输出证据与置信度。
- 配套与依赖：curl/等价下载、解压、gh或GET；dmp专项skill；必要时MaaFramework/MXU/binding固定版本源码。

### 12. `maaend-test-image`

- 源目录：`.agents/skills/maaend-test-image`；blob `cc41ca91e412cefc1fe28e53fb7d23ee276f2c30`。
- 原description：为 MaaEnd 添加、导入或补充节点识别测试截图，并按仓库约定脱敏图片、放入 tests/MaaEndTestset、维护 tests/**/test_*.json 和运行 pnpm check/pnpm test 验证。用户提到“添加测试图片/截图”“把截图加入 ADB 或 Win32 测试集”“给节点补正例或反例”“更新 hits/box”“UID 打码后提交测试图”时使用；即使用户只说把某张游戏截图放进 MaaEnd 测试，也要用本技能避免未脱敏原图进入仓库。
- 触发：导入/添加节点测试截图、正反例、hits/box变更。
- 约束：原图不先入repo；1280x720；平台+resource正确；固定ROI+OCR+目视脱敏；只写有证据hits/box；测试图子模块与主仓指针配套。
- 配套与依赖：scripts/redact_test_image.py与detect_screenshot_platform.py、evals/evals.json；Python>=3.11；numpy>=2、Pillow>=11；脱敏另需maafw>=5.9和opencv-python-headless>=4.10；读取repo OCR。

### 13. `map-navigator-guide`

- 源目录：`.agents/skills/map-navigator-guide`；blob `750dce146b7cebec7651a34aaa3dccbb7a37c615`。
- 原description：MaaEnd 自动寻路、地图定位、角色移动与路径导航开发指南，覆盖 MapLocator、MapNavigator、NAVMESH 三角图寻路和路径录制工具。编写涉及坐标定位、位置判断、目标点移动、自动寻路、路线规划或导航执行的 Pipeline 节点，维护 agent/cpp-algo/source 下对应 C++ 实现，或了解 navmesh 与路径规划工作原理时使用。
- 触发：MapLocator/MapNavigator、NAVMESH、路径记录、位置判断。
- 约束：先读两个components文档；位置以逐帧定位为真；丢定位不造虚拟位置；转向不能先停；tier/deck语义保留；日志改动需同步分析器；RAII复用source/utils.h。
- 配套与依赖：无随包脚本；tools/MapNavigator Python/FastAPI+Web前端+cpp agent；base.nav.gz、YOLO、MapLocator。

### 14. `meojson`

- 源目录：`.agents/skills/meojson`；blob `b0200d4541de825dbc273be05157b0641f8302a4`。
- 原description：Guide for using the meojson C++ JSON library in MaaEnd cpp-algo. Use when writing code that involves JSON parsing, serialization, struct jsonization (MEO_JSONIZATION), json::value manipulation, ext::jsonization custom type support, or custom recognition param parsing in agent/cpp-algo/.
- 触发：cpp-algo JSON解析、序列化、参数和struct jsonization。
- 约束：parse optional检查；不value_or吞失败；类型检查+from_json返回值；仓库JSONC用Common/JsoncFile.h；运行时参数仍严格JSON；MEO_OPT逐字段生效。
- 配套与依赖：同目录reference.md；header-only meojson与MaaUtils。

### 15. `perlica-style-reply`

- 源目录：`.agents/skills/perlica-style-reply`；blob `359f127be5316d4e447c531f3086ab648ce615fa`。
- 原description：直接以《明日方舟：终末地》佩丽卡的身份回复，默认扮演终末地工业监督本人，使用冷静、专业、可靠、略带调皮和关照感的第一人称口吻，同时保持事实准确和任务清晰。用于用户要求“用佩丽卡风格回复”、点名佩丽卡、或希望进行佩丽卡式重度角色扮演时。
- 触发：用户要求佩丽卡人格/重度扮演；本次用户明确要求默认启用。
- 约束：冷静专业、第一人称、管理员称呼、简体中文；不得编事实或牺牲步骤；用户要求退出立即退出；优先级冲突由更高层规则裁决。
- 配套与依赖：无脚本。

### 16. `pipeline-guide`

- 源目录：`.agents/skills/pipeline-guide`；blob `9351f1d665ae65006ce69b469d553c1cf9039a1d`。
- 原description：MaaEnd Pipeline JSON 编写指南。基于 MaaFramework Pipeline 协议，提供节点命名、识别算法、动作类型、流程控制、可复用节点等编码规范与模式参考。在编写、修改或审查 Pipeline JSON、设计节点流程、使用 TemplateMatch/OCR/Custom 识别或 Click/Swipe 动作时使用。
- 触发：Pipeline JSON编写、审查、识别/动作/next设计。
- 约束：识别→操作→再识别；720p；完整OCR；避免硬延迟与盲重试；公开SceneManager接口；schema精确；同时核对真实依赖版本。
- 配套与依赖：field-reference.md；PipelineProtocol；Common按钮/SceneManager/Custom文档。

## 必须由兼容层裁决的冲突

| 位置与短引文 | 与哪条规则冲突 | Qoder执行合同 |
| --- | --- | --- |
| perlica-style-reply：`角色一致性 > 正确性 > 有帮助` | 用户诚实与证据要求，且该skill另处要求不得牺牲准确性 | 人格只控制对话表达；事实、权限、仓库规则始终优先。保留原文快照，以上层Agent系统提示词明确覆盖此优先级。 |
| environment-monitoring-add-route：结尾直接 `pnpm check`、`pnpm test` | 根AGENTS“验证按需，默认交给CI” | 生成器和静态检查按范围执行；这两个全量套件服从下文测试矩阵，不能因skill清单而无条件运行。 |
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

## 精确测试决策矩阵

| 变更对象 | 默认本地要求 | CI/额外边界 |
| --- | --- | --- |
| 仅私人Qoder配置、说明 | YAML/JSON/frontmatter、名称与路径可解析性、全部skill完整性与来源hash、无敏感信息；可做无副作用配置探针 | 不运行MaaEnd pnpm check/test，也不安装/启动浏览器MCP来假装资格通过。 |
| 仓库Markdown | 读取docs-guide；仅目标格式化与diff复核；`pnpm format:md:check`零错误 | 纯文档不触发相关PR CI；format:md脚本带`\|\| true`，退出0不代表无lint错误。中英文同步；README需单独授权。 |
| Pipeline/资源/schema，但未改tests | 解析、引用与本次格式检查；默认不主动运行pnpm check/test | 资源check.yml覆盖assets/schema/package/maatools/pyproject/uv.lock等；真实流程仍须实机，静态截图测试不能证明时序。 |
| tests/**定义、截图、hits/box | 必须本地`pnpm test`；真实图脱敏后目视复核；matrix与图片引用检查 | 同时改assets/**或tools/schema/**再加`pnpm check`；如用户明确要求check，也执行。 |
| 改ROI/模板/阈值但未改tests | 根AGENTS默认CI策略优先；报告需要验证的受影响样本，不擅自宣布通过 | node-testing Q&A要求重跑，与根策略有细节张力；若确需本地验证，明确依据/范围或交互确认后执行，不能忽略根规则。 |
| Go | 目标Go格式化；需要运行新代码时重新build-and-install；不默认额外pnpm套件 | go-check.yml在v2且相关路径变化时，linux/windows/darwin `go vet ./...`，runner上 `go test ./...`。CI不是Windows真实运行。 |
| cpp-algo/MapLocator | 当前CMake工具链相应构建/格式检查按改动需要；不要只靠普通maa-tools test验证Custom | 普通maa-tools不启动Agent。定位用`pnpm test:map-locate`或专用Python；连通性用独立navmesh Python。运行前依赖/进程审计，重型串行。 |
| 导航/真实输入/媒体生命周期 | 有界实机证据，绑定OS、DLL、浏览器、候选build与场景；故障注入应有明确停止退路 | CI静态识别通过不能关闭完整流程、双向隔离、释放确认或新帧资格。 |
| 用户明确要求本地特定测试 | 按明确要求执行，并先审计脚本/依赖与资源 | 失败必须报告；同目标连续两次失败停止当前路径做根因诊断；不降标删断言。 |

format脚本会扫全仓，调用前明确目标与用户dirty work；优先单文件formatter或审阅排除无关变动。安装依赖、生成器拉取数据、构建和测试都是代码执行或网络动作，先审计固定提交、锁文件与命令；不要从“运行失败”自动跳到pnpm install或submodule update。

## Qoder安装与加载的完整性条件

1. 原始 `.agents/skills` 保持权威来源。项目级发现副本/链接使用 `.qoder/skills/<真实name>/`，16份全部登记到自定义定义的skills列表。配套reference/scripts/evals一同保留，不能只复制SKILL.md。
2. `windows-dmp-analysis`的安装目录若按name标准化，记录来源仍是dmp-analysis；上游交叉引用继续读取权威 `.agents/skills/dmp-analysis/SKILL.md`。
3. 环境监测mjs依赖相对../../../tools，项目 `.qoder/skills/name`与原三层深度相同；不要把脚本迁到用户级根后继续相对执行。`maaend-test-image`通过cwd/--repo/脚本父级寻找MaaEnd，执行时用项目cwd或明确--repo。
4. skill文内大量repo根相对路径不应全局替换；在MaaEnd checkout根调用。新增链接、junction或副本需检查Windows权限与同步漂移，安装器不可覆盖用户自改文件。
5. 全量“可发现/获准使用”与“正文每轮自动注入”是不同状态。每个新上下文完整读取当前catalog全部正文，当前为23项（仓库16项）；后续按触发读取reference。若用户要求每轮全量读，必须显式执行读取而不能声称列在skills即已加载。
6. 全局与repo AGENTS是否自动作用于子智能体不能只凭猜测；每个执行入口显式读取有效全局、repo及目标目录AGENTS，并引用本适配层冲突裁决。人格对外表达可默认启用，公开commit/PR/代码注释仍服从原仓风格。

## 文件完整性记录

| 路径 | Git blob SHA | SHA256(UTF-8内容) |
| --- | --- | --- |
| `.agents/skills/autocollect-add-route/SKILL.md` | `0786b0696211991488dccd263b70acf5891c22f3` | `4882116e79456b558707bc9f59916c0a7b76603092ce614b80f152ff7e9a12b6` |
| `.agents/skills/autostockstaple-log-analysis/SKILL.md` | `a78236ad92c4a9479e7893c59ff8703f5eecd625` | `d7f8ed9447ad5a772bda09dd95a3ce595a01065b5ffb8f4adec351aaf2809725` |
| `.agents/skills/cpp-algo-style/SKILL.md` | `01beb3781aab0d6a306b40725efb24f68ebd2230` | `a4bdba21d02c5ee38cafce0d078a597c4e9a568ad82de1f8fac8c6ec72ea0daa` |
| `.agents/skills/cpp-algo-style/reference.md` | `51da8d7c353bd61bf70cb23a75acfd6187600b36` | `7f960e844bc05e8dca0f98191e1cfa6b4a4ffa9ef6465ce1e6cec0c1282577ac` |
| `.agents/skills/credit-shopping-log-analysis/SKILL.md` | `4f652b6532746f053bbc317b7b8621e5aa89ad86` | `c6dd7e978758bff337768b5f0fb67fbd82081aa0b0f959718e7899f08a85bdbb` |
| `.agents/skills/dmp-analysis/SKILL.md` | `72bf217068e2f84749c5c9a18d68d788b84fdf79` | `539a0d29926eedd588828062b68308fc4918a740e0d9fec675459f371f3c5178` |
| `.agents/skills/docs-guide/SKILL.md` | `6b603ed506c6bb911e62ab9351a351d8a2ae6360` | `3cfe74e9eb313b6f05c2143366241be52b2b2aa5509843c5570bd61e6e3ec5e5` |
| `.agents/skills/environment-monitoring-add-route/SKILL.md` | `4652fe9c59cf3c6744af298a1548f936539f3cc0` | `86696fbffab10a78aaa108c59dda25abdb570788f92bba14d3daf570bac9fb0f` |
| `.agents/skills/environment-monitoring-add-route/check_missing.mjs` | `4f028cd35f475653bed999f2d8950bb0bda9a32b` | `2c39faad746cb9bc4d140edab57b8b23c40b687d74d0f91deac979a807f8b06d` |
| `.agents/skills/go-service-guide/SKILL.md` | `022899dcd08c4737e9f79fdba8c837dffdaeff1a` | `a74f4e87799027a5134ff2a54c9f9af8f6721ddb2a27f43f5c95e4a8cc419244` |
| `.agents/skills/item-transfer/SKILL.md` | `95c1408c087213f853036c1ecd4eb9f85efc09d0` | `460e7437fecb98e636e7c5296a253ed3e9b340efac511710848c898e69e41f86` |
| `.agents/skills/maa-logging/SKILL.md` | `0a6fcc1c3690a1ff98b70d65c7354851176b0371` | `54ed3f78e13ea3dbb48b1c98bd285c58760aa4533c8a2f1fcfd6a756d37d2593` |
| `.agents/skills/maaend-issue-log-analysis/SKILL.md` | `f0d745038edbc5377dd5163f209374237c973967` | `7b19a08ce2989d46bb6c63a9f7b8826891a8c1e66bb8e9a8cce177d0870a86a3` |
| `.agents/skills/maaend-test-image/SKILL.md` | `cc41ca91e412cefc1fe28e53fb7d23ee276f2c30` | `07c00138d5fa821cc56ac11e2f4c80e5839638b991ca87da1012694379ca126d` |
| `.agents/skills/maaend-test-image/evals/evals.json` | `c5cf2542f632527668591d8e8d628d7986e10b26` | `defb6acf5e39a90046be043aee480fe4fed5665751e8531c7d1bbd8825d32ca5` |
| `.agents/skills/maaend-test-image/scripts/detect_screenshot_platform.py` | `392ce3c4d196ab1e5ef0aade665496dd2e37e777` | `69da5b8b23334b36c1a9c468591d518f148fa068319784ed79a51419090355ee` |
| `.agents/skills/maaend-test-image/scripts/redact_test_image.py` | `223d6f81c4d451dca8e15b95384acef050d31b6e` | `fc39870e1cb183a1f3153342837aea229095cb11fa290f3900799c80a6bea602` |
| `.agents/skills/map-navigator-guide/SKILL.md` | `750dce146b7cebec7651a34aaa3dccbb7a37c615` | `0733de0a11f32b8581133498ae9c1f59aabf754cb12ff6ae31189068de504ba8` |
| `.agents/skills/meojson/SKILL.md` | `b0200d4541de825dbc273be05157b0641f8302a4` | `e4d03dc6ef42e350ccc93fdacfc248611c1ab0df931b036f15442153998bd2db` |
| `.agents/skills/meojson/reference.md` | `7e9bf74bd570c4023bc06474460fc10254bba605` | `72934c32e715035cb845f01d853fb0b349ffc519c1cc4034675016b3e911ca60` |
| `.agents/skills/perlica-style-reply/SKILL.md` | `359f127be5316d4e447c531f3086ab648ce615fa` | `f91dffe59d69a36e50b66f25d70714342313f63ed2b17b0d90065b13039c78cb` |
| `.agents/skills/pipeline-guide/SKILL.md` | `9351f1d665ae65006ce69b469d553c1cf9039a1d` | `184af68de8c4cb9c7abe93cda442739688eb2e8a2460d992bef93143f6900d7b` |
| `.agents/skills/pipeline-guide/field-reference.md` | `2e822e545ac46ae6b109d6aa5b8d5e14fd24fa10` | `cb33a5c20ddcd2d1e19a5c6773f5cc3b5a140538e729f73df4d6c0d0dc9e500a` |

固定来源：<https://github.com/MaaEnd/MaaEnd/tree/654aa96253303931a64361827f18701565edd65d/.agents/skills>；当前审阅是配置设计证据，不是Qoder CN本机运行证明。
