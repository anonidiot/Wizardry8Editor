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

#include <QMediaPlayer>

#include <QApplication>
#include <QColor>
#include <QHelpEvent>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>

#include "common.h"
#include "ScreenLevels.h"
#include "DialogSpellInfo.h"

#include "DialogDuration.h"
#include "DialogInfo.h"

#include "StringList.h"
#include "Window3DNavigator.h"
#include "WButton.h"
#include "WCheckBox.h"
#include "WDDL.h"
#include "WImage.h"
#include "WLabel.h"
#include "WLineEdit.h"
#include "WSpinBox.h"
#include "WStatBar.h"

#include "SLFFile.h"

#include "spell.h"
#include "main.h"
#include "Level.h"

#include <QDebug>

typedef enum
{
    NO_ID,

    PROF_INSPECT,

    LBL_PROFS_START,
    LBL_PROFS = LBL_PROFS_START,
    LBL_PROFS_END = LBL_PROFS_START + 15,

    VAL_PROFS_START,
    VAL_PROFS = VAL_PROFS_START,
    VAL_PROFS_END = VAL_PROFS_START + 15,

    CLC_PROFS_START,
    CLC_PROFS = CLC_PROFS_START,
    CLC_PROFS_END = CLC_PROFS_START + 15,

    VAL_XP_LAST,
    VAL_XP_NOW,
    VAL_XP_NEXT,

    BTN_XP_LAST,
    BTN_XP_NOW,
    BTN_XP_NEXT,

    LBL_POS_X,
    LBL_POS_Y,
    LBL_POS_Z,
    LBL_HEADING,

    VAL_POS_X,
    VAL_POS_Y,
    VAL_POS_Z,
    VAL_HEADING,

    BTN_POS_X,
    BTN_POS_Y,
    BTN_POS_Z,
    BTN_HEADING,

    CB_PORTAL_ENABLED,
    BTN_NAVIGATOR,
    LBL_MAP,
    DDL_MAP,

    LBL_COND_START,
    LBL_COND_END = LBL_COND_START + character::condition::CONDITION_SIZE,
    LBL_COND_POSSESSED,

    CB_COND_START,
    CB_COND_END = CB_COND_START + character::condition::CONDITION_SIZE,
    CB_COND_POSSESSED,

    LBL_DUR_START,
    LBL_DUR_END = LBL_DUR_START + character::condition::CONDITION_SIZE,
    LBL_DUR_POSSESSED,

    FKB_DUR_START,
    FKB_DUR_END = FKB_DUR_START + character::condition::CONDITION_SIZE,
    FKB_DUR_POSSESSED,

    LBL_POISON_STR,
    POISON_STRENGTH,

    LBL_HP,
    LBL_STAMINA,
    LBL_SP,

    VAL_HP,
    VAL_STAMINA,
    VAL_SP,

    CLC_HP,
    CLC_STAMINA,
    CLC_SP,

    SIZE_WIDGET_IDS
} widget_ids;

ScreenLevels::ScreenLevels(character *c, QWidget *parent) :
    Screen(parent),
    m_char(c),
    m_inspectMode(false),
    m_initialPopulate(0)
{
    m_metaProf        = QMetaEnum::fromType<character::profession>();

    QPixmap profsBox  = makeProfsBoxPixmap();
    QPixmap healthBox = makeHealthBoxPixmap();
    QPixmap portalBox = makePortalBoxPixmap();
    QPixmap rowImg    = makeRowPixmap();
    QPixmap rowImg2   = makeRowPixmap2();
    QPixmap rowImg3   = makeRowPixmap3();

    QPixmap fakeButton = makeFakeButton();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0, 450,  -1,  -1 ),    new WImage(    "REVIEW/BOTTOMBUTTONBAR.STI",                   0,              this ),  -1,  NULL },

        { NO_ID,              QRect(   0, 164,  -1,  -1 ),    new WImage(    profsBox,                                                       this ),  -1,  NULL },

        { NO_ID,              QRect(  34, 173,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 187,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 201,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 215,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 229,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 243,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 257,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 271,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 285,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 299,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 313,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 327,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 341,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 355,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 369,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },

        { NO_ID,              QRect(  14, 170,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_ICONS_BASE.STI",            2,              this ),  -1,  NULL },

        { NO_ID,              QRect(  34, 397,  -1,  -1 ),    new WImage(    rowImg2,                                                        this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 411,  -1,  -1 ),    new WImage(    rowImg2,                                                        this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 425,  -1,  -1 ),    new WImage(    rowImg2,                                                        this ),  -1,  NULL },

        { NO_ID,              QRect(  10, 390,  -1,  -1 ),    new WImage(    "AUTOMAP/MAP_MONSTERMARKER_A.TGA",       0, 0, 0, 0, 0, 0.6,    this ),  -1,  NULL },

        { PROF_INSPECT,       QRect(   0, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWITEMBUTTONS.STI",                 0, false, 1.0,  this ),  StringList::InspectProfession, SLOT(profDetail(bool)) },

        { LBL_PROFS + 0,      QRect(  34, 173, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Fighter   ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 1,      QRect(  34, 187, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Lord      ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 2,      QRect(  34, 201, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Valkyrie  ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 3,      QRect(  34, 215, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Ranger    ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 4,      QRect(  34, 229, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Samurai   ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 5,      QRect(  34, 243, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Ninja     ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 6,      QRect(  34, 257, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Monk      ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 7,      QRect(  34, 271, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Rogue     ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 8,      QRect(  34, 285, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Gadgeteer ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 9,      QRect(  34, 299, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Bard      ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 10,     QRect(  34, 313, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Priest    ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 11,     QRect(  34, 327, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Alchemist ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 12,     QRect(  34, 341, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Bishop    ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 13,     QRect(  34, 355, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Psionic   ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_PROFS + 14,     QRect(  34, 369, 108,  14 ),    new WLabel( character::getProfessionString( character::profession::Mage      ), Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },

        { CLC_PROFS + 0,      QRect( 142, 173,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 1,      QRect( 142, 187,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 2,      QRect( 142, 201,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 3,      QRect( 142, 215,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 4,      QRect( 142, 229,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 5,      QRect( 142, 243,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 6,      QRect( 142, 257,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 7,      QRect( 142, 271,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 8,      QRect( 142, 285,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 9,      QRect( 142, 299,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 10,     QRect( 142, 313,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 11,     QRect( 142, 327,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 12,     QRect( 142, 341,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 13,     QRect( 142, 355,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_PROFS + 14,     QRect( 142, 369,  82,  14 ),    new WStatBar(  true,                                                            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },

        { VAL_PROFS + 0,      QRect( 230, 173,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 1,      QRect( 230, 187,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 2,      QRect( 230, 201,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 3,      QRect( 230, 215,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 4,      QRect( 230, 229,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 5,      QRect( 230, 243,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 6,      QRect( 230, 257,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 7,      QRect( 230, 271,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 8,      QRect( 230, 285,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 9,      QRect( 230, 299,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 10,     QRect( 230, 313,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 11,     QRect( 230, 327,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 12,     QRect( 230, 341,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 13,     QRect( 230, 355,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_PROFS + 14,     QRect( 230, 369,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spinnerChanged(int)) },

        { NO_ID,              QRect(  34, 397, 186,  14 ),    new WLabel( StringList::ExpLast,            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { VAL_XP_LAST,        QRect( 220, 397,  70,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(lineeditChanged(const QString &)) },
        { BTN_XP_LAST,        QRect( 292, 396,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/PARTYMOVEMENT_BUTTONS.STI",    0, true, 0.70,  this ),  -1,  SLOT(setXpLast(bool)) },
        { NO_ID,              QRect(  34, 411, 186,  14 ),    new WLabel( StringList::ExpEarned,          Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { VAL_XP_NOW,         QRect( 220, 411,  70,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(lineeditChanged(const QString &)) },
        { BTN_XP_NOW,         QRect( 292, 410,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/PARTYMOVEMENT_BUTTONS.STI",    0, true, 0.70,  this ),  -1,  SLOT(setXpNow(bool)) },
        { NO_ID,              QRect(  34, 425, 186,  14 ),    new WLabel( StringList::ExpNext,            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { VAL_XP_NEXT,        QRect( 220, 425,  70,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(lineeditChanged(const QString &)) },
        { BTN_XP_NEXT,        QRect( 292, 424,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/PARTYMOVEMENT_BUTTONS.STI",    0, true, 0.70,  this ),  -1,  SLOT(setXpNext(bool)) },

        // We're using Monster Icons here instead of the Condition Icons, because they're larger and look nicer when scaled up like we're doing;
        // The only Condition that doesn't have a Monster Icon is Drained, so we still need to use the Condition Icon for it - and that means it's
        // missing the circle around the outside that all the others have.
        // TODO: Make a proper icon for it at the right resolution and use that.
// This actually sort of works. The drained icon does blot out some of the circle, but the skull is completely hidden
//        { NO_ID,              QRect( 330, 175,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/DEAD_A.TGA",        0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { CB_COND_START  + character::condition::Drained,   QRect( 330, 170,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 168,  -1,  -1 ),    new WImage(   "ICONS/CONDITIONS/DRAINED.STI",          0,                     this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Drained,   QRect( 366, 170, 108,  14 ),    new WLabel(   StringList::Drained,          Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Drained,   QRect( 440, 168,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Drained,   QRect( 440, 170,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Diseased,  QRect( 330, 190,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 188,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/DISEASED_A.TGA",    0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Diseased,  QRect( 366, 190, 108,  14 ),    new WLabel(   StringList::Diseased,         Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Diseased,  QRect( 440, 188,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Diseased,  QRect( 440, 190,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Irritated, QRect( 330, 210,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 208,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/IRRITATED_A.TGA",   0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Irritated, QRect( 366, 210, 108,  14 ),    new WLabel(   StringList::Irritated,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Irritated, QRect( 440, 208,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Irritated, QRect( 440, 210,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Nauseated, QRect( 330, 230,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 228,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/NAUSEATED_A.TGA",   0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Nauseated, QRect( 366, 230, 108,  14 ),    new WLabel(   StringList::Nauseated,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Nauseated, QRect( 440, 228,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Nauseated, QRect( 440, 230,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Slowed,    QRect( 330, 250,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 248,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/SLOWED_A.TGA",      0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Slowed,    QRect( 366, 250, 108,  14 ),    new WLabel(   StringList::Slowed,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Slowed,    QRect( 440, 248,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Slowed,    QRect( 440, 250,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Afraid,    QRect( 330, 270,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 268,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/AFRAID_A.TGA",      0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Afraid,    QRect( 366, 270, 108,  14 ),    new WLabel(   StringList::Afraid,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Afraid,    QRect( 440, 268,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Afraid,    QRect( 440, 270,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Poisoned,  QRect( 330, 290,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setPoisoned(int)) },
        { NO_ID,                                            QRect( 346, 288,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/POISONED_A.TGA",    0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Poisoned,  QRect( 366, 290,  60,  14 ),    new WLabel(   StringList::Poisoned,         Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Poisoned,  QRect( 440, 288,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Poisoned,  QRect( 440, 290,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Silenced,  QRect( 330, 310,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 308,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/SILENCED_A.TGA",    0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Silenced,  QRect( 366, 310, 108,  14 ),    new WLabel(   StringList::Silenced,         Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Silenced,  QRect( 440, 308,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Silenced,  QRect( 440, 310,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Hexed,     QRect( 330, 330,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 328,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/HEXED_A.TGA",       0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Hexed,     QRect( 366, 330, 108,  14 ),    new WLabel(   StringList::Hexed,            Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Hexed,     QRect( 440, 328,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Hexed,     QRect( 440, 330,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Enthralled,QRect( 330, 350,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 348,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/INFATUATED_A.TGA",  0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Enthralled,QRect( 366, 350, 108,  14 ),    new WLabel(   StringList::Enthralled,       Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Enthralled,QRect( 440, 348,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Enthralled,QRect( 440, 350,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Insane,    QRect( 490, 170,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 168,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/INSANE_A.TGA",      0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Insane,    QRect( 526, 170, 108,  14 ),    new WLabel(   StringList::Insane,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Insane,    QRect( 600, 168,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Insane,    QRect( 600, 170,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Blind,     QRect( 490, 190,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 188,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/BLIND_A.TGA",       0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Blind,     QRect( 526, 190, 108,  14 ),    new WLabel(   StringList::Blinded,          Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Blind,     QRect( 600, 188,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Blind,     QRect( 600, 190,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Turncoat,  QRect( 490, 210,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 208,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/TURNCOAT_A.TGA",    0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Turncoat,  QRect( 526, 210, 108,  14 ),    new WLabel(   StringList::Turncoat,         Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Turncoat,  QRect( 600, 208,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Turncoat,  QRect( 600, 210,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_POSSESSED,                                QRect( 490, 230,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 228,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/POSSESSED_A.TGA",   0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_POSSESSED,                               QRect( 526, 230, 108,  14 ),    new WLabel(   StringList::Possessed,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_POSSESSED,                                QRect( 600, 228,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_POSSESSED,                                QRect( 600, 230,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Webbed,    QRect( 490, 250,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 248,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/WEBBED_A.TGA",      0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Webbed,    QRect( 526, 250, 108,  14 ),    new WLabel(   StringList::Webbed,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Webbed,    QRect( 600, 248,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Webbed,    QRect( 600, 250,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Asleep,    QRect( 490, 270,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 268,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/ASLEEP_A.TGA",      0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Asleep,    QRect( 526, 270, 108,  14 ),    new WLabel(   StringList::Asleep,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Asleep,    QRect( 600, 268,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Asleep,    QRect( 600, 270,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Paralyzed, QRect( 490, 290,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 288,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/PARALYZED_A.TGA",   0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Paralyzed, QRect( 526, 290, 108,  14 ),    new WLabel(   StringList::Paralyzed,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Paralyzed, QRect( 600, 288,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Paralyzed, QRect( 600, 290,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Unconscious,QRect( 490, 310,  16,  14 ),   new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 308,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/UNCONSCIOUS_A.TGA", 0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Unconscious,QRect( 526, 310, 108,  14 ),   new WLabel(   StringList::Unconscious,      Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START + character::condition::Unconscious,QRect( 600, 308,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START + character::condition::Unconscious,QRect( 600, 310,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START + character::condition::Dead,       QRect( 490, 330,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 328,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/DEAD_A.TGA",        0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Dead,      QRect( 526, 330, 108,  14 ),    new WLabel(   StringList::Dead,             Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },

        // TODO: Some kind of warning about messing around with Missing
        { CB_COND_START + character::condition::Missing,    QRect( 490, 350,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 348,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/MISSING_A.TGA",     0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Missing,   QRect( 526, 350, 108,  14 ),    new WLabel(   StringList::Missing,          Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },

        { LBL_POISON_STR,                                   QRect( 330, 370, 108,  14 ),    new WLabel(   StringList::PoisonStrength + StringList::APPEND_COLON,   Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { POISON_STRENGTH,                                  QRect( 438, 370,  84,  14 ),    new WSpinBox(  0, 0, 255,                                                     this ),  -1,  SLOT(poisonStrChanged(int)) },

        { NO_ID,                                            QRect( 330, 388,  -1,  -1 ),    new WImage(    healthBox,                                                     this ),  -1,  NULL },

        { NO_ID,                                            QRect( 344, 396,  -1,  -1 ),    new WImage(    "AUTOMAP/MAP_MONSTERFRIENDLY_A.TGA",     0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },

        { NO_ID,                                            QRect( 364, 397,  -1,  -1 ),    new WImage(    rowImg,                                                        this ),  -1,  NULL },
        { NO_ID,                                            QRect( 364, 411,  -1,  -1 ),    new WImage(    rowImg,                                                        this ),  -1,  NULL },
        { NO_ID,                                            QRect( 364, 425,  -1,  -1 ),    new WImage(    rowImg,                                                        this ),  -1,  NULL },

        { LBL_HP,                                           QRect( 364, 397, 108,  14 ),    new WLabel(   StringList::SAHitPoints,      Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { LBL_STAMINA,                                      QRect( 364, 411, 108,  14 ),    new WLabel(   StringList::SAStamina,        Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { LBL_SP,                                           QRect( 364, 425, 108,  14 ),    new WLabel(   StringList::SpellPoints,      Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },

        { CLC_HP,                                           QRect( 472, 397,  82,  14 ),    new WStatBar(  true,                        Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_STAMINA,                                      QRect( 472, 411,  82,  14 ),    new WStatBar(  true,                        Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SP,                                           QRect( 472, 425,  82,  14 ),    new WStatBar(  true,                        Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },

        { VAL_HP,                                           QRect( 560, 397,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(hpChanged(int))      },
        { VAL_STAMINA,                                      QRect( 560, 411,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(staminaChanged(int)) },
        { VAL_SP,                                           QRect( 560, 425,  76,  14 ),    new WSpinBox(  0, 0, 50,                                                      this ),  -1,  SLOT(spChanged(int))      },

        { NO_ID,              QRect( 310,   0,  -1,  -1 ),    new WImage(    portalBox,                                                     this ),  -1,  NULL },

        { CB_PORTAL_ENABLED,  QRect( 320,  15, 200,  14 ),    new WCheckBox( "Portal Enabled",                                              this ),  -1,  SLOT(portalEnable(int)) },

        { NO_ID,              QRect( 344,  90,  -1,  -1 ),    new WImage(    rowImg3,                                                       this ),  -1,  NULL },
        { NO_ID,              QRect( 344, 104,  -1,  -1 ),    new WImage(    rowImg3,                                                       this ),  -1,  NULL },
        { NO_ID,              QRect( 344, 118,  -1,  -1 ),    new WImage(    rowImg3,                                                       this ),  -1,  NULL },
        { NO_ID,              QRect( 344, 132,  -1,  -1 ),    new WImage(    rowImg3,                                                       this ),  -1,  NULL },

        { NO_ID,              QRect( 323,  88,  -1,  -1 ),    new WImage(    "SPELLS/BITMAPS/A-GLOW-BLUE.TGA", 0, QColor(Qt::black), 0.5,   this ),  -1,  NULL },

        { LBL_POS_X,          QRect( 344,  90, 186,  14 ),    new WLabel(    ::getBaseStringTable()->getString( StringList::Position ) + " X",  Qt::AlignCenter,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_POS_X,          QRect( 520,  90,  64,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(lineeditChanged(const QString &)) },
        { BTN_POS_X,          QRect( 588,  89,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/PARTYMOVEMENT_BUTTONS.STI",    0, true, 0.70,  this ),  -1,  SLOT(setPosition(bool)) },
        { LBL_POS_Y,          QRect( 344, 104, 186,  14 ),    new WLabel(    ::getBaseStringTable()->getString( StringList::Position ) + " Y",  Qt::AlignCenter,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_POS_Y,          QRect( 520, 104,  64,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(lineeditChanged(const QString &)) },
        { BTN_POS_Y,          QRect( 588, 103,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/PARTYMOVEMENT_BUTTONS.STI",    0, true, 0.70,  this ),  -1,  SLOT(setPosition(bool)) },
        { LBL_POS_Z,          QRect( 344, 118, 186,  14 ),    new WLabel(    ::getBaseStringTable()->getString( StringList::Position ) + " Z",  Qt::AlignCenter,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_POS_Z,          QRect( 520, 118,  64,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(lineeditChanged(const QString &)) },
        { BTN_POS_Z,          QRect( 588, 117,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/PARTYMOVEMENT_BUTTONS.STI",    0, true, 0.70,  this ),  -1,  SLOT(setPosition(bool)) },
        { LBL_HEADING,        QRect( 344, 132, 186,  14 ),    new WLabel(    StringList::Heading,         Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { VAL_HEADING,        QRect( 520, 132,  64,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(lineeditChanged(const QString &)) },
        { BTN_HEADING,        QRect( 588, 131,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/PARTYMOVEMENT_BUTTONS.STI",    0, true, 0.70,  this ),  -1,  SLOT(setPosition(bool)) },

        { BTN_NAVIGATOR,      QRect( 610,  90,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/ICONS_STANDARD.STI",          35, false, 1.0,  this ),  -1,  SLOT(openNavigator(bool)) },

        { LBL_MAP,            QRect( 320,  54,  50,  14 ),    new WLabel(    ::getBaseStringTable()->getString( StringList::Map + StringList::APPEND_COLON ),  Qt::AlignRight, 10, QFont::Thin, this ),  -1,  NULL },
        { DDL_MAP,            QRect( 386,  47,  -1,  -1 ),    new WDDL(      "Lucida Calligraphy",        Qt::AlignLeft,  9, QFont::Thin,   this ),  -1,  SLOT(ddlChanged(int)) },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    if (WLabel *lbl = qobject_cast<WLabel *>( m_widgets[ LBL_COND_POSSESSED ]))
    {
        lbl->setEnabled( false );
    }
    if (WLabel *lbl = qobject_cast<WLabel *>( m_widgets[ LBL_DUR_POSSESSED ]))
    {
        lbl->setEnabled( false );
    }
    if (WCheckBox *cb = qobject_cast<WCheckBox *>( m_widgets[ CB_COND_POSSESSED ]))
    {
        cb->setCheckState( Qt::Unchecked );
        cb->setEnabled( false );
    }

    // Deactivate the XP buttons until the value is changed
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ BTN_XP_LAST ]))
    {
        q->setDisabled( true );
    }
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ BTN_XP_NOW ]))
    {
        q->setDisabled( true );
    }
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ BTN_XP_NEXT ]))
    {
        q->setDisabled( true );
    }

    // Go through the complete list of levels and try to open them. If a level exists, even if
    // not previously visited it is available as a location for a portal.
    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_MAP ] ))
    {
        const int *levels = ::getLevels();
        for (int k=0; levels[k] != -1; k++)
        {
            QListWidgetItem *map = new QListWidgetItem( getLevelName(levels[k]) );
            map->setData( Qt::UserRole, levels[k] );

            ddl->addItem( map );
        }
    }
    resetScreen( m_char, NULL );
}

ScreenLevels::~ScreenLevels()
{
    // Child widgets are automatically destroyed
}

void ScreenLevels::setPoisoned(int state)
{
    if (WLabel *q = qobject_cast<WLabel *>( m_widgets[ LBL_POISON_STR ]))
    {
        q->setEnabled( state == Qt::Checked );
    }
    if (WSpinBox *q = qobject_cast<WSpinBox *>( m_widgets[ POISON_STRENGTH ]))
    {
        q->setEnabled( state == Qt::Checked );
    }
    if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ LBL_DUR_START + character::condition::Poisoned ]))
    {
        q->setEnabled( state == Qt::Checked );
    }
}

void ScreenLevels::mouseOverLabel(bool on)
{
    int id = -1;

    if (WLabel *q = qobject_cast<WLabel *>( sender() ))
    {
        id = m_widgets.key( q );

        if ((id >= LBL_PROFS_START) && (id < LBL_PROFS_END))
        {
            if (! m_inspectMode)
                return;
        }
    }
    if (WImage *q = qobject_cast<WImage *>( sender() ))
    {
        id = m_widgets.key( q ) - FKB_DUR_START + LBL_DUR_START;
    }

    if (WLabel *q = qobject_cast<WLabel *>( m_widgets[ id ] ))
    {
        if (q->isEnabled())
        {
            if (on)
                q->setStyleSheet("QLabel {color: #169e16}"); // Green
            else
                q->setStyleSheet("QLabel {color: #e0e0c3}"); // White (ish)
        }
    }
}

void ScreenLevels::fkButton(bool down)
{
    if (down)
    {
        int cond = -1;

        if (WLabel *q = qobject_cast<WLabel *>( sender() ))
        {
            cond = m_widgets.key( q ) - LBL_DUR_START;
        }
        else if (WImage *q = qobject_cast<WImage *>( sender() ))
        {
            cond = m_widgets.key( q ) - FKB_DUR_START;
        }

        if (cond != -1)
        {
            character::condition condition = static_cast<character::condition>( cond );

            QString conditionStr;

            if (WLabel *lbl = qobject_cast<WLabel *>( m_widgets[ cond + LBL_COND_START ] ))
            {
                conditionStr = lbl->text();
            }

            if (WLabel *q = qobject_cast<WLabel *>( m_widgets[ cond + LBL_DUR_START ] ))
            {
                if (q->isEnabled())
                {
                    int duration = m_char->getCondition( condition );

                    DialogDuration d( conditionStr, duration, this );

                    if (d.exec() == QDialog::Accepted)
                    {
                        duration = d.getDuration();

                        m_char->setCondition( condition, duration );

                        if (duration == 9999)
                        {
                            q->setFont( "Arial", 14, QFont::Thin );
                            q->setText("\u221e"); // Infinity symbol
                        }
                        else
                        {
                            q->setFont( "Wizardry", 10, QFont::Thin );
                            q->setNum( duration );
                        }
                    }
                }
            }
        }
    }
}

void ScreenLevels::portalEnable(int state)
{
    int ids[] = { LBL_POS_X, VAL_POS_X,
                  LBL_POS_Y, VAL_POS_Y,
                  LBL_POS_Z, VAL_POS_Z,
                  LBL_HEADING, VAL_HEADING,
                  LBL_MAP, DDL_MAP, BTN_NAVIGATOR};

    for (unsigned int k=0; k < sizeof(ids) / sizeof(int); k++)
    {
        if (QWidget *q = qobject_cast<QWidget *>(m_widgets[ ids[k] ]))
        {
            q->setEnabled( state == Qt::Checked );
        }
    }

    // disable these until the value in the textbox is changed
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ BTN_POS_X ]))
    {
        q->setDisabled( true );
    }
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ BTN_POS_Y ]))
    {
        q->setDisabled( true );
    }
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ BTN_POS_Z ]))
    {
        q->setDisabled( true );
    }
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ BTN_HEADING ]))
    {
        q->setDisabled( true );
    }

    if ( state == Qt::Unchecked )
    {
        resetPortal();
    }

    // Force the checkbox change of state to actually be saved
    int    mapId;
    float  xyzhead[4];
    bool   on;

    m_char->getPortalPosition( &on, &mapId, &xyzhead[0], &xyzhead[1], &xyzhead[2], &xyzhead[3] );
    m_char->setPortalPosition( (state == Qt::Checked), mapId, xyzhead[0], xyzhead[1], xyzhead[2], xyzhead[3] );
}

void ScreenLevels::setCb(int state)
{
    if (WCheckBox *cb = qobject_cast<WCheckBox *>( sender() ))
    {
        int id = m_widgets.key( cb ) - CB_COND_START;

        character::condition condition = static_cast<character::condition>( id );

        m_char->setConditionActive( condition, state == Qt::Checked );

        if ((condition == character::condition::Dead) ||
            (condition == character::condition::Missing))
        {
            // These 2 don't have duration controls, so all or nothing
            m_char->setCondition( condition, ((state == Qt::Checked) ? 9999 : 0) );

            if (condition == character::condition::Dead)
            {
                // If a character is dead their HP _must_ be reset to 0 or they can't
                // be revived in game.
                if (WSpinBox *q = qobject_cast<WSpinBox *>(m_widgets[ VAL_HP ]))
                {
                    int hp = ((state == Qt::Checked) ? 0 : 1);

                    q->setValue( hp );
                    q->setEnabled( ((state == Qt::Checked) ? false : true) );
                }
            }
        }
        else
        {
            if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ id + LBL_DUR_START ]))
            {
                q->setStyleSheet("");
                q->setEnabled( state == Qt::Checked );
            }
        }
    }
}

QPixmap ScreenLevels::makeRowPixmap()
{
    // We want a combination of 2 Wizardry images here so we get the benefit
    // of the skills screen which better suits the Screen flow, but has the
    // spinner widgets for actually editting things.

    QPixmap rowImg,     spnImg;

    // images used in dialog
    rowImg = SLFFile::getPixmapFromSlf( "REVIEW/REVIEWSKILLSPAGE.STI", 3 );
    spnImg = SLFFile::getPixmapFromSlf( "CHAR GENERATION/CG_SKILLS.STI", 2 );

    QPixmap customImage( rowImg.size() );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, rowImg,  0,   0, rowImg.width(), rowImg.height() );
    p.drawPixmap(  rowImg.width() - spnImg.width() + 106,   0, spnImg, 106,   0, spnImg.width() - 106, spnImg.height() );
    p.drawPixmap(  rowImg.width() - spnImg.width() + 102,   0, spnImg, 106,   0, 50, spnImg.height() );
    p.end();

    return customImage;
}

QPixmap ScreenLevels::makeRowPixmap2()
{
    QPixmap rowImg;

    // images used in dialog
    rowImg = SLFFile::getPixmapFromSlf( "REVIEW/REVIEWSKILLSPAGE.STI", 3 );

    QPixmap customImage( rowImg.size() );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, rowImg,  0,   0, rowImg.width(), rowImg.height() );
    p.drawPixmap(  75,   0, rowImg,  5,   0, rowImg.width() - 75, rowImg.height() );
    p.drawPixmap(  81,   0, rowImg,  5,   0, rowImg.width() - 81, rowImg.height() );
    p.drawPixmap( 186,   0, rowImg,  170,   0, rowImg.width() - 186, rowImg.height() );
    p.end();

    return customImage;
}

QPixmap ScreenLevels::makeRowPixmap3()
{
    QPixmap rowImg;

    // images used in dialog
    rowImg = SLFFile::getPixmapFromSlf( "REVIEW/REVIEWSKILLSPAGE.STI", 3 );

    QPixmap customImage( 260, rowImg.height() );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, rowImg,  0,   0,  80, rowImg.height() );
    p.drawPixmap(  75,   0, rowImg,  5,   0, 120, rowImg.height() );
    p.drawPixmap( 186,   0, rowImg,  184,   0, rowImg.width() - 186, rowImg.height() );
    p.end();

    return customImage;
}

QPixmap ScreenLevels::makeHealthBoxPixmap()
{
    QPixmap baseBox = SLFFile::getPixmapFromSlf( "REVIEW/REVIEWSKILLSPAGE.STI", 2 );
    QSize   custSz( 320, 61 );

    QPixmap customImage( custSz );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, baseBox,  0,   0, 320,  50 );
    p.drawPixmap(   0,  46, baseBox,  0, 270, 320,  15 );
    p.end();

    return customImage;
}

QPixmap ScreenLevels::makePortalBoxPixmap()
{
    QPixmap bg      = SLFFile::getPixmapFromSlf( "DIALOGS/DIALOGBACKGROUND.STI", 0 );
    QPixmap baseBox = SLFFile::getPixmapFromSlf( "REVIEW/REVIEWSKILLSPAGE.STI", 2 );
    QPixmap bgDdl   = SLFFile::getPixmapFromSlf( "CHAR GENERATION/CG_PROFESSION.STI", 0 );

    QSize   custSz( 330, 168 );

    QPixmap customImage( custSz );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, bg, 0, 0, bg.width(), bg.height() );
    p.drawPixmap( 200,   0, bg, 0, 0, bg.width(), bg.height() );
    p.drawPixmap(   0,  65, baseBox, 318, 179, baseBox.width() - 318, baseBox.height() - 129 );
    p.drawPixmap(   0, 133, baseBox, 318, 150, baseBox.width() - 318,                     50 );
    p.drawPixmap( 288,  93, baseBox, 300, 222, baseBox.width() - 318, baseBox.height() - 131 );
    p.drawPixmap( 288,  83, baseBox, 300,   2,                    30,                     40 );
    p.drawPixmap(  71,  39, bgDdl,    15,   4, 200,  46 );
    p.end();

    return customImage;
}

QPixmap ScreenLevels::makeProfsBoxPixmap()
{
    QPixmap baseBox = SLFFile::getPixmapFromSlf( "REVIEW/REVIEWSKILLSPAGE.STI", 2 );
    QSize   custSz( 320, 285 );

    QPixmap customImage( custSz );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, baseBox,  0,   0, 320, 225 );
    p.drawPixmap(   0, 100, baseBox,  0,  44, 320,  50 );
    p.drawPixmap(   0, 140, baseBox,  0,  42, 320, 145 );
    p.drawPixmap(   0, 250, baseBox,  0, 250, 320,  35 );
    p.drawPixmap(   0,   0, baseBox,  0,   0,   8, 245 );
    p.end();

    return customImage;
}

QPixmap ScreenLevels::makeFakeButton()
{
    QPixmap base = SLFFile::getPixmapFromSlf( "NPC INTERACTION/NPC_BOTTOMPANEL.STI", 20 );

    QPixmap button( QSize( base.width() / 2, base.height() ) );

    QPainter p;

    p.begin( &button );
    p.drawPixmap(   0,   0, base,  0,   0, base.width() / 2, base.height() );
    p.end();

    return button;
}

void ScreenLevels::profDetail(bool checked)
{
    QPixmap pix;

    if (checked)
    {
        m_inspectMode = true;
        pix = ::getCursor(Qt::WhatsThisCursor);
    }
    else
    {
        m_inspectMode = false;
        pix = ::getCursor(Qt::ArrowCursor);
    }

    if (! pix.isNull())
    {
        this->setCursor( QCursor( pix, 0, 0 ) );
    }
    else
    {
        this->unsetCursor();
    }
}

void ScreenLevels::info(bool checked)
{
    if (m_inspectMode && checked)
    {
        if (WLabel *w = qobject_cast<WLabel *>(sender()))
        {
            QString  html;

            int prof_idx = m_widgets.key( w ) - LBL_PROFS;

            html  = QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getBaseStringTable()->getString( StringList::Description + StringList::APPEND_COLON ))
                            .arg(::getBaseStringTable()->getString( StringList::DESCProfessions + prof_idx ));

            QList<character::skill> sk = character::getProfessionalSkills( static_cast<character::profession>(m_metaProf.value(prof_idx)) );

            html += QString( "<p><font color=\"#916448\">%1</font> " ).arg(::getBaseStringTable()->getString( StringList::ProfessionalSkills + StringList::APPEND_COLON ));
            for (int k=0; k < sk.size(); k++)
            {
                if (k == 0)
                {
                    html += QString( "<font color=\"#02a7da\">%1</font>" ).arg(::getBaseStringTable()->getString( StringList::LISTSkills + sk.at(k) ));
                }
                else
                {
                    html += ", " + ::getBaseStringTable()->getString( StringList::LISTSkills + sk.at(k) );
                }
            }

            sk = character::getTrainableSkills( static_cast<character::profession>(m_metaProf.value(prof_idx)) );

            html += QString( "<p><font color=\"#916448\">%1</font> " ).arg(::getBaseStringTable()->getString( StringList::OtherSkills + StringList::APPEND_COLON ));
            for (int k=0; k < sk.size(); k++)
            {
                if (k != 0)
                {
                    html += ", ";
                }
                html += ::getBaseStringTable()->getString( StringList::LISTSkills + sk.at(k) );
            }

            new DialogInfo( StringList::LISTProfessions + prof_idx, html, this );
        }
    }
}

void ScreenLevels::spinnerChanged(int value)
{
    if (WSpinBox *q = qobject_cast<WSpinBox *>(this->sender()))
    {
        int   prof = m_widgets.key( q ) - VAL_PROFS_START;

        m_char->setProfessionLevel( static_cast<character::profession>(m_metaProf.value(prof)), value );

        if (WStatBar *clc = qobject_cast<WStatBar *>( m_widgets[ CLC_PROFS + prof ]))
        {
            clc->setValue( value, m_char->getProfessionLevel( static_cast<character::profession>(m_metaProf.value(prof)) ), 50 );
        }

        emit changedLevel();

        if (!m_initialPopulate)
        {
            // Changing the level affects just about everything, including the
            // HP, Stamina and Spell Point maximums shown by this screen

            m_char->recomputeEverything();

            resetHPStaminaSP();
        }
    }
}

void ScreenLevels::hpChanged(int value)
{
    if (!m_initialPopulate)
    {
        m_char->setHp( character::atIdx::Current, value );
    }

    if (WStatBar *clc = qobject_cast<WStatBar *>( m_widgets[ CLC_HP ]))
    {
        clc->setValue( value, value, m_char->getHp(character::atIdx::Base) );
    }
}

void ScreenLevels::staminaChanged(int value)
{
    if (!m_initialPopulate)
    {
        m_char->setStamina( character::atIdx::Current, value );
    }

    if (WStatBar *clc = qobject_cast<WStatBar *>( m_widgets[ CLC_STAMINA ]))
    {
        clc->setValue( value, value, m_char->getStamina(character::atIdx::Base) );
    }
}

void ScreenLevels::spChanged(int value)
{
    if (!m_initialPopulate)
    {
        m_char->setMp( character::realm::REALM_SIZE, character::atIdx::Current, value );
    }

    if (WStatBar *clc = qobject_cast<WStatBar *>( m_widgets[ CLC_SP ]))
    {
        clc->setValue( value, value, m_char->getMp(character::realm::REALM_SIZE, character::atIdx::Base) );
    }
}

void ScreenLevels::poisonStrChanged(int value)
{
    m_char->setPoisonStrength( value );
}

void ScreenLevels::resetScreen( void *char_tag, void *party_tag )
{
    (void)party_tag;

    m_char = (character *)char_tag;

    resetXP();
    resetHPStaminaSP();
    resetConditions();
    resetLevels();
    resetPortal();

    if (WCheckBox *q = qobject_cast<WCheckBox *>(m_widgets[ CB_PORTAL_ENABLED ]))
    {
        if (q->checkState() == Qt::Unchecked )
        {
            portalEnable( Qt::Unchecked );
        }
    }
}

void ScreenLevels::resetXP()
{
    struct { int id; quint32 num; } vals[] =
    {
        { VAL_XP_LAST,         m_char->getXpLastLevelNeeded() },
        { VAL_XP_NOW,          m_char->getXp()                },
        { VAL_XP_NEXT,         m_char->getXpNeeded()          },

        { -1, 0 }
    };

    for (int k=0; vals[k].id != -1; k++)
    {
        if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ vals[k].id ]))
        {
            q->setText( QLocale(QLocale::English).toString( vals[k].num ) );
        }
        else if (WLineEdit *q = qobject_cast<WLineEdit *>(m_widgets[ vals[k].id ]))
        {
            q->setText( QLocale(QLocale::English).toString( vals[k].num ) );
        }
    }
}

void ScreenLevels::ddlChanged(int value)
{
    if (sender() == m_widgets[ DDL_MAP ])
    {
        int    mapId;
        float  x, y, z;
        float  heading;
        bool   on;

        m_char->getPortalPosition( &on, &mapId, &x, &y, &z, &heading );
        mapId = value;
        m_char->setPortalPosition( true, mapId, x, y, z, heading );
    }
}

void ScreenLevels::resetPortal()
{
    int    mapId;
    float  x, y, z;
    float  heading;
    bool   on;

    m_char->getPortalPosition( &on, &mapId, &x, &y, &z, &heading );

    if (WCheckBox *q = qobject_cast<WCheckBox *>(m_widgets[ CB_PORTAL_ENABLED ]))
    {
        q->setCheckState( on ? Qt::Checked : Qt::Unchecked );
    }

    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_MAP ]))
    {
        for (int j=0; j<=ddl->count(); j++)
        {
            QListWidgetItem *row = ddl->item( j );
            if (row->data( Qt::UserRole ).toInt() == mapId)
            {
                ddl->setCurrentRow( j );
                break;
            }
        }
        ddl->updateList();
    }

    struct { int id; float num; } vals[] =
    {
        { VAL_POS_X,           x            },
        { VAL_POS_Y,           y            },
        { VAL_POS_Z,           z            },
        { VAL_HEADING,         heading      },

        { -1, 0 }
    };

    for (int k=0; vals[k].id != -1; k++)
    {
        if (WLineEdit *q = qobject_cast<WLineEdit *>(m_widgets[ vals[k].id ]))
        {
            q->setText( QString::number( vals[k].num ) );
        }
    }
}

void ScreenLevels::openNavigator(bool /* checked */)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        bool loop;

        int    mapId;
        float  x, y, z;
        float  heading;
        bool   on;

        m_char->getPortalPosition( &on, &mapId, &x, &y, &z, &heading );

        // One of the buttons in the Navigator allows for the loading of alternative maps.
        // Swapping to a new map entails throwing away virtually everything and starting again
        // And the lazy way of doing that is to destroy the entire class and start again
        do
        {
            // The constructor creates a new window and runs as a modal window -
            // it doesn't return until the window has been closed
            Window3DNavigator map( mapId, x, y, z, heading, false, NULL );

            loop = false;
            switch (map.exec())
            {
                case Window3DNavigator::ChangeMap:
                    mapId = map.getMapId();
                    x = y = z = heading = NAN;
                    loop = true;
                    break;

                case Window3DNavigator::Rejected:
                    break;

                case Window3DNavigator::Accepted:
                {
                    mapId     = map.getMapId();
                    map.getPosition( &x, &y, &z);
                    heading = map.getHeading();

                    m_char->setPortalPosition( true, mapId, x, y, z, heading );
                    resetPortal();
                    break;
                }
            }
        }
        while (loop);

        q->setChecked(false);
    }
}

void ScreenLevels::resetHPStaminaSP()
{
    m_initialPopulate++;
    for (int k=VAL_HP; k<=VAL_SP; k++)
    {
        int initial = 0;
        int current = 0;
        int range   = 0;

        switch (k)
        {
            case VAL_HP:
                initial = m_char->getHp( character::atIdx::Initial );
                current = m_char->getHp( character::atIdx::Current );
                range   = m_char->getHp( character::atIdx::Base    );
                break;

            case VAL_STAMINA:
                initial = m_char->getStamina( character::atIdx::Initial );
                current = m_char->getStamina( character::atIdx::Current );
                range   = m_char->getStamina( character::atIdx::Base    );
                break;

            case VAL_SP:
                initial = m_char->getMp( character::realm::REALM_SIZE, character::atIdx::Initial );
                current = m_char->getMp( character::realm::REALM_SIZE, character::atIdx::Current );
                range   = m_char->getMp( character::realm::REALM_SIZE, character::atIdx::Base    );
                break;
        }

        if (WSpinBox *q = qobject_cast<WSpinBox *>(m_widgets[ k ]))
        {
            q->setRange( 0, range );
            q->setValueEx( current, initial );

            if (WStatBar *s = qobject_cast<WStatBar *>(m_widgets[ k - VAL_HP + CLC_HP ]))
            {
                s->setValue( current, current, range );
            }
        }
    }
    m_initialPopulate--;
}

void ScreenLevels::resetConditions()
{
    for (int k=0; k< character::condition::CONDITION_SIZE; k++)
    {
        character::condition cond = static_cast<character::condition>(k);

        if (cond == character::condition::Normal)
            continue;

        if (WCheckBox *q = qobject_cast<WCheckBox *>(m_widgets[ CB_COND_START + k ]))
        {
            int state = m_char->getCondition( cond );

            q->setCheckState( (state > 0) ? Qt::Checked : Qt::Unchecked );
            if (cond == character::condition::Poisoned)
            {
                setPoisoned( q->checkState() );
                if (WSpinBox *q = qobject_cast<WSpinBox *>( m_widgets[ POISON_STRENGTH ]))
                {
                    q->setValueEx( m_char->getPoisonStrength(), m_char->getPoisonStrength() );
                }
            }
            else if ((cond == character::condition::Dead) ||
                     (cond == character::condition::Missing))
            {
                // There's no duration box for these 2
                continue;
            }

            if (WLabel *lbl = qobject_cast<WLabel *>(m_widgets[ LBL_DUR_START + k ]))
            {
                lbl->setEnabled( q->checkState() == Qt::Checked );

                if (state == 9999)
                {
                    lbl->setFont( "Arial", 14, QFont::Thin );
                    lbl->setText("\u221e"); // Infinity symbol
                }
                else
                {
                    lbl->setFont( "Wizardry", 10, QFont::Thin );
                    lbl->setNum( state );
                }
            }
        }
    }
}

void ScreenLevels::resetLevels()
{
    m_initialPopulate++;
    for (int k=0; k < m_metaProf.keyCount(); k++)
    {
        character::profession prof = static_cast<character::profession>(m_metaProf.value(k));

        int level   = m_char->getProfessionLevel( prof );
        int initial = m_char->getProfessionInitialLevel( prof );

        if (WSpinBox *q = qobject_cast<WSpinBox *>(m_widgets[ VAL_PROFS + k ]))
        {
            q->setValueEx( level, initial );
        }
        if (level == 0)
        {
            // These ones won't have got the spinnerChanged() signal
            if (WStatBar *clc = qobject_cast<WStatBar *>( m_widgets[ CLC_PROFS + k ]))
            {
                clc->setValue( level, level, 50 );
            }
        }
    }
    m_initialPopulate--;
}

void ScreenLevels::lineeditChanged(const QString &)
{
    struct { int id; int btn_id; } vals[] =
    {
        { VAL_XP_LAST, BTN_XP_LAST },
        { VAL_XP_NOW,  BTN_XP_NOW  },
        { VAL_XP_NEXT, BTN_XP_NEXT },
        { VAL_POS_X,   BTN_POS_X   },
        { VAL_POS_Y,   BTN_POS_Y   },
        { VAL_POS_Z,   BTN_POS_Z   },
        { VAL_HEADING, BTN_HEADING },

        { -1, 0 }
    };

    for (int k=0; vals[k].id != -1; k++)
    {
        if (qobject_cast<WLineEdit *>(this->sender()) == qobject_cast<WLineEdit *>(m_widgets[ vals[k].id ]))
        {
            if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ vals[k].btn_id ]))
            {
                q->setDisabled( false );
            }
            break;
        }
    }
}

void ScreenLevels::setPosition(bool)
{
    struct { int id; int tb_id; int idx; } vals[] =
    {
        { BTN_POS_X,   VAL_POS_X,   0 },
        { BTN_POS_Y,   VAL_POS_Y,   1 },
        { BTN_POS_Z,   VAL_POS_Z,   2 },
        { BTN_HEADING, VAL_HEADING, 3 },

        { -1, 0, 0 }
    };

    for (int k=0; vals[k].id != -1; k++)
    {
        QAbstractButton *q = qobject_cast<QAbstractButton *>(this->sender());

        if (q == qobject_cast<QAbstractButton *>(m_widgets[ vals[k].id ]))
        {
            q->setChecked(false);
            q->setDisabled(true);

            if (WLineEdit *tb = qobject_cast<WLineEdit *>(m_widgets[ vals[k].tb_id ]))
            {
                int    mapId;
                float  xyzhead[4];
                bool   on;

                m_char->getPortalPosition( &on, &mapId, &xyzhead[0], &xyzhead[1], &xyzhead[2], &xyzhead[3] );
                xyzhead[ vals[k].idx ] = tb->text().toFloat();

                m_char->setPortalPosition( true, mapId, xyzhead[0], xyzhead[1], xyzhead[2], xyzhead[3] );
            }
            break;
        }
    }
}

void ScreenLevels::setXpLast(bool)
{
    quint32 v = xpButton( qobject_cast<QPushButton *>(this->sender()), VAL_XP_LAST );

    m_char->setXpLastLevelNeeded( v );
}

void ScreenLevels::setXpNow(bool)
{
    quint32 v = xpButton( qobject_cast<QPushButton *>(this->sender()), VAL_XP_NOW );

    m_char->setXp( v );
}

void ScreenLevels::setXpNext(bool)
{
    quint32 v = xpButton( qobject_cast<QPushButton *>(this->sender()), VAL_XP_NEXT );

    m_char->setXpNeeded( v );
}

quint32 ScreenLevels::xpButton( QPushButton *q, int widget_id )
{
    quint32 value = 0;

    if (q)
    {
        // non-checkable buttons
        q->setChecked(false);

        if (QLineEdit *xp = qobject_cast<QLineEdit *>(m_widgets[ widget_id ]))
        {
            QString txt;
            int     i;

            // Can't just use the locale to turn it back into an int because if it WAS saying 123,456
            // for instance and someone just deleted the 4 to make it 123,56 it now isn't in the
            // correct locale format and it isn't an unmarked up number either, so manually remove the
            // commas and terminate at periods.

            txt = xp->text().replace(",", "");
            i   = txt.indexOf(".");
            if (i != -1)
            {
                txt = txt.left(i);
            }
            value = txt.toULong();

            xp->setText( QLocale(QLocale::English).toString( value ));
        }

        q->setDisabled( true );
    }
    return value;
}
