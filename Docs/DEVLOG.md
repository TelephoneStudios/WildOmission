# Development Log

Chronological record of the Rust-styled survival work. One entry per working session / feature branch.
Format: **what** changed, **why**, **how it was verified**, **what is next**. Design docs for individual systems live in `Docs/Design/`.

---

## 2026-11-04 - Phase 0 kickoff

**Branch:** `chore/p0-docs-and-lfs`

### Done
- Reviewed the Steam page, patch notes, discussions and guides, the Rust design brief and the code base. Findings are in `Docs/ROADMAP.md` (sections 1-3, 9).
- Installed tooling: Unreal Engine 5.8.4, Blender 5.2, Visual Studio 2022 Build Tools (C++ game workload, Win11 SDK), git-lfs.
- Added roadmap, design brief and art reference to `Docs/`.
- Enabled git-lfs for new source art/audio (`.fbx .blend .wav .mp3 .ogg .png .tga .psd`). `.uasset/.umap` are *not* migrated: existing history stays untouched and no force-push is needed.

### Decisions (from project owner)
- Rust features are **server toggles**, adjustable from an admin panel and easy to mod.
- Work happens on feature branches, merged via PR into `main` of the `chasejltl/WildOmission` fork only.
- UE 5.8 upgrade stays on its own branch until stable.

### Findings
- Upstream has an abandoned `Unreal_5-7_Migration` branch (dev rolled back in June 2026 citing instability). Used as reference for API fixes.
- Upstream `beta-1-4-0` has unreleased character-customization work; not merged.

### Next
- `feature/p0-ue58-upgrade`: get the project compiling on UE 5.8.4.
