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

#ifndef SCREENITEMS_H__
#define SCREENITEMS_H__

#include <QWidget>
#include <QButtonGroup>
#include <QPixmap>
#include <QPushButton>

#include "Wizardry8Scalable.h"
#include "STItoQImage.h"

#include "character.h"
#include "party.h"
#include "item.h"
#include "Screen.h"

class QHelpEvent;
class QButtonGroup;
class WImage;
class WLabel;

class ScreenItems : public Screen
{
    Q_OBJECT

public:
    ScreenItems(party *p, character *c, QWidget *parent = nullptr);
    ~ScreenItems();

public slots:
    void acInfo(bool checked);
    void equipInfo(bool checked);
    void charInfo(bool checked);

    void scrolledPartyItems(int position);
    void itemButton(bool checked);
    void itemClick(bool checked);
    void itemSwap(bool checked);
    void filterButton(bool checked);
    void sortPartyItemsButton(bool checked);
    void setGold(bool checked);

    void goldChanged(const QString &);

    void itemDragged(item i);
    void itemDropped(item i);

    void inspectItem(item i);
    void editItem(item i);
    void addItem();
    void dropItem(item i);

    void itemAdded(int tag, item i);

    void      setVisible(bool visible) override;

protected:
    void      wheelEvent(QWheelEvent *event) override;
    void      resetScreen(void *char_tag, void *party_tag) override;

private:
    void      resetPartyItemsScrollbar();

    void      setItemAtLoc( item i, int item_loc );


    party            *m_party;
    character        *m_char;

    int               m_itemMode;

    int               m_partyItems_scrollPos;

    QButtonGroup     *m_infoSelect;
    QWidget          *m_common_screen;
};
#endif
