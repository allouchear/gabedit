/* BuildPolySaccharide.c */
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
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Geometry/FragmentsPSC.h"
#include "../Geometry/RotFragments.h"
#include "../Geometry/MenuToolBarGeom.h"

void define_good_factor();
void create_GeomXYZ_from_draw_grometry();

#define LigneT 7
#define ColonneT 2

static GtkWidget* Entries[3];
static GtkWidget* ButtonAlpha = NULL;
static GtkWidget* ButtonBeta = NULL;
static GtkWidget* ButtonL = NULL;
static GtkWidget* ButtonD = NULL;

static	char *Symb[ColonneT][LigneT]={
		{"Allose","Altrose","Arabinose","Fructose","Galactose","Glucose","Gulose"},
		{"Idose","Lyxose","Mannose","Ribose","Talose","Xylose","00"}
		};
static 	GtkWidget* SButtons[ColonneT][LigneT];

static gchar conf[30];
static gint lastFragNumber = -1;
static gint O  = -1;
static gint O1 = -1;
static gint O2 = -1;
static gint O3 = -1;
static gint O4 = -1;
static gint O5 = -1;
static gint O6 = -1;
static gint HO1 = -1;
static gint HO2 = -1;
static gint HO3 = -1;
static gint HO4 = -1;
static gint HO6 = -1;
static gint C1 = -1;
static gint C2 = -1;
static gint C3 = -1;
static gint C4 = -1;
static gint C5 = -1;
static gint C6 = -1;
static gint H1 = -1;
static gint H2 = -1;
static gint H3 = -1;
static gint H4 = -1;
static gint H5 = -1;
static gint H6 = -1;
static gint H61 = -1;
static gint H62 = -1;

static gint lastO  = -1;
static gint lastO1 = -1;
static gint lastC1 = -1;
static gint lastHO1 = -1;
static gint 	connectTo = -1;
static gint 	connect2 = -1;
static gint 	become = -1;
static gint 	removeH = -1;

static gdouble phi = -57;
static gdouble psi = 47;
static gdouble omega = 180;

static gdouble lastOmega = 180;
static gdouble lastPsi = 180;
static gdouble lastPhi = 180;
gchar  fragName[30] = "L";
gchar  fullFragName[30] = "L";
static Fragment Frag = {0,NULL};
static GeomDef* G=NULL;
static gint Nb = 0;
static gboolean	capped = FALSE;

/********************************************************************************/
static void init_variables()
{
	lastFragNumber = -1;
	capped = FALSE;

	lastOmega = 180;
	lastPsi = 180;
	lastPhi = 180;
	if(Nb!=0)
	{
		gint i;
                for (i=0;i<Nb;i++)
		{
			g_free(G[i].Prop.symbol);
			g_free(G[i].Prop.name);
			g_free(G[i].mmType);
			g_free(G[i].pdbType);
			g_free(G[i].Residue);
			if(G[i].typeConnections) g_free(G[i].typeConnections);
		}

		if(G)
			g_free(G);
	}
	Nb=0;
	G = NULL;
	lastO  = -1;
	lastO1 = -1;
	lastC1 = -1;
	lastHO1 = -1;
	
}
/*****************************************************************************/
static void destroy_dlg(GtkWidget* Dlg,gpointer data) 
{
	delete_child(Dlg);
	if(Nb!=0)
	{
		gint i;
                for (i=0;i<Nb;i++)
		{
			g_free(G[i].Prop.symbol);
			g_free(G[i].Prop.name);
			g_free(G[i].mmType);
			g_free(G[i].pdbType);
			g_free(G[i].Residue);
			if(G[i].typeConnections) g_free(G[i].typeConnections);
		}

		if(G) g_free(G);
	}
	Nb=0;
	G = NULL;

	activate_rotation();
}
/*****************************************************************************/
static gboolean test_connection(gint i,gint j)
{
	gdouble distance;
	gdouble rcut;
	gdouble x,y,z;
	if(G[i].N == 0 || G[j].N == 0) return FALSE;
	x = G[i].X-G[j].X;
	y = G[i].Y-G[j].Y;
	z = G[i].Z-G[j].Z;
	distance = x*x+y*y+z*z;
	rcut = G[i].Prop.covalentRadii+G[j].Prop.covalentRadii;
	rcut = rcut* rcut;

	if(distance<rcut) return TRUE;
	else return FALSE;
}
/*****************************************************************************/
static void set_connections(gint iBegin, gint lastF, gint newF)
{
	gint i;
	gint j;

	if(Nb<1) return;
	if(!G) return;
	if(iBegin<0) iBegin = 0;
	for(i=0;i<iBegin;i++)
	{
		if(!G[i].typeConnections) continue;
		G[i].typeConnections = g_realloc(G[i].typeConnections,Nb*sizeof(gint));
		for(j=iBegin;j<Nb;j++) G[i].typeConnections[j] = 0;
	}
	for(i=iBegin;i<Nb;i++)
	{
		G[i].typeConnections = g_malloc(Nb*sizeof(gint));
		for(j=0;j<Nb;j++) G[i].typeConnections[j] = 0;
	}
	for(i=iBegin;i<Nb;i++)
	{
		for(j=i+1;j<Nb;j++)
		{
			if(test_connection(i,j)) G[i].typeConnections[j]= 1;
			G[j].typeConnections[i] = G[i].typeConnections[j];
		}
	}
	if(lastF>-1 && newF>-1)
	{
		G[lastF].typeConnections[newF] = G[newF].typeConnections[lastF]=1;
	}
}
/*****************************************************************************/
static void define_geometry_to_draw()
{
	gint i;
	gint j;
	gdouble C[3]={0.0,0.0,0.0};

	Free_One_Geom(geometry0,Natoms);
	Free_One_Geom(geometry ,Natoms);
	Natoms = 0;
	geometry0 = NULL;
	geometry  = NULL;
	reset_origine_molecule_drawgeom();

	geometry0 = g_malloc((Nb)*sizeof(GeomDef));
	geometry  = g_malloc((Nb)*sizeof(GeomDef));
	Natoms = 0;
	for(i=0;i<Nb;i++)
	{
		if(G[i].N != 0)
		{
			geometry0[Natoms].X = G[i].X;
			geometry0[Natoms].Y = G[i].Y;
			geometry0[Natoms].Z = G[i].Z;
			geometry0[Natoms].Charge = G[i].Charge;
			geometry0[Natoms].Prop = prop_atom_get(G[i].Prop.symbol);
			geometry0[Natoms].mmType = g_strdup(G[i].mmType);
			geometry0[Natoms].pdbType = g_strdup(G[i].pdbType);
			geometry0[Natoms].Residue = g_strdup(G[i].Residue);
			geometry0[Natoms].ResidueNumber = G[i].ResidueNumber;
			geometry0[Natoms].show = TRUE;
			geometry0[Natoms].N = i+1;
			geometry0[Natoms].typeConnections = NULL;
			geometry0[Natoms].Layer = HIGH_LAYER;
			geometry0[Natoms].Variable = TRUE;

			geometry[Natoms].X = G[i].X;
			geometry[Natoms].Y = G[i].Y;
			geometry[Natoms].Z = G[i].Z;
			geometry[Natoms].Charge = G[i].Charge;
			geometry[Natoms].Prop = prop_atom_get(G[i].Prop.symbol);
			geometry[Natoms].mmType = g_strdup(G[i].mmType);
			geometry[Natoms].pdbType = g_strdup(G[i].pdbType);
			geometry[Natoms].Residue = g_strdup(G[i].Residue);
			geometry[Natoms].ResidueNumber = G[i].ResidueNumber;
			geometry[Natoms].show = TRUE;
			geometry[Natoms].N = i+1;
			geometry[Natoms].typeConnections = NULL;
			geometry[Natoms].Layer = HIGH_LAYER;
			geometry[Natoms].Variable = TRUE;
			Natoms++;
			C[0] +=  G[i].X;
			C[1] +=  G[i].Y;
			C[2] +=  G[i].Z;
		}


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
	if(Natoms>0)
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
	for(i=0;i<(gint)Natoms;i++)
	{
		gint iG = geometry[i].N-1;
		for(j=i+1;j<(gint)Natoms;j++) 
		{
			gint jG = geometry[j].N-1;
			geometry[i].typeConnections[j] = G[iG].typeConnections[jG];
			geometry[j].typeConnections[i] = G[jG].typeConnections[iG];
		}
	}
	for(i=0;i<(gint)Natoms;i++)
		 geometry[i].N = geometry0[i].N = i+1;

	copy_connections(geometry0,geometry,Natoms);
	RebuildGeom = TRUE;

}
/********************************************************************************/
static void fixH6BondAngles( gint previousFragNumber, gint currentFragNumber )
{
	gint H61 = -1, H62 = -1, LastO1 = -1, C6 = -1, C5 = -1;
	gdouble pseudo[] = {0.0, 0.0, 0.0};
	gint i;
	for( i = 0; i < Nb; i++ )
	{
		if(G[i].ResidueNumber != previousFragNumber)
			continue;
		if ( !strcmp(G[i].pdbType ,"O1" ) )
			LastO1 = i;
	}
	for( i = 0; i < Nb; i++ )
	{
		if(G[i].ResidueNumber != currentFragNumber)
			continue;

		if ( !strcmp(G[i].pdbType ,"H61" ) )
			H61 = i;
		else if ( !strcmp(G[i].pdbType ,"H62" ) )
			H62 = i;
		else if ( !strcmp(G[i].pdbType ,"H62" ) )
			H62 = i;
		else if ( !strcmp(G[i].pdbType ,"C6" ) )
			C6 = i;
		else if ( !strcmp(G[i].pdbType ,"C5" ) )
			C5 = i;
	}
	if ( ( LastO1 == -1 ) || ( H61 == -1 ) || 
		 ( H62 == -1 )    || ( C6 == -1 )  ||
		 ( C5 == -1 ) 
	   )
	   return;
	pseudo[ 0 ] =( ( G[LastO1].X + G[C5].X ) / 2.0 );
	pseudo[ 1 ] =( ( G[LastO1].Y + G[C5].Y ) / 2.0 );
	pseudo[ 2 ] =( ( G[LastO1].Z + G[C5].Z ) / 2.0 );

	if(Nb>0)
		G = g_realloc(G,(Nb+1)*sizeof(GeomDef));
	else
		G = g_malloc((1)*sizeof(GeomDef));
	G[Nb].X = pseudo[0];
	G[Nb].Y = pseudo[1];
	G[Nb].Z = pseudo[2];

	SetAngle  (Nb+1, G, Nb, C6, H61, 125.25 , NULL, 0); 
	SetTorsion(Nb+1, G, lastO1, Nb, C6, H61, 90.0, NULL, 0);  

	SetAngle  (Nb+1, G, Nb, C6, H62, 125.25 , NULL, 0); 
	SetTorsion(Nb+1, G, lastO1, Nb, C6, H62, -90.0, NULL, 0);  

	if(Nb>0)
		G = g_realloc(G,(Nb)*sizeof(GeomDef));
}
/********************************************************************************/
void add_fragment(GtkWidget* button)
{
	gint i;
	gint j;
	G_CONST_RETURN gchar* t;
	gint omegaArrayCounter=0;
	gint* omegaArray=NULL;


	t = gtk_entry_get_text(GTK_ENTRY(Entries[0]));
	phi = atof(t);
	t = gtk_entry_get_text(GTK_ENTRY(Entries[1]));
	psi = atof(t);
	t = gtk_entry_get_text(GTK_ENTRY(Entries[2]));
	omega = atof(t);

	if(Nb>0)
		G = g_realloc(G,(Nb+Frag.NAtoms+1)*sizeof(GeomDef));
	else
		G = g_malloc((Frag.NAtoms+1)*sizeof(GeomDef));

	omegaArray  = g_malloc(Frag.NAtoms*sizeof(gint)); 

	Ddef = FALSE;

	j=Nb-1;
	for(i=0;i<Frag.NAtoms;i++)
	{
		j++;
		G[j].X=Frag.Atoms[i].Coord[0];
		G[j].Y=Frag.Atoms[i].Coord[1];
		G[j].Z=Frag.Atoms[i].Coord[2];
		G[j].Charge=Frag.Atoms[i].Charge;
		G[j].mmType=g_strdup(Frag.Atoms[i].mmType);
		G[j].pdbType=g_strdup(Frag.Atoms[i].pdbType);
		G[j].Residue=g_strdup(Frag.Atoms[i].Residue);
		G[j].ResidueNumber=lastFragNumber+1;
		G[j].typeConnections = NULL;

		G[j].Prop = prop_atom_get(Frag.Atoms[i].Symb);
		G[j].N = j+1;
		
		if (!strcmp(Frag.Atoms[i].pdbType, "O1" ) ){
			O1 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "O2" ) ){
			O2 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "O3" ) ){
			O3 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "O4" ) ){
			O4 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "O5" ) ){
			O5 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "O" ) ){
			O = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "O6" ) ){
			O6 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "HO1" ) ){
			HO1 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "HO2" ) ){
			HO2 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "HO3" ) ){
			HO3 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "HO4" ) ){
			HO4 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "HO6" ) ){
			HO6 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "C1" ) ){
			C1 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "C2" ) ){
			C2 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "C3" ) ){
			C3 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "C4" ) ){
			C4 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "C5" ) ){
			C5 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "C6" ) ){
			C6 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "H1" ) ){
			H1 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "H2" ) ){
			H2 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "H3" ) ){
			H3 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "H4" ) ){
			H4 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "H6" ) ){
			H6 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "H5" ) ){
			H5 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "H61" ) ){
			H61 = j;
		}
		else if (!strcmp(Frag.Atoms[i].pdbType, "H61" ) ){
			H62 = j;
		}
		omegaArray[ omegaArrayCounter++ ] = j;
	}
	Nb += Frag.NAtoms;

	connectTo = C4;
	connect2 = C3;
	become = O3;
	removeH = HO3;

	if(!strcmp(conf,"O1 - C1"))
	{
		connectTo = C1;
		connect2 = O;
		become = O1;
		removeH =  HO1;
		capped = TRUE;
	}
	else
	if(!strcmp(conf,"O1 - C2"))
	{
		connectTo = C2;
		connect2 = C1;
		become = O2;
		removeH =  HO2;
	}
	else
	if(!strcmp(conf,"O1 - C3"))
	{

		connectTo = C3;
		connect2 = C2;
		become = O3;
		removeH =  HO3;
	}
	else
	if(!strcmp(conf,"O1 - C4"))
	{
		connectTo = C4;
		connect2 = C3;
		become = O4;
		removeH = HO4;
	}
	else
	if(!strcmp(conf,"O1 - C5"))
	{
		connectTo = C5;
		connect2 = C4;
		become = O5;
		removeH =  H5;
	}
	else
	{
		connectTo = C6;
		connect2 = C5;
		become = O6;
		removeH = H6;
	}
	if(lastFragNumber !=-1)
	{
		SetBondDistance(G, lastO1, become, 0.0, omegaArray, omegaArrayCounter);
		SetAngle( Nb, G, lastC1, lastO1, connectTo, 109.5, omegaArray, omegaArrayCounter ); 
		SetTorsion(Nb, G, lastO, lastC1, lastO1, connectTo, lastPhi, omegaArray, omegaArrayCounter );  
		SetTorsion(Nb, G, lastC1, lastO1, connectTo, connect2, psi, omegaArray, omegaArrayCounter );
		if ( connectTo == C6 )
		{
			SetTorsion(Nb, G, lastO1, connectTo, connect2, C4, omega, omegaArray, omegaArrayCounter );
			fixH6BondAngles( lastFragNumber, lastFragNumber + 1 );
		}
		/* printf("removeH=%d bec = %d lastHO1 = %d\n ",removeH,become,lastHO1);*/
		G[removeH].N = 0;
		G[become].N  = 0;
		G[lastHO1].N = 0;
	}

	set_connections(Nb-Frag.NAtoms,lastO1,connectTo);
	define_geometry_to_draw();
	define_good_factor();
	create_GeomXYZ_from_draw_grometry();
	unselect_all_atoms();
	reset_multiple_bonds();
	reset_hydrogen_bonds();
	reset_charges_multiplicities();
	drawGeom();

	lastO1 = O1;
	lastC1 = C1;
	lastO = O;
	lastHO1 = HO1;

	lastOmega = omega;
	lastPhi = phi;
	lastPsi = psi;
	lastFragNumber++;
	if(capped)
	{
		GtkWidget* Dlg = g_object_get_data(G_OBJECT (button),"Dlg");
		GtkWidget* w;
		destroy_dlg(Dlg,NULL);
		w = Message(_("The O1-C1 bond acts as a cap to the chain.\nSuggestion: Use O1-C1 only for disaccharides or to cap a chain."),_("Info"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(GeomDlg));
	}

}
/********************************************************************************/
static void build_polysaccharide(GtkWidget *w,gpointer data)
{
	gboolean alpha = TRUE;
	sprintf(fragName,"%s",(gchar*)data);
	/* lowercase(fragName);*/


	if (GTK_TOGGLE_BUTTON (ButtonBeta)->active)
		alpha = FALSE; 

	sprintf(fullFragName,"%s",fragName);
	Frag = GetFragmentPSC(fullFragName,alpha);
	add_fragment(w);
}
/********************************************************************************************************/
static void traite_conformation(GtkComboBox *combobox, gpointer d)
{
	gint i;
	gint j;
	GtkTreeIter iter;
	gchar* option = NULL;
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &option, -1);
	}

	sprintf(conf,"%s",option);
	if(!strcmp(option,"O1 - C5") || !strcmp(option,"O1 - C6")) gtk_editable_set_editable((GtkEditable*) Entries[2],TRUE);
	else gtk_editable_set_editable((GtkEditable*) Entries[2],FALSE);
	if(!strcmp(option,"O1 - C5"))
	{
		for(i=0;i<LigneT;i++)
			for(j=0;j<ColonneT;j++)
			{
				if(SButtons[j][i])
				{
					if(	!strcmp(Symb[j][i],"Arabinose") ||
					   	!strcmp(Symb[j][i],"Lyxose") ||
					   	!strcmp(Symb[j][i],"Ribose") ||
					   	!strcmp(Symb[j][i],"Xylose") 
					)
						gtk_widget_set_sensitive(SButtons[j][i], TRUE);
					else
						gtk_widget_set_sensitive(SButtons[j][i], FALSE);
				}
			}
	}
	else
	if(!strcmp(option,"O1 - C4") || !strcmp(option,"O1 - C6"))
	{
		for(i=0;i<LigneT;i++)
			for(j=0;j<ColonneT;j++)
			{
				if(SButtons[j][i])
				{
					if(	!strcmp(Symb[j][i],"Arabinose") ||
					   	!strcmp(Symb[j][i],"Lyxose") ||
					   	!strcmp(Symb[j][i],"Ribose") ||
					   	!strcmp(Symb[j][i],"Xylose") 
					)
						gtk_widget_set_sensitive(SButtons[j][i], FALSE);
					else
						gtk_widget_set_sensitive(SButtons[j][i], TRUE);
				}
			}

	}
	else
	{
		for(i=0;i<LigneT;i++)
			for(j=0;j<ColonneT;j++)
				if(SButtons[j][i])
					gtk_widget_set_sensitive(SButtons[j][i], TRUE);
	}

}
/********************************************************************************************************/
static GtkWidget *get_button_connectivity()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;

	store = gtk_tree_store_new (1,G_TYPE_STRING);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "O1 - C1", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "O1 - C2", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "O1 - C3", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "O1 - C4", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "O1 - C5", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "O1 - C6", -1);
	sprintf(conf,"O1 - C4");

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traite_conformation), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	return combobox;
}
/********************************************************************************/
static GtkWidget* add_connectivity(GtkWidget* Dlg,GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* combobox;

	frame = gtk_frame_new (_("Connectivity"));
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

	gtk_container_add(GTK_CONTAINER(box),frame);  
	gtk_widget_show (frame);

  
	combobox =get_button_connectivity();
	gtk_container_add(GTK_CONTAINER(frame), combobox);
	return combobox;
}
/********************************************************************************/
static void add_anomer(GtkWidget* Dlg,GtkWidget *box)
{
	GtkWidget* Table;
	GtkWidget* frame;

	frame = gtk_frame_new ("Anomer");
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

	gtk_container_add(GTK_CONTAINER(box),frame);  
	gtk_widget_show (frame);

	Table = gtk_table_new(1,2,TRUE);
	gtk_container_add(GTK_CONTAINER(frame),Table);
  
	ButtonAlpha = gtk_radio_button_new_with_label( NULL," Alpha " );
	gtk_table_attach(GTK_TABLE(Table),ButtonAlpha,0,1,0,1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  	ButtonBeta = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonAlpha)),
                       " Beta "); 
	gtk_table_attach(GTK_TABLE(Table),ButtonBeta,1,2,0,1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  	g_object_set_data(G_OBJECT (Dlg), "ButtonAlpha",ButtonAlpha);
  	g_object_set_data(G_OBJECT (Dlg), "ButtonBeta",ButtonBeta);
}
/********************************************************************************/
static void add_isomer(GtkWidget* Dlg,GtkWidget *box)
{
	GtkWidget* Table;
	GtkWidget* frame;

	frame = gtk_frame_new (_("Isomer"));
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

	gtk_container_add(GTK_CONTAINER(box),frame);  
	gtk_widget_show (frame);

	Table = gtk_table_new(1,2,TRUE);
	gtk_container_add(GTK_CONTAINER(frame),Table);
  
	ButtonL = gtk_radio_button_new_with_label( NULL," L " );
	gtk_table_attach(GTK_TABLE(Table),ButtonL,0,1,0,1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  	ButtonD = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonL)),
                       " D "); 
	gtk_table_attach(GTK_TABLE(Table),ButtonD,1,2,0,1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  	g_object_set_data(G_OBJECT (Dlg), "ButtonL",ButtonL);
  	g_object_set_data(G_OBJECT (Dlg), "ButtonD",ButtonD);
}
/********************************************************************************/
static void add_conformation(GtkWidget* Dlg,GtkWidget *box)
{
	GtkWidget* Table;
	GtkWidget* frame;
	GtkWidget* Label;
	gchar* tlabel[] = {"Phi","Psi","Omega"};
	gchar* elabel[] = {"180.0","180.0","180.0"};
	gint i;

	frame = gtk_frame_new (_("Conformation"));
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

	gtk_container_add(GTK_CONTAINER(box),frame);  
	gtk_widget_show (frame);

	Table = gtk_table_new(3,2,TRUE);
	gtk_container_add(GTK_CONTAINER(frame),Table);
  
	for(i=0;i<3;i++)
	{
		Label = gtk_label_new(tlabel[i]);
		gtk_table_attach(GTK_TABLE(Table),Label,0,1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
		Entries[i] = gtk_entry_new();
		gtk_table_attach(GTK_TABLE(Table),Entries[i],1,2,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);

		if(i==2)
  			gtk_editable_set_editable((GtkEditable*) Entries[i],FALSE);
		else
  			gtk_editable_set_editable((GtkEditable*) Entries[i],TRUE);
  		gtk_entry_set_text(GTK_ENTRY(Entries[i]),elabel[i]);
	}
}
/******************************************************************************/
static void add_buttons(GtkWidget *Dlg,GtkWidget* box)
{
	GtkWidget* Table;
	GtkWidget* button;
	GtkWidget* frame;
	guint i;
	guint j;
        GtkStyle *button_style;
        GtkStyle *style;


  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

  gtk_container_add(GTK_CONTAINER(box),frame);  
  gtk_widget_show (frame);

  Table = gtk_table_new(LigneT,ColonneT,TRUE);
  gtk_container_add(GTK_CONTAINER(frame),Table);
  button_style = gtk_widget_get_style(Dlg); 
  
  for ( i = 0;i<LigneT;i++)
	  for ( j = 0;j<ColonneT;j++)
  {
	  SButtons[j][i] = NULL;
	  if(strcmp(Symb[j][i],"00"))
	  {
		button = gtk_button_new_with_label(Symb[j][i]);
		g_object_set_data(G_OBJECT (button), "Dlg",Dlg);
		style=set_button_style(button_style,button,"H");
		g_signal_connect(G_OBJECT(button), "clicked",
                            (GCallback)build_polysaccharide,(gpointer )Symb[j][i]);
      	  /*
	    gtk_widget_set_size_request (GTK_WIDGET(button), (ScreenWidth)*0.05, (ScreenHeight)*0.05);
	  */
	  gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	  SButtons[j][i] = button;
	  }

  }
  
}
/**********************************************************************/
void build_polysaccharide_dlg()
{
  GtkWidget *Dlg;
  GtkWidget *Button;
  GtkWidget *hbox;
  GtkWidget *vbox;
  GtkWidget *frame;
  GtkWidget *vboxframe;
  GtkWidget *combobox;
  
  init_variables();
  Dlg = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dlg),_("Build PolySaccharide molecule"));
  gtk_window_set_position(GTK_WINDOW(Dlg),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (Dlg), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(Dlg),GTK_WINDOW(GeomDlg));


  add_child(GeomDlg,Dlg,gtk_widget_destroy,_(" Build PolySacc. mol. "));


  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)destroy_dlg,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->vbox), frame,TRUE,TRUE,0);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  hbox = create_hbox_false(vboxframe);

  add_buttons(Dlg,hbox);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_end (GTK_BOX(hbox), vbox, FALSE, FALSE, 5);  
  gtk_widget_show (vbox);
  combobox = add_connectivity(Dlg,vbox);
  add_conformation(Dlg,vbox);

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_box_pack_end (GTK_BOX(vbox), hbox, FALSE, FALSE, 5);  
  gtk_widget_show (hbox);
  add_anomer(Dlg,hbox);
  add_isomer(Dlg,hbox);


  /* The "Close" button */
  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dlg)->action_area), FALSE);
  gtk_widget_realize(Dlg);
  Button = create_button(Dlg,_("Close"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 5);  
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)destroy_dlg,GTK_OBJECT(Dlg));

  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->vbox);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->action_area);
  gtk_widget_show_now(Dlg);

  /*fit_windows_position(GeomDlg, Dlg);*/

  gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 3);
}

