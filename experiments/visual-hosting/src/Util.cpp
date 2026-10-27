#include "Util.h"

#include <shlwapi.h>
#include <tlhelp32.h>
#include <wininet.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <set>

namespace {

std::string SanitizeField(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c == '\t' || c == '\r' || c == '\n' || c == '|') {
            out.push_back(' ');
        }
        else {
            out.push_back(c);
        }
    }
    return out;
}

bool WriteBmp24(const std::wstring& path, int width, int height, const std::vector<uint8_t>& rgbTopDown)
{
    if (width <= 0 || height <= 0 || rgbTopDown.size() < static_cast<size_t>(width) * height * 3) {
        return false;
    }
    const int stride = (width * 3 + 3) & ~3;
    BITMAPFILEHEADER fileHeader {};
    BITMAPINFOHEADER infoHeader {};
    infoHeader.biSize = sizeof(infoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = BI_RGB;
    infoHeader.biSizeImage = static_cast<DWORD>(stride) * height;
    fileHeader.bfType = 0x4D42;
    fileHeader.bfOffBits = sizeof(fileHeader) + sizeof(infoHeader);
    fileHeader.bfSize = fileHeader.bfOffBits + infoHeader.biSizeImage;

    std::ofstream out(std::filesystem::path(path), std::ios::binary);
    if (!out) {
        return false;
    }
    out.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    out.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));
    std::vector<uint8_t> row(static_cast<size_t>(stride), 0);
    for (int y = height - 1; y >= 0; --y) {
        std::fill(row.begin(), row.end(), 0);
        const uint8_t* src = rgbTopDown.data() + static_cast<size_t>(y) * width * 3;
        for (int x = 0; x < width; ++x) {
            row[x * 3 + 0] = src[x * 3 + 2];
            row[x * 3 + 1] = src[x * 3 + 1];
            row[x * 3 + 2] = src[x * 3 + 0];
        }
        out.write(reinterpret_cast<const char*>(row.data()), row.size());
    }
    return out.good();
}

BmpStats AnalyzeAndSave(const std::wstring& path, int width, int height, const std::vector<uint8_t>& bgraTopDown)
{
    BmpStats stats;
    stats.path = path;
    stats.width = width;
    stats.height = height;
    const size_t pixelCount = static_cast<size_t>(width) * height;
    if (pixelCount == 0 || bgraTopDown.size() < pixelCount * 4) {
        return stats;
    }
    std::vector<uint8_t> rgb(pixelCount * 3);
    size_t nonBlack = 0;
    size_t teal = 0;
    std::set<uint32_t> colors;
    for (size_t i = 0; i < pixelCount; ++i) {
        const uint8_t b = bgraTopDown[i * 4 + 0];
        const uint8_t g = bgraTopDown[i * 4 + 1];
        const uint8_t r = bgraTopDown[i * 4 + 2];
        rgb[i * 3 + 0] = r;
        rgb[i * 3 + 1] = g;
        rgb[i * 3 + 2] = b;
        if (static_cast<int>(r) + g + b > 75) {
            ++nonBlack;
        }
        // Header of wheel-page.html is #12b5b0; accept the same hue family.
        if (g > 110 && b > 90 && r < 100 && static_cast<int>(g) - r > 60) {
            ++teal;
        }
        if ((i % 5) == 0 && colors.size() < 20000) {
            colors.insert((static_cast<uint32_t>(r) << 16) | (static_cast<uint32_t>(g) << 8) | b);
        }
    }
    stats.nonBlackFraction = static_cast<double>(nonBlack) / static_cast<double>(pixelCount);
    stats.tealFraction = static_cast<double>(teal) / static_cast<double>(pixelCount);
    stats.uniqueColors = static_cast<int>(colors.size());
    stats.ok = WriteBmp24(path, width, height, rgb);
    return stats;
}

std::vector<std::string> SplitTab(const std::string& line)
{
    std::vector<std::string> fields;
    size_t start = 0;
    while (true) {
        const size_t pos = line.find('\t', start);
        if (pos == std::string::npos) {
            fields.push_back(line.substr(start));
            break;
        }
        fields.push_back(line.substr(start, pos - start));
        start = pos + 1;
    }
    return fields;
}

}  // namespace

std::string WideToUtf8(const std::wstring& w)
{
    if (w.empty()) {
        return {};
    }
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()), nullptr, 0, nullptr, nullptr);
    if (bytes <= 0) {
        return {};
    }
    std::string out(static_cast<size_t>(bytes), '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.data(), static_cast<int>(w.size()), out.data(), bytes, nullptr, nullptr);
    return out;
}

std::wstring Utf8ToWide(const std::string& s)
{
    if (s.empty()) {
        return {};
    }
    const int chars = MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), nullptr, 0);
    if (chars <= 0) {
        return {};
    }
    std::wstring out(static_cast<size_t>(chars), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.data(), static_cast<int>(s.size()), out.data(), chars);
    return out;
}

std::string JsonEscape(const std::string& s)
{
    std::string out;
    out.reserve(s.size() + 8);
    for (unsigned char c : s) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 0x20) {
                char buf[8];
                std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                out += buf;
            }
            else {
                out.push_back(static_cast<char>(c));
            }
        }
    }
    return out;
}

std::string JsonUnescape(const std::string& s)
{
    std::string text = s;
    if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
        text = text.substr(1, text.size() - 2);
    }
    std::string out;
    out.reserve(text.size());
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] != '\\' || i + 1 >= text.size()) {
            out.push_back(text[i]);
            continue;
        }
        const char c = text[++i];
        switch (c) {
        case 'n': out.push_back('\n'); break;
        case 'r': out.push_back('\r'); break;
        case 't': out.push_back('\t'); break;
        case 'b': out.push_back('\b'); break;
        case 'f': out.push_back('\f'); break;
        case '\\': out.push_back('\\'); break;
        case '/': out.push_back('/'); break;
        case '"': out.push_back('"'); break;
        case 'u': {
            if (i + 4 < text.size()) {
                const unsigned code = static_cast<unsigned>(std::strtoul(text.substr(i + 1, 4).c_str(), nullptr, 16));
                i += 4;
                if (code < 0x80) {
                    out.push_back(static_cast<char>(code));
                }
                else if (code < 0x800) {
                    out.push_back(static_cast<char>(0xC0 | (code >> 6)));
                    out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                }
                else {
                    out.push_back(static_cast<char>(0xE0 | (code >> 12)));
                    out.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
                    out.push_back(static_cast<char>(0x80 | (code & 0x3F)));
                }
            }
            break;
        }
        default: out.push_back(c); break;
        }
    }
    return out;
}

std::string HexHr(HRESULT hr)
{
    char buf[24];
    std::snprintf(buf, sizeof(buf), "0x%08lX", static_cast<unsigned long>(hr));
    return buf;
}

std::wstring ExeDirW()
{
    std::wstring path(MAX_PATH, L'\0');
    DWORD len = 0;
    while (true) {
        len = GetModuleFileNameW(nullptr, path.data(), static_cast<DWORD>(path.size()));
        if (len == 0) {
            return L".";
        }
        if (len < path.size()) {
            path.resize(len);
            break;
        }
        path.resize(path.size() * 2);
    }
    const size_t pos = path.find_last_of(L"\\/");
    return pos == std::wstring::npos ? L"." : path.substr(0, pos);
}

std::wstring JoinPath(const std::wstring& a, const std::wstring& b)
{
    if (a.empty()) {
        return b;
    }
    if (b.empty()) {
        return a;
    }
    std::wstring out = a;
    if (out.back() != L'\\' && out.back() != L'/') {
        out.push_back(L'\\');
    }
    out += b;
    return out;
}

bool EnsureDir(const std::wstring& path)
{
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(path), ec);
    return std::filesystem::is_directory(std::filesystem::path(path), ec);
}

bool WriteTextFile(const std::wstring& path, const std::string& text)
{
    std::ofstream out(std::filesystem::path(path), std::ios::binary);
    if (!out) {
        return false;
    }
    out.write(text.data(), static_cast<std::streamsize>(text.size()));
    return out.good();
}

std::string LocalTimestampForPath()
{
    SYSTEMTIME st {};
    GetLocalTime(&st);
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04u%02u%02u-%02u%02u%02u", st.wYear, st.wMonth, st.wDay, st.wHour,
        st.wMinute, st.wSecond);
    return buf;
}

std::string TimestampUtc()
{
    SYSTEMTIME st {};
    GetSystemTime(&st);
    char buf[40];
    std::snprintf(buf, sizeof(buf), "%04u-%02u-%02uT%02u:%02u:%02u.%03uZ", st.wYear, st.wMonth, st.wDay, st.wHour,
        st.wMinute, st.wSecond, st.wMilliseconds);
    return buf;
}

std::wstring MakeRunDir()
{
    std::wstring temp(MAX_PATH, L'\0');
    const DWORD len = GetTempPathW(static_cast<DWORD>(temp.size()), temp.data());
    if (len > 0) {
        temp.resize(len);
    }
    else {
        temp = L".";
    }
    const std::wstring runDir = JoinPath(JoinPath(temp, L"maaend-visual-hosting-exp"),
        L"run-" + Utf8ToWide(LocalTimestampForPath()) + L"-" + std::to_wstring(GetCurrentProcessId()));
    EnsureDir(runDir);
    return runDir;
}

std::wstring FileUrlFromPath(const std::wstring& path)
{
    wchar_t url[INTERNET_MAX_URL_LENGTH] {};
    DWORD len = INTERNET_MAX_URL_LENGTH;
    if (SUCCEEDED(UrlCreateFromPathW(path.c_str(), url, &len, 0))) {
        return url;
    }
    // Fallback: file:/// + path with forward slashes and percent-escaped spaces.
    std::wstring out = L"file:///";
    for (wchar_t c : path) {
        if (c == L'\\') {
            out.push_back(L'/');
        }
        else if (c == L' ') {
            out += L"%20";
        }
        else {
            out.push_back(c);
        }
    }
    return out;
}

std::string GetWindowClassNameUtf8(HWND hwnd)
{
    if (!hwnd) {
        return {};
    }
    wchar_t buf[256] {};
    const int len = GetClassNameW(hwnd, buf, static_cast<int>(std::size(buf)));
    return len > 0 ? WideToUtf8(std::wstring(buf, static_cast<size_t>(len))) : std::string {};
}

std::string GetWindowTitleUtf8(HWND hwnd)
{
    if (!hwnd) {
        return {};
    }
    wchar_t buf[512] {};
    const int len = GetWindowTextW(hwnd, buf, static_cast<int>(std::size(buf)));
    return len > 0 ? WideToUtf8(std::wstring(buf, static_cast<size_t>(len))) : std::string {};
}

std::string WindowInfoJson(HWND hwnd)
{
    if (!hwnd) {
        return "{\"hwnd\":null,\"valid\":false}";
    }
    DWORD pid = 0;
    GetWindowThreadProcessId(hwnd, &pid);
    std::string out = "{\"hwnd\":" + std::to_string(reinterpret_cast<uintptr_t>(hwnd));
    out += ",\"valid\":true,\"class\":\"" + JsonEscape(GetWindowClassNameUtf8(hwnd));
    out += "\",\"title\":\"" + JsonEscape(GetWindowTitleUtf8(hwnd));
    out += "\",\"pid\":" + std::to_string(pid) + "}";
    return out;
}

std::string RectJson(const RECT& r)
{
    return "[" + std::to_string(r.left) + "," + std::to_string(r.top) + "," + std::to_string(r.right) + "," +
        std::to_string(r.bottom) + "]";
}

WfpResult QueryWindowFromPoint(const std::wstring& exePath, int x, int y)
{
    WfpResult result;
    result.queried = true;

    SECURITY_ATTRIBUTES sa {sizeof(sa), nullptr, TRUE};
    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;
    if (!CreatePipe(&readPipe, &writePipe, &sa, 0)) {
        result.hr = HRESULT_FROM_WIN32(GetLastError());
        return result;
    }
    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = writePipe;
    si.hStdError = writePipe;
    si.hStdInput = nullptr;
    PROCESS_INFORMATION pi {};

    std::wstring cmd = L"\"" + exePath + L"\" --wfp " + std::to_wstring(x) + L" " + std::to_wstring(y);
    std::wstring mutableCmd = cmd;
    const BOOL created = CreateProcessW(exePath.c_str(), mutableCmd.data(), nullptr, nullptr, TRUE,
        CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
    CloseHandle(writePipe);
    if (!created) {
        result.hr = HRESULT_FROM_WIN32(GetLastError());
        CloseHandle(readPipe);
        return result;
    }

    std::string output;
    char buf[512];
    DWORD got = 0;
    while (ReadFile(readPipe, buf, sizeof(buf), &got, nullptr) && got > 0) {
        output.append(buf, got);
    }
    WaitForSingleObject(pi.hProcess, 5000);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    CloseHandle(readPipe);

    const size_t lineEnd = output.find_first_of("\r\n");
    const std::string line = output.substr(0, lineEnd);
    if (line.rfind("WFP\t", 0) != 0) {
        return result;
    }
    const std::vector<std::string> f = SplitTab(line);
    if (f.size() < 7) {
        return result;
    }
    result.hwnd = reinterpret_cast<HWND>(static_cast<uintptr_t>(std::strtoull(f[1].c_str(), nullptr, 10)));
    result.root = reinterpret_cast<HWND>(static_cast<uintptr_t>(std::strtoull(f[2].c_str(), nullptr, 10)));
    result.cls = f[3];
    result.title = f[4];
    result.pid = static_cast<DWORD>(std::strtoul(f[5].c_str(), nullptr, 10));
    result.visible = f[6] == "1";
    result.found = result.hwnd != nullptr;
    result.hr = S_OK;
    return result;
}

BmpStats CaptureScreenRectToBmp(const RECT& screenRect, const std::wstring& path)
{
    BmpStats stats;
    stats.path = path;
    const int width = screenRect.right - screenRect.left;
    const int height = screenRect.bottom - screenRect.top;
    if (width <= 0 || height <= 0) {
        return stats;
    }
    HDC screenDc = GetDC(nullptr);
    if (!screenDc) {
        return stats;
    }
    HDC memDc = CreateCompatibleDC(screenDc);
    BITMAPINFO bmi {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;  // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(screenDc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmap || !bits) {
        DeleteDC(memDc);
        ReleaseDC(nullptr, screenDc);
        return stats;
    }
    HGDIOBJ old = SelectObject(memDc, bitmap);
    const BOOL blt = BitBlt(memDc, 0, 0, width, height, screenDc, screenRect.left, screenRect.top, SRCCOPY);
    std::vector<uint8_t> bgra(static_cast<size_t>(width) * height * 4);
    if (blt) {
        std::memcpy(bgra.data(), bits, bgra.size());
    }
    SelectObject(memDc, old);
    DeleteObject(bitmap);
    DeleteDC(memDc);
    ReleaseDC(nullptr, screenDc);
    if (!blt) {
        return stats;
    }
    stats = AnalyzeAndSave(path, width, height, bgra);
    return stats;
}

BmpStats CapturePrintWindowToBmp(HWND hwnd, const std::wstring& path)
{
    BmpStats stats;
    stats.path = path;
    RECT rect {};
    if (!hwnd || !GetWindowRect(hwnd, &rect)) {
        return stats;
    }
    const int width = rect.right - rect.left;
    const int height = rect.bottom - rect.top;
    if (width <= 0 || height <= 0) {
        return stats;
    }
    HDC windowDc = GetDC(hwnd);
    HDC memDc = CreateCompatibleDC(windowDc);
    BITMAPINFO bmi {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    void* bits = nullptr;
    HBITMAP bitmap = CreateDIBSection(windowDc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!bitmap || !bits) {
        DeleteDC(memDc);
        ReleaseDC(hwnd, windowDc);
        return stats;
    }
    HGDIOBJ old = SelectObject(memDc, bitmap);
    const BOOL printed = PrintWindow(hwnd, memDc, PW_RENDERFULLCONTENT);
    std::vector<uint8_t> bgra(static_cast<size_t>(width) * height * 4);
    std::memcpy(bgra.data(), bits, bgra.size());
    SelectObject(memDc, old);
    DeleteObject(bitmap);
    DeleteDC(memDc);
    ReleaseDC(hwnd, windowDc);
    stats.printWindowResult = printed ? 1 : 0;
    BmpStats analyzed = AnalyzeAndSave(path, width, height, bgra);
    analyzed.printWindowResult = stats.printWindowResult;
    return analyzed;
}
