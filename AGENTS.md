# AGENTS.md — AI Agent Guidelines for Desktop Pet

This file defines conventions and rules for AI agents (Kiro, Codex, Copilot, etc.)
working on this repository.

---

## Project identity

- **Name:** Desktop Pet
- **Language:** Swift 5.9+
- **Platform:** macOS 14+, Apple Silicon primary
- **Architecture:** Native AppKit + SwiftUI, no cross-platform abstractions
- **Bundle ID:** `com.bssm-oss.desktop-pet`

---

## Core rules

### 1. Stay native macOS
Do NOT introduce cross-platform frameworks (Electron, Flutter, Tauri, Qt).
Do NOT add Rust unless there is a concrete, measurable performance reason.
AppKit + SwiftUI is the correct stack for this app.

### 2. No third-party dependencies
All functionality must use Apple frameworks only:
- ImageIO for GIF/APNG/PNG decode
- AVFoundation for video
- CoreAnimation / CALayer for rendering
- CVDisplayLink for frame scheduling
- PropertyListEncoder / PropertyListDecoder for the file-backed pet store in Application Support
- SMAppService for login items

Do NOT add Swift Package Manager dependencies without explicit approval.

### 3. Performance first
- Frame scheduling: always CVDisplayLink, never NSTimer for animation
- Rendering: always CALayer.contents = CGImage, never NSImageView.image per frame
- Decode: always at load time, never per-frame at runtime
- Guard against redundant CALayer updates (lastFrameIndex check)

### 4. Transparency is non-negotiable
The overlay window must always be:
```swift
window.isOpaque = false
window.backgroundColor = .clear
window.hasShadow = false
```
Never set a background color on the window or content view.

### 5. Spaces behavior is non-negotiable
The window must always have:
```swift
window.collectionBehavior = [
    .canJoinAllSpaces,
    .fullScreenAuxiliary,
    .stationary
]
```
Never remove `.fullScreenAuxiliary` — it is what keeps the overlay visible when
another app goes fullscreen.

### 6. Menubar-only app
`LSUIElement = YES` in Info.plist must never be removed.
The app must never show a dock icon.
`NSApp.setActivationPolicy(.accessory)` must be called in `applicationDidFinishLaunching`.

---

## File structure

```
DesktopPet/
├── App/            # AppDelegate, Info.plist
├── Window/         # OverlayWindow, PetView, OverlayWindowController
├── Playback/       # AnimationPlayer, decoders, FrameSequence, VideoPlayer
├── MenuBar/        # MenuBarController
├── Settings/       # AppSettings, SettingsView
└── Utilities/      # PlaceholderAnimation, SecurityScopedAccess
```

When adding new files, place them in the correct group.
Update `DesktopPet.xcodeproj/project.pbxproj` to include new source files.

---

## Coding conventions

- Swift 5.9 concurrency where appropriate (`async/await` for decode operations)
- `final class` for all controllers and players (no subclassing intended)
- `// MARK: -` sections for readability
- Every file starts with a comment block explaining its role
- No force-unwraps (`!`) except where nil is genuinely impossible and documented
- `weak var delegate` for all delegate references (no retain cycles)
- `@MainActor` on any function that touches UIKit/AppKit

---

## What agents should NOT do

- Do not add `@NSApplicationMain` or `@main` to any file other than `AppDelegate.swift`
- Do not change `window.level` to `.screenSaver` or higher without documenting why
- Do not decode animation frames on the main thread (use background queue + main dispatch)
- Do not call `CVDisplayLinkStart` without a corresponding `CVDisplayLinkStop` in deinit
- Do not store strong references to `NSWindow` in delegates (use `weak`)
- Do not add `NSApp.activate(ignoringOtherApps: true)` in the main render loop

---

## Testing guidance

- `evernight.mp4` — use for video import/loop/positioning tests only, NOT transparency tests
- For transparency tests: use a GIF or APNG with known alpha regions
- For PNG sequence tests: create a folder with `frame_0001.png` through `frame_0010.png`
- Verify Spaces behavior by: opening the app, switching to a different Space, confirming overlay is still visible
- Verify fullscreen behavior by: making Safari fullscreen, confirming overlay appears in auxiliary space

---

## Commit message format

```
type(scope): short description

Types: feat, fix, perf, refactor, docs, chore
Scopes: window, playback, menubar, settings, decoder, build

Examples:
feat(playback): add HEVC with Alpha support
fix(window): restore fullScreenAuxiliary behavior after Space switch
perf(decoder): lazy-decode PNG sequences over 100 frames
docs: add transparent video format comparison
```
