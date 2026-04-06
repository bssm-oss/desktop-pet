# Persistent pet store and multi-import

## Background

The macOS app kept pet state in per-key `UserDefaults` entries and only allowed one asset per Add Pet selection. Users needed imported pets to survive relaunches and system restarts with an explicit file-backed store, and they needed to import multiple pets in one action.

## Problem

- Persistence was spread across legacy `UserDefaults` keys instead of an explicit store file.
- Restored pets depended on security-scoped bookmarks but did not have a dedicated manifest for all pets.
- The top-level Add Pet flow only accepted one file or folder at a time.

## Changes

- Added `DesktopPet/Settings/PetsStore.swift` to persist all pet records in `~/Library/Application Support/desktop-pet/pets.plist`.
- Added one-time migration from the legacy `UserDefaults` pet records into the plist store without deleting the old defaults.
- Switched `AppSettings` persistence from direct `UserDefaults` writes to file-store callbacks.
- Changed the top-level Add Pet picker to allow multi-selection while keeping per-pet re-import single-select.
- Fixed bookmark-based restore flow to keep security-scoped access active while loading restored assets.
- Added `Tests/PetsStoreTests.swift` as a runnable regression harness for migration and plist round-trips.

## Design rationale

- A single plist is the smallest Apple-native file-backed store that cleanly supports bookmark `Data`, schema versioning, and atomic writes.
- The app still keeps one runtime pet per imported asset, which matches the existing window/controller architecture and minimizes UI changes.
- Legacy defaults are intentionally left in place for rollback safety during the first migrated release.

## Impact

- New imports now persist through an explicit Application Support store.
- Multiple files or folders can be imported in a single Add Pet action.
- Existing users are migrated on first launch after upgrade if they already have legacy pet defaults.

## Verification

- `swiftc ... DesktopPet/Settings/PetsStore.swift ...` macOS app compile
- `swiftc Tests/PetsStoreTests.swift DesktopPet/Settings/PetsStore.swift -o /tmp/... && /tmp/...`
- Additional app-level manual QA and release verification are tracked in the final report.

## Remaining limits

- The repo still has no Xcode/XCTest target; regression coverage is currently a standalone Swift test executable.
- Release automation still requires explicit version updates and a tag push for installer publication.

## Follow-up

- Add a formal test target when the project adopts a stable automated test harness.
- Consider surfacing unresolved bookmark failures in the UI if users need a relink flow later.
