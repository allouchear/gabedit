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

#ifndef __GABEDIT_SATOM_H__
#define __GABEDIT_SATOM_H__

#include "../Utils/Point3D.h"

typedef struct _SAtom  SAtom;

struct _SAtom
{
	Point3D position;
	gint number;
	gdouble mass;
	gchar symbol[10];
	gboolean isSymmetryUnique;
	gchar str[1024];

/* methods */
	gchar* (*getSymbol) (SAtom* atom);
	gint (*getNumber) (SAtom* atom);
	gdouble (*getMass)(SAtom* atom);
	Point3D (*getPosition)(SAtom* atom);
	void (*setPosition)(SAtom* atom, Point3D* newPos);
	void (*setSymmetryUnique)(SAtom* atom, gboolean isUnique);
	SAtom (*findClosestAtom)(SAtom* atom, GList* closeAtoms);
	gdouble (*distance)(SAtom* atom, SAtom* other);
	gchar* (*toString)(SAtom* atom);
	gboolean (*equals)(SAtom* atom1, SAtom* atom2, gdouble tol);
};

SAtom newSAtom();
SAtom newSAtomxyz(gchar* symbol, gint number, gdouble mass, gdouble x, gdouble y, gdouble z);
SAtom newSAtomsnm(gchar* symbol, gint number, gdouble mass);

#endif /* __GABEDIT_SATOM_H__ */

