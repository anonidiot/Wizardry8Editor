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

#include <QApplication>
#include <QMouseEvent>

#include "common.h"
#include "ScreenItems.h"
#include "ScreenCommon.h"
#include "DialogAddItem.h"
#include "DialogItemInfo.h"

#include <QLocale>
#include <QPixmap>

#include "WButton.h"
#include "WImage.h"
#include "WItem.h"
#include "WLabel.h"
#include "WLineEdit.h"
#include "WScrollBar.h"

#include "SLFFile.h"
#include "item.h"

#include "main.h"

#include <QDebug>

#define PERSONAL_ITEMS_OFFSET 1000
#define PARTY_ITEMS_OFFSET    2000

typedef enum
{
    NO_ID,

    INFO_CHAR,
    INFO_EQUIP,

    ITEM_START,
    ITEM_INSPECT = ITEM_START,
    ITEM_MERGE,
    ITEM_SPLIT,
    ITEM_USE,      // No practical use in item editor, so re-purposed for ITEM_ADD
    ITEM_DROP,
    ITEM_ADD,
    ITEM_IDENTIFY,
    ITEM_REMOVE_CURSE,
    ITEM_UNCURSE_ALL,
    ITEM_END,

    PARTY_START,
    PARTY_WEAPONS = PARTY_START,
    PARTY_ARMOR,
    PARTY_EQUIP,
    PARTY_NONEQUIP,
    PARTY_USABLE,
    PARTY_END,

    PARTY_UNIDENTIFIED, // not part of buttongroup

    ITEM_SWAP,

    VAL_GOLD,
    BTN_GOLD,

    PARTY_SCROLLBAR,

    VAL_STATUE,

    VAL_STR,
    VAL_INT,
    VAL_PIE,
    VAL_VIT,
    VAL_DEX,
    VAL_SPD,
    VAL_SEN,

    VAL_XP,
    VAL_NEXT_XP_BAR,
    VAL_NEXT_XP,

    VAL_HP1,
    VAL_HP2,
    VAL_STAMINA1,
    VAL_STAMINA2,
    VAL_LOAD1,
    VAL_LOAD2,

    VAL_BASE_AC,
    VAL_AVG_AC,

    VAL_FIRE1,
    VAL_FIRE2,
    VAL_WATER1,
    VAL_WATER2,
    VAL_AIR1,
    VAL_AIR2,
    VAL_EARTH1,
    VAL_EARTH2,
    VAL_MENTAL1,
    VAL_MENTAL2,
    VAL_DIVINE1,
    VAL_DIVINE2,

    ITEMS_START,
    VAL_BACKPACK1 = ITEMS_START,
    VAL_BACKPACK2,
    VAL_BACKPACK3,
    VAL_BACKPACK4,
    VAL_BACKPACK5,
    VAL_BACKPACK6,
    VAL_BACKPACK7,
    VAL_BACKPACK8,
    VAL_HEAD,
    VAL_CLOAK,
    VAL_TORSO,
    VAL_HAND,
    VAL_LEGS,
    VAL_FEET,
    VAL_MISC1,
    VAL_MISC2,
    VAL_WEAPON1A,
    VAL_WEAPON1B,
    VAL_WEAPON2A,
    VAL_WEAPON2B,
    ITEMS_END,

    PARTY_ITEMS_START,
    VAL_PARTY1 = PARTY_ITEMS_START,
    VAL_PARTY2,
    VAL_PARTY3,
    VAL_PARTY4,
    VAL_PARTY5,
    VAL_PARTY6,
    VAL_PARTY7,
    VAL_PARTY8,
    PARTY_ITEMS_END,

    OVL_EQUIPINFO,
    BTN_DEFENSE,
    LBL_OFFENSE,
    LBL_PRIM,
    LBL_SECO,
    LBL_INITIATIVE,
    INITIATIVE,
    LBL_KILLS,
    KILLS,
    LBL_DEATHS,
    DEATHS,
    LBL_TOINIT,
    PRIM_TOINIT,
    SECO_TOINIT,
    LBL_DAMAGE_RANGE,
    PRIM_DAMAGE,
    SECO_DAMAGE,
    LBL_ATTACK_RATING,
    PRIM_ATTACK,
    SECO_ATTACK,
    LBL_NUM_ATTACKS,
    PRIM_ATTACKS,
    SECO_ATTACKS,
    LBL_SWINGS,
    PRIM_SWINGS,
    SECO_SWINGS,
    LBL_TOHIT,
    PRIM_TOHIT,
    SECO_TOHIT,
    LBL_TOPENETRATE,
    PRIM_TOPENETRATE,
    SECO_TOPENETRATE,
    LBL_TODAMAGE,
    PRIM_TODAMAGE,
    SECO_TODAMAGE,
        OVL_ACINFO,
        BTN_OFFENSE,
        LBL_ACMODS,
        LBL_ACMOD_RACE,
        ACMOD_RACE,
        LBL_ACMOD_SPEED,
        ACMOD_SPEED,
        LBL_ACMOD_STEALTH,
        ACMOD_STEALTH,
        LBL_ACMOD_SHIELD,
        ACMOD_SHIELD,
        LBL_ACMOD_MITEMS,
        ACMOD_MAGICITEMS,
        LBL_ACMOD_MSPELLS,
        ACMOD_MAGICSPELLS,
        LBL_ACMOD_PENETR,
        ACMOD_VPENETRATION,
        LBL_ACMOD_ENCUMB,
        ACMOD_ENCUMBRANCE,
        LBL_ACMOD_CONDS,
        ACMOD_CONDITIONS,
        LBL_ACMOD_FATIGUE,
        ACMOD_FATIGUE,
        LBL_ACMOD_DEFENSE,
        ACMOD_DEFENSIVEACT,
        LBL_ACMOD_REFLEX,
        ACMOD_REFLEXTION,
        LBL_ABSORPTION,
        DAMAGE_ABSORPTION,
        OVL_ACINFO_END,
    OVL_EQUIPINFO_END,



    SIZE_WIDGET_IDS
} widget_ids;

ScreenItems::ScreenItems(party *p, character *c, QWidget *parent) :
    Screen(parent),
    m_party(p),
    m_char(c),
    m_itemMode(NO_ID),
    m_partyItems_scrollPos(0),
    m_common_screen(parent)
{
    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0, 450,  -1,  -1 ),    new WImage(    "REVIEW/BOTTOMBUTTONBAR.STI",                   0,              this ),  -1,                             NULL },

        { ITEM_INSPECT,       QRect(   0, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWITEMBUTTONS.STI",                 0, false, 1.0,  this ),  StringList::InspectItem,        SLOT(itemButton(bool)) },
        { ITEM_MERGE,         QRect(  44, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWITEMBUTTONS.STI",                10, false, 1.0,  this ),  StringList::MergeItems,         NULL },
        { ITEM_SPLIT,         QRect(  88, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWITEMBUTTONS.STI",                 5, false, 1.0,  this ),  StringList::SplitItems,         NULL },
        { ITEM_ADD,           QRect( 132, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWITEMBUTTONS.STI",                15, false, 1.0,  this ),  StringList::AddItem,            SLOT(itemButton(bool)) },
        { ITEM_DROP,          QRect( 176, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWITEMBUTTONS.STI",                20, false, 1.0,  this ),  StringList::DropItem,           SLOT(itemButton(bool)) },
        { ITEM_IDENTIFY,      QRect( 220, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWITEMBUTTONS.STI",                30, false, 1.0,  this ),  StringList::Identify,           NULL },
        { ITEM_REMOVE_CURSE,  QRect( 264, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWITEMBUTTONS.STI",                35, false, 1.0,  this ),  StringList::RemoveCurse,        NULL },
        { ITEM_UNCURSE_ALL,   QRect( 308, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWITEMBUTTONS.STI",                25, false, 1.0,  this ),  StringList::UncurseAllItems,    NULL },

        { NO_ID,              QRect( 310,   0,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWITEMPAGE.STI",                    0,              this ),  -1,                             NULL }, // template for XP, STR, INT etc in top right corner
        { NO_ID,              QRect( 316,   7,  -1,  -1 ),    new WImage(    "REVIEW/COMMOD_BUTTON_BAR.STI",                 0,              this ),  -1,                             NULL }, // The 2 little icons that toggle between the head and the sword

        // Primary Attributes - FIXME: all these labels have mouseover's prompting to right click for more info, then it blurb-dialogs an explanation of the attribute
        { NO_ID,              QRect( 450,  57,  76,  12 ),    new WLabel(    StringList::LISTPrimaryAttributes + static_cast<int>(character::attribute::Strength),
                                                                                                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_STR,            QRect( 528,  57,  22,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 450,  71,  76,  12 ),    new WLabel(    StringList::LISTPrimaryAttributes + static_cast<int>(character::attribute::Intelligence),
                                                                                                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_INT,            QRect( 528,  71,  22,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 450,  85,  76,  12 ),    new WLabel(    StringList::LISTPrimaryAttributes + static_cast<int>(character::attribute::Piety),
                                                                                                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_PIE,            QRect( 528,  85,  22,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 450,  99,  76,  12 ),    new WLabel(    StringList::LISTPrimaryAttributes + static_cast<int>(character::attribute::Vitality),
                                                                                                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_VIT,            QRect( 528,  99,  22,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 450, 113,  76,  12 ),    new WLabel(    StringList::LISTPrimaryAttributes + static_cast<int>(character::attribute::Dexterity),
                                                                                                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_DEX,            QRect( 528, 113,  22,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 450, 127,  76,  12 ),    new WLabel(    StringList::LISTPrimaryAttributes + static_cast<int>(character::attribute::Speed),
                                                                                                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_SPD,            QRect( 528, 127,  22,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 450, 141,  76,  12 ),    new WLabel(    StringList::LISTPrimaryAttributes + static_cast<int>(character::attribute::Senses),
                                                                                                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_SEN,            QRect( 528, 141,  22,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },

        { NO_ID,              QRect( 345,  10, 204,  12 ),    new WLabel(    StringList::ExperiencePoints, Qt::AlignCenter, 10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 346,  23, 100,  12 ),    new WLabel(    StringList::Earned,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_XP,             QRect( 448,  23,  70,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 346,  37, 100,  12 ),    new WLabel(    StringList::NextLevel,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_NEXT_XP_BAR,    QRect( 448,  38,  -1,  -1 ),    new WImage(    "REVIEW/LEVELING_BAR.STI",                      0,              this ),  -1,                             NULL },
        { VAL_NEXT_XP,        QRect( 448,  37,  70,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },

        { NO_ID,              QRect( 324,  57,  69,  12 ),    new WLabel(    StringList::HitPoints,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_HP1,            QRect( 394,  57,  27,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_HP2,            QRect( 421,  57,  21,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 324,  71,  69,  12 ),    new WLabel(    StringList::Stamina,          Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_STAMINA1,       QRect( 394,  71,  27,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_STAMINA2,       QRect( 421,  71,  21,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 324,  85,  69,  12 ),    new WLabel(    StringList::Load,             Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LOAD1,          QRect( 394,  85,  27,  12 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_LOAD2,          QRect( 421,  85,  21,  12 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },

        // Spell points by realm --
        // These icons are supposed to grey out if user has no professions supporting magic,
        // but seems an unnecessary complication
        { NO_ID,              QRect( 557,  13,  -1,  -1 ),    new WImage(    "SPELL CASTING/FIRE_REALM.STI",                 0,              this ),  -1,                             NULL },
        { VAL_FIRE1,          QRect( 577,  15,  29,  19 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_FIRE2,          QRect( 606,  15,  29,  19 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 557,  37,  -1,  -1 ),    new WImage(    "SPELL CASTING/WATER_REALM.STI",                8,              this ),  -1,                             NULL },
        { VAL_WATER1,         QRect( 577,  39,  29,  19 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_WATER2,         QRect( 606,  39,  29,  19 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 557,  61,  -1,  -1 ),    new WImage(    "SPELL CASTING/AIR_REALM.STI",                  0,              this ),  -1,                             NULL },
        { VAL_AIR1,           QRect( 577,  63,  29,  19 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_AIR2,           QRect( 606,  63,  29,  19 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 557,  85,  -1,  -1 ),    new WImage(    "SPELL CASTING/EARTH_REALM.STI",                0,              this ),  -1,                             NULL },
        { VAL_EARTH1,         QRect( 577,  87,  29,  19 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_EARTH2,         QRect( 606,  87,  29,  19 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 557, 109,  -1,  -1 ),    new WImage(    "SPELL CASTING/MENTAL_REALM.STI",               0,              this ),  -1,                             NULL },
        { VAL_MENTAL1,        QRect( 577, 111,  29,  19 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_MENTAL2,        QRect( 606, 111,  29,  19 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 557, 133,  -1,  -1 ),    new WImage(    "SPELL CASTING/DIVINE_REALM.STI",              11,              this ),  -1,                             NULL },
        { VAL_DIVINE1,        QRect( 577, 135,  29,  19 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_DIVINE2,        QRect( 606, 135,  29,  19 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },

        { NO_ID,              QRect( 324, 113, 117,  12 ),    new WLabel(    StringList::ArmorClass,       Qt::AlignCenter, 10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 323, 127,  71,  12 ),    new WLabel(    StringList::BaseAC,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_BASE_AC,        QRect( 394, 127,  48,  12 ),    new WLabel(    "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,                             NULL },
        { NO_ID,              QRect( 323, 141,  71,  12 ),    new WLabel(    StringList::AverageAC,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,                             NULL },
        { VAL_AVG_AC,         QRect( 394, 141,  48,  12 ),    new WLabel(    "",                           Qt::AlignCenter, 10, QFont::Thin, this ),  -1,                             NULL },

        // Overlay panel - combat and weapon modifications
        { OVL_EQUIPINFO,      QRect( 310,   0,  -1,  -1 ),    new WImage(    "REVIEW/COMMOD_BACK.STI",                       0,              this ),  -1,                             NULL },
        { BTN_DEFENSE,        QRect( 318, 137,  -1,  -1 ),    new WButton(   "REVIEW/COMMOD_BUTTONS.STI",                   15, false, 1.0,  this ),  StringList::ViewDefenseMods,    SLOT(acInfo(bool)) },
        { LBL_OFFENSE,        QRect( 347,  10, 286,  12 ),    new WLabel(   StringList::CombatWeaponMods, Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_PRIM,           QRect( 470,  34,  80,  12 ),    new WLabel(   StringList::Primary,          Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_SECO,           QRect( 552,  34,  80,  12 ),    new WLabel(   StringList::Secondary,        Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_INITIATIVE,     QRect( 347,  48,  80,  12 ),    new WLabel(   StringList::Initiative,       Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { INITIATIVE,         QRect( 429,  48,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_TOINIT,         QRect( 504,  48,  94,  12 ),    new WLabel(   StringList::ToInitiative,     Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { PRIM_TOINIT,        QRect( 469,  48,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { SECO_TOINIT,        QRect( 600,  48,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_DAMAGE_RANGE,   QRect( 504,  62,  94,  12 ),    new WLabel(   StringList::DamageRange,      Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { PRIM_DAMAGE,        QRect( 469,  62,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { SECO_DAMAGE,        QRect( 600,  62,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_ATTACK_RATING,  QRect( 504,  76,  94,  12 ),    new WLabel(   StringList::AttackRating,     Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { PRIM_ATTACK,        QRect( 469,  76,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { SECO_ATTACK,        QRect( 600,  76,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_NUM_ATTACKS,    QRect( 504,  90,  94,  12 ),    new WLabel(   StringList::NumAttacks,       Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { PRIM_ATTACKS,       QRect( 469,  90,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { SECO_ATTACKS,       QRect( 600,  90,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_SWINGS,         QRect( 504, 104,  94,  12 ),    new WLabel(   StringList::MaxSwings,        Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { PRIM_SWINGS,        QRect( 469, 104,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { SECO_SWINGS,        QRect( 600, 104,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_TOHIT,          QRect( 504, 118,  94,  12 ),    new WLabel(   StringList::ToHit,            Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { PRIM_TOHIT,         QRect( 469, 118,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { SECO_TOHIT,         QRect( 600, 118,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_TOPENETRATE,    QRect( 504, 132,  94,  12 ),    new WLabel(   StringList::ToPenetrate,      Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { PRIM_TOPENETRATE,   QRect( 469, 132,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { SECO_TOPENETRATE,   QRect( 600, 132,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_TODAMAGE,       QRect( 504, 146,  94,  12 ),    new WLabel(   StringList::ToDamage,         Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { PRIM_TODAMAGE,      QRect( 469, 146,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { SECO_TODAMAGE,      QRect( 600, 146,  33,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },

        { OVL_ACINFO,         QRect( 310,   0,  -1,  -1 ),    new WImage(    "REVIEW/COMMOD_BACK.STI",                       2,              this ),  -1,                             NULL },
        { BTN_OFFENSE,        QRect( 318, 137,  -1,  -1 ),    new WButton(   "REVIEW/COMMOD_BUTTONS.STI",                   10, false, 1.0,  this ),  StringList::ViewOffenseMods,    SLOT(equipInfo(bool)) },
        { LBL_ACMODS,         QRect( 347,  10, 285,  12 ),    new WLabel(   StringList::ACModifiers,      Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },

        { LBL_ACMOD_RACE,     QRect( 350,  34, 103,  12 ),    new WLabel(   StringList::ACModRace,        Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { ACMOD_RACE,         QRect( 455,  34,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_ACMOD_SPEED,    QRect( 350,  48, 103,  12 ),    new WLabel(   StringList::ACModSpeed,       Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { ACMOD_SPEED,        QRect( 455,  48,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_ACMOD_STEALTH,  QRect( 350,  62, 103,  12 ),    new WLabel(   StringList::ACModStealth,     Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { ACMOD_STEALTH,      QRect( 455,  62,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_ACMOD_SHIELD,   QRect( 350,  76, 103,  12 ),    new WLabel(   StringList::ACModShield,      Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { ACMOD_SHIELD,       QRect( 455,  76,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_ACMOD_MITEMS,   QRect( 350,  90, 103,  12 ),    new WLabel(   StringList::ACModMagicItems,  Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { ACMOD_MAGICITEMS,   QRect( 455,  90,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_ACMOD_MSPELLS,  QRect( 350, 104, 103,  12 ),    new WLabel(   StringList::ACModMagicSpells, Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { ACMOD_MAGICSPELLS,  QRect( 455, 104,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_ACMOD_PENETR,   QRect( 495,  34, 103,  12 ),    new WLabel(   StringList::ACModVPenetration,Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { ACMOD_VPENETRATION, QRect( 600,  34,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_ACMOD_ENCUMB,   QRect( 495,  48, 103,  12 ),    new WLabel(   StringList::ACModEncumbrance, Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { ACMOD_ENCUMBRANCE,  QRect( 600,  48,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_ACMOD_CONDS,    QRect( 495,  62, 103,  12 ),    new WLabel(   StringList::ACModConditions,  Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { ACMOD_CONDITIONS,   QRect( 600,  62,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_ACMOD_FATIGUE,  QRect( 495,  76, 103,  12 ),    new WLabel(   StringList::ACModFatigue,     Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { ACMOD_FATIGUE,      QRect( 600,  76,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_ACMOD_DEFENSE,  QRect( 495,  90, 103,  12 ),    new WLabel(   StringList::ACModDefensiveAct,Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { ACMOD_DEFENSIVEACT, QRect( 600,  90,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_ACMOD_REFLEX,   QRect( 495, 104, 103,  12 ),    new WLabel(   StringList::ACModReflextion,  Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { ACMOD_REFLEXTION,   QRect( 600, 104,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },

        { LBL_ABSORPTION,     QRect( 474, 132, 124,  12 ),    new WLabel(   StringList::DamageAbsorption, Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { DAMAGE_ABSORPTION,  QRect( 600, 132,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_KILLS,          QRect( 347, 132,  80,  12 ),    new WLabel(   StringList::Kills,            Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { KILLS,              QRect( 429, 132,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { LBL_DEATHS,         QRect( 347, 146,  80,  12 ),    new WLabel(   StringList::Deaths,           Qt::AlignLeft,    10, QFont::Thin, this ),  -1,                             NULL },
        { DEATHS,             QRect( 429, 146,  32,  12 ),    new WLabel(   "",                           Qt::AlignCenter,  10, QFont::Thin, this ),  -1,                             NULL },
        { INFO_CHAR,          QRect( 318,   9,  -1,  -1 ),    new WButton(   "REVIEW/COMMOD_BUTTONS.STI",                    0, false, 1.0,  this ),  StringList::ShowCharInfo,       SLOT(charInfo(bool))  },
        { INFO_EQUIP,         QRect( 318,  30,  -1,  -1 ),    new WButton(   "REVIEW/COMMOD_BUTTONS.STI",                    5, false, 1.0,  this ),  StringList::ShowEquipmentInfo,  SLOT(equipInfo(bool)) },

        { NO_ID,              QRect(   0, 165,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWITEMPAGE.STI",                    1,              this ),  -1,                             NULL }, // individual items -- carried
        { NO_ID,              QRect(  11, 172,  95,  12 ),    new WLabel(    StringList::Carried,          Qt::AlignCenter, 10, QFont::Bold, this ),  -1,                             NULL },
        { NO_ID,              QRect( 113, 165,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWITEMPAGE.STI",                    2,              this ),  -1,                             NULL }, // individual items -- worn
        { VAL_STATUE,         QRect( 194, 165,  -1,  -1 ),    new WImage(                                                                    this ),  -1,                             NULL }, // naked individual

        { NO_ID,              QRect( 478, 165,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWITEMPAGE.STI",                    3,              this ),  -1,                             NULL }, // party items
        { NO_ID,              QRect( 512, 175,  95,  12 ),    new WLabel(    StringList::PartyItems,       Qt::AlignCenter, 10, QFont::Bold, this ),  -1,                             NULL },
        { PARTY_WEAPONS,      QRect( 485, 193,  -1,  -1 ),    new WButton(   "REVIEW/INVENTORYFILTERBUTTONS.STI",            0, false, 1.0,  this ),  StringList::WeaponsAndShields,  SLOT(filterButton(bool)) },
        { PARTY_ARMOR,        QRect( 485, 218,  -1,  -1 ),    new WButton(   "REVIEW/INVENTORYFILTERBUTTONS.STI",           15, false, 1.0,  this ),  StringList::Armor,              SLOT(filterButton(bool)) },
        { PARTY_EQUIP,        QRect( 485, 243,  -1,  -1 ),    new WButton(   "REVIEW/INVENTORYFILTERBUTTONS.STI",            5, false, 1.0,  this ),  StringList::Equippable,         SLOT(filterButton(bool)) },
        { PARTY_NONEQUIP,     QRect( 485, 268,  -1,  -1 ),    new WButton(   "REVIEW/INVENTORYFILTERBUTTONS.STI",           20, false, 1.0,  this ),  StringList::NonEquippable,      SLOT(filterButton(bool)) },
        { PARTY_USABLE,       QRect( 485, 301,  -1,  -1 ),    new WButton(   "REVIEW/INVENTORYFILTERBUTTONS.STI",           10, false, 1.0,  this ),  StringList::Usable,             SLOT(filterButton(bool)) },
        { PARTY_UNIDENTIFIED, QRect( 485, 335,  -1,  -1 ),    new WButton(   "REVIEW/INVENTORYFILTERBUTTONS.STI",           40, false, 1.0,  this ),  StringList::Unidentified,       SLOT(filterButton(bool)) },
        { NO_ID,              QRect( 485, 369,  -1,  -1 ),    new WButton(   "REVIEW/INVENTORYFILTERBUTTONS.STI",           30, true,  1.0,  this ),  StringList::SortByType,         SLOT(sortPartyItemsButton(bool)) },
        { NO_ID,              QRect( 514, 422,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWGOLDICON.STI",                    0,              this ),  -1,                             NULL },
        { VAL_GOLD,           QRect( 550, 424,  50,  14 ),    new WLineEdit( "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,                             SLOT(goldChanged(const QString &)) },
        { BTN_GOLD,           QRect( 610, 422,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/PARTYMOVEMENT_BUTTONS.STI",     0, true,  1.0,  this ),  StringList::ApplyGold,          SLOT(setGold(bool)) },

        { ITEM_SWAP,          QRect( 125, 412,  -1,  -1 ),    new WButton(   "REVIEW/INVENTORYSWAPBUTTONS.STI",              0, true,  1.0,  this ),  StringList::SwapWeapons,        SLOT(itemSwap(bool))  },
        { VAL_BACKPACK1,      QRect(  11, 191,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_BACKPACK2,      QRect(  60, 191,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_BACKPACK3,      QRect(  11, 248,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_BACKPACK4,      QRect(  60, 248,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_BACKPACK5,      QRect(  11, 305,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_BACKPACK6,      QRect(  60, 305,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_BACKPACK7,      QRect(  11, 362,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_BACKPACK8,      QRect(  60, 362,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_HEAD,           QRect( 187, 175,  48,  59 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_CLOAK,          QRect( 375, 175,  39,  74 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_TORSO,          QRect( 133, 240,  59,  59 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_HAND,           QRect( 386, 256,  36,  44 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_LEGS,           QRect( 202, 358,  35,  78 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_FEET,           QRect( 361, 377,  27,  59 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_MISC1,          QRect( 430, 197,  41,  41 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_MISC2,          QRect( 430, 239,  41,  41 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_WEAPON1A,       QRect( 156, 307,  36, 102 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_WEAPON1B,       QRect( 399, 307,  35,  73 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_WEAPON2A,       QRect( 120, 307,  35, 102 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_WEAPON2B,       QRect( 435, 307,  36,  73 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },

        { VAL_PARTY1,         QRect( 512, 191,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_PARTY2,         QRect( 560, 191,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_PARTY3,         QRect( 512, 248,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_PARTY4,         QRect( 560, 248,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_PARTY5,         QRect( 512, 305,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_PARTY6,         QRect( 560, 305,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_PARTY7,         QRect( 512, 362,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },
        { VAL_PARTY8,         QRect( 560, 362,  46,  55 ),    new WItem( WItem::context_mode::All,                                           this ),  -1,                             SLOT(itemClick(bool)) },

        { PARTY_SCROLLBAR,    QRect( 611, 193,  15, 223 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,                             SLOT(scrolledPartyItems(int)) },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );
    // context menus on items
    for (int k=0; k<m_widgets.size(); k++)
    {
        if (WItem *itm = qobject_cast<WItem *>(m_widgets[k]))
        {
            // drag and drop handlers
            connect( itm, SIGNAL(itemDragged(item)), this, SLOT(itemDragged(item)) );
            connect( itm, SIGNAL(itemDropped(item)), this, SLOT(itemDropped(item)) );

            // context menu signals
            connect( itm, SIGNAL(inspectItem(item)), this, SLOT(inspectItem(item)) );
            connect( itm, SIGNAL(editItem(item)),    this, SLOT(editItem(item)) );
            connect( itm, SIGNAL(addItem()),         this, SLOT(addItem()) );
            connect( itm, SIGNAL(dropItem(item)),    this, SLOT(dropItem(item)) );
        }
    }

    QVector<int> unimplemented { ITEM_MERGE, ITEM_SPLIT, ITEM_IDENTIFY, ITEM_REMOVE_CURSE, ITEM_UNCURSE_ALL };
    foreach(const int k, unimplemented)
    {
        if (WButton *b = qobject_cast<WButton *>(m_widgets[k]))
        {
            b->setEnabled( false );
        }
    }

    m_infoSelect = new QButtonGroup(this);
    m_infoSelect->addButton( qobject_cast<QPushButton *>(m_widgets[ INFO_CHAR ]),  0 );
    m_infoSelect->addButton( qobject_cast<QPushButton *>(m_widgets[ INFO_EQUIP ]), 1 );

    qobject_cast<QPushButton *>(m_widgets[ INFO_CHAR ])->setChecked( true );
    charInfo(true);

    resetScreen( m_char, m_party );

    resetPartyItemsScrollbar();

    setMouseTracking(true);
}

ScreenItems::~ScreenItems()
{
    delete m_infoSelect;
    // Child widgets are automatically destroyed
}

void ScreenItems::acInfo(bool)
{
    qobject_cast<QPushButton *>(m_widgets[ BTN_OFFENSE ])->setChecked( false );
    qobject_cast<QPushButton *>(m_widgets[ BTN_DEFENSE ])->setChecked( false );

    for (int k=OVL_EQUIPINFO; k < OVL_EQUIPINFO_END; k++)
    {
        // OVL_EQUIPINFO contains OVL_ACINFO, and OVL_ACINFO_END index doesn't actually
        // have a real widget assigned to it
        if (m_widgets.value(k))
        {
            m_widgets[ k ]->setVisible( true );
        }
    }
    for (int k=OVL_ACINFO; k < OVL_ACINFO_END; k++)
    {
        m_widgets[ k ]->setVisible( true );
    }
}

void ScreenItems::equipInfo(bool)
{
    qobject_cast<QPushButton *>(m_widgets[ BTN_OFFENSE ])->setChecked( false );
    qobject_cast<QPushButton *>(m_widgets[ BTN_DEFENSE ])->setChecked( false );

    for (int k=OVL_EQUIPINFO; k < OVL_EQUIPINFO_END; k++)
    {
        // OVL_EQUIPINFO contains OVL_ACINFO, and OVL_ACINFO_END index doesn't actually
        // have a real widget assigned to it
        if (m_widgets.value(k))
        {
            m_widgets[ k ]->setVisible( true );
        }
    }
    for (int k=OVL_ACINFO; k < OVL_ACINFO_END; k++)
    {
        m_widgets[ k ]->setVisible( false );
    }
}

void ScreenItems::charInfo(bool)
{
    for (int k=OVL_EQUIPINFO; k < OVL_EQUIPINFO_END; k++)
    {
        // OVL_EQUIPINFO contains OVL_ACINFO, and OVL_ACINFO_END index doesn't actually
        // have a real widget assigned to it
        if (m_widgets.value(k))
        {
            m_widgets[ k ]->setVisible( false );
        }
    }
}

void ScreenItems::resetPartyItemsScrollbar()
{
    if (m_widgets.value( PARTY_SCROLLBAR ))
    {
        if (QScrollBar *scroll_bar = qobject_cast<QScrollBar *>(m_widgets[ PARTY_SCROLLBAR ]))
        {
            scroll_bar->setRange( 0, (m_party->getItemCount() - (PARTY_ITEMS_END - PARTY_ITEMS_START) + 1) / 2 );
            scroll_bar->setPageStep( (PARTY_ITEMS_END - PARTY_ITEMS_START) / 2 );
            scroll_bar->setSingleStep( 1 );
        }
    }

    // reset the 8 permanent on-screen widgets with actual items from the
    // larger available list of party items based on the scrollbar position
    for (unsigned int k=PARTY_ITEMS_START; k < PARTY_ITEMS_END; k++)
    {
        if (WItem *q = qobject_cast<WItem *>(m_widgets[ k ]))
        {
            if (m_partyItems_scrollPos + k - PARTY_ITEMS_START >= (unsigned int)m_party->getItemCount())
                q->setItem( item() );
            else
            {
                item i = m_party->getItem(m_partyItems_scrollPos + k - PARTY_ITEMS_START);

                q->setItem( i );
                q->setUsabilityBasedOnCharacter( *m_char );
            }
            q->update();
        }
    }
}

void ScreenItems::itemButton(bool checked)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        if (checked)
        {
            m_itemMode = m_widgets.key( q, 0 );

            // uncheck all the other buttons
            for (int i=ITEM_START; i<ITEM_END; i++)
            {
                if ((i != m_itemMode) && (m_widgets.value(i)))
                {
                    if (QPushButton *button = qobject_cast<QPushButton *>(m_widgets[i]))
                    {
                        button->setChecked( false );
                    }
                }
            }

            QPixmap pix = ::getCursor(Qt::WhatsThisCursor);
            if (! pix.isNull())
            {
                this->setCursor( QCursor( pix, 0, 0 ) );
            }
            else
            {
                this->setCursor(Qt::WhatsThisCursor);
            }
        }
        else
        {
            m_itemMode = NO_ID;
            QPixmap pix = ::getCursor(Qt::ArrowCursor);
            if (! pix.isNull())
            {
                this->setCursor( QCursor( pix, 0, 0 ) );
            }
            else
            {
                this->unsetCursor();
            }
        }
    }
}

void ScreenItems::filterButton(bool checked)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        quint32  filter = 0;

        if (checked)
        {
            int button_id = m_widgets.key( q, 0 );

            // uncheck all the other buttons
            if ((button_id >= PARTY_START) && (button_id < PARTY_END))
            {
                for (int i=PARTY_START; i<PARTY_END; i++)
                {
                    if ((i != button_id) && (m_widgets.value(i)))
                    {
                        if (QPushButton *button = qobject_cast<QPushButton *>(m_widgets[i]))
                        {
                            button->setChecked( false );
                        }
                    }
                }
            }

            if (((QPushButton *)m_widgets[PARTY_WEAPONS])->isChecked())
                filter |= party::filter::Weapons;
            if (((QPushButton *)m_widgets[PARTY_ARMOR])->isChecked())
                filter |= party::filter::Armor;
            if (((QPushButton *)m_widgets[PARTY_EQUIP])->isChecked())
                filter |= party::filter::Equippable;
            if (((QPushButton *)m_widgets[PARTY_NONEQUIP])->isChecked())
                filter |= party::filter::NonEquippable;
            if (((QPushButton *)m_widgets[PARTY_USABLE])->isChecked())
                filter |= party::filter::Usable;
        }
        else
        {
            // All filters (except maybe unidentified) get turned off
        }
        if (((QPushButton *)m_widgets[PARTY_UNIDENTIFIED])->isChecked())
            filter |= party::filter::Unidentified;

        m_party->setFilter( filter );

        resetPartyItemsScrollbar();
    }
}

void ScreenItems::sortPartyItemsButton(bool)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        // This isn't actually supposed to be a checkable button, but
        // it's one of the only ones that aren't - others that behave
        // this way usually close dialogs or screens and aren't obvious
        // It's actually nicer to have to button pop down like its
        // checkable and then pop back up again, so will leave it with
        // the checkable mode activated, but just switch it off every
        // time it comes in here.

        q->setChecked(false);

        // Sort the party items - ignore whatever filters are active,
        // we sort everything, and then the filter gets reapplied on
        // showing.

        m_party->sortItems();
        resetPartyItemsScrollbar();
    }
}

void ScreenItems::setGold(bool)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        // another non-checkable button

        q->setChecked(false);

        if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_GOLD ]))
        {
            QString txt;
            int     i;

            // Can't just use the locale to turn it back into an int because if it WAS saying 123,456
            // for instance and someone just deleted the 4 to make it 123,56 it now isn't in the
            // correct locale format and it isn't an unmarked up number either, so manually remove the
            // commas and terminate at periods.

            txt = q->text().replace(",", "");
            i   = txt.indexOf(".");
            if (i != -1)
            {
                txt = txt.left(i);
            }

            m_party->setGold( txt.toULong() );
            q->setText( QLocale(QLocale::English).toString( m_party->getGold() ));
        }

        q->setDisabled( true );
    }
}

void ScreenItems::goldChanged(const QString &)
{
    // enable the Apply Gold button now
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ BTN_GOLD ]))
    {
        q->setDisabled( false );
    }
}

void ScreenItems::itemSwap(bool)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        // another non-checkable button

        q->setChecked(false);

        // A
        item i1 = m_char->getItem( character::worn::Weapon1a );
        item i2 = m_char->getItem( character::worn::Weapon2a );

        m_char->deleteItem( character::worn::Weapon1a  );
        m_char->setItem( character::worn::Weapon1a, i2 );
        m_char->deleteItem( character::worn::Weapon2a  );
        m_char->setItem( character::worn::Weapon2a, i1 );

        WItem *w1 = qobject_cast<WItem *>(m_widgets[ VAL_WEAPON1A ]);
        WItem *w2 = qobject_cast<WItem *>(m_widgets[ VAL_WEAPON2A ]);

        w1->setItem( i2 );
        w2->setItem( i1 );
        w1->setUsabilityBasedOnCharacter( *m_char );
        w2->setUsabilityBasedOnCharacter( *m_char );
        w1->update();
        w2->update();

        // and B
        i1 = m_char->getItem( character::worn::Weapon1b );
        i2 = m_char->getItem( character::worn::Weapon2b );

        m_char->deleteItem( character::worn::Weapon1b  );
        m_char->setItem( character::worn::Weapon1b, i2 );
        m_char->deleteItem( character::worn::Weapon2b  );
        m_char->setItem( character::worn::Weapon2b, i1 );

        w1 = qobject_cast<WItem *>(m_widgets[ VAL_WEAPON1B ]);
        w2 = qobject_cast<WItem *>(m_widgets[ VAL_WEAPON2B ]);

        w1->setItem( i2 );
        w2->setItem( i1 );
        w1->setUsabilityBasedOnCharacter( *m_char );
        w2->setUsabilityBasedOnCharacter( *m_char );
        w1->update();
        w2->update();

        m_char->recomputeEverything();
        resetScreen( m_char, m_party );
    }
}

void ScreenItems::resetScreen(void *char_tag, void *party_tag)
{
    int modAR;
    int multiplier;
    bool party_changed = false;

    if (m_party != party_tag)
    {
        party_changed = true;
    }
    m_char  = (character *)char_tag;
    m_party = (party *)party_tag;

    // Populate text value fields with values from the character

    bool singleWeaponMain    = false;
    bool singleWeaponOffhand = false;

    if ((! m_char->getItem( character::worn::Weapon1a ).isNull()) &&
        (  m_char->getItem( character::worn::Weapon1b ).isNull()))
    {
        singleWeaponMain = true;
    }
    if ((! m_char->getItem( character::worn::Weapon1b ).isNull()) &&
        (  m_char->getItem( character::worn::Weapon1a ).isNull()))
    {
        singleWeaponOffhand = true;
    }

    struct { int id; QString str; } vals[] =
    {
        { VAL_STR,      QString::number( m_char->getAttribute( character::attribute::Strength,     character::atIdx::Current ) ) },
        { VAL_INT,      QString::number( m_char->getAttribute( character::attribute::Intelligence, character::atIdx::Current ) ) },
        { VAL_PIE,      QString::number( m_char->getAttribute( character::attribute::Piety,        character::atIdx::Current ) ) },
        { VAL_VIT,      QString::number( m_char->getAttribute( character::attribute::Vitality,     character::atIdx::Current ) ) },
        { VAL_DEX,      QString::number( m_char->getAttribute( character::attribute::Dexterity,    character::atIdx::Current ) ) },
        { VAL_SPD,      QString::number( m_char->getAttribute( character::attribute::Speed,        character::atIdx::Current ) ) },
        { VAL_SEN,      QString::number( m_char->getAttribute( character::attribute::Senses,       character::atIdx::Current ) ) },

        { VAL_XP,       QLocale(QLocale::English).toString(m_char->getXp()) },
        { VAL_NEXT_XP,  QLocale(QLocale::English).toString(m_char->getXpNeeded()) },

        { VAL_HP1,      QString::number( m_char->getHp( character::atIdx::Current) ) + "/" },
        { VAL_HP2,      QString::number( m_char->getHp( character::atIdx::Base) ) },
        { VAL_STAMINA1, QString::number( m_char->getStamina( character::atIdx::Current) ) + "/" },
        { VAL_STAMINA2, QString::number( m_char->getStamina( character::atIdx::Base) ) },
        { VAL_LOAD1,    QString::number( (int)m_char->getLoad( character::atIdx::Current) ) + "/" },
        { VAL_LOAD2,    QString::number( (int)m_char->getLoad( character::atIdx::Base) ) },

        { VAL_FIRE1,    QString::number( m_char->getMp( character::realm::Fire,   character::atIdx::Current) ) + "/" },
        { VAL_FIRE2,    QString::number( m_char->getMp( character::realm::Fire,   character::atIdx::Base) ) },
        { VAL_WATER1,   QString::number( m_char->getMp( character::realm::Water,  character::atIdx::Current) ) + "/" },
        { VAL_WATER2,   QString::number( m_char->getMp( character::realm::Water,  character::atIdx::Base) ) },
        { VAL_AIR1,     QString::number( m_char->getMp( character::realm::Air,    character::atIdx::Current) ) + "/" },
        { VAL_AIR2,     QString::number( m_char->getMp( character::realm::Air,    character::atIdx::Base) ) },
        { VAL_EARTH1,   QString::number( m_char->getMp( character::realm::Earth,  character::atIdx::Current) ) + "/" },
        { VAL_EARTH2,   QString::number( m_char->getMp( character::realm::Earth,  character::atIdx::Base) ) },
        { VAL_MENTAL1,  QString::number( m_char->getMp( character::realm::Mental, character::atIdx::Current) ) + "/" },
        { VAL_MENTAL2,  QString::number( m_char->getMp( character::realm::Mental, character::atIdx::Base) ) },
        { VAL_DIVINE1,  QString::number( m_char->getMp( character::realm::Divine, character::atIdx::Current) ) + "/" },
        { VAL_DIVINE2,  QString::number( m_char->getMp( character::realm::Divine, character::atIdx::Base) ) },

        { VAL_BASE_AC,  QString::number( m_char->getAC_Base() )    },
        { VAL_AVG_AC,   QString::number( m_char->getAC_Average() ) },

        { VAL_GOLD,     QLocale(QLocale::English).toString(m_party->getGold()) },

        { LBL_PRIM,     ((m_char->getItem( character::worn::Weapon1a ).isNull()) && (m_char->getItem( character::worn::Weapon1b ).isNull())) ?  ::getStringTable()->getString( StringList::Kick )
                                                                                                                                 :  ::getStringTable()->getString( StringList::Primary )   },
        { LBL_SECO,     ((m_char->getItem( character::worn::Weapon1a ).isNull()) && (m_char->getItem( character::worn::Weapon1b ).isNull())) ?  ::getStringTable()->getString( StringList::Punch )
                                                                                                                                 :  ::getStringTable()->getString( StringList::Secondary ) },

        { INITIATIVE,         QString::number( m_char->getInitiative() ) },
        { KILLS,              QString::number( m_char->getKills() )      },
        { DEATHS,             QString::number( m_char->getDeaths() )     },

        { PRIM_TOINIT,        (singleWeaponOffhand ? "" : QString::asprintf( "%+d", m_char->getToInit( true  ) ) ) },
        { SECO_TOINIT,        (singleWeaponMain    ? "" : QString::asprintf( "%+d", m_char->getToInit( false ) ) ) },
        { PRIM_DAMAGE,        (singleWeaponOffhand ? "" : m_char->getDamageString( true,  true  ) )      },
        { SECO_DAMAGE,        (singleWeaponMain    ? "" : m_char->getDamageString( false, true  ) )      },
        { PRIM_ATTACK,        (singleWeaponOffhand ? "" : m_char->getAttackRatingString( true,  true ) ) },
        { SECO_ATTACK,        (singleWeaponMain    ? "" : m_char->getAttackRatingString( false, true ) ) },
        { PRIM_ATTACKS,       (singleWeaponOffhand ? "" : QString::number( m_char->getNumAttacks( true ) ) )  },
        { SECO_ATTACKS,       (singleWeaponMain    ? "" : QString::number( m_char->getNumAttacks( false ) ) ) },
        { PRIM_SWINGS,        (singleWeaponOffhand ? "" : QString::number( m_char->getMaxSwings( true ) ) )   },
        { SECO_SWINGS,        (singleWeaponMain    ? "" : QString::number( m_char->getMaxSwings( false ) ) )  },
        { PRIM_TOHIT,         (singleWeaponOffhand ? "" : (m_char->getAttackRating( true,  NULL, &modAR ) , QString::asprintf( "%+d", modAR ) ) ) },
        { SECO_TOHIT,         (singleWeaponMain    ? "" : (m_char->getAttackRating( false, NULL, &modAR ) , QString::asprintf( "%+d", modAR ) ) ) },
        { PRIM_TOPENETRATE,   (singleWeaponOffhand ? "" : QString::asprintf( "%+d", m_char->getToPenetrate( true ) ) ) },
        { SECO_TOPENETRATE,   (singleWeaponMain    ? "" : QString::asprintf( "%+d", m_char->getToPenetrate( false ) ) ) },
        { PRIM_TODAMAGE,      (m_char->getDamage( true,  NULL, NULL, &multiplier ) ? QString::asprintf( "%+d%%", multiplier ) : "") },
        { SECO_TODAMAGE,      (m_char->getDamage( false, NULL, NULL, &multiplier ) ? QString::asprintf( "%+d%%", multiplier ) : "") },

        { ACMOD_RACE,         (m_char->getACMod_Race()         ? QString::asprintf( "%+d", m_char->getACMod_Race() )        : "" ) },
        { ACMOD_SPEED,        (m_char->getACMod_Speed()        ? QString::asprintf( "%+d", m_char->getACMod_Speed() )       : "" ) },
        { ACMOD_STEALTH,      (m_char->getACMod_Stealth()      ? QString::asprintf( "%+d", m_char->getACMod_Stealth() )     : "" ) },
        { ACMOD_SHIELD,       (m_char->getACMod_Shield()       ? QString::asprintf( "%+d", m_char->getACMod_Shield() )      : "" ) },
        { ACMOD_MAGICITEMS,   (m_char->getACMod_MagicItems()   ? QString::asprintf( "%+d", m_char->getACMod_MagicItems() )  : "" ) },
        { ACMOD_MAGICSPELLS,  (m_char->getACMod_MagicSpells()  ? QString::asprintf( "%+d", m_char->getACMod_MagicSpells() ) : "" ) },
        { ACMOD_VPENETRATION, (m_char->getACMod_Penetration()  ? QString::asprintf( "%+d", m_char->getACMod_Penetration() ) : "" ) },
        { ACMOD_ENCUMBRANCE,  (m_char->getACMod_Encumbrance()  ? QString::asprintf( "%+d", m_char->getACMod_Encumbrance() ) : "" ) },
        { ACMOD_CONDITIONS,   (m_char->getACMod_Conditions()   ? QString::asprintf( "%+d", m_char->getACMod_Conditions() )  : "" ) },
        { ACMOD_FATIGUE,      (m_char->getACMod_Fatigue()      ? QString::asprintf( "%+d", m_char->getACMod_Fatigue() )     : "" ) },
        { ACMOD_DEFENSIVEACT, (m_char->getACMod_Defensive()    ? QString::asprintf( "%+d", m_char->getACMod_Defensive() )   : "" ) },
        { ACMOD_REFLEXTION,   (m_char->getACMod_Reflextion()   ? QString::asprintf( "%+d", m_char->getACMod_Reflextion() )  : "" ) },
        { DAMAGE_ABSORPTION,  QString::asprintf( "%d%%", m_char->getDamageAbsorption() ) },

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
    // The Apply Gold button only becomes accessable when the gold text box is changed;
    // We've just reset the gold to the stored value, so disable the button again
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ BTN_GOLD ]))
    {
        q->setDisabled( true );
    }

    if (!singleWeaponOffhand)
    {
        m_widgets[ PRIM_DAMAGE ]->setToolTip( m_char->getDamageString( true,  false ) );
        m_widgets[ PRIM_ATTACK ]->setToolTip( m_char->getAttackRatingString( true,  false ) );
    }
    if (!singleWeaponMain)
    {
        m_widgets[ SECO_DAMAGE ]->setToolTip( m_char->getDamageString( false, false ) );
        m_widgets[ SECO_ATTACK ]->setToolTip( m_char->getAttackRatingString( false, false ) );
    }

    // Change the colour of the Load text based on percent of maximum
    WLabel *q1 = qobject_cast<WLabel *>(m_widgets[ VAL_LOAD1 ]);
    WLabel *q2 = qobject_cast<WLabel *>(m_widgets[ VAL_LOAD2 ]);
    if (q1 && q2)
    {
        quint32 colr = 0;

        switch (m_char->getLoadCategory())
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
        q1->setStyleSheet( QString::asprintf("QLabel {color: #%06x}", colr) );
        q2->setStyleSheet( QString::asprintf("QLabel {color: #%06x}", colr) );
    }

    // set the progress bar behind the xp need for next level
    if (WImage *w = qobject_cast<WImage *>(m_widgets[ VAL_NEXT_XP_BAR ]))
    {
        double bar_fraction = (double)(m_char->getXp() - m_char->getXpLastLevelNeeded()) /
                              (double)(m_char->getXpNeeded() - m_char->getXpLastLevelNeeded());

        if (m_char->getXp() >= m_char->getXpNeeded())
            bar_fraction = 1.0;

        w->setLengthRestrict( Qt::AlignLeft, bar_fraction, 1.0 );
    }

    // set the picture to the newly selected character
    if (WImage *w = qobject_cast<WImage *>(m_widgets[ VAL_STATUE ]))
    {
        w->setPixmap( ScreenCommon::getStatue( m_char->getRace(), m_char->getGender() ) );
    }

    // Assign personal items
    for (int k=0; k<character::worn::WORN_SIZE; k++)
    {
        WItem *qItem = qobject_cast<WItem *>(m_widgets[ ITEMS_START + k ]);

        const item w = m_char->getItem( static_cast<character::worn>(k) );
        qItem->setItem( w );

        qItem->setUsabilityBasedOnCharacter( *m_char );
    }
    // If someone opened a new file, the party will have changed,
    // so we have to redraw the party items also
    if (party_changed)
    {
        resetPartyItemsScrollbar();
    }
}

void ScreenItems::wheelEvent(QWheelEvent *event)
{
    // Send the event to the scrollbar child - our scrollbar isn't
    // actually hooked up to a scrollpane, so the only way it gets
    // the event directly itself otherwise is if the mouse is right
    // over it. This makes it work for anywhere inside the window.
    QObjectList kids = children();

    for (int k=0; k<kids.size(); k++)
    {
        if (QScrollBar *q = qobject_cast<QScrollBar *>(kids.at(k)))
        {
            // We set up recursive behaviour if we attempt to scroll up at the top of
            // the scrollbar, or down at the bottom (and you then have to unwind it
            // in the opposite direction before the wheel works again) so filter out
            // up events at the top, and down events at the bottom. This is the mess
            // you inherit by wanting the wheel to scroll one tiny area regardless of
            // where in the window it is.
            QPoint delta = event->angleDelta();

            if      ((delta.y() > 0) && (m_partyItems_scrollPos == 0))
            {
                event->ignore();
            }
            else if ((delta.y() < 0) && (m_partyItems_scrollPos + (PARTY_ITEMS_END - PARTY_ITEMS_START) >= (int)m_party->getItemCount()))
            {
                event->ignore();
            }
            else
            {
                // We have to allocate a new event on the heap or
                // else postEvent() can't free it correctly. But
                // we don't need to update positions in the event.
                QWheelEvent *r = new QWheelEvent( *event );

                QApplication::postEvent(q, r);
            }
            return;
        }
    }
    QWidget::wheelEvent(event);
}

void ScreenItems::itemClick(bool down)
{
    if (WItem *q = qobject_cast<WItem *>(this->sender()))
    {
        if (down)
        {
            const item &i = q->getItem();

            switch (m_itemMode)
            {
                case ITEM_ADD:
                {
                    addItem();
                    break;
                }

                case ITEM_INSPECT:
                {
                    if (! i.isNull())
                    {
                        inspectItem( i );
                    }
                    break;
                }

                case ITEM_DROP:
                {
                    if (! i.isNull())
                    {
                        dropItem( i );
                    }
                    break;
                }

                default:
                    // FIXME: not implemented yet
                    break;
            }
        }
    }
}

void ScreenItems::itemDragged( item )
{
    if (WItem *q = qobject_cast<WItem *>(this->sender()))
    {
        int item_id = m_widgets.key( q );

        if ((item_id >= ITEMS_START) &&
            (item_id <  ITEMS_END))
        {
            // personal item
            m_char->deleteItem( static_cast<character::worn>(item_id - ITEMS_START) );
            q->update();

            m_char->recomputeEverything();
            m_party->divvyUpPartyWeight();
        }
        else if ((item_id >= PARTY_ITEMS_START) &&
                 (item_id <  PARTY_ITEMS_END))
        {
            // party item
            m_party->deleteItem(m_partyItems_scrollPos + item_id - PARTY_ITEMS_START);
            resetPartyItemsScrollbar();

            m_party->divvyUpPartyWeight();
        }
    }
}

void ScreenItems::inspectItem(item i)
{
    new DialogItemInfo(i, this);
}

void ScreenItems::editItem(item i)
{
    if (WItem *q = qobject_cast<WItem *>(this->sender()))
    {
        int   item_id = m_widgets.key( q );

        DialogAddItem *ai = new DialogAddItem(item_id, i, this);
        connect( ai, SIGNAL(itemAdded(int, item)), this, SLOT(itemEdited(int, item)));
    }
    resetScreen( m_char, m_party );
}

void ScreenItems::addItem()
{
    if (WItem *q = qobject_cast<WItem *>(this->sender()))
    {
        int   item_id = m_widgets.key( q );

        DialogAddItem *ai = new DialogAddItem(item_id, this);
        connect( ai, SIGNAL(itemAdded(int, item)), this, SLOT(itemAdded(int, item)));
    }
    resetScreen( m_char, m_party );
}

void ScreenItems::dropItem(item i)
{
    if (WItem *q = qobject_cast<WItem *>(this->sender()))
    {
        int item_id = m_widgets.key( q );

        if ((item_id >= ITEMS_START) &&
            (item_id <  ITEMS_END))
        {
            // personal item
            m_party->addDroppedItem(i);
            m_char->deleteItem( static_cast<character::worn>(item_id - ITEMS_START) );

            q->setItem( item() );
            q->setUsabilityBasedOnCharacter( *m_char );
            q->update();

            m_char->recomputeEverything();
            m_party->divvyUpPartyWeight();
        }
        else if ((item_id >= PARTY_ITEMS_START) &&
                 (item_id <  PARTY_ITEMS_END))
        {
            // party item
            m_party->addDroppedItem(i);
            m_party->deleteItem(m_partyItems_scrollPos + item_id - PARTY_ITEMS_START);
            resetPartyItemsScrollbar();

            m_party->divvyUpPartyWeight();
        }
    }
    resetScreen( m_char, m_party );
}

void ScreenItems::itemEdited(int tag, item i )
{
    setItemAtLoc( i, tag, true );
}

void ScreenItems::itemAdded(int tag, item i )
{
    setItemAtLoc( i, tag );
}

void ScreenItems::itemDropped( item i )
{
    if (WItem *q = qobject_cast<WItem *>(this->sender()))
    {
        setItemAtLoc( i, m_widgets.key( q ));
    }
}

void ScreenItems::setItemAtLoc( item i, int item_loc, bool in_place )
{
    if (WItem *q = qobject_cast<WItem *>(m_widgets[ item_loc ]))
    {
        if ((item_loc >= ITEMS_START) &&
            (item_loc <  ITEMS_END))
        {
            // dropped over personal slot - push
            // any existing item onto our dropped item
            // stack and take its place
            const item &old_item = q->getItem();

            if (! old_item.isNull() && !in_place)
            {
                m_party->addDroppedItem( old_item );
            }
            m_char->deleteItem( static_cast<character::worn>(item_loc - ITEMS_START) );
            m_char->setItem( static_cast<character::worn>(item_loc - ITEMS_START), i );

            q->setItem( i );
            q->setUsabilityBasedOnCharacter( *m_char );
            q->update();

            m_char->recomputeEverything();
            m_party->divvyUpPartyWeight();
        }
        else if ((item_loc >= PARTY_ITEMS_START) &&
                 (item_loc <  PARTY_ITEMS_END))
        {
            // dropped over party item - shove
            // the existing item and everything
            // to the right of it one slot along
            // and insert at this point. Insertion
            // point will have limited accuracty if
            // filters are active

            m_party->insertItem(m_partyItems_scrollPos + item_loc - PARTY_ITEMS_START, i);
            resetPartyItemsScrollbar();

            m_party->divvyUpPartyWeight();
        }
    }
    resetScreen( m_char, m_party );
}

void ScreenItems::scrolledPartyItems(int position)
{
    // 2 items per row
    m_partyItems_scrollPos = position * 2;
    resetPartyItemsScrollbar();
}

void ScreenItems::setVisible( bool visible )
{
    // don't toggle child visibility - this screen has a number of hidden
    // widgets at any point in time.

    QWidget::setVisible( visible );
}
