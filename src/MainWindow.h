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

#include <QAbstractNativeEventFilter>
#include <QMainWindow>
#include <QMap>
#include <QSize>
#include "common.h"
#include "facts.h"

QT_BEGIN_NAMESPACE
class QAction;
class QActionGroup;
class QLabel;
class QMenu;
class QWidget;
QT_END_NAMESPACE

class RIFFFile;
class party;
class character;

#if defined(WIN32) || defined(WIN64)
class windowsEvents : public QAbstractNativeEventFilter
{
public:
    windowsEvents(){}

    bool nativeEventFilter(const QByteArray &eventType, void *message, long* res) override;
};
#endif

class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum class wiz7_end
    {
        Null    = 0,
        // 'Dead' ending isn't supported by Wizardry 8
        Umpani  = 1,
        TRang   = 2,
        OwnShip = 3,
        Virgin  = 4
    };

public:
    MainWindow(QString loadFile);
    ~MainWindow();

public slots:
    void droppedItemsClosed();
    void findItemsClosed();
    void factEditorClosed();
    void enableSave();
    void disableSave();

    void exit();

protected:
//#ifndef QT_NO_CONTEXTMENU
//    void contextMenuEvent(QContextMenuEvent *event) override;
//#endif // QT_NO_CONTEXTMENU

    QSize       sizeHint() const override;

private slots:
    void newFile();
    void open();
    void save();
    void saveAs();
    void saveAsResetGame();
    void print();
    void undo();
    void redo();
    void cut();
    void copy();
    void paste();
    void editPrefs();
    void ignoreModStrings();
    void enableLocalisation();
    void changeLocalisation();
    void droppedItems();
    void findItems();
    void factEditor();
    void currentPosition();
    void patchExe();
    void about();
    void aboutUrho3D();

    void       loadRPCfromNPCT( int npc_idx, int slot_idx );

private:
    QByteArray makeSnapshot( wiz7_end clip_id );

    void createActions();
    void createMenus();

    void       siftNPCs();
    QByteArray getRPC(int target_idx);
    bool       writeRPCinNPCT(int target_idx);
    void       addRPC(QByteArray rpc, int npc_idx, int npc_id);
    void       removeNpcFromLVLS(int npc_id);

    int        readMonsterIdxRecord();
    int        readMonsterDataRecord();

    QMap<int, int> buildMonsterNpcTable();

    QMap<int, int> m_npcMonsters;

    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *specialMenu;
    QMenu *helpMenu;
    QActionGroup *alignmentGroup;
    QAction *newAct;
    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;
    QAction *saveAsResetAct;
    QAction *printAct;
    QAction *exitAct;
    QAction *undoAct;
    QAction *redoAct;
    QAction *cutAct;
    QAction *copyAct;
    QAction *pasteAct;
    QAction *editPrefsAct;
    QAction *ignoreStringsAct;
    QAction *localisationAct;
    QAction *droppedItemsAct;
    QAction *findItemsAct;
    QAction *factEditorAct;
    QAction *currPosAct;
    QAction *patchAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
    QAction *aboutUrhoAct;

    QActionGroup *m_loc_langs;

    QLabel *infoLabel;

    QWidget   *m_contentWidget;

    QWidget   *m_droppedItems;
    QWidget   *m_findItems;
    QWidget   *m_factEditor;

    RIFFFile  *m_loadedGame;
    party     *m_party;
    facts      m_facts;

    QMap<int, QString>   m_rpcMap;

    // only used if we're making a 'New' game (ie. m_loadedGame == NULL
    wiz7_end   m_w7_ending;
    bool       m_barlone_dead;
    bool       m_rodan_dead;
};
