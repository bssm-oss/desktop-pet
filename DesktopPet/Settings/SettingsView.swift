// SettingsView.swift
// SwiftUI settings panel. Minimal and clean.
// Presented as a floating panel from the menubar icon.

import SwiftUI
import ServiceManagement

struct SettingsView: View {
    @ObservedObject var settings: AppSettings
    var onImport: () -> Void
    var onQuit: () -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 16) {

            // Header
            HStack {
                Text("🐾 Desktop Pet")
                    .font(.headline)
                Spacer()
                Button("Import…", action: onImport)
                    .buttonStyle(.borderedProminent)
                    .controlSize(.small)
            }

            Divider()

            // Playback
            Group {
                Toggle("Playing", isOn: $settings.playing)

                LabeledSlider(
                    label: "Speed",
                    value: $settings.speed,
                    range: 0.1...4.0,
                    format: "%.1fx"
                )
            }

            Divider()

            // Appearance
            Group {
                LabeledSlider(
                    label: "Opacity",
                    value: $settings.opacity,
                    range: 0.1...1.0,
                    format: "%.0f%%",
                    displayMultiplier: 100
                )

                LabeledSlider(
                    label: "Scale",
                    value: $settings.scale,
                    range: 0.25...4.0,
                    format: "%.2fx"
                )
            }

            Divider()

            // Behavior
            Group {
                Toggle("Always on Top", isOn: $settings.alwaysOnTop)
                Toggle("Click-Through", isOn: $settings.clickThrough)
                Toggle("Lock Position", isOn: $settings.lockPosition)
            }

            Divider()

            // System
            StartAtLoginToggle()

            Divider()

            // Quit
            HStack {
                Spacer()
                Button("Quit", action: onQuit)
                    .foregroundColor(.red)
            }
        }
        .padding(16)
        .frame(width: 280)
    }
}

// MARK: - Labeled Slider

private struct LabeledSlider: View {
    let label: String
    @Binding var value: Double
    let range: ClosedRange<Double>
    let format: String
    var displayMultiplier: Double = 1.0

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text(label)
                    .font(.caption)
                    .foregroundColor(.secondary)
                Spacer()
                Text(String(format: format, value * displayMultiplier))
                    .font(.caption.monospacedDigit())
                    .foregroundColor(.secondary)
            }
            Slider(value: $value, in: range)
        }
    }
}

// MARK: - Start at Login Toggle

private struct StartAtLoginToggle: View {
    @State private var isEnabled: Bool = false

    var body: some View {
        Toggle("Start at Login", isOn: $isEnabled)
            .onAppear { isEnabled = SMAppService.mainApp.status == .enabled }
            .onChange(of: isEnabled) { newValue in
                do {
                    if newValue {
                        try SMAppService.mainApp.register()
                    } else {
                        try SMAppService.mainApp.unregister()
                    }
                } catch {
                    // Silently fail — user can set this manually in System Settings
                }
            }
    }
}
