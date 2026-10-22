# 接入与收口

材料安装位置是当前MaaEnd主仓根下 `.qoder/maaend-private/quality/e2e`；该技能文件可安装到Qoder技能目录，无需从技能目录向上猜主仓。先用宿主工作区根与`git rev-parse --show-toplevel`核对；身处Testset/AI子模块时返回其父主仓，不能把它们当应用工程。

固定候选：e2e0.21.2、web0.15.2，发行commit393005aa；Node^22.22.3或>=24.8；Windows官方WSL路线。官方skill名`e2e`，其`references/mcp.md`对应MCP操作，不是Playwright CLI。安装成功只代表文件可见，需独立验证工具表/真实target/关闭生命周期。

读取顺序：当前全局AGENTS → 主仓/路径AGENTS → `.qoder/maaend-private`执行约定 → `quality/manifest.json` → `quality/e2e/运行与边界.md` → 实际任务所需官方reference。遇到冲突引用具体当前条款，决策用宿主交互工具；不要让上游`init`向主仓自动写package/MCP/测试文件。

默认候选测试只读页面与真实video进度，没有自动登录、游戏动作、录屏、模型配置或生产CDP接管。CDP的普通connect建新context，persistent首次取已有tab会先导航about:blank；不将它当无扰动旁观真实登录会话的工具。生产输入循环仍归sidecar/SiteAdapter，Native bridge仍归拥有Tasker的宿主。

把官方skill中的持续重跑、先写测试、自动添加CI/PR、默认并发和云provider建议限定到已授权范围；本任务只在团队及完整功能接通后集中验证。重复失败两次保存原始证据后停当前路径并诊断，不能反复改selector/增加sleep硬撞。
