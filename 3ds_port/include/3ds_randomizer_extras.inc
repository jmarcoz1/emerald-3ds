/*
 * The seeded species randomizer's OPTIONS cells (include/3ds_randomizer.h),
 * for the coordinator to put in gCtrExtras[] of src/3ds_extras.c, before its
 * last line ({0}), with #include "3ds_randomizer.h" at the top of that file:
 *
 *     #include "3ds_randomizer_extras.inc"
 *
 * A RANDOMIZER cell on CHEATS' first screen opens a third screen of its own.
 * Every value is off (seed 0) until set; the game reads them from
 * settings.txt (rand_wild, rand_starters, rand_trainers, rand_seed) whether
 * or not these cells are in the build.
 *
 * Random learnsets and evolutions share that screen; the difficulty cells
 * (difficulty, rand_bosses, all_doubles, strategy_teams) have a fourth
 * (CHEATS 4), opened from the third. Their fallbacks are the game's own
 * behaviour: NORMAL, and AUTO only acts above NORMAL.
 */
    {CTR_EXTRAS_CHEATS, CTR_TEXT("RANDOMIZER", "ALEATORIO", "ALÉATOIRE", "CASUALE", "ALEATÓRIO", "ZUFALL", "ランダム"), "rand_open", 0, 0, gCtrRandomizerOpenText, CtrRandomizer_Open, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 2), CTR_TEXT("RANDOMIZER", "ALEATORIO", "ALÉATOIRE", "CASUALE", "ALEATÓRIO", "ZUFALL", "ランダム"), "rand_back", 0, 0, gCtrRandomizerBackText, CtrRandomizer_Back, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 2), CTR_TEXT("RANDOM WILD", "SALVAJES AL AZAR", "SAUV. ALÉAT.", "SELV. CASUALI", "SELVAGENS ALEAT.", "WILDE ZUFÄLLIG", "ランダム やせい"), "rand_wild", 2, 0, gCtrExtrasOffOn, NULL, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 2), CTR_TEXT("RANDOM STARTERS", "INICIALES AL AZAR", "DÉPARTS ALÉAT.", "INIZIALI CAS.", "INICIAIS ALEAT.", "STARTER ZUFÄLL.", "ランダム さいしょ"), "rand_starters", 2, 0, gCtrExtrasOffOn, NULL, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 2), CTR_TEXT("RANDOM TRAINERS", "ENTREN. AL AZAR", "DRESS. ALÉAT.", "ALLEN. CASUALI", "TREIN. ALEAT.", "TRAINER ZUFÄLL.", "ランダム トレーナー"), "rand_trainers", 2, 0, gCtrExtrasOffOn, NULL, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 2), CTR_TEXT("SEED", "SEMILLA", "GRAINE", "SEME", "SEMENTE", "SEED", "シード"), "rand_seed", 0, 0, NULL, NULL, CtrRandomizer_StepSeed, CtrRandomizer_SeedText},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 2), CTR_TEXT("NEW SEED", "NUEVA SEMILLA", "NOUV. GRAINE", "NUOVO SEME", "NOVA SEMENTE", "NEUER SEED", "あたらしい シード"), "rand_new_seed", 0, 0, gCtrRandomizerNewSeedText, CtrRandomizer_NewSeed, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 2), CTR_TEXT("RANDOM MOVES", "MOVIM. AL AZAR", "CAPAC. ALÉAT.", "MOSSE CASUALI", "GOLPES ALEAT.", "ATTACKEN ZUF.", "ランダム わざ"), "rand_moves", 2, 0, gCtrExtrasOffOn, NULL, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 2), CTR_TEXT("MOVES BY", "MOVIM. SEGÚN", "CAPACITÉS", "MOSSE PER", "GOLPES POR", "ATTACKEN", "わざの きめかた"), "rand_moves_style", 2, 0, gCtrRandomizerMoveStyleText, NULL, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 2), CTR_TEXT("RANDOM EVOS", "EVOL. AL AZAR", "ÉVOL. ALÉAT.", "EVOL. CASUALI", "EVOL. ALEAT.", "ENTW. ZUFÄLL.", "ランダム しんか"), "rand_evos", 2, 0, gCtrExtrasOffOn, NULL, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 2), CTR_TEXT("EVOS INTO", "EVOLUCIONA A", "ÉVOLUE EN", "EVOLVE IN", "EVOLUI PARA", "ENTW. ZU", "しんかさき"), "rand_evos_style", 2, 0, gCtrRandomizerEvoStyleText, NULL, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 2), CTR_TEXT("DIFFICULTY", "DIFICULTAD", "DIFFICULTÉ", "DIFFICOLTÀ", "DIFICULDADE", "SCHWIERIGKEIT", "なんいど"), "rand_difficulty_open", 0, 0, gCtrRandomizerOpenText, CtrRandomizer_OpenDifficulty, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 3), CTR_TEXT("DIFFICULTY", "DIFICULTAD", "DIFFICULTÉ", "DIFFICOLTÀ", "DIFICULDADE", "SCHWIERIGKEIT", "なんいど"), "rand_difficulty_back", 0, 0, gCtrRandomizerBackText, CtrRandomizer_Open, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 3), CTR_TEXT("DIFFICULTY", "DIFICULTAD", "DIFFICULTÉ", "DIFFICOLTÀ", "DIFICULDADE", "SCHWIERIGKEIT", "なんいど"), "difficulty", 4, 1, gCtrRandomizerDifficultyText, NULL, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 3), CTR_TEXT("BOSS TEAMS", "EQUIPOS JEFE", "ÉQUIPES BOSS", "SQUADRE BOSS", "TIMES CHEFES", "BOSS-TEAMS", "ボスの チーム"), "rand_bosses", 2, 1, gCtrExtrasOffOn, NULL, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 3), CTR_TEXT("ALL DOUBLES", "TODO DOBLES", "TOUT EN DUO", "TUTTE DOPPIE", "TUDO EM DUPLA", "NUR DOPPEL", "ぜんぶ ダブル"), "all_doubles", 3, 1, gCtrRandomizerAutoText, NULL, NULL, NULL},
    {CTR_EXTRAS_SCREEN(CTR_EXTRAS_CHEATS, 3), CTR_TEXT("STRATEGY TEAMS", "EQUIPOS TÁCTICOS", "ÉQUIPES TACT.", "SQUADRE TATT.", "TIMES TÁTICOS", "TAKTIK-TEAMS", "さくせん チーム"), "strategy_teams", 3, 1, gCtrRandomizerAutoText, NULL, NULL, NULL},
