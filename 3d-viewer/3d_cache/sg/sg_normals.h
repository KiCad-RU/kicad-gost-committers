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
 * @file sg_normals.h
 * defines a set of vertex normals for a scene graph object
 */

#ifndef SG_NORMALS_H
#define SG_NORMALS_H

#include <vector>
#include "3d_cache/sg/sg_node.h"

class SGNORMALS : public SGNODE
{
public:
    std::vector< SGVECTOR > norms;

    void unlinkChildNode( const SGNODE* aNode );
    void unlinkRefNode( const SGNODE* aNode );

public:
    SGNORMALS( SGNODE* aParent );
    virtual ~SGNORMALS();

    virtual bool SetParent( SGNODE* aParent, bool notify = true );

    SGNODE* FindNode(const char *aNodeName, const SGNODE *aCaller);
    bool AddRefNode( SGNODE* aNode );
    bool AddChildNode( SGNODE* aNode );

    bool GetNormalList( size_t& aListSize, SGVECTOR*& aNormalList );
    void SetNormalList( size_t aListSize, const SGVECTOR* aNormalList );
    void AddNormal( double aXValue, double aYValue, double aZValue );
    void AddNormal( const SGVECTOR& aNormal );

    void ReNameNodes( void );
    bool WriteVRML( std::ofstream& aFile, bool aReuseFlag );

    bool WriteCache( std::ofstream& aFile, SGNODE* parentNode );
    bool ReadCache( std::ifstream& aFile, SGNODE* parentNode );
};

#endif  // SG_NORMALS_H
