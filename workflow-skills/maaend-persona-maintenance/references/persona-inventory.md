# 人格登记表（演进基线快照）

7 个人格文件的当前状态。行数为 `wc -l` 实测（整文件，含 frontmatter），核验日期 2026-10-23。每次增补提案批准后由维护流程更新本表。

akeData 列于 2026-10-23 用 `fetch_endfield_lore.py ake` 实测核验（数据版本 1.6.4@11262943-8）：records 全员 6 条；voices/dialogue 计数为该版本实测值，版本更新后须重跑对比。

| 角色文件 | 干员 | akeData bundleId（权威） | actorNameId | records | voices | 对白句数 | 当前数据版本 | skland gameEntryId | warfarin slug | 当前行数 | 深度状态 | 上次核验 |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| templates/perlica-maaend.md | 佩丽卡（协调者/main） | chr_0004_pelica | pelica | 6 | 84 | 1674 | 1.6.4@11262943-8 | 5 | perlica | 103 | base 5 层已建（锚点/三语域/可迁移写法/可说不可说/认知边界），另含启动顺序与执行契约段；lore references 待扩展 | 2026-10-23 |
| team-templates/maaend-issue-analyst.md | 赛希（analyst） | chr_0011_seraph | seraph（非 xaihi） | 6 | 79 | 72 | 1.6.4@11262943-8 | 150 | xaihi | 88 | base 5 层已建；lore references 待扩展 | 2026-10-23 |
| team-templates/maaend-designer.md | 祀（designer） | chr_0036_jsspsi | si | 6 | 82 | 510 | 1.6.4@11262943-8 | 2338 | si | 88 | base 5 层已建；lore references 待扩展 | 2026-10-23 |
| team-templates/maaend-implementer.md | 陈千语（builder） | chr_0005_chen | chen | 6 | 79 | 1608 | 1.6.4@11262943-8 | 12 | chen-qianyu | 89 | base 5 层已建；lore references 待扩展 | 2026-10-23 |
| team-templates/maaend-reviewer.md | 诀/李织烟（reviewer） | chr_0032_lizhiyan | lizy（非 lizhiyan） | 6 | 82 | 527 | 1.6.4@11262943-8 | 1675 | 无（404，2026-10-23 核验） | 86 | base 5 层已建；lore references 待扩展；warfarin 侧来源缺口待其收录后补 | 2026-10-23 |
| team-templates/maaend-validator.md | 提弗洛斯（validator） | chr_0034_typhoea | typhoeus | 6 | 81 | 302 | 1.6.4@11262943-8 | 2116 | typhoeus | 90 | base 5 层已建；lore references 待扩展 | 2026-10-23 |
| team-templates/maaend-delivery.md | 庄方宜（delivery） | chr_0030_zhuangfy | zhuangfy | 6 | 81 | 746 | 1.6.4@11262943-8 | 1132 | zhuang-fangyi | 86 | base 5 层已建；lore references 最丰富（zhuang-fangyi-skill 范式可参照，主线覆盖至《余晖未却》） | 2026-10-23 |

## 登记规则

- 来源权威顺序：akeData bundleId（权威，datamine 表）> skland gameEntryId > warfarin slug。akeData 映射内置于 `scripts/fetch_endfield_lore.py` 的 AKE_PERSONAS（2026-10-23 实测核验）。
- actorNameId 与 bundleId 不同名的两处：诀=`lizy`、赛希=`seraph`；跑 `ake dialogue` 时勿用 bundleId 猜。
- gameEntryId 与 `使用说明.md` 的采集记录一致（赛希=150、祀=2338、陈千语=12、庄方宜=1132、诀=1675、提弗洛斯=2116、佩丽卡=5）。
- warfarin slug 于 2026-10-23 逐个探活核验（200/404）；「诀」无已收录 slug，禁止借邻近角色文本迁移，扩展时用 akeData profile/dialogue 与 skland detail 页。
- 「当前数据版本」「records/voices/对白句数」构成新剧情检测快照：`ake --version` 的 latest 变化后重跑 profile+dialogue，计数或文本变化即有新素材（见 lore-expansion.md 第 2 节）。
- 「深度状态」在每次提案批准落地后更新：注明新增层/条数与 token 影响；行数同步实测更新，不沿用旧值。
- 本表只登记状态，不存 lore 正文；深度素材放私有研究区 `research/persona-evolution/`，按需读取，不常驻。
