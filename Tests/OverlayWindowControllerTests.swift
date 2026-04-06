import AppKit
import Foundation

@main
struct OverlayWindowControllerTests {
    static func main() throws {
        _ = NSApplication.shared

        let record = PetRecord.default(instanceID: "pet-1", index: 1)
        let settings = AppSettings(record: record)
        let controller = OverlayWindowController(settings: settings)
        let validURL = URL(fileURLWithPath: "evernight.mp4", relativeTo: URL(fileURLWithPath: FileManager.default.currentDirectoryPath))

        guard controller.loadAsset(url: validURL) else {
            throw TestFailure("Expected valid video asset to load")
        }

        let originalBookmark = settings.assetBookmark
        guard originalBookmark != nil else {
            throw TestFailure("Expected valid load to store a bookmark")
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
