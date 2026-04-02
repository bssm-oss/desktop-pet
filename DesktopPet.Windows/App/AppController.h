#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <memory>

class OverlayWindow;
class TrayController;
class AnimationPlayer;
struct AppSettings;
struct FrameSequence;

class AppController {
public:
    AppController();
    ~AppController();

    void run();
    void addPet();
    void removePet(const std::string& id);
    void quit();

    HWND getMessageWindow() const { return msgHwnd_; }

    static AppController* instance();

    static bool isStartupEnabled();
    static void setStartupEnabled(bool enabled);

private:
    void restorePets();
    void addDefaultPet();
    void processPendingRemoves();
    void saveInstanceIDs();
    std::vector<std::string> loadInstanceIDs();
    int nextPetIndex();
    int petCounter_ = 0;

    struct PetEntry {
        std::unique_ptr<AppSettings> settings;
        std::unique_ptr<OverlayWindow> window;
        std::unique_ptr<class AnimationPlayer> player;
        std::unique_ptr<FrameSequence> sequence;
    };

    std::vector<PetEntry> pets_;
    std::unique_ptr<TrayController> tray_;
    HWND msgHwnd_ = nullptr;
    bool running_ = false;
    std::vector<std::string> pendingRemoves_;

    static LRESULT CALLBACK msgWndProc(HWND, UINT, WPARAM, LPARAM);
    static AppController* s_instance;
};
