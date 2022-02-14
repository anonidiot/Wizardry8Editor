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

#include "common.h"
#include "dbHelper.h"

#define  ITEM_START_OFFSET 0x0004
#define  ITEM_RECORD_SIZE  0x010d
#define  SPELL_RECORD_SIZE 0x02c0

dbHelper *dbHelper::singleton;
QMutex    dbHelper::alloc_lock;

dbHelper::dbHelper() :
    m_numItems(0)
{
    m_item_db = new SLFFile("DATABASES/ITEMS.DBS");

    if (m_item_db)
    {
        m_item_db->open(QIODevice::ReadOnly);

        m_numItems = m_item_db->readLEUShort();

        buildItemQuickFilter();
    }

    m_itemdesc_db = new SLFFile("DATABASES/ITEMDESC.DBS");

    if (m_itemdesc_db)
    {
        m_itemdesc_db->open(QIODevice::ReadOnly);

        m_itemdesc_idx_pos = m_itemdesc_db->size() - 8;
        m_itemdesc_db->seek( m_itemdesc_idx_pos );
        m_itemdesc_idx_pos -= 4 * m_itemdesc_db->readLEULong();
    }

    m_spell_db = new SLFFile("DATABASES/SPELLTABLES.DBS");

    if (m_spell_db)
    {
        m_spell_db->open(QIODevice::ReadOnly);

        // number of spells in [0x0000--0x0003]
        m_spell_idx_pos = 8;
    }

    m_spelldesc_db = new SLFFile("DATABASES/SPELLDESC.DBS");

    if (m_spelldesc_db)
    {
        m_spelldesc_db->open(QIODevice::ReadOnly);

        m_spelldesc_idx_pos = m_spelldesc_db->size() - 8;
        m_spelldesc_db->seek( m_spelldesc_idx_pos );
        m_spelldesc_idx_pos -= 4 * m_spelldesc_db->readLEULong();
    }
}

dbHelper::~dbHelper()
{
    delete[] m_quick_filter_items;
    delete m_item_db;
    delete m_itemdesc_db;
    delete m_spell_db;
}

void dbHelper::buildItemQuickFilter()
{
    m_item_db->seek( ITEM_START_OFFSET );

    m_quick_filter_items = new quint64[m_numItems];

    for (int k=0; k < m_numItems; k++)
    {
        QByteArray   item_record = m_item_db->read(ITEM_RECORD_SIZE);
        quint8      *data        = (quint8 *)item_record.constData();

        quint64 profs   = FORMAT_LE16(data + 0x76);
        quint64 races   = FORMAT_LE16(data + 0x78);
        quint64 genders = FORMAT_8(   data + 0x7c);

        m_quick_filter_items[k] = (profs << 32) | (races << 16) | genders;
    }
}

QList<quint16> dbHelper::getFilteredItems(character::profession prof, character::race race, character::gender gender)
{
    quint64 p = (quint64) prof;
    quint64 r = (quint64) race;
    quint64 g = (quint64) gender;

    quint64 mask = (p << 32) | (r << 16) | g;

    QList<quint16> items;

    for (int k=0; k < m_numItems; k++)
    {
        if ((m_quick_filter_items[k] & mask) == mask)
        {
            items << (quint16)k;
        }
    }

    return items;
}

QString dbHelper::getItemDesc(quint32 item_id)
{
    qint32 item_offset;

    m_itemdesc_db->seek( m_itemdesc_idx_pos + item_id * sizeof(qint32) );

    item_offset = m_itemdesc_db->readLEULong();
    m_itemdesc_db->seek( item_offset );

    // All items seem to have the same byte sequence for the first 8 bytes:
    // 01 00 00 00 00 ff ff ff
    // No idea what they mean

    m_itemdesc_db->skip( 8 );

    // Next 4 bytes give the character length of the UTF-16LE text description that
    // follows. Usually it is 0, but sometimes empty descriptions are indicated
    // by 1 as well
    qint32 str_len = m_itemdesc_db->readLEULong();

    QString desc = "";
    while (str_len-- > 0)
    {
        QChar c = QChar( m_itemdesc_db->readLEUShort() );

        // don't add a trailing null to the string
        if (c.isNull() && (str_len == 0))
            break;

        desc += c;
    }
    return desc;
}

QByteArray dbHelper::getItemRecord(quint32 item_id)
{
    m_item_db->seek(ITEM_START_OFFSET + item_id * ITEM_RECORD_SIZE);

    return m_item_db->read(ITEM_RECORD_SIZE);
}

QString dbHelper::getSpellDesc(quint32 spell_id)
{
    qint32 spell_offset;

    m_spelldesc_db->seek( m_spelldesc_idx_pos + spell_id * sizeof(qint32) );

    spell_offset = m_spelldesc_db->readLEULong();
    m_spelldesc_db->seek( spell_offset );

    // Same as items, all spells seem to have the same byte sequence for the first 8 bytes:
    // 01 00 00 00 00 ff ff ff
    // No idea what they mean

    m_spelldesc_db->skip( 8 );

    // Next 4 bytes give the character length of the UTF-16LE text description that
    // follows. Usually it is 0, but sometimes empty descriptions are indicated
    // by 1 as well
    qint32 str_len = m_spelldesc_db->readLEULong();

    QString desc = "";
    while (str_len-- > 0)
    {
        QChar c = QChar( m_spelldesc_db->readLEUShort() );

        // don't add a trailing null to the string
        if (c.isNull() && (str_len == 0))
            break;

        desc += c;
    }
    return desc;
}

QByteArray dbHelper::getSpellRecord(quint32 spell_id)
{
    m_spell_db->seek(m_spell_idx_pos + spell_id * SPELL_RECORD_SIZE);

    return m_spell_db->read(SPELL_RECORD_SIZE);
}
