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
#include <QDebug>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include "WButton.h"

#include "SLFFile.h"
#include "STItoQImage.h"
#include "main.h"

WButton::WButton(QWidget* parent, Qt::WindowFlags)
    : QPushButton(parent),
    Wizardry8Scalable(1.0),
    m_draggable(false),
    m_pixmap(QPixmap()),
    m_tag(-1)
{
    setCheckable(true);

    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        Wizardry8Scalable::setScale( w->getScale() );
    }
}

WButton::WButton(QString sti_file, int image_idx, bool bNoFifthState, double extraScale, QWidget* parent, Qt::WindowFlags)
    : QPushButton(parent),
    Wizardry8Scalable(1.0),
    m_draggable(false),
    m_pixmap(QPixmap()),
    m_tag(-1)
{
    // Base class variables can't be initialized in the defaults bit above
    m_extraScale = extraScale;

    SLFFile buttons( sti_file );
    if (buttons.open(QFile::ReadOnly))
    {
        QByteArray array = buttons.readAll();
        STItoQImage sti_buttons( array );

        // expect 5 state button; those that only have 4 need to
        // do some management of their own
        QImage inactive       = sti_buttons.getImage( image_idx   );
        QImage onMouseOver    = sti_buttons.getImage( image_idx+1 );
        QImage depressed      = sti_buttons.getImage( image_idx+2 );
        QImage disabled       = sti_buttons.getImage( image_idx+3 );
        QImage depressedMouse = bNoFifthState ? depressed : sti_buttons.getImage( image_idx+4 );

        QIcon icon;
        if (! inactive.isNull())
        {
            // In case we end up making this a draggable button
            m_pixmap = QPixmap::fromImage( inactive );
            icon.addPixmap( QPixmap::fromImage( inactive ),       QIcon::Normal,   QIcon::Off );
        }
        if (! disabled.isNull())
            icon.addPixmap( QPixmap::fromImage( disabled ),       QIcon::Disabled, QIcon::Off );
        if (! onMouseOver.isNull())
            icon.addPixmap( QPixmap::fromImage( onMouseOver ),    QIcon::Active,   QIcon::Off );
        if (! depressed.isNull())
            icon.addPixmap( QPixmap::fromImage( depressed ),      QIcon::Normal,   QIcon::On  );
        if (! disabled.isNull())
            icon.addPixmap( QPixmap::fromImage( disabled ),       QIcon::Disabled, QIcon::On  );
        if (! depressedMouse.isNull())
            icon.addPixmap( QPixmap::fromImage( depressedMouse ), QIcon::Active,   QIcon::On  );


        setIcon(icon);
        setIconSize( QSize( sti_buttons.getWidth( image_idx ), sti_buttons.getHeight( image_idx ) ) );
        setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed, QSizePolicy::ButtonBox));
        setCheckable(true);

        buttons.close();
    }

    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        setScale( w->getScale() );
    }
}

WButton::WButton(QPixmap *pixmap, bool bNoFifthState, double extraScale, QWidget* parent, Qt::WindowFlags)
    : QPushButton(parent),
    Wizardry8Scalable(1.0),
    m_draggable(false),
    m_pixmap(QPixmap()),
    m_tag(-1)
{
    // Base class variables can't be initialized in the defaults bit above
    m_extraScale = extraScale;

    QIcon icon;

    if (! pixmap[0].isNull())
    {
        // In case we end up making this a draggable button
        m_pixmap = pixmap[0];
        icon.addPixmap( pixmap[0], QIcon::Normal,   QIcon::Off );
    }

    if (! pixmap[1].isNull())
        icon.addPixmap( pixmap[1], QIcon::Active,   QIcon::Off );

    if (! pixmap[2].isNull())
        icon.addPixmap( pixmap[2], QIcon::Normal,   QIcon::On  );

    if (! pixmap[3].isNull())
    {
        icon.addPixmap( pixmap[3], QIcon::Disabled, QIcon::On  );
        icon.addPixmap( pixmap[3], QIcon::Disabled, QIcon::Off );
    }

    if (bNoFifthState)
    {
        if (! pixmap[2].isNull())
            icon.addPixmap( pixmap[2], QIcon::Active,   QIcon::On );
    }
    else
    {
        if (! pixmap[4].isNull())
            icon.addPixmap( pixmap[4], QIcon::Active,   QIcon::On );
    }


    setIcon(icon);
    setIconSize( pixmap[0].size() );
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed, QSizePolicy::ButtonBox));
    setCheckable(true);

    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        setScale( w->getScale() );
    }
}

WButton::~WButton()
{
}

void WButton::setScale(double scale)
{
    if (! m_rect.isNull())
    {
        // if the base rect has been setup, then use the left
        // and top values to set the position relative to the
        // parent widget - width and height always comes from
        // the pixmap.
        move( (double)m_rect.x() * scale, (double)m_rect.y() * scale );
    }
    resize( this->iconSize() * scale * m_extraScale );
    Wizardry8Scalable::setScale(scale);
}

// Methods required to implement drag and drop functionality with the button
void WButton::setDraggable( QString mimeType, int tag )
{
    m_draggable   = true;
    m_mimeType    = mimeType;
    m_tag         = tag;

    setAcceptDrops(true);
}

void WButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_beginDragPosition = this->mapFromGlobal(QCursor::pos());
//        emit clicked( true );
    }
    QPushButton::mousePressEvent(event);
}

void WButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_beginDragPosition = QPoint();
//        emit clicked( false );
    }
    QPushButton::mouseReleaseEvent(event);
}

void WButton::mouseMoveEvent(QMouseEvent *event)
{
    if (m_draggable && (event->buttons() & Qt::LeftButton) && (! m_beginDragPosition.isNull()))
    {
        if (! this->icon().isNull() && !(m_tag & EMPTY_FLAG))
        {
            QPoint mousePosition = this->mapFromGlobal(QCursor::pos());

            // Check if they've dragged it far enough to initiate drag and drop
            if ((mousePosition - m_beginDragPosition).manhattanLength() >= QApplication::startDragDistance())
            {
                // QDrag objects must have QObject parents, and they do the cleanup
                QDrag      *drag     = new QDrag(this);
                QMimeData  *mimeData = new QMimeData;
                QByteArray  itemData;
                QDataStream dataStream(&itemData, QIODevice::WriteOnly);

                dataStream << m_tag;

                mimeData->setData( m_mimeType, itemData );

                drag->setMimeData( mimeData );
                drag->setPixmap( m_pixmap );
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

                emit buttonDragged( m_tag & 0xff );

                // Default to CopyAction so that any drag to another app (eg. LibreOffice)
                // that doesn't support our mimetype, won't actually remove the item from us
                if (drag->exec( Qt::MoveAction | Qt::CopyAction, Qt::CopyAction ) != Qt::MoveAction)
                {
                    // dropped over an invalid area - reverse the drop by
                    // telling it the drop occurred back over the same item
                    // that it was removed from
                    emit buttonDropped( m_tag & 0xff );
                }
            }
        }
    }
    QPushButton::mouseMoveEvent(event);
}

void WButton::dragEnterEvent(QDragEnterEvent *event)
{
    if (m_draggable && event->mimeData()->hasFormat( m_mimeType ))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void WButton::dragMoveEvent(QDragMoveEvent *event)
{
    if (m_draggable && event->mimeData()->hasFormat( m_mimeType ))
    {
        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void WButton::dropEvent(QDropEvent *event)
{
    if (m_draggable && event->mimeData()->hasFormat( m_mimeType ))
    {
        const QMimeData *mime       = event->mimeData();
        QByteArray       itemData   = mime->data( m_mimeType );
        QDataStream      dataStream(&itemData, QIODevice::ReadOnly);

        int     tag;

        dataStream >> tag;

        emit buttonDropped( tag & 0xff );

        event->setDropAction(Qt::MoveAction);
        event->accept();
    }
}

// Methods required to implement a context menu on the button -
// actually we just pass this back to the calling class
void WButton::contextMenuEvent(QContextMenuEvent *event)
{
    emit contextMenu( event->globalPos() );
}
