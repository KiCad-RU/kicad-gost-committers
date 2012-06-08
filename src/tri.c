/*
 * PCadToKiCad, PCad ASCII to KiCad conversion utility
 * Copyright (C) 2007, 2008 Alexander Lunev <alexanderlunev@mail.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 * USA
 */

/* dll interface to Triangle library
*/

/* If SINGLE is defined when triangle.o is compiled, it should also be       */
/*   defined here.  If not, it should not be defined here.                   */

/* #define SINGLE */

#ifdef SINGLE
#define REAL float
#else /* not SINGLE */
#define REAL double
#endif /* not SINGLE */

#define TRUE	1
#define FALSE	0
typedef int BOOL;

#include <windows.h>
#include "tri.h"

#include <stdio.h>
#include <stdlib.h>
#include "triangle.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}

TRI_API int triangulate_func(int vertices_qty, double (*vertices)[2], double (*triangles)[3][2])
{
	struct triangulateio in, out, vorout;
	int i;

	in.numberofpoints = vertices_qty;
	in.numberofpointattributes = 0;
	in.pointlist = (REAL *) vertices;

	in.pointattributelist = (REAL *) NULL;
	in.pointmarkerlist = (int *) NULL;
  
	in.numberofsegments = vertices_qty;
	in.segmentmarkerlist = (REAL *) NULL;
	in.segmentlist = (REAL *) malloc(in.numberofsegments * 2 * sizeof(int));

	in.numberofholes = 0;
	in.holelist = (REAL *) NULL;
	
	in.segmentlist[0] = vertices_qty-1;
	in.segmentlist[1] = 0;
	for (i=2; i<vertices_qty*2; i++) in.segmentlist[i] = (i-1)>>1;

	in.numberofregions = 0;;
	in.regionlist = (REAL *) NULL;

	/* Make necessary initializations so that Triangle can return a */
	/*   triangulation in `mid' and a voronoi diagram in `vorout'.  */

	out.pointlist = (REAL *) NULL;            /* Not needed if -N switch used. */
	/* Not needed if -N switch used or number of point attributes is zero: */
	out.pointattributelist = (REAL *) NULL;
	out.pointmarkerlist = (int *) NULL; /* Not needed if -N or -B switch used. */
	out.trianglelist = (int *) NULL;          /* Not needed if -E switch used. */
	/* Not needed if -E switch used or number of triangle attributes is zero: */
	out.triangleattributelist = (REAL *) NULL;
	out.neighborlist = (int *) NULL;         /* Needed only if -n switch used. */
	/* Needed only if segments are output (-p or -c) and -P not used: */
	out.segmentlist = (int *) NULL;
	/* Needed only if segments are output (-p or -c) and -P and -B not used: */
	out.segmentmarkerlist = (int *) NULL;
	out.edgelist = (int *) NULL;             /* Needed only if -e switch used. */
	out.edgemarkerlist = (int *) NULL;   /* Needed if -e used and -B not used. */

	vorout.pointlist = (REAL *) NULL;        /* Needed only if -v switch used. */
	/* Needed only if -v switch used and number of attributes is not zero: */
	vorout.pointattributelist = (REAL *) NULL;
	vorout.edgelist = (int *) NULL;          /* Needed only if -v switch used. */
	vorout.normlist = (REAL *) NULL;         /* Needed only if -v switch used. */

	triangulate("pz", &in, &out, &vorout);
  
	for (i=0; i<out.numberoftriangles; i++)
	{
		triangles[i][0][0] = vertices[out.trianglelist[i*3]][0];
		triangles[i][0][1] = vertices[out.trianglelist[i*3]][1];
		triangles[i][1][0] = vertices[out.trianglelist[i*3+1]][0];
		triangles[i][1][1] = vertices[out.trianglelist[i*3+1]][1];
		triangles[i][2][0] = vertices[out.trianglelist[i*3+2]][0];
		triangles[i][2][1] = vertices[out.trianglelist[i*3+2]][1];
	}
	/* Free all allocated arrays, including those allocated by Triangle. */
	
	free(in.pointattributelist);
	free(in.pointmarkerlist);
	free(in.regionlist);
	free(out.pointlist);
	free(out.pointattributelist);
	free(out.pointmarkerlist);
	free(out.trianglelist);
	free(out.triangleattributelist);
	free(out.neighborlist);
	free(out.segmentlist);
	free(out.segmentmarkerlist);
	free(out.edgelist);
	free(out.edgemarkerlist);
	free(vorout.pointlist);
	free(vorout.pointattributelist);
	free(vorout.edgelist);
	free(vorout.normlist);

	return out.numberoftriangles;
}
