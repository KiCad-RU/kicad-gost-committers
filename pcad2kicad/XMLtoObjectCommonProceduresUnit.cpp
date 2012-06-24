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


wxString GetWord(wxString *aStr) {
    wxString result = wxEmptyString;
    *aStr = aStr->Trim(false);
    if (aStr->Len() == 0) return result;
    if ((*aStr)[0] == '"') {
        result += (*aStr)[0];
        *aStr = aStr->Mid(1); // remove Frot apostrofe
        while (aStr->Len() > 0 && (*aStr)[0] != '"') {
            result += (*aStr)[0];
            *aStr = aStr->Mid(1);
        }

        if (aStr->Len() > 0 && (*aStr)[0] == '"') {
            result += (*aStr)[0];
            *aStr = aStr->Mid(1); // remove ending apostrophe
        }
    }
    else {
        while (aStr->Len() > 0 && !( (*aStr)[0] == ' ' || (*aStr)[0] == '(' || (*aStr)[0] == ')' )) {
            result += (*aStr)[0];
            *aStr = aStr->Mid(1);
        }
    }

    result.Trim(true);
    result.Trim(false);

    return result;
}

wxXmlNode *FindPinMap(wxXmlNode *aNode) {
    wxXmlNode *result, *lNode;

    result = NULL;
    lNode = FindNode(aNode->GetChildren(), wxT("attachedPattern"));
    if (lNode) result = FindNode(lNode->GetChildren(), wxT("padPinMap"));

    return result;
}

double StrToDoublePrecisionUnits(wxString aStr, char aAxe, wxString aActualConversion) {
    wxString ls;
    double i, precision;
    char u;

    ls = aStr;
    ls.Trim(true);
    ls.Trim(false);
    precision = 1.0;
    if (aActualConversion == wxT("PCB")) precision = 10.0;
    if (aActualConversion == wxT("SCH")) precision = 1.0;

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

    if ((aActualConversion == wxT("PCB") || aActualConversion == wxT("SCH")) && aAxe == 'Y')
        return -i;
    else return i; // Y axe is mirrored in compare with PCAD
}

int StrToIntUnits(wxString aStr, char aAxe, wxString aActualConversion) {
    return KiROUND(StrToDoublePrecisionUnits(aStr, aAxe, aActualConversion));
}

wxString GetAndCutWordWithMeasureUnits(wxString *aStr, wxString aDefaultMeasurementUnit) {
    wxString s1, s2, result;

    aStr->Trim(false);
    result = wxEmptyString;
    // value
    while (aStr->Len() > 0 && (*aStr)[0] != ' ') {
        result += (*aStr)[0];
        *aStr = aStr->Mid(1);
    }
    aStr->Trim(false);
    // if there is also measurement unit
    while (aStr->Len() > 0 &&
           (((*aStr)[0] >= 'a' && (*aStr)[0] <= 'z') ||
            ((*aStr)[0] >= 'A' && (*aStr)[0] <= 'Z')))
    {
        result += (*aStr)[0];
        *aStr = aStr->Mid(1);
    }
    // and if not, add default....
    if (result.Len() > 0 &&
        (result[result.Len() - 1] == '.' ||
         result[result.Len() - 1] == ',' ||
         (result[result.Len() - 1] >= '0' && result[result.Len() - 1] <= '9')))
    {
        result += aDefaultMeasurementUnit;
    }

    return result;
}

int StrToInt1Units(wxString aStr) {
    double num, precision = 10;
    //TODO: Is the following commented string necessary?
    //if (pos(',',s)>0) then DecimalSeparator:=',' else DecimalSeparator:='.';
    aStr.ToDouble(&num);
    return KiROUND(num * precision);
}

wxString ValidateName(wxString aName) {
    wxString o;
    int i;

    o = wxEmptyString;
    for (i = 0; i < (int)aName.Len(); i++) {
        if (aName[i] == ' ') o += '_';
        else o += aName[i];
    }

    return o;
}

void SetWidth(wxString aStr, wxString aDefaultMeasurementUnit, int *aWidth, wxString aActualConversion) {
    *aWidth = StrToIntUnits(GetAndCutWordWithMeasureUnits(&aStr, aDefaultMeasurementUnit), ' ', aActualConversion);
}

void SetHeight(wxString aStr, wxString aDefaultMeasurementUnit, int *aHeight, wxString aActualConversion) {
    *aHeight = StrToIntUnits(GetAndCutWordWithMeasureUnits(&aStr, aDefaultMeasurementUnit), ' ', aActualConversion);
}

void SetPosition(wxString aStr, wxString aDefaultMeasurementUnit, int *aX, int *aY, wxString aActualConversion) {
    *aX = StrToIntUnits(GetAndCutWordWithMeasureUnits(&aStr, aDefaultMeasurementUnit), 'X', aActualConversion);
    *aY = StrToIntUnits(GetAndCutWordWithMeasureUnits(&aStr, aDefaultMeasurementUnit), 'Y', aActualConversion);
}

void SetDoublePrecisionPosition(wxString aStr, wxString aDefaultMeasurementUnit, double *aX, double *aY, wxString aActualConversion) {
    *aX = StrToDoublePrecisionUnits(GetAndCutWordWithMeasureUnits(&aStr, aDefaultMeasurementUnit), 'X', aActualConversion);
    *aY = StrToDoublePrecisionUnits(GetAndCutWordWithMeasureUnits(&aStr, aDefaultMeasurementUnit), 'Y', aActualConversion);
}

void SetTextParameters(wxXmlNode *aNode, TTEXTVALUE *aTextValue, wxString aDefaultMeasurementUnit, wxString aActualConversion) {
    wxXmlNode *tNode;
    wxString str;

    tNode = FindNode(aNode->GetChildren(), wxT("pt"));
    if (tNode) SetPosition(tNode->GetNodeContent(), aDefaultMeasurementUnit, &aTextValue->textPositionX, &aTextValue->textPositionY, aActualConversion);

    tNode = FindNode(aNode->GetChildren(), wxT("rotation"));
    if (tNode) {
        str = tNode->GetNodeContent();
        str.Trim(false);
        aTextValue->textRotation = StrToInt1Units(str);
    }

    aTextValue->textIsVisible = 1;
    tNode = FindNode(aNode->GetChildren(), wxT("isVisible"));
    if (tNode) {
        str = tNode->GetNodeContent();
        str.Trim(false);
        str.Trim(true);
        if (str == wxT("True")) aTextValue->textIsVisible = 1;
        else aTextValue->textIsVisible = 0;
    }

    tNode = FindNode(aNode->GetChildren(), wxT("textStyleRef"));
    if (tNode) SetFontProperty(tNode, aTextValue, aDefaultMeasurementUnit, aActualConversion);
}

void SetFontProperty(wxXmlNode *aNode, TTEXTVALUE *aTextValue, wxString aDefaultMeasurementUnit, wxString aActualConversion) {
    wxString n, propValue;

    aNode->GetPropVal(wxT("Name"), &n);

    while (aNode->GetName() != wxT("www.lura.sk"))
        aNode = aNode->GetParent();

    aNode = FindNode(aNode->GetChildren(), wxT("library"));
    if (aNode) aNode = FindNode(aNode->GetChildren(), wxT("textStyleDef"));
    if (aNode) {
        while (true) {
            aNode->GetPropVal(wxT("Name"), &propValue);
            propValue.Trim(false);
            propValue.Trim(true);
            if (propValue == n) break;
            aNode = aNode->GetNext();
        }
        if (aNode) {
            aNode = FindNode(aNode->GetChildren(), wxT("font"));
            if (aNode) {
                if (FindNode(aNode->GetChildren(), wxT("fontHeight")))
                    ////SetWidth(iNode.ChildNodes.FindNode('fontHeight').Text,DefaultMeasurementUnit,tv.TextHeight);
                    // Fixed By Lubo, 02/2008
                    SetHeight(FindNode(aNode->GetChildren(), wxT("fontHeight"))->GetNodeContent(),
                              aDefaultMeasurementUnit, &aTextValue->textHeight, aActualConversion);
                if (FindNode(aNode->GetChildren(), wxT("strokeWidth")))
                    SetWidth(FindNode(aNode->GetChildren(), wxT("strokeWidth"))->GetNodeContent(),
                             aDefaultMeasurementUnit, &aTextValue->textstrokeWidth, aActualConversion);
            }
        }
    }
}

void CorrectTextPosition(TTEXTVALUE *aValue, int aRotation) {
    aValue->correctedPositionX = aValue->textPositionX;
    aValue->correctedPositionY = aValue->textPositionY;
    aValue->correctedPositionY = aValue->correctedPositionY - KiROUND((double)aValue->textHeight / 3.0);
    aValue->correctedPositionX = aValue->correctedPositionX + KiROUND(((double)aValue->text.Len() / 1.4) * ((double)aValue->textHeight / 1.8));
    if (aRotation == 900) {
        aValue->correctedPositionX = -aValue->textPositionY;
        aValue->correctedPositionY = aValue->textPositionX;
        aValue->correctedPositionX = aValue->correctedPositionX + KiROUND((double)aValue->textHeight / 3.0);
        aValue->correctedPositionY = aValue->correctedPositionY + KiROUND(((double)aValue->text.Len() / 1.4) * ((double)aValue->textHeight / 1.8));
    }
    if (aRotation == 1800) {
        aValue->correctedPositionX = -aValue->textPositionX;
        aValue->correctedPositionY = -aValue->textPositionY;
        aValue->correctedPositionY = aValue->correctedPositionY + KiROUND((double)aValue->textHeight / 3.0);
        aValue->correctedPositionX = aValue->correctedPositionX - KiROUND(((double)aValue->text.Len() / 1.4) * ((double)aValue->textHeight / 1.8));
    }
    if (aRotation == 2700) {
        aValue->correctedPositionX = aValue->textPositionY;
        aValue->correctedPositionY = -aValue->textPositionX;
        aValue->correctedPositionX = aValue->correctedPositionX + KiROUND((double)aValue->textHeight / 1.0);
        aValue->correctedPositionY = aValue->correctedPositionY - KiROUND(((double)aValue->text.Len() / 3.4) * ((double)aValue->textHeight / 1.8));
    }
}

wxXmlNode *FindNode(wxXmlNode *aChild, wxString aTag) {
    while (aChild) {
        if (aChild->GetName() == aTag) return aChild;

        aChild = aChild->GetNext();
    }

    return NULL;
}

void InitTTextValue(TTEXTVALUE *aTextValue) {
    aTextValue->text = wxEmptyString;
    aTextValue->textPositionX = 0;
    aTextValue->textPositionY = 0;
    aTextValue->textRotation = 0;
    aTextValue->textHeight = 0;
    aTextValue->textstrokeWidth = 0;
    aTextValue->textIsVisible = 0;
    aTextValue->mirror = 0;
    aTextValue->textUnit = 0;
    aTextValue->correctedPositionX = 0;
    aTextValue->correctedPositionY = 0;
}
