/* SymmetryOperators.c */
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define SQU(x,y,z) ((x)*(x) + (y)*(y) + (z)*(z))
#define SQU2(x,y) ((x)*(x) + (y)*(y))
#define MYMAX(x,y)  ( ((x) > (y)) ? x : y )

/************************************************************************************************************/
/* compare 2 atoms -- Hierarchy:
	1. Atom type
	2. distance from z-axis (bigger distances first)
	3. angle around z -axis 
	4. z coordinate
*/
gint compare2atoms(const void* av,const void* bv)
{
	gint d;
	gdouble dd;
	gdouble xa,xb;
	gdouble eps;
	MolSymAtom *a, *b;

	a = (MolSymAtom*)av;
	b = (MolSymAtom*)bv;
	eps = (a->eps + b->eps)/2;

	d = a->type - b->type;
	if (d != 0) return d;

	xa = a->position[0]*a->position[0] + a->position[1]*a->position[1];
	xb = b->position[0]*b->position[0] + b->position[1]*b->position[1]; 

	if (fabs(xa-xb) > eps*(xa+xb))	return (xa < xb) ? 1 : -1;

	if ((xa > eps*eps) && (xb > eps*eps))
	{
	/* angle is only significant if not on z-axis */
		xa = (a->position[1]>0) ? -1-a->position[0]/sqrt(xa) : 1+a->position[0]/sqrt(xa);
		xb = (b->position[1]>0) ? -1-b->position[0]/sqrt(xb) : 1+b->position[0]/sqrt(xb);
		if (fabs(xa-xb) > eps*2.*MYMAX(xa,xb))	return (xa > xb) ? 1 : -1;
	}
	dd = a->position[2] - b->position[2];
	if (dd != 0.0) return (dd > 0) ? 1 : -1; 
	return 0;
}
/************************************************************************************************************/
/*check two molecules for equivalence */
gint checkequivalence2Molecules(MolSymAtom* a,MolSymAtom* b,gint n)
{
	MolSymAtom* atomList;
	gint i,j;
	gdouble eps = 1e-3;

	eps = a->eps;


	for (i=0;i<n;i++)
	{
		atomList = b;
		for (j=0;j<n;j++)
	 	{
			if ((a->type == atomList->type) &&
			 (SQU(a->position[0] - atomList->position[0],a->position[1] - atomList->position[1],
			a->position[2] - atomList->position[2]) < eps*eps)) break;
			atomList++;;
		}
		if (j==n) return 0;
		a++;
	}
	return 1;
}
/************************************************************************************************************/
gint determineOrderOfZAxis(MolSymMolecule* mol, gint maxf) /* determine order of z - axis */
{
	gint prime[]= {2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,0};
	gint i;
	gint nf=1;
	gdouble eps = 1e-3;

	if(mol->numberOfAtoms >0) eps = mol->listOfAtoms[0].eps;
	
	for (i=0;prime[i] > 0;i++)
	 {
		do
		{
			nf *= prime[i];
			if (nf > maxf) break;
		} while (rotateAroundZaxes(mol,nf) == 1);
		nf /= prime[i];
	 }
	return nf;
}
/************************************************************************************************************/
/* rotate around z axes by angle 2PI/n and check for identity */
/* returns 1 if invariant against rotation, 0 if not, -1 : error */
/* negative n - perform improper rotation */
gint rotateAroundZaxes(MolSymMolecule* mol, gint n)
{
	gint i;
	gdouble s,c;
	MolSymAtom *atomList,*tmpAtomList;
	MolSymMolecule tmpMol;
	gdouble eps = 1e-3;

	if(mol->numberOfAtoms >0) eps = mol->listOfAtoms[0].eps;

	/* copy molecule */
	tmpMol.numberOfAtoms=mol->numberOfAtoms;
	tmpMol.listOfAtoms = (MolSymAtom*)g_malloc(tmpMol.numberOfAtoms*sizeof(MolSymAtom));
	if (tmpMol.listOfAtoms == NULL)
	 {
		return -1;
	 }

	/* perform rotation */
	s = sin(2*PI/n);
	c = cos(2*PI/n);
	if (n<0) s = -s;	 /* always turn in positive direction */
	atomList = mol->listOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;

	for (i=0;i<mol->numberOfAtoms;i++)
	 {
		tmpAtomList->position[0] = c*atomList->position[0] - s*atomList->position[1];
		tmpAtomList->position[1] = s*atomList->position[0] + c*atomList->position[1];

		if ((SQU2(atomList->position[0],atomList->position[1]) > eps*eps) &&
		(SQU2(tmpAtomList->position[0] - atomList->position[0],tmpAtomList->position[1] - atomList->position[1]) <eps*eps))
		{
			/* rotation angle too small */
			g_free(tmpMol.listOfAtoms);
			return 0;
		}

		if (n>0)	
			/* proper rotation */
			tmpAtomList->position[2] =  atomList->position[2];
		else	
			/* improper rotation */
			tmpAtomList->position[2] = -atomList->position[2];
		tmpAtomList->type = atomList->type;
		atomList++;;
		tmpAtomList++;
	 }
	/* test for equivalence */
	atomList = mol->listOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;

	i = checkequivalence2Molecules(atomList,tmpAtomList,mol->numberOfAtoms);
	g_free(tmpMol.listOfAtoms);
	return i; 
}
/************************************************************************************************************/
/* tests for reflection on xy xz or yz plane or point inversion */
/* returns 1 if invariant against reflection, 0 if not, -1 : error */
gint testRotationReflection(MolSymMolecule* mol, RotationReflectionOperator op)
{
	gint i;
	MolSymAtom *atomList,*tmpAtomList;
	MolSymMolecule tmpMol;

	/* copy molecule */
	tmpMol.numberOfAtoms=mol->numberOfAtoms;
	tmpMol.listOfAtoms = (MolSymAtom*)g_malloc(tmpMol.numberOfAtoms*sizeof(MolSymAtom));
	if (tmpMol.listOfAtoms == NULL)
	 {
		return -1;
	 }

	atomList = mol->listOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;

	for (i=0;i<mol->numberOfAtoms;i++)
	{
		switch(op)
		{
		case XY_PLANE:
		tmpAtomList->position[0] =  atomList->position[0];
		tmpAtomList->position[1] =  atomList->position[1];
		tmpAtomList->position[2] = -atomList->position[2];
		break;

		case XZ_PLANE:
		tmpAtomList->position[0] =  atomList->position[0];
		tmpAtomList->position[1] = -atomList->position[1];
		tmpAtomList->position[2] =  atomList->position[2];
		break;

		case YZ_PLANE:
		tmpAtomList->position[0] = -atomList->position[0];
		tmpAtomList->position[1] = atomList->position[1];
		tmpAtomList->position[2] = atomList->position[2];
		break;

		case POINT_INV:
		tmpAtomList->position[0] = -atomList->position[0];
		tmpAtomList->position[1] = -atomList->position[1];
		tmpAtomList->position[2] = -atomList->position[2];
		break;

		case ROT2X:
		tmpAtomList->position[0] =  atomList->position[0];
		tmpAtomList->position[1] = -atomList->position[1];
		tmpAtomList->position[2] = -atomList->position[2];
		break;

		case ROT2Y:
		tmpAtomList->position[0] = -atomList->position[0];
		tmpAtomList->position[1] =  atomList->position[1];
		tmpAtomList->position[2] = -atomList->position[2];
		break;
		case ROT2Z:
		tmpAtomList->position[0] = -atomList->position[0];
		tmpAtomList->position[1] = -atomList->position[1];
		tmpAtomList->position[2] =  atomList->position[2];
		}
		tmpAtomList->type = atomList->type;
		atomList++;;
		tmpAtomList++;
	 }

	/* test for equivalence */
	atomList = mol->listOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;

	i = checkequivalence2Molecules(atomList,tmpAtomList,mol->numberOfAtoms);
 
	g_free(tmpMol.listOfAtoms);
	return i;
}

/************************************************************************************************************/
/* rotate molecule around z-axis to place point (x,y,z) on xz plane */
/* original in mol, result in tmpMol */
void  rotateAroundZaxesToPlacePointOnXZPlane(MolSymMolecule* mol,MolSymMolecule* tmpMol,gdouble x,gdouble y)
{
	gint i;
	gdouble s,c;
	MolSymAtom *atomList,*tmpAtomList;

	/* perform rotation */
	c = x/sqrt(x*x+y*y);
	s = -y/sqrt(x*x+y*y);

	atomList = mol->listOfAtoms;
	tmpAtomList = tmpMol->listOfAtoms;

	for (i=0;i<mol->numberOfAtoms;i++)
	{
		tmpAtomList->position[0] = c*atomList->position[0] - s*atomList->position[1];
		tmpAtomList->position[1] = s*atomList->position[0] + c*atomList->position[1];
		tmpAtomList->position[2] =	atomList->position[2];
		tmpAtomList->type = atomList->type;
		tmpAtomList->eps = atomList->eps;
		atomList++;;
		tmpAtomList++;
	}
	qsort(tmpMol->listOfAtoms,tmpMol->numberOfAtoms,sizeof(MolSymAtom),compare2atoms);
}
/************************************************************************************************************/
/* rotate molecule around x-axis to place y- axis onto z_axis*/
void rotateAroundXAxisToPlaceYAxisOntoZAxis(MolSymMolecule* mol)
{
	gint i;
	gdouble y;
	MolSymAtom * atomList;

	/* perform rotation */

	atomList = mol->listOfAtoms;

	for (i=0;i<mol->numberOfAtoms;i++)
	 {
		y = atomList->position[1];
		atomList->position[1] = - atomList->position[2];
		atomList->position[2] = y;
		atomList++;;
	 }
	qsort(mol->listOfAtoms,mol->numberOfAtoms,sizeof(MolSymAtom),compare2atoms);
}
/************************************************************************************************************/
/* rotate molecule around y-axis to place x- axis onto z_axis*/
void rotateAroundYAxisToPlaceXAxisOntoZAxis(MolSymMolecule* mol)
{
	gint i;
	gdouble x;
	MolSymAtom * atomList;

	/* perform rotation */

	atomList = mol->listOfAtoms;

	for (i=0;i<mol->numberOfAtoms;i++)
	 {
		x = atomList->position[0];
		atomList->position[0] = - atomList->position[2];
		atomList->position[2] =	x;
		atomList++;;
	 }
	qsort(mol->listOfAtoms,mol->numberOfAtoms,sizeof(MolSymAtom),compare2atoms);
}

/************************************************************************************************************/
/* check for v planes or D -axis -- molecule should be sorted! */
/* returns 1 for v plane, 2 for D-axis, 0 for nothing ,-1 for error */
gint checkForVPlanesOrDAxis(MolSymMolecule* mol,gint nax, gchar* message)
{
	gint i,ret;
	gint nmin,nmax;
	gint typ;
	gdouble x,y,rr;

	MolSymAtom * atomList;
	MolSymMolecule tmpMol;

	gdouble eps = 1e-3;

	if(mol->numberOfAtoms >0) eps = mol->listOfAtoms[0].eps;

	/* copy molecule */
	tmpMol.numberOfAtoms=mol->numberOfAtoms;
	tmpMol.listOfAtoms = (MolSymAtom*)g_malloc(tmpMol.numberOfAtoms*sizeof(MolSymAtom));
	if (tmpMol.listOfAtoms == NULL)
	{
		sprintf(message, "From checkForVPlanesOrDAxis : memory allocation for molecule failed\n");
		return -1;
	}

	atomList = mol->listOfAtoms;

	/* find first atom not on z-axis */
	for (nmin=0;nmin<mol->numberOfAtoms;nmin++)
	 {
		rr = atomList->position[0]*atomList->position[0] + atomList->position[1]*atomList->position[1];
		typ = atomList->type;
		if (rr > eps*eps) break;
		atomList++;;
	 }
	
	/* should not happen for sorted Cn molecule */
	if (nmin >= mol->numberOfAtoms)
	 {
		sprintf(message, "From checkForVPlanesOrDAxis : This is a linear molecule!\n");
		return -2;
	 }

	/* count atoms on largest cylinder */
	for (nmax=nmin;nmax < mol->numberOfAtoms;nmax++)
	 {
		if ((typ != atomList->type) ||
		 (fabs(rr - atomList->position[0]*atomList->position[0] - atomList->position[1]*atomList->position[1]) > eps*2.*rr))
			break;
		atomList++;;
	 }

	/* printf("nmin = %d nmax = %d nax = %d\n",nmin,nmax, nax);*/
	if ((nax>0) && (nmax-nmin) % nax != 0)
	 {
		sprintf(message, "From checkForVPlanesOrDAxis: Inconsistent rotation symmetry \n");
		return -2;
	 }

	atomList = mol->listOfAtoms + nmin;
	ret = 0;

	/* now check for v planes and D axes */
	for (i=0;i<(nmax-nmin)/nax;i++)
	 {
		/* turn one atom onto xz -plane */
		x = atomList->position[0];
		y = atomList->position[1];
		rotateAroundZaxesToPlacePointOnXZPlane(mol,&tmpMol,x,y);

		ret = testRotationReflection(&tmpMol,XZ_PLANE); /* v-plane ? */
		/* printf("ret testRotationReflection %d\n",ret);*/
		if (ret != 0) break; /* yes- found */

		ret = testRotationReflection(&tmpMol,ROT2X); /* D-axis ? */
		if (++ret != 1) break; /* yes- found */
		ret--;	
		/* turn center point between one atom and the next onto xz -plane */
		atomList++;;
		rotateAroundZaxesToPlacePointOnXZPlane(mol,&tmpMol,(x+atomList->position[0])/2.,(y+atomList->position[1])/2.);

		ret = testRotationReflection(&tmpMol,XZ_PLANE); /* v-plane ? */
		if (ret != 0) break; /* yes- found */

		ret = testRotationReflection(&tmpMol,ROT2X); /* D-axis ? */
		if (++ret != 1) break; /* yes- found */
		ret--;
	 }
	if (ret > 0) /* copy tmpMol back to mol */
	 memcpy(mol->listOfAtoms,tmpMol.listOfAtoms,mol->numberOfAtoms*sizeof(MolSymAtom));
	g_free(tmpMol.listOfAtoms);
	return ret;
}
/************************************************************************************************************/
