/*
 * Hand-crafted strategy teams (included once, by src/3ds_randomizer_ext.c).
 *
 * With STRATEGY TEAMS on (OPTIONS > MODES > DIFFICULTY; AUTO turns them on
 * at HARD and EXTREME), the trainers below fight with these teams instead of
 * their own, randomizer or not: each is built around a double battle plan
 * that the third generation's moves and abilities can play. Levels are the
 * trainer's own (vanilla) and scale with the difficulty like everyone else's;
 * natures, abilities, held items, IVs and EVs are as written. The ability is
 * named, not numbered: the species' slot that has it is used (its first slot
 * when it has neither).
 *
 * To edit a team, change its rows; to add one, add a TEAM line to
 * sStrategyTeams. Species, moves, items and abilities are pokeemerald's
 * constants, so a typo fails the build instead of the game.
 */

#define EVS(hp, atk, def, spe, spa, spd) {hp, atk, def, spe, spa, spd}
#define EV_PHYS       EVS(4, 252, 0, 252, 0, 0)
#define EV_SPEC       EVS(4, 0, 0, 252, 252, 0)
#define EV_PHYS_BULK  EVS(252, 252, 0, 0, 0, 4)
#define EV_SPEC_BULK  EVS(252, 0, 0, 0, 252, 4)
#define EV_WALL       EVS(252, 0, 128, 0, 0, 128)
#define EV_SUPPORT    EVS(252, 0, 4, 252, 0, 0)
#define EV_EARLY      EVS(32, 32, 32, 32, 32, 32)

#define MON(species_, lvl_, nature_, ability_, item_, iv_, evs_, m1, m2, m3, m4) \
    {SPECIES_##species_, lvl_, NATURE_##nature_, ABILITY_##ability_, ITEM_##item_, iv_, evs_, \
     {MOVE_##m1, MOVE_##m2, MOVE_##m3, MOVE_##m4}}

struct CtrStrategyMon
{
    u16 species;
    u8 level;
    u8 nature;
    u8 ability;
    u16 heldItem;
    u8 iv;
    u8 evs[NUM_STATS];
    u16 moves[MAX_MON_MOVES];
};

struct CtrStrategyTeam
{
    u16 trainer;
    u8 count;
    const struct CtrStrategyMon *mons;
};

/* ---- Gym leaders ---- */

// Roxanne: Explosion lead, Sturdy walls behind it.
static const struct CtrStrategyMon sStrat_Roxanne[] = {
    MON(GEODUDE,  12, ADAMANT, STURDY,       ORAN_BERRY,  15, EV_EARLY, SELF_DESTRUCT, ROCK_TOMB, MAGNITUDE, DEFENSE_CURL),
    MON(ARON,     12, IMPISH,  STURDY,       ORAN_BERRY,  15, EV_EARLY, METAL_CLAW, HEADBUTT, ROCK_TOMB, PROTECT),
    MON(NOSEPASS, 15, BOLD,    STURDY,       SITRUS_BERRY, 20, EV_EARLY, ROCK_TOMB, THUNDER_WAVE, BLOCK, PROTECT),
};

// Brawly: Fake Out pressure and Bulk Up behind Detect.
static const struct CtrStrategyMon sStrat_Brawly[] = {
    MON(MAKUHITA, 17, ADAMANT, THICK_FAT,    SITRUS_BERRY, 18, EV_EARLY, FAKE_OUT, ARM_THRUST, BULK_UP, VITAL_THROW),
    MON(MEDITITE, 16, JOLLY,   PURE_POWER,   ORAN_BERRY,  18, EV_EARLY, BRICK_BREAK, CONFUSION, DETECT, MEDITATE),
    MON(MACHOP,   19, ADAMANT, GUTS,         BLACK_BELT,  20, EV_EARLY, KARATE_CHOP, ROCK_TOMB, SEISMIC_TOSS, FOCUS_ENERGY),
};

// Wattson: Plus and Minus with Helping Hand, paralysis support.
static const struct CtrStrategyMon sStrat_Wattson[] = {
    MON(PLUSLE,    21, MODEST, PLUS,         SITRUS_BERRY, 20, EV_EARLY, SPARK, HELPING_HAND, THUNDER_WAVE, QUICK_ATTACK),
    MON(MINUN,     21, TIMID,  MINUS,        SITRUS_BERRY, 20, EV_EARLY, SPARK, HELPING_HAND, ENCORE, QUICK_ATTACK),
    MON(MAGNETON,  22, MODEST, MAGNET_PULL,  MAGNET,       20, EV_EARLY, SHOCK_WAVE, SONIC_BOOM, THUNDER_WAVE, SUPERSONIC),
    MON(MANECTRIC, 24, TIMID,  STATIC,       SITRUS_BERRY, 22, EV_EARLY, SHOCK_WAVE, BITE, QUICK_ATTACK, ROAR),
};

// Flannery: Sunny Day for Fire moves and a Chlorophyll Solar Beam.
static const struct CtrStrategyMon sStrat_Flannery[] = {
    MON(TORKOAL,  29, QUIET,   WHITE_SMOKE,  CHARCOAL,     24, EV_SPEC_BULK, SUNNY_DAY, OVERHEAT, BODY_SLAM, PROTECT),
    MON(GLOOM,    26, MODEST,  CHLOROPHYLL,  MIRACLE_SEED, 22, EV_SPEC,      SOLAR_BEAM, SLEEP_POWDER, ACID, SUNNY_DAY),
    MON(SLUGMA,   26, CALM,    MAGMA_ARMOR,  SITRUS_BERRY, 22, EV_WALL,      LIGHT_SCREEN, YAWN, FLAMETHROWER, ROCK_SLIDE),
    MON(CAMERUPT, 27, MODEST,  MAGMA_ARMOR,  SITRUS_BERRY, 24, EV_SPEC_BULK, FLAMETHROWER, MAGNITUDE, ROCK_SLIDE, PROTECT),
};

// Norman: Clefable's Follow Me covers Slaking's Truant turns.
static const struct CtrStrategyMon sStrat_Norman[] = {
    MON(CLEFABLE, 29, BOLD,    CUTE_CHARM,   SITRUS_BERRY, 24, EV_WALL,      FOLLOW_ME, ENCORE, MOONLIGHT, BODY_SLAM),
    MON(VIGOROTH, 27, JOLLY,   VITAL_SPIRIT, QUICK_CLAW,   24, EV_PHYS,      SLASH, ENCORE, BULK_UP, FAINT_ATTACK),
    MON(LINOONE,  29, ADAMANT, PICKUP,       SALAC_BERRY,  24, EV_PHYS,      BELLY_DRUM, HEADBUTT, PROTECT, SHADOW_BALL),
    MON(SLAKING,  31, ADAMANT, TRUANT,       SILK_SCARF,   26, EV_PHYS_BULK, FACADE, SHADOW_BALL, FAINT_ATTACK, YAWN),
};

// Winona: Xatu's dual screens, then Dragon Dance and Guts sweepers.
static const struct CtrStrategyMon sStrat_Winona[] = {
    MON(XATU,     30, CALM,    EARLY_BIRD,   SITRUS_BERRY, 24, EV_SUPPORT, REFLECT, LIGHT_SCREEN, CONFUSE_RAY, NIGHT_SHADE),
    MON(PELIPPER, 30, BOLD,    KEEN_EYE,     SITRUS_BERRY, 24, EV_WALL,      PROTECT, SURF, WING_ATTACK, SUPERSONIC),
    MON(SKARMORY, 31, IMPISH,  KEEN_EYE,     LEFTOVERS,    24, EV_WALL,      SPIKES, STEEL_WING, AIR_CUTTER, PROTECT),
    MON(SWELLOW,  31, JOLLY,   GUTS,         SHARP_BEAK,   24, EV_PHYS,      AERIAL_ACE, QUICK_ATTACK, ENDEAVOR, PROTECT),
    MON(ALTARIA,  33, ADAMANT, NATURAL_CURE, LUM_BERRY,    26, EV_PHYS,      DRAGON_DANCE, DRAGON_BREATH, AERIAL_ACE, REFRESH),
};

// Tate & Liza: Earthquake beside Levitate and Flying partners.
static const struct CtrStrategyMon sStrat_TateAndLiza[] = {
    MON(CLAYDOL,  41, RELAXED, LEVITATE,     LEFTOVERS,    28, EV_WALL,      EARTHQUAKE, PSYCHIC, REFLECT, LIGHT_SCREEN),
    MON(XATU,     41, TIMID,   EARLY_BIRD,   LUM_BERRY,    28, EV_SPEC,      PSYCHIC, CONFUSE_RAY, PROTECT, NIGHT_SHADE),
    MON(LUNATONE, 42, MODEST,  LEVITATE,     SITRUS_BERRY, 28, EV_SPEC,      CALM_MIND, PSYCHIC, ICE_BEAM, HYPNOSIS),
    MON(SOLROCK,  42, ADAMANT, LEVITATE,     SITRUS_BERRY, 28, EV_PHYS,      ROCK_SLIDE, EARTHQUAKE, COSMIC_POWER, EXPLOSION),
};

// Juan: Rain Dance and Swift Swim.
static const struct CtrStrategyMon sStrat_Juan[] = {
    MON(WHISCASH,  41, RELAXED, OBLIVIOUS,   LEFTOVERS,    28, EV_WALL,      RAIN_DANCE, SURF, AMNESIA, PROTECT),
    MON(LUDICOLO,  43, MODEST,  SWIFT_SWIM,  SITRUS_BERRY, 28, EV_SPEC,      FAKE_OUT, SURF, GIGA_DRAIN, ICE_BEAM),
    MON(KABUTOPS,  41, ADAMANT, SWIFT_SWIM,  LUM_BERRY,    28, EV_PHYS,      SWORDS_DANCE, SURF, ROCK_SLIDE, BRICK_BREAK),
    MON(CRAWDAUNT, 43, ADAMANT, HYPER_CUTTER, SITRUS_BERRY, 28, EV_PHYS,     SWORDS_DANCE, CRABHAMMER, CRUNCH, PROTECT),
    MON(KINGDRA,   46, MODEST,  SWIFT_SWIM,  MYSTIC_WATER, 30, EV_SPEC,      RAIN_DANCE, SURF, ICE_BEAM, DRAGON_BREATH),
};

/* ---- Elite Four and Champion ---- */

// Sidney: Tyranitar's Sand Stream and Fake Out disruption.
static const struct CtrStrategyMon sStrat_Sidney[] = {
    MON(MIGHTYENA, 46, JOLLY,   INTIMIDATE,  SITRUS_BERRY, 31, EV_PHYS,      CRUNCH, TAUNT, PROTECT, SHADOW_BALL),
    MON(SHIFTRY,   48, ADAMANT, EARLY_BIRD,  LUM_BERRY,    31, EV_PHYS,      FAKE_OUT, EXPLOSION, FAINT_ATTACK, SWORDS_DANCE),
    MON(CACTURNE,  46, ADAMANT, SAND_VEIL,   BRIGHT_POWDER, 31, EV_PHYS,     NEEDLE_ARM, FAINT_ATTACK, LEECH_SEED, SPIKES),
    MON(ABSOL,     49, ADAMANT, PRESSURE,    SCOPE_LENS,   31, EV_PHYS,      SWORDS_DANCE, SLASH, SHADOW_BALL, AERIAL_ACE),
    MON(TYRANITAR, 49, ADAMANT, SAND_STREAM, LUM_BERRY,    31, EV_PHYS,      DRAGON_DANCE, ROCK_SLIDE, CRUNCH, EARTHQUAKE),
};

// Phoebe: Mean Look and Perish Song, Will-O-Wisp and Destiny Bond.
static const struct CtrStrategyMon sStrat_Phoebe[] = {
    MON(DUSCLOPS,   51, IMPISH, PRESSURE,    LEFTOVERS,    31, EV_WALL,      MEAN_LOOK, WILL_O_WISP, CURSE, PROTECT),
    MON(MISDREAVUS, 48, TIMID,  LEVITATE,    SITRUS_BERRY, 31, EV_SPEC,      PERISH_SONG, PROTECT, SHADOW_BALL, CONFUSE_RAY),
    MON(BANETTE,    49, ADAMANT, INSOMNIA,   LUM_BERRY,    31, EV_PHYS,      WILL_O_WISP, SHADOW_BALL, KNOCK_OFF, DESTINY_BOND),
    MON(SABLEYE,    50, CAREFUL, KEEN_EYE,   LEFTOVERS,    31, EV_WALL,      FAKE_OUT, KNOCK_OFF, NIGHT_SHADE, CONFUSE_RAY),
    MON(GENGAR,     50, TIMID,  LEVITATE,    SPELL_TAG,    31, EV_SPEC,      SHADOW_BALL, THUNDERBOLT, HYPNOSIS, DESTINY_BOND),
};

// Glacia: Hail, Blizzard over both sides, a Perish Song finisher.
static const struct CtrStrategyMon sStrat_Glacia[] = {
    MON(GLALIE,   50, JOLLY,   INNER_FOCUS,  QUICK_CLAW,   31, EV_PHYS,      HAIL, ICE_BEAM, CRUNCH, EXPLOSION),
    MON(DEWGONG,  50, CALM,    THICK_FAT,    LEFTOVERS,    31, EV_WALL,      ICE_BEAM, SURF, REST, SLEEP_TALK),
    MON(CLOYSTER, 52, ADAMANT, SHELL_ARMOR,  WHITE_HERB,   31, EV_PHYS,      SPIKES, SURF, ICE_BEAM, EXPLOSION),
    MON(LAPRAS,   52, MODEST,  WATER_ABSORB, SITRUS_BERRY, 31, EV_SPEC_BULK, PERISH_SONG, SURF, ICE_BEAM, THUNDERBOLT),
    MON(WALREIN,  53, MODEST,  THICK_FAT,    NEVER_MELT_ICE, 31, EV_SPEC_BULK, BLIZZARD, SURF, BODY_SLAM, PROTECT),
};

// Drake: Dragon Dance sweepers, Earthquake over Levitate and Flying.
static const struct CtrStrategyMon sStrat_Drake[] = {
    MON(SHELGON,   52, IMPISH,  ROCK_HEAD,   LEFTOVERS,    31, EV_WALL,      DRAGON_CLAW, PROTECT, DOUBLE_EDGE, CRUNCH),
    MON(ALTARIA,   54, ADAMANT, NATURAL_CURE, LUM_BERRY,   31, EV_PHYS,      DRAGON_DANCE, DRAGON_CLAW, EARTHQUAKE, AERIAL_ACE),
    MON(KINGDRA,   53, MODEST,  SWIFT_SWIM,  DRAGON_FANG,  31, EV_SPEC,      RAIN_DANCE, SURF, ICE_BEAM, DRAGON_BREATH),
    MON(FLYGON,    53, JOLLY,   LEVITATE,    SOFT_SAND,    31, EV_PHYS,      EARTHQUAKE, DRAGON_CLAW, ROCK_SLIDE, PROTECT),
    MON(SALAMENCE, 55, ADAMANT, INTIMIDATE,  SALAC_BERRY,  31, EV_PHYS,      DRAGON_DANCE, DRAGON_CLAW, AERIAL_ACE, ROCK_SLIDE),
};

// Wallace: rain, Swift Swim, Water Spout and Milotic's Recover.
static const struct CtrStrategyMon sStrat_Wallace[] = {
    MON(WAILORD,    57, MODEST,  WATER_VEIL,  SITRUS_BERRY, 31, EV_SPEC_BULK, WATER_SPOUT, ICE_BEAM, REST, PROTECT),
    MON(TENTACRUEL, 55, CALM,    CLEAR_BODY,  LEFTOVERS,    31, EV_WALL,      RAIN_DANCE, SURF, SLUDGE_BOMB, PROTECT),
    MON(LUDICOLO,   56, MODEST,  SWIFT_SWIM,  SITRUS_BERRY, 31, EV_SPEC,      FAKE_OUT, SURF, GIGA_DRAIN, ICE_BEAM),
    MON(WHISCASH,   56, ADAMANT, OBLIVIOUS,   LUM_BERRY,    31, EV_PHYS_BULK, RAIN_DANCE, EARTHQUAKE, SURF, AMNESIA),
    MON(GYARADOS,   56, ADAMANT, INTIMIDATE,  LUM_BERRY,    31, EV_PHYS,      DRAGON_DANCE, EARTHQUAKE, DOUBLE_EDGE, HIDDEN_POWER),
    MON(MILOTIC,    58, BOLD,    MARVEL_SCALE, LEFTOVERS,   31, EV_WALL,      RECOVER, SURF, ICE_BEAM, MIRROR_COAT),
};

/* ---- Team Magma and Team Aqua ---- */

// Maxie (Mt. Chimney): a Levitate Explosion lead beside Camerupt's Magnitude.
static const struct CtrStrategyMon sStrat_MaxieMtChimney[] = {
    MON(WEEZING,   24, BOLD,    LEVITATE,    SITRUS_BERRY, 24, EV_EARLY, EXPLOSION, SLUDGE, WILL_O_WISP, SMOKESCREEN),
    MON(CAMERUPT,  25, QUIET,   MAGMA_ARMOR, CHARCOAL,     24, EV_EARLY, MAGNITUDE, FLAMETHROWER, ROCK_SLIDE, PROTECT),
    MON(MIGHTYENA, 24, JOLLY,   INTIMIDATE,  ORAN_BERRY,   24, EV_EARLY, BITE, SWAGGER, PROTECT, TAKE_DOWN),
    MON(GOLBAT,    24, JOLLY,   INNER_FOCUS, ORAN_BERRY,   24, EV_EARLY, WING_ATTACK, CONFUSE_RAY, BITE, HAZE),
};

// Maxie (Magma Hideout): the same plan with Earthquake.
static const struct CtrStrategyMon sStrat_MaxieMagmaHideout[] = {
    MON(WEEZING,   37, BOLD,    LEVITATE,    SITRUS_BERRY, 28, EV_WALL, EXPLOSION, SLUDGE_BOMB, WILL_O_WISP, FIRE_BLAST),
    MON(MIGHTYENA, 37, JOLLY,   INTIMIDATE,  LUM_BERRY,    28, EV_PHYS, CRUNCH, TAUNT, PROTECT, SHADOW_BALL),
    MON(CROBAT,    38, JOLLY,   INNER_FOCUS, SHARP_BEAK,   28, EV_PHYS, AERIAL_ACE, CONFUSE_RAY, BITE, HAZE),
    MON(CAMERUPT,  39, QUIET,   MAGMA_ARMOR, CHARCOAL,     30, EV_SPEC_BULK, EARTHQUAKE, FIRE_BLAST, ROCK_SLIDE, PROTECT),
};

// Archie: Pelipper's rain for Swift Swim Omastar and a Rough Skin Sharpedo.
static const struct CtrStrategyMon sStrat_Archie[] = {
    MON(PELIPPER,  41, BOLD,    KEEN_EYE,    LEFTOVERS,    28, EV_WALL, RAIN_DANCE, SURF, PROTECT, WING_ATTACK),
    MON(MIGHTYENA, 41, JOLLY,   INTIMIDATE,  SITRUS_BERRY, 28, EV_PHYS, CRUNCH, TAUNT, PROTECT, ROAR),
    MON(OMASTAR,   41, MODEST,  SWIFT_SWIM,  MYSTIC_WATER, 28, EV_SPEC, SURF, ICE_BEAM, ANCIENT_POWER, PROTECT),
    MON(CROBAT,    41, JOLLY,   INNER_FOCUS, SHARP_BEAK,   28, EV_PHYS, AERIAL_ACE, CONFUSE_RAY, BITE, HAZE),
    MON(SHARPEDO,  43, ADAMANT, ROUGH_SKIN,  SCOPE_LENS,   30, EV_PHYS, SURF, CRUNCH, AGILITY, ICE_BEAM),
};

/* ---- Wally and some Cooltrainers ---- */

// Wally (Victory Road): Fake Out and Helping Hand for a Calm Mind Gardevoir.
static const struct CtrStrategyMon sStrat_WallyVR[] = {
    MON(ALTARIA,   44, ADAMANT, NATURAL_CURE, LUM_BERRY,   31, EV_PHYS,  DRAGON_DANCE, DRAGON_CLAW, EARTHQUAKE, AERIAL_ACE),
    MON(DELCATTY,  43, JOLLY,   CUTE_CHARM,  SITRUS_BERRY, 31, EV_SUPPORT, FAKE_OUT, HELPING_HAND, DOUBLE_EDGE, ATTRACT),
    MON(ROSELIA,   44, MODEST,  NATURAL_CURE, MIRACLE_SEED, 31, EV_SPEC, GIGA_DRAIN, STUN_SPORE, LEECH_SEED, MAGICAL_LEAF),
    MON(MAGNETON,  41, MODEST,  MAGNET_PULL, MAGNET,       31, EV_SPEC,  THUNDERBOLT, THUNDER_WAVE, LIGHT_SCREEN, PROTECT),
    MON(GARDEVOIR, 45, MODEST,  TRACE,       LEFTOVERS,    31, EV_SPEC,  CALM_MIND, PSYCHIC, THUNDERBOLT, PROTECT),
};

// Shannon: Shadow Tag keeps both foes in for Perish Song.
static const struct CtrStrategyMon sStrat_Shannon[] = {
    MON(WOBBUFFET,  45, BOLD,  SHADOW_TAG,   SITRUS_BERRY, 31, EV_WALL, COUNTER, MIRROR_COAT, ENCORE, DESTINY_BOND),
    MON(MISDREAVUS, 45, TIMID, LEVITATE,     LEFTOVERS,    31, EV_SPEC, PERISH_SONG, PROTECT, SHADOW_BALL, MEAN_LOOK),
};

// Katelynn: Follow Me draws the hits while Slaking loafs.
static const struct CtrStrategyMon sStrat_Katelynn[] = {
    MON(CLEFABLE, 43, BOLD,    CUTE_CHARM, LEFTOVERS,  31, EV_WALL, FOLLOW_ME, HELPING_HAND, MOONLIGHT, ENCORE),
    MON(SLAKING,  43, ADAMANT, TRUANT,     SILK_SCARF, 31, EV_PHYS, RETURN, SHADOW_BALL, EARTHQUAKE, ROCK_SLIDE),
};

// Felix: Earthquake beside a Levitate partner.
static const struct CtrStrategyMon sStrat_Felix[] = {
    MON(CLAYDOL,  43, RELAXED, LEVITATE,   LEFTOVERS,  31, EV_WALL, EARTHQUAKE, PSYCHIC, REFLECT, LIGHT_SCREEN),
    MON(MEDICHAM, 43, ADAMANT, PURE_POWER, BLACK_BELT, 31, EV_PHYS, HI_JUMP_KICK, ROCK_SLIDE, BULK_UP, DETECT),
};

// Julie: Sunny Day and Chlorophyll Solar Beams.
static const struct CtrStrategyMon sStrat_Julie[] = {
    MON(NINETALES, 42, TIMID,  FLASH_FIRE,  SITRUS_BERRY, 31, EV_SPEC, SUNNY_DAY, FLAMETHROWER, WILL_O_WISP, PROTECT),
    MON(TROPIUS,   42, MODEST, CHLOROPHYLL, MIRACLE_SEED, 31, EV_SPEC, SOLAR_BEAM, SYNTHESIS, AERIAL_ACE, PROTECT),
    MON(EXEGGUTOR, 42, MODEST, CHLOROPHYLL, LUM_BERRY,    31, EV_SPEC, SOLAR_BEAM, PSYCHIC, SLEEP_POWDER, EXPLOSION),
};

// Edgar: Sandstorm with Sand Veil on both sides.
static const struct CtrStrategyMon sStrat_Edgar[] = {
    MON(SANDSLASH, 43, ADAMANT, SAND_VEIL, BRIGHT_POWDER, 31, EV_PHYS, SANDSTORM, EARTHQUAKE, ROCK_SLIDE, SLASH),
    MON(CACTURNE,  43, ADAMANT, SAND_VEIL, QUICK_CLAW,    31, EV_PHYS, NEEDLE_ARM, FAINT_ATTACK, LEECH_SEED, SPIKES),
};

#define TEAM(trainer, mons) {TRAINER_##trainer, ARRAY_COUNT(mons), mons}

static const struct CtrStrategyTeam sStrategyTeams[] = {
    TEAM(ROXANNE_1, sStrat_Roxanne),
    TEAM(BRAWLY_1, sStrat_Brawly),
    TEAM(WATTSON_1, sStrat_Wattson),
    TEAM(FLANNERY_1, sStrat_Flannery),
    TEAM(NORMAN_1, sStrat_Norman),
    TEAM(WINONA_1, sStrat_Winona),
    TEAM(TATE_AND_LIZA_1, sStrat_TateAndLiza),
    TEAM(JUAN_1, sStrat_Juan),
    TEAM(SIDNEY, sStrat_Sidney),
    TEAM(PHOEBE, sStrat_Phoebe),
    TEAM(GLACIA, sStrat_Glacia),
    TEAM(DRAKE, sStrat_Drake),
    TEAM(WALLACE, sStrat_Wallace),
    TEAM(MAXIE_MT_CHIMNEY, sStrat_MaxieMtChimney),
    TEAM(MAXIE_MAGMA_HIDEOUT, sStrat_MaxieMagmaHideout),
    TEAM(ARCHIE, sStrat_Archie),
    TEAM(WALLY_VR_1, sStrat_WallyVR),
    TEAM(SHANNON, sStrat_Shannon),
    TEAM(KATELYNN, sStrat_Katelynn),
    TEAM(FELIX, sStrat_Felix),
    TEAM(JULIE, sStrat_Julie),
    TEAM(EDGAR, sStrat_Edgar),
};

#undef TEAM
#undef MON
