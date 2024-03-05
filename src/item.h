/*
 * Copyright (C) 20222024 Anonymous Idiot
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

#ifndef ITEM_H__
#define ITEM_H__

#include <QDataStream>
#include <QMutex>
#include <QByteArray>
#include <QPixmap>

#include "character.h"
#include "SLFFile.h"

#include "dbHelper.h"

class spell;

class item : QObject
{
    Q_OBJECT

public:
    enum type
    {
        ShortWeapon = 0x00,
        ExtendedWeapon,
        ThrownWeapon,
        RangedWeapon,
        Ammunition,
        Shield,
        TorsoArmor,
        LegArmor,
        HeadGear,
        Gloves,
        Shoes,
        MiscEquipment,
        Cloak,
        Instrument,
        Gadget,
        MiscMagic,
        Potion,
        Bomb,
        Powder,
        Spellbook,
        Scroll,
        Food,
        Drink,
        Key,
        Writing, // actually unused
        Other
    };

    enum range
    {
        Short = 0,
        Extended,
        Thrown,
        Long,
        Spash_Ring = -1
    };

    enum class weight
    {
        None = 0x00,
        Minimal,
        Light,
        Medium,
        Heavy,
        Special
    };

    enum attack
    {
        Swing   = 0x0001,
        Thrust  = 0x0002,
        Bash    = 0x0004,
        Berserk = 0x0008,
        Throw   = 0x0010,
        Punch   = 0x0020,
        Kick    = 0x0040,
        Lash    = 0x0080,
        Shoot   = 0x0100
    };
    Q_ENUM(attack);
    Q_DECLARE_FLAGS(attacks, attack);

    enum special_attack
    {
        Sleep         = 0x0001,
        Paralyze      = 0x0002,
        Poison        = 0x0004,
        Hex           = 0x0008,
        Disease       = 0x0010,
        Kill          = 0x0020,
        KO            = 0x0040,
        Blind         = 0x0080,
        Frighten      = 0x0100,
        Swallow       = 0x0200,
        Possess       = 0x0400,
        DrainHP       = 0x0800,
        DrainStamina  = 0x1000,
        DrainSP       = 0x2000,
        Nauseate      = 0x4000,
        Insane        = 0x8000,
    };
    Q_ENUM(special_attack);
    Q_DECLARE_FLAGS(special_attacks, special_attack);

    enum class spell_usage_type
    {
        None = 0x00,
        Item,
        Scroll,
        Spellbook,
        Food,
        Drink_Potion,
        Instrument,
        Bomb_Powder,
        Gadget,
        MiscMagic
    };

    // All the standard professions are in here (plus a few more)
    // but all the ids are different to those in character::profession
    enum slays
    {
        Fighter = 0x00,
        Wizard,          // Mage?
        Priest,
        Rogue,
        Ranger,
        Alchemist,
        Bard,
        Psionic,
        Valkyrie,
        Bishop,
        Lord,
        Samurai,
        Monk,
        Ninja,
        Giant,
        Tiny,
        Beast,
        Plant,
        Insect,
        Vapor,
        Undead,
        Demon,
        Android,
        SeaMonster,
        Dragon,
        Myth,
        Robot,
        Deity,
        Spirit,
        Flyer,
        Swimmer,
        Gadgeteer,
        Reptile,

        None = 0xff
    };

    item(quint32 id = 0xffffffff, quint8 cnt=0, quint8 charges=0, quint8 identified=1, quint8 uncursed=0, bool equipped=false);
    item(const quint8 *item_ptr, bool equipped=false);

    item(const item &other) : QObject()
    {
        m_id         = other.m_id;
        m_cnt        = other.m_cnt;
        m_charges    = other.m_charges;
        m_identified = other.m_identified;
        m_uncursed   = other.m_uncursed;
        m_unknown[0] = other.m_unknown[0];
        m_unknown[1] = other.m_unknown[1];
        m_unknown[2] = other.m_unknown[2];
        m_unknown[3] = other.m_unknown[3];
        m_db_record  = other.m_db_record;

        // Warning: this field needs manual update
        // to set value appropriate for new location
        m_equipped   = other.m_equipped;

        m_helper     = dbHelper::getHelper();
    }
    item & operator=(const item &other)
    {
        m_id         = other.m_id;
        m_cnt        = other.m_cnt;
        m_charges    = other.m_charges;
        m_identified = other.m_identified;
        m_uncursed   = other.m_uncursed;
        m_unknown[0] = other.m_unknown[0];
        m_unknown[1] = other.m_unknown[1];
        m_unknown[2] = other.m_unknown[2];
        m_unknown[3] = other.m_unknown[3];
        m_db_record  = other.m_db_record;

        // Warning: this field needs manual update
        // to set value appropriate for new location
        m_equipped   = other.m_equipped;

        m_helper     = dbHelper::getHelper();

        return *this;
    }

    bool operator<(const item &other) const
    {
        // Just sorting on object id doesn't give me the result
        // Wizardry itself uses; the object groups aren't
        // contiguously numbered

        // So using the types as primary sort (will make it
        // a little slower due to the object lookup)
        int type_me   = static_cast<int> (getType());
        int type_them = static_cast<int> (other.getType());

        if (type_me < type_them)
            return true;
        else if (type_me > type_them)
            return false;

        // And the id as a secondary sort
        return m_id < other.m_id;
    }
    bool isNull() const
    {
        if (m_id == 0xffffffff)
            return true;
        return false;
    }

    void                     serializeIn( QDataStream &in);
    void                     serializeOut(QDataStream &out) const;

    static const QPixmap     getMissingItemImage();

    /*
     *  Retrieve a complete formatted report of all attributes
     *  about the item in HTML
     *  Used by Drag and drop to export to other apps.
     */
    QString                  getCompleteData(bool include_image) const;

    QString                  getName() const;
    QString                  getDesc() const;

    item::type               getType() const;
    QString                  getTypeString() const;
    QString                  getStiFile() const;
    bool                     isCursed() const;
    bool                     needs2Hands() const;
    bool                     canSecondary() const;
    bool                     isShopPersist() const;
    item::special_attacks    getSpecialAttack() const;
    QString                  getSpecialAttackString() const;
    int                      getPercentageChance(item::special_attack a) const;
    int                      getPoisonStrength() const;
    item::slays              getSlays() const;
    QString                  getSlaysString() const;
    const spell              getSpell(int *power = NULL) const;
    item::spell_usage_type   getSpellUsageType() const;
    character::skill         getSkillUsed() const;
    QString                  getSkillUsedString() const;
    character::skill         getSkillBonus( int *bonus = NULL) const;
    character::attribute     getAttributeBonus( int *bonus = NULL) const;
    void                     getRequiredAttrib(int idx, character::attribute *attrib, qint32 *value) const;
    QString                  getRequiredAttribsString() const;
    void                     getRequiredSkill(int idx, character::skill *skill, qint32 *value) const;
    QString                  getRequiredSkillsString() const;
    bool                     getResistance(int *fire, int *water, int *air, int *earth, int *mental, int *divine) const;
    item::range              getRange() const;
    QString                  getRangeString() const;
    int                      getInitiative() const;
    int                      getToHit() const;
    void                     getDamage(quint16 *min_damage, quint16 *max_damage, int *percentage) const;
    item::attacks            getAttacks() const;
    QString                  getAttacksString() const;
    int                      getAC() const;
    item::weight             getArmorWeightClass() const;
    QString                  getArmorWeightClassString() const;
    quint8                   getMaxStackSize() const;
    int                      getHPRegen() const;
    int                      getStaminaRegen() const;
    int                      getSPRegen() const;
    character::professions   getUsableProfessions() const;
    character::races         getUsableRaces() const;
    character::genders       getUsableGenders() const;
    int                      getBonusSwings() const;
    bool                     hasCharges() const;
    bool                     hasUses() const;
    bool                     hasShots() const;
    double                   getWeight() const;
    quint32                  getPrice() const;

    bool                     isOmnigun() const;
    bool                     isOneShotMaximum() const;
    bool                     weaponRequiresStrength() const;
    bool                     damageComesFromAmmunition() const;
    bool                     compatibleAmmunition( item ammo ) const;

    void                     getIdentificationRequirement(int *artifacts, int *spell) const;
    int                      getMaxCharges() const;
//    bool                     needsIdentify() const;
//    bool                     isCriticalItem() const;
// getSpellUsageType


    bool            isStackable() const;
    quint32         getId() const;
    quint8          getCount() const;
    void            setCount(quint8 count);
    quint8          getCharges() const;
    void            setCharges(quint8 charges);
    bool            isIdentified() const;
    bool            isUncursed() const;
    bool            isEquipped() const;
    quint8          getUnknown(int idx) const { return m_unknown[idx]; }

private:
    quint32         m_id;
    quint8          m_cnt;
    quint8          m_charges;
    quint8          m_identified;
    quint8          m_uncursed;
    bool            m_equipped;
    quint8          m_unknown[4];
    QByteArray      m_db_record;

    dbHelper     *m_helper;
};

// These aren't allowed to be member functions
QDataStream &operator<<(QDataStream &out, const item &i);
QDataStream &operator>>(QDataStream &in, item &i);

Q_DECLARE_OPERATORS_FOR_FLAGS(item::attacks)


#endif /* ITEM_H__ */
