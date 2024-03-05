/*
 * Copyright (C) 2024 Anonymous Idiot
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

#ifndef LOCALISATION_H__
#define LOCALISATION_H__

#include <QMutex>
#include <QString>

#include "character.h"
#include "SLFFile.h"

class Localisation
{
private:
    static Localisation *singleton;
    static QMutex    alloc_lock;

protected:
    Localisation();
    ~Localisation();

public:
    // block cloning and assignment
    Localisation(Localisation &other) = delete;
    void operator=(const Localisation &) = delete;

    static Localisation *getLocalisation()
    {
        alloc_lock.lock();
        if (singleton == NULL)
            singleton = new Localisation();
        alloc_lock.unlock();
        return singleton;
    }

    static QString getModuleName();
    static QString findLocalisationFolder( QString module_name );
    static QString decode( const char *data, int lenInBytes, bool is16bit);

    void        reset();
    QStringList getLanguagesAvailable()          { return m_langs; }
    void        setLanguage( QString language );
    QString     getLanguage()                    { return m_language; }
    void        setLocalisationActive( bool on ) { m_localisationActive = on; }
    bool        isLocalisationActive();

    QString     getItemName( int item_id );
    QString     getItemDesc( int item_id );
    QString     getSpellName( int spell_id );
    QString     getSpellDesc( int spell_id );

    QString     getString( int idx );

private:
    void                       init();
    void                       determineLanguagesAvailable();
    void                       determineOriginalLanguage();
    QMap<quint32, QString>     readFile(QString folder, QString language, bool is16bit = false);
    void                       readStringTable();
    void                       readItemsDb();
    void                       readItemsDescDb();
    void                       readSpellsDb();
    void                       readSpellsDescDb();

    void                       processFanPatch();

    QStringList                processFanPatchNamesDb( QByteArray ba, int record_len );
    QStringList                processFanPatchDescsDb( QByteArray ba );

    bool                       m_localisationActive;

    QString                    m_moduleName;
    QString                    m_locdir;
    QStringList                m_langs;
    QString                    m_language;
    QString                    m_originalLanguage;

    QMap<quint32, QString>     m_stringTable;
    QMap<quint32, QString>     m_itemsDb;
    QMap<quint32, QString>     m_itemsDescDb;
    QMap<quint32, QString>     m_spellsDb;
    QMap<quint32, QString>     m_spellsDescDb;

    QStringList                m_fanpatch_localisedItems;
    QStringList                m_fanpatch_unlocalisedItems;
    QStringList                m_fanpatch_localisedItemDescs;
    QStringList                m_fanpatch_unlocalisedItemDescs;

    QStringList                m_fanpatch_localisedSpells;
    QStringList                m_fanpatch_unlocalisedSpells;
    QStringList                m_fanpatch_localisedSpellDescs;
    QStringList                m_fanpatch_unlocalisedSpellDescs;
};

#endif // LOCALISATION_H__
