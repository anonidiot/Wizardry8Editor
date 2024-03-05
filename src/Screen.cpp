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

#include <QResizeEvent>

#include "Screen.h"
#include "main.h"

#include "WButton.h"
#include "WCheckBox.h"
#include "WDDL.h"
#include "WImage.h"
#include "WItem.h"
#include "WLabel.h"
#include "WLineEdit.h"
#include "WScrollBar.h"
#include "WSpinBox.h"

Screen::Screen(QWidget *parent) :
    QWidget(parent),
    Wizardry8Scalable(::getAppScale())
{
}

Screen::~Screen()
{
}

void Screen::resetScreen(void *, void *)
{
}

void Screen::setScale(double scale)
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

QSize Screen::minimumSizeHint() const
{
    return QSize( ORIGINAL_DIM_X, ORIGINAL_DIM_Y );
}

// Many QT components cache the initial response to this call, so
// they have a stale copy of it when the scale changes. invalidate()ing
// that cache doesn't actually work with widget hierarchies - they continue
// to use the cached value anyway, but we direct call this from MainWindow
// so _we_ at least will always get the right value
QSize Screen::sizeHint() const
{
    return QSize( m_scale * ORIGINAL_DIM_X, m_scale * ORIGINAL_DIM_Y );
}

void Screen::resizeEvent(QResizeEvent *event)
{
    double h_scale = (double)event->size().width()  / (double)ORIGINAL_DIM_X;
    double v_scale = (double)event->size().height() / (double)ORIGINAL_DIM_Y;

    if (h_scale < v_scale)
        m_scale = h_scale;
    else
        m_scale = v_scale;

    ::setAppScale( m_scale );

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

void Screen::setVisible( bool visible )
{
    QObjectList kids = children();

    for (int k=0; k<kids.size(); k++)
    {
        if (QWidget *q = qobject_cast<QWidget *>(kids.at(k)))
        {
            q->setVisible( visible );
        }
    }
    QWidget::setVisible( visible );
}

// static method
QMap<int, QWidget *> Screen::widgetInit( struct layout *widgets, int num_widgets, QWidget *parent )
{
    QMap<int, QWidget *> widgetsMap;

    widgetsMap.clear();
    for (int k=0; k<num_widgets; k++)
    {
        QWidget *w = widgets[k].widget;

        if (widgets[k].trigger)
        {
            if (WLabel *label = qobject_cast<WLabel *>(w))
            {
                connect( label, SIGNAL(clicked(bool)),   parent, widgets[k].trigger );
                connect( label, SIGNAL(mouseOver(bool)), parent, SLOT(mouseOverLabel(bool)) );
            }
            else if (WLineEdit *le = qobject_cast<WLineEdit *>(w))
            {
                connect( le, SIGNAL(textEdited(const QString&)), parent, widgets[k].trigger );
            }
            else if (WCheckBox *cb = qobject_cast<WCheckBox *>(w))
            {
                connect( cb, SIGNAL(stateChanged(int)), parent, widgets[k].trigger );
            }
            else if (WImage *im = qobject_cast<WImage *>(w))
            {
                connect( im, SIGNAL(clicked(bool)), parent, widgets[k].trigger );
            }
            else if (WItem *im = qobject_cast<WItem *>(w))
            {
                connect( im, SIGNAL(clicked(bool)),     parent, widgets[k].trigger );
            }
            else if (WButton *button = qobject_cast<WButton *>(w))
            {
                connect( button, SIGNAL(clicked(bool)), parent, widgets[k].trigger );
            }
            else if (WDDL *ddl = qobject_cast<WDDL *>(w))
            {
                connect( ddl, SIGNAL(valueChanged(int)), parent, widgets[k].trigger );
            }
            else if (WSpinBox *sb = qobject_cast<WSpinBox *>(w))
            {
                connect( sb, SIGNAL(valueChanged(int)), parent, widgets[k].trigger );
            }
            else if (WScrollBar *scroll_bar = qobject_cast<WScrollBar *>(w))
            {
                connect( scroll_bar, SIGNAL(valueChanged(int)), parent, widgets[k].trigger );
            }
        }

        // Common for all widgets
        if (w != NULL)
        {
            double sc = 0.0;

            if (Wizardry8Scalable *p = dynamic_cast<Wizardry8Scalable *>(parent))
            {
                sc = p->getScale();
            }
            else
            {
                sc = ::getAppScale();
            }

            if (Wizardry8Scalable *s = dynamic_cast<Wizardry8Scalable *>(w))
            {
                s->setBaseRect( widgets[k].pos );
                s->setScale( sc );
            }
            else
            {
                w->move( widgets[k].pos.topLeft() * sc );
                w->resize( widgets[k].pos.size() * sc );
            }

            if (widgets[k].toolTip != -1)
            {
                w->setToolTip( ::getBaseStringTable()->getString( widgets[k].toolTip ) );
            }

            if (widgets[k].id != 0) // NO_ID should always be 0
            {
                widgetsMap.insert( widgets[k].id, w );
            }
        }
    }
    return widgetsMap;
}

void Screen::mouseOverLabel(bool on)
{
    if (WLabel *q = qobject_cast<WLabel *>(this->sender()))
    {
        if (on)
            q->setStyleSheet("QLabel {color: #169e16}"); // Green
        else
            q->setStyleSheet("QLabel {color: #e0e0c3}"); // Light yellow
    }
}
