#pragma once
#include <windows.h>
#include <string>

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
    std::wstring assetPath;
    std::string label;

    void load(const std::string& id);
    void save();
    void savePosition(int x, int y);
    void getPosition(int& x, int& y);
    void removeSection();

    static std::wstring configFilePath();
    static void ensureConfigDir();
};
