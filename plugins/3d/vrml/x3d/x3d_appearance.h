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
 * @file x3d_appearance.h
 */


#ifndef X3D_APPEARANCE_H
#define X3D_APPEARANCE_H

#include <vector>

#include "wrltypes.h"
#include "x3d_shape.h"


/**
 * Class X3DAPP
 */
class X3DAPP : public X3DNODE
{
private:
    void init();
    void readFields( wxXmlNode* aNode );
    wxString m_MatName; // material name

protected:
    WRLVEC3F    diffuseColor;
    WRLVEC3F    emissiveColor;
    WRLVEC3F    specularColor;
    float       ambientIntensity;
    float       shininess;
    float       transparency;

public:
    X3DAPP();
    X3DAPP( X3DNODE* aParent );
    virtual ~X3DAPP();

    // functions inherited from X3DNODE
    bool Read( wxXmlNode* aNode, X3DNODE* aTopNode, X3D_DICT& aDict );
    bool SetParent( X3DNODE* aParent, bool doUnlink = true );
    bool AddChildNode( X3DNODE* aNode );
    bool AddRefNode( X3DNODE* aNode );
    SGNODE* TranslateToSG( SGNODE* aParent );
};

#endif  // X3D_APPEARANCE_H
