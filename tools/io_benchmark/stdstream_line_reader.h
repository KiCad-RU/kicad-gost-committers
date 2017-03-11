/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef FSTREAM_LINE_READER_H
#define FSTREAM_LINE_READER_H

#include <wx/filename.h>

#include <richio.h>

#include <istream>
#include <fstream>

/**
 * LINE_READER that wraps a given std::istream instance.
 */
class STDISTREAM_LINE_READER : public LINE_READER
{
public:

    STDISTREAM_LINE_READER();

    ~STDISTREAM_LINE_READER();

    char* ReadLine() throw( IO_ERROR ) override;

protected:

    void setStream( std::istream&  aStream );

private:
    std::string m_buffer;
    std::istream* m_stream;
};


/**
 * LINE_READER interface backed by std::ifstream
 */
class IFSTREAM_LINE_READER : public STDISTREAM_LINE_READER
{
public:

    IFSTREAM_LINE_READER( const wxFileName& aFileName ) throw( IO_ERROR );

    void Rewind();

private:
    std::ifstream m_fStream;
};

#endif // FSTREAM_LINE_READER_H
