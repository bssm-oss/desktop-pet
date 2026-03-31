// SecurityScopedAccess.swift
// Manages security-scoped bookmarks so the app can re-open user-selected files
// across app restarts without showing the file picker again.
// Required for sandboxed apps with com.apple.security.files.user-selected.read-only.

import Foundation

enum SecurityScopedAccess {

    /// Create a persistent bookmark for a URL the user selected via NSOpenPanel.
    /// Store the returned Data in AppSettings.assetBookmark.
    static func bookmark(for url: URL) -> Data? {
        try? url.bookmarkData(
            options: .withSecurityScope,
            includingResourceValuesForKeys: nil,
            relativeTo: nil
        )
    }

    /// Resolve a previously stored bookmark back to a URL.
    /// Returns nil if the bookmark is stale or the file was moved/deleted.
    /// Call `stopAccessingSecurityScopedResource()` on the URL when done.
    static func resolve(bookmark: Data) -> URL? {
        var isStale = false
        guard let url = try? URL(
            resolvingBookmarkData: bookmark,
            options: .withSecurityScope,
            relativeTo: nil,
            bookmarkDataIsStale: &isStale
        ), !isStale else { return nil }
        return url
    }
}
