// AppDelegate.swift
// Manages the collection of pets and the single menubar controller.
// Each pet = one AppSettings instance + one OverlayWindowController.

import AppKit

final class AppDelegate: NSObject, NSApplicationDelegate {

    // MARK: - State
    private var pets: [OverlayWindowController] = []
    private let petsStore = PetsStore()
    private var menuBarController: MenuBarController!

    // MARK: - Launch

    func applicationDidFinishLaunching(_ notification: Notification) {
        NSApp.setActivationPolicy(.accessory)

        menuBarController = MenuBarController()
        menuBarController.onAddPet    = { [weak self] in self?.addPet() }
        menuBarController.onRemovePet = { [weak self] id in self?.removePet(id: id) }

        let savedRecords = petsStore.loadRecords()

        if savedRecords.isEmpty {
            addPet()
        } else {
            for record in savedRecords {
                let settings = makeSettings(record: record)
                let wc = OverlayWindowController(settings: settings)

                if let bookmark = settings.assetBookmark, wc.loadAssetFromBookmark(bookmark) {
                    wc.showWindow(nil)
                    pets.append(wc)
                }
            }
            menuBarController.update(pets: pets)
        }
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return false
    }

    // MARK: - Add / Remove

    func addPet() {
        let urls = menuBarController.openAddPetsPicker()
        guard !urls.isEmpty else { return }

        var didAddAnyPet = false
        for url in urls {
            if addPet(for: url) {
                didAddAnyPet = true
            }
        }

        if didAddAnyPet {
            menuBarController.update(pets: pets)
        }
    }

    func removePet(id: String) {
        guard let idx = pets.firstIndex(where: { $0.settings.instanceID == id }) else { return }
        let wc = pets[idx]
        wc.window?.close()
        pets.remove(at: idx)
        petsStore.remove(instanceID: id)
        menuBarController.update(pets: pets)
    }

    // MARK: - Helpers

    private func addPet(for url: URL) -> Bool {
        let record = petsStore.makeNewPetRecord()
        let settings = makeSettings(record: record)
        let wc = OverlayWindowController(settings: settings)

        guard wc.loadAsset(url: url) else { return false }

        wc.showWindow(nil)
        pets.append(wc)
        petsStore.upsert(settings.toRecord())
        return true
    }

    private func makeSettings(record: PetRecord) -> AppSettings {
        AppSettings(record: record) { [weak self] settings in
            self?.petsStore.upsert(settings.toRecord())
        }
    }
}
