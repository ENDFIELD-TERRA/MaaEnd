# MaaEnd / Qoder CN 修订候选独立审查

审查日期：2026-10-21。审查对象为当前 package 源码与合同，未运行额外套件、第三方安装、模型服务或浏览器。主线程正在集中验证，最终验证记录由主线程收口。修复后的两项问题已经再次静态复核闭环；本审查范围内没有剩余交付阻塞问题。

## 已修复并复核闭环的问题

1. **报告入账严格校验。** `runtime/caseflow.py` 新增 stdlib `validate_report`，record 和 checked_reports 均调用；精确检查字段集、类型、非空作者/摘要、非 bool 的正整数 generation、candidate_id 和 artifact 哈希格式。`schemas/report.schema.json` 的非空白 pattern 同步，先前错误类型或额外字段入账的缺口已经关闭。没有把格式正确写成结论真实性。
2. **MediaStream 路径与源身份。** `quality/e2e/tests/real-media.e2e.ts` 现在区分 URL/blob 与 MediaStream，在同一个 evaluate 闭包内持有原 video、srcObject、stream 和 track 引用并完成双采样；检查源/trackset/live 状态，监听源重载、track增删/结束/静音，返回前再次核对身份。URL 路径继续核对 currentSrc；不支持的 renderer/provider 明确 Pending。WebRTC 的空 currentSrc 不再被错误当作缺源。此项仅静态复核，不代表真实 WebRTC 或云游戏已经运行通过。

## 已明确的覆盖边界

`current_candidate` 检查 tracked diff 和 manifest 列明文件；未列明的新增 untracked 产品文件不在候选绑定中。使用说明及 team-workflow 已明确要求 manifest 枚举本次全部相关 untracked 产品源与验收产物，并明确未列明文件不受漂移检测，没有承诺检测整个工作区的所有漂移。

## 已复核部分

- 静态核对 7 份角色示例，每份均注册相同的 23 项技能：MaaEnd 16、浏览器 2、官方方法 2、工程适配 3；5 个命令入口一致，无技能遗漏或多余名称。
- 安装器仍从实际 checkout 复制上游技能，原位更新限定为本包拥有且未被编辑的文件；现有同版本 MCP 的显式启用/权限选择保留，改命令/参数需人工合并。没有发现新的写保护阻塞问题。
- caseflow 的事件去重、新输入 generation、报告撤销、候选 HEAD/diff/列明文件哈希、报告/证据哈希、独立 integration/review 作者和 validation 后置链路均已存在。它没有被合同写成网络监听器、模型服务、发布器或真人授权来源。
- team-workflow / commit-pr-contract 区分上游已实现机制、本包映射和待本地资格；Issue 关闭、bot 分析、CI、人工合并、发行与实机反馈没有被合并成一个完成状态。
- 原生 Teams、普通子代理、Windows 不适用的跨会话 socket、自建交接与标准 A2A 的边界表达清楚。角色定义名与实际运行时身份也有资格检查要求。
- e2e 的配置字段、Browser.evaluate、screen 查询及 MCP 勘察/run 区别与固定源码一致；gdp 的 name/defineProof/Named/Proof API、默认抛 Pending 的 reader、静态证明与真实身份/时效/TOCTOU 的限制相符。

本次审查时最终验证记录仍在收口。实际执行与未验证范围以本包已刷新的 `验证结果.md` 为准；审查结论不替代运行证据。

Windows 原生路径、NTFS junction、PowerShell/cmd、qodercn 原生 Teams/消息/任务/恢复、MCP、WSL e2e 和真实云会话均未由本次独立审查实测。此结论不构成这些运行资格的通过证明。

主线程收口补充：研究材料随后也加入私人安装目标，团队/方法Skill显式读取它；相关安装集成断言随最终20项套件通过。此补充不是原生Qoder运行资格。
