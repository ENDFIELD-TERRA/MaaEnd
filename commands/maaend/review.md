---
description: 在固定候选上启动独立 MaaEnd 审查，逐条核实问题并向实现者交接。
---

使用 Agent Teams。读取指定候选、真实diff和已定契约，确认builder已释放写scope。让独立reviewer按当前仓库/局部规范、commit-pr-contract审查；不要仅转述实现者或CodeRabbit结论。每条finding给触发、位置、影响和证据，核实后再交builder修复。新候选重开受影响审查；只写私有结果，无远端review/评论授权。
