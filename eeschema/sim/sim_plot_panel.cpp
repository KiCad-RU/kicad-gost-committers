/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 CERN
 * @author Tomasz Wlostowski <tomasz.wlostowski@cern.ch>
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

#include "sim_plot_panel.h"

#include <algorithm>
#include <limits>

static wxString formatFloat( double x, int nDigits )
{
    wxString rv, fmt;

    if( nDigits )
    {
        fmt = wxT( "%.0Nf" );
        fmt[3] = '0' + nDigits;
    }
    else
    {
        fmt = wxT( "%.0f" );
    }

    rv.Printf( fmt, x );

    return rv;
}


static void getSISuffix( double x, const wxString& unit, int& power, wxString& suffix )
{
    const int n_powers = 11;

    const struct
    {
        double exponent;
        char suffix;
    } powers[] =
    {
        { -18, 'a' },
        { -15, 'f' },
        { -12, 'p' },
        { -9,  'n' },
        { -6,  'u' },
        { -3,  'm' },
        { 0,   0   },
        { 3,   'k' },
        { 6,   'M' },
        { 9,   'G' },
        { 12,  'T' },
        { 14,  'P' }
    };

    power = 0;
    suffix = unit;

    if( x == 0.0 )
        return;

    for( int i = 0; i < n_powers - 1; i++ )
    {
        double r_cur = pow( 10, powers[i].exponent );

        if( fabs( x ) >= r_cur && fabs( x ) < r_cur * 1000.0 )
        {
            power = powers[i].exponent;

            if( powers[i].suffix )
                suffix = wxString( powers[i].suffix ) + unit;
            else
                suffix = unit;

            return;
        }
    }
}


static int countDecimalDigits( double x, int maxDigits )
{
    int64_t k = (int)( ( x - floor( x ) ) * pow( 10.0, (double) maxDigits ) );
    int n = 0;

    while( k && ( ( k % 10LL ) == 0LL || ( k % 10LL ) == 9LL ) )
    {
        k /= 10LL;
    }

    n = 0;

    while( k != 0LL )
    {
        n++;
        k /= 10LL;
    }

    return n;
}


static void formatSILabels( mpScaleBase* scale, const wxString& aUnit, int nDigits )
{
    double maxVis = scale->AbsVisibleMaxValue();

    wxString suffix;
    int power, digits = 0;

    getSISuffix( maxVis, aUnit, power, suffix );

    double sf = pow( 10.0, power );

    for( auto &l : scale->TickLabels() )
    {
        int k = countDecimalDigits( l.pos / sf, nDigits );

        digits = std::max( digits, k );
    }

    for( auto &l : scale->TickLabels() )
    {
        l.label = formatFloat ( l.pos / sf, digits ) + suffix;
        l.visible = true;
    }
}


class FREQUENCY_LOG_SCALE : public mpScaleXLog
{
public:
    FREQUENCY_LOG_SCALE( wxString name, int flags ) :
        mpScaleXLog( name, flags ) {};

    void formatLabels() override
    {
        const wxString unit = wxT( "Hz" );
        wxString suffix;
        int power;

        for( auto &l : TickLabels() )
        {
            getSISuffix( l.pos, unit, power, suffix );
            double sf = pow( 10.0, power );
            int k = countDecimalDigits( l.pos / sf, 3 );

            l.label = formatFloat( l.pos / sf, k ) + suffix;
            l.visible = true;
        }
    }
};


class FREQUENCY_LIN_SCALE : public mpScaleX
{
public:
    FREQUENCY_LIN_SCALE( wxString name, int flags ) :
        mpScaleX( name, flags, false , 0 ) {};

    void formatLabels() override
    {
        formatSILabels( this, wxT( "Hz" ), 3 );
    }
};


class TIME_SCALE : public mpScaleX
{
public:
    TIME_SCALE( wxString name, int flags ) :
        mpScaleX( name, flags, false, 0 ) {};

    void formatLabels() override
    {
        formatSILabels( this, wxT( "s" ), 3 );
    }
};


class VOLTAGE_SCALE_X : public mpScaleX
{
public:
    VOLTAGE_SCALE_X( wxString name, int flags ) :
        mpScaleX( name, flags, false, 0 ) {};

    void formatLabels() override
    {
        formatSILabels( this, wxT( "V" ), 3 );
    }
};


class GAIN_SCALE : public mpScaleY
{
public:
    GAIN_SCALE( wxString name, int flags ) :
        mpScaleY( name, flags, false ) {};

    void formatLabels() override
    {
        formatSILabels( this, wxT( "dB" ), 3 );
    }

};


class PHASE_SCALE : public mpScaleY
{
public:
    PHASE_SCALE( wxString name, int flags ) :
        mpScaleY( name, flags, false ) {};

    void formatLabels() override
    {
        formatSILabels( this, wxT( "\u00B0" ), 3 );     // degree sign
    }
};


class VOLTAGE_SCALE_Y : public mpScaleY
{
public:
    VOLTAGE_SCALE_Y( wxString name, int flags ) :
        mpScaleY( name, flags, false ) {};

    void formatLabels() override
    {
        formatSILabels( this, wxT( "V" ), 3 );
    }
};


class CURRENT_SCALE : public mpScaleY
{
public:
    CURRENT_SCALE( wxString name, int flags ) :
        mpScaleY( name, flags, false ) {};

    void formatLabels() override
    {
        formatSILabels( this, wxT( "A" ), 3 );
    }
};


void CURSOR::Plot( wxDC& aDC, mpWindow& aWindow )
{
    if( !m_window )
        m_window = &aWindow;

    if( !m_visible )
        return;

    const auto& dataX = m_trace->GetDataX();
    const auto& dataY = m_trace->GetDataY();

    if( dataX.size() <= 1 )
        return;

    if( m_updateRequired )
    {
        m_coords.x = m_trace->s2x( aWindow.p2x( m_dim.x ) );

        // Find the closest point coordinates
        auto maxXIt = std::upper_bound( dataX.begin(), dataX.end(), m_coords.x );
        int maxIdx = maxXIt - dataX.begin();
        int minIdx = maxIdx - 1;

        // Out of bounds checks
        if( minIdx < 0 )
        {
            minIdx = 0;
            maxIdx = 1;
            m_coords.x = dataX[0];
        }
        else if( maxIdx >= (int) dataX.size() )
        {
            maxIdx = dataX.size() - 1;
            minIdx = maxIdx - 1;
            m_coords.x = dataX[maxIdx];
        }

        const double leftX = dataX[minIdx];
        const double rightX = dataX[maxIdx];
        const double leftY = dataY[minIdx];
        const double rightY = dataY[maxIdx];

        // Linear interpolation
        m_coords.y = leftY + ( rightY - leftY ) / ( rightX - leftX ) * ( m_coords.x - leftX );
        m_updateRequired = false;

        // Notify the parent window about the changes
        wxQueueEvent( aWindow.GetParent(), new wxCommandEvent( EVT_SIM_CURSOR_UPDATE ) );
    }
    else
    {
        m_updateRef = true;
    }

    if( m_updateRef )
    {
        UpdateReference();
        m_updateRef = false;
    }

    // Line length in horizontal and vertical dimensions
    const wxPoint cursorPos( aWindow.x2p( m_trace->x2s( m_coords.x ) ),
                             aWindow.y2p( m_trace->y2s( m_coords.y ) ) );

    wxCoord leftPx   = m_drawOutsideMargins ? 0 : aWindow.GetMarginLeft();
    wxCoord rightPx  = m_drawOutsideMargins ? aWindow.GetScrX() : aWindow.GetScrX() - aWindow.GetMarginRight();
    wxCoord topPx    = m_drawOutsideMargins ? 0 : aWindow.GetMarginTop();
    wxCoord bottomPx = m_drawOutsideMargins ? aWindow.GetScrY() : aWindow.GetScrY() - aWindow.GetMarginBottom();

    aDC.SetPen( wxPen( *wxWHITE, 1, m_continuous ? wxPENSTYLE_SOLID : wxPENSTYLE_LONG_DASH ) );

    if( topPx < cursorPos.y && cursorPos.y < bottomPx )
        aDC.DrawLine( leftPx, cursorPos.y, rightPx, cursorPos.y );

    if( leftPx < cursorPos.x && cursorPos.x < rightPx )
        aDC.DrawLine( cursorPos.x, topPx, cursorPos.x, bottomPx );
}


bool CURSOR::Inside( wxPoint& aPoint )
{
    if( !m_window )
        return false;

    return ( std::abs( (double) aPoint.x - m_window->x2p( m_trace->x2s( m_coords.x ) ) ) <= DRAG_MARGIN )
        || ( std::abs( (double) aPoint.y - m_window->y2p( m_trace->y2s( m_coords.y ) ) ) <= DRAG_MARGIN );
}


void CURSOR::UpdateReference()
{
    if( !m_window )
        return;

    m_reference.x = m_window->x2p( m_trace->x2s( m_coords.x ) );
    m_reference.y = m_window->y2p( m_trace->y2s( m_coords.y ) );
}


SIM_PLOT_PANEL::SIM_PLOT_PANEL( SIM_TYPE aType, wxWindow* parent, wxWindowID id, const wxPoint& pos,
                const wxSize& size, long style, const wxString& name )
    : mpWindow( parent, id, pos, size, style ), m_colorIdx( 0 ),
        m_axis_x( nullptr ), m_axis_y1( nullptr ), m_axis_y2( nullptr ), m_type( aType )
{
    LimitView( true );
    SetMargins( 50, 80, 50, 80 );

    wxColour grey( 96, 96, 96 );
    SetColourTheme( *wxBLACK, *wxWHITE, grey );
    EnableDoubleBuffer( true );
    UpdateAll();

    switch( m_type )
    {
        case ST_AC:
            m_axis_x = new FREQUENCY_LOG_SCALE( wxT( "Frequency" ), mpALIGN_BOTTOM );
            m_axis_y1 = new GAIN_SCALE( wxT( "Gain" ), mpALIGN_LEFT );
            m_axis_y2 = new PHASE_SCALE( wxT( "Phase" ), mpALIGN_RIGHT );
            m_axis_y2->SetMasterScale( m_axis_y1 );
            break;

        case ST_DC:
            m_axis_x = new VOLTAGE_SCALE_X( wxT( "Voltage (sweeped)" ), mpALIGN_BOTTOM );
            m_axis_y1 = new VOLTAGE_SCALE_Y( wxT( "Voltage (measured)" ), mpALIGN_LEFT );
            break;

        case ST_NOISE:
            m_axis_x = new FREQUENCY_LOG_SCALE( wxT( "Frequency" ), mpALIGN_BOTTOM );
            m_axis_y1 = new mpScaleY( wxT( "noise [(V or A)^2/Hz]" ), mpALIGN_LEFT );
            break;

        case ST_TRANSIENT:
            m_axis_x = new TIME_SCALE( wxT( "Time" ), mpALIGN_BOTTOM );
            m_axis_y1 = new VOLTAGE_SCALE_Y( wxT( "Voltage" ), mpALIGN_LEFT );
            m_axis_y2 = new CURRENT_SCALE( wxT( "Current" ), mpALIGN_RIGHT );
            m_axis_y2->SetMasterScale( m_axis_y1 );
            break;

        default:
            // suppress warnings
            break;
    }

    if( m_axis_x )
    {
        m_axis_x->SetTicks( false );
        m_axis_x->SetNameAlign ( mpALIGN_BOTTOM );

        AddLayer( m_axis_x );
    }

    if( m_axis_y1 )
    {
        m_axis_y1->SetTicks( false );
        m_axis_y1->SetNameAlign ( mpALIGN_LEFT );
        AddLayer( m_axis_y1 );
    }

    if( m_axis_y2 )
    {
        m_axis_y2->SetTicks( false );
        m_axis_y2->SetNameAlign ( mpALIGN_RIGHT );
        AddLayer( m_axis_y2 );
    }

    m_legend = new mpInfoLegend( wxRect( 0, 40, 200, 40 ), wxTRANSPARENT_BRUSH );
    m_legend->SetVisible( false );
    AddLayer( m_legend );
    m_topLevel.push_back( m_legend );
    SetColourTheme( *wxBLACK, *wxWHITE, grey );

    EnableDoubleBuffer( true );
    UpdateAll();
}


SIM_PLOT_PANEL::~SIM_PLOT_PANEL()
{
    // ~mpWindow destroys all the added layers, so there is no need to destroy m_traces contents
}


bool SIM_PLOT_PANEL::IsPlottable( SIM_TYPE aSimType )
{
    switch( aSimType )
    {
        case ST_AC:
        case ST_DC:
        case ST_TRANSIENT:
            return true;

        default:
            return false;
    }
}


bool SIM_PLOT_PANEL::AddTrace( const wxString& aName, int aPoints,
        const double* aX, const double* aY, SIM_PLOT_TYPE aFlags )
{
    TRACE* trace = NULL;

    // Find previous entry, if there is one
    auto prev = m_traces.find( aName );
    bool addedNewEntry = ( prev == m_traces.end() );

    if( addedNewEntry )
    {
        if( m_type == ST_TRANSIENT )
        {
            bool hasVoltageTraces = false;

            for( auto tr : m_traces )
            {
                if( !( tr.second->GetFlags() & SPT_CURRENT ) )
                {
                    hasVoltageTraces = true;
                    break;
                }
            }

            if( !hasVoltageTraces )
                m_axis_y2->SetMasterScale( nullptr );
            else
                m_axis_y2->SetMasterScale( m_axis_y1 );
        }

        // New entry
        trace = new TRACE( aName );
        trace->SetTraceColour( generateColor() );
        trace->SetPen( wxPen( trace->GetTraceColour(), 2, wxPENSTYLE_SOLID ) );
        m_traces[aName] = trace;

        // It is a trick to keep legend & coords always on the top
        for( mpLayer* l : m_topLevel )
            DelLayer( l );

        AddLayer( (mpLayer*) trace );

        for( mpLayer* l : m_topLevel )
            AddLayer( l );
    }
    else
    {
        trace = prev->second;
    }

    std::vector<double> tmp( aY, aY + aPoints );

    if( m_type == ST_AC )
    {
        if( aFlags & SPT_AC_PHASE )
        {
            for( int i = 0; i < aPoints; i++ )
                tmp[i] = tmp[i] * 180.0 / M_PI;                 // convert to degrees
        }
        else
        {
            for( int i = 0; i < aPoints; i++ )
                tmp[i] = 20 * log( tmp[i] ) / log( 10.0 );      // convert to dB
        }
    }

    trace->SetData( std::vector<double>( aX, aX + aPoints ), tmp );

    if( aFlags & SPT_AC_PHASE || aFlags & SPT_CURRENT )
        trace->SetScale( m_axis_x, m_axis_y2 );
    else
        trace->SetScale( m_axis_x, m_axis_y1 );

    trace->SetFlags( aFlags );

    UpdateAll();

    return addedNewEntry;
}


bool SIM_PLOT_PANEL::DeleteTrace( const wxString& aName )
{
    auto it = m_traces.find( aName );

    if( it != m_traces.end() )
    {
        m_traces.erase( it );
        TRACE* trace = it->second;

        if( CURSOR* cursor = trace->GetCursor() )
            DelLayer( cursor, true );

        DelLayer( trace, true, true );
        ResetScales();

        return true;
    }

    return false;
}


void SIM_PLOT_PANEL::DeleteAllTraces()
{
    for( auto& t : m_traces )
    {
        DeleteTrace( t.first );
    }

    m_colorIdx = 0;
    m_traces.clear();
}


bool SIM_PLOT_PANEL::HasCursorEnabled( const wxString& aName ) const
{
    TRACE* t = GetTrace( aName );

    return t ? t->HasCursor() : false;
}


void SIM_PLOT_PANEL::EnableCursor( const wxString& aName, bool aEnable )
{
    TRACE* t = GetTrace( aName );

    if( t == nullptr || t->HasCursor() == aEnable )
        return;

    if( aEnable )
    {
        CURSOR* c = new CURSOR( t );
        int plotCenter = GetMarginLeft() + ( GetXScreen() - GetMarginLeft() - GetMarginRight() ) / 2;
        c->SetX( plotCenter );
        t->SetCursor( c );
        AddLayer( c );
    }
    else
    {
        CURSOR* c = t->GetCursor();
        t->SetCursor( NULL );
        DelLayer( c, true );
    }

    // Notify the parent window about the changes
    wxQueueEvent( GetParent(), new wxCommandEvent( EVT_SIM_CURSOR_UPDATE ) );
}


void SIM_PLOT_PANEL::ResetScales()
{
    if( m_axis_x )
        m_axis_x->ResetDataRange();

    if( m_axis_y1 )
        m_axis_y1->ResetDataRange();

    if( m_axis_y2 )
        m_axis_y2->ResetDataRange();

    for( auto t : m_traces )
        t.second->UpdateScales();
}


wxColour SIM_PLOT_PANEL::generateColor()
{
    /// @todo have a look at:
    /// http://stanford.edu/~mwaskom/software/seaborn/tutorial/color_palettes.html
    /// https://github.com/Gnuplotting/gnuplot-palettes

    const unsigned long colors[] = { 0x0000ff, 0x00ff00, 0xff0000, 0x00ffff, 0xff00ff, 0xffff000, 0xffffff };

    //const unsigned long colors[] = { 0xe3cea6, 0xb4781f, 0x8adfb2, 0x2ca033, 0x999afb, 0x1c1ae3, 0x6fbffd, 0x007fff, 0xd6b2ca, 0x9a3d6a };

    // hls
    //const unsigned long colors[] = { 0x0f1689, 0x0f7289, 0x35890f, 0x0f8945, 0x89260f, 0x890f53, 0x89820f, 0x630f89 };

    // pastels, good for dark background
    //const unsigned long colors[] = { 0x2fd8fe, 0x628dfa, 0x53d8a6, 0xa5c266, 0xb3b3b3, 0x94c3e4, 0xca9f8d, 0xac680e };

    const unsigned int colorCount = sizeof(colors) / sizeof(unsigned long);

    /// @todo generate shades to avoid repeating colors
    return wxColour( colors[m_colorIdx++ % colorCount] );
}

wxDEFINE_EVENT( EVT_SIM_CURSOR_UPDATE, wxCommandEvent );
