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

#include "DialogSpellInfo.h"
#include "SLFFile.h"
#include "main.h"

#include <QApplication>
#include <QButtonGroup>
#include <QTextEdit>

#include "WButton.h"
#include "WImage.h"
#include "WLabel.h"
#include "WScrollBar.h"

#include "spell.h"

#include <QDebug>

typedef enum
{
    NO_ID,

    I_BG,

    REALM_ANIM,

    VAL_SPELL_COST,

    TB_DESC,
    DESC_SCROLLBAR,

    SIZE_WIDGET_IDS
} widget_ids;

DialogSpellInfo::DialogSpellInfo(const spell &s, QWidget *parent)
    : Dialog(parent),
    m_spell(s),
    m_timer(NULL)
{
    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { I_BG,               QRect(   0,   0,  -1,  -1 ),    new WImage(    "DIALOGS/POPUP_SPELLINFO.STI",                  0,              this ),  -1,  NULL },

        { REALM_ANIM,         QRect(  14,  14,  -1,  -1 ),    new WImage(                                                                    this ),  -1,  NULL },
        { TB_DESC,            QRect(  16,  68, 272, 186 ),    new QTextEdit(                                                                 this ),  -1,  NULL },
        { DESC_SCROLLBAR,     QRect( 300,  68,  15, 186 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },

        { NO_ID,              QRect( 282, 263,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               0, true, 1.0,   this ),  -1,  SLOT(close()) },

        { NO_ID,              QRect( 148,  23, 140,  12 ),    new WLabel(    s.getName(),                    Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect(  48,  38,  80,  12 ),    new WLabel(    StringList::SpellLevel,        Qt::AlignRight, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 148,  38,  40,  12 ),    new WLabel(    QString::number(s.getLevel()),  Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect(  42,  53, 120,  12 ),    new WLabel(    StringList::CostPerPowerLevel, Qt::AlignRight, 10, QFont::Thin, this ),  -1,  NULL },
        { VAL_SPELL_COST,     QRect( 180,  53, 150,  12 ),    new WLabel(    "",                             Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    // cost
    if (WLabel *q = qobject_cast<WLabel *>(m_widgets[ VAL_SPELL_COST ]))
    {
        q->setText( QString( tr("%1 SP") ).arg( m_spell.getSPCost() ) );
    }

    // The textbox
    if (QTextEdit *tb = qobject_cast<QTextEdit *>(m_widgets[ TB_DESC ] ))
    {
        tb->setFont(QFont("Wizardry", 10 * m_scale, QFont::Thin));
        tb->setReadOnly(true);

        tb->setHtml( htmlGenerateSpellProps() );

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

    if (WImage *w = qobject_cast<WImage *>(m_widgets[ REALM_ANIM ] ))
    {
        switch (s.getRealm())
        {
            case character::realm::Fire:
                w->setStiFile( "FLICS/REALMS/FIRE.STI", 0, true );
                break;
            case character::realm::Water:
                w->setStiFile( "FLICS/REALMS/WATER.STI", 0, true );
                break;
            case character::realm::Air:
                w->setStiFile( "FLICS/REALMS/AIR.STI", 0, true );
                break;
            case character::realm::Earth:
                w->setStiFile( "FLICS/REALMS/EARTH.STI", 0, true );
                break;
            case character::realm::Mental:
                w->setStiFile( "FLICS/REALMS/MENTAL.STI", 0, true );
                break;
            case character::realm::Divine:
                w->setStiFile( "FLICS/REALMS/DIVINE.STI", 0, true );
                break;
            default: // shutup the compiler warnings
                break;
        }
        m_timer = new QTimer(this);
        connect(m_timer, SIGNAL(timeout()), w, SLOT(animate()));
        m_timer->start(60);
    }

    show();
}

QString DialogSpellInfo::htmlGenerateSpellProps()
{
    QString html = "";

    // Spellbook
    {
        character::professions profs = m_spell.getClasses();

        QString p;

        // Possibly a problems for some languages. Expectation is the first 2 characters
        // of all of these are ", " and can be jumped over by the mid() call below for the
        // first entry.
        if (profs & character::profession::Mage)
            p += tr(", Mage");
        if (profs & character::profession::Priest)
            p += tr(", Priest");
        if (profs & character::profession::Alchemist)
            p += tr(", Alchemist");
        if (profs & character::profession::Psionic)
            p += tr(", Psionic");

        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getBaseStringTable()->getString( StringList::Spellbook + StringList::APPEND_COLON )).arg(p.mid(2));
    }
    // Usable
    html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getBaseStringTable()->getString( StringList::SpellUsable + StringList::APPEND_COLON )).arg( m_spell.getUsabilityString() );

    // Target
    html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getBaseStringTable()->getString( StringList::Target + StringList::APPEND_COLON )).arg(m_spell.getTargetString());

    // Range
    html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getBaseStringTable()->getString( StringList::Range + StringList::APPEND_COLON )).arg(m_spell.getRangeString());

    // Damage
    {
        quint16 min_damage, max_damage;

        m_spell.getDamage( &min_damage, &max_damage );
        if (max_damage != 0)
        {
            html += QString( "<p><font color=\"#916448\">%1</font> %2-%3 %4" ).arg(::getBaseStringTable()->getString( StringList::Damage + StringList::APPEND_COLON )).arg(min_damage).arg(max_damage).arg(::getBaseStringTable()->getString( StringList::HPperPL ) );
        }
    }

    // Duration
    {
        qint32 base, per_pl;

        m_spell.getDuration( &base, &per_pl );
        if ((base == 9999) || (per_pl == 9999))
        {
            html += QString( "<p><font color=\"#916448\">%1</font> %2 %3%4" ).arg(::getBaseStringTable()->getString( StringList::Duration + StringList::APPEND_COLON )).arg(::getBaseStringTable()->getString( StringList::Permanent ));
        }
        else
        {
            QString dur = "";
            int unit_idx = -1;

            if (base > 0)
            {
                int value = base;
                unit_idx  = StringList::Rounds;

                if (base >= 120)
                {
                    value    = base / 30;
                    unit_idx = StringList::GameHours;
                }
                else if (base >= 30)
                {
                    value = base / 6;
                    unit_idx = StringList::Minutes;
                }

                if (per_pl == 0)
                {
                    if (value == 1)
                        unit_idx++;

                    dur = QString( "%1 %2" ).arg(value).arg(::getBaseStringTable()->getString( unit_idx ));
                }
                else
                {
                    dur = QString( "%1 + " ).arg(value);
                }
            }
            if (per_pl > 0)
            {
                int value     = per_pl;
                int unit_idx2 = StringList::Rounds;

                if (per_pl >= 120)
                {
                    value     = per_pl / 30;
                    unit_idx2 = StringList::GameHours;
                }
                else if (per_pl >= 30)
                {
                    value     = per_pl / 6;
                    unit_idx2 = StringList::Minutes;
                }

                if ((unit_idx != -1) && (unit_idx != unit_idx2))
                {
                    qWarning() << "Malformed spell duration. Wizardry EXE will assert on this spell";
                }

                if ((base == 0) || (base == 1))
                {
                    if (value == 1)
                        unit_idx2++;
                }

                dur += QString( "%1 %2%3" ).arg(value).arg(::getBaseStringTable()->getString( unit_idx2 )).arg(::getBaseStringTable()->getString( StringList::PerPL ));
            }
            if ((base != 0) || (per_pl != 0))
            {
                html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getBaseStringTable()->getString( StringList::Duration + StringList::APPEND_COLON )).arg(dur);
            }
        }
    }

    // Description
    QString desc = m_spell.getDesc();
    if (desc.size() > 0)
        html += QString( "<p><font color=\"#916448\">%1</font> %2" ).arg(::getBaseStringTable()->getString( StringList::Description + StringList::APPEND_COLON )).arg( desc );

    return html;
}

DialogSpellInfo::~DialogSpellInfo()
{
    if (m_timer)
    {
        m_timer->stop();
        delete m_timer;
    }
}
