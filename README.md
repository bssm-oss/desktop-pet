# 🐾 Desktop Pet

A lightweight, native desktop overlay app.  
Import a GIF, APNG, PNG sequence, or video — it floats on your screen, loops forever, and stays visible across all Spaces / virtual desktops. Run multiple pets at once, each with its own file and settings.

**macOS 14+** (Swift + AppKit) · **Windows 10+** (Win32 + C++17)

![macOS 14+](https://img.shields.io/badge/macOS-14%2B-blue)
![Windows 10+](https://img.shields.io/badge/Windows-10%2B-blue)
![Apple Silicon](https://img.shields.io/badge/Apple%20Silicon-Optimized-green)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

---

## ⬇️ 설치 방법

### 방법 1 — Homebrew (권장)

[Homebrew](https://brew.sh)가 설치되어 있다면 터미널에 아래 두 줄을 붙여넣으세요:

```bash
brew tap bssm-oss/desktop-pet https://github.com/bssm-oss/desktop-pet.git
brew install --cask bssm-oss/desktop-pet/desktop-pet
```

설치 후 **처음 실행 시 quarantine 해제**가 필요합니다:

```bash
xattr -cr /Applications/DesktopPet.app
open /Applications/DesktopPet.app
```

> 이 명령은 macOS가 인터넷에서 다운로드된 앱에 붙이는 격리 플래그를 제거합니다.  
> Apple 개발자 서명이 없는 앱은 이 과정 없이 "손상됨" 경고가 뜹니다.

업데이트:

```bash
brew upgrade --cask desktop-pet
xattr -cr /Applications/DesktopPet.app   # 업데이트 후에도 한 번 실행
```

삭제:

```bash
brew uninstall --cask desktop-pet
```

> **Homebrew가 없다면:** 터미널에서 아래 명령으로 먼저 설치하세요.
> ```bash
> /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
> ```

---

### 방법 2 — DMG 직접 다운로드

1. 이 페이지 오른쪽 **[Releases](https://github.com/bssm-oss/desktop-pet/releases)** 클릭
2. 최신 버전의 **`DesktopPet.dmg`** 다운로드
3. DMG 열기 → `DesktopPet.app`을 **Applications** 폴더로 드래그
4. 터미널에서 quarantine 해제 후 실행:

```bash
xattr -cr /Applications/DesktopPet.app
open /Applications/DesktopPet.app
```

> **왜 이 명령이 필요한가요?**  
> Apple은 앱스토어 외부에서 받은 앱에 격리(quarantine) 플래그를 붙입니다.  
> Apple Developer Program($99/년) 서명이 없는 오픈소스 앱은 이 플래그를 수동으로 제거해야 합니다.  
> `xattr -cr`은 앱 파일 자체를 수정하지 않으며, macOS의 메타데이터만 제거합니다.

또는 GUI로: **시스템 설정 → 개인 정보 보호 및 보안 → "확인 없이 열기"** 클릭

---

### 방법 3 — 소스에서 직접 빌드

요구사항: macOS 14+, Xcode Command Line Tools

```bash
# Command Line Tools 설치 (없다면)
xcode-select --install

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

---

## 🪟 Windows 설치

### 방법 1 — Scoop (권장)

[Scoop](https://scoop.sh)이 설치되어 있다면:

```powershell
scoop bucket add desktop-pet https://github.com/bssm-oss/desktop-pet.git
scoop install desktop-pet
```

> **Scoop이 없다면:** PowerShell에서 아래 명령으로 먼저 설치하세요.
> ```powershell>+
> irm get.scoop.sh | iex
> ```

### 방법 2 — WinGet

```powershell
winget install bssm-oss.desktop-pet
```

### 방법 3 — 직접 다운로드

1. 이 페이지 오른쪽 **[Releases](https://github.com/bssm-oss/desktop-pet/releases)** 클릭
2. 최신 버전의 **`DesktopPet-Windows.zip`** 다운로드
3. 압축 해제 후 `DesktopPet.exe` 실행

> 첫 실행 시 Windows Defender 경고가 뜰 수 있습니다.
> **추가 정보 → 실행** 클릭하세요. 서명되지 않은 오픈소스 앱이기 때문입니다.

### 방법 4 — 소스에서 직접 빌드

요구사항: Windows 10+, MSYS2 (MinGW-w64)

```powershell
# MSYS2 설치 (없다면)
winget install -e --id MSYS2.MSYS2

# MSYS2 UCRT64 터미널에서:
pacman -S --noconfirm mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja

git clone https://github.com/bssm-oss/desktop-pet.git
cd desktop-pet/Windows
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

빌드 결과: `Windows/build/DesktopPet.exe`

---

## 사용 방법

앱을 실행하면 독(Dock)에는 아이콘이 없고, **메뉴바에 🐾 아이콘**이 나타납니다.

### 펫 추가하기

1. 🐾 클릭 → **Add Pet…**
2. 파일 선택 창에서 GIF / APNG / PNG 폴더 / 영상 선택
3. 선택한 파일이 바로 화면에 떠오릅니다

### 여러 마리 동시에 띄우기

**Add Pet…** 을 반복하면 됩니다. 각 펫은 파일, 위치, 크기, 투명도가 모두 독립적입니다.

### 조작

| 동작 | 방법 |
|------|------|
| 위치 이동 | 캐릭터를 클릭하고 드래그 |
| 파일 바꾸기 | 파일을 캐릭터 위로 드래그 앤 드롭 |
| 설정 열기 | 🐾 클릭 → Pet N → Settings… |
| 빠른 조작 | 🐾 클릭 → Pet N → Import / Remove |
| 펫 제거 | Settings 패널 → Remove Pet |

### 각 펫별 설정 옵션

| 옵션 | 설명 |
|------|------|
| Name | 펫 이름 (메뉴에 표시) |
| Playing | 재생 / 일시정지 |
| Speed | 재생 속도 (0.1×~4.0×) |
| Opacity | 투명도 (10%~100%) |
| Scale | 크기 배율 (0.25×~4.0×) |
| Flip H | 좌우 반전 |
| Flip V | 상하 반전 |
| Always on Top | 항상 최상단 |
| Click-Through | 클릭이 아래 창으로 통과 |
| Lock Position | 드래그 잠금 |
| Start at Login | 로그인 시 자동 실행 |

---

## GIF 찾는 법

원하는 캐릭터 GIF를 빠르게 구하는 추천 방법입니다.

### 1단계 — Tenor에서 GIF 찾기

**[tenor.com](https://tenor.com)** 에서 원하는 캐릭터 이름으로 검색합니다.  
예시: [Evernight (崩壊スターレイル)](https://tenor.com/ko/view/evernight-everknight-star-rail-hsr-honkai-star-rail-gif-8534717503006384541)

### 2단계 — GIF 다운로드

Tenor는 직접 다운로드가 불편하므로 아래 사이트를 이용하세요:

**[convertico.com/tenor-downloader](https://convertico.com/tenor-downloader/)**

1. Tenor GIF 페이지 URL을 붙여넣기
2. Download 클릭 → `.gif` 파일 저장

### 3단계 — 배경 제거 (선택)

배경이 있는 GIF는 아래 사이트에서 투명 배경으로 변환할 수 있습니다:

**[onlinegiftools.com/remove-gif-background](https://onlinegiftools.com/remove-gif-background)**

1. GIF 업로드
2. 제거할 배경색 선택
3. 처리된 GIF 다운로드 → Desktop Pet에 드래그 앤 드롭

> **투명 배경이 이미 있는 GIF**는 배경 제거 없이 바로 사용 가능합니다.  
> APNG나 PNG 시퀀스도 투명 배경을 완벽하게 지원합니다.

---

## 지원 파일 형식

| 형식 | 투명 배경 | 비고 |
|------|----------|------|
| GIF | ✅ | 잔상 없이 정확한 disposal 처리 |
| APNG | ✅ | macOS 14 네이티브 지원 |
| PNG 시퀀스 (폴더) | ✅ | 파일명 순 정렬, 기본 24fps |
| MP4 / MOV (H.264/H.265) | ❌ | 투명 배경 없음 |
| ProRes 4444 (.mov) | ✅ | 투명 비디오, 하드웨어 디코딩 |
| HEVC with Alpha (.mov) | ✅ | macOS 13+, 작은 파일 크기 |

---

## 성능 — 다른 앱과 비교

Desktop Pet은 네이티브 Swift + AppKit으로 만들어졌습니다.  
유사한 기능의 앱들과 실측 비교입니다.

### 메모리 & CPU (M 시리즈 Mac, 애니메이션 재생 중)

| 앱 | 방식 | 바이너리 크기 | 유휴 CPU | 메모리(앱 자체) |
|----|------|------------|---------|--------------|
| **Desktop Pet** | Swift + AppKit | **364 KB** | **~0%** | **~5 MB** |
| Electron 앱 (일반) | Chromium 내장 | 150–300 MB | 1–5% | 200–500 MB |
| Unity 데스크탑 마스코트 | Unity Engine | 50–200 MB | 2–10% | 100–400 MB |
| Qt 기반 앱 | Qt Framework | 20–80 MB | 1–3% | 50–200 MB |
| macOS Finder | AppKit | — | ~0% | ~360 MB |
| macOS Dock | AppKit | — | ~0% | ~123 MB |

> **왜 이렇게 가볍나요?**
>
> - **Electron/CEF 없음** — Chromium 렌더러를 내장하지 않음
> - **Unity/게임엔진 없음** — 물리엔진, 셰이더 컴파일러 등 불필요한 서브시스템 없음
> - **서드파티 라이브러리 없음** — Apple 프레임워크만 사용 (ImageIO, AVFoundation, CoreAnimation)
> - **CVDisplayLink** — 화면 갱신 시에만 깨어남, 프레임 사이에는 완전 휴면
> - **CALayer 제로카피** — CPU → GPU 픽셀 복사 없이 CGImage를 GPU에 직접 업로드
> - **로드 타임 디코딩** — GIF/APNG를 실행 시점에 한 번만 디코딩, 재생 중 추가 작업 없음

### 실측 수치 (M1 Mac, 120Hz ProMotion)

```
바이너리 크기:    364 KB   (Electron 앱 대비 약 800×)
앱 번들 전체:     448 KB
유휴 CPU:         0.0%
재생 중 CPU:      < 1%
앱 고유 메모리:   ~5 MB    (malloc 기준)
스레드 수:        7개       (메인 + CVDisplayLink + GCD 워커)
배터리 영향:      0.0       (Activity Monitor power 기준)
```

---

## 프로젝트 구조

```
├── DesktopPet/             # macOS 소스 (Swift + AppKit)
│   ├── App/                # AppDelegate, main.swift, Info.plist
│   ├── Window/             # OverlayWindow, PetView, OverlayWindowController
│   ├── Playback/           # AnimationPlayer, decoders, FrameSequence
│   ├── MenuBar/            # MenuBarController
│   ├── Settings/           # AppSettings, SettingsView
│   └── Utilities/          # PlaceholderAnimation, SecurityScopedAccess
│
├── Windows/                # Windows 소스 (C++17 + Win32)
│   ├── CMakeLists.txt
│   └── DesktopPet/
│       ├── App/            # WinMain, AppController
│       ├── Window/         # OverlayWindow, PetRender
│       ├── Playback/       # AnimationPlayer, GIF/APNG/PNG decoders
│       ├── TrayIcon/       # TrayController (시스템 트레이)
│       ├── Settings/       # AppSettings, SettingsDialog
│       └── Utilities/      # PlaceholderAnimation
│
├── Casks/                  # Homebrew Cask (macOS)
│   └── desktop-pet.rb
├── bucket/                 # Scoop manifest (Windows)
│   └── desktop-pet.json
├── winget/                 # WinGet manifest (Windows)
│   └── desktop-pet.yaml
└── docs/
```

---

## Docs

- [Architecture](docs/architecture.md)
- [Build & Run](docs/build-and-run.md)
- [Supported Formats](docs/formats.md)
- [Performance](docs/performance.md)
- [Troubleshooting](docs/troubleshooting.md)

---

## License

MIT — see [LICENSE](LICENSE)
