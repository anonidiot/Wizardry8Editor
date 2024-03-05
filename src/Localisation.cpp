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

#include "common.h"
#include "main.h"
#include "Localisation.h"
#include "SLFFile.h"
#include "main.h"
#include "dbHelper.h"

#include <QByteArray>
#include <QDirIterator>
#include <QMapIterator>
#include <QSettings>
#include <QTextCodec>

#include <QDebug>

// Wizardry 1.2.8 implements its Localisation in at least 4 separate ways:
// 1. locale specific .dat files in DATA tree, eg. ENG.DAT for particular things
//    like items, spells etc.
// 2. locale specific .slf archives in DATA tree containing items as in (1) above
// 3. FANPATCH.DAT in the base wizardry folder's DATA dir - not in the base folder
//    itself like the other SLFs or in the Parallel Worlds modules.
// 4. Localization folder which contains mod specific localisations.
// Despite often similar names, these files aren't always processed the same way.
// Strings can be either 8 bit or 16 bit, and sometimes have length prefixes and
// sometimes are fixed width. Where they have length prefixes, the counts are
// sometimes in bytes, and sometimes in words. Sometimes the file comprises a
// complete list of all available items/spells etc. and sometimes it will use
// indices to only specify a few.
// I haven't determined the logic used by the 1.2.8 app itself to choose between
// these, so have simply gone with what I found files in my own tree to be using.
// This code is at high risk of being broken at some point if 1.2.8 ever decides
// to put some of these into more common formats though.

// The biggest complicating factor here is that the fanpatch based in
// the Wizardry 8 base folder is ORIGINAL mod specific, but is clearly playing a
// role in localizing all parallel worlds also. And that poses a problem because
// the item and spell databases are in there.

// If a mod never modifies the standard items or spells (ie. only adds to them)
// this doesn't cause any issues. But if it does change things the Localization
// in fanpatch.dat can potentially override the name and description of the item
// back into the text in the original version. I don't know if 1.2.8 has
// safeguards of its own for this. This editor attempts to deal with
// it, though, by only localising an item or spell string if it thinks the item/
// spell really is unchanged from the original. And it works that out by taking
// the mod's unlocalized original language text and comparing that to what is
// in fanpatch.dat for the relevant language. If they don't match, then the item
// is considered changed and shouldn't be localized by fanpatch.dat. (This has
// an impact on modules all working entirely in English.)

// This is an incomplete implementation of the Localization in 1.2.8 which
// tries to do a bare minimum to produce a functional app. Essentially this is
// all that this app supports:
// 1. Check if an explicit Localised string is available in the current lang
//    and if found use that. These exist as files in the filesystem under the
//    Localization folder
// 2. Determine the language used by the module - we work this out based on how
//    many of the localisation strings for each locale match against those in the
//    modules STRINGS/STRINGDATA.DAT
// 3. Look for this determined native locale in FanPatch.dat and extract
//    the relevant string from it for the item/spell etc. we care about.
// 4. If that string is absolutely identical to the one used by the mod, we
//    conclude the name or description of the item to be unchanged from the
//    ORIGINAL version, and will localise it based on the FanPatch.dat.
// 5. Failing all else return the base item string in the foreign language.

// What is not supported: the locale.SLF files, eg. ENG.SLF, RUS.SLF, and
// lang specific .DAT files outside of the fanpatch.dat and Localization
// folder

Localisation *Localisation::singleton;
QMutex        Localisation::alloc_lock;

Localisation::Localisation()
{
    if (::isWizardry128())
    {
        init();
    }
}

Localisation::~Localisation()
{
}

bool Localisation::isLocalisationActive()
{
    if (! ::isWizardry128())
        return false;

    return m_localisationActive;
}

void Localisation::init()
{
    QSettings settings;

    m_moduleName = getModuleName();

    m_locdir = findLocalisationFolder( m_moduleName );

    determineLanguagesAvailable();
    determineOriginalLanguage();
    qDebug() << "Original Language determined to be:" << m_originalLanguage;

    QVariant isocode = settings.value( "PreferredLanguage" );

    if (isocode.isNull())
    {
        settings.setValue( "PreferredLanguage", "ENG" );
        setLanguage( "ENG" );
    }
    else
    {
        setLanguage( isocode.toString() );
    }
}

// Multiple classes may have a pointer reference to the singleton
// localisation class (rather than retrieving a current pointer
// using getLocalisation() each time). Therefore it is kinder to
// reset all the storage in place, and reinitialise, rather than
// destroy and recreate.
// The reset would need to be performed if the parallel world was
// ever changed after the initial setup of the app.
void Localisation::reset()
{
    m_langs.clear();
    m_stringTable.clear();

    m_itemsDb.clear();
    m_itemsDescDb.clear();
    m_fanpatch_unlocalisedItems.clear();
    m_fanpatch_localisedItems.clear();
    m_fanpatch_unlocalisedItemDescs.clear();
    m_fanpatch_localisedItemDescs.clear();

    m_spellsDb.clear();
    m_spellsDescDb.clear();
    m_fanpatch_unlocalisedSpells.clear();
    m_fanpatch_localisedSpells.clear();
    m_fanpatch_unlocalisedSpellDescs.clear();
    m_fanpatch_localisedSpellDescs.clear();

    // leave m_localisationActive unchanged
    m_locdir             = "";
    m_moduleName         = "";
    m_language           = "";
    m_originalLanguage   = "";

    init();
}

QString Localisation::getModuleName()
{
    QString  module_name;
    SLFFile  module_ini("MODINFO.INI");

    if (module_ini.open(QFile::ReadOnly))
    {
        // Likely identical format to main Wiz8.ini ie.
        // a text file without character set header bytes,
        // but will contain Cyrillic characters in the High ASCII range
        // Charset appears to be windows-1251 but not explicitly stated

        QTextCodec *codec = QTextCodec::codecForName("Windows-1251");

        while (module_ini.pos() < module_ini.size())
        {
            QByteArray line = module_ini.readLine();
            QString utfLine = codec->toUnicode( line );

            // INI files usually don't have whitespace around the =, but lets
            // not force the assumption because there are exceptions

            // (BTW: The reason we're not using QSettings to read the ini file
            //  is because this code should work on linux also.)
            if (utfLine.startsWith( "Name" ) && (utfLine.size() > 5))
            {
                // Ensure it's a whole word, and not eg. NameOfTheRose
                if (utfLine[4].isSpace() || (utfLine[4] == '='))
                {
                    int equals = utfLine.indexOf( '=' );

                    module_name = utfLine.mid( equals+1 ).trimmed();
                }
            }
        }
        module_ini.close();
    }
    return module_name;
}

QString Localisation::findLocalisationFolder( QString module_name )
{
    QStringList  tests;

    if (::isParallelWorlds())
    {
        tests << SLFFile::getParallelWorldPath();
    }
    tests << SLFFile::getWizardryPath();

    for (int k=0; k<tests.size(); k++)
    {
        QStringList  filter;
        QStringList  entries;

        QDir cwd = tests.at(k);

        filter << "Localization";
        entries = cwd.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
        if (entries.size() == 1)
        {
            cwd.cd( entries.at(0) );

            filter.clear();
            filter << module_name;

            entries = cwd.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
            if (entries.size() == 1)
            {
                cwd.cd( entries.at(0) );
                return cwd.absolutePath();
            }
        }
    }
    return "";
}

QString Localisation::decode( const char *data, int lenInBytes, bool is16bit)
{
    QString    str;
    QSettings  settings;

    QByteArray codepage = settings.value( "Codepage" ).toString().toLatin1();

    if (codepage != "UTF16LE")
    {
        QTextCodec *codec = QTextCodec::codecForName(codepage.constData());

        QByteArray r;
        for (int k=0; k<lenInBytes; k+=(is16bit ? 2 : 1))
        {
            char u = FORMAT_8(data+k);

            if (u == '\0')
                break;
            r += u;
        }
        return codec->toUnicode( r );
    }

    for (int k=0; k<lenInBytes; k+=(is16bit ? 2 : 1))
    {
        quint16 c;

        if (is16bit)
            c = FORMAT_LE16(data+k);
        else
            c = FORMAT_8(data+k);

        if (c == 0)
            break;

        str += QChar(c);
    }
    return str;
}

void Localisation::determineLanguagesAvailable()
{
    if (! m_locdir.isEmpty())
    {
        QDir locdir( m_locdir );

        QDirIterator it( locdir, QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            QString file = it.next();

            if (file.endsWith( ".DAT", Qt::CaseInsensitive ))
            {
                QString basename = file.replace("\\", "/").toUpper();
                QString lang;

                basename = basename.mid( basename.lastIndexOf( '/' )+1 );

                lang = basename.left( basename.length() - 4 );

                if (! m_langs.contains( lang ))
                {
                    m_langs << lang;
                }
            }
        }
    }
}

void Localisation::determineOriginalLanguage()
{
    int max = 0;

    // Go through the string table for every language we know about
    // and see how many strings for each one match those in the
    // module itself. Then pick the one with the highest score,
    // and determine that to be the original mod language.

    // We can use this information to more carefully apply
    // the localisation in FanPatch.dat. By this I mean we DON'T
    // provide localization for a string/item/spell UNLESS the
    // string is completely identical between this mod's base class
    // and the language localisation determined here. This is to
    // prevent eg. a Sword being incorrectly localised into a potion
    // for example.

    for (int k=0; k<m_langs.size(); k++)
    {
        QMap<quint32, QString> foreignTable = readFile( "STRINGS", m_langs[k] );

        int lang_score = 0;

        QMapIterator<quint32, QString> i(foreignTable);
        while (i.hasNext())
        {
            i.next();

            if (::getStringTable()->getUnlocalisedString( i.key() ) == i.value())
            {
                lang_score++;
            }
        }

        if (lang_score > max)
        {
            max = lang_score;
            m_originalLanguage = m_langs[k];
        }
    }
}

void Localisation::setLanguage( QString language )
{
    language = language.toUpper();

    if (m_langs.contains( language ))
    {
        m_language = language;

        readStringTable();
        readItemsDb();
        readItemsDescDb();
        readSpellsDb();
        readSpellsDescDb();

        if (!m_originalLanguage.isEmpty())
        {
            processFanPatch();
        }
    }
}

QMap<quint32, QString> Localisation::readFile(QString folder, QString language, bool is16bit)
{
    QMap<quint32, QString> entries;

    if (! m_locdir.isEmpty())
    {
        QDir locdir( m_locdir );

        QDirIterator it( locdir, QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            QString file = it.next();
            QString file_normalised = file.replace("\\", "/");

            if (file_normalised.endsWith( QString("%1/%2.DAT").arg( folder ).arg( language ), Qt::CaseInsensitive ))
            {
                QFile f(file);

                if (f.open(QFile::ReadOnly))
                {
                    quint8 buf[4];

                    if (f.pos() + 4 < f.size())
                    {
                        f.read( (char *)buf, 4 );

                        quint32 num_strings = FORMAT_LE32(buf);

                        for (int k=0; k<(int)num_strings; k++)
                        {
                            if (f.pos() + 8 < f.size())
                            {
                                f.read( (char *)buf, 4 );

                                quint32 str_length = FORMAT_LE32(buf);

                                f.read( (char *)buf, 4 );

                                quint32 str_idx    = FORMAT_LE32(buf);

                                if (f.pos() + str_length <= f.size())
                                {
                                    QByteArray str = f.read( str_length );

                                    entries[ str_idx ] = decode( str.constData(), str.size(), is16bit );
                                }
                            }
                        }
                    }
                }
                break;
            }
        }
    }
    return entries;
}

void Localisation::readStringTable()
{
    m_stringTable = readFile( "STRINGS", m_language );
}

QString Localisation::getString(int idx)
{
    return m_stringTable.value( (quint32)idx, "" );
}

void Localisation::readItemsDb()
{
    m_itemsDb = readFile( "ITEMS", m_language );
}

void Localisation::readItemsDescDb()
{
    m_itemsDescDb = readFile( "ITEMDESC", m_language, true );
}

void Localisation::readSpellsDb()
{
    m_spellsDb = readFile( "SPELLTABLES", m_language );
}

void Localisation::readSpellsDescDb()
{
    m_spellsDescDb = readFile( "SPELLDESC", m_language, true );
}

void Localisation::processFanPatch()
{
    // Files inside the FanPatch.dat SLF file are NOT in the same format as those
    // found under the Localization folders for a module. Since the path they extract
    // to according to their internal path should be the Localization tree, I'm not
    // sure how the EXE determines what format is supposed to be used for each one -
    // unless special treatment is given to files in this particular SLF, or extracted
    // under ORIGINAL instead of a mod.

    // FanPatch.dat is a SLF file, but doesn't obey the placement rules
    // that everything else follows when ParallelWorlds are active. ie. it continues to
    // reside in the %WIZ%/Data folder instead of in %WIZ% or in %WIZ%/ParallelWorld/<MOD>/Data
    // So we can't use a regular SLFFile object to retrieve it.

    QDir         cwd = SLFFile::getWizardryPath();
    QStringList  filter;
    QStringList  entries;

    filter.clear();
    filter << "DATA";

    QString fanPatchPath;

    entries = cwd.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );

    if (entries.size() == 1)
    {
        cwd.cd( entries.at(0) );

        QDirIterator it( cwd );
        while (it.hasNext())
        {
            QString file = it.next();

            if (it.fileName().compare( "FanPatch.dat", Qt::CaseInsensitive ) == 0)
            {
                QFile probe( file );

                if (SLFFile::isSlf(probe))
                {
                    fanPatchPath = file;
                    break;
                }
            }
        }
    }
    if (! fanPatchPath.isEmpty())
    {
//        qDebug() << "Found FanPatch.dat and it is a SLF file:" << fanPatchPath;

        QFile *f = new QFile( fanPatchPath );

        if (f->open(QFile::ReadOnly))
        {
            SLFFile fanPatch( f );

            // The STRINGS/RUS.DAT file is a 0 length file, so we can't use that for our
            // our localisation efforts. So the FanPatch will only have an effect on items
            // and spells. Not strings.
            //fanPatch.seekToFile( "LOCALIZATION/ORIGINAL/STRINGS/" + m_originalLanguage + ".DAT" );

            fanPatch.seekToFile( "LOCALIZATION/ORIGINAL/ITEMS/" + m_originalLanguage + ".DAT" );
            m_fanpatch_unlocalisedItems = processFanPatchNamesDb( fanPatch.readAll(), 0x3c );
            fanPatch.seekToFile( "LOCALIZATION/ORIGINAL/ITEMS/" + m_language + ".DAT" );
            m_fanpatch_localisedItems = processFanPatchNamesDb( fanPatch.readAll(), 0x3c );

            fanPatch.seekToFile( "LOCALIZATION/ORIGINAL/ITEMDESC/" + m_originalLanguage + ".DAT" );
            m_fanpatch_unlocalisedItemDescs = processFanPatchDescsDb( fanPatch.readAll() );
            fanPatch.seekToFile( "LOCALIZATION/ORIGINAL/ITEMDESC/" + m_language + ".DAT" );
            m_fanpatch_localisedItemDescs = processFanPatchDescsDb( fanPatch.readAll() );

            fanPatch.seekToFile( "LOCALIZATION/ORIGINAL/SPELLTABLES/" + m_originalLanguage + ".DAT" );
            m_fanpatch_unlocalisedSpells = processFanPatchNamesDb( fanPatch.readAll(), 0x40 );
            fanPatch.seekToFile( "LOCALIZATION/ORIGINAL/SPELLTABLES/" + m_language + ".DAT" );
            m_fanpatch_localisedSpells = processFanPatchNamesDb( fanPatch.readAll(), 0x40 );

            fanPatch.seekToFile( "LOCALIZATION/ORIGINAL/SPELLDESC/" + m_originalLanguage + ".DAT" );
            m_fanpatch_unlocalisedSpellDescs = processFanPatchDescsDb( fanPatch.readAll() );
            fanPatch.seekToFile( "LOCALIZATION/ORIGINAL/SPELLDESC/" + m_language + ".DAT" );
            m_fanpatch_localisedSpellDescs = processFanPatchDescsDb( fanPatch.readAll() );

            f->close(); // Will be deleted by fanPatch going out of scope
        }
    }
}

QStringList Localisation::processFanPatchDescsDb( QByteArray ba )
{
    QStringList   s;

    if (ba.size() > 4)
    {
        quint8 *buf = (quint8 *)ba.data();
        quint8 *b;
        quint32 num_strings = FORMAT_LE32(buf);

        b = buf + 4;
        for (int k=0; k<(int)num_strings; k++)
        {
            if (buf - b + 4 > ba.size())
                break;

            quint32 str_len = FORMAT_LE32(b);

            b += 4;
            if (buf - b + str_len*2 > ba.size())
                break;

            s << decode( (const char *)b, str_len*2, true );
            b += str_len*2;
        }
    }
    return s;
}

QStringList Localisation::processFanPatchNamesDb( QByteArray ba, int record_size )
{
    QStringList   s;

    if (ba.size() > 4)
    {
        const quint8 *buf = (const quint8 *)ba.constData();
        quint32 num_strings = FORMAT_LE32(buf);

        for (int k=0; k<(int)num_strings; k++)
        {
            s << decode( (const char *)buf+4+record_size*k, record_size, true );
        }
    }
    return s;
}

QString Localisation::getItemName( int idx )
{
    if (idx == -1)
        return "";

    if (isLocalisationActive())
    {
        // See if there is an explicit localisation string
        QString s = m_itemsDb.value( (quint32)idx, "" );

        if (! s.isEmpty() )
            return s;

        // Nope
    }

    // Work out what the native unlocalised name of the item is
    // from the mod files

    dbHelper    *helper = dbHelper::getHelper();
    QByteArray   db_record = helper->getItemRecord( idx );

    QString nativeStr = decode( db_record.constData(), 0x3c, true );

    if (!nativeStr.isEmpty() && isLocalisationActive())
    {
        // stop index out of bounds - if the mod has a lot more items than
        // the fanpatch
        if (idx < m_fanpatch_unlocalisedItems.size())
        {
            // Check if the foreign lang string matches
            // the foreign lang string in the fanpatch for the
            // ORIGINAL Wizardry mod, and if so use the localisation
            // for that.
            if (nativeStr == m_fanpatch_unlocalisedItems[idx])
            {
                // expect these to be the same size, but they
                // may not be
                if (idx < m_fanpatch_localisedItems.size())
                {
                    // Yes, return the localised equivalent from FanPatch
                    return m_fanpatch_localisedItems[idx];
                }
            }
            // Nope
        }
    }
    // Go with the unlocalised string from the item itself

    return nativeStr;
}

QString Localisation::getItemDesc( int idx )
{
    if (idx == -1)
        return "";

    if (isLocalisationActive())
    {
        // See if there is an explicit localisation string
        QString s = m_itemsDescDb.value( (quint32)idx, "" );

        if (! s.isEmpty() )
            return s;

        // Nope
    }

    // Work out what the native unlocalised desc of the item is
    // from the mod files

    dbHelper    *helper = dbHelper::getHelper();
    QString      s = helper->getItemDesc( idx );

    if (!s.isEmpty() && isLocalisationActive())
    {
        // stop index out of bounds - if the mod has a lot more items than
        // the fanpatch
        if (idx < m_fanpatch_unlocalisedItemDescs.size())
        {
            // The module didn't have an explicit localisation
            // entry for this item. But if, according to the
            // Localisation DB the FanPatch.dat contains a
            // localisation for the same language as this module,
            // and the text entry for this item in it matches the
            // one here, we can localise it for the current language
            // based on FanPatch.dat instead, since we determine
            // the item string to be unchanged.

            if (s == m_fanpatch_unlocalisedItemDescs[idx])
            {
                // expect these to be the same size, but they
                // may not be
                if (idx < m_fanpatch_localisedItemDescs.size())
                {
                    return m_fanpatch_localisedItemDescs[idx];
                }
            }
            // Nope
        }
    }
    return s;
}

QString Localisation::getSpellName( int idx )
{
    if (idx == -1)
        return "";

    if (isLocalisationActive())
    {
        // See if there is an explicit localisation string
        QString s = m_spellsDb.value( (quint32)idx, "" );

        if (! s.isEmpty() )
            return s;

        // Nope
    }

    // Work out what the native unlocalised name of the item is
    // from the mod files

    dbHelper    *helper = dbHelper::getHelper();
    QByteArray   db_record = helper->getSpellRecord( idx );

    QString nativeStr = decode( db_record.constData()+0x19c, 0x84, true );

    if (!nativeStr.isEmpty() && isLocalisationActive())
    {
        // stop index out of bounds - if the mod has a lot more items than
        // the fanpatch
        if (idx < m_fanpatch_unlocalisedSpells.size())
        {
            // Check if the foreign lang string matches
            // the foreign lang string in the fanpatch for the
            // ORIGINAL Wizardry mod, and if so use the localisation
            // for that.
            if (nativeStr == m_fanpatch_unlocalisedSpells[idx])
            {
                // expect these to be the same size, but they
                // may not be
                if (idx < m_fanpatch_localisedSpells.size())
                {
                    // Yes, return the localised equivalent from FanPatch
                    return m_fanpatch_localisedSpells[idx];
                }
            }
            // Nope
        }
    }
    // Go with the unlocalised string from the spell itself

    if (nativeStr.compare("None") == 0)
        return QString();

    return nativeStr;
}

QString Localisation::getSpellDesc( int idx )
{
    if (idx == -1)
        return "";

    if (isLocalisationActive())
    {
        // See if there is an explicit localisation string
        QString s = m_spellsDescDb.value( (quint32)idx, "" );

        if (! s.isEmpty() )
            return s;

        // Nope
    }

    // Work out what the native unlocalised desc of the item is
    // from the mod files

    dbHelper    *helper = dbHelper::getHelper();
    QString      s = helper->getSpellDesc( idx );

    if (!s.isEmpty() && isLocalisationActive())
    {
        // stop index out of bounds - if the mod has a lot more items than
        // the fanpatch
        if (idx < m_fanpatch_unlocalisedSpellDescs.size())
        {
            // The module didn't have an explicit localisation
            // entry for this item. But if, according to the
            // Localisation DB the FanPatch.dat contains a
            // localisation for the same language as this module,
            // and the text entry for this item in it matches the
            // one here, we can localise it for the current language
            // based on FanPatch.dat instead, since we determine
            // the item string to be unchanged.

            if (s == m_fanpatch_unlocalisedSpellDescs[idx])
            {
                // expect these to be the same size, but they
                // may not be
                if (idx < m_fanpatch_localisedSpellDescs.size())
                {
                    return m_fanpatch_localisedSpellDescs[idx];
                }
            }
            // Nope
        }
    }
    return s;
}
