/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2016 Cirilo Bernardo <cirilo.bernardo@gmail.com>
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
 * @file vrml2_faceset.h
 */


#ifndef VRML2_FACESET_H
#define VRML2_FACESET_H

#include <vector>

#include "vrml2_node.h"

class WRL2BASE;
class SGNODE;

/**
 * Class WRL2FACESET
 */
class WRL2FACESET : public WRL2NODE
{
private:
    WRL2NODE* color;
    WRL2NODE* coord;
    WRL2NODE* normal;
    WRL2NODE* texCoord;

    bool ccw;
    bool colorPerVertex;
    bool convex;
    bool normalPerVertex;
    bool solid;

    std::vector< int > colorIndex;
    std::vector< int > coordIndex;
    std::vector< int > normalIndex;

    float creaseAngle;
    float creaseLimit;


    /**
     * Function checkNodeType
     * returns true if the node type is a valid subnode of FaceSet
     */
    bool checkNodeType( WRL2NODES aType );

    void setDefaults( void );

public:

    // functions inherited from WRL2NODE
    bool isDangling( void );


    // overloads
    void unlinkChildNode( const WRL2NODE* aNode );
    void unlinkRefNode( const WRL2NODE* aNode );

public:
    WRL2FACESET();
    WRL2FACESET( WRL2NODE* aParent );
    virtual ~WRL2FACESET();

    // functions inherited from WRL2NODE
    bool Read( WRLPROC& proc, WRL2BASE* aTopNode );
    bool AddRefNode( WRL2NODE* aNode );
    bool AddChildNode( WRL2NODE* aNode );
    SGNODE* TranslateToSG( SGNODE* aParent );

    /**
     * Function HasColors
     * returns true if the face set has a color node
     */
    bool HasColors( void );
};

#endif  // VRML2_FACESET_H
