/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015-2016 Mario Luzeiro <mrluzeiro@ua.pt>
 * Copyright (C) 1992-2016 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file  ogl_utils.h
 * @brief implements generic openGL functions that are common to any openGL target
 */

#ifndef OGL_UTILS_H_
#define OGL_UTILS_H_

#include <wx/image.h>
#include "3d_rendering/cimage.h"
#include <plugins/3dapi/c3dmodel.h>


/**
 * @brief OGL_SetMaterial - Set OpenGL materials
 * @param aMaterial: a material structure with parameters to set
 */
void OGL_SetMaterial( const SMATERIAL & aMaterial );


/**
 * @brief OGL_SetDiffuseOnlyMaterial - sets only the diffuse color and keep other
 * parameters with default values
 * @param aMaterialDiffuse: the diffese color to assign to material properties
 */
void OGL_SetDiffuseOnlyMaterial( const SFVEC3F &aMaterialDiffuse );


/**
 * @brief OGL_LoadTexture - generate a new OpenGL texture
 * @param aImage: a image to generate the texture from
 * @return the OpenGL texture index created
 */
GLuint OGL_LoadTexture( const CIMAGE &aImage );


/**
 * @brief OGL_GetScreenshot - got the pixel data of current OpenGL image
 * @param aDstImage: the output image. the image must be destroyed to free the data
 */
void OGL_GetScreenshot( wxImage &aDstImage );


/**
 * @brief OGL_DrawBackground
 * @param aTopColor
 * @param aBotColor
 */
void OGL_DrawBackground( const SFVEC3F &aTopColor, const SFVEC3F &aBotColor );

#endif // OGL_UTILS_H_
