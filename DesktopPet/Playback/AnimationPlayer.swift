// AnimationPlayer.swift
// Central playback controller for frame-based animations (GIF/APNG/PNG sequence).
//
// Uses CVDisplayLink for display-sync frame scheduling:
// - Fires in sync with the actual display refresh (up to 120Hz ProMotion)
// - Sleeps the thread between frames — no busy-wait
// - Never fires faster than the display can show
// - ProMotion-aware automatically
//
// Only updates CALayer.contents when the frame index actually changes,
// so the GPU compositor is not disturbed on frames where nothing changes.

import CoreVideo
import QuartzCore
import CoreGraphics
import Foundation
import Darwin

@MainActor
protocol AnimationPlayerDelegate: AnyObject {
    func animationPlayer(_ player: AnimationPlayer, didAdvanceTo frame: CGImage)
}

final class AnimationPlayer {

    // MARK: - Public
    weak var delegate: AnimationPlayerDelegate?

    var speed: Double = 1.0 {
        didSet { speed = max(0.1, min(speed, 8.0)) }
    }

    private(set) var isPlaying: Bool = false

    // MARK: - Private
    private var sequence: FrameSequence?
    private var displayLink: CVDisplayLink?

    /// Accumulated playback time in seconds (scaled by speed).
    private var playbackTime: TimeInterval = 0

    /// Host time of the last CVDisplayLink callback (in seconds, converted from Mach time).
    private var lastHostTime: TimeInterval = 0

    /// Cached mach_timebase_info for converting hostTime to seconds.
    /// Computed once and reused — mach_timebase_info never changes at runtime.
    private static let machTimebaseInfo: mach_timebase_info_data_t = {
        var info = mach_timebase_info_data_t()
        mach_timebase_info(&info)
        return info
    }()

    /// Last displayed frame index — avoids redundant CALayer commits.
    private var lastFrameIndex: Int = -1

    // MARK: - Load

    @MainActor
    func load(_ sequence: FrameSequence) {
        let wasPlaying = isPlaying
        stop()
        self.sequence = sequence
        playbackTime = 0
        lastFrameIndex = -1
        lastHostTime = 0

        // Show first frame immediately on main thread
        if let first = sequence.frames.first {
            delegate?.animationPlayer(self, didAdvanceTo: first)
        }

        if wasPlaying { play() }
    }

    // MARK: - Playback Control

    func play() {
        guard !isPlaying, sequence != nil else { return }
        isPlaying = true
        startDisplayLink()
    }

    func pause() {
        guard isPlaying else { return }
        isPlaying = false
        stopDisplayLink()
    }

    func togglePlayPause() {
        isPlaying ? pause() : play()
    }

    func stop() {
        pause()
        playbackTime = 0
        lastFrameIndex = -1
        lastHostTime = 0
    }

    // MARK: - CVDisplayLink

    private func startDisplayLink() {
        guard displayLink == nil else { return }

        var link: CVDisplayLink?
        CVDisplayLinkCreateWithActiveCGDisplays(&link)
        guard let link else { return }

        // Use a C-compatible trampoline. We pass a raw pointer to self.
        // The display link is stopped in deinit, so self is always valid
        // while the callback can fire.
        let rawSelf = Unmanaged.passUnretained(self).toOpaque()

        CVDisplayLinkSetOutputCallback(link, { _, inNow, _, _, _, context -> CVReturn in
            guard let context else { return kCVReturnError }
            let player = Unmanaged<AnimationPlayer>.fromOpaque(context).takeUnretainedValue()
            player.handleDisplayLinkCallback(now: inNow.pointee)
            return kCVReturnSuccess
        }, rawSelf)

        CVDisplayLinkStart(link)
        displayLink = link
    }

    private func stopDisplayLink() {
        guard let link = displayLink else { return }
        CVDisplayLinkStop(link)
        displayLink = nil
    }

    // MARK: - Callback (fires on CVDisplayLink thread, NOT main thread)

    private func handleDisplayLinkCallback(now: CVTimeStamp) {
        guard let sequence, sequence.count > 0 else { return }

        // Convert CVTimeStamp.hostTime (Mach absolute time) to seconds.
        // hostTime is in Mach ticks — must be converted via mach_timebase_info.
        // videoTimeScale is the video output timebase and must NOT be used here.
        let info = AnimationPlayer.machTimebaseInfo
        let nanos = now.hostTime * UInt64(info.numer) / UInt64(info.denom)
        let hostTime = TimeInterval(nanos) / 1_000_000_000.0

        // Calculate delta
        let delta: TimeInterval
        if lastHostTime > 0 {
            delta = hostTime - lastHostTime
        } else {
            delta = 0
        }
        lastHostTime = hostTime

        // Advance playback time
        playbackTime += delta * speed

        // Determine frame
        let newIndex = sequence.frameIndex(at: playbackTime)
        guard newIndex != lastFrameIndex else { return }
        lastFrameIndex = newIndex

        let frame = sequence.frames[newIndex]

        // Dispatch to main thread for CALayer update
        DispatchQueue.main.async { [weak self] in
            guard let self else { return }
            self.delegate?.animationPlayer(self, didAdvanceTo: frame)
        }
    }

    // MARK: - Deinit

    deinit {
        if let link = displayLink {
            CVDisplayLinkStop(link)
        }
    }
}
