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

#include <QtWidgets>

#include "MainWindow.h"
#include "DialogAbout.h"
#include "DialogAboutUrho3D.h"
#include "DialogCurrentPosition.h"
#include "DialogPatchExe.h"
#include "DialogNewFile.h"
#include "WindowDroppedItems.h"
#include "WindowItemsList.h"
#include "WindowFactEditor.h"
#include <QFile>
#include <QActionGroup>
#include <QByteArray>

#include "common.h"
#include "main.h"

#include "Localisation.h"
#include "RIFFFile.h"
#include "SLFFile.h"
#include "character.h"
#include "STI.h"
#include "ScreenCommon.h"

// MainWindow is a singleton class so we can get away with this
// even if it is bad practice. We have a second class in use on
// Windows that wants access to this and otherwise has trouble
// getting it.
static QSize g_windowPadding;

MainWindow::MainWindow(QString loadFile) :
    m_droppedItems(NULL),
    m_findItems(NULL),
    m_factEditor(NULL),
    m_w7_ending(wiz7_end::Null),
    m_barlone_dead(false),
    m_rodan_dead(false)
{
    bool nonhack_game = false;

    if (loadFile.isEmpty())
    {
        m_loadedGame = NULL;
        m_facts = facts();
        setFacts( m_facts );
        m_party = new party();

        for (unsigned int k=0; k<NUM_CHARS; k++)
        {
            m_party->m_chars.append( new character() );
        }
    }
    else
    {
        m_loadedGame = new RIFFFile(loadFile);
        if (!m_loadedGame->open(QIODevice::ReadOnly))
        {
            qWarning() << "Failed to open file" << loadFile << "for reading";
            statusBar()->showMessage(tr("Cannot open file for reading!"));
            QMessageBox::warning(this, tr("File Open Error"),
            tr("Can't open that file for reading."));
            delete m_loadedGame;
            m_loadedGame = NULL;
            m_facts = facts();
            setFacts( m_facts );
            m_party = new party();

            for (unsigned int k=0; k<NUM_CHARS; k++)
            {
                m_party->m_chars.append( new character() );
            }
        }
        else
        {
            m_facts = facts( m_loadedGame->readFacts() );
            setFacts( m_facts );
            m_party = new party( m_loadedGame->readParty() );

            for (unsigned int k=0; k<NUM_CHARS; k++)
            {
                m_party->m_chars.append( new character( m_loadedGame->readCharacter(k), m_loadedGame->readCharacterExtra(k) ));
            }
            m_party->setVisitedMaps( m_loadedGame->getVisitedMapsList() );

            if (! m_loadedGame->seekSegment("HACK"))
                nonhack_game = true;

            m_loadedGame->close();
        }
    }
    m_party->resetCharacterColors();
    m_party->divvyUpPartyWeight();


    // ScreenItems was doing this, but it was causing double free problems.
#if 0
    setAttribute(Qt::WA_DeleteOnClose);
#endif
    m_contentWidget = (QWidget *) new ScreenCommon(m_party, 2);
    m_contentWidget->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );

    QWidget *spacingWidget = new QWidget;
    setCentralWidget(spacingWidget);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    layout->addWidget(m_contentWidget);
    spacingWidget->setLayout(layout);

    createActions();
    createMenus();

    // Cannot change the current position on a 'New' or 'Reset' game
    if (nonhack_game)
        currPosAct->setEnabled(true);
    else
        currPosAct->setEnabled(false);

    connect( m_contentWidget, SIGNAL(partyEmpty()),  this, SLOT(disableSave()) );
    connect( m_contentWidget, SIGNAL(partyViable()), this, SLOT(enableSave())  );
    connect( m_contentWidget, SIGNAL(exit()), this, SLOT(exit())  );

    QString message = tr("A context menu is available by right-clicking");
    statusBar()->showMessage(message);

    g_windowPadding = this->layout()->sizeHint() + QSize( 0, statusBar()->sizeHint().height() ) - m_contentWidget->sizeHint();

    // Maximise doesn't work nicely with the auto window resize we do in this app
    // to maintain aspect ratio.
    // Haven't found any way of disabling maximise in Qt that actually works; but
    // if running under Windows we can use native methods.
#if defined(WIN32) || defined(WIN64)
    HWND hwnd = (HWND) this->winId();
    DWORD value = GetWindowLong( hwnd, GWL_STYLE );
    SetWindowLong( hwnd, GWL_STYLE, (int)(value & ~WS_MAXIMIZEBOX) );

    // And we can also deal with the window resizing a little better
    QCoreApplication::instance()->installNativeEventFilter( new windowsEvents() );
#else
    // FIXME: Doesn't work
    setWindowFlags( Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint );
#endif

    show();

    // No existing saved game loaded
    if (!m_loadedGame)
    {
        if (::isWizardry128())
        {
            QMessageBox::warning(this, tr("App Init Error"),
                tr("New and Reset games are not supported under Wizardry 1.2.8."));
            disableSave();
        }
        else
        {
            // Show the new game dialog
            newAct->activate( QAction::Trigger );
        }
    }
}

// I'm aware that WIN32 is (irritatingly) defined for WIN64 compilation also;
// I just don't like it, and think the intent is clearer to always specify both.
#if defined(WIN32) || defined(WIN64)
bool windowsEvents::nativeEventFilter(const QByteArray &eventType, void *message, long* res)
{
    if (eventType == "windows_generic_MSG")
    {
        MSG *msg = (MSG*)message;

        switch (msg->message)
        {
            // We try to constrain the window to only scale at the fixed aspect ratio
            case WM_SIZING:
            {
                RECT *r = (RECT *)msg->lParam;
                long  w = r->right - r->left;
                long  h = r->bottom - r->top;

                // Keep it on screen
                if (r->left < 0)
                    r->left = 0;
                if (r->top < 0)
                    r->top = 0;

                switch (msg->wParam)
                {
                    case WMSZ_TOP:
                    case WMSZ_TOPLEFT:
                    case WMSZ_BOTTOM:
                    case WMSZ_BOTTOMLEFT:
                        w = (h - g_windowPadding.height()) * ORIGINAL_DIM_X / ORIGINAL_DIM_Y + g_windowPadding.width();
                        r->right = r->left + w;
                        break;

                    case WMSZ_LEFT:
                    case WMSZ_RIGHT:
                        h = (w - g_windowPadding.width()) * ORIGINAL_DIM_Y / ORIGINAL_DIM_X + g_windowPadding.height();
                        r->bottom = r->top + h;
                        break;

                    case WMSZ_TOPRIGHT:
                    case WMSZ_BOTTOMRIGHT:
                        w = (h - g_windowPadding.height()) * ORIGINAL_DIM_X / ORIGINAL_DIM_Y + g_windowPadding.width();
                        r->left = r->right - w;
                        break;
                }

                *res = 0;
                break;
            }
        }
    }
    return false;
}
#endif

MainWindow::~MainWindow()
{
    delete m_party;
    delete m_loadedGame;

    // layout and child windows autodeleted, actions are not - widget is just disassociated from them
    // but we can't delete them until we ourselves have been deleted, so defer them

    newAct->deleteLater();
    openAct->deleteLater();
    saveAct->deleteLater();
    saveAsAct->deleteLater();
    saveAsResetAct->deleteLater();
    printAct->deleteLater();
    exitAct->deleteLater();
    undoAct->deleteLater();
    redoAct->deleteLater();
    cutAct->deleteLater();
    copyAct->deleteLater();
    pasteAct->deleteLater();

    ignoreStringsAct->deleteLater();
    localisationAct->deleteLater();

    QList<QAction *> actions = m_loc_langs->actions();
    for (int k=0; k<actions.size(); k++)
    {
        actions[k]->deleteLater();
    }
    m_loc_langs->deleteLater();

    droppedItemsAct->deleteLater();
    findItemsAct->deleteLater();
    factEditorAct->deleteLater();
    currPosAct->deleteLater();
    patchAct->deleteLater();
    aboutAct->deleteLater();
    aboutQtAct->deleteLater();
    aboutUrhoAct->deleteLater();

    // TODO: delete windowsEvents on WIN32/WIN64 ?
}

QSize MainWindow::sizeHint() const
{
    return m_contentWidget->sizeHint() + g_windowPadding;
}

void MainWindow::newFile()
{
    if (isWizardry128())
    {
        QMessageBox::warning(this, tr("Feature Unavailable"),
            tr("New and Reset games are not supported under Wizardry 1.2.8\ndue to its modified file format."));

        return;
    }

    DialogNewFile *d = new DialogNewFile(this);

    if (d->exec() == QDialog::Accepted)
    {
        m_barlone_dead = d->isBarloneDead();
        m_rodan_dead   = d->isRodanDead();

        switch (d->getScenarioIndex())
        {
            case StringList::W7EndUmpani:   m_w7_ending = wiz7_end::Umpani;  break;
            case StringList::W7EndTRang:    m_w7_ending = wiz7_end::TRang;   break;
            case StringList::W7EndOwnShip:  m_w7_ending = wiz7_end::OwnShip; break;
            case StringList::W8Virgin:      m_w7_ending = wiz7_end::Virgin;  break;
        }

        if (m_loadedGame)
        {
            delete m_loadedGame;
            m_loadedGame = NULL;
        }

        // Reset the screen with no characters

        party *old_party = m_party;
        m_facts = facts();
        setFacts( m_facts );
        m_party = new party();

        for (unsigned int k=0; k<NUM_CHARS; k++)
        {
            m_party->m_chars.append( new character() );
        }
        m_party->resetCharacterColors();
        m_party->divvyUpPartyWeight();

        // The file cannot be saved until at least one character has been added
        disableSave();

        if (ScreenCommon *s = qobject_cast<ScreenCommon *>(m_contentWidget))
        {
            s->updateChars( m_party );
        }
        // Only now after the screens have been updated to use the new party
        // should we release the old party and character
        if (old_party)
        {
            delete old_party;
        }
    }
    else
    {
        // If they cancelled and there is no existing file to fallback to
        // (ie. they've cancelled on the initial launch dialogs) then exit the app
        if (!m_loadedGame)
        {
            // the close method is called this way because in the scenario above
            // we've been called in scope of the constructor (QAction::activate
            // makes direct method calls, not queued ones)
            QTimer::singleShot(0, this, SLOT(close()));
        }
    }
    delete d;
}

void MainWindow::open()
{
    // Show a file dialog and ask what they want to open

    QString Path;

    if (isParallelWorlds())
    {
        Path = SLFFile::getParallelWorldPath();
    }
    else
    {
        Path = SLFFile::getWizardryPath();
    }

    QString openFile = ::getOpenFileName(NULL, tr("Open File"), Path + "/Saves", tr("Saved Games (*.sav)"));
    if (openFile.isEmpty())
        return;

    RIFFFile *loadGame = new RIFFFile(openFile);

    if (!loadGame->open(QIODevice::ReadOnly))
    {
        statusBar()->showMessage(tr("Cannot open file for reading!"));
        QMessageBox::warning(this, tr("File Open Error"),
            tr("Can't open that file for reading."));
        delete loadGame;
        return;
    }

    if (m_loadedGame)
    {
        delete m_loadedGame;
    }
    m_loadedGame = loadGame;

    party *old_party = m_party;

    m_facts = facts( m_loadedGame->readFacts() );
    setFacts( m_facts );
    m_party = new party( m_loadedGame->readParty() );

    for (unsigned int k=0; k<NUM_CHARS; k++)
    {
        m_party->m_chars.append( new character( m_loadedGame->readCharacter(k), m_loadedGame->readCharacterExtra(k) ));
    }
    m_party->divvyUpPartyWeight();
    m_party->setVisitedMaps( m_loadedGame->getVisitedMapsList() );

    // Cannot change the current position on a 'New' or 'Reset' game
    if (m_loadedGame->seekSegment("HACK"))
        currPosAct->setEnabled(false);
    else
        currPosAct->setEnabled(true);

    m_loadedGame->close();

    if (ScreenCommon *s = qobject_cast<ScreenCommon *>(m_contentWidget))
    {
        s->updateChars( m_party );
    }
    // Only now after the screens have been updated to use the new party
    // should we release the old party and character
    if (old_party)
    {
        delete old_party;
    }

    enableSave();
}

void MainWindow::disableSave()
{
    saveAct->setEnabled(false);
    saveAsAct->setEnabled(false);
    saveAsResetAct->setEnabled(false);
}

void MainWindow::enableSave()
{
    if (m_loadedGame)
    {
        saveAct->setEnabled(true);
    }
    saveAsAct->setEnabled(true);
    saveAsResetAct->setEnabled(true);
}

void MainWindow::save()
{
    if (m_loadedGame->open(QIODevice::ReadWrite))
    {
        statusBar()->showMessage(tr("Saving edited game file..."));

        m_loadedGame->writeParty( m_party->serialize() );

        for (unsigned int k=0; k<NUM_CHARS; k++)
        {
            m_loadedGame->writeCharacter( k, m_party->m_chars[k]->serialize() );
            m_loadedGame->writeCharacterExtra( k, m_party->m_chars[k]->getCharExtra() );
        }

        if (! m_facts.isNull())
        {
            m_loadedGame->writeFacts( m_facts.serialize() );
        }

        m_loadedGame->close();

        statusBar()->showMessage(tr("Game saved."));
    }
    else
    {
        statusBar()->showMessage(tr("Cannot open file for save!"));
    }
}

void MainWindow::saveAsResetGame()
{
    if (isWizardry128())
    {
        QMessageBox::warning(this, tr("Feature Unavailable"),
            tr("New and Reset games are not supported under Wizardry 1.2.8\ndue to its modified file format."));

        return;
    }

    // Popup similar dialog as New. Warn that all game state will be removed
    // and it will be saved as a new file. Warn that this file can only be loaded by patched
    // versions of the game. Also warn that RPCs will be dropped.

    DialogNewFile *d = new DialogNewFile(this);

    d->setSaveAsResetDialog();

    if (d->exec() != QDialog::Accepted)
    {
        delete d;
        return;
    }

    m_barlone_dead = d->isBarloneDead();
    m_rodan_dead   = d->isRodanDead();

    switch (d->getScenarioIndex())
    {
        case StringList::W7EndUmpani:   m_w7_ending = wiz7_end::Umpani;  break;
        case StringList::W7EndTRang:    m_w7_ending = wiz7_end::TRang;   break;
        case StringList::W7EndOwnShip:  m_w7_ending = wiz7_end::OwnShip; break;
        case StringList::W8Virgin:      m_w7_ending = wiz7_end::Virgin;  break;
    }

    delete d;


    // Remove RPCs - reset gamestate for players
    for (unsigned int k=0; k<NUM_CHARS; k++)
    {
        if (m_party->m_chars[k]->isRPC())
        {
            // If you leave the RPC into a reset game, even with all their
            // attributes reset, the game crashes when you later encounter
            // that character for real. So if the game is getting reset, these
            // chars get dropped - equipment and all.
            delete m_party->m_chars[k];

            m_party->m_chars[k] = new character();
        }
        else
        {
            m_party->m_chars[k]->resetCharExtra();
        }
    }
    m_party->resetCharacterColors();

    if (m_loadedGame)
    {
        delete m_loadedGame;
        m_loadedGame = NULL;
    }

    // As of now it is the same as if a New file was made and the party
    // all imported. Use regular saveAs() to deal with it.
    saveAs();

    // in case RPC characters were deleted, we have refreshes to make
    if (ScreenCommon *s = qobject_cast<ScreenCommon *>(m_contentWidget))
    {
        s->updateChars( m_party );
    }
}

void MainWindow::saveAs()
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

    QString saveFile = ::getSaveFileName(NULL, tr("Save File"),  Path + "/Saves", tr("Saved Games (*.sav)"));
    if (saveFile.isEmpty())
        return;

    if (!saveFile.endsWith(".SAV", Qt::CaseInsensitive))
        saveFile += ".SAV";

    // This check isn't perfect for identifying duplicate refs to the same file,
    // since we can have relative paths, symlinks and hardlinks etc. but best I
    // can come up with for a quick patch to a file corrupting issue.
    if (m_loadedGame && (m_loadedGame->fileName().compare( saveFile ) == 0))
    {
        // Selected Save As for file currently loaded. That won't work, since
        // there is some file copying involved here - instead invoke the save()
        // routine.
        save();
        return;
    }

    QFile dst(saveFile);

    if (! dst.open(QFile::WriteOnly))
    {
        statusBar()->showMessage(tr("Cannot open file for save!"));
        QMessageBox::warning(this, tr("Save Error"),
            tr("Can't open file for writing in this directory."));
        return;
    }

    if (m_loadedGame)
    {
        dst.close();
        dst.remove();
        if (m_loadedGame->copy( saveFile ))
        {
            delete m_loadedGame;
            m_loadedGame = new RIFFFile(saveFile);
            if (m_loadedGame->isGood())
            {
                save();
            }
            else
            {
                QMessageBox::warning(this, tr("Save Error"), m_loadedGame->getError());
            }
        }
        else
        {
            statusBar()->showMessage(tr("Cannot write to file for save!"));
            QMessageBox::warning(this, tr("Save Error"),
                tr("Write error occurred trying to save new game file."));
        }
    }
    else
    {
        // Saving a 'New' game
        // This is a SAV game that won't load in unpatched versions of Wizardry 8
        // The only sections present in it are:
        //  * GSTA -- contains the party and the characters
        //  * GVER -- a game version identifier - constant values
        //  * SHOT -- a thumbnail to display in the file list in the game - 80x60 16 bit raw pixmap in scanline order
        //  * HACK -- a 6 byte RIFF section we made up that contains
        //              32 bit LE: Wizardry 7 ending: 1 = Own Ship, 2 = Umpani, 3 = T'Rang, 4 = Virgin start
        //               8 bit LE: FACT_BARLONE_WAS_DEAD (409)
        //               8 bit LE: FACT_RODAN_WAS_DEAD   (123)
        // The only effect of these 2 imported facts seems to be slightly different speech given
        // to the player when Rodan or Barlone are encountered. It doesn't alter gameplay logic
        // at all.

        quint8 b[4];

        // This can stay as hardcoded RIFF. We do not support creating new games under Wizardry 1.2.8,
        // where the WIZ8 header is used instead.
        dst.write( "RIFF", 4 );
        ASSIGN_LE8(  b, 0xff );      dst.write( (char *)b, sizeof(quint8)  );
        ASSIGN_LE8(  b, 0x00 );      dst.write( (char *)b, sizeof(quint8)  );
        ASSIGN_LE32( b, 80652 );     dst.write( (char *)b, sizeof(quint32) ); // file size is fixed
        ASSIGN_LE32( b, 4 );         dst.write( (char *)b, sizeof(quint32) ); // 4 sections

        dst.write( "GVER", 4 );
        ASSIGN_LE8(  b, 0x00 );      dst.write( (char *)b, sizeof(quint8)  );
        ASSIGN_LE8(  b, 0x00 );      dst.write( (char *)b, sizeof(quint8)  );
        ASSIGN_LE32( b, 12 );        dst.write( (char *)b, sizeof(quint32) ); // section size is fixed
        ASSIGN_LE32( b, 1 );         dst.write( (char *)b, sizeof(quint32) );
        ASSIGN_LE32( b, 2 );         dst.write( (char *)b, sizeof(quint32) );
        ASSIGN_LE32( b, 4 );         dst.write( (char *)b, sizeof(quint32) );

        dst.write( "SHOT", 4 );
        ASSIGN_LE8(  b, 0x00 );      dst.write( (char *)b, sizeof(quint8)  );
        ASSIGN_LE8(  b, 0x00 );      dst.write( (char *)b, sizeof(quint8)  );
        ASSIGN_LE32( b, 9608 );      dst.write( (char *)b, sizeof(quint32) ); // section size is fixed
        // If you do the math: 80 * 60 * 2 (bytes per pixel) == 9600
        // I don't know what those extra 8 bytes are supposed to be for
        dst.write( makeSnapshot( m_w7_ending ) );

        dst.write( "GSTA", 4 );
        ASSIGN_LE8(  b, 0x00 );      dst.write( (char *)b, sizeof(quint8)  );
        ASSIGN_LE8(  b, 0x00 );      dst.write( (char *)b, sizeof(quint8)  );
        ASSIGN_LE32( b, 70982 );     dst.write( (char *)b, sizeof(quint32) ); // section size is fixed

        QByteArray party = m_party->serialize();

        Q_ASSERT( party.size() == 0x49c2 );
        ASSIGN_LE32( b, party.size() );
        dst.write( (char *)b, sizeof(quint32) );
        dst.write( party );

        for (unsigned int k=0; k<NUM_CHARS; k++)
        {
            QByteArray ch = m_party->m_chars[k]->serialize();

            Q_ASSERT( ch.size() == RIFFFile::CHARACTER_SIZE );
            ASSIGN_LE32( b, ch.size() );
            dst.write( (char *)b, sizeof(quint32) );
            dst.write( ch );
        }
        for (unsigned int k=0; k<NUM_CHARS; k++)
        {
            QByteArray ch = m_party->m_chars[k]->getCharExtra();

            Q_ASSERT( ch.size() == RIFFFile::CHARACTER_EXTRA_SIZE );
            ASSIGN_LE32( b, ch.size() );
            dst.write( (char *)b, sizeof(quint32) );
            dst.write( ch );
        }


        dst.write( "HACK", 4 );
        ASSIGN_LE8(  b, 0x00 );      dst.write( (char *)b, sizeof(quint8)  );
        ASSIGN_LE8(  b, 0x00 );      dst.write( (char *)b, sizeof(quint8)  );
        ASSIGN_LE32( b, 6 );         dst.write( (char *)b, sizeof(quint32) ); // section size is fixed
        ASSIGN_LE32( b, (quint32)m_w7_ending );    dst.write( (char *)b, sizeof(quint32) );
        ASSIGN_LE8(  b, (quint8)m_barlone_dead );  dst.write( (char *)b, sizeof(quint8)  );
        ASSIGN_LE8(  b, (quint8)m_rodan_dead );    dst.write( (char *)b, sizeof(quint8)  );

        dst.close();
        m_loadedGame = new RIFFFile(saveFile);

        enableSave();
    }
}

void MainWindow::exit()
{
    // TODO: R U Sure box

    QWidget::close();
}

QByteArray MainWindow::makeSnapshot( wiz7_end clip )
{
    QString sti_filename;
    QImage  src;

    switch (clip)
    {
        case wiz7_end::Null:
            return QByteArray( 9608, 0 );

        case wiz7_end::OwnShip:   sti_filename = "Portraits/Large/LVi.sti";      break;
        case wiz7_end::Umpani:    sti_filename = "Portraits/Large/Lrodan.sti";   break;
        case wiz7_end::TRang:     sti_filename = "Portraits/Large/Ldrazic.sti";  break;
        case wiz7_end::Virgin:    sti_filename = "Portraits/Large/Lmook.sti";    break;
    }

    if (! sti_filename.isEmpty())
    {
        SLFFile slf( sti_filename );
        if (slf.isGood())
        {
            if (slf.open(QFile::ReadOnly))
            {
                QByteArray array = slf.readAll();
                STI c( array );

                src = c.getImage( 0 );

                slf.close();

                // This 16 bti image is actually in 1-5-5-5 (ARGB) format instead of the more usual 5:6:5 (RGB)
                // used everywhere else in the game. Don't know if the alpha channel is actually used
                QImage thumb = src.scaled( 80, 60, Qt::IgnoreAspectRatio, Qt::SmoothTransformation )
                                  .convertToFormat( QImage::Format_RGB555, Qt::AvoidDither );

                QByteArray picData = QByteArray( (const char *)thumb.constBits(), thumb.bytesPerLine() * thumb.height() );

                picData.prepend( (char *)"\x00\x00\x80\x3f\x01\xad", 6);
                picData.append( (char *)"\x00\x00", 2 );

                return picData;
            }
        }
    }

    return QByteArray( 9608, 0 );
}

void MainWindow::print()
{
    statusBar()->showMessage(tr("Invoked <b>File|Print</b>")); // The html styling doesn't actually work BTW
}

void MainWindow::undo()
{
    statusBar()->showMessage(tr("Invoked <b>Edit|Undo</b>"));
}

void MainWindow::redo()
{
    statusBar()->showMessage(tr("Invoked <b>Edit|Redo</b>"));
}

void MainWindow::cut()
{
    statusBar()->showMessage(tr("Invoked <b>Edit|Cut</b>"));
}

void MainWindow::copy()
{
    statusBar()->showMessage(tr("Invoked <b>Edit|Copy</b>"));
}

void MainWindow::paste()
{
    statusBar()->showMessage(tr("Invoked <b>Edit|Paste</b>"));
}

void MainWindow::ignoreModStrings()
{
    statusBar()->showMessage(tr("Invoked <b>Edit|Ignore Most Module Strings</b>"));

    setIgnoreModStrings( ! getIgnoreModStrings() );

    // in case RPC characters were deleted, we have refreshes to make
    if (ScreenCommon *s = qobject_cast<ScreenCommon *>(m_contentWidget))
    {
        s->resetLanguage();
    }
}

void MainWindow::enableLocalisation()
{
    statusBar()->showMessage(tr("Invoked <b>Edit|Enable Localisation</b>"));

    Localisation *loc = Localisation::getLocalisation();

    loc->setLocalisationActive( ! loc->isLocalisationActive() );

    QList<QAction *> actions = m_loc_langs->actions();
    for (int k=0; k<actions.size(); k++)
    {
        actions[k]->setEnabled( loc->isLocalisationActive() );
    }

    // in case RPC characters were deleted, we have refreshes to make
    if (ScreenCommon *s = qobject_cast<ScreenCommon *>(m_contentWidget))
    {
        s->resetLanguage();
    }
}

void MainWindow::changeLocalisation()
{
    if (QAction *a = qobject_cast<QAction *>(sender()))
    {
        Localisation *loc = Localisation::getLocalisation();
        loc->setLanguage( a->text() );

        // in case RPC characters were deleted, we have refreshes to make
        if (ScreenCommon *s = qobject_cast<ScreenCommon *>(m_contentWidget))
        {
            s->resetLanguage();
        }
    }
}

void MainWindow::droppedItems()
{
    if (m_droppedItems)
    {
        m_droppedItems->show();
        m_droppedItems->setWindowState(Qt::WindowState::WindowActive);
    }
    else
    {
        m_droppedItems = new WindowDroppedItems(m_party);
        connect( m_droppedItems, SIGNAL(windowClosing()), this, SLOT(droppedItemsClosed()) );
    }
}

void MainWindow::droppedItemsClosed()
{
    // The window close should have deleted the widget object itself
    m_droppedItems = NULL;
}

void MainWindow::factEditor()
{
    if (m_factEditor)
    {
        m_factEditor->show();
        m_factEditor->setWindowState(Qt::WindowState::WindowActive);
    }
    else
    {
        m_factEditor = new WindowFactEditor( m_facts );
        connect( m_factEditor, SIGNAL(windowClosing()), this, SLOT(factEditorClosed()) );
    }
}

void MainWindow::factEditorClosed()
{
    // The window close should have deleted the widget object itself
    m_factEditor = NULL;
}

void MainWindow::currentPosition()
{
    // This is a Dialog - main window is suspended until the dialog is closed
    float  position[3];

    m_party->getPosition( &position[0], &position[1], &position[2] );

    DialogCurrentPosition d( m_party->getMapId(),
                             position,
                             m_party->getHeading(),
                             m_party->getVisitedMaps() );

    if (d.exec() == QDialog::Accepted)
    {
        m_party->setMapId( d.getMapId() );

        d.getPosition( &position[0], &position[1], &position[2] );
        m_party->setPosition( position[0], position[1], position[2] );

        m_party->setHeading( d.getHeading() );
    }
}

void MainWindow::findItems()
{
    if (m_findItems)
    {
        m_findItems->show();
        m_findItems->setWindowState(Qt::WindowState::WindowActive);
    }
    else
    {
        if (ScreenCommon *s = qobject_cast<ScreenCommon *>(m_contentWidget))
        {
            m_findItems = new WindowItemsList( s->currentChar()->getProfession(),
                                               s->currentChar()->getRace(),
                                               s->currentChar()->getGender() );
        }
        else
        {
            m_findItems = new WindowItemsList( (character::profession) 0,
                                               (character::race) 0,
                                               (character::gender) 0 );
        }
        connect( m_findItems, SIGNAL(windowClosing()), this, SLOT(findItemsClosed()) );
    }
}

void MainWindow::findItemsClosed()
{
    // The window close should have deleted the widget object itself
    m_findItems = NULL;
}

void MainWindow::patchExe()
{
    statusBar()->showMessage(tr("Save new Wizardry 8 executable with selected patches baked in."));
    new DialogPatchExe(this);
}

void MainWindow::about()
{
    statusBar()->showMessage(tr("Invoked <b>Help|About</b>"));
    new DialogAbout(this);
}

void MainWindow::aboutUrho3D()
{
    statusBar()->showMessage(tr("Invoked <b>Help|About Urho3D Graphics Engine</b>"));
    new DialogAboutUrho3D(this);
}

void MainWindow::createActions()
{
    newAct = new QAction(tr("&New..."), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new game file and party."));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);

    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open a Wizardry 8 game file."));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);

    saveAct = new QAction(tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the edited Wizardry 8 game file."));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);

    saveAsAct = new QAction(tr("&Save As..."), this);
    saveAsAct->setStatusTip(tr("Save the Wizardry 8 game file as a new file."));
    connect(saveAsAct, &QAction::triggered, this, &MainWindow::saveAs);

    saveAsResetAct = new QAction(tr("&Save As Reset Game..."), this);
    saveAsResetAct->setStatusTip(tr("Remove all game state and begin from starting point."));
    connect(saveAsResetAct, &QAction::triggered, this, &MainWindow::saveAsResetGame);

    printAct = new QAction(tr("&Print..."), this);
    printAct->setShortcuts(QKeySequence::Print);
    printAct->setStatusTip(tr("Print the document"));
    printAct->setDisabled( true );
    //connect(printAct, &QAction::triggered, this, &MainWindow::print);

    exitAct = new QAction(tr("E&xit"), this);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, &QAction::triggered, this, &MainWindow::exit);

    undoAct = new QAction(tr("&Undo"), this);
    undoAct->setShortcuts(QKeySequence::Undo);
    undoAct->setStatusTip(tr("Undo the last operation"));
    undoAct->setDisabled( true );
    //connect(undoAct, &QAction::triggered, this, &MainWindow::undo);

    redoAct = new QAction(tr("&Redo"), this);
    redoAct->setShortcuts(QKeySequence::Redo);
    redoAct->setStatusTip(tr("Redo the last operation"));
    redoAct->setDisabled( true );
    //connect(redoAct, &QAction::triggered, this, &MainWindow::redo);

    cutAct = new QAction(tr("Cu&t"), this);
    cutAct->setShortcuts(QKeySequence::Cut);
    cutAct->setStatusTip(tr("Cut the current selection's contents to the "
                            "clipboard"));
    cutAct->setDisabled( true );
    //connect(cutAct, &QAction::triggered, this, &MainWindow::cut);

    copyAct = new QAction(tr("&Copy"), this);
    copyAct->setShortcuts(QKeySequence::Copy);
    copyAct->setStatusTip(tr("Copy the current selection's contents to the "
                             "clipboard"));
    copyAct->setDisabled( true );
    //connect(copyAct, &QAction::triggered, this, &MainWindow::copy);

    pasteAct = new QAction(tr("&Paste"), this);
    pasteAct->setShortcuts(QKeySequence::Paste);
    pasteAct->setStatusTip(tr("Paste the clipboard's contents into the current "
                              "selection"));
    pasteAct->setDisabled( true );
    //connect(pasteAct, &QAction::triggered, this, &MainWindow::paste);

    ignoreStringsAct = new QAction( ::getBaseStringTable()->getString( StringList::IgnoreModStringsShort ), this);
    ignoreStringsAct->setStatusTip( ::getBaseStringTable()->getString( StringList::IgnoreModStrings ) );
    ignoreStringsAct->setCheckable( true );
    ignoreStringsAct->setChecked( getIgnoreModStrings() );
    connect(ignoreStringsAct, &QAction::triggered, this, &MainWindow::ignoreModStrings);

    Localisation *loc = Localisation::getLocalisation();

    localisationAct = new QAction(tr("Enable Localisation"), this);
    localisationAct->setStatusTip( tr("Toggle Localisation on or off") );
    localisationAct->setCheckable( true );
    localisationAct->setChecked( loc->isLocalisationActive() );
    connect(localisationAct, &QAction::triggered, this, &MainWindow::enableLocalisation);

    QStringList avail = loc->getLanguagesAvailable();

    m_loc_langs = new QActionGroup(this);
    if (m_loc_langs)
    {
        m_loc_langs->setExclusive(true);

        for (int k=0; k<avail.size(); k++)
        {
            QAction *a = m_loc_langs->addAction( avail[k] );
            a->setCheckable( true );
            if (avail[k] == loc->getLanguage())
            {
                a->setChecked( true );
            }
            connect(a, &QAction::triggered, this, &MainWindow::changeLocalisation);
        }
    }

    droppedItemsAct = new QAction(tr("Dropped Items..."), this);
    droppedItemsAct->setStatusTip(tr("Show all dropped items to enable recovery"));
    connect(droppedItemsAct, &QAction::triggered, this, &MainWindow::droppedItems);

    findItemsAct = new QAction(tr("Search Items..."), this);
    findItemsAct->setStatusTip(tr("Filter and Sort all available items"));
    connect(findItemsAct, &QAction::triggered, this, &MainWindow::findItems);

    factEditorAct = new QAction(tr("Fact Editor..."), this);
    factEditorAct->setStatusTip(tr("Edit the game state fact variables"));
    connect(factEditorAct, &QAction::triggered, this, &MainWindow::factEditor);

    currPosAct = new QAction(tr("Current Position..."), this);
    currPosAct->setStatusTip(tr("Change the current saved game position"));
    connect(currPosAct, &QAction::triggered, this, &MainWindow::currentPosition);

    patchAct = new QAction(tr("Patch Wiz8.exe..."), this);
    patchAct->setStatusTip(tr("Modify Wizardry 8 with selected patches"));
    connect(patchAct, &QAction::triggered, this, &MainWindow::patchExe);

    aboutAct = new QAction(tr("&About"), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, &QAction::triggered, this, &MainWindow::about);

    aboutQtAct = new QAction(tr("About &Qt"), this);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);

    aboutUrhoAct = new QAction(tr("&About Urho3D"), this);
    aboutUrhoAct->setStatusTip(tr("Show the Urho3D library's About box"));
    connect(aboutUrhoAct, &QAction::triggered, this, &MainWindow::aboutUrho3D);
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addSeparator();
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(saveAsResetAct);
    fileMenu->addSeparator();
    fileMenu->addAction(printAct);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);
    editMenu->addSeparator();

    editMenu->addAction(ignoreStringsAct);
    if (::isWizardry128())
    {
        editMenu->addAction(localisationAct);

        QList<QAction *> actions = m_loc_langs->actions();
        for (int k=0; k<actions.size(); k++)
        {
            editMenu->addAction(actions[k]);
        }
    }

    specialMenu = menuBar()->addMenu(tr("&Special"));
    specialMenu->addAction(droppedItemsAct);
    specialMenu->addAction(findItemsAct);
    specialMenu->addSeparator();
    specialMenu->addAction(factEditorAct);
    specialMenu->addAction(currPosAct);
    specialMenu->addSeparator();
    specialMenu->addAction(patchAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
    helpMenu->addAction(aboutQtAct);
    helpMenu->addAction(aboutUrhoAct);
}
