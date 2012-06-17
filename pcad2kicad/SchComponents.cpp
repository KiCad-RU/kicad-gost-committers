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
 * @file SchComponents.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/filename.h>

#include <common.h>

#include <SchComponents.h>
#include <SchArc.h>

CSchComponent::CSchComponent() {
    m_objType = '?';
    m_positionX = 0;
    m_positionY = 0;
    m_rotation = 0;
    m_mirror = 0;
    m_partNum = 0;
    m_width = 0;
    m_isVisible = 0;
}

CSchComponent::~CSchComponent() {
}

void CSchComponent::WriteToFile(wxFile *f, char ftype) {
}

CSch::CSch() {
    m_defaultMeasurementUnit = wxEmptyString;
    m_sizeX = 0;
    m_sizeY = 0;
}

CSch::~CSch() {
    int i;

    for (i = 0; i < (int)m_schComponents.GetCount(); i++)
        delete m_schComponents[i];
}

CSchModule::CSchModule() {
    int i;
    InitTTextValue(&m_name);
    InitTTextValue(&m_reference);
    m_numParts = 0;
    m_attachedPattern = wxEmptyString;
    m_moduleDescription = wxEmptyString;
    m_alias = wxEmptyString;

    for (i = 0; i < 10; i++)
        m_attachedSymbols[i] = wxEmptyString;
}

CSchModule::~CSchModule() {
    int i;

    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++)
        delete m_moduleObjects[i];
}

CSchJunction::CSchJunction() {
    m_net = wxEmptyString;
}

CSchJunction::~CSchJunction() {
}

void CSchJunction::WriteToFile(wxFile *f, char ftype) {
    f->Write(wxString::Format("Connection ~ %d %d\n", m_positionX, m_positionY));
}

CSchLine::CSchLine() {
    m_toX = 0;
    m_toY = 0;
    m_net = wxEmptyString;
    m_lineType = '?';
    InitTTextValue(&m_labelText);
}

CSchLine::~CSchLine() {
}

void CSchLine::WriteToFile(wxFile *f, char ftype) {
    wxString lt;

    if (ftype == 'L')
        f->Write(wxString::Format("P 2 %d 0 %d %d %d %d %d N\n",
                 m_partNum, m_width, m_positionX, m_positionY, m_toX, m_toY));

    if (ftype == 'S') {
        if (m_lineType == 'W') lt = wxString("Wire");
        if (m_lineType == 'B') lt = wxString("Bus");
        f->Write(wxT("Wire ") + lt + wxT(" Line\n"));
        f->Write(wxString::Format("               %d %d %d %d\n",
                 m_positionX, m_positionY, m_toX, m_toY));
    }
}

void CSchLine::WriteLabelToFile(wxFile *f, char ftype) {
    char lr;

    if (m_labelText.textIsVisible == 1) {
        if (m_labelText.textRotation == 0) lr = '0';
        else lr = '1';

        f->Write(wxString::Format("Text Label %d %d", m_labelText.textPositionX, m_labelText.textPositionY) +
             ' ' + lr + ' ' + wxT(" 60 ~\n"));
        f->Write(m_labelText.text + wxT("\n"));
    }
}

CSchPin::CSchPin() {
    InitTTextValue(&m_pinNum);
    InitTTextValue(&m_pinName);
    InitTTextValue(&m_number);
    m_pinType = wxEmptyString;
    m_edgeStyle = wxEmptyString;
    m_pinLength = 0;
}

CSchPin::~CSchPin() {
}

void CSchPin::WriteToFile(wxFile *f, char ftype) {
    char orientation, PType;
    wxString shape;

    orientation = 'L';
    if (m_rotation == 0) {
        orientation = 'L';
        m_positionX += m_pinLength; // Set corrected to KiCad position
    }
    if (m_rotation == 900) {
        orientation = 'D';
        m_positionY += m_pinLength; // Set corrected to KiCad position
    }
    if (m_rotation == 1800) {
        orientation = 'R';
        m_positionX -= m_pinLength; // Set corrected to KiCad position
    }
    if (m_rotation == 2700) {
        orientation = 'U';
        m_positionY -= m_pinLength; // Set corrected to KiCad position
    }

    PType = 'U';// Default
/*  E  Open E
    C Open C
    w Power Out
    W Power In
    U Unspec
    P Pasive
    T 3 State
    B BiDi
    O Output
    I Input */
    if (m_pinType == wxT("Pasive")) PType = 'P';
    if (m_pinType == wxT("Input")) PType = 'I';
    if (m_pinType == wxT("Output")) PType = 'O';
    if (m_pinType == wxT("Power")) PType = 'W';
    if (m_pinType == wxT("Bidirectional")) PType = 'B';
    shape = wxEmptyString; // Default , standard line without shape
    if (m_edgeStyle == wxT("Dot")) shape = 'I'; //Invert
    if (m_edgeStyle == wxT("Clock")) shape = 'C'; //Clock
    if (m_edgeStyle == wxT("???")) shape = wxT("IC"); //Clock Invert
    if (m_isVisible == 0) shape += 'N'; //Invisible
    //unit = 0 if common to the parts; if not, number of part (1. .n).
    //convert = 0 so common to the representations, if not 1 or 2.
    // Go out
    f->Write(wxT("X ") + m_pinName.text + ' ' + m_number.text + ' ' +
        wxString::Format("%d %d %d", m_positionX, m_positionY, m_pinLength) + ' ' + orientation +
        wxString::Format(" 30 30 %d 0 ", m_partNum) + PType + ' ' + shape + wxT("\n"));
}

void CSchModule::WriteToFile(wxFile *f, char ftype) {
    int i, symbolIndex;
    CorrectTextPosition(&m_name, m_rotation);
    CorrectTextPosition(&m_reference, m_rotation);
    // Go out
    f->Write(wxT("\n"));
    f->Write(wxT("#\n"));
    f->Write(wxT("# ") + m_name.text + wxT("\n"));
    f->Write(wxT("#\n"));
    f->Write("DEF " + ValidateName(m_name.text) + " U 0 40 Y Y " + wxString::Format("%d F N\n", m_numParts));

    // REFERENCE
    f->Write(wxT("F0 \"") + m_reference.text + "\" " +
        wxString::Format("%d %d 50 H V C C\n", m_reference.correctedPositionX, m_reference.correctedPositionY));
    // NAME
    f->Write(wxT("F1 \"") + m_name.text + "\" " +
        wxString::Format("%d %d 50 H V C C\n", m_name.correctedPositionX, m_name.correctedPositionY));
    // FOOTPRINT
    f->Write(wxT("F2 \"") + m_attachedPattern + wxT("\" 0 0 50 H I C C\n")); // invisible as default

    // Footprints filter
    if (m_attachedPattern.Len() > 0) {
        //$FPLIST  //SCHModule.AttachedPattern
        //14DIP300*
        //SO14*
        //$ENDFPLIST
        f->Write(wxT("$FPLIST\n"));
        f->Write(' ' + m_attachedPattern + wxT("*\n"));
        f->Write(wxT("$ENDFPLIST\n"));
    }
    // Alias
    if (m_alias.Len() > 0) {
        // ALIAS 74LS37 7400 74HCT00 74HC00
        f->Write(wxT("ALIAS") + m_alias + wxT("\n"));
    }

    f->Write(wxT("DRAW\n"));
    for (symbolIndex = 1; symbolIndex <= m_numParts; symbolIndex++) {
        // LINES=POLYGONS
        for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
            if (m_moduleObjects[i]->m_objType == 'L')
                if (((CSchLine *)m_moduleObjects[i])->m_partNum == symbolIndex)
                    m_moduleObjects[i]->WriteToFile(f, ftype);
        }
        // ARCS
        for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
            if (m_moduleObjects[i]->m_objType == 'A')
                if (((CSchArc *)m_moduleObjects[i])->m_partNum == symbolIndex)
                    m_moduleObjects[i]->WriteToFile(f, ftype);
        }
        // PINS
        for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
            if (m_moduleObjects[i]->m_objType == 'P')
                if (((CSchPin *)m_moduleObjects[i])->m_partNum == symbolIndex)
                    m_moduleObjects[i]->WriteToFile(f, ftype);
        }
    }
    f->Write(wxT("ENDDRAW\n"));   // ??
    f->Write(wxT("ENDDEF\n"));   // ??
}

CSchSymbol::CSchSymbol() {
    InitTTextValue(&m_module);
    InitTTextValue(&m_reference);
    InitTTextValue(&m_typ);
    m_attachedSymbol = wxEmptyString;
    m_attachedPattern = wxEmptyString;
}

CSchSymbol::~CSchSymbol() {
}

void CSchSymbol::WriteToFile(wxFile *f, char ftype) {
    char orientation;
    wxString visibility, str;
    int a, b, c, d;

    CorrectTextPosition(&m_typ, m_rotation);
    CorrectTextPosition(&m_reference, m_rotation);
    // Go out
    str = m_attachedSymbol;
    str.Replace(wxT(" "), wxT("~"), true);
    f->Write(wxT("L ") + str + ' ' + m_reference.text + wxT("\n"));
    f->Write(wxString::Format("U %d 1 00000000\n", m_partNum));
    f->Write(wxString::Format("P %d %d\n", m_positionX, m_positionY));
    // REFERENCE
    if (m_reference.textRotation == 900) orientation = 'V';
    else orientation = 'H';
    if (m_reference.textIsVisible == 1) visibility = wxT("0000");
    else visibility = wxT("0001");
    f->Write(wxT("F 0 \"") + m_reference.text + wxT("\" ") + orientation + ' ' +
        wxString::Format("%d %d %d",
                         m_reference.correctedPositionX + m_positionX,
                         m_reference.correctedPositionY + m_positionY,
                         KiROUND((double)m_reference.textHeight / 2.0)) + ' ' + visibility + wxT(" C C\n"));

    // TYP
    if (m_typ.textIsVisible == 1) visibility = wxT("0000");
    else visibility = wxT("0001");

    if (m_typ.textRotation == 900 || m_typ.textRotation == 2700) orientation = 'V';
    else orientation = 'H';
    f->Write(wxT("F 1 \"") + m_typ.text + wxT("\" ") + orientation + ' ' +
        wxString::Format("%d %d %d",
                         m_typ.correctedPositionX + m_positionX,
                         m_typ.correctedPositionY + m_positionY,
                         KiROUND((double)m_typ.textHeight / 2.0)) + ' ' + visibility + wxT(" C C\n"));

//  SOME ROTATION MATRICS ?????????????
//    1    2900 5200
/*  270 :
No Mirror       0    -1   -1    0
   MirrorX      0    -1    1    0
   MirrorY      0    -1    1    0

  180  :
No Mirror      -1   0    0     1
   MirrorX     -1   0    0    -1
   MirrorY      1   0    0     1

  R90  :
No Mirror       0    1    1    0
   MirrorX      0    1   -1    0
   MirrorY      0    1    -1   0

  0    :
No Mirror       1   0    0   -1
   MirrorX      1   0    0    1
   MirrorY     -1   0    0   -1
*/
    f->Write(wxString::Format(" 	%d %d %d\n", m_partNum, m_positionX, m_positionY));
    // Miror is negative in compare with PCad represenation...
    a = 0; b = 0; c = 0; d = 0;

    if (m_mirror == 0) {
        if (m_rotation == 0) {a = 1; d = -1; }
        if (m_rotation == 900) { b = -1; c = -1; }
        if (m_rotation == 1800) { a = -1; d = 1; }
        if (m_rotation == 2700) { b = 1; c = 1; }
    }

    if (m_mirror == 1) {
        if (m_rotation == 0) { a = -1; d = -1; }
        if (m_rotation == 900) { b = 1; c = -1; }
        if (m_rotation == 1800) { a = 1; d = 1; }
        if (m_rotation == 2700) { b = -1; c = 1; }
    }

    f->Write(wxString::Format(" %d %d %d %d \n", a, b, c, d));
    // FOOTPRINT
}

void CSch::WriteToFile(wxString fileName, char ftype) {
    wxFile f;
    int i;

    if (ftype == 'L') {
        // LIBRARY
        f.Open(fileName, wxFile::write);
        f.Write(wxT("EESchema-LIBRARY Version 2.3  Date: 01/1/2001-01:01:01\n"));
        f.Write(wxT("\n"));
        for (i = 0; i < (int)m_schComponents.GetCount(); i++) {
            if (m_schComponents[i]->m_objType == 'M')
                m_schComponents[i]->WriteToFile(&f, ftype);
        }
        f.Write(wxT("# End Library\n"));
        f.Close();    // also modules descriptions

        wxFileName dcmFile(fileName);
        dcmFile.SetExt(wxT("dcm"));
        f.Open(dcmFile.GetFullPath(), wxFile::write);
        f.Write(wxT("EESchema-DOCLIB  Version 2.0  Date: 01/01/2000-01:01:01\n"));
        for (i = 0; i < (int)m_schComponents.GetCount(); i++) {
            if (m_schComponents[i]->m_objType == 'M') {
                f.Write(wxT("#\n"));
                f.Write(wxT("$CMP ") + ValidateName(((CSchModule *)m_schComponents[i])->m_name.text) + wxT("\n"));
                f.Write(wxT("D ") + ((CSchModule *)m_schComponents[i])->m_moduleDescription + wxT("\n"));
                f.Write(wxT("K \n")); //no information available
                f.Write(wxT("$ENDCMP\n"));
            }
        }
        f.Write(wxT("#\n"));
        f.Write(wxT("#End Doc Library\n"));
        f.Close();
    } // LIBRARY

    if (ftype == 'S') {
        // SCHEMATICS
        f.Open(fileName, wxFile::write);
        f.Write(wxT("EESchema Schematic File Version 1\n"));
        wxFileName tmpFile(fileName);
        tmpFile.SetExt(wxEmptyString);
        f.Write(wxT("LIBS:") + tmpFile.GetFullPath() + wxT("\n"));
        f.Write(wxT("EELAYER 43  0\n"));
        f.Write(wxT("EELAYER END\n"));
        f.Write(wxT("$Descr User ") + wxString::Format("%d", m_sizeX) + ' ' + wxString::Format("%d", m_sizeY) + wxT("\n"));
        f.Write(wxT("$EndDescr\n"));
        // Junctions
        for (i = 0; i < (int)m_schComponents.GetCount(); i++) {
            if (m_schComponents[i]->m_objType == 'J')
                m_schComponents[i]->WriteToFile(&f, ftype);
        }
        // Lines
        for (i = 0; i < (int)m_schComponents.GetCount(); i++) {
            if (m_schComponents[i]->m_objType == 'L')
                m_schComponents[i]->WriteToFile(&f, ftype);
        }
        // Labels of lines - line and bus names
        for (i = 0; i < (int)m_schComponents.GetCount(); i++) {
            if (m_schComponents[i]->m_objType == 'L')
                ((CSchLine *)m_schComponents[i])->WriteLabelToFile(&f, ftype);
        }
        // Symbols
        for (i = 0; i < (int)m_schComponents.GetCount(); i++) {
            if (m_schComponents[i]->m_objType == 'S') {
                f.Write(wxT("$Comp\n"));
                m_schComponents[i]->WriteToFile(&f, ftype);
                f.Write(wxT("$EndComp\n"));
            }
        }

        f.Write(wxT("$EndSCHEMATC\n"));
        f.Close();
    } // SCHEMATICS
}
