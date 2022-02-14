/*
 * Copyright (C) 2022 Anonymous Idiot
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

#ifndef CHARACTER_H__
#define CHARACTER_H__

#define SIMPLIFIED_BONUSES  1

#define MAXIMUM_CHARACTER_SPELLS   114

#include <QMetaEnum>
#include <QMap>

class item;

class character : QObject
{
    Q_OBJECT

public:
    enum profession
    {
        Fighter   = 0x0001,
        Lord      = 0x0002,
        Valkyrie  = 0x0004,
        Ranger    = 0x0008,
        Samurai   = 0x0010,
        Ninja     = 0x0020,
        Monk      = 0x0040,
        Rogue     = 0x0080,
        Gadgeteer = 0x0100,
        Bard      = 0x0200,
        Priest    = 0x0400,
        Alchemist = 0x0800,
        Bishop    = 0x1000,
        Psionic   = 0x2000,
        Mage      = 0x4000,
    };
    Q_DECLARE_FLAGS(professions, profession);
    Q_ENUM(profession);

    enum race
    {
        Human     = 0x0001,
        Elf       = 0x0002,
        Dwarf     = 0x0004,
        Gnome     = 0x0008,
        Hobbit    = 0x0010,
        Faerie    = 0x0020,
        Lizardman = 0x0040,
        Dracon    = 0x0080,
        Felpurr   = 0x0100,
        Rawulf    = 0x0200,
        Mook      = 0x0400,

        // Recruitable races only (non-player)
        Trynnie   = 0x0800,
        TRang     = 0x1000, // T'Rang
        Umpani    = 0x2000,
        Rapax     = 0x4000,
        Android   = 0x8000,
    };
    Q_DECLARE_FLAGS(races, race);
    Q_ENUM(race);

    enum gender
    {
        Male     = 0x0001,
        Female   = 0x0002,
    };
    Q_DECLARE_FLAGS(genders, gender);
    Q_ENUM(gender);

    enum personality
    {
        Aggressive   = 0x00,
        Intellectual,
        Burly,
        Chaotic,
        Cunning,
        Eccentric,
        Kindly,
        Laidback,
        Loner,

        PERSONALITY_SIZE
    };
    Q_ENUM(personality);

    enum attribute
    {
        Strength = 0x00,
        Intelligence,
        Piety,
        Vitality,
        Dexterity,
        Speed,
        Senses,

        ATTRIBUTE_SIZE,
        ATTRIBUTE_NONE = -1
    };
    Q_ENUM(attribute);

    enum skill
    {
        Sword   = 0x00,
        Axe,
        Polearm,
        Mace_Flail,
        Dagger,
        Staff_Wand,
        Shield,
        ModernWeapon,
        Bow,
        Throwing_Sling,

        Locks_Traps,
        Stealth,
        Music,
        Pickpocket,
        MartialArts,
        Scouting,

        CloseCombat,
        RangedCombat,
        DualWeapons,
        CriticalStrike,
        Artifacts,
        Mythology,
        Communication,
        Engineering,

        Wizardry,
        Divinity,
        Alchemy,
        Psionics,
        FireMagic,
        WaterMagic,
        AirMagic,
        EarthMagic,
        MentalMagic,
        DivineMagic,

        PowerStrike,
        PowerCast,
        IronWill,
        IronSkin,
        Reflexion,
        SnakeSpeed,
        EagleEye,

        SKILL_SIZE,
        SKILL_NONE = -1
    };
    Q_ENUM(skill);

    enum realm
    {
        Fire,
        Water,
        Air,
        Earth,
        Mental,
        Divine,

        REALM_SIZE
    };

    enum worn
    {
        Backpack1 = 0,
        Backpack2,
        Backpack3,
        Backpack4,
        Backpack5,
        Backpack6,
        Backpack7,
        Backpack8,
        Head,
        Cloak,
        Torso,
        Hand,
        Legs,
        Feet,
        Misc1,
        Misc2,
        Weapon1a,
        Weapon1b,
        Weapon2a,
        Weapon2b,

        WORN_SIZE
    };

    enum condition
    {
        Normal,
        Drained,
        Diseased,
        Irritated,
        Nauseated,
        Slowed,
        Afraid,
        Poisoned,
        Silenced,
        Hexed,
        Enthralled,
        Insane,
        Blind,
        Turncoat,
        Webbed,
        Asleep,
        Paralyzed,
        Unconscious,
        Dead,
        Missing,

        CONDITION_SIZE
    };

    // Used by all the 2 index arrays for attributes
    enum atIdx
    {
        Base,
        Current,
        Initial,

        ATIDX_SIZE
    };

    character();
    character(const character &other);
    character(QByteArray c, QByteArray cx);
    ~character();

    static void   lkupControllingAttribs( character::skill sk, character::attribute *a1, character::attribute *a2);

    QByteArray    serialize() const;
    QByteArray    getCharExtra() const;
    void          resetCharExtra();

    bool          isNull() const;
    bool          isRPC() const;

    void          setColorIdx(int idx);
    int           getColorIdx() const;

    QString       getName() const;
    void          setName(const QString &name);
    QString       getFullName() const;
    void          setFullName(const QString &name);
    profession    getProfession() const;
    QString       getProfessionString();
    void          setProfession(profession p);

    static QString    getProfessionString(profession p);

    int           getCondition(condition c) const;
    void          setCondition(condition c, int duration);
    void          setConditionActive(condition c, bool on);
    int           getPoisonStrength() const;
    void          setPoisonStrength(int str);

    /**
     * Returns the skill which gets the 25% bonus - based on profession.
     * @return skill @enum
     */
    skill         getProfessionalSkill() const;
    static skill  getProfessionalSkill( character::profession prof );

    /**
     * Returns the favoured skills - based on profession
     * @return list of skill @enum
     */
    QList<skill>  getProfessionalSkills() const;
    static QList<skill>  getProfessionalSkills( character::profession prof );

    /**
     * Returns the list of skills available, anything not listed
     * can't be 'trained' or incremented on level up - based on profession but also primary attributes
     * @return list of skill @enum
     */
    QList<skill>  getTrainableSkills() const;
    static QList<skill>  getTrainableSkills( character::profession prof, character::race race = race::Human );

    /**
     * Returns the distinctive character abilities - based on profession and race
     * @return list of benefits as text strings
     */
    QStringList   getAbilityStrings() const;

    bool          isPureCaster() const;
    bool          isHybridCaster() const;
    bool          isAlive() const;
    race          getRace() const;
    QString       getRaceString();
    void          setRace(race r);
    gender        getGender() const;
    QString       getGenderString();
    void          setGender(gender g);
    int           getCurrentLevel() const;
    QString       getCurrentLevelString() const;
    bool          getProfessionLevel( int idx, QString &profession, int &level ) const;
    int           getProfessionLevel( character::profession profession ) const;
    int           getProfessionInitialLevel( character::profession profession ) const;
    void          setProfessionLevel( character::profession profession, int level );
    int           getPortraitIndex() const;
    void          setPortraitIndex(int index);
    personality   getPersonality() const;
    void          setPersonality(personality pers);
    int           getVoice() const;
    void          setVoice(int voice);

    int           getMagicResistance(character::realm r) const;
    int           getInitiative() const;
    int           getDeaths() const;
    int           getKills() const;
    int           getToInit(bool primary) const;
    int           getToPenetrate(bool primary) const;
    bool          getDamage(bool primary, quint16 *min_damage, quint16 *max_damage, int *percentage) const;
    QString       getDamageString(bool primary, bool summarised) const;
    void          getAttackRating(bool primary, int *baseAR, int *modAR) const;
    QString       getAttackRatingString(bool primary, bool summarised) const;
    int           getNumAttacks(bool primary) const;
    int           getMaxSwings(bool primary) const;

    qint32        getAC_Base() const;
    qint32        getAC_Average() const;

    qint32        getACMod_Race() const;
    qint32        getACMod_Speed() const;
    qint32        getACMod_Stealth() const;
    qint32        getACMod_Shield() const;
    qint32        getACMod_MagicItems() const;
    qint32        getACMod_MagicSpells() const;
    qint32        getACMod_Penetration() const;
    qint32        getACMod_Encumbrance() const;
    qint32        getACMod_Conditions() const;
    qint32        getACMod_Fatigue() const;
    qint32        getACMod_Defensive() const;
    qint32        getACMod_Reflextion() const;
    qint32        getDamageAbsorption() const;

    int           getAttribute(attribute at, atIdx idx) const;
    void          setAttribute(attribute at, int value);

    int           getSkill(skill sk, atIdx idx) const;
    void          setSkill(skill sk, int value);

    int           getHp(atIdx idx) const;
    int           getStamina(atIdx idx) const;
    int           getMp(realm r, atIdx idx) const;
    void          setHp(atIdx idx, int hp);
    void          setStamina(atIdx idx, int stamina);
    void          setMp(realm r, atIdx idx, int mp);
    int           getMagicResistance(realm r, atIdx idx);
    quint32       getXp() const;
    quint32       getXpNeeded() const;
    quint32       getXpLastLevelNeeded() const;
    void          setXp(quint32 xp);
    void          setXpNeeded(quint32 xp);
    void          setXpLastLevelNeeded(quint32 xp);

    double        getLoad(atIdx idx) const;
    int           getLoadCategory() const;
    qint32        getPersonalLoad() const;
    qint32        getPartyShare() const;
    void          setPartyShare(qint32 weight);
    item          getItem(worn idx) const;
    void          deleteItem(worn idx);
    void          setItem(worn idx, item i);
    bool          isItemUsable(const item i) const;

    bool          isSpellKnown(int idx) const;
    void          setSpellKnown(int idx, bool known=true);

    void          recomputeLoadCategory();
    void          recomputeEverything();

protected:
    struct attribute_detail
    {
        qint32            value[ atIdx::ATIDX_SIZE ];
        quint32           w[3];
    };

    struct skill_detail
    {
        quint8            enabled;
        quint8            unknown;
        qint32            value[ atIdx::ATIDX_SIZE ];
        quint32           control;  // average of the current values of the controlling attributes for training this skill
        quint32           count;    // 0<8 - governs skill increase
        quint32           w[5];
    };


    void unpackCharacter(const QByteArray &c, const QByteArray &pa);
    void loadItem( character::worn idx, const quint8 *cdata);
    void assignItem( quint8 *cdata, character::worn idx ) const;

    QString     formatUtf16(const quint8 *b, size_t max_len);
    profession  formatProfession(const quint8 *b);
    race        formatRace(const quint8 *b);
    gender      formatGender(const quint8 *b);
    personality formatPersonality(const quint8 *b);
    condition   formatCondition(const quint8 *b);
    skill       formatSkill(const quint8 *b);

    void        formatAttributeDetail(struct attribute_detail *a, const quint8 *b);
    void        assignAttributeDetail(quint8 *b, const struct attribute_detail *a) const;
    void        formatSkillDetail(struct skill_detail *s, const quint8 *b);
    void        assignSkillDetail(quint8 *b, const struct skill_detail *s) const;


private:

    struct attack
    {
        qint32            weapon_type;
        quint8            enabled;
        skill             skill1;
        skill             skill2;
        quint32           skill_total;
        qint32            BAR;
        quint32           num_attacks;
        quint32           max_swings;
        qint32            to_init;
        qint32            MAR;
        qint32            to_penetrate;
        qint32            multiplier;
        quint16           damage_c;
        quint8            damage_num_dice;
        quint8            damage_dice_sides;
        quint16           attack_modes;
        quint32           unknown51;
        quint16           unknown55;
        quint8            unknown57;
        quint8            unknown58;
        quint32           unknown59;
        quint32           unknown63;
    };

    struct bonus
    {
        qint8             to_init;
        qint8             MAR;
        qint8             to_penetrate;
        qint8             multiplier;
        qint8             mspells;
        qint8             vpenetrate;
        qint8             absorption;
        qint8             allmagic_resist;
        qint8             unknown_0x1778;
        qint8             hp_regen;
        qint8             stamina_regen;
        qint8             sp_regen;
        qint8             attributes[ ATTRIBUTE_SIZE ]; // +12
        qint8             skills[ SKILL_SIZE ]; // +19
        qint8             magic_resistance[ REALM_SIZE ]; // +60
        qint8             unknown_0x17b2; // +66
        qint8             unknown_0x17b3;
        qint8             unknown_0x17b4;
        qint8             incapacitated; // +69
        //qint8             m_unknown_0x17b6;
        //qint8             m_unknown_0x17b7;
        //qint8             m_unknown_0x17b8;
        //qint8             m_unknown_0x17b9;
        //qint8             m_unknown_0x17ba;
        qint8             conditions;
        //qint8             m_unknown_0x17bc;
        //qint8             m_unknown_0x17bd;
        //qint8             m_unknown_0x17be;
        //qint8             m_unknown_0x17bf;
        //qint8             m_unknown_0x17c0;
        //qint8             m_unknown_0x17c1;
        //qint8             m_unknown_0x17c2;
        //qint8             m_unknown_0x17c3;
        //qint8             m_unknown_0x17c4;
        //qint8             m_unknown_0x17c5;
        //qint8             m_unknown_0x17c6;
        //qint8             m_unknown_0x17c7;
        //qint8             m_unknown_0x17c8;
        //qint8             m_unknown_0x17c9;
        //qint8             m_unknown_0x17ca;
        //qint8             m_unknown_0x17cb;
        //qint8             m_unknown_0x17cc;
        //qint8             m_unknown_0x17cd;
        //qint8             m_unknown_0x17ce;
        //qint8             m_unknown_0x17cf;
        //qint8             m_unknown_0x17d0;
        //qint8             m_unknown_0x17d1;
        //qint8             m_unknown_0x17d2;
        //qint8             m_unknown_0x17d3;
        //qint8             m_unknown_0x17d4;
        //qint8             m_unknown_0x17d5;
        //qint8             m_unknown_0x17d6;
    };


    void        recomputeHp();
    void        recomputeStamina();
    void        recomputeInitiative();
    void        recomputeAcmod();
    void        recomputeAttack();
    void        resetControllingAttribs();
    void        recomputeKnownSpellsCount();
    void        recomputeManaPoints();
    void        recomputeDamageAbsorption();
    void        recomputeMagicResistance();
    void        recomputeRecoveryRates();
    void        recomputeCarryCapacity();
    void        recomputePersonalLoad();
    void        recomputeBonus();
    void        recomputeConditionPenalty(struct bonus *b);
    void        recomputeItemBonus(struct bonus *b);


    QByteArray        m_data;
    QByteArray        m_charExtra;

    int               m_color_idx;

    quint8            m_in_party;
    QString           m_charName;
    QString           m_fullName;
    profession        m_profession;
    profession        m_origProfession;
    race              m_race;
    gender            m_gender;
    personality       m_personality;
    condition         m_main_status_condition;
    int               m_voice;
    int               m_portraitIndex;
    quint8            m_charslot_inuse;
    qint32            m_rpc_id; // -1 = player

    int                    m_combinedLevel;
    QMap<profession, int>  m_currentLevels;
    QMap<profession, int>  m_initialLevels;

    struct attribute_detail    m_attribs[ attribute::ATTRIBUTE_SIZE ];
    struct skill_detail        m_skills[  skill::SKILL_SIZE ];

    bool              m_conditions_active[  condition::CONDITION_SIZE ];
    quint32           m_conditions[  condition::CONDITION_SIZE ];
    quint32           m_poison_strength;

    qint32            m_spell[MAXIMUM_CHARACTER_SPELLS];

    quint32           m_xp;
    quint32           m_xp_needed;
    quint32           m_xp_last_needed;

    qint32            m_hp[ atIdx::ATIDX_SIZE ];
    qint32            m_hp_drain;
    qint32            m_stamina[ atIdx::ATIDX_SIZE ];
    qint32            m_stamina_drain;

    float             m_healing_rate;
    float             m_rest_rate;

    quint32           m_personal_load;
    quint32           m_party_share;
    quint32           m_carry_capacity;
    quint32           m_encumbrance;
    quint32           m_load_category;
    quint8            m_dual_wielding;

    quint32           m_kills;
    quint32           m_deaths;
    quint32           m_initiative;

    qint32            m_mp[REALM_SIZE][ atIdx::ATIDX_SIZE ];
    quint32           m_mpStrongestRealm;
    quint32           m_knownSpellsCount[REALM_SIZE];
    float             m_mp_recovery[REALM_SIZE][ atIdx::ATIDX_SIZE ];

    qint32            m_acmod_base;
    qint32            m_acmod_race;
    qint32            m_acmod_speed;
    qint32            m_acmod_stealth;
    qint32            m_acmod_shield;
    qint32            m_acmod_mitems;
    qint32            m_acmod_mspells;
    qint32            m_acmod_penetrate;
    qint32            m_acmod_encumbrance;
    qint32            m_acmod_conditions;
    qint32            m_acmod_fatigue;
    qint32            m_acmod_defensive;
    qint32            m_acmod_reflextion;
    qint32            m_acmod_unknown;

    qint32            m_acmod_average;
    QMap<worn, int>   m_ac_bodypart;

    qint32            m_damage_absorption;

    qint32            m_magic_resistance[REALM_SIZE][ atIdx::ATIDX_SIZE ];

    struct attack     m_attack[2];

    qint32            m_secondary_weapon_type;
    quint8            m_secondary_enabled;
    quint32           m_secondary_skill;
    qint32            m_secondary_dunno;
    qint32            m_secondary_BAR;
    quint32           m_secondary_num_attacks;
    quint32           m_secondary_max_swings;
    qint32            m_secondary_to_init;
    qint32            m_secondary_MAR;
    qint32            m_secondary_to_penetrate;
    qint32            m_secondary_multiplier;

    quint16           m_punch_base_min;
    quint16           m_punch_base_max;
    quint16           m_punch_attack_modes;

    quint32           m_fatigue_category;

#ifndef SIMPLIFIED_BONUSES
    struct bonus      m_bonus_penalties;
    struct bonus      m_bonus_items;
#endif
    struct bonus      m_bonus;

    qint8             m_unknown_0x1830;

    item             *m_item[WORN_SIZE];

    double            m_portal_x;
    double            m_portal_y;
    double            m_portal_z;
    double            m_portal_pitch_or_direction_dunno;
    double            m_portal_direction_or_pitch_dunno;
    qint32            m_portal_map;

    QMetaEnum         m_metaProf;
    QMetaEnum         m_metaRace;
    QMetaEnum         m_metaGender;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(character::professions)
Q_DECLARE_OPERATORS_FOR_FLAGS(character::races)
Q_DECLARE_OPERATORS_FOR_FLAGS(character::genders)

#endif
