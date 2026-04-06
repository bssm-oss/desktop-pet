import AppKit
import Foundation

@main
struct OverlayWindowControllerTests {
    static func main() throws {
        _ = NSApplication.shared

        var record = PetRecord.default(instanceID: "pet-1", index: 1)
        record.assetBookmark = Data([0xAA, 0xBB, 0xCC])
        let settings = AppSettings(record: record)
        let controller = OverlayWindowController(settings: settings)

        let originalBookmark = settings.assetBookmark
        guard originalBookmark != nil else {
            throw TestFailure("Expected fixture bookmark to exist before invalid load")
        }

        let invalidURL = FileManager.default.temporaryDirectory.appendingPathComponent("desktop-pet-invalid.txt")
        try "not an animation".write(to: invalidURL, atomically: true, encoding: .utf8)
        defer { try? FileManager.default.removeItem(at: invalidURL) }

        if controller.loadAsset(url: invalidURL) {
            throw TestFailure("Expected invalid asset load to fail")
        }

        if settings.assetBookmark != originalBookmark {
            throw TestFailure("Failed load should not overwrite the existing bookmark")
        }

        print("OverlayWindowControllerTests: PASS")
    }
}

private struct TestFailure: Error, CustomStringConvertible {
    let description: String

    init(_ description: String) {
        self.description = description
    }
}
