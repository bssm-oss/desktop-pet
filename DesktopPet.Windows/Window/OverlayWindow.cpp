#include "OverlayWindow.h"
#include "../Settings/AppSettings.h"
#include "../Playback/AnimationPlayer.h"
#include "../Playback/GIFDecoder.h"
#include "../Playback/APNGDecoder.h"
#include "../Playback/PNGSequenceDecoder.h"
#include "../Playback/VideoPlayer.h"
#include "../Utilities/PlaceholderAnimation.h"
#include "../Resource.h"
#include <windows.h>
#include <dwmapi.h>
#include <shlwapi.h>
#include <algorithm>

const wchar_t* OverlayWindow::CLASS_NAME = L"DesktopPetOverlayClass";
bool OverlayWindow::classRegistered_ = false;

OverlayWindow::OverlayWindow(AppSettings& settings) : settings_(settings) {}
OverlayWindow::~OverlayWindow() { close(); }

void OverlayWindow::freeRenderCache() {
    if (hdcMem_)   { DeleteDC(hdcMem_);       hdcMem_    = nullptr; }
    if (hbmCache_) { DeleteObject(hbmCache_); hbmCache_  = nullptr; }
    bitsCache_ = nullptr;
    cachedW_ = 0;
    cachedH_ = 0;
}

bool OverlayWindow::create(int x, int y, int w, int h) {
    if (!classRegistered_) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = WndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.lpszClassName = CLASS_NAME;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        RegisterClassExW(&wc);
        classRegistered_ = true;
    }

    hwnd_ = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        CLASS_NAME, L"DesktopPet", WS_POPUP | WS_VISIBLE,
        x, y, w, h,
        nullptr, nullptr, GetModuleHandle(nullptr), this);

    if (!hwnd_) return false;
    SetWindowLongPtrW(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    return true;
}

void OverlayWindow::show() { if (hwnd_) ShowWindow(hwnd_, SW_SHOW); }
void OverlayWindow::hide() { if (hwnd_) ShowWindow(hwnd_, SW_HIDE); }
void OverlayWindow::close() {
    freeRenderCache();
    if (hwnd_) { DestroyWindow(hwnd_); hwnd_ = nullptr; }
}

void OverlayWindow::resize(int w, int h) {
    if (hwnd_) SetWindowPos(hwnd_, nullptr, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void OverlayWindow::moveTo(int x, int y) {
    if (hwnd_) SetWindowPos(hwnd_, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

void OverlayWindow::setClickThrough(bool enabled) {
    clickThrough_ = enabled;
    if (!hwnd_) return;
    LONG exStyle = GetWindowLongW(hwnd_, GWL_EXSTYLE);
    if (enabled) exStyle |= WS_EX_TRANSPARENT;
    else exStyle &= ~WS_EX_TRANSPARENT;
    SetWindowLongW(hwnd_, GWL_EXSTYLE, exStyle);
}

void OverlayWindow::setAlwaysOnTop(bool enabled) {
    if (!hwnd_) return;
    SetWindowPos(hwnd_, enabled ? HWND_TOPMOST : HWND_NOTOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void OverlayWindow::setOpacity(BYTE alpha) { opacity_ = alpha; }

void OverlayWindow::applySettings() {
    setAlwaysOnTop(settings_.alwaysOnTop);
    setClickThrough(settings_.clickThrough);
    opacity_ = (BYTE)(settings_.opacity * 255);
    applyScale(settings_.scale);
}

void OverlayWindow::renderLastFrame() {
    if (lastFrame_.pixels.empty()) return;
    renderFrame(lastFrame_);
}

FrameSequence* OverlayWindow::loadAsset(const std::wstring& path) {
    FrameSequence* seq = nullptr;
    std::wstring ext = PathFindExtensionW(path.c_str());
    std::transform(ext.begin(), ext.end(), ext.begin(), ::towlower);

    if (ext == L".gif") seq = GIFDecoder::decode(path);
    else {
        DWORD attrs = GetFileAttributesW(path.c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY))
            seq = PNGSequenceDecoder::decode(path);
        else
            seq = APNGDecoder::decode(path);
    }
    if (seq) loadSequence(seq);
    settings_.assetPath = path;
    return seq;
}

void OverlayWindow::loadPlaceholder() {
    auto* seq = PlaceholderAnimation::make(200, 24);
    if (seq) loadSequence(seq);
    settings_.assetPath.clear();
}

void OverlayWindow::loadSequence(FrameSequence* seq) {
    if (!seq || seq->frames.empty()) return;
    isVideoMode_ = false;
    naturalWidth_ = seq->frames[0].width;
    naturalHeight_ = seq->frames[0].height;
    lastFrame_ = seq->frames[0];
    applyScale(settings_.scale);
    renderFrame(seq->frames[0]);
}

void OverlayWindow::applyScale(double scale) {
    int newW = (int)(naturalWidth_ * scale);
    int newH = (int)(naturalHeight_ * scale);
    if (hwnd_) {
        RECT rc;
        GetWindowRect(hwnd_, &rc);
        int cx = (rc.left + rc.right) / 2;
        int cy = (rc.top + rc.bottom) / 2;
        SetWindowPos(hwnd_, nullptr, cx - newW / 2, cy - newH / 2, newW, newH,
            SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

void OverlayWindow::onFrame(int frameIndex, const FrameSequence::Frame& frame) {
    lastFrame_ = frame;
    renderFrame(frame);
}

void OverlayWindow::renderFrame(const FrameSequence::Frame& frame) {
    if (!hwnd_ || frame.pixels.empty()) return;

    RECT rc;
    GetWindowRect(hwnd_, &rc);
    int dstW = rc.right  - rc.left;
    int dstH = rc.bottom - rc.top;
    if (dstW <= 0 || dstH <= 0) return;

    // Reallocate destination cache when output size changes
    if (dstW != cachedW_ || dstH != cachedH_) {
        if (hdcMem_)   { DeleteDC(hdcMem_);       hdcMem_   = nullptr; }
        if (hbmCache_) { DeleteObject(hbmCache_); hbmCache_ = nullptr; }
        bitsCache_ = nullptr;

        hdcMem_ = CreateCompatibleDC(nullptr);

        BITMAPINFOHEADER bi = {};
        bi.biSize        = sizeof(bi);
        bi.biWidth       = dstW;
        bi.biHeight      = -dstH;
        bi.biPlanes      = 1;
        bi.biBitCount    = 32;
        bi.biCompression = BI_RGB;

        hbmCache_ = CreateDIBSection(nullptr, (BITMAPINFO*)&bi,
            DIB_RGB_COLORS, &bitsCache_, nullptr, 0);
        if (!hbmCache_) return;

        SelectObject(hdcMem_, hbmCache_);
        cachedW_ = dstW;
        cachedH_ = dstH;
    }

    if (dstW == frame.width && dstH == frame.height && !settings_.flipHorizontal && !settings_.flipVertical) {
        // No scaling or flip — fast path
        memcpy(bitsCache_, frame.pixels.data(), frame.pixels.size());
    } else {
        // Scale and/or flip using AlphaBlend
        HDC hdcSrc = CreateCompatibleDC(nullptr);

        BITMAPINFOHEADER srcBi = {};
        srcBi.biSize        = sizeof(srcBi);
        srcBi.biWidth       = frame.width;
        srcBi.biHeight      = -frame.height;
        srcBi.biPlanes      = 1;
        srcBi.biBitCount    = 32;
        srcBi.biCompression = BI_RGB;

        void* srcBits = nullptr;
        HBITMAP hbmSrc = CreateDIBSection(nullptr, (BITMAPINFO*)&srcBi,
            DIB_RGB_COLORS, &srcBits, nullptr, 0);
        if (!hbmSrc) { DeleteDC(hdcSrc); return; }

        memcpy(srcBits, frame.pixels.data(), frame.pixels.size());
        HBITMAP hbmSrcOld = (HBITMAP)SelectObject(hdcSrc, hbmSrc);

        memset(bitsCache_, 0, (size_t)dstW * dstH * 4);

        BLENDFUNCTION bfScale = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
        int xDst = 0, yDst = 0, wDst = dstW, hDst = dstH;
        int xSrc = 0, ySrc = 0, wSrc = frame.width, hSrc = frame.height;
        if (settings_.flipHorizontal) { xDst = dstW; wDst = -dstW; }
        if (settings_.flipVertical)   { yDst = dstH; hDst = -dstH; }
        AlphaBlend(hdcMem_, xDst, yDst, wDst, hDst,
                   hdcSrc, xSrc, ySrc, wSrc, hSrc, bfScale);

        SelectObject(hdcSrc, hbmSrcOld);
        DeleteObject(hbmSrc);
        DeleteDC(hdcSrc);
    }

    POINT ptSrc = { 0, 0 };
    SIZE  sz    = { dstW, dstH };
    POINT ptDst = { rc.left, rc.top };

    BLENDFUNCTION bf = { AC_SRC_OVER, 0, opacity_, AC_SRC_ALPHA };
    UpdateLayeredWindow(hwnd_, nullptr, &ptDst, &sz,
        hdcMem_, &ptSrc, 0, &bf, ULW_ALPHA);
}

LRESULT CALLBACK OverlayWindow::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    OverlayWindow* self = reinterpret_cast<OverlayWindow*>(
        GetWindowLongPtrW(hwnd, GWLP_USERDATA));

    switch (msg) {
    case WM_NCCREATE: {
        auto cs = reinterpret_cast<CREATESTRUCT*>(lp);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA,
            reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
        return DefWindowProcW(hwnd, msg, wp, lp);
    }
    case WM_LBUTTONDOWN:
        if (self && !self->settings_.lockPosition && !self->clickThrough_) {
            self->dragActive_ = true;
            GetCursorPos(&self->dragStartMouse_);
            RECT rc;
            GetWindowRect(hwnd, &rc);
            self->dragStartWindow_ = { rc.left, rc.top };
            SetCapture(hwnd);
        }
        return 0;
    case WM_MOUSEMOVE:
        if (self && self->dragActive_) {
            POINT cur;
            GetCursorPos(&cur);
            SetWindowPos(hwnd, nullptr,
                self->dragStartWindow_.x + cur.x - self->dragStartMouse_.x,
                self->dragStartWindow_.y + cur.y - self->dragStartMouse_.y,
                0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        return 0;
    case WM_LBUTTONUP:
        if (self && self->dragActive_) {
            self->dragActive_ = false;
            ReleaseCapture();
            if (self->onDragEnd) self->onDragEnd(self);
        }
        return 0;
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_DESTROY:
        if (self) self->hwnd_ = nullptr;
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
