#include "AppSettings.h"
#include <windows.h>
#include <shlobj.h>

static std::wstring configDir() {
    wchar_t path[MAX_PATH] = {};
    SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, path);
    return std::wstring(path) + L"\\DesktopPet";
}

std::wstring AppSettings::configFilePath() {
    return configDir() + L"\\config.ini";
}

void AppSettings::ensureConfigDir() {
    CreateDirectoryW(configDir().c_str(), nullptr);
}

static std::wstring toWide(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

void AppSettings::load(const std::string& id) {
    instanceID = id;

    int idx = 1;
    auto pos = id.find('-');
    if (pos != std::string::npos)
        idx = std::atoi(id.substr(pos + 1).c_str());
    int offset = (idx - 1) * 60;

    std::wstring sec = toWide(id);
    std::wstring p   = configFilePath();

    positionX      = (double)GetPrivateProfileIntW(sec.c_str(), L"x",             200 + offset, p.c_str());
    positionY      = (double)GetPrivateProfileIntW(sec.c_str(), L"y",             200 + offset, p.c_str());
    scale          = GetPrivateProfileIntW(sec.c_str(), L"scale",   100, p.c_str()) / 100.0;
    opacity        = GetPrivateProfileIntW(sec.c_str(), L"opacity", 100, p.c_str()) / 100.0;
    speed          = GetPrivateProfileIntW(sec.c_str(), L"speed",   100, p.c_str()) / 100.0;
    clickThrough   = GetPrivateProfileIntW(sec.c_str(), L"clickThrough",  0, p.c_str()) != 0;
    lockPosition   = GetPrivateProfileIntW(sec.c_str(), L"lockPosition",  0, p.c_str()) != 0;
    alwaysOnTop    = GetPrivateProfileIntW(sec.c_str(), L"alwaysOnTop",   1, p.c_str()) != 0;
    playing        = GetPrivateProfileIntW(sec.c_str(), L"playing",       1, p.c_str()) != 0;
    flipHorizontal = GetPrivateProfileIntW(sec.c_str(), L"flipH",         0, p.c_str()) != 0;
    flipVertical   = GetPrivateProfileIntW(sec.c_str(), L"flipV",         0, p.c_str()) != 0;

    wchar_t buf[512] = {};
    GetPrivateProfileStringW(sec.c_str(), L"label", L"", buf, 512, p.c_str());
    label = buf[0] ? std::string(buf, buf + wcslen(buf)) : "Pet " + std::to_string(idx);

    wchar_t pathBuf[32767] = {};
    GetPrivateProfileStringW(sec.c_str(), L"assetPath", L"", pathBuf, 32767, p.c_str());
    assetPath = pathBuf;
}

static void writeInt(const std::wstring& sec, const wchar_t* key, int val, const std::wstring& p) {
    WritePrivateProfileStringW(sec.c_str(), key, std::to_wstring(val).c_str(), p.c_str());
}

void AppSettings::save() {
    ensureConfigDir();
    std::wstring sec = toWide(instanceID);
    std::wstring p   = configFilePath();

    writeInt(sec, L"x",            (int)positionX,         p);
    writeInt(sec, L"y",            (int)positionY,         p);
    writeInt(sec, L"scale",        (int)(scale * 100),     p);
    writeInt(sec, L"opacity",      (int)(opacity * 100),   p);
    writeInt(sec, L"speed",        (int)(speed * 100),     p);
    writeInt(sec, L"clickThrough", clickThrough   ? 1 : 0, p);
    writeInt(sec, L"lockPosition", lockPosition   ? 1 : 0, p);
    writeInt(sec, L"alwaysOnTop",  alwaysOnTop    ? 1 : 0, p);
    writeInt(sec, L"playing",      playing        ? 1 : 0, p);
    writeInt(sec, L"flipH",        flipHorizontal ? 1 : 0, p);
    writeInt(sec, L"flipV",        flipVertical   ? 1 : 0, p);

    std::wstring wlabel(label.begin(), label.end());
    WritePrivateProfileStringW(sec.c_str(), L"label",     wlabel.c_str(),    p.c_str());
    WritePrivateProfileStringW(sec.c_str(), L"assetPath", assetPath.c_str(), p.c_str());
}

void AppSettings::savePosition(int x, int y) {
    positionX = x;
    positionY = y;
    ensureConfigDir();
    std::wstring sec = toWide(instanceID);
    std::wstring p   = configFilePath();
    WritePrivateProfileStringW(sec.c_str(), L"x", std::to_wstring(x).c_str(), p.c_str());
    WritePrivateProfileStringW(sec.c_str(), L"y", std::to_wstring(y).c_str(), p.c_str());
}

void AppSettings::getPosition(int& x, int& y) {
    x = (int)positionX;
    y = (int)positionY;
}

void AppSettings::removeSection() {
    ensureConfigDir();
    std::wstring sec = toWide(instanceID);
    std::wstring p   = configFilePath();
    // NULL key name removes the entire section
    WritePrivateProfileStringW(sec.c_str(), nullptr, nullptr, p.c_str());
}
