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

#ifndef WITEM_H__
#define WITEM_H__

#include "item.h"
#include <QPixmap>
#include <QWidget>
#include "Wizardry8Scalable.h"

class WItem : public QWidget, public Wizardry8Scalable
{
    Q_OBJECT

public:
    enum class context_mode
    {
        All,
        NoDrops,
        None
    };
    WItem(context_mode contexts = context_mode::All, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WItem(const item &i, context_mode contexts = context_mode::All, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~WItem();

    void        setItem(const item &i);
    const item &getItem();
    void        setUsabilityBasedOnCharacter(const character &c);
    void        resetPixmaps();
    void        setScale(double scale) override;

signals:
    void        clicked(bool down);
    void        itemDragged( item i );
    void        itemDropped( item i );

    void        inspectItem( item i );
    void        editItem( item i );
    void        addItem();
    void        dropItem( item i );

private:
    void        createContextMenuActions(context_mode contexts);

private slots:
    void        cmInspectItem();
    void        cmEditItem();
    void        cmAddItem();
    void        cmDropItem();

protected:
    void        contextMenuEvent(QContextMenuEvent *event) override;
    void        paintEvent(QPaintEvent *event);

    void        mouseMoveEvent(QMouseEvent *event) override;
    void        mousePressEvent(QMouseEvent *event) override;
    void        mouseReleaseEvent(QMouseEvent *event) override;

    void        dragEnterEvent(QDragEnterEvent *event) override;
    void        dragMoveEvent(QDragMoveEvent *event) override;
    void        dropEvent(QDropEvent *event) override;

private:
    item             m_item;
    bool             m_usable;
    QPixmap          m_itemPixmap;
    QPixmap          m_usablePixmap;
    double           m_fontSize;

    QPoint           m_beginDragPosition;

    QAction         *m_cmInspectItem;
    QAction         *m_cmEditItem;
    QAction         *m_cmAddItem;
    QAction         *m_cmDropItem;
};

#endif // WITEM_H__
