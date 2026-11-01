/*
 * The randomizer's rules beyond species (include/3ds_randomizer_rules.h).
 * SDK-free and game-free: the host test builds it as it is.
 */
#include <stddef.h>
#include "3ds_randomizer_rules.h"

#define DOMAIN_LEARNSET 0x4C524E53u /* "LRNS" */
#define DOMAIN_RANK     0x52414E4Bu /* "RANK" */
#define DOMAIN_EVOLVE   0x45564F4Cu /* "EVOL" */

#define MAX_LEARNSET 64

static uint64_t SplitMix64(uint64_t *state)
{
    uint64_t z = (*state += 0x9E3779B97F4A7C15ull);

    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
    return z ^ (z >> 31);
}

uint64_t CtrRules_Stream(uint32_t seed, uint32_t domain, uint32_t a, uint32_t b)
{
    uint64_t state = ((uint64_t)domain << 32) ^ seed;

    /* Each key goes through the mixer, so nearby keys share nothing. */
    state = SplitMix64(&state) ^ a;
    state = SplitMix64(&state) ^ ((uint64_t)b << 17);
    SplitMix64(&state);
    return state;
}

uint32_t CtrRules_Next(uint64_t *state)
{
    return (uint32_t)(SplitMix64(state) >> 32);
}

uint32_t CtrRules_Below(uint64_t *state, uint32_t bound)
{
    uint32_t limit, value;

    if (bound <= 1)
        return 0;
    limit = UINT32_MAX - UINT32_MAX % bound;
    do
        value = CtrRules_Next(state);
    while (value >= limit);
    return value % bound;
}

/* ---- learnsets ---- */

unsigned CtrRules_PowerCap(unsigned level)
{
    /* 43 at Lv. 1, 70 at Lv. 10, 100 at Lv. 20; no cap from Lv. 40. */
    return level >= 40 ? 255 : 40 + 3 * level;
}

static unsigned EffectivePower(const CtrRulesMove *move)
{
    /* Variable power (Seismic Toss, Low Kick...): about a mid move. */
    return move->power == 1 ? 60 : move->power;
}

enum { POOL_OWN_TYPE, POOL_NORMAL, POOL_ANY };

static bool InLearnset(const uint16_t *out, unsigned count, uint16_t move)
{
    for (unsigned i = 0; i < count; i++)
        if ((out[i] & CTR_RULES_MOVE_ID_MASK) == move)
            return true;
    return false;
}

static bool MoveFits(const CtrRulesMove *moves, uint16_t id, unsigned pool, uint8_t type1, uint8_t type2,
                     unsigned cap, bool damaging, const uint16_t *out, unsigned count)
{
    const CtrRulesMove *move = &moves[id];

    if (!move->usable)
        return false;
    if (damaging && move->power == 0)
        return false;
    if (move->power != 0 && EffectivePower(move) > cap)
        return false;
    if (pool == POOL_OWN_TYPE && move->type != type1 && move->type != type2)
        return false;
    if (pool == POOL_NORMAL && move->type != 0) /* TYPE_NORMAL */
        return false;
    return !InLearnset(out, count, id);
}

static uint16_t ChooseMove(uint64_t *state, const CtrRulesMove *moves, unsigned moveCount, unsigned pool,
                           uint8_t type1, uint8_t type2, unsigned cap, bool damaging,
                           const uint16_t *out, unsigned count)
{
    unsigned tries, i, start;

    /* A few random draws first (cheap), then a scan from a random start so
     * a small pool still gives an answer, then looser rules. */
    for (tries = 0; tries < 48; tries++)
    {
        uint16_t id = (uint16_t)(1 + CtrRules_Below(state, moveCount - 1));

        if (MoveFits(moves, id, pool, type1, type2, cap, damaging, out, count))
            return id;
    }
    start = 1 + CtrRules_Below(state, moveCount - 1);
    for (unsigned pass = 0; pass < 4; pass++)
    {
        unsigned p = pass == 0 ? pool : POOL_ANY;
        unsigned c = pass < 2 ? cap : 255;
        bool d = pass < 3 ? damaging : false;

        for (i = 0; i < moveCount - 1; i++)
        {
            uint16_t id = (uint16_t)(1 + (start - 1 + i) % (moveCount - 1));

            if (MoveFits(moves, id, p, type1, type2, c, d, out, count))
                return id;
        }
    }
    return 0;
}

unsigned CtrRules_BuildLearnset(uint32_t seed, uint16_t species, const uint16_t *original,
                                uint16_t *out, unsigned outCapacity,
                                const CtrRulesMove *moves, unsigned moveCount,
                                uint8_t type1, uint8_t type2, bool typed)
{
    uint64_t state;
    unsigned n = 0, k;

    if (original == NULL || out == NULL || moves == NULL || moveCount < 2)
        return 0;
    while (n < MAX_LEARNSET && original[n] != CTR_RULES_LEARNSET_END)
        n++;
    if (n + 1 > outCapacity)
        return 0;
    state = CtrRules_Stream(seed, DOMAIN_LEARNSET, species, typed ? 1 : 0);
    for (k = 0; k < n; k++)
    {
        unsigned level = original[k] >> CTR_RULES_LEVEL_SHIFT;
        unsigned pool = POOL_ANY;
        uint16_t move;

        if (typed)
        {
            uint32_t roll = CtrRules_Below(&state, 100);

            pool = roll < 50 ? POOL_OWN_TYPE : roll < 70 ? POOL_NORMAL : POOL_ANY;
        }
        /* The first move is a damaging one, so every Pokemon can fight. */
        move = ChooseMove(&state, moves, moveCount, pool, type1, type2,
                          CtrRules_PowerCap(level), k == 0, out, k);
        if (move == 0)
            move = original[k] & CTR_RULES_MOVE_ID_MASK;
        out[k] = CTR_RULES_ENTRY(level, move);
    }
    out[n] = CTR_RULES_LEARNSET_END;
    return n + 1;
}

/* ---- evolutions ---- */

static uint32_t Rank(uint32_t seed, uint16_t species)
{
    uint64_t state = CtrRules_Stream(seed, DOMAIN_RANK, species, 0);

    /* Ties broken by the species id, so the order is strict. */
    return (CtrRules_Next(&state) & 0xFFFFFF00u) | (species & 0xFFu);
}

static bool Listed(const uint16_t *list, unsigned count, uint16_t species)
{
    for (unsigned i = 0; i < count; i++)
        if (list[i] == species)
            return true;
    return false;
}

static bool EvolutionFits(uint32_t seed, uint32_t fromRank, uint16_t from, uint16_t target, bool legendaryOk,
                          const CtrRulesSpecies *species, const uint16_t *taken, unsigned takenCount,
                          unsigned lo, unsigned hi)
{
    const CtrRulesSpecies *s = &species[target];

    if (s->bst == 0 || target == from || s->bst < lo || s->bst > hi)
        return false;
    if (s->legendary && !legendaryOk)
        return false;
    if (Listed(taken, takenCount, target))
        return false;
    /* No cycle: up the seed's order, or onto a species that ends its line. */
    return !s->evolves || Rank(seed, target) > fromRank;
}

uint16_t CtrRules_PickEvolution(uint32_t seed, uint16_t from, uint16_t original, unsigned slot,
                                const CtrRulesSpecies *species, unsigned speciesCount,
                                const uint16_t *taken, unsigned takenCount, bool similar)
{
    uint64_t state;
    uint32_t fromRank;
    unsigned bst, width, pass;
    bool legendaryOk;

    if (species == NULL || from >= speciesCount || original >= speciesCount || species[original].bst == 0)
        return original;
    state = CtrRules_Stream(seed, DOMAIN_EVOLVE, from, slot);
    fromRank = Rank(seed, from);
    bst = species[original].bst;
    width = bst / 10 < 30 ? 30 : bst / 10;
    legendaryOk = species[original].legendary != 0;
    for (pass = similar ? 0 : 3; pass < 4; pass++)
    {
        unsigned w = pass < 3 ? width << pass : 0;
        unsigned lo = pass < 3 ? (bst > w ? bst - w : 0) : 0;
        unsigned hi = pass < 3 ? bst + w : 0xFFFF;
        unsigned count = 0, pick, t;

        for (t = 1; t < speciesCount; t++)
            if (EvolutionFits(seed, fromRank, from, (uint16_t)t, legendaryOk, species, taken, takenCount, lo, hi))
                count++;
        if (count == 0)
            continue;
        pick = CtrRules_Below(&state, count);
        for (t = 1; t < speciesCount; t++)
            if (EvolutionFits(seed, fromRank, from, (uint16_t)t, legendaryOk, species, taken, takenCount, lo, hi)
             && pick-- == 0)
                return (uint16_t)t;
    }
    return original;
}

/* ---- teams ---- */

static bool SpeciesFits(const CtrRulesSpecies *s, uint16_t id, unsigned lo, unsigned hi, uint8_t type,
                        bool allowLegendary, const uint16_t *exclude, unsigned excludeCount)
{
    if (s->bst == 0 || s->bst < lo || s->bst > hi)
        return false;
    if (s->legendary && !allowLegendary)
        return false;
    if (type != CTR_RULES_TYPE_NONE && s->type1 != type && s->type2 != type)
        return false;
    return !Listed(exclude, excludeCount, id);
}

uint16_t CtrRules_PickSpecies(uint64_t *state, const CtrRulesSpecies *species, unsigned speciesCount,
                              unsigned minBst, unsigned maxBst, uint8_t preferType, bool allowLegendary,
                              const uint16_t *exclude, unsigned excludeCount)
{
    unsigned widen, t;

    if (species == NULL || state == NULL)
        return 0;
    for (widen = 0; widen <= 6; widen++)
    {
        unsigned lo = widen == 6 ? 0 : (minBst > widen * 40 ? minBst - widen * 40 : 0);
        unsigned hi = widen == 6 ? 0xFFFF : maxBst + widen * 40;

        for (unsigned typed = 0; typed < 2; typed++)
        {
            uint8_t type = typed == 0 ? preferType : CTR_RULES_TYPE_NONE;
            unsigned count = 0, pick;

            if (typed == 0 && preferType == CTR_RULES_TYPE_NONE)
                continue;
            for (t = 1; t < speciesCount; t++)
                if (SpeciesFits(&species[t], (uint16_t)t, lo, hi, type, allowLegendary, exclude, excludeCount))
                    count++;
            /* A type is only preferred while it leaves some choice. */
            if (count == 0 || (typed == 0 && count < 3))
                continue;
            pick = CtrRules_Below(state, count);
            for (t = 1; t < speciesCount; t++)
                if (SpeciesFits(&species[t], (uint16_t)t, lo, hi, type, allowLegendary, exclude, excludeCount)
                 && pick-- == 0)
                    return (uint16_t)t;
        }
    }
    return 0;
}

void CtrRules_BossBstRange(unsigned level, unsigned *minBst, unsigned *maxBst)
{
    /* Lv. 5: 305-395 (a good first stage); Lv. 15: 355-465; Lv. 30:
     * 430-570; Lv. 46 and up: 500 and over (fully evolved, pseudo-legends). */
    unsigned lo = 280 + 5 * level, hi = 360 + 7 * level;

    if (minBst != NULL)
        *minBst = lo > 500 ? 500 : lo;
    if (maxBst != NULL)
        *maxBst = hi > 700 ? 700 : hi;
}

bool CtrRules_BossAllowsLegendary(unsigned level)
{
    return level >= 55;
}

static unsigned MoveScore(const CtrRulesMove *move, uint8_t type1, uint8_t type2)
{
    unsigned accuracy = move->accuracy == 0 ? 100 : move->accuracy;
    unsigned score = EffectivePower(move) * accuracy;

    if (move->type == type1 || move->type == type2)
        score = score * 3 / 2;
    if (move->drawback)
        score /= 2;
    return score;
}

unsigned CtrRules_BestMoves(const uint16_t *learnset, unsigned level, const CtrRulesMove *moves,
                            unsigned moveCount, uint8_t type1, uint8_t type2, uint16_t out[4])
{
    uint16_t known[MAX_LEARNSET];
    unsigned n = 0, chosen = 0, i, k;
    bool used[MAX_LEARNSET] = {false};
    int status = -1;

    for (i = 0; i < 4; i++)
        out[i] = 0;
    if (learnset == NULL || moves == NULL)
        return 0;
    for (i = 0; i < MAX_LEARNSET && learnset[i] != CTR_RULES_LEARNSET_END; i++)
    {
        uint16_t id = learnset[i] & CTR_RULES_MOVE_ID_MASK;

        if ((unsigned)(learnset[i] >> CTR_RULES_LEVEL_SHIFT) > level)
            break;
        if (id == 0 || id >= moveCount)
            continue;
        for (k = 0; k < n && known[k] != id; k++)
            ;
        if (k == n)
            known[n++] = id;
    }
    /* The latest good status move. */
    for (i = 0; i < n; i++)
        if (moves[known[i]].power == 0 && moves[known[i]].goodStatus)
            status = (int)i;
    /* Damaging moves, best first; a second one of a type counts half. */
    while (chosen < (status >= 0 ? 3u : 4u))
    {
        int best = -1;
        unsigned bestScore = 0;

        for (i = 0; i < n; i++)
        {
            unsigned score;

            if (used[i] || moves[known[i]].power == 0)
                continue;
            score = MoveScore(&moves[known[i]], type1, type2);
            for (k = 0; k < chosen; k++)
                if (moves[out[k]].type == moves[known[i]].type)
                    score /= 2;
            if (best < 0 || score > bestScore)
            {
                best = (int)i;
                bestScore = score;
            }
        }
        if (best < 0)
            break;
        used[best] = true;
        out[chosen++] = known[best];
    }
    if (status >= 0)
    {
        used[status] = true;
        out[chosen++] = known[status];
    }
    /* Whatever is left, latest first. */
    for (i = n; i-- > 0 && chosen < 4;)
        if (!used[i])
        {
            used[i] = true;
            out[chosen++] = known[i];
        }
    return chosen;
}

/* ---- difficulty ---- */

unsigned CtrRules_ScaleLevel(unsigned level, unsigned difficulty)
{
    static const unsigned percent[CTR_DIFFICULTY_COUNT] = {90, 100, 115, 125};
    unsigned scaled;

    if (difficulty >= CTR_DIFFICULTY_COUNT || difficulty == CTR_DIFFICULTY_NORMAL)
        return level;
    scaled = (level * percent[difficulty] + 50) / 100;
    if (scaled < 2)
        scaled = 2;
    if (scaled > 100)
        scaled = 100;
    return scaled;
}

uint32_t CtrRules_AiFlags(uint32_t vanilla, unsigned difficulty, bool boss)
{
    const uint32_t smart = CTR_AI_CHECK_BAD_MOVE | CTR_AI_TRY_TO_FAINT | CTR_AI_CHECK_VIABILITY;

    switch (difficulty)
    {
    case CTR_DIFFICULTY_EASY:
        /* Bosses keep their own; the others pick moves at random. */
        return boss ? vanilla : vanilla & ~(smart | CTR_AI_SETUP_FIRST_TURN | CTR_AI_HP_AWARE
                                            | CTR_AI_PREFER_POWER_EXTREMES | CTR_AI_PREFER_BATON_PASS);
    case CTR_DIFFICULTY_HARD:
        return vanilla | smart | (boss ? CTR_AI_SETUP_FIRST_TURN | CTR_AI_HP_AWARE : 0);
    case CTR_DIFFICULTY_EXTREME:
        return vanilla | smart | CTR_AI_HP_AWARE | CTR_AI_SETUP_FIRST_TURN;
    default:
        return vanilla;
    }
}

unsigned CtrRules_MinIv(unsigned difficulty, bool boss)
{
    switch (difficulty)
    {
    case CTR_DIFFICULTY_HARD:
        return boss ? 31 : 15;
    case CTR_DIFFICULTY_EXTREME:
        return 31;
    default:
        return 0;
    }
}

unsigned CtrRules_EvPerStat(unsigned difficulty, bool boss)
{
    switch (difficulty)
    {
    case CTR_DIFFICULTY_HARD:
        return boss ? 85 : 0;
    case CTR_DIFFICULTY_EXTREME:
        return boss ? 85 : 50;
    default:
        return 0;
    }
}

unsigned CtrRules_BossTeamSize(unsigned count, unsigned difficulty, unsigned max)
{
    unsigned size = count;

    if (difficulty == CTR_DIFFICULTY_HARD)
        size = count + 1;
    else if (difficulty == CTR_DIFFICULTY_EXTREME)
        size = max;
    if (size > max)
        size = max;
    return size < count ? count : size;
}
