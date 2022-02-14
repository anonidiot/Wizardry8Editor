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

#ifndef WTABLE_H__
#define WTABLE_H__

#include <QByteArray>
#include <QDataStream>
#include <QStringList>
#include <QString>
#include <QMimeData>
#include <QModelIndex>
#include <QTableWidget>
#include <QTableWidgetItem>

// QTableWidget defines a method called item() which interferes with our
// class by the same name, so the class has to be referred to as ::item
// in this file
#include "item.h"

#define ITEM_MIME_TYPE     "application/x-wiz8-item"

class WTableWidget : public QTableWidget
{
public:
    WTableWidget(QWidget *parent = nullptr) : QTableWidget(parent){}
    WTableWidget(int rows, int columns, QWidget *parent = nullptr) : QTableWidget(rows, columns, parent){}

protected:
    QStringList mimeTypes() const override
    {
        QStringList mimeTypes;

        mimeTypes << ITEM_MIME_TYPE
                  << "text/plain"
                  << "text/html";

        return mimeTypes;
    }

    QMimeData *mimeData(const QList<QTableWidgetItem*> table_items) const override
    {
         QMimeData  *mimeData = new QMimeData();
         QByteArray  itemData;
         QDataStream dataStream(&itemData, QIODevice::WriteOnly);

         // Only a single item/row is allowed to be dragged
         if (table_items[0])
         {
             int item_idx = table_items[0]->data( Qt::UserRole ).toInt();
             ::item draggedItem( item_idx );

             dataStream << draggedItem;

             mimeData->setData( ITEM_MIME_TYPE, itemData );
             mimeData->setText( draggedItem.getName() );
             mimeData->setHtml( draggedItem.getCompleteData(true) );
         }
         // non-trivial to set the pixmap since the model sets that in a private
         // class. Would need to override that as well.

         return mimeData;
    }
};

#endif // WTABLE_H__
