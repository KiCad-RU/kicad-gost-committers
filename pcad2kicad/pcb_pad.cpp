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
 * @file pcb_pad.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>
#include <trigo.h>

#include <pcb_pad.h>


PCB_PAD::PCB_PAD( PCB_CALLBACKS* aCallbacks, BOARD* aBoard ) : PCB_COMPONENT( aCallbacks, aBoard )
{
    m_objType   = 'P';
    m_number    = 0;
    m_hole      = 0;
}


PCB_PAD::~PCB_PAD()
{
    int i;

    for( i = 0; i < (int) m_shapes.GetCount(); i++ )
    {
        delete m_shapes[i];
    }
}


int PCB_PAD::ToInt( char aChar )
{
    if( aChar >= '0' && aChar <= '9' )
        return aChar - '0';
    else
        return aChar - 'A' + 10;
}


// The former mentioned here that the following implementation needs to be revised
char PCB_PAD::OrMask( char aM1, char aM2 )
{
    wxString    s;
    int         i;

    if( aM1 == aM2 )
        return aM1;

    i   = ToInt( aM1 );
    i   += ToInt( aM2 );
    s   = wxString::Format( "%X", i );
    return s[0];
}


/*
 *  0 Copper layer
 *  1 to 14   Inner layers
 *  15 Component layer
 *  16 Copper side adhesive layer    Technical layers
 *  17 Component side adhesive layer
 *  18 Copper side Solder paste layer
 *  19 Component Solder paste layer
 *  20 Copper side Silk screen layer
 *  21 Component Silk screen layer
 *  22 Copper side Solder mask layer
 *  23 Component Solder mask layer
 *  24 Draw layer (Used for general drawings)
 *  25 Comment layer (Other layer used for general drawings)
 *  26 ECO1 layer (Other layer used for general drawings)
 *  26 ECO2 layer (Other layer used for general drawings)
 *  27 Edge layer. Items on Edge layer are seen on all layers
 */

wxString PCB_PAD::KiCadLayerMask( wxString aMask, int aLayer )
{
    wxString    newmask;
    int         i;

    /* Sometimes, a mask layer parameter is used.
     *  It is a 32 bits mask used to indicate a layer group usage (0 up to 32 layers).
     *  A mask layer parameter is given in hexadecimal form.
     *  Bit 0 is the copper layer, bit 1 is the inner 1 layer,
     *   and so on...(Bit 27 is the Edge layer).
     *  Mask layer is the ORed mask of the used layers */

    newmask = wxT( "00000000" );        // default

    if( aLayer == 0 )
        newmask = wxT( "00000001" );

    if( aLayer == 15 )
        newmask = wxT( "00008000" );

    if( aLayer == 16 )
        newmask = wxT( "00010000" );

    if( aLayer == 17 )
        newmask = wxT( "00020000" );

    if( aLayer == 18 )
        newmask = wxT( "00040000" );

    if( aLayer == 19 )
        newmask = wxT( "00080000" );

    if( aLayer == 20 )
        newmask = wxT( "00100000" );

    if( aLayer == 21 )
        newmask = wxT( "00200000" );

    if( aLayer == 22 )
        newmask = wxT( "00400000" );

    if( aLayer == 23 )
        newmask = wxT( "00800000" );

    if( aLayer == 24 )
        newmask = wxT( "01000000" );

    if( aLayer == 25 )
        newmask = wxT( "02000000" );

    if( aLayer == 26 )
        newmask = wxT( "04000000" );

    if( aLayer == 27 )
        newmask = wxT( "08000000" );

    if( aLayer == 28 )
        newmask = wxT( "10000000" );

    for( i = 0; i < 8; i++ )
        newmask[i] = OrMask( aMask[i], newmask[i] );

    return newmask;
}


unsigned long PCB_PAD::KiCadLayerMask( unsigned long aMask, int aLayer )
{
    unsigned long newMask;

    newMask = 1 << aLayer;

    if( !IsValidLayerIndex( aLayer ) )
        newMask = 0;

    if( aLayer == FIRST_COPPER_LAYER )
        newMask = 1;

    newMask |= aMask;

    return newMask;
}


void PCB_PAD::Parse( wxXmlNode* aNode, wxString aDefaultMeasurementUnit,
                     wxString aActualConversion )
{
    wxXmlNode*      lNode;
    long            num;
    wxString        propValue, str;
    PCB_PAD_SHAPE*  padShape;

    m_rotation = 0;
    lNode = FindNode( aNode->GetChildren(), wxT( "padNum" ) );

    if( lNode )
    {
        lNode->GetNodeContent().ToLong( &num );
        m_number = (int) num;
    }

    lNode = FindNode( aNode->GetChildren(), wxT( "padStyleRef" ) );

    if( lNode )
    {
        lNode->GetAttribute( wxT( "Name" ), &propValue );
        propValue.Trim( false );
        m_name.text = propValue;
    }

    lNode = FindNode( aNode->GetChildren(), wxT( "pt" ) );

    if( lNode )
        SetPosition( lNode->GetNodeContent(), aDefaultMeasurementUnit,
                     &m_positionX, &m_positionY, aActualConversion );

    lNode = FindNode( aNode->GetChildren(), wxT( "rotation" ) );

    if( lNode )
    {
        str = lNode->GetNodeContent();
        str.Trim( false );
        m_rotation = StrToInt1Units( str );
    }

    lNode = aNode;

    while( lNode->GetName() != wxT( "www.lura.sk" ) )
        lNode = lNode->GetParent();

    lNode   = FindNode( lNode->GetChildren(), wxT( "library" ) );
    lNode   = FindNode( lNode->GetChildren(), wxT( "padStyleDef" ) );

    while( true )
    {
        lNode->GetAttribute( wxT( "Name" ), &propValue );

        if( propValue == m_name.text )
            break;

        lNode = lNode->GetNext();
    }

    lNode = FindNode( lNode->GetChildren(), wxT( "holeDiam" ) );

    if( lNode )
        SetWidth( lNode->GetNodeContent(), aDefaultMeasurementUnit, &m_hole, aActualConversion );

    lNode   = lNode->GetParent();
    lNode   = FindNode( lNode->GetChildren(), wxT( "padShape" ) );

    while( lNode )
    {
        if( lNode->GetName() == wxT( "padShape" ) )
        {
            // we support only Pads on specific layers......
            // we do not support pads on "Plane", "NonSignal" , "Signal" ... layerr
            if( FindNode( lNode->GetChildren(), wxT( "layerNumRef" ) ) )
            {
                padShape = new PCB_PAD_SHAPE( m_callbacks, m_board );
                padShape->Parse( lNode, aDefaultMeasurementUnit, aActualConversion );
                m_shapes.Add( padShape );
            }
        }

        lNode = lNode->GetNext();
    }
}


void PCB_PAD::WriteToFile( wxFile* aFile, char aFileType, int aRotation )
{
    PCB_PAD_SHAPE*  padShape;
    wxString        s, layerMask, padType;
    int             i, lc, ls;

    // Library
    lc = 0; ls = 0;

    // Is it SMD pad , or not ?
    for( i = 0; i < (int) m_shapes.GetCount(); i++ )
    {
        padShape = m_shapes[i];

        if( padShape->m_width > 0 && padShape->m_height > 0 )
        {
            if( padShape->m_KiCadLayer == 15 )
                lc++; // Component

            if( padShape->m_KiCadLayer == 0 )
                ls++; // Cooper
        }
    }

    // And default layers mask
    layerMask   = wxT( "00C08001" ); // Comp,Coop,SoldCmp,SoldCoop
    padType     = wxT( "STD" );

    if( lc == 0 || ls == 0 )
    {
        if( m_hole == 0 )
        {
            padType = wxT( "SMD" );

            if( ls > 0 )
                layerMask = wxT( "00440001" );

            if( lc > 0 )
                layerMask = wxT( "00888000" );
        }
        else
        {
            if( ls > 0 )
                layerMask = wxT( "00400001" );

            if( lc > 0 )
                layerMask = wxT( "00808000" );
        }
    }

    // Go out
    for( i = 0; i < (int) m_shapes.GetCount(); i++ )
    {
        padShape = m_shapes[i];

        // maybe should not to be filtered ????
        if( padShape->m_width > 0 || padShape->m_height > 0 )
        {
            if( padShape->m_shape == wxT( "Oval" ) )
            {
                if( padShape->m_width != padShape->m_height )
                    s = wxT( "O" );
                else
                    s = wxT( "C" );
            }
            else if( padShape->m_shape == wxT( "Ellipse" ) )
                s = wxT( "O" );
            else if( padShape->m_shape == wxT( "Rect" )
                     || padShape->m_shape == wxT( "RndRect" ) )
                s = wxT( "R" );
            else if( padShape->m_shape == wxT( "Polygon" ) )
                s = wxT( "R" ); // approximation.....

            aFile->Write( wxT( "$PAD\n" ) );

            // Name, Shape, Xsize Ysize Xdelta Ydelta Orientation
            aFile->Write( wxT( "Sh \"" ) + m_name.text + wxT( "\" " ) + s +
                          wxString::Format( " %d %d 0 0 %d\n",
                                            padShape->m_width, padShape->m_height, m_rotation +
                                            aRotation ) );

            // Hole size , OffsetX, OffsetY
            aFile->Write( wxString::Format( "Dr %d 0 0\n", m_hole ) );

            layerMask = KiCadLayerMask( layerMask, padShape->m_KiCadLayer );

            // <Pad type> N <layer mask>
            aFile->Write( wxT( "At " ) + padType + wxT( " N " ) + layerMask + wxT( "\n" ) );

            // Reference
            aFile->Write( wxT( "Ne 0 \"" ) + m_net + "\"\n" );

            // Position
            aFile->Write( wxString::Format( "Po %d %d\n", m_positionX, m_positionY ) );
            aFile->Write( wxT( "$EndPAD\n" ) );
        }
    }
}


void PCB_PAD::AddToModule( MODULE* aModule, int aRotation )
{
    PCB_PAD_SHAPE*  padShape;
    wxString        s;
    PAD_ATTR_T      padType;
    unsigned long   layerMask;
    int             i, lc, ls;

    // Library
    lc = 0; ls = 0;

    // Is it SMD pad , or not ?
    for( i = 0; i < (int) m_shapes.GetCount(); i++ )
    {
        padShape = m_shapes[i];

        if( padShape->m_width > 0 && padShape->m_height > 0 )
        {
            if( padShape->m_KiCadLayer == 15 )
                lc++; // Component

            if( padShape->m_KiCadLayer == 0 )
                ls++; // Cooper
        }
    }

    // And default layers mask
    layerMask   = 0x00C08001; // Comp,Coop,SoldCmp,SoldCoop
    padType     = PAD_STANDARD;

    if( lc == 0 || ls == 0 )
    {
        if( m_hole == 0 )
        {
            padType = PAD_SMD;

            if( ls > 0 )
                layerMask = 0x00440001;

            if( lc > 0 )
                layerMask = 0x00888000;
        }
        else
        {
            if( ls > 0 )
                layerMask = 0x00400001;

            if( lc > 0 )
                layerMask = 0x00808000;
        }
    }

    // Go out
    for( i = 0; i < (int) m_shapes.GetCount(); i++ )
    {
        padShape = m_shapes[i];

        // maybe should not to be filtered ????
        if( padShape->m_width > 0 || padShape->m_height > 0 )
        {
            D_PAD* pad = new D_PAD( aModule );
            aModule->m_Pads.PushBack( pad );

            pad->SetPadName( m_name.text );

            if( padShape->m_shape == wxT( "Oval" ) )
            {
                if( padShape->m_width != padShape->m_height )
                    pad->SetShape( PAD_OVAL );
                else
                    pad->SetShape( PAD_CIRCLE );
            }
            else if( padShape->m_shape == wxT( "Ellipse" ) )
                pad->SetShape( PAD_OVAL );
            else if( padShape->m_shape == wxT( "Rect" )
                     || padShape->m_shape == wxT( "RndRect" ) )
                pad->SetShape( PAD_RECT );
            else if( padShape->m_shape == wxT( "Polygon" ) )
                pad->SetShape( PAD_RECT ); // approximation

            pad->SetSize( wxSize( padShape->m_width, padShape->m_height ) );
            pad->SetDelta( wxSize( 0, 0 ) );
            pad->SetOrientation( m_rotation + aRotation );

            pad->SetDrillShape( PAD_CIRCLE );
            pad->SetOffset( wxPoint( 0, 0 ) );
            pad->SetDrillSize( wxSize( m_hole, m_hole ) );

            layerMask = KiCadLayerMask( layerMask, padShape->m_KiCadLayer );

            pad->SetLayerMask( layerMask );
            pad->SetAttribute( padType );

            pad->SetNet( 0 );
            pad->SetNetname( m_net );

            // pad's "Position" is not relative to the module's,
            // whereas Pos0 is relative to the module's but is the unrotated coordinate.
            wxPoint padpos( m_positionX, m_positionY );
            pad->SetPos0( padpos );
            RotatePoint( &padpos, aModule->GetOrientation() );
            pad->SetPosition( padpos + aModule->GetPosition() );
        }
    }
}


void PCB_PAD::AddToBoard()
{
    PCB_PAD_SHAPE*  padShape;
    int             i;

    for( i = 0; i < (int) m_shapes.GetCount(); i++ )
    {
        padShape = m_shapes[i];

        // maybe should not to be filtered ????
        if( IsValidCopperLayerIndex( m_KiCadLayer )
            && ( padShape->m_width > 0 || padShape->m_height > 0 ) )
        {
            SEGVIA* via = new SEGVIA( m_board );
            m_board->m_Track.Append( via );

            via->SetTimeStamp( 0 );

            via->SetPosition( wxPoint( m_positionX, m_positionY ) );
            via->SetEnd( wxPoint( m_positionX, m_positionY ) );

            via->SetWidth( padShape->m_height );
            via->SetShape( VIA_THROUGH );
            ( (SEGVIA*) via )->SetLayerPair( LAYER_N_FRONT, LAYER_N_BACK );
            via->SetDrill( m_hole );

            via->SetLayer( m_KiCadLayer );
            via->SetNet( 0 );
        }
    }
}
