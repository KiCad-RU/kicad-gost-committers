/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Cirilo Bernardo <cirilo.bernardo@gmail.com>
 * Copyright (C) 2015 Mario Luzeiro <mrluzeiro@gmail.com>
 * Copyright (C) 2011 Wayne Stambaugh <stambaughw@verizon.net>
 * Copyright (C) 2004 Jean-Pierre Charras, jp.charras at wanadoo.fr
 * Copyright (C) 1992-2015 KiCad Developers, see AUTHORS.txt for contributors.
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
 * @file 3d_info.h
 * defines the basic data associated with a single 3D model.
 */

#ifndef INFO_3D_H
#define INFO_3D_H

#include <wx/string.h>
#include <plugins/3dapi/sg_base.h>


struct S3D_INFO
{
    SGPOINT scale;     ///< scaling factors for the 3D footprint shape
    SGPOINT rotation;  ///< an X,Y,Z rotation (unit = degrees) for the 3D shape
    SGPOINT offset;    ///< an offset (unit = inch) for the 3D shape
    // note: the models are treated in a peculiar fashion since it is the
    // SCALE which is applied first, followed by the ROTATION and finally
    // the TRANSLATION/Offset (S-R-T). The usual order of operations is T-R-S.

    wxString filename;  ///< The 3D shape filename in 3D library
};

#endif // INFO_3D_H
