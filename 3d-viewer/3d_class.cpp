/**
 * @file 3d_class.cpp
 */

/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 1992-2012 KiCad Developers, see AUTHORS.txt for contributors.
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


#include <fctsys.h>

#include <3d_viewer.h>

S3D_MATERIAL::S3D_MATERIAL( S3D_MASTER* father, const wxString& name ) :
    EDA_ITEM( father, NOT_USED )
{
    m_DiffuseColor.x   = m_DiffuseColor.y = m_DiffuseColor.z = 1.0;
    m_SpecularColor.x  = m_SpecularColor.y = m_SpecularColor.z = 1.0;
    m_AmbientIntensity = 1.0;
    m_Transparency     = 0.0;
    m_Shininess = 1.0;
    m_Name = name;
}


void S3D_MATERIAL::SetMaterial()
{
    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glColor4f( m_DiffuseColor.x * m_AmbientIntensity,
               m_DiffuseColor.y * m_AmbientIntensity,
               m_DiffuseColor.z * m_AmbientIntensity,
               1.0 - m_Transparency );
#if 0
    glColorMaterial( GL_FRONT_AND_BACK, GL_SPECULAR );
    glColor3f( m_SpecularColor.x, m_SpecularColor.y, m_SpecularColor.z );
#endif
    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
}


void S3D_MASTER::Copy( S3D_MASTER* pattern )
{
    m_Shape3DName = pattern->m_Shape3DName;
    m_MatScale    = pattern->m_MatScale;
    m_MatRotation = pattern->m_MatRotation;
    m_MatPosition = pattern->m_MatPosition;
    m_3D_Drawings = NULL;
    m_Materials   = NULL;
}


S3D_MASTER::S3D_MASTER( EDA_ITEM* aParent ) :
    EDA_ITEM( aParent, NOT_USED )
{
    m_MatScale.x  = m_MatScale.y = m_MatScale.z = 1.0;
    m_3D_Drawings = NULL;
    m_Materials   = NULL;
}


S3D_MASTER:: ~S3D_MASTER()
{
    STRUCT_3D_SHAPE* next;
    S3D_MATERIAL*   nextmat;

    for( ; m_3D_Drawings != NULL; m_3D_Drawings = next )
    {
        next = m_3D_Drawings->Next();
        delete m_3D_Drawings;
    }

    for( ; m_Materials != NULL; m_Materials = nextmat )
    {
        nextmat = m_Materials->Next();
        delete m_Materials;
    }
}


STRUCT_3D_SHAPE::STRUCT_3D_SHAPE( EDA_ITEM* aParent ) :
    EDA_ITEM( aParent, NOT_USED )
{
    m_3D_Coord = NULL;
    m_3D_CoordIndex = NULL;
    m_3D_Points     = 0;
}


STRUCT_3D_SHAPE:: ~STRUCT_3D_SHAPE()
{
    delete m_3D_Coord;
    delete m_3D_CoordIndex;
}
