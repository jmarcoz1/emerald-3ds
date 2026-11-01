/*
 * The randomizer's rules beyond species, the game's side
 * (include/3ds_randomizer.h, include/3ds_randomizer_rules.h): random
 * learnsets and evolutions, difficulty levels, boss teams, double battles
 * against every trainer and the hand-crafted strategy teams
 * (src/3ds_strategy_teams.h). Every setting is read straight from
 * settings.txt, like the species randomizer's; with all of them at their
 * defaults (NORMAL, nothing random) the game is the original.
 */
#include "global.h"
#include "battle.h"
#include "data.h"
#include "pokemon.h"
#include "constants/abilities.h"
#include "constants/battle.h"
#include "constants/battle_ai.h"
#include "constants/battle_move_effects.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/species.h"
#include "constants/trainers.h"
#include "3ds_platform.h"
#include "3ds_randomizer.h"
#include "3ds_randomizer_rules.h"

/* The core's copy of the AI flags is the game's. */
_Static_assert(AI_SCRIPT_CHECK_BAD_MOVE == CTR_AI_CHECK_BAD_MOVE, "ai flags");
_Static_assert(AI_SCRIPT_TRY_TO_FAINT == CTR_AI_TRY_TO_FAINT, "ai flags");
_Static_assert(AI_SCRIPT_CHECK_VIABILITY == CTR_AI_CHECK_VIABILITY, "ai flags");
_Static_assert(AI_SCRIPT_SETUP_FIRST_TURN == CTR_AI_SETUP_FIRST_TURN, "ai flags");
_Static_assert(AI_SCRIPT_PREFER_POWER_EXTREMES == CTR_AI_PREFER_POWER_EXTREMES, "ai flags");
_Static_assert(AI_SCRIPT_PREFER_BATON_PASS == CTR_AI_PREFER_BATON_PASS, "ai flags");
_Static_assert(AI_SCRIPT_HP_AWARE == CTR_AI_HP_AWARE, "ai flags");
_Static_assert(LEVEL_UP_END == CTR_RULES_LEARNSET_END, "learnset layout");
_Static_assert(LEVEL_UP_MOVE_ID == CTR_RULES_MOVE_ID_MASK, "learnset layout");
_Static_assert(TYPE_NORMAL == 0, "the core takes Normal as type 0");

extern const struct Evolution gEvolutionTable[][EVOS_PER_MON];

#include "3ds_strategy_teams.h"

/* ---- settings ---- */

enum { MODE_OFF, MODE_AUTO, MODE_ON };

unsigned CtrRandomizer_Difficulty(void)
{
    int value = CtrSettings_GetInt("difficulty", CTR_DIFFICULTY_NORMAL);

    return value >= 0 && value < CTR_DIFFICULTY_COUNT ? (unsigned)value : CTR_DIFFICULTY_NORMAL;
}

/* OFF, AUTO (on at HARD and EXTREME) or ON (the default: every difficulty). */
static bool AutoSetting(const char *key)
{
    int value = CtrSettings_GetInt(key, MODE_ON);

    if (value == MODE_ON)
        return true;
    return value == MODE_AUTO && CtrRandomizer_Difficulty() >= CTR_DIFFICULTY_HARD;
}

static bool Setting(const char *key, int fallback)
{
    return CtrSettings_GetInt(key, fallback) == 1;
}

bool CtrRandomizer_DoublesEnabled(void)
{
    return AutoSetting("all_doubles");
}

bool CtrRandomizer_StrategyEnabled(void)
{
    return AutoSetting("strategy_teams");
}

static bool BossTeamsEnabled(void)
{
    return Setting("rand_bosses", 1);
}

/* ---- the tables the rules read ---- */

static CtrRulesSpecies sSpecies[NUM_SPECIES];
static CtrRulesMove sMoves[MOVES_COUNT];
static bool sTablesBuilt;

static bool IsLegendary(u16 species)
{
    switch (species)
    {
    case SPECIES_ARTICUNO: case SPECIES_ZAPDOS: case SPECIES_MOLTRES:
    case SPECIES_MEWTWO: case SPECIES_MEW:
    case SPECIES_RAIKOU: case SPECIES_ENTEI: case SPECIES_SUICUNE:
    case SPECIES_LUGIA: case SPECIES_HO_OH: case SPECIES_CELEBI:
    case SPECIES_REGIROCK: case SPECIES_REGICE: case SPECIES_REGISTEEL:
    case SPECIES_LATIAS: case SPECIES_LATIOS: case SPECIES_KYOGRE:
    case SPECIES_GROUDON: case SPECIES_RAYQUAZA: case SPECIES_JIRACHI:
    case SPECIES_DEOXYS:
        return true;
    }
    return false;
}

static bool GoodStatus(u16 move)
{
    switch (move)
    {
    case MOVE_SWORDS_DANCE: case MOVE_DRAGON_DANCE: case MOVE_CALM_MIND:
    case MOVE_BULK_UP: case MOVE_AMNESIA: case MOVE_TOXIC:
    case MOVE_THUNDER_WAVE: case MOVE_WILL_O_WISP: case MOVE_SPORE:
    case MOVE_SLEEP_POWDER: case MOVE_HYPNOSIS: case MOVE_LOVELY_KISS:
    case MOVE_STUN_SPORE: case MOVE_GLARE: case MOVE_YAWN:
    case MOVE_RECOVER: case MOVE_SOFT_BOILED: case MOVE_MILK_DRINK:
    case MOVE_SLACK_OFF: case MOVE_MOONLIGHT: case MOVE_MORNING_SUN:
    case MOVE_SYNTHESIS: case MOVE_WISH: case MOVE_LEECH_SEED:
    case MOVE_REFLECT: case MOVE_LIGHT_SCREEN: case MOVE_SPIKES:
    case MOVE_AGILITY:
        return true;
    }
    return false;
}

static bool Drawback(u8 effect)
{
    switch (effect)
    {
    case EFFECT_EXPLOSION: case EFFECT_RECHARGE: case EFFECT_DREAM_EATER:
    case EFFECT_FOCUS_PUNCH: case EFFECT_SNORE: case EFFECT_SKY_ATTACK:
    case EFFECT_SOLAR_BEAM: case EFFECT_SKULL_BASH: case EFFECT_FUTURE_SIGHT:
    case EFFECT_RAZOR_WIND: case EFFECT_SPIT_UP: case EFFECT_OHKO:
    case EFFECT_FALSE_SWIPE: case EFFECT_FRUSTRATION: case EFFECT_PRESENT:
    case EFFECT_FAKE_OUT: case EFFECT_COUNTER: case EFFECT_MIRROR_COAT:
    case EFFECT_BIDE: case EFFECT_BEAT_UP:
        return true;
    }
    return false;
}

static void BuildTables(void)
{
    u16 s, m;
    int i;

    if (sTablesBuilt)
        return;
    for (s = 0; s < NUM_SPECIES; s++)
    {
        const struct SpeciesInfo *info = &gSpeciesInfo[s];
        CtrRulesSpecies *out = &sSpecies[s];

        out->bst = 0;
        if (!CtrRandomizer_SpeciesEligible(s))
            continue;
        out->bst = info->baseHP + info->baseAttack + info->baseDefense
                 + info->baseSpeed + info->baseSpAttack + info->baseSpDefense;
        out->type1 = info->types[0];
        out->type2 = info->types[1];
        out->legendary = IsLegendary(s);
        out->evolves = 0;
        for (i = 0; i < EVOS_PER_MON; i++)
            if (gEvolutionTable[s][i].method != 0 && gEvolutionTable[s][i].targetSpecies != SPECIES_NONE)
                out->evolves = 1;
    }
    for (m = 0; m < MOVES_COUNT; m++)
    {
        const struct BattleMove *move = &gBattleMoves[m];
        CtrRulesMove *out = &sMoves[m];

        out->power = move->power;
        out->type = move->type;
        out->accuracy = move->accuracy;
        out->usable = m != MOVE_NONE && m != MOVE_STRUGGLE;
        out->goodStatus = move->power == 0 && GoodStatus(m);
        out->drawback = move->power != 0 && Drawback(move->effect);
    }
    sTablesBuilt = true;
}

/* ---- random learnsets ---- */

/* Every species' new learnset, end to end (the vanilla ones hold about 4400
 * entries); a species that does not fit keeps its own. */
#define LEARNSET_POOL_SIZE 5120

static u16 sLearnsetPool[LEARNSET_POOL_SIZE];
static u16 sLearnsetStart[NUM_SPECIES];
static bool sLearnsetsBuilt;
static uint32_t sLearnsetSeed;
static bool sLearnsetTyped;

bool CtrRandomizer_LearnsetsEnabled(void)
{
    return Setting("rand_moves", 0);
}

static void BuildLearnsets(uint32_t seed, bool typed)
{
    unsigned used = 0;
    u16 s;

    BuildTables();
    for (s = 0; s < NUM_SPECIES; s++)
    {
        unsigned wrote = 0;

        sLearnsetStart[s] = 0xFFFF;
        if (sSpecies[s].bst == 0)
            continue;
        wrote = CtrRules_BuildLearnset(seed, s, gLevelUpLearnsets[s], &sLearnsetPool[used],
                                       LEARNSET_POOL_SIZE - used, sMoves, MOVES_COUNT,
                                       sSpecies[s].type1, sSpecies[s].type2, typed);
        if (wrote == 0)
            continue;
        sLearnsetStart[s] = used;
        used += wrote;
    }
    sLearnsetSeed = seed;
    sLearnsetTyped = typed;
    sLearnsetsBuilt = true;
    CtrLog_Write(CTR_LOG_GAME, "randomizer: learnsets for seed %lu (%u entries)", (unsigned long)seed, used);
}

const u16 *CtrRandomizer_Learnset(u16 species)
{
    uint32_t seed;
    bool typed;

    if (species >= NUM_SPECIES || !CtrRandomizer_LearnsetsEnabled())
        return gLevelUpLearnsets[species < NUM_SPECIES ? species : SPECIES_NONE];
    seed = CtrRandomizer_Seed();
    typed = CtrSettings_GetInt("rand_moves_style", 0) == 0;
    if (!sLearnsetsBuilt || sLearnsetSeed != seed || sLearnsetTyped != typed)
        BuildLearnsets(seed, typed);
    if (sLearnsetStart[species] == 0xFFFF)
        return gLevelUpLearnsets[species];
    return &sLearnsetPool[sLearnsetStart[species]];
}

/* ---- random evolutions ---- */

u16 CtrRandomizer_Evolution(u16 species, u16 target)
{
    u16 taken[EVOS_PER_MON];
    uint32_t seed;
    bool similar;
    int i;

    if (!Setting("rand_evos", 0) || species >= NUM_SPECIES || target == SPECIES_NONE)
        return target;
    BuildTables();
    if (sSpecies[species].bst == 0 || target >= NUM_SPECIES || sSpecies[target].bst == 0)
        return target;
    seed = CtrRandomizer_Seed();
    similar = CtrSettings_GetInt("rand_evos_style", 0) == 0;
    /* A species' evolutions are chosen in slot order, each different from
     * the ones before it, so Eevee still has five. */
    for (i = 0; i < EVOS_PER_MON; i++)
    {
        u16 original = gEvolutionTable[species][i].targetSpecies;

        if (gEvolutionTable[species][i].method == 0 || original == SPECIES_NONE)
            break;
        taken[i] = CtrRules_PickEvolution(seed, species, original, i, sSpecies, NUM_SPECIES,
                                          taken, i, similar);
        if (original == target)
            return taken[i];
    }
    return target;
}

/* ---- trainers ---- */

#define DOMAIN_BOSS   0x424F5353u /* "BOSS" */
#define DOMAIN_EXTRA  0x45585452u /* "EXTR" */
#define DOMAIN_STRAT  0x53545241u /* "STRA" */

static bool IsBoss(u16 trainerNum)
{
    switch (gTrainers[trainerNum].trainerClass)
    {
    case TRAINER_CLASS_LEADER: case TRAINER_CLASS_ELITE_FOUR: case TRAINER_CLASS_CHAMPION:
    case TRAINER_CLASS_RIVAL: case TRAINER_CLASS_AQUA_LEADER: case TRAINER_CLASS_MAGMA_LEADER:
        return true;
    }
    return false;
}

static const struct CtrStrategyTeam *StrategyTeam(u16 trainerNum)
{
    unsigned i;

    if (!CtrRandomizer_StrategyEnabled())
        return NULL;
    for (i = 0; i < ARRAY_COUNT(sStrategyTeams); i++)
        if (sStrategyTeams[i].trainer == trainerNum)
            return &sStrategyTeams[i];
    return NULL;
}

/* Whether this trainer gets the boss treatment: a boss class with BOSS
 * TEAMS on, or a strategy team. */
static bool TreatAsBoss(u16 trainerNum)
{
    return (BossTeamsEnabled() && IsBoss(trainerNum)) || StrategyTeam(trainerNum) != NULL;
}

uint32_t CtrRandomizer_TrainerAiFlags(u16 trainerNum)
{
    uint32_t vanilla = gTrainers[trainerNum].aiFlags;

    return CtrRules_AiFlags(vanilla, CtrRandomizer_Difficulty(), TreatAsBoss(trainerNum));
}

/* The vanilla party's i-th Pokemon, whatever its layout. */
static void TrainerMon(u16 trainerNum, int i, u16 *species, u8 *level, u8 *iv, bool *customMoves)
{
    const struct Trainer *trainer = &gTrainers[trainerNum];

    *customMoves = (trainer->partyFlags & F_TRAINER_PARTY_CUSTOM_MOVESET) != 0;
    switch (trainer->partyFlags)
    {
    case 0:
        *species = trainer->party.NoItemDefaultMoves[i].species;
        *level = trainer->party.NoItemDefaultMoves[i].lvl;
        *iv = trainer->party.NoItemDefaultMoves[i].iv;
        break;
    case F_TRAINER_PARTY_CUSTOM_MOVESET:
        *species = trainer->party.NoItemCustomMoves[i].species;
        *level = trainer->party.NoItemCustomMoves[i].lvl;
        *iv = trainer->party.NoItemCustomMoves[i].iv;
        break;
    case F_TRAINER_PARTY_HELD_ITEM:
        *species = trainer->party.ItemDefaultMoves[i].species;
        *level = trainer->party.ItemDefaultMoves[i].lvl;
        *iv = trainer->party.ItemDefaultMoves[i].iv;
        break;
    default:
        *species = trainer->party.ItemCustomMoves[i].species;
        *level = trainer->party.ItemCustomMoves[i].lvl;
        *iv = trainer->party.ItemCustomMoves[i].iv;
        break;
    }
}

static void SetMoves(struct Pokemon *mon, const u16 *moves)
{
    int j;

    for (j = 0; j < MAX_MON_MOVES; j++)
    {
        u16 move = moves[j];

        SetMonData(mon, MON_DATA_MOVE1 + j, &move);
        SetMonData(mon, MON_DATA_PP1 + j, &gBattleMoves[move].pp);
    }
}

/* A boss's best four from the learnset it has now (random or not). */
static void GiveBestMoves(struct Pokemon *mon)
{
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u8 level = GetMonData(mon, MON_DATA_LEVEL, NULL);
    u16 moves[MAX_MON_MOVES];

    BuildTables();
    if (CtrRules_BestMoves(CtrRandomizer_Learnset(species), level, sMoves, MOVES_COUNT,
                           sSpecies[species].type1, sSpecies[species].type2, moves) > 0)
        SetMoves(mon, moves);
}

/* The same Pokemon, as another species or at another level: its
 * personality, trainer id and held item are kept; moves are the level-up
 * ones of what it is now. */
static void Recreate(struct Pokemon *mon, u16 species, u8 level, u8 fixedIV)
{
    u32 personality = GetMonData(mon, MON_DATA_PERSONALITY, NULL);
    u32 otId = GetMonData(mon, MON_DATA_OT_ID, NULL);
    u16 heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, NULL);

    CreateMon(mon, species, level, fixedIV, TRUE, personality, OT_ID_PRESET, otId);
    SetMonData(mon, MON_DATA_HELD_ITEM, &heldItem);
}

/* The type a themed boss (a gym leader, the Elite Four) prefers: the one
 * most of its own Pokemon have. */
static u8 ThemeType(u16 trainerNum)
{
    u8 counts[NUMBER_OF_MON_TYPES] = {0};
    u8 best = CTR_RULES_TYPE_NONE, bestCount = 0;
    int i, t;

    if (gTrainers[trainerNum].trainerClass != TRAINER_CLASS_LEADER
     && gTrainers[trainerNum].trainerClass != TRAINER_CLASS_ELITE_FOUR)
        return CTR_RULES_TYPE_NONE;
    for (i = 0; i < gTrainers[trainerNum].partySize; i++)
    {
        u16 species;
        u8 level, iv;
        bool custom;

        TrainerMon(trainerNum, i, &species, &level, &iv, &custom);
        counts[gSpeciesInfo[species].types[0]]++;
        if (gSpeciesInfo[species].types[1] != gSpeciesInfo[species].types[0])
            counts[gSpeciesInfo[species].types[1]]++;
    }
    for (t = 0; t < NUMBER_OF_MON_TYPES; t++)
        if (counts[t] > bestCount)
        {
            bestCount = counts[t];
            best = t;
        }
    return best;
}

static u16 BossSpecies(uint64_t *state, u8 level, u8 theme, const u16 *exclude, unsigned excludeCount)
{
    unsigned lo, hi;

    BuildTables();
    CtrRules_BossBstRange(level, &lo, &hi);
    return CtrRules_PickSpecies(state, sSpecies, NUM_SPECIES, lo, hi, theme,
                                CtrRules_BossAllowsLegendary(level), exclude, excludeCount);
}

/* A Pokemon of about the strength of `like` (base stats within 15%). */
static u16 SimilarSpecies(uint64_t *state, u16 like, const u16 *exclude, unsigned excludeCount)
{
    unsigned bst;

    BuildTables();
    bst = sSpecies[like].bst != 0 ? sSpecies[like].bst : 300;
    return CtrRules_PickSpecies(state, sSpecies, NUM_SPECIES, bst * 85 / 100, bst * 115 / 100,
                                CTR_RULES_TYPE_NONE, false, exclude, excludeCount);
}

static void SetIvsEvs(struct Pokemon *mon, unsigned minIv, unsigned ev)
{
    int stat;

    for (stat = 0; stat < NUM_STATS; stat++)
    {
        if (minIv != 0)
        {
            u8 iv = GetMonData(mon, MON_DATA_HP_IV + stat, NULL);

            if (iv < minIv)
            {
                iv = minIv;
                SetMonData(mon, MON_DATA_HP_IV + stat, &iv);
            }
        }
        if (ev != 0)
        {
            u8 value = ev;

            SetMonData(mon, MON_DATA_HP_EV + stat, &value);
        }
    }
}

/* A strategy Pokemon: the nature from a personality of its own (no game
 * RNG), its named ability, its item, IVs, EVs and moves. */
static void CreateStrategyMon(struct Pokemon *mon, const struct CtrStrategyMon *def, u16 trainerNum,
                              int slot, unsigned difficulty)
{
    uint64_t state = CtrRules_Stream(CtrRandomizer_Seed(), DOMAIN_STRAT, trainerNum, slot);
    u32 personality = CtrRules_Next(&state);
    u8 level = CtrRules_ScaleLevel(def->level, difficulty);
    u8 abilityNum = gSpeciesInfo[def->species].abilities[1] == def->ability ? 1 : 0;
    u16 heldItem = def->heldItem;
    int stat;

    personality -= personality % NUM_NATURES;
    personality += def->nature;
    CreateMon(mon, def->species, level, def->iv, TRUE, personality, OT_ID_RANDOM_NO_SHINY, 0);
    SetMonData(mon, MON_DATA_ABILITY_NUM, &abilityNum);
    SetMonData(mon, MON_DATA_HELD_ITEM, &heldItem);
    for (stat = 0; stat < NUM_STATS; stat++)
    {
        u8 ev = def->evs[stat];

        SetMonData(mon, MON_DATA_HP_EV + stat, &ev);
    }
    SetIvsEvs(mon, CtrRules_MinIv(difficulty, true), 0);
    SetMoves(mon, def->moves);
    CalculateMonStats(mon);
}

/* The level the extras of a team are made at: its strongest one's. */
static u8 TopLevel(struct Pokemon *party, unsigned count)
{
    u8 top = 1;
    unsigned i;

    for (i = 0; i < count; i++)
    {
        u8 level = GetMonData(&party[i], MON_DATA_LEVEL, NULL);

        if (level > top)
            top = level;
    }
    return top;
}

static bool AnyRuleActive(u16 trainerNum)
{
    return CtrRandomizer_Difficulty() != CTR_DIFFICULTY_NORMAL
        || CtrRandomizer_LearnsetsEnabled()
        || CtrRandomizer_DoublesEnabled()
        || StrategyTeam(trainerNum) != NULL
        || (BossTeamsEnabled() && IsBoss(trainerNum) && CtrRandomizer_Enabled(CTR_RANDOMIZE_TRAINERS));
}

/* Whether this battle becomes a double one: a single battle against one
 * trainer, with the player able to send out two. */
static bool ForceDoubles(u16 trainerNum, bool8 firstTrainer)
{
    if (!CtrRandomizer_DoublesEnabled() || !firstTrainer)
        return false;
    if (gBattleTypeFlags & (BATTLE_TYPE_DOUBLE | BATTLE_TYPE_TWO_OPPONENTS | BATTLE_TYPE_MULTI
                            | BATTLE_TYPE_INGAME_PARTNER | BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED))
        return false;
    if (gTrainers[trainerNum].doubleBattle)
        return false;
    return GetMonsStateToDoubles() == PLAYER_HAS_TWO_USABLE_MONS;
}

u8 CtrRandomizer_AdjustTrainerParty(struct Pokemon *party, u16 trainerNum, u8 monsCount, bool8 firstTrainer)
{
    const struct CtrStrategyTeam *team;
    unsigned difficulty, max, count, i;
    bool boss, randomSpecies, doubles;
    uint32_t seed;
    u16 chosen[PARTY_SIZE];

    if (trainerNum >= TRAINERS_COUNT || monsCount == 0 || !AnyRuleActive(trainerNum))
        return monsCount;
    difficulty = CtrRandomizer_Difficulty();
    max = (gBattleTypeFlags & BATTLE_TYPE_TWO_OPPONENTS) ? PARTY_SIZE / 2 : PARTY_SIZE;
    seed = CtrRandomizer_Seed();
    boss = TreatAsBoss(trainerNum);
    doubles = ForceDoubles(trainerNum, firstTrainer);
    team = StrategyTeam(trainerNum);

    if (team != NULL)
    {
        /* A strategy team replaces the party, randomizer or not. */
        count = team->count < max ? team->count : max;
        for (i = 0; i < count; i++)
            CreateStrategyMon(&party[i], &team->mons[i], trainerNum, i, difficulty);
        for (; i < max; i++)
            ZeroMonData(&party[i]);
        if (doubles && count >= 2)
            gBattleTypeFlags |= BATTLE_TYPE_DOUBLE;
        return count;
    }

    count = monsCount;
    randomSpecies = boss && BossTeamsEnabled() && CtrRandomizer_Enabled(CTR_RANDOMIZE_TRAINERS);
    {
        uint64_t state = CtrRules_Stream(seed, DOMAIN_BOSS, trainerNum, 0);
        u8 theme = randomSpecies ? ThemeType(trainerNum) : CTR_RULES_TYPE_NONE;

        for (i = 0; i < count; i++)
        {
            u16 vanillaSpecies, species = GetMonData(&party[i], MON_DATA_SPECIES, NULL);
            u8 vanillaLevel, ivByte, level;
            bool custom;
            u16 moves[MAX_MON_MOVES];
            int j;

            TrainerMon(trainerNum, i, &vanillaSpecies, &vanillaLevel, &ivByte, &custom);
            level = CtrRules_ScaleLevel(vanillaLevel, difficulty);
            for (j = 0; j < MAX_MON_MOVES; j++)
                moves[j] = GetMonData(&party[i], MON_DATA_MOVE1 + j, NULL);
            /* A boss of the randomized game draws from the strong species
             * of its level instead of the permutation. */
            if (randomSpecies)
            {
                u16 strong = BossSpecies(&state, level, theme, chosen, i);

                if (strong != SPECIES_NONE)
                    species = strong;
            }
            chosen[i] = species;
            Recreate(&party[i], species, level, ivByte * MAX_PER_STAT_IVS / 255);
            /* Custom moves stay with their own species, unless learnsets
             * are random (then everyone fights with what it learns). */
            if (custom && species == vanillaSpecies && !CtrRandomizer_LearnsetsEnabled())
                SetMoves(&party[i], moves);
            else if (boss && (randomSpecies || CtrRandomizer_LearnsetsEnabled()))
                GiveBestMoves(&party[i]);
        }
    }

    /* A boss's team grows on HARD and EXTREME. */
    if (boss)
    {
        unsigned size = CtrRules_BossTeamSize(count, difficulty, max);
        uint64_t state = CtrRules_Stream(seed, DOMAIN_EXTRA, trainerNum, 0);
        u8 level = TopLevel(party, count);
        u8 theme = ThemeType(trainerNum);

        for (; count < size; count++)
        {
            u16 species = BossSpecies(&state, level, theme, chosen, count);

            if (species == SPECIES_NONE)
                break;
            chosen[count] = species;
            CreateMon(&party[count], species, level, 0, FALSE, 0, OT_ID_RANDOM_NO_SHINY, 0);
            GiveBestMoves(&party[count]);
        }
    }

    /* A double battle needs a second Pokemon. */
    if (doubles && count < 2 && count < max)
    {
        uint64_t state = CtrRules_Stream(seed, DOMAIN_EXTRA, trainerNum, 1);
        u16 first = GetMonData(&party[0], MON_DATA_SPECIES, NULL);
        u8 level = TopLevel(party, count);
        u16 species = boss ? BossSpecies(&state, level, ThemeType(trainerNum), chosen, count)
                           : SimilarSpecies(&state, first, chosen, count);

        if (species != SPECIES_NONE)
        {
            chosen[count] = species;
            CreateMon(&party[count], species, level, 0, FALSE, 0, OT_ID_RANDOM_NO_SHINY, 0);
            if (boss)
                GiveBestMoves(&party[count]);
            count++;
        }
    }
    if (doubles && count >= 2)
        gBattleTypeFlags |= BATTLE_TYPE_DOUBLE;

    /* IVs, EVs and held items by difficulty. */
    {
        unsigned minIv = CtrRules_MinIv(difficulty, boss);
        unsigned ev = CtrRules_EvPerStat(difficulty, boss);

        for (i = 0; i < count; i++)
        {
            u16 item = GetMonData(&party[i], MON_DATA_HELD_ITEM, NULL);

            SetIvsEvs(&party[i], minIv, ev);
            if (item == ITEM_NONE && difficulty >= CTR_DIFFICULTY_HARD
             && (boss || (difficulty == CTR_DIFFICULTY_EXTREME && i == count - 1)))
            {
                item = (boss && difficulty == CTR_DIFFICULTY_EXTREME && i == count - 1)
                     ? ITEM_LEFTOVERS : ITEM_SITRUS_BERRY;
                SetMonData(&party[i], MON_DATA_HELD_ITEM, &item);
            }
            CalculateMonStats(&party[i]);
        }
    }
    return count;
}
