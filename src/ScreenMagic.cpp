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
#include <QListWidgetItem>

#include "common.h"
#include "ScreenMagic.h"
#include "DialogSpellInfo.h"

#include "StringList.h"
#include "WButton.h"
#include "WImage.h"
#include "WLabel.h"
#include "WListWidget.h"
#include "WScrollBar.h"
#include "WStatBar.h"
#include <QButtonGroup>
#include <QPushButton>
#include <QPixmap>

#include "SLFFile.h"

#include "spell.h"
#include "main.h"

#include <QDebug>

typedef enum
{
    NO_ID,

    MAGIC_INSPECT,

    GREEN_LABELS_START,
    SK_1 = GREEN_LABELS_START,
    SK_2,
    SK_3,
    SK_4,
    SK_5,
    SK_6,
    SP_1,
    SP_2,
    SP_3,
    SP_4,
    SP_5,
    SP_6,
    GREEN_LABELS_END,

    VAL_MR_FIRE,
    VAL_MR_WATER,
    VAL_MR_AIR,
    VAL_MR_EARTH,
    VAL_MR_MENTAL,
    VAL_MR_DIVINE,

    VAL_MR_FIRE_BAR,
    VAL_MR_WATER_BAR,
    VAL_MR_AIR_BAR,
    VAL_MR_EARTH_BAR,
    VAL_MR_MENTAL_BAR,
    VAL_MR_DIVINE_BAR,

    VAL_SK_FIRE,
    VAL_SK_WATER,
    VAL_SK_AIR,
    VAL_SK_EARTH,
    VAL_SK_MENTAL,
    VAL_SK_DIVINE,

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

    SCROLLS_START,
    SCROLL_FIRE = SCROLLS_START,
    SCROLLLIST_FIRE,
    SCROLL_WATER,
    SCROLLLIST_WATER,
    SCROLL_AIR,
    SCROLLLIST_AIR,
    SCROLL_EARTH,
    SCROLLLIST_EARTH,
    SCROLL_MENTAL,
    SCROLLLIST_MENTAL,
    SCROLL_DIVINE,
    SCROLLLIST_DIVINE,
    SCROLLS_END,

    SIZE_WIDGET_IDS
} widget_ids;

ScreenMagic::ScreenMagic(character *c, QWidget *parent) :
    Screen(parent),
    m_char(c),
    m_inspectMode(false)
{
    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0, 450,  -1,  -1 ),    new WImage(    "REVIEW/BOTTOMBUTTONBAR.STI",                   0,              this ),  -1,                             NULL },

        { MAGIC_INSPECT,      QRect(   0, 450,  -1,  -1 ),    new WButton(   "REVIEW/REVIEWITEMBUTTONS.STI",                 0, false, 1.0,  this ),  StringList::InspectSpell,     SLOT(spellDetail(bool)) },

        { NO_ID,              QRect( 310,   0,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWMAGICPAGE.STI",                   0,              this ),  -1,  NULL },
        { NO_ID,              QRect(   1, 165,  -1,  -1 ),    new WImage(    "REVIEW/REVIEWMAGICPAGE.STI",                   2,              this ),  -1,  NULL },

        { NO_ID,              QRect( 324,  29, 307,  14 ),    new WLabel(    StringList::RESISTANCES,      Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },

        { NO_ID,              QRect( 325,  54,  -1,  -1 ),    new WImage(    "SPELL CASTING/FIRE_REALM.STI",                 0,              this ),  -1,  NULL },
        { VAL_MR_FIRE_BAR,    QRect( 347,  55, 100,  14 ),    new WStatBar(                                                                  this ),  -1,  NULL },
        { VAL_MR_FIRE,        QRect( 450,  56,  21,  14 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 325,  82,  -1,  -1 ),    new WImage(    "SPELL CASTING/AIR_REALM.STI",                  0,              this ),  -1,  NULL },
        { VAL_MR_AIR_BAR,     QRect( 347,  83, 100,  14 ),    new WStatBar(                                                                  this ),  -1,  NULL },
        { VAL_MR_AIR,         QRect( 450,  84,  21,  14 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 325, 110,  -1,  -1 ),    new WImage(    "SPELL CASTING/MENTAL_REALM.STI",               0,              this ),  -1,  NULL },
        { VAL_MR_MENTAL_BAR,  QRect( 347, 111, 100,  14 ),    new WStatBar(                                                                  this ),  -1,  NULL },
        { VAL_MR_MENTAL,      QRect( 450, 112,  21,  14 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 480,  54,  -1,  -1 ),    new WImage(    "SPELL CASTING/WATER_REALM.STI",                8,              this ),  -1,  NULL },
        { VAL_MR_WATER_BAR,   QRect( 503,  55, 100,  14 ),    new WStatBar(                                                                  this ),  -1,  NULL },
        { VAL_MR_WATER,       QRect( 605,  56,  21,  14 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 480,  82,  -1,  -1 ),    new WImage(    "SPELL CASTING/EARTH_REALM.STI",                0,              this ),  -1,  NULL },
        { VAL_MR_EARTH_BAR,   QRect( 503,  83, 100,  14 ),    new WStatBar(                                                                  this ),  -1,  NULL },
        { VAL_MR_EARTH,       QRect( 605,  84,  21,  14 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 480, 110,  -1,  -1 ),    new WImage(    "SPELL CASTING/DIVINE_REALM.STI",              11,              this ),  -1,  NULL },
        { VAL_MR_DIVINE_BAR,  QRect( 503, 111, 100,  14 ),    new WStatBar(                                                                  this ),  -1,  NULL },
        { VAL_MR_DIVINE,      QRect( 605, 112,  21,  14 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },

        { NO_ID,              QRect(   7, 173,  -1,  -1 ),    new WImage(    "SPELL CASTING/FIRE_REALM.STI",                 0,              this ),  -1,  NULL },
        { SK_1,               QRect(  32, 178,  30,  16 ),    new WLabel(    StringList::Skill,            Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL }, // needs a ":" appended
        { VAL_SK_FIRE,        QRect(  68, 178,  29,  16 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { SP_1,               QRect( 128, 178,  20,  16 ),    new WLabel(    StringList::SP,               Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_FIRE1,          QRect( 161, 178,  29,  16 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { VAL_FIRE2,          QRect( 190, 178,  29,  16 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { SCROLLLIST_FIRE,    QRect(  28, 193, 156, 104 ),    new WListWidget(                                                               this ),  -1,  NULL },
        { SCROLL_FIRE,        QRect( 188, 196,  15, 100 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },

        { NO_ID,              QRect( 220, 173,  -1,  -1 ),    new WImage(    "SPELL CASTING/WATER_REALM.STI",                8,              this ),  -1,  NULL },
        { SK_2,               QRect( 245, 178,  30,  16 ),    new WLabel(    StringList::Skill,            Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL }, // needs a ":" appended
        { VAL_SK_WATER,       QRect( 281, 178,  29,  16 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { SP_2,               QRect( 341, 178,  20,  16 ),    new WLabel(    StringList::SP,               Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_WATER1,         QRect( 374, 178,  29,  16 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { VAL_WATER2,         QRect( 403, 178,  29,  16 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { SCROLLLIST_WATER,   QRect( 241, 193, 156, 104 ),    new WListWidget(                                                               this ),  -1,  NULL },
        { SCROLL_WATER,       QRect( 401, 196,  15, 100 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },

        { NO_ID,              QRect( 433, 173,  -1,  -1 ),    new WImage(    "SPELL CASTING/AIR_REALM.STI",                  0,              this ),  -1,  NULL },
        { SK_3,               QRect( 458, 178,  30,  16 ),    new WLabel(    StringList::Skill,            Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL }, // needs a ":" appended
        { VAL_SK_AIR,         QRect( 494, 178,  29,  16 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { SP_3,               QRect( 554, 178,  20,  16 ),    new WLabel(    StringList::SP,               Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_AIR1,           QRect( 587, 178,  29,  16 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { VAL_AIR2,           QRect( 616, 178,  29,  16 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { SCROLLLIST_AIR,     QRect( 454, 193, 156, 104 ),    new WListWidget(                                                               this ),  -1,  NULL },
        { SCROLL_AIR,         QRect( 614, 196,  15, 100 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },

        { NO_ID,              QRect(   7, 313,  -1,  -1 ),    new WImage(    "SPELL CASTING/EARTH_REALM.STI",                0,              this ),  -1,  NULL },
        { SK_4,               QRect(  32, 318,  30,  16 ),    new WLabel(    StringList::Skill,            Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL }, // needs a ":" appended
        { VAL_SK_EARTH,       QRect(  68, 318,  29,  16 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { SP_4,               QRect( 128, 318,  20,  16 ),    new WLabel(    StringList::SP,               Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_EARTH1,         QRect( 161, 318,  29,  16 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { VAL_EARTH2,         QRect( 190, 318,  29,  16 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { SCROLLLIST_EARTH,   QRect(  28, 331, 156, 104 ),    new WListWidget(                                                               this ),  -1,  NULL },
        { SCROLL_EARTH,       QRect( 188, 334,  15, 100 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },

        { NO_ID,              QRect( 220, 313,  -1,  -1 ),    new WImage(    "SPELL CASTING/MENTAL_REALM.STI",               0,              this ),  -1,  NULL },
        { SK_5,               QRect( 245, 318,  30,  16 ),    new WLabel(    StringList::Skill,            Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL }, // needs a ":" appended
        { VAL_SK_MENTAL,      QRect( 281, 318,  29,  16 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { SP_5,               QRect( 341, 318,  20,  16 ),    new WLabel(    StringList::SP,               Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_MENTAL1,        QRect( 374, 318,  29,  16 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { VAL_MENTAL2,        QRect( 403, 318,  29,  16 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { SCROLLLIST_MENTAL,  QRect( 241, 331, 156, 104 ),    new WListWidget(                                                               this ),  -1,  NULL },
        { SCROLL_MENTAL,      QRect( 401, 334,  15, 100 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },

        { NO_ID,              QRect( 433, 313,  -1,  -1 ),    new WImage(    "SPELL CASTING/DIVINE_REALM.STI",              11,              this ),  -1,  NULL },
        { SK_6,               QRect( 458, 318,  30,  16 ),    new WLabel(    StringList::Skill,            Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL }, // needs a ":" appended
        { VAL_SK_DIVINE,      QRect( 494, 318,  29,  16 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { SP_6,               QRect( 554, 318,  20,  16 ),    new WLabel(    StringList::SP,               Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_DIVINE1,        QRect( 587, 318,  29,  16 ),    new WLabel(    "",                           Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { VAL_DIVINE2,        QRect( 616, 318,  29,  16 ),    new WLabel(    "",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { SCROLLLIST_DIVINE,  QRect( 454, 331, 156, 104 ),    new WListWidget(                                                               this ),  -1,  NULL },
        { SCROLL_DIVINE,      QRect( 614, 334,  15, 100 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    // Connect the scrollbars to their corresponding scrollable lists
    for (int k=SCROLLS_START; k < SCROLLS_END; k += 2)
    {
        int list_widget_idx = k + 1;
        int sbar_widget_idx = k;

        if (WListWidget *items = qobject_cast<WListWidget *>(m_widgets[ list_widget_idx ] ))
        {
            if (WScrollBar *sb = qobject_cast<WScrollBar *>(m_widgets[ sbar_widget_idx ] ))
            {
                // QScrollAreas include their own scrollbar when necessary, but we need to
                // override this since the Wizardry 8 look has the scrollbar outside the
                // scroll area and a short space away to the right

                items->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
                items->setVerticalScrollBar( sb );
                // The call to setVerticalScrollBar reparents it, which prevents us drawing
                // it where we want, so reparent it (again) back to us
                sb->setParent( this );
                // return it to the same widget stack postion it was
                sb->stackUnder( m_widgets[ list_widget_idx ] );
            }
            // the currentItemChanged() signal isn't as useful for the specific requirements
            // of our altered listview here as the basic clicked signal is
            connect( items, SIGNAL(itemClicked(QListWidgetItem *)),
                      this, SLOT(itemClicked(QListWidgetItem *)) );
        }
    }

    createList( SCROLLLIST_FIRE,   character::realm::Fire );
    createList( SCROLLLIST_WATER,  character::realm::Water );
    createList( SCROLLLIST_AIR,    character::realm::Air );
    createList( SCROLLLIST_EARTH,  character::realm::Earth );
    createList( SCROLLLIST_MENTAL, character::realm::Mental );
    createList( SCROLLLIST_DIVINE, character::realm::Divine );

    resetScreen( m_char, NULL );
}

ScreenMagic::~ScreenMagic()
{
    // Child widgets are automatically destroyed
}

void ScreenMagic::spellDetail(bool checked)
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

void ScreenMagic::itemClicked(QListWidgetItem *listItem)
{
    int spell_id = listItem->data( Qt::UserRole ).toInt();

    if (m_inspectMode)
    {
        new DialogSpellInfo(spell(spell_id), this);
    }
    else
    {
        if (listItem->checkState() == Qt::Unchecked)
        {
            m_char->setSpellKnown( spell_id );
            listItem->setCheckState(Qt::Checked);
        }
        else
        {
            m_char->setSpellKnown( spell_id, false );
            listItem->setCheckState(Qt::Unchecked);
        }
    }
}

void ScreenMagic::updateList( int list_widget_idx )
{
    if (WListWidget *list = qobject_cast<WListWidget *>(m_widgets[ list_widget_idx ] ))
    {
        for (int k=list->count()-1; k>=0; k--)
        {
            QListWidgetItem *spell_widget = list->item(k);
            int              spell_id     = spell_widget->data( Qt::UserRole ).toInt();

            if (m_char->isSpellKnown( spell_id ))
                spell_widget->setCheckState(Qt::Checked);
            else
                spell_widget->setCheckState(Qt::Unchecked);
        }
    }
}

void ScreenMagic::createList( int list_widget_idx, character::realm realm )
{
    if (WListWidget *list = qobject_cast<WListWidget *>(m_widgets[ list_widget_idx ] ))
    {
        QColor lt_yellow   (0xe0, 0xe0, 0xc3); // main text colour
        QColor gr_yellow   (0x70, 0x70, 0x43); // disabled text colour

        list->setTextColorInsteadOfCheckmarks( true, lt_yellow, gr_yellow );

        for (int k = 1; k < MAXIMUM_CHARACTER_SPELLS; k++)
        {
            spell s(k);

            // Apparently 0 cost spells are something some mods expect to be able to do, so swap to
            // hiding spells which have a 0 level instead, and hope no-one expects to do that also.
            if (! s.isNull() && (s.getRealm() == realm) &&
                (s.getLevel() > 0))
            {
                QListWidgetItem *newItem = new QListWidgetItem( s.getName() + "\t" + QString::number( s.getSPCost() ) );

                // The Wizardry style will align everything after the TAB to the RHS
                // and everything before it to the LHS if we set the alignment to Right
                newItem->setTextAlignment( Qt::AlignRight );
                newItem->setData( Qt::UserRole, k );

                if (m_char->isSpellKnown( k ))
                    newItem->setCheckState(Qt::Checked);
                else
                    newItem->setCheckState(Qt::Unchecked);

                list->addItem( newItem );
            }
        }
    }
}

void ScreenMagic::resetScreen(void *char_tag, void *party_tag)
{
    (void)party_tag;

    m_char = (character *)char_tag;

    // Populate text value fields with values from the character

    struct { int id; QString str; } vals[] =
    {
        { VAL_MR_FIRE,  QString::number( m_char->getMagicResistance( character::realm::Fire,   character::atIdx::Current) ) },
        { VAL_MR_WATER, QString::number( m_char->getMagicResistance( character::realm::Water,  character::atIdx::Current) ) },
        { VAL_MR_AIR,   QString::number( m_char->getMagicResistance( character::realm::Air,    character::atIdx::Current) ) },
        { VAL_MR_EARTH, QString::number( m_char->getMagicResistance( character::realm::Earth,  character::atIdx::Current) ) },
        { VAL_MR_MENTAL,QString::number( m_char->getMagicResistance( character::realm::Mental, character::atIdx::Current) ) },
        { VAL_MR_DIVINE,QString::number( m_char->getMagicResistance( character::realm::Divine, character::atIdx::Current) ) },

        { VAL_SK_FIRE,  QString::number( m_char->getSkill( character::skill::FireMagic,   character::atIdx::Current) ) },
        { VAL_SK_WATER, QString::number( m_char->getSkill( character::skill::WaterMagic,  character::atIdx::Current) ) },
        { VAL_SK_AIR,   QString::number( m_char->getSkill( character::skill::AirMagic,    character::atIdx::Current) ) },
        { VAL_SK_EARTH, QString::number( m_char->getSkill( character::skill::EarthMagic,  character::atIdx::Current) ) },
        { VAL_SK_MENTAL,QString::number( m_char->getSkill( character::skill::MentalMagic, character::atIdx::Current) ) },
        { VAL_SK_DIVINE,QString::number( m_char->getSkill( character::skill::DivineMagic, character::atIdx::Current) ) },

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

        { -1, "" }
    };

    for (int k=0; vals[k].id != -1; k++)
    {
        if (QLabel *q = qobject_cast<QLabel *>(m_widgets[ vals[k].id ]))
        {
            q->setText( vals[k].str );
        }
    }

    // Make the green labels green
    for (int k = GREEN_LABELS_START; k < GREEN_LABELS_END; k++)
    {
        WLabel *q = qobject_cast<WLabel *>(m_widgets[ k ]);

        q->setStyleSheet("QLabel {color: #169e16}"); // Green
    }

    // Setup all the bars
    for (int k=0; k<character::REALM_SIZE; k++)
    {
        int base    =  m_char->getMagicResistance( (character::realm)(character::realm::Fire + k), character::atIdx::Base   );
        int current =  m_char->getMagicResistance( (character::realm)(character::realm::Fire + k), character::atIdx::Current);

        if (WStatBar *w = qobject_cast<WStatBar *>(m_widgets[ VAL_MR_FIRE_BAR + k ]))
        {
            w->setValue( base, current, 100 );
        }
    }

    updateList( SCROLLLIST_FIRE   );
    updateList( SCROLLLIST_WATER  );
    updateList( SCROLLLIST_AIR    );
    updateList( SCROLLLIST_EARTH  );
    updateList( SCROLLLIST_MENTAL );
    updateList( SCROLLLIST_DIVINE );
}
