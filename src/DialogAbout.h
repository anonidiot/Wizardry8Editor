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

#ifndef DLGABOUT_H
#define DLGABOUT_H

#include <QPixmap>
#include <QTimer>

#include <Dialog.h>
#include "Wizardry8Scalable.h"

class DialogAbout : public Dialog
{
    Q_OBJECT

public:
    DialogAbout(QWidget *parent = nullptr);
    ~DialogAbout();

    static void  splitYUV( QPixmap sti_pix, QByteArray &y, QByteArray &u, QByteArray &v );

public slots:
    void coloriseImage();
    void scrollText();
    void downloadSourceCode(bool down);

protected:
    void  paintEvent(QPaintEvent *event);

private:
    QTimer      m_animateText;
    QPixmap     m_Image;
    int         m_pos;
    int         m_colorSteps;

    int         m_width;
    int         m_height;

    QByteArray  m_y1;
    QByteArray  m_u1;
    QByteArray  m_v1;

    QByteArray  m_y2;
    QByteArray  m_u2;
    QByteArray  m_v2;
};

#endif

