// PlaceholderAnimation.swift
// Generates a built-in fallback animation programmatically.
// Shown on first launch before any asset is imported.
// No external files required — pure CoreGraphics drawing.

import CoreGraphics
import Foundation
import AppKit

enum PlaceholderAnimation {

    /// Generate a simple bouncing circle animation.
    /// Returns a FrameSequence ready for AnimationPlayer.
    static func make(size: CGSize = CGSize(width: 200, height: 200),
                     frameCount: Int = 24) -> FrameSequence {
        var frames: [CGImage] = []
        let delay = 1.0 / 24.0

        let colorSpace = CGColorSpaceCreateDeviceRGB()
        let bitmapInfo = CGImageAlphaInfo.premultipliedLast.rawValue

        for i in 0..<frameCount {
            guard let ctx = CGContext(
                data: nil,
                width: Int(size.width),
                height: Int(size.height),
                bitsPerComponent: 8,
                bytesPerRow: 0,
                space: colorSpace,
                bitmapInfo: bitmapInfo
            ) else { continue }

            // Transparent background
            ctx.clear(CGRect(origin: .zero, size: size))

            let t = Double(i) / Double(frameCount)
            let angle = t * .pi * 2

            // Bouncing offset
            let bounceY = sin(angle) * 30.0
            let cx = size.width / 2
            let cy = size.height / 2 + bounceY
            let radius: CGFloat = 50

            // Gradient fill: pink to purple
            let hue = 0.8 + t * 0.2 // cycles through pink/purple
            let color = NSColor(hue: hue, saturation: 0.7, brightness: 1.0, alpha: 1.0)
            ctx.setFillColor(color.cgColor)

            // Body circle
            ctx.fillEllipse(in: CGRect(
                x: cx - radius, y: cy - radius,
                width: radius * 2, height: radius * 2
            ))

            // Eyes
            ctx.setFillColor(CGColor(gray: 1.0, alpha: 0.9))
            let eyeR: CGFloat = 8
            ctx.fillEllipse(in: CGRect(x: cx - 18, y: cy + 10, width: eyeR*2, height: eyeR*2))
            ctx.fillEllipse(in: CGRect(x: cx + 2,  y: cy + 10, width: eyeR*2, height: eyeR*2))

            // Pupils
            ctx.setFillColor(CGColor(gray: 0.1, alpha: 1.0))
            let pupilR: CGFloat = 4
            ctx.fillEllipse(in: CGRect(x: cx - 14, y: cy + 14, width: pupilR*2, height: pupilR*2))
            ctx.fillEllipse(in: CGRect(x: cx + 6,  y: cy + 14, width: pupilR*2, height: pupilR*2))

            // Ears
            let earColor = NSColor(hue: hue, saturation: 0.5, brightness: 1.0, alpha: 1.0)
            ctx.setFillColor(earColor.cgColor)
            ctx.fillEllipse(in: CGRect(x: cx - radius - 10, y: cy + radius - 10, width: 24, height: 28))
            ctx.fillEllipse(in: CGRect(x: cx + radius - 14, y: cy + radius - 10, width: 24, height: 28))

            if let img = ctx.makeImage() {
                frames.append(img)
            }
        }

        let delays = Array(repeating: delay, count: frames.count)
        return FrameSequence(frames: frames, delays: delays)
    }
}
