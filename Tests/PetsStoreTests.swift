// PetsStoreTests.swift
// Exercises the file-backed pet store and its legacy migration path.

import Foundation

@main
struct PetsStoreTests {
    static func main() throws {
        try testLegacyMigration()
        try testRoundTripPersistence()
        print("PetsStoreTests: PASS")
    }

    private static func testLegacyMigration() throws {
        let suiteName = "PetsStoreTests.legacy.\(UUID().uuidString)"
        guard let defaults = UserDefaults(suiteName: suiteName) else {
            throw TestFailure("Could not create legacy defaults suite")
        }

        let tempRoot = try makeTempDirectory(named: "legacy")
        defer {
            defaults.removePersistentDomain(forName: suiteName)
            try? FileManager.default.removeItem(at: tempRoot)
        }

        defaults.set(["pet-2"], forKey: "petInstanceIDs")
        defaults.set(321.0, forKey: "pet-2.positionX")
        defaults.set(654.0, forKey: "pet-2.positionY")
        defaults.set(1.5, forKey: "pet-2.scale")
        defaults.set(0.7, forKey: "pet-2.opacity")
        defaults.set(0.8, forKey: "pet-2.speed")
        defaults.set(true, forKey: "pet-2.clickThrough")
        defaults.set(true, forKey: "pet-2.lockPosition")
        defaults.set(false, forKey: "pet-2.alwaysOnTop")
        defaults.set(false, forKey: "pet-2.playing")
        defaults.set("Migrated Pet", forKey: "pet-2.label")
        defaults.set(true, forKey: "pet-2.flipH")
        defaults.set(false, forKey: "pet-2.flipV")
        defaults.set(Data([0x01, 0x02]), forKey: "pet-2.assetBookmark")

        let store = PetsStore(defaults: defaults, storeURL: tempRoot.appendingPathComponent("pets.plist"))
        let records = store.loadRecords()
        try expect(records.count == 1, "Expected one migrated pet")

        let record = try unwrap(records.first, "Expected migrated pet record")
        try expect(record.instanceID == "pet-2", "Expected instanceID to migrate")
        try expect(record.positionX == 321.0 && record.positionY == 654.0, "Expected migrated position")
        try expect(record.scale == 1.5 && record.opacity == 0.7, "Expected migrated appearance")
        try expect(record.speed == 0.8 && !record.alwaysOnTop && !record.playing, "Expected migrated playback flags")
        try expect(record.clickThrough && record.lockPosition && record.flipHorizontal && !record.flipVertical, "Expected migrated behavior flags")
        try expect(record.label == "Migrated Pet", "Expected migrated label")
        try expect(record.assetBookmark == Data([0x01, 0x02]), "Expected migrated bookmark")

        let nextRecord = store.makeNewPetRecord()
        try expect(nextRecord.instanceID == "pet-3", "Expected monotonic pet IDs after migration")
    }

    private static func testRoundTripPersistence() throws {
        let suiteName = "PetsStoreTests.roundtrip.\(UUID().uuidString)"
        guard let defaults = UserDefaults(suiteName: suiteName) else {
            throw TestFailure("Could not create round-trip defaults suite")
        }

        let tempRoot = try makeTempDirectory(named: "roundtrip")
        defer {
            defaults.removePersistentDomain(forName: suiteName)
            try? FileManager.default.removeItem(at: tempRoot)
        }

        let storeURL = tempRoot.appendingPathComponent("pets.plist")
        let store = PetsStore(defaults: defaults, storeURL: storeURL)
        _ = store.loadRecords()

        var record = store.makeNewPetRecord()
        record.label = "Round Trip"
        record.assetBookmark = Data([0x0A, 0x0B, 0x0C])
        store.upsert(record)

        let reloadedStore = PetsStore(defaults: defaults, storeURL: storeURL)
        let reloadedRecords = reloadedStore.loadRecords()
        try expect(reloadedRecords == [record], "Expected plist round-trip to preserve pet records")

        let nextRecord = reloadedStore.makeNewPetRecord()
        try expect(nextRecord.instanceID == "pet-2", "Expected next ID to persist across reload")

        reloadedStore.remove(instanceID: record.instanceID)
        let emptiedStore = PetsStore(defaults: defaults, storeURL: storeURL)
        try expect(emptiedStore.loadRecords().isEmpty, "Expected remove() to persist")
    }

    private static func makeTempDirectory(named name: String) throws -> URL {
        let url = FileManager.default.temporaryDirectory
            .appendingPathComponent("desktop-pet-tests-\(name)-\(UUID().uuidString)", isDirectory: true)
        try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        return url
    }

    private static func expect(_ condition: @autoclosure () -> Bool, _ message: String) throws {
        if !condition() {
            throw TestFailure(message)
        }
    }

    private static func unwrap<T>(_ value: T?, _ message: String) throws -> T {
        guard let value else { throw TestFailure(message) }
        return value
    }
}

private struct TestFailure: Error, CustomStringConvertible {
    let description: String

    init(_ description: String) {
        self.description = description
    }
}
