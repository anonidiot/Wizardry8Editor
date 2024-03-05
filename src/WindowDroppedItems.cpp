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

#include <QCloseEvent>

#include "WindowDroppedItems.h"

#include "SLFFile.h"
#include "main.h"

#include <QApplication>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>

#include "DialogAddItem.h"
#include "DialogItemInfo.h"
#include "WImage.h"
#include "WItem.h"
#include "WLabel.h"
#include "WScrollBar.h"

#include <QDebug>

#define ITEMS_ACROSS 4

typedef enum
{
    NO_ID,

    ITEMS_START,
    VAL_DROPPED1 = ITEMS_START,
    VAL_DROPPED2,
    VAL_DROPPED3,
    VAL_DROPPED4,
    VAL_DROPPED5,
    VAL_DROPPED6,
    VAL_DROPPED7,
    VAL_DROPPED8,
    VAL_DROPPED9,
    VAL_DROPPED10,
    VAL_DROPPED11,
    VAL_DROPPED12,
    VAL_DROPPED13,
    VAL_DROPPED14,
    VAL_DROPPED15,
    VAL_DROPPED16,
    ITEMS_END,

    DROPPED_SCROLLBAR,

    SIZE_WIDGET_IDS
} widget_ids;


WindowDroppedItems::WindowDroppedItems(party *p)
    : QWidget(),
    Wizardry8Scalable(::getAppScale()),
    m_party(p),
    m_droppedItems_scrollPos(0)
{
    QPixmap bgImg = makeDialogForm();

    m_bgImgSize = bgImg.size();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout droppedItemsScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    bgImg,                                                          this ),  -1,  NULL },
        { NO_ID,              QRect(  34,  10, 191,  12 ),    new WLabel(    StringList::DroppedItems,     Qt::AlignCenter, 10, QFont::Bold, this ),  -1,  NULL },

        { VAL_DROPPED1,       QRect(  34,  26,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED2,       QRect(  82,  26,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED3,       QRect( 130,  26,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED4,       QRect( 178,  26,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED5,       QRect(  34,  83,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED6,       QRect(  82,  83,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED7,       QRect( 130,  83,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED8,       QRect( 178,  83,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED9,       QRect(  34, 140,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED10,      QRect(  82, 140,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED11,      QRect( 130, 140,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED12,      QRect( 178, 140,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED13,      QRect(  34, 197,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED14,      QRect(  82, 197,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED15,      QRect( 130, 197,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },
        { VAL_DROPPED16,      QRect( 178, 197,  46,  55 ),    new WItem( WItem::context_mode::NoDrops,                                       this ),  -1,  NULL },

        { DROPPED_SCROLLBAR,  QRect( 229,  28,  15, 223 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  SLOT(scrolledDroppedItems(int)) },
    };

    int num_widgets = sizeof(droppedItemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( droppedItemsScrn, num_widgets, this );
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
        }
    }

    resetDroppedItemsScrollbar();

    show();

    connect( m_party, SIGNAL(itemDropped(item)), this, SLOT(newItemDropped(item)) );
}

WindowDroppedItems::~WindowDroppedItems()
{
}

void WindowDroppedItems::closeEvent(QCloseEvent *event)
{
    emit windowClosing();
    event->accept();
}

void WindowDroppedItems::setScale(double scale)
{
    m_scale = scale;

    QList<QWidget *> widgets = this->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
    for (int k=0; k<widgets.size(); k++)
    {
        // No, we can't go straight to this in the findChildren() call above, because
        // it's a slim 'interface' that doesn't inherit from QObject
        if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(widgets[k]))
        {
            w->setScale( m_scale );
        }
    }

    this->resize( sizeHint() );

    this->update();
}

void WindowDroppedItems::resizeEvent(QResizeEvent *event)
{
    double h_scale = (double)event->size().width()  / (double)m_bgImgSize.width();
    double v_scale = (double)event->size().height() / (double)m_bgImgSize.height();

    if (h_scale < v_scale)
        m_scale = h_scale;
    else
        m_scale = v_scale;

    QList<QWidget *> widgets = this->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly);
    for (int k=0; k<widgets.size(); k++)
    {
        // We can't go straight to this in the findChildren() call above, because
        // it's a slim 'interface' that doesn't inherit from QObject
        if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(widgets[k]))
        {
            w->setScale( m_scale );
        }
    }

    this->update();
}

void WindowDroppedItems::wheelEvent(QWheelEvent *event)
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

            if      ((delta.y() > 0) && (m_droppedItems_scrollPos == 0))
            {
                event->ignore();
            }
            else if ((delta.y() < 0) && (m_droppedItems_scrollPos + (ITEMS_END - ITEMS_START) >= (int)m_party->getDroppedItemCount()))
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

void WindowDroppedItems::resetDroppedItemsScrollbar()
{
    if (QScrollBar *scroll_bar = qobject_cast<QScrollBar *>(m_widgets[ DROPPED_SCROLLBAR ]))
    {
        scroll_bar->setRange( 0, (m_party->getDroppedItemCount() - (ITEMS_END - ITEMS_START) + (ITEMS_ACROSS - 1)) / ITEMS_ACROSS );
        scroll_bar->setPageStep( (ITEMS_END - ITEMS_START) / ITEMS_ACROSS );
        scroll_bar->setSingleStep( 1 );
    }

    // reset the 16 permanent on-screen widgets with actual items from the
    // larger available list of party items based on the scrollbar position
    for (unsigned int k=ITEMS_START; k < ITEMS_END; k++)
    {
        if (WItem *q = qobject_cast<WItem *>(m_widgets[ k ]))
        {
            if (m_droppedItems_scrollPos + k - ITEMS_START >= (unsigned int)m_party->getDroppedItemCount())
                q->setItem( item() );
            else
            {
                item i = m_party->getDroppedItem(m_droppedItems_scrollPos + k - ITEMS_START);

                q->setItem( i );
            }
            q->resetPixmaps();
            q->update();
        }
    }
}

void WindowDroppedItems::itemDragged( item )
{
    if (WItem *q = qobject_cast<WItem *>(this->sender()))
    {
        int item_idx = m_droppedItems_scrollPos + m_widgets.key( q ) - ITEMS_START;

        m_party->removeDroppedItem( item_idx );
        resetDroppedItemsScrollbar();
    }
}

void WindowDroppedItems::itemDropped( item i )
{
    if (WItem *q = qobject_cast<WItem *>(this->sender()))
    {
        int item_idx = m_droppedItems_scrollPos + m_widgets.key( q ) - ITEMS_START;

        m_party->addDroppedItem( i, item_idx );
        resetDroppedItemsScrollbar();
    }
}

void WindowDroppedItems::newItemDropped(item i)
{
    (void)i;
    resetDroppedItemsScrollbar();
}

void WindowDroppedItems::scrolledDroppedItems(int position)
{
    m_droppedItems_scrollPos = position * ITEMS_ACROSS;
    resetDroppedItemsScrollbar();
}

QPixmap WindowDroppedItems::makeDialogForm()
{
    // there is no suitable existing Wizardry image for the dialog we
    // want here, so we hack one together from a few different pieces
    QPixmap bgImg,     bgPtn;

    // images used in dialog
    bgImg = SLFFile::getPixmapFromSlf( "REVIEW/REVIEWITEMPAGE.STI", 3 );
    bgPtn = SLFFile::getPixmapFromSlf( "DIALOGS/DIALOGBACKGROUND.STI", 0 );

    QPixmap customImage( QSize( bgImg.width() + 96, bgImg.height() ) );

    QPainter p;

    p.begin( &customImage );
    // First make it a little taller, by 20 rows and squeeze the hole for
    // the scrollarea and the buttonbox over 20 pixels to the right
    p.drawPixmap(   0,   0, bgImg,   0,   0, bgImg.width(),      bgImg.height() );  // LHS
    p.drawPixmap( 126,   0, bgImg,  30,   0, bgImg.width() - 30, bgImg.height() );  // RHS
    p.drawPixmap(   6,  26, bgPtn,   0,   0, 25, bgImg.height() - 30 ); // LHS holes
    p.drawPixmap(   6, 254, bgPtn,   0, 226, 230, 26 ); // Bottom holes

    p.end();

    return customImage;
}

void WindowDroppedItems::inspectItem(item i)
{
    new DialogItemInfo(i, this);
}

void WindowDroppedItems::editItem(item i)
{
    if (WItem *q = qobject_cast<WItem *>(this->sender()))
    {
        int   item_id = m_widgets.key( q );

        DialogAddItem *ai = new DialogAddItem(item_id, i, this);
        connect( ai, SIGNAL(itemAdded(int, item)), this, SLOT(itemEdited(int, item)));
    }
}

void WindowDroppedItems::itemEdited( int tag, item i )
{
    if (WItem *q = qobject_cast<WItem *>(m_widgets[ tag ]))
    {
        q->setItem( i );
        q->update();
    }
}
