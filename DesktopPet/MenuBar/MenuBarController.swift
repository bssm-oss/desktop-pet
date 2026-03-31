// MenuBarController.swift
// Manages the NSStatusItem (menubar icon) and its dropdown menu.
// Also owns the floating settings panel.

import AppKit
import SwiftUI
import UniformTypeIdentifiers

final class MenuBarController: NSObject {

    // MARK: - Dependencies
    private let settings: AppSettings
    private weak var overlayWC: OverlayWindowController?

    // MARK: - Status Item
    private var statusItem: NSStatusItem!

    // MARK: - Settings Panel
    private var settingsPanel: NSPanel?

    // MARK: - Init

    init(settings: AppSettings, overlayWindowController: OverlayWindowController) {
        self.settings = settings
        self.overlayWC = overlayWindowController
        super.init()
        setupStatusItem()
    }

    // MARK: - Status Item Setup

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
        let event = NSApp.currentEvent
        if event?.type == .rightMouseUp {
            showContextMenu(sender)
        } else {
            toggleSettingsPanel(sender)
        }
    }

    // MARK: - Settings Panel (left click)

    private func toggleSettingsPanel(_ sender: NSStatusBarButton) {
        if let panel = settingsPanel, panel.isVisible {
            panel.orderOut(nil)
            return
        }

        let panel = makeSettingsPanel()
        self.settingsPanel = panel

        // Position below the status bar button
        if let buttonWindow = sender.window {
            let buttonRect = buttonWindow.convertToScreen(sender.frame)
            let panelX = buttonRect.midX - panel.frame.width / 2
            let panelY = buttonRect.minY - panel.frame.height - 4
            panel.setFrameOrigin(NSPoint(x: panelX, y: panelY))
        }

        panel.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
    }

    private func makeSettingsPanel() -> NSPanel {
        let view = SettingsView(
            settings: settings,
            onImport: { [weak self] in self?.openFilePicker() },
            onQuit: { NSApp.terminate(nil) }
        )
        let hosting = NSHostingController(rootView: view)
        hosting.view.setFrameSize(hosting.sizeThatFits(in: NSSize(width: 280, height: 600)))

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

    // MARK: - Context Menu (right click)

    private func showContextMenu(_ sender: NSStatusBarButton) {
        let menu = NSMenu()

        menu.addItem(withTitle: "Import Animation…", action: #selector(openFilePicker), keyEquivalent: "o").target = self
        menu.addItem(.separator())

        let playItem = NSMenuItem(
            title: settings.playing ? "Pause" : "Play",
            action: #selector(togglePlay),
            keyEquivalent: " "
        )
        playItem.target = self
        menu.addItem(playItem)

        menu.addItem(.separator())

        let topItem = NSMenuItem(
            title: settings.alwaysOnTop ? "✓ Always on Top" : "Always on Top",
            action: #selector(toggleAlwaysOnTop),
            keyEquivalent: ""
        )
        topItem.target = self
        menu.addItem(topItem)

        let ctItem = NSMenuItem(
            title: settings.clickThrough ? "✓ Click-Through" : "Click-Through",
            action: #selector(toggleClickThrough),
            keyEquivalent: ""
        )
        ctItem.target = self
        menu.addItem(ctItem)

        let lockItem = NSMenuItem(
            title: settings.lockPosition ? "✓ Lock Position" : "Lock Position",
            action: #selector(toggleLock),
            keyEquivalent: ""
        )
        lockItem.target = self
        menu.addItem(lockItem)

        menu.addItem(.separator())
        menu.addItem(withTitle: "Settings…", action: #selector(openSettings), keyEquivalent: ",").target = self
        menu.addItem(.separator())
        menu.addItem(withTitle: "Quit Desktop Pet", action: #selector(NSApplication.terminate(_:)), keyEquivalent: "q")

        statusItem.menu = menu
        statusItem.button?.performClick(nil)
        statusItem.menu = nil // Reset so left-click works next time
    }

    // MARK: - Actions

    @objc private func openFilePicker() {
        settingsPanel?.orderOut(nil)

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
        panel.message = "Choose a GIF, APNG, PNG sequence folder, or video file"
        panel.prompt = "Open"

        NSApp.activate(ignoringOtherApps: true)
        if panel.runModal() == .OK, let url = panel.url {
            overlayWC?.loadAsset(url: url)
        }
    }

    @objc private func togglePlay() {
        settings.playing.toggle()
    }

    @objc private func toggleAlwaysOnTop() {
        settings.alwaysOnTop.toggle()
    }

    @objc private func toggleClickThrough() {
        settings.clickThrough.toggle()
    }

    @objc private func toggleLock() {
        settings.lockPosition.toggle()
    }

    @objc private func openSettings() {
        if let button = statusItem.button {
            toggleSettingsPanel(button)
        }
    }
}
