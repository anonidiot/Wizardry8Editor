/*
 * Copyright (C) 2022-2024 Anonymous Idiot
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

#ifndef WIMAGE_H__
#define WIMAGE_H__

#include <QPixmap>
#include <QPoint>
#include <QWidget>
#include "Wizardry8Scalable.h"

class STI;

class WImage : public QWidget, public Wizardry8Scalable
{
    Q_OBJECT

public:
    WImage(QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WImage(QPixmap &pix, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WImage(QString file, int image_idx, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WImage(QString file, int image_idx, int crop_left, int crop_top, int crop_width, int crop_height, double extraScale = 1.0, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WImage(QString file, int image_idx, QColor transparentColor, double extraScale = 1.0, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    WImage(int width, int height, QColor c, QWidget* parent = Q_NULLPTR, Qt::WindowFlags f = Qt::WindowFlags());
    ~WImage();

    void  setLengthRestrict(Qt::Alignment anchor, double widthPercent, double heightPercent);
    void  setScale(double scale) override;

    void  setFill(int width, int height, QColor c);
    void  setStiFile(QString sti_file, int image_idx, bool keep=false);
    void  setTargaFile(QString tga_file);
    void  setImageFile(QString img_file, const char *extension);
    void  setPixmap(QPixmap pixmap);
    QSize getPixmapSize() const;

public slots:
    void  animate();

signals:
    void  clicked(bool down);
    void  mouseOver(bool on);

    void  contextMenu( QPoint globalPosition );

protected:
    void  contextMenuEvent(QContextMenuEvent *event) override;

    void  paintEvent(QPaintEvent *event);

    void  mousePressEvent(QMouseEvent *event);
    void  mouseReleaseEvent(QMouseEvent *event);

    void  enterEvent(QEvent *event);
    void  leaveEvent(QEvent *event);


private:
    QPixmap        m_pixmap;
    Qt::Alignment  m_anchor;
    double         m_xScale;
    double         m_yScale;
    bool           m_mouseInLabel;
    bool           m_fakeInvisible;

    QByteArray     m_stiData;
    STI           *m_stiImages;
    int            m_frameIdx;

    QRect          m_crop;
};

#endif // WIMAGE_H__
