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
 * @file PCBComponents.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <PCBComponents.h>


int ToInt(char c) {
    if (c >= '0' && c <= '9') return (c - '0');
    else return (c - 'A' + 10);
}

// The former mentioned here that the following implementation needs to be revised
char OrMask(char m1, char m2) {
    wxString s;
    int i;

    if (m1 == m2) return m1;

    i = ToInt(m1);
    i += ToInt(m2);
    s = wxString::Format("%X", i);
    return s[0];
}

/*
0 Copper layer
1 to 14   Inner layers
15 Component layer
16 Copper side adhesive layer    Technical layers
17 Component side adhesive layer
18 Copper side Solder paste layer
19 Component Solder paste layer
20 Copper side Silk screen layer
21 Component Silk screen layer
22 Copper side Solder mask layer
23 Component Solder mask layer
24 Draw layer (Used for general drawings)
25 Comment layer (Other layer used for general drawings)
26 ECO1 layer (Other layer used for general drawings)
26 ECO2 layer (Other layer used for general drawings)
27 Edge layer. Items on Edge layer are seen on all layers
*/

wxString KiCadLayerMask(wxString mask, int l) {
    wxString newmask;
    int i;
    /* Sometimes, a mask layer parameter is used.
       It is a 32 bits mask used to indicate a layer group usage (0 up to 32 layers).
       A mask layer parameter is given in hexadecimal form.
       Bit 0 is the copper layer, bit 1 is the inner 1 layer, and so on...(Bit 27 is the Edge layer).
       Mask layer is the ORed mask of the used layers */

    newmask = wxT("00000000");        // default
    if (l == 0) newmask = wxT("00000001");
    if (l == 15) newmask= wxT("00008000");
    if (l == 16) newmask= wxT("00010000");
    if (l == 17) newmask= wxT("00020000");
    if (l == 18) newmask= wxT("00040000");
    if (l == 19) newmask= wxT("00080000");
    if (l == 20) newmask= wxT("00100000");
    if (l == 21) newmask= wxT("00200000");
    if (l == 22) newmask= wxT("00400000");
    if (l == 23) newmask= wxT("00800000");
    if (l == 24) newmask= wxT("01000000");
    if (l == 25) newmask= wxT("02000000");
    if (l == 26) newmask= wxT("04000000");
    if (l == 27) newmask= wxT("08000000");
    if (l == 28) newmask= wxT("10000000");

    for (i = 0; i < 8; i++)
        newmask[i] = OrMask(mask[i], newmask[i]);

    return newmask;
}

int FlipLayers(int l) {
    int result = l; // no swap default....

    // routed layers
    if (l == 0) result = 15;
    if (l == 15) result = 0;

    // Silk
    if (l == 21) result = 20;
    if (l == 20) result = 21;

    //Paste
    if (l == 19) result = 18;
    if (l == 18) result = 19;

    // Mask
    if (l == 23) result = 22;
    if (l == 22) result = 23;

    return result;
}

CNetNode::CNetNode() {
    m_compRef = wxEmptyString;
    m_pinRef = wxEmptyString;
}

CNetNode::~CNetNode() {
}

CNet::CNet(wxString iName) {
    m_name = iName;
}

CNet::~CNet() {
    int i;

    for (i = 0; i < (int)m_netNodes.GetCount(); i++) {
        delete m_netNodes[i];
    }
}

CPCBComponent::CPCBComponent() {
    m_tag = 0;
    m_objType = '?';
    m_PCadLayer = 0;
    m_KiCadLayer = 0;
    m_timestamp = 0;
    m_positionX = 0;
    m_positionY = 0;
    m_rotation = 0;
    InitTTextValue(&m_name);
    m_net = wxEmptyString;
    m_compRef = wxEmptyString;
    m_patGraphRefName = wxEmptyString;
}

CPCBComponent::~CPCBComponent() {
}

void CPCBComponent::WriteToFile(wxFile *f, char ftype) {
}

void CPCBComponent::SetPosOffset(int x_offs, int y_offs) {
    m_positionX += x_offs;
    m_positionY += y_offs;
}

CPCBPadViaShape::CPCBPadViaShape() {
    CPCBComponent::CPCBComponent();
    m_shape = wxEmptyString;
    m_width = 0;
    m_height = 0;
}

CPCBPadViaShape::~CPCBPadViaShape() {
}

CPCBLine::CPCBLine() {
    CPCBComponent::CPCBComponent();
    m_width = 0;
    m_toX = 0;
    m_toY = 0;
    m_objType = 'L';
}

CPCBLine::~CPCBLine() {
}

void CPCBLine::SetPosOffset(int x_offs, int y_offs) {
    CPCBComponent::SetPosOffset(x_offs, y_offs);
    m_toX += x_offs;
    m_toY += y_offs;
}

void CPCBLine::WriteToFile(wxFile *f, char ftype) {
    if (ftype == 'L') { // Library
        f->Write(wxString::Format("DS %d %d %d %d %d %d\n", m_positionX, m_positionY,
                 m_toX, m_toY, m_width, m_KiCadLayer)); // Position
    }

    if (ftype == 'P') { // PCB
        f->Write(wxString::Format("Po 0 %d %d %d %d %d\n", m_positionX, m_positionY,
                 m_toX, m_toY, m_width));
        if (m_timestamp == 0)
            f->Write(wxString::Format("De %d 0 0 0 0\n", m_KiCadLayer));
        else f->Write(wxString::Format("De %d 0 0 %8X 0\n", m_KiCadLayer, m_timestamp));
    }
}

CPCBPolygon::CPCBPolygon() {
    CPCBComponent::CPCBComponent();
    m_width = 0;
    m_objType = 'Z';
}

CPCBPolygon::~CPCBPolygon() {
    int i, island;

    for (i = 0; i < (int)m_outline.GetCount(); i++) {
        delete m_outline[i];
    }

    for (island = 0; island < (int)m_cutouts.GetCount(); island++) {
        for (i = 0; i < (int)m_cutouts[island]->GetCount(); i++) {
            delete (*m_cutouts[island])[i];
        }

        delete m_cutouts[island];
    }

    for (island = 0; island < (int)m_islands.GetCount(); island++) {
        for (i = 0; i < (int)m_islands[island]->GetCount(); i++) {
            delete (*m_islands[island])[i];
        }

        delete m_islands[island];
    }
}

void CPCBPolygon::WriteToFile(wxFile *f, char ftype) {
}

void CPCBPolygon::WriteOutlineToFile(wxFile *f, char ftype) {
    int i, island;
    char c;

    if (m_outline.GetCount() > 0) {
        f->Write(wxT("$CZONE_OUTLINE\n"));
        f->Write(wxString::Format("ZInfo %8X 0 \"", m_timestamp) + m_net + wxT("\"\n"));
        f->Write(wxString::Format("ZLayer %d\n", m_KiCadLayer));
        f->Write(wxString::Format("ZAux %d E\n", (int)m_outline.GetCount()));
        // print outline
        c = '0';
        for (i = 0; i < (int)m_outline.GetCount(); i++) {
            if (i == (int)m_outline.GetCount() - 1) c = '1';
            f->Write(wxString::Format("ZCorner %d %d %c\n", KiROUND(m_outline[i]->x), KiROUND(m_outline[i]->y), c));
        }

        // print cutouts
        for (island = 0; island < (int)m_cutouts.GetCount(); island++) {
            c = '0';
            for (i = 0; i < (int)m_cutouts[island]->GetCount(); i++) {
                if (i == (int)m_cutouts[island]->GetCount() - 1) c = '1';
                f->Write(wxString::Format("ZCorner %d %d %c\n",
                         KiROUND((*m_cutouts[island])[i]->x), KiROUND((*m_cutouts[island])[i]->y), c));
            }
        }

        // print filled islands
        for (island = 0; island < (int)m_islands.GetCount(); island++) {
            f->Write(wxT("$POLYSCORNERS\n"));
            c = '0';
            for (i = 0; i < (int)m_islands[island]->GetCount(); i++) {
                if (i == (int)m_islands[island]->GetCount() - 1) c = '1';
                f->Write(wxString::Format("%d %d %c 0\n",
                         KiROUND((*m_islands[island])[i]->x), KiROUND((*m_islands[island])[i]->y), c));
            }
            f->Write(wxT("$endPOLYSCORNERS\n"));
        }

        f->Write(wxT("$endCZONE_OUTLINE\n"));
    }
}

void CPCBPolygon::SetPosOffset(int x_offs, int y_offs) {
    int i, island;

    CPCBComponent::SetPosOffset(x_offs, y_offs);

    for (i = 0; i < (int)m_outline.GetCount(); i++) {
        m_outline[i]->x += x_offs;
        m_outline[i]->y += y_offs;
    }

    for (island = 0; island < (int)m_islands.GetCount(); island++) {
        for (i = 0; i < (int)m_islands[island]->GetCount(); i++) {
            (*m_islands[island])[i]->x += x_offs;
            (*m_islands[island])[i]->y += y_offs;
        }
    }

    for (island = 0; island < (int)m_cutouts.GetCount(); island++) {
        for (i = 0; i < (int)m_cutouts[island]->GetCount(); i++) {
            (*m_cutouts[island])[i]->x += x_offs;
            (*m_cutouts[island])[i]->y += y_offs;
        }
    }
}

CPCBCopperPour::CPCBCopperPour() : CPCBPolygon() {
    CPCBPolygon::CPCBPolygon();
}

CPCBCopperPour::~CPCBCopperPour() {
}

CPCBCutout::CPCBCutout() {
    CPCBComponent::CPCBComponent();
    m_objType = 'C';
}

CPCBCutout::~CPCBCutout() {
}

void CPCBCutout::WriteToFile(wxFile *f, char ftype) {
    //no operation
    //(It seems that the same cutouts (with the same vertices) are inside of copper pour objects)
}

CPCBPad::CPCBPad(wxString iName) {
    CPCBComponent::CPCBComponent();
    m_objType = 'P';
    m_number = 0;
    m_hole = 0;
    m_name.text = iName;
}

void CPCBPad::WriteToFile(wxFile *f, char ftype, int r) {
    CPCBPadViaShape *padShape;
    wxString s, layerMask, padType;
    int i, lc, ls;

    if (ftype == 'P') { // PCB
        for (i = 0; i < (int)m_shapes.GetCount(); i++) {
            padShape = m_shapes[i];

            if (padShape->m_width > 0 || padShape->m_height > 0) { // maybe should not to be filtered ????
                s = wxT("3") ; // default
                f->Write(wxT("Po ") + s + wxString::Format(" %d %d %d %d %d %d\n", m_positionX, m_positionY,
                         m_positionX, m_positionY, padShape->m_height, m_hole));
                f->Write(wxString::Format("De %d 1 0 0 0\n", padShape->m_KiCadLayer));
            }
        }
    }
    else {
        // Library
        lc = 0 ; ls = 0;
        // Is it SMD pad , or not ?
        for (i = 0; i < (int)m_shapes.GetCount(); i++) {
            padShape = m_shapes[i];
            if (padShape->m_width > 0 && padShape->m_height > 0) {
                if (padShape->m_KiCadLayer == 15) lc++; // Component
                if (padShape->m_KiCadLayer == 0) ls++;  // Cooper
            }
        }

        // And default layers mask
        layerMask = wxT("00C08001"); //Comp,Coop,SoldCmp,SoldCoop
        padType = wxT("STD");
        if (lc == 0 || ls == 0) {
            if (m_hole == 0) {
                padType = wxT("SMD");
                if (ls > 0) layerMask = wxT("00440001");
                if (lc > 0) layerMask = wxT("00888000");
            }
            else {
                if (ls > 0) layerMask = wxT("00400001");
                if (lc > 0) layerMask = wxT("00808000");
            }
        }

        // Go out
        for (i = 0; i < (int)m_shapes.GetCount(); i++) {
            padShape = m_shapes[i];
            if (padShape->m_width > 0 || padShape->m_height > 0) { // maybe should not to be filtered ????
                if (padShape->m_shape == wxT("Oval")) {
                    if (padShape->m_width != padShape->m_height) s = wxT("O");
                    else s = wxT("C");
                }
                else if (padShape->m_shape == wxT("Ellipse")) s = wxT("O");
                else if (padShape->m_shape == wxT("Rect") || padShape->m_shape == wxT("RndRect")) s = wxT("R");
                else if (padShape->m_shape == wxT("Polygon")) s = wxT("R"); // approximation.....

                f->Write(wxT("$PAD\n"));
                f->Write(wxT("Sh \"") + m_name.text + wxT("\" ") + s +
                         wxString::Format(" %d %d 0 0 %d\n",
                             padShape->m_width, padShape->m_height, m_rotation + r)); // Name, Shape, Xsize Ysize Xdelta Ydelta Orientation

                f->Write(wxString::Format("Dr %d 0 0\n", m_hole)); // Hole size , OffsetX, OffsetY

                layerMask = KiCadLayerMask(layerMask, padShape->m_KiCadLayer);
                f->Write(wxT("At ") + padType + wxT(" N ") + layerMask + wxT("\n")); // <Pad type> N <layer mask>
                f->Write(wxT("Ne 0 \"") + m_net + "\"\n"); // Reference
                f->Write(wxString::Format("Po %d %d\n", m_positionX, m_positionY)); // Position
                f->Write(wxT("$EndPAD\n"));
            }
        }
    }
}

CPCBPad::~CPCBPad() {
    int i;

    for (i = 0; i < (int)m_shapes.GetCount(); i++) {
        delete m_shapes[i];
    }
}

CPCBVia::CPCBVia() : CPCBPad(wxEmptyString) {
    m_objType = 'V';
}

CPCBVia::~CPCBVia() {
}

CPCBText::CPCBText() {
    CPCBComponent::CPCBComponent();
    m_objType = 'T';
}

CPCBText::~CPCBText() {
}

void CPCBText::WriteToFile(wxFile *f, char ftype) {
    char visibility, mirrored;

    if (m_name.textIsVisible == 1) visibility = 'V';
    else visibility = 'I';

    if (m_name.mirror == 1) mirrored = 'M';
    else mirrored = 'N';

    // Simple, not the best, but acceptable text positioning.....
    CorrectTextPosition(&m_name, m_rotation);

    // Go out
    if (ftype == 'L') { // Library component
        f->Write(wxString::Format("T%d %d %d %d %d %d %d ", m_tag, m_name.correctedPositionX, m_name.correctedPositionY,
                 KiROUND(m_name.textHeight / 2), KiROUND(m_name.textHeight / 1.1),
                 m_rotation, m_name.textstrokeWidth) + mirrored + ' ' + visibility +
                 wxString::Format(" %d \"", m_KiCadLayer) + m_name.text + wxT("\"\n")); // ValueString
    }

    if (ftype == 'P') { // Library component
        if (m_name.mirror == 1) mirrored = '0';
        else mirrored = '1';

        f->Write(wxT("Te \"") + m_name.text + wxT("\"\n"));

        f->Write(wxString::Format("Po %d %d %d %d %d %d\n", m_name.correctedPositionX, m_name.correctedPositionY,
                 KiROUND(m_name.textHeight / 2), KiROUND(m_name.textHeight / 1.1),
                 m_name.textstrokeWidth, m_rotation));

        f->Write(wxString::Format("De %d ", m_KiCadLayer) + mirrored + wxT(" 0 0\n"));
    }
}

void CPCBText::SetPosOffset(int x_offs, int y_offs) {
    CPCBComponent::SetPosOffset(x_offs, y_offs);
    m_name.textPositionX += x_offs;
    m_name.textPositionY += y_offs;
}

CPCBArc::CPCBArc() {
    CPCBComponent::CPCBComponent();
    m_objType = 'A';
    m_startX = 0;
    m_startY = 0;
    m_angle = 0;
    m_width = 0;
}

CPCBArc::~CPCBArc() {
}

void CPCBArc::WriteToFile(wxFile *f, char ftype) {
/*
 DC ox oy fx fy w  DC is a Draw Circle  DC Xcentre Ycentre Xpoint Ypoint Width Layer
 DA x0 y0 x1 y1 angle width layer  DA is a Draw ArcX0,y0 = Start point x1,y1 = end point
*/
    if (ftype == 'L') { // Library component
        f->Write(wxString::Format("DA %d %d %d %d %d %d %d\n", m_positionX, m_positionY, m_startX,
                 m_startY, m_angle, m_width, m_KiCadLayer)); // ValueString
    }

    if (ftype == 'P') { // PCB
        f->Write(wxString::Format("Po 2 %d %d %d %d %d", m_positionX, m_positionY,
                 m_startX, m_startY, m_width));
        f->Write(wxString::Format("De %d 0 %d 0 0\n", m_KiCadLayer, -m_angle));
    }
}

void CPCBArc::SetPosOffset(int x_offs, int y_offs) {
    CPCBComponent::SetPosOffset(x_offs, y_offs);
    m_startX += x_offs;
    m_startY += y_offs;
}

CPCBModule::CPCBModule(wxString iName) {
    CPCBComponent::CPCBComponent();
    InitTTextValue(&m_value);
    m_mirror = 0;
    m_objType = 'M';  // MODULE
    m_name.text = iName;
    m_KiCadLayer = 21; // default
}

wxString ModuleLayer(int mirror) {
    wxString result;

 /////NOT !   {IntToStr(KiCadLayer)}    NOT !
 ///  MODULES ARE HARD PLACED ON COMPONENT OR COPPER LAYER.
 ///  IsFLIPPED--> MIRROR attribute is decision Point!!!

    if (mirror == 0) result = wxT("15"); //Components side
    else result = wxT("0"); // Copper side

    return result;
}

void CPCBModule::WriteToFile(wxFile *f, char ftype) {
    char visibility, mirrored;
    int i;

    // transform text positions ....
    CorrectTextPosition(&m_name, m_rotation);
    CorrectTextPosition(&m_value, m_rotation);
    // Go out
    f->Write(wxT("\n"));
    f->Write(wxT("$MODULE ") + m_name.text + wxT("\n"));
    f->Write(wxString::Format("Po %d %d %d ", m_positionX, m_positionY, m_rotation) +
            ModuleLayer(m_mirror) + wxT(" 00000000 00000000 ~~\n")); // Position
    f->Write(wxT("Li ") + m_name.text + wxT("\n"));   // Modulename
    f->Write(wxT("Sc 00000000\n")); // Timestamp
    f->Write(wxT("Op 0 0 0\n")); // Orientation
    f->Write(wxT("At SMD\n"));   // ??

    // MODULE STRINGS
    if (m_name.textIsVisible == 1) visibility = 'V';
    else visibility = 'I';

    if (m_name.mirror == 1) mirrored = 'M';
    else mirrored = 'N';

    f->Write(wxString::Format("T0 %d %d %d %d %d %d", m_name.correctedPositionX, m_name.correctedPositionY,
               KiROUND(m_name.textHeight / 2), KiROUND(m_name.textHeight / 1.5),
               m_name.textRotation, m_value.textstrokeWidth /* TODO: Is it correct to use m_value.textstrokeWidth here? */) +
               ' ' + mirrored + ' ' + visibility + wxString::Format(" %d \"", m_KiCadLayer) + m_name.text + wxT("\"\n")); // NameString

    if (m_value.textIsVisible == 1) visibility = 'V';
    else visibility = 'I';

    if (m_value.mirror == 1) mirrored = 'M';
    else mirrored = 'N';

    f->Write(wxString::Format("T1 %d %d %d %d %d %d", m_value.correctedPositionX, m_value.correctedPositionY,
               KiROUND(m_value.textHeight / 2), KiROUND(m_value.textHeight / 1.5),
               m_value.textRotation, m_value.textstrokeWidth) +
               ' ' + mirrored + ' ' + visibility + wxString::Format(" %d \"", m_KiCadLayer) + m_value.text + wxT("\"\n")); // ValueString

    // TEXTS
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'T') {
            ((CPCBText *)m_moduleObjects[i])->m_tag = i + 2;
            m_moduleObjects[i]->WriteToFile(f, ftype);
        }
    }

    // MODULE LINES
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'L')
            m_moduleObjects[i]->WriteToFile(f, ftype);
    }

    // MODULE Arcs
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'A')
            m_moduleObjects[i]->WriteToFile(f, ftype);
    }

    // PADS
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'P')
            ((CPCBPad *)m_moduleObjects[i])->WriteToFile(f, ftype, m_rotation);
    }

    // VIAS
    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        if (m_moduleObjects[i]->m_objType == 'V')
            ((CPCBVia *)m_moduleObjects[i])->WriteToFile(f, ftype, m_rotation);
    }

    // END
    f->Write(wxT("$EndMODULE ") + m_name.text + wxT("\n"));
}

void CPCBModule::Flip() {
    int i, j;

    if (m_mirror == 1) {
        // Flipped
        m_KiCadLayer = FlipLayers(m_KiCadLayer);
        m_rotation = -m_rotation;
        m_name.textPositionX = -m_name.textPositionX;
        m_name.mirror = m_mirror;
        m_value.textPositionX = -m_value.textPositionX;
        m_value.mirror = m_mirror;

        for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
            // MODULE LINES
            if (m_moduleObjects[i]->m_objType == 'L') {
                m_moduleObjects[i]->m_positionX = -m_moduleObjects[i]->m_positionX;
                ((CPCBLine *)m_moduleObjects[i])->m_toX = -((CPCBLine *)m_moduleObjects[i])->m_toX;
                m_moduleObjects[i]->m_KiCadLayer = FlipLayers(m_moduleObjects[i]->m_KiCadLayer);
            }

            // MODULE Arcs
            if (m_moduleObjects[i]->m_objType == 'A') {
                m_moduleObjects[i]->m_positionX = -m_moduleObjects[i]->m_positionX;
                ((CPCBArc *)m_moduleObjects[i])->m_startX = -((CPCBArc *)m_moduleObjects[i])->m_startX;
                m_moduleObjects[i]->m_KiCadLayer = FlipLayers(m_moduleObjects[i]->m_KiCadLayer);
            }

            // PADS
            if (m_moduleObjects[i]->m_objType == 'P') {
                m_moduleObjects[i]->m_positionX = -m_moduleObjects[i]->m_positionX;
                m_moduleObjects[i]->m_rotation = -m_moduleObjects[i]->m_rotation;

                for (j = 0; j < (int)((CPCBPad *)m_moduleObjects[i])->m_shapes.GetCount(); j++)
                    ((CPCBPad *)m_moduleObjects[i])->m_shapes[j]->m_KiCadLayer =
                        FlipLayers(((CPCBPad *)m_moduleObjects[i])->m_shapes[j]->m_KiCadLayer);
            }

            // VIAS
            if (m_moduleObjects[i]->m_objType == 'V') {
                m_moduleObjects[i]->m_positionX = -m_moduleObjects[i]->m_positionX;

                for (j = 0; j < (int)((CPCBVia *)m_moduleObjects[i])->m_shapes.GetCount(); j++)
                    ((CPCBVia *)m_moduleObjects[i])->m_shapes[j]->m_KiCadLayer =
                        FlipLayers(((CPCBVia *)m_moduleObjects[i])->m_shapes[j]->m_KiCadLayer);
            }
        }
    }
}

CPCBModule::~CPCBModule() {
    int i;

    for (i = 0; i < (int)m_moduleObjects.GetCount(); i++) {
        delete m_moduleObjects[i];
    }
}

CPCB::CPCB() {
    int i;

    m_defaultMeasurementUnit = wxT("mil");
    for (i = 0; i < 28; i++)
        m_layersMap[i] = 23; // default

    m_sizeX = 0;
    m_sizeY = 0;

    m_layersMap[1] = 15;
    m_layersMap[2] = 0;
    m_layersMap[3] = 27;
    m_layersMap[6] = 21;
    m_layersMap[7] = 20;
    m_timestamp_cnt = 0x10000000;
}

CPCB::~CPCB() {
    int i;

    for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
        delete m_pcbComponents[i];
    }

    for (i = 0; i < (int)m_pcbNetlist.GetCount(); i++) {
        delete m_pcbNetlist[i];
    }
}

void CPCB::WriteToFile(wxString fileName, char ftype) {
    wxFile f;
    int i;

    f.Open(fileName, wxFile::write);
    if (ftype == 'L') {
        // LIBRARY
        f.Write(wxT("PCBNEW-LibModule-V1  01/01/2001-01:01:01\n"));
        f.Write(wxT("\n"));
        f.Write(wxT("$INDEX\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if (m_pcbComponents[i]->m_objType == 'M')
                f.Write(m_pcbComponents[i]->m_name.text + wxT("\n"));
        }

        f.Write(wxT("$EndINDEX\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if (m_pcbComponents[i]->m_objType == 'M')
                m_pcbComponents[i]->WriteToFile(&f, 'L');
        }

        f.Write(wxT("$EndLIBRARY\n"));
    } // LIBRARY

    if (ftype == 'P') {
        // PCB
        f.Write(wxT("PCBNEW-BOARD Version 1 date 01/1/2000-01:01:01\n"));
        f.Write(wxT("$SHEETDESCR\n"));
        f.Write(wxString::Format("$Sheet User %d %d\n", m_sizeX, m_sizeY));
        f.Write(wxT("$EndSHEETDESCR\n"));

        // MODULES
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if (m_pcbComponents[i]->m_objType == 'M') m_pcbComponents[i]->WriteToFile(&f, 'L');
        }

        // TEXTS
        f.Write(wxT("\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if (m_pcbComponents[i]->m_objType == 'T') {
                f.Write(wxT("$TEXTPCB\n"));
                m_pcbComponents[i]->WriteToFile(&f, 'P');
                f.Write(wxT("$EndTEXTPCB\n"));
            }
        }

        // SEGMENTS
        f.Write(wxT("\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if ((m_pcbComponents[i]->m_objType == 'L' ||
                 m_pcbComponents[i]->m_objType == 'A') &&
                !(m_pcbComponents[i]->m_KiCadLayer >= 0 && m_pcbComponents[i]->m_KiCadLayer <= 15))
            {
                f.Write(wxT("$DRAWSEGMENT\n"));
                m_pcbComponents[i]->WriteToFile(&f, 'P');
                f.Write(wxT("$EndDRAWSEGMENT\n"));
            }
        }

        // TRACKS
        f.Write(wxT("\n"));
        f.Write(wxT("$TRACK\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            // LINES
            if (m_pcbComponents[i]->m_objType == 'L' &&
                (m_pcbComponents[i]->m_KiCadLayer >= 0 && m_pcbComponents[i]->m_KiCadLayer <= 15))
            {
                m_pcbComponents[i]->WriteToFile(&f, 'P');
            }

            // VIAS
            if (m_pcbComponents[i]->m_objType == 'V')
                ((CPCBVia *)m_pcbComponents[i])->WriteToFile(&f, 'P', 0);
        }

        f.Write(wxT("$EndTRACK\n"));
        // ZONES
        f.Write(wxT("\n"));
        f.Write(wxT("$ZONE\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if (m_pcbComponents[i]->m_objType == 'Z')
                m_pcbComponents[i]->WriteToFile(&f, 'P');
        }

        f.Write(wxT("$EndZONE\n"));
        for (i = 0; i < (int)m_pcbComponents.GetCount(); i++) {
            if (m_pcbComponents[i]->m_objType == 'Z')
                ((CPCBPolygon *)m_pcbComponents[i])->WriteOutlineToFile(&f, 'P');
        }

        f.Write(wxT("\n"));
        f.Write(wxT("$EndBOARD\n"));
    }

    f.Close();
}

int CPCB::GetNewTimestamp() {
    return m_timestamp_cnt++;
}
