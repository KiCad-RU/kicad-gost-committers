/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Cirilo Bernardo <cirilo.bernardo@gmail.com>
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

// Note: the board's bottom side is at Z = 0

#include <iostream>
#include <sstream>
#include <cmath>
#include <string>
#include <map>
#include <wx/log.h>
#include <wx/string.h>
#include "plugins/3d/3d_plugin.h"
#include "plugins/3dapi/ifsg_all.h"
#include "idf_parser.h"
#include "vrml_layer.h"

#define PLUGIN_3D_IDF_MAJOR 1
#define PLUGIN_3D_IDF_MINOR 0
#define PLUGIN_3D_IDF_PATCH 0
#define PLUGIN_3D_IDF_REVNO 0

// number of colors in the palette; cycles from 1..NCOLORS;
// number 0 is special (the PCB board color)
#define NCOLORS 6

// log mask for wxLogTrace
#define MASK_IDF "PLUGIN_IDF"


// read and instantiate an IDF component outline
static SCENEGRAPH* loadIDFOutline( const wxString& aFileName );
// read and render an IDF board assembly
static SCENEGRAPH* loadIDFBoard( const wxString& aFileName );
// model a single extruded outline
// idxColor = color index to use
// aParent = parent SCENEGRAPH object, if any
static SCENEGRAPH* addOutline( IDF3_COMP_OUTLINE* outline, int idxColor, SGNODE* aParent );
// model the board extrusion
static SCENEGRAPH* makeBoard( IDF3_BOARD& brd, SGNODE* aParent );
// model all included components
static bool makeComponents( IDF3_BOARD& brd, SGNODE* aParent );
// model any .OTHER_OUTLINE items
static bool makeOtherOutlines( IDF3_BOARD& brd, SGNODE* aParent );
// convert the IDF outline to VRML intermediate data
static bool getOutlineModel( VRML_LAYER& model, const std::list< IDF_OUTLINE* >* items );
// convert IDF segment data to VRML segment data
static bool addSegment( VRML_LAYER& model, IDF_SEGMENT* seg, int icont, int iseg );
// convert the VRML intermediate data into SG* data
static SCENEGRAPH* vrmlToSG( VRML_LAYER& vpcb, int idxColor, SGNODE* aParent, double top, double bottom );


class LOCALESWITCH
{
public:
    LOCALESWITCH()
    {
        setlocale( LC_NUMERIC, "C" );
    }

    ~LOCALESWITCH()
    {
        setlocale( LC_NUMERIC, "" );
    }
};


static SGNODE* getColor( IFSG_SHAPE& shape, int colorIdx )
{
    IFSG_APPEARANCE material( shape );

    static int cidx = 1;
    int idx;

    if( colorIdx == -1 )
        idx = cidx;
    else
        idx = colorIdx;

    switch( idx )
    {
    case 0:
        // green for PCB
        material.SetSpecular( 0.13, 0.81, 0.22 );
        material.SetDiffuse( 0.13, 0.81, 0.22 );
        // default ambient intensity
        material.SetShininess( 0.3 );

        break;

    case 1:
        // magenta
        material.SetSpecular( 0.8, 0.0, 0.8 );
        material.SetDiffuse( 0.6, 0.0, 0.6 );
        // default ambient intensity
        material.SetShininess( 0.3 );

        break;

    case 2:
        // red
        material.SetSpecular( 0.69, 0.14, 0.14 );
        material.SetDiffuse( 0.69, 0.14, 0.14 );
        // default ambient intensity
        material.SetShininess( 0.3 );

        break;

    case 3:
        // orange
        material.SetSpecular( 1.0, 0.44, 0.0 );
        material.SetDiffuse( 1.0, 0.44, 0.0 );
        // default ambient intensity
        material.SetShininess( 0.3 );

        break;

    case 4:
        // yellow
        material.SetSpecular( 0.93, 0.94, 0.16 );
        material.SetDiffuse( 0.93, 0.94, 0.16 );
        // default ambient intensity
        material.SetShininess( 0.3 );

        break;

    case 5:
        // blue
        material.SetSpecular( 0.1, 0.11, 0.88 );
        material.SetDiffuse( 0.1, 0.11, 0.88 );
        // default ambient intensity
        material.SetShininess( 0.3 );

        break;

    default:
        // violet
        material.SetSpecular( 0.32, 0.07, 0.64 );
        material.SetDiffuse( 0.32, 0.07, 0.64 );
        // default ambient intensity
        material.SetShininess( 0.3 );

        break;
    }

    if( ( colorIdx == -1 ) && ( ++cidx > NCOLORS ) )
        cidx = 1;

    return material.GetRawPtr();
};


const char* GetKicadPluginName( void )
{
    return "PLUGIN_3D_IDF";
}


void GetPluginVersion( unsigned char* Major,
    unsigned char* Minor, unsigned char* Patch, unsigned char* Revision )
{
    if( Major )
        *Major = PLUGIN_3D_IDF_MAJOR;

    if( Minor )
        *Minor = PLUGIN_3D_IDF_MINOR;

    if( Patch )
        *Patch = PLUGIN_3D_IDF_PATCH;

    if( Revision )
        *Revision = PLUGIN_3D_IDF_REVNO;

    return;
}

// number of extensions supported
#ifdef _WIN32
    #define NEXTS 2
#else
    #define NEXTS 4
#endif

// number of filter sets supported
#define NFILS 2

static char ext0[] = "idf";
static char ext1[] = "emn";

#ifdef _WIN32
    static char fil0[] = "IDF (*.idf)|*.idf";
    static char fil1[] = "IDF BRD v2/v3 (*.emn)|*.emn";
#else
    static char ext2[] = "IDF";
    static char ext3[] = "EMN";
    static char fil0[] = "IDF (*.idf;*.IDF)|*.idf;*.IDF";
    static char fil1[] = "IDF BRD (*.emn;*.EMN)|*.emn;*.EMN";
#endif

static struct FILE_DATA
{
    char const* extensions[NEXTS];
    char const* filters[NFILS];

    FILE_DATA()
    {
        extensions[0] = ext0;
        extensions[1] = ext1;
        filters[0] = fil0;
        filters[1] = fil1;

#ifndef _WIN32
        extensions[2] = ext2;
        extensions[3] = ext3;
#endif

        return;
    }

} file_data;


int GetNExtensions( void )
{
    return NEXTS;
}


char const* GetModelExtension( int aIndex )
{
    if( aIndex < 0 || aIndex >= NEXTS )
        return NULL;

    return file_data.extensions[aIndex];
}


int GetNFilters( void )
{
    return NFILS;
}


char const* GetFileFilter( int aIndex )
{
    if( aIndex < 0 || aIndex >= NFILS )
        return NULL;

    return file_data.filters[aIndex];
}


bool CanRender( void )
{
    // this plugin supports rendering of IDF component outlines
    return true;
}


SCENEGRAPH* Load( char const* aFileName )
{
    if( NULL == aFileName )
        return NULL;

    wxFileName fname;
    fname.Assign( wxString::FromUTF8Unchecked( aFileName ) );

    wxString ext = fname.GetExt();

    SCENEGRAPH* data = NULL;

    if( !ext.Cmp( wxT( "idf" ) ) || !ext.Cmp( wxT( "IDF" ) ) )
    {
        data = loadIDFOutline( fname.GetFullPath() );
    }

    if( !ext.Cmp( wxT( "emn" ) ) || !ext.Cmp( wxT( "EMN" ) ) )
    {
        data = loadIDFBoard( fname.GetFullPath() );
    }

    // DEBUG: WRITE OUT IDF FILE TO CONFIRM NORMALS
    #if defined( DEBUG_IDF ) && DEBUG_IDF > 3
    if( data )
    {
        wxFileName fn( aFileName );
        wxString output = wxT( "_idf-" );
        output.append( fn.GetName() );
        output.append( wxT(".wrl") );
        S3D::WriteVRML( output.ToUTF8(), true, (SGNODE*)(data), true, true );
    }
    #endif

    return data;
}


static bool getOutlineModel( VRML_LAYER& model, const std::list< IDF_OUTLINE* >* items )
{
    // empty outlines are not unusual so we fail quietly
    if( items->size() < 1 )
        return false;

    int nvcont = 0;
    int iseg   = 0;

    std::list< IDF_OUTLINE* >::const_iterator scont = items->begin();
    std::list< IDF_OUTLINE* >::const_iterator econt = items->end();
    std::list<IDF_SEGMENT*>::iterator sseg;
    std::list<IDF_SEGMENT*>::iterator eseg;

    IDF_SEGMENT lseg;

    while( scont != econt )
    {
        nvcont = model.NewContour();

        if( nvcont < 0 )
        {
            #ifdef DEBUG
            do {
                std::ostringstream ostr;
                std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
                std::cerr << " * [INFO] cannot create an outline";
                wxLogTrace( MASK_IDF, "%s\n", ostr.str().c_str() );
            } while( 0 );
            #endif

            return false;
        }

        if( (*scont)->size() < 1 )
        {
            #ifdef DEBUG
            do {
                std::ostringstream ostr;
                std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
                std::cerr << " * [INFO] invalid contour: no vertices";
                wxLogTrace( MASK_IDF, "%s\n", ostr.str().c_str() );
            } while( 0 );
            #endif

            return false;
        }

        sseg = (*scont)->begin();
        eseg = (*scont)->end();

        iseg = 0;
        while( sseg != eseg )
        {
            lseg = **sseg;

            if( !addSegment( model, &lseg, nvcont, iseg ) )
            {
                #ifdef DEBUG
                do {
                    std::ostringstream ostr;
                    std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
                    std::cerr << " * [BUG] cannot add segment";
                    wxLogTrace( MASK_IDF, "%s\n", ostr.str().c_str() );
                } while( 0 );
                #endif

                return false;
            }

            ++iseg;
            ++sseg;
        }

        ++scont;
    }

    return true;
}


static bool addSegment( VRML_LAYER& model, IDF_SEGMENT* seg, int icont, int iseg )
{
    // note: in all cases we must add all but the last point in the segment
    // to avoid redundant points

    if( seg->angle != 0.0 )
    {
        if( seg->IsCircle() )
        {
            if( iseg != 0 )
            {
                #ifdef DEBUG
                do {
                    std::ostringstream ostr;
                    std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
                    std::cerr << " * [INFO] adding a circle to an existing vertex list";
                    wxLogTrace( MASK_IDF, "%s\n", ostr.str().c_str() );
                } while( 0 );
                #endif

                return false;
            }

            return model.AppendCircle( seg->center.x, seg->center.y, seg->radius, icont );
        }
        else
        {
            return model.AppendArc( seg->center.x, seg->center.y, seg->radius,
                                    seg->offsetAngle, seg->angle, icont );
        }
    }

    if( !model.AddVertex( icont, seg->startPoint.x, seg->startPoint.y ) )
        return false;

    return true;
}


static SCENEGRAPH* vrmlToSG( VRML_LAYER& vpcb, int idxColor, SGNODE* aParent, double top, double bottom )
{
    vpcb.Tesselate( NULL );
    std::vector< double > vertices;
    std::vector< int > idxPlane;
    std::vector< int > idxSide;

    if( top < bottom )
    {
        double tmp = top;
        top = bottom;
        bottom = tmp;
    }

    if( !vpcb.Get3DTriangles( vertices, idxPlane, idxSide, top, bottom ) )
    {
        #ifdef DEBUG
        do {
            std::ostringstream ostr;
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [INFO] no vertex data";
            wxLogTrace( MASK_IDF, "%s\n", ostr.str().c_str() );
        } while( 0 );
        #endif

        return NULL;
    }

    if( ( idxPlane.size() % 3 ) || ( idxSide.size() % 3 ) )
    {
        #ifdef DEBUG
        do {
            std::ostringstream ostr;
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [BUG] index lists are not a multiple of 3 (not a triangle list)";
            wxLogTrace( MASK_IDF, "%s\n", ostr.str().c_str() );
        } while( 0 );
        #endif

        return NULL;
    }

    std::vector< SGPOINT > vlist;
    size_t nvert = vertices.size() / 3;
    size_t j = 0;

    for( size_t i = 0; i < nvert; ++i, j+= 3 )
        vlist.push_back( SGPOINT( vertices[j], vertices[j+1], vertices[j+2] ) );

    // create the intermediate scenegraph
    IFSG_TRANSFORM* tx0 = new IFSG_TRANSFORM( aParent );            // tx0 = Transform for this outline
    IFSG_SHAPE* shape = new IFSG_SHAPE( *tx0 );                     // shape will hold (a) all vertices and (b) a local list of normals
    IFSG_FACESET* face = new IFSG_FACESET( *shape );                // this face shall represent the top and bottom planes
    IFSG_COORDS* cp = new IFSG_COORDS( *face );                     // coordinates for all faces
    cp->SetCoordsList( nvert, &vlist[0] );
    IFSG_COORDINDEX* coordIdx = new IFSG_COORDINDEX( *face );       // coordinate indices for top and bottom planes only
    coordIdx->SetIndices( idxPlane.size(), &idxPlane[0] );
    IFSG_NORMALS* norms = new IFSG_NORMALS( *face );                // normals for the top and bottom planes

    // number of TOP (and bottom) vertices
    j = nvert / 2;

    // set the TOP normals
    for( size_t i = 0; i < j; ++i )
        norms->AddNormal( 0.0, 0.0, 1.0 );

    // set the BOTTOM normals
    for( size_t i = 0; i < j; ++i )
        norms->AddNormal( 0.0, 0.0, -1.0 );

    // assign a color from the palette
    SGNODE* modelColor = getColor( *shape, idxColor );

    // create a second shape describing the vertical walls of the IDF extrusion
    // using per-vertex-per-face-normals
    shape->NewNode( *tx0 );
    shape->AddRefNode( modelColor );    // set the color to be the same as the top/bottom
    face->NewNode( *shape );
    cp->NewNode( *face );               // new vertex list
    norms->NewNode( *face );            // new normals list
    coordIdx->NewNode( *face );         // new index list

    // populate the new per-face vertex list and its indices and normals
    std::vector< int >::iterator sI = idxSide.begin();
    std::vector< int >::iterator eI = idxSide.end();

    size_t sidx = 0;    // index to the new coord set
    SGPOINT p1, p2, p3;
    SGVECTOR vnorm;

    while( sI != eI )
    {
        p1 = vlist[*sI];
        cp->AddCoord( p1 );
        ++sI;

        p2 = vlist[*sI];
        cp->AddCoord( p2 );
        ++sI;

        p3 = vlist[*sI];
        cp->AddCoord( p3 );
        ++sI;

        vnorm.SetVector( S3D::CalcTriNorm( p1, p2, p3 ) );
        norms->AddNormal( vnorm );
        norms->AddNormal( vnorm );
        norms->AddNormal( vnorm );

        coordIdx->AddIndex( (int)sidx );
        ++sidx;
        coordIdx->AddIndex( (int)sidx );
        ++sidx;
        coordIdx->AddIndex( (int)sidx );
        ++sidx;
    }

    SCENEGRAPH* data = (SCENEGRAPH*)tx0->GetRawPtr();

    // delete the API wrappers
    delete shape;
    delete face;
    delete coordIdx;
    delete cp;
    delete tx0;

    return data;
}


static SCENEGRAPH* addOutline( IDF3_COMP_OUTLINE* outline, int idxColor, SGNODE* aParent )
{
    VRML_LAYER vpcb;

    if( !getOutlineModel( vpcb, outline->GetOutlines() ) )
    {
        #ifdef DEBUG
        do {
            std::ostringstream ostr;
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [INFO] no valid outline data";
            wxLogTrace( MASK_IDF, "%s\n", ostr.str().c_str() );
        } while( 0 );
        #endif

        return NULL;
    }

    vpcb.EnsureWinding( 0, false );

    double top = outline->GetThickness();
    double bot = 0.0;

    // note: some IDF entities permit negative heights
    if( top < bot )
    {
        bot = top;
        top = 0.0;
    }

    SCENEGRAPH* data = vrmlToSG( vpcb, idxColor, aParent, top, bot );

    return data;
}


static SCENEGRAPH* loadIDFOutline( const wxString& aFileName )
{
    LOCALESWITCH switcher;
    IDF3_BOARD brd( IDF3::CAD_ELEC );
    IDF3_COMP_OUTLINE* outline = NULL;

    outline = brd.GetComponentOutline( aFileName );

    if( NULL == outline )
    {
        #ifdef DEBUG
        do {
            std::ostringstream ostr;
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [INFO] Failed to read IDF data:\n";
            std::cerr << brd.GetError() << "\n\n";
            std::cerr << " * [INFO] no outline for file '";
            std::cerr << aFileName << "'";
            wxLogTrace( MASK_IDF, "%s\n", ostr.str().c_str() );
        } while( 0 );
        #endif

        return NULL;
    }

    SCENEGRAPH* data = addOutline( outline, -1, NULL );

    return data;
}


static SCENEGRAPH* loadIDFBoard( const wxString& aFileName )
{
    LOCALESWITCH switcher;
    IDF3_BOARD brd( IDF3::CAD_ELEC );

    // note: if the IDF model is defective no outline substitutes shall be made
    if( !brd.ReadFile( aFileName, true ) )
    {
        #ifdef DEBUG
        do {
            std::ostringstream ostr;
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [INFO] Failed to read IDF file:\n";
            std::cerr << brd.GetError() << "\n\n";
            std::cerr << " * [INFO] IDF file '" << aFileName.ToUTF8() << "'";
            wxLogTrace( MASK_IDF, "%s\n", ostr.str().c_str() );
        } while( 0 );
        #endif

        return NULL;
    }

    IFSG_TRANSFORM tx0( true );
    SGNODE* topNode = tx0.GetRawPtr();

    bool noBoard = false;
    bool noComp = false;
    bool noOther = false;

    if( NULL == makeBoard( brd, topNode ) )
        noBoard = true;

    if( !makeComponents( brd, topNode ) )
        noComp = true;

    if( !makeOtherOutlines( brd, topNode ) )
        noOther = true;

    if( noBoard && noComp && noOther )
    {
        tx0.Destroy();
        return NULL;
    }

    return (SCENEGRAPH*) topNode;
}


static SCENEGRAPH* makeBoard( IDF3_BOARD& brd, SGNODE* aParent )
{
    if( NULL == aParent )
        return NULL;

    VRML_LAYER vpcb;

    // check if no board outline
    if( brd.GetBoardOutlinesSize() < 1 )
        return NULL;


    if( !getOutlineModel( vpcb, brd.GetBoardOutline()->GetOutlines() ) )
        return NULL;

    vpcb.EnsureWinding( 0, false );

    int nvcont = vpcb.GetNContours() - 1;

    while( nvcont > 0 )
        vpcb.EnsureWinding( nvcont--, true );

    // Add the drill holes
    const std::list<IDF_DRILL_DATA*>* drills = &brd.GetBoardDrills();

    std::list<IDF_DRILL_DATA*>::const_iterator sd = drills->begin();
    std::list<IDF_DRILL_DATA*>::const_iterator ed = drills->end();

    while( sd != ed )
    {
        vpcb.AddCircle( (*sd)->GetDrillXPos(), (*sd)->GetDrillYPos(),
            (*sd)->GetDrillDia() / 2.0, true );
        ++sd;
    }

    std::map< std::string, IDF3_COMPONENT* >*const comp = brd.GetComponents();
    std::map< std::string, IDF3_COMPONENT* >::const_iterator sc = comp->begin();
    std::map< std::string, IDF3_COMPONENT* >::const_iterator ec = comp->end();

    while( sc != ec )
    {
        drills = sc->second->GetDrills();
        sd = drills->begin();
        ed = drills->end();

        while( sd != ed )
        {
            vpcb.AddCircle( (*sd)->GetDrillXPos(), (*sd)->GetDrillYPos(),
                (*sd)->GetDrillDia() / 2.0, true );
            ++sd;
        }

        ++sc;
    }

    double top = brd.GetBoardThickness();

    SCENEGRAPH* data = vrmlToSG( vpcb, 0, aParent, top, 0.0 );

    return data;
}


static bool makeComponents( IDF3_BOARD& brd, SGNODE* aParent )
{
    if( NULL == aParent )
        return false;

    int ncomponents = 0;

    double brdTop = brd.GetBoardThickness();

    // Add the component outlines
    const std::map< std::string, IDF3_COMPONENT* >*const comp = brd.GetComponents();
    std::map< std::string, IDF3_COMPONENT* >::const_iterator sc = comp->begin();
    std::map< std::string, IDF3_COMPONENT* >::const_iterator ec = comp->end();

    std::list< IDF3_COMP_OUTLINE_DATA* >::const_iterator so;
    std::list< IDF3_COMP_OUTLINE_DATA* >::const_iterator eo;

    double vX, vY, vA;
    double tX, tY, tZ, tA;
    bool   bottom;
    IDF3::IDF_LAYER lyr;

    std::map< std::string, SGNODE* > dataMap;    // map data by UID
    std::map< std::string, SGNODE* >::iterator dataItem;
    IDF3_COMP_OUTLINE* pout;

    while( sc != ec )
    {
        sc->second->GetPosition( vX, vY, vA, lyr );

        if( lyr == IDF3::LYR_BOTTOM )
            bottom = true;
        else
            bottom = false;

        so = sc->second->GetOutlinesData()->begin();
        eo = sc->second->GetOutlinesData()->end();

        while( so != eo )
        {
            if( std::abs( (*so)->GetOutline()->GetThickness() ) < 0.001 )
            {
                ++so;
                continue;
            }

            (*so)->GetOffsets( tX, tY, tZ, tA );
            tX += vX;
            tY += vY;
            tA += vA;

            pout = (IDF3_COMP_OUTLINE*)((*so)->GetOutline());

            if( NULL == pout  )
            {
                ++so;
                continue;
            }

            dataItem = dataMap.find( pout->GetUID() );
            SCENEGRAPH* sg = NULL;

            if( dataItem == dataMap.end() )
            {
                sg = addOutline( pout, -1, NULL );

                if( NULL == sg )
                {
                    ++so;
                    continue;
                }

                ++ncomponents;
                dataMap.insert( std::pair< std::string, SGNODE* >
                    ( pout->GetUID(), (SGNODE*)sg ) );
            }
            else
            {
                sg = (SCENEGRAPH*) dataItem->second;
            }

            IFSG_TRANSFORM tx0( aParent );
            IFSG_TRANSFORM txN( false );
            txN.Attach( (SGNODE*)sg );

            if( NULL == txN.GetParent() )
                tx0.AddChildNode( txN );
            else
                tx0.AddRefNode( txN );

            if( bottom )
            {
                tx0.SetTranslation( SGPOINT( tX, tY, -tZ ) );
                // for an item on the back of the board we have a  compounded rotation,
                // first a flip on the Y axis as per the IDF spec and then a rotation
                // of -tA degrees on the Z axis. The resultant rotation axis is an
                // XY vector equivalent to (0,1) rotated by -(tA/2) degrees
                //
                double ang = -tA * M_PI / 360.0;
                double sinA = sin( ang );
                double cosA = cos( ang );
                tx0.SetRotation( SGVECTOR( -sinA, cosA , 0 ), M_PI );
            }
            else
            {
                tx0.SetTranslation( SGPOINT( tX, tY, tZ + brdTop ) );
                tx0.SetRotation( SGVECTOR( 0, 0, 1 ), tA * M_PI / 180.0 );
            }

            ++so;
        }

        ++sc;
    }

    if( 0 == ncomponents )
        return false;

    return true;
}


static bool makeOtherOutlines( IDF3_BOARD& brd, SGNODE* aParent )
{
    if( NULL == aParent )
        return false;

    VRML_LAYER vpcb;
    int ncomponents = 0;

    double brdTop = brd.GetBoardThickness();
    double top, bot;

    // Add the component outlines
    const std::map< std::string, OTHER_OUTLINE* >*const comp = brd.GetOtherOutlines();
    std::map< std::string, OTHER_OUTLINE* >::const_iterator sc = comp->begin();
    std::map< std::string, OTHER_OUTLINE* >::const_iterator ec = comp->end();

    int nvcont;

    OTHER_OUTLINE* pout;

    while( sc != ec )
    {
        pout = sc->second;

        if( std::abs( pout->GetThickness() ) < 0.001 )
        {
            ++sc;
            continue;
        }

        if( !getOutlineModel( vpcb, pout->GetOutlines() ) )
        {
            vpcb.Clear();
            ++sc;
            continue;
        }

        vpcb.EnsureWinding( 0, false );

        nvcont = vpcb.GetNContours() - 1;

        while( nvcont > 0 )
            vpcb.EnsureWinding( nvcont--, true );

        if( pout->GetSide() == IDF3::LYR_BOTTOM )
        {
            top = 0.0;
            bot = -pout->GetThickness();
        }
        else
        {
            bot = brdTop;
            top = bot + pout->GetThickness();
        }

        if( NULL == vrmlToSG( vpcb, -1, aParent, top, bot ) )
        {
            vpcb.Clear();
            ++sc;
            continue;
        }

        ++ncomponents;

        vpcb.Clear();
        ++sc;
    }

    if( 0 == ncomponents )
        return false;

    return true;
}
