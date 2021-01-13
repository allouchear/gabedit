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

#ifndef __GABEDIT_MOLECULESE_H__
#define __GABEDIT_MOLECULESE_H__

typedef struct _MoleculeSE
{
	gint nAtoms;
	AtomSE* atoms;
	gint spinMultiplicity;
	gint totalCharge;
	gdouble energy;
	gdouble* gradient[3];

	gint numberOf2Connections;
	gint* connected2[2];
	gint numberOf3Connections;
	gint* connected3[3];
	gdouble dipole[3];
}MoleculeSE;

MoleculeSE newMoleculeSE();
MoleculeSE createMoleculeSE(GeomDef* geom,gint natoms, gint charge, gint spin, gboolean connections);
MoleculeSE createFromGeomXYZMoleculeSE(gint charge, gint spin, gboolean connections);
void freeMoleculeSE(MoleculeSE* molecule);
void redrawMoleculeSE(MoleculeSE* molecule,gchar* str);
MoleculeSE copyMoleculeSE(MoleculeSE* m);
gboolean saveMoleculeSEHIN(MoleculeSE* mol, char* fileName);
gboolean saveMoleculeSEMol2(MoleculeSE* mol, char* fileName);
void computeMoleculeSEDipole(MoleculeSE* mol);
void readGeomMoleculeSEFromOpenBabelOutputFile(MoleculeSE* mol, char* fileName, int numgeometry);
gboolean addGeometryMoleculeSEToGabedit(MoleculeSE* molecule,FILE* file);
gboolean addMoleculeSEToFile(MoleculeSE* molecule,FILE* file);
gboolean saveMoleculeSETypeSave(MoleculeSE* molecule, char* fileName, char* typeSave);
gboolean saveMoleculeSE(MoleculeSE* molecule, char* fileName);
gdouble getGradientNormMoleculeSE(MoleculeSE* molecule);
void setConnectionsMoleculeSE(MoleculeSE* molecule);
gboolean readGeometryFromGenericOutputFile(MoleculeSE* molecule, char* namefile);

#endif /* __GABEDIT_MOLECULESE_H__ */

