# Seeded species randomizer (optional, off by default)

The randomizer swaps species for others, the same way every time for a given
seed. It works on a clean, supported Emerald and does not accept hacked ROMs.
It never changes the ROM, a save, or Pokémon the player already has.

## What it changes

Each kind has its own setting (`settings.txt`, `0` or `1`, all `0` by default):

| key             | randomizes                                                        |
|-----------------|-------------------------------------------------------------------|
| `rand_wild`     | wild Pokémon from the map tables: grass, surfing, Rock Smash, fishing, Feebas, visible wild Pokémon |
| `rand_starters` | the three Pokémon in Birch's bag (label, picture, cry and the Pokémon given all match) |
| `rand_trainers` | the parties of ordinary trainers (gym leaders, rivals and the Elite Four included) |
| `rand_seed`     | the seed, 0..2147483647                                           |

One seed gives one permutation of the 386 real National Pokédex species, and
all three kinds use it. So with a given seed, a Zigzagoon is always the same
other species, whether it's wild or on a trainer's team. Internal IDs that
aren't real species (`SPECIES_NONE`, the old Unown IDs 252–276, the Egg and
Unown's alternate forms) are never mapped and never produced. Species
conversion goes through the game's own `SpeciesToNationalPokedexNum` and
`NationalPokedexNumToSpecies`.

The permutation comes from the randomizer's own generator (SplitMix64 with
Fisher–Yates), never from the game's `Random()`. Building the permutation
does not consume game RNG. Generating a different species can still change
later rolls: for example, Cute Charm uses different paths for mixed-sex and
genderless species.

## Set the seed before starting a new game

The settings live in `settings.txt`, not in the save file. A change takes
effect from the next Pokémon generated:

* a new seed changes the Pokémon you meet **from then on**. Pokémon already
  caught or given stay as they are;
* the starters follow the settings that apply when Birch's bag opens.

For a consistent run, pick the seed and kinds first, then start a new game.

## Setting it

* **OPTIONS > MODES > RANDOMIZER**: on/off for each
  kind, the seed (step ±1), and NEW SEED (a seed from the clock).
* **OPTIONS > MODES > MOVES & EVOS** and **OPTIONS > MODES > DIFFICULTY**:
  the rules beyond species (below).
* **settings.txt**, edited directly, or with lines printed by
  `scripts/randomizer_config.py`:

  ```
  python scripts/randomizer_config.py --seed 12345 --wild --starters --trainers
  python scripts/randomizer_config.py --seed-text "my run" --wild
  ```

## What it leaves alone

Link battles and recorded battles; the Battle Frontier (including Battle Pike
and Battle Pyramid wild Pokémon, which the game looks up by species); Trainer
Hill, e-Reader and secret base trainers; Steven's party as your partner; mass
outbreaks (TV announces the species and gives it its own moves); roamers;
gift, scripted and static Pokémon (Wally's Ralts, Birch's Zigzagoon, the
legendaries); and what `GetLocalWildMon` reports to TV and Match Call.
Evolutions and learnsets have their own settings (below).

## Behaviour notes

* A trainer Pokémon whose species changed loses its custom moveset and keeps
  the level-up moves of its new species (custom moves were chosen for the
  original). It keeps its held item, and its personality value is the one
  the vanilla game derives from the original species.
* Magnet Pull and Static still choose the encounter slot by the types of the
  original species in the table.
* Visible wild Pokémon pick their species when they appear, so the one you
  see is the one you battle. Ones already on screen keep their species if
  the setting changes.

## Beyond species: learnsets, evolutions, difficulty

All of these come from the same seed, through streams of their own
(`include/3ds_randomizer_rules.h`), never the game's `Random()`, and are
kept in `settings.txt`, not the save. With their defaults (nothing random,
NORMAL) the game is the original. Code: `src/3ds_randomizer_ext.c` (the
game's side), `src/3ds_randomizer_rules.c` (the rules, host-tested), the
hand-crafted teams in `src/3ds_strategy_teams.h`; the hooks are
`0083-randomizer-rules.patch`.

### OPTIONS > MODES > MOVES & EVOS

| key                | values                 | what it does |
|--------------------|------------------------|--------------|
| `rand_moves`       | OFF / ON               | every species' level-up learnset is random |
| `rand_moves_style` | TYPE (0) / ANY (1)     | TYPE: about half the moves of the species' own types, a fifth Normal, the rest anything |
| `rand_evos`        | OFF / ON               | what each species evolves into is random |
| `rand_evos_style`  | SIMILAR (0) / ANY (1)  | SIMILAR: a species within about 10% of the original target's base stat total |

Random learnsets keep every level and the number of moves of the original;
a learnset has no move twice, its first move always deals damage, and a
move's power is capped by the level it comes at (43 at Lv. 1, 100 at Lv. 20,
no cap from Lv. 40), so no Pokémon learns Explosion at Lv. 5. Struggle is
never handed out. Everything that reads level-up moves uses the random
learnset: a new Pokémon's first moves (wild, trainers', gifts, eggs'
level-1 moves), moves learned on levelling up or evolving, and the Move
Relearner. Trainers' hand-picked movesets are replaced by the random
learnset's moves too while it is on (they were chosen for the original
learnset). Pokémon already caught keep the moves they know.

Random evolutions keep each evolution's method and level, stone, friendship
or trade: only the species it leads to changes. A species never evolves into
itself, its evolutions (Eevee's five) are all different, and no chain loops
back: a target either ranks above the species in an order fixed by the seed
or has no evolution of its own. Legendaries only replace legendaries (never,
in Emerald's table). Shedinja still appears beside a Nincada that evolves
(into whatever it now evolves into), and eggs still hatch as the species'
original first stage (the Day Care walks the original table back).

### OPTIONS > MODES > DIFFICULTY

| key              | values                                | default |
|------------------|---------------------------------------|---------|
| `difficulty`     | EASY (0) / NORMAL (1) / HARD (2) / EXTREME (3) | NORMAL |
| `rand_bosses`    | BOSS TEAMS OFF / ON                   | ON      |
| `all_doubles`    | OFF (0) / AUTO (1) / ON (2)           | ON      |
| `strategy_teams` | OFF (0) / AUTO (1) / ON (2)           | ON      |

Both default to ON, at every difficulty from EASY to EXTREME, whether the
species randomizer is on or not. AUTO keeps them to HARD and EXTREME (off at
EASY and NORMAL); OFF gives the original single battles and teams.

What the difficulty changes, for the trainers of the game (the same ones the
species randomizer touches: not the Battle Frontier, the Trainer Hill,
e-Reader or secret bases, link or recorded battles):

|                    | EASY        | NORMAL   | HARD                         | EXTREME |
|--------------------|-------------|----------|------------------------------|---------|
| levels             | −10%        | original | +15%                         | +25% (at most 100) |
| AI                 | ordinary trainers pick moves at random; bosses keep theirs | original | + CHECK_BAD_MOVE, TRY_TO_FAINT, CHECK_VIABILITY; bosses also SETUP_FIRST_TURN, HP_AWARE | all of those for everyone |
| IVs (at least)     | original    | original | 15; bosses 31                | 31 |
| EVs per stat       | —           | —        | bosses 85                    | 85 bosses, 50 others |
| held items         | —           | —        | bosses' itemless Pokémon a Sitrus Berry | the same, the ace Leftovers; an ordinary trainer's last Pokémon a Sitrus Berry |
| boss team size     | original    | original | +1                           | 6 (3 each in a two-trainer battle) |

Level scaling applies to everyone, randomized, original or strategy teams.

**Bosses** are gym leaders (rematches included), the Elite Four, the
Champion, the rivals (Brendan or May, Wally, Steven at Meteor Falls) and
Maxie and Archie. With BOSS TEAMS on and RANDOM TRAINERS on, a boss's
Pokémon are not drawn from the permutation but from the strong species of
their level: a base stat total range that grows with the level (305–395 at
Lv. 5, 355–465 at Lv. 15, 430–570 at Lv. 30, 500 and up from Lv. 44),
legendaries only from Lv. 55, no species twice, and a gym leader or Elite
Four member keeps the type most of its original team had when there are at
least three such species. Their moves are the best four of what their
(possibly random) learnset gives them at their level: up to three damaging
moves scored by power, accuracy and same-type bonus, spread over types, and
one useful status move (Swords Dance, Thunder Wave, Recover, Reflect...).
The extra Pokémon of HARD and EXTREME come from the same pool.

**ALL DOUBLES** makes every single battle against one trainer a double
battle, when the player has two Pokémon able to fight (otherwise it stays
single, as the game does for its own double trainers). A trainer with one
Pokémon gets a second one: of about the same strength (base stats within
15%), or from the boss pool for a boss, at its first Pokémon's level.
Battles that are already double (two trainers, Tate & Liza, the
multi battle with Steven) are left as they are. Exceptions: Wally's
catching tutorial and Birch's Zigzagoon are not trainer battles and are
never touched; the first rival battle on Route 103 stays single because the
player has only one Pokémon then.

**STRATEGY TEAMS** are hand-crafted, non-random teams that replace a
trainer's party (randomizer or not): Roxanne (Explosion lead, Sturdy walls),
Brawly (Fake Out, Bulk Up), Wattson (Plus/Minus with Helping Hand), Flannery
(Sunny Day and Chlorophyll), Norman (Follow Me covering Slaking's Truant),
Winona (Reflect + Light Screen, Dragon Dance), Tate & Liza (Earthquake beside
Levitate and Flying), Juan (Rain Dance + Swift Swim), Sidney (Tyranitar's
Sand Stream), Phoebe (Mean Look + Perish Song, Will-O-Wisp, Destiny Bond),
Glacia (Hail, Blizzard, Perish Song), Drake (Dragon Dance, Earthquake over
Levitate), Wallace (rain, Water Spout, Recover), Maxie twice (a Levitate
Weezing's Explosion beside Camerupt's Magnitude or Earthquake), Archie (rain and Swift
Swim), Wally at Victory Road (Fake Out and Helping Hand for a Calm Mind
Gardevoir), and the Cooltrainers Shannon (Wobbuffet's Shadow Tag + Perish
Song), Katelynn (Follow Me + Slaking), Felix (Earthquake + Levitate), Julie
(sun) and Edgar (Sandstorm + Sand Veil). Only third-generation moves and
abilities are used. Each Pokémon has its nature, ability, held item, IVs,
EVs and four moves written out; the levels are the trainer's own and scale
with the difficulty. They are made for double battles but work in singles.
The table is meant to be edited: see the header of
`src/3ds_strategy_teams.h`.

### Saves

Nothing of this is written to the save: the settings live in
`settings.txt`. A Pokémon generated while a rule is on keeps what it got
(its moves, species) when the rule is turned off, like any Pokémon. A save
made with the rules on loads in a build without them. Turning random
evolutions or learnsets off mid-run simply gives the original evolutions and
level-up moves from then on.

## Tests

`make verify-randomizer` (part of `make verify`) builds `tests/randomizer_test.c`
with `src/3ds_randomizer_core.c` on the host. It checks that each seed gives
a real permutation, the same seed gives the same result, different seeds
differ, and invalid IDs stay untouched.

`make verify-randomizer-rules` (part of `make verify`) builds
`tests/randomizer_rules_test.c` with `src/3ds_randomizer_rules.c` on
synthetic species and move tables: learnsets keep their levels and length,
have no duplicates, start with a damaging move and respect the power cap;
evolutions never target themselves, never loop and stay near the original's
strength in SIMILAR; team picks stay in range and never repeat; the
difficulty's levels, AI flags, IVs, EVs and team sizes.

`make verify-strategy-teams` (also in `make verify`, after bootstrap) checks
every strategy team row against the tree: species, moves, items, natures
and trainers exist, each named ability is one of its species' own, no
Pokémon knows a move twice.

`make verify-randomizer-locales` (also part of `make verify`) runs against the
complete bootstrap-patched English source tree. It checks every Spanish,
French and Italian manifest source hash and edit range, verifies that the
26 `battle_main.c` text targets and ROM references remain unchanged, and runs
all three production stagers with generated bytes. It also checks that each
stager rejects a modified source before writing anything. The target fixture
contains only positions and hashes; no ROM payload or game text is stored.
On an already-localized tree, the production `.emerald3ds-locale` marker makes
this English-source regression suite report an explicit skip. Run it on the
English bootstrap tree before staging a language; the localization scripts'
production source-hash guards remain unchanged.

The game hooks are applied by `0075-seeded-species-randomizer.patch` and
`0083-randomizer-rules.patch`. Their
insertions shift source text positions, so the three localization manifests
carry the corresponding patched source hash and rebased edit positions.
