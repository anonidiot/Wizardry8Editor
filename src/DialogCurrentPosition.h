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

#ifndef DLGCURRPOS_H
#define DLGCURRPOS_H

#include "Dialog.h"
#include "Wizardry8Scalable.h"

#include "item.h"

class QListWidgetItem;

struct level
{
    level(int i, QString s, bool v)
         : id(i), name(s), isPreviouslyVisited(v)
    {
    }

    int      id;
    QString  name;
    bool     isPreviouslyVisited;
};

class DialogCurrentPosition : public Dialog
{
    Q_OBJECT

public:
    DialogCurrentPosition(int mapId, const float *position, float heading, QVector<qint32> visitedMaps, QWidget *parent = nullptr);

    ~DialogCurrentPosition();

    qint32      getMapId() const;
    void        getPosition(float *x, float *y, float *z) const;
    float       getHeading() const;

public slots:
    void        openNavigator(bool checked);

    void        ddlChanged(int value);
    void        xChanged(const QString &value);
    void        yChanged(const QString &value);
    void        zChanged(const QString &value);
    void        headingChanged(const QString &value);

private:
    QPixmap    makeDialogForm();
    QPixmap    makeRowPixmap();

    qint32         *m_visitedMaps;
    QList<level>    m_maps;

    qint32          m_map;
    float           m_position[3];
    float           m_heading;
};

#endif

