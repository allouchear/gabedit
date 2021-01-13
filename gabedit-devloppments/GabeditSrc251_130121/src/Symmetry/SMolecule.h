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

#ifndef __GABEDIT_SMOLECULE_H__
#define __GABEDIT_SMOLECULE_H__

#include "../Utils/Point3D.h"
#include "../Symmetry/SAtom.h"

typedef struct _SMolecule  SMolecule;

struct _SMolecule
{
	GList* listOfAtoms;
	gint nAtoms;
/* methods */
	void (*addAtom) (SMolecule* mol, SAtom* atom);
	void (*addAtomsnmxyz) (SMolecule* mol, gchar* symbol, gint number, gdouble mass, gdouble x, gdouble y, gdouble z);
	GList* (*getAtoms) (SMolecule* mol);
	gint (*size) (SMolecule* mol);
	SAtom* (*get) (SMolecule* mol, gint i);
	SMolecule (*getCopy) (SMolecule* mol);
	void (*clear) (SMolecule* mol);
	void (*free) (SMolecule* mol);
	void (*addSMolecule) (SMolecule* mol, SMolecule* mol2);
	void (*setMolecule)(SMolecule* mol, gint nAtoms, gchar** symbols, gdouble* mass, gdouble* X, gdouble* Y, gdouble* Z);
	void (*set) (SMolecule* mol, gint i, SAtom* atom);
	void (*setSymmetryUniqueAll)(SMolecule* mol, gboolean u);
	void (*setSymmetryUnique)(SMolecule* mol, gint i, gboolean u);
	gboolean (*contains)(SMolecule* mol, SAtom* atom, gdouble tol);
	gint (*indexOf)(SMolecule* mol,  SAtom* atom, gdouble tol);
	void (*print)(SMolecule* mol);
};

SMolecule newSMolecule();
SMolecule newSMoleculeSize(gint n);

#endif /* __GABEDIT_SMOLECULE_H__ */

