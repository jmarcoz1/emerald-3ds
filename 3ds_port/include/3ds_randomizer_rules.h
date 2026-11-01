#ifndef CTR_RANDOMIZER_RULES_H
#define CTR_RANDOMIZER_RULES_H

/*
 * The randomizer's rules beyond species (optional, off by default): random
 * level-up learnsets, random evolutions, difficulty levels, boss teams and
 * the doubles opponents' second Pokemon (include/3ds_randomizer.h has the
 * species permutation and the game's side).
 *
 * SDK-free and game-free, like the permutation (tests/randomizer_rules_test.c
 * builds it on the host): the game's side hands it plain tables of the
 * species and moves it needs (src/3ds_randomizer_ext.c fills them from
 * gSpeciesInfo and gBattleMoves). Everything comes from the seed through its
 * own SplitMix64 streams, one per purpose, never from the game's Random():
 * the same seed always gives the same learnsets, evolutions and boss teams.
 */
#include <stdbool.h>
#include <stdint.h>

/* A level-up learnset entry as the game stores it (include/pokemon.h). */
#define CTR_RULES_LEARNSET_END 0xFFFFu
#define CTR_RULES_MOVE_ID_MASK 0x01FFu
#define CTR_RULES_LEVEL_SHIFT 9
#define CTR_RULES_ENTRY(level, move) ((uint16_t)(((level) << CTR_RULES_LEVEL_SHIFT) | (move)))

#define CTR_RULES_TYPE_NONE 0xFFu

typedef struct
{
    uint8_t power;      /* 0: a status move; 1: variable power */
    uint8_t type;
    uint8_t accuracy;   /* 0: never misses */
    uint8_t usable;     /* 0: never handed out (MOVE_NONE, Struggle...) */
    uint8_t goodStatus; /* a status move worth a slot of a boss's four */
    uint8_t drawback;   /* a damaging move worth less than its power (Explosion,
                         * Hyper Beam's recharge, two turns...) */
} CtrRulesMove;

typedef struct
{
    uint16_t bst;       /* base stat total; 0: not a species to hand out */
    uint8_t type1, type2;
    uint8_t legendary;
    uint8_t evolves;    /* it has an evolution of its own (vanilla table) */
} CtrRulesSpecies;

/* One stream for one purpose of one seed (domain), keyed by a and b. */
uint64_t CtrRules_Stream(uint32_t seed, uint32_t domain, uint32_t a, uint32_t b);
uint32_t CtrRules_Next(uint64_t *state);
/* Uniform in 0..bound-1 (bound >= 1). */
uint32_t CtrRules_Below(uint64_t *state, uint32_t bound);

/*
 * Random learnsets. The new learnset keeps every level of the original and
 * its length; its moves are all different, a damaging move comes first, and
 * a move's power is capped by the level it is learned at (CtrRules_PowerCap)
 * so an early level never hands out Explosion. typed: half the moves are of
 * the species' own types, a fifth Normal, the rest anything; otherwise any
 * usable move. Writes the entries and the terminator to out and returns how
 * many it wrote (terminator included), or 0 when out is too small.
 */
unsigned CtrRules_PowerCap(unsigned level);
unsigned CtrRules_BuildLearnset(uint32_t seed, uint16_t species, const uint16_t *original,
                                uint16_t *out, unsigned outCapacity,
                                const CtrRulesMove *moves, unsigned moveCount,
                                uint8_t type1, uint8_t type2, bool typed);

/*
 * Random evolutions. The species `from` would evolve into `original` (its
 * slot-th evolution); this gives what it evolves into instead: never itself,
 * never one of `taken` (its other evolutions already chosen), and never a
 * cycle: a target either ranks above `from` in an order fixed by the seed or
 * has no evolution of its own. similar: a species within about 10% of the
 * original target's base stat total (widened when there is none); otherwise
 * any. Legendaries only replace legendaries. Returns `original` when nothing
 * qualifies.
 */
uint16_t CtrRules_PickEvolution(uint32_t seed, uint16_t from, uint16_t original, unsigned slot,
                                const CtrRulesSpecies *species, unsigned speciesCount,
                                const uint16_t *taken, unsigned takenCount, bool similar);

/*
 * A species for a team: base stat total in minBst..maxBst (widened when there
 * is none), not one of `exclude`, legendaries only if allowed, preferring
 * preferType (when at least 3 qualify). 0 when the table has none at all.
 */
uint16_t CtrRules_PickSpecies(uint64_t *state, const CtrRulesSpecies *species, unsigned speciesCount,
                              unsigned minBst, unsigned maxBst, uint8_t preferType, bool allowLegendary,
                              const uint16_t *exclude, unsigned excludeCount);
/* The base stat totals a boss's Pokemon of this level is drawn from. */
void CtrRules_BossBstRange(unsigned level, unsigned *minBst, unsigned *maxBst);
bool CtrRules_BossAllowsLegendary(unsigned level);

/*
 * A boss's four moves from the learnset it has (entries up to its level):
 * up to three damaging moves scored by power, accuracy and same-type bonus,
 * spread over types, and one good status move when it has one. Fills out
 * (MOVE_NONE = 0 past the moves it has) and returns how many it chose.
 */
unsigned CtrRules_BestMoves(const uint16_t *learnset, unsigned level, const CtrRulesMove *moves,
                            unsigned moveCount, uint8_t type1, uint8_t type2, uint16_t out[4]);

/*
 * Difficulty levels. NORMAL is the game as it is.
 */
enum
{
    CTR_DIFFICULTY_EASY,
    CTR_DIFFICULTY_NORMAL,
    CTR_DIFFICULTY_HARD,
    CTR_DIFFICULTY_EXTREME,
    CTR_DIFFICULTY_COUNT
};

/* pokeemerald's AI script flags (include/constants/battle_ai.h, checked by
 * src/3ds_randomizer_ext.c). */
#define CTR_AI_CHECK_BAD_MOVE        (1u << 0)
#define CTR_AI_TRY_TO_FAINT          (1u << 1)
#define CTR_AI_CHECK_VIABILITY       (1u << 2)
#define CTR_AI_SETUP_FIRST_TURN      (1u << 3)
#define CTR_AI_RISKY                 (1u << 4)
#define CTR_AI_PREFER_POWER_EXTREMES (1u << 5)
#define CTR_AI_PREFER_BATON_PASS     (1u << 6)
#define CTR_AI_DOUBLE_BATTLE         (1u << 7)
#define CTR_AI_HP_AWARE              (1u << 8)

/* EASY -10%, HARD +15%, EXTREME +25%, rounded; 2..100. */
unsigned CtrRules_ScaleLevel(unsigned level, unsigned difficulty);
/* The AI scripts a trainer runs: its own on NORMAL, none for an ordinary
 * trainer on EASY, the smart ones added on HARD and EXTREME (bosses more). */
uint32_t CtrRules_AiFlags(uint32_t vanilla, unsigned difficulty, bool boss);
/* The lowest IV (0..31) of each stat; 0: as the trainer has them. */
unsigned CtrRules_MinIv(unsigned difficulty, bool boss);
/* EVs given to each stat (six times this stays within 510). */
unsigned CtrRules_EvPerStat(unsigned difficulty, bool boss);
/* How many Pokemon a boss's team grows to (from its own count, at most max). */
unsigned CtrRules_BossTeamSize(unsigned count, unsigned difficulty, unsigned max);

#endif // CTR_RANDOMIZER_RULES_H
