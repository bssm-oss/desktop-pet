// OverlayWindowController.swift
// Owns and manages one transparent overlay NSWindow.
// Coordinates between AppSettings, AnimationPlayer, VideoPlayer, and PetView.

import AppKit
import AVFoundation
import QuartzCore
import Combine

final class OverlayWindowController: NSWindowController {

    // MARK: - Dependencies
    let settings: AppSettings
    private var cancellables = Set<AnyCancellable>()

    // MARK: - Playback
    private let animationPlayer = AnimationPlayer()
    private var videoPlayer: VideoPlayer?

    // MARK: - Views
    private var petView: PetView!

    // MARK: - State
    private var currentAssetURL: URL?
    private var currentAssetIsSecurityScoped: Bool = false
    private var isVideoMode: Bool = false
    private var naturalSize: NSSize = NSSize(width: 200, height: 200)

    // MARK: - Init

    init(settings: AppSettings) {
        self.settings = settings

        let size = NSSize(width: 300, height: 300)
        let origin = settings.savedPosition()
        let rect = NSRect(origin: origin, size: size)
        let window = OverlayWindow(contentRect: rect)

        super.init(window: window)

        setupPetView()
        setupAnimationPlayer()
        applySettings()
        observeSettings()
    }

    required init?(coder: NSCoder) { fatalError("not used") }

    deinit {
        stopCurrentSecurityScopedAccess()
    }

    // MARK: - Setup

    private func setupPetView() {
        guard let window else { return }
        let view = PetView(frame: window.contentView!.bounds)
        view.autoresizingMask = [.width, .height]
        view.delegate = self
        window.contentView?.addSubview(view)
        self.petView = view
    }

    private func setupAnimationPlayer() {
        animationPlayer.delegate = self
        animationPlayer.speed = settings.speed
    }

    private func applySettings() {
        guard let window = window as? OverlayWindow else { return }
        window.setClickThrough(settings.clickThrough)
        window.setAlwaysOnTop(settings.alwaysOnTop)
        window.alphaValue = settings.opacity
        petView.lockPosition = settings.lockPosition
        petView.clickThrough = settings.clickThrough
        animationPlayer.speed = settings.speed
        if settings.playing { animationPlayer.play() }
    }

    // MARK: - Settings Observation

    private func observeSettings() {
        settings.$opacity.sink { [weak self] v in
            self?.window?.alphaValue = v
        }.store(in: &cancellables)

        // dropFirst(1): skip the initial emit that fires during init.
        // At that point naturalSize is still (200,200) — the default before any
        // asset is loaded — so applyScale would recompute the origin from a wrong
        // center and silently move the window away from its saved position.
        settings.$scale.dropFirst(1).sink { [weak self] v in
            self?.applyScale(v)
        }.store(in: &cancellables)

        settings.$clickThrough.sink { [weak self] v in
            (self?.window as? OverlayWindow)?.setClickThrough(v)
            self?.petView.clickThrough = v
        }.store(in: &cancellables)

        settings.$lockPosition.sink { [weak self] v in
            self?.petView.lockPosition = v
        }.store(in: &cancellables)

        settings.$alwaysOnTop.sink { [weak self] v in
            (self?.window as? OverlayWindow)?.setAlwaysOnTop(v)
        }.store(in: &cancellables)

        settings.$speed.sink { [weak self] v in
            self?.animationPlayer.speed = v
            self?.videoPlayer?.speed = Float(v)
        }.store(in: &cancellables)

        settings.$playing.sink { [weak self] v in
            guard let self else { return }
            if v { self.animationPlayer.play(); self.videoPlayer?.play() }
            else { self.animationPlayer.pause(); self.videoPlayer?.pause() }
        }.store(in: &cancellables)

        settings.$flipHorizontal.sink { [weak self] v in
            self?.petView.flipHorizontal = v
        }.store(in: &cancellables)

        settings.$flipVertical.sink { [weak self] v in
            self?.petView.flipVertical = v
        }.store(in: &cancellables)
    }

    // MARK: - Visibility

    /// Show or hide the overlay window without closing it.
    /// The animation continues running in the background while hidden.
    func setVisible(_ visible: Bool) {
        if visible {
            window?.orderFront(nil)
        } else {
            window?.orderOut(nil)
        }
    }

    var isVisible: Bool { window?.isVisible ?? false }

    // MARK: - Asset Loading

    func loadAssetFromBookmark(_ bookmark: Data) -> Bool {
        guard let url = SecurityScopedAccess.resolve(bookmark: bookmark) else { return false }
        stopCurrentSecurityScopedAccess()
        let didStartSecurityScope = url.startAccessingSecurityScopedResource()
        currentAssetIsSecurityScoped = didStartSecurityScope

        let didLoad = loadAsset(url: url, preservingSecurityScope: true)
        if !didLoad, didStartSecurityScope {
            url.stopAccessingSecurityScopedResource()
            currentAssetIsSecurityScoped = false
        }

        return didLoad
    }

    /// Stop accessing the current security-scoped resource if one is active.
    private func stopCurrentSecurityScopedAccess() {
        guard currentAssetIsSecurityScoped, let url = currentAssetURL else { return }
        url.stopAccessingSecurityScopedResource()
        currentAssetIsSecurityScoped = false
    }

    func loadAsset(url: URL) -> Bool {
        loadAsset(url: url, preservingSecurityScope: false)
    }

    private func loadAsset(url: URL, preservingSecurityScope: Bool) -> Bool {
        if !preservingSecurityScope {
            stopCurrentSecurityScopedAccess()
        }

        currentAssetURL = url
        let ext = url.pathExtension.lowercased()

        if let bookmark = SecurityScopedAccess.bookmark(for: url) {
            settings.assetBookmark = bookmark
        }

        switch ext {
        case "gif":
            return loadGIF(url: url)
        case "png", "apng":
            return loadAPNG(url: url)
        case "mp4", "mov", "m4v", "avi", "mkv":
            return loadVideo(url: url)
        default:
            var isDir: ObjCBool = false
            if FileManager.default.fileExists(atPath: url.path, isDirectory: &isDir), isDir.boolValue {
                return loadPNGSequence(directory: url)
            } else {
                return loadAPNG(url: url)
            }
        }
    }

    private func loadGIF(url: URL) -> Bool {
        isVideoMode = false
        removeVideoLayer()
        guard let seq = GIFDecoder.decode(url: url) else { return false }
        resizeWindow(to: sizeForSequence(seq))
        animationPlayer.load(seq)
        if settings.playing { animationPlayer.play() }
        return true
    }

    private func loadAPNG(url: URL) -> Bool {
        isVideoMode = false
        removeVideoLayer()
        guard let seq = APNGDecoder.decode(url: url) else { return false }
        resizeWindow(to: sizeForSequence(seq))
        animationPlayer.load(seq)
        if settings.playing { animationPlayer.play() }
        return true
    }

    private func loadPNGSequence(directory: URL) -> Bool {
        isVideoMode = false
        removeVideoLayer()
        guard let seq = PNGSequenceDecoder.decode(directory: directory) else { return false }
        resizeWindow(to: sizeForSequence(seq))
        animationPlayer.load(seq)
        if settings.playing { animationPlayer.play() }
        return true
    }

    private func loadVideo(url: URL) -> Bool {
        isVideoMode = true
        animationPlayer.stop()
        guard let vp = VideoPlayer(url: url) else { return false }
        videoPlayer = vp
        vp.speed = Float(settings.speed)
        if let rootLayer = petView.layer {
            vp.playerLayer.frame = rootLayer.bounds
            vp.playerLayer.autoresizingMask = [.layerWidthSizable, .layerHeightSizable]
            rootLayer.addSublayer(vp.playerLayer)
        }
        if settings.playing { vp.play() }
        return true
    }

    private func removeVideoLayer() {
        videoPlayer?.playerLayer.removeFromSuperlayer()
        videoPlayer = nil
    }

    // MARK: - Window Sizing

    private func sizeForSequence(_ seq: FrameSequence) -> NSSize {
        guard let first = seq.frames.first else { return NSSize(width: 200, height: 200) }
        naturalSize = NSSize(width: CGFloat(first.width), height: CGFloat(first.height))
        return NSSize(
            width: naturalSize.width * settings.scale,
            height: naturalSize.height * settings.scale
        )
    }

    private func resizeWindow(to size: NSSize) {
        guard let window else { return }
        // Preserve the saved origin exactly — do NOT recompute from center.
        // The origin is the bottom-left corner in macOS screen coordinates.
        // Re-reading savedPosition() (not window.frame.origin) ensures that
        // a freshly-restored window uses the persisted value, not whatever
        // temporary origin was set during the window init phase.
        let origin = settings.savedPosition()
        window.setFrame(NSRect(origin: origin, size: size), display: true, animate: false)
    }

    private func applyScale(_ scale: Double) {
        guard let window else { return }
        let center = NSPoint(x: window.frame.midX, y: window.frame.midY)
        let newW = naturalSize.width * scale
        let newH = naturalSize.height * scale
        let newOrigin = NSPoint(x: center.x - newW / 2, y: center.y - newH / 2)
        window.setFrame(NSRect(origin: newOrigin, size: NSSize(width: newW, height: newH)),
                        display: true, animate: false)
        if let vp = videoPlayer, let rootLayer = petView.layer {
            vp.playerLayer.frame = rootLayer.bounds
        }
    }
}

// MARK: - AnimationPlayerDelegate

extension OverlayWindowController: AnimationPlayerDelegate {
    func animationPlayer(_ player: AnimationPlayer, didAdvanceTo frame: CGImage) {
        petView.display(frame: frame)
    }
}

// MARK: - PetViewDelegate

extension OverlayWindowController: PetViewDelegate {
    func petView(_ view: PetView, didDropFileAt url: URL) {
        _ = loadAsset(url: url)
    }

    func petViewDidFinishDrag(_ view: PetView) {
        guard let window else { return }
        settings.savePosition(window.frame.origin)
    }
}
