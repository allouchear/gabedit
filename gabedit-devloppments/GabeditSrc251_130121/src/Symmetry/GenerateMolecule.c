/* GenerateMolecule.c */
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
#include "../Symmetry/ReduceMolecule.h"
#include "../Symmetry/ReducePolyHedralMolecule.h"
#include "../Symmetry/PolyHedralOperators.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define SQU(x,y,z) ((x)*(x) + (y)*(y) + (z)*(z))
#define  C5X  0.6070619987
#define  C5Z  0.7946544719

/************************************************************************************************************/
static void printAMolecule(MolSymMolecule* mol)
{

	gint i;
	MolSymAtom *atomList;
	atomList = mol->listOfAtoms;

	for (i=0;i<mol->numberOfAtoms;i++)
	{
		printf("%d %f %f %f\n",
		atomList->type ,
		atomList->position[0],
		atomList->position[1],
		atomList->position[2]
		      );
		atomList++;;
	}
}
/************************************************************************************************************/
static gint applyProperRotationOneAtom(gdouble point[], gdouble axis[], gdouble angle, gdouble oldAtom[], gdouble newAtom[])
{
	gint i,j,k;
	gdouble c, s;
	gdouble m[3][3];

	for (i=0;i<3;i++) newAtom[i] = oldAtom[i];
	c = cos(angle);
	s = sin(angle);

	for (i=0;i<3;i++) 
	for (j=i+1;j<3;j++) 
	{
		gdouble p = s;
		k = (j+1)%3;
		if(k==i) { k = (i+1)%3; p = -p;}
		m[i][j] = (1-c)*axis[i]*axis[j]-p*axis[k];
		m[j][i] = (1-c)*axis[i]*axis[j]+p*axis[k];
	}
	for (i=0;i<3;i++) 
		m[i][i] = c + (1-c)*axis[i]*axis[i];

	/*Translate the reflection plane to the origin, and translate atom by the same vector*/
	for (i=0;i<3;i++) oldAtom[i] -= point[i];
	/*Apply the reflection matrix*/
	for (i=0;i<3;i++) 
	{
		newAtom[i] = 0.0;
		for (j=0;j<3;j++) 
		newAtom[i] += m[i][j]*oldAtom[j]; 
	}
	/*Translate the atoms and reflection plane back*/
	for (i=0;i<3;i++) oldAtom[i] += point[i];
	for (i=0;i<3;i++) newAtom[i] += point[i];
	return 0;
}
/************************************************************************************************************/
static gint applyReflectionOneAtom(gdouble point[], gdouble normal[], gdouble oldAtom[], gdouble newAtom[])
{
	gdouble len = 0;
	gint i,j;
	gdouble a[3];
	gdouble m[3][3];

	for (i=0;i<3;i++) newAtom[i] = oldAtom[i];
	for (i=0;i<3;i++) len += normal[i]*normal[i];
	if(len<1e-14) return 1;
	len = sqrt(len);
	for (i=0;i<3;i++) a[i] = normal[i]/len;
	for (i=0;i<3;i++) 
	for (j=i+1;j<3;j++) 
	{
		m[i][j] = -2*a[i]*a[j];
		m[j][i] = m[i][j];
	}
	for (i=0;i<3;i++) 
		m[i][i] = -a[i]*a[i]+a[(i+1)%3]*a[(i+1)%3]+a[(i+2)%3]*a[(i+2)%3];

	/*Translate the reflection plane to the origin, and translate atom by the same vector*/
	for (i=0;i<3;i++) oldAtom[i] -= point[i];
	/*Apply the reflection matrix*/
	for (i=0;i<3;i++) 
	{
		newAtom[i] = 0.0;
		for (j=0;j<3;j++) 
		newAtom[i] += m[i][j]*oldAtom[j]; 
	}
	/*Translate the atoms and reflection plane back*/
	for (i=0;i<3;i++) oldAtom[i] += point[i];
	for (i=0;i<3;i++) newAtom[i] += point[i];
	return 0;
}
/************************************************************************************************************/
/*
static gint applyImproperRotationOneAtom(gdouble point[], gdouble axis[], gdouble angle, gdouble oldAtom[], gdouble newAtom[])
{
 	gint res = applyProperRotationOneAtom(point, axis, angle, oldAtom, newAtom);
 	if(res==0) res = applyReflectionOneAtom(point, axis, oldAtom, newAtom);
	return res;
}
*/
/************************************************************************************************************/
static gint freeAMolecule(MolSymMolecule* mol)
{

	gint i;
	if(mol->numberOfAtoms>0 && mol->listOfAtoms) g_free(mol->listOfAtoms);
	mol->listOfAtoms = NULL;
	mol->numberOfAtoms = 0;
	if(mol->numberOfDifferentKindsOfAtoms>0 && mol->symbol)
	{
		for(i=0;i<mol->numberOfDifferentKindsOfAtoms;i++)
			if( mol->symbol[i]) g_free(mol->symbol[i]);
		g_free(mol->symbol);
	}
	mol->symbol = NULL;
	if(mol->numberOfDifferentKindsOfAtoms>0 && mol->numberOfAtomsOfEachType) g_free(mol->numberOfAtomsOfEachType);
	mol->numberOfAtomsOfEachType = 0;
	if(mol->numberOfDifferentKindsOfAtoms>0 && mol->masse) g_free(mol->masse);
	mol->masse = 0;
	mol->numberOfDifferentKindsOfAtoms = 0;
	return 0;
}
/************************************************************************************************************/
static void removeEquivAtoms(MolSymMolecule* mol)
{
	gdouble eps = 1e-3;
	gint i;
	gint j;
	gint numberOfAtoms = mol->numberOfAtoms;
	gint deleted = mol->numberOfDifferentKindsOfAtoms;     /* tag for deleted atom */
	gdouble rr = 0;
	gdouble xx = 0;
	gdouble yy = 0;
	gdouble zz = 0;

	for(i=1;i<mol->numberOfAtoms;i++)
	{
		for(j=0;j<i;j++)
		{
			if(mol->listOfAtoms[i].type != deleted && mol->listOfAtoms[j].type == mol->listOfAtoms[i].type)
			{
				xx = mol->listOfAtoms[i].position[0] -  mol->listOfAtoms[j].position[0];
				yy = mol->listOfAtoms[i].position[1] -  mol->listOfAtoms[j].position[1];
				zz = mol->listOfAtoms[i].position[2] -  mol->listOfAtoms[j].position[2];
				eps = fabs( mol->listOfAtoms[i].eps +  mol->listOfAtoms[j].eps)/2;
				rr = SQU(xx,yy,zz);
				if(rr<eps*eps)
				{
					mol->listOfAtoms[j].type = deleted;
					numberOfAtoms--;
				}
			}
		}
	}
	qsort(mol->listOfAtoms,mol->numberOfAtoms,sizeof(MolSymAtom),compare2atoms);
	mol->numberOfAtoms = numberOfAtoms;
}
/************************************************************************************************************/
static gint copyAMolecule(MolSymMolecule* mol, MolSymMolecule* mol2Copy)
{

	gint i;
	MolSymAtom *atomList;
	MolSymAtom *tmpAtomList;

	if(mol->numberOfAtoms>0 && mol->listOfAtoms) g_free(mol->listOfAtoms);
	mol->listOfAtoms = NULL;
	if(mol2Copy->numberOfAtoms<1) return -1;

	mol->numberOfDifferentKindsOfAtoms = mol2Copy->numberOfDifferentKindsOfAtoms;

	if(mol->numberOfDifferentKindsOfAtoms>0 && mol->symbol)
	{
		for(i=0;i<mol->numberOfDifferentKindsOfAtoms;i++)
			if( mol->symbol[i]) g_free(mol->symbol[i]);
		g_free(mol->symbol);
	}
	mol->symbol = (gchar**)g_malloc(mol->numberOfDifferentKindsOfAtoms*sizeof(gchar*));

	if(mol->numberOfDifferentKindsOfAtoms>0 && mol->numberOfAtomsOfEachType) g_free(mol->numberOfAtomsOfEachType);
	mol->numberOfAtomsOfEachType = (gint*)g_malloc(mol->numberOfDifferentKindsOfAtoms*sizeof(gint));

	if(mol->numberOfDifferentKindsOfAtoms>0 && mol->masse) g_free(mol->masse);
	mol->masse = (gdouble*)g_malloc(mol->numberOfDifferentKindsOfAtoms*sizeof(gdouble));

	mol->numberOfAtoms=mol2Copy->numberOfAtoms;
	mol->listOfAtoms = (MolSymAtom*)g_malloc(mol->numberOfAtoms*sizeof(MolSymAtom));

	atomList = mol->listOfAtoms;
	tmpAtomList = mol2Copy->listOfAtoms;

	for (i=0;i<mol->numberOfAtoms;i++)
	{
		atomList->position[0] =  tmpAtomList->position[0];
		atomList->position[1] =  tmpAtomList->position[1];
		atomList->position[2] =  tmpAtomList->position[2];
		atomList->type = tmpAtomList->type;
		atomList->eps = tmpAtomList->eps;

		atomList++;;
		tmpAtomList++;
	}
	for (i=0;i<mol->numberOfDifferentKindsOfAtoms;i++)
	{
		mol->numberOfAtomsOfEachType[i] = mol2Copy->numberOfAtomsOfEachType[i];
		mol->symbol[i] = g_strdup(mol2Copy->symbol[i]);
		mol->masse[i] = mol2Copy->masse[i];
	}

	return 0;
}
/************************************************************************************************************/
static gint AddHalfSpace(MolSymMolecule* mol, RotationReflectionOperator op)
{
	gint n;
	gint i;
	MolSymAtom *atomList;
	MolSymAtom *tmpAtomList;
	MolSymMolecule tmpMol;
	gint numberOfAtoms = 0;
	gchar** symbols = NULL;
	gdouble* masses = NULL;
	gint originalNumberOfAtoms = mol->numberOfAtoms;



	tmpMol.numberOfAtoms=2*mol->numberOfAtoms;
	tmpMol.listOfAtoms = (MolSymAtom*)g_malloc(tmpMol.numberOfAtoms*sizeof(MolSymAtom));
	tmpMol.symbol = (gchar**)g_malloc(tmpMol.numberOfAtoms*sizeof(gchar*));
	tmpMol.numberOfAtomsOfEachType = (gint*)g_malloc(tmpMol.numberOfAtoms*sizeof(gint));
	tmpMol.masse = (gdouble*)g_malloc(tmpMol.numberOfAtoms*sizeof(gdouble));

	symbols = (gchar**)g_malloc(tmpMol.numberOfAtoms*sizeof(gchar*));
	masses = (gdouble*)g_malloc(tmpMol.numberOfAtoms*sizeof(gdouble));

	if (tmpMol.listOfAtoms == NULL) return -1;

	atomList = mol->listOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;

	/* copy molecule */
	numberOfAtoms = 0;
	for (n=0;n<mol->numberOfAtoms;n++)
	{
		numberOfAtoms++;
		tmpAtomList->position[0] =  atomList->position[0];
		tmpAtomList->position[1] =  atomList->position[1];
		tmpAtomList->position[2] =  atomList->position[2];
		tmpAtomList->type = atomList->type;
		tmpAtomList->eps = atomList->eps;
		symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
		masses[numberOfAtoms-1] = mol->masse[atomList->type]; 

		atomList++;;
		tmpAtomList++;
	}

	atomList = mol->listOfAtoms;
	for (n=0;n<mol->numberOfAtoms;n++)
	{
		switch(op)
		{
			case XY_PLANE:
			tmpAtomList->position[0] =  atomList->position[0];
			tmpAtomList->position[1] =  atomList->position[1];
			tmpAtomList->position[2] = -atomList->position[2];
			numberOfAtoms++;
			symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
			masses[numberOfAtoms-1] = mol->masse[atomList->type]; 
			tmpAtomList->type = atomList->type;
			tmpAtomList->eps = atomList->eps;
			atomList++;;
			tmpAtomList++;
			break;

			case XZ_PLANE:
			tmpAtomList->position[0] =  atomList->position[0];
			tmpAtomList->position[1] = -atomList->position[1];
			tmpAtomList->position[2] =  atomList->position[2];
			numberOfAtoms++;
			symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
			masses[numberOfAtoms-1] = mol->masse[atomList->type]; 
			tmpAtomList->type = atomList->type;
			tmpAtomList->eps = atomList->eps;
			atomList++;;
			tmpAtomList++;
			break;

			case YZ_PLANE:
			tmpAtomList->position[0] = -atomList->position[0];
			tmpAtomList->position[1] = atomList->position[1];
			tmpAtomList->position[2] = atomList->position[2];
			numberOfAtoms++;
			symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
			masses[numberOfAtoms-1] = mol->masse[atomList->type]; 
			tmpAtomList->type = atomList->type;
			tmpAtomList->eps = atomList->eps;
			atomList++;;
			tmpAtomList++;
			break;

			case POINT_INV:
			tmpAtomList->position[0] = -atomList->position[0];
			tmpAtomList->position[1] = -atomList->position[1];
			tmpAtomList->position[2] = -atomList->position[2];
			numberOfAtoms++;
			symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
			masses[numberOfAtoms-1] = mol->masse[atomList->type]; 
			tmpAtomList->type = atomList->type;
			tmpAtomList->eps = atomList->eps;
			atomList++;;
			tmpAtomList++;
			break;

			case ROT2X:
			tmpAtomList->position[0] =  atomList->position[0];
			tmpAtomList->position[1] = -atomList->position[1];
			tmpAtomList->position[2] = -atomList->position[2];
			numberOfAtoms++;
			symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
			masses[numberOfAtoms-1] = mol->masse[atomList->type]; 
			tmpAtomList->type = atomList->type;
			tmpAtomList->eps = atomList->eps;
			atomList++;;
			tmpAtomList++;
			break;

			case ROT2Y:
			tmpAtomList->position[0] = -atomList->position[0];
			tmpAtomList->position[1] =  atomList->position[1];
			tmpAtomList->position[2] = -atomList->position[2];
			numberOfAtoms++;
			symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
			masses[numberOfAtoms-1] = mol->masse[atomList->type]; 
			tmpAtomList->type = atomList->type;
			tmpAtomList->eps = atomList->eps;
			atomList++;;
			tmpAtomList++;
			break;

			case ROT2Z:
			tmpAtomList->position[0] = -atomList->position[0];
			tmpAtomList->position[1] = -atomList->position[1];
			tmpAtomList->position[2] =  atomList->position[2];
			numberOfAtoms++;
			symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
			masses[numberOfAtoms-1] = mol->masse[atomList->type]; 
			tmpAtomList->type = atomList->type;
			tmpAtomList->eps = atomList->eps;
			atomList++;;
			tmpAtomList++;
		}
	 }
	tmpMol.numberOfAtoms = numberOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;
      	tmpMol.numberOfDifferentKindsOfAtoms = 0;
	for(n=0; n<tmpMol.numberOfAtoms; n++)
	{
		i = 0;
      		for (i=0;i<tmpMol.numberOfDifferentKindsOfAtoms;i++) /* search symbol */
		{
	  		if ((strcmp(symbols[n],tmpMol.symbol[i]))==0)
	    		{
	      			tmpMol.numberOfAtomsOfEachType[i]++;
	      			break;      
	    		}
		}
      		if (i==tmpMol.numberOfDifferentKindsOfAtoms)
		{
			tmpMol.symbol[i] = g_strdup(symbols[n]);
			tmpMol.masse[i] = masses[n];
	  		tmpMol.numberOfAtomsOfEachType[i] = 1;
	  		tmpMol.numberOfDifferentKindsOfAtoms++;
		}
		tmpAtomList->type = i;
		tmpAtomList++;
	}

	/* printf("Avant remove\n"); printAMolecule(&tmpMol);*/
	removeEquivAtoms(&tmpMol);
	/* printf("Apres remove\n"); printAMolecule(&tmpMol);*/
	copyAMolecule(mol, &tmpMol);
	freeAMolecule(&tmpMol);

	if(symbols)
	{
		for(i=0; i<originalNumberOfAtoms*2; i++)
			if(symbols[i]) g_free(symbols[i]);
	       	g_free(symbols);
	}
	if(masses) g_free(masses);
 
	return 0;
}
/************************************************************************************************************/
static gint AddRotnZ(MolSymMolecule* mol, gint nax)
{
	gint n;
	gint i;
	MolSymAtom *atomList;
	MolSymAtom *tmpAtomList;
	MolSymMolecule tmpMol;
	gint numberOfAtoms = 0;
	gchar** symbols = NULL;
	gdouble* masses = NULL;
	gint originalNumberOfAtoms = mol->numberOfAtoms;

	if(nax<2) return 0;

	tmpMol.numberOfAtoms=nax*mol->numberOfAtoms;
	tmpMol.listOfAtoms = (MolSymAtom*)g_malloc(tmpMol.numberOfAtoms*sizeof(MolSymAtom));
	tmpMol.symbol = (gchar**)g_malloc(tmpMol.numberOfAtoms*sizeof(gchar*));
	tmpMol.numberOfAtomsOfEachType = (gint*)g_malloc(tmpMol.numberOfAtoms*sizeof(gint));
	tmpMol.masse = (gdouble*)g_malloc(tmpMol.numberOfAtoms*sizeof(gdouble));

	symbols = (gchar**)g_malloc(tmpMol.numberOfAtoms*sizeof(gchar*));
	masses = (gdouble*)g_malloc(tmpMol.numberOfAtoms*sizeof(gdouble));

	if (tmpMol.listOfAtoms == NULL) return -1;

	atomList = mol->listOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;

	/* copy molecule */
	numberOfAtoms = 0;
	for (n=0;n<mol->numberOfAtoms;n++)
	{
		numberOfAtoms++;
		tmpAtomList->position[0] =  atomList->position[0];
		tmpAtomList->position[1] =  atomList->position[1];
		tmpAtomList->position[2] =  atomList->position[2];
		tmpAtomList->type = atomList->type;
		tmpAtomList->eps = atomList->eps;
		symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
		masses[numberOfAtoms-1] = mol->masse[atomList->type]; 

		atomList++;;
		tmpAtomList++;
	}

	atomList = mol->listOfAtoms;
	for (n=0;n<mol->numberOfAtoms;n++)
	{
		gint k;
		gdouble cr,sr,x,y,rr;
		rr = atomList->position[0]*atomList->position[0] + atomList->position[1]*atomList->position[1];
		if (rr >  atomList->eps*atomList->eps)
		for(k=1;k<nax;k++)
		{
			cr = cos(2*k*M_PI/nax);
			sr = sin(2*k*M_PI/nax);
			x = atomList->position[0]*cr-atomList->position[1]*sr;
			y = atomList->position[0]*sr+atomList->position[1]*cr;

			tmpAtomList->position[0] =  x;
			tmpAtomList->position[1] =  y;
			tmpAtomList->position[2] = atomList->position[2];
			numberOfAtoms++;
			symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
			masses[numberOfAtoms-1] = mol->masse[atomList->type]; 
			tmpAtomList->type = atomList->type;
			tmpAtomList->eps = atomList->eps;
			tmpAtomList++;
			printf("n=%d k=%d\n",n,k);
		}
		atomList++;;
	 }
	printf("numberofAtoms after rotation = %d\n",numberOfAtoms);
	tmpMol.numberOfAtoms = numberOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;
      	tmpMol.numberOfDifferentKindsOfAtoms = 0;
	for(n=0; n<tmpMol.numberOfAtoms; n++)
	{
		i = 0;
      		for (i=0;i<tmpMol.numberOfDifferentKindsOfAtoms;i++) /* search symbol */
		{
	  		if ((strcmp(symbols[n],tmpMol.symbol[i]))==0)
	    		{
	      			tmpMol.numberOfAtomsOfEachType[i]++;
	      			break;      
	    		}
		}
      		if (i==tmpMol.numberOfDifferentKindsOfAtoms)
		{
			tmpMol.symbol[i] = g_strdup(symbols[n]);
			tmpMol.masse[i] = masses[n];
	  		tmpMol.numberOfAtomsOfEachType[i] = 1;
	  		tmpMol.numberOfDifferentKindsOfAtoms++;
		}
		tmpAtomList->type = i;
		tmpAtomList++;
	}

	printf("Avant remove\n"); printAMolecule(&tmpMol);
	removeEquivAtoms(&tmpMol);
	printf("Apres remove\n"); printAMolecule(&tmpMol);
	copyAMolecule(mol, &tmpMol);
	printf("End copy molecule\n");
	freeAMolecule(&tmpMol);
	printf("End freeMolecule\n");

	if(symbols)
	{
		for(i=0; i<originalNumberOfAtoms*nax; i++)
			if(symbols[i]) g_free(symbols[i]);
	       	g_free(symbols);
	}
	printf("End symbols\n");
	if(masses) g_free(masses);
 
	return 0;
}
/************************************************************************************************************/
static gint AddHalfSpaceMolecule(MolSymMolecule* mol, gdouble x, gdouble y, gdouble z)
{
	gint n;
	gint i;
	MolSymAtom *atomList;
	MolSymAtom *tmpAtomList;
	MolSymMolecule tmpMol;
	gint numberOfAtoms = 0;
	gchar** symbols = NULL;
	gdouble* masses = NULL;
	gint originalNumberOfAtoms = mol->numberOfAtoms;
	gdouble point[] = {0,0,0};
	gdouble normal[] = {x,y,z};

	tmpMol.numberOfAtoms=2*mol->numberOfAtoms;
	tmpMol.listOfAtoms = (MolSymAtom*)g_malloc(tmpMol.numberOfAtoms*sizeof(MolSymAtom));
	tmpMol.symbol = (gchar**)g_malloc(tmpMol.numberOfAtoms*sizeof(gchar*));
	tmpMol.numberOfAtomsOfEachType = (gint*)g_malloc(tmpMol.numberOfAtoms*sizeof(gint));
	tmpMol.masse = (gdouble*)g_malloc(tmpMol.numberOfAtoms*sizeof(gdouble));

	symbols = (gchar**)g_malloc(tmpMol.numberOfAtoms*sizeof(gchar*));
	masses = (gdouble*)g_malloc(tmpMol.numberOfAtoms*sizeof(gdouble));

	if (tmpMol.listOfAtoms == NULL) return -1;

	atomList = mol->listOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;

	/* copy molecule */
	numberOfAtoms = 0;
	for (atomList = mol->listOfAtoms,n=0;n<mol->numberOfAtoms;n++,atomList++)
	{
		numberOfAtoms++;
		tmpAtomList->position[0] =  atomList->position[0];
		tmpAtomList->position[1] =  atomList->position[1];
		tmpAtomList->position[2] =  atomList->position[2];
		tmpAtomList->type = atomList->type;
		tmpAtomList->eps = atomList->eps;
		symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
		masses[numberOfAtoms-1] = mol->masse[atomList->type]; 

		tmpAtomList++;
	}

	for (atomList = mol->listOfAtoms,n=0;n<mol->numberOfAtoms;n++,atomList++)
	{
		applyReflectionOneAtom(point, normal,  atomList->position, tmpAtomList->position);
		numberOfAtoms++;
		symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
		masses[numberOfAtoms-1] = mol->masse[atomList->type]; 
		tmpAtomList->type = atomList->type;
		tmpAtomList->eps = atomList->eps;
		tmpAtomList++;
	}
	tmpMol.numberOfAtoms = numberOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;
      	tmpMol.numberOfDifferentKindsOfAtoms = 0;
	for(n=0; n<tmpMol.numberOfAtoms; n++)
	{
		i = 0;
      		for (i=0;i<tmpMol.numberOfDifferentKindsOfAtoms;i++) /* search symbol */
		{
	  		if ((strcmp(symbols[n],tmpMol.symbol[i]))==0)
	    		{
	      			tmpMol.numberOfAtomsOfEachType[i]++;
	      			break;      
	    		}
		}
      		if (i==tmpMol.numberOfDifferentKindsOfAtoms)
		{
			tmpMol.symbol[i] = g_strdup(symbols[n]);
			tmpMol.masse[i] = masses[n];
	  		tmpMol.numberOfAtomsOfEachType[i] = 1;
	  		tmpMol.numberOfDifferentKindsOfAtoms++;
		}
		tmpAtomList->type = i;
		tmpAtomList++;
	}

	/* printf("Avant remove\n"); printAMolecule(&tmpMol);*/
	removeEquivAtoms(&tmpMol);
	/* printf("Apres remove\n"); printAMolecule(&tmpMol);*/
	copyAMolecule(mol, &tmpMol);
	freeAMolecule(&tmpMol);

	if(symbols)
	{
		for(i=0; i<originalNumberOfAtoms*2; i++)
			if(symbols[i]) g_free(symbols[i]);
	       	g_free(symbols);
	}
	if(masses) g_free(masses);
 
	return 0;
}
/************************************************************************************************************/
static gint AddProperRotationMolecule(MolSymMolecule* mol, gint nax, gdouble x, gdouble y, gdouble z)
{
	gint n;
	gint i;
	MolSymAtom *atomList;
	MolSymAtom *tmpAtomList;
	MolSymMolecule tmpMol;
	gint numberOfAtoms = 0;
	gchar** symbols = NULL;
	gdouble* masses = NULL;
	gint originalNumberOfAtoms = mol->numberOfAtoms;
	gdouble point[] = {0,0,0};
	gdouble axis[] = {x,y,z};
	gint k;
	gdouble angle;

	tmpMol.numberOfAtoms=nax*mol->numberOfAtoms;
	tmpMol.listOfAtoms = (MolSymAtom*)g_malloc(tmpMol.numberOfAtoms*sizeof(MolSymAtom));
	tmpMol.symbol = (gchar**)g_malloc(tmpMol.numberOfAtoms*sizeof(gchar*));
	tmpMol.numberOfAtomsOfEachType = (gint*)g_malloc(tmpMol.numberOfAtoms*sizeof(gint));
	tmpMol.masse = (gdouble*)g_malloc(tmpMol.numberOfAtoms*sizeof(gdouble));

	symbols = (gchar**)g_malloc(tmpMol.numberOfAtoms*sizeof(gchar*));
	masses = (gdouble*)g_malloc(tmpMol.numberOfAtoms*sizeof(gdouble));

	if (tmpMol.listOfAtoms == NULL) return -1;

	atomList = mol->listOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;

	/* copy molecule */
	numberOfAtoms = 0;
	for (n=0;n<mol->numberOfAtoms;n++)
	{
		numberOfAtoms++;
		tmpAtomList->position[0] =  atomList->position[0];
		tmpAtomList->position[1] =  atomList->position[1];
		tmpAtomList->position[2] =  atomList->position[2];
		tmpAtomList->type = atomList->type;
		tmpAtomList->eps = atomList->eps;
		symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
		masses[numberOfAtoms-1] = mol->masse[atomList->type]; 

		atomList++;;
		tmpAtomList++;
	}

	for ( atomList = mol->listOfAtoms, n=0;n<mol->numberOfAtoms;n++,atomList++)
	{
		for(k=1;k<nax;k++)
		{
			angle = 2*k*M_PI/nax;
			applyProperRotationOneAtom(point, axis, angle, atomList->position, tmpAtomList->position);
			numberOfAtoms++;
			symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
			masses[numberOfAtoms-1] = mol->masse[atomList->type]; 
			tmpAtomList->type = atomList->type;
			tmpAtomList->eps = atomList->eps;
			tmpAtomList++;
		}
	 }
	tmpMol.numberOfAtoms = numberOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;
      	tmpMol.numberOfDifferentKindsOfAtoms = 0;
	for(n=0; n<tmpMol.numberOfAtoms; n++)
	{
		i = 0;
      		for (i=0;i<tmpMol.numberOfDifferentKindsOfAtoms;i++) /* search symbol */
		{
	  		if ((strcmp(symbols[n],tmpMol.symbol[i]))==0)
	    		{
	      			tmpMol.numberOfAtomsOfEachType[i]++;
	      			break;      
	    		}
		}
      		if (i==tmpMol.numberOfDifferentKindsOfAtoms)
		{
			tmpMol.symbol[i] = g_strdup(symbols[n]);
			tmpMol.masse[i] = masses[n];
	  		tmpMol.numberOfAtomsOfEachType[i] = 1;
	  		tmpMol.numberOfDifferentKindsOfAtoms++;
		}
		tmpAtomList->type = i;
		tmpAtomList++;
	}

	/* printf("Avant remove\n"); printAMolecule(&tmpMol);*/
	removeEquivAtoms(&tmpMol);
	/* printf("Apres remove\n"); printAMolecule(&tmpMol);*/
	copyAMolecule(mol, &tmpMol);
	freeAMolecule(&tmpMol);

	if(symbols)
	{
		for(i=0; i<originalNumberOfAtoms*nax; i++)
			if(symbols[i]) g_free(symbols[i]);
	       	g_free(symbols);
	}
	if(masses) g_free(masses);
 
	return 0;
}
/************************************************************************************************************/
/*
static gint AddImproperRotationMolecule(MolSymMolecule* mol, gint nax, gdouble x, gdouble y, gdouble z)
{
	gint n;
	gint i;
	MolSymAtom *atomList;
	MolSymAtom *tmpAtomList;
	MolSymMolecule tmpMol;
	gint numberOfAtoms = 0;
	gchar** symbols = NULL;
	gdouble* masses = NULL;
	gint originalNumberOfAtoms = mol->numberOfAtoms;
	gdouble point[] = {0,0,0};
	gdouble axis[] = {x,y,z};
	gint k;
	gdouble angle;

	tmpMol.numberOfAtoms=2*mol->numberOfAtoms;
	tmpMol.listOfAtoms = (MolSymAtom*)g_malloc(tmpMol.numberOfAtoms*sizeof(MolSymAtom));
	tmpMol.symbol = (gchar**)g_malloc(tmpMol.numberOfAtoms*sizeof(gchar*));
	tmpMol.numberOfAtomsOfEachType = (gint*)g_malloc(tmpMol.numberOfAtoms*sizeof(gint));
	tmpMol.masse = (gdouble*)g_malloc(tmpMol.numberOfAtoms*sizeof(gdouble));

	symbols = (gchar**)g_malloc(tmpMol.numberOfAtoms*sizeof(gchar*));
	masses = (gdouble*)g_malloc(tmpMol.numberOfAtoms*sizeof(gdouble));

	if (tmpMol.listOfAtoms == NULL) return -1;

	atomList = mol->listOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;

	// copy molecule 
	numberOfAtoms = 0;
	for (n=0;n<mol->numberOfAtoms;n++)
	{
		numberOfAtoms++;
		tmpAtomList->position[0] =  atomList->position[0];
		tmpAtomList->position[1] =  atomList->position[1];
		tmpAtomList->position[2] =  atomList->position[2];
		tmpAtomList->type = atomList->type;
		tmpAtomList->eps = atomList->eps;
		symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
		masses[numberOfAtoms-1] = mol->masse[atomList->type]; 

		atomList++;;
		tmpAtomList++;
	}

	for ( atomList = mol->listOfAtoms, n=0;n<mol->numberOfAtoms;n++,atomList++)
	{
		for(k=1;k<nax;k++)
		{
			angle = 2*k*M_PI/nax;
			applyImproperRotationOneAtom(point, axis, angle, atomList->position, tmpAtomList->position);
			numberOfAtoms++;
			symbols[numberOfAtoms-1] = g_strdup(mol->symbol[atomList->type]); 
			masses[numberOfAtoms-1] = mol->masse[atomList->type]; 
			tmpAtomList->type = atomList->type;
			tmpAtomList->eps = atomList->eps;
			tmpAtomList++;
		}
	 }
	tmpMol.numberOfAtoms = numberOfAtoms;
	tmpAtomList = tmpMol.listOfAtoms;
      	tmpMol.numberOfDifferentKindsOfAtoms = 0;
	for(n=0; n<tmpMol.numberOfAtoms; n++)
	{
		i = 0;
      		for (i=0;i<tmpMol.numberOfDifferentKindsOfAtoms;i++) // search symbol 
		{
	  		if ((strcmp(symbols[n],tmpMol.symbol[i]))==0)
	    		{
	      			tmpMol.numberOfAtomsOfEachType[i]++;
	      			break;      
	    		}
		}
      		if (i==tmpMol.numberOfDifferentKindsOfAtoms)
		{
			tmpMol.symbol[i] = g_strdup(symbols[n]);
			tmpMol.masse[i] = masses[n];
	  		tmpMol.numberOfAtomsOfEachType[i] = 1;
	  		tmpMol.numberOfDifferentKindsOfAtoms++;
		}
		tmpAtomList->type = i;
		tmpAtomList++;
	}

	// printf("Avant remove\n"); printAMolecule(&tmpMol);
	removeEquivAtoms(&tmpMol);
	// printf("Apres remove\n"); printAMolecule(&tmpMol);
	copyAMolecule(mol, &tmpMol);
	freeAMolecule(&tmpMol);

	if(symbols)
	{
		for(i=0; i<originalNumberOfAtoms*2; i++)
			if(symbols[i]) g_free(symbols[i]);
	       	g_free(symbols);
	}
	if(masses) g_free(masses);
 
	return 0;
}
*/
/************************************************************************************************************/
void generateMoleculeFromTheBasisSetOfAtoms(MolSymMolecule* mol, gint numberOfElements, gchar** elements)
{
	gint i;
	if(numberOfElements<2) return;
	for(i=1 ; i<numberOfElements ; i++)
	{
		if(!strcmp(elements[i],"C2z")) AddHalfSpace(mol, ROT2Z);
		else if(!strcmp(elements[i],"C2y")) AddHalfSpace(mol, ROT2Y);
		else if(!strcmp(elements[i],"C2x")) AddHalfSpace(mol, ROT2X);
		else if(!strcmp(elements[i],"i")) AddHalfSpace(mol, POINT_INV);
		else if(!strcmp(elements[i],"Sigma")) AddHalfSpace(mol, XY_PLANE);
		else if(!strcmp(elements[i],"Sigma v")) AddHalfSpace(mol, XZ_PLANE);
		else if(!strcmp(elements[i],"Sigma' v")) AddHalfSpace(mol, YZ_PLANE);
		else if(!strcmp(elements[i],"Sigma h")) AddHalfSpace(mol, XY_PLANE);
		else if(!strcmp(elements[i],"Sigma xy")) AddHalfSpace(mol, XY_PLANE);
		else if(!strcmp(elements[i],"Sigma xz")) AddHalfSpace(mol, XZ_PLANE);
		else if(!strcmp(elements[i],"Sigma yz")) AddHalfSpace(mol, YZ_PLANE);
		else if(strstr(elements[i],"C") && strstr(elements[i],"z") && strlen(elements[i])>=3)
		{
			gint nax = atoi(&elements[i][1]);
			printf("nax = %d\n",nax);
			AddRotnZ(mol, nax);
			printf("end AddRotnZ\n");
		}
	}
}
/************************************************************************************************************/
/* tetraedal symmetry */
void generateForTetraedalSymmetry(MolSymMolecule *mol,gint sym, gdouble axes[3][3])
{
	AddProperRotationMolecule(mol, 3, sqrt(8.)/3., 0.,-1./3.);
	AddProperRotationMolecule(mol, 3, 0, 0,1);

	if (sym & SYM_D) AddHalfSpaceMolecule(mol, -0.866025404, 0.5, 0.0);

}    
/************************************************************************************************************/
/* octaedral symmetry */
void generateForOctaedralSymmetry(MolSymMolecule *mol,gint sym, gdouble axes[3][3])
{
	rotationAroundZAxisByPi(mol);

	AddHalfSpaceMolecule(mol, 0.0, -1.0, 0.0);
	AddHalfSpaceMolecule(mol, 0.577350269, 0.0, -0.816496581);
	AddHalfSpaceMolecule(mol, -0.816496581, 0.0, -0.577350269); 

}    
/************************************************************************************************************/
/* icosaedral symmetry */
void generateForIcosaedralSymmetry(MolSymMolecule *mol,gint sym, gdouble axes[3][3])
{
	AddProperRotationMolecule(mol, 5, C5X, 0, C5Z);
	AddHalfSpace(mol, POINT_INV);
	AddProperRotationMolecule(mol, 3, 0, 0, 1.0);
	AddHalfSpace(mol, POINT_INV);
	/*
	AddHalfSpaceMolecule(mol, -0.467086179481, -0.809016994375, +0.356822089773); 
	AddHalfSpaceMolecule(mol, 0.0, +1.0, 0.0);
	*/

}    

