/* MoleculeSE.c */
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
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "../Common/Global.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/Measure.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomXYZ.h"
#include "../Utils/Constants.h"
#include "../Utils/Utils.h"
#include "AtomSE.h"
#include "MoleculeSE.h"
void create_GeomXYZ_from_draw_grometry();

static gboolean** bondedMatrix = NULL;

#define BOHR_TO_ANG  0.52917726

/**********************************************************************/
MoleculeSE newMoleculeSE()
{
	gint i;
	MoleculeSE molecule;

	molecule.nAtoms = 0;
	molecule.totalCharge = 0;
	molecule.spinMultiplicity = 0;
	molecule.atoms = NULL;
	molecule.energy = 0;

	for(i=0;i<3;i++)
		molecule.gradient[i] = NULL;
	molecule.numberOf2Connections = 0;
	for(i=0;i<2;i++)
		molecule.connected2[i] = NULL;
	molecule.numberOf3Connections = 0;
	for(i=0;i<3;i++)
		molecule.connected3[i] = NULL;

	return molecule;

}
/**********************************************************************/
void freeMoleculeSE(MoleculeSE* molecule)
{

	gint i;
        if(molecule->nAtoms<=0)
		return;

	if(molecule->atoms != NULL)
	{
		for(i=0;i<molecule->nAtoms;i++)
		{
			if(molecule->atoms[i].prop.symbol != NULL)
				g_free(molecule->atoms[i].prop.symbol);
			if(molecule->atoms[i].mmType !=NULL )
				g_free(molecule->atoms[i].mmType);
			if(molecule->atoms[i].pdbType !=NULL )
				g_free(molecule->atoms[i].pdbType);
		}

		g_free(molecule->atoms);
		molecule->atoms = NULL;
	}
	molecule->nAtoms = 0;
	molecule->energy = 0;
	molecule->totalCharge = 0;
	molecule->spinMultiplicity = 0;

	for(i=0;i<3;i++)
		if(molecule->gradient[i] != NULL)
		{
			g_free(molecule->gradient[i]);
			molecule->gradient[i] = NULL;
		}
	molecule->numberOf2Connections = 0;
	for(i=0;i<2;i++)
	{
		if(molecule->connected2[i] != NULL)
			g_free(molecule->connected2[i]);
		molecule->connected2[i] = NULL;
	}
	molecule->numberOf3Connections = 0;
	for(i=0;i<3;i++)
	{
		if(molecule->connected3[i] != NULL)
			g_free(molecule->connected3[i]);
		molecule->connected3[i] = NULL;
	}
}
/*****************************************************************************/
static void createBondedMatrix(MoleculeSE* molecule)
{
	gint nAtoms = molecule->nAtoms;
	gint i;
	gint j;

	if(nAtoms<1)
		return;

	bondedMatrix = g_malloc(nAtoms*sizeof(gboolean*));
	for(i=0;i<nAtoms;i++)
		bondedMatrix[i] = g_malloc(nAtoms*sizeof(gboolean));

	for(i=0;i<nAtoms;i++)
	{
		for(j=0;j<nAtoms;j++)
			bondedMatrix[i][j] = FALSE;

		bondedMatrix[i][i] = TRUE;
	}

}
/*****************************************************************************/
static void freeBondedMatrix(MoleculeSE* molecule)
{
	gint nAtoms = molecule->nAtoms;
	gint i;

	if(bondedMatrix == NULL)
	       return;
	for(i=0;i<nAtoms;i++)
		if(bondedMatrix[i] != NULL)
		       	g_free(bondedMatrix[i]);

	g_free(bondedMatrix);
	bondedMatrix = NULL;

}
/*****************************************************************************/
static void updatebondedMatrix(gint a1, gint a2)
{
	bondedMatrix[a1][a2] = TRUE;
	bondedMatrix[a2][a1] = TRUE;

}
/*****************************************************************************/
static gboolean isConnected2(MoleculeSE* molecule,gint i,gint j)
{
	gdouble distance;
	gdouble dij;
	gint k;
	AtomSE a1 = molecule->atoms[i];
	AtomSE a2 = molecule->atoms[j];

	if(molecule->atoms[i].typeConnections)
	{
		 	gint nj = molecule->atoms[j].N-1;
			if(molecule->atoms[i].typeConnections[nj]>0) return TRUE;
			else return FALSE;
	}
	distance = 0;
	for (k=0;k<3;k++)
	{
		dij = a1.coordinates[k]-a2.coordinates[k];
		distance +=dij*dij;
	}
	distance = sqrt(distance)/BOHR_TO_ANG;

	if(distance<(a1.prop.covalentRadii+a2.prop.covalentRadii)) return TRUE;
  	else return FALSE;
}
/*****************************************************************************/
static void set2Connections(MoleculeSE* molecule)
{
	gint i;
	gint j;
	gint k=0;

	k = molecule->nAtoms;
	k = k*(k-1)/2;
	for(i=0;i<2;i++)
		molecule->connected2[i] = g_malloc(k*sizeof(gint));

	k=0;
	for(i=0;i<molecule->nAtoms-1;i++)
		for(j=i+1;j<molecule->nAtoms;j++)
	{
		if(isConnected2(molecule,i,j))
		{
			molecule->connected2[0][k]= i;
			molecule->connected2[1][k]= j;

			updatebondedMatrix(i,j);

			k++;

		}
	}
	molecule->numberOf2Connections = k;
	if(k==0)
		for(i=0;i<2;i++)
		{
			g_free(molecule->connected2[i]);
			molecule->connected2[i] = NULL;
		}
	else
		for(i=0;i<2;i++)
			molecule->connected2[i] = g_realloc(molecule->connected2[i],k*sizeof(gint));
	/* printing for test*/
	/*
	printf("%d 2 connections : \n",molecule->numberOf2Connections);
	for(k=0;k<molecule->numberOf2Connections;k++)
	{

		i =  molecule->connected2[0][k];
		j =  molecule->connected2[1][k];
		printf("%d-%d ",i,j);
	}
	printf("\n");
	*/


}
/*****************************************************************************/
static void permut(gint* a,gint *b)
{
	gint c = *a;
	*a = *b;
	*b = c;
}
/*****************************************************************************/
static gboolean  isConnected3(MoleculeSE* molecule,gint n,gint i,gint j, gint k)
{
	gint c;
	gint a1,a2,a3;
	for(c=0;c<n;c++)
	{
		a1 =  molecule->connected3[0][c];
		a2 =  molecule->connected3[1][c];
		a3 =  molecule->connected3[2][c];
		if(a1==i && a2 == j && a3 == k)
			return TRUE;
	}
	return FALSE;

}
/*****************************************************************************/
static gboolean  connect3(MoleculeSE* molecule,gint n,gint i,gint j, gint k)
{
	if(i>k)permut(&i,&k);
	if(!isConnected3(molecule,n,i,j,k))
	{
		molecule->connected3[0][n]= i;
		molecule->connected3[1][n]= j;
		molecule->connected3[2][n]= k;

		updatebondedMatrix(i,j);
		updatebondedMatrix(i,k);
		updatebondedMatrix(j,k);

		return TRUE;
	}
	return FALSE;

}
/*****************************************************************************/
static void set3Connections(MoleculeSE* molecule)
{
	gint i;
	gint j;
	gint k=0;
	gint l=0;
	gint n=0;

	k = molecule->numberOf2Connections*molecule->nAtoms;
	for(i=0;i<3;i++)
		molecule->connected3[i] = g_malloc(k*sizeof(gint));

	n=0;
	for(k=0;k<molecule->numberOf2Connections;k++)
	{
		i = molecule->connected2[0][k];
		j = molecule->connected2[1][k];
		for(l=0;l<molecule->nAtoms;l++)
		{
			if(l!=i && l!=j)
			{
				if( isConnected2(molecule,i,l))
					if( connect3(molecule,n,l,i,j))
						n++;

				if( isConnected2(molecule,j,l))
					if( connect3(molecule,n,i,j,l))
						n++;
			}
		}

	}
	molecule->numberOf3Connections = n;
	if(n==0)
		for(i=0;i<3;i++)
		{
			g_free(molecule->connected3[i]);
			molecule->connected3[i] = NULL;
		}
	else
		for(i=0;i<3;i++)
			molecule->connected3[i] = g_realloc(molecule->connected3[i],n*sizeof(gint));
	/* printing for test*/
	/*
	printf("%d 3 connections : \n",molecule->numberOf3Connections);
	for(k=0;k<molecule->numberOf3Connections;k++)
	{

		i =  molecule->connected3[0][k];
		j =  molecule->connected3[1][k];
		l =  molecule->connected3[2][k];
		printf("%d-%d-%d ",i,j,l);
	}
	printf("\n");
	*/


}
/*****************************************************************************/
void setConnectionsMoleculeSE(MoleculeSE* molecule)
{
	createBondedMatrix(molecule);

	/* printf("Set Connection\n");*/
	set_text_to_draw(_("Establishing connectivity : 2 connections..."));
	set_statubar_operation_str(_("Establishing connectivity : 2 connections..."));
	drawGeom();
    	while( gtk_events_pending() )
        	gtk_main_iteration();
	set2Connections(molecule);
	set_text_to_draw(_("Establishing connectivity : 3 connections..."));
	set_statubar_operation_str(_("Establishing connectivity : 3 connections..."));

	drawGeom();
	if(StopCalcul) return;
    	while( gtk_events_pending() ) gtk_main_iteration();
	set3Connections(molecule);

	freeBondedMatrix(molecule);
}
/*****************************************************************************/
MoleculeSE createMoleculeSE(GeomDef* geom,gint natoms, gint charge, gint spin, gboolean connections)
{

	gint i;
	MoleculeSE molecule = newMoleculeSE();

	molecule.nAtoms = natoms;
	molecule.atoms = g_malloc(molecule.nAtoms*sizeof(AtomSE));
	for(i=0;i<molecule.nAtoms;i++)
	{
		molecule.atoms[i].prop = prop_atom_get(geom[i].Prop.symbol);
		molecule.atoms[i].coordinates[0] = geom[i].X*BOHR_TO_ANG;
		molecule.atoms[i].coordinates[1] = geom[i].Y*BOHR_TO_ANG;
		molecule.atoms[i].coordinates[2] = geom[i].Z*BOHR_TO_ANG;
		molecule.atoms[i].charge = geom[i].Charge;
		molecule.atoms[i].mmType = g_strdup(geom[i].mmType);
		molecule.atoms[i].pdbType = g_strdup(geom[i].pdbType);
		molecule.atoms[i].residueName = g_strdup(geom[i].Residue);
		molecule.atoms[i].residueNumber = geom[i].ResidueNumber;
		molecule.atoms[i].N = geom[i].N;
		molecule.atoms[i].layer = geom[i].Layer;
		molecule.atoms[i].show = geom[i].show;
		molecule.atoms[i].variable = geom[i].Variable;
		molecule.atoms[i].typeConnections = NULL; 
		if(geom[i].typeConnections)
		{
			gint j;
			molecule.atoms[i].typeConnections = g_malloc(molecule.nAtoms*sizeof(gint));
			for(j=0;j<molecule.nAtoms;j++)
			{
			 	gint nj = geom[j].N-1;
				molecule.atoms[i].typeConnections[nj] = geom[i].typeConnections[nj];
			}
		}
	}
	if(connections) setConnectionsMoleculeSE(&molecule);
	molecule.totalCharge = charge;
	molecule.spinMultiplicity = spin;

	for(i=0;i<3;i++) /* x, y and z derivatives */
		molecule.gradient[i] = g_malloc(molecule.nAtoms*sizeof(gdouble));
	for(i=0;i<3;i++) molecule.dipole[i] = 0.0;

	return molecule;
}
/*****************************************************************************/
MoleculeSE createFromGeomXYZMoleculeSE(gint charge, gint spin, gboolean connections)
{

	gdouble x,y,z;
	gint i;
	MoleculeSE molecule = newMoleculeSE();

	molecule.nAtoms = NcentersXYZ;
	molecule.atoms = g_malloc(molecule.nAtoms*sizeof(AtomSE));

	for(i=0;i<molecule.nAtoms;i++)
	{
		molecule.atoms[i].prop = prop_atom_get(GeomXYZ[i].Symb);
         	if(!test(GeomXYZ[i].X)) x = get_value_variableXYZ(GeomXYZ[i].X);
          	else x = atof(GeomXYZ[i].X);
         	if(!test(GeomXYZ[i].Y)) y = get_value_variableXYZ(GeomXYZ[i].Y);
          	else y = atof(GeomXYZ[i].Y);
         	if(!test(GeomXYZ[i].Z)) z = get_value_variableXYZ(GeomXYZ[i].Z);
          	else z = atof(GeomXYZ[i].Z);

		if(Units==0)
		{
			x *= BOHR_TO_ANG;
			y *= BOHR_TO_ANG;
			z *= BOHR_TO_ANG;
		}
		molecule.atoms[i].coordinates[0] = x;
		molecule.atoms[i].coordinates[1] = y;
		molecule.atoms[i].coordinates[2] = z;
		molecule.atoms[i].charge = atof(GeomXYZ[i].Charge);
		molecule.atoms[i].mmType = g_strdup(GeomXYZ[i].mmType);
		molecule.atoms[i].pdbType = g_strdup(GeomXYZ[i].pdbType);
		molecule.atoms[i].residueName = g_strdup(GeomXYZ[i].Residue);
		molecule.atoms[i].residueNumber = GeomXYZ[i].ResidueNumber;
		molecule.atoms[i].N = i+1;
		if(strstr(GeomXYZ[i].Layer,"Low")) molecule.atoms[i].layer = LOW_LAYER;
		if(strstr(GeomXYZ[i].Layer,"Medium")) molecule.atoms[i].layer = MEDIUM_LAYER;
		else molecule.atoms[i].layer = HIGH_LAYER;
		molecule.atoms[i].show = TRUE;
		molecule.atoms[i].variable = FALSE;
         	if(
			test(GeomXYZ[i].X) ||
			test(GeomXYZ[i].Y) ||
			test(GeomXYZ[i].Z)
		) 
		molecule.atoms[i].variable = TRUE;
		molecule.atoms[i].typeConnections = NULL;
		if(GeomXYZ[i].typeConnections)
		{
			gint j;
			molecule.atoms[i].typeConnections = g_malloc(molecule.nAtoms*sizeof(gint));
			for(j=0;j<molecule.nAtoms;j++)
			{
			 	gint nj = j;
				molecule.atoms[i].typeConnections[nj] = GeomXYZ[i].typeConnections[nj];
			}
		}
	}
	if(connections) setConnectionsMoleculeSE(&molecule);
	molecule.totalCharge = charge;
	molecule.spinMultiplicity = spin;

	for(i=0;i<3;i++) /* x, y and z derivatives */
		molecule.gradient[i] = g_malloc(molecule.nAtoms*sizeof(gdouble));
	for(i=0;i<3;i++) molecule.dipole[i] = 0.0;

	return molecule;
}
/*****************************************************************************/
void redrawMoleculeSE(MoleculeSE* molecule,gchar* str)
{
	gint i;
	gdouble C[3] = {0.0,0.0,0.0};

	Free_One_Geom(geometry0,Natoms);
	Free_One_Geom(geometry ,Natoms);
	Natoms = 0;
	geometry0 = NULL;
	geometry  = NULL;

	Natoms = molecule->nAtoms;
	geometry0 = g_malloc((Natoms)*sizeof(GeomDef));
	geometry  = g_malloc((Natoms)*sizeof(GeomDef));

	for(i=0;i<(gint)Natoms;i++)
	{
		geometry0[i].X = molecule->atoms[i].coordinates[0];
		geometry0[i].Y = molecule->atoms[i].coordinates[1];
		geometry0[i].Z = molecule->atoms[i].coordinates[2];
		geometry0[i].Charge =  molecule->atoms[i].charge;
		geometry0[i].Prop = prop_atom_get(molecule->atoms[i].prop.symbol);
		geometry0[i].pdbType =  g_strdup(molecule->atoms[i].pdbType);
		geometry0[i].mmType =  g_strdup(molecule->atoms[i].mmType);
		geometry0[i].Residue =  g_strdup(molecule->atoms[i].residueName);
		geometry0[i].ResidueNumber =  molecule->atoms[i].residueNumber;
		geometry0[i].show =  molecule->atoms[i].show;
		geometry0[i].Variable =  molecule->atoms[i].variable;
		geometry0[i].Layer =  molecule->atoms[i].layer;
		geometry0[i].N = molecule->atoms[i].N;

		geometry[i].X = molecule->atoms[i].coordinates[0];
		geometry[i].Y = molecule->atoms[i].coordinates[1];
		geometry[i].Z = molecule->atoms[i].coordinates[2];
		geometry[i].Charge =  molecule->atoms[i].charge;
		geometry[i].Prop = prop_atom_get(molecule->atoms[i].prop.symbol);
		geometry[i].pdbType =  g_strdup(molecule->atoms[i].pdbType);
		geometry[i].mmType =  g_strdup(molecule->atoms[i].mmType);
		geometry[i].Residue =  g_strdup(molecule->atoms[i].residueName);
		geometry[i].ResidueNumber =  molecule->atoms[i].residueNumber;
		geometry[i].show =  molecule->atoms[i].show;
		geometry[i].Variable =  molecule->atoms[i].variable;
		geometry[i].Layer =  molecule->atoms[i].layer;
		geometry[i].N = molecule->atoms[i].N;

		C[0] +=  geometry0[i].X;
		C[1] +=  geometry0[i].Y;
		C[2] +=  geometry0[i].Z;


	}
	for(i=0;i<3;i++)
		C[i] /= Natoms;
	/* center */
	for(i=0;i<(gint)Natoms;i++)
	{
		geometry0[i].X -= C[0];
		geometry0[i].Y -= C[1];
		geometry0[i].Z -= C[2];

		geometry[i].X -= C[0];
		geometry[i].Y -= C[1];
		geometry[i].Z -= C[2];
	}
	
	for(i=0;i<(gint)Natoms;i++)
	{
		geometry0[i].X /=BOHR_TO_ANG;
		geometry0[i].Y /=BOHR_TO_ANG;
		geometry0[i].Z /=BOHR_TO_ANG;

		geometry[i].X /=BOHR_TO_ANG;
		geometry[i].Y /=BOHR_TO_ANG;
		geometry[i].Z /=BOHR_TO_ANG;
	}
	unselect_all_atoms();
	set_text_to_draw(str);
	set_statubar_operation_str(str);
	change_of_center(NULL,NULL);
	reset_all_connections();
	create_GeomXYZ_from_draw_grometry();
	RebuildGeom = TRUE;
	drawGeom();

    	while( gtk_events_pending() )
        	gtk_main_iteration();

}
/********************************************************************************/
MoleculeSE copyMoleculeSE(MoleculeSE* m)
{

	gint i;
	gint j;
	MoleculeSE molecule = newMoleculeSE();

	molecule.energy = m->energy;
	molecule.nAtoms = m->nAtoms;
	molecule.totalCharge = m->totalCharge;
	molecule.spinMultiplicity = m->spinMultiplicity;
	if( molecule.nAtoms>0) molecule.atoms = g_malloc(molecule.nAtoms*sizeof(AtomSE));
	for(i=0;i<molecule.nAtoms;i++)
	{
		molecule.atoms[i].prop = prop_atom_get(m->atoms[i].prop.symbol);
		for(j=0;j<3;j++) molecule.atoms[i].coordinates[j] = m->atoms[i].coordinates[j];
		molecule.atoms[i].charge = m->atoms[i].charge;
		molecule.atoms[i].mmType = g_strdup(m->atoms[i].mmType);
		molecule.atoms[i].pdbType = g_strdup(m->atoms[i].pdbType);
		molecule.atoms[i].residueName = g_strdup(m->atoms[i].residueName);
		molecule.atoms[i].residueNumber = m->atoms[i].residueNumber;
		molecule.atoms[i].N = m->atoms[i].N;
		molecule.atoms[i].layer = m->atoms[i].layer;
		molecule.atoms[i].show = m->atoms[i].show;
		molecule.atoms[i].variable = m->atoms[i].variable;
	       if(m->atoms[i].typeConnections)
                {
                        gint j;
                        molecule.atoms[i].typeConnections = g_malloc(molecule.nAtoms*sizeof(gint));
                        for(j=0;j<molecule.nAtoms;j++)
                        {
                                gint nj = m->atoms[j].N-1;
                                molecule.atoms[i].typeConnections[nj] = m->atoms[i].typeConnections[nj];
                        }
                }
	}


	if(molecule.nAtoms>0)
	for(j=0;j<3;j++) /* x, y and z derivatives */
	{
		molecule.gradient[j] = g_malloc(molecule.nAtoms*sizeof(gdouble));
		for(i=0;i<molecule.nAtoms;i++)
		molecule.gradient[j][i] = m->gradient[j][i];
	}
	for(i=0;i<3;i++) molecule.dipole[i] = m->dipole[i];

	return molecule;
}
/******************************************************************************/
static void save_atom_hin_file(FILE* file, char* name, int atomNumber, char* atomPDBType, char* atomMMType,
                double x, double y, double z, char* symbol, double charge,
                int N, int* connection, int* connectionType)
{
        int i;
        fprintf(file,"%s %d ",name,atomNumber);
        fprintf(file,"%s ",atomPDBType);
        fprintf(file,"%s ",symbol);
        fprintf(file,"%s - ",atomMMType);
        fprintf(file,"%0.14f ",charge);
        fprintf(file,"%0.14f ",x);
        fprintf(file,"%0.14f ",y);
        fprintf(file,"%0.14f ",z);
        if(N>0)
        {
                fprintf(file,"%d ",N);
                for(i=0;i<N;i++)
                {
                        if(connectionType[i]==3) fprintf(file,"%d t ",connection[i]);
                        else if(connectionType[i]==2) fprintf(file,"%d d ",connection[i]);
                        else fprintf(file,"%d s ",connection[i]);
                }
        }
        fprintf(file,"\n");
}
/******************************************************************************/
gboolean saveMoleculeSEHIN(MoleculeSE* mol, char* fileName)
{
	int i,n,k;
 	FILE* file = fopen(fileName, "w");
	int nAtoms;
	AtomSE* atoms;
	int* connection;
	int* connectionType;
	if(!file) return FALSE;
	nAtoms = mol->nAtoms;
	atoms = mol->atoms;

	fprintf(file,"forcefield Amber99\n");
	fprintf(file,"sys 0 0 1\n");
	fprintf(file,"view 40 0.1272 55 15 0.247224 0.3713666 0.8949677 -0.8641704 0.5022867 0.0302929 -0.4382806 -0.7808937 0.4451014 6.191 0.64575 -54.754\n");
	fprintf(file,"seed -1108\n");
	fprintf(file,"mol 1\n");

	connection = malloc(nAtoms*sizeof(int));
	connectionType = malloc(nAtoms*sizeof(int));

	for(i=0;i<nAtoms;i++)
	{
		n = 0;
		if(atoms[i].typeConnections)
		for(k=0;k<nAtoms;k++)
		{
			if(i==k) continue;
			if(atoms[i].typeConnections[k]>0)
			{
				connection[n] = k+1;
				connectionType[n] = atoms[i].typeConnections[k];
				n++;
			}
		}
		save_atom_hin_file(file,"ATOM",i+1,atoms[i].pdbType, atoms[i].mmType,
		atoms[i].coordinates[0], atoms[i].coordinates[1], atoms[i].coordinates[2],
		atoms[i].prop.symbol, atoms[i].charge,n,connection, connectionType);
	}
	fprintf(file,"endmol 1\n");
	fclose(file);
	free(connection);
	free(connectionType);
	return TRUE;
}

/*****************************************************************************/
gboolean saveMoleculeSEMol2(MoleculeSE* mol, char* fileName)
{
	int i,n,j;
 	FILE* file = fopen(fileName, "w");
	if(!file) return FALSE;
	n = 0;
	for(i=0;i<mol->nAtoms;i++)
        if(mol->atoms[i].typeConnections)
        for(j=i+1;j<mol->nAtoms;j++)
                if(mol->atoms[i].typeConnections[j]) n++;

	fprintf(file,"@<TRIPOS>MOLECULE\n");
	fprintf(file,"MOL2  : Made in CChemI. mol2 file\n");
	fprintf(file," %10d %10d %10d\n",mol->nAtoms,n,1);
	fprintf(file,"SMALL\n");
	fprintf(file,"GASTEIGER\n");
	fprintf(file,"\n");
	fprintf(file,"@<TRIPOS>ATOM\n");
      	for (i=0;i<mol->nAtoms;i++)
	{
       		fprintf(file,"%7d%1s%-6s%12.4f%10.4f%10.4f%1s%-5s%4d%1s%-8s%10.4f\n",
               		i+1,"",mol->atoms[i].prop.symbol,
				mol->atoms[i].coordinates[0],
				mol->atoms[i].coordinates[1],
				mol->atoms[i].coordinates[2],
				"",mol->atoms[i].prop.symbol,1," ","LIG111",mol->atoms[i].charge);
	}
	fprintf(file,"@<TRIPOS>BOND\n");
	n = 0;
	for(i=0;i<mol->nAtoms;i++)
        if(mol->atoms[i].typeConnections)
        for(j=i+1;j<mol->nAtoms;j++)
                if(mol->atoms[i].typeConnections[j]) 
		{
			n++;
			fprintf(file,"%6d%6d%6d%3s%2d\n",n+1, i+1, j+1, "",mol->atoms[i].typeConnections[j]);
		}


	fclose(file);
	return TRUE;
}
/*****************************************************************************/
void computeMoleculeSEDipole(MoleculeSE* mol)
{
        int i,k;
        for(k=0;k<3;k++) mol->dipole[k] = 0;
        for(i=0;i<mol->nAtoms;i++)
        for(k=0;k<3;k++)
                mol->dipole[k] += mol->atoms[i].charge*mol->atoms[i].coordinates[k];
        for(k=0;k<3;k++) mol->dipole[k] *= ANG_TO_BOHR*AUTODEB;
}

/********************************************************************************/
void readGeomMoleculeSEFromOpenBabelOutputFile(MoleculeSE* mol, char* fileName, int numgeometry)
{
	FILE* file = NULL;
	char buffer[BSIZE];
	char* pdest = NULL;
	//char* energyTag = "TOTAL ENERGY =";
	char* energyTag = "FINAL ENERGY:";
	char* geomTag = "Geometry";
	char* gradTag = "Gradients:";
	double dum;

	printf("Read geom from %s\n",fileName);
 	file = fopen(fileName, "r");
	if(!file) return;
	 while(!feof(file))
	 {
		if(!fgets(buffer,BSIZE,file))break;
		pdest = strstr( buffer, energyTag);
		if(pdest &&sscanf(pdest+strlen(energyTag)+1,"%lf",&mol->energy)==1)
		{
			if(strstr(pdest,"kJ")) mol->energy /= KCALTOKJ;
			break;
		}
	 }
	 while(!feof(file))
	 {
		if(!fgets(buffer,BSIZE,file))break;
		if(strstr(buffer, geomTag))
		{
			int i;
			for(i=0;i<mol->nAtoms;i++)
			{
				if(!fgets(buffer,BSIZE,file))break;
				//printf("%s\n",buffer);
				if(sscanf(buffer,"%lf %lf %lf %lf",
					&dum,
					&mol->atoms[i].coordinates[0],
					&mol->atoms[i].coordinates[1],
					&mol->atoms[i].coordinates[2]
					)!=4) break;
			}
			break;
		}
	 }
	 while(!feof(file))
	 {
		if(!fgets(buffer,BSIZE,file))break;
		if(strstr(buffer, gradTag))
		{
			int i;
			for(i=0;i<mol->nAtoms;i++)
			{
				if(!fgets(buffer,BSIZE,file))break;
				//printf("%s\n",buffer);
				if(sscanf(buffer,"%lf %lf %lf",
					&mol->gradient[0][i],
					&mol->gradient[1][i],
					&mol->gradient[2][i]
					)!=3) break;
			}
			break;
		}
	 }
	/*
	{
		int i;
		for(i=0;i<mol->nAtoms;i++) mol->atoms[i].typeConnections = NULL; 
	}
	*/
	fclose(file);
}
/********************************************************************************/
gint get_connections_one_atom(gchar* t, gint nAtoms, gint ibeg, gint* connections)
{
        gint k;
        gint nc;
        gint nj;
        gchar** ssplit = NULL;
        gint nA = 0;
        /* int ibeg = 12;*/
        for(k=0;k<nAtoms;k++) connections[k] = 0;
        ssplit = gab_split(t);
        nA = 0;
        while(ssplit && ssplit[nA]!=NULL) nA++;
        if(nA<ibeg)
        {
                gab_strfreev(ssplit);
                return 0;
        }
        nc = atoi(ssplit[ibeg-1]);
        for(k=0;k<2*nc;k+=2)
        {
                if(!ssplit[ibeg+k]) break;
                if(!ssplit[ibeg+k+1]) break;
                nj = atoi(ssplit[ibeg+k]);
                connections[nj-1] = atoi(ssplit[ibeg+k+1]);
        }

        gab_strfreev(ssplit);

        return 1;
}
/********************************************************************************/
gboolean readGeometryFromGenericOutputFile(MoleculeSE* molecule, char* namefile)
{
	FILE* file = NULL;
	static gchar *t = NULL; 
	gboolean Ok = FALSE;
	gint n,ic,is;
	MoleculeSE mols = newMoleculeSE();
	MoleculeSE* mol = &mols;
#define SZ 50
	gchar symbol[SZ];
	gchar mmType[SZ];
	gchar pdbType[SZ];
	gchar residueName[SZ];
	gdouble X,Y,Z;
	gdouble charge;
	gint layer;
	gint i,j,l;
	gchar* pos;

	if(!namefile) 
	{
		printf("Sorry I cannot read geometry namefile = NULL\n");
		exit(1);
	}
	if(t==NULL) t = malloc(BSIZE*sizeof(char));
	file = fopen(namefile,"rb");
	if(!file)
	{
		printf("Sorry I cannot open %s file\n",namefile);
		exit(1);
	}
	rewind(file);
	while(!feof(file))
  	{
    		if(!fgets(t,BSIZE, file)) break;
		deleteFirstSpaces(t);
		if(t[0]=='#') continue;
		uppercase(t);
		pos = strstr(t,"GEOMETRY");
		if(pos)
		{ 
			while(!feof(file))
			{
    				if(!fgets(t,BSIZE, file)) break;
				deleteFirstSpaces(t);
				if(t[0]=='#') continue;
				break;
			}
			if(3==sscanf(t,"%d%d%d",&n,&ic,&is) && n>0 && is>0)
			{
				/*
				printf("Mult = %d\n",is);
				printf("nAtoms = %d\n",n);
				*/
				mol->nAtoms = n;
				mol->spinMultiplicity = is;
				mol->totalCharge = ic;
				mol->atoms = g_malloc(mol->nAtoms*sizeof(AtomSE));
				for(i=0; i<mol->nAtoms; i++) mol->atoms[i].typeConnections = g_malloc(mol->nAtoms*sizeof(gint));
				Ok = TRUE;
			}
			break;
		}
	}
	if(!Ok)
	{
		printf("Sorry I cannot read geometry from %s file\n",namefile);
		exit(1);
	}
	for(i=0; i<mol->nAtoms; i++)
	{
			int variable = 0;
			gint ibeg = 12;
			if(!fgets(t,BSIZE,file))
			{
				printf("Sorry I cannot read geometry from %s file.\n",namefile);
				exit(1);
			}
			deleteFirstSpaces(t);
			if(t[0]=='#') { i--;continue;}
    			sscanf(t,"%s %s %s %s %d %lf %d %d %lf %lf %lf",
					symbol,mmType,pdbType,residueName, 
					&mol->atoms[i].residueNumber,
					&charge,&layer,&variable,&X,&Y,&Z);
			symbol[0]=toupper(symbol[0]);
			l=strlen(symbol);
			if (l==2) symbol[1]=tolower(symbol[1]);

			mol->atoms[i].prop = prop_atom_get(symbol);
			mol->atoms[i].mmType=strdup(mmType);
			mol->atoms[i].pdbType=strdup(pdbType);
			mol->atoms[i].residueName=strdup(residueName);
			mol->atoms[i].N=i+1;
			mol->atoms[i].layer=layer;
			mol->atoms[i].variable=variable;
			mol->atoms[i].show=TRUE;
			mol->atoms[i].coordinates[0] = X;
			mol->atoms[i].coordinates[1] = Y;
			mol->atoms[i].coordinates[2] = Z;
			mol->atoms[i].charge = charge;
			if(!get_connections_one_atom(t, mol->nAtoms, ibeg, mol->atoms[i].typeConnections))
			{
				printf("Sorry I cannot read the connection for atom # %d from the %s file.\n",i+1,namefile);
				exit(1);
			}
	}
	fclose(file);
	if(mol->nAtoms>0)
	for(j=0;j<3;j++) /* x, y and z derivatives */
	{
		mol->gradient[j] = g_malloc(mol->nAtoms*sizeof(gdouble));
		for(i=0;i<mol->nAtoms;i++) mol->gradient[j][i] = 0.0;
	}
	for(i=0;i<3;i++) mol->dipole[i] = 0.0;
	/*
	printf("Begin copyMol\n");
	*molecule = copyMoleculeSE(mol);
	printf("End copyMol\n");
	*/
	if(molecule->nAtoms==mol->nAtoms)
	{
		for(j=0;j<3;j++) 
		for(i=0;i<mol->nAtoms;i++) 
			molecule->atoms[i].coordinates[j] = mol->atoms[i].coordinates[j];
		
	}
	freeMoleculeSE(mol);
	return TRUE;
}
/*****************************************************************************/
gboolean addGeometryMoleculeSEToGabedit(MoleculeSE* molecule,FILE* file)
{
	int j,k;
	int nc;

	if(!molecule) return FALSE;
	fprintf(file,"%d %d %d\n",molecule->nAtoms, molecule->totalCharge, molecule->spinMultiplicity);
	for(j=0;j<molecule->nAtoms;j++)
	{
		nc = 0;
		for(k=0;k<molecule->nAtoms;k++) if(molecule->atoms[j].typeConnections[k]>0) nc++;
		fprintf(file," %s %s %s %s %d %0.12lf %d %d %0.12lf %0.12lf %0.12lf %d ", 
				molecule->atoms[j].prop.symbol,
				molecule->atoms[j].mmType,
				molecule->atoms[j].pdbType,
				molecule->atoms[j].residueName,
				molecule->atoms[j].residueNumber,
				molecule->atoms[j].charge,
				molecule->atoms[j].layer,
				molecule->atoms[j].variable,
				molecule->atoms[j].coordinates[0],
				molecule->atoms[j].coordinates[1],
				molecule->atoms[j].coordinates[2],
				nc
				);
		for(k=0;k<molecule->nAtoms;k++) 
		{
	 		int nk = molecule->atoms[k].N-1;
			if(molecule->atoms[j].typeConnections[nk]>0) 
			fprintf(file," %d %d", nk+1,molecule->atoms[j].typeConnections[nk]);
		}
        	fprintf(file," GRADIENT %0.14f %0.14f %0.14f",molecule->gradient[0][j], molecule->gradient[1][j],molecule->gradient[2][j]);
		fprintf(file,"\n");
	}
	return TRUE;

}
/*****************************************************************************/
gboolean addMoleculeSEToFile(MoleculeSE* molecule,FILE* file)
{
	int j,k;
	int nc;

	if(!molecule) return FALSE;

	fprintf(file,"Geometry\n");
	return addGeometryMoleculeSEToGabedit(molecule,file);
}
/*****************************************************************************/
gboolean saveMoleculeSETypeSave(MoleculeSE* molecule, char* fileName, char* typeSave)
{
	FILE* file = NULL;
	int j;
	int form = 1;

	printf("Save molecule in %s\n",fileName);
	if(!molecule) return FALSE;

 	file = fopen(fileName, typeSave);

	if(!file) return FALSE;

	fprintf(file,"[Gabedit Format]\n");
	fprintf(file,"[GEOCONV]\n");
	fprintf(file,"energy\n");
	fprintf(file,"%f\n",molecule->energy);
	fprintf(file,"max-force\n");
	fprintf(file,"%f\n",0.0);
	fprintf(file,"rms-force\n");
	fprintf(file,"%f\n",0.0);

	fprintf(file,"\n");
	fprintf(file,"[GEOMETRIES]\n");
	{
		fprintf(file,"%d\n",molecule->nAtoms);
		fprintf(file,"\n");
		for(j=0;j<molecule->nAtoms;j++)
		fprintf(file," %s %0.8f %0.8f %0.8f\n", 
				molecule->atoms[j].prop.symbol,
				molecule->atoms[j].coordinates[0],
				molecule->atoms[j].coordinates[1],
				molecule->atoms[j].coordinates[2]
				);
	}
	fprintf(file,"\n");
	fprintf(file,"[GEOMS] %d\n",form);
	fprintf(file,"%d 3\n",1);
	fprintf(file,"energy kcal/mol 1\n");
	fprintf(file,"deltaE K 1\n");
	fprintf(file,"Dipole Debye 3\n");
	//molecule->klass->computeDipole(molecule);
	{
		fprintf(file,"%0.14f\n",molecule->energy);
		fprintf(file,"0\n");
		fprintf(file,"%0.14f %0.14f %0.14f\n",molecule->dipole[0],molecule->dipole[1],molecule->dipole[2]);
		addGeometryMoleculeSEToGabedit(molecule,file);
	}
	//addVibrationToFile(molecule, file);
	fclose(file);
	return TRUE;

}
/*****************************************************************************/
gboolean saveMoleculeSE(MoleculeSE* molecule, char* fileName)
{
	return saveMoleculeSETypeSave(molecule, fileName, "w");
}
/*****************************************************************************/
gdouble getGradientNormMoleculeSE(MoleculeSE* molecule)
{
	int j,k;
	gdouble norm = 0;

	if(!molecule) return -1.0;
	for(j=0;j<molecule->nAtoms;j++)
	for(k=0;k<3;k++)
		norm += molecule->gradient[k][j]*molecule->gradient[k][j];
	return sqrt(norm);

}
