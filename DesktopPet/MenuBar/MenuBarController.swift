// MenuBarController.swift
// Manages the NSStatusItem and its menus.
// Supports multiple pets — each with its own settings panel.

import AppKit
import SwiftUI
import UniformTypeIdentifiers

final class MenuBarController: NSObject {

    // MARK: - Callbacks to AppDelegate
    var onAddPet: (() -> Void)?
    var onRemovePet: ((String) -> Void)?   // instanceID

    // MARK: - Status Item
    private var statusItem: NSStatusItem!

    // MARK: - Per-pet panels  [instanceID: NSPanel]
    private var settingsPanels: [String: NSPanel] = [:]

    // MARK: - Pets snapshot (kept in sync by AppDelegate)
    private(set) var pets: [OverlayWindowController] = []

    // MARK: - Init

    override init() {
        super.init()
        setupStatusItem()
    }

    // MARK: - Status Item

    private func setupStatusItem() {
        statusItem = NSStatusBar.system.statusItem(withLength: NSStatusItem.variableLength)
        if let button = statusItem.button {
            button.title = "🐾"
            button.toolTip = "Desktop Pet"
            button.action = #selector(statusButtonClicked)
            button.target = self
            button.sendAction(on: [.leftMouseUp, .rightMouseUp])
        }
    }

    @objc private func statusButtonClicked(_ sender: NSStatusBarButton) {
        if NSApp.currentEvent?.type == .rightMouseUp {
            showContextMenu(sender)
        } else {
            showContextMenu(sender)   // left-click also shows menu for multi-pet
        }
    }

    // MARK: - Context Menu

    private func showContextMenu(_ sender: NSStatusBarButton) {
        let menu = NSMenu()

        // ── Add pet ──────────────────────────────────────────────────────
        let addItem = NSMenuItem(title: "Add Pet…", action: #selector(addPet), keyEquivalent: "n")
        addItem.target = self
        menu.addItem(addItem)

        // ── Per-pet section ───────────────────────────────────────────────
        if !pets.isEmpty {
            menu.addItem(.separator())
            for (idx, pet) in pets.enumerated() {
                // Use the user-defined label instead of "Pet N"
                let header = NSMenuItem(title: pet.settings.label, action: nil, keyEquivalent: "")
                header.isEnabled = false
                menu.addItem(header)

                let settingsItem = NSMenuItem(
                    title: "  Settings…",
                    action: #selector(openPetSettings(_:)),
                    keyEquivalent: ""
                )
                settingsItem.target = self
                settingsItem.representedObject = pet.settings.instanceID
                menu.addItem(settingsItem)

                let importItem = NSMenuItem(
                    title: "  Import Animation…",
                    action: #selector(importForPet(_:)),
                    keyEquivalent: ""
                )
                importItem.target = self
                importItem.representedObject = pet
                menu.addItem(importItem)

                let removeItem = NSMenuItem(
                    title: "  Remove",
                    action: #selector(removePet(_:)),
                    keyEquivalent: ""
                )
                removeItem.target = self
                removeItem.representedObject = pet.settings.instanceID
                menu.addItem(removeItem)

                if idx < pets.count - 1 { menu.addItem(.separator()) }
            }
        }

        // ── Quit ──────────────────────────────────────────────────────────
        menu.addItem(.separator())
        menu.addItem(withTitle: "Quit Desktop Pet",
                     action: #selector(NSApplication.terminate(_:)),
                     keyEquivalent: "q")

        statusItem.menu = menu
        statusItem.button?.performClick(nil)
        statusItem.menu = nil
    }

    // MARK: - Actions

    @objc private func addPet() {
        onAddPet?()
    }

    @objc private func openPetSettings(_ item: NSMenuItem) {
        guard let id = item.representedObject as? String,
              let pet = pets.first(where: { $0.settings.instanceID == id }),
              let button = statusItem.button
        else { return }
        toggleSettingsPanel(for: pet, relativeTo: button)
    }

    @objc private func importForPet(_ item: NSMenuItem) {
        guard let pet = item.representedObject as? OverlayWindowController else { return }
        openFilePicker(for: pet)
    }

    @objc private func removePet(_ item: NSMenuItem) {
        guard let id = item.representedObject as? String else { return }
        settingsPanels[id]?.orderOut(nil)
        settingsPanels.removeValue(forKey: id)
        onRemovePet?(id)
    }

    // MARK: - Settings Panel

    func toggleSettingsPanel(for pet: OverlayWindowController, relativeTo button: NSStatusBarButton) {
        let id = pet.settings.instanceID

        if let panel = settingsPanels[id], panel.isVisible {
            panel.orderOut(nil)
            return
        }

        let panel = makeSettingsPanel(for: pet)
        settingsPanels[id] = panel

        if let buttonWindow = button.window {
            let buttonRect = buttonWindow.convertToScreen(button.frame)
            let panelX = buttonRect.midX - panel.frame.width / 2
            let panelY = buttonRect.minY - panel.frame.height - 4
            panel.setFrameOrigin(NSPoint(x: panelX, y: panelY))
        }

        panel.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
    }

    private func makeSettingsPanel(for pet: OverlayWindowController) -> NSPanel {
        let id = pet.settings.instanceID
        let view = SettingsView(
            settings: pet.settings,
            onImport: { [weak self, weak pet] in
                guard let self, let pet else { return }
                self.openFilePicker(for: pet)
            },
            onRemove: { [weak self] in
                self?.settingsPanels[id]?.orderOut(nil)
                self?.settingsPanels.removeValue(forKey: id)
                self?.onRemovePet?(id)
            },
            onQuit: { NSApp.terminate(nil) }
        )
        let hosting = NSHostingController(rootView: view)
        hosting.view.setFrameSize(hosting.sizeThatFits(in: NSSize(width: 280, height: 700)))

        let panel = NSPanel(
            contentRect: NSRect(origin: .zero, size: hosting.view.frame.size),
            styleMask: [.titled, .closable, .nonactivatingPanel],
            backing: .buffered,
            defer: false
        )
        panel.title = ""
        panel.titlebarAppearsTransparent = true
        panel.isFloatingPanel = true
        panel.level = .floating
        panel.contentViewController = hosting
        panel.isReleasedWhenClosed = false
        return panel
    }

    // MARK: - File Picker

    func openFilePicker(for pet: OverlayWindowController) {
        settingsPanels[pet.settings.instanceID]?.orderOut(nil)

        let panel = NSOpenPanel()
        panel.canChooseFiles = true
        panel.canChooseDirectories = true
        panel.allowsMultipleSelection = false
        panel.allowedContentTypes = [
            UTType.gif,
            UTType.png,
            UTType(filenameExtension: "apng") ?? UTType.png,
            UTType.mpeg4Movie,
            UTType.quickTimeMovie,
            UTType.folder
        ]
        let petName = pet.settings.label
        panel.message = "Choose an animation for \"\(petName)\"\n\nSupported: GIF · APNG · PNG sequence folder · MP4 · MOV"
        panel.prompt = "Open"

        NSApp.activate(ignoringOtherApps: true)
        if panel.runModal() == .OK, let url = panel.url {
            pet.loadAsset(url: url)
        }
    }

    // MARK: - Sync

    /// Called by AppDelegate whenever the pets array changes.
    func update(pets: [OverlayWindowController]) {
        self.pets = pets
        // Close panels for removed pets
        let activeIDs = Set(pets.map { $0.settings.instanceID })
        for id in settingsPanels.keys where !activeIDs.contains(id) {
            settingsPanels[id]?.orderOut(nil)
            settingsPanels.removeValue(forKey: id)
        }
    }
}
