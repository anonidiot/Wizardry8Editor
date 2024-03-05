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

#include <QPainter>

#include <QDebug>

#include "DialogChooseColumns.h"
#include "main.h"

#include "WButton.h"
#include "WImage.h"
#include "WLabel.h"
#include "WListWidget.h"
#include "WScrollBar.h"

typedef enum
{
    NO_ID,

    I_BG,

    COLUMNS_SCROLLLIST,
    COLUMNS_SCROLLBAR,

    SIZE_WIDGET_IDS
} widget_ids;

DialogChooseColumns::DialogChooseColumns(const QList<column> &cols, QWidget *parent)
    : Dialog(parent)
{
    QPixmap bgImg = makeDialogForm();
    QSize   bgImgSize = bgImg.size();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    bgImg,                                                          this ),  -1,  NULL },

        { NO_ID,              QRect(  16,  22, 290,  12 ),    new WLabel( tr("Choose columns to display:"), Qt::AlignLeft, 10, QFont::Thin,  this ),  -1,  NULL },

        { COLUMNS_SCROLLLIST, QRect(  16,  38, 202, 186 ),    new WListWidget(                                                               this ),  -1,  NULL },
        { COLUMNS_SCROLLBAR,  QRect( 230,  39,  15, 185 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },

        { NO_ID,              QRect( 280,  60,  -1,  -1 ),    new WButton(   "DIALOGS/POPUP_SPLITITEM.STI",                  9, false, 1.0,  this ),  -1,  SLOT(moveUp(bool)) },
        { NO_ID,              QRect( 280,  92,  -1,  -1 ),    new WButton(   "DIALOGS/POPUP_SPLITITEM.STI",                  4, false, 1.0,  this ),  -1,  SLOT(moveDown(bool)) },

        { NO_ID,              QRect( 248, 234,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               0, true, 1.0,   this ),  -1,  SLOT(accept()) },

        { NO_ID,              QRect( 280, 234,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               4, true, 1.0,   this ),  -1,  SLOT(reject()) },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    // The columns list
    if (WListWidget *cols = qobject_cast<WListWidget *>(m_widgets[ COLUMNS_SCROLLLIST ] ))
    {
        if (WScrollBar *sb = qobject_cast<WScrollBar *>(m_widgets[ COLUMNS_SCROLLBAR ] ))
        {
            // QScrollAreas include their own scrollbar when necessary, but we need to
            // override this since the Wizardry 8 look has the scrollbar outside the
            // scroll area and a short space away to the right

            cols->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
            cols->setVerticalScrollBar( sb );
            // The call to setVerticalScrollBar reparents it, which prevents us drawing
            // it where we want, so reparent it (again) back to us
            sb->setParent( this );
            // return it to the same widget stack postion it was
            sb->stackUnder( m_widgets[ COLUMNS_SCROLLLIST ] );
        }
    }

    m_metaCols = QMetaEnum::fromType<DialogChooseColumns::column>();

    setColumns(cols);

    this->setMinimumSize( bgImgSize * m_scale );
    this->setMaximumSize( bgImgSize * m_scale );

    // Something in the OK button displeased the dialog layout, and made a
    // bigger than minimum dialog. We have to make an additional call to force
    // it back to the right size after adding the OK button.
    this->resize( bgImgSize * m_scale );
}

DialogChooseColumns::~DialogChooseColumns()
{
}


QPixmap DialogChooseColumns::makeDialogForm()
{
    // there is no suitable existing Wizardry image for the dialog we
    // want here, so we hack one together from a few different pieces
    QPixmap bgImg,     bgPtn;

    // images used in dialog
    bgImg = SLFFile::getPixmapFromSlf( "DIALOGS/POPUP_MONSTERINFO.STI", 0 );
    bgPtn = SLFFile::getPixmapFromSlf( "DIALOGS/DIALOGBACKGROUND.STI", 0 );

    QPixmap customImage( QSize( bgImg.width(), bgImg.height() ) );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, bgImg,   0,   0, bgImg.width(), bgImg.height() );       // Base
    p.drawPixmap( 100,  36, bgImg, 170,  36, bgImg.width() - 180, 190 );            // Shrunk scrollpane
    p.drawPixmap( 250,  36, bgPtn,   0,   0,  72, 190 );
    p.end();

    return customImage;
}

void DialogChooseColumns::moveUp(bool checked)
{
    if (checked)
    {
        if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
        {
            // non-checkable button
            q->setChecked(false);
        }

        if (WListWidget *collist = qobject_cast<WListWidget *>(m_widgets[ COLUMNS_SCROLLLIST ] ))
        {
            // This section of code does actually work with multiple columns selected

            int                      newrow = -1;
            QList<QListWidgetItem *> selected = collist->selectedItems();

            for (int k=0; k<selected.size(); k++)
            {
                int  row = collist->row( selected.at(k) );

                // Move it up a row if it isn't already at the top
                if (row > 0)
                {
                    QListWidgetItem *i = collist->takeItem( row );

                    collist->insertItem( row - 1, i );
                    if (newrow == -1)
                        newrow = row - 1;
                }
            }

            // But QListWidget doesn't have a method for changing the selection to all
            // the new indexes we've just given these rows. Rather than implement it
            // myself I've just decided to only support single select in this dialog,
            // and hence we just set the selection to the top row selected afterwards
            // (because it'll be the only one selected now).

            if (newrow != -1)
            {
                collist->setCurrentRow( newrow );
            }

            this->update();
        }
    }
}

void DialogChooseColumns::moveDown(bool checked)
{
    if (checked)
    {
        if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
        {
            // non-checkable button
            q->setChecked(false);
        }

        if (WListWidget *collist = qobject_cast<WListWidget *>(m_widgets[ COLUMNS_SCROLLLIST ] ))
        {
            // This section of code does actually work with multiple columns selected

            int                      newrow = -1;
            QList<QListWidgetItem *> selected = collist->selectedItems();

            for (int k=selected.size() - 1; k >=0; k--)
            {
                int  row = collist->row( selected.at(k) );

                // Move it down a row if it isn't already at the bottom
                if (row < collist->count())
                {
                    QListWidgetItem *i = collist->takeItem( row );

                    collist->insertItem( row + 1, i );
                    newrow = row+1;
                }
                else
                {
                    newrow = -1;
                }
            }
            // This bit doesn't (same as above).

            if (newrow != -1)
            {
                collist->setCurrentRow( newrow );
            }

            this->update();
        }
    }
}

QList<DialogChooseColumns::column> DialogChooseColumns::getColumns()
{
    QList<column>  cols;

    if (WListWidget *collist = qobject_cast<WListWidget *>(m_widgets[ COLUMNS_SCROLLLIST ] ))
    {
        for (int k=0; k<collist->count(); k++)
        {
            QListWidgetItem *i = collist->item( k );

            if ( i->checkState() )
            {
                cols.append( static_cast<column> ( i->data( Qt::UserRole ).toInt() ) );
            }
        }
    }

    return cols;
}

void DialogChooseColumns::setColumns(const QList<column> &cols)
{
    if (WListWidget *collist = qobject_cast<WListWidget *>(m_widgets[ COLUMNS_SCROLLLIST ] ))
    {
        collist->clear();

        // First the active columns, in the order selected
        for (int k=0; k<cols.size(); k++)
        {
            QListWidgetItem *newCol = new QListWidgetItem( ::getBaseStringTable()->getString( cols.at(k) ) );
            newCol->setData( Qt::UserRole, cols.at(k) );
            newCol->setFlags( newCol->flags() | Qt::ItemIsUserCheckable );
            newCol->setCheckState( Qt::Checked );

            collist->addItem( newCol );            
        }
        // Now all the other columns that weren't selected
        for (int k=0; k<m_metaCols.keyCount(); k++)
        {
            column  c = static_cast<column>( m_metaCols.value(k) );

            if (! cols.contains( c ))
            {
                QListWidgetItem *newCol = new QListWidgetItem( ::getBaseStringTable()->getString( c ) );
                newCol->setData( Qt::UserRole, c );
                newCol->setFlags( newCol->flags() | Qt::ItemIsUserCheckable );
                newCol->setCheckState( Qt::Unchecked );

                collist->addItem( newCol );            
            }
        }
    }
}
