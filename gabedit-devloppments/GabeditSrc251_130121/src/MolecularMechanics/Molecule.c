/* Molecule.c */
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

#include "../Common/Global.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/Measure.h"
#include "Atom.h"
#include "Molecule.h"
void rafresh_window_geom();
void create_GeomXYZ_from_draw_grometry();

static gboolean** bondedMatrix = NULL;

#define BOHR_TO_ANG  0.52917726

/**********************************************************************/
Molecule newMolecule()
{
	gint i;
	Molecule molecule;

	molecule.nAtoms = 0;
	molecule.atoms = NULL;
	molecule.energy = 0;
	molecule.numberOf2Connections = 0;
	for(i=0;i<2;i++)
		molecule.connected2[i] = NULL;
	molecule.numberOf3Connections = 0;
	for(i=0;i<3;i++)
		molecule.connected3[i] = NULL;
	molecule.numberOf4Connections = 0;
	for(i=0;i<4;i++)
		molecule.connected4[i] = NULL;

	molecule.numberOfNonBonded = 0;
	for(i=0;i<2;i++)
		molecule.nonBonded[i] = NULL;

	for(i=0;i<3;i++)
		molecule.gradient[i] = NULL;

	return molecule;

}
/**********************************************************************/
void freeMolecule(Molecule* molecule)
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
			if(molecule->atoms[i].typeConnections !=NULL )
				g_free(molecule->atoms[i].typeConnections);
		}

		g_free(molecule->atoms);
		molecule->atoms = NULL;
	}
	molecule->nAtoms = 0;
	molecule->energy = 0;
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
	molecule->numberOf4Connections = 0;
	for(i=0;i<4;i++)
	{
		if(molecule->connected4[i] != NULL)
			g_free(molecule->connected4[i]);
		molecule->connected4[i] = NULL;
	}

	for(i=0;i<3;i++)
		if(molecule->gradient[i] != NULL)
		{
			g_free(molecule->gradient[i]);
			molecule->gradient[i] = NULL;
		}
}
/*****************************************************************************/
void createBondedMatrix(Molecule* molecule)
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
void freeBondedMatrix(Molecule* molecule)
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
void updatebondedMatrix(gint a1, gint a2)
{
	bondedMatrix[a1][a2] = TRUE;
	bondedMatrix[a2][a1] = TRUE;

}
/*****************************************************************************/
gboolean isConnected2(Molecule* molecule,gint i,gint j)
{
	gdouble distance;
	gdouble dij;
	gint k;
	AtomMol a1 = molecule->atoms[i];
	AtomMol a2 = molecule->atoms[j];

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
void set2Connections(Molecule* molecule)
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
gboolean  isConnected3(Molecule* molecule,gint n,gint i,gint j, gint k)
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
gboolean  connect3(Molecule* molecule,gint n,gint i,gint j, gint k)
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
void set3Connections(Molecule* molecule)
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
gboolean  isConnected4(Molecule* molecule,gint n,gint i,gint j, gint k,gint l)
{
	gint c;
	gint a1,a2,a3,a4;
	for(c=0;c<n;c++)
	{
		a1 =  molecule->connected4[0][c];
		a2 =  molecule->connected4[1][c];
		a3 =  molecule->connected4[2][c];
		a4 =  molecule->connected4[3][c];

		if(a1==i && a2 == j && a3 == k && a4 == l)
			return TRUE;
	}
	return FALSE;

}
/*****************************************************************************/
gboolean  connect4(Molecule* molecule,gint n,gint i,gint j, gint k,gint l)
{
	if(i>l)
	{
		permut(&i,&l);
		permut(&j,&k);
	}
	if(!isConnected4(molecule,n,i,j,k,l))
	{
		molecule->connected4[0][n]= i;
		molecule->connected4[1][n]= j;
		molecule->connected4[2][n]= k;
		molecule->connected4[3][n]= l;

		updatebondedMatrix(i,j);
		updatebondedMatrix(i,k);
		updatebondedMatrix(i,l);
		updatebondedMatrix(j,k);
		updatebondedMatrix(j,l);
		updatebondedMatrix(k,l);

		return TRUE;
	}
	return FALSE;

}
/*****************************************************************************/
void set4Connections(Molecule* molecule)
{
	gint i;
	gint j;
	gint k=0;
	gint m=0;
	gint l=0;
	gint n=0;

	k = molecule->numberOf3Connections*molecule->nAtoms;
	for(i=0;i<4;i++)
		molecule->connected4[i] = g_malloc(k*sizeof(gint));

	n=0;
	for(k=0;k<molecule->numberOf3Connections;k++)
	{
		i = molecule->connected3[0][k];
		j = molecule->connected3[1][k];
		m = molecule->connected3[2][k];
		for(l=0;l<molecule->nAtoms;l++)
		{
			/* a refaire voir Set3Co */
			if(l!=i && l!=j && l!= m)
			{
				if( isConnected2(molecule,i,l))
					if(connect4(molecule,n,l,i,j,m))
						n++;
				if( isConnected2(molecule,m,l))
					if(connect4(molecule,n,i,j,m,l))
						n++;
			}
		}

	}
	molecule->numberOf4Connections = n;
	if(n==0)
		for(i=0;i<4;i++)
		{
			g_free(molecule->connected4[i]);
			molecule->connected4[i] = NULL;
		}
	else
		for(i=0;i<4;i++)
			molecule->connected4[i] = g_realloc(molecule->connected4[i],n*sizeof(gint));
	/* printing for test*/
	/*
	printf("%d 4 connections : \n",molecule->numberOf4Connections);
	for(k=0;k<molecule->numberOf4Connections;k++)
	{

		i =  molecule->connected4[0][k];
		j =  molecule->connected4[1][k];
		l =  molecule->connected4[2][k];
		m =  molecule->connected4[3][k];
		printf("%d-%d-%d-%d ",i,j,l,m);
	}
	printf("\n");
	*/


}
/*****************************************************************************/
void setNonBondedConnections(Molecule* molecule)
{
	gint i;
	gint j;
	gint k;
	gint numberOfNonBonded =0;
	gint numberOfAtoms = molecule->nAtoms;
	gint *nonBonded[2];

	k = numberOfAtoms;
	k = k*(k-1)/2;
	for(i=0;i<2;i++)
		nonBonded[i] = g_malloc(k*sizeof(gint));

	/* list for all nonbonded connections */
	numberOfNonBonded = 0;
	for (  i = 0; i < numberOfAtoms; i++ )
		for (  j = i + 1; j < numberOfAtoms; j++ )
		{
			if ( !bondedMatrix[ i ][ j ] )
			{
				nonBonded[0][numberOfNonBonded] = i;
				nonBonded[1][numberOfNonBonded] = j;
				numberOfNonBonded++;
			}
		}
	if(numberOfNonBonded==0)
		for(i=0;i<2;i++)
		{
			g_free(nonBonded[i]);
			nonBonded[i] = NULL;
		}
	else
		for(i=0;i<2;i++)
		{
			nonBonded[i] = g_realloc(nonBonded[i],numberOfNonBonded*sizeof(gint));
		}
	molecule->numberOfNonBonded = numberOfNonBonded;
	for(i=0;i<2;i++)
		molecule->nonBonded[i] = nonBonded[i];
	/* printing for test*/
	/*
	printf("%d nonBonded connections : \n",molecule->numberOfNonBonded);
	for(k=0;k<molecule->numberOfNonBonded;k++)
	{

		i =  molecule->nonBonded[0][k];
		j =  molecule->nonBonded[1][k];
		printf("%d-%d ",i,j);
	}
	printf("\n");
	*/
}
/*****************************************************************************/
void setConnections(Molecule* molecule)
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
	if(StopCalcul)
		return;
    	while( gtk_events_pending() )
        	gtk_main_iteration();
	set3Connections(molecule);
	set_text_to_draw(_("Establishing connectivity : 4 connections..."));
	set_statubar_operation_str(_("Establishing connectivity : 4 connections..."));
	drawGeom();
	if(StopCalcul)
		return;
    	while( gtk_events_pending() )
        	gtk_main_iteration();
	set4Connections(molecule);

	set_text_to_draw(_("Establishing connectivity : non bonded ..."));
	set_statubar_operation_str(_("Establishing connectivity : non bonded ..."));
	drawGeom();
	if(StopCalcul)
		return;
    	while( gtk_events_pending() )
        	gtk_main_iteration();
	setNonBondedConnections(molecule);

	freeBondedMatrix(molecule);
}
/*****************************************************************************/
Molecule createMolecule(GeomDef* geom,gint natoms,gboolean connections)
{

	gint i;
	Molecule molecule = newMolecule();

	molecule.nAtoms = natoms;
	molecule.atoms = g_malloc(molecule.nAtoms*sizeof(AtomMol));
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
		molecule.atoms[i].layer = geom[i].Layer;
		molecule.atoms[i].show = geom[i].show;
		molecule.atoms[i].variable = geom[i].Variable;
		molecule.atoms[i].N = geom[i].N;
	
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
	if(connections)
		setConnections(&molecule);

	for(i=0;i<3;i++) /* x, y and z derivatives */
		molecule.gradient[i] = g_malloc(molecule.nAtoms*sizeof(gdouble));
	/* if all freezed, set all to variable */
	{
		gint j = 0;
		for(i=0;i<molecule.nAtoms;i++)
			if(!molecule.atoms[i].variable) j++;
		if(j==molecule.nAtoms)
		for(i=0;i<molecule.nAtoms;i++)
			molecule.atoms[i].variable = TRUE;
	}

	return molecule;
}
/*****************************************************************************/
void redrawMolecule(Molecule* molecule,gchar* str)
{
	gint i;
	gint j;
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
		geometry0[i].typeConnections = NULL;

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
		geometry[i].typeConnections = NULL;

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

	if(molecule->atoms[0].typeConnections)
	{
		for(i=0;i<(gint)Natoms;i++)
		{
			geometry[i].typeConnections = g_malloc(Natoms*sizeof(gint));
			geometry0[i].typeConnections = g_malloc(Natoms*sizeof(gint));
			if(molecule->atoms[i].typeConnections)
			{
				for(j=0;j<(gint)Natoms;j++)
				{
			 		gint nj = geometry[j].N-1;
					geometry[i].typeConnections[nj] = molecule->atoms[i].typeConnections[nj];
					geometry0[i].typeConnections[nj] = molecule->atoms[i].typeConnections[nj];
				}
			}
			else
			{
				for(j=0;j<(gint)Natoms;j++)
				{
			 		gint nj = geometry[j].N-1;
					geometry[i].typeConnections[nj] = 0;
					geometry0[i].typeConnections[nj] = 0;
				}
			}
		}
		reset_hydrogen_bonds();
	}
	else
	{
		reset_all_connections();
	}
	unselect_all_atoms();
	set_text_to_draw(str);
	set_statubar_operation_str(str);
	change_of_center(NULL,NULL);

	create_GeomXYZ_from_draw_grometry();
	rafresh_window_geom();

    	while( gtk_events_pending() )
        	gtk_main_iteration();

}
/********************************************************************************/
Molecule copyMolecule(Molecule* m)
{

	gint i;
	gint j;
	gint k;
	Molecule molecule = newMolecule();

	molecule.energy = m->energy;
	molecule.nAtoms = m->nAtoms;
	if( molecule.nAtoms>0) molecule.atoms = g_malloc(molecule.nAtoms*sizeof(AtomMol));
	for(i=0;i<molecule.nAtoms;i++)
	{
		molecule.atoms[i].prop = prop_atom_get(m->atoms[i].prop.symbol);
		for(j=0;j<3;j++) molecule.atoms[i].coordinates[j] = m->atoms[i].coordinates[j];
		molecule.atoms[i].charge = m->atoms[i].charge;
		molecule.atoms[i].mmType = g_strdup(m->atoms[i].mmType);
		molecule.atoms[i].pdbType = g_strdup(m->atoms[i].pdbType);
		molecule.atoms[i].residueName = g_strdup(m->atoms[i].residueName);
		molecule.atoms[i].residueNumber = m->atoms[i].residueNumber;
		molecule.atoms[i].layer = m->atoms[i].layer;
		molecule.atoms[i].show = m->atoms[i].show;
		molecule.atoms[i].variable = m->atoms[i].variable;
		molecule.atoms[i].N = m->atoms[i].N;

		molecule.atoms[i].typeConnections = NULL; 
		if(m->atoms[i].typeConnections)
		{
			gint j;
			molecule.atoms[i].typeConnections = g_malloc(molecule.nAtoms*sizeof(gint));
			for(j=0;j<molecule.nAtoms;j++)
				molecule.atoms[i].typeConnections[j] = m->atoms[i].typeConnections[j];
		}
	}

	molecule.numberOf2Connections = m->numberOf2Connections;
	k = molecule.numberOf2Connections;
	if(k>0)
	for(j=0;j<2;j++)
	{
		molecule.connected2[j] = g_malloc(k*sizeof(gint));
		for(i=0;i<k;i++) molecule.connected2[j][i] = m->connected2[j][i];
	}
	molecule.numberOf3Connections = m->numberOf3Connections;
	k = molecule.numberOf3Connections;
	if(k>0)
	for(j=0;j<3;j++)
	{
		molecule.connected3[j] = g_malloc(k*sizeof(gint));
		for(i=0;i<k;i++) molecule.connected3[j][i] = m->connected3[j][i];
	}
	molecule.numberOf4Connections = m->numberOf4Connections;
	k = molecule.numberOf4Connections;
	if(k>0)
	for(j=0;j<4;j++)
	{
		molecule.connected4[j] = g_malloc(k*sizeof(gint));
		for(i=0;i<k;i++) molecule.connected4[j][i] = m->connected4[j][i];
	}

	molecule.numberOfNonBonded = m->numberOfNonBonded;
	k = molecule.numberOfNonBonded;
	if(k>0)
	for(j=0;j<2;j++)
	{
		molecule.nonBonded[j] = g_malloc(k*sizeof(gint));
		for(i=0;i<k;i++) molecule.nonBonded[j][i] = m->nonBonded[j][i];
	}

	if(molecule.nAtoms>0)
	for(j=0;j<3;j++) /* x, y and z derivatives */
	{
		molecule.gradient[j] = g_malloc(molecule.nAtoms*sizeof(gdouble));
		for(i=0;i<molecule.nAtoms;i++)
		molecule.gradient[j][i] = m->gradient[j][i];
	}

	return molecule;
}
