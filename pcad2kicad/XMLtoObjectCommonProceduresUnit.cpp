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
 * @file XMLtoObjectCommonProceduresUnit.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <XMLtoObjectCommonProceduresUnit.h>


wxString GetWord(wxString *s) {
    wxString result = wxEmptyString;
    (*s) = s->Trim(false);
    if (s->Len() == 0) return result;
    if ((*s)[0] == '"') {
        result += (*s)[0];
        *s = s->Mid(1); // remove Frot apostrofe
        while (s->Len() > 0 && (*s)[0] != '"') {
            result += (*s)[0];
            *s = s->Mid(1);
        }

        if (s->Len() > 0 && (*s)[0] == '"') {
            result += (*s)[0];
            *s = s->Mid(1); // remove ending apostrophe
        }
    }
    else {
        while (s->Len() > 0 && !( (*s)[0] == ' ' || (*s)[0] == '(' || (*s)[0] == ')' )) {
            result += (*s)[0];
            *s = s->Mid(1);
        }
    }

    result.Trim(true);
    result.Trim(false);

    return result;
}

wxXmlNode *FindPinMap(wxXmlNode *iNode) {
    wxXmlNode *result, *lNode;

    result = NULL;
    lNode = FindNode(iNode->GetChildren(), wxT("attachedPattern"));
    if (lNode) result = FindNode(lNode->GetChildren(), wxT("padPinMap"));

    return result;
}

double StrToDoublePrecisionUnits(wxString s, char axe, wxString actualConversion) {
    wxString ls;
    double i, precision;
    char u;

    ls = s;
    ls.Trim(true);
    ls.Trim(false);
    precision = 1.0;
    if (actualConversion == wxT("PCB")) precision = 10.0;
    if (actualConversion == wxT("SCH")) precision = 1.0;

    if (ls.Len() > 0) {
        u = ls[ls.Len() - 1];
        while (ls.Len() > 0 &&
             !(ls[ls.Len() - 1] == '.' ||
               ls[ls.Len() - 1] == ',' ||
               (ls[ls.Len() - 1] >= '0' && ls[ls.Len() - 1] <= '9')))
        {
            ls = ls.Left(ls.Len() - 1);
        }

        while (ls.Len() > 0 &&
             !(ls[0] == '-' ||
               ls[0] == '+' ||
               ls[0] == '.' ||
               ls[0] == ',' ||
               (ls[0] >= '0' && ls[0] <= '9')))
        {
            ls = ls.Mid(1);
        }

        //TODO: Is the following commented string necessary?
        //if (pos(',',ls) > 0) DecimalSeparator:=',' else DecimalSeparator:='.';
        if (u == 'm') {
            ls.ToDouble(&i);
            i = i * precision / 0.0254;
        }
        else {
            ls.ToDouble(&i);
            i = i * precision;
        }
    }
    else i = 0.0;

    if ((actualConversion == wxT("PCB") || actualConversion == wxT("SCH")) && axe == 'Y')
        return -i;
    else return i; // Y axe is mirrored in compare with PCAD
}

int StrToIntUnits(wxString s, char axe, wxString actualConversion) {
    return KiROUND(StrToDoublePrecisionUnits(s, axe, actualConversion));
}

wxString GetAndCutWordWithMeasureUnits(wxString *i, wxString defaultMeasurementUnit) {
    wxString s1, s2, result;

    i->Trim(false);
    result = wxEmptyString;
    // value
    while (i->Len() > 0 && (*i)[0] != ' ') {
        result += (*i)[0];
        *i = i->Mid(1);
    }
    i->Trim(false);
    // if there is also measurement unit
    while (i->Len() > 0 &&
           (((*i)[0] >= 'a' && (*i)[0] <= 'z') ||
            ((*i)[0] >= 'A' && (*i)[0] <= 'Z')))
    {
        result += (*i)[0];
        *i = i->Mid(1);
    }
    // and if not, add default....
    if (result.Len() > 0 &&
        (result[result.Len() - 1] == '.' ||
         result[result.Len() - 1] == ',' ||
         (result[result.Len() - 1] >= '0' && result[result.Len() - 1] <= '9')))
    {
        result += defaultMeasurementUnit;
    }

    return result;
}

int StrToInt1Units(wxString s) {
    double num, precision = 10;
    //TODO: Is the following commented string necessary?
    //if (pos(',',s)>0) then DecimalSeparator:=',' else DecimalSeparator:='.';
    s.ToDouble(&num);
    return KiROUND(num * precision);
}

wxString ValidateName(wxString n) {
    wxString o;
    int i;

    o = wxEmptyString;
    for (i = 0; i < (int)n.Len(); i++) {
        if (n[i] == ' ') o += '_';
        else o += n[i];
    }

    return o;
}

void SetWidth(wxString t, wxString defaultMeasurementUnit, int *width, wxString actualConversion) {
    *width = StrToIntUnits(GetAndCutWordWithMeasureUnits(&t, defaultMeasurementUnit), ' ', actualConversion);
}

void SetHeight(wxString t, wxString defaultMeasurementUnit, int *height, wxString actualConversion) {
    *height = StrToIntUnits(GetAndCutWordWithMeasureUnits(&t, defaultMeasurementUnit), ' ', actualConversion);
}

void SetPosition(wxString t, wxString defaultMeasurementUnit, int *x, int *y, wxString actualConversion) {
    *x = StrToIntUnits(GetAndCutWordWithMeasureUnits(&t, defaultMeasurementUnit), 'X', actualConversion);
    *y = StrToIntUnits(GetAndCutWordWithMeasureUnits(&t, defaultMeasurementUnit), 'Y', actualConversion);
}

/*
procedure SetDoublePrecisionPosition(t,DefaultMeasurementUnit:string;var x,y:double);
begin
    X:=StrToDoublePrecisionUnits(GetAndCutWordWithMeasureUnits(t,DefaultMeasurementUnit),'X');
    Y:=StrToDoublePrecisionUnits(GetAndCutWordWithMeasureUnits(t,DefaultMeasurementUnit),'Y');
end;
*/

void SetDoublePrecisionPosition(wxString t, wxString defaultMeasurementUnit, double *x, double *y, wxString actualConversion) {
    *x = StrToDoublePrecisionUnits(GetAndCutWordWithMeasureUnits(&t, defaultMeasurementUnit), 'X', actualConversion);
    *y = StrToDoublePrecisionUnits(GetAndCutWordWithMeasureUnits(&t, defaultMeasurementUnit), 'Y', actualConversion);
}

void SetTextParameters(wxXmlNode *iNode, TTEXTVALUE *tv, wxString defaultMeasurementUnit, wxString actualConversion) {
    wxXmlNode *tNode;
    wxString str;

    tNode = FindNode(iNode->GetChildren(), wxT("pt"));
    if (tNode) SetPosition(tNode->GetNodeContent(), defaultMeasurementUnit, &tv->textPositionX, &tv->textPositionY, actualConversion);

    tNode = FindNode(iNode->GetChildren(), wxT("rotation"));
    if (tNode) {
        str = tNode->GetNodeContent();
        str.Trim(false);
        tv->textRotation = StrToInt1Units(str);
    }

    tv->textIsVisible = 1;
    tNode = FindNode(iNode->GetChildren(), wxT("isVisible"));
    if (tNode) {
        str = tNode->GetNodeContent();
        str.Trim(false);
        str.Trim(true);
        if (str == wxT("True")) tv->textIsVisible = 1;
        else tv->textIsVisible = 0;
    }

    tNode = FindNode(iNode->GetChildren(), wxT("textStyleRef"));
    if (tNode) SetFontProperty(tNode, tv, defaultMeasurementUnit, actualConversion);
}

void SetFontProperty(wxXmlNode *iNode, TTEXTVALUE *tv, wxString defaultMeasurementUnit, wxString actualConversion) {
    wxString n, propValue;

    iNode->GetPropVal(wxT("Name"), &n);

    while (iNode->GetName() != wxT("www.lura.sk"))
        iNode = iNode->GetParent();

    iNode = FindNode(iNode->GetChildren(), wxT("library"));
    if (iNode) iNode = FindNode(iNode->GetChildren(), wxT("textStyleDef"));
    if (iNode) {
        while (true) {
            iNode->GetPropVal(wxT("Name"), &propValue);
            propValue.Trim(false);
            propValue.Trim(true);
            if (propValue == n) break;
            iNode = iNode->GetNext();
        }
        if (iNode) {
            iNode = FindNode(iNode->GetChildren(), wxT("font"));
            if (iNode) {
                if (FindNode(iNode->GetChildren(), wxT("fontHeight")))
                    ////SetWidth(iNode.ChildNodes.FindNode('fontHeight').Text,DefaultMeasurementUnit,tv.TextHeight);
                    // Fixed By Lubo, 02/2008
                    SetHeight(FindNode(iNode->GetChildren(), wxT("fontHeight"))->GetNodeContent(),
                              defaultMeasurementUnit, &tv->textHeight, actualConversion);
                if (FindNode(iNode->GetChildren(), wxT("strokeWidth")))
                    SetWidth(FindNode(iNode->GetChildren(), wxT("strokeWidth"))->GetNodeContent(),
                             defaultMeasurementUnit, &tv->textstrokeWidth, actualConversion);
            }
        }
    }
}

void CorrectTextPosition(TTEXTVALUE *value, int rotation) {
    value->correctedPositionX = value->textPositionX;
    value->correctedPositionY = value->textPositionY;
    value->correctedPositionY = value->correctedPositionY - KiROUND((double)value->textHeight / 3.0);
    value->correctedPositionX = value->correctedPositionX + KiROUND(((double)value->text.Len() / 1.4) * ((double)value->textHeight / 1.8));
    if (rotation == 900) {
        value->correctedPositionX = -value->textPositionY;
        value->correctedPositionY = value->textPositionX;
        value->correctedPositionX = value->correctedPositionX + KiROUND((double)value->textHeight / 3.0);
        value->correctedPositionY = value->correctedPositionY + KiROUND(((double)value->text.Len() / 1.4) * ((double)value->textHeight / 1.8));
    }
    if (rotation == 1800) {
        value->correctedPositionX = -value->textPositionX;
        value->correctedPositionY = -value->textPositionY;
        value->correctedPositionY = value->correctedPositionY + KiROUND((double)value->textHeight / 3.0);
        value->correctedPositionX = value->correctedPositionX - KiROUND(((double)value->text.Len() / 1.4) * ((double)value->textHeight / 1.8));
    }
    if (rotation == 2700) {
        value->correctedPositionX = value->textPositionY;
        value->correctedPositionY = -value->textPositionX;
        value->correctedPositionX = value->correctedPositionX + KiROUND((double)value->textHeight / 1.0);
        value->correctedPositionY = value->correctedPositionY - KiROUND(((double)value->text.Len() / 3.4) * ((double)value->textHeight / 1.8));
    }
}

wxXmlNode *FindNode(wxXmlNode *child, wxString tag) {
    while (child) {
        if (child->GetName() == tag) return child;

        child = child->GetNext();
    }

    return NULL;
}

void InitTTextValue(TTEXTVALUE *textValue) {
    textValue->text = wxEmptyString;
    textValue->textPositionX = 0;
    textValue->textPositionY = 0;
    textValue->textRotation = 0;
    textValue->textHeight = 0;
    textValue->textstrokeWidth = 0;
    textValue->textIsVisible = 0;
    textValue->mirror = 0;
    textValue->textUnit = 0;
    textValue->correctedPositionX = 0;
    textValue->correctedPositionY = 0;
}
