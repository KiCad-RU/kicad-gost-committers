/**
 * @file wxwineda.cpp
 */

#include "fctsys.h"
#include "wxstruct.h"
#include "dialog_helpers.h"


/*******************************************************/
/* Class to edit a graphic + text size in INCHES or MM */
/*******************************************************/
EDA_GRAPHIC_TEXT_CTRL::EDA_GRAPHIC_TEXT_CTRL( wxWindow*       parent,
                                              const wxString& Title,
                                              const wxString& TextToEdit,
                                              int             textsize,
                                              EDA_UNITS_T     user_unit,
                                              wxBoxSizer*     BoxSizer,
                                              int             framelen,
                                              int             internal_unit )
{
    m_UserUnit = user_unit;
    m_Internal_Unit = internal_unit;
    m_Title = NULL;

    m_Title = new wxStaticText( parent, -1, Title );

    BoxSizer->Add( m_Title, 0,
                   wxGROW | wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE, 5 );

    m_FrameText = new wxTextCtrl( parent, -1, TextToEdit );

    BoxSizer->Add( m_FrameText, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

    if( !Title.IsEmpty() )
    {
        wxString      msg = _( "Size" ) + ReturnUnitSymbol( m_UserUnit );
        wxStaticText* text = new wxStaticText( parent, -1, msg );

        BoxSizer->Add( text, 0,
                       wxGROW | wxLEFT | wxRIGHT | wxADJUST_MINSIZE, 5 );
    }

    wxString value = FormatSize( m_Internal_Unit, m_UserUnit, textsize );

    m_FrameSize = new wxTextCtrl( parent, -1, value, wxDefaultPosition,
                                  wxSize( 70, -1 ) );

    BoxSizer->Add( m_FrameSize, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 5 );
}


EDA_GRAPHIC_TEXT_CTRL::~EDA_GRAPHIC_TEXT_CTRL()
{
    /* no, these are deleted by the BoxSizer
    delete m_FrameText;
    delete m_Title;
    */
}


wxString EDA_GRAPHIC_TEXT_CTRL::FormatSize( int internalUnit, EDA_UNITS_T aUnit,
                                            int textSize )
{
    wxString value;

    // Limiting the size of the text of reasonable values.
    if( textSize < 10 )
        textSize = 10;

    if( textSize > 3000 )
        textSize = 3000;

    value.Printf( ( internalUnit > 1000 ) ? wxT( "%.4f" ) : wxT( "%.3f" ),
                  To_User_Unit( aUnit, textSize, internalUnit ) );

    return value;
}


void EDA_GRAPHIC_TEXT_CTRL::SetTitle( const wxString& title )
{
    m_Title->SetLabel( title );
}


void EDA_GRAPHIC_TEXT_CTRL::SetValue( const wxString& value )
{
    m_FrameText->SetValue( value );
}


void EDA_GRAPHIC_TEXT_CTRL::SetValue( int textSize )
{
    wxString value = FormatSize( m_Internal_Unit, m_UserUnit, textSize );
    m_FrameSize->SetValue( value );
}


wxString EDA_GRAPHIC_TEXT_CTRL::GetText()
{
    wxString text = m_FrameText->GetValue();
    return text;
}


int EDA_GRAPHIC_TEXT_CTRL::ParseSize( const wxString& sizeText,
                                      int internalUnit, EDA_UNITS_T aUnit )
{
    int    textsize;

    textsize = ReturnValueFromString( aUnit, sizeText, internalUnit );

    // Limit to reasonable size
    if( textsize < 10 )
        textsize = 10;

    if( textsize > 3000 )
        textsize = 3000;

    return textsize;
}


int EDA_GRAPHIC_TEXT_CTRL::GetTextSize()
{
    return ParseSize( m_FrameSize->GetValue(), m_Internal_Unit, m_UserUnit );
}


void EDA_GRAPHIC_TEXT_CTRL::Enable( bool state )
{
    m_FrameText->Enable( state );
}


/********************************************************/
/* Class to display and edit a coordinated INCHES or MM */
/********************************************************/
EDA_POSITION_CTRL::EDA_POSITION_CTRL( wxWindow*       parent,
                                      const wxString& title,
                                      const wxPoint&  pos_to_edit,
                                      EDA_UNITS_T     user_unit,
                                      wxBoxSizer*     BoxSizer,
                                      int             internal_unit )
{
    wxString text;

    m_UserUnit = user_unit;
    m_Internal_Unit = internal_unit;

    if( title.IsEmpty() )
        text = _( "Pos " );
    else
        text = title;

    text   += _( "X" ) + ReturnUnitSymbol( m_UserUnit );
    m_TextX = new wxStaticText( parent, -1, text );

    BoxSizer->Add( m_TextX, 0,
                   wxGROW | wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE, 5 );
    m_FramePosX = new wxTextCtrl( parent, -1, wxEmptyString,
                                  wxDefaultPosition );

    BoxSizer->Add( m_FramePosX, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 5 );


    if( title.IsEmpty() )
        text = _( "Pos " );
    else
        text = title;
    text   += _( "Y" ) + ReturnUnitSymbol( m_UserUnit );

    m_TextY = new wxStaticText( parent, -1, text );

    BoxSizer->Add( m_TextY, 0,
                   wxGROW | wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE, 5 );

    m_FramePosY = new wxTextCtrl( parent, -1, wxEmptyString );

    BoxSizer->Add( m_FramePosY, 0, wxGROW | wxLEFT | wxRIGHT | wxBOTTOM, 5 );

    SetValue( pos_to_edit.x, pos_to_edit.y );
}


EDA_POSITION_CTRL::~EDA_POSITION_CTRL()
{
    delete m_TextX;
    delete m_TextY;
    delete m_FramePosX;
    delete m_FramePosY;
}


/* Returns (in internal units) to coordinate between (in user units)
 */
wxPoint EDA_POSITION_CTRL::GetValue()
{
    wxPoint coord;

    coord.x = ReturnValueFromString( m_UserUnit, m_FramePosX->GetValue(), m_Internal_Unit );
    coord.y = ReturnValueFromString( m_UserUnit, m_FramePosY->GetValue(), m_Internal_Unit );

    return coord;
}


void EDA_POSITION_CTRL::Enable( bool x_win_on, bool y_win_on )
{
    m_FramePosX->Enable( x_win_on );
    m_FramePosY->Enable( y_win_on );
}


void EDA_POSITION_CTRL::SetValue( int x_value, int y_value )
{
    wxString msg;

    m_Pos_To_Edit.x = x_value;
    m_Pos_To_Edit.y = y_value;

    msg = ReturnStringFromValue( m_UserUnit, m_Pos_To_Edit.x, m_Internal_Unit );
    m_FramePosX->Clear();
    m_FramePosX->SetValue( msg );

    msg = ReturnStringFromValue( m_UserUnit, m_Pos_To_Edit.y, m_Internal_Unit );
    m_FramePosY->Clear();
    m_FramePosY->SetValue( msg );
}


/*******************/
/* EDA_SIZE_CTRL */
/*******************/
EDA_SIZE_CTRL::EDA_SIZE_CTRL( wxWindow* parent, const wxString& title,
                              const wxSize& size_to_edit,
                              EDA_UNITS_T aUnit, wxBoxSizer* aBoxSizer,
                              int internal_unit ) :
    EDA_POSITION_CTRL( parent, title,
                       wxPoint( size_to_edit.x, size_to_edit.y ),
                       aUnit, aBoxSizer, internal_unit )
{
}


wxSize EDA_SIZE_CTRL::GetValue()
{
    wxPoint pos = EDA_POSITION_CTRL::GetValue();
    wxSize  size;

    size.x = pos.x;
    size.y = pos.y;
    return size;
}


/**************************************************************/
/* Class to display and edit a dimension INCHES, MM, or other */
/**************************************************************/
EDA_VALUE_CTRL::EDA_VALUE_CTRL( wxWindow* parent, const wxString& title,
                                int value, EDA_UNITS_T user_unit, wxBoxSizer* BoxSizer,
                                int internal_unit )
{
    wxString label = title;

    m_UserUnit = user_unit;
    m_Internal_Unit = internal_unit;
    m_Value = value;
    label  += ReturnUnitSymbol( m_UserUnit );

    m_Text = new wxStaticText( parent, -1, label );

    BoxSizer->Add( m_Text, 0,
                   wxGROW | wxLEFT | wxRIGHT | wxTOP | wxADJUST_MINSIZE, 5 );

    wxString stringvalue = ReturnStringFromValue( m_UserUnit, m_Value,
                                                  m_Internal_Unit );
    m_ValueCtrl = new   wxTextCtrl( parent, -1, stringvalue );

    BoxSizer->Add( m_ValueCtrl,
                   0,
                   wxGROW | wxALIGN_CENTER_VERTICAL | wxLEFT | wxRIGHT | wxBOTTOM,
                   5 );
}


EDA_VALUE_CTRL::~EDA_VALUE_CTRL()
{
    delete m_ValueCtrl;
    delete m_Text;
}


int EDA_VALUE_CTRL::GetValue()
{
    int      coord;
    wxString txtvalue = m_ValueCtrl->GetValue();

    coord = ReturnValueFromString( m_UserUnit, txtvalue, m_Internal_Unit );
    return coord;
}


void EDA_VALUE_CTRL::SetValue( int new_value )
{
    wxString buffer;

    m_Value = new_value;

    buffer = ReturnStringFromValue( m_UserUnit, m_Value, m_Internal_Unit );
    m_ValueCtrl->SetValue( buffer );
}


void EDA_VALUE_CTRL::Enable( bool enbl )
{
    m_ValueCtrl->Enable( enbl );
    m_Text->Enable( enbl );
}
