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

#ifndef __SIM_PLOT_PANEL_H
#define __SIM_PLOT_PANEL_H

#include <widgets/mathplot.h>
#include <map>
#include "sim_types.h"

class TRACE;

///> Cursor attached to a trace to follow its values:
class CURSOR : public mpInfoLayer
{
public:
    CURSOR( const TRACE* aTrace )
        : mpInfoLayer( wxRect( 0, 0, DRAG_MARGIN, DRAG_MARGIN ), wxTRANSPARENT_BRUSH ),
        m_trace( aTrace ), m_updateRequired( true ), m_updateRef( false ),
        m_coords( 0.0, 0.0 ), m_window( nullptr )
    {
        SetDrawOutsideMargins( false );
    }

    void Plot( wxDC& aDC, mpWindow& aWindow ) override;

    void SetX( int aX )
    {
        m_reference.x = 0;
        m_updateRef = true;
        Move( wxPoint( aX, 0 ) );
    }

    void Update()
    {
        m_updateRequired = true;
    }

    bool Inside( wxPoint& aPoint ) override;

    void Move( wxPoint aDelta ) override
    {
        Update();
        mpInfoLayer::Move( aDelta );
    }

    void UpdateReference() override;

    const wxRealPoint& GetCoords() const
    {
        return m_coords;
    }

private:
    const TRACE* m_trace;
    bool m_updateRequired, m_updateRef;
    wxRealPoint m_coords;
    mpWindow* m_window;

    const int DRAG_MARGIN = 10;
};


class TRACE : public mpFXYVector
{
public:
    TRACE( const wxString& aName ) :
        mpFXYVector( aName ), m_cursor( nullptr ), m_flags( 0 )
    {
        SetContinuity( true );
        SetDrawOutsideMargins( false );
        ShowName( false );
    }

    /**
     * @brief Assigns new data set for the trace. aX and aY need to have the same length.
     * @param aX are the X axis values.
     * @param aY are the Y axis values.
     */
    void SetData( const std::vector<double>& aX, const std::vector<double>& aY ) override
    {
        if( m_cursor )
            m_cursor->Update();

        mpFXYVector::SetData( aX, aY );
    }

    const std::vector<double>& GetDataX() const
    {
        return m_xs;
    }

    const std::vector<double>& GetDataY() const
    {
        return m_ys;
    }

    bool HasCursor() const
    {
        return m_cursor != nullptr;
    }

    void SetCursor( CURSOR* aCursor )
    {
        m_cursor = aCursor;
    }

    CURSOR* GetCursor() const
    {
        return m_cursor;
    }

    void SetFlags( int aFlags )
    {
        m_flags = aFlags;
    }

    int GetFlags() const
    {
        return m_flags;
    }

    void SetTraceColour( wxColour aColour )
    {
        m_traceColour = aColour;
    }

    wxColour GetTraceColour()
    {
        return m_traceColour;
    }

protected:
    CURSOR* m_cursor;
    int m_flags;
    wxColour m_traceColour;
};


class SIM_PLOT_PANEL : public mpWindow
{
public:
    SIM_PLOT_PANEL( SIM_TYPE aType, wxWindow* parent, wxWindowID id, const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize, long style = 0, const wxString& name = wxPanelNameStr );

    ~SIM_PLOT_PANEL();

    SIM_TYPE GetType() const
    {
        return m_type;
    }

    static bool IsPlottable( SIM_TYPE aSimType );

    wxString GetLabelX() const
    {
        return m_axis_x ? m_axis_x->GetName() : "";
    }

    wxString GetLabelY1() const
    {
        return m_axis_y1 ? m_axis_y1->GetName() : "";
    }

    wxString GetLabelY2() const
    {
        return m_axis_y2 ? m_axis_y2->GetName() : "";
    }

    bool AddTrace( const wxString& aName, int aPoints,
            const double* aX, const double* aY, SIM_PLOT_TYPE aFlags );

    bool DeleteTrace( const wxString& aName );

    void DeleteAllTraces();

    bool IsShown( const wxString& aName ) const
    {
        return m_traces.count( aName ) > 0;
    }

    const std::map<wxString, TRACE*>& GetTraces() const
    {
        return m_traces;
    }

    TRACE* GetTrace( const wxString& aName ) const
    {
        auto trace = m_traces.find( aName );

        return trace == m_traces.end() ? NULL : trace->second;
    }

    void ShowGrid( bool aEnable )
    {
        m_axis_x->SetTicks( !aEnable );
        m_axis_y1->SetTicks( !aEnable );
        m_axis_y2->SetTicks( !aEnable );
        UpdateAll();
    }

    bool IsGridShown() const
    {
        assert( m_axis_x->GetTicks() == m_axis_y1->GetTicks() );
        return !m_axis_x->GetTicks();
    }

    void ShowLegend( bool aEnable )
    {
        m_legend->SetVisible( aEnable );
        UpdateAll();
    }

    bool IsLegendShown() const
    {
        return m_legend->IsVisible();
    }

    ///> Returns true if the trace has cursor shown.
    bool HasCursorEnabled( const wxString& aName ) const;

    ///> Toggles cursor for a particular trace.
    void EnableCursor( const wxString& aName, bool aEnable );

    ///> Resets scale ranges to fit the current traces
    void ResetScales();

private:
    ///> Returns a new color from the palette
    wxColour generateColor();

    // Color index to get a new color from the palette
    unsigned int m_colorIdx;

    // Traces to be plotted
    std::map<wxString, TRACE*> m_traces;

    mpScaleXBase* m_axis_x;
    mpScaleY* m_axis_y1;
    mpScaleY* m_axis_y2;
    mpInfoLegend* m_legend;

    std::vector<mpLayer*> m_topLevel;

    const SIM_TYPE m_type;
};

wxDECLARE_EVENT( EVT_SIM_CURSOR_UPDATE, wxCommandEvent );

#endif
