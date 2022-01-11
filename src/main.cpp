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

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QSettings>
#include <QStyleFactory>

#include <QtGui/QImage>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

#include <QDebug>

#include "SLFFile.h"
#include "STItoQImage.h"

#include "Wizardry8Style.h"

#include "DialogBegin.h"
#include "MainWindow.h"
#include "main.h"
#include "bspatch.h"

StringList *wiz8Strings = NULL;

// We can't initialise these before the application,
// so they have to be pointers instead of objects
QPixmap *arrowCursor      = NULL;
QPixmap *whatsThisCursor  = NULL;
QPixmap *closedHandCursor = NULL;

const StringList *getStringTable()
{
    return (const StringList *)wiz8Strings;
}

int makeFont(QString bitmapFont, QString otfFont, QString patchfile)
{
    int       rv = -1;
    SLFFile   font( bitmapFont );

    if (font.open(QFile::ReadOnly))
    {
        QByteArray bmp_ba = font.readAll();
        font.close();

        QFile patch( patchfile );

        if (patch.open(QFile::ReadOnly))
        {
            QByteArray patch_ba = patch.readAll();
            patch.close();

            otfFont = QDir::tempPath() + QDir::separator() + otfFont;
            QByteArray otf_filename = otfFont.toLatin1();

            rv = bspatch( (unsigned char *)bmp_ba.data(),   bmp_ba.size(),    // Buffer of binary data and size
                          otf_filename.constData(),                           // output filename
                          (unsigned char *)patch_ba.data(), patch_ba.size()); // Buffer of patch data and size
        }
        else
        {
            qWarning() << "Could open patch" << patchfile << "for font" << bitmapFont;
        }
    }
    else
    {
        qWarning() << "Couldn't open SLF font" << bitmapFont << "to patch";
    }
    return rv;
}

bool setupWizardryPath(bool reset)
{
    QSettings settings;

    QVariant wpath = settings.value( "Wizardry Path" );
    QString  wizardry_path;

    if (! wpath.isNull() && !reset)
    {
        wizardry_path = wpath.toString();
    }

    while (1)
    {
        if (! wizardry_path.isEmpty())
        {
            QDir check( wizardry_path );

            if (check.exists())
            {
                QDirIterator it( check, QDirIterator::NoIteratorFlags );
                while (it.hasNext())
                {
                    it.next();

                    if (it.fileName().compare( "Wiz8.exe", Qt::CaseInsensitive ) == 0)
                    {
                        settings.setValue( "Wizardry Path", wizardry_path );
                        SLFFile::setWizardryPath( wizardry_path );
                        return true;
                    }
                }
            }
        }
     
        wizardry_path = QFileDialog::getExistingDirectory( NULL,
                  QObject::tr("Select your Wizardry 8 Folder..."),
                  QDir::rootPath(),
                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog );

        if (wizardry_path.isEmpty())
            return false;
    }
}

int main(int argc, char *argv[])
{
// Windows is the only OS I can test HiDPI on at the moment, not sure if this is
// wanted on the others yet or not.
#ifdef WIN64
  #if QT_VERSION >= QT_VERSION_CHECK(5,6,0) && QT_VERSION < QT_VERSION_CHECK(6,0,0)
    // QT6 defaults to the right behaviour
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  #endif
#endif

    QCoreApplication::setOrganizationName("Misc");
    QCoreApplication::setApplicationName("Wizardry 8 Editor");

    QApplication a(argc, argv);

    QString fileToOpen;

    bool reset = false;
    for (int k=1; k<argc; k++)
    {
        if ((strcmp( argv[k], "--help" ) == 0) ||   // usual unix help option
            (strcmp( argv[k], "/?" ) == 0))         // common Windows help option
        {
            printf("%s usage:\n  %s [--resetWizardryPath] <save game file>\n", argv[0], argv[0]);
            printf("\n  --resetWizardryPath       Forget existing stored Wizardry folder location and prompt again.\n");
            return 0;
        }
        else if (strcmp( argv[k], "--resetWizardryPath" ) == 0)
            reset = true;
        else if ((strncmp( argv[k], "-", 1) != 0) && fileToOpen.isEmpty())
        {
            fileToOpen = QString( argv[k] );
        }
    }

    // Can't even load our font until we know the Wizardry folder,
    // so can't use any app-specific style in this dialog

    if (false == setupWizardryPath( reset ))
        return 0;

    // OTF version looks nicer, except on Windows XP which doesn't kern it properly

    // The point of deriving these as binary patches from the original files it to
    // try to avoid infringing any copyright in basing standard fonts off the originals;
    // ie. require someone to have the original game in order to be able to produce the
    // font, and only keep it around for a game-related purpose (editing the save games).
    // In actual point of fact the fonts are smaller if just compressed outright and
    // included as is.
#if defined(WIN32) && !defined(WIN64)
    makeFont( "FONTS/WIZ_TEXT_FONT.STI",      "Wizardry-Regular.ttf", ":/WizFontTtfReg.bspatch"  );
    makeFont( "FONTS/WIZ_TEXT_FONT_BOLD.STI", "Wizardry-Bold.ttf",    ":/WizFontTtfBold.bspatch" );

    if ((-1 == QFontDatabase::addApplicationFont(QDir::tempPath() + QDir::separator() + "Wizardry-Regular.ttf")) ||
        (-1 == QFontDatabase::addApplicationFont(QDir::tempPath() + QDir::separator() + "Wizardry-Bold.ttf")))
    {
        qWarning() << "couldn't add application font";
    }
#else
    makeFont( "FONTS/WIZ_TEXT_FONT.STI",      "Wizardry-Regular.otf", ":/WizFontOtfReg.bspatch"  );
    makeFont( "FONTS/WIZ_TEXT_FONT_BOLD.STI", "Wizardry-Bold.otf",    ":/WizFontOtfBold.bspatch" );

    if ((-1 == QFontDatabase::addApplicationFont(QDir::tempPath() + QDir::separator() + "Wizardry-Regular.otf")) ||
        (-1 == QFontDatabase::addApplicationFont(QDir::tempPath() + QDir::separator() + "Wizardry-Bold.otf")))
    {
        qWarning() << "couldn't add application font";
    }
#endif

    // Lucida Calligraphy is the best match font to one particular Wizardry font
    // I've found but not one I'm free to redistribute. It likely isn't installed,
    // so drop back to Monotype Corsiva, which possibly is, if it isn't found.
    QFont::insertSubstitution("Lucida Calligraphy", "Monotype Corsiva");

    // If we failed the patch application for either of the above, then we
    // don't have the font we want, and the app will look awful and automatically
    // use a fallback determined by the system, but we should at least let the
    // user try to progress with running the app and not terminate here.

    QApplication::setStyle(new Wizardry8Style);
    QApplication::setPalette( QApplication::style()->standardPalette() );

    // This gets used a lot by everything, so just process the table once
    // at startup and store it for app lifetime
    wiz8Strings = new StringList( "Strings/stringdata.dat" );

#ifndef USE_STANDARD_CURSORS
    SLFFile cursors( "CURSORS/2D-CURSORS.STI" );
    if (cursors.open(QFile::ReadOnly))
    {
        QByteArray array = cursors.readAll();
        STItoQImage c( array );

        arrowCursor      = new QPixmap( QPixmap::fromImage( c.getImage(  0 ) ) );
        whatsThisCursor  = new QPixmap( QPixmap::fromImage( c.getImage( 13 ) ) );
        closedHandCursor = new QPixmap( QPixmap::fromImage( c.getImage( 14 ) ) );

        cursors.close();
    }
#endif

    if (fileToOpen.isEmpty() || !QFile::exists( fileToOpen ))
    {
        DialogBegin d;

        if (d.exec() != QDialog::Accepted)
            return -1;

        switch (d.getAction())
        {
            default:
                return -1;

            case DialogBegin::Action::NewFile:
            {
                // leave fileToOpen unset and the new dialog will be shown
                break;
            }
            case DialogBegin::Action::OpenFile:
            {
                QString &wizardryPath = SLFFile::getWizardryPath();

                fileToOpen = ::getOpenFileName(NULL, QObject::tr("Open File"), wizardryPath + "/Saves", QObject::tr("Saved Games (*.sav)"));
                break;
            }
        }
    }

    MainWindow w( fileToOpen );

    a.exec();

#ifndef USE_STANDARD_CURSORS
    delete arrowCursor;
    delete whatsThisCursor;
    delete closedHandCursor;
#endif

    delete wiz8Strings;

    return 0;
}

static double s_scale = (double)ORIGINAL_DIM_X / (double)ORIGINAL_DIM_Y;

void setAppScale(double scale)
{
    s_scale = scale;
}

double getAppScale()
{
    return s_scale;
}

QPixmap getCursor(int id)
{
#ifndef USE_STANDARD_CURSORS
    switch (id)
    {
        case Qt::ArrowCursor:
            return *arrowCursor;

        case Qt::WhatsThisCursor:
            return *whatsThisCursor;

        case Qt::ClosedHandCursor:
            return *closedHandCursor;
    }
#endif
    return QPixmap();
}

// Unfortunately the simple QFileDialog::getOpenFileName() and
// QFileDialog::getSaveFileName() calls aren't usable by us due
// to the fact we've removed all the text space on the button
// widgets for our style, so these are replacement versions that
// uses a different style.

QString getOpenFileName(QWidget *parent, const QString &caption, const QString &directory, const QString &filter)
{
    QString   response;
    QStyle   *s = NULL;

    QFileDialog dialog(parent, caption, directory, filter);

    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setOption(QFileDialog::Option::DontUseNativeDialog, true);
    QStringList styleNames = QStyleFactory::keys();
    for (const auto &style : styleNames)
    {
        s = QStyleFactory::create(style);

//        qDebug() << style;
        if (s)
        {
            dialog.setStyle( s );
            break;
        }
    }

    if (dialog.exec() == QDialog::Accepted)
        response = dialog.selectedUrls().value(0).toLocalFile();

    dialog.close();
    dialog.setStyle( QApplication::style() );

    if (s)
    {
        delete s;
    }

    return response;
}

QString getSaveFileName(QWidget *parent, const QString &caption, const QString &directory, const QString &filter)
{
    QString   response;
    QStyle   *s = NULL;

    QFileDialog dialog(parent, caption, directory, filter);

    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setOption(QFileDialog::Option::DontUseNativeDialog, true);
    QStringList styleNames = QStyleFactory::keys();
    for (const auto &style : styleNames)
    {
        s = QStyleFactory::create(style);

//        qDebug() << style;
        if (s)
        {
            dialog.setStyle( s );
            break;
        }
    }

    if (dialog.exec() == QDialog::Accepted)
        response = dialog.selectedUrls().value(0).toLocalFile();

    dialog.close();
    dialog.setStyle( QApplication::style() );

    if (s)
    {
        delete s;
    }

    return response;
}
