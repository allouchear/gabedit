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

#ifndef __GABEDIT_MOLECULE_H__
#define __GABEDIT_MOLECULE_H__

typedef struct _Molecule
{
	gint nAtoms;
	AtomMol* atoms;
	gdouble energy;
	gdouble* gradient[3];
	gint numberOf2Connections;
	gint* connected2[2];
	gint numberOf3Connections;
	gint* connected3[3];
	gint numberOf4Connections;
	gint* connected4[4];

	gint numberOfNonBonded;
	gint* nonBonded[2];
}Molecule;

Molecule newMolecule();
Molecule createMolecule(GeomDef* geom,gint natoms,gboolean connections);
void freeMolecule(Molecule* molecule);
void redrawMolecule(Molecule* molecule,gchar* str);
Molecule copyMolecule(Molecule* m);

#endif /* __GABEDIT_MOLECULE_H__ */

