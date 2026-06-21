# The Voided

A 2D metroidvania about a small shade born of god and void, made to unshackle
the void from its cage. **Prototype v0.2** — game-feel pass complete.

> *"You… who are you? No…. what are you? Are you one of them? Maybe you are…
> but you seem small….hmmmm…"*

This is a C + SDL2 prototype. All art is hand-authored as 32x32 grid sprites
in C headers — no PNGs, no asset pipeline. Each enemy lives in its own file
under `art/`. A Python preview script renders those C grids to PNG so you can
see what the sprites look like without running the game.

---

## v0.2 changelog (this update)

Game-feel pass + architectural refactor:

**Bug fixes (from code review):**
- Fixed `combat.c` pogo-state-after-clear bug (now captures kind before clearing).
- Animation timing now driven by per-entity `anim_timer` incremented in update, not `SDL_GetTicks()` wall-clock.
- Added **ledge detection** to the Crawler (`ground_ahead()` probe offset by facing).
- World draw is now two-layer: parallax `bgwall` scrolls at 0.6× camera speed; solids draw on top.
- All user-visible strings wrapped in `_("...")` macro (i18n hook, English-only for now).

**Game feel:**
- **Hit-stop**: world freezes 3 frames on hit, 8 on parry, 14 on perfect parry, 20 on kill.
- **Screen shake**: trauma² model. Decays exponentially per tick. Trauma added on hit/parry/pogo/hurt/kill/sprint/anchor.
- **Particles**: 256-slot pool with 5 kinds (spark, dust, void, blood, shard). Spawned on every combat event.
- **Parry drama**: camera zooms to 1.08× on clash (1.15× on perfect), centered on the clash point; eases back when freeze ends.
- **Perfect parry window**: first 4 ticks of the 20-tick parry freeze = "perfect" → 4 dmg (vs 2 normal), kenotita refund, free void burst particles.
- **Procedural SFX**: 9 sounds synthesized at runtime from sine + noise (slash, parry chime, perfect, pogo thud, hurt, sprint void hum, kill, menu move/select). No external samples. Falls back to silent if audio device unavailable.

**Enemy vtable refactor:**
- Generic `Enemy` struct + `EnemyVTable` (init/update/draw/on_hit/on_parried/on_death/attack_hitbox/hurtbox/sprite/sprite_flip).
- Crawler moved to `enemy_crawler.c`. Adding a new enemy type is now: write a new `.c`, declare its `xxx_init(Enemy*, Vec2)`, register the vtable.
- Pool raised from 32 → 64. Bosses will use the same pool with `ES_CUSTOM+` state values.

**Mindless Sprint (central verb):**
- `kenotita` resource (max 100, regen 0.15/tick, cost 30/sprint).
- Two triggers: **manual** (LShift / B on gamepad) costs kenotita; **panic-auto** is free but fires when 3+ enemies are within aggro range, locking control for 1.5s.
- 5-segment **tentacle IK chain** (FABRIK-style distance constraint + sine thrash) trails behind the player. Drawn as decaying black circles.
- I-frames for the first 6 ticks of sprint.
- Tentacles yank small enemies into your wake; deal contact damage every 10 ticks.
- World background darkens during sprint (visual absorption cue).

**Scene stack:**
- TITLE → INTRO (typewriter dialogue of the opening "You… who are you?") → WORLD → (pause overlay) → back to TITLE on quit.
- SceneVTable has init/update/draw/free/pause/resume. Stack supports push/pop/replace.

**Hot reload (Linux/inotify):**
- Watches `tunables.txt` and `levels/`. Edit a tunable while the game runs, save, see it live.
- `Tunables` struct mirrors the most-tweaked `config.h` values (gravity, parry windows, hit-stop, shake, sprint speed, kenotita). Loaded at startup, reloaded on file change.

**Gamepad support:**
- `SDL_GameController` polled each tick. D-pad + left stick = move, A = jump, X = attack, Y = pogo, B/R Shoulder = sprint, Start = pause, Back = debug.

---

## v0.1 (initial prototype) — preserved below

Core mechanics in this prototype:
- Player movement: walk, run, jump (coyote time, jump buffering, jump cut)
- Voidscrew combat: horizontal slash + downward pogo (bounce)
- Parry system: 20-frame freeze when player+enemy attacks clash
- Enemy: Void Crawler with patrol AI + parryable claw telegraph
- Level: ASCII tilemap (levels/town.txt) for Ash-Foot Town biome
- Camera: smooth follow with lookahead + deadzone
- Debug overlay: F1 for hitboxes

Each enemy gets its own C header file under `art/`. A Python preview script
(read-only) renders those grids to a PNG for visual verification.

---

## Building

Requirements: `gcc`, `make`, SDL2 (dev headers + runtime lib).

```sh
sudo apt install libsdl2-dev     # Debian/Ubuntu
brew install sdl2                # macOS
make && ./build/voided
```

### Headless smoke test / screenshot

```sh
./build/voided --shot 180 /tmp/shot.bmp --auto 180 --debug --skip-title
```

Flags:
- `--shot N PATH`  Capture frame N to PATH (BMP), then quit.
- `--auto N`       Simulate holding RIGHT for N ticks (headless demo).
- `--debug`        Force-enable hitbox overlay.
- `--skip-title`   Jump straight to WORLD scene (bypass title + intro).

### Live-tweak tunables while playing

Edit `tunables.txt` and save — inotify reloads it instantly.

### Preview the grid art as PNG

```sh
python3 /home/z/my-project/scripts/preview_art.py
# View: /home/z/my-project/download/art_preview.png
```

---

## Project Layout

```
the-voided/
├── art/                         # Hand-authored grid sprites (C headers)
│   ├── palette.h                # 16-color ABGR palette
│   ├── player_sprite.h          # Player: idle/walk1/walk2/jump/attack/pogo
│   ├── voidscrew_sprite.h       # The Voidscrew blade
│   ├── enemy_void_crawler.h     # Enemy: Void Crawler (one enemy per file)
│   └── town_tiles.h             # Tile set: stone, moss, ground, bgwall
├── levels/
│   └── town.txt                 # ASCII tilemap (Ash-Foot Town platforming)
├── tunables.txt                 # Live-tweakable gameplay values (hot-reloaded)
├── src/
│   ├── types.h                  # Vec2, Rect, IVec2, IRect, Color + helpers
│   ├── config.h                 # Compile-time tunables (array sizes, etc.)
│   ├── tunables.{h,c}           # Runtime tunables loaded from tunables.txt
│   ├── i18n.h                   # _() macro for localization hooks
│   ├── input.{h,c}              # Keyboard + gamepad -> button snapshot
│   ├── level.{h,c}              # ASCII level loader + tile queries
│   ├── render.{h,c}             # SDL2 renderer + sprite atlas baker
│   ├── player.{h,c}             # Player state machine + physics + attacks
│   ├── enemy.{h,c}              # Enemy framework + vtable (generic)
│   ├── enemy_crawler.c          # Crawler impl using vtable
│   ├── combat.{h,c}             # Parry clash + perfect parry + hit-stop
│   ├── camera.{h,c}             # Smooth follow + trauma² shake + zoom
│   ├── particles.{h,c}          # 256-slot particle pool (5 kinds)
│   ├── audio.{h,c}              # Procedural SFX (sine + noise synthesis)
│   ├── tentacles.{h,c}          # Mindless Sprint 5-segment IK tentacles
│   ├── hotreload.{h,c}          # inotify watcher for tunables/levels
│   ├── scene.{h,c}              # Scene stack with vtable
│   ├── scenes_builtin.c         # TITLE / INTRO / WORLD / PAUSE scenes
│   ├── world.{h,c}              # World update + draw (orchestrates everything)
│   └── main.c                   # Entry point, fixed-timestep loop
├── Makefile
└── README.md
```

---

## Controls

| Key          | Action                       |
|--------------|------------------------------|
| ←/→          | Move                         |
| ↑/↓          | Look / aim pogo              |
| Z            | Jump (tap = cut, hold = full)|
| X            | Voidscrew slash              |
| ↓ + X        | Pogo (downward slash, bounces off terrain/enemies) |
| C            | Alt pogo trigger             |
| Left Shift   | Mindless Sprint (costs kenotita) |
| P            | Pause                        |
| F1           | Toggle debug hitboxes        |
| F2           | Toggle slow-mo (1/4 speed)   |
| Esc          | Quit / back to title         |

Gamepad: A=jump, X=attack, Y=pogo, B/RS=sprint, Start=pause, Back=debug.

---

## Tweakable Tunables

Two layers:

1. **`config.h`** — compile-time (array sizes, enum-like). Recompile to change.
2. **`tunables.txt`** — runtime, hot-reloaded via inotify. Edit while playing.

Key runtime tunables:

| Key                    | Default | Meaning                              |
|------------------------|---------|--------------------------------------|
| `gravity`              | 0.55    | px/tick²                             |
| `move_speed`           | 2.4     | px/tick max horizontal               |
| `jump_velocity`        | -9.5    | initial jump impulse                 |
| `coyote_time`          | 6       | ticks of post-ledge jump grace       |
| `jump_buffer`          | 6       | ticks of pre-land jump grace         |
| `pogo_velocity`        | -11.0   | bounce impulse off pogo              |
| `parry_freeze_ticks`   | 20      | parry clash freeze window            |
| `parry_perfect_window` | 4       | first N ticks = perfect parry        |
| `parry_stun_ticks`     | 40      | enemy stun after losing a parry      |
| `hitstop_hit`          | 3       | frames frozen on hit                 |
| `hitstop_parry`        | 8       | frames frozen on parry resolve       |
| `hitstop_perfect`      | 14      | frames frozen on perfect parry       |
| `hitstop_kill`         | 20      | frames frozen on enemy kill          |
| `shake_max_pixels`     | 8.0     | max camera shake offset              |
| `shake_decay`          | 0.86    | per-tick trauma multiplier           |
| `shake_trauma_hit`     | 0.20    | trauma added on hit                  |
| `shake_trauma_parry`   | 0.45    | trauma added on parry                |
| `shake_trauma_perfect` | 0.75    | trauma added on perfect parry        |
| `sprint_speed`         | 8.0     | mindless sprint horizontal speed     |
| `sprint_duration_ticks`| 30      | length of sprint (0.5s @ 60Hz)       |
| `kenotita_regen`       | 0.15    | kenotita regen per tick              |
| `kenotita_sprint_cost` | 30      | kenotita cost per manual sprint      |

---

## Adding a New Enemy (workflow)

1. Create `art/enemy_<name>.h` with grid sprites (one frame per row array).
2. Register new `SPRITE_*` IDs in `src/render.h` (extend the enum + `SPRITE_COUNT`).
3. Add the sprites to `sprite_defs[]` in `src/render.c`.
4. Create `src/enemy_<name>.c` following the `enemy_crawler.c` template:
   - Define type-specific data struct (fits in `Enemy.data.bytes[128]`).
   - Implement all 10 vtable functions.
   - Expose `<name>_init(Enemy*, Vec2)`.
5. To spawn from a level: add a new marker char in `levels/*.txt`, switch on it
   in `world_init()`.

---

## What's Next (roadmap toward the full vision)

- [ ] Save system (`save.dat` + `world.dat`)
- [ ] Sibling boss: **Moth** (chase sequence in Solar Canopy; grants Cosmic Drift)
- [ ] Sibling boss: **Lost** (dialogue puzzle; grants Phase-Dash)
- [ ] Sibling boss: **Keres** (attacks on sight; grants Void Burst)
- [ ] **Greatest of Fates**: time-rewind boss that records and replays the player's past self
- [ ] **Gaia**: non-hostile ecosystem boss; killing her requires destroying each root; ash forest rewards you for the atrocity
- [ ] Tether / anchor shatter system (world darkens as anchors break)
- [ ] Biomes: Crystal Spires, Solar Canopy, Abyssal Fracture, Singularity
- [ ] Voidstruck ending: Beyond One absorption sequence
- [ ] Font + text rendering for dialogue/UI (currently placeholder rects)
- [ ] Hot-reload levels mid-play (preserve player state across reload)
- [ ] Localized string tables (i18n)
- [ ] In-game tunables editor (debug overlay sliders)

---

## License

Prototype source is yours to read, fork, and modify. The setting, names, and
lore ("The Voided", "Voidscrew", "Moth", "Lost", "Keres", "Beyond One",
"Greatest of Fates", "Gaia", "Voidstruck") are original to this project.
