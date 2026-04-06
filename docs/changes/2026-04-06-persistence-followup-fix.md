# Persistence follow-up fix

## Background

The initial 1.2.0 plist-store release shipped the requested file-backed persistence and multi-import flow, but it still allowed a failed asset load to overwrite the current persisted bookmark before the new asset had loaded successfully. Oracle review also flagged that the persistence regression harness was not wired into CI and that some contributor-facing docs still described the legacy UserDefaults model.

## Problem

- A failed import or re-import could corrupt the stored bookmark for the current pet.
- CI compiled the app but did not execute the new persistence regression harness.
- `AGENTS.md`, `CONTRIBUTING.md`, and `docs/architecture.md` still described the old persistence model.

## Changes

- Deferred `currentAssetURL` / `assetBookmark` persistence until after a candidate asset has been decoded or constructed successfully.
- Added an `OverlayWindowController` regression harness that proves a failed re-import does not overwrite the existing bookmark.
- Wired both standalone regression harnesses into `.github/workflows/build-check.yml`.
- Updated contributor/architecture docs to describe the plist-backed persistence model.
- Bumped the corrective release version to `1.2.1`.

## Design rationale

- Persistence must only commit after success, otherwise restart reliability is still broken even with a file-backed store.
- The build check should execute the critical regressions that protect persistence, not just compile the app.
- A patch release is safer than mutating the already-published `v1.2.0` release.

## Impact

- Failed imports no longer poison saved pet state.
- CI now exercises both the store migration path and the failed-import safety path.
- The published `v1.2.1` release now carries both the persistence fix and the repeatable verification chain.

## Verification

- `swiftc ... DesktopPet/Window/OverlayWindowController.swift ...` macOS app compile
- `swiftc Tests/PetsStoreTests.swift DesktopPet/Settings/PetsStore.swift ... && /tmp/...`
- `swiftc Tests/OverlayWindowControllerTests.swift ... DesktopPet/Window/OverlayWindowController.swift ... && /tmp/...`

## Remaining limits

- The repo still uses standalone Swift executables instead of a first-class XCTest target.

## Follow-up

- Replace the standalone Swift regression harnesses with a first-class test target when the project adopts one.
