// Standalone experiment: does ICoreWebView2CompositionController::SendMouseInput
// deliver wheel input while the host window is occluded?
//
// Modes:
//   visual-hosting-wheel.exe [--page PATH] [--out REPORT.json] [--evidence-dir DIR]
//                            [--udf DIR] [--x N] [--y N] [--width N] [--height N]
//                            [--interactive]
//   visual-hosting-wheel.exe --wfp X Y        (internal WindowFromPoint helper)
//
// The harness never calls SetForegroundWindow and never moves the physical cursor.

#include "Util.h"
#include "VisualHost.h"

#include <shellscalingapi.h>
#include <windows.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

struct Options {
    std::wstring page;
    std::wstring out;
    std::wstring evidenceDir;
    std::wstring userDataDir;
    int width = 1280;
    int height = 720;
    int x = 60;
    int y = 60;
    bool interactive = false;
    bool wfp = false;
    int wfpX = 0;
    int wfpY = 0;
};

struct CaseSpec {
    std::string name;
    std::string state;
    std::string kind;  // wheel | hwheel | move
    int x = 0;
    int y = 0;
    int delta = 0;
    int msgs = 1;
    int gapMs = 0;
    bool moveFirst = false;
};

struct CaseResult {
    std::string phase;
    CaseSpec spec;
    double rasterScale = 1.0;
    HWND hostHwnd = nullptr;
    HWND coverHwnd = nullptr;
    RECT hostRect {};
    RECT clientScreen {};
    UINT dpi = 96;
    bool hostVisible = false;
    bool hostIconic = false;
    WfpResult wfp;
    bool wfpIsHost = false;
    bool wfpIsCover = false;
    HWND fgBefore = nullptr;
    HWND fgAfter = nullptr;
    POINT cursorBefore {};
    POINT cursorAfter {};
    ProbeSnapshot before;
    ProbeSnapshot after;
    std::vector<HRESULT> sendHrs;
    int nativeWheelsBefore = 0;
    int nativeWheelsAfter = 0;
    int wheelEvents = 0;
    int moveEvents = 0;
    bool allTrusted = true;
    double sumDeltaX = 0.0;
    double sumDeltaY = 0.0;
    std::string coverage;
    std::string verdict;
    std::string note;
};

struct PhaseInfo {
    std::string name;
    double requestedScale = 1.0;
    bool scaleApplied = false;
    int viewportWidth = 0;
    int viewportHeight = 0;
    double dpr = 0.0;
    std::string note;
};

struct MonitorEntry {
    RECT rect {};
    UINT dpiX = 96;
    UINT dpiY = 96;
    bool primary = false;
    std::string device;
};

struct PointerLockResult {
    bool ran = false;
    std::string clickHrs;
    std::string stateCalled;
    std::string stateAfter;
    std::string error;
};

struct CursorProbeEntry {
    std::string state;
    POINT before {};
    POINT immediate {};
    POINT after {};
    HWND fgBefore = nullptr;
    HWND fgImmediate = nullptr;
    HWND fgAfter = nullptr;
    HWND hostHwnd = nullptr;
    HWND coverHwnd = nullptr;
    HRESULT hr = S_OK;
};

std::string JStr(const std::string& value)
{
    return "\"" + JsonEscape(value) + "\"";
}

std::string JBool(bool value)
{
    return value ? "true" : "false";
}

std::string HwndNum(HWND hwnd)
{
    return std::to_string(static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(hwnd)));
}

std::wstring CurrentExePath()
{
    std::wstring path(MAX_PATH, L'\0');
    while (true) {
        const DWORD len = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
        if (len == 0) {
            return {};
        }
        if (len < path.size()) {
            path.resize(len);
            return path;
        }
        path.resize(path.size() * 2);
    }
}

std::string RectArray(const RECT& rect)
{
    return "[" + std::to_string(rect.left) + "," + std::to_string(rect.top) + "," + std::to_string(rect.right) +
        "," + std::to_string(rect.bottom) + "]";
}

std::string PointArray(const POINT& point)
{
    return "[" + std::to_string(point.x) + "," + std::to_string(point.y) + "]";
}

std::string DoubleStr(double value)
{
    std::ostringstream out;
    out.precision(6);
    out << value;
    return out.str();
}

BOOL CALLBACK MonitorEnumProc(HMONITOR monitor, HDC, LPRECT, LPARAM data)
{
    auto* list = reinterpret_cast<std::vector<MonitorEntry>*>(data);
    MONITORINFOEXW info {};
    info.cbSize = sizeof(info);
    if (!GetMonitorInfoW(monitor, &info)) {
        return TRUE;
    }
    MonitorEntry entry;
    entry.rect = info.rcMonitor;
    entry.primary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
    entry.device = WideToUtf8(info.szDevice);
    UINT dpiX = 96;
    UINT dpiY = 96;
    if (FAILED(GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY))) {
        dpiX = 96;
        dpiY = 96;
    }
    entry.dpiX = dpiX;
    entry.dpiY = dpiY;
    list->push_back(entry);
    return TRUE;
}

std::string MonitorsJson()
{
    std::vector<MonitorEntry> monitors;
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));
    std::ostringstream out;
    out << "[";
    for (size_t i = 0; i < monitors.size(); ++i) {
        if (i) {
            out << ",";
        }
        const MonitorEntry& m = monitors[i];
        out << "{\"rect\":" << RectArray(m.rect) << ",\"dpi_x\":" << m.dpiX << ",\"dpi_y\":" << m.dpiY
            << ",\"scale\":" << DoubleStr(m.dpiX / 96.0) << ",\"primary\":" << JBool(m.primary)
            << ",\"device\":" << JStr(m.device) << "}";
    }
    out << "]";
    return out.str();
}

std::string EnvironmentJson(VisualHost* host, bool hostForegroundAfterCreate)
{
    DWORD sessionId = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
    POINT cursor {};
    GetCursorPos(&cursor);

    std::ostringstream out;
    out << "{";
    out << "\"generated_at\":" << JStr(TimestampUtc());
    out << ",\"session_id\":" << sessionId;
    out << ",\"process_id\":" << GetCurrentProcessId();
    out << ",\"command_line\":" << JStr(WideToUtf8(GetCommandLineW()));
    out << ",\"exe_dir\":" << JStr(WideToUtf8(ExeDirW()));
    out << ",\"system_dpi\":" << GetDpiForSystem();
    out << ",\"host_dpi\":" << (host ? host->dpi() : 0);
    out << ",\"screen_metrics\":[" << GetSystemMetrics(SM_CXSCREEN) << "," << GetSystemMetrics(SM_CYSCREEN) << "]";
    out << ",\"virtual_screen\":[" << GetSystemMetrics(SM_XVIRTUALSCREEN) << ","
        << GetSystemMetrics(SM_YVIRTUALSCREEN) << "," << GetSystemMetrics(SM_CXVIRTUALSCREEN) << ","
        << GetSystemMetrics(SM_CYVIRTUALSCREEN) << "]";
    out << ",\"monitors\":" << MonitorsJson();
    out << ",\"foreground_at_report\":" << WindowInfoJson(GetForegroundWindow());
    out << ",\"active_window\":" << WindowInfoJson(GetActiveWindow());
    out << ",\"cursor_at_report\":" << PointArray(cursor);
    out << ",\"host_foreground_after_create\":" << JBool(hostForegroundAfterCreate);
    out << "}";
    return out.str();
}

}  // namespace

// Forward declarations for functions defined after the anonymous namespace.
static std::string ProbeJson(const ProbeSnapshot& snapshot, bool withEvents);
static std::string CaseJson(const CaseResult& result);
static std::string CompactCaseJson(const CaseResult& result);
static CaseResult RunCase(VisualHost& host, const Options& options, const std::string& phase, double scale,
    const CaseSpec& spec);
static void ApplyState(VisualHost& host, const std::string& state, const Options& options);
static PointerLockResult RunPointerLockProbe(VisualHost& host, const Options& options);
static int RunInteractive(const Options& options);
static int RunHarness(const Options& options);
static int RunWfpMode(int x, int y);
static bool ParseArgs(int argc, wchar_t** argv, Options* options, std::string* error);
static void MaybeHideOwnConsole();

int wmain(int argc, wchar_t** argv)
{
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    SetConsoleOutputCP(CP_UTF8);

    Options options;
    std::string error;
    if (!ParseArgs(argc, argv, &options, &error)) {
        std::cout << "{\"error\":" << JStr(error) << "}\n";
        return 1;
    }
    if (options.wfp) {
        return RunWfpMode(options.wfpX, options.wfpY);
    }
    MaybeHideOwnConsole();

    const HRESULT comHr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    int code = 0;
    if (FAILED(comHr)) {
        std::cout << "{\"error\":" << JStr("CoInitializeEx failed: " + HexHr(comHr)) << "}\n";
        code = 2;
    }
    else {
        code = RunHarness(options);
        CoUninitialize();
    }
    return code;
}

namespace {

bool ParseArgsImpl(int argc, wchar_t** argv, Options* options, std::string* error)
{
    for (int i = 1; i < argc; ++i) {
        const std::wstring arg = argv[i];
        auto nextW = [&](std::wstring* value) -> bool {
            if (i + 1 >= argc) {
                return false;
            }
            *value = argv[++i];
            return true;
        };
        auto nextInt = [&](int* value) -> bool {
            std::wstring text;
            if (!nextW(&text)) {
                return false;
            }
            *value = std::stoi(text);
            return true;
        };
        if (arg == L"--page") {
            if (!nextW(&options->page)) {
                *error = "--page requires a value";
                return false;
            }
        }
        else if (arg == L"--out") {
            if (!nextW(&options->out)) {
                *error = "--out requires a value";
                return false;
            }
        }
        else if (arg == L"--evidence-dir") {
            if (!nextW(&options->evidenceDir)) {
                *error = "--evidence-dir requires a value";
                return false;
            }
        }
        else if (arg == L"--udf") {
            if (!nextW(&options->userDataDir)) {
                *error = "--udf requires a value";
                return false;
            }
        }
        else if (arg == L"--width") {
            if (!nextInt(&options->width)) {
                *error = "--width requires a number";
                return false;
            }
        }
        else if (arg == L"--height") {
            if (!nextInt(&options->height)) {
                *error = "--height requires a number";
                return false;
            }
        }
        else if (arg == L"--x") {
            if (!nextInt(&options->x)) {
                *error = "--x requires a number";
                return false;
            }
        }
        else if (arg == L"--y") {
            if (!nextInt(&options->y)) {
                *error = "--y requires a number";
                return false;
            }
        }
        else if (arg == L"--interactive") {
            options->interactive = true;
        }
        else if (arg == L"--wfp") {
            options->wfp = true;
            if (!nextInt(&options->wfpX) || !nextInt(&options->wfpY)) {
                *error = "--wfp requires two numbers";
                return false;
            }
        }
        else if (arg == L"--help" || arg == L"-h") {
            std::cout << "usage: visual-hosting-wheel [--page PATH] [--out REPORT.json] [--evidence-dir DIR]\n"
                         "                            [--udf DIR] [--x N] [--y N] [--width N] [--height N]\n"
                         "                            [--interactive]\n";
            return false;
        }
        else {
            *error = "unknown argument: " + WideToUtf8(arg);
            return false;
        }
    }

    const std::wstring exeDir = ExeDirW();
    if (options->page.empty()) {
        options->page = JoinPath(exeDir, L"wheel-page.html");
    }
    if (options->out.empty()) {
        options->out = JoinPath(exeDir, L"visual-hosting-report.json");
    }
    if (options->evidenceDir.empty()) {
        options->evidenceDir = exeDir;
    }
    if (options->interactive) {
        options->wfp = false;
    }
    if (GetFileAttributesW(options->page.c_str()) == INVALID_FILE_ATTRIBUTES) {
        *error = "page not found: " + WideToUtf8(options->page);
        return false;
    }
    return true;
}

void MaybeHideOwnConsoleImpl()
{
    HWND console = GetConsoleWindow();
    if (!console) {
        return;
    }
    const DWORD type = GetFileType(GetStdHandle(STD_OUTPUT_HANDLE));
    if (type != FILE_TYPE_PIPE && type != FILE_TYPE_DISK) {
        return;  // interactive console is useful; leave it alone
    }
    DWORD processes[4] {};
    const DWORD count = GetConsoleProcessList(processes, 4);
    if (count == 1) {
        ShowWindow(console, SW_HIDE);
    }
}

int RunWfpModeImpl(int x, int y)
{
    POINT point {x, y};
    HWND hit = WindowFromPoint(point);
    HWND root = hit ? GetAncestor(hit, GA_ROOT) : nullptr;
    DWORD pid = 0;
    bool visible = false;
    if (root) {
        GetWindowThreadProcessId(root, &pid);
        visible = IsWindowVisible(root) != FALSE;
    }
    std::cout << "WFP\t" << reinterpret_cast<uintptr_t>(hit) << "\t" << reinterpret_cast<uintptr_t>(root) << "\t"
              << GetWindowClassNameUtf8(root) << "\t" << GetWindowTitleUtf8(root) << "\t" << pid << "\t"
              << (visible ? 1 : 0) << "\n";
    std::cout.flush();
    return 0;
}

}  // namespace

static bool ParseArgs(int argc, wchar_t** argv, Options* options, std::string* error)
{
    return ParseArgsImpl(argc, argv, options, error);
}

static void MaybeHideOwnConsole()
{
    MaybeHideOwnConsoleImpl();
}

static int RunWfpMode(int x, int y)
{
    return RunWfpModeImpl(x, y);
}

static void ApplyState(VisualHost& host, const std::string& state, const Options& options)
{
    host.SetIsVisible(true);
    if (state == "visible-topmost") {
        host.DestroyCover();
        host.ShowHost(true);
        host.MoveHost(options.x, options.y);
        host.SetTopmost(false);
        host.PumpFor(50);
        host.SetTopmost(true);
    }
    else if (state == "occluded-cover") {
        host.ShowHost(true);
        host.MoveHost(options.x, options.y);
        host.SetTopmost(false);
        host.PumpFor(80);
        host.CreateCover(host.HostWindowRect());
    }
    else if (state == "bottom-cover") {
        host.ShowHost(true);
        host.MoveHost(options.x, options.y);
        host.SetTopmost(false);
        host.SetBottom();
        host.PumpFor(80);
        host.CreateCover(host.HostWindowRect());
    }
    else if (state == "minimized") {
        host.DestroyCover();
        host.ShowHost(true);
        host.MoveHost(options.x, options.y);
        host.SetTopmost(false);
        host.MinimizeHost();
    }
    else if (state == "hidden") {
        host.DestroyCover();
        host.ShowHost(true);
        host.MoveHost(options.x, options.y);
        host.SetTopmost(false);
        host.ShowHost(false);
    }
    else if (state == "offscreen") {
        host.DestroyCover();
        host.ShowHost(true);
        host.SetTopmost(false);
        host.MoveHost(-2400, 100);
    }
    else if (state == "isvisible-false") {
        host.DestroyCover();
        host.ShowHost(true);
        host.MoveHost(options.x, options.y);
        host.SetTopmost(true);
        host.SetIsVisible(false);
    }
    host.PumpFor(300);
}

static CaseResult RunCase(VisualHost& host, const Options& options, const std::string& phase, double scale,
    const CaseSpec& spec)
{
    CaseResult result;
    result.phase = phase;
    result.spec = spec;
    result.rasterScale = scale;
    result.hostHwnd = host.hwnd();
    result.coverHwnd = host.coverHwnd();

    ApplyState(host, spec.state, options);
    result.hostHwnd = host.hwnd();
    result.coverHwnd = host.coverHwnd();
    result.hostRect = host.HostWindowRect();
    result.clientScreen = host.ClientScreenRect();
    result.dpi = host.dpi();
    result.hostVisible = IsWindowVisible(host.hwnd()) != FALSE;
    result.hostIconic = IsIconic(host.hwnd()) != FALSE;

    const POINT point {spec.x, spec.y};
    const POINT screenPoint {result.clientScreen.left + spec.x, result.clientScreen.top + spec.y};
    result.wfp = QueryWindowFromPoint(CurrentExePath(), screenPoint.x, screenPoint.y);
    result.wfpIsHost = result.wfp.found && result.wfp.root == host.hwnd();
    result.wfpIsCover = result.wfp.found && host.coverHwnd() && result.wfp.root == host.coverHwnd();
    if (result.wfpIsHost) {
        result.coverage = "host";
    }
    else if (result.wfpIsCover) {
        result.coverage = "cover";
    }
    else if (result.wfp.found) {
        result.coverage = "other";
    }
    else {
        result.coverage = "none";
    }

    result.fgBefore = GetForegroundWindow();
    GetCursorPos(&result.cursorBefore);

    host.TakeProbe();  // discard anything pending
    host.ResetPage(800, 800);
    host.PumpFor(150);
    result.before = host.TakeProbe();
    result.nativeWheelsBefore = host.nativeWheelMessages();

    if (spec.kind == "move") {
        result.sendHrs.push_back(host.SendMouse(COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE, 0, point));
    }
    else {
        if (spec.moveFirst) {
            result.sendHrs.push_back(host.SendMouse(COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE, 0, point));
            host.PumpFor(120);
        }
        for (int i = 0; i < spec.msgs; ++i) {
            const auto kind =
                spec.kind == "hwheel" ? COREWEBVIEW2_MOUSE_EVENT_KIND_HORIZONTAL_WHEEL
                                       : COREWEBVIEW2_MOUSE_EVENT_KIND_WHEEL;
            result.sendHrs.push_back(host.SendMouse(kind, spec.delta, point));
            if (i + 1 < spec.msgs && spec.gapMs > 0) {
                host.PumpFor(spec.gapMs);
            }
        }
    }

    host.PumpFor(1000);
    result.after = host.TakeProbe();
    result.fgAfter = GetForegroundWindow();
    GetCursorPos(&result.cursorAfter);
    result.nativeWheelsAfter = host.nativeWheelMessages();

    for (const ProbeEvent& event : result.after.events) {
        if (event.type == "wheel") {
            ++result.wheelEvents;
            result.sumDeltaX += event.deltaX;
            result.sumDeltaY += event.deltaY;
            if (!event.trusted) {
                result.allTrusted = false;
            }
        }
        else if (event.type == "mousemove") {
            ++result.moveEvents;
        }
    }

    const bool inLeftPane = spec.x * 2 < host.width();
    const double viewportHalf = result.after.innerWidth > 0 ? result.after.innerWidth * 0.5 : host.width() * 0.5;
    bool correctPane = true;
    bool sawPane = false;
    for (const ProbeEvent& event : result.after.events) {
        if (event.type != "wheel") {
            continue;
        }
        sawPane = true;
        if (inLeftPane && event.x >= viewportHalf) {
            correctPane = false;
        }
        if (!inLeftPane && event.x < viewportHalf) {
            correctPane = false;
        }
    }

    const double expectedMagnitude = std::fabs(static_cast<double>(spec.delta)) * std::max(spec.msgs, 1);
    const double axisSum = spec.kind == "hwheel" ? result.sumDeltaX : result.sumDeltaY;
    const double actualMagnitude = std::fabs(axisSum);

    if (!result.after.valid) {
        result.verdict = "script-error";
    }
    else if (spec.kind == "move") {
        result.verdict = result.moveEvents > 0 ? "move-delivered" : "move-no-event";
    }
    else if (result.wheelEvents == 0) {
        result.verdict = "no-event";
    }
    else if (actualMagnitude > expectedMagnitude * 1.5) {
        result.verdict = "duplicated";
    }
    else if (std::fabs(actualMagnitude - expectedMagnitude) <= expectedMagnitude * 0.25) {
        result.verdict = "delivered";
    }
    else {
        result.verdict = "delta-mismatch";
    }
    if (result.verdict == "delivered" && sawPane && !correctPane) {
        result.verdict = "wrong-pane";
    }
    if (result.verdict == "delivered" && result.wheelEvents != spec.msgs) {
        result.note = "coalesced/split: " + std::to_string(result.wheelEvents) + " event(s) for " +
            std::to_string(spec.msgs) + " message(s)";
    }
    return result;
}

static std::string ProbeJson(const ProbeSnapshot& snapshot, bool withEvents)
{
    std::ostringstream out;
    out << "{\"valid\":" << JBool(snapshot.valid) << ",\"ready\":" << JBool(snapshot.ready)
        << ",\"inner_width\":" << snapshot.innerWidth << ",\"inner_height\":" << snapshot.innerHeight
        << ",\"device_pixel_ratio\":" << DoubleStr(snapshot.dpr)
        << ",\"left_scroll_top\":" << snapshot.leftTop << ",\"right_scroll_top\":" << snapshot.rightTop
        << ",\"left_scroll_left\":" << snapshot.leftLeft << ",\"right_scroll_left\":" << snapshot.rightLeft
        << ",\"has_focus\":" << JBool(snapshot.hasFocus) << ",\"visibility\":" << JStr(snapshot.visibility)
        << ",\"pointer_lock\":" << JStr(snapshot.pointerLock) << ",\"event_count\":" << snapshot.events.size();
    if (withEvents) {
        out << ",\"events\":[";
        for (size_t i = 0; i < snapshot.events.size() && i < 80; ++i) {
            if (i) {
                out << ",";
            }
            const ProbeEvent& event = snapshot.events[i];
            out << "{\"type\":" << JStr(event.type) << ",\"x\":" << event.x << ",\"y\":" << event.y
                << ",\"deltaX\":" << DoubleStr(event.deltaX) << ",\"deltaY\":" << DoubleStr(event.deltaY)
                << ",\"target\":" << JStr(event.target) << ",\"trusted\":" << JBool(event.trusted) << "}";
        }
        out << "]";
    }
    out << "}";
    return out.str();
}

static std::string CaseJson(const CaseResult& result)
{
    std::ostringstream out;
    out << "{";
    out << "\"phase\":" << JStr(result.phase);
    out << ",\"name\":" << JStr(result.spec.name);
    out << ",\"state\":" << JStr(result.spec.state);
    out << ",\"kind\":" << JStr(result.spec.kind);
    out << ",\"point\":[" << result.spec.x << "," << result.spec.y << "]";
    out << ",\"delta\":" << result.spec.delta;
    out << ",\"messages_sent\":" << result.spec.msgs;
    out << ",\"gap_ms\":" << result.spec.gapMs;
    out << ",\"move_first\":" << JBool(result.spec.moveFirst);
    out << ",\"raster_scale\":" << DoubleStr(result.rasterScale);
    out << ",\"host_hwnd\":" << HwndNum(result.hostHwnd);
    out << ",\"cover_hwnd\":" << (result.coverHwnd ? HwndNum(result.coverHwnd) : "null");
    out << ",\"host_rect\":" << RectArray(result.hostRect);
    out << ",\"client_rect_screen\":" << RectArray(result.clientScreen);
    out << ",\"host_dpi\":" << result.dpi;
    out << ",\"host_visible\":" << JBool(result.hostVisible);
    out << ",\"host_iconic\":" << JBool(result.hostIconic);
    out << ",\"coverage\":" << JStr(result.coverage);
    out << ",\"wfp\":{\"queried\":" << JBool(result.wfp.queried) << ",\"found\":" << JBool(result.wfp.found)
        << ",\"hwnd\":" << (result.wfp.found ? HwndNum(result.wfp.hwnd) : "null")
        << ",\"root\":" << (result.wfp.found ? HwndNum(result.wfp.root) : "null")
        << ",\"class\":" << JStr(result.wfp.cls) << ",\"pid\":" << result.wfp.pid
        << ",\"visible\":" << JBool(result.wfp.visible) << ",\"is_host\":" << JBool(result.wfpIsHost)
        << ",\"is_cover\":" << JBool(result.wfpIsCover) << "}";
    out << ",\"foreground_before\":" << WindowInfoJson(result.fgBefore);
    out << ",\"foreground_after\":" << WindowInfoJson(result.fgAfter);
    out << ",\"foreground_changed\":" << JBool(result.fgBefore != result.fgAfter);
    out << ",\"cursor_before\":" << PointArray(result.cursorBefore);
    out << ",\"cursor_after\":" << PointArray(result.cursorAfter);
    out << ",\"cursor_moved\":"
        << JBool(result.cursorBefore.x != result.cursorAfter.x || result.cursorBefore.y != result.cursorAfter.y);
    out << ",\"native_wheel_messages\":[" << result.nativeWheelsBefore << "," << result.nativeWheelsAfter << "]";
    out << ",\"page_before\":" << ProbeJson(result.before, false);
    out << ",\"page_after\":" << ProbeJson(result.after, false);
    out << ",\"events\":";
    out << "[";
    for (size_t i = 0; i < result.after.events.size() && i < 80; ++i) {
        if (i) {
            out << ",";
        }
        const ProbeEvent& event = result.after.events[i];
        out << "{\"type\":" << JStr(event.type) << ",\"x\":" << event.x << ",\"y\":" << event.y
            << ",\"deltaX\":" << DoubleStr(event.deltaX) << ",\"deltaY\":" << DoubleStr(event.deltaY)
            << ",\"target\":" << JStr(event.target) << ",\"trusted\":" << JBool(event.trusted) << "}";
    }
    out << "]";
    out << ",\"wheel_events\":" << result.wheelEvents;
    out << ",\"mousemove_events\":" << result.moveEvents;
    out << ",\"all_wheel_trusted\":" << JBool(result.allTrusted);
    out << ",\"sum_delta_x\":" << DoubleStr(result.sumDeltaX);
    out << ",\"sum_delta_y\":" << DoubleStr(result.sumDeltaY);
    out << ",\"scroll_before\":{\"left_top\":" << result.before.leftTop << ",\"right_top\":" << result.before.rightTop
        << ",\"left_left\":" << result.before.leftLeft << ",\"right_left\":" << result.before.rightLeft << "}";
    out << ",\"scroll_after\":{\"left_top\":" << result.after.leftTop << ",\"right_top\":" << result.after.rightTop
        << ",\"left_left\":" << result.after.leftLeft << ",\"right_left\":" << result.after.rightLeft << "}";
    out << ",\"scroll_delta\":{\"left_top\":" << (result.after.leftTop - result.before.leftTop)
        << ",\"right_top\":" << (result.after.rightTop - result.before.rightTop)
        << ",\"left_left\":" << (result.after.leftLeft - result.before.leftLeft)
        << ",\"right_left\":" << (result.after.rightLeft - result.before.rightLeft) << "}";
    out << ",\"send_hr\":[";
    for (size_t i = 0; i < result.sendHrs.size(); ++i) {
        if (i) {
            out << ",";
        }
        out << JStr(HexHr(result.sendHrs[i]));
    }
    out << "]";
    out << ",\"verdict\":" << JStr(result.verdict);
    out << ",\"note\":" << JStr(result.note);
    out << "}";
    return out.str();
}

static std::string CompactCaseJson(const CaseResult& result)
{
    std::ostringstream out;
    out << "{";
    out << "\"case\":" << JStr(result.spec.name);
    out << ",\"phase\":" << JStr(result.phase);
    out << ",\"state\":" << JStr(result.spec.state);
    out << ",\"point\":[" << result.spec.x << "," << result.spec.y << "]";
    out << ",\"delta\":" << result.spec.delta;
    out << ",\"msgs\":" << result.spec.msgs;
    out << ",\"coverage\":" << JStr(result.coverage);
    out << ",\"wheel_events\":" << result.wheelEvents;
    out << ",\"move_events\":" << result.moveEvents;
    out << ",\"sum_dy\":" << DoubleStr(result.sumDeltaY);
    out << ",\"sum_dx\":" << DoubleStr(result.sumDeltaX);
    out << ",\"scroll\":{\"lt\":" << (result.after.leftTop - result.before.leftTop)
        << ",\"rt\":" << (result.after.rightTop - result.before.rightTop)
        << ",\"ll\":" << (result.after.leftLeft - result.before.leftLeft)
        << ",\"rl\":" << (result.after.rightLeft - result.before.rightLeft) << "}";
    out << ",\"target\":";
    std::string target;
    for (const ProbeEvent& event : result.after.events) {
        if (event.type == "wheel") {
            target = event.target;
            break;
        }
    }
    out << JStr(target);
    out << ",\"fg_changed\":" << JBool(result.fgBefore != result.fgAfter);
    out << ",\"cursor_moved\":"
        << JBool(result.cursorBefore.x != result.cursorAfter.x || result.cursorBefore.y != result.cursorAfter.y);
    out << ",\"hr\":" << (result.sendHrs.empty() ? JStr("none") : JStr(HexHr(result.sendHrs.front())));
    out << ",\"verdict\":" << JStr(result.verdict);
    if (!result.note.empty()) {
        out << ",\"note\":" << JStr(result.note);
    }
    out << "}";
    return out.str();
}

static PointerLockResult RunPointerLockProbe(VisualHost& host, const Options& options)
{
    PointerLockResult result;
    ApplyState(host, "visible-topmost", options);
    host.TakeProbe();
    const POINT point {300, 300};
    const HRESULT down = host.SendMouse(COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_DOWN, 0, point);
    const HRESULT up = host.SendMouse(COREWEBVIEW2_MOUSE_EVENT_KIND_LEFT_BUTTON_UP, 0, point);
    result.clickHrs = HexHr(down) + "," + HexHr(up);
    host.PumpFor(300);

    bool ok = false;
    const std::wstring called = host.ExecScriptSync(
        L"(function(){window.__plError='';var el=document.getElementById('left');if(!el)return 'noel';"
        L"try{var p=el.requestPointerLock();if(p&&p.catch){p.catch(function(e){window.__plError=String(e&&(e.message||e.name||e));});}"
        L"return 'called';}catch(e){window.__plError=String(e);return 'threw';}})()",
        &ok);
    result.stateCalled = ok ? WideToUtf8(called) : "script-failed";
    host.PumpFor(900);

    const std::wstring state = host.ExecScriptSync(L"window.__plState ? window.__plState() : 'no-state'", &ok);
    const std::string stateText = ok ? WideToUtf8(state) : "script-failed";
    const size_t sep = stateText.find('|');
    result.stateAfter = sep == std::string::npos ? stateText : stateText.substr(0, sep);
    if (sep != std::string::npos) {
        const size_t sep2 = stateText.find('|', sep + 1);
        result.error = stateText.substr(sep + 1, sep2 == std::string::npos ? std::string::npos : sep2 - sep - 1);
    }
    host.ExecScriptSync(L"document.exitPointerLock && document.exitPointerLock(); 'ok'");
    host.PumpFor(200);
    result.ran = true;
    return result;
}

static int RunInteractive(const Options& options)
{
    const std::wstring runDir = MakeRunDir();
    const std::wstring userDataDir =
        options.userDataDir.empty() ? JoinPath(runDir, L"WebView2") : options.userDataDir;
    EnsureDir(options.evidenceDir);

    VisualHost host;
    std::string error;
    if (!host.Create(options.page, userDataDir, options.width, options.height, options.x, options.y, &error)) {
        std::cout << "{\"error\":" << JStr(error) << "}" << std::endl;
        return 2;
    }
    const bool ready = host.WaitForPageReady(20000);
    std::cout << "{\"interactive\":true,\"hwnd\":" << HwndNum(host.hwnd()) << ",\"dpi\":" << host.dpi()
              << ",\"runtime_version\":" << JStr(WideToUtf8(host.runtimeVersion())) << ",\"page_ready\":"
              << JBool(ready) << "}" << std::endl;
    std::cout << "commands: state | probe | reset | wheel X Y D | hwheel X Y D | move X Y | cover on|off | "
                 "topmost on|off | bottom | show on|off | min | vis on|off | scale S | plock | shot NAME | wait MS | "
                 "quit"
              << std::endl;

    std::string line;
    while (std::getline(std::cin, line)) {
        std::istringstream in(line);
        std::string command;
        in >> command;
        if (command.empty()) {
            continue;
        }
        if (command == "quit" || command == "exit") {
            break;
        }
        if (command == "probe") {
            std::cout << ProbeJson(host.TakeProbe(), true) << std::endl;
        }
        else if (command == "reset") {
            std::cout << "{\"reset\":" << JBool(host.ResetPage(800, 800)) << "}" << std::endl;
        }
        else if (command == "wheel" || command == "hwheel") {
            int x = 0;
            int y = 0;
            int delta = 0;
            in >> x >> y >> delta;
            const auto kind = command == "hwheel" ? COREWEBVIEW2_MOUSE_EVENT_KIND_HORIZONTAL_WHEEL
                                                  : COREWEBVIEW2_MOUSE_EVENT_KIND_WHEEL;
            const HRESULT hr = host.SendMouse(kind, delta, POINT {x, y});
            host.PumpFor(800);
            std::cout << "{\"send_hr\":" << JStr(HexHr(hr)) << "}" << std::endl;
        }
        else if (command == "move") {
            int x = 0;
            int y = 0;
            in >> x >> y;
            const HRESULT hr = host.SendMouse(COREWEBVIEW2_MOUSE_EVENT_KIND_MOVE, 0, POINT {x, y});
            std::cout << "{\"send_hr\":" << JStr(HexHr(hr)) << "}" << std::endl;
        }
        else if (command == "cover") {
            std::string value;
            in >> value;
            if (value == "on") {
                std::cout << "{\"cover\":" << JBool(host.CreateCover(host.HostWindowRect())) << "}" << std::endl;
            }
            else {
                host.DestroyCover();
                std::cout << "{\"cover\":false}" << std::endl;
            }
        }
        else if (command == "topmost") {
            std::string value;
            in >> value;
            host.SetTopmost(value == "on");
        }
        else if (command == "bottom") {
            host.SetBottom();
        }
        else if (command == "show") {
            std::string value;
            in >> value;
            host.ShowHost(value == "on");
        }
        else if (command == "min") {
            host.MinimizeHost();
        }
        else if (command == "vis") {
            std::string value;
            in >> value;
            std::cout << "{\"is_visible\":" << JBool(host.SetIsVisible(value == "on")) << "}" << std::endl;
        }
        else if (command == "scale") {
            double scale = 1.0;
            in >> scale;
            std::cout << "{\"scale_applied\":" << JBool(host.SetRasterizationScale(scale)) << "}" << std::endl;
            host.PumpFor(300);
        }
        else if (command == "plock") {
            const PointerLockResult result = RunPointerLockProbe(host, options);
            std::cout << "{\"state\":" << JStr(result.stateAfter) << ",\"error\":" << JStr(result.error)
                      << ",\"call\":" << JStr(result.stateCalled) << "}" << std::endl;
        }
        else if (command == "shot") {
            std::string name;
            in >> name;
            if (name.empty()) {
                name = "interactive";
            }
            const std::wstring base = JoinPath(options.evidenceDir, Utf8ToWide(name));
            BmpStats screen;
            BmpStats printed;
            host.CaptureScreen(base + L"-screen.bmp", &screen);
            host.CaptureWindow(base + L"-printwindow.bmp", &printed);
            std::cout << "{\"screen\":{\"ok\":" << JBool(screen.ok) << ",\"teal\":" << DoubleStr(screen.tealFraction)
                      << "},\"printwindow\":{\"ok\":" << JBool(printed.ok)
                      << ",\"teal\":" << DoubleStr(printed.tealFraction) << ",\"returned\":"
                      << printed.printWindowResult << "}}" << std::endl;
        }
        else if (command == "wait") {
            int ms = 500;
            in >> ms;
            host.PumpFor(ms);
        }
        else if (command == "state") {
            const RECT client = host.ClientScreenRect();
            const WfpResult wfp = QueryWindowFromPoint(CurrentExePath(), client.left + host.width() / 2,
                client.top + host.height() / 2);
            POINT cursor {};
            GetCursorPos(&cursor);
            std::cout << "{\"hwnd\":" << HwndNum(host.hwnd()) << ",\"host_rect\":" << RectArray(host.HostWindowRect())
                      << ",\"client_screen\":" << RectArray(client) << ",\"dpi\":" << host.dpi()
                      << ",\"cover\":" << (host.coverHwnd() ? HwndNum(host.coverHwnd()) : "null")
                      << ",\"foreground\":" << WindowInfoJson(GetForegroundWindow())
                      << ",\"cursor\":" << PointArray(cursor) << ",\"wfp_class\":" << JStr(wfp.cls)
                      << ",\"wfp_is_host\":" << JBool(wfp.found && wfp.root == host.hwnd()) << "}" << std::endl;
        }
        else {
            std::cout << "{\"error\":\"unknown command\"}" << std::endl;
        }
    }

    host.Close();
    return 0;
}

static int RunHarness(const Options& options)
{
    if (options.interactive) {
        return RunInteractive(options);
    }
    const std::wstring runDir = MakeRunDir();
    const std::wstring userDataDir =
        options.userDataDir.empty() ? JoinPath(runDir, L"WebView2") : options.userDataDir;
    EnsureDir(options.evidenceDir);
    const std::wstring exePath = CurrentExePath();

    VisualHost host;
    std::string error;
    if (!host.Create(options.page, userDataDir, options.width, options.height, options.x, options.y, &error)) {
        std::ostringstream report;
        report << "{\"experiment\":\"webview2-visual-hosting-wheel\",\"error\":" << JStr(error)
               << ",\"environment\":" << EnvironmentJson(nullptr, false) << "}";
        WriteTextFile(options.out, report.str());
        std::cout << "{\"error\":" << JStr(error) << "}\n";
        return 2;
    }
    const bool hostForegroundAfterCreate = GetForegroundWindow() == host.hwnd();

    const bool pageReady = host.WaitForPageReady(20000);
    ProbeSnapshot initialProbe = host.TakeProbe();
    const int initialWidth = initialProbe.innerWidth;
    double defaultScale = initialWidth > 0 ? static_cast<double>(options.width) / initialWidth : 1.0;
    if (!std::isfinite(defaultScale) || defaultScale <= 0.0) {
        defaultScale = 1.0;
    }

    std::vector<CaseResult> results;
    std::vector<PhaseInfo> phases;
    std::vector<std::string> reportNotes;

    const int initialViewportWidth = initialProbe.innerWidth;
    const int initialViewportHeight = initialProbe.innerHeight;

    // Make sure the host is genuinely on top for the render check. A window that
    // is already topmost is not re-inserted by SetTopmost(TRUE), so toggle it.
    WfpResult wfpCenter;
    for (int attempt = 0; attempt < 3; ++attempt) {
        host.SetTopmost(false);
        host.PumpFor(80);
        host.SetTopmost(true);
        host.PumpFor(500);
        wfpCenter = QueryWindowFromPoint(exePath, (host.ClientScreenRect().left + host.ClientScreenRect().right) / 2,
            (host.ClientScreenRect().top + host.ClientScreenRect().bottom) / 2);
        if (wfpCenter.found && wfpCenter.root == host.hwnd()) {
            break;
        }
    }
    const bool wfpCenterIsHost = wfpCenter.found && wfpCenter.root == host.hwnd();
    BmpStats screenStats;
    host.CaptureScreen(JoinPath(options.evidenceDir, L"host-visible-screen.bmp"), &screenStats);
    BmpStats printStats;
    host.CaptureWindow(JoinPath(options.evidenceDir, L"host-visible-printwindow.bmp"), &printStats);
    // In this automation desktop the on-screen BitBlt did not contain the host
    // window, so rendering is verified through PrintWindow, which does capture
    // the composed WebView2 content. Both numbers are reported.
    const bool renderVerified = printStats.ok && printStats.printWindowResult == 1 && printStats.tealFraction > 0.03;
    if (wfpCenterIsHost && screenStats.tealFraction > 0.03) {
        reportNotes.push_back(
            "Rendering verified by both the desktop BitBlt (teal fraction " + DoubleStr(screenStats.tealFraction) +
            ", WindowFromPoint=host) and PrintWindow (teal fraction " + DoubleStr(printStats.tealFraction) + ").");
    }
    else {
        reportNotes.push_back(
            "Desktop BitBlt at render-check time showed another window (teal fraction " +
            DoubleStr(screenStats.tealFraction) + ", WindowFromPoint class " + wfpCenter.cls +
            "); PrintWindow captured the WebView2 content (teal fraction " + DoubleStr(printStats.tealFraction) +
            "), so rendering is verified via PrintWindow.");
    }
    if (std::fabs(defaultScale - 1.0) > 0.02) {
        reportNotes.push_back(
            "Host display is at DPI " + std::to_string(host.dpi()) + " (150% scaling; measured raster ratio " +
            DoubleStr(defaultScale) + "); system scaling was not changed. Phase A forces rasterization scale 1.0 "
            "(canonical 1:1 viewport), Phase B restores the DPI-scaled viewport to observe coordinate units.");
    }
    reportNotes.push_back(
        "Occlusion is produced by a topmost cover window owned by the same test process; WindowFromPoint is "
        "evaluated by a secondary process. The host is never activated or foregrounded.");

    const double productScale = 1.0;
    double alternateScale = defaultScale;
    if (std::fabs(alternateScale - 1.0) < 0.02) {
        alternateScale = 1.5;
    }

    auto runPhase = [&](const std::string& phaseName, double scale, const std::vector<CaseSpec>& cases) {
        PhaseInfo info;
        info.name = phaseName;
        info.requestedScale = scale;
        info.scaleApplied = host.SetRasterizationScale(scale);
        if (!info.scaleApplied) {
            info.note = "put_RasterizationScale failed or ICoreWebView2Controller3 unavailable";
        }
        host.PumpFor(500);
        ProbeSnapshot viewport = host.TakeProbe();
        info.viewportWidth = viewport.innerWidth;
        info.viewportHeight = viewport.innerHeight;
        info.dpr = viewport.dpr;
        phases.push_back(info);
        for (const CaseSpec& spec : cases) {
            CaseResult caseResult = RunCase(host, options, phaseName, scale, spec);
            results.push_back(caseResult);
            std::cout << CompactCaseJson(caseResult) << std::endl;
        }
    };

    std::vector<CaseSpec> fullMatrix = {
        {"visible-left-wheel", "visible-topmost", "wheel", 300, 300, 120, 1, 0, false},
        {"visible-right-wheel", "visible-topmost", "wheel", 900, 300, -120, 1, 0, false},
        {"visible-left-hwheel", "visible-topmost", "hwheel", 300, 300, 120, 1, 0, false},
        {"visible-left-wheel240", "visible-topmost", "wheel", 300, 300, 240, 1, 0, false},
        {"visible-move-only", "visible-topmost", "move", 300, 300, 0, 0, 0, false},
        {"occluded-left-wheel", "occluded-cover", "wheel", 300, 300, 120, 1, 0, false},
        {"occluded-right-wheel", "occluded-cover", "wheel", 900, 300, -120, 1, 0, false},
        {"occluded-left-hwheel", "occluded-cover", "hwheel", 300, 300, 120, 1, 0, false},
        {"occluded-two-wheel-gap", "occluded-cover", "wheel", 300, 300, 120, 2, 60, false},
        {"occluded-two-wheel-burst", "occluded-cover", "wheel", 300, 300, 120, 2, 0, false},
        {"occluded-left-wheel240", "occluded-cover", "wheel", 300, 300, 240, 1, 0, false},
        {"occluded-move-then-wheel", "occluded-cover", "wheel", 300, 300, 120, 1, 0, true},
        {"occluded-move-only", "occluded-cover", "move", 300, 300, 0, 0, 0, false},
        {"bottom-cover-left-wheel", "bottom-cover", "wheel", 300, 300, 120, 1, 0, false},
        {"isvisible-false-left-wheel", "isvisible-false", "wheel", 300, 300, 120, 1, 0, false},
        {"minimized-left-wheel", "minimized", "wheel", 300, 300, 120, 1, 0, false},
        {"hidden-left-wheel", "hidden", "wheel", 300, 300, 120, 1, 0, false},
        {"offscreen-left-wheel", "offscreen", "wheel", 300, 300, 120, 1, 0, false},
    };

    std::vector<CaseSpec> dpiMatrix = {
        {"dpi-visible-left-wheel", "visible-topmost", "wheel", 300, 300, 120, 1, 0, false},
        {"dpi-visible-right-wheel", "visible-topmost", "wheel", 900, 300, -120, 1, 0, false},
        {"dpi-occluded-left-wheel", "occluded-cover", "wheel", 300, 300, 120, 1, 0, false},
        {"dpi-occluded-right-wheel", "occluded-cover", "wheel", 900, 300, -120, 1, 0, false},
        {"dpi-occluded-two-wheel", "occluded-cover", "wheel", 300, 300, 120, 2, 60, false},
    };

    runPhase("A", productScale, fullMatrix);
    if (std::fabs(alternateScale - productScale) > 0.02) {
        runPhase("B", alternateScale, dpiMatrix);
    }

    // CloudShell screenshots go through PrintWindow; check it while the host is
    // covered by the topmost cover window.
    ApplyState(host, "occluded-cover", options);
    host.PumpFor(600);
    const RECT occludedClient = host.ClientScreenRect();
    const WfpResult wfpOccluded = QueryWindowFromPoint(exePath, (occludedClient.left + occludedClient.right) / 2,
        (occludedClient.top + occludedClient.bottom) / 2);
    BmpStats occludedScreen;
    host.CaptureScreen(JoinPath(options.evidenceDir, L"host-occluded-screen.bmp"), &occludedScreen);
    BmpStats occludedPrint;
    host.CaptureWindow(JoinPath(options.evidenceDir, L"host-occluded-printwindow.bmp"), &occludedPrint);
    const bool wfpOccludedIsHost = wfpOccluded.found && wfpOccluded.root == host.hwnd();
    const bool wfpOccludedIsCover =
        wfpOccluded.found && host.coverHwnd() && wfpOccluded.root == host.coverHwnd();
    reportNotes.push_back(
        "While covered, PrintWindow(PW_RENDERFULLCONTENT) still captured the page (teal fraction " +
        DoubleStr(occludedPrint.tealFraction) + ", result " + std::to_string(occludedPrint.printWindowResult) +
        "); the desktop BitBlt showed the cover window (teal fraction " + DoubleStr(occludedScreen.tealFraction) +
        ", unique colors " + std::to_string(occludedScreen.uniqueColors) + ").");

    // Tight cursor/foreground probe: sample immediately around the SendMouseInput
    // call so concurrent user activity can be distinguished from API side effects.
    std::vector<CursorProbeEntry> cursorProbe;
    for (int i = 0; i < 6; ++i) {
        CursorProbeEntry entry;
        entry.state = (i % 2 == 0) ? "visible-topmost" : "occluded-cover";
        ApplyState(host, entry.state, options);
        host.PumpFor(250);
        entry.hostHwnd = host.hwnd();
        entry.coverHwnd = host.coverHwnd();
        GetCursorPos(&entry.before);
        entry.fgBefore = GetForegroundWindow();
        entry.hr = host.SendMouse(COREWEBVIEW2_MOUSE_EVENT_KIND_WHEEL, 120, POINT {300, 300});
        GetCursorPos(&entry.immediate);
        entry.fgImmediate = GetForegroundWindow();
        host.PumpFor(150);
        GetCursorPos(&entry.after);
        entry.fgAfter = GetForegroundWindow();
        host.TakeProbe();
        cursorProbe.push_back(entry);
    }
    int immediateCursorChanges = 0;
    int immediateForegroundChanges = 0;
    bool hostForegroundInProbe = false;
    bool coverForegroundInProbe = false;
    for (const CursorProbeEntry& entry : cursorProbe) {
        if (entry.before.x != entry.immediate.x || entry.before.y != entry.immediate.y) {
            ++immediateCursorChanges;
        }
        if (entry.fgBefore != entry.fgImmediate) {
            ++immediateForegroundChanges;
        }
        hostForegroundInProbe = hostForegroundInProbe || entry.fgBefore == entry.hostHwnd ||
            entry.fgImmediate == entry.hostHwnd || entry.fgAfter == entry.hostHwnd;
        coverForegroundInProbe = coverForegroundInProbe || (entry.coverHwnd != nullptr &&
            (entry.fgBefore == entry.coverHwnd || entry.fgImmediate == entry.coverHwnd ||
                entry.fgAfter == entry.coverHwnd));
    }

    PointerLockResult pointerLock = RunPointerLockProbe(host, options);

    // Restore a neutral, visible state before closing.
    host.SetRasterizationScale(productScale);
    ApplyState(host, "visible-topmost", options);
    host.SetIsVisible(true);

    bool hostForegroundSeen = GetForegroundWindow() == host.hwnd();
    for (const CaseResult& result : results) {
        if (result.fgAfter == host.hwnd() || result.fgBefore == host.hwnd()) {
            hostForegroundSeen = true;
        }
    }

    std::ostringstream report;
    report << "{";
    report << "\"experiment\":\"webview2-visual-hosting-wheel\"";
    report << ",\"generated_at\":" << JStr(TimestampUtc());
    report << ",\"environment\":" << EnvironmentJson(&host, hostForegroundAfterCreate);
    report << ",\"webview2\":{\"runtime_version\":" << JStr(WideToUtf8(host.runtimeVersion()))
           << ",\"sdk_version\":\"1.0.2210.55\",\"user_data_dir\":" << JStr(WideToUtf8(userDataDir))
           << ",\"browser_process_id\":" << host.browserProcessId() << "}";
    report << ",\"host\":{\"hwnd\":" << HwndNum(host.hwnd()) << ",\"window_rect\":" << RectArray(host.HostWindowRect())
           << ",\"client_size\":[" << host.width() << "," << host.height() << "],\"dpi\":" << host.dpi()
           << ",\"initial_foreground_after_create\":" << JBool(hostForegroundAfterCreate)
           << ",\"host_foreground_seen_during_matrix\":" << JBool(hostForegroundSeen) << "}";
    report << ",\"page\":{\"path\":" << JStr(WideToUtf8(options.page)) << ",\"ready\":" << JBool(pageReady)
           << ",\"initial_viewport\":{\"width\":" << initialViewportWidth << ",\"height\":" << initialViewportHeight
           << ",\"dpr\":" << DoubleStr(initialProbe.dpr) << "},\"default_effective_scale\":"
           << DoubleStr(defaultScale) << "}";
    report << ",\"render\":{\"screen_capture\":{\"ok\":" << JBool(screenStats.ok)
           << ",\"path\":" << JStr(WideToUtf8(screenStats.path))
           << ",\"non_black_fraction\":" << DoubleStr(screenStats.nonBlackFraction)
           << ",\"teal_fraction\":" << DoubleStr(screenStats.tealFraction)
           << ",\"unique_colors\":" << screenStats.uniqueColors << "}"
           << ",\"printwindow\":{\"ok\":" << JBool(printStats.ok)
           << ",\"path\":" << JStr(WideToUtf8(printStats.path))
           << ",\"printwindow_result\":" << printStats.printWindowResult
           << ",\"non_black_fraction\":" << DoubleStr(printStats.nonBlackFraction)
           << ",\"teal_fraction\":" << DoubleStr(printStats.tealFraction)
           << ",\"unique_colors\":" << printStats.uniqueColors << "}"
           << ",\"wfp_at_client_center\":{\"class\":" << JStr(wfpCenter.cls) << ",\"is_host\":"
           << JBool(wfpCenterIsHost) << "}"
           << ",\"render_verified\":" << JBool(renderVerified) << "}";
    report << ",\"render_occluded\":{\"screen_capture\":{\"ok\":" << JBool(occludedScreen.ok)
           << ",\"path\":" << JStr(WideToUtf8(occludedScreen.path))
           << ",\"teal_fraction\":" << DoubleStr(occludedScreen.tealFraction)
           << ",\"unique_colors\":" << occludedScreen.uniqueColors << "}"
           << ",\"printwindow\":{\"ok\":" << JBool(occludedPrint.ok)
           << ",\"path\":" << JStr(WideToUtf8(occludedPrint.path))
           << ",\"printwindow_result\":" << occludedPrint.printWindowResult
           << ",\"teal_fraction\":" << DoubleStr(occludedPrint.tealFraction)
           << ",\"unique_colors\":" << occludedPrint.uniqueColors << "}"
           << ",\"wfp_at_client_center\":{\"class\":" << JStr(wfpOccluded.cls) << ",\"is_host\":"
           << JBool(wfpOccludedIsHost) << ",\"is_cover\":" << JBool(wfpOccludedIsCover) << "}}";
    report << ",\"cursor_foreground_probe\":{\"samples\":[";
    for (size_t i = 0; i < cursorProbe.size(); ++i) {
        if (i) {
            report << ",";
        }
        const CursorProbeEntry& entry = cursorProbe[i];
        report << "{\"state\":" << JStr(entry.state) << ",\"send_hr\":" << JStr(HexHr(entry.hr))
               << ",\"cursor_before\":" << PointArray(entry.before)
               << ",\"cursor_immediate\":" << PointArray(entry.immediate)
               << ",\"cursor_after\":" << PointArray(entry.after)
               << ",\"cursor_changed_immediate\":"
               << JBool(entry.before.x != entry.immediate.x || entry.before.y != entry.immediate.y)
               << ",\"foreground_before\":" << WindowInfoJson(entry.fgBefore)
               << ",\"foreground_immediate\":" << WindowInfoJson(entry.fgImmediate)
               << ",\"foreground_after\":" << WindowInfoJson(entry.fgAfter) << "}";
    }
    report << "],\"summary\":{\"samples\":" << cursorProbe.size()
           << ",\"immediate_cursor_changes\":" << immediateCursorChanges
           << ",\"immediate_foreground_changes\":" << immediateForegroundChanges
           << ",\"host_foreground_ever\":" << JBool(hostForegroundInProbe)
           << ",\"cover_foreground_ever\":" << JBool(coverForegroundInProbe) << "}}";
    report << ",\"phases\":[";
    for (size_t i = 0; i < phases.size(); ++i) {
        if (i) {
            report << ",";
        }
        const PhaseInfo& phase = phases[i];
        report << "{\"name\":" << JStr(phase.name) << ",\"requested_scale\":" << DoubleStr(phase.requestedScale)
               << ",\"scale_applied\":" << JBool(phase.scaleApplied) << ",\"viewport\":{\"width\":"
               << phase.viewportWidth << ",\"height\":" << phase.viewportHeight << ",\"dpr\":"
               << DoubleStr(phase.dpr) << "},\"note\":" << JStr(phase.note) << "}";
    }
    report << "]";
    report << ",\"cases\":[";
    for (size_t i = 0; i < results.size(); ++i) {
        if (i) {
            report << ",";
        }
        report << CaseJson(results[i]);
    }
    report << "]";
    report << ",\"pointer_lock\":{\"ran\":" << JBool(pointerLock.ran) << ",\"click_hr\":"
           << JStr(pointerLock.clickHrs) << ",\"call\":" << JStr(pointerLock.stateCalled)
           << ",\"state_after\":" << JStr(pointerLock.stateAfter) << ",\"error\":" << JStr(pointerLock.error)
           << "}";
    report << ",\"notes\":[";
    for (size_t i = 0; i < reportNotes.size(); ++i) {
        if (i) {
            report << ",";
        }
        report << JStr(reportNotes[i]);
    }
    report << "]";
    report << "}";

    const bool written = WriteTextFile(options.out, report.str());
    std::cout << "{\"report\":\"" << JsonEscape(WideToUtf8(options.out)) << "\",\"written\":" << JBool(written)
              << ",\"runtime_version\":" << JStr(WideToUtf8(host.runtimeVersion()))
              << ",\"page_ready\":" << JBool(pageReady) << ",\"render_verified\":" << JBool(renderVerified)
              << "}" << std::endl;

    host.Close();
    return written ? 0 : 4;
}
