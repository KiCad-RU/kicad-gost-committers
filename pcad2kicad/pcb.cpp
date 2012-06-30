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
 * @file pcb.cpp
 */

#include <wx/wx.h>
#include <wx/config.h>

#include <common.h>

#include <pcb.h>
#include <pcb_arc.h>
#include <pcb_copper_pour.h>
#include <pcb_cutout.h>
#include <pcb_line.h>
#include <pcb_module.h>
#include <pcb_pad_shape.h>
#include <pcb_via_shape.h>
#include <pcb_pad.h>
#include <pcb_text.h>
#include <pcb_via.h>


int PCB::GetKiCadLayer( int aPCadLayer )
{
    assert( aPCadLayer >= 0 && aPCadLayer <= 28 );
    return m_layersMap[aPCadLayer];
}


PCB::PCB( BOARD* aBoard ) : PCB_MODULE( this, aBoard )
{
    int i;

    m_defaultMeasurementUnit = wxT( "mil" );

    for( i = 0; i < 28; i++ )
        m_layersMap[i] = 23; // default

    m_sizeX = 0;
    m_sizeY = 0;

    m_layersMap[1]  = 15;
    m_layersMap[2]  = 0;
    m_layersMap[3]  = 27;
    m_layersMap[6]  = 21;
    m_layersMap[7]  = 20;
    m_timestamp_cnt = 0x10000000;
}


PCB::~PCB()
{
    int i;

    for( i = 0; i < (int) m_pcbComponents.GetCount(); i++ )
    {
        delete m_pcbComponents[i];
    }

    for( i = 0; i < (int) m_pcbNetlist.GetCount(); i++ )
    {
        delete m_pcbNetlist[i];
    }
}


int PCB::GetNewTimestamp()
{
    return m_timestamp_cnt++;
}


wxXmlNode* PCB::FindCompDefName( wxXmlNode* aNode, wxString aName )
{
    wxXmlNode*  result = NULL, * lNode;
    wxString    propValue;

    lNode = FindNode( aNode->GetChildren(), wxT( "compDef" ) );

    while( lNode )
    {
        if( lNode->GetName() == wxT( "compDef" ) )
        {
            lNode->GetPropVal( wxT( "Name" ), &propValue );

            if( ValidateName( propValue ) == aName )
            {
                result  = lNode;
                lNode   = NULL;
            }
        }

        if( lNode )
            lNode = lNode->GetNext();
    }

    return result;
}


void PCB::SetTextProperty( wxXmlNode* aNode, TTEXTVALUE* aTextValue,
                           wxString aPatGraphRefName, wxString aXmlName,
                           wxString aActualConversion )
{
    wxXmlNode*  tNode, * t1Node;
    wxString    n, pn, propValue, str;

    // aNode is pattern now
    tNode   = aNode;
    t1Node  = aNode;
    n = aXmlName;

    // new file foramat version
    if( FindNode( tNode->GetChildren(), wxT( "patternGraphicsNameRef" ) ) )
    {
        FindNode( tNode->GetChildren(),
                  wxT( "patternGraphicsNameRef" ) )->GetPropVal( wxT( "Name" ),
                                                                 &pn );
        pn.Trim( false );
        pn.Trim( true );
        tNode = FindNode( tNode->GetChildren(), wxT( "patternGraphicsRef" ) );

        while( tNode )
        {
            if( tNode->GetName() == wxT( "patternGraphicsRef" ) )
            {
                if( FindNode( tNode->GetChildren(), wxT( "patternGraphicsNameRef" ) ) )
                {
                    FindNode( tNode->GetChildren(),
                              wxT( "patternGraphicsNameRef" ) )->GetPropVal( wxT( "Name" ),
                                                                             &propValue );

                    if( propValue == pn )
                    {
                        t1Node  = tNode; // find correct section with same name.
                        str     = aTextValue->text;
                        str.Trim( false );
                        str.Trim( true );
                        n       = n + ' ' + str; // changed in new file version.....
                        tNode   = NULL;
                    }
                }
            }

            if( tNode )
                tNode = tNode->GetNext();
        }
    }

    // old version and compatibile fr both from this point
    tNode = FindNode( t1Node->GetChildren(), wxT( "attr" ) );

    while( tNode )
    {
        tNode->GetPropVal( wxT( "Name" ), &propValue );
        propValue.Trim( false );
        propValue.Trim( true );

        if( propValue == n )
            break;

        tNode = tNode->GetNext();
    }

    if( tNode )
        SetTextParameters( tNode, aTextValue, m_defaultMeasurementUnit, aActualConversion );
}


void PCB::DoPCBComponents( wxXmlNode*       aNode,
                           wxXmlDocument*   aXmlDoc,
                           wxString         aActualConversion,
                           wxStatusBar*     aStatusBar )
{
    wxXmlNode*  lNode, * tNode, * mNode;
    PCB_MODULE* mc;
    PCB_PAD*    pad;
    PCB_VIA*    via;
    wxString    cn, str, propValue;

    lNode = aNode->GetChildren();

    while( lNode )
    {
        mc = NULL;

        if( lNode->GetName() == wxT( "pattern" ) )
        {
            FindNode( lNode->GetChildren(), wxT( "patternRef" ) )->GetPropVal( wxT( "Name" ),
                                                                               &cn );
            cn      = ValidateName( cn );
            tNode   = FindNode( aXmlDoc->GetRoot()->GetChildren(), wxT( "library" ) );

            if( tNode && cn.Len() > 0 )
            {
                tNode = FindModulePatternDefName( tNode, cn );

                if( tNode )
                {
                    mc = new PCB_MODULE( this, m_board );
                    mc->Parse( tNode, aStatusBar, m_defaultMeasurementUnit, aActualConversion );
                }
            }

            if( mc )
            {
                mc->m_compRef = cn;    // default - in new version of file it is updated later....
                tNode = FindNode( lNode->GetChildren(), wxT( "refDesRef" ) );

                if( tNode )
                {
                    tNode->GetPropVal( wxT( "Name" ), &mc->m_name.text );
                    SetTextProperty( lNode, &mc->m_name, mc->m_patGraphRefName, wxT(
                                         "RefDes" ), aActualConversion );
                    SetTextProperty( lNode, &mc->m_value, mc->m_patGraphRefName, wxT(
                                         "Value" ), aActualConversion );
                }

                tNode = FindNode( lNode->GetChildren(), wxT( "pt" ) );

                if( tNode )
                    SetPosition( tNode->GetNodeContent(),
                                 m_defaultMeasurementUnit,
                                 &mc->m_positionX,
                                 &mc->m_positionY,
                                 aActualConversion );

                tNode = FindNode( lNode->GetChildren(), wxT( "rotation" ) );

                if( tNode )
                {
                    str = tNode->GetNodeContent();
                    str.Trim( false );
                    mc->m_rotation = StrToInt1Units( str );
                }

                tNode = FindNode( lNode->GetChildren(), wxT( "isFlipped" ) );

                if( tNode )
                {
                    str = tNode->GetNodeContent();
                    str.Trim( false );
                    str.Trim( true );

                    if( str == wxT( "True" ) )
                        mc->m_mirror = 1;
                }

                tNode = aNode;

                while( tNode->GetName() != wxT( "www.lura.sk" ) )
                    tNode = tNode->GetParent();

                tNode = FindNode( tNode->GetChildren(), wxT( "netlist" ) );

                if( tNode )
                {
                    tNode = FindNode( tNode->GetChildren(), wxT( "compInst" ) );

                    while( tNode )
                    {
                        tNode->GetPropVal( wxT( "Name" ), &propValue );

                        if( propValue == mc->m_name.text )
                        {
                            if( FindNode( tNode->GetChildren(), wxT( "compValue" ) ) )
                            {
                                FindNode( tNode->GetChildren(),
                                          wxT( "compValue" ) )->GetPropVal( wxT( "Name" ),
                                                                            &mc->m_value.text );
                                mc->m_value.text.Trim( false );
                                mc->m_value.text.Trim( true );
                            }

                            if( FindNode( tNode->GetChildren(), wxT( "compRef" ) ) )
                            {
                                FindNode( tNode->GetChildren(),
                                          wxT( "compRef" ) )->GetPropVal( wxT( "Name" ),
                                                                          &mc->m_compRef );
                                mc->m_compRef.Trim( false );
                                mc->m_compRef.Trim( true );
                            }

                            tNode = NULL;
                        }
                        else
                            tNode = tNode->GetNext();
                    }
                }

                // map pins
                tNode   = FindNode( aXmlDoc->GetRoot()->GetChildren(), wxT( "library" ) );
                tNode   = FindCompDefName( tNode, mc->m_compRef );

                if( tNode )
                {
                    tNode = FindPinMap( tNode );

                    if( tNode )
                    {
                        mNode = tNode->GetChildren();

                        while( mNode )
                        {
                            if( mNode->GetName() == wxT( "padNum" ) )
                            {
                                str     = mNode->GetNodeContent();
                                mNode   = mNode->GetNext();

                                if( !mNode )
                                    break;

                                mNode->GetPropVal( wxT( "Name" ), &propValue );
                                mc->SetPadName( str, propValue );
                                mNode = mNode->GetNext();
                            }
                            else
                            {
                                mNode = mNode->GetNext();

                                if( !mNode )
                                    break;

                                mNode = mNode->GetNext();
                            }
                        }
                    }
                }

                m_pcbComponents.Add( mc );
            }
        }
        else if( lNode->GetName() == wxT( "pad" ) )
        {
            pad = new PCB_PAD( this, m_board );
            pad->Parse( lNode, m_defaultMeasurementUnit, aActualConversion );
            m_pcbComponents.Add( pad );
        }
        else if( lNode->GetName() == wxT( "via" ) )
        {
            via = new PCB_VIA( this, m_board );
            via->Parse( lNode, m_defaultMeasurementUnit, aActualConversion );
            m_pcbComponents.Add( via );
        }

        lNode = lNode->GetNext();
    }
}


void PCB::ConnectPinToNet( wxString aCr, wxString aPr, wxString aNetName )
{
    PCB_MODULE* module;
    PCB_PAD*    cp;
    int         i, j;

    for( i = 0; i < (int) m_pcbComponents.GetCount(); i++ )
    {
        module = (PCB_MODULE*) m_pcbComponents[i];

        if( module->m_objType == 'M' && module->m_name.text == aCr )
        {
            for( j = 0; j < (int) module->m_moduleObjects.GetCount(); j++ )
            {
                if( module->m_moduleObjects[j]->m_objType == 'P' )
                {
                    cp = (PCB_PAD*) module->m_moduleObjects[j];

                    if( cp->m_name.text == aPr )
                        cp->m_net = aNetName;
                }
            }
        }
    }
}


/* KiCad layers
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
 *  26 ECO1 layer (Other layer used for general drawings)       // BUG
 *  26 ECO2 layer (Other layer used for general drawings)       // BUG      27
 *  27 Edge layer. Items on Edge layer are seen on all layers   // BUG     28
 */
void PCB::MapLayer( wxXmlNode* aNode )
{
    wxString    lName;
    int         KiCadLayer;
    long        num;

    aNode->GetPropVal( wxT( "Name" ), &lName );
    lName = lName.MakeUpper();
    KiCadLayer = 24;    // defaullt

    if( lName == wxT( "TOP ASSY" ) )
    {
    }                                 // ?

    if( lName == wxT( "TOP SILK" ) )
        KiCadLayer = 21;

    if( lName == wxT( "TOP PASTE" ) )
        KiCadLayer = 19;

    if( lName == wxT( "TOP MASK" ) )
        KiCadLayer = 23;

    if( lName == wxT( "TOP" ) )
        KiCadLayer = 15;

    if( lName == wxT( "BOTTOM" ) )
        KiCadLayer = 0;

    if( lName == wxT( "BOT MASK" ) )
        KiCadLayer = 22;

    if( lName == wxT( "BOT PASTE" ) )
        KiCadLayer = 18;

    if( lName == wxT( "BOT SILK" ) )
        KiCadLayer = 20;

    if( lName == wxT( "BOT ASSY" ) )
    {
    }                                 // ?

    if( lName == wxT( "BOARD" ) )
        KiCadLayer = 28;

    FindNode( aNode->GetChildren(), wxT( "layerNum" ) )->GetNodeContent().ToLong( &num );
    m_layersMap[(int) num] = KiCadLayer;
}


void PCB::Parse( wxStatusBar* aStatusBar, wxString aXMLFileName, wxString aActualConversion )
{
    wxXmlDocument   xmlDoc;
    wxXmlNode*      aNode;
    PCB_NET*        net;
    PCB_COMPONENT*  comp;
    PCB_MODULE*     module;
    wxString        cr, pr;
    int             i, j;

    if( !xmlDoc.Load( aXMLFileName ) )
        return;

    // Defaut measurement units
    aNode = FindNode( xmlDoc.GetRoot()->GetChildren(), wxT( "asciiHeader" ) );

    if( aNode )
    {
        aNode = FindNode( aNode->GetChildren(), wxT( "fileUnits" ) );

        if( aNode )
        {
            m_defaultMeasurementUnit = aNode->GetNodeContent();
            m_defaultMeasurementUnit.Trim( true );
            m_defaultMeasurementUnit.Trim( false );
        }
    }

    // Layers mapping
    aNode = FindNode( xmlDoc.GetRoot()->GetChildren(), wxT( "pcbDesign" ) );

    if( aNode )
    {
        aNode = FindNode( aNode->GetChildren(), wxT( "layerDef" ) );

        while( aNode )
        {
            if( aNode->GetName() == wxT( "layerDef" ) )
                MapLayer( aNode );

            aNode = aNode->GetNext();
        }
    }

    // NETLIST
    // aStatusBar->SetStatusText( wxT( "Loading NETLIST " ) );

    aNode = FindNode( xmlDoc.GetRoot()->GetChildren(), wxT( "netlist" ) );

    if( aNode )
    {
        aNode = FindNode( aNode->GetChildren(), wxT( "net" ) );

        while( aNode )
        {
            net = new PCB_NET;
            net->Parse( aNode );
            m_pcbNetlist.Add( net );

            aNode = aNode->GetNext();
        }
    }

    // BOARD FILE
    // aStatusBar->SetStatusText( wxT( "Loading BOARD DEFINITION " ) );

    aNode = FindNode( xmlDoc.GetRoot()->GetChildren(), wxT( "pcbDesign" ) );

    if( aNode )
    {
        // COMPONENTS AND OBJECTS
        aNode = aNode->GetChildren();

        while( aNode )
        {
            // Components/modules
            if( aNode->GetName() == wxT( "multiLayer" ) )
                DoPCBComponents( aNode, &xmlDoc, aActualConversion, aStatusBar );

            // objects
            if( aNode->GetName() == wxT( "layerContents" ) )
                DoLayerContentsObjects( aNode, NULL, &m_pcbComponents, aStatusBar,
                                        m_defaultMeasurementUnit, aActualConversion );

            aNode = aNode->GetNext();
        }

        // POSTPROCESS -- SET NETLIST REFERENCES
        // aStatusBar->SetStatusText( wxT( "Processing NETLIST " ) );

        for( i = 0; i < (int) m_pcbNetlist.GetCount(); i++ )
        {
            net = m_pcbNetlist[i];

            for( j = 0; j < (int) net->m_netNodes.GetCount(); j++ )
            {
                cr = net->m_netNodes[j]->m_compRef;
                cr.Trim( false );
                cr.Trim( true );
                pr = net->m_netNodes[j]->m_pinRef;
                pr.Trim( false );
                pr.Trim( true );
                ConnectPinToNet( cr, pr, net->m_name );
            }
        }

        // POSTPROCESS -- FLIP COMPONENTS
        for( i = 0; i < (int) m_pcbComponents.GetCount(); i++ )
        {
            if( m_pcbComponents[i]->m_objType == 'M' )
                ( (PCB_MODULE*) m_pcbComponents[i] )->Flip();
        }

        // POSTPROCESS -- SET/OPTIMIZE NEW PCB POSITION
        // aStatusBar->SetStatusText( wxT( "Optimizing BOARD POSITION " ) );

        m_sizeX = 10000000;
        m_sizeY = 0;

        for( i = 0; i < (int) m_pcbComponents.GetCount(); i++ )
        {
            comp = m_pcbComponents[i];

            if( comp->m_positionY < m_sizeY )
                m_sizeY = comp->m_positionY; // max Y

            if( comp->m_positionX < m_sizeX && comp->m_positionX > 0 )
                m_sizeX = comp->m_positionX; // Min X
        }

        m_sizeY -= 10000;
        m_sizeX -= 10000;
        // aStatusBar->SetStatusText( wxT( " POSITIONING POSTPROCESS " ) );

        for( i = 0; i < (int) m_pcbComponents.GetCount(); i++ )
            m_pcbComponents[i]->SetPosOffset( -m_sizeX, -m_sizeY );

        m_sizeX = 0;
        m_sizeY = 0;

        for( i = 0; i < (int) m_pcbComponents.GetCount(); i++ )
        {
            comp = m_pcbComponents[i];

            if( comp->m_positionY < m_sizeY )
                m_sizeY = comp->m_positionY; // max Y

            if( comp->m_positionX > m_sizeX )
                m_sizeX = comp->m_positionX; // Min X
        }

        // SHEET SIZE CALCULATION
        m_sizeY = -m_sizeY;    // it is in absolute units
        m_sizeX += 10000;
        m_sizeY += 10000;

        // A4 is minimum $Descr A4 11700 8267
        if( m_sizeX < 11700 )
            m_sizeX = 11700;

        if( m_sizeY < 8267 )
            m_sizeY = 8267;
    }
    else
    {
        // LIBRARY FILE
        // aStatusBar->SetStatusText( wxT( "Processing LIBRARY FILE " ) );

        aNode = FindNode( xmlDoc.GetRoot()->GetChildren(), wxT( "library" ) );

        if( aNode )
        {
            aNode = FindNode( aNode->GetChildren(), wxT( "compDef" ) );

            while( aNode )
            {
                // aStatusBar->SetStatusText( wxT( "Processing COMPONENTS " ) );

                if( aNode->GetName() == wxT( "compDef" ) )
                {
                    module = new PCB_MODULE( this, m_board );
                    module->Parse( aNode, aStatusBar, m_defaultMeasurementUnit,
                                   aActualConversion );
                    m_pcbComponents.Add( module );
                }

                aNode = aNode->GetNext();
            }
        }
    }
}


void PCB::WriteToFile( wxString aFileName )
{
    wxFile  f;
    int     i;

    f.Open( aFileName, wxFile::write );

    // LIBRARY
    f.Write( wxT( "PCBNEW-LibModule-V1  01/01/2001-01:01:01\n" ) );
    f.Write( wxT( "\n" ) );
    f.Write( wxT( "$INDEX\n" ) );

    for( i = 0; i < (int) m_pcbComponents.GetCount(); i++ )
    {
        if( m_pcbComponents[i]->m_objType == 'M' )
            f.Write( m_pcbComponents[i]->m_name.text + wxT( "\n" ) );
    }

    f.Write( wxT( "$EndINDEX\n" ) );

    for( i = 0; i < (int) m_pcbComponents.GetCount(); i++ )
    {
        if( m_pcbComponents[i]->m_objType == 'M' )
            m_pcbComponents[i]->WriteToFile( &f, 'L' );
    }

    f.Write( wxT( "$EndLIBRARY\n" ) );

    f.Close();
}


void PCB::AddToBoard()
{
    int i;

    for( i = 0; i < (int) m_pcbComponents.GetCount(); i++ )
    {
        m_pcbComponents[i]->AddToBoard();
    }
}
