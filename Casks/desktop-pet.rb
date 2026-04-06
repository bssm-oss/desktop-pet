cask "desktop-pet" do
  version "1.2.0"
  sha256 "bb694fa96858483f17dcdcefb55b98019342741fb7226021aeff1ce830f28a3e"

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
