/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2015 Mario Luzeiro <mrluzeiro@ua.pt>
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
 * @file  3d_math.h
 * @brief Defines math related functions
 */

#ifndef _3D_MATH_H
#define _3D_MATH_H

#include "plugins/3dapi/xv3d_types.h"


// https://en.wikipedia.org/wiki/Spherical_coordinate_system
/**
 * @brief SphericalToCartesian
 * @param aInclination θ ∈ [0, π]
 * @param aAzimuth φ ∈ [0, 2π]
 * @return Cartesian cordinates
 */
inline SFVEC3F SphericalToCartesian( float aInclination, float aAzimuth )
{
    float sinInc = glm::sin( aInclination );
    return SFVEC3F( sinInc * glm::cos( aAzimuth ),
                    sinInc * glm::sin( aAzimuth ),
                    glm::cos( aInclination ) );
}


// https://pathtracing.wordpress.com/2011/03/03/cosine-weighted-hemisphere/
// !TODO: this is not correct because it is not a gaussian random
inline SFVEC3F UniformRandomHemisphereDirection( )
{
    SFVEC3F b( (rand()/(float)RAND_MAX) - 0.5f, (rand()/(float)RAND_MAX) - 0.5f, (rand()/(float)RAND_MAX) - 0.5f);
    return b;
}


// https://pathtracing.wordpress.com/2011/03/03/cosine-weighted-hemisphere/
inline SFVEC3F CosWeightedRandomHemisphereDirection( SFVEC3F n )
{
    const float Xi1 = (float)rand() / (float)RAND_MAX;
    const float Xi2 = (float)rand() / (float)RAND_MAX;

    const float theta = acos( sqrt( 1.0f - Xi1 ) );
    const float phi = 2.0f * glm::pi<float>() * Xi2;

    const float xs = sinf( theta ) * cosf( phi );
    const float ys = cosf( theta );
    const float zs = sinf( theta ) * sinf( phi );

    const SFVEC3F y( n.x, n.y, n.z );
    SFVEC3F h = y;

    if (fabs( h.x ) <= fabs( h.y ) && fabs( h.x ) <= fabs( h.z ) )
        h.x= 1.0f;
    else if (fabs(h.y)<=fabs(h.x) && fabs(h.y)<=fabs(h.z))
        h.y= 1.0f;
    else
        h.z= 1.0f;


    const SFVEC3F x = glm::normalize( glm::cross( h, y ) );
    const SFVEC3F z = glm::normalize( glm::cross( x, y ) );

    SFVEC3F direction = xs * x + ys * y + zs * z;
    return glm::normalize( direction );
}


/**
 * @brief Refract
 * Based on: https://github.com/mmp/pbrt-v3/blob/master/src/core/reflection.h
 * See also: http://www.flipcode.com/archives/Raytracing_Topics_Techniques-Part_3_Refractions_and_Beers_Law.shtml
 * @param aInVector incoming vector
 * @param aNormal normal in the intersection point
 * @param aRin_over_Rout incoming refraction index / out refraction index
 * @param aOutVector the refracted vector
 * @return true
 */
inline bool Refract( const SFVEC3F &aInVector, const SFVEC3F &aNormal, float aRin_over_Rout, SFVEC3F &aOutVector )
{
    float cosThetaI = -glm::dot( aNormal, aInVector );
    float sin2ThetaI = glm::max( 0.0f, 1.0f - cosThetaI * cosThetaI );
    float sin2ThetaT = aRin_over_Rout * aRin_over_Rout * sin2ThetaI;

    // Handle total internal reflection for transmission
    if( sin2ThetaT >= 1.0f )
        return false;

    float cosThetaT = sqrtf( 1.0f - sin2ThetaT );
    aOutVector = glm::normalize( aRin_over_Rout * aInVector + ( aRin_over_Rout * cosThetaI - cosThetaT ) * aNormal );

    return true;
}

inline float mapf( float x, float in_min, float in_max, float out_min, float out_max)
{
  x = glm::clamp( x, in_min, in_max );
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#endif // 3D_MATH_H
