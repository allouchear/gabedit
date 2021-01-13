/* MoleculeSymmetry.c */
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
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <glib/gi18n.h>
#include "../Symmetry/MoleculeSymmetryType.h"
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/PrincipalAxis.h"
#include "../Symmetry/PolyHedralOperators.h"
#include "../Symmetry/SymmetryOperators.h"
#include "../Symmetry/ReduceMolecule.h"
#include "../Symmetry/GenerateMolecule.h"
#include "../Common/GabeditType.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Constants.h"
#include "../Utils/Utils.h"
#include "../Symmetry/SymmetryGabedit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define SQU(x,y,z) ((x)*(x) + (y)*(y) + (z)*(z))
#define  C5X  0.6070619987
#define  C5Z  0.7946544719

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
/*
static void printMolSymMolecules(MolSymMolecule* mol)
{
	gint i;
	MolSymAtom *atomList;
	gint deleted = mol->numberOfDifferentKindsOfAtoms;     // tag for deleted atom 
 
	atomList = mol->listOfAtoms;
	
	for (i=0;i<mol->numberOfAtoms;i++)
	 {
	        gint j = mol->listOfAtoms[i].type;
		if(j!=deleted)
		printf("%s %d %f %f %f\n",mol->symbol[j],atomList->type,
			atomList->position[0],atomList->position[1], atomList->position[2]);
		else
		printf("%s %d %f %f %f\n","DE",atomList->type,
			atomList->position[0],atomList->position[1], atomList->position[2]);
		atomList++;
	 }
}
*/
/************************************************************************************************************/
static gdouble getSmallestDistanceBetweenAtoms(MolSymMolecule* mol)
{
	gint i,j;
	gdouble min_dd,max_dd,dd;
	MolSymAtom *atomList;
	MolSymAtom *tmpAtomList;
 
	min_dd = 1.E99;
	max_dd = 0.0;
	atomList = mol->listOfAtoms;
	
	for (i=0;i<mol->numberOfAtoms;i++)
	 {
		tmpAtomList = atomList + 1;
		for (j=i+1;j<mol->numberOfAtoms;j++)
		{
			dd = SQU(atomList->position[0] - tmpAtomList->position[0], atomList->position[1] - tmpAtomList->position[1],
			atomList->position[2] - tmpAtomList->position[2]);
			if (dd < min_dd) min_dd = dd;
			if (dd > max_dd) max_dd = dd;
			tmpAtomList++;
		}
		atomList++;
	 }
	if ((max_dd > 0.0) && (min_dd/max_dd < 1.E-6))
		printf(_("Warning: Two atoms on same position\n"));
	if(mol->numberOfAtoms<2) min_dd = 0.1;
	return sqrt(min_dd);
}
/************************************************************************************************************/
void setGeneratorsAbelianGroup(gchar* groupName,
		gint* nGenerators, gchar** generators, 
		gint* nMolcas, gchar** molcasGenerators, 
		gint* nElements, gchar** elements
		)
{

	gchar group[BSIZE];
	sprintf(group,"%s",groupName);
	uppercase(group);

	if(!strcmp(group,"D2H"))
	{
		*nGenerators = 3;
		sprintf(generators[0],"C2z");
		sprintf(generators[1],"C2y");
		sprintf(generators[2],"i");
			
		*nMolcas = 3;
		sprintf(molcasGenerators[0],"xy");
		sprintf(molcasGenerators[1],"xz");
		sprintf(molcasGenerators[2],"xyz");

		*nElements = 8;
		sprintf(elements[0],"E");
		sprintf(elements[1],"C2z");
		sprintf(elements[2],"C2y");
		sprintf(elements[3],"C2x");
		sprintf(elements[4],"i");
		sprintf(elements[5],"Sigma xy");
		sprintf(elements[6],"Sigma xz");
		sprintf(elements[7],"Sigma yz");
	}
	else if(!strcmp(group,"C2H"))
	{
		*nGenerators = 2;
		sprintf(generators[0],"C2z");
		sprintf(generators[1],"i");

		*nMolcas = 2;
		sprintf(molcasGenerators[0],"xy");
		sprintf(molcasGenerators[1],"xyz");

		*nElements = 4;
		sprintf(elements[0],"E");
		sprintf(elements[1],"C2z");
		sprintf(elements[2],"i");
		sprintf(elements[3],"Sigma h");

	}
	else if(!strcmp(group,"CI"))
	{
		*nGenerators = 1;
		sprintf(generators[0],"i");

		*nMolcas = 1;
		sprintf(molcasGenerators[0],"xyz");

		*nElements = 2;
		sprintf(elements[0],"E");
		sprintf(elements[1],"i");
	}
	else if(!strcmp(group,"C2V"))
	{
		*nGenerators = 2;
		sprintf(generators[0],"C2z");
		sprintf(generators[1],"Sigma v");

		*nMolcas = 2;
		sprintf(molcasGenerators[0],"xy");
		sprintf(molcasGenerators[1],"y");

		*nElements = 4;
		sprintf(elements[0],"E");
		sprintf(elements[1],"C2z");
		sprintf(elements[2],"Sigma v");
		sprintf(elements[3],"Sigma' v");
	}
	else if(!strcmp(group,"D2"))
	{
		*nGenerators = 2;
		sprintf(generators[0],"C2z");
		sprintf(generators[1],"C2y");

		*nMolcas = 2;
		sprintf(molcasGenerators[0],"xy");
		sprintf(molcasGenerators[1],"xz");

		*nElements = 4;
		sprintf(elements[0],"E");
		sprintf(elements[1],"C2z");
		sprintf(elements[2],"C2y");
		sprintf(elements[3],"C2z");
	}
	else if(!strcmp(group,"CS"))
	{
		*nGenerators = 1;
		sprintf(generators[0],"Sigma");

		*nMolcas = 1;
		sprintf(molcasGenerators[0],"x");

		*nElements = 2;
		sprintf(elements[0],"E");
		sprintf(elements[1],"Sigma");
	}
	else if(!strcmp(group,"C2"))
	{
		*nGenerators = 1;
		sprintf(generators[0],"C2z");

		*nMolcas = 1;
		sprintf(molcasGenerators[0],"xy");

		*nElements = 2;
		sprintf(elements[0],"E");
		sprintf(elements[1],"C2z");
	}
	else
	{
		*nGenerators = 0;
		*nMolcas = 0;
		*nElements = 0;
	}
}
/************************************************************************************************************/
static void getAbelianGroupSymbol(MolSymMolecule* mol, gchar* groupName)
{
	gint nax = 0;
	gint iz = 0;
	gint iy = 0;
	gint ix = 0;
	gint ixy = 0;
	gint ixz = 0;
	gint ixyz = 0;

	nax = determineOrderOfZAxis(mol, 12);

	ixy = testRotationReflection(mol,XY_PLANE);
	ixz = testRotationReflection(mol,XZ_PLANE);
	ixyz =  testRotationReflection(mol,POINT_INV);
	ix =  testRotationReflection(mol,ROT2X);
	iy =  testRotationReflection(mol,ROT2X);

	if(nax%2==0) iz = 1;
	if(ixyz)
	{
		if(iz && iy) sprintf(groupName,"D2h");
		else if(iz) sprintf(groupName,"C2h");
		else sprintf(groupName,"Ci");
	}
	else if(ixz && iz ) sprintf(groupName,"C2v");
	else if( iz && iy ) sprintf(groupName,"D2");
	else if( ixy ) sprintf(groupName,"Cs");
	else if( iz ) sprintf(groupName,"C2");
	else sprintf(groupName,"C1");
}
/************************************************************************************************************/
static void getAbelianGroup(MolSymMolecule* mol, 
		gchar* groupName,
		gint* nGenerators, 
		gchar** generators, 
		gint* nMolcas, 
		gchar** molcasGenerators, 
		gint* nElements, 
		gchar** elements, 
		gchar* error)
{
	getAbelianGroupSymbol(mol, groupName);
	setGeneratorsAbelianGroup(groupName, nGenerators, generators, nMolcas, molcasGenerators, nElements, elements);
}
/************************************************************************************************************/
static void getPointGroupSymbol(gint sym, gint nax, gchar* pgsym)
{

	*pgsym = 0;
	if (sym==0 && nax>0) sprintf(pgsym,"C%d",nax);
	else if (sym & SYM_R) strcat(pgsym,"R3");
	else if (sym & SYM_T) 
	{
		strcat(pgsym,"T");
		if (sym & SYM_D) strcat(pgsym,"d");
		if (sym & SYM_I) strcat(pgsym,"h");
	}
	else if (sym & SYM_O) 
	{
		strcat(pgsym,"O");
		if (sym & SYM_D) strcat(pgsym,"h");
	}
	else if (sym & SYM_IC)
	{ 
		strcat(pgsym,"I");
		if (sym & SYM_D) strcat(pgsym,"h");
	}
	else if (sym & SYM_S)
	{
		sprintf(pgsym,"S%d",nax);
		if (sym & SYM_V) 
		{
			if(nax==4) sprintf(pgsym,"S%d",2*nax);
			else sprintf(pgsym,"D%dd",nax);
		}
	}
	else 
	{
		if (sym & SYM_D) strcat(pgsym,"D");
		else strcat(pgsym,"C");

		if (nax == 0) strcat(pgsym,"inf");
		else if (nax == 1)
			if (sym & SYM_I) strcat(pgsym,"i");
			else if (sym & SYM_H) strcat(pgsym,"s");
				else strcat(pgsym,"1");
		else sprintf(pgsym+1,"%d",nax);

		if ((nax != 1) && (sym & SYM_H)) strcat(pgsym,"h");
		else if (sym & SYM_V) strcat(pgsym,"v");
	}
}

/************************************************************************************************************/
static gint determineSymmetry(MolSymMolecule* mol,int* nax, gint numberOfEquivalentAxes, gint maximalOrder, gdouble principalAxisTolerance, gchar* error)
{
	gint symmetry = 0;
	gint ret;

	switch(numberOfEquivalentAxes)
	{
		case -3:
		symmetry |= SYM_R;  /* spherical symmetry */
		break;

		case -2:
		symmetry |= SYM_V;
		if (testRotationReflection(mol,XY_PLANE)==1) symmetry |= SYM_H; /* linear molecule */
		break;

		case 1:
		qsort(mol->listOfAtoms,mol->numberOfAtoms,sizeof(MolSymAtom),compare2atoms);
		*nax = determineOrderOfZAxis(mol, maximalOrder);
		if (*nax == 0)   /* error */
		{
			sprintf(error,_("Nonlinear molecule with symmetry Cinf! Try smaller precision for atom position\n"
				"Current precision = %0.5f"),mol->listOfAtoms[0].eps);
			return 0;
		}
		if (*nax == 1)
		{
			if (testRotationReflection(mol,ROT2X)==1)
			{
				*nax = 2;
				rotateAroundYAxisToPlaceXAxisOntoZAxis(mol); /* may not work sometimes ! */
			}
			else if (testRotationReflection(mol,ROT2Y)==1)
			{
				*nax = 2;
				rotateAroundXAxisToPlaceYAxisOntoZAxis(mol); 
			}
			else if (testRotationReflection(mol,XY_PLANE))
			{
				symmetry |= SYM_H;    /* symmetry Cs */
				break;
			}
			else if (testRotationReflection(mol,XZ_PLANE))
			{
				rotateAroundXAxisToPlaceYAxisOntoZAxis(mol); 
				symmetry |= SYM_H;    /* symmetry Cs */
				break;
			}
			else if (testRotationReflection(mol,YZ_PLANE))
			{
				rotateAroundYAxisToPlaceXAxisOntoZAxis(mol); 
				symmetry |= SYM_H;    /* symmetry Cs */
				break;
			}
			else if  (testRotationReflection(mol,POINT_INV))
			{
				symmetry |= SYM_I;    /* symmetry Ci */
				break;
			}
		}
		if (*nax > 1)
		{
			if (testRotationReflection(mol,XY_PLANE))  symmetry |= SYM_H;
			ret = checkForVPlanesOrDAxis(mol,*nax, error);
			if (ret == 1) symmetry |= SYM_V;
			if (ret == 2) symmetry |= SYM_D;	    
		}
		break;
		case 2:
		qsort(mol->listOfAtoms,mol->numberOfAtoms,sizeof(MolSymAtom),compare2atoms);
		*nax = determineOrderOfZAxis(mol, maximalOrder);
		if (*nax == 0)   /* error */
		{
			sprintf(error,_("Nonlinear molecule with symmetry Cinf! Try smaller precision for atom position\n"
				"Current precision = %0.5f"),mol->listOfAtoms[0].eps);
			return 0;
		}
		if (*nax == 1)
		{
			if (testRotationReflection(mol,ROT2Y)==1)
			{
				rotateAroundXAxisToPlaceYAxisOntoZAxis(mol); 
				*nax = determineOrderOfZAxis(mol, maximalOrder);
			}
		}
		if (rotateAroundZaxes(mol,-2* *nax))
		{
			*nax *=2;   /* improper rotation axis */
			symmetry |= SYM_S;
		}
		else if (testRotationReflection(mol,XY_PLANE))
			symmetry |= SYM_H;
		ret = checkForVPlanesOrDAxis(mol,*nax, error);
		/* printf("ret = %d\n",ret);*/
		if (ret == 1) symmetry |= SYM_V;
		if (ret == 2) symmetry |= SYM_D;	    
		if (testRotationReflection(mol,POINT_INV) ==1) symmetry |= SYM_I;
		if ((*nax == 1) && (symmetry & SYM_V))
		{
			rotateAroundXAxisToPlaceYAxisOntoZAxis(mol);	
			symmetry |= SYM_H;
			symmetry &=~SYM_V;
		} 
		break;
		case 3:
		*nax = searchForC3Axes(mol, error);
		if (*nax<3)
		{
			sprintf(error,_("Could not find symmetry axis (see tolerance parameters)\n"
				"Current precision for position of atoms = %0.5f\n"
				"Current tolerance for principal axis = %0.5f"),mol->listOfAtoms[0].eps,principalAxisTolerance);
			return 0;
		}
      
		if (rotateAroundArbitraryAxes(mol,3, sqrt(8.)/3., 0.,-1./3., error) == 1) symmetry = SYM_T;
		else if (rotateAroundArbitraryAxes(mol,5,C5X,0.,C5Z, error) ==1) symmetry = SYM_IC;
		else
		{
			rotationAroundZAxisByPi(mol);
			if (rotateAroundArbitraryAxes(mol,3, sqrt(8.)/3., 0.,-1./3., error) == 1) symmetry = SYM_T;
			else if (rotateAroundArbitraryAxes(mol,5,C5X,0.,C5Z, error) ==1) symmetry = SYM_IC;
			else
			{
				sprintf(error,_("Irregular 3-fold axis detected\n"));
				return 0;
			}
		}
		if (symmetry == SYM_T)
		{
			if (rotateAroundArbitraryAxes(mol,4,sqrt(2./3.), 0.0, sqrt(1./3.), error) == 1)  
				symmetry = SYM_O;      /* CUBIC symmetry */
			else if (testRotationReflection(mol,POINT_INV) == 1)
				symmetry |= SYM_I;     /* Th symmetry */
		}
		if (testRotationReflection(mol,XZ_PLANE) == 1) symmetry |= SYM_D;
		*nax = 3;
		break;
	}
	if (*nax == 0) symmetry |= SYM_V;
	if ((symmetry & (SYM_S | SYM_V)) == (SYM_S | SYM_V))  *nax /= 2;
	if ((symmetry & (SYM_H | SYM_V)) == (SYM_H | SYM_V)) symmetry |= SYM_D; 
	if (((symmetry & SYM_H) == SYM_H) && ((*nax % 2) ==0)) symmetry |= SYM_I; 
	return symmetry;
}
/************************************************************************************************************/
static void getSymmetryFromSymbol(gchar* syml,int* sym,int* nax, gchar* error)
{
	gchar *s;
	gchar c1,c2;
	*nax = 1;
	*sym = 0;

	for(s=syml;*s != 0;s++) *s = toupper(*s);

	if (strcmp(syml,"CS") == 0) { *sym = SYM_H; return;}
	if (strcmp(syml,"CI") == 0) { *sym = SYM_I; return;}
	if (strcmp(syml,"R3") == 0) { *sym = SYM_R; return;}
	if (strcmp(syml,"DINFH") == 0) { *sym = SYM_H; *nax = 0; return;}
	if (strcmp(syml,"CINFV") == 0) { *nax = 0; return; }
	if (strcmp(syml,"D3D") == 0) { *nax = 3; *sym = (SYM_I | SYM_S | SYM_V); return; }
	c2 = 0;
	if (sscanf(syml,"%c%d%c",&c1,nax,&c2) < 2)
	{
		if (c1 == 'T')
		{
			*sym = SYM_T;
			if (syml[1] == 'H') *sym |= SYM_I;
			else if (syml[1] == 'D') *sym |= SYM_D;
			else *sym = SYM_U;
		}
		else if (c1 == 'O')
		{
			*sym = SYM_O;
			if (syml[1] == 'H') *sym |= SYM_D;
			else *sym = SYM_U;
		}
		else if (c1 == 'I')
		{
			*sym = SYM_I;
			if (syml[1] == 'H') *sym |= SYM_D;
			else *sym = SYM_U;
		}
		else *sym = SYM_U;
	}
	else if ((c1 == 'S') && (c2 == 0)) *sym = SYM_S;
	else if ((c1 == 'D') && (c2 == 0)) *sym = SYM_D;
	else if ((c1 == 'D') && (c2 == 'H')) *sym |= (SYM_D | SYM_H | SYM_V);
	else if ((c1 == 'D') && (c2 == 'D')) *sym |= (SYM_S | SYM_V);
	else if ((c1 == 'C') && (c2 == 'V')) *sym |= SYM_V;
	else if ((c1 == 'C') && (c2 == 'H')) *sym |= SYM_H;
	else if ((c1 == 'C') && (c2 == 0));
	else if ((c1 == 'S') && (c2 == 0))*sym |= SYM_H;
	else *sym = SYM_U;

	if ((c1 == 'D') && (c2 == 'H') && *nax%2==0) *sym |= SYM_I;

	if (*sym == SYM_U)
	{
		sprintf(error,_("Unknown symmetry - set to C1"));
		*sym = 0;
		*nax = 1;
	}
}
/************************************************************************************************************/
static gint setMolSymMolecule(MolSymMolecule* mol, gint nAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z, gdouble eps)
{
	gint i;
	gint n;
	MolSymAtom* atomList = NULL;

	mol->listOfAtoms = NULL;  
	mol->numberOfAtoms = 0;
	mol->numberOfDifferentKindsOfAtoms=0;
	mol->symbol = NULL;
	mol->numberOfAtomsOfEachType = NULL;

	mol->listOfAtoms = (MolSymAtom*)g_malloc(sizeof(MolSymAtom)*(nAtoms));
	if(mol->listOfAtoms==NULL) return -1;

	mol->numberOfAtoms = nAtoms;

	mol->symbol = (gchar**)g_malloc(sizeof(gchar*)*(mol->numberOfAtoms));
	if( mol->symbol == NULL) return -1;
	for(n=0; n<mol->numberOfAtoms; n++) mol->symbol[n] = NULL;

	mol->numberOfAtomsOfEachType = (gint*)g_malloc(sizeof(gint)*(mol->numberOfAtoms));
	if(mol->numberOfAtomsOfEachType == NULL) return -1;

	mol->masse = (gdouble*)g_malloc(sizeof(gdouble)*(mol->numberOfAtoms));
	if(mol->masse == NULL) return -1;


	atomList = mol->listOfAtoms;
	for(n=0; n<mol->numberOfAtoms; n++)
	{
		atomList->position[0]  = X[n];
		atomList->position[1]  = Y[n];
		atomList->position[2]  = Z[n];
		atomList->eps  = eps;
		i = 0;
      		for (i=0;i<mol->numberOfDifferentKindsOfAtoms;i++) /* search symbol */
		{
	  		if ((strcmp(symbols[n],mol->symbol[i]))==0)
	    		{
	      			mol->numberOfAtomsOfEachType[i]++;
	      			break;      
	    		}
		}
      		if (i==mol->numberOfDifferentKindsOfAtoms)
		{
			SAtomsProp prop = prop_atom_get(symbols[n]);
			mol->symbol[i] = g_strdup(symbols[n]);
			mol->masse[i] = prop.masse;
	  		mol->numberOfAtomsOfEachType[i] = 1;
	  		mol->numberOfDifferentKindsOfAtoms++;
		}
		atomList->type = i;
		atomList++;
	}
  	if (eps < 0.) eps = getSmallestDistanceBetweenAtoms(mol)/50.;  /* default */
	atomList = mol->listOfAtoms;
	for(n=0; n<mol->numberOfAtoms; n++)
	{
		atomList->eps  = eps;
		atomList++;
	}

	return 0;
}
/*
 INPUT 
gdouble principalAxisTolerance;	  	tolerance for principal axis classification
gboolean axz_3;  			 O and I symmetry: z-axis of C3 type ?
gint maximalOrder;	 		maximal order for z-axis
gboolean redu;    			reduction to symmetry base

INPUT OUPUT
gint* numberOfAtoms;   			number of atoms (modified if reduction)
gchar** symbols;			symbol of atoms;
gdouble *X,*Y,*Z;  			positions of atoms; (modified)
gint pointGroupSymbol;	  		if not NO ->  symmetry fixed manually (modified if pointGroupSymbol="NO")

OUPUT
gchar* message;      			Error Message 
*/
/************************************************************************************************************/
/*
 INPUT 
gdouble principalAxisTolerance;	  	tolerance for principal axis classification
gint maximalOrder;	 		maximal order for z-axis
gboolean redu;    			reduction to symmetry base

INPUT OUPUT
gint* numberOfAtoms;   			number of atoms (modified if reduction)
gchar** symbols;			symbol of atoms;
gdouble *X,*Y,*Z;  			positions of atoms; (modified)

OUPUT
gchar* message;      			Error Message 
gint pointGroupSymbol;	  		point group name
gint abelianPointGroupSymbol	  	abelian point group name
gint* nGenerators			numbuer of generators
gchar** generators			generators name
gint* nMolcas				numbuer of generators with Molcas style
gchar** molcasGenerators		generators name with Molcas style
gint* nElements				number of all elements
gchar** elements			name for all elements
*/
/************************************************************************************************************/
int computeAbelianGroup(
		gdouble principalAxisTolerance, gchar* pointGroupSymbol, gchar* abelianPointGroupSymbol,
		gint maximalOrder, gboolean redu,
	       	gint* numberOfAtoms, 
		gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z, 
		gint* nGenerators,
		gchar** generators,
		gint* nMolcas,
		gchar** molcasGenerators,
		gint* nElements,
		gchar** elements,
		gdouble* precision, gchar* message)
{
	gdouble eps = *precision;
	MolSymMolecule mol;
	gdouble centerOfGravity[3]; 
	gint numberOfEquivalentAxes;
	gdouble inertialMoment[3];
	gdouble axes[3][3];
	gint ret;
	gint nax = 0; /* order of z axis */
	gint symmetry;
	gint i;
	gint nAtoms = *numberOfAtoms;
	gchar buffer[BSIZE];

	sprintf(message,"C1 Group");

	sprintf(pointGroupSymbol,"C1");
	sprintf(abelianPointGroupSymbol,"C1");
	ret = setMolSymMolecule(&mol, nAtoms, symbols, X, Y, Z, eps);
	if (ret != 0)
	{

		sprintf(message,_("Not enough memory for molecule"));
		return ret;
	}

	determinePrincipalAxis(&mol,centerOfGravity,&numberOfEquivalentAxes,inertialMoment,axes, principalAxisTolerance,TRUE);  /* transform to principal axes */

	symmetry = determineSymmetry(&mol,&nax,numberOfEquivalentAxes, maximalOrder, principalAxisTolerance, message);
	
	if(symmetry == 0)
	{
		*precision = mol.listOfAtoms[0].eps;
		*nGenerators = 0;
	       	*nMolcas = 0;
		*nElements = 0;
		return -1;
	}
	getPointGroupSymbol(symmetry,nax, pointGroupSymbol);
	getAbelianGroup(&mol, abelianPointGroupSymbol, nGenerators, generators, nMolcas, molcasGenerators, nElements, elements, NULL);
	if (redu)
	{
		sprintf(buffer,"%s",abelianPointGroupSymbol);
		getSymmetryFromSymbol(buffer,&symmetry,&nax, message);
		reduceMoleculeToItsBasisSetOfAtoms(&mol,symmetry,nax);
	}
      	for (i=0;i < mol.numberOfAtoms;i++)
	{ 
		sprintf(symbols[i],"%s",mol.symbol[mol.listOfAtoms[i].type]);
		X[i] = mol.listOfAtoms[i].position[0];
		Y[i] = mol.listOfAtoms[i].position[1];
		Z[i] = mol.listOfAtoms[i].position[2];
	}
	*numberOfAtoms = mol.numberOfAtoms;
	*precision = mol.listOfAtoms[0].eps;

	freeAMolecule(&mol);
	return 0;
}
/************************************************************************************************************/
int generateMoleculeUsingAbelianGroup(
		gchar* abelianPointGroupSymbol,
	       	gint* numberOfAtoms, 
		gchar*** symbols, gdouble** X, gdouble** Y, gdouble** Z
		)
{

	gint numberOfGenerators;
	gint numberOfMolcasGenerators;
	gint numberOfElements;
	gchar* generators[3];
	gchar* molcasGenerators[3];
	gchar* elements[8];
	gint nAtoms = *numberOfAtoms;
	MolSymMolecule  mol;
	gdouble eps = -1;
	gint ret = 0;
	gint i;
	 
	if(*numberOfAtoms<1) return -1;

	ret = setMolSymMolecule(&mol, nAtoms, *symbols, *X, *Y, *Z, eps);
	if (ret != 0) return -1;

	for(i=0;i<3;i++)
	{
		generators[i] = g_malloc(100*sizeof(gchar));
		molcasGenerators[i] = g_malloc(100*sizeof(gchar));
	}
	for(i=0;i<8;i++)
		elements[i] = g_malloc(100*sizeof(gchar));

	setGeneratorsAbelianGroup(abelianPointGroupSymbol, &numberOfGenerators, generators, 
		&numberOfMolcasGenerators, molcasGenerators, 
		&numberOfElements, elements);

	generateMoleculeFromTheBasisSetOfAtoms(&mol, numberOfElements, elements);

	if(*symbols)
	{
		for(i=0;i<*numberOfAtoms;i++)
			g_free((*symbols)[i]);
		g_free((*symbols));
	}
	if(*X) g_free(*X);
	if(*Y) g_free(*Y);
	if(*Z) g_free(*Z);
	*symbols = g_malloc(mol.numberOfAtoms*sizeof(gchar*));
	*X = g_malloc(mol.numberOfAtoms*sizeof(gdouble));
	*Y = g_malloc(mol.numberOfAtoms*sizeof(gdouble));
	*Z = g_malloc(mol.numberOfAtoms*sizeof(gdouble));

	*numberOfAtoms = mol.numberOfAtoms;
	for(i=0;i<mol.numberOfAtoms;i++)
	{
		(*symbols)[i] = g_strdup(mol.symbol[mol.listOfAtoms[i].type]);
		(*X)[i] = mol.listOfAtoms[i].position[0];
		(*Y)[i] = mol.listOfAtoms[i].position[1];
		(*Z)[i] = mol.listOfAtoms[i].position[2];
	}

	for(i=0;i<3;i++)
	{
		g_free(generators[i]);
		g_free(molcasGenerators[i]);
	}
	for(i=0;i<8;i++)
		g_free(elements[i]);


	return 0;
}
/************************************************************************************************************/
void buildStandardOrientation(gint numberOfAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z)
{
	gdouble eps = -1.0;
	MolSymMolecule mol;
	gdouble centerOfGravity[3]; 
	gint numberOfEquivalentAxes;
	gdouble inertialMoment[3];
	gdouble axes[3][3];
	gint ret;
	gint i;
	gdouble principalAxisTolerance = 0.001;
	gdouble x,y,z,m,mtot;
	MolSymAtom *atomList;

	ret = setMolSymMolecule(&mol, numberOfAtoms, symbols, X, Y, Z, eps);
	if (ret != 0) return;

	determinePrincipalAxis(&mol,centerOfGravity,&numberOfEquivalentAxes,inertialMoment,axes, principalAxisTolerance,FALSE);  /* transform to principal axes */


	atomList = mol.listOfAtoms;
	x = y = z =0.0;
	mtot = 0;
	for (i=0;i<mol.numberOfAtoms;i++)	  /* center of gravity and total mass */
	{
		/* m = sqrt(prime[atomList->type]);*/
		m = fabs(mol.masse[atomList->type]);
		x += m*atomList->position[0];
		y += m*atomList->position[1];
		z += m*atomList->position[2];
		mtot += m;
		atomList++;
	  }
	centerOfGravity[0] = x/mtot;
	centerOfGravity[1] = y/mtot;
	centerOfGravity[2] = z/mtot;

      	for (i=0;i < mol.numberOfAtoms;i++)
	{ 
		X[i] = mol.listOfAtoms[i].position[0]-centerOfGravity[0];
		Y[i] = mol.listOfAtoms[i].position[1]-centerOfGravity[1];
		Z[i] = mol.listOfAtoms[i].position[2]-centerOfGravity[2];
	}

	freeAMolecule(&mol);

	
}
/*
 INPUT 
gdouble principalAxisTolerance;	  	tolerance for principal axis classification
gboolean axz_3;  			 O and I symmetry: z-axis of C3 type ?
gint maximalOrder;	 		maximal order for z-axis
gboolean redu;    			reduction to symmetry base

INPUT OUPUT
gint* numberOfAtoms;   			number of atoms (modified if reduction)
gchar** symbols;			symbol of atoms;
gdouble *X,*Y,*Z;  			positions of atoms; (modified)
gint pointGroupSymbol;	  		if not NO ->  symmetry fixed manually (modified if pointGroupSymbol="NO")

OUPUT
gchar* message;      			Error Message 
*/
/************************************************************************************************************/
int computeSymmetryOld(
		gdouble principalAxisTolerance, gboolean axz_3, gchar* pointGroupSymbol,
		gint maximalOrder, gboolean redu,
	       	gint* numberOfAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z,  gdouble* precision, gchar* message)
{
	gdouble eps = *precision;
	MolSymMolecule mol;
	gdouble centerOfGravity[3]; 
	gint numberOfEquivalentAxes;
	gdouble inertialMoment[3];
	gdouble axes[3][3];
	gdouble ax1[3],ax2[3];
	gint ret;
	gint nax = 0; /* order of z axis */
	gint symmetry;
	gint symf = -1;
	gchar* syml = pointGroupSymbol;
	gint i;
	gint nAtoms = *numberOfAtoms;

	sprintf(message,"C1 Group");

	if(strcmp(syml,"NO")!=0) getSymmetryFromSymbol(syml,&symf,&nax, message);
	
	if(symf == 0 && nax<2)
	{
		sprintf(pointGroupSymbol,"C1");
		return 0;
	}

	ret = setMolSymMolecule(&mol, nAtoms, symbols, X, Y, Z, eps);
	if (ret != 0)
	{

		sprintf(message,_("Not enough memory for molecule"));
		return ret;
	}

	determinePrincipalAxis(&mol,centerOfGravity,&numberOfEquivalentAxes,inertialMoment,axes, principalAxisTolerance, TRUE);  /* transform to principal axes */
	
	if (symf < 0 ) /* symmetry not fixed */
	{
		symmetry = determineSymmetry(&mol,&nax,numberOfEquivalentAxes, maximalOrder, principalAxisTolerance, message);
	}
	else
	{
		gchar ns[100];
		gint naxOld = nax;
		symmetry =determineSymmetry(&mol,&nax,numberOfEquivalentAxes, maximalOrder, principalAxisTolerance, message);
		getPointGroupSymbol(symmetry, nax, ns);
		uppercase(pointGroupSymbol);
		uppercase(ns);
		if(strcmp(ns,pointGroupSymbol)!=0) 
		{
			nax = naxOld;

			symmetry = symf;
		}
	}
	



	if(symmetry == 0 && nax<2)
	{
		*precision = mol.listOfAtoms[0].eps;
		return -1;
	}
	if (redu) reduceMoleculeToItsBasisSetOfAtoms(&mol,symmetry,nax);
	if ((symmetry & SYM_O) && !axz_3)           /* x,y,z axis C4 symmetry */
	{
		ax1[0] = sqrt(2./3.);
		ax1[1] = 0.0;
		ax1[2] = sqrt(1./3.);
		ax2[0] = sqrt(1./6.);
		ax2[1] = sqrt(1./2.);
		ax2[2] =-sqrt(1./3.);
      
		rotateMoleculeToPlaceFirstPointOnZAxisAndSecondOnXZPlane(&mol,ax1,ax2);                 
		qsort(mol.listOfAtoms,mol.numberOfAtoms, sizeof(MolSymAtom),compare2atomsUsingCenterOfGravity);
		nax = 4;
	}

	if ((symmetry & SYM_IC) && ! axz_3)      /* z axis C5 symmetry */
	{
		ax1[0] = C5X;
		ax1[1] = 0.0;
		ax1[2] = C5Z;
		ax2[0] = 0.0;
		ax2[1] = 0.0;
		ax2[2] = 1.0;
      
		rotateMoleculeToPlaceFirstPointOnZAxisAndSecondOnXZPlane(&mol,ax1,ax2);                 
		qsort(mol.listOfAtoms,mol.numberOfAtoms, sizeof(MolSymAtom),compare2atomsUsingCenterOfGravity);
		nax = 5;
	}
	getPointGroupSymbol(symmetry,nax, pointGroupSymbol);


      	for (i=0;i < mol.numberOfAtoms;i++)
	{ 
		sprintf(symbols[i],"%s",mol.symbol[mol.listOfAtoms[i].type]);
		X[i] = mol.listOfAtoms[i].position[0];
		Y[i] = mol.listOfAtoms[i].position[1];
		Z[i] = mol.listOfAtoms[i].position[2];
	}
	*numberOfAtoms = mol.numberOfAtoms;
	*precision = mol.listOfAtoms[0].eps;

	freeAMolecule(&mol);
	return 0;
}
/************************************************************************************************************/
int computeSymmetry(
		gdouble principalAxisTolerance, gchar* pointGroupSymbol,
		gint maximalOrder, gboolean redu,
	       	gint* numberOfAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z,  gdouble* precision, gchar* message)
{
	gdouble eps = *precision;
	gint nAtoms = *numberOfAtoms;
	gdouble* mass = NULL;
	gint i = 0;
	SMolecule mol = newSMolecule();
	Symmetry sym;
	SMolecule* pmol = NULL;

	if(eps<=0) eps = 0.1;
	sprintf(message,"C1 Group");
	if(nAtoms<1) return 1;
	mass = g_malloc(nAtoms*sizeof(gdouble));
	for(i=0;i<nAtoms;i++) { SAtomsProp prop = prop_atom_get(symbols[i]); mass[i] =  prop.masse;}
	mol.setMolecule(&mol,  nAtoms, symbols, mass, X, Y, Z);
	sym = newSymmetry(&mol, eps);
	sym.setMaxDegree(&sym,maximalOrder);
	sym.setMomentTolerance(&sym,principalAxisTolerance);
	sym.findAllPointGroups(&sym);
	sprintf(pointGroupSymbol,"%s",sym.findSinglePointGroup(&sym));
	printf("group=%s\n",pointGroupSymbol);
       	sym.printElementResults(&sym);
	g_free(mass);
	sym.getUniqueMolecule(&sym);

	if(!strcmp(pointGroupSymbol,"C1")) return 0;

	if (redu) sym.getUniqueMolecule(&sym);

	pmol = &sym.molecule;
      	for (i=0;i < pmol->size(pmol);i++)
	{ 
		SAtom* atom = pmol->get(pmol,i);
		Point3D pos = atom->getPosition(atom);
		X[i] = pos.x;
		Y[i] = pos.y;
		Z[i] = pos.z;
	}
	*numberOfAtoms = pmol->size(pmol);
	sym.free(&sym);
	mol.free(&mol);

	return 0;
}
/************************************************************************************************************/
int computeSymmetrization(
		gdouble principalAxisTolerance, gchar* pointGroupSymbol, gint maximalOrder,
	       	gint* numberOfAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z,  gdouble* precision, gchar* message)
{
	gdouble eps = *precision;
	gint nAtoms = *numberOfAtoms;
	gdouble* mass = NULL;
	gint i = 0;
	SMolecule mol = newSMolecule();
	Symmetry sym;
	gchar* gn;
	if(eps<=0) eps = 0.1;

	sprintf(message,"C1 Group");
	if(nAtoms<1) return 1;
	mass = g_malloc(nAtoms*sizeof(gdouble));
	for(i=0;i<nAtoms;i++) { SAtomsProp prop = prop_atom_get(symbols[i]); mass[i] =  prop.masse;}
	mol.setMolecule(&mol,  nAtoms, symbols, mass, X, Y, Z);
	sym = newSymmetry(&mol, eps);
	sym.setMaxDegree(&sym,maximalOrder);
	sym.setMomentTolerance(&sym,principalAxisTolerance);
	sym.findAllPointGroups(&sym);
	gn =sym.getGroupName(&sym);
	sprintf(pointGroupSymbol,"%s",gn);
	g_free(gn);
	if(!strcmp(pointGroupSymbol,"C1")) return 0;
       	sym.printElementResults(&sym);
       	sym.printPointGroupResults(&sym);
	g_free(mass);

	mol.free(&mol);
	mol = sym.getSymmetrizeMolecule(&sym);

	if(mol.size(&mol)==nAtoms)
      	for (i=0;i < mol.size(&mol);i++)
	{ 
		SAtom* atom = mol.get(&mol,i);
		Point3D pos = atom->getPosition(atom);
		X[i] = pos.x;
		Y[i] = pos.y;
		Z[i] = pos.z;
		if(symbols && symbols[i]) g_free(symbols[i]);
		symbols[i] = g_strdup(atom->getSymbol(atom));
	}
	sym.free(&sym);
	mol.free(&mol);

	return 0;
}
