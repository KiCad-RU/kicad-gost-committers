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
 * @file vrml1_shapehints.h
 */


#ifndef VRML1_SHAPEHINTS_H
#define VRML1_SHAPEHINTS_H

#include <vector>
#include "vrml1_node.h"

class WRL1BASE;
class SGNODE;

/**
 * Class WRL1SHAPEHINTS
 */
class WRL1SHAPEHINTS : public WRL1NODE
{
private:
    WRL1_ORDER m_order;     // vertex order
    float      m_crease;    // VRML creaseAngle

public:
    WRL1SHAPEHINTS( NAMEREGISTER* aDictionary );
    WRL1SHAPEHINTS( NAMEREGISTER* aDictionary, WRL1NODE* aParent );
    virtual ~WRL1SHAPEHINTS();

    // functions inherited from WRL1NODE
    bool Read( WRLPROC& proc, WRL1BASE* aTopNode );
    bool AddRefNode( WRL1NODE* aNode );
    bool AddChildNode( WRL1NODE* aNode );
    SGNODE* TranslateToSG( SGNODE* aParent, WRL1STATUS* sp );
};

#endif  // VRML1_SHAPEHINTS_H
