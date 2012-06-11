/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2007, 2008 Lubo Racko <developer@lura.sk>
 * Copyright (C) 2008, 2012 Alexander Lunev <al.lunev@yahoo.com>
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
 * @file XMLtoObjectCommonProceduresUnit.h
 */

#ifndef XMLTOOBJECTCOMMONPROCEDURESUNIT_H_
#define XMLTOOBJECTCOMMONPROCEDURESUNIT_H_

#include <wx/wx.h>
#include <wx/xml/xml.h>

/*
HTextValue = record
       Text:string;
       TextPositionX,TextPositionY,
       TextRotation,TextHeight,TextstrokeWidth:integer;
       TextIsVisible,Mirror,TextUnit:integer;
       CorrectedPositionX,CorrectedPositionY:Integer;
     end;
*/

typedef struct _TTextValue {
    wxString text;
    int textPositionX, textPositionY,
        textRotation, textHeight, textstrokeWidth;
    int textIsVisible, mirror, textUnit;
    int correctedPositionX, correctedPositionY;
} TTextValue, *pTTextValue;

extern wxString GetWord(wxString *s);

extern int StrToIntUnits(wxString s, char axe, wxString actualConversion);
extern wxString GetAndCutWordWithMeasureUnits(wxString *i, wxString defaultMeasurementUnit);
extern int StrToInt1Units(wxString s);
extern wxString ValidateName(wxString n);
extern void SetPosition(wxString t, wxString defaultMeasurementUnit, int *x, int *y, wxString actualConversion);
extern void SetTextParameters(wxXmlNode *iNode, TTextValue *tv, wxString defaultMeasurementUnit, wxString actualConversion);
extern void SetFontProperty(wxXmlNode *iNode, TTextValue *tv, wxString defaultMeasurementUnit, wxString actualConversion);
extern void CorrectTextPosition(TTextValue *value, int rotation);

extern wxXmlNode *FindNode(wxXmlNode *child, wxString tag);

#endif // XMLTOOBJECTCOMMONPROCEDURESUNIT_H_
