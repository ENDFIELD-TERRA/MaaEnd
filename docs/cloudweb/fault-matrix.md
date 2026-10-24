# CloudWeb CW9-1 fault matrix

Source: `cloudweb-roadmap.json` (baselines below).

Baselines: sidecar `8b9cc6f670fc59652ffb7f50e312d1949e7f73ac`; MXU `8bddfee34bba3ea8ec81e3b8fdcb118375329b76`; MaaEnd `c7fd1b25071d9976703635b7dda66812172d1629`.

Rows: 51; domains: 8; all **Pending**. This file defines the matrix and evidence schema only; no fault has been executed.

Closure layers: `protocol_fixture` (protocol/fake actuator), `local_browser` (dedicated local Chrome/profile), `real_cloud_session` (official cloud session). A fixture result cannot close a `real_cloud_session` row, and no row may be marked Pass without the full evidence schema.

## Domain summary

| Domain | Scope | Rows | Mapped CW | Status |
| --- | --- | ---: | --- | --- |
| D1 控制面派发、回执与未知结果 | control_plane | 9 | CW9-2 | pending |
| D2 租约、owner 与代次 | lease_ownership | 6 | CW9-2 / CW9-4 | pending |
| D3 Stop、取消与中和 | stop_neutralize | 6 | CW9-2 / CW9-5 | pending |
| D4 媒体进度、帧健康与几何 | media | 8 | CW9-3 | pending |
| D5 浏览器、CDP 与云会话 | browser_session | 6 | CW9-3 / CW9-4 | pending |
| D6 Host、GUI 与进程生命周期 | host_lifecycle | 7 | CW9-4 | pending |
| D7 人工接管与所有权转移 | manual_takeover | 3 | CW9-5 | pending |
| D8 长运行、资源与跨 run 隔离 | long_run | 6 | CW9-6 | pending |

## Execution order

| Batch | Scope | Rows | Depends | Gate | Note |
| --- | --- | ---: | --- | --- | --- |
| A | protocol_fixture rows (D1/D2) | 15 | CW2 | G4-pre | Sidecar fake actuator / pipe harness first; no cloud session needed. Results close only protocol/state-machine claims. |
| B | local_browser rows (D3/D4/D5/D6) | 26 | CW4, CW5 | G4-pre | Dedicated local Chrome/profile; covers media, CDP, page, process and Stop behavior. Still not a real-cloud claim. |
| C | real_cloud_session rows (D5/D6/D7/D8) | 10 | CW7 | G4 | Official cloud session; only these results may support G4 behavior claims. Long-run comes last. |

## Matrix

### D1 控制面派发、回执与未知结果 (`control_plane`)

Mapped: CW9-2. Sources: Review §6.2; FRD §10.1 B10; Plan CW9-2.

| ID | Fault / injection | Expected | Forbidden | Evidence | Layer / depends | Status |
| --- | --- | --- | --- | --- | --- | --- |
| F-CP-01 | **下行 ACK 丢失（key_down 已派发）**: fake actuator 在应用 key_down 后丢弃回执；命令停留在未知结果。 | 记录 confirmed→maybe_held，不把丢失回执当作未执行。<br>Stop 对 maybe_held 发送幂等 up；无法确认释放则保留 Faulted/未知账本并禁止新 run。 | 自动重放 down<br>标记成功或已释放<br>用新 run 覆盖旧账本 | command_id/seq<br>disposition<br>ledger before/after<br>state<br>timeline<br>logs+sha256 | `protocol_fixture`; CW2 | pending |
| F-CP-02 | **上行 ACK 丢失（release）**: fake actuator 应用 key_up 后丢弃回执；或回执在超时后到达。 | 得到确定响应前不得清除 maybe_held。<br>超时后重复 up 并保持 Faulted，直到确认；不得把重复 up 当新输入。 | 假释放<br>把超时回执当成功<br>静默继续派发 | command_id/seq<br>release 次数<br>ledger<br>state<br>timeline<br>logs+sha256 | `protocol_fixture`; CW2 | pending |
| F-CP-03 | **半包、超长与畸形控制帧**: 在管道写入截断长度头、声明超大 payload、非法 JSON/字段类型。 | 解析有界，拒绝并返回明确错误；连接策略有界。<br>拒绝前后状态不变，不触发派发或释放。 | 进程崩溃<br>无界分配<br>把畸形帧当合法命令<br>无限重连 | frame bytes/上限<br>error code<br>state before/after<br>process exit<br>logs+sha256 | `protocol_fixture`; CW2 | pending |
| F-CP-04 | **seq 错乱与重复旧命令**: 重发旧 seq、跳号 seq、同一 command_id 重复提交。 | 旧/重复命令在执行前被拒绝；去重缓存有界。<br>不得因重放再次产生 down/up。 | 重复派发<br>用无界缓存吞掉内存<br>把跳号当新代次 | command_id/seq<br>dedup result<br>dispatch count<br>logs+sha256 | `protocol_fixture`; CW2 | pending |
| F-CP-05 | **旧 generation 命令**: 连接换代后仍提交上一代 run/document 命令。 | 派发前 fence，返回 GENERATION_STALE；旧代次不写目标。<br>不得把旧命令迁移到新代次执行。 | 执行旧命令<br>静默丢弃但不返回错误<br>沿用旧 held 假设 | generation before/after<br>disposition<br>dispatch count<br>state<br>logs+sha256 | `protocol_fixture`; CW2 | pending |
| F-CP-06 | **派发在途超时（结果未知）**: actuator 延迟响应超过 deadline，随后可能实际执行。 | 标记结果未知并入 maybe_held；Stop 先 fence 再中和。<br>不得自动重放原命令；未知未确认则 Faulted。 | 把超时当拒绝<br>自动重试 down<br>宣称云端未执行 | deadline<br>disposition<br>ledger<br>fence 顺序<br>state<br>timeline | `protocol_fixture`; CW2 | pending |
| F-CP-07 | **慢 RPC 不得阻塞续租与安全释放**: 在按住键时注入长截图/长 CDP 调用，同时观察租约 tick 与 Stop。 | renew 独立于长 RPC；2 秒长按不被 350ms 租约误释放。<br>Stop/release 不排在图像路径后面，有明确总预算。 | 长 RPC 共锁续租<br>用超时自动松键掩盖<br>Stop 等待无界 | hold duration<br>renew timestamps<br>lease state<br>stop latency p50/p95<br>logs+sha256 | `protocol_fixture`; CW2 | pending |
| F-CP-08 | **pending 队列超过 64**: 快速提交超过上限的命令并混入 Stop/release。 | 过载明确拒绝/背压；安全信号不被大图像或普通命令阻塞。<br>拒绝不改变已确认账本。 | 无界排队<br>丢弃 release/fault 信号<br>用队列长度冒充健康 | queue depth<br>reject code<br>safety latency<br>ledger<br>logs+sha256 | `protocol_fixture`; CW2 | pending |
| F-CP-09 | **Stop 时命令队列非空**: 在长动作与多个未完成命令期间触发 Stop。 | 先 fence，再丢弃后续命令，再中和 held∪maybe；有界等待并报告未确认项。<br>Stop 之后不得再出现目标输入。 | 先排空工作队列再 fence<br>Stop 后继续派发<br>把取消当成功 | queue snapshot<br>fence order<br>neutralized keys<br>post-stop dispatch count<br>exit_code | `protocol_fixture`; CW2 | pending |

### D2 租约、owner 与代次 (`lease_ownership`)

Mapped: CW9-2 / CW9-4. Sources: Review §6.2; FRD §9.3; Plan CW9-2/CW9-4.

| ID | Fault / injection | Expected | Forbidden | Evidence | Layer / depends | Status |
| --- | --- | --- | --- | --- | --- | --- |
| F-LS-01 | **持有期间租约过期**: 暂停 renew，让 owner lease 过期；保持键按下。 | tick 周期内 fence 代次并释放 held；迟到命令被拒绝。<br>不得让过期 owner 继续写目标。 | 过期后继续派发<br>不释放直接换代<br>把 tick 当可选优化 | lease expiry<br>tick time<br>generation +1<br>released keys<br>late command disposition | `protocol_fixture`; CW2 | pending |
| F-LS-02 | **renew 通道丢失或延迟**: 阻断 renew 通道或注入延迟，让安全预算先到期。 | 在继续不安全写入前进入 Faulted，UI/诊断可见。<br>不得静默续用旧租约。 | 假续租<br>无提示继续任务<br>等待超过安全预算 | renew gaps<br>fault transition time<br>ui state<br>ledger<br>logs+sha256 | `protocol_fixture`; CW2 | pending |
| F-LS-03 | **owner 冲突与第二写入者**: 第二个 Host/会话携带同一 session 身份请求 owner。 | 拒绝第二 owner；同一 session 不产生两个可写 controller。<br>不得双写或互相覆盖 held。 | 第二个 owner 静默接管<br>双写目标<br>清除第一个 owner 的账本 | owner ids<br>reject code<br>write count<br>pool identity<br>logs+sha256 | `protocol_fixture`; CW2, CW5 | pending |
| F-LS-04 | **Host 管道 EOF**: Host 端关闭控制管道，同时保留目标页面。 | 识别 EOF，释放 held 并 fence；退出/回收有界。<br>不得留下继续派发的孤儿 sidecar。 | EOF 后继续输入<br>把 EOF 当任务成功<br>留下孤儿进程 | EOF time<br>released keys<br>generation<br>process inventory<br>exit_code | `protocol_fixture`; CW2 | pending |
| F-LS-05 | **hello token 错误**: 使用错误/空 token 发起 hello。 | 拒绝连接，不创建 session，不泄露目标信息。<br>正确 token 仍可建立会话（正对照）。 | 错误 token 进入 Ready<br>错误信息泄露 token/路径<br>无限重试 | hello request/response<br>session created?<br>logs+sha256 | `protocol_fixture`; CW2 | pending |
| F-LS-06 | **重连后 generation 递增与旧状态隔离**: 断开后重连，复用旧 run/document/geometry 与旧 snapshot token。 | 代次递增；旧 pending/queued/token 全部失效。<br>新会话不继承旧 held 假设，重新识别 document 与 geometry。 | 旧 token 继续识别<br>跨代次复用 held<br>把重连当同一 run | old/new generation<br>token validity<br>held reset<br>document identity<br>logs+sha256 | `protocol_fixture`; CW2, CW4 | pending |

### D3 Stop、取消与中和 (`stop_neutralize`)

Mapped: CW9-2 / CW9-5. Sources: Review §6.2; FRD §9.1 最小停止与断链退路; Plan CW9-2/CW9-5.

| ID | Fault / injection | Expected | Forbidden | Evidence | Layer / depends | Status |
| --- | --- | --- | --- | --- | --- | --- |
| F-ST-01 | **confirmed 与 maybe_held 并集释放**: 同时制造已确认按下与回执未知的键/按钮，然后 Stop。 | 遍历并集，逐项发送幂等 up；每项有独立结果。<br>已确认与未知项都不得被遗漏。 | 只遍历 confirmed<br>用一次总 release 冒充逐项<br>未确认项标成功 | ledger snapshot<br>per-key release result<br>residual ledger<br>state | `local_browser`; CW2 | pending |
| F-ST-02 | **释放失败或浏览器无响应**: Stop 时让目标页/浏览器不可达，使 up 无法确认。 | 保留 Faulted/未知账本，禁止新 run；UI 与诊断明确。<br>不得把 kill 进程或关闭管道等同于云端已松键。 | 假成功<br>自动开新 run<br>宣称云端按键已释放 | release attempts<br>error/timeout<br>faulted state<br>new-run blocked<br>logs+sha256 | `local_browser`; CW2 | pending |
| F-ST-03 | **长动作中 cancel**: 在导航/战斗长动作与持续 held 输入期间取消任务。 | 先 fence，再释放，再有界等待；取消结果与释放结果分开报告。<br>Stop 后不再产生目标输入，恢复不重放旧输入。 | 取消普通工作队列代替 fence<br>取消后继续动作<br>恢复时重放 | cancel time<br>fence order<br>release result<br>post-stop inputs<br>exit_code | `local_browser`; CW2, CW3 | pending |
| F-ST-04 | **页面已死时 close**: 关闭/崩溃目标页后执行 close，无法执行 up。 | 记录不可确认的残留输入，按 owned session 策略终结并留证。<br>不得猜测已释放，也不得擅自终结用户浏览器。 | 把 close 当 100% 释放<br>杀用户浏览器<br>省略未知账本 | close time<br>unconfirmed ledger<br>session terminal state<br>process ownership<br>logs+sha256 | `local_browser`; CW2 | pending |
| F-ST-05 | **Stop 预算实测**: 在持有输入、慢 RPC、队列非空三种条件下重复 Stop。 | 记录 p50/p95 停止时延与最坏路径；安全信号不等待图像。<br>超预算如实 Fail，不取平均掩盖长尾。 | 只报单次最好值<br>用平均掩盖长尾<br>把 exit 0 当释放成功 | samples<br>p50/p95<br>path breakdown<br>exit_code<br>logs+sha256 | `local_browser`; CW2 | pending |
| F-ST-06 | **远端残留输入不可确认的收口**: 浏览器/页面无法响应，close 后仍无法确认云端 held 是否释放。 | 保留未知账本与 Faulted，禁止新 run；记录可恢复或结束专属会话的退路。<br>用户可见地要求人工确认，不以 kill 进程或关闭管道推断已释放。 | 宣称已释放<br>自动重开并继承旧 held<br>删除未知账本 | unknown ledger<br>manual confirmation<br>new-run blocked<br>session exit path<br>logs+sha256 | `real_cloud_session`; CW2, CW7 | pending |

### D4 媒体进度、帧健康与几何 (`media`)

Mapped: CW9-3. Sources: Review §6.2; FRD §9.3; Plan CW9-3.

| ID | Fault / injection | Expected | Forbidden | Evidence | Layer / depends | Status |
| --- | --- | --- | --- | --- | --- | --- |
| F-MD-01 | **rVFC 回调停止但页面仍存活**: 停止视频呈现/挂起 rVFC 回调，保持页面与截图 RPC 可用。 | 媒体进度停走，判 MEDIA_STALLED；帧不标 Fresh。<br>截图 RPC 成功不能替代媒体进度。 | 用截图计数冒充进度<br>冻结画面标 Fresh<br>继续基于旧反馈控制 | media progress<br>frame age<br>health state<br>screenshot counter separately<br>logs+sha256 | `local_browser`; CW2 | pending |
| F-MD-02 | **重复相同帧**: 保持静止画面，重复取帧；像素相同但媒体仍在推进。 | freshness 由媒体进度判定，像素相同不自动等于死流。<br>token/序号区分同一帧与新帧。 | 按像素哈希判死流<br>把重复帧当新反馈<br>跳过健康检查 | pixel hash<br>media progress<br>token sequence<br>health state | `local_browser`; CW2 | pending |
| F-MD-03 | **latest slot A/B 覆盖**: 在读取 A 帧时快速写入 B 帧，检查像素与 metadata 是否同源。 | 单次快照像素与 metadata 原子一致；旧 token 失效。<br>不得混合 A 的像素与 B 的 geometry/序号。 | 半新半旧<br>读到 torn frame<br>用 B metadata 解释 A 像素 | token A/B<br>pixel+metadata hash<br>slot state<br>reader result | `local_browser`; CW2 | pending |
| F-MD-04 | **旧 snapshot token 用于识别**: 在产生新帧/新 geometry 后继续提交旧 token。 | 拒绝 stale token，识别回到新 observation。<br>不得用旧 token 的坐标执行动作。 | 接受旧 token<br>静默替换为新帧<br>用旧几何点击 | token age<br>reject code<br>geometry revision<br>action blocked<br>logs+sha256 | `local_browser`; CW2 | pending |
| F-MD-05 | **截图 RPC 成功但媒体冻结**: 冻结视频后仍允许 screenshot 返回成功。 | 健康判定仍为 stale/frozen；不得只凭 RPC 成功标 Fresh。<br>分别记录截图成功与媒体进度。 | 把 screenshot 成功当直播<br>混用两个计数器<br>继续自动任务 | screenshot result<br>media progress<br>health<br>task paused?<br>logs+sha256 | `local_browser`; CW2 | pending |
| F-MD-06 | **页面重建与新 document**: 刷新/重导航目标页，替换 video 与 document。 | 旧 collector epoch/token 失效；迟到回调被 epoch 丢弃。<br>重新获取 video/source/track，Start/Stop 幂等。 | 跨 document 复用回调<br>迟到回调写入新状态<br>重复订阅 | old/new document id<br>epoch<br>late callback count<br>token validity<br>logs+sha256 | `local_browser`; CW2 | pending |
| F-MD-07 | **geometry 变化与 resize**: 改变视口/视频尺寸，触发 geometry revision。 | revision 递增，旧坐标/token 失效；重新做 canonical 检查。<br>不得沿用旧 ROI 或点击坐标。 | geometry 恒 0<br>旧坐标继续点击<br>把非 canonical 帧放大冒充 | geometry before/after<br>revision<br>canonical check<br>old token reject<br>logs+sha256 | `local_browser`; CW2 | pending |
| F-MD-08 | **非 1280×720 视频**: 将视频源设为非 canonical 尺寸。 | 不出帧，返回明确错误；session 以 FRAME_STALE/等价错误拒绝。<br>不得用 upscale 或截图裁剪冒充 canonical。 | 输出非 canonical 帧<br>静默缩放<br>把截图当视频帧 | videoWidth/Height<br>error code<br>frame emitted?<br>session state | `local_browser`; CW2 | pending |

### D5 浏览器、CDP 与云会话 (`browser_session`)

Mapped: CW9-3 / CW9-4. Sources: Review §6.3; FRD §9.1/§10.1 B02; Plan CW9-3/CW9-4.

| ID | Fault / injection | Expected | Forbidden | Evidence | Layer / depends | Status |
| --- | --- | --- | --- | --- | --- | --- |
| F-BR-01 | **CDP 断链**: 在持有输入时断开 CDP 连接。 | 输入派发显式失败，进入 fence/release；不静默回退 raw CDP 或 Win32。<br>重连前不继续派发。 | 回退 Win32/OS 焦点<br>断链后继续输入<br>跳过 release | CDP state<br>dispatch error<br>ledger<br>fence time<br>reconnect policy<br>logs+sha256 | `local_browser`; CW2 | pending |
| F-BR-02 | **浏览器进程崩溃**: 强杀专属浏览器进程，观察 Host/sidecar 状态与残留输入。 | session 进入 Faulted/终止；只回收自有进程；残留输入按未知账本处理。<br>不得宣称云端已释放，不得杀用户浏览器。 | 假释放<br>留下孤儿 Chrome<br>自动重开并继承旧 held | process tree before/after<br>exit code<br>unknown ledger<br>session state<br>logs+sha256 | `local_browser`; CW5 | pending |
| F-BR-03 | **目标页关闭/重建**: 关闭目标 target 后出现同名页面或新 target。 | 按 target identity 拒绝错误页面，不选第一个 page；旧 token 失效。<br>重新发现需显式确认身份。 | 选第一个同名页面<br>沿用旧 target<br>静默重连 | target id/url<br>selection result<br>token validity<br>logs+sha256 | `local_browser`; CW2 | pending |
| F-BR-04 | **云会话强制登出或超时**: 在任务中输入/等待至云会话被强制登出，或模拟 auth 过期。 | 检测为会话终止；UI 明确；不拿旧 auth 盲重试。<br>释放/未知账本按 Stop 规则处理。 | 盲重试登录<br>把登出当任务成功<br>泄露 token/cookie/UID | logout evidence (redacted)<br>state transition<br>retry policy<br>release result<br>logs+sha256 | `real_cloud_session`; CW2 | pending |
| F-BR-05 | **重连/新会话的身份重建**: 断线后重连到同一实例或新实例。 | 重新识别 instance、geometry、input_ver；不继承旧 held/token。<br>身份不匹配则拒绝。 | 沿用旧 input_ver<br>把新实例当旧实例<br>跳过 geometry 重建 | instance/rtcType/input_ver<br>geometry revision<br>held reset<br>logs+sha256 | `real_cloud_session`; CW2 | pending |
| F-BR-06 | **自有浏览器与用户浏览器边界**: 在用户已有 Chrome 会话旁启动专属 profile，并触发清理。 | 只 attach/drive 自有实例；清理只针对自有进程/profile。<br>不得按进程名批量 kill 或触碰用户浏览器。 | 按进程名 kill<br>attach 用户页面<br>复用用户 profile | process ownership<br>profile path (redacted)<br>cleanup scope<br>user browser untouched<br>logs+sha256 | `local_browser`; CW5 | pending |

### D6 Host、GUI 与进程生命周期 (`host_lifecycle`)

Mapped: CW9-4. Sources: Review §6.2; FRD §9.3; Plan CW9-4.

| ID | Fault / injection | Expected | Forbidden | Evidence | Layer / depends | Status |
| --- | --- | --- | --- | --- | --- | --- |
| F-HS-01 | **MXU 崩溃**: 强杀 MXU/Host 进程，保留 sidecar 与浏览器。 | sidecar 检测 Host death，释放/fence 并有界退出；浏览器按 owned 策略回收。<br>无孤儿进程、无持续输入。 | 孤儿 sidecar<br>继续派发<br>只杀浏览器不释放账本 | process tree before/after<br>release/fence<br>exit code<br>orphan check<br>logs+sha256 | `local_browser`; CW2, CW5 | pending |
| F-HS-02 | **GUI/WebView 崩溃但后端存活**: 强杀 GUI 窗口/WebView，保留 controller 与任务。 | API 有界响应，controller 状态可查询；无卡死写锁。<br>任务结果不因 GUI 消失而伪造成功。 | API 无响应<br>任务状态丢失<br>后台继续无限派发 | API latency<br>controller state<br>task state<br>process inventory<br>logs+sha256 | `local_browser`; CW5 | pending |
| F-HS-03 | **state API 长时间无响应**: 在任务运行中让 `/api/maa/state` 持续超时；同时观察 supervisor 与任务。 | 请求有界、诊断可见；supervisor 的超时策略不得无证据地杀死运行中任务。<br>任务是否仍在执行、是否进入菜单，必须有独立证据，不由 API 单一来源推断。 | 把 API 超时当任务失败<br>无界等待<br>轮询超时误杀<br>宣称根因已定 | API latency/samples<br>supervisor policy<br>task process state<br>task result evidence<br>logs+sha256 | `real_cloud_session`; CW5, CW7 | pending |
| F-HS-04 | **control pipe 中途关闭**: 在任务中关闭 MXU control pipe（或模拟写失败）。 | 显式失败并保留原始错误；不静默重试或改写状态。<br>按 owned 进程与未知账本处理，留下可定位证据。 | 静默重连并继续<br>把写失败当成功<br>删除原始日志 | pipe error<br>first failure time<br>backend last_error<br>observation age<br>exit code<br>logs+sha256 | `local_browser`; CW5 | pending |
| F-HS-05 | **双实例与同 session 争写**: 启动第二个 MaaEnd/MXU 实例并尝试连接同一 session。 | pool/run/session identity 阻止第二个可写 controller；失败不入 pool。<br>不得产生双写或跨实例事件。 | 两个可写 controller<br>跨实例覆盖任务<br>共享 held | instance ids<br>pool state<br>second connect result<br>write count<br>logs+sha256 | `local_browser`; CW5 | pending |
| F-HS-06 | **idle 回收与进程所有权**: 任务结束/空闲后检查 MaaEnd、MXU、sidecar、Chrome 进程与临时 profile。 | 只回收自有进程；无多个 MaaEnd 实例；无用户浏览器影响。<br>回收失败有界重试并留证，不按进程名批量 kill。 | kill 用户浏览器<br>留下孤儿进程<br>删除用户 profile | process inventory<br>ownership<br>temp/profile cleanup<br>retry result<br>logs+sha256 | `local_browser`; CW5 | pending |
| F-HS-07 | **Host-death（父进程强杀）**: 对 Host 父进程发送强杀信号，观察 sidecar 的父死检测与退出。 | sidecar 在有限时间内检测父死，释放/fence 并退出；owned Chrome 按策略回收。<br>无孤儿、无持续输入。 | sidecar 永久存活<br>跳过释放<br>未授权终止用户进程 | parent pid/child pid<br>detection time<br>release/fence<br>exit code<br>orphan check | `local_browser`; CW2 | pending |

### D7 人工接管与所有权转移 (`manual_takeover`)

Mapped: CW9-5. Sources: FRD §9.1 手动接管; Review §6.3; Plan CW9-5.

| ID | Fault / injection | Expected | Forbidden | Evidence | Layer / depends | Status |
| --- | --- | --- | --- | --- | --- | --- |
| F-MT-01 | **pause→fence→release→owner 转移**: 在自动化持有输入时请求人工接管。 | 按顺序停止自动化、fence、释放、确认后转移 owner；状态可见。<br>转移后自动化不得再写入。 | 未释放就转移<br>转移后自动输入<br>把暂停当已释放 | state transitions<br>fence time<br>release result<br>owner ids<br>post-transfer inputs | `real_cloud_session`; CW2, CW3 | pending |
| F-MT-02 | **接管后恢复识别**: 人工操作改变界面/镜头后恢复自动化。 | 重新取帧与识别，使用新 document/geometry/token；不重放接管前输入。<br>旧 held/旧坐标不得沿用。 | 恢复时重放旧输入<br>沿用旧 frame/token<br>跳过重新识别 | new frame token<br>geometry<br>re-identification result<br>replayed inputs=0<br>logs+sha256 | `real_cloud_session`; CW2, CW3, CW4 | pending |
| F-MT-03 | **接管时仍有 held 键**: 在 W/右键/技能按住时请求接管并立即断开自动化。 | 转移前逐项释放并确认；无法确认则保持 Faulted，不得把卡键留给用户。<br>用户获得明确的所有权状态。 | 把卡键留给用户<br>未确认就标释放<br>继续自动化 | held ledger<br>per-key release<br>residual state<br>owner transfer<br>ui message | `real_cloud_session`; CW2, CW3 | pending |

### D8 长运行、资源与跨 run 隔离 (`long_run`)

Mapped: CW9-6. Sources: FRD §9.3; Review §7.1 CW9; Plan CW9-6.

| ID | Fault / injection | Expected | Forbidden | Evidence | Layer / depends | Status |
| --- | --- | --- | --- | --- | --- | --- |
| F-LR-01 | **100 次 connect/start/cancel/disconnect**: 固定候选上循环 100 次连接、启动、取消、断开；每轮记录退出与清理。 | 每轮有界停止，无孤儿进程/临时 profile/端口残留；失败可复现。<br>不得只统计成功轮次或跳过失败轮。 | 只报成功轮<br>跳过清理<br>用平均掩盖失败 | cycle results<br>exit codes<br>process/temp inventory<br>failure list<br>logs+sha256 | `real_cloud_session`; CW9-2, CW9-3, CW9-4, CW9-5 | pending |
| F-LR-02 | **2 小时混合任务、预览与重连**: 连续运行 2 小时，混合任务、只读预览、断线重连与空闲。 | 记录 frame age p50/p95、资源趋势与清理后基线；无崩溃/跨 run 事件。<br>不得用一次短时成功代替长运行。 | 只看 RSS 一次升高<br>省略 p95<br>把重连当新 run 而不清理 | duration<br>frame age p50/p95<br>resource trend<br>reconnect results<br>cleanup baseline<br>logs+sha256 | `real_cloud_session`; CW9-2, CW9-3, CW9-4, CW9-5 | pending |
| F-LR-03 | **重复失败后的资源清理**: 连续制造连接失败、启动失败与取消失败，检查残留。 | 失败路径同样释放/回收；下一轮不继承旧锁、旧代次、旧进程。<br>失败如实保留，不因清理成功改成 Pass。 | 失败后不清理<br>用清理成功掩盖功能失败<br>复用旧 profile | failure cases<br>cleanup result<br>next-cycle state<br>logs+sha256 | `local_browser`; CW9-2, CW9-4 | pending |
| F-LR-04 | **跨 run 事件、token 与账本隔离**: 在 run A 结束后立即启动 run B，并注入 A 的迟到事件/回执。 | A 的事件、token、held 账本不得进入 B；迟到回调被拒绝。<br>B 不得因 A 的未知账本被污染。 | 跨 run 复用状态<br>迟到事件写入 B<br>A 的 held 留在 B | run ids<br>late event disposition<br>token validity<br>ledger isolation<br>logs+sha256 | `local_browser`; CW9-2, CW9-4 | pending |
| F-LR-05 | **controller 关闭后资源归零**: 在任务与预览均关闭后检查 callback、订阅、URL、进程与内存基线。 | owned callback/订阅/URL/进程归零；预热后无持续增长。<br>关闭结果与任务结果分开报告。 | 只测一次 RSS<br>把进程存活当正常<br>忽略 callback 泄漏 | callback count<br>subscriptions<br>process/url inventory<br>memory after warmup<br>logs+sha256 | `local_browser`; CW9-2, CW9-4 | pending |
| F-LR-06 | **停止后无后台重连或新输入**: 任务 Stop/close 后继续观察 5 分钟，检查是否自动重连或产生输入。 | 停止后无新输入、无自动重连、无后台任务；进程按策略退出。<br>不得把“未观察到”写成“已证明无残留”。 | Stop 后重连<br>后台继续派发<br>省略观察窗口 | stop time<br>post-stop events<br>process state<br>observation window<br>logs+sha256 | `real_cloud_session`; CW9-2, CW9-4 | pending |

## Evidence schema

Required fields for every row: candidate_sha; runtime（MaaEnd/MXU/sidecar/MaaFramework/browser）; fault_injection（hook/信号/断链方式与参数）; identity（run/document/geometry generation + seq）; before_state（owner/generation/confirmed+maybe ledger）; timeline_ms; observed_state（状态机/disposition/ledger）; process_inventory（自有 PID 前后；用户浏览器未触碰）; frames（media progress/frame age/token/canonical）; ui_diagnostic; exit_code; logs+sha256; unverified.

缺任一字段不得标 pass；fixture 结果不能替代 real_cloud_session 行；fake/fixed PNG/fake SDK/独立 Node 脚本不计；旧候选证据在候选变化后失效。

## Observed inputs

These are historical observations registered as inputs, not root-cause conclusions and not evidence for the current candidate.

| ID | Run | Mapped faults | Observation | Root cause |
| --- | --- | --- | --- | --- |
| OBS-01 | `run-20261024-175718` | F-HS-03 | 6 次连续 state_poll_error，MXU API 持续 >60s 不可用，exit 2；任务已进入 MenuListEnterMenuRegionalDevelopment，无法从 API 取得结果。 | unverified |
| OBS-02 | `run-20261024-172548` | F-HS-04 | CDP 鼠标实验期间 MXU control pipe 关闭；screencap 连续失败，backend last_error=cloudweb_cloudweb_control_write，observation_age_ms=4930；exit 7。实验提交已 revert，不入候选。 | unverified; not attributable to the current candidate |
| OBS-03 | `run-20261024-175512` | F-HS-03 | 5s 状态轮询超时触发误杀，exit 7；属 supervisor 策略问题，不归因产品 controller。 | supervisor timeout policy |

## Non-claims

- 本文件只定义故障矩阵；51 行全部 pending，未执行任何故障注入。
- OBS-01/02/03 是历史运行输入，根因未闭合，不代表当前候选行为。
- CW9-1 verified 仅指矩阵与证据 schema 已建立，不关闭 CW9-2..6，也不代表 G4。
- 不创建 GitHub issue/PR/评论；不泄露 token/cookie/UID/本机路径。
