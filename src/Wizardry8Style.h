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

#ifndef WIZARDRY8STYLE_H__
#define WIZARDRY8STYLE_H__

#include <QProxyStyle>
#include <QPalette>

class STI;
class QSpinBox;
class QLineEdit;

QT_BEGIN_NAMESPACE
class QPainterPath;
QT_END_NAMESPACE

class Wizardry8Style : public QProxyStyle
{
    Q_OBJECT

public:
    Wizardry8Style();
    ~Wizardry8Style();

    QPalette standardPalette() const override;

    void polish(QWidget *widget) override;
    void unpolish(QWidget *widget) override;
    int pixelMetric(PixelMetric metric, const QStyleOption *option,
                    const QWidget *widget) const override;
    int styleHint(StyleHint hint, const QStyleOption *option,
                  const QWidget *widget, QStyleHintReturn *returnData) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget) const override;
    void drawControl(ControlElement control, const QStyleOption *option,
                     QPainter *painter, const QWidget *widget) const override;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                     QPainter *p, const QWidget *widget) const;
    QRect subControlRect(ComplexControl control, const QStyleOptionComplex *option,
                SubControl subControl, const QWidget *widget) const override;
    QSize sizeFromContents(ContentsType ct, const QStyleOption *opt,
                          const QSize &csz, const QWidget *widget) const;

    double getParentScale(const QWidget *widget) const;


private:
    mutable QPalette     m_standardPalette;

    STI *m_up_arrow;
    STI *m_down_arrow;
    STI *m_sbslider;
    STI *m_slider;
    STI *m_cb;
    STI *m_spinner;
};

class SpinBoxExtra : public QObject
{
    Q_OBJECT

public:
    SpinBoxExtra(QSpinBox *superControl);
    ~SpinBoxExtra();

public slots:
    void parentDestroyed() { deleteLater(); } // FIXME: don't know if this is necessary
    void valueChanged(int value);

private:
    QSpinBox  *m_super;
    QLineEdit *m_delta;
    int        m_initialValue;
    bool       m_inited;
};

#endif // WIZARDRY8STYLE_H__
