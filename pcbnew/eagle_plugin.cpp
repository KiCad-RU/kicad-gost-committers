
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

*) test footprint placement on board back
*) netclass info?
*) verify zone fill clearances are correct
*) write BOARD::Move() and reposition to center of page from centerBoard()

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

/// segment (element) of our XPATH into the Eagle XML document tree in PTREE form.
struct TRIPLET
{
    const char* element;
    const char* attribute;
    const char* value;

    TRIPLET( const char* aElement, const char* aAttribute = "", const char* aValue = "" ) :
        element( aElement ),
        attribute( aAttribute ),
        value( aValue )
    {}
};


/**
 * Class XPATH
 * keeps track of what we are working on within a PTREE.
 * Then if an exception is thrown, the place within the tree that gave us
 * grief can be reported almost accurately.  To minimally impact
 * speed, merely assign const char* pointers during the tree walking
 * expedition.  The const char* pointers must be to C strings residing either in
 * the data or code segment (i.e. "compiled in") or within the XML document, but
 * not on the stack, since the stack is unwound during the throwing of the
 * exception.  The XML document will not immediately vanish since we capture
 * the xpath (using function Contents()) before the XML document tree (PTREE)
 * is destroyed.
 */
class XPATH
{
    std::vector<TRIPLET>    p;

public:
    void push( const char* aPathSegment, const char* aAttribute="" )
    {
        p.push_back( TRIPLET( aPathSegment, aAttribute ) );
    }

    void clear()    { p.clear(); }

    void pop()      { p.pop_back(); }

    /// modify the last path node's value
    void Value( const char* aValue )
    {
        p.back().value = aValue;
    }

    /// modify the last path node's attribute
    void Attribute( const char* aAttribute )
    {
        p.back().attribute = aAttribute;
    }

    /// return the contents of the XPATH as a single string
    std::string Contents()
    {
        typedef std::vector<TRIPLET>::const_iterator CITER;

        std::string ret;

        for( CITER it = p.begin();  it != p.end();  ++it )
        {
            if( it != p.begin() )
                ret += '.';

            ret += it->element;

            if( it->attribute[0] && it->value[0] )
            {
                ret += '[';
                ret += it->attribute;
                ret += '=';
                ret += it->value;
                ret += ']';
            }
        }

        return ret;
    }
};


typedef EAGLE_PLUGIN::BIU                   BIU;
typedef PTREE::const_assoc_iterator         CA_ITER;
typedef PTREE::const_iterator               CITER;
typedef std::pair<CA_ITER, CA_ITER>         CA_ITER_RANGE;

typedef MODULE_MAP::iterator                MODULE_ITER;
typedef MODULE_MAP::const_iterator          MODULE_CITER;

typedef boost::optional<std::string>        opt_string;
typedef boost::optional<int>                opt_int;
typedef boost::optional<double>             opt_double;
typedef boost::optional<bool>               opt_bool;


/**
 * Function parseOptionalBool
 * returns an opt_bool and sets it true or false according to the presence
 * and value of an attribute within the CPTREE element.
 */
static opt_bool parseOptionalBool( CPTREE& attribs, const char* aName )
{
    opt_bool    ret;
    opt_string  stemp = attribs.get_optional<std::string>( aName );

    if( stemp )
        ret = !stemp->compare( "yes" );

    return ret;
}


// All of the 'E'STRUCTS below merely hold Eagle XML information verbatim, in binary.
// For maintenance and troubleshooting purposes, it was thought that we'd need to
// separate the conversion process into distinct steps. There is no intent to have KiCad
// forms of information in these 'E'STRUCTS.  They are only binary forms
// of the Eagle information in the corresponding Eagle XML nodes.


/// Eagle rotation
struct EROT
{
    bool    mirror;
    bool    spin;
    double  degrees;

    EROT() : mirror( false ), spin( false ), degrees( 0 ) {}
};

typedef boost::optional<EROT>   opt_erot;

/// parse an Eagle XML "rot" field.  Unfortunately the DTD seems not to explain
/// this format very well.  R[S][M]<degrees>.   Examples: "R90", "MR180", "SR180"
static EROT erot( const std::string& aRot )
{
    EROT    rot;

    rot.spin    = aRot.find( 'S' ) != aRot.npos;
    rot.mirror  = aRot.find( 'M' ) != aRot.npos;
    rot.degrees = strtod( aRot.c_str()
                        + 1                     // skip leading 'R'
                        + int( rot.spin )       // skip optional leading 'S'
                        + int( rot.mirror ),    // skip optional leading 'M'
                        NULL );
    return rot;
}

/// Eagle "rot" fields are optional, handle that by returning opt_erot.
static opt_erot parseOptionalEROT( CPTREE& attribs )
{
    opt_erot    ret;
    opt_string  stemp = attribs.get_optional<std::string>( "rot" );
    if( stemp )
        ret = erot( *stemp );
    return ret;
}

/// Eagle wire
struct EWIRE
{
    double  x1;
    double  y1;
    double  x2;
    double  y2;
    double  width;
    int     layer;
    EWIRE( CPTREE& aWire );
};

/**
 * Constructor EWIRE
 * converts a <wire>'s xml attributes to binary without additional conversion.
 * This result is an EWIRE with the <wire> textual data merely converted to binary.
 */
EWIRE::EWIRE( CPTREE& aWire )
{
    CPTREE& attribs = aWire.get_child( "<xmlattr>" );

    /*
    <!ELEMENT wire EMPTY>
    <!ATTLIST wire
          x1            %Coord;        #REQUIRED
          y1            %Coord;        #REQUIRED
          x2            %Coord;        #REQUIRED
          y2            %Coord;        #REQUIRED
          width         %Dimension;    #REQUIRED
          layer         %Layer;        #REQUIRED
          extent        %Extent;       #IMPLIED  -- only applicable for airwires --
          style         %WireStyle;    "continuous"
          curve         %WireCurve;    "0"
          cap           %WireCap;      "round"   -- only applicable if 'curve' is not zero --
          >
    */

    x1    = attribs.get<double>( "x1" );
    y1    = attribs.get<double>( "y1" );
    x2    = attribs.get<double>( "x2" );
    y2    = attribs.get<double>( "y2" );
    width = attribs.get<double>( "width" );
    layer = attribs.get<int>( "layer" );

    // ignoring extent, style, curve and cap
}


/// Eagle via
struct EVIA
{
    double      x;
    double      y;
    int         layer_front_most;   /// < extent
    int         layer_back_most;    /// < inclusive
    double      drill;
    opt_double  diam;
    opt_string  shape;
    EVIA( CPTREE& aVia );
};

EVIA::EVIA( CPTREE& aVia )
{
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

    x     = attribs.get<double>( "x" );
    y     = attribs.get<double>( "y" );

    std::string ext = attribs.get<std::string>( "extent" );

    sscanf( ext.c_str(), "%u-%u", &layer_front_most, &layer_back_most );

    drill = attribs.get<double>( "drill" );
    diam  = attribs.get_optional<double>( "diameter" );
    shape = attribs.get_optional<std::string>( "shape" );
}


/// Eagle circle
struct ECIRCLE
{
    double  x;
    double  y;
    double  radius;
    double  width;
    int     layer;

    ECIRCLE( CPTREE& aCircle );
};

ECIRCLE::ECIRCLE( CPTREE& aCircle )
{
    CPTREE& attribs = aCircle.get_child( "<xmlattr>" );

    /*
    <!ELEMENT circle EMPTY>
    <!ATTLIST circle
          x             %Coord;        #REQUIRED
          y             %Coord;        #REQUIRED
          radius        %Coord;        #REQUIRED
          width         %Dimension;    #REQUIRED
          layer         %Layer;        #REQUIRED
          >
    */

    x      = attribs.get<double>( "x" );
    y      = attribs.get<double>( "y" );
    radius = attribs.get<double>( "radius" );
    width  = attribs.get<double>( "width" );
    layer  = attribs.get<int>( "layer" );
}


/// Eagle XML rectangle in binary
struct ERECT
{
    double      x1;
    double      y1;
    double      x2;
    double      y2;
    int         layer;
    opt_erot    rot;

    ERECT( CPTREE& aRect );
};

ERECT::ERECT( CPTREE& aRect )
{
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

    x1    = attribs.get<double>( "x1" );
    y1    = attribs.get<double>( "y1" );
    x2    = attribs.get<double>( "x2" );
    y2    = attribs.get<double>( "y2" );
    layer = attribs.get<int>( "layer" );
    rot   = parseOptionalEROT( attribs );
}


/// Eagle "attribute" XML element, no foolin'.
struct EATTR
{
    std::string name;
    opt_string  value;
    opt_double  x;
    opt_double  y;
    opt_double  size;
    opt_int     layer;
    opt_double  ratio;
    opt_erot    rot;
    opt_int     display;

    enum {  // for 'display' field above
        Off,
        VALUE,
        NAME,
        BOTH,
    };

    EATTR( CPTREE& aTree );
};

/**
 * Constructor EATTR
 * parses an Eagle "attribute" XML element.  Note that an attribute element
 * is different than an XML element attribute.  The attribute element is a
 * full XML node in and of itself, and has attributes of its own.  Blame Eagle.
 */
EATTR::EATTR( CPTREE& aAttribute )
{
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

    name    = attribs.get<std::string>( "name" );                    // #REQUIRED
    value   = attribs.get_optional<std::string>( "value" );

    x       = attribs.get_optional<double>( "x" );
    y       = attribs.get_optional<double>( "y" );
    size    = attribs.get_optional<double>( "size" );

    // KiCad cannot currently put a TEXTE_MODULE on a different layer than the MODULE
    // Eagle can it seems.
    layer   = attribs.get_optional<int>( "layer" );

    ratio   = attribs.get_optional<double>( "ratio" );
    rot     = parseOptionalEROT( attribs );

    opt_string stemp = attribs.get_optional<std::string>( "display" );
    if( stemp )
    {
        // (off | value | name | both)
        if( !stemp->compare( "off" ) )
            display = EATTR::Off;
        else if( !stemp->compare( "value" ) )
            display = EATTR::VALUE;
        else if( !stemp->compare( "name" ) )
            display = EATTR::NAME;
        else if( !stemp->compare( "both" ) )
            display = EATTR::BOTH;
    }
}


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
    opt_erot    rot;

    enum {          // for align
        CENTER,
        CENTER_LEFT,
        TOP_CENTER,
        TOP_LEFT,
        TOP_RIGHT,

        // opposites are -1 x above, used by code tricks in here
        CENTER_RIGHT  = -CENTER_LEFT,
        BOTTOM_CENTER = -TOP_CENTER,
        BOTTOM_LEFT   = -TOP_RIGHT,
        BOTTOM_RIGHT  = -TOP_LEFT,
    };

    opt_int     align;

    ETEXT( CPTREE& aText );
};

ETEXT::ETEXT( CPTREE& aText )
{
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

    text   = aText.data();
    x      = attribs.get<double>( "x" );
    y      = attribs.get<double>( "y" );
    size   = attribs.get<double>( "size" );
    layer  = attribs.get<int>( "layer" );

    font   = attribs.get_optional<std::string>( "font" );
    ratio  = attribs.get_optional<double>( "ratio" );
    rot    = parseOptionalEROT( attribs );

    opt_string stemp = attribs.get_optional<std::string>( "align" );
    if( stemp )
    {
        // (bottom-left | bottom-center | bottom-right | center-left |
        //   center | center-right | top-left | top-center | top-right)
        if( !stemp->compare( "center" ) )
            align = ETEXT::CENTER;
        else if( !stemp->compare( "center-right" ) )
            align = ETEXT::CENTER_RIGHT;
        else if( !stemp->compare( "top-left" ) )
            align = ETEXT::TOP_LEFT;
        else if( !stemp->compare( "top-center" ) )
            align = ETEXT::TOP_CENTER;
        else if( !stemp->compare( "top-right" ) )
            align = ETEXT::TOP_RIGHT;
        else if( !stemp->compare( "bottom-left" ) )
            align = ETEXT::BOTTOM_LEFT;
        else if( !stemp->compare( "bottom-center" ) )
            align = ETEXT::BOTTOM_CENTER;
        else if( !stemp->compare( "bottom-right" ) )
            align = ETEXT::BOTTOM_RIGHT;
        else if( !stemp->compare( "center-left" ) )
            align = ETEXT::CENTER_LEFT;
    }
}


/// Eagle thru hol pad
struct EPAD
{
    std::string name;
    double      x;
    double      y;
    double      drill;
    opt_double  diameter;

    // for shape: (square | round | octagon | long | offset)
    enum {
        SQUARE,
        ROUND,
        OCTAGON,
        LONG,
        OFFSET,
    };
    opt_int     shape;
    opt_erot    rot;
    opt_bool    stop;
    opt_bool    thermals;
    opt_bool    first;

    EPAD( CPTREE& aPad );
};

EPAD::EPAD( CPTREE& aPad )
{
    CPTREE& attribs = aPad.get_child( "<xmlattr>" );

    /*
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

    // #REQUIRED says DTD, throw exception if not found
    name  = attribs.get<std::string>( "name" );
    x     = attribs.get<double>( "x" );
    y     = attribs.get<double>( "y" );
    drill = attribs.get<double>( "drill" );

    diameter = attribs.get_optional<double>( "diameter" );

    opt_string s = attribs.get_optional<std::string>( "shape" );
    if( s )
    {
        // (square | round | octagon | long | offset)
        if( !s->compare( "square" ) )
            shape = EPAD::SQUARE;
        else if( !s->compare( "round" ) )
            shape = EPAD::ROUND;
        else if( !s->compare( "octagon" ) )
            shape = EPAD::OCTAGON;
        else if( !s->compare( "long" ) )
            shape = EPAD::LONG;
        else if( !s->compare( "offset" ) )
            shape = EPAD::OFFSET;
    }

    rot      = parseOptionalEROT( attribs );
    stop     = parseOptionalBool( attribs, "stop" );
    thermals = parseOptionalBool( attribs, "thermals" );
    first    = parseOptionalBool( attribs, "first" );
}


/// Eagle SMD pad
struct ESMD
{
    std::string name;
    double      x;
    double      y;
    double      dx;
    double      dy;
    int         layer;
    opt_int     roundness;
    opt_erot    rot;
    opt_bool    stop;
    opt_bool    thermals;
    opt_bool    cream;

    ESMD( CPTREE& aSMD );
};

ESMD::ESMD( CPTREE& aSMD )
{
    CPTREE& attribs = aSMD.get_child( "<xmlattr>" );

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

    // DTD #REQUIRED, throw exception if not found
    name  = attribs.get<std::string>( "name" );
    x     = attribs.get<double>( "x" );
    y     = attribs.get<double>( "y" );
    dx    = attribs.get<double>( "dx" );
    dy    = attribs.get<double>( "dy" );
    layer = attribs.get<int>( "layer" );
    rot   = parseOptionalEROT( attribs );

    roundness = attribs.get_optional<int>( "roundness" );
    thermals  = parseOptionalBool( attribs, "thermals" );
    stop      = parseOptionalBool( attribs, "stop" );
    thermals  = parseOptionalBool( attribs, "thermals" );
    cream     = parseOptionalBool( attribs, "cream" );
}


struct EVERTEX
{
    double      x;
    double      y;

    EVERTEX( CPTREE& aVertex );
};

EVERTEX::EVERTEX( CPTREE& aVertex )
{
    CPTREE& attribs = aVertex.get_child( "<xmlattr>" );

    /*
    <!ELEMENT vertex EMPTY>
    <!ATTLIST vertex
          x             %Coord;        #REQUIRED
          y             %Coord;        #REQUIRED
          curve         %WireCurve;    "0" -- the curvature from this vertex to the next one --
          >
    */

    x = attribs.get<double>( "x" );
    y = attribs.get<double>( "y" );
}


/// Eagle polygon, without vertices which are parsed as needed
struct EPOLYGON
{
    double      width;
    int         layer;
    opt_double  spacing;

    enum {      // for pour
        SOLID,
        HATCH,
        CUTOUT,
    };
    opt_int     pour;
    opt_double  isolate;
    opt_bool    orphans;
    opt_bool    thermals;
    opt_int     rank;

    EPOLYGON( CPTREE& aPolygon );
};

EPOLYGON::EPOLYGON( CPTREE& aPolygon )
{
    CPTREE& attribs = aPolygon.get_child( "<xmlattr>" );

    /*
    <!ATTLIST polygon
          width         %Dimension;    #REQUIRED
          layer         %Layer;        #REQUIRED
          spacing       %Dimension;    #IMPLIED
          pour          %PolygonPour;  "solid"
          isolate       %Dimension;    #IMPLIED -- only in <signal> or <package> context --
          orphans       %Bool;         "no"  -- only in <signal> context --
          thermals      %Bool;         "yes" -- only in <signal> context --
          rank          %Int;          "0"   -- 1..6 in <signal> context, 0 or 7 in <package> context --
          >
    */

    width   = attribs.get<double>( "width" );
    layer   = attribs.get<int>( "layer" );
    spacing = attribs.get_optional<double>( "spacing" );

    opt_string s = attribs.get_optional<std::string>( "pour" );
    if( s )
    {
        // (solid | hatch | cutout)
        if( !s->compare( "hatch" ) )
            pour = EPOLYGON::HATCH;
        else if( !s->compare( "cutout" ) )
            pour = EPOLYGON::CUTOUT;
        else
            pour = EPOLYGON::SOLID;
    }

    orphans  = parseOptionalBool( attribs, "orphans" );
    thermals = parseOptionalBool( attribs, "thermals" );
    rank     = attribs.get_optional<int>( "rank" );
}

/// Eagle hole element
struct EHOLE
{
    double      x;
    double      y;
    double      drill;

    EHOLE( CPTREE& aHole );
};

EHOLE::EHOLE( CPTREE& aHole )
{
    CPTREE& attribs = aHole.get_child( "<xmlattr>" );

    /*
    <!ELEMENT hole EMPTY>
    <!ATTLIST hole
          x             %Coord;        #REQUIRED
          y             %Coord;        #REQUIRED
          drill         %Dimension;    #REQUIRED
          >
    */

    // #REQUIRED:
    x     = attribs.get<double>( "x" );
    y     = attribs.get<double>( "y" );
    drill = attribs.get<double>( "drill" );
}


/// Eagle element element
struct EELEMENT
{
    std::string name;
    std::string library;
    std::string package;
    std::string value;
    double      x;
    double      y;
    opt_bool    locked;
    // opt_bool    smashed;
    opt_erot    rot;

    EELEMENT( CPTREE& aElement );
};

EELEMENT::EELEMENT( CPTREE& aElement )
{
    CPTREE& attribs = aElement.get_child( "<xmlattr>" );

    /*
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

    // #REQUIRED
    name    = attribs.get<std::string>( "name" );
    library = attribs.get<std::string>( "library" );
    package = attribs.get<std::string>( "package" );
    value   = attribs.get<std::string>( "value" );

    x = attribs.get<double>( "x" );
    y = attribs.get<double>( "y" );

    // optional
    locked  = parseOptionalBool( attribs, "locked" );
    // smashed = pasreOptionalBool( attribs, "smashed" );
    rot = parseOptionalEROT( attribs );
}


struct ELAYER
{
    int         number;
    std::string name;
    int         color;
    int         fill;
    opt_bool    visible;
    opt_bool    active;

    ELAYER( CPTREE& aLayer );
};

ELAYER::ELAYER( CPTREE& aLayer )
{
    CPTREE& attribs = aLayer.get_child( "<xmlattr>" );

    /*
    <!ELEMENT layer EMPTY>
    <!ATTLIST layer
          number        %Layer;        #REQUIRED
          name          %String;       #REQUIRED
          color         %Int;          #REQUIRED
          fill          %Int;          #REQUIRED
          visible       %Bool;         "yes"
          active        %Bool;         "yes"
          >
    */

    number = attribs.get<int>( "number" );
    name   = attribs.get<std::string>( "name" );
    color  = attribs.get<int>( "color" );
    visible = parseOptionalBool( attribs, "visible" );
    active  = parseOptionalBool( attribs, "active" );
}


/// Assemble a two part key as a simple concatonation of aFirst and aSecond parts,
/// using a separator.
static inline std::string makeKey( const std::string& aFirst, const std::string& aSecond )
{
    std::string key = aFirst + '\x02' +  aSecond;
    return key;
}


/// Make a unique time stamp
static inline unsigned long timeStamp( CPTREE& aTree )
{
    // in this case from a unique tree memory location
    return (unsigned long)(void*) &aTree;
}


EAGLE_PLUGIN::EAGLE_PLUGIN() :
    m_xpath( new XPATH() )
{
    init( NULL );
}


EAGLE_PLUGIN::~EAGLE_PLUGIN()
{
    delete m_xpath;
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
        // 8 bit "filename" should be encoded according to disk filename encoding,
        // (maybe this is current locale, maybe not, its a filesystem issue),
        // and is not necessarily utf8.
        std::string filename = (const char*) aFileName.char_str( wxConvFile );

        read_xml( filename, doc, xml_parser::trim_whitespace | xml_parser::no_comments );

        loadAllSections( doc );

        // should be empty, else missing m_xpath->pop()
        wxASSERT( m_xpath->Contents().size() == 0 );
    }

    catch( file_parser_error fpe )
    {
        // for xml_parser_error, what() has the line number in it,
        // but no byte offset.  That should be an adequate error message.
        THROW_IO_ERROR( fpe.what() );
    }

    // Class ptree_error is a base class for xml_parser_error & file_parser_error,
    // so one catch should be OK for all errors.
    catch( ptree_error pte )
    {
        std::string errmsg = pte.what();

        errmsg += " @\n";
        errmsg += m_xpath->Contents();

        THROW_IO_ERROR( errmsg );
    }

    // IO_ERROR exceptions are left uncaught, they pass upwards from here.

    centerBoard();

    deleter.release();
    return m_board;
}


void EAGLE_PLUGIN::init( PROPERTIES* aProperties )
{
    m_hole_count = 0;

    m_xpath->clear();
    m_pads_to_nets.clear();
    m_templates.clear();

    m_board = NULL;
    m_props = aProperties;

    mm_per_biu = 1/IU_PER_MM;
    biu_per_mm = IU_PER_MM;
}


void EAGLE_PLUGIN::loadAllSections( CPTREE& aDoc )
{
    CPTREE& drawing = aDoc.get_child( "eagle.drawing" );
    m_xpath->push( "eagle.drawing" );

    {
        CPTREE& layers = drawing.get_child( "layers" );
        loadLayerDefs( layers );
    }

    {
        CPTREE& board = drawing.get_child( "board" );
        m_xpath->push( "board" );

        CPTREE& plain = board.get_child( "plain" );
        loadPlain( plain );

        CPTREE&  signals = board.get_child( "signals" );
        loadSignals( signals );

        CPTREE&  libs = board.get_child( "libraries" );
        loadLibraries( libs );

        CPTREE& elems = board.get_child( "elements" );
        loadElements( elems );

        m_xpath->pop();     // "board"
    }

    m_xpath->pop();     // "eagle.drawing"
}


void EAGLE_PLUGIN::loadLayerDefs( CPTREE& aLayers )
{
    m_xpath->push( "layers.layer" );

    typedef std::vector<ELAYER>     ELAYERS;
    typedef ELAYERS::const_iterator EITER;

    ELAYERS     cu;  // copper layers

    // find the subset of layers that are copper, and active
    for( CITER layer = aLayers.begin();  layer != aLayers.end();  ++layer )
    {
        ELAYER  elayer( layer->second );

        if( elayer.number >= 1 && elayer.number <= 16 && ( !elayer.active || *elayer.active ) )
        {
            cu.push_back( elayer );
        }
    }

    m_board->SetCopperLayerCount( cu.size() );

    for( EITER it = cu.begin();  it != cu.end();  ++it )
    {
        int layer = kicad_layer( it->number );

        m_board->SetLayerName( layer, FROM_UTF8( it->name.c_str() ) );
        m_board->SetLayerType( layer, LT_SIGNAL );

        // could map the colors here
    }

    m_xpath->pop();
}


void EAGLE_PLUGIN::loadPlain( CPTREE& aGraphics )
{
    m_xpath->push( "plain" );

    // (polygon | wire | text | circle | rectangle | frame | hole)*
    for( CITER gr = aGraphics.begin();  gr != aGraphics.end();  ++gr )
    {
        if( !gr->first.compare( "wire" ) )
        {
            m_xpath->push( "wire" );
            EWIRE w( gr->second );

            DRAWSEGMENT* dseg = new DRAWSEGMENT( m_board );
            m_board->Add( dseg, ADD_APPEND );

            dseg->SetTimeStamp( timeStamp( gr->second ) );
            dseg->SetLayer( kicad_layer( w.layer ) );
            dseg->SetStart( wxPoint( kicad_x( w.x1 ), kicad_y( w.y1 ) ) );
            dseg->SetEnd( wxPoint( kicad_x( w.x2 ), kicad_y( w.y2 ) ) );
            dseg->SetWidth( kicad( w.width ) );
            m_xpath->pop();
        }

        else if( !gr->first.compare( "text" ) )
        {
#if defined(DEBUG)
            if( !gr->second.data().compare( "ATMEGA328" ) )
            {
                int breakhere = 1;
                (void) breakhere;
            }
#endif
            m_xpath->push( "text" );
            ETEXT   t( gr->second );
            int     layer = kicad_layer( t.layer );

            int     sign = 1;

            TEXTE_PCB* pcbtxt = new TEXTE_PCB( m_board );
            m_board->Add( pcbtxt, ADD_APPEND );

            pcbtxt->SetLayer( layer );
            pcbtxt->SetTimeStamp( timeStamp( gr->second ) );
            pcbtxt->SetText( FROM_UTF8( t.text.c_str() ) );
            pcbtxt->SetPosition( wxPoint( kicad_x( t.x ), kicad_y( t.y ) ) );

            pcbtxt->SetSize( kicad_fontz( t.size ) );

            double  ratio = t.ratio ? *t.ratio : 8;     // DTD says 8 is default

            pcbtxt->SetThickness( kicad( t.size * ratio / 100 ) );

            if( t.rot )
            {
#if 0
                if( t.rot->spin || ( t.rot->degrees != 180 && t.rot->degrees != 270 ) )
                    pcbtxt->SetOrientation( t.rot->degrees * 10 );

                else
                    // flip the justification to opposite
                    sign = -1;
#else
                // eagles does not rotate text spun to 180 degrees unless spin is set.
                if( t.rot->spin || t.rot->degrees != 180 )
                    pcbtxt->SetOrientation( t.rot->degrees * 10 );

                else
                    // flip the justification to opposite
                    sign = -1;

                if( t.rot->degrees == 270 )
                    sign = -1;
#endif
                pcbtxt->SetMirrored( t.rot->mirror );

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
            m_xpath->pop();
        }

        else if( !gr->first.compare( "circle" ) )
        {
            m_xpath->push( "circle" );
            ECIRCLE c( gr->second );

            DRAWSEGMENT* dseg = new DRAWSEGMENT( m_board );
            m_board->Add( dseg, ADD_APPEND );

            dseg->SetShape( S_CIRCLE );
            dseg->SetTimeStamp( timeStamp( gr->second ) );
            dseg->SetLayer( kicad_layer( c.layer ) );
            dseg->SetStart( wxPoint( kicad_x( c.x ), kicad_y( c.y ) ) );
            dseg->SetEnd( wxPoint( kicad_x( c.x + c.radius ), kicad_y( c.y ) ) );
            dseg->SetWidth( kicad( c.width ) );
            m_xpath->pop();
        }

        // This seems to be a simplified rectangular [copper] zone, cannot find any
        // net related info on it from the DTD.
        else if( !gr->first.compare( "rectangle" ) )
        {
            m_xpath->push( "rectangle" );
            ERECT   r( gr->second );
            int     layer = kicad_layer( r.layer );

            if( IsValidCopperLayerIndex( layer ) )
            {
                // use a "netcode = 0" type ZONE:
                ZONE_CONTAINER* zone = new ZONE_CONTAINER( m_board );
                m_board->Add( zone, ADD_APPEND );

                zone->SetTimeStamp( timeStamp( gr->second ) );
                zone->SetLayer( layer );
                zone->SetNet( 0 );

                int outline_hatch = CPolyLine::DIAGONAL_EDGE;

                zone->m_Poly->Start( layer, kicad_x( r.x1 ), kicad_y( r.y1 ), outline_hatch );
                zone->AppendCorner( wxPoint( kicad_x( r.x2 ), kicad_y( r.y1 ) ) );
                zone->AppendCorner( wxPoint( kicad_x( r.x2 ), kicad_y( r.y2 ) ) );
                zone->AppendCorner( wxPoint( kicad_x( r.x1 ), kicad_y( r.y2 ) ) );
                zone->m_Poly->Close();

                // this is not my fault:
                zone->m_Poly->SetHatch( outline_hatch,
                                      Mils2iu( zone->m_Poly->GetDefaultHatchPitchMils() ) );
            }
            m_xpath->pop();
        }

        else if( !gr->first.compare( "hole" ) )
        {
            m_xpath->push( "hole" );
            EHOLE   e( gr->second );

            // Fabricate a MODULE with a single PAD_HOLE_NOT_PLATED pad.
            // Use m_hole_count to gen up a unique name.

            MODULE* module = new MODULE( m_board );
            m_board->Add( module, ADD_APPEND );

            char    temp[40];
            sprintf( temp, "@HOLE%d", m_hole_count++ );
            module->SetReference( FROM_UTF8( temp ) );
            module->Reference().SetVisible( false );

            wxPoint pos( kicad_x( e.x ), kicad_y( e.y ) );

            module->SetPosition( pos );

            // Add a PAD_HOLE_NOT_PLATED pad to this module.
            D_PAD* pad = new D_PAD( module );
            module->m_Pads.PushBack( pad );

            pad->SetShape( PAD_ROUND );
            pad->SetAttribute( PAD_HOLE_NOT_PLATED );

            /* pad's position is already centered on module at relative (0, 0)
            wxPoint padpos( kicad_x( e.x ), kicad_y( e.y ) );

            pad->SetPos0( padpos );
            pad->SetPosition( padpos + module->GetPosition() );
            */

            wxSize  sz( kicad( e.drill ), kicad( e.drill ) );

            pad->SetDrillSize( sz );
            pad->SetSize( sz );

            pad->SetLayerMask( ALL_CU_LAYERS /* | SOLDERMASK_LAYER_BACK | SOLDERMASK_LAYER_FRONT */ );
            m_xpath->pop();
        }

        else if( !gr->first.compare( "frame" ) )
        {
            // picture this
        }
        else if( !gr->first.compare( "polygon" ) )
        {
            // could be on a copper layer, could be on another layer.
            // copper layer would be done using netCode=0 type of ZONE_CONTAINER.
        }
    }
    m_xpath->pop();
}


void EAGLE_PLUGIN::loadLibraries( CPTREE& aLibs )
{
    m_xpath->push( "libraries.library", "name" );

    for( CITER library = aLibs.begin();  library != aLibs.end();  ++library )
    {
        const std::string& lib_name = library->second.get<std::string>( "<xmlattr>.name" );

        m_xpath->Value( lib_name.c_str() );

        {
            m_xpath->push( "packages" );

            // library will have <xmlattr> node, skip that and get the single packages node
            CPTREE& packages = library->second.get_child( "packages" );

            // Create a MODULE for all the eagle packages, for use later via a copy constructor
            // to instantiate needed MODULES in our BOARD.  Save the MODULE templates in
            // a MODULE_MAP using a single lookup key consisting of libname+pkgname.

            for( CITER package = packages.begin();  package != packages.end();  ++package )
            {
                m_xpath->push( "package", "name" );
                const std::string& pack_name = package->second.get<std::string>( "<xmlattr>.name" );

    #if defined(DEBUG)
                if( !pack_name.compare( "TO220H" ) )
                {
                    int breakhere = 1;
                    (void) breakhere;
                }
    #endif
                m_xpath->Value( pack_name.c_str() );

                std::string key = makeKey( lib_name, pack_name );

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

                m_xpath->pop();
            }

            m_xpath->pop();     // "packages"
        }
    }

    m_xpath->pop();
}


void EAGLE_PLUGIN::loadElements( CPTREE& aElements )
{
    m_xpath->push( "elements.element", "name" );

    for( CITER it = aElements.begin();  it != aElements.end();  ++it )
    {
        if( it->first.compare( "element" ) )
            continue;

        EELEMENT    e( it->second );

        m_xpath->Value( e.name.c_str() );

#if 1 && defined(DEBUG)
        if( !e.value.compare( "LP2985-33DBVR" ) )
        {
            int breakhere = 1;
            (void) breakhere;
        }
#endif

        std::string key = makeKey( e.library, e.package );

        MODULE_CITER mi = m_templates.find( key );

        if( mi == m_templates.end() )
        {
            wxString emsg = wxString::Format( _( "No '%s' package in library '%s'" ),
                GetChars( FROM_UTF8( e.package.c_str() ) ),
                GetChars( FROM_UTF8( e.library.c_str() ) ) );
            THROW_IO_ERROR( emsg );
        }

#if defined(DEBUG)
        if( !e.name.compare( "ATMEGA328" ) )
        {
            int breakhere = 1;
            (void) breakhere;
        }
#endif

        // copy constructor to clone the template
        MODULE* m = new MODULE( *mi->second );
        m_board->Add( m, ADD_APPEND );

        // update the nets within the pads of the clone
        for( D_PAD* pad = m->m_Pads;  pad;  pad = pad->Next() )
        {
            std::string key  = makeKey( e.name, TO_UTF8( pad->GetPadName() ) );

            NET_MAP_CITER ni = m_pads_to_nets.find( key );
            if( ni != m_pads_to_nets.end() )
            {
                const ENET* enet = &ni->second;
                pad->SetNetname( FROM_UTF8( enet->netname.c_str() ) );
                pad->SetNet( enet->netcode );
            }
        }

        m->SetPosition( wxPoint( kicad_x( e.x ), kicad_y( e.y ) ) );
        m->SetReference( FROM_UTF8( e.name.c_str() ) );
        m->SetValue( FROM_UTF8( e.value.c_str() ) );
        // m->Value().SetVisible( false );

        if( e.rot )
        {
            m->SetOrientation( e.rot->degrees * 10 );

            if( e.rot->mirror )
            {
                m->Flip( m->GetPosition() );
            }
        }

        m_xpath->push( "attribute", "name" );

        // VALUE and NAME can have something like our text "effects" overrides
        // in SWEET and new schematic.  Eagle calls these XML elements "attribute".
        // There can be one for NAME and/or VALUE both.  Features present in the
        // EATTR override the ones established in the package only if they are
        // present here (except for rot, which if not present means angle zero).
        // So the logic is a bit different than in packageText() and in plain text.
        for( CITER ait = it->second.begin();  ait != it->second.end();  ++ait )
        {
            if( ait->first.compare( "attribute" ) )
                continue;

            EATTR   a( ait->second );

            TEXTE_MODULE*   txt;

            if( !a.name.compare( "NAME" ) )
                txt = &m->Reference();
            else if( !a.name.compare( "VALUE" ) )
                txt = &m->Value();
            else
            {
                  // our understanding of file format is incomplete?
                  continue;
            }

            m_xpath->Value( a.name.c_str() );

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

            // Even though size and ratio are both optional, I am not seeing
            // a case where ratio is present but size is not.

            if( a.size )
            {
                wxSize  fontz = kicad_fontz( *a.size );
                txt->SetSize( fontz );

                if( a.ratio )
                {
                    double  ratio = *a.ratio;
                    int     lw = int( fontz.y * ratio / 100.0 );
                    txt->SetThickness( lw );
                }
            }

            // The "rot" in a EATTR seems to be assumed to be zero if it is not
            // present, and this zero rotation becomes an override to the
            // package's text field.  If they did not want zero, they specify
            // what they want explicitly.
            EROT rot;

            if( a.rot )
                rot = *a.rot;

            if( rot.spin || (rot.degrees != 180 && rot.degrees != 270) )
            {
                double angle = rot.degrees * 10;
                angle -= m->GetOrientation();   // subtract module's angle
                txt->SetOrientation( angle );
            }
            else
            {
                // ETEXT::TOP_RIGHT:
                txt->SetHorizJustify( GR_TEXT_HJUSTIFY_RIGHT );
                txt->SetVertJustify( GR_TEXT_VJUSTIFY_TOP );
            }

            txt->SetMirrored( rot.mirror );
        }

        m_xpath->pop();     // "attribute"
    }

    m_xpath->pop();     // "elements.element"
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
    EWIRE   w( aTree );
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
    // this is thru hole technology here, no SMDs
    EPAD e( aTree );

    D_PAD*  pad = new D_PAD( aModule );
    aModule->m_Pads.PushBack( pad );

    pad->SetPadName( FROM_UTF8( e.name.c_str() ) );

    // pad's "Position" is not relative to the module's,
    // whereas Pos0 is relative to the module's but is the unrotated coordinate.

    wxPoint padpos( kicad_x( e.x ), kicad_y( e.y ) );

    pad->SetPos0( padpos );

    RotatePoint( &padpos, aModule->GetOrientation() );

    pad->SetPosition( padpos + aModule->GetPosition() );

    pad->SetDrillSize( wxSize( kicad( e.drill ), kicad( e.drill ) ) );

    pad->SetLayerMask( ALL_CU_LAYERS | SOLDERMASK_LAYER_BACK | SOLDERMASK_LAYER_FRONT );

    if( e.shape )
    {
        switch( *e.shape )
        {
        case EPAD::ROUND:
            wxASSERT( pad->GetShape()==PAD_CIRCLE );    // verify set in D_PAD constructor
            break;
        case EPAD::OCTAGON:
            // no KiCad octagonal pad shape, use PAD_CIRCLE for now.
            // pad->SetShape( PAD_OCTAGON );
            wxASSERT( pad->GetShape()==PAD_CIRCLE );    // verify set in D_PAD constructor
            break;
        case EPAD::LONG:
            pad->SetShape( PAD_OVAL );
            break;
        case EPAD::SQUARE:
            pad->SetShape( PAD_RECT );
            break;
        case EPAD::OFFSET:
            ;   // don't know what to do here.
        }
    }
    else
    {
        // if shape is not present, our default is circle and that matches their default "round"
    }

    if( e.diameter )
    {
        int diameter = kicad( *e.diameter );
        pad->SetSize( wxSize( diameter, diameter ) );
    }
    else
    {
        // The pad size is optional in the eagle DTD, so we must guess.
        // Supply something here that is a minimum copper surround, or otherwise
        // 120% of drillz whichever is greater.  But for PAD_OVAL, we can use
        // a smaller minimum than for a round pad, since there is a larger copper
        // body on the elongated ends.

        int min_copper;

        if( pad->GetShape() == PAD_OVAL )
            min_copper = Mils2iu( 4 );
        else
            min_copper = Mils2iu( 6 );

        // minz copper surround as a minimum, otherwise 110% of drillz.
        int drillz   = pad->GetDrillSize().x;
        int diameter = std::max( drillz + 2 * min_copper, int( drillz * 1.2 ) );

        pad->SetSize( wxSize( diameter, diameter ) );
    }

    if( pad->GetShape() == PAD_OVAL )
    {
        // The Eagle "long" pad seems to be tall, "width = height x 4/3" apparently.
        wxSize sz = pad->GetSize();
        sz.x = (sz.x * 4)/3;
        pad->SetSize( sz );
    }

    if( e.rot )
    {
        pad->SetOrientation( e.rot->degrees * 10 );
    }

    // @todo: handle stop and thermal
}


void EAGLE_PLUGIN::packageText( MODULE* aModule, CPTREE& aTree ) const
{
    ETEXT   t( aTree );
    int     layer = kicad_layer( t.layer );

    TEXTE_MODULE* txt;

    if( !t.text.compare( ">NAME" ) || !t.text.compare( ">name" ) )
        txt = &aModule->Reference();
    else if( !t.text.compare( ">VALUE" ) || !t.text.compare( ">value" ) )
        txt = &aModule->Value();
    else
    {
        txt = new TEXTE_MODULE( aModule );
        aModule->m_Drawings.PushBack( txt );
    }

    txt->SetTimeStamp( timeStamp( aTree ) );
    txt->SetText( FROM_UTF8( t.text.c_str() ) );

    wxPoint pos( kicad_x( t.x ), kicad_y( t.y ) );

    txt->SetPosition( pos );
    txt->SetPos0( pos - aModule->GetPosition() );

    /*
    switch( layer )
    {
    case COMMENT_N: layer = SILKSCREEN_N_FRONT; break;
    }
    */

    txt->SetLayer( layer );

    txt->SetSize( kicad_fontz( t.size ) );

    double ratio = t.ratio ? *t.ratio : 8;  // DTD says 8 is default

    txt->SetThickness( kicad( t.size * ratio / 100 ) );

    double angle = t.rot ? t.rot->degrees * 10 : 0;

    // An eagle package is never rotated, the DTD does not allow it.
    // angle -= aModule->GetOrienation();

    int sign = 1;
    if( t.rot )
    {
        if( t.rot->spin || (angle != 1800 && angle != 2700) )
            txt->SetOrientation( angle );

        else    // 180 or 270 degrees, reverse justification below, don't spin
            sign = -1;

        txt->SetMirrored( t.rot->mirror );
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
    ERECT   r( aTree );
    int     layer = kicad_layer( r.layer );

    if( IsValidNonCopperLayerIndex( layer ) )  // skip copper "package.rectangle"s
    {
        EDGE_MODULE* dwg = new EDGE_MODULE( aModule, S_POLYGON );
        aModule->m_Drawings.PushBack( dwg );

        dwg->SetLayer( layer );
        dwg->SetWidth( 0 );

        dwg->SetTimeStamp( timeStamp( aTree ) );

        std::vector<wxPoint> pts;

        wxPoint start( wxPoint( kicad_x( r.x1 ), kicad_y( r.y1 ) ) );
        wxPoint end(   wxPoint( kicad_x( r.x1 ), kicad_y( r.y2 ) ) );

        pts.push_back( start );
        pts.push_back( wxPoint( kicad_x( r.x2 ), kicad_y( r.y1 ) ) );
        pts.push_back( wxPoint( kicad_x( r.x2 ), kicad_y( r.y2 ) ) );
        pts.push_back( end );

        dwg->SetPolyPoints( pts );

        dwg->SetStart0( start );
        dwg->SetEnd0( end );
    }
}


void EAGLE_PLUGIN::packagePolygon( MODULE* aModule, CPTREE& aTree ) const
{
    EPOLYGON    p( aTree );
    int         layer = kicad_layer( p.layer );

    if( IsValidNonCopperLayerIndex( layer ) )  // skip copper "package.rectangle"s
    {
        EDGE_MODULE* dwg = new EDGE_MODULE( aModule, S_POLYGON );
        aModule->m_Drawings.PushBack( dwg );

        dwg->SetWidth( 0 );     // it's filled, no need for boundary width

        /*
        switch( layer )
        {
        case ECO1_N:    layer = SILKSCREEN_N_FRONT; break;
        case ECO2_N:    layer = SILKSCREEN_N_BACK;  break;

        // all MODULE templates (created from eagle packages) are on front layer
        // until cloned.
        case COMMENT_N: layer = SILKSCREEN_N_FRONT; break;
        }
        */

        dwg->SetLayer( layer );

        dwg->SetTimeStamp( timeStamp( aTree ) );

        std::vector<wxPoint> pts;
        pts.reserve( aTree.size() );

        for( CITER vi = aTree.begin();  vi != aTree.end();  ++vi )
        {
            if( vi->first.compare( "vertex" ) ) // skip <xmlattr> node
                continue;

            EVERTEX v( vi->second );

            pts.push_back( wxPoint( kicad_x( v.x ), kicad_y( v.y ) ) );
        }

        dwg->SetPolyPoints( pts );

        dwg->SetStart0( *pts.begin() );
        dwg->SetEnd0( pts.back() );
    }
}


void EAGLE_PLUGIN::packageCircle( MODULE* aModule, CPTREE& aTree ) const
{
    ECIRCLE e( aTree );
    int     layer = kicad_layer( e.layer );

    EDGE_MODULE* gr = new EDGE_MODULE( aModule, S_CIRCLE );
    aModule->m_Drawings.PushBack( gr );

    gr->SetWidth( kicad( e.width ) );

    /*
    switch( layer )
    {
    case ECO1_N:    layer = SILKSCREEN_N_FRONT; break;
    case ECO2_N:    layer = SILKSCREEN_N_BACK;  break;
    }
    */

    gr->SetLayer( layer );
    gr->SetTimeStamp( timeStamp( aTree ) );

    gr->SetStart0( wxPoint( kicad_x( e.x ), kicad_y( e.y ) ) );
    gr->SetEnd0( wxPoint( kicad_x( e.x + e.radius ), kicad_y( e.y ) ) );
}


void EAGLE_PLUGIN::packageHole( MODULE* aModule, CPTREE& aTree ) const
{
    EHOLE   e( aTree );

    // we add a PAD_HOLE_NOT_PLATED pad to this module.
    D_PAD* pad = new D_PAD( aModule );
    aModule->m_Pads.PushBack( pad );

    pad->SetShape( PAD_ROUND );
    pad->SetAttribute( PAD_HOLE_NOT_PLATED );

    // Mechanical purpose only:
    // no offset, no net name, no pad name allowed
    // pad->SetOffset( wxPoint( 0, 0 ) );
    // pad->SetPadName( wxEmptyString );
    // pad->SetNetname( wxEmptyString );

    wxPoint padpos( kicad_x( e.x ), kicad_y( e.y ) );

    pad->SetPos0( padpos );
    pad->SetPosition( padpos + aModule->GetPosition() );

    wxSize  sz( kicad( e.drill ), kicad( e.drill ) );

    pad->SetDrillSize( sz );
    pad->SetSize( sz );

    pad->SetLayerMask( ALL_CU_LAYERS /* | SOLDERMASK_LAYER_BACK | SOLDERMASK_LAYER_FRONT */ );
}


void EAGLE_PLUGIN::packageSMD( MODULE* aModule, CPTREE& aTree ) const
{
    ESMD    e( aTree );
    int     layer = kicad_layer( e.layer );

    if( !IsValidCopperLayerIndex( layer ) )
    {
        return;
    }

    D_PAD*  pad = new D_PAD( aModule );
    aModule->m_Pads.PushBack( pad );

    pad->SetPadName( FROM_UTF8( e.name.c_str() ) );
    pad->SetShape( PAD_RECT );
    pad->SetAttribute( PAD_SMD );

    // pad's "Position" is not relative to the module's,
    // whereas Pos0 is relative to the module's but is the unrotated coordinate.

    wxPoint padpos( kicad_x( e.x ), kicad_y( e.y ) );

    pad->SetPos0( padpos );

    RotatePoint( &padpos, aModule->GetOrientation() );

    pad->SetPosition( padpos + aModule->GetPosition() );

    pad->SetSize( wxSize( kicad( e.dx ), kicad( e.dy ) ) );

    pad->SetLayer( layer );
    pad->SetLayerMask( LAYER_FRONT | SOLDERPASTE_LAYER_FRONT | SOLDERMASK_LAYER_FRONT );

    // Optional according to DTD
    if( e.roundness )    // set set shape to PAD_RECT above, in case roundness is not present
    {
        if( *e.roundness >= 75 )       // roundness goes from 0-100% as integer
        {
            if( e.dy == e.dx )
                pad->SetShape( PAD_ROUND );
            else
                pad->SetShape( PAD_OVAL );
        }
    }

    if( e.rot )
    {
        pad->SetOrientation( e.rot->degrees * 10 );
    }

    // don't know what stop, thermals, and cream should look like now.
}


void EAGLE_PLUGIN::loadSignals( CPTREE& aSignals )
{
    m_xpath->push( "signals.signal", "name" );

    int netCode = 1;

    for( CITER net = aSignals.begin();  net != aSignals.end();  ++net, ++netCode )
    {
        const std::string& nname = net->second.get<std::string>( "<xmlattr>.name" );
        wxString netName = FROM_UTF8( nname.c_str() );

        m_xpath->Value( nname.c_str() );

        m_board->AppendNet( new NETINFO_ITEM( m_board, netName, netCode ) );

        // (contactref | polygon | wire | via)*
        for( CITER it = net->second.begin();  it != net->second.end();  ++it )
        {
            if( !it->first.compare( "wire" ) )
            {
                m_xpath->push( "wire" );
                EWIRE   w( it->second );
                int     layer = kicad_layer( w.layer );

                if( IsValidCopperLayerIndex( layer ) )
                {
                    TRACK*  t = new TRACK( m_board );

                    t->SetTimeStamp( timeStamp( it->second ) );

                    t->SetPosition( wxPoint( kicad_x( w.x1 ), kicad_y( w.y1 ) ) );
                    t->SetEnd( wxPoint( kicad_x( w.x2 ), kicad_y( w.y2 ) ) );

                    t->SetWidth( kicad( w.width ) );
                    t->SetLayer( layer );
                    t->SetNet( netCode );

                    m_board->m_Track.Insert( t, NULL );
                }
                else
                {
                    // put non copper wires where the sun don't shine.
                }

                m_xpath->pop();
            }

            else if( !it->first.compare( "via" ) )
            {
                m_xpath->push( "via" );
                EVIA    v( it->second );

                int layer_front_most = kicad_layer( v.layer_front_most );
                int layer_back_most  = kicad_layer( v.layer_back_most );

                if( IsValidCopperLayerIndex( layer_front_most ) &&
                    IsValidCopperLayerIndex( layer_back_most ) )
                {
                    int     drillz = kicad( v.drill );
                    SEGVIA* via = new SEGVIA( m_board );
                    m_board->m_Track.Insert( via, NULL );

                    via->SetLayerPair( layer_front_most, layer_back_most );

                    // via diameters are externally controllable, not usually in a board:
                    // http://www.eaglecentral.ca/forums/index.php/mv/msg/34704/119478/
                    if( v.diam )
                    {
                        int kidiam = kicad( *v.diam );
                        via->SetWidth( kidiam );
                    }
                    else
                    {
                        int diameter = std::max( drillz + 2 * Mils2iu( 6 ), int( drillz * 2.0 ) );
                        via->SetWidth( diameter );
                    }

                    via->SetDrill( drillz );

                    if( layer_front_most == LAYER_N_FRONT && layer_back_most == LAYER_N_BACK )
                        via->SetShape( VIA_THROUGH );
                    else if( layer_front_most == LAYER_N_FRONT || layer_back_most == LAYER_N_BACK )
                        via->SetShape( VIA_MICROVIA );
                    else
                        via->SetShape( VIA_BLIND_BURIED );

                    via->SetTimeStamp( timeStamp( it->second ) );

                    wxPoint pos( kicad_x( v.x ), kicad_y( v.y ) );

                    via->SetPosition( pos  );
                    via->SetEnd( pos );

                    via->SetNet( netCode );

                    via->SetShape( S_CIRCLE );  // @todo should be in SEGVIA constructor
                }
                m_xpath->pop();
            }

            else if( !it->first.compare( "contactref" ) )
            {
                m_xpath->push( "contactref" );
                // <contactref element="RN1" pad="7"/>
                CPTREE& attribs = it->second.get_child( "<xmlattr>" );

                const std::string& reference = attribs.get<std::string>( "element" );
                const std::string& pad       = attribs.get<std::string>( "pad" );

                std::string key = makeKey( reference, pad ) ;

                // D(printf( "adding refname:'%s' pad:'%s' netcode:%d netname:'%s'\n", reference.c_str(), pad.c_str(), netCode, nname.c_str() );)

                m_pads_to_nets[ key ] = ENET( netCode, nname );

                m_xpath->pop();
            }

            else if( !it->first.compare( "polygon" ) )
            {
                m_xpath->push( "polygon" );
                EPOLYGON p( it->second );
                int      layer = kicad_layer( p.layer );

                if( IsValidCopperLayerIndex( layer ) )
                {
                    // use a "netcode = 0" type ZONE:
                    ZONE_CONTAINER* zone = new ZONE_CONTAINER( m_board );
                    m_board->Add( zone, ADD_APPEND );

                    zone->SetTimeStamp( timeStamp( it->second ) );
                    zone->SetLayer( layer );
                    zone->SetNet( netCode );
                    zone->SetNetName( netName );

                    int outline_hatch = CPolyLine::DIAGONAL_EDGE;

                    bool first = true;
                    for( CITER vi = it->second.begin();  vi != it->second.end();  ++vi )
                    {
                        if( vi->first.compare( "vertex" ) ) // skip <xmlattr> node
                            continue;

                        EVERTEX v( vi->second );

                        // the ZONE_CONTAINER API needs work, as you can see:
                        if( first )
                        {
                            zone->m_Poly->Start( layer,  kicad_x( v.x ), kicad_y( v.y ), outline_hatch );
                            first = false;
                        }
                        else
                            zone->AppendCorner( wxPoint( kicad_x( v.x ), kicad_y( v.y ) ) );
                    }

                    zone->m_Poly->Close();

                    zone->m_Poly->SetHatch( outline_hatch,
                                          Mils2iu( zone->m_Poly->GetDefaultHatchPitchMils() ) );

                    // clearances, etc.
                    zone->SetArcSegCount( 32 );     // @todo: should be a constructor default?
                    zone->SetMinThickness( kicad( p.width ) );

                    if( p.spacing )
                        zone->SetZoneClearance( kicad( *p.spacing ) );

                    if( p.rank )
                        zone->SetPriority( *p.rank );

                    // missing == yes per DTD.
                    bool thermals = !p.thermals || *p.thermals;
                    zone->SetPadConnection( thermals ? THERMAL_PAD : PAD_IN_ZONE );

                    int rank = p.rank ? *p.rank : 0;
                    zone->SetPriority( rank );
                }

                m_xpath->pop();     // "polygon"
            }
        }
    }

    m_xpath->pop();     // "signals.signal"
}


int EAGLE_PLUGIN::kicad_layer( int aEagleLayer )
{
    /* will assume this is a valid mapping for all eagle boards until I get paid more:

    <layers>
    <layer number="1" name="Top" color="4" fill="1" visible="yes" active="yes"/>
    <layer number="2" name="Route2" color="1" fill="3" visible="no" active="no"/>
    <layer number="3" name="Route3" color="4" fill="3" visible="no" active="no"/>
    <layer number="4" name="Route4" color="1" fill="4" visible="no" active="no"/>
    <layer number="5" name="Route5" color="4" fill="4" visible="no" active="no"/>
    <layer number="6" name="Route6" color="1" fill="8" visible="no" active="no"/>
    <layer number="7" name="Route7" color="4" fill="8" visible="no" active="no"/>
    <layer number="8" name="Route8" color="1" fill="2" visible="no" active="no"/>
    <layer number="9" name="Route9" color="4" fill="2" visible="no" active="no"/>
    <layer number="10" name="Route10" color="1" fill="7" visible="no" active="no"/>
    <layer number="11" name="Route11" color="4" fill="7" visible="no" active="no"/>
    <layer number="12" name="Route12" color="1" fill="5" visible="no" active="no"/>
    <layer number="13" name="Route13" color="4" fill="5" visible="no" active="no"/>
    <layer number="14" name="Route14" color="1" fill="6" visible="no" active="no"/>
    <layer number="15" name="Route15" color="4" fill="6" visible="no" active="no"/>
    <layer number="16" name="Bottom" color="1" fill="1" visible="yes" active="yes"/>
    <layer number="17" name="Pads" color="2" fill="1" visible="yes" active="yes"/>
    <layer number="18" name="Vias" color="14" fill="1" visible="yes" active="yes"/>
    <layer number="19" name="Unrouted" color="6" fill="1" visible="yes" active="yes"/>
    <layer number="20" name="Dimension" color="15" fill="1" visible="yes" active="yes"/>
    <layer number="21" name="tPlace" color="7" fill="1" visible="yes" active="yes"/>
    <layer number="22" name="bPlace" color="7" fill="1" visible="yes" active="yes"/>
    <layer number="23" name="tOrigins" color="15" fill="1" visible="yes" active="yes"/>
    <layer number="24" name="bOrigins" color="15" fill="1" visible="yes" active="yes"/>
    <layer number="25" name="tNames" color="7" fill="1" visible="yes" active="yes"/>
    <layer number="26" name="bNames" color="7" fill="1" visible="yes" active="yes"/>
    <layer number="27" name="tValues" color="7" fill="1" visible="no" active="yes"/>
    <layer number="28" name="bValues" color="7" fill="1" visible="no" active="yes"/>
    <layer number="29" name="tStop" color="2" fill="3" visible="no" active="yes"/>
    <layer number="30" name="bStop" color="5" fill="6" visible="no" active="yes"/>
    <layer number="31" name="tCream" color="7" fill="4" visible="no" active="yes"/>
    <layer number="32" name="bCream" color="7" fill="5" visible="no" active="yes"/>
    <layer number="33" name="tFinish" color="6" fill="3" visible="no" active="yes"/>
    <layer number="34" name="bFinish" color="6" fill="6" visible="no" active="yes"/>
    <layer number="35" name="tGlue" color="7" fill="4" visible="no" active="yes"/>
    <layer number="36" name="bGlue" color="7" fill="5" visible="no" active="yes"/>
    <layer number="37" name="tTest" color="7" fill="1" visible="no" active="yes"/>
    <layer number="38" name="bTest" color="7" fill="1" visible="no" active="yes"/>
    <layer number="39" name="tKeepout" color="4" fill="11" visible="no" active="yes"/>
    <layer number="40" name="bKeepout" color="1" fill="11" visible="no" active="yes"/>
    <layer number="41" name="tRestrict" color="4" fill="10" visible="no" active="yes"/>
    <layer number="42" name="bRestrict" color="1" fill="10" visible="no" active="yes"/>
    <layer number="43" name="vRestrict" color="2" fill="10" visible="no" active="yes"/>
    <layer number="44" name="Drills" color="7" fill="1" visible="no" active="yes"/>
    <layer number="45" name="Holes" color="7" fill="1" visible="no" active="yes"/>
    <layer number="46" name="Milling" color="3" fill="1" visible="no" active="yes"/>
    <layer number="47" name="Measures" color="7" fill="1" visible="no" active="yes"/>
    <layer number="48" name="Document" color="7" fill="1" visible="no" active="yes"/>
    <layer number="49" name="ReferenceLC" color="13" fill="1" visible="yes" active="yes"/>
    <layer number="50" name="ReferenceLS" color="12" fill="1" visible="yes" active="yes"/>
    <layer number="51" name="tDocu" color="7" fill="1" visible="yes" active="yes"/>
    <layer number="52" name="bDocu" color="7" fill="1" visible="yes" active="yes"/>
    <layer number="91" name="Nets" color="2" fill="1" visible="no" active="no"/>
    <layer number="92" name="Busses" color="1" fill="1" visible="no" active="no"/>
    <layer number="93" name="Pins" color="2" fill="1" visible="no" active="no"/>
    <layer number="94" name="Symbols" color="4" fill="1" visible="no" active="no"/>
    <layer number="95" name="Names" color="7" fill="1" visible="no" active="no"/>
    <layer number="96" name="Values" color="7" fill="1" visible="no" active="no"/>
    <layer number="97" name="Info" color="7" fill="1" visible="no" active="no"/>
    <layer number="98" name="Guide" color="6" fill="1" visible="no" active="no"/>
    </layers>

    */


    int kiLayer;

    // eagle copper layer:
    if( aEagleLayer >=1 && aEagleLayer <= 16 )
    {
        kiLayer = LAYER_N_FRONT - ( aEagleLayer - 1 );
    }

    else
    {
/*
#define FIRST_NO_COPPER_LAYER   16
#define ADHESIVE_N_BACK         16
#define ADHESIVE_N_FRONT        17
#define SOLDERPASTE_N_BACK      18
#define SOLDERPASTE_N_FRONT     19
#define SILKSCREEN_N_BACK       20
#define SILKSCREEN_N_FRONT      21
#define SOLDERMASK_N_BACK       22
#define SOLDERMASK_N_FRONT      23
#define DRAW_N                  24
#define COMMENT_N               25
#define ECO1_N                  26
#define ECO2_N                  27
#define EDGE_N                  28
#define LAST_NO_COPPER_LAYER    28
#define UNUSED_LAYER_29         29
#define UNUSED_LAYER_30         30
#define UNUSED_LAYER_31         31
*/
        // translate non-copper eagle layer to pcbnew layer
        switch( aEagleLayer )
        {
        case 20:    kiLayer = EDGE_N;               break;  // eagle says "Dimension" layer, but it's for board perimeter
        case 21:    kiLayer = SILKSCREEN_N_FRONT;   break;
        case 22:    kiLayer = SILKSCREEN_N_BACK;    break;
        case 25:    kiLayer = SILKSCREEN_N_FRONT;   break;
        case 26:    kiLayer = SILKSCREEN_N_BACK;    break;
        case 27:    kiLayer = SILKSCREEN_N_FRONT;   break;
        case 28:    kiLayer = SILKSCREEN_N_BACK;    break;
        case 29:    kiLayer = SOLDERMASK_N_FRONT;   break;
        case 30:    kiLayer = SOLDERMASK_N_BACK;    break;
        case 31:    kiLayer = SOLDERPASTE_N_FRONT;  break;
        case 32:    kiLayer = SOLDERPASTE_N_BACK;   break;
        case 33:    kiLayer = SOLDERMASK_N_FRONT;   break;
        case 34:    kiLayer = SOLDERMASK_N_BACK;    break;
        case 35:    kiLayer = ADHESIVE_N_FRONT;     break;
        case 36:    kiLayer = ADHESIVE_N_BACK;      break;
        case 49:    kiLayer = COMMENT_N;            break;
        case 50:    kiLayer = COMMENT_N;            break;

        // Packages show the future chip pins on SMD parts using layer 51.
        // This is an area slightly smaller than the PAD/SMD copper area.
        // Carry those visual aids into the MODULE on the drawing layer, not silkscreen.
        case 51:    kiLayer = DRAW_N;               break;
        case 52:    kiLayer = DRAW_N;               break;

        case 95:    kiLayer = ECO1_N;               break;
        case 96:    kiLayer = ECO2_N;               break;
        default:
            D( printf( "unsupported eagle layer: %d\n", aEagleLayer );)
            kiLayer = -1;       break;  // some layers do not map to KiCad
        }
    }

    return kiLayer;
}


void EAGLE_PLUGIN::centerBoard()
{
    /*
    if( m_props )
    {
        const wxString& pageWidth  = (*m_props)["page_width"];
        const wxString& pageHeight = (*m_props)["page_height"];

        if( pageWidth.size() && pageHeight.size() )
        {
            EDA_RECT bbbox = m_board->GetBoundingBox();
            int w = wxAtoi( pageWidth );
            int h = wxAtoi( pageHeight );

            m_board->Move( );
        }
    }
    */
}


/*
void EAGLE_PLUGIN::Save( const wxString& aFileName, BOARD* aBoard, PROPERTIES* aProperties )
{
    // Eagle lovers apply here.
}


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

*/
