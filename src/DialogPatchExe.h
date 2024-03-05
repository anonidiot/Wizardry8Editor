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

#ifndef DLGPATCHEXE_H
#define DLGPATCHEXE_H

#include "Dialog.h"
#include "Wizardry8Scalable.h"

class QListWidgetItem;

class DialogPatchExe : public Dialog
{
    Q_OBJECT

public:
    DialogPatchExe(QWidget *parent = nullptr);
    ~DialogPatchExe();

    enum wizardry_ver
    {
        WIZ_VER_UNKNOWN,
        WIZ_VER_1_0,
        WIZ_VER_1_2_4,
        WIZ_VER_1_2_4_PATCHED,
        WIZ_VER_1_2_8_UNKNOWN,
        WIZ_VER_1_2_8_BUILD_6200,
        WIZ_VER_1_2_8_BUILD_6200_PATCHED
    };
    Q_ENUM(wizardry_ver);

    static QString identifyWizardryExeVersion( QString exeFile, wizardry_ver *ver, char *md5Hash, char **exePath );
    static quint64 getExpectedExeSize(wizardry_ver ver);
    static QString getVersionStr(wizardry_ver ver);

public slots:
    void patchHighlighted(int);

    void patchAway(bool checked);

protected:
    void updateList();

private:
    QPixmap    makeDialogForm();

    wizardry_ver    m_myWizVer;
    QString         m_exePath;
};

#endif

