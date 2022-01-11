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

#ifndef SCREENATTRIBS_H__
#define SCREENATTRIBS_H__

#include <QWidget>
#include <QPixmap>

#include "character.h"
#include "Screen.h"

class QHelpEvent;
class QListWidget;
class QListWidgetItem;

class ScreenAttribs : public Screen
{
    Q_OBJECT

public:
    ScreenAttribs(character *c, QWidget *parent = nullptr);
    ~ScreenAttribs();

    void        setVisible(bool visible) override;

public slots:
    void        spinnerChanged(int value);
    void        attributeDetail(bool checked);
    void        dropDownList(bool down);
    void        info(bool checked);

    void        mouseOverLabel(bool on) override;

    void        professionChanged(QListWidgetItem *);
    void        raceChanged(QListWidgetItem *);
    void        genderChanged(QListWidgetItem *);

    void        prevProf(bool);
    void        nextProf(bool);
    void        prevRace(bool);
    void        nextRace(bool);
    void        prevGender(bool);
    void        nextGender(bool);

private:
    void        resetScreen(void *char_tag, void *party_tag) override;
    int         changeListItem( int widgetId, int delta );

    QPixmap     makeRowPixmap();

    void        loadDDLPixmaps();
    void        updateLists();

    void        populateDDLProfessions(QListWidget *ddl);
    void        populateDDLRaces(QListWidget *ddl);
    void        populateDDLGenders(QListWidget *ddl);

    character  *m_char;
    bool        m_inspectMode;

    QPixmap     m_ddlInactive;
    QPixmap     m_ddlActive;
    QPixmap     m_ddlTop;
    QPixmap     m_ddlMiddle;
    QPixmap     m_ddlBottom;
};
#endif
