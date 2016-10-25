/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Maciej Suminski <maciej.suminski@cern.ch>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * https://www.gnu.org/licenses/gpl-3.0.html
 * or you may search the http://www.gnu.org website for the version 3 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include "netlist_exporter_pspice_sim.h"

wxString NETLIST_EXPORTER_PSPICE_SIM::GetSpiceVector( const wxString& aName, SIM_PLOT_TYPE aType,
        const wxString& aParam ) const
{
    wxString res;

    // Some of the flags should exclude mutually
    assert( ( ( aType & SPT_VOLTAGE ) == 0 ) != ( ( aType & SPT_CURRENT ) == 0 ) );
    assert( ( ( aType & SPT_AC_PHASE ) == 0 ) || ( ( aType & SPT_AC_MAG ) == 0 ) );

    if( aType & SPT_VOLTAGE )
    {
        // Spice netlist netnames does not accept some chars, whicyh are replaced
        // by eeschema netlist generator.
        // Replace these forbidden chars to find the actual spice net name
        wxString spicenet = aName;
        NETLIST_EXPORTER_PSPICE::ReplaceForbiddenChars( spicenet );

        return wxString::Format( "V(%s)", spicenet.GetData() );
    }

    else if( aType & SPT_CURRENT )
    {
        return wxString::Format( "@%s[%s]", GetSpiceDevice( aName ).Lower(),
                aParam.IsEmpty() ? "i" : aParam.Lower() );
    }

    return res;
}


const std::vector<wxString>& NETLIST_EXPORTER_PSPICE_SIM::GetCurrents( SPICE_PRIMITIVE aPrimitive )
{
    static const std::vector<wxString> passive = { "I" };
    static const std::vector<wxString> diode = { "Id" };
    static const std::vector<wxString> bjt = { "Ib", "Ic", "Ie" };
    static const std::vector<wxString> mos = { "Ig", "Id", "Is" };
    static const std::vector<wxString> empty;

    switch( aPrimitive )
    {
        case SP_RESISTOR:
        case SP_CAPACITOR:
        case SP_INDUCTOR:
        case SP_VSOURCE:
            return passive;

        case SP_DIODE:
            return diode;

        case SP_BJT:
            return bjt;

        case SP_MOSFET:
            return mos;

        default:
            return empty;
    }
}


wxString NETLIST_EXPORTER_PSPICE_SIM::GetSpiceDevice( const wxString& aComponent ) const
{
    const auto& spiceItems = GetSpiceItems();

    auto it = std::find_if( spiceItems.begin(), spiceItems.end(), [&]( const SPICE_ITEM& item ) {
        return item.m_refName == aComponent;
    } );

    if( it == spiceItems.end() )
        return wxEmptyString;

    return wxString( it->m_primitive + it->m_refName );
}


wxString NETLIST_EXPORTER_PSPICE_SIM::GetSheetSimCommand()
{
    wxString simCmd;

    UpdateDirectives( NET_ALL_FLAGS );

    for( const auto& dir : GetDirectives() )
    {
        if( IsSimCommand( dir ) )
            simCmd += wxString::Format( "%s\r\n", dir );
    }

    return simCmd;
}


SIM_TYPE NETLIST_EXPORTER_PSPICE_SIM::GetSimType()
{
    return CommandToSimType( m_simCommand.IsEmpty() ? GetSheetSimCommand() : m_simCommand );
}


SIM_TYPE NETLIST_EXPORTER_PSPICE_SIM::CommandToSimType( const wxString& aCmd )
{
    const std::map<wxString, SIM_TYPE> simCmds = {
        { ".ac", ST_AC }, { ".dc", ST_DC }, { ".disto", ST_DISTORTION }, { ".noise", ST_NOISE },
        { ".op", ST_OP }, { ".pz", ST_POLE_ZERO }, { ".sens", ST_SENSITIVITY }, { ".tf", ST_TRANS_FUNC },
        { ".tran", ST_TRANSIENT }
    };
    wxString lcaseCmd = aCmd.Lower();

    for( const auto& c : simCmds )
    {
        if( lcaseCmd.StartsWith( c.first ) )
            return c.second;
    }

    return ST_UNKNOWN;
}


void NETLIST_EXPORTER_PSPICE_SIM::writeDirectives( OUTPUTFORMATTER* aFormatter, unsigned aCtl ) const
{
    // Add a directive to obtain currents
    //aFormatter->Print( 0, ".options savecurrents\n" );        // does not work :(

    for( const auto& item : GetSpiceItems() )
    {
        for( const auto& current :
                NETLIST_EXPORTER_PSPICE_SIM::GetCurrents( (SPICE_PRIMITIVE) item.m_primitive ) )
        {
            if( !item.m_enabled )
                continue;

            /// @todo is it required to switch to lowercase
            aFormatter->Print( 0, ".save %s\n",
                    (const char*) GetSpiceVector( item.m_refName, SPT_CURRENT, current ).c_str() );
        }
    }

    // If we print out .save directives for currents, then it needs to be done for voltages as well
    for( const auto& netMap : GetNetIndexMap() )
    {
        aFormatter->Print( 0, ".save %s\n",
                (const char*) GetSpiceVector( netMap.first, SPT_VOLTAGE ).c_str() );
    }

    if( m_simCommand.IsEmpty() )
    {
        // Fallback to the default behavior and just write all directives
        NETLIST_EXPORTER_PSPICE::writeDirectives( aFormatter, aCtl );
    }
    else
    {
        // Dump all directives but simulation commands
        for( const auto& dir : GetDirectives() )
        {
            if( !IsSimCommand( dir ) )
                aFormatter->Print( 0, "%s\n", (const char*) dir.c_str() );
        }

        // Finish with our custom simulation command
        aFormatter->Print( 0, "%s\n", (const char*) m_simCommand.c_str() );
    }
}
