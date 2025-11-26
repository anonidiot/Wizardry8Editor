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

#ifndef WBUTTON_H__
#define WBUTTON_H__

#include <QPixmap>
#include <QPoint>
#include <QString>

#include <QPushButton>
#include <QWidget>
#include "Wizardry8Scalable.h"

class WButton : public QPushButton, public Wizardry8Scalable
{
    Q_OBJECT

public:
    static const int EMPTY_FLAG = 256;

    WButton(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WButton(QString sti_file, int image_idx, bool bNoFifthState = false, double extraScale = 1.0, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WButton(QPixmap *pixmap, bool bNoFifthState = false, double extraScale = 1.0, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

    ~WButton();

    void setScale(double scale) override;

    void setDraggable( QString mimeType, int tag );

    int  getTag() { return m_tag & 0xff; }


signals:
    void        buttonDragged( int tag );
    void        buttonDropped( int tag );

    void        contextMenu( QPoint globalPosition );

protected:
    void        contextMenuEvent(QContextMenuEvent *event) override;

    void        mouseMoveEvent(QMouseEvent *event) override;
    void        mousePressEvent(QMouseEvent *event) override;
    void        mouseReleaseEvent(QMouseEvent *event) override;

    void        dragEnterEvent(QDragEnterEvent *event) override;
    void        dragMoveEvent(QDragMoveEvent *event) override;
    void        dropEvent(QDropEvent *event) override;


private:
    bool       m_draggable;
    QString    m_mimeType;
    QPixmap    m_pixmap;
    int        m_tag;

    QPoint     m_beginDragPosition;
};

#endif // WBUTTON_H__
