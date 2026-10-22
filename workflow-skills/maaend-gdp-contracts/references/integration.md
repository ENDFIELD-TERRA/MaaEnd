# 采用边界

材料安装在当前MaaEnd主仓根 `.qoder/maaend-private/quality/gdp`，技能目录不承担工作目录假设。官方skill名`gdp-ts`；完整references保留相对路径。固定源码为rauchg/gdp-ts0b6ebeee，MIT。package.json的0.1.0发布未独立证实，因此默认vendor已审计源码，不自动安装npm包。

先读全局及仓库规则、当前执行约定和`quality/manifest.json`，再读候选材料。用户要求先完整接通再集中验证，官方skill里“立刻加mistakes/test”的顺序不覆盖本次阶段方针；必要反例先作为材料准备，测试等集成完成后统一执行。

`Named`的身份在TypeScript checker中，运行时只有`{value}`wrapper；proof运行时只有按kind共享的对象。本包额外附metadata也不产生可信来源。trusted reader应从认证的真实runner/sidecar取得当前授权与结果，核验artifact对应实际PID/target、scope、时间、完整suite和无跳过；IPC边界仍运行时解析和校验，不能传proof对象当token。

候选`buildReviewRecord`只创建私有审查记录，不授权push、PR、Issue、发布或控制输入。实际副作用点必须重新核验当前generation/授权并遵循交互审批。TS检查不能证明远端抬键、取消屏障、帧新鲜度或Windows隔离。

上游现有CI、lint、提交及PR风格保持权威。不要自动迁移成ESLint/Oxlint，也不要把“证明对象用错会编译失败”夸大成“验证器可信且任务成功”。
