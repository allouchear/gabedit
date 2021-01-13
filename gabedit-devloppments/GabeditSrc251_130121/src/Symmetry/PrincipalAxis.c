/* PrincipalAxis.c */
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
#include "../Symmetry/SymmetryOperators.h"

#include <stdlib.h>
#include <math.h>

#define EPSILON 1.0E-12
#define SQU(x,y,z) ((x)*(x) + (y)*(y) + (z)*(z))

/************************************************************************************************************/
/* Jacobi diagonalisation of 3x3 symmetric matrix */
/* matrix mat stored like   0 3 5    
                              1 4
                                2   */
static void jacobi(gdouble *mat, gdouble evec[3][3])
{
  
	gdouble t,s,u;
	gdouble a;
	evec[0][1] = evec[0][2] = evec[1][0] = 0.0;  /* unity matrix */
	evec[1][2] = evec[2][0] = evec[2][1] = 0.0;  /* unity matrix */
	evec[0][0] = evec[1][1] = evec[2][2] = 1.0;

	/* do jacobi sweep */
	while(SQU(mat[3],mat[4],mat[5]) > EPSILON)
	{
		/* set mat[3] to zero */
		if (mat[3]*mat[3] > EPSILON)
		{
			t = (mat[1]-mat[0])/(2.*mat[3]);
	 		t = (t>0) ? 1./(t+sqrt(t*t+1.)) : -1./(-t+sqrt(t*t+1.));
	 		s = t/(sqrt(t*t+1.));
	  		u = s*t/(s+t);

	  		mat[0] -= t*mat[3];
	  		mat[1] += t*mat[3];
	  		a = mat[5];
	  		mat[5] -= s*(mat[4]+u*mat[5]);
	  		mat[4] += s*(  a   -u*mat[4]);
	  		mat[3] = 0.;
	  
	 		a = evec[0][0];
	  		evec[0][0] -= s*(evec[0][1]+u*evec[0][0]);
	  		evec[0][1] += s*(   a   -u*evec[0][1]);

	  		a = evec[1][0];
	  		evec[1][0] -= s*(evec[1][1]+u*evec[1][0]);
	  		evec[1][1] += s*(   a   -u*evec[1][1]);

	 		a = evec[2][0];
	  		evec[2][0] -= s*(evec[2][1]+u*evec[2][0]);
	  		evec[2][1] += s*(   a   -u*evec[2][1]);
		}

		/* set mat[5] to zero */
		if (mat[5]*mat[5] > EPSILON)
		{
			t = (mat[2]-mat[0])/(2.*mat[5]);
	  		t = (t>0) ? 1./(t+sqrt(t*t+1.)) : -1./(-t+sqrt(t*t+1.));
	  		s = t/(sqrt(t*t+1.));
	  		u = s*t/(s+t);

	  		mat[0] -= t*mat[5];
	  		mat[2] += t*mat[5];
	  		a = mat[3];
	  		mat[3] -= s*(mat[4]+u*mat[3]);
	  		mat[4] += s*(  a   -u*mat[4]);
	  		mat[5] = 0.;
	  
	  		a = evec[0][0];
	  		evec[0][0] -= s*(evec[0][2]+u*evec[0][0]);
	  		evec[0][2] += s*(   a   -u*evec[0][2]);

	  		a = evec[1][0];
	  		evec[1][0] -= s*(evec[1][2]+u*evec[1][0]);
	  		evec[1][2] += s*(   a   -u*evec[1][2]);

	  		a = evec[2][0];
	  		evec[2][0] -= s*(evec[2][2]+u*evec[2][0]);
	  		evec[2][2] += s*(   a   -u*evec[2][2]);
		}

		/* set mat[4] to zero */
		if (mat[4]*mat[4] > EPSILON)
		{
	  		t = (mat[2]-mat[1])/(2.*mat[4]);
	  		t = (t>0) ? 1./(t+sqrt(t*t+1.)) : -1./(-t+sqrt(t*t+1.));
	  		s = t/(sqrt(t*t+1.));
	  		u = s*t/(s+t);

	  		mat[1] -= t*mat[4];
	  		mat[2] += t*mat[4];
	  		a = mat[3];
	  		mat[3] -= s*(mat[5]+u*mat[3]);
	  		mat[5] += s*(  a   -u*mat[5]);
	  		mat[4] = 0.;

	  		a = evec[0][1];
	  		evec[0][1] -= s*(evec[0][2]+u*evec[0][1]);
	  		evec[0][2] += s*(   a   -u*evec[0][2]);

	  		a = evec[1][1];
	  		evec[1][1] -= s*(evec[1][2]+u*evec[1][1]);
	  		evec[1][2] += s*(   a   -u*evec[1][2]);

	  		a = evec[2][1];
	  		evec[2][1] -= s*(evec[2][2]+u*evec[2][1]);
	  		evec[2][2] += s*(   a   -u*evec[2][2]);
		}
	}
}
/************************************************************************************************************/
static void swap(gint i,gint j,gdouble* mat, gdouble vecs[3][3])
{
	gint k;
	gdouble t;
	t = mat[i];
	mat[i] = mat[j];
	mat[j] = t;
	for(k=0;k<3;k++)
	{
		t = vecs[k][i];
		vecs[k][i] = vecs[k][j];
		vecs[k][j] = t;
	}
}
/************************************************************************************************************/
/* inertial moment of a molecule */
/* matrix mat stored like   0 3 5    
                              1 4
                                2   */
void determinePrincipalAxis(MolSymMolecule* mol,gdouble* centerOfGravity, gint* numberOfEquivalentAxes,
	       gdouble* inertialMoment, gdouble axes[3][3], gdouble principalAxisTolerance, gboolean sorting)
{
	gint i,j;
	gdouble mat[6];
	gdouble m;
	gdouble mtot = 0.0;
	gdouble x,y,z;
	MolSymAtom *atomList;

	atomList = mol->listOfAtoms;
	x = y = z =0.0;
	for (i=0;i<mol->numberOfAtoms;i++)	  /* center of gravity and total mass */
	{
		/* m = sqrt(prime[atomList->type]);*/
		m = fabs(mol->masse[atomList->type]);
		x += m*atomList->position[0];
		y += m*atomList->position[1];
		z += m*atomList->position[2];
		mtot += m;
		atomList++;
	  }
	centerOfGravity[0] = x/mtot;
	centerOfGravity[1] = y/mtot;
	centerOfGravity[2] = z/mtot;
	
	for(i=0;i<6;i++) mat[i]=0.0;

	atomList = mol->listOfAtoms;
	for (i=0;i<mol->numberOfAtoms;i++)	  /* build up inertial tensor */
	{
		x = (atomList->position[0] -= centerOfGravity[0]);
		y = (atomList->position[1] -= centerOfGravity[1]);
		z = (atomList->position[2] -= centerOfGravity[2]);
		/* m = sqrt(prime[atomList->type]);*/
		m = fabs(mol->masse[atomList->type]);
		mat[0] += m*(y*y+z*z);
		mat[1] += m*(x*x+z*z);
		mat[2] += m*(x*x+y*y);
		mat[3] -= m*(x*y);
		mat[4] -= m*(y*z);
		mat[5] -= m*(x*z);
		atomList++;
	}

	jacobi(mat,axes);	  /* diagonalize tensor */

	/* sort eigenvalues */
	if (mat[0]<mat[1]) swap(0,1,mat,axes);
	if (mat[1]<mat[2]) swap(1,2,mat,axes);
	if (mat[0]<mat[1]) swap(0,1,mat,axes);

	inertialMoment[0] = mat[0];
	inertialMoment[1] = mat[1];
	inertialMoment[2] = mat[2];

	/* normalize moments if not pointlike */
	if (mat[0] > 1.E-8) 
	{
		mat[1] /= mat[0];
		mat[2] /= mat[0];
		mat[0] = 1.0;
	}

	if ((mat[1]-mat[2])*(mat[1]-mat[2])< principalAxisTolerance*principalAxisTolerance) 
		swap(0,2,mat,axes); 

	/* determin number of equivalent axes */
	*numberOfEquivalentAxes = 1;  
	if ((mat[0]-mat[1])*(mat[0]-mat[1]) < principalAxisTolerance*principalAxisTolerance)
  		(*numberOfEquivalentAxes)++; /* 2 axes equiv. */

	if ((mat[0]-mat[2])*(mat[0]-mat[2]) < principalAxisTolerance*principalAxisTolerance)
  		(*numberOfEquivalentAxes)++; /* 3 axes equiv. */
	if (mat[2] < principalAxisTolerance) *numberOfEquivalentAxes = -*numberOfEquivalentAxes; /* linear or point */

	/* multiply atom vectors v_i by eigenvector matrix v_i' = v_i * V
	   to rotate molecule - principal axes will be equivalent to coordinate
	   axes
	 */
	atomList = mol->listOfAtoms;
	for (i=0;i<mol->numberOfAtoms;i++)	  /* perform rotation */
	{
		x = atomList->position[0];
		y = atomList->position[1];
		for (j=0;j<3;j++)
			atomList->position[j] = x*axes[0][j] + y*axes[1][j] + atomList->position[2]*axes[2][j];
		atomList++;
	}
	if(sorting) qsort(mol->listOfAtoms,mol->numberOfAtoms,sizeof(MolSymAtom),compare2atoms);
}
