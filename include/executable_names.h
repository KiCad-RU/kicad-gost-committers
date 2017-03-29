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

/**
 * KiCad executable names
 * @file executable_names.h
 */

#ifndef INCLUDE__EXECUTABLE_NAMES_H_
#define INCLUDE__EXECUTABLE_NAMES_H_

#include <wx/string.h>

extern const wxString CVPCB_EXE;
extern const wxString PCBNEW_EXE;
extern const wxString EESCHEMA_EXE;
extern const wxString GERBVIEW_EXE;
extern const wxString BITMAPCONVERTER_EXE;
extern const wxString PCB_CALCULATOR_EXE;
extern const wxString PL_EDITOR_EXE;

#if defined( KICAD_GOST )
extern const wxString PCAD2KICADSCH_EXE;
#endif

#endif  // INCLUDE__EXECUTABLE_NAMES_H_
