---
description: 只读接取指定 MaaEnd Issue，用原生团队分析证据、补证与下一步；不自动公开回复。
---

使用 Agent Teams。以本次用户提供的完整 Issue URL 为入口，读取当前有效全局和仓库规则、team-workflow及commit-pr-contract。没有明确URL时用真实交互工具收集，不扫描无关仓库。只读核验状态、全部可见评论/日志附件、已有bot分析、重复工作和相关PR；按实际触发条件与事件ID建立私有case。创建analyst，给出原始输入、scope、依赖、输出目录和schemas/handoff.schema.json；完整读取并调用正确专项Skills。结果先落盘复读再SendMessage。缺证明确Blocked；需求咨询缺日志不自动当bug失败。此入口不创建占位评论、不写远端、不启动计费事件监听器。
