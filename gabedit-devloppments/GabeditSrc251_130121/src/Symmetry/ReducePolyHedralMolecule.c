/* ReducePolyHedralMolecule.c */
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
#include "../../Config.h"
#include <glib.h>
#include "../Symmetry/MoleculeSymmetryType.h"
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/PolyHedralOperators.h"
#include "../Symmetry/ReducePolyHedralMolecule.h"
#include "../Symmetry/SymmetryOperators.h"

#include <stdlib.h>

/************************************************************************************************************/
/* remove all points on one side of a plane given in Hessian form */
static void removeAllPointsOnOneSideOfAPlane(MolSymMolecule *mol,gdouble nx,gdouble ny,gdouble nz,gdouble d)
{
	gint i;
	gdouble r;
	gdouble eps = 1e-3;
	gint numberOfAtoms;
	gint deleted;     /* tag for deleted atom */
	MolSymAtom *atomList;

	if(mol->numberOfAtoms >0) eps = mol->listOfAtoms[0].eps;
  

	numberOfAtoms = mol->numberOfAtoms;
	deleted = mol->numberOfDifferentKindsOfAtoms;     /* tag for deleted atom */

	atomList = mol->listOfAtoms;

	for(i=0;i<mol->numberOfAtoms;i++)
	{
		if ((nz*nz > 0.00001) || (atomList->position[0]*atomList->position[0] + atomList->position[1]*atomList->position[1] > eps*eps))
		{
			r = atomList->position[0]*nx + atomList->position[1]*ny + atomList->position[2]*nz - d;
			if (r > 0.0)
			{
				atomList->type = deleted;
				numberOfAtoms--;
			}
		}
		atomList++;
	}
	qsort(mol->listOfAtoms,mol->numberOfAtoms,sizeof(MolSymAtom),compare2atomsUsingCenterOfGravity);
	mol->numberOfAtoms = numberOfAtoms;
}
/************************************************************************************************************/
/* tetraedal symmetry */
void reduceForTetraedalSymmetry(MolSymMolecule *mol,gint sym)
{
	gdouble eps = 1e-3;

	if(mol->numberOfAtoms >0) eps = mol->listOfAtoms[0].eps;
	removeAllPointsOnOneSideOfAPlane(mol, 0.577350269,  0.0, -0.816496581, eps);

	if (sym & SYM_D)
		removeAllPointsOnOneSideOfAPlane(mol,-0.866025404, 0.5, 0.0,-eps);
	else
		removeAllPointsOnOneSideOfAPlane(mol,0.5, -0.866025404, 0.0,eps);
}    
/************************************************************************************************************/
/* octaedral symmetry */
void reduceForOctaedralSymmetry(MolSymMolecule *mol,gint sym)
{
	gdouble eps = 1e-3;

	if(mol->numberOfAtoms >0) eps = mol->listOfAtoms[0].eps;
	rotationAroundZAxisByPi(mol);
	removeAllPointsOnOneSideOfAPlane(mol, 0.0, -1.0, 0.0, eps);
	removeAllPointsOnOneSideOfAPlane(mol, 0.577350269, 0.0, -0.816496581, eps);
	removeAllPointsOnOneSideOfAPlane(mol, -0.816496581, 0.0, -0.577350269, eps); 

	/*
	if (sym & SYM_D)
		removeAllPointsOnOneSideOfAPlane(mol,-0.866025404, 0.5, 0.0, -eps);
	else
		removeAllPointsOnOneSideOfAPlane(mol,0.408248290, 1.414213562, -0.577350269, eps);
		*/
}    
/************************************************************************************************************/
/* icosaedral symmetry */
void reduceForIcosaedralSymmetry(MolSymMolecule *mol,gint sym)
{
	gdouble eps = 1e-3;

	if(mol->numberOfAtoms >0) eps = mol->listOfAtoms[0].eps;
	removeAllPointsOnOneSideOfAPlane(mol, 0.0, -1.0, 0.0, eps);
	removeAllPointsOnOneSideOfAPlane(mol, 0.467086179481, 0.809016994375, -0.356822089773,eps); 

	if (sym & SYM_D)
		removeAllPointsOnOneSideOfAPlane(mol,-0.866025404, 0.5, 0.0, eps);
	else
		removeAllPointsOnOneSideOfAPlane(mol,-0.866025404,-0.5, 0.0,-eps);
}    

