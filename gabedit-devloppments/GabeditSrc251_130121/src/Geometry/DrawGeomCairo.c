/* DrawGeom.c */
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


#ifndef DRAWGEOMGL
#include "../../Config.h"
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <cairo-svg.h>

#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/PovrayUtils.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Measure.h"
#include "../Geometry/Povray.h"
#include "../Common/Windows.h"
#include "../Utils/Transformation.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/GeomZmatrix.h"
#include "../Geometry/GeomSymmetry.h"
#include "../Files/FileChooser.h"
#include "../Geometry/ImagesGeom.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/FragmentsPPD.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/RotFragments.h"
#include "../Geometry/GeomConversion.h"
#include "../Geometry/PersonalFragments.h"
#include "../Geometry/ResultsAnalise.h"
#include "../Utils/HydrogenBond.h"
#include "../MolecularMechanics/PDBTemplate.h"
#include "../MolecularMechanics/CalculTypesAmber.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"
#include "../Utils/Jacobi.h"
#include "../Utils/Vector3d.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Utils/UtilsCairo.h"
#include "../Geometry/MenuToolBarGeom.h"
#include "../Geometry/PreviewGeom.h"
#include "../Geometry/FragmentsSelector.h"
#include "../Geometry/SelectionDlg.h"
#include "../IsotopeDistribution/IsotopeDistributionCalculatorDlg.h"
#include "../Geometry/TreeMolecule.h"
#include "../Geometry/BuildCrystal.h"

/* extern of .h */
FragmentsItems *FragItems;
gint NFrags;

CoordMaxMin coordmaxmin;

GeomDef *geometry;
GeomDef *geometry0;
guint Natoms;

gint *NumFatoms;
guint NFatoms;

gint TransX;
gint TransY;
GtkWidget *GeomDlg;
GtkWidget *StopButton;
gboolean StopCalcul;

gboolean ShadMode;
gboolean PersMode;
gboolean LightMode;
gboolean OrtepMode;
gboolean DrawDistance;
gboolean DrawDipole;
gboolean ShowDipole;
gboolean ShowHBonds;

gdouble dipole[NDIVDIPOLE][3];
gdouble dipole0[NDIVDIPOLE][3];
gdouble dipole00[NDIVDIPOLE][3];
gint DXi[NDIVDIPOLE];
gint DYi[NDIVDIPOLE];
gint Ndipole[NDIVDIPOLE];
gchar* AtomToInsert;
gint NumSelAtoms[4];
gboolean Ddef;



/********************************************************************************/
#define MAT 30
#define SCALE(i) (i / 65535.)
/********************************************************************************/

static gdouble Quat[4] = {0.0,0.0,0.0,1.0};
static gdouble QuatFrag[4] = {0.0,0.0,0.0,1.0};
static gdouble QuatAtom[4] = {0.0,0.0,0.0,1.0};
static gdouble Orig[3] = {0.0,0.0,0.0};
static gdouble BeginX = 0;
static gdouble BeginY = 0;
static Camera camera = { 10,5};

static gdouble CSselectedAtom[3] = {0.0,0.0,0.0};
static gint NumSelectedAtom = -1;
static gint NumProcheAtom = -1;
static gboolean ButtonPressed = FALSE;
static gboolean ShiftKeyPressed = FALSE;
static gboolean ControlKeyPressed = FALSE;
static gboolean FKeyPressed = FALSE;
static gboolean GKeyPressed = FALSE;

gchar* strToDraw = NULL;

static gdouble minDistanceH = 1.50; /* in Agnstrom */
static gdouble maxDistanceH = 3.15; /* in Agnstrom */ 
static gdouble minAngleH = 145.0;
static gdouble maxAngleH = 215.0;

static gboolean showMultipleBonds = TRUE;
static gboolean CartoonMode = TRUE;
static gboolean ShowHydrogenAtoms = TRUE;
static gboolean AdjustHydrogenAtoms = TRUE;
static gboolean RebuildConnectionsDuringEdition = FALSE;

static gint NumBatoms[2] = {-1,-1};
static gint NBatoms = 0;

static gint atomToDelete = -1;
static gint atomToBondTo = -1;
static gint angleTo = -1;
static gdouble fragAngle = 180.0;

static gdouble factor = 1.0;
static gdouble factorstick = 1.0;
static gdouble factorball = 1.0;
static gdouble factordipole = 1.0;

static gboolean buttonpress = FALSE;

static GabEditTypeGeom TypeGeom = GABEDIT_TYPEGEOM_STICK;

/********************************************************************************/
void set_statubar_pop_sel_atom();
void calcul_ndipole();
void redefine_dipole();
void drawGeom_dipole();
GtkWidget *AddNoteBookPage(GtkWidget *NoteBook,char *label);
void drawGeom();
void destroy_all_drawing(GtkWidget *win);
void ActivateButtonOperation (GtkWidget *widget, guint data);
void delete_selected_atoms();
void delete_selected_bond();
void change_selected_bond();
void add_bond();
gint unselected_atom(GdkEventButton *bevent);
static gint insert_atom(GdkEventButton *event);
static gint insert_fragment(GtkWidget *widget,GdkEvent *event);
void define_good_factor();
void set_optimal_geom_view();
void define_coefs_pers();
gboolean if_selected(gint Num);
gint index_selected(gint Num);
void sort_with_zaxis();
void define_geometry();
void buildRotation();
void deleteHydrogensConnectedTo(gint n, gint nH);
void delete_one_atom(gint NumDel);
static gint replace_atom();
/********************************************************************************/
static  GdkGC* gc=NULL;
static	GdkColor* BackColor=NULL;
static GdkPixmap *pixmap = NULL;
static cairo_t *cr = NULL;
static 	GtkWidget *NoteBookDraw;
static	GtkWidget *vboxmeasure;
static gdouble TCOS[91],TSIN[91];
static gdouble CenterCoor[2];
static GtkWidget *vboxhandle;
static GtkWidget *StatusRotation = NULL;
static GtkWidget *StatusPopup = NULL;
static GtkWidget *StatusOperation = NULL;
static guint idStatusRotation = 0;
static guint idStatusPopup = 0;
static guint idStatusOperation = 0;
static GabEditGeomOperation OperationType = ROTATION;
static GabEditSelectType SelectType = RECTANGLE;
guint LabelOption = LABELNO;

static Fragment Frag = {0,NULL};
static cairo_t *crExport = NULL;
static GList* fifoGeometries = NULL;
static GList* currentFifoGeometries = NULL;
static gboolean oldNext = FALSE;

/**********************************************************************************/
static void stop_calcul(GtkWidget *wi, gpointer data);
void delete_all_selected_atoms();
static void reset_connections_between_selected_atoms();
static void reset_connections_between_selected_and_notselected_atoms();
/*********************************************************************************************/
gboolean testShowBoxGeom()
{
        return FALSE;
}
/*********************************************************************************************/
void getQuatGeom(gdouble q[])
{
	gint i;
	/*for(i=0;i<4;i++) q[i] = Quat[i];*/
	for(i=0;i<4;i++) q[i] = 0.0;
}
/*********************************************************************************************/
void  reset_origine_molecule_drawgeom()
{
	Orig[0] = 0.0;
	Orig[1] = 0.0;
	Orig[2] = 0.0;
}
/*********************************************************************************************/
void  get_origine_molecule_drawgeom(gdouble orig[])
{
	orig[0] = Orig[0];
	orig[1] = Orig[1];
	orig[2] = Orig[2];
}
/*********************************************************************************************/
gdouble  get_symprec_from_geomdlg()
{
	gdouble symprec = 1e-4;
	gchar* tmp = g_object_get_data(G_OBJECT(GeomDlg), "SymPrecision");
	if(tmp) symprec = atof(tmp);
	return symprec;
}
/**********************************************************************************/
static gchar* getFormulaOfTheMolecule()
{
	gint i,j;
	gchar* formula = NULL;
	gchar* dum = NULL;
	gint *tag = NULL;
	if(Natoms<1) return formula;
	tag = g_malloc(Natoms*sizeof(gint));
	if(!tag) return formula;
        for(i=0;i<Natoms;i++) tag[i] = 1;

        for(i=0;i<Natoms-1;i++)
	{
		if(tag[i]<=0) continue;
        	for(j=i+1;j<Natoms;j++)
			if(!strcmp(geometry0[i].Prop.symbol,geometry0[j].Prop.symbol)) 
			{
				tag[i]++;
				tag[j]--;
			}
	}
	formula = g_strdup("");
        for(i=0;i<Natoms;i++)
	{
		if(tag[i]<=0) continue;
		dum = formula;
		formula = g_strdup_printf("%s%s%d",formula,geometry0[i].Prop.symbol,tag[i]);
		g_free(dum);
	}
	if(tag) g_free(tag);
	if(formula && strlen(formula)<1)
	{
		g_free(formula);
		formula = NULL;
		return formula;

	}
	return formula;
}
/**********************************************************************************/
void createIstopeDistributionCalculationFromDrawGeom()
{
	gchar* formula = getFormulaOfTheMolecule();
	compute_distribution_dlg(Fenetre, formula);
}
/*********************************************************************************************/
void  copy_screen_geom_clipboard()
{
	 while( gtk_events_pending() ) gtk_main_iteration();
	gabedit_save_image(GeomDrawingArea, NULL, NULL);
}
/*********************************************************************************************/
void  get_orgin_molecule_drawgeom(gdouble orig[])
{
	gdouble Rmax, Cmax;
	orig[0] = 0;
	orig[1] = 0;
	orig[2] = 0.0;

	Rmax = GeomDrawingArea->allocation.width;
	if(Rmax<GeomDrawingArea->allocation.height) Rmax = GeomDrawingArea->allocation.height;

	if(PersMode) Cmax  = coordmaxmin.Cmax*camera.f/(camera.position);
	else Cmax = coordmaxmin.Cmax;
	orig[0] = TransX *2*Cmax/factor/Rmax; 
	orig[1] = -TransY *2*Cmax/factor/Rmax; 
}
/*********************************************************************************************/
void  get_camera_values_drawgeom(gdouble* zn, gdouble* zf, gdouble* angle, gdouble* aspect, gboolean* persp)
{
	gdouble width = 500;
	gdouble height = 500;

	if(GeomDrawingArea)
	{
		width =  GeomDrawingArea->allocation.width;
		height = GeomDrawingArea->allocation.height;
	}
	*aspect = width/height;
	if(PersMode)
	{
		*zf = camera.position;
		*zn = camera.f;
		*angle = 45.0*1.2/factor;
		if(*angle>=180) *angle = 160;
	}
	else
	{
		gdouble Cmax = coordmaxmin.Cmax;
		gdouble Rmax = GeomDrawingArea->allocation.width;

		if(Rmax<GeomDrawingArea->allocation.height) Rmax = GeomDrawingArea->allocation.height;
		*zf = Cmax*Rmax;
		*zn = 10;
		*angle = *aspect/factor*45*36*(Cmax/Rmax);
		if(*angle>=180) *angle = 160;
	}
	*persp = PersMode;
}
/**********************************************************************************/
static void free_geometries_from_current_to_end()
{
	if( !fifoGeometries) return;
	if( !currentFifoGeometries) return;
	if( g_list_last(fifoGeometries) == currentFifoGeometries) return;
	if( fifoGeometries && currentFifoGeometries)
	{
		GList* list;
		if(currentFifoGeometries)
		for(list=currentFifoGeometries;list!=NULL;list=list->next)
		{
			GeomDraw* geom = NULL; 
			geom = (GeomDraw*)list->data;
			if(geom && geom->atoms ) Free_One_Geom(geom->atoms,geom->nAtoms);
			list->data = NULL;
		}
		if(currentFifoGeometries->prev) 
			currentFifoGeometries->prev->next = NULL;
		else fifoGeometries = NULL;
		g_list_free(currentFifoGeometries);
		currentFifoGeometries = g_list_last(fifoGeometries);
	}
}
/**********************************************************************************/
void add_geometry_to_fifo()
{
	gint i,j;
	GeomDraw* geom = g_malloc(sizeof(GeomDraw));
	geom->nAtoms = Natoms;
	if(Natoms>0) geom->atoms = g_malloc(Natoms*sizeof(GeomDef));
	else geom->atoms = NULL;
        for(i=0;i<Natoms;i++)
	{
		geom->atoms[i].X = geometry0[i].X;
		geom->atoms[i].Y = geometry0[i].Y;
		geom->atoms[i].Z = geometry0[i].Z;
		geom->atoms[i].Xi = geometry0[i].Xi;
		geom->atoms[i].Yi = geometry0[i].Yi;
		geom->atoms[i].Prop = prop_atom_get(geometry0[i].Prop.symbol);
		geom->atoms[i].mmType = g_strdup(geometry0[i].mmType);
		geom->atoms[i].pdbType = g_strdup(geometry0[i].pdbType);
		geom->atoms[i].Residue = g_strdup(geometry0[i].Residue);
		geom->atoms[i].ResidueNumber = geometry0[i].ResidueNumber;
		geom->atoms[i].show = geometry0[i].show;
		geom->atoms[i].Charge = geometry0[i].Charge;
		geom->atoms[i].Layer = geometry0[i].Layer;
		geom->atoms[i].N = geometry0[i].N;
		geom->atoms[i].Variable = geometry0[i].Variable;
		geom->atoms[i].ColorAlloc = geometry0[i].ColorAlloc;
		geom->atoms[i].Rayon = geometry0[i].Rayon;
		geom->atoms[i].Coefpers = geometry0[i].Coefpers;
		if(geometry0[i].typeConnections)
		{
			geom->atoms[i].typeConnections = g_malloc(Natoms*sizeof(gint));
			for(j=0;j<Natoms;j++) 
				geom->atoms[i].typeConnections[j] = geometry0[i].typeConnections[j];
		}
		else geom->atoms[i].typeConnections = NULL;
	}
	free_geometries_from_current_to_end();
	fifoGeometries = g_list_append(fifoGeometries,geom);
	currentFifoGeometries = g_list_last(fifoGeometries);
	oldNext = FALSE;
}
/**********************************************************************************/
void get_geometry_from_fifo(gboolean toNext)
{
	gint i,j;
	GeomDraw* geom = NULL; 
	GList* list = NULL;
	if(!fifoGeometries) return;
	if(!currentFifoGeometries) return;
	if(toNext) 
	{
		GList* last = g_list_last(fifoGeometries);
		if(currentFifoGeometries == last) return;
		list = g_list_next(currentFifoGeometries);
		currentFifoGeometries  = list;
		oldNext = TRUE;

	}
	else 
	{
		if( !oldNext && currentFifoGeometries == g_list_last(fifoGeometries))
		{
			add_geometry_to_fifo();
		}
		if(g_list_previous(currentFifoGeometries))
			currentFifoGeometries = g_list_previous(currentFifoGeometries);
		list = currentFifoGeometries;
		oldNext = FALSE;
	}
	geom = (GeomDraw*)list->data;
	if(!geom) return;
	geometry0 = Free_One_Geom(geometry0,Natoms);
	geometry = Free_One_Geom(geometry,Natoms);
	Natoms = geom->nAtoms;
	if(Natoms>0) 
	{
		geometry0 = g_malloc(Natoms*sizeof(GeomDef));
		geometry = g_malloc(Natoms*sizeof(GeomDef));
	}
        for(i=0;i<Natoms;i++)
	{
		geometry0[i].X = geom->atoms[i].X;
		geometry0[i].Y = geom->atoms[i].Y;
		geometry0[i].Z = geom->atoms[i].Z;
		geometry0[i].Xi = geom->atoms[i].Xi;
		geometry0[i].Yi = geom->atoms[i].Yi;
		geometry0[i].Prop = prop_atom_get(geom->atoms[i].Prop.symbol);
		geometry0[i].mmType = g_strdup(geom->atoms[i].mmType);
		geometry0[i].pdbType = g_strdup(geom->atoms[i].pdbType);
		geometry0[i].Residue = g_strdup(geom->atoms[i].Residue);
		geometry0[i].ResidueNumber = geom->atoms[i].ResidueNumber;
		geometry0[i].show = geom->atoms[i].show;
		geometry0[i].Charge = geom->atoms[i].Charge;
		geometry0[i].Layer = geom->atoms[i].Layer;
		geometry0[i].N = geom->atoms[i].N;
		geometry0[i].Variable = geom->atoms[i].Variable;
		geometry0[i].ColorAlloc = geom->atoms[i].ColorAlloc;
		geometry0[i].Rayon = geom->atoms[i].Rayon;
		geometry0[i].Coefpers = geom->atoms[i].Coefpers;
		if(geom->atoms[i].typeConnections)
		{
			geometry0[i].typeConnections = g_malloc(Natoms*sizeof(gint));
			for(j=0;j<Natoms;j++) 
				geometry0[i].typeConnections[j] = geom->atoms[i].typeConnections[j];
		}
		else 
		{
			geometry0[i].typeConnections = NULL;
		}
	}
        for(i=0;i<Natoms;i++)
	{
		geometry[i].X = geom->atoms[i].X;
		geometry[i].Y = geom->atoms[i].Y;
		geometry[i].Z = geom->atoms[i].Z;
		geometry[i].Xi = geom->atoms[i].Xi;
		geometry[i].Yi = geom->atoms[i].Yi;
		geometry[i].Prop = prop_atom_get(geom->atoms[i].Prop.symbol);
		geometry[i].mmType = g_strdup(geom->atoms[i].mmType);
		geometry[i].pdbType = g_strdup(geom->atoms[i].pdbType);
		geometry[i].Residue = g_strdup(geom->atoms[i].Residue);
		geometry[i].ResidueNumber = geom->atoms[i].ResidueNumber;
		geometry[i].show = geom->atoms[i].show;
		geometry[i].Charge = geom->atoms[i].Charge;
		geometry[i].Layer = geom->atoms[i].Layer;
		geometry[i].N = geom->atoms[i].N;
		geometry[i].Variable = geom->atoms[i].Variable;
		geometry[i].ColorAlloc = geom->atoms[i].ColorAlloc;
		geometry[i].Rayon = geom->atoms[i].Rayon;
		geometry[i].Coefpers = geom->atoms[i].Coefpers;
		if(geom->atoms[i].typeConnections)
		{
			geometry[i].typeConnections = g_malloc(Natoms*sizeof(gint));
			for(j=0;j<Natoms;j++) 
				geometry[i].typeConnections[j] = geom->atoms[i].typeConnections[j];
		}
		else 
		{
			geometry[i].typeConnections = NULL;
		}
	}
	/* if(!fifoGeometries) printf("fifoGeometries is void\n");*/
	change_of_center(NULL,NULL);
	create_GeomXYZ_from_draw_grometry();
}
/**********************************************************************************/
static gint get_indice(gint n)
{
	gint i;
	if(n<0) return n;
	for(i=0;i<(gint)Natoms;i++)
		if(geometry[i].N==n)return i;
	return -1;
}
/*****************************************************************************/
static gushort get_epaisseur()
{
        gushort e;
	if(Natoms>0) e = (gushort)(7.0/1200*GeomDrawingArea->allocation.width*factorstick);
	else e = 3;
	if(e<3) e = 3;
	return e;
}
/*****************************************************************************/
static gushort get_rayon(gint i)
{
        gushort rayon;
        if ( !stick_mode() && geometry[i].Layer != LOW_LAYER )
        { 
                rayon =(gushort)(0.8*geometry[i].Rayon*factorball);
    		if (PersMode) rayon =(gushort)(geometry[i].Coefpers*geometry[i].Rayon*factorball);
		if(geometry[i].Layer == LOW_LAYER) rayon /= 6;
		if(geometry[i].Layer == MEDIUM_LAYER) rayon /= 2;
	}
	else rayon = get_epaisseur();
	if(rayon<5) rayon = 5;
	return rayon;
}
/**********************************************************************************/
void create_drawmolecule_file()
{
	gchar *drawMolecule;
	FILE *fd;

	drawMolecule = g_strdup_printf("%s%sdrawmolecule",gabedit_directory(),G_DIR_SEPARATOR_S);

	fd = FOpen(drawMolecule, "w");
	if(fd !=NULL)
	{
		if(BackColor) fprintf(fd,"%d %d %d\n",BackColor->red, BackColor->green, BackColor->blue);
		else fprintf(fd,"0 0 0\n");
		fprintf(fd,"%d\n",AdjustHydrogenAtoms);
		fprintf(fd,"%d\n",RebuildConnectionsDuringEdition);
		fprintf(fd,"%f\n",fragAngle);
		fprintf(fd,"%f\n",factorstick);
		fprintf(fd,"%f\n",factorball);
		fclose(fd);
	}
	g_free(drawMolecule);
}
/*************************************************************************************/
void read_drawmolecule_file()
{
	gchar *drawMolecule;
	FILE *fd;
	gint r = 0;
	gint g = 0;
	gint b = 0;
	gint sh = 1;
	gint rc = 0;

	drawMolecule = g_strdup_printf("%s%sdrawmolecule",gabedit_directory(),G_DIR_SEPARATOR_S);
	fragAngle = 180;
	factor_stick_default(NULL,NULL);
	factor_ball_default(NULL,NULL);

	fd = fopen(drawMolecule, "rb");
	if(fd !=NULL)
	{
 		guint taille = BSIZE;
 		gchar t[BSIZE];
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d %d %d",&r, &g, &b)!=3)
			{
				printf("t=%s\n",t);
				r = 0;
				g = 0;
				b = 0;
			}
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d",&sh)!=1) sh = 1;
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%d",&rc)!=1) rc = 0;
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%lf",&fragAngle)!=1) fragAngle = 180;
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%lf",&factorstick)!=1) factor_stick_default(NULL,NULL);
 		if(fgets(t,taille,fd))
			if(sscanf(t,"%lf",&factorball)!=1) factor_ball_default(NULL,NULL);

		fclose(fd);
	}
	g_free(drawMolecule);
	if(r==0 && g == 0 && b == 0) r = g = b = 20000;
	{
		BackColor = g_malloc(sizeof(GdkColor));
		BackColor->red = r;
		BackColor->green = g;
		BackColor->blue = b;

	}
	AdjustHydrogenAtoms = (gboolean)sh;
	RebuildConnectionsDuringEdition = (gboolean)rc;
}
/*************************************************************************************/
GeomDef* copyGeometry(GeomDef* geom0)
{
	gint i;
	GeomDef* geom = NULL;
	if(Natoms<1) return geom;
	geom =g_malloc(Natoms*sizeof(GeomDef));
	for(i=0;i<Natoms;i++)
	{
		geom[i].X = geom0[i].X;
		geom[i].Y = geom0[i].Y;
		geom[i].Z = geom0[i].Z; 
		geom[i].N = geom0[i].N; 
        	geom[i].typeConnections = NULL;
		geom[i].Prop = prop_atom_get(geom0[i].Prop.symbol);
		geom[i].mmType = g_strdup(geom0[i].mmType);
		geom[i].pdbType = g_strdup(geom0[i].pdbType);
		geom[i].Layer = geom0[i].Layer;
		geom[i].Variable = geom0[i].Variable;
		geom[i].Residue = g_strdup(geom0[i].Residue);
		geom[i].ResidueNumber = geom0[i].ResidueNumber;
		geom[i].show = geom0[i].show;
		geom[i].Charge = geom0[i].Charge;
	}
	return geom;
}
/*****************************************************************************/
void freeGeometry(GeomDef* geom)
{
	gint i;
	if(!geom) return;
	if(Natoms<1) return;
	for(i=0;i<Natoms;i++)
	{
		if(geom[i].Prop.name) g_free(geom[i].Prop.name);
		if(geom[i].Prop.symbol) g_free(geom[i].Prop.symbol);
		if(geom[i].mmType) g_free(geom[i].mmType);
		if(geom[i].pdbType) g_free(geom[i].pdbType);
		if(geom[i].Residue) g_free(geom[i].Residue);
		if(geom[i].typeConnections) g_free(geom[i].typeConnections);
	}
	g_free(geom);
}
/*****************************************************************************/
gdouble get_factorstick()
{
	return factorstick;
}
/*****************************************************************************/
gdouble get_factorball()
{
	return factorball;
}
/*****************************************************************************/
gdouble get_factordipole()
{
	return factordipole;
}
/*****************************************************************************/
gdouble get_factor()
{
	return factor;
}
/*****************************************************************************/
gint get_connection_type(gint i, gint j)
{
	gint nj = 0;
	if(i<0 || j<0 || i>=Natoms || j>=Natoms ) return 0;
	if(!geometry[i].typeConnections)return 0;
	nj = geometry[j].N-1;
	if(geometry[i].typeConnections[nj]>0) return geometry[i].typeConnections[nj];
	return 0;
}
/*****************************************************************************/
static void set_fix_variable_of_selected_atoms(gboolean variable)
{
	gint i;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(if_selected(i))
		{
			geometry[i].Variable = variable;
			geometry0[i].Variable = variable;
		}
	}
	create_GeomXYZ_from_draw_grometry();
	drawGeom();
}
/*****************************************************************************/
void set_fix_selected_atoms()
{
	gint i;
	gint nf=0;
	for (i=0;i<(gint)Natoms;i++)
			if(geometry[i].Variable) nf++;
	if(nf==0)
	{
		for (i=0;i<(gint)Natoms;i++)
		{
			geometry[i].Variable=TRUE;
			geometry0[i].Variable = TRUE;
		}
	}
	set_fix_variable_of_selected_atoms(FALSE);
}
/*****************************************************************************/
void set_variable_selected_atoms()
{
	set_fix_variable_of_selected_atoms(TRUE);
}
/*****************************************************************************/
void hide_selected_atoms()
{
	gint i;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(if_selected(i))
		{
			geometry[i].show = FALSE;
			geometry0[i].show = FALSE;
		}
	}
	unselect_all_atoms();
	drawGeom();
}
/*****************************************************************************/
void hide_not_selected_atoms()
{
	gint i;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!if_selected(i))
		{
			geometry[i].show = FALSE;
			geometry0[i].show = FALSE;
		}
	}
	drawGeom();
}
/*****************************************************************************/
void show_hydrogen_atoms()
{
	gint i;
	gint j;
	gint ni;
	gint nj;
	for (i=0;i<(gint)Natoms;i++)
	{
		ni = geometry[i].N-1;
		if(!geometry[i].show) continue;
		for (j=0;j<(gint)Natoms;j++)
		{
			nj = geometry[j].N-1;
			if(!geometry[i].typeConnections) continue;
			if(geometry[i].typeConnections[nj]<1) continue;
			if(!strcmp(geometry[j].Prop.symbol,"H")) 
			{
				geometry[j].show = TRUE;
				geometry0[j].show = TRUE;
			}
		}

	}
	drawGeom();
	ShowHydrogenAtoms = TRUE;
}
/*****************************************************************************/
void show_all_atoms()
{
	gint i;
	for (i=0;i<(gint)Natoms;i++)
	{
		geometry[i].show = TRUE;
		geometry0[i].show = TRUE;
	}
	drawGeom();
	ShowHydrogenAtoms = TRUE;
}
/********************************************************************************/
static gint testAmberTypesDefine()
{
	gint i;
	gint k;
	if(Natoms<1)return 0;
	for(i=0;i<(gint)Natoms;i++)
		if(geometry[i].mmType && strlen(geometry[i].mmType)>2) 
		{
			printf("Problem with %s\n",geometry[i].mmType);
			return -1;
		}
	k = 0;
	for(i=0;i<(gint)Natoms;i++)
		if(geometry[i].mmType && strcmp(geometry[i].mmType,geometry[i].Prop.symbol)==0) k++;
	if(k==Natoms) return -2;
	return 0;
}
/********************************************************************************/
void messageAmberTypesDefine()
{
	gint k = testAmberTypesDefine();
	if(k==0) return;
	if(k==-1)
	{
    		GtkWidget* m;
		m = Message(_("The type of One (or several) of atoms is not a Amber type.\n"
		        "You can set the types of atoms by : \n"
		        "                  \"Set/Atom Type&Charge using PDB Template\" \n"
		        "                  \"Or\" \n"
		        "                  \"Set/Atom Types using connections types\" \n"
			)
			,_("Warning"),TRUE);
		gtk_window_set_modal (GTK_WINDOW (m), TRUE);
	}
	if(k==-2)
	{
    		GtkWidget* m;
		m = Message(_("The types of the atoms are identical to the symbols of these atoms.\n"
		        "You can set the types of atoms by \n"
		        "\"Set/Atom Types using connections types\" \n"
			)
			,_("Warning"),TRUE);
		gtk_window_set_modal (GTK_WINDOW (m), TRUE);
	}
}
/*****************************************************************************/
gboolean getShowMultipleBonds()
{
	return showMultipleBonds;
}
/*****************************************************************************/
void RenderMultipleBonds(GtkWidget *win,gboolean show)
{
	showMultipleBonds = show;
	drawGeom();
}
/*****************************************************************************/
void resetConnections()
{
	reset_all_connections();
	drawGeom();
}
/*****************************************************************************/
void resetConnectionsBetweenSelectedAndNotSelectedAtoms()
{
	reset_connections_between_selected_and_notselected_atoms();
	drawGeom();
}
/*****************************************************************************/
void resetConnectionsBetweenSelectedAtoms()
{
	reset_connections_between_selected_atoms();
	drawGeom();
}
/*****************************************************************************/
void resetMultipleConnections()
{
	reset_multiple_bonds();
	drawGeom();
}
/*****************************************************************************/
GabEditGeomOperation getOperationType()
{
	return OperationType;
}
/*****************************************************************************/
gdouble get_real_distance2(GeomDef* g,gint i,gint j)
{
 	gdouble xx;
	gdouble d;

	xx = g[i].X-g[j].X;
	d = xx*xx;
	xx = g[i].Y-g[j].Y;
	d += xx*xx;
	xx = g[i].Z-g[j].Z;
	d += xx*xx;

	return d;
}
/*******************************************************************/
void adjust_multiple_bonds_with_one_atom(gint n)
{
	if(Natoms<2) return;
	if(!geometry[n].typeConnections) return;
	{
		gint ni = 0;
		gint nj = 0;
		gint i,j;
		gint nBonds = 0;

		i = n;
		ni = geometry[i].N-1;
		for(j=0;j<(gint)Natoms;j++)
		{
			if(j==i) continue;
			nj = geometry[j].N-1;
		 	if(geometry[i].typeConnections[nj]>0) 
		 	{
				nBonds += geometry[i].typeConnections[nj];
		 	}
		}
		if(nBonds<=geometry[i].Prop.maximumBondValence) return;
		for(j=0;j<(gint)Natoms;j++)
		{
			nj = geometry[j].N-1;
			if(i==j) continue;
			if(geometry[i].typeConnections[nj]<=1) continue;
			geometry[i].typeConnections[nj] -=1;
			geometry[j].typeConnections[ni] -=1;
			nBonds--;
			if(nBonds<=geometry[i].Prop.maximumBondValence) return;
		}
		copy_connections(geometry0, geometry, Natoms);
	}
}
/*******************************************************************/
void reset_connection_with_one_atom(gint n)
{
	if(Natoms<2) return;
	if(!geometry[n].typeConnections) return;
	{
		gint ni = 0;
		gint nj = 0;
		gint i,j;
		gint* nBonds = g_malloc(Natoms*sizeof(gint));

		for(i=0;i<(gint)Natoms;i++) nBonds[i] = 0;
		for(i=0;i<(gint)Natoms-1;i++)
		{
			if(i==n) continue;
			for(j=i+1;j<(gint)Natoms;j++)
			{
				if(j==n) continue;
				nj = geometry[j].N-1;
			 	if(geometry[i].typeConnections[nj]>0) 
			 	{
					nBonds[i] += geometry[i].typeConnections[nj];
				 	nBonds[j] += geometry[i].typeConnections[nj];
			 	}
			}
		}
		i = n;
		ni = geometry[i].N-1;
		for(j=0;j<(gint)Natoms;j++)
		{
			if(i==j) continue;
			nj = geometry[j].N-1;
			if(draw_lines_yes_no(i,j)) geometry[i].typeConnections[nj]= 1;
			else geometry[i].typeConnections[nj]=0;

			if(geometry[j].typeConnections)
				geometry[j].typeConnections[ni] = geometry[i].typeConnections[nj];

			nBonds[i]+= geometry[i].typeConnections[nj];
			nBonds[j]+= geometry[i].typeConnections[nj];
			if(nBonds[i]>geometry[i].Prop.maximumBondValence || 
			nBonds[j]>geometry[j].Prop.maximumBondValence 
					)
			{
				geometry[i].typeConnections[nj]= 0;
				if(geometry[j].typeConnections)
					geometry[j].typeConnections[ni] = 0;
				nBonds[i]--;
				nBonds[j]--;

			}
		}
		for(j=0;j<(gint)Natoms;j++)
		{
			nj = geometry[j].N-1;
			if(i==j) continue;
			if(geometry[i].typeConnections[nj]==0) continue;
			if(
		 	nBonds[i] < geometry[i].Prop.maximumBondValence &&
		 	nBonds[j] < geometry[j].Prop.maximumBondValence 
			)
			{
				geometry[i].typeConnections[nj] = geometry[j].typeConnections[ni] = 2;
				nBonds[i] += 1;
				nBonds[j] += 1;
			}
		}
		for(j=0;j<(gint)Natoms;j++)
		{
			nj = geometry[j].N-1;
			if(i==j) continue;
			if(geometry[i].typeConnections[nj]==0) continue;
			if(
		 	nBonds[i] < geometry[i].Prop.maximumBondValence &&
		 	nBonds[j] < geometry[j].Prop.maximumBondValence 
			)
			{
				geometry[i].typeConnections[nj] = geometry[j].typeConnections[ni] = 3;
				nBonds[i] += 1;
				nBonds[j] += 1;
			}
		}
		copy_connections(geometry0, geometry, Natoms);
	}
}
/*****************************************************************************/
static void init_connections()
{
	gint i;
	gint j;
	if(Natoms<1) return;
	if(geometry)
	for(i=0;i<(gint)Natoms;i++)
	{
		geometry[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<(gint)Natoms;j++) geometry[i].typeConnections[j] = 0;
	}
	if(geometry0)
	for(i=0;i<(gint)Natoms;i++)
	{
		geometry0[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<(gint)Natoms;j++) geometry0[i].typeConnections[j] = 0;
	}
}
/************************************************************************/
static void reSetSimpleConnections()
{
	gint* nBonds = NULL;
	gint* num = NULL;
	gint* numConn = NULL;
	gdouble* dists = NULL;
	gint i;
	gint j;
	gint ni;
	gint nj;
	gint k;
	gint kmax;
	gint nb0;
	if(Natoms<1) return;
	nBonds = g_malloc(Natoms*sizeof(gint));
	num = g_malloc(Natoms*sizeof(gint));
	numConn = g_malloc(Natoms*sizeof(gint));
	dists = g_malloc(Natoms*sizeof(gdouble));
	for(i=0;i<(gint)Natoms;i++) num[i] = i;

	for(i=0;i<(gint)Natoms-1;i++)
	{
		gint k = i;
		for(j=i+1;j<(gint)Natoms;j++)
			if(geometry[num[j]].N<geometry[num[k]].N) k = j;
		if(k!=i)
		{
			gint t = num[i];
			num[i] = num[k];
			num[k] = t;
		}
	}

	for(i=0;i<(gint)Natoms;i++) nBonds[i] = 0;
	for(i=0;i<(gint)Natoms;i++)
	{
		ni = geometry[num[i]].N-1;
		for(j=i+1;j<(gint)Natoms;j++)
		{
			nj = geometry[num[j]].N-1;
			 if(geometry[num[i]].typeConnections[nj]>0) 
			 {
				 nBonds[i] += 1;
				 nBonds[j] += 1;
			 }
		}
	}
	/* remove H1-H2 connections if H1 and H2 are not connected to others atoms */
	for(i=0;i<(gint)Natoms;i++)
	{
		ni = geometry[num[i]].N-1;
		if( nBonds[i] <= geometry[num[i]].Prop.maximumBondValence) continue;
		if( geometry[num[i]].Prop.maximumBondValence>1) continue; 
		for(k=0;k<(gint)nBonds[i];k++) numConn[k] = -1;
		k = 0;
		for(j=0;j<(gint)Natoms;j++)
		{
			nj = geometry[num[j]].N-1;
			if(i!=j && geometry[num[i]].typeConnections[nj]>0) 
			{
        			gdouble p = geometry[num[i]].Prop.covalentRadii+geometry[num[j]].Prop.covalentRadii;
				p = p*p;
				numConn[k] = j;
				dists[k] = get_real_distance2(geometry, num[i], num[j])/p;
				if( geometry[num[j]].Prop.maximumBondValence>1) numConn[k] =-1;
				k++;
			}
		}
		nb0 = nBonds[i];
		do{
			kmax = -1;
			for(k=0;k<nb0;k++) if(numConn[k]>-1) { kmax = k; break;}
			if(kmax<0) break;
			for(k=0;k<nb0;k++) if(numConn[k]>-1 && dists[k]>dists[kmax]) kmax = k;
		 	nBonds[i] -= 1;
		 	j = numConn[kmax];
		 	nBonds[j] -= 1;
		 	nj = geometry[num[j]].N-1;
			geometry[num[i]].typeConnections[nj] = 0;
			geometry[num[j]].typeConnections[ni] = 0;
		 	numConn[kmax]=-1;
		}while( nBonds[i] > geometry[num[i]].Prop.maximumBondValence);
	}
	/* remove A-H-B connections */
	for(i=0;i<(gint)Natoms;i++)
	{
		ni = geometry[num[i]].N-1;
		if( nBonds[i] <= geometry[num[i]].Prop.maximumBondValence) continue;
		if( geometry[num[i]].Prop.maximumBondValence>1) continue; 
		for(k=0;k<(gint)nBonds[i];k++) numConn[k] = -1;
		k = 0;
		for(j=0;j<(gint)Natoms;j++)
		{
			nj = geometry[num[j]].N-1;
			if(i!=j && geometry[num[i]].typeConnections[nj]>0) 
			{
        			gdouble p = geometry[num[i]].Prop.covalentRadii+geometry[num[j]].Prop.covalentRadii;
				p = p*p;
				numConn[k] = j;
				dists[k] = get_real_distance2(geometry, num[i], num[j])/p;
				k++;
			}
		}
		nb0 = nBonds[i];
		do{
			kmax = -1;
			for(k=0;k<nb0;k++) if(numConn[k]>-1) { kmax = k; break;}
			if(kmax<0) break;
			for(k=0;k<nb0;k++) if(numConn[k]>-1 && dists[k]>dists[kmax]) kmax = k;
		 	nBonds[i] -= 1;
		 	j = numConn[kmax];
		 	nBonds[j] -= 1;
		 	nj = geometry[num[j]].N-1;
			geometry[num[i]].typeConnections[nj] = 0;
			geometry[num[j]].typeConnections[ni] = 0;
		 	numConn[kmax]=-1;
		}while( nBonds[i] > geometry[num[i]].Prop.maximumBondValence);
	}
	/* remove H-B connections if B saturated */
	for(i=0;i<(gint)Natoms;i++)
	{
		gint nV = geometry[num[i]].Prop.maximumBondValence;
		if(geometry[num[i]].mmType && !strcmp(geometry[num[i]].mmType,"N3")) nV++;
		if( nBonds[i] <= nV) continue;
		if( nV<=1) continue; 
		ni = geometry[num[i]].N-1;
		for(k=0;k<(gint)nBonds[i];k++) numConn[k] = -1;
		k = 0;
		for(j=0;j<(gint)Natoms;j++)
		{
			nj = geometry[num[j]].N-1;
			if(i!=j && geometry[num[i]].typeConnections[nj]>0) 
			{
        			gdouble p = geometry[num[i]].Prop.covalentRadii+geometry[num[j]].Prop.covalentRadii;
				p = p*p;
				numConn[k] = j;
				dists[k] = get_real_distance2(geometry, num[i], num[j])/p;
				if( geometry[num[j]].Prop.maximumBondValence>1) numConn[k] =-1;
				k++;
			}
		}
		nb0 = nBonds[i];
		do{
			kmax = -1;
			for(k=0;k<nb0;k++) if(numConn[k]>-1) { kmax = k; break;}
			if(kmax<0) break;
			for(k=0;k<nb0;k++) if(numConn[k]>-1 && dists[k]>dists[kmax]) kmax = k;
		 	nBonds[i] -= 1;
		 	j = numConn[kmax];
		 	nBonds[j] -= 1;
		 	nj = geometry[num[j]].N-1;
			geometry[num[i]].typeConnections[nj] = 0;
			geometry[num[j]].typeConnections[ni] = 0;
		 	numConn[kmax]=-1;
		}while( nBonds[i] > geometry[num[i]].Prop.maximumBondValence);
	}
	/* remove A-B connections if A or B saturated */
	for(i=0;i<(gint)Natoms;i++)
	{
		ni = geometry[num[i]].N-1;
		if( geometry[num[i]].Prop.maximumBondValence<=1) continue;
		if( nBonds[i] <= geometry[num[i]].Prop.maximumBondValence) continue;
		if(geometry[num[i]].mmType && !strcmp(geometry[num[i]].mmType,"N3") && nBonds[i] <= geometry[num[i]].Prop.maximumBondValence+1) continue;
		for(k=0;k<(gint)nBonds[i];k++) numConn[k] = -1;
		k = 0;
		for(j=0;j<(gint)Natoms;j++)
		{
			nj = geometry[num[j]].N-1;
			if(i!=j && geometry[num[i]].typeConnections[nj]>0) 
			{
        			gdouble p = geometry[num[i]].Prop.covalentRadii+geometry[num[j]].Prop.covalentRadii;
				p = p*p;
				numConn[k] = j;
				dists[k] = get_real_distance2(geometry, num[i], num[j])/p;
				k++;
			}
		}
		nb0 = nBonds[i];
		do{
			kmax = -1;
			for(k=0;k<nb0;k++) if(numConn[k]>-1) { kmax = k; break;}
			if(kmax<0) break;
			for(k=0;k<nb0;k++) if(numConn[k]>-1 && dists[k]>dists[kmax]) kmax = k;
		 	nBonds[i] -= 1;
		 	j = numConn[kmax];
		 	nBonds[j] -= 1;
		 	nj = geometry[num[j]].N-1;
			geometry[num[i]].typeConnections[nj] = 0;
			geometry[num[j]].typeConnections[ni] = 0;
		 	numConn[kmax]=-1;
		}while( nBonds[i] > geometry[num[i]].Prop.maximumBondValence);
	}

	g_free(nBonds);
	g_free(num);
	g_free(numConn);
	g_free(dists);
}
/************************************************************************/
static void setMultipleBonds()
{
	gint* nBonds = NULL;
	gint* num = NULL;
	gint i;
	gint j;
	gint ni;
	gint nj;
	if(Natoms<1) return;
	nBonds = g_malloc(Natoms*sizeof(gint));
	num = g_malloc(Natoms*sizeof(gint));
	for(i=0;i<(gint)Natoms;i++) num[i] = i;

	for(i=0;i<(gint)Natoms-1;i++)
	{
		gint k = i;
		for(j=i+1;j<(gint)Natoms;j++)
			if(geometry[num[j]].N<geometry[num[k]].N) k = j;
		if(k!=i)
		{
			gint t = num[i];
			num[i] = num[k];
			num[k] = t;
		}
	}

	for(i=0;i<(gint)Natoms;i++) nBonds[i] = 0;
	for(i=0;i<(gint)Natoms;i++)
	{
		ni = geometry[num[i]].N-1;
		for(j=i+1;j<(gint)Natoms;j++)
		{
			nj = geometry[num[j]].N-1;
			 if(geometry[num[i]].typeConnections[nj]>0) 
			 {
				 nBonds[i] += 1;
				 nBonds[j] += 1;
			 }
		}
	}
// sort atoms nBonds min at first
	for(i=0;i<(gint)Natoms-1;i++)
	{
		gint k = i;
		for(j=i+1;j<(gint)Natoms;j++)
			if(nBonds[j]<nBonds[k])  k = j;
		if(k!=i)
		{
			gint t = num[i];
			num[i] = num[k];
			num[k] = t;
			t = nBonds[i];
			nBonds[i] = nBonds[k];
			nBonds[k] = t;
		}
	}
	for(i=0;i<(gint)Natoms;i++)
	{
		ni = geometry[num[i]].N-1;
		for(j=i+1;j<(gint)Natoms;j++)
		{
			nj = geometry[num[j]].N-1;
			if(geometry[num[i]].typeConnections[nj]==0) continue;
			if(
		 	nBonds[i] < geometry[num[i]].Prop.maximumBondValence &&
		 	nBonds[j] < geometry[num[j]].Prop.maximumBondValence 
			)
			{
				geometry[num[i]].typeConnections[nj] = geometry[num[j]].typeConnections[ni] = 2;
				nBonds[i] += 1;
				nBonds[j] += 1;
			}
		}
	}
	for(i=0;i<(gint)Natoms;i++)
	{
		ni = geometry[num[i]].N-1;
		for(j=i+1;j<(gint)Natoms;j++)
		{
			nj = geometry[num[j]].N-1;
			if(geometry[num[i]].typeConnections[nj]==0) continue;
			if(
		 	nBonds[i] < geometry[num[i]].Prop.maximumBondValence &&
		 	nBonds[j] < geometry[num[j]].Prop.maximumBondValence 
			)
			{
				geometry[num[i]].typeConnections[nj] = geometry[num[j]].typeConnections[ni] = 3;
				nBonds[i] += 1;
				nBonds[j] += 1;
			}
		}
	}
	g_free(nBonds);
	g_free(num);
}
/*****************************************************************************/
static void set_connections()
{
	gint i;
	gint j;
	gint ni;
	gint nj;

	init_connections();
	for(i=0;i<(gint)Natoms;i++)
	{
		ni = geometry[i].N-1;
		for(j=i+1;j<(gint)Natoms;j++)
		{
			nj = geometry[j].N-1;
			if(draw_lines_yes_no(i,j)) geometry[i].typeConnections[nj]= 1;
			else geometry[i].typeConnections[nj]=0;
			geometry[j].typeConnections[ni] = geometry[i].typeConnections[nj];
		}
	}
	reSetSimpleConnections();
	setMultipleBonds();
}
/*****************************************************************************/
static void set_Hconnections()
{
	gint i;
	gint j;
	gint k;
	gboolean Ok;
	gdouble distance2;
	gdouble dx;
	gdouble dy;
	gdouble dz;
	gdouble angle;
	gchar* strAngle;
	gdouble minDistanceH2;
	gdouble maxDistanceH2;
        Point A;
        Point B;
	gint ni, nj, nk;

	minDistanceH = getMinDistanceHBonds();
	minDistanceH2 = minDistanceH*minDistanceH*ANG_TO_BOHR*ANG_TO_BOHR;

	maxDistanceH = getMaxDistanceHBonds();
	maxDistanceH2 = maxDistanceH*maxDistanceH*ANG_TO_BOHR*ANG_TO_BOHR;

	minAngleH = getMinAngleHBonds();
	maxAngleH = getMaxAngleHBonds();

	for(i=0;i<(gint)Natoms;i++)
	{
		ni = geometry[i].N-1;
		Ok = FALSE;
		Ok = atomCanDoHydrogenBond(geometry[i].Prop.symbol);
		if(!Ok) continue;
		for(j=0;j<(gint)Natoms;j++)
		{
			nj = geometry[j].N-1;
			if(geometry[i].typeConnections[nj]>0) continue;
			if(i==j) continue;
			if(strcmp(geometry[j].Prop.symbol, "H")!=0)continue;

			dx = geometry[i].X-geometry[j].X;
			dy = geometry[i].Y-geometry[j].Y;
			dz = geometry[i].Z-geometry[j].Z;
			distance2 = (dx*dx+dy*dy+dz*dz);
			if(distance2<minDistanceH2 || distance2>maxDistanceH2) continue;


			Ok = FALSE;
			for(k=0;k<(gint)Natoms;k++)
			{
				nk = geometry[k].N-1;
				if(k==j) continue;
				if(k==i) continue;
				if(geometry[j].typeConnections[nk]<=0) continue;
				A.C[0]=geometry[i].X-geometry[j].X;
				A.C[1]=geometry[i].Y-geometry[j].Y;
				A.C[2]=geometry[i].Z-geometry[j].Z;
        
				B.C[0]=geometry[k].X-geometry[j].X;
				B.C[1]=geometry[k].Y-geometry[j].Y;
				B.C[2]=geometry[k].Z-geometry[j].Z;

        			strAngle = get_angle_vectors(A,B);
				angle = atof(strAngle);
				if(strAngle) g_free(strAngle);
				if(angle>=minAngleH &&angle<=maxAngleH)
				{
					Ok = TRUE;
					break;
				}
			}
			if(Ok)
			{
				geometry[i].typeConnections[nj]=-1;
				geometry[j].typeConnections[ni]=-1;
			}
		}
	}
}
/*****************************************************************************/
void copy_connections(GeomDef* geom0, GeomDef* geom, gint n)
{
	gint i;
	gint j;
	if(!geom) return;
	if(!geom0) return;
	for(i=0;i<n;i++)
	{
		if(!geom[i].typeConnections) continue;
		for(j=0;j<n;j++)
		{
			gint nj = geom[j].N-1;
			if(!geom0[j].typeConnections) continue;
			geom0[i].typeConnections[nj] = geom[i].typeConnections[nj];
		}	
	}
}
/************************************************************************************************************/
static gint get_number_of_model_connections()
{
	gint i;
	gint j;
	gint nc = 0;
	gint NC = Natoms;
	if(Natoms<1) return 0;	
    	for(i=0;i<NC;i++)
	{
		if(geometry[i].Layer==MEDIUM_LAYER || geometry[i].Layer==LOW_LAYER) continue;
    		for(j=0;j<NC;j++)
		{
			gint nj = geometry[j].N-1;
			if(i==j) continue;
			if(geometry[j].Layer==MEDIUM_LAYER || geometry[j].Layer==LOW_LAYER) continue;
			if(geometry[i].typeConnections[nj]>0) nc++;
		}
	}
	return nc;
}
/************************************************************************************************************/
static gint get_number_of_inter_connections()
{
	gint i;
	gint j;
	gint nc = 0;
	gint NC = Natoms;
    	for(i=0;i<NC;i++)
	{
		if(geometry[i].Layer==HIGH_LAYER || geometry[i].Layer==LOW_LAYER) continue;
    		for(j=0;j<NC;j++)
		{
			gint nj = geometry[j].N-1;
			if(i==j) continue;
			if(geometry[j].Layer==HIGH_LAYER || geometry[j].Layer==LOW_LAYER) continue;
			if(geometry[i].typeConnections[nj]>0) nc++;
		}
	}
	return nc;
}
/*************************************************************************************/
static guint get_number_of_electrons(guint type)
{
/* 
   type = 1 : Medium and High
   type = 2 : High
   type = other : All
*/
   guint i;
   guint Ne=0;
   SAtomsProp Atom;
   for(i=0;i<Natoms;i++)
   {
       Atom = geometry[i].Prop;
       switch (type)
       {
      	case 1 : if( geometry[i].Layer==HIGH_LAYER ||  geometry[i].Layer==MEDIUM_LAYER) Ne += Atom.atomicNumber;
		 break;
       	case 2 : if( geometry[i].Layer==HIGH_LAYER) Ne += Atom.atomicNumber;
		 break;
       	default : Ne += Atom.atomicNumber;
        }
   }
   return Ne;
} 
/************************************************************************************************************/
static void reset_spin_of_electrons()
{
        gint i;
        guint NumberElectrons[3];
        guint SpinElectrons[3];
	gint n = 1;
	gint nL = 0;
	gint nM = 0;
	gint nH = 0;

	if(Natoms<1) return;	

        NumberElectrons[2]= get_number_of_electrons(2);
        NumberElectrons[1]= get_number_of_electrons(1);
        NumberElectrons[0]= get_number_of_electrons(0);


        for(i=0;i<3;i++)
		SpinElectrons[i]=0;
    	for(i=0;i<Natoms;i++)
	{
		if(geometry[i].Layer==LOW_LAYER) nH =1;
		if(geometry[i].Layer==MEDIUM_LAYER) nM =1;
		if(geometry[i].Layer==HIGH_LAYER) nH =1;
	}
	n = nH + nM + nL;
        if(n==3)
	{
        	NumberElectrons[2] += get_number_of_model_connections();
        	NumberElectrons[1] += get_number_of_inter_connections();
	}
        if(n==2)
	{
        	NumberElectrons[1] += get_number_of_model_connections();
	}

        for(i=0;(guint)i<n;i++)
        	if((NumberElectrons[i]-TotalCharges[i])%2==0)
			SpinElectrons[i]=1;
                else
			SpinElectrons[i]=2;

        for(i=0;(guint)i<n;i++)
	{
		if(SpinMultiplicities[i]%2 != SpinElectrons[i]%2)
			SpinMultiplicities[i] = SpinElectrons[i];
	}
}
/*****************************************************************************/
void reset_charges_multiplicities()
{
	gint i;
	if(Natoms<1) return;	
	for(i=0;i<3;i++)
		TotalCharges[i] = 0;
	reset_spin_of_electrons();

}
/*****************************************************************************/
void reset_hydrogen_bonds()
{
	if(Natoms<1) return;	
	if(ShowHBonds) 
	{
		set_Hconnections();
		copy_connections(geometry0, geometry, Natoms);
	}
}
/*****************************************************************************/
void reset_multiple_bonds()
{
	gint i;
	gint j;
    	for(i=0;i<Natoms;i++)
	{
    		for(j=0;j<Natoms;j++)
		{
			gint nj = geometry[j].N-1;
			if(geometry[i].typeConnections[nj]>1)
				geometry[i].typeConnections[nj] = 1;
		}
	}
	setMultipleBonds();
}
/*****************************************************************************/
void reset_all_connections()
{
	if(Natoms<1) return;	
	buildRotation();
	set_connections();
	if(ShowHBonds) set_Hconnections();
	copy_connections(geometry0, geometry, Natoms);
}
/*****************************************************************************/
static void reset_connections_between_selected_atoms()
{
	gint i;
	gint j;
	gint ni;
	gint nj;
	gint* nBonds = NULL;
	gint* num = NULL;

	if(Natoms<2) return;
	nBonds = g_malloc(Natoms*sizeof(gint));
	num = g_malloc(Natoms*sizeof(gint));
	for(i=0;i<(gint)Natoms;i++) num[i] = i;
	for(i=0;i<(gint)Natoms-1;i++)
	{
		gint k = i;
		for(j=i+1;j<(gint)Natoms;j++)
			if(geometry[num[j]].N<geometry[num[k]].N) k = j;
		if(k!=i)
		{
			gint t = num[i];
			num[i] = num[k];
			num[k] = t;
		}
	}

	for(i=0;i<(gint)Natoms;i++) nBonds[i] = 0;
	for(i=0;i<(gint)Natoms-1;i++)
	{
		gboolean isa = if_selected(num[i]);
		ni = geometry[num[i]].N-1;
		if(geometry[num[i]].typeConnections)
		for(j=i+1;j<(gint)Natoms;j++)
		{
			gboolean jsa = if_selected(num[j]);
			if(isa==jsa) continue;
			nj = geometry[num[j]].N-1;
			 if(geometry[num[i]].typeConnections[nj]>0) 
			 {
				 nBonds[i] += geometry[num[i]].typeConnections[nj];
				 nBonds[j] += geometry[num[i]].typeConnections[nj];
			 }
		}
	}
	for(i=0;i<(gint)Natoms;i++)
	{
		gboolean isa = if_selected(num[i]);
		if(!isa) continue;
		ni = geometry[num[i]].N-1;
		if(geometry[num[i]].typeConnections)
		for(j=i+1;j<(gint)Natoms;j++)
		{
			gboolean jsa = if_selected(num[j]);
			if(!jsa) continue;
			nj = geometry[num[j]].N-1;
			if(draw_lines_yes_no(num[i],num[j])) geometry[num[i]].typeConnections[nj]= 1;
			else geometry[num[i]].typeConnections[nj]=0;

			if(geometry[num[j]].typeConnections)
				geometry[num[j]].typeConnections[ni] = geometry[num[i]].typeConnections[nj];

			nBonds[i]+= geometry[num[i]].typeConnections[nj];
			nBonds[j]+= geometry[num[i]].typeConnections[nj];
			if(nBonds[i]>geometry[num[i]].Prop.maximumBondValence || 
			nBonds[j]>geometry[num[j]].Prop.maximumBondValence 
					)
			{
				geometry[num[i]].typeConnections[nj]= 0;
				if(geometry[num[j]].typeConnections)
					geometry[num[j]].typeConnections[ni] = 0;
				nBonds[i]--;
				nBonds[j]--;

			}
		}
	}
	for(i=0;i<(gint)Natoms;i++)

	{
		gboolean isa = if_selected(num[i]);
		ni = geometry[num[i]].N-1;
		if(!isa) continue;
		for(j=i+1;j<(gint)Natoms;j++)
		{
			gboolean jsa = if_selected(num[j]);
			if(!jsa) continue;
			nj = geometry[num[j]].N-1;
			if(geometry[num[i]].typeConnections[nj]==0) continue;
			if(
		 	nBonds[i] < geometry[num[i]].Prop.maximumBondValence &&
		 	nBonds[j] < geometry[num[j]].Prop.maximumBondValence 
			)
			{
				geometry[num[i]].typeConnections[nj] = geometry[num[j]].typeConnections[ni] = 2;
				nBonds[i] += 1;
				nBonds[j] += 1;
			}
		}
	}
	for(i=0;i<(gint)Natoms;i++)
	{
		gboolean isa = if_selected(num[i]);
		if(!isa) continue;
		ni = geometry[num[i]].N-1;
		for(j=i+1;j<(gint)Natoms;j++)
		{
			gboolean jsa = if_selected(num[j]);
			if(!jsa) continue;
			nj = geometry[num[j]].N-1;
			if(geometry[num[i]].typeConnections[nj]==0) continue;
			if(
		 	nBonds[i] < geometry[num[i]].Prop.maximumBondValence &&
		 	nBonds[j] < geometry[num[j]].Prop.maximumBondValence 
			)
			{
				geometry[num[i]].typeConnections[nj] = geometry[num[j]].typeConnections[ni] = 3;
				nBonds[i] += 1;
				nBonds[j] += 1;
			}
		}
	}
	g_free(nBonds);
	g_free(num);
	copy_connections(geometry0, geometry, Natoms);
}
/*****************************************************************************/
static void reset_connections_between_selected_and_notselected_atoms()
{
	gint i;
	gint j;
	gint ni;
	gint nj;
	gint* nBonds = NULL;
	gint* num = NULL;

	if(Natoms<2) return;
	nBonds = g_malloc(Natoms*sizeof(gint));
	num = g_malloc(Natoms*sizeof(gint));
	for(i=0;i<(gint)Natoms;i++) num[i] = i;
	for(i=0;i<(gint)Natoms-1;i++)
	{
		gint k = i;
		for(j=i+1;j<(gint)Natoms;j++)
			if(geometry[num[j]].N<geometry[num[k]].N) k = j;
		if(k!=i)
		{
			gint t = num[i];
			num[i] = num[k];
			num[k] = t;
		}
	}

	for(i=0;i<(gint)Natoms;i++) nBonds[i] = 0;
	for(i=0;i<(gint)Natoms-1;i++)
	{
		gboolean isa = if_selected(num[i]);
		ni = geometry[num[i]].N-1;
		if(geometry[num[i]].typeConnections)
		for(j=i+1;j<(gint)Natoms;j++)
		{
			gboolean jsa = if_selected(num[j]);
			if(isa!=jsa) continue;
			nj = geometry[num[j]].N-1;
			 if(geometry[num[i]].typeConnections[nj]>0) 
			 {
				 nBonds[i] += geometry[num[i]].typeConnections[nj];
				 nBonds[j] += geometry[num[i]].typeConnections[nj];
			 }
		}
	}
	for(i=0;i<(gint)Natoms;i++)
	{
		gboolean isa = if_selected(num[i]);
		ni = geometry[num[i]].N-1;
		if(geometry[num[i]].typeConnections)
		for(j=i+1;j<(gint)Natoms;j++)
		{
			gboolean jsa = if_selected(num[j]);
			if(isa==jsa) continue;
			nj = geometry[num[j]].N-1;
			if(draw_lines_yes_no(num[i],num[j])) geometry[num[i]].typeConnections[nj]= 1;
			else geometry[num[i]].typeConnections[nj]=0;

			if(geometry[num[j]].typeConnections)
				geometry[num[j]].typeConnections[ni] = geometry[num[i]].typeConnections[nj];

			nBonds[i]+= geometry[num[i]].typeConnections[nj];
			nBonds[j]+= geometry[num[i]].typeConnections[nj];
			if(nBonds[i]>geometry[num[i]].Prop.maximumBondValence || 
			nBonds[j]>geometry[num[j]].Prop.maximumBondValence 
					)
			{
				geometry[num[i]].typeConnections[nj]= 0;
				if(geometry[num[j]].typeConnections)
					geometry[num[j]].typeConnections[ni] = 0;
				nBonds[i]--;
				nBonds[j]--;

			}
		}
	}
	for(i=0;i<(gint)Natoms;i++)

	{
		gboolean isa = if_selected(num[i]);
		ni = geometry[num[i]].N-1;
		for(j=i+1;j<(gint)Natoms;j++)
		{
			gboolean jsa = if_selected(num[j]);
			nj = geometry[num[j]].N-1;
			if(geometry[num[i]].typeConnections[nj]==0) continue;
			if(
		 	nBonds[i] < geometry[num[i]].Prop.maximumBondValence &&
		 	nBonds[j] < geometry[num[j]].Prop.maximumBondValence 
			)
			{
				if(isa != jsa)
				{
				geometry[num[i]].typeConnections[nj] = geometry[num[j]].typeConnections[ni] = 2;
				nBonds[i] += 1;
				nBonds[j] += 1;
				}
			}
		}
	}
	for(i=0;i<(gint)Natoms;i++)
	{
		gboolean isa = if_selected(num[i]);
		ni = geometry[num[i]].N-1;
		for(j=i+1;j<(gint)Natoms;j++)
		{
			gboolean jsa = if_selected(num[j]);
			nj = geometry[num[j]].N-1;
			if(geometry[num[i]].typeConnections[nj]==0) continue;
			if(
		 	nBonds[i] < geometry[num[i]].Prop.maximumBondValence &&
		 	nBonds[j] < geometry[num[j]].Prop.maximumBondValence 
			)
			{
				if(isa != jsa)
				{
				geometry[num[i]].typeConnections[nj] = geometry[num[j]].typeConnections[ni] = 3;
				nBonds[i] += 1;
				nBonds[j] += 1;
				}
			}
		}
	}
	g_free(nBonds);
	g_free(num);
	copy_connections(geometry0, geometry, Natoms);
}
/*****************************************************************************/
gboolean hbond_connections(gint i, gint j)
{

	if(ShowHBonds)
	{
		gint nj = geometry[j].N-1;
		if(i<(gint)Natoms && j<(gint)Natoms && geometry[i].typeConnections[nj]==-1) return TRUE;
		else return FALSE;
	}
	else return FALSE;
}
/*****************************************************************************/
void init_quat(gdouble quat[])
{
	gint i;
	for(i=0;i<3;i++) quat[i] = 0.0;
	quat[3] = 1.0;
}
/********************************************************************************/
static void set_origin_to_point(gdouble center[])
{
	gint n;
	for(n = 0;n<(gint)Natoms;n++)
	{
			geometry0[n].X -= center[0];
			geometry0[n].Y -= center[1];
			geometry0[n].Z -= center[2];

			geometry[n].X -= center[0];
			geometry[n].Y -= center[1];
			geometry[n].Z -= center[2];
	}
	for (n=0;n<3;n++) Orig[n] += center[n];
}
/********************************************************************************/
void set_origin_to_center_of_fragment()
{

	gdouble C[3] = {0,0,0};
	gint j = 0;
	gint i;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(if_selected(i))
		{
			j++;
			C[0] += geometry0[i].X;
			C[1] += geometry0[i].Y;
			C[2] += geometry0[i].Z;
		}
	}
	if(j==0) return;
	for (i=0;i<3;i++) C[i] /= j;
	set_origin_to_point(C);
	create_GeomXYZ_from_draw_grometry();
	TransX = 0;
	TransY = 0;
	drawGeom();
}
/********************************************************************************/
static void set_geom_to_axes(gdouble axis1[], gdouble axis2[], gdouble axis3[])
{
	gdouble **m0 = NULL;
	gdouble** minv;
	gint i,j;
	guint n;

	gdouble A[3];
	gdouble B[3];
	guint k;
	gdouble* X;
	gdouble* Y;
	gdouble* Z;

	if(Natoms<1) return;

	m0 = g_malloc(3*sizeof(gdouble*));
	X = g_malloc(Natoms*sizeof(gdouble));
	Y = g_malloc(Natoms*sizeof(gdouble));
	Z = g_malloc(Natoms*sizeof(gdouble));

	for(i=0;i<3;i++)
		m0[i] = g_malloc(3*sizeof(gdouble));


	m0[0][0] = axis1[0];
	m0[0][1] = axis1[1];
	m0[0][2] = axis1[2];

	m0[1][0] = axis2[0];
	m0[1][1] = axis2[1];
	m0[1][2] = axis2[2];

	m0[2][0] = axis3[0];
	m0[2][1] = axis3[1];
	m0[2][2] = axis3[2];

	minv = Inverse(m0,3,1e-7);

	for(n = 0;n<Natoms;n++)
	{
		A[0] = geometry[n].X;
		A[1] = geometry[n].Y;
		A[2] = geometry[n].Z;

		for(j=0;j<3;j++)
		{
			B[j] = 0.0;
			for(k=0;k<3;k++)
				B[j] += minv[k][j]*A[k];
		}

		X[n] = B[0];
		Y[n] = B[1];
		Z[n] = B[2];
	}
	for(n = 0;n<Natoms;n++)
	{
			geometry0[n].X = X[n];
			geometry0[n].Y = Y[n];
			geometry0[n].Z = Z[n];

			geometry[n].X = X[n];
			geometry[n].Y = Y[n];
			geometry[n].Z = Z[n];
	}
	
	for(i=0;i<3;i++) if(minv[i]) g_free(minv[i]);
	if(minv) g_free(minv);

	for(i=0;i<3;i++) if(m0[i]) g_free(m0[i]);
	if(m0) g_free(m0);

	if(X) g_free(X);
	if(Y) g_free(Y);
	if(Z) g_free(Z);
}
/********************************************************************************/
void move_the_center_of_selected_or_not_selected_atoms_to_origin(gboolean sel)
{

	gdouble C[3] = {0,0,0};
	gdouble C0[3] = {0,0,0};
	gdouble mt = 0;
	gint i;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(if_selected(i)==sel)
		{
			gdouble m = geometry[i].Prop.masse;
			mt += m;
			C[0] += m*geometry[i].X;
			C[1] += m*geometry[i].Y;
			C[2] += m*geometry[i].Z;
			C0[0] += m*geometry0[i].X;
			C0[1] += m*geometry0[i].Y;
			C0[2] += m*geometry0[i].Z;
		}
	}
	if(mt==0) return;
	for (i=0;i<3;i++) C[i] /= mt;
	for (i=0;i<3;i++) C0[i] /= mt;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(if_selected(i)==sel)
		{
			geometry[i].X -= C[0];
			geometry[i].Y -= C[1];
			geometry[i].Z -= C[2];

			geometry0[i].X -= C0[0];
			geometry0[i].Y -= C0[1];
			geometry0[i].Z -= C0[2];
		}
	}
}
/*****************************************************************************/
static int set_fragment_rotational_matrix(gdouble m[6], gdouble C[], gboolean sel)
{
	gint i;
	gint ip;
	gint j;
	gint k;
	gdouble a;
	gdouble* XYZ[3];
	gdouble mt = 0;
	gint nFrag = 0;
	gint* numAtoms;

	for(i=0;i<3;i++)
	{
		XYZ[i] = g_malloc(Natoms*sizeof(gdouble));
		C[i] = 0.0;
	}
	numAtoms = g_malloc(Natoms*sizeof(gint));


	for(j=0;j<(gint)Natoms;j++)
	{
		if(if_selected(j)==sel)
		{
			numAtoms[nFrag] = j;
			mt += geometry[j].Prop.masse;
			XYZ[0][nFrag] = geometry[j].X;
			XYZ[1][nFrag] = geometry[j].Y;
			XYZ[2][nFrag] = geometry[j].Z;
			nFrag++;
		}
	}
	for(i=0;i<3;i++)
		for(j=0;j<nFrag;j++)
			C[i] += geometry[numAtoms[j]].Prop.masse*XYZ[i][j];
	if(mt != 0)
	for(i=0;i<3;i++) C[i] /= mt;

	for(j=0;j<nFrag;j++)
		for(i=0;i<3;i++)
			XYZ[i][j] -= C[i];
	
	k = 0;
	for(i=0;i<3;i++)
		for(ip=i;ip<3;ip++)
	{
		m[k] = 0.0;

		for(j=0;j<nFrag;j++)
		{
			if(i==ip)
			a = XYZ[(i+1)%3][j]*XYZ[(ip+1)%3][j]
			  + XYZ[(i+2)%3][j]*XYZ[(ip+2)%3][j];
			else
			{
				a =-XYZ[i][j]*XYZ[ip][j];
			}
			m[k] += geometry[numAtoms[j]].Prop.masse*a;
		}
		k++;
	}

	for(i=0;i<3;i++) g_free(XYZ[i]);
	g_free(numAtoms);
	return nFrag;
}
/********************************************************************************/
static int compute_fragment_principal_axes(gdouble axis1[], gdouble axis2[], gdouble axis3[], gdouble C[], gboolean sel)
{
	gdouble m[6];
	gdouble I[3];
	gdouble** v = NULL;
	gint nrot;
	gint i;
	gint nFrag;


	if(Natoms<1) return 0;

	nFrag = set_fragment_rotational_matrix(m, C,sel);
	if(nFrag == 0) return 0;

	v = g_malloc(3*sizeof(gdouble*));
	for(i=0;i<3;i++) v[i] = g_malloc(3*sizeof(gdouble));

	jacobi(m, 3,I,v,&nrot);

	for(i=0;i<3;i++) axis1[i] =  v[i][0];
	for(i=0;i<3;i++) axis2[i] =  v[i][1];
	for(i=0;i<3;i++) axis3[i] =  v[(i+1)%3][0]*v[(i+2)%3][1]-v[(i+2)%3][0]*v[(i+1)%3][1];
	/*
	for(i=0;i<3;i++) axis3[i] =  v[i][2];
	*/

	for(i=0;i<3;i++) g_free(v[i]);
	g_free(v);
	printf("I= %f %f %f\n",I[0],I[1],I[2]);
	return nFrag;
}
/********************************************************************************/
static void rotate_slected_atoms_minimize_rmsd()
{
	gint i,j;

	gint nA = 0;
	const gint N = 8;
        gdouble d[8];
        gdouble w[8][3] = {
                {1,1,1},
                {-1,1,1},
                {1,-1,1},
                {1,1,-1},
                {-1,-1,1},
                {-1,1,-1},
                {1,-1,-1},
                {-1,-1,-1}
        };
        gdouble x, y, z;
	gint* numA = NULL;
	gint* numB = NULL;
	gint iA, iB;

	if(Natoms<1) return;

	j = 0;
	for(i = 0;i<Natoms;i++)
		if(if_selected(i)) nA++;

	if(nA != Natoms - nA || nA == 0) return;
	numA = g_malloc(nA*sizeof(gint));
	numB = g_malloc(nA*sizeof(gint));
        for (i=0;i<nA;i++) numA[i] = -1;
        for (i=0;i<nA;i++) numB[i] = -1;

	iA = iB = 0;
	for(i = 0;i<Natoms;i++)
		if(if_selected(i)) numA[iA++] = i;
		else numB[iB++] = i;

	for(i = 0;i<nA;i++)
	if(strcmp(geometry[numA[i]].Prop.symbol, geometry[numB[i]].Prop.symbol))
	{
		g_free(numA);
		g_free(numB);
		return;
	}

        for (j=0;j<N;j++) d[j] = 0;
        for (i=0;i<nA;i++)
        {
                for (j=0;j<N;j++)
                {
                x = geometry[numA[i]].X - w[j][0]*geometry[numB[i]].X;
                y = geometry[numA[i]].Y - w[j][1]*geometry[numB[i]].Y;
                z = geometry[numA[i]].Z - w[j][2]*geometry[numB[i]].Z;
                d[j] += x*x + y*y + z*z;
                }
        }
        double dmin = d[0];
	int jmin = 0;
        for (j=1;j<N;j++) if(dmin>d[j]) {dmin=d[j]; jmin = j;}
        for (i=0;i<nA;i++)
	{
                geometry[numB[i]].X *=  w[jmin][0];
                geometry[numB[i]].Y *=  w[jmin][1];
                geometry[numB[i]].Z *=  w[jmin][2];

                geometry0[numB[i]].X *=  w[jmin][0];
                geometry0[numB[i]].Y *=  w[jmin][1];
                geometry0[numB[i]].Z *=  w[jmin][2];
	}


	RebuildGeom = TRUE;
}
/********************************************************************************/
/* type = 0 all atoms, type = 1 selected atoms, type = 2 not selected atoms */
static void set_xyz_to_standard_orientation(gint type)
{
	gint i,j;

	gdouble* X;
	gdouble* Y;
	gdouble* Z;
	gchar** symbols;

	if(Natoms<1) return;

	X = g_malloc(Natoms*sizeof(gdouble));
	Y = g_malloc(Natoms*sizeof(gdouble));
	Z = g_malloc(Natoms*sizeof(gdouble));
	symbols = g_malloc(Natoms*sizeof(gchar*));
	for(i = 0;i<Natoms;i++) symbols[i] = NULL;

	j = 0;
	for(i = 0;i<Natoms;i++)
	{
		if(type == 0 || (type==1 && if_selected(i))  || (type==2 && !if_selected(i)))
		{
			X[j] = geometry[i].X;
			Y[j] = geometry[i].Y;
			Z[j] = geometry[i].Z;
			symbols[j] = g_strdup(geometry[i].Prop.symbol);
			j++;
		}
	}

	buildStandardOrientationDlg(j, symbols, X, Y, Z);

	j = 0;
	for(i = 0;i<Natoms;i++)
	{
		if(type == 0 || (type==1 && if_selected(i))  || (type==2 && !if_selected(i)))
		{
			geometry0[i].X = X[j];
			geometry0[i].Y = Y[j];
			geometry0[i].Z = Z[j];

			geometry[i].X = X[j];
			geometry[i].Y = Y[j];
			geometry[i].Z = Z[j];
			j++;
		}
	}

	if(symbols) for(i = 0;i<Natoms;i++) if(symbols[i]) g_free(symbols[i]);
	if(symbols) g_free(symbols);
	
	if(X) g_free(X);
	if(Y) g_free(Y);
	if(Z) g_free(Z);

	RebuildGeom = TRUE;
}
/********************************************************************************/
void set_xyz_to_standard_orientation_all()
{
	set_xyz_to_standard_orientation(0);
	create_GeomXYZ_from_draw_grometry();
	init_quat(Quat);
	RebuildGeom = TRUE;
	drawGeom();
}
/********************************************************************************/
void set_xyz_to_standard_orientation_selected_atoms()
{
	set_xyz_to_standard_orientation(1);
	create_GeomXYZ_from_draw_grometry();
	init_quat(Quat);
	RebuildGeom = TRUE;
	drawGeom();
}
/********************************************************************************/
void set_xyz_to_standard_orientation_not_selected_atoms()
{
	set_xyz_to_standard_orientation(2);
	create_GeomXYZ_from_draw_grometry();
	init_quat(Quat);
	RebuildGeom = TRUE;
	drawGeom();
}
/********************************************************************************/
void set_xyz_to_standard_orientation_selected_and_not_selected_atoms()
{
	set_xyz_to_standard_orientation(1);
	set_xyz_to_standard_orientation(2);
	rotate_slected_atoms_minimize_rmsd();
	create_GeomXYZ_from_draw_grometry();
	init_quat(Quat);
	RebuildGeom = TRUE;
	drawGeom();
}
/********************************************************************************/
void set_xyz_to_principal_axes_of_selected_atoms(gpointer data, guint Operation,GtkWidget* wid)
{
	gdouble axis1[3] = {1,0,0};
	gdouble axis2[3] = {0,1,0};
	gdouble axis3[3] = {0,0,1};
	gdouble C[3] = {0,0,0};
	int nFrag = compute_fragment_principal_axes(axis1,axis2,axis3,C,TRUE);
	if(nFrag <2 ) return;
	set_origin_to_point(C);
	if(Operation == 0) set_geom_to_axes(axis1, axis2, axis3);
	else set_geom_to_axes(axis3, axis2, axis1);
	create_GeomXYZ_from_draw_grometry();
	init_quat(Quat);
	drawGeom();
}
/********************************************************************************/
void create_tolerance_window(GtkWidget*w, gpointer data)
{
	createToleranceWindow(GeomDlg, NULL);
}
/********************************************************************************/
void get_abelian_orientation_with_reduction(GtkWidget*w, gpointer data)
{
	gchar** symbols = NULL;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble* Z = NULL;
	gint i;
	gint numberOfAtoms = Natoms;
	gchar pointGroupSymbol[BSIZE];
	gchar abelianPointGroupSymbol[BSIZE];

	if(Natoms<1)
	{
		 Message(_("Sorry, the number of atoms is not positive"),_("Error"),TRUE);
		return;
	}
	symbols = (gchar**)g_malloc(sizeof(gchar*)*(Natoms));
	if(symbols == NULL) return;

	X = (gdouble*)g_malloc(sizeof(gdouble)*(Natoms));
	if(X == NULL) return;
	Y = (gdouble*)g_malloc(sizeof(gdouble)*(Natoms));
	if(Y == NULL) return;
	Z = (gdouble*)g_malloc(sizeof(gdouble)*(Natoms));
	if(Z == NULL) return;

	for (i=0;i<(gint)Natoms;i++)
	{
		symbols[i] = g_strdup(geometry0[i].Prop.symbol);
		X[i] = geometry0[i].X*BOHR_TO_ANG;
		Y[i] = geometry0[i].Y*BOHR_TO_ANG;
		Z[i] = geometry0[i].Z*BOHR_TO_ANG;
	}
	createGeometryAbelianGroupWindow(numberOfAtoms, symbols, X, Y, Z, pointGroupSymbol, abelianPointGroupSymbol);

	for (i=0;i<(gint)Natoms;i++)
		g_free( symbols[i]);
	g_free( symbols);
	g_free(X);
	g_free(Y);
	g_free(Z);
	return;
}
/********************************************************************************/
void get_standard_orientation_with_reduction(GtkWidget*w, gpointer data)
{
	gchar** symbols = NULL;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble* Z = NULL;
	gint i;
	gint numberOfAtoms = Natoms;
	gchar groupeSymbol[BSIZE];

	if(Natoms<1)
	{
		 Message(_("Sorry, the number of atoms is not positive"),_("Error"),TRUE);
		return;
	}
	symbols = (gchar**)g_malloc(sizeof(gchar*)*(Natoms));
	if(symbols == NULL) return;

	X = (gdouble*)g_malloc(sizeof(gdouble)*(Natoms));
	if(X == NULL) return;
	Y = (gdouble*)g_malloc(sizeof(gdouble)*(Natoms));
	if(Y == NULL) return;
	Z = (gdouble*)g_malloc(sizeof(gdouble)*(Natoms));
	if(Z == NULL) return;

	for (i=0;i<(gint)Natoms;i++)
	{
		symbols[i] = g_strdup(geometry0[i].Prop.symbol);
		X[i] = geometry0[i].X*BOHR_TO_ANG;
		Y[i] = geometry0[i].Y*BOHR_TO_ANG;
		Z[i] = geometry0[i].Z*BOHR_TO_ANG;
	}
	createGeometrySymmetryWindow(numberOfAtoms,symbols,X, Y, Z, groupeSymbol);

	for (i=0;i<(gint)Natoms;i++)
		g_free( symbols[i]);
	g_free( symbols);
	g_free(X);
	g_free(Y);
	g_free(Z);
	return;
}
/********************************************************************************/
void get_standard_orientation_with_symmetrization(GtkWidget*w, gpointer data)
{
	gchar** symbols = NULL;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble* Z = NULL;
	gint i;
	gint numberOfAtoms = Natoms;
	gchar groupeSymbol[BSIZE];

	if(Natoms<1)
	{
		 Message(_("Sorry, the number of atoms is not positive"),_("Error"),TRUE);
		return;
	}
	symbols = (gchar**)g_malloc(sizeof(gchar*)*(Natoms));
	if(symbols == NULL) return;

	X = (gdouble*)g_malloc(sizeof(gdouble)*(Natoms));
	if(X == NULL) return;
	Y = (gdouble*)g_malloc(sizeof(gdouble)*(Natoms));
	if(Y == NULL) return;
	Z = (gdouble*)g_malloc(sizeof(gdouble)*(Natoms));
	if(Z == NULL) return;

	for (i=0;i<(gint)Natoms;i++)
	{
		symbols[i] = g_strdup(geometry0[i].Prop.symbol);
		X[i] = geometry0[i].X*BOHR_TO_ANG;
		Y[i] = geometry0[i].Y*BOHR_TO_ANG;
		Z[i] = geometry0[i].Z*BOHR_TO_ANG;
	}
	createGeometrySymmetrizationWindow(numberOfAtoms,symbols,X, Y, Z, groupeSymbol);

// reset geometry0
	for(i = 0;i<numberOfAtoms;i++)
	{
		g_free(geometry0[i].Prop.symbol);
		geometry0[i].Prop = prop_atom_get(symbols[i]);
		geometry0[i].mmType = g_strdup(symbols[i]);
		geometry0[i].pdbType = g_strdup(symbols[i]);
		geometry0[i].Residue = g_strdup(symbols[i]);
		geometry0[i].Charge = 0.0;
		geometry0[i].X = X[i]/BOHR_TO_ANG;
		geometry0[i].Y = Y[i]/BOHR_TO_ANG;
		geometry0[i].Z = Z[i]/BOHR_TO_ANG;

		g_free(geometry[i].Prop.symbol);
		geometry[i].Prop = prop_atom_get(symbols[i]);
		geometry[i].mmType = g_strdup(symbols[i]);
		geometry[i].pdbType = g_strdup(symbols[i]);
		geometry[i].Residue = g_strdup(symbols[i]);
		geometry[i].Charge = 0.0;
		geometry[i].X = geometry0[i].X;
		geometry[i].Y = geometry0[i].Y;
		geometry[i].Z = geometry0[i].Z;
	}
	resetConnections();
	create_GeomXYZ_from_draw_grometry();

	for (i=0;i<(gint)Natoms;i++)
		g_free( symbols[i]);
	g_free( symbols);
	g_free(X);
	g_free(Y);
	g_free(Z);
	return;
}
/********************************************************************************/
static gint set_key_press(GtkWidget* wid, GdkEventKey *event, gpointer data)
{
	if((event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R) )
		ShiftKeyPressed = TRUE;
	else if((event->keyval == GDK_Control_L || event->keyval == GDK_Control_R) )
	{
		ControlKeyPressed = TRUE;
	}
	else if((event->keyval == GDK_Alt_L || event->keyval == GDK_Alt_R) )
	{
		ControlKeyPressed = TRUE;
	}
	else if((event->keyval == GDK_F || event->keyval == GDK_f) )
	{
		FKeyPressed = TRUE;
	}
	else if((event->keyval == GDK_G || event->keyval == GDK_g) )
	{
		GKeyPressed = TRUE;
	}
	else if((event->keyval == GDK_A || event->keyval == GDK_a)  && ControlKeyPressed)
	{
		SelectAllAtoms();
	}
	else if((event->keyval == GDK_u || event->keyval == GDK_U))
	{
        	switch(OperationType)
        	{
			case MOVEFRAG : 
			case DELETEOBJECTS : 
			case MEASURE     : 
			case EDITOBJECTS : 
			case ADDFRAGMENT : 
			case ROTLOCFRAG : 
			case ROTZLOCFRAG : 
				get_geometry_from_fifo( event->keyval == GDK_U);
				drawGeom();
				break;
			default:break;
		}
	}
	else if((event->keyval == GDK_z || event->keyval == GDK_y) && ControlKeyPressed)
	{
        	switch(OperationType)
        	{
			case MOVEFRAG : 
			case DELETEOBJECTS : 
			case MEASURE     : 
			case EDITOBJECTS : 
			case ADDFRAGMENT : 
			case ROTLOCFRAG : 
			case ROTZLOCFRAG : 
				get_geometry_from_fifo(event->keyval == GDK_y);
				drawGeom();
				break;
			default:break;
		}
	}
	GTK_WIDGET_GET_CLASS(wid)->key_press_event(wid, event);
	return TRUE;

}
/********************************************************************************/
static gint set_key_release(GtkWidget* wid, GdkEventKey *event, gpointer data)
{
	if((event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R) )
		ShiftKeyPressed = FALSE;
	else if((event->keyval == GDK_Control_L || event->keyval == GDK_Control_R) )
		ControlKeyPressed = FALSE;
	else if((event->keyval == GDK_Alt_L || event->keyval == GDK_Alt_R) )
		ControlKeyPressed = FALSE;
	else if((event->keyval == GDK_F || event->keyval == GDK_f) )
		FKeyPressed = FALSE;
	else if((event->keyval == GDK_G || event->keyval == GDK_g) )
		GKeyPressed = FALSE;
	return TRUE;
}
/********************************************************************************/
void setMMTypesChargesFromPDBTpl(guint Operation)
{
	gint i;
	gchar* mmType = NULL;
	gdouble charge = 0.0;
	for(i=0;i<(gint)Natoms;i++)
	{
		if(Operation!=3)
		{
			mmType = getMMTypeFromPDBTpl(geometry[i].Residue,geometry[i].pdbType,&charge);
			if(!strcmp(mmType,"UNK"))
			{
				g_free(mmType);
				continue;
			}
		}
		switch(Operation)
		{
			case 0: geometry[i].Charge = charge;
				geometry0[i].Charge = charge;
				break;
			case 1: g_free(geometry[i].mmType);
				g_free(geometry0[i].mmType);
				geometry[i].mmType = g_strdup(mmType);
				geometry0[i].mmType = g_strdup(mmType);
				break;
			case 2: g_free(geometry[i].mmType);
				g_free(geometry0[i].mmType);
				geometry[i].mmType = g_strdup(mmType);
				geometry0[i].mmType = g_strdup(mmType);
				geometry[i].Charge = charge;
				geometry0[i].Charge = charge;
				break;
			case 3:geometry[i].Charge = charge;
			       geometry0[i].Charge = charge;
			       break;
			default: break;
		}
		if(Operation!=3) g_free(mmType);
	}
}
/********************************************************************************/
void setMMTypesCharges(gpointer data, guint Operation,GtkWidget* wid)
{
	if(Operation==4)
	{
		gint i;
		calculTypesAmber(geometry, (gint)Natoms);
		for(i=0;i<(gint)Natoms;i++)
		{
			geometry0[i].mmType = g_strdup(geometry[i].mmType);
			geometry0[i].pdbType = g_strdup(geometry[i].pdbType);
		}
	}
	else setMMTypesChargesFromPDBTpl(Operation);
	create_GeomXYZ_from_draw_grometry();
	drawGeom();
}
/*****************************************************************************/
GeomDef* Free_One_Geom(GeomDef* geom,gint N) 
{
	gint i;
	if(!geom)  return NULL;
	for (i=0;i<N;i++)
	{
		g_free(geom[i].Prop.name);
		g_free(geom[i].Prop.symbol);
		g_free(geom[i].mmType);
		g_free(geom[i].pdbType);
		g_free(geom[i].Residue);
		if(geom[i].typeConnections) g_free(geom[i].typeConnections);
	}
	g_free(geom);
	return NULL;
}
/*****************************************************************************/
void free_text_to_draw()
{
	if(strToDraw)
		g_free(strToDraw);
	strToDraw = NULL;
}
/*****************************************************************************/
void set_text_to_draw(gchar* str)
{
	free_text_to_draw();

	strToDraw = g_strdup(str);
}
/********************************************************************************/
void set_statubar_operation_str(gchar* str)
{
	if(str && GeomDrawingArea)
	{
		gtk_statusbar_pop(GTK_STATUSBAR(StatusOperation),idStatusOperation);
		gtk_statusbar_push(GTK_STATUSBAR(StatusOperation),idStatusOperation,str);
	}
}
/*****************************************************************************/
void draw_text(gchar* str)
{
	GdkColormap *colormap;
	GdkColor color;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);

	gint x0  = GeomDrawingArea->allocation.width/20;
	gint y0  = GeomDrawingArea->allocation.height-GeomDrawingArea->allocation.height/10;

	color.red = FontsStyleLabel.TextColor.red;
	color.green = FontsStyleLabel.TextColor.green;
	color.blue = FontsStyleLabel.TextColor.blue;

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);
	gdk_gc_set_foreground(gc,&color);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, x0, y0, str, FALSE,TRUE);
	if(crExport)
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, x0, y0, str, FALSE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);

}
/*****************************************************************************/
gboolean select_atoms_by_groupe()
{
	gint i;
	gdouble x1=0;
	gdouble y1=0;
	gdouble xi;
	gdouble yi;
	gdouble d = 0;
	gint j;
	gint k;
	gboolean OK = FALSE;

	x1 = BeginX;
	y1 = BeginY;

	for(i=0;i<(gint)Natoms;i++)
	{
		xi = geometry[i].Xi;
		yi = geometry[i].Yi;
		d = (xi-x1)*(xi-x1) + (yi-y1)*(yi-y1);
		d = sqrt(d);
		if(d<=geometry[i].Rayon)
		{
			if(NumFatoms == NULL) NumFatoms = g_malloc((NFatoms+1)*sizeof(gint));
			else NumFatoms = g_realloc(NumFatoms, (NFatoms+1)*sizeof(gint));
			NumFatoms[NFatoms] = geometry[i].N;
			NFatoms+=1;

			for(j=0;j<(gint)Natoms;j++)
			{
				if(get_connection_type(i,j)>0)
				{

					gint nGroupAtoms=0;
					gint * listGroupAtoms = getListGroupe(&nGroupAtoms, geometry0, Natoms, i, j,-1,-1);
					if(NumFatoms == NULL) NumFatoms = g_malloc((NFatoms+nGroupAtoms+1)*sizeof(gint));
					else NumFatoms = g_realloc(NumFatoms, (NFatoms+nGroupAtoms+1)*sizeof(gint));

					NumFatoms[NFatoms] = geometry[j].N;
					for(k=NFatoms+1;k<NFatoms+nGroupAtoms+1;k++)
						NumFatoms[k] = geometry[listGroupAtoms[k-NFatoms-1]].N;
					NFatoms+=nGroupAtoms+1;
					if(listGroupAtoms) g_free(listGroupAtoms);
					OK = TRUE;
				}
			}
			break;

		}
	}
	drawGeom();
	return OK;
}
/*****************************************************************************/
gboolean select_atoms_by_residues()
{
	gint i;
	gdouble x1=0;
	gdouble y1=0;
	gdouble xi;
	gdouble yi;
	gdouble d = 0;
	gint j;
	gint k;
	gboolean del = FALSE;
	gint selectedj;
	gboolean OK = FALSE;


	x1 = BeginX;
	y1 = BeginY;

	for(i=0;i<(gint)Natoms;i++)
	{
		xi = geometry[i].Xi;
		yi = geometry[i].Yi;
		d = (xi-x1)*(xi-x1) + (yi-y1)*(yi-y1);
		d = sqrt(d);
		if(d<=geometry[i].Rayon)
		{
			OK = TRUE;
			del = if_selected(i);
			for(j=0;j<(gint)Natoms;j++)
			{
				if(geometry[i].ResidueNumber == geometry[j].ResidueNumber)
				{
					selectedj = index_selected(j);
					if(selectedj<0 && !del)
					{
						if(NumFatoms == NULL)
							NumFatoms = g_malloc((NFatoms+1)*sizeof(gint));
						else
							NumFatoms = g_realloc(NumFatoms,
									(NFatoms+1)*sizeof(gint));
						NumFatoms[NFatoms] = geometry[j].N;
						NFatoms++;
					}
					if(selectedj>=0 && del)
					{
						if((NFatoms-1)>0)
						{
							for(k=selectedj;k<(gint)(NFatoms-1);k++)
								NumFatoms[k] = NumFatoms[k+1];

							NumFatoms = g_realloc(NumFatoms,
									(NFatoms-1)*sizeof(gint));
							NFatoms--;
						}
						else
						{
							g_free(NumFatoms);
							NumFatoms = NULL;
							NFatoms = 0;
						}
					}

				}
			}
			break;

		}
	}
	drawGeom();
	return OK;
}
/*****************************************************************************/
void select_atoms_by_rectangle(gdouble x,gdouble y)
{
	gint i;
	gdouble x1=0;
	gdouble y1=0;
	gdouble x2=0;
	gdouble y2=0;
	gdouble xi;
	gdouble yi;

	if(x>BeginX)
	{
		x1 = BeginX;
		x2 = x;
	}
	else
	{
		x1 = x;
		x2 = BeginX;
	}
	if(y>BeginY)
	{
		y1 = BeginY;
		y2 = y;
	}
	else
	{
		y1 = y;
		y2 = BeginY;
	}
	if(!ShiftKeyPressed)
	{
		if(!NumFatoms)
			g_free(NumFatoms);
		NumFatoms = NULL;

		NFatoms = 0;
	}
	for(i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		xi = geometry[i].Xi;
		yi = geometry[i].Yi;
		if(xi>=x1 && xi<=x2 && yi>=y1 && yi<=y2 && !if_selected(i))
		{
			if(NumFatoms == NULL)
				NumFatoms = g_malloc((NFatoms+1)*sizeof(gint));
			else
				NumFatoms = g_realloc(NumFatoms,(NFatoms+1)*sizeof(gint));
			NumFatoms[NFatoms] = geometry[i].N;
			NFatoms++;

		}
	}
}
/********************************************************************************/
void draw_selection_rectangle(gdouble x,gdouble y)
{
	gdouble xi=0;
	gdouble yi=0;
	gdouble xf=0;
	gdouble yf=0;
        GdkColor color;
	GdkColormap *colormap;

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
	color.red = 65535;
	color.green = 65535;
	color.blue = 65535;
	
	drawGeom();
        gdk_colormap_alloc_color(colormap,&color,FALSE,TRUE);
    	gdk_gc_set_foreground(gc,&color);
	if(x>BeginX)
	{
		xi = BeginX;
		xf = x-BeginX;
	}
	else
	{
		xi = x;
		xf = BeginX-x;
	}
	if(y>BeginY)
	{
		yi = BeginY;
		yf = y-BeginY;
	}
	else
	{
		yi = y;
		yf = BeginY-y;
	}
	gdk_gc_set_line_attributes(gc,1,GDK_LINE_DOUBLE_DASH,GDK_CAP_NOT_LAST,GDK_JOIN_MITER);
#if !defined(G_OS_WIN32)
  	gdk_draw_rectangle (GeomDrawingArea->window,gc,FALSE,xi, yi, xf, yf);
#else
  	gdk_draw_rectangle (GeomDrawingArea->window,GeomDrawingArea->style->white_gc,FALSE,xi, yi, xf, yf);
#endif
}
/********************************************************************************/
void draw_selection_circle(gdouble x,gdouble y)
{
	gdouble xi=0;
	gdouble yi=0;
	gdouble xf=0;
	gdouble yf=0;
        GdkColor color;
	GdkColormap *colormap;

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
	color.red = 65535;
	color.green = 65535;
	color.blue = 65535;
	
	drawGeom();
        gdk_colormap_alloc_color(colormap,&color,FALSE,TRUE);
    	gdk_gc_set_foreground(gc,&color);
/*	GDK_LINE_ON_OFF_DASH, GDK_LINE_DOUBLE_DASH*/
	if(x>BeginX)
	{
		xi = BeginX;
		xf = x-BeginX;
	}
	else
	{
		xi = x;
		xf = BeginX-x;
	}
	if(y>BeginY)
	{
		yi = BeginY;
		yf = y-BeginY;
	}
	else
	{
		yi = y;
		yf = BeginY-y;
	}
	gdk_gc_set_line_attributes(gc,1,GDK_LINE_DOUBLE_DASH,GDK_CAP_NOT_LAST,GDK_JOIN_MITER);
	gdk_draw_arc(GeomDrawingArea->window,gc,FALSE,xi,yi,xf,yf,0,380*64);
}
/********************************************************************************/
static void delete_molecule()
{
	if(Natoms<1) return;

	add_geometry_to_fifo();
	geometry0 =Free_One_Geom(geometry0,Natoms);
	geometry =Free_One_Geom(geometry,Natoms);
	Natoms = 0;

	if(NumFatoms) g_free(NumFatoms);
  	NumFatoms = NULL;
  	NFatoms = 0;

	create_GeomXYZ_from_draw_grometry();

	Ddef = FALSE;
	drawGeom();
	set_statubar_pop_sel_atom();
	free_text_to_draw();
	change_of_center(NULL,NULL);
}
/********************************************************************************/
void copySelectedAtoms()
{
	Fragment F;
	gint i;
	gint k;

	if(Frag.NAtoms) FreeFragment(&Frag);
	Frag.NAtoms = 0;
	Frag.Atoms = NULL;
	if(NFatoms<1) return;
	F.NAtoms = NFatoms;
	F.Atoms = g_malloc(F.NAtoms*sizeof(Atom));

	for (k=0;k<(gint)NFatoms;k++)
	{
		for (i=0;i<(gint)Natoms;i++)
		{
			if(NumFatoms[k]==geometry[i].N)
			{
				F.Atoms[k].Symb=g_strdup(geometry[i].Prop.symbol);
				F.Atoms[k].mmType=g_strdup(geometry[i].mmType);
				F.Atoms[k].pdbType=g_strdup(geometry[i].pdbType);
				F.Atoms[k].Residue = g_strdup(geometry[i].Residue);
				F.Atoms[k].Coord[0]=geometry[i].X;
				F.Atoms[k].Coord[1]=geometry[i].Y;
				F.Atoms[k].Coord[2]=geometry[i].Z;
				F.Atoms[k].Charge=geometry[i].Charge;
				break;
			}
		}
		if(i==Natoms) break;
	}
	if(k!=NFatoms)
	{
		if(F.Atoms) g_free(F.Atoms);
		return;
	}

	F.atomToDelete = -1;
	F.atomToBondTo = -1;
	F.angleAtom    = -1;

	CenterFrag(&F);

	Frag = F;
	SetOperation (NULL,ADDFRAGMENT);
}
/********************************************************************************/
static gchar** freeList(gchar** strs, gint nlist)
{
	gint i;

	for(i=0;i<nlist;i++)
		if(strs[i])
			g_free(strs[i]);

	g_free(strs);

	return NULL;
}
/********************************************************************************/
static void setMMTypeOfselectedAtoms(GtkWidget* button, GtkWidget* entry)
{
	gint i;
	gint k = 0;
	G_CONST_RETURN gchar *tName;


	if(Natoms<1) return;
	tName = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(tName)<1) return;
	if(NFatoms<1) return;
	if(!NumFatoms) return;

	for (k=0;k<(gint)NFatoms;k++)
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N== NumFatoms[k])
		{
			if(geometry[i].mmType) g_free(geometry[i].mmType);
			geometry[i].mmType = g_strdup(tName);
			if(geometry0[i].mmType) g_free(geometry0[i].mmType);
			geometry0[i].mmType = g_strdup(tName);
		}
	}
	create_GeomXYZ_from_draw_grometry();
	if(!strcmp(tName,"N3")) reset_all_connections();
	drawGeom();
}
/********************************************************************************/
void setMMTypeOfselectedAtomsDlg()
{
	GtkWidget *winDlg;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint n=0;
	gchar** t = NULL;
	gchar tmp[100] = "UNK";
	gint i;
	gint k;

	if(Natoms<1) return;
	if(NFatoms<1) return;
	if(!NumFatoms) return;

	k=0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N == NumFatoms[k])
		{
			sprintf(tmp,"%s",geometry[i].mmType);
			break;
		}
	}
  
	winDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Set MM Type of selected atoms"));
	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(GeomDlg));

	add_child(GeomDlg,winDlg,gtk_widget_destroy,_(" Set Sel. Type."));
	g_signal_connect(G_OBJECT(winDlg),"delete_event",(GCallback)delete_child,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	hbox=create_hbox_false(vboxframe);
	n=0;
	t = getListGeomMMTypes(&n);
	entry = create_label_combo(hbox,_(" Type Name : "),t,n, TRUE,-1,-1);
	if(strcmp(tmp,"UNK")) gtk_entry_set_text(GTK_ENTRY(entry),tmp);
	gtk_editable_set_editable((GtkEditable*) entry,TRUE);
	if(t) freeList(t,n);

	gtk_widget_realize(winDlg);

	button = create_button(winDlg,_("Cancel"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	button = create_button(winDlg,_("OK"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)setMMTypeOfselectedAtoms,entry);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
    

	gtk_widget_show_all(winDlg);
}
/********************************************************************************/
static void setPDBTypeOfselectedAtoms(GtkWidget* button, GtkWidget* entry)
{
	gint i;
	gint k = 0;
	G_CONST_RETURN gchar *tName;


	if(Natoms<1) return;
	tName = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(tName)<1) return;
	if(NFatoms<1) return;
	if(!NumFatoms) return;

	for (k=0;k<(gint)NFatoms;k++)
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N== NumFatoms[k])
		{
			if(geometry[i].pdbType) g_free(geometry[i].pdbType);
			geometry[i].pdbType = g_strdup(tName);
			if(geometry0[i].pdbType) g_free(geometry0[i].pdbType);
			geometry0[i].pdbType = g_strdup(tName);
		}
	}
	create_GeomXYZ_from_draw_grometry();
	drawGeom();
}
/********************************************************************************/
void setPDBTypeOfselectedAtomsDlg()
{
	GtkWidget *winDlg;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint n=0;
	gchar** t = NULL;
	gchar tmp[100] = "UNK";
	gint i;
	gint k;

	if(Natoms<1) return;
	if(NFatoms<1) return;
	if(!NumFatoms) return;

	k=0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N == NumFatoms[k])
		{
			sprintf(tmp,"%s",geometry[i].pdbType);
			break;
		}
	}
  
	winDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Set PDB Type of selected atoms"));
	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(GeomDlg));

	add_child(GeomDlg,winDlg,gtk_widget_destroy,_(" Set Sel. Type."));
	g_signal_connect(G_OBJECT(winDlg),"delete_event",(GCallback)delete_child,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	hbox=create_hbox_false(vboxframe);
	n=0;
	t = getListPDBTypesFromGeom(&n);
	entry = create_label_combo(hbox,_(" Type Name : "),t,n, TRUE,-1,-1);
	if(strcmp(tmp,"UNK")) gtk_entry_set_text(GTK_ENTRY(entry),tmp);
	gtk_editable_set_editable((GtkEditable*) entry,TRUE);
	if(t) freeList(t,n);

	gtk_widget_realize(winDlg);

	button = create_button(winDlg,_("Cancel"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	button = create_button(winDlg,_("OK"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)setPDBTypeOfselectedAtoms,entry);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
    

	gtk_widget_show_all(winDlg);
}
/********************************************************************************/
static void setResidueNameOfselectedAtoms(GtkWidget* button, GtkWidget* entry)
{
	gint i;
	gint k = 0;
	G_CONST_RETURN gchar *tName;


	if(Natoms<1) return;
	tName = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(tName)<1) return;
	if(NFatoms<1) return;
	if(!NumFatoms) return;

	for (k=0;k<(gint)NFatoms;k++)
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N== NumFatoms[k])
		{
			if(geometry[i].Residue) g_free(geometry[i].Residue);
			geometry[i].Residue = g_strdup(tName);
			if(geometry0[i].Residue) g_free(geometry0[i].Residue);
			geometry0[i].Residue = g_strdup(tName);
		}
	}
	create_GeomXYZ_from_draw_grometry();
	drawGeom();
}
/********************************************************************************/
static gchar* getResidueNameOfselectedAtoms()
{
	gint i;
	gint k = 0;

	if(Natoms<1) return g_strdup("DUM");
	if(NFatoms<1) return  g_strdup("DUM");
	if(!NumFatoms) return  g_strdup("DUM");

	for (k=0;k<(gint)NFatoms;k++)
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N== NumFatoms[k])
		{
			 g_strdup(geometry[i].Residue);
		}
	}
	return  g_strdup("DUM");
}
/********************************************************************************/
void setResidueNameOfselectedAtomsDlg()
{
	GtkWidget *winDlg;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint n=0;
	gchar** t = NULL;
	gchar tmp[100] = "UNK";
	gint i;
	gint k;

	if(Natoms<1) return;
	if(NFatoms<1) return;
	if(!NumFatoms) return;

	k=0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N == NumFatoms[k])
		{
			sprintf(tmp,"%s",geometry[i].pdbType);
			break;
		}
	}
  
	winDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Set Residue name of selected atoms"));
	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(GeomDlg));

	add_child(GeomDlg,winDlg,gtk_widget_destroy,_(" Set Sel. Type."));
	g_signal_connect(G_OBJECT(winDlg),"delete_event",(GCallback)delete_child,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	hbox=create_hbox_false(vboxframe);
	n=1;
	t = g_malloc(sizeof(gchar*)*2);
	t[0]  = NULL;
	t[1]  = NULL;
	t[0] = getResidueNameOfselectedAtoms();
		
	entry = create_label_combo(hbox,_(" Residue Name : "),t,n, TRUE,-1,-1);
	g_free(t[0]);
	g_free(t);
	if(strcmp(tmp,"UNK")) gtk_entry_set_text(GTK_ENTRY(entry),tmp);
	gtk_editable_set_editable((GtkEditable*) entry,TRUE);

	gtk_widget_realize(winDlg);

	button = create_button(winDlg,_("Cancel"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	button = create_button(winDlg,_("OK"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)setResidueNameOfselectedAtoms,entry);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
    

	gtk_widget_show_all(winDlg);
}
/********************************************************************************/
gchar** getListCharges(gint* nlist)
{

	gchar** t = NULL;
	
	gint i;
	gint j;

	*nlist = 0;
	if(Natoms<1) return NULL;

	t = g_malloc(Natoms*sizeof(gchar*));
	*nlist = 1;
	t[*nlist-1] =  g_strdup_printf("%f",geometry0[0].Charge);


	for(i=1;i<Natoms;i++)
	{
		gboolean inList = FALSE;
		for(j=0;j<*nlist;j++)
		{
			if(geometry0[i].Charge==atof(t[j]))
			{
				inList = TRUE;
				break;
			}
		}
		if(!inList)
		{
			(*nlist)++;
			t[*nlist-1] =  g_strdup_printf("%f",geometry0[i].Charge);
		}
	}
	t = g_realloc(t,*nlist*sizeof(gchar*));

	return t;
}
/********************************************************************************/
static void setChargeOfselectedAtoms(GtkWidget* button, GtkWidget* entry)
{
	gint i;
	gint k = 0;
	G_CONST_RETURN gchar *tValue;


	if(Natoms<1) return;
	tValue = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(tValue)<1) return;
	if(NFatoms<1) return;
	if(!NumFatoms) return;

	for (k=0;k<(gint)NFatoms;k++)
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N== NumFatoms[k])
		{
			geometry[i].Charge = atof(tValue);
			geometry0[i].Charge = atof(tValue);
		}
	}
	create_GeomXYZ_from_draw_grometry();

	drawGeom();
}
/********************************************************************************/
void setChargeOfselectedAtomsDlg()
{
	GtkWidget *winDlg;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint n=0;
	gchar** t = NULL;
	gchar tmp[100] = "UNK";
	gint i;
	gint k;

	if(Natoms<1) return;
	if(NFatoms<1) return;
	if(!NumFatoms) return;

	k=0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N == NumFatoms[k])
		{
			sprintf(tmp,"%f",geometry[i].Charge);
			break;
		}
	}
  
	winDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Set Charge of selected atoms"));
	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(GeomDlg));

	add_child(GeomDlg,winDlg,gtk_widget_destroy,_(" Set Sel. Charge."));
	g_signal_connect(G_OBJECT(winDlg),"delete_event",(GCallback)delete_child,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	hbox=create_hbox_false(vboxframe);
	n=0;
	t = getListCharges(&n);
	entry = create_label_combo(hbox, _(" Charge : "),t,n, TRUE,-1,-1);
	if(strcmp(tmp,"UNK")) gtk_entry_set_text(GTK_ENTRY(entry),tmp);
	gtk_editable_set_editable((GtkEditable*) entry,TRUE);
	if(t) freeList(t,n);

	gtk_widget_realize(winDlg);

	button = create_button(winDlg,_("Cancel"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	button = create_button(winDlg,_("OK"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)setChargeOfselectedAtoms,entry);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
    

	gtk_widget_show_all(winDlg);
}
/********************************************************************************/
static void scaleChargesOfSelectedAtoms(GtkWidget* button, GtkWidget* entry)
{
	gint i;
	gint k;
	G_CONST_RETURN gchar *strEntry;
	gdouble factor = 1.0;


	if(Natoms<1) return;
	if(NFatoms<1) return;
	if(!NumFatoms) return;
	strEntry = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(strEntry)<1) return;
	factor = atof(strEntry);

	for (k=0;k<(gint)NFatoms;k++)
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N== NumFatoms[k])
		{
			geometry[i].Charge *= factor;
			geometry0[i].Charge *= factor;
		}
	}

	drawGeom();
}
/********************************************************************************/
void scaleChargesOfSelectedAtomsDlg()
{
	GtkWidget *winDlg;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vboxframe;
  
	winDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Scale charges of selected atoms"));
	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(GeomDlg));

	add_child(GeomDlg,winDlg,gtk_widget_destroy,_(" Scal. Char. "));
	g_signal_connect(G_OBJECT(winDlg),"delete_event",(GCallback)delete_child,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	hbox=create_hbox_false(vboxframe);
	label = gtk_label_new(_(" Factor : "));
	gtk_box_pack_start( GTK_BOX(hbox), label,TRUE,TRUE,0);
	entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entry),"1.0");
	gtk_box_pack_start( GTK_BOX(hbox), entry,TRUE,TRUE,0);

	gtk_widget_realize(winDlg);

	button = create_button(winDlg,_("Cancel"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	button = create_button(winDlg,_("OK"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)scaleChargesOfSelectedAtoms,entry);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
    

	gtk_widget_show_all(winDlg);
}
/*****************************************************************************/
static void set_vect_ij(gint i, gint j, gdouble V[])
{
	V[0] = geometry0[j].X-geometry0[i].X;
	V[1] = geometry0[j].Y-geometry0[i].Y;
	V[2] = geometry0[j].Z-geometry0[i].Z;
}
/*****************************************************************************/
static void add_hydrogen_atoms(gint addToI, gint nH, gchar* HType)
{
	static SAtomsProp propH = {0};
	static gint begin = 0;
	gint i;
	gint k;
	gdouble dist = 1.0;
	gint geom = 0; /* 0=3D, 1=planar, 2=linear */
	gdouble angle = 109.5*PI/180.0;
	gint nV;
	gint nC;
	gint* listOfConnectedAtoms = NULL;

	if(begin==0)
	{
		begin++;
		propH = prop_atom_get("H");
	}
	dist = (geometry[addToI].Prop.covalentRadii+propH.covalentRadii)*0.9;
	if(nH==1 && strcmp(geometry0[addToI].Prop.symbol,"N")) 
	{
		angle = 180.0*PI/180.0;
		geom = 2;
	}
	if(nH==2 && !strcmp(geometry0[addToI].Prop.symbol,"N")) 
	{
		angle = 120.0*PI/180.0;
		geom = 1;
	}
	nV =geometry[addToI].Prop.maximumBondValence;
	if(nV<1) return;
	listOfConnectedAtoms = g_malloc(nV*sizeof(gint));

	if(Natoms>0)
	{
		geometry0 = g_realloc(geometry0,(Natoms+nH)*sizeof(GeomDef));
		geometry = g_realloc(geometry,(Natoms+nH)*sizeof(GeomDef)); 
	}
	else
	{
		geometry0 = g_malloc(nH*sizeof(GeomDef));
		geometry = g_malloc(nH*sizeof(GeomDef)); 
	}


	nC = 0;
	for(i=0;i<(gint)Natoms;i++)
	{
		gint ni = geometry[i].N-1;
		if(geometry[addToI].typeConnections[ni]>0)
		{
			listOfConnectedAtoms[nC] = i;
			nC++;
		}
	}

	for(i=0;i<nH;i++)
	{
		gdouble v1[3];
		gdouble v2[3];
		gdouble v3[3];
		gdouble v4[3];
		gdouble v5[3];
		for(k=0;k<3;k++)
			v1[k] = rand()/(gdouble)RAND_MAX-0.5;
		v3d_normal(v1);

		switch (nC)
		{
			case 1:
				{
					set_vect_ij(addToI, listOfConnectedAtoms[0], v2);
					v3d_normal(v2);
					v3d_cross(v1,v2,v3);
					v3d_normal(v3);
					v3d_scale(v2,cos(angle));
					v3d_scale(v3,sin(angle));
					v3d_add(v2,v3,v1);
					break;
				}
			case 2:
				{
					set_vect_ij(addToI, listOfConnectedAtoms[0], v2);
					v3d_normal(v2);
					set_vect_ij(addToI, listOfConnectedAtoms[1], v3);
					v3d_normal(v3);
					if(geom==0 || nH>1)
					{
						v3d_add(v2,v3,v4);
						if(v3d_length(v4)>1e-2) v3d_normal(v4);
						v3d_scale(v4,-1.0);
						v3d_cross(v2,v3,v5);
						if(v3d_length(v5)<1e-2)
							for(k=0;k<3;k++) v5[k] = rand()/(gdouble)RAND_MAX-0.5;
						v3d_normal(v5);
						v3d_scale(v4,0.5);
						v3d_scale(v5,0.5);
						v3d_add(v4,v5,v1);
						v3d_normal(v1);
					}
					else
					{
						v3d_add(v2,v3,v1);
						v3d_normal(v1);
						v3d_scale(v1,-1.0);
					}
					break;
				}
			default:
				if(nC>=3)
				{
					set_vect_ij(addToI, listOfConnectedAtoms[nC-3], v2);
					v3d_normal(v2);
					set_vect_ij(addToI, listOfConnectedAtoms[nC-2], v3);
					v3d_normal(v3);
					set_vect_ij(addToI, listOfConnectedAtoms[nC-1], v4);
					v3d_normal(v4);
					v3d_add(v2,v3,v5);
					v3d_add(v5,v4,v1);
					if(v3d_length(v1)<2e-1) v3d_cross(v2,v3,v1);
					v3d_normal(v1);
					v3d_scale(v1,-1.0);
					break;
				}

		}
		geometry[Natoms].X= geometry0[addToI].X + dist*v1[0];
		geometry[Natoms].Y= geometry0[addToI].Y + dist*v1[1];
		geometry[Natoms].Z= geometry0[addToI].Z + dist*v1[2];
		geometry[Natoms].Prop = prop_atom_get("H");
		geometry[Natoms].pdbType = g_strdup(HType);
		if(Natoms==0)
		{
			geometry[Natoms].Residue = g_strdup("H");
			geometry[Natoms].ResidueNumber = 0;
		}
		else
		{
			geometry[Natoms].Residue = g_strdup(geometry[addToI].Residue);
			geometry[Natoms].ResidueNumber = geometry[addToI].ResidueNumber;
		}
		{
			gdouble charge;
			gchar* mmType = getMMTypeFromPDBTpl(geometry[Natoms].Residue,geometry[Natoms].pdbType,&charge);
			if(!strcmp(mmType,"UNK"))
			{
				geometry[Natoms].mmType = g_strdup(HType);
				g_free(mmType);
			}
			else geometry[Natoms].mmType = mmType;
		}
		geometry[Natoms].Layer = geometry[addToI].Layer;
		geometry[Natoms].N = Natoms+1;
		geometry[Natoms].typeConnections = NULL;
		geometry[Natoms].Charge = 0.0;
		geometry[Natoms].Variable = TRUE;
		geometry[Natoms].show = ShowHydrogenAtoms;

		geometry0[Natoms].X = geometry[Natoms].X;
		geometry0[Natoms].Y = geometry[Natoms].Y;
		geometry0[Natoms].Z = geometry[Natoms].Z;
		geometry0[Natoms].Prop = prop_atom_get("H");
		geometry0[Natoms].mmType = g_strdup(geometry[Natoms].mmType);
		geometry0[Natoms].pdbType = g_strdup(geometry[Natoms].pdbType);
		geometry0[Natoms].Layer = geometry[Natoms].Layer;
		geometry0[Natoms].Residue = g_strdup(geometry[Natoms].Residue);
		geometry0[Natoms].ResidueNumber = geometry[Natoms].ResidueNumber;
		geometry0[Natoms].Charge = 0.0;
		geometry0[Natoms].Variable = TRUE;
		geometry0[Natoms].N = geometry[Natoms].N;
		geometry0[Natoms].show = geometry[Natoms].show;
		geometry0[Natoms].typeConnections = NULL;

		Natoms++;
		nC++;
		listOfConnectedAtoms = g_realloc(listOfConnectedAtoms, nC*sizeof(gint));
		listOfConnectedAtoms[nC-1] = Natoms-1;
	}
	{
		gint i,j;
		for(i=0;i<(gint)Natoms-nH;i++)
		{
			geometry[i].typeConnections = g_realloc(geometry[i].typeConnections,Natoms*sizeof(gint));
			for(j=Natoms-nH;j<(gint)Natoms;j++) geometry[i].typeConnections[j] = 0;
			geometry0[i].typeConnections = g_realloc(geometry0[i].typeConnections,Natoms*sizeof(gint));
			for(j=Natoms-nH;j<(gint)Natoms;j++) geometry0[i].typeConnections[j] = 0;
		}
		for(i=(gint)Natoms-nH;i<(gint)Natoms;i++)
		{
			geometry[i].typeConnections = g_malloc(Natoms*sizeof(gint));
			for(j=0;j<(gint)Natoms;j++) geometry[i].typeConnections[j] = 0;
			geometry0[i].typeConnections = g_malloc(Natoms*sizeof(gint));
			for(j=0;j<(gint)Natoms;j++) geometry0[i].typeConnections[j] = 0;
		}
		for(i=(gint)Natoms-nH;i<(gint)Natoms;i++)
		{
			geometry[addToI].typeConnections[geometry[i].N-1] = 1;
			geometry0[addToI].typeConnections[geometry0[i].N-1] = 1;
			geometry[i].typeConnections[geometry[addToI].N-1] = 1;
			geometry0[i].typeConnections[geometry0[addToI].N-1] = 1;
		}
		/* adjust_multiple_bonds_with_one_atom(addToI);*/
	}
	if(listOfConnectedAtoms) g_free(listOfConnectedAtoms);
}
/*****************************************************************************/
static gboolean add_hydrogen_atom(gint addToI)
{
	gint nMultiple = 0;
	gint nAll = 0;
	gchar HType[100] = "H";
	gint j;
	gint nV = 0;
	gint nH = 0;

	
	if(Natoms<1 ) return FALSE;
	if(addToI>Natoms-1) return FALSE;

	nV =geometry[addToI].Prop.maximumBondValence;
	if(nV<1) return FALSE;

	for(j=0;j<(gint)Natoms;j++)
	{
		gint nj = geometry[j].N-1;
		if(geometry[addToI].typeConnections[nj]>1) nMultiple++;
		nAll += geometry[addToI].typeConnections[nj];
	}

	nH = nV - nAll;
	if(nH<1)
	{
		return FALSE;
	}
	if(nAll>=geometry[addToI].Prop.maximumBondValence && nMultiple==0) 
	{
		return FALSE;
	}


	if(!strcmp(geometry[addToI].pdbType,"CA")) sprintf(HType,"HA");
	else if(!strcmp(geometry[addToI].pdbType,"OH")) sprintf(HType,"HO");
	else if(strstr(geometry[addToI].pdbType,"OE")) sprintf(HType,"HO");
	else if(strstr(geometry[addToI].pdbType,"OD")) sprintf(HType,"HO");
	else if(!strcmp(geometry[addToI].pdbType,"OG1")) sprintf(HType,"HG1");
	else if(!strcmp(geometry[addToI].pdbType,"CT")) sprintf(HType,"HT");
	else if(!strcmp(geometry[addToI].pdbType,"CB")) sprintf(HType,"HB1");
	else if(!strcmp(geometry[addToI].pdbType,"SG")) sprintf(HType,"HG");
	else if(!strcmp(geometry[addToI].pdbType,"CD1")) sprintf(HType,"HD11");
	else if(!strcmp(geometry[addToI].pdbType,"CD2")) sprintf(HType,"HD22");
	else if(strstr(geometry[addToI].pdbType,"CD")) sprintf(HType,"HD1");
	else if(!strcmp(geometry[addToI].pdbType,"CG1")) sprintf(HType,"HG11");
	else if(!strcmp(geometry[addToI].pdbType,"CG2")) sprintf(HType,"HG22");
	else if(strstr(geometry[addToI].pdbType,"CG")) sprintf(HType,"HG1");
	else if(!strcmp(geometry[addToI].pdbType,"CE1")) sprintf(HType,"HE1");
	else if(!strcmp(geometry[addToI].pdbType,"CE2")) sprintf(HType,"HE2");
	else if(strstr(geometry[addToI].pdbType,"CE")) sprintf(HType,"HE2");
	else if(strstr(geometry[addToI].pdbType,"NZ")) sprintf(HType,"HZ2");
	else if(strstr(geometry[addToI].pdbType,"NZ")) sprintf(HType,"HZ2");
	else if(!strcmp(geometry[addToI].pdbType,"CZ1")) sprintf(HType,"HZ1");
	else if(!strcmp(geometry[addToI].pdbType,"CZ2")) sprintf(HType,"HZ2");
	else if(!strcmp(geometry[addToI].pdbType,"CZ3")) sprintf(HType,"HZ3");
	else if(strstr(geometry[addToI].pdbType,"CZ")) sprintf(HType,"HZ");
	else if(!strcmp(geometry[addToI].pdbType,"NE")) sprintf(HType,"HE");
	else if(!strcmp(geometry[addToI].pdbType,"N")) sprintf(HType,"H");
	else if(!strcmp(geometry[addToI].pdbType,"NE1")) sprintf(HType,"HE1");
	else if(!strcmp(geometry[addToI].pdbType,"NH1")) sprintf(HType,"HH11");
	else if(!strcmp(geometry[addToI].pdbType,"NH2")) sprintf(HType,"HH21");
	else if(!strcmp(geometry[addToI].pdbType,"CH1")) sprintf(HType,"HH1");
	else if(!strcmp(geometry[addToI].pdbType,"CH2")) sprintf(HType,"HH2");
	else if(!strcmp(geometry[addToI].pdbType,"CH3")) sprintf(HType,"H2");
	else 
	{
		sprintf(HType,"%s1",geometry[addToI].pdbType);
		if(strlen(HType)>0) HType[0] = 'H';
	}
	add_hydrogen_atoms(addToI, nH, HType);

	reset_charges_multiplicities();
	return TRUE;
}
/*****************************************************************************/
static gboolean add_max_hydrogen_atom(gint addToI)
{
	gint nAll = 0;
	gchar HType[100] = "H";
	gint j;
	gint nV = 0;
	gint nH = 0;

	
	if(Natoms<1 ) return FALSE;
	if(addToI>Natoms-1) return FALSE;

	nV =geometry[addToI].Prop.maximumBondValence;
	if(nV<1) return FALSE;

	for(j=0;j<(gint)Natoms;j++)
	{
		gint nj = geometry[j].N-1;
		if(geometry[addToI].typeConnections[nj]>0)
		nAll += 1;
	}
	/*
	printf("Type = %s res = %s\n", geometry[addToI].mmType, geometry[addToI].Residue);
	*/

	nH = nV - nAll;
	if(nH<1)
	{
		return FALSE;
	}

	if(!strcmp(geometry[addToI].pdbType,"CA")) sprintf(HType,"HA");
	else if(!strcmp(geometry[addToI].pdbType,"OH")) sprintf(HType,"HO");
	else if(strstr(geometry[addToI].pdbType,"OE")) sprintf(HType,"HO");
	else if(strstr(geometry[addToI].pdbType,"OD")) sprintf(HType,"HO");
	else if(!strcmp(geometry[addToI].pdbType,"OG1")) sprintf(HType,"HG1");
	else if(!strcmp(geometry[addToI].pdbType,"CT")) sprintf(HType,"HT");
	else if(!strcmp(geometry[addToI].pdbType,"CB")) sprintf(HType,"HB1");
	else if(!strcmp(geometry[addToI].pdbType,"SG")) sprintf(HType,"HG");
	else if(!strcmp(geometry[addToI].pdbType,"CD1")) sprintf(HType,"HD11");
	else if(!strcmp(geometry[addToI].pdbType,"CD2")) sprintf(HType,"HD22");
	else if(strstr(geometry[addToI].pdbType,"CD")) sprintf(HType,"HD1");
	else if(!strcmp(geometry[addToI].pdbType,"CG1")) sprintf(HType,"HG11");
	else if(!strcmp(geometry[addToI].pdbType,"CG2")) sprintf(HType,"HG22");
	else if(strstr(geometry[addToI].pdbType,"CG")) sprintf(HType,"HG1");
	else if(!strcmp(geometry[addToI].pdbType,"CE1")) sprintf(HType,"HE1");
	else if(!strcmp(geometry[addToI].pdbType,"CE2")) sprintf(HType,"HE2");
	else if(strstr(geometry[addToI].pdbType,"CE")) sprintf(HType,"HE2");
	else if(strstr(geometry[addToI].pdbType,"NZ")) sprintf(HType,"HZ2");
	else if(strstr(geometry[addToI].pdbType,"NZ")) sprintf(HType,"HZ2");
	else if(!strcmp(geometry[addToI].pdbType,"CZ1")) sprintf(HType,"HZ1");
	else if(!strcmp(geometry[addToI].pdbType,"CZ2")) sprintf(HType,"HZ2");
	else if(!strcmp(geometry[addToI].pdbType,"CZ3")) sprintf(HType,"HZ3");
	else if(strstr(geometry[addToI].pdbType,"CZ")) sprintf(HType,"HZ");
	else if(!strcmp(geometry[addToI].pdbType,"NE")) sprintf(HType,"HE");
	else if(!strcmp(geometry[addToI].pdbType,"N")) sprintf(HType,"H");
	else if(!strcmp(geometry[addToI].pdbType,"NE1")) sprintf(HType,"HE1");
	else if(!strcmp(geometry[addToI].pdbType,"NH1")) sprintf(HType,"HH11");
	else if(!strcmp(geometry[addToI].pdbType,"NH2")) sprintf(HType,"HH21");
	else if(!strcmp(geometry[addToI].pdbType,"CH1")) sprintf(HType,"HH1");
	else if(!strcmp(geometry[addToI].pdbType,"CH2")) sprintf(HType,"HH2");
	else if(!strcmp(geometry[addToI].pdbType,"CH3")) sprintf(HType,"H2");
	else 
	{
		sprintf(HType,"%s1",geometry[addToI].pdbType);
		if(strlen(HType)>0) HType[0] = 'H';
	}
	add_hydrogen_atoms(addToI, nH, HType);

	setMultipleBonds();
	reset_charges_multiplicities();
	return TRUE;
}
/*****************************************************************************/
static gboolean add_one_hydrogen_atom(gint addToI)
{
	gint nMultiple = 0;
	gint nAll = 0;
	gchar HType[100] = "H";
	gint j;
	gint nV = 0;
	gint nH = 0;
	gint nC = 0;

	
	if(Natoms<1 ) return FALSE;
	if(addToI>Natoms-1) return FALSE;

	nV =geometry[addToI].Prop.maximumBondValence;
	if(nV<1) return FALSE;

	for(j=0;j<(gint)Natoms;j++)
	{
		gint nj = geometry[j].N-1;
		if(geometry[addToI].typeConnections[nj]>0) nC++;
		if(geometry[addToI].typeConnections[nj]>1) nMultiple++;
		nAll += geometry[addToI].typeConnections[nj];
	}

	nH = nV - nC;
	if(nH<1)
	{
		return FALSE;
	}
	else nH = 1;
	if(nAll>=geometry[addToI].Prop.maximumBondValence && nMultiple==0) 
	{
		return FALSE;
	}


	if(!strcmp(geometry[addToI].pdbType,"CA")) sprintf(HType,"HA");
	else if(!strcmp(geometry[addToI].pdbType,"OH")) sprintf(HType,"HO");
	else if(strstr(geometry[addToI].pdbType,"OE")) sprintf(HType,"HO");
	else if(strstr(geometry[addToI].pdbType,"OD")) sprintf(HType,"HO");
	else if(!strcmp(geometry[addToI].pdbType,"OG1")) sprintf(HType,"HG1");
	else if(!strcmp(geometry[addToI].pdbType,"CT")) sprintf(HType,"HT");
	else if(!strcmp(geometry[addToI].pdbType,"CB")) sprintf(HType,"HB1");
	else if(!strcmp(geometry[addToI].pdbType,"SG")) sprintf(HType,"HG");
	else if(!strcmp(geometry[addToI].pdbType,"CD1")) sprintf(HType,"HD11");
	else if(!strcmp(geometry[addToI].pdbType,"CD2")) sprintf(HType,"HD22");
	else if(strstr(geometry[addToI].pdbType,"CD")) sprintf(HType,"HD1");
	else if(!strcmp(geometry[addToI].pdbType,"CG1")) sprintf(HType,"HG11");
	else if(!strcmp(geometry[addToI].pdbType,"CG2")) sprintf(HType,"HG22");
	else if(strstr(geometry[addToI].pdbType,"CG")) sprintf(HType,"HG1");
	else if(!strcmp(geometry[addToI].pdbType,"CE1")) sprintf(HType,"HE1");
	else if(!strcmp(geometry[addToI].pdbType,"CE2")) sprintf(HType,"HE2");
	else if(strstr(geometry[addToI].pdbType,"CE")) sprintf(HType,"HE2");
	else if(strstr(geometry[addToI].pdbType,"NZ")) sprintf(HType,"HZ2");
	else if(strstr(geometry[addToI].pdbType,"NZ")) sprintf(HType,"HZ2");
	else if(!strcmp(geometry[addToI].pdbType,"CZ1")) sprintf(HType,"HZ1");
	else if(!strcmp(geometry[addToI].pdbType,"CZ2")) sprintf(HType,"HZ2");
	else if(!strcmp(geometry[addToI].pdbType,"CZ3")) sprintf(HType,"HZ3");
	else if(strstr(geometry[addToI].pdbType,"CZ")) sprintf(HType,"HZ");
	else if(!strcmp(geometry[addToI].pdbType,"NE")) sprintf(HType,"HE");
	else if(!strcmp(geometry[addToI].pdbType,"N")) sprintf(HType,"H");
	else if(!strcmp(geometry[addToI].pdbType,"NE1")) sprintf(HType,"HE1");
	else if(!strcmp(geometry[addToI].pdbType,"NH1")) sprintf(HType,"HH11");
	else if(!strcmp(geometry[addToI].pdbType,"NH2")) sprintf(HType,"HH21");
	else if(!strcmp(geometry[addToI].pdbType,"CH1")) sprintf(HType,"HH1");
	else if(!strcmp(geometry[addToI].pdbType,"CH2")) sprintf(HType,"HH2");
	else if(!strcmp(geometry[addToI].pdbType,"CH3")) sprintf(HType,"H2");
	else 
	{
		sprintf(HType,"%s1",geometry[addToI].pdbType);
		if(strlen(HType)>0) HType[0] = 'H';
	}
	add_hydrogen_atoms(addToI, nH, HType);

	return TRUE;
}
/********************************************************************************/
void addMaxHydrogens()
{
	gint i;
	gint k = 0;


	if(Natoms<1) return;
	if(!NumFatoms) SelectAllAtoms();
	if(NFatoms<1) return;
	if(!NumFatoms) return;

	for (k=0;k<(gint)NFatoms;k++)
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N== NumFatoms[k])
		{
			add_max_hydrogen_atom(i);
			break;
		}
	}
	reset_multiple_bonds();
	create_GeomXYZ_from_draw_grometry();
	reset_charges_multiplicities();
	drawGeom();
}
/********************************************************************************/
void addHydrogens()
{
	gint i;
	gint k = 0;


	if(Natoms<1) return;
	if(!NumFatoms) SelectAllAtoms();
	if(NFatoms<1) return;
	if(!NumFatoms) return;

	for (k=0;k<(gint)NFatoms;k++)
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N== NumFatoms[k])
		{
			add_hydrogen_atom(i);
			break;
		}
	}
	create_GeomXYZ_from_draw_grometry();
	reset_charges_multiplicities();
	drawGeom();
}
/********************************************************************************/
void addOneHydrogen()
{
	gint i;
	gint k = 0;


	if(Natoms<1) return;
	if(!NumFatoms) SelectAllAtoms();
	if(NFatoms<1) return;
	if(!NumFatoms) return;

	for (k=0;k<(gint)NFatoms;k++)
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].N== NumFatoms[k])
		{
			add_one_hydrogen_atom(i);
			break;
		}
	}
	create_GeomXYZ_from_draw_grometry();
	reset_charges_multiplicities();
	drawGeom();
}
/*******************************************************************/
void adjust_hydrogens_connected_to_atom(gint ia)
{
	if(Natoms<1) return;
	if(ia<0 || ia>Natoms-1) return;
	{
		gint nj = 0;
		gint j;
		gint nBondsA = 0;

		for(j=0;j<(gint)Natoms;j++)
		{
			nj = geometry[j].N-1;
			if(geometry[ia].typeConnections && geometry[ia].typeConnections[nj]>0) 
				nBondsA += geometry[ia].typeConnections[nj];
		}
		if( nBondsA==geometry[ia].Prop.maximumBondValence ) return;
		if(nBondsA<geometry[ia].Prop.maximumBondValence)
		{
			gint nHA = geometry[ia].Prop.maximumBondValence-nBondsA;
			for(j=0;j<nHA;j++) add_hydrogen_atom(ia);
			copy_connections(geometry0, geometry, Natoms);
		}
		if(nBondsA>geometry[ia].Prop.maximumBondValence)
		{
			gint nHA = nBondsA-geometry[ia].Prop.maximumBondValence;
			deleteHydrogensConnectedTo(ia, nHA);
		}
		reset_charges_multiplicities();
	}
}
/*******************************************************************/
void adjust_hydrogens_connected_to_atoms(gint ia, gint ib)
{
	if(Natoms<2) return;
	if(ia<0 || ib<0 || ia>Natoms-1 || ib>Natoms-1) return;
	if(!geometry[ia].typeConnections) return;
	if(!geometry[ib].typeConnections) return;
	{
		gint nj = 0;
		gint j;
		gint nBondsA = 0;
		gint nBondsB = 0;

		for(j=0;j<(gint)Natoms;j++)
		{
			nj = geometry[j].N-1;
		 	if(geometry[ia].typeConnections[nj]>0) 
				nBondsA += geometry[ia].typeConnections[nj];
		 	if(geometry[ib].typeConnections[nj]>0) 
				nBondsB += geometry[ib].typeConnections[nj];
		}
		if(
			nBondsA==geometry[ia].Prop.maximumBondValence &&
			nBondsB==geometry[ib].Prop.maximumBondValence 
		) return;
		if(nBondsA<geometry[ia].Prop.maximumBondValence)
		{
			gint nHA = geometry[ia].Prop.maximumBondValence-nBondsA;
			for(j=0;j<nHA;j++) add_hydrogen_atom(ia);
			copy_connections(geometry0, geometry, Natoms);
		}
		if(nBondsB<geometry[ib].Prop.maximumBondValence)
		{
			gint nHB = geometry[ib].Prop.maximumBondValence-nBondsB;
			for(j=0;j<nHB;j++) add_hydrogen_atom(ib);
			copy_connections(geometry0, geometry, Natoms);
		}
		if(nBondsA>geometry[ia].Prop.maximumBondValence && nBondsB<=geometry[ib].Prop.maximumBondValence)
		{
			gint nHA = nBondsA-geometry[ia].Prop.maximumBondValence;
			deleteHydrogensConnectedTo(ia, nHA);
		}
		if(nBondsB>geometry[ib].Prop.maximumBondValence && nBondsA<=geometry[ia].Prop.maximumBondValence)
		{
			gint nHB = nBondsA-geometry[ib].Prop.maximumBondValence;
			deleteHydrogensConnectedTo(ib, nHB);
		}
		if(nBondsA>geometry[ia].Prop.maximumBondValence && nBondsB>geometry[ib].Prop.maximumBondValence)
		{
			gint nHA = nBondsA-geometry[ia].Prop.maximumBondValence;
			gint nHB = nBondsB-geometry[ib].Prop.maximumBondValence;
			gint* numHA = NULL;
			gint* numHB = NULL;
			gint kA = 0;
			gint kB = 0;
			gint k;
			gint i;
			GeomDef tmp;
			gint* oldN = NULL;

			if(nHA>0) numHA = g_malloc(nHA*sizeof(gint));
			if(nHB>0) numHB = g_malloc(nHB*sizeof(gint));
			for(j=0;j<nHA;j++) numHA[j] = -1;
			for(j=0;j<nHB;j++) numHB[j] = -1;

			buildRotation();
			
			kA = 0;
			for(j=0;j<(gint)Natoms;j++)
			{
				if(j==ia) continue;
				nj = geometry[j].N-1;
				if(geometry[ia].typeConnections[nj] &&
				!strcmp(geometry[j].Prop.symbol,"H"))
				{
					numHA[kA++] = geometry[j].N;
					if(kA>=nHA) break;
				}
			}
			kB = 0;
			for(j=0;j<(gint)Natoms;j++)
			{
				if(j==ib) continue;
				nj = geometry[j].N-1;
				if(geometry[ib].typeConnections[nj] &&
				!strcmp(geometry[j].Prop.symbol,"H"))
				{
					numHB[kB++] = geometry[j].N;
					if(kB>=nHB) break;
				}
			}
			copy_connections(geometry0, geometry, Natoms);
			for (i=0;i<(gint)Natoms-1;i++)
			{
				gboolean toDelete = FALSE;
				for(k=0;k<kA;k++) if(geometry[i].N==numHA[k]) {toDelete = TRUE; break;}
				if(!toDelete) for(k=0;k<kB;k++) if(geometry[i].N==numHB[k]) {toDelete = TRUE; break;}
				if(!toDelete) continue;
				for(j=i+1;j<(gint)Natoms;j++)
				{
					gboolean toDelete = FALSE;
					for(k=0;k<kA;k++) if(geometry[j].N==numHA[k]) {toDelete = TRUE; break;}
					if(!toDelete) for(k=0;k<kB;k++) if(geometry[j].N==numHB[k]) {toDelete = TRUE; break;}
					if(toDelete) continue;
					tmp = geometry0[i];
					geometry0[i] = geometry0[j];
					geometry0[j] = tmp ;
					tmp = geometry[i];
					geometry[i] = geometry[j];
					geometry[j] = tmp ;
					break;
				}
			}
			j = 0;
			for (i=0;i<(gint)Natoms;i++)
			{
				gboolean toDelete = FALSE;
				for(k=0;k<kA;k++) if(geometry[i].N==numHA[k]) {toDelete = TRUE; break;}
				if(!toDelete) for(k=0;k<kB;k++) if(geometry[i].N==numHB[k]) {toDelete = TRUE; break;}
				if(!toDelete) continue;
				if(geometry0[i].typeConnections) g_free(geometry0[i].typeConnections);
				if(geometry[i].typeConnections) g_free(geometry[i].typeConnections);
				geometry0[i].typeConnections=NULL;
				geometry[i].typeConnections=NULL;
				j++;
			}
			if(numHA) g_free(numHA);
			if(numHB) g_free(numHB);
	
			oldN = g_malloc(Natoms*sizeof(gint));
			for (i=0;i<(gint)Natoms;i++) oldN[i] = geometry0[i].N-1;
			Natoms-=kA+kB;

			for(j=0;j<(gint)NFatoms;j++)
			{
				for (i=0;i<(gint)Natoms;i++)
					if(NumFatoms[j] ==(gint) geometry[i].N) { NumFatoms[j] = i+1; break;}
				if(i==(gint)Natoms)  NumFatoms[j] =-1;
			}
			for(j=0;j<(gint)NFatoms;j++)
				if( NumFatoms[j]<0)
				{
					for (i=j;i<(gint)NFatoms-1;i++)
				 		NumFatoms[i]= NumFatoms[i+1];
					NFatoms--;
					j--;
				}
			for (i=0;i<(gint)Natoms;i++)
			{
				geometry0[i].N = i+1;
				geometry[i].N = i+1;
			}
			/* in geometry0 : old connections , in geometry new connection */
			for (i=0;i<(gint)Natoms;i++)
			{
				if(geometry[i].typeConnections)
				{
					for(j=0;j<(gint)Natoms;j++)
					{
						geometry[i].typeConnections[j] = geometry0[i].typeConnections[oldN[j]];
					}
				}
			}
			if(oldN) g_free(oldN);
			copy_connections(geometry0, geometry, Natoms);
			if(Natoms>0)
			{
				geometry0 = g_realloc(geometry0,Natoms*sizeof(GeomDef));
				geometry = g_realloc(geometry,Natoms*sizeof(GeomDef)); 
			}
			else
			{
				if(geometry0) g_free(geometry0); geometry0 = NULL;
				if(geometry) g_free(geometry);
				geometry = NULL;
				Natoms = 0;
			}
			Ddef = FALSE;
			reset_charges_multiplicities();
		}
	}
}
/*****************************************************************************/
static void add_hydrogen_atoms_tpl(gint addToI, gint nA)
{
	static SAtomsProp propH = {0};
	static gint begin = 0;
	gint i;
	gint k;
	gdouble dist = 1.0;
	gint geom = 0; /* 0=3D, 1=planar, 2=linear */
	gdouble angle = 109.5*PI/180.0;
	gint nV;
	gint nC;
	gint* listOfConnectedAtoms = NULL;
	gint nH;
	gchar* hAtoms[10];
	gint nAll;
	gint nOldH = 0;

	nV =geometry[addToI].Prop.maximumBondValence;
	if(nV<1) return;
	nAll = 0;
	nOldH = 0;
	for(i=0;i<(gint)nA;i++)
	{
		gint ni = geometry[i].N-1;
		if(geometry[addToI].typeConnections[ni]>0) 
		{
			nAll += 1;
		}
	}
	if(!strcmp(geometry[addToI].Prop.symbol,"N")) nV++;

	for(i=0;i<10;i++)
		hAtoms[i] = g_malloc(sizeof(gchar)*100);
	nH = getHydrogensFromPDBTpl(geometry[addToI].Residue,geometry[addToI].pdbType, hAtoms);
	nH -= nOldH;

	if(nH<1)
	{
		for(i=0;i<10;i++)
			g_free(hAtoms[i]);
		return;
	}

	if(begin==0)
	{
		begin++;
		propH = prop_atom_get("H");
	}
	dist = (geometry[addToI].Prop.covalentRadii+propH.covalentRadii)*0.9;
	listOfConnectedAtoms = g_malloc(nV*sizeof(gint));

	if(Natoms>0)
	{
		geometry0 = g_realloc(geometry0,(Natoms+nH)*sizeof(GeomDef));
		geometry = g_realloc(geometry,(Natoms+nH)*sizeof(GeomDef)); 
	}
	else
	{
		geometry0 = g_malloc(nH*sizeof(GeomDef));
		geometry = g_malloc(nH*sizeof(GeomDef)); 
	}


	nC = 0;
	for(i=0;i<(gint)nA;i++)
	{
		gint ni = geometry[i].N-1;
		if(geometry[addToI].typeConnections[ni]>0)
		{
			listOfConnectedAtoms[nC] = i;
			nC++;
		}
	}

	for(i=0;i<nH;i++)
	{
		gdouble v1[3];
		gdouble v2[3];
		gdouble v3[3];
		gdouble v4[3];
		gdouble v5[3];
		for(k=0;k<3;k++)
			v1[k] = rand()/(gdouble)RAND_MAX-0.5;
		v3d_normal(v1);

		switch (nC)
		{
			case 1:
				{
					set_vect_ij(addToI, listOfConnectedAtoms[0], v2);
					v3d_normal(v2);
					v3d_cross(v1,v2,v3);
					v3d_normal(v3);
					v3d_scale(v2,cos(angle));
					v3d_scale(v3,sin(angle));
					v3d_add(v2,v3,v1);
					break;
				}
			case 2:
				{
					set_vect_ij(addToI, listOfConnectedAtoms[0], v2);
					v3d_normal(v2);
					set_vect_ij(addToI, listOfConnectedAtoms[1], v3);
					v3d_normal(v3);
					if(geom==0)
					{
						v3d_add(v2,v3,v4);
						v3d_normal(v4);
						v3d_scale(v4,-1.0);
						v3d_cross(v2,v3,v5);
						v3d_normal(v5);
						v3d_scale(v4,0.5);
						v3d_scale(v5,0.5);
						v3d_add(v4,v5,v1);
						v3d_normal(v1);
					}
					else
					{
						v3d_add(v2,v3,v1);
						v3d_normal(v1);
						v3d_scale(v1,-1.0);
					}
					break;
				}
			default:
				if(nC>=3)
				{
					set_vect_ij(addToI, listOfConnectedAtoms[nC-3], v2);
					v3d_normal(v2);
					set_vect_ij(addToI, listOfConnectedAtoms[nC-2], v3);
					v3d_normal(v3);
					set_vect_ij(addToI, listOfConnectedAtoms[nC-1], v4);
					v3d_normal(v4);
					v3d_add(v2,v3,v5);
					v3d_add(v5,v4,v1);
					v3d_normal(v1);
					v3d_scale(v1,-1.0);
					break;
				}

		}
		geometry[Natoms].X= geometry0[addToI].X + dist*v1[0];
		geometry[Natoms].Y= geometry0[addToI].Y + dist*v1[1];
		geometry[Natoms].Z= geometry0[addToI].Z + dist*v1[2];
		geometry[Natoms].Prop = prop_atom_get("H");
		geometry[Natoms].mmType = g_strdup(hAtoms[i]);
		geometry[Natoms].pdbType = g_strdup(hAtoms[i]);
		geometry[Natoms].Layer = geometry[addToI].Layer;
		geometry[Natoms].N = Natoms+1;
		geometry[Natoms].typeConnections = NULL;
		if(Natoms==0)
		{
			geometry[Natoms].Residue = g_strdup("H");
			geometry[Natoms].ResidueNumber = 0;
		}
		else
		{
			geometry[Natoms].Residue = g_strdup(geometry[addToI].Residue);
			geometry[Natoms].ResidueNumber = geometry[addToI].ResidueNumber;
		}
		geometry[Natoms].Charge = 0.0;
		geometry[Natoms].Variable = TRUE;
		geometry[Natoms].show = ShowHydrogenAtoms;

		geometry0[Natoms].X = geometry[Natoms].X;
		geometry0[Natoms].Y = geometry[Natoms].Y;
		geometry0[Natoms].Z = geometry[Natoms].Z;
		geometry0[Natoms].Prop = prop_atom_get("H");
		geometry0[Natoms].mmType = g_strdup(geometry[Natoms].mmType);
		geometry0[Natoms].pdbType = g_strdup(geometry[Natoms].pdbType);
		geometry0[Natoms].Layer = geometry[Natoms].Layer;
		geometry0[Natoms].Residue = g_strdup(geometry[Natoms].Residue);
		geometry0[Natoms].ResidueNumber = geometry[Natoms].ResidueNumber;
		geometry0[Natoms].Charge = 0.0;
		geometry0[Natoms].Variable = TRUE;
		geometry0[Natoms].N = geometry[Natoms].N;
		geometry0[Natoms].show = geometry[Natoms].show;
		geometry0[Natoms].typeConnections = NULL;

		Natoms++;
		
		nC++;
		listOfConnectedAtoms = g_realloc(listOfConnectedAtoms, nC*sizeof(gint));
		listOfConnectedAtoms[nC-1] = Natoms-1;
	}
	{
		gint i,j;
		for(i=0;i<(gint)Natoms-nH;i++)
		{
			geometry[i].typeConnections = g_realloc(geometry[i].typeConnections,Natoms*sizeof(gint));
			for(j=Natoms-nH;j<(gint)Natoms;j++) geometry[i].typeConnections[j] = 0;
			geometry0[i].typeConnections = g_realloc(geometry0[i].typeConnections,Natoms*sizeof(gint));
			for(j=Natoms-nH;j<(gint)Natoms;j++) geometry0[i].typeConnections[j] = 0;
		}
		for(i=(gint)Natoms-nH;i<(gint)Natoms;i++)
		{
			geometry[i].typeConnections = g_malloc(Natoms*sizeof(gint));
			for(j=0;j<(gint)Natoms;j++) geometry[i].typeConnections[j] = 0;
			geometry0[i].typeConnections = g_malloc(Natoms*sizeof(gint));
			for(j=0;j<(gint)Natoms;j++) geometry0[i].typeConnections[j] = 0;
		}
		for(i=(gint)Natoms-nH;i<(gint)Natoms;i++)
		{
			geometry[addToI].typeConnections[geometry[i].N-1] = 1;
			geometry0[addToI].typeConnections[geometry0[i].N-1] = 1;
			geometry[i].typeConnections[geometry[addToI].N-1] = 1;
			geometry0[i].typeConnections[geometry0[addToI].N-1] = 1;
		}
		adjust_multiple_bonds_with_one_atom(addToI);
	}
	if(listOfConnectedAtoms) g_free(listOfConnectedAtoms);
	for(i=0;i<10;i++)
		g_free(hAtoms[i]);
}
/********************************************************************************/
void addHydrogensTpl()
{
	gint i;
	gint k = 0; 
	gint nA = Natoms;


	if(Natoms<1) return;
	if(!NumFatoms) SelectAllAtoms();
	if(NFatoms<1) return;
	if(!NumFatoms) return;

	for (k=0;k<(gint)NFatoms;k++)
	for (i=0;i<(gint)nA;i++)
	{
		if(geometry[i].N== NumFatoms[k])
		{
			add_hydrogen_atoms_tpl(i,nA);
			break;
		}
	}
	setMMTypesChargesFromPDBTpl(2);
	create_GeomXYZ_from_draw_grometry();
	reset_charges_multiplicities();
	drawGeom();
}
/********************************************************************************/
void DeleteMolecule()
{
	gchar *t =N_("Do you want to really destroy this molecule?");
	if(Natoms>0)
		Continue_YesNo(delete_molecule, NULL,t);
	else
		Message(_("No molecule to delete\n"),_("Warning"),TRUE);
}
/********************************************************************************/
void SetOriginAtCenter(gpointer data, guint Operation,GtkWidget* wid)
{
	gdouble C[3];
	gint i;

	if(Natoms<1)
		return;

	for(i=0;i<3;i++)
		C[i] = 0.0; 

	for(i=0;i<(gint)Natoms;i++)
	{
		C[0] += geometry0[i].X;
		C[1] += geometry0[i].Y;
		C[2] += geometry0[i].Z;
	}
	for(i=0;i<3;i++)
		C[i] /= Natoms;

	for(i=0;i<(gint)Natoms;i++)
	{
		 geometry0[i].X -= C[0];
		 geometry0[i].Y -= C[1];
		 geometry0[i].Z -= C[2];
	}

	Ddef = FALSE;
	TransX = 0;
	TransY = 0;
	//reset_origine_molecule_drawgeom();
	for (i=0;i<3;i++) Orig[i] += C[i];
	drawGeom();
	set_statubar_pop_sel_atom();
	return;
}
/********************************************************************************/
void TraitementGeom(gpointer data, guint Operation,GtkWidget* wid)
{
  switch((GabEditGeomOperation)Operation)
  {
	case SAVEJPEG:
 		file_chooser_save(save_geometry_jpeg_file,_("Save image in jpeg file format"),GABEDIT_TYPEFILE_JPEG,GABEDIT_TYPEWIN_GEOM);
		break;
	case SAVEPPM:
 		file_chooser_save(save_geometry_ppm_file,_("Save image in ppm file format"),GABEDIT_TYPEFILE_PPM,GABEDIT_TYPEWIN_GEOM);
		break;
	case SAVEBMP:
 		file_chooser_save(save_geometry_bmp_file,_("Save image in bmp file format"),GABEDIT_TYPEFILE_BMP,GABEDIT_TYPEWIN_GEOM);
		break;
	case SAVEPS:
 		file_chooser_save(save_geometry_ps_file,_("Save image in ps file format"),GABEDIT_TYPEFILE_PS,GABEDIT_TYPEWIN_GEOM);
		break;
	default:
		printf("Operation = %d\n",Operation);
  }
}
/********************************************************************************/  
GdkPixmap *get_drawing_pixmap()
{
  return pixmap;
}
/********************************************************************************/  
cairo_t *get_drawing_cairo()
{
  return cr;
}
/********************************************************************************/  
GdkColormap* get_drawing_colormap()
{
  GdkColormap *colormap = gdk_drawable_get_colormap(GeomDrawingArea->window);

  return colormap;
}
/********************************************************************************/  
void read_geometries_convergence(gpointer data, guint Operation,GtkWidget* wid)
{
	switch(Operation)
	{
		case FGEOMCONVDALTON:
 			  	  file_chooser_open(read_geometries_conv_dalton,_("Load Geom. Conv. From Dalton Output file"),
				  GABEDIT_TYPEFILE_DALTON,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FGEOMCONVGAMESS:
 			  	  file_chooser_open(read_geometries_conv_dalton,_("Load Geom. Conv. From Gamess Output file"),
				  GABEDIT_TYPEFILE_GAMESS,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FGEOMCONVGAUSS:
 			  	  file_chooser_open(read_geometries_conv_gaussian,_("Load Geom. Conv. From Gaussian Output file"),
				  GABEDIT_TYPEFILE_GAUSSIAN,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FGEOMCONVMOLPRO:
 			  	  file_chooser_open(read_geometries_conv_molpro,_("Load Geom. Conv. From Molpro log file"),
				  GABEDIT_TYPEFILE_MOLPRO_LOG,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FGEOMCONVMOLDEN:
 			  	  file_chooser_open(read_geometries_conv_molden,_("Load Geom. Conv. From Molden file"),
				  GABEDIT_TYPEFILE_MOLDEN,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FGEOMCONVGABEDIT:
 			  	  file_chooser_open(read_geometries_conv_gabedit,_("Load Geom. Conv. From Gabedit file"),
				  GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FGEOMCONVMPQC:
 			  	  file_chooser_open(read_geometries_conv_mpqc,_("Load Geom. Conv. From MPQC output file"),
				  GABEDIT_TYPEFILE_MPQC,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FGEOMCONVXYZ:
 			  	  file_chooser_open(read_geometries_conv_xyz,_("Load Geom. Conv. From XYZ"),
				  GABEDIT_TYPEFILE_XYZ,GABEDIT_TYPEWIN_GEOM);
				  break;
		default : break;
	}
}
/********************************************************************************/  
void save_geometry(gpointer data, guint Operation,GtkWidget* wid)
{
	if(Natoms<1)
	{
		Message(_("Sorry,No molecule to save\n"),_("Warning"),TRUE);
		return;
	}
	switch(Operation)
	{
		case FXYZ 	: create_GeomXYZ_from_draw_grometry();
 			  	  file_chooser_save(save_geometry_xyz_file,_("Save geometry in xyz file"),
				  GABEDIT_TYPEFILE_XYZ,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FMOL2 	: create_GeomXYZ_from_draw_grometry();
 			  	  file_chooser_save(save_geometry_mol2_file,_("Save geometry in mol2 file"),
				  GABEDIT_TYPEFILE_MOL2,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FTINKER 	: create_GeomXYZ_from_draw_grometry();
 			  	  file_chooser_save(save_geometry_tinker_file,_("Save geometry in tinker file"),
				  GABEDIT_TYPEFILE_TINKER,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FPDB 	: create_GeomXYZ_from_draw_grometry();
 			  	  file_chooser_save(save_geometry_pdb_file,_("Save geometry in pdb file"),
				  GABEDIT_TYPEFILE_PDB,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FHIN 	: create_GeomXYZ_from_draw_grometry();
 			  	  file_chooser_save(save_geometry_hin_file,_("Save geometry in hyperchem file"),
				  GABEDIT_TYPEFILE_HIN,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FGABEDIT 	: create_GeomXYZ_from_draw_grometry();
 			  	  file_chooser_save(save_geometry_gabedit_file,_("Save geometry in gabedit file"),
				  GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_GEOM);
				  break;
		case FMZMAT 	: create_GeomXYZ_from_draw_grometry(); 
				  if(!xyz_to_zmat())
				  {
					Message(_("Sorry\nConversion is not possible from XYZ to Zmat"),_("Error"),TRUE);
					return;
				  }
 			  	  file_chooser_save(save_geometry_mzmatrix_file,_("Save geometry in mopac z-matrix file"),
				  GABEDIT_TYPEFILE_MZMAT,GABEDIT_TYPEWIN_GEOM);
				  create_GeomXYZ_from_draw_grometry(); 
				  MethodeGeom = GEOM_IS_XYZ;
				  break;
		case FGZMAT 	: create_GeomXYZ_from_draw_grometry(); 
				  if(!xyz_to_zmat())
				  {
					Message(_("Sorry\nConversion is not possible from XYZ to Zmat"),_("Error"),TRUE);
					return;
				  }
 			  	  file_chooser_save(save_geometry_gzmatrix_file,_("Save geometry in gaussian z-matrix file"),
				  GABEDIT_TYPEFILE_GZMAT,GABEDIT_TYPEWIN_GEOM);
				  create_GeomXYZ_from_draw_grometry(); 
				  MethodeGeom = GEOM_IS_XYZ;
				  break;
		default : break;
	}
}
/********************************************************************************/  
void read_geometry(gpointer data, guint Operation,GtkWidget* wid)
{
	switch(Operation)
	{
		case FXYZ 	:  MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_XYZ);break;
		case FGZMAT 	:  MethodeGeom = GEOM_IS_ZMAT;selc_ZMatrix_file();break;
		case FMZMAT 	:  MethodeGeom = GEOM_IS_ZMAT;selc_ZMatrix_mopac_file();break;
		case FMOL2 	:  MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOL2);break;
		case FTINKER 	: MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_TINKER);break;
		case FPDB 	: MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_PDB);break;
		case FHIN 	: MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_HIN);break;

		case FDALTONIN : break;
		case FDALTONFIRST: MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_DALTONFIRST);break;
		case FDALTONLAST : MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_DALTONLAST);break;

		case FGAUSSIN : selc_all_input_file(_("Read Geometry from a Gaussian input file"));break;
		case FGAUSSOUTFIRST: MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_GAUSSOUTFIRST);break;
		case FGAUSSOUTLAST : MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_GAUSSOUTLAST);break;

		case FMOLCASIN : selc_all_input_file(_("Read Geometry from a Molcas input file"));break;
		case FMOLCASOUTFIRST: MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOLCASOUTFIRST);break;
		case FMOLCASOUTLAST : MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOLCASOUTLAST);break;

		case FMOLPROIN : selc_all_input_file(_("Read Geometry from a Molpro input file"));break;
		case FMOLPROOUTFIRST: MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOLPROOUTFIRST);break;
		case FMOLPROOUTLAST : MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MOLPROOUTLAST);break;

		case FMPQCIN : selc_all_input_file(_("Read Geometry from a MPQC input file"));break;
		case FMPQCOUTFIRST: MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MPQCOUTFIRST);break;
		case FMPQCOUTLAST : MethodeGeom = GEOM_IS_XYZ;selc_XYZ_file(GABEDIT_TYPEFILEGEOM_MPQCOUTLAST);break;
	}
}
/********************************************************************************/  
guint label_option()
{
	return LabelOption;
}
/********************************************************************************/  
gboolean dipole_draw_mode()
{
	return DrawDipole;
}
/********************************************************************************/  
gboolean distances_draw_mode()
{
	return DrawDistance;
}
/********************************************************************************/  
gboolean dipole_mode()
{
	return ShowDipole;
}
/********************************************************************************/  
gboolean stick_mode()
{
	if(TypeGeom== GABEDIT_TYPEGEOM_STICK) return TRUE;
	return FALSE;
}
/********************************************************************************/  
gboolean space_fill_mode()
{
	if( TypeGeom == GABEDIT_TYPEGEOM_SPACE ) return TRUE;
	return FALSE;
}
/********************************************************************************/  
gboolean pers_mode()
{
	return PersMode;
}
/********************************************************************************/  
gboolean shad_mode()
{
	return ShadMode;
}
/********************************************************************************/  
gboolean light_mode()
{
	return LightMode;
}
/********************************************************************************/  
gboolean ortep_mode()
{
	return OrtepMode;
}
/********************************************************************************/  
gboolean cartoon_mode()
{
	return CartoonMode;
}
/********************************************************************************/  
gdouble get_frag_angle()
{
	return fragAngle;
} 
/********************************************************************************/  
void set_frag_angle(gdouble a)
{
	fragAngle = a;
} 
/********************************************************************************/  
void set_HBonds_dialog_geom(GtkWidget *win, guint data)
{
	set_HBonds_dialog (GeomDlg);
}
/********************************************************************************/  
void set_povray_options_geom(GtkWidget *win, guint data)
{
	 createPovrayOptionsWindow(GeomDlg);
}
/********************************************************************************/  
void HideShowMeasure(gboolean hiding)
{
	if(hiding)
	{
  		gtk_widget_hide(vboxhandle);
  		gtk_widget_hide(GeomDrawingArea);
  		gtk_widget_show(GeomDrawingArea);
    		while( gtk_events_pending() ) gtk_main_iteration();
	}
	else gtk_widget_show(vboxhandle);

   	MeasureIsHide=hiding;
}
/********************************************************************************/  
void AdjustHydrogensYesNo(gboolean adjust)
{
	AdjustHydrogenAtoms = adjust;
}
/********************************************************************************/  
gboolean getAdjustHydrogensYesNo()
{
	return AdjustHydrogenAtoms;
}
/********************************************************************************/  
void RebuildConnectionsDuringEditionYesNo(gboolean rebuild)
{
	RebuildConnectionsDuringEdition = rebuild;
}
/********************************************************************************/  
gboolean getRebuildConnectionsDuringEditionYesNo()
{
	return RebuildConnectionsDuringEdition;
}
/********************************************************************************/
static gint ScaleByMouse(gpointer data)
{
	GdkEventButton *bevent=(GdkEventButton *)data;

        switch(OperationType)
        {
	case SCALEGEOM :
			factor +=((bevent->y - BeginY) / GeomDrawingArea->allocation.height) * 5;
			if(factor<0.1) factor = 0.1;
			if(factor>10) factor = 10;
			drawGeom();
		break;
	case SCALESTICK :
			factorstick +=((bevent->y - BeginY) / GeomDrawingArea->allocation.height) * 5;
			if(factorstick <0.1) factorstick  = 0.1;
			if(factorstick >10) factorstick = 10;
			drawGeom();

		break;
	case SCALEBALL :
			factorball +=((bevent->y - BeginY) / GeomDrawingArea->allocation.height) * 5;
			if(factorball <0.1) factorball  = 0.1;
			if(factorball >10) factorball = 10;
			drawGeom();
		break;
	case SCALEDIPOLE :
			factordipole +=((bevent->y - BeginY) / GeomDrawingArea->allocation.height) * 5;
			if(factordipole <0.1) factordipole  = 0.1;
			if(factordipole >100) factordipole = 100;
			redefine_dipole();
			drawGeom();
		break;
	default : break;

        }
	BeginX = bevent->x;
	BeginY = bevent->y;
	

 	return TRUE;
}
/********************************************************************************/
static gint TranslationByMouse(GtkWidget *widget, GdkEventMotion *event)
{
	int x, y;
	GdkRectangle area;
	GdkModifierType state;

	if (event->is_hint)
	{
#if !defined(G_OS_WIN32)
		gdk_window_get_pointer(event->window, &x, &y, &state);
#else
		x = event->x;
		y = event->y;
		state = event->state;
#endif
	}
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}
  
	area.x = 0;
	area.y = 0;
	area.width  = widget->allocation.width;
	area.height = widget->allocation.height;

    TransX =(gint)(TransX+(x - BeginX)); 
    TransY =(gint)(TransY+(y - BeginY)); 
	drawGeom();

	BeginX = x;
	BeginY = y;
 return TRUE;
}
/********************************************************************************/
static gint RotationByMouse(GtkWidget *widget, GdkEventMotion *event)
{
	int x, y;
	GdkRectangle area;
	GdkModifierType state;
	gdouble spin_quat[4];

	if (event->is_hint)
	{
#if !defined(G_OS_WIN32)
		gdk_window_get_pointer(event->window, &x, &y, &state);
#else
		x = event->x;
		y = event->y;
		state = event->state;
#endif
	}
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}
  
	area.x = 0;
	area.y = 0;
	area.width  = widget->allocation.width;
	area.height = widget->allocation.height;

	
	trackball(spin_quat,
		(2.0*BeginX  - area.width) / area.width,
		(area.height - 2.0*BeginY) / area.height,
		(2.0*x       - area.width) / area.width,
		(area.height - 2.0*y     ) / area.height);
	add_quats(spin_quat, Quat, Quat);
	drawGeom();

	BeginX = x;
	BeginY = y;
 return TRUE;
}
/********************************************************************************/
static gint RotationZByMouse(GtkWidget *widget, GdkEventMotion *event)
{
	int x, y;
	GdkModifierType state;
	gdouble spin_quat[4] = {0,0,0,0};
	gdouble phi = 1.0/180*PI;
	gdouble width;
	gdouble height;
	gint Xi;
	gint Yi;

	if (event->is_hint)
	{
#if !defined(G_OS_WIN32)
		gdk_window_get_pointer(event->window, &x, &y, &state);
#else
		x = event->x;
		y = event->y;
		state = event->state;
#endif
	}
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}
  
	width  = widget->allocation.width;
	height = widget->allocation.height;

	Xi = width/2 + TransX;
	Yi = height/2 + TransY;

	
	if(abs(BeginX-x)>abs(BeginY-y))
	 {
		  gdouble sign  = 1.0;
		  if(BeginY> Yi && BeginX<x) sign = -1;
		  if(BeginY< Yi && BeginX>x) sign = -1;
		  phi = sign* fabs(BeginX-x)/width*PI;
	  }
	  else
	  {
		  gdouble sign = 1.0;
		  if(BeginX> Xi && BeginY>y) sign = -1;
		  if(BeginX< Xi && BeginY<y) sign = -1;
		  phi = sign* fabs(BeginY-y)/height*PI;
	  }
	spin_quat[2] = 1.0;

	spin_quat[2]= sin(phi/2);
	spin_quat[3] = cos(phi/2);

	add_quats(spin_quat, Quat, Quat);
	drawGeom();

	BeginX = x;
	BeginY = y;
	return TRUE;
}
/********************************************************************************/
static void rotation_fragment_quat(gdouble m[4][4],gdouble C[])
{
	gdouble A[3];
	gdouble B[3];
	guint i,j,k;
	gdouble M[4][4];
	gdouble O[3];

	build_rotmatrix(M,Quat);
	for (i=0;i<Natoms;i++)
	{
		A[0] = geometry[i].X;
		A[1] = geometry[i].Y;
		A[2] = geometry[i].Z;
		for(j=0;j<3;j++)
		{
			B[j] = 0.0;
			for(k=0;k<3;k++)
				B[j] += M[k][j]*A[k];
		}
		geometry[i].X=B[0];
		geometry[i].Y=B[1];
		geometry[i].Z=B[2];
	}
	for(i=0;i<3;i++) O[i] = Orig[i];
	for(j=0;j<3;j++)
	{
		B[j] = 0.0;
		for(k=0;k<3;k++)
			B[j] += M[k][j]*O[k];
	}
	for(i=0;i<3;i++) Orig[i] = B[i];

	for (i=0;i<Natoms;i++)
	{
		if(if_selected(i))
		{
			A[0] = geometry[i].X-C[0];
			A[1] = geometry[i].Y-C[1];
			A[2] = geometry[i].Z-C[2];
			for(j=0;j<3;j++)
			{
				B[j] = 0.0;
				for(k=0;k<3;k++)
					B[j] += m[k][j]*A[k];
			}
			geometry[i].X=C[0]+B[0];
			geometry[i].Y=C[1]+B[1];
			geometry[i].Z=C[2]+B[2];
		}

	}
	Ddef = FALSE;
	for (i=0;i<Natoms;i++)
	{
		geometry0[i].X=geometry[i].X;
		geometry0[i].Y=geometry[i].Y;
		geometry0[i].Z=geometry[i].Z;
	}
	init_quat(Quat);


	sort_with_zaxis();
	define_coefs_pers();
}
/********************************************************************************/
static gint local_zrotate_fragment(GtkWidget *widget, GdkEventMotion *event)
{
	int x, y;
	GdkModifierType state;
	gdouble spin_quat[4] = {0,0,0,0};
	gdouble m[4][4];
	gdouble C[3]={0,0,0};/* Center of Fragment */
	gint i;
	gint j;
	gint k;
	gushort Xmax;
	gushort Ymax;
	gushort Rmax;
	gdouble Cmax;
	gint Xi;
	gint Yi;
	gdouble width;
	gdouble height;
	gdouble phi = 1.0/180*PI;

	if(Natoms>0)
	{
		if(PersMode)
			Cmax  = coordmaxmin.Cmax*camera.f/(camera.position);
		else
			Cmax = coordmaxmin.Cmax;
	}
	else
		return FALSE;

	Xmax=GeomDrawingArea->allocation.width;
	Ymax=GeomDrawingArea->allocation.height;
	Rmax = Xmax;
	if(Rmax<Ymax)
		Rmax = Ymax;



	j=0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(if_selected(i))
		{
			j++;
			C[0] += geometry[i].X;
			C[1] += geometry[i].Y;
			C[2] += geometry[i].Z;
		}
	}
	if(j<1)
		return FALSE;
	for(k=0;k<3;k++)
		C[k] /= (gdouble)j;

	Xi = (gint)(C[0]/Cmax*factor*Rmax/2)+Xmax/2;
	Yi = (gint)(C[1]/Cmax*factor*Rmax/2)+Ymax/2;

	Xi = Xi + TransX;
	Yi = Yi + TransY;

	if (event->is_hint)
	{
#if !defined(G_OS_WIN32)
		gdk_window_get_pointer(event->window, &x, &y, &state);
#else
		x = event->x;
		y = event->y;
		state = event->state;
#endif
	}
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}
  
	width  = widget->allocation.width;
	height = widget->allocation.height;

	if(abs(BeginX-x)>abs(BeginY-y))
	 {
		  gdouble sign  = 1.0;
		  if(BeginY> Yi && BeginX<x) sign = -1;
		  if(BeginY< Yi && BeginX>x) sign = -1;
		  phi = sign* fabs(BeginX-x)/width*PI;
	  }
	  else
	  {
		  gdouble sign = 1.0;
		  if(BeginX> Xi && BeginY>y) sign = -1;
		  if(BeginX< Xi && BeginY<y) sign = -1;
		  phi = sign* fabs(BeginY-y)/height*PI;
	  }
	spin_quat[2] = 1.0;

	spin_quat[2]= sin(phi/2);
	spin_quat[3] = cos(phi/2);

	add_quats(spin_quat, QuatFrag, QuatFrag);
	build_rotmatrix(m,QuatFrag);
	rotation_fragment_quat(m,C);

	if(RebuildConnectionsDuringEdition)
		reset_connections_between_selected_and_notselected_atoms();
	/* reset_all_connections();*/
	drawGeom();

	init_quat(QuatFrag);
	BeginX = x;
	BeginY = y;
 return TRUE;
}
/********************************************************************************/
static gint local_rotate_fragment(GtkWidget *widget, GdkEventMotion *event)
{
	int x, y;
	GdkModifierType state;
	gdouble spin_quat[4];
	gdouble m[4][4];
	gdouble C[3]={0,0,0};/* Center of Fragment */
	gint i;
	gint j;
	gint k;
	gushort Xmax;
	gushort Ymax;
	gushort Rmax;
	gdouble Cmax;
	gint Xi;
	gint Yi;
	gdouble width;
	gdouble height;

	if(Natoms>0)
	{
		if(PersMode)
			Cmax  = coordmaxmin.Cmax*camera.f/(camera.position);
		else
			Cmax = coordmaxmin.Cmax;
	}
	else
		return FALSE;

	Xmax=GeomDrawingArea->allocation.width;
	Ymax=GeomDrawingArea->allocation.height;
	Rmax = Xmax;
	if(Rmax<Ymax)
		Rmax = Ymax;



	j=0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(if_selected(i))
		{
			j++;
			C[0] += geometry[i].X;
			C[1] += geometry[i].Y;
			C[2] += geometry[i].Z;
		}
	}
	if(j<1)
		return FALSE;
	for(k=0;k<3;k++)
		C[k] /= (gdouble)j;

	Xi = (gint)(C[0]/Cmax*factor*Rmax/2)+Xmax/2;
	Yi = (gint)(C[1]/Cmax*factor*Rmax/2)+Ymax/2;

	Xi = Xi + TransX;
	Yi = Yi + TransY;

	if (event->is_hint)
	{
#if !defined(G_OS_WIN32)
		gdk_window_get_pointer(event->window, &x, &y, &state);
#else
		x = event->x;
		y = event->y;
		state = event->state;
#endif
	}
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}
  
	width  = widget->allocation.width;
	height = widget->allocation.height;

	
	trackball(spin_quat,
		(2.0*(width/2+BeginX-Xi)  - (width)) / (width),
		((height) - 2.0*(height/2+BeginY-Yi)) / (height),
		(2.0*(width/2+x-Xi)       - (width)) / (width),
		((height) - 2.0*(height/2+y-Yi)     ) / (height));

	add_quats(spin_quat, QuatFrag, QuatFrag);
	build_rotmatrix(m,QuatFrag);
	rotation_fragment_quat(m,C);

	if(RebuildConnectionsDuringEdition)
		reset_connections_between_selected_and_notselected_atoms();
	/* reset_all_connections();*/
	drawGeom();

	init_quat(QuatFrag);
	BeginX = x;
	BeginY = y;
 return TRUE;
}
/********************************************************************************/
void set_statubar_pop_sel_atom()
{
	gchar* temp = NULL;
	if(NumSelectedAtom >=0 && OperationType == DELETEFRAG)
	{
		temp = g_strdup_printf(_("%s[%d] ; Coord (Ang) : %f %f %f ; Move your mouse to cancel the operation "),
			geometry0[NumSelectedAtom].Prop.symbol,NumSelectedAtom+1,
			geometry0[NumSelectedAtom].X*BOHR_TO_ANG,
			geometry0[NumSelectedAtom].Y*BOHR_TO_ANG,
			geometry0[NumSelectedAtom].Z*BOHR_TO_ANG);
	}
	if(OperationType == CUTBOND || OperationType == CHANGEBOND)
	{
		temp = g_strdup_printf(_("Move your mouse to cancel the operation "));
	}
	if(temp)
	{
		gtk_statusbar_pop(GTK_STATUSBAR(StatusOperation),idStatusOperation);
		gtk_statusbar_push(GTK_STATUSBAR(StatusOperation),idStatusOperation,temp);
		g_free(temp);
	}
}
/********************************************************************************/
static gint move_one_atom(GdkEventMotion *event)
{
	int x, y;
	GdkModifierType state;
	gdouble X;
	gdouble Y;
	gdouble Z;
	gdouble Cmax;
	gushort Xmax;
	gushort Ymax;
	gushort Rmax;

	if(NumSelectedAtom<0) return -1;
	Xmax=GeomDrawingArea->allocation.width;
	Ymax=GeomDrawingArea->allocation.height;
	Rmax = Xmax;
	if(Rmax<Ymax) Rmax = Ymax;

	if (event->is_hint)
	{
#if !defined(G_OS_WIN32)
		gdk_window_get_pointer(event->window, &x, &y, &state);
#else
		x = event->x;
		y = event->y;
		state = event->state;
#endif
	}
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}
  

	if(PersMode) Cmax  = coordmaxmin.Cmax*camera.f/(camera.position);
	else Cmax = coordmaxmin.Cmax;

	X = (gdouble)(x-Xmax/2-TransX)*2.0*Cmax/(factor*Rmax);
	Y = -(gdouble)(y-Ymax/2-TransY)*2.0*Cmax/(factor*Rmax);
	Z = geometry[NumSelectedAtom].Z;

	if(PersMode)
	{
		X = X/camera.f*(-Z+camera.position);
		Y = Y/camera.f*(-Z+camera.position);
	}
	{
		gdouble m[4][4];
		gdouble **m0 = g_malloc(3*sizeof(gdouble*));
		gdouble** minv;
		gint i,j;

		gdouble A[3];
		gdouble B[3];
		guint k;

		for(i=0;i<3;i++)
			m0[i] = g_malloc(3*sizeof(gdouble));

		build_rotmatrix(m,Quat);

		for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			m0[i][j] = m[i][j];

		minv = Inverse(m0,3,1e-7);

		A[0] = X;
		A[1] = Y;
		A[2] = Z;
		for(j=0;j<3;j++)
		{
			B[j] = 0.0;
			for(k=0;k<3;k++)
				B[j] += minv[k][j]*A[k];
		}
		X=B[0];
		Y=B[1];
		Z=B[2];
		i = NumSelectedAtom;
		geometry0[i].X=B[0];
		geometry0[i].Y=B[1];
		geometry0[i].Z=B[2];

		for(i=0;i<3;i++)
			if(minv[i])
				g_free(minv[i]);
		if(minv)
			g_free(minv);

		for(i=0;i<3;i++)
			if(m0[i])
				g_free(m0[i]);
		if(m0)
			g_free(m0);
	}
	Ddef = FALSE;
	buildRotation();

	drawGeom();
	set_statubar_pop_sel_atom();
	return TRUE;
}

/***********************************************************************************/
static gint move_all_selected_atoms(GtkWidget *widget, GdkEventMotion *event)
{
	int x, y;
	GdkModifierType state;
	gdouble X;
	gdouble Y;
	gdouble Z;
	gdouble Cmax;
	gushort Xmax;
	gushort Ymax;
	gushort Rmax;

	if(NumSelectedAtom<0) return -1;
	Xmax=GeomDrawingArea->allocation.width;
	Ymax=GeomDrawingArea->allocation.height;
	Rmax = Xmax;
	if(Rmax<Ymax) Rmax = Ymax;
	

	if (event->is_hint)
	{
#if !defined(G_OS_WIN32)
		gdk_window_get_pointer(event->window, &x, &y, &state);
#else
		x = event->x;
		y = event->y;
		state = event->state;
#endif
	}
	else
	{
		x = event->x;
		y = event->y;
		state = event->state;
	}
  
	if(PersMode)
		Cmax  = coordmaxmin.Cmax*camera.f/(camera.position);
	else
		Cmax = coordmaxmin.Cmax;

	X = (gdouble)(x-Xmax/2-TransX)*2.0*Cmax/(factor*Rmax);
	Y = -(gdouble)(y-Ymax/2-TransY)*2.0*Cmax/(factor*Rmax);
	Z = geometry[NumSelectedAtom].Z;

	if(PersMode)
	{
		X = X/camera.f*(-Z+camera.position);
		Y = Y/camera.f*(-Z+camera.position);
	}
	{
		gdouble m[4][4];
		gdouble **m0 = g_malloc(3*sizeof(gdouble*));
		gdouble** minv;
		gint i,j;

		gdouble A[3];
		gdouble B[3];
		guint k;

		for(i=0;i<3;i++)
			m0[i] = g_malloc(3*sizeof(gdouble));

		build_rotmatrix(m,Quat);

		for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			m0[i][j] = m[i][j];

		minv = Inverse(m0,3,1e-7);

		A[0] = X;
		A[1] = Y;
		A[2] = Z;
		for(j=0;j<3;j++)
		{
			B[j] = 0.0;
			for(k=0;k<3;k++)
				B[j] += minv[k][j]*A[k];
		}
		X=B[0];
		Y=B[1];
		Z=B[2];
		i = NumSelectedAtom;
		B[0] -=geometry0[i].X;
		B[1] -=geometry0[i].Y;
		B[2] -=geometry0[i].Z;

		for(i=0;i<(gint)Natoms;i++)
		for(j=0;j<(gint)NFatoms;j++)
			if(NumFatoms[j]==(gint)geometry0[i].N)
			{
				geometry0[i].X += B[0];
				geometry0[i].Y += B[1];
				geometry0[i].Z += B[2];
			}


		for(i=0;i<3;i++)
			if(minv[i])
				g_free(minv[i]);
		if(minv)
			g_free(minv);

		for(i=0;i<3;i++)
			if(m0[i])
				g_free(m0[i]);
		if(m0)
			g_free(m0);
	}
	Ddef = FALSE;
	if(RebuildConnectionsDuringEdition)
		reset_connections_between_selected_and_notselected_atoms();
	drawGeom();
	set_statubar_pop_sel_atom();
	return TRUE;
}
/********************************************************************************/
static gint MoveAtomByMouse(GtkWidget *widget, GdkEventMotion *event)
{
	gboolean MoveAll = FALSE;
	gint i;
	gint j;

	if(NumSelectedAtom<0) return FALSE;
	for(i=0;i<(gint)Natoms;i++)
		if((gint)i==NumSelectedAtom)
		{
			for(j = 0;j<(gint)NFatoms;j++)
				if(NumFatoms[j] == (gint)geometry[i].N)
					MoveAll = TRUE;
			break;
		}
	if(!MoveAll)
	{
		move_one_atom(event);
		if(RebuildConnectionsDuringEdition)
			reset_connection_with_one_atom(NumSelectedAtom);
		return TRUE;
	}
	move_all_selected_atoms(widget, event);
	return TRUE;
}
/*****************************************************************************/
gint set_proche_atom(GdkEventButton *bevent)
{
	gdouble xi,yi,xii,yii;
	gint i;
	gdouble mindist = -1;
	gdouble d1 ;

	xi = bevent->x;
	yi = bevent->y;

	NumProcheAtom = -1;
	for(i=Natoms-1;i>=0;i--)
	{
		xii = xi-geometry[i].Xi;
		yii = yi-geometry[i].Yi;
		d1 = xii*xii+yii*yii;
			if(mindist<0)
			{
				mindist = fabs(d1);
				NumProcheAtom = i;
			}
			if(mindist>fabs(d1))
			{
				mindist = fabs(d1);
				NumProcheAtom = i;
			}
	}
	return NumProcheAtom;
}
/*****************************************************************************/
gint set_selected_atoms(GdkEventButton *bevent)
{
	gdouble xi,yi,xii,yii;
	gint i;
	gdouble mindist = -1;
	gdouble d2 ;
	gdouble d1 ;
	gint ns = -1;

	xi = bevent->x;
	yi = bevent->y;

	for(i=Natoms-1;i>=0;i--)
	{
		gdouble rayon = 2*get_rayon(i);
		xii = xi-geometry[i].Xi;
		yii = yi-geometry[i].Yi;
		d1 = xii*xii+yii*yii;
		d2 = d1-rayon*rayon;
		if(d2<0)
		{
			if(mindist<0)
			{
				mindist = fabs(d1);
				ns = i;
			}
			if(mindist>fabs(d1))
			{
				mindist = fabs(d1);
				ns = i;
			}
		}
	}
	if(ns != -1)
	{
		gboolean Ok = FALSE;
		for(i=0;i<4;i++)
			if(NumSelAtoms[i] ==(gint) geometry[ns].N)
			{
				NumSelAtoms[i] = -1;
				Ok = TRUE;
				break;
			}
		if(!Ok)
		for(i=0;i<4;i++)
			if(NumSelAtoms[i] == -1 || NumSelAtoms[i] >(gint)Natoms)
			{
				NumSelAtoms[i] = geometry[ns].N;
				Ok = TRUE;
				break;
			}

		if(!Ok)
		{
			for(i=0;i<3;i++)
				NumSelAtoms[i] = NumSelAtoms[i+1]; 
			NumSelAtoms[3] = geometry[ns].N; 
		}
	}
	for(i=0;i<3;i++)
		if(NumSelAtoms[i] == -1)
		{
			NumSelAtoms[i] = NumSelAtoms[i+1];
			NumSelAtoms[i+1] = -1;
		}
	drawGeom();
	change_of_center(NULL,NULL);
	set_statubar_pop_sel_atom();
	return ns;
}
/*****************************************************************************/
gint unselected_atom(GdkEventButton *bevent)
{
	gdouble xi,yi,xii,yii;

	xi = bevent->x;
	yi = bevent->y;

	xii = xi-BeginX;
	yii = yi-BeginY;
	if(xii*xii+yii*yii > 8)
	{
		NumSelectedAtom = -1;
		drawGeom();
		SetOperation (NULL,OperationType);
	}
	return NumSelectedAtom;
}
/*****************************************************************************/
gint unselected_bond(GdkEventButton *bevent)
{
	gdouble xi,yi,xii,yii;

	xi = bevent->x;
	yi = bevent->y;

	xii = xi-BeginX;
	yii = yi-BeginY;
	if(xii*xii+yii*yii > 8)
	{
		NumBatoms[0] = NumBatoms[1] = -1;
		NBatoms = 0;
		drawGeom();
		SetOperation (NULL,OperationType);
	}
	return 0;
}
/*****************************************************************************/
gint set_selected_second_atom_bond(GdkEventButton *bevent)
{
	gdouble xi,yi,xa,ya;
	gint i;
	gdouble da ;
	gint nb=0;

	xi = bevent->x;
	yi = bevent->y;


	for(i=Natoms-1;i>=0;i--)
	{
		gdouble rayon = 2*get_rayon(i);
		xa = xi-geometry[i].Xi;
		ya = yi-geometry[i].Yi;
		da = xa*xa+ya*ya;
		if(geometry[i].N==geometry[NumSelectedAtom].N) continue;
		if(da<rayon*rayon)
		{
			nb = (gint) geometry[i].N;
			break;
		}
	}
	if(nb>0)
	{
		NBatoms = 2;
		NumBatoms[1] = nb;
	}
	else
	{
		NBatoms = 1;
	}
	drawGeom();
	change_of_center(NULL,NULL);
	set_statubar_pop_sel_atom();
	return NBatoms;
}
/*****************************************************************************/
gint set_selected_atom(GdkEventButton *bevent)
{
	gdouble xi,yi,xii,yii;
	gint i;
	gdouble mindist = -1;
	gdouble d2 ;
	gdouble d1 ;

	xi = bevent->x;
	yi = bevent->y;

	NumSelectedAtom = -1;
	for(i=Natoms-1;i>=0;i--)
	{
		gdouble rayon;
		if(!geometry[i].show) continue;
		rayon = 2*get_rayon(i);
		xii = xi-geometry[i].Xi;
		yii = yi-geometry[i].Yi;
		d1 = xii*xii+yii*yii;
		d2 = d1-rayon*rayon;
		if(d2<0)
		{
			if(mindist<0)
			{
				mindist = fabs(d1);
				NumSelectedAtom = i;
			}
			if(mindist>fabs(d1))
			{
				mindist = fabs(d1);
				NumSelectedAtom = i;
			}
		}
	}
	for(i=0;i<3;i++)
		QuatAtom[i] = 0;
	QuatAtom[3] = 1;
	if(NumSelectedAtom>=0)
	{
	CSselectedAtom[0] = geometry0[NumSelectedAtom].X;
	CSselectedAtom[1] = geometry0[NumSelectedAtom].Y;
	CSselectedAtom[2] = geometry0[NumSelectedAtom].Z;
	}
	drawGeom();
	set_statubar_pop_sel_atom();
	return NumSelectedAtom;
}
/*****************************************************************************/
gint set_selected_bond(GdkEventButton *bevent)
{
	gdouble xi,yi,xa,ya,xb,yb;
	gint i,j;
	gdouble da ;
	gdouble db ;
	gint na = -1;
	gint nb = -1;

	xi = bevent->x;
	yi = bevent->y;

	NumBatoms[0] = NumBatoms[1] = -1;
	NBatoms = 0;

	for(i=Natoms-1;i>=0;i--)
	{
		gdouble rayoni;
		xa = xi-geometry[i].Xi;
		ya = yi-geometry[i].Yi;
		da = xa*xa+ya*ya;
		rayoni = get_rayon(i)/2;
		rayoni = rayoni*rayoni;
		for(j=Natoms-1;j>=0;j--)
		{
			gdouble rayonj;
			gdouble minrayon;
			if(i==j)  continue;
			gint nj = geometry[j].N-1;
			if(geometry[i].typeConnections[nj]<1)  continue;
			xb = xi-geometry[j].Xi;
			yb = yi-geometry[j].Yi;
			db = xb*xb+yb*yb;
			rayonj = get_rayon(j)/2;
			rayonj = rayonj*rayonj;
			minrayon = rayoni;
			if(minrayon>rayonj) minrayon = rayonj;

			if( da<minrayon || db<minrayon) 
			{
				na = i;
				nb = j;
				break;
			}

			if( fabs((xa*xb+ya*yb)/sqrt(da*db)+1.0)<0.1)
			{
				na = i;
				nb = j;
				break;
			}

		}
		if(na>-1 && nb>-1) break;
	}
	if(na != -1 && nb != -1)
	{
		NBatoms = 2;
		NumBatoms[0] = (gint) geometry[na].N;
		NumBatoms[1] = (gint) geometry[nb].N;
	}
	drawGeom();
	change_of_center(NULL,NULL);
	set_statubar_pop_sel_atom();
	return NBatoms;
}
/*****************************************************************************/
gint set_selected_atom_bond(GdkEventButton *bevent)
{
	gdouble xi,yi,xa,ya;
	gint i;
	gdouble da ;

	xi = bevent->x;
	yi = bevent->y;

	NumBatoms[0] = NumBatoms[1] = -1;
	NBatoms = 0;

	for(i=Natoms-1;i>=0;i--)
	{
		gdouble rayon = 2*get_rayon(i);
		xa = xi-geometry[i].Xi;
		ya = yi-geometry[i].Yi;
		da = xa*xa+ya*ya;
		if(da<rayon*rayon)
		{
			NBatoms = 1;
			NumBatoms[0] = (gint) geometry[i].N;
			break;
		}
	}
	drawGeom();
	change_of_center(NULL,NULL);
	set_statubar_pop_sel_atom();
	return NBatoms;
}
/*****************************************************************************/
gint get_atom_to_select(GdkEventButton *bevent)
{
	gdouble xi,yi,xii,yii;
	gint i;
	gdouble d2 ;
	gdouble d1 ;

	xi = bevent->x;
	yi = bevent->y;

	for(i=Natoms-1;i>=0;i--)
	{
		gdouble rayon;
		if(!geometry[i].show) continue;
		if(i==NumSelectedAtom) continue;
		xii = xi-geometry[i].Xi;
		yii = yi-geometry[i].Yi;
		d1 = xii*xii+yii*yii;
		rayon = 2*get_rayon(i);
		d2 = d1-rayon*rayon;
		if(d2<0) return i;
	}
	return -1;
}
/*****************************************************************************/
gint set_selected_atom_or_bond_to_delete(GdkEventButton *bevent)
{
	NumSelectedAtom = -1;
	if(get_atom_to_select(bevent)>=0)
	{
		OperationType = DELETEFRAG;
		return set_selected_atom(bevent);
	}
	OperationType = CUTBOND;
	return set_selected_bond(bevent);
}
/*****************************************************************************/
gint add_begin_atoms_bond(GdkEventButton *bevent)
{
	gint ni,nj,i,j;
	if(NBatoms==1 && NumBatoms[0]>0)
	{
		insert_atom(bevent);
		j = Natoms-1;
		nj = geometry[j].N-1;
		for(i=0;i<(gint)Natoms;i++)
			if(geometry[i].N==NumBatoms[0])
			{ 
				ni = geometry[i].N-1;
				geometry0[j].typeConnections[ni] = 1;
				geometry0[i].typeConnections[nj] = 1;
				geometry[j].typeConnections[ni] = 1;
				geometry[i].typeConnections[nj] = 1;
				break; 
			}
		NumSelectedAtom = Natoms-1;
		return 1;
	}
	insert_atom(bevent);
	NumProcheAtom = Natoms-1;
	insert_atom(bevent);
	j = Natoms-1;
	i = Natoms-2;
	ni = geometry[i].N-1;
	nj = geometry[j].N-1;
	geometry0[j].typeConnections[ni] = 1;
	geometry0[i].typeConnections[nj] = 1;
	geometry[j].typeConnections[ni] = 1;
	geometry[i].typeConnections[nj] = 1;
	NumSelectedAtom = Natoms-1;
	NumBatoms[0] = -(geometry0[i].N+Natoms);
	return 2;
}
/*****************************************************************************/
gint set_selected_atom_or_bond_to_edit(GdkEventButton *bevent)
{
	gint res = -1;
	NumSelectedAtom = -1;
	res = get_atom_to_select(bevent);
	if(res==-1)
	{
		set_selected_bond(bevent);
		if(NBatoms==2) 
		{
			OperationType = CHANGEBOND;
			return NBatoms;
		}
	}
	set_selected_atom_bond(bevent);
	OperationType = ADDATOMSBOND;
	res = set_selected_atom(bevent);
	set_proche_atom(bevent);
	return res;
}
/*****************************************************************************/
static gint atom_noni_connected_to(gint i, gint k)
{
	gint j;
	gint l;
	if(Natoms<3) return -1;
	if(geometry[i].typeConnections)
	for(j=0;j<Natoms;j++)
	{
		gint nj = geometry[j].N-1;
		if(j==i) continue;
		if(geometry[j].typeConnections)
		for(l=0;l<Natoms;l++)
		{
			gint nl = geometry[l].N-1;
			if(k==geometry[l].N && ( geometry[l].typeConnections[nj]>0 || geometry[j].typeConnections[nl]>0)
					&& geometry[l].Prop.symbol[0] !='H') return geometry[j].N;
		}
	}
	for(j=0;j<Natoms;j++)
	{
		gint nj = geometry[j].N-1;
		if(j==i) continue;
		if(geometry[j].typeConnections)
		for(l=0;l<Natoms;l++)
			if(k==geometry[l].N && geometry[l].typeConnections && geometry[l].typeConnections[nj]>0) return geometry[j].N;
	}
	return -1;
}
/*****************************************************************************/
gint set_selected_atoms_for_insert_frag(GdkEventButton *bevent)
{
	gint nb = 0;
	gint i;
	gint j;

	NumSelectedAtom = -1;
	angleTo = -1;
	atomToBondTo = -1;
	NumSelectedAtom = -1;

	i = get_atom_to_select(bevent);
	if(i<0)  return i;
	atomToDelete = geometry[i].N;

	atomToBondTo = -1;
	for (j=0;j<(gint)Natoms;j++)
	if(geometry[j].typeConnections && geometry[j].typeConnections[atomToDelete-1]>0) 
	{
		nb++;
		atomToBondTo = geometry[j].N;
		angleTo = atom_noni_connected_to( i, atomToBondTo);
	}
	if(nb != 1 || atomToBondTo==-1) 
	{
		atomToDelete = -1;
		atomToBondTo = -1;
		angleTo = -1;
	}
	if(Frag.NAtoms>0 && Frag.atomToDelete != -1) drawGeom();
	return atomToDelete;
}
/*****************************************************************************
*  event_dispatcher
******************************************************************************/
gint button_press(GtkWidget *DrawingArea, GdkEvent *event, gpointer Menu)
{
	GdkEventButton *bevent;

	switch (event->type)
	{
		case GDK_BUTTON_PRESS:
		{
			buttonpress = TRUE;
			bevent = (GdkEventButton *) event;
			if (bevent->button == 3) /* Right Button ==> Popup Menu */
			{
				buttonpress = FALSE;
				popup_menu_geom( bevent->button, bevent->time);
			}
			else
			if (bevent->button == 1 && ControlKeyPressed)
			{
				
				BeginX= bevent->x;
				BeginY = bevent->y;
				return TRUE;
			}
			else
			if (bevent->button == 1)
			{
				/* beginning of drag, reset mouse position */
				BeginX= bevent->x;
				BeginY = bevent->y;
        			switch(OperationType)
        			{
					case SELECTOBJECTS :
						if(GKeyPressed && select_atoms_by_groupe()) 
							SetOperation(NULL,SELECTRESIDUE); 
						else
						if(!FKeyPressed && select_atoms_by_residues()) 
							SetOperation(NULL,SELECTRESIDUE); 
						else
							SetOperation(NULL,SELECTFRAG); 
						break;
					case MOVEFRAG : 
						add_geometry_to_fifo();
						set_selected_atom(bevent); break;
					case DELETEOBJECTS : 
						add_geometry_to_fifo();
						set_selected_atom_or_bond_to_delete(bevent);break;
					case MEASURE     : set_selected_atoms(bevent);break;
					case EDITOBJECTS : 
							  add_geometry_to_fifo();
							  set_selected_atom_or_bond_to_edit(bevent); 
							  if(NBatoms<2) add_begin_atoms_bond(bevent);
							  drawGeom();
							  break;
					case ADDFRAGMENT : 
							  add_geometry_to_fifo();
							  set_selected_atoms_for_insert_frag(bevent);
							  set_proche_atom(bevent);break;
					case ROTLOCFRAG : 
							  add_geometry_to_fifo();
							  init_quat(QuatFrag);
							  ButtonPressed = TRUE;
							  drawGeom();
							  break;
					case ROTZLOCFRAG : 
							  add_geometry_to_fifo();
							  init_quat(QuatFrag);
							  ButtonPressed = TRUE;
							  drawGeom();
							  break;
					default:break;
				}
			return TRUE;
			}
			else
			if (bevent->button == 2)
			{
				BeginX= bevent->x;
				BeginY = bevent->y;
				return TRUE;
			}
		}
		default: break;
	}
	return FALSE;
}
/*************************/
void unselect_all_atoms()
{
	NFatoms = 0;
	if(NumFatoms)
	       g_free(NumFatoms);
	NumFatoms = NULL;
}
/*****************************************************************************
*  event_release
******************************************************************************/
gint button_release(GtkWidget *DrawingArea, GdkEvent *event, gpointer Menu)
{
	GdkEventButton *bevent;
	buttonpress = FALSE;
	if(event->type == GDK_BUTTON_RELEASE)
	{
		bevent = (GdkEventButton *) event;
		if (bevent->button == 3) return TRUE;
		if (bevent->button == 2) { drawGeom(); return TRUE;}
		if (bevent->button == 1 && ControlKeyPressed) return TRUE;
	}
	if(NumSelectedAtom !=-1)
	{
		switch(OperationType)
		{
		case MOVEFRAG :
			create_GeomXYZ_from_draw_grometry();
			NumSelectedAtom = -1;
			if(GeomIsOpen)
				unselect_all_atoms();

			free_text_to_draw();
			drawGeom();
			SetOperation (NULL,MOVEFRAG);
			change_of_center(NULL,NULL);
			break;
		case DELETEFRAG :
			delete_selected_atoms();
			create_GeomXYZ_from_draw_grometry();
			NumSelectedAtom = -1;
			free_text_to_draw();
			drawGeom();
			SetOperation (NULL,DELETEOBJECTS);
			change_of_center(NULL,NULL);
			break;
		default:break;

		}
	}
	switch(OperationType)
	{
	case SELECTRESIDUE : SetOperation(NULL,SELECTOBJECTS);  break;
	case SELECTFRAG : SetOperation(NULL,SELECTOBJECTS);  break;
	case ADDATOMSBOND :
		if(NBatoms==2 && NumBatoms[0]>0 &&  NumBatoms[1]>0 && NumBatoms[0]!=NumBatoms[1])
		{
			delete_one_atom(NumSelectedAtom);
			add_bond();
		}
		else if(NBatoms==2 && NumBatoms[0]<-Natoms &&  NumBatoms[1]>0)
		{
			NumBatoms[0] = -NumBatoms[0]-Natoms;
			delete_one_atom(NumSelectedAtom);
			if( NumBatoms[0] != NumBatoms[1]) add_bond();
		}
		else if(NumBatoms[0]>0)
		{
			gint res = get_atom_to_select((GdkEventButton *)event);
			if(res != -1)
			{
				delete_one_atom(NumSelectedAtom);
				replace_atom(get_indice(NumBatoms[0]));
				if(AdjustHydrogenAtoms) adjust_hydrogens_connected_to_atom(get_indice(NumBatoms[0]));
			}
			else if(AdjustHydrogenAtoms) 
			{
				adjust_hydrogens_connected_to_atoms(NumSelectedAtom,get_indice(NumBatoms[0]));
			}
		}
		else if(NumBatoms[0]<-Natoms)
		{
			gint res = -1;
			NumBatoms[0] = -NumBatoms[0]-Natoms;
			res = get_atom_to_select((GdkEventButton *)event);
			if(res != -1)
			{
				delete_one_atom(NumSelectedAtom);
				if(AdjustHydrogenAtoms) adjust_hydrogens_connected_to_atom(get_indice(NumBatoms[0]));
			}
			else if(AdjustHydrogenAtoms) 
			{
				adjust_hydrogens_connected_to_atoms(NumSelectedAtom,get_indice(NumBatoms[0]));
			}
		}
		create_GeomXYZ_from_draw_grometry();
		reset_charges_multiplicities();
		SetOperation (NULL,EDITOBJECTS);
		change_of_center(NULL,NULL);
		NumProcheAtom = -1;
		NumSelectedAtom = -1;
		free_text_to_draw();
		NBatoms = 0;
		NumBatoms[0] = NumBatoms[1] = -1;
		drawGeom();
		break;
	case ADDFRAGMENT :
		insert_fragment(DrawingArea,event);
		create_GeomXYZ_from_draw_grometry();
		NumProcheAtom = -1;
		atomToDelete = -1;
		atomToBondTo = -1;
		angleTo = -1;
		free_text_to_draw();
		drawGeom();
		/*activate_rotation();*/
		SetOperation (NULL,ADDFRAGMENT);
		change_of_center(NULL,NULL);
		break;
	case ROTLOCFRAG :
		ButtonPressed = FALSE;
		create_GeomXYZ_from_draw_grometry();
		drawGeom();
		change_of_center(NULL,NULL);
		break;
	case ROTZLOCFRAG :
		ButtonPressed = FALSE;
		create_GeomXYZ_from_draw_grometry();
		drawGeom();
		change_of_center(NULL,NULL);
		break;
	case CUTBOND :
		delete_selected_bond();
		free_text_to_draw();
		create_GeomXYZ_from_draw_grometry();
		reset_charges_multiplicities();
		drawGeom();
		SetOperation (NULL,DELETEOBJECTS);
		break;
	case CHANGEBOND :
		change_selected_bond();
		free_text_to_draw();
		create_GeomXYZ_from_draw_grometry();
		reset_charges_multiplicities();
		drawGeom();
		SetOperation (NULL,EDITOBJECTS);
		break;

	default:
		drawGeom();
	}

	return TRUE;

}
/*****************************************************************************
*  event_dispatcher
******************************************************************************/
gint event_dispatcher(GtkWidget *DrawingArea, GdkEvent *event, gpointer Menu)
{
	return button_press(DrawingArea,event,Menu);

}
/********************************************************************************/
/* Moption Notify */
/********************************************************************************/
gint motion_notify(GtkWidget *widget, GdkEventMotion *event)
{
	GdkModifierType state;

	if (event->is_hint)
	{
#if !defined(G_OS_WIN32)
		int x, y;
		gdk_window_get_pointer(event->window, &x, &y, &state);
#else
		state = event->state;
#endif

	}
	else
		state = event->state;
	if (state & GDK_BUTTON1_MASK)
	{
		if(ControlKeyPressed)
		{
			RotationByMouse(widget,event);
			return TRUE;
		}
	}
	if (state & GDK_BUTTON1_MASK)
	{
		switch(OperationType)
		{
			case ROTATION 	: RotationByMouse(widget,event);break;
			case ROTATIONZ 	: RotationZByMouse(widget,event);break;
			case TRANSMOVIE : TranslationByMouse(widget,event);break;
			case SCALEGEOM 	: 
			case SCALESTICK	:
			case SCALEBALL 	: 
			case SCALEDIPOLE: ScaleByMouse((gpointer)event);break;
			case SELECTFRAG : 
					  switch(SelectType)
					  {
						case CIRCLE :
							draw_selection_circle(event->x,event->y);
							break;
						case RECTANGLE:
							draw_selection_rectangle(event->x,event->y);
							select_atoms_by_rectangle(event->x,event->y);
							break;
						default:break;
					  }
					  break;
			case MOVEFRAG   : 
					  MoveAtomByMouse(widget,event);
					  free_text_to_draw();
					  change_of_center(NULL,NULL);
					  break;
			case ROTLOCFRAG :
					local_rotate_fragment(widget,event);
					free_text_to_draw();
					change_of_center(NULL,NULL);
					break;
			case ROTZLOCFRAG :
					local_zrotate_fragment(widget,event);
					free_text_to_draw();
					change_of_center(NULL,NULL);
					break;
			case DELETEFRAG : 
					  if(unselected_atom((GdkEventButton *)event)==-1)
					  {
					  	OperationType = DELETEOBJECTS;
					  	unselected_atom((GdkEventButton *)event);
					  }
					  free_text_to_draw();
					  change_of_center(NULL,NULL);
					  break;
			case CUTBOND : 
					  OperationType = DELETEOBJECTS;
					  unselected_bond((GdkEventButton *)event);
					  free_text_to_draw();
					  drawGeom();
					  break;
			case CHANGEBOND : unselected_bond((GdkEventButton *)event);
					  free_text_to_draw();
					  break;
			case ADDATOMSBOND : 
					  move_one_atom(event);
					  set_selected_second_atom_bond((GdkEventButton *)event);
					  free_text_to_draw();
					  break;
			case ADDFRAGMENT : 
					  if(atomToDelete>-1)
					  {
						gint j = get_atom_to_select((GdkEventButton *)event);
						if(j>=0 && geometry[j].N != atomToDelete && geometry[j].N != atomToBondTo && 
							fabs(atof(get_angle(atomToDelete,atomToBondTo,geometry[j].N))-180)>0.1)
						{
							angleTo = geometry[j].N;
							drawGeom();
						}
					  }
            default : return FALSE;
		}
	}

	if (state & GDK_BUTTON2_MASK)
	{
		RotationByMouse(widget,event);
	}
	return TRUE;
}
/********************************************************************************/
static void redraw()
{
  gdk_draw_drawable(GeomDrawingArea->window,
                  GeomDrawingArea->style->fg_gc[GTK_WIDGET_STATE (GeomDrawingArea)],
                  pixmap,
                  0,0,
                  0,0,
                  GeomDrawingArea->allocation.width,
                  GeomDrawingArea->allocation.height);    
}
/********************************************************************************/
static void pixmap_init(GtkWidget *widget)
{
  GdkColormap *colormap;

  if(!BackColor)
  gdk_draw_rectangle (pixmap,
                      widget->style->black_gc,
                      TRUE,
                      0, 0,
                      widget->allocation.width,
                      widget->allocation.height);    
  else
  {
   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap,BackColor,FALSE,TRUE);
	gdk_gc_set_foreground(gc,BackColor);

        gdk_draw_rectangle (pixmap,
                      gc,
                      TRUE,
                      0, 0,
                      widget->allocation.width,
                      widget->allocation.height);    
  }
}
/*****************************************************************************/
static gint configure_event( GtkWidget *widget, GdkEventConfigure *event )
{
	if(!gc) gc = gdk_gc_new(GeomDrawingArea->window);
	if (pixmap) g_object_unref(pixmap);
	pixmap = gdk_pixmap_new(widget->window, widget->allocation.width, widget->allocation.height, -1);
	cr = gdk_cairo_create (pixmap);
	drawGeom();

	return TRUE;
}
/********************************************************************************/   
static gint expose_event( GtkWidget *widget, GdkEventExpose *event )
{
	if(event->count >0) return FALSE;
	gdk_draw_drawable(widget->window,
                  widget->style->fg_gc[GTK_WIDGET_STATE (widget)],
                  pixmap,
                  event->area.x, event->area.y,
                  event->area.x, event->area.y,
                  event->area.width, event->area.height);
 
	return FALSE;
}                                                                               
/*****************************************************************************/
void SetCosSin()
{
  int i;
 
  for ( i=0; i < 91; i++ ) {
        TSIN[i] = sin((PI*i)/180.0);
        TCOS[i] = cos((PI*i)/180.0);
  }
}
/*****************************************************************************/ 
void RenderStick()
{
	TypeGeom = GABEDIT_TYPEGEOM_STICK;
	drawGeom();
}
/*****************************************************************************/
void RenderBallStick()
{
	TypeGeom =  GABEDIT_TYPEGEOM_BALLSTICK;
	drawGeom();
}
/*****************************************************************************/
void RenderSpaceFill()
{
	TypeGeom = GABEDIT_TYPEGEOM_SPACE;
	drawGeom();
}
/*****************************************************************************/
void ActivateButtonOperation (GtkWidget *widget, guint data)
{
	SetOperation (widget,data);
}
/*****************************************************************************/
void SetOperation (GtkWidget *widget, guint data)
{
	gchar* temp = NULL;
	if(data == CENTER)
	{
			TransX = 0;
			TransY = 0;
			drawGeom();
		return;
	}

	OperationType = data ;
	if( OperationType != ADDFRAGMENT)
		hide_fragments_selector();
	switch(data)
	{
		case ROTATION	: temp = g_strdup(_(" Press the Left mouse button and move your mouse for a \"Rotate molecule\". "));break;
		case ROTATIONZ	: temp = g_strdup(_(" Press the Left mouse button and move your mouse for a \"Rotate molecule about z axis\". "));break;
		case TRANSMOVIE : temp = g_strdup(_(" Press the Left mouse button and move your mouse for a \"Translation\". "));break;
		case SCALEGEOM	: temp = g_strdup(_(" Press the Left mouse button and move your mouse for a \"Zoom\". "));break;
		case SCALESTICK : temp = g_strdup(_(" Press the Left mouse button and move your mouse for \"Scale Stick\". "));break;
		case SCALEBALL  :  temp = g_strdup(_(" Press the Left mouse button and move your mouse for \"Scale Ball\". "));break;
		case SCALEDIPOLE:  temp = g_strdup(_(" Press the Left mouse button and move your mouse for \"Scale Dipole\". "));break;
		case SELECTOBJECTS :  temp = g_strdup(_("Pick an atom to select a residue, G key + pick an atom to select a group, Or F key + move your mouse to select a fragment. Use shift key for more selections."));break;
		case SELECTFRAG :  temp = g_strdup(_(" Press the Left mouse button and move your mouse for \"select a fragment\".Use shift key for more selections. "));break;
		case SELECTRESIDUE :  temp = g_strdup(_("  Press the Left mouse button for pick an atom, all atoms for residue of this atom are selected(or unselected)."));break;
		case DELETEFRAG :  temp = g_strdup(_(" Press the Left mouse button(for pick an atom or all selected atoms) and release for \"Delete selected atom(s)\". "));break;
		case ROTLOCFRAG :  temp = g_strdup(_(" Press the Left mouse button and move your mouse for \"Rotatation of selected atom(s)[Local Rotation]\". "));break;
		case ROTZLOCFRAG :  temp = g_strdup(_(" Press the Left mouse button and move your mouse for \"Rotation, about z axis, of selected atom(s)[Local Rotation]\". "));break;
		case MOVEFRAG   :  temp = g_strdup(_(" Press the Left mouse button(for pick an atom or all selected atoms) and move your mouse for \"Move selected atom(s)\". "));break;
		case EDITOBJECTS :  temp = g_strdup(_(" Press and release the Left mouse button for \"Insert/Change atom(s)/bond\".\"Pick an atom for replace it.\""));break;
		case ADDATOMSBOND :  temp = g_strdup(_(" Press and release the Left mouse button for \"Insert atom(s)/bond\".\"Pick an atom for replace it.\""));break;
		case CHANGEBOND :  temp = g_strdup(_(" Press the Left mouse button(for pick a bond) and release for \"Change selected bond\". "));break;
		case CUTBOND :  temp = g_strdup(_(" Press the Left mouse button(for pick a bond) and release for \"Delete selected bond\". "));break;
		case MEASURE		:  temp = g_strdup(_(" Press and release the Left mouse button for \"Select your atoms\". "));
							HideShowMeasure(FALSE);
							change_of_center(NULL,NULL);
							drawGeom();
							break;
		case ADDFRAGMENT :  temp = g_strdup(_(" Press and release the Left mouse button for \"Insert a Fragment\". "));break;
	}
	if(temp)
	{
		gtk_statusbar_pop(GTK_STATUSBAR(StatusOperation),idStatusOperation);
		gtk_statusbar_push(GTK_STATUSBAR(StatusOperation),idStatusOperation,temp);
		g_free(temp);
	}
	drawGeom();
}
/*****************************************************************************/
void setPersonalFragment(Fragment F)
{
	gint i;
	Frag.NAtoms = F.NAtoms;
	Frag.Atoms = g_malloc(Frag.NAtoms*sizeof(Atom));
	for(i=0;i<F.NAtoms;i++)
	{
		Frag.Atoms[i].Residue = g_strdup(F.Atoms[i].Residue);
		Frag.Atoms[i].Symb = g_strdup(F.Atoms[i].Symb);
		Frag.Atoms[i].mmType = g_strdup(F.Atoms[i].mmType);
		Frag.Atoms[i].pdbType = g_strdup(F.Atoms[i].pdbType);
		Frag.Atoms[i].Coord[0] = F.Atoms[i].Coord[0];
		Frag.Atoms[i].Coord[1] = F.Atoms[i].Coord[1];
		Frag.Atoms[i].Coord[2] = F.Atoms[i].Coord[2];
		Frag.Atoms[i].Charge = F.Atoms[i].Charge;
	}
	Frag.atomToDelete = F.atomToDelete; 
	Frag.atomToBondTo = F.atomToBondTo;
	Frag.angleAtom    = F.angleAtom;

	SetOperation (NULL,ADDFRAGMENT);
}
/*****************************************************************************/
void AddFragment(GtkWidget *widget, guint data)
{
	FreeFragment(&Frag);
	Frag = GetFragment(FragItems[data].Name);
	SetOperation (NULL,ADDFRAGMENT);
}
/*****************************************************************************/
void add_a_fragment(GtkWidget* button, gchar* fragName)
{
	GtkWidget* drawingArea = NULL;
	gchar* slash = NULL;
	if(!fragName) return;
	FreeFragment(&Frag);
	slash = strstr(fragName,"/");
	if(slash && strlen(fragName)>strlen(slash))
	{
		Frag = GetFragment(slash+1);
		if(Frag.NAtoms>1 && strstr(fragName,"Fullerenes")) 
		{
			AddHToAtomPDB(&Frag,"C");
		}
		if(Frag.NAtoms<1)
		{
			gint OC1 = -1;
			gint C = -1;
			gint N = -1;
			gint i ;

			Frag = GetFragmentPPD(slash+1);
			for(i=0;i<Frag.NAtoms;i++)
			{
				if(!strcmp(Frag.Atoms[i].pdbType,"OC1")) OC1 =  i;
				if(!strcmp(Frag.Atoms[i].pdbType,"C")) C =  i;
				if(!strcmp(Frag.Atoms[i].pdbType,"N")) N =  i;
			}
			/* printf("%s\n",Frag.Atoms[0].Residue);*/
			if(Frag.NAtoms>1)
			{
				if(C != -1 && N == -1) AddHToAtomPDB(&Frag,"C");
				else if(C == -1 && N != -1) AddHToAtomPDB(&Frag,"N");
				else if(strlen(Frag.Atoms[0].Residue)<4) AddHToAtomPDB(&Frag,"C");
				else if(strlen(Frag.Atoms[0].Residue)==4) 
				{
					if( toupper(Frag.Atoms[0].Residue[0])=='C' || toupper(Frag.Atoms[0].Residue[0])=='O' ) AddHToAtomPDB(&Frag,"N");
					else AddHToAtomPDB(&Frag,"C");
				}
			}
		}
		if(Frag.NAtoms<1)
			addPersonalFragment(fragName, 0, NULL);
	}
	else
		Frag = GetFragment(fragName);

	drawingArea = g_object_get_data(G_OBJECT(button), "DrawingArea");
	if(drawingArea) add_frag_to_preview_geom(drawingArea, &Frag);
}
/*****************************************************************************/
void addAFragment(gchar* fragName)
{
	FreeFragment(&Frag);
	if(fragName) Frag = GetFragment(fragName);
	else return;
	SetOperation (NULL,ADDFRAGMENT);
}
/*****************************************************************************/
void initLabelOptions (guint data)
{
	LabelOption = data ;
}
/*****************************************************************************/
void SetLabelOptions (GtkWidget *widget, guint data)
{
	if(LabelOption != data)
	{
		LabelOption = data ;
		drawGeom();
	}
}
/*****************************************************************************/
void SetLabelDistances(GtkWidget *win,gboolean YesNo)
{
	DrawDistance = !DrawDistance;
	drawGeom();
}
/*****************************************************************************/
void SetLabelDipole(GtkWidget *win,gboolean YesNo)
{
	DrawDipole = !DrawDipole;
	drawGeom();
}
/*****************************************************************************/
void RenderShad(GtkWidget *win,gboolean YesNo)
{
	ShadMode = !ShadMode;
	drawGeom();
}
/*****************************************************************************/
void RenderPers(GtkWidget *win,gboolean YesNo)
{
	PersMode = !PersMode;
	if(PersMode)
		define_coefs_pers();
	drawGeom();
}
/*****************************************************************************/
void RenderLight(GtkWidget *win,gboolean YesNo)
{
	LightMode = !LightMode;
	drawGeom();
}
/*****************************************************************************/
void RenderOrtep(GtkWidget *win,gboolean YesNo)
{
	OrtepMode = !OrtepMode;
	drawGeom();
}
/*****************************************************************************/
void RenderCartoon(GtkWidget *win,gboolean YesNo)
{
	CartoonMode = !CartoonMode;
	drawGeom();
}
/*****************************************************************************/
void RenderHBonds(GtkWidget *win,gboolean YesNo)
{
	ShowHBonds = !ShowHBonds;
	if(ShowHBonds) set_Hconnections();
	drawGeom();
}
/*****************************************************************************/
void RenderHAtoms(GtkWidget *win,gboolean YesNo)
{
	gint i;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!strcmp(geometry0[i].Prop.symbol,"H"))
		{
			geometry[i].show = YesNo;
			geometry0[i].show = YesNo;
		}
	}
	drawGeom();
	ShowHydrogenAtoms = YesNo;
}
/*****************************************************************************/
void RenderDipole(GtkWidget *win,gboolean YesNo)
{
	ShowDipole = !ShowDipole;
	drawGeom();
}
/*****************************************************************************/
void set_dipole_from_charges()
{
	gint i;
	gint j;

	create_GeomXYZ_from_draw_grometry();
	NumSelectedAtom = -1;
	unselect_all_atoms();
	Dipole.def = TRUE;
	for(i=0;i<3;i++)
		Dipole.value[i] = 0.0;
	for(j=0;j<(gint)Natoms;j++)
	{
		Dipole.value[0] += geometry0[j].X*geometry0[j].Charge;
		Dipole.value[1] += geometry0[j].Y*geometry0[j].Charge;
		Dipole.value[2] += geometry0[j].Z*geometry0[j].Charge;

	}	
	define_geometry();
	drawGeom();
}
/*****************************************************************************/
gdouble get_sum_charges()
{
	gdouble c = 0;
	gint j;

	for(j=0;j<(gint)Natoms;j++)
		c += geometry0[j].Charge;
	return c;
}
/*****************************************************************************/
void compute_total_charge()
{
	gdouble c = 0;
	gdouble cNeg = 0;
	gdouble cPos = 0;
	gint j;
    	GtkWidget* m;
	gchar tmp[BSIZE];

	for(j=0;j<(gint)Natoms;j++)
	{
		if(geometry0[j].Charge>0) cPos += geometry0[j].Charge;
		if(geometry0[j].Charge<0) cNeg += geometry0[j].Charge;
		c += geometry0[j].Charge;

	}	
	if(cNeg !=0 && cPos != 0)
	sprintf(tmp,
			_(
			"Total Charge = %f\n"
			"Sum of positive charges = %f\n"
			"Sum of negative charges = %f\n"
			"positive/negative       = %f\n"
			"negative/positive       = %f\n"
			)
			,
			c, cPos, cNeg, cPos/cNeg, cNeg/cPos);
	else
	sprintf(tmp,
			_(
			"Total Charge = %f\n"
			"Sum of positive charges = %f\n"
			"Sum of negative charges = %f\n"
			)
			,
			c, cPos, cNeg);
	m = Message(tmp,_("Info"),TRUE);
	gtk_window_set_modal (GTK_WINDOW (m), TRUE);
}
/*****************************************************************************/
void compute_charge_of_selected_atoms()
{
	gdouble c = 0;
	gdouble cNeg = 0;
	gdouble cPos = 0;
	gint j;
    	GtkWidget* m;
	gchar tmp[BSIZE];

	for(j=0;j<(gint)Natoms;j++)
	{
		if(!if_selected(j)) continue;

		if(geometry0[j].Charge>0) cPos += geometry0[j].Charge;
		if(geometry0[j].Charge<0) cNeg += geometry0[j].Charge;
		c += geometry0[j].Charge;

	}	
	if(cNeg !=0 && cPos != 0)
	sprintf(tmp,
			_(
			"Total Charge = %f\n"
			"Sum of positive charges = %f\n"
			"Sum of negative charges = %f\n"
			"positive/negative       = %f\n"
			"negative/positive       = %f\n"
			)
			,
			c, cPos, cNeg, cPos/cNeg, cNeg/cPos);
	else
	sprintf(tmp,
			_(
			"Total Charge = %f\n"
			"Sum of positive charges = %f\n"
			"Sum of negative charges = %f\n"
			)
			,
			c, cPos, cNeg);
	m = Message(tmp,_("Info"),TRUE);
	gtk_window_set_modal (GTK_WINDOW (m), TRUE);
}
/********************************************************************************/
static GtkWidget* create_text_win(gchar* title)
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *text;

	/* Principal Window */
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),title);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(Fenetre));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	gtk_widget_realize(Win);
	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)gtk_widget_destroy,NULL);

	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	vboxall = create_vbox(Win);
	vboxwin = vboxall;

	frame = gtk_frame_new (NULL);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_add(GTK_CONTAINER(vboxall),frame);
	gtk_widget_show (frame);
	text = create_text(Win,frame,TRUE);
	set_font (text,FontsStyleResult.fontname);
	set_base_style(text,FontsStyleResult.BaseColor.red ,FontsStyleResult.BaseColor.green ,FontsStyleResult.BaseColor.blue);
	set_text_style(text,FontsStyleResult.TextColor.red ,FontsStyleResult.TextColor.green ,FontsStyleResult.TextColor.blue);
	g_object_set_data(G_OBJECT (Win), "Text", text);
	return Win;
}
/*****************************************************************************/
void compute_charge_by_residue()
{
	gint i;
	gint j;
	gint k;
	gdouble* charges = NULL;
	gint nr = 0;
	GtkWidget *win;
	GtkWidget *text;
	gchar tmp[BSIZE];
	gint* nums = NULL;

	if(Natoms<1) return;

	for(j=0;j<(gint)Natoms;j++)
	{
		if( nr<geometry0[j].ResidueNumber+1) nr = geometry0[j].ResidueNumber+1;
	}
	if(nr<1) return;

	charges = g_malloc(nr*sizeof(gdouble));
	nums = g_malloc(nr*sizeof(gint));

	for(k=0;k<nr;k++) charges[k] = 0;
	for(k=0;k<nr;k++) nums[k] = 0;
	

	for(j=0;j<(gint)Natoms;j++)
	{
		k =  geometry0[j].ResidueNumber;
		charges[k] += geometry0[j].Charge;
		nums[k] = j;
	}
	for(i=0;i<nr-1;i++)
	{
		k = i;
		for(j=i+1;j<nr;j++)
			if(geometry0[nums[k]].ResidueNumber>geometry0[nums[j]].ResidueNumber) k = j;
		if(i!=k)
		{
			gdouble x;
			gint ix;
			x = charges[k];
			charges[k] = charges[i];
			charges[i] = x;
			ix = nums[k];
			nums[k] = nums[i];
			nums[i] = ix;
			
		}
	}
	win = create_text_win(_("Charge by residues"));
    	gtk_widget_set_size_request(GTK_WIDGET(win),(gint)(ScreenHeight*0.5),(gint)(ScreenHeight*0.5));
	text = g_object_get_data(G_OBJECT (win), "Text");
	if(text)
	for(k=0;k<nr;k++)
	{
		j = nums[k]+1;
		sprintf(tmp,_("%s[%d] Charge = %f\n"),
				geometry0[j-1].Residue,geometry0[j-1].ResidueNumber+1,charges[k]);
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,tmp,-1);
	}
	if(nums)g_free(nums);
	if(charges)g_free(charges);
	gtk_widget_show_all(win);
}
/*****************************************************************************/
void compute_dipole_from_charges()
{
	gint j;
	gdouble D[3] = {0,0,0};
    	GtkWidget* m;
	gchar tmp[100];
	gdouble tot = 0;

	for(j=0;j<(gint)Natoms;j++)
	{
		D[0] += geometry0[j].X*geometry0[j].Charge;
		D[1] += geometry0[j].Y*geometry0[j].Charge;
		D[2] += geometry0[j].Z*geometry0[j].Charge;
	}	
	for(j=0;j<3;j++)
		D[j] *= AUTODEB;

	for(j=0;j<3;j++)
		tot += D[j]*D[j];
	tot = sqrt(tot);

	sprintf(tmp,_("Dipole (Debye) : X= %f Y= %f Z= %f  Tot=%f\n"),D[0] ,  D[1], D[2],tot);
	m = Message(tmp,_("Info"),TRUE);
	gtk_window_set_modal (GTK_WINDOW (m), TRUE);
}
/*****************************************************************************/
gchar *get_distance(gint i,gint j)
{
        Point A;
        Point B;
        guint k;
        guint Ni=-1;
        guint Nj=-1;
        gchar *serr;

       for (k=0;k<Natoms;k++)
	  if(geometry[k].N== (guint)i)
		Ni = k;
       for (k=0;k<Natoms;k++)
	  if(geometry[k].N== (guint)j)
		Nj = k;
        
        if(Ni==-1 || Nj == -1)
        {
           serr=g_strdup("ERROR");
           return serr; 
        }

	A.C[0]=geometry[Ni].X;
	A.C[1]=geometry[Ni].Y;
	A.C[2]=geometry[Ni].Z;
        
	B.C[0]=geometry[Nj].X;
	B.C[1]=geometry[Nj].Y;
	B.C[2]=geometry[Nj].Z;

        return get_distance_points(A,B,FALSE);
}
/*****************************************************************************/
gchar *get_angle(gint i,gint j,gint l)
{
        Point A;
        Point B;
        guint k;
        guint Ni=0;
        guint Nj=0;
        guint Nl=0;

       for (k=0;k<Natoms;k++)
	  if(geometry[k].N== (guint)i)
		Ni = k;
       for (k=0;k<Natoms;k++)
	  if(geometry[k].N== (guint)j)
		Nj = k;
       for (k=0;k<Natoms;k++)
	  if(geometry[k].N== (guint)l)
		Nl = k;
        

	A.C[0]=geometry[Ni].X-geometry[Nj].X;
	A.C[1]=geometry[Ni].Y-geometry[Nj].Y;
	A.C[2]=geometry[Ni].Z-geometry[Nj].Z;
        
	B.C[0]=geometry[Nl].X-geometry[Nj].X;
	B.C[1]=geometry[Nl].Y-geometry[Nj].Y;
	B.C[2]=geometry[Nl].Z-geometry[Nj].Z;

        return get_angle_vectors(A,B);
}
/*****************************************************************************/
gchar *get_dihedral(gint i,gint j,gint l,gint m)
{
        Point A;
        Point B;
        Point V1;
        Point V2;
        Point W1;
        guint k;
        guint Ni=0;
        guint Nj=0;
        guint Nl=0;
        guint Nm=0;
	gdouble angle;
	gdouble dihsgn;

       for (k=0;k<Natoms;k++)
	  if(geometry[k].N== (guint)i)
		Ni = k;
       for (k=0;k<Natoms;k++)
	  if(geometry[k].N==(guint)j)
		Nj = k;
       for (k=0;k<Natoms;k++)
	  if(geometry[k].N== (guint)l)
		Nl = k;
       for (k=0;k<Natoms;k++)
	  if(geometry[k].N== (guint)m)
		Nm = k;
        

	V1.C[0]=geometry[Ni].X-geometry[Nj].X;
	V1.C[1]=geometry[Ni].Y-geometry[Nj].Y;
	V1.C[2]=geometry[Ni].Z-geometry[Nj].Z;

	V2.C[0]=geometry[Nl].X-geometry[Nj].X;
	V2.C[1]=geometry[Nl].Y-geometry[Nj].Y;
	V2.C[2]=geometry[Nl].Z-geometry[Nj].Z;

        A = get_produit_vectoriel(V1,V2);

	V1.C[0]=geometry[Nm].X-geometry[Nl].X;
	V1.C[1]=geometry[Nm].Y-geometry[Nl].Y;
	V1.C[2]=geometry[Nm].Z-geometry[Nl].Z;

	V2.C[0]=geometry[Nj].X-geometry[Nl].X;
	V2.C[1]=geometry[Nj].Y-geometry[Nl].Y;
	V2.C[2]=geometry[Nj].Z-geometry[Nl].Z;

        B = get_produit_vectoriel(V2,V1);

        angle = atof(get_angle_vectors(A,B));

        W1 = get_produit_vectoriel(A,B);
        if (get_module(W1)<1e-5 )
              dihsgn = 1.0e0;
        else
	{
        dihsgn = get_scalaire(W1,V2);
        if (dihsgn>0)
            dihsgn = -1.0e0;
        else
           dihsgn = 1.0e0;
	}
          angle *=dihsgn;
	return g_strdup_printf("%f",angle);
}
/*****************************************************************************/
void define_coord_maxmin()
{
	guint i;
        gdouble XmaxMmin;
        gdouble YmaxMmin;
        if(Natoms==0) 
        	return;
	coordmaxmin.Xmax =geometry[0].X;
	coordmaxmin.Ymax =geometry[0].Y;
	coordmaxmin.Zmax =geometry[0].Z;

	coordmaxmin.Xmin =geometry[0].X;
	coordmaxmin.Ymin =geometry[0].Y;
	coordmaxmin.Zmin =geometry[0].Z;

	for (i = 1;i<Natoms;i++)
	{
	if (geometry[i].X>coordmaxmin.Xmax)
			   coordmaxmin.Xmax =geometry[i].X;
	if (geometry[i].X<coordmaxmin.Xmin)
			   coordmaxmin.Xmin =geometry[i].X;

	if (geometry[i].Y>coordmaxmin.Ymax)
			   coordmaxmin.Ymax =geometry[i].Y;
	if (geometry[i].Y<coordmaxmin.Ymin)
			   coordmaxmin.Ymin =geometry[i].Y;

    if (geometry[i].Z>coordmaxmin.Zmax)
			   coordmaxmin.Zmax =geometry[i].Z;	
	if (geometry[i].Z<coordmaxmin.Zmin)
			   coordmaxmin.Zmin =geometry[i].Z;

	}
	XmaxMmin = coordmaxmin.Xmax-coordmaxmin.Xmin;
        if(fabs(XmaxMmin)<1.e-4 ) XmaxMmin = 1.0;
	YmaxMmin = coordmaxmin.Ymax-coordmaxmin.Ymin;
        if(fabs(YmaxMmin)<1.e-4 ) YmaxMmin = 1.0;
        coordmaxmin.Cmax = XmaxMmin;
	if(coordmaxmin.Cmax<YmaxMmin)
        	coordmaxmin.Cmax = YmaxMmin;

}
/*****************************************************************************/
void geometry_in_au()
{
        guint i;
        for(i=0;i<Natoms;i++)
	{
           geometry[i].X *=ANG_TO_BOHR ;
           geometry[i].Y *=ANG_TO_BOHR ;
           geometry[i].Z *=ANG_TO_BOHR ;
        }
}
/*****************************************************************************/
static void set_layer(gchar* layer, GabEditLayerType* l)
{
	if(strstr(layer,"Low")) *l = LOW_LAYER;
	else if(strstr(layer,"Medium")) *l= MEDIUM_LAYER;
	else *l = HIGH_LAYER;
}
/*****************************************************************************/
static void set_constant_variable(gint i, gboolean xyz)
{
	if(xyz)
	{
        	if(!test(GeomXYZ[i].X) || !test(GeomXYZ[i].Y) || !test(GeomXYZ[i].Z))
			geometry[i].Variable = TRUE;
		else
			geometry[i].Variable = FALSE;
	}
	else
	{
		if( i<1) 
		{
			geometry[i].Variable = FALSE;
			return;
		}
		if(i>0 && !test(Geom[i].R)) 
		{
			geometry[i].Variable = TRUE;
			return;
		}
		if(i>1 && !test(Geom[i].Angle)) 
		{
			geometry[i].Variable = TRUE;
			return;
		}
		if(i>2 && !test(Geom[i].Dihedral)) 
		{
			geometry[i].Variable = TRUE;
			return;
		}
		geometry[i].Variable = FALSE;
	}
}
/*****************************************************************************/
void set_layer_of_selected_atoms(GabEditLayerType l)
{
	gint i;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(if_selected(i))
		{
			geometry[i].Layer = l;
			geometry0[i].Layer = l;
		}
	}
	create_GeomXYZ_from_draw_grometry();
	drawGeom();
}
/*****************************************************************************/
void define_geometry_from_xyz()
{
        guint i;

        for(i=0;i<Natoms;i++)
	{
         if(!test(GeomXYZ[i].X))
                 geometry[i].X = get_value_variableXYZ(GeomXYZ[i].X);
          else
		geometry[i].X = atof(GeomXYZ[i].X);
         if(!test(GeomXYZ[i].Y))
                 geometry[i].Y = get_value_variableXYZ(GeomXYZ[i].Y);
          else
		geometry[i].Y = atof(GeomXYZ[i].Y);
         if(!test(GeomXYZ[i].Z))
                 geometry[i].Z = get_value_variableXYZ(GeomXYZ[i].Z);
          else
		geometry[i].Z = atof(GeomXYZ[i].Z);
	geometry[i].Prop = prop_atom_get(GeomXYZ[i].Symb);
	geometry[i].mmType = g_strdup(GeomXYZ[i].mmType);
	geometry[i].pdbType = g_strdup(GeomXYZ[i].pdbType);
	geometry[i].Residue = g_strdup(GeomXYZ[i].Residue);
	geometry[i].ResidueNumber = GeomXYZ[i].ResidueNumber;
	geometry[i].show = TRUE;
	geometry[i].Charge = atof(GeomXYZ[i].Charge);
	set_layer(GeomXYZ[i].Layer, &geometry[i].Layer);
	set_constant_variable(i, TRUE);
	geometry[i].N = i+1;
	geometry[i].typeConnections = NULL;
	}
	for(i=0;i<Natoms;i++)
	{
		gint j;
		geometry[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<(gint)Natoms;j++) geometry[i].typeConnections[j] = 0;
		if(GeomXYZ[i].typeConnections)
		{
			for(j=0;j<Natoms;j++) 
			geometry[i].typeConnections[j] = GeomXYZ[i].typeConnections[j];
		}
	}
}
/*****************************************************************************/
gboolean define_geometry_from_zmat()
{
  gdouble cosph,sinph,costh,sinth,coskh,sinkh;
  gdouble cosa,sina,cosd,sind;
  gdouble dist,angle,dihed;
  gdouble xpd,ypd,zpd,xqd,yqd,zqd;
  gdouble xa,ya,za,xb,yb,zb;
  gdouble rbc,xyb,yza,temp;
  gdouble xpa,ypa,zqa;
  gdouble xd,yd,zd;
  gboolean flag;
  gint i, na, nb, nc;

  if (Natoms == 0)
    return( FALSE );
  /* Atom #1 */
  geometry[0].X = 0.0;
  geometry[0].Y = 0.0;
  geometry[0].Z = 0.0;
  geometry[0].N = 1;
  geometry[0].typeConnections = NULL;
  
  if (Natoms == 1)
  {
    geometry[0].Prop = prop_atom_get(Geom[0].Symb);
    geometry[0].mmType   = g_strdup(Geom[0].mmType);
    geometry[0].pdbType   = g_strdup(Geom[0].pdbType);
    geometry[0].Residue   = g_strdup(Geom[0].Residue);
    geometry[0].ResidueNumber   = Geom[0].ResidueNumber;
    geometry[0].show   = TRUE;
    geometry[0].Charge = atof(Geom[0].Charge);
    set_layer(Geom[0].Layer, &geometry[0].Layer);
    set_constant_variable(0, FALSE);
    return( TRUE );
  }
  
  /* Atom #2 */
  if(!test(Geom[1].R))
    geometry[1].X = get_value_variableZmat(Geom[1].R);
  else
    geometry[1].X = atof(Geom[1].R);
  geometry[1].Y = 0.0;
  geometry[1].Z = 0.0;
  geometry[1].N = 2;
  geometry[1].typeConnections = NULL;
  
  if( Natoms == 2 )
  {
    geometry[0].Prop = prop_atom_get(Geom[0].Symb);
    geometry[1].Prop = prop_atom_get(Geom[1].Symb);
    geometry[0].mmType = g_strdup(Geom[0].mmType);
    geometry[1].mmType = g_strdup(Geom[1].mmType);
    geometry[0].pdbType = g_strdup(Geom[0].pdbType);
    geometry[1].pdbType = g_strdup(Geom[1].pdbType);
    geometry[0].Residue = g_strdup(Geom[0].Residue);
    geometry[1].Residue = g_strdup(Geom[1].Residue);
    geometry[0].ResidueNumber   = Geom[0].ResidueNumber;
    geometry[1].ResidueNumber   = Geom[1].ResidueNumber;
    geometry[0].show   = TRUE;
    geometry[1].show   = TRUE;
    geometry[0].Charge = atof(Geom[0].Charge);
    geometry[1].Charge = atof(Geom[1].Charge);
    set_layer(Geom[0].Layer, &geometry[0].Layer);
    set_layer(Geom[1].Layer, &geometry[1].Layer);
    set_constant_variable(0, FALSE);
    set_constant_variable(1, FALSE);
    return( TRUE );
  }
  
  /* Atom #3 */
  if(!test(Geom[2].R))
    dist = get_value_variableZmat(Geom[2].R);
  else
    dist = atof(Geom[2].R);

  if(!test(Geom[2].Angle))
    angle = get_value_variableZmat(Geom[2].Angle);
  else
    angle = atof(Geom[2].Angle);

    angle *=  DEG_TO_RAD;

  cosa = cos(angle);
  sina = sin(angle);
  
  if( atoi (Geom[2].NAngle) == 2 )
  	geometry[2].X =  geometry[0].X + cosa*dist;
  else 
  	geometry[2].X =  geometry[1].X - cosa*dist;

  geometry[2].Y =  sina*dist;
  geometry[2].Z = 0.0;
  geometry[2].N = 3;
  geometry[2].typeConnections = NULL;
  
  for (i = 3; i <(gint)Natoms; i++)
  {   
  if(!test(Geom[i].R))
    dist = get_value_variableZmat(Geom[i].R);
  else
    dist = atof(Geom[i].R);

  if(!test(Geom[i].Angle))
    angle = get_value_variableZmat(Geom[i].Angle);
  else
    angle = atof(Geom[i].Angle) ;

  if(!test(Geom[i].Dihedral))
    dihed = get_value_variableZmat(Geom[i].Dihedral);
  else
    dihed = atof(Geom[i].Dihedral) ;

    angle *= DEG_TO_RAD;
    dihed *= DEG_TO_RAD;

    na = atoi(Geom[i].NR)-1;
    nb = atoi(Geom[i].NAngle)-1;
    nc = atoi(Geom[i].NDihedral)-1;
    
    xb = geometry[nb].X - geometry[na].X;
    yb = geometry[nb].Y - geometry[na].Y;
    zb = geometry[nb].Z - geometry[na].Z;
    
    rbc = xb*xb + yb*yb + zb*zb;
    if( rbc < 0.0001 )
      return( FALSE );
    rbc = 1.0/sqrt(rbc);
    
    cosa = cos(angle);
    sina = sin(angle);
    
    
    if( fabs(cosa) >= 0.999999 )
    { 
      /* Colinear */
      temp = dist*rbc*cosa;
      geometry[i].X  = geometry[na].X + temp*xb;
      geometry[i].Y  = geometry[na].Y + temp*yb;
      geometry[i].Z  = geometry[na].Z + temp*zb;
      geometry[i].N = i+1;
      geometry[i].typeConnections = NULL;
    } 
    else
    {
      xa = geometry[nc].X - geometry[na].X;
      ya = geometry[nc].Y - geometry[na].Y;
      za = geometry[nc].Z - geometry[na].Z;
      
      sind = -sin(dihed);
      cosd = cos(dihed);
      
      xd = dist*cosa;
      yd = dist*sina*cosd;
      zd = dist*sina*sind;
      
      xyb = sqrt(xb*xb + yb*yb);
      if( xyb < 0.1 )
      {  
	/* Rotate about y-axis! */
	temp = za; za = -xa; xa = temp;
	temp = zb; zb = -xb; xb = temp;
	xyb = sqrt(xb*xb + yb*yb);
	flag = TRUE;
      }
      else 
	flag = FALSE;
      
      costh = xb/xyb;
      sinth = yb/xyb;
      xpa = costh*xa + sinth*ya;
      ypa = costh*ya - sinth*xa;
      
      sinph = zb*rbc;
      cosph = sqrt(1.0 - sinph*sinph);
      zqa = cosph*za  - sinph*xpa;
      
      yza = sqrt(ypa*ypa + zqa*zqa);
      
      if( yza > 1.0E-10 )
      {   
	coskh = ypa/yza;
	sinkh = zqa/yza;
	
	ypd = coskh*yd - sinkh*zd;
	zpd = coskh*zd + sinkh*yd;
      } 
      else
      { 
	/* coskh = 1.0; */
	/* sinkh = 0.0; */
	ypd = yd;
	zpd = zd;
      }
      
      xpd = cosph*xd  - sinph*zpd;
      zqd = cosph*zpd + sinph*xd;
      xqd = costh*xpd - sinth*ypd;
      yqd = costh*ypd + sinth*xpd;
      
      if( flag )
      { 
	/* Rotate about y-axis! */
	geometry[i].X = geometry[na].X - zqd;
	geometry[i].Y = geometry[na].Y + yqd;
	geometry[i].Z = geometry[na].Z + xqd;
        geometry[i].N = i+1;
        geometry[i].typeConnections = NULL;
      } 
      else
      {  
	geometry[i].X = geometry[na].X + xqd;
	geometry[i].Y = geometry[na].Y + yqd;
	geometry[i].Z = geometry[na].Z + zqd;
        geometry[i].N = i+1;
        geometry[i].typeConnections = NULL;
      }
    }
  }
  for(i=0;i<(gint)Natoms;i++)
  {
	geometry[i].Prop = prop_atom_get(Geom[i].Symb);
    	geometry[i].mmType = g_strdup(Geom[i].mmType);
    	geometry[i].pdbType = g_strdup(Geom[i].pdbType);
    	geometry[i].Residue = g_strdup(Geom[i].Residue);
    	geometry[i].ResidueNumber = Geom[i].ResidueNumber;
    	geometry[i].Charge = atof(Geom[i].Charge);
    	geometry[i].show   = TRUE;
    	set_layer(Geom[i].Layer, &geometry[i].Layer);
    	set_constant_variable(i, FALSE);
  }
  return( TRUE );
}
/*****************************************************************************/
void set_optimal_geom_view()
{
	gushort Xmax;
	gushort Ymax;
	gushort Rmax;
	gint Xi;
	gint Yi;
	guint i;
	gdouble X;
	gdouble Y;
	gdouble Cmax;
	gint X1,X2;
	gint Y1,Y2;

	Xmax=GeomDrawingArea->allocation.width;
	Ymax=GeomDrawingArea->allocation.height;
	X1 = Xmax;
	X2 = Xmax;
	Y1 = Ymax;
	Y2 = Ymax;

	Rmax = Xmax;
	if(Rmax<Ymax) Rmax = Ymax;

	sort_with_zaxis();
	define_coefs_pers();
	define_coord_maxmin();
	
	for (i = 0;i<Natoms;i++)
	{
		if(PersMode)
		{
			X = geometry[i].X*camera.f/(-geometry[i].Z+camera.position);
			Y = geometry[i].Y*camera.f/(-geometry[i].Z+camera.position);
			Cmax  = coordmaxmin.Cmax*camera.f/(camera.position);
		}
		else
		{
			X = geometry[i].X;
			Y = geometry[i].Y;
			Cmax = coordmaxmin.Cmax;
		}
		Xi = (gint)(X/Cmax*factor*Rmax/2)+Xmax/2+TransX;
		Yi = (gint)(-Y/Cmax*factor*Rmax/2)+Ymax/2+TransY;
		if(i==0)
		{
			X1 = Xi; X2 =Xi; Y1 = Yi; Y2 = Yi;
		}
		else
		{
			if(X1>Xi) X1 = Xi;
			if(X2<Xi) X2 = Xi;
			if(Y1>Yi) Y1 = Yi;
			if(Y2<Yi) Y2 = Yi;
		}
	}
	if((X2-X1)>(gint)Xmax)
		factor *= fabs((gdouble)(Xmax-20)/(X2-X1));

	if((Y2-Y1)>(gint)Ymax)
		factor *= fabs((gdouble)(Ymax-20)/(Y2-Y1));
}
/********************************************************************************/
void define_good_factor()
{
	set_optimal_geom_view();
}
/********************************************************************************/
/*
static void define_good_trans()
{
	gdouble C[3]={0,0,0};
	gint i;
	gint k;
	gushort Xmax;
	gushort Ymax;
	gushort Rmax;
	gdouble Cmax;

	printf("Begin define_good_trans\n");
	if(!GeomDrawingArea) return;
	if(Natoms>0)
	{
		if(PersMode) Cmax  = coordmaxmin.Cmax*camera.f/(camera.position);
		else Cmax = coordmaxmin.Cmax;
	}
	else return;

	Xmax=GeomDrawingArea->allocation.width;
	Ymax=GeomDrawingArea->allocation.height;
	Rmax = Xmax;
	if(Rmax<Ymax) Rmax = Ymax;

	for (i=0;i<(gint)Natoms;i++)
	{
		C[0] += geometry[i].X;
		C[1] += geometry[i].Y;
		C[2] += geometry[i].Z;
	}
	for(k=0;k<3;k++) C[k] /= (gdouble)Natoms;
	TransX = (gint)(-C[0]/Cmax*factor*Rmax/2);
	TransY = (gint)(C[1]/Cmax*factor*Rmax/2);
}
*/
/*****************************************************************************/
void define_coord_ecran()
{
	gushort Xmax;
	gushort Ymax;
	gushort Rmax;
	gint Xi;
	gint Yi;
	guint i;
	gdouble X;
	gdouble Y;
	gdouble Cmax;

	Xmax=GeomDrawingArea->allocation.width;
	Ymax=GeomDrawingArea->allocation.height;
	Rmax = Xmax;
	if(Rmax<Ymax)
		Rmax = Ymax;
	
	for (i = 0;i<Natoms;i++)
	{
	if(PersMode)
	{
	X = geometry[i].X*camera.f/(-geometry[i].Z+camera.position);
	Y = geometry[i].Y*camera.f/(-geometry[i].Z+camera.position);
	Cmax  = coordmaxmin.Cmax*camera.f/(camera.position);
	}
	else
	{
	X = geometry[i].X;
	Y = geometry[i].Y;
	Cmax = coordmaxmin.Cmax;
	}
	Xi = (gint)(X/Cmax*factor*Rmax/2)+Xmax/2;
	Yi = (gint)(-Y/Cmax*factor*Rmax/2)+Ymax/2;

	geometry[i].Xi = Xi + TransX;
	geometry[i].Yi = Yi + TransY;
	geometry[i].Rayon = (gushort)(geometry[i].Prop.radii/coordmaxmin.Cmax*factor*Rmax/2);
	}
	CenterCoor[0]=0.0;
	CenterCoor[1]=0.0;
	for (i = 0;i<Natoms;i++)
	{
	CenterCoor[0] +=geometry[i].Xi;
	CenterCoor[1] +=geometry[i].Yi;
	}

	CenterCoor[0] /=Natoms;
	CenterCoor[1] /=Natoms;
	if(Ddef)
	{
		for(i=0;i<NDIVDIPOLE;i++)
		{
			if(PersMode)
			{
			X = dipole[i][0]*camera.f/(-dipole[i][2]+camera.position);
			Y = dipole[i][1]*camera.f/(-dipole[i][2]+camera.position);
			Cmax  = coordmaxmin.Cmax*camera.f/(camera.position);
			}
			else
			{
			X = dipole[i][0];
			Y = dipole[i][1];
			Cmax = coordmaxmin.Cmax;
			}
			DXi[i] = (gint)(X/Cmax*factor*Rmax/2)+Xmax/2 + TransX;
			DYi[i] = (gint)(-Y/Cmax*factor*Rmax/2)+Ymax/2 + TransY;
		}
	}
}
/*****************************************************************************/
void sort_with_zaxis()
{
	guint i;
	guint j;
	guint k;
	GeomDef tmpgeom;
	gdouble Z = 0;

	for (i = 0;i<Natoms-1;i++)
	{
        	Z = geometry[i].Z;
		k=i;
		for (j = i+1;j<Natoms;j++)
			if(Z>geometry[j].Z) 
			{
				k=j;
        			Z =geometry[k].Z;
			}
		if(k!=i)
		{
        		tmpgeom = geometry[i];
        		geometry[i] = geometry[k];
        		geometry[k] = tmpgeom;


			tmpgeom = geometry0[i];
        		geometry0[i] = geometry0[k];
        		geometry0[k] = tmpgeom;

			if(NumSelectedAtom == (gint)i)
				NumSelectedAtom = (gint)k;
			else
			if(NumSelectedAtom == (gint)k)
				NumSelectedAtom = (gint)i;
		
		}
	}
	calcul_ndipole();
}
/*****************************************************************************/
void define_coefs_pers()
{
	guint i;
        if(fabs(coordmaxmin.Zmax-coordmaxmin.Zmin)<1.e-6)
	{
		camera.position = 10;
		camera.f = 5;
        	for(i=0;i<Natoms;i++)
			geometry[i].Coefpers = 1.0;
	}
        else
	{
		gdouble posx = 4*fabs(coordmaxmin.Xmax-coordmaxmin.Xmin);
		gdouble posy = 4*fabs(coordmaxmin.Ymax-coordmaxmin.Ymin);
		gdouble posz = 4*fabs(coordmaxmin.Zmax-coordmaxmin.Zmin);
		gdouble pos = posx;

		if(pos<posy) pos = posy;
		if(pos<posz) pos = posz;

		camera.position = pos;

		camera.f = camera.position/2;
        	for(i=0;i<Natoms;i++)
			geometry[i].Coefpers = 1.5*camera.f/(-geometry[i].Z+camera.position); 
	}
}
/*****************************************************************************/
void set_color_shad(GdkColor *color,guint i)
{
 gdouble Coef=1.0;
 if(fabs(coordmaxmin.Zmax-coordmaxmin.Zmin)>1.e-6)
 {
 	Coef = fabs(0.99-0.2*(coordmaxmin.Zmax-geometry[i].Z)/(coordmaxmin.Zmax-coordmaxmin.Zmin));
        if(Coef<1.0)
	{
	color->red =  (gushort)(color->red*Coef) ;
 	color->green =  (gushort)(color->green*Coef) ;
 	color->blue =  (gushort)(color->blue*Coef) ;
	}
 }
}
/*****************************************************************************/
void rotationGen(gdouble alpha,gdouble Axe[],gdouble A[],gdouble B[])
{
/* Axe est un tableau de 2 elements qui definit l'axe de rotation qui se trouve dans le paln xOy*/
/* theta angle entre l'axe de rotation et l'axe x. l'axe de rotation se trouve dans le plan xoy */
/* alpha angle rotation autour de l'axe de rotation defini par theta */
	gdouble cosa = cos(alpha/180*PI);
	gdouble sina = sin(alpha/180*PI);
        gdouble cost=Axe[0]/sqrt(Axe[0]*Axe[0] + Axe[1]*Axe[1] );
        gdouble sint=Axe[1]/sqrt(Axe[0]*Axe[0] + Axe[1]*Axe[1] );
	
	B[0] = A[0]*cost*cost + A[1]*sint*cost + A[0]*sint*sint*cosa - A[1]*cost*cosa*sint+A[2]*sint*sina;
	B[1] = A[0]*cost*sint + A[1]*sint*sint - A[0]*sint*cost*cosa + A[1]*cost*cosa*cost-A[2]*cost*sina;
	B[2] = -A[0]*sint*sina + A[1]*cost*sina + A[2]*cosa;
}
/*****************************************************************************/
static gint replace_atom(gint i)
{
	g_free(geometry[i].Prop.symbol);
	geometry[i].Prop = prop_atom_get(AtomToInsert);
	g_free(geometry0[i].Prop.symbol);
	geometry0[i].Prop = prop_atom_get(AtomToInsert);

	g_free(geometry[i].mmType);
	geometry[i].mmType = g_strdup(AtomToInsert);
	g_free(geometry0[i].mmType);
	geometry0[i].mmType = g_strdup(AtomToInsert);

	g_free(geometry[i].pdbType);
	geometry[i].pdbType = g_strdup(AtomToInsert);
	g_free(geometry0[i].pdbType);
	geometry0[i].pdbType = g_strdup(AtomToInsert);
	return 1;
}
/*****************************************************************************/
static gint insert_atom(GdkEventButton *bevent)
{
	int x, y;
	gdouble X;
	gdouble Y;
	gdouble Z;
	gdouble Cmax;
	gushort Xmax;
	gushort Ymax;
	gushort Rmax;
	
	if(Natoms>0 && NumProcheAtom<0) return -1;
	if(Natoms>0)
	{
		geometry0 = g_realloc(geometry0,(Natoms+1)*sizeof(GeomDef));
		geometry = g_realloc(geometry,(Natoms+1)*sizeof(GeomDef)); 
	}
	else
	{
		geometry0 = g_malloc(sizeof(GeomDef));
		geometry = g_malloc(sizeof(GeomDef)); 
	}

	Ddef = FALSE;

	Xmax=GeomDrawingArea->allocation.width;
	Ymax=GeomDrawingArea->allocation.height;
	Rmax = Xmax;
	if(Rmax<Ymax)
		Rmax = Ymax;
	

	x = bevent->x;
	y = bevent->y;
  
	if(Natoms>0)
	{
		if(PersMode)
			Cmax  = coordmaxmin.Cmax*camera.f/(camera.position);
		else
			Cmax = coordmaxmin.Cmax;
	}
	else
	{
		geometry[Natoms].Prop = prop_atom_get(AtomToInsert);
		Cmax = 4*geometry[Natoms].Prop.covalentRadii;
		coordmaxmin.Cmax = Cmax;
	}

	X = (gdouble)(x-Xmax/2-TransX)*2.0*Cmax/(factor*Rmax);
	Y = -(gdouble)(y-Ymax/2-TransY)*2.0*Cmax/(factor*Rmax);
	if(Natoms>0)
		Z = geometry[NumProcheAtom].Z;
	else
		Z = 0.0;

	if(PersMode)
	{
		X = X/camera.f*(-Z+camera.position);
		Y = Y/camera.f*(-Z+camera.position);
	}
	{
		gdouble m[4][4];
		gdouble **m0 = g_malloc(3*sizeof(gdouble*));
		gdouble** minv;
		gint i,j;

		gdouble A[3];
		gdouble B[3];
		guint k;

		for(i=0;i<3;i++)
			m0[i] = g_malloc(3*sizeof(gdouble));

		build_rotmatrix(m,Quat);

		for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			m0[i][j] = m[i][j];

		minv = Inverse(m0,3,1e-7);
		if(minv)
		{
			A[0] = X;
			A[1] = Y;
			A[2] = Z;
			for(j=0;j<3;j++)
			{
				B[j] = 0.0;
				for(k=0;k<3;k++)
				B[j] += minv[k][j]*A[k];
			}
			geometry[Natoms].X=B[0];
			geometry[Natoms].Y=B[1];
			geometry[Natoms].Z=B[2];
			geometry[Natoms].Prop = prop_atom_get(AtomToInsert);
			geometry[Natoms].mmType = g_strdup(AtomToInsert);
			geometry[Natoms].pdbType = g_strdup(AtomToInsert);
			geometry[Natoms].Layer = HIGH_LAYER;
			geometry[Natoms].show = TRUE;
			geometry[Natoms].N = Natoms+1;
        		geometry[Natoms].typeConnections = NULL;
			if(Natoms==0)
			{
				geometry[Natoms].Residue = g_strdup(AtomToInsert);
				geometry[Natoms].ResidueNumber = 0;
			}
			else
			{
				gint k;
				gint proche = 0;
				gdouble d;
				gdouble d1;
				d = get_real_distance2(geometry,0,Natoms);
				for(k=1;k<(gint)Natoms;k++)
				{
					
					d1 = get_real_distance2(geometry,k,Natoms);
					if(d1<d)
					{
						proche = k;
						d = d1;
					}
				}
				geometry[Natoms].Residue = g_strdup(geometry[proche].Residue);
				geometry[Natoms].ResidueNumber = geometry[proche].ResidueNumber;
			}

			geometry[Natoms].Charge = 0.0;

			geometry0[Natoms].Prop = prop_atom_get(AtomToInsert);
			geometry0[Natoms].mmType = g_strdup(AtomToInsert);
			geometry0[Natoms].pdbType = g_strdup(AtomToInsert);
			geometry0[Natoms].Layer = HIGH_LAYER;
			geometry0[Natoms].Variable = TRUE;
			geometry0[Natoms].Residue = g_strdup(geometry[Natoms].Residue);
			geometry0[Natoms].ResidueNumber = geometry[Natoms].ResidueNumber;
			geometry0[Natoms].show = geometry[Natoms].show;
			geometry0[Natoms].X = geometry[Natoms].X;
			geometry0[Natoms].Y = geometry[Natoms].Y;
			geometry0[Natoms].Z = geometry[Natoms].Z;
			geometry0[Natoms].Charge = 0.0;
			geometry0[Natoms].N = Natoms+1;
        		geometry0[Natoms].typeConnections = NULL;

			for(i=0;i<3;i++)
				if(minv[i])
				g_free(minv[i]);
			g_free(minv);
			Natoms++;
		}
		if(m0)
		{
			for(i=0;i<3;i++)
				if(m0[i])
					g_free(m0[i]);
				g_free(m0);
		}
	}
	Ddef = FALSE;
	{
		gint i;
		gint j;
		for(i=0;i<(gint)Natoms-1;i++)
		{
			geometry[i].typeConnections = g_realloc(geometry[i].typeConnections,Natoms*sizeof(gint));
			geometry[i].typeConnections[Natoms-1] = 0;
			geometry0[i].typeConnections = g_realloc(geometry0[i].typeConnections,Natoms*sizeof(gint));
		}
		i = Natoms-1;
		geometry[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<(gint)Natoms;j++) geometry[i].typeConnections[j] = 0;
		geometry0[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<(gint)Natoms;j++) geometry0[i].typeConnections[j] = 0;
	}
	reset_charges_multiplicities();
	return TRUE;
}
/*****************************************************************************/
static void select_fragment(gint NDelAtom)
{
	gint i;

	if(!NumFatoms)
		g_free(NumFatoms);
	NumFatoms = NULL;

	NFatoms = Frag.NAtoms-NDelAtom;
	if(NFatoms<1)
		return;
	NumFatoms = g_malloc(NFatoms*sizeof(gint));
	for(i=0;i<(gint)NFatoms;i++)
	{
			NumFatoms[i] = geometry[i+Natoms-NFatoms].N;
	}
}
/*****************************************************************************/
static gint get_number_new_residue()
{
	gint numMax =-1;
	gint i;
	if(Natoms<1 || !geometry0)
		return numMax + 1;
	for(i=0;i<(gint)Natoms;i++)
		if(numMax<geometry0[i].ResidueNumber)
			numMax = geometry0[i].ResidueNumber;
	return numMax +1;
}
/*****************************************************************************/
static gint insert_fragment_without_delete_an_atom(GtkWidget *widget,GdkEvent *event)
{
	int x, y;
	GdkEventButton *bevent=(GdkEventButton *)event;
	gdouble X;
	gdouble Y;
	gdouble Z;
	gdouble Cmax;
	gushort Xmax;
	gushort Ymax;
	gushort Rmax;
	gint i;
	gint iBegin;
	gint j;

	if(Natoms>0 && NumProcheAtom<0)
	{
		FreeFragment(&Frag);
		return 0;
	}
	if(Frag.NAtoms<=0)
	{
		FreeFragment(&Frag);
		return 0;
	}

	if(Natoms>0)
	{
		geometry0 = g_realloc(geometry0,(Natoms+Frag.NAtoms)*sizeof(GeomDef));
		geometry  = g_realloc(geometry,(Natoms+Frag.NAtoms)*sizeof(GeomDef)); 
	}
	else
	{
		geometry0 = g_malloc(Frag.NAtoms*sizeof(GeomDef));
		geometry  = g_malloc(Frag.NAtoms*sizeof(GeomDef)); 
	}

	Ddef = FALSE;

	Xmax=GeomDrawingArea->allocation.width;
	Ymax=GeomDrawingArea->allocation.height;
	Rmax = Xmax;
	if(Rmax<Ymax)
		Rmax = Ymax;
	

	x = bevent->x;
	y = bevent->y;
  
	if(Natoms>0)
	{
		if(PersMode) Cmax  = coordmaxmin.Cmax*camera.f/(camera.position);
		else Cmax = coordmaxmin.Cmax;
	}
	else
	{
		Natoms = 0;
		for(i=0;i<Frag.NAtoms;i++)
			geometry[Natoms+i].Prop = prop_atom_get(Frag.Atoms[i].Symb);
		Cmax = 4*geometry[Natoms].Prop.covalentRadii;
		for(i=1;i<Frag.NAtoms;i++)
			if(Cmax<geometry[Natoms+i].Prop.covalentRadii)
			       Cmax = geometry[Natoms+i].Prop.covalentRadii; 
		coordmaxmin.Cmax = Cmax;
	}

	X = (gdouble)(x-Xmax/2-TransX)*2.0*Cmax/(factor*Rmax);
	Y = -(gdouble)(y-Ymax/2-TransY)*2.0*Cmax/(factor*Rmax);
	if(Natoms>0) Z = geometry[NumProcheAtom].Z;
	else Z = 0.0;

	if(PersMode)
	{
		X = X/camera.f*(-Z+camera.position);
		Y = Y/camera.f*(-Z+camera.position);
	}
	{
		gdouble m[4][4];
		gdouble **m0 = g_malloc(3*sizeof(gdouble*));
		gdouble** minv;
		gint i,j;

		gdouble A[3];
		gdouble B[3];
		guint k;

		for(i=0;i<3;i++)
			m0[i] = g_malloc(3*sizeof(gdouble));

		build_rotmatrix(m,Quat);

		for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			m0[i][j] = m[i][j];

		minv = Inverse(m0,3,1e-7);
		if(minv)
		{
			A[0] = X;
			A[1] = Y;
			A[2] = Z;
			for(j=0;j<3;j++)
			{
				B[j] = 0.0;
				for(k=0;k<3;k++)
				B[j] += minv[k][j]*A[k];
			}
			j = -1;

			for(i=0;i<Frag.NAtoms;i++)
			{
				j++;
				geometry0[Natoms+j].X=B[0]+Frag.Atoms[i].Coord[0];
				geometry0[Natoms+j].Y=B[1]+Frag.Atoms[i].Coord[1];
				geometry0[Natoms+j].Z=B[2]+Frag.Atoms[i].Coord[2];
				geometry0[Natoms+j].Prop = prop_atom_get(Frag.Atoms[i].Symb);
				geometry0[Natoms+j].mmType =g_strdup(Frag.Atoms[i].mmType);
				geometry0[Natoms+j].pdbType =g_strdup(Frag.Atoms[i].pdbType);
				geometry0[Natoms+j].Layer = HIGH_LAYER;
				geometry0[Natoms+j].Variable = TRUE;
				geometry0[Natoms+j].Residue =g_strdup(Frag.Atoms[i].Residue);
				geometry0[Natoms+j].ResidueNumber= get_number_new_residue();
				geometry0[Natoms+j].show= TRUE;
				geometry0[Natoms+j].Charge = Frag.Atoms[i].Charge;
				geometry0[Natoms+j].N = Natoms+j+1;
        			geometry0[Natoms+j].typeConnections = NULL;

				geometry[Natoms+j].Prop = prop_atom_get(Frag.Atoms[i].Symb);
				geometry[Natoms+j].mmType =g_strdup(Frag.Atoms[i].mmType);
				geometry[Natoms+j].pdbType =g_strdup(Frag.Atoms[i].pdbType);
				geometry[Natoms+j].Layer = HIGH_LAYER;
				geometry[Natoms+j].Residue =g_strdup(Frag.Atoms[i].Residue);
				geometry[Natoms+j].ResidueNumber=geometry0[Natoms+j].ResidueNumber;
				geometry[Natoms+j].show=geometry0[Natoms+j].show;
				geometry[Natoms+j].Charge = Frag.Atoms[i].Charge;
				geometry[Natoms+j].N = Natoms+j+1;
        			geometry[Natoms+j].typeConnections = NULL;

				geometry[Natoms+j].X= geometry0[Natoms+j].X;
				geometry[Natoms+j].Y= geometry0[Natoms+j].Y;
				geometry[Natoms+j].Z= geometry0[Natoms+j].Z;
			}

			for(i=0;i<3;i++)
				if(minv[i]) g_free(minv[i]);
			g_free(minv);
			
			Natoms+=Frag.NAtoms;
		}
		if(m0)
		{
			for(i=0;i<3;i++)
				if(m0[i])
					g_free(m0[i]);
				g_free(m0);
		}
	}
	Ddef = FALSE;
	iBegin = Natoms-Frag.NAtoms;
	for(i=0;i<iBegin;i++)
	{
		if(!geometry[i].typeConnections) continue;
		geometry[i].typeConnections = g_realloc(geometry[i].typeConnections,Natoms*sizeof(gint));
		for(j=iBegin;j<Natoms;j++) geometry[i].typeConnections[j] = 0;

		if(!geometry0[i].typeConnections) continue;
		geometry0[i].typeConnections = g_realloc(geometry0[i].typeConnections,Natoms*sizeof(gint));
		for(j=iBegin;j<Natoms;j++) geometry0[i].typeConnections[j] = 0;
	}
	for(i=iBegin;i<Natoms;i++)
	{
		geometry[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<Natoms;j++) geometry[i].typeConnections[j] = 0;

		geometry0[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<Natoms;j++) geometry0[i].typeConnections[j] = 0;
	}
	for(i=iBegin;i<Natoms;i++)
	{
		for(j=i+1;j<Natoms;j++)
		{
			if(draw_lines_yes_no(i,j)) geometry[i].typeConnections[j]= 1;
			geometry[j].typeConnections[i] = geometry[i].typeConnections[j];
		}
	}
	reset_multiple_bonds();
	define_good_factor();
	reset_charges_multiplicities();
	/* SetOriginAtCenter(NULL,0,NULL);*/
	set_statubar_pop_sel_atom();
	return 1;
}
/*****************************************************************************/
static gint insert_fragment_connected_to_an_atom(gint toD, gint toB, gint toA)
{
	gint i;
	gint toBond=-1;
	gint toAngle=-1;
	gint* atomlist = NULL;
	gint toDel = -1;
	gdouble B[3];
	gint j;
	gint iBegin;
	gdouble bondLength;

	if(toD <0 || toB <0 || Frag.atomToDelete<0 || Frag.atomToBondTo<0) return 0;
	if(toD >= Natoms || toB>= Natoms || Frag.atomToDelete>= Frag.NAtoms || Frag.atomToBondTo>= Frag.NAtoms) return 0;

	if(Natoms>0)
	{
		geometry0 = g_realloc(geometry0,(Natoms+Frag.NAtoms)*sizeof(GeomDef));
		geometry  = g_realloc(geometry,(Natoms+Frag.NAtoms)*sizeof(GeomDef)); 
	}
	else
	{
		geometry0 = g_malloc(Frag.NAtoms*sizeof(GeomDef));
		geometry  = g_malloc(Frag.NAtoms*sizeof(GeomDef)); 
	}

	Ddef = FALSE;

	atomlist = g_malloc((Frag.NAtoms)*sizeof(gint));
	B[0] = geometry0[toB].X - Frag.Atoms[Frag.atomToDelete].Coord[0];
	B[1] = geometry0[toB].Y - Frag.Atoms[Frag.atomToDelete].Coord[1];
	B[2] = geometry0[toB].Z - Frag.Atoms[Frag.atomToDelete].Coord[2];

	j = -1;
	toBond = -1;
	toAngle = -1;
	toDel = -1;
	for(i=0;i<Frag.NAtoms;i++)
	{
		j++;
		geometry0[Natoms+j].X=B[0]+Frag.Atoms[i].Coord[0];
		geometry0[Natoms+j].Y=B[1]+Frag.Atoms[i].Coord[1];
		geometry0[Natoms+j].Z=B[2]+Frag.Atoms[i].Coord[2];

		if(i==Frag.atomToDelete) toDel = Natoms+j;
		if(i==Frag.atomToBondTo) toBond = Natoms+j;
		if(i==Frag.angleAtom) toAngle = Natoms+j;
		atomlist[j] = Natoms+j;

		geometry0[Natoms+j].Prop = prop_atom_get(Frag.Atoms[i].Symb);
		geometry0[Natoms+j].mmType =g_strdup(Frag.Atoms[i].mmType);
		geometry0[Natoms+j].pdbType =g_strdup(Frag.Atoms[i].pdbType);
		geometry0[Natoms+j].Layer = HIGH_LAYER;
		geometry0[Natoms+j].Variable = TRUE;
		geometry0[Natoms+j].Residue =g_strdup(Frag.Atoms[i].Residue);
		geometry0[Natoms+j].ResidueNumber= get_number_new_residue();
		geometry0[Natoms+j].show= TRUE;
		geometry0[Natoms+j].Charge = Frag.Atoms[i].Charge;
		geometry0[Natoms+j].N = Natoms+j+1;
        	geometry0[Natoms+j].typeConnections = NULL;

		geometry[Natoms+j].Prop = prop_atom_get(Frag.Atoms[i].Symb);
		geometry[Natoms+j].mmType =g_strdup(Frag.Atoms[i].mmType);
		geometry[Natoms+j].pdbType =g_strdup(Frag.Atoms[i].pdbType);
		geometry[Natoms+j].Layer = HIGH_LAYER;
		geometry[Natoms+j].Residue =g_strdup(Frag.Atoms[i].Residue);
		geometry[Natoms+j].ResidueNumber=geometry0[Natoms+j].ResidueNumber;
		geometry[Natoms+j].show=geometry0[Natoms+j].show;
		geometry[Natoms+j].Charge = Frag.Atoms[i].Charge;
		geometry[Natoms+j].N = Natoms+j+1;
        	geometry[Natoms+j].typeConnections = NULL;

		geometry[Natoms+j].X= geometry0[Natoms+j].X;
		geometry[Natoms+j].Y= geometry0[Natoms+j].Y;
		geometry[Natoms+j].Z= geometry0[Natoms+j].Z;
	}

	Natoms +=Frag.NAtoms;
	/* Set Distance toB-toD to radius toB + radius Frag.atomToBondTo */
	bondLength = geometry[toB].Prop.covalentRadii + geometry[toBond].Prop.covalentRadii; 
	bondLength /= (ANG_TO_BOHR);
	bondLength *= 0.85;
	SetBondDistance(geometry,toB, toBond, bondLength,atomlist, Frag.NAtoms);
			
			
	SetAngle(Natoms,geometry0,toD, toB, toBond,0.0,atomlist, Frag.NAtoms);

	if(toA>-1 && toAngle>-1)
	{
		SetTorsion(Natoms,geometry0, toA,toB, toBond, toAngle, fragAngle,atomlist, Frag.NAtoms);
	}
		
	if(!atomlist) g_free(atomlist);

	iBegin = Natoms-Frag.NAtoms;
	for(i=0;i<iBegin;i++)
	{
		if(!geometry[i].typeConnections) continue;
		geometry[i].typeConnections = g_realloc(geometry[i].typeConnections,Natoms*sizeof(gint));
		for(j=iBegin;j<Natoms;j++) geometry[i].typeConnections[j] = 0;

		if(!geometry0[i].typeConnections) continue;
		geometry0[i].typeConnections = g_realloc(geometry0[i].typeConnections,Natoms*sizeof(gint));
		for(j=iBegin;j<Natoms;j++) geometry0[i].typeConnections[j] = 0;
	}
	for(i=iBegin;i<Natoms;i++)
	{
		geometry[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<Natoms;j++) geometry[i].typeConnections[j] = 0;

		geometry0[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<Natoms;j++) geometry0[i].typeConnections[j] = 0;
	}
	for(i=iBegin;i<Natoms;i++)
	{
		for(j=i+1;j<Natoms;j++)
		{
			if(draw_lines_yes_no(i,j)) geometry[i].typeConnections[j]= 1;
			geometry[j].typeConnections[i] = geometry[i].typeConnections[j];
		}
	}
	geometry[toB].typeConnections[toBond] =1;
	geometry[toBond].typeConnections[geometry[toB].N-1]=1;

	if(!NumFatoms) g_free(NumFatoms);
	NumFatoms = NULL;

	NFatoms = 2;
	NumFatoms = g_malloc(2*sizeof(gint));
	NumFatoms[0] = geometry[toD].N;
	NumFatoms[1] = geometry[toDel].N;
	delete_all_selected_atoms();
	copy_connections(geometry0, geometry, Natoms);
	select_fragment(2);
				
	Ddef = FALSE;
	define_good_factor();
	reset_multiple_bonds();
	reset_charges_multiplicities();

	/* SetOriginAtCenter(NULL,0,NULL);*/
	set_statubar_pop_sel_atom();
	return 1;
}
/*****************************************************************************/
static gint insert_fragment(GtkWidget *widget,GdkEvent *event)
{
	if(Natoms>0 && NumProcheAtom<0)
	{
		FreeFragment(&Frag);
		return 0;
	}
	if(Frag.NAtoms<=0)
	{
		FreeFragment(&Frag);
		return 0;
	}
	if(atomToDelete>=0)  
	{
		return insert_fragment_connected_to_an_atom(
				get_indice(atomToDelete),
				get_indice(atomToBondTo),get_indice(angleTo));
	}
	else if(Frag.NAtoms>0 && Frag.atomToDelete<0)  
		return insert_fragment_without_delete_an_atom(widget,event);
	else if(Natoms==0)
		return insert_fragment_without_delete_an_atom(widget,event);
	return 0;
}
/*****************************************************************************/
void delete_one_atom(gint NumDel)
{

	gint i;
	gint j;
	gint* oldN = NULL;

	if(Natoms<1) return;
	copy_connections(geometry0, geometry, Natoms);
	i = NumDel;
	if(geometry0[i].typeConnections) g_free(geometry0[i].typeConnections);
	if(geometry[i].typeConnections) g_free(geometry[i].typeConnections);

	
	for (i=NumDel;i<(gint)Natoms-1;i++)
	{
		geometry0[i]=geometry0[i+1];
		geometry[i]=geometry[i+1];
	}
	oldN = g_malloc(Natoms*sizeof(gint));
	for (i=0;i<(gint)Natoms;i++) oldN[i] = geometry0[i].N-1;
	Natoms--;
	for(j=0;j<(gint)NFatoms;j++)
	{
		for (i=0;i<(gint)Natoms;i++)
			if(NumFatoms[j] ==(gint) geometry[i].N) { NumFatoms[j] = i+1; break;}
		if(i==Natoms)  NumFatoms[j] =-1;
	}
	for(j=0;j<(gint)NFatoms;j++)
		if( NumFatoms[j]<0)
		{
			for (i=j;i<(gint)NFatoms-1;i++)
				 NumFatoms[i]= NumFatoms[i+1];
			NFatoms--;
			j--;
		}

	for(j=0;j<2;j++)
		for (i=0;i<(gint)Natoms;i++)
			if(NumBatoms[j]==(gint) geometry[i].N) {NumBatoms[j] = i+1; break;}
	for (i=0;i<(gint)Natoms;i++)
	{
		geometry0[i].N = i+1;
		geometry[i].N = i+1;
	}
	/* in geometry0 : old connections , in geometry new connection */
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].typeConnections)
		{
			for(j=0;j<(gint)Natoms;j++)
			{
				geometry[i].typeConnections[j] = geometry0[i].typeConnections[oldN[j]];
			}
		}
	}
	if(oldN) g_free(oldN);
	copy_connections(geometry0, geometry, Natoms);
	if(Natoms>0)
	{
		geometry0 = g_realloc(geometry0,Natoms*sizeof(GeomDef));
		geometry = g_realloc(geometry,Natoms*sizeof(GeomDef)); 
	}
	else
	{
		if(geometry0) g_free(geometry0); geometry0 = NULL;
		if(geometry) g_free(geometry);
		geometry = NULL;
		Natoms = 0;
	}
	Ddef = FALSE;
	reset_charges_multiplicities();
}
/********************************************************************************/
void deleteHydrogensConnectedTo(gint n, gint nH)
{
	gint i;
	gint k = 0;
	gint ni;

	if(Natoms<1) return;

	add_geometry_to_fifo();
	for (i=0;i<(gint)Natoms;i++)
	{
		ni = geometry[i].N-1;
		if(geometry[n].typeConnections[ni] &&
		!strcmp(geometry[i].Prop.symbol,"H"))
		{
			delete_one_atom(i);
			k++;
			if(k>=nH) break;
		}
	}
}
/*****************************************************************************/
gint index_selected(gint Num)
{
	gint j;
	if(NFatoms<1 || !NumFatoms )
		return -1;

	for(j=0;j<(gint)NFatoms;j++)
		if(NumFatoms[j] == (gint)geometry[Num].N)
			return j;
	return -1;
}
/*****************************************************************************/
gboolean if_selected(gint Num)
{
	gint j;
	if(NFatoms<1 || !NumFatoms )
		return FALSE;

	for(j=0;j<(gint)NFatoms;j++)
		if(NumFatoms[j] == (gint)geometry[Num].N)
			return TRUE;
	return FALSE;
}
/*****************************************************************************/
void delete_all_selected_atoms()
{

	gint i;
	gint j;
	GeomDef tmp;
	gint* oldN = NULL;

	if(Natoms<1) return;

	copy_connections(geometry0, geometry, Natoms);

	for (i=0;i<(gint)Natoms-1;i++)
	{
		if(!if_selected(i)) continue;

		for(j=i+1;j<(gint)Natoms;j++)
			if(!if_selected(j))
			{
				tmp = geometry0[i];
				geometry0[i] = geometry0[j];
				geometry0[j] = tmp ;
				tmp = geometry[i];
				geometry[i] = geometry[j];
				geometry[j] = tmp ;
				break;
			}
	}
	oldN = g_malloc(Natoms*sizeof(gint));
	for (i=0;i<(gint)Natoms;i++) oldN[i] = geometry0[i].N-1;

	if(NFatoms>Natoms)
	{
		printf("internal error\n");
		Natoms = 0;
	}
	else
	{
		for (i=(gint)Natoms-NFatoms;i<(gint)Natoms;i++)
		{
			if(geometry0[i].typeConnections) g_free(geometry0[i].typeConnections);
			if(geometry[i].typeConnections) g_free(geometry[i].typeConnections);
		}
		Natoms-=NFatoms;
	}
	for(j=0;j<(gint)NBatoms;j++)
		for (i=0;i<(gint)Natoms;i++)
			if(NumBatoms[j] ==(gint) geometry[i].N) NumBatoms[j] = i+1;

	for (i=0;i<(gint)Natoms;i++)
	{
		geometry0[i].N = i+1;
		geometry[i].N = i+1;
	}
	/* in geometry0 : old connections , in geometry new connection */
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].typeConnections)
		{
			for(j=0;j<(gint)Natoms;j++)
			{
				geometry[i].typeConnections[j] = geometry0[i].typeConnections[oldN[j]];
			}
		}
	}
	if(oldN) g_free(oldN);
	copy_connections(geometry0, geometry, Natoms);

	if(Natoms>0)
	{
		geometry0 = g_realloc(geometry0,Natoms*sizeof(GeomDef));
		geometry = g_realloc(geometry,Natoms*sizeof(GeomDef)); 
	}
	else
	{
		if(geometry0)
			g_free(geometry0);
		geometry0 = NULL;
		if(geometry)
			g_free(geometry);
		geometry = NULL;
		Natoms = 0;
	}
	NFatoms = 0;
	if(NumFatoms)
		g_free(NumFatoms);
	NumFatoms = NULL;
	Ddef = FALSE;
	reset_charges_multiplicities();
}
/********************************************************************************/
static void rotate_frag_for_set_its_principal_axes_to_xyz(gboolean sel)
{
	gdouble **m0 = NULL;
	gdouble** minv;
	gint i,j;
	guint n;

	gdouble A[3];
	gdouble B[3];
	guint k;
	gdouble* X;
	gdouble* Y;
	gdouble* Z;
	gdouble axis1[3] = {1,0,0};
	gdouble axis2[3] = {0,1,0};
	gdouble axis3[3] = {0,0,1};
	gdouble C[3] = {0,0,0};
	gint nFrag = 0;

	if(Natoms<1) return;
	nFrag = compute_fragment_principal_axes(axis1,axis2,axis3,C,sel);
	if(nFrag <2 ) return;
	printf("nFrag = %d\n",nFrag);

	m0 = g_malloc(3*sizeof(gdouble*));
	X = g_malloc(Natoms*sizeof(gdouble));
	Y = g_malloc(Natoms*sizeof(gdouble));
	Z = g_malloc(Natoms*sizeof(gdouble));

	for(i=0;i<3;i++) m0[i] = g_malloc(3*sizeof(gdouble));


	m0[0][0] = axis1[0];
	m0[0][1] = axis1[1];
	m0[0][2] = axis1[2];

	m0[1][0] = axis2[0];
	m0[1][1] = axis2[1];
	m0[1][2] = axis2[2];

	m0[2][0] = axis3[0];
	m0[2][1] = axis3[1];
	m0[2][2] = axis3[2];

	minv = Inverse(m0,3,1e-7);

	for(n = 0;n<Natoms;n++)
	{
		if(if_selected(n)!=sel) continue;
		A[0] = geometry[n].X-C[0];
		A[1] = geometry[n].Y-C[1];
		A[2] = geometry[n].Z-C[2];

		for(j=0;j<3;j++)
		{
			B[j] = 0.0;
			for(k=0;k<3;k++)
				B[j] += minv[k][j]*A[k];
		}

		X[n] = B[0]+C[0];
		Y[n] = B[1]+C[1];
		Z[n] = B[2]+C[2];
	}
	for(n = 0;n<Natoms;n++)
	{
		if(if_selected(n)!=sel) 
		{
			geometry0[n].X = geometry[n].X;
			geometry0[n].Y = geometry[n].Y;
			geometry0[n].Z = geometry[n].Z;
		}
		else
		{
			geometry0[n].X = X[n];
			geometry0[n].Y = Y[n];
			geometry0[n].Z = Z[n];

			geometry[n].X = X[n];
			geometry[n].Y = Y[n];
			geometry[n].Z = Z[n];
		}
	}
	
	if(minv!=m0)
	{
	for(i=0;i<3;i++) if(minv[i]) g_free(minv[i]);
	if(minv) g_free(minv);
	}

	for(i=0;i<3;i++) if(m0[i]) g_free(m0[i]);
	if(m0) g_free(m0);

	if(X) g_free(X);
	if(Y) g_free(Y);
	if(Z) g_free(Z);
}
/*****************************************************************************/
void alignPrincipalAxesOfSelectedAtomsToXYZ()
{
	add_geometry_to_fifo();
	rotate_frag_for_set_its_principal_axes_to_xyz(TRUE);
	create_GeomXYZ_from_draw_grometry();
	init_quat(Quat);
	drawGeom();
	activate_edit_objects();
}
/*****************************************************************************/
void alignSelectedAndNotSelectedAtoms()
{
	add_geometry_to_fifo();
	move_the_center_of_selected_or_not_selected_atoms_to_origin(TRUE);
	move_the_center_of_selected_or_not_selected_atoms_to_origin(FALSE);
	create_GeomXYZ_from_draw_grometry();
	init_quat(Quat);
	drawGeom();
	activate_edit_objects();
}
/*****************************************************************************/
void deleteSelectedAtoms()
{
	add_geometry_to_fifo();
	delete_all_selected_atoms();
	create_GeomXYZ_from_draw_grometry();
	drawGeom();
	activate_edit_objects();
}
/*****************************************************************************/
void moveCenterOfSelectedAtomsToOrigin()
{
	add_geometry_to_fifo();
	move_the_center_of_selected_or_not_selected_atoms_to_origin(TRUE);
	create_GeomXYZ_from_draw_grometry();
	drawGeom();
	activate_edit_objects();
}
/*****************************************************************************/
void delete_hydrogen_atoms()
{

	gint i;
	gint j;
	GeomDef tmp;
	gint nA = 0;
	gint* oldN = NULL;

	add_geometry_to_fifo();

	for (i=0;i<(gint)Natoms-1;i++)
	{
		if(strcmp(geometry[i].Prop.symbol,"H")) continue;

		for(j=i+1;j<(gint)Natoms;j++)
			if(strcmp(geometry[j].Prop.symbol,"H"))
			{
				tmp = geometry0[i];
				geometry0[i] = geometry0[j];
				geometry0[j] = tmp ;
				tmp = geometry[i];
				geometry[i] = geometry[j];
				geometry[j] = tmp ;
				break;
			}
	}
	nA = 0;
	for (i=0;i<(gint)Natoms;i++)
		if(strcmp(geometry[i].Prop.symbol,"H")) nA++;


	if(Natoms>0) oldN = g_malloc(Natoms*sizeof(gint));
	for (i=0;i<(gint)Natoms;i++) oldN[i] = geometry0[i].N-1;
	Natoms = nA;

	for(j=0;j<(gint)NFatoms;j++)
	{
		for (i=0;i<(gint)Natoms;i++)
			if(NumFatoms[j] ==(gint) geometry[i].N) { NumFatoms[j] = i+1; break;}
		if(i==(gint)Natoms)  NumFatoms[j] =-1;
	}
	for(j=0;j<(gint)NFatoms;j++)
		if( NumFatoms[j]<0)
		{
			for (i=j;i<(gint)NFatoms-1;i++)
		 		NumFatoms[i]= NumFatoms[i+1];
			NFatoms--;
			j--;
		}
	for (i=0;i<(gint)Natoms;i++)
	{
		geometry0[i].N = i+1;
		geometry[i].N = i+1;
	}
	/* in geometry0 : old connections , in geometry new connection */
	for (i=0;i<(gint)Natoms;i++)
	{
		if(geometry[i].typeConnections)
		{
			for(j=0;j<(gint)Natoms;j++)
			{
				geometry[i].typeConnections[j] = geometry0[i].typeConnections[oldN[j]];
			}
		}
	}
	if(oldN) g_free(oldN);
	copy_connections(geometry0, geometry, Natoms);

	if(Natoms>0)
	{
		geometry0 = g_realloc(geometry0,Natoms*sizeof(GeomDef));
		geometry = g_realloc(geometry,Natoms*sizeof(GeomDef)); 
	}
	else
	{
		if(geometry0) g_free(geometry0);
		geometry0 = NULL;
		if(geometry) g_free(geometry);
		geometry = NULL;
		Natoms = 0;
	}
	Ddef = FALSE;
	/* reset_all_connections();*/
	reset_charges_multiplicities();
	create_GeomXYZ_from_draw_grometry();
	drawGeom();
	activate_edit_objects();
}
/********************************************************************************/
void deleteHydrogenAtoms()
{
	gchar *t =N_("Do you want to really remove all hydrogen atoms?");
	if(Natoms>0) Continue_YesNo(delete_hydrogen_atoms, NULL,t);
	else Message(_("No hydrogen atoms to remove\n"),_("Warning"),TRUE);
}
/*****************************************************************************/
void delete_selected_atoms()
{

	gint i;
	gint j;
	gboolean DelAll = FALSE;

	if(NumSelectedAtom<0)
		return;

	add_geometry_to_fifo();
	for(i=0;i<(gint)Natoms;i++)
		if((gint)i==NumSelectedAtom)
		{
			for(j = 0;j<(gint)NFatoms;j++)
				if(NumFatoms[j] == (gint)geometry[i].N)
					DelAll = TRUE;
			break;
		}
	if(DelAll==FALSE)
	{
		delete_one_atom(NumSelectedAtom);
		return;
	}
	delete_all_selected_atoms();
}
/*****************************************************************************/
void delete_selected_bond()
{

	if(NBatoms!=2) return;
	if(NBatoms>Natoms)
	{
		printf("internal error\n");
		Natoms = 0;
	}
	if(NumBatoms[0]>0 && NumBatoms[1]>0)
	{
		gint na = NumBatoms[0]-1;
		gint nb = NumBatoms[1]-1;
		gint ia = -1;
		gint ib = -1;
		gint i;
		for(i=0;i<(gint)Natoms;i++)
		if(geometry[i].N-1==na) { ia = i; break; }
		for(i=0;i<(gint)Natoms;i++)
		if(geometry[i].N-1==nb) { ib = i; break; }

		if(ia>=0 && ib>=0)
		{
			geometry[ia].typeConnections[nb] = 0;
			geometry[ib].typeConnections[na] = 0;
		}
		if(ia>=0 && ib>=0 &&  AdjustHydrogenAtoms)
			adjust_hydrogens_connected_to_atoms(ia,ib);
	}
	copy_connections(geometry0, geometry, Natoms);
	NBatoms = 0;
	NumBatoms[0] = NumBatoms[1] = -1;
}
/*****************************************************************************/
void change_selected_bond()
{

	if(NBatoms!=2) return;
	if(NBatoms>Natoms)
	{
		printf("internal error\n");
		Natoms = 0;
	}
	if(NumBatoms[0]>0 && NumBatoms[1]>0)
	{
		gint na = NumBatoms[0]-1;
		gint nb = NumBatoms[1]-1;
		gint newC = 1;
		gint i;
		gint ni=-1;
		gint nj=-1;
		gint nBondsA = 0;
		gint nBondsB = 0;

		for(i=0;i<(gint)Natoms;i++) if(geometry[i].N-1==na) { ni = i; break; }
		for(i=0;i<(gint)Natoms;i++) if(geometry[i].N-1==nb) { nj = i; break; }
		if(ni>=0)
		newC = geometry[ni].typeConnections[nb]+1;
		if(newC>3) newC = 1;
		if(newC==1)
		{
			if(ni>=0 && nj>=0) 
				geometry[ni].typeConnections[nb] = geometry[nj].typeConnections[na] = newC;
		}
		else
		{

			if(ni>=0 && nj>=0)
			for(i=0;i<(gint)Natoms;i++)
			{
				gint nk = geometry[i].N-1; 
				if(i==ni) continue;
				if(i==nj) continue;
				if(geometry[ni].typeConnections[nk]==0) continue;
				if(!strcmp(geometry[i].Prop.symbol, "H") && AdjustHydrogenAtoms)continue;
				nBondsA+=geometry[ni].typeConnections[nk];
			}
			if(ni>=0 && nj>=0)
			for(i=0;i<(gint)Natoms;i++)
			{
				gint nk = geometry[i].N-1; 
				if(i==ni) continue;
				if(i==nj) continue;
				if(geometry[nj].typeConnections[nk]==0) continue;
				if(!strcmp(geometry[i].Prop.symbol, "H") && AdjustHydrogenAtoms )continue;
				nBondsB+=geometry[nj].typeConnections[nk];
			}
			if(    !( ni>=0 && nj>=0 && 
				((gint)geometry[ni].Prop.maximumBondValence-(nBondsA+newC))>=0 &&
				((gint)geometry[nj].Prop.maximumBondValence-(nBondsB+newC))>=0 
				)
			) newC = 1;
			if(ni>=0 && nj>=0) 
				geometry[ni].typeConnections[nb] = geometry[nj].typeConnections[na] = newC;
		}
		if(ni>=0 && nj>=0 &&  AdjustHydrogenAtoms)
			adjust_hydrogens_connected_to_atoms(ni,nj);
	}
	copy_connections(geometry0, geometry, Natoms);
	NBatoms = 0;
	NumBatoms[0] = NumBatoms[1] = -1;
}
/*****************************************************************************/
void add_bond()
{
	if(NBatoms==1)
	{
		NBatoms = 0;
		NumBatoms[0] = NumBatoms[1] = -1;
	}
	if(NBatoms!=2) return;
	if(NBatoms>Natoms)
	{
		printf("internal error\n");
		Natoms = 0;
	}
	if(NumBatoms[0]>0 && NumBatoms[1]>0)
	{
		gint na = NumBatoms[0]-1;
		gint nb = NumBatoms[1]-1;
		gint newC = 1;
		gint i;
		gint ni=-1;
		gint nj=-1;
		gint nBondsA = 0;
		gint nBondsB = 0;
		{
			for(i=0;i<(gint)Natoms;i++)
			if(geometry[i].N-1==na)
			{
				ni = i;
				break;
			}
			if(ni>=0)
			for(i=0;i<(gint)Natoms;i++)
			if(geometry[i].N-1==nb)
			{
				nj = i;
				break;
			}
			if(ni>=0 && nj>=0)
			for(i=0;i<(gint)Natoms;i++)
			{
				gint nk = geometry[i].N-1; 
				if(i==ni) continue;
				if(i==nj) continue;
				if(geometry[ni].typeConnections[nk]==0) continue;
				if(!strcmp(geometry[i].Prop.symbol, "H") && AdjustHydrogenAtoms )continue;
				nBondsA+=geometry[ni].typeConnections[nk];
			}
			if(ni>=0 && nj>=0)
			for(i=0;i<(gint)Natoms;i++)
			{
				gint nk = geometry[i].N-1; 
				if(i==ni) continue;
				if(i==nj) continue;
				if(geometry[nj].typeConnections[nk]==0) continue;
				if(!strcmp(geometry[i].Prop.symbol, "H"))continue;
				nBondsB+=geometry[nj].typeConnections[nk];
			}
			if(    !( ni>=0 && nj>=0 && 
				((gint)geometry[ni].Prop.maximumBondValence-(nBondsA+newC))>=0 &&
				((gint)geometry[nj].Prop.maximumBondValence-(nBondsB+newC))>=0 
				)
			) newC = 0;
			if(ni>=0 && nj>=0) 
				geometry[ni].typeConnections[nb] = geometry[nj].typeConnections[na] = newC;
		}
		if(ni>=0 && nj>=0 &&  AdjustHydrogenAtoms)
			adjust_hydrogens_connected_to_atoms(ni,nj);
	}
	copy_connections(geometry0, geometry, Natoms);
	NBatoms = 0;
	NumBatoms[0] = NumBatoms[1] = -1;
}
/*****************************************************************************/
void rotation_geometry_quat(gdouble m[4][4])
{
	gdouble A[3];
	gdouble B[3];
	guint i,j,k;

	for (i=0;i<Natoms;i++)
	{
		A[0] = geometry0[i].X;
		A[1] = geometry0[i].Y;
		A[2] = geometry0[i].Z;
		for(j=0;j<3;j++)
		{
			B[j] = 0.0;
			for(k=0;k<3;k++)
				B[j] += m[k][j]*A[k];
		}
		geometry[i].X=B[0];
		geometry[i].Y=B[1];
		geometry[i].Z=B[2];
	}
	if(Ddef)
	{
		
	for (i=0;i<NDIVDIPOLE;i++)
	{
	A[0] = dipole0[i][0];
	A[1] = dipole0[i][1];
	A[2] = dipole0[i][2];
	for(j=0;j<3;j++)
	{
		B[j] = 0.0;
		for(k=0;k<3;k++)
			B[j] += m[k][j]*A[k];
	}
	dipole[i][0]=B[0];
	dipole[i][1]=B[1];
	dipole[i][2]=B[2];
	}
	}

	sort_with_zaxis();
	define_coefs_pers();
}
/*****************************************************************************/
void rotation_atom_quat(gint i,gdouble m[4][4])
{
	gdouble A[3];
	gdouble B[3];
	guint j,k;

	A[0] = CSselectedAtom[0] ;
	A[1] = CSselectedAtom[1] ;
	A[2] = CSselectedAtom[2] ;
	for(j=0;j<3;j++)
	{
		B[j] = 0.0;
		for(k=0;k<3;k++)
			B[j] += m[k][j]*A[k];
	}
	geometry0[i].X=B[0];
	geometry0[i].Y=B[1];
	geometry0[i].Z=B[2];
}
/*****************************************************************************/
void rotation(double a,double b,double angle,double *ap,double *bp)
{
	double cosangle = cos(angle/180*PI);
	double sinangle = sin(angle/180*PI);
	
	*ap = a*cosangle - b *sinangle;
	*bp = b*cosangle + a *sinangle;
}
/*****************************************************************************/
void set_to_yaxis()
{
	double A;
	double B;
	double angle=90;
	double Xmax,Xmin;
	double Ymax,Ymin;
	double Zmax,Zmin;
	guint i;
	guint axe;


        if(Natoms<1)
          return;

	Xmax =geometry[0].X;
	Ymax =geometry[0].Y;
	Zmax =geometry[0].Z;

	Xmin =geometry[0].X;
	Ymin =geometry[0].Y;
	Zmin =geometry[0].Z;

	for (i = 1;i<Natoms;i++)
	{
	if (geometry[i].X>Xmax)
			  Xmax =geometry[i].X;
	if (geometry[i].X<Xmin)
			  Xmin =geometry[i].X;

	if (geometry[i].Y>Ymax)
			   Ymax =geometry[i].Y;
	if (geometry[i].Y<Ymin)
			  Ymin =geometry[i].Y;

    	if (geometry[i].Z>Zmax)
			Zmax =geometry[i].Z;	
	if (geometry[i].Z<Zmin)
			Zmin =geometry[i].Z;

	}
        axe=0;
        if(fabs(Ymax-Ymin)<=fabs(Xmax-Xmin))
        {
        	if(fabs(Zmax-Zmin)<fabs(Ymax-Ymin))
                    axe=2;
                 else
		    axe=1;
         }
         else
        {
        	if(fabs(Zmax-Zmin)<fabs(Xmax-Xmin))
                    axe=2;
         }
         
        
	if(axe==0)
	{
		for (i=0;i<Natoms;i++)
		{
			A = geometry[i].Z;
			B = geometry[i].X;
			rotation(A,B,angle,&geometry[i].Z,&geometry[i].X);
		}
		for (i=0;i<NDIVDIPOLE;i++)
		{
			A = dipole[i][2];
			B = dipole[i][0];
			rotation(A,B,angle,&dipole[i][2],&dipole[i][0]);
		}
	}
        else if(axe==1)
	{
		for (i=0;i<Natoms;i++)
		{
			A = geometry[i].Y;
			B = geometry[i].Z;
			rotation(A,B,angle,&geometry[i].Y,&geometry[i].Z);
		}
		for (i=0;i<NDIVDIPOLE;i++)
		{
			A = dipole[i][1];
			B = dipole[i][2];
			rotation(A,B,angle,&dipole[i][1],&dipole[i][2]);
		}
	}
}
/*****************************************************************************/
void define_geometry()
{
        guint i;
        guint j;
        gdouble X0[3];
	gint nC = 0;

	if(geometry != NULL) 
	{
                for (i=0;i<Natoms;i++)
			g_free(geometry[i].Prop.symbol);
		        
		g_free(geometry);
		geometry = NULL;
	}
        if(MethodeGeom == GEOM_IS_XYZ) Natoms = NcentersXYZ;
        else Natoms = NcentersZmat;

	geometry =g_malloc(Natoms*sizeof(GeomDef));

        if((MethodeGeom == GEOM_IS_XYZ) && (GeomXYZ  != NULL) ) define_geometry_from_xyz();
        if((MethodeGeom == GEOM_IS_ZMAT) && (Geom  != NULL) ) 
	{
		if(!define_geometry_from_zmat()) Message(_("Error in  conversion\n Zmatix to xyz "),_("Warning"),TRUE);
		for(i=0;i<Natoms;i++)
		{
			gint j;
			geometry[i].typeConnections = g_malloc(Natoms*sizeof(gint));
			for(j=0;j<(gint)Natoms;j++) geometry[i].typeConnections[j] = 0;
		}
	}

         if(Units == 1 ) geometry_in_au();
/* Center of molecule */
        if(Natoms<1) return;
        for(i=0;i<3;i++) X0[i] = 0.0;
        for(i=0;i<Natoms;i++)
	{
	X0[0] +=geometry[i].X;
	X0[1] +=geometry[i].Y;
	X0[2] +=geometry[i].Z;
	}
        for(i=0;i<3;i++)
		X0[i] /= Natoms;
        for(i=0;i<Natoms;i++)
	{
	geometry[i].X -= X0[0];
	geometry[i].Y -= X0[1];
	geometry[i].Z -= X0[2]; 
	} 

	Ddef = Dipole.def;
	if(Ddef)
	{
		gdouble step[3] ={Dipole.value[0]/(NDIVDIPOLE-1),Dipole.value[1]/(NDIVDIPOLE-1),Dipole.value[2]/(NDIVDIPOLE-1)};
		dipole[0][0] = -X0[0];
		dipole[0][1] = -X0[1];
		dipole[0][2] = -X0[2];
		for(i=1;i<NDIVDIPOLE;i++)
			for(j=0;j<3;j++)
				dipole[i][j] = dipole[0][j]+step[j]*i;
	}
        for(i=0;i<3;i++) Orig[i] = X0[i];

	set_to_yaxis();

	if(geometry0) g_free(geometry0);
	geometry0 = NULL;

	geometry0 =g_malloc(Natoms*sizeof(GeomDef));
	for(i=0;i<Natoms;i++)
	{
	geometry0[i].X = geometry[i].X;
	geometry0[i].Y = geometry[i].Y;
	geometry0[i].Z = geometry[i].Z; 
	geometry0[i].N = geometry[i].N; 
        geometry0[i].typeConnections = NULL;
	geometry0[i].Prop = prop_atom_get(geometry[i].Prop.symbol);
	geometry0[i].mmType = g_strdup(geometry[i].mmType);
	geometry0[i].pdbType = g_strdup(geometry[i].pdbType);
	geometry0[i].Layer = geometry[i].Layer;
	geometry0[i].Variable = geometry[i].Variable;
	geometry0[i].Residue = g_strdup(geometry[i].Residue);
	geometry0[i].ResidueNumber = geometry[i].ResidueNumber;
	geometry0[i].show = geometry[i].show;
	geometry0[i].Charge = geometry[i].Charge;
	geometry0[i].typeConnections = NULL;
	} 
	if(Ddef)
	{
	for(i=0;i<NDIVDIPOLE;i++)
		for(j=0;j<3;j++)
		{
			dipole0[i][j] = dipole[0][j] + (dipole[i][j]-dipole[0][j])*factordipole;
			dipole00[i][j] = dipole0[i][j];
		}
	}
	
	nC = 0;
	for(i=0;i<Natoms;i++)
	{
		geometry0[i].typeConnections = g_malloc(Natoms*sizeof(gint));
		for(j=0;j<(gint)Natoms;j++) geometry0[i].typeConnections[j] = 0;
	}
	copy_connections(geometry0,geometry,Natoms);
	nC = 0;
	for(i=0;i<Natoms;i++) 
		for(j=0;j<(gint)Natoms;j++) nC+= geometry[i].typeConnections[j];
	if(nC==0) reset_all_connections();
	sort_with_zaxis();
	define_coord_maxmin();
	define_coefs_pers();
	free_text_to_draw();
	/* define_good_trans();*/

	/* reset_charges_multiplicities();*/
	reset_spin_of_electrons();

}
/*****************************************************************************/
guint get_num_min_rayonIJ(guint i,guint j)
{	
	gint rmin;
	guint kmin;

	rmin = geometry[i].Rayon;
        kmin = i;
	if(rmin > geometry[j].Rayon)
		{
			rmin = geometry[j].Rayon;
			kmin =j;
		}
	return kmin;

}
/*****************************************************************************/
void draw_triangle(gint x1,gint y1,gint x2,gint y2,gint x3,gint y3, GdkColor colori)
{
	GdkColormap *colormap;

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap,&colori,FALSE,TRUE);
	gdk_gc_set_foreground(gc,&colori);
	gdk_gc_set_line_attributes(gc,1,GDK_LINE_SOLID,GDK_CAP_NOT_LAST,GDK_JOIN_MITER);
	gabedit_cairo_triangle(cr, GeomDrawingArea, gc, x1, y1, x2, y2,  x3, y3);
	if(crExport)  
	gabedit_cairo_triangle(crExport, GeomDrawingArea, gc, x1, y1, x2, y2,  x3, y3);
}
/*****************************************************************************/
GdkColor get_color_string(guint i)
{
 GdkColor color;
 color.pixel = 0;
 if (ShadMode) 
 {
  color.red   =(gushort)(65535-geometry[i].Prop.color.red*geometry[i].Coefpers);
  color.green =(gushort)(65535-geometry[i].Prop.color.green*geometry[i].Coefpers);
  color.blue  =(gushort)(65535-geometry[i].Prop.color.blue*geometry[i].Coefpers);
 }
 else
 {
  color.red   =FontsStyleLabel.TextColor.red;
  color.green =FontsStyleLabel.TextColor.green;
  color.blue  =FontsStyleLabel.TextColor.blue;
 }
 return color;
}
/*****************************************************************************/
void draw_distance(gint i,gint j,gint x0,gint y0)
{
	GdkColormap *colormap;
        GdkColor color;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);

        Point A;
        Point B;

	A.C[0]=geometry[i].X;
	A.C[1]=geometry[i].Y;
	A.C[2]=geometry[i].Z;
        
	B.C[0]=geometry[j].X;
	B.C[1]=geometry[j].Y;
	B.C[2]=geometry[j].Z;

        color = get_color_string(i);
	
   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);

	gdk_gc_set_foreground(gc,&color);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, x0,y0,get_distance_points(A,B,TRUE),TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, x0,y0,get_distance_points(A,B,TRUE),TRUE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);

}
/*****************************************************************************/
void draw_line(gdouble x1,gdouble y1,gdouble x2,gdouble y2,GdkColor colori,gint epaisseuri, gboolean round)
{
	GdkColormap *colormap;
        gint epaisseur=epaisseuri;

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);

	gdk_colormap_alloc_color(colormap,&colori,FALSE,TRUE);
	gdk_gc_set_foreground(gc,&colori);
	if(round)
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	else
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_NOT_LAST,GDK_JOIN_ROUND);
	gabedit_cairo_line(cr, GeomDrawingArea, gc, x1,y1,x2,y2);
	if(crExport) gabedit_cairo_line(crExport, GeomDrawingArea, gc, x1,y1,x2,y2);
}
/*****************************************************************************/
static void draw_line_hbond(gint x1,gint y1,gint x2,gint y2,GdkColor color,gint epaisseur)
{
	GdkColormap *colormap;

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap,&color,FALSE,TRUE);
	gdk_gc_set_foreground(gc,&color);
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_ON_OFF_DASH,GDK_CAP_NOT_LAST,GDK_JOIN_MITER);

	gabedit_cairo_line(cr, GeomDrawingArea, gc, x1,y1,x2,y2);
	if(crExport) gabedit_cairo_line(crExport, GeomDrawingArea, gc, x1,y1,x2,y2);
}
/*****************************************************************************/
static void draw_line2_hbond(gint x1,gint y1,gint x2,gint y2, gint i, gint j, GdkColor color1,GdkColor color2, gint epaisseur)
{
	gdouble x0;
	gdouble y0;

        gdouble poid1;
        gdouble poid2;
        gdouble poid;
        gushort rayon;
        gdouble xp;
        gdouble yp;
        gdouble k;
	gint nj = geometry[j].N-1;

	GdkColor colorblack;

	colorblack.red = 0;
	colorblack.green = 0;
	colorblack.blue = 0;
	if(epaisseur<1) epaisseur = 1;

	if((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)<epaisseur*epaisseur) return;

        if ( !stick_mode() && geometry[i].Layer != LOW_LAYER )
        { 
                rayon = get_rayon(i);

                 k= ((gdouble)rayon*(gdouble)rayon-(gdouble)epaisseur*(gdouble)epaisseur/4.0);
		 if(geometry[i].typeConnections[nj]==2) k= ((gdouble)rayon*(gdouble)rayon-(gdouble)epaisseur*(gdouble)epaisseur*9.0/4);
		 if(geometry[i].typeConnections[nj]==3) k= ((gdouble)rayon*(gdouble)rayon-(gdouble)epaisseur*(gdouble)epaisseur*25.0/4);

		if(k>0 &&(( (gdouble)(x2-x1)*(gdouble)(x2-x1)+(gdouble)(y2-y1)*(gdouble)(y2-y1) )>2))
                k = (sqrt(k))/(gdouble)(sqrt( (gdouble)(x2-x1)*(gdouble)(x2-x1)+(gdouble)(y2-y1)*(gdouble)(y2-y1) ) );     
                else
                 k=0.0;

                xp = x1 + k *(x2-x1);
                yp = y1 + k *(y2-y1);   


        }
        else
        {
	      xp = x1;
              yp = y1;
        }

        poid1 = geometry[i].Prop.covalentRadii+geometry[i].Prop.radii;
        poid2 = geometry[j].Prop.covalentRadii+geometry[j].Prop.radii;
        poid = poid1 + poid2 ;
	x0=((x1*poid2+x2*poid1)/poid);
	y0=((y1*poid2+y2*poid1)/poid);

	if(color1.red==color2.red && color1.green==color2.green && color1.blue==color2.blue)
	{
		draw_line_hbond(xp,yp,x2,y2,color1,epaisseur);
	}
	else
	{
	draw_line_hbond(xp,yp,x0,y0,color1,epaisseur);
	draw_line_hbond(x0,y0,x2,y2,color2,epaisseur); 
	}

	if(DrawDistance) draw_distance(i,j,x0,y0); 
}
/*****************************************************************************/
void draw_anneau(gint xi,gint yi,gint rayoni,GdkColor colori)
{
	GdkColormap *colormap;

        colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap,&colori,FALSE,TRUE);
	gdk_gc_set_foreground(gc,&colori);
	gdk_gc_set_line_attributes(gc,4,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gdk_gc_set_fill(gc,GDK_STIPPLED);

	gabedit_cairo_cercle(cr, GeomDrawingArea, gc, xi, yi,rayoni);
	if(crExport) gabedit_cairo_cercle(crExport, GeomDrawingArea, gc, xi, yi,rayoni);
	gdk_gc_set_fill(gc,GDK_SOLID);
}
/*****************************************************************************/
gboolean draw_lines_yes_no(guint i,guint j)
{
  gdouble distance;
  gdouble rcut;
  gdouble x,y,z;
  x = geometry[i].X-geometry[j].X;
  y = geometry[i].Y-geometry[j].Y;
  z = geometry[i].Z-geometry[j].Z;
  distance = x*x+y*y+z*z;
  rcut = geometry[i].Prop.covalentRadii+geometry[j].Prop.covalentRadii;
  rcut = rcut* rcut;

  if(distance<rcut)
	return TRUE;
  else 
	return FALSE;
}
/*****************************************************************************/
void draw_symb(guint epaisseur,guint i)
{
	GdkColormap *colormap;
	GdkColor color;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
	gchar* t= g_strdup_printf("%s", geometry[i].Prop.symbol);
        

	color = get_color_string(i);
   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);

	gdk_gc_set_foreground(gc,&color);
        if(epaisseur == 0)epaisseur =1;
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,t,TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,t,TRUE,TRUE);


	if(font_desc) pango_font_description_free (font_desc);
	g_free(t);

}
/*****************************************************************************/
void draw_numb(guint epaisseur,guint i)
{
	GdkColormap *colormap;
        GdkColor color;
        gchar *temp;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
        temp = g_strdup_printf("%d",geometry[i].N);

        color = get_color_string(i);

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);
	gdk_gc_set_foreground(gc,&color);
        if(epaisseur == 0)epaisseur =1;
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);

        g_free(temp);
}
/*****************************************************************************/
void draw_layer(guint epaisseur,guint i)
{
	GdkColormap *colormap;
	GdkColor color;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
	gchar* t= NULL;

	if(geometry[i].Layer==LOW_LAYER) t= g_strdup_printf("L");
	else if(geometry[i].Layer==MEDIUM_LAYER) t= g_strdup_printf("M");
	else t= g_strdup_printf(" ");

	color = get_color_string(i);
   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);

	gdk_gc_set_foreground(gc,&color);
        if(epaisseur == 0)epaisseur =1;
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,t,TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,t,TRUE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);
	g_free(t);

}
/*****************************************************************************/
void draw_mmtyp(guint epaisseur,guint i)
{
	GdkColormap *colormap;
	GdkColor color;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
	gchar* t= g_strdup_printf("%s", geometry[i].mmType);
        

	color = get_color_string(i);
   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);

	gdk_gc_set_foreground(gc,&color);
        if(epaisseur == 0)epaisseur =1;
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,t,TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,t,TRUE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);
	g_free(t);

}
/*****************************************************************************/
void draw_pdbtyp(guint epaisseur,guint i)
{
	GdkColormap *colormap;
	GdkColor color;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
	gchar* t= g_strdup_printf("%s", geometry[i].pdbType);
        

	color = get_color_string(i);
   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);

	gdk_gc_set_foreground(gc,&color);
        if(epaisseur == 0)epaisseur =1;
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,t,TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,t,TRUE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);
	g_free(t);

}
/*****************************************************************************/
void draw_numb_symb(guint epaisseur,guint i)
{
	GdkColormap *colormap;
        GdkColor color;
        gchar *temp;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
        temp = g_strdup_printf("%s[%d]",geometry[i].Prop.symbol,geometry[i].N);

        color = get_color_string(i);

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);
	gdk_gc_set_foreground(gc,&color);
        if(epaisseur == 0)epaisseur =1;
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);
        g_free(temp);
}
/*****************************************************************************/
void draw_charge(guint epaisseur,guint i)
{
	GdkColormap *colormap;
        GdkColor color;
        gchar *temp;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
        temp = g_strdup_printf("%0.3f",geometry[i].Charge);

        color = get_color_string(i);
   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);
	gdk_gc_set_foreground(gc,&color);
        if(epaisseur == 0)epaisseur =1;
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);

        g_free(temp);
}
/*****************************************************************************/
void draw_symb_charge(guint epaisseur,guint i)
{
	GdkColormap *colormap;
        GdkColor color;
        gchar *temp;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
        temp = g_strdup_printf("%s[%0.3f]",geometry[i].Prop.symbol,geometry[i].Charge);

        color = get_color_string(i);

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);
	gdk_gc_set_foreground(gc,&color);
        if(epaisseur == 0)epaisseur =1;
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);

        g_free(temp);
}
/*****************************************************************************/
void draw_numb_charge(guint epaisseur,guint i)
{
	GdkColormap *colormap;
        GdkColor color;
        gchar *temp;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
        temp = g_strdup_printf("%d[%0.3f]",geometry[i].N,geometry[i].Charge);

        color = get_color_string(i);

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);
	gdk_gc_set_foreground(gc,&color);
        if(epaisseur == 0)epaisseur =1;
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);

        g_free(temp);
}
/*****************************************************************************/
void draw_residues(guint epaisseur,guint i)
{
	GdkColormap *colormap;
        GdkColor color;
        gchar *temp;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
        temp = g_strdup_printf("%s[%d]",geometry[i].Residue,geometry[i].ResidueNumber+1);

        color = get_color_string(i);

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);
	gdk_gc_set_foreground(gc,&color);
        if(epaisseur == 0)epaisseur =1;
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);

        g_free(temp);
}
/*****************************************************************************/
void draw_coordinates(guint epaisseur,guint i)
{
	GdkColormap *colormap;
        GdkColor color;
        gchar *temp;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);
        temp = g_strdup_printf("%s[%0.3f,%0.3f,%0.3f] Ang",
			geometry0[i].Prop.symbol,
			geometry0[i].X*BOHR_TO_ANG,
			geometry0[i].Y*BOHR_TO_ANG,
			geometry0[i].Z*BOHR_TO_ANG);

        color = get_color_string(i);

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);
	gdk_gc_set_foreground(gc,&color);
        if(epaisseur == 0)epaisseur =1;
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, geometry[i].Xi,geometry[i].Yi,temp,TRUE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);

        g_free(temp);
}
/*****************************************************************************/
void draw_label(guint epaisseur,guint i)
{
   switch(LabelOption)
   {
   case LABELSYMB: draw_symb(epaisseur,i);break;
   case LABELNUMB: draw_numb(epaisseur,i);break;
   case LABELMMTYP: draw_mmtyp(epaisseur,i);break;
   case LABELPDBTYP: draw_pdbtyp(epaisseur,i);break;
   case LABELLAYER: draw_layer(epaisseur,i);break;
   case LABELSYMBNUMB: draw_numb_symb(epaisseur,i);break;
   case LABELCHARGE: draw_charge(epaisseur,i);break;
   case LABELSYMBCHARGE: draw_symb_charge(epaisseur,i);break;
   case LABELNUMBCHARGE: draw_numb_charge(epaisseur,i);break;
   case LABELRESIDUES: draw_residues(epaisseur,i);break;
   case LABELCOORDINATES: draw_coordinates(epaisseur,i);break;
   }
}
/*****************************************************************************/
void draw_line2(gint epaisseur,guint i,guint j,gint x1,gint y1,gint x2,gint y2,
				GdkColor color1,GdkColor color2, gboolean hideDistance)
{
	gdouble x0;
	gdouble y0;

        gdouble poid1;
        gdouble poid2;
        gdouble poid;
        gushort rayon;
        gdouble xp;
        gdouble yp;
        gdouble k;
	gint nj = geometry[j].N-1;

	GdkColor colorblack;

	colorblack.red = 0;
	colorblack.green = 0;
	colorblack.blue = 0;

	if((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)<epaisseur*epaisseur) return;

        if ( !stick_mode() && geometry[i].Layer != LOW_LAYER )
        { 
                rayon = get_rayon(i);

                 k= ((gdouble)rayon*(gdouble)rayon-(gdouble)epaisseur*(gdouble)epaisseur/4.0);
		 if(geometry[i].typeConnections[nj]==2 && showMultipleBonds) k= ((gdouble)rayon*(gdouble)rayon-(gdouble)epaisseur*(gdouble)epaisseur*9.0/4);
		 if(geometry[i].typeConnections[nj]==3 && showMultipleBonds) k= ((gdouble)rayon*(gdouble)rayon-(gdouble)epaisseur*(gdouble)epaisseur*25.0/4);

		if(k>0 &&(( (gdouble)(x2-x1)*(gdouble)(x2-x1)+(gdouble)(y2-y1)*(gdouble)(y2-y1) )>2))
                k = (sqrt(k))/(gdouble)(sqrt( (gdouble)(x2-x1)*(gdouble)(x2-x1)+(gdouble)(y2-y1)*(gdouble)(y2-y1) ) );     
                else
                 k=0.0;

                xp = x1 + k *(x2-x1);
                yp = y1 + k *(y2-y1);   


        }
        else
        {
	      xp = x1;
              yp = y1;
        }
	gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);
	if(CartoonMode && !(buttonpress&&Natoms>MAT)) draw_line(xp,yp,x2,y2,colorblack,epaisseur+2,TRUE);

        poid1 = geometry[i].Prop.covalentRadii+geometry[i].Prop.radii;
        poid2 = geometry[j].Prop.covalentRadii+geometry[j].Prop.radii;
        poid = poid1 + poid2 ;
	x0=((x1*poid2+x2*poid1)/poid);
	y0=((y1*poid2+y2*poid1)/poid);

	if(color1.red==color2.red && color1.green==color2.green && color1.blue==color2.blue)
	{
		if(LightMode)
		{
        		if (!stick_mode())
			{
			color1.red = color1.green = color1.blue = 0;
			draw_line(xp,yp,x2,y2,color1,epaisseur,TRUE);
			}
			else
			{
			GdkColormap *colormap;
   			colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
			gdk_colormap_alloc_color(colormap,&color1,FALSE,TRUE);
			gdk_gc_set_foreground(gc,&color1);
			gdk_colormap_alloc_color(colormap,&color2,FALSE,TRUE);
			if(!stick_mode())
			gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_NOT_LAST,GDK_JOIN_ROUND);
			else gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);

			gabedit_cairo_line_gradient(cr, GeomDrawingArea, gc, color1,  color2,  xp, yp, x2, y2);
			if(crExport) gabedit_cairo_line_gradient(crExport, GeomDrawingArea, gc, color1,  color2,  xp, yp, x2, y2);
			}
		}
		else
		draw_line(xp,yp,x2,y2,color1,epaisseur,TRUE);
	}
	else
	{
		if(LightMode)
		{
        		if (!stick_mode())
			{
			color1.red = color1.green = color1.blue = 0;
			draw_line(xp,yp,x2,y2,color1,epaisseur,TRUE);
			}
			else
			{
			GdkColormap *colormap;
   			colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
			gdk_colormap_alloc_color(colormap,&color1,FALSE,TRUE);
			gdk_gc_set_foreground(gc,&color1);
			gdk_colormap_alloc_color(colormap,&color2,FALSE,TRUE);
        		if (!stick_mode())
			gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_NOT_LAST,GDK_JOIN_ROUND);
			else
			gdk_gc_set_line_attributes(gc,epaisseur,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);

			gabedit_cairo_line_gradient(cr, GeomDrawingArea, gc, color1,  color2,  xp, yp, x2, y2);
			if(crExport)
			gabedit_cairo_line_gradient(crExport, GeomDrawingArea, gc, color1,  color2,  xp, yp, x2, y2);
			}
		}
		else
		{
			draw_line(xp,yp,x0,y0,color1,epaisseur,TRUE);
			draw_line(x0,y0,x2,y2,color2,epaisseur,TRUE); 
		}
	}


	if(DrawDistance && !hideDistance)
                 draw_distance(i,j,x0,y0); 
}
/*****************************************************************************/
void draw_cercle(gint xi,gint yi,gint rayoni,GdkColor colori, gboolean fill, gboolean cartoon, gboolean lighting)
{
	GdkColormap *colormap;
        gint x=xi,y=yi,rayon=rayoni;
        GdkColor colorblack;

        colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
	if(cartoon)
	{
       		colorblack.red = 0;
       		colorblack.green = 0;
       		colorblack.blue = 0;


        	gdk_colormap_alloc_color(colormap,&colorblack,FALSE,TRUE);
		gdk_gc_set_line_attributes(gc,1,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);

		rayon = rayoni+1;
		gdk_gc_set_foreground(gc,&colorblack);
		gdk_gc_set_fill(gc,GDK_STIPPLED);
		gabedit_cairo_cercle(cr, GeomDrawingArea, gc, x, y,rayon);
		if(crExport) gabedit_cairo_cercle(crExport, GeomDrawingArea, gc, x, y,rayon);
		gdk_gc_set_fill(gc,GDK_SOLID);
	}

	if(fill)
	{
		rayon = rayoni;
        	gdk_colormap_alloc_color(colormap,&colori,FALSE,TRUE);
		gdk_gc_set_foreground(gc,&colori);
		gdk_gc_set_fill(gc,GDK_SOLID);

    		if (lighting) 
		{
			gabedit_cairo_cercle_gradient(cr, GeomDrawingArea, gc, x, y,rayon);
			if(crExport) gabedit_cairo_cercle_gradient(crExport, GeomDrawingArea, gc, x, y,rayon);
		}
		else 
		{
			gabedit_cairo_cercle(cr, GeomDrawingArea, gc, x, y,rayon);
			if(crExport) gabedit_cairo_cercle(crExport, GeomDrawingArea, gc, x, y,rayon);
		}
	}
}
/*****************************************************************************/
void draw_arc(gint xi,gint yi,gint rayoni,gdouble angle1, gdouble angle2, gdouble scale1, gdouble scale2, GdkColor colori)
{
	GdkColormap *colormap;
        gint x=xi,y=yi,rayon=rayoni;
        GdkColor colorblack;

        colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
	if(CartoonMode)
	{
		gint lw = 2;
       		colorblack.red = 0;
       		colorblack.green = 0;
       		colorblack.blue = 0;


        	gdk_colormap_alloc_color(colormap,&colorblack,FALSE,TRUE);
		gdk_gc_set_line_attributes(gc,lw,GDK_LINE_SOLID,GDK_CAP_ROUND,GDK_JOIN_ROUND);

		/* rayon = rayoni+1;*/
		rayon = rayoni;
		gdk_gc_set_foreground(gc,&colorblack);
		gabedit_cairo_arc(cr, GeomDrawingArea, gc, x, y,rayon, angle1, angle2, scale1, scale2);
		if(crExport) gabedit_cairo_arc(crExport, GeomDrawingArea, gc, x, y,rayon, angle1, angle2,scale1, scale2);
	}
	/*

	rayon = rayoni;
        gdk_colormap_alloc_color(colormap,&colori,FALSE,TRUE);
	gdk_gc_set_foreground(gc,&colori);
	gdk_gc_set_fill(gc,GDK_SOLID);

    	if (LightMode) 
	{
		gabedit_cairo_arc(cr, GeomDrawingArea, gc, x, y,rayon,angle1, angle2, scale1, scale2);
		if(crExport) gabedit_cairo_arc(crExport, GeomDrawingArea, gc, x, y,rayon,angle1, angle2, scale1, scale2);
	}
	else 
	{
		gabedit_cairo_arc(cr, GeomDrawingArea, gc, x, y,rayon,angle1, angle2, scale1, scale2);
		if(crExport) gabedit_cairo_arc(crExport, GeomDrawingArea, gc, x, y,rayon,angle1, angle2,scale1, scale2);
	}
	*/
}
/************************************************************************/
void getOptimalCiCj(gint i, gint j, gdouble* Ci, gdouble* Cj, gdouble* C0)
{
	C0[0] = 0;
	C0[1] = 0;
	C0[2] = 0;

	Ci[0] = geometry[i].X;
	Ci[1] = geometry[i].Y;
	Ci[2] = geometry[i].Z;

	Cj[0] = geometry[j].X;
	Cj[1] = geometry[j].Y;
	Cj[2] = geometry[j].Z;

/* serach a one none hydrogen atom connected to i or j atoms */
	if(geometry[i].typeConnections)
	{
		gint l;
		gint nl;
		for(l=0, nl = geometry[l].N-1;l<Natoms;l++, nl = geometry[l].N-1)
			if(l != j && l != i &&  geometry[i].typeConnections[nl]>0 && strcmp(geometry[l].Prop.symbol,"H"))
			{
				C0[0] = geometry[l].X;
				C0[1] = geometry[l].Y;
				C0[2] = geometry[l].Z;
				/* printf("---%s\n",geometry[l].Prop.symbol);*/
				return;
			}
	}
	if(geometry[j].typeConnections)
	{
		gint l;
		gint nl;
		for(l=0, nl = geometry[l].N-1;l<Natoms;l++, nl = geometry[l].N-1)
			if(l != j && l != i &&  geometry[j].typeConnections[nl]>0  && strcmp(geometry[l].Prop.symbol,"H"))
			{
				C0[0] = geometry[l].X;
				C0[1] = geometry[l].Y;
				C0[2] = geometry[l].Z;
				/* printf("--%s\n",geometry[l].Prop.symbol);*/
				return;
			}
	}
	if(geometry[i].typeConnections)
	{
		gint l;
		gint nl;
		for(l=0, nl = geometry[l].N-1;l<Natoms;l++, nl = geometry[l].N-1)
			if(l != j && l != i &&  geometry[i].typeConnections[nl]>0)
			{
				C0[0] = geometry[l].X;
				C0[1] = geometry[l].Y;
				C0[2] = geometry[l].Z;
				/* printf("%s\n",geometry[l].Prop.symbol);*/
				return;
			}
	}
	if(geometry[j].typeConnections)
	{
		gint l;
		gint nl;
		for(l=0, nl = geometry[l].N-1;l<Natoms;l++, nl = geometry[l].N-1)
			if(l != j && l != i &&  geometry[j].typeConnections[nl]>0)
			{
				C0[0] = geometry[l].X;
				C0[1] = geometry[l].Y;
				C0[2] = geometry[l].Z;
				/* printf("%s\n",geometry[l].Prop.symbol);*/
				return;
			}
	}
}
/*****************************************************************************/
void draw_ball(gint xi,gint yi,gint rayoni,GdkColor colori)
{
	if(!(buttonpress&&Natoms>MAT)) draw_cercle(xi,yi,rayoni,colori, TRUE, CartoonMode, LightMode);
	else draw_cercle(xi,yi,rayoni,colori, TRUE, FALSE, LightMode);

	if(OrtepMode && !(buttonpress&&Natoms>MAT))
	{
    		draw_arc(xi,yi,rayoni,0, M_PI, 1.0, 0.5, colori);
    		draw_arc(xi,yi,rayoni,M_PI/2,3*M_PI/2,0.5,1.0, colori);
	}
}
/*****************************************************************************/
void drawGeom_byLayer()
{	
	guint i;
	guint j;
	guint k;
	gint epaisseur;
        gushort rayon;
	GdkColor color1;
	GdkColor color2;
	GdkColor colorRed;
	GdkColor colorGreen;
	GdkColor colorBlue;
	GdkColor colorYellow;
	GdkColor colorFrag;
	gint ni;
	gint nj;
	gint epMin = -1;
	gint epMinH = -1;
	gint epMinM = -1;
	gint epMinL = -1;

	colorRed.red   = 40000;
	colorRed.green = 0;
	colorRed.blue  = 0;
	colorRed.pixel  = 0;

	colorGreen.red   = 0;
	colorGreen.green = 40000;
	colorGreen.blue  = 0;

	colorBlue.red   = 0;
	colorBlue.green = 0;
	colorBlue.blue  = 40000;

	colorYellow.red   = 40000;
	colorYellow.green = 40000;
	colorYellow.blue  = 0;

	colorFrag = colorGreen;

	if(Natoms<1) return;

	for(i=0;i<Natoms;i++)
	if((gint)i==NumSelectedAtom)
	{
		for(j = 0;j<NFatoms;j++)
			if(NumFatoms[j] == (gint)geometry[i].N) colorFrag = colorRed;
		break;
	}
        if(ButtonPressed && OperationType==ROTLOCFRAG) colorFrag = colorRed;
        if(ButtonPressed && OperationType==ROTZLOCFRAG) colorFrag = colorRed;

	define_coord_ecran();

	for(i=0;i<Natoms;i++)
	{
		gint e = get_rayon(i);
		if(geometry[i].Layer == LOW_LAYER)
		{
			if(epMinL<0) epMinL=e;
			else if(e<epMinL) epMinL=e;
		}
		else
		if(geometry[i].Layer == MEDIUM_LAYER)
		{
			if(epMinM<0) epMinM=e;
			else if(e<epMinM) epMinM=e;
		}
		else
		{
			if(epMinH<0) epMinH=e;
			else if(e<epMinH) epMinH=e;
		}
	}
	if(epMinH>0) epMin = epMinH;
	else if(epMinM>0) epMin = epMinM;
	else if(epMinL>0) epMin = epMinL;
	else epMin = 2;

	epMin *= factorstick;
	if(epMin<1) epMin = 1;

	for(i=0;i<Natoms-1;i++)
	{
		ni = geometry[i].N-1;
		if(!geometry[i].show)
		{
			if(ShowDipole) for(j = 0;j<NDIVDIPOLE;j++) if(Ndipole[j]==(gint)i) drawGeom_dipole(j);
			continue;
		}
                rayon = get_rayon(i);
		color1 = geometry[i].Prop.color;  
    		if (ShadMode) set_color_shad(&color1,i);

		if(geometry0[i].Layer != LOW_LAYER)
		{
			draw_ball(geometry[i].Xi,geometry[i].Yi,rayon,color1);
		}
		if((gint)i==NumSelectedAtom) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorRed);
		else
		{
			if(NSA[0]>-1 && (gint)geometry[i].N == NSA[0]) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorRed);
			if(NSA[1]>-1 && (gint)geometry[i].N == NSA[1]) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorGreen);
			if(NSA[2]>-1 && (gint)geometry[i].N == NSA[2]) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorBlue);
			if(NSA[3]>-1 && (gint)geometry[i].N == NSA[3]) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorYellow);
		}
		if(OperationType == MEASURE)
		for(j = 0;j<4;j++)
		if(NumSelAtoms[j] == (gint)geometry[i].N) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorGreen);
        	switch(OperationType)
		{
			case ADDFRAGMENT :
				if(atomToDelete == (gint)geometry[i].N)
	 				draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorRed);
				if(atomToBondTo == (gint)geometry[i].N)
	 				draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorGreen);
				if(angleTo == (gint)geometry[i].N)
	 				draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorBlue);
			break;
			case SELECTOBJECTS :
			case SELECTFRAG :
			case SELECTRESIDUE :
			case DELETEOBJECTS :
			case DELETEFRAG :
			case ROTLOCFRAG :
			case ROTZLOCFRAG :
			case MOVEFRAG :
			for(j = 0;j<NFatoms;j++)
				if(NumFatoms[j] == (gint)geometry[i].N)
	 				draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorFrag);
			break;
			case CUTBOND :
			for(j = 0;j<NBatoms;j++)
				if(NumBatoms[j] == (gint)geometry[i].N)
	 				draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorRed);
			break;
			case CHANGEBOND :
			case ADDATOMSBOND :
			for(j = 0;j<NBatoms;j++)
				if(NumBatoms[j] == (gint)geometry[i].N)
	 				draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorFrag);
			break;
			default :break;
		}

		for(j=i+1, nj = geometry[j].N-1;j<Natoms;j++,nj = geometry[j].N-1)
                if(geometry[i].typeConnections[nj]>0)
		{
			if(!geometry[j].show) continue;
			gint split[2] = {0,0};
			gdouble ab[] = {0,0};
			k = i;
			if(geometry0[i].Layer == geometry0[j].Layer ) k =get_num_min_rayonIJ(i,j);
			else 
			{
				if(geometry0[i].Layer == MEDIUM_LAYER) k = i;
				if(geometry0[j].Layer == MEDIUM_LAYER) k = j;
				if(geometry0[i].Layer == LOW_LAYER) k = i;
				if(geometry0[j].Layer == LOW_LAYER) k = j;
			}
			/* epaisseur = get_rayon(k)*factorstick;*/
			epaisseur = epMin;
			if(geometry[i].Layer == LOW_LAYER || geometry[j].Layer == LOW_LAYER) epaisseur=(gint)(epaisseur/2.5);
			else if(geometry[i].Layer == MEDIUM_LAYER || geometry[j].Layer == MEDIUM_LAYER) epaisseur=(gint)(epaisseur/1.2); 

			if(epaisseur<3) epaisseur = 3;

			color2 = geometry[j].Prop.color;  
    			if (ShadMode) set_color_shad(&color2,j);
			if(geometry[i].typeConnections[nj]>1 && showMultipleBonds)
			{
				gdouble m = 0;
				ab[0] = geometry[j].Yi-geometry[i].Yi;
				ab[1] = -geometry[j].Xi+geometry[i].Xi;
				m = sqrt(ab[0]*ab[0]+ab[1]*ab[1]);
				if(m !=0)
				{
					ab[0] /= m;
					ab[1] /= m;

				}
			}
			if(geometry[i].typeConnections[nj]==3 && showMultipleBonds)
			{
				gint x1;
				gint x2;
				gint y1;
				gint y2;

				split[0] = (gint)(ab[0]*(epaisseur+10)/5);
				split[1] = (gint)(ab[1]*(epaisseur+10)/5);

				x1 = geometry[i].Xi-2*split[0];
				x2 = geometry[j].Xi-2*split[0];
				y1 = geometry[i].Yi-2*split[1];
				y2 = geometry[j].Yi-2*split[1];
				draw_line2(epaisseur/5,i,j,x1,y1, x2, y2, color1,color2,TRUE);

				x1 = geometry[i].Xi;
				x2 = geometry[j].Xi;
				y1 = geometry[i].Yi;
				y2 = geometry[j].Yi;
				draw_line2(epaisseur/5,i,j,x1,y1, x2, y2, color1,color2,TRUE);

				x1 = geometry[i].Xi+2*split[0];
				x2 = geometry[j].Xi+2*split[0];
				y1 = geometry[i].Yi+2*split[1];
				y2 = geometry[j].Yi+2*split[1];
				draw_line2(epaisseur/5,i,j,x1,y1, x2, y2, color1,color2,FALSE);

			}
			else if(geometry[i].typeConnections[nj]==2 && showMultipleBonds)
			{
				gint x1;
				gint x2;
				gint y1;
				gint y2;

				split[0] = (gint)(ab[0]*(epaisseur+3)/3);
				split[1] = (gint)(ab[1]*(epaisseur+3)/3);

				x1 = geometry[i].Xi-split[0];
				x2 = geometry[j].Xi-split[0];
				y1 = geometry[i].Yi-split[1];
				y2 = geometry[j].Yi-split[1];
				draw_line2(epaisseur/3,i,j,x1,y1, x2, y2, color1,color2,TRUE);

				x1 = geometry[i].Xi+split[0];
				x2 = geometry[j].Xi+split[0];
				y1 = geometry[i].Yi+split[1];
				y2 = geometry[j].Yi+split[1];
				draw_line2(epaisseur/3,i,j,x1,y1, x2, y2, color1,color2,FALSE);
			}
			else
			draw_line2(epaisseur,i,j,geometry[i].Xi,geometry[i].Yi,
						geometry[j].Xi,geometry[j].Yi,
						color1,color2,FALSE);
        		if((OperationType==CUTBOND || OperationType==CHANGEBOND) 
			&& NBatoms==2 && NumBatoms[0]>0 && NumBatoms[1]>0) 
			{
				gint na = NumBatoms[0];
				gint nb = NumBatoms[1];
				if( 
			    		(na == (gint)geometry[i].N && geometry[i].show &&
			    		nb == (gint)geometry[j].N && geometry[j].show)
					||
			    		(nb == (gint)geometry[i].N && geometry[i].show &&
			    		na == (gint)geometry[j].N && geometry[j].show)
					)
				{
					if(OperationType==CUTBOND)
					draw_line2(epaisseur,i,j,geometry[i].Xi,geometry[i].Yi,
						geometry[j].Xi,geometry[j].Yi,
						colorRed,colorRed,FALSE);
					else
					draw_line2(epaisseur,i,j,geometry[i].Xi,geometry[i].Yi,
						geometry[j].Xi,geometry[j].Yi,
						colorFrag,colorFrag,FALSE);
				}
			}
		}
		else
		{
			k =get_num_min_rayonIJ(i,j);
			epaisseur = get_rayon(k);
        		if(OperationType==ADDATOMSBOND 
			&& NFatoms==2 && NumFatoms[0]>0 && NumFatoms[1]>0) 
			{
				gint na = NumFatoms[0];
				gint nb = NumFatoms[1];
			if( 
			    (na == (gint)geometry[i].N && geometry[i].show &&
			    nb == (gint)geometry[j].N && geometry[j].show)
					||
			    (nb == (gint)geometry[i].N && geometry[i].show &&
			    na == (gint)geometry[j].N && geometry[j].show)
					)
			draw_line2(epaisseur,i,j,geometry[i].Xi,geometry[i].Yi,
						geometry[j].Xi,geometry[j].Yi,
						colorFrag,colorFrag,FALSE);
			}
			if(geometry[i].show && geometry[j].show && ShowHBonds && geometry[i].typeConnections[nj]==-1)
			{
				epaisseur = 6;
                		epaisseur*=factorstick;
				color1 = geometry[i].Prop.color;  
				color2 = geometry[j].Prop.color;  
				draw_line2_hbond(geometry[i].Xi,geometry[i].Yi, geometry[j].Xi,geometry[j].Yi, i,  j,  color1, color2,  epaisseur);
			}
		}
    		if (LabelOption != 0) draw_label(5,i);
		if(ShowDipole) for(j = 0;j<NDIVDIPOLE;j++) if(Ndipole[j]==(gint)i) drawGeom_dipole(j);
	}
        i=Natoms-1;
        rayon = get_rayon(i);
	color1 = geometry[i].Prop.color;  
    	if (ShadMode) set_color_shad(&color1,i);

	if(geometry[i].Layer != LOW_LAYER && geometry[i].show)
	{
		draw_ball(geometry[i].Xi,geometry[i].Yi,rayon,color1);
	}
	if((gint)i==NumSelectedAtom && geometry[i].show) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorRed);
	else
	{
		if(NSA[0]>-1 && (gint)geometry[i].N == NSA[0] && geometry[i].show) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorRed);
		if(NSA[1]>-1 && (gint)geometry[i].N == NSA[1] && geometry[i].show) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorGreen);
		if(NSA[2]>-1 && (gint)geometry[i].N == NSA[2] && geometry[i].show) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorBlue);
		if(NSA[3]>-1 && (gint)geometry[i].N == NSA[2] && geometry[i].show) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorYellow);
	}
	if(OperationType == MEASURE && geometry[i].show)
	for(j = 0;j<4;j++)
		if(NumSelAtoms[j] == (gint)geometry[i].N )
 			draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorGreen);
        switch(OperationType)
	{
		case ADDFRAGMENT :
			if(atomToDelete == (gint)geometry[i].N)
	 			draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorRed);
			if(atomToBondTo == (gint)geometry[i].N)
	 			draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorGreen);
			if(angleTo == (gint)geometry[i].N)
	 			draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorBlue);
		break;
		case SELECTOBJECTS :
		case SELECTFRAG :
		case SELECTRESIDUE :
		case DELETEFRAG :
		case DELETEOBJECTS :
		case ROTLOCFRAG :
		case ROTZLOCFRAG :
		case MOVEFRAG :
		for(j = 0;j<NFatoms;j++)
		if(NumFatoms[j] == (gint)geometry[i].N && geometry[i].show)
 			draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorFrag);
		break;
		case CUTBOND :
		for(j = 0;j<NBatoms;j++)
		if(NumBatoms[j] == (gint)geometry[i].N && geometry[i].show)
 			draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorRed);
		break;
		case CHANGEBOND :
		case ADDATOMSBOND :
		for(j = 0;j<NBatoms;j++)
		if(NumBatoms[j] == (gint)geometry[i].N && geometry[i].show)
 			draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorFrag);
		break;
		default :break;
	}

	if(ShowDipole) for(j = 0;j<NDIVDIPOLE;j++) if(Ndipole[j]==(gint)i) drawGeom_dipole(j);
    	if (LabelOption != 0 && geometry[i].show) draw_label(5,i);

	
}
/*****************************************************************************/
void redefine_dipole()
{
        guint i;
        guint j;

	if(!Ddef)
		return;
	{
		for(i=0;i<NDIVDIPOLE;i++)
			for(j=0;j<3;j++)
				dipole0[i][j] = dipole00[0][j] + (dipole00[i][j]-dipole00[0][j])*factordipole;
	}
}
/*****************************************************************************/
void drawGeom_stick()
{	
	guint i;
	guint j;
	guint k;
	gint epaisseur;
	GdkColor color1;
	GdkColor color2;
	GdkColor colorRed;
	GdkColor colorGreen;
	GdkColor colorBlue;
	GdkColor colorYellow;
	GdkColor colorFrag;
    	gushort rayon;
	gboolean* FreeAtoms = g_malloc(Natoms*sizeof(gboolean));
	gint ni, nj;

	colorRed.red   = 40000;
	colorRed.green = 0;
	colorRed.blue  = 0;
	colorRed.pixel  = 0;

	colorGreen.red   = 0;
	colorGreen.green = 40000;
	colorGreen.blue  = 0;

	colorBlue.red   = 0;
	colorBlue.green = 0;
	colorBlue.blue  = 40000;

	colorYellow.red   = 40000;
	colorYellow.green = 40000;
	colorYellow.blue  = 0;

	colorFrag = colorGreen;


	for(i=0;i<Natoms;i++)
	if((gint)i==NumSelectedAtom)
	{
		for(j = 0;j<NFatoms;j++)
		if(NumFatoms[j] == (gint)geometry[i].N) colorFrag = colorRed;
		break;
	}
        if(ButtonPressed && OperationType==ROTLOCFRAG) colorFrag = colorRed;
        if(ButtonPressed && OperationType==ROTZLOCFRAG) colorFrag = colorRed;

	for(i=0;i<Natoms;i++) FreeAtoms[i] = TRUE;

	define_coord_ecran();

	for(i=0;i<Natoms;i++)
        {
		ni = geometry[i].N-1;
		if(!geometry[i].show)
		{
			if(ShowDipole) for(j = 0;j<NDIVDIPOLE;j++) if(Ndipole[j]==(gint)i) drawGeom_dipole(j);
			continue;
		}
		k = -1;
		for(j=i+1, nj = geometry[j].N-1;j<Natoms;j++, nj = geometry[j].N-1)
                if(geometry[i].typeConnections[nj]>0)
		{
			if(!geometry[j].show) continue;
			gint split[2] = {0,0};
			gdouble ab[] = {0,0};
			if(geometry[i].typeConnections[nj]>1 && showMultipleBonds)
			{
				gdouble m = 0;
				ab[0] = geometry[j].Yi-geometry[i].Yi;
				ab[1] = -geometry[j].Xi+geometry[i].Xi;
				m = sqrt(ab[0]*ab[0]+ab[1]*ab[1]);
				if(m !=0)
				{
					ab[0] /= m;
					ab[1] /= m;

				}
			}
			FreeAtoms[j] = FALSE;
			FreeAtoms[i] = FALSE;
			k =get_num_min_rayonIJ(i,j);
			epaisseur = (gint) (geometry[k].Rayon/2*factorstick);

			epaisseur = get_epaisseur();


    			if (PersMode) 
               		 	epaisseur =(gint)(geometry[k].Coefpers*epaisseur);
			color1 = geometry[i].Prop.color;  
			color2 = geometry[j].Prop.color;  
    			if (ShadMode) 
			{
				set_color_shad(&color1,i);
				set_color_shad(&color2,j);
			}
			draw_line2(epaisseur,i,j,geometry[i].Xi,geometry[i].Yi, geometry[j].Xi,geometry[j].Yi, color1,color2,FALSE);
			if(geometry[i].typeConnections[nj]==2 && showMultipleBonds)
			{
				gint x1;
				gint x2;
				gint y1;
				gint y2;
				split[0] = (gint)(ab[0]*epaisseur*1.5);
				split[1] = (gint)(ab[1]*epaisseur*1.5);

				x1 = geometry[i].Xi-split[0]-split[1];
				y1 = geometry[i].Yi-split[1]+split[0];
				x2 = geometry[j].Xi-split[0]+split[1];
				y2 = geometry[j].Yi-split[1]-split[0];
				draw_line2(epaisseur/3,i,j,x1, y1, x2, y2, color1,color2,TRUE);
			}
			if(geometry[i].typeConnections[nj]==3 && showMultipleBonds)
			{
				gint x1;
				gint x2;
				gint y1;
				gint y2;
				split[0] = (gint)(ab[0]*epaisseur*1.5);
				split[1] = (gint)(ab[1]*epaisseur*1.5);

				x1 = geometry[i].Xi-split[0]-split[1];
				y1 = geometry[i].Yi-split[1]+split[0];
				x2 = geometry[j].Xi-split[0]+split[1];
				y2 = geometry[j].Yi-split[1]-split[0];
				draw_line2(epaisseur/2,i,j,x1, y1, x2, y2, color1,color2,TRUE);
				x1 = geometry[i].Xi+split[0]-split[1];
				y1 = geometry[i].Yi+split[1]+split[0];
				x2 = geometry[j].Xi+split[0]+split[1];
				y2 = geometry[j].Yi+split[1]-split[0];
				draw_line2(epaisseur/2,i,j,x1, y1, x2, y2, color1,color2,TRUE);
			}
        		if((OperationType==CUTBOND || OperationType==CHANGEBOND) 
			&& NBatoms==2 && NumBatoms[0]>0 && NumBatoms[1]>0) 
			{
				gint na = NumBatoms[0];
				gint nb = NumBatoms[1];
				if( 
			    		(na == (gint)geometry[i].N && geometry[i].show &&
			    		nb == (gint)geometry[j].N && geometry[j].show)
					||
			    		(nb == (gint)geometry[i].N && geometry[i].show &&
			    		na == (gint)geometry[j].N && geometry[j].show)
					)
				{
					if(OperationType==CUTBOND)
					draw_line2(epaisseur,i,j,geometry[i].Xi,geometry[i].Yi,
						geometry[j].Xi,geometry[j].Yi,
						colorRed,colorRed,FALSE);
					else
					draw_line2(epaisseur,i,j,geometry[i].Xi,geometry[i].Yi,
						geometry[j].Xi,geometry[j].Yi,
						colorFrag,colorFrag,FALSE);
				}
			}
		}
		else
		{
			k =get_num_min_rayonIJ(i,j);
			epaisseur = (gint) (geometry[k].Rayon/2*factorstick);
			epaisseur = get_epaisseur();
    			if (PersMode) epaisseur =(gint)(geometry[k].Coefpers*epaisseur);
        		if(OperationType==ADDATOMSBOND && NFatoms==2 && NumFatoms[0]>0 && NumFatoms[1]>0) 
			{
				gint na = NumFatoms[0];
				gint nb = NumFatoms[1];
			if( 
			    (na == (gint)geometry[i].N && geometry[i].show &&
			    nb == (gint)geometry[j].N && geometry[j].show)
					||
			    (nb == (gint)geometry[i].N && geometry[i].show &&
			    na == (gint)geometry[j].N && geometry[j].show)
					)
			draw_line2(epaisseur,i,j,geometry[i].Xi,geometry[i].Yi,
						geometry[j].Xi,geometry[j].Yi,
						colorFrag,colorFrag,FALSE);
			}
			if(geometry[i].show && geometry[j].show && ShowHBonds && geometry[i].typeConnections[nj]==-1)
			{
				epaisseur = 3;
                		epaisseur*=factorstick;
				color1 = geometry[i].Prop.color;  
				color2 = geometry[j].Prop.color;  
				draw_line2_hbond(geometry[i].Xi,geometry[i].Yi, geometry[j].Xi,geometry[j].Yi, i,  j,  color1, color2,  epaisseur);
			}
		}
		if(FreeAtoms[i])
		{
        		rayon =(gushort)(geometry[i].Rayon*factorball)/2;
    			if (PersMode) rayon =(gushort)(geometry[i].Coefpers*geometry[i].Rayon*factorball)/2;
			color1 = geometry[i].Prop.color;  
    			if (ShadMode) set_color_shad(&color1,i);
			if(rayon<5) rayon = 5;
			draw_ball(geometry[i].Xi,geometry[i].Yi,rayon,color1);
		}
		else
		{
        		rayon =(gushort)(geometry[i].Rayon*factorstick)/3;
    			if (PersMode) rayon =(gushort)(geometry[i].Coefpers*geometry[i].Rayon*factorstick)/3;
			if(rayon<5) rayon = 5;
		}
		if((gint)i==NumSelectedAtom) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorRed);
		else
		{
			if(NSA[0]>-1 && (gint)geometry[i].N == NSA[0]) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorRed);
			if(NSA[1]>-1 && (gint)geometry[i].N == NSA[1]) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorGreen);
			if(NSA[2]>-1 && (gint)geometry[i].N == NSA[2]) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorBlue);
			if(NSA[3]>-1 && (gint)geometry[i].N == NSA[3]) draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorYellow);
		}
		if(OperationType == MEASURE)
		for(j = 0;j<4;j++)
		if(NumSelAtoms[j] == (gint)geometry[i].N)
	 		draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorGreen);

        	switch(OperationType)
		{
			case ADDFRAGMENT :
				if(atomToDelete == (gint)geometry[i].N)
	 				draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorRed);
				if(atomToBondTo == (gint)geometry[i].N)
	 				draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorGreen);
				if(angleTo == (gint)geometry[i].N)
	 				draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorBlue);
			break;
			case SELECTOBJECTS :
			case SELECTFRAG :
			case SELECTRESIDUE :
			case DELETEFRAG :
			case DELETEOBJECTS :
			case ROTLOCFRAG :
			case ROTZLOCFRAG :
			case MOVEFRAG :
			for(j = 0;j<NFatoms;j++)
			if(NumFatoms[j] == (gint)geometry[i].N)
	 			draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorFrag);
			break;
			case CUTBOND :
			for(j = 0;j<NBatoms;j++)
			if(NumBatoms[j] == (gint)geometry[i].N)
	 			draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorRed);
			break;
			case CHANGEBOND :
			case ADDATOMSBOND :
			for(j = 0;j<NBatoms;j++)
			if(NumBatoms[j] == (gint)geometry[i].N)
	 			draw_anneau(geometry[i].Xi,geometry[i].Yi,rayon,colorFrag);
			break;
			default : break;
		}



		if(ShowDipole) for(j = 0;j<NDIVDIPOLE;j++) if(Ndipole[j]==(gint)i) drawGeom_dipole(j);
    		if (LabelOption != 0) draw_label(5,i);
        }
    	if (LabelOption != 0 && geometry[Natoms-1].show) draw_label(5,Natoms-1);
	g_free(FreeAtoms);
	
}
/*****************************************************************************/
void draw_dipole(gint x0,gint y0)
{
	GdkColormap *colormap;
        GdkColor color;
	gchar* t;
	gdouble d = 0.0;
	gint i;
 	PangoFontDescription *font_desc = pango_font_description_from_string (FontsStyleLabel.fontname);

	for(i=0;i<3;i++)
		d += Dipole.value[i]*Dipole.value[i];

	t = g_strdup_printf("%0.3f D",sqrt(d)*AUTODEB);

        color = get_color_string(0);

   	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);

        gdk_colormap_alloc_color(colormap, &color, FALSE, TRUE);
	gdk_gc_set_foreground(gc,&color);
	gabedit_cairo_string(cr, GeomDrawingArea, font_desc, gc, x0,y0,t,TRUE,TRUE);
	if(crExport)  
	gabedit_cairo_string(crExport, GeomDrawingArea, font_desc, gc, x0,y0,t,TRUE,TRUE);

	if(font_desc) pango_font_description_free (font_desc);

}
/*****************************************************************************/
void calcul_ndipole()
{
	gint i;
	gint j;

	for (j = 0;j<NDIVDIPOLE;j++)
		Ndipole[j] = 0; 

	for (i = 1;i<(gint)Natoms;i++)
	{
		for (j = 0;j<NDIVDIPOLE;j++)
			if(dipole[j][2]>geometry[i].Z)
				Ndipole[j] = i;
	}

}
/*****************************************************************************/
void drawGeom_dipole(gint i)
{	
	gint epaisseur;
	GdkColor color;

    	if(!Ddef) return;

	epaisseur = (gint) (10*factorstick);
        epaisseur = (gint) (epaisseur/2.0); 
        epaisseur =(gint)(epaisseur*Dipole.radius/0.25);
	if(i<NDIVDIPOLE-1)
	{
	color.red = Dipole.color[0];
	color.green = Dipole.color[1];
	color.blue = Dipole.color[2];
	draw_line(DXi[i],DYi[i],DXi[i+1],DYi[i+1],color,epaisseur,TRUE);
	if(DrawDipole && i==NDIVDIPOLE/2)
	 	draw_dipole(DXi[i],DYi[i]);
	}
	else
	{
		color.red = Dipole.color[0]/1.1;
		color.green = Dipole.color[1]/1.1;
		color.blue = Dipole.color[2]/1.1;
 		draw_cercle(DXi[i],DYi[i],epaisseur,color,TRUE, FALSE,FALSE);
	}
}
/*****************************************************************************/
void buildRotation()
{
	gdouble m[4][4];
	build_rotmatrix(m,Quat);
	rotation_geometry_quat(m);
}
/*****************************************************************************/
void drawGeom()
{
	if(!GeomDrawingArea) return;
     	pixmap_init(GeomDrawingArea);


     	if(Natoms<1)
	{
		redraw();
		return;
	}

	if(strToDraw)
		draw_text(strToDraw);
	
	buildRotation();


       	if (!stick_mode())
		drawGeom_byLayer();
	else 
   		drawGeom_stick();

	redraw();
}
/*****************************************************************************/
void rafresh_drawing()
{
  
	guint i;
	HideShowMeasure(MeasureIsHide);
	i= gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBookDraw));
	define_geometry();
	gtk_notebook_remove_page((GtkNotebook *)NoteBookDraw,0);
	vboxmeasure =AddNoteBookPage(NoteBookDraw,_("Measure"));
	AddMeasure(GeomDlg,vboxmeasure);

	gtk_widget_hide_all(NoteBookDraw);
	gtk_widget_show_all(NoteBookDraw);
	gtk_notebook_set_current_page((GtkNotebook*)NoteBookDraw,i);


	drawGeom();
	change_of_center(NULL,NULL);
}
/********************************************************************************/
static void copyCoordinates2to1(GeomDef *geom1, GeomDef *geom2)
{
	gint i;
	if(!geom1) return;
	if(!geom2) return;
	for (i=0;i<Natoms;i++)
	{
		geom1[i].X = geom2[i].X;
		geom1[i].Y = geom2[i].Y;
		geom1[i].Z = geom2[i].Z;
	}
}
/******************************************************************************/
void rafresh_window_geom()
{
         if(GeomDrawingArea != NULL)
         {
                RebuildGeom = TRUE;
                copyCoordinates2to1(geometry, geometry0);
                redraw(GeomDrawingArea);
         }
	drawGeom();

     /* rafresh_drawing();*/
}
/*****************************************************************************/
void multi_geometry_by_factor(gdouble fa0)
{
        guint i;

        for(i=0;i<Natoms;i++)
	{
          geometry[i].X *= fa0;
          geometry[i].Y *= fa0;
          geometry[i].Z *= fa0;
	}
	define_coord_maxmin();
}
/*****************************************************************************/
void multi_geometry_by_a0(GtkWidget *win, gpointer d)
{
	multi_geometry_by_factor(BOHR_TO_ANG);
	drawGeom();
}
/*****************************************************************************/
void divide_geometry_by_a0(GtkWidget *win, gpointer d)
{
 multi_geometry_by_factor(1.0/BOHR_TO_ANG);
 drawGeom();
}
/*****************************************************************************/
void factor_default(GtkWidget *win,gpointer d)
{
	factor =1.0;
	drawGeom();
}
/*****************************************************************************/
void factor_stick_default(GtkWidget *win,gpointer d)
{
	factorstick =1.0;
	drawGeom();
}
/*****************************************************************************/
void factor_ball_default(GtkWidget *win,gpointer d)
{
	factorball =1.0;
	drawGeom();
}
/*****************************************************************************/
void factor_dipole_default(GtkWidget *win,gpointer d)
{
	factordipole =1.0;
	redefine_dipole();
	drawGeom();
}
/*****************************************************************************/
void factor_all_default(GtkWidget *win,gpointer d)
{
	factorball =1.0;
	factorstick =1.0;
	factor =1.0;
	factordipole =1.0;
	redefine_dipole();
	SetOperation(NULL,     	CENTER);
}
/*****************************************************************************/
void set_back_color_black()
{

	if(BackColor)
        {
		gdk_color_free(BackColor);
		BackColor=NULL;
        }
        gdk_draw_rectangle (pixmap,
                      GeomDrawingArea->style->black_gc,
                      TRUE,
                      0, 0,
                      GeomDrawingArea->allocation.width,
                      GeomDrawingArea->allocation.height);    
        drawGeom();
}
/*****************************************************************************/
void set_back_color(GtkColorSelection *Sel,gpointer *d)
{
	GdkColor color;
	GdkColormap *colormap;

	gtk_color_selection_get_current_color(Sel, &color);
	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
	
        BackColor = gdk_color_copy(&color);
        gdk_colormap_alloc_color(colormap,&color,FALSE,TRUE);

	gdk_gc_set_foreground(gc,&color);

        gdk_draw_rectangle (pixmap,
                      gc,
                      TRUE,
                      0, 0,
                      GeomDrawingArea->allocation.width,
                      GeomDrawingArea->allocation.height);    
        drawGeom();

}
/*****************************************************************************/
void set_back_color_grey()
{
	GdkColor color;
	GdkColormap *colormap;

	color.red = 80*257;
	color.green = 80*257;
	color.blue = 80*257;
	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
	
        BackColor = gdk_color_copy(&color);
        gdk_colormap_alloc_color(colormap,&color,FALSE,TRUE);

	gdk_gc_set_foreground(gc,&color);

        gdk_draw_rectangle (pixmap,
                      gc,
                      TRUE,
                      0, 0,
                      GeomDrawingArea->allocation.width,
                      GeomDrawingArea->allocation.height);    
        drawGeom();
}
/*****************************************************************************/
void set_back_color_default()
{
	static gint first = 0;
	GdkColor color;
	GdkColormap *colormap;

	if(!BackColor) 
	{
		set_back_color_grey();
		return;
	}
	color.red = BackColor->red;
	color.green = BackColor->green;
	color.blue = BackColor->blue;
	colormap  = gdk_drawable_get_colormap(GeomDrawingArea->window);
	
	if(first==0)
	{
		g_free(BackColor);
		first = 1;
	}
        BackColor = gdk_color_copy(&color);
        gdk_colormap_alloc_color(colormap,&color,FALSE,TRUE);

	gdk_gc_set_foreground(gc,&color);

        gdk_draw_rectangle (pixmap,
                      gc,
                      TRUE,
                      0, 0,
                      GeomDrawingArea->allocation.width,
                      GeomDrawingArea->allocation.height);    
        drawGeom();
}
/*****************************************************************************/
void open_color_dlg(GtkWidget *win,gpointer *DrawingArea)
{

	GtkColorSelectionDialog *ColorDlg;
	ColorDlg = 
		(GtkColorSelectionDialog *)gtk_color_selection_dialog_new(
		_("Set Background Color"));
	gtk_window_set_modal (GTK_WINDOW (ColorDlg), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(ColorDlg),GTK_WINDOW(Fenetre));
        
	gtk_widget_hide(ColorDlg->help_button);
	
	g_signal_connect_swapped(G_OBJECT(ColorDlg->ok_button),"clicked",
		(GCallback)set_back_color,GTK_OBJECT(ColorDlg->colorsel));

	g_signal_connect_swapped(G_OBJECT(ColorDlg->ok_button),"clicked",
		(GCallback)gtk_widget_destroy,GTK_OBJECT(ColorDlg));

	g_signal_connect_swapped(G_OBJECT(ColorDlg->cancel_button),"clicked",
		(GCallback)gtk_widget_destroy,GTK_OBJECT(ColorDlg));

	gtk_widget_show(GTK_WIDGET(ColorDlg));

}
/*****************************************************************************/
GtkWidget *create_drawing_in_box(GtkWidget *box)
{
    GtkWidget *DrawingArea;

    DrawingArea = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(box), DrawingArea,TRUE,TRUE,0);
    gtk_widget_set_size_request(GTK_WIDGET(DrawingArea),400,-1);
    gtk_widget_show(DrawingArea);
    return DrawingArea;
  
}
/*****************************************************************************/
GtkWidget *create_drawing_in_table(GtkWidget *Table,gint i,gint j,gint k,gint l)
{
    GtkWidget *DrawingArea;

    DrawingArea = gtk_drawing_area_new();

    gtk_table_attach(GTK_TABLE(Table), DrawingArea, i,j, k,l,
                   GTK_FILL | GTK_EXPAND, GTK_EXPAND | GTK_FILL, 0, 0);
    gtk_widget_set_size_request(GTK_WIDGET(DrawingArea),400,-1);
    gtk_widget_show(DrawingArea);
    return DrawingArea;
  
}
/*****************************************************************************/
GtkWidget *create_frame_in_vbox(gchar *title,GtkWidget *win,GtkWidget *vbox,gboolean type)
{
  GtkWidget *frame;
  frame = gtk_frame_new (title);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (win), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  gtk_box_pack_start(GTK_BOX(vbox), frame,type,type,1);
  gtk_widget_show (frame);
  return frame;

}
/*****************************************************************************/
GtkWidget *create_vbox_in_hbox(GtkWidget *win,GtkWidget *hbox,gboolean type)
{
	GtkWidget *vbox;
	vbox = gtk_vbox_new (FALSE, 0);
	g_object_ref (vbox);
	g_object_set_data_full(G_OBJECT (win), "vbox", vbox,
                            (GDestroyNotify) g_object_unref);
	gtk_widget_show (vbox);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, type, type, 1);

	return vbox;
}
/*****************************************************************************/
GtkWidget *create_hbox_in_vbox(GtkWidget *vbox)
{
	GtkWidget *hbox;
	hbox = gtk_hbox_new (FALSE, 0);
	g_object_ref (hbox);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);	
	return hbox;
}
/*****************************************************************************/
void destroy_drawing_and_children(GtkWidget *win,gpointer data)
{
  destroy_children(GeomDlg);
}
/*****************************************************************************/
void destroy_all_drawing(GtkWidget *win)
{

  stop_calcul(win, NULL);
  while( gtk_events_pending() ) gtk_main_iteration();
  geometry  = Free_One_Geom(geometry ,Natoms);
  geometry0 = Free_One_Geom(geometry0,Natoms);

  g_free(NumFatoms);
  NumFatoms = NULL;
  NFatoms = 0;
  Natoms = 0;

 gtk_widget_destroy(GeomDrawingArea);
 GeomDrawingArea = NULL;
 gtk_widget_destroy(GeomDlg);

 if (pixmap) g_object_unref(pixmap);
 pixmap = NULL;

 if (cr) cairo_destroy (cr);
 cr = NULL;

 if (gc) g_object_unref(gc);
 gc = NULL;

 Orig[0] = Orig[1] = Orig[2] = 0.0;
}
/*****************************************************************************/
GtkWidget *AddNoteBookPage(GtkWidget *NoteBook,char *label)
{
  GtkWidget *vboxpage;
  GtkWidget *Frame;
  

  Frame = gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 2);

  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
                                Frame,
                                NULL, NULL);

  g_object_set_data(G_OBJECT (Frame), "Frame", Frame);

  gtk_widget_show(Frame);
  vboxpage = create_vbox(Frame);

  return vboxpage; 
}
/********************************************************************************/
void set_sensitive_stop_button(gboolean sens)
{
	if(GeomDrawingArea) gtk_widget_set_sensitive(StopButton, sens);
}
/********************************************************************************************/
static void stop_calcul(GtkWidget *wi, gpointer data)
{
	StopCalcul = TRUE;
}
/********************************************************************************/
void add_stop_button(GtkWidget *Win, GtkWidget *box)
{
  StopButton = gtk_button_new_with_label("Cancel");
  gtk_box_pack_start (GTK_BOX (box), StopButton, FALSE, TRUE, 0);	
  gtk_widget_set_sensitive(StopButton, FALSE);
  
  g_signal_connect(G_OBJECT(StopButton), "clicked", G_CALLBACK (stop_calcul), NULL);  
}
/********************************************************************************/
static void open_menu(GtkWidget *Win,  GdkEvent *event, gpointer Menu)
{
	GdkEventButton *bevent;
	bevent = (GdkEventButton *) event;
	popup_menu_geom( bevent->button, bevent->time);
}
/********************************************************************************/
static void add_menu_button(GtkWidget *Win, GtkWidget *box)
{
	GtkWidget* menuButton;
        menuButton = gtk_button_new_with_label("M");
  	gtk_box_pack_start (GTK_BOX (box), menuButton, FALSE, TRUE, 0);	
  
	g_signal_connect(G_OBJECT(menuButton), "button_press_event",G_CALLBACK(open_menu), NULL);
	gtk_widget_show (menuButton);
}
/********************************************************************************/
void create_window_drawing()
{
	GtkWidget *vboxframe;
	GtkWidget *frame;
	GtkWidget *hboxframe;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *hboxoperation;
	GtkWidget *DrawingArea;
	GtkWidget *vboxleft;
	GtkWidget *vboxright;
	GtkWidget *NoteBook;
	GtkWidget *Table;
	GtkWidget *handelbox;
	GtkWidget *Status;
	GtkWidget *VboxWin;
	GtkWidget *hboxtoolbar;
  	GtkWidget* handlebox;
  	GtkWidget* table;

	{
		gint i;
		factor=1.0;
		/*
		factorstick=1.0;
		factorball=1.0;
		*/
		factordipole=1.0;
		TransX=0;
		TransY=0;
		SetCosSin();
		Ddef = FALSE;
		AtomToInsert = g_strdup("C");
		for(i=0;i<4;i++) NumSelAtoms[i] = -1;
		TypeGeom =  GABEDIT_TYPEGEOM_STICK;
		ShadMode = FALSE;
		PersMode = FALSE;
		LightMode = FALSE;
		OrtepMode = FALSE;
		CartoonMode = TRUE;
		DrawDistance=FALSE;
		DrawDipole = FALSE;
		StopCalcul = FALSE;
		ShowHBonds = FALSE;
		Frag.NAtoms = 0;
		Frag.Atoms = NULL;
		FragItems = NULL;
		NFrags = 0;
		OperationType = ROTATION ;
	}
	

	geometry = NULL;
	geometry0 = NULL;
	Natoms = 0;

	NumFatoms = NULL;
	NFatoms = 0;

	define_geometry();

	if(Natoms == 0) OperationType = EDITOBJECTS;

	GeomDlg = NULL ;
	GeomDlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	VboxWin = gtk_vbox_new (TRUE, 0);
	gtk_container_add(GTK_CONTAINER(GeomDlg),VboxWin);
	gtk_widget_show(VboxWin);
	gtk_window_set_title(GTK_WINDOW(GeomDlg),_("Gabedit : Draw Geometry "));
	gtk_window_set_transient_for(GTK_WINDOW(GeomDlg),GTK_WINDOW(Fenetre));

	gtk_window_move(GTK_WINDOW(GeomDlg),0,0);
	init_child(GeomDlg,destroy_all_drawing,_(" Draw Geom. "));
	g_signal_connect(G_OBJECT(GeomDlg),"delete_event",(GCallback)destroy_children,NULL);

	frame = create_frame_in_vbox(NULL,GeomDlg,VboxWin,TRUE);
	gtk_widget_show (frame);
	vboxframe = create_vbox(frame);
	gtk_widget_show (vboxframe);

	hboxframe = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vboxframe), hboxframe, TRUE, TRUE, 0);
	gtk_widget_show (hboxframe);

	
	/* DrawingArea */
	vbox = create_vbox_in_hbox(frame,hboxframe,TRUE);
	frame = create_frame_in_vbox(NULL,GeomDlg,vbox,TRUE);
	hbox = create_hbox_in_vbox(vbox);
	vbox = create_vbox(frame);
	vboxleft= vbox;

	vbox = create_vbox_in_hbox(frame,hboxframe,FALSE);  
	vboxhandle = vbox;

	handelbox =gtk_handle_box_new ();
	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handelbox),GTK_SHADOW_NONE);
	gtk_handle_box_set_handle_position  (GTK_HANDLE_BOX(handelbox),GTK_POS_TOP);   
	gtk_container_add( GTK_CONTAINER (vbox), handelbox);
	gtk_widget_show (handelbox);
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add( GTK_CONTAINER(handelbox), vbox);
	gtk_widget_show (vbox);

	vboxright= vbox;
	vbox = vboxleft;

	/* The Table */
	Table = gtk_table_new(1, 2, FALSE);
	gtk_container_add(GTK_CONTAINER(vbox), Table);
	gtk_widget_show(Table); 

	DrawingArea = create_drawing_in_table(Table,1,2,0,1);
	gtk_widget_set_size_request(GTK_WIDGET(DrawingArea ),(gint)(ScreenHeight*0.5),(gint)(ScreenHeight*0.5));

	GeomDrawingArea = DrawingArea;
	g_signal_connect(G_OBJECT(GeomDrawingArea),"configure_event", (GCallback)configure_event,NULL);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_table_attach(GTK_TABLE(Table), vbox, 0,1, 0,1, GTK_FILL , GTK_FILL, 0, 0);
	gtk_widget_show(vbox);
	add_menu_button(GeomDlg, vbox);
	hboxtoolbar = gtk_hbox_new (FALSE, 0);
  	gtk_box_pack_start (GTK_BOX (vbox), hboxtoolbar, TRUE, TRUE, 0);	
	gtk_widget_show(hboxtoolbar);

	gtk_widget_set_events (GeomDrawingArea, GDK_EXPOSURE_MASK
					| GDK_LEAVE_NOTIFY_MASK
					| GDK_CONTROL_MASK 
					| GDK_BUTTON_PRESS_MASK
					| GDK_BUTTON_RELEASE_MASK
					| GDK_POINTER_MOTION_MASK
					| GDK_POINTER_MOTION_HINT_MASK);


	gtk_widget_realize(GeomDlg);

	NoteBook = gtk_notebook_new();
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(NoteBook), FALSE);
	gtk_notebook_set_show_border(GTK_NOTEBOOK(NoteBook), FALSE);

	NoteBookDraw = NoteBook;
	gtk_box_pack_start(GTK_BOX (vboxright), NoteBook,TRUE, TRUE, 0);

	vboxmeasure =AddNoteBookPage(NoteBook,_("Measure"));
	

	AddMeasure(GeomDlg,vboxmeasure);
	gtk_widget_show(NoteBook);
	gtk_widget_show_all(vboxmeasure);
	change_of_center(NULL,NULL);
	gtk_widget_show(vboxright);


  	handlebox = gtk_handle_box_new ();
  	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handlebox),GTK_SHADOW_NONE);    
	gtk_handle_box_set_handle_position  (GTK_HANDLE_BOX(handlebox),GTK_POS_LEFT);
  	gtk_widget_show (handlebox);
	gtk_box_pack_start(GTK_BOX (hbox), handlebox,TRUE, TRUE, 0);

	table = gtk_table_new(2,2,FALSE);
	gtk_container_add (GTK_CONTAINER (handlebox), table);
	gtk_widget_show(table);
	/* Rotation Status */
	Status = gtk_statusbar_new();
	gtk_table_attach(GTK_TABLE(table),Status,0,1,0,1,
					(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
					(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
					1,1);
	idStatusRotation = gtk_statusbar_get_context_id(GTK_STATUSBAR(Status),_("Rotation"));
	StatusRotation = Status;
	gtk_statusbar_pop(GTK_STATUSBAR(StatusRotation),idStatusRotation);
	gtk_statusbar_push(GTK_STATUSBAR(StatusRotation),idStatusRotation,
		_(" Press the Middle mouse button and move your mouse for a \"Rotation\". "));


	/* Mode Status */
	Status = gtk_statusbar_new();
	gtk_table_attach(GTK_TABLE(table),Status,1,2,0,1,
					(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
					(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
					1,1);
	idStatusPopup= gtk_statusbar_get_context_id(GTK_STATUSBAR(Status),"Ball&Stick");
	StatusPopup = Status;
	gtk_statusbar_pop(GTK_STATUSBAR(StatusPopup),idStatusPopup);
	gtk_statusbar_push(GTK_STATUSBAR(StatusPopup),idStatusPopup,
		_(" Press the Right mouse button for display the popup menu. "));

	/* Operation Status */
	Status = gtk_statusbar_new();
	hboxoperation = gtk_hbox_new (FALSE, 0);
	gtk_table_attach(GTK_TABLE(table), hboxoperation,0,2,1,2,
					(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
					(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
					1,1);
	gtk_box_pack_start (GTK_BOX(hboxoperation),Status, TRUE, TRUE, 1);

	idStatusOperation = gtk_statusbar_get_context_id(GTK_STATUSBAR(Status),_("Rotation"));
	StatusOperation = Status;
	gtk_statusbar_pop(GTK_STATUSBAR(StatusOperation),idStatusOperation);
	gtk_statusbar_push(GTK_STATUSBAR(StatusOperation),idStatusOperation,
		_(" Press the Left mouse button and move your mouse for a \"Rotation\". "));

	add_stop_button(GeomDlg, hboxoperation);
	gtk_widget_show_all(hbox);
	gtk_widget_show_all(hboxoperation);
	gtk_widget_show_all(vbox);


	gtk_widget_show(frame);

	if(MeasureIsHide)
	{
  		gtk_widget_hide(vboxhandle);
	}
	else
		gtk_widget_show(vboxhandle);

	gtk_window_set_default_size (GTK_WINDOW(GeomDlg), (gint)(ScreenHeight*0.85), (gint)(ScreenHeight*0.85));


	g_object_set_data(G_OBJECT(GeomDlg), "StatusBox",handlebox);
	create_toolbar_and_popup_menu_geom(hboxtoolbar);


	/* Evenments */
	g_signal_connect(G_OBJECT(GeomDrawingArea),"expose_event",(GCallback)expose_event,NULL);
	g_signal_connect(G_OBJECT(GeomDrawingArea), "button_press_event",G_CALLBACK(event_dispatcher), NULL);
	g_signal_connect(G_OBJECT(GeomDrawingArea), "motion_notify_event",G_CALLBACK(motion_notify), NULL);
	g_signal_connect(G_OBJECT(GeomDrawingArea), "button_release_event",G_CALLBACK(button_release), NULL);
	g_signal_connect(G_OBJECT (GeomDlg), "key_press_event", (GCallback) set_key_press, GeomDlg);
	g_signal_connect(G_OBJECT (GeomDlg), "key_release_event", (GCallback) set_key_release, NULL);
	gtk_widget_show(GeomDlg);

	gtk_window_move(GTK_WINDOW(GeomDlg),0,0);


	/* set_back_color_black();*/
	/* set_back_color_grey();*/
	set_back_color_default();
	set_icone(GeomDlg);
	if(Natoms == 0) SetOperation(NULL,EDITOBJECTS);
	/*define_good_trans();*/
	setSymmetryPrecision(GeomDlg,"1e-4");
}
/*****************************************************************************/
void draw_geometry(GtkWidget *w,gpointer d)
{
 if(GeomDrawingArea == NULL)
          create_window_drawing();
 else
 {
	gtk_widget_hide(GeomDlg);
	gtk_widget_show(GeomDlg);
  	rafresh_drawing();
 }
}
/*****************************************************************************/
void export_geometry(gchar* fileName, gchar* fileType)
{
	if(!fileName) return;
	if(!fileType) return;
	if(!GeomDrawingArea) return;
	if(!strcmp(fileType,"pdf"))
	{
		
		cairo_surface_t *surface;
		surface = cairo_pdf_surface_create(fileName, GeomDrawingArea->allocation.width, GeomDrawingArea->allocation.height);
		crExport = cairo_create(surface);
		drawGeom();
		cairo_show_page(crExport);
		cairo_surface_destroy(surface);
		cairo_destroy(crExport);
		crExport = NULL;
		return;
	}
	else
	if(!strcmp(fileType,"ps"))
	{
		
		cairo_surface_t *surface;
		surface = cairo_ps_surface_create(fileName, GeomDrawingArea->allocation.width, GeomDrawingArea->allocation.height);
		crExport = cairo_create(surface);
		drawGeom();
		cairo_show_page(crExport);
		cairo_surface_destroy(surface);
		cairo_destroy(crExport);
		crExport = NULL;
		return;
	}
	else
	if(!strcmp(fileType,"eps"))
	{
		
		cairo_surface_t *surface;
		surface = cairo_ps_surface_create(fileName, GeomDrawingArea->allocation.width, GeomDrawingArea->allocation.height);
		cairo_ps_surface_set_eps(surface, TRUE);
		crExport = cairo_create(surface);
		drawGeom();
		cairo_show_page(crExport);
		cairo_surface_destroy(surface);
		cairo_destroy(crExport);
		crExport = NULL;
		return;
	}
	else
	if(!strcmp(fileType,"svg"))
	{
		
		cairo_surface_t *surface;
		surface = cairo_svg_surface_create(fileName, GeomDrawingArea->allocation.width, GeomDrawingArea->allocation.height);
		crExport = cairo_create(surface);
		drawGeom();
		cairo_show_page(crExport);
		cairo_surface_destroy(surface);
		cairo_destroy(crExport);
		crExport = NULL;
		return;
	}
}
/********************************************************************************/
static void setSymbolOfselectedAtoms(GtkWidget *button,gpointer data)
{
	GtkWidget* winDlg  = (GtkWidget*) g_object_get_data(G_OBJECT (button), "WinDlg");
	gchar* symbol = NULL;
	if(data && (gchar*) data) symbol = (gchar*) data;
	/* fprintf(stderr,"%s\n",symbol);*/
	if(symbol && Natoms>0 && NFatoms>0 && NumFatoms)
	{
		gint k,i;
		for (k=0;k<(gint)NFatoms;k++)
		for (i=0;i<(gint)Natoms;i++)
		{
			if(geometry[i].N== NumFatoms[k])
			{
				if(geometry0[i].Prop.name) g_free(geometry0[i].Prop.name);
				if(geometry0[i].Prop.symbol) g_free(geometry0[i].Prop.symbol);
				geometry0[i].Prop = prop_atom_get(symbol);
				if(geometry[i].Prop.name) g_free(geometry[i].Prop.name);
				if(geometry[i].Prop.symbol) g_free(geometry[i].Prop.symbol);
				geometry[i].Prop = prop_atom_get(symbol);
			}
		}
		reset_all_connections();
        	reset_charges_multiplicities();
        	drawGeom();
        	set_optimal_geom_view();
        	create_GeomXYZ_from_draw_grometry();
	}
	gtk_widget_destroy(winDlg);
}
/********************************************************************************/
void setSymbolOfselectedAtomsDlg()
{
	GtkWidget* Table;
	GtkWidget* button;
	GtkWidget* frame;
	guint i;
	guint j;
        GtkStyle *button_style;
        GtkStyle *style;

	gchar*** Symb = get_periodic_table();
	GtkWidget* winDlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_modal(GTK_WINDOW(winDlg),TRUE);
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Select your atom"));
	gtk_window_set_default_size (GTK_WINDOW(winDlg),(gint)(ScreenWidth*0.5),(gint)(ScreenHeight*0.4));

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

	gtk_container_add(GTK_CONTAINER(winDlg),frame);  
	gtk_widget_show (frame);

	Table = gtk_table_new(PERIODIC_TABLE_N_ROWS-1,PERIODIC_TABLE_N_COLUMNS,TRUE);
	gtk_container_add(GTK_CONTAINER(frame),Table);
	 button_style = gtk_widget_get_style(winDlg); 
  
	for ( i = 0;i<PERIODIC_TABLE_N_ROWS-1;i++)
	for ( j = 0;j<PERIODIC_TABLE_N_COLUMNS;j++)
	{
	  if(strcmp(Symb[j][i],"00"))
	  {
	  	button = gtk_button_new_with_label(Symb[j][i]);
          	style=set_button_style(button_style,button,Symb[j][i]);
		g_object_set_data(G_OBJECT (button), "WinDlg",winDlg);
          	g_signal_connect(G_OBJECT(button), "clicked", (GCallback)setSymbolOfselectedAtoms,(gpointer )Symb[j][i]);
	  	gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	  }
	}
 	
	gtk_widget_show_all(winDlg);
}
#endif /* DRAWGEOMGL */
