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

#include <iostream>
#include <sstream>
#include <cmath>
#include <cstring>
#include <algorithm>

#include "3d_cache/sg/sg_node.h"
#include "plugins/3dapi/c3dmodel.h"

static const std::string node_names[V2_END + 1] = {
    "TXFM",
    "APP",
    "COL",
    "COLIDX",
    "FACE",
    "COORD",
    "COORDIDX",
    "NORM",
    "SHAPE",
    "INVALID"
};


static unsigned int node_counts[S3D::SGTYPE_END] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };


char const* S3D::GetNodeTypeName( S3D::SGTYPES aType )
{
    return node_names[aType].c_str();
}


static void getNodeName( S3D::SGTYPES nodeType, std::string& aName )
{
    if( nodeType < 0 || nodeType >= S3D::SGTYPE_END )
    {
        aName = node_names[S3D::SGTYPE_END];
        return;
    }

    unsigned int seqNum = node_counts[nodeType];
    ++node_counts[nodeType];

    std::ostringstream ostr;
    ostr << node_names[nodeType] << "_" << seqNum;
    aName = ostr.str();

    return;
}


SGNODE::SGNODE( SGNODE* aParent )
{
    m_Parent = aParent;
    m_Association = NULL;
    m_written = false;
    m_SGtype = S3D::SGTYPE_END;

    return;
}


SGNODE::~SGNODE()
{
    if( m_Parent )
        m_Parent->unlinkChildNode( this );

    if( m_Association )
        *m_Association = NULL;

    std::list< SGNODE* >::iterator sBP = m_BackPointers.begin();
    std::list< SGNODE* >::iterator eBP = m_BackPointers.end();

    while( sBP != eBP )
    {
        (*sBP)->unlinkRefNode( this );
        ++sBP;
    }

    return;
}


S3D::SGTYPES SGNODE::GetNodeType( void ) const
{
    return m_SGtype;
}


SGNODE* SGNODE::GetParent( void ) const
{
    return m_Parent;
}


const char* SGNODE::GetName( void )
{
    if( m_Name.empty() )
        getNodeName( m_SGtype, m_Name );

    return m_Name.c_str();
}


void SGNODE::SetName( const char *aName )
{
    if( NULL == aName || 0 == aName[0] )
        getNodeName( m_SGtype, m_Name );
    else
        m_Name = aName;

    return;
}


const char * SGNODE::GetNodeTypeName( S3D::SGTYPES aNodeType ) const
{
    return node_names[aNodeType].c_str();
}


void SGNODE::addNodeRef( SGNODE* aNode )
{
    std::list< SGNODE* >::iterator np =
        std::find( m_BackPointers.begin(), m_BackPointers.end(), aNode );

    if( np != m_BackPointers.end() )
        return;

    m_BackPointers.push_back( aNode );
    return;
}


void SGNODE::delNodeRef( SGNODE* aNode )
{
    std::list< SGNODE* >::iterator np =
        std::find( m_BackPointers.begin(), m_BackPointers.end(), aNode );

    if( np != m_BackPointers.end() )
    {
        m_BackPointers.erase( np );
        return;
    }

    std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
    std::cerr << " * [BUG] delNodeRef() did not find its target\n";

    return;
}


void SGNODE::AssociateWrapper( SGNODE** aWrapperRef )
{
    if( NULL == aWrapperRef )
    {
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] NULL handle\n";
        return;
    }

    if( *aWrapperRef != this )
    {
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] handle value does not match this object's pointer\n";
        return;
    }

    // if there is an existing association then break it and emit a warning
    // just in case the behavior is undesired
    if( m_Association )
    {
        *m_Association = NULL;
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [WARNING] association being broken with previous wrapper\n";
    }

    m_Association = aWrapperRef;

    return;
}

void SGNODE::DisassociateWrapper( SGNODE** aWrapperRef )
{
    if( !m_Association )
        return;

    if( !aWrapperRef )
    {
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] invalid handle value aWrapperRef\n";
        return;
    }

    if( *aWrapperRef != *m_Association || aWrapperRef != m_Association )
    {
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] *aWrapperRef (" << *aWrapperRef;
        std::cerr << ") does not match *m_Association (" << *m_Association << ") in type ";
        std::cerr << node_names[ m_SGtype] << "\n";
        std::cerr << " * [INFO] OR aWrapperRef(" << aWrapperRef << ") != m_Association(";
        std::cerr << m_Association << ")\n";
        std::cerr << " * [INFO] node name: " << GetName() << "\n";
        return;
    }

    m_Association = NULL;

    return;
}


void SGNODE::ResetNodeIndex( void )
{
    for( int i = 0; i < (int)S3D::SGTYPE_END; ++i )
        node_counts[i] = 1;

    return;
}


bool S3D::GetMatIndex( MATLIST& aList, SGNODE* aNode, int& aIndex )
{
    aIndex = 0;

    if( NULL == aNode || S3D::SGTYPE_APPEARANCE != aNode->GetNodeType() )
    {
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";

        if( NULL == aNode )
        {
            std::cerr << " * [BUG] aNode is NULL\n";
        }
        else
        {
            std::cerr << " * [BUG] invalid node type (" << aNode->GetNodeType();
            std::cerr << "), expected " << S3D::SGTYPE_APPEARANCE << "\n";
        }

        return false;
    }

    SGAPPEARANCE* node = (SGAPPEARANCE*)aNode;

    std::map< SGAPPEARANCE const*, int >::iterator it = aList.matmap.find( node );

    if( it != aList.matmap.end() )
    {
        aIndex = it->second;
        return true;
    }

    int idx = (int)aList.matorder.size();
    aList.matorder.push_back( node );
    aList.matmap.insert( std::pair< SGAPPEARANCE const*, int >( node, idx ) );
    aIndex = idx;

    return true;
}


void S3D::INIT_SMATERIAL( SMATERIAL& aMaterial )
{
    memset( &aMaterial, 0, sizeof( aMaterial ) );
    return;
}


void S3D::INIT_SMESH( SMESH& aMesh )
{
    memset( &aMesh, 0, sizeof( aMesh ) );
    return;
}


void S3D::INIT_S3DMODEL( S3DMODEL& aModel )
{
    memset( &aModel, 0, sizeof( aModel ) );
    return;
}


void S3D::FREE_SMESH( SMESH& aMesh)
{
    if( NULL != aMesh.m_Positions )
    {
        delete [] aMesh.m_Positions;
        aMesh.m_Positions = NULL;
    }

    if( NULL != aMesh.m_Normals )
    {
        delete [] aMesh.m_Normals;
        aMesh.m_Normals = NULL;
    }

    if( NULL != aMesh.m_Texcoords )
    {
        delete [] aMesh.m_Texcoords;
        aMesh.m_Texcoords = NULL;
    }

    if( NULL != aMesh.m_Color )
    {
        delete [] aMesh.m_Color;
        aMesh.m_Color = NULL;
    }

    if( NULL != aMesh.m_FaceIdx )
    {
        delete [] aMesh.m_FaceIdx;
        aMesh.m_FaceIdx = NULL;
    }

    aMesh.m_VertexSize = 0;
    aMesh.m_FaceIdxSize = 0;
    aMesh.m_MaterialIdx = 0;

    return;
}


void S3D::FREE_S3DMODEL( S3DMODEL& aModel )
{
    if( NULL != aModel.m_Materials )
    {
        delete [] aModel.m_Materials;
        aModel.m_Materials = NULL;
    }

    aModel.m_MaterialsSize = 0;

    if( NULL != aModel.m_Meshes )
    {
        for( unsigned int i = 0; i < aModel.m_MeshesSize; ++i )
            FREE_SMESH( aModel.m_Meshes[i] );

        delete [] aModel.m_Meshes;
        aModel.m_Meshes = NULL;
    }

    aModel.m_MeshesSize = 0;

    return;
}
