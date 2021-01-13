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
#include "../Common/Global.h"
#include "GlobalOrb.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Display/StatusOrb.h"
#include "../Display/GLArea.h"
#include "../Display/AnimationMD.h"
#include "../Display/GeomOrbXYZ.h"
#include "../Display/AtomicOrbitals.h"
#include "../Files/FileChooser.h"
#include "../Geometry/GeomGlobal.h"
#include "../Files/FolderChooser.h"
#include "../Files/GabeditFolderChooser.h"
#include "../Common/Help.h"
#include "../Common/StockIcons.h"
#include "../Display/PovrayGL.h"
#include "../Display/Images.h"
#include "../Display/UtilsOrb.h"
#include "../Display/BondsOrb.h"
#include "../Display/RingsOrb.h"
#include "../Utils/GabeditXYPlot.h"
#include "../../pixmaps/Open.xpm"

/* extern AnimationMD.h */
GeometriesMD geometriesMD;
/*********************************/

static	GtkWidget *WinDlg = NULL;
static	GtkWidget *EntryVelocity = NULL;
static	GtkWidget *PlayButton = NULL;
static	GtkWidget *StopButton = NULL;
static	GtkTreeView *treeView = NULL;
static gboolean play = FALSE;

static GtkWidget *buttonCheckFilm = NULL;
static GtkWidget *buttonDirFilm = NULL;
static GtkWidget* comboListFilm = NULL;
static gboolean createFilm = FALSE;
static gint numFileFilm = 0;
static gchar formatFilm[100] = "BMP";

static gint rowSelected = -1;

static gchar* inputGaussDirectory = NULL;
static gint spinMultiplicity=1;
static gint totalCharge = 0;
static GtkWidget* buttonChkgauss = NULL;

/********************************************************************************/
static void animate();
static void computeConformerTypes();
static void rafreshList();
static void stopAnimation(GtkWidget *win, gpointer data);
static void playAnimation(GtkWidget *win, gpointer data);
static gboolean set_geometry(gint k);
static GtkWidget* addComboListToATable(GtkWidget* table, gchar** list, gint nlist, gint i, gint j, gint k);
void  add_cancel_ok_button(GtkWidget *Win,GtkWidget *vbox,GtkWidget *entry, GCallback myFunc);
static void print_gaussian_geometries(GtkWidget* Win, gpointer data);

/************************************************************************************************************/
static void setComboReference(GtkWidget *comboReference)
{
	GList *glist = NULL;
	gchar** list = NULL;
	gint n = 0;

	if(geometriesMD.numberOfGeometries>0)
	{
		gint i;
		list = g_malloc(geometriesMD.numberOfGeometries*sizeof(gchar*));
		n = geometriesMD.numberOfGeometries;
		if(n>10)
		{ 
			n = 10; 
			gtk_editable_set_editable((GtkEditable*)GTK_BIN (comboReference)->child,TRUE);
			gtk_widget_set_sensitive(GTK_BIN (comboReference)->child, TRUE);
		}
  		for(i=0;i<n;i++) 
		{
			list[i] = g_strdup_printf("%d",i+1);
			glist = g_list_append(glist,list[i]);
		}
	}

  	glist = g_list_append(glist,"Average");

  	gtk_combo_box_entry_set_popdown_strings(comboReference, glist) ;

	if(list && n>0) 
	{
		gint i;
		for(i=0;i<n;i++) g_free(list[i]);
		g_free(list);
	}
  	g_list_free(glist);
}
/************************************************************************************************************/
static void setComboRMSDMethod(GtkWidget *comboRMSDMethod)
{
	GList *glist = NULL;

  	glist = g_list_append(glist,"All atoms");
  	glist = g_list_append(glist,"Non-hydrogen atoms");
  	glist = g_list_append(glist,"Selected atoms by numbers");
  	glist = g_list_append(glist,"Selected atoms by symbols");
  	glist = g_list_append(glist,"Selected atoms by MM types");
  	glist = g_list_append(glist,"Selected atoms by PDB types");

  	gtk_combo_box_entry_set_popdown_strings(comboRMSDMethod, glist) ;

  	g_list_free(glist);
}
/********************************************************************************/
static void setEntryRMSDAtomsList(GtkWidget *entry)
{
	GList *glist = NULL;
	GtkWidget *comboRMSDMethod = g_object_get_data(G_OBJECT (entry), "ComboRMSDMethod");
	G_CONST_RETURN gchar* entryText = NULL;
	GtkWidget *entryCombo =  GTK_BIN (comboRMSDMethod)->child;
	entryText = gtk_entry_get_text(GTK_ENTRY(entryCombo));
	gchar** list = NULL;
	gint n = 0;

	if(entryText && strstr(entryText,"All atoms")) gtk_entry_set_text(GTK_ENTRY(entry),"");
	else if(entryText && strstr(entryText,"Non")) gtk_entry_set_text(GTK_ENTRY(entry),"");
	else if(entryText && strstr(entryText,"numbers")&&geometriesMD.numberOfGeometries>0)
	{
		gint i;
                gint nAtoms = geometriesMD.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = nAtoms;
  		for(i=0;i<n;i++) list[i] = g_strdup_printf("%d ",i+1);
	}
	else if(entryText && strstr(entryText,"symbol")&&geometriesMD.numberOfGeometries>0)
	{
		gint i,j;
                gint nAtoms = geometriesMD.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = 0;
  		for(i=0;i<nAtoms;i++) 
		{
			gchar* t = geometriesMD.geometries[0].listOfAtoms[i].symbol;
			for(j=0;j<n;j++)
			{
				gchar tmp[100];
				sprintf(tmp,"%s ",t);
				if(!strcmp(tmp,list[j])) break;
			}
			if(j==n) list[n++] = g_strdup_printf("%s ",t);
		}
	}
	else if(entryText && strstr(entryText,"MM")&&geometriesMD.numberOfGeometries>0)
	{
		gint i,j;
                gint nAtoms = geometriesMD.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = 0;
  		for(i=0;i<nAtoms;i++) 
		{
			gchar* t = geometriesMD.geometries[0].listOfAtoms[i].mmType;
			for(j=0;j<n;j++)
			{
				gchar tmp[100];
				sprintf(tmp,"%s ",t);
				if(!strcmp(tmp,list[j])) break;
			}
			if(j==n) list[n++] = g_strdup_printf("%s ",t);
		}
	}
	else if(entryText && strstr(entryText,"PDB")&&geometriesMD.numberOfGeometries>0)
	{
		gint i,j;
                gint nAtoms = geometriesMD.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = 0;
  		for(i=0;i<nAtoms;i++) 
		{
			gchar* t = geometriesMD.geometries[0].listOfAtoms[i].pdbType;
			for(j=0;j<n;j++)
			{
				gchar tmp[100];
				sprintf(tmp,"%s ",t);
				if(!strcmp(tmp,list[j])) break;
			}
			if(j==n) list[n++] = g_strdup_printf("%s ",t);
		}
	}

	if(list && n>0) 
	{
		gint i;
		gint pos;
		gtk_widget_set_sensitive(entry, TRUE);
		gtk_entry_set_text(GTK_ENTRY(entry),"");
		for(i=0;i<(n>10?10:n);i++) gtk_editable_insert_text(GTK_EDITABLE(entry), list[i], -1, &pos);
		for(i=0;i<n;i++) g_free(list[i]);
		g_free(list);
	}
	else gtk_widget_set_sensitive(entry, FALSE);
	if(glist) g_list_free(glist);
}
/**********************************************************************/
static void changedRMSDMethod(GtkWidget *entry, gpointer data)
{
	GtkWidget* entryList = NULL;
	entryList  = g_object_get_data(G_OBJECT (entry), "EntryRMSDAtomsList");
	setEntryRMSDAtomsList(entryList);
}
/***********************************************************************************************/
static GtkWidget *addReferenceToTable(GtkWidget *table, gint i)
{
	GtkWidget* entry = NULL;
	GtkWidget* combo = NULL;
	gint nlist = 1;
	gchar* list[] = {"1"};

	entry = addComboListToATable(table, list, nlist, i, 2, 1);
	combo  = g_object_get_data(G_OBJECT (entry), "Combo");
	gtk_widget_set_sensitive(entry, FALSE);

	return combo;
}
/***********************************************************************************************/
static GtkWidget *addComboRMSDMethodToTable(GtkWidget *table, gint i)
{
	GtkWidget* entry = NULL;
	GtkWidget* combo = NULL;
	gint nlist = 1;
	gchar* list[] = {"All atoms"};

	entry = addComboListToATable(table, list, nlist, i, 2, 1);
	combo  = g_object_get_data(G_OBJECT (entry), "Combo");
	gtk_widget_set_sensitive(entry, FALSE);

	return combo;
}
/********************************************************************************/
static GtkWidget*   add_input_rmsd_entrys(GtkWidget *Wins,GtkWidget *vbox)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(7,4,FALSE);
	GtkWidget* comboReference = NULL;
	GtkWidget* comboRMSMethod = NULL;
	gint i;

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	i = 0;
	add_label_table(table,_(" Reference "),i,0);
	add_label_table(table,":",i,1);
	comboReference = addReferenceToTable(table, i);
	i = 1;
	add_label_table(table,_(" Method "),i,0);
	add_label_table(table,":",i,1);
	comboRMSMethod = addComboRMSDMethodToTable(table, i);
	i = 2;
	add_label_table(table,_(" List "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryRMSDAtomsList", entry);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
	i = 3;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
		(GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                  2,2);

	if(GTK_IS_COMBO_BOX(comboReference))
	{
		g_object_set_data(G_OBJECT (GTK_BIN(comboRMSMethod)->child), "EntryRMSDAtomsList", entry);
		g_object_set_data(G_OBJECT (entry), "ComboRMSDMethod", comboRMSMethod);
		setEntryRMSDAtomsList(entry);
		g_object_set_data(G_OBJECT (Wins), "EntryRMSDAtomsList",(entry));
		g_object_set_data(G_OBJECT (Wins), "ComboRMSDMethod",GTK_BIN(comboRMSMethod)->child);
		g_object_set_data(G_OBJECT (Wins), "ComboReference",GTK_BIN(comboReference)->child);
	}
	setComboReference(comboReference);
	setComboRMSDMethod(comboRMSMethod);
	g_signal_connect(G_OBJECT(GTK_BIN(comboRMSMethod)->child),"changed", G_CALLBACK(changedRMSDMethod),NULL);

	gtk_widget_show_all(table);
	entry = GTK_BIN (comboReference)->child;
	return entry;
}
/*************************************************************************************************************/
static void build_rmsd(GtkWidget* Win, gpointer data)
{
	gint g;
	GtkWidget* entryReference = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ComboReference"));	
	GtkWidget* entryMethod = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ComboRMSDMethod"));	
	GtkWidget* entryList = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryRMSDAtomsList"));	
	gdouble** C0 = NULL;
	gint nref;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gint i;
	G_CONST_RETURN gchar* str = NULL;
	GtkWidget* xyplot;
	GtkWidget* window;
	gint N = geometriesMD.numberOfGeometries;

	if(geometriesMD.numberOfGeometries<1) return;
	if(geometriesMD.geometries[0].numberOfAtoms<1) return;
	

	if(entryReference) str = gtk_entry_get_text(GTK_ENTRY(entryReference));
	if(!str) return;
	if(strstr(str,"Aver")) nref = -1;
	else nref = atof(str)-1;
	C0 = g_malloc(geometriesMD.geometries[0].numberOfAtoms*sizeof(gdouble*));
	for(i=0;i<geometriesMD.geometries[0].numberOfAtoms;i++) C0[i] = g_malloc(3*sizeof(gdouble));
	if(nref>=0)
	{
		gint j;
		for(i=0;i<geometriesMD.geometries[0].numberOfAtoms;i++) 
		for(j=0;j<3;j++)
			C0[i][j] = geometriesMD.geometries[nref].listOfAtoms[i].C[j];
	}
	else
	{
		gint j;
		for(i=0;i<geometriesMD.geometries[0].numberOfAtoms;i++) 
		for(j=0;j<3;j++)
			C0[i][j] = 0;
		for(g = 0;g<geometriesMD.numberOfGeometries;g++)
		for(i=0;i<geometriesMD.geometries[0].numberOfAtoms;i++) 
		for(j=0;j<3;j++)
			C0[i][j] += geometriesMD.geometries[g].listOfAtoms[i].C[j];
		for(i=0;i<geometriesMD.geometries[0].numberOfAtoms;i++) 
		for(j=0;j<3;j++)
			C0[i][j] /= geometriesMD.numberOfGeometries;
	}

	if(entryMethod) str = gtk_entry_get_text(GTK_ENTRY(entryMethod));
	if(!str) return;

	X = g_malloc(N*sizeof(gdouble));
	Y = g_malloc(N*sizeof(gdouble));
	for(i=0;i<N;i++) X[i] = i+1;
	for(i=0;i<N;i++) Y[i] = 0;
	if(strstr(str,"All atoms"))
	{
		for(g = 0;g<geometriesMD.numberOfGeometries;g++)
		{
			gdouble d = 0;
			gdouble xx = 0;
			gint j;

			gint a;

			for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
			for(j=0;j<3;j++) 
			{
				xx = geometriesMD.geometries[g].listOfAtoms[a].C[j]-C0[a][j];
				d+= xx*xx;
			}
			d = sqrt(d)*BOHR_TO_ANG;
			d /=geometriesMD.geometries[g].numberOfAtoms;
			Y[g] = d;
		}
	}
	else if(strstr(str,"Non"))
	{
		for(g = 0;g<geometriesMD.numberOfGeometries;g++)
		{
			gdouble d = 0;
			gdouble xx = 0;
			gint j;

			gint a;
			gint k = 0;

			for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
			if(strcmp( geometriesMD.geometries[g].listOfAtoms[a].symbol,"H"))
			for(j=0;j<3;j++) 
			{
				xx = geometriesMD.geometries[g].listOfAtoms[a].C[j]-C0[a][j];
				d+= xx*xx;
				k++;
			}
			d = sqrt(d)*BOHR_TO_ANG;
			if(k>0) d /=k/3.0;
			Y[g] = d;
		}
	}
	else if(strstr(str,"symbol"))
	{
		str = gtk_entry_get_text(GTK_ENTRY(entryList));
		for(g = 0;g<geometriesMD.numberOfGeometries;g++)
		{
			gdouble d = 0;
			gdouble xx = 0;
			gint j;

			gint a;
			gint k = 0;

			for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
			{
				gchar tmp[100];
				gchar tmp2[10000];
				sprintf(tmp2,"%s ",str);
				sprintf(tmp,"%s ",geometriesMD.geometries[g].listOfAtoms[a].symbol);
				if(strstr(tmp2,tmp))
				for(j=0;j<3;j++) 
				{
					xx = geometriesMD.geometries[g].listOfAtoms[a].C[j]-C0[a][j];
					d+= xx*xx;
					k++;
				}
			}
			d = sqrt(d)*BOHR_TO_ANG;
			if(k>0) d /=k/3.0;
			Y[g] = d;
		}
	}
	else if(strstr(str,"number"))
	{
		str = gtk_entry_get_text(GTK_ENTRY(entryList));
		for(g = 0;g<geometriesMD.numberOfGeometries;g++)
		{
			gdouble d = 0;
			gdouble xx = 0;
			gint j;

			gint a;
			gint k = 0;

			for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
			{
				gchar tmp[100];
				gchar tmp2[10000];
				sprintf(tmp2,"%s ",str);
				sprintf(tmp,"%d ",a+1);
				if(strstr(tmp2,tmp))
				for(j=0;j<3;j++) 
				{
					xx = geometriesMD.geometries[g].listOfAtoms[a].C[j]-C0[a][j];
					d+= xx*xx;
					k++;
				}
			}
			d = sqrt(d)*BOHR_TO_ANG;
			if(k>0) d /=k/3.0;
			Y[g] = d;
		}
	}
	else if(strstr(str,"MM"))
	{
		str = gtk_entry_get_text(GTK_ENTRY(entryList));
		for(g = 0;g<geometriesMD.numberOfGeometries;g++)
		{
			gdouble d = 0;
			gdouble xx = 0;
			gint j;

			gint a;
			gint k = 0;

			for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
			{
				gchar tmp[100];
				gchar tmp2[10000];
				sprintf(tmp2,"%s ",str);
				sprintf(tmp,"%s ",geometriesMD.geometries[g].listOfAtoms[a].mmType);
				if(strstr(tmp2,tmp))
				for(j=0;j<3;j++) 
				{
					xx = geometriesMD.geometries[g].listOfAtoms[a].C[j]-C0[a][j];
					d+= xx*xx;
					k++;
				}
			}
			d = sqrt(d)*BOHR_TO_ANG;
			if(k>0) d /=k/3.0;
			Y[g] = d;
		}
	}
	else if(strstr(str,"PDB"))
	{
		str = gtk_entry_get_text(GTK_ENTRY(entryList));
		for(g = 0;g<geometriesMD.numberOfGeometries;g++)
		{
			gdouble d = 0;
			gdouble xx = 0;
			gint j;

			gint a;
			gint k = 0;

			for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
			{
				gchar tmp[100];
				gchar tmp2[10000];
				sprintf(tmp2,"%s ",str);
				sprintf(tmp,"%s ",geometriesMD.geometries[g].listOfAtoms[a].pdbType);
				if(strstr(tmp2,tmp))
				for(j=0;j<3;j++) 
				{
					xx = geometriesMD.geometries[g].listOfAtoms[a].C[j]-C0[a][j];
					d+= xx*xx;
					k++;
				}
			}
			d = sqrt(d)*BOHR_TO_ANG;
			if(k>0) d /=k/3.0;
			Y[g] = d;
		}
	}

	gtk_widget_destroy(Win);

	window = gabedit_xyplot_new_window(_("RMSD"),NULL);
	xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
	gabedit_xyplot_add_data_conv(GABEDIT_XYPLOT(xyplot),N, X,  Y, 1.0, GABEDIT_XYPLOT_CONV_NONE,NULL);
	gabedit_xyplot_set_range_xmin (GABEDIT_XYPLOT(xyplot), 0.0);
	gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), "Frame");
	gabedit_xyplot_set_y_label (GABEDIT_XYPLOT(xyplot), "RMSD (Ang)");
	if(C0) for(i=0;i<geometriesMD.geometries[0].numberOfAtoms;i++) g_free(C0[i]);
	if(C0) g_free(C0);
	g_free(X); 
	g_free(Y);
}
/********************************************************************************************************/
static void create_rmsd_dlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget* vbox;
	GtkWidget* entry;


	/* Principal Window */
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"compute pair radial distribution");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"gr");
	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vboxall = create_vbox(Win);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(vboxall), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
  	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

  	gtk_widget_realize(Win);
	entry =  add_input_rmsd_entrys(Win,vbox);
	add_cancel_ok_button(Win,vbox,entry,(GCallback)build_rmsd);

	/* Show all */
	gtk_widget_show_all (Win);
}
/************************************************************************************************************/
static void setComboMethod(GtkWidget *comboMethod)
{
	GList *glist = NULL;

  	glist = g_list_append(glist,"Number");
  	glist = g_list_append(glist,"Symbol");
  	glist = g_list_append(glist,"MM Type");
  	glist = g_list_append(glist,"PDB Type");

  	gtk_combo_box_entry_set_popdown_strings(comboMethod, glist) ;

  	g_list_free(glist);
}
/********************************************************************************/
static void setComboVal(GtkWidget *comboVal)
{
	GList *glist = NULL;
	GtkWidget *comboMethod = g_object_get_data(G_OBJECT (comboVal), "ComboMethod");
	G_CONST_RETURN gchar* entryText = NULL;
	GtkWidget *entry =  GTK_BIN (comboMethod)->child;
	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	gchar** list = NULL;
	gint n = 0;

	if(entryText && strstr(entryText,"Number")&&geometriesMD.numberOfGeometries>0)
	{
		gint i;
                gint nAtoms = geometriesMD.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = nAtoms;
  		for(i=0;i<nAtoms;i++) 
			list[i] = g_strdup_printf("%d",i+1);
  		for(i=0;i<n;i++) 
			glist = g_list_append(glist,list[i]);
	}
	if(entryText && strstr(entryText,"Symbol")&&geometriesMD.numberOfGeometries>0)
	{
		gint i,j;
                gint nAtoms = geometriesMD.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = 0;
  		for(i=0;i<nAtoms;i++) 
		{
			gchar* t = geometriesMD.geometries[0].listOfAtoms[i].symbol;
			for(j=0;j<n;j++)
			{
				if(!strcmp(t,list[j])) break;
			}
			if(j==n)
				list[n++] = g_strdup(t);
		}
  		for(i=0;i<n;i++) 
			glist = g_list_append(glist,list[i]);
	}
	if(entryText && strstr(entryText,"MM Type")&&geometriesMD.numberOfGeometries>0)
	{
		gint i,j;
                gint nAtoms = geometriesMD.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = 0;
  		for(i=0;i<nAtoms;i++) 
		{
			gchar* t = geometriesMD.geometries[0].listOfAtoms[i].mmType;
			for(j=0;j<n;j++)
			{
				if(!strcmp(t,list[j])) break;
			}
			if(j==n)
				list[n++] = g_strdup(t);
		}
  		for(i=0;i<n;i++) 
			glist = g_list_append(glist,list[i]);
	}
	if(entryText && strstr(entryText,"PDB Type")&&geometriesMD.numberOfGeometries>0)
	{
		gint i,j;
                gint nAtoms = geometriesMD.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = 0;
  		for(i=0;i<nAtoms;i++) 
		{
			gchar* t = geometriesMD.geometries[0].listOfAtoms[i].pdbType;
			for(j=0;j<n;j++)
			{
				if(!strcmp(t,list[j])) break;
			}
			if(j==n)
				list[n++] = g_strdup(t);
		}
  		for(i=0;i<n;i++) 
			glist = g_list_append(glist,list[i]);
	}

  	gtk_combo_box_entry_set_popdown_strings( comboVal, glist) ;
	if(list && n>0) 
	{
		gint i;
		for(i=0;i<n;i++) g_free(list[i]);
		g_free(list);
	}
  	g_list_free(glist);
}
/**********************************************************************/
static void changedMethod(GtkWidget *entry, gpointer data)
{
	GtkWidget* comboVal = NULL;
	 
	comboVal  = g_object_get_data(G_OBJECT (entry), "ComboVal1");
	setComboVal(comboVal);
	comboVal  = g_object_get_data(G_OBJECT (entry), "ComboVal2");
	setComboVal(comboVal);

}
/***********************************************************************************************/
static GtkWidget *addMethodToTable(GtkWidget *table, gint i)
{
	GtkWidget* entry = NULL;
	GtkWidget* combo = NULL;
	gint nlist = 1;
	gchar* list[] = {"Number"};

	entry = addComboListToATable(table, list, nlist, i, 0, 3);
	combo  = g_object_get_data(G_OBJECT (entry), "Combo");
	gtk_widget_set_sensitive(entry, FALSE);

	return combo;
}
/***********************************************************************************************/
static GtkWidget *addValToTable(GtkWidget *table, gint i)
{
	GtkWidget* entry = NULL;
	GtkWidget* combo = NULL;
	gint nlist = 1;
	gchar* list[] = {"1"};

	entry = addComboListToATable(table, list, nlist, i, 0, 3);
	combo  = g_object_get_data(G_OBJECT (entry), "Combo");
	gtk_widget_set_sensitive(entry, FALSE);

	return combo;
}
/********************************************************************************/
static GtkWidget*   add_inputgr_entrys(GtkWidget *Wins,GtkWidget *vbox)
{
	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(7,4,FALSE);
	GtkWidget* comboMethod = NULL;
	GtkWidget* comboVal1 = NULL;
	GtkWidget* comboVal2 = NULL;
	gint i;

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	i = 0;
	comboMethod = addMethodToTable(table, i);
	i = 1;
	comboVal1 = addValToTable(table, i);
	i = 2;
	comboVal2 = addValToTable(table, i);
	i = 3;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
		(GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions) (GTK_FILL | GTK_EXPAND),
                  2,2);

	if(GTK_IS_COMBO_BOX(comboMethod))
	{
		g_object_set_data(G_OBJECT (GTK_BIN(comboMethod)->child), "ComboVal1", comboVal1);
		g_object_set_data(G_OBJECT (GTK_BIN(comboMethod)->child), "ComboVal2", comboVal2);
		g_object_set_data(G_OBJECT (comboVal1), "ComboMethod", comboMethod);
		g_object_set_data(G_OBJECT (comboVal2), "ComboMethod", comboMethod);
		setComboVal(comboVal1);
		setComboVal(comboVal2);
		g_object_set_data(G_OBJECT (Wins), "EntryMethod",GTK_BIN(comboMethod)->child);
		g_object_set_data(G_OBJECT (Wins), "EntryVal1",GTK_BIN(comboVal1)->child);
		g_object_set_data(G_OBJECT (Wins), "EntryVal2",GTK_BIN(comboVal2)->child);
	}
	setComboMethod(comboMethod);
	g_signal_connect(G_OBJECT(GTK_BIN(comboMethod)->child),"changed", G_CALLBACK(changedMethod),NULL);

	i = 4;
	add_label_table(table,_(" dr "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "Entrydr", entry);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"0.1");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);

	i = 5;
	add_label_table(table,_(" max r "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "Entrymaxr", entry);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"10.0");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);

	gtk_widget_show_all(table);
	entry = GTK_BIN (comboMethod)->child;
	return entry;
}
/*************************************************************************************************************/
static void build_pair_radial_distribution(GtkWidget* Win, gpointer data)
{
	gint g;
	GtkWidget* entrydr = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"Entrydr"));	
	GtkWidget* entrymaxr = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"Entrymaxr"));	
	GtkWidget* entryVal1 = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryVal1"));	
	GtkWidget* entryVal2 = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryVal2"));	
	GtkWidget* entryMethod = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryMethod"));	
	gdouble dr,maxr;
	gint N;
	gint n1,n2;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gint i;
	G_CONST_RETURN gchar* str = NULL;
	GtkWidget* xyplot;
	GtkWidget* window;

	if(geometriesMD.numberOfGeometries<1) return;

	if(entrydr) str = gtk_entry_get_text(GTK_ENTRY(entrydr));
	if(!str) return;
	dr = atof(str);
	if(dr<1e-10) return;
	if(entrymaxr) str = gtk_entry_get_text(GTK_ENTRY(entrymaxr));
	if(!str) return;
	maxr = atof(str);
	N = maxr/dr+1;
	if(N<2) return;
	if(entryMethod) str = gtk_entry_get_text(GTK_ENTRY(entryMethod));
	if(!str) return;
	X = g_malloc(N*sizeof(gdouble));
	Y = g_malloc(N*sizeof(gdouble));
	for(i=0;i<N;i++) X[i] = dr*i;
	for(i=0;i<N;i++) Y[i] = 0;

	/*
	if(strstr(str,"All"))
	{
		for(g = 0;g<geometryConvergence.numberOfGeometries;g++)
		{
			int n=geometryConvergence.geometries[g].numberOfAtoms;
			double fact = 1.0/(4*M_PI*dr)/n/n*(maxr*maxr*maxr)/geometriesMD.numberOfGeometries;
			gint a;
			gint b;
			for(a=0;a<geometryConvergence.geometries[g].numberOfAtoms;a++)
			for(b=0;b<a;b++)
			{
				gdouble d = 0;
				gdouble xx = 0;
				gint j;
				for(j=0;j<3;j++) 
				{
					xx = geometryConvergence.geometries[g].listOfAtoms[a].C[j]-geometryConvergence.geometries[g].listOfAtoms[b].C[j];
					d+= xx*xx;
				}
				d = sqrt(d)*BOHR_TO_ANG;
				if(d>maxr) continue;
				Y[(gint)(d/dr)]+= fact/(d*d);
			}
		}
	}
	else 
	*/
	if(strstr(str,"Number"))
	{
		if(entryVal1) str = gtk_entry_get_text(GTK_ENTRY(entryVal1));
		if(!str) return;
		n1 = atoi(str);
		if(entryVal2) str = gtk_entry_get_text(GTK_ENTRY(entryVal2));
		if(!str) return;
		n2 = atoi(str);

		n1--;
		n2--;
		if(n1>=0 && n2>=0)
		for(g = 0;g<geometriesMD.numberOfGeometries;g++)
		{
			gint a = n1;
			gint b = n2;
			gdouble d = 0;
			gdouble xx = 0;
			gint j;
			for(j=0;j<3;j++) 
			{
				xx = geometriesMD.geometries[g].listOfAtoms[a].C[j]-geometriesMD.geometries[g].listOfAtoms[b].C[j];
				d+= xx*xx;
			}
			d = sqrt(d)*BOHR_TO_ANG;
			if(d>maxr) continue;
			Y[(gint)(d/dr)]++;
		}
	}
	else if(strstr(str,"Symbol"))
	{
		gchar s1[100];
		gchar s2[100];
		if(entryVal1) str = gtk_entry_get_text(GTK_ENTRY(entryVal1));
		if(!str) return;
		sprintf(s1,"%s",str);
		if(entryVal2) str = gtk_entry_get_text(GTK_ENTRY(entryVal2));
		if(!str) return;
		sprintf(s2,"%s",str);

		for(g = 0;g<geometriesMD.numberOfGeometries;g++)
		{
			gint a;
			gint b;
			int na=0;
			int nb=0;
			double fact = 1.0;
			for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
				if(!strcmp(geometriesMD.geometries[g].listOfAtoms[a].symbol,s1)) na++;
			for(b=0;b<geometriesMD.geometries[g].numberOfAtoms;b++)
				if(!strcmp(geometriesMD.geometries[g].listOfAtoms[b].symbol,s2)) nb++;

			if(na>0 && nb>0) fact = 1.0/(2*M_PI*dr)/na/nb*(maxr*maxr*maxr)/geometriesMD.numberOfGeometries;
			if(na>0 && nb>0)
			for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
			{
				for(b=0;b<a;b++)
				{
					if(
					(!strcmp(geometriesMD.geometries[g].listOfAtoms[a].symbol,s1) && !strcmp(geometriesMD.geometries[g].listOfAtoms[b].symbol,s2))
					||
					(!strcmp(geometriesMD.geometries[g].listOfAtoms[a].symbol,s2) && !strcmp(geometriesMD.geometries[g].listOfAtoms[b].symbol,s1))
					)
					{
						gdouble d = 0;
						gdouble xx = 0;
						gint j;
						for(j=0;j<3;j++) 
						{
							xx = geometriesMD.geometries[g].listOfAtoms[a].C[j]-geometriesMD.geometries[g].listOfAtoms[b].C[j];
							d+= xx*xx;
						}
						d = sqrt(d)*BOHR_TO_ANG;
						if(d>maxr) continue;
						//Y[(gint)(d/dr)]++;
						Y[(gint)(d/dr)] += fact/(d*d);
					}

				}
			}
		}
	}
	else if(strstr(str,"MM Type"))
	{
		gchar s1[100];
		gchar s2[100];
		if(entryVal1) str = gtk_entry_get_text(GTK_ENTRY(entryVal1));
		if(!str) return;
		sprintf(s1,"%s",str);
		if(entryVal2) str = gtk_entry_get_text(GTK_ENTRY(entryVal2));
		if(!str) return;
		sprintf(s2,"%s",str);

		for(g = 0;g<geometriesMD.numberOfGeometries;g++)
		{
			gint a;
			gint b;
			for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
			for(b=0;b<a;b++)
			{
				if(
				(!strcmp(geometriesMD.geometries[g].listOfAtoms[a].mmType,s1) && !strcmp(geometriesMD.geometries[g].listOfAtoms[b].mmType,s2))
				||
				(!strcmp(geometriesMD.geometries[g].listOfAtoms[a].mmType,s2) && !strcmp(geometriesMD.geometries[g].listOfAtoms[b].mmType,s1))
				)
				{
					gdouble d = 0;
					gdouble xx = 0;
					gint j;
					for(j=0;j<3;j++) 
					{
						xx = geometriesMD.geometries[g].listOfAtoms[a].C[j]-geometriesMD.geometries[g].listOfAtoms[b].C[j];
						d+= xx*xx;
					}
					d = sqrt(d)*BOHR_TO_ANG;
					if(d>maxr) continue;
					Y[(gint)(d/dr)]++;
				}

			}
		}
	}
	else if(strstr(str,"PDB Type"))
	{
		gchar s1[100];
		gchar s2[100];
		if(entryVal1) str = gtk_entry_get_text(GTK_ENTRY(entryVal1));
		if(!str) return;
		sprintf(s1,"%s",str);
		if(entryVal2) str = gtk_entry_get_text(GTK_ENTRY(entryVal2));
		if(!str) return;
		sprintf(s2,"%s",str);

		for(g = 0;g<geometriesMD.numberOfGeometries;g++)
		{
			gint a;
			gint b;
			for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
			for(b=0;b<a;b++)
			{
				if(
				(!strcmp(geometriesMD.geometries[g].listOfAtoms[a].pdbType,s1) && !strcmp(geometriesMD.geometries[g].listOfAtoms[b].pdbType,s2))
				||
				(!strcmp(geometriesMD.geometries[g].listOfAtoms[a].pdbType,s2) && !strcmp(geometriesMD.geometries[g].listOfAtoms[b].pdbType,s1))
				)
				{
					gdouble d = 0;
					gdouble xx = 0;
					gint j;
					for(j=0;j<3;j++) 
					{
						xx = geometriesMD.geometries[g].listOfAtoms[a].C[j]-geometriesMD.geometries[g].listOfAtoms[b].C[j];
						d+= xx*xx;
					}
					d = sqrt(d)*BOHR_TO_ANG;
					if(d>maxr) continue;
					Y[(gint)(d/dr)]++;
				}

			}
		}
	}

	gtk_widget_destroy(Win);

	window = gabedit_xyplot_new_window(_("Pair radial distribution"),NULL);
	xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
	gabedit_xyplot_add_data_conv(GABEDIT_XYPLOT(xyplot),N, X,  Y, 1.0, GABEDIT_XYPLOT_CONV_NONE,NULL);
	gabedit_xyplot_set_range_xmin (GABEDIT_XYPLOT(xyplot), 0.0);
	gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), "r(Ang)");
	gabedit_xyplot_set_y_label (GABEDIT_XYPLOT(xyplot), "g(r)");
	g_free(X); 
	g_free(Y);
}
/********************************************************************************************************/
static void create_gr_dlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget* vbox;
	GtkWidget* entry;


	/* Principal Window */
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"compute pair radial distribution");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"gr");
	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vboxall = create_vbox(Win);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(vboxall), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
  	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

  	gtk_widget_realize(Win);
	entry =  add_inputgr_entrys(Win,vbox);
	add_cancel_ok_button(Win,vbox,entry,(GCallback)build_pair_radial_distribution);

	/* Show all */
	gtk_widget_show_all (Win);
}
  
/*************************************************************************************************************/
static gint getNumberOfValenceElectrons(gint g)
{
	gint ne = 0;
	gint a;

	for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
	{
  		SAtomsProp Prop = prop_atom_get(geometriesMD.geometries[g].listOfAtoms[a].symbol);
		ne += Prop.atomicNumber;
	}
	return ne;
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
	gint ne = getNumberOfValenceElectrons(0) - totalCharge;

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
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
}
/********************************************************************************/
static void setComboCharge(GtkWidget *comboCharge)
{
	GList *glist = NULL;
	gint i;
	gint nlist;
	gchar** list = NULL;
	gint k;

	nlist = getNumberOfValenceElectrons(0)*2-2+1;

	if(nlist<1) return;
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

  	for(i=0;i<nlist;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboCharge, glist) ;
  	g_list_free(glist);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
}
/**********************************************************************/
static void changedEntrySpinMultiplicity(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	spinMultiplicity=atoi(entryText);
}
/**********************************************************************/
static void changedEntryCharge(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	GtkWidget* comboSpinMultiplicity = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	totalCharge = atoi(entryText);

	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entry), "ComboSpinMultiplicity");
	if(GTK_IS_WIDGET(comboSpinMultiplicity)) setComboSpinMultiplicity(comboSpinMultiplicity);

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
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeightD*0.2),-1);

	return entry;
}
/***********************************************************************************************/
static GtkWidget *addChargeToTable(GtkWidget *table, gint i)
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
static GtkWidget *addSpinToTable(GtkWidget *table, gint i)
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
/*************************************************************************************************************/
static void print_gaussian_geometries_link(GtkWidget* Win, gpointer data)
{
	gint a;
	gchar* fileName = NULL;
	FILE* file;
	gchar p = '%';
	gint g;
	GtkWidget* entry = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryKeywords"));	
	G_CONST_RETURN gchar* supstr = NULL;

	if(geometriesMD.numberOfGeometries<1) return;
	if(entry) supstr = gtk_entry_get_text(GTK_ENTRY(entry));
	if(!supstr) return;
	fileName = g_strdup_printf("%s%sgabmd.com",inputGaussDirectory,G_DIR_SEPARATOR_S);
 	file = fopen(fileName, "w");
	if(!file)
	{
		gchar* t = g_strdup_printf(_("Sorry\n I can not create %s file"),fileName); 
		Message(t,_("Error"),TRUE);
		if(fileName) g_free(fileName);
		if(t)g_free(t);
		return;
	}
	for(g = 0;g<geometriesMD.numberOfGeometries;g++)
	{
		if(g !=0) fprintf(file,"--Link1--\n");
		fprintf(file,"%cChk=gabmd\n",p);
		fprintf(file,"# %s\n",supstr);
		fprintf(file,"# Gfinput IOP(6/7=3) pop=full Test\n");
		fprintf(file,"# Units(Ang,Deg)\n");
		if(g!=0) fprintf(file,"# Guess=read\n");
		fprintf(file,"\n File generated by Gabedit(MD)\n\n");
		fprintf(file,"%d   %d\n",totalCharge,spinMultiplicity);

		for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
		{
			fprintf(file,"%s %lf %lf %lf\n",geometriesMD.geometries[g].listOfAtoms[a].symbol,
			geometriesMD.geometries[g].listOfAtoms[a].C[0]*BOHR_TO_ANG,
			geometriesMD.geometries[g].listOfAtoms[a].C[1]*BOHR_TO_ANG,
			geometriesMD.geometries[g].listOfAtoms[a].C[2]*BOHR_TO_ANG
			);
		}
		fprintf(file,"\n");
	}
	fclose(file);
	gtk_widget_destroy(Win);
	{
		gchar* t = g_strdup_printf(_("The %s file was created"),fileName); 
		Message(t,_("Error"),TRUE);
		if(t)g_free(t);
	}
}
/*************************************************************************************************************/
static void print_gaussian_script_run()
{
	gchar* fileName = NULL;
	FILE* file;
	gchar p = '%';

	if(geometriesMD.numberOfGeometries<1) return;
	fileName = g_strdup_printf("%s%srungabmd.sh",inputGaussDirectory,G_DIR_SEPARATOR_S);
 	file = fopen(fileName, "w");
	if(!file)
	{
		if(fileName) g_free(fileName);
		return;
	}
	fprintf(file,"#!/bin/sh\n");
	fprintf(file,"#############################################\n");
	fprintf(file,"# script for run gaussian with gabmd_ files #\n");
	fprintf(file,"#    this script is created by Gabedit      #\n");
	fprintf(file,"#############################################\n");
	fprintf(file,"\n");
	fprintf(file,"runGauss=g03\n");
	fprintf(file,"echo run gabmd_0.com ....\n");
	fprintf(file,"$runGauss gabmd_0.com\n");
	fprintf(file,"for filecom in gabmd_*.com\n");
	fprintf(file,"do\n");
	fprintf(file,"   filename=${filecom%c.com}\n",p);
	fprintf(file,"   if test \"$filename\" != \"gabmd_0\"\n");
	fprintf(file,"   then\n");
	fprintf(file,"      echo copy gabmd_0.chk in $filename.chk ....\n");
	fprintf(file,"      cp gabmd_0.chk $filename.chk\n");
	fprintf(file,"      echo run $filecom ....\n");
	fprintf(file,"      $runGauss $filecom\n");
	fprintf(file,"   fi\n");
	fprintf(file,"done\n");
	fprintf(file,"\n");
	fclose(file);
}
/*************************************************************************************************************/
static void print_gaussian_one_geometry(gint g, G_CONST_RETURN gchar* supstr)
{
	gint a;
	gchar* fileName = NULL;
	FILE* file;
	gchar p = '%';

	if(g>=geometriesMD.numberOfGeometries || geometriesMD.numberOfGeometries<1) return;
	fileName = g_strdup_printf("%s%sgabmd_%d.com",inputGaussDirectory,G_DIR_SEPARATOR_S,g);
 	file = fopen(fileName, "w");
	if(!file)
	{
		if(fileName) g_free(fileName);
		return;
	}
	if(GTK_IS_WIDGET(buttonChkgauss)&& GTK_TOGGLE_BUTTON (buttonChkgauss)->active)
		fprintf(file,"%cChk=gabmg_%d\n",p,g);
	fprintf(file,"# %s\n",supstr);
	fprintf(file,"# Gfinput IOP(6/7=3) pop=full Test\n");
	fprintf(file,"# Units(Ang,Deg)\n");
	if(g!=0) fprintf(file,"# Guess=read\n");
	fprintf(file,"\n File generated by Gabedit(MD)\n\n");
	fprintf(file,"%d   %d\n",totalCharge,spinMultiplicity);

	for(a=0;a<geometriesMD.geometries[g].numberOfAtoms;a++)
	{
		fprintf(file,"%s %lf %lf %lf\n",geometriesMD.geometries[g].listOfAtoms[a].symbol,
				geometriesMD.geometries[g].listOfAtoms[a].C[0]*BOHR_TO_ANG,
				geometriesMD.geometries[g].listOfAtoms[a].C[1]*BOHR_TO_ANG,
				geometriesMD.geometries[g].listOfAtoms[a].C[2]*BOHR_TO_ANG
				);
	}
	fprintf(file,"\n");
	fclose(file);
}
/*************************************************************************************************************/
static void print_gaussian_geometries(GtkWidget* Win, gpointer data)
{
	gint g;
	GtkWidget* entry = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryKeywords"));	
	G_CONST_RETURN gchar* supstr = NULL;
	if(entry) supstr = gtk_entry_get_text(GTK_ENTRY(entry));
	if(supstr)
	{
		gchar* t = g_strdup_printf(
				_(
				"In %s directory, the gabmd_* and rungabmd.sh files were created.\n"
				"gabmd_* is input files for gaussian. \n"
				"rungabmd.sh is a script for run gaussian with gabmd_* input files"
				)
				, inputGaussDirectory); 
		for(g = 0;g<geometriesMD.numberOfGeometries;g++)
			print_gaussian_one_geometry(g,supstr);
		print_gaussian_script_run();
		Message(t,_("Error"),TRUE);
		if(t)g_free(t);
	}
	gtk_widget_destroy(Win);
}
/*************************************************************************************************************/
static void print_gaussian_selected_geometry(GtkWidget* Win, gpointer data)
{
	gint j;

	gint g;
	GtkWidget* entry = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryKeywords"));	
	G_CONST_RETURN gchar* supstr = NULL;
	gint k = rowSelected;
	if(k<0 || k >= geometriesMD.numberOfGeometries) return;
	if(entry) supstr = gtk_entry_get_text(GTK_ENTRY(entry));
	if(supstr)
	{
		gchar* t = g_strdup_printf(
				_(
				"In %s directory, the gabmd_%d.com was created.\n"
				)
				, inputGaussDirectory,k); 
		print_gaussian_one_geometry(k,supstr);
		Message(t,_("Info"),TRUE);
		if(t)g_free(t);
	}
	gtk_widget_destroy(Win);
}
/*************************************************************************************************************/
static gdouble get_velocity_dot_velocity(gint g1, gint g2, gint a)
{
	gdouble p = 0;
	gint i;

	if(g1>=geometriesMD.numberOfGeometries) return 0;
	if(g2>=geometriesMD.numberOfGeometries) return 0;
	if(a>=geometriesMD.geometries[g1].numberOfAtoms) return 0;
	if(a>=geometriesMD.geometries[g2].numberOfAtoms) return 0;
	for(i=0;i<3;i++)
		p += geometriesMD.geometries[g1].listOfAtoms[a].V[i]*geometriesMD.geometries[g2].listOfAtoms[a].V[i];
	return p;
}
/*************************************************************************************************************/
static gdouble get_averaged_velocity_dot_velocity(gint g1, gint g2)
{
	gdouble p = 0;
	gint a;
	if(g1>=geometriesMD.numberOfGeometries) return 0;
	if(g2>=geometriesMD.numberOfGeometries) return 0;
	if(geometriesMD.geometries[g1].numberOfAtoms!=geometriesMD.geometries[g2].numberOfAtoms)return 0;
	for(a=0;a<geometriesMD.geometries[g1].numberOfAtoms;a++)
	{
		/*
		 * p += get_velocity_dot_velocity(g1, g2, a)/sqrt(get_velocity_dot_velocity(g1, g1, a)*get_velocity_dot_velocity(g2, g2, a));
		 * */
		p += get_velocity_dot_velocity(g1, g2, a);
	}
	if(geometriesMD.geometries[g1].numberOfAtoms>0) p/=geometriesMD.geometries[g1].numberOfAtoms;
	return p;
}
/*************************************************************************************************************/
static gdouble* get_velocity_velocity_correlation_function(gint* N, gdouble*time)
{
	gint g;
	gint gp;
	gdouble n = 0.0;
	gdouble* Cvv = NULL;
	gint nTau = 3*geometriesMD.numberOfGeometries/4;
	gint ntmax = geometriesMD.numberOfGeometries-nTau;

	*N = nTau;
	if(geometriesMD.numberOfGeometries<2) return NULL;
	*time = geometriesMD.geometries[1].time;
	Cvv = g_malloc(*N*sizeof(gdouble));

	n = 0;
	for(g=0;g<ntmax;g++)
	{
		n += get_averaged_velocity_dot_velocity(g,g);
	}
	n/=ntmax;
	Cvv[0] = 1;
	for(g=1;g<nTau;g++)
	{
		gdouble c = 0;
		
		for(gp=0;gp<ntmax;gp++)
			c += get_averaged_velocity_dot_velocity(gp,gp+g);
		c /= ntmax;

		Cvv[g] =c/n;
	}
	/*
	n = get_averaged_velocity_dot_velocity(0,0);
	for(g=0;g<geometriesMD.numberOfGeometries;g++)
	{
		Cvv[g] = get_averaged_velocity_dot_velocity(0,g)/n;
	}
	*/
	return Cvv;
}
/*************************************************************************************************************/
/*
static void compute_infrared_spectra(gdouble fmin, gdouble fmax, gdouble df)
{
	gint n = 0;
	gdouble dt = 0;
	gdouble* Cvv = get_velocity_velocity_correlation_function(&n, &dt);
	gchar* fileName = "InfraredSpectra.txt";
	FILE* file;
	gdouble f;
	gint i;
	if(!Cvv) return;

 	file = fopen(fileName, "w");
	for(f=fmin;f<=fmax;f+=df)
	{
		gdouble I = 0;
		for(i=0;i<n;i++)
		{
			I += Cvv[i]*cos(2*PI*f*2.99792458e10*i*dt*1e-15)*dt;
		}
		printf("%lf %lf\n",f,I);
		fprintf(file,"%lf %lf\n",f,I);
	}
	fclose(file);
}
*/
/*************************************************************************************************************/
static void print_velocity_velocity_correlation_function(gchar* fileName)
{
	gint n = 0;
	gdouble dt = 0;
	gdouble* Cvv = get_velocity_velocity_correlation_function(&n, &dt);
	FILE* file;
	gint g;
	if(!Cvv) return;
 	file = fopen(fileName, "w");
	for(g=0;g<n;g++)
	{
		fprintf(file,"%lf %lf\n",geometriesMD.geometries[g].time,Cvv[g]);
	}
	fclose(file);
}
/*************************************************************************************************************/
static void display_velocity_velocity_correlation_function()
{
	GtkWidget* xyplot;
	GtkWidget* window;
	gint n = 0;
	gdouble dt = 0;
	gdouble* X = NULL;
	gdouble* Y = get_velocity_velocity_correlation_function(&n, &dt);
	gint i;
	if(!Y) return;
	X = g_malloc(n*sizeof(gdouble));
	for(i=0;i<n;i++) X[i] = geometriesMD.geometries[i].time;
	

	window = gabedit_xyplot_new_window(_("Velocity-Velocity autocorrelation"),NULL);
	xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
	gabedit_xyplot_add_data_conv(GABEDIT_XYPLOT(xyplot),n, X,  Y, 1.0, GABEDIT_XYPLOT_CONV_NONE,NULL);
	gabedit_xyplot_set_range_xmin (GABEDIT_XYPLOT(xyplot), 0.0);
	gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), "cm<sup>-1</sup>");
	g_free(X); 
	g_free(Y);
}
/********************************************************************************/
static void reset_last_directory(GtkWidget *dirSelector, gpointer data)
{
	gchar* dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));
	gchar* filename = NULL;

	if(dirname && strlen(dirname)>0)
	{
		if(dirname[strlen(dirname)-1] != G_DIR_SEPARATOR)
			filename = g_strdup_printf("%s%sdump.txt",dirname,G_DIR_SEPARATOR_S);
		else
			filename = g_strdup_printf("%sdump.txt",dirname);
	}
	else
	{
		dirname = g_strdup(g_get_home_dir());
		filename = g_strdup_printf("%s%sdump.txt",dirname,G_DIR_SEPARATOR_S);
	}
	if(dirname) g_free(dirname);
	if(filename)
	{
		set_last_directory(filename);
		g_free(filename);
	}
}
/********************************************************************************/
static void set_directory(GtkWidget *win, gpointer data)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(reset_last_directory, "Set folder", GABEDIT_TYPEWIN_ORB);
	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(dirSelector),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_transient_for(GTK_WINDOW(dirSelector),GTK_WINDOW(WinDlg));
}
/********************************************************************************/
void initGeometryMD()
{
	geometriesMD.fileName = NULL;
	geometriesMD.typeOfFile = GABEDIT_TYPEFILE_UNKNOWN;
	geometriesMD.numberOfGeometries = 0;
	geometriesMD.geometries = NULL;
	geometriesMD.velocity = 0.1;
	rowSelected = -1;
}
/********************************************************************************/
void freeGeometryMD()
{
	static gboolean begin = TRUE;

	if(begin)
	{
		begin  = FALSE;
		initGeometryMD();
		return;
	}
	if(geometriesMD.fileName) g_free(geometriesMD.fileName);
	if(geometriesMD.geometries)
	{
		gint i;
		GeometryMD* geometries = geometriesMD.geometries;
		for(i=0;i<geometriesMD.numberOfGeometries;i++)
		{
			if(geometries[i].listOfAtoms) g_free(geometries[i].listOfAtoms);
			if(geometries[i].comments) g_free(geometries[i].comments);
		}
		g_free(geometries);
	}

	initGeometryMD();
}
/********************************************************************************/
static void delete_one_geometry()
{

	gint k = rowSelected;
	gint j;

	if(k<0 || k >= geometriesMD.numberOfGeometries) return;
	if(!geometriesMD.geometries) return;
	if(geometriesMD.numberOfGeometries==1)
	{
		freeGeometryMD();
		rafreshList();
		return;
	}

	if(geometriesMD.geometries)
	{
			if(geometriesMD.geometries[k].listOfAtoms) g_free(geometriesMD.geometries[k].listOfAtoms);
			if(geometriesMD.geometries[k].comments) g_free(geometriesMD.geometries[k].comments);
	}
	for(j=k;j<geometriesMD.numberOfGeometries-1;j++)
	{
		geometriesMD.geometries[j].energy = geometriesMD.geometries[j+1].energy;
		geometriesMD.geometries[j].time = geometriesMD.geometries[j+1].time;
		geometriesMD.geometries[j].comments = geometriesMD.geometries[j+1].comments;
		geometriesMD.geometries[j].listOfAtoms = geometriesMD.geometries[j+1].listOfAtoms;
	}
	geometriesMD.numberOfGeometries--;
	geometriesMD.geometries = g_realloc(geometriesMD.geometries,geometriesMD.numberOfGeometries*sizeof(GeometryMD));
	rafreshList();

	return;
}
/********************************************************************************/
static void delete_geometries_between(gint begin, gint end)
{

	gint k;
	gint j;

	if(!geometriesMD.geometries) return;
	if(begin<0) begin = 0;
	if(end>=geometriesMD.numberOfGeometries) end = geometriesMD.numberOfGeometries-1;
	if(end<0) end = geometriesMD.numberOfGeometries-1;
	if(geometriesMD.numberOfGeometries<1) return;
	if(end<begin)
	{
		k = begin;
		begin = end;
		end = k;
	}

	for(k=begin;k<=end;k++)
	{
		if(geometriesMD.geometries[k].listOfAtoms) g_free(geometriesMD.geometries[k].listOfAtoms);
		if(geometriesMD.geometries[k].comments) g_free(geometriesMD.geometries[k].comments);

	}
	for(k=begin;k<geometriesMD.numberOfGeometries-1;k++)
	{
		j = k+end-begin+1;
		if(j>=geometriesMD.numberOfGeometries) break;
		geometriesMD.geometries[k].energy = geometriesMD.geometries[j].energy;
		geometriesMD.geometries[k].time = geometriesMD.geometries[j].time;
		geometriesMD.geometries[k].comments = geometriesMD.geometries[j].comments;
		geometriesMD.geometries[k].listOfAtoms = geometriesMD.geometries[j].listOfAtoms;
	}
	geometriesMD.numberOfGeometries -= end-begin+1;
	if(geometriesMD.numberOfGeometries>0)
	geometriesMD.geometries = g_realloc(geometriesMD.geometries,geometriesMD.numberOfGeometries*sizeof(GeometryMD));
	else
	{
		if(geometriesMD.geometries ) g_free(geometriesMD.geometries );
		geometriesMD.numberOfGeometries  = 0;
	}
	rafreshList();

	return;
}
/********************************************************************************/
static void delete_beforee_selected_geometry()
{

	gint k = rowSelected;

	if(k<=0) return;
	if(!geometriesMD.geometries) return;
	if(geometriesMD.numberOfGeometries==1)
	{
		freeGeometryMD();
		rafreshList();
		return;
	}
	delete_geometries_between(0, k-1);
	return;
}
/********************************************************************************/
static void delete_after_selected_geometry()
{

	gint k = rowSelected;

	if(k<0) return;
	if(k>=geometriesMD.numberOfGeometries-1) return;
	if(!geometriesMD.geometries) return;
	if(geometriesMD.numberOfGeometries==1)
	{
		freeGeometryMD();
		rafreshList();
		return;
	}
	delete_geometries_between(k+1, geometriesMD.numberOfGeometries-1);
	return;
}
/********************************************************************************/
static void delete_half_geometries()
{

	gint k;
	gint j;

	if(!geometriesMD.geometries) return;
	if(geometriesMD.numberOfGeometries<2) return;

	for(k=1;k<geometriesMD.numberOfGeometries;k+=2)
	{
		if(geometriesMD.geometries[k].listOfAtoms) g_free(geometriesMD.geometries[k].listOfAtoms);
		if(geometriesMD.geometries[k].comments) g_free(geometriesMD.geometries[k].comments);

	}
	j = 0;
	for(k=1;k<geometriesMD.numberOfGeometries-1;k+=2)
	{
		j++;
		geometriesMD.geometries[j].energy = geometriesMD.geometries[k+1].energy;
		geometriesMD.geometries[j].time = geometriesMD.geometries[k+1].time;
		geometriesMD.geometries[j].comments = geometriesMD.geometries[k+1].comments;
		geometriesMD.geometries[j].listOfAtoms = geometriesMD.geometries[k+1].listOfAtoms;
	}
	geometriesMD.numberOfGeometries = j+1;
	if(geometriesMD.numberOfGeometries>0)
	geometriesMD.geometries = g_realloc(geometriesMD.geometries,geometriesMD.numberOfGeometries*sizeof(GeometryMD));
	else
	{
		if(geometriesMD.geometries ) g_free(geometriesMD.geometries );
		geometriesMD.numberOfGeometries  = 0;
	}
	rafreshList();

	return;
}
/*************************************************************************************************************/
static gboolean read_gaussian_file_geomi_str(gchar *FileName, gint num, gchar* str, GeometryMD* geometry)
{
 	gchar *t;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *file;
 	guint idummy;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
 	gchar *pdest;
 	gint result;
 	guint itype=0;
	AtomMD* listOfAtoms = NULL;

  
 	file = FOpen(FileName, "rb");

	t=g_malloc(BSIZE);
 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(BSIZE*sizeof(char));

	numgeom =1;
 	do 
 	{
 		OK=FALSE;
 		while(!feof(file))
		{
    			{ char* e = fgets(t,BSIZE,file);}
	 		if (strstr( t,str) )
	  		{
    				{ char* e = fgets(t,BSIZE,file);}
    				{ char* e = fgets(t,BSIZE,file);}
    				{ char* e = fgets(t,BSIZE,file);}
				if(strstr( t, "Type" )) itype=1;
				else itype=0;
    				{ char* e = fgets(t,BSIZE,file);}
                		numgeom++;
				OK = TRUE;
				break;
	  		}
		}
 		if(!OK && (numgeom == 1) )
		{
 			fclose(file);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			return FALSE;
		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(file) )
  		{
    			{ char* e = fgets(t,BSIZE,file);}
    			if (strstr( t, "----------------------------------" ) )
    			{
      				break;
    			}
    			j++;
    			if(listOfAtoms == NULL) listOfAtoms = g_malloc(sizeof(AtomMD));
    			else listOfAtoms = g_realloc(listOfAtoms, (j+1)*sizeof(AtomMD));

    			if(itype==0) sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
    			else sscanf(t,"%d %s %d %s %s %s",&idummy,AtomCoord[0],&idummy,AtomCoord[1],AtomCoord[2],AtomCoord[3]);

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			sprintf(listOfAtoms[j].symbol,"%s",symb_atom_get((guint)atoi(AtomCoord[0])));
    			for(i=0;i<3;i++) listOfAtoms[j].C[i]=atof(ang_to_bohr(AtomCoord[i+1]));
			listOfAtoms[j].partialCharge = 0.0;
			listOfAtoms[j].variable = TRUE;
			listOfAtoms[j].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[j].symbol);
			sprintf(listOfAtoms[j].mmType,"%s",listOfAtoms[j].symbol);
			sprintf(listOfAtoms[j].pdbType,"%s",listOfAtoms[j].symbol);
			sprintf(listOfAtoms[j].resName,"%s",listOfAtoms[j].symbol);
			listOfAtoms[j].resNumber = j;
		}
		if(num >0 && (gint)numgeom-1 == num) break;
 	}while(!feof(file));

 	fclose(file);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);

 	if(j+1 == 0 && listOfAtoms )
	{
		g_free(listOfAtoms);
		listOfAtoms = NULL;
	}
 	else
	{
		geometry->numberOfAtoms = j+1;
		geometry->listOfAtoms = listOfAtoms;
	}
	return TRUE;
}
/********************************************************************************/
static gboolean read_gaussian_file_geomi(gchar *FileName, gint num, GeometryMD* geometry)
{
	FILE* file;
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return FALSE;
 	}
 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
 	}
	fclose(file);


	if(read_gaussian_file_geomi_str(FileName,num,"Input orientation:", geometry)) return TRUE;
	if(read_gaussian_file_geomi_str(FileName,num,"Standard orientation:", geometry)) return TRUE;
	/* for calculation with nosym option */
	if(!read_gaussian_file_geomi_str(FileName,num,"Z-Matrix orientation:", geometry))
	{
  		Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
		return FALSE;
	}
	return TRUE;

}
#define MAXNAME 6
#define MAXATOMTYPE 4
#define MAXRESIDUENAME 4
#define MAXSYMBOL 2
/******************************************************************************/
static void save_atom_pdb_file(FILE* file, 
		gchar*name, gint atomNumber,
		gchar* atomType, gchar* residueName,
		gint residueNumber,
		gdouble x, gdouble y, gdouble z,
		gdouble occupancy,
		gdouble temperature,
		gchar* symbol,
		gdouble charge
		)
{
	gchar localName[MAXNAME+1];
	gchar localAtomType[MAXATOMTYPE+1];
	gchar localResidueName[MAXRESIDUENAME+1];
	gchar localSymbol[MAXSYMBOL+1];

	localName[MAXNAME] = '\0';
	localAtomType[MAXATOMTYPE] = '\0';
	localResidueName[MAXRESIDUENAME] = '\0';
	localSymbol[MAXSYMBOL] = '\0';

	if(strlen(name)>MAXNAME)
		strncpy(localName, name, MAXNAME);
	else
		strcpy(localName, name);
	uppercase(localName);

	if(strlen(atomType)>MAXATOMTYPE)
	{
		strncpy(localAtomType, atomType, MAXATOMTYPE);
	}
	else
	{
		if(atomType && isdigit(atomType[0])) strcpy(localAtomType,atomType);
		else 
		{
			sprintf(localAtomType," %s",atomType);
			if(strlen(localAtomType)>=MAXATOMTYPE)
			{
				if(isdigit(localAtomType[MAXATOMTYPE]))
					localAtomType[0] = localAtomType[MAXATOMTYPE];
				localAtomType[MAXATOMTYPE] = '\0';
			}
		}
	}

	if(strlen(residueName)>=MAXRESIDUENAME)
		strncpy(localResidueName, residueName, MAXRESIDUENAME);
	else
	{
		sprintf(localResidueName," %s",residueName);
		if(strlen(localResidueName)>=MAXRESIDUENAME)
		{
			if(isdigit(localResidueName[MAXRESIDUENAME]))
				localResidueName[0] = localResidueName[MAXRESIDUENAME];
			localResidueName[MAXRESIDUENAME] = '\0';
		}
	}
	uppercase(localResidueName);

	if(strlen(symbol)>MAXSYMBOL)
		strncpy(localSymbol, symbol, MAXSYMBOL);
	else
		strcpy(localSymbol,symbol);
	localSymbol[0] = toupper(localSymbol[0]);
	if(strlen(localSymbol)>1)
		localSymbol[1] = tolower(localSymbol[1]);

	if(atomNumber>99999)
		atomNumber = 99999;
	if(residueNumber>9999)
		residueNumber = 9999;


        fprintf(file,"%-6s",localName); /* Atom or HETATM */
        fprintf(file,"%-6d",atomNumber); 
        fprintf(file,"%-4s",localAtomType); 
        fprintf(file,"%-4s",localResidueName); 
        fprintf(file,"  "); 
        fprintf(file,"%-4d",residueNumber); 
        fprintf(file,"    "); 
        fprintf(file,"%-8.3f",x); 
        fprintf(file,"%-8.3f",y); 
        fprintf(file,"%-8.3f",z); 
        fprintf(file,"%-6.2f",occupancy); 
        fprintf(file,"%-6.2f",temperature); 
        fprintf(file,"      "); 
        fprintf(file,"    "); 
        fprintf(file,"%-2s",localSymbol); 
        fprintf(file,"%-9.4f\n",charge); 
}
#undef MAXNAME
#undef MAXATOMTYPE
#undef MAXRESIDUENAME
#undef MAXSYMBOL
/********************************************************************************/
static gboolean save_geometry_MD_pdb_format(gchar *FileName)
{
 	FILE *file;
	gint j;
	gboolean OK = TRUE;
	gchar* temp = NULL;

	if(geometriesMD.numberOfGeometries<1) return FALSE;
 	file = FOpen(FileName, "w");
	if(file == NULL)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I  can not create '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return FALSE;
	}
	fprintf(file,"HEADER    PROTEIN\n");
	fprintf(file,"COMPND    UNNAMED\n");
	temp = get_time_str();
	if(temp) fprintf(file,"AUTHOR    GENERATED BY GABEDIT %d.%d.%d at %s",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION,temp);
	else fprintf(file,"AUTHOR    GENERATED BY GABEDIT %d.%d.%d\n",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
	for(j=0;j<geometriesMD.numberOfGeometries;j++)
	{
		gint i;
		gint nAtoms = geometriesMD.geometries[j].numberOfAtoms;
		AtomMD* listOfAtoms = geometriesMD.geometries[j].listOfAtoms;
		if(nAtoms<1 || !listOfAtoms) { OK = FALSE; break;}
		fprintf(file,"TITLE nAtoms=%d Energy=%lf\n", nAtoms, geometriesMD.geometries[j].energy);
		fprintf(file,"REMARK Times=%lf\n", geometriesMD.geometries[j].time);
		fprintf(file,"REMARK Check the CRYSTAL PARAMETERS\n");
		fprintf(file,"CRYST1   20.325   20.325   23.634  90.00  90.00  90.00 P 1           1\n");
		fprintf(file,"MODEL %d\n", j);

		for(i=0;i<nAtoms;i++)
		{
			gdouble X = listOfAtoms[i].C[0]*BOHR_TO_ANG;
			gdouble Y = listOfAtoms[i].C[1]*BOHR_TO_ANG;
			gdouble Z = listOfAtoms[i].C[2]*BOHR_TO_ANG;
			save_atom_pdb_file(file,"ATOM",i+1,listOfAtoms[i].pdbType,listOfAtoms[i].resName,
			listOfAtoms[i].resNumber+1, X,Y,Z,
			1.0, 300.0, listOfAtoms[i].symbol, listOfAtoms[i].partialCharge);
		}
		fprintf(file,"TER\n");
		fprintf(file,"ENDMDL\n");
	}
	fclose(file);
	return OK;
}
/********************************************************************************/
static gboolean save_geometry_MD_gabedit_format(gchar *FileName)
{
 	FILE *file;
	gint j;
	gboolean OK = TRUE;

	if(geometriesMD.numberOfGeometries<1) return FALSE;
 	file = FOpen(FileName, "w");
	if(file == NULL)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I  can not create '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return FALSE;
	}
	fprintf(file,"[Gabedit Format]\n");
	fprintf(file,"\n");
	fprintf(file,"[MD]\n");
	fprintf(file," %d\n",geometriesMD.numberOfGeometries);
	for(j=0;j<geometriesMD.numberOfGeometries;j++)
	{
		gint i;
		gint nAtoms = geometriesMD.geometries[j].numberOfAtoms;
		AtomMD* listOfAtoms = geometriesMD.geometries[j].listOfAtoms;
		if(nAtoms<1 || !listOfAtoms) { OK = FALSE; break;}
		fprintf(file," %d %lf %lf\n", nAtoms,geometriesMD.geometries[j].time, geometriesMD.geometries[j].energy);
		fprintf(file," %s\n", geometriesMD.geometries[j].comments);
		for(i=0;i<nAtoms;i++)
		{
			fprintf(file," %s %lf %lf %lf %lf %lf %lf %lf %s %s %s %d %d\n", 
				listOfAtoms[i].symbol,
				listOfAtoms[i].C[0]*BOHR_TO_ANG,
				listOfAtoms[i].C[1]*BOHR_TO_ANG,
				listOfAtoms[i].C[2]*BOHR_TO_ANG,
				listOfAtoms[i].V[0],
				listOfAtoms[i].V[1],
				listOfAtoms[i].V[2],
				listOfAtoms[i].partialCharge,
				listOfAtoms[i].mmType,
				listOfAtoms[i].pdbType,
				listOfAtoms[i].resName,
				listOfAtoms[i].resNumber,
				listOfAtoms[i].variable
				);
		}
	}
	fclose(file);
	return OK;
}
/********************************************************************************/
static void reset_parameters(GtkWidget *win, gpointer data)
{
	gdouble velo = atof(gtk_entry_get_text(GTK_ENTRY(EntryVelocity)));
	gchar* t;

	if(velo<0)
	{

		velo = -velo;
		t = g_strdup_printf("%lf",velo);
		gtk_entry_set_text(GTK_ENTRY(EntryVelocity),t);
		g_free(t);
	}

	geometriesMD.velocity = velo;
	if(!play) stopAnimation(NULL,NULL);

}
/*************************************************************************************************************/
static gint get_number_of_geomtries_in_gaussian(gchar *fileName)
{
 	gchar *t;
 	FILE *file;
	gint nG = 0;
  
 	file = FOpen(fileName, "rb");
	if(!file) return 0;
	t = g_malloc(BSIZE*sizeof(gchar));
 	while(!feof(file))
	{
  		if(!fgets(t,BSIZE,file))break;
		if(strstr( t,"Summary information for step")) nG++;
	}
	g_free(t);
	fclose(file);
	return nG;
}
/*************************************************************************************************************/
static void scan_geomtries_position_in_gaussian(gchar *fileName)
{
 	gchar *t;
 	FILE *file;
	gint j;
	long int pos = -1;

	for(j=0;j<geometriesMD.numberOfGeometries;j++)
		geometriesMD.geometries[j].filePos = -1;
  
 	file = FOpen(fileName, "rb");
	if(!file) return ;
	t = g_malloc(BSIZE*sizeof(gchar));
	j = 0;
 	while(!feof(file))
	{
		/* pos = ftell(file);*/
  		if(!fgets(t,BSIZE,file))break;
		if(strstr( t,"Summary information for step"))
		{ 
			pos = ftell(file);
			geometriesMD.geometries[j].filePos = pos; 
			j++;
		}
	}
	g_free(t);
	fclose(file);
}
/*************************************************************************************************************/
static gboolean read_MD_gaussian_file_step(gchar* fileName, gint step)
{
 	gchar *t;
 	gchar *pos;
	gint k = 0;
 	FILE *file;
	gint j = step;
	gint i = 0;
	gint c = 0;
	AtomMD* listOfAtoms = NULL;

	geometriesMD.geometries[j].time = 0.0;
	geometriesMD.geometries[j].energy = 0.0;
	geometriesMD.geometries[j].comments = g_strdup_printf("Step n %d",step);
	geometriesMD.geometries[j].numberOfAtoms = geometriesMD.geometries[0].numberOfAtoms;
	if(j!=0) geometriesMD.geometries[j].listOfAtoms = g_malloc(geometriesMD.geometries[j].numberOfAtoms*sizeof(AtomMD));
	listOfAtoms = geometriesMD.geometries[j].listOfAtoms;
	for(i=0;i<geometriesMD.geometries[j].numberOfAtoms;i++)
	{
		if(j!=0) sprintf(listOfAtoms[i].symbol,"%s",geometriesMD.geometries[0].listOfAtoms[i].symbol);
		listOfAtoms[i].C[0] = 0;
		listOfAtoms[i].C[1] = 0;
		listOfAtoms[i].C[2] = 0;
		listOfAtoms[i].V[0] = 0;
		listOfAtoms[i].V[1] = 0;
		listOfAtoms[i].V[2] = 0;
		listOfAtoms[i].partialCharge = 0.0;
		listOfAtoms[i].variable = TRUE;
		listOfAtoms[i].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[i].symbol);
		sprintf(listOfAtoms[i].mmType,"%s",listOfAtoms[i].symbol);
		sprintf(listOfAtoms[i].pdbType,"%s",listOfAtoms[i].symbol);
		sprintf(listOfAtoms[i].resName,"%s",listOfAtoms[i].symbol);
		listOfAtoms[i].resNumber = i;
	}
	
	if(geometriesMD.geometries[j].filePos<0) return FALSE;
  
 	file = FOpen(fileName, "rb");
	if(!file) return FALSE;
	t = g_malloc(BSIZE*sizeof(gchar));
	fseek(file, geometriesMD.geometries[j].filePos, SEEK_SET);
	k = 0;
 	while(!feof(file))
	{
  		if(!fgets(t,BSIZE,file))break;
		if(strstr( t,"Predicted information ")) break;
		if(strstr( t,"TRJ-TRJ-TRJ-TRJ")) break;
		if(strstr( t,"Time (fs)"))
		{
			pos = strstr( t,")")+1;
			for(i=0;i<strlen(t);i++) if(t[i]=='D' || t[i]=='d') t[i] = 'e';
			geometriesMD.geometries[j].time = atof(pos);
			k++;
		}
		if(strstr( t,"Total energy"))
		{
			pos = strstr( t,"gy")+2;
			for(i=0;i<strlen(t);i++) if(t[i]=='D' || t[i]=='d') t[i] = 'e';
			geometriesMD.geometries[j].energy = atof(pos);
			k++;
		}
		if(strstr( t,"Cartesian coordinates:"))
		{
			for(i=0;i<geometriesMD.geometries[j].numberOfAtoms;i++)
			{
  				if(!fgets(t,BSIZE,file))break;
				for(c=0;c<strlen(t);c++) if(t[c]=='D' || t[c]=='d') t[c] = 'e';
				pos = strstr( t,"X=");
				if(pos) listOfAtoms[i].C[0] = atof(pos+2);
				else break;
				pos = strstr( t,"Y=");
				if(pos) listOfAtoms[i].C[1] = atof(pos+2);
				else break;
				pos = strstr( t,"Z=");
				if(pos) listOfAtoms[i].C[2] = atof(pos+2);
				else break;
			}
			if(i!=geometriesMD.geometries[j].numberOfAtoms) break;
			k++;
		}
		if(strstr( t,"artesian velocity:"))
		{
			for(i=0;i<geometriesMD.geometries[j].numberOfAtoms;i++)
			{
  				if(!fgets(t,BSIZE,file))break;
				for(c=0;c<strlen(t);c++) if(t[c]=='D' || t[c]=='d') t[c] = 'e';
				pos = strstr( t,"X=");
				if(pos) listOfAtoms[i].V[0] = atof(pos+2);
				else break;
				pos = strstr( t,"Y=");
				if(pos) listOfAtoms[i].V[1] = atof(pos+2);
				else break;
				pos = strstr( t,"Z=");
				if(pos) listOfAtoms[i].V[2] = atof(pos+2);
				else break;
			}
			if(i!=geometriesMD.geometries[j].numberOfAtoms) break;
			k++;
		}
		if(k==4) break;

	}

	g_free(t);
	fclose(file);
	if(k==4) return TRUE;
	return FALSE;
}
/********************************************************************************/
static gboolean read_gaussian_output(gchar* fileName)
{
	gint  j=0;
	gchar *t = NULL;
	gint nG = 0;
        
	t = get_name_file(fileName);
	set_status_label_info(_("File name"),t);
	g_free(t);
	t = NULL;
	set_status_label_info(_("File type"),"Gaussian output");

	nG=get_number_of_geomtries_in_gaussian(fileName);
	if(nG<1) 
	{
		t = g_strdup_printf(_(" Error : I can not read %s file \n"),fileName);
		Message(t,_(" Error "),TRUE);
		g_free(t);
		return FALSE;
	}
	geometriesMD.numberOfGeometries=nG;
	geometriesMD.geometries = g_malloc(geometriesMD.numberOfGeometries*sizeof(GeometryMD));
	geometriesMD.fileName = g_strdup(fileName);
	geometriesMD.typeOfFile = GABEDIT_TYPEFILE_GAUSSIAN;

	for(j=0;j<geometriesMD.numberOfGeometries;j++)
	{
		geometriesMD.geometries[j].numberOfAtoms = 0;
		geometriesMD.geometries[j].time = 0;
		geometriesMD.geometries[j].energy = 0;
		geometriesMD.geometries[j].listOfAtoms = NULL;
	}

	if(!read_gaussian_file_geomi(fileName, 1, &geometriesMD.geometries[0])) 
	{
		freeGeometryMD();
		t = g_strdup_printf(_(" Error : I can not read the first geometry from %s file\n"),fileName);
		Message(t,_("Error"),TRUE);
  		rafreshList();
		return FALSE;
	}
	scan_geomtries_position_in_gaussian(fileName);
	for(j=0;j<geometriesMD.numberOfGeometries;j++)
		if(!read_MD_gaussian_file_step(fileName, j)) 
		{
			break;
		}
	if(j!=geometriesMD.numberOfGeometries) 
	{
		printf("j=%d\n",j);
		if(j>1)
		{
			if(geometriesMD.geometries[j].listOfAtoms) g_free(geometriesMD.geometries[j].listOfAtoms);
			if(geometriesMD.geometries[j].comments) g_free(geometriesMD.geometries[j].comments);
			geometriesMD.numberOfGeometries--;
			geometriesMD.geometries = g_realloc(geometriesMD.geometries,geometriesMD.numberOfGeometries*sizeof(GeometryMD));
		}
		else
		{
			freeGeometryMD();
			t = g_strdup_printf(_(" Error : I can not read step number %d from %s file\n"),j,fileName);
			Message(t,_("Error"),TRUE);
			g_free(t);
  			rafreshList();
			return FALSE;
		}
	}
  	rafreshList();
	return TRUE;
}
/*************************************************************************************************************/
static gboolean read_gamess_trj_first_geometry(gchar *FileName, GeometryMD* geometry)
{
 	gchar *t;
 	gchar *pos;
 	FILE *file;
 	gint i;
	gint nAtoms = 0;
	AtomMD* listOfAtoms = NULL;

  
 	file = FOpen(FileName, "rb");
	t=g_malloc(BSIZE*sizeof(gchar));

	if(!t) return FALSE;
 	while(!feof(file))
	{
  		if(!fgets(t,BSIZE,file))break;
		if(strstr( t,"NAT="))
		{
			pos = strstr( t,"AT=")+3;
			nAtoms = atoi(pos);
			break;
		}
	}
	if(nAtoms<1) 
	{
 		fclose(file);
		g_free(t);
		return FALSE;
	}
    	listOfAtoms = g_malloc(nAtoms*sizeof(AtomMD));
 	while(!feof(file))
	{
  		if(!fgets(t,BSIZE,file))break;
		if(strstr( t,"QM PARTICLE COORDINATES"))
		{
			for(i=0;i<nAtoms;i++)
			{
  				if(!fgets(t,BSIZE,file))break;
				if(5!=sscanf(t,"%s %lf %lf %lf %lf",
				listOfAtoms[i].symbol,
				&listOfAtoms[i].nuclearCharge,
				&listOfAtoms[i].C[0],
				&listOfAtoms[i].C[1],
				&listOfAtoms[i].C[2]))break;
				listOfAtoms[i].partialCharge = 0.0;
				listOfAtoms[i].variable = TRUE;
				listOfAtoms[i].C[0] *= ANG_TO_BOHR;
				listOfAtoms[i].C[1] *= ANG_TO_BOHR;
				listOfAtoms[i].C[2] *= ANG_TO_BOHR;
				sprintf(listOfAtoms[i].mmType,"%s",listOfAtoms[i].symbol);
				sprintf(listOfAtoms[i].pdbType,"%s",listOfAtoms[i].symbol);
				sprintf(listOfAtoms[i].resName,"%s",listOfAtoms[i].symbol);
				listOfAtoms[i].resNumber = i;
			}
			if(i!=nAtoms)
			{
				g_free(listOfAtoms);
				listOfAtoms = NULL;
 				fclose(file);
				g_free(t);
				return FALSE;
			}
			else 
			{
				geometry->listOfAtoms = listOfAtoms;
				geometry->numberOfAtoms = nAtoms;
			}
			break;
		}
	}
 	fclose(file);
 	g_free(t);
	return TRUE;
}
/*************************************************************************************************************/
static gint get_number_of_geomtries_in_gamess_trj(gchar *fileName)
{
 	gchar *t;
 	FILE *file;
	gint nG = 0;
  
 	file = FOpen(fileName, "rb");
	if(!file) return 0;
	t = g_malloc(BSIZE*sizeof(gchar));
 	while(!feof(file))
	{
  		if(!fgets(t,BSIZE,file))break;
		if(strstr( t,"MD DATA PACKET")) nG++;
	}
	g_free(t);
	fclose(file);
	return nG;
}
/*************************************************************************************************************/
static void scan_geomtries_position_in_gamess_trj(gchar *fileName)
{
 	gchar *t;
 	FILE *file;
	gint j;
	long int pos = -1;

	for(j=0;j<geometriesMD.numberOfGeometries;j++)
		geometriesMD.geometries[j].filePos = -1;
  
 	file = FOpen(fileName, "rb");
	if(!file) return ;
	t = g_malloc(BSIZE*sizeof(gchar));
	j = 0;
 	while(!feof(file))
	{
		/* pos = ftell(file);*/
  		if(!fgets(t,BSIZE,file))break;
		if(strstr( t,"MD DATA PACKET"))
		{ 
			pos = ftell(file);
			geometriesMD.geometries[j].filePos = pos; 
			j++;
		}
	}
	g_free(t);
	fclose(file);
}
/*************************************************************************************************************/
static gboolean read_MD_gamess_trj_file_step(gchar* fileName, gint step)
{
 	gchar *t;
 	gchar *pos;
	gint k = 0;
 	FILE *file;
	gint j = step;
	gint i = 0;
	AtomMD* listOfAtoms = NULL;

	geometriesMD.geometries[j].time = 0.0;
	geometriesMD.geometries[j].energy = 0.0;
	geometriesMD.geometries[j].comments = g_strdup_printf("Step n %d",step);
	geometriesMD.geometries[j].numberOfAtoms = geometriesMD.geometries[0].numberOfAtoms;
	if(j!=0) geometriesMD.geometries[j].listOfAtoms = g_malloc(geometriesMD.geometries[j].numberOfAtoms*sizeof(AtomMD));
	listOfAtoms = geometriesMD.geometries[j].listOfAtoms;
	for(i=0;i<geometriesMD.geometries[j].numberOfAtoms;i++)
	{
		if(j!=0) sprintf(listOfAtoms[i].symbol,"%s",geometriesMD.geometries[0].listOfAtoms[i].symbol);
		listOfAtoms[i].C[0] = 0;
		listOfAtoms[i].C[1] = 0;
		listOfAtoms[i].C[2] = 0;
		listOfAtoms[i].V[0] = 0;
		listOfAtoms[i].V[1] = 0;
		listOfAtoms[i].V[2] = 0;
		listOfAtoms[i].partialCharge = 0.0;
		listOfAtoms[i].variable = TRUE;
		listOfAtoms[i].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[i].symbol);
		sprintf(listOfAtoms[i].mmType,"%s",listOfAtoms[i].symbol);
		sprintf(listOfAtoms[i].pdbType,"%s",listOfAtoms[i].symbol);
		sprintf(listOfAtoms[i].resName,"%s",listOfAtoms[i].symbol);
		listOfAtoms[i].resNumber = i;
	}
	
	if(geometriesMD.geometries[j].filePos<0) return FALSE;
  
 	file = FOpen(fileName, "rb");
	if(!file) return FALSE;
	t = g_malloc(BSIZE*sizeof(gchar));
	fseek(file, geometriesMD.geometries[j].filePos, SEEK_SET);
	k = 0;
 	while(!feof(file))
	{
  		if(!fgets(t,BSIZE,file))break;
		if(strstr( t,"MD DATA PACKET")) break;
		if(strstr( t,"TTOTAL=") && strstr( t,"TOT. E="))
		{
			pos = strstr( t,"AL=")+3;
			for(i=0;i<strlen(t);i++) if(t[i]=='D' || t[i]=='d') t[i] = 'e';
			geometriesMD.geometries[j].time = atof(pos);
			k++;
		}
		if(strstr( t,"TOT. E="))
		{
			pos = strstr( t,"E=")+2;
			for(i=0;i<strlen(t);i++) if(t[i]=='D' || t[i]=='d') t[i] = 'e';
			geometriesMD.geometries[j].energy = atof(pos);
			k++;
		}
		if(strstr( t,"QM PARTICLE COORDINATES"))
		{
			gchar symb[10];
			gdouble dum;
			for(i=0;i<geometriesMD.geometries[j].numberOfAtoms;i++)
			{
  				if(!fgets(t,BSIZE,file))break;
				if(5!=sscanf(t,"%s %lf %lf %lf %lf",symb,&dum,
				&listOfAtoms[i].C[0],
				&listOfAtoms[i].C[1],
				&listOfAtoms[i].C[2]))break;
				listOfAtoms[i].C[0] *= ANG_TO_BOHR;
				listOfAtoms[i].C[1] *= ANG_TO_BOHR;
				listOfAtoms[i].C[2] *= ANG_TO_BOHR;
			}
			if(i!=geometriesMD.geometries[j].numberOfAtoms) break;
			k++;
		}
		if(strstr( t,"QM ATOM TRANS. VELOCITIES"))
		{
			for(i=0;i<geometriesMD.geometries[j].numberOfAtoms;i++)
			{
  				if(!fgets(t,BSIZE,file))break;
				if(3!=sscanf(t,"%lf %lf %lf",
				&listOfAtoms[i].V[0],
				&listOfAtoms[i].V[1],
				&listOfAtoms[i].V[2]))break;
			}
			if(i!=geometriesMD.geometries[j].numberOfAtoms) break;
			k++;
		}
		if(k==4) break;

	}

	g_free(t);
	fclose(file);
	if(k==4) return TRUE;
	return FALSE;
}
/********************************************************************************/
static gboolean read_gamess_trj(gchar* fileName)
{
	gint  j=0;
	gchar *t = NULL;
	gint nG = 0;
        
	t = get_name_file(fileName);
	set_status_label_info(_("File name"),t);
	g_free(t);
	t = NULL;
	set_status_label_info(_("File type"),"Gamess/FireFly TRJ file");

	nG=get_number_of_geomtries_in_gamess_trj(fileName);
	if(nG<1) 
	{
		t = g_strdup_printf(_(" Error : I can not read %s file \n"),fileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return FALSE;
	}
	geometriesMD.numberOfGeometries=nG;
	geometriesMD.geometries = g_malloc(geometriesMD.numberOfGeometries*sizeof(GeometryMD));
	geometriesMD.fileName = g_strdup(fileName);
	geometriesMD.typeOfFile = GABEDIT_TYPEFILE_TRJ;

	for(j=0;j<geometriesMD.numberOfGeometries;j++)
	{
		geometriesMD.geometries[j].numberOfAtoms = 0;
		geometriesMD.geometries[j].time = 0;
		geometriesMD.geometries[j].energy = 0;
		geometriesMD.geometries[j].listOfAtoms = NULL;
	}

	if(!read_gamess_trj_first_geometry(fileName, &geometriesMD.geometries[0])) 
	{
		freeGeometryMD();
		t = g_strdup_printf(_(" Error : I can not read the first geometry from %s file\n"),fileName);
		Message(t,_("Error"),TRUE);
  		rafreshList();
		return FALSE;
	}
	scan_geomtries_position_in_gamess_trj(fileName);
	for(j=0;j<geometriesMD.numberOfGeometries;j++)
		if(!read_MD_gamess_trj_file_step(fileName, j)) 
		{
			break;
		}
	if(j!=geometriesMD.numberOfGeometries) 
	{
		printf("j=%d\n",j);
		if(j>1)
		{
			if(geometriesMD.geometries[j].listOfAtoms) g_free(geometriesMD.geometries[j].listOfAtoms);
			if(geometriesMD.geometries[j].comments) g_free(geometriesMD.geometries[j].comments);
			geometriesMD.numberOfGeometries--;
			geometriesMD.geometries = g_realloc(geometriesMD.geometries,geometriesMD.numberOfGeometries*sizeof(GeometryMD));
		}
		else
		{
			freeGeometryMD();
			t = g_strdup_printf(_(" Error : I can not read step number %d from %s file\n"),j,fileName);
			Message(t,_("Error"),TRUE);
			g_free(t);
  			rafreshList();
			return FALSE;
		}
	}
  	rafreshList();
	return TRUE;
}
/********************************************************************************/
static gboolean read_gabedit_MD_file(gchar *fileName)
{
 	gchar* t;
 	gchar* tmp;
 	gchar* sdum;
 	gchar* sdum1;
 	gchar* sdum2;
 	gchar* sdum3;
 	gchar* pdest;
 	gdouble cdum1, cdum2, cdum3;
 	gdouble vdum1, vdum2, vdum3;
	gint idum,idum2;
 	gboolean OK;
 	FILE *file;
	gint i;
	gint j;
	gint k;
	gint nG = 0;
	gdouble pc;

	tmp = get_name_file(fileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Gabedit");

 	file = FOpen(fileName, "rb");
	t = g_malloc(BSIZE*sizeof(gchar));
	if(file ==NULL)
	{
		sprintf(t,_("Sorry\nI can not open %s file "),fileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return FALSE;
	}
	sdum = g_malloc(BSIZE*sizeof(gchar));
	sdum1 = g_malloc(BSIZE*sizeof(gchar));
	sdum2 = g_malloc(BSIZE*sizeof(gchar));
	sdum3 = g_malloc(BSIZE*sizeof(gchar));
	OK=FALSE;

	freeGeometryMD();
	while(!feof(file))
	{
		if(!fgets(t, BSIZE,file))break;
		pdest = strstr( t, "[MD]");
 		if (pdest)
		{
			if(!fgets(t, BSIZE,file))break;
			sscanf(t,"%d",&geometriesMD.numberOfGeometries);
			/*
			printf("geometriesMD.numberOfGeometries=%d\n",geometriesMD.numberOfGeometries);
			*/
			if(geometriesMD.numberOfGeometries<1) break;
			geometriesMD.geometries = g_malloc(geometriesMD.numberOfGeometries*sizeof(GeometryMD));
			geometriesMD.fileName = g_strdup(fileName);
			geometriesMD.typeOfFile = GABEDIT_TYPEFILE_GABEDIT;
			for(j=0;j<geometriesMD.numberOfGeometries;j++)
			{
				geometriesMD.geometries[j].numberOfAtoms = 0;
				geometriesMD.geometries[j].time = 0;
				geometriesMD.geometries[j].energy = 0;
				geometriesMD.geometries[j].listOfAtoms = NULL;
			}
			for(j=0;j<geometriesMD.numberOfGeometries;j++)
			{
				if(!fgets(t, BSIZE,file))break;
				sscanf(t,"%d %lf %lf",&geometriesMD.geometries[j].numberOfAtoms,
						&geometriesMD.geometries[j].time, &geometriesMD.geometries[j].energy);
				/*
			        printf("na = %d time = %lf ener = %lf\n", geometriesMD.geometries[j].numberOfAtoms,
						geometriesMD.geometries[j].time, geometriesMD.geometries[j].energy);
						*/
				if(!fgets(t, BSIZE,file))break;
				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
				if(strlen(t)>0) geometriesMD.geometries[j].comments=g_strdup(t);
				else geometriesMD.geometries[j].comments=NULL;
				if(geometriesMD.geometries[j].numberOfAtoms>0)
				{
					geometriesMD.geometries[j].listOfAtoms = 
						 g_malloc(geometriesMD.geometries[j].numberOfAtoms*sizeof(AtomMD));
				}
				for(i=0;i<geometriesMD.geometries[j].numberOfAtoms;i++)
				{
					gint ncv = 0;
					if(!fgets(t, BSIZE,file))break;
					for(k=0;k<strlen(t);k++)
						if(t[k]=='D' || t[k]=='d') t[k] = 'e';
					ncv = sscanf(t,"%s %lf %lf %lf %lf %lf %lf",
								sdum, 
								&cdum1, &cdum2, &cdum3,
								&vdum1, &vdum2, &vdum3
								);
					if(ncv != 7 && ncv != 4 ) break;
					sprintf(geometriesMD.geometries[j].listOfAtoms[i].symbol,"%s",sdum);
					geometriesMD.geometries[j].listOfAtoms[i].C[0] = cdum1*ANG_TO_BOHR;
					geometriesMD.geometries[j].listOfAtoms[i].C[1] = cdum2*ANG_TO_BOHR;
					geometriesMD.geometries[j].listOfAtoms[i].C[2] = cdum3*ANG_TO_BOHR;

					geometriesMD.geometries[j].listOfAtoms[i].V[0] = vdum1;
					geometriesMD.geometries[j].listOfAtoms[i].V[1] = vdum2;
					geometriesMD.geometries[j].listOfAtoms[i].V[2] = vdum3;
					geometriesMD.geometries[j].listOfAtoms[i].partialCharge = 0.0;
					geometriesMD.geometries[j].listOfAtoms[i].variable = TRUE;
					geometriesMD.geometries[j].listOfAtoms[i].nuclearCharge = 
						get_atomic_number_from_symbol(geometriesMD.geometries[j].listOfAtoms[i].symbol);
					ncv = sscanf(t,"%s %lf %lf %lf %lf %lf %lf %lf %s %s %s %d %d",
								sdum, 
								&cdum1, &cdum2, &cdum3,
								&vdum1, &vdum2, &vdum3,
								&pc,
								sdum1, sdum2,sdum3,&idum,&idum2);
					if(ncv==13)
					{
						sprintf(geometriesMD.geometries[j].listOfAtoms[i].mmType,"%s",sdum1);
						sprintf(geometriesMD.geometries[j].listOfAtoms[i].pdbType,"%s",sdum1);
						sprintf(geometriesMD.geometries[j].listOfAtoms[i].resName,"%s",sdum1);
						geometriesMD.geometries[j].listOfAtoms[i].resNumber = idum;
						geometriesMD.geometries[j].listOfAtoms[i].variable = idum2;
					}
					else
					if(ncv==12)
					{
						sprintf(geometriesMD.geometries[j].listOfAtoms[i].mmType,"%s",sdum1);
						sprintf(geometriesMD.geometries[j].listOfAtoms[i].pdbType,"%s",sdum1);
						sprintf(geometriesMD.geometries[j].listOfAtoms[i].resName,"%s",sdum1);
						geometriesMD.geometries[j].listOfAtoms[i].resNumber = idum;
					}
					else
					{
						sprintf(geometriesMD.geometries[j].listOfAtoms[i].mmType,
							"%s",geometriesMD.geometries[j].listOfAtoms[i].symbol);
						sprintf(geometriesMD.geometries[j].listOfAtoms[i].pdbType,
							"%s",geometriesMD.geometries[j].listOfAtoms[i].symbol);
						sprintf(geometriesMD.geometries[j].listOfAtoms[i].resName,
							"%s",geometriesMD.geometries[j].listOfAtoms[i].symbol);
						geometriesMD.geometries[j].listOfAtoms[i].resNumber = i;
					}
				}
			}
			nG = j;
			OK = TRUE;
		}
	}
	fclose(file);
 	g_free(t);
 	g_free(sdum);
 	g_free(sdum1);
 	g_free(sdum2);
 	g_free(sdum3);
	if(nG<=0) OK = FALSE;
	if(nG>0 && nG<geometriesMD.numberOfGeometries)
	{
		geometriesMD.numberOfGeometries = nG;
		geometriesMD.geometries = g_realloc(geometriesMD.geometries,
				geometriesMD.numberOfGeometries*sizeof(GeometryMD));
	}
	if(!OK) freeGeometryMD();
	rafreshList();
	return OK;
}
/********************************************************************************/
static void read_gabedit_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryMD();
  	rafreshList();
	read_gabedit_MD_file(FileName);
}
/********************************************************************************/
static void read_gaussian_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryMD();
  	rafreshList();
	read_gaussian_output(FileName);
	
	/*
	print_velocity_velocity_correlation_function("velocityAutoCorrelation.txt");
	compute_infrared_spectra(0, 10000, 10);
	*/

}
/********************************************************************************/
static void read_gamess_trj_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryMD();
  	rafreshList();
	read_gamess_trj(FileName);
	
	/*
	print_velocity_velocity_correlation_function("velocityAutoCorrelation.txt");
	compute_infrared_spectra(0, 10000, 10);
	*/

}
/*************************************************************************/
static void read_file(GabeditFileChooser *selecFile, gint response_id)
{
	gchar *fileName;
	GabEditTypeFile fileType = GABEDIT_TYPEFILE_UNKNOWN;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(selecFile);
	gtk_widget_hide(GTK_WIDGET(selecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	fileType = get_type_file(fileName);
	if(fileType == GABEDIT_TYPEFILE_TRJ) read_gamess_trj_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GAUSSIAN) read_gaussian_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GABEDIT) read_gabedit_file(selecFile, response_id);
	else 
	{
		Message(
			_("Sorry, I cannot find the type of your file\n")
			,_("Error"),TRUE);
	}
}
/********************************************************************************/
static void read_gabedit_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gabedit_file,
			_("Read geometries from a Gabedit file"),
			GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_gaussian_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gaussian_file, _("Read geometries from a Gaussian output file"), GABEDIT_TYPEFILE_GAUSSIAN,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_gamess_trj_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gamess_trj_file, _("Read geometries from a Gamess trj file"), GABEDIT_TYPEFILE_TRJ,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_file, _("Read geometries"), GABEDIT_TYPEFILE_UNKNOWN,GABEDIT_TYPEWIN_ORB);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void save_velocity_autocorrelation(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	print_velocity_velocity_correlation_function(FileName);
}
/********************************************************************************/
static void save_gabedit_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	save_geometry_MD_gabedit_format(FileName);
}
/********************************************************************************/
static void save_pdb_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	save_geometry_MD_pdb_format(FileName);
}
/********************************************************************************/
static gboolean set_geometry(gint k)
{
	AtomMD* listOfAtoms = NULL;
	gint nAtoms = 0;
	gint j;

	if(k<0 || k >= geometriesMD.numberOfGeometries) return FALSE;

	if(GeomOrb)
	{
		free_atomic_orbitals();
		for(j=0;j<nCenters;j++) if(GeomOrb[j].Symb) g_free(GeomOrb[j].Symb);
		g_free(GeomOrb);
		GeomOrb = NULL;
	}
	nAtoms = geometriesMD.geometries[k].numberOfAtoms;
	listOfAtoms = geometriesMD.geometries[k].listOfAtoms;

	GeomOrb=g_malloc(nAtoms*sizeof(TypeGeomOrb));
	for(j=0;j<nAtoms;j++)
	{
    		GeomOrb[j].Symb=g_strdup(listOfAtoms[j].symbol);
    		GeomOrb[j].C[0] = listOfAtoms[j].C[0];
    		GeomOrb[j].C[1] = listOfAtoms[j].C[1];
    		GeomOrb[j].C[2] = listOfAtoms[j].C[2];
  		GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
		GeomOrb[j].Prop.covalentRadii *=1.0;
		GeomOrb[j].partialCharge = listOfAtoms[j].partialCharge;
		GeomOrb[j].variable = listOfAtoms[j].variable;
		GeomOrb[j].nuclearCharge = listOfAtoms[j].nuclearCharge;
	}
	nCenters = nAtoms;
	init_atomic_orbitals();
	init_dipole();
	buildBondsOrb();
	RebuildGeomD = TRUE;
	glarea_rafresh(GLArea);

	return TRUE;
}
/********************************************************************************/
static void save_velocity_autocorrelation_dlg()
{
	GtkWidget* filesel;
	G_CONST_RETURN gchar* dir = g_get_current_dir();
	if(geometriesMD.numberOfGeometries<1)
	{
		return;
	}
 	filesel = file_chooser_save(save_velocity_autocorrelation,
			_("Save velocity-velocity autocorrelation function"),
			GABEDIT_TYPEFILE_UNKNOWN,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	if(dir)
	{
		gchar* t = g_strdup_printf("%s%svelocityAutoCorrelation.txt",dir,G_DIR_SEPARATOR_S);
		gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(filesel),t);
		g_free(t);
	}
	else
	{
		gchar* t = g_strdup_printf("%s%svelocityAutoCorrelation.txt",g_get_home_dir(),G_DIR_SEPARATOR_S);
		gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(filesel),t);
		g_free(t);
	}
}
/********************************************************************************/
static void save_gabedit_file_dlg()
{
	GtkWidget* filesel;
	if(geometriesMD.numberOfGeometries<1)
	{
		return;
	}
 	filesel = file_chooser_save(save_gabedit_file,
			_("Save geometries in gabedit file format"),
			GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void save_pdb_file_dlg()
{
	GtkWidget* filesel;
	if(geometriesMD.numberOfGeometries<1)
	{
		return;
	}
 	filesel = file_chooser_save(save_pdb_file,
			"Save geometries in pdb file format",
			GABEDIT_TYPEFILE_PDB,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void set_entry_inputGaussDir(GtkWidget* dirSelector, gint response_id)
{
	gchar* dirname = NULL;
	GtkWidget *entry;
	if(response_id != GTK_RESPONSE_OK) return;
	dirname = gabedit_folder_chooser_get_current_folder(GABEDIT_FOLDER_CHOOSER(dirSelector));


	entry = (GtkWidget*)(g_object_get_data(G_OBJECT(dirSelector),"EntryFile"));	
	gtk_entry_set_text(GTK_ENTRY(entry),dirname);

	if(inputGaussDirectory) g_free(inputGaussDirectory);
	inputGaussDirectory = g_strdup(dirname);
}
/********************************************************************************/
static void set_entry_inputGaussDir_selection(GtkWidget* entry)
{
	GtkWidget *dirSelector;
	dirSelector = selectionOfDir(set_entry_inputGaussDir, "Select folder for the input Gaussian files", GABEDIT_TYPEWIN_ORB); 
  	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
  	g_signal_connect(G_OBJECT(dirSelector),"delete_event", (GCallback)gtk_widget_destroy,NULL);

	g_object_set_data(G_OBJECT (dirSelector), "EntryFile", entry);

	g_signal_connect (dirSelector, "response",  G_CALLBACK (set_entry_inputGaussDir), GTK_OBJECT(dirSelector));
	g_signal_connect (dirSelector, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(dirSelector));

	gtk_widget_show(dirSelector);
}
/********************************************************************************/
static GtkWidget*   add_inputGauss_entrys(GtkWidget *Wins,GtkWidget *vbox,gboolean expand)
{
	GtkWidget *button;

	GtkWidget* entry;
	GtkWidget* sep;
  	GtkWidget *table = gtk_table_new(8,4,FALSE);
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* comboCharge = NULL;
	gint i;

	totalCharge = 0;
	spinMultiplicity=1;

	if(!inputGaussDirectory) inputGaussDirectory = g_strdup_printf("%s",g_get_home_dir());

	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	i = 0;
	add_label_table(table,_(" Directory "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),inputGaussDirectory);
	gtk_editable_set_editable((GtkEditable*)entry,FALSE);
	gtk_widget_set_sensitive(entry, FALSE);
	button = create_button_pixmap(Wins,open_xpm,NULL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(GTK_OBJECT (button), "clicked",
                                     G_CALLBACK(set_entry_inputGaussDir_selection),
                                     GTK_OBJECT(entry));
	add_widget_table(table,button,i,3);

	i = 1;
	comboCharge = addChargeToTable(table, i);
	i = 2;
	comboSpinMultiplicity = addSpinToTable(table, i);
	i = 3;
	sep = gtk_hseparator_new ();;
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  2,2);

	if(GTK_IS_COMBO_BOX(comboCharge))
		g_object_set_data(G_OBJECT (GTK_BIN(comboCharge)->child), "ComboSpinMultiplicity", comboSpinMultiplicity);
	setComboCharge(comboCharge);
	setComboSpinMultiplicity(comboSpinMultiplicity);
	g_signal_connect(G_OBJECT(GTK_BIN(comboCharge)->child),"changed", G_CALLBACK(changedEntryCharge),NULL);

	i = 4;
	add_label_table(table,_(" Keywords "),i,0);
	add_label_table(table,":",i,1);
  	entry = gtk_entry_new ();
	g_object_set_data(G_OBJECT(Wins), "EntryKeywords", entry);
	gtk_widget_set_size_request(GTK_WIDGET(entry),-1,32);
	gtk_table_attach(GTK_TABLE(table),entry,2,2+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
  	gtk_entry_set_text (GTK_ENTRY (entry),"B3LYP/6-31G* Opt");
	gtk_editable_set_editable((GtkEditable*)entry,TRUE);
	gtk_widget_set_sensitive(entry, TRUE);

	i = 5;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);


	i = 6;
	buttonChkgauss = gtk_check_button_new_with_label (_("check file"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonChkgauss), FALSE);
	gtk_table_attach(GTK_TABLE(table),buttonChkgauss,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);

	i = 7;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep,0,0+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);

	gtk_widget_show_all(table);
	return entry;
}
/********************************************************************************************************/
void  add_cancel_ok_button(GtkWidget *Win,GtkWidget *vbox,GtkWidget *entry, GCallback myFunc)
{
	GtkWidget *hbox;
	GtkWidget *button;
	/* buttons box */
	hbox = create_hbox_false(vbox);
	gtk_widget_realize(Win);

	button = create_button(Win,"Cancel");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, GTK_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,"OK");
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)myFunc,GTK_OBJECT(Win));
	if(entry)
	g_signal_connect_swapped(G_OBJECT (entry), "activate", (GCallback) gtk_button_clicked, GTK_OBJECT (button));

	gtk_widget_show_all(vbox);
}
/********************************************************************************/
static void create_gaussian_file_dlg(gint type)
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget* vbox;
	GtkWidget* entryKeywords;


	/* Principal Window */
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"Create a series of input files for Gaussian");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Input Gaussian");
	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vboxall = create_vbox(Win);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(vboxall), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
  	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

  	gtk_widget_realize(Win);
	entryKeywords = add_inputGauss_entrys(Win,vbox,TRUE);
	if(type==1) add_cancel_ok_button(Win,vbox,entryKeywords,(GCallback)print_gaussian_geometries_link);
	else if(type==2) add_cancel_ok_button(Win,vbox,entryKeywords,(GCallback)print_gaussian_geometries);
	else add_cancel_ok_button(Win,vbox,entryKeywords,(GCallback)print_gaussian_selected_geometry);

	/* Show all */
	gtk_widget_show_all (Win);
}
  
/********************************************************************************/
static void playAnimation(GtkWidget *win, gpointer data)
{
	play = TRUE;
	gtk_widget_set_sensitive(PlayButton, FALSE); 
	gtk_widget_set_sensitive(StopButton, TRUE); 
	gtk_window_set_modal (GTK_WINDOW (WinDlg), TRUE);
	animate();

}
/********************************************************************************/
static void stopAnimation(GtkWidget *win, gpointer data)
{
	play = FALSE;
	if(GTK_IS_WIDGET(PlayButton)) gtk_widget_set_sensitive(PlayButton, TRUE); 
	if(GTK_IS_WIDGET(StopButton)) gtk_widget_set_sensitive(StopButton, FALSE); 
	if(GTK_IS_WIDGET(WinDlg)) gtk_window_set_modal (GTK_WINDOW (WinDlg), FALSE);
	while( gtk_events_pending() ) gtk_main_iteration();

	buildBondsOrb();
	RebuildGeomD = TRUE;
	init_dipole();
	init_atomic_orbitals();
	free_iso_all();
	if(this_is_an_object((GtkObject*)GLArea)) glarea_rafresh(GLArea);
}
/********************************************************************************/
static void destroyDlg(GtkWidget *win)
{
	createFilm = FALSE;
	numFileFilm = 0;

	delete_child(WinDlg);
	freeGeometryMD();
}
/********************************************************************************/
static void destroyAnimGeomConvDlg(GtkWidget *win)
{
	createFilm = FALSE;
	numFileFilm = 0;
	stopAnimation(NULL, NULL);
	gtk_widget_destroy(WinDlg);
	WinDlg = NULL;
	rowSelected = -1;
	freeGeometryMD();

}
/**********************************************************************************/
static void set_sensitive_option(GtkUIManager *manager, gchar* path)
{
	GtkWidget *wid = gtk_ui_manager_get_widget (manager, path);
	gboolean sensitive = TRUE;
  	if(geometriesMD.numberOfGeometries<1) sensitive = FALSE;
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, sensitive);
}
/**********************************************************************************/
static gboolean show_menu_popup(GtkUIManager *manager, guint button, guint32 time)
{
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuGeomMD");
	if (GTK_IS_MENU (menu)) 
	{
		set_sensitive_option(manager,"/MenuGeomMD/SaveGabedit");
		set_sensitive_option(manager,"/MenuGeomMD/SavePDB");
		set_sensitive_option(manager,"/MenuGeomMD/SaveVelocityAutocorrelation");
		set_sensitive_option(manager,"/MenuGeomMD/DisplayVelocityAutocorrelation");
		set_sensitive_option(manager,"/MenuGeomMD/CreateGaussInput");
		set_sensitive_option(manager,"/MenuGeomMD/CreateGaussInputLink");
		set_sensitive_option(manager,"/MenuGeomMD/CreateGaussInputSelected");
		set_sensitive_option(manager,"/MenuGeomMD/CreateGr");
		set_sensitive_option(manager,"/MenuGeomMD/ComputeConformerTypes");
		set_sensitive_option(manager,"/MenuGeomMD/ComputeRMSD");
		set_sensitive_option(manager,"/MenuGeomMD/DeleteGeometry");
		set_sensitive_option(manager,"/MenuGeomMD/DeleteHalfGeometries");
		set_sensitive_option(manager,"/MenuGeomMD/DeleteBeforeSelectedGeometry");
		set_sensitive_option(manager,"/MenuGeomMD/DeleteAfterSelectedGeometry");
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, time);
		return TRUE;
	}
	else printf("Erreur menu n'est pas un menu\n");
	return FALSE;
}
/**********************************************************************************/
static void event_dispatcher(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	GtkTreePath *path;
	gint row = -1;
	if (event->window == gtk_tree_view_get_bin_window (GTK_TREE_VIEW (widget))
	    && !gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget),
					       event->x, event->y, NULL, NULL, NULL, NULL)) {
		gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)));
	}
	if(gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, &path, NULL, NULL, NULL))
	{
		if(path)
		{
			gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)), path);
			row = atoi(gtk_tree_path_to_string(path));
			gtk_tree_path_free(path);
		}
	}

	rowSelected = row;
	if(!play) stopAnimation(NULL, NULL);
	if(row>=0) set_geometry(rowSelected);

  	if (event->type == GDK_BUTTON_PRESS && ((GdkEventButton *) event)->button == 3)
	{
		GdkEventButton *bevent = (GdkEventButton *) event;
		GtkUIManager *manager = GTK_UI_MANAGER(user_data);
		show_menu_popup(manager, bevent->button, bevent->time);
	}
}
/**********************************************************************************/
static void rafreshList()
{
	gint i;
	gint k;
        GtkTreeIter iter;
	GtkTreeModel *model = NULL;
        GtkTreeStore *store = NULL;
	gchar *texts[3];
	gboolean visible[3] = { TRUE, TRUE, TRUE};
	gchar* titles[]={"Energy(Hartree)","Time(fs)","Comments"};
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	model = gtk_tree_view_get_model(treeView);
        store = GTK_TREE_STORE (model);
	gtk_tree_store_clear(store);
        model = GTK_TREE_MODEL (store);

	if(geometriesMD.numberOfGeometries>0)
	{
		for(i=0;i<geometriesMD.numberOfGeometries;i++)
			if(!geometriesMD.geometries[i].comments) { visible[2] = FALSE; break;}
	}
	else
	{
		for(i=0;i<3;i++) visible[i] = FALSE;
	}

	do
	{
		column = gtk_tree_view_get_column(treeView, 0);
		if(column) gtk_tree_view_remove_column(treeView, column);
	}while(column != NULL);

	for(k=0;k<3;k++)
	{
		if(!visible[k]) continue;
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, titles[k]);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		gtk_tree_view_column_set_resizable(column,TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", k, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (treeView), column);
	}

  
	for(i=0;i<geometriesMD.numberOfGeometries;i++)
	{
		if(visible[0]) texts[0] = g_strdup_printf("%lf",geometriesMD.geometries[i].energy);
		if(visible[1]) texts[1] = g_strdup_printf("%lf",geometriesMD.geometries[i].time);
		if(visible[2]) texts[2] = g_strdup_printf("%s",geometriesMD.geometries[i].comments);
        	gtk_tree_store_append (store, &iter, NULL);
		for(k=0;k<3;k++)
		{
			if(visible[k])
			{
				gtk_tree_store_set (store, &iter, k, texts[k], -1);
				g_free(texts[k]);
			}
		}
	}

	if(geometriesMD.numberOfGeometries>0)
	{
		GtkTreePath *path;
		rowSelected = 0;
		path = gtk_tree_path_new_from_string  ("0");
		gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeView)), path);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW (treeView), path, NULL, FALSE,0.5,0.5);
		gtk_tree_path_free(path);
		set_geometry(rowSelected);
	}
	
	reset_parameters(NULL,NULL);
}
/*****************************************************************************/
static gchar* get_format_image_from_option()
{
	if(strcmp(formatFilm,"BMP")==0) return "bmp";
	if(strcmp(formatFilm,"PPM")==0) return "ppm";
	if(strcmp(formatFilm,"PNG")==0) return "png";
	if(strcmp(formatFilm,"JPEG")==0) return "jpg";
	if(strcmp(formatFilm,"PNG transparent")==0) return "png";
	if(strcmp(formatFilm,"Povray")==0) return "pov";
	return "UNK";
}
/********************************************************************************/
static void filmSelected(GtkWidget *widget)
{
	if(GTK_IS_WIDGET(buttonCheckFilm)&& GTK_TOGGLE_BUTTON (buttonCheckFilm)->active)
	{
		createFilm = TRUE;
		if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, TRUE);
		if(GTK_IS_WIDGET(comboListFilm))  gtk_widget_set_sensitive(comboListFilm, TRUE);
	}
	else
	{
		createFilm = FALSE;
		if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);
		if(GTK_IS_WIDGET(comboListFilm))  gtk_widget_set_sensitive(comboListFilm, FALSE);
	}
}
/********************************************************************************/
static void showMessageEnd()
{
	gchar* format =get_format_image_from_option();
	gchar* message = messageAnimatedImage(format);
	gchar* t = g_strdup_printf(_("\nA seriess of gab*.%s files was created in \"%s\" directeory.\n\n\n%s") , format, get_last_directory(),message);
	GtkWidget* winDlg = Message(t,_("Info"),TRUE);
	g_free(message);
	gtk_window_set_modal (GTK_WINDOW (winDlg), TRUE);
	g_free(t);
}
/********************************************************************************/
static void unActivateFilm()
{
	createFilm = FALSE;
	numFileFilm = 0;
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheckFilm), FALSE);
	if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);
	if(GTK_IS_WIDGET(comboListFilm))  gtk_widget_set_sensitive(comboListFilm, FALSE);
}
/********************************************************************************************************/
static void set_format_film(GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	if(!data) return;
	sprintf(formatFilm ,"%s",(gchar*)data);
}
/********************************************************************************************************/
static GtkWidget *create_list_of_formats()
{
        GtkTreeIter iter;
        GtkListStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
	gint k;
	gchar* options[] = {"BMP","PPM", "JPEG", "PNG", "PNG transparent", "Povray"};
	guint numberOfElements = G_N_ELEMENTS (options);


	k = 0;
	store = gtk_list_store_new (1,G_TYPE_STRING);
	for(i=0;i<numberOfElements; i++)
	{
        	gtk_list_store_append (store, &iter);
        	gtk_list_store_set (store, &iter, 0, options[i], -1);
		if(strcmp(options[i],formatFilm)==0) k = i;
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(set_format_film), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);
  	gtk_combo_box_set_active(GTK_COMBO_BOX (combobox), k);
	return combobox;
}
/********************************************************************************************************/
static void addEntriesButtons(GtkWidget* box)
{
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *table;
	gchar t[BSIZE];
	gint i;
	GtkWidget *separator;
	GtkWidget* formatBox;

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(box), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);

  	table = gtk_table_new(5,3,FALSE);
	gtk_box_pack_start(GTK_BOX(vboxframe), table,TRUE,TRUE,0);

	i = 0;
	add_label_table(table,_(" Time step(s) "),(gushort)i,0);
	add_label_table(table," : ",(gushort)i,1); 
	EntryVelocity = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),EntryVelocity,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	gtk_editable_set_editable((GtkEditable*) EntryVelocity,TRUE);
	sprintf(t,"%lf",geometriesMD.velocity);
	gtk_entry_set_text(GTK_ENTRY(EntryVelocity),t);

	i++;
	separator = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(table),separator,0,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);

  	table = gtk_table_new(2,3,FALSE);
	gtk_box_pack_start(GTK_BOX(vboxframe), table,TRUE,TRUE,0);

	i=0;
	buttonCheckFilm = gtk_check_button_new_with_label (_("Create a film"));
	createFilm = FALSE;
	numFileFilm = 0;
	gtk_table_attach(GTK_TABLE(table),buttonCheckFilm,0,1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  	g_signal_connect (G_OBJECT(buttonCheckFilm), "toggled", G_CALLBACK (filmSelected), NULL);  

	formatBox = create_list_of_formats();
	gtk_table_attach(GTK_TABLE(table),formatBox,1,1+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);

	buttonDirFilm = create_button(WinDlg,"Folder");
	gtk_table_attach(GTK_TABLE(table),buttonDirFilm,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  	g_signal_connect(G_OBJECT(buttonDirFilm), "clicked",(GCallback)set_directory,NULL);
	comboListFilm = formatBox;

	if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);
	if(GTK_IS_WIDGET(comboListFilm))  gtk_widget_set_sensitive(comboListFilm, FALSE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCheckFilm), FALSE);


	i++;
	separator = gtk_hseparator_new();
	gtk_table_attach(GTK_TABLE(table),separator,0,2,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);



  	table = gtk_table_new(1,2,TRUE);
	gtk_box_pack_start (GTK_BOX (vboxframe), table, TRUE, TRUE, 0);
	i=0;
	Button = create_button(WinDlg,"Play");
	gtk_table_attach(GTK_TABLE(table),Button,0,0+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	PlayButton = Button;

	Button = create_button(WinDlg,"Stop");
	gtk_table_attach(GTK_TABLE(table),Button,1,1+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	StopButton = Button;

  	g_signal_connect(G_OBJECT(PlayButton), "clicked",(GCallback)playAnimation,NULL);
  	g_signal_connect(G_OBJECT(StopButton), "clicked",(GCallback)stopAnimation,NULL);
  	g_signal_connect_swapped(G_OBJECT (EntryVelocity), "activate", (GCallback)reset_parameters, NULL);
}
/********************************************************************************/
static GtkTreeView* addList(GtkWidget *vbox, GtkUIManager *manager)
{
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeView *treeView;
	GtkTreeViewColumn *column;
	GtkWidget *scr;
	gint i;
	gint widall=0;
	gint widths[]={10,10,10};
	gchar* titles[]={"Energy(Hartree)","Time(fs)","Comments"};
	gint Factor=7;
	gint len = 3;


	for(i=0;i<len;i++) widall+=widths[i];

	widall=widall*Factor+40;

	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeightD*0.53));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 2);

	store = gtk_tree_store_new (7,G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
        model = GTK_TREE_MODEL (store);

	treeView = (GtkTreeView*)gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeView), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeView), TRUE);
  	gtk_container_add(GTK_CONTAINER(scr),GTK_WIDGET(treeView));


	for (i=0;i<len;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, titles[i]);
		gtk_tree_view_column_set_min_width(column, widths[i]*Factor);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		gtk_tree_view_column_set_sizing (column, GTK_TREE_VIEW_COLUMN_GROW_ONLY);
		gtk_tree_view_column_set_resizable(column,TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (treeView), column);
	}

	gtk_tree_view_set_reorderable(treeView, TRUE);
	set_base_style(GTK_WIDGET(treeView), 55000,55000,55000);
	gtk_widget_show (GTK_WIDGET(treeView));
  	g_signal_connect(G_OBJECT (treeView), "button_press_event", G_CALLBACK(event_dispatcher), manager);      
	return treeView;
}
/*****************************************************************************/
static gboolean createImagesFile()
{
	gchar* message = NULL;
	gchar* t;
	gchar* format;
	if(!createFilm)
	{
		setTextInProgress(" ");
		return FALSE;
	}
	format =get_format_image_from_option();
	t = g_strdup_printf(_("The %s%sgab%d.%s file was created"), get_last_directory(),G_DIR_SEPARATOR_S,numFileFilm, format);

	if(!strcmp(formatFilm,"BMP")) message = new_bmp(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"PPM")) message = new_ppm(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"JPEG")) message = new_jpeg(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"PNG")) message = new_png(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"Povray")) message = new_pov(get_last_directory(), ++numFileFilm);
	if(!strcmp(formatFilm,"PNG transparent")) message = new_png_without_background(get_last_directory(), ++numFileFilm);

	if(message == NULL) setTextInProgress(t);
	else
	{
    		GtkWidget* m;
		createFilm = FALSE;
		numFileFilm = 0;
    		m = Message(message,_("Error"),TRUE);
		gtk_window_set_modal (GTK_WINDOW (m), TRUE);
	}
	g_free(t);
	return TRUE;
}
/********************************************************************************/
static void computeConformerTypes()
{

	gint i;
	gchar* old = NULL;
	gchar* results5 = NULL;
	gchar* results6 = NULL;
	gint nRings5 = 0;
	gint nRings6 = 0;
	gint n = 0;
	GList** rings = NULL;
	gint k5=0;
	gint k6=0;

	play = TRUE;
        gtk_widget_set_sensitive(PlayButton, FALSE);
        gtk_widget_set_sensitive(StopButton, TRUE);


	for(i=0;i<geometriesMD.numberOfGeometries;i++)
	{
		gchar* energy = geometriesMD.geometries[i].energy?g_strdup_printf("%15.7f",geometriesMD.geometries[i].energy):g_strdup("0.0");
		set_geometry(i);

		nRings5 = getNumberOfRing5();
		nRings6 = getNumberOfRing6();
		if(nRings5<1 && nRings6<1) continue;
		if(nRings5>0) { 

			gchar* result = computeConformerTypeRing5MinInfo(energy, k5==0);
			if(result)
			{
				printf("%5d/%-5d : %s\n",i+1,geometriesMD.numberOfGeometries,result);
				k5++;
				if(results5)
				{
					old = results5;
					results5 = g_strdup_printf("%s%s\n",old, result);
                			if(old) g_free(old);
				}
				else results5 = g_strdup_printf("%s\n",result);
				if(result) g_free(result);
			}
		}
		if(nRings6>0) { 
			gchar* result = computeConformerTypeRing6MinInfo(energy, k6==0);
			if(result)
			{
				printf("%5d/%-5d : %s\n",i+1,geometriesMD.numberOfGeometries,result);
				k6++;
				if(results6)
				{
					old = results6;
					results6 = g_strdup_printf("%s%s\n",old, result);
                			if(old) g_free(old);
				}
				else results6 = g_strdup_printf("%s\n", result);
				if(result) g_free(result);
			}
		}
		if(energy) g_free(energy);
		if(!play) break;
	}
	//if(results5) printf("%s\n",results5);
	//if(results6) printf("%s\n",results6);
	if(results5)
        {
                GtkWidget* message = NULL;
		old = results5;
		results5 = g_strdup_printf("%s%s\n",
		"==============================================================================================================\n"
		"Type of conformation calculated using the method given in\n"
		"C. Altona and M. Sundaralingam, Journal of the American Chemical Society, 94:23 (1972) 8205–8212\n"
		"==============================================================================================================\n"
		,old 
		);
               	if(old) g_free(old);
                message = MessageTxt(results5,"Pentagons");
                gtk_window_set_default_size (GTK_WINDOW(message),(gint)(ScreenWidthD*0.8),-1);
                gtk_widget_set_size_request(message,(gint)(ScreenWidthD*0.45),-1);
        }
	if(results6)
        {
                GtkWidget* message = NULL;
		old = results6;
		results6 = g_strdup_printf("%s%s\n",
		"============================================================================\n"
		"Type of conformation calculated using the method given in\n"
		"Anthony D Hill and Peter J. Reilly J. Chem. Inf. Model, 47 (2007) 1031-1035\n"
		"============================================================================\n"
		,old 
		);
               	if(old) g_free(old);
                message = MessageTxt(results6,"Hexagons");
                gtk_window_set_default_size (GTK_WINDOW(message),(gint)(ScreenWidthD*0.8),-1);
                gtk_widget_set_size_request(message,(gint)(ScreenWidthD*0.45),-1);
        }
	play = FALSE;
        gtk_widget_set_sensitive(PlayButton, TRUE);
        gtk_widget_set_sensitive(StopButton, FALSE);

}
/********************************************************************************/
static void animate()
{

	gint m = -1;
	gint step = +1;
	GtkTreePath *path;
	gchar* t;


	reset_parameters(NULL, NULL);

	numFileFilm = 0;

	if(GTK_IS_WIDGET(buttonCheckFilm)) gtk_widget_set_sensitive(buttonCheckFilm, FALSE);
	if(GTK_IS_WIDGET(comboListFilm)) gtk_widget_set_sensitive(comboListFilm, FALSE);
	if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);


  	if(GTK_IS_WIDGET(WinDlg))
	{
		GtkWidget* handelbox = g_object_get_data(G_OBJECT (WinDlg), "HandelBox");
		if(GTK_IS_WIDGET(handelbox))  gtk_widget_set_sensitive(handelbox, FALSE);
	}
	if(GTK_IS_WIDGET(treeView))gtk_widget_set_sensitive(GTK_WIDGET(treeView), FALSE);

	if(GeomOrb) free_atomic_orbitals();
	if(geometriesMD.numberOfGeometries<1) play = FALSE;

	while(play)
	{
		while( gtk_events_pending() ) gtk_main_iteration();

		m += step;
		if(m>=geometriesMD.numberOfGeometries)
		{
			m--;
			step = -1;
			continue;
		}
		if(m<0)
		{
			m++;
			step = +1;
			continue;
		}
		if(m==0 && step<0)
		{
			if(numFileFilm>0) showMessageEnd();
			unActivateFilm();
		}
		rowSelected = m;
		t = g_strdup_printf("%d",m);
		path = gtk_tree_path_new_from_string  (t);
		g_free(t);
		gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeView)), path);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW (treeView), path, NULL, FALSE,0.5,0.5);
		gtk_tree_path_free(path);
		set_geometry(rowSelected);

		createImagesFile();
		Waiting(geometriesMD.velocity);
	}
	if(numFileFilm>0) showMessageEnd();
	unActivateFilm();

	if(GTK_IS_WIDGET(buttonCheckFilm)) gtk_widget_set_sensitive(buttonCheckFilm, TRUE);
	if(GTK_IS_WIDGET(comboListFilm)) gtk_widget_set_sensitive(comboListFilm, FALSE);
	if(GTK_IS_WIDGET(buttonDirFilm))  gtk_widget_set_sensitive(buttonDirFilm, FALSE);

	if(GTK_IS_WIDGET(treeView))gtk_widget_set_sensitive(GTK_WIDGET(treeView), TRUE);
  	if(GTK_IS_WIDGET(WinDlg))
	{
		GtkWidget* handelbox = g_object_get_data(G_OBJECT (WinDlg), "HandelBox");
		if(GTK_IS_WIDGET(handelbox))  gtk_widget_set_sensitive(handelbox, TRUE);
	}
	stopAnimation(NULL, NULL);
}
/********************************************************************************************/
static void help_supported_format()
{
	gchar temp[BSIZE];
	GtkWidget* win;
	sprintf(temp,
		_(
		" You can read more geometries from :\n"
	        "     * a Gabedit input file.\n"
	        "     * a Gamess/FireFly TRJ file.\n"
	        "     * a Gaussian output file.\n"
		)
		 );
	win = Message(temp,_("Info"),FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
}
/***************************************************************************/
static void help_animated_file()
{
	GtkWidget* win;
	gchar* temp = NULL;
	temp = g_strdup(
		_(
		" For create an animated file :\n"
		" ============================\n"
	        "   1) Read geometries from a Gaussian, Gamess TRJ, FireFly TRJ or from Gabedit file.\n"
	        "   2) Select \"create a seriess of BMP (or PPM or POV) images\" button.\n"
	        "      You can select your favorite directory by clicking to \"Directory\" button.\n"
	        "   3) Click to Play button.\n"
	        "   4) After on cycle Gabedit create a seriess of BMP(gab*.bmp) or PPM (gab*.ppm)  or POV(gab*.pov) files.\n"
	        "      From these files, you can create a gif or a png animated file using convert software.\n"
	        "              with \"convert -delay 10 -loop 1000 gab*.bmp imageAnim.gif\" command you can create a gif animated file.\n"
	        "              with \"convert -delay 10 -loop 1000 gab*.bmp imageAnim.mng\" command you can create a png animated file.\n\n"
	        "         For gab*.pov files :\n"
	        "            You must initially create files gab*.bmp using gab*.pov files.\n"
	        "            Then you can create the animated file using gab*.bmp file.\n\n"
	        "            You can use the xPovAnim script (from utils/povray directory) for create the gif animated file from gab*.pov files.\n\n"
	        "            convert is a free software. You can download this(for any system) from http://www.imagemagick.org\n"
	        "            povray is a free software. You can download this(for any system) from http://www.povray.org\n\n"
		)
		 );
	win = Message(temp,_("Info"),FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
	g_free(temp);
}
/*********************************************************************************************************************/
static void activate_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	/* const gchar *typename = G_OBJECT_TYPE_NAME (action);*/

	if(!strcmp(name, "File"))
	{
		GtkUIManager *manager = g_object_get_data(G_OBJECT(action), "Manager");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/SaveGabedit");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/SavePDB");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/SaveVelocityAutocorrelation");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/DisplayVelocityAutocorrelation");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/CreateGaussInput");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/CreateGaussInputLink");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/CreateGaussInputSelected");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/CreateGr");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/ComputeConformerTypes");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/ComputeRMSD");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/DeleteGeometry");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/DeleteHalfGeometries");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/DeleteBeforeSelectedGeometry");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/DeleteAfterSelectedGeometry");
	}
	else if(!strcmp(name, "ReadAuto")) read_file_dlg();
	else if(!strcmp(name, "ReadGabedit")) read_gabedit_file_dlg();
	else if(!strcmp(name, "ReadGaussian")) read_gaussian_file_dlg();
	else if(!strcmp(name, "ReadGamess")) read_gamess_trj_file_dlg();
	else if(!strcmp(name, "SaveGabedit")) save_gabedit_file_dlg();
	else if(!strcmp(name, "SavePDB")) save_pdb_file_dlg();
	else if(!strcmp(name, "SaveVelocityAutocorrelation")) save_velocity_autocorrelation_dlg();
	else if(!strcmp(name, "DisplayVelocityAutocorrelation")) display_velocity_velocity_correlation_function();
	else if(!strcmp(name, "CreateGaussInput")) create_gaussian_file_dlg(2);
	else if(!strcmp(name, "CreateGaussInputLink")) create_gaussian_file_dlg(1);
	else if(!strcmp(name, "CreateGaussInputSelected")) create_gaussian_file_dlg(3);
	else if(!strcmp(name, "CreateGr")) create_gr_dlg();
	else if(!strcmp(name, "ComputeConformerTypes")) computeConformerTypes();
	else if(!strcmp(name, "ComputeRMSD")) create_rmsd_dlg();
	else if(!strcmp(name, "DeleteGeometry")) delete_one_geometry();
	else if(!strcmp(name, "DeleteHalfGeometries")) delete_half_geometries();
	else if(!strcmp(name, "DeleteBeforeSelectedGeometry")) delete_beforee_selected_geometry();
	else if(!strcmp(name, "DeleteAfterSelectedGeometry")) delete_after_selected_geometry();
	else if(!strcmp(name, "Close")) destroyDlg(NULL);
	else if(!strcmp(name, "HelpSupportedFormat")) help_supported_format();
	else if(!strcmp(name, "HelpAnimation")) help_animated_file();
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntries[] =
{
	{"File",     NULL, N_("_File"), NULL, NULL, G_CALLBACK (activate_action)},
	{"Read",     NULL, N_("_Read")},
	{"ReadAuto", NULL, N_("Read a file(Auto)"), NULL, "Read a file", G_CALLBACK (activate_action) },
	{"ReadGabedit", GABEDIT_STOCK_GABEDIT, N_("Read a G_abedit file"), NULL, "Read a Gabedit file", G_CALLBACK (activate_action) },
	{"ReadGaussian", GABEDIT_STOCK_GAUSSIAN, N_("Read a _Gaussian output file"), NULL, "Read a Gaussian output file", G_CALLBACK (activate_action) },
	{"ReadGamess", GABEDIT_STOCK_GAMESS, N_("Read a Games_s trj file"), NULL, "Read a Gamess trj file", G_CALLBACK (activate_action) },
	{"SaveGabedit", GABEDIT_STOCK_SAVE, N_("_Save"), NULL, "Save", G_CALLBACK (activate_action) },
	{"SavePDB", GABEDIT_STOCK_PDB, N_("_Save as pdb file "), NULL, "Save as pdb", G_CALLBACK (activate_action) },
	{"SaveVelocityAutocorrelation", GABEDIT_STOCK_SAVE, N_("_Save velocity-velocity autocorrelation function"), NULL, "Save velocity-velocity autocorrelation function", G_CALLBACK (activate_action) },
	{"DisplayVelocityAutocorrelation", NULL, N_("_Display velocity-velocity autocorrelation function"), NULL, "Display velocity-velocity autocorrelation function", G_CALLBACK (activate_action) },
	{"CreateGaussInputSelected", GABEDIT_STOCK_GAUSSIAN, N_("_Create a gaussian input file for the selected geometry"), NULL, "Save", G_CALLBACK (activate_action) },
	{"CreateGaussInput", GABEDIT_STOCK_GAUSSIAN, N_("_Create a series of single input file for Gaussian"), NULL, "Save", G_CALLBACK (activate_action) },
	{"CreateGaussInputLink", GABEDIT_STOCK_GAUSSIAN, N_("Create _single input file for Gaussian with more geometries"), NULL, "Save", G_CALLBACK (activate_action) },
	{"CreateGr", GABEDIT_STOCK_GAUSSIAN, N_("Compute pair _radial distribution"), NULL, "Gr", G_CALLBACK (activate_action) },
	{"ComputeConformerTypes", GABEDIT_STOCK_GAUSSIAN, N_("Compute conformer _Types"), NULL, "Gr", G_CALLBACK (activate_action) },
	{"ComputeRMSD", GABEDIT_STOCK_GAUSSIAN, N_("_Compute RMSD"), NULL, "RMSD", G_CALLBACK (activate_action) },
	{"DeleteGeometry", GABEDIT_STOCK_CUT, N_("_Delete selected geometry"), NULL, "Delete selected geometry", G_CALLBACK (activate_action) },
	{"DeleteHalfGeometries", GABEDIT_STOCK_CUT, N_("Remove the _half of the geometries"), NULL, "remove the half of the geometries", G_CALLBACK (activate_action) },
	{"DeleteBeforeSelectedGeometry", GABEDIT_STOCK_CUT, N_("Remove geometries beforee the selected geometry"), NULL, "remove beforee", G_CALLBACK (activate_action) },
	{"DeleteAfterSelectedGeometry", GABEDIT_STOCK_CUT, N_("Remove geometries after the selected geometry"), NULL, "remove beforee", G_CALLBACK (activate_action) },
	{"Close", GABEDIT_STOCK_CLOSE, N_("_Close"), NULL, "Close", G_CALLBACK (activate_action) },
	{"Help",     NULL, N_("_Help")},
	{"HelpSupportedFormat", NULL, N_("_Supported format..."), NULL, "Supported format...", G_CALLBACK (activate_action) },
	{"HelpAnimation", NULL, N_("Creation of an _animated file..."), NULL, "Creation of an animated file...", G_CALLBACK (activate_action) },
};
static guint numberOfGtkActionEntries = G_N_ELEMENTS (gtkActionEntries);
/********************************************************************************/
/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiMenuInfo =
"  <popup name=\"MenuGeomMD\">\n"
"    <separator name=\"sepMenuPopGabedit\" />\n"
"    <menuitem name=\"ReadAuto\" action=\"ReadAuto\" />\n"
"    <separator name=\"sepMenuAuto\" />\n"
"    <menuitem name=\"ReadGabedit\" action=\"ReadGabedit\" />\n"
"    <menuitem name=\"ReadGaussian\" action=\"ReadGaussian\" />\n"
"    <menuitem name=\"ReadGamess\" action=\"ReadGamess\" />\n"
"    <separator name=\"sepMenuPopSave\" />\n"
"    <menuitem name=\"SaveGabedit\" action=\"SaveGabedit\" />\n"
"    <menuitem name=\"SavePDB\" action=\"SavePDB\" />\n"
"    <separator name=\"sepMenuPopSaveVelocityAutocorrelation\" />\n"
"    <menuitem name=\"SaveVelocityAutocorrelation\" action=\"SaveVelocityAutocorrelation\" />\n"
"    <separator name=\"sepMenuPopDisplayVelocityAutocorrelation\" />\n"
"    <menuitem name=\"DisplayVelocityAutocorrelation\" action=\"DisplayVelocityAutocorrelation\" />\n"
"    <separator name=\"sepMenuCreateGauss\" />\n"
"    <menuitem name=\"CreateGaussInputSelected\" action=\"CreateGaussInputSelected\" />\n"
"    <menuitem name=\"CreateGaussInput\" action=\"CreateGaussInput\" />\n"
"    <menuitem name=\"CreateGaussInputLink\" action=\"CreateGaussInputLink\" />\n"
"    <separator name=\"sepMenuCreateGr\" />\n"
"    <menuitem name=\"CreateGr\" action=\"CreateGr\" />\n"
"    <separator name=\"sepMenuComputeConformerTypes\" />\n"
"    <menuitem name=\"ComputeConformerTypes\" action=\"ComputeConformerTypes\" />\n"
"    <separator name=\"sepMenuComputeRMSD\" />\n"
"    <menuitem name=\"ComputeRMSD\" action=\"ComputeRMSD\" />\n"
"    <separator name=\"sepMenuDelete\" />\n"
"    <menuitem name=\"DeleteGeometry\" action=\"DeleteGeometry\" />\n"
"    <menuitem name=\"DeleteHalfGeometries\" action=\"DeleteHalfGeometries\" />\n"
"    <menuitem name=\"DeleteBeforeSelectedGeometry\" action=\"DeleteBeforeSelectedGeometry\" />\n"
"    <menuitem name=\"DeleteAfterSelectedGeometry\" action=\"DeleteAfterSelectedGeometry\" />\n"
"    <separator name=\"sepMenuPopClose\" />\n"
"    <menuitem name=\"Close\" action=\"Close\" />\n"
"  </popup>\n"
"  <menubar name = \"MenuBar\">\n"
"    <menu name=\"File\" action=\"File\">\n"
"      <menu name=\"Read\" action=\"Read\">\n"
"        <menuitem name=\"ReadAuto\" action=\"ReadAuto\" />\n"
"        <separator name=\"sepMenuAuto\" />\n"
"        <menuitem name=\"ReadGabedit\" action=\"ReadGabedit\" />\n"
"        <menuitem name=\"ReadGaussian\" action=\"ReadGaussian\" />\n"
"        <menuitem name=\"ReadGamess\" action=\"ReadGamess\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuSave\" />\n"
"      <menuitem name=\"SaveGabedit\" action=\"SaveGabedit\" />\n"
"      <menuitem name=\"SavePDB\" action=\"SavePDB\" />\n"
"      <separator name=\"sepMenuSaveVelocityAutocorrelation\" />\n"
"      <menuitem name=\"SaveVelocityAutocorrelation\" action=\"SaveVelocityAutocorrelation\" />\n"
"      <separator name=\"sepMenuDisplayVelocityAutocorrelation\" />\n"
"      <menuitem name=\"DisplayVelocityAutocorrelation\" action=\"DisplayVelocityAutocorrelation\" />\n"
"      <separator name=\"sepMenuCreateGauss\" />\n"
"      <menuitem name=\"CreateGaussInputSelected\" action=\"CreateGaussInputSelected\" />\n"
"      <menuitem name=\"CreateGaussInput\" action=\"CreateGaussInput\" />\n"
"      <menuitem name=\"CreateGaussInputLink\" action=\"CreateGaussInputLink\" />\n"
"      <separator name=\"sepMenuCreateGr\" />\n"
"      <menuitem name=\"CreateGr\" action=\"CreateGr\" />\n"
"      <separator name=\"sepMenuComputeConformerTypes\" />\n"
"      <menuitem name=\"ComputeConformerTypes\" action=\"ComputeConformerTypes\" />\n"
"      <separator name=\"sepMenuComputeRMSD\" />\n"
"      <menuitem name=\"ComputeRMSD\" action=\"ComputeRMSD\" />\n"
"      <separator name=\"sepMenuDelete\" />\n"
"      <menuitem name=\"DeleteGeometry\" action=\"DeleteGeometry\" />\n"
"      <menuitem name=\"DeleteHalfGeometries\" action=\"DeleteHalfGeometries\" />\n"
"      <menuitem name=\"DeleteBeforeSelectedGeometry\" action=\"DeleteBeforeSelectedGeometry\" />\n"
"      <menuitem name=\"DeleteAfterSelectedGeometry\" action=\"DeleteAfterSelectedGeometry\" />\n"
"      <separator name=\"sepMenuClose\" />\n"
"      <menuitem name=\"Close\" action=\"Close\" />\n"
"    </menu>\n"
"      <menu name=\"Help\" action=\"Help\">\n"
"         <menuitem name=\"HelpSupportedFormat\" action=\"HelpSupportedFormat\" />\n"
"      <separator name=\"sepMenuHelpAnimation\" />\n"
"         <menuitem name=\"HelpAnimation\" action=\"HelpAnimation\" />\n"
"      </menu>\n"
"  </menubar>\n"
;
/*******************************************************************************************************************************/
static void add_widget (GtkUIManager *manager, GtkWidget   *widget, GtkContainer *container)
{
	GtkWidget *handlebox;

	handlebox =gtk_handle_box_new ();
	g_object_ref (handlebox);
  	gtk_handle_box_set_handle_position  (GTK_HANDLE_BOX(handlebox),GTK_POS_LEFT);
	/*   GTK_SHADOW_NONE,  GTK_SHADOW_IN,  GTK_SHADOW_OUT, GTK_SHADOW_ETCHED_IN, GTK_SHADOW_ETCHED_OUT */
	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handlebox),GTK_SHADOW_OUT);
	gtk_box_pack_start (GTK_BOX (container), handlebox, TRUE, TRUE, 0);

	gtk_widget_show (widget);
	gtk_container_add (GTK_CONTAINER (handlebox), widget);
	gtk_widget_show (handlebox);
}
/*******************************************************************************************************************************/
static GtkUIManager *create_menu(GtkWidget* box)
{
	GtkActionGroup *actionGroup = NULL;
	GtkUIManager *manager = NULL;
	GError *error = NULL;

  	manager = gtk_ui_manager_new ();
  	g_signal_connect_swapped (PrincipalWindow, "destroy", G_CALLBACK (g_object_unref), manager);

	actionGroup = gtk_action_group_new ("GabeditAnimationGeomConvActions");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntries, numberOfGtkActionEntries, NULL);

  	gtk_ui_manager_insert_action_group (manager, actionGroup, 0);

	g_signal_connect (manager, "add_widget", G_CALLBACK (add_widget), box);
  	gtk_window_add_accel_group (GTK_WINDOW (PrincipalWindow), gtk_ui_manager_get_accel_group (manager));
	if (!gtk_ui_manager_add_ui_from_string (manager, uiMenuInfo, -1, &error))
	{
		g_message (_("building menus failed: %s"), error->message);
		g_error_free (error);
	}
	if(GTK_IS_UI_MANAGER(manager))
	{
		GtkAction     *action = NULL;
		action = gtk_ui_manager_get_action(manager, "/MenuBar/File");
		if(G_IS_OBJECT(action)) g_object_set_data(G_OBJECT(action), "Manager", manager);
	}
	return manager;
}
/********************************************************************************/
void geometriesMDDlg()
{
	GtkWidget *Win;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *parentWindow = PrincipalWindow;
	GtkUIManager *manager = NULL;

	if(WinDlg) return;

	initGeometryMD();

	Win= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_default_size (GTK_WINDOW(Win),-1,(gint)(ScreenHeightD*0.69));
	gtk_window_set_title(GTK_WINDOW(Win),"Molecular dynamic trajectory");
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	WinDlg = Win;

  	add_child(PrincipalWindow,Win,destroyAnimGeomConvDlg,"M.D.");
  	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (Win), vbox);

	hbox = create_hbox_false(vbox);
	manager = create_menu(hbox);

	hbox = create_hbox_false(vbox);
	gtk_widget_realize(Win);

	treeView = addList(hbox, manager);
	addEntriesButtons(vbox);
	gtk_widget_show_all(vbox);

	gtk_widget_show_now(Win);

	/* fit_windows_position(PrincipalWindow, Win);*/

  	rafreshList();
	stopAnimation(NULL, NULL);
}
