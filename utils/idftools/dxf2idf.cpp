/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2014  Cirilo Bernardo
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

#include <cstdio>
#include <iostream>
#include <libdxfrw.h>
#include <dxf2idf.h>

// differences in angle smaller than MIN_ANG are considered equal
#define MIN_ANG     (0.01)

DXF2IDF::~DXF2IDF()
{
    while( !lines.empty() )
    {
#ifdef DEBUG_IDF
        IDF3::printSeg( lines.back() );
#endif
        delete lines.back();
        lines.pop_back();
    }
}


bool DXF2IDF::ReadDxf( const std::string aFile )
{
    dxfRW* reader = new dxfRW( aFile.c_str() );

    if( !reader )
        return false;

    bool success = reader->read( this, true );

    delete reader;
    return success;
}


void DXF2IDF::addLine( const DRW_Line& data )
{
    IDF_POINT p1, p2;

    p1.x = data.basePoint.x;
    p1.y = data.basePoint.y;
    p2.x = data.secPoint.x;
    p2.y = data.secPoint.y;

    IDF_SEGMENT* seg = new IDF_SEGMENT( p1, p2 );

    if( !seg )
    {
        std::cerr << "* FAULT: could not add a linear segment to the outline\n";
    }
    else
    {
        lines.push_back( seg );
    }

    return;
}


void DXF2IDF::addCircle( const DRW_Circle& data )
{
    IDF_POINT p1, p2;

    p1.x = data.basePoint.x;
    p1.y = data.basePoint.y;

    p2.x = p1.x + data.radious;
    p2.y = p1.y;

    IDF_SEGMENT* seg = new IDF_SEGMENT( p1, p2, 360, true );

    if( !seg )
    {
        std::cerr << "* FAULT: could not add a linear segment to the outline\n";
    }
    else
    {
        lines.push_back( seg );
    }

    return;
}


void DXF2IDF::addArc( const DRW_Arc& data )
{
    IDF_POINT p1, p2;

    p1.x = data.basePoint.x;
    p1.y = data.basePoint.y;

    // note: DXF circles always run CCW
    double ea = data.endangle;

    while( ea < data.staangle )
        ea += M_PI;

    p2.x = p1.x + cos( data.staangle ) * data.radious;
    p2.y = p1.y + sin( data.staangle ) * data.radious;

    double angle = ( ea - data.staangle ) * 180.0 / M_PI;

    IDF_SEGMENT* seg = new IDF_SEGMENT( p1, p2, angle, true );

    if( !seg )
    {
        std::cerr << "* FAULT: could not add a linear segment to the outline\n";
    }
    else
    {
        lines.push_back( seg );
    }

    return;
}


bool DXF2IDF::WriteOutline( FILE* aFile, bool isInch )
{
    if( lines.empty() )
    {
        std::cerr << "* DXF2IDF: empty outline\n";
        return false;
    }

    // 1. find lowest X value
    // 2. string an outline together
    // 3. emit warnings if more than 1 outline
    IDF_OUTLINE outline;

    IDF3::GetOutline( lines, outline );

    if( outline.empty() )
    {
        std::cerr << "* DXF2IDF::WriteOutline(): no valid outline in file\n";
        return false;
    }

    if( !lines.empty() )
    {
        std::cerr << "* DXF2IDF::WriteOutline(): WARNING: more than 1 outline in file\n";
        std::cerr << "*                          Only the first outline will be used\n";
    }

    char loopDir = '1';

    if( outline.IsCCW() )
        loopDir = '0';

    std::list<IDF_SEGMENT*>::iterator bo;
    std::list<IDF_SEGMENT*>::iterator eo;

    if( outline.size() == 1 )
    {
        if( !outline.front()->IsCircle() )
        {
            std::cerr << "* DXF2IDF::WriteOutline(): bad outline\n";
            return false;
        }

            // NOTE: a circle always has an angle of 360, never -360,
            // otherwise SolidWorks chokes on the file.
            if( isInch )
            {
                fprintf( aFile, "%c %d %d 0\n", loopDir,
                         (int) (1000 * outline.front()->startPoint.x),
                         (int) (1000 * outline.front()->startPoint.y) );
                fprintf( aFile, "%c %d %d 360\n", loopDir,
                         (int) (1000 * outline.front()->endPoint.x),
                         (int) (1000 * outline.front()->endPoint.y) );
            }
            else
            {
                fprintf( aFile, "%c %.3f %.3f 0\n", loopDir,
                         outline.front()->startPoint.x, outline.front()->startPoint.y );
                fprintf( aFile, "%c %.3f %.3f 360\n", loopDir,
                         outline.front()->endPoint.x, outline.front()->endPoint.y );
            }

        return true;
    }

    // ensure that the very last point is the same as the very first point
    outline.back()-> endPoint = outline.front()->startPoint;

    bo  = outline.begin();
    eo  = outline.end();

    // for the first item we write out both points
    if( (*bo)->angle < MIN_ANG && (*bo)->angle > -MIN_ANG )
    {
        if( isInch )
        {
            fprintf( aFile, "%c %d %d 0\n", loopDir,
                     (int) (1000 * (*bo)->startPoint.x),
                     (int) (1000 * (*bo)->startPoint.y) );
            fprintf( aFile, "%c %d %d 0\n", loopDir,
                     (int) (1000 * (*bo)->endPoint.x),
                     (int) (1000 * (*bo)->endPoint.y) );
        }
        else
        {
            fprintf( aFile, "%c %.3f %.3f 0\n", loopDir,
                     (*bo)->startPoint.x, (*bo)->startPoint.y );
            fprintf( aFile, "%c %.3f %.3f 0\n", loopDir,
                     (*bo)->endPoint.x, (*bo)->endPoint.y );
        }
    }
    else
    {
        if( isInch )
        {
            fprintf( aFile, "%c %d %d 0\n", loopDir,
                     (int) (1000 * (*bo)->startPoint.x),
                     (int) (1000 * (*bo)->startPoint.y) );
            fprintf( aFile, "%c %d %d %.2f\n", loopDir,
                     (int) (1000 * (*bo)->endPoint.x),
                     (int) (1000 * (*bo)->endPoint.y),
                     (*bo)->angle );
        }
        else
        {
            fprintf( aFile, "%c %.3f %.3f 0\n", loopDir,
                     (*bo)->startPoint.x, (*bo)->startPoint.y );
            fprintf( aFile, "%c %.3f %.3f %.2f\n", loopDir,
                     (*bo)->endPoint.x, (*bo)->endPoint.y, (*bo)->angle );
        }
    }

    ++bo;

    // for all other segments we only write out the last point
    while( bo != eo )
    {
        if( isInch )
        {
            if( (*bo)->angle < MIN_ANG && (*bo)->angle > -MIN_ANG )
            {
                fprintf( aFile, "%c %d %d 0\n", loopDir,
                         (int) (1000 * (*bo)->endPoint.x),
                         (int) (1000 * (*bo)->endPoint.y) );
            }
            else
            {
                fprintf( aFile, "%c %d %d %.2f\n", loopDir,
                         (int) (1000 * (*bo)->endPoint.x),
                         (int) (1000 * (*bo)->endPoint.y),
                         (*bo)->angle );
            }
        }
        else
        {
            if( (*bo)->angle < MIN_ANG && (*bo)->angle > -MIN_ANG )
            {
                fprintf( aFile, "%c %.5f %.5f 0\n", loopDir,
                         (*bo)->endPoint.x, (*bo)->endPoint.y );
            }
            else
            {
                fprintf( aFile, "%c %.5f %.5f %.2f\n", loopDir,
                         (*bo)->endPoint.x, (*bo)->endPoint.y, (*bo)->angle );
            }
        }

        ++bo;
    }

    return true;
}
