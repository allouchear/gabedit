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
#ifndef __GABEDIT_ANIMATIONGEOMCONV_H__
#define __GABEDIT_ANIMATIONGEOMCONV_H__

typedef struct _Atom
{
	gchar symbol[5];
	gdouble C[3];
	gchar mmType[5];
	gchar pdbType[5];
	gboolean variable;
	gdouble partialCharge;
	gdouble nuclearCharge;
}Atom;
typedef struct _Geometry
{
	gint numberOfAtoms;
	Atom* listOfAtoms;
}Geometry;

typedef struct _GeometryConvergence
{
	gchar* fileName;
	GabEditTypeFile typeOfFile;
	gint numberOfGeometries;
	gint* numGeometry;
	gdouble* energy;
	gdouble* maxForce;
	gdouble* rmsForce;
	gdouble* maxStep;
	gdouble* rmsStep;
	gdouble velocity;
	Geometry* geometries;
	gchar** comments;
}GeometryConvergence;
void initGeometryConvergence();
void geometryConvergenceDlg();

extern GeometryConvergence geometryConvergence;

#endif /* __GABEDIT_ANIMATIONGEOMCONV_H__ */

