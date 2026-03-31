// AppSettings.swift
// Single source of truth for all persisted app state.
// Uses UserDefaults under the hood. Observable so SwiftUI views react to changes.

import Foundation
import Combine
import AppKit

final class AppSettings: ObservableObject {

    // MARK: - Keys
    private enum Key: String {
        case positionX, positionY
        case scale, opacity, speed
        case clickThrough, lockPosition, alwaysOnTop
        case playing
        case assetBookmark
    }

    private let defaults = UserDefaults.standard

    // MARK: - Published Properties
    // Each setter persists immediately to UserDefaults.

    @Published var positionX: Double {
        didSet { defaults.set(positionX, forKey: Key.positionX.rawValue) }
    }
    @Published var positionY: Double {
        didSet { defaults.set(positionY, forKey: Key.positionY.rawValue) }
    }
    @Published var scale: Double {
        didSet { defaults.set(scale, forKey: Key.scale.rawValue) }
    }
    @Published var opacity: Double {
        didSet { defaults.set(opacity, forKey: Key.opacity.rawValue) }
    }
    @Published var speed: Double {
        didSet { defaults.set(speed, forKey: Key.speed.rawValue) }
    }
    @Published var clickThrough: Bool {
        didSet { defaults.set(clickThrough, forKey: Key.clickThrough.rawValue) }
    }
    @Published var lockPosition: Bool {
        didSet { defaults.set(lockPosition, forKey: Key.lockPosition.rawValue) }
    }
    @Published var alwaysOnTop: Bool {
        didSet { defaults.set(alwaysOnTop, forKey: Key.alwaysOnTop.rawValue) }
    }
    @Published var playing: Bool {
        didSet { defaults.set(playing, forKey: Key.playing.rawValue) }
    }

    // Security-scoped bookmark for the last imported asset
    var assetBookmark: Data? {
        get { defaults.data(forKey: Key.assetBookmark.rawValue) }
        set { defaults.set(newValue, forKey: Key.assetBookmark.rawValue) }
    }

    // MARK: - Init (load from UserDefaults with sensible defaults)
    init() {
        // Register defaults so first-launch values are sane
        defaults.register(defaults: [
            Key.positionX.rawValue: 200.0,
            Key.positionY.rawValue: 200.0,
            Key.scale.rawValue: 1.0,
            Key.opacity.rawValue: 1.0,
            Key.speed.rawValue: 1.0,
            Key.clickThrough.rawValue: false,
            Key.lockPosition.rawValue: false,
            Key.alwaysOnTop.rawValue: true,
            Key.playing.rawValue: true,
        ])

        positionX    = defaults.double(forKey: Key.positionX.rawValue)
        positionY    = defaults.double(forKey: Key.positionY.rawValue)
        scale        = defaults.double(forKey: Key.scale.rawValue)
        opacity      = defaults.double(forKey: Key.opacity.rawValue)
        speed        = defaults.double(forKey: Key.speed.rawValue)
        clickThrough = defaults.bool(forKey: Key.clickThrough.rawValue)
        lockPosition = defaults.bool(forKey: Key.lockPosition.rawValue)
        alwaysOnTop  = defaults.bool(forKey: Key.alwaysOnTop.rawValue)
        playing      = defaults.bool(forKey: Key.playing.rawValue)
    }

    // MARK: - Helpers
    func savePosition(_ point: NSPoint) {
        positionX = point.x
        positionY = point.y
    }

    func savedPosition() -> NSPoint {
        NSPoint(x: positionX, y: positionY)
    }
}
