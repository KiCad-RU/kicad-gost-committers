/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 CERN
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
 * @file pcb_parser.h
 * @brief Pcbnew s-expression file format parser definition.
 */

#ifndef _PCBNEW_PARSER_H_
#define _PCBNEW_PARSER_H_

#include <pcb_lexer.h>
#include <hashtables.h>

using namespace PCB_KEYS_T;


class BOARD;
class BOARD_ITEM;
class D_PAD;
class DIMENSION;
class DRAWSEGMENT;
class EDGE_MODULE;
class TEXTE_MODULE;
class TEXTE_PCB;
class MODULE;
class PCB_TARGET;
class S3D_MASTER;
class ZONE_CONTAINER;
class FPL_CACHE;



/**
 * Class PCB_PARSER
 * reads a Pcbnew s-expression formatted #LINE_READER object and returns the appropriate
 * #BOARD_ITEM object.
 */
class PCB_PARSER : public PCB_LEXER
{
    typedef KEYWORD_MAP     LAYER_MAP;

    BOARD*          m_board;
    LAYER_MAP       m_layerIndices;     ///< map layer name to it's index
    LAYER_MAP       m_layerMasks;       ///< map layer names to their masks


    /**
     * Function init
     * clears and re-establishes m_layerMap with the default layer names.
     * m_layerMap will have some of its entries overwritten whenever a (new) board
     * is encountered.
     */
    void init();

    void parseHeader() throw( IO_ERROR, PARSE_ERROR );
    void parseGeneralSection() throw( IO_ERROR, PARSE_ERROR );
    void parsePAGE_INFO() throw( IO_ERROR, PARSE_ERROR );
    void parseTITLE_BLOCK() throw( IO_ERROR, PARSE_ERROR );
    void parseLayers() throw( IO_ERROR, PARSE_ERROR );
    void parseSetup() throw( IO_ERROR, PARSE_ERROR );
    void parseNETINFO_ITEM() throw( IO_ERROR, PARSE_ERROR );
    void parseNETCLASS() throw( IO_ERROR, PARSE_ERROR );
    DRAWSEGMENT* parseDRAWSEGMENT() throw( IO_ERROR, PARSE_ERROR );
    TEXTE_PCB* parseTEXTE_PCB() throw( IO_ERROR, PARSE_ERROR );
    DIMENSION* parseDIMENSION() throw( IO_ERROR, PARSE_ERROR );
    MODULE* parseMODULE() throw( IO_ERROR, PARSE_ERROR );
    TEXTE_MODULE* parseTEXTE_MODULE() throw( IO_ERROR, PARSE_ERROR );
    EDGE_MODULE* parseEDGE_MODULE() throw( IO_ERROR, PARSE_ERROR );
    D_PAD* parseD_PAD() throw( IO_ERROR, PARSE_ERROR );
    TRACK* parseTRACK() throw( IO_ERROR, PARSE_ERROR );
    SEGVIA* parseSEGVIA() throw( IO_ERROR, PARSE_ERROR );
    ZONE_CONTAINER* parseZONE_CONTAINER() throw( IO_ERROR, PARSE_ERROR );
    PCB_TARGET* parsePCB_TARGET() throw( IO_ERROR, PARSE_ERROR );
    BOARD* parseBOARD() throw( IO_ERROR, PARSE_ERROR );


    /**
     * Function lookUpLayer
     * parses the current token for the layer definition of a #BOARD_ITEM object.
     *
     * @param aMap is the LAYER_MAP to use for the lookup.
     *
     * @throw IO_ERROR if the layer is not valid.
     * @throw PARSE_ERROR if the layer syntax is incorrect.
     * @return int - The result of the parsed #BOARD_ITEM layer or set designator.
     */
    int lookUpLayer( const LAYER_MAP& aMap ) throw( PARSE_ERROR, IO_ERROR );

    /**
     * Function parseBoardItemLayer
     * parses the layer definition of a #BOARD_ITEM object.
     *
     * @throw IO_ERROR if the layer is not valid.
     * @throw PARSE_ERROR if the layer syntax is incorrect.
     * @return The index the parsed #BOARD_ITEM layer.
     */
    int parseBoardItemLayer() throw( IO_ERROR, PARSE_ERROR );

    /**
     * Function parseBoardItemLayersAsMask
     * parses the layers definition of a #BOARD_ITEM object.
     *
     * @throw IO_ERROR if any of the layers is not valid.
     * @throw PARSE_ERROR if the layers syntax is incorrect.
     * @return The mask of layers the parsed #BOARD_ITEM is on.
     */
    int parseBoardItemLayersAsMask() throw( PARSE_ERROR, IO_ERROR );

    /**
     * Function parseXY
     * parses a coordinate pair (xy X Y) in board units (mm).
     *
     * The parser checks if the previous token was T_LEFT and parses the remainder of
     * the token syntax.  This is used when parsing a list of coordinate points.  This
     * way the parser can be used in either case.
     *
     * @throw PARSE_ERROR if the coordinate pair syntax is incorrect.
     * @return A wxPoint object containing the coordinate pair.
     */
    wxPoint parseXY() throw( PARSE_ERROR );

    void parseXY( int* aX, int* aY ) throw( PARSE_ERROR );

    /**
     * Function parseEDA_TEXT
     * parses the common settings for any object derived from #EDA_TEXT.
     *
     * @throw PARSE_ERROR if the text syntax is not valid.
     * @param aText A point to the #EDA_TEXT object to save the parsed settings into.
     */
    void parseEDA_TEXT( EDA_TEXT* aText ) throw( PARSE_ERROR );

    S3D_MASTER* parse3DModel() throw( PARSE_ERROR );

    /**
     * Function parseDouble
     * parses the current token as an ASCII numeric string with possible leading whitespace into
     * a double precision floating point number.
     *
     * @throw IO_ERROR if an error occurs attempting to convert the current token.
     * @return The result of the parsed token.
     */
    double parseDouble() throw( IO_ERROR );

    inline double parseDouble( const char* aExpected ) throw( IO_ERROR )
    {
        NeedNUMBER( aExpected );
        return parseDouble();
    }

    inline double parseDouble( T aToken ) throw( IO_ERROR )
    {
        return parseDouble( GetTokenText( aToken ) );
    }

    inline int parseBoardUnits() throw( IO_ERROR )
    {
        // There should be no major rounding issues here,
        // since the values in the file are in mm
        // and get converted to nano-meters.
        return KiROUND( parseDouble() * IU_PER_MM );
    }

    inline int parseBoardUnits( const char* aExpected ) throw( PARSE_ERROR )
    {
        // Use here KiROUND, not KIROUND (see comments about them)
        // when having a function as argument, because it will be called twice
        // with KIROUND
        return KiROUND( parseDouble( aExpected ) * IU_PER_MM );
    }

    inline int parseBoardUnits( T aToken ) throw( PARSE_ERROR )
    {
        return parseBoardUnits( GetTokenText( aToken ) );
    }

    inline int parseInt() throw( PARSE_ERROR )
    {
        return (int)strtol( CurText(), NULL, 10 );
    }

    inline int parseInt( const char* aExpected ) throw( PARSE_ERROR )
    {
        NeedNUMBER( aExpected );
        return parseInt();
    }

    inline long parseHex() throw( PARSE_ERROR )
    {
        NextTok();
        return strtol( CurText(), NULL, 16 );
    }

    bool parseBool() throw( PARSE_ERROR );


public:

    PCB_PARSER( LINE_READER* aReader = NULL ) :
        PCB_LEXER( aReader ),
        m_board( 0 )
    {
        init();
    }

    // ~PCB_PARSER() {}

    /**
     * Function SetLineReader
     * sets @a aLineReader into the parser, and returns the previous one, if any.
     * @param aReader is what to read from for tokens, no ownership is received.
     * @return LINE_READER* - previous LINE_READER or NULL if none.
     */
    LINE_READER* SetLineReader( LINE_READER* aReader )
    {
        LINE_READER* ret = PopReader();
        PushReader( aReader );
        return ret;
    }

    void SetBoard( BOARD* aBoard )
    {
        init();
        m_board = aBoard;
    }

    BOARD_ITEM* Parse() throw( IO_ERROR, PARSE_ERROR );
};


#endif    // _PCBNEW_PARSER_H_
