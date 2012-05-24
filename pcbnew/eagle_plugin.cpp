
/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 SoftPLC Corporation, Dick Hollenbeck <dick@softplc.com>
 * Copyright (C) 2012 KiCad Developers, see change_log.txt for contributors.

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


/*

Pcbnew PLUGIN for Eagle 6.x XML *.brd and footprint format.

XML parsing and converting:
Getting line numbers and byte offsets from the source XML file is not
possible using currently available XML libraries within KiCad project:
wxXmlDocument and boost::property_tree.

property_tree will give line numbers but no byte offsets, and only during
document loading. This means that if we have a problem after the document is
successfully loaded, there is no way to correlate back to line number and byte
offset of the problem. So a different approach is taken, one which relies on the
XML elements themselves using an XPATH type of reporting mechanism. The path to
the problem is reported in the error messages. This means keeping track of that
path as we traverse the XML document for the sole purpose of accurate error
reporting.

User can load the source XML file into firefox or other xml browser and follow
our error message.

Load() TODO's

*) finish xpath support
*) set layer counts, types and names into BOARD
*) fix text twisting and final location issues.
*) net info
*) netclass info?
*) code factoring, for polygon at least


*/

#include <errno.h>

#include <wx/string.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <eagle_plugin.h>

#include <common.h>
#include <macros.h>
#include <fctsys.h>
#include <trigo.h>

#include <class_board.h>
#include <class_module.h>
#include <class_track.h>
#include <class_edge_mod.h>
#include <class_zone.h>
#include <class_pcb_text.h>

using namespace boost::property_tree;


typedef EAGLE_PLUGIN::BIU                   BIU;
typedef PTREE::const_assoc_iterator         CA_ITER;
typedef PTREE::const_iterator               CITER;
typedef std::pair<CA_ITER, CA_ITER>         CA_ITER_RANGE;

typedef MODULE_MAP::iterator                MODULE_ITER;
typedef MODULE_MAP::const_iterator          MODULE_CITER;

typedef boost::optional<std::string>        opt_string;
typedef boost::optional<int>                opt_int;
typedef boost::optional<double>             opt_double;
typedef boost::optional<CPTREE&>            opt_cptree;

/// Eagle wire
struct EWIRE
{
    double  x1;
    double  y1;
    double  x2;
    double  y2;
    double  width;
    int     layer;
};

/// Eagle via
struct EVIA
{
    double      x;
    double      y;
    int         layer_start;        /// < extent
    int         layer_end;          /// < inclusive
    double      drill;
    opt_double  diam;
    opt_string  shape;
};

/// Eagle circle
struct ECIRCLE
{
    double  x;
    double  y;
    double  radius;
    double  width;
    int     layer;
};


/// Eagle XML rectangle in binary
struct ERECT
{
    double  x1;
    double  y1;
    double  x2;
    double  y2;
    int     layer;
};


/// Eagle rotation
struct EROT
{
    bool    mirror;
    bool    spin;
    double  degrees;
};

typedef boost::optional<EROT>   opt_erot;


/// Eagle "attribute" XML element, no foolin'.
struct EATTR
{
    std::string name;
    opt_string  value;
    opt_double  x;
    opt_double  y;
    opt_double  size;
    // opt_int  layer;
    opt_double  ratio;
    opt_erot    erot;
    opt_int     display;

    enum {  // for 'display' field above
        Off,
        VALUE,
        NAME,
        BOTH,
    };
};

/// Eagle text element
struct ETEXT
{
    std::string text;
    double      x;
    double      y;
    double      size;
    int         layer;
    opt_string  font;
    opt_double  ratio;
    opt_erot    erot;
    opt_int     align;

    enum {
        CENTER,
        CENTER_LEFT,
        TOP_CENTER,
        TOP_LEFT,
        TOP_RIGHT,

        // opposites are -1 * above:
        CENTER_RIGHT  = -CENTER_LEFT,
        BOTTOM_CENTER = -TOP_CENTER,
        BOTTOM_LEFT   = -TOP_RIGHT,
        BOTTOM_RIGHT  = -TOP_LEFT,
    };
};


/// Assemble a MODULE factory key as a simple concatonation of library name and
/// package name, using '\x02' as a separator.
static inline std::string makePkgKey( const std::string& aLibName, const std::string& aPkgName )
{
    std::string key = aLibName + '\x02' +  aPkgName;
    return key;
}

/// Make a unique time stamp, in this case from a unique tree memory location
static inline unsigned long timeStamp( CPTREE& aTree )
{
    return (unsigned long)(void*) &aTree;
}


EAGLE_PLUGIN::EAGLE_PLUGIN()
{
    init( NULL );
}


EAGLE_PLUGIN::~EAGLE_PLUGIN()
{
}


const wxString& EAGLE_PLUGIN::PluginName() const
{
    static const wxString name = wxT( "Eagle" );
    return name;
}


const wxString& EAGLE_PLUGIN::GetFileExtension() const
{
    static const wxString extension = wxT( "brd" );
    return extension;
}


int inline EAGLE_PLUGIN::kicad( double d ) const
{
    return KiROUND( biu_per_mm * d );
}


wxSize inline EAGLE_PLUGIN::kicad_fontz( double d ) const
{
    // texts seem to better match eagle when scaled down by 0.95
    int kz = kicad( d ) * 95 / 100;
    return wxSize( kz, kz );
}


BOARD* EAGLE_PLUGIN::Load( const wxString& aFileName, BOARD* aAppendToMe,  PROPERTIES* aProperties )
{
    LOCALE_IO   toggle;     // toggles on, then off, the C locale.
    PTREE       doc;

    init( aProperties );

    m_board = aAppendToMe ? aAppendToMe : new BOARD();

    // delete on exception, iff I own m_board, according to aAppendToMe
    auto_ptr<BOARD> deleter( aAppendToMe ? NULL : m_board );

    try
    {
        // 8 bit filename should be encoded in current locale, not necessarily utf8.
        std::string filename = (const char*) aFileName.fn_str();

        read_xml( filename, doc, xml_parser::trim_whitespace | xml_parser::no_comments );

        std::string xpath = "eagle.drawing.board";
        CPTREE&     brd   = doc.get_child( xpath );

        loadAllSections( brd, xpath, bool( aAppendToMe ) );
    }

    // Class ptree_error is a base class for xml_parser_error & file_parser_error,
    // so one catch should be OK for all errors.
    catch( ptree_error pte )
    {
        // for xml_parser_error, what() has the line number in it,
        // but no byte offset.  That should be an adequate error message.
        THROW_IO_ERROR( pte.what() );
    }

    // IO_ERROR exceptions are left uncaught, they pass upwards from here.

    deleter.release();
    return m_board;
}


void EAGLE_PLUGIN::Save( const wxString& aFileName, BOARD* aBoard, PROPERTIES* aProperties )
{
}


void EAGLE_PLUGIN::init( PROPERTIES* aProperties )
{
    m_templates.clear();

    m_board = NULL;
    m_props = aProperties;

    mm_per_biu = 1/IU_PER_MM;
    biu_per_mm = IU_PER_MM;
}

/*
int EAGLE_PLUGIN::biuSprintf( char* buf, BIU aValue ) const
{
    double  engUnits = mm_per_biu * aValue;
    int     len;

    if( engUnits != 0.0 && fabs( engUnits ) <= 0.0001 )
    {
        // printf( "f: " );
        len = sprintf( buf, "%.10f", engUnits );

        while( --len > 0 && buf[len] == '0' )
            buf[len] = '\0';

        ++len;
    }
    else
    {
        // printf( "g: " );
        len = sprintf( buf, "%.10g", engUnits );
    }
    return len;
}


std::string EAGLE_PLUGIN::fmtBIU( BIU aValue ) const
{
    char    temp[50];

    int len = biuSprintf( temp, aValue );

    return std::string( temp, len );
}

*/

void EAGLE_PLUGIN::loadAllSections( CPTREE& aEagleBoard, const std::string& aXpath, bool aAppendToMe )
{
    std::string xpath;

    {
        xpath = aXpath + '.' + "plain";
        CPTREE& plain = aEagleBoard.get_child( "plain" );
        loadPlain( plain, xpath );
    }

    {
        xpath = aXpath + '.' + "libraries";
        CPTREE&  libs = aEagleBoard.get_child( "libraries" );
        loadLibraries( libs, xpath );
    }

    {
        xpath = aXpath + '.' + "elements";
        CPTREE& elems = aEagleBoard.get_child( "elements" );
        loadElements( elems, xpath );
    }

    {
        xpath = aXpath + '.' + "signals";
        CPTREE&  signals = aEagleBoard.get_child( "signals" );
        loadSignals( signals, xpath );
    }
}


void EAGLE_PLUGIN::loadPlain( CPTREE& aGraphics, const std::string& aXpath )
{
    // (polygon | wire | text | circle | rectangle | frame | hole)*
    for( CITER gr = aGraphics.begin();  gr != aGraphics.end();  ++gr )
    {
        if( !gr->first.compare( "wire" ) )
        {
            EWIRE w = ewire( gr->second );

            DRAWSEGMENT* dseg = new DRAWSEGMENT( m_board );
            m_board->Add( dseg, ADD_APPEND );

            dseg->SetTimeStamp( timeStamp( gr->second ) );
            dseg->SetLayer( kicad_layer( w.layer ) );
            dseg->SetStart( wxPoint( kicad_x( w.x1 ), kicad_y( w.y1 ) ) );
            dseg->SetEnd( wxPoint( kicad_x( w.x2 ), kicad_y( w.y2 ) ) );
            dseg->SetWidth( kicad( w.width ) );
        }
        else if( !gr->first.compare( "text" ) )
        {
            double  ratio = 6;
            int     sign = 1;

#if defined(DEBUG)
            if( !gr->second.data().compare( "designed by" ) )
            {
                int breakhere = 1;
                (void) breakhere;
            }
#endif

            ETEXT   t = etext( gr->second );

            TEXTE_PCB* pcbtxt = new TEXTE_PCB( m_board );
            m_board->Add( pcbtxt, ADD_APPEND );

            pcbtxt->SetTimeStamp( timeStamp( gr->second ) );
            pcbtxt->SetText( FROM_UTF8( t.text.c_str() ) );
            pcbtxt->SetPosition( wxPoint( kicad_x( t.x ), kicad_y( t.y ) ) );
            pcbtxt->SetLayer( kicad_layer( t.layer ) );

            pcbtxt->SetSize( kicad_fontz( t.size ) );

            if( t.ratio )
                ratio = *t.ratio;

            pcbtxt->SetThickness( kicad( t.size * ratio / 100 ) );

            if( t.erot )
            {
                // eagles does not rotate text spun to 180 degrees unless spin is set.
                if( t.erot->spin || t.erot->degrees != 180 )
                    pcbtxt->SetOrientation( t.erot->degrees * 10 );

                else    // 180 degree no spin text, flip the justification to opposite
                    sign = -1;

                pcbtxt->SetMirrored( t.erot->mirror );
            }

            int align = t.align ? *t.align : ETEXT::BOTTOM_LEFT;

            switch( align * sign )  // if negative, opposite is chosen
            {
            case ETEXT::CENTER:
                // this was the default in pcbtxt's constructor
                break;

            case ETEXT::CENTER_LEFT:
                pcbtxt->SetHorizJustify( GR_TEXT_HJUSTIFY_LEFT );
                break;

            case ETEXT::CENTER_RIGHT:
                pcbtxt->SetHorizJustify( GR_TEXT_HJUSTIFY_RIGHT );
                break;

            case ETEXT::TOP_CENTER:
                pcbtxt->SetVertJustify( GR_TEXT_VJUSTIFY_TOP );
                break;

            case ETEXT::TOP_LEFT:
                pcbtxt->SetHorizJustify( GR_TEXT_HJUSTIFY_LEFT );
                pcbtxt->SetVertJustify( GR_TEXT_VJUSTIFY_TOP );
                break;

            case ETEXT::TOP_RIGHT:
                pcbtxt->SetHorizJustify( GR_TEXT_HJUSTIFY_RIGHT );
                pcbtxt->SetVertJustify( GR_TEXT_VJUSTIFY_TOP );
                break;

            case ETEXT::BOTTOM_CENTER:
                pcbtxt->SetVertJustify( GR_TEXT_VJUSTIFY_BOTTOM );
                break;

            case ETEXT::BOTTOM_LEFT:
                pcbtxt->SetHorizJustify( GR_TEXT_HJUSTIFY_LEFT );
                pcbtxt->SetVertJustify( GR_TEXT_VJUSTIFY_BOTTOM );
                break;

            case ETEXT::BOTTOM_RIGHT:
                pcbtxt->SetHorizJustify( GR_TEXT_HJUSTIFY_RIGHT );
                pcbtxt->SetVertJustify( GR_TEXT_VJUSTIFY_BOTTOM );
                break;
            }
        }
        else if( !gr->first.compare( "circle" ) )
        {
            ECIRCLE c = ecircle( gr->second );

            DRAWSEGMENT* dseg = new DRAWSEGMENT( m_board );
            m_board->Add( dseg, ADD_APPEND );

            dseg->SetShape( S_CIRCLE );
            dseg->SetTimeStamp( timeStamp( gr->second ) );
            dseg->SetLayer( kicad_layer( c.layer ) );
            dseg->SetStart( wxPoint( kicad_x( c.x ), kicad_y( c.y ) ) );
            dseg->SetEnd( wxPoint( kicad_x( c.x + c.radius ), kicad_y( c.y ) ) );
            dseg->SetWidth( kicad( c.width ) );
        }

        // This seems to be a simplified rectangular [copper] zone, cannot find any
        // net related info on it from the DTD.
        else if( !gr->first.compare( "rectangle" ) )
        {
#if 0
            ERECT   r = erect( gr->second );
            int     layer = kicad_layer( r.layer );

            // hope the angle of rotation is zero.


            // might be better off making this into a ZONE:

            if( IsValidCopperLayerIndex( layer ) )
            {
                auto_ptr<DRAWSEGMENT> dseg = new DRAWSEGMENT( m_board );

                dseg->SetTimeStamp( timeStamp( gr->second ) );
                dseg->SetLayer( layer );
                dseg->SetShape( S_POLYGON );
                dseg->SetWidth( Mils2iu( 12 ) );

                std::vector<wxPoint>    pts;

                pts.push_back( wxPoint( kicad_x( r.x1 ), kicad_y( r.y1 ) ) );
                pts.push_back( wxPoint( kicad_x( r.x2 ), kicad_y( r.y1 ) ) );
                pts.push_back( wxPoint( kicad_x( r.x2 ), kicad_y( r.y2 ) ) );
                pts.push_back( wxPoint( kicad_x( r.x1 ), kicad_y( r.y2 ) ) );
                dseg->SetPolyPoints( pts );

                m_board->Add( dseg.release(), ADD_APPEND );
            }
#elif 0
            auto_ptr<ZONE_CONTAINER> zone = new ZONE_CONTAINER( m_board );

            ;
            m_board->Add( zone.release(), ADD_APPEND );
#endif
        }
        else if( !gr->first.compare( "hole" ) )
        {
        }
        else if( !gr->first.compare( "frame" ) )
        {
        }
        else if( !gr->first.compare( "polygon" ) )
        {
        }
    }
}


void EAGLE_PLUGIN::loadLibraries( CPTREE& aLibs, const std::string& aXpath )
{
    for( CITER library = aLibs.begin();  library != aLibs.end();  ++library )
    {
        const std::string& lib_name = library->second.get<std::string>( "<xmlattr>.name" );

        // library will have <xmlattr> node, skip that and get the packages node
        CPTREE& packages = library->second.get_child( "packages" );

        // Create a MODULE for all the eagle packages, for use later via a copy constructor
        // to instantiate needed MODULES in our BOARD.  Save the MODULE templates in
        // a MODULE_MAP using a single lookup key consisting of libname+pkgname.

        for( CITER package = packages.begin();  package != packages.end();  ++package )
        {
            const std::string& pack_name = package->second.get<std::string>( "<xmlattr>.name" );

#if defined(DEBUG)
            if( !pack_name.compare( "TO220H" ) )
            {
                int breakhere = 1;
                (void) breakhere;
            }
#endif

            std::string key = makePkgKey( lib_name, pack_name );

            MODULE* m = makeModule( package->second, pack_name );

            // add the templating MODULE to the MODULE template factory "m_templates"
            std::pair<MODULE_ITER, bool> r = m_templates.insert( key, m );

            if( !r.second )
            {
                wxString lib = FROM_UTF8( lib_name.c_str() );
                wxString pkg = FROM_UTF8( pack_name.c_str() );

                wxString emsg = wxString::Format(
                    _( "<package> name:'%s' duplicated in eagle <library>:'%s'" ),
                    GetChars( pkg ),
                    GetChars( lib )
                    );
                THROW_IO_ERROR( emsg );
            }
        }
    }
}


void EAGLE_PLUGIN::loadElements( CPTREE& aElements, const std::string& aXpath )
{
    for( CITER it = aElements.begin();  it != aElements.end();  ++it )
    {
        if( it->first.compare( "element" ) )
            continue;

        CPTREE& attrs = it->second.get_child( "<xmlattr>" );

        /*

        a '*' means zero or more times

        <!ELEMENT element (attribute*, variant*)>
        <!ATTLIST element
            name          %String;       #REQUIRED
            library       %String;       #REQUIRED
            package       %String;       #REQUIRED
            value         %String;       #REQUIRED
            x             %Coord;        #REQUIRED
            y             %Coord;        #REQUIRED
            locked        %Bool;         "no"
            smashed       %Bool;         "no"
            rot           %Rotation;     "R0"
            >
        */

        std::string name    = attrs.get<std::string>( "name" );
        std::string library = attrs.get<std::string>( "library" );
        std::string package = attrs.get<std::string>( "package" );
        std::string value   = attrs.get<std::string>( "value" );

#if 1 && defined(DEBUG)
        if( !name.compare( "GROUND" ) )
        {
            int breakhere = 1;
            (void) breakhere;
        }
#endif

        double x = attrs.get<double>( "x" );
        double y = attrs.get<double>( "y" );

        opt_string rot = attrs.get_optional<std::string>( "rot" );

        std::string key = makePkgKey( library, package );

        MODULE_CITER mi = m_templates.find( key );

        if( mi == m_templates.end() )
        {
            wxString emsg = wxString::Format( _( "No '%s' package in library '%s'" ),
                GetChars( FROM_UTF8( package.c_str() ) ),
                GetChars( FROM_UTF8( library.c_str() ) ) );
            THROW_IO_ERROR( emsg );
        }

        // copy constructor to clone the template
        MODULE* m = new MODULE( *mi->second );
        m_board->Add( m, ADD_APPEND );

        m->SetPosition( wxPoint( kicad_x( x ), kicad_y( y ) ) );
        m->SetReference( FROM_UTF8( name.c_str() ) );
        m->SetValue( FROM_UTF8( value.c_str() ) );
        // m->Value().SetVisible( false );

        if( rot )
        {
            EROT r = erot( *rot );

            m->SetOrientation( r.degrees * 10 );

            if( r.mirror )
            {
                m->Flip( m->GetPosition() );
            }
        }

        // VALUE and NAME can have something like our text "effects" overrides
        // in SWEET and new schematic.  Eagle calls these XML elements "attribute".
        // There can be one for NAME and/or VALUE both.
        CA_ITER_RANGE attributes = it->second.equal_range( "attribute" );
        for( CA_ITER ait = attributes.first;  ait != attributes.second;  ++ait )
        {
            double  ratio = 6;
            EATTR   a = eattr( ait->second );

            TEXTE_MODULE*   txt;

            if( !a.name.compare( "NAME" ) )
                txt = &m->Reference();
            else    // "VALUE" or else our understanding of file format is incomplete.
                txt = &m->Value();

            if( a.value )
            {
                txt->SetText( FROM_UTF8( a.value->c_str() ) );
            }

            if( a.x && a.y )    // boost::optional
            {
                wxPoint pos( kicad_x( *a.x ), kicad_y( *a.y ) );
                wxPoint pos0 = pos - m->GetPosition();

                txt->SetPosition( pos );
                txt->SetPos0( pos0 );
            }

            if( a.ratio )
                ratio = *a.ratio;

            if( a.size )
            {
                wxSize  fontz = kicad_fontz( *a.size );
                txt->SetSize( fontz );

                int     lw = int( fontz.y * ratio / 100.0 );
                txt->SetThickness( lw );
            }

            if( a.erot )
            {
                double angle = a.erot->degrees * 10;

                if( angle != 1800 )
                {
                    angle -= m->GetOrientation();   // subtract module's angle
                    txt->SetOrientation( angle );
                }
                else
                {
                    txt->SetHorizJustify( GR_TEXT_HJUSTIFY_RIGHT );
                    txt->SetVertJustify( GR_TEXT_VJUSTIFY_TOP );
                }
            }
        }
    }
}


EWIRE EAGLE_PLUGIN::ewire( CPTREE& aWire ) const
{
    EWIRE   w;
    CPTREE& attribs = aWire.get_child( "<xmlattr>" );

    w.x1    = attribs.get<double>( "x1" );
    w.y1    = attribs.get<double>( "y1" );
    w.x2    = attribs.get<double>( "x2" );
    w.y2    = attribs.get<double>( "y2" );
    w.width = attribs.get<double>( "width" );
    w.layer = attribs.get<int>( "layer" );
    return w;
}


EVIA EAGLE_PLUGIN::evia( CPTREE& aVia ) const
{
    EVIA    v;
    CPTREE& attribs = aVia.get_child( "<xmlattr>" );

    /*
    <!ELEMENT via EMPTY>
    <!ATTLIST via
          x             %Coord;        #REQUIRED
          y             %Coord;        #REQUIRED
          extent        %Extent;       #REQUIRED
          drill         %Dimension;    #REQUIRED
          diameter      %Dimension;    "0"
          shape         %ViaShape;     "round"
          alwaysstop    %Bool;         "no"
          >
    */

    v.x     = attribs.get<double>( "x" );
    v.y     = attribs.get<double>( "y" );

    std::string ext = attribs.get<std::string>( "extent" );

    sscanf( ext.c_str(), "%u-%u", &v.layer_start, &v.layer_end );

    v.drill = attribs.get<double>( "drill" );
    v.diam  = attribs.get_optional<double>( "diameter" );
    v.shape = attribs.get_optional<std::string>( "shape" );
    return v;
}


ECIRCLE EAGLE_PLUGIN::ecircle( CPTREE& aCircle ) const
{
    ECIRCLE c;
    CPTREE& attribs = aCircle.get_child( "<xmlattr>" );

    c.x      = attribs.get<double>( "x" );
    c.y      = attribs.get<double>( "y" );
    c.radius = attribs.get<double>( "radius" );
    c.width  = attribs.get<double>( "width" );
    c.layer  = attribs.get<int>( "layer" );
    return c;
}


ERECT EAGLE_PLUGIN::erect( CPTREE& aRect ) const
{
    ERECT   r;
    CPTREE& attribs = aRect.get_child( "<xmlattr>" );

    /*
    <!ELEMENT rectangle EMPTY>
    <!ATTLIST rectangle
          x1            %Coord;        #REQUIRED
          y1            %Coord;        #REQUIRED
          x2            %Coord;        #REQUIRED
          y2            %Coord;        #REQUIRED
          layer         %Layer;        #REQUIRED
          rot           %Rotation;     "R0"
          >
    */

    r.x1     = attribs.get<double>( "x1" );
    r.y1     = attribs.get<double>( "y1" );
    r.x2     = attribs.get<double>( "x2" );
    r.y2     = attribs.get<double>( "y2" );
    r.layer  = attribs.get<int>( "layer" );

    // @todo: hoping that rot is not used

    return r;
}


ETEXT EAGLE_PLUGIN::etext( CPTREE& aText ) const
{
    ETEXT   t;
    CPTREE& attribs = aText.get_child( "<xmlattr>" );

    /*
    <!ELEMENT text (#PCDATA)>
    <!ATTLIST text
          x             %Coord;        #REQUIRED
          y             %Coord;        #REQUIRED
          size          %Dimension;    #REQUIRED
          layer         %Layer;        #REQUIRED
          font          %TextFont;     "proportional"
          ratio         %Int;          "8"
          rot           %Rotation;     "R0"
          align         %Align;        "bottom-left"
          >
    */

    t.text   = aText.data();
    t.x      = attribs.get<double>( "x" );
    t.y      = attribs.get<double>( "y" );
    t.size   = attribs.get<double>( "size" );
    t.layer  = attribs.get<int>( "layer" );

    t.font   = attribs.get_optional<std::string>( "font" );
    t.ratio  = attribs.get_optional<double>( "ratio" );

    opt_string rot = attribs.get_optional<std::string>( "rot" );
    if( rot )
    {
        t.erot = erot( *rot );
    }

    opt_string align = attribs.get_optional<std::string>( "align" );
    if( align )
    {
        // (bottom-left | bottom-center | bottom-right | center-left |
        //   center | center-right | top-left | top-center | top-right)
        if( !align->compare( "center" ) )
            *t.align = ETEXT::CENTER;
        else if( !align->compare( "center-right" ) )
            *t.align = ETEXT::CENTER_RIGHT;
        else if( !align->compare( "top-left" ) )
            *t.align = ETEXT::TOP_LEFT;
        else if( !align->compare( "top-center" ) )
            *t.align = ETEXT::TOP_CENTER;
        else if( !align->compare( "top-right" ) )
            *t.align = ETEXT::TOP_RIGHT;
        else if( !align->compare( "bottom-left" ) )
            *t.align = ETEXT::BOTTOM_LEFT;
        else if( !align->compare( "bottom-center" ) )
            *t.align = ETEXT::BOTTOM_CENTER;
        else if( !align->compare( "bottom-right" ) )
            *t.align = ETEXT::BOTTOM_RIGHT;
        else if( !align->compare( "center-left" ) )
            *t.align = ETEXT::CENTER_LEFT;
    }

    return t;
}


EROT EAGLE_PLUGIN::erot( const std::string& aRot ) const
{
    EROT    rot;

    rot.spin    = aRot.find( 'S' ) != aRot.npos;
    rot.mirror  = aRot.find( 'M' ) != aRot.npos;
    rot.degrees = strtod( aRot.c_str() + 1 + int( rot.spin || rot.mirror ), NULL );

    return rot;
}


EATTR EAGLE_PLUGIN::eattr( CPTREE& aAttribute ) const
{
    EATTR   a;
    CPTREE& attribs = aAttribute.get_child( "<xmlattr>" );

    /*
    <!ELEMENT attribute EMPTY>
    <!ATTLIST attribute
      name          %String;       #REQUIRED
      value         %String;       #IMPLIED
      x             %Coord;        #IMPLIED
      y             %Coord;        #IMPLIED
      size          %Dimension;    #IMPLIED
      layer         %Layer;        #IMPLIED
      font          %TextFont;     #IMPLIED
      ratio         %Int;          #IMPLIED
      rot           %Rotation;     "R0"
      display       %AttributeDisplay; "value" -- only in <element> or <instance> context --
      constant      %Bool;         "no"     -- only in <device> context --
      >
    */

    a.name    = attribs.get<std::string>( "name" );                    // #REQUIRED
    a.value   = attribs.get_optional<std::string>( "value" );

    a.x       = attribs.get_optional<double>( "x" );
    a.y       = attribs.get_optional<double>( "y" );

    // KiCad cannot currently put a TEXTE_MODULE on a different layer than the MODULE
    // Eagle can it seems.  Skip layer.

    a.size    = attribs.get_optional<double>( "size" );
    a.ratio   = attribs.get_optional<double>( "ratio" );

    opt_string rot = attribs.get_optional<std::string>( "rot" );
    if( rot )
    {
        a.erot = erot( *rot );
    }

    opt_string display = attribs.get_optional<std::string>( "display" );
    if( display )
    {
        // (off | value | name | both)
        if( !display->compare( "off" ) )
            a.display = EATTR::Off;
        else if( !display->compare( "value" ) )
            a.display = EATTR::VALUE;
        else if( !display->compare( "name" ) )
            a.display = EATTR::NAME;
        else if( !display->compare( "both" ) )
            a.display = EATTR::BOTH;
    }

    return a;
}


MODULE* EAGLE_PLUGIN::makeModule( CPTREE& aPackage, const std::string& aPkgName ) const
{
    std::auto_ptr<MODULE>   m( new MODULE( NULL ) );

    m->SetLibRef( FROM_UTF8( aPkgName.c_str() ) );

    opt_string description = aPackage.get_optional<std::string>( "description" );
    if( description )
        m->SetDescription( FROM_UTF8( description->c_str() ) );

    for( CITER it = aPackage.begin();  it != aPackage.end();  ++it )
    {
        CPTREE& t = it->second;

        if( it->first.compare( "wire" ) == 0 )
            packageWire( m.get(), t );

        else if( !it->first.compare( "pad" ) )
            packagePad( m.get(), t );

        else if( !it->first.compare( "text" ) )
            packageText( m.get(), t );

        else if( !it->first.compare( "rectangle" ) )
            packageRectangle( m.get(), t );

        else if( !it->first.compare( "polygon" ) )
            packagePolygon( m.get(), t );

        else if( !it->first.compare( "circle" ) )
            packageCircle( m.get(), t );

        else if( !it->first.compare( "hole" ) )
            packageHole( m.get(), t );

        else if( !it->first.compare( "smd" ) )
            packageSMD( m.get(), t );
    }

    return m.release();
}


void EAGLE_PLUGIN::packageWire( MODULE* aModule, CPTREE& aTree ) const
{
    EWIRE   w = ewire( aTree );
    int     layer = kicad_layer( w.layer );

    if( IsValidNonCopperLayerIndex( layer ) )  // skip copper package wires
    {
        wxPoint start( kicad_x( w.x1 ), kicad_y( w.y1 ) );
        wxPoint end(   kicad_x( w.x2 ), kicad_y( w.y2 ) );
        int     width = kicad( w.width );

        EDGE_MODULE* dwg = new EDGE_MODULE( aModule, S_SEGMENT );
        aModule->m_Drawings.PushBack( dwg );

        dwg->SetStart0( start );
        dwg->SetEnd0( end );
        dwg->SetLayer( layer );
        dwg->SetWidth( width );
    }
}


void EAGLE_PLUGIN::packagePad( MODULE* aModule, CPTREE& aTree ) const
{
    // pay for this tree traversal only once
    CPTREE& attrs = aTree.get_child( "<xmlattr>" );

    /* from <ealge>/doc/eagle.dtd

    <!ELEMENT pad EMPTY>
    <!ATTLIST pad
          name          %String;       #REQUIRED
          x             %Coord;        #REQUIRED
          y             %Coord;        #REQUIRED
          drill         %Dimension;    #REQUIRED
          diameter      %Dimension;    "0"
          shape         %PadShape;     "round"
          rot           %Rotation;     "R0"
          stop          %Bool;         "yes"
          thermals      %Bool;         "yes"
          first         %Bool;         "no"
          >
    */

    D_PAD*  pad = new D_PAD( aModule );
    aModule->m_Pads.PushBack( pad );

    // the DTD says these must be present, throw exception if not found
    double x = attrs.get<double>( "x" );
    double y = attrs.get<double>( "y" );
    double drill = attrs.get<double>( "drill" );

    // pad's "Position" is not relative to the module's,
    // whereas Pos0 is relative to the module's but is the unrotated coordinate.

    wxPoint padpos( kicad_x( x ), kicad_y( y ) );

    pad->SetPos0( padpos );

    RotatePoint( &padpos, aModule->GetOrientation() );

    pad->SetPosition( padpos + aModule->GetPosition() );

    pad->SetDrillSize( wxSize( kicad( drill ), kicad( drill ) ) );

    pad->SetLayerMask( 0x00C0FFFF );    // should tell it to go through all layers

    // Optional according to DTD.
    opt_double diameter = attrs.get_optional<double>( "diameter" );
    opt_string shape    = attrs.get_optional<std::string>( "shape" );
    opt_string rot      = attrs.get_optional<std::string>( "rot" );
    opt_string stop     = attrs.get_optional<std::string>( "stop" );
    opt_string thermals = attrs.get_optional<std::string>( "thermals" );
    opt_string first    = attrs.get_optional<std::string>( "first" );

    if( diameter )
    {
        int kidiam = kicad( *diameter );
        pad->SetSize( wxSize( kidiam, kidiam ) );
    }

    if( shape ) // if not shape, our default is circle and that matches their default "round"
    {
        // <!ENTITY % PadShape "(square | round | octagon | long | offset)">

        if( !shape->compare( "round" ) )
            wxASSERT( pad->GetShape()==PAD_CIRCLE );    // verify set in D_PAD constructor

        else if( !shape->compare( "octagon" ) )
        {
            wxASSERT( pad->GetShape()==PAD_CIRCLE );    // verify set in D_PAD constructor

            // @todo no KiCad octagonal pad shape, use PAD_CIRCLE for now.
            // pad->SetShape( PAD_OCTAGON );
        }

        else if( !shape->compare( "long" ) )
        {
            pad->SetShape( PAD_OVAL );

            wxSize z = pad->GetSize();
            z.x *= 2;
            pad->SetSize( z );
        }
        else if( !shape->compare( "square" ) )
        {
            pad->SetShape( PAD_RECT );
        }
    }

    if( rot )
    {
        EROT r = erot( *rot );
        pad->SetOrientation( r.degrees * 10 );
    }

    // don't know what stop and thermals should look like now.
}


void EAGLE_PLUGIN::packageText( MODULE* aModule, CPTREE& aTree ) const
{
    int     sign  = 1;
    double  ratio = 6;
    ETEXT   t = etext( aTree );

    TEXTE_MODULE* txt;

    if( !t.text.compare( ">NAME" ) )
        txt = &aModule->Reference();
    else if( !t.text.compare( ">VALUE" ) )
        txt = &aModule->Value();
    else
        return;

    txt->SetTimeStamp( timeStamp( aTree ) );
    txt->SetText( FROM_UTF8( t.text.c_str() ) );

    wxPoint pos( kicad_x( t.x ), kicad_y( t.y ) );

    txt->SetPosition( pos );
    txt->SetPos0( pos - aModule->GetPosition() );

    txt->SetLayer( kicad_layer( t.layer ) );

    txt->SetSize( kicad_fontz( t.size ) );

    if( t.ratio )
        ratio = *t.ratio;

    txt->SetThickness( kicad( t.size * ratio / 100 ) );

    if( t.erot )
    {
        if( t.erot->spin || t.erot->degrees != 180 )
            txt->SetOrientation( t.erot->degrees * 10 );

        else    // 180 degrees, reverse justification below, don't spin
        {
            sign = -1;
        }

        txt->SetMirrored( t.erot->mirror );
    }

    int align = t.align ? *t.align : ETEXT::BOTTOM_LEFT;  // bottom-left is eagle default

    switch( align * sign )  // when negative, opposites are chosen
    {
    case ETEXT::CENTER:
        // this was the default in pcbtxt's constructor
        break;

    case ETEXT::CENTER_LEFT:
        txt->SetHorizJustify( GR_TEXT_HJUSTIFY_LEFT );
        break;

    case ETEXT::CENTER_RIGHT:
        txt->SetHorizJustify( GR_TEXT_HJUSTIFY_RIGHT );
        break;

    case ETEXT::TOP_CENTER:
        txt->SetVertJustify( GR_TEXT_VJUSTIFY_TOP );
        break;

    case ETEXT::TOP_LEFT:
        txt->SetHorizJustify( GR_TEXT_HJUSTIFY_LEFT );
        txt->SetVertJustify( GR_TEXT_VJUSTIFY_TOP );
        break;

    case ETEXT::TOP_RIGHT:
        txt->SetHorizJustify( GR_TEXT_HJUSTIFY_RIGHT );
        txt->SetVertJustify( GR_TEXT_VJUSTIFY_TOP );
        break;

    case ETEXT::BOTTOM_CENTER:
        txt->SetVertJustify( GR_TEXT_VJUSTIFY_BOTTOM );
        break;

    case ETEXT::BOTTOM_LEFT:
        txt->SetHorizJustify( GR_TEXT_HJUSTIFY_LEFT );
        txt->SetVertJustify( GR_TEXT_VJUSTIFY_BOTTOM );
        break;

    case ETEXT::BOTTOM_RIGHT:
        txt->SetHorizJustify( GR_TEXT_HJUSTIFY_RIGHT );
        txt->SetVertJustify( GR_TEXT_VJUSTIFY_BOTTOM );
        break;
    }
}


void EAGLE_PLUGIN::packageRectangle( MODULE* aModule, CPTREE& aTree ) const
{
    /*
    ERECT r = erect( aTree );
    */
}


void EAGLE_PLUGIN::packagePolygon( MODULE* aModule, CPTREE& aTree ) const
{
    // CPTREE& attrs = aTree.get_child( "<xmlattr>" );
}


void EAGLE_PLUGIN::packageCircle( MODULE* aModule, CPTREE& aTree ) const
{
    // CPTREE& attrs = aTree.get_child( "<xmlattr>" );
}


void EAGLE_PLUGIN::packageHole( MODULE* aModule, CPTREE& aTree ) const
{
    // CPTREE& attrs = aTree.get_child( "<xmlattr>" );
}


void EAGLE_PLUGIN::packageSMD( MODULE* aModule, CPTREE& aTree ) const
{
    // pay for this tree traversal only once
    CPTREE& attrs = aTree.get_child( "<xmlattr>" );

    /*
    <!ATTLIST smd
          name          %String;       #REQUIRED
          x             %Coord;        #REQUIRED
          y             %Coord;        #REQUIRED
          dx            %Dimension;    #REQUIRED
          dy            %Dimension;    #REQUIRED
          layer         %Layer;        #REQUIRED
          roundness     %Int;          "0"
          rot           %Rotation;     "R0"
          stop          %Bool;         "yes"
          thermals      %Bool;         "yes"
          cream         %Bool;         "yes"
          >
    */

    D_PAD*  pad = new D_PAD( aModule );
    aModule->m_Pads.PushBack( pad );

    pad->SetShape( PAD_RECT );
    pad->SetAttribute( PAD_SMD );

    // the DTD says these must be present, throw exception if not found
    double x  = attrs.get<double>( "x" );
    double y  = attrs.get<double>( "y" );
    double dx = attrs.get<double>( "dx" );
    double dy = attrs.get<double>( "dy" );
    int layer = attrs.get<int>( "layer" );

    // pad's "Position" is not relative to the module's,
    // whereas Pos0 is relative to the module's but is the unrotated coordinate.

    wxPoint padpos( kicad_x( x ), kicad_y( y ) );

    pad->SetPos0( padpos );

    RotatePoint( &padpos, aModule->GetOrientation() );

    pad->SetPosition( padpos + aModule->GetPosition() );

    pad->SetSize( wxSize( kicad( dx ), kicad( dy ) ) );

    pad->SetLayer( kicad_layer( layer ) );
    pad->SetLayerMask( 0x00888000 );

    // Optional according to DTD
    opt_double roundness = attrs.get_optional<double>( "roundness" );
    opt_string rot       = attrs.get_optional<std::string>( "rot" );
    opt_string stop      = attrs.get_optional<std::string>( "stop" );
    opt_string thermals  = attrs.get_optional<std::string>( "thermals" );
    opt_string cream     = attrs.get_optional<std::string>( "cream" );

    if( roundness ) // set set shape to PAD_RECT above, in case roundness is not present
    {
        if( *roundness >= 75 )       // roundness goes from 0-100%
        {
            if( dy == dx )
                pad->SetShape( PAD_ROUND );
            else
                pad->SetShape( PAD_OVAL );
        }
    }

    if( rot )
    {
        EROT r = erot( *rot );
        pad->SetOrientation( r.degrees * 10 );
    }

    // don't know what stop, thermals, and cream should look like now.
}


void EAGLE_PLUGIN::loadSignals( CPTREE& aSignals, const std::string& aXpath )
{
    int netCode = 1;

    for( CITER net = aSignals.begin();  net != aSignals.end();  ++net, ++netCode )
    {
        wxString netName = FROM_UTF8( net->second.get<std::string>( "<xmlattr>.name" ).c_str() );

        m_board->AppendNet( new NETINFO_ITEM( m_board, netName, netCode ) );

        // (contactref | polygon | wire | via)*
        for( CITER it = net->second.begin();  it != net->second.end();  ++it )
        {
            if( !it->first.compare( "wire" ) )
            {
                EWIRE   w = ewire( it->second );
                TRACK*  t = new TRACK( m_board );

                t->SetTimeStamp( timeStamp( it->second ));

                t->SetPosition( wxPoint( kicad_x( w.x1 ), kicad_y( w.y1 ) ) );
                t->SetEnd( wxPoint( kicad_x( w.x2 ), kicad_y( w.y2 ) ) );

                t->SetWidth( kicad( w.width ) );
                t->SetLayer( kicad_layer( w.layer ) );
                t->SetNet( netCode );

                m_board->m_Track.Insert( t, NULL );
            }

            else if( !it->first.compare( "via" ) )
            {
                EVIA    v = evia( it->second );

                int layer_start = kicad_layer( v.layer_start );
                int layer_end   = kicad_layer( v.layer_end );
                int drill       = kicad( v.drill );

                SEGVIA* via = new SEGVIA( m_board );

                via->SetLayerPair( layer_start, layer_end );

                if( v.diam )
                {
                    int kidiam = kicad( *v.diam );
                    via->SetWidth( kidiam );
                }

                via->SetDrill( drill );

                via->SetTimeStamp( timeStamp( it->second ) );

                wxPoint pos( kicad_x( v.x ), kicad_y( v.y ) );

                via->SetPosition( pos  );
                via->SetEnd( pos );

                // via->SetWidth( width );
                // via->SetShape( shape );
                via->SetNet( netCode );
                // via->SetState( flags, ON );

                via->SetShape( S_SEGMENT );

                m_board->m_Track.Insert( via, NULL );
            }

            else if( !it->first.compare( "contactref" ) )
            {
            }

            else if( !it->first.compare( "polygon" ) )
            {
            }
        }
    }
}


int EAGLE_PLUGIN::kicad_layer( int aLayer )
{
    int ret;

    switch( aLayer )    // translate eagle layer to pcbnew layer
    {
    case 1:     ret = 15;       break;  // Top copper
    case 2:     ret = 14;       break;
    case 3:     ret = 13;       break;
    case 4:     ret = 12;       break;
    case 5:     ret = 11;       break;
    case 6:     ret = 10;       break;
    case 7:     ret = 9;        break;
    case 8:     ret = 8;        break;
    case 9:     ret = 7;        break;
    case 10:    ret = 6;        break;
    case 11:    ret = 5;        break;
    case 12:    ret = 4;        break;
    case 13:    ret = 3;        break;
    case 14:    ret = 2;        break;
    case 15:    ret = 1;        break;
    case 16:    ret = 0;        break;  // Bottom Copper
    case 20:    ret = 28;       break;  // Edge Layer
    case 21:    ret = 21;       break;  // Top Silk Screen
    case 22:    ret = 20;       break;  // Bottom Silk Screen
    case 25:    ret = 25;       break;  // Misc Comment Layers
    case 26:    ret = 25;       break;
    case 27:    ret = 26;       break;
    case 28:    ret = 26;       break;
    case 29:    ret = 23;       break;
    case 30:    ret = 22;       break;
    case 31:    ret = 19;       break;
    case 32:    ret = 18;       break;
    case 35:    ret = 17;       break;
    case 36:    ret = 16;       break;
    case 51:    ret = 26;       break;
    case 52:    ret = 27;       break;
    case 95:    ret = 26;       break;
    case 96:    ret = 27;       break;
    default:    ret = -1;       break;  // our eagle understanding is incomplete
    }

    return ret;
}


wxArrayString EAGLE_PLUGIN::FootprintEnumerate( const wxString& aLibraryPath, PROPERTIES* aProperties )
{
    return wxArrayString();
}


MODULE* EAGLE_PLUGIN::FootprintLoad( const wxString& aLibraryPath, const wxString& aFootprintName, PROPERTIES* aProperties )
{
    return NULL;
}


void EAGLE_PLUGIN::FootprintSave( const wxString& aLibraryPath, const MODULE* aFootprint, PROPERTIES* aProperties )
{
}


void EAGLE_PLUGIN::FootprintDelete( const wxString& aLibraryPath, const wxString& aFootprintName )
{
}


void EAGLE_PLUGIN::FootprintLibCreate( const wxString& aLibraryPath, PROPERTIES* aProperties )
{
}


void EAGLE_PLUGIN::FootprintLibDelete( const wxString& aLibraryPath, PROPERTIES* aProperties )
{
}


bool EAGLE_PLUGIN::IsFootprintLibWritable( const wxString& aLibraryPath )
{
    return true;
}

