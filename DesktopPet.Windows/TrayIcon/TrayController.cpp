#include "TrayController.h"
#include "../Window/OverlayWindow.h"
#include "../Settings/AppSettings.h"
#include "../Resource.h"
#include <windows.h>
#include <shellapi.h>
#include <algorithm>

TrayController::TrayController() {
    nid_.cbSize = sizeof(nid_);
    nid_.uID = IDR_TRAY_ICON;
    nid_.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = WM_TRAYICON_MSG;
    // Try embedded resource first; fall back to default Windows icon
    nid_.hIcon = (HICON)LoadImageW(
        GetModuleHandle(nullptr),
        MAKEINTRESOURCEW(IDR_TRAY_ICON),
        IMAGE_ICON,
        GetSystemMetrics(SM_CXSMICON),
        GetSystemMetrics(SM_CYSMICON),
        LR_DEFAULTCOLOR);
    if (!nid_.hIcon)
        nid_.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcscpy_s(nid_.szTip, L"Desktop Pet");
}

TrayController::~TrayController() {
    Shell_NotifyIconW(NIM_DELETE, &nid_);
    if (hMenu_) DestroyMenu(hMenu_);
}

void TrayController::addPet(OverlayWindow* pet) {
    pets_.push_back(pet);
}

void TrayController::removePet(const std::string& id) {
    pets_.erase(
        std::remove_if(pets_.begin(), pets_.end(),
            [&id](OverlayWindow* p) { return p->settings().instanceID == id; }),
        pets_.end());
}

void TrayController::update() {}

void TrayController::showContextMenu(HWND hwnd) {
    if (hMenu_) DestroyMenu(hMenu_);
    hMenu_ = CreatePopupMenu();

    AppendMenuW(hMenu_, MF_STRING, WM_PET_ADD, L"Add Pet...");

    int idx = 1;
    for (auto* pet : pets_) {
        AppendMenuW(hMenu_, MF_SEPARATOR, 0, nullptr);

        const std::string& lbl = pet->settings().label;
        std::wstring header = lbl.empty()
            ? L"Pet " + std::to_wstring(idx)
            : std::wstring(lbl.begin(), lbl.end());
        AppendMenuW(hMenu_, MF_GRAYED | MF_STRING, 0, header.c_str());

        AppendMenuW(hMenu_, MF_STRING, WM_PET_SETTINGS + (idx - 1), L"  Settings...");
        AppendMenuW(hMenu_, MF_STRING, WM_PET_SETTINGS + 100 + (idx - 1), L"  Import Animation...");
        AppendMenuW(hMenu_, MF_STRING, WM_PET_REMOVE + (idx - 1), L"  Remove");
        ++idx;
    }

    AppendMenuW(hMenu_, MF_SEPARATOR, 0, nullptr);

    bool startup = isStartupEnabled ? isStartupEnabled() : false;
    AppendMenuW(hMenu_, MF_STRING | (startup ? MF_CHECKED : 0), WM_CMD_STARTUP, L"Start with Windows");

    AppendMenuW(hMenu_, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu_, MF_STRING, WM_PET_QUIT, L"Quit Desktop Pet");

    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow(hwnd);
    // The message-only hwnd has no monitor → menus would render at 96 DPI and
    // get stretched on HiDPI displays. Override to PerMonitorV2 for this call.
    auto prevCtx = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    TrackPopupMenu(hMenu_, TPM_BOTTOMALIGN | TPM_LEFTALIGN,
        pt.x, pt.y, 0, hwnd, nullptr);
    SetThreadDpiAwarenessContext(prevCtx);
    PostMessageW(hwnd, WM_NULL, 0, 0);
}
