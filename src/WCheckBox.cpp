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

#include "WCheckBox.h"
#include "StringList.h"

#include "main.h"

WCheckBox::WCheckBox(const QString &text, QWidget* parent)
    : QCheckBox(text, parent),
    Wizardry8Scalable(1.0)
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        setScale( w->getScale() );
    }
}

WCheckBox::WCheckBox(int stringNum, QWidget* parent)
    : WCheckBox("", parent)
{
    setText( ::getStringTable()->getString( stringNum ) );
}

WCheckBox::~WCheckBox()
{
}

void WCheckBox::setScale(double scale)
{
    if (! m_rect.isNull())
    {
        // if the base rect has been setup, then use it to
        // reposition _and_ resize the widget
        move(  (double)m_rect.x()     * scale, (double)m_rect.y()      * scale );
        resize((double)m_rect.width() * scale, (double)m_rect.height() * scale );
    }
    setFont(QFont("Wizardry", 10 * scale, QFont::Thin));
    Wizardry8Scalable::setScale(scale);
}
