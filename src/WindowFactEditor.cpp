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

#include "WindowFactEditor.h"

#include "SLFFile.h"
#include "main.h"

#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>

#include "WButton.h"
#include "WImage.h"
#include "WLabel.h"
#include "WListWidget.h"
#include "WScrollBar.h"

#include <QDebug>

typedef enum
{
    NO_ID,

    FACTS_SCROLLLIST,
    FACTS_SCROLLBAR,

    SIZE_WIDGET_IDS
} widget_ids;


WindowFactEditor::WindowFactEditor(facts &f)
    : QWidget(),
    Wizardry8Scalable(::getAppScale()),
    m_facts(f)
{
    QPixmap bgImg = makeDialogForm();

    m_bgImgSize = bgImg.size();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout factsScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    bgImg,                                                          this ),  -1,  NULL },

        { NO_ID,              QRect(  10,  32,  70,  12 ),    new WLabel( tr("Facts:"), Qt::AlignRight, 10, QFont::Thin,                     this ),  -1,  NULL },

        { FACTS_SCROLLBAR,    QRect( 504,  30,  15, 225 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },
        { FACTS_SCROLLLIST,   QRect(  88,  29, 402, 226 ),    new WListWidget(                                                               this ),  -1,  NULL },

        { NO_ID,              QRect( 462, 268,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               0, true, 1.0,   this ),  -1,  SLOT(apply(bool)) },
        { NO_ID,              QRect( 494, 268,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               4, true, 1.0,   this ),  -1,  SLOT(close()) },
    };

    int num_widgets = sizeof(factsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( factsScrn, num_widgets, this );


    // The fact list
    if (WListWidget *factlist = qobject_cast<WListWidget *>(m_widgets[ FACTS_SCROLLLIST ] ))
    {
        if (WScrollBar *sb = qobject_cast<WScrollBar *>(m_widgets[ FACTS_SCROLLBAR ] ))
        {
            // QScrollAreas include their own scrollbar when necessary, but we need to
            // override this since the Wizardry 8 look has the scrollbar outside the
            // scroll area and a short space away to the right

            factlist->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

            factlist->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
            factlist->setVerticalScrollBar( sb );
            // The call to setVerticalScrollBar reparents it, which prevents us drawing
            // it where we want, so reparent it (again) back to us
            sb->setParent( this );
            // return it to the same widget stack postion it was
            sb->stackUnder( m_widgets[ FACTS_SCROLLLIST ] );
        }
    }

    updateList();

    this->setMinimumSize( m_bgImgSize * m_scale );
    this->setMaximumSize( m_bgImgSize * m_scale );

    // Something in the OK button displeased the dialog layout, and made a
    // bigger than minimum dialog. We have to make an additional call to force
    // it back to the right size after adding the OK button.
    this->resize( m_bgImgSize * m_scale );
    show();
}

WindowFactEditor::~WindowFactEditor()
{
}

void WindowFactEditor::closeEvent(QCloseEvent *event)
{
    emit windowClosing();
    event->accept();
}

void WindowFactEditor::setScale(double scale)
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

void WindowFactEditor::resizeEvent(QResizeEvent *event)
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

QPixmap WindowFactEditor::makeDialogForm()
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
    p.drawPixmap(   6,   6, bgPtn,   0,   0, 140,  64 ); // facts slot
    p.drawPixmap(   6,   6, bgPtn,   0,   0,  82, 242 ); // most of the professions slot
    p.drawPixmap(   6,  bgImg.height() - bgPtn.height() + 124, bgPtn,   0, 110,  79, bgPtn.height() - 110);
    p.drawPixmap(   6, 145, bgPtn,   0, 95,  bgPtn.width(), 119 );       // LHS of the hole we made
    p.drawPixmap( 178, 145, bgPtn,   bgPtn.width()-200, 95,  200, 119 ); // RHS of the hole we made
    p.end();

    // And this is me being lazy, not wanting to have to redo all of the above from scratch for
    // a slightly wider dialog that otherwise matches the Patch dialog
    QPixmap beginAgain( QSize( customImage.width() + 150, customImage.height() ) );

    p.begin( &beginAgain );
    p.drawPixmap(   0,   0, customImage );
    p.drawPixmap( 250,   0, customImage, 100,   0, customImage.width() - 100, customImage.height() );
    p.end();

    return beginAgain;
}

void WindowFactEditor::updateList()
{
    // Repopulate the fact list
    if (WListWidget *factlist = qobject_cast<WListWidget *>(m_widgets[ FACTS_SCROLLLIST ] ))
    {
        factlist->clear();

        int num_items = m_facts.size();
        for (int k=0; k < num_items; k++)
        {
            QListWidgetItem *newFact = new QListWidgetItem( m_facts.getKey( k ) );
            newFact->setData( Qt::UserRole, k );
            newFact->setFlags(newFact->flags() | Qt::ItemIsUserCheckable); // set checkable flag

            if (m_facts.getValue( k ))
            {
                newFact->setCheckState( Qt::Checked );
            }
            else
            {
                newFact->setCheckState( Qt::Unchecked );
            }

            factlist->addItem( newFact );
        }
        factlist->setCurrentRow( 0 );
    }
}

void WindowFactEditor::apply(bool)
{
    if (WListWidget *factlist = qobject_cast<WListWidget *>(m_widgets[ FACTS_SCROLLLIST ] ))
    {
        int num_items = m_facts.size();
        for (int k=0; k < num_items; k++)
        {
            m_facts.setValue( k, (factlist->item(k)->checkState() == Qt::Checked) );
        }
    }
    close();
}
