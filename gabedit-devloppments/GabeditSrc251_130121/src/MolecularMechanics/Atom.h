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

#ifndef __GABEDIT_ATOM_H__
#define __GABEDIT_ATOM_H__

typedef struct _AtomMol
{
	gdouble coordinates[3];
	gdouble charge;
	SAtomsProp prop;
	gchar* mmType;
	gchar* pdbType;
	gchar* residueName;
	gint residueNumber;
	gint N;

	gboolean show;
	gboolean variable;
	GabEditLayerType layer;
	gint* typeConnections;
}AtomMol;

gdouble getAngle(AtomMol *a1,AtomMol* a2,AtomMol* a3);
gdouble getTorsion(AtomMol* a1,AtomMol* a2,AtomMol* a3,AtomMol* a4);

#endif /* __GABEDIT_ATOM_H__ */

