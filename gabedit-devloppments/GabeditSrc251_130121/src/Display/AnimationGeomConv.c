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
#include "GlobalOrb.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Display/StatusOrb.h"
#include "../Display/GLArea.h"
#include "../Display/AnimationGeomConv.h"
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

/* extern nimationGeomConv.h */
GeometryConvergence geometryConvergence;
/****************************************/

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
static void rafreshList();
static void stopAnimation(GtkWidget *win, gpointer data);
static void playAnimation(GtkWidget *win, gpointer data);
static gboolean set_geometry(gint k);
static GtkWidget* addComboListToATable(GtkWidget* table, gchar** list, gint nlist, gint i, gint j, gint k);
static void  add_cancel_ok_button(GtkWidget *Win,GtkWidget *vbox,GtkWidget *entry, GCallback myFunc);

/************************************************************************************************************/
static void setComboMethod(GtkWidget *comboMethod)
{
	GList *glist = NULL;

  	glist = g_list_append(glist,"Number");
  	glist = g_list_append(glist,"Symbol");
  	glist = g_list_append(glist,"MM Type");
  	glist = g_list_append(glist,"PDB Type");
  	glist = g_list_append(glist,"All");

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

	if(entryText && strstr(entryText,"All")&&geometryConvergence.numberOfGeometries>0)
	{
		gint i;
                gint nAtoms = geometryConvergence.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = nAtoms;
  		for(i=0;i<nAtoms;i++) 
			list[i] = g_strdup_printf("%d",i+1);
  		for(i=0;i<n;i++) 
			glist = g_list_append(glist,list[i]);
	}
	if(entryText && strstr(entryText,"Number")&&geometryConvergence.numberOfGeometries>0)
	{
		gint i;
                gint nAtoms = geometryConvergence.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = nAtoms;
  		for(i=0;i<nAtoms;i++) 
			list[i] = g_strdup_printf("%d",i+1);
  		for(i=0;i<n;i++) 
			glist = g_list_append(glist,list[i]);
	}
	if(entryText && strstr(entryText,"Symbol")&&geometryConvergence.numberOfGeometries>0)
	{
		gint i,j;
                gint nAtoms = geometryConvergence.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = 0;
  		for(i=0;i<nAtoms;i++) 
		{
			gchar* t = geometryConvergence.geometries[0].listOfAtoms[i].symbol;
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
	if(entryText && strstr(entryText,"MM Type")&&geometryConvergence.numberOfGeometries>0)
	{
		gint i,j;
                gint nAtoms = geometryConvergence.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = 0;
  		for(i=0;i<nAtoms;i++) 
		{
			gchar* t = geometryConvergence.geometries[0].listOfAtoms[i].mmType;
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
	if(entryText && strstr(entryText,"PDB Type")&&geometryConvergence.numberOfGeometries>0)
	{
		gint i,j;
                gint nAtoms = geometryConvergence.geometries[0].numberOfAtoms;
		list = g_malloc(nAtoms*sizeof(gchar*));
		n = 0;
  		for(i=0;i<nAtoms;i++) 
		{
			gchar* t = geometryConvergence.geometries[0].listOfAtoms[i].pdbType;
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

	if(geometryConvergence.numberOfGeometries<1) return;

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
	if(strstr(str,"All"))
	{
		double ff = 1.0/(4*M_PI*dr)*(maxr*maxr*maxr)/geometryConvergence.numberOfGeometries;
		double L=maxr/2*ANG_TO_BOHR;
		for(g = 0;g<geometryConvergence.numberOfGeometries;g++)
		{
			int n=geometryConvergence.geometries[g].numberOfAtoms;
			double fact = ff/n/n;
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
					if(xx>L) xx -= L;
					if(xx<-L) xx += L;
					d += xx*xx;
				}
				d = sqrt(d)*BOHR_TO_ANG;
				if(d>maxr) continue;
				Y[(gint)(d/dr)]+=fact;
			}
		}
		for(i=0;i<N;i++)
		{
			double d=(i+0.5)*dr;
                	//val = hist[i]/ norm / ((rrr * rrr) + (dr * dr) / 12.0)
			Y[i] /= d*d;
		}
	}
	else if(strstr(str,"Number"))
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
		for(g = 0;g<geometryConvergence.numberOfGeometries;g++)
		{
			gint a = n1;
			gint b = n2;
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

		for(g = 0;g<geometryConvergence.numberOfGeometries;g++)
		{
			gint a;
			gint b;
			for(a=0;a<geometryConvergence.geometries[g].numberOfAtoms;a++)
			for(b=0;b<a;b++)
			{
				if(
				(!strcmp(geometryConvergence.geometries[g].listOfAtoms[a].symbol,s1) && !strcmp(geometryConvergence.geometries[g].listOfAtoms[b].symbol,s2))
				||
				(!strcmp(geometryConvergence.geometries[g].listOfAtoms[a].symbol,s2) && !strcmp(geometryConvergence.geometries[g].listOfAtoms[b].symbol,s1))
				)
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
					Y[(gint)(d/dr)]++;
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

		for(g = 0;g<geometryConvergence.numberOfGeometries;g++)
		{
			gint a;
			gint b;
			for(a=0;a<geometryConvergence.geometries[g].numberOfAtoms;a++)
			for(b=0;b<a;b++)
			{
				if(
				(!strcmp(geometryConvergence.geometries[g].listOfAtoms[a].mmType,s1) && !strcmp(geometryConvergence.geometries[g].listOfAtoms[b].mmType,s2))
				||
				(!strcmp(geometryConvergence.geometries[g].listOfAtoms[a].mmType,s2) && !strcmp(geometryConvergence.geometries[g].listOfAtoms[b].mmType,s1))
				)
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

		for(g = 0;g<geometryConvergence.numberOfGeometries;g++)
		{
			gint a;
			gint b;
			for(a=0;a<geometryConvergence.geometries[g].numberOfAtoms;a++)
			for(b=0;b<a;b++)
			{
				if(
				(!strcmp(geometryConvergence.geometries[g].listOfAtoms[a].pdbType,s1) && !strcmp(geometryConvergence.geometries[g].listOfAtoms[b].pdbType,s2))
				||
				(!strcmp(geometryConvergence.geometries[g].listOfAtoms[a].pdbType,s2) && !strcmp(geometryConvergence.geometries[g].listOfAtoms[b].pdbType,s1))
				)
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
	gtk_window_set_title(GTK_WINDOW(Win),"compute radial distribution");
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

	for(a=0;a<geometryConvergence.geometries[g].numberOfAtoms;a++)
	{
  		SAtomsProp Prop = prop_atom_get(geometryConvergence.geometries[g].listOfAtoms[a].symbol);
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
	gchar layer[20] = " ";

	if(geometryConvergence.numberOfGeometries<1) return;
	if(entry) supstr = gtk_entry_get_text(GTK_ENTRY(entry));
	if(!supstr) return;
	fileName = g_strdup_printf("%s%sgabmg.com",inputGaussDirectory,G_DIR_SEPARATOR_S);
 	file = fopen(fileName, "w");
	if(!file)
	{
		gchar* t = g_strdup_printf(_("Sorry\n I can not create %s file"),fileName); 
		Message(t,_("Error"),TRUE);
		if(fileName) g_free(fileName);
		if(t)g_free(t);
		return;
	}
	for(g = 0;g<geometryConvergence.numberOfGeometries;g++)
	{
		gint ne = 0;
		if(g !=0) fprintf(file,"--Link1--\n");
		if(GTK_IS_WIDGET(buttonChkgauss)&& GTK_TOGGLE_BUTTON (buttonChkgauss)->active)
			fprintf(file,"%cChk=gabmg\n",p);

		fprintf(file,"# %s\n",supstr);

		fprintf(file,"# Test\n");
		fprintf(file,"# Units(Ang,Deg)\n");
		/* if(g!=0) fprintf(file,"# Guess=read\n");*/
		fprintf(file,"\n File generated by Gabedit(Multiple Geometries)\n\n");
		for(a=0;a<geometryConvergence.geometries[g].numberOfAtoms;a++)
		{
  			SAtomsProp Prop = prop_atom_get(geometryConvergence.geometries[g].listOfAtoms[a].symbol);
			ne += Prop.atomicNumber;
		}
		if( mystrcasestr(supstr,"ONIOM"))
			fprintf(file,"%d   %d   %d    %d    %d    %d\n",
					totalCharge,spinMultiplicity,
					totalCharge,spinMultiplicity,
					totalCharge,spinMultiplicity
				);
		else
			fprintf(file,"%d   %d\n",totalCharge,spinMultiplicity);

		for(a=0;a<geometryConvergence.geometries[g].numberOfAtoms;a++)
		{
			if( mystrcasestr(supstr,"ONIOM"))
			{
				sprintf(layer," ");
				if(!strcmp(geometryConvergence.geometries[g].listOfAtoms[a].mmType,"HW")) sprintf(layer,"Low");
				if(!strcmp(geometryConvergence.geometries[g].listOfAtoms[a].mmType,"OW")) sprintf(layer,"Low");
				fprintf(file,"%s-%s-%0.8f 0 %0.8lf %0.8lf %0.8lf   %s\n",
				geometryConvergence.geometries[g].listOfAtoms[a].symbol,
				geometryConvergence.geometries[g].listOfAtoms[a].mmType,
				geometryConvergence.geometries[g].listOfAtoms[a].partialCharge,
				geometryConvergence.geometries[g].listOfAtoms[a].C[0]*BOHR_TO_ANG,
				geometryConvergence.geometries[g].listOfAtoms[a].C[1]*BOHR_TO_ANG,
				geometryConvergence.geometries[g].listOfAtoms[a].C[2]*BOHR_TO_ANG,
				layer
				);
			}
			else
				fprintf(file,"%s %0.8lf %0.8lf %0.8lf\n",geometryConvergence.geometries[g].listOfAtoms[a].symbol,
				geometryConvergence.geometries[g].listOfAtoms[a].C[0]*BOHR_TO_ANG,
				geometryConvergence.geometries[g].listOfAtoms[a].C[1]*BOHR_TO_ANG,
				geometryConvergence.geometries[g].listOfAtoms[a].C[2]*BOHR_TO_ANG
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
static void print_gaussian_script_run_bat()
{
	gchar* fileName = NULL;
	FILE* file;
	gchar p = '%';
	gint i;

	if(geometryConvergence.numberOfGeometries<1) return;
	fileName = g_strdup_printf("%s%srungabmg.bat",inputGaussDirectory,G_DIR_SEPARATOR_S);
 	file = fopen(fileName, "w");
	if(!file)
	{
		if(fileName) g_free(fileName);
		return;
	}
	fprintf(file,"@set runGauss=g03\n");
	for(i=0;i<geometryConvergence.numberOfGeometries;i++)
	{
		fprintf(file,"%crunGauss%c gabmg_%d.com\n",p,p,i);
	}
	fprintf(file,"\n");
	fclose(file);
}
/*************************************************************************************************************/
static void print_gaussian_script_run()
{
	gchar* fileName = NULL;
	FILE* file;
	gchar p = '%';

	if(geometryConvergence.numberOfGeometries<1) return;
	fileName = g_strdup_printf("%s%srungabmg.sh",inputGaussDirectory,G_DIR_SEPARATOR_S);
 	file = fopen(fileName, "w");
	if(!file)
	{
		if(fileName) g_free(fileName);
		return;
	}
	fprintf(file,"#!/bin/sh\n");
	fprintf(file,"#############################################\n");
	fprintf(file,"# script for run gaussian with gabmg_ files #\n");
	fprintf(file,"#    this script is created by Gabedit      #\n");
	fprintf(file,"#############################################\n");
	fprintf(file,"\n");
	fprintf(file,"runGauss=g03\n");
	fprintf(file,"echo run gabmg_0.com ....\n");
	fprintf(file,"$runGauss gabmg_0.com\n");
	fprintf(file,"for filecom in gabmg_*.com\n");
	fprintf(file,"do\n");
	fprintf(file,"   filename=${filecom%c.com}\n",p);
	fprintf(file,"   if test \"$filename\" != \"gabmg_0\"\n");
	fprintf(file,"   then\n");
	fprintf(file,"      echo copy gabmg_0.chk in $filename.chk ....\n");
	fprintf(file,"      cp gabmg_0.chk $filename.chk\n");
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
	gint ne = 0;
	gchar layer[20]=" ";

	if(g>=geometryConvergence.numberOfGeometries || geometryConvergence.numberOfGeometries<1) return;
	fileName = g_strdup_printf("%s%sgabmg_%d.com",inputGaussDirectory,G_DIR_SEPARATOR_S,g);
 	file = fopen(fileName, "w");
	if(!file)
	{
		if(fileName) g_free(fileName);
		return;
	}
	if(GTK_IS_WIDGET(buttonChkgauss)&& GTK_TOGGLE_BUTTON (buttonChkgauss)->active)
		fprintf(file,"%cChk=gabmg_%d\n",p,g);

	fprintf(file,"# %s\n",supstr);


	fprintf(file,"# Test\n");
	fprintf(file,"# Units(Ang,Deg)\n");
	/* if(g!=0) fprintf(file,"# Guess=read\n");*/
	fprintf(file,"\n File generated by Gabedit(Multiple Geometries)\n\n");
	for(a=0;a<geometryConvergence.geometries[g].numberOfAtoms;a++)
	{
  		SAtomsProp Prop = prop_atom_get(geometryConvergence.geometries[g].listOfAtoms[a].symbol);
		ne += Prop.atomicNumber;
	}
	if( mystrcasestr(supstr,"ONIOM"))
		fprintf(file,"%d   %d   %d    %d    %d    %d\n",
				totalCharge,spinMultiplicity,
				totalCharge,spinMultiplicity,
				totalCharge,spinMultiplicity
			);
	else
		fprintf(file,"%d   %d\n",totalCharge,spinMultiplicity);

	for(a=0;a<geometryConvergence.geometries[g].numberOfAtoms;a++)
	{
		if( mystrcasestr(supstr,"ONIOM"))
		{
			sprintf(layer," ");
			if(!strcmp(geometryConvergence.geometries[g].listOfAtoms[a].mmType,"HW")) sprintf(layer,"Low");
			if(!strcmp(geometryConvergence.geometries[g].listOfAtoms[a].mmType,"OW")) sprintf(layer,"Low");
			fprintf(file,"%s-%s-%0.8f 0 %0.8lf %0.8lf %0.8lf   %s\n",
			geometryConvergence.geometries[g].listOfAtoms[a].symbol,
			geometryConvergence.geometries[g].listOfAtoms[a].mmType,
			geometryConvergence.geometries[g].listOfAtoms[a].partialCharge,
			geometryConvergence.geometries[g].listOfAtoms[a].C[0]*BOHR_TO_ANG,
			geometryConvergence.geometries[g].listOfAtoms[a].C[1]*BOHR_TO_ANG,
			geometryConvergence.geometries[g].listOfAtoms[a].C[2]*BOHR_TO_ANG,
			layer
			);
		}
		else
			fprintf(file,"%s %0.8lf %0.8lf %0.8lf\n",geometryConvergence.geometries[g].listOfAtoms[a].symbol,
				geometryConvergence.geometries[g].listOfAtoms[a].C[0]*BOHR_TO_ANG,
				geometryConvergence.geometries[g].listOfAtoms[a].C[1]*BOHR_TO_ANG,
				geometryConvergence.geometries[g].listOfAtoms[a].C[2]*BOHR_TO_ANG
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
				"In %s directory, the gabmg_*, rungabmg.sh and \nrungabmg.bat files were created.\n"
				"\ngabmg_* is input files for gaussian. \n"
				"rungabmg.bat is a batch file for run gaussian with gabmg_* input files\n"
				"rungabmg.sh is a script for run gaussian with gabmg_* input files"
				), inputGaussDirectory); 
		for(g = 0;g<geometryConvergence.numberOfGeometries;g++)
			print_gaussian_one_geometry(g,supstr);
		print_gaussian_script_run();
		print_gaussian_script_run_bat();
		Message(t,_("Error"),TRUE);
		if(t)g_free(t);
	}
	gtk_widget_destroy(Win);
}
/*************************************************************************************************************/
static gboolean print_mobcal_geometries(gchar* fileName)
{
	gint a;
	gchar* fileNameData = NULL;
	FILE* file;
	gint g;

	if(geometryConvergence.numberOfGeometries<1) return FALSE;
	fileNameData = g_strdup_printf("%s.mfj",get_suffix_name_file(fileName));
 	file = fopen(fileNameData, "w");
	if(!file)
	{
		gchar* t = g_strdup_printf(_("Sorry\n I can not create %s file"),fileNameData); 
		Message(t,_("Error"),TRUE);
		if(fileNameData) g_free(fileNameData);
		if(t)g_free(t);
		return FALSE;
	}
	if(fileNameData) g_free(fileNameData);
	fprintf(file,"Made in Gabedit\n");
	fprintf(file,"%d\n",geometryConvergence.numberOfGeometries);
	fprintf(file,"%d\n",geometryConvergence.geometries[0].numberOfAtoms);
	fprintf(file,"ang\n");
	fprintf(file,"calc\n");
	fprintf(file,"1.0\n");


	for(g = 0;g<geometryConvergence.numberOfGeometries;g++)
	{
		for(a=0;a<geometryConvergence.geometries[g].numberOfAtoms;a++)
		{
  			SAtomsProp Prop = prop_atom_get(geometryConvergence.geometries[g].listOfAtoms[a].symbol);
			fprintf(file,"%13.5f %13.5f %13.5f %4d %13.5f\n",
			geometryConvergence.geometries[g].listOfAtoms[a].C[0]*BOHR_TO_ANG,
			geometryConvergence.geometries[g].listOfAtoms[a].C[1]*BOHR_TO_ANG,
			geometryConvergence.geometries[g].listOfAtoms[a].C[2]*BOHR_TO_ANG,
			(gint)(Prop.masse+0.5),
			geometryConvergence.geometries[g].listOfAtoms[a].partialCharge
			);
		}
		fprintf(file," %d\n",geometryConvergence.geometries[g].numberOfAtoms);
	}
	fclose(file);
	return TRUE;
}
/*************************************************************************************************************/
static void print_mobcal_run(gchar* fileName)
{
	gchar* suff = get_suffix_name_file(fileName);
	gchar* fileNameData = g_strdup_printf("%s.mfj",get_name_file(suff));
	gchar* fileNameOut = g_strdup_printf("%s.out",get_name_file(suff));
	gchar* dirName = get_name_dir(fileName);
	gchar* newFile = g_strdup_printf("%s%smobcal.run",dirName,G_DIR_SEPARATOR_S);
	FILE* file;
 	file = fopen(newFile, "w");
	if(!file)
	{
		gchar* t = g_strdup_printf(_("Sorry\n I can not create %s file"),newFile); 
		Message(t,_("Error"),TRUE);
		if(t)g_free(t);
		if(suff) g_free(suff);
		if(fileNameData) g_free(fileNameData);
		if(fileNameOut) g_free(fileNameOut);
		if(dirName) g_free(dirName);
		if(newFile) g_free(newFile);
		return;
	}
	fprintf(file,"%s\n",fileNameData);
	fprintf(file,"%s\n",fileNameOut);
	fprintf(file,"5013486\n");
	fclose(file);
}
/*************************************************************************************************************/
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
	dirSelector = selectionOfDir(reset_last_directory, _("Set folder"), GABEDIT_TYPEWIN_ORB);
	gtk_window_set_modal (GTK_WINDOW (dirSelector), TRUE);
	gtk_window_set_transient_for(GTK_WINDOW(dirSelector),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_transient_for(GTK_WINDOW(dirSelector),GTK_WINDOW(WinDlg));
}
/********************************************************************************/
void initGeometryConvergence()
{
	geometryConvergence.fileName = NULL;
	geometryConvergence.typeOfFile = GABEDIT_TYPEFILE_UNKNOWN;
	geometryConvergence.numberOfGeometries = 0;
	geometryConvergence.numGeometry = NULL;
	geometryConvergence.velocity = 0.1;
	geometryConvergence.energy = NULL;
	geometryConvergence.maxForce = NULL;
	geometryConvergence.rmsForce= NULL;
	geometryConvergence.maxStep = NULL;
	geometryConvergence.rmsStep = NULL;
	geometryConvergence.geometries = NULL;
	geometryConvergence.comments = NULL;
	rowSelected = -1;
}
/********************************************************************************/
void freeGeometryConvergence()
{
	static gboolean begin = TRUE;

	if(begin)
	{
		begin  = FALSE;
		initGeometryConvergence();
		return;
	}
	if(geometryConvergence.fileName) g_free(geometryConvergence.fileName);
	if(geometryConvergence.numGeometry) g_free(geometryConvergence.numGeometry);
	if(geometryConvergence.energy) g_free(geometryConvergence.energy);
	if(geometryConvergence.maxForce) g_free(geometryConvergence.maxForce);
	if(geometryConvergence.rmsForce) g_free(geometryConvergence.rmsForce);
	if(geometryConvergence.maxStep) g_free(geometryConvergence.maxStep);
	if(geometryConvergence.rmsStep) g_free(geometryConvergence.rmsStep);
	if(geometryConvergence.geometries)
	{
		gint i;
		Geometry* geometries = geometryConvergence.geometries;
		for(i=0;i<geometryConvergence.numberOfGeometries;i++)
		{
			if(geometries[i].listOfAtoms) g_free(geometries[i].listOfAtoms);
		}
		g_free(geometries);
	}
	if(geometryConvergence.comments)
	{
		gint i;
		for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			if(geometryConvergence.comments[i]) g_free(geometryConvergence.comments[i]);
		g_free(geometryConvergence.comments);
	}

	initGeometryConvergence();
}
/********************************************************************************/
static void delete_one_geometry()
{

	gint j;
	gint k = rowSelected;

	if(k<0 || k >= geometryConvergence.numberOfGeometries) return;
	if(!geometryConvergence.geometries) return;
	if(geometryConvergence.numberOfGeometries==1)
	{
		freeGeometryConvergence();
		rafreshList();
		return;
	}

	if(geometryConvergence.geometries)
			if(geometryConvergence.geometries[k].listOfAtoms) g_free(geometryConvergence.geometries[k].listOfAtoms);
	for(j=k;j<geometryConvergence.numberOfGeometries-1;j++)
	{
		if(geometryConvergence.energy) geometryConvergence.energy[j] = geometryConvergence.energy[j+1];
		if(geometryConvergence.maxForce) geometryConvergence.maxForce[j] = geometryConvergence.maxForce[j+1];
		if(geometryConvergence.rmsForce) geometryConvergence.rmsForce[j] = geometryConvergence.rmsForce[j+1];
		if(geometryConvergence.maxStep) geometryConvergence.maxStep[j] = geometryConvergence.maxStep[j+1];
		if(geometryConvergence.rmsStep) geometryConvergence.rmsStep[j] = geometryConvergence.rmsStep[j+1];
		if(geometryConvergence.numGeometry) geometryConvergence.numGeometry[j] = geometryConvergence.numGeometry[j+1];
		if(geometryConvergence.comments) geometryConvergence.comments[j] = geometryConvergence.comments[j+1];
		if(geometryConvergence.geometries)
		{
			geometryConvergence.geometries[j] = geometryConvergence.geometries[j+1];
		}
	}
	geometryConvergence.numberOfGeometries--;
	if(geometryConvergence.energy)
		geometryConvergence.energy = g_realloc(geometryConvergence.energy, geometryConvergence.numberOfGeometries*sizeof(gdouble));
	if(geometryConvergence.maxForce)
		geometryConvergence.maxForce = g_realloc(geometryConvergence.maxForce, geometryConvergence.numberOfGeometries*sizeof(gdouble));
	if(geometryConvergence.rmsForce)
		geometryConvergence.rmsForce = g_realloc(geometryConvergence.rmsForce, geometryConvergence.numberOfGeometries*sizeof(gdouble));
	if(geometryConvergence.maxStep)
		geometryConvergence.maxStep = g_realloc(geometryConvergence.maxStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));
	if(geometryConvergence.rmsStep)
		geometryConvergence.rmsStep = g_realloc(geometryConvergence.rmsStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));
	if(geometryConvergence.numGeometry)
		geometryConvergence.numGeometry = g_realloc(geometryConvergence.numGeometry, geometryConvergence.numberOfGeometries*sizeof(gint));
	if(geometryConvergence.geometries)
	{
		geometryConvergence.geometries = g_realloc(geometryConvergence.geometries, geometryConvergence.numberOfGeometries*sizeof(Geometry));
	}
	rafreshList();

	return;
}
/*************************************************************************************************************/
static gboolean read_molden_gabedit_file_geomi(gchar *fileName, gint geometryNumber, GabEditTypeFile type, Geometry* geometry)
{
	gchar* t;
	gboolean OK;
	gchar* AtomCoord[5];
	FILE *file;
	gint i;
	gint j;
	gint l;
	gint numgeom;
	gchar *pdest;
	gint nn;
    	Atom* listOfAtoms = NULL;

	file = FOpen(fileName, "rb");

	if(file ==NULL)
	{
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),fileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return FALSE;
	}
	t = g_malloc(BSIZE*sizeof(gchar));
	for(i=0;i<5;i++) AtomCoord[i] = g_malloc(BSIZE*sizeof(gchar));

	numgeom =0;
	OK=TRUE;
	while(!feof(file))
	{
		if(!fgets(t, BSIZE,file))break;
		pdest = strstr( t, "[GEOMETRIES]");
		if(pdest && strstr(t,"ZMAT"))
		{
			if(type == GABEDIT_TYPEFILE_MOLDEN)
				sprintf(t,_("Sorry\nMolden file with ZMAT coordinate is not supported by Gabedit"));
			if(type == GABEDIT_TYPEFILE_GABEDIT)
				sprintf(t,_("Sorry\nGabedit file with ZMAT coordinate is not supported by Gabedit"));

			Message(t,_("Error"),TRUE);
			g_free(t);
			return FALSE;
		}	
 		if (pdest)
		{
			while(!feof(file))
			{
				if(!fgets(t, BSIZE,file))break;

				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
				if(!isInteger(t))break;
               			numgeom++;
				if(numgeom == geometryNumber)
				{
					nn = atoi(t);
					if(nn<1)break;
    					listOfAtoms = g_malloc(nn*sizeof(Atom));
					if(!fgets(t, BSIZE,file))break; /* title */
					for(j=0; j<nn; j++)
					{
						if(!fgets(t, BSIZE,file))break;
    						sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
						{
							gint i;
							for(i=0;i<(gint)strlen(AtomCoord[0]);i++) if(isdigit(AtomCoord[0][i])) AtomCoord[0][i] = ' ';
							delete_all_spaces(AtomCoord[0]);
						}
						AtomCoord[0][0]=toupper(AtomCoord[0][0]);
						l=strlen(AtomCoord[0]);
						if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    						sprintf(listOfAtoms[j].symbol,"%s",AtomCoord[0]);
    						sprintf(listOfAtoms[j].mmType,"%s",AtomCoord[0]);
    						sprintf(listOfAtoms[j].pdbType,"%s",AtomCoord[0]);
    						for(i=0;i<3;i++) listOfAtoms[j].C[i]=atof(AtomCoord[i+1])*ANG_TO_BOHR;
						listOfAtoms[j].partialCharge = 0.0;
						listOfAtoms[j].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[j].symbol);
						listOfAtoms[j].variable = 0;
					}
 					geometry->numberOfAtoms = nn;
 					geometry->listOfAtoms = listOfAtoms;
					OK = TRUE;
					break;
				}
				else
				{
					nn = atoi(t);
					if(!fgets(t, BSIZE,file)) break;
					for(i=0; i<nn; i++)
						if(!fgets(t, BSIZE,file))break;
					if(i!=nn) { OK = FALSE; break;}
				}
				if(!OK) break;
			}
		}
		if(!OK) break;
	}

	fclose(file);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(geometry->numberOfAtoms == 0 )
	{
		if(geometry->listOfAtoms) g_free(geometry->listOfAtoms);
	}
	return TRUE;
}
/********************************************************************************/
static gboolean read_dalton_file_geomi(gchar *FileName, gint num, Geometry* geometry)
{
 	gchar *t;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *file;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
	Atom* listOfAtoms = NULL;
	gchar dum[100];
	gint kk;

  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return FALSE ;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
 	}
 	t=g_malloc(BSIZE);
 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(BSIZE*sizeof(char));

 	numgeom =1;
 	do 
 	{
 		OK=FALSE;
 		while(!feof(file))
		{
			{char* e = fgets(t,BSIZE,file);}
			/* if (strstr(t,"Next geometry"))*/
			if(strstr(t,"Next geometry") || strstr(t,"Final geometry"))
			{
				{char* e = fgets(t,BSIZE,file);}
				{char* e = fgets(t,BSIZE,file);}
 				numgeom++;
                		OK = TRUE;
	  			break;
	  		}
        	}
 		if(!OK && (numgeom == 1) )
		{
  			Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
 			fclose(file);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			return FALSE;
    		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(file) )
  		{
			{char* e = fgets(t,BSIZE,file);}
			if (!strcmp(t,"\n"))
			{
				break;
			}
    			j++;
    			if(listOfAtoms == NULL) listOfAtoms = g_malloc(sizeof(Atom));
    			else listOfAtoms = g_realloc(listOfAtoms, (j+1)*sizeof(Atom));

			kk = sscanf(t,"%s %s %s %s %s",AtomCoord[0],AtomCoord[1], AtomCoord[2],AtomCoord[3], dum);
			if(kk==5) sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);

			for(i=0;i<(gint)strlen(AtomCoord[0]);i++) if(isdigit(AtomCoord[0][i])) AtomCoord[0][i] = ' ';
			delete_all_spaces(AtomCoord[0]);
			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);


    			sprintf(listOfAtoms[j].symbol,"%s",AtomCoord[0]);
    			sprintf(listOfAtoms[j].mmType,"%s",AtomCoord[0]);
    			sprintf(listOfAtoms[j].pdbType,"%s",AtomCoord[0]);
    			for(i=0;i<3;i++) listOfAtoms[j].C[i]=atof((AtomCoord[i+1]));
			listOfAtoms[j].partialCharge = 0.0;
			listOfAtoms[j].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[j].symbol);
			listOfAtoms[j].variable = 0;
  		}
		if(num >0 && (gint)numgeom-1 == num) break;
			
 	}while(!feof(file));

 	fclose(file);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(j+1 == 0 && listOfAtoms )
	{
		g_free(listOfAtoms);
	}
 	else
	{
		geometry->numberOfAtoms = j+1;
		geometry->listOfAtoms = listOfAtoms;
	}
	return TRUE;
}
/********************************************************************************/
static gboolean read_gamess_file_geomi(gchar *FileName, gint num, Geometry* geometry)
{
 	gchar *t;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *file;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
	Atom* listOfAtoms = NULL;
	gchar dum[100];

  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return FALSE ;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
 	}
 	t=g_malloc(BSIZE*sizeof(gchar));
 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(BSIZE*sizeof(char));
 	numgeom =1;
 	do 
 	{
 		OK=FALSE;
 		while(!feof(file))
		{
			{char* e = fgets(t,BSIZE,file);}
			if(strstr(t,"COORDINATES OF ALL ATOMS ARE (ANGS)"))
			{
				{char* e = fgets(t,BSIZE,file);}
				{char* e = fgets(t,BSIZE,file);}
 				numgeom++;
                		OK = TRUE;
	  			break;
	  		}
        	}
 		if(!OK && (numgeom == 1) )
		{
  			Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
 			fclose(file);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			return FALSE;
    		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(file) )
  		{
			{char* e = fgets(t,BSIZE,file);}
			if (this_is_a_backspace(t))
			{
				break;
			}
    			j++;
    			if(listOfAtoms == NULL) listOfAtoms = g_malloc(sizeof(Atom));
    			else listOfAtoms = g_realloc(listOfAtoms, (j+1)*sizeof(Atom));

			/* printf("t=%s\n",t);*/
			sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);

			for(i=0;i<(gint)strlen(AtomCoord[0]);i++) if(isdigit(AtomCoord[0][i])) AtomCoord[0][i] = ' ';
			delete_all_spaces(AtomCoord[0]);
			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);


			sprintf(AtomCoord[0],"%s",get_symbol_using_z(atoi(dum)));
    			sprintf(listOfAtoms[j].symbol,"%s",AtomCoord[0]);
    			sprintf(listOfAtoms[j].mmType,"%s",AtomCoord[0]);
    			sprintf(listOfAtoms[j].pdbType,"%s",AtomCoord[0]);
    			for(i=0;i<3;i++) listOfAtoms[j].C[i]=atof((AtomCoord[i+1]))*ANG_TO_BOHR;
			listOfAtoms[j].partialCharge = 0.0;
			listOfAtoms[j].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[j].symbol);
			listOfAtoms[j].variable = 0;
  		}
		if(num >0 && (gint)numgeom-1 == num) break;
			
 	}while(!feof(file));

 	fclose(file);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(j+1 == 0 && listOfAtoms )
	{
		g_free(listOfAtoms);
	}
 	else
	{
		geometry->numberOfAtoms = j+1;
		geometry->listOfAtoms = listOfAtoms;
	}
	return TRUE;
}
/********************************************************************************/
static gboolean read_gamess_output(gchar* fileName)
{
	gchar *pdest;
	gint  k=0;
	gchar *maxGrad =  NULL;
	gchar *rmsGrad =  NULL;
	gchar *temp =  NULL;
	gchar *tmp =  NULL;
	gchar *t = NULL;
	FILE *file;
	gboolean OK;
        
	temp = get_name_file(fileName);
	set_status_label_info(_("File name"),temp);
	g_free(temp);
	set_status_label_info(_("File type"),"Gamess");

 	file = FOpen(fileName, "rb"); 
        if(!file)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),fileName);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return FALSE;
	}
	t=g_malloc(BSIZE*sizeof(gchar));
	maxGrad =  g_malloc(50*sizeof(gchar));	
	rmsGrad =  g_malloc(50*sizeof(gchar));	
	temp =  g_malloc(50*sizeof(gchar));	
	tmp =  g_malloc(50*sizeof(gchar));	
        
	OK = TRUE;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) break;
		/* pdest = strstr( t,"NSERCH=   ");*/
		pdest = strstr( t,"1NSERCH=");
		if( pdest == NULL) pdest = strstr( t,"POINT NSERCH=");
   		if( pdest != NULL )
		{
			OK = TRUE;
			while(!feof(file) && OK )
			{
		 		if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
				if(strstr(t,"COORDINATES OF ALL ATOMS ARE (ANGS)"))
				{
		 			if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
		 			if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;

			OK = TRUE;
			while(!feof(file) && OK )
			{
		 		if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
				if(strstr(t,"NSERCH") && strstr(t,"ENERGY="))
				{
		 			gchar* t1 = strstr(t,"ENERGY=");
					if(t1) sscanf(t1+7,"%s",tmp); /* energy */
					else { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
			OK = TRUE;
			while(!feof(file) && OK )
			{
		 		if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
				if(strstr(t,"MAXIMUM GRADIENT =")&& strstr(t,"RMS GRADIENT ="))
				{
		 			gchar* t1 = strstr(t,"MAXIMUM GRADIENT =");
					if(t1) sscanf(t1+19,"%s",maxGrad); /* maxGrad */
					else { OK = FALSE; break; }
		 			t1 = strstr(t,"RMS GRADIENT =");
					if(t1) sscanf(t1+15,"%s",rmsGrad); /* rmsGrad */
					else { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
		}
   		if(pdest != NULL)
		{
			geometryConvergence.numberOfGeometries++;
		  	if(geometryConvergence.numberOfGeometries == 1 )
		  	{
				geometryConvergence.typeOfFile = GABEDIT_TYPEFILE_GAMESS;
				geometryConvergence.fileName = g_strdup(fileName);
				geometryConvergence.numGeometry =  g_malloc(sizeof(gint));	
				geometryConvergence.numGeometry[0] =  1;
				geometryConvergence.energy =  g_malloc(sizeof(gdouble));	
				geometryConvergence.energy[0] = atof(tmp);
				geometryConvergence.maxStep =  g_malloc(sizeof(gdouble));	
				geometryConvergence.maxStep[0] = atof(maxGrad);
				geometryConvergence.rmsStep =  g_malloc(sizeof(gdouble));	
				geometryConvergence.rmsStep[0] = atof(rmsGrad);
		  	}
		  	else
		  	{
				geometryConvergence.numGeometry =  
				g_realloc(geometryConvergence.numGeometry,geometryConvergence.numberOfGeometries*sizeof(gint));	
				k = geometryConvergence.numberOfGeometries-1;
				geometryConvergence.numGeometry[k] =  k+1;
				geometryConvergence.energy =  
				g_realloc(geometryConvergence.energy,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.energy[k] = atof(tmp);

				geometryConvergence.maxStep =  g_realloc(geometryConvergence.maxStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.maxStep[k] = atof(maxGrad);
				geometryConvergence.rmsStep =  g_realloc(geometryConvergence.rmsStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.rmsStep[k] = atof(rmsGrad);
		  	}
			OK = TRUE;
		}
	}
	if(!OK)
	{
		freeGeometryConvergence();
		sprintf(t,_("Sorry\nI can not read energy or convergence parameters from %s file "),fileName);
		Message(t,_("Error"),TRUE);
		OK = FALSE;
	 }

	fclose(file);
	g_free(t);
	g_free(temp);
	g_free(maxGrad);
	g_free(rmsGrad);
	g_free(tmp);

	if(geometryConvergence.numberOfGeometries>0)
	{
		gint i;
		geometryConvergence.geometries = g_malloc(geometryConvergence.numberOfGeometries*sizeof(Geometry));
		for(i=0;i<geometryConvergence.numberOfGeometries;i++)
		{
			if(!read_gamess_file_geomi(fileName,geometryConvergence.numGeometry[i], &geometryConvergence.geometries[i])) break;
		}
		if(i!=geometryConvergence.numberOfGeometries)
		{
			freeGeometryConvergence();
			OK = FALSE;
		}
	}
  	rafreshList();
	return OK;
}
/********************************************************************************/
static gboolean read_gamess_irc_file_geomi(gchar *FileName, gint num, Geometry* geometry)
{
 	gchar *t;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *file;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom;
	Atom* listOfAtoms = NULL;
	gchar dum[100];

  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return FALSE ;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
 	}
 	t=g_malloc(BSIZE);
 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(BSIZE*sizeof(char));

 	numgeom =1;
 	do 
 	{
 		OK=FALSE;
 		while(!feof(file))
		{
			{char* e = fgets(t,BSIZE,file);}
			if(strstr(t,"CARTESIAN COORDINATES (BOHR)"))
			{
 				numgeom++;
                		OK = TRUE;
	  			break;
	  		}
        	}
 		if(!OK && (numgeom == 1) )
		{
  			Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
 			fclose(file);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			return FALSE;
    		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(file) )
  		{
			gdouble rdum = 0;
			if(!fgets(t,BSIZE,file))break;
			if (!strcmp(t,"\n")) break;
			if ( !strcmp(t,"GRADIENT")) break;
			if(2!=sscanf(t,"%s %lf",AtomCoord[0],&rdum)) break;
    			j++;
    			if(listOfAtoms == NULL) listOfAtoms = g_malloc(sizeof(Atom));
    			else listOfAtoms = g_realloc(listOfAtoms, (j+1)*sizeof(Atom));

			sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);

			for(i=0;i<(gint)strlen(AtomCoord[0]);i++) if(isdigit(AtomCoord[0][i])) AtomCoord[0][i] = ' ';
			delete_all_spaces(AtomCoord[0]);
			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);


    			sprintf(listOfAtoms[j].symbol,"%s",AtomCoord[0]);
    			sprintf(listOfAtoms[j].mmType,"%s",AtomCoord[0]);
    			sprintf(listOfAtoms[j].pdbType,"%s",AtomCoord[0]);
    			for(i=0;i<3;i++) listOfAtoms[j].C[i]=atof((AtomCoord[i+1]));
			listOfAtoms[j].partialCharge = 0.0;
			listOfAtoms[j].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[j].symbol);
			listOfAtoms[j].variable = 0;
  		}
		if(num >0 && (gint)numgeom-1 == num) break;
			
 	}while(!feof(file));

 	fclose(file);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(j+1 == 0 && listOfAtoms )
	{
		g_free(listOfAtoms);
	}
 	else
	{
		geometry->numberOfAtoms = j+1;
		geometry->listOfAtoms = listOfAtoms;
	}
	return TRUE;
}
/********************************************************************************/
static gboolean read_gamess_irc(gchar* fileName)
{
	gint  k=0;
	gchar *temp =  NULL;
	gchar *tmp =  NULL;
	gchar *t = NULL;
	FILE *file;
	gboolean OK;
        
	temp = get_name_file(fileName);
	set_status_label_info(_("File name"),temp);
	g_free(temp);
	set_status_label_info(_("File type"),"Gamess");

 	file = FOpen(fileName, "rb"); 
        if(!file)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),fileName);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return FALSE;
	}
	t=g_malloc(BSIZE*sizeof(gchar));
	temp =  g_malloc(50*sizeof(gchar));	
	tmp =  g_malloc(50*sizeof(gchar));	
        
	OK = TRUE;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) break;
   		if( strstr( t,"POINT=") && strstr( t,"E=") )
		{
			sscanf(strstr(t,"E=")+2,"%s",tmp); /* energy */
			OK = TRUE;
			while(!feof(file) && OK )
			{
		 		if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
				if(strstr(t,"CARTESIAN COORDINATES (BOHR)"))
				{
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
			geometryConvergence.numberOfGeometries++;
		  	if(geometryConvergence.numberOfGeometries == 1 )
		  	{
				geometryConvergence.typeOfFile = GABEDIT_TYPEFILE_GAMESS;
				geometryConvergence.fileName = g_strdup(fileName);
				geometryConvergence.numGeometry =  g_malloc(sizeof(gint));	
				geometryConvergence.numGeometry[0] =  1;
				geometryConvergence.energy =  g_malloc(sizeof(gdouble));	
				geometryConvergence.energy[0] = atof(tmp);
				geometryConvergence.maxStep =  NULL;
				geometryConvergence.rmsStep =  NULL;
		  	}
		  	else
		  	{
				geometryConvergence.numGeometry =  
				g_realloc(geometryConvergence.numGeometry,geometryConvergence.numberOfGeometries*sizeof(gint));	
				k = geometryConvergence.numberOfGeometries-1;
				geometryConvergence.numGeometry[k] =  k+1;
				geometryConvergence.energy =  
				g_realloc(geometryConvergence.energy,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.energy[k] = atof(tmp);

				geometryConvergence.maxStep =  NULL;
				geometryConvergence.rmsStep =  NULL;
		  	}
			OK = TRUE;
		}
	}
	if(!OK)
	{
		freeGeometryConvergence();
		sprintf(t,_("Sorry\nI can not read energy from %s file "),fileName);
		Message(t,_("Error"),TRUE);
		OK = FALSE;
	 }

	fclose(file);
	g_free(t);
	g_free(temp);
	g_free(tmp);
	if(geometryConvergence.numberOfGeometries>0)
	{
		gint i;
		geometryConvergence.geometries = g_malloc(geometryConvergence.numberOfGeometries*sizeof(Geometry));
		for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			if(!read_gamess_irc_file_geomi(fileName,geometryConvergence.numGeometry[i], &geometryConvergence.geometries[i])) break;
		if(i!=geometryConvergence.numberOfGeometries)
		{
			freeGeometryConvergence();
			OK = FALSE;
		}
	}
  	rafreshList();
	return OK;
}
/*****************************************************************************************************/
static gboolean read_gaussian_file_geomi_str(gchar *FileName, gint num, gchar* str, Geometry* geometry)
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
	Atom* listOfAtoms = NULL;

  
 	file = FOpen(FileName, "rb");

	t=g_malloc(BSIZE);
 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(BSIZE*sizeof(char));

	numgeom =1;
 	do 
 	{
 		OK=FALSE;
 		while(!feof(file))
		{
			{char* e = fgets(t,BSIZE,file);}
	 		if ( strstr( t,str))
	  		{
				{char* e = fgets(t,BSIZE,file);}
				{char* e = fgets(t,BSIZE,file);}
				{char* e = fgets(t,BSIZE,file);}

				if(strstr( t, "Type" )) itype=1;
				else itype=0;
				{char* e = fgets(t,BSIZE,file);}
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
			{char* e = fgets(t,BSIZE,file);}
    			if (strstr( t, "----------------------------------" ) >0 )
    			{
      				break;
    			}
    			j++;
    			if(listOfAtoms == NULL) listOfAtoms = g_malloc(sizeof(Atom));
    			else listOfAtoms = g_realloc(listOfAtoms, (j+1)*sizeof(Atom));

    			if(itype==0) sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
    			else sscanf(t,"%d %s %d %s %s %s",&idummy,AtomCoord[0],&idummy,AtomCoord[1],AtomCoord[2],AtomCoord[3]);

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			sprintf(listOfAtoms[j].symbol,"%s",symb_atom_get((guint)atoi(AtomCoord[0])));
    			sprintf(listOfAtoms[j].mmType,"%s",listOfAtoms[j].symbol);
    			sprintf(listOfAtoms[j].pdbType,"%s",listOfAtoms[j].symbol);
    			for(i=0;i<3;i++) listOfAtoms[j].C[i]=atof(ang_to_bohr(AtomCoord[i+1]));
			listOfAtoms[j].partialCharge = 0.0;
			listOfAtoms[j].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[j].symbol);
			listOfAtoms[j].variable = 0;
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
static gboolean read_gaussian_file_geomi(gchar *FileName, gint num, Geometry* geometry)
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


	if(read_gaussian_file_geomi_str(FileName,num,"Standard orientation:", geometry)) return TRUE;
	if(read_gaussian_file_geomi_str(FileName,num,"Input orientation:", geometry)) return TRUE;
	/* for calculation with nosym option */
	if(!read_gaussian_file_geomi_str(FileName,num,"Z-Matrix orientation:", geometry))
	{
  		Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
		return FALSE;
	}
	return TRUE;

}
/********************************************************************************/
static gboolean read_molpro_file_geomi(gchar *FileName, gint num, Geometry* geometry)
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
	Atom* listOfAtoms = NULL;

  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return FALSE ;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
 	}
 	t=g_malloc(BSIZE);
 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(BSIZE*sizeof(char));

 	numgeom =1;
 	do 
 	{
 		OK=FALSE;
 		while(!feof(file))
		{
			{char* e = fgets(t,BSIZE,file);}
			if (strstr(t,"Optimization point")) 
			{
				OK=TRUE;
				break;
			}
		}
		if(!OK) break;
 		OK=FALSE;
 		while(!feof(file))
		{
			{char* e = fgets(t,BSIZE,file);}
			if ( !strcmp(t," ATOMIC COORDINATES\n"))
			{
				{char* e = fgets(t,BSIZE,file);}
				{char* e = fgets(t,BSIZE,file);}
				if(strstr(t,"Q_EFF"))continue;
				{char* e = fgets(t,BSIZE,file);}
 				numgeom++;
                		OK = TRUE;
	  			break;
	  		}
        	}
 		if(!OK && (numgeom == 1) )
		{
  			Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
 			fclose(file);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			return FALSE;
    		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(file) )
  		{
			{char* e = fgets(t,BSIZE,file);}
			if ( !strcmp(t,"\n"))
			{
				break;
			}
    			j++;
    			if(listOfAtoms == NULL) listOfAtoms = g_malloc(sizeof(Atom));
    			else listOfAtoms = g_realloc(listOfAtoms, (j+1)*sizeof(Atom));

			sscanf(t,"%d %s %s %s %s %s",&idummy, AtomCoord[0],AtomCoord[1],AtomCoord[1], AtomCoord[2],AtomCoord[3]);

			for(i=0;i<(gint)strlen(AtomCoord[0]);i++) if(isdigit(AtomCoord[0][i])) AtomCoord[0][i] = ' ';
			delete_all_spaces(AtomCoord[0]);

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			sprintf(listOfAtoms[j].symbol,"%s",AtomCoord[0]);
    			sprintf(listOfAtoms[j].mmType,"%s",listOfAtoms[j].symbol);
    			sprintf(listOfAtoms[j].pdbType,"%s",listOfAtoms[j].symbol);
    			for(i=0;i<3;i++) listOfAtoms[j].C[i]=atof((AtomCoord[i+1]));
			listOfAtoms[j].partialCharge = 0.0;
			listOfAtoms[j].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[j].symbol);
			listOfAtoms[j].variable = 0;
  		}

		if(num >0 && (gint)numgeom-1 == num) break;
			
 	}while(!feof(file));

 	fclose(file);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(j+1 == 0 && listOfAtoms )
	{
		g_free(listOfAtoms);
		OK = FALSE;
	}
 	else
	{
		geometry->numberOfAtoms = j+1;
		geometry->listOfAtoms = listOfAtoms;
		OK = TRUE;
	}
	return OK;
}
/********************************************************************************/
static gboolean read_mpqc_file_geomi(gchar *fileName,gint numGeometry, Geometry* geometry)
{
 	gchar *t;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *file;
 	guint idummy;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numGeom;
	gdouble tmpReal;
	Atom* listOfAtoms = NULL;

  
 	file = FOpen(fileName, "rb");

 	if(file ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
 	}

	t=g_malloc(BSIZE);
 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(BSIZE*sizeof(char));

	numGeom = 0;
	do 
	{
		gboolean unitOfOutAng = FALSE;
		OK=FALSE;
		while(!feof(file))
		{
			if(!fgets(t,BSIZE,file)) break;
			if (strstr(t,"<Molecule>"))
			{
				gboolean OkUnit = FALSE;
				do{
		 			if(!fgets(t,BSIZE,file))break;
					if(strstr(t,"unit"))
					{
						OkUnit = TRUE;
						if(strstr(t,"angstrom"))unitOfOutAng=TRUE;
						break;
					};
					
				}while(!feof(file));
				if(!OkUnit) break;
				numGeom++;
	       			if((gint)numGeom == numGeometry )
				{
					OK = TRUE;
		 			break;
				}
	       			if(numGeometry<0 )
				{
					OK = TRUE;
		 			break;
				}
			}
	       }
		if(!OK && (numGeom == 0) )
		{
	 		g_free(t);
	 		t = g_strdup_printf(_("Sorry\nI can read Geometry from %s  file "),fileName);
	 		Message(t,_("Error"),TRUE);
	 		g_free(t);
			for(i=0;i<5;i++) g_free(AtomCoord[i]);
	 		return FALSE;
		}
		if(!OK) break;
		OK = FALSE;
		while(!feof(file) )
		{
	   		if(!fgets(t,BSIZE,file))break;
			if ( !(strstr(t,"atoms") && strstr(t,"geometry"))) continue;
			OK = TRUE;
			break;
		}
		if(!OK)
		{
	 		g_free(t);
	 		t = g_strdup_printf(_("Sorry\nI can read Geometry from %s  file "),fileName);
	 		Message(t,_("Error"),TRUE);
	 		g_free(t);
			for(i=0;i<5;i++) g_free(AtomCoord[i]);
	 		return FALSE;
		}

		j=-1;
		while(!feof(file) )
		{
	   		if(!fgets(t,BSIZE,file))break;
	   		if(strstr(t,"}"))break;
			j++;

    			if(listOfAtoms==NULL) listOfAtoms=g_malloc(sizeof(Atom));
    			else listOfAtoms=g_realloc(listOfAtoms,(j+1)*sizeof(Atom));

			for(i=0;i<strlen(t);i++) if(t[i]=='[' || t[i] ==']') t[i]=' ';
			sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
			for(i=1;i<=3;i++)
			{
				tmpReal = atof(AtomCoord[i]);
				sprintf(AtomCoord[i],"%lf",tmpReal);
			}

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);

			l=strlen(AtomCoord[0]);

			if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			sprintf(listOfAtoms[j].symbol,"%s",AtomCoord[0]);
    			sprintf(listOfAtoms[j].mmType,"%s",listOfAtoms[j].symbol);
    			sprintf(listOfAtoms[j].pdbType,"%s",listOfAtoms[j].symbol);
    			for(i=0;i<3;i++)
				if(unitOfOutAng)
    					listOfAtoms[j].C[i]=atof(ang_to_bohr(AtomCoord[i+1]));
				else
    					listOfAtoms[j].C[i]=atof(AtomCoord[i+1]);
			listOfAtoms[j].partialCharge = 0.0;
			listOfAtoms[j].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[j].symbol);
			listOfAtoms[j].variable = 0;

		}

		if(numGeometry<0) continue;
		if(OK) break;
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
		geometry->numberOfAtoms = j + 1;
		geometry->listOfAtoms = listOfAtoms;
	}
	return TRUE;
}
/********************************************************************************/
static gboolean read_mopac_aux_file_geomi(gchar *FileName, gint numgeometry, Geometry* geometry)
{
 	gchar *t;
 	/* gboolean OK;*/
 	gchar *AtomCoord[5];
 	FILE *file;
 	guint i;
 	gint j=0;
 	gint l;
 	guint numgeom = 0;
	Atom* listOfAtoms = NULL;
	gchar* pdest = NULL;
	gint nElements = 0;
	gchar** elements = NULL;
	long int geomposok = 0;
	gchar** nuclearCharges = NULL;
	gint nNuclearCharges = 0;
  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return FALSE ;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
 	}
	elements = get_one_block_from_aux_mopac_file(file, "ATOM_EL[",  &nElements);
	if(!elements)
	{
  		Message(_("Sorry\nI can not read the symbol of atoms from this file"),_("Error"),TRUE);
		fclose(file);
  		return FALSE;
	}
 	geomposok = ftell(file);
	nuclearCharges = get_one_block_from_aux_mopac_file(file, "ATOM_CORE[",  &nNuclearCharges);
	if(!nuclearCharges) fseek(file, geomposok, SEEK_SET);

 	t=g_malloc(BSIZE*sizeof(gchar));
 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(BSIZE*sizeof(gchar));
	fseek(file, 0, SEEK_SET);
	 while(!feof(file))
	 {
		if(!fgets(t,BSIZE,file))break;
		if(numgeometry<0) pdest = strstr( t, "ATOM_X_OPT:ANGSTROMS");
		else pdest = strstr( t, "ATOM_X_UPDATED:ANGSTROMS");
		if ( pdest )
		{
			numgeom++;
			geomposok = ftell(file);
			if(numgeom == numgeometry )
			{
				/* OK = TRUE;*/
				break;
			}
			/*
			if(numgeometry<0) { OK = TRUE; }
			*/
		}
	 }
	 if(numgeom == 0)
	 {
		free_one_string_table(elements, nElements);
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not read geometry in %s file "),FileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return FALSE;
	  }

	init_dipole();
	for(i=0;i<3;i++) Dipole.value[i] = 0;
	j=-1;
	fseek(file, geomposok, SEEK_SET);
	while(!feof(file) )
	{
		if(!fgets(t,BSIZE,file))break;
		if(strstr( t, "[")
		  || strstr(t,"HEAT_OF_FORM_UPDATED")
		  || strstr( t, "####################################")
		  ||this_is_a_backspace(t))
		{
			break;
		}
		if(j+1>nElements)break;
		j++;
    		if(listOfAtoms == NULL) listOfAtoms = g_malloc(sizeof(Atom));
    		else listOfAtoms = g_realloc(listOfAtoms, (j+1)*sizeof(Atom));

		sscanf(t,"%s %s %s",AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		if(j<nElements) sprintf(AtomCoord[0],"%s",elements[j]);
		else sprintf(AtomCoord[0],"X");
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
		l=strlen(AtomCoord[0]); 
		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
		if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);


    		sprintf(listOfAtoms[j].symbol,"%s",t);
    		sprintf(listOfAtoms[j].mmType,"%s",listOfAtoms[j].symbol);
    		sprintf(listOfAtoms[j].pdbType,"%s",listOfAtoms[j].symbol);
    		for(i=0;i<3;i++) listOfAtoms[j].C[i]=atof((AtomCoord[i+1]))*ANG_TO_BOHR;
		/* TO DO Read charge and nuclear charge from output file */
		listOfAtoms[j].partialCharge = 0.0;

		if(nuclearCharges && nNuclearCharges>j) listOfAtoms[j].nuclearCharge = atof(nuclearCharges[j]);
		else listOfAtoms[j].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[j].symbol);

	}

 	fclose(file);
	free_one_string_table(elements, nElements);
	free_one_string_table(nuclearCharges, nNuclearCharges);

 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(j+1 == 0 && listOfAtoms )
	{
		g_free(listOfAtoms);
	}
 	else
	{
		geometry->numberOfAtoms = j+1;
		geometry->listOfAtoms = listOfAtoms;
	}
	return TRUE;
}
/********************************************************************************/
static gboolean read_qchem_file_geomi(gchar *FileName, gint num, Geometry* geometry)
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
	Atom* listOfAtoms = NULL;
	gchar* pdest = NULL;

  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return FALSE ;
 	}

 	file = FOpen(FileName, "rb");
 	if(file ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return FALSE;
 	}
 	t=g_malloc(BSIZE);
 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(BSIZE*sizeof(gchar));

 	numgeom =1;
 	do 
 	{
 		OK=FALSE;
 		while(!feof(file))
		{
			if(!fgets(t,BSIZE,file))break;
			pdest = strstr( t, "Atom         X            Y            Z");
			if(pdest) 
			{
				if(!fgets(t,BSIZE,file))break;
				pdest = strstr( t, "----------------------------------------");
			}
			if (pdest)
			{
 				numgeom++;
                		OK = TRUE;
	  			break;
	  		}
        	}
 		if(!OK && (numgeom == 1) )
		{
  			Message(_("Sorry\nI can not read geometry in this file"),_("Error"),TRUE);
 			fclose(file);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			return FALSE;
    		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(file) )
  		{
			{char* e = fgets(t,BSIZE,file);}
			if ( strstr( t, "----------------------------------------"))
			{
				break;
			}
    			j++;
    			if(listOfAtoms == NULL) listOfAtoms = g_malloc(sizeof(Atom));
    			else listOfAtoms = g_realloc(listOfAtoms, (j+1)*sizeof(Atom));

			sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
			l=strlen(AtomCoord[0]); 
			if(isdigit(AtomCoord[0][1]))l=1;
			if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
			else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);

    			sprintf(listOfAtoms[j].symbol,"%s",t);
    			sprintf(listOfAtoms[j].mmType,"%s",listOfAtoms[j].symbol);
    			sprintf(listOfAtoms[j].pdbType,"%s",listOfAtoms[j].symbol);
    			for(i=0;i<3;i++) listOfAtoms[j].C[i]=atof((AtomCoord[i+1]))*ANG_TO_BOHR;
			listOfAtoms[j].partialCharge = 0.0;
			listOfAtoms[j].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[j].symbol);
			listOfAtoms[j].variable = 0;
  		}
		if(num >0 && (gint)numgeom-1 == num) break;
			
 	}while(!feof(file));

 	fclose(file);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(j+1 == 0 && listOfAtoms )
	{
		g_free(listOfAtoms);
	}
 	else
	{
		geometry->numberOfAtoms = j+1;
		geometry->listOfAtoms = listOfAtoms;
	}
	return TRUE;
}
/*********************************************************************/
static gboolean read_xyz_file_geomi(gchar *fileName, gint geometryNumber, Geometry* geometry)
{
	gchar* t;
	gboolean OK;
	gchar* AtomCoord[5];
	FILE *file;
	gint i;
	gint j;
	gint l;
	gint numgeom = 0;
	gint nn = 0;
	Atom* listOfAtoms = NULL;

	file = FOpen(fileName, "rb");

	if(file ==NULL)
	{
		t = g_strdup_printf(_("Sorry\nI can not open %s file "),fileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return FALSE;
	}
	t = g_malloc(BSIZE*sizeof(gchar));
	for(i=0;i<5;i++) AtomCoord[i] = g_malloc(BSIZE*sizeof(gchar));

	numgeom =0;
	OK=TRUE;
	while(!feof(file))
	{
		if(!fgets(t, BSIZE,file))break;

		str_delete_n(t);
		delete_last_spaces(t);
		delete_first_spaces(t);
		if(!isInteger(t))break;
		numgeom++;
		if(numgeom == geometryNumber)
		{
			nn = atoi(t);
			if(nn<1)break;
    			listOfAtoms=g_malloc(nn*sizeof(Atom));
			if(!fgets(t, BSIZE,file))break; /* title */
			for(j=0; j<nn; j++)
			{
				if(!fgets(t, BSIZE,file))break;
    				sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
				AtomCoord[0][0]=toupper(AtomCoord[0][0]);
				l=strlen(AtomCoord[0]);
				if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    				sprintf(listOfAtoms[j].symbol,"%s",AtomCoord[0]);
    				sprintf(listOfAtoms[j].mmType,"%s",listOfAtoms[j].symbol);
    				sprintf(listOfAtoms[j].pdbType,"%s",listOfAtoms[j].symbol);
    				for(i=0;i<3;i++) listOfAtoms[j].C[i]=atof(AtomCoord[i+1])*ANG_TO_BOHR;
				listOfAtoms[j].partialCharge = 0.0;
				listOfAtoms[j].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[j].symbol);
				listOfAtoms[j].variable = 0;
			}
			OK = TRUE;
			break;
		}
		else
		{
			nn = atoi(t);
			if(!fgets(t, BSIZE,file)) break;
			for(i=0; i<nn; i++)
				if(!fgets(t, BSIZE,file))break;
			if(i!=nn) { OK = FALSE; break;}
		}
		if(!OK) break;
	}

	fclose(file);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nn == 0 )
	{
		if(listOfAtoms) g_free(listOfAtoms);
		listOfAtoms = NULL;
	}
 	else
	{
		geometry->numberOfAtoms = nn;
		geometry->listOfAtoms = listOfAtoms;
	}

	return TRUE;
}
/********************************************************************************/
static gboolean save_geometry_convergence_gabedit_format(gchar *FileName)
{
 	FILE *file;
	gint j;
	gboolean OK = TRUE;

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
	fprintf(file,"[GEOCONV]\n");
	if(geometryConvergence.energy)
	{
		fprintf(file," energy\n");
		for(j=0;j<geometryConvergence.numberOfGeometries;j++)
			fprintf(file," %lf\n", geometryConvergence.energy[j]);
	}
	else
	{
		fprintf(file," energy\n");
		for(j=0;j<geometryConvergence.numberOfGeometries;j++) fprintf(file," %lf\n",j+1.0);
	}
	if(geometryConvergence.maxForce)
	{
		fprintf(file," max-force\n");
		for(j=0;j<geometryConvergence.numberOfGeometries;j++)
			fprintf(file," %lf\n", geometryConvergence.maxForce[j]);
	}
	if(geometryConvergence.rmsForce)
	{
		fprintf(file," rms-force\n");
		for(j=0;j<geometryConvergence.numberOfGeometries;j++)
			fprintf(file," %lf\n", geometryConvergence.rmsForce[j]);
	}
	if(geometryConvergence.maxStep)
	{
		fprintf(file," max-step\n");
		for(j=0;j<geometryConvergence.numberOfGeometries;j++)
			fprintf(file," %lf\n", geometryConvergence.maxStep[j]);
	}
	if(geometryConvergence.rmsStep)
	{
		fprintf(file," rms-step\n");
		for(j=0;j<geometryConvergence.numberOfGeometries;j++)
			fprintf(file," %lf\n", geometryConvergence.rmsStep[j]);
	}
	fprintf(file,"\n");
	if(!geometryConvergence.numGeometry) { fclose(file); return FALSE;}
	fprintf(file,"[GEOMETRIES]\n");
	for(j=0;j<geometryConvergence.numberOfGeometries;j++)
	{
		gint i;
		gint nAtoms = geometryConvergence.geometries[j].numberOfAtoms;
		Atom* listOfAtoms = geometryConvergence.geometries[j].listOfAtoms;
		if(nAtoms<1 || !listOfAtoms) { OK = FALSE; break;}
		fprintf(file," %d\n", nAtoms);
		fprintf(file," All coordinates are in Angshtrom\n");
		for(i=0;i<nAtoms;i++)
		{
			fprintf(file," %s %lf %lf %lf\n", 
				listOfAtoms[i].symbol,
				listOfAtoms[i].C[0]*BOHR_TO_ANG,
				listOfAtoms[i].C[1]*BOHR_TO_ANG,
				listOfAtoms[i].C[2]*BOHR_TO_ANG
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

	geometryConvergence.velocity = velo;
	if(!play) stopAnimation(NULL,NULL);

}
/********************************************************************************/
static gboolean read_gabedit_molden_rms_conv(FILE* file)
{
 	gchar* t = g_malloc(BSIZE*sizeof(gchar));
	gboolean OK = FALSE;
	gint i;

	fseek(file, 0L, SEEK_SET);
	OK = FALSE;
 	while(!feof(file))
	{
		if(!fgets(t, BSIZE,file)) break;
		if(strstr(t,"[GEOCONV]")) { OK = TRUE; break;}
	}
	if(!OK)
	{
		g_free(t);
		return FALSE;
	}
 	while(!feof(file))
	{
		if(!fgets(t, BSIZE,file)) break;
		uppercase(t);
   		if( strstr( t,"ENERGY") != NULL )
		{
			geometryConvergence.energy = g_malloc(geometryConvergence.numberOfGeometries*sizeof(gdouble));
			for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			{
		 		if(!fgets(t,BSIZE,file)) break;
				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
		 		if(!isFloat(t)) break;
				geometryConvergence.energy[i] = atof(t);
			}
			if(i!=geometryConvergence.numberOfGeometries)
			{
				g_free(geometryConvergence.energy);
				geometryConvergence.energy = NULL;
			}
		}
   		if( strstr( t,"MAX-FORCE") != NULL )
		{
			geometryConvergence.maxForce = g_malloc(geometryConvergence.numberOfGeometries*sizeof(gdouble));
			for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			{
		 		if(!fgets(t,BSIZE,file)) break;
				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
		 		if(!isFloat(t)) break;
				geometryConvergence.maxForce[i] = atof(t);
			}
			if(i!=geometryConvergence.numberOfGeometries)
			{
				g_free(geometryConvergence.maxForce);
				geometryConvergence.maxForce = NULL;
			}
		}
   		if( strstr( t,"RMS-FORCE") != NULL )
		{
			geometryConvergence.rmsForce = g_malloc(geometryConvergence.numberOfGeometries*sizeof(gdouble));
			for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			{
		 		if(!fgets(t,BSIZE,file)) break;
				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
		 		if(!isFloat(t)) break;
				geometryConvergence.rmsForce[i] = atof(t);
			}
			if(i!=geometryConvergence.numberOfGeometries)
			{
				g_free(geometryConvergence.rmsForce);
				geometryConvergence.rmsForce = NULL;
			}
		}
   		if( strstr( t,"MAX-STEP") != NULL )
		{
			geometryConvergence.maxStep = g_malloc(geometryConvergence.numberOfGeometries*sizeof(gdouble));
			for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			{
		 		if(!fgets(t,BSIZE,file)) break;
				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
		 		if(!isFloat(t)) break;
				geometryConvergence.maxStep[i] = atof(t);
			}
			if(i!=geometryConvergence.numberOfGeometries)
			{
				g_free(geometryConvergence.maxStep);
				geometryConvergence.maxStep = NULL;
			}
		}
   		if( strstr( t,"RMS-STEP") != NULL )
		{
			geometryConvergence.rmsStep = g_malloc(geometryConvergence.numberOfGeometries*sizeof(gdouble));
			for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			{
		 		if(!fgets(t,BSIZE,file)) break;
				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
		 		if(!isFloat(t)) break;
				geometryConvergence.rmsStep[i] = atof(t);
			}
			if(i!=geometryConvergence.numberOfGeometries)
			{
				g_free(geometryConvergence.rmsStep);
				geometryConvergence.rmsStep = NULL;
			}
		}
   		if( strstr( t,"[GEOMETRIES]") != NULL ) break;
	}
	return TRUE;

}
/********************************************************************************/
static gboolean read_gabedit_molden_geom_conv(gchar *fileName, GabEditTypeFile type)
{
 	gchar* t;
 	gchar* tmp;
 	gchar* sdum;
 	gchar* pdest;
 	gdouble fdum1, fdum2, fdum3;
 	gboolean OK;
 	FILE *file;
	gint i;
	gint ne;
	gint numgeom = 0;

	tmp = get_name_file(fileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	if(type== GABEDIT_TYPEFILE_GABEDIT) set_status_label_info(_("File type"),"Gabedit");
	if(type== GABEDIT_TYPEFILE_MOLDEN) set_status_label_info(_("File type"),"Molden");

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
	numgeom =0;
	OK=TRUE;
	while(!feof(file))
	{
		if(!fgets(t, BSIZE,file))break;
		pdest = strstr( t, "[GEOMETRIES]");
		if(pdest && strstr(t,"ZMAT"))
		{
			if(type == GABEDIT_TYPEFILE_MOLDEN) sprintf(t,_("Sorry\nMolden file with ZMAT coordinate is not supported by Gabedit"));
			if(type == GABEDIT_TYPEFILE_GABEDIT) sprintf(t,_("Sorry\nGabedit file with ZMAT coordinate is not supported by Gabedit"));
			Message(t,_("Error"),TRUE);
			return FALSE;
		}	
 		if (pdest)
		{
			while(!feof(file))
			{
				if(!fgets(t,BSIZE,file))break;
				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
				if(!isInteger(t))break;
               			numgeom++;
				OK = TRUE;
				if(numgeom==1)
				{
					geometryConvergence.fileName = g_strdup(fileName);
					geometryConvergence.typeOfFile = type;
					geometryConvergence.numberOfGeometries = 1;
					geometryConvergence.numGeometry = g_malloc(sizeof(gint));
					geometryConvergence.numGeometry[0] = 1;
				}
				else
				{
					geometryConvergence.numberOfGeometries = numgeom;
					geometryConvergence.numGeometry = 
						g_realloc(geometryConvergence.numGeometry, geometryConvergence.numberOfGeometries*sizeof(gint));
					geometryConvergence.numGeometry[numgeom-1] = numgeom;
				}
				ne = atoi(t);
				if(!fgets(t,BSIZE,file)) break;
				for(i=0; i<ne; i++)
				{
					if(!fgets(t,BSIZE,file))break;
					if(sscanf(t,"%s %lf %lf %lf",sdum, &fdum1, &fdum2, &fdum3)!=4) break;
				}
				if(i!=ne) { OK = FALSE;break;}
			}
			if(!OK) break;
		}
	}
	/* energy and rms and max */
	if(OK)
	{
		OK = read_gabedit_molden_rms_conv(file);
			
		if(!geometryConvergence.energy)
		{
			freeGeometryConvergence();
			sprintf(t,_("Sorry\nI can not read energy from %s file "),fileName);
			Message(t,_("Error"),TRUE);
			OK = FALSE;
		}
	}

	fclose(file);
 	g_free(t);
 	g_free(sdum);
	if(!OK) freeGeometryConvergence();
	if(geometryConvergence.numberOfGeometries>0)
	{
		geometryConvergence.geometries = g_malloc(geometryConvergence.numberOfGeometries*sizeof(Geometry));
		for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			if(!read_molden_gabedit_file_geomi(fileName,geometryConvergence.numGeometry[i], geometryConvergence.typeOfFile, &geometryConvergence.geometries[i])) break;
		if(i!=geometryConvergence.numberOfGeometries)
		{
			freeGeometryConvergence();
			OK = FALSE;
		}
	}
	rafreshList();


	return TRUE;
}
/*********************************************************************/
static gint get_number_geoms_gabedit(gchar* fileName)
{
	gchar t[BSIZE];
	FILE *file;
	gint nGeometries = 0;
	gint nLabels = 0;
	gchar* pdest = NULL;
        
 	file = FOpen(fileName, "rb"); 
        if(!file) return -1;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		pdest = strstr( t, "[GEOMS]");
 		if (pdest)
		{
			if(!fgets(t,BSIZE,file))break;
			sscanf(t,"%d %d",&nGeometries, &nLabels);
			fclose(file);
			return nGeometries;
		}
	}
	fclose(file);
	return 0;
}
/********************************************************************************/
static gint read_gabedit_geoms_file(gchar* fileName)
{
#define SZ 50
	gboolean OK;
	gchar symbol[SZ];
	gchar mmType[SZ];
	gchar pdbType[SZ];
	gchar residueName[SZ];
	gdouble X,Y,Z;
	gdouble charge;
	gint layer;
	FILE *fd;
	guint taille=BSIZE;
	gint i;
	gint j;
	gint k;
	gint nGeometries = 0;
	gint nLabels = 0;
	gint nAtoms=0;
	gint l;
	GabEditTypeFile type = GABEDIT_TYPEFILE_GABEDIT;
	gchar* tmp = NULL;
	gchar* t = NULL;
	Atom* listOfAtoms = NULL;
	gint residueNumber = 0;
	gint ienergy = -1;

	tmp = get_name_file(fileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Gabedit");

 	fd = FOpen(fileName, "rb");
	t = g_malloc(BSIZE*sizeof(gchar));
	if(fd ==NULL)
	{
		sprintf(t,_("Sorry\nI can not open %s file "),fileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return FALSE;
	}
	OK=FALSE;
	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
		if(strstr( t, "[GEOMS]"))
		{
			OK = TRUE;
			break;
		};
	}
	if(!OK)
	{
		fclose(fd);
		g_free(t);
		return 1;
	}

	if(!fgets(t,taille,fd)){ fclose(fd); g_free(t); return 1;}
	sscanf(t,"%d %d",&nGeometries, &nLabels);
	if(nGeometries<1) { fclose(fd); g_free(t); return 1;}
	for(i=0;i<nLabels;i++)
	{
		if(!fgets(t,taille,fd)) { fclose(fd); g_free(t); return 1;}
		if(mystrcasestr(t,"Energy")) ienergy = i;
	}
	if(i!=nLabels) { fclose(fd); g_free(t); return 1;}
	OK = TRUE;

	geometryConvergence.fileName = g_strdup(fileName);
	geometryConvergence.typeOfFile = type;
	geometryConvergence.numberOfGeometries = nGeometries;
	geometryConvergence.numGeometry = g_malloc(nGeometries*sizeof(gint));
	geometryConvergence.geometries = g_malloc(geometryConvergence.numberOfGeometries*sizeof(Geometry));
	geometryConvergence.energy =  g_malloc(geometryConvergence.numberOfGeometries*sizeof(gdouble));	

	for(j=0;j<nGeometries;j++)
	{
		geometryConvergence.energy[j] = 0.0;
		geometryConvergence.numGeometry[j] = j+1;
 		geometryConvergence.geometries[j].numberOfAtoms = 0;
 		geometryConvergence.geometries[j].listOfAtoms = NULL;
	}

	OK = TRUE;
	for(j=0;j<nGeometries;j++)
	{
		for(i=0;i<nLabels;i++) 
		{
			if(!fgets(t,taille,fd)){ OK = TRUE;break;}
			if(i==ienergy) geometryConvergence.energy[j] = atof(t);
		}
		if(i!=nLabels) { OK = TRUE;break;}
		if(!fgets(t,taille,fd)){ OK = TRUE;break;}
		sscanf(t,"%d %d %d",&nAtoms, &TotalCharges[0], &SpinMultiplicities[0]);
		if(nAtoms<1){ OK = TRUE;break;}

    		listOfAtoms = g_malloc(nAtoms*sizeof(Atom));
		for(i=0; i<nAtoms; i++)
		{
			gint variable = 0;
			if(!fgets(t,taille,fd))break;
    			k = sscanf(t,"%s %s %s %s %d %lf %d %d %lf %lf %lf",
					symbol,mmType,pdbType,residueName, 
					&residueNumber,
					&charge,&layer,&variable,&X,&Y,&Z);
			if(k!=11) 
			{
				OK = FALSE;
				break;
			}
			symbol[0]=toupper(symbol[0]);
			l=strlen(symbol);
			if (l==2) symbol[1]=tolower(symbol[1]);

    			sprintf(listOfAtoms[i].symbol,"%s",symbol);
    			sprintf(listOfAtoms[i].mmType,"%s",mmType);
    			sprintf(listOfAtoms[i].pdbType,"%s",pdbType);
    			listOfAtoms[i].C[0]=X*ANG_TO_BOHR;
    			listOfAtoms[i].C[1]=Y*ANG_TO_BOHR;
    			listOfAtoms[i].C[2]=Z*ANG_TO_BOHR;
			listOfAtoms[i].partialCharge = charge;
			listOfAtoms[i].variable = variable;
			listOfAtoms[i].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[i].symbol);

 			geometryConvergence.geometries[j].numberOfAtoms = nAtoms;
 			geometryConvergence.geometries[j].listOfAtoms = listOfAtoms;
		}
		if(!OK) break;
	}
	fclose(fd);
	if(!OK) 
	{
		freeGeometryConvergence();
	}

	rafreshList();
	return 0;
#undef SZ
}
/********************************************************************************/
static gboolean read_dalton_output(gchar* fileName)
{
	gchar *pdest;
	gint  k=0;
	gchar *temp =  NULL;
	gchar *tmp =  NULL;
	gchar *t = NULL;
	FILE *file;
	gboolean OK;
        
	temp = get_name_file(fileName);
	set_status_label_info(_("File name"),temp);
	g_free(temp);
	set_status_label_info(_("File type"),"Dalton");

 	file = FOpen(fileName, "rb"); 
        if(!file)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),fileName);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return FALSE;
	}
	t=g_malloc(BSIZE*sizeof(gchar));
	temp =  g_malloc(50*sizeof(char));	
	tmp =  g_malloc(50*sizeof(char));	
        
	OK = TRUE;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) break;
		pdest = strstr( t,"Optimization Control Center");
   		if( pdest != NULL )
		{
			OK = TRUE;
			while(!feof(file) && OK )
			{
		 		if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
				if(strstr(t,"Next geometry") || strstr(t,"Final geometry"))
				{
		 			if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
		 			if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;

			OK = TRUE;
			while(!feof(file) && OK )
			{
		 		if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
				if(strstr(t,"Energy at this geometry is"))
				{
		 			gchar* t1 = strstr(t,":");
					if(t1) sscanf(t1+1,"%s",tmp); /* energy */
					else { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
			OK = TRUE;
			while(!feof(file) && OK )
			{
		 		if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
				if(strstr(t,"Norm of step"))
				{
		 			gchar* t1 = strstr(t,":");
					if(t1) sscanf(t1+1,"%s",temp); /* rmsStep */
					else { OK = FALSE; break; }
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
		}
   		if(pdest != NULL)
		{
			geometryConvergence.numberOfGeometries++;
		  	if(geometryConvergence.numberOfGeometries == 1 )
		  	{
				geometryConvergence.typeOfFile = GABEDIT_TYPEFILE_DALTON;
				geometryConvergence.fileName = g_strdup(fileName);
				geometryConvergence.numGeometry =  g_malloc(sizeof(gint));	
				geometryConvergence.numGeometry[0] =  1;
				geometryConvergence.energy =  g_malloc(sizeof(gdouble));	
				geometryConvergence.energy[0] = atof(tmp);
				geometryConvergence.rmsStep =  g_malloc(sizeof(gdouble));	
				geometryConvergence.rmsStep[0] = atof(temp);
		  	}
		  	else
		  	{
				geometryConvergence.numGeometry =  
				g_realloc(geometryConvergence.numGeometry,geometryConvergence.numberOfGeometries*sizeof(gint));	
				k = geometryConvergence.numberOfGeometries-1;
				geometryConvergence.numGeometry[k] =  k+1;
				geometryConvergence.energy =  
				g_realloc(geometryConvergence.energy,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.energy[k] = atof(tmp);

				geometryConvergence.rmsStep =  g_realloc(geometryConvergence.rmsStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.rmsStep[k] = atof(temp);
		  	}
			OK = TRUE;
		}
	}
	if(!OK)
	{
		freeGeometryConvergence();
		sprintf(t,_("Sorry\nI can not read energy or convergence parameters from %s file "),fileName);
		Message(t,_("Error"),TRUE);
		OK = FALSE;
	 }

	fclose(file);
	g_free(t);
	g_free(temp);
	g_free(tmp);
	if(geometryConvergence.numberOfGeometries>0)
	{
		gint i;
		geometryConvergence.geometries = g_malloc(geometryConvergence.numberOfGeometries*sizeof(Geometry));
		for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			if(!read_dalton_file_geomi(fileName,geometryConvergence.numGeometry[i], &geometryConvergence.geometries[i])) break;
		if(i!=geometryConvergence.numberOfGeometries)
		{
			freeGeometryConvergence();
			OK = FALSE;
		}
	}
  	rafreshList();
	return OK;
}
/********************************************************************************/
static gboolean read_gaussian_output(gchar* fileName)
{
	gchar *pdest;
	gint  k=0;
	gchar *temp =  NULL;
	gchar *t = NULL;
	FILE *file;
	gint nE = 0;
	gint nSF = 0;
	gboolean OK;
        
	temp = get_name_file(fileName);
	set_status_label_info(_("File name"),temp);
	g_free(temp);
	set_status_label_info(_("File type"),"Gaussian output");

 	file = FOpen(fileName, "rb"); 
        if(!file)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),fileName);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return FALSE;
	}
	t=g_malloc(BSIZE*sizeof(gchar));
	temp =  g_malloc(50*sizeof(char));	
        
	OK = TRUE;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) break;
		uppercase(t);
    		pdest = strstr( t,"SCF DONE");
   		if( pdest != NULL ) pdest = strstr( t,"=");
         	if(!pdest)
		{
    		 	pdest = strstr( t,"ENERGY=");
          	 	if(pdest && t[1] == 'E' ) pdest = strstr( t,"=");
			else pdest = NULL;
		}
   		if(pdest != NULL)
		{
			pdest++;
			geometryConvergence.numberOfGeometries++;
			nE++;
		  	if(geometryConvergence.numberOfGeometries == 1 )
		  	{
				geometryConvergence.typeOfFile = GABEDIT_TYPEFILE_GAUSSIAN;
				geometryConvergence.fileName = g_strdup(fileName);
				geometryConvergence.numGeometry =  g_malloc(sizeof(gint));	
				geometryConvergence.numGeometry[0] =  1;
				geometryConvergence.energy =  g_malloc(sizeof(gdouble));	
				geometryConvergence.energy[0] = atof(pdest);
				geometryConvergence.maxForce =  g_malloc(sizeof(gdouble));	
				geometryConvergence.rmsForce =  g_malloc(sizeof(gdouble));	
				geometryConvergence.maxStep =  g_malloc(sizeof(gdouble));	
				geometryConvergence.rmsStep =  g_malloc(sizeof(gdouble));	
		  	}
		  	else
		  	{
				geometryConvergence.numGeometry =  
				g_realloc(geometryConvergence.numGeometry,geometryConvergence.numberOfGeometries*sizeof(gint));	
				k = geometryConvergence.numberOfGeometries-1;
				geometryConvergence.numGeometry[k] =  k+1;
				geometryConvergence.energy =  
				g_realloc(geometryConvergence.energy,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.energy[k] = atof(pdest);

				geometryConvergence.maxForce =  g_realloc(geometryConvergence.maxForce,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.rmsForce =  g_realloc(geometryConvergence.rmsForce, geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.maxStep =  g_realloc(geometryConvergence.maxStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.rmsStep =  g_realloc(geometryConvergence.rmsStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));	
		  	}
		}
                pdest = NULL;
    		pdest = strstr( t,"CONVERGED?");
                if( pdest != NULL && geometryConvergence.numberOfGeometries>0)
		{
			nSF++;
			if(nSF != nE)
			{
				OK = FALSE;
				break;
			}
			k = geometryConvergence.numberOfGeometries-1;
			{char* e = fgets(t,BSIZE,file);}
                 	sscanf(t,"%s %s %lf", temp,temp,&geometryConvergence.maxForce[k]);
			{char* e = fgets(t,BSIZE,file);}
                 	sscanf(t,"%s %s %lf", temp,temp,&geometryConvergence.rmsForce[k]);
			{char* e = fgets(t,BSIZE,file);}
                 	sscanf(t,"%s %s %lf", temp,temp,&geometryConvergence.maxStep[k]);
			{char* e = fgets(t,BSIZE,file);}
                 	sscanf(t,"%s %s %lf", temp,temp,&geometryConvergence.rmsStep[k]);
		}
	}
	if(!OK)
	{
		freeGeometryConvergence();
		sprintf(t,_("Sorry\nI can not read energy or convergence parameters from %s file "),fileName);
		Message(t,_("Error"),TRUE);
		OK = FALSE;
	 }
	if(nSF<nE)
	{
		if(nSF>0)
		{
			geometryConvergence.numberOfGeometries = nSF;
			geometryConvergence.numGeometry =  
			g_realloc(geometryConvergence.numGeometry,geometryConvergence.numberOfGeometries*sizeof(gint));	
			geometryConvergence.energy =  
			g_realloc(geometryConvergence.energy,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
			geometryConvergence.maxForce =  g_realloc(geometryConvergence.maxForce,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
			geometryConvergence.rmsForce =  g_realloc(geometryConvergence.rmsForce, geometryConvergence.numberOfGeometries*sizeof(gdouble));	
			geometryConvergence.maxStep =  g_realloc(geometryConvergence.maxStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.rmsStep =  g_realloc(geometryConvergence.rmsStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));	

		}
		else
		{
			freeGeometryConvergence();
			sprintf(t,_("Sorry\nI can not read energy or convergence parameters from %s file "),fileName);
			Message(t,_("Error"),TRUE);
			OK = FALSE;
		}

	}
	fclose(file);
	g_free(t);
	g_free(temp);
	if(geometryConvergence.numberOfGeometries>0)
	{
		gint i;
		geometryConvergence.geometries = g_malloc(geometryConvergence.numberOfGeometries*sizeof(Geometry));
		for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			if(!read_gaussian_file_geomi(fileName,geometryConvergence.numGeometry[i], &geometryConvergence.geometries[i])) break;
		if(i!=geometryConvergence.numberOfGeometries)
		{
			freeGeometryConvergence();
			OK = FALSE;
		}
	}
  	rafreshList();
	return OK;
}
/********************************************************************************/
static gboolean read_molpro_log(gchar* fileName)
{
	gchar *pdest;
	gint  k=0;
	gchar *temp =  NULL;
	gchar *tmp =  NULL;
	gchar *t = NULL;
	FILE *file;
	gint nE = 0;
	gint nSF = 0;
	gboolean OK;
        
	temp = get_name_file(fileName);
	set_status_label_info(_("File name"),temp);
	g_free(temp);
	set_status_label_info(_("File type"),"Molpro log");

 	file = FOpen(fileName, "rb"); 
        if(!file)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),fileName);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return FALSE;
	}
	t=g_malloc(BSIZE*sizeof(gchar));
	temp =  g_malloc(50*sizeof(char));	
	tmp =  g_malloc(50*sizeof(char));	
        
	OK = TRUE;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) break;
		pdest = strstr( t,"Optimization point");
   		if( pdest != NULL )
		{
			OK = TRUE;
			while(!feof(file) && OK )
			{
		 		if(!fgets(t, BSIZE,file))
				{
					OK = FALSE;
					break;
				}
				if(strstr(t,"(") && strstr(t,")"))
				{
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
		}
   		if(pdest != NULL)
		{
                 	sscanf(t,"%s %s %s %s %s",temp,temp,temp,temp,tmp);
			geometryConvergence.numberOfGeometries++;
			nE++;
		  	if(geometryConvergence.numberOfGeometries == 1 )
		  	{
				geometryConvergence.typeOfFile = GABEDIT_TYPEFILE_MOLPRO_LOG;
				geometryConvergence.fileName = g_strdup(fileName);
				geometryConvergence.numGeometry =  g_malloc(sizeof(gint));	
				geometryConvergence.numGeometry[0] =  1;
				geometryConvergence.energy =  g_malloc(sizeof(gdouble));	
				geometryConvergence.energy[0] = atof(tmp);
				geometryConvergence.rmsStep =  g_malloc(sizeof(gdouble));	
		  	}
		  	else
		  	{
				geometryConvergence.numGeometry =  
				g_realloc(geometryConvergence.numGeometry,geometryConvergence.numberOfGeometries*sizeof(gint));	
				k = geometryConvergence.numberOfGeometries-1;
				geometryConvergence.numGeometry[k] =  k+1;
				geometryConvergence.energy =  
				g_realloc(geometryConvergence.energy,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.energy[k] = atof(tmp);

				geometryConvergence.rmsStep =  g_realloc(geometryConvergence.rmsStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));	
		  	}
			OK = TRUE;
                 	do
                 	{
		 		if(!fgets(t, BSIZE,file)) { OK = FALSE; break;}
                 	}while(strstr(t,"Convergence:")==NULL && !feof(file) ) ;
			if(!OK) break;
                 	if(strstr(t,"Convergence:")!=NULL)
			{
				nSF++;
				k = geometryConvergence.numberOfGeometries-1;
                 		sscanf(t,"%s %s %s %s %s ", temp,temp,temp,temp,tmp);
                 		geometryConvergence.rmsStep[k] = atof(tmp);
			}
		}
	}
	if(!OK)
	{
		freeGeometryConvergence();
		sprintf(t,_("Sorry\nI can not read energy or convergence parameters from %s file "),fileName);
		Message(t,_("Error"),TRUE);
		OK = FALSE;
	 }
	if(nSF<nE)
	{
		if(nSF>0)
		{
			geometryConvergence.numberOfGeometries = nSF;
			geometryConvergence.numGeometry =  
			g_realloc(geometryConvergence.numGeometry,geometryConvergence.numberOfGeometries*sizeof(gint));	
			geometryConvergence.energy =  
			g_realloc(geometryConvergence.energy,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
			geometryConvergence.rmsStep =  g_realloc(geometryConvergence.rmsStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));	

		}
		else
		{
			freeGeometryConvergence();
			sprintf(t,_("Sorry\nI can not read energy or convergence parameters from %s file "),fileName);
			Message(t,_("Error"),TRUE);
			OK = FALSE;
		}

	}

	fclose(file);
	g_free(t);
	g_free(temp);
	g_free(tmp);
	if(geometryConvergence.numberOfGeometries>0)
	{
		gint i;
		geometryConvergence.geometries = g_malloc(geometryConvergence.numberOfGeometries*sizeof(Geometry));
		for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			if(!read_molpro_file_geomi(fileName,geometryConvergence.numGeometry[i], &geometryConvergence.geometries[i])) break;
		if(i!=geometryConvergence.numberOfGeometries)
		{
			freeGeometryConvergence();
			OK = FALSE;
		}
	}
  	rafreshList();
	return OK;
}
/********************************************************************************/
static gboolean read_mpqc_output(gchar* fileName)
{
	gchar *pdest;
	gint  k=0;
	gchar *temp =  NULL;
	gchar *t = NULL;
	FILE *file;
	gint nE = 0;
	gint nSF = 0;
	gboolean OK;
	gboolean newGeom;
	gboolean mp2;
        
	temp = get_name_file(fileName);
	set_status_label_info(_("File name"),temp);
	g_free(temp);
	set_status_label_info(_("File type"),"MPQC output");

 	file = FOpen(fileName, "rb"); 
        if(!file)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),fileName);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return FALSE;
	}
	t=g_malloc(BSIZE*sizeof(gchar));
	temp =  g_malloc(50*sizeof(char));	
        
	OK = TRUE;
	mp2 = FALSE;
	newGeom = FALSE;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) break;
                 if(strstr(t,"changing atomic coordinates:"))
		 {
			newGeom = TRUE;
			if(strstr(t,"MBPT2")) mp2 = TRUE;
		 }
		 pdest = NULL;
          	if(newGeom && mp2)
		{
			pdest = NULL;
    		 	pdest = strstr( t,"MP2");
    		 	if(pdest)
			{
				if(strstr(t,"correlation")) pdest = NULL;
				else pdest = strstr( t,"energy");
   				if( pdest != NULL ) pdest = strstr( t,":");
			}
		}
		if(newGeom && !mp2)
		{
			pdest = NULL;
			pdest = strstr( t,"total scf energy");
   			if( pdest != NULL ) pdest = strstr( t,"=");
		}
   		if(pdest != NULL)
		{
			pdest++;
			geometryConvergence.numberOfGeometries++;
			nE++;
		  	if(geometryConvergence.numberOfGeometries == 1 )
		  	{
				geometryConvergence.typeOfFile = GABEDIT_TYPEFILE_MPQC;
				geometryConvergence.fileName = g_strdup(fileName);
				geometryConvergence.numGeometry =  g_malloc(sizeof(gint));	
				geometryConvergence.numGeometry[0] =  1;
				geometryConvergence.energy =  g_malloc(sizeof(gdouble));	
				geometryConvergence.energy[0] = atof(pdest);
				geometryConvergence.maxStep =  g_malloc(sizeof(gdouble));	
		  	}
		  	else
		  	{
				geometryConvergence.numGeometry =  
				g_realloc(geometryConvergence.numGeometry,geometryConvergence.numberOfGeometries*sizeof(gint));	
				k = geometryConvergence.numberOfGeometries-1;
				geometryConvergence.numGeometry[k] =  k+1;
				geometryConvergence.energy =  
				g_realloc(geometryConvergence.energy,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.energy[k] = atof(pdest);

				geometryConvergence.maxStep =  g_realloc(geometryConvergence.maxStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));	
		  	}
		}
                pdest = NULL;
    		pdest = strstr( t,"Max Displacement");
                if( pdest != NULL && geometryConvergence.numberOfGeometries>0)
		{
    		 	pdest = strstr( t,":");
			if(pdest)
			{
				pdest++;
				nSF++;
				k = geometryConvergence.numberOfGeometries-1;
                 		sscanf(pdest,"%lf",&geometryConvergence.maxStep[k]);
			}
		 }
	}
	if(!OK)
	{
		freeGeometryConvergence();
		sprintf(t,_("Sorry\nI can not read energy or convergence parameters from %s file "),fileName);
		Message(t,_("Error"),TRUE);
		OK = FALSE;
	 }
	if(nSF<nE)
	{
		if(nSF>0)
		{
			geometryConvergence.numberOfGeometries = nSF;
			geometryConvergence.numGeometry =  
			g_realloc(geometryConvergence.numGeometry,geometryConvergence.numberOfGeometries*sizeof(gint));	
			geometryConvergence.energy =  
			g_realloc(geometryConvergence.energy,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
			geometryConvergence.maxStep =  g_realloc(geometryConvergence.maxStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));	

		}
		else
		{
			freeGeometryConvergence();
			sprintf(t,_("Sorry\nI can not read energy or convergence parameters from %s file "),fileName);
			Message(t,_("Error"),TRUE);
			OK = FALSE;
		}

	}

	fclose(file);
	g_free(t);
	g_free(temp);
	if(geometryConvergence.numberOfGeometries>0)
	{
		gint i;
		geometryConvergence.geometries = g_malloc(geometryConvergence.numberOfGeometries*sizeof(Geometry));
		for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			if(!read_mpqc_file_geomi(fileName,geometryConvergence.numGeometry[i], &geometryConvergence.geometries[i])) break;
		if(i!=geometryConvergence.numberOfGeometries)
		{
			freeGeometryConvergence();
			OK = FALSE;
		}
	}
  	rafreshList();
	return OK;
}
/********************************************************************************/
static gboolean read_mopac_aux(gchar* fileName)
{
	gint  k=0;
	gchar *temp =  NULL;
	gchar *tmp =  NULL;
	gchar *t = NULL;
	FILE *file;
	gboolean OK;
        
	temp = get_name_file(fileName);
	set_status_label_info(_("File name"),temp);
	g_free(temp);
	set_status_label_info(_("File type"),"Mopac aux");

 	file = FOpen(fileName, "rb"); 
        if(!file)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),fileName);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return FALSE;
	}
	t=g_malloc(BSIZE*sizeof(gchar));
	temp=g_malloc(100*sizeof(gchar));
	tmp=g_malloc(100*sizeof(gchar));
	OK = FALSE;
	while(!feof(file) && !OK)
	{
		 if(!fgets(t,BSIZE,file))break;
                 if(strstr(t,"Geometry optimization") && strstr(t,"#") )
		 {
			OK = TRUE;
			while(!feof(file) && OK )
			{
		 		if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
				if(strstr(t,"####################################"))
				{
					OK = TRUE;
					break;
				}
		 	}
		 }
	}
        if(!OK)
	{
		t = g_strdup_printf(_(" Error : No Geometry optimization in %s file\n"),fileName);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return FALSE;
	}
	OK = TRUE;
	while(!feof(file) && OK )
	{
		if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
		if(strstr(t,"####################################")) break;

		if(strstr(t,"HEAT_OF_FORM_UPDATED"))
		{
			gchar* t1 = strstr(t,"=");
			if(t1) 
			{
				sscanf(t1+1,"%s",tmp); /* energy */
				OK = TRUE;
			}
			else OK = FALSE;
		}
		else continue;
	 	if(!OK) break;
	 	if(!fgets(t, BSIZE,file)) { OK = FALSE; break; }
		if(strstr(t,"GRADIENT_UPDATED"))
		{
	 		gchar* t1 = strstr(t,"=");
			if(t1) 
			{
				sscanf(t1+1,"%s",temp); /* gradient */
				OK = TRUE;
			}
			else OK = FALSE;
		}
	 	if(!OK) break;
		
		for(k=0;k<strlen(tmp);k++)
		{
			if(tmp[k]=='D') tmp[k]='e';
			if(tmp[k]=='d') tmp[k]='e';
		}
		for(k=0;k<strlen(temp);k++)
		{
			if(temp[k]=='D') temp[k]='e';
			if(temp[k]=='d') temp[k]='e';
		}
		geometryConvergence.numberOfGeometries++;
	  	if(geometryConvergence.numberOfGeometries == 1 )
	  	{
			geometryConvergence.typeOfFile = GABEDIT_TYPEFILE_MOPAC;
			geometryConvergence.fileName = g_strdup(fileName);
			geometryConvergence.numGeometry =  g_malloc(sizeof(gint));	
			geometryConvergence.numGeometry[0] =  1;
			geometryConvergence.energy =  g_malloc(sizeof(gdouble));	
			geometryConvergence.energy[0] = atof(tmp);
			geometryConvergence.rmsStep =  g_malloc(sizeof(gdouble));	
			geometryConvergence.rmsStep[0] = atof(temp);
	  	}
		else
		{
			geometryConvergence.numGeometry =  
			g_realloc(geometryConvergence.numGeometry,geometryConvergence.numberOfGeometries*sizeof(gint));	
			k = geometryConvergence.numberOfGeometries-1;
			geometryConvergence.numGeometry[k] =  k+1;
			geometryConvergence.energy =  
			g_realloc(geometryConvergence.energy,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
			geometryConvergence.energy[k] = atof(tmp);

			geometryConvergence.rmsStep =  g_realloc(geometryConvergence.rmsStep, geometryConvergence.numberOfGeometries*sizeof(gdouble));	
			geometryConvergence.rmsStep[k] = atof(temp);
	  	}
	}
	if(!OK)
	{
		freeGeometryConvergence();
		sprintf(t,_("Sorry\nI can not read energy or convergence parameters from %s file "),fileName);
		Message(t,_("Error"),TRUE);
		OK = FALSE;
	 }
	fclose(file);
	g_free(t);
	g_free(temp);
	g_free(tmp);
	if(geometryConvergence.numberOfGeometries>0)
	{
		gint i;
		geometryConvergence.geometries = g_malloc(geometryConvergence.numberOfGeometries*sizeof(Geometry));
		for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			if(!read_mopac_aux_file_geomi(fileName,geometryConvergence.numGeometry[i], &geometryConvergence.geometries[i])) break;
		if(i!=geometryConvergence.numberOfGeometries)
		{
			freeGeometryConvergence();
			OK = FALSE;
		}
	}
  	rafreshList();
	return OK;
}
/********************************************************************************/
static gboolean read_qchem_output(gchar* fileName)
{
	gint  k=0;
	gchar *temp =  NULL;
	gchar *tmp =  NULL;
	gchar *t = NULL;
	FILE *file;
	gboolean OK;
	gchar* pdest;
	gint nE = 0;
	gint nSF = 0;
        
	temp = get_name_file(fileName);
	set_status_label_info(_("File name"),temp);
	g_free(temp);
	set_status_label_info(_("File type"),"Q-Chem out");

 	file = FOpen(fileName, "rb"); 
        if(!file)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),fileName);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return FALSE;
	}
	t=g_malloc(BSIZE*sizeof(gchar));
	temp =  g_malloc(50*sizeof(char));	
	tmp =  g_malloc(50*sizeof(char));	
        
	OK = TRUE;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file)) break;
    		pdest = strstr( t,"Optimization Cycle:");
   		if( pdest != NULL )
		{
			OK = TRUE;
			while(!feof(file) && OK )
			{
		 		if(!fgets(t, BSIZE,file))
				{
					OK = FALSE;
					break;
				}
				if(strstr(t,"Energy is"))
				{
					OK = TRUE;
					break;
				}
		 	}
		 	if(!OK) break;
		}
    		pdest = strstr( t,"Energy is");
   		if(pdest != NULL)
		{
                 	sscanf(t,"%s %s %s",temp,temp,tmp);
			geometryConvergence.numberOfGeometries++;
			nE++;
		  	if(geometryConvergence.numberOfGeometries == 1 )
		  	{
				geometryConvergence.typeOfFile = GABEDIT_TYPEFILE_QCHEM;
				geometryConvergence.fileName = g_strdup(fileName);
				geometryConvergence.numGeometry =  g_malloc(sizeof(gint));	
				geometryConvergence.numGeometry[0] =  1;
				geometryConvergence.energy =  g_malloc(sizeof(gdouble));	
				geometryConvergence.energy[0] = atof(tmp);
				geometryConvergence.maxForce =  g_malloc(sizeof(gdouble));	
		  	}
		  	else
		  	{
				geometryConvergence.numGeometry =  
				g_realloc(geometryConvergence.numGeometry,geometryConvergence.numberOfGeometries*sizeof(gint));	
				k = geometryConvergence.numberOfGeometries-1;
				geometryConvergence.numGeometry[k] =  k+1;
				geometryConvergence.energy =  
				g_realloc(geometryConvergence.energy,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.energy[k] = atof(tmp);

				geometryConvergence.maxForce =  g_realloc(geometryConvergence.maxForce, geometryConvergence.numberOfGeometries*sizeof(gdouble));	
				geometryConvergence.maxForce[k] = -1;
		  	}
			OK = TRUE;
                 	do
                 	{
		 		if(!fgets(t, BSIZE,file)) { OK = FALSE; break;}
                 	}while(strstr(t,"Cnvgd?")==NULL && !feof(file) ) ;
			if(!OK) break;
                 	if(strstr(t,"Cnvgd?")!=NULL)
			{
		 		if(!fgets(t, BSIZE,file)) { OK = FALSE; break;}
				nSF++;
				k = geometryConvergence.numberOfGeometries-1;
                 		sscanf(t,"%s %s", temp,tmp);
                 		geometryConvergence.maxForce[k] = atof(tmp);
			}
		}
	}
	if(!OK)
	{
		freeGeometryConvergence();
		sprintf(t,_("Sorry\nI can not read energy or convergence parameters from %s file "),fileName);
		Message(t,_("Error"),TRUE);
		OK = FALSE;
	 }
	if(nSF<nE)
	{
		if(nSF>0)
		{
			geometryConvergence.numberOfGeometries = nSF;
			geometryConvergence.numGeometry =  
			g_realloc(geometryConvergence.numGeometry,geometryConvergence.numberOfGeometries*sizeof(gint));	
			geometryConvergence.energy =  
			g_realloc(geometryConvergence.energy,geometryConvergence.numberOfGeometries*sizeof(gdouble));	
			geometryConvergence.maxForce =  g_realloc(geometryConvergence.maxForce, geometryConvergence.numberOfGeometries*sizeof(gdouble));	

		}
		else
		{
			freeGeometryConvergence();
			sprintf(t,_("Sorry\nI can not read energy or convergence parameters from %s file "),fileName);
			Message(t,_("Error"),TRUE);
			OK = FALSE;
		}

	}

	fclose(file);
	g_free(t);
	g_free(temp);
	g_free(tmp);
	if(geometryConvergence.numberOfGeometries>0)
	{
		gint i;
		geometryConvergence.geometries = g_malloc(geometryConvergence.numberOfGeometries*sizeof(Geometry));
		for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			if(!read_qchem_file_geomi(fileName,geometryConvergence.numGeometry[i], &geometryConvergence.geometries[i])) break;
		if(i!=geometryConvergence.numberOfGeometries)
		{
			freeGeometryConvergence();
			OK = FALSE;
		}
	}
  	rafreshList();
	return OK;
}
/********************************************************************************/
static gboolean read_xyz_multi(gchar* fileName)
{
	gint  i=0;
	gchar *t = NULL;
	FILE *file;
	gint nGeom = 0;
	gboolean OK;
	gchar *sdum = NULL;
	gdouble fdum1;
	gdouble fdum2;
	gdouble fdum3;
	gint ne;
	gint nAtoms;
	gchar** comments = NULL;
        
	t = get_name_file(fileName);
	set_status_label_info(_("File name"),t);
	if(t) g_free(t);
	set_status_label_info(_("File type"),"XYZ");

 	file = FOpen(fileName, "rb"); 
        if(!file)
	{
		t = g_strdup_printf(_(" Error : I can not open file %s\n"),fileName);
		Message(t,_("Error"),TRUE);
		if(t) g_free(t);
		return FALSE;
	}
	t=g_malloc(BSIZE*sizeof(gchar));
	sdum =  g_malloc(50*sizeof(gchar));	
        
	OK = TRUE;
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		ne = sscanf(t,"%d",&nAtoms);
		if(ne==1 && nAtoms>0)
		 {
		 	if(!fgets(t, BSIZE,file)) break; /* title */
			str_delete_n(t);
			delete_last_spaces(t);
			delete_first_spaces(t);
			if(comments==NULL) comments = g_malloc(sizeof(gchar*));
			else comments = g_realloc(comments, (nGeom+1)*sizeof(gchar*));
			comments[nGeom] = g_strdup(t);
			for(i=0;i<nAtoms;i++)
			{
				if(!fgets(t, BSIZE,file)) { OK = FALSE; break;};
				ne = sscanf(t,"%s %lf %lf %lf",sdum,&fdum1, &fdum2, &fdum3);
				if(ne != 4) { OK = FALSE; break;};
			}
			if(!OK) break;
			nGeom++;
		 }
		else break;
	}
	if(OK && nGeom>0)
	{
		geometryConvergence.numberOfGeometries = nGeom;
		geometryConvergence.typeOfFile = GABEDIT_TYPEFILE_XYZ;
		geometryConvergence.fileName = g_strdup(fileName);
		geometryConvergence.numGeometry =  g_malloc(nGeom*sizeof(gint));
		geometryConvergence.comments =  comments;
		for(i=0;i<nGeom;i++) geometryConvergence.numGeometry[i] =  i+1;
	}
	else
	{
		sprintf(t,_("Sorry\nI can not read geometries from %s file "),fileName);
		Message(t,_("Error"),TRUE);
		OK = FALSE;
		if(comments) g_free(comments);
	 }

	fclose(file);
	g_free(t);
	g_free(sdum);
	if(geometryConvergence.numberOfGeometries>0)
	{
		gint i;
		geometryConvergence.geometries = g_malloc(geometryConvergence.numberOfGeometries*sizeof(Geometry));
		for(i=0;i<geometryConvergence.numberOfGeometries;i++)
			if(!read_xyz_file_geomi(fileName,geometryConvergence.numGeometry[i], &geometryConvergence.geometries[i])) break;
		if(i!=geometryConvergence.numberOfGeometries)
		{
			freeGeometryConvergence();
			OK = FALSE;
		}
	}
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

	freeGeometryConvergence();
  	rafreshList();
	if(0==get_number_geoms_gabedit(FileName))
		read_gabedit_molden_geom_conv(FileName, GABEDIT_TYPEFILE_GABEDIT);
	else
	{
		read_gabedit_geoms_file(FileName);
	}
}
/********************************************************************************/
static void read_molden_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryConvergence();
  	rafreshList();
	read_gabedit_molden_geom_conv(FileName, GABEDIT_TYPEFILE_MOLDEN);
}
/********************************************************************************/
static void read_dalton_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryConvergence();
  	rafreshList();
	read_dalton_output(FileName);
}
/********************************************************************************/
static void read_gamess_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryConvergence();
  	rafreshList();
	read_gamess_output(FileName);
}
/********************************************************************************/
static void read_gamess_irc_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryConvergence();
  	rafreshList();
	read_gamess_irc(FileName);
}
/********************************************************************************/
static void read_gaussian_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryConvergence();
  	rafreshList();
	read_gaussian_output(FileName);
}
/********************************************************************************/
static void read_molpro_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryConvergence();
  	rafreshList();
	read_molpro_log(FileName);
}
/********************************************************************************/
static void read_mpqc_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryConvergence();
  	rafreshList();
	read_mpqc_output(FileName);
}
/********************************************************************************/
static void read_mopac_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryConvergence();
  	rafreshList();
	read_mopac_aux(FileName);
}
/********************************************************************************/
static void read_qchem_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryConvergence();
  	rafreshList();
	read_qchem_output(FileName);
}
/********************************************************************************/
static void read_xyz_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	stopAnimation(NULL, NULL);

	freeGeometryConvergence();
  	rafreshList();
	read_xyz_multi(FileName);
}
/*************************************************************************************/
static void read_hin_numbers_of_atoms(FILE* file, int* natoms, int* nresidues)
{
	guint taille = BSIZE;
	gchar t[BSIZE];
	gchar dump[BSIZE];

	*natoms = 0;
	*nresidues = 0;


	fseek(file, 0L, SEEK_SET);
	while(!feof(file))
	{
    		if(!fgets(t,taille,file)) break;
    		sscanf(t,"%s",dump);
		lowercase(dump);
		if(!strcmp(dump,"atom")) (*natoms)++;
		if(!strcmp(dump,"res")) (*nresidues)++;
	}
}
/*************************************************************************************/
static gboolean read_atom_hin_file(FILE* file,gchar* listFields[])
{
	guint taille = BSIZE;
	gchar t[BSIZE];
	gchar dump[BSIZE];
	gint i;

    	if(!fgets(t,taille,file)) return FALSE;
    	sscanf(t,"%s",dump);
	lowercase(dump);
	if(strcmp(dump,"atom")!=0)
	{
		if(strcmp(dump,"res")==0)
		{
    			sscanf(t,"%s %s %s",dump,dump,listFields[1]);
			sprintf(listFields[0],"Unknown");
		}
		else
			return FALSE;
	}
	else
	{
		/* 0 -> Atom Type PDB Style*/
		/* 1 -> Atom Symbol*/
		/* 2 -> Atom Type Amber*/
		/* 3 -> Atom Charge*/
		/* 4 -> x*/
		/* 5 -> y*/
		/* 6 -> z*/
    		sscanf(t,"%s %s %s %s %s %s %s %s %s %s",dump,dump,listFields[0],listFields[1],listFields[2],dump,listFields[3],listFields[4],listFields[5],listFields[6]);
	}
	for(i=0;i<6;i++)
	{
		delete_last_spaces(listFields[i]);
		delete_first_spaces(listFields[i]);
	}
	return TRUE;

}
/********************************************************************************/
gboolean read_one_hin_file(gchar* FileName, Geometry* geometry)
{
	gchar *t;
	gboolean OK;
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j;
	gint k;
	gchar *listFields[8];
	int natoms = 0;
	int nresidues = 0;
    	Atom* listOfAtoms = NULL;

	if ((!FileName) || (strcmp(FileName,"") == 0)) return FALSE ;

	fd = FOpen(FileName, "rb");

	if(fd==NULL)
    		return FALSE ;
	
	read_hin_numbers_of_atoms(fd,&natoms, &nresidues);
	if(natoms<1)
    		return FALSE ;

	OK=TRUE;
	for(i=0;i<8;i++) listFields[i]=g_malloc(taille*sizeof(gchar));
	for(i=0;i<8;i++) sprintf(listFields[i]," ");
	t=g_malloc(taille*sizeof(gchar));

  	init_dipole();

	j=0;

    	listOfAtoms = g_malloc(natoms*sizeof(Atom));

	fseek(fd, 0L, SEEK_SET);
	j = 0;
	i = -1; /* number residu */
	while(!feof(fd))
	{
		if(!read_atom_hin_file(fd,listFields))continue;
		if(strcmp(listFields[0],"Unknown")==0)
		{
			i++;
			continue;
		}
		/* 0 -> Atom Type PDB Style*/
		/* 1 -> Atom Symbol*/
		/* 2 -> Atom Type Amber*/
		/* 3 -> Atom Charge*/
		/* 4 -> x*/
		/* 5 -> y*/
		/* 6 -> z*/
		/* printf("%s %s %s %s %s %s %s \n",listFields[0],listFields[1],listFields[2],listFields[3],listFields[4],listFields[5],listFields[6]);*/

    		sprintf(listOfAtoms[j].symbol,"%s",listFields[1]);
    		sprintf(listOfAtoms[j].mmType,"%s",listFields[2]);
    		sprintf(listOfAtoms[j].pdbType,"%s",listFields[0]);
    		for(k=0;k<3;k++) listOfAtoms[j].C[k]=atof(ang_to_bohr(listFields[k+4]));
		listOfAtoms[j].partialCharge = atof(listFields[3]);
		listOfAtoms[j].nuclearCharge = get_atomic_number_from_symbol(listOfAtoms[j].symbol);
		listOfAtoms[j].variable = 0;

		j++;
		if(j>=natoms)break;
	}
	fclose(fd);
	OK = TRUE;
	if(natoms!=j) OK = FALSE;

	if(OK)
	{
		geometry->numberOfAtoms = natoms;
		geometry->listOfAtoms = listOfAtoms;
	}
	else
	{
		if(listOfAtoms) g_free(listOfAtoms);
	}
	g_free(t);
	for(i=0;i<8;i++) g_free(listFields[i]);
	return OK;
}
/********************************************************************************/
static void read_hin_files(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;
	GSList* lists = NULL;
	GSList* cur = NULL;

	if(response_id != GTK_RESPONSE_OK) return;

	lists = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(SelecFile));

	stopAnimation(NULL, NULL);

	freeGeometryConvergence();
	cur = lists;
	geometryConvergence.numberOfGeometries = 0;
	while(cur != NULL)
	{
		geometryConvergence.numberOfGeometries++;
		cur = cur->next;
	}
	if(geometryConvergence.numberOfGeometries>0)
	{
		gint k = 0;
		geometryConvergence.geometries = g_malloc(geometryConvergence.numberOfGeometries*sizeof(Geometry));
		geometryConvergence.comments = g_malloc(geometryConvergence.numberOfGeometries*sizeof(gchar*));
		cur = lists;
		while(cur != NULL)
		{
			FileName = (gchar*)(cur->data);

			if(read_one_hin_file(FileName,  &geometryConvergence.geometries[k]))
			{
				geometryConvergence.comments[k] = g_strdup(get_name_file(FileName));
			       	k++;
			}
			cur = cur->next;
		}
		if(k<=0)
		{
			freeGeometryConvergence();
		}
		else
		if(k!=geometryConvergence.numberOfGeometries)
		{
			geometryConvergence.numberOfGeometries = k;
			geometryConvergence.geometries = g_realloc(geometryConvergence.geometries,
					geometryConvergence.numberOfGeometries*sizeof(Geometry));
		}
		if(k>0)
		{
			gint i = 0;
			geometryConvergence.typeOfFile = GABEDIT_TYPEFILE_XYZ;
			geometryConvergence.fileName = g_strdup("MultipleFile");
			geometryConvergence.numGeometry =  g_malloc(k*sizeof(gint));
			for(i=0;i<k;i++) geometryConvergence.numGeometry[i] =  i+1;
		}
	}
  	rafreshList();
	
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
	if(fileType == GABEDIT_TYPEFILE_DALTON) read_dalton_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GAUSSIAN) read_gaussian_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GABEDIT) read_gabedit_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOLPRO) read_molpro_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOPAC_AUX) read_mopac_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GAMESS) read_gamess_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GAMESSIRC) read_gamess_irc_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_FIREFLY) read_gamess_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOLDEN) read_molden_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_XYZ) read_xyz_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_HIN) read_hin_files(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MPQC) read_mpqc_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_QCHEM) read_qchem_file(selecFile, response_id);
	else 
	{
		Message( _("Sorry, I cannot find the type of your file\n")
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
static void read_molden_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_molden_file, _("Read geometries from a Molden file"), GABEDIT_TYPEFILE_MOLDEN,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_dalton_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_dalton_file, _("Read geometries from a Dalton output file"), GABEDIT_TYPEFILE_DALTON,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_gamess_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gamess_file, _("Read geometries from a Gamess output file"), GABEDIT_TYPEFILE_GAMESS,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_gamess_irc_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gamess_irc_file, _("Read geometries from a Gamess IRC file"), GABEDIT_TYPEFILE_GAMESSIRC,GABEDIT_TYPEWIN_ORB);

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
static void read_molpro_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_molpro_file, _("Read geometries from a Molpro log file"), GABEDIT_TYPEFILE_MOLPRO_LOG,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_mpqc_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_mpqc_file, _("Read geometries from a MPQC output file"), GABEDIT_TYPEFILE_MPQC,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_mopac_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_mopac_file, _("Read geometries from a Mopac aux file"), GABEDIT_TYPEFILE_MOPAC_AUX,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_qchem_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_qchem_file, _("Read geometries from a Q-Chem output file"), GABEDIT_TYPEFILE_QCHEM,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_xyz_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_xyz_file, _("Read geometries from a XYZ file"), GABEDIT_TYPEFILE_XYZ,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_hin_multiple_files_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_hin_files, _("Read geometries from a multiple hyperchem files"), GABEDIT_TYPEFILE_HIN,GABEDIT_TYPEWIN_ORB);
	
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(filesel),TRUE);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_file, _("Read geometries from a file"), GABEDIT_TYPEFILE_UNKNOWN,GABEDIT_TYPEWIN_ORB);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void save_gabedit_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
	save_geometry_convergence_gabedit_format(FileName);
}
/********************************************************************************/
static void save_mobcal_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
	if(print_mobcal_geometries(fileName))
		print_mobcal_run(fileName);
}
/********************************************************************************/
static gboolean set_geometry(gint k)
{
	Atom* listOfAtoms = NULL;
	gint nAtoms = 0;
	gint j;

	if(k<0 || k >= geometryConvergence.numberOfGeometries) return FALSE;

	if(GeomOrb)
	{
		free_atomic_orbitals();
		for(j=0;j<nCenters;j++) if(GeomOrb[j].Symb) g_free(GeomOrb[j].Symb);
		g_free(GeomOrb);
		GeomOrb = NULL;
	}
	nAtoms = geometryConvergence.geometries[k].numberOfAtoms;
	listOfAtoms = geometryConvergence.geometries[k].listOfAtoms;

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
		GeomOrb[j].nuclearCharge = listOfAtoms[j].nuclearCharge;
		GeomOrb[j].variable = listOfAtoms[j].variable;
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
static void save_gabedit_file_dlg()
{
	GtkWidget* filesel;
	if(geometryConvergence.numberOfGeometries<1)
	{
		return;
	}
 	filesel = file_chooser_save(save_gabedit_file,
			_("Save geometry convergences in gabedit file format"),
			GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void save_mobcal_file_dlg()
{
	GtkWidget* filesel;
	if(geometryConvergence.numberOfGeometries<1)
	{
		return;
	}
 	filesel = file_chooser_save(save_mobcal_file,
			_("Save geometries in a mobcal input file"),
			GABEDIT_TYPEFILE_MOBCAL,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
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
	freeGeometryConvergence();
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
	freeGeometryConvergence();

}
/**********************************************************************************/
static void set_sensitive_option(GtkUIManager *manager, gchar* path)
{
	GtkWidget *wid = gtk_ui_manager_get_widget (manager, path);
	gboolean sensitive = TRUE;
  	if(geometryConvergence.numberOfGeometries<1) sensitive = FALSE;
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, sensitive);
}
/**********************************************************************************/
static gboolean show_menu_popup(GtkUIManager *manager, guint button, guint32 time)
{
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuGeomConv");
	if (GTK_IS_MENU (menu)) 
	{
		set_sensitive_option(manager,"/MenuGeomConv/SaveMobcal");
		set_sensitive_option(manager,"/MenuGeomConv/SaveGabedit");
		set_sensitive_option(manager,"/MenuGeomConv/DeleteGeometry");
		set_sensitive_option(manager,"/MenuGeomConv/CreateGaussInput");
		set_sensitive_option(manager,"/MenuGeomConv/CreateGaussInputLink");
		set_sensitive_option(manager,"/MenuGeomConv/CreateGr");
		set_sensitive_option(manager,"/MenuGeomConv/ComputeConformerTpes");
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
	gchar *texts[7];
	gboolean visible[7] = { TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE};
	gchar* titles[]={"Energy","MAX step","RMS step", "MAX Force","RMS Force","Number of Geometry","Comments"};
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	model = gtk_tree_view_get_model(treeView);
        store = GTK_TREE_STORE (model);
	gtk_tree_store_clear(store);
        model = GTK_TREE_MODEL (store);

	if(geometryConvergence.numberOfGeometries>0)
	{
		if(!geometryConvergence.energy)   { visible[0] = FALSE;}
		if(!geometryConvergence.maxStep)  { visible[1] = FALSE;}
		if(!geometryConvergence.rmsStep)  { visible[2] = FALSE;}
		if(!geometryConvergence.maxForce) { visible[3] = FALSE;}
		if(!geometryConvergence.rmsForce) { visible[4] = FALSE;}
		if(!geometryConvergence.numGeometry) { visible[5] = FALSE;}
		if(!geometryConvergence.comments) { visible[6] = FALSE;}
	}
	else
	{
		for(i=0;i<7;i++) visible[i] = FALSE;
	}


	if(!visible[0]) visible[5] = TRUE;
	else visible[5] = FALSE;

	do
	{
		column = gtk_tree_view_get_column(treeView, 0);
		if(column) gtk_tree_view_remove_column(treeView, column);
	}while(column != NULL);

	for(k=0;k<7;k++)
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

  
	for(i=0;i<geometryConvergence.numberOfGeometries;i++)
	{
		if(visible[0]) texts[0] = g_strdup_printf("%lf",geometryConvergence.energy[i]);
		if(visible[1]) texts[1] = g_strdup_printf("%lf",geometryConvergence.maxStep[i]);
		if(visible[2]) texts[2] = g_strdup_printf("%lf",geometryConvergence.rmsStep[i]);
		if(visible[3]) texts[3] = g_strdup_printf("%lf",geometryConvergence.maxForce[i]);
		if(visible[4]) texts[4] = g_strdup_printf("%lf",geometryConvergence.rmsForce[i]);
		if(visible[5]) texts[5] = g_strdup_printf("%d",geometryConvergence.numGeometry[i]);
		if(visible[6]) texts[6] = g_strdup_printf("%s",geometryConvergence.comments[i]);
        	gtk_tree_store_append (store, &iter, NULL);
		for(k=0;k<7;k++)
		{
			if(visible[k])
			{
				gtk_tree_store_set (store, &iter, k, texts[k], -1);
				g_free(texts[k]);
			}
		}
	}

	if(geometryConvergence.numberOfGeometries>0)
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
	gchar* t = g_strdup_printf(
			_("\nA seriess of gab*.%s files was created in \"%s\" directeory.\n\n\n%s")
			, format, get_last_directory(),message);
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
	sprintf(t,"%lf",geometryConvergence.velocity);
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

	buttonDirFilm = create_button(WinDlg,_("Folder"));
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
	Button = create_button(WinDlg,_("Play"));
	gtk_table_attach(GTK_TABLE(table),Button,0,0+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	PlayButton = Button;

	Button = create_button(WinDlg,_("Stop"));
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
	gint widths[]={10,10,10,10,10,10,10};
	gchar* titles[]={"Energy","MAX step","RMS step", "MAX Force","RMS Force","Number of Geometry","Comments"};
	gint Factor=7;
	gint len = 7;


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


	for(i=0;i<geometryConvergence.numberOfGeometries;i++)
	{
		gchar* energy = geometryConvergence.energy?g_strdup_printf("%15.7f",geometryConvergence.energy[i]):g_strdup("0.0");
		set_geometry(i);

		nRings5 = getNumberOfRing5();
		nRings6 = getNumberOfRing6();
		if(nRings5<1 && nRings6<1) continue;
		if(nRings5>0) { 

			gchar* result = computeConformerTypeRing5MinInfo(energy, k5==0);
			if(result)
			{
				printf("%5d/%-5d : %s\n",i+1,geometryConvergence.numberOfGeometries,result);
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
				printf("%5d/%-5d : %s\n",i+1,geometryConvergence.numberOfGeometries,result);
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
	if(geometryConvergence.numberOfGeometries<1) play = FALSE;

	while(play)
	{
		while( gtk_events_pending() ) gtk_main_iteration();

		m += step;
		if(m>=geometryConvergence.numberOfGeometries)
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
		Waiting(geometryConvergence.velocity);
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
	        "     * a Dalton output file.\n"
	        "     * a Gamess output file.\n"
	        "     * a Gamess IRC file.\n"
	        "     * a Gaussian output file.\n"
	        "     * a Molpro log file.\n"
	        "     * a Mopac aux file.\n"
	        "     * a MPQC output file.\n"
	        "     * a FireFly output file.\n"
	        "     * a Q-Chem output file.\n"
	        "     * a Molden input file.\n"
	        "     * a XYZ file.\n"
	        "     * multiple Hyperchem files.\n"
		)
		 );
	win = Message(temp," Info ",FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
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
static void  add_cancel_ok_button(GtkWidget *Win,GtkWidget *vbox,GtkWidget *entry, GCallback myFunc)
{
	GtkWidget *hbox;
	GtkWidget *button;
	/* buttons box */
	hbox = create_hbox_false(vbox);
	gtk_widget_realize(Win);

	button = create_button(Win,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, GTK_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,_("OK"));
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
static void create_gaussian_file_dlg(gboolean oneFile)
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *vboxall;
	GtkWidget* vbox;
	GtkWidget* entryKeywords;

	if(geometryConvergence.numberOfGeometries<1) 
	{
		gchar* t = g_strdup_printf(_("Sorry\n You should read the geometries before")); 
		Message(t,_("Error"),TRUE);
		return;
	}

	/* Principal Window */
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),"Create multiple input files for Gaussian");
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 2);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Input Gaussian");
	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

	vboxall = create_vbox(Win);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
	gtk_box_pack_start(GTK_BOX(vboxall), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);
  	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

  	gtk_widget_realize(Win);
	
	entryKeywords = add_inputGauss_entrys(Win,vbox,TRUE);
	if(oneFile) add_cancel_ok_button(Win,vbox,entryKeywords,(GCallback)print_gaussian_geometries_link);
	else add_cancel_ok_button(Win,vbox,entryKeywords,(GCallback)print_gaussian_geometries);


	/* Show all */
	gtk_widget_show_all (Win);
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
	        "   1) Read geometries from a Dalton, Gaussian, Molpro, Gabedit, Molden, MPQC or from XYZ file.\n"
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
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/SaveMobcal");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/DeleteGeometry");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/CreateGaussInput");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/CreateGaussInputLink");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/CreateGr");
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/ComputeConformerTpes");
	}
	else if(!strcmp(name, "ReadAuto")) read_file_dlg();
	else if(!strcmp(name, "ReadGabedit")) read_gabedit_file_dlg();
	else if(!strcmp(name, "ReadDalton")) read_dalton_file_dlg();
	else if(!strcmp(name, "ReadGamess")) read_gamess_file_dlg();
	else if(!strcmp(name, "ReadGamessIRC")) read_gamess_irc_file_dlg();
	else if(!strcmp(name, "ReadFireFlyIRC")) read_gamess_irc_file_dlg();
	else if(!strcmp(name, "ReadGaussian")) read_gaussian_file_dlg();
	else if(!strcmp(name, "ReadMolpro")) read_molpro_file_dlg();
	else if(!strcmp(name, "ReadMopac")) read_mopac_file_dlg();
	else if(!strcmp(name, "ReadMPQC")) read_mpqc_file_dlg();
	else if(!strcmp(name, "ReadFireFly")) read_gamess_file_dlg();
	else if(!strcmp(name, "ReadQChem")) read_qchem_file_dlg();
	else if(!strcmp(name, "ReadMolden")) read_molden_file_dlg();
	else if(!strcmp(name, "ReadXYZ")) read_xyz_file_dlg();
	else if(!strcmp(name, "ReadHIN")) read_hin_multiple_files_dlg();
	else if(!strcmp(name, "SaveGabedit")) save_gabedit_file_dlg();
	else if(!strcmp(name, "SaveMobcal")) save_mobcal_file_dlg();
	else if(!strcmp(name, "DeleteGeometry")) delete_one_geometry();
	else if(!strcmp(name, "Close")) destroyDlg(NULL);
	else if(!strcmp(name, "HelpSupportedFormat")) help_supported_format();
	else if(!strcmp(name, "HelpAnimation")) help_animated_file();
	else if(!strcmp(name, "CreateGaussInput")) create_gaussian_file_dlg(FALSE);
	else if(!strcmp(name, "CreateGaussInputLink")) create_gaussian_file_dlg(TRUE);
	else if(!strcmp(name, "CreateGr")) create_gr_dlg(TRUE);
	else if(!strcmp(name, "ComputeConformerTpes")) computeConformerTypes();
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntries[] =
{
	{"File",     NULL, N_("_File"), NULL, NULL, G_CALLBACK (activate_action)},
	{"Read",     NULL, N_("_Read")},
	{"ReadAuto", NULL, N_("Read a file(Auto)"), NULL, "Read a file", G_CALLBACK (activate_action) },
	{"ReadGabedit", GABEDIT_STOCK_GABEDIT, N_("Read a G_abedit file"), NULL, "Read a Gabedit file", G_CALLBACK (activate_action) },
	{"ReadDalton", GABEDIT_STOCK_DALTON, N_("Read a _Dalton output file"), NULL, "Read a Dalton output file", G_CALLBACK (activate_action) },
	{"ReadGamess", GABEDIT_STOCK_GAMESS, N_("Read a _Gamess output file"), NULL, "Read a Gamess output file", G_CALLBACK (activate_action) },
	{"ReadGamessIRC", GABEDIT_STOCK_GAMESS, N_("Read a _Gamess IRC file"), NULL, "Read a Gamess IRC file", G_CALLBACK (activate_action) },
	{"ReadFireFlyIRC", GABEDIT_STOCK_FIREFLY, N_("Read a _FireFly IRC file"), NULL, "Read a FireFly IRC file", G_CALLBACK (activate_action) },
	{"ReadGaussian", GABEDIT_STOCK_GAUSSIAN, N_("Read a _Gaussian output file"), NULL, "Read a Gaussian output file", G_CALLBACK (activate_action) },
	{"ReadMolpro", GABEDIT_STOCK_MOLPRO, N_("Read a Mol_pro output file"), NULL, "Read Molpro output file", G_CALLBACK (activate_action) },
	{"ReadMopac", GABEDIT_STOCK_MOPAC, N_("Read a _Mopac aux file"), NULL, "Read a Mopac aux file", G_CALLBACK (activate_action) },
	{"ReadMPQC", GABEDIT_STOCK_MPQC, N_("Read a MP_QC output file"), NULL, "Read a MPQC output file", G_CALLBACK (activate_action) },
	{"ReadFireFly", GABEDIT_STOCK_FIREFLY, N_("Read a _FireFly output file"), NULL, "Read a FireFly output file", G_CALLBACK (activate_action) },
	{"ReadQChem", GABEDIT_STOCK_QCHEM, N_("Read a Q-_Chem output file"), NULL, "Read a Q-Chem output file", G_CALLBACK (activate_action) },
	{"ReadMolden", GABEDIT_STOCK_MOLDEN, N_("Read a Mol_den output file"), NULL, "Read a Molden file", G_CALLBACK (activate_action) },
	{"CreateGaussInput", GABEDIT_STOCK_GAUSSIAN, N_("_Create multiple input file for Gaussian"), NULL, "Save", G_CALLBACK (activate_action) },
	{"CreateGaussInputLink", GABEDIT_STOCK_GAUSSIAN, N_("_Create single input file for Gaussian with more geometries"), NULL, "Save", G_CALLBACK (activate_action) },
	{"CreateGr", GABEDIT_STOCK_GAUSSIAN, N_("_Compute pair radial distribution"), NULL, "gr", G_CALLBACK (activate_action) },
	{"ComputeConformerTpes", GABEDIT_STOCK_GAUSSIAN, N_("Compute conformer _Types"), NULL, "gr", G_CALLBACK (activate_action) },
	{"ReadXYZ", NULL, N_("Read a _xyz file"), NULL, "Read a xyz file", G_CALLBACK (activate_action) },
	{"ReadHIN", NULL, N_("Read a _Hyperchem files"), NULL, "Read a hyperchem files", G_CALLBACK (activate_action) },
	{"SaveGabedit", GABEDIT_STOCK_SAVE, N_("_Save in Gabedit format"), NULL, "Save", G_CALLBACK (activate_action) },
	{"SaveMobcal", GABEDIT_STOCK_SAVE, N_("_Save in Mobcal format"), NULL, "Save in mobcal format", G_CALLBACK (activate_action) },
	{"DeleteGeometry", GABEDIT_STOCK_CUT, N_("_Delete selected geometry"), NULL, "Delete selected geometry", G_CALLBACK (activate_action) },
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
"  <popup name=\"MenuGeomConv\">\n"
"    <separator name=\"sepMenuPopGabedit\" />\n"
"    <menuitem name=\"ReadAuto\" action=\"ReadAuto\" />\n"
"    <separator name=\"sepMenuAuto\" />\n"
"    <menuitem name=\"ReadGabedit\" action=\"ReadGabedit\" />\n"
"    <menuitem name=\"ReadFireFly\" action=\"ReadFireFly\" />\n"
"    <menuitem name=\"ReadDalton\" action=\"ReadDalton\" />\n"
"    <menuitem name=\"ReadGamess\" action=\"ReadGamess\" />\n"
"    <menuitem name=\"ReadGaussian\" action=\"ReadGaussian\" />\n"
"    <menuitem name=\"ReadMolpro\" action=\"ReadMolpro\" />\n"
"    <menuitem name=\"ReadMopac\" action=\"ReadMopac\" />\n"
"    <menuitem name=\"ReadMPQC\" action=\"ReadMPQC\" />\n"
"    <menuitem name=\"ReadQChem\" action=\"ReadQChem\" />\n"
"    <menuitem name=\"ReadMolden\" action=\"ReadMolden\" />\n"
"    <menuitem name=\"ReadXYZ\" action=\"ReadXYZ\" />\n"
"    <menuitem name=\"ReadHIN\" action=\"ReadHIN\" />\n"
"    <menuitem name=\"ReadFireFlyIRC\" action=\"ReadFireFlyIRC\" />\n"
"    <menuitem name=\"ReadGamessIRC\" action=\"ReadGamessIRC\" />\n"
"    <separator name=\"sepMenuPopSave\" />\n"
"    <menuitem name=\"SaveGabedit\" action=\"SaveGabedit\" />\n"
"    <menuitem name=\"SaveMobcal\" action=\"SaveMobcal\" />\n"
"    <separator name=\"sepMenuDelete\" />\n"
"    <menuitem name=\"DeleteGeometry\" action=\"DeleteGeometry\" />\n"
"    <separator name=\"sepMenuCreateGauss\" />\n"
"    <menuitem name=\"CreateGaussInput\" action=\"CreateGaussInput\" />\n"
"    <menuitem name=\"CreateGaussInputLink\" action=\"CreateGaussInputLink\" />\n"
"    <separator name=\"sepMenuCreateGr\" />\n"
"    <menuitem name=\"CreateGr\" action=\"CreateGr\" />\n"
"    <separator name=\"sepMenuComputeConformerTpes\" />\n"
"    <menuitem name=\"ComputeConformerTpes\" action=\"ComputeConformerTpes\" />\n"
"    <separator name=\"sepMenuPopClose\" />\n"
"    <menuitem name=\"Close\" action=\"Close\" />\n"
"  </popup>\n"
"  <menubar name = \"MenuBar\">\n"
"    <menu name=\"File\" action=\"File\">\n"
"      <menu name=\"Read\" action=\"Read\">\n"
"        <menuitem name=\"ReadAuto\" action=\"ReadAuto\" />\n"
"        <separator name=\"sepMenuAuto\" />\n"
"        <menuitem name=\"ReadGabedit\" action=\"ReadGabedit\" />\n"
"        <menuitem name=\"ReadFireFly\" action=\"ReadFireFly\" />\n"
"        <menuitem name=\"ReadDalton\" action=\"ReadDalton\" />\n"
"        <menuitem name=\"ReadGamess\" action=\"ReadGamess\" />\n"
"        <menuitem name=\"ReadGaussian\" action=\"ReadGaussian\" />\n"
"        <menuitem name=\"ReadMolpro\" action=\"ReadMolpro\" />\n"
"        <menuitem name=\"ReadMopac\" action=\"ReadMopac\" />\n"
"        <menuitem name=\"ReadMPQC\" action=\"ReadMPQC\" />\n"
"        <menuitem name=\"ReadQChem\" action=\"ReadQChem\" />\n"
"        <menuitem name=\"ReadMolden\" action=\"ReadMolden\" />\n"
"        <menuitem name=\"ReadXYZ\" action=\"ReadXYZ\" />\n"
"        <menuitem name=\"ReadHIN\" action=\"ReadHIN\" />\n"
"        <menuitem name=\"ReadGamessIRC\" action=\"ReadGamessIRC\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuSave\" />\n"
"      <menuitem name=\"SaveGabedit\" action=\"SaveGabedit\" />\n"
"      <menuitem name=\"SaveMobcal\" action=\"SaveMobcal\" />\n"
"      <separator name=\"sepMenuDelete\" />\n"
"      <menuitem name=\"DeleteGeometry\" action=\"DeleteGeometry\" />\n"
"      <separator name=\"sepMenuCreateGauss\" />\n"
"      <menuitem name=\"CreateGaussInput\" action=\"CreateGaussInput\" />\n"
"      <menuitem name=\"CreateGaussInputLink\" action=\"CreateGaussInputLink\" />\n"
"      <menuitem name=\"CreateGr\" action=\"CreateGr\" />\n"
"      <menuitem name=\"ComputeConformerTpes\" action=\"ComputeConformerTpes\" />\n"
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
void geometryConvergenceDlg()
{
	GtkWidget *Win;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *parentWindow = PrincipalWindow;
	GtkUIManager *manager = NULL;

	if(WinDlg) return;

	initGeometryConvergence();

	Win= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_default_size (GTK_WINDOW(Win),-1,(gint)(ScreenHeightD*0.69));
	gtk_window_set_title(GTK_WINDOW(Win),"Multiple Geometries");
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	WinDlg = Win;

  	add_child(PrincipalWindow,Win,destroyAnimGeomConvDlg,"Mult. Geom.");
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
