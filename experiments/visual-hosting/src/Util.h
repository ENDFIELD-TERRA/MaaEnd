// Shared Win32 / text / capture helpers for the visual-hosting wheel experiment.
#pragma once

#include <windows.h>

#include <cstdint>
#include <string>
#include <vector>

std::string WideToUtf8(const std::wstring& w);
std::wstring Utf8ToWide(const std::string& s);
std::string JsonEscape(const std::string& s);
std::string JsonUnescape(const std::string& s);
std::string HexHr(HRESULT hr);

std::wstring ExeDirW();
std::wstring JoinPath(const std::wstring& a, const std::wstring& b);
bool EnsureDir(const std::wstring& path);
bool WriteTextFile(const std::wstring& path, const std::string& text);
std::wstring MakeRunDir();
std::string TimestampUtc();
std::string LocalTimestampForPath();
std::wstring FileUrlFromPath(const std::wstring& path);

std::string GetWindowClassNameUtf8(HWND hwnd);
std::string GetWindowTitleUtf8(HWND hwnd);
std::string WindowInfoJson(HWND hwnd);
std::string RectJson(const RECT& r);

struct WfpResult {
    bool queried = false;
    bool found = false;
    HWND hwnd = nullptr;
    HWND root = nullptr;
    std::string cls;
    std::string title;
    DWORD pid = 0;
    bool visible = false;
    HRESULT hr = S_OK;
};

// Spawns this executable with `--wfp x y` so WindowFromPoint is evaluated from a
// secondary process, and parses its single-line answer.
WfpResult QueryWindowFromPoint(const std::wstring& exePath, int x, int y);

struct BmpStats {
    bool ok = false;
    int width = 0;
    int height = 0;
    double nonBlackFraction = 0.0;
    double tealFraction = 0.0;
    int uniqueColors = 0;
    unsigned long printWindowResult = 0;
    std::wstring path;
};

// Screen capture of an on-screen rectangle (physical pixels). Requires the
// rectangle to be unobstructed.
BmpStats CaptureScreenRectToBmp(const RECT& screenRect, const std::wstring& path);

// PrintWindow(PW_RENDERFULLCONTENT) capture of the whole window.
BmpStats CapturePrintWindowToBmp(HWND hwnd, const std::wstring& path);
