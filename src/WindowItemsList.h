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

#ifndef WNDITEMSLST_H
#define WNDITEMSLST_H

#include <QWidget>
#include <QList>
#include <QMap>
#include <QPixmap>
#include <QString>

#include "character.h"

#include "DialogChooseColumns.h"
#include "Wizardry8Scalable.h"

class QListWidget;
class QListWidgetItem;
class QMenu;

class WindowItemsList : public QWidget, public Wizardry8Scalable
{
    Q_OBJECT

public:
    WindowItemsList(character::profession profession, character::race race, character::gender gender);
    ~WindowItemsList();

    void        setVisible(bool visible) override;

    void        setScale(double scale) override;

signals:
    void        windowClosing();


public slots:
    void        mouseOverLabel(bool on);

    void        dropDownList(bool down);

    void        filterProf(int);
    void        filterRace(int);
    void        filterSex(int);

    void        professionChanged(QListWidgetItem *);
    void        raceChanged(QListWidgetItem *);
    void        genderChanged(QListWidgetItem *);

    void        prevProf(bool);
    void        nextProf(bool);
    void        prevRace(bool);
    void        nextRace(bool);
    void        prevGender(bool);
    void        nextGender(bool);

    void        tableMenu(QPoint pos);
    void        chooseColumns();

protected:
    void        closeEvent(QCloseEvent *event) override;
    void        resizeEvent(QResizeEvent *event) override;

private:
    int         changeListItem( int widgetId, int delta );
    QString     lookupItemProperty( item *i, DialogChooseColumns::column col, bool *numeric );

    QList<DialogChooseColumns::column> loadColumnsFromRegistry();

    void        populateColumns();

    void        loadDDLPixmaps();
    void        updateLists();
    void        updateFilter();

    void        populateDDLProfessions(QListWidget *ddl);
    void        populateDDLRaces(QListWidget *ddl);
    void        populateDDLGenders(QListWidget *ddl);

    QPixmap     makeDialogForm();

    character::profession  m_prof_filter;
    character::race        m_race_filter;
    character::gender      m_gender_filter;

    QPixmap     m_ddlInactive;
    QPixmap     m_ddlActive;
    QPixmap     m_ddlTop;
    QPixmap     m_ddlMiddle;
    QPixmap     m_ddlBottom;

    QPixmap     m_bgImg;
    QMap<int, QWidget *>   m_widgets;

    QMenu      *m_contextMenu;
};

#endif
