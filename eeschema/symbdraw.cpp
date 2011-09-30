/**
 * @file symbdraw.cpp
 * @brief Create, move .. graphic shapes used to build and draw a component (lines, arcs ..)
 */

#include "fctsys.h"
#include "gr_basic.h"
#include "class_drawpanel.h"
#include "confirm.h"
#include "class_sch_screen.h"

#include "eeschema_id.h"
#include "general.h"
#include "libeditframe.h"
#include "class_libentry.h"
#include "lib_arc.h"
#include "lib_circle.h"
#include "lib_polyline.h"
#include "lib_rectangle.h"
#include "lib_text.h"

#include "dialogs/dialog_lib_edit_draw_item.h"


static void SymbolDisplayDraw( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aPosition,
                               bool aErase );
static void RedrawWhileMovingCursor( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aPosition,
                                     bool aErase );


/*
 * Show the dialog box for editing a graphical item properties
 */
void LIB_EDIT_FRAME::EditGraphicSymbol( wxDC* DC, LIB_ITEM* DrawItem )
{
    if( DrawItem == NULL )
        return;

    LIB_COMPONENT* component = DrawItem->GetParent();

    DIALOG_LIB_EDIT_DRAW_ITEM dialog( this, DrawItem->GetTypeName() );

    dialog.SetWidthUnits( ReturnUnitSymbol( g_UserUnit ) );

    wxString val = ReturnStringFromValue( g_UserUnit, m_drawLineWidth, m_InternalUnits );
    dialog.SetWidth( val );
    dialog.SetApplyToAllUnits( !m_drawSpecificUnit );
    dialog.EnableApplyToAllUnits( component && component->GetPartCount() > 1 );
    dialog.SetApplyToAllConversions( !m_drawSpecificConvert );
    dialog.EnableApplyToAllConversions( component && component->HasConversion() );
//    dialog.SetFillStyle( m_drawFillStyle );   // could better to show the current setting
    dialog.SetFillStyle( DrawItem->GetFillMode() );
    dialog.EnableFillStyle( DrawItem->IsFillable() );

    if( dialog.ShowModal() == wxID_CANCEL )
        return;

    val = dialog.GetWidth();
    m_drawLineWidth = ReturnValueFromString( g_UserUnit, val, m_InternalUnits );
    m_drawSpecificConvert = !dialog.GetApplyToAllConversions();
    m_drawSpecificUnit    = !dialog.GetApplyToAllUnits();

#if 0
    /* TODO: see if m_drawFillStyle must retain the last fill option or not.
     * if the last is Filled, having next new graphic items created
     * with filled body is often bad.
     * currently m_drawFillStyle is left with the default value (not filled)
     */
    if( DrawItem->IsFillable() )
        m_drawFillStyle = (FILL_T) dialog.GetFillStyle();
#endif

    // Save copy for undo if not in edit (edit command already handle the save copy)
    if( !DrawItem->InEditMode() )
        SaveCopyInUndoList( DrawItem->GetParent() );

    if( m_drawSpecificUnit )
        DrawItem->SetUnit( GetUnit() );
    else
        DrawItem->SetUnit( 0 );

    if( m_drawSpecificConvert )
        DrawItem->SetConvert( GetConvert() );
    else
        DrawItem->SetConvert( 0 );

    if( DrawItem->IsFillable() )
        DrawItem->SetFillMode( (FILL_T) dialog.GetFillStyle() );

    DrawItem->SetWidth( m_drawLineWidth );

    if( component )
        component->GetDrawItemList().sort();
    OnModify( );

    DrawItem->DisplayInfo( this );
    DrawPanel->Refresh();
}


static void AbortSymbolTraceOn( EDA_DRAW_PANEL* Panel, wxDC* DC )
{
    LIB_EDIT_FRAME* parent = (LIB_EDIT_FRAME*) Panel->GetParent();
    LIB_ITEM* item = parent->GetDrawItem();

    if( item == NULL )
        return;

    bool newItem = item->IsNew();
    item->EndEdit( parent->GetScreen()->GetCrossHairPosition( true ), true );

    if( newItem )
    {
        delete item;
    }
    else
        parent->RestoreComponent();

    parent->SetDrawItem( NULL );
    Panel->Refresh();
}


LIB_ITEM* LIB_EDIT_FRAME::CreateGraphicItem( LIB_COMPONENT* LibEntry, wxDC* DC )
{
    DrawPanel->SetMouseCapture( SymbolDisplayDraw, AbortSymbolTraceOn );
    wxPoint drawPos = GetScreen()->GetCrossHairPosition( true );

    // no temp copy -> the current version of component will be used for Undo
    // This is normal when adding new items to the current component
    ClearTempCopyComponent();

    switch( GetToolId() )
    {
    case ID_LIBEDIT_BODY_ARC_BUTT:
        m_drawItem = new LIB_ARC( LibEntry );
        break;

    case ID_LIBEDIT_BODY_CIRCLE_BUTT:
        m_drawItem = new LIB_CIRCLE( LibEntry );
        break;

    case ID_LIBEDIT_BODY_RECT_BUTT:
        m_drawItem = new LIB_RECTANGLE( LibEntry );
        break;

    case ID_LIBEDIT_BODY_LINE_BUTT:
        m_drawItem = new LIB_POLYLINE( LibEntry );
        break;

    case ID_LIBEDIT_BODY_TEXT_BUTT:
    {
        LIB_TEXT* Text = new LIB_TEXT( LibEntry );
        Text->m_Size.x = Text->m_Size.y = m_textSize;
        Text->m_Orient = m_textOrientation;

        // Enter the graphic text info
        DrawPanel->m_IgnoreMouseEvents = true;
        EditSymbolText( NULL, Text );
        DrawPanel->m_IgnoreMouseEvents = false;
        DrawPanel->MoveCursorToCrossHair();

        if( Text->m_Text.IsEmpty() )
        {
            delete Text;
            m_drawItem = NULL;
        }
        else
        {
            m_drawItem = Text;
        }
        break;
    }
    default:
        DisplayError( this, wxT( "LIB_EDIT_FRAME::CreateGraphicItem() error" ) );
        return NULL;
    }

    if( m_drawItem )
    {
        m_drawItem->BeginEdit( IS_NEW, drawPos );
        m_drawItem->SetWidth( m_drawLineWidth );
        m_drawItem->SetFillMode( m_drawFillStyle );

        if( m_drawSpecificUnit )
            m_drawItem->SetUnit( m_unit );
        if( m_drawSpecificConvert )
            m_drawItem->SetConvert( m_convert );

        // Draw initial symbol:
        DrawPanel->m_mouseCaptureCallback( DrawPanel, DC, wxDefaultPosition, false );
    }
    else
    {
        DrawPanel->EndMouseCapture();
        return NULL;
    }

    DrawPanel->MoveCursorToCrossHair();
    DrawPanel->m_IgnoreMouseEvents = FALSE;

    return m_drawItem;
}


/* Create new library component graphic object.
 */
void LIB_EDIT_FRAME::GraphicItemBeginDraw( wxDC* DC )
{
    if( m_drawItem == NULL )
        return;

    wxPoint pos = GetScreen()->GetCrossHairPosition( true );

    if( m_drawItem->ContinueEdit( pos ) )
    {
        m_drawItem->Draw( DrawPanel, DC, pos, -1, g_XorMode, NULL, DefaultTransform );
        return;
    }

    EndDrawGraphicItem( DC );
}


/*
 * Redraw the graphic shape while moving
 */
static void RedrawWhileMovingCursor( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aPosition,
                                     bool aErase )
{
    LIB_ITEM* item;

    item = ( (LIB_EDIT_FRAME*) aPanel->GetParent() )->GetDrawItem();

    if( item == NULL )
        return;

    BASE_SCREEN* Screen = aPanel->GetScreen();

    item->SetEraseLastDrawItem( aErase );

    // if item is the reference field, we must add the current unit id
    if( item->Type() == LIB_FIELD_T )
    {
        int unit = ((LIB_EDIT_FRAME*)aPanel->GetParent())->GetUnit();
        wxString text = ((LIB_FIELD*)item)->GetFullText( unit );
        item->Draw( aPanel, aDC, Screen->GetCrossHairPosition( true ), -1, g_XorMode, &text,
                    DefaultTransform );
    }
    else
        item->Draw( aPanel, aDC, Screen->GetCrossHairPosition( true ), -1, g_XorMode, NULL,
                    DefaultTransform );
}


void LIB_EDIT_FRAME::StartMoveDrawSymbol( wxDC* DC )
{
    if( m_drawItem == NULL )
        return;

    SetCursor( wxCURSOR_HAND );

    TempCopyComponent();
    m_drawItem->BeginEdit( IS_MOVED, GetScreen()->GetCrossHairPosition( true ) );
    DrawPanel->SetMouseCapture( RedrawWhileMovingCursor, AbortSymbolTraceOn );
    DrawPanel->m_mouseCaptureCallback( DrawPanel, DC, wxDefaultPosition, true );
}


// @brief Modify a graphic symbol (drag edges etc.)
void LIB_EDIT_FRAME::StartModifyDrawSymbol( wxDC* DC )
{
    if( m_drawItem == NULL )
        return;

    TempCopyComponent();
    m_drawItem->BeginEdit( IS_RESIZED, GetScreen()->GetCrossHairPosition( true ) );
    DrawPanel->SetMouseCapture( SymbolDisplayDraw, AbortSymbolTraceOn );
    DrawPanel->m_mouseCaptureCallback( DrawPanel, DC, wxDefaultPosition, true );
}


//! @brief Manage mouse events when creating new graphic object or modifying an graphic object.
static void SymbolDisplayDraw( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aPosition,
                               bool aErase )
{
    BASE_SCREEN*   Screen   = aPanel->GetScreen();
    LIB_ITEM* item = ( (LIB_EDIT_FRAME*) aPanel->GetParent() )->GetDrawItem();

    if( item == NULL )
        return;

    item->SetEraseLastDrawItem( aErase );
    item->Draw( aPanel, aDC, Screen->GetCrossHairPosition( true ), -1, g_XorMode, NULL,
                DefaultTransform );
}


/*
 * Place the new graphic object in the list of component drawing objects,
 * or terminate a draw item edition
 */
void LIB_EDIT_FRAME::EndDrawGraphicItem( wxDC* DC )
{
    if( m_component == NULL || m_drawItem == NULL )
        return;

    if( GetToolId() != ID_NO_TOOL_SELECTED )
        SetCursor( wxCURSOR_PENCIL );
    else
        SetCursor( DrawPanel->GetDefaultCursor() );

    if( GetTempCopyComponent() )    // used when editing an existing item
        SaveCopyInUndoList( GetTempCopyComponent() );
    else    // When creating a new item, there is still no change for the current component
            // So save it.
        SaveCopyInUndoList( m_component );

    if( m_drawItem->IsNew() )
        m_component->AddDrawItem( m_drawItem );

    m_drawItem->EndEdit( GetScreen()->GetCrossHairPosition( true ) );

    m_drawItem = NULL;

    OnModify();

    DrawPanel->SetMouseCapture( NULL, NULL );
    DrawPanel->Refresh();
}
