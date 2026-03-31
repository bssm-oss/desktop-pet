# 🐾 Desktop Pet

A lightweight, native macOS desktop overlay app for Apple Silicon Macs.  
Import a GIF, APNG, PNG sequence, or video — it floats on your screen, loops forever, and stays visible across all Spaces.

![macOS 14+](https://img.shields.io/badge/macOS-14%2B-blue)
![Apple Silicon](https://img.shields.io/badge/Apple%20Silicon-Optimized-green)
![Swift](https://img.shields.io/badge/Swift-5.9%2B-orange)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

---

## What it does

- Displays any animation or video as a **transparent floating overlay** above your desktop
- Stays visible when you **switch Spaces or use fullscreen apps**
- Lives in the **menubar** — no dock icon, no clutter
- Extremely **lightweight**: CVDisplayLink scheduling, GPU-composited CALayer rendering, hardware-decoded media
- Remembers your last position, scale, opacity, speed, and asset across restarts

---

## Supported formats

| Format | Transparency | Notes |
|--------|-------------|-------|
| GIF | ✅ | Full alpha, per-frame disposal |
| APNG | ✅ | Native macOS 14 ImageIO support |
| PNG sequence (folder) | ✅ | Sorted alphabetically, 24fps default |
| MP4 / MOV (H.264/H.265) | ❌ | No alpha — renders with background |
| ProRes 4444 (.mov) | ✅ | Transparent video, hardware decoded |
| HEVC with Alpha (.mov) | ✅ | macOS 13+, smaller than ProRes |

> **Note:** Standard MP4 files (including `evernight.mp4`) do not support alpha transparency. They will display with a solid background. For a true transparent overlay, use GIF, APNG, PNG sequence, or ProRes 4444.

---

## Quick start

### Requirements
- macOS 14.0+
- Apple Silicon Mac (M1/M2/M3/M4)
- Xcode 15+

### Build & Run

```bash
git clone https://github.com/bssm-oss/desktop-pet.git
cd desktop-pet
open DesktopPet.xcodeproj
```

In Xcode:
1. Select the `DesktopPet` scheme
2. Set your Development Team in Signing & Capabilities
3. Press `⌘R` to build and run

The app launches as a menubar icon (🐾). No dock icon appears.

---

## Usage

### Import an animation

**Option 1 — Drag and drop:**  
Drag any supported file directly onto the floating overlay window.

**Option 2 — File picker:**  
Click the 🐾 menubar icon → "Import Animation…"

**Option 3 — PNG sequence:**  
Select a folder containing `frame_0001.png`, `frame_0002.png`, etc.

### Controls

| Action | How |
|--------|-----|
| Move overlay | Click and drag the character |
| Open settings | Left-click 🐾 |
| Quick toggles | Right-click 🐾 |
| Play / Pause | Settings panel or right-click menu |
| Opacity | Settings panel slider |
| Scale | Settings panel slider |
| Speed | Settings panel slider |
| Click-through | Toggle in settings — clicks pass through |
| Lock position | Toggle in settings — prevents accidental moves |
| Always on top | Toggle in settings |
| Start at Login | Settings panel toggle |

---

## Project structure

```
DesktopPet/
├── App/
│   ├── AppDelegate.swift          # Entry point, app lifecycle
│   └── Info.plist                 # LSUIElement=YES, no dock icon
├── Window/
│   ├── OverlayWindow.swift        # Transparent NSWindow subclass
│   ├── PetView.swift              # CALayer render view + drag + drop
│   └── OverlayWindowController.swift  # Coordinates window + playback
├── Playback/
│   ├── AnimationPlayer.swift      # CVDisplayLink frame scheduler
│   ├── FrameSequence.swift        # Decoded frames + timing
│   ├── GIFDecoder.swift           # ImageIO GIF decode
│   ├── APNGDecoder.swift          # ImageIO APNG decode
│   ├── PNGSequenceDecoder.swift   # Folder of PNGs → animation
│   └── VideoPlayer.swift          # AVFoundation MP4/MOV
├── MenuBar/
│   └── MenuBarController.swift    # NSStatusItem + menus + settings panel
├── Settings/
│   ├── AppSettings.swift          # UserDefaults-backed observable state
│   └── SettingsView.swift         # SwiftUI settings panel
└── Utilities/
    ├── PlaceholderAnimation.swift # Built-in fallback animation
    └── SecurityScopedAccess.swift # Bookmark persistence for sandboxed file access
```

---

## Architecture

See [docs/architecture.md](docs/architecture.md) for the full technical design.

---

## Performance

- **CVDisplayLink** — display-sync scheduling, ProMotion-aware, sleeps between frames
- **CALayer.contents** — zero-copy GPU upload, no CPU pixel blitting
- **ImageIO** — hardware-backed decode on Apple Silicon
- **AVFoundation** — hardware video decode via Neural Engine / Media Engine
- **Frame-change detection** — CALayer only updated when frame index changes
- **No unnecessary redraws** — compositor not disturbed on static frames

Typical CPU usage at steady state: **< 1%** on M-series chips.

---

## License

MIT — see [LICENSE](LICENSE)
