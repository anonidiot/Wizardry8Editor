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

#include <QMapIterator>

#include "facts.h"
#include "SLFFile.h"

#include "common.h"

#define FACTDB_RECORD_SIZE    0x1d8

facts::facts() : QObject(),
    m_data(QByteArray())
{
}

// Character specific information is split into 2 separate areas.
// A block of 0x1866 bytes and a block of 0x106 bytes
// We keep an exact copy of the data passed into us because there
// are a lot of fields we _don't_ parse and we'd like to preserve
// those as far as possible on save.
facts::facts(QByteArray f) : QObject(),
    m_data(f)
{
    SLFFile fact_db("DATABASES/FACT.DBS");

    if (fact_db.isGood())
    {
        fact_db.open( QIODevice::ReadOnly );

        int named_facts_cnt = fact_db.readLEULong();

        fact_db.skip( 4 );

        m_namedFacts.reserve( named_facts_cnt );
        for (int k=0; k < named_facts_cnt; k++)
        {
            QByteArray fact = fact_db.read( FACTDB_RECORD_SIZE );

            // Fact is in ASCII, not unicode

            m_namedFacts << QString::fromLatin1( fact );
        }
    }
}

facts::~facts()
{
    m_namedFacts.clear();
}

QByteArray facts::serialize() const
{
    return m_data;
}

void facts::reset()
{
    m_data.fill( '\x00' );
}

bool facts::isNull() const
{
    if (m_data.isNull() || (m_data.isEmpty()))
    {
        return true;
    }
    return false;
}

int facts::size() const
{
    if (m_data.isNull())
    {
        return 0;
    }

    return m_data.size();
}

QString facts::getKey( int idx ) const
{
    Q_ASSERT( idx < m_data.size() );

    if (idx < m_namedFacts.size())
    {
        return m_namedFacts[idx];
    }
    if (idx < m_data.size())
    {
        return QString("UNNAMED_FACT_%1").arg(idx);
    }
    return QString("ERROR");
}

bool facts::getValue( int idx ) const
{
    Q_ASSERT( idx < m_data.size() );

    if (idx < m_data.size())
    {
        quint8 *fdata = (quint8 *) m_data.data();

        if (fdata[idx] == 0x00)
        {
            return false;
        }
        return true;
    }
    // can't return an error without changing prototype, which I don't
    // want to do, so assert above if the index is out of range. We
    // shouldn't ever get here if the caller checks the size() first.

    return false;
}

void facts::setValue( int idx, bool value )
{
    Q_ASSERT( idx < m_data.size() );

    if (idx < m_data.size())
    {
        quint8 *fdata = (quint8 *) m_data.data();

        if (value == true)
            fdata[idx] = 0x01;
        else
            fdata[idx] = 0x00;
    }
}

bool facts::testFact( QString fact_name )
{
    for (int k=0; k < m_namedFacts.size(); k++)
    {
        if (m_namedFacts[k].compare( fact_name ) == 0)
            return getValue(k);
    }
    return false;
}
