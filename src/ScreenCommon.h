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

#ifndef SCREENCOMMON_H__
#define SCREENCOMMON_H__

#include <QMap>
#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QWidget>

#include "party.h"
#include "character.h"
#include "item.h"
#include "Wizardry8Scalable.h"
#include "Screen.h"

class QButtonGroup;

class ScreenCommon : public Screen
{
    Q_OBJECT

public:
    ScreenCommon(party *p, int charIdx, QWidget *parent = nullptr);
    ~ScreenCommon();

    void             updateChars(party *p);

    void             setVisible(bool visible) override;

    static QPixmap   getSmallPortrait(int portraitIndex);
    static QPixmap   getMediumPortrait(int portraitIndex);
    static QPixmap   getLargePortrait(int portraitIndex);
    static QPixmap   getStatue( character::race r, character::gender g );

signals:
    void partyEmpty();
    void partyViable();
    void exit();

public slots:
    void charswap(bool checked);    // new character in party selected
    void professionLevels(bool down);

    void changeName(bool down);     // goto Personality and Voice screen
    void changedName(QString name); // name changed by Personality and Voice screen
    void changedPortrait();         // portrait changed by Personality and Voice screen

    void reviewAttribs(bool down);   // goto ScreenAttribs
    void reviewItems(bool down);     // goto ScreenItems
    void reviewLevels(bool down);    // goto ScreenLevels
    void reviewMagic(bool down);     // goto ScreenMagic
    void reviewSkills(bool down);    // goto ScreenSkills
    void exitButton(bool checked);   // exit

    // reordering characters by drag and drop
    void characterMoved(int tag);

    // popup menu on character
    void characterPopup(QPoint point);
    void cmImportChar();
    void cmExportChar();
    void cmDropChar();
    void cmCopyChar();
    void cmPasteChar();

protected:
    void        resetScreen(void *char_tag, void *party_tag) override;
    void        resetCharacterSelectButtons(void);

private:
    QPixmap     makeAttribsButton(int state);

    void        setSumWeapon( character::worn weapon );
    void        showLevelsOverlay( bool show );

    character        *m_copiedChar;
    party            *m_party;
    int               m_charIdx;
    int               m_cmTargetCharIdx;

    QButtonGroup     *m_charSelect;
    QButtonGroup     *m_pageSelect;

    Screen           *m_currentScreen;

    QAction          *m_cmImportChar;
    QAction          *m_cmExportChar;
    QAction          *m_cmDropChar;
    QAction          *m_cmCopyChar;
    QAction          *m_cmPasteChar;
};
#endif
