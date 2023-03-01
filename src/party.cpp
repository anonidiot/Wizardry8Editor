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

#include "party.h"
#include "RIFFFile.h"
#include "item.h"

#include <QDebug>
#include "common.h"

#include <algorithm>

party::party() :
    QObject(),
    m_data(QByteArray(RIFFFile::PARTY_SIZE, 0)),
    m_gold(0),
    m_filter(0)
{
    for (int posn = 0; posn < FORMATION_SIZE; posn++)
    {
        for (int plyr_idx = 0; plyr_idx < MAX_PLAYERS_PER_FORMATION_CELL; plyr_idx++)
        {
            m_positions[ posn ][ plyr_idx ] = -1;
        }
    }

    // WE created the party file from scratch here, and currently it's all
    // zeros. Some of the fields we know how to populate properly, but some
    // of them we don't. These byte sequences are just straight copies of
    // currently unknown sequences from a working game, required to get the
    // "New Game" feature to work properly the same as the "Save as Reset Game"
    // works. When they've been identified they'll move to proper setup.

    // One of these contains the game time, so it's going to be VERY wrong for
    // a newly started game, but that's of minor importance compared to getting
    // this working.

    quint8 *blank = (quint8 *) m_data.data();
    blank[0x0000] = 0xcd;
    blank[0x0001] = 0xcc;
    blank[0x0002] = 0x8c;
    blank[0x0003] = 0x3f;
    blank[0x0004] = 0x98;
    blank[0x0005] = 0x88;
    blank[0x0006] = 0x3f;
    blank[0x0007] = 0x0b;
    blank[0x0008] = 0xb0;
    blank[0x0009] = 0x4b;
    blank[0x000a] = 0x40;
    blank[0x000b] = 0x0b;
    blank[0x000c] = 0x01;

    ASSIGN_LE32( blank + 0x18d0, 0x00000111 );
    ASSIGN_LE32( blank + 0x18d4, 0x00000111 );
    ASSIGN_LE32( blank + 0x18d8, 0x00000014 );
    ASSIGN_LE32( blank + 0x18dc, 0x00000040 );

    ASSIGN_LE32( blank + 0x1900, 0x00000008 );

    ASSIGN_LE32( blank + 0x1b0c, 0x00000001 );

    blank[0x22a7] = 0x3a;
    blank[0x22a8] = 0x4d;
    blank[0x22a9] = 0x94;
    blank[0x22aa] = 0x47;
    blank[0x22ab] = 0x17;
    blank[0x22ac] = 0xf4;
    blank[0x22ad] = 0xb1;
    blank[0x22ae] = 0x44;
    blank[0x22af] = 0x00;
    blank[0x22b0] = 0xfb;
    blank[0x22b1] = 0x4d;
    blank[0x22b2] = 0x47;
    blank[0x22b3] = 0x5a;
    blank[0x22b4] = 0x88;
    blank[0x22b5] = 0xa8;
    blank[0x22b6] = 0x34;

    blank[0x22cb] = 0x08;
    blank[0x22cc] = 0x82;
    blank[0x22cd] = 0x98;
    blank[0x22ce] = 0x40;

    ASSIGN_LE32( blank + 0x234a, 0x00000157 );
    ASSIGN_LE32( blank + 0x234e, 0x0000031d );
    ASSIGN_LE32( blank + 0x2352, 0x0000009c );
    ASSIGN_LE32( blank + 0x2356, 0x00000077 );
    blank[0x235a] = 0x00;
    ASSIGN_LE32( blank + 0x235b, 0xffffffff );

    blank[0x2387] = 0x60;
    blank[0x2388] = 0x7c;
    blank[0x2389] = 0x93;
    blank[0x238a] = 0x02;
    blank[0x238b] = 0x60;
    blank[0x238c] = 0x4e;

    blank[0x2391] = 0x4e;
    blank[0x2392] = 0x1b;
    blank[0x2393] = 0xfa;
    blank[0x2394] = 0x43;

    blank[0x2399] = 0x03;

    blank[0x239d] = 0x60;
    blank[0x239e] = 0x4e;

    blank[0x2425] = 0x01;

    blank[0x242e] = 0x01;

    blank[0x2434] = 0xff;

    blank[0x2447] = 0x01;

    ASSIGN_LE32( blank + 0x2483, 0x000001d0 );

    blank[0x2497] = 0x01;

    blank[0x4997] = 0x01;
    blank[0x49a7] = 0x01;
}

party::party(QByteArray p) :
    QObject(),
    m_data(p),
    m_filter(0)
{
    Q_ASSERT(m_data.size());
    unpackParty(p);
}

party::~party()
{
    m_items.clear();

    while (!m_chars.isEmpty())
    {
        delete m_chars.takeFirst();
    }
}

void party::resetCharacterColors()
{
    int charColIdx = 0;

    Q_ASSERT( m_chars.size() == NUM_CHARS );

    //Players
    for (int k=2; k<NUM_CHARS; k++)
    {
        if (!m_chars[k]->isNull())
        {
            m_chars[k]->setColorIdx( charColIdx++ );
        }
    }
    // RPCs get assigned colours _after the PCs

    // RPCs
    for (int k=0; k<2; k++)
    {
        if (!m_chars[k]->isNull())
        {
            m_chars[k]->setColorIdx( charColIdx++ );
        }
    }
}

QByteArray party::serialize()
{
    quint8 *cdata = (quint8 *) m_data.data();

    // FIXME: There isn't presently anything in the UI for editing the formation,
    // but if it isn't setup properly the save game list can crash (if it's all null)
    // or at a minimum the party can not have pegs for some players until eventually
    // the game autofixes it. So we force a validate here - removing any pegs that
    // no longer represent players, and putting pegs in for those that are present.

    for (int chr_id = 0; chr_id < m_chars.size(); chr_id++)
    {
        if (m_chars[chr_id]->isNull())
        {
            for (int posn = 0; posn < FORMATION_SIZE; posn++)
            {
                for (int plyr_idx = 0; plyr_idx < MAX_PLAYERS_PER_FORMATION_CELL; plyr_idx++)
                {
                    if (m_positions[ posn ][ plyr_idx ] == chr_id)
                        m_positions[ posn ][ plyr_idx ] = -1;
                }
            }
        }
        else
        {
            bool found = false;
            for (int posn = 0; posn < FORMATION_SIZE; posn++)
            {
                for (int plyr_idx = 0; plyr_idx < MAX_PLAYERS_PER_FORMATION_CELL; plyr_idx++)
                {
                    if (m_positions[ posn ][ plyr_idx ] == chr_id)
                    {
                        found = true;
                        // break out of both loops
                        posn = FORMATION_SIZE;
                        break;
                    }
                }
            }
            if (!found)
            {
                for (int posn = 0; posn < FORMATION_SIZE; posn++)
                {
                    for (int plyr_idx = 0; plyr_idx < MAX_PLAYERS_PER_FORMATION_CELL; plyr_idx++)
                    {
                        if (m_positions[ posn ][ plyr_idx ] == -1)
                        {
                            m_positions[ posn ][ plyr_idx ] = chr_id;
                            // break out of both loops
                            posn = FORMATION_SIZE;
                            break;
                        }
                    }
                }
            }
        }
    }

    int num_rpcs = 0;
    int num_pcs  = 0;
    for (int k = 0; k <  m_chars.size(); k++)
    {
        if (!m_chars[k]->isNull())
        {
            if (m_chars[k]->isRPC())
                num_rpcs++;
            else
                num_pcs++;
        }
    }

    // [000d--0010] number of non-RPC characters in party
    ASSIGN_LE32(cdata+0x0d, num_pcs);

    // [0015--0018] total number of people in party
    ASSIGN_LE32(cdata+0x15, (num_pcs + num_rpcs));

    // [0019--001c] gold
    ASSIGN_LE32(cdata+0x19, m_gold);

    // [001d--0020] currently selected character's index - just make it PC 1 for simplicity
    ASSIGN_LE32(cdata+0x1d, 2);

    // [0021--1790] 500 items in party pool
    for (int k=0; k<500; k++)
    {
        if (k < m_items.size())
        {
            ASSIGN_LE32(cdata+0x21 + k*12,      m_items[k].getId());
            ASSIGN_LE8( cdata+0x21 + k*12 +  4, m_items[k].getCount());
            ASSIGN_LE8( cdata+0x21 + k*12 +  5, m_items[k].getCharges());
            ASSIGN_LE8( cdata+0x21 + k*12 +  6, m_items[k].isIdentified());
            ASSIGN_LE8( cdata+0x21 + k*12 +  7, 0);
            ASSIGN_LE8( cdata+0x21 + k*12 +  8, 0);
            ASSIGN_LE8( cdata+0x21 + k*12 +  9, 0);
            ASSIGN_LE8( cdata+0x21 + k*12 + 10, m_items[k].isUncursed());
            ASSIGN_LE8( cdata+0x21 + k*12 + 11, 0);
        }
        else
        {
            ASSIGN_LE32(cdata+0x21 + k*12,      0xffffffff);
            ASSIGN_LE32(cdata+0x21 + k*12 +  4, 0);
            ASSIGN_LE32(cdata+0x21 + k*12 +  5, 0);
            ASSIGN_LE32(cdata+0x21 + k*12 +  6, 0);
            ASSIGN_LE32(cdata+0x21 + k*12 +  7, 0);
            ASSIGN_LE32(cdata+0x21 + k*12 +  8, 0);
            ASSIGN_LE32(cdata+0x21 + k*12 +  9, 0);
            ASSIGN_LE32(cdata+0x21 + k*12 + 10, 0);
            ASSIGN_LE32(cdata+0x21 + k*12 + 11, 0);
        }
    }

    // [1791-1794] number of items that should be in the list
    ASSIGN_LE32(cdata+0x1791, m_items.size());

    // [18e0-18ff] character indexes
    int offset = 0;
    for (int k = 0; k < num_pcs; k++)
    {
        ASSIGN_LE32(cdata + 0x18e0 + offset*4, (k+2));
        offset++;
    }
    for (int k = 0; k < num_rpcs; k++)
    {
        ASSIGN_LE32(cdata + 0x18e0 + offset*4, k);
        offset++;
    }
    while (offset < 8)
    {
        ASSIGN_LE32(cdata + 0x18e0 + offset*4, 0xffffffff);
        offset++;
    }

    // [1900-1903] Current location: map
    ASSIGN_LE32(cdata + 0x1900, m_mapId);

    // [22a7-22b3] Current location: position (stored with *500.0 already applied)
    ASSIGN_FLOAT(cdata + 0x22a7 + 0, m_position[0]);
    ASSIGN_FLOAT(cdata + 0x22a7 + 4, m_position[1]);
    ASSIGN_FLOAT(cdata + 0x22a7 + 8, m_position[2]);

    // [22cb-22ce] Current location: heading
    ASSIGN_FLOAT(cdata + 0x22cb, m_heading);

    // [23a1-23af] player formation in the party
    quint8 *fdata = cdata + 0x23a1;
    for (int posn = 0; posn < FORMATION_SIZE; posn++)
    {
        for (int plyr_idx = 0; plyr_idx < MAX_PLAYERS_PER_FORMATION_CELL; plyr_idx++)
        {
            ASSIGN_LE8(fdata, m_positions[ posn ][ plyr_idx ]);
            fdata++;
        }
    }
    // [23b0-23b4] number of characters in each formation segment
    for (int posn = 0; posn < FORMATION_SIZE; posn++)
    {
        int count = 0;
        for (int plyr_idx = 0; plyr_idx < MAX_PLAYERS_PER_FORMATION_CELL; plyr_idx++)
        {
            if (m_positions[ posn ][ plyr_idx ] != -1)
                count++;
        }
        ASSIGN_LE8(cdata + 0x23b0 + posn, count);
    }
    // [23b5-2414] last position (I think) - game will assert displaying Load screen if this isn't setup
    for (int k = 0; k < m_chars.size(); k++)
    {
        if (m_chars[k]->isNull())
        {
            ASSIGN_LE8(cdata + 0x23b5 + k * 12 + 0, 0xff);
            ASSIGN_LE8(cdata + 0x23b5 + k * 12 + 1, 0xff);
            ASSIGN_LE8(cdata + 0x23b5 + k * 12 + 2, 0xff);
            ASSIGN_LE8(cdata + 0x23b5 + k * 12 + 3, 0x04);
        }
        else
        {
            for (int posn = 0; posn < FORMATION_SIZE; posn++)
            {
                for (int plyr_idx = 0; plyr_idx < MAX_PLAYERS_PER_FORMATION_CELL; plyr_idx++)
                {
                    if (m_positions[ posn ][ plyr_idx ] == k)
                    {
                        ASSIGN_LE8(cdata + 0x23b5 + k * 12 + 0, posn);
                        ASSIGN_LE8(cdata + 0x23b5 + k * 12 + 1, 0xff);
                        ASSIGN_LE8(cdata + 0x23b5 + k * 12 + 2, plyr_idx);
                        // not so sure about this one
                        ASSIGN_LE8(cdata + 0x23b5 + k * 12 + 3, (m_chars[k]->isRPC() ? 2 : 0));
                    }
                }
            }
        }

        ASSIGN_LE32(cdata + 0x23b5 + k * 12 + 4, 0);
        ASSIGN_LE32(cdata + 0x23b5 + k * 12 + 8, 0);
    }

    // FIXME: Journal
    // FIXME: travelling spells - level and turns remaining

    return m_data;
}

bool party::appendItem(const quint8 *cdata)
{
    quint32   id      = FORMAT_LE32(cdata);
    quint8    cnt     = FORMAT_8(cdata+4);
    quint8    charges = FORMAT_8(cdata+5);

    quint8    identified = FORMAT_8(cdata+6);  // 0=unidentified, 1=identified, TODO: other values?
    quint8    uncursed   = FORMAT_8(cdata+10); // 0=object in default state, 1=magically uncursed, TODO: other values?


    // FIXME: Don't know what these other ones are for, but they're doing something
    /*
    quint8    c   = FORMAT_8(cdata+7);
    quint8    d   = FORMAT_8(cdata+8);
    quint8    e   = FORMAT_8(cdata+9);
    quint8    g   = FORMAT_8(cdata+11);
    */

    if (id == 0xffffffff)
        return false;

    m_items << item( id, cnt, charges, identified, uncursed, false );
    return true;
}

void party::unpackParty(const QByteArray &c)
{
    const quint8 *cdata = (const quint8 *) c.constData();

    // [0019--001c] gold
    m_gold = FORMAT_LE32(cdata+0x19);

    // [1791-1794] number of items that should be in the list
    qint32 num_items = FORMAT_LE32(cdata+0x1791);

    // [0021--1790] 500 items in party pool
    for (int k=0; (k < 500) && (k < num_items); k++)
    {
        if (! appendItem( cdata+0x0021 + k*12 ))
            break;
    }

    // [1791-1794] number of items that should be in the list
    // (done up above -- needed to be done before importing items)


    // [1900-1903] Current location: map
    m_mapId = FORMAT_LE32( cdata + 0x1900 );

    // [22a7-22b3] Current location: position (stored with *500.0 already applied)
    m_position[0] = FORMAT_FLOAT( cdata + 0x22a7 + 0 );
    m_position[1] = FORMAT_FLOAT( cdata + 0x22a7 + 4 );
    m_position[2] = FORMAT_FLOAT( cdata + 0x22a7 + 8 );

    // [22cb-22ce] Current location: heading
    m_heading = FORMAT_FLOAT( cdata + 0x22cb );

    // Formation -- player's number (0x00-0x07 is put in one of the bytes, 0xff any empty positions)

    const quint8 *fdata = cdata + 0x23a1;
    for (int posn = 0; posn < FORMATION_SIZE; posn++)
    {
        for (int plyr_idx = 0; plyr_idx < MAX_PLAYERS_PER_FORMATION_CELL; plyr_idx++)
        {
            m_positions[ posn ][ plyr_idx ] = FORMAT_8(fdata);
            fdata++;
        }
    }

    // FIXME: Journal
    // FIXME: travelling spells - level and turns remaining
}

quint32 party::getGold() const
{
    return m_gold;
}

void party::setGold(quint32 gold)
{
    m_gold = gold;
}

void party::addDroppedItem(item i, int idx)
{
    int cnt = getDroppedItemCount();

    if (idx < cnt)
    {
        m_droppedItems.insert( idx, i );
    }
    else
    {
        m_droppedItems.append( i );
    }
    emit itemDropped( i );
}

item party::getDroppedItem(int idx) const
{
    int cnt = getDroppedItemCount();

    if (idx < cnt)
    {
        return m_droppedItems[idx];
    }
    return item();
}

int party::getDroppedItemCount() const
{
    return m_droppedItems.size();
}

void party::removeDroppedItem(int idx)
{
    int cnt = getDroppedItemCount();

    if (idx < cnt)
    {
        m_droppedItems.remove(idx);
    }
}

void party::deleteItem(int idx)
{
    int real_idx = defilterIdx( idx );

    if (real_idx < m_items.size())
    {
        m_items.removeAt(real_idx);
    }
}

void party::insertItem(int idx, item i)
{
    int real_idx = defilterIdx( idx );

    if (real_idx <= m_items.size())
    {
        m_items.insert(real_idx, i);
    }
    else
    {
        m_items.append(i);
    }
}

item party::getItem(int idx) const
{
    int real_idx = defilterIdx( idx );

    if (real_idx < m_items.size())
    {
        return m_items[real_idx];
    }
    return item();
}

int party::getItemCount() const
{
    int cnt = 0;

    if (m_filter == 0)
        return m_items.size();

    // slow
    for (int k=0; k<m_items.size(); k++)
    {
        if ((m_filter & filter::Unidentified) && m_items[k].isIdentified())
        {
            // Combined filter only cares about unidentified objects, and this
            // one is known
            continue;
        }

        if (m_filter & filter::Usable)
            cnt += (m_items[k].hasUses() ? 1 : 0);
        else
        {
            switch (m_items[k].getType())
            {
                case item::type::ShortWeapon:
                case item::type::ExtendedWeapon:
                case item::type::ThrownWeapon:
                case item::type::RangedWeapon:
                case item::type::Ammunition:
                case item::type::Shield:
                    if (m_filter & (filter::Weapons | filter::Equippable))
                        cnt++;
                    break;

                case item::type::TorsoArmor:
                case item::type::LegArmor:
                case item::type::HeadGear:
                case item::type::Gloves:
                case item::type::Shoes:
                    if (m_filter & (filter::Armor | filter::Equippable))
                        cnt++;
                    break;

                case item::type::MiscEquipment:
                case item::type::Cloak:
                    if (m_filter & filter::Equippable)
                        cnt++;
                    break;

                case item::type::Instrument:
                case item::type::Gadget:
                case item::type::MiscMagic:
                case item::type::Potion:
                case item::type::Bomb:
                case item::type::Powder:
                case item::type::Spellbook:
                case item::type::Scroll:
                case item::type::Food:
                case item::type::Drink:
                    if (m_filter & (filter::Usable | filter::NonEquippable))
                        cnt++;
                    break;

                case item::type::Key:
                case item::type::Writing:
                case item::type::Other:
                    if (m_filter & filter::NonEquippable)
                        cnt++;
                    break;
            }
        }
    }
    return cnt;
}

int party::defilterIdx( int idx ) const
{
    if (m_filter == 0)
        return idx;

    // slow
    for (int k=0; k<m_items.size(); k++)
    {
        if ((m_filter & filter::Unidentified) && m_items[k].isIdentified())
        {
            // Combined filter only cares about unidentified objects, and this
            // one is known
            continue;
        }

        if (m_filter & filter::Usable)
            idx -= (m_items[k].hasUses() ? 1 : 0);
        else
        {
            switch (m_items[k].getType())
            {
                case item::type::ShortWeapon:
                case item::type::ExtendedWeapon:
                case item::type::ThrownWeapon:
                case item::type::RangedWeapon:
                case item::type::Ammunition:
                case item::type::Shield:
                    if (m_filter & (filter::Weapons | filter::Equippable))
                        idx--;
                    break;

                case item::type::TorsoArmor:
                case item::type::LegArmor:
                case item::type::HeadGear:
                case item::type::Gloves:
                case item::type::Shoes:
                    if (m_filter & (filter::Armor | filter::Equippable))
                        idx--;
                    break;

                case item::type::MiscEquipment:
                case item::type::Cloak:
                    if (m_filter & filter::Equippable)
                        idx--;
                    break;

                case item::type::Instrument:
                case item::type::Gadget:
                case item::type::MiscMagic:
                case item::type::Potion:
                case item::type::Bomb:
                case item::type::Powder:
                case item::type::Spellbook:
                case item::type::Scroll:
                case item::type::Food:
                case item::type::Drink:
                    if (m_filter & (filter::Usable | filter::NonEquippable))
                        idx--;
                    break;

                case item::type::Key:
                case item::type::Writing:
                case item::type::Other:
                    if (m_filter & filter::NonEquippable)
                        idx--;
                    break;
            }
        }

        if (idx == -1)
            return k;
    }
    return -1;
}

void party::setFilter(quint32 filter)
{
    m_filter = filter;
}

void party::divvyUpPartyWeight()
{
    quint32 party_load = (quint32)(getLoad() * 10.0);

    for (int k=0; k<NUM_CHARS; k++)
        m_chars[k]->setPartyShare( 0 );

    // Items in the party space only count half as much as if they're
    // on a character - obviously the same school of thought that says
    // snacks in between meals don't contain any calories...
    for (unsigned int j = 0; j < party_load / 2; j++)
    {
        double  max_spare_capacity = -999999.9;
        int     most_space_idx     = -1;

        for (int k = 0; k < NUM_CHARS; k++)
        {
            if (m_chars[k]->isAlive())
            {
                qint32 carry_capacity = (qint32)(m_chars[k]->getLoad(character::atIdx::Base)*10.0);
                double spare_capacity = 100.0 * (carry_capacity - m_chars[k]->getPersonalLoad()
                                                                - m_chars[k]->getPartyShare()) /
                                                 carry_capacity;
                if (max_spare_capacity < spare_capacity)
                {
                    most_space_idx = k;
                    max_spare_capacity = spare_capacity;
                }
            }
        }
        // If the party has items but no characters then this will be -1
        if (most_space_idx == -1)
            break;

        // Now that we know who has the most space, add a 1/10 pound (really a 1/20 pound)
        // onto their carried weight and repeat until all party weight accounted for
        m_chars[most_space_idx]->setPartyShare( 1 + m_chars[most_space_idx]->getPartyShare() );
    }
    for (int k = 0; k < NUM_CHARS; k++)
    {
        if (m_chars[k]->isAlive())
        {
            m_chars[k]->recomputeLoadCategory();
//            qDebug() << m_chars[k]->getName() << "got " << m_chars[k]->getPartyShare() << " of " << party_load;
        }
    }
}

double party::getLoad() const
{
    double weight = 0;
    for (int k=0; k< m_items.size(); k++)
    {
        if (m_items[k].isStackable())
            weight += m_items[k].getCount() * m_items[k].getWeight();
        else
            weight += m_items[k].getWeight();
    }
    return weight;
}

void party::sortItems()
{
    // This uses the custom operator '<' defined in the item
    // class in order to work. That method uses the type of
    // the item first, and then the id field to sort with.
    //
    // That is a much lazier approach than the actual game uses
    // but don't think it's important for an item editor.
    // The game uses the following in this order:
    //  1. the item type
    //  2. the sti_idx
    //  3. identified before unidentified
    //  4. price
    //  5. count
    //  6. charges
    std::sort(m_items.begin(), m_items.end());
}

void party::setVisitedMaps(QVector<qint32> maps)
{
    m_maps = maps;
}

QVector<qint32> party::getVisitedMaps()
{
    return m_maps;
}

qint32 party::getMapId() const
{
    return m_mapId;
}

void party::getPosition(float *x, float *y, float *z) const
{
    *x = m_position[0];
    *y = m_position[1];
    *z = m_position[2];
}

float party::getHeading() const
{
    return m_heading;
}

void  party::setMapId(qint32 mapId)
{
    m_mapId = mapId;
}

void  party::setPosition(float x, float y, float z)
{
    m_position[0] = x;
    m_position[1] = y;
    m_position[2] = z;
}

void  party::setHeading(float heading)
{
    m_heading = heading;
}


