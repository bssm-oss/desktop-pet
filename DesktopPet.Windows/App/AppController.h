#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <memory>

class OverlayWindow;
struct TrayController;
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
    struct PetEntry {
        std::unique_ptr<AppSettings> settings;
        std::unique_ptr<OverlayWindow> window;
        std::unique_ptr<class AnimationPlayer> player;
        std::shared_ptr<FrameSequence> sequence;
        std::map<std::string, std::shared_ptr<FrameSequence>> actionCache;
        double actionRemaining = 0.0;
        double nextDecisionIn = 0.0;
        bool actionLocked = false;
        std::string lastAutoAction;
    };

    void loadAction(PetEntry& pet, const std::string& action, bool manual = true);
    void importAction(PetEntry& pet, const std::string& action);
    void tickScheduler(PetEntry& pet, double deltaSeconds);
    std::string chooseAutoAction(const PetEntry& pet) const;
    double randomIdleDelay(const PetEntry& pet) const;
    void restorePets();
    void addDefaultPet();
    void processPendingRemoves();
    void saveInstanceIDs();
    std::vector<std::string> loadInstanceIDs();
    int nextPetIndex();
    int petCounter_ = 0;

    std::vector<PetEntry> pets_;
    std::unique_ptr<TrayController> tray_;
    HWND msgHwnd_ = nullptr;
    bool running_ = false;
    std::vector<std::string> pendingRemoves_;

    static LRESULT CALLBACK msgWndProc(HWND, UINT, WPARAM, LPARAM);
    static AppController* s_instance;
};
