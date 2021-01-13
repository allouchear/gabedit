/**********************************************************************************************************
Copyright (c) 2002-2013 Abdul-Rahman Allouche. All rights reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the Gabedit), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions
  of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
************************************************************************************************************/

#ifndef __GABEDIT_ISOSURFACE_H__
#define __GABEDIT_ISOSURFACE_H__

typedef struct _Vertex
{
	gdouble C[4];
}Vertex;
typedef struct _Triangle
{
	Vertex *vertex[3];
	gint numvertex[3];
	Vertex Normal[3];
}Triangle;

typedef struct _Cube
{
	gint Nvertex;
	Vertex *vertex;
	gint Ntriangles;
	Triangle* triangles;
}Cube;
typedef struct _IsoSurface
{
	gint N[3];
	Cube ***cube;
	Grid *grid;
}IsoSurface;
IsoSurface* define_iso_surface(Grid* grid,gdouble isolevel, gboolean mapping);
IsoSurface* iso_free(IsoSurface* iso);

#endif /* __GABEDIT_ISOSURFACE_H__ */

