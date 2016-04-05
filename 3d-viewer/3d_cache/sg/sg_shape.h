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

/**
 * @file sg_shape.h
 * defines a complex 3D shape for a scenegraph object
 */


#ifndef SG_SHAPE_H
#define SG_SHAPE_H

#include <vector>
#include "3d_cache/sg/sg_node.h"

class SGAPPEARANCE;
class SGFACESET;

class SGSHAPE : public SGNODE
{
private:
    void unlinkNode( const SGNODE* aNode, bool isChild );
    bool addNode( SGNODE* aNode, bool isChild );

public:
    // owned node
    SGAPPEARANCE* m_Appearance;
    SGFACESET*    m_FaceSet;

    // referenced nodes
    SGAPPEARANCE* m_RAppearance;
    SGFACESET*    m_RFaceSet;

    void unlinkChildNode( const SGNODE* aNode );
    void unlinkRefNode( const SGNODE* aNode );

public:
    SGSHAPE( SGNODE* aParent );
    virtual ~SGSHAPE();

    virtual bool SetParent( SGNODE* aParent, bool notify = true );

    SGNODE* FindNode(const char *aNodeName, const SGNODE *aCaller);
    bool AddRefNode( SGNODE* aNode );
    bool AddChildNode( SGNODE* aNode );

    void ReNameNodes( void );
    bool WriteVRML( std::ofstream& aFile, bool aReuseFlag );

    bool WriteCache( std::ofstream& aFile, SGNODE* parentNode );
    bool ReadCache( std::ifstream& aFile, SGNODE* parentNode );

    bool Prepare( const glm::dmat4* aTransform,
        S3D::MATLIST& materials, std::vector< SMESH >& meshes );
};

/*
    p.107
    Shape {
        appearance  NULL
        geometry    NULL
    }
*/

#endif  // SG_SHAPE_H
