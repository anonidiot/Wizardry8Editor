/*
 * Copyright (C) 2022-2024 Anonymous Idiot
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef STRINGLIST_H__
#define STRINGLIST_H__

#include <QList>
#include <QString>

class Localisation;

class StringList
{
public:
    friend class Localisation;

    // This enum list is NOT strongly typed as a class because we
    // always end up wanting the int value in order to call
    // getString() with, and having to static cast every single
    // usage is a pain. Many of these string offsets here represent
    // the start of a list of strings that can be used with existing
    // enum types we have elsewhere, so getString() itself doesn't
    // want to restrict input arguments to onlying being values in
    // this list.
    // Additionally any base string with an ID over 5000 was created
    // for this application and doesn't have a string in the Wizardry 8
    // datafiles.
    // And any value can have 0x20000 (APPEND_COLON) added to it to
    // (surprise) add a colon to the returned string

    static const int APPEND_COLON =  0x20000;
    enum id
    {
        ///// Ids over 5000 are created by us
        NewSaveFile            =  5001,
        OpenExistingSave       =  5002,
        HideThisMessage        =  5003,

        Copy                   =  5010,
        Paste                  =  5011,
        IgnoreModStringsShort  =  5012,

        DroppedItems           =  5020,
        Levels                 =  5021,

        WarningNewFile         =  5050,
        WarningSaveReset       =  5051,
        Scenario               =  5052,

        Scenarios              =  5053, // [4] MainWindow::wiz7_end (-1)
        W7EndUmpani            =  Scenarios,
        W7EndTRang             =  5054,
        W7EndOwnShip           =  5055,
        W8Virgin               =  5056,

        W7BarloneDead          =  5057,
        W7BarloneDeadHelp      =  5058,
        W7RodanDead            =  5059,
        W7RodanDeadHelp        =  5060,

        Wiz128Detected         =  5070,
        ParallelWorldsEnabled  =  5071,
        SelectParallelWorld    =  5072,
        IgnoreModStrings       =  5073,

        Wiz8Path               =  5080,
        PreferredLang          =  5081,
        PreferredCodepage      =  5082,
        SuppressWarnings       =  5083,

        AddItem                =  5100,
        EditItem               =  5101,
        Identify               =  5102,
        RemoveCurse            =  5103,
        Quantity               =  5104,
        ApplyGold              =  5105,

        BonusSwings            =  5150,
        SwingPenalty           =  5151,

        InspectSpell           =  5200,
        InspectSkill           =  5201,
        InspectProfession      =  5202,

        ImportCharacter        =  5300,
        ExportCharacter        =  5301,
        DropCharacter          =  5302,
        RecruitRPC             =  5303,

        ExpLast                =  5350,
        ExpNext                =  5351,

        Possessed              =  5400,
        PoisonStrength         =  5401,

        FilterByProfession     =  5500,
        FilterByRace           =  5501,
        FilterByGender         =  5502,
        Special                =  5503,
        Stackable              =  5504,

        OpenNavigator          =  5600,
        Position               =  5601,
        Heading                =  5602,
        MapWarning             =  5603,
        PortalEnabled          =  5604,

        PortraitInfo           =  5700,
        PortraitModify         =  5701,
        PortraitReset          =  5702,

        /////

        FullName               =  132,
        NickName               =  133,
        PersonalityAndVoice    =  134,
        Level                  =  137,
        Voice1                 =  141,
        Voice2                 =  142,

        PrimaryAttributes      =  150,

        SecondaryAttributes    =  166,
        SAHitPoints            =  167,

        SAStamina              =  169,

        SAArmorClass           =  173,

        SACarryCapacity        =  175,

        AbilitiesTraits        =  177,
        SkillBonus             =  178,
        ProfessionalSkills     =  179,

        ResistanceModifiers    =  185,

        PreviousPage           =  221,
        UndoChanges            =  222,
        CancelAndExit          =  223,
        Done                   =  224,
        ReplaceCharacter       =  225,

        InstructPersonality    =  233,

        SpellPoints            =  237,
        PrevRace               =  238,
        NextRace               =  239,
        PrevPortrait           =  240,
        NextPortrait           =  241,

        PlayVoiceSample        =  245,

        Type                   =  271,
        Weight                 =  272,

        Each                   =  279,
        SpellLevel             =  280,
        CostPerPowerLevel      =  281,

        Spellbook              =  284,
        SpellUsable            =  285,
        Target                 =  286,
        Range                  =  287,
        Damage                 =  288,
        HPperPL                =  289,
        Duration               =  290,
        Rounds                 =  291,
        Round                  =  292,
        Minutes                =  293,
        Minute                 =  294,
        GameHours              =  295,
        GameHour               =  296,
        Permanent              =  297,
        PerPL                  =  298,

        OtherSkills            =  335,

        ControllingAttribs     =  342,
        BestInParty            =  343,
        SkillClosed            =  344,
        SkillPrimary           =  345,

        LISTPrimaryAttributes  =  628, // [ 7]        character::attribute

        LISTRaces              =  644, // [16]        character::race       (bitfield enum indexed)

        LISTProfessions        =  676, // [15]        character::profession (bitfield enum indexed)
        DESCProfessions        =  691, // [15]        character::profession (bitfield enum indexed)

        LISTGenders            =  721, // [ 2]        character::gender     (bitfield enum indexed)

        LISTSkills             =  738, // [41]        character::skill

        Anytime                =  795,
        NonCombat              =  796,
        Combat                 =  797,
        LISTTargets            =  798, // [10]        spell::target
        ProfBonusStaminaReg    =  808,
        ProfBonusHealthReg     =  809,
        ProfBonusCheatDeath    =  810,
        ProfBonusFearless      =  811,
        ProfBonusRangedCrit    =  812,
        ProfBonusThrownCrit    =  813,
        ProfBonusDmgResist     =  814,
        ProfBonusFightBlind    =  815,
        ProfBonusMakeGadget    =  816,
        ProfBonusBackstab      =  817,
        ProfBonusPartyCamp     =  818,
        ProfBonusPrayMiracle   =  819,
        ProfBonusAutoSearch    =  820,
        ProfBonusRemoveCurse   =  821,
        ProfBonusMentalImm     =  822,
        ProfBonusMgcResist     =  823,
        ProfBonusLightning     =  824,
        ProfBonusDispelUnd     =  825,
        ProfBonusMakePotions   =  826,
        ProfBonusKO            =  827,
        ProfBonusBerserk       =  828,
        ProfBonusThrownPen     =  829,
        RaceBonusAC            =  830,
        RaceBonusEquipment     =  831,
        RaceBonusReducedCC     =  832,
        RaceBonusWeightLimit   =  833,
        RaceBonusFastMgcRec    =  834,
        RaceBonusSlowMgcRec    =  835,
        RaceBonusAcidBreath    =  836,
        RaceBonusDmgResist     =  837,
        RaceBonusAndroid       =  838,
        RaceBonusNoMagic       =  839,

        Drained                =  844, // character::condition (sort of - paralyzed and unconscious swapped)
        Diseased               =  848,
        Irritated              =  852,
        Nauseated              =  856,
        Slowed                 =  860,
        Afraid                 =  864,
        Poisoned               =  868,
        Silenced               =  872,
        Hexed                  =  876,
        Enthralled             =  880,
        Insane                 =  884,
        Blinded                =  888,
        Turncoat               =  892,
        Webbed                 =  896,
        Asleep                 =  900,
        Unconscious            =  904,
        Paralyzed              =  908,
        Dead                   =  912,
        Missing                =  916,

        LISTPersonalities      =  941, // [ 9]        character::personality

        Novice                 =  951,
        Journeyman             =  952,
        Warrior                =  953,
        Marauder               =  954,
        Gladiator              =  955,
        Swordsman              =  956,
        Warlord                =  957,
        Conqueror              =  958,
        Squire                 =  959,
        Gallant                =  960,
        Knight                 =  961,
        Chevalier              =  962,
        Paladin                =  963,
        Crusader               =  964,
        Monarch                =  965,
        Lancer                 =  966,
        Cavalier               =  967,
        Champion               =  968,
        Heroine                =  969,
        Olympian               =  970,
        Woodsman               =  971,
        Scout                  =  972,
        Archer                 =  973,
        Pathfinder             =  974,
        Weaponeer              =  975,
        Outrider               =  976,
        RangerLord             =  977,
        Bladesman              =  978,
        Shugenja               =  979,
        Hatamoto               =  980,
        DaishoMaster           =  981,
        Daimyo                 =  982,
        Shogun                 =  983,
        Genin                  =  984,
        Executioner            =  985,
        Assassin               =  986,
        Chunin                 =  987,
        Master                 =  988,
        Jonin                  =  989,
        Grandfather            =  990,
        Seeker                 =  991,
        Disciple               =  992,
        Apostle                =  993,
        Immaculate             =  994,
        Grandmaster            =  995,
        Thief                  =  996,
        Trickster              =  997,
        Highwayman             =  998,
        Bushwhacker            =  999,
        Pirate                 = 1000,
        MasterOfShadows        = 1001,
        Guildmaster            = 1002,
        Tinker                 = 1003,
        Machinist              = 1004,
        Craftsman              = 1005,
        Toolmaster             = 1006,
        Inventor               = 1007,
        Creator                = 1008,
        Genius                 = 1009,
        Minstrel               = 1010,
        Cantor                 = 1011,
        Sonneteer              = 1012,
        Troubadour             = 1013,
        Poet                   = 1014,
        MasterOfLutes          = 1015,
        Muse                   = 1016,
        Acolyte                = 1017,
        Healer                 = 1018,
        Curate                 = 1019,
        Priest                 = 1020,
        HighPriest             = 1021,
        Patriarch              = 1022,
        Saint                  = 1023,
        Herbalist              = 1024,
        Physician              = 1025,
        Adept                  = 1026,
        Shaman                 = 1027,
        Evocator               = 1028,
        MasterOfElixirs        = 1029,
        Enchanter              = 1030,
        Friar                  = 1031,
        Vicar                  = 1032,
        Canon                  = 1033,
        Magistrate             = 1034,
        Diocesan               = 1035,
        Cardinal               = 1036,
        Pontiff                = 1037,
        Psychic                = 1038,
        Soothsayer             = 1039,
        Visionist              = 1040,
        Illusionist            = 1041,
        Mystic                 = 1042,
        Oracle                 = 1043,
        Prophet                = 1044,
        Magician               = 1045,
        Conjurer               = 1046,
        Warlock                = 1047,
        Sorcerer               = 1048,
        Necromancer            = 1049,
        Wizard                 = 1050,
        Magus                  = 1051,

        ACModRace              = 1052,
        ACModSpeed             = 1053,
        ACModStealth           = 1054,
        ACModShield            = 1055,
        ACModMagicItems        = 1056,
        ACModMagicSpells       = 1057,
        ACModVPenetration      = 1058,
        ACModEncumbrance       = 1059,
        ACModConditions        = 1060,
        ACModFatigue           = 1061,
        ACModDefensiveAct      = 1062,
        ACModReflextion        = 1063,

        Head                   = 1075,
        MiscItem1              = 1076,
        MiscItem2              = 1077,
        Cloak                  = 1078,
        Torso                  = 1079,
        Hands                  = 1080,
        PrimaryWeapon          = 1081,
        SecondaryWeapon        = 1082,

        Legs                   = 1085,
        Feet                   = 1086,
        LISTItemTypes          = 1087, // [26]        item::type

        Map                    = 1237,

        Item                   = 1242,

        TwoHandedWeapon        = 1257,

        NumFound               = 1265,
        Charges                = 1266,
        Uses                   = 1267,
        Shots                  = 1268,

        LISTArmorWeights       = 1289, // [ 6]        item::weight

        LISTRanges             = 1307, // [ 4]        item::range
        LISTAttacks            = 1311, // [ 9 step 2] item::attack          (bitfield enum indexed)
        LISTSpecialAttacks     = 1330, // [16]        item::special_attack  (bitfield enum indexed)

        LISTSlays              = 1400, // [33]        item::slays

        DESCSkills             = 1655, // [41]        character::skill
        DESCPrimaryAttributes  =  1696, // [ 7]   character::attribute

        CombatWeaponMods       = 2224,
        Initiative             = 2225,
        Primary                = 2226,
        Secondary              = 2227,
        Punch                  = 2228,
        Kick                   = 2229,
        ToInitiative           = 2230,
        ToHit                  = 2231,
        ToPenetrate            = 2232,
        ToDamage               = 2233,
        NumAttacks             = 2234,
        MaxSwings              = 2235,
        AttackRating           = 2236,
        DamageRange            = 2237,
        ACModifiers            = 2238,
        DamageAbsorption       = 2239,

        LevelsByProf           = 2242,
        AttributeBonus         = 2243,
        AttributePenalty       = 2244,
        SkillBonusDup          = 2245,
        SkillPenalty           = 2246,
        Skill                  = 2247,

        SP                     = 2249,
        RESISTANCES            = 2250,
        Resistances            = 2251,

        SpecialAttack          = 2262,
        AttackModes            = 2263,
        DoubleDamageV          = 2264,
        HPRegeneration         = 2265,
        HPDrain                = 2266,
        StaminaRegeneration    = 2267,
        StaminaDrain           = 2268,
        SPRegeneration         = 2269,
        SPDrain                = 2270,
        AC                     = 2271,

        EquippableSlots        = 2277,

        AttackRange            = 2279,

        ArmorWeightClass       = 2281,
        Spell                  = 2282,
        SpellCasterLevel       = 2283,

        Professions            = 2286,

        Races                  = 2290,
        Sex                    = 2291,
        Requires               = 2292,

        Value                  = 2294,

        MaxItemsPerSlot        = 2296,
        UncursedRemovable      = 2297,
        CursedUnremovable      = 2298,
        NotCursed              = 2299,

        CursedStatus           = 2301,
        BardLevel              = 2302,
        GadgeteerLevel         = 2303,

        Description            = 2319,
        MaleOnly               = 2320,
        FemaleOnly             = 2321,

        SpecialAttributes      = 2323,

        ExperiencePoints       = 2331,
        Earned                 = 2332,
        NextLevel              = 2333,
        HitPoints              = 2334,
        Stamina                = 2335,
        Load                   = 2336,
        ArmorClass             = 2337,
        BaseAC                 = 2338,
        AverageAC              = 2339,

        Carried                = 2347,
        PartyItems             = 2348,

        ExpEarned              = 2355,

        Kills                  = 2357,
        Deaths                 = 2358,
        Attribs                = 2359,

        Change                 = 2362,
        ViewProfStatus         = 2363,
        Items                  = 2364,
        Stats                  = 2365,
        Skills                 = 2366,
        Magic                  = 2367,
        Exit                   = 2368,
        InspectItem            = 2369,
        MergeItems             = 2370,
        SplitItems             = 2371,
        UseItem                = 2372,
        DropItem               = 2373,
        UncurseAllItems        = 2374,

        WeaponsAndShields      = 2378,
        Armor                  = 2379,
        NonEquippable          = 2380,
        Equippable             = 2381,
        Usable                 = 2382,
        Unidentified           = 2383,
        SortByType             = 2384,
        SwapWeapons            = 2385,

        Base                   = 2393,
        Mod                    = 2394,
        BaseMax                = 2395,
        BaseMin                = 2396,
        ShowCharInfo           = 2397,
        ShowEquipmentInfo      = 2398,
        ViewOffenseMods        = 2399,
        ViewDefenseMods        = 2400,
    };

    StringList( QString filename, bool force_base=false );
    ~StringList();

    bool              isNull() const;
    int               getNumStrings() const;
    const QString     getString( int idx ) const;

    static QString    decipher( QByteArray in );

private:
    QList<QString>    m_strings;

    const QString     getUnlocalisedString( int idx ) const;
};

#endif /* STRINGLIST_H__ */
