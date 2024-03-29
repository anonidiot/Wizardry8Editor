/*
 * Copyright (C) 2022-2024 Anonymous Idiot
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

#include "StringList.h"
#include "Localisation.h"
#include "SLFFile.h"

#include <QTextCodec>
#include <QDebug>

// This app was written to use the the strings from the app where
// possible. Then I found the Grey Tiefing mod and realised that
// if I loaded it the entire interface now came up in Russian as
// a result. It forced some changes in order to try to retain the
// interface in the intended language, but support the mod's language
// for mod specific content.

// The force_base parameter allows the mod dependent localisation
// to be ignored and strings to be retrieved from the base DATA.SLF
// directly for all the standard text labels in the interface.

// We have to be able to retrieve strings from the mod file for
// other activities, though, so now we have to run two string
// tables in parallel.

// There is no sane 'normal' here. A mod could legitimately be
// desiring to change the Fairy class into Smurf, for example
// (whether that works with the game I don't know, but it would
//  have worked with this editor at least). By ignoring the mod
// strings for the interface we stop this from working. But if
// we don't do this, foreign language mods will affect all the
// text on screen, not just their own content. So depending on
// the mod being viewed, the user may want to toggle the menu
// option.
StringList::StringList( QString filename, bool force_base )
{
    SLFFile strings( filename, force_base );

    if (strings.open(QFile::ReadOnly))
    {
        quint32 num_strings = strings.readLEULong();

        for (quint32 k = 0; k < num_strings; k++)
        {
            quint32 str_sz = strings.readLEULong();

            QByteArray s = strings.read( str_sz );

            m_strings << decipher( s );
        }

        strings.close();
    }
}

StringList::~StringList()
{
    m_strings.clear();
}

QString StringList::decipher( QByteArray in )
{
    quint8 *dec = (quint8 *) in.data();
    for (int k=0; k<in.size(); k++)
    {
        int idiotCypher = 0x96 - dec[k];

        if (idiotCypher < 0)
            idiotCypher += 256;

        dec[k] = idiotCypher;
    }
    return Localisation::decode( (const char *)dec, in.size(), true );
}

bool StringList::isNull() const
{
    if (m_strings.size() > 0)
        return false;

    return true;
}

int StringList::getNumStrings() const
{
    return m_strings.size();
}

const QString StringList::getString( int idx ) const
{
    int base_idx = idx & ~APPEND_COLON;

    if ((base_idx > 0) && (base_idx < m_strings.size()))
    {
        Localisation *loc = Localisation::getLocalisation();

        if (loc->isLocalisationActive())
        {
            QString s = loc->getString( base_idx );

            if (!s.isEmpty())
            {
                return s + ((idx & APPEND_COLON) ? ":" : "");
            }

        }
    }

    return getUnlocalisedString( idx );
}

const QString StringList::getUnlocalisedString( int idx ) const
{
    QString s = "";

    int base_idx = idx & ~APPEND_COLON;

    if ((base_idx > 0) && (base_idx < m_strings.size()))
    {
        s = m_strings.at( base_idx );
    }
    else if (base_idx > 5000)
    {
        // Strings we needed which Wizardry's string table lacked
        switch (base_idx)
        {
            case NewSaveFile:
                s = QObject::tr("Start a New Wizardry 8 saved game");
                break;
            case OpenExistingSave:
                s = QObject::tr("Edit an existing saved game");
                break;

            case Copy:
                s = QObject::tr("Copy");
                break;
            case Paste:
                s = QObject::tr("Paste");
                break;
            case IgnoreModStringsShort:
                s = QObject::tr("Ignore Most Module Strings.");
                break;

            case DroppedItems:
                s = QObject::tr("Dropped Items");
                break;
            case Levels:
                s = QObject::tr("Levels");
                break;

            case WarningNewFile:
                s = QObject::tr("Warning: 'New' files created in this program can ONLY be opened by Patched versions of Wizardry (See Special->Patch). DO NOT add RPC characters into the party or the game will crash when those characters are first met in the game.");
                break;
            case WarningSaveReset:
                s = QObject::tr("Warning: 'Reset Save' files created in this program can ONLY be opened by Patched versions of Wizardry (See Special->Patch). Any RPC characters present in the party will be removed if you proceed to prevent game crash when those characters are first met in the game.");
                break;
            case Scenario:
                s = QObject::tr("Scenario");
                break;
            case W7EndUmpani:
                s = QObject::tr("Allied with Umpani from Wizardry 7.");
                break;
            case W7EndTRang:
                s = QObject::tr("Allied with T'Rang from Wizardry 7.");
                break;
            case W7EndOwnShip:
                s = QObject::tr("Stole own ship from Wizardry 7.");
                break;
            case W8Virgin:
                s = QObject::tr("Wizardry 8 Virgin game.");
                break;
            case W7BarloneDead:
                s = QObject::tr("Mark Don Barlone as dead from Wizardry 7.");
                break;
            case W7BarloneDeadHelp:
                s = QObject::tr("Doesn't affect gameplay - just initial dialog when first meet Don Barlone.");
                break;
            case W7RodanDead:
                s = QObject::tr("Mark Rodan Lewarx as dead from Wizardry 7.");
                break;
            case W7RodanDeadHelp:
                s = QObject::tr("Doesn't affect gameplay - just initial dialog when first meet Rodan Lewarx.");
                break;

            case Wiz128Detected:
                s = QObject::tr("Wizardry 1.2.8 detected.");
                break;
            case ParallelWorldsEnabled:
                s = QObject::tr("Parallel Worlds are enabled.");
                break;
            case SelectParallelWorld:
                s = QObject::tr("Select which Parallel World to use with the editor.");
                break;
            case IgnoreModStrings:
                s = QObject::tr("Ignore (most) world specific strings.");
                break;

            case AddItem:
                s = QObject::tr("Add Item");
                break;
            case EditItem:
                s = QObject::tr("Edit Item");
                break;
            case Identify:
                s = QObject::tr("Identify");
                break;
            case RemoveCurse:
                s = QObject::tr("Remove Curse");
                break;
            case Quantity:
                s = QObject::tr("Quantity");
                break;
            case ApplyGold:
                s = QObject::tr("Set Party Gold");
                break;

            case BonusSwings:
                s = QObject::tr("Bonus Swings");
                break;
            case SwingPenalty:
                s = QObject::tr("Swing Penalty");
                break;

            case InspectSpell:
                s = QObject::tr("Inspect Spell");
                break;
            case InspectSkill:
                s = QObject::tr("Inspect Skill");
                break;
            case InspectProfession:
                s = QObject::tr("Inspect Profession");
                break;

            case ImportCharacter:
                s = QObject::tr("Import Character");
                break;
            case ExportCharacter:
                s = QObject::tr("Export Character");
                break;
            case DropCharacter:
                s = QObject::tr("Drop Character");
                break;

            case ExpLast:
                s = QObject::tr("Exp. Last Level");
                break;
            case ExpNext:
                s = QObject::tr("Exp. Needed for Next Level");
                break;

            case Possessed:
                s = QObject::tr("Possessed");
                break;
            case PoisonStrength:
                s = QObject::tr("Poison Strength");
                break;

            case FilterByProfession:
                s = QObject::tr("Filter by Profession");
                break;
            case FilterByRace:
                s = QObject::tr("Filter by Race");
                break;
            case FilterByGender:
                s = QObject::tr("Filter by Gender");
                break;
            case Special:
                s = QObject::tr("Special");
                break;
            case Stackable:
                s = QObject::tr("Stackable");
                break;

            case OpenNavigator:
                s = QObject::tr("Open Navigator");
                break;
            case Position:
                s = QObject::tr("Position");
                break;
            case Heading:
                s = QObject::tr("Heading");
                break;
            case MapWarning:
                s = QObject::tr("Current position can only be changed to maps already visited.");
                break;
            case PortalEnabled:
                s = QObject::tr("Portal Enabled");
                break;

            case PortraitInfo:
                s = QObject::tr("The active portrait can be replaced using the popup menu on the character. Modified images are not stored in the save game file but saved in a Wizardry Patch file (PATCHES\\PATCH.010).");
                break;
            case PortraitModify:
                s = QObject::tr("Customise Portrait");
                break;
            case PortraitReset:
                s = QObject::tr("Reset Portrait to default");
                break;
        }
    }

    if (s.isEmpty())
    {
        qWarning() << "Idx" << base_idx << "not found in string table.";
    }
    return s + ((idx & APPEND_COLON) ? ":" : "");
}
