# Architecture

## Overview

Desktop Pet is a native macOS menubar app built with Swift + AppKit.  
It displays a transparent floating animation overlay that persists across all Spaces.

---

## Stack decision

### Why Swift + AppKit (not Rust)

This app's runtime behavior is:
1. Hold one transparent `NSWindow` open
2. Advance an animation frame on a timer
3. Push a `CGImage` to a `CALayer`
4. Respond to drag, menu, and file drop events

The CPU-intensive work (frame decoding) happens **once at load time**.  
At steady state the app is mostly sleeping between display refreshes.

Every Rust macOS crate (`cocoa`, `objc2`) is a thin wrapper around the same Objective-C runtime calls Swift makes natively — but without type safety, without Apple documentation, and with a lag behind SDK changes. Using Rust here adds FFI complexity with zero performance benefit, because the actual work (window compositing, GPU rendering, event dispatch) happens in Apple's frameworks regardless of which language calls them.

**Swift + AppKit is the correct choice for this app.**

---

## Component map

```
AppDelegate
    │
    ├── AppSettings (UserDefaults, @Published, Combine)
    │
    ├── OverlayWindowController
    │       │
    │       ├── OverlayWindow (NSWindow subclass)
    │       │       • Borderless, transparent
    │       │       • NSWindowLevel.floating
    │       │       • canJoinAllSpaces + fullScreenAuxiliary
    │       │
    │       ├── PetView (NSView)
    │       │       • CALayer.contents = CGImage  ← GPU composited
    │       │       • Mouse drag handling
    │       │       • File drop target
    │       │
    │       ├── AnimationPlayer
    │       │       • CVDisplayLink (display-sync, ProMotion-aware)
    │       │       • FrameSequence (decoded frames + delays)
    │       │       • Only updates layer when frame index changes
    │       │
    │       └── VideoPlayer (optional, for MP4/MOV)
    │               • AVPlayerLayer (hardware decode)
    │               • Looping via AVPlayerItemDidPlayToEndTime
    │
    └── MenuBarController
            • NSStatusItem (🐾)
            • Left click → SwiftUI settings panel (NSHostingController)
            • Right click → NSMenu quick toggles
```

---

## Windowing

### Transparency
```swift
window.isOpaque = false
window.backgroundColor = .clear
window.hasShadow = false
```
The window compositor (WindowServer) handles blending. No CPU work involved.

### Spaces behavior
```swift
window.collectionBehavior = [
    .canJoinAllSpaces,      // visible on every Space
    .fullScreenAuxiliary,   // stays when another app goes fullscreen
    .stationary             // doesn't move when switching Spaces
]
```
`.fullScreenAuxiliary` is the critical flag. Without it, the window disappears when any app enters fullscreen.

### Always on top
```swift
window.level = .floating   // NSWindowLevel(3) — above normal windows
```

### Click-through
```swift
window.ignoresMouseEvents = true
```
When enabled, all mouse events pass through to windows below. The `PetView.hitTest` override also returns `nil` for belt-and-suspenders correctness.

---

## Rendering pipeline

```
CVDisplayLink callback (display thread)
    │
    ├── Calculate delta time
    ├── Advance playbackTime by delta × speed
    ├── Compute frameIndex from FrameSequence.frameIndex(at:)
    ├── Guard: skip if frameIndex == lastFrameIndex  ← key optimization
    │
    └── DispatchQueue.main.async
            │
            └── CATransaction (animations disabled)
                    └── imageLayer.contents = cgImage  ← GPU upload
```

`CALayer.contents = cgImage` on Apple Silicon is a zero-copy operation — the GPU reads directly from the IOSurface backing the CGImage. No pixel data crosses the CPU-GPU boundary.

---

## Frame scheduling

**CVDisplayLink** is used instead of `NSTimer` or `DispatchSourceTimer` because:

- Fires in sync with the actual display refresh (60Hz or 120Hz ProMotion)
- The OS sleeps the thread between callbacks — no busy-wait
- Never fires faster than the display can show frames
- Automatically adapts to ProMotion displays on MacBook Pro

A `NSTimer` at 60fps would drift, fire at wrong times, and waste CPU on frames the display hasn't refreshed for yet.

---

## Asset decoding

### GIF
`CGImageSourceCreateWithURL` + per-frame `CGImageSourceCreateImageAtIndex`.  
Frames are decoded once at load time into a `[CGImage]` array.  
The GIF disposal method (background/previous/keep) is handled by compositing onto a persistent `CGContext` canvas.

### APNG
Same ImageIO path. macOS 14 has native APNG support — no third-party library needed.  
`kCGImagePropertyAPNGDelayTimeNumerator` / `Denominator` for accurate timing.

### PNG sequence
Sorted directory listing → `CGImageSourceCreateWithURL` per file.  
Frame rate defaults to 24fps; can be overridden.

### Video (MP4/MOV)
`AVPlayerLayer` — hardware decode on Apple Silicon Media Engine.  
**No alpha support for H.264/H.265.** For transparent video use ProRes 4444 or HEVC with Alpha.

---

## Persistence

Pet metadata is persisted in `~/Library/Application Support/desktop-pet/pets.plist` via `PetsStore`.  
Each record stores the per-pet settings plus the security-scoped bookmark for its imported asset.  
Legacy `UserDefaults` pet keys are migrated once into the plist store and then left untouched for rollback safety.

---

## Power efficiency

| Technique | Benefit |
|-----------|---------|
| CVDisplayLink | Sleeps between frames, no busy-wait |
| Frame-change guard | CALayer not touched on unchanged frames |
| CALayer.contents | Zero-copy GPU upload |
| ImageIO decode at load | No per-frame decode work at runtime |
| AVFoundation hardware decode | Media Engine, not CPU |
| No background timers | Nothing runs when paused |
