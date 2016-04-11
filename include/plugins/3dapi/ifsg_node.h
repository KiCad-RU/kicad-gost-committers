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
 * @file ifsg_node.h
 * defines the wrapper of the base class SG_NODE
 */

/*
 * NOTES:
 * 1. The IFSG wrapper classes shall be aimed at creating a VRML-like
 *    intermediate scenegraph representation. Although objects are
 *    readily created and added to the structure, no provision shall
 *    be made to inspect the structures in detail. For example the
 *    SCENEGRAPH class may contain various SGSHAPE and SCENEGRAPH
 *    nodes but there shall be no provision to extract those nodes.
 *    This was done because in principle all the detailed data shall
 *    only be handled within the SG* classes and only data processed
 *    via GetRenderData() shall be available via the wrappers.
 */

#ifndef IFSG_NODE_H
#define IFSG_NODE_H

#include "plugins/3dapi/sg_base.h"
#include "plugins/3dapi/sg_types.h"

class SGNODE;

/**
 * Class IFSG_NODE
 * represents the base class of all DLL-safe Scene Graph nodes
 */
class SGLIB_API IFSG_NODE
{
protected:
    SGNODE* m_node;

public:
    IFSG_NODE();
    virtual ~IFSG_NODE();

    // deleted operators
    IFSG_NODE( const IFSG_NODE& aParent ) = delete;
    IFSG_NODE( IFSG_NODE& aParent ) = delete;
    IFSG_NODE( volatile const IFSG_NODE& aParent ) = delete;
    IFSG_NODE( volatile IFSG_NODE& aParent ) = delete;
    IFSG_NODE& operator= ( const IFSG_NODE& ) = delete;

    /**
     * Function Destroy
     * deletes the object held by this wrapper
     */
    void Destroy( void );

    /**
     * Function Attach
     * associates a given SGNODE* with this wrapper
     */
    virtual bool Attach( SGNODE* aNode ) = 0;

    /**
     * Function NewNode
     * creates a new node to associate with this wrapper
     */
    virtual bool NewNode( SGNODE* aParent ) = 0;
    virtual bool NewNode( IFSG_NODE& aParent ) = 0;

    /**
     * Function GetRawPtr()
     * returns the raw internal SGNODE pointer
     */
    SGNODE* GetRawPtr( void );

    /**
     * Function GetNodeType
     * returns the type of this node instance
     */
    S3D::SGTYPES GetNodeType( void ) const;

    /**
     * Function GetParent
     * returns a pointer to the parent SGNODE of this object
     * or NULL if the object has no parent (ie. top level transform).
     */
    SGNODE* GetParent( void ) const;

    /**
     * Function SetParent
     * sets the parent SGNODE of this object.
     *
     * @param aParent [in] is the desired parent node
     * @return true if the operation succeeds; false if
     * the given node is not allowed to be a parent to
     * the derived object
     */
    bool SetParent( SGNODE* aParent );

    /**
     * Function GetName
     * returns a pointer to the node name (NULL if no name assigned)
     */
    const char* GetName( void );

    /**
     * Function SetName
     * sets the node's name; if the pointer passed is NULL
     * then the node's name is erased
     *
     * @return true on success
     */
    bool SetName( const char *aName );

    /**
     * Function GetNodeTypeName
     * returns the text representation of the node type
     * or NULL if the node somehow has an invalid type
     */
    const char * GetNodeTypeName( S3D::SGTYPES aNodeType ) const;

    /**
     * Function FindNode searches the tree of linked nodes and returns a
     * reference to the first node found with the given name. The reference
     * is then typically added to another node via AddRefNode().
     *
     * @param aNodeName is the name of the node to search for
     * @param aCaller is a pointer to the node invoking this function
     * @return is a valid node pointer on success, otherwise NULL
     */
    SGNODE* FindNode( const char *aNodeName );

    /**
     * Function AddRefNode
     * adds a reference to an existing node which is not owned by
     * (not a child of) this node.
     *
     * @return true on success
     */
    bool AddRefNode( SGNODE* aNode );
    bool AddRefNode( IFSG_NODE& aNode );

    /**
     * Function AddChildNode
     * adds a node as a child owned by this node.
     *
     * @return true on success
     */
    bool AddChildNode( SGNODE* aNode );
    bool AddChildNode( IFSG_NODE& aNode );
};

#endif  // IFSG_NODE_H
