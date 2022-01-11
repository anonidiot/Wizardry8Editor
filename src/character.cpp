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

#include <QMapIterator>

#include "character.h"
#include "item.h"
#include "spell.h"
#include "main.h"

#include <QDebug>
#include "common.h"

#include "RIFFFile.h"

// We use this with the QMaps to avoid making entries for 0 values on some of the maps,
// eg. the one used for levels by profession - we only include professions the character
// has actually been.
#define CHECK_SET(A, B)   { if (B) { A = B; } }

character::character() : QObject(),
    m_data(QByteArray(RIFFFile::CHARACTER_SIZE, 0)),
    m_charExtra(QByteArray(RIFFFile::CHARACTER_EXTRA_SIZE, 0)),
    m_in_party(false),
    m_charslot_inuse(false)
{
    for (int k=0; k<WORN_SIZE; k++)
    {
        m_item[k] = NULL;
    }
}

// Character specific information is split into 2 separate areas.
// A block of 0x1866 bytes and a block of 0x106 bytes
// We keep an exact copy of the data passed into us because there
// are a lot of fields we _don't_ parse and we'd like to preserve
// those as far as possible on save.
character::character(QByteArray c, QByteArray cx) : QObject(),
    m_data(c),
    m_charExtra(cx),
    m_profession(profession::Fighter),
    m_race(race::Human),
    m_gender(gender::Male),
    m_combinedLevel(-1)
{
    m_metaProf        = QMetaEnum::fromType<character::profession>();
    m_metaRace        = QMetaEnum::fromType<character::race>();
    m_metaGender      = QMetaEnum::fromType<character::gender>();

    unpackCharacter( m_data, m_charExtra );

    // Shouldn't have to do this except someone could have hexedited the save game
    // and left it in an inconsistent state
    recomputeEverything();
}

character::character(const character &other) : QObject(),
    m_profession(profession::Fighter),
    m_race(race::Human),
    m_gender(gender::Male),
    m_combinedLevel(-1)
{
    m_data            = other.serialize();
    m_charExtra       = other.getCharExtra();

    m_metaProf        = QMetaEnum::fromType<character::profession>();
    m_metaRace        = QMetaEnum::fromType<character::race>();
    m_metaGender      = QMetaEnum::fromType<character::gender>();

    unpackCharacter( m_data, m_charExtra );
    recomputeEverything();
}

character::~character()
{
    for (int k=0; k<WORN_SIZE; k++)
    {
        if (m_item[k])
            delete m_item[k];
    }
}

bool character::isNull() const
{
    return ! m_in_party;
}

void character::loadItem( worn idx, const quint8 *cdata)
{
    quint32   id      = FORMAT_LE32(cdata);
    quint8    cnt     = FORMAT_8(cdata+4);
    quint8    charges = FORMAT_8(cdata+5);

    quint8    identified = FORMAT_8(cdata+6);  // 0=unidentified, 1=identified, other values?
    quint8    uncursed   = FORMAT_8(cdata+10); // 0=object in default state, 1=magically uncursed, other values?

    // Don't know what these other ones are for, but they're doing something
    /*
    quint8    c   = FORMAT_8(cdata+7);
    quint8    d   = FORMAT_8(cdata+8);
    quint8    e   = FORMAT_8(cdata+9);
    quint8    g   = FORMAT_8(cdata+11);
    */

    if (id == 0xffffffff)
        m_item[idx] = NULL;
    else
    {
        bool isEquipped = true;

        if (idx <= worn::Backpack8)
            isEquipped = false;

        m_item[idx] = new item( id, cnt, charges, identified, uncursed, isEquipped );
    }
}

void character::assignItem( quint8 *cdata, worn idx) const
{
    if (m_item[idx])
    {
        ASSIGN_LE32( cdata,    m_item[idx]->getId()        );
        ASSIGN_LE8(  cdata+ 4, m_item[idx]->getCount()     );
        ASSIGN_LE8(  cdata+ 5, m_item[idx]->getCharges()   );
        ASSIGN_LE8(  cdata+ 6, m_item[idx]->isIdentified() );
        ASSIGN_LE8(  cdata+ 7, 0                           );
        ASSIGN_LE8(  cdata+ 8, 0                           );
        ASSIGN_LE8(  cdata+ 9, 0                           );
        ASSIGN_LE8(  cdata+10, m_item[idx]->isUncursed()   );
        ASSIGN_LE8(  cdata+11, 0                           );
    }
    else
    {
        ASSIGN_LE32( cdata,    0xffffffff                  );
        ASSIGN_LE8(  cdata+ 4, 0                           );
        ASSIGN_LE8(  cdata+ 5, 0                           );
        ASSIGN_LE8(  cdata+ 6, 0                           );
        ASSIGN_LE8(  cdata+ 7, 0                           );
        ASSIGN_LE8(  cdata+ 8, 0                           );
        ASSIGN_LE8(  cdata+ 9, 0                           );
        ASSIGN_LE8(  cdata+10, 0                           );
        ASSIGN_LE8(  cdata+11, 0                           );
    }
}

QString character::formatUtf16(const quint8 *b, size_t max_len)
{
    quint16   buf[ max_len ];

    // We can't just run QString::fromUtf16() on an arbitrary memory offset.
    // ALL the Qt unicode functions have the requirement for the input to
    // be word aligned for the type in question, ie. UTF-16 has to use
    // 16 bit chars and be aligned on an even address, UCS-4 has to use
    // 32 bit chars and be aligned on a multiple of 4 address.
    // So we have to memory copy in order to guarantee alignment is good.

    for (unsigned int k=0; k<max_len; k++)
    {
        buf[k] = FORMAT_LE16(b + k*2);
    }
    buf[max_len-1] = 0;

    return QString::fromUtf16( buf );
}

character::profession character::formatProfession(const quint8 *b)
{
    int v = m_metaProf.value( FORMAT_LE32( b ) );
    if (v == -1)
    {
        qWarning() << "profession was set to invalid value = " << QString::number(FORMAT_LE32(b), 16);
        return (character::profession) m_metaProf.value(0);
    }
    return (character::profession) v;
}

character::race character::formatRace(const quint8 *b)
{
    int v = m_metaRace.value( FORMAT_LE32( b ) );
    if (v == -1)
    {
        qWarning() << "race was set to invalid value = " << QString::number(FORMAT_LE32(b), 16);
        return (character::race) m_metaRace.value(0);
    }
    return (character::race) v;
}

character::gender character::formatGender(const quint8 *b)
{
    int v = m_metaGender.value( FORMAT_LE32( b ) );
    if (v == -1)
    {
        qWarning() << "gender was set to invalid value = " << QString::number(FORMAT_LE32(b), 16);
        return (character::gender) m_metaGender.value(0);
    }
    return (character::gender) v;
}

character::personality character::formatPersonality(const quint8 *b)
{
    qint32 v = FORMAT_LE32( b );

    if ((v < 0) || (v >= PERSONALITY_SIZE))
    {
        // RPCs are going to legitimately have this set to -1
        // qDebug() << "personality was set to invalid value = " << QString::number(v, 16);
        return (character::personality) 0;
    }
    return (character::personality) v;
}

character::skill character::formatSkill(const quint8 *b)
{
    qint32 v = FORMAT_LE32( b );

    if ((v < -1) || (v >= SKILL_SIZE))
    {
        qWarning() << "skill was set to invalid value = " << QString::number(v, 16);
        return skill::SKILL_NONE;
    }
    return (character::skill) v;
}

character::condition character::formatCondition(const quint8 *b)
{
    qint32 v = FORMAT_LE32( b );

    if ((v < -1) || (v >= CONDITION_SIZE))
    {
        qWarning() << "skill was set to invalid value = " << QString::number(v, 16);
        return condition::Normal;
    }
    return (character::condition) v;
}

void character::formatAttributeDetail(struct attribute_detail *a, const quint8 *b)
{
    // actual character value with no race/profession bonuses or item impacts
    a->value[ atIdx::Base ]     = FORMAT_LE32( b );
    if (a->value[ atIdx::Base ] > 125)
        a->value[ atIdx::Base ] = 125;

    a->value[ atIdx::Initial ] = a->value[ atIdx::Base ];

    // adjusted value with all bonuses applied
    a->value[ atIdx::Current ]     = FORMAT_LE32( b + 4 );
    if (a->value[ atIdx::Current ] > 125)
        a->value[ atIdx::Current ] = 125;

    a->w[0]     = FORMAT_LE32( b + 8 );
    a->w[1]     = FORMAT_LE32( b + 12 );
    a->w[2]     = FORMAT_LE32( b + 16 );
}

void character::assignAttributeDetail(quint8 *b, const struct attribute_detail *a) const
{
    ASSIGN_LE32( b,      a->value[ atIdx::Base ]    );
    ASSIGN_LE32( b +  4, a->value[ atIdx::Current ] );
    ASSIGN_LE32( b +  8, a->w[0] );
    ASSIGN_LE32( b + 12, a->w[1] );
    ASSIGN_LE32( b + 16, a->w[2] );
}

void character::formatSkillDetail(struct skill_detail *s, const quint8 *b)
{
    // true if skill has been unlocked/is available to profession for training
    s->enabled  = FORMAT_8   ( b );
    s->unknown  = FORMAT_8   ( b + 1 );

    // actual character value with no race/profession bonuses or item impacts
    s->value[ atIdx::Base ]    = FORMAT_LE32( b + 2 );
    s->value[ atIdx::Initial ] = s->value[ atIdx::Base ];

    // adjusted value with all bonuses applied
    s->value[ atIdx::Current ] = FORMAT_LE32( b + 6 );

    // average of controlling attributes ALCH = (INT + DEX)/2 (+power cast factor)
    s->control  = FORMAT_LE32( b + 10 );
    // counter value for skill increase (0<8)
    s->count    = FORMAT_LE32( b + 14 );
    s->w[0]     = FORMAT_LE32( b + 18 );
    s->w[1]     = FORMAT_LE32( b + 22 );
    s->w[2]     = FORMAT_LE32( b + 26 );
    s->w[3]     = FORMAT_LE32( b + 30 );
    s->w[4]     = FORMAT_LE32( b + 34 );
}

void character::assignSkillDetail(quint8 *b, const struct skill_detail *s) const
{
    ASSIGN_LE8 ( b,      s->enabled  );
    ASSIGN_LE8 ( b +  1, s->unknown  );
    ASSIGN_LE32( b +  2, s->value[ atIdx::Base ]    );
    ASSIGN_LE32( b +  6, s->value[ atIdx::Current ] );
    ASSIGN_LE32( b + 10, s->control  );
    ASSIGN_LE32( b + 14, s->count    );
    ASSIGN_LE32( b + 18, s->w[0]     );
    ASSIGN_LE32( b + 22, s->w[1]     );
    ASSIGN_LE32( b + 26, s->w[2]     );
    ASSIGN_LE32( b + 30, s->w[3]     );
    ASSIGN_LE32( b + 34, s->w[4]     );
}

QByteArray character::getCharExtra() const
{
    if (!m_in_party)
    {
        return QByteArray( RIFFFile::CHARACTER_EXTRA_SIZE, 0 );
    }
    Q_ASSERT(m_charExtra.size() == RIFFFile::CHARACTER_EXTRA_SIZE);

    QByteArray response = m_charExtra;

    quint8 *cdata = (quint8 *) response.data();

    ASSIGN_LE8(  cdata+0x0000, m_in_party);
    ASSIGN_LE32( cdata+0x00f1, m_color_idx);

    return response;
}

void character::resetCharExtra()
{
    quint8 *cdata = (quint8 *) m_charExtra.data();

    quint8  unknown_d0 = FORMAT_8(    cdata+0x00d0 );
    quint8  unknown_ff = FORMAT_8(    cdata+0x00ff );

    memset( cdata, 0, m_charExtra.size() );

    ASSIGN_LE8( cdata+0x0000, m_in_party);

    if (m_in_party)
    {
        ASSIGN_LE32( cdata+0x00f1, m_color_idx );
        ASSIGN_LE32( cdata+0x00fa, m_rpc_id    );

        // Don't know what these fields even are
        ASSIGN_LE32( cdata+0x0071, 0xffffffff );
        ASSIGN_LE8(  cdata+0x00d0, unknown_d0 );
        ASSIGN_LE8(  cdata+0x00ff, unknown_ff );
    }
}

QByteArray character::serialize() const
{
    if (!m_in_party)
    {
        return QByteArray( RIFFFile::CHARACTER_SIZE, 0 );
    }

    QByteArray response = m_data;

    quint8 *cdata = (quint8 *) response.data();

    /* [0x0004        ] */
    ASSIGN_LE8(  cdata+0x0004, m_in_party);
    /* [0x0005--0x0018] */
    for (int k=0; k < m_charName.size(); k++)
    {
        ASSIGN_LE16( cdata+0x0005 + k*2, m_charName.at(k).unicode());
    }
    ASSIGN_LE16( cdata+0x0005 + m_charName.size()*2, 0);
    /* [0x0019--0x003f] */
    for (int k=0; k < m_fullName.size(); k++)
    {
        ASSIGN_LE16( cdata+0x0019 + k*2, m_fullName.at(k).unicode());
    }
    ASSIGN_LE16( cdata+0x0019 + m_fullName.size()*2, 0);
    /* [0x0069--0x006c] */
    for (int k=0; k<m_metaProf.keyCount(); k++)
    {
        if (m_metaProf.value(k) == m_profession)
        {
            ASSIGN_LE32( cdata+0x0069, k );
            break;
        }
    }
    /* [0x006d--0x0070] */
    for (int k=0; k<m_metaProf.keyCount(); k++)
    {
        if (m_metaProf.value(k) == m_origProfession)
        {
            ASSIGN_LE32( cdata+0x006d, k );
            break;
        }
    }
    /* [0x0071--0x0074] */
    for (int k=0; k<m_metaRace.keyCount(); k++)
    {
        if (m_metaRace.value(k) == m_race)
        {
            ASSIGN_LE32( cdata+0x0071, k );
            break;
        }
    }
    /* [0x0075--0x0078] */
    for (int k=0; k<m_metaGender.keyCount(); k++)
    {
        if (m_metaGender.value(k) == m_gender)
        {
            ASSIGN_LE32( cdata+0x0075, k );
            break;
        }
    }
    /* [0x0079--0x007c] */
    ASSIGN_LE32( cdata+0x0079, m_portraitIndex );
    /* [0x0081--0x0084] */
    ASSIGN_LE32( cdata+0x0081, static_cast<qint32>(m_personality) );
    /* [0x0085--0x0088] */
    ASSIGN_LE32( cdata+0x0085, m_voice );

    /* [0x0089--0x008c] */
    ASSIGN_LE32( cdata+0x0089, m_combinedLevel );
    /* [0x008d--0x00c8] */
    ASSIGN_LE32( cdata+0x008d, m_currentLevels.value( profession::Fighter,   0 ) );
    ASSIGN_LE32( cdata+0x0091, m_currentLevels.value( profession::Lord,      0 ) );
    ASSIGN_LE32( cdata+0x0095, m_currentLevels.value( profession::Valkyrie,  0 ) );
    ASSIGN_LE32( cdata+0x0099, m_currentLevels.value( profession::Ranger,    0 ) );
    ASSIGN_LE32( cdata+0x009d, m_currentLevels.value( profession::Samurai,   0 ) );
    ASSIGN_LE32( cdata+0x00a1, m_currentLevels.value( profession::Ninja,     0 ) );
    ASSIGN_LE32( cdata+0x00a5, m_currentLevels.value( profession::Monk,      0 ) );
    ASSIGN_LE32( cdata+0x00a9, m_currentLevels.value( profession::Rogue,     0 ) );
    ASSIGN_LE32( cdata+0x00ad, m_currentLevels.value( profession::Gadgeteer, 0 ) );
    ASSIGN_LE32( cdata+0x00b1, m_currentLevels.value( profession::Bard,      0 ) );
    ASSIGN_LE32( cdata+0x00b5, m_currentLevels.value( profession::Priest,    0 ) );
    ASSIGN_LE32( cdata+0x00b9, m_currentLevels.value( profession::Alchemist, 0 ) );
    ASSIGN_LE32( cdata+0x00bd, m_currentLevels.value( profession::Bishop,    0 ) );
    ASSIGN_LE32( cdata+0x00c1, m_currentLevels.value( profession::Psionic,   0 ) );
    ASSIGN_LE32( cdata+0x00c5, m_currentLevels.value( profession::Mage,      0 ) );

    /* [0x00e5--0x0170] */
    assignAttributeDetail( cdata+0x00e5, &m_attribs[ attribute::Strength     ] );
    assignAttributeDetail( cdata+0x00f9, &m_attribs[ attribute::Intelligence ] );
    assignAttributeDetail( cdata+0x010d, &m_attribs[ attribute::Piety        ] );
    assignAttributeDetail( cdata+0x0121, &m_attribs[ attribute::Vitality     ] );
    assignAttributeDetail( cdata+0x0135, &m_attribs[ attribute::Dexterity    ] );
    assignAttributeDetail( cdata+0x0149, &m_attribs[ attribute::Speed        ] );
    assignAttributeDetail( cdata+0x015d, &m_attribs[ attribute::Senses       ] );

    /* [0x019d--0x07b2] */
    assignSkillDetail( cdata+0x019d, &m_skills[ skill::Sword          ] );
    assignSkillDetail( cdata+0x01c3, &m_skills[ skill::Axe            ] );
    assignSkillDetail( cdata+0x01e9, &m_skills[ skill::Polearm        ] );
    assignSkillDetail( cdata+0x020f, &m_skills[ skill::Mace_Flail     ] );
    assignSkillDetail( cdata+0x0235, &m_skills[ skill::Dagger         ] );
    assignSkillDetail( cdata+0x025b, &m_skills[ skill::Staff_Wand     ] );
    assignSkillDetail( cdata+0x0281, &m_skills[ skill::Shield         ] );
    assignSkillDetail( cdata+0x02a7, &m_skills[ skill::ModernWeapon   ] );
    assignSkillDetail( cdata+0x02cd, &m_skills[ skill::Bow            ] );
    assignSkillDetail( cdata+0x02f3, &m_skills[ skill::Throwing_Sling ] );
    assignSkillDetail( cdata+0x0319, &m_skills[ skill::Locks_Traps    ] );
    assignSkillDetail( cdata+0x033f, &m_skills[ skill::Stealth        ] );
    assignSkillDetail( cdata+0x0365, &m_skills[ skill::Music          ] );
    assignSkillDetail( cdata+0x038b, &m_skills[ skill::Pickpocket     ] );
    assignSkillDetail( cdata+0x03b1, &m_skills[ skill::MartialArts    ] );
    assignSkillDetail( cdata+0x03d7, &m_skills[ skill::Scouting       ] );
    assignSkillDetail( cdata+0x03fd, &m_skills[ skill::CloseCombat    ] );
    assignSkillDetail( cdata+0x0423, &m_skills[ skill::RangedCombat   ] );
    assignSkillDetail( cdata+0x0449, &m_skills[ skill::DualWeapons    ] );
    assignSkillDetail( cdata+0x046f, &m_skills[ skill::CriticalStrike ] );
    assignSkillDetail( cdata+0x0495, &m_skills[ skill::Artifacts      ] );
    assignSkillDetail( cdata+0x04bb, &m_skills[ skill::Mythology      ] );
    assignSkillDetail( cdata+0x04e1, &m_skills[ skill::Communication  ] );
    assignSkillDetail( cdata+0x0507, &m_skills[ skill::Engineering    ] );
    assignSkillDetail( cdata+0x052d, &m_skills[ skill::Wizardry       ] );
    assignSkillDetail( cdata+0x0553, &m_skills[ skill::Divinity       ] );
    assignSkillDetail( cdata+0x0579, &m_skills[ skill::Alchemy        ] );
    assignSkillDetail( cdata+0x059f, &m_skills[ skill::Psionics       ] );
    assignSkillDetail( cdata+0x05c5, &m_skills[ skill::FireMagic      ] );
    assignSkillDetail( cdata+0x05eb, &m_skills[ skill::WaterMagic     ] );
    assignSkillDetail( cdata+0x0611, &m_skills[ skill::AirMagic       ] );
    assignSkillDetail( cdata+0x0637, &m_skills[ skill::EarthMagic     ] );
    assignSkillDetail( cdata+0x065d, &m_skills[ skill::MentalMagic    ] );
    assignSkillDetail( cdata+0x0683, &m_skills[ skill::DivineMagic    ] );
    assignSkillDetail( cdata+0x06a9, &m_skills[ skill::PowerStrike    ] );
    assignSkillDetail( cdata+0x06cf, &m_skills[ skill::PowerCast      ] );
    assignSkillDetail( cdata+0x06f5, &m_skills[ skill::IronWill       ] );
    assignSkillDetail( cdata+0x071b, &m_skills[ skill::IronSkin       ] );
    assignSkillDetail( cdata+0x0741, &m_skills[ skill::Reflexion      ] );
    assignSkillDetail( cdata+0x0767, &m_skills[ skill::SnakeSpeed     ] );
    assignSkillDetail( cdata+0x078d, &m_skills[ skill::EagleEye       ] );

    /* [0x09ed--0x09f0] */
    ASSIGN_LE32( cdata+0x9ed, m_xp );
    /* [0x09f1--0x09f4] */
    ASSIGN_LE32( cdata+0x9f1, m_xp_needed );
    /* [0x09f5--0x09f8] */
    ASSIGN_LE32( cdata+0x9f5, m_xp_last_needed );
    /* [0x09f9--0x09fc] */
    ASSIGN_LE32( cdata+0x9f9, m_kills );
    /* [0x09fd--0x0a00] */
    ASSIGN_LE32( cdata+0x9fd, m_deaths );

    /* [0x0a01--0x0a50] */
    // If a condition is active the numeric value holds the time the condition will
    // be active for. Each tick represents 10 seconds, so a value of 0x00000006 is 1 minute.
    // Values >= 0x0000270f (9999) are permanent for any condition until healed.
    // If the character is Dead or Missing (m_main_status_condition set to one of these) then
    // the timer stops.
    // There are graphics for a Possessed condition in the game, but no support here for it.

    quint32 conditions[ condition::CONDITION_SIZE ];
    for (int k=0; k < condition::CONDITION_SIZE; k++)
    {
        if (m_conditions_active[k] == false)
            conditions[k] = 0;
        else
            conditions[k] = m_conditions[ k ];
    }

    ASSIGN_LE32( cdata+0x0a01, conditions[ Normal       ] );
    ASSIGN_LE32( cdata+0x0a05, conditions[ Drained      ] ); // should set/clear m_stamina_drain and m_hp_drain
    ASSIGN_LE32( cdata+0x0a09, conditions[ Diseased     ] );
    ASSIGN_LE32( cdata+0x0a0d, conditions[ Irritated    ] );
    ASSIGN_LE32( cdata+0x0a11, conditions[ Nauseated    ] );
    ASSIGN_LE32( cdata+0x0a15, conditions[ Slowed       ] );
    ASSIGN_LE32( cdata+0x0a19, conditions[ Afraid       ] );
    ASSIGN_LE32( cdata+0x0a1d, conditions[ Poisoned     ] ); // should set/clear m_poison_strength - got 0x00000010 for 100 is Cosmic Forge == (160 seconds)
    ASSIGN_LE32( cdata+0x0a21, conditions[ Silenced     ] );
    ASSIGN_LE32( cdata+0x0a25, conditions[ Hexed        ] );
    ASSIGN_LE32( cdata+0x0a29, conditions[ Enthralled   ] );
    ASSIGN_LE32( cdata+0x0a2d, conditions[ Insane       ] );
    ASSIGN_LE32( cdata+0x0a31, conditions[ Blind        ] );
    ASSIGN_LE32( cdata+0x0a35, conditions[ Turncoat     ] );
    ASSIGN_LE32( cdata+0x0a39, conditions[ Webbed       ] );
    ASSIGN_LE32( cdata+0x0a3d, conditions[ Asleep       ] );
    ASSIGN_LE32( cdata+0x0a41, conditions[ Paralyzed    ] );
    ASSIGN_LE32( cdata+0x0a45, conditions[ Unconscious  ] );
    ASSIGN_LE32( cdata+0x0a49, conditions[ Dead         ] );
    ASSIGN_LE32( cdata+0x0a4d, conditions[ Missing      ] ); // dangerous to alter because affect gameplot

    /* [0x0a65--0x0ac4] */
    // same sort of <0x270f countdown timers as the conditions
    /* -- unsupported currently
    ASSIGN_LE32( cdata+0x0a65, m_enchantments[ Normal        ].powerlevel );
    ASSIGN_LE16( cdata+0x0a69, m_enchantments[ Normal        ].bonus      ); // usually 0
    ASSIGN_LE32( cdata+0x0a6d, m_enchantments[ Normal        ].duration   );
    ASSIGN_LE32( cdata+0x0a71, m_enchantments[ DraconBreath  ].powerlevel );
    ASSIGN_LE16( cdata+0x0a75, m_enchantments[ DraconBreath  ].bonus      );
    ASSIGN_LE32( cdata+0x0a79, m_enchantments[ DraconBreath  ].duration   );
    ASSIGN_LE32( cdata+0x0a7d, m_enchantments[ GuardianAngel ].powerlevel );
    ASSIGN_LE16( cdata+0x0a81, m_enchantments[ GuardianAngel ].bonus      );
    ASSIGN_LE32( cdata+0x0a85, m_enchantments[ GuardianAngel ].duration   );
    ASSIGN_LE32( cdata+0x0a89, m_enchantments[ RazorCloak    ].powerlevel );
    ASSIGN_LE16( cdata+0x0a8d, m_enchantments[ RazorCloak    ].bonus      );
    ASSIGN_LE32( cdata+0x0a91, m_enchantments[ RazorCloak    ].duration   );
    ASSIGN_LE32( cdata+0x0a95, m_enchantments[ EyeForAnEye   ].powerlevel );
    ASSIGN_LE16( cdata+0x0a99, m_enchantments[ EyeForAnEye   ].bonus      );
    ASSIGN_LE32( cdata+0x0a9d, m_enchantments[ EyeForAnEye   ].duration   );
    ASSIGN_LE32( cdata+0x0aa1, m_enchantments[ Haste         ].powerlevel );
    ASSIGN_LE16( cdata+0x0aa5, m_enchantments[ Haste         ].bonus      );
    ASSIGN_LE32( cdata+0x0aa9, m_enchantments[ Haste         ].duration   );
    ASSIGN_LE32( cdata+0x0aad, m_enchantments[ Superman      ].powerlevel );
    ASSIGN_LE16( cdata+0x0ab1, m_enchantments[ Superman      ].bonus      );
    ASSIGN_LE32( cdata+0x0ab5, m_enchantments[ Superman      ].duration   );
    ASSIGN_LE32( cdata+0x0ab9, m_enchantments[ BodyOfStone   ].powerlevel );
    ASSIGN_LE16( cdata+0x0abd, m_enchantments[ BodyOfStone   ].bonus      );
    ASSIGN_LE32( cdata+0x0ac1, m_enchantments[ BodyOfStone   ].duration   );
    */

    /* [0x0b01--0x0b04] */
    ASSIGN_LE32( cdata+0x0b01, m_main_status_condition ); // later indexes = higher priority
    /* [0x0b05--0x0b08] */
    //ASSIGN_LE32( cdata+0x0b05, m_main_enchantment );    // later indexes = higher priority
    /* [0x0b09--0x0b0c] */
    ASSIGN_LE32( cdata+0x0b09, m_poison_strength ); // 0-255

    // Health and Stamina Points - calculated attributes
    /* [0x0b0d--0x0b24] */
    ASSIGN_LE32( cdata+0x0b0d, m_hp[atIdx::Base]         );
    ASSIGN_LE32( cdata+0x0b11, m_hp[atIdx::Current]      );
    ASSIGN_LE32( cdata+0x0b15, m_hp_drain                );
    ASSIGN_LE32( cdata+0x0b19, m_stamina[atIdx::Base]    );
    ASSIGN_LE32( cdata+0x0b1d, m_stamina[atIdx::Current] );
    ASSIGN_LE32( cdata+0x0b21, m_stamina_drain           );

    /* [0x0b25--0x0b3c] */
    ASSIGN_LE32( cdata+0x0b25, m_mp[realm::Fire][atIdx::Base]      );
    ASSIGN_LE32( cdata+0x0b29, m_mp[realm::Water][atIdx::Base]     );
    ASSIGN_LE32( cdata+0x0b2d, m_mp[realm::Air][atIdx::Base]       );
    ASSIGN_LE32( cdata+0x0b31, m_mp[realm::Earth][atIdx::Base]     );
    ASSIGN_LE32( cdata+0x0b35, m_mp[realm::Mental][atIdx::Base]    );
    ASSIGN_LE32( cdata+0x0b39, m_mp[realm::Divine][atIdx::Base]    );

    /* [0x0b45--0x0b5c] */
    ASSIGN_LE32( cdata+0x0b45, m_mp[realm::Fire][atIdx::Current]   );
    ASSIGN_LE32( cdata+0x0b49, m_mp[realm::Water][atIdx::Current]  );
    ASSIGN_LE32( cdata+0x0b4d, m_mp[realm::Air][atIdx::Current]    );
    ASSIGN_LE32( cdata+0x0b51, m_mp[realm::Earth][atIdx::Current]  );
    ASSIGN_LE32( cdata+0x0b55, m_mp[realm::Mental][atIdx::Current] );
    ASSIGN_LE32( cdata+0x0b59, m_mp[realm::Divine][atIdx::Current] );

    /* [0x0b69--0x0b6c] */
    ASSIGN_FLOAT( cdata+0x0b69, m_healing_rate );

    /* [0x0b71--0x0b75] */
    ASSIGN_FLOAT( cdata+0x0b71, m_rest_rate );

    /* [0x0b79--0x0b90] */
    ASSIGN_FLOAT( cdata+0x0b79, m_mp_recovery[realm::Fire][0]   );
    ASSIGN_FLOAT( cdata+0x0b7d, m_mp_recovery[realm::Water][0]  );
    ASSIGN_FLOAT( cdata+0x0b81, m_mp_recovery[realm::Air][0]    );
    ASSIGN_FLOAT( cdata+0x0b85, m_mp_recovery[realm::Earth][0]  );
    ASSIGN_FLOAT( cdata+0x0b89, m_mp_recovery[realm::Mental][0] );
    ASSIGN_FLOAT( cdata+0x0b8d, m_mp_recovery[realm::Divine][0] );

    /* [0x0bb9--0x0bbc] */
    ASSIGN_LE32( cdata+0x0bb9, m_personal_load  );
    /* [0x0bbd--0x0bc0] */
    ASSIGN_LE32( cdata+0x0bbd, m_party_share    );
    /* [0x0bc1--0x0bc4] */
    ASSIGN_LE32( cdata+0x0bc1, m_encumbrance    );
    /* [0x0bc5--0x0bc8] */
    ASSIGN_LE32( cdata+0x0bc5, m_carry_capacity );
    /* [0x0bc9--0x0bcc] */
    ASSIGN_LE32( cdata+0x0bc9, m_load_category  );

    /* [0x0bcd--0x0d91] */ // Spells known
    for (int k = 0; k < MAXIMUM_CHARACTER_SPELLS; k++)
    {
        ASSIGN_LE32( cdata+0x0bcd + k*4, m_spell[k] );
    }

    /* [0x0e5d--0x0e74] */
    ASSIGN_LE32( cdata+0x0e5d, m_knownSpellsCount[realm::Fire]   );
    ASSIGN_LE32( cdata+0x0e61, m_knownSpellsCount[realm::Water]  );
    ASSIGN_LE32( cdata+0x0e65, m_knownSpellsCount[realm::Air]    );
    ASSIGN_LE32( cdata+0x0e69, m_knownSpellsCount[realm::Earth]  );
    ASSIGN_LE32( cdata+0x0e6d, m_knownSpellsCount[realm::Mental] );
    ASSIGN_LE32( cdata+0x0e71, m_knownSpellsCount[realm::Divine] );


    /* [0x0e7d--0x0ec0] */
    ASSIGN_LE32( cdata+0x0e7d, m_mpStrongestRealm  );
    ASSIGN_LE32( cdata+0x0e81, m_initiative        );
    ASSIGN_LE32( cdata+0x0e85, m_acmod_base        );
    ASSIGN_LE32( cdata+0x0e89, m_acmod_average     );
    ASSIGN_LE32( cdata+0x0e8d, m_acmod_race        );
    ASSIGN_LE32( cdata+0x0e91, m_acmod_speed       );
    ASSIGN_LE32( cdata+0x0e95, m_acmod_stealth     );
    ASSIGN_LE32( cdata+0x0e99, m_acmod_shield      );
    ASSIGN_LE32( cdata+0x0e9d, m_acmod_mitems      );
    ASSIGN_LE32( cdata+0x0ea1, m_acmod_mspells     );
    ASSIGN_LE32( cdata+0x0ea5, m_acmod_penetrate   );
    ASSIGN_LE32( cdata+0x0ea9, m_acmod_encumbrance );
    ASSIGN_LE32( cdata+0x0ead, m_acmod_conditions  );
    ASSIGN_LE32( cdata+0x0eb1, m_acmod_fatigue     );
    ASSIGN_LE32( cdata+0x0eb5, m_acmod_defensive   );
    ASSIGN_LE32( cdata+0x0eb9, m_acmod_reflextion  );
    ASSIGN_LE32( cdata+0x0ebd, m_acmod_unknown     );

    /* [0x0ec1--0x0ed4] */
    ASSIGN_LE32( cdata+0x0ec1, m_ac_bodypart[ worn::Head  ] );
    ASSIGN_LE32( cdata+0x0ec5, m_ac_bodypart[ worn::Torso ] );
    ASSIGN_LE32( cdata+0x0ec9, m_ac_bodypart[ worn::Legs  ] );
    ASSIGN_LE32( cdata+0x0ecd, m_ac_bodypart[ worn::Hand  ] );
    ASSIGN_LE32( cdata+0x0ed1, m_ac_bodypart[ worn::Feet  ] );

    /* [0x0ed9--0x0edc] */
    ASSIGN_LE32( cdata+0x0ed9, m_damage_absorption  );

    // Magic resistances
    /* [0x0edd--0x0ee4] */
    ASSIGN_LE32( cdata+0x0edd, m_magic_resistance[realm::Fire][atIdx::Base]      );
    ASSIGN_LE32( cdata+0x0ee1, m_magic_resistance[realm::Fire][atIdx::Current]   );

    /* [0x0eed--0x0ef4] */
    ASSIGN_LE32( cdata+0x0eed, m_magic_resistance[realm::Water][atIdx::Base]     );
    ASSIGN_LE32( cdata+0x0ef1, m_magic_resistance[realm::Water][atIdx::Current]  );

    /* [0x0efd--0x0f04] */
    ASSIGN_LE32( cdata+0x0efd, m_magic_resistance[realm::Air][atIdx::Base]       );
    ASSIGN_LE32( cdata+0x0f01, m_magic_resistance[realm::Air][atIdx::Current]    );

    /* [0x0f0d--0x0f14] */
    ASSIGN_LE32( cdata+0x0f0d, m_magic_resistance[realm::Earth][atIdx::Base]     );
    ASSIGN_LE32( cdata+0x0f11, m_magic_resistance[realm::Earth][atIdx::Current]  );

    /* [0x0f1d--0x0f24] */
    ASSIGN_LE32( cdata+0x0f1d, m_magic_resistance[realm::Mental][atIdx::Base]    );
    ASSIGN_LE32( cdata+0x0f21, m_magic_resistance[realm::Mental][atIdx::Current] );

    /* [0x0f2d--0x0f34] */
    ASSIGN_LE32( cdata+0x0f2d, m_magic_resistance[realm::Divine][atIdx::Base]    );
    ASSIGN_LE32( cdata+0x0f31, m_magic_resistance[realm::Divine][atIdx::Current] );

    /* [0x0f5d--0x0fec] */
    assignItem( cdata+0x0f5d, worn::Head     );
    assignItem( cdata+0x0f69, worn::Misc1    );
    assignItem( cdata+0x0f75, worn::Misc2    );
    assignItem( cdata+0x0f81, worn::Cloak    );
    assignItem( cdata+0x0f8d, worn::Torso    );
    assignItem( cdata+0x0f99, worn::Hand     );
    assignItem( cdata+0x0fa5, worn::Weapon1a );
    assignItem( cdata+0x0fb1, worn::Weapon1b );
    assignItem( cdata+0x0fbd, worn::Weapon2a );
    assignItem( cdata+0x0fc9, worn::Weapon2b );
    assignItem( cdata+0x0fd5, worn::Legs     );
    assignItem( cdata+0x0fe1, worn::Feet     );

    /* [0x1029--0x1088] */ // Items carried by character
    for (int k=worn::Backpack1; k<=worn::Backpack8; k++)
    {
        assignItem( cdata+0x1029 + (k - worn::Backpack1)*12, (character::worn)k);
    }

    // Primary Weapon parameters
    /* [0x1149--0x118b] */
    ASSIGN_LE32( cdata+0x1149, m_attack[0].weapon_type  );
    ASSIGN_LE8(  cdata+0x114d, m_attack[0].enabled      );
    ASSIGN_LE32( cdata+0x114e, static_cast<qint32>(m_attack[0].skill1) );
    ASSIGN_LE32( cdata+0x1152, static_cast<qint32>(m_attack[0].skill2) );
    ASSIGN_LE32( cdata+0x1156, m_attack[0].skill_total  );
    ASSIGN_LE32( cdata+0x115a, m_attack[0].BAR          );
    ASSIGN_LE32( cdata+0x115e, m_attack[0].num_attacks  );
    ASSIGN_LE32( cdata+0x1162, m_attack[0].max_swings   );
    ASSIGN_LE32( cdata+0x1166, m_attack[0].to_init      );
    ASSIGN_LE32( cdata+0x116a, m_attack[0].MAR          );
    ASSIGN_LE32( cdata+0x116e, m_attack[0].to_penetrate );
    ASSIGN_LE32( cdata+0x1172, m_attack[0].multiplier   );
    ASSIGN_LE16( cdata+0x1176, m_attack[0].damage_c     );
    ASSIGN_LE8(  cdata+0x1178, m_attack[0].damage_num_dice   );
    ASSIGN_LE8(  cdata+0x1179, m_attack[0].damage_dice_sides );
    ASSIGN_LE16( cdata+0x117a, m_attack[0].attack_modes );
    ASSIGN_LE32( cdata+0x117c, m_attack[0].unknown51    );
    ASSIGN_LE16( cdata+0x1180, m_attack[0].unknown55    );
    ASSIGN_LE8(  cdata+0x1182, m_attack[0].unknown57    );
    ASSIGN_LE8(  cdata+0x1183, m_attack[0].unknown58    );
    ASSIGN_LE32( cdata+0x1184, m_attack[0].unknown59    );
    ASSIGN_LE32( cdata+0x1188, m_attack[0].unknown63    );

    // Offhand Weapon parameters
    /* [0x11a4--0x11e6] */
    ASSIGN_LE32( cdata+0x11a4, m_attack[1].weapon_type  );
    ASSIGN_LE8(  cdata+0x11a8, m_attack[1].enabled      );
    ASSIGN_LE32( cdata+0x11a9, static_cast<qint32>(m_attack[1].skill1) );
    ASSIGN_LE32( cdata+0x11ad, static_cast<qint32>(m_attack[1].skill2) );
    ASSIGN_LE32( cdata+0x11b1, m_attack[1].skill_total  );
    ASSIGN_LE32( cdata+0x11b5, m_attack[1].BAR          );
    ASSIGN_LE32( cdata+0x11b9, m_attack[1].num_attacks  );
    ASSIGN_LE32( cdata+0x11bd, m_attack[1].max_swings   );
    ASSIGN_LE32( cdata+0x11c1, m_attack[1].to_init      );
    ASSIGN_LE32( cdata+0x11c5, m_attack[1].MAR          );
    ASSIGN_LE32( cdata+0x11c9, m_attack[1].to_penetrate );
    ASSIGN_LE16( cdata+0x11cd, m_attack[1].multiplier   );
    ASSIGN_LE16( cdata+0x11d1, m_attack[1].damage_c     );
    ASSIGN_LE8(  cdata+0x11d3, m_attack[1].damage_num_dice   );
    ASSIGN_LE8(  cdata+0x11d4, m_attack[1].damage_dice_sides );
    ASSIGN_LE16( cdata+0x11d5, m_attack[1].attack_modes );
    ASSIGN_LE32( cdata+0x11d7, m_attack[1].unknown51    );
    ASSIGN_LE16( cdata+0x11db, m_attack[1].unknown55    );
    ASSIGN_LE8(  cdata+0x11dd, m_attack[1].unknown57    );
    ASSIGN_LE8(  cdata+0x11de, m_attack[1].unknown58    );
    ASSIGN_LE32( cdata+0x11df, m_attack[1].unknown59    );
    ASSIGN_LE32( cdata+0x11e3, m_attack[1].unknown63    );

    /* [0x12b5        ] */
    ASSIGN_LE8( cdata+0x12b5, m_dual_wielding );

    /* [0x169e--0x16a1] */
    ASSIGN_LE32( cdata+0x169e, m_fatigue_category );

    /* [0x1770--0x177b] */
    ASSIGN_LE8( cdata+0x1770, m_bonus.to_init         );
    ASSIGN_LE8( cdata+0x1771, m_bonus.MAR             );
    ASSIGN_LE8( cdata+0x1772, m_bonus.to_penetrate    );
    ASSIGN_LE8( cdata+0x1773, m_bonus.multiplier      );
    ASSIGN_LE8( cdata+0x1774, m_bonus.mspells         );
    ASSIGN_LE8( cdata+0x1775, m_bonus.vpenetrate      );
    ASSIGN_LE8( cdata+0x1776, m_bonus.absorption      );
    ASSIGN_LE8( cdata+0x1777, m_bonus.allmagic_resist );
    ASSIGN_LE8( cdata+0x1778, m_bonus.unknown_0x1778  );
    ASSIGN_LE8( cdata+0x1779, m_bonus.hp_regen        );
    ASSIGN_LE8( cdata+0x177a, m_bonus.stamina_regen   );
    ASSIGN_LE8( cdata+0x177b, m_bonus.sp_regen        );

    // [0x177c--0x1782] Attribute bonuses
    for (int k=0; k<attribute::ATTRIBUTE_SIZE; k++)
    {
        cdata[0x177c + k ] = m_bonus.attributes[ k ];
    }

    // [0x1783--0x17ab] Skill bonuses
    for (int k=0; k<skill::SKILL_SIZE; k++)
    {
        cdata[0x1783 + k ] = m_bonus.skills[ k ];
    }

    /* [0x17ac--0x17b5] */
    ASSIGN_LE8( cdata+0x17ac, m_bonus.magic_resistance[realm::Fire  ] );
    ASSIGN_LE8( cdata+0x17ad, m_bonus.magic_resistance[realm::Water ] );
    ASSIGN_LE8( cdata+0x17ae, m_bonus.magic_resistance[realm::Air   ] );
    ASSIGN_LE8( cdata+0x17af, m_bonus.magic_resistance[realm::Earth ] );
    ASSIGN_LE8( cdata+0x17b0, m_bonus.magic_resistance[realm::Mental] );
    ASSIGN_LE8( cdata+0x17b1, m_bonus.magic_resistance[realm::Divine] );

    ASSIGN_LE8( cdata+0x17b2, m_bonus.unknown_0x17b2                  );
    ASSIGN_LE8( cdata+0x17b3, m_bonus.unknown_0x17b3                  );
    ASSIGN_LE8( cdata+0x17b4, m_bonus.unknown_0x17b4                  );
    ASSIGN_LE8( cdata+0x17b5, m_bonus.incapacitated                   );

    /* [0x17bb        ] */
    ASSIGN_LE8( cdata+0x17bb, m_bonus.conditions );

    // Portal Location
    /* [0x17db--0x1816] */
    ASSIGN_FLOAT( cdata+0x17db, m_portal_x * 500.0 );
    ASSIGN_FLOAT( cdata+0x17df, m_portal_y * 500.0 );
    ASSIGN_FLOAT( cdata+0x17e3, m_portal_z * 500.0 );
    ASSIGN_FLOAT( cdata+0x17e7, m_portal_pitch_or_direction_dunno );
    ASSIGN_FLOAT( cdata+0x17ff, m_portal_direction_or_pitch_dunno );
    ASSIGN_LE32(  cdata+0x1813, m_portal_map );

    /* [0x1830        ] */
    ASSIGN_LE8( cdata+0x1830, m_unknown_0x1830 );

    return response;
}

void character::unpackCharacter(const QByteArray &c, const QByteArray &cx)
{
    const quint8 *cdata = (const quint8 *) c.constData();

    /* [0x0004        ] */ m_in_party = FORMAT_8(    cdata+0x0004 );             // in the party
    /* [0x0005--0x0018] */ m_charName = formatUtf16( cdata+0x0005, 10 );         // nickname
    /* [0x0019--0x003f] */ m_fullName = formatUtf16( cdata+0x0019, 40 );         // full name
    /* [0x0040--0x0068] */ // UNKNOWN
    /* [0x0069--0x006c] */ m_profession     = formatProfession(  cdata+0x0069 ); // current profession
    /* [0x006d--0x0070] */ m_origProfession = formatProfession(  cdata+0x006d ); // starting profession
    /* [0x0071--0x0074] */ m_race           = formatRace(        cdata+0x0071 ); // race
    /* [0x0075--0x0078] */ m_gender         = formatGender(      cdata+0x0075 ); // gender
    /* [0x0079--0x007c] */ m_portraitIndex  = FORMAT_LE32(       cdata+0x0079 ); // character image
    /* [0x007d--0x0080] */ // UNKNOWN
    /* [0x0081--0x0084] */ m_personality    = formatPersonality( cdata+0x0081 ); // personality
    /* [0x0085--0x0088] */ m_voice          = FORMAT_LE32(       cdata+0x0085 ); // voice

    // Levels
    /* [0x0089--0x008c] */ m_combinedLevel  = FORMAT_LE32(       cdata+0x0089 ); // levels across all professions
    /* [0x008d--0x0090] */ CHECK_SET( m_currentLevels[ profession::Fighter   ], FORMAT_LE32( cdata+0x008d ));
    /* [0x0091--0x0094] */ CHECK_SET( m_currentLevels[ profession::Lord      ], FORMAT_LE32( cdata+0x0091 ));
    /* [0x0095--0x0098] */ CHECK_SET( m_currentLevels[ profession::Valkyrie  ], FORMAT_LE32( cdata+0x0095 ));
    /* [0x0099--0x009c] */ CHECK_SET( m_currentLevels[ profession::Ranger    ], FORMAT_LE32( cdata+0x0099 ));
    /* [0x009d--0x00a0] */ CHECK_SET( m_currentLevels[ profession::Samurai   ], FORMAT_LE32( cdata+0x009d ));
    /* [0x00a1--0x00a4] */ CHECK_SET( m_currentLevels[ profession::Ninja     ], FORMAT_LE32( cdata+0x00a1 ));
    /* [0x00a5--0x00a8] */ CHECK_SET( m_currentLevels[ profession::Monk      ], FORMAT_LE32( cdata+0x00a5 ));
    /* [0x00a9--0x00ac] */ CHECK_SET( m_currentLevels[ profession::Rogue     ], FORMAT_LE32( cdata+0x00a9 ));
    /* [0x00ad--0x00b0] */ CHECK_SET( m_currentLevels[ profession::Gadgeteer ], FORMAT_LE32( cdata+0x00ad ));
    /* [0x00b1--0x00b4] */ CHECK_SET( m_currentLevels[ profession::Bard      ], FORMAT_LE32( cdata+0x00b1 ));
    /* [0x00b5--0x00b8] */ CHECK_SET( m_currentLevels[ profession::Priest    ], FORMAT_LE32( cdata+0x00b5 ));
    /* [0x00b9--0x00bc] */ CHECK_SET( m_currentLevels[ profession::Alchemist ], FORMAT_LE32( cdata+0x00b9 ));
    /* [0x00bd--0x00c0] */ CHECK_SET( m_currentLevels[ profession::Bishop    ], FORMAT_LE32( cdata+0x00bd ));
    /* [0x00c1--0x00c4] */ CHECK_SET( m_currentLevels[ profession::Psionic   ], FORMAT_LE32( cdata+0x00c1 ));
    /* [0x00c5--0x00c8] */ CHECK_SET( m_currentLevels[ profession::Mage      ], FORMAT_LE32( cdata+0x00c5 ));

    // Copy the m_currentLevels into m_initialLevels, preserving any unset professions
    QMapIterator<profession, int>  i(m_currentLevels);
    while (i.hasNext())
    {
        i.next();
        m_initialLevels[ i.key() ] = i.value();
    }

    /* [0x00c9--0x00e4] */ // UNKNOWN

    // Primary attributes
    /* [0x00e5--0x00f8] */ formatAttributeDetail( &m_attribs[ attribute::Strength     ], cdata+0x00e5 );
    /* [0x00f9--0x010c] */ formatAttributeDetail( &m_attribs[ attribute::Intelligence ], cdata+0x00f9 );
    /* [0x010d--0x0120] */ formatAttributeDetail( &m_attribs[ attribute::Piety        ], cdata+0x010d );
    /* [0x0121--0x0134] */ formatAttributeDetail( &m_attribs[ attribute::Vitality     ], cdata+0x0121 );
    /* [0x0135--0x0148] */ formatAttributeDetail( &m_attribs[ attribute::Dexterity    ], cdata+0x0135 );
    /* [0x0149--0x015c] */ formatAttributeDetail( &m_attribs[ attribute::Speed        ], cdata+0x0149 );
    /* [0x015d--0x0170] */ formatAttributeDetail( &m_attribs[ attribute::Senses       ], cdata+0x015d );

    /* [0x0171--0x019c] */ // UNKNOWN

    // Skill Levels and Training progress
    /* [0x019d--0x01c2] */ formatSkillDetail( &m_skills[ skill::Sword          ], cdata+0x019d );
    /* [0x01c3--0x01e8] */ formatSkillDetail( &m_skills[ skill::Axe            ], cdata+0x01c3 );
    /* [0x01e9--0x020e] */ formatSkillDetail( &m_skills[ skill::Polearm        ], cdata+0x01e9 );
    /* [0x020f--0x0234] */ formatSkillDetail( &m_skills[ skill::Mace_Flail     ], cdata+0x020f );
    /* [0x0235--0x025a] */ formatSkillDetail( &m_skills[ skill::Dagger         ], cdata+0x0235 );
    /* [0x025b--0x0280] */ formatSkillDetail( &m_skills[ skill::Staff_Wand     ], cdata+0x025b );
    /* [0x0281--0x02a6] */ formatSkillDetail( &m_skills[ skill::Shield         ], cdata+0x0281 );
    /* [0x02a7--0x02cc] */ formatSkillDetail( &m_skills[ skill::ModernWeapon   ], cdata+0x02a7 );
    /* [0x02cd--0x02f2] */ formatSkillDetail( &m_skills[ skill::Bow            ], cdata+0x02cd );
    /* [0x02f3--0x0318] */ formatSkillDetail( &m_skills[ skill::Throwing_Sling ], cdata+0x02f3 );
    /* [0x0319--0x033e] */ formatSkillDetail( &m_skills[ skill::Locks_Traps    ], cdata+0x0319 );
    /* [0x033f--0x0364] */ formatSkillDetail( &m_skills[ skill::Stealth        ], cdata+0x033f );
    /* [0x0365--0x038a] */ formatSkillDetail( &m_skills[ skill::Music          ], cdata+0x0365 );
    /* [0x038b--0x03b0] */ formatSkillDetail( &m_skills[ skill::Pickpocket     ], cdata+0x038b );
    /* [0x03b1--0x03d6] */ formatSkillDetail( &m_skills[ skill::MartialArts    ], cdata+0x03b1 );
    /* [0x03d7--0x03fc] */ formatSkillDetail( &m_skills[ skill::Scouting       ], cdata+0x03d7 );
    /* [0x03fd--0x0422] */ formatSkillDetail( &m_skills[ skill::CloseCombat    ], cdata+0x03fd );
    /* [0x0423--0x0448] */ formatSkillDetail( &m_skills[ skill::RangedCombat   ], cdata+0x0423 );
    /* [0x0449--0x046e] */ formatSkillDetail( &m_skills[ skill::DualWeapons    ], cdata+0x0449 );
    /* [0x046f--0x0494] */ formatSkillDetail( &m_skills[ skill::CriticalStrike ], cdata+0x046f );
    /* [0x0495--0x04ba] */ formatSkillDetail( &m_skills[ skill::Artifacts      ], cdata+0x0495 );
    /* [0x04bb--0x04e0] */ formatSkillDetail( &m_skills[ skill::Mythology      ], cdata+0x04bb );
    /* [0x04e1--0x0506] */ formatSkillDetail( &m_skills[ skill::Communication  ], cdata+0x04e1 );
    /* [0x0507--0x052c] */ formatSkillDetail( &m_skills[ skill::Engineering    ], cdata+0x0507 );
    /* [0x052d--0x0552] */ formatSkillDetail( &m_skills[ skill::Wizardry       ], cdata+0x052d );
    /* [0x0553--0x0578] */ formatSkillDetail( &m_skills[ skill::Divinity       ], cdata+0x0553 );
    /* [0x0579--0x059e] */ formatSkillDetail( &m_skills[ skill::Alchemy        ], cdata+0x0579 );
    /* [0x059f--0x05c4] */ formatSkillDetail( &m_skills[ skill::Psionics       ], cdata+0x059f );
    /* [0x05c5--0x05ea] */ formatSkillDetail( &m_skills[ skill::FireMagic      ], cdata+0x05c5 );
    /* [0x05eb--0x0610] */ formatSkillDetail( &m_skills[ skill::WaterMagic     ], cdata+0x05eb );
    /* [0x0611--0x0636] */ formatSkillDetail( &m_skills[ skill::AirMagic       ], cdata+0x0611 );
    /* [0x0637--0x065c] */ formatSkillDetail( &m_skills[ skill::EarthMagic     ], cdata+0x0637 );
    /* [0x065d--0x0682] */ formatSkillDetail( &m_skills[ skill::MentalMagic    ], cdata+0x065d );
    /* [0x0683--0x06a8] */ formatSkillDetail( &m_skills[ skill::DivineMagic    ], cdata+0x0683 );
    /* [0x06a9--0x06ce] */ formatSkillDetail( &m_skills[ skill::PowerStrike    ], cdata+0x06a9 );
    /* [0x06cf--0x06f4] */ formatSkillDetail( &m_skills[ skill::PowerCast      ], cdata+0x06cf );
    /* [0x06f5--0x071a] */ formatSkillDetail( &m_skills[ skill::IronWill       ], cdata+0x06f5 );
    /* [0x071b--0x0740] */ formatSkillDetail( &m_skills[ skill::IronSkin       ], cdata+0x071b );
    /* [0x0741--0x0766] */ formatSkillDetail( &m_skills[ skill::Reflexion      ], cdata+0x0741 );
    /* [0x0767--0x078c] */ formatSkillDetail( &m_skills[ skill::SnakeSpeed     ], cdata+0x0767 );
    /* [0x078d--0x07b2] */ formatSkillDetail( &m_skills[ skill::EagleEye       ], cdata+0x078d );

    /* [0x07b3--0x09ec] */ // UNKNOWN
    /* [0x09ed--0x09f0] */ m_xp             = FORMAT_LE32( cdata+0x9ed );        // XP earned
    /* [0x09f1--0x09f4] */ m_xp_needed      = FORMAT_LE32( cdata+0x9f1 );        // XP needed for next level
    /* [0x09f5--0x09f8] */ m_xp_last_needed = FORMAT_LE32( cdata+0x9f5 );        // XP needed for previous level rise -- used to show level progress bar
    /* [0x09f9--0x09fc] */ m_kills          = FORMAT_LE32( cdata+0x9f9 );        // Number of Kills
    /* [0x09fd--0x0a00] */ m_deaths         = FORMAT_LE32( cdata+0x9fd );        // Number of times character died

    // is game time the value getting poked in here?
    /* [0x0a01--0x0a04] */ m_conditions[ Normal       ] = FORMAT_LE32( cdata+0x0a01 ); // must be 0 for "Normal" or shows a blank line in conditions
    /* [0x0a05--0x0a08] */ m_conditions[ Drained      ] = FORMAT_LE32( cdata+0x0a05 );
    /* [0x0a09--0x0a0c] */ m_conditions[ Diseased     ] = FORMAT_LE32( cdata+0x0a09 );
    /* [0x0a0d--0x0a10] */ m_conditions[ Irritated    ] = FORMAT_LE32( cdata+0x0a0d );
    /* [0x0a11--0x0a14] */ m_conditions[ Nauseated    ] = FORMAT_LE32( cdata+0x0a11 );
    /* [0x0a15--0x0a18] */ m_conditions[ Slowed       ] = FORMAT_LE32( cdata+0x0a15 );
    /* [0x0a19--0x0a1c] */ m_conditions[ Afraid       ] = FORMAT_LE32( cdata+0x0a19 );
    /* [0x0a1d--0x0a20] */ m_conditions[ Poisoned     ] = FORMAT_LE32( cdata+0x0a1d );
    /* [0x0a21--0x0a24] */ m_conditions[ Silenced     ] = FORMAT_LE32( cdata+0x0a21 );
    /* [0x0a25--0x0a28] */ m_conditions[ Hexed        ] = FORMAT_LE32( cdata+0x0a25 );
    /* [0x0a29--0x0a2c] */ m_conditions[ Enthralled   ] = FORMAT_LE32( cdata+0x0a29 );
    /* [0x0a2d--0x0a30] */ m_conditions[ Insane       ] = FORMAT_LE32( cdata+0x0a2d );
    /* [0x0a31--0x0a34] */ m_conditions[ Blind        ] = FORMAT_LE32( cdata+0x0a31 );
    /* [0x0a35--0x0a38] */ m_conditions[ Turncoat     ] = FORMAT_LE32( cdata+0x0a35 );
    /* [0x0a39--0x0a3c] */ m_conditions[ Webbed       ] = FORMAT_LE32( cdata+0x0a39 );
    /* [0x0a3d--0x0a40] */ m_conditions[ Asleep       ] = FORMAT_LE32( cdata+0x0a3d );
    /* [0x0a41--0x0a44] */ m_conditions[ Paralyzed    ] = FORMAT_LE32( cdata+0x0a41 );
    /* [0x0a45--0x0a48] */ m_conditions[ Unconscious  ] = FORMAT_LE32( cdata+0x0a45 );
    /* [0x0a49--0x0a4c] */ m_conditions[ Dead         ] = FORMAT_LE32( cdata+0x0a49 );
    /* [0x0a4d--0x0a50] */ m_conditions[ Missing      ] = FORMAT_LE32( cdata+0x0a4d );
    for (int k=0; k < condition::CONDITION_SIZE; k++)
    {
        if (m_conditions[k] == 0)
            m_conditions_active[k] = false;
        else
            m_conditions_active[k] = true;
    }
    /* [0x0a51--0x0b00] */ // UNKNOWN
    /* [0x0b01--0x0b04] */ m_main_status_condition   = formatCondition( cdata+0xb01 ); // if status_condition not set can't see conditions
    /* [0x0b05--0x0b08] */ // UNKNOWN
    /* [0x0b09--0x0b0c] */ m_poison_strength         = FORMAT_LE32( cdata+0xb09 );

    // Health and Stamina Points - calculated attributes
    /* [0x0b0d--0x0b10] */ m_hp[atIdx::Base]         = FORMAT_LE32( cdata+0xb0d ); // HP at full health
    /* [0x0b11--0x0b14] */ m_hp[atIdx::Current]      = FORMAT_LE32( cdata+0xb11 ); // current HP (if character is dead MUST be 0 or they won't be revivable)
    /* [0x0b15--0x0b18] */ m_hp_drain                = FORMAT_LE32( cdata+0xb15 ); // negative value when active - DISEASE _can_ switch it on
    /* [0x0b19--0x0b1c] */ m_stamina[atIdx::Base]    = FORMAT_LE32( cdata+0xb19 ); // Stamina at full health
    /* [0x0b1d--0x0b20] */ m_stamina[atIdx::Current] = FORMAT_LE32( cdata+0xb1d ); // current Stamina
    /* [0x0b21--0x0b24] */ m_stamina_drain           = FORMAT_LE32( cdata+0xb21 ); // positive value when active - DISEASE _can_ switch it on

    // Mana (Magic) Points - calculated attributes
    /* [0x0b25--0x0b28] */ m_mp[realm::Fire][atIdx::Base]      = FORMAT_LE32( cdata+0x0b25 );
    /* [0x0b29--0x0b2c] */ m_mp[realm::Water][atIdx::Base]     = FORMAT_LE32( cdata+0x0b29 );
    /* [0x0b2d--0x0b30] */ m_mp[realm::Air][atIdx::Base]       = FORMAT_LE32( cdata+0x0b2d );
    /* [0x0b31--0x0b34] */ m_mp[realm::Earth][atIdx::Base]     = FORMAT_LE32( cdata+0x0b31 );
    /* [0x0b35--0x0b38] */ m_mp[realm::Mental][atIdx::Base]    = FORMAT_LE32( cdata+0x0b35 );
    /* [0x0b39--0x0b3c] */ m_mp[realm::Divine][atIdx::Base]    = FORMAT_LE32( cdata+0x0b39 );
    /* [0x0b3d--0x0b44] */ // UNKNOWN
    /* [0x0b45--0x0b48] */ m_mp[realm::Fire][atIdx::Current]   = FORMAT_LE32( cdata+0x0b45 );
    /* [0x0b49--0x0b4c] */ m_mp[realm::Water][atIdx::Current]  = FORMAT_LE32( cdata+0x0b49 );
    /* [0x0b4d--0x0b50] */ m_mp[realm::Air][atIdx::Current]    = FORMAT_LE32( cdata+0x0b4d );
    /* [0x0b51--0x0b54] */ m_mp[realm::Earth][atIdx::Current]  = FORMAT_LE32( cdata+0x0b51 );
    /* [0x0b55--0x0b58] */ m_mp[realm::Mental][atIdx::Current] = FORMAT_LE32( cdata+0x0b55 );
    /* [0x0b59--0x0b5c] */ m_mp[realm::Divine][atIdx::Current] = FORMAT_LE32( cdata+0x0b59 );

    /* [0x0b5d--0x0b68] */ // UNKNOWN
    /* [0x0b69--0x0b6c] */ m_healing_rate                      = FORMAT_FLOAT( cdata+0x0b69 );
    /* [0x0b6d--0x0b70] */ // UNKNOWN
    /* [0x0b71--0x0b75] */ m_rest_rate                         = FORMAT_FLOAT( cdata+0x0b71 );
    /* [0x0b76--0x0b78] */ // UNKNOWN
    /* [0x0b79--0x0b7c] */ m_mp_recovery[realm::Fire][0]       = FORMAT_FLOAT( cdata+0x0b79 );
    /* [0x0b7d--0x0b80] */ m_mp_recovery[realm::Water][0]      = FORMAT_FLOAT( cdata+0x0b7d );
    /* [0x0b81--0x0b84] */ m_mp_recovery[realm::Air][0]        = FORMAT_FLOAT( cdata+0x0b81 );
    /* [0x0b85--0x0b88] */ m_mp_recovery[realm::Earth][0]      = FORMAT_FLOAT( cdata+0x0b85 );
    /* [0x0b89--0x0b8c] */ m_mp_recovery[realm::Mental][0]     = FORMAT_FLOAT( cdata+0x0b89 );
    /* [0x0b8d--0x0b90] */ m_mp_recovery[realm::Divine][0]     = FORMAT_FLOAT( cdata+0x0b8d );
    /* [0x0b91--0x0bb8] */ // UNKNOWN

    /* [0x0bb9--0x0bbc] */ m_personal_load     = FORMAT_LE32( cdata+0x0bb9 ); // (weight of just the character's items in 1/10 pounds)
    /* [0x0bbd--0x0bc0] */ m_party_share       = FORMAT_LE32( cdata+0x0bbd ); // (weight of their share of the (half) the party items)
    /* [0x0bc1--0x0bc4] */ m_encumbrance       = FORMAT_LE32( cdata+0x0bc1 ); // (weight of carried objects in 1/10 pounds)
    /* [0x0bc5--0x0bc8] */ m_carry_capacity    = FORMAT_LE32( cdata+0x0bc5 ); // Carry Capacity
    /* [0x0bc9--0x0bcc] */ m_load_category     = FORMAT_LE32( cdata+0x0bc9 ); // extent of penalty the load imparts to combat etc.

    // Spells Known
    /* [0x0bcd--0x0d91] */ // Spells known
    for (int k = 0; k < MAXIMUM_CHARACTER_SPELLS; k++)
    {
        // all spells known by the character have '1' in the LE32 field
        // unknown spells are either 0xffffffff or 0x00000000 - and I can't
        // discern the meaning behind it. It isn't about spells not being
        // available or anything because my Bishop has both values, even on
        // low level spells
        // Seen values of 2 checked for in code too, and don't know what they
        // are for either, except usually treated the same as '1'.
        m_spell[k] = FORMAT_LE32( cdata+0x0bcd + k*4 );
    }
    /* [0x0d95--0x0e5c] */ // UNKNOWN

    /* [0x0e5d--0x0e60] */ m_knownSpellsCount[realm::Fire]   = FORMAT_LE32( cdata+0x0e5d );
    /* [0x0e61--0x0e64] */ m_knownSpellsCount[realm::Water]  = FORMAT_LE32( cdata+0x0e61 );
    /* [0x0e65--0x0e68] */ m_knownSpellsCount[realm::Air]    = FORMAT_LE32( cdata+0x0e65 );
    /* [0x0e69--0x0e6c] */ m_knownSpellsCount[realm::Earth]  = FORMAT_LE32( cdata+0x0e69 );
    /* [0x0e6d--0x0e70] */ m_knownSpellsCount[realm::Mental] = FORMAT_LE32( cdata+0x0e6d );
    /* [0x0e71--0x0e74] */ m_knownSpellsCount[realm::Divine] = FORMAT_LE32( cdata+0x0e71 );

    /* [0x0e75--0x0e7c] */ // UNKNOWN

    /* [0x0e7d--0x0e80] */ m_mpStrongestRealm  = FORMAT_LE32( cdata+0x0e7d );
    /* [0x0e81--0x0e84] */ m_initiative        = FORMAT_LE32( cdata+0x0e81 ); // Initiative
    /* [0x0e85--0x0e88] */ m_acmod_base        = FORMAT_LE32( cdata+0x0e85 ); // Total of all the acmod params excluding penetrate
    /* [0x0e89--0x0e8c] */ m_acmod_average     = FORMAT_LE32( cdata+0x0e89 ); // of Head, Torso, Legs, Hands and Feet

    /* [0x0e8d--0x0e90] */ m_acmod_race        = FORMAT_LE32( cdata+0x0e8d ); // Race (AC Mod)
    /* [0x0e91--0x0e94] */ m_acmod_speed       = FORMAT_LE32( cdata+0x0e91 ); // Speed (AC Mod)
    /* [0x0e95--0x0e98] */ m_acmod_stealth     = FORMAT_LE32( cdata+0x0e95 ); // Stealth Skill (AC Mod)
    /* [0x0e99--0x0e9c] */ m_acmod_shield      = FORMAT_LE32( cdata+0x0e99 ); // Shield (AC Mod)
    /* [0x0e9d--0x0ea0] */ m_acmod_mitems      = FORMAT_LE32( cdata+0x0e9d ); // Magic Items (AC Mod)
    /* [0x0ea1--0x0ea4] */ m_acmod_mspells     = FORMAT_LE32( cdata+0x0ea1 ); // Magic Spells (AC Mod)
    /* [0x0ea5--0x0ea8] */ m_acmod_penetrate   = FORMAT_LE32( cdata+0x0ea5 ); // V Penetration (AC Mod)
    /* [0x0ea9--0x0eac] */ m_acmod_encumbrance = FORMAT_LE32( cdata+0x0ea9 ); // Encumbrance (AC Mod)
    /* [0x0ead--0x0eb0] */ m_acmod_conditions  = FORMAT_LE32( cdata+0x0ead ); // Conditions (AC Mod)
    /* [0x0eb1--0x0eb4] */ m_acmod_fatigue     = FORMAT_LE32( cdata+0x0eb1 ); // Fatigue (AC Mod)
    /* [0x0eb5--0x0eb8] */ m_acmod_defensive   = FORMAT_LE32( cdata+0x0eb5 ); // Defensive Action (AC Mod)
    /* [0x0eb9--0x0ebc] */ m_acmod_reflextion  = FORMAT_LE32( cdata+0x0eb9 ); // Reflextion Skill (AC Mod)
    /* [0x0ebd--0x0ec0] */ m_acmod_unknown     = FORMAT_LE32( cdata+0x0ebd );

    /* [0x0ec1--0x0ec4] */ m_ac_bodypart[ worn::Head  ] = FORMAT_LE32( cdata+0x0ec1 );
    /* [0x0ec5--0x0ec8] */ m_ac_bodypart[ worn::Torso ] = FORMAT_LE32( cdata+0x0ec5 );
    /* [0x0ec9--0x0ecc] */ m_ac_bodypart[ worn::Legs  ] = FORMAT_LE32( cdata+0x0ec9 );
    /* [0x0ecd--0x0ed0] */ m_ac_bodypart[ worn::Hand  ] = FORMAT_LE32( cdata+0x0ecd );
    /* [0x0ed1--0x0ed4] */ m_ac_bodypart[ worn::Feet  ] = FORMAT_LE32( cdata+0x0ed1 );

    /* [0x0ed9--0x0edc] */ m_damage_absorption = FORMAT_LE32( cdata+0x0ed9 ); // Damage Absorption

    // Magic resistances
    /* [0x0edd--0x0ee0] */ m_magic_resistance[realm::Fire][atIdx::Base]      = FORMAT_LE32( cdata+0x0edd );
    /* [0x0ee1--0x0ee4] */ m_magic_resistance[realm::Fire][atIdx::Current]   = FORMAT_LE32( cdata+0x0ee1 );

    /* [0x0eed--0x0ef0] */ m_magic_resistance[realm::Water][atIdx::Base]     = FORMAT_LE32( cdata+0x0eed );
    /* [0x0ef1--0x0ef4] */ m_magic_resistance[realm::Water][atIdx::Current]  = FORMAT_LE32( cdata+0x0ef1 );

    /* [0x0efd--0x0f00] */ m_magic_resistance[realm::Air][atIdx::Base]       = FORMAT_LE32( cdata+0x0efd );
    /* [0x0f01--0x0f04] */ m_magic_resistance[realm::Air][atIdx::Current]    = FORMAT_LE32( cdata+0x0f01 );

    /* [0x0f0d--0x0f10] */ m_magic_resistance[realm::Earth][atIdx::Base]     = FORMAT_LE32( cdata+0x0f0d );
    /* [0x0f11--0x0f14] */ m_magic_resistance[realm::Earth][atIdx::Current]  = FORMAT_LE32( cdata+0x0f11 );

    /* [0x0f1d--0x0f20] */ m_magic_resistance[realm::Mental][atIdx::Base]    = FORMAT_LE32( cdata+0x0f1d );
    /* [0x0f21--0x0f24] */ m_magic_resistance[realm::Mental][atIdx::Current] = FORMAT_LE32( cdata+0x0f21 );

    /* [0x0f2d--0x0f30] */ m_magic_resistance[realm::Divine][atIdx::Base]    = FORMAT_LE32( cdata+0x0f2d );
    /* [0x0f31--0x0f34] */ m_magic_resistance[realm::Divine][atIdx::Current] = FORMAT_LE32( cdata+0x0f31 );

    /* [0x0f35--0x0f5c] */ // UNKNOWN

    // Items Equipped and worn
    /* [0x0f5d--0x0f68] */ loadItem( worn::Head,     cdata+0x0f5d );
    /* [0x0f69--0x0f74] */ loadItem( worn::Misc1,    cdata+0x0f69 );
    /* [0x0f75--0x0f80] */ loadItem( worn::Misc2,    cdata+0x0f75 );
    /* [0x0f81--0x0f8c] */ loadItem( worn::Cloak,    cdata+0x0f81 );
    /* [0x0f8d--0x0f98] */ loadItem( worn::Torso,    cdata+0x0f8d );
    /* [0x0f99--0x0fa4] */ loadItem( worn::Hand,     cdata+0x0f99 );
    /* [0x0fa5--0x0fb0] */ loadItem( worn::Weapon1a, cdata+0x0fa5 );
    /* [0x0fb1--0x0fbc] */ loadItem( worn::Weapon1b, cdata+0x0fb1 );
    /* [0x0fbd--0x0fc8] */ loadItem( worn::Weapon2a, cdata+0x0fbd );
    /* [0x0fc9--0x0fd4] */ loadItem( worn::Weapon2b, cdata+0x0fc9 );
    /* [0x0fd5--0x0fe0] */ loadItem( worn::Legs,     cdata+0x0fd5 );
    /* [0x0fe1--0x0fec] */ loadItem( worn::Feet,     cdata+0x0fe1 );

    /* [0x0fed--0x1028] */ // UNKNOWN

    /* [0x1029--0x1088] */ // Items carried by character
    for (int k=worn::Backpack1; k<=worn::Backpack8; k++)
    {
        loadItem( (character::worn)k, cdata+0x1029 + (k - worn::Backpack1)*12);
    }

    /* [0x1089--0x1148] */ // UNKNOWN

    // Primary Weapon parameters
    /* [0x1149--0x114c] */ m_attack[0].weapon_type  = FORMAT_LE32( cdata+0x1149 ); // Primary weapon type (0 == nothing, 1 == weapon, 2 == shield, 3 == ammo)
    /* [0x114d        ] */ m_attack[0].enabled      = FORMAT_8(    cdata+0x114d );
    /* [0x114e--0x1151] */ m_attack[0].skill1       = formatSkill( cdata+0x114e ); // main skill used
    /* [0x1152--0x1155] */ m_attack[0].skill2       = formatSkill( cdata+0x1152 ); // lesser skill used
    /* [0x1156--0x1159] */ m_attack[0].skill_total  = FORMAT_LE32( cdata+0x1156 );
    /* [0x115a--0x115d] */ m_attack[0].BAR          = FORMAT_LE32( cdata+0x115a );
    /* [0x115e--0x1161] */ m_attack[0].num_attacks  = FORMAT_LE32( cdata+0x115e );
    /* [0x1162--0x1165] */ m_attack[0].max_swings   = FORMAT_LE32( cdata+0x1162 );
    /* [0x1166--0x1169] */ m_attack[0].to_init      = FORMAT_LE32( cdata+0x1166 );
    /* [0x116a--0x116d] */ m_attack[0].MAR          = FORMAT_LE32( cdata+0x116a );
    /* [0x116e--0x1171] */ m_attack[0].to_penetrate = FORMAT_LE32( cdata+0x116e );
    /* [0x1172--0x1175] */ m_attack[0].multiplier   = FORMAT_LE32( cdata+0x1172 );
    /* [0x1176--0x1179] */ m_attack[0].damage_c     = FORMAT_LE16( cdata+0x1176 );// "Kick" base damage - real items get theirs from the item
                           m_attack[0].damage_num_dice   = FORMAT_8( cdata+0x1178 );
                           m_attack[0].damage_dice_sides = FORMAT_8( cdata+0x1179 );
    /* [0x117a--0x117b] */ m_attack[0].attack_modes = FORMAT_LE16( cdata+0x117a ); // "Kick" attack modes
    /* [0x117c--0x117f] */ m_attack[0].unknown51    = FORMAT_LE32( cdata+0x117c );
    /* [0x1180--0x1181] */ m_attack[0].unknown55    = FORMAT_LE16( cdata+0x1180 );
    /* [0x1182        ] */ m_attack[0].unknown57    = FORMAT_8(    cdata+0x1182 );
    /* [0x1183        ] */ m_attack[0].unknown58    = FORMAT_8(    cdata+0x1183 );
    /* [0x1184--0x1187] */ m_attack[0].unknown59    = FORMAT_LE32( cdata+0x1184 );
    /* [0x1188--0x118b] */ m_attack[0].unknown63    = FORMAT_LE32( cdata+0x1188 );
    /* [0x118c--0x11a3] */ // UNKNOWN

    // Offhand Weapon parameters
    /* [0x11a4--0x11a7] */ m_attack[1].weapon_type  = FORMAT_LE32( cdata+0x11a4 ); // Offhand weapon equipped (or blocked by primary)
    /* [0x11a8        ] */ m_attack[1].enabled      = FORMAT_8(    cdata+0x11a8 );
    /* [0x11a9--0x11ac] */ m_attack[1].skill1       = formatSkill( cdata+0x11a9 ); // main skill used
    /* [0x11ad--0x11b4] */ m_attack[1].skill2       = formatSkill( cdata+0x11ad ); // lesser skill used
    /* [0x11b1--0x11b4] */ m_attack[1].skill_total  = FORMAT_LE32( cdata+0x11b1 );
    /* [0x11b5--0x11b8] */ m_attack[1].BAR          = FORMAT_LE32( cdata+0x11b5 );
    /* [0x11b9--0x11bc] */ m_attack[1].num_attacks  = FORMAT_LE32( cdata+0x11b9 );
    /* [0x11bd--0x11c0] */ m_attack[1].max_swings   = FORMAT_LE32( cdata+0x11bd );
    /* [0x11c1--0x11c4] */ m_attack[1].to_init      = FORMAT_LE32( cdata+0x11c1 );
    /* [0x11c5--0x11c8] */ m_attack[1].MAR          = FORMAT_LE32( cdata+0x11c5 );
    /* [0x11c9--0x11cc] */ m_attack[1].to_penetrate = FORMAT_LE32( cdata+0x11c9 );
    /* [0x11cd--0x11d0] */ m_attack[1].multiplier   = FORMAT_LE16( cdata+0x11cd );
    /* [0x11d1--0x11d4] */ m_attack[1].damage_c     = FORMAT_LE16( cdata+0x11d1 ); // "Punch" base damage - real items get theirs from the item
                           m_attack[1].damage_num_dice   = FORMAT_8( cdata+0x11d3 );
                           m_attack[1].damage_dice_sides = FORMAT_8( cdata+0x11d4 );
    /* [0x11d5--0x11d6] */ m_attack[1].attack_modes = FORMAT_LE16( cdata+0x11d5 ); // "Punch" Attack modes
    /* [0x11d7--0x11da] */ m_attack[1].unknown51    = FORMAT_LE32( cdata+0x11d7 );
    /* [0x11db--0x11dc] */ m_attack[1].unknown55    = FORMAT_LE16( cdata+0x11db );
    /* [0x11dd--0x11dd] */ m_attack[1].unknown57    = FORMAT_8(    cdata+0x11dd );
    /* [0x11de--0x11de] */ m_attack[1].unknown58    = FORMAT_8(    cdata+0x11de );
    /* [0x11df--0x11e2] */ m_attack[1].unknown59    = FORMAT_LE32( cdata+0x11df );
    /* [0x11e3--0x11e6] */ m_attack[1].unknown63    = FORMAT_LE32( cdata+0x11e3 );
    /* [0x11e7--0x11fe] */ // UNKNOWN

    /* [0x11ff--0x12b4] */ // UNKNOWN

    /* [0x12b5        ] */ m_dual_wielding      = FORMAT_8( cdata+0x12b5 ); // Dual wielding currently

    /* [0x12b6--0x169d] */ // UNKNOWN
    /* [0x169e--0x16a1] */ m_fatigue_category   = FORMAT_LE32( cdata+0x169e );

    /* [0x16a2--0x1708] */ // Penalties (negative bonuses) imposed by current character conditions -- gets added onto Current
    /* [0x1709--0x176f] */ // Bonuses from items equipped -- gets added onto Current

    /* [0x1770        ] */ m_bonus.to_init        = FORMAT_8( cdata+0x1770 );
    /* [0x1771        ] */ m_bonus.MAR            = FORMAT_8( cdata+0x1771 );
    /* [0x1772        ] */ m_bonus.to_penetrate   = FORMAT_8( cdata+0x1772 );
    /* [0x1773        ] */ m_bonus.multiplier     = FORMAT_8( cdata+0x1773 );
    /* [0x1774        ] */ m_bonus.mspells        = FORMAT_8( cdata+0x1774 );
    /* [0x1775        ] */ m_bonus.vpenetrate     = FORMAT_8( cdata+0x1775 );
    /* [0x1776        ] */ m_bonus.absorption     = FORMAT_8( cdata+0x1776 );
    /* [0x1777        ] */ m_bonus.allmagic_resist= FORMAT_8( cdata+0x1777 );
    /* [0x1778        ] */ m_bonus.unknown_0x1778 = FORMAT_8( cdata+0x1778 );
    /* [0x1779        ] */ m_bonus.hp_regen       = FORMAT_8( cdata+0x1779 );
    /* [0x177a        ] */ m_bonus.stamina_regen  = FORMAT_8( cdata+0x177a );
    /* [0x177b        ] */ m_bonus.sp_regen       = FORMAT_8( cdata+0x177b );

    // [0x177c--0x0000] Attribute bonuses
    //   [0x177c]    Strength
    //   [0x177d]    Intelligence
    //   [0x177e]    Piety
    //   [0x177f]    Vitality
    //   [0x1780]    Dexterity
    //   [0x1781]    Speed
    //   [0x1782]    Senses
    for (int k=0; k<attribute::ATTRIBUTE_SIZE; k++)
    {
        m_bonus.attributes[ k ] = cdata[0x177c + k ];
    }

    // [0x1783--0x17ab] Skill bonuses
    //   [0x1783]    Sword
    //   [0x1784]    Axe
    //   [0x1785]    Polearm
    //   [0x1786]    Mace_Flail
    //   [0x1787]    Dagger
    //   [0x1788]    Staff_Wand
    //   [0x1789]    Shield
    //   [0x178a]    ModernWeapon
    //   [0x178b]    Bow
    //   [0x178c]    Throwing_Sling
    //   [0x178d]    Locks_Traps
    //   [0x178e]    Stealth
    //   [0x178f]    Music
    //   [0x1790]    Pickpocket
    //   [0x1791]    MartialArts
    //   [0x1792]    Scouting
    //   [0x1793]    CloseCombat
    //   [0x1794]    RangedCombat
    //   [0x1795]    DualWeapons
    //   [0x1796]    CriticalStrike
    //   [0x1797]    Artifacts
    //   [0x1798]    Mythology
    //   [0x1799]    Communication
    //   [0x179a]    Engineering
    //   [0x179b]    Wizardry
    //   [0x179c]    Divinity
    //   [0x179d]    Alchemy
    //   [0x179e]    Psionics
    //   [0x179f]    FireMagic
    //   [0x17a0]    WaterMagic
    //   [0x17a1]    AirMagic
    //   [0x17a2]    EarthMagic
    //   [0x17a3]    MentalMagic
    //   [0x17a4]    DivineMagic
    //   [0x17a5]    PowerStrike
    //   [0x17a6]    PowerCast
    //   [0x17a7]    IronWill
    //   [0x17a8]    IronSkin
    //   [0x17a9]    Reflexion
    //   [0x17aa]    SnakeSpeed
    //   [0x17ab]    EagleEye
    for (int k=0; k<skill::SKILL_SIZE; k++)
    {
        m_bonus.skills[ k ] = cdata[0x1783 + k ];
    }

    /* [0x17ac        ] */ m_bonus.magic_resistance[realm::Fire  ] = FORMAT_8( cdata+0x17ac );
    /* [0x17ad        ] */ m_bonus.magic_resistance[realm::Water ] = FORMAT_8( cdata+0x17ad );
    /* [0x17ae        ] */ m_bonus.magic_resistance[realm::Air   ] = FORMAT_8( cdata+0x17ae );
    /* [0x17af        ] */ m_bonus.magic_resistance[realm::Earth ] = FORMAT_8( cdata+0x17af );
    /* [0x17b0        ] */ m_bonus.magic_resistance[realm::Mental] = FORMAT_8( cdata+0x17b0 );
    /* [0x17b1        ] */ m_bonus.magic_resistance[realm::Divine] = FORMAT_8( cdata+0x17b1 );

    /* [0x17b2        ] */ m_bonus.unknown_0x17b2                  = FORMAT_8( cdata+0x17b2 );
    /* [0x17b3        ] */ m_bonus.unknown_0x17b3                  = FORMAT_8( cdata+0x17b3 );
    /* [0x17b4        ] */ m_bonus.unknown_0x17b4                  = FORMAT_8( cdata+0x17b4 );
    /* [0x17b5        ] */ m_bonus.incapacitated                   = FORMAT_8( cdata+0x17b5 );

    // [0x17b6--0x17ba] UNKNOWN

    /* [0x17bb        ] */ m_bonus.conditions                      = FORMAT_8( cdata+0x17bb );

    // [0x17bc--0x17da] UNKNOWN

    // Portal Location
    /* [0x17db--0x17de] */ m_portal_x = FORMAT_FLOAT( cdata+0x17db ) / 500.0;
    /* [0x17df--0x17e2] */ m_portal_y = FORMAT_FLOAT( cdata+0x17df ) / 500.0;
    /* [0x17e3--0x17e6] */ m_portal_z = FORMAT_FLOAT( cdata+0x17e3 ) / 500.0;
    /* [0x17e7--0x17ea] */ m_portal_pitch_or_direction_dunno = FORMAT_FLOAT( cdata+0x17e7 );
    // 5 32bit 0s
    /* [0x17ff--0x1802] */ m_portal_direction_or_pitch_dunno = FORMAT_FLOAT( cdata+0x17ff );
    // 5 32bit 0s
    /* [0x1813--0x1816] */ m_portal_map = FORMAT_LE32( cdata+0x1813 );

    // [0x1817--0x1829] UNKNOWN
    // looks like might be a group of 17 values uint8_t 0x181f[2][17];
    /* [0x1830        ] */ m_unknown_0x1830                                        = FORMAT_8( cdata+0x1830 );
    // [0x1831--0x1861] UNKNOWN

    const quint8 *pdata = (const quint8 *) cx.constData();

    /* [0x0000        ] */ m_charslot_inuse = pdata[0];

    /* [0x00f1--0x00f4] */ m_color_idx = FORMAT_LE32(pdata+0xf1);

    // hmmm... according to a debug comment offset 0x25 is the real "ID",
    // so that makes this something else. offset 0x21 is called "char" - which looks like m_charIdx.
    // and 0x1d is called "Target Type" (or maybe just "Type")
    /* [0x00fa--0x00fd] */ m_rpc_id    = FORMAT_LE32(pdata+0xfa);
}

int character::getCondition(condition c) const
{
    return m_conditions[ c ];
}

void character::setCondition(condition c, int duration)
{
    m_conditions[ c ] = duration;
    if (duration == 0)
    {
        m_conditions_active[ c ] = false;
    }
    else
    {
        m_conditions_active[ c ] = true;
    }

    // reset the main status condition for the most serious (highest index)
    // sickness
    m_main_status_condition = condition::Normal; // default
    for (int k=CONDITION_SIZE-1; k >= 0; k--)
    {
        if (m_conditions_active[ k ])
        {
            m_main_status_condition = static_cast<condition>( k );
            break;
        }
    }
}

void character::setConditionActive(condition c, bool on)
{
    m_conditions_active[ c ] = on;
}

int character::getPoisonStrength() const
{
    return m_poison_strength;
}

int character::getColorIdx() const
{
    return m_color_idx;
}

void character::setColorIdx(int idx)
{
    m_color_idx = idx;
}

void character::recomputeEverything()
{
    if (m_in_party)
    {
        recomputeBonus();

        if (m_combinedLevel > 50)
            m_combinedLevel = 50;

        QMutableMapIterator<character::profession, int> i(m_currentLevels);
        while (i.hasNext())
        {
            i.next();
            if (i.value() > 50)
                i.setValue( 50 );
        }

        resetControllingAttribs();
        recomputeHp();
        recomputeStamina();

        // What about former spell caster that is now only a Fighter, Rogue, Bard or Gadgeteer?
        if (isPureCaster() || isHybridCaster())
            recomputeManaPoints();

        recomputeAcmod();
        recomputeRecoveryRates();
        recomputeDamageAbsorption();
        recomputeMagicResistance();
        recomputeCarryCapacity();
        recomputePersonalLoad();

        // These 2 will get done after the party load is calculated
        //recomputeInitiative();
        //recomputeAttack();
    }
}

qint32 character::getAC_Base() const
{
    return m_acmod_base;
}

qint32 character::getAC_Average() const
{
    return m_acmod_average;
}

qint32 character::getACMod_Race() const
{
    return m_acmod_race;
}

qint32 character::getACMod_Speed() const
{
    return m_acmod_speed;
}

qint32 character::getACMod_Stealth() const
{
    return m_acmod_stealth;
}

qint32 character::getACMod_Shield() const
{
    return m_acmod_shield;
}

qint32 character::getACMod_MagicItems() const
{
    return m_acmod_mitems;
}

qint32 character::getACMod_MagicSpells() const
{
    return m_acmod_mspells;
}

qint32 character::getACMod_Penetration() const
{
    return m_acmod_penetrate;
}

qint32 character::getACMod_Encumbrance() const
{
    return m_acmod_encumbrance;
}

qint32 character::getACMod_Conditions() const
{
    return m_acmod_conditions;
}

qint32 character::getACMod_Fatigue() const
{
    return m_acmod_fatigue;
}

qint32 character::getACMod_Defensive() const
{
    return m_acmod_defensive;
}

qint32 character::getACMod_Reflextion() const
{
    return m_acmod_reflextion;
}

qint32 character::getDamageAbsorption() const
{
    return m_damage_absorption;
}

int character::getToInit(bool primary) const
{
    if (primary)
    {
        return m_attack[0].to_init + m_bonus.to_init;
    }
    // secondary

    return m_attack[1].to_init + m_bonus.to_init;
}

int character::getToPenetrate(bool primary) const
{
    if (primary)
    {
        return m_bonus.to_penetrate + m_attack[0].to_penetrate;
    }
    // secondary

    return m_bonus.to_penetrate + m_attack[1].to_penetrate;
}

bool character::getDamage(bool primary, quint16 *min_damage, quint16 *max_damage, int *percentage) const
{
    const item i_primary   = getItem( character::worn::Weapon1a );
    const item i_secondary = getItem( character::worn::Weapon1b );
    if (i_primary.isNull() && i_secondary.isNull())
    {
        if (primary)
        {
            // Kick
            if (min_damage) *min_damage = m_attack[0].damage_c + m_attack[0].damage_num_dice;
            if (max_damage) *max_damage = m_attack[0].damage_c + m_attack[0].damage_num_dice * m_attack[0].damage_dice_sides;
            if (percentage) *percentage = m_attack[0].multiplier + m_bonus.multiplier;
        }
        else
        {
            // Punch
            if (min_damage) *min_damage = m_attack[1].damage_c + m_attack[1].damage_num_dice;
            if (max_damage) *max_damage = m_attack[1].damage_c + m_attack[1].damage_num_dice * m_attack[1].damage_dice_sides;
            if (percentage) *percentage = m_attack[1].multiplier + m_bonus.multiplier;
        }
        return true;
    }
    else
    {
        // at least one of the weapons is equipped
        if (min_damage) *min_damage = 0;
        if (max_damage) *max_damage = 0;
        if (percentage) *percentage = 0;

        if (primary)
        {
            if (! i_primary.isNull())
            {
                if (i_primary.damageComesFromAmmunition())
                {
                    if (! i_secondary.isNull())
                    {
                        i_secondary.getDamage( min_damage, max_damage, NULL );
                        if (percentage) *percentage = m_attack[0].multiplier + m_bonus.multiplier;
                    }
                }
                else
                {
                    i_primary.getDamage( min_damage, max_damage, NULL );
                    if (percentage) *percentage = m_attack[0].multiplier + m_bonus.multiplier;
                }
                return true;
            }
        }
        else
        {
            if (! i_secondary.isNull())
            {
                i_secondary.getDamage( min_damage, max_damage, NULL );
                if (percentage) *percentage = m_attack[1].multiplier + m_bonus.multiplier;
                return true;
            }
        }
    }
    return false;
}

QString character::getDamageString(bool primary, bool summarised) const
{
    quint16 dmin = 0;
    quint16 dmax = 0;
    int     dpercent = 0;

    getDamage( primary, &dmin, &dmax, &dpercent );

    if (summarised)
    {
        // The +50 causes a true round to occur, not just a floor()
        dmin = ((dpercent + 100) * dmin + 50) / 100;
        if (dmin < 1)
            dmin = 1;

        dmax = ((dpercent + 100) * dmax + 50) / 100;
        if (dmax < 1)
            dmax = 1;

        return QString("%1-%2").arg(dmin).arg(dmax);
    }

    return QString("%1 %2, %3 %4, %5 %6").arg(::getStringTable()->getString( StringList::BaseMin )).arg( dmin )
                                         .arg(::getStringTable()->getString( StringList::BaseMax )).arg( dmax )
                                         .arg(::getStringTable()->getString( StringList::Mod ))
                                         .arg( QString::asprintf("%+d%%", dpercent) );
}

void character::getAttackRating(bool primary, int *baseAR, int *modAR) const
{
    if (baseAR)
    {
        if (primary)
            *baseAR = m_attack[0].BAR;
        else
            *baseAR = m_attack[1].BAR;

        if (*baseAR >= 0)
            *baseAR += 2;
        else
            *baseAR -= 2;
        *baseAR /= 5;
    }
    if (modAR)
    {
        if (primary)
            *modAR = m_bonus.MAR + m_attack[0].MAR;
        else
            *modAR = m_bonus.MAR + m_attack[1].MAR;
    }
}

QString character::getAttackRatingString(bool primary, bool summarised) const
{
    int BaseAR = 0;
    int ModAR  = 0;

    getAttackRating( primary, &BaseAR, &ModAR );

    if (summarised)
    {
        return QString::number( BaseAR + ModAR );
    }

    return QString("%1 %2, %3 %4").arg(::getStringTable()->getString( StringList::Base )).arg( BaseAR )
                                  .arg(::getStringTable()->getString( StringList::Mod ))
                                  .arg( QString::asprintf("%+d", ModAR) );
}

int character::getNumAttacks(bool primary) const
{
    if (primary)
        return m_attack[0].num_attacks;

    return m_attack[1].num_attacks;
}

int character::getMaxSwings(bool primary) const
{
    if (primary)
        return m_attack[0].max_swings;

    return m_attack[1].max_swings;
}

QString character::getName() const
{
    return m_charName;
}

void character::setName(const QString &name)
{
    m_charName = name;
}

QString character::getFullName() const
{
    return m_fullName;
}

void character::setFullName(const QString &name)
{
    m_fullName = name;
}

character::profession character::getProfession() const
{
    return m_profession;
}

QString character::getProfessionString()
{
    return getProfessionString( m_profession );
}

QString character::getProfessionString( character::profession p )
{
    QMetaEnum metaProf = QMetaEnum::fromType<character::profession>();

    for (int k=0; k<metaProf.keyCount(); k++)
    {
        if (metaProf.value(k) == p)
        {
            return ::getStringTable()->getString( StringList::LISTProfessions + k );
        }
    }

    return "";
}

void character::setProfession(character::profession p)
{
    m_profession = p;

    recomputeMagicResistance();
}

character::race character::getRace() const
{
    return m_race;
}

QString character::getRaceString()
{
    for (int k=0; k<m_metaRace.keyCount(); k++)
    {
        if (m_metaRace.value(k) == m_race)
        {
            return ::getStringTable()->getString( StringList::LISTRaces + k );
        }
    }

    return "";
}

void character::setRace(character::race r)
{
    m_race = r;

    recomputeMagicResistance();
    recomputeCarryCapacity();
}

character::gender character::getGender() const
{
    return m_gender;
}

QString character::getGenderString()
{
    for (int k=0; k<m_metaGender.keyCount(); k++)
    {
        if (m_metaGender.value(k) == m_gender)
        {
            return ::getStringTable()->getString( StringList::LISTGenders + k );
        }
    }

    return "";
}

void character::setGender(character::gender g)
{
    m_gender = g;
}

int character::getCurrentLevel() const
{
    return m_currentLevels[ m_profession ];
}

QString character::getCurrentLevelString() const
{
    int prof_idx = 0;

    int string_ids[] =
    {
        // Fighter:
        StringList::Novice, StringList::Journeyman, StringList::Warrior, StringList::Marauder,
        StringList::Gladiator, StringList::Swordsman, StringList::Warlord, StringList::Conqueror,
        // Lord:
        StringList::Novice, StringList::Squire, StringList::Gallant, StringList::Knight,
        StringList::Chevalier, StringList::Paladin, StringList::Crusader, StringList::Monarch,
        // Valkyrie:
        StringList::Novice, StringList::Lancer, StringList::Warrior, StringList::Cavalier,
        StringList::Chevalier, StringList::Champion, StringList::Heroine, StringList::Olympian,
        // Ranger:
        StringList::Novice, StringList::Woodsman, StringList::Scout, StringList::Archer,
        StringList::Pathfinder, StringList::Weaponeer, StringList::Outrider, StringList::RangerLord,
        // Samurai:
        StringList::Novice, StringList::Bladesman, StringList::Shugenja, StringList::Hatamoto,
        StringList::DaishoMaster, StringList::Daimyo, StringList::Warlord, StringList::Shogun,
        // Ninja:
        StringList::Novice, StringList::Genin, StringList::Executioner, StringList::Assassin,
        StringList::Chunin, StringList::Master, StringList::Jonin, StringList::Grandfather,
        // Monk:
        StringList::Novice, StringList::Acolyte, StringList::Seeker, StringList::Disciple,
        StringList::Apostle, StringList::Master, StringList::Immaculate, StringList::Grandmaster,
        // Rogue:
        StringList::Novice, StringList::Thief, StringList::Trickster, StringList::Highwayman,
        StringList::Bushwhacker, StringList::Pirate, StringList::MasterOfShadows, StringList::Guildmaster,
        // Gadgeteer:
        StringList::Novice, StringList::Tinker, StringList::Machinist, StringList::Craftsman,
        StringList::Toolmaster, StringList::Inventor, StringList::Creator, StringList::Genius,
        // Bard:
        StringList::Novice, StringList::Minstrel, StringList::Cantor, StringList::Sonneteer,
        StringList::Troubadour, StringList::Poet, StringList::MasterOfLutes, StringList::Muse,
        // Priest:
        StringList::Novice, StringList::Acolyte, StringList::Healer, StringList::Curate,
        StringList::Priest, StringList::HighPriest, StringList::Patriarch, StringList::Saint,
        // Alchemist:
        StringList::Novice, StringList::Herbalist, StringList::Physician, StringList::Adept,
        StringList::Shaman, StringList::Evocator, StringList::MasterOfElixirs, StringList::Enchanter,
        // Bishop:
        StringList::Novice, StringList::Friar, StringList::Vicar, StringList::Canon,
        StringList::Magistrate, StringList::Diocesan, StringList::Cardinal, StringList::Pontiff,
        // Psionic:
        StringList::Novice, StringList::Psychic, StringList::Soothsayer, StringList::Visionist,
        StringList::Illusionist, StringList::Mystic, StringList::Oracle, StringList::Prophet,
        // Mage:
        StringList::Novice, StringList::Magician, StringList::Conjurer, StringList::Warlock,
        StringList::Sorcerer, StringList::Necromancer, StringList::Wizard, StringList::Magus
    };

    for (int k=0; k<m_metaProf.keyCount(); k++)
    {
        if (m_metaProf.value(k) == m_profession)
        {
            prof_idx = k * 8;
            break;
        }
    }

    // Wizardry 8 has inconsistent behaviour with respect to level strings
    // When you first change class it shows string based on overall, but then on
    // first level change reasserts based on current profession.
    // I've chosen just to use the current profession. Wizardry may alter this at
    // some point if it doesn't like it, but it's just a text string on screen AFAICT.

    if (m_currentLevels[ m_profession ] == 1)
        return ::getStringTable()->getString( string_ids[prof_idx + 0] );
    else if (m_currentLevels[ m_profession ] < 4)
        return ::getStringTable()->getString( string_ids[prof_idx + 1] );
    else if (m_currentLevels[ m_profession ] < 7)
        return ::getStringTable()->getString( string_ids[prof_idx + 2] );
    else if (m_currentLevels[ m_profession ] < 11)
        return ::getStringTable()->getString( string_ids[prof_idx + 3] );
    else if (m_currentLevels[ m_profession ] < 16)
        return ::getStringTable()->getString( string_ids[prof_idx + 4] );
    else if (m_currentLevels[ m_profession ] < 22)
        return ::getStringTable()->getString( string_ids[prof_idx + 5] );
    else if (m_currentLevels[ m_profession ] < 28)
        return ::getStringTable()->getString( string_ids[prof_idx + 6] );
    else
        return ::getStringTable()->getString( string_ids[prof_idx + 7] );
}

int character::getInitiative() const
{
    return m_initiative;
}

int character::getKills() const
{
    return m_kills;
}

int character::getDeaths() const
{
    return m_deaths;
}

int character::getPortraitIndex() const
{
    return m_portraitIndex;
}

void character::setPortraitIndex(int idx)
{
    m_portraitIndex = idx;
}

character::personality character::getPersonality() const
{
    return m_personality;
}

void character::setPersonality(character::personality pers)
{
    m_personality = pers;
}

int character::getVoice() const
{
    return m_voice;
}

void character::setVoice(int voice)
{
    m_voice = voice;
}

int character::getAttribute(attribute at, atIdx idx) const
{
    if (idx < atIdx::ATIDX_SIZE)
        return m_attribs[ at ].value[ idx ];
    return 0;
}

void character::setAttribute(attribute at, int value)
{
    m_attribs[ at ].value[ atIdx::Base ] = value;
    // _Everything_ actually needs to be reset, but we
    // only do this one because the screen that adjusts it
    // only has the magic resistances & primary attribs visible at the time.
    resetControllingAttribs();
    recomputeMagicResistance();
}

int character::getSkill(skill sk, atIdx idx) const
{
    if (idx < atIdx::ATIDX_SIZE)
        return m_skills[ sk ].value[ idx ];
    return 0;
}

void character::setSkill(skill sk, int value)
{
    m_skills[ sk ].value[ atIdx::Base ] = value;
    // _Everything_ actually needs to be reset, but we
    // only do this one because the screen that adjusts it
    // only has the skills visible at the time.
    resetControllingAttribs();
}

int character::getHp(atIdx idx)
{
    return m_hp[idx];
}

int character::getStamina(atIdx idx)
{
    return m_stamina[idx];
}

int character::getMp(realm r, atIdx idx)
{
    if (r == realm::REALM_SIZE)
    {
        return m_mp[ realm::Fire   ][idx] +
               m_mp[ realm::Water  ][idx] +
               m_mp[ realm::Air    ][idx] +
               m_mp[ realm::Earth  ][idx] +
               m_mp[ realm::Mental ][idx] +
               m_mp[ realm::Divine ][idx];
    }
    return m_mp[r][idx];
}

int character::getMagicResistance(realm r, atIdx idx)
{
    return m_magic_resistance[r][idx];
}

quint32 character::getXp() const
{
    return m_xp;
}

quint32 character::getXpNeeded() const
{
    return m_xp_needed;
}

quint32 character::getXpLastLevelNeeded() const
{
    return m_xp_last_needed;
}

void character::setXp(quint32 xp)
{
    m_xp = xp;
}

void character::setXpNeeded(quint32 xp)
{
    m_xp_needed = xp;
}

void character::setXpLastLevelNeeded(quint32 xp)
{
    m_xp_last_needed = xp;
}

item character::getItem(worn idx) const
{
    if (m_item[idx])
        return *m_item[idx];
    return item();
}

void character::deleteItem(worn idx)
{
    if (m_item[idx])
    {
        delete m_item[idx];
        m_item[idx] = NULL;

        if ((idx == worn::Weapon1a) ||
            (idx == worn::Weapon1b))
        {
            recomputeAttack();
        }
    }
}

void character::setItem(worn idx, item i)
{
    Q_ASSERT( m_item[idx] == NULL );

    // Don't allow null items to be assigned to the character, we keep
    // empty item slots as NULL pointers in here.
    if (! i.isNull())
    {
        m_item[idx] = new item(i);

        if ((idx == worn::Weapon1a) ||
            (idx == worn::Weapon1b))
        {
            recomputeAttack();
        }
    }
}

void character::recomputeInitiative()
{
    m_initiative = m_bonus.to_init +
                   m_attribs[ attribute::Senses ].value[ atIdx::Current ] / 5 +
                   m_attribs[ attribute::Speed ].value[ atIdx::Current ] / 5 +
                   (m_combinedLevel + 1) / 2 -
                   10;

    if ( m_skills[ skill::SnakeSpeed ].enabled )
    {
        m_initiative += m_skills[ skill::SnakeSpeed ].value[ atIdx::Current ] / 10 + 1;
    }
    int load_penalty = 0;
    switch (m_load_category)
    {
        case 0:
            load_penalty = 0;
            break;
        case 1:
            load_penalty = -1;
            break;
        case 2:
            load_penalty = -2;
            break;
        case 3:
            load_penalty = -4;
            break;
        case 4:
            load_penalty = -8;
            break;
        default:
            qWarning() << "Invalid load category";
            break;
    }
    m_initiative += load_penalty;
}

void character::recomputeAttack()
{
    for (int k=0; k<2; k++)
    {
        item  *currentItem   = m_item[ (k == 0 ) ? worn::Weapon1a : worn::Weapon1b ];
        item  *companionItem = m_item[ (k != 0 ) ? worn::Weapon1a : worn::Weapon1b ];

        m_attack[ k ].enabled = 1;

        if ( !currentItem )
        {
            m_attack[ k ].weapon_type = -1;
            m_attack[ k ].skill1 = skill::MartialArts;
            m_attack[ k ].skill2 = skill::CloseCombat;
        }
        else
        {
            switch (currentItem->getType())
            {
                case item::type::ShortWeapon:
                case item::type::ExtendedWeapon:
                case item::type::RangedWeapon:
                case item::type::ThrownWeapon:
                    m_attack[ k ].weapon_type = 1;
                    if ((k == 1) && companionItem && companionItem->isOmnigun())
                        m_attack[ k ].weapon_type = 3;
                    break;

                case item::type::Ammunition:
                    m_attack[ k ].weapon_type = 3;
                    break;

                case item::type::Shield:
                    m_attack[ k ].weapon_type = 2;
                    break;

                default: // someone has shoved an inappropriate type of object in this slot
                    m_attack[ k ].weapon_type = -1;
                    break;
            }

            m_attack[ k ].skill1 = currentItem->getSkillUsed();

            switch (m_attack[ k ].skill1)
            {
                case skill::Sword:
                case skill::Axe:
                case skill::Polearm:
                case skill::Mace_Flail:
                case skill::Dagger:
                case skill::Staff_Wand:
                case skill::Shield:
                case skill::MartialArts:
                    m_attack[ k ].skill2 = skill::CloseCombat;
                    break;

                case skill::ModernWeapon:
                case skill::Bow:
                case skill::Throwing_Sling:
                    m_attack[ k ].skill2 = skill::RangedCombat;

                    // The combined effect of these ifs is that you have to manually load the
                    // weapon yourself AND ensure additional rounds are available in the offhand
                    // or else the weapon gets disabled on you - this effectively means it won't
                    // load itself automatically, and you can get into a situation where the
                    // final shots in the breach can't be used when the rest of the ammunition is
                    // exhausted. None of which sounds right.
                    if (currentItem->damageComesFromAmmunition())
                    {
                        if ( ! companionItem || ! currentItem->compatibleAmmunition( *companionItem ) )
                            m_attack[ k ].enabled = 0;
                    }
                    if (currentItem->hasShots() && (currentItem->getCharges() == 0) )
                    {
                        m_attack[ k ].enabled = 0;
                    }
                    break;

                default:
                    m_attack[ k ].skill2 = skill::SKILL_NONE;
                    m_attack[ k ].enabled = 0;
                    break;
            }
        }
    }

    if ( m_attack[0].skill2 != m_attack[1].skill2 )
        m_attack[1].enabled = 0;

    if ((m_attack[1].weapon_type == 2) || (m_attack[1].weapon_type == 3))
        m_attack[1].enabled = 0;

    if ( m_item[worn::Weapon1a] && (m_item[worn::Weapon1a]->isOmnigun()))
        m_attack[1].enabled = 0;

    if ((m_attack[1].weapon_type == 0) && (m_attack[0].weapon_type > 0) )
        m_attack[1].enabled = 0;

    m_dual_wielding = (m_attack[0].weapon_type == 1) &&
                      m_attack[0].enabled &&
                      (m_attack[1].weapon_type == 1) &&
                      m_attack[1].enabled;

    int prof_BAR = 0;
    for (int j=0; j<m_metaProf.keyCount(); j++)
    {
        profession prof = static_cast<profession>(m_metaProf.value(j));
        int level = m_currentLevels.value( prof, 0 );

        switch (prof)
        {
            case profession::Fighter:
            case profession::Lord:
            case profession::Valkyrie:
            case profession::Ranger:
            case profession::Samurai:
            case profession::Ninja:
            case profession::Monk:
                prof_BAR += 4 * level;
                break;

            case profession::Rogue:
            case profession::Gadgeteer:
            case profession::Bard:
                prof_BAR += 3 * level;
                break;

            case profession::Priest:
            case profession::Alchemist:
            case profession::Bishop:
            case profession::Psionic:
            case profession::Mage:
                prof_BAR += 2 * level;
                break;
        }
    }

    for (int k=0; k < 2; k++ )
    {
        int    currentIdx    =  k;
        int    companionIdx  = !k;
        item  *currentItem   = m_item[ (k == 0 ) ? worn::Weapon1a : worn::Weapon1b ];
        item  *companionItem = m_item[ (k != 0 ) ? worn::Weapon1a : worn::Weapon1b ];

        if ( m_attack[ currentIdx ].enabled )
        {
            int avg_skill = (m_skills[ m_attack[ currentIdx ].skill2 ].value[ atIdx::Current ] +
                             m_skills[ m_attack[ currentIdx ].skill1 ].value[ atIdx::Current ] * 2) * 2 / 3;
            int divisor   = 20;

            if ((m_attack[ companionIdx ].weapon_type == 1) && m_attack[ companionIdx ].enabled )
            {
                avg_skill += m_skills [ m_attack[ companionIdx ].skill1 ].value[ atIdx::Current ] / 2;
                divisor   += 5;
            }
            if (m_dual_wielding)
            {
                avg_skill += m_skills[ skill::DualWeapons ].value[ atIdx::Current ];
                divisor   += 10;
            }
            m_attack[ currentIdx ].skill_total = 10 * avg_skill / divisor;

            int load_penalty = 0;
            switch (m_load_category)
            {
                case 0:
                    load_penalty = 0;
                    break;
                case 1:
                    load_penalty = -1;
                    break;
                case 2:
                    load_penalty = -2;
                    break;
                case 3:
                    load_penalty = -4;
                    break;
                case 4:
                    load_penalty = -8;
                    break;
                default:
                    qWarning() << "Invalid load category";
                    break;
            }
            if ( m_attack[ currentIdx ].skill1 == skill::ModernWeapon )
                load_penalty /= 2;

            int dual_wield_penalty = 0;
            if (m_dual_wielding)
            {
                dual_wield_penalty = -10 - (100 - m_skills[ skill::DualWeapons ].value[ atIdx::Current ]) / 4;
                if (k != 0) // offhand weapon penalised more than the main
                    dual_wield_penalty -= 10;
            }
            m_attack[ currentIdx ].BAR = (prof_BAR +
                                          m_attribs[ attribute::Dexterity ].value[ atIdx::Current ] / 2 +
                                          dual_wield_penalty +
                                          m_attack[ currentIdx ].skill_total * 2) / 3 + 60;

            if (m_in_party)
            {
                if ( m_race == character::race::Android )
                {
                    // An unrepaired android suffers an attack penalty - BAR halved

                    // FIXME: Don't have the support routines for testing FACT_RFS81_HAS_BEEN_FIXED
                    //        nor for validating the RPC id
#if 0
                    v21 = sub_50B800( m_rpc_id );
                    if ( v21 )
                    {
                        // Think this might be trying to detect if it is the unrepaired android (RFS81A)
                        // and if so halving its BAR compared to the repaired one (RFS81B)
                        if ( *(_BYTE *)(v21 + 46) == 32 && !sub_506280(68) ) // FACT 68 = FACT_RFS81_HAS_BEEN_FIXED
                            m_attack[ currentIdx ].BAR /= 2;
                    }
#endif
                }
            }
            int attack_score = (m_attack[ currentIdx ].skill_total +
                                load_penalty * 15 +
                                prof_BAR +
                                dual_wield_penalty +
                                ((unsigned int)(m_attribs[ attribute::Dexterity ].value[ atIdx::Current ] + m_attribs[ attribute::Speed ].value[ atIdx::Current ]) / 2)) / 3;

            m_attack[ currentIdx ].num_attacks = 1;
            if ( k )
            {
                if ( attack_score >= 75 )
                    m_attack[ currentIdx ].num_attacks = 2;
            }
            else
            {
                if ( attack_score >= 50 )
                {
                    m_attack[ currentIdx ].num_attacks = 2;
                    if ( attack_score >= 100 )
                        m_attack[ currentIdx ].num_attacks = 3;
                }
            }

            m_attack[ currentIdx ].to_penetrate = 0;
            m_attack[ currentIdx ].multiplier   = 0;

            if ( currentItem )
            {
                m_attack[ currentIdx ].to_init = currentItem->getInitiative();
                m_attack[ currentIdx ].MAR = currentItem->getToHit();
                if ( m_attack[ companionIdx].weapon_type == 3 )
                {
                    if ( companionItem )
                        m_attack[ currentIdx ].MAR += companionItem->getToHit();
                }
                if ( currentItem->damageComesFromAmmunition())
                {
                    int percentage;

                    currentItem->getDamage(NULL, NULL, &percentage);
                    m_attack[ currentIdx ].multiplier = percentage;
                }
            }
            else // unarmed = kick and punch
            {
                m_attack[ currentIdx ].to_init = m_attack[ currentIdx ].skill_total / 10;
                m_attack[ currentIdx ].MAR          = 0;
                m_attack[ currentIdx ].multiplier   = 0;

                m_attack[ currentIdx ].attack_modes = item::attack::Punch;
                if ( m_skills[ m_attack[ currentIdx ].skill1 ].value[ atIdx::Current ] >= 5 )
                    m_attack[ currentIdx ].attack_modes |= item::attack::Kick;

                switch (m_skills[ m_attack[ currentIdx ].skill1 ].value[ atIdx::Current ] / 11)
                {
                    case 0:
                        m_attack[ currentIdx ].damage_c = 0;
                        m_attack[ currentIdx ].damage_num_dice = 1;
                        m_attack[ currentIdx ].damage_dice_sides = 2;
                        break;
                    case 1:
                        m_attack[ currentIdx ].damage_c = 0;
                        m_attack[ currentIdx ].damage_num_dice = 1;
                        m_attack[ currentIdx ].damage_dice_sides = 3;
                        break;
                    case 2:
                        m_attack[ currentIdx ].damage_c = 0;
                        m_attack[ currentIdx ].damage_num_dice = 2;
                        m_attack[ currentIdx ].damage_dice_sides = 2;
                        break;
                    case 3:
                        m_attack[ currentIdx ].damage_c = 0;
                        m_attack[ currentIdx ].damage_num_dice = 2;
                        m_attack[ currentIdx ].damage_dice_sides = 3;
                        break;
                    case 4:
                        m_attack[ currentIdx ].damage_c = 0;
                        m_attack[ currentIdx ].damage_num_dice = 2;
                        m_attack[ currentIdx ].damage_dice_sides = 4;
                        break;
                    case 5:
                        m_attack[ currentIdx ].damage_c = 0;
                        m_attack[ currentIdx ].damage_num_dice = 3;
                        m_attack[ currentIdx ].damage_dice_sides = 3;
                        break;
                    case 6:
                        m_attack[ currentIdx ].damage_c = 1;
                        m_attack[ currentIdx ].damage_num_dice = 3;
                        m_attack[ currentIdx ].damage_dice_sides = 3;
                        break;
                    case 7:
                        m_attack[ currentIdx ].damage_c = 2;
                        m_attack[ currentIdx ].damage_num_dice = 3;
                        m_attack[ currentIdx ].damage_dice_sides = 3;
                        break;
                    case 8:
                        m_attack[ currentIdx ].damage_c = 0;
                        m_attack[ currentIdx ].damage_num_dice = 3;
                        m_attack[ currentIdx ].damage_dice_sides = 5;
                        break;
                    case 9:
                        m_attack[ currentIdx ].damage_c = 0;
                        m_attack[ currentIdx ].damage_num_dice = 4;
                        m_attack[ currentIdx ].damage_dice_sides = 4;
                        break;
                    case 10:
                        m_attack[ currentIdx ].damage_c = 2;
                        m_attack[ currentIdx ].damage_num_dice = 4;
                        m_attack[ currentIdx ].damage_dice_sides = 4;
                        break;
                    case 11:
                        m_attack[ currentIdx ].damage_c = 4;
                        m_attack[ currentIdx ].damage_num_dice = 4;
                        m_attack[ currentIdx ].damage_dice_sides = 4;
                        break;
                }
                if (k == 0) // main weapon gets a damage boost
                    m_attack[0].damage_c += 2;

                m_attack[ currentIdx ].unknown51 = 0;
                m_attack[ currentIdx ].unknown55 = 0;
                m_attack[ currentIdx ].unknown57 = 0;
                m_attack[ currentIdx ].unknown58 = 0;
                m_attack[ currentIdx ].unknown59 = 0;
                m_attack[ currentIdx ].unknown63 = 0;
                if ( m_attribs[ attribute::Strength ].value[ atIdx::Current ] >= 50 )
                    m_attack[ currentIdx ].unknown57 = (m_attribs[ attribute::Strength ].value[ atIdx::Current ] - 50) / 5;
            }

            int strength_factor = 1;
            if ( currentItem && currentItem->getAttacks() & (item::attack::Shoot | item::attack::Lash | item::attack::Throw) )
            {
                // Some ranged weapons (Modern ones in particular) don't rely on character's strength
                // in order to operate.
                // Those that do, though (eg. Crossbow) are doubly reliant on it compared to melee weapons
                if (currentItem->weaponRequiresStrength())
                {
                    strength_factor = 2;
                }
                else
                {
                    strength_factor = 0;
                }
            }

            // Protect against divide by zero
            if (strength_factor != 0)
            {
                if ( m_attribs[ attribute::Strength ].value[ atIdx::Current ] < 50 )
                {
                    m_attack[ currentIdx ].MAR -= (50 - m_attribs[ attribute::Strength ].value[ atIdx::Current ]) / (10 * strength_factor);
                    m_attack[ currentIdx ].multiplier -= (50 - m_attribs[ attribute::Strength ].value[ atIdx::Current ]) / strength_factor;
                }
                else if ( m_attribs[ attribute::Strength ].value[ atIdx::Current ] > 50 )
                {
                    if (k == 1)
                        strength_factor *= 2;
                    m_attack[ currentIdx ].MAR += (m_attribs[ attribute::Strength ].value[ atIdx::Current ] - 50) / (10 * strength_factor);
                    m_attack[ currentIdx ].multiplier += (2 * m_attribs[ attribute::Strength ].value[ atIdx::Current ] - 100) / strength_factor;
                }
            }

            int swing_score = (m_attack[ currentIdx ].skill_total +
                               load_penalty * 15 +
                               prof_BAR +
                               dual_wield_penalty +
                               m_attribs[ attribute::Speed ].value[ atIdx::Current ] +
                               m_attack[ currentIdx ].to_init * 10) / 3;

            m_attack[ currentIdx ].max_swings = 1;
            if ( swing_score >= 67 )
            {
                m_attack[ currentIdx ].max_swings = 2;
                if ( swing_score >= 100 )
                    m_attack[ currentIdx ].max_swings = 3;
            }
            if ( m_item[worn::Weapon1a] && m_item[worn::Weapon1a]->isOneShotMaximum())
                m_attack[ currentIdx ].max_swings = 1;

            if ( m_attribs[ attribute::Dexterity ].value[ atIdx::Current ] < 50 )
            {
                m_attack[ currentIdx ].MAR -= (50 - m_attribs[ attribute::Dexterity ].value[ atIdx::Current ]) / 10;
            }
            else if ( m_attribs[ attribute::Dexterity ].value[ atIdx::Current ] > 50 )
            {
                m_attack[ currentIdx ].MAR += (m_attribs[ attribute::Dexterity ].value[ atIdx::Current ] - 50) / 10;
            }

            if ( m_attribs[ attribute::Senses ].value[ atIdx::Current ] < 30 )
            {
                m_attack[ currentIdx ].MAR -= (30 - m_attribs[ attribute::Senses ].value[ atIdx::Current ]) / 10;
            }
            else if ( m_attribs[ attribute::Senses ].value[ atIdx::Current ] > 70 )
            {
                m_attack[ currentIdx ].MAR += (m_attribs[ attribute::Senses ].value[ atIdx::Current ] - 70) / 10;
            }

            if ( k == 1 )
            {
                load_penalty = 3 * load_penalty / 2;
            }
            // The penalty variables are all negatives already
            m_attack[ currentIdx ].MAR += load_penalty;

            if ( m_skills[ skill::EagleEye ].enabled && ( m_attack[ currentIdx ].skill2 == skill::RangedCombat ))
                m_attack[ currentIdx ].MAR += m_skills[ skill::EagleEye ].value[ atIdx::Current ] / 20 + 1;

            if ( m_skills[ skill::PowerStrike ].enabled && ( m_attack[ currentIdx ].skill2 == skill::CloseCombat ))
                m_attack[ currentIdx ].MAR += m_skills[ skill::PowerStrike ].value[ atIdx::Current ] / 20 + 1;
        }
    }
}

bool character::getProfessionLevel( int idx, QString &profession, int &level ) const
{
    QMapIterator<character::profession, int>   iter(m_currentLevels);

    while (iter.hasNext())
    {
        iter.next();

        if (idx-- == 0)
        {
            for (int k=0; k<m_metaProf.keyCount(); k++)
            {
                if (m_metaProf.value(k) == iter.key())
                {
                    profession = ::getStringTable()->getString( StringList::LISTProfessions + k );
                    break;
                }
            }
            level = iter.value();
            return true;
        }
    }
    return false;
}

int character::getProfessionInitialLevel( character::profession prof ) const
{
    if (! m_initialLevels.contains( prof ))
        return 0;

    return m_initialLevels.value( prof );
}

int character::getProfessionLevel( character::profession prof ) const
{
    if (! m_currentLevels.contains( prof ))
        return 0;

    return m_currentLevels.value( prof );
}

void character::setProfessionLevel( character::profession prof, int level )
{
    if (m_currentLevels.contains( prof ) || (level > 0))
    {
        m_currentLevels[ prof ] = level;
    }
}

bool character::isItemUsable( const item i) const
{
    if (! (m_profession & i.getUsableProfessions()))
        return false;
    if (! (m_race & i.getUsableRaces()))
        return false;
    if (! (m_gender & i.getUsableGenders()))
        return false;

    for (int k=0; k<2; k++)
    {
        character::attribute a;
        qint32               v;

        i.getRequiredAttrib( 0, &a, &v );

        if ((a != attribute::ATTRIBUTE_NONE)  && m_attribs[ a ].value[ atIdx::Current ] < v)
            return false;
    }
    for (int k=0; k<2; k++)
    {
        character::skill  s;
        qint32            v;

        i.getRequiredSkill( 0, &s, &v );

        if ((s != skill::SKILL_NONE)  && m_skills[ s ].value[ atIdx::Current ] < v)
            return false;
    }
    spell item_spell = i.getSpell();
    switch (i.getSpellUsageType())
    {
        case item::spell_usage_type::Spellbook:
            if (m_spell[ item_spell.getIndex() ] == 1) // Spell already known
                return false;
            break;

        case item::spell_usage_type::Instrument:
            if ( m_currentLevels.value( profession::Bard, 0 ) < item_spell.getLevelAsPureClass() )
                return false;
            break;

        case item::spell_usage_type::Gadget:
            if ( m_currentLevels.value( profession::Gadgeteer, 0 ) < item_spell.getLevelAsPureClass() )
                return false;
            break;

        default:
            break;
    }
    return true;
}

character::skill character::getProfessionalSkill() const
{
    return getProfessionalSkill( m_profession );
}

character::skill character::getProfessionalSkill( character::profession prof )
{
    switch (prof)
    {
        case profession::Fighter:    return skill::CloseCombat;
        case profession::Lord:       return skill::DualWeapons;
        case profession::Valkyrie:   return skill::Polearm;
        case profession::Ranger:     return skill::RangedCombat;
        case profession::Samurai:    return skill::Sword;
        case profession::Ninja:      return skill::CriticalStrike;
        case profession::Monk:       return skill::MartialArts;
        case profession::Rogue:      return skill::Locks_Traps;
        case profession::Gadgeteer:  return skill::ModernWeapon;
        case profession::Bard:       return skill::Communication;
        case profession::Priest:     return skill::Divinity;
        case profession::Alchemist:  return skill::Alchemy;
        case profession::Bishop:     return skill::Artifacts;
        case profession::Psionic:    return skill::Psionics;
        case profession::Mage:       return skill::Wizardry;
    }
    return skill::SKILL_NONE;
}

QList<character::skill> character::getProfessionalSkills() const
{
    return getProfessionalSkills( m_profession );
}

QList<character::skill> character::getProfessionalSkills( character::profession prof )
{
    QList<character::skill> skills;

    switch (prof)
    {
        case profession::Fighter:
            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::Sword );
            skills.append( skill::Axe );
            skills.append( skill::Shield );
            break;

        case profession::Lord:
            skills.append( skill::DualWeapons );
            skills.append( skill::CloseCombat );
            skills.append( skill::Sword );
            skills.append( skill::Dagger );
            break;

        case profession::Valkyrie:
            skills.append( skill::Polearm );
            skills.append( skill::CloseCombat );
            skills.append( skill::Mythology );
            skills.append( skill::Axe );
            break;

        case profession::Ranger:
            skills.append( skill::RangedCombat );
            skills.append( skill::Scouting );
            skills.append( skill::Mythology );
            skills.append( skill::Bow );
            break;

        case profession::Samurai:
            skills.append( skill::Sword );
            skills.append( skill::CloseCombat );
            skills.append( skill::DualWeapons );
            skills.append( skill::CriticalStrike );
            break;

        case profession::Ninja:
            skills.append( skill::CriticalStrike );
            skills.append( skill::CloseCombat );
            skills.append( skill::Stealth );
            skills.append( skill::Throwing_Sling );
            skills.append( skill::MartialArts );
            break;

        case profession::Monk:
            skills.append( skill::MartialArts );
            skills.append( skill::CloseCombat );
            skills.append( skill::CriticalStrike );
            skills.append( skill::Stealth );
            skills.append( skill::Staff_Wand );
            break;

        case profession::Rogue:
            skills.append( skill::Locks_Traps );
            skills.append( skill::DualWeapons );
            skills.append( skill::Pickpocket );
            skills.append( skill::Stealth );
            skills.append( skill::Dagger );
            break;

        case profession::Gadgeteer:
            skills.append( skill::ModernWeapon );
            skills.append( skill::RangedCombat );
            skills.append( skill::Engineering );
            skills.append( skill::Locks_Traps );
            break;

        case profession::Bard:
            skills.append( skill::Communication );
            skills.append( skill::Music );
            skills.append( skill::Mythology );
            skills.append( skill::Artifacts );
            break;

        case profession::Priest:
            skills.append( skill::Divinity );
            skills.append( skill::Communication );
            skills.append( skill::Mace_Flail );
            skills.append( skill::Staff_Wand );
            break;

        case profession::Alchemist:
            skills.append( skill::Alchemy );
            skills.append( skill::Mythology );
            skills.append( skill::Throwing_Sling );
            break;

        case profession::Bishop:
            skills.append( skill::Artifacts );
            skills.append( skill::Alchemy );
            skills.append( skill::Wizardry );
            skills.append( skill::Divinity );
            skills.append( skill::Psionics );
            break;

        case profession::Psionic:
            skills.append( skill::Psionics );
            skills.append( skill::Communication );
            skills.append( skill::Mythology );
            skills.append( skill::MentalMagic );
            break;

        case profession::Mage:
            skills.append( skill::Wizardry );
            skills.append( skill::FireMagic );
            skills.append( skill::WaterMagic );
            skills.append( skill::AirMagic );
            skills.append( skill::MentalMagic );
            break;
    }
    return skills;
}

QList<character::skill> character::getTrainableSkills() const
{
    QList<character::skill> skills = getTrainableSkills( m_profession, m_race );

    if (m_attribs[ attribute::Strength ].value[ atIdx::Base ] >= 100)
    {
        skills.append( skill::PowerStrike );
    }
    if (m_attribs[ attribute::Intelligence ].value[ atIdx::Base ] >= 100)
    {
        skills.append( skill::PowerCast );
    }
    if (m_attribs[ attribute::Piety ].value[ atIdx::Base ] >= 100)
    {
        skills.append( skill::IronWill );
    }
    if (m_attribs[ attribute::Vitality ].value[ atIdx::Base ] >= 100)
    {
        skills.append( skill::IronSkin );
    }
    if (m_attribs[ attribute::Dexterity ].value[ atIdx::Base ] >= 100)
    {
        skills.append( skill::Reflexion );
    }
    if (m_attribs[ attribute::Speed ].value[ atIdx::Base ] >= 100)
    {
        skills.append( skill::SnakeSpeed );
    }
    if (m_attribs[ attribute::Senses ].value[ atIdx::Base ] >= 100)
    {
        skills.append( skill::EagleEye );
    }

    return skills;
}

QList<character::skill> character::getTrainableSkills( character::profession prof, character::race race )
{
    QList<character::skill> skills;

    // Slight deviation from the game - character level is also
    // important in determining the availability of a skill, at
    // least where it relates to magic. The hybrid casters cannot
    // develop their magic at all until they are a few levels up,
    // but even pure casters don't have all realms available to
    // them at level 1. Enforcing or even indicating this restriction
    // in the editor is just confusing, so we don't bother.

    switch (prof)
    {
        case profession::Fighter:
            skills.append( skill::Sword );
            skills.append( skill::Axe );
            skills.append( skill::Polearm );
            skills.append( skill::Mace_Flail );
            skills.append( skill::Dagger );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Shield );
            skills.append( skill::ModernWeapon );
            skills.append( skill::Bow );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::DualWeapons );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );
            break;

        case profession::Lord:
        case profession::Valkyrie:
            skills.append( skill::Sword );
            skills.append( skill::Axe );
            skills.append( skill::Polearm );
            skills.append( skill::Mace_Flail );
            skills.append( skill::Dagger );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Shield );
            skills.append( skill::ModernWeapon );
            skills.append( skill::Bow );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::DualWeapons );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );

            if (race != character::race::Android)
            {
                skills.append( skill::Divinity );
                skills.append( skill::FireMagic );
                skills.append( skill::WaterMagic );
                skills.append( skill::AirMagic );
                skills.append( skill::EarthMagic );
                skills.append( skill::MentalMagic );
                skills.append( skill::DivineMagic );
            }
            break;

        case profession::Ranger:
            skills.append( skill::Sword );
            skills.append( skill::Axe );
            skills.append( skill::Polearm );
            skills.append( skill::Mace_Flail );
            skills.append( skill::Dagger );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Shield );
            skills.append( skill::ModernWeapon );
            skills.append( skill::Bow );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::Scouting );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::DualWeapons );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );

            if (race != character::race::Android)
            {
                skills.append( skill::Alchemy );
                skills.append( skill::FireMagic );
                skills.append( skill::WaterMagic );
                skills.append( skill::AirMagic );
                skills.append( skill::EarthMagic );
                // NB: No alchemical spells use mental
                skills.append( skill::DivineMagic );
            }
            break;

        case profession::Samurai:
            skills.append( skill::Sword );
            skills.append( skill::Dagger );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Shield );
            skills.append( skill::Bow );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::DualWeapons );
            skills.append( skill::CriticalStrike );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );

            if (race != character::race::Android)
            {
                skills.append( skill::Wizardry );
                skills.append( skill::FireMagic );
                skills.append( skill::WaterMagic );
                skills.append( skill::AirMagic );
                skills.append( skill::EarthMagic );
                skills.append( skill::MentalMagic );
                skills.append( skill::DivineMagic );
            }
            break;

        case profession::Ninja:
            skills.append( skill::Sword );
            skills.append( skill::Axe );
            skills.append( skill::Mace_Flail );
            skills.append( skill::Dagger );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Bow );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::Locks_Traps );
            skills.append( skill::Stealth );
            skills.append( skill::Pickpocket );
            skills.append( skill::MartialArts );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::DualWeapons );
            skills.append( skill::CriticalStrike );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );

            if (race != character::race::Android)
            {
                skills.append( skill::Alchemy );
                skills.append( skill::FireMagic );
                skills.append( skill::WaterMagic );
                skills.append( skill::AirMagic );
                skills.append( skill::EarthMagic );
                // NB: No alchemical spells use mental
                skills.append( skill::DivineMagic );
            }
            break;

        case profession::Monk:
            skills.append( skill::Polearm );
            skills.append( skill::Mace_Flail );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::Stealth );
            skills.append( skill::MartialArts );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::DualWeapons );
            skills.append( skill::CriticalStrike );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );

            if (race != character::race::Android)
            {
                skills.append( skill::Psionics );
                skills.append( skill::FireMagic );
                skills.append( skill::WaterMagic );
                skills.append( skill::AirMagic );
                skills.append( skill::EarthMagic );
                skills.append( skill::MentalMagic );
                skills.append( skill::DivineMagic );
            }
            break;

        case profession::Rogue:
            skills.append( skill::Sword );
            skills.append( skill::Dagger );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Shield );
            skills.append( skill::ModernWeapon );
            skills.append( skill::Bow );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::Locks_Traps );
            skills.append( skill::Stealth );
            skills.append( skill::Pickpocket );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::DualWeapons );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );
            break;

        case profession::Gadgeteer:
            skills.append( skill::Sword );
            skills.append( skill::Dagger );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Shield );
            skills.append( skill::ModernWeapon );
            skills.append( skill::Bow );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::Locks_Traps );
            skills.append( skill::Engineering );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::DualWeapons );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );
            break;

        case profession::Bard:
            skills.append( skill::Sword );
            skills.append( skill::Dagger );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Shield );
            skills.append( skill::Bow );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::Locks_Traps );
            skills.append( skill::Music );
            skills.append( skill::Pickpocket );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::DualWeapons );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );
            break;

        case profession::Priest:
            skills.append( skill::Mace_Flail );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Shield );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );

            if (race != character::race::Android)
            {
                skills.append( skill::Divinity );
                skills.append( skill::FireMagic );
                skills.append( skill::WaterMagic );
                skills.append( skill::AirMagic );
                skills.append( skill::EarthMagic );
                skills.append( skill::MentalMagic );
                skills.append( skill::DivineMagic );
            }
            break;

        case profession::Alchemist:
            skills.append( skill::Dagger );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Shield );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );

            if (race != character::race::Android)
            {
                skills.append( skill::Alchemy );
                skills.append( skill::FireMagic );
                skills.append( skill::WaterMagic );
                skills.append( skill::AirMagic );
                skills.append( skill::EarthMagic );
                // NB: No alchemical spells use mental
                skills.append( skill::DivineMagic );
            }
            break;

        case profession::Bishop:
            skills.append( skill::Mace_Flail );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Shield );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );

            if (race != character::race::Android)
            {
                skills.append( skill::Wizardry );
                skills.append( skill::Divinity );
                skills.append( skill::Alchemy );
                skills.append( skill::Psionics );
                skills.append( skill::FireMagic );
                skills.append( skill::WaterMagic );
                skills.append( skill::AirMagic );
                skills.append( skill::EarthMagic );
                skills.append( skill::MentalMagic );
                skills.append( skill::DivineMagic );
            }
            break;

        case profession::Psionic:
            skills.append( skill::Dagger );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );

            if (race != character::race::Android)
            {
                skills.append( skill::Psionics );
                skills.append( skill::FireMagic );
                skills.append( skill::WaterMagic );
                skills.append( skill::AirMagic );
                skills.append( skill::EarthMagic );
                skills.append( skill::MentalMagic );
                skills.append( skill::DivineMagic );
            }
            break;

        case profession::Mage:
            skills.append( skill::Dagger );
            skills.append( skill::Staff_Wand );
            skills.append( skill::Throwing_Sling );

            skills.append( skill::CloseCombat );
            skills.append( skill::RangedCombat );
            skills.append( skill::Artifacts );
            skills.append( skill::Mythology );
            skills.append( skill::Communication );

            if (race != character::race::Android)
            {
                skills.append( skill::Wizardry );
                skills.append( skill::FireMagic );
                skills.append( skill::WaterMagic );
                skills.append( skill::AirMagic );
                skills.append( skill::EarthMagic );
                skills.append( skill::MentalMagic );
                skills.append( skill::DivineMagic );
            }
            break;
    }

    return skills;
}

QStringList character::getAbilityStrings() const
{
    QStringList abilities;

    abilities << QString("%1 %2").arg( ::getStringTable()->getString( StringList::LISTSkills + getProfessionalSkill() ) )
                                 .arg( ::getStringTable()->getString( StringList::SkillBonus ) );

    switch (m_profession)
    {
        case profession::Fighter:
            abilities << ::getStringTable()->getString( StringList::ProfBonusStaminaReg );
            abilities << ::getStringTable()->getString( StringList::ProfBonusKO         );
            abilities << ::getStringTable()->getString( StringList::ProfBonusBerserk    );
            break;

        case profession::Lord:
            abilities << ::getStringTable()->getString( StringList::ProfBonusHealthReg  );
            break;

        case profession::Valkyrie:
            abilities << ::getStringTable()->getString( StringList::ProfBonusCheatDeath );
            break;

        case profession::Ranger:
            abilities << ::getStringTable()->getString( StringList::ProfBonusRangedCrit );
            abilities << ::getStringTable()->getString( StringList::ProfBonusAutoSearch );
            break;

        case profession::Samurai:
            abilities << ::getStringTable()->getString( StringList::ProfBonusFearless   );
            abilities << ::getStringTable()->getString( StringList::ProfBonusLightning  );
            break;

        case profession::Ninja:
            abilities << ::getStringTable()->getString( StringList::ProfBonusThrownCrit );
            abilities << ::getStringTable()->getString( StringList::ProfBonusThrownPen  );
            break;

        case profession::Monk:
            abilities << ::getStringTable()->getString( StringList::ProfBonusDmgResist  );
            abilities << ::getStringTable()->getString( StringList::ProfBonusFightBlind );
            break;

        case profession::Rogue:
            abilities << ::getStringTable()->getString( StringList::ProfBonusBackstab   );
            break;

        case profession::Gadgeteer:
            abilities << ::getStringTable()->getString( StringList::ProfBonusMakeGadget );
            break;

        case profession::Bard:
            abilities << ::getStringTable()->getString( StringList::ProfBonusPartyCamp  );
            break;

        case profession::Priest:
            abilities << ::getStringTable()->getString( StringList::ProfBonusPrayMiracle);
            abilities << ::getStringTable()->getString( StringList::ProfBonusDispelUnd  );
            break;

        case profession::Alchemist:
            abilities << ::getStringTable()->getString( StringList::ProfBonusMakePotions);
            break;

        case profession::Bishop:
            abilities << ::getStringTable()->getString( StringList::ProfBonusRemoveCurse);
            abilities << ::getStringTable()->getString( StringList::ProfBonusDispelUnd  );
            break;

        case profession::Psionic:
            abilities << ::getStringTable()->getString( StringList::ProfBonusFearless   );
            abilities << ::getStringTable()->getString( StringList::ProfBonusMentalImm  );
            break;

        case profession::Mage:
            abilities << ::getStringTable()->getString( StringList::ProfBonusMgcResist  );
            break;
    }

    switch (m_race)
    {
        case race::Dwarf:
            abilities << ::getStringTable()->getString( StringList::RaceBonusDmgResist  );
            break;

        case race::Lizardman:
            abilities << ::getStringTable()->getString( StringList::RaceBonusSlowMgcRec );
            break;

        case race::Dracon:
            abilities << ::getStringTable()->getString( StringList::RaceBonusAcidBreath );
            break;

        case race::Faerie:
            abilities << ::getStringTable()->getString( StringList::RaceBonusAC         );
            abilities << ::getStringTable()->getString( StringList::RaceBonusEquipment  );
            abilities << ::getStringTable()->getString( StringList::RaceBonusReducedCC  );
            abilities << ::getStringTable()->getString( StringList::RaceBonusWeightLimit);
            abilities << ::getStringTable()->getString( StringList::RaceBonusFastMgcRec );
            break;

        case race::Android:
            abilities << ::getStringTable()->getString( StringList::RaceBonusAndroid    );
            abilities << ::getStringTable()->getString( StringList::RaceBonusNoMagic    );
            break;

        default:
            break;
    }

    return abilities;
}

bool character::isAlive() const
{
    return m_charslot_inuse && (m_main_status_condition < condition::Dead);
}

bool character::isSpellKnown( int idx ) const
{
    if ((idx > 0) && (idx < MAXIMUM_CHARACTER_SPELLS))
    {
        if ((m_spell[idx] == 1) || (m_spell[idx] == 2))
            return true;
        return false;
    }
    return false;
}

void character::setSpellKnown( int idx, bool known )
{
    if ((idx > 0) && (idx < MAXIMUM_CHARACTER_SPELLS))
    {
        // I don't know under what circumstances we ever put a 2
        // into the known spell
        if (known)
            m_spell[idx] = 1;
        else
            m_spell[idx] = -1;
    }
}

double character::getLoad(atIdx idx) const
{
    if (idx == Base)
        return (double)m_carry_capacity / 10.0;
    return (double)m_encumbrance / 10.0;
}

int character::getLoadCategory() const
{
    return m_load_category;
}

void character::recomputeBonus()
{
#ifdef SIMPLIFIED_BONUSES
    // Don't process any penalties at all, just the item bonuses, and
    // paste them directly onto the bonus struct
    memset( &m_bonus, 0, sizeof(struct bonus));

    recomputeItemBonus( &m_bonus );
#else
    // Process all the penalties, bonuses etc. same as in the game. And
    // maintain the corrrect separate game arrays for tracking them all
    // independently.
    // This does make for a confusing editing experience when trying to
    // adjust attributes and skills and it is extremely incomplete, and
    // not at present something I intend to pursue further.

    memset( &m_bonus_penalties, 0, sizeof(struct bonus));
    memset( &m_bonus_items,     0, sizeof(struct bonus));
    memset( &m_bonus,           0, sizeof(struct bonus));

    recomputeConditionPenalty( &m_bonus_penalties );
    //sub_50ECC0(v1 + 2661, &m_bonus_penalties ); -- depends on 0xa65--0xac5
    //sub_50DBF0(v1, &m_bonus_penalties );

    recomputeItemBonus( &m_bonus_items );

    for (int k=0; k<sizeof(struct bonus); k++)
    {
        *((qint8 *)m_bonus+k) = *((qint8 *)m_bonus_items+k) +
                                *((qint8 *)m_bonus_penalties+k);
        if (m_in_party)
            *((qint8 *)m_bonus+k) += *((qint8 *)unk_687453+k);
    }
#endif
}

void character::recomputeItemBonus(struct character::bonus *b)
{
    int mr[ REALM_SIZE ];

    b->magic_resistance[ realm::Fire   ] = 0;
    b->magic_resistance[ realm::Water  ] = 0;
    b->magic_resistance[ realm::Air    ] = 0;
    b->magic_resistance[ realm::Earth  ] = 0;
    b->magic_resistance[ realm::Mental ] = 0;
    b->magic_resistance[ realm::Divine ] = 0;

    for (int k=Head; k < WORN_SIZE; k++ )
    {
        character::worn bodypart = static_cast<character::worn>(k);

        if ((bodypart != worn::Weapon2a) &&
            (bodypart != worn::Weapon2b) &&
            m_item[ bodypart ] )
        {
            if ((bodypart != worn::Weapon1a) &&
                (bodypart != worn::Weapon1b))
            {
                b->to_init   += m_item[ bodypart ]->getInitiative();
                b->MAR       += m_item[ bodypart ]->getToHit();
            }
            b->hp_regen      += m_item[ bodypart ]->getHPRegen();
            b->stamina_regen += m_item[ bodypart ]->getStaminaRegen();
            b->sp_regen      += m_item[ bodypart ]->getSPRegen();

            int howmuch = 0;
            skill s = m_item[ bodypart ]->getSkillBonus(&howmuch);
            if ( s != SKILL_NONE )
                b->skills[ (int)s ] += howmuch;

            howmuch = 0;
            attribute a = m_item[ bodypart ]->getAttributeBonus(&howmuch);
            if ( a != ATTRIBUTE_NONE )
                b->attributes[ (int)a ] += howmuch;

            bool affectsMR = m_item[  bodypart ]->getResistance( &mr[ realm::Fire   ],
                                                                 &mr[ realm::Water  ],
                                                                 &mr[ realm::Air    ],
                                                                 &mr[ realm::Earth  ],
                                                                 &mr[ realm::Mental ],
                                                                 &mr[ realm::Divine ] );

            if (affectsMR)
            {
                for (int k = 0; k < REALM_SIZE; k++ )
                {
                    b->magic_resistance[ k ] += mr[ k ];

                    if ( b->magic_resistance[k] <= -125 )
                         b->magic_resistance[k]  = -125;
                    if ( b->magic_resistance[k] >=  125 )
                         b->magic_resistance[k]  =  125;
                }
            }
        }
    }
}

void character::recomputeLoadCategory()
{
    quint32 loadage = 100 * m_encumbrance / m_carry_capacity;

    if      (loadage < 50)   m_load_category = 0;
    else if (loadage < 70)   m_load_category = 1;
    else if (loadage < 85)   m_load_category = 2;
    else if (loadage <= 100) m_load_category = 3;
    else                     m_load_category = 4;

    recomputeInitiative();
    recomputeAttack();
    recomputeAcmod();
}

void character::recomputeCarryCapacity()
{
    m_carry_capacity = 12 * (m_attribs[ attribute::Vitality ].value[ atIdx::Current ] +
                             m_attribs[ attribute::Strength ].value[ atIdx::Current ] * 2);

    if (m_race == race::Faerie)
        m_carry_capacity = m_carry_capacity * 2 / 3;
}

void character::recomputePersonalLoad()
{
    double weight = 0;

    // Personal load
    for (int k=0; k<WORN_SIZE; k++)
    {
        if (m_item[k])
        {
            if (m_item[k]->isStackable())
            {
                weight += m_item[k]->getCount() * m_item[k]->getWeight();
            }
            else
            {
                weight += m_item[k]->getWeight();
            }
        }
    }
    m_personal_load = (quint16)(weight*10);
}

qint32 character::getPersonalLoad() const
{
    return m_personal_load;
}

qint32 character::getPartyShare() const
{
    return m_party_share;
}

void character::setPartyShare(qint32 weight)
{
    m_party_share = weight;
    m_encumbrance = m_party_share + m_personal_load;
}

void character::lkupControllingAttribs( character::skill sk, character::attribute *a1, character::attribute *a2)
{
    struct control
    {
        attribute   attrib1;
        attribute   attrib2;
        quint32     unknown1;
        quint32     unknown2;
    };

    static const struct control s[] =
    {
        { attribute::Strength,       attribute::Dexterity,      0,   1 },
        { attribute::Strength,       attribute::Dexterity,      0,   1 },
        { attribute::Strength,       attribute::Dexterity,      0,   1 },
        { attribute::Strength,       attribute::Dexterity,      0,   1 },
        { attribute::Dexterity,      attribute::Speed,          0,   0 },
        { attribute::Strength,       attribute::Dexterity,      0,   1 },
        { attribute::Strength,       attribute::Dexterity,      0,   1 },
        { attribute::Dexterity,      attribute::Speed,          0,   1 },
        { attribute::Dexterity,      attribute::Strength,       0,   0 },
        { attribute::Dexterity,      attribute::Strength,       1,   1 },
        { attribute::Dexterity,      attribute::Intelligence,   1,   1 },
        { attribute::Dexterity,      attribute::Intelligence,   1,   2 },
        { attribute::Dexterity,      attribute::Intelligence,   1,   1 },
        { attribute::Dexterity,      attribute::Speed,          0,   1 },
        { attribute::Dexterity,      attribute::Speed,          1,   2 },
        { attribute::Senses,         attribute::Intelligence,   2,   0 },
        { attribute::Senses,         attribute::Intelligence,   2,   0 },
        { attribute::Senses,         attribute::Intelligence,   2,   1 },
        { attribute::Dexterity,      attribute::Senses,         2,   1 },
        { attribute::Senses,         attribute::Speed,          2,   0 },
        { attribute::Intelligence,   attribute::Senses,         2,   0 },
        { attribute::Senses,         attribute::Intelligence,   2,   0 },
        { attribute::Intelligence,   attribute::Senses,         2,   2 },
        { attribute::Intelligence,   attribute::Dexterity,      3,   1 },
        { attribute::Intelligence,   attribute::Intelligence,   3,   1 },
        { attribute::Piety,          attribute::Piety,          3,   1 },
        { attribute::Dexterity,      attribute::Intelligence,   3,   1 },
        { attribute::Senses,         attribute::Intelligence,   3,   1 },
        { attribute::Intelligence,   attribute::Piety,          3,   1 },
        { attribute::Intelligence,   attribute::Piety,          3,   1 },
        { attribute::Intelligence,   attribute::Piety,          3,   1 },
        { attribute::Intelligence,   attribute::Piety,          3,   1 },
        { attribute::Intelligence,   attribute::Piety,          3,   1 },
        { attribute::Intelligence,   attribute::Piety,          4,   3 },
        { attribute::Strength,       attribute::Strength,       4,   3 },
        { attribute::Intelligence,   attribute::Intelligence,   4,   3 },
        { attribute::Piety,          attribute::Piety,          4,   3 },
        { attribute::Vitality,       attribute::Vitality,       4,   3 },
        { attribute::Dexterity,      attribute::Dexterity,      4,   3 },
        { attribute::Speed,          attribute::Speed,          4,   3 },
        { attribute::Senses,         attribute::Senses,         1,   1 }
    };

    Q_ASSERT(sizeof(s) / sizeof(struct control) == skill::SKILL_SIZE);

    *a1 = s[ sk ].attrib1;
    *a2 = s[ sk ].attrib2;
}

void character::resetControllingAttribs() // and applyProfessionBonus()
{
    for (int k=0; k<attribute::ATTRIBUTE_SIZE; k++)
    {
        character::attribute at = static_cast<character::attribute>(k);

        m_attribs[ at ].value[ atIdx::Current ] = m_attribs[ at ].value[ atIdx::Base ] +
                                   m_bonus.attributes[ at ];
        if (m_attribs[ at ].value[ atIdx::Current ] < 1)
            m_attribs[ at ].value[ atIdx::Current ] = 1;
        if (m_attribs[ at ].value[ atIdx::Current ] > 125)
            m_attribs[ at ].value[ atIdx::Current ] = 125;
    }

    for (int k=0; k<skill::SKILL_SIZE; k++)
    {
        character::skill      sk = static_cast<character::skill>(k);
        character::attribute  attrib1;
        character::attribute  attrib2;

        m_skills[ sk ].value[ atIdx::Current ] = m_skills[ sk ].value[ atIdx::Base ];

        if (sk == getProfessionalSkill())
        {
            int prof_bonus = m_skills[ sk ].value[ atIdx::Base ] * 25 / 100;
            if (prof_bonus == 0)
                prof_bonus = 1;

            m_skills[ sk ].value[ atIdx::Current ] += prof_bonus;
        }

        m_skills[ sk ].value[ atIdx::Current ] += m_bonus.skills[ sk ];

        if (m_skills[ sk ].value[ atIdx::Current ] < 0)
            m_skills[ sk ].value[ atIdx::Current ] = 0;
        else if (m_skills[ sk ].value[ atIdx::Current ] > 125)
            m_skills[ sk ].value[ atIdx::Current ] = 125;

        // So skill training depends on the BASE values of the primary attributes, and you
        // can't make it increase faster by using items which affect the adjusted value
        lkupControllingAttribs( sk, &attrib1, &attrib2 );

        m_skills[ sk ].control = (m_attribs[ attrib1 ].value[ atIdx::Base ] +
                                  m_attribs[ attrib2 ].value[ atIdx::Base ]) / 2;
    }
}

void character::recomputeStamina()
{
    qint32 old_stamina = m_stamina[ atIdx::Base ];

    // Stamina base gets overwritten, Stamina adj gets incorporated - same sort of
    // deal as what they did with HP
    m_stamina[ atIdx::Base ] = (qint32)((double)(m_attribs[ attribute::Strength ].value[ atIdx::Current ] +
                                                 m_attribs[ attribute::Piety    ].value[ atIdx::Current ] +
                                                 m_attribs[ attribute::Vitality ].value[ atIdx::Current ]) / 3.0
                                        * ((double)m_combinedLevel / 10.0 + 2.0)
                                        + 0.5);

    if ( m_stamina_drain >= m_stamina[ atIdx::Base ] )
        m_stamina[ atIdx::Base ] = 0;
    else
        m_stamina[ atIdx::Base ] -= m_stamina_drain;

    if (m_stamina[ atIdx::Base ] != old_stamina)
    {
        // adjust the Current value by the same amount we just adjusted the Base
        m_stamina[ atIdx::Current ] += m_stamina[ atIdx::Base ] - old_stamina;
    }

    if (m_stamina[ atIdx::Current ] > m_stamina[ atIdx::Base ])
    {
        // Don't ever let a recalculated Base be lower than the Current value
        // or it breaks a lot of in-game algorithms
        m_stamina[ atIdx::Current ] = m_stamina[ atIdx::Base ];
    }

    int tmp = 100 - 100 * m_stamina[ atIdx::Current ] / m_stamina[ atIdx::Base ];

    if      (tmp < 50) m_fatigue_category = 0; // some kind of penalty incurred by low stamina percentage; affects attack
    else if (tmp < 70) m_fatigue_category = 1;
    else if (tmp < 85) m_fatigue_category = 2;
    else if (tmp < 95) m_fatigue_category = 3;
    else               m_fatigue_category = 4;
}

void character::recomputeHp()
{
    double hp = 0.0;

    QMapIterator<character::profession, int>   iter(m_currentLevels);

    while (iter.hasNext())
    {
        iter.next();

        profession p     = static_cast<character::profession>(iter.key());
        quint32    level = iter.value();

        if (p == m_origProfession)
            level++;
        if (level != 0)
        {
            double lvl_mult = 0.0;
            switch (p)
            {
                case profession::Fighter:    lvl_mult = 9.0; break;
                case profession::Lord:       lvl_mult = 7.5; break;
                case profession::Valkyrie:   lvl_mult = 7.5; break;
                case profession::Ranger:     lvl_mult = 6.5; break;
                case profession::Samurai:    lvl_mult = 7.0; break;
                case profession::Ninja:      lvl_mult = 6.5; break;
                case profession::Monk:       lvl_mult = 6.5; break;
                case profession::Rogue:      lvl_mult = 6.0; break;
                case profession::Gadgeteer:  lvl_mult = 5.5; break;
                case profession::Bard:       lvl_mult = 6.0; break;
                case profession::Priest:     lvl_mult = 5.0; break;
                case profession::Alchemist:  lvl_mult = 4.5; break;
                case profession::Bishop:     lvl_mult = 4.0; break;
                case profession::Psionic:    lvl_mult = 3.5; break;
                case profession::Mage:       lvl_mult = 3.0; break;
            }
            hp += ((double)(m_attribs[ attribute::Vitality ].value[ atIdx::Current ]) * 0.008 + 0.6) * lvl_mult * level;
        }
    }
    // m_hp_drain is a negative value when active, unlike m_stamina_drain
    // which is positive when active
    hp += m_hp_drain + 0.5;
    if (hp < 1.0)
        hp = 1.0;

    qint32 old_base_hp  = m_hp[ atIdx::Base ];

    m_hp[ atIdx::Base ] = (qint32)hp;

    if ( old_base_hp != m_hp[atIdx::Base] )
    {
        // Attempt to preserve the difference between the Current and Base
        // values if possible.

        m_hp[atIdx::Current] -= old_base_hp - m_hp[atIdx::Base];
    }
    // The Current value should always be less than or equal to the Base.
    if (m_hp[ atIdx::Current ] > m_hp[ atIdx::Base ])
        m_hp[ atIdx::Current ] = m_hp[ atIdx::Base ];

    if (m_hp[ atIdx::Current ] <= 0)
    {
        m_hp[ atIdx::Current ] = 0;

        // And technically at this point you should be transitioned to DEAD
        // But we don't have to do that because the game will do it for us
        // on game load and no doubt do a better job of making sure it does
        // everything it should do when this happens.
    }
}

void character::recomputeKnownSpellsCount()
{
    m_knownSpellsCount[ realm::Fire   ] = 0;
    m_knownSpellsCount[ realm::Water  ] = 0;
    m_knownSpellsCount[ realm::Air    ] = 0;
    m_knownSpellsCount[ realm::Earth  ] = 0;
    m_knownSpellsCount[ realm::Mental ] = 0;
    m_knownSpellsCount[ realm::Divine ] = 0;

    for (int k = 0; k < MAXIMUM_CHARACTER_SPELLS; k++)
    {
        if ((m_spell[k] == 1) || (m_spell[k] == 2))
        {
            spell s(k);
            m_knownSpellsCount[ s.getRealm() ]++;
        }
    }
}

int qsort_cmpfunc (const void * a, const void * b)
{
    return ( *(int*)b - *(int*)a );
}

void character::recomputeManaPoints()
{
    quint32        mostExpensiveSpell[6];
    quint32        max;
    quint32        school[4];
    quint32        school_weighted_av;

    max = 0;
    recomputeKnownSpellsCount();

    mostExpensiveSpell[ realm::Fire   ] = 0;
    mostExpensiveSpell[ realm::Water  ] = 0;
    mostExpensiveSpell[ realm::Air    ] = 0;
    mostExpensiveSpell[ realm::Earth  ] = 0;
    mostExpensiveSpell[ realm::Mental ] = 0;
    mostExpensiveSpell[ realm::Divine ] = 0;

    for (int k = 0; k < MAXIMUM_CHARACTER_SPELLS; k++)
    {
        if ( m_spell[k] == 1 || m_spell[k] == 2 )
        {
            spell s(k);

            quint32 spcost = s.getSPCost();
            quint32 realm  = s.getRealm();

            if ( spcost > mostExpensiveSpell[realm] )
                mostExpensiveSpell[realm] = spcost;
        }
    }

    school[ 0 ] = m_skills[ skill::Wizardry ].value[ atIdx::Current ];
    school[ 1 ] = m_skills[ skill::Divinity ].value[ atIdx::Current ];
    school[ 2 ] = m_skills[ skill::Alchemy  ].value[ atIdx::Current ];
    school[ 3 ] = m_skills[ skill::Psionics ].value[ atIdx::Current ];

    // sort largest to smallest
    qsort( school, 4, sizeof(quint32), qsort_cmpfunc );

    // so most powerful school counts 2* as much as next powerful, etc.
    school_weighted_av = (school[3] >> 3) +
                         (school[2] >> 2) +
                         (school[1] >> 1) +
                          school[0];

    if (school_weighted_av >= 125)
        school_weighted_av = 125;

    for (int k = 0; k < REALM_SIZE; k++)
    {
        quint32 old_mana = m_mp[ realm::Fire + k ][atIdx::Base];

        quint32 m1 = m_skills[ skill::FireMagic + k ].value[ atIdx::Current ] * 3 +
                     m_attribs[ attribute::Piety ].value[ atIdx::Current ]     +
                     school_weighted_av;
        quint32 m2 = m_knownSpellsCount[k] +
                     m_combinedLevel + 1;
        quint32 mp = (quint32)((double)(m1 * m2) * 0.02  + 0.5);

        if (mp > max)
            max = mp;

        if (m_knownSpellsCount[k] > 0)
            m_mp[realm::Fire + k][atIdx::Base] = mp;
        else
            m_mp[realm::Fire + k][atIdx::Base] = 0;

        // Ensure they have enough points to cast their most expensive spell, even
        // if it depletes them entirely in one go to do so.
        if (m_mp[realm::Fire + k][atIdx::Base] < mostExpensiveSpell[k])
            m_mp[realm::Fire + k][atIdx::Base] = mostExpensiveSpell[k];

        // If our new calculation is any different to before, then change the current too
        if (m_mp[realm::Fire + k][atIdx::Base] != old_mana)
            m_mp[realm::Fire + k][atIdx::Current] += m_mp[realm::Fire + k][atIdx::Base] - old_mana;
    }

    m_mpStrongestRealm = max;
}

void character::recomputeAcmod()
{
    m_acmod_race        = 0;
    m_acmod_speed       = 0;
    m_acmod_stealth     = 0;
    m_acmod_shield      = 0;
    m_acmod_mitems      = 0;
    m_acmod_mspells     = 0;
    m_acmod_penetrate   = 0;
    m_acmod_encumbrance = 0;
    m_acmod_conditions  = 0;
    m_acmod_fatigue     = 0;
    m_acmod_defensive   = 0;
    m_acmod_reflextion  = 0;
    m_acmod_unknown     = 0;

    for (int i=0; i< WORN_SIZE; i++)
    {
        item *it = m_item[ static_cast<character::worn>(i) ];

        if (it)
        {
            switch (i)
            {
                case worn::Cloak:
                case worn::Misc1:
                case worn::Misc2:
                case worn::Weapon1a:
                case worn::Weapon1b:
                    if (it->getType() == item::type::Shield)
                        m_acmod_shield += it->getAC();
                    else
                        m_acmod_mitems += it->getAC();
                    break;

                default:
                    break;
            }
        }
    }

    if ( (int)m_main_status_condition < condition::Dead )
    {
        if (m_race == race::Faerie)
            m_acmod_race += 2;

        if ( m_attribs[ attribute::Speed ].value[ atIdx::Current ] < 10 )
            m_acmod_speed -= 2;
        else if ( m_attribs[ attribute::Speed ].value[ atIdx::Current ] < 20 )
            m_acmod_speed -= 1;
        else if ( m_attribs[ attribute::Speed ].value[ atIdx::Current ] >= 90 )
            m_acmod_speed += 2;
        else if ( m_attribs[ attribute::Speed ].value[ atIdx::Current ] >= 80 )
            m_acmod_speed += 1;

        m_acmod_stealth += m_skills[ skill::Stealth ].value[ atIdx::Current ] / 10;

        if (m_skills[ skill::Reflexion ].enabled)
            m_acmod_reflextion += m_skills[ skill::Reflexion ].value[ atIdx::Current ] / 20 + 1;

        if ( m_acmod_shield > 0 )
        {
            int enh = m_skills[ skill::Shield ].value[ atIdx::Current ] / 25;
            if (enh >= m_acmod_shield)
                enh =  m_acmod_shield;

            m_acmod_shield += enh;
        }

        m_acmod_mspells    += m_bonus.mspells;
        m_acmod_conditions += m_bonus.conditions;

        if ( m_load_category == 2 )
        {
            m_acmod_encumbrance -= 1;
        }
        else if ( m_load_category == 3 )
        {
            m_acmod_encumbrance -= 2;
        }
        else if ( m_load_category == 4 )
        {
            m_acmod_encumbrance -= 4;
        }
        switch (m_fatigue_category)
        {
            case 0:                        break;
            case 1:                        break;
            case 2:  m_acmod_fatigue -= 1; break;
            case 3:  m_acmod_fatigue -= 2; break;
            case 4:  m_acmod_fatigue -= 4; break;
            default: m_acmod_fatigue -= m_fatigue_category / 10; break;
        }
        m_acmod_penetrate += m_bonus.vpenetrate;
    }

    m_acmod_base =
         m_acmod_race        +
         m_acmod_speed       +
         m_acmod_stealth     +
         m_acmod_shield      +
         m_acmod_mitems      +
         m_acmod_mspells     +
         m_acmod_encumbrance +
         m_acmod_conditions  +
         m_acmod_fatigue     +
         m_acmod_defensive   +
         m_acmod_reflextion;

    if ( m_bonus.incapacitated && (m_acmod_base > -5) )
        m_acmod_base = -5;

    int bodypart_sum = 0;
    for (int i=0; i< WORN_SIZE; i++)
    {
        character::worn bodypart = static_cast<character::worn>(i);

        int multiplier = -1;

        switch (bodypart)
        {
            case worn::Head:  multiplier = 15; break;
            case worn::Torso: multiplier = 40; break;
            case worn::Legs:  multiplier = 30; break;
            case worn::Hand:  multiplier = 10; break;
            case worn::Feet:  multiplier =  5; break;
            default:          multiplier =  0; break;
        }

        switch (bodypart)
        {
            case worn::Head:
            case worn::Torso:
            case worn::Legs:
            case worn::Hand:
            case worn::Feet:
                m_ac_bodypart[ bodypart ] = m_acmod_penetrate + m_acmod_base;
                if (m_item[ bodypart ])
                    m_ac_bodypart[ bodypart ] += m_item[ bodypart ]->getAC();
                bodypart_sum += m_ac_bodypart[ bodypart ] * multiplier;
                break;

            default:
                break;
        }
    }

    // Rounded average calculation.
    if ( bodypart_sum < 0 )
        m_acmod_average = (bodypart_sum - 50) / 100;
    else
        m_acmod_average = (bodypart_sum + 50) / 100;
}

void character::recomputeRecoveryRates()
{
    m_healing_rate = ((double)m_hp[atIdx::Base] * 0.40000001 + 20.0) * 0.0041666669;
    if ( m_bonus.unknown_0x17b2 != 0 )
        m_healing_rate *= 1.5;

    m_rest_rate = ((double)m_stamina[atIdx::Base] * 0.89999998 + 20.0) * 0.0041666669;
    if ( m_bonus.unknown_0x17b3 != 0 )
        m_rest_rate *= 1.5;

    for (int k = 0; k < REALM_SIZE; k++)
    {
        if ( m_mp[realm::Fire + k][atIdx::Base] <= 0u )
        {
            m_mp_recovery[k][0] = 0;
        }
        else
        {
            m_mp_recovery[k][0] = ((double)m_mp[realm::Fire + k][atIdx::Base] * 0.64999998 + 20.0) * 0.0041666669;
            if ( m_bonus.unknown_0x17b4 != 0 )
                m_mp_recovery[k][0] *= 1.5;
        }
    }
}

void character::recomputeDamageAbsorption()
{
    m_damage_absorption = 0;

    if (m_race == race::Dwarf)
        m_damage_absorption += m_attribs[ attribute::Vitality ].value[ atIdx::Current ] / 10;

    if (m_profession == profession::Monk)
    {
        if (m_currentLevels[ m_profession ] < 20)
            m_damage_absorption += (int)((double)((2 * m_currentLevels[ m_profession ] + 60) * 15) * 0.0099999998);
        else
            m_damage_absorption += 15;
    }
    if (m_skills[ skill::IronSkin ].enabled)
    {
        m_damage_absorption += m_skills[ IronSkin ].value[ atIdx::Current ] / 4 + 5;
    }
    m_damage_absorption += m_bonus.absorption;
}

void character::recomputeConditionPenalty(struct character::bonus *b)
{
    for (int k = 0; k < CONDITION_SIZE; k++ )
    {
        if (m_conditions[ k ])
        {
            switch ( k )
            {
                case condition::Irritated:
                    b->MAR        -= 2;
                    b->conditions -= 2;
                    break;
                case condition::Hexed:
                    b->MAR -= 5;
                    for (int j=0; j<ATTRIBUTE_SIZE; j++)
                        b->attributes[j] -= 20;
                    for (int j=0; j<SKILL_SIZE; j++)
                        b->skills[j] -= 20;
                    break;
                case condition::Nauseated:
                    b->MAR        -= 5;
                    b->conditions -= 4;
                    break;
                case condition::Afraid:
                    b->MAR        -= 3;
                    b->conditions -= 2;
                    break;
                case condition::Slowed:
                    b->attributes[ attribute::Speed ] -= 50;
                    break;
                case condition::Poisoned:
                    b->MAR        -= 2;
                    b->conditions -=  2;
                    b->unknown_0x1778 += m_poison_strength;
                    break;
                case condition::Blind:
                    if (m_profession == Monk)
                    {
                        if (m_currentLevels[ m_profession ] < 20)
                            b->attributes[ attribute::Senses ] += (int)(((double)m_currentLevels[ m_profession ]*2 + 60) * 50.0 * 0.0099999998) - 50;
                    }
                    else
                    {
                        b->attributes[ attribute::Senses ] -= 50;
                        b->incapacitated = 1;
                    }
                    break;
                case condition::Webbed:
                    b->attributes[ attribute::Dexterity ] -= 50;
                    // don't break here -- fall through
                case condition::Asleep:
                case condition::Paralyzed:
                case condition::Unconscious:
                    b->incapacitated = 1;
                    break;
                case condition::Insane:
                    b->attributes[ attribute::Intelligence ] -= 50;
                    break;
                case condition::Missing:
                    // Simplified from the game logic, because it isn't possible to
                    // save a game mid combat, so we don't have to deal with the monster
                    // swallowing a character scenario (only Croc kidnapping and Mook visiting)
                    if (m_unknown_0x1830)
                        b->hp_regen -= 5;
                    break;
                default:
                    break;
            }
        }
    }
}

int character::getMagicResistance(character::realm r) const
{
    int mr = 0;

    if (m_skills[ skill::IronWill ].enabled)
        mr +=  m_skills[ skill::IronWill ].value[ atIdx::Current ] / 5 + 5;

    if (m_profession == profession::Mage)
        mr += 5;

    switch (m_race)
    {
        case race::Human:
            break;

        case race::Elf:
          if (r == realm::Mental) mr +=  20;
          if (r == realm::Air   ) mr +=  10;
          break;

        case race::Dwarf:
          if (r == realm::Fire  ) mr += m_attribs[ attribute::Vitality ].value[ atIdx::Base ] / 5;
          break;

        case race::Gnome:
          if (r == realm::Mental) mr += m_attribs[ attribute::Vitality ].value[ atIdx::Base ] / 5;
          if (r == realm::Earth ) mr +=  10;
          break;

        case race::Hobbit:
          if (r == realm::Earth ) mr += m_attribs[ attribute::Vitality ].value[ atIdx::Base ] / 5;
          break;

        case race::Faerie:
          if (r == realm::Air   ) mr +=  15;
          if (r == realm::Earth ) mr +=  15;
          if (r == realm::Mental) mr +=  15;
          if (r == realm::Divine) mr +=  15;
          break;

        case race::Lizardman:
          if (r == realm::Water ) mr +=  10;
          if (r == realm::Earth ) mr +=  10;
          if (r == realm::Fire  ) mr +=  15;
          if (r == realm::Divine) mr -=  10;
          if (r == realm::Mental) mr -=  10;
          break;

        case race::Dracon:
          if (r == realm::Water ) mr +=  15;
          if (r == realm::Air   ) mr +=   5;
          if (r == realm::Divine) mr -=   5;
          if (r == realm::Mental) mr -=   5;
          break;

        case race::Felpurr:
          if (r == realm::Water ) mr -=  15;
          if (r == realm::Earth ) mr +=  10;
          if (r == realm::Air   ) mr +=  10;
          if (r == realm::Mental) mr +=  10;
          break;

        case race::Rawulf:
          if (r == realm::Water ) mr +=  10;
          if (r == realm::Earth ) mr +=   5;
          if (r == realm::Divine) mr +=  15;
          break;

        case race::Mook:
          if (r == realm::Divine) mr +=  10;
          if (r == realm::Water ) mr +=  15;
          if (r == realm::Mental) mr +=  15;
          break;

        case race::Trynnie:
          if (r == realm::Earth ) mr +=  20;
          if (r == realm::Air   ) mr +=  20;
          if (r == realm::Mental) mr -=  10;
          break;

        case race::TRang:
          if (r == realm::Mental) mr +=  15;
          if (r == realm::Water ) mr +=  15;
          break;

        case race::Umpani:
          if (r == realm::Earth ) mr +=  15;
          if (r == realm::Air   ) mr +=  15;
          break;

        case race::Rapax:
          if (r == realm::Fire  ) mr +=  45;
          if (r == realm::Mental) mr -=  20;
          break;

        case race::Android:
          if (r == realm::Mental) mr +=  75;
          if (r == realm::Divine) mr -=  19;
          break;
    }

    if (m_attribs[ attribute::Intelligence ].value[ atIdx::Current ] > 80)
    {
        if (r == realm::Mental)
            mr += (m_attribs[ attribute::Intelligence ].value[ atIdx::Current ] - 80) >> 1;
    }

    if (m_attribs[ attribute::Piety ].value[ atIdx::Current ] > 80)
    {
        if (r == realm::Divine)
            mr += (m_attribs[ attribute::Piety ].value[ atIdx::Current ] - 80) >> 1;
    }
    return mr;
}

void character::recomputeMagicResistance()
{
    for (int k = 0; k < REALM_SIZE; k++)
    {
        m_magic_resistance[ realm::Fire + k ][atIdx::Base] = m_skills[ skill::FireMagic + k ].value[ atIdx::Current ] / 10 + 25;

        m_magic_resistance[ realm::Fire + k ][atIdx::Base] += getMagicResistance( static_cast<character::realm>(realm::Fire + k) );

        if (m_magic_resistance[realm::Fire + k][atIdx::Base] > 100)
            m_magic_resistance[realm::Fire + k][atIdx::Base] = 100;

        m_magic_resistance[realm::Fire + k][atIdx::Current] = m_magic_resistance[realm::Fire + k][atIdx::Base] +
                                                              m_bonus.allmagic_resist +
                                                              m_bonus.magic_resistance[k];

        if (m_magic_resistance[realm::Fire + k][atIdx::Current] > 100)
            m_magic_resistance[realm::Fire + k][atIdx::Current] = 100;
    }
}

bool character::isPureCaster() const
{
    switch (m_profession)
    {
        case profession::Priest:
        case profession::Alchemist:
        case profession::Bishop:
        case profession::Psionic:
        case profession::Mage:
            return true;

        default:
            break;
    }
    return false;
}

bool character::isHybridCaster() const
{
    switch (m_profession)
    {
        case profession::Lord:
        case profession::Valkyrie:
        case profession::Ranger:
        case profession::Samurai:
        case profession::Ninja:
        case profession::Monk:
            return true;

        default:
            break;
    }
    return false;
}

bool character::isRPC() const
{
    if (m_rpc_id == -1)
        return false;

    return true;
}
