# CloudWeb CW8-2 Windows task matrix

Source inventory: `inventory-current.json` sha256 `bc0bb67d6b8b93e7efeb9b2c8e476ec519b912b613e048aff201428b5f514cc8`
MaaEnd: `c7fd1b25071d9976703635b7dda66812172d1629`
Tasks: 46 Windows-applicable; presets 3; settings 2; pretask 1.

Status: all rows Pending. A row may only move to Pass with the full evidence schema; fake/fake SDK/fixed PNG/standalone Node scripts do not count.

| Task | Family | Group | Options | Status | Note |
| --- | --- | --- | ---: | --- | --- |
| AccountSwitch | lifecycle | other_menu | 1 | pending | 云账号 profile/退出登录语义。 |
| AeroSalvage | open_world | open_world | 0 | pending |  |
| AutoCollect | open_world | open_world | 5 | pending |  |
| AutoEcoFarm | open_world | open_world | 3 | pending |  |
| AutoEssence | auto_essence | sanity_sink | 3 | pending | 地点与战斗配置/导航路线需分别取证，不能只测一个点。 |
| AutoSell | regional | regional_development | 3 | pending |  |
| AutoStockpile | regional | regional_development | 5 | pending |  |
| AutoStockStaple | regional | regional_development | 3 | pending |  |
| BakerEntry | other_menu | other_menu | 0 | pending |  |
| BatchAddFriends | dijiang | dijiang_ship | 1 | pending |  |
| BatchDeleteFriends | dijiang | dijiang_ship | 1 | pending |  |
| BatchUseDetector | valuables | valuables_vault | 3 | pending |  |
| ClaimSimulationRewards | other_menu | other_menu | 1 | pending |  |
| CloseGamePC | lifecycle | other_menu | 2 | pending | 云会话结束语义，不是本地进程 kill。 |
| Crafting | backpack | backpack | 0 | pending |  |
| CreditShoppingN2 | other_menu | other_menu | 9 | pending |  |
| DailyRewards | daily_claim | other_menu | 6 | pending | 上游只启用 DailyClaimDeliveryJobsRewardSub；其余四项 enabled=false，不计通过。 |
| DecomposeWeaponEssence | valuables | valuables_vault | 1 | pending |  |
| DeliveryJobs | delivery_jobs | regional_development | 5 | pending | 导航与送货分支。 |
| DijiangRewards | dijiang | dijiang_ship | 3 | pending |  |
| EnvironmentMonitoring | regional | regional_development | 0 | pending |  |
| EssenceFilter | valuables | valuables_vault | 1 | pending |  |
| GearAssembly | valuables | valuables_vault | 2 | pending |  |
| GiftOperator | dijiang | dijiang_ship | 2 | pending |  |
| ImportBluePrints | open_world | open_world | 1 | pending |  |
| IntelArchive | other_menu | other_menu | 0 | pending |  |
| ItemTransfer | backpack | backpack | 7 | pending |  |
| LevelUpOperator | other_menu | other_menu | 1 | pending |  |
| ProtocolSpace | protocol_space | sanity_sink | 4 | pending | 关键战斗模式需分别取证，不能只测入口。 |
| PullCountCalculator | other_menu | other_menu | 0 | pending |  |
| PuzzleSolver | open_world | open_world | 1 | pending |  |
| ReadAllWiki | other_menu | other_menu | 0 | pending |  |
| RealTimeTask | realtime_assist | realtime | 12 | pending | AutoSkip/AutoClose/AutoFight/AutoPick/QuickTeleport/AutoPuzzle/AeroSalvage/AutoZipline/AutoPat；SklandMap/VideoBrowser 附属。 |
| ReceiveProdManual | backpack | backpack | 0 | pending |  |
| ResourceRecycleStation | open_world | open_world | 1 | pending |  |
| SeizeDeliveryJobs | delivery_jobs | regional_development | 4 | pending | 导航与抢占分支。 |
| SellProduct | regional | regional_development | 7 | pending |  |
| SharedZiplineDelete | open_world | open_world | 2 | pending |  |
| SimpleProductionBatchStart | backpack | backpack | 0 | pending |  |
| StashBackpack | backpack | backpack | 1 | pending |  |
| SwitchTeam | open_world | open_world | 1 | pending |  |
| TrialOfSwordmancy | trial_of_swordmancy | regional_development | 1 | pending | Daily/Coating/Farm25 分开，不得由 Daily 推断其余。 |
| UmbralMonument | other_menu | other_menu | 4 | pending |  |
| VisitFriends | dijiang | dijiang_ship | 3 | pending |  |
| WeaponUpgrade | valuables | valuables_vault | 0 | pending |  |
| ZiplineImport | open_world | open_world | 1 | pending | 独立 Firefox/profile 与云游戏 profile 生命周期分开。 |

## Pretask

| Pretask | Entry | Status | Note |
| --- | --- | --- | --- |
| LaunchGamePC | null | pending | LaunchGamePC 需改成 CloudWeb 启动/连接流程；不是本地进程启动。 |

## Presets / settings

| Kind | Name | Status |
| --- | --- | --- |
| preset | DailyFull | pending |
| preset | QuickDaily | pending |
| preset | RealtimeAssist | pending |
| setting | Keymap | pending |
| setting | Zipline | pending |

## Evidence schema

Required fields: candidate_sha; runtime（MaaEnd/MXU/sidecar/MaaFramework/browser）; actuator（kind/target/profile）; options/routes/presets; preconditions; steps; game_result; cancel/recover; evidence_paths+sha256; unverified.

Status enum: pending / pass / fail / blocked_fixture / not_applicable / not_implemented.
