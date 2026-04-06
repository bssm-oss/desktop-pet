// AppSettings.swift
// Per-instance settings for one pet.
// Observable so SwiftUI views react to changes in real time.

import Foundation
import Combine
import AppKit

final class AppSettings: ObservableObject {

    // MARK: - Instance identity
    let instanceID: String   // e.g. "pet-1", "pet-2", …
    private var onChange: ((AppSettings) -> Void)?
    private var isInitializing = true

    // MARK: - Published Properties

    @Published var positionX: Double   { didSet { notifyChangeIfNeeded() } }
    @Published var positionY: Double   { didSet { notifyChangeIfNeeded() } }
    @Published var scale: Double       { didSet { notifyChangeIfNeeded() } }
    @Published var opacity: Double     { didSet { notifyChangeIfNeeded() } }
    @Published var speed: Double       { didSet { notifyChangeIfNeeded() } }
    @Published var clickThrough: Bool  { didSet { notifyChangeIfNeeded() } }
    @Published var lockPosition: Bool  { didSet { notifyChangeIfNeeded() } }
    @Published var alwaysOnTop: Bool   { didSet { notifyChangeIfNeeded() } }
    @Published var playing: Bool       { didSet { notifyChangeIfNeeded() } }
    @Published var label: String       { didSet { notifyChangeIfNeeded() } }
    @Published var flipHorizontal: Bool { didSet { notifyChangeIfNeeded() } }
    @Published var flipVertical: Bool   { didSet { notifyChangeIfNeeded() } }

    // Security-scoped bookmark for the last imported asset
    var assetBookmark: Data? {
        didSet { notifyChangeIfNeeded() }
    }

    // MARK: - Init

    init(record: PetRecord, onChange: ((AppSettings) -> Void)? = nil) {
        self.instanceID = record.instanceID
        self.positionX = record.positionX
        self.positionY = record.positionY
        self.scale = record.scale
        self.opacity = record.opacity
        self.speed = record.speed
        self.clickThrough = record.clickThrough
        self.lockPosition = record.lockPosition
        self.alwaysOnTop = record.alwaysOnTop
        self.playing = record.playing
        self.label = record.label
        self.flipHorizontal = record.flipHorizontal
        self.flipVertical = record.flipVertical
        self.assetBookmark = record.assetBookmark
        self.onChange = onChange
        self.isInitializing = false
    }

    // MARK: - Helpers

    func savePosition(_ point: NSPoint) {
        positionX = point.x
        positionY = point.y
    }

    func savedPosition() -> NSPoint {
        NSPoint(x: positionX, y: positionY)
    }

    func toRecord() -> PetRecord {
        PetRecord(
            instanceID: instanceID,
            positionX: positionX,
            positionY: positionY,
            scale: scale,
            opacity: opacity,
            speed: speed,
            clickThrough: clickThrough,
            lockPosition: lockPosition,
            alwaysOnTop: alwaysOnTop,
            playing: playing,
            label: label,
            flipHorizontal: flipHorizontal,
            flipVertical: flipVertical,
            assetBookmark: assetBookmark
        )
    }

    // MARK: - Persistence callback

    private func notifyChangeIfNeeded() {
        guard !isInitializing else { return }
        onChange?(self)
    }
}
