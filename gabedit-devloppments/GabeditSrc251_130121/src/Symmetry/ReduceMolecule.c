/* ReduceMolecule.c */
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
#include "../Utils/Constants.h"
#include "../Symmetry/MoleculeSymmetryType.h"
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/SymmetryOperators.h"
#include "../Symmetry/ReduceMolecule.h"
#include "../Symmetry/ReducePolyHedralMolecule.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SQU(x,y,z) ((x)*(x) + (y)*(y) + (z)*(z))

/************************************************************************************************************/
static void setEpsToZero(MolSymMolecule* mol)
{
	gdouble eps = 1e-3;
	gint i;
	gint numberOfAtoms = mol->numberOfAtoms;
	MolSymAtom *atomList = mol->listOfAtoms;

	if(mol->numberOfAtoms >0) eps = mol->listOfAtoms[0].eps;

	for(i=0;i<numberOfAtoms;i++)
	{
		if( fabs(atomList->position[0]) < eps) atomList->position[0] = 0.0;
		if( fabs(atomList->position[1]) < eps) atomList->position[1] = 0.0;
		if( fabs(atomList->position[2]) < eps) atomList->position[2] = 0.0;
		atomList++;
	}
}
/************************************************************************************************************/
static void removeHalfSpace(MolSymMolecule* mol, RotationReflectionOperator op)
{
	gdouble eps = 1e-3;
	gint i;
	gint numberOfAtoms = mol->numberOfAtoms;
	gint deleted = mol->numberOfDifferentKindsOfAtoms;     /* tag for deleted atom */
	MolSymAtom *atomList = mol->listOfAtoms;

	if(mol->numberOfAtoms >0) eps = mol->listOfAtoms[0].eps;

	/* printf("eps = %f\n",eps);*/

	for(i=0;i<mol->numberOfAtoms;i++)
	{
		switch(op)
		{
			case XY_PLANE:
			if (atomList->position[2] < -eps) 
			{
				atomList->type = deleted;
				numberOfAtoms--;
			}
			break;

			case XZ_PLANE:
			if (atomList->position[1] < -eps) 
			{
				atomList->type = deleted;
				numberOfAtoms--;
			}
			break;

			case ROT2X:
			if (atomList->position[2] > eps) break;
			if (atomList->position[2] < -eps)  
			{
				atomList->type = deleted;
				numberOfAtoms--;
				break;
			}
			/* now  |z| < eps */
			if (atomList->position[1] < -eps)  
			{
				atomList->type = deleted;
				numberOfAtoms--;
				break;
			}
			break;

			case POINT_INV:
			/* printf("pointinv = %f %f %f %f\n", atomList->position[0], atomList->position[1], atomList->position[2], eps);*/
			if (atomList->position[2] > eps) break;
			if (atomList->position[2] < -eps)  
			{
				atomList->type = deleted;
				numberOfAtoms--;
				break;
			}
			/* now  |z| < eps */
			if (atomList->position[1] > eps) break;
			if (atomList->position[1] < -eps)  
			{
				atomList->type = deleted;
				numberOfAtoms--;
				break;
			}
			/* now |y| <eps */
			if (atomList->position[0] < -eps)
			{
				atomList->type = deleted;
				numberOfAtoms--;
				break;
			}
			break;

			default:
			return;
		}
		atomList++;
	}
	qsort(mol->listOfAtoms,mol->numberOfAtoms,sizeof(MolSymAtom),compare2atoms);
	mol->numberOfAtoms = numberOfAtoms;
}
/************************************************************************************************************/
static void removeByRotation(MolSymMolecule* mol, gint nax, gint sym)
{
	gdouble eps = 1e-3;
	gint i;
	gdouble c,ca,rr;
	gint numberOfAtoms = mol->numberOfAtoms;
	gint deleted = mol->numberOfDifferentKindsOfAtoms;     /* tag for deleted atom */
	MolSymAtom *atomList;

	if (nax<2) return;                

	if(mol->numberOfAtoms >0) eps = mol->listOfAtoms[0].eps;

	if (sym & SYM_V) c = cos(PI/nax);
	else c = cos(2.0*PI/nax);
  
	atomList = mol->listOfAtoms;

	for(i=0;i<mol->numberOfAtoms;i++)
 	{
		rr = atomList->position[0]*atomList->position[0] + atomList->position[1]*atomList->position[1];

		if ((atomList->type != deleted) && (rr > eps*eps))
		{
			if (atomList->position[1] < -eps)
			{
				atomList->type = deleted;
				numberOfAtoms--;
			}
			else 
			{
				/* calculate cos of molecule turned a bit clockwise */
				ca = atomList->position[0]/sqrt(rr);

				if (sym & SYM_V) ca += eps*atomList->position[1]/rr;
				else ca -= eps*atomList->position[1]/rr;
				if (ca < c)
				{
					atomList->type = deleted;
					numberOfAtoms--;
				}
			}
		}
		atomList++;
	}
	qsort(mol->listOfAtoms,mol->numberOfAtoms,sizeof(MolSymAtom),compare2atoms);
	mol->numberOfAtoms = numberOfAtoms;
}
/************************************************************************************************************/
void reduceMoleculeToItsBasisSetOfAtoms(MolSymMolecule* mol, gint sym, gint nax)
{
	/*
	printf("sym symh = %d\n", sym & SYM_H);
	printf("sym symv = %d\n", sym & SYM_V);
	printf("sym symd = %d\n", sym & SYM_D);
	printf("sym symi = %d\n", sym & SYM_I);
	printf("sym syms = %d\n", sym & SYM_S);
	printf("sym symDD = %d\n", sym & SYM_DD);
	printf("sym symT = %d\n", sym & SYM_T);
	printf("sym symO = %d\n", sym & SYM_O);
	printf("sym symIC = %d\n", sym & SYM_IC);
	printf("sym symR = %d\n", sym & SYM_R);
	printf("sym symU = %d\n", sym & SYM_U);
	printf("nax = %d\n", nax);
	*/

	/* cannot deal with high symmetries */
	if (sym & SYM_R) return;

	if (sym & SYM_O)
	{
		reduceForOctaedralSymmetry(mol,sym);
		setEpsToZero(mol);
		return;
	}
	if (sym & SYM_T)
	{
		reduceForTetraedalSymmetry(mol,sym);
		setEpsToZero(mol);
		return;
	}
	if (sym & SYM_IC)
	{
		reduceForIcosaedralSymmetry(mol,sym);
		setEpsToZero(mol);
		return;
	}
	if (sym & SYM_H) removeHalfSpace(mol,XY_PLANE);
	else if (sym & SYM_I) removeHalfSpace(mol,POINT_INV);

	if (nax > 1)
	{
		removeByRotation(mol,nax,sym);
		if (sym & SYM_D) removeHalfSpace(mol,ROT2X);
		if (sym & SYM_S) removeHalfSpace(mol,XY_PLANE);
	}
	setEpsToZero(mol);
}
