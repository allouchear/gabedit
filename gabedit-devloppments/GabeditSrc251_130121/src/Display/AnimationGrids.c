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
#include "../Display/AnimationGrids.h"
#include "../Display/GridCube.h"
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

/* extern AnimationGrids.h */
AnimationGrids animationGrids;
/*****************************************/

static	GtkWidget *WinDlg = NULL;
static	GtkWidget *EntryVelocity = NULL;
static	GtkWidget *EntryIsoValue = NULL;
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


/********************************************************************************/
static void animate();
static void rafreshList();
static void stopAnimation(GtkWidget *win, gpointer data);
static void playAnimation(GtkWidget *win, gpointer data);
static gboolean set_grid(gint k);

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
void initAnimationGrids()
{
	animationGrids.typeOfFile = GABEDIT_TYPEFILE_UNKNOWN;
	animationGrids.numberOfFiles = 0;
	animationGrids.geometries = NULL;
	animationGrids.fileNames = NULL;
	animationGrids.grids = NULL;
	rowSelected = -1;
	animationGrids.velocity = 0.0;
	animationGrids.isovalue = 0.1;
}
/********************************************************************************/
void freeAnimationGrids()
{
	static gboolean begin = TRUE;

	if(begin)
	{
		begin  = FALSE;
		initAnimationGrids();
		return;
	}
	if(animationGrids.geometries)
	{
		gint i;
		GeomGrid* geometries = animationGrids.geometries;
		for(i=0;i<animationGrids.numberOfFiles;i++)
		{
			if(geometries[i].listOfAtoms) g_free(geometries[i].listOfAtoms);
		}
		g_free(geometries);
	}
	if(animationGrids.grids)
	{
		gint i;
		for(i=0;i<animationGrids.numberOfFiles;i++)
			if(animationGrids.grids[i]) free_grid(animationGrids.grids[i]);
		g_free(animationGrids.grids);
	}
	if(animationGrids.fileNames)
	{
		gint i;
		for(i=0;i<animationGrids.numberOfFiles;i++)
			if(animationGrids.fileNames[i]) g_free(animationGrids.fileNames[i]);
		g_free(animationGrids.fileNames);
	}

	initAnimationGrids();
}
/********************************************************************************/
static void delete_one_grid()
{

	gint j;
	gint k = rowSelected;

	if(k<0 || k >= animationGrids.numberOfFiles) return;
	if(!animationGrids.geometries) return;
	if(animationGrids.numberOfFiles==1)
	{
		freeAnimationGrids();
		rafreshList();
		return;
	}

	if(animationGrids.geometries)
			if(animationGrids.geometries[k].listOfAtoms) g_free(animationGrids.geometries[k].listOfAtoms);
	if(animationGrids.grids) if(animationGrids.grids[k]) free_grid(animationGrids.grids[k]);
	if(animationGrids.fileNames) if(animationGrids.fileNames[k]) g_free(animationGrids.fileNames[k]);
	for(j=k;j<animationGrids.numberOfFiles-1;j++)
	{
		if(animationGrids.fileNames) animationGrids.fileNames[j] = animationGrids.fileNames[j+1];
		if(animationGrids.geometries) animationGrids.geometries[j] = animationGrids.geometries[j+1];
		if(animationGrids.grids) animationGrids.grids[j] = animationGrids.grids[j+1];
	}
	animationGrids.numberOfFiles--;
	if(animationGrids.geometries)
		animationGrids.geometries = g_realloc(animationGrids.geometries, animationGrids.numberOfFiles*sizeof(GeomGrid));
	if(animationGrids.grids)
		animationGrids.grids = g_realloc(animationGrids.grids, animationGrids.numberOfFiles*sizeof(GeomGrid));
	if(animationGrids.fileNames)
		animationGrids.fileNames = g_realloc(animationGrids.fileNames, animationGrids.numberOfFiles*sizeof(gchar*));
	rafreshList();

	return;
}
/********************************************************************************/
static gboolean set_geometry(gint k)
{
	AtomGrid* listOfAtoms = NULL;
	gint nAtoms = 0;
	gint j;

	if(k<0 || k >= animationGrids.numberOfFiles) return FALSE;
	if(!GeomOrb) return FALSE;
	if(nCenters<1) return FALSE;

	nAtoms = nCenters;
	listOfAtoms = g_malloc(nAtoms*sizeof(AtomGrid));
	for(j=0;j<nAtoms;j++)
	{
    		sprintf(listOfAtoms[j].symbol,"%s",GeomOrb[j].Symb);
    		listOfAtoms[j].C[0] = GeomOrb[j].C[0];
    		listOfAtoms[j].C[1] = GeomOrb[j].C[1];
    		listOfAtoms[j].C[2] = GeomOrb[j].C[2];
    		sprintf(listOfAtoms[j].mmType,"%s",listOfAtoms[j].symbol);
    		sprintf(listOfAtoms[j].pdbType,"%s",listOfAtoms[j].symbol);
		listOfAtoms[j].partialCharge = GeomOrb[j].partialCharge;
		listOfAtoms[j].variable = GeomOrb[j].variable;
		listOfAtoms[j].nuclearCharge = GeomOrb[j].nuclearCharge;
	}
	return TRUE;
}
/*************************************************************************************************************/
static void read_gabedit_files(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;
	GSList* lists = NULL;
	GSList* cur = NULL;

	if(response_id != GTK_RESPONSE_OK) return;

	lists = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(SelecFile));

	stopAnimation(NULL, NULL);

	freeAnimationGrids();
	cur = lists;
	animationGrids.numberOfFiles = 0;
	while(cur != NULL)
	{
		animationGrids.numberOfFiles++;
		cur = cur->next;
	}
	if(animationGrids.numberOfFiles>0)
	{
		gint k = 0;
		animationGrids.geometries = g_malloc(animationGrids.numberOfFiles*sizeof(GeomGrid));
		animationGrids.fileNames = g_malloc(animationGrids.numberOfFiles*sizeof(gchar*));
		animationGrids.grids = g_malloc(animationGrids.numberOfFiles*sizeof(Grid*));
		for(k=0;k<animationGrids.numberOfFiles;k++)
		{
			animationGrids.fileNames[k] = NULL;
			animationGrids.grids[k] = NULL;
			animationGrids.geometries[k].numberOfAtoms = 0;
			animationGrids.geometries[k].listOfAtoms = NULL;
		}
		k = 0;
		cur = lists;
		while(cur != NULL)
		{
			fileName = (gchar*)(cur->data);

			if(read_gauss_molpro_cube_orbitals_file(fileName,0,0,GABEDIT_TYPEFILE_CUBEGABEDIT, FALSE))
			{
				set_geometry(k);
				animationGrids.fileNames[k] = g_strdup(fileName);
				animationGrids.grids[k] = copyGrid(grid);
			       	k++;
			}
			cur = cur->next;
		}
		if(k<=0)
		{
			freeAnimationGrids();
		}
		else
		if(k!=animationGrids.numberOfFiles)
		{
			animationGrids.numberOfFiles = k;
			animationGrids.geometries = g_realloc(animationGrids.geometries, animationGrids.numberOfFiles*sizeof(GeomGrid));
			animationGrids.fileNames = g_realloc(animationGrids.fileNames, animationGrids.numberOfFiles*sizeof(gchar*));
			animationGrids.grids = g_realloc(animationGrids.grids, animationGrids.numberOfFiles*sizeof(Grid*));
		}
		if(k>0)
		{
			animationGrids.typeOfFile = GABEDIT_TYPEFILE_CUBEDX;
		}
	}
  	rafreshList();
	
}
/*************************************************************************************************************/
static void read_dx_files(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;
	GSList* lists = NULL;
	GSList* cur = NULL;

	if(response_id != GTK_RESPONSE_OK) return;

	gtk_widget_hide(GTK_WIDGET(SelecFile));
	lists = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(SelecFile));

	stopAnimation(NULL, NULL);

	freeAnimationGrids();
	cur = lists;
	animationGrids.numberOfFiles = 0;
	while(cur != NULL)
	{
		animationGrids.numberOfFiles++;
		cur = cur->next;
	}
	if(animationGrids.numberOfFiles>0)
	{
		gint k = 0;
		animationGrids.geometries = g_malloc(animationGrids.numberOfFiles*sizeof(GeomGrid));
		animationGrids.fileNames = g_malloc(animationGrids.numberOfFiles*sizeof(gchar*));
		animationGrids.grids = g_malloc(animationGrids.numberOfFiles*sizeof(Grid*));
		for(k=0;k<animationGrids.numberOfFiles;k++)
		{
			animationGrids.fileNames[k] = NULL;
			animationGrids.grids[k] = NULL;
			animationGrids.geometries[k].numberOfAtoms = 0;
			animationGrids.geometries[k].listOfAtoms = NULL;
		}
		k = 0;
		cur = lists;
		while(cur != NULL)
		{
			fileName = (gchar*)(cur->data);

			if(read_dx_grid_file(fileName, FALSE))
			{
				animationGrids.fileNames[k] = g_strdup(fileName);
				set_geometry(k);
				animationGrids.grids[k] = copyGrid(grid);
			       	k++;
			}
			cur = cur->next;
		}
		if(k<=0)
		{
			freeAnimationGrids();
		}
		else
		if(k!=animationGrids.numberOfFiles)
		{
			animationGrids.numberOfFiles = k;
			animationGrids.geometries = g_realloc(animationGrids.geometries,animationGrids.numberOfFiles*sizeof(GeomGrid));
			animationGrids.fileNames = g_realloc(animationGrids.fileNames, animationGrids.numberOfFiles*sizeof(gchar*));
			animationGrids.grids = g_realloc(animationGrids.grids, animationGrids.numberOfFiles*sizeof(Grid*));
		}
		if(k>0)
		{
			animationGrids.typeOfFile = GABEDIT_TYPEFILE_CUBEDX;
		}
	}
  	rafreshList();
	
}
/*************************************************************************/
static void read_files(GabeditFileChooser *selecFile, gint response_id)
{
	gchar *fileName;
	GabEditTypeFile fileType = GABEDIT_TYPEFILE_UNKNOWN;

	GSList* lists = NULL;


	if(response_id != GTK_RESPONSE_OK) return;

	lists = gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(selecFile));
	if(!lists)
	{
		Message( _("Sorry, I cannot no selected files\n"),_("Error"),TRUE);
		return;
	}

 	fileName = (gchar*)lists->data;

	gtk_widget_hide(GTK_WIDGET(selecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	fileType = get_type_file(fileName);
	if(fileType == GABEDIT_TYPEFILE_CUBEGABEDIT) read_gabedit_files(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_CUBEDX) read_dx_files(selecFile, response_id);
	else 
	{
		Message( _("Sorry, I cannot find the type of your file\n") ,_("Error"),TRUE);
	}
}
/********************************************************************************/
static void read_gabedit_files_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gabedit_files,
			_("Read grids from a Gabedit file"),
			GABEDIT_TYPEFILE_CUBEGABEDIT,GABEDIT_TYPEWIN_ORB);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_dx_files_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_dx_files, _("Read grids from dx files"), GABEDIT_TYPEFILE_CUBEDX,GABEDIT_TYPEWIN_ORB);
	
	gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(filesel),TRUE);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_files_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_files, _("Read grids from a file"), GABEDIT_TYPEFILE_UNKNOWN,GABEDIT_TYPEWIN_ORB);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean set_grid(gint k)
{
	AtomGrid* listOfAtoms = NULL;
	gint nAtoms = 0;
	gint j;

	if(k<0 || k >= animationGrids.numberOfFiles) return FALSE;
	if(!animationGrids.grids[k]) return FALSE;

	if(GeomOrb)
	{
		free_atomic_orbitals();
		for(j=0;j<nCenters;j++) if(GeomOrb[j].Symb) g_free(GeomOrb[j].Symb);
		g_free(GeomOrb);
		GeomOrb = NULL;
	}
        free_grid_all();
        /* free_iso_all();*/

	nAtoms = animationGrids.geometries[k].numberOfAtoms;
	listOfAtoms = animationGrids.geometries[k].listOfAtoms;

	if(nAtoms>0) GeomOrb=g_malloc(nAtoms*sizeof(TypeGeomOrb));
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
	if(nAtoms>0) init_atomic_orbitals();
	init_dipole();
	buildBondsOrb();
	RebuildGeomD = TRUE;

	grid = copyGrid(animationGrids.grids[k]);

	Define_Iso(animationGrids.isovalue);

	glarea_rafresh(GLArea);

	return TRUE;
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
	freeAnimationGrids();
}
/********************************************************************************/
static void destroyAnimAnimGridDlg(GtkWidget *win)
{
	createFilm = FALSE;
	numFileFilm = 0;
	stopAnimation(NULL, NULL);
	gtk_widget_destroy(WinDlg);
	WinDlg = NULL;
	rowSelected = -1;
	freeAnimationGrids();

}
/**********************************************************************************/
static void set_sensitive_option(GtkUIManager *manager, gchar* path)
{
	GtkWidget *wid = gtk_ui_manager_get_widget (manager, path);
	gboolean sensitive = TRUE;
  	if(animationGrids.numberOfFiles<1) sensitive = FALSE;
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, sensitive);
}
/**********************************************************************************/
static gboolean show_menu_popup(GtkUIManager *manager, guint button, guint32 time)
{
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuAnimGrid");
	if (GTK_IS_MENU (menu)) 
	{
		set_sensitive_option(manager,"/MenuAnimGrid/DeleteGrid");
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
	if(row>=0) set_grid(rowSelected);

  	if (event->type == GDK_BUTTON_PRESS && ((GdkEventButton *) event)->button == 3)
	{
		GdkEventButton *bevent = (GdkEventButton *) event;
		GtkUIManager *manager = GTK_UI_MANAGER(user_data);
		show_menu_popup(manager, bevent->button, bevent->time);
	}
}
/********************************************************************************/
static void reset_parameters(GtkWidget *win, gpointer data)
{
	gdouble velo = atof(gtk_entry_get_text(GTK_ENTRY(EntryVelocity)));
	gdouble isovalue = atof(gtk_entry_get_text(GTK_ENTRY(EntryIsoValue)));
	gchar* t;

	if(velo<0)
	{

		velo = -velo;
		t = g_strdup_printf("%lf",velo);
		gtk_entry_set_text(GTK_ENTRY(EntryVelocity),t);
		g_free(t);
	}

	animationGrids.velocity = velo;
	animationGrids.isovalue = isovalue;
	if(!play) stopAnimation(NULL,NULL);

}
/**********************************************************************************/
static void rafreshList()
{
	gint i;
	gint k;
        GtkTreeIter iter;
	GtkTreeModel *model = NULL;
        GtkTreeStore *store = NULL;
	gchar *texts[1];
	gchar* titles[]={"Comments"};
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	model = gtk_tree_view_get_model(treeView);
        store = GTK_TREE_STORE (model);
	gtk_tree_store_clear(store);
        model = GTK_TREE_MODEL (store);

	do
	{
		column = gtk_tree_view_get_column(treeView, 0);
		if(column) gtk_tree_view_remove_column(treeView, column);
	}while(column != NULL);

	for(k=0;k<1;k++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, titles[k]);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		gtk_tree_view_column_set_resizable(column,TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", k, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (treeView), column);
	}

  
	for(i=0;i<animationGrids.numberOfFiles;i++)
	{
		for(k=0;k<1;k++)
		{
			texts[k] = g_strdup_printf("%s",animationGrids.fileNames[i]);
        		gtk_tree_store_append (store, &iter, NULL);
			gtk_tree_store_set (store, &iter, k, texts[k], -1);
			g_free(texts[k]);
		}
	}

	if(animationGrids.numberOfFiles>0)
	{
		GtkTreePath *path;
		rowSelected = 0;
		path = gtk_tree_path_new_from_string  ("0");
		gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeView)), path);
		gtk_tree_view_scroll_to_cell(GTK_TREE_VIEW (treeView), path, NULL, FALSE,0.5,0.5);
		gtk_tree_path_free(path);
		set_grid(rowSelected);
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
	sprintf(t,"%lf",animationGrids.velocity);
	gtk_entry_set_text(GTK_ENTRY(EntryVelocity),t);

	i++;
	add_label_table(table,_(" IsoValue "),(gushort)i,0);
	add_label_table(table," : ",(gushort)i,1); 
	EntryIsoValue = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),EntryIsoValue,2,2+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);
	gtk_editable_set_editable((GtkEditable*) EntryIsoValue,TRUE);
	sprintf(t,"%lf",animationGrids.isovalue);
	gtk_entry_set_text(GTK_ENTRY(EntryIsoValue),t);

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
	gint widths[]={10};
	gchar* titles[]={"Comments"};
	gint Factor=7;
	gint len = 1;


	for(i=0;i<len;i++) widall+=widths[i];

	widall=widall*Factor+40;

	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeightD*0.53));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 2);

	store = gtk_tree_store_new (len,G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
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
	if(animationGrids.numberOfFiles<1) play = FALSE;

	while(play)
	{
		while( gtk_events_pending() ) gtk_main_iteration();

		m += step;
		if(m>=animationGrids.numberOfFiles)
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
		set_grid(rowSelected);

		createImagesFile();
		Waiting(animationGrids.velocity);
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
		" You can read grids from :\n"
	        "     * Gabedit cube input files.\n"
	        "     * DX files.\n"
		)
		 );
	win = Message(temp," Info ",FALSE);
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
}
/********************************************************************************/
static void help_animated_file()
{
	GtkWidget* win;
	gchar* temp = NULL;
	temp = g_strdup(
		_(
		" For create an animated file :\n"
		" ============================\n"
	        "   1) Read grids from Gabedit or from DX files.\n"
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
		if(GTK_IS_UI_MANAGER(manager)) set_sensitive_option(manager,"/MenuBar/File/DeleteGrid");
	}
	else if(!strcmp(name, "ReadAuto")) read_files_dlg();
	else if(!strcmp(name, "ReadGabedit")) read_gabedit_files_dlg();
	else if(!strcmp(name, "ReadDX")) read_dx_files_dlg();
	else if(!strcmp(name, "DeleteGrid")) delete_one_grid();
	else if(!strcmp(name, "Close")) destroyDlg(NULL);
	else if(!strcmp(name, "HelpSupportedFormat")) help_supported_format();
	else if(!strcmp(name, "HelpAnimation")) help_animated_file();
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntries[] =
{
	{"File",     NULL, N_("_File"), NULL, NULL, G_CALLBACK (activate_action)},
	{"Read",     NULL, N_("_Read")},
	{"ReadAuto", NULL, N_("Read a file(Auto)"), NULL, "Read grids", G_CALLBACK (activate_action) },
	{"ReadGabedit", GABEDIT_STOCK_GABEDIT, N_("Read G_abedit cube files"), NULL, "Read Gabedit cube files", G_CALLBACK (activate_action) },
	{"ReadDX", NULL, N_("Read  grids from _DX files"), NULL, "Read grids from DX files", G_CALLBACK (activate_action) },
	{"DeleteGrid", GABEDIT_STOCK_CUT, N_("_Delete selected grid"), NULL, "Delete selected grid", G_CALLBACK (activate_action) },
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
"  <popup name=\"MenuAnimGrid\">\n"
"    <separator name=\"sepMenuPopGabedit\" />\n"
"    <menuitem name=\"ReadAuto\" action=\"ReadAuto\" />\n"
"    <separator name=\"sepMenuAuto\" />\n"
"    <menuitem name=\"ReadGabedit\" action=\"ReadGabedit\" />\n"
"    <menuitem name=\"ReadDX\" action=\"ReadDX\" />\n"
"    <separator name=\"sepMenuDelete\" />\n"
"    <menuitem name=\"DeleteGrid\" action=\"DeleteGrid\" />\n"
"    <separator name=\"sepMenuPopClose\" />\n"
"    <menuitem name=\"Close\" action=\"Close\" />\n"
"  </popup>\n"
"  <menubar name = \"MenuBar\">\n"
"    <menu name=\"File\" action=\"File\">\n"
"      <menu name=\"Read\" action=\"Read\">\n"
"        <menuitem name=\"ReadAuto\" action=\"ReadAuto\" />\n"
"        <separator name=\"sepMenuAuto\" />\n"
"        <menuitem name=\"ReadGabedit\" action=\"ReadGabedit\" />\n"
"        <menuitem name=\"ReadDX\" action=\"ReadDX\" />\n"
"      </menu>\n"
"      <separator name=\"sepMenuDelete\" />\n"
"      <menuitem name=\"DeleteGrid\" action=\"DeleteGrid\" />\n"
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

	actionGroup = gtk_action_group_new ("GabeditAnimationAnimGridActions");
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
void animationGridsDlg()
{
	GtkWidget *Win;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *parentWindow = PrincipalWindow;
	GtkUIManager *manager = NULL;

	if(WinDlg) return;

	initAnimationGrids();

	Win= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_default_size (GTK_WINDOW(Win),-1,(gint)(ScreenHeightD*0.69));
	gtk_window_set_title(GTK_WINDOW(Win),"Multiple Grids");
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	WinDlg = Win;

  	add_child(PrincipalWindow,Win,destroyAnimAnimGridDlg,"Mult. Grid");
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
