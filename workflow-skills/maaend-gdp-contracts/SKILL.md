---
name: maaend-gdp-contracts
description: 评估或实施 MaaEnd 私有TypeScript协调层的授权、owner/generation/target及候选build/suite证据绑定；使用gdp-ts真实API，区分编译期约束、可信运行时验证和实际功能验收。
---

# MaaEnd 证据契约

先读 [采用边界](references/integration.md) 和 `.qoder/maaend-private/research/testing-methods-research.md`，按需读官方 `gdp-ts` 技能及其limits/where-to-stop。gdp-ts不是E2E框架、浏览器协议或安全沙箱。

1. 定位MaaEnd主仓根下 `.qoder/maaend-private/quality/gdp/运行与边界.md`。确认实际需要TypeScript边界；Go/Rust/Cpp任务不为采用库而改语言或给主仓加TS依赖。
2. 把授权、当前owner/generation/target、实际候选build与suite、有效期和覆盖结论的可信运行时来源先接通。默认`runtime-bridge.ts`抛Pending；保持它拒绝直到真实reader有可核实来源，不造Passed JSON使类型示例跑通。
3. 若采用proof，按`proofs/proof-boundary.ts`把运行时检查放小trusted模块，prover私有，敏感函数要求同一组`Named`。freeze输入原子字段；proof只对检查时刻成立，执行前仍需真实generation/租约/事务检查。
4. 先完成全链路与团队交接，之后集中执行正例、未标注负例、运行时拒绝矩阵和必要lint。正例通过/负例按指定类型错误失败不是功能通过；依赖缺失或解析错误不是有效负例。
5. 采用官方lint时显式覆盖本例相对vendor导入与trusted目录；禁止以`as`/`any`/ambient declaration/definite assignment绕过。对同kind proof运行时共用对象、TOCTOU与跨进程边界给出实际限制，不把类型保证写成绝对安全承诺。

完成条件：真实reader与实际动作/报告消费点建立完整边界；每项拒绝条件有证据；原生和云游戏验收仍按FRD及当前仓库规范关闭。未接通reader时仅交付候选并标Pending。
