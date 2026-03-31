# Performance & Power Optimization

## Design goals

- CPU usage at steady state: < 1% on M-series chips
- No unnecessary redraws
- No busy-wait timers
- Hardware-accelerated decode and render
- Minimal memory footprint

---

## Frame scheduling: CVDisplayLink

`CVDisplayLink` is the correct tool for display-sync animation on macOS.

**Why not NSTimer?**
- `NSTimer` fires on the run loop — it can be delayed by other work
- `NSTimer` at 60fps drifts over time
- `NSTimer` fires even when the display hasn't refreshed

**CVDisplayLink advantages:**
- Fires in sync with the actual display hardware refresh
- The OS sleeps the thread between callbacks (no busy-wait)
- ProMotion-aware: automatically uses 120Hz on MacBook Pro
- Never fires faster than the display can show

**Frame-change guard:**
```swift
guard newIndex != lastFrameIndex else { return }
```
This single check prevents CALayer from being updated on frames where the animation hasn't advanced. For a 24fps GIF on a 120Hz display, 5 out of 6 CVDisplayLink callbacks are skipped entirely.

---

## Rendering: CALayer.contents

```swift
imageLayer.contents = cgImage
```

On Apple Silicon, this is a **zero-copy GPU upload**.  
The CGImage is backed by an IOSurface. Setting it as `CALayer.contents` hands the IOSurface reference to the GPU compositor (WindowServer). No pixel data crosses the CPU-GPU boundary.

Compare to the naive approach:
```swift
// BAD: copies pixels, creates NSImage, triggers layout
imageView.image = NSImage(cgImage: frame, size: .zero)
```

**CATransaction with disabled animations:**
```swift
CATransaction.begin()
CATransaction.setDisableActions(true)
imageLayer.contents = frame
CATransaction.commit()
```
Without `setDisableActions(true)`, CALayer would animate the transition between frames (cross-fade), wasting GPU work.

---

## Decode strategy

All frames are decoded **once at load time** into a `[CGImage]` array.

**Tradeoff:**
- Pro: zero decode work at runtime, perfectly smooth playback
- Con: memory proportional to frame count × frame size

For typical desktop pet animations (< 100 frames, < 512×512):
- Memory: ~100 frames × 512×512 × 4 bytes = ~100MB worst case
- In practice: GIFs are usually < 30 frames, much smaller

For very large animations (200+ frames, high resolution), lazy decoding could be added — decode the next N frames ahead of the current position. Not implemented in v1.0 for simplicity.

---

## Video: AVFoundation hardware decode

`AVPlayerLayer` uses the Apple Silicon **Media Engine** for H.264/H.265 decode.  
This is dedicated silicon — it does not use CPU or GPU cores.  
Power consumption for video playback is extremely low.

---

## What runs when paused

When `playing = false`:
- CVDisplayLink is stopped (`CVDisplayLinkStop`)
- No callbacks fire
- No CALayer updates
- CPU usage: effectively 0%

The only active component is the `NSStatusItem` (menubar icon), which is handled entirely by the OS.

---

## Memory

| Asset type | Memory estimate |
|-----------|----------------|
| 30-frame GIF, 200×200 | ~5MB |
| 60-frame APNG, 400×400 | ~38MB |
| 100-frame PNG seq, 512×512 | ~100MB |
| MP4 video | ~2MB (AVFoundation buffers) |

GIF/APNG/PNG frames are held in memory for the lifetime of the app.  
Switching assets releases the previous `FrameSequence` immediately (ARC).

---

## Profiling

To measure CPU/GPU usage:
1. Open Instruments (`⌘I` in Xcode)
2. Choose "Time Profiler" for CPU
3. Choose "Metal System Trace" for GPU
4. Run the app and import an animation

Expected: CVDisplayLink callback takes < 0.1ms per frame. CALayer commit takes < 0.05ms.
