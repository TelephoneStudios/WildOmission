# Wild Omission → Rust-Styled Survival: Development Roadmap

_Draft v1 – 2026-11-04. Based on: Steam store page, discussions, guides and patch notes; the local source fork; and `Rust to Wild Omission.pdf`._

---

## 1. Where the game is today

**Wild Omission** (Telephone Studios, solo dev "Larch"; free-to-play + open source since Apr 2025; currently Beta 1.3.3, Very Positive, 88% of 85 reviews).
Infinite procedural world, severe weather (F1–F5 tornadoes, storms, lightning), solo / co-op / PvP, Steam Workshop worlds, Creative Mode DLC.

### History since launch (from Steam patch notes)
| Date | Version | Highlights |
|---|---|---|
| Nov 2023 | EA launch | Core survival, infinite worlds |
| Feb 2024 | Alpha 1.3.x | Pooka Pookas, Lurkers, tougher zombies, signs, many QoL patches |
| Mar–Apr 2024 | 1.4.0 | World **structures**, barrels, advanced crates |
| Jun 2024 | 1.5.0 | **Weapon Madness**: rocket launcher, assault/semi-auto rifles, melee, decorations |
| Jun 2024 | 1.6.0 | Creative Mode, F1 item spawn menu |
| Sep 2024 – Mar 2025 | – | Dev hiatus ("What's Next?" plan) |
| Apr 2025 | Beta 1.0.0 | F2P + open source, F1–F5 tornadoes |
| Jun 2026 | 1.1.0 – 1.2.0 | Camera bob, **dedicated server discontinued**, foundations must touch ground, Creative DLC overhaul |
| Jul 2026 | 1.3.0 – 1.3.3 | Workshop integration, new save format, structure overhaul |

### Community feedback themes (Discussions)
- **Content shortage** is the #1 complaint (the dev agrees: clothing, tools, weapons, "an end goal").
- Day/night cycle too fast, torches burn too quickly, night sometimes pitch-black after a crash.
- Crash reports, gold/items falling through the world, "Abandoned?" worries (dev hiatus).
- Dev's own **"Must Have" list**: Clothing, Better Weather, Item Skins, Better Enemy AI, Foundation Steps, Weather items (rain collector, radar).

### Guides
Only three exist: *How To Play*, *Getting Started with Open Source*, *Using Creative Mode*. Nothing on advanced systems, so we should add docs as we build.

---

## 2. Art style (must be preserved)

Seen in store screenshots (`reference/`):
- **Flat-shaded low-poly, no textures.** Faceted solids, single-colour materials, no PBR detail maps.
- Warm, saturated palette; muted blue-green nights with star-field sky; strong dynamic lighting does the mood work.
- Chunky, simplified props (lamps, tables, potted saplings) with ~8-sided silhouettes; trees are stacked faceted blobs.
- Minimal HUD: pale rounded hotbar squares with simple white icons; thin coloured bars (green health, blue thirst, orange hunger) bottom-right; small crosshair dot.
- Log-cabin walls = horizontal stacked bands.

**Asset rules for all new content (Blender):**
1. Flat shading, ≤ ~1.5k tris per prop (weapons ≤ 3k), no UV textures: colour via a shared palette atlas or per-material colours.
2. Same ~24-colour palette file (`Art/Palette.json` → Blender + UE material), so new items match.
3. Hard edges, silhouette first. Rust items are reinterpreted, not copied (AK → blocky assault rifle like the existing 1.5.0 one).
4. Export FBX at 1 UU = 1 cm, forward -Y / up Z; naming `SM_`, `SK_`, `T_`, `M_`.
5. Icons rendered from Blender with a fixed orthographic camera, transparent bg, white-ish tint for HUD consistency.

---

## 3. Gap analysis: Rust plan vs. current code

Source: UE C++ project with ~40 modules, 644 source files. Already present:

| Rust plan item | Existing in code | Gap |
|---|---|---|
| Harvest nodes, tool tiers | `HarvestableNode/Resource/Component`, `EToolType` | Add Metal Ore, HQM, sulfur, charcoal, animal fat; yield scaling per tool tier |
| Building tiers (Twig→Wood→Stone→Metal→Armored) | `ABuildingBlock::Upgrade()`, `UpgradedTier`, `MaterialType` | No twig, HP/decay model, stability |
| Tool Cupboard | `AToolCupboard` (authorization + `Range`) | **No upkeep/decay**; no build-block radius restriction check; no 24 h tax |
| Code locks / key locks | `ALock` (code, auth), `KeypadWidget` | Key lock variant |
| Furnaces | `ItemSmelterBase` | Small/large furnace, refineries |
| Weapons | Bow, Firearm (semi/full/manual/shotgun), Rocket | Crossbow, handmade revolver, spear, ammo types, bolt rifle, SMG, C4, recoil/ballistics |
| Clothing | `Clothing` module | Insulation / radiation / armor stats; hazmat |
| Vitals | Health, Thirst, Hunger | **Temperature, radiation, bleeding** |
| Weather | Storm, Tornado, Lightning, `WeatherManager`, `DamagedByWind` | Acid rain, blizzard/sandstorm, cold mechanic, wind-damaged structures (partial hook exists) |
| AI | Behavior trees, `WildOmissionAI`, Monsters | Cover, flanking, grenades, healing, sound perception; heli/patrols |
| Workbenches / research | Crafting module (recipes, no gates) | T1–T3 workbenches, blueprints |
| Electricity | – | Entire system |
| Monuments | `Structure` (world-gen structures, 1.4.0) | Monuments with loot + guards, rad towns |
| Crops | `Sapling`, `Seedling` | Hemp/corn/pumpkin, genetics |
| Dedicated server | `DedicatedServer` module (dev discontinued, on 1.0.1) | Needed for Rust-like persistent PvP; revive |

> [!IMPORTANT]
> The Rust plan assumes a persistent, server-authoritative PvP world. The dev **discontinued the dedicated server in 1.1.0** and the save system was restructured in 1.3.0 for workshop. Reviving and testing dedicated-server support is an early dependency for Phases 2–3 (raiding, upkeep, offline decay).

---

## 4. Tooling & environment

| Need | Status |
|---|---|
| Unreal Engine **5.8.4** (`C:\Program Files\Epic Games\UE_5.8`) | ✅ Installed |
| Project targets UE **5.3** → upgraded to **5.8.4** (`feature/p0-ue58-upgrade`) | ✅ Built & Linked |
| Visual Studio 2022 Build Tools + .NET Framework 4.8.1 SDK | ✅ Installed |
| Blender 5.2 (`C:\Program Files\Blender Foundation\Blender 5.2`) | ✅ Installed; scripted via `blender -b -P` |
| git / gh / git-lfs | ✅ (LFS not configured in repo: 550 MB of `.uasset` committed as plain binaries; see Phase 0) |
| Python 3.14 | ✅ for tooling scripts |
| Remote | `origin` = `chasejltl/WildOmission` (push), `upstream` = `TelephoneStudios/WildOmission` (pull only) |

### Planned repo additions
- `Art/Blender/` (`.blend` sources + `export_fbx.py` batch exporter + `Palette.json`)
- `Docs/` (design docs, this roadmap mirrored, item tables)
- `Tools/` (item-table → DataAsset generator, automated tests runner)
- Local `Build.bat` / automation test script.

---

## 5. Git workflow

- **Feature branches** off `main`: `feature/<phase>-<topic>`, e.g. `feature/p1-metal-ore-node`. One topic per branch, PR into `main` on the fork.
- **Small, atomic commits** (target 5–20 per feature), imperative subject ≤ 72 chars, body explains why. Conventional prefixes: `feat:`, `fix:`, `refactor:`, `art:`, `docs:`, `chore:`, `test:`.
  - e.g. `feat(gathering): add tool tier multiplier`, then `art: add metal ore node mesh`, then `feat(world): spawn metal ore in rocky biome`, then `test: yield scaling by tool tier`.
- Code and binary assets are committed separately; never mix a refactor with a feature.
- Never push to `upstream`. Periodically `git fetch upstream` and merge into `main` to track the original dev.
- Version bumps follow the dev's convention (`Beta 1.x.y`).

---

## 6. Phased roadmap

Estimates are "focused work sessions" (~half a day) and are rough.

### Phase 0: Foundation (≈ 6–8 sessions)
1. Finish VS Build Tools install; verify `UnrealBuildTool` compiles.
2. `feature/p0-ue58-upgrade`: bump `EngineAssociation`, fix deprecated APIs module by module (one commit per module), regenerate project files, get a clean editor launch. _Risk: Steam subsystem, Volumetrics plugin, Workshop code._ Fallback: install UE 5.3.
3. Decide LFS: migrate `*.uasset`, `*.umap`, `*.fbx`, `*.wav` to git-lfs (needs your OK).
4. Smoke test: new world, craft, build, join as client, tornado.
5. Add `Art/` + Blender export pipeline; produce one proof-of-style prop and import it.
6. Add `Docs/` + coding conventions; basic automation test harness for gameplay maths.
7. Quick wins from community feedback (small commits): slower day/night, longer torch burn, night-visibility fix.

**Exit:** project opens/packages on 5.8, build script, first art asset in-engine.

### Phase 1: Core economy, primitives, basic combat (≈ 15–20 sessions)
- **Data-driven items:** item definitions in a table (`Tools/items.csv` → DataAssets).
- **Resources:** Metal Ore, HQM, sulfur nodes (rocky biomes); charcoal (burn wood in furnace), animal fat (animals). Blender: ore nodes, icons.
- **Tool tiers:** Stone → Metal hatchet/pickaxe. Extend `EToolType`/`HarvestableComponent` with tier multiplier and minimum-tier gates.
- **Primitive weapons:** spear variants, bow upgrades, Crossbow, Handmade revolver. Ammo: wood arrows, bone arrows, pistol cartridge.
- **Building tiers:** add **Twig** (decays, hand-tool breakable) → Wood → Stone → Sheet Metal → Armored. Per-tier HP and damage multipliers on `ABuildingBlock::Upgrade`. Includes dev's "foundation steps".
- **Verification:** automation tests for yield-vs-tool-tier and upgrade chain; manual checklist.

### Phase 2: Industrial crafting, electricity, base fortification (≈ 20–25 sessions)
- **Workbenches T1–T3** + research/blueprints; gate recipes in `UCraftingComponent`.
- **Deployables:** key locks, large wooden box, small & large furnace, refineries.
- **Tool Cupboard upkeep:** per-tier upkeep cost, 24 h (configurable) tax from TC inventory, decay when empty; privilege blocks foreign placement within `Range`. Server-authoritative; persisted via `SaveSystem`.
- **Electricity:** generator, switch, splitter, wire tool/UI, power-graph; consumers: auto-turret, shotgun trap, lights.
- **Dedicated server revival** + offline-raid-protection option.
- **Verification:** TC blocks enemy placement; upkeep drains correctly each cycle; power graph load/overload.

### Phase 3: Advanced ballistics, loot, monuments (≈ 25–30 sessions)
- **Firearms:** tune assault rifle, add bolt-action, SMG, C4; rocket launcher hooked to structure damage tiers. Recoil, bullet drop.
- **Medical / clothing:** syringes, bandages, bleeding, armor sets, **hazmat**, insulation on `Clothing`.
- **Vitals:** **radiation** (temperature in Phase 4).
- **Crops:** hemp/corn/pumpkin, gene model and crossbreeding on `Sapling`/`Seedling`.
- **Monuments & rad towns:** extend `Structure`/`ChunkManager` to place Airfield, Power Plant, Military Tunnels with locked crates, guards, radiation zones. Many new Blender kits in the flat low-poly style.
- **Verification:** radiation drains HP without hazmat; loot tables by monument tier; chunk-streaming performance.

### Phase 4: Weather physics & advanced AI (≈ 20–25 sessions)
- **Weather:** acid rain, sandstorm/blizzard, **temperature** vital + clothing insulation, campfire warmth, rain collectors / weather radar. Extend `AStorm`/`WeatherManager`.
- **Structural wind damage:** use `DamagedByWind` / `WindParameters` so tornadoes and high winds strip Twig/Wood panels.
- **AI:** Scientists via Behavior Trees (cover, blind fire, grenades, flank, heal) on `WildOmissionAI`/`Monsters`; EQS cover queries, AI hearing; patrol helicopter and heavy squads at monuments.
- **Verification:** AI line-of-sight, sound reaction, cover-seeking in test arenas.

### Phase 5: Polish & release (≈ 10 sessions)
Balance, performance (Unreal Insights), UI polish, Workshop compatibility, updated Steam guides/changelog, optional upstream PR proposal.

---

## 7. Risks
| Risk | Mitigation |
|---|---|
| UE 5.3 → 5.8 migration breaks C++ / BP assets | Dedicated branch, module-by-module fixes, fallback to 5.3 |
| Scope is enormous | Phase gates, data-driven items, vertical slices |
| Large binary assets bloat repo | git-lfs in Phase 0 |
| Save-format changes break worlds | Version field + migration in `SaveSystem` before persisting new data |
| Rust IP | Re-interpret designs in WO's art style; no copied models |
| Multiplayer authority bugs (upkeep/raiding) | Server-only logic, automated tests |
| Tone mismatch (WO is weather-focused; Rust is hardcore PvP) | Keep Rust systems as server-rule toggles where possible |

---

## 8. Decisions (v2, 2026-11-04)

| # | Decision |
|---|---|
| 1 | **git-lfs: yes.** Track binary assets going forward; history is *not* rewritten (no force-push). |
| 2 | **Build in phases, documenting as we go.** Every phase/feature gets a `Docs/DEVLOG.md` entry and, for systems, a `Docs/Design/<system>.md`. |
| 3 | **Rust systems are server toggles.** Upkeep, offline raid protection, PvP, decay, radiation, etc. are exposed as server rules (data asset + config + console commands) and designed to be driven by an **admin panel** and be easy to mod. See "Server rules framework" below. |
| 4 | **UE 5.8 upgrade stays on a feature branch** until stable (compiles, launches, smoke test passes). |
| 5 | **Start Phase 0 now.** |

### Server rules framework (new, Phase 0/2)
A single `UServerRules` settings object (replicated, saved with the world, overridable by `Server.ini`/command line) that every Rust-style system reads instead of hard-coding behaviour:
- `bUpkeepEnabled`, `UpkeepIntervalHours`, `UpkeepCostMultiplier`
- `bOfflineRaidProtection`, `bPvPEnabled`, `bBuildingDecayEnabled`, `ResourceGatherMultiplier`
- `bRadiationEnabled`, `bTemperatureEnabled`, `bAdvancedAIEnabled`, ...
- Admin panel (in-game UI for host/admins, plus console/RCON-style commands) edits these live; mods can register extra rules.
- Item/recipe tables are data-driven (CSV/DataAsset) so mods can add or rebalance content.

## 9. Findings during Phase 0 kickoff
- Upstream already has `Unreal_5-7_Migration` (10 commits, ~52 files of API fixes). The original dev **abandoned it on 2026-06-14** ("New Unreal Version is too unstable for Wild Omission" and rolled back to Beta 1.0.1). We can reuse its source fixes as a reference, but expect runtime/Blueprint instability on 5.8: this is the highest risk in Phase 0.
- Upstream `beta-1-4-0` contains 30 unreleased commits of **character customization** (colour wheel, custom player model). Out of scope; we stay on `main` (Beta 1.3.3) and may merge it later.
- Visual Studio 2022 Build Tools (C++ game workload) are installed.
