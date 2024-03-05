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

#include "DialogNewFile.h"
#include "SLFFile.h"
#include "main.h"

#include <QApplication>
#include <QButtonGroup>
#include <QPainter>
#include <QPixmap>

#include "WButton.h"
#include "WCheckBox.h"
#include "WImage.h"
#include "WLabel.h"

#include <QDebug>

typedef enum
{
    NO_ID,

    LBL_WARNING,

    SCENARIO_START,
    CB_SC1 = SCENARIO_START,
    CB_SC2,
    CB_SC3,
    CB_SC4,
    SCENARIO_END,

    CB_RODAN,
    CB_BARLONE,

    SIZE_WIDGET_IDS
} widget_ids;


DialogNewFile::DialogNewFile(QWidget *parent)
    : Dialog(parent)
{
    QPixmap bgImg = makeDialogForm();
    QSize   bgImgSize = bgImg.size();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    bgImg,                                                          this ),  -1,  NULL },

        { NO_ID,              QRect(  10,  30,  70,  12 ),    new WLabel(    ::getBaseStringTable()->getString( StringList::Scenario + StringList::APPEND_COLON ),  Qt::AlignRight, 10, QFont::Thin, this ),  -1,  NULL },

        { CB_SC1,             QRect( 100,  30, 227,  13 ),    new WCheckBox( StringList::W7EndUmpani,                                        this ),  -1,  NULL },
        { CB_SC2,             QRect( 100,  47, 227,  13 ),    new WCheckBox( StringList::W7EndTRang,                                         this ),  -1,  NULL },
        { CB_SC3,             QRect( 100,  64, 227,  13 ),    new WCheckBox( StringList::W7EndOwnShip,                                       this ),  -1,  NULL },
        { CB_SC4,             QRect( 100,  81, 227,  13 ),    new WCheckBox( StringList::W8Virgin,                                           this ),  -1,  NULL },

        { CB_RODAN,           QRect(  30, 120, 300,  13 ),    new WCheckBox( StringList::W7RodanDead,                                        this ),  StringList::W7RodanDeadHelp,  NULL },
        { CB_BARLONE,         QRect(  30, 137, 300,  13 ),    new WCheckBox( StringList::W7BarloneDead,                                      this ),  StringList::W7BarloneDeadHelp,  NULL },

        { LBL_WARNING,        QRect(  20, 180, 344,  70 ),    new WLabel(    ::getBaseStringTable()->getString( StringList::WarningNewFile ),    Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 312, 268,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               0, true, 1.0,   this ),  -1,  SLOT(accept()) },
        { NO_ID,              QRect( 344, 268,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               4, true, 1.0,   this ),  -1,  SLOT(reject()) },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    m_scenario_cb_group = new QButtonGroup(this);
    for (int k=SCENARIO_START; k < SCENARIO_END; k++)
    {
        m_scenario_cb_group->addButton( qobject_cast<QAbstractButton *>(m_widgets[ k ]), k );
    }
    // Set W8 Virgin as the default
    if (QCheckBox *q = qobject_cast<QCheckBox *>(m_widgets[ CB_SC4 ]))
    {
        q->setCheckState( Qt::Checked );
    }

    this->setMinimumSize( bgImgSize * m_scale );
    this->setMaximumSize( bgImgSize * m_scale );

    // Something in the OK button displeased the dialog layout, and made a
    // bigger than minimum dialog. We have to make an additional call to force
    // it back to the right size after adding the OK button.
    this->resize( bgImgSize * m_scale );
}

DialogNewFile::~DialogNewFile()
{
    delete m_scenario_cb_group;
}

void DialogNewFile::setSaveAsResetDialog()
{
    // Change the warning to be applicable to the Save as Reset Game... dialog
    // instead of the New File... dialog

    if (QLabel *q = qobject_cast<QLabel *>(m_widgets[ LBL_WARNING ]))
    {
        q->setText( ::getBaseStringTable()->getString( StringList::WarningSaveReset ) );
    }
}

QPixmap DialogNewFile::makeDialogForm()
{
    // there is no suitable existing Wizardry image for the dialog we
    // want here, so we hack one together from a few different pieces
    QPixmap bgImg,     bgPtn;

    // images used in dialog
    bgImg = SLFFile::getPixmapFromSlf( "DIALOGS/POPUP_ITEMINFO.STI", 0 );
    bgPtn = SLFFile::getPixmapFromSlf( "DIALOGS/DIALOGBACKGROUND.STI", 0 );

    QPixmap customImage( QSize( bgImg.width(), bgImg.height() + 20 ) );

    QPainter p;

    p.begin( &customImage );
    // First make it a little taller, by 20 rows and squeeze the hole for
    // the scrollarea and the buttonbox over 20 pixels to the right
    p.drawPixmap(   0,   0, bgImg,   0,   0, bgImg.width(), 242 );       // Top
    p.drawPixmap(  26,  69, bgImg,   6,  69, 66, 171 );                      // Shrunk scrollpane
    p.drawPixmap(   0, 177, bgImg,   0, 157,  6, bgImg.height() - 157 ); // LHS bevel
    p.drawPixmap( 378, 177, bgImg, 378, 157, bgImg.width() - 378, bgImg.height() - 157 ); // RHS bevel
    p.drawPixmap(   0, 262, bgImg,   0, 242, bgImg.width(), bgImg.height() - 242 ); // Bottom
    p.drawPixmap(  26, 262, bgImg,   6, 242, 66, bgImg.height() - 248 ); // Shrunk buttonbox
    // Now fill in the slot where the professions/races went, the slot for the icon, and the
    // empty space we just left in the middle.
    p.drawPixmap(   6,   6, bgPtn,   0,   0, 140,  64 ); // patches slot
    p.drawPixmap(   6,   6, bgPtn,   0,   0,  82, 242 ); // most of the professions slot
    p.drawPixmap(   6,  bgImg.height() - bgPtn.height() + 124, bgPtn,   0, 110,  79, bgPtn.height() - 110);
    p.drawPixmap(   6, 145, bgPtn,   0, 95,  bgPtn.width(), 119 );       // LHS of the hole we made
    p.drawPixmap( 178, 145, bgPtn,   bgPtn.width()-200, 95,  200, 119 ); // RHS of the hole we made

    p.drawPixmap(  60,  70, bgPtn,   0,   0, 256,  80 );
    p.drawPixmap( 265,  70, bgPtn,   0,   0, 110,  80 );

    p.drawPixmap(  15, 175, bgImg,   65, 245, 309, 29 ); // Box around warning
    p.drawPixmap(  67, 175, bgImg,   70, 245, 304, 29 );
    p.drawPixmap(  15, 200, bgImg,   65, 250, 309, 24 );
    p.drawPixmap(  67, 200, bgImg,   70, 250, 304, 24 );
    p.drawPixmap(  15, 220, bgImg,   65, 250, 309, 24 );
    p.drawPixmap(  67, 220, bgImg,   70, 250, 304, 24 );
    p.drawPixmap(  15, 232, bgImg,   65, 250, 309, 25 );
    p.drawPixmap(  67, 232, bgImg,   70, 250, 304, 25 );
    p.end();

    return customImage;
}

int DialogNewFile::getScenarioIndex()
{
    switch (m_scenario_cb_group->checkedId())
    {
        case CB_SC1: return StringList::W7EndUmpani;
        case CB_SC2: return StringList::W7EndTRang;
        case CB_SC3: return StringList::W7EndOwnShip;
        case CB_SC4: return StringList::W8Virgin;
    }
    return -1;
}

bool DialogNewFile::isBarloneDead()
{
    return qobject_cast<QAbstractButton *>(m_widgets[ CB_BARLONE ])->isChecked();
}

bool DialogNewFile::isRodanDead()
{
    return qobject_cast<QAbstractButton *>(m_widgets[ CB_RODAN ])->isChecked();
}
