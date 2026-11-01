# Generation 4 and 5 moves and Pokémon: feasibility and plan

Status: **plan only, nothing of it is implemented.** The randomizer rules
(random learnsets and evolutions, difficulty, boss and strategy teams,
doubles; `3ds_port/docs/seeded_randomizer.md`) use only what Emerald has:
386 species, 354 moves, 77 abilities. This document records what adding
later generations' moves (Trick Room, Stealth Rock, U-turn, Close Combat...)
and the Sinnoh and Unova Pokémon would take, the options, and a staged path.

## Why it is not a small change

Emerald 3DS runs the pinned **pret/pokeemerald** (`upstream.lock`) with its
changes as patches, and it never ships game content: the data pack is
rebuilt from the player's own ROM through recipes (`docs/ASSET_PIPELINE.md`).
Everything below follows from those two facts.

| Area | What pokeemerald fixes today | What Gen 4/5 needs |
|---|---|---|
| Species ids | `NUM_SPECIES` 412 (`SPECIES_EGG`), 386 National Pokédex numbers; tables sized by it (`gSpeciesInfo`, pics, icons, cries, learnsets, evolutions, dex text, footprints) | 107 (Gen 4) + 156 (Gen 5) species: every table grows and new data for each |
| Pokédex flags in the save | `owned`/`seen`/`seen1`/`seen2` are `NUM_DEX_FLAG_BYTES` (52) in SaveBlock1/2 | 649 species need 82 bytes each: **the save layout changes** (new fields elsewhere, a migration, or a separate port-side file) |
| Moves | `MOVES_COUNT` 355; learnset entries keep the move in 9 bits (`LEVEL_UP_MOVE_ID` 0x1FF, max 511) | Gen 4 brings the count to 467 (fits), Gen 5 to 559 (**does not fit**: a learnset format change) |
| Move effects | ~214 `EFFECT_*` with scripts in `data/battle_scripts_1.s` | Trick Room, Stealth Rock, U-turn/Volt Switch, Roost, Tailwind, Gravity, Toxic Spikes, entry hazards, pivoting... each a new effect, battle script and often new battle state (field timers, side status bits) |
| Abilities | 78, one `u8` per slot, two slots | Gen 4/5 add 87 (Technician, Levitate-likes, Sturdy changes, Mold Breaker...): new hooks across the battle engine |
| Physical/special | per type in Gen 3 (the port's optional `move_split`, patch 0046, already adds a per-move split) | per move: already partly there |
| Graphics, cries | from the ROM, through the recipe | not in an Emerald ROM at all: front/back pics, shiny palettes, icons, footprints, cries must come from elsewhere at build time |
| Text | names and descriptions in English source, other languages staged from their ROMs (`tools/locales/*.json.gz`) | new names/descriptions for every language, hand-written (as for the port's own texts) |
| AI | `data/battle_ai_scripts.s` knows Gen 3 effects | scores for every new effect |

## Option A: rebase onto rh-hideout/pokeemerald-expansion

[pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion)
already has species, moves, abilities and items up to Generation 9, the
physical/special split, a rewritten battle engine and AI, and configurable
generation behaviour.

- **Pros:** the battle engine work (hundreds of effects and abilities) is
  done and tested by a large community; Trick Room, Stealth Rock, U-turn and
  every later move exist; learnsets and evolutions of later generations are
  in its data; the randomizer pools would simply use its tables.
- **Cons, and why it is a project of its own:**
  - The port's **80-odd patches** are written against pret's `battle_main.c`,
    `pokemon.c`, `battle_setup.c`, the options, the Pokédex, the text
    layout of every language. Expansion has rewritten most of those files;
    almost every patch would have to be redone by hand, not rebased.
  - The **recipe** (`tools/gen_recipe.py`) maps the game data to the ROM
    through the ELF of the *unpatched pinned pokeemerald*, which must rebuild
    the supported ROM byte for byte (`upstream.lock`, `[agbcc]`). Expansion
    does not build the original ROM, its tables have different layouts and
    contents, so a large share of the data pack would become literals or new
    `R`-style remaps, and the release audit's "no ROM bytes in literals" rule
    has to be re-proven.
  - The **localization manifests** (Spanish, French, Italian, German,
    Japanese, Brazilian Portuguese) are fingerprints of patched pret sources:
    all of them would have to be regenerated.
  - **Saves** are not compatible between pret's and expansion's layouts.
  - Expansion's graphics for new species are in its repository; they are
    Nintendo's artwork (see Licensing). The port could not ship them inside
    its releases, and fetching them from expansion's repository at build
    time would be the same pattern as below, with the same caveats.
- **Estimate:** weeks to months of work by someone who knows both trees,
  plus hardware testing of the whole game again.

## Option B: a limited set added to pret pokeemerald, in patches

Add a curated set rather than everything:

1. **Gen 4/5 moves with Gen 3 effects** (no engine work): Close Combat-like
   moves need one new effect (lower Def and Sp. Def), but many reuse Gen 3
   effects as they are: Brave Bird / Flare Blitz / Wood Hammer (recoil, as
   Double-Edge), Shadow Claw / Night Slash / Psycho Cut / Cross Poison /
   Stone Edge (high critical ratio), X-Scissor, Power Whip, Seed Bomb,
   Zen Headbutt / Dark Pulse / Air Slash (flinch chance), Aura Sphere /
   Magnet Bomb (never miss), Earth Power / Energy Ball / Bug Buzz / Focus
   Blast / Flash Cannon (lower Sp. Def), Nasty Plot (Sp. Atk +2), Ice Shard /
   Shadow Sneak / Bullet Punch / Aqua Jet / Vacuum Wave (priority),
   Poison Jab / Gunk Shot (poison chance), Discharge / Lava Plume (both
   foes, burn or paralysis chance), Draco Meteor / Leaf Storm (as Overheat),
   Hone Claws, Work Up, Coil-like boosts. Each needs: an id after
   `MOVE_PSYCHO_BOOST`, a `gBattleMoves` row, a name (12 characters), a
   description, contest data, an animation (an existing one reused, e.g.
   Close Combat with Cross Chop's), and the move tutor/TM tables if wanted.
   Roughly 30-40 moves fit under the 511 limit of learnset entries.
2. **A few new effects** for the iconic moves: Stealth Rock (a side status
   bit and a switch-in hook in `battle_util.c`), U-turn/Volt Switch (the
   Baton Pass switch path without passing stats), Trick Room (a field timer
   and the turn order in `GetWhoStrikesFirst`), Tailwind, Roost, Toxic
   Spikes. Each: an effect, a battle script, its messages, AI scores in
   `data/battle_ai_scripts.s`. This is battle-engine work that must be
   tested in battle.
3. **Species**: a set of Gen 4/5 Pokémon (for example evolutions of Gen 1-3
   lines: Magnezone, Rhyperior, Togekiss, Weavile, Honchkrow, Mismagius,
   Gliscor, Mamoswine, Porygon-Z, Gallade, Froslass, Dusknoir, Electivire,
   Magmortar, Leafeon, Glaceon, Lucario, Garchomp line...). For each: an id
   (new ids after Chimecho's, before the Egg, would move `SPECIES_EGG` and
   Unown's forms: every table and the save's box data use these ids, so the
   ids must go **after** the existing ones and `NUM_SPECIES` must stop
   meaning "the Egg"), base stats, learnset, evolutions, dex data, and a
   **National Pokédex number above 386**, which grows the save's Pokédex
   flags (a save migration, or the new flags in a port-side file next to
   `settings.txt`).
4. **Graphics and cries at build time**, never in the repository or a
   release, following the follower pattern (`builder/emerald3ds_builder/
   followers.py`, `followers_sources.py`): the builder downloads files from
   a **pinned commit** of a decompilation repository, checks **size and
   SHA-256** of each, caches them, and converts them into the pack; the
   player's own machine does it, as for the HeartGold/SoulSilver follower
   sprites (fetched from a pinned fork of pret/pokeheartgold). For Gen 4,
   pret/pokeplatinum or pret/pokeheartgold carry the battle sprites, icons
   and cries in their asset trees; Gen 5 has no comparable decompilation, so
   its sprites would come from another pinned source. The converter would
   have to make 64×64 4bpp front/back pics, palettes (normal and shiny),
   32×32 icons with an icon palette index, footprints and cries in the
   game's sample format. The game side needs the pic tables to accept ids
   whose data come from the pack only (`3ds_assets.c` already resolves
   INCBIN stubs to pack files).
5. **Randomizer pools**: once the species and moves exist, the rule tables
   of `src/3ds_randomizer_ext.c` are built from `gSpeciesInfo` and
   `gBattleMoves`, so the pools include them automatically; the species
   permutation (`CTR_RANDOMIZER_NATIONAL_COUNT` 386) would grow to the new
   National numbers. Strategy teams (`src/3ds_strategy_teams.h`) could then
   use Trick Room, Stealth Rock or U-turn and the new species.

## Licensing and attribution

- The port's code is CC BY-NC-SA 4.0 (`LICENSE-PORT.md`); pret's
  decompilations and pokeemerald-expansion are community code; the game
  assets (sprites, cries, text) are Nintendo / Creatures / GAME FREAK's
  copyright in any of those repositories. The project's rule stays:
  **no asset in the repository or a release** (`CONTRIBUTING.md`, *Never
  commit*; `tools/release_audit.py`). Build-time downloads keep the data on
  the player's machine, as the follower sprites do, and the README and
  installer must say that the builder downloads them.
- Each pinned source gets a provenance entry (`docs/PROVENANCE.md`) and the
  credits (`NOTICE.md`) name the projects whose data or code is used
  (pokeemerald-expansion's data tables, if taken, with its contributors).
- Base stats, learnsets and move numbers are facts of the games; text
  (names, descriptions) should be written for the port, not copied from a
  later game's ROM.

## Save compatibility

- Moves above 354 in a party or box Pokémon: a save holding one would show
  garbage in a build without them. Acceptable only with a clear warning, or
  with a guard that refuses such saves in older builds.
- New species ids and Pokédex numbers: the same, and the Pokédex flag arrays
  must grow (see above). Saves made before the change stay loadable if the
  new flags live outside the original structures.
- The randomizer settings stay in `settings.txt`; nothing new in the save.

## Suggested order

1. Gen 4/5 moves that reuse Gen 3 effects (step 1), with animations
   borrowed from similar moves, English names first. Learnset randomizer and
   strategy teams pick them up. Host tests for the tables; hardware test of
   battles using each.
2. The handful of new effects (step 2), one patch each, with AI scores.
3. The save extension for new Pokédex numbers, then species with build-time
   sprites (steps 3-4), starting with Gen 4 evolutions of existing lines.
4. Re-evaluate Option A only if the goal becomes "everything up to Gen 9".
