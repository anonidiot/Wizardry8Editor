/*
 * Copyright (C) 2023 Anonymous Idiot
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

#include <QString>
#include <QVector>
#include "main.h"
#include "Level.h"

#include "SLFFile.h"

#define UNUSED_STR_IDX  1817

// We assume cf agent is always available, because to be honest just looking
// for the file in the folder isn't useful - Wizardry might just not be using
// and be getting directly invoked.

bool cf_agent_installed = true;

QString getLevelName(int level_id)
{
    // The level names are hardcoded as string ids in the Wiz8.exe.
    // By default there are 47 of them.
    // If CFAgent is installed it increases this to 56.

    quint16 str_id;
    quint16 string_ids[] = {
        1783, 1784, 1785, 1786, 1787, 1789, 1788, 1790,
        1791, 1792, 1793, 1817, 1794, 1795, 1796, 1797,
        1798, 1799, 1800, 1801, 1802, 1803, 1804, 1817,
        1805, 1806, 1807, 1808, 1817, 1809, 1817, 1810,
        1811, 1812, 1817, 1813, 1814, 1815, 1798, 1816,
        1817, 1817, 1817, 1817, 1817, 1817, 1817, 1817,
        1817, 1817, 1817, 1817, 1817, 1817, 1817, 1817,
    };

    if (cf_agent_installed)
    {
        quint16 cf_agent_str_id = 2440;

        for (unsigned int k=0; k<sizeof(string_ids)/sizeof(quint16); k++)
        {
            if (string_ids[k] == UNUSED_STR_IDX)
            {
                string_ids[k] = cf_agent_str_id++;
            }
        }
    }

    if (( cf_agent_installed && (level_id > 56)) ||
        (!cf_agent_installed && (level_id > 47)))
    {
        str_id = UNUSED_STR_IDX;
    }
    else
    {
        str_id = string_ids[ level_id ];
    }
    return ::getStringTable()->getString( str_id );
}
    
QString getLevelFilename(int level_id)
{
    const char *levels[] = {
        "Arnika/Arnika2.PVL", 
        "Ascension/Ascension.PVL", 
        "Bayjin/Bayjin1.PVL", 
        "Bayjin/Bayjin2.PVL", 
        "Circle/Circle.PVL", 
        "Marten/Marten1.PVL", 
        "Marten/Marten2.PVL", 
        "MountainPass/MountainPass.PVL", 
        "Monastery/Monastery1.PVL", 
        "Monastery/Monastery2.PVL", 
        "MtGigas/MtGigasWaterCaves.PVL", 
        "MtGigas/MtGigasBelowCaves.PVL", 
        "MtGigas/MtGigas1.PVL", 
        "MtGigas/MtGigas2.PVL", 
        "MtGigas/MtGigasOuter.PVL", 
        "MtGigas/MtGigasTop.PVL", 
        "Camp/Camp.PVL", 
        "Rapax/RapaxCellar.PVL", 
        "Rapax/RapaxMainFloor.PVL", 
        "Rapax/RapaxUpperFloor.PVL", 
        "Rapax/RapaxExterior.PVL", 
        "Rift/Rift1.PVL", 
        "SeaCaves/SeaCave1.PVL", 
        "SeaCaves/SeaCave2.PVL", 
        "Swamp/Swamp.PVL", 
        "Trynnie/Trynnie1.PVL", 
        "Trynnie/Trynnie2.PVL", 
        "ConnectiveTissue/Arnika_Trynton.PVL", 
        "ConnectiveTissue/Trynton_Swamp.PVL", 
        "ConnectiveTissue2/SouthEastWilderness.PVL", 
        "ConnectiveTissue/Rift_Peak.PVL", 
        "ConnectiveTissue2/NorthEastWilderness.PVL", 
        "ConnectiveTissue/NorthWilderness.PVL", 
        "ConnectiveTissue/Arnika_StarterDungeon.PVL", 
        "ConnectiveTissue/Peak_RapaxCastle.PVL", 
        "Footsteps/Footsteps.PVL", 
        "SavantTower/SavantTower.PVL", 
        "Trynnie/Ratkin.PVL", 
        "Camp/CampNoRapax.PVL", 
        "Dungeon/Dungeon.PVL", 
        "Spare14/Spare14.PVL", 
        "Spare15/Spare15.PVL", 
        "Spare16/Spare16.PVL", 
        "Spare17/Spare17.PVL", 
        "Spare18/Spare18.PVL", 
        "Spare19/Spare19.PVL", 
        "Spare20/Spare20.PVL", 
        "Spare21/Spare21.PVL", 
        "Spare22/Spare22.PVL", 
        "Spare23/Spare23.PVL", 
        "Spare24/Spare24.PVL", 
        "Spare25/Spare25.PVL", 
        "Spare26/Spare26.PVL", 
        "Spare27/Spare27.PVL", 
        "Spare28/Spare28.PVL", 
        "Spare29/Spare29.PVL"
    };

    if (( cf_agent_installed && (level_id > 56)) ||
        (!cf_agent_installed && (level_id > 47)))
    {
        return QString();
    }
    return QString( levels[ level_id ] );
}

const int *getLevels()
{
    // Dereferencing a last sequence of SLF archives and pulling files from them
    // is an expensive operation, so cache the result so second and subsequent
    // calls are faster
    static int       *cache = NULL;
    QVector<int>      maps;

    if (cache)
        return cache;

    int max_map_idx = ::getLevelMax();
    for (int k=0; k<max_map_idx; k++)
    {
        SLFFile f( "LEVELS", "LEVELS.SLF", ::getLevelFilename(k) );

        // No point even adding levels that aren't available
        if (f.isGood())
        {
            maps << k;
        }
    }
    maps << -1;

    // make a flat int array of mapIds
    cache = (int *) malloc(maps.size() * sizeof(int));
    memcpy( cache, maps.constData(), maps.size() * sizeof(int) );

    return cache;
}

int getLevelMax()
{
    if ( cf_agent_installed )
        return 56;

    return 47;
}
