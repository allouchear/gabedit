/* MInterfaceBasis.c */
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <gtk/gtk.h>

#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Constants.h"
#include "../Gaussian/GaussGlobal.h"
#include "../Geometry/GeomGlobal.h"
#include "../Utils/Utils.h"
#include "../Molpro/MInterfaceGeom.h"
#include "../Molpro/MInterfaceBasis.h"
#include "../Molpro/MolproLib.h"
#include "../Molpro/MolproBasisLibrary.h"

/* extern MInterfaceBasis.h */
gint NRatoms;
/********************************************************************************/

#define NHBOX 5
/********************************************************************************/
static GtkWidget *listOfAtoms;
static GtkWidget* BoiteVP;
static GtkWidget* BoiteHP[NHBOX];
static GtkWidget *BoiteListeBaseAll[NORB+1] ;
static GtkWidget *Wins = NULL;
static gchar pathSelectedAtom[100] = "-1";
static gint NListeBase;
static gchar *OrbSel;

#ifdef G_OS_WIN32
static gint originbasis = 1; /* 1 -> from file */
#else
static gint originbasis = 0; /* 0 -> libmol */
#endif

gpointer d=NULL;
GtkWidget *LabelInfo;

typedef struct _ProgressData {
    GtkWidget *window;
    GtkWidget *pbar;
    GtkWidget *pbox;
} ProgressData;
ProgressData *pdata;
/********************************************************************************/
static Cbasetot* get_base_total_from_path()
{
	GtkTreeModel *model;
	GtkTreeIter  iter;
 	Cbasetot *base;

	if(atoi(pathSelectedAtom)<0) return NULL;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(listOfAtoms));
	if(!gtk_tree_model_get_iter_from_string (model, &iter, pathSelectedAtom)) return NULL;
	gtk_tree_model_get (model, &iter, ATOMLIST_DATA, &base, -1);

	return base;
}
/********************************************************************************/
static void set_base_total_from_path(Cbasetot *basetot)
{
	GtkTreeModel *model;
	GtkTreeIter  iter;
        GtkListStore *store;

	if(atoi(pathSelectedAtom)<0) return;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(listOfAtoms));
	if(!gtk_tree_model_get_iter_from_string (model, &iter, pathSelectedAtom)) return;
        store = GTK_LIST_STORE (model);
	gtk_list_store_set (store, &iter, ATOMLIST_DATA, (gpointer) basetot, -1);
}
/********************************************************************************/
static void removeSeletedAtom()
{
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	if(atoi(pathSelectedAtom)<0) return;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(listOfAtoms));
	if(!gtk_tree_model_get_iter_from_string (model, &iter, pathSelectedAtom)) return;
        store = GTK_LIST_STORE (model);
	gtk_list_store_remove(store, &iter);
}
/************************************************************************************************************/
static void selectRow(GtkWidget* list, gint row)
{
	GtkTreePath *path;
	gchar* tmp = g_strdup_printf("%d",row);

	path = gtk_tree_path_new_from_string  (tmp);
	g_free(tmp);
	gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (list)), path);
	gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW (list), path, NULL, FALSE,0.5,0.5);
	gtk_tree_path_free(path);
}
/********************************************************************************/
/* Update the value of the progress bar so that we get some movement */
gint progress( gpointer d )
{
    gpointer data = pdata->pbar;
    gdouble new_val;
    gchar *t = NULL;

    if(d) new_val = 0;
    else
    	new_val = gtk_progress_bar_get_fraction( GTK_PROGRESS_BAR(data) ) + 0.1;

    if (new_val > 1) new_val = 1.0;
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (data), new_val);

    if(originbasis == 0)
    	t = g_strdup_printf(_(" Get Basis using libmol : %.0f%%"),new_val*100);
    else
    	t = g_strdup_printf(_(" Get Basis from gabedit basis file : %.0f%%"),new_val*100);

    gtk_label_set_text( GTK_LABEL(LabelInfo),t);
    g_free(t);

    while( gtk_events_pending() ) gtk_main_iteration();

    return TRUE;
} 
/********************************************************************************/
void destroy_progress( GtkWidget  *widget, gpointer data)
{
    gtk_widget_destroy(pdata->pbox);
    g_free(pdata);
    pdata = NULL;
}
/********************************************************************************/
void create_progress_bar(GtkWidget *hbox)
{
    GtkWidget *align;
    GtkWidget *separator;
    GtkWidget *vbox;

    pdata = g_malloc( sizeof(ProgressData) );
  
    pdata->window = Wins;
    gtk_widget_show(hbox);


    pdata->pbox =  vbox = gtk_vbox_new (FALSE, 5);
    gtk_container_set_border_width (GTK_CONTAINER (vbox), 10);
    gtk_container_add (GTK_CONTAINER (hbox), vbox);
    gtk_widget_show(vbox);
  
    align = gtk_alignment_new (0.5, 0.5, 0, 0);
    gtk_box_pack_start (GTK_BOX (vbox), align, FALSE, FALSE, 5);
    gtk_widget_show(align);

    pdata->pbar = gtk_progress_bar_new ();

    gtk_container_add (GTK_CONTAINER (align), pdata->pbar);
    gtk_widget_show_all(pdata->pbar);

    separator = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, FALSE, 0);
    gtk_widget_show(separator);

}
/********************************************************************************/
gchar *get_one_base(gchar **AtomTypeBase,gchar* cont)
{
  gchar *t;
  if(AtomTypeBase[2][strlen(AtomTypeBase[2])-1] =='\n')
	AtomTypeBase[2][strlen(AtomTypeBase[2])-1] = ' ';
  if(cont)
       	t=g_strdup_printf("%s,%s,%s%s\n",AtomTypeBase[0],AtomTypeBase[1],AtomTypeBase[2],cont);
  else
       	t=g_strdup_printf("%s,%s,%s\n",AtomTypeBase[0],AtomTypeBase[1],AtomTypeBase[2]);

  return t;
}
/********************************************************************************/
gint get_num_orb(gchar *nameorb)
{
  gchar *orb = g_strdup(nameorb);
  gint k=-1;
  uppercase(orb);
  if(strcmp(orb,"ECP")==0)
	return -1;
  switch(orb[0])
  {
  case 'S' : k=0;break;
  case 'P' : k=1;break;
  case 'D' : k=2;break;
  default : if(orb[0]<='I')
	     k=(gint)orb[0]-(gint)'F'+3;
  }

  g_free(orb);
  return k;
}
/********************************************************************************/
void DefineTypeAll(Cbasetot *basetot,gchar **AtomTypeBase,gchar* cont)
{
	gint k = -1;
  	gchar *t = NULL;
  	gint i;
  	gint j;
  	if(!strcmp(AtomTypeBase[0],"ECP") )
   	{
      		if(basetot->ECP) g_free(basetot->ECP);
       		basetot->ECP = get_one_base(AtomTypeBase,cont);
       		k=-1;
   	}
   	else
   	{
  		k = get_num_orb(AtomTypeBase[0]);
  		if(k==-1)
			return;
  		if(basetot->Orb[k]) g_free(basetot->Orb[k]);
  		basetot->Orb[k] = get_one_base(AtomTypeBase,cont);
	}
/*  BoiteListeBaseAll[1] <=> s and BoiteListeBaseAll[2] <=> p ...*/
  	if(k+2 >NORB)
		return;

  	for(j=k+2;j<=NORB;j++)
    		if(BoiteListeBaseAll[j]) gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (BoiteListeBaseAll[j])));
  
  	j=k+2;
  	if(BoiteListeBaseAll[j])
  	{
		GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(BoiteListeBaseAll[j]));
		gchar* pathString;
		GtkTreeIter  iter;

		i=0;
		pathString = g_strdup_printf("%d", i);
		while (gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
		{
			gtk_tree_model_get (model, &iter, BASISLIST_NAME, &t, -1);
			i++;
			g_free(pathString);
			pathString = g_strdup_printf("%d", i);
        		if(!strcmp(t,AtomTypeBase[2]))
			{
  				selectRow(BoiteListeBaseAll[j],i-1);
				break;
			}
		}
		g_free(pathString);
  	}
}
/********************************************************************************/
void UnDefineTypeAll(Cbasetot *basetot,gchar **AtomTypeBase)
{
  	gint k;
  	if(!strcmp(AtomTypeBase[0],"ECP") )
   	{
      		if(basetot->ECP) 
         		g_free(basetot->ECP);
       		basetot->ECP = NULL;
       		return;
   	}

  	k = get_num_orb(AtomTypeBase[0]);
  	if(k==-1)
		return;
  	if(basetot->Orb[k]) 
         	g_free(basetot->Orb[k]);
  	basetot->Orb[k] = NULL;
}
/********************************************************************************/
void AfficheTypeAll(Cbasetot *basetot)
{
  if(basetot->ECP != NULL )
      printf("%s",basetot->ECP);
  if(basetot->Orb[0] != NULL )
      printf("%s",basetot->Orb[0]);
  if(basetot->Orb[1] != NULL )
      printf("%s",basetot->Orb[1]);
  if(basetot->Orb[1] != NULL )
      printf("%s",basetot->Orb[1]);
  if(basetot->Orb[2] != NULL )
      printf("%s",basetot->Orb[2]);
  if(basetot->Orb[3] != NULL )
      printf("%s",basetot->Orb[3]);
  if(basetot->Orb[4] != NULL )
      printf("%s",basetot->Orb[4]);
  if(basetot->Orb[5] != NULL )
      printf("%s",basetot->Orb[5]);
  if(basetot->Orb[6] != NULL )
      printf("%s",basetot->Orb[6]);
}
/********************************************************************************/
static void DeselectionBase(Cbase *base)
{
	gchar **AtomTypeBase;
	gchar *temp;
	Cbasetot *basetot;

	AtomTypeBase =g_strsplit (base->TypeBase," ",3);

	temp=AtomTypeBase[0];
	AtomTypeBase[0]=AtomTypeBase[1];
	AtomTypeBase[1]=temp;
	basetot = get_base_total_from_path();
	UnDefineTypeAll(basetot,AtomTypeBase);

	set_base_total_from_path(basetot);

	g_strfreev(AtomTypeBase);
}
/********************************************************************************/
static void resetSelectedBasis(Cbase *base, gboolean cont)
{
	gchar *AtomTypeBase[3];
	gchar *temp;
	Cbasetot *basetot;
	gint i;

	for(i=0;i<3;i++) AtomTypeBase[i] = g_malloc(40*sizeof(gchar));

	sscanf(base->TypeBase,"%s %s %s",AtomTypeBase[0],AtomTypeBase[1],AtomTypeBase[2]);

	temp=AtomTypeBase[0];
	AtomTypeBase[0]=AtomTypeBase[1];
	AtomTypeBase[1]=temp;

	basetot = get_base_total_from_path();
	if(!strstr(AtomTypeBase[0],"ECP"))
	{
        	if(cont) DefineTypeAll(basetot,AtomTypeBase,";c");
		else DefineTypeAll(basetot,AtomTypeBase,NULL);
 	}
	else DefineTypeAll(basetot,AtomTypeBase,NULL);
        

	set_base_total_from_path(basetot);

	for(i=0;i<3;i++) g_free(AtomTypeBase[i]);
}

/********************************************************************************/
static void selectionABasis(GtkTreeSelection *selection, gpointer data)
{
	Cbase *base;
	gboolean cont;

	GtkTreeIter iter;
	GtkTreeModel *model;
	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) return;

	gtk_tree_model_get (model, &iter, BASISLIST_CONTRACTION, &cont, BASISLIST_DATA, &base, -1);
	DeselectionBase(base);
	resetSelectedBasis(base, cont);
}
/********************************************************************************/
gboolean ListeBasis(GtkWidget *listOfBasis,char *NomFichier)
{
	gchar basisName[40];
	FILE *fd;
	Cbase *base;
	gboolean OK = FALSE;
	gchar temp[40];
	gchar t[40];

	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(listOfBasis));
        store = GTK_LIST_STORE (model);

	fd = fopen(NomFichier, "r");
	if(fd!=NULL)
	{
		while(!feof(fd))
		{
			if(!fgets(t,40,fd))break;
			sscanf(t,"%s %s %s", temp,temp, basisName);
			base = g_malloc(sizeof(Cbase));
			base->TypeBase = g_strdup(t);
			gtk_list_store_append(store, &iter);
       			gtk_list_store_set (store, &iter, BASISLIST_NAME, basisName, BASISLIST_CONTRACTION, TRUE, BASISLIST_DATA, (gpointer) base, -1);
		}
		fclose(fd);
		OK = TRUE;
	}
	return OK;
}
/********************************************************************************/
void select_one_list(gint norb)
{
	gchar *basename = NULL;
 	Cbasetot *basetot = get_base_total_from_path();
 	gchar **AtomTypeBase;
	gint j = norb;
	gint i = 0;
 	gchar *t;

		if(basename)
			g_free(basename);
		basename = NULL;
		if(norb==0 && basetot->ECP)
		{
 			AtomTypeBase =g_strsplit (basetot->ECP,",",3);
			basename= g_strdup(AtomTypeBase[2]);
 			g_strfreev(AtomTypeBase);
			if(basename)
			{
 				AtomTypeBase =g_strsplit (basename,";",1);
				g_free(basename);
				basename = NULL;
				basename= g_strdup(AtomTypeBase[0]);
 				g_strfreev(AtomTypeBase);
			}
		}
		else
		if(basetot->Orb[norb-1])
		{
 			AtomTypeBase =g_strsplit (basetot->Orb[norb-1],",",3);
			basename= g_strdup(AtomTypeBase[2]);
 			g_strfreev(AtomTypeBase);
			if(basename)
			{
 				AtomTypeBase =g_strsplit (basename,";",1);
				g_free(basename);
				basename = NULL;
				basename= g_strdup(AtomTypeBase[0]);
 				g_strfreev(AtomTypeBase);
			}
		}
	
		if(basename == NULL)
			return;

		g_strchug(basename);
		g_strchomp(basename);
  		if(j >NORB)
			return;


    		if(BoiteListeBaseAll[j])
		{
			gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (BoiteListeBaseAll[j])));
		}

  		if(BoiteListeBaseAll[j])
  		{
			GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(BoiteListeBaseAll[j]));
			gchar* pathString;
			GtkTreeIter  iter;

			i=0;
			pathString = g_strdup_printf("%d", i);
			while (gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
			{
				gtk_tree_model_get (model, &iter, BASISLIST_NAME, &t, -1);
				i++;
				g_free(pathString);
				pathString = g_strdup_printf("%d", i);
        			if(strcmp(t,basename) == 0)
				{
  					selectRow(BoiteListeBaseAll[j],i-1);
					break;
				}
			}
			g_free(pathString);
  		}
		if(basename) g_free(basename);
  
}
/********************************************************************************/
static void toggledContraction (GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
	GtkTreeModel *model = GTK_TREE_MODEL (data);
	GtkTreeIter iter;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	gboolean cont;
	Cbase *base;

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, BASISLIST_CONTRACTION, &cont, BASISLIST_DATA, &base, -1);

	cont = !cont;
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, BASISLIST_CONTRACTION, cont, -1);

	gtk_tree_path_free (path);
	DeselectionBase(base);
	resetSelectedBasis(base, cont);
}
/********************************************************************************/
GtkWidget *CreateListeBase(char * NomFichier)
{
	GtkWidget *Scr;
  	GtkWidget *window;
  	gchar *Titre[2];
  	gchar *temp;
  	gint nhbox = 1;
  	gboolean CreaHbox = FALSE;
 	GtkWidget *CListeBase = NULL;


  	if (!strcmp(OrbSel,"ECP") ) 
  	{
     		temp =g_strdup("ECP");
     		nhbox = 1;
  	}
   	else
   	{
     		temp = g_strdup_printf(_("    Basis for %s "),OrbSel);
     		if(strstr(OrbSel,"s") || strstr(OrbSel,"p")|| strstr(OrbSel,"d"))
			nhbox = 2;
     		else
     		if(strstr(OrbSel,"f") || strstr(OrbSel,"g")|| strstr(OrbSel,"h"))
			nhbox = 3;
     		else
		nhbox = 4;
   	}
   	if(!BoiteHP[nhbox])
   	{
  		CreaHbox = TRUE;
  		BoiteHP[nhbox] = gtk_hbox_new(TRUE, 0);
  		gtk_box_pack_start (GTK_BOX (BoiteVP), BoiteHP[nhbox], TRUE, TRUE, 2);
   	}
   	Titre[0]=g_strdup(temp);
   	Titre[1]=g_strdup(" Cont. ");
   	g_free(temp);
 
  
  	window = gtk_vbox_new(FALSE, 0);
  	gtk_box_pack_start (GTK_BOX (BoiteHP[nhbox]), window, TRUE, TRUE, 2);
	gtk_widget_show(window);

    	Scr = gtk_scrolled_window_new(NULL, NULL);
    	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(Scr),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    	gtk_container_add(GTK_CONTAINER(window), Scr);

    	if (!strcmp(OrbSel,"ECP") ) 
	{
		GtkListStore *store;
		GtkTreeModel *model;
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;

		store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_POINTER); /* the third column is not visible, used for data */
        	model = GTK_TREE_MODEL (store);

		CListeBase = gtk_tree_view_new_with_model (model);
		gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (CListeBase), TRUE);
		gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (CListeBase), TRUE);
		gtk_tree_view_set_reorderable(GTK_TREE_VIEW (CListeBase), FALSE);

		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, Titre[0]);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", BASISLIST_NAME, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (CListeBase), column);
	}
    	else
    	{
		GtkListStore *store;
		GtkTreeModel *model;
		GtkCellRenderer *renderer;
		GtkTreeViewColumn *column;

		store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_POINTER); /* the third column is not visible, used for data */
        	model = GTK_TREE_MODEL (store);

		CListeBase = gtk_tree_view_new_with_model (model);
		gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (CListeBase), TRUE);
		gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (CListeBase), TRUE);
		gtk_tree_view_set_reorderable(GTK_TREE_VIEW (CListeBase), FALSE);

		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, Titre[0]);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", BASISLIST_NAME, NULL);
		gtk_tree_view_column_set_min_width(column, (gint)(ScreenHeight*0.012));
		gtk_tree_view_append_column (GTK_TREE_VIEW (CListeBase), column);

		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, Titre[1]);
		renderer = gtk_cell_renderer_toggle_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "active", BASISLIST_CONTRACTION, NULL);
		gtk_tree_view_column_set_min_width(column, (gint)(ScreenHeight*0.09));
		gtk_tree_view_append_column (GTK_TREE_VIEW (CListeBase), column);
		g_signal_connect (renderer, "toggled", G_CALLBACK (toggledContraction), model);

    	}
    	gtk_widget_set_size_request(listOfAtoms, (gint)(ScreenHeight*0.136),(gint)(ScreenHeight*0.06)); 
    	gtk_container_add(GTK_CONTAINER(Scr), CListeBase);

    	if(!ListeBasis(CListeBase,NomFichier) && CreaHbox ) gtk_widget_destroy(BoiteHP[nhbox]);
   	return (CListeBase);
}
/********************************************************************************/
gchar* get_error_message()
{
 gchar *errfile= g_strdup_printf("%s%stmp%serrfile",gabedit_directory(), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S);
 gchar *terr = NULL;
 gchar *dump = NULL;
 gint taille = BSIZE;
 gchar t[BSIZE];
 FILE *fd;
 fd = fopen(errfile, "r");
 if(fd)
 {
  	while(!feof(fd))
  	{
    		if(!fgets(t,taille, fd))
			break;
                dump = terr;
		if(!terr)
			terr = g_strdup_printf("%s",t);
		else
		{
			terr = g_strdup_printf("%s%s",terr,t);
			g_free(dump);
		}
  	}
 	fclose(fd);
#ifdef G_OS_WIN32
	unlink (errfile);
#else
 	dump = g_strdup_printf("rm %s&",errfile);
 	{int ierr = system(dump);}
	g_free(dump);
#endif


 }
 g_free(errfile);
 return terr;
}
/********************************************************************************/
gint create_one_liste (GtkWidget *widget, gpointer data,gboolean fromfile)
{
 	gint OK = 1;
 	gchar *t;
 	gchar *terr;
 	FILE *fd;
 	Cbasetot *basetot;
 	gchar *temp=NULL;
 	gchar *outfile= g_strdup_printf("%s%stmp%soutfile",gabedit_directory(), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S);
 	gchar *errfile= g_strdup_printf("%s%stmp%serrfile",gabedit_directory(), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S);
 	gint k;

 	OrbSel = (gchar *) data;
 	basetot = get_base_total_from_path();

 	if(!fromfile)
	{
 		temp = g_strdup_printf("sh -c 'libmol -e %s -t %s>%s 2>%s'",basetot->Name,(gchar*)data,outfile,errfile);
 		{int ierr = system(temp);}
 	}
 	else
 	{
		temp=g_strdup(" ");
  		get_molpro_basis_list(basetot->Name,(gchar*)data,outfile,errfile);
	}

	t=g_malloc(40);
 	terr = NULL;
 	fd = fopen(outfile, "r");
 	if(fd)
 	{
 		terr=fgets(t,40,fd);
 		fclose(fd);
 	}
 	if(terr!=NULL)
 	{
     		NListeBase++;
     		k = get_num_orb(OrbSel);
			
     		if(k<0)
     			BoiteListeBaseAll[0] = CreateListeBase(outfile);
     		else
     			BoiteListeBaseAll[k+1] = CreateListeBase(outfile);		
	} 
 	else
    		OK = 0;
	
 	fd = fopen(outfile, "r");
 	if(fd)
 	{
 		fclose(fd);
#ifdef G_OS_WIN32
		unlink (outfile);
#else
		if(temp)
     		g_free(temp);
     	temp = g_strdup_printf("rm %s",outfile);
 		{int ierr = system(temp);}
#endif
 	}

	g_free(t);
	g_free(temp);
	g_free(errfile);
	g_free(outfile);
 	return OK;
}
/********************************************************************************/
gchar *CreateOrbitalTypeAll(gboolean fromfile)
{
  gint n=0;
  gchar *terr = NULL;
  gchar *dump = NULL;
	
  progress(&n);
  gtk_widget_show (BoiteHP[0]);
  n += create_one_liste(NULL,(gpointer)"ECP",fromfile);
  progress( NULL);
  n += create_one_liste(NULL,(gpointer)"s",fromfile);
  progress( NULL);
  n += create_one_liste(NULL,(gpointer)"p",fromfile );
  progress( NULL);
  n += create_one_liste(NULL,(gpointer)"d",fromfile );
  progress( NULL);
  n += create_one_liste(NULL,(gpointer)"f",fromfile );
  progress( NULL);
  n += create_one_liste(NULL,(gpointer)"g",fromfile );
  progress( NULL);
  n += create_one_liste(NULL,(gpointer)"h",fromfile );
  progress( NULL);
  n += create_one_liste(NULL,(gpointer)"i",fromfile );
  progress( NULL);
  if( n == 0)
  {
         terr = get_error_message();
	 if(!terr)
	 {
    		if(originbasis == 0)
	 		terr = g_strdup(_(" Sorry,\nNo basis available for this atom,\nOr libmol is not installed in your system."));
		else
	 		terr = g_strdup(_(" Sorry,\nNo basis available for this atom,\nOr gabedit molpro basis file is corrupted."));
	  }
         else
	 {
		dump = terr;
	 	terr = g_strdup_printf(_("Error : %s"),dump);
		g_free(dump);
	 }
  }
  progress( NULL);

  Waiting(0.2);
  gtk_widget_hide (BoiteHP[0]);
  return terr;
}
/********************************************************************************/
void RedefineListeBasis()
{
	guint Ncenters;
	gboolean OK;
	guint i,j;
	Cbasetot *basetot;
	gchar *texts[1]={NULL};
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(listOfAtoms));
        store = GTK_LIST_STORE (model);


  	if(MethodeGeom == GEOM_IS_XYZ) Ncenters = NcentersXYZ;
  	else Ncenters = NcentersZmat;

	for (i=0;i<Ncenters;i++)
  	{
  		OK=TRUE;
  		if(MethodeGeom == GEOM_IS_XYZ)
  		{
   			texts[0]=g_strdup(GeomXYZ[i].Symb);
   			for (j=0;j<i;j++)
    			if( !strcmp(GeomXYZ[i].Symb,GeomXYZ[j].Symb)  )
    			{
				OK=FALSE;
      				break;
    			}
  		}
  		else
     		if(MethodeGeom == GEOM_IS_ZMAT)
  		{
			texts[0]=g_strdup(Geom[i].Symb);
    			for (j=0;j<i;j++)
			if( !strcmp(Geom[i].Symb,Geom[j].Symb)  )
			{
				OK=FALSE;
				break;
			}
  		}
  		if(!OK)continue;
    	
    		if(*texts[0] != 'X' )
    		{
    			NRatoms++;
    			basetot = g_malloc(sizeof(Cbasetot));
    			basetot->Name = g_strdup(texts[0]);
    			basetot->ECP = NULL;
    			for(j=0;j<NORB;j++) basetot->Orb[j] = NULL;
			gtk_list_store_append(store, &iter);
       			gtk_list_store_set (store, &iter, ATOMLIST_SYMBOL, texts[0], ATOMLIST_DATA, (gpointer) basetot, -1);
   		}
  	}
}
/********************************************************************************/
static void DeselectionAnAtom()
{
	guint i;
    	if(NListeBase != -1 )
    	{
    		for (i=0;(gint)i<=NORB;i++)
        	{
          		if(BoiteListeBaseAll[i]!=NULL)
				gtk_widget_hide_on_delete (BoiteListeBaseAll[i]);
          		BoiteListeBaseAll[i] = NULL;
        	}
    		NListeBase=-1;
    	}
    	for(i=1;i<NHBOX;i++)
    	{
  		if(BoiteHP[i])
			gtk_widget_destroy(BoiteHP[i]);
  		BoiteHP[i] = NULL;
    	}

    	gtk_label_set_text( GTK_LABEL(LabelInfo),_("Please select a atom"));

}
/********************************************************************************/
static void selectionAnAtom(GtkTreeSelection *selection, gpointer data)
{
	gchar *terr = NULL;
  	gint i;

	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar* pathString;
	GtkTreeSelection *select;

	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) return;
	pathString =gtk_tree_model_get_string_from_iter(model, &iter);

  	sprintf(pathSelectedAtom,"%s", pathString);

	DeselectionAnAtom();

  	if(!BoiteHP[0])
  	{
  		BoiteHP[0] = gtk_hbox_new(FALSE, 0);
  		gtk_box_pack_start (GTK_BOX (BoiteVP), BoiteHP[0], FALSE, FALSE, 2);
  		create_progress_bar(BoiteHP[0]);
  	}

	if(originbasis == 0)
	{
  		terr = CreateOrbitalTypeAll(FALSE);
		if(terr)
		{
			originbasis = 1;
  			gtk_label_set_text( GTK_LABEL(LabelInfo),terr);
    			while( gtk_events_pending() )
          			gtk_main_iteration();
			Waiting(1.0);
			g_free(terr);
  			terr = CreateOrbitalTypeAll(TRUE);
		}
	}
	else
  		terr = CreateOrbitalTypeAll(TRUE);

  	for(i=1;i<NHBOX;i++)
  		if(BoiteHP[i])
			gtk_widget_show_all(BoiteHP[i]);
	
    	for (i=0;i<=NORB;i++)
	{
		if(BoiteListeBaseAll[i]!=NULL)
		{
			select_one_list(i);
			select = gtk_tree_view_get_selection (GTK_TREE_VIEW (BoiteListeBaseAll[i]));
			gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
			g_signal_connect (G_OBJECT (select), "changed", G_CALLBACK (selectionABasis), NULL);
		}
	}

  	if(terr) gtk_label_set_text( GTK_LABEL(LabelInfo),terr);
  	else gtk_label_set_text( GTK_LABEL(LabelInfo),_("Please select your basis"));

}
/********************************************************************************/
static void DelAtomList(GtkWidget *w,gpointer data)
{
	NRatoms--;	
	removeSeletedAtom();
	sprintf(pathSelectedAtom,"-1");
	if(NRatoms>0) selectRow(listOfAtoms, 0);
	else DeselectionAnAtom();
}
/********************************************************************************/
static void DialogueDelete(GtkWidget *w)
{
  GtkWidget *Dialogue;
  GtkWidget *Label;
  GtkWidget *Bouton;
  GtkWidget *frame, *vboxframe;


  if (NRatoms <1)
  {
    Message(_(" No Atom to delete !"),_("Warning"),TRUE);
    return ;
  }
    if (atoi(pathSelectedAtom) <0)
  {
    Message(_(" Please Select Your Atom to delete!"),_("Warning"),TRUE);
    return ;
  }


  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),_("Delete Atom"));
  gtk_window_set_modal (GTK_WINDOW (Dialogue), TRUE);
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);

  Label = gtk_label_new(_("Are you sure to delete \nthe selected atom? \n"));
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  gtk_box_pack_start(GTK_BOX(vboxframe), Label,TRUE,TRUE,0);
 
  gtk_widget_realize(Dialogue);

  Bouton = create_button(Dialogue,"No");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);

  Bouton = create_button(Dialogue,"Yes");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked",(GCallback)DelAtomList, NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)gtk_widget_destroy, GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
static void SetAtom(GtkWidget *w,gpointer data)
{
   gchar *texts[1];
   Cbasetot *basetot;
   GtkTreeModel *model;
   GtkListStore *store;
   GtkTreeIter  iter;

   model = gtk_tree_view_get_model(GTK_TREE_VIEW(listOfAtoms));
   store = GTK_LIST_STORE (model);

    texts[0]=g_strdup((gchar*)data);
    NRatoms++;

    basetot = g_malloc(sizeof(Cbasetot));
    basetot->Name = g_strdup(texts[0]);
    basetot->ECP = NULL;
    basetot->Orb[0] = NULL;
    basetot->Orb[1] = NULL;
    basetot->Orb[2] = NULL;
    basetot->Orb[3] = NULL;
    basetot->Orb[4] = NULL;
    basetot->Orb[5] = NULL;
    basetot->Orb[6] = NULL;
   gtk_list_store_append(store, &iter);
   gtk_list_store_set (store, &iter, ATOMLIST_SYMBOL, texts[0], ATOMLIST_DATA, (gpointer) basetot, -1);
   selectRow(listOfAtoms, NRatoms-1);

}
/********************************************************************************/
static void DialogueAdd(GtkWidget *w,gpointer data)
{
	GtkWidget* Table;
	GtkWidget* button;
	GtkWidget* FenetreTable;
	GtkWidget* frame;
	GtkWidget* vbox;
	GtkWidget* hbox;
	guint i;
	guint j;
        GtkStyle *button_style;
        GtkStyle *style;


	gchar*** Symb = get_periodic_table();

  FenetreTable = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(FenetreTable),TRUE);
  gtk_window_set_title(GTK_WINDOW(FenetreTable),_("Select your atom"));
  gtk_window_set_position(GTK_WINDOW(FenetreTable),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(FenetreTable),GTK_WINDOW(Wins));
  gtk_window_set_modal (GTK_WINDOW (FenetreTable), TRUE);
  gtk_window_set_default_size (GTK_WINDOW(FenetreTable),(gint)(ScreenWidth*0.5),(gint)(ScreenHeight*0.4));

  add_child(Wins,FenetreTable,gtk_widget_destroy,_(" Selec. atom "));
  g_signal_connect(G_OBJECT(FenetreTable),"delete_event",(GCallback)delete_child,NULL);

  button_style = gtk_widget_get_style(FenetreTable); 

  vbox = gtk_vbox_new (FALSE, 5);
  gtk_container_add(GTK_CONTAINER(FenetreTable),vbox);
  
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

 
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
  gtk_container_add(GTK_CONTAINER(hbox),frame);  
  gtk_widget_show (frame);


  Table = gtk_table_new(PERIODIC_TABLE_N_ROWS-1,PERIODIC_TABLE_N_COLUMNS,TRUE);
  gtk_container_add(GTK_CONTAINER(frame),Table);
  
  for ( i = 0;i<PERIODIC_TABLE_N_ROWS-1;i++)
	  for ( j = 0;j<PERIODIC_TABLE_N_COLUMNS;j++)
  {
	  if(strcmp(Symb[j][i],"00"))
	  {
	  button = gtk_button_new_with_label(Symb[j][i]);
          style=set_button_style(button_style,button,Symb[j][i]);
          g_signal_connect(G_OBJECT(button), "clicked",(GCallback)SetAtom,(gpointer )Symb[j][i]);
	  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(FenetreTable));

	  gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	  }

  }

  hbox = create_hbox(vbox);
  gtk_widget_realize(FenetreTable);
  button = create_button(FenetreTable,_("Cancel"));
  gtk_box_pack_start (GTK_BOX (hbox), button, FALSE, FALSE, 0);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(FenetreTable));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show(button);
  
  gtk_widget_show_all(FenetreTable);
  
}
/********************************************************************************/
void ButtonBar(GtkWidget *BoiteV)
{
  GtkWidget *hbox;
  GtkWidget *button;

  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (BoiteV), hbox, FALSE, FALSE, 5);

  button = gtk_button_new_with_label (_(" New Atom "));

  g_signal_connect(G_OBJECT(button), "clicked",(GCallback)DialogueAdd,Wins);
 
  gtk_box_pack_start (GTK_BOX(hbox), button, FALSE, FALSE, 5);
  gtk_widget_show (button);


  button = gtk_button_new_with_label (_(" Delete Atom "));

  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)DialogueDelete,NULL);

  gtk_box_pack_start (GTK_BOX(hbox), button, FALSE, FALSE, 5);


}
/********************************************************************************/
void AjoutePageBasis(GtkWidget *Win,GtkWidget *NoteBook,BaseS *base)
{
	GtkWidget *Frame;
	GtkWidget *LabelOnglet;
	GtkWidget *LabelMenu;
	GtkWidget *BoiteV;
	GtkWidget *HPaned;
	GtkWidget *Scr;
	GtkWidget *window1;
	GtkWidget *hbox3;
	gchar *titleForAtoms[] = {_("Atoms")};
	gint i;

	GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;

	Wins = Win;
	NRatoms=0;
	NListeBase=-1;
	Frame = gtk_frame_new(NULL);
	gtk_widget_set_size_request(GTK_WIDGET(Frame), (gint)(ScreenHeight*0.6),(gint)(ScreenHeight*0.4)); 
	gtk_frame_set_shadow_type( GTK_FRAME(Frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);

	LabelOnglet = gtk_label_new(_("Basis"));
	LabelMenu = gtk_label_new(_("Basis"));
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),Frame,LabelOnglet, LabelMenu);

	BoiteV = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(Frame), BoiteV);

	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox3);
	gtk_box_pack_start (GTK_BOX (BoiteV), hbox3, FALSE, FALSE, 0);

	HPaned = gtk_hpaned_new();
	gtk_box_pack_start (GTK_BOX (BoiteV), HPaned,TRUE,TRUE, 0);

	BoiteVP = gtk_vbox_new(FALSE, 0);
	gtk_paned_add2(GTK_PANED(HPaned), BoiteVP);

	window1 = BoiteVP; 
	hbox3 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox3);
	gtk_box_pack_start (GTK_BOX (BoiteVP), hbox3, FALSE, FALSE, 0);

	LabelInfo = gtk_label_new (_("Please select a atom "));
	gtk_widget_show (LabelInfo);
	gtk_box_pack_start (GTK_BOX (BoiteVP), LabelInfo, FALSE, FALSE, 0);

	Scr = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(Scr),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

	gtk_paned_add1(GTK_PANED(HPaned), Scr);
	  
	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER); /* the second column is not visible, used for data */
        model = GTK_TREE_MODEL (store);

	listOfAtoms = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (listOfAtoms), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (listOfAtoms), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (listOfAtoms), FALSE);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, titleForAtoms[0]);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_attributes (column, renderer, "text", ATOMLIST_SYMBOL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (listOfAtoms), column);


	RedefineListeBasis();


	gtk_widget_set_size_request(listOfAtoms,(gint)(ScreenHeight*0.136),(gint)(ScreenHeight*0.05)); 
	gtk_container_add(GTK_CONTAINER(Scr), listOfAtoms);

	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (listOfAtoms));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (select), "changed", G_CALLBACK (selectionAnAtom), NULL);

	base->listOfAtoms = listOfAtoms;
	base->NRatoms = NRatoms;
	
	 ButtonBar(BoiteV);
	 for(i=0;i<NHBOX;i++) BoiteHP[i] = NULL;
	 for(i=0;i<NORB+1;i++) BoiteListeBaseAll[i] = NULL ;
}
/********************************************************************************/

