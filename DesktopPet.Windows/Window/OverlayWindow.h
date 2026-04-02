#pragma once
#include <windows.h>
#include <string>
#include <functional>
#include "../Playback/AnimationPlayer.h"
#include "../Playback/FrameSequence.h"

class AppSettings;

class OverlayWindow : public AnimationPlayer::Delegate {
public:
    OverlayWindow(AppSettings& settings);
    ~OverlayWindow();

    bool create(int x, int y, int w, int h);
    void show();
    void hide();
    void close();
    void resize(int w, int h);
    void moveTo(int x, int y);
    void setClickThrough(bool enabled);
    void setAlwaysOnTop(bool enabled);
    void setOpacity(BYTE alpha);
    FrameSequence* loadAsset(const std::wstring& path);
    void loadPlaceholder();
    void loadSequence(FrameSequence* seq);
    void applySettings();
    void renderLastFrame();

    HWND getHwnd() const { return hwnd_; }
    AppSettings& settings() { return settings_; }

    static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

    std::function<void(OverlayWindow*)> onDragEnd;

private:
    void onFrame(int frameIndex, const FrameSequence::Frame& frame) override;
    void renderFrame(const FrameSequence::Frame& frame);
    void applyScale(double scale);
    void freeRenderCache();
    void loadGIF(const std::wstring& path);
    void loadAPNG(const std::wstring& path);
    void loadPNGSequence(const std::wstring& path);
    void loadVideo(const std::wstring& path);

    HWND hwnd_ = nullptr;
    AppSettings& settings_;
    BYTE opacity_ = 255;
    FrameSequence::Frame lastFrame_;
    int naturalWidth_ = 200;
    int naturalHeight_ = 200;
    bool isVideoMode_ = false;
    bool clickThrough_ = false;
    bool dragActive_ = false;
    POINT dragStartMouse_ = {};
    POINT dragStartWindow_ = {};

    // Cached render resources — reallocated only on size change
    HDC   hdcMem_    = nullptr;
    HBITMAP hbmCache_ = nullptr;
    void* bitsCache_ = nullptr;
    int   cachedW_   = 0;
    int   cachedH_   = 0;

    static const wchar_t* CLASS_NAME;
    static bool classRegistered_;
};
