/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2004 Jean-Pierre Charras, jaen-pierre.charras@gipsa-lab.inpg.com
 * Copyright (C) 2008-2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004-2011 KiCad Developers, see change_log.txt for contributors.
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
 * @file lib_field.cpp
 */

#include "fctsys.h"
#include "appl_wxstruct.h"
#include "gr_basic.h"
#include "macros.h"
#include "base_struct.h"
#include "drawtxt.h"
#include "kicad_string.h"
#include "class_drawpanel.h"
#include "plot_common.h"
#include "trigo.h"

#include "general.h"
#include "protos.h"
#include "class_libentry.h"
#include "transform.h"
#include "lib_field.h"
#include "template_fieldnames.h"


LIB_FIELD::LIB_FIELD(LIB_COMPONENT * aParent, int idfield ) :
    LIB_ITEM( LIB_FIELD_T, aParent )
{
    Init( idfield );
}


LIB_FIELD::LIB_FIELD( int idfield ) : LIB_ITEM( LIB_FIELD_T, NULL )
{
    Init( idfield );
}


LIB_FIELD::LIB_FIELD( const LIB_FIELD& field ) : LIB_ITEM( field )
{
    m_id        = field.m_id;
    m_Pos       = field.m_Pos;
    m_Size      = field.m_Size;
    m_Thickness = field.m_Thickness;
    m_Orient    = field.m_Orient;
    m_Attributs = field.m_Attributs;
    m_Text      = field.m_Text;
    m_name      = field.m_name;
    m_HJustify  = field.m_HJustify;
    m_VJustify  = field.m_VJustify;
    m_Italic    = field.m_Italic;
    m_Bold      = field.m_Bold;
}


LIB_FIELD::~LIB_FIELD()
{
}


void LIB_FIELD::Init( int id )
{
    m_id = id;
    m_Size.x = m_Size.y = DEFAULT_SIZE_TEXT;
    m_typeName = _( "Field" );
    m_Orient = TEXT_ORIENT_HORIZ;
    m_rotate = false;
    m_updateText = false;

    // fields in RAM must always have names, because we are trying to get
    // less dependent on field ids and more dependent on names.
    // Plus assumptions are made in the field editors.
    m_name = TEMPLATE_FIELDNAME::GetDefaultFieldName( id );
}


bool LIB_FIELD::Save( OUTPUTFORMATTER& aFormatter )
{
    int      hjustify, vjustify;
    wxString text = m_Text;

    hjustify = 'C';

    if( m_HJustify == GR_TEXT_HJUSTIFY_LEFT )
        hjustify = 'L';
    else if( m_HJustify == GR_TEXT_HJUSTIFY_RIGHT )
        hjustify = 'R';

    vjustify = 'C';

    if( m_VJustify == GR_TEXT_VJUSTIFY_BOTTOM )
        vjustify = 'B';
    else if( m_VJustify == GR_TEXT_VJUSTIFY_TOP )
        vjustify = 'T';

    if( text.IsEmpty() )
        text = wxT( "~" );

    aFormatter.Print( 0, "F%d %s %d %d %d %c %c %c %c%c%c",
                      m_id,
                      EscapedUTF8( text ).c_str(),       // wraps in quotes
                      m_Pos.x, m_Pos.y, m_Size.x,
                      m_Orient == 0 ? 'H' : 'V',
                      (m_Attributs & TEXT_NO_VISIBLE ) ? 'I' : 'V',
                      hjustify, vjustify,
                      m_Italic ? 'I' : 'N',
                      m_Bold ? 'B' : 'N' );

    /* Save field name, if necessary
     * Field name is saved only if it is not the default name.
     * Just because default name depends on the language and can change from
     * a country to an other
     */
    wxString defName = TEMPLATE_FIELDNAME::GetDefaultFieldName( m_id );

    if( m_id >= FIELD1 && !m_name.IsEmpty() && m_name != defName )
        aFormatter.Print( 0, " %s", EscapedUTF8( m_name ).c_str() );

    aFormatter.Print( 0, "\n" );

    return true;
}


bool LIB_FIELD::Load( LINE_READER& aLineReader, wxString& errorMsg )
{
    int   cnt;
    char  textOrient;
    char  textVisible;
    char  textHJustify;
    char  textVJustify[256];
    char* line = (char*) aLineReader;

    if( sscanf( line + 1, "%d", &m_id ) != 1 || m_id < 0 )
    {
        errorMsg = wxT( "invalid field header" );
        return false;
    }

    /* Search the beginning of the data. */
    while( *line != 0 )
        line++;

    while( *line == 0 )
        line++;

    while( *line && (*line != '"') )
        line++;

    if( *line == 0 )
        return false;

    line += ReadDelimitedText( &m_Text, line );

    if( *line == 0 )
        return false;

    memset( textVJustify, 0, sizeof( textVJustify ) );

    cnt = sscanf( line, " %d %d %d %c %c %c %s", &m_Pos.x, &m_Pos.y, &m_Size.y,
                  &textOrient, &textVisible, &textHJustify, textVJustify );

    if( cnt < 5 )
    {
        errorMsg.Printf( wxT( "field %d does not have the correct number of parameters" ),
                         m_id );
        return false;
    }

    m_Size.x = m_Size.y;

    if( textOrient == 'H' )
        m_Orient = TEXT_ORIENT_HORIZ;
    else if( textOrient == 'V' )
        m_Orient = TEXT_ORIENT_VERT;
    else
    {
        errorMsg.Printf( wxT( "field %d text orientation parameter <%c> is not valid" ),
                         textOrient );
        return false;
    }

    if( textVisible == 'V' )
        m_Attributs &= ~TEXT_NO_VISIBLE;
    else if ( textVisible == 'I' )
        m_Attributs |= TEXT_NO_VISIBLE;
    else
    {
        errorMsg.Printf( wxT( "field %d text visible parameter <%c> is not valid" ),
                         textVisible );
        return false;
    }

    m_HJustify = GR_TEXT_HJUSTIFY_CENTER;
    m_VJustify = GR_TEXT_VJUSTIFY_CENTER;

    if( cnt >= 6 )
    {
        if( textHJustify == 'C' )
            m_HJustify = GR_TEXT_HJUSTIFY_CENTER;
        else if( textHJustify == 'L' )
            m_HJustify = GR_TEXT_HJUSTIFY_LEFT;
        else if( textHJustify == 'R' )
            m_HJustify = GR_TEXT_HJUSTIFY_RIGHT;
        else
        {
            errorMsg.Printf(
                wxT( "field %d text horizontal justification parameter <%c> is not valid" ),
                textHJustify );
            return false;
        }

        if( textVJustify[0] == 'C' )
            m_VJustify = GR_TEXT_VJUSTIFY_CENTER;
        else if( textVJustify[0] == 'B' )
            m_VJustify = GR_TEXT_VJUSTIFY_BOTTOM;
        else if( textVJustify[0] == 'T' )
            m_VJustify = GR_TEXT_VJUSTIFY_TOP;
        else
        {
            errorMsg.Printf(
                wxT( "field %d text vertical justification parameter <%c> is not valid" ),
                textVJustify[0] );
            return false;
        }

        if ( textVJustify[1] == 'I' )  // Italic
            m_Italic = true;
        if ( textVJustify[2] == 'B' )  // Bold
            m_Bold = true;
    }

    // fields in RAM must always have names.
    if( m_id < MANDATORY_FIELDS )
    {
        // Fields in RAM must always have names, because we are trying to get
        // less dependent on field ids and more dependent on names.
        // Plus assumptions are made in the field editors.
        m_name = TEMPLATE_FIELDNAME::GetDefaultFieldName( m_id );
    }
    else
    {
        ReadDelimitedText( &m_name, line );
    }

    return true;
}


int LIB_FIELD::GetPenSize() const
{
    return ( m_Thickness == 0 ) ? g_DrawDefaultLineThickness : m_Thickness;
}


void LIB_FIELD::drawGraphic( EDA_DRAW_PANEL* aPanel, wxDC* aDC, const wxPoint& aOffset,
                             int aColor, int aDrawMode, void* aData, const TRANSFORM& aTransform )
{
    wxPoint  text_pos;
    int      color;
    int      linewidth = GetPenSize();

    if( m_Bold )
        linewidth = GetPenSizeForBold( m_Size.x );
    else
        linewidth = Clamp_Text_PenSize( linewidth, m_Size, m_Bold );

    if( ( m_Attributs & TEXT_NO_VISIBLE ) && ( aColor < 0 ) )
    {
        color = g_InvisibleItemColor;
    }
    else if( ( m_Selected & IS_SELECTED ) && ( aColor < 0 ) )
    {
        color = g_ItemSelectetColor;
    }
    else
    {
        color = aColor;
    }

    if( color < 0 )
        color = GetDefaultColor();

    text_pos = aTransform.TransformCoordinate( m_Pos ) + aOffset;

    wxString text;

    if( aData )
        text = *(wxString*)aData;
    else
        text = m_Text;

    GRSetDrawMode( aDC, aDrawMode );
    DrawGraphicText( aPanel, aDC, text_pos, (EDA_Colors) color, text, m_Orient, m_Size,
                     m_HJustify, m_VJustify, linewidth, m_Italic, m_Bold );

    /* Set to one (1) to draw bounding box around field text to validate
     * bounding box calculation. */
#if 0
    EDA_RECT bBox = GetBoundingBox();
    EDA_RECT grBox;
    grBox.SetOrigin( aTransform.TransformCoordinate( bBox.GetOrigin() ) );
    grBox.SetEnd( aTransform.TransformCoordinate( bBox.GetEnd() ) );
    grBox.Move( aOffset );
    GRRect( &aPanel->m_ClipBox, aDC, grBox, 0, LIGHTMAGENTA );
#endif
}


bool LIB_FIELD::HitTest( const wxPoint& aPosition )
{
    // Because HitTest is mainly used to select the field
    // return always false if this field is void
    if( IsVoid() )
        return false;

    return HitTest( aPosition, 0, DefaultTransform );
}


bool LIB_FIELD::HitTest( wxPoint aPosition, int aThreshold, const TRANSFORM& aTransform )
{
    if( aThreshold < 0 )
        aThreshold = 0;

    int extraCharCount = 0;

    // Reference designator text has one or 2 additional character (displays
    // U? or U?A)
    if( m_id == REFERENCE )
    {
        extraCharCount++;
        m_Text.Append('?');
        LIB_COMPONENT* parent = (LIB_COMPONENT*)m_Parent;

        if ( parent && ( parent->GetPartCount() > 1 ) )
        {
            m_Text.Append('A');
            extraCharCount++;
        }
    }

    wxPoint physicalpos = aTransform.TransformCoordinate( m_Pos );
    wxPoint tmp = m_Pos;
    m_Pos = physicalpos;

    /* The text orientation may need to be flipped if the
     *  transformation matrix causes xy axes to be flipped.
     * this simple algo works only for schematic matrix (rot 90 or/and mirror)
     */
    int t1 = ( aTransform.x1 != 0 ) ^ ( m_Orient != 0 );
    int orient = t1 ? TEXT_ORIENT_HORIZ : TEXT_ORIENT_VERT;
    EXCHG( m_Orient, orient );

    bool hit = TextHitTest( aPosition );

    EXCHG( m_Orient, orient );
    m_Pos = tmp;

    while( extraCharCount-- )
        m_Text.RemoveLast( );

    return hit;
}


EDA_ITEM* LIB_FIELD::doClone() const
{
    LIB_FIELD* newfield = new LIB_FIELD( m_id );

    Copy( newfield );

    return (EDA_ITEM*) newfield;
}


void LIB_FIELD::Copy( LIB_FIELD* aTarget ) const
{
    aTarget->SetParent( m_Parent );
    aTarget->m_Pos       = m_Pos;
    aTarget->m_Size      = m_Size;
    aTarget->m_Thickness = m_Thickness;
    aTarget->m_Orient    = m_Orient;
    aTarget->m_Attributs = m_Attributs;
    aTarget->m_Text      = m_Text;
    aTarget->m_name      = m_name;
    aTarget->m_HJustify  = m_HJustify;
    aTarget->m_VJustify  = m_VJustify;
    aTarget->m_Italic    = m_Italic;
    aTarget->m_Bold      = m_Bold;
}


int LIB_FIELD::DoCompare( const LIB_ITEM& other ) const
{
    wxASSERT( other.Type() == LIB_FIELD_T );

    const LIB_FIELD* tmp = ( LIB_FIELD* ) &other;

    if( m_id != tmp->m_id )
        return m_id - tmp->m_id;

    int result = m_Text.CmpNoCase( tmp->m_Text );

    if( result != 0 )
        return result;

    if( m_Pos.x != tmp->m_Pos.x )
        return m_Pos.x - tmp->m_Pos.x;

    if( m_Pos.y != tmp->m_Pos.y )
        return m_Pos.y - tmp->m_Pos.y;

    if( m_Size.x != tmp->m_Size.x )
        return m_Size.x - tmp->m_Size.x;

    if( m_Size.y != tmp->m_Size.y )
        return m_Size.y - tmp->m_Size.y;

    return 0;
}


void LIB_FIELD::DoOffset( const wxPoint& offset )
{
    m_Pos += offset;
}


bool LIB_FIELD::DoTestInside( EDA_RECT& rect ) const
{
    /*
     * FIXME: This fails to take into account the size and/or orientation of
     *        the text.
     */
    return rect.Contains( m_Pos.x, -m_Pos.y );
}


void LIB_FIELD::DoMove( const wxPoint& newPosition )
{
    m_Pos = newPosition;
}


void LIB_FIELD::DoMirrorHorizontal( const wxPoint& center )
{
    m_Pos.x -= center.x;
    m_Pos.x *= -1;
    m_Pos.x += center.x;
}

void LIB_FIELD::DoMirrorVertical( const wxPoint& center )
{
    m_Pos.y -= center.y;
    m_Pos.y *= -1;
    m_Pos.y += center.y;
}

void LIB_FIELD::DoRotate( const wxPoint& center, bool aRotateCCW )
{
    int rot_angle = aRotateCCW ? -900 : 900;
    RotatePoint( &m_Pos, center, rot_angle );
    m_Orient = m_Orient ? 0 : 900;
}


void LIB_FIELD::DoPlot( PLOTTER* plotter, const wxPoint& offset, bool fill,
                        const TRANSFORM& aTransform )
{
}


wxString LIB_FIELD::GetFullText( int unit )
{
    if( m_id != REFERENCE )
        return m_Text;

    wxString text = m_Text;
    text << wxT( "?" );

    if( GetParent()->IsMulti() )
        text << LIB_COMPONENT::ReturnSubReference( unit );

    return text;
}


EDA_RECT LIB_FIELD::GetBoundingBox() const
{
    /* Y coordinates for LIB_ITEMS are bottom to top, so we must invert the Y position when
     * calling GetTextBox() that works using top to bottom Y axis orientation.
     */
    EDA_RECT rect = GetTextBox( -1, -1, true );

    wxPoint orig = rect.GetOrigin();
    wxPoint end = rect.GetEnd();
    NEGATE( orig.y);
    NEGATE( end.y);

    RotatePoint( &orig, m_Pos, -m_Orient );
    RotatePoint( &end, m_Pos, -m_Orient );
    rect.SetOrigin( orig );
    rect.SetEnd( end );
    rect.Normalize();

    return rect;
}


int LIB_FIELD::GetDefaultColor()
{
    int color;

    switch( m_id )
    {
    case REFERENCE:
        color = ReturnLayerColor( LAYER_REFERENCEPART );
        break;

    case VALUE:
        color = ReturnLayerColor( LAYER_VALUEPART );
        break;

    default:
        color = ReturnLayerColor( LAYER_FIELDS );
        break;
    }

    return color;
}


void LIB_FIELD::Rotate()
{
    if( InEditMode() )
    {
        m_rotate = true;
    }
    else
    {
        m_Orient = ( m_Orient == TEXT_ORIENT_VERT ) ? TEXT_ORIENT_HORIZ : TEXT_ORIENT_VERT;
    }
}


wxString LIB_FIELD::GetName(bool aTranslate) const
{
    wxString name;

    switch( m_id )
    {
    case REFERENCE:
        if( aTranslate )
            name = _( "Reference" );
        else
            name = wxT( "Reference" );
        break;

    case VALUE:
        if( aTranslate )
            name = _( "Value" );
        else
            name = wxT( "Value" );
        break;

    case FOOTPRINT:
        if( aTranslate )
            name = _( "Footprint" );
        else
            name = wxT( "Footprint" );
        break;

    case DATASHEET:
        if( aTranslate )
           name = _( "Datasheet" );
        else
           name = wxT( "Datasheet" );
        break;

    default:
        if( m_name.IsEmpty() )
        {
            if( aTranslate )
                name.Printf( _( "Field%d" ), m_id );
            else
                name.Printf( wxT( "Field%d" ), m_id );
        }
        else
            name = m_name;
    }

    return name;
}


void LIB_FIELD::SetName( const wxString& aName )
{
    // Mandatory field names are fixed.

    // So what?  Why should the low level code be in charge of such a policy issue?
    // Besides, m_id is a relic that is untrustworthy now.
    if( m_id >=0 && m_id < MANDATORY_FIELDS )
    {
        D(printf( "trying to set a MANDATORY_FIELD's name\n" );)
        return;
    }

    if( m_name != aName )
    {
        m_name = aName;
        SetModified();
    }
}


void LIB_FIELD::SetText( const wxString& aText )
{
    if( aText == m_Text )
        return;

    wxString oldName = m_Text;

    if( m_id == VALUE && m_Parent != NULL )
    {
        LIB_COMPONENT* parent = GetParent();

        // Set the parent component and root alias to the new name.
        if( parent->GetName().CmpNoCase( aText ) != 0 )
            parent->SetName( aText );
    }

    if( InEditMode() )
    {
        m_Text = oldName;
        m_savedText = aText;
        m_updateText = true;
    }
    else
    {
        m_Text = aText;
    }
}


wxString LIB_FIELD::GetSelectMenuText() const
{
    return wxString::Format( _( "Field %s %s" ),
                             GetChars( GetName() ),
                             GetChars( GetText() ) );
}


void LIB_FIELD::BeginEdit( int aEditMode, const wxPoint aPosition )
{
    wxCHECK_RET( ( aEditMode & ( IS_NEW | IS_MOVED ) ) != 0,
                 wxT( "Invalid edit mode for LIB_FIELD object." ) );

    if( aEditMode == IS_MOVED )
    {
        m_initialPos = m_Pos;
        m_initialCursorPos = aPosition;
        SetEraseLastDrawItem();
    }
    else
    {
        m_Pos = aPosition;
    }

    m_Flags = aEditMode;
}


bool LIB_FIELD::ContinueEdit( const wxPoint aPosition )
{
    wxCHECK_MSG( ( m_Flags & ( IS_NEW | IS_MOVED ) ) != 0, false,
                   wxT( "Bad call to ContinueEdit().  Text is not being edited." ) );

    return false;
}


void LIB_FIELD::EndEdit( const wxPoint& aPosition, bool aAbort )
{
    wxCHECK_RET( ( m_Flags & ( IS_NEW | IS_MOVED ) ) != 0,
                   wxT( "Bad call to EndEdit().  Text is not being edited." ) );

    m_Flags = 0;
    m_rotate = false;
    m_updateText = false;
    SetEraseLastDrawItem( false );
}


void LIB_FIELD::calcEdit( const wxPoint& aPosition )
{
    if( m_rotate )
    {
        m_Orient = ( m_Orient == TEXT_ORIENT_VERT ) ? TEXT_ORIENT_HORIZ : TEXT_ORIENT_VERT;
        m_rotate = false;
    }

    if( m_updateText )
    {
        EXCHG( m_Text, m_savedText );
        m_updateText = false;
    }

    if( m_Flags == IS_NEW )
    {
        m_Pos = aPosition;
    }
    else if( m_Flags == IS_MOVED )
    {
        Move( m_initialPos + aPosition - m_initialCursorPos );
    }
}

void LIB_FIELD::DisplayInfo( EDA_DRAW_FRAME* aFrame )
{
    wxString msg;

    LIB_ITEM::DisplayInfo( aFrame );

    // Display style:
    msg = GetTextStyleName();
    aFrame->AppendMsgPanel( _( "Style" ), msg, MAGENTA );

    msg = ReturnStringFromValue( g_UserUnit, m_Size.x, EESCHEMA_INTERNAL_UNIT, true );
    aFrame->AppendMsgPanel( _( "Size X" ), msg, BLUE );

    msg = ReturnStringFromValue( g_UserUnit, m_Size.y, EESCHEMA_INTERNAL_UNIT, true );
    aFrame->AppendMsgPanel( _( "Size Y" ), msg, BLUE );

    // Display field name (ref, value ...)
    msg = GetName();
    aFrame->AppendMsgPanel( _( "Field" ), msg, BROWN );

    // Display field text:
    aFrame->AppendMsgPanel( _( "Value" ), m_Text, BROWN );
}
