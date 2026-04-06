# Contributing to Desktop Pet

Desktop Pet is an open-source project and contributions are welcome.  
Please read this document before opening an issue or submitting a pull request.

---

## Table of Contents

1. [Code of Conduct](#code-of-conduct)
2. [Getting Started](#getting-started)
3. [How to Contribute](#how-to-contribute)
4. [Branch Naming](#branch-naming)
5. [Commit Message Format](#commit-message-format)
6. [Pull Request Rules](#pull-request-rules)
7. [Code Style](#code-style)
8. [What NOT to Do](#what-not-to-do)

---

## Code of Conduct

- Be respectful and constructive in all discussions.
- Issues and PRs are in English or Korean — both are fine.

---

## Getting Started

### Requirements

| Tool | Version |
|------|---------|
| macOS | 14.0 (Sonoma) or later |
| Xcode Command Line Tools | 15.0 or later |
| Swift | 5.9 or later |
| Platform | Apple Silicon (M1/M2/M3/M4) |

### Build from source

```bash
git clone https://github.com/bssm-oss/desktop-pet.git
cd desktop-pet

SDK=$(xcrun --sdk macosx --show-sdk-path)
mkdir -p build/DesktopPet.app/Contents/{MacOS,Resources}

swiftc \
  -sdk "$SDK" -target arm64-apple-macosx14.0 -O \
  -framework AppKit -framework AVFoundation \
  -framework CoreGraphics -framework ServiceManagement \
  DesktopPet/App/main.swift \
  DesktopPet/App/AppDelegate.swift \
  DesktopPet/Settings/AppSettings.swift \
  DesktopPet/Settings/PetsStore.swift \
  DesktopPet/Settings/SettingsView.swift \
  DesktopPet/Playback/FrameSequence.swift \
  DesktopPet/Playback/GIFDecoder.swift \
  DesktopPet/Playback/APNGDecoder.swift \
  DesktopPet/Playback/PNGSequenceDecoder.swift \
  DesktopPet/Playback/AnimationPlayer.swift \
  DesktopPet/Playback/VideoPlayer.swift \
  DesktopPet/Utilities/PlaceholderAnimation.swift \
  DesktopPet/Utilities/SecurityScopedAccess.swift \
  DesktopPet/Window/OverlayWindow.swift \
  DesktopPet/Window/PetView.swift \
  DesktopPet/Window/OverlayWindowController.swift \
  DesktopPet/MenuBar/MenuBarController.swift \
  -o build/DesktopPet.app/Contents/MacOS/DesktopPet

cp DesktopPet/App/Info.plist build/DesktopPet.app/Contents/Info.plist
open build/DesktopPet.app
```

Zero warnings = the bar. A PR that introduces new warnings will not be merged.

---

## How to Contribute

1. **Check existing issues** before opening a new one.
2. **Open an issue first** for non-trivial changes — discuss the approach before writing code.
3. **Fork** the repository and create a branch from `master`.
4. **Write your changes** following the code style below.
5. **Verify it compiles** with zero errors and zero warnings using `swiftc` as shown above.
6. **Open a Pull Request** against `master`.

---

## Branch Naming

```
<type>/<short-description>
```

| Type | When to use |
|------|-------------|
| `feat/` | New feature |
| `fix/` | Bug fix |
| `perf/` | Performance improvement |
| `refactor/` | Code restructuring without behavior change |
| `docs/` | Documentation only |
| `chore/` | Build scripts, CI, release tooling |

Examples:
```
feat/hevc-alpha-support
fix/position-restore-on-relaunch
docs/add-contributing-guide
chore/update-cask-sha256
```

---

## Commit Message Format

```
type(scope): short description (max 72 chars)

Optional longer body explaining the WHY, not the what.
Wrap at 72 characters.
```

### Types

| Type | Description |
|------|-------------|
| `feat` | New feature |
| `fix` | Bug fix |
| `perf` | Performance improvement |
| `refactor` | Restructuring without behavior change |
| `docs` | Documentation |
| `chore` | Build, CI, release |

### Scopes

| Scope | Files |
|-------|-------|
| `app` | App/, main.swift, AppDelegate |
| `window` | Window/ |
| `playback` | Playback/ |
| `menubar` | MenuBar/ |
| `settings` | Settings/ |
| `decoder` | GIFDecoder, APNGDecoder, PNGSequenceDecoder |
| `build` | CI, release workflow, Makefile |
| `cask` | Casks/desktop-pet.rb |

Examples:
```
feat(playback): add HEVC with Alpha support
fix(window): restore position correctly on relaunch
perf(decoder): lazy-decode PNG sequences over 100 frames
docs: add CONTRIBUTING guide and PR rules
chore(cask): bump version to 1.1.0 and update SHA256
```

---

## Pull Request Rules

### Title

Use the same format as commit messages:
```
fix(window): restore position correctly on relaunch
```

### Description template

```markdown
## What

Short summary of what changed.

## Why

Why this change is needed. Link to the related issue if applicable.
Closes #<issue-number>

## How

Key implementation decisions, trade-offs considered.

## Testing

How you verified the fix/feature works. Be specific:
- "Relaunched app 5 times, window always restored to saved position"
- "Tested with 10 different GIFs including sub-rect frames"
```

### Checklist before requesting review

- [ ] Compiles with **zero errors, zero warnings** (`swiftc` command above)
- [ ] Tested on Apple Silicon Mac running macOS 14+
- [ ] No third-party Swift Package Manager dependencies added
- [ ] No `@NSApplicationMain` or `@main` added outside `AppDelegate.swift`
- [ ] No `NSTimer` used for animation (use `CVDisplayLink`)
- [ ] No `NSImageView.image =` per-frame (use `CALayer.contents =`)
- [ ] Window transparency invariants maintained (`isOpaque=false`, `backgroundColor=.clear`, `hasShadow=false`)
- [ ] Spaces behavior invariants maintained (`collectionBehavior` includes `.fullScreenAuxiliary`)
- [ ] Commit messages follow the format above
- [ ] One logical change per commit (no "fix stuff" mega-commits)

### Review process

- PRs need **1 approving review** from a maintainer before merge.
- Maintainers may request changes — address them with new commits, not force-push.
- Squash-merge is used for all PRs to keep `master` history clean.

---

## Code Style

Follow what is already in the codebase. Key points:

- `final class` for all controllers and players — no subclassing intended
- `// MARK: -` sections for readability in every file
- Every file starts with a comment block explaining its role
- `@MainActor` on any function that touches AppKit/SwiftUI
- `weak var delegate` for all delegate references — no retain cycles
- No force-unwraps (`!`) unless nil is genuinely impossible, and document why
- `async/await` for decode operations that run off the main thread
- Persist pet state through the shared plist store in Application Support; only use namespaced UserDefaults keys for legacy migration or unrelated app defaults

---

## What NOT to Do

These will cause a PR to be rejected immediately:

| Don't | Why |
|-------|-----|
| Add Electron, Flutter, Tauri, Qt | Goes against the native macOS design |
| Add Swift Package Manager dependencies | Zero third-party deps is a design goal |
| Use `NSTimer` for animation | CVDisplayLink only — see AGENTS.md |
| Set `window.level = .screenSaver` or higher | Breaks expected z-order |
| Decode frames on the main thread | Causes jank |
| Remove `.fullScreenAuxiliary` from `collectionBehavior` | Breaks fullscreen Spaces |
| Set `window.backgroundColor` to non-clear | Breaks transparency |
| Add `NSApp.activate(ignoringOtherApps:)` in render loop | Steals focus every frame |
| Remove `LSUIElement = YES` from Info.plist | Shows unwanted dock icon |

---

## Questions?

Open a [GitHub Discussion](https://github.com/bssm-oss/desktop-pet/discussions) or an issue tagged `question`.
