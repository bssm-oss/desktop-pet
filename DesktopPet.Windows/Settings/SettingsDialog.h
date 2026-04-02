#pragma once
#include <windows.h>
#include <string>
#include <functional>

struct AppSettings;

void showSettingsDialog(HWND parent, AppSettings& settings, bool* removedOut = nullptr,
    std::function<void()> onChanged = nullptr);
std::wstring openFileDialog(HWND parent);
