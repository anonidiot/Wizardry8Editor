/*
 * Copyright (C) 2022-2023 Anonymous Idiot
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

#ifndef SCREENLEVELS_H__
#define SCREENLEVELS_H__

#include <QWidget>
#include <QMetaEnum>
#include <QPixmap>

#include "character.h"
#include "Screen.h"

class QPushButton;

class ScreenLevels : public Screen
{
    Q_OBJECT

public:
    ScreenLevels(character *c, QWidget *parent = nullptr);
    ~ScreenLevels();

signals:
    void        changedLevel();

public slots:
    void        spinnerChanged(int value);
    void        hpChanged(int value);
    void        staminaChanged(int value);
    void        spChanged(int value);

    void        poisonStrChanged(int value);
    void        profDetail(bool checked);
    void        info(bool checked);

    void        setPoisoned(int cb);
    void        setCb(int cb);

    void        portalEnable(int cb);

    void        lineeditChanged(const QString &);

    void        setXpLast(bool);
    void        setXpNow(bool);
    void        setXpNext(bool);
    void        setPosition(bool);

    void        fkButton(bool);

    void        openNavigator(bool checked);
    void        ddlChanged(int value);

protected:
    void        mouseOverLabel(bool on) override;

private:
    void        resetScreen(void *char_tag, void *party_tag) override;
    void        resetXP();
    void        resetHPStaminaSP();
    void        resetConditions();
    void        resetLevels();
    void        resetPortal();

    quint32     xpButton( QPushButton *q, int widget_id );

    QPixmap     makeRowPixmap();
    QPixmap     makeRowPixmap2();
    QPixmap     makeRowPixmap3();
    QPixmap     makeProfsBoxPixmap();
    QPixmap     makePortalBoxPixmap();
    QPixmap     makeHealthBoxPixmap();
    QPixmap     makeFakeButton();

    character  *m_char;
    bool        m_inspectMode;

    int         m_initialPopulate;

    QMetaEnum         m_metaProf;
};
#endif
