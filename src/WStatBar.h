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

#ifndef WSTATBAR_H__
#define WSTATBAR_H__

#include <QPixmap>
#include <QWidget>
#include "Wizardry8Scalable.h"


class WStatBar : public QWidget, public Wizardry8Scalable
{
    Q_OBJECT

public:
    WStatBar(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WStatBar(bool showCurrent, Qt::Alignment alignment, int pointSize, int weight, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~WStatBar();

    void setFontSize(int pointSize, int weight = -1);
    void setValue(int base, int current, int range);
    void setScale(double scale) override;

signals:
    void  clicked(bool down);
    void  mouseOver(bool on);

protected:
    void  paintEvent(QPaintEvent *event);

    void  mousePressEvent(QMouseEvent *event);
    void  mouseReleaseEvent(QMouseEvent *event);

    void  enterEvent(QEvent *event);
    void  leaveEvent(QEvent *event);


private:
    void loadPixmaps();

    bool           m_showCurrent;
    int            m_fontSize;
    int            m_fontWeight;
    int            m_alignment;
    bool           m_mouseInLabel;

    int            m_base;
    int            m_current;
    int            m_range;
};

#endif // WIMAGE_H__
