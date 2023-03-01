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

#ifndef WDDL_H__
#define WDDL_H__

#include <QMap>
#include <QPixmap>
#include <QString>
#include <QWidget>
#include "Wizardry8Scalable.h"

class QListWidgetItem;

class WDDL : public QWidget, public Wizardry8Scalable
{
    Q_OBJECT

public:
    WDDL(QString fontname, Qt::Alignment alignment, int pointSize, int weight, QWidget* parent = Q_NULLPTR);

    ~WDDL();

    int                getValue() { return changeListItem( 0 ); }

    void               addItem(QListWidgetItem *i);
    bool               setCurrentRow(int row);
    QListWidgetItem   *item(int idx);
    int                count();
    void               showDDL( bool show );

    void               updateList();

    void               setScale(double scale) override;

public slots:
    void               setEnabled(bool enabled);
//    void               setVisible(bool visible) override;

    void               mouseOverLabel(bool on);
    void               dropDownList(bool down);

    void               prev(bool);
    void               next(bool);
    void               ddlChanged(QListWidgetItem *);

signals:
    void               listActive();
    void               valueChanged(int value);

private:
    void               loadDDLPixmaps();

    int                changeListItem( int delta );

    QMap<int, QWidget *>   m_widgets;    
    
    QPixmap            m_ddlInactive;
    QPixmap            m_ddlActive;
    QPixmap            m_ddlTop;
    QPixmap            m_ddlBottom;
};

#endif // WDDL_H__
