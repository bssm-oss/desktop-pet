## What

<!-- Short summary of what changed. -->

## Why

<!-- Why this change is needed. Link to the related issue if applicable. -->
<!-- Closes #<issue-number> -->

## How

<!-- Key implementation decisions, trade-offs considered. -->

## Testing

<!-- How you verified the fix/feature. Be specific: -->
<!-- - "Relaunched app 5 times, window always restored to saved position" -->
<!-- - "Tested with 10 different GIFs including ones with transparency" -->

---

## Pre-merge checklist

- [ ] Compiles with **zero errors and zero warnings** (`Build Check` CI job passes)
- [ ] Tested on Apple Silicon Mac running macOS 14+
- [ ] No third-party Swift Package Manager dependencies added
- [ ] No `NSTimer` used for animation (CVDisplayLink only)
- [ ] No `NSImageView.image =` per-frame (use `CALayer.contents =`)
- [ ] Window transparency invariants maintained
- [ ] Spaces / fullscreen behavior maintained (`.fullScreenAuxiliary` present)
- [ ] Commits follow `type(scope): description` format
- [ ] One logical change per commit
