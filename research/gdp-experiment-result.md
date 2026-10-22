# gdp 最小无安装实验结果

执行日期：2026-10-21。对象为本轮已组装包 `package/quality/gdp` 与 `package/quality/e2e` 的候选文件；未安装依赖、调用模型/浏览器、执行 e2e、修改源码或降低测试标准。

## 环境与资源

- Node：`v24.19.0`。
- 执行前只读盘点 node/nodejs/npm/npx/pnpm/tsc/tsgo 同类进程，结果为空。
- 命令均有硬超时：原库测试30秒，单文件语法检查30秒，批次外层55秒；没有启动常驻服务。
- 先前只读检查 runtime-owned 模块目录，没有 `typescript/lib/tsc.js`、`typescript/bin/tsc`、native-preview 或 typescript6，因此没有安装编译器，类型检查仍Pending。

## 实际结果

| 检查 | 命令退出码 | 结论 |
| --- | --- | --- |
| 上游4项原库运行时语义 | 0 | 4通过、0失败、0跳过；约259毫秒 |
| gdp候选6文件与e2e config/spec，共8文件的Node语法检查 | 每文件0 | 仅语法通过，不等于类型检查或功能执行 |
| vendored gdp源码直接 `node --check` | 1 | `declare const NAME`处SyntaxError，保留失败 |
| 同源码显式module-typescript输入的`--check`诊断 | 1 | 同处SyntaxError；同一目标两次失败，停止该检查路径 |
| gdp编译期正例/四个负例 | 未运行 | Pending：无预装TypeScript |
| 候选proof验证器/可信运行时reader | 未运行 | Pending；reader仍明确拒绝 |
| e2e页面/媒体与Windows/Native/真实云游戏 | 未运行 | Pending，原库语义和语法检查均不能替代 |

## 原库语义命令与原始输出

工作目录：`package/quality/gdp`。

```bash
timeout --signal=TERM --kill-after=5s 30s node --test upstream-test/runtime.test.ts
```

工具实际退出0，完整合并输出如下；其中未观察到stderr警告，本次工具输出未分别标注stdout/stderr：

```text
✔ name passes values through and returns the callback result (1.513355ms)
✔ name works with async callbacks (0.203777ms)
✔ named values are frozen (1.11082ms)
✔ proofs are a frozen singleton carrying only their kind (0.35248ms)
ℹ tests 4
ℹ suites 0
ℹ pass 4
ℹ fail 0
ℹ cancelled 0
ℹ skipped 0
ℹ todo 0
ℹ duration_ms 258.76287
```

这些结果只证明原库name传值、异步回调、wrapper freeze和同kind proof singleton语义。它们不证明授权检查、generation、构建/套件绑定、可信结果数据源或任何MaaEnd能力。

## 语法检查逐项记录

每条用Python subprocess运行`node --check <file>`，独立30秒超时，分别保留stdout和stderr。

| 文件（相对package） | 退出码 | stdout | stderr |
| --- | --- | --- | --- |
| `quality/gdp/misuse.expected.ts` | 0 | 空 | 空 |
| `quality/gdp/misuse.negative.ts` | 0 | 空 | 空 |
| `quality/gdp/positive.ts` | 0 | 空 | 空 |
| `quality/gdp/proofs/proof-boundary.ts` | 0 | 空 | 空 |
| `quality/gdp/runtime-bridge.ts` | 0 | 空 | 空 |
| `quality/gdp/upstream-test/runtime.test.ts` | 0 | 空 | 空 |
| `quality/gdp/vendor/gdp/index.ts` | 1 | 空 | 下方原始异常 |
| `quality/e2e/e2e.config.ts` | 0 | 空 | 空 |
| `quality/e2e/tests/real-media.e2e.ts` | 0 | 空 | 空 |

未标注负例的`--check`退出0是预期的语法层现象，不是类型负例已经通过；必须等TypeScript checker验证指定错误。

直接文件检查原始stderr（仅本机绝对前缀替换为<package>，异常内容未改）：

```text
<package>/quality/gdp/vendor/gdp/index.ts:29
declare const NAME: unique symbol;
        ^^^^^

SyntaxError: Unexpected token 'const'
    at wrapSafe (node:internal/modules/cjs/loader:1804:18)
    at checkSyntax (node:internal/main/check_syntax:76:3)

Node.js v24.19.0
```

## 一次有边界的诊断

只读确认最近的`quality/gdp/package.json`含`"type":"module"`，vendor路径下无覆盖它的package.json。Node帮助列出`--input-type`与TS strip选项。为区分普通文件模式与显式TypeScript输入模式，第二次只对同一固定源码通过stdin执行：

```text
node --input-type=module-typescript --check
```

实际退出1、stdout空，原始stderr：

```text
[stdin]:29
declare const NAME: unique symbol;
        ^^^^^

SyntaxError: Unexpected token 'const'
    at wrapSafe (node:internal/modules/cjs/loader:1804:18)
    at checkSyntax (node:internal/main/check_syntax:76:3)
    at node:internal/main/check_syntax:45:5
    at Socket.<anonymous> (node:internal/process/execution:205:5)
    at Socket.emit (node:events:521:24)
    at endReadableNT (node:internal/streams/readable:1736:12)
    at process.processTicksAndRejections (node:internal/process/task_queues:90:21)

Node.js v24.19.0
```

同一vendor源码已被原库测试成功作为ESM导入，而`--check`两种输入均在TS声明处失败，支持“本环境check路径与运行时TS加载路径不同”的推断；本轮没有进一步断言Node具体实现根因。已按两次失败规则停止该路径，不剥除声明或改写上游源码来制造通过。后续应使用已审核TypeScript checker；不能将这两次失败隐去或把本包标成全检查通过。

所有验证都是一次性命令。结束后复查同类进程与本次子进程树；无本次常驻服务或监听端口需要保留。

## 媒体候选后续修订

本报告的批次语法检查后，修正了WebRTC srcObject支持；受改real-media.e2e.ts再次单文件node --check，退出0、stdout/stderr为空。未追加浏览器或e2e执行，也未再次尝试两次失败的vendor检查。最终源码完整性以checksums.json为准。
