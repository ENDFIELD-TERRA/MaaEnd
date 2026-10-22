# 相关仓库与浏览器工具核验

核验时间：2026-10-20。全部查询通过 GitHub plugin 只读完成；未安装或启动任何包、浏览器、测试或服务。

## 固定基线与职责

| 对象 | 本轮读取的默认分支与 HEAD | 作用与边界 |
| --- | --- | --- |
| MaaEnd/MaaEndTestset | main / `68f40d1290b5f1b6cb2a064b6b26efa44d8d9b6a` | 截图与识别期望素材；根目录 ADB、CloudADB、Win32；完整递归树 612 项，无 AGENTS.md、CLAUDE.md、SKILL.md、独立 runner 或 LICENSE |
| MaaEnd/MaaEnd-AI | main / `fd3ed6462d2ee7191950131776e62136ae7bfb94` | 识别与地图模型；detect/AutoFight、detect/ProtocolSpace、map、ocr；AGPL-3.0；无 AGENTS、Skills 或独立测试入口 |
| microsoft/playwright-mcp | main / `ab6d67cbf8b619338f096980806fdbe29432cc4d` | Playwright 浏览器 MCP；package.json 0.0.83，Node >=18，Apache-2.0 |
| ChromeDevTools/chrome-devtools-mcp | main / `384d9aad349a2e19940a5f93aaac6b2e8312bc91` | Chrome 调试 MCP 与官方 Skills；package.json 1.11.0，Node `^20.19.0 || ^22.12.0 || >=23`，Apache-2.0 |

MaaEnd `v2@8168467f16802933c7636fd8f22780da70e5bc45` 的两个 gitlink 恰好分别固定在以上 Testset 与 AI HEAD。安装与开发仍以实际 checkout 的 gitlink 为准，不自动追随上述仓库 main，也不执行 `git submodule update --remote`。

来源：[主仓 .gitmodules](https://github.com/MaaEnd/MaaEnd/blob/8168467f16802933c7636fd8f22780da70e5bc45/.gitmodules)、[模型 gitlink](https://api.github.com/repos/MaaEnd/MaaEnd/contents/assets/resource/model?ref=8168467f16802933c7636fd8f22780da70e5bc45)、[测试集 gitlink](https://api.github.com/repos/MaaEnd/MaaEnd/contents/tests/MaaEndTestset?ref=8168467f16802933c7636fd8f22780da70e5bc45)、[Testset 树](https://github.com/MaaEnd/MaaEndTestset/tree/68f40d1290b5f1b6cb2a064b6b26efa44d8d9b6a)、[AI 树](https://github.com/MaaEnd/MaaEnd-AI/tree/fd3ed6462d2ee7191950131776e62136ae7bfb94)。

## 测试集与模型的实际消费方式

主仓 `maatools.config.mts` 使用 `tests/MaaEndTestset` 作为 casesCwd；测试定义在主仓 `tests/**/test_*.json`，截图属于 submodule。节点用例验证静态图像命中及 box，不证明完整流程时序。新控制器/资源还需审查 `tests/scripts/loader.mts` 的 controllerMap/resourceMap，不可仅增加一个截图目录就声称支持 CloudWeb。

当前通用 `maa-tools test` 不启动自定义识别服务；MapLocateAssertLocation 与 MapNavmeshConnectivity 各有独立 Python runner。模板、ROI、阈值等变化先按主仓当前 AGENTS.md 判断本地测试触发条件；不要让旧文档或技能把“默认交 CI”覆盖成无条件全套测试。截图先脱敏后入库，保留子仓独立提交与主仓 gitlink 更新的审查边界。

AI 仓库 OCR 文档标明 PP-OCRv6_small_det / PP-OCRv6_small_rec 与相应字典来源。模型变更应核验文件来源、许可、校验和、加载路径和真正消费者；不因模型目录已存在就宣称兼容、性能或识别质量已验证。

来源：[maatools.config.mts](https://github.com/MaaEnd/MaaEnd/blob/8168467f16802933c7636fd8f22780da70e5bc45/maatools.config.mts)、[节点测试文档](https://github.com/MaaEnd/MaaEnd/blob/8168467f16802933c7636fd8f22780da70e5bc45/docs/zh_cn/developers/node-testing.md)、[OCR 来源](https://github.com/MaaEnd/MaaEnd-AI/blob/fd3ed6462d2ee7191950131776e62136ae7bfb94/ocr/README.md)。

## MCP 的可用候选

GitHub release 已核验 `@playwright/mcp@0.0.83`（2026-09-28）和 `chrome-devtools-mcp@1.11.0`（2026-10-15），可固定这两个候选版本，再在本机完成 npm integrity / 安装和握手验证。Node 建议使用满足二者 engines 的受支持 LTS；仅知道 Node >=18 不足以运行当前 DevTools 包。

以下为 Windows stdio 候选，`cmd /d /s /c` 用于稳健调用 npx.cmd；Qoder CN 最终容器文件与字段由 Qoder 官方文档核验结果决定。安装会执行第三方软件，先完成包与机器环境审查；本报告未执行安装。

```json
{
  "mcpServers": {
    "playwright": {
      "command": "cmd",
      "args": ["/d", "/s", "/c", "npx", "-y", "@playwright/mcp@0.0.83", "--browser=chrome", "--isolated", "--no-webmcp"]
    },
    "chrome-devtools": {
      "command": "cmd",
      "args": ["/d", "/s", "/c", "npx", "-y", "chrome-devtools-mcp@1.11.0", "--isolated", "--no-usage-statistics", "--no-performance-crux"],
      "env": {
        "CHROME_DEVTOOLS_MCP_NO_UPDATE_CHECKS": "1"
      }
    }
  }
}
```

这个基础配置让两个 MCP 各自启动专属浏览器。可见窗口是默认值，未设置 headless。临时 profile 适合受控页验证，不保存真实云游戏登录；真实会话应使用明确指定、由一个 owner 管理的专用 profile。不得把默认私人 profile 暴露给自动化。

若必须观察同一真实云会话：显式启动一个非默认 `--user-data-dir` 的 Chrome，调试端口仅本机可达；Playwright 用 `--cdp-endpoint=http://127.0.0.1:<port>`，DevTools 用 `--browser-url=http://127.0.0.1:<port>`。同时取消独立启动用的 `--isolated`。两个连接并存不等于允许并发输入：建立单 owner / 租约，另一个仅观察，任何会触发导航、focus、resize、close、录屏状态变化的工具也先协调。不得扫描任意已有调试端口后接管会话。

DevTools 仅正式支持 Chrome / Chrome for Testing；Edge 或其他 Chromium 不受同等保证。`--autoConnect` 要求 Chrome 144+ 与人为允许远程调试，可能连接默认 profile 并暴露其全部窗口，因此不作为本项目默认值。显式端口方案要求 Chrome 使用非默认 profile。

DevTools 默认上报 usage statistics，并可能把 trace URL 发给 CrUX；以上显式关闭两项，关闭定期更新检查。日志、网络、trace、截图都可能含凭据和用户数据，入公开 PR/Issue 前仍须脱敏。Playwright 的 origin allow/block flags 不是完整安全边界；不启用 unrestricted-file-access、通配扩展权限或远程公网 CDP。

来源：[Playwright MCP README](https://github.com/microsoft/playwright-mcp/blob/ab6d67cbf8b619338f096980806fdbe29432cc4d/README.md)、[Playwright release 0.0.83](https://github.com/microsoft/playwright-mcp/releases/tag/v0.0.83)、[DevTools release 1.11.0](https://github.com/ChromeDevTools/chrome-devtools-mcp/releases/tag/chrome-devtools-mcp-v1.11.0)、[DevTools 配置](https://github.com/ChromeDevTools/chrome-devtools-mcp/blob/384d9aad349a2e19940a5f93aaac6b2e8312bc91/docs/configuration.md)、[高级用法](https://github.com/ChromeDevTools/chrome-devtools-mcp/blob/384d9aad349a2e19940a5f93aaac6b2e8312bc91/docs/advanced-usage.md)、[客户端配置](https://github.com/ChromeDevTools/chrome-devtools-mcp/blob/384d9aad349a2e19940a5f93aaac6b2e8312bc91/docs/client-configurations.md)。

## 浏览器 Skills 来源

ChromeDevTools 官方仓库 `skills/` 包含：`a11y-debugging`、`chrome-devtools-cli`、`chrome-devtools`、`cookie-debugging`、`debug-optimize-lcp`、`memory-leak-debugging`、`troubleshooting`，应复制完整目录和 references，保留 Apache-2.0 LICENSE、来源与固定 commit。若仅使用 MCP，主入口是 `chrome-devtools`，CLI 技能要求另行验证命令。

Playwright MCP 仓库自身没有官方浏览器操作 `skills/*/SKILL.md`；`.claude/skills/release.md` 是上游发版维护技能，不应当作用户浏览器技能。官方操作技能来自 [microsoft/playwright-cli/skills/playwright-cli](https://github.com/microsoft/playwright-cli/tree/b1a5903c5b633b8c9f843ad7b629c2a78437040c/skills/playwright-cli)，包含 session、storage、trace、测试生成等 references，但它描述 CLI，不能宣称安装该 skill 即可通过 MCP 运行全部命令。可以把 CLI 当可选工具，并另加小型本地 Playwright MCP 操作技能，要求实际 tools/list 驱动、snapshot/locator 驱动、有界等待、单浏览器 owner、脱敏与资源回收。

Qoder skill 是否自动索引以及 subagent 是否预加载，需分别验证：文件存在/能列出、frontmatter `skills` 允许使用、正文是否真的进入当前上下文是三个不同层次。不能用目录复制成功声称协作已生效。

## 验收状态

补充：交付的 DevTools 技能源码按正式发行 `chrome-devtools-mcp-v1.11.0` 的 commit `a878bcad39584337e055778353246820deafa7c0` 读取，保存在同目录 `browser-skill-sources.json`。数组包含两个技能的 SKILL.md 和上游 LICENSE；该 tag 中这两个技能没有附属 references。三项 UTF-8 字节均按 Git blob SHA-1 算法与 API 返回 SHA 核对一致。发行 tag 的 package.json、configuration、client-configurations 已另行读取，确认上述开关与 Windows cmd /c npx 示例在发行版文档中存在。`startup_timeout_ms` 是其 Codex 客户端示例字段，不是 Qoder 通用字段，不应移植。

- 已验证：GitHub 固定树、README、package engines、官方发布、MCP 参数文档、Skills 来源与两个子仓职责。
- 未验证：本机 Qoder CN 的配置解析、Node/npm 安装、两个 MCP stdio handshake、tools/list、浏览器操作、Windows 双向输入隔离、真实云终末地媒体持续性。
- 本轮没有外部写入、安装、构建、测试、常驻进程或端口。
