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

/*
THSCHComponent = class (TObject)
     ObjType:char;
     PositionX,PositionY,Rotation,Mirror:integer;
     PartNum:Integer;
     Width:Integer;
     IsVisible:integer;
     procedure WriteToFile(var f:text;ftype:char); overload; virtual; abstract;
   end;
*/

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

protected:
    virtual void WriteToFile(char *f, char ftype);
};

/*
THSCHSymbol = class (THSCHComponent)
     Module,Reference,Typ:HTextValue;
     PatrtNum:integer;
     AttachedSymbol,AttachedPattern:string;
     procedure WriteToFile(var f:text;ftype:char); override;
   end;
*/

class CSchSymbol : public CSchComponent
{
public:
    TTextValue m_module, m_reference, m_typ;
    int m_patrtNum;
    wxString m_attachedSymbol, m_attachedPattern;

    CSchSymbol();
    ~CSchSymbol();

protected:
    virtual void WriteToFile(char *f, char ftype);
};

/*
THSCHPin = class (THSCHComponent)
      PinNum:HTextVAlue;
      PinName:HTextVAlue;
      Number:HTextVAlue;
      PinType,EdgeStyle:string;
      PinLength:integer;
      procedure WriteToFile(var f:text;ftype:char); override;
    end;
*/

class CSchPin : public CSchComponent
{
public:
    TTextValue m_pinNum, m_pinName, m_number;
    wxString m_pinType, m_edgeStyle;
    int m_pinLength;

    CSchPin();
    ~CSchPin();

protected:
    virtual void WriteToFile(char *f, char ftype);
};

/*
THSCHArc = class(THSCHComponent)
      StartX:integer;
      StartY:integer;
      ToX,ToY:Integer;
      StartAngle,SweepAngle:integer;
      Radius:integer;
      procedure WriteToFile(var f:text;ftype:char); override;
   end;
*/

class CSchArc : public CSchComponent
{
public:
    int m_startX, m_startY, m_toX, m_toY, m_startAngle, m_sweepAngle, m_radius;

    CSchArc();
    ~CSchArc();

protected:
    virtual void WriteToFile(char *f, char ftype);
};

/*
THSCHJunction = class (THSCHComponent)
     Net:string;
     procedure WriteToFile(var f:text;ftype:char); override;
   end;
*/

class CSchJunction : public CSchComponent
{
public:
    wxString m_net;

    CSchJunction();
    ~CSchJunction();

protected:
    virtual void WriteToFile(char *f, char ftype);
};


//THSCHNoConnect = class (THSCHComponent)
   //end;

/*
   THSCHText = class (THSCHComponent)
     Orientation:integer;
     Hight:integer;
   end;
*/

class CSchText : public CSchComponent
{
public:
    int m_orientation, m_hight;

    CSchText();
    ~CSchText();
};


//THSCHLabel = class (THSCHText)
   //end;

   //THSCHGLabel = class (THSCHText)
   //  shape:string;
   //end;

/*
   THSCHLine = class (THSCHComponent)
     ToX,ToY:integer;
     Net:string;
     LineType:Char; // Wire, Bus,....
     LabelText:HTextValue;
     procedure WriteToFile(var f:text;ftype:char); override;
     procedure WriteLabelToFile(var f:text;ftype:char);
   end;
*/

//   THSCHLine = class (THSCHWire);

class CSchLine : public CSchComponent
{
public:
    int m_toX, m_toY;
    wxString m_net;
    char m_lineType; // Wire, Bus, ...
    TTextValue m_labelText;

    CSchLine();
    ~CSchLine();

protected:
    virtual void WriteToFile(char *f, char ftype);
    virtual void WriteLabelToFile(char *f, char ftype);
};

//THSCHDottedWire = class (THSCHComponent)
   //  ToX,ToY:integer;
   //end;

   //THSCHBus = class (THSCHComponent)
   //  ToX,ToY:integer;
   //end;

   //THSCHEntryBus = class (THSCHComponent)
   //  ToX,ToY:integer;
   //end;

/*
   THSCHModule = class (THSCHComponent)
      ModuleObjects:TList;
      Name,Reference:HTextValue;
      NumParts:integer;
      AttachedPattern,ModuleDescription,Alias:string;
      AttachedSymbols:Array [1..10] of String;
      procedure WriteToFile(var f:text;ftype:char); override;
      constructor Create();
      destructor Free;
   end;
*/

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

protected:
    virtual void WriteToFile(char *f, char ftype);
};

/*
THSCH = class(TObject)
     SCHComponents:TList;
     DefaultMeasurementUnit:string;
     SizeX,SizeY:integer;
     constructor Create();
     procedure WriteToFile(FileName:string;ftype:char);
     destructor Free;
   end;
*/

class CSch : public wxObject
{
public:
    CSchComponentsArray m_schComponents;
    wxString m_defaultMeasurementUnit;
    int m_sizeX, m_sizeY;

    CSch();
    ~CSch();

protected:
    virtual void WriteToFile(wxString fileName, char ftype);
};

#endif // SCHCOMPONENTS_H_
