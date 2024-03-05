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

#ifndef SCREENPERSONALITY_H__
#define SCREENPERSONALITY_H__

#include <QWidget>
#include <QAudioProbe>
#include <QMediaPlayer>
#include <QPixmap>
#include <QPushButton>
#include <QStack>
#include <QVector>

#include "Wizardry8Scalable.h"

#include "character.h"
#include "Screen.h"
#include "PortraitsDb.h"

class QHelpEvent;
class QButtonGroup;
class QFile;

class ScreenPersonality : public Screen
{
    Q_OBJECT

public:
    ScreenPersonality(character *c, QWidget *parent = nullptr);
    ~ScreenPersonality();

    void        setVisible(bool visible) override;

signals:
    void        changedName(QString);
    void        changedPortrait();

public slots:
    void        nextRace(bool down);
    void        prevRace(bool down);
    void        nextFace(bool down);
    void        prevFace(bool down);
    void        quotePlay(bool down);
    void        audioPlayerProbe(const QAudioBuffer &buffer);
    void        audioPlayerStatusChanged(QMediaPlayer::MediaStatus status);
    void        undoPersonalityScreen(bool checked);
    void        abortPersonalityScreen(bool checked);
    void        exitPersonalityScreen(bool checked);
    void        changePersonalityVoice(int state);
    void        nameChanged(const QString &name);
    void        portraitPopup(QPoint point);
    void        cmPortraitModify();
    void        cmPortraitReset();
    void        resetLanguage();

protected:
    void        resizeEvent(QResizeEvent *event) override;

private:
    QString     generatePersonalityFilename();

    void        resetScreen(void *char_tag, void *party_tag) override;

    int         nextImageIdx(bool race, bool up);

    void        assemblePortraitIndices();


    character        *m_char;
    int               m_mugIdx;

    QPixmap           m_audioLevels[4];

    QButtonGroup     *m_personality_cb_group;
    QButtonGroup     *m_voice_cb_group;
    QMediaPlayer     *m_audio_player;
    QAudioProbe      *m_visualiser;
    QFile            *m_snd_file;
 
    QAction          *m_cmPortraitModify;
    QAction          *m_cmPortraitReset;

    QVector<int>      m_portraits[ PORTRAIT_GRP_SIZE];
};
#endif
