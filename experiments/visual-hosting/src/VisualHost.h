// Minimal WebView2 visual-hosting host used only by the wheel-delivery experiment.
#pragma once

#include <windows.h>

#include <objbase.h>  // defines the `interface` macro used by WebView2.h
#include <WebView2.h>
#include <dcomp.h>
#include <wrl/client.h>

#include <functional>
#include <string>
#include <vector>

struct ProbeEvent {
    std::string type;
    int x = 0;
    int y = 0;
    double deltaX = 0.0;
    double deltaY = 0.0;
    std::string target;
    bool trusted = false;
};

struct ProbeSnapshot {
    bool valid = false;
    bool ready = false;
    int innerWidth = 0;
    int innerHeight = 0;
    double dpr = 0.0;
    int leftTop = 0;
    int rightTop = 0;
    int leftLeft = 0;
    int rightLeft = 0;
    bool hasFocus = false;
    std::string visibility;
    std::string pointerLock;
    std::vector<ProbeEvent> events;
};

class VisualHost {
public:
    VisualHost() = default;
    ~VisualHost();

    VisualHost(const VisualHost&) = delete;
    VisualHost& operator=(const VisualHost&) = delete;

    bool Create(const std::wstring& pagePath, const std::wstring& userDataDir, int width, int height, int x, int y,
        std::string* error);
    void Close();

    HWND hwnd() const { return hwnd_; }
    HWND coverHwnd() const { return cover_; }
    UINT dpi() const { return dpi_; }
    int width() const { return width_; }
    int height() const { return height_; }
    const std::wstring& runtimeVersion() const { return runtimeVersion_; }
    DWORD browserProcessId() const { return browserPid_; }
    int nativeWheelMessages() const { return nativeWheelMessages_; }
    int nativeHWheelMessages() const { return nativeHWheelMessages_; }

    // Composition / controller controls.
    bool CreateCover(const RECT& hostRect);
    void DestroyCover();
    void ShowHost(bool visible);
    void MinimizeHost();
    void MoveHost(int x, int y);
    void SetTopmost(bool topmost);
    void SetBottom();
    bool SetIsVisible(bool visible);
    bool SetRasterizationScale(double scale);
    bool SetBounds(int width, int height);

    RECT HostWindowRect() const;
    RECT ClientScreenRect() const;
    RECT ClientRect() const;

    bool WaitForPageReady(int timeoutMs);
    std::wstring ExecScriptSync(const std::wstring& script, bool* ok = nullptr);
    ProbeSnapshot TakeProbe();
    bool ResetPage(int scrollTop, int scrollLeft);

    HRESULT SendMouse(COREWEBVIEW2_MOUSE_EVENT_KIND kind, int wheelDelta, POINT point,
        COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS keys = COREWEBVIEW2_MOUSE_EVENT_VIRTUAL_KEYS_NONE);

    void PumpFor(int ms);
    bool PumpUntil(const std::function<bool()>& predicate, int timeoutMs);

    bool CaptureScreen(const std::wstring& path, struct BmpStats* stats);
    bool CaptureWindow(const std::wstring& path, struct BmpStats* stats);

private:
    static LRESULT CALLBACK HostProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK CoverProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool RegisterClasses();
    bool CreateHostWindow(int width, int height, int x, int y, std::string* error);
    bool InitComposition(std::string* error);
    bool CreateWebView(const std::wstring& pagePath, const std::wstring& userDataDir, std::string* error);
    void CommitComposition();

    HWND hwnd_ = nullptr;
    HWND cover_ = nullptr;
    UINT dpi_ = 96;
    int width_ = 0;
    int height_ = 0;
    int homeX_ = 0;
    int homeY_ = 0;
    int nativeWheelMessages_ = 0;
    int nativeHWheelMessages_ = 0;
    bool navCompleted_ = false;
    bool navSucceeded_ = false;
    std::wstring runtimeVersion_;
    UINT32 browserPid_ = 0;

    Microsoft::WRL::ComPtr<IDCompositionDevice> dcompDevice_;
    Microsoft::WRL::ComPtr<IDCompositionTarget> dcompTarget_;
    Microsoft::WRL::ComPtr<IDCompositionVisual> rootVisual_;
    Microsoft::WRL::ComPtr<ICoreWebView2Environment> environment_;
    Microsoft::WRL::ComPtr<ICoreWebView2CompositionController> composition_;
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller_;
    Microsoft::WRL::ComPtr<ICoreWebView2Controller3> controller3_;
    Microsoft::WRL::ComPtr<ICoreWebView2> webview_;
};
