#include "AppController.h"
#include "../Window/OverlayWindow.h"
#include "../Playback/AnimationPlayer.h"
#include "../TrayIcon/TrayController.h"
#include "../Settings/AppSettings.h"
#include "../Settings/SettingsDialog.h"
#include "../Utilities/PlaceholderAnimation.h"
#include "../Resource.h"
#include <windows.h>
#include <chrono>
#include <algorithm>
#include <cstdlib>

AppController* AppController::s_instance = nullptr;
AppController::AppController() { s_instance = this; }
AppController::~AppController() { s_instance = nullptr; }
AppController* AppController::instance() { return s_instance; }

bool AppController::isStartupEnabled() {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_READ, &hKey) != ERROR_SUCCESS) return false;
    wchar_t buf[MAX_PATH] = {};
    DWORD size = sizeof(buf);
    LSTATUS st = RegQueryValueExW(hKey, L"DesktopPet", nullptr, nullptr,
        (BYTE*)buf, &size);
    RegCloseKey(hKey);
    return st == ERROR_SUCCESS;
}

void AppController::setStartupEnabled(bool enabled) {
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
        0, KEY_WRITE, &hKey) != ERROR_SUCCESS) return;
    if (enabled) {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        RegSetValueExW(hKey, L"DesktopPet", 0, REG_SZ,
            (const BYTE*)exePath, (DWORD)((wcslen(exePath) + 1) * sizeof(wchar_t)));
    } else {
        RegDeleteValueW(hKey, L"DesktopPet");
    }
    RegCloseKey(hKey);
}

static const wchar_t* MSG_WND_CLASS = L"DesktopPetMsgWindow";

LRESULT CALLBACK AppController::msgWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    auto* ctrl = AppController::instance();
    if (!ctrl) return DefWindowProcW(hwnd, msg, wp, lp);

    switch (msg) {
    case TrayController::WM_TRAYICON_MSG:
        if (LOWORD(lp) == WM_RBUTTONUP)
            ctrl->tray_->showContextMenu(hwnd);
        return 0;

    case WM_COMMAND: {
        int cmd = LOWORD(wp);
        if (cmd == WM_PET_ADD) {
            ctrl->addPet();
        } else if (cmd == WM_PET_QUIT) {
            ctrl->quit();
        } else if (cmd == WM_CMD_STARTUP) {
            bool cur = AppController::isStartupEnabled();
            AppController::setStartupEnabled(!cur);
        } else if (cmd >= WM_PET_REMOVE && cmd < WM_PET_REMOVE + 100) {
            int idx = cmd - WM_PET_REMOVE;
            if (idx >= 0 && idx < (int)ctrl->pets_.size()) {
                auto* p = new std::string(ctrl->pets_[idx].settings->instanceID);
                PostMessageW(hwnd, WM_APP_REMOVE_PET, (WPARAM)p, 0);
            }
        } else if (cmd >= WM_PET_SETTINGS && cmd < WM_PET_SETTINGS + 100) {
            int idx = cmd - WM_PET_SETTINGS;
            if (idx >= 0 && idx < (int)ctrl->pets_.size()) {
                bool removed = false;
                showSettingsDialog(hwnd, *ctrl->pets_[idx].settings, &removed);
                if (removed) {
                    auto* p = new std::string(ctrl->pets_[idx].settings->instanceID);
                    PostMessageW(hwnd, WM_APP_REMOVE_PET, (WPARAM)p, 0);
                } else {
                    ctrl->pets_[idx].window->applySettings();
                    ctrl->pets_[idx].window->renderLastFrame();
                }
            }
        } else if (cmd >= WM_PET_SETTINGS + 100 && cmd < WM_PET_SETTINGS + 200) {
            int idx = cmd - WM_PET_SETTINGS - 100;
            if (idx >= 0 && idx < (int)ctrl->pets_.size()) {
                std::wstring path = openFileDialog(hwnd);
                if (!path.empty()) {
                auto* seq = ctrl->pets_[idx].window->loadAsset(path);
                if (seq) {
                    ctrl->pets_[idx].player->load(*seq);
                    ctrl->pets_[idx].sequence.reset(seq);
                    if (ctrl->pets_[idx].settings->playing)
                        ctrl->pets_[idx].player->play();
                }
            }
            }
        }
        return 0;
    }

    case WM_APP_REMOVE_PET: {
        auto* idPtr = reinterpret_cast<std::string*>(wp);
        if (idPtr) {
            std::string id = *idPtr;
            delete idPtr;
            ctrl->pendingRemoves_.push_back(id);
        }
        return 0;
    }

    case WM_TIMER:
        if (wp == TIMER_ANIMATION) {
            ctrl->processPendingRemoves();
            static auto lastTime = std::chrono::high_resolution_clock::now();
            auto now = std::chrono::high_resolution_clock::now();
            double delta = std::chrono::duration<double>(now - lastTime).count();
            lastTime = now;
            if (delta > 0.1) delta = 0.016;
            for (auto& pet : ctrl->pets_) {
                if (pet.player) pet.player->tick(delta);
            }
        }
        return 0;

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ANIMATION);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

void AppController::processPendingRemoves() {
    if (pendingRemoves_.empty()) return;
    for (auto& rid : pendingRemoves_) {
        auto it = std::find_if(pets_.begin(), pets_.end(),
            [&rid](const PetEntry& e) { return e.settings->instanceID == rid; });
        if (it == pets_.end()) continue;
        tray_->removePet(rid);
        it->player->stop();
        it->player->setDelegate(nullptr);
        it->settings->removeSection();
        ShowWindow(it->window->getHwnd(), SW_HIDE);
        pets_.erase(it);
    }
    pendingRemoves_.clear();
    if (pets_.empty()) addDefaultPet();
    else saveInstanceIDs();
}

void AppController::run() {
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = msgWndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = MSG_WND_CLASS;
    RegisterClassExW(&wc);

    msgHwnd_ = CreateWindowExW(0, MSG_WND_CLASS, L"DesktopPetMsg", 0,
        0, 0, 0, 0, HWND_MESSAGE, nullptr, GetModuleHandle(nullptr), nullptr);

    tray_ = std::make_unique<TrayController>();
    tray_->setHwnd(msgHwnd_);
    tray_->addIcon();
    tray_->onToggleStartup = []() {
        bool cur = AppController::isStartupEnabled();
        AppController::setStartupEnabled(!cur);
    };
    tray_->isStartupEnabled = []() { return AppController::isStartupEnabled(); };

    SetTimer(msgHwnd_, TIMER_ANIMATION, 16, nullptr);
    restorePets();
    if (pets_.empty()) addDefaultPet();

    running_ = true;
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    running_ = false;
    pets_.clear();
    DestroyWindow(msgHwnd_);
}

void AppController::addPet() {
    int idx = nextPetIndex();
    std::string id = "pet-" + std::to_string(idx);
    auto settings = std::make_unique<AppSettings>();
    settings->load(id);

    std::wstring path = openFileDialog(msgHwnd_);
    if (path.empty()) return;

    auto wc = std::make_unique<OverlayWindow>(*settings);
    int x, y;
    settings->getPosition(x, y);
    wc->create(x, y, 300, 300);

    auto player = std::make_unique<AnimationPlayer>();
    player->setDelegate(wc.get());
    player->setSpeed(settings->speed);

    FrameSequence* seq = wc->loadAsset(path);
    if (seq) player->load(*seq);
    wc->show();
    wc->applySettings();
    if (settings->playing) player->play();

    wc->onDragEnd = [&s = *settings](OverlayWindow* w) {
        RECT rc; GetWindowRect(w->getHwnd(), &rc);
        s.savePosition(rc.left, rc.top);
    };

    tray_->addPet(wc.get());
    PetEntry entry;
    entry.settings = std::move(settings);
    entry.window = std::move(wc);
    entry.player = std::move(player);
    entry.sequence.reset(seq);
    pets_.push_back(std::move(entry));
    saveInstanceIDs();
}

void AppController::addDefaultPet() {
    int idx = nextPetIndex();
    std::string id = "pet-" + std::to_string(idx);
    auto settings = std::make_unique<AppSettings>();
    settings->load(id);

    auto wc = std::make_unique<OverlayWindow>(*settings);
    int x, y;
    settings->getPosition(x, y);
    wc->create(x, y, 200, 200);

    auto player = std::make_unique<AnimationPlayer>();
    player->setDelegate(wc.get());
    player->setSpeed(settings->speed);

    FrameSequence* rawSeq = PlaceholderAnimation::make(200, 24);
    if (rawSeq) { wc->loadSequence(rawSeq); player->load(*rawSeq); }
    wc->show();
    wc->applySettings();
    if (settings->playing) player->play();

    wc->onDragEnd = [&s = *settings](OverlayWindow* w) {
        RECT rc; GetWindowRect(w->getHwnd(), &rc);
        s.savePosition(rc.left, rc.top);
    };

    tray_->addPet(wc.get());
    PetEntry entry;
    entry.settings = std::move(settings);
    entry.window = std::move(wc);
    entry.player = std::move(player);
    entry.sequence.reset(rawSeq);
    pets_.push_back(std::move(entry));
    saveInstanceIDs();
}

void AppController::removePet(const std::string& id) {
    auto it = std::find_if(pets_.begin(), pets_.end(),
        [&id](const PetEntry& e) { return e.settings->instanceID == id; });
    if (it == pets_.end()) return;
    tray_->removePet(id);
    it->player->stop();
    it->player->setDelegate(nullptr);
    it->settings->removeSection();
    ShowWindow(it->window->getHwnd(), SW_HIDE);
    pets_.erase(it);
    if (pets_.empty()) addDefaultPet();
    else saveInstanceIDs();
}

void AppController::quit() {
    for (auto& pet : pets_) {
        pet.player->stop();
        pet.player->setDelegate(nullptr);
        pet.settings->save();
        RECT rc;
        if (pet.window->getHwnd() && GetWindowRect(pet.window->getHwnd(), &rc))
            pet.settings->savePosition(rc.left, rc.top);
    }
    saveInstanceIDs();
    PostMessageW(msgHwnd_, WM_DESTROY, 0, 0);
}

int AppController::nextPetIndex() { return ++petCounter_; }

std::vector<std::string> AppController::loadInstanceIDs() {
    std::wstring p = AppSettings::configFilePath();
    wchar_t buf[4096] = {};
    GetPrivateProfileStringW(L"global", L"ids", L"", buf, 4096, p.c_str());

    std::vector<std::string> result;
    std::wstring token;
    for (wchar_t ch : std::wstring(buf)) {
        if (ch == L',') {
            if (!token.empty()) {
                result.push_back(std::string(token.begin(), token.end()));
                token.clear();
            }
        } else {
            token += ch;
        }
    }
    if (!token.empty())
        result.push_back(std::string(token.begin(), token.end()));
    return result;
}

void AppController::saveInstanceIDs() {
    AppSettings::ensureConfigDir();
    std::wstring p = AppSettings::configFilePath();
    std::wstring ids;
    for (size_t i = 0; i < pets_.size(); ++i) {
        if (i > 0) ids += L',';
        const auto& id = pets_[i].settings->instanceID;
        ids += std::wstring(id.begin(), id.end());
    }
    WritePrivateProfileStringW(L"global", L"ids", ids.c_str(), p.c_str());
}

void AppController::restorePets() {
    auto ids = loadInstanceIDs();
    for (auto& id : ids) {
        auto settings = std::make_unique<AppSettings>();
        settings->load(id);
        auto wc = std::make_unique<OverlayWindow>(*settings);
        int x, y;
        settings->getPosition(x, y);
        wc->create(x, y, 300, 300);
        auto player = std::make_unique<AnimationPlayer>();
        player->setDelegate(wc.get());
        player->setSpeed(settings->speed);
        FrameSequence* seq = nullptr;
        if (!settings->assetPath.empty()) {
            seq = wc->loadAsset(settings->assetPath);
            if (seq) player->load(*seq);
        }
        wc->show();
        wc->applySettings();
        if (settings->playing) player->play();
        wc->onDragEnd = [&s = *settings](OverlayWindow* w) {
            RECT rc; GetWindowRect(w->getHwnd(), &rc);
            s.savePosition(rc.left, rc.top);
        };
        tray_->addPet(wc.get());
        PetEntry entry;
        entry.settings = std::move(settings);
        entry.window = std::move(wc);
        entry.player = std::move(player);
        entry.sequence.reset(seq);
        pets_.push_back(std::move(entry));
        int num = 0;
        if (id.rfind("pet-", 0) == 0) {
            num = std::atoi(id.c_str() + 4);
        }
        if (num > petCounter_) petCounter_ = num;
    }
}
