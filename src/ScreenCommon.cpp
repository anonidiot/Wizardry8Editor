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

#include <QButtonGroup>
#include <QByteArray>
#include <QColor>
#include <QDirIterator>
#include <QIcon>
#include <QMainWindow>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QResizeEvent>
#include <QStatusBar>

#include "Screen.h"
#include "ScreenCommon.h"
#include "ScreenItems.h"
#include "ScreenLevels.h"
#include "ScreenMagic.h"
#include "ScreenSkills.h"
#include "ScreenAttribs.h"
#include "ScreenPersonality.h"
#include "PortraitsDb.h"

#include "DialogRUSure.h"

#include "common.h"
#include "main.h"
#include "character.h"

#include "WButton.h"
#include "WCheckBox.h"
#include "WImage.h"
#include "WItem.h"
#include "WLabel.h"
#include "WLineEdit.h"
#include "WScrollBar.h"
#include "WSpinBox.h"

#include "RIFFFile.h"
#include "SLFFile.h"
#include "STI.h"

#include <QDebug>

static const QString k_large_portrait  = "Portraits/Large/L";
static const QString k_medium_portrait = "Portraits/Medium/M";
static const QString k_rpcmed_portrait = "Portraits/Medium/a";
static const QString k_small_portrait  = "Portraits/Small/S";

static const QString sti_portraits[] =
{
    "hummf.sti",
    "hummn.sti",
    "hummm.sti",
    "humm1.sti",
    "humm2.sti",
    "humm3.sti",
    "humff.sti",
    "humfn.sti",
    "humfm.sti",
    "humf1.sti",
    "humf2.sti",
    "humf3.sti",
    "elfmf.sti",
    "elfmn.sti",
    "elfmm.sti",
    "elfff.sti",
    "elffn.sti",
    "elffm.sti",
    "dwarfmf.sti",
    "dwarfmn.sti",
    "dwarfmm.sti",
    "dwarfff.sti",
    "dwarffn.sti",
    "dwarffm.sti",
    "gnomemn.sti",
    "gnomemm.sti",
    "gnomeff.sti",
    "gnomefn.sti",
    "hobmf.sti",
    "hobmn.sti",
    "hobff.sti",
    "hobfn.sti",
    "fairymf.sti",
    "fairymm.sti",
    "fairyff.sti",
    "fairyfn.sti",
    "lizmf.sti",
    "lizmn.sti",
    "lizff.sti",
    "lizfn.sti",
    "dracmf.sti",
    "dracmn.sti",
    "dracff.sti",
    "dracfn.sti",
    "felpmf.sti",
    "felpmn.sti",
    "felpff.sti",
    "felpfn.sti",
    "rawmn.sti",
    "rawmm.sti",
    "rawff.sti",
    "rawfn.sti",
    "mookmf.sti",
    "mookmn.sti",
    "mookff.sti",
    "mookfn.sti",
    "ninmf.sti",
    "ninff.sti",
    "mook.sti",
    "trynm1.sti",
    "trynm2.sti",
    "trynm3.sti",
    "trang.sti",
    "umpani.sti",
    "sexus.sti",
    "urq.sti",
    "RFS-81.sti",
    "madras.sti",
    "sparkle.sti",
    "myles.sti",
    "Vi.sti",
    "drazic.sti",
    "tantris.sti",
    "rodan.sti",
    "glumph.sti",
    "saxx.sti",
    "humm4.sti",
    "humf4.sti",
    "elfm1.sti",
    "elff1.sti"
};

typedef enum
{
    NO_ID,

    PLAYER_START,
    RPC1 = PLAYER_START,
    RPC2,
    PC1,
    PC2,
    PC3,
    PC4,
    PC5,
    PC6,
    PLAYER_END,

    PAGE_START,
    PAGE_ATTRIBS = PAGE_START,
    PAGE_ITEMS,
    PAGE_MAGIC,
    PAGE_SKILLS,
    PAGE_LEVELS,
    PAGE_EXIT,
    PAGE_END,

    VAL_NAME,
    VAL_RACE,
    VAL_PROF,
    VAL_LEVEL,
    VAL_PORTRAIT,
    VAL_SUM_AC,
    VAL_SUM_WEAPON1A,
    VAL_SUM_WEAPON1B,
    VAL_SUM_PROF,
    VAL_SUM_HP,
    VAL_HP_BAR,
    VAL_STAMINA_BAR,
    VAL_MP_BAR,

    OVL_LEVELS,
    LBL_LEVELS,
    VAL_PROF1,
    VAL_LEVEL1,
    VAL_PROF2,
    VAL_LEVEL2,
    VAL_PROF3,
    VAL_LEVEL3,
    VAL_PROF4,
    VAL_LEVEL4,
    VAL_PROF5,
    VAL_LEVEL5,
    VAL_PROF6,
    VAL_LEVEL6,
    VAL_PROF7,
    VAL_LEVEL7,
    VAL_PROF8,
    VAL_LEVEL8,
    VAL_PROF9,
    VAL_LEVEL9,
    VAL_PROF10,
    VAL_LEVEL10,
    VAL_PROF11,
    VAL_LEVEL11,
    VAL_PROF12,
    VAL_LEVEL12,
    VAL_PROF13,
    VAL_LEVEL13,
    VAL_PROF14,
    VAL_LEVEL14,
    VAL_PROF15,
    VAL_LEVEL15,
    VAL_PROF16,
    VAL_LEVEL16,
    OVL_LEVELS_END,
} widget_ids;


ScreenCommon::ScreenCommon(party *p, int charIdx, QWidget *parent) :
    Screen(parent),
    m_copiedChar(NULL),
    m_party(p),
    m_charIdx(charIdx)
{
    QPixmap attribsButton[5];

    for (int k = 0; k< 5; k++)
    {
        attribsButton[k] = makeAttribsButton( k );
    }

    struct layout commonItems[] =
    {
        { PAGE_ATTRIBS,       QRect( 376, 450,  -1,  -1 ),    new WButton(   attribsButton,                                     false, 1.0,  this ),  StringList::Attribs,            SLOT(reviewAttribs(bool)) },
        { PAGE_LEVELS,        QRect( 420, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWPAGEBUTTONS.STI",                 5, false, 1.0,  this ),  StringList::Levels,             SLOT(reviewLevels(bool)) },
        { PAGE_ITEMS,         QRect( 464, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWPAGEBUTTONS.STI",                 0, false, 1.0,  this ),  StringList::Items,              SLOT(reviewItems(bool)) },
        { PAGE_MAGIC,         QRect( 508, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWPAGEBUTTONS.STI",                10, false, 1.0,  this ),  StringList::Magic,              SLOT(reviewMagic(bool)) },
        { PAGE_SKILLS,        QRect( 552, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWPAGEBUTTONS.STI",                15, false, 1.0,  this ),  StringList::Skills,             SLOT(reviewSkills(bool)) },
        { PAGE_EXIT,          QRect( 596, 450,  -1,  -1 ),    new WButton(   "CHAR GENERATION/CG_BOTTOMBUTTONS.STI",        16, true,  1.0,  this ),  StringList::Exit,               SLOT(exitButton(bool)) },

        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    "REVIEW/COMMONCORNER.STI",                      0,              this ),  -1,                             NULL }, // Character selection frame
        { RPC1,               QRect(   6,   5,  -1,  -1 ),    new WButton(                                                                   this ),  -1,                             SLOT(charswap(bool)) },
        { RPC2,               QRect(  54,   5,  -1,  -1 ),    new WButton(                                                                   this ),  -1,                             SLOT(charswap(bool)) },
        { PC1,                QRect(   6,  44,  -1,  -1 ),    new WButton(                                                                   this ),  -1,                             SLOT(charswap(bool)) },
        { PC2,                QRect(  54,  44,  -1,  -1 ),    new WButton(                                                                   this ),  -1,                             SLOT(charswap(bool)) },
        { PC3,                QRect(   6,  83,  -1,  -1 ),    new WButton(                                                                   this ),  -1,                             SLOT(charswap(bool)) },
        { PC4,                QRect(  54,  83,  -1,  -1 ),    new WButton(                                                                   this ),  -1,                             SLOT(charswap(bool)) },
        { PC5,                QRect(   6, 122,  -1,  -1 ),    new WButton(                                                                   this ),  -1,                             SLOT(charswap(bool)) },
        { PC6,                QRect(  54, 122,  -1,  -1 ),    new WButton(                                                                   this ),  -1,                             SLOT(charswap(bool)) },
        { VAL_PORTRAIT,       QRect( 164,  12,  -1,  -1 ),    new WImage(                                                                    this ),  -1,                             NULL },
        { NO_ID,              QRect( 164,  12,  -1,  -1 ),    new WImage(    "REVIEW/COMMONCORNER.STI",                      9,              this ),  -1,                             NULL }, // overlay for active character
        { NO_ID,              QRect( 126,  11,  -1,  -1 ),    new WImage(    "MAIN INTERFACE/MAIN_COLUMN_INFO_NORMAL.STI",   0,              this ),  -1,                             NULL },
        { VAL_SUM_WEAPON1A,   QRect( 130,  29,  -1,  -1 ),    new WImage(                                                                    this ),  -1,                             NULL },
        { VAL_SUM_WEAPON1B,   QRect( 130,  59,  -1,  -1 ),    new WImage(                                                                    this ),  -1,                             NULL },
        { NO_ID,              QRect( 267,  11,  -1,  -1 ),    new WImage(    "MAIN INTERFACE/MAIN_COLUMN_INFO_NORMAL.STI",   3,              this ),  -1,                             NULL },
        { VAL_SUM_AC,         QRect( 126,  14,  20,   8 ),    new WLabel(    "",                           Qt::AlignCenter,  8, QFont::Thin, this ),  -1,                             NULL },
        { VAL_SUM_PROF,       QRect( 267,  14,  22,   8 ),    new WLabel(    "",                           Qt::AlignCenter,  8, QFont::Thin, this ),  -1,                             NULL },
        { VAL_SUM_HP,         QRect( 267,  74,  22,   8 ),    new WLabel(    "",                           Qt::AlignCenter,  8, QFont::Thin, this ),  -1,                             NULL },
        { VAL_HP_BAR,         QRect( 270,  28,  -1,  -1 ),    new WImage(     4, 45, QColor::fromRgb(0xc8, 0x00, 0x00),                      this ),  -1,                             NULL },
        { VAL_STAMINA_BAR,    QRect( 276,  28,  -1,  -1 ),    new WImage(     4, 45, QColor::fromRgb(0xfc, 0xed, 0x00),                      this ),  -1,                             NULL },
        { VAL_MP_BAR,         QRect( 283,  28,  -1,  -1 ),    new WImage(     4, 45, QColor::fromRgb(0x48, 0x37, 0xf8),                      this ),  -1,                             NULL },

        { VAL_NAME,           QRect( 115,  95, 186,  12 ),    new WLabel(    "",                           Qt::AlignCenter, 12, QFont::Bold, this ),  StringList::Change,             SLOT(changeName(bool)) },
        { VAL_RACE,           QRect( 115, 109, 186,  12 ),    new WLabel(    "",                           Qt::AlignCenter, 12, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF,           QRect( 115, 123, 186,  12 ),    new WLabel(    "",                           Qt::AlignCenter, 12, QFont::Thin, this ),  StringList::ViewProfStatus,     SLOT(professionLevels(bool)) },
        { VAL_LEVEL,          QRect( 115, 137, 186,  12 ),    new WLabel(    "",                           Qt::AlignCenter, 12, QFont::Thin, this ),  -1,                             NULL },

        // Overlay panel showing Professions by Level
        { OVL_LEVELS,         QRect(   0,   0,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWSCREEN_POPUPS.STI",               0,              this ),  -1,                             NULL },
        { LBL_LEVELS,         QRect(  10,  11, 289,  14 ),    new WLabel(    StringList::LevelsByProf,     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF1,          QRect(  14,  35, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL1,         QRect( 118,  35,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF2,          QRect(  14,  49, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL2,         QRect( 118,  49,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF3,          QRect(  14,  63, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL3,         QRect( 118,  63,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF4,          QRect(  14,  77, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL4,         QRect( 118,  77,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF5,          QRect(  14,  91, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL5,         QRect( 118,  91,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF6,          QRect(  14, 105, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL6,         QRect( 118, 105,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF7,          QRect(  14, 119, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL7,         QRect( 118, 119,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF8,          QRect(  14, 133, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL8,         QRect( 118, 133,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF9,          QRect( 160,  35, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL9,         QRect( 264,  35,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF10,         QRect( 160,  49, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL10,        QRect( 264,  49,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF11,         QRect( 160,  63, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL11,        QRect( 264,  63,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF12,         QRect( 160,  77, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL12,        QRect( 264,  77,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF13,         QRect( 160,  91, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL13,        QRect( 264,  91,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF14,         QRect( 160, 105, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL14,        QRect( 264, 105,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF15,         QRect( 160, 119, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL15,        QRect( 264, 119,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PROF16,         QRect( 160, 133, 104,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LEVEL16,        QRect( 264, 133,  31,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
    };

    QPixmap pix = ::getCursor(Qt::ArrowCursor);
    if (! pix.isNull())
    {
        this->setCursor( QCursor( pix, 0, 0 ) );
    }

    int num_widgets = sizeof(commonItems) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( commonItems, num_widgets, this );

    resetCharacterSelectButtons();

    m_charSelect = new QButtonGroup(this);
    for (int k=PLAYER_START; k < PLAYER_END; k++)
    {
        m_charSelect->addButton( qobject_cast<QPushButton *>(m_widgets[ k ]), k - PLAYER_START );
    }

    // Set the buttons to reflect selected character
    qobject_cast<QPushButton *>(m_widgets[ PLAYER_START + m_charIdx ])->setChecked( true );

    m_pageSelect = new QButtonGroup(this);
    for (int k=PAGE_START; k < PAGE_END; k++)
    {
        m_pageSelect->addButton( qobject_cast<QPushButton *>(m_widgets[ k ]), k - PAGE_START );
    }

    m_currentScreen = new ScreenAttribs( m_party->m_chars[m_charIdx], this );
    m_currentScreen->lower();
    m_currentScreen->setVisible(true);

    connect( m_currentScreen, SIGNAL(changedRace()), this, SLOT(changedRace()) );
    connect( m_currentScreen, SIGNAL(changedProf()), this, SLOT(changedProf()) );
    connect( m_currentScreen, SIGNAL(changedSex()),  this, SLOT(changedSex())  );

    // Set the buttons to reflect selected page
    qobject_cast<QPushButton *>(m_widgets[ PAGE_ATTRIBS ])->setChecked( true );

    // Popup menu for the characters
    m_cmImportChar = new QAction( ::getBaseStringTable()->getString( StringList::ImportCharacter ), this);
    m_cmImportChar->setStatusTip(tr("Import new character file to party, replacing any existing character."));
    connect(m_cmImportChar, SIGNAL(triggered()), this, SLOT(cmImportChar()));

    m_cmExportChar = new QAction( ::getBaseStringTable()->getString( StringList::ExportCharacter ), this);
    m_cmExportChar->setStatusTip(tr("Export character as a separate file."));
    connect(m_cmExportChar, SIGNAL(triggered()), this, SLOT(cmExportChar()));

    m_cmDropChar = new QAction( ::getBaseStringTable()->getString( StringList::DropCharacter ), this);
    m_cmDropChar->setStatusTip(tr("Drop character from party."));
    connect(m_cmDropChar, SIGNAL(triggered()), this, SLOT(cmDropChar()));

    m_cmRecruitRPCs = new QMenu( ::getBaseStringTable()->getString( StringList::RecruitRPC ) );

    m_cmCopyChar = new QAction( ::getBaseStringTable()->getString( StringList::Copy ), this);
    m_cmCopyChar->setStatusTip(tr("Copy character."));
    connect(m_cmCopyChar, SIGNAL(triggered()), this, SLOT(cmCopyChar()));

    m_cmPasteChar = new QAction( ::getBaseStringTable()->getString( StringList::Paste ), this);
    m_cmPasteChar->setStatusTip(tr("Paste previously copied character into this slot."));
    connect(m_cmPasteChar, SIGNAL(triggered()), this, SLOT(cmPasteChar()));

    resetScreen( m_party->m_chars[ m_charIdx], m_party );

    this->update();
}

ScreenCommon::~ScreenCommon()
{
    if (m_copiedChar)
        delete m_copiedChar;

    delete m_currentScreen;
    delete m_charSelect;
    delete m_pageSelect;

    m_cmImportChar->deleteLater();
    m_cmExportChar->deleteLater();
    m_cmDropChar->deleteLater();
    m_cmRecruitRPCs->deleteLater();
    m_cmCopyChar->deleteLater();
    m_cmPasteChar->deleteLater();
}

void ScreenCommon::rebuildRPCMenu( QMenu *m)
{
    QStringList s;

    m->clear();

    QMapIterator<int, QString>  i( m_party->getKnownRPCs() );
    while (i.hasNext())
    {
        QString rpcName;

        i.next();
        if (! s.contains( i.value() ) )
            rpcName = i.value();
        else
        {
            // Obviously this only deals with a single level of duplication.
            // It's intended for RFS-81 which has a 'broken' and 'repaired' version
            rpcName = QString("%1 (2)").arg(i.value());
        }
        s << rpcName;

        QAction *r = new QAction( rpcName, NULL );
        r->setData( i.key() );
        r->setStatusTip(tr("Install RPC character into this slot."));
        connect( r, SIGNAL(triggered()), this, SLOT(cmRecruitRPC()));

        m->addAction( r );
    }

    // Disable the menu if there are no RPCs listed (ie. most likely a NEW game)
    // Enable it otherwise

    m->setEnabled( s.isEmpty() ? false : true );
}

QPixmap ScreenCommon::getPortraitFromFilePath( QString portraitPath )
{
    QDir         cwd = SLFFile::getWizardryPath();
    QPixmap      img;

    QStringList  entries;
    QStringList  filter;

    QString normalisedPath = portraitPath.replace("\\", "/").toUpper();

    // speed this up a bit by getting into the first folder at least - which should be USER
    if (normalisedPath.startsWith("USER/"))
    {
        filter << "USER";

        portraitPath = normalisedPath.mid( 5 );

        entries = cwd.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );

        if (entries.size() == 1)
        {
            cwd.cd( entries.at(0) );

            QDirIterator it( cwd, QDirIterator::Subdirectories);

            while (it.hasNext())
            {
                QString file = it.next();

                if (file.compare( cwd.absoluteFilePath( portraitPath ), Qt::CaseInsensitive ) == 0)
                {
                    QFile     portrait(file);

                    if (portrait.open(QFile::ReadOnly))
                    {
                        QByteArray array = portrait.readAll();
                        STI c( array );

                        img = QPixmap::fromImage( c.getImage( 0 ) );

                        portrait.close();
                    }
                    break;
                }
            }
        }
    }
    return img;
}

int ScreenCommon::getInternalPortraitCount()
{
    return sizeof(sti_portraits)/sizeof(QString);
}

QPixmap ScreenCommon::getSmallPortrait(int portraitIndex)
{
    if (portraitIndex >= getInternalPortraitCount())
    {
        if (!isWizardry128())
        {
            // Force a wraparound to the first image when we try to draw it,
            // so we don't exceed the array bounds. Whatever is using the
            // out of range index will continue to do so for what gets
            // written to file, though.
            portraitIndex = 0;
        }
        else
        {
            // Wizardry 8 has support for additional images beyond those
            // hardcoded in the exe.
            portraitIndex -= getInternalPortraitCount();

            QString portraitPath = ::getSmallPortraitFromPortraitDB( portraitIndex );
//            qDebug() << "SMALL" << portraitPath;

            return getPortraitFromFilePath( portraitPath );
        }
    }

    return SLFFile::getPixmapFromSlf( getSmallPortraitName( portraitIndex ), 0 );
}

QPixmap ScreenCommon::getMediumPortrait(int portraitIndex)
{
    if (portraitIndex >= getInternalPortraitCount())
    {
        if (!isWizardry128())
        {
            // Force a wraparound to the first image when we try to draw it,
            // so we don't exceed the array bounds. Whatever is using the
            // out of range index will continue to do so for what gets
            // written to file, though.
            portraitIndex = 0;
        }
        else
        {
            // Wizardry 8 has support for additional images beyond those
            // hardcoded in the exe.
            portraitIndex -= getInternalPortraitCount();

            QString portraitPath = ::getMediumPortraitFromPortraitDB( portraitIndex );
//            qDebug() << "MEDIUM" << portraitPath;

            return getPortraitFromFilePath( portraitPath );
        }
    }

    return SLFFile::getPixmapFromSlf( getMediumPortraitName( portraitIndex ), 0 );
}

QPixmap ScreenCommon::getLargePortrait(int portraitIndex)
{
    if (portraitIndex >= getInternalPortraitCount())
    {
        if (!isWizardry128())
        {
            // Force a wraparound to the first image when we try to draw it,
            // so we don't exceed the array bounds. Whatever is using the
            // out of range index will continue to do so for what gets
            // written to file, though.
            portraitIndex = 0;
        }
        else
        {
            // Wizardry 8 has support for additional images beyond those
            // hardcoded in the exe.
            portraitIndex -= getInternalPortraitCount();

            QString portraitPath = ::getLargePortraitFromPortraitDB( portraitIndex );
//            qDebug() << "LARGE" << portraitPath;

            return getPortraitFromFilePath( portraitPath );
        }
    }

    return SLFFile::getPixmapFromSlf( getLargePortraitName( portraitIndex ), 0 );
}

QString ScreenCommon::getSmallPortraitName(int portraitIndex)
{
    return k_small_portrait + sti_portraits[ portraitIndex ];
}

QString ScreenCommon::getMediumPortraitName(int portraitIndex)
{
    QString  med_portrait_filename = k_medium_portrait + sti_portraits[ portraitIndex ];

    // SLFFile classes don't support the copy constructor because they
    // depend on QFile which also doesn't support it.
    // So have to use this as a pointer type
    SLFFile *portrait_file = new SLFFile( med_portrait_filename );

    if (portrait_file)
    {
        if (! portrait_file->isGood() )
        {
            // irritatingly some of the portraits use a different file prefix letter, but
            // only in the medium portrait size
            delete portrait_file;
            med_portrait_filename = k_rpcmed_portrait + sti_portraits[ portraitIndex ];
            portrait_file = new SLFFile( med_portrait_filename );
        }
        if (portrait_file->isGood() )
        {
            return med_portrait_filename;
        }
        delete portrait_file;
    }
    return QString();
}

QString ScreenCommon::getLargePortraitName(int portraitIndex)
{
    return k_large_portrait + sti_portraits[ portraitIndex ];
}

bool ScreenCommon::isCustomPortrait(int portraitIndex)
{
    // Not supported in Wizardry 1.2.8 because it ignores the Patch files
    // used to modify the stock pictures in earlier versions
    if (isWizardry128())
        return false;

    if (portraitIndex >= getInternalPortraitCount())
    {
        // prevent an array index out of bounds
        portraitIndex = 0;
    }
    SLFFile slf( k_large_portrait + sti_portraits[ portraitIndex ] );

    if (slf.isGood())
    {
        return slf.isFromPatch();
    }
    return false;
}

QPixmap ScreenCommon::getStatue( character::race r, character::gender g )
{
    char *race = "";
    char *gender = "";

    switch (g)
    {
        case character::gender::Male:    gender = "_M";      break;
        case character::gender::Female:  gender = "_F";      break;
    }

    switch (r)
    {
        case character::race::Human:     race = "HUMAN";     break;
        case character::race::Elf:       race = "ELF";       break;
        case character::race::Dwarf:     race = "DWARF";     break;
        case character::race::Gnome:     race = "GNOME";     break;
        case character::race::Hobbit:    race = "HOBBIT";    break;
        case character::race::Faerie:    race = "FAIRY";     break;
        case character::race::Lizardman: race = "LIZARD";    break;
        case character::race::Dracon:    race = "DRACON";    break;
        case character::race::Felpurr:   race = "FELPURR";   break;
        case character::race::Rawulf:    race = "RAWULF";    break;
        case character::race::Mook:      race = "MOOK";      break;
        case character::race::Trynnie:   race = "TRYNNIE";   gender = "";   break;
        case character::race::TRang:     race = "TRANG";     gender = "";   break;
        case character::race::Umpani:    race = "UMPANI";    gender = "";   break;
        case character::race::Rapax:     race = "RAPAX";     gender = "";   break;
        case character::race::Android:   race = "ANDROID";   gender = "";   break;
    };

    QString sti_nudist = QString ("REVIEW/STATUE_%1%2.STI" ).arg( race ).arg( gender );

    return SLFFile::getPixmapFromSlf( sti_nudist, 0 );
}

void ScreenCommon::resetCharacterSelectButtons(void)
{
    // Set up the character switch buttons - we need to do this every reset, because
    // the picture of the last character could have changed on us
    SLFFile buttons( "REVIEW/COMMONCORNER.STI" );
    if (buttons.open(QFile::ReadOnly))
    {
        QByteArray array = buttons.readAll();
        STI sti_buttons( array );

        for (int k=0; k<NUM_CHARS; k++)
        {
            // These images ARE NOT the same size. We don't get the effect we want
            // if we try to put them all in the one icon, so instead do one case
            // for disabled, and one case for usable button

            QImage   no_char = sti_buttons.getImage( k+1 );

            QIcon icon;
            QSize iconSize;
            if (! m_party->m_chars[ k ]->isNull())
            {
                QPixmap  rl_char = getSmallPortrait( m_party->m_chars[ k ]->getPortraitIndex() );

                if (! rl_char.isNull())
                {
                    QPainter painter;

                    // inactive = unmodified image
                    icon.addPixmap( rl_char, QIcon::Normal,   QIcon::Off );

                    // * mouseOver should be image with a whitish frame
                    painter.begin(&rl_char);
                    painter.setPen(QColor::fromRgb(0xee, 0xee, 0xd5)); // Whitish
                    painter.drawRect(rl_char.rect().adjusted(0,0,-1,-1));
                    painter.end();
                    icon.addPixmap( rl_char, QIcon::Active,   QIcon::Off );

                    // * depressed should be image with a lt blue
                    painter.begin(&rl_char);
                    painter.setPen(QColor::fromRgb(0x63, 0x91, 0xb2)); // Light blue
                    painter.drawRect(rl_char.rect().adjusted(0,0,-1,-1));
                    painter.end();
                    icon.addPixmap( rl_char, QIcon::Normal,   QIcon::On  );
                    icon.addPixmap( rl_char, QIcon::Active,   QIcon::On  );

                    iconSize = QSize( rl_char.width(), rl_char.height() );
                }
            }

            if (icon.isNull())
            {
                // unoccupied character slot
                if (! no_char.isNull())
                {
                    // button is disabled and only needs disabled icons
                    icon.addPixmap( QPixmap::fromImage( no_char ), QIcon::Disabled, QIcon::Off );
                    icon.addPixmap( QPixmap::fromImage( no_char ), QIcon::Disabled, QIcon::On  );
                }
                iconSize = QSize( sti_buttons.getWidth( k+1 ), sti_buttons.getHeight( k+1 ) );
            }

            ((WButton *)m_widgets[PLAYER_START + k])->setIcon( icon );
            ((WButton *)m_widgets[PLAYER_START + k])->setIconSize( iconSize );

            // Reapply the scale to apply any icon size change that could have occurred
            ((WButton *)m_widgets[PLAYER_START + k])->setScale( m_scale );
        }
        buttons.close();
    }

    // Set the character buttons so they can be rearranged by drag and drop
    for (int k = PLAYER_START; k < PLAYER_END; k++)
    {
        if (WButton *w = qobject_cast<WButton *>(m_widgets[ k ]))
        {
            int slot_type = 0;

            if (m_party->m_chars[ k - PLAYER_START ]->isNull())
                slot_type |= WButton::EMPTY_FLAG;

            w->setDraggable( "application/x-wiz8-char", (k - PLAYER_START) | slot_type);

            disconnect( w, SIGNAL(buttonDropped(int)), this, SLOT(characterMoved(int)) );
            connect(    w, SIGNAL(buttonDropped(int)), this, SLOT(characterMoved(int)) );

            disconnect( w, SIGNAL(contextMenu(QPoint)), this, SLOT(characterPopup(QPoint)) );
            connect(    w, SIGNAL(contextMenu(QPoint)), this, SLOT(characterPopup(QPoint)) );
        }
    }
}

void ScreenCommon::setVisible( bool visible )
{
    QObjectList kids = children();

    for (int k=0; k<kids.size(); k++)
    {
        if (QWidget *q = qobject_cast<QWidget *>(kids.at(k)))
        {
            // FIXME: Needs a bit in here to not show DDLs that aren't active
            q->setVisible( visible );
        }
    }
    // but not the levels overlay
    showLevelsOverlay( false );

    QWidget::setVisible( visible );
}

void ScreenCommon::showLevelsOverlay( bool show )
{
    for (int k=OVL_LEVELS; k<OVL_LEVELS_END; k++)
    {
        if (QWidget *q = qobject_cast<QWidget *>(m_widgets[ k ]))
        {
            q->setVisible( show );
        }
    }
}

void ScreenCommon::changeName(bool down)
{
    if (down)
    {
        character *c = m_party->m_chars[ m_charIdx ];

        ScreenPersonality *s = new ScreenPersonality( c, this );
        connect(s,    SIGNAL(changedName(QString)), this, SLOT(changedName(QString)));
        connect(s,    SIGNAL(changedPortrait()),    this, SLOT(changedPortrait()));
        connect(this, SIGNAL(languageReset()),      s,    SLOT(resetLanguage()));
        s->setVisible(true);
        this->update();
    }
}

void ScreenCommon::changedName(QString name)
{
    setWindowTitle(name);

    if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ VAL_NAME ]))
    {
        q->setText( name );
    }
}

void ScreenCommon::changedRace()
{
    if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ VAL_RACE ]))
    {
        q->setText( m_party->m_chars[m_charIdx]->getGenderString() + " " + m_party->m_chars[m_charIdx]->getRaceString() );
    }
}

void ScreenCommon::changedSex()
{
    // Race and sex are in the same text box
    changedRace();
}

void ScreenCommon::changedProf()
{
    if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ VAL_PROF ]))
    {
        q->setText( m_party->m_chars[m_charIdx]->getProfessionString() );
    }
    // Changing the profession also affects the level string
    changedLevel();
}

void ScreenCommon::changedLevel()
{
    if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ VAL_LEVEL ]))
    {
        q->setText( ::getBaseStringTable()->getString( StringList::Level ) +
                    " " + QString::number(m_party->m_chars[m_charIdx]->getCurrentLevel()) +
                    " (" + m_party->m_chars[m_charIdx]->getCurrentLevelString() + ")" );
    }
}

void ScreenCommon::changedPortrait()
{
    resetCharacterSelectButtons();

    // update the current character picture
    if (WImage *w = qobject_cast<WImage *>(m_widgets[ VAL_PORTRAIT ]))
    {
        if (! m_party->m_chars[ m_charIdx ]->isNull())
        {
            w->setPixmap( getMediumPortrait( m_party->m_chars[ m_charIdx ]->getPortraitIndex() ) );
        }
    }
}

void ScreenCommon::setSumWeapon( character::worn weapon )
{
    QPixmap pix;
    QString sti_file = "";
    int     sti_idx = 0;

    const item w = m_party->m_chars[ m_charIdx ]->getItem( weapon );
    if (w.isNull())
    {
        if (weapon == character::worn::Weapon1a)
            sti_file = "MAIN INTERFACE/CLAWRIGHT.STI";
        else
            sti_file = "MAIN INTERFACE/CLAWLEFT.STI";
    }
    else
    {
        sti_file = "ITEMS/" + w.getStiFile().toUpper();
        // weapons have an index 2 small enough to fit here
        sti_idx  = 2;
    }

    pix = SLFFile::getPixmapFromSlf( sti_file, sti_idx );
    if (pix.size().height() > 26)
    {
        pix = pix.scaledToHeight( 26, Qt::SmoothTransformation );
    }

    WImage *qImage = qobject_cast<WImage *>(m_widgets[ (weapon == character::worn::Weapon1a) ? VAL_SUM_WEAPON1A : VAL_SUM_WEAPON1B ]);

    qImage->setPixmap( pix );
}

void ScreenCommon::charswap(bool)
{
    int checkedIdx = m_charSelect->checkedId();

    if (m_party->m_chars[ checkedIdx ]->isNull())
    {
        // reject the swap
        m_charSelect->button( m_charIdx )->setChecked( true );
    }
    else
    {
        m_charIdx = checkedIdx;
        resetScreen( m_party->m_chars[m_charIdx], m_party );
        this->update();
    }
}

void ScreenCommon::updateChars(party *p)
{
    m_party = p;

    m_charIdx = m_charSelect->checkedId();
    if (m_party->m_chars[ m_charIdx ]->isNull())
    {
        m_charIdx = 2;
        m_charSelect->button( m_charIdx )->setChecked( true );
    }

    resetCharacterSelectButtons();

    resetScreen( m_party->m_chars[m_charIdx], m_party );
    this->update();
}

void ScreenCommon::resetScreen(void *char_tag, void *party_tag)
{
    character *c = (character *)char_tag;
    party     *p = (party *)party_tag;

    if (c->isNull())
    {
        setWindowTitle("");

        struct { int id; QString str; } vals[] =
        {
            { VAL_NAME,     "" },
            { VAL_RACE,     "" },
            { VAL_PROF,     "" },
            { VAL_LEVEL,    "" },
            { VAL_SUM_AC,   "" },
            { VAL_SUM_PROF, "" },
            { VAL_SUM_HP,   "" },
            { -1, "" }
        };

        for (int k=0; vals[k].id != -1; k++)
        {
            if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ vals[k].id ]))
            {
                q->setText( vals[k].str );
            }
        }
        if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_HP_BAR ]))
        {
            q->setLengthRestrict( Qt::AlignBottom, 1.0, 0.0 );
        }
        if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_STAMINA_BAR ]))
        {
            q->setLengthRestrict( Qt::AlignBottom, 1.0, 0.0 );
        }
        if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_MP_BAR ]))
        {
            q->setLengthRestrict( Qt::AlignBottom, 1.0, 0.0 );
        }
        if (WImage *w = qobject_cast<WImage *>(m_widgets[ VAL_PORTRAIT ]))
        {
            w->setPixmap( QPixmap() );
        }
        if (WImage *w = qobject_cast<WImage *>(m_widgets[ VAL_SUM_WEAPON1A ]))
        {
            w->setPixmap( QPixmap() );
        }
        if (WImage *w = qobject_cast<WImage *>(m_widgets[ VAL_SUM_WEAPON1B ]))
        {
            w->setPixmap( QPixmap() );
        }

        showLevelsOverlay( false );

        m_currentScreen->setVisible( false );

        // Disable and hide all the page buttons
        if (WButton *w = qobject_cast<WButton *>(m_widgets[ PAGE_ATTRIBS ]))
        {
            w->setDisabled( true );
            w->setVisible( false );
        }
        if (WButton *w = qobject_cast<WButton *>(m_widgets[ PAGE_ITEMS ]))
        {
            w->setDisabled( true );
            w->setVisible( false );
        }
        if (WButton *w = qobject_cast<WButton *>(m_widgets[ PAGE_MAGIC ]))
        {
            w->setDisabled( true );
            w->setVisible( false );
        }
        if (WButton *w = qobject_cast<WButton *>(m_widgets[ PAGE_SKILLS ]))
        {
            w->setDisabled( true );
            w->setVisible( false );
        }
        if (WButton *w = qobject_cast<WButton *>(m_widgets[ PAGE_LEVELS ]))
        {
            w->setDisabled( true );
            w->setVisible( false );
        }
    }
    else
    {
        setWindowTitle(c->getName());

        // Populate text value fields with values from the character

        struct { int id; QString str; } vals[] =
        {
            { VAL_NAME,     c->getName()                                              },
            { VAL_RACE,     c->getGenderString() + " " + c->getRaceString()           },
            { VAL_PROF,     c->getProfessionString()                                  },
            { VAL_LEVEL,    ::getBaseStringTable()->getString( StringList::Level ) +
                            " " + QString::number(c->getCurrentLevel()) +
                            " (" + c->getCurrentLevelString() + ")"                   },

            { VAL_SUM_AC,   QString::number( c->getAC_Average() )                     },
            { VAL_SUM_PROF, c->getProfessionString().left(3).toUpper()                },
            { VAL_SUM_HP,   QString::number(c->getHp(character::atIdx::Current))      },

            { -1, "" }
        };

        for (int k=0; vals[k].id != -1; k++)
        {
            if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ vals[k].id ]))
            {
                q->setText( vals[k].str );
            }
        }

        // The abbreviated profession text that appears to the right of the
        // portrait is coloured based on the position in the party
        if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ VAL_SUM_PROF ]))
        {
            QColor colrs[] = { QColor(0xc6, 0x00, 0x21), // Red
                               QColor(0x29, 0x8c, 0x00), // Green
                               QColor(0x9c, 0x18, 0x7b), // Purple
                               QColor(0x00, 0xa5, 0xde), // Lt Blue
                               QColor(0xde, 0x63, 0x00), // Orange
                               QColor(0xff, 0xef, 0x00), // Yellow
                               QColor(0xff, 0x84, 0xb5), // Pink
                               QColor(0x6b, 0x4a, 0x21)  // Brown
                             };


            int colrIdx = m_party->m_chars[ m_charIdx ]->getColorIdx();

            q->setStyleSheet( QString("QLabel {color: %1}").arg(colrs[colrIdx].name()) );
        }

        // HP, Stamina and MP bars next to portrait
        if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_HP_BAR ]))
        {
            double nom   = c->getHp(character::atIdx::Current);
            double denom = c->getHp(character::atIdx::Base);

            if (denom == 0.0) denom = 1.0; // prevent division by zero
            q->setLengthRestrict( Qt::AlignBottom, 1.0, nom / denom );
        }
        if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_STAMINA_BAR ]))
        {
            double nom   = c->getStamina(character::atIdx::Current);
            double denom = c->getStamina(character::atIdx::Base);

            if (denom == 0.0) denom = 1.0; // prevent division by zero
            q->setLengthRestrict( Qt::AlignBottom, 1.0, nom / denom );
        }
        if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_MP_BAR ]))
        {
            double nom   = c->getMp( character::realm::REALM_SIZE, character::atIdx::Current );
            double denom = c->getMp( character::realm::REALM_SIZE, character::atIdx::Base );

            if (denom == 0.0) denom = 1.0; // prevent division by zero
            q->setLengthRestrict( Qt::AlignBottom, 1.0, nom / denom );
        }

        // Change the colour of the Load text based on percent of maximum
        WLabel *acLabel = qobject_cast<WLabel *>(m_widgets[ VAL_SUM_AC ]);
        if (acLabel)
        {
            quint32 colr = 0;

            switch (c->getLoadCategory())
            {
                default:
                case 0:
                    colr = 0xe0e0c3; // Light Yellow
                    break;

                case 1:
                    colr = 0x00ade3; // Blue
                    break;

                case 2:
                    colr = 0x169e16; // Green
                    break;

                case 3:
                    colr = 0xfcf449; // Yellow
                    break;

                case 4:
                    colr = 0xec000f; // Red
                    break;
            }
            acLabel->setStyleSheet( QString::asprintf("QLabel {color: #%06x}", colr) );
        }

        // set the picture to the newly selected character
        if (WImage *w = qobject_cast<WImage *>(m_widgets[ VAL_PORTRAIT ]))
        {
            w->setPixmap( getMediumPortrait( m_party->m_chars[ m_charIdx ]->getPortraitIndex() ) );
        }

        // Assign personal items
        setSumWeapon( character::worn::Weapon1a );
        setSumWeapon( character::worn::Weapon1b );

        showLevelsOverlay( false );

        m_currentScreen->resetScreen( c, p );
        m_currentScreen->setVisible( true );

        // Enable and show all the page buttons
        if (WButton *w = qobject_cast<WButton *>(m_widgets[ PAGE_ATTRIBS ]))
        {
            w->setDisabled( false );
            w->setVisible( true );
        }
        if (WButton *w = qobject_cast<WButton *>(m_widgets[ PAGE_ITEMS ]))
        {
            w->setDisabled( false );
            w->setVisible( true );
        }
        if (WButton *w = qobject_cast<WButton *>(m_widgets[ PAGE_MAGIC ]))
        {
            w->setDisabled( false );
            w->setVisible( true );
        }
        if (WButton *w = qobject_cast<WButton *>(m_widgets[ PAGE_SKILLS ]))
        {
            w->setDisabled( false );
            w->setVisible( true );
        }
        if (WButton *w = qobject_cast<WButton *>(m_widgets[ PAGE_LEVELS ]))
        {
            w->setDisabled( false );
            w->setVisible( true );
        }
    }

    rebuildRPCMenu( m_cmRecruitRPCs );
}

void ScreenCommon::professionLevels(bool down)
{
    if (WLabel *w = qobject_cast<WLabel *>(m_widgets[ VAL_PROF ]))
    {
        // Only show the levels overlay if the character has a profession -
        // empty slots don't
        if (!w->text().isEmpty())
        {
            character *c = m_party->m_chars[ m_charIdx ];

            // Populate the data on the Professions by Level overlay
            // (this gets displayed when you mouseDown on a character's
            // profession)
            for (int j = 0, k = VAL_PROF1; k <= VAL_PROF16; k+=2)
            {
                QString profession;
                QString level;
                int     lvl = -1;

                if (c->getProfessionLevel( j++, profession, lvl))
                {
                    level = QString::number( lvl );
                }
                else
                {
                    profession = "";
                    level      = "";
                }

                if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ k  ]))
                {
                    q->setText( profession );
                }
                if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ k + 1 ]))
                {
                    q->setText( level );
                }
            }

            showLevelsOverlay( down );
        }
    }
}

void ScreenCommon::reviewItems(bool down)
{
    if (down)
    {
        m_party->m_chars[m_charIdx]->recomputeEverything();

        if (m_currentScreen)
            delete m_currentScreen;
        m_currentScreen = new ScreenItems( m_party, m_party->m_chars[m_charIdx], this );
        m_currentScreen->lower();
        m_currentScreen->setVisible(true);
        this->update();
    }
}

void ScreenCommon::reviewMagic(bool down)
{
    if (down)
    {
        m_party->m_chars[m_charIdx]->recomputeEverything();

        if (m_currentScreen)
            delete m_currentScreen;
        m_currentScreen = new ScreenMagic( m_party->m_chars[m_charIdx], this );
        m_currentScreen->lower();
        m_currentScreen->setVisible(true);
        this->update();
    }
}

void ScreenCommon::reviewSkills(bool down)
{
    if (down)
    {
        m_party->m_chars[m_charIdx]->recomputeEverything();

        if (m_currentScreen)
            delete m_currentScreen;
        m_currentScreen = new ScreenSkills( m_party, m_party->m_chars[m_charIdx], this );
        m_currentScreen->lower();
        m_currentScreen->setVisible(true);
        this->update();
    }
}

void ScreenCommon::reviewLevels(bool down)
{
    if (down)
    {
        m_party->m_chars[m_charIdx]->recomputeEverything();

        if (m_currentScreen)
            delete m_currentScreen;
        m_currentScreen = new ScreenLevels( m_party->m_chars[m_charIdx], this );
        m_currentScreen->lower();
        m_currentScreen->setVisible(true);

        connect( m_currentScreen, SIGNAL(changedLevel()), this, SLOT(changedLevel()) );

        this->update();
    }
}

void ScreenCommon::reviewAttribs(bool down)
{
    if (down)
    {
        m_party->m_chars[m_charIdx]->recomputeEverything();

        if (m_currentScreen)
            delete m_currentScreen;
        m_currentScreen = new ScreenAttribs( m_party->m_chars[m_charIdx], this );
        m_currentScreen->lower();
        m_currentScreen->setVisible(true);

        connect( m_currentScreen, SIGNAL(changedRace()), this, SLOT(changedRace()) );
        connect( m_currentScreen, SIGNAL(changedProf()), this, SLOT(changedProf()) );
        connect( m_currentScreen, SIGNAL(changedSex()),  this, SLOT(changedSex())  );

        this->update();
    }
}

void ScreenCommon::characterMoved(int tag)
{
    if (WButton *w = qobject_cast<WButton *>(sender()))
    {
        int orig_posn = tag;
        int dest_posn = w->getTag();

        if (orig_posn != dest_posn)
        {
            // Move the character from a previous position to a new position,
            // reordering other characters as appropriate.

            if (((orig_posn >= 2) && (dest_posn <  2)) ||
                ((orig_posn <  2) && (dest_posn >= 2)))
            {
                // Moving RPCs into PC area or vice versa - this WILL affect gameplay
                // but sometimes that's nice. RPCs stop abandoning you or suffering Hex
                // derangements when they go somewhere they don't want to go for example.

                // At least in 1.2.4 - not sure what happens in 1.2.8. Certainly RPCs
                // recruited _normally_ in 1.2.8 and put into player slots continue to
                // behave as normal, so assume RPCs moved by us in 1.2.8 do the same.
            }

            character *selectedChar = m_party->m_chars[m_charIdx];

            if (m_party->m_chars[dest_posn]->isNull())
            {
                // Move directly into any blank position straight away, be it in RPC or PC area
                m_party->m_chars.swapItemsAt(orig_posn, dest_posn);
            }
            else if ((dest_posn < 2) &&                 // RPC area
                     ((m_party->m_chars[0]->isNull()) || // and one of the slots is empty
                      (m_party->m_chars[1]->isNull())))
            {
                // Try to keep a second RPC character in the RPC area if possible
                Q_ASSERT(orig_posn >= 2);
                m_party->m_chars.swapItemsAt(0, 1);

                Q_ASSERT(m_party->m_chars[dest_posn]->isNull());
                m_party->m_chars.swapItemsAt(orig_posn, dest_posn);
            }
            else
            {
                m_party->m_chars.move(orig_posn, dest_posn);
            }

            for (int k=2, blnks=0; k + blnks < NUM_CHARS; )
            {
                if (m_party->m_chars[k]->isNull())
                {
                    m_party->m_chars.move(k, NUM_CHARS-1);
                    blnks++;
                }
                else
                {
                    k++;
                }
            }

            m_charIdx = m_party->m_chars.indexOf( selectedChar );

            m_charSelect->button( m_charIdx )->setChecked( true );
            m_party->resetCharacterColors();

            resetCharacterSelectButtons();
        }
    }
}

void ScreenCommon::characterPopup(QPoint point)
{
    if (WButton *w = qobject_cast<WButton *>(sender()))
    {
        m_cmTargetCharIdx = w->getTag();

        if (m_party->m_chars[ m_cmTargetCharIdx ]->isNull())
        {
            m_cmExportChar->setEnabled(false);
            m_cmDropChar->setEnabled(false);
            m_cmCopyChar->setEnabled(false);
        }
        else
        {
            m_cmExportChar->setEnabled(true);
            m_cmDropChar->setEnabled(true);
            m_cmCopyChar->setEnabled(true);
        }

        if (m_copiedChar)
        {
            m_cmPasteChar->setEnabled(true);
        }
        else
        {
            m_cmPasteChar->setEnabled(false);
        }

        QMenu menu(w);

        menu.addAction(m_cmImportChar);
        menu.addAction(m_cmExportChar);
        menu.addAction(m_cmDropChar);
        menu.addMenu(m_cmRecruitRPCs);

        menu.addSeparator();

        menu.addAction(m_cmCopyChar);
        menu.addAction(m_cmPasteChar);

        menu.exec( point );
    }
}

void ScreenCommon::cmRecruitRPC()
{
    if (QAction *src = qobject_cast<QAction *>( sender() ) )
    {
        int npc_idx = src->data().toInt();

        // Make sure there are no gaps in front of this character if it is in the PC Area
        while ((m_cmTargetCharIdx > 2) && m_party->m_chars[m_cmTargetCharIdx-1]->isNull())
        {
            m_cmTargetCharIdx--;
        }

        DialogRUSure check(tr("This option can crash the Wizardry game if misused. "
                              "The crash can occur when the party contacts an RPC already "
                              "in the party. Any maps already visited are therefore automatically "
                              "filtered to prevent this, but if you recruit an RPC here before "
                              "having met them the first time in a game, the crash can happen "
                              "much later on when you do meet them.\n\n"
                              "It is intended to be used to recover a dismissed RPC who "
                              "can't be re-recruited due to in-game bug causing them to "
                              "vanish or not respond to conversation.\n\n"
                              "Please only use with a backup of your game. Proceed?"), "SuppressWarningRPC", 420, 260);

        if (check.exec() == QDialog::Accepted)
        {
            emit loadRPCintoSlot( npc_idx, m_cmTargetCharIdx );
        }
    }
}

void ScreenCommon::cmImportChar()
{
    QString &wizardryPath = SLFFile::getWizardryPath();

    QString openChar = ::getOpenFileName(this, tr("Import Character"),  wizardryPath + "/Saves/Characters", tr("Characters (*.chr)"));
    if (openChar.isEmpty())
        return;

    if (QMainWindow *w = qobject_cast<QMainWindow *>(parent()))
    {
        w->statusBar()->showMessage(QString( tr("Importing %1.") ).arg( openChar ));
    }

    QFile chrFile(openChar);

    // There's no protection here to prevent a character being loaded multiple times;
    // if that's what someone wants to do go for it. But the game probably won't like
    // it much if they don't rename the characters afterwards.

    if (chrFile.open(QIODevice::ReadOnly))
    {
        QByteArray  header = chrFile.read( 4 );
        QByteArray  chr    = chrFile.read( RIFFFile::CHARACTER_SIZE );

        quint8     *hbytes = (quint8 *) header.data();
        quint8     *cbytes = (quint8 *) chr.data();

        if ((chr.size() == RIFFFile::CHARACTER_SIZE) &&
            (chr.size() == FORMAT_LE32(hbytes)))
        {
            // Exported characters need to turn on the byte that says they are in the party
            ASSIGN_LE8(  cbytes+0x0004, 1);
            ASSIGN_LE32( cbytes+0x0000, 0); // and turn this one off

            // We don't have any character extra info to restore, so if the player
            // has restored a RPC all that info has been lost - the better option
            // is to Recruit an RPC from the existing game.
            QByteArray newCharX   = QByteArray( RIFFFile::CHARACTER_EXTRA_SIZE, 0 );
            qint8     *charx_data = (qint8 *) newCharX.data();

            // Don't actually know what these fields represent
            ASSIGN_LE32( charx_data+0x0071, -1 );
            ASSIGN_LE8(  charx_data+0x00d0, -1 );
            ASSIGN_LE32( charx_data+0x00fa, -1 ); // RPC idx (order in NPCT segment)

            // Make sure there are no gaps in front of this character if it is in the PC Area
            while ((m_cmTargetCharIdx > 2) && m_party->m_chars[m_cmTargetCharIdx-1]->isNull())
            {
                m_cmTargetCharIdx--;
            }

            delete m_party->m_chars[m_cmTargetCharIdx];
            m_party->m_chars[m_cmTargetCharIdx] = new character( chr, newCharX, isWizardry128() );
            m_party->resetCharacterColors();

            // And you need to refresh the screen afterwards
            resetCharacterSelectButtons();

            resetScreen( m_party->m_chars[m_charIdx], m_party );
            this->update();

            emit partyViable();
        }
        else
        {
            if (QMainWindow *w = qobject_cast<QMainWindow *>(parent()))
            {
                w->statusBar()->showMessage(tr("This is not a valid character file!"));
            }
        }
    }
    else
    {
        if (QMainWindow *w = qobject_cast<QMainWindow *>(parent()))
        {
            w->statusBar()->showMessage(tr("Cannot open character file!"));
        }
    }
}

void ScreenCommon::cmExportChar()
{
    QString &wizardryPath = SLFFile::getWizardryPath();

    QString saveChar = ::getSaveFileName(this, tr("Save Character"),  wizardryPath + "/Saves/Characters", tr("Characters (*.chr)"));
    if (saveChar.isEmpty())
        return;

    if (!saveChar.endsWith(".CHR", Qt::CaseInsensitive))
        saveChar += ".CHR";

    if (QMainWindow *w = qobject_cast<QMainWindow *>(parent()))
    {
        w->statusBar()->showMessage(QString( tr("Saving %1 to %2.") ).arg( m_party->m_chars[m_cmTargetCharIdx]->getName() ).arg( saveChar ));
    }

    QFile chrFile(saveChar);

    if (chrFile.open(QIODevice::WriteOnly))
    {
        // The format of the CHR file is determined based on the EXE - 1.2.8 or 1.2.4 in this case
        QByteArray  chr = m_party->m_chars[m_cmTargetCharIdx]->serialize( isWizardry128() );
        QByteArray  header(4, 0);
        quint8     *hbytes = (quint8 *) header.data();
        quint8     *cbytes = (quint8 *) chr.data();

        // Exported characters need to turn off the byte that says they are in the party
        ASSIGN_LE8(  cbytes+0x0004, 0);
        ASSIGN_LE32( cbytes+0x0000, 1); // and turn this one on

        ASSIGN_LE32( hbytes+0x0000, chr.size());

        chrFile.write( header );
        chrFile.write( chr );

        chrFile.close();

        if (QMainWindow *w = qobject_cast<QMainWindow *>(parent()))
        {
            w->statusBar()->showMessage(tr("Character saved."));
        }
    }
    else
    {
        if (QMainWindow *w = qobject_cast<QMainWindow *>(parent()))
        {
            w->statusBar()->showMessage(tr("Cannot open file for save!"));
        }
    }
}

void ScreenCommon::cmDropChar()
{
    delete m_party->m_chars[m_cmTargetCharIdx];

    m_party->m_chars[m_cmTargetCharIdx] = new character();

    if (m_cmTargetCharIdx >= 2)
    {
        // push the blank slot to the back
        m_party->m_chars.move( m_cmTargetCharIdx, NUM_CHARS-1 );
    }
    // If you just deleted the last indexed character in the party
    // select an earlier character
    while ((m_charIdx > 0) && m_party->m_chars[m_charIdx]->isNull())
        m_charIdx--;
    m_party->resetCharacterColors();

    resetCharacterSelectButtons();

    resetScreen( m_party->m_chars[m_charIdx], m_party );
    this->update();

    bool no_chars = true;
    for (int k=0; k<m_party->m_chars.size(); k++)
    {
        if (!m_party->m_chars[k]->isNull())
        {
            no_chars = false;
            break;
        }
    }
    if (no_chars)
    {
        emit partyEmpty();
    }
}

void ScreenCommon::cmCopyChar()
{
    if (m_copiedChar)
        delete m_copiedChar;

    m_copiedChar = new character( *m_party->m_chars[m_cmTargetCharIdx] );
}

void ScreenCommon::cmPasteChar()
{
    // Make sure there are no gaps in front of this character if it is in the PC Area
    while ((m_cmTargetCharIdx >= 2) && m_party->m_chars[m_cmTargetCharIdx-1]->isNull())
    {
        m_cmTargetCharIdx--;
    }

    delete m_party->m_chars[m_cmTargetCharIdx];
    m_party->m_chars[m_cmTargetCharIdx] = new character( *m_copiedChar );
    m_party->resetCharacterColors();

    // And you need to refresh the screen afterwards
    resetCharacterSelectButtons();

    resetScreen( m_party->m_chars[m_charIdx], m_party );
    this->update();
}

void ScreenCommon::exitButton(bool checked)
{
    (void)checked;

    emit exit();
}

QPixmap ScreenCommon::makeAttribsButton(int state)
{
    QPixmap buttonMagic  = SLFFile::getPixmapFromSlf( "REVIEW/REVIEWPAGEBUTTONS.STI", 10 + state );

    QColor fillColor = buttonMagic.toImage().pixel( 28, 8 );

    QPixmap buttonAttribs( buttonMagic.size() );

    QPainter p;

    p.begin( &buttonAttribs );
    p.drawPixmap(   0,   0, buttonMagic );
    p.drawPixmap(  12,  13, buttonMagic,   8,  13, 12,   7 );
    p.drawPixmap(  24,   5, buttonMagic,  31,   5,  9,  15 );
    p.fillRect( 33, 5, 6, 15, fillColor );
    p.end();

    return buttonAttribs;
}

void ScreenCommon::resetLanguage()
{
    resetScreen( m_party->m_chars[m_charIdx], m_party );
    this->update();

    // When the language option gets changed, absolutely everything (including
    // all the text labels) has to be redrawn

    if (ScreenAttribs *s = qobject_cast<ScreenAttribs *> (m_currentScreen))
    {
        Q_UNUSED(s);
        reviewAttribs(true);
    }
    else if (ScreenLevels *s = qobject_cast<ScreenLevels *> (m_currentScreen))
    {
        Q_UNUSED(s);
        reviewLevels(true);
    }
    else if (ScreenItems *s = qobject_cast<ScreenItems *> (m_currentScreen))
    {
        Q_UNUSED(s);
        reviewItems(true);
    }
    else if (ScreenMagic *s = qobject_cast<ScreenMagic *> (m_currentScreen))
    {
        Q_UNUSED(s);
        reviewMagic(true);
    }
    else if (ScreenSkills *s = qobject_cast<ScreenSkills *> (m_currentScreen))
    {
        Q_UNUSED(s);
        reviewSkills(true);
    }

    // ScreenPersonality listens for this signal if it is active
    emit languageReset();
}
