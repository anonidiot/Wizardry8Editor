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

#include "DialogItemInfo.h"
#include "SLFFile.h"
#include "STItoQImage.h"
#include "main.h"

#include <QApplication>
#include <QButtonGroup>
#include <QTextEdit>

#include "WButton.h"
#include "WImage.h"
#include "WItem.h"
#include "WLabel.h"
#include "WScrollBar.h"

#include "spell.h"

#include <QDebug>

typedef enum
{
    NO_ID,

    I_BG,
    I_PROFBAR,
    I_RACEBAR,

    VAL_WEIGHT,

    TB_DESC,
    DESC_SCROLLBAR,

    SHOW_PROFS,
    SHOW_RACES,

    PROFS_START,
    I_FIGHTER = PROFS_START,
    I_LORD,
    I_VALKYRIE,
    I_RANGER,
    I_SAMURAI,
    I_NINJA,
    I_MONK,
    I_ROGUE,
    I_GADGETEER,
    I_BARD,
    I_PRIEST,
    I_ALCHEMIST,
    I_BISHOP,
    I_PSIONIC,
    I_MAGE,
    PROFS_END,

    D_PROFS_START,
    D_FIGHTER = D_PROFS_START,
    D_LORD,
    D_VALKYRIE,
    D_RANGER,
    D_SAMURAI,
    D_NINJA,
    D_MONK,
    D_ROGUE,
    D_GADGETEER,
    D_BARD,
    D_PRIEST,
    D_ALCHEMIST,
    D_BISHOP,
    D_PSIONIC,
    D_MAGE,
    D_PROFS_END,

    RACES_START,
    I_HUMAN = RACES_START,
    I_ELF,
    I_DWARF,
    I_GNOME,
    I_HOBBIT,
    I_FAERIE,
    I_LIZARDMAN,
    I_DRACON,
    I_FELPURR,
    I_RAWULF,
    I_MOOK,
    I_TRYNNIE,
    I_TRANG,
    I_UMPANI,
    I_RAPAX,
    I_ANDROID,
    RACES_END,

    D_RACES_START,
    D_HUMAN = D_RACES_START,
    D_ELF,
    D_DWARF,
    D_GNOME,
    D_HOBBIT,
    D_FAERIE,
    D_LIZARDMAN,
    D_DRACON,
    D_FELPURR,
    D_RAWULF,
    D_MOOK,
    D_TRYNNIE,
    D_TRANG,
    D_UMPANI,
    D_RAPAX,
    D_ANDROID,
    D_RACES_END,

    SIZE_WIDGET_IDS
} widget_ids;

DialogItemInfo::DialogItemInfo(const item &i, QWidget *parent)
    : Dialog(parent),
    m_item(i)
{
    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { I_BG,               QRect(   0,   0,  -1,  -1 ),    new WImage(    "DIALOGS/POPUP_ITEMINFO.STI",                   0,              this ),  -1,  NULL },
        { I_PROFBAR,          QRect(  11,  11,  -1,  -1 ),    new WImage(    "DIALOGS/POPUP_ITEMINFO.STI",                   1,              this ),  -1,  NULL },
        { I_RACEBAR,          QRect(  11,  11,  -1,  -1 ),    new WImage(    "DIALOGS/POPUP_ITEMINFO.STI",                   2,              this ),  -1,  NULL },

        { TB_DESC,            QRect(  68,  72, 272, 166 ),    new QTextEdit(                                                                 this ),  -1,  NULL },
        { DESC_SCROLLBAR,     QRect( 354,  73,  15, 165 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },

        { SHOW_PROFS,         QRect(  12,  13,  -1,  -1 ),    new WButton(   "DIALOGS/ITEMINFO_TABBUTTON.STI",               0, false, 1.0,  this ),  -1,  SLOT(profButton(bool)) },
        { SHOW_RACES,         QRect(  39,  13,  -1,  -1 ),    new WButton(   "DIALOGS/ITEMINFO_TABBUTTON.STI",               5, false, 1.0,  this ),  -1,  SLOT(raceButton(bool)) },
        { NO_ID,              QRect( 344, 248,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               0, true,  1.0,  this ),  -1,  SLOT(close()) },

        { I_FIGHTER,          QRect(  12,  42,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                10,              this ),  -1,  NULL },
        { D_FIGHTER,          QRect(  12,  42,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                11,              this ),  -1,  NULL },
        { I_LORD,             QRect(  12,  71,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                24,              this ),  -1,  NULL },
        { D_LORD,             QRect(  12,  71,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                25,              this ),  -1,  NULL },
        { I_VALKYRIE,         QRect(  12, 100,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                 8,              this ),  -1,  NULL },
        { D_VALKYRIE,         QRect(  12, 100,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                 9,              this ),  -1,  NULL },
        { I_RANGER,           QRect(  12, 129,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                 2,              this ),  -1,  NULL },
        { D_RANGER,           QRect(  12, 129,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                 3,              this ),  -1,  NULL },
        { I_SAMURAI,          QRect(  12, 158,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                16,              this ),  -1,  NULL },
        { D_SAMURAI,          QRect(  12, 158,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                17,              this ),  -1,  NULL },
        { I_NINJA,            QRect(  12, 187,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                20,              this ),  -1,  NULL },
        { D_NINJA,            QRect(  12, 187,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                21,              this ),  -1,  NULL },
        { I_MONK,             QRect(  12, 216,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                12,              this ),  -1,  NULL },
        { D_MONK,             QRect(  12, 216,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                13,              this ),  -1,  NULL },
        { I_ROGUE,            QRect(  39,  42,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                28,              this ),  -1,  NULL },
        { D_ROGUE,            QRect(  39,  42,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                29,              this ),  -1,  NULL },
        { I_GADGETEER,        QRect(  39, 100,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                 6,              this ),  -1,  NULL },
        { D_GADGETEER,        QRect(  39, 100,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                 7,              this ),  -1,  NULL },
        { I_BARD,             QRect(  39,  71,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                 0,              this ),  -1,  NULL },
        { D_BARD,             QRect(  39,  71,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                 1,              this ),  -1,  NULL },
        { I_PRIEST,           QRect(  39, 129,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                18,              this ),  -1,  NULL },
        { D_PRIEST,           QRect(  39, 129,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                19,              this ),  -1,  NULL },
        { I_ALCHEMIST,        QRect(  39, 245,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                22,              this ),  -1,  NULL },
        { D_ALCHEMIST,        QRect(  39, 245,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                23,              this ),  -1,  NULL },
        { I_BISHOP,           QRect(  39, 158,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                 4,              this ),  -1,  NULL },
        { D_BISHOP,           QRect(  39, 158,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                 5,              this ),  -1,  NULL },
        { I_PSIONIC,          QRect(  39, 216,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                14,              this ),  -1,  NULL },
        { D_PSIONIC,          QRect(  39, 216,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                15,              this ),  -1,  NULL },
        { I_MAGE,             QRect(  39, 187,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                26,              this ),  -1,  NULL },
        { D_MAGE,             QRect(  39, 187,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_PROFESSION.STI",                27,              this ),  -1,  NULL },

        { I_HUMAN,            QRect(  12,  42,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                       8,              this ),  -1,  NULL },
        { D_HUMAN,            QRect(  12,  42,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                       9,              this ),  -1,  NULL },
        { I_ELF,              QRect(  39,  42,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                       4,              this ),  -1,  NULL },
        { D_ELF,              QRect(  39,  42,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                       5,              this ),  -1,  NULL },
        { I_DWARF,            QRect(  12,  71,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      12,              this ),  -1,  NULL },
        { D_DWARF,            QRect(  12,  71,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      13,              this ),  -1,  NULL },
        { I_GNOME,            QRect(  39,  71,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      16,              this ),  -1,  NULL },
        { D_GNOME,            QRect(  39,  71,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      17,              this ),  -1,  NULL },
        { I_HOBBIT,           QRect(  12, 100,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      14,              this ),  -1,  NULL },
        { D_HOBBIT,           QRect(  12, 100,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      15,              this ),  -1,  NULL },
        { I_FAERIE,           QRect(  39, 100,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      20,              this ),  -1,  NULL },
        { D_FAERIE,           QRect(  39, 100,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      21,              this ),  -1,  NULL },
        { I_LIZARDMAN,        QRect(  12, 158,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                       0,              this ),  -1,  NULL },
        { D_LIZARDMAN,        QRect(  12, 158,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                       1,              this ),  -1,  NULL },
        { I_DRACON,           QRect(  39, 158,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      10,              this ),  -1,  NULL },
        { D_DRACON,           QRect(  39, 158,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      11,              this ),  -1,  NULL },
        { I_FELPURR,          QRect(  12, 129,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                       2,              this ),  -1,  NULL },
        { D_FELPURR,          QRect(  12, 129,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                       3,              this ),  -1,  NULL },
        { I_RAWULF,           QRect(  39, 129,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      18,              this ),  -1,  NULL },
        { D_RAWULF,           QRect(  39, 129,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      19,              this ),  -1,  NULL },
        { I_MOOK,             QRect(  12, 187,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                       6,              this ),  -1,  NULL },
        { D_MOOK,             QRect(  12, 187,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                       7,              this ),  -1,  NULL },
        { I_TRYNNIE,          QRect(  39, 245,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      30,              this ),  -1,  NULL },
        { D_TRYNNIE,          QRect(  39, 245,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      31,              this ),  -1,  NULL },
        { I_TRANG,            QRect(  12, 245,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      24,              this ),  -1,  NULL },
        { D_TRANG,            QRect(  12, 245,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      25,              this ),  -1,  NULL },
        { I_UMPANI,           QRect(  12, 216,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      22,              this ),  -1,  NULL },
        { D_UMPANI,           QRect(  12, 216,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      23,              this ),  -1,  NULL },
        { I_RAPAX,            QRect(  39, 216,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      26,              this ),  -1,  NULL },
        { D_RAPAX,            QRect(  39, 216,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      27,              this ),  -1,  NULL },
        { I_ANDROID,          QRect(  39, 187,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      28,              this ),  -1,  NULL },
        { D_ANDROID,          QRect(  39, 187,  -1,  -1 ),    new WImage(    "DIALOGS/ICONS_RACE.STI",                      29,              this ),  -1,  NULL },

        { NO_ID,              QRect(  68,  12,  46,  55 ),    new WItem(     i, WItem::context_mode::None,                                   this ),  -1,  NULL },
        { NO_ID,              QRect( 120,  15, 225,  12 ),    new WLabel(    i.getName(),                               Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 120,  33,  40,  12 ),    new WLabel( StringList::Type + StringList::APPEND_COLON,  Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 160,  33, 185,  12 ),    new WLabel(    i.getTypeString(),                         Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 120,  51,  40,  12 ),    new WLabel( StringList::Weight + StringList::APPEND_COLON, Qt::AlignRight,  10, QFont::Thin, this ),  -1,  NULL },
        { VAL_WEIGHT,         QRect( 160,  51, 185,  12 ),    new WLabel(    "",                                        Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    // Races and Profs toggle one another off
    m_showGroup = new QButtonGroup(this);
    for (int k=SHOW_PROFS; k < SHOW_RACES; k++)
    {
        m_showGroup->addButton( qobject_cast<QPushButton *>(m_widgets[ k ]), k - SHOW_PROFS );
    }
    qobject_cast<QPushButton *>(m_widgets[ SHOW_PROFS ])->setChecked( true );
    profButton(true);

    // weight
    if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ VAL_WEIGHT ]))
    {
        if (i.isStackable())
        {
            q->setText( QString( tr("%1 pounds (%2 %3)") ).arg( m_item.getWeight() * m_item.getCount(), 0, 'f', 1 )
                                                    .arg( m_item.getWeight(), 0, 'f', 1 )
                                                    .arg(::getStringTable()->getString( StringList::Each )) );
        }
        else
        {
            q->setText( QString( tr("%1 pounds") ).arg( m_item.getWeight(), 0, 'f', 1 ) );
        }
    }

    // The textbox
    if (QTextEdit *tb = qobject_cast<QTextEdit *>(m_widgets[ TB_DESC ] ))
    {
        tb->setFont(QFont("Wizardry", 10 * m_scale, QFont::Thin));
        tb->setReadOnly(true);

        tb->setHtml( htmlGenerateItemProps() );

        if (WScrollBar *sb = qobject_cast<WScrollBar *>(m_widgets[ DESC_SCROLLBAR ] ))
        {
            // QTextEdits include their own scrollbar when necessary, but we need to
            // override this since the Wizardry 8 look has the scrollbar outside the
            // textedit area and a short space away to the right
            tb->setVerticalScrollBar( sb );
            // The call to setVerticalScrollBar reparents it, which prevents us drawing
            // it where we want, so reparent it (again) back to us
            sb->setParent( this );
        }
    }

    if (WImage *w = qobject_cast<WImage *>(m_widgets[ I_BG ] ))
    {
        QSize bgImgSize = w->getPixmapSize();

        this->setMinimumSize( bgImgSize * m_scale );
        this->setMaximumSize( bgImgSize * m_scale );

        // Something in the OK button displeased the dialog layout, and made a
        // bigger than minimum dialog. We have to make an additional call to force
        // it back to the right size after adding the OK button.
        this->resize( bgImgSize * m_scale );
    }

    show();
}

QString DialogItemInfo::htmlGenerateItemProps()
{
    QString html = "";

    // Damage, To Hit and Initiative
    {
        quint16 min_damage, max_damage;
        int percentage;

        m_item.getDamage( &min_damage, &max_damage, &percentage );
        if (percentage != 0)
            html += QString( "<p><font color=\"#916448\">%1</font> +%2%" ).arg(::getStringTable()->getString( StringList::Damage + StringList::APPEND_COLON )).arg(percentage);
        else if (max_damage != 0)
            html += QString( "<p><font color=\"#916448\">%1</font> %2 - %3" ).arg(::getStringTable()->getString( StringList::Damage + StringList::APPEND_COLON )).arg(min_damage).arg(max_damage);

        if (m_item.getToHit() > 0)
            html += QString( "<p><font color=\"#916448\">%1</font> +%2" ).arg(::getStringTable()->getString( StringList::ToHit + StringList::APPEND_COLON )).arg( m_item.getToHit() );
        else if (m_item.getToHit() < 0)
            html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::ToHit + StringList::APPEND_COLON )).arg( m_item.getToHit() );

        if (m_item.getInitiative() > 0)
            html += QString( "<p><font color=\"#916448\">%1</font> +%2" ).arg(::getStringTable()->getString( StringList::Initiative + StringList::APPEND_COLON )).arg( m_item.getInitiative() );
        else if (m_item.getInitiative() < 0)
            html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::Initiative + StringList::APPEND_COLON )).arg( m_item.getInitiative() );
    }

    // Equippable Slots and Cursed Status
    {
        QString equip = "";
        bool    showCursed = true;

        switch (m_item.getType())
        {
            case item::type::ShortWeapon:
            case item::type::ExtendedWeapon:
            case item::type::ThrownWeapon:
            case item::type::RangedWeapon:
                equip = ::getStringTable()->getString( StringList::PrimaryWeapon );
                if (m_item.canSecondary())
                {
                    equip += ", " + ::getStringTable()->getString( StringList::SecondaryWeapon );
                }
                break;

            case item::type::Ammunition:
                // should have secondary set - but don't
                equip = ::getStringTable()->getString( StringList::SecondaryWeapon );
                showCursed = false;
                break;

            case item::type::Shield:
                // should have secondary set - but don't
                equip = ::getStringTable()->getString( StringList::SecondaryWeapon );
                break;

            case item::type::TorsoArmor:
                equip = ::getStringTable()->getString( StringList::Torso );
                break;

            case item::type::LegArmor:
                equip = ::getStringTable()->getString( StringList::Legs );
                break;

            case item::type::HeadGear:
                equip = ::getStringTable()->getString( StringList::Head );
                break;

            case item::type::Gloves:
                equip = ::getStringTable()->getString( StringList::Hands );
                break;

            case item::type::Shoes:
                equip = ::getStringTable()->getString( StringList::Feet );
                break;

            case item::type::MiscEquipment:
                equip = ::getStringTable()->getString( StringList::MiscItem1 ) + ", " + ::getStringTable()->getString( StringList::MiscItem2 );
                break;

            case item::type::Cloak:
                equip = ::getStringTable()->getString( StringList::Cloak );
                break;

            case item::type::Instrument:
            case item::type::Gadget:
            case item::type::MiscMagic:
            case item::type::Potion:
            case item::type::Bomb:
            case item::type::Powder:
            case item::type::Spellbook:
            case item::type::Scroll:
            case item::type::Food:
            case item::type::Drink:
            case item::type::Key:
            case item::type::Writing:
            case item::type::Other:
                // Non-equipable
                showCursed = false;
                break;

        }
        if (equip.size() > 0)
        {
            html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::EquippableSlots + StringList::APPEND_COLON )).arg( equip );
        }

        if (showCursed)
        {
            if (! m_item.isCursed() )
            {
                html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::CursedStatus + StringList::APPEND_COLON )).arg(::getStringTable()->getString( StringList::NotCursed ) );
            }
            else if (m_item.isUncursed() || !m_item.isEquipped())
            {
                html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::CursedStatus + StringList::APPEND_COLON )).arg(::getStringTable()->getString( StringList::UncursedRemovable ));
            }
            else
            {
                html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::CursedStatus + StringList::APPEND_COLON )).arg( "<font color=\"#ec000f\">" + ::getStringTable()->getString( StringList::CursedUnremovable ) + "</font>" );
            }
        }
    }


    if (m_item.needs2Hands())
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::SpecialAttributes + StringList::APPEND_COLON )).arg(::getStringTable()->getString( StringList::TwoHandedWeapon ));

    QString special_attack = m_item.getSpecialAttackString();
    if (special_attack.size() > 0)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::SpecialAttack + StringList::APPEND_COLON )).arg(special_attack);

    QString double_damage = m_item.getSlaysString();
    if (double_damage.size() > 0)
        html += QString( "<p><font color=\"#916448\">%1l</font> %2" ).arg(::getStringTable()->getString( StringList::DoubleDamageV + StringList::APPEND_COLON )).arg(double_damage);

    if (m_item.getAC() > 0)
        html += QString( "<p><font color=\"#916448\">%1</font> +%2" ).arg(::getStringTable()->getString( StringList::AC + StringList::APPEND_COLON )).arg( m_item.getAC() );
    else if (m_item.getAC() < 0)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::AC + StringList::APPEND_COLON )).arg( m_item.getAC() );

    QString armor_weight = m_item.getArmorWeightClassString();
    if (armor_weight.size() > 0)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::ArmorWeightClass + StringList::APPEND_COLON )).arg(armor_weight);

    // _EVERY_ item has an attack range on it: Short, Extended, Thrown or Long, but
    // for most of them it doesn't make sense to have it. They all get lumped in the
    // Short category. So if We have a Short range also check that we're actually a
    // ShortWeapon before showing it.
    if ((m_item.getRange() != item::range::Short) || (m_item.getType() == item::type::ShortWeapon))
    {
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::AttackRange + StringList::APPEND_COLON )).arg(m_item.getRangeString());
    }

    QString skill = m_item.getSkillUsedString();
    if (skill.size() > 0)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::Skills + StringList::APPEND_COLON )).arg(skill);

    QString attacks = m_item.getAttacksString();
    if (attacks.size() > 0)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::AttackModes + StringList::APPEND_COLON )).arg(attacks);

#define EXTRA_FIELDS
#ifdef EXTRA_FIELDS
    int swings = m_item.getBonusSwings();
    if (swings > 0)
        html += QString( "<p><font color=\"#916448\">%1</font> +%2" ).arg(::getStringTable()->getString( StringList::BonusSwings + StringList::APPEND_COLON )).arg( swings);
    else if (swings < 0)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::SwingPenalty + StringList::APPEND_COLON )).arg( swings);
#endif

    // Spell and Spell level or power
    {
        int power;
        spell s = m_item.getSpell(&power);
        QString spell_name = s.getName();
        if (spell_name.size() > 0)
        {
            if (m_item.getType() == item::type::Spellbook)
            {
                // Don't show a power level because spellbooks don't actually "cast" the spell
                html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::Spell + StringList::APPEND_COLON )).arg(spell_name);

                html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::SpellCasterLevel + StringList::APPEND_COLON )).arg( s.getLevel() );
            }
            else
            {
                html += QString( "<p><font color=\"#916448\">%1</font> %2 (Pwr %3)" ).arg(::getStringTable()->getString( StringList::Spell + StringList::APPEND_COLON )).arg(spell_name).arg(power);
            }
        }
    }

    if (m_item.hasCharges())
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::Charges + StringList::APPEND_COLON )).arg(m_item.getCharges());

    int regen = m_item.getHPRegen();
    if (regen < 0)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::HPDrain + StringList::APPEND_COLON )).arg(regen);
    else if (regen > 0)
        html += QString( "<p><font color=\"#916448\">%1</font> +%2" ).arg(::getStringTable()->getString( StringList::HPRegeneration + StringList::APPEND_COLON )).arg(regen);

    regen = m_item.getStaminaRegen();
    if (regen < 0)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::StaminaDrain + StringList::APPEND_COLON )).arg(regen);
    else if (regen > 0)
        html += QString( "<p><font color=\"#916448\">%1</font> +%2" ).arg(::getStringTable()->getString( StringList::StaminaRegeneration + StringList::APPEND_COLON )).arg(regen);

    regen = m_item.getSPRegen();
    if (regen < 0)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::SPDrain + StringList::APPEND_COLON )).arg(regen);
    else if (regen > 0)
        html += QString( "<p><font color=\"#916448\">%1</font> +%2" ).arg(::getStringTable()->getString( StringList::SPRegeneration + StringList::APPEND_COLON )).arg(regen);

    // Attribute Bonus / Attribute Penalty
    {
        int bonus;
        character::attribute a = m_item.getAttributeBonus( &bonus );

        if ((a != character::attribute::ATTRIBUTE_NONE) && (bonus != 0))
        {
            QString attribStr = ::getStringTable()->getString( StringList::LISTPrimaryAttributes + static_cast<int>(a) );

            if (bonus > 0)
            {
                html += QString( "<p><font color=\"#916448\">%1</font> %2 +%3" ).arg(::getStringTable()->getString( StringList::AttributeBonus + StringList::APPEND_COLON )).arg( attribStr ).arg( bonus );
            }
            else
            {
                html += QString( "<p><font color=\"#916448\">%1</font> %2 %3" ).arg(::getStringTable()->getString( StringList::AttributePenalty + StringList::APPEND_COLON )).arg( attribStr ).arg( bonus );
            }
        }
    }

    // Skill Bonus / Skill Penalty
    {
        int bonus;
        character::skill s = m_item.getSkillBonus( &bonus );

        if ((s != character::skill::SKILL_NONE) && (bonus != 0))
        {
            QString skillStr = ::getStringTable()->getString( StringList::LISTSkills + static_cast<int>(s) );

            if (bonus > 0)
            {
                html += QString( "<p><font color=\"#916448\">%1</font> %2 +%3" ).arg(::getStringTable()->getString( StringList::SkillBonusDup + StringList::APPEND_COLON )).arg( skillStr ).arg( bonus );
            }
            else
            {
                html += QString( "<p><font color=\"#916448\">%1</font> %2 %3" ).arg(::getStringTable()->getString( StringList::SkillPenalty + StringList::APPEND_COLON )).arg( skillStr ).arg( bonus );
            }
        }
    }

    // Resistances
    {
        int f=0, w=0, a=0, e=0, m=0, d=0;

        if (m_item.getResistance(&f, &w, &a, &e, &m, &d))
        {
            QString list;

            // Possibly a problem for some languages. Expectation is the first 2 chars of all of these are ", "
            // so they can be jumped over for the first item in the list by the mid() call below
            if (f) list += QString( tr(", %1% vs Fire") ).arg(f);
            if (w) list += QString( tr(", %1% vs Water") ).arg(w);
            if (a) list += QString( tr(", %1% vs Air") ).arg(a);
            if (e) list += QString( tr(", %1% vs Earth") ).arg(e);
            if (m) list += QString( tr(", %1% vs Mental") ).arg(m);
            if (d) list += QString( tr(", %1% vs Divine") ).arg(d);

            html += QString( "<p><font color=\"#916448\">%1</font> " ).arg(::getStringTable()->getString( StringList::Resistances + StringList::APPEND_COLON )) + list.mid(2);
        }
    }

    if (m_item.getUsableGenders() == character::gender::Male)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::Sex + StringList::APPEND_COLON )).arg(::getStringTable()->getString( StringList::MaleOnly ));
    else if (m_item.getUsableGenders() == character::gender::Female)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::Sex + StringList::APPEND_COLON )).arg(::getStringTable()->getString( StringList::FemaleOnly ));

    // Requirements
    {
        QString required = m_item.getRequiredAttribsString();
        QString required_s = m_item.getRequiredSkillsString();
        if (required.size() == 0)
        {
            required   = required_s;
            required_s = "";
        }
        /* NOT else if */
        if (required.size() > 0)
        {
            if (required_s.size() > 0)
                required += ", " + required_s;

            html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::Requires + StringList::APPEND_COLON )).arg(required);
        }
    }

    int sz = m_item.getMaxStackSize();
    if (sz > 0)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::MaxItemsPerSlot + StringList::APPEND_COLON )).arg( sz );

#ifdef EXTRA_FIELDS
    html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::Value + StringList::APPEND_COLON )).arg( m_item.getPrice() );
#endif

    QString desc = m_item.getDesc();
    if (desc.size() > 0)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getStringTable()->getString( StringList::Description + StringList::APPEND_COLON )).arg( desc );

    if (m_item.hasShots())
    {
        // Shots gets stored in the same field as charges since they're mutually exclusive
        // but semantically similar
        html += QString( "<p><font color=\"#916448\">%1</font> %2/%3" ).arg(::getStringTable()->getString( StringList::Shots + StringList::APPEND_COLON )).arg( m_item.getCharges() ).arg( m_item.getMaxCharges() );
    }
    else if (m_item.hasUses())
    {
        // Uses gets stored in the same field as charges since they're mutually exclusive
        // but semantically similar
        html += QString( "<p><font color=\"#916448\">%1</font> %2/%3" ).arg(::getStringTable()->getString( StringList::Uses + StringList::APPEND_COLON )).arg( m_item.getCharges() ).arg( m_item.getMaxCharges() );
    }

    return html;
}

void DialogItemInfo::profButton(bool)
{
    m_widgets[ I_PROFBAR ]->setVisible( true  );
    m_widgets[ I_RACEBAR ]->setVisible( false );

    character::professions profs = m_item.getUsableProfessions();

    QMetaEnum metaProf = QMetaEnum::fromType<character::profession>();

    for (int k=0; k<metaProf.keyCount(); k++)
    {
        character::profession p = static_cast<character::profession>( metaProf.value(k) );

        if (profs & p)
        {
            m_widgets[ PROFS_START + k   ]->setVisible( true  );
            m_widgets[ D_PROFS_START + k ]->setVisible( false );
        }
        else
        {
            m_widgets[ PROFS_START + k   ]->setVisible( false );
            m_widgets[ D_PROFS_START + k ]->setVisible( true  );
        }
    }
    for (int k=RACES_START; k<RACES_END; k++)
    {
        m_widgets[ k ]->setVisible( false );
    }
    for (int k=D_RACES_START; k<D_RACES_END; k++)
    {
        m_widgets[ k ]->setVisible( false );
    }
}

void DialogItemInfo::raceButton(bool)
{
    m_widgets[ I_PROFBAR ]->setVisible( false );
    m_widgets[ I_RACEBAR ]->setVisible( true  );

    character::races races = m_item.getUsableRaces();

    QMetaEnum metaRace = QMetaEnum::fromType<character::race>();

    for (int k=0; k<metaRace.keyCount(); k++)
    {
        character::race r = static_cast<character::race>( metaRace.value(k) );

        if (races & r)
        {
            m_widgets[ RACES_START + k   ]->setVisible( true  );
            m_widgets[ D_RACES_START + k ]->setVisible( false );
        }
        else
        {
            m_widgets[ RACES_START + k   ]->setVisible( false );
            m_widgets[ D_RACES_START + k ]->setVisible( true  );
        }
    }
    for (int k=PROFS_START; k<PROFS_END; k++)
    {
        m_widgets[ k ]->setVisible( false );
    }
    for (int k=D_PROFS_START; k<D_PROFS_END; k++)
    {
        m_widgets[ k ]->setVisible( false );
    }
}

DialogItemInfo::~DialogItemInfo()
{
    delete m_showGroup;
}


