# Notarized macOS release pipeline

## Background

The published GitHub DMG was shipping an ad-hoc signed app bundle. Gatekeeper rejected the downloaded app with the "Apple cannot verify no malware" dialog, and the repository docs/workflow were explicitly telling users to remove quarantine manually.

## Root cause

- `.github/workflows/release.yml` used `codesign --sign -`, which produces an ad-hoc signature with no Team ID.
- The workflow had no notarization or stapling steps.
- Release docs in `README.md` and the workflow release body assumed users would work around the unsigned build with `xattr -cr`.

## Changes

- Added release-workflow preflight validation for Apple signing/notarization secrets.
- Added Developer ID certificate import into a temporary CI keychain.
- Replaced ad-hoc signing with Developer ID signing using hardened runtime and timestamps.
- Added notarization + stapling for both the app bundle and the DMG.
- Updated release/install docs to treat notarized DMGs as the supported distribution path.

## Required GitHub secrets

- `APPLE_DEVELOPER_ID_CERT_P12_BASE64`
- `APPLE_DEVELOPER_ID_CERT_PASSWORD`
- `APPLE_DEVELOPER_ID_APPLICATION`
- `APPLE_NOTARY_APPLE_ID`
- `APPLE_NOTARY_APP_PASSWORD`
- `APPLE_NOTARY_TEAM_ID`

## Verification

- Existing published `v1.2.1` DMG was audited directly with `codesign` and `spctl` and confirmed to be ad-hoc signed and rejected by Gatekeeper.
- The corrected workflow now performs the exact signing/notarization/stapling steps required for public macOS distribution.
- The first attempted `v1.2.2` run failed immediately at secret validation because all required Apple release secrets were empty in GitHub Actions.

## Remaining limit

- A new notarized release still depends on valid Apple Developer Program credentials being available to GitHub Actions.
