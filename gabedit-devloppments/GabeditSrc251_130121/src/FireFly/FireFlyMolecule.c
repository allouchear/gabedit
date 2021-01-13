/* FireFlyMolecule.c */
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

#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "../../Config.h"
#include "../Common/Global.h"
#include "../FireFly/FireFlyTypes.h"
#include "../FireFly/FireFlyGlobal.h"
#include "../FireFly/FireFlyControl.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomConversion.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"
#include "../Utils/AtomsProp.h"
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"

/************************************************************************************************************/
typedef enum
{
	GABEDIT = 0, GABFIXED
}TypeOfSymmetryButton;


static gboolean symReduction = TRUE;
static GtkWidget* labelSymmetry = NULL;
static GtkWidget *buttonTolerance = NULL;
static GtkWidget *buttonSymWithCalc = NULL;
/************************************************************************************************************/
static gint totalCharge = 0;
static gint spinMultiplicity=1;
/************************************************************************************************************/
gint getFireFlyMultiplicity()
{
	return spinMultiplicity;
}
/************************************************************************************************************/
void initFireFlyMoleculeButtons()
{
	labelSymmetry = NULL;
	buttonTolerance = NULL;
	buttonSymWithCalc = NULL;
}
/************************************************************************************************************/
void initFireFlyMolecule()
{
	fireflyMolecule.listOfAtoms = NULL;  
	fireflyMolecule.totalNumberOfElectrons = 0;
	fireflyMolecule.numberOfValenceElectrons = 0;
	fireflyMolecule.numberOfAtoms = 0;
	fireflyMolecule.groupSymmetry = NULL;
}
/************************************************************************************************************/
void freeFireFlyMolecule()
{
	static gboolean first = TRUE;

	if(first)
	{
		initFireFlyMolecule();
		first = FALSE;
		return;
	}

	if(fireflyMolecule.listOfAtoms) g_free(fireflyMolecule.listOfAtoms);
	if(fireflyMolecule.groupSymmetry) g_free(fireflyMolecule.groupSymmetry);
	initFireFlyMolecule();
}
/************************************************************************************************************/
static gint setFireFlyMoleculeFromSXYZ(gint nAtoms, gchar** symbols, gdouble* X, gdouble* Y, gdouble* Z)
{
	gint n;
	FireFlyAtom* atomList = NULL;

	fireflyMolecule.listOfAtoms = NULL;  
	fireflyMolecule.numberOfAtoms = 0;
	if(nAtoms<1) return 1;

	fireflyMolecule.listOfAtoms = (FireFlyAtom*)g_malloc(sizeof(FireFlyAtom)*(nAtoms));
	if(fireflyMolecule.listOfAtoms==NULL) return -1;

	fireflyMolecule.numberOfAtoms = nAtoms;

	atomList = fireflyMolecule.listOfAtoms;
	for(n=0; n<fireflyMolecule.numberOfAtoms; n++)
	{
		atomList->position[0]  = X[n];
		atomList->position[1]  = Y[n];
		atomList->position[2]  = Z[n];
		atomList->symbol  = g_strdup(symbols[n]);
		atomList++;
	}

	return 0;
}
/************************************************************************************************************/
static void setXYZFromGeomXYZ(gint i, gdouble* x, gdouble* y, gdouble *z)
{
  	if(!test(GeomXYZ[i].X))
    		*x = get_value_variableXYZ(GeomXYZ[i].X);
  	else
    		*x = atof(GeomXYZ[i].X);
  	if(!test(GeomXYZ[i].Y))
    		*y = get_value_variableXYZ(GeomXYZ[i].Y);
  	else
    		*y = atof(GeomXYZ[i].Y);
  	if(!test(GeomXYZ[i].Z))
    		*z = get_value_variableXYZ(GeomXYZ[i].Z);
  	else
    		*z = atof(GeomXYZ[i].Z);

         if(Units==0)
         {
              *x *= BOHR_TO_ANG;
              *y *= BOHR_TO_ANG;
              *z *= BOHR_TO_ANG;
         }
}
/************************************************************************************************************/
static void setFireFlyFormatGroup(gchar* pointGroupSymbol, gchar* fireflyName)
{
	if(!pointGroupSymbol) return;
	if(strlen(pointGroupSymbol)<2 || strcmp(pointGroupSymbol,"C1")==0)
	{
		sprintf(fireflyName,"%s",pointGroupSymbol);
		return;
	}
	if(strcmp(pointGroupSymbol,"Cinfv")==0)
	{
		sprintf(fireflyName,"Cnv  4");
		return;
	}
	if(strcmp(pointGroupSymbol,"Dinfh")==0)
	{
		sprintf(fireflyName,"Dnh  4");
		return;
	}
	if(isdigit(pointGroupSymbol[1]))
	{
		gchar a[3];
		sprintf(a,"%c",pointGroupSymbol[1]);
		gint n=atoi(a);
		if(pointGroupSymbol[0] !='S')
		{
			if(strlen(pointGroupSymbol)>2)
			sprintf(fireflyName,"%cn%c %d",pointGroupSymbol[0],pointGroupSymbol[2],n);
			else
			sprintf(fireflyName,"%cn %d",pointGroupSymbol[0],n);
		}
		else sprintf(fireflyName,"%c2n %d",pointGroupSymbol[0],n/2);
		return;
	}
	sprintf(fireflyName,"%s",pointGroupSymbol);
	return;
}
/************************************************************************************************************/
static gchar* computeGroupSymmetry()
{
	gint i;
	gchar** symbols = NULL;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble* Z = NULL;
	gint numberOfAtoms = fireflyMolecule.numberOfAtoms;
	gchar pointGroupSymbol[BSIZE];
	gchar message[BSIZE];
	gint maximalOrder = 8;
	gdouble principalAxisTolerance = getTolerancePrincipalAxis();
	gdouble positionTolerance = getTolerancePosition();

	if(numberOfAtoms<1) return NULL;


	symbols = (gchar**)g_malloc(sizeof(gchar*)*(numberOfAtoms));
	if(symbols == NULL) return NULL;

	X = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(X == NULL) return NULL;
	Y = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(Y == NULL) return NULL;
	Z = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(Z == NULL) return NULL;

	for(i=0; i<numberOfAtoms; i++)
	{
		symbols[i] = g_strdup(fireflyMolecule.listOfAtoms[i].symbol);
		X[i] = fireflyMolecule.listOfAtoms[i].position[0];
		Y[i] = fireflyMolecule.listOfAtoms[i].position[1];
		Z[i] = fireflyMolecule.listOfAtoms[i].position[2];
	}
	sprintf(pointGroupSymbol,"NO");
	computeSymmetryOld(principalAxisTolerance, FALSE, pointGroupSymbol,maximalOrder, TRUE, &numberOfAtoms,symbols, X, Y, Z, &positionTolerance, message);


	for (i=0;i<(gint)numberOfAtoms;i++) g_free( symbols[i]);
	g_free( symbols);
	g_free(X);
	g_free(Y);
	g_free(Z);
	return g_strdup(pointGroupSymbol);
}
/************************************************************************************************************/
static gboolean setFireFlyMoleculeFromGeomXYZ()
{
	gint i;
	gchar** symbols = NULL;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble* Z = NULL;
	gint numberOfAtoms = NcentersXYZ;

	if(numberOfAtoms<1) return FALSE;

	symbols = (gchar**)g_malloc(sizeof(gchar*)*(numberOfAtoms));

	if(symbols == NULL) return FALSE;

	X = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(X == NULL) return FALSE;
	Y = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(Y == NULL) return FALSE;
	Z = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(Z == NULL) return FALSE;

	fireflyMolecule.totalNumberOfElectrons = 0;
	for(i=0; i<numberOfAtoms; i++)
	{
		SAtomsProp prop = prop_atom_get(GeomXYZ[i].Symb);

		symbols[i] = g_strdup(GeomXYZ[i].Symb);
		setXYZFromGeomXYZ(i, &X[i] , &Y[i] , &Z[i]);
		fireflyMolecule.totalNumberOfElectrons += prop.atomicNumber;
	}
	fireflyMolecule.numberOfValenceElectrons = fireflyMolecule.totalNumberOfElectrons;
	setFireFlyMoleculeFromSXYZ(numberOfAtoms, symbols, X, Y, Z);
	fireflyMolecule.groupSymmetry = computeGroupSymmetry();

	for (i=0;i<(gint)NcentersXYZ;i++) g_free( symbols[i]);
	g_free( symbols);
	g_free(X);
	g_free(Y);
	g_free(Z);
	return TRUE;
}
/************************************************************************************************************/
static gboolean setFireFlyMoleculeFromGeomZMatrix()
{
	iprogram=PROG_IS_FIREFLY;
	if(!zmat_to_xyz()) return FALSE;
	/*delete_dummy_atoms();*/
	/* conversion_zmat_to_xyz();*/
	return setFireFlyMoleculeFromGeomXYZ();
}
/************************************************************************************************************/
gboolean setFireFlyMolecule()
{
	freeFireFlyMolecule();
	if(MethodeGeom==GEOM_IS_XYZ && setFireFlyMoleculeFromGeomXYZ()) return TRUE;
	if(setFireFlyMoleculeFromGeomZMatrix()) return TRUE;
	return FALSE;
}
/************************************************************************************************************/
void setFireFlyGeometryFromInputFile(gchar* fileName)
{
	read_XYZ_from_gamess_input_file(fileName);
	setFireFlyMolecule();
}
/*************************************************************************************************************/
/*
static gdouble getMinDistance()
{
	gdouble d=0;
	gint i;
	gint k;
	FireFlyAtom* atomList = fireflyMolecule.listOfAtoms;
	for(i=0; i<fireflyMolecule.numberOfAtoms-1; i++)
	{
		gdouble dd = 0;
		for(k=0;k<3;k++) 
		{
			gdouble xx = atomList->position[k]-atomList->position[k+1];
			dd += xx*xx;
		}
		if(i==0) d = dd;
		else if(d>dd) d= dd;
		atomList++;
	}
	d = sqrt(d);

	return d;
}
*/
/*************************************************************************************************************/
/*
static void setFirstAtomToXAxis(gint numberOfAtoms, gdouble* X, gdouble* Y, gdouble*Z)
{
	gdouble d;
	gdouble s;
	gdouble c;
	gint i;
	gdouble positionTolerance = getTolerancePosition();

	if(numberOfAtoms<1) return;
	d = X[0]*X[0]+Y[0]*Y[0];
	if(d<1e-10) return;
	d = sqrt(d);
	if(positionTolerance<0) positionTolerance= getMinDistance()/50;

	s = -Y[0]/d;
	c = +X[0]/d;

	for (i=0;i<numberOfAtoms;i++)
	 {
		 gdouble x = X[i];
		 gdouble y = Y[i];
		X[i] = c*x - s*y;
		Y[i] = s*x + c*y;
		if(fabs(Y[i])<positionTolerance) Y[i]=0.0;
	 }

}
*/
/*************************************************************************************************************/
static gint getRealNumberXYZVariables()
{
	gint k=0;
	gint i;
        for(i=0;i<NcentersXYZ;i++)
	{
		if(test(GeomXYZ[i].X))k++;
		if(test(GeomXYZ[i].Y))k++;
		if(test(GeomXYZ[i].Z))k++;
	}
	return k;
}
/*************************************************************************************************************/
static void putFireFlyMoleculeXYZFixedInTextEditor()
{
        gchar buffer[BSIZE];
	gint i,k,l;
	gint nvar = 0;
	gint nrvar = 0;
	if(MethodeGeom!=GEOM_IS_XYZ)return;
        if(NcentersXYZ<2)return;
	nrvar = getRealNumberXYZVariables();
	nvar = 3*NcentersXYZ;
	if(nrvar==nvar) return;
	if(nrvar==0) return;

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$STATPT\n",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "   IFREEZ(1)=",-1);

	k = 0;
	l = 0;
        for(i=0;i<NcentersXYZ;i++)
	{
		k++;
		if(test(GeomXYZ[i].X))
		{
			l++;
			sprintf(buffer,"%d, ",k);
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
			if(l%10==0)
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n   ", -1);
		}
		k++;
		if(test(GeomXYZ[i].Y))
		{
			l++;
			sprintf(buffer,"%d, ",k);
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
			if(l%10==0)
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n   ", -1);
		}
		k++;
		if(test(GeomXYZ[i].Z))
		{
			l++;
			sprintf(buffer,"%d, ",k);
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
			if(l%10==0)
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n   ", -1);
		}
	}
	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);
}
/*************************************************************************************************************/
static gint getRealNumberZmatVariables()
{
	gint k=0;
	gint i;
        for(i=0;i<NcentersZmat;i++)
	{
        	if(Geom[i].Nentry>NUMBER_ENTRY_0 && !test(Geom[i].R)) k++;
        	if(Geom[i].Nentry>NUMBER_ENTRY_R && !test(Geom[i].Angle)) k++;
        	if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE && !test(Geom[i].Dihedral)) k++;
	}
	return k;
}
/*************************************************************************************************************/
static void putFireFlyMoleculeZMatInTextEditor()
{
        gchar buffer[BSIZE];
	gint i,k,l;
	gint nzvar = 0;
	gint nrzvar = 0;
	if(MethodeGeom==GEOM_IS_XYZ)return;
        if(NcentersZmat<2)return;
	 if(NcentersZmat==2) nzvar = 3*NcentersZmat-5;
	 else nzvar = 3*NcentersZmat-6;
	nrzvar = getRealNumberZmatVariables(); 

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$CONTRL",-1);
	sprintf(buffer," COORD=ZMT NZVAR=%d ",nzvar);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$ZMAT\n",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "   IZMAT(1)=\n   ",-1);
        for(i=0;i<NcentersZmat;i++)
	{
        	if(Geom[i].Nentry>NUMBER_ENTRY_0)
		{
			sprintf(buffer,"1,%d,%s, ",i+1,Geom[i].NR);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
			if((i+1)%3==0)
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n   ", -1);
		}
	}
	if(NcentersZmat%3!=0)
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n   ", -1);
        for(i=0;i<NcentersZmat;i++)
	{
        	if(Geom[i].Nentry>NUMBER_ENTRY_R)
		{
			sprintf(buffer,"2,%d,%s,%s, ",i+1,Geom[i].NR,Geom[i].NAngle);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
			if((i+1)%3==0)
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n   ", -1);
		}
	}
	if(NcentersZmat%3!=0)
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n   ", -1);
        for(i=0;i<NcentersZmat;i++)
	{
        	if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE)
		{
			sprintf(buffer,"3,%d,%s,%s,%s, ",i+1,Geom[i].NR,Geom[i].NAngle,Geom[i].NDihedral);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
			if((i+1)%3==0)
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n   ", -1);
		}
	}
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);
	if(nrzvar==nzvar) return;
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$STATPT\n",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "   IFREEZ(1)=",-1);

	k = 0;
	l = 0;
        for(i=0;i<NcentersZmat;i++)
	{
        	if(Geom[i].Nentry>NUMBER_ENTRY_0)
		{
			k++;
			if(test(Geom[i].R))
			{
				l++;
				sprintf(buffer,"%d, ",k);
				gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
				if(l%5==0)
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n   ", -1);
			}
		}
	}
	 if(l!=0 && l%5!=0) gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n    ", -1);
	 l = 0;
        for(i=0;i<NcentersZmat;i++)
	{
        	if(Geom[i].Nentry>NUMBER_ENTRY_R)
		{
			k++;
			if(test(Geom[i].Angle))
			{
				l++;
				sprintf(buffer,"%d, ",k);
				gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
				if(l%5==0)
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n   ", -1);
			}
		}
	}
	 if(l!=0 && l%5!=0) gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n   ", -1);
	 l = 0;
        for(i=0;i<NcentersZmat;i++)
	{
        	if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE)
		{
			k++;
			if(test(Geom[i].Dihedral))
			{
				l++;
				sprintf(buffer,"%d, ",k);
				gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
				if(l%5==0)
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n   ", -1);
			}
		}
	}
	 gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);
}
/************************************************************************************************************/
static void setEpsToZero(gint n, gdouble* X, gdouble* Y, gdouble* Z, gdouble eps)
{
	gint i;

	if(n<1) return;
	if(!X) return;
	if(!Y) return;
	if(!Z) return;
	if(eps<=0) eps = 1e-3;

	for(i=0;i<n;i++)
	{
		if( fabs(X[i]) < eps) X[i] = 0;
		if( fabs(Y[i]) < eps) Y[i] = 0;
		if( fabs(Z[i]) < eps) Z[i] = 0;
	}
}
/************************************************************************/
static gboolean build_rotation_matrix_about_an_axis(gdouble m[3][3], gdouble* vect, gdouble angle)
{
	gdouble q[4];
	gdouble norm = 1;
	gdouble angleRad = angle/180.0*PI;
	gdouble vcos ;
	gdouble vsin ;
	if(!vect)return FALSE;
	norm = vect[0]*vect[0] + vect[1]*vect[1] + vect[2]*vect[2];
	norm = sqrt(norm);
	if(norm <1e-8) return FALSE;
	vect[0] /= norm;
	vect[1] /= norm;
	vect[2] /= norm;
	vcos = cos(angleRad/2);
	vsin = sin(angleRad/2);
	q[0] = vect[0]*vsin;
	q[1] = vect[1]*vsin;
	q[2] = vect[2]*vsin;
	q[3] = vcos;

	m[0][0] = 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]);
    	m[0][1] = 2.0 * (q[0] * q[1] - q[2] * q[3]);
    	m[0][2] = 2.0 * (q[2] * q[0] + q[1] * q[3]);

    	m[1][0] = 2.0 * (q[0] * q[1] + q[2] * q[3]);
    	m[1][1]= 1.0 - 2.0 * (q[2] * q[2] + q[0] * q[0]);
    	m[1][2] = 2.0 * (q[1] * q[2] - q[0] * q[3]);

    	m[2][0] = 2.0 * (q[2] * q[0] - q[1] * q[3]);
    	m[2][1] = 2.0 * (q[1] * q[2] + q[0] * q[3]);
    	m[2][2] = 1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]);
	return TRUE;
}
/************************************************************************/
static gboolean build_rotation_about_an_axis(gdouble* vect, gdouble angle, gint n, gdouble* X, gdouble* Y, gdouble* Z)
{
	gdouble m[3][3];
	gboolean res = build_rotation_matrix_about_an_axis(m,vect, angle);
	gint i;
	if(!res) return res;
	for(i=0;i<n;i++)
	{
		gdouble x = X[i]*m[0][0]+Y[i]*m[0][1]+Z[i]*m[0][2];
		gdouble y = X[i]*m[1][0]+Y[i]*m[1][1]+Z[i]*m[1][2];
		gdouble z = X[i]*m[2][0]+Y[i]*m[2][1]+Z[i]*m[2][2];
		X[i] =x;
		Y[i] =y;
		Z[i] =z;
	}
	return TRUE;
}
/*************************************************************************************************************/
static void putFireFlyMoleculeInTextEditor()
{
        gchar buffer[BSIZE];
        gchar g[BSIZE];
	gint i;
	gchar** symbols = NULL;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble* Z = NULL;
	gint numberOfAtoms = fireflyMolecule.numberOfAtoms;
	gchar pointGroupSymbol[BSIZE];
	gchar message[BSIZE];
	gint maximalOrder = 8;
	gdouble principalAxisTolerance = getTolerancePrincipalAxis();
	gdouble positionTolerance = getTolerancePosition();
	gint nrvar = 0;

	if(fireflyMolecule.numberOfAtoms<1) return;
	nrvar = getRealNumberXYZVariables();

	symbols = (gchar**)g_malloc(sizeof(gchar*)*(numberOfAtoms));
	if(symbols == NULL) return;

	X = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(X == NULL) return;
	Y = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(Y == NULL) return;
	Z = (gdouble*)g_malloc(sizeof(gdouble)*(numberOfAtoms));
	if(Z == NULL) return;
	for(i=0; i<numberOfAtoms; i++)
	{
		symbols[i] = g_strdup(fireflyMolecule.listOfAtoms[i].symbol);
		X[i] = fireflyMolecule.listOfAtoms[i].position[0];
		Y[i] = fireflyMolecule.listOfAtoms[i].position[1];
		Z[i] = fireflyMolecule.listOfAtoms[i].position[2];
	}

	if(MethodeGeom==GEOM_IS_XYZ && (nrvar== 3*NcentersXYZ || nrvar==0))
	{
		sprintf(pointGroupSymbol,"%s",fireflyMolecule.groupSymmetry);
		computeSymmetryOld(principalAxisTolerance, FALSE, pointGroupSymbol,maximalOrder, TRUE, &numberOfAtoms,symbols, X, Y, Z, &positionTolerance, message);
		/*
		if(strlen(pointGroupSymbol)>1 && strcmp(pointGroupSymbol,"C1")!=0 && isdigit(pointGroupSymbol[1]))
			setFirstAtomToXAxis(numberOfAtoms, X, Y, Z);
			*/
		setFireFlyFormatGroup(fireflyMolecule.groupSymmetry,g);
	}
	else
	{
		sprintf(g,"C1");
	}

	if(strcmp(g,"C1")==0 || MethodeGeom!=GEOM_IS_XYZ) sprintf(buffer,"%s\n","C1");
	else sprintf(buffer,"%s\n\n",g);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	if(MethodeGeom==GEOM_IS_XYZ)
	{
		if(buffer[0]=='T') 
		{
			gdouble vect[3] = {-1.0,1.0,0.0};
			vect[0] = 0.0;
			vect[1] = 0.0;
			vect[2] = 1.0;
			build_rotation_about_an_axis(vect, -135, numberOfAtoms, X, Y,Z);
			setEpsToZero(numberOfAtoms, X, Y, Z, positionTolerance);
			vect[0] = -1.0;
			vect[1] = 1.0;
			vect[2] = 0.0;
			build_rotation_about_an_axis(vect, atan(sqrt(2.0))*180/PI, numberOfAtoms, X, Y,Z);
			setEpsToZero(numberOfAtoms, X, Y, Z, positionTolerance);
		}
		if(strlen(buffer)>2 && buffer[0]=='D' && buffer[2]=='d') 
		{
			gdouble vect[3] = {0.0,0.0,1.0};
			gchar* t = strstr(buffer,"d")+1;
			gint o = 1;
			if(t && atoi(t)>0) o = atoi(t);
			
			build_rotation_about_an_axis(vect, 180/2/o, numberOfAtoms, X, Y,Z);
			setEpsToZero(numberOfAtoms, X, Y, Z, positionTolerance);
		}
		if(strlen(buffer)>2 && buffer[0]=='D' && buffer[2]=='h') 
		{
			gdouble vect[3] = {0.0,0.0,1.0};
			gchar* t = strstr(buffer,"h")+1;
			gint o = 1;
			if(t && atoi(t)>0) o = atoi(t);
			
			build_rotation_about_an_axis(vect, 2*180/2/o, numberOfAtoms, X, Y,Z);
			setEpsToZero(numberOfAtoms, X, Y, Z, positionTolerance);
		}
      		for (i=0;i<numberOfAtoms;i++)
		{
			SAtomsProp prop = prop_atom_get(symbols[i]);

			sprintf(buffer,"%s  %f %f %f %f\n",symbols[i], (gdouble)prop.atomicNumber, X[i], Y[i], Z[i]);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
		}
	}
	else
	{
        	for(i=0;i<NcentersZmat;i++)
        	{
			SAtomsProp prop = prop_atom_get(symbols[i]);
        		if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE)
			{
				sprintf(buffer,"%s  %s %s %s %s %s %s\n",
						Geom[i].Symb,
						Geom[i].NR,Geom[i].R,
						Geom[i].NAngle,Geom[i].Angle,
						Geom[i].NDihedral,Geom[i].Dihedral);
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, &prop.color, buffer, -1);
			}
			else
        		if(Geom[i].Nentry>NUMBER_ENTRY_R)
			{
				sprintf(buffer,"%s  %s %s %s %s\n",
						Geom[i].Symb,
						Geom[i].NR,Geom[i].R,
						Geom[i].NAngle,Geom[i].Angle
						);
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, &prop.color, buffer, -1);
			}
			else
        		if(Geom[i].Nentry>NUMBER_ENTRY_0)
			{
				sprintf(buffer,"%s  %s %s\n",
						Geom[i].Symb,
						Geom[i].NR,Geom[i].R
						);
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, &prop.color, buffer, -1);
			}
			else
			{
				sprintf(buffer,"%s \n",
						Geom[i].Symb
						);
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, &prop.color, buffer, -1);
			}
        	}
        	if(NVariables>0 && getRealNumberZmatVariables()>0)
		{
        	for(i=0;i<NVariables;i++)
        	{
        		if(Variables[i].Used)
			{
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\n",-1);
				break;
			}
        	}
        	for(i=0;i<NVariables;i++)
        	{
        		if(Variables[i].Used)
			{
  				sprintf(buffer,"%s=%s\n",Variables[i].Name,Variables[i].Value);
        			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,buffer,-1);
			}
        	}
		}
	}
	for (i=0;i<(gint)numberOfAtoms;i++) g_free( symbols[i]);
	g_free( symbols);
	g_free(X);
	g_free(Y);
	g_free(Z);

}
/************************************************************************************************************/
static void putNoSymmetryWithCalcul()
{
	if(!GTK_TOGGLE_BUTTON (buttonSymWithCalc)->active ) return;
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$CONTRL",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " NOSYM=1 ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);
}
/************************************************************************************************************/
static void putBeginGeometryInTextEditor()
{
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$DATA\n",-1);
}
/************************************************************************************************************/
static void putTitleGeometryInTextEditor()
{
        gchar buffer[BSIZE];

	sprintf(buffer,"Molecule specification\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
}
/************************************************************************************************************/
static void putEndGeometryInTextEditor()
{
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL,  &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);
}
/************************************************************************************************************/
void putFireFlyGeometryInfoInTextEditor()
{
	putNoSymmetryWithCalcul();
	if(MethodeGeom!=GEOM_IS_XYZ) putFireFlyMoleculeZMatInTextEditor();
	else putFireFlyMoleculeXYZFixedInTextEditor();

	putBeginGeometryInTextEditor();
	putTitleGeometryInTextEditor();

	putFireFlyMoleculeInTextEditor();
	putEndGeometryInTextEditor();
}
/**************************************************************************************************************************************/
static void activateRadioButton(GtkWidget *button, gpointer data)
{
	gint* type = NULL;
	GtkWidget* label = NULL;
	GtkWidget* comboSymmetry = NULL;
	gchar buffer[BSIZE];
	 
	if(!GTK_IS_WIDGET(button)) return;

	type  = g_object_get_data(G_OBJECT (button), "Type");
	label = g_object_get_data(G_OBJECT (button), "Label");
	comboSymmetry = g_object_get_data(G_OBJECT (button), "ComboSymmetry");
	if(type)
	{
		setFireFlyMolecule();

		if( GTK_TOGGLE_BUTTON (button)->active && *type == GABFIXED)
		{
			if(fireflyMolecule.groupSymmetry) g_free(fireflyMolecule.groupSymmetry);
			fireflyMolecule.groupSymmetry = g_strdup("C1");
			symReduction = FALSE;
			if(GTK_IS_WIDGET(comboSymmetry))
				gtk_widget_set_sensitive(comboSymmetry, TRUE);
		}
		if(GTK_TOGGLE_BUTTON (button)->active &&  *type == GABEDIT)
		{
			/* groupSymmetry define in setFireFlyMolecule */
			symReduction = TRUE;
			if(GTK_IS_WIDGET(comboSymmetry))
				gtk_widget_set_sensitive(comboSymmetry, FALSE);
		}

		if(label) gtk_label_set_text(GTK_LABEL(label)," ");
		if(GTK_IS_WIDGET(buttonTolerance) && symReduction ) gtk_widget_set_sensitive(buttonTolerance, TRUE);
		if(GTK_IS_WIDGET(buttonTolerance) && !symReduction ) gtk_widget_set_sensitive(buttonTolerance, FALSE);

		if(GTK_TOGGLE_BUTTON (button)->active && label && symReduction)
		{
			sprintf(buffer,"%s group",fireflyMolecule.groupSymmetry);
			gtk_label_set_text(GTK_LABEL(label),buffer);
		}
	}
}
/************************************************************************************************************/
static GtkWidget* addRadioButtonToATable(GtkWidget* table, GtkWidget* friendButton, gchar* label, gint i, gint j, gint k)
{
	GtkWidget *newButton;

	if(friendButton)
		newButton = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (friendButton)), label);
	else
		newButton = gtk_radio_button_new_with_label( NULL, label);

	gtk_table_attach(GTK_TABLE(table),newButton,j,j+k,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);

	g_object_set_data(G_OBJECT (newButton), "Label",NULL);
	g_object_set_data(G_OBJECT (newButton), "Type",NULL);
	return newButton;
}
/**************************************************************************************************************************************/
static void resetTolerance(GtkWidget *win)
{
	gchar buffer[BSIZE];

	setFireFlyMolecule();
	if(labelSymmetry)
	{
		sprintf(buffer,"%s group",fireflyMolecule.groupSymmetry);
		gtk_label_set_text(GTK_LABEL(labelSymmetry),buffer);
	}
}
/**************************************************************************************************************************************/
static void activateToleranceButton(GtkWidget *button, gpointer data)
{
	if(!GTK_IS_WIDGET(button)) return;
	createToleranceWindow(fireflyWin, resetTolerance);
}
/********************************************************************************/
static void setComboSymmetry(GtkWidget *comboSymmetry)
{
	GList *glist = NULL;

  	glist = g_list_append(glist,"C1");
  	glist = g_list_append(glist,"Cs");
  	glist = g_list_append(glist,"Ci");

  	glist = g_list_append(glist,"C2");
  	glist = g_list_append(glist,"C3");
  	glist = g_list_append(glist,"C4");
  	glist = g_list_append(glist,"C5");
  	glist = g_list_append(glist,"C6");
  	glist = g_list_append(glist,"C7");
  	glist = g_list_append(glist,"C8");

  	glist = g_list_append(glist,"C2v");
  	glist = g_list_append(glist,"C3v");
  	glist = g_list_append(glist,"C4v");
  	glist = g_list_append(glist,"C5v");
  	glist = g_list_append(glist,"C6v");
  	glist = g_list_append(glist,"C7v");
  	glist = g_list_append(glist,"C8v");

  	glist = g_list_append(glist,"C2h");
  	glist = g_list_append(glist,"C3h");
  	glist = g_list_append(glist,"C4h");
  	glist = g_list_append(glist,"C5h");
  	glist = g_list_append(glist,"C6h");
  	glist = g_list_append(glist,"C7h");
  	glist = g_list_append(glist,"C8h");

  	glist = g_list_append(glist,"D2");
  	glist = g_list_append(glist,"D3");
  	glist = g_list_append(glist,"D4");
  	glist = g_list_append(glist,"D5");
  	glist = g_list_append(glist,"D6");
  	glist = g_list_append(glist,"D7");
  	glist = g_list_append(glist,"D8");

  	glist = g_list_append(glist,"D2h");
  	glist = g_list_append(glist,"D3h");
  	glist = g_list_append(glist,"D4h");
  	glist = g_list_append(glist,"D5h");
  	glist = g_list_append(glist,"D6h");
  	glist = g_list_append(glist,"D7h");
  	glist = g_list_append(glist,"D8h");

  	glist = g_list_append(glist,"D2d");
  	glist = g_list_append(glist,"D3d");
  	glist = g_list_append(glist,"D4d");
  	glist = g_list_append(glist,"D5d");
  	glist = g_list_append(glist,"D6d");
  	glist = g_list_append(glist,"D7d");
  	glist = g_list_append(glist,"D8d");

  	glist = g_list_append(glist,"S2");
  	glist = g_list_append(glist,"S4");
  	glist = g_list_append(glist,"S6");
  	glist = g_list_append(glist,"S8");
  	glist = g_list_append(glist,"S10");
  	glist = g_list_append(glist,"S12");
  	glist = g_list_append(glist,"S14");
  	glist = g_list_append(glist,"S16");

  	glist = g_list_append(glist,"T");
  	glist = g_list_append(glist,"Td");
  	glist = g_list_append(glist,"Th");
  	glist = g_list_append(glist,"O");
  	glist = g_list_append(glist,"Oh");

  	gtk_combo_box_entry_set_popdown_strings( comboSymmetry, glist) ;

  	g_list_free(glist);
}
/**********************************************************************/
static void changedEntrySymmetry(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;
	if(fireflyMolecule.groupSymmetry) g_free(fireflyMolecule.groupSymmetry);
	fireflyMolecule.groupSymmetry = g_strdup(entryText);

	if(strstr(entryText,"C1")) symReduction = FALSE;
	else symReduction = FALSE;
}
/************************************************************************************************************/
void createFireFlySymmetryFrame(GtkWidget *win, GtkWidget *box)
{
	GtkWidget* button;
	GtkWidget* buttonGabedit;
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* sep;
	GtkWidget* comboSymmetry = NULL;
	GtkWidget* entrySymmetry = NULL;
	GtkWidget* label = gtk_label_new(" ");
	GtkWidget *table = gtk_table_new(4,3,FALSE);
	static TypeOfSymmetryButton typeOfSymmetry[] = { GABEDIT, GABFIXED};
	gchar* list[] = {"C1"};

	frame = gtk_frame_new ("Symmetry");
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	comboSymmetry = create_combo_box_entry(list, 1, TRUE, -1, -1);
	entrySymmetry = GTK_BIN(comboSymmetry)->child;
	gtk_widget_set_sensitive(entrySymmetry, FALSE);


	if(fireflyMolecule.groupSymmetry) g_free(fireflyMolecule.groupSymmetry);
	fireflyMolecule.groupSymmetry = g_strdup("C1");

	button = addRadioButtonToATable(table, NULL, _("Detected by Gabedit"), 0, 0, 1);
	g_object_set_data(G_OBJECT (button), "Label",label);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfSymmetry[GABEDIT]);
	g_object_set_data(G_OBJECT (button), "ComboSymmetry",comboSymmetry);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	add_widget_table(table, label, 0, 1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
	buttonGabedit=button;

	buttonTolerance = create_button(win,_("Tolerance"));
	add_widget_table(table, buttonTolerance, 0, 2);
	g_signal_connect(G_OBJECT(buttonTolerance),"clicked", G_CALLBACK(activateToleranceButton),NULL);

	labelSymmetry = label;
	gtk_widget_set_sensitive(buttonTolerance, FALSE);

	button = addRadioButtonToATable(table, button, _("Fixed Symmetry"), 1, 0,1);
	g_signal_connect(G_OBJECT(entrySymmetry),"changed", G_CALLBACK(changedEntrySymmetry),NULL);
	setComboSymmetry(comboSymmetry);
	gtk_table_attach(GTK_TABLE(table),comboSymmetry,1,1+2,1,1+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);
	g_object_set_data(G_OBJECT (button), "Label",label);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfSymmetry[GABFIXED]);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
	g_object_set_data(G_OBJECT (button), "ComboSymmetry",comboSymmetry);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	gtk_widget_set_sensitive(comboSymmetry, FALSE);

	sep = gtk_hseparator_new ();;
	gtk_table_attach(GTK_TABLE(table),sep,0,0+3,2,2+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);

	buttonSymWithCalc = gtk_check_button_new_with_label (_("Symmetry not used during calculation"));
	gtk_table_attach(GTK_TABLE(table),buttonSymWithCalc,0,0+3,3,3+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonGabedit), FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonGabedit), TRUE);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
}
/********************************************************************************/
static void setSpinMultiplicityComboSpinMultiplicity(GtkWidget *comboSpinMultiplicity, gint spin)
{
	GtkWidget *entry = NULL;
	gchar* t = NULL;
	if(!comboSpinMultiplicity) return;
	entry = GTK_BIN (comboSpinMultiplicity)->child;
	t = g_strdup_printf("%d",spin);
	gtk_entry_set_text(GTK_ENTRY(entry),t);
	g_free(t);
}
/************************************************************************************************************/
static void setComboSpinMultiplicity(GtkWidget *comboSpinMultiplicity)
{
	GList *glist = NULL;
	gint i;
	gint nlist = 0;
	gchar** list = NULL;
	gint k;
	gint kinc;
	gint ne = fireflyMolecule.numberOfValenceElectrons - totalCharge;

	if(ne%2==0) nlist = ne/2+1;
	else nlist = (ne+1)/2;

	if(nlist<1) return;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	if(GTK_IS_WIDGET(comboSpinMultiplicity)) gtk_widget_set_sensitive(comboSpinMultiplicity, TRUE);
	if(ne%2==0) k = 1;
	else k = 2;

	kinc = 2;
	for(i=0;i<nlist;i++)
	{
		sprintf(list[i],"%d",k);
		k+=kinc;
	}

  	for(i=0;i<nlist;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboSpinMultiplicity, glist) ;
  	g_list_free(glist);
	if( SpinMultiplicities[0]%2 == atoi(list[0])%2) setSpinMultiplicityComboSpinMultiplicity(comboSpinMultiplicity, SpinMultiplicities[0]);
	else SpinMultiplicities[0] = atoi(list[0]);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
}
/********************************************************************************/
static void setChargeComboCharge(GtkWidget *comboCharge, gint charge)
{
	GtkWidget *entry = NULL;
	gchar* t = NULL;
	if(!comboCharge) return;
	entry = GTK_BIN (comboCharge)->child;
	t = g_strdup_printf("%d",charge);
	gtk_entry_set_text(GTK_ENTRY(entry),t);
	g_free(t);
}
/********************************************************************************/
static void setComboCharge(GtkWidget *comboCharge)
{
	GList *glist = NULL;
	gint i;
	gint nlist;
	gchar** list = NULL;
	gint k;

	nlist = fireflyMolecule.numberOfValenceElectrons*2-2+1;

	if(nlist<1) return;
	if(nlist==1) nlist++;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	sprintf(list[0],"0");
	k = 1;
	for(i=1;i<nlist-1;i+=2)
	{
		sprintf(list[i],"+%d",k);
		sprintf(list[i+1],"%d",-k);
		k += 1;
	}
	if(nlist==2) sprintf(list[1],"%d",-1);

  	for(i=0;i<nlist;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboCharge, glist) ;
  	g_list_free(glist);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
	setChargeComboCharge(comboCharge, totalCharge);
}
/**********************************************************************/
static void changedEntrySpinMultiplicity(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	spinMultiplicity=atoi(entryText);
	if(spinMultiplicity==1)
	{
		/* OK RHF*/
		setFireFlySCFMethod(TRUE);
	}
	else 
	{
		/* remove RHF from list*/
		setFireFlySCFMethod(FALSE);
	}
	if(spinMultiplicity!=1 && spinMultiplicity!=3)
		setFireFlyTD(FALSE);
	else
		setFireFlyTD(TRUE);
}
/**********************************************************************/
static void changedEntryCharge(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* labelNumberOfElectrons = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	totalCharge = atoi(entryText);
	TotalCharges[0] = totalCharge;

	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entry), "ComboSpinMultiplicity");
	if(GTK_IS_WIDGET(comboSpinMultiplicity)) setComboSpinMultiplicity(comboSpinMultiplicity);

	labelNumberOfElectrons = g_object_get_data(G_OBJECT (entry), "LabelNumberOfElectrons");

	if(GTK_IS_WIDGET(labelNumberOfElectrons))
	{
		gint ne = fireflyMolecule.numberOfValenceElectrons - totalCharge;
		gchar buffer[BSIZE];
		sprintf(buffer, _("Number of electrons = %d"),ne);
		gtk_label_set_text(GTK_LABEL(labelNumberOfElectrons),buffer);
	}
}
/**********************************************************************/
static GtkWidget* addComboListToATable(GtkWidget* table,
		gchar** list, gint nlist, gint i, gint j, gint k)
{
	GtkWidget *entry = NULL;
	GtkWidget *combo = NULL;

	combo = create_combo_box_entry(list, nlist, TRUE, -1, -1);

	gtk_table_attach(GTK_TABLE(table),combo,j,j+k,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	entry = GTK_BIN (combo)->child;
	g_object_set_data(G_OBJECT (entry), "Combo",combo);
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeight*0.2),-1);

	return entry;
}
/***********************************************************************************************/
static GtkWidget *addFireFlyChargeToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryCharge = NULL;
	GtkWidget* comboCharge = NULL;
	gint nlistCharge = 1;
	gchar* listCharge[] = {"0"};

	add_label_table(table,_("Charge"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryCharge = addComboListToATable(table, listCharge, nlistCharge, i, 2, 1);
	comboCharge  = g_object_get_data(G_OBJECT (entryCharge), "Combo");
	gtk_widget_set_sensitive(entryCharge, FALSE);

	return comboCharge;
}
/***********************************************************************************************/
static GtkWidget *addFireFlySpinToTable(GtkWidget *table, gint i)
{
	GtkWidget* entrySpinMultiplicity = NULL;
	GtkWidget* comboSpinMultiplicity = NULL;
	gint nlistspinMultiplicity = 1;
	gchar* listspinMultiplicity[] = {"0"};

	add_label_table(table,_("Spin multiplicity"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entrySpinMultiplicity = addComboListToATable(table, listspinMultiplicity, nlistspinMultiplicity, i, 2, 1);
	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entrySpinMultiplicity), "Combo");
	gtk_widget_set_sensitive(entrySpinMultiplicity, FALSE);

	g_signal_connect(G_OBJECT(entrySpinMultiplicity),"changed", G_CALLBACK(changedEntrySpinMultiplicity),NULL);
	return comboSpinMultiplicity;
}
/***********************************************************************************************/
static GtkWidget *addLabelNumberOfElectronsToTable(GtkWidget *table, gint i, GtkWidget *comboCharge)
{
	GtkWidget* labelNumberOfElectrons = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* entryCharge = GTK_BIN(comboCharge)->child;

	labelNumberOfElectrons = gtk_label_new(" ");
	hbox = gtk_hbox_new(0,FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), labelNumberOfElectrons, FALSE, FALSE, 0);
	gtk_table_attach(GTK_TABLE(table),hbox,0,0+3,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);

	g_object_set_data(G_OBJECT (entryCharge), "LabelNumberOfElectrons", labelNumberOfElectrons);
	g_signal_connect(G_OBJECT(entryCharge),"changed", G_CALLBACK(changedEntryCharge),NULL);
	return labelNumberOfElectrons;
}
/***********************************************************************************************/
void createFireFlyChargeMultiplicityFrame(GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* sep;
	GtkWidget* labelNumberOfElectrons;
	GtkWidget* vboxFrame;
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* comboCharge = NULL;
	GtkWidget *table = NULL;
	gint i;

	totalCharge = TotalCharges[0];
	spinMultiplicity=SpinMultiplicities[0];

	table = gtk_table_new(3,5,FALSE);

	frame = gtk_frame_new (_("Charge & Multiplicty"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	i = 0;
	comboCharge = addFireFlyChargeToTable(table, i);
	i = 1;
	comboSpinMultiplicity = addFireFlySpinToTable(table, i);
	i = 2;
	sep = gtk_hseparator_new ();;
	gtk_table_attach(GTK_TABLE(table),sep,0,0+3,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);

	i = 3;
	labelNumberOfElectrons=addLabelNumberOfElectronsToTable(table, i, comboCharge);

	if(GTK_IS_COMBO_BOX(comboCharge))
		g_object_set_data(G_OBJECT (GTK_BIN(comboCharge)->child), "ComboSpinMultiplicity", comboSpinMultiplicity);
	setComboCharge(comboCharge);
	setComboSpinMultiplicity(comboSpinMultiplicity);
	if(GTK_IS_WIDGET(labelNumberOfElectrons))
	{
		gint ne = fireflyMolecule.numberOfValenceElectrons - totalCharge;
		gchar buffer[BSIZE];
		sprintf(buffer, _("Number of electrons = %d"),ne);
		gtk_label_set_text(GTK_LABEL(labelNumberOfElectrons),buffer);
	}

	/* activate sensitivity */
	/*
	if(GTK_IS_WIDGET(comboMethod)) setComboMethod(comboMethod);
	g_object_set_data(G_OBJECT (box), "EntryMethod", GTK_BIN(comboMethod)->child);
	*/
}
/************************************************************************************************************/
void putFireFlyChargeAndSpinInfoInTextEditor()
{
	gchar buffer[BSIZE];
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$CONTRL",-1);
	sprintf(buffer," ICHARG=%d ",totalCharge);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	sprintf(buffer," MULT=%d ",spinMultiplicity);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);
}
