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

---

## 2026-11-05 - Phase 0: UE 5.8.4 Engine Upgrade & Asset Pipeline

**Branch:** `feature/p0-ue58-upgrade`

### What changed & why
1. **Project & Target Rules (`WildOmission.uproject`, `Source/*.Target.cs`):**
   - Associated project with Unreal Engine `5.8` (`5.8.4`).
   - Removed obsolete `bUseIris` and `bUsesSteam` properties from `TargetRules`.
   - Upgraded `DefaultBuildSettings` from `BuildSettingsVersion.V4` to `BuildSettingsVersion.V7` so `WildOmissionEditorTarget` matches `UnrealEditor`'s warning and floating-point semantics.
2. **Breaking Circular Module Dependency (`Structures` module split):**
   - UE 5.8's `UnrealBuildTool` enforces stricter module cycle validation and rejected the `WorldGeneration <-> Deployables` cycle caused by `ULootSpawnComponent` spawning `ADeployable`.
   - Extracted `AStructure` and `ULootSpawnComponent` into a new `Structures` runtime module (`Source/Structures/`) with explicit `CircularlyReferencedDependentModules` and added `[CoreRedirects]` in `Config/DefaultEngine.ini` so existing Blueprints (`/Script/WorldGeneration.Structure` and `/Script/WorldGeneration.LootSpawnComponent`) resolve seamlessly.
3. **UE 5.8 C++ API & Strictness Fixes:**
   - Replaced removed `TArray::SetNum(int32, bool)` overload with `EAllowShrinking::No` in `UInventoryComponent`.
   - Added explicit `#include "Engine/OverlapResult.h"` in `ATornado`, `UWindSuckerComponent`, and `ARocketProjectile`.
   - Updated `SteamHelpers` and `Workshop` to include `steam/steam_api.h` via the `Steamworks` module's public system include path and exposed `Steamworks` publicly on `Workshop`.
   - Fixed compile-time `FString::Printf` / `UE_LOG` format string validation errors in `ULoadingMenuWidget` (`%%` escape) and `AWildOmissionGameMode` (missing `%s` specifiers).
   - Linked `CoreOnline` in `Locks`, `Deployables`, `MenuSystem`, and `WildOmissionCore` for `FUniqueNetIdWrapper::ToString()`.
   - Fixed `C4996` deprecation warnings (`USkeletalMeshComponent::SetAnimInstanceClass` and `AActor::SetNetUpdateFrequency`).
4. **Headless Blender 5.2 Pipeline & Build Tooling:**
   - Added `Art/Palette.json` (24-color low-poly flat-shaded palette), `Art/Blender/export_fbx.py`, and `Art/Blender/generate_metal_ore_node.py`, producing `SM_MetalOreNode.blend` and `SM_MetalOreNode.fbx` (tracked via Git LFS).
   - Added `Tools/Build.bat` wrapper for CLI builds.

### How it was verified
- Full `WildOmissionEditor Win64 Development` build succeeded on UE 5.8.4 (`Result: Succeeded`, producing `Binaries/Win64/UnrealEditor-*.dll` for all 40 project modules).
- Verified headless Blender 5.2.3 LTS generation and FBX export (`Art/Export/SM_MetalOreNode.fbx`).

### Next
- Smoke-test editor/game runtime on `feature/p0-ue58-upgrade`, implement `UServerRules` (admin-configurable server toggles), and merge `feature/p0-ue58-upgrade` to `main` once verified stable.