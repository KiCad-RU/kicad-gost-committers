/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2017 Eldar Khayrullin <eldar.khayrullin@mail.ru>
 * Copyright (C) 2017 KiCad Developers, see CHANGELOG.TXT for contributors.
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
 * @file sch_ieeesymbol.h
 */

#ifndef SCH_IEEESYMBOL_H_
#define SCH_IEEESYMBOL_H_

#include <wx/wx.h>

#include <sch_component.h>
#include <pcad2kicad_common.h>

namespace PCAD2KICAD {

enum IEEESymbolType
{
    None,
    Adder,
    Amplifier,
    Astable,
    Complex,
    Generator,
    Hysteresis,
    Multiplier
};


class SCH_IEEESYMBOL : public SCH_COMPONENT
{
public:
    SCH_IEEESYMBOL();
    ~SCH_IEEESYMBOL();

    virtual void    Parse( XNODE* aNode, int aSymbolIndex, wxString aDefaultMeasurementUnit,
                           wxString aActualConversion );
    virtual void    WriteToFile( wxFile* aFile, char aFileType );

private:
    typedef wxPoint2DInt Point;
    typedef std::vector<Point> Points;

    const Point BREAK_POINT = Point( INT_MAX, INT_MAX );

    IEEESymbolType m_type;
    int m_height;

    void AppendAdderPoints( Points& aPoints );
    void AppendAmplifierPoints( Points& aPoints );
    void AppendAstablePoints( Points& aPoints );
    void AppendComplexPoints( Points& aPoints );
    void AppendGeneratorPoints( Points& aPoints );
    void AppendHysteresisPoints( Points& aPoints );
    void AppendMultiplierPoints( Points& aPoints );
    void TransformPoints( Points& aPoints );
    void WriteComplexCircleToFile( wxFile* aFile );
    void WritePointsToFile( wxFile* aFile, const Points& aPoints );
};

} // namespace PCAD2KICAD

#endif    // SCH_IEEESYMBOL_H_
