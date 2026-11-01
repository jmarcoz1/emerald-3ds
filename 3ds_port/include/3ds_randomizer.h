#ifndef CTR_RANDOMIZER_H
#define CTR_RANDOMIZER_H

/*
 * The seeded species randomizer (optional, off by default).
 *
 * One seed gives one permutation of the 386 real National Pokedex species;
 * every randomized Pokemon of a species becomes the species the permutation
 * maps it to. The same seed always gives the same permutation, on any
 * machine: building the permutation uses its own generator (SplitMix64),
 * never the game's Random(). Generating a different species can still
 * change the game's later rolls, for example through Cute Charm.
 *
 * What it randomizes, each on its own setting (settings.txt, 0 or 1):
 *   rand_wild      wild Pokemon of the map's tables (land, water, rocks,
 *                  fishing, Feebas, the visible wild ones);
 *   rand_starters  the three Pokemon of Birch's bag;
 *   rand_trainers  the parties of the game's ordinary trainers.
 * rand_seed is the seed (0 to 2147483647).
 *
 * Never randomized: Pokemon the player already has, saves, link and
 * recorded battles, the Battle Frontier (Pike and Pyramid wild Pokemon
 * included), the Trainer Hill, e-Reader and secret base trainers, mass
 * outbreaks, roamers, gift and scripted Pokemon (Wally's Ralts, Birch's
 * Zigzagoon, the legendaries).
 *
 * Set the seed before starting a new game: the settings live in
 * settings.txt, not in the save, so changing the seed later changes the
 * Pokemon met from then on (and the starters, if not yet chosen) and nothing
 * already caught or given.
 *
 * The first part is SDK-free and game-free (tests/randomizer_test.c builds
 * it on the host): it works on National Pokedex numbers and on internal
 * species ids through the converters the caller gives (the game's own
 * SpeciesToNationalPokedexNum and NationalPokedexNumToSpecies).
 */
#include <stdbool.h>
#include <stdint.h>

#define CTR_RANDOMIZER_NATIONAL_COUNT 386
/* Emerald's internal species ids (include/constants/species.h, checked in
 * src/3ds_randomizer.c): 1-251 the first two generations, 252-276 the unused
 * old Unown, 277-411 Hoenn's, 412 the Egg, then Unown's other forms. */
#define CTR_RANDOMIZER_SPECIES_LAST_JOHTO 251
#define CTR_RANDOMIZER_SPECIES_FIRST_HOENN 277
#define CTR_RANDOMIZER_SPECIES_LAST_HOENN 411
#define CTR_RANDOMIZER_SEED_MAX 0x7FFFFFFFu

typedef struct
{
    uint32_t seed;
    /* map[n]: the National Pokedex number n becomes (n 1-386); map[0] = 0. */
    uint16_t map[CTR_RANDOMIZER_NATIONAL_COUNT + 1];
} CtrRandomizerTable;

typedef uint16_t (*CtrRandomizerConvert)(uint16_t value);

/* The permutation of this seed. */
void CtrRandomizer_Build(CtrRandomizerTable *table, uint32_t seed);
/* The number a National Pokedex number becomes; 0 for anything not 1-386. */
uint16_t CtrRandomizer_MapNational(const CtrRandomizerTable *table, uint16_t national);
/* A real species id: not NONE, an old Unown, the Egg or an Unown form. */
bool CtrRandomizer_SpeciesEligible(uint16_t species);
/* The species a species id becomes, or the id itself when it is not an
 * eligible species or the converters do not agree on the result. */
uint16_t CtrRandomizer_MapSpecies(const CtrRandomizerTable *table, uint16_t species,
                                  CtrRandomizerConvert toNational, CtrRandomizerConvert toSpecies);
/* A seed from a word (FNV-1a, 31 bits): scripts/randomizer_config.py agrees. */
uint32_t CtrRandomizer_SeedFromText(const char *text);
/* A seed from two values (a new seed from the clock). */
uint32_t CtrRandomizer_MixSeed(uint64_t a, uint64_t b);

/*
 * The game's side (src/3ds_randomizer.c, a game translation unit).
 */
enum
{
    CTR_RANDOMIZE_WILD,
    CTR_RANDOMIZE_STARTERS,
    CTR_RANDOMIZE_TRAINERS,
    CTR_RANDOMIZE_KINDS
};

bool CtrRandomizer_Enabled(unsigned kind);
uint32_t CtrRandomizer_Seed(void);
/* What this species becomes for that kind: itself while the kind is off. */
uint16_t CtrRandomizer_Species(unsigned kind, uint16_t species);

/*
 * Beyond species (src/3ds_randomizer_ext.c; the rules themselves are in
 * include/3ds_randomizer_rules.h). Settings, all in settings.txt:
 *   rand_moves        random level-up learnsets (0/1);
 *   rand_moves_style  0 TYPED (half of a species' moves of its own types),
 *                     1 ANY;
 *   rand_evos         random evolutions (0/1);
 *   rand_evos_style   0 SIMILAR (a target of about the same strength), 1 ANY;
 *   difficulty        0 EASY, 1 NORMAL (the original game, default),
 *                     2 HARD, 3 EXTREME;
 *   rand_bosses       BOSS TEAMS (default 1): gym leaders, the Elite Four,
 *                     the Champion, rivals and the team leaders get strong
 *                     randomized teams and the hard-mode extras;
 *   all_doubles       every trainer battle double: 0 OFF, 1 AUTO (on at
 *                     HARD and EXTREME, default), 2 ON;
 *   strategy_teams    the hand-crafted teams of src/3ds_strategy_teams.h:
 *                     0 OFF, 1 AUTO (default), 2 ON.
 * Like the species, these change what is generated from then on, never a
 * Pokemon already caught, and nothing is stored in the save.
 */
struct Pokemon;
unsigned CtrRandomizer_Difficulty(void);
bool CtrRandomizer_LearnsetsEnabled(void);
bool CtrRandomizer_DoublesEnabled(void);
bool CtrRandomizer_StrategyEnabled(void);
/* The level-up learnset a species has: the original unless rand_moves. */
const uint16_t *CtrRandomizer_Learnset(uint16_t species);
/* What a species evolving into target evolves into instead. */
uint16_t CtrRandomizer_Evolution(uint16_t species, uint16_t target);
/* After the vanilla party is made (battle_main.c, CreateNPCTrainerParty):
 * the difficulty, boss and strategy teams and doubles; the new count. */
uint8_t CtrRandomizer_AdjustTrainerParty(struct Pokemon *party, uint16_t trainerNum, uint8_t monsCount,
                                         uint8_t firstTrainer);
/* The AI scripts of an ordinary trainer battle (battle_ai_script_commands.c). */
uint32_t CtrRandomizer_TrainerAiFlags(uint16_t trainerNum);

/* For the OPTIONS cells (include/3ds_randomizer_extras.inc). */
extern const char *const gCtrRandomizerOpenText[1];
extern const char *const gCtrRandomizerBackText[1];
extern const char *const gCtrRandomizerNewSeedText[1];
extern const char *const gCtrRandomizerMoveStyleText[2];
extern const char *const gCtrRandomizerEvoStyleText[2];
extern const char *const gCtrRandomizerDifficultyText[4];
extern const char *const gCtrRandomizerAutoText[3];
void CtrRandomizer_OpenDifficulty(void);
void CtrRandomizer_Open(void);
void CtrRandomizer_Back(void);
void CtrRandomizer_StepSeed(int direction);
const uint8_t *CtrRandomizer_SeedText(void);
void CtrRandomizer_NewSeed(void);

#endif // CTR_RANDOMIZER_H
