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

#include <QDebug>
#include "WSpinBox.h"

// If you're wondering why there is an obvious hack in the style to embed the
// delta textfield there instead of in this widget here it is because the original
// goal was to not need these subclassed Widgets at all - ie. to be able to use
// standard QWidgets and have the style do _all_ the work for rendering it in the
// Wizardry style. That might have been mostly possible if I hadn't also decided
// I wanted to mess with the window scaling and use it to scale all the widgets
// every time the window changed - and that did require subclassing. And once the
// subclasses were started they tended to keep growing.
// In the case of the spinbox it should still be possible to use a QSpinBox and
// have it get rendered like a Wizardry8 widget - you just lose the ability to
// reliably reset the delta value if you don't inherit from this class instead.

WSpinBox::WSpinBox(QWidget* parent, Qt::WindowFlags)
    : QSpinBox(parent),
    Wizardry8Scalable(1.0),
    m_initialValue(0)
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        Wizardry8Scalable::setScale( w->getScale() );
    }
}

WSpinBox::WSpinBox(int value, int initialValue, int min, int max, QWidget* parent, Qt::WindowFlags)
    : QSpinBox(parent),
    Wizardry8Scalable(1.0),
    m_initialValue(initialValue),
    m_fontSize(10),
    m_fontWeight(QFont::Thin)
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        setScale( w->getScale() );
    }
    setRange( min, max );
    setValue( value );
    setAlignment( Qt::AlignRight );
}

WSpinBox::WSpinBox(int value, int min, int max, QWidget* parent, Qt::WindowFlags)
    : QSpinBox(parent),
    Wizardry8Scalable(1.0),
    m_initialValue(value),
    m_fontSize(10),
    m_fontWeight(QFont::Thin)
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        setScale( w->getScale() );
    }
    setRange( min, max );
    setValue( value );
    setAlignment( Qt::AlignRight );
}

WSpinBox::~WSpinBox()
{
}

void WSpinBox::setValueEx(int value, int initialValue)
{
    m_initialValue = initialValue;
    setValue( value );
    // if we're resetting the initialValue we always have to emit the
    // valueChanged signal, because if the value didn't actually
    // change from the previous value the base class won't fire
    // it, and the deltabox won't update until the _next_ change.
    emit valueChanged( value );
}

void WSpinBox::setFontSize(int pointSize, int weight)
{
    m_fontSize   = pointSize;
    m_fontWeight = weight;
}

void WSpinBox::setScale(double scale)
{
    if (! m_rect.isNull())
    {
        // if the base rect has been setup, then use it to
        // reposition _and_ resize the widget
        move(  (double)m_rect.x()     * scale, (double)m_rect.y()      * scale );
        resize((double)m_rect.width() * scale, (double)m_rect.height() * scale );
    }
    setFont(QFont("Wizardry", m_fontSize * scale, m_fontWeight));
    Wizardry8Scalable::setScale(scale);
}
