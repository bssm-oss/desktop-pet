#include "SettingsDialog.h"
#include "AppSettings.h"
#include "../Resource.h"
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>

static AppSettings* g_dlgSettings = nullptr;
static bool g_removeRequested = false;
static std::function<void()> g_onChanged;

static void applyFromUI(HWND hDlg) {
    if (!g_dlgSettings) return;

    g_dlgSettings->playing        = IsDlgButtonChecked(hDlg, IDC_CHK_PLAYING)   == BST_CHECKED;
    g_dlgSettings->alwaysOnTop    = IsDlgButtonChecked(hDlg, IDC_CHK_ONTOP)     == BST_CHECKED;
    g_dlgSettings->clickThrough   = IsDlgButtonChecked(hDlg, IDC_CHK_CLICKTHRU) == BST_CHECKED;
    g_dlgSettings->lockPosition   = IsDlgButtonChecked(hDlg, IDC_CHK_LOCKPOS)   == BST_CHECKED;
    g_dlgSettings->flipHorizontal = IsDlgButtonChecked(hDlg, IDC_CHK_FLIPH)     == BST_CHECKED;
    g_dlgSettings->flipVertical   = IsDlgButtonChecked(hDlg, IDC_CHK_FLIPV)     == BST_CHECKED;

    g_dlgSettings->speed   = SendMessageW(GetDlgItem(hDlg, IDC_SLD_SPEED),   TBM_GETPOS, 0, 0) / 100.0;
    g_dlgSettings->opacity = SendMessageW(GetDlgItem(hDlg, IDC_SLD_OPACITY), TBM_GETPOS, 0, 0) / 100.0;
    g_dlgSettings->scale   = SendMessageW(GetDlgItem(hDlg, IDC_SLD_SCALE),   TBM_GETPOS, 0, 0) / 100.0;

    wchar_t buf[128];
    GetDlgItemTextW(hDlg, IDC_EDT_LABEL, buf, 128);
    g_dlgSettings->label = std::string(buf, buf + wcslen(buf));

    g_dlgSettings->save();
    if (g_onChanged) g_onChanged();
}

static void updateSliderLabel(HWND hDlg, int id) {
    int pos = (int)SendMessageW(GetDlgItem(hDlg, id), TBM_GETPOS, 0, 0);
    wchar_t buf[32];
    if (id == IDC_SLD_SPEED) {
        swprintf(buf, 31, L"%.1fx", pos / 100.0);
        SetDlgItemTextW(hDlg, IDC_LBL_SPEED, buf);
    } else if (id == IDC_SLD_OPACITY) {
        swprintf(buf, 31, L"%d%%", pos);
        SetDlgItemTextW(hDlg, IDC_LBL_OPACITY, buf);
    } else if (id == IDC_SLD_SCALE) {
        swprintf(buf, 31, L"%.2fx", pos / 100.0);
        SetDlgItemTextW(hDlg, IDC_LBL_SCALE, buf);
    }
}

static INT_PTR CALLBACK dlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_INITDIALOG: {
        if (!g_dlgSettings) return TRUE;
        AppSettings& s = *g_dlgSettings;

        std::wstring wlabel(s.label.begin(), s.label.end());
        SetDlgItemTextW(hDlg, IDC_EDT_LABEL, wlabel.c_str());

        auto initSlider = [&](int id, int minV, int maxV, int val) {
            HWND sld = GetDlgItem(hDlg, id);
            SendMessageW(sld, TBM_SETRANGE, TRUE, MAKELPARAM(minV, maxV));
            SendMessageW(sld, TBM_SETPOS,   TRUE, val);
        };
        initSlider(IDC_SLD_SPEED,   10, 400, (int)(s.speed   * 100));
        initSlider(IDC_SLD_OPACITY, 10, 100, (int)(s.opacity * 100));
        initSlider(IDC_SLD_SCALE,   25, 400, (int)(s.scale   * 100));

        CheckDlgButton(hDlg, IDC_CHK_PLAYING,   s.playing        ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHK_ONTOP,     s.alwaysOnTop    ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHK_CLICKTHRU, s.clickThrough   ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHK_LOCKPOS,   s.lockPosition   ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHK_FLIPH,     s.flipHorizontal ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hDlg, IDC_CHK_FLIPV,     s.flipVertical   ? BST_CHECKED : BST_UNCHECKED);

        updateSliderLabel(hDlg, IDC_SLD_SPEED);
        updateSliderLabel(hDlg, IDC_SLD_OPACITY);
        updateSliderLabel(hDlg, IDC_SLD_SCALE);
        return TRUE;
    }
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_BTN_IMPORT:
            applyFromUI(hDlg);
            return TRUE;
        case IDC_BTN_REMOVE:
            g_removeRequested = true;
            DestroyWindow(hDlg);
            return TRUE;
        case IDOK:
        case IDCANCEL:
            applyFromUI(hDlg);
            DestroyWindow(hDlg);
            return TRUE;
        default:
            if (HIWORD(wp) == BN_CLICKED) {
                applyFromUI(hDlg);
                return TRUE;
            }
            break;
        }
        break;
    case WM_HSCROLL:
        updateSliderLabel(hDlg, GetDlgCtrlID((HWND)lp));
        applyFromUI(hDlg);
        return TRUE;
    case WM_CLOSE:
        applyFromUI(hDlg);
        DestroyWindow(hDlg);
        return TRUE;
    }
    return FALSE;
}

void showSettingsDialog(HWND parent, AppSettings& settings, bool* removedOut,
    std::function<void()> onChanged) {
    g_dlgSettings = &settings;
    g_removeRequested = false;
    g_onChanged = std::move(onChanged);

    // The message-only parent window has no monitor → GetDpiForWindow returns 96.
    // Temporarily set PerMonitorV2 context so the dialog and its controls scale
    // to the actual DPI of whichever monitor it appears on.
    auto prevCtx = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    HWND hDlg = CreateDialogW(GetModuleHandle(nullptr),
        MAKEINTRESOURCEW(IDD_SETTINGS), nullptr, dlgProc);

    if (!hDlg) {
        SetThreadDpiAwarenessContext(prevCtx);
        if (removedOut) *removedOut = false;
        return;
    }

    ShowWindow(hDlg, SW_SHOW);
    UpdateWindow(hDlg);

    MSG msg;
    while (IsWindow(hDlg) && GetMessageW(&msg, nullptr, 0, 0)) {
        if (!IsDialogMessageW(hDlg, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    SetThreadDpiAwarenessContext(prevCtx);
    if (removedOut) *removedOut = g_removeRequested;
    g_dlgSettings = nullptr;
    g_removeRequested = false;
    g_onChanged = nullptr;
}

std::wstring openFileDialog(HWND parent) {
    OPENFILENAMEW ofn = {};
    wchar_t szFile[MAX_PATH] = {};
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = parent;
    ofn.lpstrFile    = szFile;
    ofn.nMaxFile     = MAX_PATH;
    ofn.lpstrFilter  = L"Animations\0*.gif;*.png;*.apng\0Video\0*.mp4;*.mov;*.m4v;*.avi;*.mkv\0All\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameW(&ofn)) return szFile;
    return {};
}
