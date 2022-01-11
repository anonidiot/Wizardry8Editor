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

#ifndef SCREEN_H__
#define SCREEN_H__

#include <QMap>
#include <QRect>
#include <QSize>
#include <QWidget>

#include "item.h"
#include "Wizardry8Scalable.h"

struct layout
{
    int           id;
    QRect         pos;
    QWidget      *widget;
    int           toolTip;
    char         *trigger;
};

class Screen : public QWidget, public Wizardry8Scalable
{
    Q_OBJECT

public:
    Screen(QWidget *parent = nullptr);
    ~Screen();

    static QMap<int, QWidget *>    widgetInit( struct layout *widgets, int num_widgets, QWidget * );

    void             setVisible(bool visible) override;
    void             setScale(double scale) override;

    virtual void     resetScreen(void *char_tag, void *party_tag);

public slots:
    virtual void mouseOverLabel(bool on);

protected:
    QSize       minimumSizeHint() const override;
    QSize       sizeHint() const override;
    void        resizeEvent(QResizeEvent *event) override;

    QMap<int, QWidget *>   m_widgets;
};
#endif
