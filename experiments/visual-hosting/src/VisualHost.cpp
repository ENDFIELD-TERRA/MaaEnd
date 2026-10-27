#include "VisualHost.h"

#include "Util.h"

#include <algorithm>
#include <sstream>

#include <wrl/event.h>
#include <wrl/implements.h>

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

namespace {

constexpr wchar_t kHostClass[] = L"MaaEnd.VisualHostingExperiment.Host";
constexpr wchar_t kCoverClass[] = L"MaaEnd.VisualHostingExperiment.Cover";
constexpr wchar_t kHostTitle[] = L"MaaEnd visual-hosting experiment";

void SetError(std::string* error, const std::string& what, HRESULT hr)
{
    if (error) {
        *error = what + " failed: " + HexHr(hr);
    }
}

std::vector<std::string> SplitChar(const std::string& text, char separator)
{
    std::vector<std::string> parts;
    size_t start = 0;
    while (true) {
        const size_t pos = text.find(separator, start);
        if (pos == std::string::npos) {
            parts.push_back(text.substr(start));
            return parts;
        }
        parts.push_back(text.substr(start, pos - start));
        start = pos + 1;
    }
}

std::string TrimCr(std::string s)
{
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) {
        s.pop_back();
    }
    return s;
}

}  // namespace

VisualHost::~VisualHost()
{
    Close();
}

bool VisualHost::RegisterClasses()
{
    static bool registered = false;
    if (registered) {
        return true;
    }
    WNDCLASSEXW host {};
    host.cbSize = sizeof(host);
    host.style = CS_HREDRAW | CS_VREDRAW;
    host.lpfnWndProc = HostProc;
    host.hInstance = GetModuleHandleW(nullptr);
    host.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    host.hbrBackground = CreateSolidBrush(RGB(4, 5, 7));
    host.lpszClassName = kHostClass;
    if (!RegisterClassExW(&host) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }

    WNDCLASSEXW cover {};
    cover.cbSize = sizeof(cover);
    cover.style = CS_HREDRAW | CS_VREDRAW;
    cover.lpfnWndProc = CoverProc;
    cover.hInstance = GetModuleHandleW(nullptr);
    cover.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    cover.hbrBackground = CreateSolidBrush(RGB(200, 20, 120));
    cover.lpszClassName = kCoverClass;
    if (!RegisterClassExW(&cover) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        return false;
    }
    registered = true;
    return true;
}

bool VisualHost::CreateHostWindow(int width, int height, int x, int y, std::string* error)
{
    const DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    hwnd_ = CreateWindowExW(0, kHostClass, kHostTitle, style, x, y, width, height, nullptr, nullptr,
        GetModuleHandleW(nullptr), this);
    if (!hwnd_) {
        SetError(error, "CreateWindowExW(host)", HRESULT_FROM_WIN32(GetLastError()));
        return false;
    }
    dpi_ = GetDpiForWindow(hwnd_);
    homeX_ = x;
    homeY_ = y;

    // Force an exact physical-pixel client area regardless of DPI.
    RECT windowRect {};
    RECT clientRect {};
    GetWindowRect(hwnd_, &windowRect);
    GetClientRect(hwnd_, &clientRect);
    const int deltaW = width - (clientRect.right - clientRect.left);
    const int deltaH = height - (clientRect.bottom - clientRect.top);
    SetWindowPos(hwnd_, nullptr, 0, 0, (windowRect.right - windowRect.left) + deltaW,
        (windowRect.bottom - windowRect.top) + deltaH, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    width_ = width;
    height_ = height;
    return true;
}

LRESULT CALLBACK VisualHost::HostProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    VisualHost* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<VisualHost*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        if (self) {
            self->hwnd_ = hwnd;
        }
        return TRUE;
    }
    self = reinterpret_cast<VisualHost*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (msg) {
    case WM_MOUSEWHEEL:
        if (self) {
            ++self->nativeWheelMessages_;
        }
        return 0;
    case WM_MOUSEHWHEEL:
        if (self) {
            ++self->nativeHWheelMessages_;
        }
        return 0;
    case WM_PAINT: {
        PAINTSTRUCT ps {};
        HDC dc = BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        (void)dc;
        return 0;
    }
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK VisualHost::CoverProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_PAINT: {
        PAINTSTRUCT ps {};
        HDC dc = BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        (void)dc;
        return 0;
    }
    default:
        break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool VisualHost::InitComposition(std::string* error)
{
    HRESULT hr = DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&dcompDevice_));
    if (FAILED(hr)) {
        SetError(error, "DCompositionCreateDevice", hr);
        return false;
    }
    hr = dcompDevice_->CreateTargetForHwnd(hwnd_, TRUE, &dcompTarget_);
    if (FAILED(hr)) {
        SetError(error, "IDCompositionDevice::CreateTargetForHwnd", hr);
        return false;
    }
    hr = dcompDevice_->CreateVisual(&rootVisual_);
    if (FAILED(hr)) {
        SetError(error, "IDCompositionDevice::CreateVisual", hr);
        return false;
    }
    hr = dcompTarget_->SetRoot(rootVisual_.Get());
    if (FAILED(hr)) {
        SetError(error, "IDCompositionTarget::SetRoot", hr);
        return false;
    }
    CommitComposition();
    return true;
}

void VisualHost::CommitComposition()
{
    if (dcompDevice_) {
        dcompDevice_->Commit();
    }
}

bool VisualHost::Create(const std::wstring& pagePath, const std::wstring& userDataDir, int width, int height, int x,
    int y, std::string* error)
{
    if (!RegisterClasses()) {
        if (error) {
            *error = "RegisterClassExW failed: " + HexHr(HRESULT_FROM_WIN32(GetLastError()));
        }
        return false;
    }
    EnsureDir(userDataDir);
    if (!CreateHostWindow(width, height, x, y, error)) {
        return false;
    }
    if (!InitComposition(error)) {
        return false;
    }
    ShowWindow(hwnd_, SW_SHOWNOACTIVATE);
    SetWindowPos(hwnd_, nullptr, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    if (!CreateWebView(pagePath, userDataDir, error)) {
        return false;
    }
    return true;
}

bool VisualHost::CreateWebView(const std::wstring& pagePath, const std::wstring& userDataDir, std::string* error)
{
    wchar_t* version = nullptr;
    if (SUCCEEDED(GetAvailableCoreWebView2BrowserVersionString(nullptr, &version)) && version) {
        runtimeVersion_ = version;
        CoTaskMemFree(version);
    }

    bool envDone = false;
    HRESULT envHr = E_PENDING;
    ComPtr<ICoreWebView2Environment> environment;
    auto envHandler = Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
        [&](HRESULT hr, ICoreWebView2Environment* env) -> HRESULT {
            envHr = hr;
            environment = env;
            envDone = true;
            return S_OK;
        });
    const HRESULT envCall = CreateCoreWebView2EnvironmentWithOptions(nullptr, userDataDir.c_str(), nullptr,
        envHandler.Get());
    if (FAILED(envCall)) {
        SetError(error, "CreateCoreWebView2EnvironmentWithOptions", envCall);
        return false;
    }
    if (!PumpUntil([&] { return envDone; }, 30000)) {
        if (error) {
            *error = "timeout waiting for WebView2 environment";
        }
        return false;
    }
    if (FAILED(envHr) || !environment) {
        SetError(error, "WebView2 environment creation", envHr);
        return false;
    }
    environment_ = environment;

    ComPtr<ICoreWebView2Environment3> environment3;
    HRESULT hr = environment_.As(&environment3);
    if (FAILED(hr)) {
        SetError(error, "QI ICoreWebView2Environment3", hr);
        return false;
    }

    bool controllerDone = false;
    HRESULT controllerHr = E_PENDING;
    ComPtr<ICoreWebView2CompositionController> composition;
    auto controllerHandler = Callback<ICoreWebView2CreateCoreWebView2CompositionControllerCompletedHandler>(
        [&](HRESULT callbackHr, ICoreWebView2CompositionController* controller) -> HRESULT {
            controllerHr = callbackHr;
            composition = controller;
            controllerDone = true;
            return S_OK;
        });
    hr = environment3->CreateCoreWebView2CompositionController(hwnd_, controllerHandler.Get());
    if (FAILED(hr)) {
        SetError(error, "CreateCoreWebView2CompositionController", hr);
        return false;
    }
    if (!PumpUntil([&] { return controllerDone; }, 30000)) {
        if (error) {
            *error = "timeout waiting for composition controller";
        }
        return false;
    }
    if (FAILED(controllerHr) || !composition) {
        SetError(error, "composition controller creation", controllerHr);
        return false;
    }
    composition_ = composition;

    hr = composition_.As(&controller_);
    if (FAILED(hr)) {
        SetError(error, "QI ICoreWebView2Controller", hr);
        return false;
    }
    hr = composition_.As(&controller3_);  // Composition controller exposes Controller3 as well.
    if (FAILED(hr)) {
        controller3_.Reset();
    }
    if (controller3_) {
        controller3_->put_ShouldDetectMonitorScaleChanges(FALSE);
    }

    RECT bounds {0, 0, width_, height_};
    hr = controller_->put_Bounds(bounds);
    if (FAILED(hr)) {
        SetError(error, "put_Bounds", hr);
        return false;
    }
    hr = composition_->put_RootVisualTarget(rootVisual_.Get());
    if (FAILED(hr)) {
        SetError(error, "put_RootVisualTarget", hr);
        return false;
    }
    CommitComposition();
    hr = controller_->put_IsVisible(TRUE);
    if (FAILED(hr)) {
        SetError(error, "put_IsVisible", hr);
        return false;
    }

    hr = controller_->get_CoreWebView2(&webview_);
    if (FAILED(hr) || !webview_) {
        SetError(error, "get_CoreWebView2", hr);
        return false;
    }
    webview_->get_BrowserProcessId(&browserPid_);

    ComPtr<ICoreWebView2Settings> settings;
    if (SUCCEEDED(webview_->get_Settings(&settings)) && settings) {
        settings->put_AreDefaultContextMenusEnabled(FALSE);
        settings->put_AreDefaultScriptDialogsEnabled(FALSE);
        settings->put_IsZoomControlEnabled(FALSE);
        settings->put_IsStatusBarEnabled(FALSE);
        settings->put_AreDevToolsEnabled(TRUE);
        ComPtr<ICoreWebView2Settings3> settings3;
        if (SUCCEEDED(settings.As(&settings3))) {
            settings3->put_AreBrowserAcceleratorKeysEnabled(FALSE);
        }
        ComPtr<ICoreWebView2Settings5> settings5;
        if (SUCCEEDED(settings.As(&settings5))) {
            settings5->put_IsPinchZoomEnabled(FALSE);
        }
        ComPtr<ICoreWebView2Settings6> settings6;
        if (SUCCEEDED(settings.As(&settings6))) {
            settings6->put_IsSwipeNavigationEnabled(FALSE);
        }
    }

    EventRegistrationToken token {};
    auto navHandler = Callback<ICoreWebView2NavigationCompletedEventHandler>(
        [&](ICoreWebView2* sender, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
            (void)sender;
            BOOL success = FALSE;
            args->get_IsSuccess(&success);
            navSucceeded_ = success != FALSE;
            navCompleted_ = true;
            return S_OK;
        });
    hr = webview_->add_NavigationCompleted(navHandler.Get(), &token);
    if (FAILED(hr)) {
        SetError(error, "add_NavigationCompleted", hr);
        return false;
    }

    const std::wstring url = FileUrlFromPath(pagePath);
    hr = webview_->Navigate(url.c_str());
    if (FAILED(hr)) {
        SetError(error, "Navigate", hr);
        return false;
    }
    if (!PumpUntil([&] { return navCompleted_; }, 30000)) {
        if (error) {
            *error = "timeout waiting for navigation (" + WideToUtf8(url) + ")";
        }
        return false;
    }
    return true;
}

void VisualHost::Close()
{
    DestroyCover();
    if (controller_) {
        controller_->Close();
    }
    webview_.Reset();
    controller_.Reset();
    composition_.Reset();
    environment_.Reset();
    rootVisual_.Reset();
    dcompTarget_.Reset();
    dcompDevice_.Reset();
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

bool VisualHost::CreateCover(const RECT& hostRect)
{
    if (cover_) {
        return true;
    }
    const int margin = 40;
    const int x = hostRect.left - margin;
    const int y = hostRect.top - margin;
    const int w = (hostRect.right - hostRect.left) + margin * 2;
    const int h = (hostRect.bottom - hostRect.top) + margin * 2;
    cover_ = CreateWindowExW(WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_TOOLWINDOW, kCoverClass, L"cover",
        WS_POPUP, x, y, w, h, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
    if (!cover_) {
        return false;
    }
    ShowWindow(cover_, SW_SHOWNOACTIVATE);
    SetWindowPos(cover_, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
    return true;
}

void VisualHost::DestroyCover()
{
    if (cover_) {
        DestroyWindow(cover_);
        cover_ = nullptr;
    }
}

void VisualHost::ShowHost(bool visible)
{
    if (hwnd_) {
        ShowWindow(hwnd_, visible ? SW_SHOWNOACTIVATE : SW_HIDE);
    }
}

void VisualHost::MinimizeHost()
{
    if (hwnd_) {
        ShowWindow(hwnd_, SW_SHOWMINNOACTIVE);
    }
}

void VisualHost::MoveHost(int x, int y)
{
    if (hwnd_) {
        SetWindowPos(hwnd_, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void VisualHost::SetTopmost(bool topmost)
{
    if (hwnd_) {
        SetWindowPos(hwnd_, topmost ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

void VisualHost::SetBottom()
{
    if (hwnd_) {
        SetWindowPos(hwnd_, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}

bool VisualHost::SetIsVisible(bool visible)
{
    return controller_ && SUCCEEDED(controller_->put_IsVisible(visible ? TRUE : FALSE));
}

bool VisualHost::SetRasterizationScale(double scale)
{
    return controller3_ && SUCCEEDED(controller3_->put_RasterizationScale(scale));
}

bool VisualHost::SetBounds(int width, int height)
{
    if (!controller_) {
        return false;
    }
    width_ = width;
    height_ = height;
    RECT bounds {0, 0, width, height};
    return SUCCEEDED(controller_->put_Bounds(bounds));
}

RECT VisualHost::HostWindowRect() const
{
    RECT rect {};
    if (hwnd_) {
        GetWindowRect(hwnd_, &rect);
    }
    return rect;
}

RECT VisualHost::ClientScreenRect() const
{
    POINT origin {0, 0};
    if (hwnd_) {
        ClientToScreen(hwnd_, &origin);
    }
    return RECT {origin.x, origin.y, origin.x + width_, origin.y + height_};
}

RECT VisualHost::ClientRect() const
{
    return RECT {0, 0, width_, height_};
}

void VisualHost::PumpFor(int ms)
{
    const ULONGLONG end = GetTickCount64() + static_cast<ULONGLONG>(std::max(ms, 0));
    for (;;) {
        MSG msg {};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                continue;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (GetTickCount64() >= end) {
            break;
        }
        MsgWaitForMultipleObjectsEx(0, nullptr, 5, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
    }
}

bool VisualHost::PumpUntil(const std::function<bool()>& predicate, int timeoutMs)
{
    const ULONGLONG end = GetTickCount64() + static_cast<ULONGLONG>(std::max(timeoutMs, 0));
    for (;;) {
        if (predicate()) {
            return true;
        }
        if (GetTickCount64() >= end) {
            return predicate();
        }
        PumpFor(10);
    }
}

std::wstring VisualHost::ExecScriptSync(const std::wstring& script, bool* ok)
{
    if (!webview_) {
        if (ok) {
            *ok = false;
        }
        return {};
    }
    bool done = false;
    bool succeeded = false;
    std::wstring result;
    auto handler = Callback<ICoreWebView2ExecuteScriptCompletedHandler>(
        [&](HRESULT hr, LPCWSTR json) -> HRESULT {
            succeeded = SUCCEEDED(hr) && json != nullptr;
            result = json ? json : L"";
            done = true;
            return S_OK;
        });
    const HRESULT hr = webview_->ExecuteScript(script.c_str(), handler.Get());
    if (FAILED(hr)) {
        if (ok) {
            *ok = false;
        }
        return {};
    }
    if (!PumpUntil([&] { return done; }, 5000)) {
        if (ok) {
            *ok = false;
        }
        return {};
    }
    if (ok) {
        *ok = succeeded;
    }
    return Utf8ToWide(JsonUnescape(WideToUtf8(result)));
}

ProbeSnapshot VisualHost::TakeProbe()
{
    ProbeSnapshot snapshot;
    bool ok = false;
    const std::wstring raw = ExecScriptSync(L"window.__take ? window.__take() : ''", &ok);
    if (!ok || raw.empty()) {
        return snapshot;
    }
    const std::string text = WideToUtf8(raw);
    std::istringstream stream(text);
    std::string line;
    bool first = true;
    while (std::getline(stream, line)) {
        line = TrimCr(line);
        if (line.empty()) {
            continue;
        }
        if (first) {
            first = false;
            const std::vector<std::string> f = SplitChar(line, '|');
            if (f.size() < 12 || f[0] != "META") {
                return snapshot;
            }
            snapshot.valid = true;
            snapshot.ready = f[1] == "1";
            snapshot.innerWidth = std::atoi(f[2].c_str());
            snapshot.innerHeight = std::atoi(f[3].c_str());
            snapshot.dpr = std::atof(f[4].c_str());
            snapshot.leftTop = std::atoi(f[5].c_str());
            snapshot.rightTop = std::atoi(f[6].c_str());
            snapshot.leftLeft = std::atoi(f[7].c_str());
            snapshot.rightLeft = std::atoi(f[8].c_str());
            snapshot.hasFocus = f[9] == "1";
            snapshot.visibility = f[10];
            snapshot.pointerLock = f[11];
            continue;
        }
        const std::vector<std::string> f = SplitChar(line, '|');
        if (f.size() < 7) {
            continue;
        }
        ProbeEvent event;
        event.type = f[0];
        event.x = std::atoi(f[1].c_str());
        event.y = std::atoi(f[2].c_str());
        event.deltaX = std::atof(f[3].c_str());
        event.deltaY = std::atof(f[4].c_str());
        event.target = f[5];
        event.trusted = f[6] == "1";
        snapshot.events.push_back(event);
    }
    return snapshot;
}

bool VisualHost::ResetPage(int scrollTop, int scrollLeft)
{
    const std::wstring script = L"window.__reset ? window.__reset(" + std::to_wstring(scrollTop) + L"," +
        std::to_wstring(scrollLeft) + L") : ''";
    bool ok = false;
    const std::wstring result = ExecScriptSync(script, &ok);
    return ok && result == L"ok";
}

bool VisualHost::WaitForPageReady(int timeoutMs)
{
    const ULONGLONG end = GetTickCount64() + static_cast<ULONGLONG>(std::max(timeoutMs, 0));
    while (GetTickCount64() < end) {
        bool ok = false;
        const std::wstring value = ExecScriptSync(L"window.__probeReady === true ? '1' : '0'", &ok);
        if (ok && value == L"1") {
            return true;
        }
        PumpFor(100);
    }
    return false;
}

HRESULT VisualHost::SendMouse(COREWEBVIEW2_MOUSE_EVENT_KIND kind, int wheelDelta, POINT point,
    COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS keys)
{
    if (!composition_) {
        return E_UNEXPECTED;
    }
    return composition_->SendMouseInput(kind, keys, static_cast<UINT32>(wheelDelta), point);
}

bool VisualHost::CaptureScreen(const std::wstring& path, BmpStats* stats)
{
    const BmpStats captured = CaptureScreenRectToBmp(ClientScreenRect(), path);
    if (stats) {
        *stats = captured;
    }
    return captured.ok;
}

bool VisualHost::CaptureWindow(const std::wstring& path, BmpStats* stats)
{
    const BmpStats captured = CapturePrintWindowToBmp(hwnd_, path);
    if (stats) {
        *stats = captured;
    }
    return captured.ok;
}
