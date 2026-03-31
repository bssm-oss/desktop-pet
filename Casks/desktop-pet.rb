cask "desktop-pet" do
  version "1.0.0"
  sha256 "a6d3bfe786473fd1137bc522b06b8a0ec7d3db1a27d41887a903f19c603dd264"

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

  zap trash: [
    "~/Library/Preferences/com.bssm-oss.desktop-pet.plist",
    "~/Library/Application Support/desktop-pet",
  ]
end
