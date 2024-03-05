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
#include "ScreenSkills.h"
#include "DialogInfo.h"

#include "StringList.h"
#include "WButton.h"
#include "WImage.h"
#include "WLabel.h"
#include "WSpinBox.h"
#include "WStatBar.h"

#include "SLFFile.h"

#include "main.h"

#include <QDebug>

typedef enum
{
    NO_ID,

    SKILL_INSPECT,

    LBL_SKILLS_START,
    LBL_SKILLS = LBL_SKILLS_START,
    LBL_SKILLS_END = LBL_SKILLS_START + character::skill::SKILL_SIZE,

    VAL_SKILLS_START,
    VAL_SKILLS = VAL_SKILLS_START,
    VAL_SKILLS_END = VAL_SKILLS_START + character::skill::SKILL_SIZE,

    CLC_SKILLS_START,
    CLC_SKILLS = CLC_SKILLS_START,
    CLC_SKILLS_END = CLC_SKILLS_START + character::skill::SKILL_SIZE,

    SIZE_WIDGET_IDS
} widget_ids;

ScreenSkills::ScreenSkills(party *p, character *c, QWidget *parent) :
    Screen(parent),
    m_char(c),
    m_party(p),
    m_inspectMode(false)
{
    QPixmap rowImg = makeRowPixmap();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0, 450,  -1,  -1 ),    new WImage(    "REVIEW/BOTTOMBUTTONBAR.STI",                   0,              this ),  -1,                             NULL },

        { NO_ID,              QRect( 310,   0,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWSKILLSPAGE.STI",                  1,              this ),  -1,  NULL },
        { NO_ID,              QRect(   0, 164,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWSKILLSPAGE.STI",                  2,              this ),  -1,  NULL },

        { NO_ID,              QRect(  34, 173,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 187,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 201,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 215,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 229,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 243,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 257,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 271,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },

        { NO_ID,              QRect(  34, 298,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 312,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 326,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 340,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 354,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 368,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 382,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 396,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 410,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 424,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },

        { NO_ID,              QRect( 352,  33,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352,  47,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352,  61,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352,  75,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352,  89,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 103,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 117,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },

        { NO_ID,              QRect( 352, 173,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 187,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 201,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 215,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 229,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 243,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 257,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 271,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 285,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 299,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 313,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },

        { NO_ID,              QRect( 352, 368,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 382,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 396,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 410,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },
        { NO_ID,              QRect( 352, 424,  -1,  -1 ),    new WImage(    rowImg,                                                         this ),  -1,  NULL },

        { NO_ID,              QRect(  14, 170,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWSKILLSICONS.STI",                 0,              this ),  -1,  NULL },
        { NO_ID,              QRect(  14, 296,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWSKILLSICONS.STI",                 1,              this ),  -1,  NULL },
        { NO_ID,              QRect( 332,  30,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWSKILLSICONS.STI",                 4,              this ),  -1,  NULL },
        { NO_ID,              QRect( 332, 170,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWSKILLSICONS.STI",                 2,              this ),  -1,  NULL },
        { NO_ID,              QRect( 332, 365,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWSKILLSICONS.STI",                 3,              this ),  -1,  NULL },

        { SKILL_INSPECT,      QRect(   0, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWITEMBUTTONS.STI",                 0, false, 1.0,  this ),  StringList::InspectSkill,     SLOT(skillDetail(bool)) },

        { LBL_SKILLS + character::skill::CloseCombat,   QRect(  34, 173, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::CloseCombat,   Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::RangedCombat,  QRect(  34, 187, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::RangedCombat,  Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::DualWeapons,   QRect(  34, 201, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::DualWeapons,   Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::CriticalStrike,QRect(  34, 215, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::CriticalStrike,Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Artifacts,     QRect(  34, 229, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Artifacts,     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Mythology,     QRect(  34, 243, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Mythology,     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Communication, QRect(  34, 257, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Communication, Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Engineering,   QRect(  34, 271, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Engineering,   Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },

        { LBL_SKILLS + character::skill::Wizardry,      QRect(  34, 298, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Wizardry,      Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Divinity,      QRect(  34, 312, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Divinity,      Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Alchemy,       QRect(  34, 326, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Alchemy,       Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Psionics,      QRect(  34, 340, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Psionics,      Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::FireMagic,     QRect(  34, 354, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::FireMagic,     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::WaterMagic,    QRect(  34, 368, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::WaterMagic,    Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::AirMagic,      QRect(  34, 382, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::AirMagic,      Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::EarthMagic,    QRect(  34, 396, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::EarthMagic,    Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::MentalMagic,   QRect(  34, 410, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::MentalMagic,   Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::DivineMagic,   QRect(  34, 424, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::DivineMagic,   Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },

        { LBL_SKILLS + character::skill::PowerStrike,   QRect( 352,  33, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::PowerStrike,   Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::PowerCast,     QRect( 352,  47, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::PowerCast,     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::IronWill,      QRect( 352,  61, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::IronWill,      Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::IronSkin,      QRect( 352,  75, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::IronSkin,      Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Reflexion,     QRect( 352,  89, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Reflexion,     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::SnakeSpeed,    QRect( 352, 103, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::SnakeSpeed,    Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::EagleEye,      QRect( 352, 117, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::EagleEye,      Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },

        { LBL_SKILLS + character::skill::Sword,         QRect( 352, 173, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Sword,         Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Axe,           QRect( 352, 187, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Axe,           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Polearm,       QRect( 352, 201, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Polearm,       Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Mace_Flail,    QRect( 352, 215, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Mace_Flail,    Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Dagger,        QRect( 352, 229, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Dagger,        Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Staff_Wand,    QRect( 352, 243, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Staff_Wand,    Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Shield,        QRect( 352, 257, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Shield,        Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::ModernWeapon,  QRect( 352, 271, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::ModernWeapon,  Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Bow,           QRect( 352, 285, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Bow,           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Throwing_Sling,QRect( 352, 299, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Throwing_Sling,Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::MartialArts,   QRect( 352, 313, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::MartialArts,   Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },

        { LBL_SKILLS + character::skill::Locks_Traps,   QRect( 352, 368, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Locks_Traps,   Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Stealth,       QRect( 352, 382, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Stealth,       Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Music,         QRect( 352, 396, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Music,         Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Pickpocket,    QRect( 352, 410, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Pickpocket,    Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_SKILLS + character::skill::Scouting,      QRect( 352, 424, 108,  14 ),    new WLabel(    StringList::LISTSkills + character::skill::Scouting,      Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },

        { CLC_SKILLS + character::skill::CloseCombat,   QRect( 142, 173,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::RangedCombat,  QRect( 142, 187,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::DualWeapons,   QRect( 142, 201,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::CriticalStrike,QRect( 142, 215,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Artifacts,     QRect( 142, 229,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Mythology,     QRect( 142, 243,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Communication, QRect( 142, 257,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Engineering,   QRect( 142, 271,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },

        { CLC_SKILLS + character::skill::Wizardry,      QRect( 142, 298,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Divinity,      QRect( 142, 312,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Alchemy,       QRect( 142, 326,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Psionics,      QRect( 142, 340,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::FireMagic,     QRect( 142, 354,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::WaterMagic,    QRect( 142, 368,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::AirMagic,      QRect( 142, 382,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::EarthMagic,    QRect( 142, 396,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::MentalMagic,   QRect( 142, 410,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::DivineMagic,   QRect( 142, 424,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },

        { CLC_SKILLS + character::skill::PowerStrike,   QRect( 460,  33,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::PowerCast,     QRect( 460,  47,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::IronWill,      QRect( 460,  61,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::IronSkin,      QRect( 460,  75,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Reflexion,     QRect( 460,  89,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::SnakeSpeed,    QRect( 460, 103,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::EagleEye,      QRect( 460, 117,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },

        { CLC_SKILLS + character::skill::Sword,         QRect( 460, 173,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Axe,           QRect( 460, 187,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Polearm,       QRect( 460, 201,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Mace_Flail,    QRect( 460, 215,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Dagger,        QRect( 460, 229,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Staff_Wand,    QRect( 460, 243,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Shield,        QRect( 460, 257,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::ModernWeapon,  QRect( 460, 271,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Bow,           QRect( 460, 285,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Throwing_Sling,QRect( 460, 299,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::MartialArts,   QRect( 460, 313,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },

        { CLC_SKILLS + character::skill::Locks_Traps,   QRect( 460, 368,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Stealth,       QRect( 460, 382,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Music,         QRect( 460, 396,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Pickpocket,    QRect( 460, 410,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_SKILLS + character::skill::Scouting,      QRect( 460, 424,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },

        { VAL_SKILLS + character::skill::CloseCombat,   QRect( 230, 173,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::RangedCombat,  QRect( 230, 187,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::DualWeapons,   QRect( 230, 201,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::CriticalStrike,QRect( 230, 215,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Artifacts,     QRect( 230, 229,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Mythology,     QRect( 230, 243,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Communication, QRect( 230, 257,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Engineering,   QRect( 230, 271,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },

        { VAL_SKILLS + character::skill::Wizardry,      QRect( 230, 298,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Divinity,      QRect( 230, 312,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Alchemy,       QRect( 230, 326,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Psionics,      QRect( 230, 340,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::FireMagic,     QRect( 230, 354,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::WaterMagic,    QRect( 230, 368,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::AirMagic,      QRect( 230, 382,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::EarthMagic,    QRect( 230, 396,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::MentalMagic,   QRect( 230, 410,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::DivineMagic,   QRect( 230, 424,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },

        { VAL_SKILLS + character::skill::PowerStrike,   QRect( 548,  33,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::PowerCast,     QRect( 548,  47,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::IronWill,      QRect( 548,  61,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::IronSkin,      QRect( 548,  75,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Reflexion,     QRect( 548,  89,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::SnakeSpeed,    QRect( 548, 103,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::EagleEye,      QRect( 548, 117,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },

        { VAL_SKILLS + character::skill::Sword,         QRect( 548, 173,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Axe,           QRect( 548, 187,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Polearm,       QRect( 548, 201,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Mace_Flail,    QRect( 548, 215,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Dagger,        QRect( 548, 229,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Staff_Wand,    QRect( 548, 243,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Shield,        QRect( 548, 257,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::ModernWeapon,  QRect( 548, 271,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Bow,           QRect( 548, 285,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Throwing_Sling,QRect( 548, 299,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::MartialArts,   QRect( 548, 313,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },

        { VAL_SKILLS + character::skill::Locks_Traps,   QRect( 548, 368,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Stealth,       QRect( 548, 382,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Music,         QRect( 548, 396,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Pickpocket,    QRect( 548, 410,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_SKILLS + character::skill::Scouting,      QRect( 548, 424,  76,  14 ),    new WSpinBox(  0, 0, 125,                                                                                  this ),  -1,  SLOT(spinnerChanged(int)) },

    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    resetScreen( m_char, m_party );
}

ScreenSkills::~ScreenSkills()
{
    // Child widgets are automatically destroyed
}

QPixmap ScreenSkills::makeRowPixmap()
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

void ScreenSkills::skillDetail(bool checked)
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

void ScreenSkills::info(bool checked)
{
    if (m_inspectMode && checked)
    {
        if (WLabel *w = qobject_cast<WLabel *>(sender()))
        {
            QString  html;

            character::attribute  attrib1;
            character::attribute  attrib2;

            character::skill      skill = static_cast<character::skill>(m_widgets.key( w ) - LBL_SKILLS);

            html  = QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getBaseStringTable()->getString( StringList::Description + StringList::APPEND_COLON ))
                            .arg(::getBaseStringTable()->getString( StringList::DESCSkills + skill ));

            character::lkupControllingAttribs( skill, &attrib1, &attrib2 );

            html += QString( "<p><font color=\"#916448\">%1</font> " ).arg(::getBaseStringTable()->getString( StringList::ControllingAttribs + StringList::APPEND_COLON ));
            html += ::getBaseStringTable()->getString( StringList::LISTPrimaryAttributes + attrib1 );
            if (attrib1 != attrib2)
            {
                html += ", " + ::getBaseStringTable()->getString( StringList::LISTPrimaryAttributes + attrib2 );
            }

            if (skill == m_char->getProfessionalSkill())
            {
                QString s = QString("<p><font color=\"#ffff00\">%1</font>").arg( ::getBaseStringTable()->getString( StringList::SkillPrimary ));

                // This Wizardry string has a printf style format string we want to replace
                s.replace("%d%%", "25%");

                html += s;
            }
            if (! m_char->getTrainableSkills().contains( skill ))
            {
                html += QString("<p><font color=\"#ffff00\">%1</font>").arg( ::getBaseStringTable()->getString( StringList::SkillClosed ));
            }
            // See if we're the best in the party
            int  me   = m_char->getSkill( skill, character::atIdx::Base );
            bool best = true;

            for (int k=0; k<m_party->m_chars.size(); k++)
            {
                character *c = m_party->m_chars.at(k);
                if (c && (c != m_char))
                {
                    int  them = c->getSkill( skill, character::atIdx::Base );
                    if (them > me)
                    {
                        best = false;
                        break;
                    }
                }
            }
            if (best)
            {
                html += QString("<p><font color=\"#ffff00\">%1</font>").arg( ::getBaseStringTable()->getString( StringList::BestInParty ));
            }

            new DialogInfo( StringList::LISTSkills + skill, html, this );
        }
    }
}

void ScreenSkills::mouseOverLabel(bool on)
{
    if (m_inspectMode)
    {
        if (WLabel *q = qobject_cast<WLabel *>(this->sender()))
        {
            if (on)
                q->setStyleSheet("QLabel {color: #169e16}"); // Green
            else
                q->setStyleSheet( q->property( "base_style" ).toString() ); // whatever it was before
        }
    }
}

void ScreenSkills::spinnerChanged(int value)
{
    if (WSpinBox *q = qobject_cast<WSpinBox *>(this->sender()))
    {
        int   skill = m_widgets.key( q ) - VAL_SKILLS_START;

        m_char->setSkill( static_cast<character::skill>(skill), value );
        if (WStatBar *clc = qobject_cast<WStatBar *>( m_widgets[ CLC_SKILLS + skill ]))
        {
            clc->setValue( value, m_char->getSkill( static_cast<character::skill>(skill), character::atIdx::Current ), 125 );
        }
    }
}

void ScreenSkills::resetScreen( void *char_tag, void *party_tag )
{
    m_char  = (character *)char_tag;
    m_party = (party *)party_tag;

    character::skill         bonus_skill  = m_char->getProfessionalSkill();
    QList<character::skill>  class_skills = m_char->getTrainableSkills();

    for (int k=0; k < character::skill::SKILL_SIZE; k++)
    {
        character::skill skill = static_cast<character::skill>(k);

        int base    = m_char->getSkill( skill, character::atIdx::Base );
        int initial = m_char->getSkill( skill, character::atIdx::Initial );

        if (WSpinBox *q = qobject_cast<WSpinBox *>(m_widgets[ VAL_SKILLS + k ]))
        {
            q->setValueEx( base, initial );
        }
        if (base == 0)
        {
            // These ones won't have got the spinnerChanged() signal
            if (WStatBar *clc = qobject_cast<WStatBar *>( m_widgets[ CLC_SKILLS + k ]))
            {
                clc->setValue( base, m_char->getSkill( static_cast<character::skill>(k), character::atIdx::Current ), 125 );
            }
        }

        // Have decided against showing a skill in yellow if it is the highest in the party;
        // it doesn't really convey useful information and in fact only obscures the info I
        // do want to convey by adding a fourth colour into the mix. As it stands we
        // highlight the professional skill, the actual skills available to the character
        // currently, and those that aren't. And that's probably enough.

        // But implementing the yellow highlight could be done with a signal hook on all the
        // spinners so it adjusts as the skill is incremented in realtime. And keeping a record
        // of m_party in this class similar to ScreenItems so that the current value could be
        // compared to everyone else in the party.

        QColor skillColor;

        if (skill == bonus_skill)
        {
            skillColor.setRgb( 0x02, 0xa7, 0xda ); // Cyan (or 0x00, 0xb4, 0xed or 0x00, 0xa6, 0xda)
        }
        else if (class_skills.contains( skill ))
        {
            skillColor.setRgb( 0xe0, 0xe0, 0xc3 ); // White (ish)
        }
        else
        {
            skillColor.setRgb( 0x70, 0x70, 0x43 ); // Grey (yellowish grey)
        }

        if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ LBL_SKILLS + k ]))
        {
            q->setStyleSheet( QString("QLabel {color: %1}").arg( skillColor.name() ) );
            // Keep a record of the assigned colour so it can be restored on mouseExit
            q->setProperty( "base_style", q->styleSheet() );
        }
    }
}
