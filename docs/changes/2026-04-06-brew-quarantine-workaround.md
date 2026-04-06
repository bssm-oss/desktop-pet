# Brew quarantine workaround

## Background

The public macOS release is still blocked from full notarized distribution because Apple release secrets are not configured in GitHub Actions yet. Users asked for the Homebrew install path to smooth over the current Gatekeeper workaround instead of requiring a manual copy-paste every time.

## Change

- Added a Homebrew cask `postflight` step that runs `xattr -cr /Applications/DesktopPet.app` after install.
- Added a cask `caveats` block with the same manual command as a fallback if quarantine still remains on a given machine.
- Updated README and build/install docs to distinguish the automated Homebrew workaround from the still-manual DMG path.

## Verification

- `brew install --cask --no-quarantine ...` was tested on the current Homebrew version and is disabled, so the repo-local workaround path remains the cask `postflight` hook.
- The cask definition now contains a `postflight` block and a matching user-visible fallback caveat.
- The documented brew path now aligns with the actual cask behavior.

## Remaining limit

- This is still a workaround for the current unsigned public release. The real long-term fix remains a notarized public release once Apple secrets are configured in GitHub Actions.
