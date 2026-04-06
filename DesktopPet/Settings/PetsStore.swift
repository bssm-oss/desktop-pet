// PetsStore.swift
// Stores all pets in a single Application Support plist and migrates legacy defaults.

import Foundation

struct PetRecord: Codable, Equatable {
    let instanceID: String
    var positionX: Double
    var positionY: Double
    var scale: Double
    var opacity: Double
    var speed: Double
    var clickThrough: Bool
    var lockPosition: Bool
    var alwaysOnTop: Bool
    var playing: Bool
    var label: String
    var flipHorizontal: Bool
    var flipVertical: Bool
    var assetBookmark: Data?

    static func `default`(instanceID: String, index: Int) -> PetRecord {
        let offset = Double((index - 1) * 60)
        return PetRecord(
            instanceID: instanceID,
            positionX: 200.0 + offset,
            positionY: 200.0 + offset,
            scale: 1.0,
            opacity: 1.0,
            speed: 1.0,
            clickThrough: false,
            lockPosition: false,
            alwaysOnTop: true,
            playing: true,
            label: "Pet \(index)",
            flipHorizontal: false,
            flipVertical: false,
            assetBookmark: nil
        )
    }
}

private struct PetsManifest: Codable {
    var schemaVersion: Int
    var nextPetIndex: Int
    var pets: [PetRecord]
}

final class PetsStore {

    // MARK: - Constants

    private enum Constants {
        static let schemaVersion = 1
        static let legacyInstanceIDsKey = "petInstanceIDs"
        static let directoryName = "desktop-pet"
        static let fileName = "pets.plist"
    }

    // MARK: - Dependencies

    private let fileManager: FileManager
    private let defaults: UserDefaults
    private let storeURL: URL

    // MARK: - State

    private var manifest = PetsManifest(schemaVersion: Constants.schemaVersion, nextPetIndex: 1, pets: [])

    // MARK: - Init

    init(
        fileManager: FileManager = .default,
        defaults: UserDefaults = .standard,
        storeURL: URL? = nil
    ) {
        self.fileManager = fileManager
        self.defaults = defaults
        self.storeURL = storeURL ?? PetsStore.defaultStoreURL(fileManager: fileManager)
    }

    // MARK: - Public

    var fileURL: URL { storeURL }

    @discardableResult
    func loadRecords() -> [PetRecord] {
        if fileManager.fileExists(atPath: storeURL.path) {
            manifest = loadManifestFromDisk() ?? migrateOrCreateEmptyManifest()
        } else {
            manifest = migrateOrCreateEmptyManifest()
            saveManifest()
        }

        return manifest.pets
    }

    func makeNewPetRecord() -> PetRecord {
        let index = manifest.nextPetIndex
        manifest.nextPetIndex += 1
        saveManifest()
        return .default(instanceID: "pet-\(index)", index: index)
    }

    func upsert(_ record: PetRecord) {
        if let index = manifest.pets.firstIndex(where: { $0.instanceID == record.instanceID }) {
            manifest.pets[index] = record
        } else {
            manifest.pets.append(record)
        }
        saveManifest()
    }

    func remove(instanceID: String) {
        manifest.pets.removeAll { $0.instanceID == instanceID }
        saveManifest()
    }

    // MARK: - Disk I/O

    private func loadManifestFromDisk() -> PetsManifest? {
        do {
            let data = try Data(contentsOf: storeURL)
            return try PropertyListDecoder().decode(PetsManifest.self, from: data)
        } catch {
            backupCorruptedStore()
            NSLog("PetsStore: failed to decode plist at %@: %@", storeURL.path, String(describing: error))
            return nil
        }
    }

    private func saveManifest() {
        do {
            try ensureStoreDirectoryExists()
            let data = try PropertyListEncoder().encode(manifest)
            try data.write(to: storeURL, options: .atomic)
        } catch {
            NSLog("PetsStore: failed to save plist at %@: %@", storeURL.path, String(describing: error))
        }
    }

    private func ensureStoreDirectoryExists() throws {
        let directoryURL = storeURL.deletingLastPathComponent()
        try fileManager.createDirectory(at: directoryURL, withIntermediateDirectories: true)
    }

    private func backupCorruptedStore() {
        guard fileManager.fileExists(atPath: storeURL.path) else { return }

        let timestamp = ISO8601DateFormatter().string(from: Date()).replacingOccurrences(of: ":", with: "-")
        let backupURL = storeURL.deletingPathExtension().appendingPathExtension("corrupted-\(timestamp).plist")

        do {
            if fileManager.fileExists(atPath: backupURL.path) {
                try fileManager.removeItem(at: backupURL)
            }
            try fileManager.moveItem(at: storeURL, to: backupURL)
        } catch {
            NSLog("PetsStore: failed to back up corrupted plist: %@", String(describing: error))
        }
    }

    // MARK: - Legacy Migration

    private func migrateOrCreateEmptyManifest() -> PetsManifest {
        let legacyIDs = defaults.array(forKey: Constants.legacyInstanceIDsKey) as? [String] ?? []
        guard !legacyIDs.isEmpty else {
            return PetsManifest(schemaVersion: Constants.schemaVersion, nextPetIndex: 1, pets: [])
        }

        let records = legacyIDs.map(makeLegacyRecord)
        let nextPetIndex = (legacyIDs
            .compactMap { Int($0.replacingOccurrences(of: "pet-", with: "")) }
            .max() ?? 0) + 1

        return PetsManifest(
            schemaVersion: Constants.schemaVersion,
            nextPetIndex: max(nextPetIndex, 1),
            pets: records
        )
    }

    private func makeLegacyRecord(instanceID: String) -> PetRecord {
        let legacyIndex = Int(instanceID.components(separatedBy: "-").last ?? "1") ?? 1
        let defaultsRecord = PetRecord.default(instanceID: instanceID, index: legacyIndex)
        func key(_ suffix: String) -> String { "\(instanceID).\(suffix)" }

        return PetRecord(
            instanceID: instanceID,
            positionX: value(for: key("positionX"), fallback: defaultsRecord.positionX),
            positionY: value(for: key("positionY"), fallback: defaultsRecord.positionY),
            scale: value(for: key("scale"), fallback: defaultsRecord.scale),
            opacity: value(for: key("opacity"), fallback: defaultsRecord.opacity),
            speed: value(for: key("speed"), fallback: defaultsRecord.speed),
            clickThrough: boolValue(for: key("clickThrough"), fallback: defaultsRecord.clickThrough),
            lockPosition: boolValue(for: key("lockPosition"), fallback: defaultsRecord.lockPosition),
            alwaysOnTop: boolValue(for: key("alwaysOnTop"), fallback: defaultsRecord.alwaysOnTop),
            playing: boolValue(for: key("playing"), fallback: defaultsRecord.playing),
            label: defaults.string(forKey: key("label")) ?? defaultsRecord.label,
            flipHorizontal: boolValue(for: key("flipH"), fallback: defaultsRecord.flipHorizontal),
            flipVertical: boolValue(for: key("flipV"), fallback: defaultsRecord.flipVertical),
            assetBookmark: defaults.data(forKey: key("assetBookmark"))
        )
    }

    private func value(for key: String, fallback: Double) -> Double {
        defaults.object(forKey: key) == nil ? fallback : defaults.double(forKey: key)
    }

    private func boolValue(for key: String, fallback: Bool) -> Bool {
        defaults.object(forKey: key) == nil ? fallback : defaults.bool(forKey: key)
    }

    // MARK: - Paths

    private static func defaultStoreURL(fileManager: FileManager) -> URL {
        let appSupportURL = fileManager.urls(for: .applicationSupportDirectory, in: .userDomainMask).first
            ?? URL(fileURLWithPath: NSTemporaryDirectory(), isDirectory: true)

        return appSupportURL
            .appendingPathComponent(Constants.directoryName, isDirectory: true)
            .appendingPathComponent(Constants.fileName)
    }
}
