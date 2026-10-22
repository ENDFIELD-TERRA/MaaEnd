# MaaEnd 仓库关系与来源

核验时间：2026-10-21。此文件只作索引，实际开发必须读当前checkout，不复制旧快照的指令来覆盖新规范。

| 对象 | 本轮固定版本 | 工作边界 |
| --- | --- | --- |
| MaaEnd/MaaEnd v2 | 051d17f2527e2e990952d6655f1f380d5d7de81e | 主仓Pipeline、Go/Cpp、任务、测试定义与CI；当前Python目标3.14，新增ruff检查 |
| MaaEnd/MaaEndTestset main | 68f40d1290b5f1b6cb2a064b6b26efa44d8d9b6a | 主仓 tests/MaaEndTestset gitlink；ADB/CloudADB/Win32素材，不是独立runner |
| MaaEnd/MaaEnd-AI main | fd3ed6462d2ee7191950131776e62136ae7bfb94 | assets/resource/model gitlink；OCR/检测/地图模型，保留模型来源、许可和校验和 |
| MaaXYZ/MaaFramework | 实际发行runtime/SDK/binding组合 | 接口能力必须对实际DLL验证，不能拿main源码替发行包能力背书 |
| MistEO/MXU | 实际MaaEnd打包版本 | GUI/Tasker/CustomController/preview桥；修改须在它自己的规则和review边界内 |

Testset和AI本次未发现独立AGENTS/Skills/runner。主仓maatools.config.mts指定tests/MaaEndTestset作为casesCwd；测试定义在主仓，截图素材在子仓；检查tests/scripts/loader.mts的controller/resource映射。静态命中测试不能证明导航/战斗时序。模型、截图子仓独立提交与主仓gitlink更新是两个审查对象，不能一次更新main替代。

前次CloudWeb FRD固定5155c76c；上一版配置审阅8168467f，本轮v2相对它推进16个提交，包含Python/CI/依赖和两个技能脚本变化。不要静默重置FRD的证据基线或声称其功能统计在新HEAD重跑。先比较实际差异，再决定受影响验收。

相关官方入口：

- https://github.com/MaaEnd/MaaEnd/blob/v2/AGENTS.md
- https://github.com/MaaEnd/MaaEnd/tree/v2/.agents/skills
- https://github.com/MaaEnd/MaaEnd/blob/v2/docs/zh_cn/developers/coding-standards.md
- https://github.com/MaaEnd/MaaEnd/blob/v2/docs/zh_cn/developers/node-testing.md
- https://github.com/MaaEnd/MaaEndTestset
- https://github.com/MaaEnd/MaaEnd-AI

本轮还核验了Issue #6483中在途Windows WebView2方案、Draft PR #6696的Linux方案与维护者的Windows疑问。开工前刷新相关工作，避免重复与职责错位；它们不是上游接受承诺，也不能据此自行放弃Windows目标。没有创建或评论远端议题。
