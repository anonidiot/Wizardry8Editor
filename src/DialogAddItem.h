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

#ifndef DLGADDITEM_H
#define DLGADDITEM_H

#include "Dialog.h"
#include "Wizardry8Scalable.h"

#include "item.h"

class QListWidgetItem;

class DialogAddItem : public Dialog
{
    Q_OBJECT

public:
    DialogAddItem(int tag, QWidget *parent = nullptr);
    ~DialogAddItem();

signals:
    void itemAdded(int tag, item i);

public slots:
    void typeChanged(QListWidgetItem *now);
    void itemChanged(QListWidgetItem *now);

    void addNewItem(bool checked);

    void prevType(bool down);
    void nextType(bool down);
    void dropDownList(bool down);

    void mouseOverDropDownList(bool mouseover);

protected:
    void updateList();
    void updateMulti( item &i );

    void mouseOverLabel(bool on) override;

private:
    QPixmap    makeDialogForm();
    QPixmap    makeWider( QImage im, int width );
    void       makeTypePixmaps();

    QPixmap         m_ddlInactive;
    QPixmap         m_ddlActive;
    QPixmap         m_ddlTop;
    QPixmap         m_ddlMiddle;
    QPixmap         m_ddlBottom;

    QPixmap         m_typeIcon[static_cast<int>(item::type::Other) + 1];

    int             m_tag;
    item::type      m_type;
};

#endif

