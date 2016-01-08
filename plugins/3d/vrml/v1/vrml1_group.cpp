/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015-2016 Cirilo Bernardo <cirilo.bernardo@gmail.com>
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

// note: this was copied from the vrml1_separator class. the difference
// between a separator and a group is that a group propagates its
// current settings to its parent. While it would be possible to
// implement the separator as a derived class, it is easy enough to
// simply duplicate the code

#include <iostream>

#include "vrml1_base.h"
#include "vrml1_group.h"
#include "plugins/3dapi/ifsg_all.h"


WRL1GROUP::WRL1GROUP( NAMEREGISTER* aDictionary ) : WRL1NODE( aDictionary )
{
    m_Type = WRL1_GROUP;
    return;
}


WRL1GROUP::WRL1GROUP( NAMEREGISTER* aDictionary, WRL1NODE* aParent ) :
    WRL1NODE( aDictionary )
{
    m_Type = WRL1_GROUP;
    m_Parent = aParent;

    if( NULL != m_Parent )
        m_Parent->AddChildNode( this );

    return;
}


WRL1GROUP::~WRL1GROUP()
{
    #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 2 )
    std::cerr << " * [INFO] Destroying Group with " << m_Children.size();
    std::cerr << " children, " << m_Refs.size() << " references and ";
    std::cerr << m_BackPointers.size() << " backpointers\n";
    #endif
    return;
}


// functions inherited from WRL1NODE
bool WRL1GROUP::Read( WRLPROC& proc, WRL1BASE* aTopNode )
{
    if( NULL == aTopNode )
    {
        #ifdef DEBUG_VRML1
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] aTopNode is NULL\n";
        #endif
        return false;
    }

    size_t line, column;
    proc.GetFilePosData( line, column );

    char tok = proc.Peek();

    if( proc.eof() )
    {
        #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 1 )
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [INFO] bad file format; unexpected eof at line ";
        std::cerr << line << ", column " << column << "\n";
        #endif
        return false;
    }

    if( '{' != tok )
    {
        #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 1 )
        std::cerr << proc.GetError() << "\n";
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [INFO] bad file format; expecting '{' but got '" << tok;
        std::cerr  << "' at line " << line << ", column " << column << "\n";
        #endif

        return false;
    }

    proc.Pop();
    std::string glob;

    while( true )
    {
        if( proc.Peek() == '}' )
        {
            proc.Pop();
            break;
        }

        proc.GetFilePosData( line, column );

        if( !aTopNode->ReadNode( proc, this, NULL ) )
        {
            #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 1 )
            std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
            std::cerr << " * [INFO] bad file format; unexpected eof at line ";
            std::cerr << line << ", column " << column << "\n";
            #endif

            return false;
        }

        if( proc.Peek() == ',' )
            proc.Pop();

    }   // while( true ) -- reading contents of Group{}

    return true;
}


SGNODE* WRL1GROUP::TranslateToSG( SGNODE* aParent, WRL1STATUS* sp )
{
    #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 2 )
    std::cerr << " * [INFO] Translating Group with " << m_Children.size();
    std::cerr << " children, " << m_Refs.size() << " references and ";
    std::cerr << m_BackPointers.size() << " backpointers (total ";
    std::cerr << m_Items.size() << " items)\n";
    #endif

    std::cerr << "XXX: GROUP!!!!!!!!!!\n";

    if( !m_Parent )
    {
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] Group has no parent\n";

        return NULL;
    }

    if( WRL1_BASE != m_Parent->GetNodeType() )
    {
        if( NULL == sp )
        {
            #if defined( DEBUG_VRML1 ) && ( DEBUG_VRML1 > 1 )
            std::cerr << " * [INFO] bad model: no base data given\n";
            #endif

            return NULL;
        }
    }
    else
    {
        m_current.Init();
        sp = &m_current;
    }

    S3D::SGTYPES ptype = S3D::GetSGNodeType( aParent );

    if( NULL != aParent && ptype != S3D::SGTYPE_TRANSFORM )
    {
        #ifdef DEBUG_VRML1
        std::cerr << __FILE__ << ": " << __FUNCTION__ << ": " << __LINE__ << "\n";
        std::cerr << " * [BUG] Group does not have a Transform parent (parent ID: ";
        std::cerr << ptype << ")\n";
        #endif

        return NULL;
    }

    IFSG_TRANSFORM txNode( aParent );
    bool hasContent = false;

    std::list< WRL1NODE* >::iterator sI = m_Items.begin();
    std::list< WRL1NODE* >::iterator eI = m_Items.end();

    SGNODE* node = txNode.GetRawPtr();

    while( sI != eI )
    {
        if( NULL != (*sI)->TranslateToSG( node, sp ) )
            hasContent = true;

        ++sI;
    }

    if( !hasContent )
    {
        txNode.Destroy();
        return NULL;
    }

    return node;
}
