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
//
// Thread safety:
// - CVDisplayLink fires its callback on a private thread.
// - The main thread calls load/play/pause/stop.
// - `playbackState` (time, lastFrameIndex, lastHostTime) and `sequence` are
//   accessed from both threads, so they are protected by `stateLock`.

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

    // MARK: - Lock-protected state
    //
    // These properties are read/written from both the main thread and the
    // CVDisplayLink callback thread. Access them only while holding `stateLock`.

    private var stateLock = OSAllocatedUnfairLock(initialState: PlaybackState())

    private struct PlaybackState {
        var sequence: FrameSequence? = nil
        var playbackTime: TimeInterval = 0
        var lastHostTime: TimeInterval = 0
        var lastFrameIndex: Int = -1
    }

    // MARK: - Main-thread-only state
    private var displayLink: CVDisplayLink?

    // MARK: - Cached mach_timebase_info
    // mach_timebase_info never changes at runtime — compute once.
    private static let machTimebaseInfo: mach_timebase_info_data_t = {
        var info = mach_timebase_info_data_t()
        mach_timebase_info(&info)
        return info
    }()

    // MARK: - Load

    @MainActor
    func load(_ sequence: FrameSequence) {
        let wasPlaying = isPlaying
        stop()
        stateLock.withLock { state in
            state.sequence = sequence
            state.playbackTime = 0
            state.lastFrameIndex = -1
            state.lastHostTime = 0
        }

        // Show first frame immediately on main thread
        if let first = sequence.frames.first {
            delegate?.animationPlayer(self, didAdvanceTo: first)
        }

        if wasPlaying { play() }
    }

    // MARK: - Playback Control

    func play() {
        guard !isPlaying else { return }
        let hasSequence = stateLock.withLock { $0.sequence != nil }
        guard hasSequence else { return }
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
        stateLock.withLock { state in
            state.playbackTime = 0
            state.lastFrameIndex = -1
            state.lastHostTime = 0
        }
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
        // Convert CVTimeStamp.hostTime (Mach absolute time) to seconds.
        // hostTime is in Mach ticks — must be converted via mach_timebase_info.
        // videoTimeScale is the video output timebase and must NOT be used here.
        let info = AnimationPlayer.machTimebaseInfo
        let nanos = now.hostTime * UInt64(info.numer) / UInt64(info.denom)
        let hostTime = TimeInterval(nanos) / 1_000_000_000.0

        // All shared state is accessed under the lock.
        let result: (frameIndex: Int, frame: CGImage)? = stateLock.withLock { state in
            guard let sequence = state.sequence, sequence.count > 0 else { return nil }

            // Calculate delta
            let delta: TimeInterval
            if state.lastHostTime > 0 {
                delta = hostTime - state.lastHostTime
            } else {
                delta = 0
            }
            state.lastHostTime = hostTime

            // Advance playback time
            state.playbackTime += delta * speed

            // Determine frame
            let newIndex = sequence.frameIndex(at: state.playbackTime)
            guard newIndex != state.lastFrameIndex else { return nil }
            state.lastFrameIndex = newIndex

            return (newIndex, sequence.frames[newIndex])
        }

        guard let result else { return }

        // Dispatch to main thread for CALayer update
        DispatchQueue.main.async { [weak self] in
            guard let self else { return }
            self.delegate?.animationPlayer(self, didAdvanceTo: result.frame)
        }
    }

    // MARK: - Deinit

    deinit {
        if let link = displayLink {
            CVDisplayLinkStop(link)
        }
    }
}
