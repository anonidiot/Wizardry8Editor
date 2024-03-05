/*
 * Copyright (C) 2022-2023 Anonymous Idiot
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

#include <QListWidgetItem>

#include <QDebug>

#include "Screen.h"
#include "STI.h"

#include "WDDL.h"

#include "WButton.h"
#include "WImage.h"
#include "WLabel.h"
#include "WListWidget.h"
#include "WScrollBar.h"

#define WIDGET_WIDTH           185 // width of 2 arrow buttons + ddl + spacing in between
#define CLOSED_WIDGET_HEIGHT    24 // when the listview isn't expanded
#define OPEN_ROW_HEIGHT         20 // diff in y co-ord between DDL_ROW2 and DDL_TOP (and all others)
#define FIRST_ROW_OFFSET        25 // y offset of DDL_TOP
#define DDL_WIDTH              122

typedef enum
{
    NO_ID,

    BTN_PREV,
    BTN_NEXT,

    DDL_FRAME,
    DDL_I,
    DDL_VAL,

    DDL_TOP,
    DDL_ROW2,
    DDL_ROW3,
    DDL_ROW4,
    DDL_ROW5,
    DDL_ROW6,
    DDL_ROW7,
    DDL_ROW8,
    DDL_ROW9,
    DDL_ROW10,
    DDL_ROW11,
    DDL_BOTTOM,
    DDL_SCROLLLIST,

    SIZE_WIDGET_IDS
} widget_ids;


WDDL::WDDL(QString fontName, Qt::Alignment alignment, int pointSize, int weight, QWidget* parent)
    : QWidget(parent),
    Wizardry8Scalable(1.0)
{
    loadDDLPixmaps();

    struct layout widgets_ddl[] =
    {
        { BTN_PREV,           QRect(   0,  0,  -1,  -1 ),    new WButton(   "CHAR GENERATION/CG_BUTTONS.STI",     10, false, 1.0,  this ),  -1,  SLOT(prev(bool)) },
        { DDL_FRAME,          QRect(  31,  2,  -1,  -1 ),    new WImage(                                                           this ),  -1,  SLOT(dropDownList(bool)) },
        { DDL_I,              QRect(  34,  5,  -1,  -1 ),    new WImage(                                                           this ),  -1,  SLOT(dropDownList(bool)) },
        { DDL_VAL,            QRect(  54,  5, 120,  18 ),    new WLabel(    "", fontName, alignment, pointSize, weight,            this ),  -1,  SLOT(dropDownList(bool)) },
        { BTN_NEXT,           QRect( 161,  0,  -1,  -1 ),    new WButton(   "CHAR GENERATION/CG_BUTTONS.STI",     15, false, 1.0,  this ),  -1,  SLOT(next(bool)) },

        { DDL_TOP,            QRect( 31, FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT*0,  -1,  -1 ),    new WImage( m_ddlTop,                this ),  -1,  NULL },
        { DDL_ROW2,           QRect( 31, FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT*1,  -1,  -1 ),    new WImage( m_ddlBottom,             this ),  -1,  NULL },
        { DDL_ROW3,           QRect( 31, FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT*2,  -1,  -1 ),    new WImage( m_ddlBottom,             this ),  -1,  NULL },
        { DDL_ROW4,           QRect( 31, FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT*3,  -1,  -1 ),    new WImage( m_ddlBottom,             this ),  -1,  NULL },
        { DDL_ROW5,           QRect( 31, FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT*4,  -1,  -1 ),    new WImage( m_ddlBottom,             this ),  -1,  NULL },
        { DDL_ROW6,           QRect( 31, FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT*5,  -1,  -1 ),    new WImage( m_ddlBottom,             this ),  -1,  NULL },
        { DDL_ROW7,           QRect( 31, FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT*6,  -1,  -1 ),    new WImage( m_ddlBottom,             this ),  -1,  NULL },
        { DDL_ROW8,           QRect( 31, FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT*7,  -1,  -1 ),    new WImage( m_ddlBottom,             this ),  -1,  NULL },
        { DDL_ROW9,           QRect( 31, FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT*8,  -1,  -1 ),    new WImage( m_ddlBottom,             this ),  -1,  NULL },
        { DDL_ROW10,          QRect( 31, FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT*9,  -1,  -1 ),    new WImage( m_ddlBottom,             this ),  -1,  NULL },
        { DDL_ROW11,          QRect( 31, FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT*10, -1,  -1 ),    new WImage( m_ddlBottom,             this ),  -1,  NULL },
        { DDL_BOTTOM,         QRect( 31, FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT*11, -1,  -1 ),    new WImage( m_ddlBottom,             this ),  -1,  NULL },
        { DDL_SCROLLLIST,     QRect( 33,  28, DDL_WIDTH, 240 ),    new WListWidget(  fontName, pointSize, weight,                        this ),  -1,  NULL },
    };

    int num_widgets = sizeof(widgets_ddl) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( widgets_ddl, num_widgets, this );

    if (WListWidget *ddl = qobject_cast<WListWidget *>(m_widgets[ DDL_SCROLLLIST ] ))
    {
        ddl->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
        ddl->clear();

        connect( ddl, SIGNAL(itemClicked(QListWidgetItem *)),
                 this, SLOT(ddlChanged(QListWidgetItem *)) );

        ddl->setMouseTracking(true);
        ddl->setStyleSheet("QListWidget { selection-color: #00df00; selection-background-color: transparent; } "
                           "*::item::hover { border-style: outset; border-width: 1px; border-color: beige; }");
    }

    // hide the Drop-down lists until they are clicked on
    showDDL( false );

    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        setScale( w->getScale() );
    }
}

WDDL::~WDDL()
{
}

void WDDL::setEnabled( bool enabled )
{
    QWidget::setEnabled( enabled );

    if (enabled == false)
    {
        showDDL( false );
    }
}

/*
This method was breaking the mouseover on the buttons and the DDL
void WDDL::setVisible( bool visible )
{
    QObjectList kids = children();

    for (int k=0; k<kids.size(); k++)
    {
        if (QWidget *q = qobject_cast<QWidget *>(kids.at(k)))
        {
            q->setVisible( visible );
        }
    }
    // but not the drop down lists
    for (int k=DDL_TOP; k<=DDL_BOTTOM; k++)
    {
        m_widgets[ k ]->setVisible( false );
    }
}
*/

void WDDL::loadDDLPixmaps()
{
    // DDL Pixmaps
    SLFFile imgs( "CHAR GENERATION/CG_PROFESSION.STI" );
    if (imgs.open(QFile::ReadOnly))
    {
        QByteArray array = imgs.readAll();
        STI sti_imgs( array );

        m_ddlInactive = QPixmap::fromImage( sti_imgs.getImage( 1 ) );
        m_ddlActive   = QPixmap::fromImage( sti_imgs.getImage( 2 ) );
        m_ddlTop      = QPixmap::fromImage( sti_imgs.getImage( 4 ) );

        // Middle isn't actually needed since we overlap the widgets slightly,
        // we can use the bottom widget for the middle one, and it keeps the
        // code simpler.

        // m_ddlMiddle   = QPixmap::fromImage( sti_imgs.getImage( 5 ) );
        m_ddlBottom   = QPixmap::fromImage( sti_imgs.getImage( 6 ) );

        imgs.close();
    }
}

void WDDL::mouseOverLabel(bool on)
{
    if (this->sender() == m_widgets[ DDL_VAL ])
    {
        if (WLabel *q = qobject_cast<WLabel *>(this->sender()))
        {
            if (q->isEnabled() && on)
                q->setStyleSheet("QLabel {color: #169e16}"); // Green
            else
                q->setStyleSheet("");
        }
    }
}

void WDDL::dropDownList(bool down)
{
    if (down)
    {
        bool show = false;

        if ((sender() == m_widgets[ DDL_FRAME ]) ||
            (sender() == m_widgets[ DDL_I     ]) ||
            (sender() == m_widgets[ DDL_VAL   ]))
        {
            show = ! m_widgets[ DDL_TOP ]->isVisible();
        }

        // If the clicked DDL list was open, close it
        // If it was closed open it
        // And close any other lists which may have already been open
        showDDL( show );
    }
}

void WDDL::showDDL(bool show)
{
    if (WListWidget *ddl = qobject_cast<WListWidget *>(m_widgets[ DDL_SCROLLLIST ] ))
    {
        int cnt = ddl->count();

        if (cnt > DDL_BOTTOM - DDL_TOP + 1)
            cnt = DDL_BOTTOM - DDL_TOP + 1;

        if (QWidget *wnd = qobject_cast<QWidget *>( parent() ))
        {
            if (((double)(OPEN_ROW_HEIGHT * cnt - 2) * m_scale ) > wnd->size().height() - this->pos().y())
            {
                // Keep the DDL within the bounds of the dialog
                cnt = ((wnd->size().height() - this->pos().y()) / m_scale + 2) / OPEN_ROW_HEIGHT -1;
                if (cnt < 1)
                    cnt = 1;
            }
        }

        int height = CLOSED_WIDGET_HEIGHT;

        ddl->setVisible( show );

        for (int k=DDL_TOP; k<=DDL_BOTTOM; k++)
        {
            m_widgets[ k ]->setVisible( (k - DDL_TOP < cnt) ? show : false );
        }

        if (show)
        {
            // Magic number fudges to get the bootom frame to not be occluded
            height = FIRST_ROW_OFFSET + OPEN_ROW_HEIGHT * cnt + 4;

            ddl->resize( (double)DDL_WIDTH * m_scale, (double)(OPEN_ROW_HEIGHT * cnt - 2) * m_scale );
        }

        resize( (double)WIDGET_WIDTH * m_scale, (double)height * m_scale );

        // Since putting this in its own widget, now don't have a way to disable the other DDLs;
        // so emit a signal that the implementer can use to disable them when this one is active
        if (show)
        {
            emit listActive();
        }
    }
}

int WDDL::count()
{
    if (WListWidget *ddl = qobject_cast<WListWidget *>(m_widgets[ DDL_SCROLLLIST ] ))
    {
        return ddl->count();
    }
    return 0;
}

void WDDL::addItem(QListWidgetItem *i)
{
    if (WListWidget *ddl = qobject_cast<WListWidget *>(m_widgets[ DDL_SCROLLLIST ] ))
    {
        ddl->addItem( i );
    }
}

bool WDDL::setCurrentRow(int row)
{
    if (WListWidget *ddl = qobject_cast<WListWidget *>(m_widgets[ DDL_SCROLLLIST ] ))
    {
        if (ddl->item(row)->flags() & Qt::ItemIsEnabled)
        {
            ddl->setCurrentRow( row );
            return true;
        }
    }
    return false;
}

QListWidgetItem *WDDL::item(int idx)
{
    if (WListWidget *ddl = qobject_cast<WListWidget *>(m_widgets[ DDL_SCROLLLIST ] ))
    {
        return ddl->item(idx);
    }
    return NULL;
}

void WDDL::prev(bool)
{
    int value = changeListItem( -1 );

    updateList();

    emit valueChanged( value );
}

void WDDL::next(bool)
{
    int value = changeListItem( 1 );

    updateList();

    emit valueChanged( value );
}

int WDDL::changeListItem( int delta )
{
    int newValue = -1;
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        q->setChecked(false);
    }

    if (WListWidget *ddl = qobject_cast<WListWidget *>(m_widgets[ DDL_SCROLLLIST ] ))
    {
        int newIndex = ddl->currentRow();

        do
        {
            newIndex += delta;
            if (newIndex >= ddl->count())
                newIndex = 0;
            else if (newIndex < 0)
                newIndex = ddl->count() - 1;
        }
        while (! setCurrentRow( newIndex ));

        newValue = ddl->currentItem()->data( Qt::UserRole ).toInt();
    }
    return newValue;
}

void WDDL::ddlChanged(QListWidgetItem *now)
{
    if (now)
    {
        showDDL( false );

        int value = now->data( Qt::UserRole ).toInt();

        updateList();

        emit valueChanged( value );
    }
}

void WDDL::setScale(double scale)
{
    if (! m_rect.isNull())
    {
        // if the base rect has been setup, then use it to
        // reposition _and_ resize the widget
        move( (double)m_rect.x() * scale, (double)m_rect.y() * scale );

        int height = CLOSED_WIDGET_HEIGHT;

        if (WListWidget *ddl = qobject_cast<WListWidget *>(m_widgets[ DDL_SCROLLLIST ] ))
        {
            if (ddl->isVisible())
            {
                height += ddl->count() * OPEN_ROW_HEIGHT + FIRST_ROW_OFFSET;
            }
        }
        resize( (double)WIDGET_WIDTH * scale, (double)height * scale );
    }

    QObjectList kids = children();

    for (int k=0; k<kids.size(); k++)
    {
        if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(kids[k]))
        {
            w->setScale( scale );
        }
    }
    Wizardry8Scalable::setScale(scale);
}

void WDDL::updateList()
{
    if (WListWidget *ddl = qobject_cast<WListWidget *>(m_widgets[ DDL_SCROLLLIST ] ))
    {
        if (ddl->currentItem())
        {
            if (WLabel *w = qobject_cast<WLabel *>(m_widgets[ DDL_VAL ] ))
            {
                QString s = ddl->currentItem()->text();

                w->setText( s.isNull() ? "" : s );
            }
            if (WImage *w = qobject_cast<WImage *>(m_widgets[ DDL_I ] ))
            {
                QVariant v = ddl->currentItem()->data( Qt::DecorationRole );
                QPixmap ic = v.value<QPixmap>();

                w->setPixmap( ic );
            }
        }
        update();
    }
}
