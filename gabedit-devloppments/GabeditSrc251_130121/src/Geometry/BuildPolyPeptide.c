/* BuildPolyPeptide.c */
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
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Geometry/FragmentsPPD.h"
#include "../Geometry/RotFragments.h"
#include "../Geometry/MenuToolBarGeom.h"
#include "../MolecularMechanics/PDBTemplate.h"

void define_good_factor();
void create_GeomXYZ_from_draw_grometry();

#define LigneT 7
#define ColonneT 4

static GtkWidget* Entries[3];
static GtkWidget* ButtonL = NULL;
static GtkWidget* ButtonD = NULL;
static int nbuffer = 0;
static int ibuffer = 0;
static gdouble phiBuffer[ 4 ];
static gdouble psiBuffer[ 4 ];
static gdouble omegaBuffer[ 4 ];

static gchar conf[30];
static gint lastFragNumber = -1;
static gint CA = -1;
static gint C = -1;
static gint N = -1;
static gint O = -1;
static gint H = -1;
static gint CH3 = -1;

static gint lastCA = -1;
static gint lastC = -1;
static gint lastN = -1;
static gint lastO = -1;
static gint lastH = -1;
static gint lastCH3 = -1;
static gdouble phi = -57;
static gdouble psi = 47;
static gdouble omega = 180;
static gdouble lastOmega = 180;
static gdouble lastPsi = 180;
static gchar  fragName[30] = "L";
static gchar  fullFragName[30] = "L";
static gboolean NCapped = FALSE;
static gboolean zwitterionic = FALSE;
static Fragment Frag = {0,NULL};
static GeomDef* G=NULL;
static gint Nb = 0;

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
static void init_variables()
{
	lastFragNumber = -1;
	CA = -1;
	C = -1;
	N = -1;
	O = -1;
	H = -1;
	CH3 = -1;

	lastCA = -1;
	lastC = -1;
	lastN = -1;
	lastO = -1;
	lastH = -1;
	lastCH3 = -1;

	phi = -57;
	psi = 47;
	omega = 180;

	lastOmega = 180;
	lastPsi = 180;
	sprintf(fragName,"%s","L");
	sprintf(fullFragName,"%s","L");
	NCapped = FALSE;
	zwitterionic = FALSE;
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
			g_free(G[i].typeConnections);
		}

		if(G) g_free(G);
	}
	Nb=0;
	G = NULL;


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
			g_free(G[i].typeConnections);
		}

		if(G)
			g_free(G);
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
static void set_connections(gint iBegin, gint lastC, gint N)
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
	if(lastC>-1 && N>-1)
	{
		G[lastC].typeConnections[N] = G[N].typeConnections[lastC]=1;
	}
}
/*****************************************************************************/
static void define_geometry_to_draw()
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

	Natoms = Nb;
	if(Natoms<1) return;
	reset_origine_molecule_drawgeom();
	geometry0 = g_malloc((Natoms)*sizeof(GeomDef));
	geometry  = g_malloc((Natoms)*sizeof(GeomDef));
	n = 0;
	for(i=0;i<Nb;i++)
	{
		if(G[i].N == 0) continue;
		geometry0[n].X = G[i].X;
		geometry0[n].Y = G[i].Y;
		geometry0[n].Z = G[i].Z;
		geometry0[n].Charge = G[i].Charge;
		geometry0[n].Prop = prop_atom_get(G[i].Prop.symbol);
		geometry0[n].mmType = g_strdup(G[i].mmType);
		geometry0[n].pdbType = g_strdup(G[i].pdbType);
		geometry0[n].Residue = g_strdup(G[i].Residue);
		geometry0[n].ResidueNumber = G[i].ResidueNumber;
		geometry0[n].show = TRUE;
		geometry0[n].Layer = HIGH_LAYER;
		geometry0[n].Variable = TRUE;


		geometry0[n].N = i+1;
        	geometry0[n].typeConnections = NULL;

		geometry[n].X = G[i].X;
		geometry[n].Y = G[i].Y;
		geometry[n].Z = G[i].Z;
		geometry[n].Charge = G[i].Charge;
		geometry[n].Prop = prop_atom_get(G[i].Prop.symbol);
		geometry[n].mmType = g_strdup(geometry0[n].mmType);
		geometry[n].pdbType = g_strdup(geometry0[n].pdbType);
		geometry[n].Residue = g_strdup(geometry0[n].Residue);
		geometry[n].ResidueNumber = G[i].ResidueNumber;
		geometry[n].show = TRUE;
		geometry[n].N = i+1;
        	geometry[n].typeConnections = NULL;
		geometry[n].Layer = HIGH_LAYER;
		geometry[n].Variable = TRUE;
		C[0] +=  G[i].X;
		C[1] +=  G[i].Y;
		C[2] +=  G[i].Z;
		n++;
	}
	if(n>0)
	for(i=0;i<3;i++)
		C[i] /= n;
	/* center */
	for(i=0;i<n;i++)
	{
		geometry0[i].X -= C[0];
		geometry0[i].Y -= C[1];
		geometry0[i].Z -= C[2];

		geometry[i].X -= C[0];
		geometry[i].Y -= C[1];
		geometry[i].Z -= C[2];
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
static void re_set_angles(gboolean forward)
{
	gchar* t = NULL;
	if(nbuffer<1) return;
	if(forward) ibuffer++;
	else ibuffer--;
	if(ibuffer>nbuffer-1) ibuffer = 0;
	if(ibuffer<0) ibuffer = nbuffer-1;
	t = g_strdup_printf("%0.1f ",phiBuffer[ibuffer]);
	gtk_entry_set_text(GTK_ENTRY(Entries[0]),t);
	g_free(t);

	t = g_strdup_printf("%0.1f ",psiBuffer[ibuffer]);
	gtk_entry_set_text(GTK_ENTRY(Entries[1]),t);
	g_free(t);

	t = g_strdup_printf("%0.1f ",omegaBuffer[ibuffer]);
	gtk_entry_set_text(GTK_ENTRY(Entries[2]),t);
	g_free(t);
		
}
/********************************************************************************/
static void add_fragment(gchar* what)
{
	gint i;
	gint j;
	gint omegaArrayCounter=0;
	gint* omegaArray=NULL;
	gint phiArrayCounter = 0;
	gint* phiArray = NULL;
	gboolean CCapped = FALSE;
	gint H2 = -1;

	G_CONST_RETURN gchar* t;

	C = -1;
	N = -1;
	CA = -1;
	CH3 = -1;
	O = -1;
	H = -1;

	if(Frag.NAtoms<1) return;

	t = gtk_entry_get_text(GTK_ENTRY(Entries[0]));
	phi = atof(t);
	t = gtk_entry_get_text(GTK_ENTRY(Entries[1]));
	psi = atof(t);
	t = gtk_entry_get_text(GTK_ENTRY(Entries[2]));
	omega = atof(t);

	if(Nb>0) G = g_realloc(G,(Nb+Frag.NAtoms+1)*sizeof(GeomDef));
	else G = g_malloc((Frag.NAtoms+1)*sizeof(GeomDef));

	omegaArray  = g_malloc(Frag.NAtoms*sizeof(gint)); 
	phiArray    = g_malloc(Frag.NAtoms*sizeof(gint)); 

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

		G[j].Prop = prop_atom_get(Frag.Atoms[i].Symb);
		G[j].N = j+1;

		if(!strcmp(Frag.Atoms[i].pdbType,"C"))
		{
			C =  j;
			phiArray[ phiArrayCounter++ ] =  j;
		}
		else if(!strcmp(Frag.Atoms[i].pdbType,"N")) N =  j;
		else if(!strcmp(Frag.Atoms[i].pdbType,"H2")) H2 =  j;
		else if(!strcmp(Frag.Atoms[i].pdbType,"CA")) CA = j;
		else if(!strcmp(Frag.Atoms[i].pdbType,"CH3")) CH3 = j;
		else if(!strcmp(Frag.Atoms[i].pdbType,"O"))
		{
			O =  j;
			phiArray[ phiArrayCounter++ ] =  j;
		}
		else if(!strcmp(Frag.Atoms[i].pdbType,"H"))
		{
			H =  j;
			phiArray[ phiArrayCounter++ ] =  j;
		}
		else phiArray[ phiArrayCounter++ ] =  j;

		omegaArray[ omegaArrayCounter++ ] = j;
	}
	if(!strcmp(what , "Nme")) { CA = CH3; CCapped = TRUE;}
	if(!strcmp(what , "Nhe")) { CA = H2; CCapped = TRUE;}
	if(!strcmp(what , "Ace")) CA = CH3;
	Nb += Frag.NAtoms;
	if(lastFragNumber != -1)
	{
		if(lastC != -1 && N != -1) SetBondDistanceX(G,lastC,N,1.47,omegaArray,omegaArrayCounter);

		if ( lastN > -1 )
		{
			if ( lastN > -1 && lastCA > -1 &&  lastC > 1 && N > -1 )
			{
				SetAngle( Nb,G,lastCA, lastC, N, 111.2, omegaArray, omegaArrayCounter); 
				SetAngle( Nb,G, lastC, N, CA, 121.9, omegaArray, omegaArrayCounter); 

				SetTorsion( Nb,G,lastN, lastCA, lastC, N, lastPsi, omegaArray, omegaArrayCounter );
			}
			if ( !CCapped )
			{
				if ( lastCA > -1 &&  lastC > 1 && N > -1  && C > -1)
				SetTorsion( Nb,G, lastCA, lastC, N, C, lastOmega, omegaArray, omegaArrayCounter );
			}
			else
			{
				if ( lastCA > -1 &&  lastC > 1 && N > -1  && CA > -1)
				SetTorsion( Nb,G, lastCA, lastC, N, CA, lastOmega, omegaArray, omegaArrayCounter );
			}
		}
		else
		{
				if ( lastCA > -1 &&  lastC > 1 && N > -1  && CA > -1)
				SetTorsion( Nb,G, lastCA, lastC, N, CA, lastOmega, omegaArray, omegaArrayCounter );
		}

		if ( !CCapped )
		{
			if ( lastC > -1 &&  N > 1 && CA > -1  && C > -1)
			SetTorsion( Nb,G, lastC, N, CA, C, phi, phiArray, phiArrayCounter );  
		}
	}
	g_free(omegaArray);
	g_free(phiArray);
	/* Pseudo atom */
	G[Nb].X=0.0;
	G[Nb].Y=0.0;
	G[Nb].Z=0.0;

	G[Nb].Charge = 0.0;
	G[Nb].Prop = prop_atom_get("H");
	G[Nb].mmType = g_strdup("H");
	G[Nb].pdbType = g_strdup("H");
	G[Nb].Residue = g_strdup("H");
	G[Nb].ResidueNumber = lastFragNumber+1; 
	G[Nb].N = Nb+1;
	if ( lastCA > -1 && N > -1)
	{
        	G[Nb].X = (G[lastCA].X + G[N].X)/2.0;
        	G[Nb].Y = (G[lastCA].Y + G[N].Y)/2.0;
        	G[Nb].Z = (G[lastCA].Z + G[N].Z)/2.0;
	}
	else
	if ( lastCH3 > -1 && N > -1)
	{
        	G[Nb].X = (G[lastCH3].X + G[N].X)/2.0;
        	G[Nb].Y = (G[lastCH3].Y + G[N].Y)/2.0;
        	G[Nb].Z = (G[lastCH3].Z + G[N].Z)/2.0;
	}
	if(lastC>-1 && lastO>-1)
		SetAngle( Nb+1,G,Nb, lastC, lastO, 180.0, NULL, 0 );
	if ( strcmp(what, "Pro") )
	{
		if ( CA > -1 && lastC > -1)
		{
        	G[Nb].X = (G[CA].X + G[lastC].X)/2.0;
        	G[Nb].Y = (G[CA].Y + G[lastC].Y)/2.0;
        	G[Nb].Z = (G[CA].Z + G[lastC].Z)/2.0;
		if(N>-1 && H > -1)
			SetAngle( Nb+1,G,Nb, N, H, 180.0, NULL, 0 );
		}
	}


	set_connections(Nb-Frag.NAtoms,lastC,N);
	define_geometry_to_draw();
	define_good_factor();
	unselect_all_atoms();

	reset_all_connections();

	reset_charges_multiplicities();
	drawGeom();
	create_GeomXYZ_from_draw_grometry();
	lastFragNumber++;
	lastC = C;
	lastN = N;
	lastCA = CA;
	lastCH3 = CH3;
	lastO = O;
	lastH = H;
	lastOmega = omega;
	lastPsi = psi;
}
/********************************************************************************/
static void delete_last_fragment()
{
	if(lastFragNumber >= 0 && Nb>0) 
	{
		gint i;
		gint k;
		G_CONST_RETURN gchar* t;
		/* delete the last Fragment */
		k = 0;
		for ( i = 0; i < Nb; i++ )
		{
			if(G[i].ResidueNumber != lastFragNumber) continue;
			if(G[i].pdbType) g_free(G[i].pdbType);
			if(G[i].Prop.symbol) g_free(G[i].Prop.symbol);
			if(G[i].Prop.name) g_free(G[i].Prop.name);
			if(G[i].mmType) g_free(G[i].mmType);
			if(G[i].Residue) g_free(G[i].Residue);
			if(G[i].typeConnections) g_free(G[i].typeConnections);
			k++;
		}
		Nb -= k;
		lastFragNumber--;
		lastC =  -1;
		lastN =  -1;
		lastCA =  -1;
		lastCH3 =  -1;
		lastO =  -1;
		lastH =  -1;
		for ( i = 0; i < Nb; i++ )
		{
			if(G[i].ResidueNumber != lastFragNumber) continue;
			if(!strcmp(G[i].pdbType,"C")) lastC =  i;
			else if(!strcmp(G[i].pdbType,"N")) lastN =  i;
			else if(!strcmp(G[i].pdbType,"CA")) lastCA =  i;
			else if(!strcmp(G[i].pdbType,"CH3")) lastCH3 =  i;
			else if(!strcmp(G[i].pdbType,"O")) lastO =  i;
			else if(!strcmp(G[i].pdbType,"H")) lastH =  i;
		}
		i = Nb-1;
		if(i>=0)
		{
			if(!strcmp(G[i].Residue , "NME")) { CA = CH3;}
			if(!strcmp(G[i].Residue , "NHE")) { CA = N;}
			if(!strcmp(G[i].Residue , "ACE")) { CA = CH3;}
		}

		re_set_angles(FALSE);
		t = gtk_entry_get_text(GTK_ENTRY(Entries[1]));
		lastPsi = atof(t);
		t = gtk_entry_get_text(GTK_ENTRY(Entries[2]));
		lastOmega = atof(t);
	}
}
/********************************************************************************/
static void reset_n_cap(GtkWidget *button,gpointer data)
{
  	GtkWidget* Dlg = g_object_get_data(G_OBJECT (button), "WinDlg");
	if(Dlg) g_object_set_data(G_OBJECT (Dlg), "N-CAP",(gchar*)data);
}
/********************************************************************************/
static void reset_c_cap(GtkWidget *button,gpointer data)
{
  	GtkWidget* Dlg = g_object_get_data(G_OBJECT (button), "WinDlg");
	if(Dlg) g_object_set_data(G_OBJECT (Dlg), "C-CAP",(gchar*)data);
}
/********************************************************************************/
static void reset_sensitive_undo_button(GtkWidget *Dlg)
{
  	GtkWidget* undoButton = g_object_get_data(G_OBJECT (Dlg), "UndoButton");
	if(!undoButton) return;
	if(Nb<1) gtk_widget_set_sensitive(undoButton, FALSE);
	else gtk_widget_set_sensitive(undoButton, TRUE);
}
/********************************************************************************/
static void reset_sensitive_build_c_capping(GtkWidget *Dlg, gboolean sens)
{
  	GtkWidget* buildCCapping = g_object_get_data(G_OBJECT (Dlg), "BuildCCapping");
	if(!buildCCapping) return;
	gtk_widget_set_sensitive(buildCCapping, sens);
}
/********************************************************************************/
static void undo(GtkWidget *Dlg, gpointer data)
{
  	GtkWidget* frameAminoAcide = NULL;
	if(!Dlg) return;
	if(Nb>0 && (strlen(G[Nb-1].Residue)<4 || (strlen(G[Nb-1].Residue)==4 && (G[Nb-1].Residue[0]=='N'||G[Nb-1].Residue[0]=='H') ) ) ) 
	{
		delete_last_fragment();
		if( lastFragNumber == 0 && Nb>0 && !strcmp(G[Nb-1].Residue,"ACE")) delete_last_fragment();
	}
	else
	{
		sprintf(fragName,"%s",G[Nb-1].Residue+1);
		delete_last_fragment();
		lowercase(fragName);
		if (GTK_TOGGLE_BUTTON (ButtonL)->active) sprintf(fullFragName,"L%s",fragName);
		else sprintf(fullFragName,"D%s",fragName);
		FreeFragment(&Frag);
		Frag = GetFragmentPPD(fullFragName);
		add_fragment(fragName);
		re_set_angles(TRUE);
	}
	if( lastFragNumber == -1)
	{
  		GtkWidget* frameNCap = g_object_get_data(G_OBJECT (Dlg), "FrameNCap");
		if(frameNCap) gtk_widget_set_sensitive(frameNCap, TRUE);
	}
	define_geometry_to_draw();
	if(Nb>0) define_good_factor();
	unselect_all_atoms();
	reset_all_connections();
	reset_charges_multiplicities();
	drawGeom();
	create_GeomXYZ_from_draw_grometry();

	frameAminoAcide = g_object_get_data(G_OBJECT (Dlg), "FrameAminoAcide");
	if(frameAminoAcide) gtk_widget_set_sensitive(frameAminoAcide, TRUE);
	reset_sensitive_undo_button(Dlg);

	if( lastFragNumber == -1 || Nb<1) reset_sensitive_build_c_capping(Dlg, FALSE);
	else if( lastFragNumber == 0 && strlen(G[Nb-1].Residue)==4) reset_sensitive_build_c_capping(Dlg, FALSE);
	else reset_sensitive_build_c_capping(Dlg, TRUE);
}
/********************************************************************************/
static void build_c_capping(GtkWidget *Dlg,gpointer data)
{
  	GtkWidget* frameAminoAcide = NULL;
	gchar* ccap = NULL;
	gboolean sens = TRUE;
	if(!Dlg) return;
	frameAminoAcide = g_object_get_data(G_OBJECT (Dlg), "FrameAminoAcide");
	ccap = (gchar*) g_object_get_data(G_OBJECT (Dlg), "C-CAP");
	if(!ccap ) return;
	if(!strcmp(ccap,_("Nothing"))) return;
	if(lastFragNumber < 0) return;
	if(!strcmp(ccap,"NME")) 
	{
		sprintf(fragName,"%s","nme");
		lowercase(fragName);
		if (GTK_TOGGLE_BUTTON (ButtonL)->active) sprintf(fullFragName,"L%s",fragName);
		else sprintf(fullFragName,"D%s",fragName);
		FreeFragment(&Frag);
		Frag = GetFragmentPPD(fullFragName);
		add_fragment("Nme");
		re_set_angles(TRUE);
		sens = FALSE;
	}
	else if(!strcmp(ccap,"NHE")) 
	{
		sprintf(fragName,"%s","nhe");
		lowercase(fragName);
		if (GTK_TOGGLE_BUTTON (ButtonL)->active) sprintf(fullFragName,"L%s",fragName);
		else sprintf(fullFragName,"D%s",fragName);
		FreeFragment(&Frag);
		Frag = GetFragmentPPD(fullFragName);
		add_fragment("Nhe");
		re_set_angles(TRUE);
		sens = FALSE;
	}
	else if(lastFragNumber >= 0 && Nb>0 && strlen(G[Nb-1].Residue)<4) 
	{
		if(!strcmp(ccap,"COOH")) sprintf(fragName,"O%s",G[Nb-1].Residue);
		else sprintf(fragName,"C%s",G[Nb-1].Residue);
		delete_last_fragment();
		lowercase(fragName);
		if (GTK_TOGGLE_BUTTON (ButtonL)->active) sprintf(fullFragName,"L%s",fragName);
		else sprintf(fullFragName,"D%s",fragName);
		FreeFragment(&Frag);
		Frag = GetFragmentPPD(fullFragName);
		add_fragment(fragName);
		re_set_angles(TRUE);
		sens = FALSE;
	}
	if(frameAminoAcide) gtk_widget_set_sensitive(frameAminoAcide, sens);
  	reset_sensitive_build_c_capping(Dlg, sens);
}
/********************************************************************************/
static void build_n_capping(GtkWidget *Dlg,gchar* firstResidue)
{
	gchar* ncap = NULL;
	if(!Dlg) return;
	ncap = (gchar*) g_object_get_data(G_OBJECT (Dlg), "N-CAP");
	if(!ncap) return;
	if(!strcmp(ncap,"ACE")) 
	{
		sprintf(fragName,"%s","ace");
		lowercase(fragName);
		if (GTK_TOGGLE_BUTTON (ButtonL)->active) sprintf(fullFragName,"L%s",fragName);
		else sprintf(fullFragName,"D%s",fragName);
		FreeFragment(&Frag);
		Frag = GetFragmentPPD(fullFragName);
		add_fragment("Ace");
		re_set_angles(TRUE);
		sprintf(fragName,"%s",firstResidue);
		lowercase(fragName);
		if (GTK_TOGGLE_BUTTON (ButtonL)->active) sprintf(fullFragName,"L%s",fragName);
		else sprintf(fullFragName,"D%s",fragName);
		FreeFragment(&Frag);
		Frag = GetFragmentPPD(fullFragName);
		add_fragment(firstResidue);
		re_set_angles(TRUE);
  		reset_sensitive_build_c_capping(Dlg, TRUE);
	}
	else if(!strcmp(ncap,_("Nothing"))) 
	{
		sprintf(fragName,"%s",firstResidue);
		lowercase(fragName);
		if (GTK_TOGGLE_BUTTON (ButtonL)->active) sprintf(fullFragName,"L%s",fragName);
		else sprintf(fullFragName,"D%s",fragName);
		FreeFragment(&Frag);
		Frag = GetFragmentPPD(fullFragName);
		add_fragment(firstResidue);
		re_set_angles(TRUE);
  		reset_sensitive_build_c_capping(Dlg, TRUE);
	}
	else
	{
		if(!strcmp(ncap,"NH2")) sprintf(fragName,"H%s",firstResidue);
		else sprintf(fragName,"N%s",firstResidue);
		lowercase(fragName);
		if (GTK_TOGGLE_BUTTON (ButtonL)->active) sprintf(fullFragName,"L%s",fragName);
		else sprintf(fullFragName,"D%s",fragName);
		FreeFragment(&Frag);
		Frag = GetFragmentPPD(fullFragName);
		add_fragment(firstResidue);
		re_set_angles(TRUE);
  		reset_sensitive_build_c_capping(Dlg, TRUE);
	}
}
/********************************************************************************/
static void build_polypeptide(GtkWidget *button,gpointer data)
{
  	GtkWidget* Dlg = g_object_get_data(G_OBJECT (button), "WinDlg");
	if(lastFragNumber == -1)
	{
  		GtkWidget* frameNCap = NULL;
		if(!Dlg) return;
		build_n_capping(Dlg,(gchar*)data);
  		frameNCap = g_object_get_data(G_OBJECT (Dlg), "FrameNCap");
		if(frameNCap) gtk_widget_set_sensitive(frameNCap, FALSE);
		reset_sensitive_undo_button(Dlg);
		return;
	}
	sprintf(fragName,"%s",(gchar*)data);
	lowercase(fragName);
	if (GTK_TOGGLE_BUTTON (ButtonL)->active) sprintf(fullFragName,"L%s",fragName);
	else sprintf(fullFragName,"D%s",fragName);

	FreeFragment(&Frag);
	Frag = GetFragmentPPD(fullFragName);
	add_fragment((gchar*)data);
	re_set_angles(TRUE);
	reset_sensitive_undo_button(Dlg);
  	reset_sensitive_build_c_capping(Dlg, TRUE);
}
/********************************************************************************************************/
static void traite_conformation(GtkComboBox *combobox, gpointer d)
{
	gint i;
	GtkTreeIter iter;
	gchar* option = NULL;
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &option, -1);
	}
	if(!option) return;
	ibuffer = 0;
	sprintf(conf,"%s",option);
	if(!strcmp(option,"Other"))
		for(i=0;i<3;i++)
		{
  			gtk_editable_set_editable((GtkEditable*) Entries[i],TRUE);
		}
	else
		for(i=0;i<3;i++)
		{
  			gtk_editable_set_editable((GtkEditable*) Entries[i],FALSE);
		}

	if ( !strcmp(option, "3-10 Helix" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-49.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-26.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");
		nbuffer = 0;
		return ;
	}	
	else if ( !strcmp(option, "Alpha Helix" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");
		nbuffer = 0;
		return ;
	}
	else if ( !strcmp(option, "Alpha Helix (LH)" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");
		nbuffer = 0;
		return ;
	}
	else if ( !strcmp(option, "Beta Sheet (anti-prl)" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-139.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"135.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"-178.0");
		nbuffer = 0;
		return ;
	}	
	else if ( !strcmp(option, "Beta Sheet (parallel)" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-119.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"113.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"-178.0");
		nbuffer = 0;
		return ;
	}	
	else if ( !strcmp(option, "Omega Helix" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"64.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"55.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");
		nbuffer = 0;
		return ;
	}
	else if ( !strcmp(option, "Pi Helix" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-70.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");
		nbuffer = 0;
		return ;
	}
	else if ( !strcmp(option, "Polyglycine II" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"80.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-150.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");
		nbuffer = 0;
		return ;
	}
	else if ( !strcmp(option, "Polyglycine II (LH)" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-80.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"150.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");
		nbuffer = 0;
		return ;
	}
	else if ( !strcmp(option, "Polyproline I" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-83.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"158.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"0.0");
		nbuffer = 0;
		return ;
	}
	else if ( !strcmp(option, "Polyproline II" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-80.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"150.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");
		nbuffer = 0;
		return ;
	}
	else if ( !strcmp(option, "Other" ))
	{
		nbuffer = 0;
		return ;
	}
	else if ( !strcmp(option, "Extended" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"180.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"180.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");
		nbuffer = 0;
		return ;
	}	
	else if ( !strcmp(option, "Gamma Turn" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"180.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"180.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 3;

		phiBuffer[ 0 ] = 180.0;
		phiBuffer[ 1 ] = 75.0;
		phiBuffer[ 2 ] = 180.0;

		psiBuffer[ 0 ] = 180.0;
		psiBuffer[ 1 ] = -65.0;
		psiBuffer[ 2 ] = 180.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		return ;
	}	
	else if ( !strcmp(option, "Gamma Turn (rev)" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 3;


		phiBuffer[ 0 ] = -57.0;
		phiBuffer[ 1 ] = -75.0;
		phiBuffer[ 2 ] = 180.0;

		psiBuffer[ 0 ] = -47.0;
		psiBuffer[ 1 ] = 65.0;
		psiBuffer[ 2 ] = 180.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		return ;
	}	
	else if ( !strcmp(option, "Beta 1 Turn" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 4;


		phiBuffer[ 0 ] = -57.0;
		phiBuffer[ 1 ] = -60.0;
		phiBuffer[ 2 ] = -90.0;
		phiBuffer[ 3 ] = -57.0;

		psiBuffer[ 0 ] = -47.0;
		psiBuffer[ 1 ] = -30.0;
		psiBuffer[ 2 ] = 0.0;
		psiBuffer[ 3 ] = -47.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		omegaBuffer[ 3 ] = 180.0;
		return ;
	}	
	else if ( !strcmp(option, "Beta 2 Turn" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 4;


		phiBuffer[ 0 ] = -57.0;
		phiBuffer[ 1 ] = -60.0;
		phiBuffer[ 2 ] = 80.0;
		phiBuffer[ 3 ] = -57.0;

		psiBuffer[ 0 ] = -47.0;
		psiBuffer[ 1 ] = 120.0;
		psiBuffer[ 2 ] = 0.0;
		psiBuffer[ 3 ] = -47.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		omegaBuffer[ 3 ] = 180.0;
		return ;
	}	
	else if ( !strcmp(option, "Beta 3 Turn" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 4;


		phiBuffer[ 0 ] = -57.0;
		phiBuffer[ 1 ] = -60.0;
		phiBuffer[ 2 ] = -60.0;
		phiBuffer[ 3 ] = -57.0;

		psiBuffer[ 0 ] = -47.0;
		psiBuffer[ 1 ] = -30.0;
		psiBuffer[ 2 ] = -30.0;
		psiBuffer[ 3 ] = -47.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		omegaBuffer[ 3 ] = 180.0;
		return ;
	}	
	else if ( !strcmp(option, "Beta 5 Turn" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 4;


		phiBuffer[ 0 ] = -57.0;
		phiBuffer[ 1 ] = -80.0;
		phiBuffer[ 2 ] = -80.0;
		phiBuffer[ 3 ] = -57.0;

		psiBuffer[ 0 ] = -47.0;
		psiBuffer[ 1 ] = 80.0;
		psiBuffer[ 2 ] = 80.0;
		psiBuffer[ 3 ] = -47.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		omegaBuffer[ 3 ] = 180.0;
		return ;
	}	
	else if ( !strcmp(option, "Beta 6a Turn" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 4;


		phiBuffer[ 0 ] = -57.0;
		phiBuffer[ 1 ] = -60.0;
		phiBuffer[ 2 ] = -90.0;
		phiBuffer[ 3 ] = -57.0;

		psiBuffer[ 0 ] = -47.0;
		psiBuffer[ 1 ] = 120.0;
		psiBuffer[ 2 ] = 0.0;
		psiBuffer[ 3 ] = -47.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		omegaBuffer[ 3 ] = 180.0;
		return ;
	}	
	else if ( !strcmp(option, "Beta 6b Turn" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 4;


		phiBuffer[ 0 ] = -57.0;
		phiBuffer[ 1 ] = -120.0;
		phiBuffer[ 2 ] = -60.0;
		phiBuffer[ 3 ] = -57.0;

		psiBuffer[ 0 ] = -47.0;
		psiBuffer[ 1 ] = 120.0;
		psiBuffer[ 2 ] = 0.0;
		psiBuffer[ 3 ] = -47.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		omegaBuffer[ 3 ] = 180.0;
		return ;
	}	
	else if ( !strcmp(option, "Beta 8 Turn" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 4;


		phiBuffer[ 0 ] = -57.0;
		phiBuffer[ 1 ] = -60.0;
		phiBuffer[ 2 ] = -120.0;
		phiBuffer[ 3 ] = -57.0;

		psiBuffer[ 0 ] = -47.0;
		psiBuffer[ 1 ] = -30.0;
		psiBuffer[ 2 ] = 120.0;
		psiBuffer[ 3 ] = -47.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		omegaBuffer[ 3 ] = 180.0;
		return ;
	}	
	else if ( !strcmp(option, "Beta 1' Turn" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 4;


		phiBuffer[ 0 ] = -57.0;
		phiBuffer[ 1 ] = 60.0;
		phiBuffer[ 2 ] = 90.0;
		phiBuffer[ 3 ] = -57.0;

		psiBuffer[ 0 ] = -47.0;
		psiBuffer[ 1 ] = 30.0;
		psiBuffer[ 2 ] = 0.0;
		psiBuffer[ 3 ] = -47.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		omegaBuffer[ 3 ] = 180.0;
		return ;
	}	
	else if ( !strcmp(option, "Beta 2' Turn" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 4;


		phiBuffer[ 0 ] = -57.0;
		phiBuffer[ 1 ] = 60.0;
		phiBuffer[ 2 ] = -80.0;
		phiBuffer[ 3 ] = -57.0;

		psiBuffer[ 0 ] = -47.0;
		psiBuffer[ 1 ] = -120.0;
		psiBuffer[ 2 ] = 0.0;
		psiBuffer[ 3 ] = -47.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		omegaBuffer[ 3 ] = 180.0;
		return ;
	}	
	else if ( !strcmp(option, "Beta 3' Turn" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 4;


		phiBuffer[ 0 ] = -57.0;
		phiBuffer[ 1 ] = 60.0;
		phiBuffer[ 2 ] = 60.0;
		phiBuffer[ 3 ] = -57.0;

		psiBuffer[ 0 ] = -47.0;
		psiBuffer[ 1 ] = 30.0;
		psiBuffer[ 2 ] = 30.0;
		psiBuffer[ 3 ] = -47.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		omegaBuffer[ 3 ] = 180.0;
		return ;
	}	
	else if ( !strcmp(option, "Beta 5' Turn" ))
	{
		gtk_entry_set_text(GTK_ENTRY(Entries[0]),"-57.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[1]),"-47.0");
		gtk_entry_set_text(GTK_ENTRY(Entries[2]),"180.0");

		nbuffer = 4;


		phiBuffer[ 0 ] = -57.0;
		phiBuffer[ 1 ] = 80.0;
		phiBuffer[ 2 ] = 80.0;
		phiBuffer[ 3 ] = -57.0;

		psiBuffer[ 0 ] = -47.0;
		psiBuffer[ 1 ] = -80.0;
		psiBuffer[ 2 ] = -80.0;
		psiBuffer[ 3 ] = -47.0;

		omegaBuffer[ 0 ] = 180.0;
		omegaBuffer[ 1 ] = 180.0;
		omegaBuffer[ 2 ] = 180.0;
		omegaBuffer[ 3 ] = 180.0;
		return ;
	}	
}
/********************************************************************************************************/
static GtkWidget *get_button_conformation()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;

	store = gtk_tree_store_new (1,G_TYPE_STRING);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "3-10 Helix", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Alpha Helix", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Alpha Helix (LH)", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta Sheet (anti-prl)", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta Sheet (parallel)", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta 1 Turn", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta 1' Turn", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta 2 Turn", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta 2' Turn", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta 3 Turn", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta 3' Turn", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta 5 Turn", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta 5' Turn", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta 6a Turn", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta 6b Turn", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Beta 8 Turn", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Extended", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Gamma Turn", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Gamma Turn (rev)", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Omega Helix", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Pi Helix", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Polyglycine II", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Polyproline I", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Polyproline II", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Other", -1);

	sprintf(conf,"Extended");

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
static GtkWidget* add_options(GtkWidget* Dlg,GtkWidget *box)
{
	GtkWidget* Table;
	GtkWidget* frame;
	GtkWidget* combobox;
	GtkWidget* Label;
	gchar* tlabel[] = {"Phi","Psi","Omega"};
	gchar* elabel[] = {"180.0","180.0","180.0"};
	gint i;

	frame = gtk_frame_new ("Conformation");
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);

	gtk_container_add(GTK_CONTAINER(box),frame);  
	gtk_widget_show (frame);

	Table = gtk_table_new(5,3,TRUE);
	gtk_container_add(GTK_CONTAINER(frame),Table);
  
	combobox =get_button_conformation();
	gtk_table_attach(GTK_TABLE(Table), combobox,0,3,0,1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	for(i=0;i<3;i++)
	{
		Label = gtk_label_new(tlabel[i]);
		gtk_table_attach(GTK_TABLE(Table),Label,0,1,i+1,i+2,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
		Entries[i] = gtk_entry_new();
		gtk_table_attach(GTK_TABLE(Table),Entries[i],1,3,i+1,i+2,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  		gtk_editable_set_editable((GtkEditable*) Entries[i],FALSE);
  		gtk_entry_set_text(GTK_ENTRY(Entries[i]),elabel[i]);
	}
	Label = gtk_label_new("Isomer");
	gtk_table_attach(GTK_TABLE(Table),Label,0,1,4,5,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	ButtonL = gtk_radio_button_new_with_label( NULL," L " );
	gtk_table_attach(GTK_TABLE(Table),ButtonL,1,2,4,5,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  	ButtonD = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonL)),
                       " D "); 
	gtk_table_attach(GTK_TABLE(Table),ButtonD,2,3,4,5,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  	g_object_set_data(G_OBJECT (Dlg), "ButtonL",ButtonL);
  	g_object_set_data(G_OBJECT (Dlg), "ButtonD",ButtonD);
	return combobox;
}
/********************************************************************************/
static GtkWidget* add_n_cap(GtkWidget* Dlg,GtkWidget *box)
{
	GtkWidget* table;
	GtkWidget* frame;
	GtkWidget* button1;
	GtkWidget* button;
	static gchar* list[] = {"ACE","NH3+","NH2",N_("Nothing")};
	gint i;
	gint nOptions = G_N_ELEMENTS (list);

	frame = gtk_frame_new ("N-cap");
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);

	gtk_container_add(GTK_CONTAINER(box),frame);  
	gtk_widget_show (frame);

	table = gtk_table_new(nOptions/2+nOptions%2,2,TRUE);
	gtk_container_add(GTK_CONTAINER(frame),table);
  
	button1 = NULL;
	for(i=0;i<nOptions;i++)
	{
		if(i==0) 
		{
			button = gtk_radio_button_new_with_label(NULL, list[i]);
			button1 = button;
		}
		else button = gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (button1)), list[i]);
		gtk_table_attach(GTK_TABLE(table),button,i%2,i%2+1,i/2,i/2+1,
			  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  	(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  	1,1);
  		g_object_set_data(G_OBJECT (button), "Name",list[i]);
  		g_object_set_data(G_OBJECT (button), "WinDlg",Dlg);
          	g_signal_connect(G_OBJECT(button), "clicked", (GCallback)reset_n_cap,(gpointer )list[i]);
	}
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  	g_object_set_data(G_OBJECT (Dlg), "N-CAP",list[nOptions-1]);
  	g_object_set_data(G_OBJECT (Dlg), "FrameNCap",frame);
	return frame;
}
/********************************************************************************/
static GtkWidget* add_c_cap(GtkWidget* Dlg,GtkWidget *box)
{
	GtkWidget* table;
	GtkWidget* frame;
	GtkWidget* button1;
	GtkWidget* button;
	static gchar* list[] = {"NME","NHE","COO-","COOH",N_("Nothing")};
	gint i;
	gint nOptions = G_N_ELEMENTS (list);

	frame = gtk_frame_new ("C-cap");
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);

	gtk_container_add(GTK_CONTAINER(box),frame);  
	gtk_widget_show (frame);

	table = gtk_table_new(nOptions/2+nOptions%2,2,TRUE);
	gtk_container_add(GTK_CONTAINER(frame),table);
  
	button1 = NULL;
	for(i=0;i<nOptions;i++)
	{
		if(i==0) 
		{
			button = gtk_radio_button_new_with_label(NULL, list[i]);
			button1 = button;
		}
		else button = gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (button1)), list[i]);
		gtk_table_attach(GTK_TABLE(table),button,i%2,i%2+1,i/2,i/2+1,
			  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  	(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  	1,1);
  		g_object_set_data(G_OBJECT (button), "Name",list[i]);
  		g_object_set_data(G_OBJECT (button), "WinDlg",Dlg);
          	g_signal_connect(G_OBJECT(button), "clicked", (GCallback)reset_c_cap,(gpointer )list[i]);
	}
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
  	g_object_set_data(G_OBJECT (Dlg), "C-CAP",list[nOptions-1]);
	return frame;
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

	static char *Symb[ColonneT][LigneT]={
		{"Ala","Cys","Gly","His","Met","Thr","Ash"},
		{"Arg","Cyx","Hid","Ile","Phe","Tyr","Glh"},
		{"Asn","Gln","Hie","Leu","Pro","Trp","Lyn"},
		{"Asp","Glu","Hip","Lys","Ser","Val","Cym"},
		};
	static char *SymbOne[ColonneT][LigneT]={
		{"A","C","G","H","M","T","D"},
		{"R","C","H","I","F","Y","E"},
		{"N","Q","H","L","P","W","K"},
		{"D","E","H","K","S","V","C"},
		};

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 1);

  gtk_container_add(GTK_CONTAINER(box),frame);  
  gtk_widget_show (frame);

  Table = gtk_table_new(LigneT,ColonneT,TRUE);
  gtk_container_add(GTK_CONTAINER(frame),Table);
  button_style = gtk_widget_get_style(Dlg); 
  
  for ( i = 0;i<LigneT;i++)
	  for ( j = 0;j<ColonneT;j++)
  {
	  if(strcmp(Symb[j][i],"00"))
	  {
		gchar tmp[100];
		if(strcmp(SymbOne[j][i],"00"))
			sprintf(tmp,"%s (%s)",Symb[j][i],SymbOne[j][i]);
		else
			sprintf(tmp,"%s",Symb[j][i]);
	  	button = gtk_button_new_with_label(tmp);
  		g_object_set_data(G_OBJECT (button), "WinDlg",Dlg);

          	style=set_button_style(button_style,button,"H");
          	g_signal_connect(G_OBJECT(button), "clicked",
                            (GCallback)build_polypeptide,(gpointer )Symb[j][i]);
	  	gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	  }

  }
  g_object_set_data(G_OBJECT (Dlg), "FrameAminoAcide",frame);
  
}
/**********************************************************************/
void build_polypeptide_dlg()
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
  gtk_window_set_title(GTK_WINDOW(Dlg),_("Build PolyPeptide molecule"));
  gtk_window_set_modal (GTK_WINDOW (Dlg), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(Dlg),GTK_WINDOW(GeomDlg));


  add_child(GeomDlg,Dlg,gtk_widget_destroy,_(" Build PolyPep. mol. "));

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
  combobox = add_options(Dlg,hbox);
  vbox = create_vbox_false(hbox);
  add_n_cap(Dlg,vbox);
  add_c_cap(Dlg,vbox);



  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dlg)->action_area), FALSE);
  gtk_widget_realize(Dlg);

  Button = gtk_button_new_with_mnemonic("_Undo");
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, FALSE, 5);  
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)undo,GTK_OBJECT(Dlg));
  g_object_set_data(G_OBJECT (Dlg), "UndoButton",Button);
  reset_sensitive_undo_button(Dlg);

  Button = create_button(Dlg,_("Add C-capping"));
  gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, FALSE, 5);  
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)build_c_capping,GTK_OBJECT(Dlg));
  g_object_set_data(G_OBJECT (Dlg), "BuildCCapping",Button);
  reset_sensitive_build_c_capping(Dlg, FALSE);

  Button = create_button(Dlg,_("Close"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 2);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)destroy_dlg,GTK_OBJECT(Dlg));

  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->vbox);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->action_area);
  gtk_widget_show_now(Dlg);

  /* fit_windows_position(GeomDlg, Dlg);*/

  gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 16);
}

