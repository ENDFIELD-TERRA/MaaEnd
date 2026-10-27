# WebView2 visual-hosting wheel experiment

Standalone research prototype (not a product target). It answers one question:

> Does `ICoreWebView2CompositionController::SendMouseInput` deliver a wheel event to the page while the
> host window is covered, without moving the real cursor or taking foreground?

## Measured result (see the local evidence report for the full matrix)

- Wheel messages were delivered while a topmost cover window was over the host (`WindowFromPoint` verified
  from a secondary process): 1 DOM `wheel` event per `SendMouseInput`, `isTrusted === true`, correct pane
  and scroll movement, no duplicates (including back-to-back bursts).
- Client point `(300,300)` hit the left pane, `(900,300)` the right pane. The point is in physical WebView
  client coordinates: at rasterization scale 1.0 the DOM saw `(300,300)`, at 1.5 it saw `(200,200)`.
- `mouseData = +120` produced DOM `deltaY = -100` and 100 px of scroll; `+240` produced `-200`;
  horizontal wheel `+120` produced `deltaX = +100` and 100 px of horizontal scroll.
- The host window was never activated or foregrounded, and the page kept `document.visibilityState =
  "visible"` while covered.
- `SendMouseInput` needs `put_IsVisible(TRUE)`. Hiding, minimizing, or moving the host off-screen still
  delivered wheel input; setting `IsVisible` to false stopped it.
- Pointer move is different: `SendMouseInput(MOVE)` was delivered while the host was visible but produced
  no `mousemove` while covered. Wheel-only adoption is therefore not the same as a complete pointer bridge.
- A trusted synthetic click followed by `requestPointerLock()` was granted (`document.pointerLockElement`
  became non-null). An embedder that must not lock the real cursor has to keep an injected reject policy.
- `PrintWindow(PW_RENDERFULLCONTENT)` captured the composed page both while visible and while covered in
  this environment; desktop `BitBlt` showed the cover window when covered.

## Layout

```
experiments/visual-hosting/
  CMakeLists.txt          standalone CMake project (not referenced by CloudShell)
  src/main.cpp            matrix harness, interactive harness, --wfp helper mode
  src/VisualHost.*        Win32 window + DirectComposition + WebView2 composition controller
  src/Util.*              text/JSON helpers, WindowFromPoint helper process, BMP capture
  wheel-page.html         local two-pane probe page (no network, no account data)
```

## Build

Requirements: Windows x64, Visual Studio 2022/2026 BuildTools with the C++ x64 toolset,
CMake 3.24+, Ninja Multi-Config, and a WebView2 SDK (same layout as the CloudShell dependency).

```bat
call "<VS install>\VC\Auxiliary\Build\vcvarsall.bat" x64
cmake -S experiments/visual-hosting -B experiments/visual-hosting/build ^
      -G "Ninja Multi-Config" -DWEBVIEW2_SDK_DIR=<extracted Microsoft.Web.WebView2 root>
cmake --build experiments/visual-hosting/build --config RelWithDebInfo
```

The executable and `wheel-page.html` land in `experiments/visual-hosting/build/bin/RelWithDebInfo/`.
`WEBVIEW2_SDK_DIR` can also be supplied through the environment variable of the same name.

## Run

Matrix mode (creates the host window, runs all cases, writes a JSON report and BMP captures):

```bat
visual-hosting-wheel.exe --out report.json --evidence-dir evidence
```

Interactive mode for manual probing (host is shown with `SW_SHOWNOACTIVATE`):

```bat
visual-hosting-wheel.exe --interactive
```

Commands: `state`, `probe`, `reset`, `wheel X Y DELTA`, `hwheel X Y DELTA`, `move X Y`,
`cover on|off`, `topmost on|off`, `bottom`, `show on|off`, `min`, `vis on|off`, `scale S`,
`plock`, `shot NAME`, `wait MS`, `quit`.

## What the matrix does

- Creates a 1280x720 (physical pixels) top-level window, a DirectComposition target/visual, and a
  `CoreWebView2CompositionController`; loads `file:///.../wheel-page.html`; calls `put_Bounds`,
  `put_RootVisualTarget`, `put_IsVisible(TRUE)`.
- Never calls `SetForegroundWindow` and never moves the cursor. The cover is a topmost,
  `WS_EX_NOACTIVATE` plain window owned by the same process.
- Verifies occlusion with `WindowFromPoint` from a secondary process (the same executable, `--wfp X Y`).
- Runs the same wheel sequence while visible/topmost, covered, host-at-bottom plus cover, minimized,
  hidden, off-screen, and with `put_IsVisible(FALSE)`; phase B repeats key cases at the DPI-scaled
  rasterization ratio.
- Records per case: host HWND/rect, DPI, coverage, foreground before/after, cursor before/after,
  page visibility/focus, full probe events, scroll positions, HRESULTs, native wheel message count,
  and the verdict. A tight cursor/foreground probe samples immediately around `SendMouseInput`.

The page records `{type,x,y,deltaX,deltaY,target,trusted}` for `wheel` and `mousemove` in
`window.__probe`; the host reads it via `ExecuteScript` (no CDP).

## Caveats

- The test page is a local file; the prototype does not use the cloud game, an account, or the network.
- Wheel delta magnitude in the DOM (100 per 120 `mouseData`) is Chromium's pixel-based wheel conversion,
  not the raw `WHEEL_DELTA`; do not assume delta values of 120 in page handlers.
- `PrintWindow` support for composed content is environment-dependent; verify it on the target machines.
- `requestPointerLock` behaviour is raw WebView2 here; an automation host must enforce its own policy.
- The tight probe was run on a desktop that was also being used interactively, so per-case long-window
  cursor samples can contain user movement; the immediate before/after samples around the API call are the
  attribution evidence.
