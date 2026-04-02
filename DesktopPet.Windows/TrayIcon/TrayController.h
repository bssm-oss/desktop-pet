#pragma once
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>
#include <functional>

class OverlayWindow;

struct TrayController {
    TrayController();
    ~TrayController();

    void addPet(OverlayWindow* pet);
    void removePet(const std::string& id);
    void update();
    void showContextMenu(HWND hwnd);
    void setHwnd(HWND hwnd) { nid_.hWnd = hwnd; }
    void addIcon() { Shell_NotifyIconW(NIM_ADD, &nid_); }
    void removeIcon() { Shell_NotifyIconW(NIM_DELETE, &nid_); }

    std::function<void()> onAddPet;
    std::function<void(const std::string&)> onRemovePet;
    std::function<void()> onQuit;
    std::function<void()> onToggleStartup;
    std::function<bool()> isStartupEnabled;

    static const UINT WM_TRAYICON_MSG = WM_USER + 200;

private:
    NOTIFYICONDATAW nid_ = {};
    std::vector<OverlayWindow*> pets_;
    HMENU hMenu_ = nullptr;
};
