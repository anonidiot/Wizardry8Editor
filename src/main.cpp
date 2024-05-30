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

#include <QApplication>
#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMessageBox>
#include <QSettings>
#include <QStringList>
#include <QStyleFactory>
#include <QTextCodec>

#include <QtGui/QImage>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidget>

#include <QDebug>

#include "SLFFile.h"
#include "STI.h"

#include "Wizardry8Style.h"

#include "DialogBegin.h"
#include "DialogPatchExe.h"
#include "DialogParallelWorlds.h"
#include "Localisation.h"
#include "MainWindow.h"
#include "main.h"
#include "bspatch.h"
#include "facts.h"

#include <Urho3D/IO/File.h>

StringList *wiz8Strings = NULL;
StringList *wiz8BaseStrings = NULL;

// We can't initialise these before the application,
// so they have to be pointers instead of objects
QPixmap *arrowCursor      = NULL;
QPixmap *whatsThisCursor  = NULL;
QPixmap *closedHandCursor = NULL;

static DialogPatchExe::wizardry_ver   s_app_version     = DialogPatchExe::wizardry_ver::WIZ_VER_UNKNOWN;
static QStringList                    s_parallel_worlds;
static QString                        s_parallel_world;

// This setting is to prevent a foreign language mod (eg. Gray Tiefing)
// completely overrunning the entire interface and making everything
// unintelligible when not desired. But since some of this stuff is a
// module's prerogrative to intentionally change, it needs to be
// configurable.
// We ended up in this situation because even when localisation strings
// exist, they don't necessarily cover _every_single_string_.
// So it's choice 1 of whether or not to apply localisation at all,
// choice 2 of which language to use for localisation if you do use it,
// and choice 3, regardless of whether you use localisation or not, over
// whether to ignore the bulk of the strings coming out of the module,
// and instead use the ones from the default DATA.SLF so that the interface
// still stays in the language you are used to even when you are looking
// at foreign mods.
static bool                           s_ignore_most_mod_strings = false;

bool getIgnoreModStrings()
{
    return s_ignore_most_mod_strings;
}

void setIgnoreModStrings(bool value)
{
    s_ignore_most_mod_strings = value;
}

const StringList *getBaseStringTable()
{
    // If module specific strings have been disabled - or, in the case of the
    // parallel worlds dialog, they haven't been loaded yet, use base strings only
    if (s_ignore_most_mod_strings || !wiz8Strings)
       return (const StringList *)wiz8BaseStrings;

    // Else get them from the mod/parallel world the same as everything else
    return getStringTable();
}

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

void setupLanguageCode(bool reset)
{
    QSettings settings;

    QVariant codepage = settings.value( "Codepage" );

    if (codepage.isNull() || reset)
    {
        settings.setValue( "Codepage", "Windows-1251" );
    }
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

bool isWizardry128()
{
    if (s_app_version >= DialogPatchExe::wizardry_ver::WIZ_VER_1_2_8_UNKNOWN)
        return true;

    return false;
}

bool isParallelWorlds()
{
    if (s_parallel_worlds.size() > 0)
        return true;

    return false;
}

// return value:
//    true = exit app
//    false = keep going
bool identifyWizardryVersion( bool techIdentify, QString customExe )
{
    QList<struct DialogPatchExe::exe_info>        exeIds;

    bool     rv = false;

    exeIds = DialogPatchExe::identifyWizardryExeVersion( customExe );

    if (techIdentify)
    {
        rv = true;

#if defined( Q_OS_WIN )
        if (exeIds.size() < 1)
        {
            QMessageBox::critical(NULL, "ERROR", "Couldn't process Wizardry folder");
        }
        else if (exeIds[0].exePath.isEmpty() && !exeIds[0].error.isEmpty())
        {
            QMessageBox::critical(NULL, "ERROR", exeIds[0].error);
        }
        else
        {
            QString txt;

            if (!exeIds[0].exePath.isEmpty())
            {
                 QFileInfo fi( exeIds[0].exePath );

                 txt = QString("Wizardry Folder: %1\n").arg( QDir::toNativeSeparators( fi.dir().absolutePath() ) );
            }

            for (int k=0; k<exeIds.size(); k++)
            {
                if (!exeIds[k].exePath.isEmpty())
                {
                    QFileInfo fi( exeIds[k].exePath );

                    txt += QString("\nExecutable: %1\n").arg( fi.fileName() );
                }
                if (! exeIds[k].error.isEmpty())
                {
                    txt += QString("Error: %1\n").arg( exeIds[k].error );
                }
                else
                {
                    txt += QString("Identified as %1\n").arg( DialogPatchExe::getVersionStr(exeIds[k].ver) );
                    if (! exeIds[k].patchesApplied.empty())
                    {
                        QString  patches;

                        for (int j=0; j<exeIds[k].patchesApplied.size(); j++)
                            patches += ", " + exeIds[k].patchesApplied[j];
                        
                        txt += "Recognised Patches: " + patches.mid(2) + "\n";
                    }
                    txt += QString("Modified MD5Hash: %1\n").arg( exeIds[k].md5Hash );
                }
            }
            QMessageBox m( QMessageBox::NoIcon, QCoreApplication::applicationName(), QString(), QMessageBox::Ok );

            m.setText( txt );
            m.exec();
        }
#else
        for (int k=0; k<exeIds.size(); k++)
        {
            if (exeIds[k].error.isEmpty())
            {
                QByteArray v = DialogPatchExe::getVersionStr(exeIds[k].ver).toLatin1();
                QByteArray e = exeIds[k].exePath.toLatin1();

                printf("Wizardry Executable at location: %s\n", e.data());
                printf("Identified as: %s\n", v.data());
                if (! exeIds[k].patchesApplied.empty())
                {
                    QString  patches;

                    for (int j=0; j<exeIds[k].patchesApplied.size(); j++)
                        patches += ", " + exeIds[k].patchesApplied[j];
                    
                    QByteArray p = patches.toLatin1();
                    printf("Recognised Patches: %s\n", p.data()+2);
                }
                printf("Modified MD5Hash: %s\n\n", exeIds[k].md5Hash);
            }
            else
            {
                QByteArray err = exeIds[k].error.toLatin1();

                if (! exeIds[k].exePath.isEmpty())
                {
                    QByteArray e = exeIds[k].exePath.toLatin1();

                    fprintf(stderr, "Wizardry Executable at location: %s\n", e.data());
                }
                fprintf(stderr, "%s\n", err.data());
            }
        }
#endif
    }
    else
    {
        if (! exeIds[0].error.isEmpty())
        {
            QMessageBox::critical(NULL, "ERROR", exeIds[0].error);
            
            rv = true;
        }
        else
        {
            s_app_version = exeIds[0].ver;
            s_parallel_worlds.clear();

            if (isWizardry128()) // this will work now s_app_version is set
            {
                // Parallel worlds could be active.
                // Check the directory structure out, but also probe WIZ8.INI to find out

                QString     &wizardryPath    = SLFFile::getWizardryPath();
                QDir         path            = QDir(wizardryPath);
                QFile       *iniFile         = NULL;
                QStringList  filter;

                // folder structure first

                filter << "ParallelWorld";
                QStringList entries = path.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );

                if (entries.size() == 1)
                {
                    QDir    parworlds = path;

                    parworlds.cd( entries.at(0) );

                    filter.clear();
                    s_parallel_worlds = parworlds.entryList(filter, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot );
                }

                // now the ini file

                QDirIterator it( path, QDirIterator::Subdirectories);
                while (it.hasNext())
                {
                    QString file = it.next();

                    if (file.compare( path.absoluteFilePath( "Wiz8.ini" ), Qt::CaseInsensitive ) == 0)
                    {
                        iniFile = new QFile(file);
                        break;
                    }
                }

                if (iniFile)
                {
                    bool    parallel_worlds = false;

                    // INI File is text file without character set header bytes,
                    // but will contain Cyrillic characters in the High ASCII range
                    // Charset appears to be windows-1251 but not explicitly stated

                    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
                    if (iniFile->open(QIODevice::ReadOnly))
                    {
                        QTextStream in( iniFile );

                        while (!in.atEnd())
                        {
                            QByteArray line = iniFile->readLine();
                            QString utfLine = codec->toUnicode( line );

                            // INI files usually don't have whitespace around the =, but lets
                            // not force the assumption because there are exceptions

                            // (BTW: The reason we're not using QSettings to read the ini file
                            //  is because this code should work on linux also.)
                            if (utfLine.startsWith( "ParallelWorldExists" ))
                            {
                                int equals = utfLine.indexOf( '=' );

                                if (utfLine.mid( equals+1 ).trimmed() == "1")
                                {
                                    parallel_worlds = true;
                                }
                            }
                            else if (utfLine.startsWith( "ParallelWorld" ))
                            {
                                int equals = utfLine.indexOf( '=' );

                                s_parallel_world = utfLine.mid( equals+1 ).trimmed();
                            }
                        }
                        iniFile->close();
                    }
                    delete iniFile;

                    // ini file says the directory structure we found is unused, so clear it
                    if (parallel_worlds == false)
                    {
                        s_parallel_worlds.clear();
                    }
                }
            }
        }
    }

    return rv;
}

    void *_open(const char *pathname, Urho3D::FileMode /* mode */)
    {
        QIODevice::OpenMode qt_mode = QIODevice::ReadOnly;

        /* -- deliberately disabled to force ReadOnly mode for QT resources
        switch (mode)
        {
            case FILE_READ:      qt_mode = QIODevice::ReadOnly;  break;
            case FILE_WRITE:     qt_mode = QIODevice::WriteOnly; break;
            case FILE_READWRITE: qt_mode = QIODevice::ReadWrite; break;
        }
        */

        QFile *f = new QFile(pathname);

        if (f->open( qt_mode ))
            return (void *)f;

        delete f;
        return NULL;
    }
    int  _close(void *handle)
    {
        delete ((QFile *)handle);
        return 0;
    }
    int  _size(void *handle)
    {
        return (int) ((QFile *)handle)->size();
    }
    int  _flush(void *handle)
    {
        return (int) ((QFile *)handle)->flush();
    }
    int  _seek(void *handle, long offset, int whence)
    {
        qint64 o = offset;

        if (whence == SEEK_CUR)
            o += ((QFile *)handle)->pos();
        else if (whence == SEEK_END)
            o += ((QFile *)handle)->size();

        return (int) ((QFile *)handle)->seek( o );
    }
    long _tell(void *handle)
    {
        return (int) ((QFile *)handle)->pos();
    }
    size_t _read(void *ptr, size_t size, size_t nmemb, void *handle)
    {
        qint64 to_read = size * nmemb;
        qint64 actual  = ((QFile *)handle)->read((char *)ptr, to_read);

        return (int)(actual / size);
    }
    size_t _write(const void * /* ptr */, size_t /* size */, size_t /* nmemb */, void * /* handle */)
    {
        /* -- deliberately disabled to force ReadOnly mode for QT resources
        qint64 to_write = size * nmemb;
        qint64 actual  = ((QFile *)handle)->write(ptr, to_read);

        return (int)(actual / size);
        */
        return -1;
    }

void registerQFileWithUrho3D(void)
{
    struct Urho3D::external_file_ops qfile_ops;

    strcpy( qfile_ops._filePrefix, ":/" );
    qfile_ops._open   = _open;
    qfile_ops._close  = _close;
    qfile_ops._size   = _size;
    qfile_ops._flush  = _flush;
    qfile_ops._seek   = _seek;
    qfile_ops._tell   = _tell;
    qfile_ops._read   = _read;
    qfile_ops._write  = _write;

    Urho3D::File::RegisterExternalFileType( qfile_ops );
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

    bool reset    = false;
    bool identify = false;
    for (int k=1; k<argc; k++)
    {
        if ((strcmp( argv[k], "--help" ) == 0) ||   // usual unix help option
            (strcmp( argv[k], "/?" ) == 0))         // common Windows help option
        {
#if defined( Q_OS_WIN )
            QMessageBox m( QMessageBox::NoIcon, QCoreApplication::applicationName(), QString(), QMessageBox::Ok );

            m.setText( QString("Edits Wizardry 8 Save game files.\n\n"
                           "%1 [--resetWizardryPath] [<save game file>]\n\n"
                           "--resetWizardryPath\n"
                           "     Forget existing stored Wizardry folder location and prompt again.\n"
                           "--identifyExeVersion\n"
                           "     Display identifiying information for the Wizardry EXE in use.\n").arg(argv[0]));
            m.exec();
#else
            printf("%s usage:\n  %s [<options>] [<save game file>]\n\n", argv[0], argv[0]);
            printf("  --resetWizardryPath         Forget existing stored Wizardry folder location and prompt again.\n");
            printf("  --identifyExeVersion [exe]  Print out identification information for the Wizardry EXE in use.\n");
#endif
            return 0;
        }
        else if (strcmp( argv[k], "--resetWizardryPath" ) == 0)
            reset = true;
        else if (strcmp( argv[k], "--identifyExeVersion" ) == 0)
            identify = true;
        else if ((strncmp( argv[k], "-", 1) != 0) && fileToOpen.isEmpty())
        {
            fileToOpen = QString( argv[k] );
        }
    }

    // Can't even load our font until we know the Wizardry folder,
    // so can't use any app-specific style in this dialog

    if (false == setupWizardryPath( reset ))
        return 0;

    // There are numerous text sections throughout Wizardry objects
    // that _look_ (if you are looking at the English version at least)
    // like they are intended to be Unicode text sections - based on
    // the fact they are all low-ASCII and have intervening '\0' bytes.

    // But, the Russian language mod Gray Tiefing clearly isn't using
    // it that way. And I suspect it's the same elsewhere.

    // Setup a default codepage to use for converting text instead of
    // expecting unicode all the time.
    setupLanguageCode( false );

    if (identify)
    {
        identifyWizardryVersion( true, fileToOpen );

        // exit application without error
        return 0;
    }

    // exit app with error if all not as expected
    if (identifyWizardryVersion( false, "" ))
        return -1;

    if (isWizardry128() && isParallelWorlds())
    {
        // Wizardry 1.2.8 with Parallel Worlds active changes everything, in respect to how
        // to locate SLFs.

        // What we need to do next is to present a dialog asking the user which Parallel
        // World they are using. Unfortunately if we do that at this point the incorrect
        // app style will be used because the SLF paths and everything that depends on
        // them like the fonts haven't been setup yet.

        // So instead we set the initial parallel world to the empty string which forces
        // a parallel world structure but only uses files from the base tree (ie. DATA.SLF
        // only). This is enough for us to load fonts and the STIs needed to do a dialog.

        SLFFile::setParallelWorld( "" );

        // (After the dialog has closed and a real selection has been made the application
        // of the correct parallel world in SLFFile::setParallelWorld resets the entire
        // path cache because all the locations have now changed. Provided there's nothing
        // on screen at the time, and no Urho3D rendering has occurred yet this is ok.
        // Otherwise it makes for unexpected results. BTW: the fact that the Urho3D engine's
        // own cache is affected by this is the reason we only let the Parallel World be
        // setup at app launch.)

        // The changes to the SLF search order as a result of parallel worlds being active
        // are explained in the SLF file.
    }

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
    wiz8BaseStrings = new StringList( "Strings/stringdata.dat", true );

    if (isWizardry128() && isParallelWorlds())
    {
        // Far enough. If we proceed any further initialising cursors and strings etc.
        // we have to redo it all after choosing a world.

        DialogParallelWorlds d( s_parallel_worlds, s_parallel_world );

        if (d.exec() != QDialog::Accepted)
            return -1;

        s_parallel_world = d.getWorld();

        SLFFile::setParallelWorld( s_parallel_world );
        qDebug() << s_parallel_world << SLFFile::getParallelWorldPath() << Localisation::getModuleName();

        s_ignore_most_mod_strings = d.getIgnoreModStrings();
    }

    wiz8Strings     = new StringList( "Strings/stringdata.dat" );

    Localisation *loc = Localisation::getLocalisation();

    loc->setLocalisationActive( true );

#ifndef USE_STANDARD_CURSORS
    SLFFile cursors( "CURSORS/2D-CURSORS.STI" );
    if (cursors.open(QFile::ReadOnly))
    {
        QByteArray array = cursors.readAll();
        STI c( array );

        arrowCursor      = new QPixmap( QPixmap::fromImage( c.getImage(  0 ) ) );
        whatsThisCursor  = new QPixmap( QPixmap::fromImage( c.getImage( 13 ) ) );
        closedHandCursor = new QPixmap( QPixmap::fromImage( c.getImage( 14 ) ) );

        cursors.close();
    }
#endif

    registerQFileWithUrho3D();

    if (fileToOpen.isEmpty() || !QFile::exists( fileToOpen ))
    {
        // If we are using Wizardry 1.2.8 there is no choice -
        // you have to open an existing game as there is no
        // support for new games. Otherwise display a dialog
        // and use the user's choice.

        int  action = DialogBegin::Action::OpenFile;

        if (! isWizardry128())
        {
            DialogBegin d;

            if (d.exec() != QDialog::Accepted)
                return -1;

            action = d.getAction();
        }

        switch (action)
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
                QString Path;

                if (isParallelWorlds())
                {
                    Path = SLFFile::getParallelWorldPath();
                }
                else
                {
                    Path = SLFFile::getWizardryPath();
                }

                fileToOpen = ::getOpenFileName(NULL, QObject::tr("Open File"), Path + "/Saves", QObject::tr("Saved Games (*.sav)"));
                break;
            }
        }
    }

    if (fileToOpen.isEmpty() && isWizardry128())
    {
        QMessageBox::warning(NULL, QObject::tr("Not Supported"),
            QObject::tr("You are using a Wizardry 1.2.8 executable.\nNew and Reset games are not supported\nby this editor for that version.\n\nYou need to open an existing game file."));
    }
    else
    {
        MainWindow w( fileToOpen );

        a.exec();
    }

#ifndef USE_STANDARD_CURSORS
    delete arrowCursor;
    delete whatsThisCursor;
    delete closedHandCursor;
#endif

    delete wiz8Strings;
    delete wiz8BaseStrings;

    return 0;
}

// FIXME: this is AR, not scale
// Should really be eg [800|1024|1280|...] / 640
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

facts s_facts = facts();

void setFacts(facts f)
{
    s_facts = f;
}

bool testFact(int fact_id)
{
    if (! s_facts.isNull())
    {
        return s_facts.getValue( fact_id );
    }
    return false;
}

bool testFact(QString fact_name)
{
    if (! s_facts.isNull())
    {
        return s_facts.testFact( fact_name );
    }
    return false;
}

void  setBoolSetting(char *setting, bool value)
{
    QSettings settings;

    settings.setValue(setting, value);
}

bool  getBoolSetting(char *setting)
{
    QSettings settings;

    QVariant v = settings.value(setting);

    if (v.isNull())
        return false;

    return v.toBool();
}
