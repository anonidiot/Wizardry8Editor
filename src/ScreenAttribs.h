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
class WDDL;

class ScreenAttribs : public Screen
{
    Q_OBJECT

public:
    ScreenAttribs(character *c, QWidget *parent = nullptr);
    ~ScreenAttribs();

    void        setVisible(bool visible) override;

signals:
    void        changedRace();
    void        changedProf();
    void        changedSex();

public slots:
    void        spinnerChanged(int value);
    void        attributeDetail(bool checked);
    void        info(bool checked);

    void        mouseOverLabel(bool on) override;

    void        ddlChanged(int value);
    void        ddlActive();

private:
    void        resetScreen(void *char_tag, void *party_tag) override;
    int         changeListItem( int widgetId, int delta );

    QPixmap     makeRowPixmap();

    void        updateLists();

    void        populateDDLProfessions(WDDL *ddl);
    void        populateDDLRaces(WDDL *ddl);
    void        populateDDLGenders(WDDL *ddl);

    character  *m_char;

    int         m_initialPopulate;

    bool        m_inspectMode;
};
#endif
