// PetView.swift
// NSView hosting the CALayer render target.
// Handles frame display, drag-to-reposition, file drop, flip transforms.

import AppKit
import QuartzCore
import CoreGraphics

protocol PetViewDelegate: AnyObject {
    func petView(_ view: PetView, didDropFileAt url: URL)
    func petViewDidFinishDrag(_ view: PetView)
}

final class PetView: NSView {

    // MARK: - Public
    weak var delegate: PetViewDelegate?
    var lockPosition: Bool  = false
    var clickThrough: Bool  = false

    var flipHorizontal: Bool = false { didSet { applyFlip() } }
    var flipVertical:   Bool = false { didSet { applyFlip() } }

    // MARK: - Render Layer
    private let imageLayer = CALayer()

    // MARK: - Drag State
    private var dragStartMouseLocation: NSPoint?
    private var dragStartWindowOrigin:  NSPoint?

    // MARK: - Init

    override init(frame: NSRect) { super.init(frame: frame); setup() }
    required init?(coder: NSCoder) { super.init(coder: coder); setup() }

    private func setup() {
        wantsLayer = true
        layer?.backgroundColor = CGColor.clear

        imageLayer.frame = bounds
        imageLayer.contentsGravity = .resizeAspect
        imageLayer.backgroundColor = CGColor.clear
        imageLayer.autoresizingMask = [.layerWidthSizable, .layerHeightSizable]
        layer?.addSublayer(imageLayer)

        registerForDraggedTypes([.fileURL])
    }

    // MARK: - Frame Display

    func display(frame: CGImage) {
        CATransaction.begin()
        CATransaction.setDisableActions(true)
        imageLayer.contents = frame
        CATransaction.commit()
    }

    // MARK: - Flip

    private func applyFlip() {
        CATransaction.begin()
        CATransaction.setDisableActions(true)
        var t = CATransform3DIdentity
        if flipHorizontal { t = CATransform3DScale(t, -1, 1, 1) }
        if flipVertical   { t = CATransform3DScale(t, 1, -1, 1) }
        imageLayer.transform = t
        CATransaction.commit()
    }

    // MARK: - Layout

    override func layout() {
        super.layout()
        imageLayer.frame = bounds
    }

    // MARK: - Mouse Events

    override func mouseDown(with event: NSEvent) {
        guard !lockPosition, !clickThrough else { return }
        dragStartMouseLocation = NSEvent.mouseLocation
        dragStartWindowOrigin  = window?.frame.origin
    }

    override func mouseDragged(with event: NSEvent) {
        guard !lockPosition, !clickThrough,
              let startMouse  = dragStartMouseLocation,
              let startOrigin = dragStartWindowOrigin,
              let window = window
        else { return }

        let cur = NSEvent.mouseLocation
        window.setFrameOrigin(NSPoint(
            x: startOrigin.x + cur.x - startMouse.x,
            y: startOrigin.y + cur.y - startMouse.y
        ))
    }

    override func mouseUp(with event: NSEvent) {
        guard dragStartMouseLocation != nil else { return }
        dragStartMouseLocation = nil
        dragStartWindowOrigin  = nil
        delegate?.petViewDidFinishDrag(self)
    }

    // MARK: - Drag & Drop Import

    override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation {
        fileURL(from: sender) != nil ? .copy : []
    }

    override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
        guard let url = fileURL(from: sender) else { return false }
        delegate?.petView(self, didDropFileAt: url)
        return true
    }

    private func fileURL(from info: NSDraggingInfo) -> URL? {
        guard let item = info.draggingPasteboard.pasteboardItems?.first,
              let str  = item.string(forType: .fileURL),
              let url  = URL(string: str)
        else { return nil }
        let supported = ["gif","png","apng","mp4","mov","m4v"]
        return supported.contains(url.pathExtension.lowercased()) ? url : nil
    }

    // MARK: - Hit Testing

    override func hitTest(_ point: NSPoint) -> NSView? {
        clickThrough ? nil : super.hitTest(point)
    }
}
