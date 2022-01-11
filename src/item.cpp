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

#include <item.h>
#include "main.h"

#include <QFile>
#include <QTemporaryFile>

#include "character.h"
#include "constants.h"
#include "common.h"
#include "spell.h"

#include <QDebug>

/* XPM - Pinata icon taken from Emojipedia: https://emojipedia.org/emojipedia/13.0/pinata/
 */
#include "pinata.xpm"

item::item(quint32 id, quint8 cnt, quint8 charges, quint8 identified, quint8 uncursed, bool equipped) :
    m_id(id),
    m_cnt(cnt),
    m_charges(charges),
    m_identified(identified),
    m_uncursed(uncursed),
    m_equipped(equipped)
{
    m_helper = dbHelper::getHelper();

    m_db_record = m_helper->getItemRecord(m_id);
}

// 0x0000: Item name (maximum length unknown)
QString item::getName() const
{
    // The item name is a UTF16 string at the start of the item record.
    // Maximum length is 30 - "Potion of Cure Light Condition", which isn't
    // NULL terminated if it gets that long

    // Don't make any assumptions about the endianness of the host being LE
    // ie. can't just do:
    // quint16 *data = (quint16 *)m_db_record.constData();
    // return QString::fromUtf16(data);

    QString name = "";

    quint8 *data = (quint8 *)m_db_record.constData();
    for (int k=0; k<0x3c; k+=2)
    {
        quint16 c = FORMAT_LE16(data+k);

        if (c == 0)
            break;

        name += QChar(c);
    }

    return name;
}

QString item::getDesc() const
{
    return m_helper->getItemDesc( m_id );
}

// 0x003c--0x003d: UNKNOWN

// 0x003e: Type of item
item::type item::getType() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return (item::type)data[0x3e];
}
QString item::getTypeString() const
{
    return ::getStringTable()->getString( StringList::LISTItemTypes + static_cast<int>( getType() ) );
}

// 0x003f--0x0040: appearance (icon) of item
QString item::getStiFile() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    int sti_idx = FORMAT_LE16(data+0x3f);

    // The default sti file is item_stis[ sti_idx ]
    // which comes from a list hardcoded in Wiz.exe itself. There's no
    // simple way to gather it from the executable at runtime without
    // tying it to a specific version of the exe, so it's been extracted
    // for use here.
    //
    // The default is not used if an ASCII string at offset 0xcd specifies
    // something else.

    if (! data[0xcd])
        return item_stis[ sti_idx ];

    // This relies on null termination, which may not be valid for longest items,
    // but it's also the last item I know about in the item database, so if it isn't
    // null terminated it should fill the remainder of the array completely, and still
    // work. Issue is that if this isn't the last field...

    return QString::fromLatin1((char *)data + 0xcd) + ".sti";
}

// Wizardry 8 bases a number of considerations on the sti_idx in addition
// to the default image that is shown. This includes whether an item requires
// strength from the user in order to operate (eg. Sword that does versus
// rocket launcher that doesn't), and whether it is the ammunition that imparts
// the damage, or the launcher itself. It also hardcodes the Omnigun and rocket
// launcher to specific ids for special treatment in a few areas. If these are
// relocated by a mod to alternate ids, they will break these hard codings within
// the game.
bool item::weaponRequiresStrength() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    int sti_idx = FORMAT_LE16(data+0x3f);

    switch (sti_idx)
    {
        case 0x68: // "QuestionMark.sti"
        case 0x6e: // "QuestionMark.sti"
        case 0x72: // "Musket.sti"
        case 0x83: // "Omnigun_1.sti"
        case 0x90: // "rocketlauncher.sti"
            return false;
    }
    return true;
}

bool item::isOmnigun() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    int sti_idx = FORMAT_LE16(data+0x3f);

    switch (sti_idx)
    {
        case 0x83: // "Omnigun_1.sti"
            return true;
    }
    return false;
}

bool item::isOneShotMaximum() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    int sti_idx = FORMAT_LE16(data+0x3f);

    switch (sti_idx)
    {
        case 0x90: // "rocketlauncher.sti"
            return true;
    }
    return false;
}

bool item::damageComesFromAmmunition() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    int sti_idx = FORMAT_LE16(data+0x3f);

    switch (sti_idx)
    {
        case 0x0b: // "LongBow.sti"
        case 0x0c: // "Crossbow.sti"
        case 0x0d: // "Sling.sti"
        case 0x25: // "GreatBow.sti"
        case 0x83: // "Omnigun_1.sti"
            return true;
    }
    return false;
}

bool item::compatibleAmmunition( item ammo ) const
{
    quint8 *ranged_data = (quint8 *)m_db_record.constData();
    quint8 *ammo_data   = (quint8 *)ammo.m_db_record.constData();

    int ranged_sti_idx = FORMAT_LE16(ranged_data+0x3f);
    int ammo_sti_idx   = FORMAT_LE16(ammo_data+0x3f);

    switch (ranged_sti_idx)
    {
        case 0x0b: // "LongBow.sti"
        case 0x25: // "GreatBow.sti"
            if (ammo_sti_idx != 0x16) // "Arrow.sti"
                return false;
            return true;

        case 0x0d: // "Sling.sti"
            if (ammo_sti_idx != 0x17) // "SlingShot.sti"
                return false;
            return true;

        case 0x0c: // "Crossbow.sti"
            if (ammo_sti_idx != 0x53) // "Quarrel.sti"
                return false;
            return true;

        case 0x68: // "QuestionMark.sti"
        case 0x6E: // "QuestionMark.sti"
        case 0x72: // "Musket.sti"
        {
            item::type t = getType();

            if ((t == ShortWeapon)    ||
                (t == ExtendedWeapon) ||
                (t == ThrownWeapon)   ||
                (t == RangedWeapon))
            {
                if (getRange() != ammo.getRange())
                    return false;
            }
            if (ranged_sti_idx == 0x72) // "Musket.sti"
            {
                if (ammo_sti_idx != 0x71) // "PowderShot.sti"
                    return false;
            }
            else // the 2 question mark weapons
            {
                if (ammo_sti_idx != 0x84) // "QuestionMark.sti"
                    return false;
            }
            return true;
        }

        case 0x83: // "Omnigun_1.sti"
            // As you can see the omnigun ids are hard-coded so you'll mess up any mod
            // that tries to allocate other items in this range, or move the omnigun
            // somewhere else
            switch (m_id)
            {
                case 0x262:
                case 0x261:
                case 0x260:
                case 0x25f:
                case 0x25e:
                    if ((ammo_sti_idx == 0x16) || // "Arrow.sti"
                        (ammo_sti_idx == 0x53))   // "Quarrell.sti"
                    {
                        return true;
                    }
                    // don't break

                case 0x25d:
                case 0x25c:
                case 0x25b:
                case 0x25a:
                    if ((ammo_sti_idx == 0x15) || // "Shuriken.sti"
                        (ammo_sti_idx == 0x72) || // "Shuriken2.sti"
                       ((ammo_sti_idx == 0x00) && // "Dagger.sti"
                        (ammo.getType() == item::type::ThrownWeapon)))
                    {
                        return true;
                    }
                    // don't break

                case 0x259:
                case 0x258:
                    if  (ammo_sti_idx == 0x70)    // "Dart.sti"
                        return true;
                    // don't break

                case 0x257:
                    if ((ammo_sti_idx == 0x17) || // "SlingShot.sti"
                        (ammo_sti_idx == 0x71))   // "PowderShot.sti"
                    {
                        return true;
                    }
                    return false;
            }
            return true;
    }
    return false;
}

// 0x0041:
//  00000001 DOESN'T need identify
//  00000010 Is Critical Item
//  00000100 2 Hands
//  00001000 Usable as secondary weapon
//  00010000 Persists in shops
//  00100000 Only one thing uses this: bag of items (0x21) - don't think it can _be_ in inventory though
//  10000000 Only one thing uses this: boomerang shuriken (0x88) - think it means "auto replenishes"
/*
bool item::needsIdentify() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    if (data[0x41] & 1) // DOESN'T need identify -- reversed flag
        return false;
    return true;
}
bool item::isCriticalItem() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    if (data[0x41] & 2)
        return true;
    return false;
}
*/
bool item::needs2Hands() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    if (data[0x41] & 4)
        return true;
    return false;
}
bool item::canSecondary() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    if (data[0x41] & 8)
        return true;
    return false;
}
/*
bool item::isShopPersist() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    if (data[0x41] & 16)
        return true;
    return false;
}
bool item::autoReplenishes() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    if (data[0x41] & 128)
        return true;
    return false;
}
*/

// 0x0042: Spell usage type
item::spell_usage_type item::getSpellUsageType() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return static_cast<item::spell_usage_type>(data[0x42]);
}

// 0x0043: UNKNOWN

// 0x0044--0x0045: 00 00 for all items

// 0x0046: Usage Skill - ie. the one which makes you better at using it and trains if you use it
character::skill item::getSkillUsed() const
{
    qint8 *data = (qint8 *)m_db_record.constData();

    return static_cast<character::skill> (data[0x46]);
}
QString item::getSkillUsedString() const
{
    QString skills = "";

    if (! getSpell().isNull())
    {
        // implied skills
        switch (getSpellUsageType())
        {
            case item::spell_usage_type::None:
                break;

            case item::spell_usage_type::Instrument:
                skills = ::getStringTable()->getString( StringList::LISTSkills + static_cast<int>( character::skill::Music ) );
                break;

            case item::spell_usage_type::Gadget:
                skills = ::getStringTable()->getString( StringList::LISTSkills + static_cast<int>( character::skill::Engineering ) );
                break;

            case item::spell_usage_type::Item:
            case item::spell_usage_type::Scroll:
            case item::spell_usage_type::Spellbook:
            case item::spell_usage_type::Food:
            case item::spell_usage_type::Drink_Potion:
            case item::spell_usage_type::Bomb_Powder:
            case item::spell_usage_type::MiscMagic:
                skills = ::getStringTable()->getString( StringList::LISTSkills + static_cast<int>( character::skill::Artifacts ) );
                break;
        }
    }

    character::skill k = getSkillUsed();

    if (k == character::skill::SKILL_NONE)
        return skills;

    if (skills.size() > 0)
        skills += ", ";

    return skills + ::getStringTable()->getString( StringList::LISTSkills + static_cast<int>(k) );
}

// 0x0047: Range
item::range item::getRange() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return (item::range)data[0x47];
}
QString item::getRangeString() const
{
    return ::getStringTable()->getString( StringList::LISTRanges + static_cast<int>(getRange() ) );
}

// 0x0048: Initiative +/-
int item::getInitiative() const
{
    // Can be negative
    qint8 *data = (qint8 *)m_db_record.constData();

    return (data[0x48]);
}

// 0x0049: ToHit +/-
int item::getToHit() const
{
    // Can be negative
    qint8 *data = (qint8 *)m_db_record.constData();

    return (data[0x49]);
}

// 0x004a--0x004d: Damage (constant + dice)
void item::getDamage(quint16 *min_damage, quint16 *max_damage, int *percentage) const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    quint16 c          = FORMAT_LE16(data+0x4a);
    quint8  num_dice   = data[0x4c];
    quint8  dice_sides = data[0x4d];

    if (min_damage)
        *min_damage = c + num_dice * 1;
    if (max_damage)
        *max_damage = c + num_dice * dice_sides;

    // The bows store a percentage in here instead,
    // eg +10%, +20%, +30%, and they do this by
    // storing a 1, 2 or 3 in c, and the 0 into
    // each of num_dice and dice_sides

    if (percentage)
    {
        // Theoretically I guess the percentage could be negative, but
        // haven't seen this anywhere
        *percentage = 0;
        if ((num_dice == 0) && (dice_sides == 0))
        {
            *percentage = c * 10;
            if (min_damage)
                *min_damage = 0;
            if (max_damage)
                *max_damage = 0;
        }
    }
}

// 0x004e--0x004f: attack types supported
item::attacks item::getAttacks() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return (item::attacks)FORMAT_LE16(data+0x4e);
}
QString item::getAttacksString() const
{
    QString   list;

    QMetaEnum metaAttack = QMetaEnum::fromType<item::attack>();

    item::attacks a = getAttacks();

    for (int k=0; k<metaAttack.keyCount(); k++)
    {
        if (a & metaAttack.value(k))
            list += QString(", ") + ::getStringTable()->getString( StringList::LISTAttacks + k*2  );
    }

    return list.mid(2);
}

// 0x0050: Percentage chance of inflicting Sleep
// 0x0051: Percentage chance of inflicting Paralysis
// 0x0052: Percentage chance of inflicting Poison
// 0x0053: Percentage chance of inflicting Hex
// 0x0054: Percentage chance of inflicting Disease
// 0x0055: Percentage chance of inflicting Kill
// 0x0056: Percentage chance of inflicting Knock Out (K.O.)
// 0x0057: Percentage chance of inflicting Blinding
// 0x0058: Percentage chance of inflicting Frightening
// 0x0059: Percentage chance of inflicting Swallow
// 0x005a: Percentage chance of inflicting Possession
// 0x005b: Percentage chance of inflicting Drain HP
// 0x005c: Percentage chance of inflicting Drain Stamina
// 0x005d: Percentage chance of inflicting Drain Spell Points
// 0x005e: Percentage chance of inflicting Nauseate
// 0x005f: Percentage chance of inflicting Insanity
// 0x0060: Poison Strength
item::special_attacks item::getSpecialAttack() const
{
    quint8 *data = (quint8 *)m_db_record.constData();
    item::special_attacks list = {};

    if (data[0x50])
        list |= special_attack::Sleep;
    if (data[0x51])
        list |= special_attack::Paralyze;
    if (data[0x52])
        list |= special_attack::Poison;
    if (data[0x53])
        list |= special_attack::Hex;
    if (data[0x54])
        list |= special_attack::Disease;
    if (data[0x55])
        list |= special_attack::Kill;
    if (data[0x56])
        list |= special_attack::KO;
    if (data[0x57])
        list |= special_attack::Blind;
    if (data[0x58])
        list |= special_attack::Frighten;
    if (data[0x59])
        list |= special_attack::Swallow;
    if (data[0x5a])
        list |= special_attack::Possess;
    if (data[0x5b])
        list |= special_attack::DrainHP;
    if (data[0x5c])
        list |= special_attack::DrainStamina;
    if (data[0x5d])
        list |= special_attack::DrainSP;
    if (data[0x5e])
        list |= special_attack::Nauseate;
    if (data[0x5f])
        list |= special_attack::Insane;

    return list;
}
int item::getPercentageChance(item::special_attack a) const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    QMetaEnum metaAttack = QMetaEnum::fromType<item::special_attack>();

    for (int k=0; k<metaAttack.keyCount(); k++)
    {
        if (a == metaAttack.value(k))
            return data[0x50 + k];
    }
    return -1;
}
int item::getPoisonStrength() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return data[0x50];
}
QString item::getSpecialAttackString() const
{
    QString list = "";

    QMetaEnum metaAttack = QMetaEnum::fromType<item::special_attack>();

    item::special_attacks a = getSpecialAttack();

    for (int k=0; k<metaAttack.keyCount(); k++)
    {
        if (a & metaAttack.value(k))
        {
            list += QString( ", %1 %2%").arg( ::getStringTable()->getString( StringList::LISTSpecialAttacks + k ) ).arg( getPercentageChance( static_cast<item::special_attack>( metaAttack.value(k) ) ) );
            if (k == special_attack::Poison)
            {
                list += QString( tr(" (STR %3)") ).arg( getPoisonStrength() );
            }
        }
    }

    return list.mid(2);
}

// 0x0061: Slays (item::slays)
item::slays item::getSlays() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return (item::slays) data[0x61];
}
QString item::getSlaysString() const
{
    item::slays k = getSlays();

    if (k == item::slays::None)
        return QString();

    return ::getStringTable()->getString( StringList::LISTSlays + static_cast<int>(k) );
}

// 0x0062: Armor Class
int item::getAC() const
{
    // Can be negative
    qint8 *data = (qint8 *)m_db_record.constData();

    return data[0x62];
}

// 0x0063: Contains Spell (0x00 = None)
// 0x0064: Spell Power
const spell item::getSpell(int *power) const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    int spell_id = data[0x63];

    if ((spell_id != 0 /* None */) && (power != NULL))
        *power = (int)data[0x64];

    return spell(spell_id);
}
// 0x0065: UNKNOWN

// 0x0066: solo item  = 0x00,
//         is stackable = 0x01.
//         has charges = 0x02,
//         has uses = 0x03
//         has shots = 0x04
bool item::isStackable() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    if (data[0x66] == 0x01)
        return true;

    return false;
}
bool item::hasCharges() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    if (data[0x66] == 0x02)
        return true;

    return false;
}
bool item::hasUses() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    if (data[0x66] == 0x03)
        return true;

    return false;
}
bool item::hasShots() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    if (data[0x66] == 0x04)
        return true;

    return false;
}

// 0x0067-0x006a: charges for spell in item (constant + dice) - found quantity
int item::getMaxCharges() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    quint16 c          = FORMAT_LE16(data+0x67);
    quint8  num_dice   = data[0x69];
    quint8  dice_sides = data[0x6a];

    //min_charges = c + num_dice * 1;
    //max_charges = c + num_dice * dice_sides;

    // We only want the maximum
    return (c + num_dice * dice_sides);
}


// 0x006b: Max in stack
quint8 item::getMaxStackSize() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return data[0x6b];
}

// 0x006c: HP Regen
int item::getHPRegen() const
{
    // Can be negative
    qint8 *data = (qint8 *)m_db_record.constData();

    return (int)data[0x6c];
}

// 0x006d: Stamina Regen
int item::getStaminaRegen() const
{
    // Can be negative
    qint8 *data = (qint8 *)m_db_record.constData();

    return (int)data[0x6d];
}

// 0x006e: Spell Points Regen
int item::getSPRegen() const
{
    // Can be negative
    qint8 *data = (qint8 *)m_db_record.constData();

    return (int)data[0x6e];
}

// 0x006f: Fire Resistance Percent
// 0x0070: Water Resistance Percent
// 0x0071: Air Resistance Percent
// 0x0072: Earth Resistance Percent
// 0x0073: Mental Resistance Percent
// 0x0074: Divine Resistance Percent
bool item::getResistance(int *fire, int *water, int *air, int *earth, int *mental, int *divine) const
{
    qint8 *data = (qint8 *)m_db_record.constData();

    if ((data[0x6f] == 0) &&
        (data[0x70] == 0) &&
        (data[0x71] == 0) &&
        (data[0x72] == 0) &&
        (data[0x73] == 0) &&
        (data[0x74] == 0))
    {
        return false;
    }

    if (fire)
        *fire = data[0x6f];
    if (water)
        *water = data[0x70];
    if (air)
        *air = data[0x71];
    if (earth)
        *earth = data[0x72];
    if (mental)
        *mental = data[0x73];
    if (divine)
        *divine = data[0x74];

    return true;
}

// 0x0075: Armor Weight Class
item::weight item::getArmorWeightClass() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return (item::weight) data[0x75];
}
QString item::getArmorWeightClassString() const
{
    item::weight k = getArmorWeightClass();

    // FIXME: do we want this or is it unneeded? without it an empty string comes back for None
    if (k == item::weight::None)
        return tr("None");

    return ::getStringTable()->getString( StringList::LISTArmorWeights + static_cast<int>(k) );
}

// 0x0076: usable by professions
character::professions item::getUsableProfessions() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return (character::professions) FORMAT_LE16(data + 0x76);
}

// 0x0077: UNKNOWN

// 0x0078: usable by races
character::races item::getUsableRaces() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return (character::races) FORMAT_LE16(data + 0x78);
}

// 0x0079: UNKNOWN

// 0x007a-0x007b: 00 00 for every single item

// 0x007c: usable by gender
character::genders item::getUsableGenders() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return (character::genders) data[0x7c];
}

// 0x007d--0x007e: requires attrib (STR INT PIE VIT DEX SPD SEN) at level %d
// 0x007f--0x0080: requires attrib (STR INT PIE VIT DEX SPD SEN) at level %d
void item::getRequiredAttrib(int idx, character::attribute *attrib, qint32 *value) const
{
    qint8   *data = (qint8 *)m_db_record.constData();

    *attrib = static_cast<character::attribute> (data[0x7d + 2 * idx]);
    *value  = (qint32)data[0x7e + 2 * idx];
}

QString item::getRequiredAttribsString() const
{
    QString   attribs = "";

    character::attribute a1;
    character::attribute a2;
    qint32   v1;
    qint32   v2;

    getRequiredAttrib( 0, &a1, &v1);
    getRequiredAttrib( 1, &a2, &v2);

    if (a1 != character::attribute::ATTRIBUTE_NONE)
    {
        attribs += QString(", %1 %2").arg( ::getStringTable()->getString(StringList::LISTPrimaryAttributes + static_cast<int>(a1)) ).arg( v1 );
    }
    if (a2 != character::attribute::ATTRIBUTE_NONE)
    {
        attribs += QString(", %1 %2").arg( ::getStringTable()->getString(StringList::LISTPrimaryAttributes + static_cast<int>(a2)) ).arg( v2 );
    }

    return attribs.mid( 2 );
}

// 0x0081--0x0082: requires skill (character::skill) at level %d
// 0x0083--0x0084: requires skill (character::skill) at level %d
void item::getRequiredSkill(int idx, character::skill *skill, qint32 *value) const
{
    qint8   *data = (qint8 *)m_db_record.constData();

    *skill = static_cast<character::skill> (data[0x81 + 2 * idx]);
    *value = (qint32)data[0x82 + 2 * idx];
}

QString item::getRequiredSkillsString() const
{
    QString   skills = "";

    character::skill s1;
    character::skill s2;
    qint32   v1;
    qint32   v2;

    getRequiredSkill( 0, &s1, &v1);
    getRequiredSkill( 1, &s2, &v2);

    if (s1 != character::skill::SKILL_NONE)
    {
        skills += QString(", %1 %2").arg( ::getStringTable()->getString(StringList::LISTSkills + static_cast<int>(s1)) ).arg( v1 );
    }
    if (s2 != character::skill::SKILL_NONE)
    {
        skills += QString(", %1 %2").arg( ::getStringTable()->getString(StringList::LISTSkills + static_cast<int>(s2)) ).arg( v2 );
    }

    spell item_spell = getSpell();
    if ((s1 == character::skill::Music) || (s2 == character::skill::Music))
    {
        // If it requries Music, it's also going to require a certain Bard Level
        skills += QString( ", %1\u00a0%2" ).arg(::getStringTable()->getString( StringList::BardLevel )).arg( item_spell.getLevelAsPureClass() );
    }

    if ((s1 == character::skill::Engineering) || (s2 == character::skill::Engineering))
    {
        // If it requries Engineering, it's also going to require a certain Gadgeteer Level
        skills += QString( ", %1\u00a0%2" ).arg(::getStringTable()->getString( StringList::GadgeteerLevel )).arg( item_spell.getLevelAsPureClass() );
    }

    // The real game doesn't show this because it is VERY messy and complicated due
    // to the AND/OR conditions, but including it here anyway.
    if (getType() == item::type::Spellbook)
    {
        // This is messy because of all the this or that for the
        // different classes.
        character::professions profs = item::getUsableProfessions();

        QString prof_reqs = "";
        QString pure_users = "";
        QString hybrid_users = "";

        QMetaEnum metaProf = QMetaEnum::fromType<character::profession>();

        for (int k=0; k<metaProf.keyCount(); k++)
        {
            character::profession p = static_cast<character::profession>( metaProf.value(k) );

            if (profs & p)
            {
                switch (p)
                {
                    // -- these are non-magic users that aren't expected to ever
                    //    be listed in the professions for a spellbook
                    case character::profession::Fighter:
                    case character::profession::Rogue:
                    case character::profession::Gadgeteer:
                    case character::profession::Bard:

                    // Pure magic users
                    case character::profession::Priest:
                    case character::profession::Alchemist:
                    case character::profession::Bishop:
                    case character::profession::Psionic:
                    case character::profession::Mage:
                        pure_users += "/" + ::getStringTable()->getString( StringList::LISTProfessions + k );
                        break;

                    // Hybrid Fighter & magic users
                    case character::profession::Lord:
                    case character::profession::Valkyrie:
                    case character::profession::Ranger:
                    case character::profession::Samurai:
                    case character::profession::Ninja:
                    case character::profession::Monk:
                        hybrid_users += "/" + ::getStringTable()->getString( StringList::LISTProfessions + k );
                        break;
                }
            }
        }
        if (pure_users.size() > 0)
        {
            prof_reqs += tr(" or ") + pure_users.mid(1) + QString( " %1\u00a0%2").arg(::getStringTable()->getString( StringList::Level ))
                             .arg( item_spell.getLevelAsPureClass() );
        }
        if (hybrid_users.size() > 0)
        {
            prof_reqs += tr(" or ") + hybrid_users.mid(1) + QString( " %1\u00a0%2").arg(::getStringTable()->getString( StringList::Level ))
                             .arg( item_spell.getLevelAsHybridClass() );
        }
        skills += ", " + prof_reqs.mid( 4 );

        // This is even messier because of the interaction of the
        // spell school with the realm to produce a total.

        int  spell_level = item_spell.getLevel();

        if ((spell_level - 1) * 15 > 0)
        {
            int  spell_realm = static_cast<int>( item_spell.getRealm() ) + static_cast<int>( character::skill::FireMagic );

            QString school_reqs = "";

            // Don't seem to have a way to query the school of the spell, so
            // we re-use the professions above
            // NOTE that these are NOT else if conditions - each needs to be
            // evaluated
            if (profs & character::profession::Mage)
            {
                school_reqs += QString( tr(" or (%1 + %2\u00f710) %3"))
                                     .arg( ::getStringTable()->getString( StringList::LISTSkills + static_cast<int>( character::skill::Wizardry )) )
                                     .arg( ::getStringTable()->getString( StringList::LISTSkills + spell_realm) )
                                     .arg( (spell_level - 1)*15 );
            }
            if (profs & character::profession::Alchemist)
            {
                school_reqs += QString( tr(" or (%1 + %2\u00f710) %3"))
                                     .arg( ::getStringTable()->getString( StringList::LISTSkills + static_cast<int>( character::skill::Alchemy )) )
                                     .arg( ::getStringTable()->getString( StringList::LISTSkills + spell_realm) )
                                     .arg( (spell_level - 1)*15 );
            }
            if (profs & character::profession::Priest)
            {
                school_reqs += QString( tr(" or (%1 + %2\u00f710) %3"))
                                     .arg( ::getStringTable()->getString( StringList::LISTSkills + static_cast<int>( character::skill::Divinity )) )
                                     .arg( ::getStringTable()->getString( StringList::LISTSkills + spell_realm) )
                                     .arg( (spell_level - 1)*15 );
            }
            if (profs & character::profession::Psionic)
            {
                school_reqs += QString( tr(" or (%1 + %2\u00f710) %3"))
                                     .arg( ::getStringTable()->getString( StringList::LISTSkills + static_cast<int>( character::skill::Psionics )) )
                                     .arg( ::getStringTable()->getString( StringList::LISTSkills + spell_realm) )
                                     .arg( (spell_level - 1)*15 );
            }

            skills += ", " + school_reqs.mid( 4 );
        }
    }

    return skills.mid( 2 );
}

// 0x0085: identification requirements
//    Artefacts = 6 * curio ; Identify = int((curio+2)/3) (except 1 for 0th index)
//    0:  Artifacts   0;  Identify 1
//   20:  Artifacts 120;  Identify 7
void item::getIdentificationRequirement(int *artifacts, int *spell) const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    int id = (int)data[0x0085];

    if (artifacts != NULL)
    {
        if (id == 1)
            *artifacts = 1;
        else
            *artifacts = 6 * id;
    }

    if (spell != NULL)
    {
        *spell = (id + 2) / 3;
        if (*spell == 0)
            *spell = 1;
    }
}

// 0x0086--0x0089: Price
quint32 item::getPrice() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return FORMAT_LE32(data+0x86);
}

// 0x008a--0x008b: Weight in pounds
double item::getWeight() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    return (double)(FORMAT_LE16(data+0x8a)) / 10.0;
}

// 0x008c: cursed = 0x01, normal = 0x00 - no other values used
bool item::isCursed() const
{
    quint8 *data = (quint8 *)m_db_record.constData();

    if (data[0x8c] == 1)
        return true;
    return false;
}

// 0x008d--0x00ac: UNKNOWN

// 0x00ad: bonus number of swings
int item::getBonusSwings() const
{
    // Can be negative
    qint8 *data = (qint8 *)m_db_record.constData();

    return (int) data[0xad];
}

// 0x00ae--0x00b0: UNKNOWN

// 0x00b1--0x00b2: skill (character::skill) bonus/penalty +/-
character::skill item::getSkillBonus( int *bonus ) const
{
    qint8 *data = (qint8 *)m_db_record.constData();

    // Can be negative - we can't cast the data pointer above
    // because it interferes with the static cast below if it
    // isn't unsigned
    if (bonus)
        *bonus = (int)data[0xb2];

    return static_cast<character::skill>(data[0xb1]);
}

// 0x00b3--0x00b4: attrib (STR INT PIE VIT DEX SPD SEN) bonus/penalty +/-
character::attribute item::getAttributeBonus( int *bonus ) const
{
    qint8 *data = (qint8 *)m_db_record.constData();

    // Can be negative - we can't cast the data pointer above
    // because it interferes with the static cast below if it
    // isn't unsigned
    if (bonus)
        *bonus = (int)data[0xb4];

    return static_cast<character::attribute>(data[0xb3]);
}

quint32 item::getId() const
{
    return m_id;
}

quint8 item::getCount() const
{
    return m_cnt;
}

void item::setCount(quint8 count)
{
    m_cnt = count;
}

quint8 item::getCharges() const
{
    return m_charges;
}

void item::setCharges(quint8 charges)
{
    m_charges = charges;
}

bool item::isIdentified() const
{
    return (bool)m_identified;
}

bool item::isUncursed() const
{
    return (bool)m_uncursed;
}

bool item::isEquipped() const
{
    return m_equipped;
}

const QPixmap item::getMissingItemImage()
{
    return QPixmap( pinata );
}

// These aren't allowed to be member functions
QDataStream &operator<<(QDataStream &out, const item &i)
{
    i.serializeOut(out);
    return out;
}

QDataStream &operator>>(QDataStream &in, item &i)
{
    i.serializeIn(in);
    return in;
}

void item::serializeOut(QDataStream &out) const
{
    out << m_id
        << m_cnt
        << m_charges
        << m_identified
        << m_uncursed
        << m_db_record

        // Warning: this field needs manual update
        // to set value appropriate for new location
        << m_equipped;
}

void item::serializeIn(QDataStream &in)
{
    in  >> m_id
        >> m_cnt
        >> m_charges
        >> m_identified
        >> m_uncursed
        >> m_db_record

        // Warning: this field needs manual update
        // to set value appropriate for new location
        >> m_equipped;

    m_helper = dbHelper::getHelper();
}

// FIXME: Fairly incomplete. Doesn't support tr() macro yet among other things
QString item::getCompleteData(bool include_image) const
{
    QString html = "<html><body><table><tr>";

    if (include_image)
    {
        QPixmap pix;
        SLFFile imgs( "ITEMS/" + getStiFile().toUpper() );

        if (imgs.open(QFile::ReadOnly))
        {
            QByteArray array = imgs.readAll();
            STItoQImage sti_imgs( array );

            pix = QPixmap::fromImage( sti_imgs.getImage( 0 ));

            imgs.close();
        }
        else
        {
            // The item lacks an image - some mods have this problem; use our
            // generic replacement icon
            pix = QPixmap( item::getMissingItemImage() );
        }
        QTemporaryFile png;
        png.setFileTemplate( png.fileTemplate() + ".png" );

        // have to open the file before we can get the filename
        png.open();
        pix.save( png.fileName() );
        QByteArray pngData = png.readAll();
        png.close();

        html += "<th><img src=\"data:image/png;base64, " + pngData.toBase64() + "\"/></th>";
    }
    html += "<th>" + getName() + "</th></tr>";

    html += "<tr><td>Description</td><td>" + getDesc() + "</td></tr>";
    html += "<tr><td>Type</td><td>" + getTypeString() + "</td></tr>";

    // FIXME: paste into LibreOffice doesn't show checkbox set right, and it looks pretty awful
    // FIXME: not using StringLists either
    {
        character::professions profs = getUsableProfessions();
        html += "<tr><td>Usable by Professions</td><td><form><table><tr>";
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Fighter ) != 0 ).arg("Fighter");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Lord ) != 0 ).arg("Lord");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Valkyrie ) != 0 ).arg("Valkyrie");
        html += "</tr><tr>";
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Ranger ) != 0 ).arg("Ranger");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Samurai ) != 0 ).arg("Samurai");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Ninja ) != 0 ).arg("Ninja");
        html += "</tr><tr>";
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Monk ) != 0 ).arg("Monk");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Rogue ) != 0 ).arg("Rogue");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Gadgeteer ) != 0 ).arg("Gadgeteer");
        html += "</tr><tr>";
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Bard ) != 0 ).arg("Bard");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Priest ) != 0 ).arg("Priest");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Alchemist ) != 0 ).arg("Alchemist");
        html += "</tr><tr>";
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Bishop ) != 0 ).arg("Bishop");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Psionic ) != 0 ).arg("Psionic");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( profs & character::profession::Mage ) != 0 ).arg("Mage");
        html += "</tr></table></form></td></tr>";
    }
    {
        character::races races = item::getUsableRaces();
        html += "<tr><td>Usable by Races</td><td><form><table><tr>";
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Human ) != 0 ).arg("Human");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Elf ) != 0 ).arg("Elf");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Dwarf ) != 0 ).arg("Dwarf");
        html += "</tr><tr>";
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Gnome ) != 0 ).arg("Gnome");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Hobbit ) != 0 ).arg("Hobbit");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Faerie ) != 0 ).arg("Faerie");
        html += "</tr><tr>";
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Lizardman ) != 0 ).arg("Lizardman");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Dracon ) != 0 ).arg("Dracon");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Felpurr ) != 0 ).arg("Felpurr");
        html += "</tr><tr>";
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Rawulf ) != 0 ).arg("Rawulf");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Mook ) != 0 ).arg("Mook");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Trynnie ) != 0 ).arg("Trynnie");
        html += "</tr><tr>";
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::TRang ) != 0 ).arg("T'Rang");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Umpani ) != 0 ).arg("Umpani");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Rapax ) != 0 ).arg("Rapax");
        html += "</tr><tr>";
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( races & character::race::Android ) != 0 ).arg("Android");
        html += "<td></td><td></td>";
        html += "</tr></table></form></td></tr>";
    }
    {
        character::genders sexes = item::getUsableGenders();
        html += "<tr><td>Usable by Genders</td><td><form><table><tr>";
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( sexes & character::gender::Male ) != 0 ).arg("Male");
        html += QString("<td><input type=\"checkbox\" value=\"%1\"/><label>%2</label></td>").arg(( sexes & character::gender::Female ) != 0 ).arg("Female");
        html += "<td></td>";
        html += "</tr></table></form></td></tr>";
    }

    // FIXME: put in all the other attributes here too

// 0x0041:
    html += "</table></body></html>";

    return html;
}
