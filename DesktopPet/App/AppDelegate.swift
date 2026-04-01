// AppDelegate.swift
// Manages the collection of pets and the single menubar controller.
// Each pet = one AppSettings instance + one OverlayWindowController.

import AppKit

final class AppDelegate: NSObject, NSApplicationDelegate {

    // MARK: - State
    private var pets: [OverlayWindowController] = []
    private var menuBarController: MenuBarController!

    // Persisted list of instance IDs so we restore all pets on relaunch
    private let kInstanceIDs = "petInstanceIDs"

    /// Returns a collision-free ID by taking the max existing numeric suffix and adding 1.
    /// Using count-based IDs caused collisions: deleting pet-2 from ["pet-1","pet-2","pet-3"]
    /// made the next pet also "pet-3", silently overwriting its UserDefaults keys.
    private var nextPetID: String {
        let existing = UserDefaults.standard.array(forKey: kInstanceIDs) as? [String] ?? []
        let maxIdx = existing
            .compactMap { Int($0.replacingOccurrences(of: "pet-", with: "")) }
            .max() ?? 0
        return "pet-\(maxIdx + 1)"
    }

    // MARK: - Launch

    func applicationDidFinishLaunching(_ notification: Notification) {
        NSApp.setActivationPolicy(.accessory)

        menuBarController = MenuBarController()
        menuBarController.onAddPet    = { [weak self] in self?.addPet() }
        menuBarController.onRemovePet = { [weak self] id in self?.removePet(id: id) }

        // Restore previously open pets
        let savedIDs = UserDefaults.standard.array(forKey: kInstanceIDs) as? [String] ?? []

        if savedIDs.isEmpty {
            // First launch: open file picker immediately for one pet
            addPet()
        } else {
            for id in savedIDs {
                let settings = AppSettings(instanceID: id)
                let wc = OverlayWindowController(settings: settings)
                wc.showWindow(nil)
                if let bookmark = settings.assetBookmark {
                    wc.loadAssetFromBookmark(bookmark)
                }
                pets.append(wc)
            }
            menuBarController.update(pets: pets)
        }
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return false
    }

    // MARK: - Add / Remove

    /// Creates a new pet, opens the file picker for it.
    /// The window only appears after the user picks a file.
    func addPet() {
        let id = nextPetID
        let settings = AppSettings(instanceID: id)
        let wc = OverlayWindowController(settings: settings)

        // Show file picker — pet window appears only on successful pick
        menuBarController.openFilePicker(for: wc) // picks file → calls wc.loadAsset
        // After picker, check if an asset was actually loaded (bookmark set)
        if settings.assetBookmark != nil {
            wc.showWindow(nil)
            pets.append(wc)
            saveInstanceIDs()
            menuBarController.update(pets: pets)
        }
        // If user cancelled picker, wc is just discarded
    }

    func removePet(id: String) {
        guard let idx = pets.firstIndex(where: { $0.settings.instanceID == id }) else { return }
        let wc = pets[idx]
        wc.settings.removeAllKeys()
        wc.window?.close()
        pets.remove(at: idx)
        saveInstanceIDs()
        menuBarController.update(pets: pets)
    }

    // MARK: - Persistence

    private func saveInstanceIDs() {
        let ids = pets.map { $0.settings.instanceID }
        UserDefaults.standard.set(ids, forKey: kInstanceIDs)
    }
}
