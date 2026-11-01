/*
 * Host test of the randomizer's rules beyond species
 * (src/3ds_randomizer_rules.c) on synthetic species and move tables: no game
 * data, no SDK. make verify-randomizer-rules (part of make verify).
 */
#include <stdio.h>
#include <string.h>
#include "3ds_randomizer_rules.h"

#define SPECIES 412
#define MOVES 355
#define CHECK(cond) do { if (!(cond)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

static int failures;
static CtrRulesSpecies sSpecies[SPECIES];
static CtrRulesMove sMoves[MOVES];
/* A synthetic evolution table: species s of a line evolves into s+1. */
static uint16_t sEvolvesTo[SPECIES];

static void MakeTables(void)
{
    uint64_t state = CtrRules_Stream(1, 0x54455354u, 0, 0);
    unsigned s, m;

    for (s = 1; s < SPECIES; s++)
    {
        bool real = s <= 251 || s >= 277;

        sSpecies[s].bst = real ? (uint16_t)(200 + CtrRules_Below(&state, 481)) : 0;
        sSpecies[s].type1 = (uint8_t)CtrRules_Below(&state, 18);
        sSpecies[s].type2 = CtrRules_Below(&state, 2) ? sSpecies[s].type1 : (uint8_t)CtrRules_Below(&state, 18);
        sSpecies[s].legendary = real && s % 50 == 0;
    }
    /* Lines of three: s, s+1, s+2 for s % 3 == 1 (both real). */
    for (s = 1; s + 2 < SPECIES; s += 3)
        if (sSpecies[s].bst && sSpecies[s + 1].bst && sSpecies[s + 2].bst)
        {
            sEvolvesTo[s] = (uint16_t)(s + 1);
            sEvolvesTo[s + 1] = (uint16_t)(s + 2);
            sSpecies[s].evolves = sSpecies[s + 1].evolves = 1;
        }
    for (m = 1; m < MOVES; m++)
    {
        sMoves[m].usable = m != 165;
        sMoves[m].type = (uint8_t)(m % 18);
        sMoves[m].power = m % 4 == 0 ? 0 : (uint8_t)(10 + (m * 7) % 141);
        sMoves[m].accuracy = (uint8_t)(m % 5 == 0 ? 0 : 70 + m % 31);
        sMoves[m].goodStatus = sMoves[m].power == 0 && m % 8 == 0;
        sMoves[m].drawback = m == 153;
    }
    sMoves[153].power = 250;
}

static void TestLearnsets(void)
{
    static const uint16_t original[] = {
        CTR_RULES_ENTRY(1, 33), CTR_RULES_ENTRY(1, 45), CTR_RULES_ENTRY(5, 10), CTR_RULES_ENTRY(9, 55),
        CTR_RULES_ENTRY(15, 98), CTR_RULES_ENTRY(22, 44), CTR_RULES_ENTRY(30, 120), CTR_RULES_ENTRY(41, 153),
        CTR_RULES_ENTRY(50, 200), CTR_RULES_LEARNSET_END};
    const unsigned n = sizeof(original) / sizeof(original[0]);
    uint16_t a[32], b[32];
    unsigned seed, own = 0, total = 0;

    for (seed = 0; seed < 300; seed++)
    {
        uint16_t species = (uint16_t)(1 + seed % 400);
        unsigned wrote = CtrRules_BuildLearnset(seed, species, original, a, 32, sMoves, MOVES, 3, 7, seed & 1);
        unsigned i, j;

        CHECK(wrote == n);
        CHECK(a[n - 1] == CTR_RULES_LEARNSET_END);
        CHECK(CtrRules_BuildLearnset(seed, species, original, b, 32, sMoves, MOVES, 3, 7, seed & 1) == n);
        CHECK(memcmp(a, b, n * sizeof(a[0])) == 0); /* the same seed, the same learnset */
        CHECK(sMoves[a[0] & CTR_RULES_MOVE_ID_MASK].power != 0); /* it can fight */
        for (i = 0; i + 1 < n; i++)
        {
            uint16_t move = a[i] & CTR_RULES_MOVE_ID_MASK;
            unsigned level = a[i] >> CTR_RULES_LEVEL_SHIFT;

            CHECK(level == (unsigned)(original[i] >> CTR_RULES_LEVEL_SHIFT));
            CHECK(move != 0 && move < MOVES && sMoves[move].usable);
            CHECK(sMoves[move].power == 0 || sMoves[move].power <= CtrRules_PowerCap(level));
            for (j = 0; j < i; j++)
                CHECK((a[j] & CTR_RULES_MOVE_ID_MASK) != move);
            if (seed & 1)
            {
                own += sMoves[move].type == 3 || sMoves[move].type == 7;
                total++;
            }
        }
    }
    /* TYPED: about half of the moves of the species' own types. */
    CHECK(own * 100 / total >= 35);
    CHECK(CtrRules_BuildLearnset(5, 5, original, a, 4, sMoves, MOVES, 0, 0, false) == 0); /* too small */
    CtrRules_BuildLearnset(1, 10, original, a, 32, sMoves, MOVES, 0, 0, false);
    CtrRules_BuildLearnset(2, 10, original, b, 32, sMoves, MOVES, 0, 0, false);
    CHECK(memcmp(a, b, n * sizeof(a[0])) != 0); /* seeds differ */
}

static uint16_t Evolution(uint32_t seed, uint16_t from, bool similar)
{
    return sEvolvesTo[from] ? CtrRules_PickEvolution(seed, from, sEvolvesTo[from], 0, sSpecies, SPECIES,
                                                     NULL, 0, similar) : 0;
}

static void TestEvolutions(void)
{
    unsigned seed, s;

    for (seed = 0; seed < 40; seed++)
        for (unsigned similar = 0; similar < 2; similar++)
        {
            unsigned near = 0, count = 0;

            for (s = 1; s < SPECIES; s++)
            {
                uint16_t to = Evolution(seed, (uint16_t)s, similar);
                uint16_t walk = (uint16_t)s;
                unsigned steps = 0;

                if (!sEvolvesTo[s])
                    continue;
                CHECK(to != s && to != 0 && sSpecies[to].bst != 0);
                CHECK(to == Evolution(seed, (uint16_t)s, similar));
                CHECK(!sSpecies[to].legendary || sSpecies[sEvolvesTo[s]].legendary);
                /* No cycle: following the new evolutions always ends. */
                while (walk != 0 && sEvolvesTo[walk] && steps < SPECIES)
                {
                    walk = Evolution(seed, walk, similar);
                    steps++;
                }
                CHECK(steps < SPECIES);
                count++;
                {
                    int diff = (int)sSpecies[to].bst - (int)sSpecies[sEvolvesTo[s]].bst;

                    near += (diff < 0 ? -diff : diff) <= 120;
                }
            }
            if (similar)
                CHECK(near * 100 / count >= 90);
        }
    /* Taken targets are not chosen again. */
    {
        uint16_t taken[5];
        unsigned i, j;

        for (i = 0; i < 5; i++)
        {
            taken[i] = CtrRules_PickEvolution(9, 133, 134, i, sSpecies, SPECIES, taken, i, true);
            for (j = 0; j < i; j++)
                CHECK(taken[i] != taken[j]);
        }
    }
}

static void TestTeams(void)
{
    uint64_t state = CtrRules_Stream(3, 1, 2, 3);
    uint16_t chosen[6];
    unsigned i, j, lo, hi;

    CtrRules_BossBstRange(30, &lo, &hi);
    CHECK(lo == 430 && hi == 570);
    for (i = 0; i < 6; i++)
    {
        chosen[i] = CtrRules_PickSpecies(&state, sSpecies, SPECIES, lo, hi, 4, false, chosen, i);
        CHECK(chosen[i] != 0);
        CHECK(sSpecies[chosen[i]].bst >= lo && sSpecies[chosen[i]].bst <= hi);
        CHECK(!sSpecies[chosen[i]].legendary);
        for (j = 0; j < i; j++)
            CHECK(chosen[i] != chosen[j]);
    }
    /* An impossible range widens instead of failing. */
    CHECK(CtrRules_PickSpecies(&state, sSpecies, SPECIES, 900, 950, CTR_RULES_TYPE_NONE, false, NULL, 0) != 0);
    CHECK(!CtrRules_BossAllowsLegendary(54) && CtrRules_BossAllowsLegendary(55));
}

static void TestBestMoves(void)
{
    /* 3 (power 31), 6 (52), 9 (73), 8 (status, good), 153 (250, drawback). */
    static const uint16_t learnset[] = {
        CTR_RULES_ENTRY(1, 3), CTR_RULES_ENTRY(1, 8), CTR_RULES_ENTRY(10, 6), CTR_RULES_ENTRY(20, 9),
        CTR_RULES_ENTRY(30, 153), CTR_RULES_ENTRY(60, 150), CTR_RULES_LEARNSET_END};
    uint16_t out[4];
    unsigned n = CtrRules_BestMoves(learnset, 35, sMoves, MOVES, 0, 0, out), i;
    bool hasStatus = false, has150 = false;

    CHECK(n == 4);
    for (i = 0; i < 4; i++)
    {
        hasStatus |= out[i] == 8;
        has150 |= out[i] == 150; /* above its level */
    }
    CHECK(hasStatus && !has150);
    CHECK(CtrRules_BestMoves(learnset, 1, sMoves, MOVES, 0, 0, out) == 2);
}

static void TestDifficulty(void)
{
    CHECK(CtrRules_ScaleLevel(50, CTR_DIFFICULTY_NORMAL) == 50);
    CHECK(CtrRules_ScaleLevel(50, CTR_DIFFICULTY_EASY) == 45);
    CHECK(CtrRules_ScaleLevel(40, CTR_DIFFICULTY_HARD) == 46);
    CHECK(CtrRules_ScaleLevel(40, CTR_DIFFICULTY_EXTREME) == 50);
    CHECK(CtrRules_ScaleLevel(90, CTR_DIFFICULTY_EXTREME) == 100);
    CHECK(CtrRules_ScaleLevel(2, CTR_DIFFICULTY_EASY) == 2);
    CHECK(CtrRules_AiFlags(CTR_AI_CHECK_BAD_MOVE, CTR_DIFFICULTY_NORMAL, true) == CTR_AI_CHECK_BAD_MOVE);
    CHECK(CtrRules_AiFlags(CTR_AI_CHECK_BAD_MOVE, CTR_DIFFICULTY_EASY, false) == 0);
    CHECK(CtrRules_AiFlags(0, CTR_DIFFICULTY_HARD, false) & CTR_AI_TRY_TO_FAINT);
    CHECK(CtrRules_AiFlags(0, CTR_DIFFICULTY_HARD, true) & CTR_AI_HP_AWARE);
    CHECK(CtrRules_MinIv(CTR_DIFFICULTY_EXTREME, false) == 31);
    CHECK(CtrRules_EvPerStat(CTR_DIFFICULTY_EXTREME, true) * 6 <= 510);
    CHECK(CtrRules_BossTeamSize(3, CTR_DIFFICULTY_NORMAL, 6) == 3);
    CHECK(CtrRules_BossTeamSize(3, CTR_DIFFICULTY_HARD, 6) == 4);
    CHECK(CtrRules_BossTeamSize(3, CTR_DIFFICULTY_EXTREME, 6) == 6);
    CHECK(CtrRules_BossTeamSize(3, CTR_DIFFICULTY_EXTREME, 3) == 3);
}

int main(void)
{
    MakeTables();
    TestLearnsets();
    TestEvolutions();
    TestTeams();
    TestBestMoves();
    TestDifficulty();
    if (failures)
    {
        fprintf(stderr, "randomizer_rules_test: %d failure(s)\n", failures);
        return 1;
    }
    printf("randomizer_rules_test: ok\n");
    return 0;
}
