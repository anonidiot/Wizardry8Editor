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

#include "DialogInfo.h"
#include "main.h"

#include <QTextEdit>

#include "WButton.h"
#include "WImage.h"
#include "WLabel.h"
#include "WScrollBar.h"

#include <QDebug>

typedef enum
{
    NO_ID,

    I_BG,

    TB_DESC,
    DESC_SCROLLBAR,

    SIZE_WIDGET_IDS
} widget_ids;

DialogInfo::DialogInfo(const QString &title, const QString &content, QWidget *parent)
    : Dialog(parent)
{
    init( title, content );
    show();
}

DialogInfo::DialogInfo(int stringNum, const QString &content, QWidget *parent)
    : Dialog(parent)
{
    init( ::getStringTable()->getString( stringNum ), content );
    show();
}

DialogInfo::~DialogInfo()
{
}

void DialogInfo::init(const QString &title, const QString &content)
{
    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { I_BG,               QRect(   0,   0,  -1,  -1 ),    new WImage(    "DIALOGS/POPUP_MONSTERINFO.STI",                0,              this ),  -1,  NULL },

        { TB_DESC,            QRect(  16,  38, 270, 186 ),    new QTextEdit(                                                                 this ),  -1,  NULL },
        { DESC_SCROLLBAR,     QRect( 300,  38,  15, 186 ),    new WScrollBar( Qt::Orientation::Vertical,                                     this ),  -1,  NULL },

        { NO_ID,              QRect( 282, 231,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",               0, true, 1.0,   this ),  -1,  SLOT(close()) },

        { NO_ID,              QRect(  16,  15, 286,  12 ),    new WLabel(    title,                        Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    // The textbox
    if (QTextEdit *tb = qobject_cast<QTextEdit *>(m_widgets[ TB_DESC ] ))
    {
        tb->setFont(QFont("Wizardry", 10 * m_scale, QFont::Thin));
        tb->setReadOnly(true);

        tb->setHtml( content );

        if (WScrollBar *sb = qobject_cast<WScrollBar *>(m_widgets[ DESC_SCROLLBAR ] ))
        {
            // QTextEdits include their own scrollbar when necessary, but we need to
            // override this since the Wizardry 8 look has the scrollbar outside the
            // textedit area and a short space away to the right
            tb->setVerticalScrollBar( sb );
            // The call to setVerticalScrollBar reparents it, which prevents us drawing
            // it where we want, so reparent it (again) back to us
            sb->setParent( this );
        }
    }

    if (WImage *w = qobject_cast<WImage *>(m_widgets[ I_BG ] ))
    {
        QSize bgImgSize = w->getPixmapSize();

        this->setMinimumSize( bgImgSize * m_scale );
        this->setMaximumSize( bgImgSize * m_scale );

        // Something in the OK button displeased the dialog layout, and made a
        // bigger than minimum dialog. We have to make an additional call to force
        // it back to the right size after adding the OK button.
        this->resize( bgImgSize * m_scale );
    }
}
