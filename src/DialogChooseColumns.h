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

#ifndef DLGCHOOSECOL_H
#define DLGCHOOSECOL_H

#include <QList>
#include <QMetaEnum>

#include "Dialog.h"
#include "StringList.h"
#include "Wizardry8Scalable.h"

class DialogChooseColumns : public Dialog
{
    Q_OBJECT

public:
    enum column
    {
        Name                 = StringList::Item,
        Type                 = StringList::Type,
        Equippable           = StringList::EquippableSlots,
        Special              = StringList::Special,
        AC                   = StringList::AC,
        Weight               = StringList::Weight,
        WeightClass          = StringList::ArmorWeightClass,
        Price                = StringList::Value,
        Stackable            = StringList::Stackable,
        Damage               = StringList::Damage,
        ToHit                = StringList::ToHit,
        Initiative           = StringList::Initiative,
        Hands                = StringList::Hands,
        Cursed               = StringList::CursedStatus,
        SpecialAttack        = StringList::SpecialAttack,
        DoubleDamage         = StringList::DoubleDamageV,
        SkillsUsed           = StringList::Skills,
        Required             = StringList::Requires,             // Primary attribs and skills needed
        AttackModes          = StringList::AttackModes,
        BonusSwings          = StringList::BonusSwings,
        Spell                = StringList::Spell,
        HPRegen              = StringList::HPRegeneration,
        StaminaRegen         = StringList::StaminaRegeneration,
        SPRegen              = StringList::SPRegeneration,
        AttribBonus          = StringList::AttributeBonus,
        SkillBonus           = StringList::SkillBonusDup,
        MagicResistances     = StringList::Resistances,
        Races                = StringList::Races,
        Profs                = StringList::Professions,
        Genders              = StringList::Sex,
    };
    Q_ENUM(column);  

    DialogChooseColumns(const QList<column> &cols, QWidget *parent = nullptr);
    ~DialogChooseColumns();

    QList<column> getColumns();

    void setColumns(const QList<column> &cols);

public slots:
    void moveUp(bool checked);
    void moveDown(bool checked);
    
private:
    QPixmap    makeDialogForm();

    QMetaEnum         m_metaCols;
};

#endif

