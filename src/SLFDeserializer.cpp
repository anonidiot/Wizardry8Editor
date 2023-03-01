/*
 * Copyright (C) 2023 Anonymous Idiot
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

#include "Urho3D/Math/MathDefs.h"
#include "SLFDeserializer.h"

SLFDeserializer::SLFDeserializer(SLFFile &slf) : Deserializer(),
    slf_(slf),
    checksumDone_(false)
{
    if (slf_.isGood())
    {
        slf_.open(QFile::ReadOnly);
        position_ = 0;
        size_     = slf.size();
    }
}

SLFDeserializer::~SLFDeserializer()
{
    // leave the file open in case caller still using it
}

unsigned SLFDeserializer::Read(void *dest, unsigned size)
{
    qint64 r=0;

    if (slf_.isGood())
    {
        try
        {
            r = slf_.read((char *)dest, (qint64) size);
            position_ += r;
        }
        catch (SLFFileException &e)
        {
        }
    }
    return (unsigned)r;
}

unsigned SLFDeserializer::Seek(unsigned position)
{
    qint64 p = (qint64)position;

    if (slf_.isGood())
    {
        if (p > slf_.size())
            p = slf_.size();

        slf_.seek(p);
        position_ = (unsigned)slf_.pos(); // which should be p

        return position_;
    }
    return 0;
}

// copied from Urho3D/IO/File.cpp
unsigned SLFDeserializer::GetChecksum()
{
    if (checksumDone_)
        return checksum_;

    unsigned oldPos = position_;
    checksum_ = 0;

    Seek(0);
    while (!IsEof())
    {
        unsigned char block[1024];
        unsigned readBytes = Read(block, 1024);
        for (unsigned i = 0; i < readBytes; ++i)
            checksum_ = Urho3D::SDBMHash(checksum_, block[i]);
    }

    Seek(oldPos);
    checksumDone_ = true;
    return checksum_;
}
