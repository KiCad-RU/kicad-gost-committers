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
 * @file scenegraph.h
 * defines the basic data set required to represent a 3D model;
 * this model must remain compatible with VRML2.0 in order to
 * facilitate VRML export of scene graph data created by avaiable
 * 3D plugins.
 */


#ifndef SCENE_GRAPH_H
#define SCENE_GRAPH_H

#include <vector>
#include "3d_cache/sg/sg_node.h"

class SGSHAPE;

class SCENEGRAPH : public SGNODE
{
private:
    // The following are items which may be defined for reuse
    // in a VRML output file. They do not necessarily correspond
    // to the use of DEF within a VRML input file; it is the
    // responsibility of the plugin to perform any necessary
    // conversions to comply with the restrictions imposed by
    // this scene graph structure
    std::vector< SCENEGRAPH* > m_Transforms;   // local Transform nodes
    std::vector< SGSHAPE* > m_Shape;           // local Shape nodes

    std::vector< SCENEGRAPH* > m_RTransforms;   // referenced Transform nodes
    std::vector< SGSHAPE* > m_RShape;           // referenced Shape nodes

    void unlinkNode( const SGNODE* aNode, bool isChild );
    bool addNode( SGNODE* aNode, bool isChild );

public:
    void unlinkChildNode( const SGNODE* aNode );
    void unlinkRefNode( const SGNODE* aNode );

public:
    // note: order of transformation is Translate, Rotate, Offset
    SGPOINT  center;
    SGPOINT  translation;
    SGVECTOR rotation_axis;
    double   rotation_angle;    // radians
    SGPOINT  scale;
    SGVECTOR scale_axis;
    double   scale_angle;       // radians

    SCENEGRAPH( SGNODE* aParent );
    virtual ~SCENEGRAPH();

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
    p.120
    Transform {
        center              0 0 0
        children            []
        rotation            0 0 1 0
        scale               1 1 1
        scaleOrientation    0 0 1 0
        translation         0 0 0
        bboxCenter          0 0 0
        bboxSize            -1 -1 -1
    }
*/

#endif  // SCENE_GRAPH_H
