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

#include <QPixmap>
#include <QWidget>

#include "StringList.h"

class facts;

#define ORIGINAL_DIM_X   640
#define ORIGINAL_DIM_Y   480

const StringList     *getStringTable();
const StringList     *getBaseStringTable();

bool                  getIgnoreModStrings();
void                  setIgnoreModStrings(bool value);

void                  setFacts(facts f);
bool                  testFact(int fact_id);
bool                  testFact(QString fact_name);

void                  setAppScale(double scale);
double                getAppScale();

QPixmap               getCursor(int id);

bool                  isWizardry128();
bool                  isParallelWorlds();

QString getOpenFileName(QWidget *parent, const QString &caption, const QString &directory, const QString &filter);
QString getSaveFileName(QWidget *parent, const QString &caption, const QString &directory, const QString &filter);
