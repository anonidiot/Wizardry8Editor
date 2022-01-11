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
#include "WLineEdit.h"

WLineEdit::WLineEdit(QWidget* parent, Qt::WindowFlags)
    : QLineEdit(parent),
    Wizardry8Scalable(1.0)
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        Wizardry8Scalable::setScale( w->getScale() );
    }
}

WLineEdit::WLineEdit(QString text, Qt::Alignment alignment, int pointSize, int weight, QWidget* parent, Qt::WindowFlags)
    : QLineEdit(parent),
    Wizardry8Scalable(1.0),
    m_fontSize(pointSize),
    m_fontWeight(weight)
{
    setAlignment( alignment );
    setText( text );
    setFontSize( pointSize, weight );

    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        setScale( w->getScale() );
    }
}

WLineEdit::~WLineEdit()
{
}

void WLineEdit::setFontSize(int pointSize, int weight)
{
    m_fontSize   = pointSize;
    m_fontWeight = weight;
}

void WLineEdit::setScale(double scale)
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
