/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2007, 2008, 2012 Alexander Lunev <al.lunev@yahoo.com>
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
 * @file PCBComponents.h
 */

#ifndef PCBCOMPONENTS_H_
#define PCBCOMPONENTS_H_

#include <wx/wx.h>

#include <XMLtoObjectCommonProceduresUnit.h>


class CNetNode : public wxObject
{
public:
    wxString m_compRef;
    wxString m_pinRef;

    CNetNode();
    ~CNetNode();
};

WX_DEFINE_ARRAY(CNetNode *, CNetNodesArray);

class CNet : public wxObject
{
public:
    wxString m_name;
    CNetNodesArray m_netNodes;

    CNet(wxString iName);
    ~CNet();
};

// basic parent class for PCB objects
class CPCBComponent : public wxObject
{
public:
    int m_tag;
    char m_objType;
    int m_PCadLayer;
    int m_KiCadLayer;
    int m_timestamp;
    int m_positionX;
    int m_positionY;
    int m_rotation;
    TTextValue m_name;   // name has also privete positions, rotations nand so on....
    wxString m_net;
    wxString m_compRef;  // internal ussage for XL parsing
    wxString m_patGraphRefName;  // internal ussage for XL parsing

    CPCBComponent();
    ~CPCBComponent();

    virtual void WriteToFile(wxFile *f, char ftype);
    virtual void SetPosOffset(int x_offs, int y_offs);
};

class CPCBPadViaShape : public CPCBComponent
{
public:
    wxString m_shape;
    int m_width;
    int m_height;

    CPCBPadViaShape();
    ~CPCBPadViaShape();
};

WX_DEFINE_ARRAY(CPCBPadViaShape *, CPCBPadViaShapesArray);

class CPCBPad : public CPCBComponent
{
public:
    int m_number;
    int m_hole;
    CPCBPadViaShapesArray m_shapes;

    CPCBPad(wxString iName);
    ~CPCBPad();

    virtual void WriteToFile(wxFile *f, char ftype, int r);
};

// will be replaced by pad in next version ????
class CPCBVia : public CPCBPad
{
public:

    CPCBVia();
    ~CPCBVia();
};

// Line , routes and drawings
class CPCBLine : public CPCBComponent
{
public:
    int m_width;
    int m_toX;
    int m_toY;

    CPCBLine();
    ~CPCBLine();

    virtual void WriteToFile(wxFile *f, char ftype);
    virtual void SetPosOffset(int x_offs, int y_offs);
};

WX_DEFINE_ARRAY(CPCBLine *, CPCBLinesArray);

class CPCBPolygon : public CPCBComponent
{
public:
    int m_width;
    CPCBLinesArray m_outline;  // collection of boundary/outline lines - objects
    CPCBLinesArray m_fill_lines;

    CPCBPolygon();
    ~CPCBPolygon();

    virtual void WriteToFile(wxFile *f, char ftype);
    virtual void WriteOutlineToFile(wxFile *f, char ftype);
    virtual void SetPosOffset(int x_offs, int y_offs);
};

class CVertex : public wxObject
{
public:
    double m_x;
    double m_y;

    CVertex();
    ~CVertex();
};

WX_DEFINE_ARRAY(CVertex *, CVerticesArray);

class CPCBCopperPour : public CPCBPolygon
{
public:
    CVerticesArray m_islands;

    CPCBCopperPour();
    ~CPCBCopperPour();
};

class CPCBCutout : public CPCBComponent
{
public:
    CPCBLinesArray m_outline; // collection of boundary/outline lines - objects

    CPCBCutout();
    ~CPCBCutout();

    virtual void WriteToFile(wxFile *f, char ftype);
};

class CPCBArc : public CPCBComponent
{
public:
    int m_startX;
    int m_startY;
    int m_angle;
    int m_width;

    CPCBArc();
    ~CPCBArc();

    virtual void WriteToFile(wxFile *f, char ftype);
    virtual void SetPosOffset(int x_offs, int y_offs);
};

// Name property of parent is used for text value
class CPCBText : public CPCBComponent
{
public:

    CPCBText();
    ~CPCBText();

    virtual void WriteToFile(wxFile *f, char ftype);
    virtual void SetPosOffset(int x_offs, int y_offs);
};

WX_DEFINE_ARRAY(CPCBComponent *, CPCBComponentsArray);

class CPCBModule : public CPCBComponent
{
public:
    TTextValue m_value; // has reference (Name from parent) and value
    CPCBComponentsArray m_moduleObjects;  // set of objects like CPCBLines,CPCBPads,CPCBVias,....
    int m_mirror;

    CPCBModule(wxString iName);
    ~CPCBModule();

    virtual void WriteToFile(wxFile *f, char ftype);
    virtual void Flip();
};

WX_DEFINE_ARRAY(CNet *, CNetsArray);

class CPCB : public wxObject
{
public:
    CPCBComponentsArray m_pcbComponents;  // CPCB Modules,Lines,Routes,Texts, .... and so on
    CNetsArray m_pcbNetlist;  // net objects collection
    wxString m_defaultMeasurementUnit;
    int m_layersMap[28]; // flexible layers mapping
    int m_sizeX;
    int m_sizeY;

    CPCB();
    ~CPCB();

    virtual void WriteToFile(wxString fileName, char ftype);
    virtual int GetNewTimestamp();

private:
    int m_timestamp_cnt;
};

#endif // PCBCOMPONENTS_H_
