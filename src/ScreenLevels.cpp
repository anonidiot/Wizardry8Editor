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
#include "WButton.h"
#include "WCheckBox.h"
#include "WImage.h"
#include "WLabel.h"
#include "WLineEdit.h"
#include "WSpinBox.h"
#include "WStatBar.h"

#include "SLFFile.h"

#include "spell.h"
#include "main.h"

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

    SIZE_WIDGET_IDS
} widget_ids;

ScreenLevels::ScreenLevels(character *c, QWidget *parent) :
    Screen(parent),
    m_char(c),
    m_inspectMode(false)
{
    m_metaProf        = QMetaEnum::fromType<character::profession>();

    QPixmap profsBox = makeProfsBoxPixmap();
    QPixmap rowImg   = makeRowPixmap();
    QPixmap rowImg2  = makeRowPixmap2();

    QPixmap fakeButton = makeFakeButton();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect( 310,   0,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWSKILLSPAGE.STI",                  0,              this ),  -1,  NULL },
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
        { VAL_XP_LAST,        QRect( 220, 397,  70,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(xpLastChanged(const QString &)) },
        { BTN_XP_LAST,        QRect( 292, 396,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/PARTYMOVEMENT_BUTTONS.STI",    0, true, 0.70,  this ),  -1,  SLOT(setXpLast(bool)) },
        { NO_ID,              QRect(  34, 411, 186,  14 ),    new WLabel( StringList::ExpEarned,          Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { VAL_XP_NOW,         QRect( 220, 411,  70,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(xpNowChanged(const QString &)) },
        { BTN_XP_NOW,         QRect( 292, 410,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/PARTYMOVEMENT_BUTTONS.STI",    0, true, 0.70,  this ),  -1,  SLOT(setXpNow(bool)) },
        { NO_ID,              QRect(  34, 425, 186,  14 ),    new WLabel( StringList::ExpNext,            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { VAL_XP_NEXT,        QRect( 220, 425,  70,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(xpNextChanged(const QString &)) },
        { BTN_XP_NEXT,        QRect( 292, 424,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/PARTYMOVEMENT_BUTTONS.STI",    0, true, 0.70,  this ),  -1,  SLOT(setXpNext(bool)) },

        // We're using Monster Icons here instead of the Condition Icons, because they're larger and look nicer when scaled up like we're doing;
        // The only Condition that doesn't have a Monster Icon is Drained, so we still need to use the Condition Icon for it - and that means it's
        // missing the circle around the outside that all the others have.
        // TODO: Make a proper icon for it at the right resolution and use that.
// This actually sort of works. The drained icon does blot out some of the circle, but the skull is completely hidden
//        { NO_ID,              QRect( 330, 175,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/DEAD_A.TGA",        0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { CB_COND_START  + character::condition::Drained,   QRect( 330, 177,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 175,  -1,  -1 ),    new WImage(   "ICONS/CONDITIONS/DRAINED.STI",          0,                     this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Drained,   QRect( 366, 177, 108,  14 ),    new WLabel(   StringList::Drained,          Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Drained,   QRect( 440, 175,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Drained,   QRect( 440, 177,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Diseased,  QRect( 330, 197,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 195,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/DISEASED_A.TGA",    0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Diseased,  QRect( 366, 197, 108,  14 ),    new WLabel(   StringList::Diseased,         Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Diseased,  QRect( 440, 195,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Diseased,  QRect( 440, 197,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Irritated, QRect( 330, 217,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 215,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/IRRITATED_A.TGA",   0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Irritated, QRect( 366, 217, 108,  14 ),    new WLabel(   StringList::Irritated,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Irritated, QRect( 440, 215,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Irritated, QRect( 440, 217,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Nauseated, QRect( 330, 237,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 235,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/NAUSEATED_A.TGA",   0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Nauseated, QRect( 366, 237, 108,  14 ),    new WLabel(   StringList::Nauseated,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Nauseated, QRect( 440, 235,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Nauseated, QRect( 440, 237,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Slowed,    QRect( 330, 257,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 255,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/SLOWED_A.TGA",      0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Slowed,    QRect( 366, 257, 108,  14 ),    new WLabel(   StringList::Slowed,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Slowed,    QRect( 440, 255,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Slowed,    QRect( 440, 257,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Afraid,    QRect( 330, 277,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 275,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/AFRAID_A.TGA",      0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Afraid,    QRect( 366, 277, 108,  14 ),    new WLabel(   StringList::Afraid,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Afraid,    QRect( 440, 275,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Afraid,    QRect( 440, 277,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Poisoned,  QRect( 330, 297,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setPoisoned(int)) },
        { NO_ID,                                            QRect( 346, 295,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/POISONED_A.TGA",    0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Poisoned,  QRect( 366, 297,  60,  14 ),    new WLabel(   StringList::Poisoned,         Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Poisoned,  QRect( 440, 295,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Poisoned,  QRect( 440, 297,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Silenced,  QRect( 330, 317,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 315,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/SILENCED_A.TGA",    0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Silenced,  QRect( 366, 317, 108,  14 ),    new WLabel(   StringList::Silenced,         Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Silenced,  QRect( 440, 315,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Silenced,  QRect( 440, 317,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Hexed,     QRect( 330, 337,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 335,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/HEXED_A.TGA",       0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Hexed,     QRect( 366, 337, 108,  14 ),    new WLabel(   StringList::Hexed,            Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Hexed,     QRect( 440, 335,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Hexed,     QRect( 440, 337,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Enthralled,QRect( 330, 357,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 346, 355,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/INFATUATED_A.TGA",  0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Enthralled,QRect( 366, 357, 108,  14 ),    new WLabel(   StringList::Enthralled,       Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Enthralled,QRect( 440, 355,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Enthralled,QRect( 440, 357,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Insane,    QRect( 490, 177,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 175,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/INSANE_A.TGA",      0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Insane,    QRect( 526, 177, 108,  14 ),    new WLabel(   StringList::Insane,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Insane,    QRect( 600, 175,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Insane,    QRect( 600, 177,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Blind,     QRect( 490, 197,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 195,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/BLIND_A.TGA",       0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Blind,     QRect( 526, 197, 108,  14 ),    new WLabel(   StringList::Blinded,          Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Blind,     QRect( 600, 195,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Blind,     QRect( 600, 197,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Turncoat,  QRect( 490, 217,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 215,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/TURNCOAT_A.TGA",    0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Turncoat,  QRect( 526, 217, 108,  14 ),    new WLabel(   StringList::Turncoat,         Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Turncoat,  QRect( 600, 215,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Turncoat,  QRect( 600, 217,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_POSSESSED,                                QRect( 490, 237,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 235,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/POSSESSED_A.TGA",   0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_POSSESSED,                               QRect( 526, 237, 108,  14 ),    new WLabel(   StringList::Possessed,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_POSSESSED,                                QRect( 600, 235,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_POSSESSED,                                QRect( 600, 237,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Webbed,    QRect( 490, 257,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 255,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/WEBBED_A.TGA",      0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Webbed,    QRect( 526, 257, 108,  14 ),    new WLabel(   StringList::Webbed,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Webbed,    QRect( 600, 255,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Webbed,    QRect( 600, 257,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Asleep,    QRect( 490, 277,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 275,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/ASLEEP_A.TGA",      0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Asleep,    QRect( 526, 277, 108,  14 ),    new WLabel(   StringList::Asleep,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Asleep,    QRect( 600, 275,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Asleep,    QRect( 600, 277,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Paralyzed, QRect( 490, 297,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 295,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/PARALYZED_A.TGA",   0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Paralyzed, QRect( 526, 297, 108,  14 ),    new WLabel(   StringList::Paralyzed,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START  + character::condition::Paralyzed, QRect( 600, 295,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START  + character::condition::Paralyzed, QRect( 600, 297,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START  + character::condition::Unconscious,QRect( 490, 317,  16,  14 ),   new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 315,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/UNCONSCIOUS_A.TGA", 0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Unconscious,QRect( 526, 317, 108,  14 ),   new WLabel(   StringList::Unconscious,      Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { FKB_DUR_START + character::condition::Unconscious,QRect( 600, 315,  -1,  -1 ),    new WImage(   fakeButton,                                                     this ),  -1,  SLOT(fkButton(bool)) },
        { LBL_DUR_START + character::condition::Unconscious,QRect( 600, 317,  42,  14 ),    new WLabel(   "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(fkButton(bool)) },

        { CB_COND_START + character::condition::Dead,       QRect( 490, 337,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 335,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/DEAD_A.TGA",        0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Dead,      QRect( 526, 337, 108,  14 ),    new WLabel(   StringList::Dead,             Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },

        { CB_COND_START + character::condition::Missing,    QRect( 490, 357,  16,  14 ),    new WCheckBox( "",                                                            this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                                            QRect( 506, 355,  -1,  -1 ),    new WImage(   "ICONS/MONSTERSPELLS/MISSING_A.TGA",     0, 0, 0, 0, 0, 0.5,    this ),  -1,  NULL },
        { LBL_COND_START + character::condition::Missing,   QRect( 526, 357, 108,  14 ),    new WLabel(   StringList::Missing,          Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },

        { LBL_POISON_STR,                                   QRect( 330, 385, 108,  14 ),    new WLabel(   StringList::PoisonStrength + StringList::APPEND_COLON,   Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { POISON_STRENGTH,                                  QRect( 438, 385,  84,  14 ),    new WSpinBox(  0, 0, 255,                                                     this ),  -1,  SLOT(poisonStrChanged(int)) },

        // TODO: Some kind of warning about messing around with Missing
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

            html  = QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::Description + StringList::APPEND_COLON ))
                            .arg(::getStringTable()->getString( StringList::DESCProfessions + prof_idx ));

            QList<character::skill> sk = character::getProfessionalSkills( static_cast<character::profession>(m_metaProf.value(prof_idx)) );

            html += QString( "<p><font color=\"#916448\">%1</font> " ).arg(::getStringTable()->getString( StringList::ProfessionalSkills + StringList::APPEND_COLON ));
            for (int k=0; k < sk.size(); k++)
            {
                if (k == 0)
                {
                    html += QString( "<font color=\"#02a7da\">%1</font>" ).arg(::getStringTable()->getString( StringList::LISTSkills + sk.at(k) ));
                }
                else
                {
                    html += ", " + ::getStringTable()->getString( StringList::LISTSkills + sk.at(k) );
                }
            }

            sk = character::getTrainableSkills( static_cast<character::profession>(m_metaProf.value(prof_idx)) );

            html += QString( "<p><font color=\"#916448\">%1</font> " ).arg(::getStringTable()->getString( StringList::OtherSkills + StringList::APPEND_COLON ));
            for (int k=0; k < sk.size(); k++)
            {
                if (k != 0)
                {
                    html += ", ";
                }
                html += ::getStringTable()->getString( StringList::LISTSkills + sk.at(k) );
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

    struct { int id; QString str; } vals[] =
    {
        { VAL_XP_LAST,         QLocale(QLocale::English).toString(m_char->getXpLastLevelNeeded()) },
        { VAL_XP_NOW,          QLocale(QLocale::English).toString(m_char->getXp())                },
        { VAL_XP_NEXT,         QLocale(QLocale::English).toString(m_char->getXpNeeded())          },

        { -1, "" }
    };

    for (int k=0; vals[k].id != -1; k++)
    {
        if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ vals[k].id ]))
        {
            q->setText( vals[k].str );
        }
        else if (WLineEdit *q = qobject_cast<WLineEdit *>(m_widgets[ vals[k].id ]))
        {
            q->setText( vals[k].str );
        }
    }

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
}
void ScreenLevels::xpLastChanged(const QString &)
{
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ BTN_XP_LAST ]))
    {
        q->setDisabled( false );
    }
}

void ScreenLevels::xpNowChanged(const QString &)
{
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ BTN_XP_NOW ]))
    {
        q->setDisabled( false );
    }
}

void ScreenLevels::xpNextChanged(const QString &)
{
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ BTN_XP_NEXT ]))
    {
        q->setDisabled( false );
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
