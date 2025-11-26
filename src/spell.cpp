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

#include <spell.h>
#include "main.h"

#include <QFile>

#include "character.h"
#include "constants.h"
#include "common.h"
#include "Localisation.h"

#include <QDebug>

spell::spell(quint32 id) :
    m_id(id)
{
    m_helper = dbHelper::getHelper();

    m_db_record = m_helper->getSpellRecord(m_id);
    if (m_db_record.size() == 0)
    {
        m_id = 0xffffffff;
    }
}

// 0x019c: Spell name (maximum length unknown)
QString spell::getName() const
{
    Localisation *loc = Localisation::getLocalisation();

    return loc->getSpellName( m_id );
}

QString spell::getDesc() const
{
    Localisation *loc = Localisation::getLocalisation();

    return loc->getSpellDesc( m_id );
}

// 0x014a--0x014d: Spell cost
int spell::getSPCost() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return FORMAT_LE32(data+0x14a);
}

// 0x0157--0x015a: Spell level
int spell::getLevel() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return FORMAT_LE32(data+0x157);
}

int spell::getLevelAsPureClass() const
{
    int class_level = 0;

    switch (getLevel())
    {
        case 1: class_level =  1; break;
        case 2: class_level =  3; break;
        case 3: class_level =  5; break;
        case 4: class_level =  8; break;
        case 5: class_level = 11; break;
        case 6: class_level = 14; break;
        case 7: class_level = 18; break;
    }
    return class_level;
}

int spell::getLevelAsHybridClass() const
{
    int class_level = 0;

    switch (getLevel())
    {
        case 1: class_level =  5; break;
        case 2: class_level =  7; break;
        case 3: class_level =  9; break;
        case 4: class_level = 12; break;
        case 5: class_level = 15; break;
        case 6: class_level = 18; break;
        case 7: class_level = 22; break;
    }
    return class_level;
}

// 0x0152--0x0155: Spell damage
void spell::getDamage(quint16 *min_damage, quint16 *max_damage) const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    if (FORMAT_LE32(data+0x248)) // spell that causes damage
    {
        quint16 c          = FORMAT_LE16(data+0x152);
        quint8  num_dice   = data[0x154];
        quint8  dice_sides = data[0x155];

        if (min_damage)
            *min_damage = c + num_dice * 1;
        if (max_damage)
            *max_damage = c + num_dice * dice_sides;
    }
    else
    {
        if (min_damage)
            *min_damage = 0;
        if (max_damage)
            *max_damage = 0;
    }
}

void spell::getDuration(qint32 *base, qint32 *per_pl) const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    qint32 const_duration = FORMAT_LE32(data+0x14e);

    // const_duration == number of rounds, but:
    // 0x2D0 (720) equates to 24 hours

    qint32 per_pl_duration = FORMAT_LE32(data+0x145);

    // per_pl_duration == number of rounds, but:
    // 0x1e (30) equates to 5 minutes
    // 0x78 (120) equates to 4 hours

    if (base)
        *base = const_duration;
    if (per_pl)
        *per_pl = per_pl_duration;
}

// 0x0230--0x0233: Spell realm
item::range spell::getRange() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return static_cast<item::range>(FORMAT_LE32(data+0x230));
}

QString spell::getRangeString() const
{
    return ::getBaseStringTable()->getString( StringList::LISTRanges + static_cast<int>(getRange() ) );
}

// 0x0234--0x0237: Spell realm
character::realm spell::getRealm() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return static_cast<character::realm>(FORMAT_LE32(data+0x234));
}

// 0x0238--0x023b: Spell target
spell::target spell::getTarget() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return static_cast<spell::target>(FORMAT_LE32(data+0x238));
}

QString spell::getTargetString() const
{
    spell::target t = getTarget();

    // Both of these get represented as just "Adventuring"
    if (t == AdventuringTraps)
        t = Adventuring;

    return ::getBaseStringTable()->getString( StringList::LISTTargets + static_cast<int>( t ) );
}

// 0x023c--0x023f: Spell usability
spell::usable spell::getUsability() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return static_cast<spell::usable>(FORMAT_LE32(data+0x23c));
}

QString spell::getUsabilityString() const
{
    spell::usable u = getUsability();

    switch (u)
    {
        case spell::usable::AnyTime:
            return ::getBaseStringTable()->getString( StringList::Anytime );

        case spell::usable::Combat:
            return ::getBaseStringTable()->getString( StringList::Combat );

        case spell::usable::NonCombat:
        case spell::usable::NonCombatNPCInteraction:
        case spell::usable::NonCombatTraps:
            return ::getBaseStringTable()->getString( StringList::NonCombat );
    }
    return "";
}

// 0x0149: Alchemist
// 0x015b: Mage
// 0x0220: Priest
// 0x0221: Psionic
character::professions spell::getClasses() const
{
    character::professions profs = {};
    quint8 *data = (quint8 *)m_db_record.constData();

    if (data[0x149] == 0x01)
    {
        profs |= character::profession::Alchemist | character::profession::Ranger | character::profession::Ninja | character::profession::Bishop;
    }
    if (data[0x15b] == 0x01)
    {
        profs |= character::profession::Mage | character::profession::Samurai | character::profession::Bishop;
    }
    if (data[0x220] == 0x01)
    {
        profs |= character::profession::Priest | character::profession::Lord | character::profession::Valkyrie | character::profession::Bishop;
    }
    if (data[0x221] == 0x01)
    {
        profs |= character::profession::Psionic | character::profession::Monk | character::profession::Bishop;
    }

    return profs;
}
