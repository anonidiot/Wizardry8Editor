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

#ifndef WLABEL_H__
#define WLABEL_H__

#include <QLabel>
#include <QVariant>
#include <QWidget>
#include "Wizardry8Scalable.h"

class WLabel : public QLabel, public Wizardry8Scalable
{
    Q_OBJECT

public:
    WLabel(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

    WLabel(QString text, Qt::Alignment alignment, int pointSize, int weight, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WLabel(int stringNum, Qt::Alignment alignment, int pointSize, int weight, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

    WLabel(QString text, QString fontname, Qt::Alignment alignment, int pointSize, int weight, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WLabel(int stringNum, QString fontname, Qt::Alignment alignment, int pointSize, int weight, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());

    ~WLabel();

    void setFont(QString fontname, int pointSize, int weight = -1);
    void setFontSize(int pointSize, int weight = -1);
    void setScale(double scale) override;

    void     setData( QVariant v ) { m_data = v; }
    QVariant data()                { return m_data; }

signals:
    void clicked(bool down);
    void mouseOver(bool on);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);

private:
    QVariant m_data;

    QString  m_fontName;
    int      m_fontSize;
    int      m_fontWeight;

    bool     m_mouseInLabel;
};

#endif // WLABEL_H__
