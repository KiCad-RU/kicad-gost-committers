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

#ifdef TRI_EXPORTS
#define TRI_API __declspec(dllexport)
#else
#define TRI_API __declspec(dllimport)
#endif

TRI_API int triangulate_func(int vertexes_qty, double (*vertices)[2], double (*triangles)[3][2]);

