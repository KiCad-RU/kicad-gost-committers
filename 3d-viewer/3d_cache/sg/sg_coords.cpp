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
#include "3d_cache/sg/sg_coords.h"
#include "3d_cache/sg/sg_helpers.h"
#include "3d_cache/sg/sg_normals.h"
#include "3d_cache/sg/sg_faceset.h"


SGCOORDS::SGCOORDS( SGNODE* aParent ) : SGNODE( aParent )
{
    m_SGtype = S3D::SGTYPE_COORDS;

    if( NULL != aParent && S3D::SGTYPE_FACESET != aParent->GetNodeType() )
    {
        m_Parent = NULL;

#ifdef DEBUG
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] inappropriate parent to SGCOORDS (type ";
        std::cerr << aParent->GetNodeType() << ")\n";
#endif
    }
    else if( NULL != aParent && S3D::SGTYPE_FACESET == aParent->GetNodeType() )
    {
        m_Parent->AddChildNode( this );
    }

    return;
}


SGCOORDS::~SGCOORDS()
{
    coords.clear();
    return;
}


bool SGCOORDS::SetParent( SGNODE* aParent, bool notify )
{
    if( NULL != m_Parent )
    {
        if( aParent == m_Parent )
            return true;

        // handle the change in parents
        if( notify )
            m_Parent->unlinkChildNode( this );

        m_Parent = NULL;

        if( NULL == aParent )
            return true;
    }

    // only a SGFACESET may be parent to a SGCOORDS
    if( NULL != aParent && S3D::SGTYPE_FACESET != aParent->GetNodeType() )
        return false;

    m_Parent = aParent;

    if( m_Parent )
        m_Parent->AddChildNode( this );

    return true;
}


SGNODE* SGCOORDS::FindNode(const char *aNodeName, const SGNODE *aCaller)
{
    if( NULL == aNodeName || 0 == aNodeName[0] )
        return NULL;

    if( !m_Name.compare( aNodeName ) )
        return this;

    return NULL;
}


void SGCOORDS::unlinkChildNode( const SGNODE* aCaller )
{
    std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
    std::cerr << " * [BUG] unexpected code branch; node should have no children or refs\n";
    return;
}


void SGCOORDS::unlinkRefNode( const SGNODE* aCaller )
{
    std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
    std::cerr << " * [BUG] unexpected code branch; node should have no children or refs\n";
    return;
}


bool SGCOORDS::AddRefNode( SGNODE* aNode )
{
    std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
    std::cerr << " * [BUG] this node does not accept children or refs\n";
    return false;
}


bool SGCOORDS::AddChildNode( SGNODE* aNode )
{
    std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
    std::cerr << " * [BUG] this node does not accept children or refs\n";
    return false;
}


bool SGCOORDS::GetCoordsList( size_t& aListSize, SGPOINT*& aCoordsList )
{
    if( coords.empty() )
    {
        aListSize = 0;
        aCoordsList = NULL;
        return false;
    }

    aListSize = coords.size();
    aCoordsList = &coords[0];
    return true;
}


void SGCOORDS::SetCoordsList( size_t aListSize, const SGPOINT* aCoordsList )
{
    coords.clear();

    if( 0 == aListSize || NULL == aCoordsList )
        return;

    for( size_t i = 0; i < aListSize; ++i )
        coords.push_back( aCoordsList[i] );

    return;
}


void SGCOORDS::AddCoord( double aXValue, double aYValue, double aZValue )
{
    coords.push_back( SGPOINT( aXValue, aYValue, aZValue ) );
    return;
}


void SGCOORDS::AddCoord( const SGPOINT& aPoint )
{
    coords.push_back( aPoint );
    return;
}


void SGCOORDS::ReNameNodes( void )
{
    m_written = false;

    // rename this node
    m_Name.clear();
    GetName();
}


bool SGCOORDS::WriteVRML( std::ofstream& aFile, bool aReuseFlag )
{
    if( coords.empty() )
        return false;

    if( aReuseFlag )
    {
        if( !m_written )
        {
            aFile << "  coord DEF " << GetName() << " Coordinate { point [\n  ";
            m_written = true;
        }
        else
        {
            aFile << "  coord USE " << GetName() << "\n";
            return true;
        }
    }
    else
    {
        aFile << "  coord Coordinate { point [\n  ";
    }

    std::string tmp;
    size_t n = coords.size();
    bool nline = false;

    for( size_t i = 0; i < n; )
    {
        S3D::FormatPoint( tmp, coords[i] );
        aFile << tmp ;
        ++i;

        if( i < n )
        {
            aFile << ",";

            if( nline )
            {
                aFile << "\n  ";
                nline = false;
            }
            else
            {
                nline = true;
            }

        }
    }

    aFile << "] }\n";

    return true;
}


bool SGCOORDS::WriteCache( std::ofstream& aFile, SGNODE* parentNode )
{
    if( NULL == parentNode )
    {
        if( NULL == m_Parent )
        {
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [BUG] corrupt data; m_aParent is NULL\n";
            return false;
        }

        SGNODE* np = m_Parent;

        while( NULL != np->GetParent() )
            np = np->GetParent();

        return np->WriteCache( aFile, NULL );
    }

    if( parentNode != m_Parent )
    {
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] corrupt data; parentNode != m_aParent\n";
        return false;
    }

    if( !aFile.good() )
    {
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [INFO] bad stream\n";
        return false;
    }

    aFile << "[" << GetName() << "]";
    size_t npts = coords.size();
    aFile.write( (char*)&npts, sizeof(size_t) );

    for( size_t i = 0; i < npts; ++i )
        S3D::WritePoint( aFile, coords[i] );

    if( aFile.fail() )
        return false;

    return true;
}


bool SGCOORDS::ReadCache( std::ifstream& aFile, SGNODE* parentNode )
{
    if( !coords.empty() )
    {
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] non-empty node\n";
        return false;
    }

    size_t npts;
    aFile.read( (char*)&npts, sizeof(size_t) );
    SGPOINT tmp;

    if( aFile.fail() )
        return false;

    for( size_t i = 0; i < npts; ++i )
    {
        if( !S3D::ReadPoint( aFile, tmp ) || aFile.fail() )
            return false;

        coords.push_back( tmp );
    }

    return true;
}


bool SGCOORDS::CalcNormals( void )
{
    if( NULL == m_Parent )
        return false;

    // the parent and all references must have indices; collect all
    // indices into one std::vector<>
    std::vector< int > ilist;
    ((SGFACESET*)m_Parent)->GatherCoordIndices( ilist );

    std::list< SGNODE* >::iterator sB = m_BackPointers.begin();
    std::list< SGNODE* >::iterator eB = m_BackPointers.end();

    while( sB != eB )
    {
        SGFACESET* fp = (SGFACESET*)(*sB);
        fp->GatherCoordIndices( ilist );
        ++sB;
    }

    SGNORMALS* np = ((SGFACESET*)m_Parent)->m_Normals;

    if( !np )
        np = new SGNORMALS( m_Parent );

    if( S3D::CalcTriangleNormals( coords, ilist, np->norms )  )
        return true;

    delete np;

    return false;
}
