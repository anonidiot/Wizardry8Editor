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

#ifndef WSPINBOX_H__
#define WSPINBOX_H__

#include <QSpinBox>
#include <QWidget>
#include "Wizardry8Scalable.h"

class WSpinBox : public QSpinBox, public Wizardry8Scalable
{
    Q_OBJECT

public:
    WSpinBox(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WSpinBox(int value, int baseValue, int min, int max, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WSpinBox(int value, int min, int max, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~WSpinBox();

    void setFontSize(int pointSize, int weight = -1);
    void setScale(double scale) override;
    // C++ won't let us name this method the same as the base class, even if it has a
    // different signature. If we do so it will mask the base method from all callers.
    void setValueEx(int value, int baseValue);

    int    m_initialValue;

private:
    int    m_fontSize;
    int    m_fontWeight;
};

#endif // WSPINBOX_H__
