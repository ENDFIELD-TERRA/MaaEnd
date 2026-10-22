---
description: 联合接通并固定候选后集中进行 MaaEnd 分层验证，区分e2e、类型证明与真实Windows资格。
---

使用 Agent Teams。先核验整链路已接通、联合演示和独立review报告绑定当前候选。未满足时继续实现/补证，只准备验证矩阵，不用测试/性能优化取代目标。满足后让validator读取maaend-e2e-validation、maaend-gdp-contracts及原仓测试规范：按路径触发必要套件，以专属串行会话执行实际可用工具。e2e MCP勘察不算run；gdp编译期证明不算行为通过；Linux/WSL不算Windows云输入隔离。记录命令/退出码/候选/图像与媒体证据，无法执行显式Blocked，清理所属资源。
