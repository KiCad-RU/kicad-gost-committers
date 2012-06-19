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
 * @file Sch.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/xml/xml.h>
#include <Sch.h>
#include <XMLtoObjectCommonProceduresUnit.h>
#include <common.h>

#include <SchArc.h>
#include <SchBus.h>
#include <SchJunction.h>
#include <SchLine.h>
#include <SchModule.h>
#include <SchPin.h>
#include <SchSymbol.h>


void DoAlias(wxString alias, CSch *sch) {
    int i;
    wxString a, n;

    for (i = 0; i < (int)sch->m_schComponents.GetCount(); i++) {
        if ((sch->m_schComponents[i])->m_objType == 'M') {
            n = alias;
            a = GetWord(&n);
            n.Trim(true);
            n.Trim(false);
            if (((CSchModule *)sch->m_schComponents[i])->m_name.text == n) {
                ((CSchModule *)sch->m_schComponents[i])->m_alias = ((CSchModule *)sch->m_schComponents[i])->m_alias + ' ' + a;
            }
        }
    }
}

// FULL PROCESS OF LIBRARY CONVERSION
void DoLibrary(wxXmlDocument *xmlDoc, CSch *sch, wxStatusBar* statusBar, wxString actualConversion) {
    wxXmlNode *aNode;
    wxString propValue;
    CSchModule *module;

    aNode = FindNode(xmlDoc->GetRoot()->GetChildren(), wxT("library"));
    if (aNode) { // ORIGINAL Modules
        aNode = aNode->GetChildren();
        while (aNode) {
            if (aNode->GetName() == wxT("compDef")) {
                module = new CSchModule;
                module->Parse(aNode, statusBar, sch->m_defaultMeasurementUnit, actualConversion);
                sch->m_schComponents.Add(module);
            }
            aNode = aNode->GetNext();
        }
    }

    aNode = FindNode(xmlDoc->GetRoot()->GetChildren(), wxT("library"));
    if (aNode) { // ALIASes
        aNode = aNode->GetChildren();
        while (aNode) {
            if (aNode->GetName() == wxT("compAlias")) {
                aNode->GetPropVal(wxT("Name"), &propValue);
                DoAlias(propValue, sch);
            }
            aNode = aNode->GetNext();
        }
    }
}

// LinesIntersect procedure is copied from website
// http://www.pdas.com/lineint.htm
// extended format replaced by integer
void LinesIntersect(const int x1, const int y1, const int x2, const int y2, // first line
                    const int x3, const int y3, const int x4, const int y4, // second line
                    int *code, // =0 OK; =1 lines parallel
                    int *x, int *y) { // intersection point
    long double a1, a2, b1, b2, c1, c2; // Coefficients of line eqns./
    long double denom;

    a1 = y2 - y1;
    b1 = x1 - x2;
    c1 = (long double)x2 * (long double)y1 - (long double)x1 * (long double)y2;  // a1*x + b1*y + c1 = 0 is line 1

    a2 = y4 - y3;
    b2 = x3 - x4;
    c2 = (long double)x4 * (long double)y3 - (long double)x3 * (long double)y4;  // a2*x + b2*y + c2 = 0 is line 2

    denom = a1 * b2 - a2 * b1;
    if (denom == 0) {
        *code = 1;
    }
    else {
        *x = KiROUND((b1 * c2 - b2 * c1) / denom);
        *y = KiROUND((a2 * c1 - a1 * c2) / denom);
        *code = 0;
    }
}

bool IsPointOnLine(int x, int y, CSchLine *l) {
    long double a, b, c; // Coefficients of line eqns./
    bool px, py, result;

    a = l->m_toY - l->m_positionY;
    b = l->m_positionX - l->m_toX;
    c = (long double)l->m_toX * (long double)l->m_positionY -
        (long double)l->m_positionX * (long double)l->m_toY;  // a1*x + b1*y + c1 = 0 is line 1
    px = false; py = false;
    if (a * (long double)x + b * (long double)y + c == 0) {
        if (l->m_positionX <= l->m_toX && x >= l->m_positionX && x <= l->m_toX)
            px = true;
        if (l->m_toX <= l->m_positionX && x >= l->m_toX && x <= l->m_positionX)
            px = true;
        if (l->m_positionY <= l->m_toY && y >= l->m_positionY && y <= l->m_toY)
            py = true;
        if (l->m_toY <= l->m_positionY && y >= l->m_toY && y <= l->m_positionY)
            py = true;
        if (px && py) result = true;
    }
    else result = false;

    return result;
}

CSchJunction *CheckJunction(CSchLine *iSchLine, int index, CSch *sch) {
    CSchJunction *result = NULL;
    int i, j, code, x, y;
    bool p;

    for (i = 0; i < (int)sch->m_schComponents.GetCount(); i++) {
        if ((sch->m_schComponents[i])->m_objType == 'L' &&
             index != i) { // not  for itself
            if (iSchLine->m_net == ((CSchLine *)sch->m_schComponents[i])->m_net) {
                // IS JUNCTION ????
                LinesIntersect(iSchLine->m_positionX, iSchLine->m_positionY, iSchLine->m_toX, iSchLine->m_toY,
                               ((CSchLine *)sch->m_schComponents[i])->m_positionX,
                               ((CSchLine *)sch->m_schComponents[i])->m_positionY,
                               ((CSchLine *)sch->m_schComponents[i])->m_toX,
                               ((CSchLine *)sch->m_schComponents[i])->m_toY,
                               &code, &x, &y);
                if (code == 0) {
                    // there is intersection, is it inside line segment ?
                    if (IsPointOnLine(x, y, iSchLine)) {
                        // Point Candidate
                        // Firstly to check if the otrher line with dufferent NET is crossing this point, if so, there is not point !
                        p = true;
                        for (j = 0; j < (int)sch->m_schComponents.GetCount(); j++) {
                            if ((sch->m_schComponents[j])->m_objType == 'L'
                                 && p) { // not  for itself
                                if (((CSchLine *)sch->m_schComponents[j])->m_net != ((CSchLine *)sch->m_schComponents[i])->m_net)
                                    if (IsPointOnLine(x, y, (CSchLine *)sch->m_schComponents[j]))
                                           p = false; // NOT POINT - net cross
                            }
                        }
                        if (p) result = new CSchJunction(x, y, iSchLine->m_net); //CJunction(x, y, iSchLine->m_net);
                    }
                }
            }
        }
    }

    return result;
}

void ProcessXMLtoSch(CSch *sch, wxStatusBar* statusBar, wxString XMLFileName, wxString actualConversion) {
    wxXmlDocument xmlDoc;
    wxXmlNode *aNode;
    CSchComponent *schComp;
    CSchJunction *schJunction;
    CSchLine *line;
    CSchBus *bus;
    CSchJunction *junction;
    CSchSymbol *symbol;
    bool isJunction;
    int i;

    if (!xmlDoc.Load(XMLFileName)) return;

    // Defaut measurement units
    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("asciiHeader"));
    if (aNode) {
        aNode = FindNode(aNode->GetChildren(), wxT("fileUnits"));
        if (aNode) {
            sch->m_defaultMeasurementUnit = aNode->GetNodeContent();
            sch->m_defaultMeasurementUnit.Trim(true);
            sch->m_defaultMeasurementUnit.Trim(false);
        }
    }

    // Allways also library file
    isJunction = false;
    actualConversion = wxT("SCHLIB");

    DoLibrary(&xmlDoc, sch, statusBar, actualConversion);

    aNode = FindNode(xmlDoc.GetRoot()->GetChildren(), wxT("schematicDesign"));
    if (aNode) {
        //  SCHEMATIC FILE
        // aNode is schematicDesign node actually
        actualConversion = wxT("SCH");
        aNode = FindNode(aNode->GetChildren(), wxT("sheet"))->GetChildren();
        while (aNode) {
            if (aNode->GetName() == wxT("symbol")) {
                symbol = new CSchSymbol;
                symbol->Parse(aNode, sch->m_defaultMeasurementUnit, actualConversion);
                sch->m_schComponents.Add(symbol);
            }

            if (aNode->GetName() == wxT("wire"))
                if (FindNode(aNode->GetChildren(), wxT("line"))) {
                    line = new CSchLine;
                    line->Parse(FindNode(aNode->GetChildren(), wxT("line")), 0,
                        sch->m_defaultMeasurementUnit, actualConversion);
                    sch->m_schComponents.Add(line);
                }

            if (aNode->GetName() == wxT("bus")) {
                bus = new CSchBus;
                bus->Parse(aNode, sch->m_defaultMeasurementUnit, actualConversion);
                sch->m_schComponents.Add(bus);
            }

            if (aNode->GetName() == wxT("junction")) {
                isJunction = true;
                junction = new CSchJunction();
                junction->Parse(aNode, sch->m_defaultMeasurementUnit, actualConversion);
                sch->m_schComponents.Add(junction);
            }

            aNode = aNode->GetNext();
        }

        // POSTPROCESS -- SET/OPTIMIZE NEW SCH POSITION

        sch->m_sizeX = 1000000; sch->m_sizeY = 0;
        for (i = 0; i < (int)sch->m_schComponents.GetCount(); i++) {
            schComp = sch->m_schComponents[i];
            if (schComp->m_positionY < sch->m_sizeY)
                sch->m_sizeY = schComp->m_positionY;
            if (schComp->m_positionX < sch->m_sizeX && schComp->m_positionX > 0)
                sch->m_sizeX = schComp->m_positionX;
        }
        // correction
        sch->m_sizeY = sch->m_sizeY - 1000;
        sch->m_sizeX = sch->m_sizeX - 1000;
        for (i = 0; i < (int)sch->m_schComponents.GetCount(); i++) {
            schComp = sch->m_schComponents[i];
            schComp->m_positionY -= sch->m_sizeY;
            schComp->m_positionX -= sch->m_sizeX;
            if (schComp->m_objType == 'L') {
                ((CSchLine *)schComp)->m_toY -= sch->m_sizeY;
                ((CSchLine *)schComp)->m_labelText.textPositionY -= sch->m_sizeY;
                ((CSchLine *)schComp)->m_toX -= sch->m_sizeX;
                ((CSchLine *)schComp)->m_labelText.textPositionX -= sch->m_sizeX;
            }
        }
        // final sheet settings
        for (i = 0; i < (int)sch->m_schComponents.GetCount(); i++) {
            schComp = sch->m_schComponents[i];
            if (schComp->m_positionY < sch->m_sizeY)
                sch->m_sizeY = schComp->m_positionY;
            if (schComp->m_positionX > sch->m_sizeX)
                sch->m_sizeX = schComp->m_positionX;
        }
        sch->m_sizeY = -sch->m_sizeY; // is in absolute units
        sch->m_sizeX = sch->m_sizeX + 1000;
        sch->m_sizeY = sch->m_sizeY + 1000;
        // A4 is minimum $Descr A4 11700 8267
        if (sch->m_sizeX < 11700) sch->m_sizeX = 11700;
        if (sch->m_sizeY < 8267) sch->m_sizeY = 8267;

        // POSTPROCESS -- CREATE JUNCTIONS FROM NEWTLIST
        if (!isJunction) {
            if (wxMessageBox(wxT("There are not JUNCTIONS in your schematics file ."
                                 " It can be, that your design is without Junctions."
                                 " But it can be that your input file is in obsolete format."
                                 " Would you like to run postprocess and create junctions from Netlist/Wires information ?"
                                 "    YOU HAVE TO CHECK/CORRECT Juntions in converted design, placement is only approximation !"),
                             wxEmptyString, wxYES_NO) == wxYES)
            {
                for (i = 0; i < (int)sch->m_schComponents.GetCount(); i++) {
                    schComp = sch->m_schComponents[i];
                    if (schComp->m_objType == 'L') {
                        schJunction = CheckJunction((CSchLine *)schComp, i, sch);
                        if (schJunction)
                            sch->m_schComponents.Add(schJunction);
                    }
                }
            }
        }

    } //  SCHEMATIC FILE
}
