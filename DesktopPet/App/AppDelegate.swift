// AppDelegate.swift
// Entry point. Configures the app as a menubar-only accessory (no dock icon).
// Owns the overlay window controller and menubar controller.
// Restores persisted state on launch.

import AppKit
import SwiftUI

final class AppDelegate: NSObject, NSApplicationDelegate {

    // MARK: - Owned Controllers
    private var overlayWindowController: OverlayWindowController?
    private var menuBarController: MenuBarController?

    // MARK: - Shared Settings (single source of truth)
    let settings = AppSettings()

    // MARK: - Launch
    func applicationDidFinishLaunching(_ notification: Notification) {
        // Menubar-only: no dock icon, no activation on launch
        NSApp.setActivationPolicy(.accessory)

        // Build overlay window
        let overlayWC = OverlayWindowController(settings: settings)
        self.overlayWindowController = overlayWC
        overlayWC.showWindow(nil)

        // Build menubar icon + menu
        let menuBar = MenuBarController(
            settings: settings,
            overlayWindowController: overlayWC
        )
        self.menuBarController = menuBar

        // Restore last asset if available
        if let bookmark = settings.assetBookmark {
            overlayWC.loadAssetFromBookmark(bookmark)
        } else {
            // Show built-in placeholder so the window isn't empty
            overlayWC.loadPlaceholder()
        }
    }

    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        // Menubar app — never quit when window closes
        return false
    }
}
