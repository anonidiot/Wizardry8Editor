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

#ifndef DBHELPER_H__
#define DBHELPER_H__

#include <QMutex>
#include <QByteArray>

#include "character.h"
#include "SLFFile.h"

class dbHelper
{
private:
    static dbHelper *singleton;
    static QMutex    alloc_lock;

    SLFFile *m_item_db;
    SLFFile *m_itemdesc_db;
    qint64   m_itemdesc_idx_pos;
    int      m_numItems;

    SLFFile *m_spell_db;
    SLFFile *m_spelldesc_db;
    qint64   m_spelldesc_idx_pos;
    qint64   m_spell_idx_pos;

    quint64 *m_quick_filter_items;

    void buildItemQuickFilter();

protected:
    dbHelper();
    ~dbHelper();

public:
    // block cloning and assignment
    dbHelper(dbHelper &other) = delete;
    void operator=(const dbHelper &) = delete;

    int                 getNumItems() { return m_numItems; }

    QString             getItemDesc(quint32 item_id);
    QByteArray          getItemRecord(quint32 item_id);

    QString             getSpellDesc(quint32 item_id);
    QByteArray          getSpellRecord(quint32 item_id);

    QList<quint16>      getFilteredItems(character::profession p, character::race r, character::gender g);

    static dbHelper *getHelper()
    {
        alloc_lock.lock();
        if (singleton == NULL)
            singleton = new dbHelper();
        alloc_lock.unlock();
        return singleton;
    }
};

#endif // DBHELPER_H__
