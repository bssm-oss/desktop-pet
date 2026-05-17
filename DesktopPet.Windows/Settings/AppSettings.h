#pragma once
#include <windows.h>
#include <string>
#include <map>
#include <vector>

struct AppSettings {
    std::string instanceID;
    double positionX = 200.0;
    double positionY = 200.0;
    double scale = 1.0;
    double opacity = 1.0;
    double speed = 1.0;
    bool clickThrough = false;
    bool lockPosition = false;
    bool alwaysOnTop = true;
    bool playing = true;
    bool flipHorizontal = false;
    bool flipVertical = false;
    bool schedulerEnabled = true;
    int idleDelayMinMs = 2500;
    int idleDelayMaxMs = 5000;
    std::wstring assetPath;
    std::map<std::string, std::wstring> actionPaths;
    std::map<std::string, int> actionWeights = {
        { "gesture", 5 },
        { "reaction", 3 },
        { "locomotion", 2 },
        { "focus", 1 },
        { "special", 1 },
    };
    std::string currentAction = "idle";
    std::string label;

    void load(const std::string& id);
    void save();
    void savePosition(int x, int y);
    void getPosition(int& x, int& y);
    void removeSection();
    std::wstring activeAssetPath() const;
    std::vector<std::string> actionNames() const;
    void setActionPath(const std::string& action, const std::wstring& path);

    static std::wstring configFilePath();
    static void ensureConfigDir();
};
