/* GabeditSPG.c */
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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "../../spglib/spglib.h"
#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Crystallography/Crystallo.h"


gboolean crystalloGetNiggli(GList* atoms, gdouble newTv[][3], gdouble symprec)
{
	gint i,j;
	double lattice[3][3];
	gint nTv = crystalloGetTv(atoms, newTv);
	gboolean ok = FALSE;
	if(nTv<3) return ok;
	for(i=0;i<3;i++) for(j=0;j<3;j++) lattice[i][j] = newTv[j][i];
	if (0 != spg_niggli_reduce(lattice, (const double) symprec))
	{
		fprintf(stderr," Ok new Tv vector\n");
		fprintf(stderr," oldTV =\n"); for(i=0;i<3;i++) { for(j=0;j<3;j++) fprintf(stderr,"%f ",newTv[i][j]); fprintf(stderr,"\n"); }

		for(i=0;i<3;i++) for(j=0;j<3;j++) newTv[j][i] = lattice[i][j];
		fprintf(stderr," newTV =\n"); for(i=0;i<3;i++) { for(j=0;j<3;j++) fprintf(stderr,"%f ",newTv[i][j]); fprintf(stderr,"\n"); }
		ok = TRUE;
	}
	return ok;
}
gboolean crystalloGetDelaunay(GList* atoms, gdouble newTv[][3], gdouble symprec)
{
	gint i,j;
	double lattice[3][3];
	gint nTv = crystalloGetTv(atoms, newTv);
	gboolean ok = FALSE;
	if(nTv<3) return ok;
	for(i=0;i<3;i++) for(j=0;j<3;j++) lattice[i][j] = newTv[j][i];
	if (0 != spg_delaunay_reduce(lattice, (const double) symprec))
	{
		for(i=0;i<3;i++) for(j=0;j<3;j++) newTv[j][i] = lattice[i][j];
		ok = TRUE;
	}
	return ok;
}
/*
gboolean crystalloGetPrimitive(GList* atoms, gdouble newTv[][3], gdouble symprec)
{
	double (*position)[3];
	int* types = NULL;
	gint i,j;
	double lattice[3][3];
	gint nTv = crystalloGetTv(atoms, newTv);
	GList *l = NULL;
	int nAtoms = 0;
	gboolean ok = FALSE;

	if(nTv<3) return ok;
	for(i=0;i<3;i++) for(j=0;j<3;j++) lattice[i][j] = newTv[j][i];

	nAtoms = 0;
        for(l = g_list_first(atoms); l != NULL; l = l->next)
       	{
               	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
               	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
		nAtoms++;
	}
	if(nAtoms<1) return ok;
	position = (double (*)[3]) malloc(sizeof(double[3]) * nAtoms);
	types = (int*) malloc(sizeof(int) * nAtoms);
	i=0;
        for(l = g_list_first(atoms); l != NULL; l = l->next)
       	{
               	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
               	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
		types[i] =  (int)get_atomic_number_from_symbol(crystalloAtom->symbol);
		for(j=0;j<3;j++) position[i][j] = crystalloAtom->C[j];
		i++;
	}

	if( 0!= spg_find_primitive(lattice, position, types, nAtoms, (double)symprec))
	{
		for(i=0;i<3;i++) for(j=0;j<3;j++) newTv[j][i] = lattice[i][j];
		ok=TRUE;
	}

	if(types) free(types);
	if(position) free(position);
	return ok;
}
*/
SpglibDataset * crystalloGetDataSet(GList* atoms, gdouble symprec)
{
	SpglibDataset * spgDataSet = NULL;
	double (*position)[3];
	int* types = NULL;
	gint i,j;
	double lattice[3][3];
	gdouble newTv[3][3];
	gint nTv = crystalloGetTv(atoms, newTv);
	GList *l = NULL;
	int nAtoms = 0;
	gint numGroup = 0;

	if(nTv<3) return spgDataSet;
	for(i=0;i<3;i++) for(j=0;j<3;j++) lattice[i][j] = newTv[j][i];

	nAtoms = 0;
        for(l = g_list_first(atoms); l != NULL; l = l->next)
       	{
               	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
               	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
		nAtoms++;
	}
	if(nAtoms<1) return spgDataSet;
	position = (double (*)[3]) malloc(sizeof(double[3]) * nAtoms);
	types = (int*) malloc(sizeof(int) * nAtoms);
	i=0;
        for(l = g_list_first(atoms); l != NULL; l = l->next)
       	{
               	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
               	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
		types[i] =  (int)get_atomic_number_from_symbol(crystalloAtom->symbol);
		for(j=0;j<3;j++) position[i][j] = crystalloAtom->C[j];
		i++;
	}
	spgDataSet =  spg_get_dataset(lattice, position, types, nAtoms, (double)symprec);

	if(types) free(types);
	if(position) free(position);
	return spgDataSet;
}

gint crystalloGetGroupName(GList* atoms, char groupName[], gdouble symprec)
{
	double (*position)[3];
	int* types = NULL;
	gint i,j;
	double lattice[3][3];
	gdouble newTv[3][3];
	gint nTv = crystalloGetTv(atoms, newTv);
	GList *l = NULL;
	int nAtoms = 0;
	gint numGroup = 0;

	if(nTv<3) return numGroup;
	for(i=0;i<3;i++) for(j=0;j<3;j++) lattice[i][j] = newTv[j][i];

	nAtoms = 0;
        for(l = g_list_first(atoms); l != NULL; l = l->next)
       	{
               	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
               	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
		nAtoms++;
	}
	if(nAtoms<1) return numGroup;
	position = (double (*)[3]) malloc(sizeof(double[3]) * nAtoms);
	types = (int*) malloc(sizeof(int) * nAtoms);
	i=0;
        for(l = g_list_first(atoms); l != NULL; l = l->next)
       	{
               	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
               	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
		types[i] =  (int)get_atomic_number_from_symbol(crystalloAtom->symbol);
		for(j=0;j<3;j++) position[i][j] = crystalloAtom->C[j];
		i++;
	}

	numGroup = (gint) spg_get_international(groupName, lattice, position, types, nAtoms, (double)symprec);
	if(types) free(types);
	if(position) free(position);
	return numGroup;
}
static void setAtomTypes(CrystalloAtom* crystalAtom, gchar* type)
{
        sprintf(crystalAtom->mmType,"%s",type);
        sprintf(crystalAtom->pdbType,"%s",type);
        sprintf(crystalAtom->residueName,"%s",type);
}
GList* crystalloPrimitiveSPG(GList* atoms, gdouble symprec)
{
	double (*position)[3];
	int* types = NULL;
	gint i,j;
	double lattice[3][3];
	gdouble newTv[3][3];
	gint nTv = crystalloGetTv(atoms, newTv);
	GList *l = NULL;
	int nAtoms = 0;
	gint numGroup = 0;
	GList* newAtoms = NULL;
	int nAtomsNew = 0;

	if(nTv<3) return newAtoms;
	for(i=0;i<3;i++) for(j=0;j<3;j++) lattice[i][j] = newTv[j][i];

	nAtoms = 0;
        for(l = g_list_first(atoms); l != NULL; l = l->next)
       	{
               	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
               	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
		nAtoms++;
	}
	if(nAtoms<1) return newAtoms;
	position = (double (*)[3]) malloc(sizeof(double[3]) * 4*nAtoms);
	types = (int*) malloc(sizeof(int) * 4*nAtoms);
	i=0;
        for(l = g_list_first(atoms); l != NULL; l = l->next)
       	{
               	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
               	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
		types[i] =  (int)get_atomic_number_from_symbol(crystalloAtom->symbol);
		for(j=0;j<3;j++) position[i][j] = crystalloAtom->C[j];
		i++;
	}

	nAtomsNew = (gint) spg_find_primitive(lattice, position, types, nAtoms, (double)symprec);
	newAtoms = NULL;
	i=0;
        for(i=0;i<nAtomsNew;i++)
       	{
               	CrystalloAtom* crystalAtom = g_malloc(sizeof(CrystalloAtom));
		gchar* symbol = get_symbol_using_z(types[i]);
		sprintf(crystalAtom->symbol,"%s", symbol);
		g_free(symbol);
		setAtomTypes(crystalAtom, crystalAtom->symbol);
		crystalAtom->residueNumber=1;
		crystalAtom->charge=0.0;

		for(j=0;j<3;j++) crystalAtom->C[j]= position[i][j];
		newAtoms = g_list_append(newAtoms, (gpointer) crystalAtom);
	}
        for(i=0;i<3;i++)
       	{
               	CrystalloAtom* crystalAtom = g_malloc(sizeof(CrystalloAtom));
		sprintf(crystalAtom->symbol,"%s","Tv");
		setAtomTypes(crystalAtom, crystalAtom->symbol);
		crystalAtom->residueNumber=1;
		crystalAtom->charge=0.0;

		for(j=0;j<3;j++) crystalAtom->C[j]= lattice[j][i];
		newAtoms = g_list_append(newAtoms, (gpointer) crystalAtom);
	}
	
	if(types) free(types);
	if(position) free(position);
	return newAtoms;
}
GList* crystalloStandardizeCellSPG(GList* atoms, gint to_primitive, gint no_idealize, gdouble symprec)
{
	double (*position)[3];
	int* types = NULL;
	gint i,j;
	double lattice[3][3];
	gdouble newTv[3][3];
	gint nTv = crystalloGetTv(atoms, newTv);
	GList *l = NULL;
	int nAtoms = 0;
	gint numGroup = 0;
	GList* newAtoms = NULL;
	int nAtomsNew = 0;

	if(nTv<3) return newAtoms;
	for(i=0;i<3;i++) for(j=0;j<3;j++) lattice[i][j] = newTv[j][i];

	nAtoms = 0;
        for(l = g_list_first(atoms); l != NULL; l = l->next)
       	{
               	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
               	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
		nAtoms++;
	}
	if(nAtoms<1) return newAtoms;
	position = (double (*)[3]) malloc(sizeof(double[3]) * 4*nAtoms);
	types = (int*) malloc(sizeof(int) * 4*nAtoms);
	i=0;
        for(l = g_list_first(atoms); l != NULL; l = l->next)
       	{
               	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
               	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
		types[i] =  (int)get_atomic_number_from_symbol(crystalloAtom->symbol);
		for(j=0;j<3;j++) position[i][j] = crystalloAtom->C[j];
		i++;
	}

	nAtomsNew = (gint) spg_standardize_cell(lattice, position, types, nAtoms,  (const int) to_primitive, (const int)no_idealize, (const double) symprec);
	newAtoms = NULL;
	i=0;
        for(i=0;i<nAtomsNew;i++)
       	{
               	CrystalloAtom* crystalAtom = g_malloc(sizeof(CrystalloAtom));
		gchar* symbol = get_symbol_using_z(types[i]);
		sprintf(crystalAtom->symbol,"%s", symbol);
		g_free(symbol);
		setAtomTypes(crystalAtom, crystalAtom->symbol);
		crystalAtom->residueNumber=1;
		crystalAtom->charge=0.0;

		for(j=0;j<3;j++) crystalAtom->C[j]= position[i][j];
		newAtoms = g_list_append(newAtoms, (gpointer) crystalAtom);
	}
        for(i=0;i<3;i++)
       	{
               	CrystalloAtom* crystalAtom = g_malloc(sizeof(CrystalloAtom));
		sprintf(crystalAtom->symbol,"%s","Tv");
		setAtomTypes(crystalAtom, crystalAtom->symbol);
		crystalAtom->residueNumber=1;
		crystalAtom->charge=0.0;

		for(j=0;j<3;j++) crystalAtom->C[j]= lattice[j][i];
		newAtoms = g_list_append(newAtoms, (gpointer) crystalAtom);
	}
	
	if(types) free(types);
	if(position) free(position);
	return newAtoms;
}
/********************************************************************************/
SpglibDataset* standardizeFromDataSetSPG(Crystal* crystal, gdouble symprec)
{
	gboolean ok  = crystalloCartnToFract(crystal->atoms);
	SpglibDataset * spgDataSet = crystalloGetDataSet(crystal->atoms, symprec);
	if(spgDataSet)
	{
		GList* l;
		gint i,j;
		i=0;
		GList* newAtoms = NULL;
		i=0;
        	for(i=0;i<spgDataSet->n_std_atoms;i++)
       		{
               		CrystalloAtom* crystalAtom = g_malloc(sizeof(CrystalloAtom));
			gchar* symbol = get_symbol_using_z(spgDataSet->std_types[i]);
			sprintf(crystalAtom->symbol,"%s", symbol);
			g_free(symbol);
			setAtomTypes(crystalAtom, crystalAtom->symbol);
			crystalAtom->residueNumber=1;
			crystalAtom->charge=0.0;

			for(j=0;j<3;j++) crystalAtom->C[j]= spgDataSet->std_positions[i][j];
			newAtoms = g_list_append(newAtoms, (gpointer) crystalAtom);
		}
        	for(i=0;i<3;i++)
       		{
               		CrystalloAtom* crystalAtom = g_malloc(sizeof(CrystalloAtom));
			sprintf(crystalAtom->symbol,"%s","Tv");
			setAtomTypes(crystalAtom, crystalAtom->symbol);
			crystalAtom->residueNumber=1;
			crystalAtom->charge=0.0;

			for(j=0;j<3;j++) crystalAtom->C[j]= spgDataSet->std_lattice[j][i];
			newAtoms = g_list_append(newAtoms, (gpointer) crystalAtom);
		}
		if(newAtoms)
		{
    			g_list_foreach (crystal->atoms, (GFunc) crystalloFreeAtom, NULL);
    			g_list_free (crystal->atoms);
			crystal->atoms = newAtoms;
		}
	}
	if(ok) ok=crystalloFractToCartn(crystal->atoms);
	return spgDataSet;
}
