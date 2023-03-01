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

#ifndef SLFDESERIALIZER_H__
#define SLFDESERIALIZER_H__

#include "Urho3D/IO/Deserializer.h"
#include "SLFFile.h"

class SLFDeserializer : public Urho3D::Deserializer
{
public:
    SLFDeserializer(SLFFile &slf);
    ~SLFDeserializer();

    /// Read bytes from the stream. Return number of bytes actually read.
    unsigned Read(void* dest, unsigned size) override;
    /// Set position from the beginning of the stream. Return actual new position.
    unsigned Seek(unsigned position) override;
    /// Change the file name. Used by the resource system.
    /// @property
    virtual void SetName(const Urho3D::String& name) { name_ = name; }
    /// Return the file name.
    const Urho3D::String& GetName() const override { return name_; }
    /// Return a checksum of the file contents using the SDBM hash algorithm.
    unsigned GetChecksum() override;

protected:
    /// File name.
   Urho3D::String    name_;

private:
    SLFFile         &slf_;
    unsigned         checksum_;
    bool             checksumDone_;
};
#endif /* SLFDESERIALIZER_H__ */
