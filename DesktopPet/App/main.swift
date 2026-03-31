// main.swift
// App entry point. Uses NSApplicationMain to avoid @main conflicts with AppKit.

import AppKit

let delegate = AppDelegate()
NSApplication.shared.delegate = delegate
_ = NSApplicationMain(CommandLine.argc, CommandLine.unsafeArgv)
