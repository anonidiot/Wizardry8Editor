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
#include <QHelpEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>

#include "common.h"
#include "ScreenCommon.h"
#include "ScreenAttribs.h"
#include "DialogInfo.h"

#include "StringList.h"
#include "WButton.h"
#include "WDDL.h"
#include "WImage.h"
#include "WLabel.h"
#include "WListWidget.h"
#include "WSpinBox.h"
#include "WStatBar.h"

#include "SLFFile.h"
#include "STI.h"

#include "spell.h"
#include "main.h"

#include <QDebug>

typedef enum
{
    NO_ID,

    DDL_PROFS,
    DDL_RACES,
    DDL_GENDERS,

    VAL_STATUE,

    VAL_HP,
    VAL_STAMINA,
    VAL_SP,
    VAL_AC,
    VAL_CC,

    VAL_MR_FIRE,
    VAL_MR_WATER,
    VAL_MR_AIR,
    VAL_MR_EARTH,
    VAL_MR_MENTAL,
    VAL_MR_DIVINE,

    SCROLL_ABILITIES,
    SCROLL_SKILLS,

    ATTRIB_INSPECT,

    LBL_ATTRIBUTES_START,
    LBL_ATTRIBUTES = LBL_ATTRIBUTES_START,
    LBL_ATTRIBUTES_END = LBL_ATTRIBUTES_START + character::attribute::ATTRIBUTE_SIZE,

    VAL_ATTRIBUTES_START,
    VAL_ATTRIBUTES = VAL_ATTRIBUTES_START,
    VAL_ATTRIBUTES_END = VAL_ATTRIBUTES_START + character::attribute::ATTRIBUTE_SIZE,

    CLC_ATTRIBUTES_START,
    CLC_ATTRIBUTES = CLC_ATTRIBUTES_START,
    CLC_ATTRIBUTES_END = CLC_ATTRIBUTES_START + character::attribute::ATTRIBUTE_SIZE,

    SIZE_WIDGET_IDS
} widget_ids;


ScreenAttribs::ScreenAttribs(character *c, QWidget *parent) :
    Screen(parent),
    m_char(c),
    m_inspectMode(false)
{
    QPixmap rowImg = makeRowPixmap();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0, 450,  -1,  -1 ),    new WImage(    "REVIEW/BOTTOMBUTTONBAR.STI",          0,                          this ),  -1,                             NULL },

        { NO_ID,              QRect(   0, 164,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWSTATSPAGE.STI",          1,                          this ),  -1,  NULL },
        { NO_ID,              QRect( 160, 296,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWSTATSPAGE.STI",          1, 205, 132, 104, 153, 1.0, this ),  -1,  NULL },

        { NO_ID,              QRect( 310,   0,  -1,  -1 ),    new WImage(    "DIALOGS/DIALOGBACKGROUND.STI",        0,                          this ),  -1,  NULL },
        { NO_ID,              QRect( 310, 150,  -1,  -1 ),    new WImage(    "DIALOGS/DIALOGBACKGROUND.STI",        0,   0,   0, 190, 290, 1.0, this ),  -1,  NULL },
        { NO_ID,              QRect( 450, 200,  -1,  -1 ),    new WImage(    "DIALOGS/DIALOGBACKGROUND.STI",        0,   0,   0, 190, 240, 1.0, this ),  -1,  NULL },
        { NO_ID,              QRect( 450,   0,  -1,  -1 ),    new WImage(    "DIALOGS/DIALOGBACKGROUND.STI",        0,                          this ),  -1,  NULL },

        { NO_ID,              QRect( 310,   1,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PROFESSION.STI",   0,   0,   0, 225, 158, 1.0, this ),  -1,  NULL },

        { NO_ID,              QRect( 477, 297,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PROFESSION.STI",   0,  12, 312, 161,  90, 1.0, this ),  -1,  NULL },
        { NO_ID,              QRect( 540, 297,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PROFESSION.STI",   0, 138, 312,  98,  90, 1.0, this ),  -1,  NULL },

        { NO_ID,              QRect( 477, 353,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PROFESSION.STI",   0,  12, 342, 161,  60, 1.0, this ),  -1,  NULL },
        { NO_ID,              QRect( 540, 353,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PROFESSION.STI",   0, 138, 342,  98,  60, 1.0, this ),  -1,  NULL },

        { NO_ID,              QRect( 264, 299,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PROFESSION.STI",   0,  20, 166, 215,  80, 1.0, this ),  -1,  NULL },
        { NO_ID,              QRect( 264, 359,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PROFESSION.STI",   0,  20, 230, 217,  82, 1.0, this ),  -1,  NULL },

        { NO_ID,              QRect( 310, 168,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PROFESSION.STI",   0, 238, 166, 208, 105, 1.0, this ),  -1,  NULL },

        { VAL_STATUE,         QRect( 520, 130,  -1,  -1 ),    new WImage(                                                                       this ),  -1,  NULL }, // naked individual

        { NO_ID,              QRect( 315, 173, 181,  14 ),    new WLabel(    StringList::SecondaryAttributes, Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 318, 188, 148,  14 ),    new WLabel(    StringList::SAHitPoints,         Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_HP,             QRect( 436, 188,  37,  14 ),    new WLabel(    "",                              Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 318, 202, 148,  14 ),    new WLabel(    StringList::SAStamina,           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_STAMINA,        QRect( 436, 202,  37,  14 ),    new WLabel(    "",                              Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 318, 216, 148,  14 ),    new WLabel(    StringList::SpellPoints,         Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_SP,             QRect( 436, 216,  37,  14 ),    new WLabel(    "",                              Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 318, 230, 148,  14 ),    new WLabel(    StringList::SAArmorClass,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_AC,             QRect( 436, 230,  37,  14 ),    new WLabel(    "",                              Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 318, 244, 148,  14 ),    new WLabel(    StringList::SACarryCapacity,     Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_CC,             QRect( 436, 244,  37,  14 ),    new WLabel(    "",                              Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },

        { NO_ID,              QRect(  11, 304, 224,  14 ),    new WLabel(    StringList::AbilitiesTraits,     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { SCROLL_ABILITIES,   QRect(  11, 319, 246, 120 ),    new WListWidget(                                                                  this ),  -1,  NULL },
        { NO_ID,              QRect( 269, 304, 203,  14 ),    new WLabel(    StringList::ProfessionalSkills,  Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { SCROLL_SKILLS,      QRect( 269, 319, 205, 120 ),    new WListWidget(                                                                  this ),  -1,  NULL },

        { NO_ID,              QRect( 491, 304, 140,  14 ),    new WLabel(    StringList::ResistanceModifiers, Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 500, 326,  -1,  -1 ),    new WImage(    "SPELL CASTING/FIRE_REALM.STI",           0,                       this ),  -1,  NULL },
        { VAL_MR_FIRE,        QRect( 520, 326,  34,  14 ),    new WLabel(    "",                              Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 563, 326,  -1,  -1 ),    new WImage(    "SPELL CASTING/WATER_REALM.STI",          8,                       this ),  -1,  NULL },
        { VAL_MR_WATER,       QRect( 583, 326,  34,  14 ),    new WLabel(    "",                              Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 500, 351,  -1,  -1 ),    new WImage(    "SPELL CASTING/AIR_REALM.STI",            0,                       this ),  -1,  NULL },
        { VAL_MR_AIR,         QRect( 520, 352,  34,  14 ),    new WLabel(    "",                              Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 563, 351,  -1,  -1 ),    new WImage(    "SPELL CASTING/EARTH_REALM.STI",          0,                       this ),  -1,  NULL },
        { VAL_MR_EARTH,       QRect( 583, 352,  34,  14 ),    new WLabel(    "",                              Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 500, 376,  -1,  -1 ),    new WImage(    "SPELL CASTING/MENTAL_REALM.STI",         0,                       this ),  -1,  NULL },
        { VAL_MR_MENTAL,      QRect( 520, 378,  34,  14 ),    new WLabel(    "",                              Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 563, 376,  -1,  -1 ),    new WImage(    "SPELL CASTING/DIVINE_REALM.STI",        11,                       this ),  -1,  NULL },
        { VAL_MR_DIVINE,      QRect( 583, 378,  34,  14 ),    new WLabel(    "",                              Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },

        { NO_ID,              QRect( 136, 188,  -1,  -1 ),    new WImage(    rowImg,                                                            this ),  -1,  NULL },
        { NO_ID,              QRect( 136, 202,  -1,  -1 ),    new WImage(    rowImg,                                                            this ),  -1,  NULL },
        { NO_ID,              QRect( 136, 216,  -1,  -1 ),    new WImage(    rowImg,                                                            this ),  -1,  NULL },
        { NO_ID,              QRect( 136, 230,  -1,  -1 ),    new WImage(    rowImg,                                                            this ),  -1,  NULL },
        { NO_ID,              QRect( 136, 244,  -1,  -1 ),    new WImage(    rowImg,                                                            this ),  -1,  NULL },
        { NO_ID,              QRect( 136, 258,  -1,  -1 ),    new WImage(    rowImg,                                                            this ),  -1,  NULL },
        { NO_ID,              QRect( 136, 272,  -1,  -1 ),    new WImage(    rowImg,                                                            this ),  -1,  NULL },


        { ATTRIB_INSPECT,     QRect(   0, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWITEMBUTTONS.STI",          0, false, 1.0,         this ),  StringList::InspectSkill,     SLOT(attributeDetail(bool)) },

        { NO_ID,                                              QRect(  11, 173, 286,  14 ),    new WLabel(    StringList::PrimaryAttributes,                                          Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { LBL_ATTRIBUTES + character::attribute::Strength,    QRect(  11, 188, 122,  14 ),    new WLabel(    StringList::LISTPrimaryAttributes + character::attribute::Strength,     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_ATTRIBUTES + character::attribute::Intelligence,QRect(  11, 202, 122,  14 ),    new WLabel(    StringList::LISTPrimaryAttributes + character::attribute::Intelligence, Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_ATTRIBUTES + character::attribute::Piety,       QRect(  11, 216, 122,  14 ),    new WLabel(    StringList::LISTPrimaryAttributes + character::attribute::Piety,        Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_ATTRIBUTES + character::attribute::Vitality,    QRect(  11, 230, 122,  14 ),    new WLabel(    StringList::LISTPrimaryAttributes + character::attribute::Vitality,     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_ATTRIBUTES + character::attribute::Dexterity,   QRect(  11, 244, 122,  14 ),    new WLabel(    StringList::LISTPrimaryAttributes + character::attribute::Dexterity,    Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_ATTRIBUTES + character::attribute::Speed,       QRect(  11, 258, 122,  14 ),    new WLabel(    StringList::LISTPrimaryAttributes + character::attribute::Speed,        Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },
        { LBL_ATTRIBUTES + character::attribute::Senses,      QRect(  11, 272, 122,  14 ),    new WLabel(    StringList::LISTPrimaryAttributes + character::attribute::Senses,       Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  SLOT(info(bool)) },

        { CLC_ATTRIBUTES + character::attribute::Strength,    QRect( 136, 188,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_ATTRIBUTES + character::attribute::Intelligence,QRect( 136, 202,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_ATTRIBUTES + character::attribute::Piety,       QRect( 136, 216,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_ATTRIBUTES + character::attribute::Vitality,    QRect( 136, 230,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_ATTRIBUTES + character::attribute::Dexterity,   QRect( 136, 244,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_ATTRIBUTES + character::attribute::Speed,       QRect( 136, 258,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { CLC_ATTRIBUTES + character::attribute::Senses,      QRect( 136, 272,  82,  14 ),    new WStatBar(  true,                                                     Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },

        { VAL_ATTRIBUTES + character::attribute::Strength,    QRect( 224, 188,  76,  14 ),    new WSpinBox(  0, 0, 125,                         this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_ATTRIBUTES + character::attribute::Intelligence,QRect( 224, 202,  76,  14 ),    new WSpinBox(  0, 0, 125,                         this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_ATTRIBUTES + character::attribute::Piety,       QRect( 224, 216,  76,  14 ),    new WSpinBox(  0, 0, 125,                         this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_ATTRIBUTES + character::attribute::Vitality,    QRect( 224, 230,  76,  14 ),    new WSpinBox(  0, 0, 125,                         this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_ATTRIBUTES + character::attribute::Dexterity,   QRect( 224, 244,  76,  14 ),    new WSpinBox(  0, 0, 125,                         this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_ATTRIBUTES + character::attribute::Speed,       QRect( 224, 258,  76,  14 ),    new WSpinBox(  0, 0, 125,                         this ),  -1,  SLOT(spinnerChanged(int)) },
        { VAL_ATTRIBUTES + character::attribute::Senses,      QRect( 224, 272,  76,  14 ),    new WSpinBox(  0, 0, 125,                         this ),  -1,  SLOT(spinnerChanged(int)) },

        { DDL_GENDERS,        QRect( 329, 114,  -1,  -1 ),    new WDDL(          "Lucida Calligraphy",        Qt::AlignLeft,  9, QFont::Thin,   this ),  -1,  SLOT(ddlChanged(int)) },
        { DDL_RACES,          QRect( 329,  63,  -1,  -1 ),    new WDDL(          "Lucida Calligraphy",        Qt::AlignLeft,  9, QFont::Thin,   this ),  -1,  SLOT(ddlChanged(int)) },
        { DDL_PROFS,          QRect( 329,  12,  -1,  -1 ),    new WDDL(          "Lucida Calligraphy",        Qt::AlignLeft,  9, QFont::Thin,   this ),  -1,  SLOT(ddlChanged(int)) },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_PROFS ] ))
    {
        populateDDLProfessions( ddl );
        connect( ddl, SIGNAL(listActive()), this, SLOT(ddlActive()) );
    }
    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_RACES ] ))
    {
        populateDDLRaces( ddl );
        connect( ddl, SIGNAL(listActive()), this, SLOT(ddlActive()) );
    }
    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_GENDERS ] ))
    {
        populateDDLGenders( ddl );
        connect( ddl, SIGNAL(listActive()), this, SLOT(ddlActive()) );
    }
    if (WListWidget *w = qobject_cast<WListWidget *>(m_widgets[ SCROLL_ABILITIES ] ))
    {
        w->setSelectionMode( QAbstractItemView::NoSelection );
    }
    if (WListWidget *w = qobject_cast<WListWidget *>(m_widgets[ SCROLL_SKILLS ] ))
    {
        w->setSelectionMode( QAbstractItemView::NoSelection );
    }

    if (! m_char->isNull() )
        resetScreen( m_char, NULL );
}

ScreenAttribs::~ScreenAttribs()
{
    // Child widgets are automatically destroyed
}

void ScreenAttribs::ddlChanged(int value)
{
    if (sender() == m_widgets[ DDL_PROFS ])
    {
        m_char->setProfession( static_cast<character::profession>( value ) );

        emit changedProf();
    }
    else if (sender() == m_widgets[ DDL_RACES ])
    {
        m_char->setRace( static_cast<character::race>( value ) );

        emit changedRace();
    }
    else if (sender() == m_widgets[ DDL_GENDERS ])
    {
        m_char->setGender( static_cast<character::gender>( value ) );

        emit changedSex();
    }

    resetScreen( m_char, NULL ); // update professional skills and any profession related stats
}

void ScreenAttribs::ddlActive()
{
    if (sender() != m_widgets[ DDL_PROFS ])
    {
        if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_PROFS ] ))
        {
            ddl->showDDL( false );
        }
    }
    if (sender() != m_widgets[ DDL_RACES ])
    {
        if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_RACES ] ))
        {
            ddl->showDDL( false );
        }
    }
    if (sender() != m_widgets[ DDL_GENDERS ])
    {
        if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_GENDERS ] ))
        {
            ddl->showDDL( false );
        }
    }
}

QPixmap ScreenAttribs::makeRowPixmap()
{
    // We want a combination of 2 Wizardry images here so we get the benefit
    // of the skills screen which better suits the Screen flow, but has the
    // spinner widgets for actually editting things.

    QPixmap rowImg,     spnImg;
    QSize   rowImgSize;

    // images used in dialog
    rowImg = SLFFile::getPixmapFromSlf( "REVIEW/REVIEWSKILLSPAGE.STI", 3 );
    spnImg = SLFFile::getPixmapFromSlf( "CHAR GENERATION/CG_SKILLS.STI", 2 );

    rowImgSize = rowImg.size();
    rowImgSize.setWidth( rowImg.width() - 108);

    QPixmap customImage( rowImgSize );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, rowImg,  108,   0, rowImgSize.width(), rowImgSize.height() );
    p.drawPixmap(  rowImgSize.width() - spnImg.width() + 106,   0, spnImg, 106,   0, spnImg.width() - 106, spnImg.height() );
    p.drawPixmap(  rowImgSize.width() - spnImg.width() + 102,   0, spnImg, 106,   0, 50, spnImg.height() );
    p.end();

    return customImage;
}

void ScreenAttribs::populateDDLProfessions(WDDL *ddl)
{
    if (ddl)
    {
        QMetaEnum metaProf = QMetaEnum::fromType<character::profession>();

        SLFFile ic( "CHAR GENERATION/CG_ICONS_PROFESSION.STI" );

        if (ic.open(QFile::ReadOnly))
        {
            QByteArray array = ic.readAll();
            STI c( array );

            for (int k=0; k<metaProf.keyCount(); k++)
            {
                character::profession p = static_cast<character::profession>( metaProf.value(k) );

                int  imIdx;

                switch (p)
                {
                    default:
                    case character::profession::Fighter:   imIdx = 10; break;
                    case character::profession::Lord:      imIdx = 24; break;
                    case character::profession::Valkyrie:  imIdx =  8; break;
                    case character::profession::Ranger:    imIdx =  2; break;
                    case character::profession::Samurai:   imIdx = 16; break;
                    case character::profession::Ninja:     imIdx = 20; break;
                    case character::profession::Monk:      imIdx = 12; break;
                    case character::profession::Rogue:     imIdx = 28; break;
                    case character::profession::Gadgeteer: imIdx =  6; break;
                    case character::profession::Bard:      imIdx =  0; break;
                    case character::profession::Priest:    imIdx = 18; break;
                    case character::profession::Alchemist: imIdx = 22; break;
                    case character::profession::Bishop:    imIdx =  4; break;
                    case character::profession::Psionic:   imIdx = 14; break;
                    case character::profession::Mage:      imIdx = 26; break;
                }

                QListWidgetItem *prof = new QListWidgetItem( ::getBaseStringTable()->getString( StringList::LISTProfessions + k ));
                prof->setData( Qt::DecorationRole, QPixmap::fromImage( c.getImage( imIdx ) ) );
                prof->setData( Qt::UserRole, p );

                ddl->addItem( prof );
            }

            ic.close();
        }
    }
}

void ScreenAttribs::populateDDLRaces(WDDL *ddl)
{
    if (ddl)
    {
        QMetaEnum metaRace = QMetaEnum::fromType<character::race>();

        SLFFile ic( "CHAR GENERATION/CG_ICONS_RACE.STI" );

        if (ic.open(QFile::ReadOnly))
        {
            QByteArray array = ic.readAll();
            STI c( array );

            for (int k=0; k<metaRace.keyCount(); k++)
            {
                character::race r = static_cast<character::race>( metaRace.value(k) );

                int  imIdx;

                switch (r)
                {
                    default:
                    case character::race::Human:     imIdx =  8; break;
                    case character::race::Elf:       imIdx =  4; break;
                    case character::race::Dwarf:     imIdx = 12; break;
                    case character::race::Gnome:     imIdx = 16; break;
                    case character::race::Hobbit:    imIdx = 14; break;
                    case character::race::Faerie:    imIdx = 20; break;
                    case character::race::Lizardman: imIdx =  0; break;
                    case character::race::Dracon:    imIdx = 10; break;
                    case character::race::Felpurr:   imIdx =  2; break;
                    case character::race::Rawulf:    imIdx = 18; break;
                    case character::race::Mook:      imIdx =  6; break;
                    case character::race::Trynnie:   imIdx = 30; break;
                    case character::race::TRang:     imIdx = 24; break;
                    case character::race::Umpani:    imIdx = 22; break;
                    case character::race::Rapax:     imIdx = 26; break;
                    case character::race::Android:   imIdx = 28; break;
                }

                QListWidgetItem *race = new QListWidgetItem( ::getBaseStringTable()->getString( StringList::LISTRaces + k ));
                race->setData( Qt::DecorationRole, QPixmap::fromImage( c.getImage( imIdx ) ) );
                race->setData( Qt::UserRole, r );

                ddl->addItem( race );
            }

            ic.close();
        }
    }
}

void ScreenAttribs::populateDDLGenders(WDDL *ddl)
{
    if (ddl)
    {
        QMetaEnum metaGender = QMetaEnum::fromType<character::gender>();

        SLFFile ic( "CHAR GENERATION/CG_ICONS_GENDER.STI" );

        if (ic.open(QFile::ReadOnly))
        {
            QByteArray array = ic.readAll();
            STI c( array );

            for (int k=0; k<metaGender.keyCount(); k++)
            {
                character::gender g = static_cast<character::gender>( metaGender.value(k) );

                int  imIdx;

                switch (g)
                {
                    default:
                    case character::gender::Male:     imIdx =  0; break;
                    case character::gender::Female:   imIdx =  2; break;
                }

                QListWidgetItem *gender = new QListWidgetItem( ::getBaseStringTable()->getString( StringList::LISTGenders + k ));
                gender->setData( Qt::DecorationRole, QPixmap::fromImage( c.getImage( imIdx ) ) );
                gender->setData( Qt::UserRole, g );

                ddl->addItem( gender );
            }

            ic.close();
        }
    }
}

void ScreenAttribs::attributeDetail(bool checked)
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

void ScreenAttribs::info(bool checked)
{
    if (m_inspectMode && checked)
    {
        if (WLabel *w = qobject_cast<WLabel *>(sender()))
        {
            QString html;
            int     attrib = m_widgets.key( w ) - LBL_ATTRIBUTES_START;

            html = QString( "<p><font color=\"#916448\">%1:</font> %2" ).arg(::getBaseStringTable()->getString( StringList::Description + StringList::APPEND_COLON ))
                            .arg(::getBaseStringTable()->getString( StringList::DESCPrimaryAttributes + attrib ));

            new DialogInfo( StringList::LISTPrimaryAttributes + attrib, html, this );
        }
    }
}

void ScreenAttribs::mouseOverLabel(bool on)
{
    if (WLabel *q = qobject_cast<WLabel *>(this->sender()))
    {
        if (on)
        {
            int     lbl_id = m_widgets.key( q );

            if (m_inspectMode || (lbl_id < LBL_ATTRIBUTES_START) || (lbl_id > LBL_ATTRIBUTES_END))
            {
                q->setStyleSheet("QLabel {color: #169e16}"); // Green
            }
        }
        else
            q->setStyleSheet("QLabel {color: #e0e0c3}"); // Light yellow
    }
}

void ScreenAttribs::spinnerChanged(int value)
{
    if (WSpinBox *q = qobject_cast<WSpinBox *>(this->sender()))
    {
        int   attrib = m_widgets.key( q ) - VAL_ATTRIBUTES_START;

        m_char->setAttribute( static_cast<character::attribute>(attrib), value );
        if (WStatBar *clc = qobject_cast<WStatBar *>( m_widgets[ CLC_ATTRIBUTES + attrib ]))
        {
            clc->setValue( value, m_char->getAttribute( static_cast<character::attribute>(attrib), character::atIdx::Current ), 125 );
        }

        // Vitality affects HP
        // Strength, Piety and Vitality all affect Stamina
        // Piety affects Spell Points
        // Speed affects AC Mod
        // Vitality and Strength affect Carry Capacity
        m_char->recomputeEverything();

        struct { int id; int num; } vals[] =
        {
            { VAL_HP,        m_char->getHp(character::atIdx::Base)         },
            { VAL_STAMINA,   m_char->getStamina(character::atIdx::Base)    },
            { VAL_SP,        m_char->getMp(character::realm::REALM_SIZE, character::atIdx::Base) },
            { VAL_AC,        m_char->getAC_Average()                       },
            { VAL_CC,        (int)m_char->getLoad(character::atIdx::Base)  },
            { -1, -1 }
        };

        for (int k=0; vals[k].id != -1; k++)
        {
            if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ vals[k].id ]))
            {
                q->setNum( vals[k].num );
            }
        }
    }
}

void ScreenAttribs::resetScreen(void *char_tag, void *party_tag)
{
    (void)party_tag;

    m_char = (character *)char_tag;

    struct { int id; QString str; } vals[] =
    {
        { VAL_HP,        QString::number( m_char->getHp(character::atIdx::Base) )         },
        { VAL_STAMINA,   QString::number( m_char->getStamina(character::atIdx::Base) )    },
        { VAL_SP,        QString::number( m_char->getMp(character::realm::REALM_SIZE, character::atIdx::Base) ) },
        { VAL_AC,        QString::number( m_char->getAC_Average() )                       },
        { VAL_CC,        QString::number( (int)m_char->getLoad(character::atIdx::Base) )  },

        { VAL_MR_FIRE,   QString::asprintf( "%+d", m_char->getMagicResistance(character::realm::Fire) )         },
        { VAL_MR_WATER,  QString::asprintf( "%+d", m_char->getMagicResistance(character::realm::Water) )        },
        { VAL_MR_AIR,    QString::asprintf( "%+d", m_char->getMagicResistance(character::realm::Air) )          },
        { VAL_MR_EARTH,  QString::asprintf( "%+d", m_char->getMagicResistance(character::realm::Earth) )        },
        { VAL_MR_MENTAL, QString::asprintf( "%+d", m_char->getMagicResistance(character::realm::Mental) )       },
        { VAL_MR_DIVINE, QString::asprintf( "%+d", m_char->getMagicResistance(character::realm::Divine) )       },
        { -1, "" }
    };

    for (int k=0; vals[k].id != -1; k++)
    {
        if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ vals[k].id ]))
        {
            q->setText( vals[k].str );
        }
    }

    for (int k=0; k < character::attribute::ATTRIBUTE_SIZE; k++)
    {
        int base    = m_char->getAttribute( static_cast<character::attribute>(k), character::atIdx::Base    );
        int initial = m_char->getAttribute( static_cast<character::attribute>(k), character::atIdx::Initial );

        if (WSpinBox *q = qobject_cast<WSpinBox *>(m_widgets[ VAL_ATTRIBUTES + k ]))
        {
            q->setValueEx( base, initial );
        }
        if (base == 0)
        {
            // These ones won't have got the spinnerChanged() signal
            if (WStatBar *clc = qobject_cast<WStatBar *>( m_widgets[ CLC_ATTRIBUTES + k ]))
            {
                clc->setValue( base, m_char->getAttribute( static_cast<character::attribute>(k), character::atIdx::Current ), 125 );
            }
        }
    }

    if (WListWidget *abilities = qobject_cast<WListWidget *>(m_widgets[ SCROLL_ABILITIES ] ))
    {
        abilities->clear();
        abilities->addItems( m_char->getAbilityStrings() );
    }

    if (WListWidget *skills = qobject_cast<WListWidget *>(m_widgets[ SCROLL_SKILLS ] ))
    {
        skills->clear();
        QList<character::skill> profSkills = m_char->getProfessionalSkills();

        foreach (character::skill s, profSkills)
        {
            skills->addItem( ::getBaseStringTable()->getString( StringList::LISTSkills + static_cast<int>(s) ) );
        }
    }

    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_PROFS ] ))
    {
        character::profession p = m_char->getProfession();

        QMetaEnum metaProf = QMetaEnum::fromType<character::profession>();

        for (int k=0; k<metaProf.keyCount(); k++)
        {
            if (p == static_cast<character::profession>( metaProf.value(k) ))
            {
                ddl->setCurrentRow( k );
            }
        }
    }

    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_RACES ] ))
    {
        character::race r = m_char->getRace();

        QMetaEnum metaRace = QMetaEnum::fromType<character::race>();

        for (int k=0; k<metaRace.keyCount(); k++)
        {
            if (r == static_cast<character::race>( metaRace.value(k) ))
            {
                ddl->setCurrentRow( k );
            }
        }
    }
    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_GENDERS ] ))
    {
        character::gender g = m_char->getGender();

        QMetaEnum metaGender = QMetaEnum::fromType<character::gender>();

        for (int k=0; k<metaGender.keyCount(); k++)
        {
            if (g == static_cast<character::gender>( metaGender.value(k) ))
            {
                ddl->setCurrentRow( k );
            }
        }
    }

    updateLists();
}

void ScreenAttribs::setVisible( bool visible )
{
    QObjectList kids = children();

    for (int k=0; k<kids.size(); k++)
    {
        if (QWidget *q = qobject_cast<QWidget *>(kids.at(k)))
        {
            q->setVisible( visible );
        }
    }

    QWidget::setVisible( visible );
}

void ScreenAttribs::updateLists()
{
    if (WDDL *q = qobject_cast<WDDL *>(m_widgets[ DDL_PROFS ]))
    {
        q->updateList();
    }
    if (WDDL *q = qobject_cast<WDDL *>(m_widgets[ DDL_RACES ]))
    {
        q->updateList();
    }
    if (WDDL *q = qobject_cast<WDDL *>(m_widgets[ DDL_GENDERS ]))
    {
        q->updateList();
    }

    // Update the statue image based on the new choice of race and gender
    if (WImage *w = qobject_cast<WImage *>(m_widgets[ VAL_STATUE ]))
    {
        QPixmap statue = ScreenCommon::getStatue( m_char->getRace(), m_char->getGender() );

        QImage i = statue.toImage();

        uchar *imageBits = i.bits();
        int    imageLen  = i.sizeInBytes();

        for (uchar *p = imageBits; p < imageBits + imageLen; p += 4)
        {
            // If the blue, green and red colour components are all
            // within 3 numbers of each other, then set alpha to 0
            // This effectively turns all the blacks, whites and greys
            // transparent, which should obscure the black item boxes
            // some images have, and a fair portion of the arrow lines
            // but leave the peachy coloured character reasonably alone
            // I know the effect isn't perfect - any more aggressive and
            // we start losing bits of statue detail, though.

            if ((abs(p[0] - p[1]) < 3) &&
                (abs(p[0] - p[2]) < 3) &&
                (abs(p[1] - p[2]) < 3))
            {
                // This is (probably) a premultiplied pixel format, which
                // means colour components aren't allowed to be higher than
                // the alpha value, so set all components to zero
                p[0] = p[1] = p[2] = p[3] = 0;
            }
        }
        QImage mod_img = QImage( imageBits, i.width(), i.height(), i.bytesPerLine(), i.format() );

        statue = QPixmap::fromImage( mod_img );

        w->setPixmap( statue.scaledToHeight( 160, Qt::SmoothTransformation ) );
    }
}
