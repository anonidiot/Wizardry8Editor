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

#include <QMediaPlayer>
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
 #include <QRandomGenerator>
#else
 #include <stdlib.h>
#endif

#include <QApplication>
#include <QHelpEvent>
#include <QFile>

#include "common.h"
#include "ScreenCommon.h"
#include "ScreenPersonality.h"

#include "StringList.h"
#include "WButton.h"
#include "WCheckBox.h"
#include "WImage.h"
#include "WLabel.h"
#include "WLineEdit.h"
#include <QButtonGroup>
#include <QPushButton>
#include <QPixmap>

#include "SLFFile.h"

#include "main.h"

#include <QDebug>

#define REMOVE_DUPLICATE_PORTRAITS

typedef enum
{
    NO_ID,

    VAL_FULL_NAME,
    VAL_NICKNAME,

    PERS_START,
    CB_PERS1 = PERS_START,
    CB_PERS2,
    CB_PERS3,
    CB_PERS4,
    CB_PERS5,
    CB_PERS6,
    CB_PERS7,
    CB_PERS8,
    CB_PERS9,
    PERS_END,

    VOICE_START,
    CB_VOICE1 = VOICE_START,
    CB_VOICE2,
    VOICE_END,

    VAL_AUDIO_LEVEL,
    VAL_QUOTE,
    QUOTE_PLAY,

    VAL_MUGSHOT,
    VAL_MUGNAME,
    VAL_MUGRACE,
    VAL_MUGPROF,
    VAL_MUGLEVEL,
    VAL_FACELIFT,

    PERS_REPLACE,
    PERS_UNDO,
    PERS_PREV,
    PERS_CANCEL,
    PERS_EXIT,

    SIZE_WIDGET_IDS
} widget_ids;

ScreenPersonality::ScreenPersonality(character *c, QWidget *parent) :
    Screen(parent), // This screen actually _does_ disable the ScreenCommon widgets, unlike the others
    m_char(c),
    m_mugIdx(0),
    m_audio_player(NULL),
    m_visualiser(NULL),
    m_snd_file(NULL)
{
    // All these controls are added as children of this widget, and hence will be destructed automatically
    // when we are destroyed

    struct layout itemsScrn[] =
    {
        { NO_ID,              QRect(   0,   0,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PIECES.STI",                   1,              this ),  -1,  NULL },
        { NO_ID,              QRect( 195,   0,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PIECES.STI",                   0,              this ),  -1,  NULL },
        { NO_ID,              QRect(  44, 450,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PIECES.STI",                   4,              this ),  -1,  NULL },
        { NO_ID,              QRect( 195,  43,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PERSONALITY.STI",              0,              this ),  -1,  NULL },
        { NO_ID,              QRect(   4, 290, 200,  50 ),    new WLabel(    StringList::InstructPersonality, Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },

        { NO_ID,              QRect( 236,  64,  54,  30 ),    new WLabel(    StringList::PrevRace,            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 296,  69,  -1,  -1 ),    new WButton(   "CHAR GENERATION/CG_BUTTONS.STI",                  0, true,  1.0,  this ),  -1,  SLOT(prevRace(bool)) },
        { NO_ID,              QRect( 548,  64,  54,  30 ),    new WLabel(    StringList::NextRace,            Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 520,  69,  -1,  -1 ),    new WButton(   "CHAR GENERATION/CG_BUTTONS.STI",                  5, true,  1.0,  this ),  -1,  SLOT(nextRace(bool)) },
        { NO_ID,              QRect( 236, 143,  54,  30 ),    new WLabel(    StringList::PrevPortrait,        Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 296, 147,  -1,  -1 ),    new WButton(   "CHAR GENERATION/CG_BUTTONS.STI",                  0, true,  1.0,  this ),  -1,  SLOT(prevFace(bool)) },
        { NO_ID,              QRect( 548, 143,  54,  30 ),    new WLabel(    StringList::NextPortrait,        Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },
        { NO_ID,              QRect( 520, 147,  -1,  -1 ),    new WButton(   "CHAR GENERATION/CG_BUTTONS.STI",                  5, true,  1.0,  this ),  -1,  SLOT(nextFace(bool)) },
        { NO_ID,              QRect( 259, 214,  60,  12 ),    new WLabel(    StringList::FullName,            Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_FULL_NAME,      QRect( 344, 212, 265,  19 ),    new WLineEdit( "",                              Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  SLOT(nameChanged(const QString&)) },
        { NO_ID,              QRect( 259, 242,  60,  12 ),    new WLabel(    StringList::NickName,            Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  NULL },
        { VAL_NICKNAME,       QRect( 344, 240, 265,  19 ),    new WLineEdit( "",                              Qt::AlignLeft,   10, QFont::Thin, this ),  -1,  SLOT(nameChanged(const QString&)) },
        { NO_ID,              QRect( 217, 278, 411,  16 ),    new WLabel(    StringList::PersonalityAndVoice, Qt::AlignCenter, 10, QFont::Thin, this ),  -1,  NULL },

        { CB_PERS1,           QRect( 231, 303, 127,  13 ),    new WCheckBox( StringList::LISTPersonalities + static_cast<int>(character::personality::Aggressive),   this ),  -1,  SLOT(changePersonalityVoice(int)) },
        { CB_PERS2,           QRect( 359, 303, 127,  13 ),    new WCheckBox( StringList::LISTPersonalities + static_cast<int>(character::personality::Intellectual), this ),  -1,  SLOT(changePersonalityVoice(int)) },
        { CB_PERS3,           QRect( 487, 303, 127,  13 ),    new WCheckBox( StringList::LISTPersonalities + static_cast<int>(character::personality::Burly),        this ),  -1,  SLOT(changePersonalityVoice(int)) },
        { CB_PERS4,           QRect( 231, 317, 127,  13 ),    new WCheckBox( StringList::LISTPersonalities + static_cast<int>(character::personality::Chaotic),      this ),  -1,  SLOT(changePersonalityVoice(int)) },
        { CB_PERS5,           QRect( 359, 317, 127,  13 ),    new WCheckBox( StringList::LISTPersonalities + static_cast<int>(character::personality::Cunning),      this ),  -1,  SLOT(changePersonalityVoice(int)) },
        { CB_PERS6,           QRect( 487, 317, 127,  13 ),    new WCheckBox( StringList::LISTPersonalities + static_cast<int>(character::personality::Eccentric),    this ),  -1,  SLOT(changePersonalityVoice(int)) },
        { CB_PERS7,           QRect( 231, 331, 127,  13 ),    new WCheckBox( StringList::LISTPersonalities + static_cast<int>(character::personality::Kindly),       this ),  -1,  SLOT(changePersonalityVoice(int)) },
        { CB_PERS8,           QRect( 359, 331, 127,  13 ),    new WCheckBox( StringList::LISTPersonalities + static_cast<int>(character::personality::Laidback),     this ),  -1,  SLOT(changePersonalityVoice(int)) },
        { CB_PERS9,           QRect( 487, 331, 127,  13 ),    new WCheckBox( StringList::LISTPersonalities + static_cast<int>(character::personality::Loner),        this ),  -1,  SLOT(changePersonalityVoice(int)) },
        { CB_VOICE1,          QRect( 228, 360, 127,  13 ),    new WCheckBox( StringList::Voice1,                                                this ),  -1,  SLOT(changePersonalityVoice(int)) },
        { CB_VOICE2,          QRect( 228, 389, 127,  13 ),    new WCheckBox( StringList::Voice2,                                                this ),  -1,  SLOT(changePersonalityVoice(int)) },
        { VAL_AUDIO_LEVEL,    QRect( 555, 357,  -1,  -1 ),    new WImage(    "CHAR GENERATION/CG_PERSONALITY.STI",              1,              this ),  -1,                            NULL },
        { VAL_QUOTE,          QRect( 306, 354, 227,  54 ),    new WLabel(    "",                              Qt::AlignCenter, 10, QFont::Thin, this ),  -1,                            NULL },
        { QUOTE_PLAY,         QRect( 560, 384,  -1,  -1 ),    new WButton(   "CHAR GENERATION/CG_BUTTONS.STI",                 20, false, 1.0,  this ),  StringList::PlayVoiceSample,   SLOT(quotePlay(bool)) },

        { VAL_FACELIFT,       QRect( 328,  46,  -1,  -1 ),    new WImage(    "Portraits/Large/Lhummf.sti",                      0,              this ),  -1,                            NULL },
        { VAL_MUGSHOT,        QRect(   8,   9,  -1,  -1 ),    new WImage(    "Portraits/Large/Lhummf.sti",                      0,              this ),  -1,                            NULL },
        { VAL_MUGNAME,        QRect(   0, 165, 194,  12 ),    new WLabel(    "",                              Qt::AlignCenter, 10, QFont::Thin, this ),  -1,                            NULL },
        { VAL_MUGRACE,        QRect(   0, 179, 194,  12 ),    new WLabel(    "",                              Qt::AlignCenter, 10, QFont::Thin, this ),  -1,                            NULL },
        { VAL_MUGPROF,        QRect(   0, 193, 194,  12 ),    new WLabel(    "",                              Qt::AlignCenter, 10, QFont::Thin, this ),  -1,                            NULL },
        { VAL_MUGLEVEL,       QRect(   0, 207, 194,  12 ),    new WLabel(    "",                              Qt::AlignCenter, 10, QFont::Thin, this ),  -1,                            NULL },

        { PERS_REPLACE,       QRect(   0, 450,  -1,  -1 ),    new WButton( "CHAR GENERATION/CG_BOTTOMBUTTONS.STI",             29, true,  1.0,  this ),  StringList::ReplaceCharacter,  NULL },
        { PERS_CANCEL,        QRect( 464, 450,  -1,  -1 ),    new WButton( "CHAR GENERATION/CG_BOTTOMBUTTONS.STI",              4, true,  1.0,  this ),  StringList::CancelAndExit,     SLOT(abortPersonalityScreen(bool)) },
        { PERS_UNDO,          QRect( 508, 450,  -1,  -1 ),    new WButton( "CHAR GENERATION/CG_BOTTOMBUTTONS.STI",             20, true,  1.0,  this ),  StringList::UndoChanges,       SLOT(undoPersonalityScreen(bool)) },
        { PERS_PREV,          QRect( 552, 450,  -1,  -1 ),    new WButton( "CHAR GENERATION/CG_BOTTOMBUTTONS.STI",             12, true,  1.0,  this ),  StringList::PreviousPage,      NULL },
        { PERS_EXIT,          QRect( 596, 450,  -1,  -1 ),    new WButton( "CHAR GENERATION/CG_BOTTOMBUTTONS.STI",             16, true,  1.0,  this ),  StringList::Done,              SLOT(exitPersonalityScreen(bool)) },

    };

    int num_widgets = sizeof(itemsScrn) / sizeof(struct layout);

    m_widgets = Screen::widgetInit( itemsScrn, num_widgets, this );

    // Make the checkboxes representing personality on the Personality and Voice
    // screen work as a group (like radiobuttons)
    m_personality_cb_group = new QButtonGroup(this);
    for (int k=PERS_START; k < PERS_END; k++)
    {
        m_personality_cb_group->addButton( qobject_cast<QAbstractButton *>(m_widgets[ k ]), k );
    }
    m_voice_cb_group = new QButtonGroup(this);
    for (int k=VOICE_START; k < VOICE_END; k++)
    {
        m_voice_cb_group->addButton( qobject_cast<QAbstractButton *>(m_widgets[ k ]), k );
    }

    // load the pixmaps used for the audio playback levels
    SLFFile imgs( "CHAR GENERATION/CG_PERSONALITY.STI" );
    if (imgs.open(QFile::ReadOnly))
    {
        QByteArray array = imgs.readAll();
        STItoQImage sti_imgs( array );

        m_audioLevels[0] = QPixmap::fromImage( sti_imgs.getImage( 1 ));
        m_audioLevels[1] = QPixmap::fromImage( sti_imgs.getImage( 2 ));
        m_audioLevels[2] = QPixmap::fromImage( sti_imgs.getImage( 3 ));
        m_audioLevels[3] = QPixmap::fromImage( sti_imgs.getImage( 4 ));

        imgs.close();
    }

    resetScreen( m_char, NULL );

    // This button is only here because it's present in the original UI. We permanently
    // disable it. If you want to replace a character you do it by right clicking on the
    // small image in the upper left corner where the party is shown in the Main Window.
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ PERS_REPLACE ]))
    {
        q->setDisabled( true );
    }
    // There's no previous page in this case
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ PERS_PREV ]))
    {
        q->setDisabled( true );
    }
    // Initially disabled, enables when anything changed
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ PERS_UNDO ]))
    {
        q->setDisabled( true );
    }
}

ScreenPersonality::~ScreenPersonality()
{
    if (m_audio_player)
    {
        m_audio_player->stop();
    }
    if (m_visualiser)
    {
        delete m_visualiser;
    }
    if (m_audio_player)
    {
        delete m_audio_player;
    }
    if (m_snd_file)
    {
        m_snd_file->remove();
        delete m_snd_file;
    }

    delete m_personality_cb_group;
    delete m_voice_cb_group;
}


void ScreenPersonality::resetScreen(void *char_tag, void *party_tag)
{
    (void)party_tag;

    m_char = (character *)char_tag;

    // Populate text value fields with values from the character

    struct { int id; QString str; } vals[] =
    {
        { VAL_FULL_NAME, m_char->getFullName()                                          },
        { VAL_NICKNAME,  m_char->getName()                                              },
        { VAL_MUGNAME,   m_char->getName()                                              },
        { VAL_MUGRACE,   m_char->getGenderString() + " " + m_char->getRaceString()      },
        { VAL_MUGPROF,   m_char->getProfessionString()                                  },
        { VAL_MUGLEVEL,  ::getStringTable()->getString( StringList::Level ) +
                         " " + QString::number(m_char->getCurrentLevel()) +
                         " (" + m_char->getCurrentLevelString() + ")"                   },

        { -1, "" }
    };

    for (int k=0; vals[k].id != -1; k++)
    {
        if (QLabel *q = qobject_cast<QLabel *>(m_widgets[ vals[k].id ]))
        {
            q->setText( vals[k].str );
        }
        else if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[ vals[k].id ]))
        {
            q->setText( vals[k].str );
        }
    }

    // Populate the data on the Personality screen (shown when
    // you click the character's name.
    int p = static_cast<int>(m_char->getPersonality());
    for (int k=PERS_START; k < PERS_END; k++)
    {
        if (PERS_START + p == k)
            qobject_cast<QCheckBox *>(m_widgets[ k ])->setCheckState( Qt::Checked );
        else
            qobject_cast<QCheckBox *>(m_widgets[ k ])->setCheckState( Qt::Unchecked );
    }
    p = m_char->getVoice();
    for (int k=VOICE_START; k < VOICE_END; k++)
    {
        if (VOICE_START + p == k)
            qobject_cast<QCheckBox *>(m_widgets[ k ])->setCheckState( Qt::Checked );
        else
            qobject_cast<QCheckBox *>(m_widgets[ k ])->setCheckState( Qt::Unchecked );
    }
    if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_MUGSHOT ]))
    {
        m_mugIdx = m_char->getPortraitIndex();

        q->setPixmap( ScreenCommon::getLargePortrait( m_mugIdx ) );
    }
    if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_FACELIFT ]))
    {
        q->setPixmap( ScreenCommon::getLargePortrait( m_mugIdx ) );
    }
}

void ScreenPersonality::setVisible( bool visible )
{
    if (visible)
    {
        // We need it to populate the message text the first time through
        changePersonalityVoice(Qt::Checked);
    }

    QObjectList kids = children();

    for (int k=0; k<kids.size(); k++)
    {
        if (QWidget *q = qobject_cast<QWidget *>(kids.at(k)))
        {
            q->setVisible( visible );
        }
    }
    QWidget::setVisible( visible );
}

void ScreenPersonality::resizeEvent(QResizeEvent *event)
{
    (void)event;
}

// @race = true  -> next/prev race
// @race = false -> next/prev face
// @up   = true  -> next
// @up   = false -> prev
int ScreenPersonality::nextImageIdx(bool race, bool up)
{
    // The base game has several duplicate portaits in it we wish
    // to filter out. But of course a mod can go and change these
    // and they might not actually be duplicates for it.
    // While the Mook/Urq portraits and the Umpani/Rodan portraits
    // are binary identical though, the other 3 aren't, so we can't
    // just automatically decide what to do here.
    // Currently it's a compile time switch; probably should be a
    // userspace setting at some point
#ifdef REMOVE_DUPLICATE_PORTRAITS
    // First get rid of the duplicate indexes - apologies to any mods
    // who actually changed these ones.
    // FIXME: Make this a user preference
    switch (m_mugIdx)
    {
        case 58:           // "mook.sti",   // double up of Urq
            m_mugIdx = 65; // "urq.sti",
            break;
        case 59:           // "trynm1.sti", // double up of Madras
            m_mugIdx = 67; // "madras.sti",
            break;
        case 60:           // "trynm2.sti", // double up of Sparkle
            m_mugIdx = 68; // "sparkle.sti",
            break;
        case 62:           // "trang.sti",  // double up of Drazic
            m_mugIdx = 71; // "drazic.sti",
            break;
        case 63:           // "umpani.sti", // double up of Rodan
            m_mugIdx = 73; // "rodan.sti",
    }
#endif

    static const int img_idx[][17] =
    {
        /* Humans    */ { /* Male */  0,  1,  2,  3, 76,  4,  5, 69 /* Myles */,
                          /*Female*/  6,  7,  8,  9, 77, 10, 11, 70 /* Vi */, -1 },
        /* Elves     */ { /* Male */ 12, 13, 14, 78,
                          /*Female*/ 15, 16, 17, 79, -1 },
        /* Dwarves   */ { /* Male */ 18, 19, 20,
                          /*Female*/ 21, 22, 23, -1 },
        /* Gnomes    */ { /* Male */ 24, 25,
                          /*Female*/ 26, 27, -1 },
        /* Hobbits   */ { /* Male */ 28, 29,
                          /*Female*/ 30, 31, -1 },
        /* Fairies   */ { /* Male */ 32, 33,
                          /*Female*/ 34, 35, -1 },
        /* Lizardmen */ { /* Male */ 36, 37,
                          /*Female*/ 38, 39, -1 },
        /* Dracons   */ { /* Male */ 40, 41,
                          /*Female*/ 42, 43, -1 },
        /* Felpurrs  */ { /* Male */ 44, 45,
                          /*Female*/ 46, 47, -1 },
        /* Rawulfs   */ { /* Male */ 48, 49,
                          /*Female*/ 50, 51, -1 },
#ifdef REMOVE_DUPLICATE_PORTAITS
        /* Mook      */ { /* Male */ 52, 53, 65 /* Urq */,
#else
        /* Mook      */ { /* Male */ 52, 53, 65 /* Urq */, 58,
#endif
                          /*Female*/ 54, 55, -1 },
        /* Ninja     */ { /* Male */ 56,
                          /*Female*/ 57, -1 },
#ifdef REMOVE_DUPLICATE_PORTRAITS
        /* Trynnie   */ { /* Male */ 61, 67 /* Madras */,
                          /*Female*/ 68 /* Sparkle */, -1 },
        /* T'Rang    */ { /* Male */ 71 /* Drazic */, 72 /* Tantris */, -1 },
        /* Umpani    */ { /* Male */ 73 /* Rodan */, 74 /* Glumph */, 75 /* Saxx */, -1 },
#else
        /* Trynnie   */ { /* Male */ 61, 67 /* Madras */, 59,
                          /*Female*/ 68 /* Sparkle */, 60, -1 },
        /* T'Rang    */ { /* Male */ 71 /* Drazic */, 72 /* Tantris */, 62, -1 },
        /* Umpani    */ { /* Male */ 73 /* Rodan */, 74 /* Glumph */, 75 /* Saxx */, 63, -1 },
#endif
        /* Rapax     */ { /* Male */ 64 /* Sexus */, -1 },
        /* Android   */ {            66 /* RFS-81 */, -1 }
    };

    for (unsigned int k=0; k<sizeof(img_idx)/sizeof(img_idx[0]); k++)
    {
        for (int j=0; img_idx[k][j] != -1; j++)
        {
            if (img_idx[k][j] == m_mugIdx)
            {
                if (race == false)
                {
                    // next / previous face within a race

                    if (up == false)
                    {
                        if (j > 0)
                            return img_idx[k][j-1];
                        for (int i=j; img_idx[k][i] != -1; i++)
                        {
                            if (img_idx[k][i+1] == -1)
                                return img_idx[k][i];
                        }
                        Q_ASSERT(0);
                        return -1;
                    }
                    else
                    {
                        if (img_idx[k][j+1] != -1)
                            return img_idx[k][j+1];
                        return img_idx[k][0];
                    }
                }
                else
                {
                    // next / previous race
                    if (up == false)
                    {
                        if (k > 0)
                            return img_idx[k-1][0];
                        return img_idx[ sizeof(img_idx)/sizeof(img_idx[0]) - 1 ][0];
                    }
                    else
                    {
                        if (k < sizeof(img_idx)/sizeof(img_idx[0]))
                            return img_idx[k+1][0];
                        return img_idx[0][0];
                    }
                }
            }
        }
    }
    return -1;
}

void ScreenPersonality::nextRace(bool)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        q->setChecked(false);

        if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_FACELIFT ]))
        {
            m_mugIdx = nextImageIdx(true, true);
            q->setPixmap( ScreenCommon::getLargePortrait( m_mugIdx ) );
        }
        // enable the Undo button now
        if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ PERS_UNDO ]))
        {
            q->setDisabled( false );
        }
    }
}

void ScreenPersonality::prevRace(bool)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        q->setChecked(false);

        if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_FACELIFT ]))
        {
            m_mugIdx = nextImageIdx(true, false);
            q->setPixmap( ScreenCommon::getLargePortrait( m_mugIdx ) );
        }
        // enable the Undo button now
        if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ PERS_UNDO ]))
        {
            q->setDisabled( false );
        }
    }
}

void ScreenPersonality::nextFace(bool)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        q->setChecked(false);

        if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_FACELIFT ]))
        {
            m_mugIdx = nextImageIdx(false, true);
            q->setPixmap( ScreenCommon::getLargePortrait( m_mugIdx ) );
        }
        // enable the Undo button now
        if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ PERS_UNDO ]))
        {
            q->setDisabled( false );
        }
    }
}

void ScreenPersonality::prevFace(bool)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        q->setChecked(false);

        if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_FACELIFT ]))
        {
            m_mugIdx = nextImageIdx(false, false);
            q->setPixmap( ScreenCommon::getLargePortrait( m_mugIdx ) );
        }
        // enable the Undo button now
        if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ PERS_UNDO ]))
        {
            q->setDisabled( false );
        }
    }
}


void ScreenPersonality::nameChanged(const QString &)
{
    // enable the Undo button now
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ PERS_UNDO ]))
    {
        q->setDisabled( false );
    }
}

void ScreenPersonality::audioPlayerStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status)
    {
        case QMediaPlayer::NoMedia:
        case QMediaPlayer::StalledMedia:
        case QMediaPlayer::EndOfMedia:
        case QMediaPlayer::InvalidMedia:
            // treat it as playback over
            if (m_visualiser)
            {
                m_visualiser->deleteLater();
                m_visualiser = NULL;
            }
            if (m_audio_player)
            {
                m_audio_player->deleteLater();
                m_audio_player = NULL;
            }
            if (m_snd_file)
            {
                m_snd_file->remove();
                m_snd_file->deleteLater();
                m_snd_file = NULL;
            }
            // Pop the play button back up
            if (QPushButton *q = qobject_cast<QPushButton *>(m_widgets[ QUOTE_PLAY ]))
            {
                q->setChecked(false);
            }
            break;

        default:
            break;
    }
}

void ScreenPersonality::audioPlayerProbe(const QAudioBuffer &buffer)
{
    // The real Wizardry just iterates over the frames when playing
    // the audio. Seems a better choice to animate it based on the
    // audio levels, though, so done that here.
    if (buffer.format().isValid() &&
        (0 == buffer.format().codec().compare("audio/pcm")))
    {
        qreal audio_level  = 0;

        int frames = buffer.frameCount();
        int chans  = buffer.format().channelCount();

        const quint8 *bufr = buffer.constData<quint8>();
        for (int i=0; i < frames; i++)
        {
            for (int j=0; j < chans; j++)
            {
                qreal peak_value = 0;

                union
                {
                    qint32  i;
                    quint32 u;
                    float   f;
                    qint16  ih;
                    quint16 uh;
                    qint8   iq;
                    quint8  uq;
                } val;

                switch (buffer.format().sampleSize())
                {
                    case 32:
                        peak_value = qreal(0x7fffffff);
                        val.u = FORMAT_LE32( bufr );
                        bufr += 4;
                        break;
                    case 16:
                        peak_value = qreal(0x00007fff);
                        val.uh = FORMAT_LE16( bufr );
                        val.i = val.ih; // sign extend
                        bufr += 2;
                        break;
                    case 8:
                        peak_value = qreal(0x0000007f);
                        val.uq = FORMAT_8(    bufr );
                        val.i = val.iq; // sign extend
                        bufr++;
                        break;
                }
                switch (buffer.format().sampleType())
                {
                    case QAudioFormat::Unknown:
                    case QAudioFormat::UnSignedInt:
                    {
                        qreal r = qAbs((qreal)val.u - peak_value) / peak_value;
                        audio_level = qMax( audio_level, r);
                        break;
                    }
                    case QAudioFormat::Float:
                        audio_level = qMax( audio_level, (qreal)qAbs(val.f) / qreal(1.00003));
                        break;
                    case QAudioFormat::SignedInt:
                        audio_level = qMax( audio_level, (qreal)qAbs(val.i) / peak_value);
                        break;
                }
            }
        }

        if (WImage *q = qobject_cast<WImage *>(m_widgets[ VAL_AUDIO_LEVEL ]))
        {
            int level = (audio_level / 0.25);
            if (level >= 3)
                level = 3;

            q->setPixmap( m_audioLevels[ level ] );
        }
    }
}

void ScreenPersonality::quotePlay(bool down)
{
    if (down)
    {
        // play the file
        QString persFile = generatePersonalityFilename();

        SLFFile mp3quote( "SOUND.SLF", "PCS/" + persFile + "/" + persFile + "_000.MP3" );

        if (!mp3quote.open(QFile::ReadOnly))
        {
            qWarning() << "Could not open file" << mp3quote.fileName();
        }
        else
        {
            Q_ASSERT(!m_snd_file);
            if (m_snd_file)
            {
                m_snd_file->remove();
                delete m_snd_file;
            }

            // The SLFFile does not inherit from QFile (see comments in SLFFile
            // class for why this is the case).
            // This means we can't pass through a reference directly to QMediaPlayer
            // and need to copy our data to a temporary file in order to play it.
            // Unfortunately the QTemporaryFile class only works in linux. If you
            // attempt to use it in Windows you get a 0x80070020 sharing violation
            // error from DirectShow - so we have to manage and cleanup the file
            // manually ourselves.
            m_snd_file = new QFile( QDir::tempPath() + QDir::separator() + "Vocab_" +
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
                             QString::number(QRandomGenerator::global()->generate())
#else
                             QString::number(rand())
#endif
                             + ".mp3" );
            m_snd_file->open(QIODevice::WriteOnly);

            while (true)
            {
                char buffer[4096];

                qint64 len = mp3quote.read(buffer, sizeof(buffer));
                if (len < 1)
                    break;
                m_snd_file->write(buffer, len);
            }
            m_snd_file->close();

            Q_ASSERT(!m_audio_player);
            if (m_audio_player == NULL)
            {
                m_audio_player = new QMediaPlayer;
                Q_ASSERT(m_audio_player);
                connect(m_audio_player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)),
                                  this, SLOT(audioPlayerStatusChanged(QMediaPlayer::MediaStatus)));

                m_visualiser = new QAudioProbe;
                Q_ASSERT(m_visualiser);
                m_visualiser->setSource( m_audio_player );
                connect(m_visualiser, SIGNAL(audioBufferProbed(const QAudioBuffer&)),
                                this, SLOT(audioPlayerProbe(const QAudioBuffer&)));
            }
            m_audio_player->setMedia(QMediaContent(QUrl::fromLocalFile(m_snd_file->fileName())), nullptr);
            m_audio_player->setVolume(50);
            m_audio_player->play();
        }
    }
    else
    {
        // stop the file
        m_audio_player->stop();

        delete m_visualiser;
        m_visualiser = NULL;

        delete m_audio_player;
        m_audio_player = NULL;

        if (m_snd_file)
        {
            m_snd_file->remove();
            delete m_snd_file;
            m_snd_file = NULL;
        }
    }
}

QString ScreenPersonality::generatePersonalityFilename()
{
    QString msgFilename = "";

    switch (m_char->getGender())
    {
        case character::gender::Male:     msgFilename += "M_";     break;
        case character::gender::Female:   msgFilename += "F_";     break;
        default:
            return "";
    }

    // when the initial value is being set on the checkboxes, this
    // callback fires, and the buttongroups don't have the correct
    // values, so return if we detect that situation. We need BOTH
    // buttongroups providing correct values in order to proceed.
    switch (m_personality_cb_group->checkedId())
    {
        case CB_PERS1: /* Aggressive   */ msgFilename += "AGGR";   break;
        case CB_PERS2: /* Intellectual */ msgFilename += "INTELL"; break;
        case CB_PERS3: /* Burly        */ msgFilename += "BURLY";  break;
        case CB_PERS4: /* Chaotic      */ msgFilename += "CHAOS";  break;
        case CB_PERS5: /* Cunning      */ msgFilename += "CUN";    break;
        case CB_PERS6: /* Eccentric    */ msgFilename += "ECC";    break;
        case CB_PERS7: /* Kindly       */ msgFilename += "KIND";   break;
        case CB_PERS8: /* Laidback     */ msgFilename += "LAID";   break;
        case CB_PERS9: /* Loner        */ msgFilename += "LONER";  break;
        default:
            return "";
    }

    switch (m_voice_cb_group->checkedId())
    {
        case CB_VOICE1:                   msgFilename += "10"; break;
        case CB_VOICE2:                   msgFilename += "20"; break;
        default:
            return "";
    }
    return msgFilename;
}

void ScreenPersonality::changePersonalityVoice(int state)
{
    if (state == Qt::Checked)
    {
        QString msg_file =  generatePersonalityFilename();

        // This slot gets triggered as part of the widget setup, so
        // prevent it from logging an error in SLFFile if we don't
        // have enough state information yet to produce a filename.

        if (msg_file.size() > 0)
        {
            SLFFile quote_file( "QUOTES/PCS/" + msg_file + ".MSG" );

            if (!quote_file.open(QFile::ReadOnly))
            {
                qWarning() << "Could not open file" << quote_file.fileName();
            }
            else
            {
                quote_file.skip(13);

                int quote_len = quote_file.readLEULong();

                QByteArray quote_enc = quote_file.read( quote_len * 2 ); // encrypted UTF-16

                // The text is encrypted (why?) with a basic cypher
                QString quote = StringList::decipher( quote_enc );

                // Shove that text onto a QLabel after putting quotes around it.
                QLabel *q = qobject_cast<QLabel *>(m_widgets[ VAL_QUOTE ]);

                q->setWordWrap( true );
                q->setText( "\"" + quote.trimmed() + "\"" );
            }

            // This callback gets called even when the checkbox states are changed
            // programmatically. And that's a pest for our undo state logic.
            // We do actually need an initialisation call to this to explicitly
            // setup the first message string, but that's already done in the
            // setVisible() method. And we know when that's been triggered because
            // the sender() is NULL.
            // So in order to undo the damage caused by the unwanted signals at
            // startup we not only enable the undo button on a legitimate change,
            // but we also have to DISABLE it on the startup condition.

            if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ PERS_UNDO ]))
            {
                if (sender() == NULL)
                    q->setDisabled( true );
                else
                    q->setDisabled( false );
            }
        }
    }
}

void ScreenPersonality::undoPersonalityScreen(bool)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        q->setChecked(false);
    }

    // FIXME: Ask if they wish to discard changes
    resetScreen( m_char, NULL );

    // disable the Undo button again
    if (QAbstractButton *q = qobject_cast<QAbstractButton *>(m_widgets[ PERS_UNDO ]))
    {
        q->setDisabled( true );
    }
}

void ScreenPersonality::abortPersonalityScreen(bool)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        // even though we're now hiding this button, if we don't do
        // this, the state persists for the next time the screen gets
        // shown, and it has the wrong lingering icon then.
        q->setChecked(false);
    }
    // FIXME: Ask if they wish to discard changes

    setVisible( false );
    this->setParent(NULL);
    this->deleteLater();
}

void ScreenPersonality::exitPersonalityScreen(bool)
{
    if (QPushButton *q = qobject_cast<QPushButton *>(this->sender()))
    {
        // even though we're now hiding this button, if we don't do
        // this, the state persists for the next time the screen gets
        // shown, and it has the wrong lingering icon then.
        q->setChecked(false);
    }

    // Save changes

    if (m_mugIdx != m_char->getPortraitIndex())
    {
        m_char->setPortraitIndex( m_mugIdx );
        emit changedPortrait();
    }
    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[VAL_FULL_NAME]))
    {
        m_char->setFullName( q->text() );
    }
    if (QLineEdit *q = qobject_cast<QLineEdit *>(m_widgets[VAL_NICKNAME]))
    {
        m_char->setName( q->text() );
        emit changedName( q->text() );
    }
    m_char->setPersonality( static_cast<character::personality>(m_personality_cb_group->checkedId() - PERS_START) );
    m_char->setVoice( m_voice_cb_group->checkedId() - VOICE_START );

    setVisible( false );
    this->setParent(NULL);
    this->deleteLater();

}
