/*
 * Copyright (C) 2024 Anonymous Idiot
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

#include "DialogParallelWorlds.h"
#include "SLFFile.h"
#include "common.h"
#include "main.h"

#include <QApplication>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>

#include "WButton.h"
#include "WCheckBox.h"
#include "WImage.h"
#include "WLabel.h"
#include "WListWidget.h"
#include "WScrollBar.h"

#include <QDebug>

typedef enum
{
    NO_ID,

    CB_IGNORE_STRINGS,

    WORLD_SCROLLLIST,
    WORLD_SCROLLBAR,

    SIZE_WIDGET_IDS
} widget_ids;


DialogParallelWorlds::DialogParallelWorlds(QStringList worlds, QString currentWorld, QWidget *parent)
    : Dialog(parent),
    m_currentWorld(""),
    m_ignoreModStrings(false)
{
    init();
    updateList(worlds);

    if (WListWidget *w = qobject_cast<WListWidget *>(m_widgets[ WORLD_SCROLLLIST ] ))
    {
        for (int k=0; k < w->count(); k++)
        {
            QListWidgetItem *wi = w->item(k);

            if (wi->text() == currentWorld)
            {
                w->setCurrentItem( wi );
                break;
            }
        }

        if (QListWidgetItem *it = w->currentItem())
        {
            m_currentWorld = it->text();
        }
    }

    show();
}

DialogParallelWorlds::~DialogParallelWorlds()
{
}

void DialogParallelWorlds::init()
{
    QPixmap bgImg = makeDialogForm();
    QSize   bgImgSize = bgImg.size();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    bgImg,                                                           this ),  -1,  NULL },

        { NO_ID,              QRect(  20,  25, 350,  12 ),    new WLabel(    QString("%1 %2").arg( ::getBaseStringTable()->getString( StringList::Wiz128Detected ) )
                                                                                             .arg( ::getBaseStringTable()->getString( StringList::ParallelWorldsEnabled ) ),  Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect(  20,  40, 350,  12 ),    new WLabel(    StringList::SelectParallelWorld, Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },

        { CB_IGNORE_STRINGS,  QRect(  20, 245, 350,  12 ),    new WCheckBox( StringList::IgnoreModStrings,                                    this ),  -1,  SLOT(setCb(int)) },
        { WORLD_SCROLLBAR,    QRect( 354,  63,  15, 175 ),    new WScrollBar( Qt::Orientation::Vertical,                                      this ),  -1,  NULL },
        { WORLD_SCROLLLIST,   QRect(  38,  62, 302, 176 ),    new WListWidget(                                                                this ),  -1,  NULL },
        { NO_ID,              QRect( 312, 268,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",                0, true, 1.0,   this ),  -1,  SLOT(accept()) },
        { NO_ID,              QRect( 344, 268,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",                4, true, 1.0,   this ),  -1,  SLOT(reject()) },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    // The worlds list
    if (WListWidget *worldList = qobject_cast<WListWidget *>(m_widgets[ WORLD_SCROLLLIST ] ))
    {
        if (WScrollBar *sb = qobject_cast<WScrollBar *>(m_widgets[ WORLD_SCROLLBAR ] ))
        {
            // QScrollAreas include their own scrollbar when necessary, but we need to
            // override this since the Wizardry 8 look has the scrollbar outside the
            // scroll area and a short space away to the right

            worldList->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
            worldList->setVerticalScrollBar( sb );
            // The call to setVerticalScrollBar reparents it, which prevents us drawing
            // it where we want, so reparent it (again) back to us
            sb->setParent( this );
            // return it to the same widget stack postion it was
            sb->stackUnder( m_widgets[ WORLD_SCROLLLIST ] );
        }
        connect( worldList, SIGNAL(itemClicked(QListWidgetItem *)),
                  this, SLOT(worldChanged(QListWidgetItem *)) );
    }

    this->setMinimumSize( bgImgSize * m_scale );
    this->setMaximumSize( bgImgSize * m_scale );

    // Something in the OK button displeased the dialog layout, and made a
    // bigger than minimum dialog. We have to make an additional call to force
    // it back to the right size after adding the OK button.
    this->resize( bgImgSize * m_scale );
}

void DialogParallelWorlds::worldChanged(QListWidgetItem *now)
{
    if (now)
    {
        m_currentWorld = now->text();
    }
}

void DialogParallelWorlds::setCb(int state)
{
    if (WCheckBox *cb = qobject_cast<WCheckBox *>( sender() ))
    {
        if (m_widgets.key( cb ) == CB_IGNORE_STRINGS)
        {
            m_ignoreModStrings = ( state == Qt::Checked );
        }
    }
}

void DialogParallelWorlds::updateList(QStringList worlds)
{
    // Repopulate the worlds list
    if (WListWidget *worldList = qobject_cast<WListWidget *>(m_widgets[ WORLD_SCROLLLIST ] ))
    {
        worldList->clear();
        worldList->addItems( worlds );
        worldList->sortItems(Qt::AscendingOrder);
        worldList->setCurrentRow(0);
    }
}

QPixmap DialogParallelWorlds::makeDialogForm()
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
    p.drawPixmap(   6,   6, bgPtn,   0,   0, 140,  64 ); // item slot
    p.drawPixmap(   6,   6, bgPtn,   0,   0,  82, 242 ); // most of the professions slot
    p.drawPixmap(   6,  bgImg.height() - bgPtn.height() + 124, bgPtn,   0, 110,  79, bgPtn.height() - 110);
    p.drawPixmap(   6, 240, bgPtn,   0, 190,  bgPtn.width(), 24 );       // LHS of the hole we made
    p.drawPixmap( 178, 240, bgPtn,   bgPtn.width()-200, 190,  200, 24 ); // RHS of the hole we made
    p.end();

    return customImage;
}

QPixmap DialogParallelWorlds::makeWider( QImage im, int width )
{
    QPainter p;
    QPixmap  in = QPixmap::fromImage( im );
    QPixmap  out( width, in.height() );

    out.fill( Qt::red );
    p.begin(&out);
    p.drawPixmap( 0, 0, in );
    p.drawPixmap( width - in.width() + 6, 0, in, 6, 0, in.width() - 6, in.height() );
    p.end();

    return out;
}
