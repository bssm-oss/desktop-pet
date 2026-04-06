# Build & Run

## 릴리즈 DMG로 설치 (일반 사용자)

1. [Releases 페이지](https://github.com/bssm-oss/desktop-pet/releases)에서 `DesktopPet.dmg` 다운로드
2. DMG 열기 → `DesktopPet.app`을 `/Applications` 폴더로 드래그
3. `DesktopPet.app` 실행
4. 현재 공개 릴리즈가 아직 unsigned 상태라면 `xattr -cr /Applications/DesktopPet.app && open /Applications/DesktopPet.app` 으로 quarantine를 해제해야 할 수 있습니다.

## Homebrew 설치 경로

`Casks/desktop-pet.rb` 는 `postflight`에서 `/usr/bin/xattr -cr /Applications/DesktopPet.app` 를 자동 시도합니다. 그래도 Gatekeeper 경고가 남으면 `xattr -cr /Applications/DesktopPet.app && open /Applications/DesktopPet.app` 를 수동으로 실행합니다.

---

## Requirements

- macOS 14.0 or later
- Apple Silicon Mac (M1 / M2 / M3 / M4)
- Xcode 15.0 or later
- Apple Developer account (free tier is fine for local builds)

---

## Steps

### 1. Clone

```bash
git clone https://github.com/bssm-oss/desktop-pet.git
cd desktop-pet
```

### 2. Open in Xcode

```bash
open DesktopPet.xcodeproj
```

### 3. Set signing

In Xcode:
- Select the `DesktopPet` target
- Go to **Signing & Capabilities**
- Set your **Team** (your Apple ID)
- Bundle ID: `com.bssm-oss.desktop-pet` (or change to your own)

### 4. Build and run

Press `⌘R` or click the Run button.

The app launches silently — no dock icon appears.  
Look for the 🐾 icon in your menubar.

---

## Testing with evernight.mp4

`evernight.mp4` is a standard H.264 video. It will play correctly but **without transparency** — it will show with a solid background.

To test:
1. Click 🐾 → "Import Animation…"
2. Select `evernight.mp4`
3. The video appears as a floating overlay (with background)
4. Drag it to reposition
5. Use the settings panel to adjust opacity, scale, speed

This validates: import flow, looping, positioning, overlay behavior, and playback controls.

---

## Testing with transparent assets

To test true transparency, use one of:

### GIF
Any animated GIF with transparency. Free sources: [Giphy](https://giphy.com), [Tenor](https://tenor.com).

### APNG
Any APNG file. Test with: https://apng.onevcat.com/assets/elephant.png

### PNG sequence
Create a folder with files named:
```
frame_0001.png
frame_0002.png
frame_0003.png
...
```
Each PNG should have a transparent background (RGBA).  
Import by selecting the **folder** in the file picker.

### ProRes 4444 (transparent video)
Export from After Effects or Final Cut Pro as ProRes 4444.  
This is the recommended format for transparent video on macOS.

---

## Release build

```bash
xcodebuild -project DesktopPet.xcodeproj \
           -scheme DesktopPet \
           -configuration Release \
           -archivePath build/DesktopPet.xcarchive \
           archive
```

---

## Troubleshooting

See [troubleshooting.md](troubleshooting.md).
