cask "desktop-pet" do
  version "1.2.1"
  sha256 "f381b3b76f71cc9d7f9af4de28d4c90f879d42a934a91ac1615807fd1e502555"

  url "https://github.com/bssm-oss/desktop-pet/releases/download/v#{version}/DesktopPet.dmg"
  name "Desktop Pet"
  desc "Transparent animated overlay for your macOS desktop"
  homepage "https://github.com/bssm-oss/desktop-pet"

  livecheck do
    url :url
    strategy :github_latest
  end

  depends_on macos: ">= :sonoma" # macOS 14.0+

  app "DesktopPet.app"

  postflight do
    system_command "/usr/bin/xattr",
                   args: ["-cr", "#{appdir}/DesktopPet.app"],
                   must_succeed: false
  end

  caveats do
    <<~EOS
      Homebrew will try to clear the app quarantine flag after installation.

      If Gatekeeper still blocks launch on your machine, run:
        xattr -cr #{appdir}/DesktopPet.app
        open #{appdir}/DesktopPet.app
    EOS
  end

  zap trash: [
    "~/Library/Preferences/com.bssm-oss.desktop-pet.plist",
    "~/Library/Application Support/desktop-pet",
  ]
end
