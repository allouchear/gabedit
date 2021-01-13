/* BuildCrystal.c */
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
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Constants.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Geometry/MenuToolBarGeom.h"
#include "../Crystallography/Crystallo.h"

void define_good_factor();
void create_GeomXYZ_from_draw_grometry();

#define LigneT 9
#define ColonneT 2


typedef enum
{
  LEFT1 = 0,
  RIGHT1,
  LEFT2,
  RIGHT2,
  LEFT3,
  RIGHT3
} FragPositions;

typedef struct _DataCrystalloDlg
{
	GeomDef* G;
	GeomDef* G0;
	gint Nb;
	gint Nb0;
	gint fisrtFragNumber;
	gint nTv; 
	gint iTv[3];
	gboolean* toRemove;
}DataCrystalloDlg;

typedef struct _DataWulffDlg
{
	gint nMaxSurfaces;
	gint nSurfaces;
	gdouble** surfaces;
	gdouble* layers;
	gdouble TvSmall[3][3];
	gint nTvSmall;
}DataWulffDlg;

typedef struct _DataSlabDlg
{
	gdouble surface[3];/* h, k, l */
	gdouble layers[3];
}DataSlabDlg;

/*
static gdouble getTorsion(GeomDef* geom, gint a1, gint a2, gint a3,gint a4)
{
	gdouble C1[3]={geom[a1].X,geom[a1].Y,geom[a1].Z};
	gdouble C2[3]={geom[a2].X,geom[a2].Y,geom[a2].Z};
	gdouble C3[3]={geom[a3].X,geom[a3].Y,geom[a3].Z};
	gdouble C4[3]={geom[a4].X,geom[a4].Y,geom[a4].Z};
	return TorsionToAtoms(C4,C1,C2,C3);
}
*/
/********************************************************************************/
static gboolean isItTv(gchar* symbol)
{
	static gchar tmp[BSIZE];
	sprintf(tmp,"%s",symbol);
	uppercase(tmp);
	if(!strcmp(tmp,"TV")) return TRUE;
	return FALSE;
}
/********************************************************************************/
static DataWulffDlg* init_wulff()
{ 
	gint i,j;
	DataWulffDlg* dataWulffDlg = g_malloc(sizeof(DataWulffDlg));
	dataWulffDlg->nMaxSurfaces = 10;
	dataWulffDlg->nSurfaces = 0;
	dataWulffDlg->surfaces = g_malloc(dataWulffDlg->nMaxSurfaces*sizeof(gdouble*));
	for(i=0;i<dataWulffDlg->nMaxSurfaces;i++) dataWulffDlg->surfaces[i] = g_malloc(3*sizeof(gdouble));
	dataWulffDlg->layers = g_malloc(dataWulffDlg->nMaxSurfaces*sizeof(gdouble));
	dataWulffDlg->nTvSmall=3;
	for(i=0;i<3;i++)  for(j=0;j<3;j++) dataWulffDlg->TvSmall[i][j] = 0;
	for(i=0;i<3;i++) dataWulffDlg->TvSmall[i][i] = 1;
	return dataWulffDlg;
}
/********************************************************************************/
static DataCrystalloDlg* init_variables()
{ 
	DataCrystalloDlg* dataDlg = g_malloc(sizeof(DataCrystalloDlg));
	gint j;
	gint i;
	dataDlg->G=NULL;
	dataDlg->G0=NULL;
	dataDlg->Nb = 0;
	dataDlg->Nb0 = 0;
	dataDlg->fisrtFragNumber = 0;
	dataDlg->nTv = 0;

	for(i=0;i<3;i++) dataDlg->iTv[i] = 0;

	if(Natoms<1) return dataDlg;
	dataDlg->fisrtFragNumber = geometry0[0].ResidueNumber;
	dataDlg->Nb = Natoms;
	dataDlg->G = g_malloc(dataDlg->Nb*sizeof(GeomDef));
	for(j=0;j<dataDlg->Nb;j++)
	{
		dataDlg->G[j].X=geometry0[j].X;
		dataDlg->G[j].Y=geometry0[j].Y;
		dataDlg->G[j].Z=geometry0[j].Z;
		dataDlg->G[j].Charge=geometry0[j].Charge;
		dataDlg->G[j].mmType=g_strdup(geometry0[j].mmType);
		dataDlg->G[j].pdbType=g_strdup(geometry0[j].pdbType);
		dataDlg->G[j].Residue=g_strdup(geometry0[j].Residue);
		dataDlg->G[j].ResidueNumber=dataDlg->fisrtFragNumber;
		dataDlg->G[j].Prop = prop_atom_get(geometry0[j].Prop.symbol);
		dataDlg->G[j].N = j+1;
		//printf("%s %f %f %f \n",dataDlg->G[j].Prop.symbol,dataDlg->G[j].X,dataDlg->G[j].Y,dataDlg->G[j].Z);
		dataDlg->G[j].typeConnections = NULL;
	}
	dataDlg->nTv = 0;
	for(j=0;j<dataDlg->Nb;j++)
	if(isItTv(dataDlg->G[j].Prop.symbol)) 
	{
		dataDlg->iTv[dataDlg->nTv] = j;
		dataDlg->nTv++;
	}
	dataDlg->Nb0 = Natoms-dataDlg->nTv;
	dataDlg->G0 = g_malloc(dataDlg->Nb0*sizeof(GeomDef));
	i = 0;
	for(j=0;j<dataDlg->Nb;j++)
	{
		if(isItTv(dataDlg->G[j].Prop.symbol)) continue; 
		dataDlg->G0[i].X=dataDlg->G[j].X;
		dataDlg->G0[i].Y=dataDlg->G[j].Y;
		dataDlg->G0[i].Z=dataDlg->G[j].Z;
		dataDlg->G0[i].Charge=dataDlg->G[j].Charge;
		dataDlg->G0[i].mmType=g_strdup(dataDlg->G[j].mmType);
		dataDlg->G0[i].pdbType=g_strdup(dataDlg->G[j].pdbType);
		dataDlg->G0[i].Residue=g_strdup(dataDlg->G[j].Residue);
		dataDlg->G0[i].ResidueNumber=dataDlg->fisrtFragNumber;
		dataDlg->G0[i].Prop = prop_atom_get(dataDlg->G[j].Prop.symbol);
		dataDlg->G0[i].N = i+1;
		dataDlg->G0[i].typeConnections = NULL;
		i++;
	}
	dataDlg->toRemove = g_malloc(dataDlg->Nb*sizeof(gboolean));
	for(j=0;j<dataDlg->Nb;j++) dataDlg->toRemove[j] = FALSE;
	return dataDlg;
}
/********************************************************************************/
static DataSlabDlg* init_slab()
{ 
	gint i;
	DataSlabDlg* dataSlabDlg = g_malloc(sizeof(DataSlabDlg));
	for(i=0;i<3;i++) dataSlabDlg->surface[i] = 1;
	for(i=0;i<3;i++) dataSlabDlg->layers[i] = 2;
	return dataSlabDlg;
}
/********************************************************************************/
static Crystal* init_crystal()
{ 
	Crystal* crystal = g_malloc(sizeof(Crystal));
	gint i;
	gdouble orig[3];

	initCrystal(crystal);

	if(Natoms<1) return crystal;

	get_origine_molecule_drawgeom(orig);
	//fprintf(stderr,"init_cryt %f %f %f\n",orig[0], orig[1], orig[2]);

	for(i=0;i<Natoms;i++)
	{
		CrystalloAtom* a = g_malloc(sizeof(CrystalloAtom));
		a->C[0] = geometry0[i].X;
		a->C[1] = geometry0[i].Y;
		a->C[2] = geometry0[i].Z;


		a->charge = geometry0[i].Charge;
		a->residueNumber = geometry0[i].ResidueNumber;

		sprintf(a->symbol,"%s",geometry0[i].Prop.symbol);
		sprintf(a->mmType,"%s",geometry0[i].mmType);
		sprintf(a->pdbType,"%s",geometry0[i].pdbType);
		sprintf(a->residueName,"%s",geometry0[i].Residue);

		//DEBUG
		//if(isItTv(a->symbol))
		{
			gint j;
			for(j=0;j<3;j++)  a->C[j] += orig[j];
			for(j=0;j<3;j++)  a->C[j] *= BOHR_TO_ANG;
			//fprintf(stderr,"TV %f %f %f\n",a->C[0], a->C[1], a->C[2]);
		}
		crystal->atoms=g_list_append(crystal->atoms, (gpointer) a);
	}
	return crystal;
}
/*****************************************************************************/
static void deleteDataDlg(DataCrystalloDlg* dataDlg)
{
	if(!dataDlg) return;

	if(dataDlg->Nb!=0)
	{
		gint i;
		if(dataDlg->G)
                for (i=0;i<dataDlg->Nb;i++)
		{
			g_free(dataDlg->G[i].Prop.symbol);
			g_free(dataDlg->G[i].Prop.name);
			g_free(dataDlg->G[i].mmType);
			g_free(dataDlg->G[i].pdbType);
			g_free(dataDlg->G[i].Residue);
			if(dataDlg->G[i].typeConnections) g_free(dataDlg->G[i].typeConnections);
		}

		if(dataDlg->G) g_free(dataDlg->G);
	}
	dataDlg->Nb=0;
	dataDlg->G = NULL;
	if(dataDlg->Nb0!=0)
	{
		gint i;
		if(dataDlg->G0)
                for (i=0;i<dataDlg->Nb0;i++)
		{
			g_free(dataDlg->G0[i].Prop.symbol);
			g_free(dataDlg->G0[i].Prop.name);
			g_free(dataDlg->G0[i].mmType);
			g_free(dataDlg->G0[i].pdbType);
			g_free(dataDlg->G0[i].Residue);
			if(dataDlg->G0[i].typeConnections) g_free(dataDlg->G0[i].typeConnections);
		}

		if(dataDlg->G0) g_free(dataDlg->G0);
	}
	dataDlg->Nb0=0;
	dataDlg->G0 = NULL;
	if(dataDlg->toRemove) g_free(dataDlg->toRemove);
}
/*****************************************************************************/
static void destroy_dlg(GtkWidget* Dlg,gpointer data) 
{
  	DataCrystalloDlg* dataDlg = g_object_get_data(G_OBJECT (Dlg), "DataDlg");
  	DataWulffDlg* dataWulffDlg = g_object_get_data(G_OBJECT (Dlg), "DataWulffDlg");
  	Crystal* crystal = g_object_get_data(G_OBJECT (Dlg), "Crystal");

	if(crystal)
	{
		freeCrystal(crystal);
		g_free(crystal);
	}
	if(dataWulffDlg)
	{
		gint i;
		if(dataWulffDlg->layers) g_free(dataWulffDlg->layers);
		if(dataWulffDlg->surfaces)
		{
			for(i=0;i<dataWulffDlg->nMaxSurfaces;i++) if(dataWulffDlg->surfaces[i]) g_free(dataWulffDlg->surfaces[i]);
			g_free(dataWulffDlg->surfaces);
		}
		g_free(dataWulffDlg);
	}
	if(dataDlg) deleteDataDlg(dataDlg);

	delete_child(Dlg);
	
	activate_rotation();
}
/*****************************************************************************/
static void define_geometry_to_draw_from_crystal(Crystal* crystal)
{
	gint i;
	gint j;
	gdouble C[3] = {0.0,0.0,0.0};
	gint n;
	GList *l = NULL;
	gdouble orig[3];
	get_origine_molecule_drawgeom(orig);

	//fprintf(stderr,"%f %f %f\n",orig[0], orig[1], orig[2]);

	Free_One_Geom(geometry0,Natoms);
	Free_One_Geom(geometry ,Natoms);
	geometry0 = NULL;
	geometry  = NULL;

	Natoms = crystalloNumberOfAtoms(crystal->atoms);
	if(Natoms<1) return;
	//reset_origine_molecule_drawgeom();
	geometry0 = g_malloc((Natoms)*sizeof(GeomDef));
	geometry  = g_malloc((Natoms)*sizeof(GeomDef));


	n = 0;
       	for(l = g_list_first(crystal->atoms); l != NULL; l = l->next) 
	{
		CrystalloAtom* a = (CrystalloAtom*)l->data;

		geometry0[n].Charge = a->charge;
		geometry0[n].Prop = prop_atom_get(a->symbol);
		geometry0[n].mmType = g_strdup(a->mmType);
		geometry0[n].pdbType = g_strdup(a->pdbType);
		geometry0[n].Residue = g_strdup(a->residueName);
		geometry0[n].ResidueNumber = a->residueNumber;
		geometry0[n].show = TRUE;
		geometry0[n].Layer = HIGH_LAYER;
		geometry0[n].Variable = TRUE;
		geometry0[n].N = n+1;
        	geometry0[n].typeConnections = NULL;

		geometry[n].Charge = a->charge;
		geometry[n].Prop = prop_atom_get(a->symbol);
		geometry[n].mmType = g_strdup(a->mmType);
		geometry[n].pdbType = g_strdup(a->pdbType);
		geometry[n].Residue = g_strdup(a->residueName);
		geometry[n].ResidueNumber = a->residueNumber;
		geometry[n].show = TRUE;
		geometry[n].Layer = HIGH_LAYER;
		geometry[n].Variable = TRUE;
		geometry[n].N = n+1;
        	geometry[n].typeConnections = NULL;

		// DEBUG
		//if(isItTv(a->symbol))
		{
			geometry0[n].X = a->C[0]/BOHR_TO_ANG-orig[0];
			geometry0[n].Y = a->C[1]/BOHR_TO_ANG-orig[1];
			geometry0[n].Z = a->C[2]/BOHR_TO_ANG-orig[2];

			geometry[n].X = a->C[0]/BOHR_TO_ANG-orig[0];
			geometry[n].Y = a->C[1]/BOHR_TO_ANG-orig[1];
			geometry[n].Z = a->C[2]/BOHR_TO_ANG-orig[2];
			//fprintf(stderr,"TV %f %f %f\n",a->C[0], a->C[1], a->C[2]);
		}

		n++;
	}
	Natoms = n;
	if(n>0)
	{
		geometry0 = g_realloc(geometry0,(Natoms)*sizeof(GeomDef));
		geometry  = g_realloc(geometry,(Natoms)*sizeof(GeomDef));
	}

	for(i=0;i<(gint)Natoms;i++)
	{
		geometry[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<(gint)Natoms;j++) geometry[i].typeConnections[j] = 0;
		geometry0[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<(gint)Natoms;j++) geometry0[i].typeConnections[j] = 0;
	}
	for(i=0;i<(gint)Natoms;i++) geometry[i].N = geometry0[i].N = i+1;

	RebuildGeom = TRUE;
}
/********************************************************************************/
static void define_geometry_to_draw(DataCrystalloDlg* dataDlg)
{
	gint i;
	gint j;
	gdouble C[3] = {0.0,0.0,0.0};
	gint n;

	Free_One_Geom(geometry0,Natoms);
	Free_One_Geom(geometry ,Natoms);
	Natoms = 0;
	geometry0 = NULL;
	geometry  = NULL;

	Natoms = dataDlg->Nb;
	if(Natoms<1) return;
	//reset_origine_molecule_drawgeom();
	geometry0 = g_malloc((Natoms)*sizeof(GeomDef));
	geometry  = g_malloc((Natoms)*sizeof(GeomDef));
	n = 0;
	for(i=0;i<dataDlg->Nb;i++)
	{
		geometry0[n].X = dataDlg->G[i].X;
		geometry0[n].Y = dataDlg->G[i].Y;
		geometry0[n].Z = dataDlg->G[i].Z;
		geometry0[n].Charge = dataDlg->G[i].Charge;
		geometry0[n].Prop = prop_atom_get(dataDlg->G[i].Prop.symbol);
		geometry0[n].mmType = g_strdup(dataDlg->G[i].mmType);
		geometry0[n].pdbType = g_strdup(dataDlg->G[i].pdbType);
		geometry0[n].Residue = g_strdup(dataDlg->G[i].Residue);
		geometry0[n].ResidueNumber = dataDlg->G[i].ResidueNumber;
		geometry0[n].show = TRUE;
		geometry0[n].Layer = HIGH_LAYER;
		geometry0[n].Variable = TRUE;


		geometry0[n].N = i+1;
        	geometry0[n].typeConnections = NULL;

		geometry[n].X = dataDlg->G[i].X;
		geometry[n].Y = dataDlg->G[i].Y;
		geometry[n].Z = dataDlg->G[i].Z;
		geometry[n].Charge = dataDlg->G[i].Charge;
		geometry[n].Prop = prop_atom_get(dataDlg->G[i].Prop.symbol);
		geometry[n].mmType = g_strdup(geometry0[n].mmType);
		geometry[n].pdbType = g_strdup(geometry0[n].pdbType);
		geometry[n].Residue = g_strdup(geometry0[n].Residue);
		geometry[n].ResidueNumber = dataDlg->G[i].ResidueNumber;
		geometry[n].show = TRUE;
		geometry[n].N = i+1;
        	geometry[n].typeConnections = NULL;
		geometry[n].Layer = HIGH_LAYER;
		geometry[n].Variable = TRUE;
		C[0] +=  dataDlg->G[i].X;
		C[1] +=  dataDlg->G[i].Y;
		C[2] +=  dataDlg->G[i].Z;
		n++;
	}
	Natoms = n;
	if(n>0)
	{
		geometry0 = g_realloc(geometry0,(Natoms)*sizeof(GeomDef));
		geometry  = g_realloc(geometry,(Natoms)*sizeof(GeomDef));
	}

	for(i=0;i<(gint)Natoms;i++)
	{
		geometry[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<(gint)Natoms;j++) geometry[i].typeConnections[j] = 0;
		geometry0[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<(gint)Natoms;j++) geometry0[i].typeConnections[j] = 0;
	}
	for(i=0;i<(gint)Natoms;i++) geometry[i].N = geometry0[i].N = i+1;

	RebuildGeom = TRUE;
}
/********************************************************************************/
static gboolean getTvVector(DataCrystalloDlg* dataDlg, gchar* what, gdouble Tv[])
{
	GeomDef* G = dataDlg->G;
	gint* iTv = dataDlg->iTv;
	gint nTv = dataDlg->nTv;
	gint k = 0;
	gboolean Ok = FALSE;
	gint j;
	gdouble orig[3];
	get_origine_molecule_drawgeom(orig);

	if(dataDlg->nTv<1) return FALSE;
	j = dataDlg->iTv[0];
	if(strstr(what,"LEFT 1"))
	{
		Tv[0] = -(G[j].X+orig[0]);
		Tv[1] = -(G[j].Y+orig[1]);
		Tv[2] = -(G[j].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"RIGHT 1"))
	{
		Tv[0] = (G[j].X+orig[0]);
		Tv[1] = (G[j].Y+orig[1]);
		Tv[2] = (G[j].Z+orig[2]);
		return TRUE;
	}
	if(nTv<2) return FALSE;

	j = iTv[1];
	if(strstr(what,"LEFT 2"))
	{
		Tv[0] = -(G[j].X+orig[0]);
		Tv[1] = -(G[j].Y+orig[1]);
		Tv[2] = -(G[j].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"RIGHT 2"))
	{
		Tv[0] = (G[j].X+orig[0]);
		Tv[1] = (G[j].Y+orig[1]);
		Tv[2] = (G[j].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"L1-L2"))
	{
		Tv[0] =-(G[iTv[0]].X+orig[0])-(G[iTv[1]].X+orig[0]);
		Tv[1] =-(G[iTv[0]].Y+orig[1])-(G[iTv[1]].Y+orig[1]);
		Tv[2] =-(G[iTv[0]].Z+orig[2])-(G[iTv[1]].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"L1-R2"))
	{
		Tv[0] =-(G[iTv[0]].X+orig[0])+(G[iTv[1]].X+orig[0]);
		Tv[1] =-(G[iTv[0]].Y+orig[1])+(G[iTv[1]].Y+orig[1]);
		Tv[2] =-(G[iTv[0]].Z+orig[2])+(G[iTv[1]].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"L2-R1"))
	{
		Tv[0] =(G[iTv[0]].X+orig[0])-(G[iTv[1]].X+orig[0]);
		Tv[1] =(G[iTv[0]].Y+orig[1])-(G[iTv[1]].Y+orig[1]);
		Tv[2] =(G[iTv[0]].Z+orig[2])-(G[iTv[1]].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"R1-R2"))
	{
		Tv[0] =(G[iTv[0]].X+orig[0])+(G[iTv[1]].X+orig[0]);
		Tv[1] =(G[iTv[0]].Y+orig[1])+(G[iTv[1]].Y+orig[1]);
		Tv[2] =(G[iTv[0]].Z+orig[2])+(G[iTv[1]].Z+orig[2]);
		return TRUE;
	}

	if(nTv<2) return FALSE;
	j = iTv[2];
	if(strstr(what,"LEFT 3"))
	{
		Tv[0] = -(G[j].X+orig[0]);
		Tv[1] = -(G[j].Y+orig[1]);
		Tv[2] = -(G[j].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"RIGHT 3"))
	{
		Tv[0] = (G[j].X+orig[0]);
		Tv[1] = (G[j].Y+orig[1]);
		Tv[2] = (G[j].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"L1-L3"))
	{
		Tv[0] =-(G[iTv[0]].X+orig[0])-(G[iTv[2]].X+orig[0]);
		Tv[1] =-(G[iTv[0]].Y+orig[1])-(G[iTv[2]].Y+orig[1]);
		Tv[2] =-(G[iTv[0]].Z+orig[2])-(G[iTv[2]].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"L1-R3"))
	{
		Tv[0] =-(G[iTv[0]].X+orig[0])+(G[iTv[2]].X+orig[0]);
		Tv[1] =-(G[iTv[0]].Y+orig[1])+(G[iTv[2]].Y+orig[1]);
		Tv[2] =-(G[iTv[0]].Z+orig[2])+(G[iTv[2]].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"L2-L3"))
	{
		Tv[0] =-(G[iTv[1]].X+orig[0])-(G[iTv[2]].X+orig[0]);
		Tv[1] =-(G[iTv[1]].Y+orig[1])-(G[iTv[2]].Y+orig[1]);
		Tv[2] =-(G[iTv[1]].Z+orig[2])-(G[iTv[2]].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"L2-R3"))
	{
		Tv[0] =-(G[iTv[1]].X+orig[0])+(G[iTv[2]].X+orig[0]);
		Tv[1] =-(G[iTv[1]].Y+orig[1])+(G[iTv[2]].Y+orig[1]);
		Tv[2] =-(G[iTv[1]].Z+orig[2])+(G[iTv[2]].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"L3-R1"))
	{
		Tv[0] =-(G[iTv[2]].X+orig[0])+(G[iTv[0]].X+orig[0]);
		Tv[1] =-(G[iTv[2]].Y+orig[1])+(G[iTv[0]].Y+orig[1]);
		Tv[2] =-(G[iTv[2]].Z+orig[2])+(G[iTv[0]].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"L3-R2"))
	{
		Tv[0] =-(G[iTv[2]].X+orig[0])+(G[iTv[1]].X+orig[0]);
		Tv[1] =-(G[iTv[2]].Y+orig[1])+(G[iTv[1]].Y+orig[1]);
		Tv[2] =-(G[iTv[2]].Z+orig[2])+(G[iTv[1]].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"R1-R3"))
	{
		Tv[0] =(G[iTv[0]].X+orig[0])+(G[iTv[2]].X+orig[0]);
		Tv[1] =(G[iTv[0]].Y+orig[1])+(G[iTv[2]].Y+orig[1]);
		Tv[2] =(G[iTv[0]].Z+orig[2])+(G[iTv[2]].Z+orig[2]);
		return TRUE;
	}
	if(strstr(what,"R2-R3"))
	{
		Tv[0] =(G[iTv[1]].X+orig[0])+(G[iTv[2]].X+orig[0]);
		Tv[1] =(G[iTv[1]].Y+orig[1])+(G[iTv[2]].Y+orig[1]);
		Tv[2] =(G[iTv[1]].Z+orig[2])+(G[iTv[2]].Z+orig[2]);
		return TRUE;
	}
	return FALSE;
	
}
/********************************************************************************/
static gint getNumFrag(gchar* what, gint fisrtFragNumber)
{
	gint k = 0;
	gboolean Ok = FALSE;
	gint j;
	gint ll;
	if(strstr(what,"LEFT 1")) return fisrtFragNumber+1;
	if(strstr(what,"RIGHT 1")) return fisrtFragNumber+2;
	if(strstr(what,"LEFT 2")) return fisrtFragNumber+3;
	if(strstr(what,"RIGHT 2")) return fisrtFragNumber+4;
	if(strstr(what,"LEFT 3")) return fisrtFragNumber+5;
	if(strstr(what,"RIGHT 3")) return fisrtFragNumber+6;
	ll = fisrtFragNumber+6;
	if(strstr(what,"L1-L2")) return ll+1;
	if(strstr(what,"L1-L3")) return ll+2;
	if(strstr(what,"L1-R2")) return ll+3;
	if(strstr(what,"L1-R3")) return ll+4;
	if(strstr(what,"L2-R1")) return ll+5;
	if(strstr(what,"L2-R3")) return ll+6;
	if(strstr(what,"L2-L3")) return ll+7;
	if(strstr(what,"L3-R1")) return ll+8;
	if(strstr(what,"L3-R2")) return ll+9;
	if(strstr(what,"R1-R2")) return ll+10;
	if(strstr(what,"R1-R3")) return ll+11;
	if(strstr(what,"R2-R3")) return ll+12;
	return ll+13;
}
/********************************************************************************/
static void add_fragment(DataCrystalloDlg* dataDlg, gchar* what)
{
	gint i;
	gint j;
	gint nAtomsNew= 0;
	gdouble Tv[3];
	gint numFrag = 0;

	G_CONST_RETURN gchar* t;

	if(dataDlg->nTv<1) return;
	nAtomsNew= dataDlg->Nb0;
	if(nAtomsNew<1) return;
	
	if(!getTvVector(dataDlg, what, Tv)) return;
	if(dataDlg->Nb>0) dataDlg->G = g_realloc(dataDlg->G,(dataDlg->Nb+nAtomsNew)*sizeof(GeomDef));
	else dataDlg->G = g_malloc((nAtomsNew)*sizeof(GeomDef));

	if(dataDlg->Nb>0) dataDlg->toRemove = g_realloc(dataDlg->toRemove,(dataDlg->Nb+nAtomsNew)*sizeof(gboolean));
	else dataDlg->toRemove = g_malloc((nAtomsNew)*sizeof(gboolean));

	Ddef = FALSE;
	//printf("Tv= %f %f %f\n",G[numTv].X,G[numTv].Y,G[numTv].Z);

	numFrag = getNumFrag(what, dataDlg->fisrtFragNumber);
	j=dataDlg->Nb-1;
	for(i=0;i<dataDlg->Nb0;i++)
	{
		j++;
		dataDlg->G[j].X=dataDlg->G0[i].X+Tv[0];
		dataDlg->G[j].Y=dataDlg->G0[i].Y+Tv[1];
		dataDlg->G[j].Z=dataDlg->G0[i].Z+Tv[2];
		dataDlg->G[j].Charge=dataDlg->G0[i].Charge;
		dataDlg->G[j].mmType=g_strdup(dataDlg->G0[i].mmType);
		dataDlg->G[j].pdbType=g_strdup(dataDlg->G0[i].pdbType);
		dataDlg->G[j].Residue=g_strdup(dataDlg->G0[i].Residue);
		dataDlg->G[j].ResidueNumber=numFrag;

		dataDlg->G[j].Prop = prop_atom_get(dataDlg->G0[i].Prop.symbol);
		dataDlg->G[j].N = j+1;
		dataDlg->G[j].typeConnections = NULL;
		dataDlg->toRemove[j] = FALSE;
	}

	dataDlg->Nb += nAtomsNew;

	define_geometry_to_draw(dataDlg);
	//define_good_factor();
	unselect_all_atoms();

	reset_all_connections();

	reset_charges_multiplicities();
	drawGeom();
	set_optimal_geom_view();
	create_GeomXYZ_from_draw_grometry();
}
/********************************************************************************/
static void build_crystal(GtkWidget *button,gpointer data)
{
  	GtkWidget* Dlg = g_object_get_data(G_OBJECT (button), "WinDlg");
  	DataCrystalloDlg* dataDlg = g_object_get_data(G_OBJECT (Dlg), "DataDlg");
	gchar fragName[BSIZE];
	sprintf(fragName,"%s",(gchar*)data);
	uppercase(fragName);
	add_fragment(dataDlg, fragName);
}
/********************************************************************************************************/
static void add_buttons(GtkWidget *Dlg,GtkWidget* box)
{
	GtkWidget* Table;
	GtkWidget* button;
	GtkWidget* frame;
	guint i;
	guint j;
        /* GtkStyle *button_style;*/
        /* GtkStyle *style;*/

	static char *Symb[LigneT][ColonneT]={
		{"Left 1","Right 1"},
		{"Left 2","Right 2"},
		{"Left 3","Right 3"},
		{"L1-L2","L1-L3"},
		{"L1-R2","L1-R3"},
		{"L2-L3","L2-R1"},
		{"L2-R3","L3-R1"},
		{"L3-R2","R1-R2"},
		{"R1-R3","R2-R3"},
		};
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);

  gtk_container_add(GTK_CONTAINER(box),frame);  
  gtk_widget_show (frame);

  Table = gtk_table_new(LigneT,ColonneT,TRUE);
  gtk_container_add(GTK_CONTAINER(frame),Table);
  /* button_style = gtk_widget_get_style(Dlg); */
  
  for ( i = 0;i<LigneT;i++)
	  for ( j = 0;j<ColonneT;j++)
  	{
		gchar tmp[BSIZE];
		sprintf(tmp,"%s",Symb[i][j]);
	  	button = gtk_button_new_with_label(tmp);
  		g_object_set_data(G_OBJECT (button), "WinDlg",Dlg);

          	/* style=set_button_style(button_style,button,"H");*/
          	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)build_crystal,(gpointer )Symb[i][j]);
	  	gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	  }

  g_object_set_data(G_OBJECT (Dlg), "FrameCrystal",frame);
  
}
/**********************************************************************/
void build_crystal_dlg()
{
  GtkWidget *Dlg;
  GtkWidget *Button;
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *vboxframe;
  gint nTv = 0;
  
  DataCrystalloDlg* dataDlg = init_variables();

  if(dataDlg->nTv == 0 ) 
  {
      	gchar* t = g_strdup_printf(_("Sorry, You must read/build a molecule with at least one Tv vector."));
        GtkWidget* w = Message(t,_("Error"),TRUE);
        g_free(t);
	return;
  }
  Dlg = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dlg),_("Build Crystal"));
  gtk_window_set_modal (GTK_WINDOW (Dlg), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(Dlg),GTK_WINDOW(GeomDlg));
  g_object_set_data(G_OBJECT(Dlg), "DataDlg", dataDlg);


  add_child(GeomDlg,Dlg,gtk_widget_destroy,_(" Build Crystal "));

  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)destroy_dlg,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->vbox), frame,TRUE,TRUE,0);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  hbox = create_hbox_false(vboxframe);
  add_buttons(Dlg,hbox);

  hbox = create_hbox_false(vboxframe);



  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dlg)->action_area), FALSE);
  gtk_widget_realize(Dlg);
  Button = create_button(Dlg,_("Close"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 2);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)destroy_dlg,GTK_OBJECT(Dlg));

  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->vbox);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->action_area);
  gtk_widget_show_now(Dlg);

  /* fit_windows_position(GeomDlg, Dlg);*/

}

/********************************************************************************/
static void build_supercell_simple(GtkWidget *Dlg,gpointer data)
{
  	Crystal* crystal = g_object_get_data(G_OBJECT (Dlg), "Crystal");
  	GtkWidget* entry[3];
	entry[0] = g_object_get_data(G_OBJECT (Dlg), "Entry1");
	entry[1] = g_object_get_data(G_OBJECT (Dlg), "Entry2");
	entry[2] = g_object_get_data(G_OBJECT (Dlg), "Entry3");
	G_CONST_RETURN gchar* str;
	gint i[3] = {0,0,0};
	gint k;
	
	for(k=0;k<3;k++)
	if(entry[k]) 
	{
		str = gtk_entry_get_text(GTK_ENTRY(entry[k]));
		if(str) i[k] = atoi(str);
	}
	//printf("NbAv = %d \n",Nb);
	if(crystal) 
	{
		if(buildSuperCellSimple(&crystal->atoms, i[0], i[1], i[2]))
			crystalloRemoveAtomsWithSmallDistance(&crystal->atoms);
	}

	//printf("NbAp = %d \n",Nb);
	define_geometry_to_draw_from_crystal(crystal);
	//define_good_factor();
	unselect_all_atoms();

	reset_all_connections();

	reset_charges_multiplicities();
	drawGeom();
	set_optimal_geom_view();
	create_GeomXYZ_from_draw_grometry();
	destroy_dlg(Dlg,data);
}
/********************************************************************************/
static void  add_number_of_cells_entrys(GtkWidget *Wins,GtkWidget *vbox, gint nTv)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(5,4,FALSE);
	GtkWidget* entry1 = NULL;
	GtkWidget* entry2 = NULL;
	GtkWidget* entry3 = NULL;
	GtkWidget* label = NULL;
	gint i;
	gint j;

	if(nTv<1) return;

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	add_label_table(table,_("A repeats"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entry1 = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry1),"2");
	gtk_widget_set_size_request(GTK_WIDGET(entry1),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entry1, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "Entry1", entry1);
	if(nTv<2) return;
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("B repeats"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entry1 = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry1),"2");
	gtk_widget_set_size_request(GTK_WIDGET(entry1),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entry1, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "Entry2", entry1);
	if(nTv<2) return;
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("C repeats"),(gushort)i,(gushort)j);
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j = 2;
	entry1 = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry1),"2");
	gtk_widget_set_size_request(GTK_WIDGET(entry1),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entry1, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "Entry3", entry1);
/*----------------------------------------------------------------------------------*/

	gtk_widget_show_all(table);
}
/**********************************************************************/
void build_supercell_simple_dlg()
{
  GtkWidget *Dlg;
  GtkWidget *Button;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *vboxframe;
  gint nTv = 0;

  Crystal* crystal = init_crystal();
  nTv = crystalloNumberOfTv(crystal->atoms);
  if(nTv == 0 ) 
  
  {
      	gchar* t = g_strdup_printf(_("Sorry, You must read/build a molecule with at least one Tv vector."));
        GtkWidget* w = Message(t,_("Error"),TRUE);
        g_free(t);
	freeCrystal(crystal);
	g_free(crystal);
	return;
  }
  Dlg = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dlg),_("Build SuperCell"));
  gtk_window_set_modal (GTK_WINDOW (Dlg), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(Dlg),GTK_WINDOW(GeomDlg));
  g_object_set_data(G_OBJECT(Dlg), "Crystal", crystal);


  add_child(GeomDlg,Dlg,gtk_widget_destroy,_(" Build SuperCell "));
  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)destroy_dlg,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->vbox), frame,TRUE,TRUE,0);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  add_number_of_cells_entrys(Dlg, vboxframe, nTv);

  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dlg)->action_area), FALSE);
  gtk_widget_realize(Dlg);
  Button = create_button(Dlg,_("Close"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 2);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)destroy_dlg,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);

  Button = create_button(Dlg,_("OK"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 2);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)build_supercell_simple,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);

  gtk_widget_show_all(GTK_DIALOG(Dlg)->vbox);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->action_area);

  gtk_widget_show_now(Dlg);

  /* fit_windows_position(GeomDlg, Dlg);*/

}
/********************************************************************************/
static void build_supercell(GtkWidget *Dlg,gpointer data)
{
  	Crystal* crystal = g_object_get_data(G_OBJECT (Dlg), "Crystal");
	GtkWidget* entry;
	gint P[3][3];
	gdouble p[3];
	gchar tmp[BSIZE];
	G_CONST_RETURN gchar* str;
	gint i,j;

	for(i=0;i<3;i++) 
	{
		p[i] = 0.0;
		for(j=0;j<3;j++)  
		{
			P[i][j] = 0;
			sprintf(tmp,"EntryRot%d%d", i,j);
			entry =  g_object_get_data(G_OBJECT (Dlg), tmp);
			if(entry) 
			{
				str = gtk_entry_get_text(GTK_ENTRY(entry));
				if(str) P[i][j] = atoi(str);
			}
		}
		sprintf(tmp,"EntryShift%d", i);
		entry =  g_object_get_data(G_OBJECT (Dlg), tmp);
		if(entry) 
		{
			str = gtk_entry_get_text(GTK_ENTRY(entry));
			if(str) p[i] = atof(str);
		}
	}
	//printf("NbAv = %d \n",Nb);
	if(crystal && buildSuperCell(&crystal->atoms, P,p))
		crystalloRemoveAtomsWithSmallDistance(&crystal->atoms);

	//printf("NbAp = %d \n",Nb);
	define_geometry_to_draw_from_crystal(crystal);
	//define_good_factor();
	unselect_all_atoms();

	reset_all_connections();

	reset_charges_multiplicities();
	drawGeom();
	set_optimal_geom_view();
	create_GeomXYZ_from_draw_grometry();
	destroy_dlg(Dlg,data);
}
/********************************************************************************/
static void  add_rotshift_matrix_entrys(GtkWidget *Wins,GtkWidget *vbox, gint nTv)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(8,4,FALSE);
	GtkWidget* entryRot[3][3];
	GtkWidget* entryShift[3];
	GtkWidget* label = NULL;
	gint i;
	gint j;
	gchar tmp[BSIZE];

	for(i=0;i<3;i++) entryShift[i] = NULL;
	for(i=0;i<3;i++) for(j=0;j<3;j++)  entryRot[i][j] = NULL;

	if(nTv<1) return;

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	label = gtk_label_new("Rotation matrix P(Integer) ");
	gtk_table_attach(GTK_TABLE(table),label, j,j+3,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j=3;
	label = gtk_label_new("Origin shift p (Real)");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	for(i=0;i<3;i++) 
	{
		if(nTv<i+1) break;
		for(j=0;j<3;j++)  
		{
			entryRot[i][j] = gtk_entry_new();
			if(i==j) gtk_entry_set_text(GTK_ENTRY(entryRot[i][j]),"2");
			else  gtk_entry_set_text(GTK_ENTRY(entryRot[i][j]),"0");
			gtk_table_attach(GTK_TABLE(table),entryRot[i][j], j,j+1,i+1,i+2,
                  	(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  	(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  	1,1);
			sprintf(tmp,"EntryRot%d%d", i,j);
			g_object_set_data(G_OBJECT(Wins), tmp, entryRot[i][j]);
		}
		entryShift[i] = gtk_entry_new();
		gtk_entry_set_text(GTK_ENTRY(entryShift[i]),"0.0");
		j =  3;
		gtk_table_attach(GTK_TABLE(table),entryShift[i], j,j+1,i+1,i+2,
                	(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  	(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  	1,1);
		sprintf(tmp,"EntryShift%d", i);
		g_object_set_data(G_OBJECT(Wins), tmp, entryShift[i]);
	}
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 0;
	label = gtk_label_new(" ");
	gtk_label_set_markup(GTK_LABEL(label), "<b>a'</b>= P<sub>11</sub> <b>a</b> + P<sub>21</sub> <b>b</b> + P<sub>31</sub> <b>c</b>");
	gtk_table_attach(GTK_TABLE(table),label, j,j+2,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	i++;
	j = 0;
	label = gtk_label_new(" ");
	gtk_label_set_markup(GTK_LABEL(label), "<b>b'</b>= P<sub>12</sub> <b>a</b> + P<sub>22</sub> <b>b</b> + P<sub>32</sub> <b>c</b>");
	gtk_table_attach(GTK_TABLE(table),label, j,j+2,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	j=2;
	label = gtk_label_new(" ");
	gtk_label_set_markup(GTK_LABEL(label), "Origin shifted by <b>p</b>= p<sub>1</sub><b>a</b> + p<sub>2</sub> <b>b</b> + p<sub>3</sub> <b>c</b>");
	gtk_table_attach(GTK_TABLE(table),label, j,j+2,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	i++;
	j = 0;
	label = gtk_label_new(" ");
	gtk_label_set_markup(GTK_LABEL(label), "<b>c'</b>= P<sub>13</sub> <b>a</b> + P<sub>23</sub> <b>b</b> + P<sub>33</sub> <b>c</b>");
	gtk_table_attach(GTK_TABLE(table),label, j,j+2,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	gtk_widget_show_all(table);
}
/**********************************************************************/
void build_supercell_dlg()
{
  GtkWidget *Dlg;
  GtkWidget *Button;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *vboxframe;
  gint nTv = 0;

  Crystal* crystal = init_crystal();
  nTv = crystalloNumberOfTv(crystal->atoms);
  if(nTv == 0 ) 
  
  {
      	gchar* t = g_strdup_printf(_("Sorry, You must read/build a molecule with at least one Tv vector."));
        GtkWidget* w = Message(t,_("Error"),TRUE);
        g_free(t);
	freeCrystal(crystal);
	g_free(crystal);
	return;
  }
  Dlg = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dlg),_("Build SuperCell"));
  gtk_window_set_modal (GTK_WINDOW (Dlg), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(Dlg),GTK_WINDOW(GeomDlg));
  g_object_set_data(G_OBJECT(Dlg), "Crystal", crystal);


  add_child(GeomDlg,Dlg,gtk_widget_destroy,_(" Build SuperCell "));
  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)destroy_dlg,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->vbox), frame,TRUE,TRUE,0);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  add_rotshift_matrix_entrys(Dlg, vboxframe, nTv);

  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dlg)->action_area), FALSE);
  gtk_widget_realize(Dlg);
  Button = create_button(Dlg,_("Close"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 2);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)destroy_dlg,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);

  Button = create_button(Dlg,_("OK"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 2);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)build_supercell,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);

  gtk_widget_show_all(GTK_DIALOG(Dlg)->vbox);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->action_area);

  gtk_widget_show_now(Dlg);

  /* fit_windows_position(GeomDlg, Dlg);*/

}
/********************************************************************************/
static void build_wulff(GtkWidget *Dlg,gpointer data)
{
	GtkWidget*** entrys =  g_object_get_data(G_OBJECT (Dlg), "Entries");
  	DataWulffDlg* dataWulffDlg = g_object_get_data(G_OBJECT (Dlg), "DataWulffDlg");
  	Crystal* crystal = g_object_get_data(G_OBJECT (Dlg), "Crystal");

	G_CONST_RETURN gchar* str;
	gint i,k;
	gdouble radius = -1;
	gint nSteps[3] = {1,1,1};

	if(!entrys) return;
	if(!dataWulffDlg) return;
	

	for(i=0;i<dataWulffDlg->nMaxSurfaces;i++) for(k=0;k<3;k++) dataWulffDlg->surfaces[i][k] = 0;
	for(i=0;i<dataWulffDlg->nMaxSurfaces;i++) dataWulffDlg->layers[i] = 0;

	/* printf("nMaxSurfaces = %d\n", dataWulffDlg->nMaxSurfaces);*/
	for(i=0;i<dataWulffDlg->nMaxSurfaces;i++)
	{
		for(k=0;k<4;k++)
		if(entrys[i][k]) 
		{
			str = gtk_entry_get_text(GTK_ENTRY(entrys[i][k]));
			if(!str|| strlen(str)<1) continue;
			if(k<3) dataWulffDlg->surfaces[i][k] = atof(str);
			if(k==3) dataWulffDlg->layers[i] = atof(str);
		}
	}
	dataWulffDlg->nSurfaces=0;
	for(i=0;i<dataWulffDlg->nMaxSurfaces;i++) 
		if(fabs(dataWulffDlg->layers[i])>1e-14) dataWulffDlg->nSurfaces++;
		else break;

	createWulffCluster(&crystal->atoms, dataWulffDlg->nSurfaces, dataWulffDlg->surfaces, dataWulffDlg->layers);

	define_geometry_to_draw_from_crystal(crystal);
	unselect_all_atoms();
	reset_all_connections();
	reset_charges_multiplicities();
	drawGeom();
	set_optimal_geom_view();
	create_GeomXYZ_from_draw_grometry();
	destroy_dlg(Dlg,data);
}
/********************************************************************************/
static void  add_wulff_parameters_entrys(GtkWidget *Wins,GtkWidget *vbox, gint nTv, gint nMaxSurfaces)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(nMaxSurfaces+3,4,FALSE);
	GtkWidget*** entrys = NULL;
	gint i;
	gint j;
	gchar* labels[4] = { "h","k","l","#Layers"};

	if(nTv!=3) return;
	entrys = g_malloc(nMaxSurfaces*sizeof(GtkWidget**)); 
	for(i=0;i<nMaxSurfaces;i++) entrys[i] = g_malloc(4*sizeof(GtkWidget*));

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);
	i=0;
	for(j=0;j<4;j++) add_label_table(table,labels[j],(gushort)i,(gushort)j);
	
	for(i=1;i<=nMaxSurfaces;i++)
	for(j=0;j<4;j++) 
	{
		gint k = i-1;
		entrys[k][j] = gtk_entry_new();
		gtk_widget_set_size_request(GTK_WIDGET(entrys[k][j]),(gint)(ScreenHeight*0.15),-1);
		gtk_table_attach(GTK_TABLE(table),entrys[k][j], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	}
	if(nMaxSurfaces>=1)
	{
		gchar* str[]={"0","0","1","1.0"};
		i=0;
		for(j=0;j<4;j++) gtk_entry_set_text(GTK_ENTRY(entrys[i][j]),str[j]);
	}
	if(nMaxSurfaces>=2)
	{
		gchar* str[]={"1","-1","0","1.0"};
		i=1;
		for(j=0;j<4;j++) gtk_entry_set_text(GTK_ENTRY(entrys[i][j]),str[j]);
	}
	if(nMaxSurfaces>=3)
	{
		gchar* str[]={"0","1","0","1.0"};
		i=2;
		for(j=0;j<4;j++) gtk_entry_set_text(GTK_ENTRY(entrys[i][j]),str[j]);
	}
	if(nMaxSurfaces>=4)
	{
		gchar* str[]={"1","0","0","1.0"};
		i=3;
		for(j=0;j<4;j++) gtk_entry_set_text(GTK_ENTRY(entrys[i][j]),str[j]);
	}
	g_object_set_data(G_OBJECT(Wins), "Entries", entrys);
	gtk_widget_show_all(table);
}
/**********************************************************************/
void build_wulff_dlg()
{
  GtkWidget *Dlg;
  GtkWidget *Button;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *vboxframe;
  gint nTv;
  Crystal* crystal = init_crystal();

  nTv = crystalloNumberOfTv(crystal->atoms);

  DataWulffDlg* dataWulffDlg =  init_wulff();

  if(nTv != 3 ) 
  {
      	gchar* t = g_strdup_printf(_("Sorry, You must read/build a molecule with 3 Tv vector."));
        GtkWidget* w = Message(t,_("Error"),TRUE);
        g_free(t);
	return;
  }
  Dlg = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dlg),_("Build Cluster using Wulff construction"));
  gtk_window_set_modal (GTK_WINDOW (Dlg), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(Dlg),GTK_WINDOW(GeomDlg));
  g_object_set_data(G_OBJECT(Dlg), "Crystal", crystal);
  g_object_set_data(G_OBJECT(Dlg), "DataWulffDlg", dataWulffDlg);


  add_child(GeomDlg,Dlg,gtk_widget_destroy,_(" Build Wulff"));
  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)destroy_dlg,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->vbox), frame,TRUE,TRUE,0);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  add_wulff_parameters_entrys(Dlg, vboxframe, nTv, dataWulffDlg->nMaxSurfaces);

  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dlg)->action_area), FALSE);
  gtk_widget_realize(Dlg);
  Button = create_button(Dlg,_("Close"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 2);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)destroy_dlg,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);

  Button = create_button(Dlg,_("OK"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 2);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)build_wulff,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);

  gtk_widget_show_all(GTK_DIALOG(Dlg)->vbox);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->action_area);

  gtk_widget_show_now(Dlg);

  /* fit_windows_position(GeomDlg, Dlg);*/

}
/********************************************************************************/
static void  add_slab_parameters_entrys(GtkWidget *Wins,GtkWidget *vbox)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(5,4,FALSE);
	GtkWidget*** entrys = NULL;
	GtkWidget* entryEmptySpace = NULL;
	gint i;
	gint j;
	gint nL=2;
	gchar* labelshkl[3] = { "h","k","l"};
	gchar* labelsXYZ[3] = { "x repeats","y repeats","z repeats"};
	GtkWidget* surfaceXYButton =  NULL;
	GtkWidget* hseparator =  NULL;


	entrys = g_malloc(nL*sizeof(GtkWidget**)); 
	for(i=0;i<nL;i++) entrys[i] = g_malloc(3*sizeof(GtkWidget*));

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);
	i=0;
	for(j=0;j<3;j++) add_label_table(table,labelshkl[j],(gushort)i,(gushort)j);
	i=2;
	for(j=0;j<3;j++) add_label_table(table,labelsXYZ[j],(gushort)i,(gushort)j);
	
	i=1;
	for(j=0;j<3;j++) 
	{
		gint k = i-1;
		entrys[k][j] = gtk_entry_new();
		gtk_widget_set_size_request(GTK_WIDGET(entrys[k][j]),(gint)(ScreenHeight*0.15),-1);
		gtk_table_attach(GTK_TABLE(table),entrys[k][j], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	}
	i=3;
	for(j=0;j<3;j++) 
	{
		gint k = i-2;
		entrys[k][j] = gtk_entry_new();
		gtk_widget_set_size_request(GTK_WIDGET(entrys[k][j]),(gint)(ScreenHeight*0.15),-1);
		gtk_table_attach(GTK_TABLE(table),entrys[k][j], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	}
	{
		gchar* str[]={"1","1","1"};
		i=0;
		for(j=0;j<3;j++) gtk_entry_set_text(GTK_ENTRY(entrys[i][j]),str[j]);
	}
	{
		gchar* str[]={"1","1","2"};
		i=1;
		for(j=0;j<3;j++) gtk_entry_set_text(GTK_ENTRY(entrys[i][j]),str[j]);
	}
	g_object_set_data(G_OBJECT(Wins), "Entries", entrys);

	i=4;
	j=0;
	add_label_table(table,"EmptySpace size",(gushort)i,(gushort)j);
	entryEmptySpace = gtk_entry_new();
	gtk_widget_set_size_request(GTK_WIDGET(entryEmptySpace),(gint)(ScreenHeight*0.15),-1);
	i =4;
	j=1;
	gtk_table_attach(GTK_TABLE(table),entryEmptySpace, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_entry_set_text(GTK_ENTRY(entryEmptySpace),"2");
	i=4;
	j=2;
	add_label_table(table," * Cell zise",(gushort)i,(gushort)j);
	g_object_set_data(G_OBJECT(Wins), "EntryEmptySpace", entryEmptySpace);

	i =5;
	j=0;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),hseparator,j,j+3,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL|GTK_EXPAND),3,3);

	surfaceXYButton =  gtk_check_button_new_with_label (_("Surface normal to z axis"));
	gtk_widget_show (surfaceXYButton);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (surfaceXYButton), TRUE);
	i =6;
	j=0;
	gtk_table_attach(GTK_TABLE(table),surfaceXYButton, j,j+3,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	g_object_set_data(G_OBJECT(Wins), "SurfaceXYButton", surfaceXYButton);

	gtk_widget_show_all(table);
}
/********************************************************************************/
static void build_slab(GtkWidget *Dlg,gpointer data)
{
	GtkWidget*** entrys =  g_object_get_data(G_OBJECT (Dlg), "Entries");
	GtkWidget* entryEmptySpace =  g_object_get_data(G_OBJECT (Dlg), "EntryEmptySpace");
  	DataSlabDlg* dataSlabDlg = g_object_get_data(G_OBJECT (Dlg), "DataSlabDlg");
  	Crystal* crystal = g_object_get_data(G_OBJECT (Dlg), "Crystal");
	GtkWidget* surfaceXYButton =  g_object_get_data(G_OBJECT (Dlg), "SurfaceXYButton");
	gboolean surfaceXY = TRUE;
	gint nL = 2;
	gdouble emptySpace = 2;

	G_CONST_RETURN gchar* str;
	gint i,k;
	gdouble radius = -1;
	gint nSteps[3] = {1,1,1};

	if(!entrys) return;
	if(!dataSlabDlg) return;
	

	for(k=0;k<3;k++) dataSlabDlg->surface[k] = 1;
	for(k=0;k<3;k++) dataSlabDlg->layers[k] = 1;

	for(i=0;i<nL;i++)
	{
		for(k=0;k<3;k++)
		if(entrys[i][k]) 
		{
			str = gtk_entry_get_text(GTK_ENTRY(entrys[i][k]));
			if(!str|| strlen(str)<1) continue;
			if(i==0) dataSlabDlg->surface[k] = atof(str);
			if(i==1) dataSlabDlg->layers[k] = atof(str);
		}
	}
	if(entryEmptySpace) 
	{
		str = gtk_entry_get_text(GTK_ENTRY(entryEmptySpace));
		if(str && strlen(str)>0) emptySpace = atof(str);
	}
	surfaceXY = GTK_TOGGLE_BUTTON (surfaceXYButton)->active;

	createSlab(&crystal->atoms, dataSlabDlg->surface, dataSlabDlg->layers, emptySpace, surfaceXY);

	define_geometry_to_draw_from_crystal(crystal);
	unselect_all_atoms();
	reset_all_connections();
	reset_charges_multiplicities();
	drawGeom();
	set_optimal_geom_view();
	create_GeomXYZ_from_draw_grometry();
	destroy_dlg(Dlg,data);
}
/**********************************************************************/
void build_slab_dlg()
{
  GtkWidget *Dlg;
  GtkWidget *Button;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *vboxframe;
  gint nTv;
  Crystal* crystal = init_crystal();

  nTv = crystalloNumberOfTv(crystal->atoms);

  DataSlabDlg* dataSlabDlg =  init_slab();

  if(nTv != 3 ) 
  {
      	gchar* t = g_strdup_printf(_("Sorry, You must read/build a molecule with 3 Tv vector."));
        GtkWidget* w = Message(t,_("Error"),TRUE);
        g_free(t);
	return;
  }
  Dlg = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dlg),_("Build a slab"));
  gtk_window_set_modal (GTK_WINDOW (Dlg), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(Dlg),GTK_WINDOW(GeomDlg));
  g_object_set_data(G_OBJECT(Dlg), "Crystal", crystal);
  g_object_set_data(G_OBJECT(Dlg), "DataSlabDlg", dataSlabDlg);


  add_child(GeomDlg,Dlg,gtk_widget_destroy,_(" Build Slab"));
  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)destroy_dlg,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->vbox), frame,TRUE,TRUE,0);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  add_slab_parameters_entrys(Dlg, vboxframe);

  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dlg)->action_area), FALSE);
  gtk_widget_realize(Dlg);
  Button = create_button(Dlg,_("Close"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 2);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)destroy_dlg,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);

  Button = create_button(Dlg,_("OK"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 2);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)build_slab,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);

  gtk_widget_show_all(GTK_DIALOG(Dlg)->vbox);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->action_area);

  gtk_widget_show_now(Dlg);

  /*fit_windows_position(GeomDlg, Dlg);*/

}
/********************************************************************************/
gboolean wrap_atoms_to_cell()
{
	Crystal* crystal = init_crystal();
	gint  nTv = crystalloNumberOfTv(crystal->atoms);
	gboolean ok = FALSE;
	if(nTv != 0 ) 
	{
		ok = crystalloSetCartnAtomsInBox(crystal->atoms);
		if(ok) crystalloRemoveAtomsWithSmallDistance(&crystal->atoms);

		//printf("NbAp = %d \n",Nb);
		define_geometry_to_draw_from_crystal(crystal);
		//define_good_factor();
		unselect_all_atoms();

		reset_all_connections();

		reset_charges_multiplicities();
		drawGeom();
		set_optimal_geom_view();
		create_GeomXYZ_from_draw_grometry();
	}
	freeCrystal(crystal);
	g_free(crystal);
	return ok;
}
/***************************************************************************/
void help_references_crystallography()
{
	gchar* temp;
	temp = g_strdup_printf(
		_(
		" * Prototype crystals are collected using :\n"
	        "       The Structure of Materials, Marc De Graef and Michael McHenry, Cambridge University Press;\n"
		"       M. J. Mehl, D. Hicks, C. Toher, O. Levy, R. M. Hanson, G. L. W. Hart, and S. Curtarolo,\n"
	        "       http://som.web.cmu.edu \n"
		"       The AFLOW Library of Crystallographic Prototypes: Part 1, Comp. Mat. Sci. 136, S1-S828 (2017).\n"
		"       https://doi.org/10.1016/j.commatsci.2017.01.017 and http://aflow.org/CrystalDatabase\n"
		"       http://www.catalysthub.net/\n"
		"       http://www.crystallography.net\n"
		"       \n"
		" * Slab building implementation is based on method described in :\n"
	        "       Theory and implementation behind:Universal surface creation - smallest unitcell.\n"
		"       Bjarke Brink Buus, Jakob Howalt & Thomas Bligaard.\n"
		"       wiki.fysik.dtu.dk/ase/_downloads/general_surface.pdf.\n"
		"       \n"
		" * Spglib is used to compute space group, to make reduction and to standardize the cell :\n"
	        "       https://atztogo.github.io/spglib/index.html and references cited in this web page\n"
		"       See also 𝚂𝚙𝚐𝚕𝚒𝚋: a software library for crystal symmetry search”\n"
		"       Atsushi Togo and Isao Tanaka, https://arxiv.org/abs/1808.01590 (written at version 1.10.4)\n"
		"       \n"
		" *  To generate kpoints for band structure calculations, Gabedit use :\n"
		"       the methode described in : Hinuma et al. Comput. Mat. Science 128 (2017) 140–184\n"
		"       and 𝚂𝚙𝚐𝚕𝚒𝚋 library\n"
		)
		 );
	Message(temp,_("Info"),FALSE);
	g_free(temp);
}
/********************************************************************************/
static gboolean reduce_cell(GabeditCrystalloReductionType type)
{
	gdouble symprec=get_symprec_from_geomdlg();
	Crystal* crystal = init_crystal();
	gint  nTv = crystalloNumberOfTv(crystal->atoms);
	gboolean ok = FALSE;
	if(nTv == 3 ) 
	{
		ok = crystalloReduceCell(&crystal->atoms, symprec, type);
		if(ok) crystalloRemoveAtomsWithSmallDistance(&crystal->atoms);

		//printf("NbAp = %d \n",Nb);
		define_geometry_to_draw_from_crystal(crystal);
		//define_good_factor();
		unselect_all_atoms();

		reset_all_connections();

		reset_charges_multiplicities();
		drawGeom();
		set_optimal_geom_view();
		create_GeomXYZ_from_draw_grometry();
	}
	freeCrystal(crystal);
	g_free(crystal);
	return ok;
}
/********************************************************************************/
gboolean reduce_cell_niggli()
{
	return reduce_cell(GABEDIT_CRYSTALLO_REDUCTION_NIGGLI);
}
/********************************************************************************/
gboolean reduce_cell_delaunay()
{
	return reduce_cell(GABEDIT_CRYSTALLO_REDUCTION_DELAUNAY);
}
/********************************************************************************/
gboolean reduce_cell_primitive()
{
	return reduce_cell(GABEDIT_CRYSTALLO_REDUCTION_PRIMITIVE);
}
/***************************************************************************/
void compute_space_symmetry_group()
{
	gchar* temp;
	char groupName[20];
	gdouble symprec=get_symprec_from_geomdlg();
	Crystal* crystal = init_crystal();
	gint numGroup = crystalloGetSpaceSymmetryGroup(crystal->atoms, groupName, symprec);
	freeCrystal(crystal);
	g_free(crystal);
	temp = g_strdup_printf(
		" Group name = %s (%d)\n"
		, groupName, numGroup
		 );
	Message(temp,_("Info"),FALSE);
	g_free(temp);
}
/***************************************************************************/
void compute_symmetry_info()
{
	gchar* temp;
	gdouble symprec=get_symprec_from_geomdlg();
	Crystal* crystal = init_crystal();
	temp = crystalloGetSymmetryInfo(crystal, symprec);
	freeCrystal(crystal);
	g_free(crystal);

	if(strstr(temp,"Error")) Message(temp,_("Error"),FALSE);
	else MessageTxt(temp,_("Info"));
	g_free(temp);
}
/***************************************************************************/
void standardize_cell(gint to_primitive, gint no_idealize, gdouble symprec)
{
	Crystal* crystal = init_crystal();
	gboolean ok = crystalloStandardizeCell(&crystal->atoms,  to_primitive, no_idealize, symprec);

	if(ok) 
	{
		ok = crystalloSetCartnAtomsInBox(crystal->atoms);
		if(ok) crystalloRemoveAtomsWithSmallDistance(&crystal->atoms);

		//printf("NbAp = %d \n",Nb);
		define_geometry_to_draw_from_crystal(crystal);
		//define_good_factor();
		unselect_all_atoms();

		reset_all_connections();

		reset_charges_multiplicities();
		drawGeom();
		set_optimal_geom_view();
		create_GeomXYZ_from_draw_grometry();
	}
	freeCrystal(crystal);
	g_free(crystal);
}
/***************************************************************************/
void standardize_cell_primitive()
{
	gdouble symprec=get_symprec_from_geomdlg();
	gint to_primitive = 1;
	gint no_idealize = 0;
	standardize_cell(to_primitive, no_idealize, symprec);
}
/***************************************************************************/
void standardize_cell_conventional()
{
	gdouble symprec=get_symprec_from_geomdlg();
	gint to_primitive = 0;
	gint no_idealize = 0;
	standardize_cell(to_primitive, no_idealize, symprec);
}
/********************************************************************************/
void setSymmetryPrecision(GtkWidget* GeomDlg, G_CONST_RETURN gchar* value)
{
	gchar* s;

	if(!GeomDlg) return;

	s = g_object_get_data(G_OBJECT(GeomDlg), "SymPrecision");
	if(s) g_free(s);
	s = g_strdup(value);
	g_object_set_data(G_OBJECT(GeomDlg), "SymPrecision",s);
}
/********************************************************************************/
static void setSymPrec(GtkWidget* button, GtkWidget* entry)
{
	gint i;
	gint k;
	G_CONST_RETURN gchar *strEntry;
	gdouble symprec = 1e-4;
	static gchar s[BSIZE];

	if(!GeomDlg) return;

	strEntry = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(strEntry)<1) return;
	setSymmetryPrecision(GeomDlg, strEntry);
}
/********************************************************************************/
void setSymPrecDlg()
{
	GtkWidget *winDlg;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gchar* tmp = NULL;
  
	if(!GeomDlg) return;
	winDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Set symmetry precision for crystallography tools"));
	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(GeomDlg));

	add_child(GeomDlg,winDlg,gtk_widget_destroy,_(" Sym. Prec. "));
	g_signal_connect(G_OBJECT(winDlg),"delete_event",(GCallback)delete_child,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	hbox=create_hbox_false(vboxframe);
	label = gtk_label_new(_(" Symmetry Precision : "));
	gtk_box_pack_start( GTK_BOX(hbox), label,TRUE,TRUE,0);
	entry = gtk_entry_new();
	tmp =  g_object_get_data(G_OBJECT(GeomDlg), "SymPrecision");

        if(tmp) gtk_entry_set_text(GTK_ENTRY(entry),tmp);
	else gtk_entry_set_text(GTK_ENTRY(entry),"1e-4");

	gtk_box_pack_start( GTK_BOX(hbox), entry,TRUE,TRUE,0);

	gtk_widget_realize(winDlg);

	button = create_button(winDlg,_("Cancel"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	button = create_button(winDlg,_("OK"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)setSymPrec,entry);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);

	gtk_widget_show_all(winDlg);
}
/***************************************************************************/
void compute_kpoints_path()
{
	gchar* temp;
	gdouble symprec=get_symprec_from_geomdlg();
	Crystal* crystal = init_crystal();
	temp = getVASPKPointsPath(crystal, symprec);
	freeCrystal(crystal);
	g_free(crystal);

	if(strstr(temp,"Error")) Message(temp,_("Error"),FALSE);
	else MessageTxt(temp,_("Info"));
	g_free(temp);
}
/***************************************************************************/
