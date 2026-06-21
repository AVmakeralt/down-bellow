# The Voided

A 2D metroidvania about a small shade born of god and void, made to unshackle
the void from its cage. **Prototype v0.1** — core movement and combat loop.

> *"You… who are you? No…. what are you? Are you one of them? Maybe you are…
> but you seem small….hmmmm…"*

This is a C + SDL2 prototype. All art is hand-authored as 32x32 grid sprites
in C headers — no PNGs, no asset pipeline. Each enemy lives in its own file
under `art/`. A Python preview script renders those C grids to PNG so you can
see what the sprites look like without running the game.

---

## Lore Snapshot

You are the smallest thing in the setting — a grey-cloaked shade with two
glowing eyes and one disfigured horn, born of the void from the death of the
god of order. Your siblings are **Moth**, **Lost**, and **Keres**. Your
weapon is the **Voidscrew**. The void must be freed; the gods must fall.

Cosmology (4 layers, climbing upward):
1. **The Light** — pure divinity, ascended beings.
2. **The Normal World** — mortals, ruins, the Ash-Foot Town.
3. **The Surface of the Void** — reality tearing, reverse gravity pockets.
4. **The Void Core** — the heart of the shade lord; nothing can harm it;
   whatever enters is erased.

Higher beings are beyond death and beyond the normal realm. Sibling bosses
(Moth, Lost, Keres) gate the layers. Optional bosses (Greatest of Fates,
Gaia) unlock the Voidstruck ending — absorbing the **Beyond One**, the
living embodiment of everything.

---

## Prototype Scope (v0.1)

- Player: walk, run, jump (with coyote time + jump buffering + jump cut)
- Voidscrew attack: horizontal slash + downward pogo (pogo bounces you off
  enemies and terrain)
- **Parry system**: when the player's voidslash hitbox overlaps an enemy's
  attack hitbox, both freeze for 20 frames. Then:
  - If the player's hitbox still overlaps the enemy body → player wins
    (enemy stunned, takes 2 damage).
  - If only the enemy's hitbox overlaps the player body → enemy wins
    (player takes damage).
  - Otherwise both attacks fizzle.
- Enemy: **Void Crawler** — patrols, lunges when player is in range, claws
  are parryable. 3 HP.
- Level: a small Ash-Foot Town tilemap with platforms, walls, and one
  enemy encounter.
- Camera: smooth follow with lookahead + deadzone.
- Debug overlay: F1 to toggle hitbox visualization.

---

## Building

Requirements:
- `gcc`, `make`
- SDL2 (dev headers + runtime lib)

```sh
# Debian/Ubuntu:
sudo apt install libsdl2-dev

# macOS (homebrew):
brew install sdl2

# Then:
make
./build/voided
```

### Headless smoke test / screenshot

```sh
# Run the game for N frames, save a BMP screenshot, then quit:
./build/voided --shot 180 /tmp/shot.bmp --auto 180 --debug
```

Flags:
- `--shot N PATH`  Capture frame N to PATH (BMP), then quit.
- `--auto N`       Simulate holding RIGHT for N ticks (headless demo).
- `--debug`        Force-enable hitbox overlay.

### Preview the grid art as PNG

```sh
# Render every sprite to a contact sheet PNG (read-only, does not modify C):
python3 /home/z/my-project/scripts/preview_art.py
# View:
# /home/z/my-project/download/art_preview.png
```

---

## Project Layout

```
the-voided/
├── art/                         # Hand-authored grid sprites (C headers)
│   ├── palette.h                # Shared 16-color palette
│   ├── player_sprite.h          # Player: idle/walk1/walk2/jump/attack/pogo
│   ├── voidscrew_sprite.h       # The Voidscrew blade
│   ├── enemy_void_crawler.h     # Enemy: Void Crawler (one enemy per file)
│   └── town_tiles.h             # Tile set: stone, moss, ground, bgwall
├── levels/
│   └── town.txt                 # ASCII tilemap (Ash-Foot Town platforming)
├── src/
│   ├── types.h                  # Vec2, Rect, IVec2, IRect + helpers
│   ├── config.h                 # All gameplay tunables (tweak freely)
│   ├── input.{h,c}              # Keyboard -> button snapshot
│   ├── level.{h,c}              # ASCII level loader + tile queries
│   ├── render.{h,c}             # SDL2 renderer + sprite atlas baker
│   ├── player.{h,c}             # Player state machine + physics + attacks
│   ├── enemy.{h,c}              # Void Crawler AI + parryable telegraph
│   ├── combat.{h,c}             # Parry clash resolution
│   ├── camera.{h,c}             # Smooth follow camera
│   ├── world.{h,c}              # World update + draw
│   └── main.c                   # Entry point, fixed-timestep loop
├── Makefile
└── README.md
```

---

## Controls

| Key     | Action                       |
|---------|------------------------------|
| ←/→     | Move                         |
| ↑/↓     | Look / aim pogo              |
| Z       | Jump (tap = cut, hold = full)|
| X       | Voidscrew slash              |
| ↓ + X   | Pogo (downward slash, bounces off terrain/enemies) |
| C       | Alt pogo trigger             |
| F1      | Toggle debug hitboxes        |
| F2      | Toggle slow-mo (1/4 speed)   |
| Esc     | Quit                         |

---

## Tweakable Tunables

All gameplay numbers live in `src/config.h`. Common ones:

| Define              | Default | Meaning                              |
|---------------------|---------|--------------------------------------|
| `GRAVITY`           | 0.55    | px/tick²                             |
| `MOVE_SPEED`        | 2.4     | px/tick max horizontal               |
| `JUMP_VELOCITY`     | -9.5    | initial jump impulse                 |
| `COYOTE_TIME`       | 6       | ticks of post-ledge jump grace       |
| `JUMP_BUFFER`       | 6       | ticks of pre-land jump grace         |
| `POGO_VELOCITY`     | -11.0   | bounce impulse off pogo              |
| `PARRY_FREEZE_TICKS`| 20      | parry clash freeze window            |
| `PARRY_STUN_TICKS`  | 40      | enemy stun after losing a parry      |
| `CRAWLER_HP`        | 3       | hits to kill a void crawler          |

---

## What's Next (roadmap toward the full vision)

- [ ] More enemies (each in its own file under `art/`)
- [ ] Sibling boss: **Moth** (grants cosmic flight on absorb)
- [ ] Sibling boss: **Lost** (narrative checkpoint)
- [ ] Sibling boss: **Keres** (grants void burst on absorb)
- [ ] **Mindless Sprint** mechanic (void tentacles, i-frames, env destruction)
- [ ] Tether / anchor shatter system (world darkens as anchors break)
- [ ] Biomes: Crystal Spires, Solar Canopy, Abyssal Fracture, Singularity
- [ ] Optional bosses: Greatest of Fates (time rewind), Gaia (tragic kill)
- [ ] Voidstruck ending (absorb the Beyond One)
- [ ] Audio: void hum, slash SFX, parry chime
- [ ] Hot-reload: inotify watcher for `levels/*.txt` and `art/*.h`

---

## License

Prototype source is yours to read, fork, and modify. The setting, names, and
lore ("The Voided", "Voidscrew", "Moth", "Lost", "Keres", "Beyond One",
"Greatest of Fates", "Gaia", "Voidstruck") are original to this project.
