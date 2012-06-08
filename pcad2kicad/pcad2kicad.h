/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2012 Alexander Lunev <al.lunev@yahoo.com>
 * Copyright (C) 2012 KiCad Developers, see CHANGELOG.TXT for contributors.
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

/**
 * @file pcad2kicad.h
 */

#ifndef  PCAD2KICAD_H
#define  PCAD2KICAD_H

#include <pcad2kicad_frame_base.h>

/* Class PCAD2KICAD_FRAME_BASE
This is the main frame for this application
*/
class PCAD2KICAD_FRAME : public PCAD2KICAD_FRAME_BASE
{
private:
    wxString m_actualConversion;

    wxSize m_FrameSize;
    wxPoint m_FramePos;

public:
    PCAD2KICAD_FRAME( wxWindow * parent = NULL );
    ~PCAD2KICAD_FRAME();

private:

    // Event handlers
    void OnSch( wxCommandEvent& event );
};

#endif  // PCAD2KICAD_H
