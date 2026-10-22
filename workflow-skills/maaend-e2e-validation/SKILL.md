---
name: maaend-e2e-validation
description: 在 MaaEnd 完整工程、团队委派和工具链已接通后，选择 e2e 的真实网页资格检查、Qoder CN到WSL的MCP适配和集中验证；用于区分自然语言执行、确定性断言、媒体健康与原生验收。
---

# MaaEnd 网页验证

先读 [接入与收口约定](references/integration.md) 和 `.qoder/maaend-private/research/testing-methods-research.md`，再按需读官方 `e2e` 技能的 MCP、setup、running、writing-tests references。官方技能是参考，实际授权、当前主仓AGENTS、全局规则和本次“先接通、后集中验证”优先。

1. 定位工作区MaaEnd主仓根与 `.qoder/maaend-private/quality/e2e/运行与边界.md`，确认材料版本、目标/候选build/suite和owner。没有该文件则报告安装缺口，不在主仓临时拼一套替代框架。
2. 先完成整体接通：主代理真实委派、交接协议、工具发现、数据源、权限与资源回收。把尚未完成项记Pending，继续可独立实施部分；不先批量造测试、假应用或合成媒体制造通过率。
3. 真实web目标明确后才启用固定版本的e2e。Windows按官方WSL路线，WSL的通过不等于Windows宿主/DLL/Tauri/游戏输入通过。安装、浏览器下载、模型/云provider费用、常驻MCP仍遵守全局授权边界。
4. 通过MCP检查当前DOM/语义树、定位与实际媒体：`open_session`→`tools`→`call`→`close_session`。工具参数取自本次schema。MCP交互不算test run；每target一个owner，Playwright/DevTools/e2e/sidecar不能抢同一会话。
5. 实际适配全部接通后才集中运行`quality/e2e`候选spec。workers1、retries0、cache off、固定build，命令硬超时，记录stderr/exit。缺heading/video/renderer映射是适配缺口，不改成假页面或跳过。普通`expect`检查精确状态，模型`agent.assert`不代替输入释放或媒体原子性证据。
6. 按Pending矩阵归档结论；说明只覆盖页面/视频还是实际任务。保留MaaEnd既有节点、Native回调/生命周期和Windows真实云会话验收。新构建改变输入、帧源或时序后重开受影响门槛。

完成条件：明确的真实target上取得与实际候选build/suite绑定的验证结果，失败/Blocked/Pending均有原因；本次会话、进程和端口清理可核实。未取得证据不声称工具链或测试生效。
