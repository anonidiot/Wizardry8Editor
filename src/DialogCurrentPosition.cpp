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

#include "DialogCurrentPosition.h"
#include "Window3DNavigator.h"

#include "SLFFile.h"
#include "STItoQImage.h"
#include "Level.h"
#include "common.h"
#include "main.h"

#include <QApplication>
#include <QListWidgetItem>
#include <QPainter>
#include <QPixmap>

#include "WButton.h"
#include "WDDL.h"
#include "WImage.h"
#include "WLabel.h"
#include "WLineEdit.h"
#include "WListWidget.h"

typedef enum
{
    NO_ID,

    DDL_MAP,
    VAL_POS_X,
    VAL_POS_Y,
    VAL_POS_Z,
    VAL_HEADING,

    SIZE_WIDGET_IDS
} widget_ids;


DialogCurrentPosition::DialogCurrentPosition(int mapId, const float *position, float heading, QVector<qint32> visitedMaps, QWidget *parent)
    : Dialog(parent),
    m_map(mapId),
    m_heading(heading)
{
    m_position[0] = position[0];
    m_position[1] = position[1];
    m_position[2] = position[2];

    const int *levels = ::getLevels();

    // Go through the complete list of levels and try to open them. If a level exists, and has
    // been visited previously it is selectable in the DDL, otherwise it is greyed out if not
    // visited, and completely missing if nonexistant.
    for (int k=0; levels[k] != -1; k++)
    {
        m_maps << level( levels[k], getLevelName(levels[k]), visitedMaps.contains( levels[k] ) );
    }

    // Copy the visited maps out of the vector and store as a flat array with a -1 termination
    m_visitedMaps = new qint32[ visitedMaps.size() + 1 ];
    memcpy( m_visitedMaps, visitedMaps.data(), sizeof(qint32) * visitedMaps.size() );
    m_visitedMaps[ visitedMaps.size() ] = -1;

    QPixmap rowImg    = makeRowPixmap();
    QPixmap bgImg     = makeDialogForm();
    QSize   bgImgSize = bgImg.size();


    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    bgImg,                                                         this ),  -1,  NULL },

        { NO_ID,              QRect(  34,  75,  -1,  -1 ),    new WImage(    rowImg,                                                        this ),  -1,  NULL },
        { NO_ID,              QRect(  34,  89,  -1,  -1 ),    new WImage(    rowImg,                                                        this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 103,  -1,  -1 ),    new WImage(    rowImg,                                                        this ),  -1,  NULL },
        { NO_ID,              QRect(  34, 131,  -1,  -1 ),    new WImage(    rowImg,                                                        this ),  -1,  NULL },

        { NO_ID,              QRect(  44,  75,  55,  14 ),    new WLabel(    StringList::Position,        Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 100,  75,  30,  14 ),    new WLabel( "X:",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_POS_X,          QRect( 144,  75,  70,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(xChanged(const QString &)) },
        { NO_ID,              QRect( 100,  89,  30,  14 ),    new WLabel( "Y:",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_POS_Y,          QRect( 144,  89,  70,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(yChanged(const QString &)) },
        { NO_ID,              QRect( 100, 103,  30,  14 ),    new WLabel( "Z:",                           Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_POS_Z,          QRect( 144, 103,  70,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(zChanged(const QString &)) },

        { NO_ID,              QRect(  44, 131, 100,  14 ),    new WLabel(    ::getStringTable()->getString( StringList::Heading + StringList::APPEND_COLON ),  Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },
        { VAL_HEADING,        QRect( 144, 131,  70,  14 ),    new WLineEdit( "",                          Qt::AlignRight,  10, QFont::Thin, this ),  -1,  SLOT(headingChanged(const QString &)) },

        { NO_ID,              QRect( 239,  75,  -1,  -1 ),    new WButton(   "MAIN INTERFACE/ICONS_STANDARD.STI",          35, false, 1.0,  this ),  -1,  SLOT(openNavigator(bool)) },
        { NO_ID,              QRect( 207, 168,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",              0,  true, 1.0,  this ),  -1,  SLOT(accept()) },
        { NO_ID,              QRect( 239, 168,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",              4,  true, 1.0,  this ),  -1,  SLOT(reject()) },

        { NO_ID,              QRect(  34, 156, 150,  50 ),    new WLabel(    StringList::MapWarning,      Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },

        { NO_ID,              QRect(  10,  32,  50,  14 ),    new WLabel(    ::getStringTable()->getString( StringList::Map + StringList::APPEND_COLON ),  Qt::AlignRight, 10, QFont::Thin, this ),  -1,  NULL },
        { DDL_MAP,            QRect(  76,  25,  -1,  -1 ),    new WDDL(      "Lucida Calligraphy",        Qt::AlignLeft,  9, QFont::Thin,   this ),  -1,  SLOT(ddlChanged(int)) },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    // hide the Drop-down list until it is clicked on
    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_MAP ] ))
    {
        for (int k=0; k<m_maps.size(); k++)
        {
            QListWidgetItem *map = new QListWidgetItem( m_maps[k].name );
            map->setData( Qt::UserRole, m_maps[k].id );

            if (m_maps[k].isPreviouslyVisited)
                map->setFlags( map->flags() | (Qt::ItemIsSelectable | Qt::ItemIsEnabled) );
            else
                map->setFlags( map->flags() & ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled) );

            ddl->addItem( map );
        }

        for (int k=0; k<=m_maps.size(); k++)
        {
            if (m_maps[k].id == m_map)
            {
                ddl->setCurrentRow( k );
                break;
            }
        }
        ddl->updateList();
    }
    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_POS_X ]))
    {
        q->setText( QString::number( m_position[0] ) );
    }
    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_POS_Y ]))
    {
        q->setText( QString::number( m_position[1] ) );
    }
    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_POS_Z ]))
    {
        q->setText( QString::number( m_position[2] ) );
    }
    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_HEADING ]))
    {
        q->setText( QString::number( m_heading ) );
    }

    this->setMinimumSize( bgImgSize * m_scale );
    this->setMaximumSize( bgImgSize * m_scale );

    // Something in the OK button displeased the dialog layout, and made a
    // bigger than minimum dialog. We have to make an additional call to force
    // it back to the right size after adding the OK button.
    this->resize( bgImgSize * m_scale );
}

DialogCurrentPosition::~DialogCurrentPosition()
{
    delete[] m_visitedMaps;
}

void DialogCurrentPosition::ddlChanged(int value)
{
    if (sender() == m_widgets[ DDL_MAP ])
        m_map = value;
}

void DialogCurrentPosition::xChanged(const QString &value)
{
    (void)value;
    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_POS_X ]))
    {
        m_position[0] = q->text().toFloat();
    }
}

void DialogCurrentPosition::yChanged(const QString &value)
{
    (void)value;
    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_POS_Y ]))
    {
        m_position[1] = q->text().toFloat();
    }
}

void DialogCurrentPosition::zChanged(const QString &value)
{
    (void)value;
    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_POS_Z ]))
    {
        m_position[2] = q->text().toFloat();
    }
}

void DialogCurrentPosition::headingChanged(const QString &value)
{
    (void)value;
    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_HEADING ]))
    {
        m_heading = q->text().toFloat();
    }
}

QPixmap DialogCurrentPosition::makeDialogForm()
{
    // there is no suitable existing Wizardry image for the dialog we
    // want here, so we hack one together from a few different pieces
    QPixmap bgImg, bgFrame, bgDdl;

    // images used in dialog
    bgFrame = SLFFile::getPixmapFromSlf( "DIALOGS/POPUP_ITEMINFO.STI", 0 );
    bgImg   = SLFFile::getPixmapFromSlf( "DIALOGS/DIALOGBACKGROUND.STI", 0 );
    bgDdl   = SLFFile::getPixmapFromSlf( "CHAR GENERATION/CG_PROFESSION.STI", 0 );

    QPixmap customImage( QSize( 290, 215 ) );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, bgImg,                      0,                     0, bgImg.width(), 145 );
    p.drawPixmap( 240,   0, bgImg,    bgImg.width() - 180,                     0,           180, 145 );
    p.drawPixmap(   0, 125, bgImg,                      0,                     0, bgImg.width(), 145 );
    p.drawPixmap( 240, 125, bgImg,    bgImg.width() - 180,                     0,           180, 145 );

    p.drawPixmap(   0,   0, bgFrame,                    0,                     0,           290,  10 );
    p.drawPixmap(   0,   0, bgFrame,                    0,                     0,            10, 215 );
    p.drawPixmap(   0, 205, bgFrame,                    0, bgFrame.height() - 10,           290,  10 );
    p.drawPixmap( 280,   0, bgFrame, bgFrame.width() - 10,                     0,            10, 195 );
    p.drawPixmap( 280, 195, bgFrame, bgFrame.width() - 10, bgFrame.height() - 20,            10,  20 );

    p.drawPixmap(  71,  17, bgDdl,  15,   4, 200,  46 );

    p.end();
    return customImage;
}

QPixmap DialogCurrentPosition::makeRowPixmap()
{
    QPixmap rowImg;

    // images used in dialog
    rowImg = SLFFile::getPixmapFromSlf( "REVIEW/REVIEWSKILLSPAGE.STI", 3 );

    QPixmap customImage( 190, rowImg.height() );

    QPainter p;

    p.begin( &customImage );
    p.drawPixmap(   0,   0, rowImg,                    0,   0, rowImg.width(), rowImg.height() );
    p.drawPixmap( 110,   0, rowImg, rowImg.width() - 110,   0,             80, rowImg.height() );
    p.end();

    return customImage;
}

qint32 DialogCurrentPosition::getMapId() const
{
    return m_map;
}

void   DialogCurrentPosition::getPosition(float *x, float *y, float *z) const
{
    *x = m_position[0];
    *y = m_position[1];
    *z = m_position[2];
}

float  DialogCurrentPosition::getHeading() const
{
    return m_heading;
}

void  DialogCurrentPosition::openNavigator(bool /* checked */)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        bool loop;

        int   mapId   = m_map;
        float x       = m_position[0];
        float y       = m_position[1];
        float z       = m_position[2];
        float heading = m_heading;

        // One of the buttons in the Navigator allows for the loading of alternative maps.
        // Swapping to a new map entails throwing away virtually everything and starting again
        // And the lazy way of doing that is to destroy the entire class and start again
        do
        {
            // The constructor creates a new window and runs as a modal window -
            // it doesn't return until the window has been closed
            Window3DNavigator map( mapId, x, y, z, heading, true, m_visitedMaps );

            loop = false;
            switch (map.exec())
            {
                case Window3DNavigator::ChangeMap:
                    mapId = map.getMapId();
                    x = y = z = heading = NAN;
                    loop = true;
                    break;

                case Window3DNavigator::Rejected:
                    break;

                case Window3DNavigator::Accepted:
                {
                    m_map     = map.getMapId();
                    map.getPosition( &m_position[0], &m_position[1], &m_position[2]);
                    m_heading = map.getHeading();

                    if (WDDL *ddl = qobject_cast<WDDL *>(m_widgets[ DDL_MAP ] ))
                    {
                        for (int k=0; k<=m_maps.size(); k++)
                        {
                            if (m_maps[k].id == m_map)
                            {
                                ddl->setCurrentRow( k );
                                break;
                            }
                        }
                        ddl->updateList();
                    }

                    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_POS_X ]))
                    {
                        q->setText( QString::number( m_position[0] ) );
                    }
                    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_POS_Y ]))
                    {
                        q->setText( QString::number( m_position[1] ) );
                    }
                    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_POS_Z ]))
                    {
                        q->setText( QString::number( m_position[2] ) );
                    }
                    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ VAL_HEADING ]))
                    {
                        q->setText( QString::number( m_heading ) );
                    }
                }
            }
        }
        while (loop);

        q->setChecked(false);
    }
}
