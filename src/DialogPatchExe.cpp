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

#include "DialogPatchExe.h"
#include "SLFFile.h"
#include "main.h"

#include <QCryptographicHash>
#include <QDirIterator>
#include <QFile>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QStyleFactory>

#include "WButton.h"
#include "WImage.h"
#include "WLabel.h"
#include "WListWidget.h"
#include "WScrollBar.h"

#include <QDebug>

// v 1.2.4 version patches

int apply_greedy_pickpocket_patch( quint8 *data );
int apply_levelsensor_patch( quint8 *data );
int apply_chestreset_patch( quint8 *data );
int apply_hacksav_patch( quint8 *data );

// v 1.2.8 build 6200 versions of some of these patches.
// Our modified SAV file format is _not_ supported under v1.2.8.
// due to the changes 1.2.8 itself makes to the file format
// (or more exactly the code hooks it implements for this
//  purpose which are modifying the same code area as us)

int apply_greedy_pickpocket_patch_6200( quint8 *data );
int apply_levelsensor_patch_6200( quint8 *data );
int apply_chestreset_patch_6200( quint8 *data );

struct patch
{
    DialogPatchExe::wizardry_ver    game_ver;
    const char                     *name;
    const char                     *description;
    int                             (*function)(quint8 *);
};

struct patch known_patches[] =
{
    { DialogPatchExe::wizardry_ver::WIZ_VER_1_2_4,
      "Greedy Pickpocket (v3)",
      "Removes the escalating difficulty of pickpocketing more and more items from a character to match the"
      "1.0 version behaviour. With patience and much saving around 40 items can be stolen from an individual"
      "NPC. Also removes the XP based seed on the item retrieved, so it randomises for each attempt.",
      &apply_greedy_pickpocket_patch },

    { DialogPatchExe::wizardry_ver::WIZ_VER_1_2_4,
      "Disable Level Sensor on Treasure",
      "Disables the level sensor check on treasure chests and item drops from monsters, but retains it for"
      "random attack encounters.",
      &apply_levelsensor_patch },

    { DialogPatchExe::wizardry_ver::WIZ_VER_1_2_4,
      "Randomise Chests on first Open",
      "Determine treasure chest contents when first opened, not when level first entered.",
      &apply_chestreset_patch },

    { DialogPatchExe::wizardry_ver::WIZ_VER_1_2_4,
      "NEW Savegame support",
      "Patches the game so it can recognise SAV game files without ANY level info (such as the ones produced"
      "by this editor when creating a NEW file). Such games only need the patch to load the first time, if"
      "they are resaved afterwards they become a regular save game and can be played in unpatched versions"
      "of the game also.",
      &apply_hacksav_patch },

    // I don't intend to keep updating these for every single build of v1.2.8
    // Someone has already made a request to have these patches integrated as
    // an option into the standard 1.2.8, and that's a better place for them.

    { DialogPatchExe::wizardry_ver::WIZ_VER_1_2_8_BUILD_6200,
      "Greedy Pickpocket (v3)",
      "Removes the escalating difficulty of pickpocketing more and more items from a character to match the"
      "1.0 version behaviour. With patience and much saving around 40 items can be stolen from an individual"
      "NPC. Also removes the XP based seed on the item retrieved, so it randomises for each attempt.",
      &apply_greedy_pickpocket_patch_6200 },

    { DialogPatchExe::wizardry_ver::WIZ_VER_1_2_8_BUILD_6200,
      "Disable Level Sensor on Treasure",
      "Disables the level sensor check on treasure chests and item drops from monsters, but retains it for"
      "random attack encounters.",
      &apply_levelsensor_patch_6200 },

    { DialogPatchExe::wizardry_ver::WIZ_VER_1_2_8_BUILD_6200,
      "Randomise Chests on first Open",
      "Determine treasure chest contents when first opened, not when level first entered.",
      &apply_chestreset_patch_6200 }
};

struct patch_detail
{
    quint32      offset; // don't need to worry about files larger than 4GB
    int          cnt;
    const char  *bytes;
};


typedef enum
{
    NO_ID,

    PATCH_LIST_LBL,

    PATCHES_SCROLLLIST,
    PATCHES_SCROLLBAR,
    PATCHES_DESC,

    SIZE_WIDGET_IDS
} widget_ids;


DialogPatchExe::DialogPatchExe(QWidget *parent)
    : Dialog(parent)
{
    QPixmap bgImg = makeDialogForm();
    QSize   bgImgSize = bgImg.size();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout patchesScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    bgImg,                                                          this ),  -1,  NULL },

        { PATCH_LIST_LBL,     QRect(  10,  22, 350,  12 ),    new WLabel( tr("Patches:"), Qt::AlignLeft,  10, QFont::Thin,                   this ),  -1,  NULL },

        { PATCHES_SCROLLBAR,  QRect( 354,  42,  15, 115 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },
        { PATCHES_SCROLLLIST, QRect(  88,  41, 252, 116 ),    new WListWidget(                                                               this ),  -1,  NULL },
        { PATCHES_DESC,       QRect(  88, 167, 252, 100 ),    new WLabel(    "",          Qt::AlignLeft,   9, QFont::Thin,                   this ),  -1,  NULL },

        { NO_ID,              QRect( 312, 268,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               0, true, 1.0,   this ),  -1,  SLOT(patchAway(bool)) },
        { NO_ID,              QRect( 344, 268,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               4, true, 1.0,   this ),  -1,  SLOT(close()) },
    };

    int num_widgets = sizeof(patchesScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( patchesScrn, num_widgets, this );

    char *exePath = NULL;
    QString err = identifyWizardryExeVersion( "", &m_myWizVer, NULL, &exePath );

    if (! err.isEmpty() )
    {
        QMessageBox::warning(this, tr("Patch Application Error"), err );
    }
    else
    {
        if (WLabel *lbl = qobject_cast<WLabel *>(m_widgets[ PATCH_LIST_LBL ] ))
        {
            lbl->setText( tr("Patches available for Wizardry EXE v%1:").arg( getVersionStr(m_myWizVer) ) );
        }        
    }
    if (exePath)
    {
        m_exePath = exePath;
        free( exePath );
    }

    // The patch list
    if (WListWidget *patches = qobject_cast<WListWidget *>(m_widgets[ PATCHES_SCROLLLIST ] ))
    {
        if (WScrollBar *sb = qobject_cast<WScrollBar *>(m_widgets[ PATCHES_SCROLLBAR ] ))
        {
            // QScrollAreas include their own scrollbar when necessary, but we need to
            // override this since the Wizardry 8 look has the scrollbar outside the
            // scroll area and a short space away to the right

            patches->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
            patches->setVerticalScrollBar( sb );
            // The call to setVerticalScrollBar reparents it, which prevents us drawing
            // it where we want, so reparent it (again) back to us
            sb->setParent( this );
            // return it to the same widget stack postion it was
            sb->stackUnder( m_widgets[ PATCHES_SCROLLLIST ] );
        }
        connect( patches, SIGNAL(currentRowChanged(int)),
                  this, SLOT(patchHighlighted(int)) );
    }

    updateList();
    if (WListWidget *patches = qobject_cast<WListWidget *>(m_widgets[ PATCHES_SCROLLLIST ] ))
    {
        if (patches->count() == 0)
        {
            QMessageBox::warning(this, tr("Patch Application Error"),
                tr("There are no patches for the version of Wizardry you have installed."));
        }
    }

    this->setMinimumSize( bgImgSize * m_scale );
    this->setMaximumSize( bgImgSize * m_scale );

    // Something in the OK button displeased the dialog layout, and made a
    // bigger than minimum dialog. We have to make an additional call to force
    // it back to the right size after adding the OK button.
    this->resize( bgImgSize * m_scale );
    show();
}

DialogPatchExe::~DialogPatchExe()
{
}

QPixmap DialogPatchExe::makeDialogForm()
{
    // there is no suitable existing Wizardry image for the dialog we
    // want here, so we hack one together from a few different pieces
    QPixmap bgImg,     bgPtn;

    // images used in dialog
    bgImg = SLFFile::getPixmapFromSlf( "DIALOGS/POPUP_ITEMINFO.STI", 0 );
    bgPtn = SLFFile::getPixmapFromSlf( "DIALOGS/DIALOGBACKGROUND.STI", 0 );

    QPixmap customImage( QSize( bgImg.width(), bgImg.height() + 20 ) );

    QPainter p;

    p.begin( &customImage );
    // First make it a little taller, by 20 rows and squeeze the hole for
    // the scrollarea and the buttonbox over 20 pixels to the right
    p.drawPixmap(   0,   0, bgImg,   0,   0, bgImg.width(), 242 );       // Top
    p.drawPixmap(  26,  69, bgImg,   6,  69, 66, 171 );                      // Shrunk scrollpane
    p.drawPixmap(   0, 177, bgImg,   0, 157,  6, bgImg.height() - 157 ); // LHS bevel
    p.drawPixmap( 378, 177, bgImg, 378, 157, bgImg.width() - 378, bgImg.height() - 157 ); // RHS bevel
    p.drawPixmap(   0, 262, bgImg,   0, 242, bgImg.width(), bgImg.height() - 242 ); // Bottom
    p.drawPixmap(  26, 262, bgImg,   6, 242, 66, bgImg.height() - 248 ); // Shrunk buttonbox
    // Now fill in the slot where the professions/races went, the slot for the icon, and the
    // empty space we just left in the middle.
    p.drawPixmap(   6,   6, bgPtn,   0,   0, 140,  64 ); // patches slot
    p.drawPixmap(   6,   6, bgPtn,   0,   0,  82, 242 ); // most of the professions slot
    p.drawPixmap(   6,  bgImg.height() - bgPtn.height() + 124, bgPtn,   0, 110,  79, bgPtn.height() - 110);
    p.drawPixmap(   6, 145, bgPtn,   0, 95,  bgPtn.width(), 119 );       // LHS of the hole we made
    p.drawPixmap( 178, 145, bgPtn,   bgPtn.width()-200, 95,  200, 119 ); // RHS of the hole we made
    p.end();

    return customImage;
}

void DialogPatchExe::updateList()
{
    // Repopulate the patch list
    if (WListWidget *patches = qobject_cast<WListWidget *>(m_widgets[ PATCHES_SCROLLLIST ] ))
    {
        patches->clear();

        int num_items = sizeof(known_patches) / sizeof(struct patch);
        for (int k=0; k<num_items; k++)
        {
            if (known_patches[k].game_ver == m_myWizVer)
            {
                QListWidgetItem *newPatch = new QListWidgetItem( tr(known_patches[k].name) );
                newPatch->setData( Qt::UserRole, k );
                newPatch->setFlags(newPatch->flags() | Qt::ItemIsUserCheckable); // set checkable flag
                newPatch->setCheckState( Qt::Unchecked );

                patches->addItem( newPatch );
            }
        }
        patches->sortItems(Qt::AscendingOrder);

        patches->setCurrentRow( 0 );
        patchHighlighted( 0 );
    }
}

void DialogPatchExe::patchHighlighted(int)
{
    if (WListWidget *patches = qobject_cast<WListWidget *>(m_widgets[ PATCHES_SCROLLLIST ] ))
    {
        QListWidgetItem *now = patches->currentItem();

        if (now)
        {
            int patch_idx = now->data( Qt::UserRole ).toInt();

            if (WLabel *desc = qobject_cast<WLabel *>(m_widgets[ PATCHES_DESC ] ))
            {
                desc->setText( tr(known_patches[patch_idx].description) );
            }
        }
    }
}

QString DialogPatchExe::getVersionStr(wizardry_ver ver)
{
    switch (ver)
    {
        case WIZ_VER_1_0:
            return "1.0 (Original)";

        case WIZ_VER_1_2_4:
            return "1.2.4 (Original)";

        case WIZ_VER_1_2_4_PATCHED:
            return "1.2.4 (Patched)";

        case WIZ_VER_1_2_8_BUILD_6200:
            return "1.2.8 (Build 6200)";

        case WIZ_VER_1_2_8_BUILD_6200_PATCHED:
            return "1.2.8 (Build 6200 - Patched)";

        case WIZ_VER_1_2_8_UNKNOWN:
            return "1.2.8 (Unknown)";

        default:
            break;
    }

    return "UNKNOWN";
}

quint64 DialogPatchExe::getExpectedExeSize(wizardry_ver ver)
{
    switch (ver)
    {
        case WIZ_VER_1_0:
            return 2641920;

        case WIZ_VER_1_2_4:
        case WIZ_VER_1_2_4_PATCHED:
            return 3580513;

        case WIZ_VER_1_2_8_BUILD_6200:
        case WIZ_VER_1_2_8_BUILD_6200_PATCHED:
            return 3299840;

        default:
        case WIZ_VER_1_2_8_UNKNOWN:
            break;
    }

    return 0;
}

void DialogPatchExe::patchAway(bool)
{
    if (m_exePath.isEmpty())
    {
        QMessageBox::warning(this, tr("Patch Application Error"),
            tr("The Wizardry executable can't be located."));
        return;
    }

    QFile   src(m_exePath);
 
    if (src.open(QFile::ReadOnly))
    {
        if (src.size() != getExpectedExeSize(m_myWizVer))
        {
            src.close();
            QMessageBox::warning(this, tr("Patch Application Error"),
                tr("The Wizardry executable isn't the correct size for the %1 version (%2 bytes).").arg(getVersionStr(m_myWizVer)).arg(getExpectedExeSize(m_myWizVer)));
            return;
        }
    }

    QByteArray data = src.readAll();

    src.close();

    // Show a file dialog and ask where they want to save it -

    QString &wizardryPath = SLFFile::getWizardryPath();
    QString saveFile = ::getSaveFileName(NULL, tr("Save File"), wizardryPath, tr("Exes (*.exe)"));
    if (saveFile.isEmpty())
        return;

    if (! saveFile.endsWith(".EXE", Qt::CaseInsensitive))
        saveFile += ".EXE";

    QFile dst(saveFile);

    if (! dst.open(QFile::WriteOnly))
    {
        QMessageBox::warning(this, tr("Patch Application Error"),
            tr("Can't open file for writing in this directory."));
        return;
    }

    // Apply checked patches
    if (WListWidget *patches = qobject_cast<WListWidget *>(m_widgets[ PATCHES_SCROLLLIST ] ))
    {
        int num_items = patches->count();
        for (int j=0; j<num_items; j++)
        {
            int idx = patches->item(j)->data( Qt::UserRole ).toInt();

            if (patches->item(j)->checkState() == Qt::Checked)
            {
                // If already patched, just patch again. All these patches can safely
                // be applied over the top of themselves again and again
                known_patches[ idx ].function( (quint8 *) data.data() );
            }
        }
    }

    // Save the file
    dst.write( data );
    dst.close();

    // Close the entire dialog
    close();
}

// Combined the old pickpocket v2 patch into this and labelled it v3.
// Together they restore the 1.0 behaviour, and are better taken together
// or omitted together.
int apply_greedy_pickpocket_patch( quint8 *data )
{
    memset( data + 0x0010bcec, 0x90, 120 );                                  // 0x0050bcec
    memset( data + 0x0010c04c, 0x90, 125 );                                  // 0x0050c04c

    struct patch_detail p[] =
    {
        { 0x0010bcdd,  11, "\x83\xc4\x04\x89\x6c\x24\x3c\xc7\x44\x24\x14" }, // 0x0050bcdd
        { 0x0010bcea,   2, "\xff\xff" },                                     // 0x0050bcea

        { 0x0016d0a8,   2, "\x90\x90" },

        // These bits used to be part of the v1 Pickpocket patch.
        // It is more appropriate here to be enabled if Greed is active
        { 0x0010be3d,   4, "\x90\x90\x90\x90" },                             // 0x0050be3d
        { 0x0010c0ea,   4, "\x90\x90\x90\x90" },                             // 0x0050c0ea
    };

    for (unsigned long j=0; j < sizeof(p) / sizeof(p[0]); j++)
    {
        for (int k=0; k < p[j].cnt; k++)
        {
            data[ p[j].offset + k ] = p[j].bytes[k];
        }
    }
    return 0;
}

/*
 * Wizardry 1.2.8 hotfixes the same function we want to modify here at
 * runtime (ie. self modifies its own code).
 * The instruction at 0x50be03 is altered to zero a different struct field
 * and a code jump to bypass the end of the function and complete with new
 * code of their own outside of the segment map (likely runtime allocated)
 * is performed at 0x50be74.
 * An altered version of the 1.2.4 patch was made here to attempt to not
 * interfere with this, but still achieve the greedy effect. It could,
 * however, be breaking the functionality Wizardry 1.2.8 is patching it for.
 * Going forwards, it is better that 1.2.8 implements this itself if
 * people want it, rather than risk this interfering with it. And it saves
 * the trouble of constantly needing to redo it for every build number.
 */
int apply_greedy_pickpocket_patch_6200( quint8 *data )
{
    memset( data + 0x0010b75c, 0x90, 125 );                                  // 0x0050c15c

    struct patch_detail p[] =
    {
        { 0x0010b3ed,  11, "\x83\xc4\x04\x89\x6c\x24\x3c\xc7\x44\x24\x14" }, // 0x0050bded
        { 0x0010b3fa,   4, "\xff\xff\xeb\x75" },                             // 0x0050bdfa

        { 0x0016c7e8,   2, "\x90\x90" },                                     // 0x0056d1e8

        // These bits used to be part of the v1 Pickpocket patch.
        // It is more appropriate here to be enabled if Greed is active
        { 0x0010b54d,   4, "\x90\x90\x90\x90" },                             // 0x0050bf4d
        { 0x0010b7fa,   4, "\x90\x90\x90\x90" },                             // 0x0050c1fa
    };

    for (unsigned long j=0; j < sizeof(p) / sizeof(p[0]); j++)
    {
        for (int k=0; k < p[j].cnt; k++)
        {
            data[ p[j].offset + k ] = p[j].bytes[k];
        }
    }
    return 0;
}

int apply_levelsensor_patch( quint8 *data )
{
    struct patch_detail p[] =
    {
        { 0x000f8a40,   6, "\x32\xc9\x90\x90\x90\x90" },
        { 0x000f8b17,   6, "\x32\xc0\x90\x90\x90\x90" },
    };

    for (unsigned long j=0; j < sizeof(p) / sizeof(p[0]); j++)
    {
        for (int k=0; k < p[j].cnt; k++)
        {
            data[ p[j].offset + k ] = p[j].bytes[k];
        }
    }
    return 0;
}

int apply_levelsensor_patch_6200( quint8 *data )
{
    struct patch_detail p[] =
    {
        { 0x000f8150,   6, "\x32\xc9\x90\x90\x90\x90" }, // Patches instruction at 0x004f8b50
        { 0x000f8227,   6, "\x32\xc0\x90\x90\x90\x90" }, // Patches instruction at 0x004f8c27
    };

    for (unsigned long j=0; j < sizeof(p) / sizeof(p[0]); j++)
    {
        for (int k=0; k < p[j].cnt; k++)
        {
            data[ p[j].offset + k ] = p[j].bytes[k];
        }
    }
    return 0;
}

int apply_chestreset_patch( quint8 *data )
{
    struct patch_detail p[] =
    {
        { 0x00045585,   6, "\x90\x90\x90\x90\x90\x90" },
    };

    for (unsigned long j=0; j < sizeof(p) / sizeof(p[0]); j++)
    {
        for (int k=0; k < p[j].cnt; k++)
        {
            data[ p[j].offset + k ] = p[j].bytes[k];
        }
    }
    return 0;
}

int apply_chestreset_patch_6200( quint8 *data )
{
    struct patch_detail p[] =
    {
        { 0x00044a75,   6, "\x90\x90\x90\x90\x90\x90" }, // Patches instruction at 0x00445475
    };

    for (unsigned long j=0; j < sizeof(p) / sizeof(p[0]); j++)
    {
        for (int k=0; k < p[j].cnt; k++)
        {
            data[ p[j].offset + k ] = p[j].bytes[k];
        }
    }
    return 0;
}

int apply_hacksav_patch( quint8 *data )
{
    memmove( data + 0x00112968, data + 0x0011295d, 0x158 );
    memmove( data + 0x00112b72, data + 0x00112b6d,  0xaa );

    struct patch_detail p[] =
    {
        { 0x00112937,   1, "\x50" },
        { 0x00112942,   1, "\x14" },
        { 0x00112955,   1, "\x68" },
        { 0x0011295d,   6, "\xc7\x84\x24\x10\x00\x00" },
        { 0x00112964,   4, "\x01\x00\x00\x00" },
        { 0x00112970,   1, "\x6c" },
        { 0x00112978,   1, "\x74" },
        { 0x00112987,   2, "\xf5\xb1" },
        { 0x0011298c,   1, "\x70" },
        { 0x00112991,   1, "\xbb" },
        { 0x00112996,   1, "\x86" },
        { 0x001129b1,   1, "\x4b" },
        { 0x001129b9,   1, "\xf3" },
        { 0x001129be,   1, "\xbe" },
        { 0x001129c5,   1, "\x70" },
        { 0x001129d7,   1, "\x68" },
        { 0x00112a22,   1, "\x4a" },
        { 0x00112a2b,   1, "\x64" },
        { 0x00112a33,   1, "\x99" },
        { 0x00112a65,   1, "\x18" },
        { 0x00112a67,   1, "\x95" },
        { 0x00112a72,   1, "\x14" },
        { 0x00112a76,   1, "\x68" },
        { 0x00112a7b,   1, "\x61" },
        { 0x00112a82,   1, "\x9d" },
        { 0x00112a89,   1, "\x14" },
        { 0x00112a8b,   1, "\x31" },
        { 0x00112a93,   1, "\xfd" },
        { 0x00112a94,   1, "\x00" },
        { 0x00112aa0,   1, "\x1c" },
        { 0x00112aa2,   1, "\x2a" },
        { 0x00112aa9,   1, "\x14" },
        { 0x00112aab,   1, "\xb1" },
        { 0x00112ab3,   1, "\xc4" },
        { 0x00112aba,   1, "\x14" },
        { 0x00112abc,   1, "\xa0" },
        { 0x00112ac0,  53, "\x8b\x54\x24\x14\x8d\x4c\x24\x14\x3d\x47\x53\x54\x41\x74\x33\x3d\x48"
                           "\x41\x43\x4b\x74\x1e\x3d\x4a\x52\x4e\x4c\x77\x5f\x74\x55\x3d\x4e\x50"
                           "\x43\x46\x77\x3f\x74\x35\x3d\x46\x41\x54\x41\x74\x26\xe9\x87\x00\x00"
                           "\x00\x52" },
        { 0x00112af6,   3, "\x91\x4b\x0d" },
        { 0x00112afa,   4, "\x33\xc0\x89\x44" },
        { 0x00112aff, 115, "\x14\xeb\x76\x68\x70\x51\x68\x00\x51\xe8\xe3\x31\x00\x00\x83\xc4\x08"
                           "\xe9\x66\x00\x00\x00\x52\xe8\x55\x35\x02\x00\xeb\x5b\x52\xe8\x7d\x39"
                           "\xff\xff\xeb\x53\x3d\x4e\x50\x43\x49\x75\x4f\x52\xe8\x9e\x25\x06\x00"
                           "\xeb\x44\x52\xe8\xe6\x5f\x04\x00\xeb\x3c\x3d\x4e\x50\x43\x54\x77\x28"
                           "\x74\x1e\x3d\x48\x59\x50\x4e\x74\x0f\x3d\x54\x56\x41\x52\x75\x28\x52"
                           "\xe8\xb7\x17\xf3\xff\xeb\x1d\x51\xe8\xaf\x37\x00\x00\xeb\x15\x51\xe8"
                           "\x57\x74\xff\xff\xeb\x0d\x3d\x54\x45\x58\x54\x75\x09" },
        { 0x00112b74,   1, "\xb8" },
        { 0x00112b7e,   1, "\x14" },
        { 0x00112b80,   1, "\x0c" },
        { 0x00112b87,   1, "\x14" },
        { 0x00112b89,   1, "\xa3" },
        { 0x00112b90,   2, "\x05\xff" },
        { 0x00112b97,   1, "\x14" },
        { 0x00112b99,   1, "\x63" },
        { 0x00112bb6,   1, "\x36" },
        { 0x00112be1,   1, "\x9b" },
        { 0x00112be6,   1, "\x16" },
        { 0x00112bf9,   1, "\x63" },
        { 0x00112c03,   1, "\xd9" },
        { 0x00112c08,   1, "\xe4" },
        { 0x00112c0f,   1, "\x14" },
        { 0x00112c13,   1, "\x68" },
        { 0x00112c18,   1, "\xc4" },
        { 0x00112c1c,  12, "\x8b\x84\x24\x10\x00\x00\x00\x8b\x8c\x24\x60\x01" },
        { 0x00112c2a,  18, "\x5f\x5e\x5d\x5b\x64\x89\x0d\x00\x00\x00\x00\x81\xc4\x5c\x01\x00\x00\xc3" },
        { 0x00191381,   1, "\xe9" },
        { 0x00191382,   1, "\xac" },
        { 0x00191383,   1, "\x01" },
        { 0x00191384,   1, "\x00" },
        { 0x00191386,  16, "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90" },
        { 0x001e768c, 146, "\x4c\x24\x04\x68\x00\x00\x00\x00\x68\x04\x00\x00\x00\x68\x50\xde\x68"
                           "\x00\x51\xe8\xfc\xd7\xe1\xff\x59\x68\x00\x00\x00\x00\x68\x01\x00\x00"
                           "\x00\x68\x63\xde\x68\x00\x51\xe8\xe6\xd7\xe1\xff\x59\x68\x00\x00\x00"
                           "\x00\x68\x01\x00\x00\x00\x68\x5d\xde\x68\x00\x51\xe8\xd0\xd7\xe1\xff"
                           "\x83\xc4\x28\x83\x3d\x50\xde\x68\x00\x04\x75\x13\xc6\x05\xb4\x75\x68"
                           "\x00\x00\xc7\x05\x50\xde\x68\x00\x00\x00\x00\x00\xeb\x07\xc6\x05\xb4"
                           "\x75\x68\x00\x01\xe8\x15\xec\xf1\xff\xe8\x00\xc1\xfd\xff\xc7\x05\xcc"
                           "\xb7\x69\x00\x00\x00\x00\x00\xe8\x21\x86\xfa\xff\xe8\x7c\x21\xf2\xff"
                           "\xe8\x07\x22\xf2\xff\xe8\x02\x11\xf7\xff" },
        { 0x001e771f,   3, "\xfd\xe1\xf4" },
        { 0x001e7723,  23, "\x66\xc7\x05\x05\x69\x68\x00\x03\x00\x33\xc0\xb9\x08\x00\x00\x00\xbf"
                           "\x07\x9b\x68\x00\xf3\xab" },
        { 0x001f405c,   2, "\x43\x78" },
        { 0x001f4064,   2, "\x43\x78" },
        { 0x001f406c,   2, "\x43\x78" },
        { 0x001f4074,   2, "\x43\x78" },
        { 0x001f407c,   2, "\x43\x78" },
        { 0x001f4084,   2, "\x43\x78" },
        { 0x001f408c,   2, "\x43\x78" },
        { 0x001f4094,   2, "\x43\x78" },
        { 0x001f409c,   2, "\x43\x78" },
        { 0x001f40a4,   2, "\x43\x78" },
        { 0x001f40ac,   2, "\x43\x78" },
        { 0x001f40b4,   2, "\x43\x78" },
        { 0x001f40bc,   2, "\x43\x78" },
        { 0x001f40c4,   2, "\x43\x78" },
        { 0x001f40cc,   2, "\x43\x78" },
        { 0x001f40d4,   2, "\x43\x78" }
    };

    for (unsigned long j=0; j < sizeof(p) / sizeof(p[0]); j++)
    {
        for (int k=0; k < p[j].cnt; k++)
        {
            data[ p[j].offset + k ] = p[j].bytes[k];
        }
    }
    return 0;
}

QString DialogPatchExe::identifyWizardryExeVersion( QString exeFile, wizardry_ver *ver, char *md5Hash, char **exePath )
{
    if (exePath != NULL)
    {
        *exePath = NULL;
    }
    if (ver != NULL)
    {
        *ver = WIZ_VER_UNKNOWN;
    }

    QString &wizardryPath = SLFFile::getWizardryPath();

    QFile   *src = NULL;

    if (! exeFile.isEmpty())
    {
        src = new QFile( exeFile );
    }
    else
    {
        // Fan Patch 1.2.8 doesn't replace the original executable, but instead installs
        // its own, Wiz8_v128.exe. We need to check for that first up, otherwise it will
        // make a wrong version determination based on the original, unused executable.
        // If we fail to find that we then search for the original, Wiz8.exe.
        QStringList  wizardryExe;

        wizardryExe << "Wiz8_v128.exe" << "Wiz8.exe";

        for (int i = 0; !src && (i < wizardryExe.size()); i++)
        {
            // For the benefit of case sensitive filesystems actually check all combinations of
            // casing of the filename, and don't assume a particular format
            QDir        path    = QDir(wizardryPath);

            QDirIterator it( path, QDirIterator::Subdirectories);
            while (it.hasNext())
            {
                QString file = it.next();

                if (file.compare( path.absoluteFilePath( wizardryExe.at(i) ), Qt::CaseInsensitive ) == 0)
                {
                    src = new QFile(file);
                    if ((ver != NULL) && (wizardryExe.at(i) == "Wiz8_v128.exe"))
                    {
                        *ver = WIZ_VER_1_2_8_UNKNOWN;
                    }
                    break;
                }
            }
        }
    }

    if (!src)
    {
        return QString( "The Wizardry executable can't be located." );
    }
    if (exePath != NULL)
    {
        QByteArray ba = src->fileName().toLatin1();

        *exePath = strdup( ba.data() );
    }
    if (src->open(QFile::ReadOnly))
    {
        QByteArray machineType = src->read(2);

        // Wizardry EXEs should be original format 16-bit MZ executables
        if ((machineType[0] != 'M') &&
            (machineType[1] != 'Z'))
        {
            src->close();
            delete src;

            return QString( "Not correct type of Wizardry executable!" );
        }
        if (ver != NULL)
        {
            // ver is either WIZ_VER_UNKNOWN or WIZ_VER_1_2_8_UNKNOWN at this point
            // See if we can identify a specific version by making an initial guess,
            // applying patches for that guess and seeing if it matches the final
            // expected MD5 for a particular version.

            src->reset();
            QByteArray exeData = src->readAll();

            QByteArray hashData  = QCryptographicHash::hash(exeData, QCryptographicHash::Md5);
            QByteArray md5before = hashData.toHex();

            wizardry_ver patch_ver = WIZ_VER_UNKNOWN;

            if (src->size() == getExpectedExeSize(WIZ_VER_1_0))
            {
                patch_ver = WIZ_VER_1_0;
            }
            else if (src->size() == getExpectedExeSize(WIZ_VER_1_2_4))
            {
                patch_ver = WIZ_VER_1_2_4;
            }
            else if (src->size() == getExpectedExeSize(WIZ_VER_1_2_8_BUILD_6200))
            {
                patch_ver = WIZ_VER_1_2_8_BUILD_6200;
            }
            else if (*ver == WIZ_VER_1_2_8_UNKNOWN)
            {
                // Wizardry 1.2.8 fan patches are different sizes for every release
                patch_ver = WIZ_VER_1_2_8_UNKNOWN;
            }

            int num_items = sizeof(known_patches) / sizeof(struct patch);
            for (int k=0; k<num_items; k++)
            {
                if (known_patches[k].game_ver == patch_ver)
                {
                    // Ignore any already patched errors
                    known_patches[ k ].function( (quint8 *) exeData.data() );
                }
            }
            hashData = QCryptographicHash::hash(exeData, QCryptographicHash::Md5);
            QByteArray md5after = hashData.toHex();

            if (md5Hash)
            {
                strcpy( md5Hash, md5after.data() );
            }

            // Not presently any patches for the 1.0 exe, so should
            // match the pristine MD5
            if (md5after == "73ad900ac38e38bdc93d25627ea66035")
            {
                Q_ASSERT(md5before == "73ad900ac38e38bdc93d25627ea66035");
                *ver = WIZ_VER_1_0;
            }
            else if (md5after == "1a3b555563966d3a96dfcd8ce990b2e6")
            {
                if (md5before == "cdf6a691d899fa192e05ceb4a3cc1e67")
                {
                    *ver = WIZ_VER_1_2_4;
                    if (md5Hash)
                    {
                        strcpy( md5Hash, md5before.data() );
                    }
                }
                else
                {
                    *ver = WIZ_VER_1_2_4_PATCHED;
                }
            }
            else if (md5after == "5a8404639ef0cb1ef43fc50a3bf11147")
            {
                if (md5before == "a80aaed35cd3c2d71620d23d7394aa54")
                {
                    *ver = WIZ_VER_1_2_8_BUILD_6200;
                    if (md5Hash)
                    {
                        strcpy( md5Hash, md5before.data() );
                    }
                }
                else
                {
                    *ver = WIZ_VER_1_2_8_BUILD_6200_PATCHED;
                }
            }
        }

        src->close();
    }
    else
    {
        delete src;

        return QString( "The Wizardry executable can't be opened." );
    }
    delete src;

    return "";
}
