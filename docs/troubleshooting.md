# Troubleshooting

## Transparency

**Problem:** White or black box appears behind the animation.  
**Cause:** The window or view background is not fully transparent.  
**Fix:** Ensure `window.isOpaque = false`, `window.backgroundColor = .clear`, and the view's layer `backgroundColor = CGColor.clear`. All three must be set.

**Problem:** GIF/APNG shows transparency but video does not.  
**Cause:** MP4 (H.264/H.265) has no alpha channel. This is a format limitation, not a bug.  
**Fix:** Use GIF, APNG, PNG sequence, ProRes 4444, or HEVC with Alpha for transparent overlays.

---

## Spaces / desktop switching

**Problem:** Overlay disappears when switching to another Space.  
**Fix:** Ensure `collectionBehavior` includes `.canJoinAllSpaces`. This must be set before the window is shown.

**Problem:** Overlay disappears when an app goes fullscreen.  
**Fix:** Add `.fullScreenAuxiliary` to `collectionBehavior`. Without this flag, macOS hides the window when any app enters fullscreen mode.

**Problem:** Overlay moves to a different position when switching Spaces.  
**Fix:** Add `.stationary` to `collectionBehavior`.

---

## Click-through

**Problem:** Click-through is enabled but clicks are still intercepted.  
**Fix:** Check both `window.ignoresMouseEvents = true` AND `PetView.hitTest` returning `nil`. Both are needed for reliable click-through.

**Problem:** Can't drag the overlay after enabling click-through.  
**Cause:** This is expected — click-through passes ALL mouse events through, including drag.  
**Fix:** Disable click-through to drag, then re-enable it.

---

## Always on top

**Problem:** Other windows appear above the overlay.  
**Fix:** Set `window.level = .floating`. If you need to appear above system UI elements, use `.screenSaver` level, but this may interfere with other apps.

**Problem:** Overlay appears above fullscreen apps incorrectly.  
**Cause:** `.fullScreenAuxiliary` behavior varies by macOS version.  
**Fix:** This is a known macOS limitation. The overlay will appear in the auxiliary space alongside fullscreen apps, not directly on top of them.

---

## Drag / lock

**Problem:** Can't drag the overlay.  
**Check:**
1. `lockPosition` is false in settings
2. `clickThrough` is false (click-through disables drag)
3. `window.canBecomeKey` returns `true` (it does in `OverlayWindow`)

---

## Start at Login

**Problem:** "Start at Login" toggle doesn't work.  
**Fix:** The app must be code-signed with a valid team. `SMAppService` requires a signed app. Free developer accounts work for local testing.

**Problem:** App doesn't start at login after enabling.  
**Fix:** Check System Settings → General → Login Items. The app should appear there. If not, the entitlement may be missing.

---

## Release installation

**Problem:** A GitHub release build is blocked by Gatekeeper with a malware verification warning.  
**Cause:** The downloaded app bundle or DMG is unsigned, improperly signed, or not notarized for public distribution.  
**Fix:** Rebuild the release through the notarized GitHub workflow. Do not treat `xattr -cr` as the product fix for published releases.

---

## Import failures

**Problem:** GIF loads but shows wrong colors or missing frames.  
**Cause:** GIF disposal method not handled correctly.  
**Fix:** `GIFDecoder` composites frames onto a canvas — if you see issues, check that the canvas size matches the GIF logical screen size, not the individual frame size.

**Problem:** PNG sequence doesn't load.  
**Fix:** The folder must contain only `.png` files. Files are sorted alphabetically — use zero-padded numbers (`frame_0001.png` not `frame_1.png`) to ensure correct order.

**Problem:** Video doesn't loop.  
**Fix:** `VideoPlayer` uses `AVPlayerItemDidPlayToEndTime` notification to seek to `.zero` and restart. Ensure the notification is registered before playback starts.

---

## Performance

**Problem:** High CPU usage.  
**Check:**
1. CVDisplayLink is being used (not NSTimer)
2. The frame-change guard is working (`lastFrameIndex` comparison)
3. No unnecessary `CATransaction` commits

**Problem:** Memory grows over time.  
**Cause:** Large GIF/APNG with many frames decoded into `[CGImage]`.  
**Fix:** For very large animations (>200 frames), consider lazy decoding. Current implementation decodes all frames at load time for simplicity and smooth playback.
