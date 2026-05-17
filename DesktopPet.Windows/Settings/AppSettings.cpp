#include "AppSettings.h"
#include <windows.h>
#include <shlobj.h>
#include <algorithm>

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

static std::string toNarrow(const std::wstring& s) {
    return std::string(s.begin(), s.end());
}

static std::vector<std::string> splitCSV(const std::wstring& value) {
    std::vector<std::string> result;
    std::wstring token;
    for (wchar_t ch : value) {
        if (ch == L',') {
            if (!token.empty()) {
                result.push_back(toNarrow(token));
                token.clear();
            }
        } else {
            token += ch;
        }
    }
    if (!token.empty()) result.push_back(toNarrow(token));
    return result;
}

static std::wstring joinCSV(const std::vector<std::string>& values) {
    std::wstring joined;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) joined += L',';
        joined += toWide(values[i]);
    }
    return joined;
}

static int readClampedInt(
    const std::wstring& section,
    const wchar_t* key,
    int defaultValue,
    int minValue,
    int maxValue,
    const std::wstring& path) {
    const int value = GetPrivateProfileIntW(section.c_str(), key, defaultValue, path.c_str());
    return std::clamp(value, minValue, maxValue);
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
    schedulerEnabled = GetPrivateProfileIntW(sec.c_str(), L"schedulerEnabled", 1, p.c_str()) != 0;
    idleDelayMinMs = readClampedInt(sec, L"idleDelayMinMs", 2500, 250, 600000, p);
    idleDelayMaxMs = readClampedInt(sec, L"idleDelayMaxMs", 5000, idleDelayMinMs, 600000, p);

    wchar_t buf[512] = {};
    GetPrivateProfileStringW(sec.c_str(), L"label", L"", buf, 512, p.c_str());
    label = buf[0] ? std::string(buf, buf + wcslen(buf)) : "Pet " + std::to_string(idx);

    wchar_t pathBuf[32767] = {};
    GetPrivateProfileStringW(sec.c_str(), L"assetPath", L"", pathBuf, 32767, p.c_str());
    assetPath = pathBuf;

    wchar_t actionBuf[4096] = {};
    GetPrivateProfileStringW(sec.c_str(), L"actions", L"", actionBuf, 4096, p.c_str());
    for (const auto& action : splitCSV(actionBuf)) {
        std::wstring key = L"action_" + toWide(action);
        wchar_t actionPathBuf[32767] = {};
        GetPrivateProfileStringW(sec.c_str(), key.c_str(), L"", actionPathBuf, 32767, p.c_str());
        if (actionPathBuf[0]) actionPaths[action] = actionPathBuf;
    }

    wchar_t currentActionBuf[128] = {};
    GetPrivateProfileStringW(sec.c_str(), L"currentAction", L"idle", currentActionBuf, 128, p.c_str());
    currentAction = currentActionBuf[0] ? toNarrow(currentActionBuf) : "idle";

    if (actionPaths.empty() && !assetPath.empty()) {
        actionPaths["idle"] = assetPath;
        currentAction = "idle";
    }

    actionWeights["gesture"] = readClampedInt(sec, L"weight_gesture", 5, 0, 1000, p);
    actionWeights["reaction"] = readClampedInt(sec, L"weight_reaction", 3, 0, 1000, p);
    actionWeights["locomotion"] = readClampedInt(sec, L"weight_locomotion", 2, 0, 1000, p);
    actionWeights["focus"] = readClampedInt(sec, L"weight_focus", 1, 0, 1000, p);
    actionWeights["special"] = readClampedInt(sec, L"weight_special", 1, 0, 1000, p);
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
    writeInt(sec, L"schedulerEnabled", schedulerEnabled ? 1 : 0, p);
    writeInt(sec, L"idleDelayMinMs", idleDelayMinMs, p);
    writeInt(sec, L"idleDelayMaxMs", idleDelayMaxMs, p);

    std::wstring wlabel(label.begin(), label.end());
    WritePrivateProfileStringW(sec.c_str(), L"label",     wlabel.c_str(),    p.c_str());
    assetPath = activeAssetPath();
    WritePrivateProfileStringW(sec.c_str(), L"assetPath", assetPath.c_str(), p.c_str());

    const auto names = actionNames();
    std::wstring joinedActions = joinCSV(names);
    WritePrivateProfileStringW(sec.c_str(), L"actions", joinedActions.c_str(), p.c_str());
    WritePrivateProfileStringW(sec.c_str(), L"currentAction", toWide(currentAction).c_str(), p.c_str());
    for (const auto& action : names) {
        std::wstring key = L"action_" + toWide(action);
        WritePrivateProfileStringW(sec.c_str(), key.c_str(), actionPaths[action].c_str(), p.c_str());
    }
    for (const auto& entry : actionWeights) {
        std::wstring key = L"weight_" + toWide(entry.first);
        writeInt(sec, key.c_str(), entry.second, p);
    }
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

std::wstring AppSettings::activeAssetPath() const {
    auto it = actionPaths.find(currentAction);
    if (it != actionPaths.end()) return it->second;
    return assetPath;
}

std::vector<std::string> AppSettings::actionNames() const {
    std::vector<std::string> names;
    names.reserve(actionPaths.size());
    for (const auto& entry : actionPaths) names.push_back(entry.first);
    return names;
}

void AppSettings::setActionPath(const std::string& action, const std::wstring& path) {
    actionPaths[action] = path;
    currentAction = action;
    assetPath = path;
}
