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
 * @file sg_node.h
 * defines the base class of the intermediate scene graph NODE
 */


#ifndef SG_NODE_H
#define SG_NODE_H

#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <glm/glm.hpp>

#include "plugins/3dapi/c3dmodel.h"
#include "plugins/3dapi/sg_base.h"
#include "plugins/3dapi/sg_types.h"

class SGNODE;
class SGAPPEARANCE;

namespace S3D
{
    /**
     * Function GetNodeTypeName
     * returns the name of the given type of node
     */
    char const* GetNodeTypeName( S3D::SGTYPES aType );

    struct MATLIST
    {
        std::vector< SGAPPEARANCE const* > matorder;    // materials in order of addition
        std::map< SGAPPEARANCE const*, int > matmap;    // mapping from material to index
    };

    bool GetMatIndex( MATLIST& aList, SGNODE* aNode, int& aIndex );

    void INIT_SMATERIAL( SMATERIAL& aMaterial );
    void INIT_SMESH( SMESH& aMesh );
    void INIT_S3DMODEL( S3DMODEL& aModel );

    void FREE_SMESH( SMESH& aMesh);
    void FREE_S3DMODEL( S3DMODEL& aModel );
};


/**
 * Class SGNODE
 * represents the base class of all Scene Graph nodes
 */
class SGNODE
{
private:
    SGNODE** m_Association;                 // handle to the instance held by a wrapper

protected:
    std::list< SGNODE* > m_BackPointers;    // nodes which hold a reference to this
    SGNODE* m_Parent;       // pointer to parent node; may be NULL for top level transform
    S3D::SGTYPES m_SGtype;  // type of SG node
    std::string m_Name;     // name to use for referencing the entity by name
    bool m_written;         // set true when the object has been written after a ReNameNodes()

public:
    /**
     * Function unlinkChild
     * removes references to an owned child; it is invoked by the child upon destruction
     * to ensure that the parent has no invalid references.
     *
     * @param aNode is the child which is being deleted
     */
    virtual void unlinkChildNode( const SGNODE* aNode ) = 0;

    /**
     * Function unlinkRef
     * removes pointers to a referenced node; it is invoked by the referenced node
     * upon destruction to ensure that the referring node has no invalid references.
     *
     * @param aNode is the node which is being deleted
     */
    virtual void unlinkRefNode( const SGNODE* aNode ) = 0;

    /**
     * Function addNodeRef
     * adds a pointer to a node which references, but does not own, this node.
     * Such back-pointers are required to ensure that invalidated references
     * are removed when a node is deleted
     *
     * @param aNode is the node holding a reference to this object
     */
    void addNodeRef( SGNODE* aNode );

    /**
     * Function delNodeRef
     * removes a pointer to a node which references, but does not own, this node.
     *
     * @param aNode is the node holding a reference to this object
     */
    void delNodeRef( const SGNODE* aNode );

    /**
     * Function IsWritten
     * returns true if the object had already been written to a
     * cache file or VRML file; for internal use only.
     */
    bool isWritten( void )
    {
        return m_written;
    }

public:
    SGNODE( SGNODE* aParent );
    virtual ~SGNODE();

    /**
     * Function GetNodeType
     * returns the type of this node instance
     */
    S3D::SGTYPES GetNodeType( void ) const;

    /**
     * Function GetParent
     * returns a pointer to the parent SGNODE of this object
     * or NULL if the object has no parent (ie. top level transform)
     */
    SGNODE* GetParent( void ) const;

    /**
     * Function SetParent
     * sets the parent SGNODE of this object.
     *
     * @param aParent [in] is the desired parent node
     * @return true if the operation succeeds; false if
     * the given node is not allowed to be a parent to
     * the derived object.
     */
    virtual bool SetParent( SGNODE* aParent, bool notify = true ) = 0;

    /**
     * Function SwapParent
     * swaps the ownership with the given parent. This operation
     * may be required when reordering nodes for optimization.
     *
     * @param aNewParent [in] will become the new parent to the
     * object; it must be the same type as the parent of this
     * instance.
     */
    bool SwapParent( SGNODE* aNewParent );

    const char* GetName( void );
    void SetName(const char *aName);

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
    virtual SGNODE* FindNode( const char *aNodeName, const SGNODE *aCaller ) = 0;

    virtual bool AddRefNode( SGNODE* aNode ) = 0;

    virtual bool AddChildNode( SGNODE* aNode ) = 0;

    /**
     * Function AssociateWrapper
     * associates this object with a handle to itself; this handle
     * is typically held by an IFSG* wrapper and the pointer which
     * it refers to is set to NULL upon destruction of this object.
     * This mechanism provides a scheme by which a wrapper can be
     * notified of the destruction of the object which it wraps.
     */
    void AssociateWrapper( SGNODE** aWrapperRef );

    /**
     * Function DisassociateWrapper
     * removes the association between an IFSG* wrapper
     * object and this object.
     */
    void DisassociateWrapper( SGNODE** aWrapperRef );

    /**
     * Function ResetNodeIndex
     * resets the global SG* node indices in preparation for
     * Write() operations
     */
    void ResetNodeIndex( void );

    /**
     * Function ReNameNodes
     * renames a node and all its child nodes in preparation for
     * Write() operations
     */
    virtual void ReNameNodes( void ) = 0;

    /**
     * Function WriteVRML
     * writes this node's data to a VRML file; this includes
     * all data of child and referenced nodes.
     */
    virtual bool WriteVRML( std::ofstream& aFile, bool aReuseFlag ) = 0;

    /**
     * Function WriteCache
     * write's this node's data to a binary cache file; the data
     * includes all data of children and references to children.
     * If this function is invoked by the user, parentNode must be
     * set to NULL in order to ensure coherent data.
     */
    virtual bool WriteCache( std::ofstream& aFile, SGNODE* parentNode ) = 0;

    /**
     * Function ReadCache
     * Reads binary format data from a cache file. To read a cache file,
     * open the file for reading and invoke this function from a new
     * SCENEGRAPH node.
     */
    virtual bool ReadCache( std::ifstream& aFile, SGNODE* parentNode ) = 0;
};

#endif  // SG_NODE_H
