/**********************************************************************************************************
Copyright (c) 2002-2013 Abdul-Rahman Allouche. All rights reserved

Permision is hereby granted, free of charge, to any person obtaining a copy of this software and asociated
documentation files (the Gabedit), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permision notice shall be included in all copies or substantial portions
  of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
************************************************************************************************************/
#ifndef __GABEDIT_ANIMATIONGRIDS_H__
#define __GABEDIT_ANIMATIONGRIDS_H__

typedef struct _AtomGrid
{
	gchar symbol[5];
	gdouble C[3];
	gchar mmType[5];
	gchar pdbType[5];
	gdouble partialCharge;
	gboolean variable;
	gdouble nuclearCharge;
}AtomGrid;
typedef struct _GeomGrid
{
	gint numberOfAtoms;
	AtomGrid* listOfAtoms;
}GeomGrid;

typedef struct _AnimationGrids
{
	gdouble velocity;
	gdouble isovalue;
	GabEditTypeFile typeOfFile;
	gint numberOfFiles;
	GeomGrid* geometries;
	Grid** grids;
	gchar** fileNames;
}AnimationGrids;
void initAnimationGrids();
void animationGridsDlg();

extern AnimationGrids animationGrids;

#endif /* __GABEDIT_ANIMATIONGRIDS_H__ */

