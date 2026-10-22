> 上一版安装器修复的历史记录；本轮团队/方法修订的独立审查见revision-review.md，实际资格以包根验证结果.md为准。

# Qoder CN 配置修复复核

结论：本轮指出的四项问题已在配置包中闭环，未发现新的交付阻塞问题。复核仅覆盖这些修复，没有扩大到第三方软件或实际游戏运行。

| 修复项 | 复核证据 | 结果 |
| --- | --- | --- |
| 回执绕过写保护 | 回执增加格式标识和完整性校验，并纳入统一 proposed 及 tracked/link 检查；foreign 或修改过的回执在写入前被拒绝 | 通过 |
| 写入路径重定向 | reject_link 检查符号链接和 Windows FILE_ATTRIBUTE_REPARSE_POINT；全部目标祖先预检；仓库内目标还检查解析后的包含关系 | 静态通过；Windows 实机资格待验 |
| 当前 Skills 与规则漂移 | 模板明确每次新上下文重新枚举源技能，比较真实名称、文件集、哈希与副本，并比较全局规则、仓库规则及 HEAD；漂移先记 Pending 并审查同步计划 | 通过 |
| 测试矩阵引用 | skill-conflicts.md 已明确指向 execution-contract.md 的测试矩阵 | 通过 |

独立执行了两个受影响的回执用例：test_edited_receipt_is_preserved、test_tracked_receipt_is_preserved，均通过，退出码 0。原始缺陷复现曾证明已跟踪回执会被覆盖；修复后的回归覆盖相同保护目标。临时 fixture 由测试清理。

完整性校验用于发现意外修改，不构成抵御恶意本机篡改的签名或安全边界。提示词的规则遵守与主会话权限也仍须按本地资格清单验收。

Windows junction 只进行了静态审阅，未在 Windows 上创建 junction 运行；qodercn、MCP 握手、交互提问工具和真实浏览器会话均未实测。
