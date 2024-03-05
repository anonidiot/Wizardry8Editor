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

#ifndef SPELL_H__
#define SPELL_H__

#include <QDataStream>
#include <QMutex>
#include <QByteArray>
#include <QPixmap>

#include "item.h"
#include "SLFFile.h"

#include "dbHelper.h"

class spell : QObject
{
    Q_OBJECT

public:
    enum target
    {
        Caster = 0,
        OneAlly,
        Party,
        OneEnemy,
        EnemyGroup,
        Cone,
        Radius,
        AllEnemies,
        Point,
        Adventuring,
        AdventuringTraps
    };

    enum usable
    {
        AnyTime = 0,
        Combat,
        NonCombat,
        NonCombatNPCInteraction,
        NonCombatTraps
    };

    spell(quint32 id = 0xffffffff);
    spell(const spell &other) : QObject()
    {
        m_id         = other.m_id;
        m_db_record  = other.m_db_record;

        m_helper     = dbHelper::getHelper();
    }
    spell & operator=(const spell &other)
    {
        m_id         = other.m_id;
        m_db_record  = other.m_db_record;

        m_helper     = dbHelper::getHelper();

        return *this;
    }

    bool isNull() const
    {
        if (m_id == 0xffffffff)
            return true;
        return false;
    }

    int                      getIndex() const { return m_id; }

    QString                  getName() const;
    QString                  getDesc() const;

    int                      getSPCost() const;
    int                      getLevel() const;
    int                      getLevelAsPureClass() const;
    int                      getLevelAsHybridClass() const;

    void                     getDuration(qint32 *base, qint32 *per_pl) const;
    void                     getDamage(quint16 *min_damage, quint16 *max_damage) const;
    item::range              getRange() const;
    QString                  getRangeString() const;
    character::realm         getRealm() const;
    spell::target            getTarget() const;
    QString                  getTargetString() const;
    spell::usable            getUsability() const;
    QString                  getUsabilityString() const;
    character::professions   getClasses() const;

private:
    quint32         m_id;
    QByteArray      m_db_record;

    dbHelper     *m_helper;
};

#endif /* SPELL_H__ */
