// FIXME: The app restart code works once to change to 1.2.4 and then once to change back to 1.2.8, but then exits after the parallel world dialog before opening a file

/*
 * Copyright (C) 2025 Anonymous Idiot
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

#include "DialogPreferences.h"
#include "SLFFile.h"
#include "common.h"
#include "main.h"

#include <QApplication>
#include <QFileDialog>
#include <QPainter>
#include <QPixmap>
#include <QSettings>

#include "WButton.h"
#include "WCheckBox.h"
#include "WImage.h"
#include "WLabel.h"
#include "WLineEdit.h"

#include <QDebug>

// TODO: Have to account for every individual warning you add here also

QStringList individual_warnings = { "SuppressWarningRPC" };

typedef enum
{
    NO_ID,

    VAL_FULL_PATH,
    VAL_ISO_LANG,
    VAL_CHARSET,

    CB_SUPPRESS_WARNINGS,

    SIZE_WIDGET_IDS
} widget_ids;


DialogPreferences::DialogPreferences(QWidget *parent)
    : Dialog(parent),
    m_wizardryPathChanged(false)
{
    init();

    show();
}

DialogPreferences::~DialogPreferences()
{
}

void DialogPreferences::init()
{
    QPixmap bgImg = makeDialogForm();
    QSize   bgImgSize = bgImg.size();

    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,                QRect(   0,   0,  -1,  -1 ),    new WImage(    bgImg,                                                           this ),  -1,  NULL },

        { NO_ID,                QRect(  20,  22, 350,  14 ),    new WLabel(    StringList::Wiz8Path,            Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },
        { VAL_FULL_PATH,        QRect(  20,  43, 308,  19 ),    new WLineEdit( "",                              Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,                QRect( 335,  39,  -1,  -1 ),    new WButton(   "LOCKS AND TRAPS/TRAPS_DEVICEICONS.STI",         8, true,  1.0,  this ),  -1,  SLOT(chooseDir(bool)) },

        { NO_ID,                QRect(  30,  87, 200,  14 ),    new WLabel(    StringList::PreferredLang,       Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },
        { VAL_ISO_LANG,         QRect( 240,  85, 120,  19 ),    new WLineEdit( "",                              Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },

        { NO_ID,                QRect(  30, 115, 200,  14 ),    new WLabel(    StringList::PreferredCodepage,   Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },
        { VAL_CHARSET,          QRect( 240, 113, 120,  19 ),    new WLineEdit( "",                              Qt::AlignLeft, 10, QFont::Thin, this ),  -1,  NULL },

        { CB_SUPPRESS_WARNINGS, QRect(  30, 200, 340,  14 ),    new WCheckBox( StringList::SuppressWarnings,                                    this ),  -1,  SLOT(setCb(int)) },
        { NO_ID,                QRect( 312, 268,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",                0, true, 1.0,   this ),  -1,  SLOT(save()) },
        { NO_ID,                QRect( 344, 268,  -1,  -1 ),    new WButton(   "DIALOGS/DIALOGCONFIRMATION.STI",                4, true, 1.0,   this ),  -1,  SLOT(reject()) },
    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    if (WLineEdit *path = qobject_cast<WLineEdit *>(m_widgets[ VAL_FULL_PATH ] ))
    {
        path->setText( SLFFile::getWizardryPath() );
        path->setReadOnly( true );
    }
    if (WLineEdit *iso = qobject_cast<WLineEdit *>(m_widgets[ VAL_ISO_LANG ] ))
    {
        QSettings settings;

        QVariant lang = settings.value( "PreferredLanguage", "ENG" );

        iso->setText( lang.toString() );
        iso->setMaxLength( 3 );
    }
    if (WLineEdit *cpage = qobject_cast<WLineEdit *>(m_widgets[ VAL_CHARSET ] ))
    {
        QSettings settings;

        QVariant charset = settings.value( "Codepage", "Windows-1251" );

        cpage->setText( charset.toString() );
    }
    if (WCheckBox *cb = qobject_cast<WCheckBox *>(m_widgets[ CB_SUPPRESS_WARNINGS ] ))
    {
        QSettings settings;

        QVariant warn = settings.value( "SuppressWarningAll" );

        cb->setTristate( false );
        if (!warn.isNull() && (warn.toBool() == true))
        {
            cb->setCheckState( Qt::Checked );
        }
        else
        {
            cb->setCheckState( Qt::Unchecked );

            foreach ( const QString &w, individual_warnings )
            {
                warn = settings.value( w );

                if (!warn.isNull() && (warn.toBool() == true))
                {
                    // setCheckState() will set tristate implicitly if doing a partial check,
                    // but being explicit here.
                    cb->setTristate( true );
                    cb->setCheckState( Qt::PartiallyChecked );
                    break;
                }
            }
        }
    }

    this->setMinimumSize( bgImgSize * m_scale );
    this->setMaximumSize( bgImgSize * m_scale );

    // Something in the OK button displeased the dialog layout, and made a
    // bigger than minimum dialog. We have to make an additional call to force
    // it back to the right size after adding the OK button.
    this->resize( bgImgSize * m_scale );
}

void DialogPreferences::save()
{
    if (m_wizardryPathChanged)
    {
        if (WLineEdit *path = qobject_cast<WLineEdit *>(m_widgets[ VAL_FULL_PATH ] ))
        {
            QString  new_wizardry_path = path->text();

            if (new_wizardry_path != SLFFile::getWizardryPath() )
            {
                QSettings settings;

                settings.setValue( "Wizardry Path", new_wizardry_path );
            }
        }
    }
    if (WLineEdit *iso = qobject_cast<WLineEdit *>(m_widgets[ VAL_ISO_LANG ] ))
    {
        QSettings settings;

        settings.setValue( "PreferredLanguage", iso->text() );
    }
    if (WLineEdit *cpage = qobject_cast<WLineEdit *>(m_widgets[ VAL_CHARSET ] ))
    {
        QSettings settings;

        settings.setValue( "Codepage", cpage->text() );
    }

    emit accept();
}

void DialogPreferences::chooseDir(bool)
{
    QString new_wizardry_path = QFileDialog::getExistingDirectory( NULL,
                  QObject::tr("Select your Wizardry 8 Folder..."),
                  QDir::rootPath(),
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog );

    if (! new_wizardry_path.isEmpty())
    {
        if (WLineEdit *path = qobject_cast<WLineEdit *>(m_widgets[ VAL_FULL_PATH ] ))
        {
            path->setText( new_wizardry_path );
        }
        m_wizardryPathChanged = true;
    }
}

void DialogPreferences::setCb(int state)
{
    if (WCheckBox *cb = qobject_cast<WCheckBox *>( sender() ))
    {
        if (m_widgets.key( cb ) == CB_SUPPRESS_WARNINGS)
        {
            QSettings settings;

            if (state == Qt::Checked)
            {
                settings.setValue( "SuppressWarningAll", true );
            }
            else if (state == Qt::Unchecked)
            {
                settings.setValue( "SuppressWarningAll", false );

                // TODO: Have to add in every individual warning you add here also
                foreach ( const QString &w, individual_warnings )
                {
                    settings.setValue( w, false );
                }
            }
        }
    }
}

QPixmap DialogPreferences::makeDialogForm()
{
    // there is no suitable existing Wizardry image for the dialog we
    // want here, so we hack one together from a few different pieces
    QPixmap bgImg, bgPtn, bgTbs;

    // images used in dialog
    bgImg = SLFFile::getPixmapFromSlf( "DIALOGS/POPUP_ITEMINFO.STI", 0 );
    bgPtn = SLFFile::getPixmapFromSlf( "DIALOGS/DIALOGBACKGROUND.STI", 0 );
    bgTbs = SLFFile::getPixmapFromSlf( "CHAR GENERATION/CG_PERSONALITY.STI", 0 );

    QPixmap customImage( QSize( bgImg.width(), bgImg.height() + 20 ) );

    QPainter p;

    p.begin( &customImage );
    // First make it a little taller, by 20 rows and squeeze the hole for
    // the scrollarea and the buttonbox over 20 pixels to the right
    qWarning() << bgImg.width() << bgImg.height() << bgPtn.width() << bgPtn.height();
    p.drawPixmap(   0,   0, bgImg,   0,   0, bgImg.width(), 242 );       // Top

    p.drawPixmap(   6,   6, bgPtn,   0,   0,           122, bgPtn.height() );
    p.drawPixmap( 128,   6, bgPtn,   0,   0, bgPtn.width(), bgPtn.height() );
    p.drawPixmap(   6, 262, bgPtn,   0,   0,           122, 17             ); // This area should be getting overwritten by the draw 3 lines down, but without it you get garbage in the dialog!

    p.drawPixmap(   0, 177, bgImg,                 0,                157,             6, bgImg.height() - 157 ); // LHS bevel
    p.drawPixmap( 378, 177, bgImg, bgImg.width() - 6,                157,             6, bgImg.height() - 157 ); // RHS bevel
    p.drawPixmap(   0, 262, bgImg,                 0,                242, bgImg.width(), bgImg.height() - 242 ); // Bottom


    p.drawPixmap(  16,  39, bgTbs, 146, 165,           274,  25 );       // Text box for Wiz8 path
    p.drawPixmap(  75,  39, bgTbs, 160, 165,           260,  25 );

    p.drawPixmap(   8,  76, bgTbs,  20, 160,            80,  65 );       // Text boxes for language
    p.drawPixmap(  85,  76, bgTbs,  40, 160,            80,  65 );
    p.drawPixmap( 125,  76, bgTbs,  40, 160,           230,  65 );
    p.drawPixmap( 351,  76, bgTbs, 400, 160,            25,  65 );

    p.end();

    return customImage;
}

QPixmap DialogPreferences::makeWider( QImage im, int width )
{
    QPainter p;
    QPixmap  in = QPixmap::fromImage( im );
    QPixmap  out( width, in.height() );

    out.fill( Qt::red );
    p.begin(&out);
    p.drawPixmap( 0, 0, in );
    p.drawPixmap( width - in.width() + 6, 0, in, 6, 0, in.width() - 6, in.height() );
    p.end();

    return out;
}
