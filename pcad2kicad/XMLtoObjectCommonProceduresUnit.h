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

typedef struct _TTEXTVALUE
{
    wxString    text;
    int         textPositionX, textPositionY,
                textRotation, textHeight, textstrokeWidth;
    int textIsVisible, mirror, textUnit;
    int correctedPositionX, correctedPositionY;
} TTEXTVALUE;

extern wxString     GetWord( wxString* aStr );
extern wxXmlNode*   FindPinMap( wxXmlNode* aNode );
extern int          StrToIntUnits( wxString aStr, char aAxe, wxString aActualConversion );
extern wxString     GetAndCutWordWithMeasureUnits( wxString*    aStr,
                                                   wxString     aDefaultMeasurementUnit );
extern int          StrToInt1Units( wxString aStr );
extern wxString     ValidateName( wxString aName );
extern void         SetWidth( wxString  aStr,
                              wxString  aDefaultMeasurementUnit,
                              int*      aWidth,
                              wxString  aActualConversion );
extern void         SetPosition( wxString   aStr,
                                 wxString   aDefaultMeasurementUnit,
                                 int*       aX,
                                 int*       aY,
                                 wxString   aActualConversion );
extern void         SetDoublePrecisionPosition( wxString    aStr,
                                                wxString    aDefaultMeasurementUnit,
                                                double*     aX,
                                                double*     aY,
                                                wxString    aActualConversion );
extern void         SetTextParameters( wxXmlNode*   aNode,
                                       TTEXTVALUE*  aTextValue,
                                       wxString     aDefaultMeasurementUnit,
                                       wxString     aActualConversion );
extern void         SetFontProperty( wxXmlNode*     aNode,
                                     TTEXTVALUE*    aTextValue,
                                     wxString       aDefaultMeasurementUnit,
                                     wxString       aActualConversion );
extern void         CorrectTextPosition( TTEXTVALUE* aValue, int aRotation );

extern wxXmlNode*   FindNode( wxXmlNode* aChild, wxString aTag );
extern void         InitTTextValue( TTEXTVALUE* aTextValue );

#endif    // XMLTOOBJECTCOMMONPROCEDURESUNIT_H_
