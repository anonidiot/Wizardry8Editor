/*
 * Copyright (C) 2022-2023 Anonymous Idiot
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

#ifndef PARTY_H__
#define PARTY_H__

#include <QList>
#include <QMetaEnum>
#include <QObject>
#include <QVector>

#include "item.h"

#define NUM_CHARS 8
#define MAX_PLAYERS_PER_FORMATION_CELL 3

class character;

class party : public QObject
{
    Q_OBJECT

public:
    enum formation
    {
        Front,
        Right,
        Back,
        Left,
        Middle,

        FORMATION_SIZE
    };

    enum filter
    {
        Weapons       = 0x0001,
        Armor         = 0x0002,
        Equippable    = 0x0004,
        NonEquippable = 0x0008,
        Usable        = 0x0010,
        Unidentified  = 0x0020,
    };

    party();
    party(QByteArray p);
    ~party();

    QByteArray serialize();

    quint32    getGold() const;
    void       setGold(quint32 gold);

    QVector<qint32> getVisitedMaps();
    void            setVisitedMaps(QVector<qint32> maps);

    void       resetCharacterColors();
    void       divvyUpPartyWeight();
    double     getLoad() const;
    void       insertItem(int idx, item i);
    void       deleteItem(int idx);
    item       getItem(int idx) const;
    int        getItemCount() const;

    qint32     getMapId() const;
    void       getPosition(float *x, float *y, float *z) const;
    float      getHeading() const;
    void       setMapId(qint32 mapId);
    void       setPosition(float x, float y, float z);
    void       setHeading(float heading);

    void       setFilter(quint32 filter);

    void       sortItems();

    QList<character *> m_chars;

    void       addDroppedItem(item i, int idx=0);
    item       getDroppedItem(int idx) const;
    int        getDroppedItemCount() const;
    void       removeDroppedItem(int idx);

signals:
    void       itemDropped(item i);

protected:
    void       unpackParty(const QByteArray &p);
    bool       appendItem( const quint8 *cdata);

private:
    int        defilterIdx(int idx) const;

    QByteArray        m_data;

    quint32           m_gold;

    QVector<qint32>   m_maps;

    qint32            m_mapId;
    float             m_position[3];
    float             m_heading;

    QList<item>       m_items;
    quint32           m_filter;

    QVector<item>     m_droppedItems;

    qint8             m_positions[FORMATION_SIZE][MAX_PLAYERS_PER_FORMATION_CELL];
};

#endif
