# Supported Formats

## First-class (transparent overlay)

### GIF
- Full alpha transparency support
- Per-frame delays (1/100s units, minimum 20ms enforced)
- Disposal methods: background, previous, keep
- Decoded via ImageIO at load time — no per-frame CPU work at runtime
- Limitation: 256-color palette per frame (GIF spec)

### APNG (Animated PNG)
- Full RGBA transparency
- Native macOS 14 support via ImageIO — no third-party library needed
- Per-frame delay via numerator/denominator (accurate sub-frame timing)
- Falls back to static PNG if not animated
- Best format for high-quality transparent animations

### PNG Sequence (folder)
- Full RGBA transparency
- Any resolution, any frame count
- Sorted alphabetically — use zero-padded filenames:
  ```
  frame_0001.png
  frame_0002.png
  frame_0003.png
  ```
- Default frame rate: 24fps (configurable in code)
- Best for: exporting from animation tools like After Effects, Blender, etc.

---

## Secondary (no transparency)

### MP4 / MOV (H.264 / H.265)
- ⚠️ **No alpha channel** — renders with solid background
- Hardware decoded on Apple Silicon (Media Engine)
- Smooth playback, low power
- Use for: testing import/loop/positioning behavior
- Do NOT use for transparent overlays

### ProRes 4444 (.mov)
- ✅ **Full alpha transparency**
- Hardware decoded on Apple Silicon
- Large file size (lossless quality)
- Export from: After Effects, Final Cut Pro, DaVinci Resolve
- Best for: high-quality transparent video overlays

### HEVC with Alpha (.mov, hvc1)
- ✅ **Full alpha transparency**
- macOS 13+ required
- Much smaller than ProRes 4444
- Export from: Final Cut Pro 10.6+
- Best for: production-quality transparent video with small file size

---

## Format comparison

| Format | Transparency | File Size | Quality | Decode |
|--------|-------------|-----------|---------|--------|
| GIF | ✅ (1-bit) | Small | Low (256 colors) | CPU (ImageIO) |
| APNG | ✅ (full) | Medium | High | CPU (ImageIO) |
| PNG sequence | ✅ (full) | Large | Lossless | CPU (ImageIO) |
| MP4 H.264 | ❌ | Small | Good | Hardware |
| ProRes 4444 | ✅ (full) | Very large | Lossless | Hardware |
| HEVC+Alpha | ✅ (full) | Small | Very good | Hardware |

---

## Recommendation

For a dancing desktop character:
1. **APNG** — best balance of quality, transparency, and file size
2. **PNG sequence** — easiest to export from animation tools
3. **GIF** — widest compatibility, lower quality
4. **HEVC with Alpha** — best for video-based characters (Final Cut Pro required)
