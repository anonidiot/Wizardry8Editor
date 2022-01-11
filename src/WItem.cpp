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
#include <QAction>
#include <QByteArray>
#include <QContextMenuEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include "WItem.h"

#include "main.h"
#include "SLFFile.h"
#include "STItoQImage.h"

#include <QDebug>

static const int     kBackpackItemMaxHeight = 55;
static const int     kBackpackItemIndex     =  0;
static const int     kExtendedItemIndex     =  1;
static const QString kItemMimeType          = "application/x-wiz8-item";

WItem::WItem(context_mode contexts, QWidget* parent, Qt::WindowFlags)
    : QWidget(parent),
    Wizardry8Scalable(1.0),
    m_item(),
    m_usable(true),
    m_beginDragPosition(QPoint())
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        Wizardry8Scalable::setScale( w->getScale() );
    }

    if (contexts != context_mode::None)
    {
        setAcceptDrops(true);
    }
    createContextMenuActions(contexts);
}

WItem::WItem(const item &i, context_mode contexts, QWidget* parent, Qt::WindowFlags)
    : QWidget(parent),
    Wizardry8Scalable(1.0),
    m_usable(true),
    m_beginDragPosition(QPoint())
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        Wizardry8Scalable::setScale( w->getScale() );
    }

    if (contexts != context_mode::None)
    {
        setAcceptDrops(true);
    }
    createContextMenuActions(contexts);

    setItem( i );
    resetPixmaps();
}

WItem::~WItem()
{
    if (m_cmInspectItem) m_cmInspectItem->deleteLater();
    if (m_cmEditItem)    m_cmEditItem->deleteLater();
    if (m_cmAddItem)     m_cmAddItem->deleteLater();
    if (m_cmDropItem)    m_cmDropItem->deleteLater();
}

void WItem::setUsabilityBasedOnCharacter(const character &c)
{
    m_usable = c.isItemUsable( m_item );

    resetPixmaps();
}

void WItem::setItem(const item &i)
{
    m_item   = i;
    m_usable = true; // until told otherwise

    if (! i.isNull())
    {
        this->setToolTip( i.getName() );
    }
}

const item &WItem::getItem()
{
    return m_item;
}

void WItem::setScale(double scale)
{
    if (! m_rect.isNull())
    {
        // if the base rect has been setup, then use it to
        // reposition _and_ resize the widget
        move(  (double)m_rect.x()     * scale, (double)m_rect.y()      * scale );
        resize((double)m_rect.width() * scale, (double)m_rect.height() * scale );
        resetPixmaps();
    }
    m_fontSize = 10 * scale;
    Wizardry8Scalable::setScale(scale);
}

void WItem::resetPixmaps()
{
    if (m_item.isNull())
    {
        m_itemPixmap = QPixmap();
    }
    else
    {
        SLFFile imgs( "ITEMS/" + m_item.getStiFile().toUpper() );

        if (imgs.open(QFile::ReadOnly))
        {
            QByteArray array = imgs.readAll();
            STItoQImage sti_imgs( array );

            if (m_rect.height() <= kBackpackItemMaxHeight)
                m_itemPixmap = QPixmap::fromImage( sti_imgs.getImage( kBackpackItemIndex ));
            else // Wearable item, which has more height (different aspect ratio)
                m_itemPixmap = QPixmap::fromImage( sti_imgs.getImage( kExtendedItemIndex ));

            imgs.close();
        }
        else
        {
            // The item lacks an image - some mods have this problem; use our
            // generic replacement icon
            m_itemPixmap = QPixmap( item::getMissingItemImage() );
        }

        if (m_usable)
        {
            m_usablePixmap = QPixmap();
        }
        else
        {
            // item not usable by this profession
            SLFFile sti_file( "REVIEW/ITEMUSABLEBACKGROUND.STI" );

            if (!sti_file.open(QFile::ReadOnly))
            {
                qWarning() << "Could not open file" << sti_file.fileName();
            }
            else
            {
                QByteArray array = sti_file.readAll();

                STItoQImage s( array);

                m_usablePixmap = QPixmap::fromImage( s.getImage( 0 ));
            }
        }
    }
}

void WItem::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    // Make the upscaling look nicer
    painter.setRenderHint( QPainter::Antialiasing,          true );
    painter.setRenderHint( QPainter::TextAntialiasing,      true );
    painter.setRenderHint( QPainter::SmoothPixmapTransform, true );

    if (m_item.isNull())
    {
        // draw a big lot of nothing
    }
    else
    {
        if (! m_usable)
        {
            painter.drawPixmap( rect(), m_usablePixmap );
        }

        // inset the rect slightly to draw the item itself - don't want it
        // completely filling the rectangle because it doesn't look right.
        QRect item_rect = rect().adjusted( m_scale, m_scale, -2*m_scale, -2*m_scale );

        // Scale, but maintain aspect ratio - no convenience methods on QPainter
        // to do this, so work it out ourselves
        double width_ratio  = item_rect.width()  / (double)m_itemPixmap.width();
        double height_ratio = item_rect.height() / (double)m_itemPixmap.height();

        if (width_ratio > height_ratio)
        {
            painter.drawPixmap((int)(item_rect.x() + (item_rect.width() - height_ratio * m_itemPixmap.width())/2),
                               item_rect.y(),
                               (int)(height_ratio * m_itemPixmap.width()),
                               item_rect.height(),
                               m_itemPixmap);
        }
        else
        {
            painter.drawPixmap(item_rect.x(),
                               (int)(item_rect.y() + (item_rect.height() - width_ratio * m_itemPixmap.height())/2),
                               item_rect.width(),
                               (int)(width_ratio * m_itemPixmap.height()),
                               m_itemPixmap);
        }

        // Draw a red rectangle around the item if it is a cursed object
        // (irrespective of whether curse currently in effect or not)
        if (m_item.isCursed())
        {
            QRectF russian_doll(item_rect);

            // This is a lazy replacement for using the object specific
            // cursed pixmaps in Wizardry - each position has its own
            // pixmap.

            painter.setPen( QColor::fromRgb(0x8e, 0x08, 0x08) );
            painter.drawRect( russian_doll );
            russian_doll.adjust( m_scale, m_scale, -m_scale, -m_scale );
            painter.setPen( QColor::fromRgb(0x6d, 0x02, 0x02) );
#ifdef WIN64
            // Crash observed on Windows 10 trying to draw at FP coordinates. Only
            // happens with 64 bit version, but 32 bit version doesn't support HiDPI
            // either. Could be a Qt, Windows or HiDpi thing. Don't know. So probably
            // not ifdefed appropriately here.
            painter.drawRect( russian_doll.toAlignedRect() );
#else
            painter.drawRect( russian_doll );
#endif
            russian_doll.adjust( m_scale, m_scale, -m_scale, -m_scale );
            painter.setPen( QColor::fromRgb(0x5a, 0x00, 0x00) );
#ifdef WIN64
            painter.drawRect( russian_doll.toAlignedRect() );
#else
            painter.drawRect( russian_doll );
#endif
            russian_doll.adjust( m_scale, m_scale, -m_scale, -m_scale );
            painter.setPen( QColor::fromRgb(0x3e, 0x00, 0x00) );
#ifdef WIN64
            painter.drawRect( russian_doll.toAlignedRect() );
#else
            painter.drawRect( russian_doll );
#endif
        }

        painter.setFont(QFont("Wizardry", m_fontSize, QFont::Thin));

        // Text numbers for various counts / charges / shots
        if (m_item.isStackable())
        {
            painter.setPen(QColor::fromRgb(0xe0, 0xe0, 0xc3)); // Light yellow
            painter.drawText(
                QRectF((int)(item_rect.x()),
                       (int)(item_rect.y() + item_rect.height() - 14 * m_scale),
                       item_rect.width() - 2 * m_scale, 12 * m_scale),
                Qt::AlignRight, QString::number( m_item.getCount() ));
        }
        else if (m_item.hasCharges())
        {
            painter.setPen(QColor::fromRgb(0x00, 0xad, 0xe3)); // Blue
            painter.drawText(
                QRectF((int)(item_rect.x()),
                       (int)(item_rect.y() + item_rect.height() - 14 * m_scale),
                       item_rect.width() - 2 * m_scale, 12 * m_scale),
                Qt::AlignRight, QString::number( m_item.getCharges() ));
        }
        else if (m_item.hasShots())
        {
            painter.setPen(QColor::fromRgb(0xfc, 0xf4, 0x49)); // Yellow
            painter.drawText(
                QRectF((int)(item_rect.x()),
                       (int)(item_rect.y() + item_rect.height() - 14 * m_scale),
                       item_rect.width() - 2 * m_scale, 12 * m_scale),
                Qt::AlignRight, QString::number( m_item.getCharges() ));
        }
    }
}

void WItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_beginDragPosition = this->mapFromGlobal(QCursor::pos());
        emit clicked( true );
    }
}

void WItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_beginDragPosition = QPoint();
        emit clicked( false );
    }
}

void WItem::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) && (! m_beginDragPosition.isNull()))
    {
        if (! m_item.isNull())
        {
            QPoint mousePosition = this->mapFromGlobal(QCursor::pos());

            // Check if they've dragged it far enough to initiate drag and drop
            if ((mousePosition - m_beginDragPosition).manhattanLength() >= QApplication::startDragDistance())
            {
                item     draggedItem = m_item;

                // QDrag objects must have QObject parents, and they do the cleanup
                QDrag      *drag     = new QDrag(this);
                QMimeData  *mimeData = new QMimeData;
                QByteArray  itemData;
                QDataStream dataStream(&itemData, QIODevice::WriteOnly);

                dataStream << draggedItem;

                mimeData->setData( kItemMimeType, itemData );
                mimeData->setText( m_item.getName() );
                mimeData->setHtml( m_item.getCompleteData(true) );

                drag->setMimeData( mimeData );
                drag->setPixmap( m_itemPixmap );
                drag->setHotSpot( event->pos() );
                QPixmap handCursor = ::getCursor(Qt::ClosedHandCursor );

                if (! handCursor.isNull() )
                {
                    QPainter painter;

                    drag->setDragCursor( handCursor, Qt::MoveAction );

                    // overlay the crossed circle not allowed image
                    painter.begin(&handCursor);
                    painter.setPen( QPen( QColor::fromRgb(0xec, 0x00, 0x0f), 2 ) );

                    QRectF r( handCursor.width() / 2,
                              handCursor.height() / 2,
                              handCursor.width() / 3,
                              handCursor.width() / 3 );

                    painter.drawChord( r.toAlignedRect(), 40 * 16,  180 * 16 );
                    painter.drawChord( r.toAlignedRect(), 40 * 16, -180 * 16 );
                    painter.end();

                    // Windows won't obey this
                    drag->setDragCursor( handCursor, Qt::IgnoreAction );
                }

                // Normal procedure is to only hide the data being moved, and not actually
                // remove it until the Qt::MoveAction is successful. But that invites a race
                // condition if the object is dropped back in the same place it was taken from -
                // we then get the dropEvent() trying to allocate something in this place, and
                // then code here trying to remove it afterwards.
                // So we have to remove the item properly BEFORE initiating the drag operation,
                // and if it gets cancelled for any reason, we have to reverse it.

                setItem( item() );
                emit itemDragged( draggedItem );

                // Default to CopyAction so that any drag to another app (eg. LibreOffice)
                // that doesn't support our mimetype, won't actually remove the item from us
                if (drag->exec( Qt::MoveAction | Qt::CopyAction, Qt::CopyAction ) != Qt::MoveAction)
                {
                    // dropped over an invalid area - reverse the drop by
                    // telling it the drop occurred back over the same item
                    // that it was removed from
                    emit itemDropped( draggedItem );
                }
            }
        }
    }
}

void WItem::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat( kItemMimeType ))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void WItem::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat( kItemMimeType ))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void WItem::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat( kItemMimeType ))
    {
        const QMimeData *mime       = event->mimeData();
        QByteArray       itemData   = mime->data( kItemMimeType );
        QDataStream      dataStream(&itemData, QIODevice::ReadOnly);

        item     i;

        dataStream >> i;

        // Confusing choice of name, sorry. The 'drop' here relates to
        // the drop in "Drag and Drop" and has nothing to do with removing
        // an object. It is signalling the completion of an item move.
        emit itemDropped( i );

        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

void WItem::cmInspectItem()
{
    if (! m_item.isNull())
    {
        emit inspectItem( m_item );
    }
}

void WItem::cmEditItem()
{
    if (! m_item.isNull())
    {
        emit editItem( m_item );
    }
}

void WItem::cmAddItem()
{
    emit addItem();
}

void WItem::cmDropItem()
{
    if (! m_item.isNull())
    {
        emit dropItem( m_item );
    }
}

void WItem::createContextMenuActions(context_mode contexts)
{
    if (contexts == context_mode::None)
    {
        m_cmInspectItem = NULL;
        m_cmEditItem    = NULL;
        m_cmAddItem     = NULL;
        m_cmDropItem    = NULL;
    }
    else
    {
        m_cmInspectItem = new QAction( ::getStringTable()->getString( StringList::InspectItem ), this);
        m_cmInspectItem->setStatusTip(tr("View Item Properties"));
        connect(m_cmInspectItem, SIGNAL(triggered()), this, SLOT(cmInspectItem()));

        m_cmEditItem = new QAction( ::getStringTable()->getString( StringList::EditItem ), this);
        m_cmEditItem->setStatusTip(tr("Edit Item Properties"));
        connect(m_cmEditItem, SIGNAL(triggered()), this, SLOT(cmEditItem()));

        if (contexts == context_mode::All)
        {
            m_cmAddItem = new QAction( ::getStringTable()->getString( StringList::AddItem ), this);
            m_cmAddItem->setStatusTip(tr("Add new Item"));
            connect(m_cmAddItem, SIGNAL(triggered()), this, SLOT(cmAddItem()));

            m_cmDropItem = new QAction( ::getStringTable()->getString( StringList::DropItem ), this);
            m_cmDropItem->setStatusTip(tr("Discard Item"));
            connect(m_cmDropItem, SIGNAL(triggered()), this, SLOT(cmDropItem()));
        }
        else
        {
            m_cmAddItem  = NULL;
            m_cmDropItem = NULL;
        }
    }
}

void WItem::contextMenuEvent(QContextMenuEvent *event)
{
    if (!m_cmInspectItem && !m_cmEditItem && !m_cmDropItem && !m_cmAddItem)
        return;

    if (m_item.isNull())
    {
        if (m_cmInspectItem) m_cmInspectItem->setEnabled(false);
        if (m_cmEditItem)    m_cmEditItem->setEnabled(false);
        if (m_cmDropItem)    m_cmDropItem->setEnabled(false);
    }
    else
    {
        if (m_cmInspectItem) m_cmInspectItem->setEnabled(true);
        if (m_cmEditItem)    m_cmEditItem->setEnabled(true);
        if (m_cmDropItem)    m_cmDropItem->setEnabled(true);
    }

    QMenu menu(this);

    if (m_cmInspectItem)
    {
        menu.addAction(m_cmInspectItem);
        menu.addSeparator();
    }
    if (m_cmEditItem) menu.addAction(m_cmEditItem);
    if (m_cmAddItem)  menu.addAction(m_cmAddItem);
    if (m_cmDropItem) menu.addAction(m_cmDropItem);

    menu.exec(event->globalPos());
}
