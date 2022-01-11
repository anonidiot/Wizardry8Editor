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

#include "WListWidget.h"

WListWidget::WListWidget(QWidget* parent)
    : QListWidget(parent),
    Wizardry8Scalable(1.0),
    m_fontName("Wizardry"),
    m_fontSize(10),
    m_fontWeight(QFont::Thin),
    m_alternateChecks(false)
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        setScale( w->getScale() );
    }
}

WListWidget::WListWidget(QString fontName, int pointSize, int weight, QWidget* parent)
    : QListWidget(parent),
    Wizardry8Scalable(1.0),
    m_fontName(fontName),
    m_fontSize(pointSize),
    m_fontWeight(weight),
    m_alternateChecks(false)
{
    if (Wizardry8Scalable *w = dynamic_cast<Wizardry8Scalable *>(parent))
    {
        setScale( w->getScale() );
    }
}

WListWidget::~WListWidget()
{
}

void WListWidget::setFontSize(int pointSize, int weight)
{
    m_fontSize   = pointSize;
    m_fontWeight = weight;
}

void WListWidget::setScale(double scale)
{
    if (! m_rect.isNull())
    {
        // if the base rect has been setup, then use it to
        // reposition _and_ resize the widget
        move(  (double)m_rect.x()     * scale, (double)m_rect.y()      * scale );
        resize((double)m_rect.width() * scale, (double)m_rect.height() * scale );
    }
    // Monotype Corsiva renders at almost half the size of Lucida Calligraphy
    // which we use it as a replacement for. Adjust the scale to compensate if
    // it is detected.
    // QFont::exactMatch() is NOT returning reliable results for the Lucida
    // Calligraphy font. It ALWAYS returns false, even when the font is in use. (QT 5.15.2)
    QFont text(m_fontName, m_fontSize * scale, m_fontWeight);
    QFontInfo fi(text);

    if (fi.family().compare("Monotype Corsiva") == 0)
    {
        text = QFont(m_fontName, m_fontSize * 1.5 * scale, m_fontWeight);
    }
    setFont(text);
    Wizardry8Scalable::setScale(scale);
}

void WListWidget::setTextColorInsteadOfCheckmarks( bool yes, QColor checked, QColor unchecked )
{
    m_alternateChecks = yes;
    if (yes)
    {
        m_checkedColor   = checked;
        m_uncheckedColor = unchecked;
    }
}

bool WListWidget::useTextColorInsteadOfCheckmarks() const
{
    return m_alternateChecks;
}
