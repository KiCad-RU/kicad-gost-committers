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
 * @file SchComponents.h
 */

#ifndef SCHCOMPONENTS_H_
#define SCHCOMPONENTS_H_

#include <wx/wx.h>
#include <wx/dynarray.h>

#include <XMLtoObjectCommonProceduresUnit.h>

class CSchComponent : public wxObject
{
public:
    char m_objType;
    int m_positionX, m_positionY, m_rotation, m_mirror;
    int m_partNum;
    int m_width;
    int m_isVisible;

    CSchComponent();
    ~CSchComponent();

    virtual void WriteToFile(wxFile *f, char ftype);
};

class CSchSymbol : public CSchComponent
{
public:
    TTextValue m_module, m_reference, m_typ;
    wxString m_attachedSymbol, m_attachedPattern;

    CSchSymbol();
    ~CSchSymbol();

    virtual void WriteToFile(wxFile *f, char ftype);
};

// This class is not yet used
class CSchText : public CSchComponent
{
public:
    int m_orientation, m_hight;

    CSchText();
    ~CSchText();
};

WX_DEFINE_ARRAY(CSchComponent *, CSchComponentsArray);

class CSchModule : public CSchComponent
{
public:
    CSchComponentsArray m_moduleObjects;
    TTextValue m_name, m_reference;
    int m_numParts;
    wxString m_attachedPattern, m_moduleDescription, m_alias;
    wxString m_attachedSymbols[10];

    CSchModule();
    ~CSchModule();

    virtual void WriteToFile(wxFile *f, char ftype);
};

class CSch : public wxObject
{
public:
    CSchComponentsArray m_schComponents;
    wxString m_defaultMeasurementUnit;
    int m_sizeX, m_sizeY;

    CSch();
    ~CSch();

    virtual void WriteToFile(wxString fileName, char ftype);
};

#endif // SCHCOMPONENTS_H_
