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

#ifndef WTABLEITEM_H__
#define WTABLEITEM_H__

#include <QTableWidgetItem>

class WTableWidgetItem : public QTableWidgetItem
{
public:
    WTableWidgetItem(int type = Type) : QTableWidgetItem(type){}
    WTableWidgetItem(const QString &text, int type = Type) : QTableWidgetItem(text, type){}
    WTableWidgetItem(const QIcon &icon, const QString &text, int type = Type) : QTableWidgetItem(icon, text, type){}

    bool operator<(const QTableWidgetItem &other) const override
    {
        // All our table cells have QStrings in them, even the purely numeric ones
        // _Most_ of the numeric cells have suffixes. A few of them can have leading '+'s
        // as well.
        QString s1 = data(Qt::DisplayRole).toString();
        QString s2 = other.data(Qt::DisplayRole).toString();

        // We right align the numeric fields, so that's the best way to determine
        // what sort to use
        if (((textAlignment() & Qt::AlignHorizontal_Mask) != Qt::AlignRight) ||
            ((other.textAlignment() & Qt::AlignHorizontal_Mask) != Qt::AlignRight))
        {
            // String compare
            return (s1.compare( s2, Qt::CaseInsensitive ) < 0);
        }

        // toDouble(), toInt() can handle leading '+'s, but they can't handle suffixes.
        // So use the plain clib versions which can.
        QByteArray  qb1 = s1.toLatin1();
        QByteArray  qb2 = s2.toLatin1();
        char       *end1_ptr = NULL;
        char       *end2_ptr = NULL;

        double d1 = strtod( qb1.data(), &end1_ptr );
        double d2 = strtod( qb2.data(), &end2_ptr );

        if (s1.isEmpty() || (end1_ptr == qb1.data()))
            d1 = 0.0;
        if (s2.isEmpty() || (end2_ptr == qb2.data()))
            d2 = 0.0;

        return (d1 < d2);
    }
};

#endif // WTABLEITEM_H__
