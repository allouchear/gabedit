/* PolyHedralOperators.c */
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
#include "../Symmetry/PolyHedralOperators.h"
#include "../Symmetry/SymmetryOperators.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SQU(x,y,z) ((x)*(x) + (y)*(y) + (z)*(z))
#define MYMAX(x,y)  ( ((x) > (y)) ? x : y )

/************************************************************************************************************/
/* 
 * compare 2 atoms -- order hierarchy:
 * 	1. Atom type
 * 	2. distance from center of gravity (bigger distances first)
 * 	3. z coordinate 
*/
gint compare2atomsUsingCenterOfGravity(const void* av,const void* bv)
{
	gint d;
	gdouble dd;
	gdouble xa,xb;
	MolSymAtom *a, *b;
	gdouble eps = 1e-3;


	a = (MolSymAtom*)av;
	b = (MolSymAtom*)bv;	/*cast from void* for compatibility with qsort*/

	eps = (a->eps+b->eps)/2;

	d = a->type - b->type;
	if (d != 0) return d;


	xa = a->position[0]*a->position[0] + a->position[1]*a->position[1] + a->position[2]*a->position[2];
	xb = b->position[0]*b->position[0] + b->position[1]*b->position[1] + b->position[2]*b->position[2];

	if (fabs(xa-xb) > 2.*eps*MYMAX(xa,xb))	return (xa < xb) ? 1 : -1;

	dd = a->position[2] - b->position[2];
	if (dd != 0.0) return (dd > 0) ? 1 : -1; 
	return 0;
}
/************************************************************************************************************/
gint searchForC3Axes(MolSymMolecule* mol, gchar* message)
{
	MolSymAtom *atomList;
	MolSymAtom *tmpAtomList;
	gdouble eps = 1e-3;


	gdouble rr,rr2;
	gdouble x,y,z;
	gdouble vx,vy,vz;
	gdouble ux,uy,uz;
	gdouble ax,ay,az;

	gdouble axis1[3], axis2[3];

	gint imin,imax;
	gint i,j;
	gint ret,nax;

	if(mol->numberOfAtoms >0) eps = mol->listOfAtoms[0].eps;

	qsort(mol->listOfAtoms,mol->numberOfAtoms,sizeof(MolSymAtom),compare2atomsUsingCenterOfGravity);

	/* determine number of atoms on shell with radius rr^1/2 */
	rr = 1.0;
	atomList = mol->listOfAtoms;
	for (imin=0;imin < mol->numberOfAtoms;imin++)
	{
		rr = atomList->position[0]*atomList->position[0]+atomList->position[1]*atomList->position[1]+atomList->position[2]*atomList->position[2];
		atomList++;
		if (rr > eps*eps) break;
	}

	for (imax = imin+1;imax<mol->numberOfAtoms;imax++)
	{
		rr2 = atomList->position[0]*atomList->position[0]+atomList->position[1]*atomList->position[1]+atomList->position[2]*atomList->position[2];
		if (fabs(rr-rr2) > 2.*eps*rr) break;
		atomList++;
	}

	if (imax-imin<4) return 0;	/* this cannot be a regular polyhedron */

	atomList = mol->listOfAtoms+imin;
	x = atomList->position[0];
	y = atomList->position[1];
	z = atomList->position[2];
	nax = 0;

	for (i=imin+1;i<imax;i++)
	{
		atomList++;
		/* first difference vector */
		vx = x - atomList->position[0];	
		vy = y - atomList->position[1];	
		vz = z - atomList->position[2];	
		rr = sqrt(vx*vx + vy*vy + vz*vz);
		vx /= rr;
		vy /= rr;
		vz /= rr;

		tmpAtomList = atomList;

		for (j=i+1;j<imax;j++)
		{
			tmpAtomList++;
			/* second normalized difference vector */
			ux = x - tmpAtomList->position[0];	
			uy = y - tmpAtomList->position[1];	
			uz = z - tmpAtomList->position[2];	
			rr = sqrt(ux*ux + uy*uy + uz*uz);
			ux /= rr;
			uy /= rr;
			uz /= rr;

			/* cos(phi) */
			if (fabs(vx*ux + vy*uy + vz*uz -0.5) < 0.866*eps/rr)
			{
				/* vector product */	
				ax = vy*uz - vz*uy;
				ay = vz*ux - vx*uz;
				az = vx*uy - vy*ux;
				rr = sqrt(ax*ax + ay*ay + az*az);
				ax /= rr;
				ay /= rr;
				az /= rr;

				/* is this a C3 axis ? */
				ret = rotateAroundArbitraryAxes(mol,3,ax,ay,az, message);
				if (ret<0) return ret; /* error */
				if (ret == 1)
				{
					nax++;
					if (nax == 1) /* first axis ? */
				 	{
						axis1[0] = ax;
						axis1[1] = ay;
						axis1[2] = az;
					}
					else if (nax == 2) /* first axis ? */
				 	{
						axis2[0] = ax;
						axis2[1] = ay;
						axis2[2] = az;
				 	}
				}
			}
		}
	}
	rotateMoleculeToPlaceFirstPointOnZAxisAndSecondOnXZPlane(mol,axis1,axis2); 
	qsort(mol->listOfAtoms,mol->numberOfAtoms,sizeof(MolSymAtom),compare2atomsUsingCenterOfGravity);
	return nax;
}

/************************************************************************************************************/
/* rotate around artbitary axes u by angle 2PI/n and check for identity */
/* returns 1 if invariant against rotation, 0 if not, -1 : error */
/* u = (ux,uy,uz) should be a normalized vector */
gint rotateAroundArbitraryAxes(MolSymMolecule* mol, gint n,gdouble ux, gdouble uy, gdouble uz, gchar* message)
{
	gint i;
	gdouble s,c;
	gdouble a11,a12,a13;
	gdouble a21,a22,a23;
	gdouble a31,a32,a33;
	gdouble eps = 1e-3;

	MolSymAtom *atomList;
	MolSymAtom *tmpAtomList;
	MolSymMolecule tmpMol;
	if(mol->numberOfAtoms>0) eps = mol->listOfAtoms[0].eps;

	/* copy molecule */
	tmpMol.numberOfAtoms=mol->numberOfAtoms;
	tmpMol.listOfAtoms = (MolSymAtom*)g_malloc(tmpMol.numberOfAtoms*sizeof(MolSymAtom));
	if (tmpMol.listOfAtoms == NULL)
	 {
		if(message)
			sprintf(message,"rotateAroundArbitraryAxes : memory allocation for molecule failed\n");
		return -1;
	 }

	/* construct rotation matrix */
	s = sin(2*PI/n);
	c = cos(2*PI/n);
	a11 = ux*ux*(1-c) + c;
	a12 = ux*uy*(1-c) - s*uz;
	a13 = ux*uz*(1-c) + s*uy;

	a21 = uy*ux*(1-c) + s*uz;
	a22 = uy*uy*(1-c) + c;
	a23 = uy*uz*(1-c) - s*ux;

	a31 = uz*ux*(1-c) - s*uy;
	a32 = uz*uy*(1-c) + s*ux;
	a33 = uz*uz*(1-c) + c;

	/* perform rotation */
	atomList = mol->listOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;

	for (i=0;i<mol->numberOfAtoms;i++)
	 {
		tmpAtomList->position[0] = a11*atomList->position[0] + a12*atomList->position[1] + a13*atomList->position[2];
		tmpAtomList->position[1] = a21*atomList->position[0] + a22*atomList->position[1] + a23*atomList->position[2];
		tmpAtomList->position[2] = a31*atomList->position[0] + a32*atomList->position[1] + a33*atomList->position[2];
		tmpAtomList->type = atomList->type;
		tmpAtomList->eps  = atomList->eps;
		atomList++;
		tmpAtomList++;
	 }

	/* test for equivalence */
	atomList = mol->listOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;

	i = checkequivalence2Molecules(atomList, tmpAtomList, mol->numberOfAtoms);
	g_free(tmpMol.listOfAtoms);
	return i; 
}
/************************************************************************************************************/
/* rotate molecule to place point x1 on z axis, x2 on xz plane */
/* original in mol, result in tmpMol */
void rotateMoleculeToPlaceFirstPointOnZAxisAndSecondOnXZPlane(MolSymMolecule* mol,gdouble* x1,gdouble* x2)
{
	gint i;
	gdouble a11,a12,a21,a22,a23,a31,a32,a33;
	gdouble r,R;
	gdouble x,y;
	MolSymAtom *atomList;

	/* calculate rotation matrix elements */

	r = sqrt(x1[0]*x1[0]+x1[1]*x1[1]);
	R = sqrt(x1[0]*x1[0]+x1[1]*x1[1]+x1[2]*x1[2]);

	if (r<1.E-12) return; /* nothing to do */

	a11 =  x1[1]/r;
	a12 = -x1[0]/r;
	a21 =  x1[0]*x1[2]/(r*R);
	a22 =  x1[1]*x1[2]/(r*R);
	a23 = -r/R;
	a31 =  x1[0]/R;
	a32 =  x1[1]/R;
	a33 =  x1[2]/R;

	/* perform rotation */
	atomList = mol->listOfAtoms;

	for (i=0;i<mol->numberOfAtoms;i++)
	 {
		x = a11*atomList->position[0] + a12*atomList->position[1];
		y = a21*atomList->position[0] + a22*atomList->position[1] + a23*atomList->position[2];
		atomList->position[2] = a31*atomList->position[0] + a32*atomList->position[1] + a33*atomList->position[2];
		atomList->position[0] = x;
		atomList->position[1] = y;
		atomList++;
	 }
	/* turn second axis */
	x = a11*x2[0] + a12*x2[1];
	y = a21*x2[0] + a22*x2[1] + a23*x2[2];
	/* rotate now to xz plane */

	r = sqrt(x*x+y*y);
	a11 = x/r;
	a12 = y/r;
	a21 = -a12;
	a22 = a11;

	atomList =  mol->listOfAtoms;

	for (i=0;i<mol->numberOfAtoms;i++)
	 {
		x = a11*atomList->position[0] + a12*atomList->position[1];
		atomList->position[1] = a21*atomList->position[0] + a22*atomList->position[1];
		atomList->position[0] = x;
		atomList++;
	 }
}
/************************************************************************************************************/
void rotationAroundZAxisByPi(MolSymMolecule* mol)
{
	gint i;
	MolSymAtom *atomList;

	/* perform reflexion */

	atomList = mol->listOfAtoms;

	for (i=0;i<mol->numberOfAtoms;i++)
	 {
		atomList->position[0] = - atomList->position[0];
		atomList->position[1] = - atomList->position[1];
		atomList++;
	 }
}
