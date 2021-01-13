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

#ifndef __GABEDIT_SYMMETRY_H__
#define __GABEDIT_SYMMETRY_H__

#include "../Utils/Point3D.h"
#include "../Symmetry/SOperations.h"
#include "../Symmetry/SMolecule.h"
#include "../Symmetry/Elements.h"
#include "../Symmetry/HashMapSAtoms.h"
#include "../Symmetry/PointGroups.h"

typedef struct _Symmetry  Symmetry;

struct _Symmetry
{
	gdouble GAUSSIAN_SYMMETRY_TOLERANCE;
	gint MAX_DEGREE;
	gdouble DEFAULT_TOLERANCE;
	gdouble MOMENT_TOLERANCE;
	gdouble DOT_TOLERANCE;
	gdouble GABEDIT_T_APOTHEM;
	
	Elements elements;
	SMolecule molecule;

	PointGroups pointGroups;
	
	Elements rotations;
	Element* primaryAxis;

	gdouble tolerance;
	
	Point3D centerOfMass;
	Point3D principalAxes[3];
	gdouble principalMoments[3];
	gint degeneracy;

/* methods */
	void (*findProperRotationAxes) (Symmetry* symmetry);
	void (*findImproperRotationAxes) (Symmetry* symmetry);
	void (*findReflectionPlanes) (Symmetry* symmetry);
	PointGroups (*findAllPointGroups)(Symmetry* symmetry);
	void (*findSymmetryElements) (Symmetry* symmetry);
	void (*findPointGroups) (Symmetry* symmetry);
	void (*findInversionCenter) (Symmetry* symmetry);
	void (*free) (Symmetry* symmetry);
	void (*clear) (Symmetry* symmetry);
	PointGroups (*getPointGroups)(Symmetry* symmetry);
	Elements (*getElements)(Symmetry* symmetry);
	SMolecule (*getMolecule)(Symmetry* symmetry);
	void (*printPointGroupResults)(Symmetry* symmetry);
	gchar* (*findSinglePointGroup)(Symmetry* symmetry);
	void (*printElementResults)(Symmetry* symmetry);
	SMolecule (*getUniqueMolecule)(Symmetry* symmetry);
	SMolecule (*getSymmetrizeMolecule)(Symmetry* symmetry);
	void (*setMaxDegree)(Symmetry* symmetry, gint maxDegree);
	void (*setMomentTolerance)(Symmetry* symmetry, gdouble tolerance);
	gchar* (*getGroupName)(Symmetry* symmetry);
};

Symmetry newSymmetry(SMolecule* mol, gdouble tolerance);

#endif /* __GABEDIT_SYMMETRY_H__ */

