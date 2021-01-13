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
#include <stdlib.h>

#include "../../Config.h"
#include "../Common/Global.h"
#include "../Molcas/MolcasLib.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Constants.h"
#include "../Molcas/MolcasBasisLibrary.h"
#include "../Common/StockIcons.h"

static GtkWidget* Entry;
static GtkWidget* EntryBasisName;
static GtkWidget* EntryAuthor;
static GtkWidget* EntryPrimitive;
static GtkWidget* EntryContraction;
static GtkWidget* EntryEcpType;


#define lengthList	6
/************************************************************************************************************/
typedef enum
{
  LIST_ATOM_SYMBOL = 0,
  LIST_BASIS_NAME,
  LIST_AUTHOR,
  LIST_PRIMITIVE,
  LIST_CONTRACTION,
  LIST_ECPTYPE,
  LIST_DATA /* column for data, this column is not visible */
}ListColumnsTypes;
/************************************************************************************************************/

static gchar *listTitles[]={ "Atom", "Basis Name", "Author","Primitive", "Contraction","ECP Type" };

static	MolcasBasis molcasBasis={0,NULL};

static GtkWidget *SetWinDlg = NULL;
static GtkWidget *treeView = NULL;

typedef struct _DataMolcasTree DataMolcasTree;
struct _DataMolcasTree
{
	gint atomNumber;
	gint basisNumber;
};
static gchar selectedRow[100] = "-1";
static void rafreshTreeView();
/********************************************************************************/
typedef enum
{
	MENU_NEW_ATOM = 0,
	MENU_DELETE_ATOM,
	MENU_NEW_BASIS,
	MENU_EDIT_BASIS,
	MENU_DELETE_BASIS,
	MENU_SAVE,
	MENU_CLOSE
} EnumMenuType;

/********************************************************************************/
static void deleteBasisDlg(GtkWidget *win,gpointer d);
static void deleteAtomDlg(GtkWidget *win,gpointer d);
static void newBasisDlg();
static void editBasisDlg();
static void newAtomDlg();
/********************************************************************************/
static void freeMolcasBasis()
{
	if(molcasBasis.numberOfAtoms<1)
	{
		molcasBasis.numberOfAtoms = 0;
		molcasBasis.atoms = NULL;
		return;
	}
	molcasBasis.numberOfAtoms = 0;
	molcasBasis.atoms = NULL;
}
/********************************************************************************/
void get_molcas_basis_list(gchar* Symb,gchar* outfile,gchar* errfile)
{
	FILE* fout=FOpen(outfile,"w");  
	FILE* ferr=FOpen(errfile,"w");  
	gchar bas[BSIZE];
	gint atomNumber = -1;
	gint i;

	if(molcasBasis.numberOfAtoms<1)
	{
		loadMolcasBasis();
		if(molcasBasis.numberOfAtoms<1)
			return;
	}

	atomNumber = -1;
	for(i=0;i<molcasBasis.numberOfAtoms;i++)
		if(strcmp(molcasBasis.atoms[i].symbol,Symb)==0)
		{
			atomNumber = i;
			break;
		}
	if(atomNumber>=0)
	for(i=0;i<molcasBasis.atoms[atomNumber].numberOfBasis;i++)
	{
		sprintf(bas,"%s.%s.%s.%s.%s.%s.",molcasBasis.atoms[atomNumber].symbol,
				molcasBasis.atoms[atomNumber].basis[i].basisName,
				molcasBasis.atoms[atomNumber].basis[i].author,
				molcasBasis.atoms[atomNumber].basis[i].primitive,
				molcasBasis.atoms[atomNumber].basis[i].contraction,
				molcasBasis.atoms[atomNumber].basis[i].ecpType
				);
		fprintf(fout,"%s\n",bas);
	}
	fclose(ferr);
	fclose(fout);
}
/********************************************************************************/
static void destroyDlg(GtkWidget* win, gpointer data)
{
	gtk_widget_destroy(win);
}
/********************************************************************************/
static void Traitement(guint Operation)
{
	switch(Operation)
	{
		case MENU_NEW_ATOM :
			newAtomDlg();
			break;
		case MENU_DELETE_ATOM :
			deleteAtomDlg(NULL,NULL);
			break;
		case MENU_NEW_BASIS :
			newBasisDlg();
			break;
		case MENU_EDIT_BASIS :
			editBasisDlg();
			break;
		case MENU_DELETE_BASIS :
			deleteBasisDlg(NULL,NULL);
			break;
		case MENU_SAVE :
			saveMolcasBasis();
			break;
		case MENU_CLOSE :
			destroyDlg(SetWinDlg, NULL);
			SetWinDlg = NULL;
	}
}
/********************************************************************************/
static void set_sensitive_option(GtkUIManager *manager, gchar* path, gboolean sensitive)
{
	GtkWidget *wid = gtk_ui_manager_get_widget (manager, path);
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, sensitive);
}
/**********************************************************************************/
static void set_sensitive_menu(GtkUIManager *manager, gboolean norow, gint basisNumber)
{
	gint i;
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuMolcasBasis");
	static gchar* paths[] = {
		"/MenuMolcasBasis/NewAtom", "/MenuMolcasBasis/DeleteAtom",
		"/MenuMolcasBasis/NewBasis","/MenuMolcasBasis/EditBasis", "/MenuMolcasBasis/DeleteBasis",
		"/MenuMolcasBasis/Save", "/MenuMolcasBasis/Close" };
	static gboolean sensitive[] = {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE};
	static gint o = MENU_NEW_ATOM;
	static gint n = MENU_CLOSE-MENU_NEW_ATOM;
	if (!GTK_IS_MENU (menu)) return;
	if(norow < 0)
	{
		sensitive[MENU_NEW_ATOM-o] = TRUE;
		sensitive[MENU_DELETE_ATOM-o] = FALSE;
		sensitive[MENU_NEW_BASIS-o] = FALSE;
		sensitive[MENU_EDIT_BASIS-o] = FALSE;
		sensitive[MENU_DELETE_BASIS-o] = FALSE;
		sensitive[MENU_SAVE-o] = TRUE;
		sensitive[MENU_CLOSE-o] = TRUE;
		for(i=0;i<n;i++) set_sensitive_option(manager, paths[i], sensitive[i]);
	}
	else
	{
		if(basisNumber<0)
		{
			sensitive[MENU_NEW_ATOM-o] = TRUE;
			sensitive[MENU_DELETE_ATOM-o] = TRUE;
			sensitive[MENU_NEW_BASIS-o] = TRUE;
			sensitive[MENU_EDIT_BASIS-o] = FALSE;
			sensitive[MENU_DELETE_BASIS-o] = FALSE;
			sensitive[MENU_SAVE-o] = TRUE;
			sensitive[MENU_CLOSE-o] = TRUE;
			for(i=0;i<n;i++) set_sensitive_option(manager, paths[i], sensitive[i]);
		}
		else
		if(basisNumber>=0)
		{
			sensitive[MENU_NEW_ATOM-o] = FALSE;
			sensitive[MENU_DELETE_ATOM-o] = FALSE;
			sensitive[MENU_NEW_BASIS-o] = TRUE;
			sensitive[MENU_EDIT_BASIS-o] = TRUE;
			sensitive[MENU_DELETE_BASIS-o] = TRUE;
			sensitive[MENU_SAVE-o] = TRUE;
			sensitive[MENU_CLOSE-o] = TRUE;
			for(i=0;i<n;i++) set_sensitive_option(manager, paths[i], sensitive[i]);
		}
	}
}
/**********************************************************************************/
static gboolean show_menu_popup(GtkUIManager *manager, guint button, guint32 time)
{
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuMolcasBasis");
	if (GTK_IS_MENU (menu)) 
	{
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, time);
		return TRUE;
	}
	return FALSE;
}
/*********************************************************************************************************************/
static void activate_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	if(!strcmp(name, "NewAtom")) Traitement(MENU_NEW_ATOM);
	else if(!strcmp(name, "DeleteAtom")) Traitement(MENU_DELETE_ATOM);
	else if(!strcmp(name, "NewBasis")) Traitement(MENU_NEW_BASIS);
	else if(!strcmp(name, "EditBasis")) Traitement(MENU_EDIT_BASIS);
	else if(!strcmp(name, "DeleteBasis")) Traitement(MENU_DELETE_BASIS);
	else if(!strcmp(name, "Save")) Traitement(MENU_SAVE);
	else if(!strcmp(name, "Close")) Traitement(MENU_CLOSE);
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntries[] =
{
	{"NewAtom", GABEDIT_STOCK_NEW, "New _atom", NULL, "New atom", G_CALLBACK (activate_action) },
	{"DeleteAtom", GABEDIT_STOCK_CUT, "_Delete selected atom", NULL, "Delete selected atom", G_CALLBACK (activate_action) },
	{"NewBasis", GABEDIT_STOCK_NEW, "New _basis", NULL, "New _basis", G_CALLBACK (activate_action) },
	{"EditBasis", GABEDIT_STOCK_SELECT, "_Edit basis", NULL, "_Edit basis", G_CALLBACK (activate_action) },
	{"DeleteBasis", GABEDIT_STOCK_CUT, "_Delete selected basis", NULL, "Delete selected basis", G_CALLBACK (activate_action) },
	{"Save", GABEDIT_STOCK_SAVE, "_Save", NULL, "Save", G_CALLBACK (activate_action) },
	{"Close", GABEDIT_STOCK_CLOSE, "_Close", NULL, "Close", G_CALLBACK (activate_action) },
};
static guint numberOfGtkActionEntries = G_N_ELEMENTS (gtkActionEntries);
/********************************************************************************/
/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiMenuInfo =
"  <popup name=\"MenuMolcasBasis\">\n"
"    <separator name=\"sepMenuPopAtom\" />\n"
"    <menuitem name=\"NewAtom\" action=\"NewAtom\" />\n"
"    <menuitem name=\"DeleteAtom\" action=\"DeleteAtom\" />\n"
"    <separator name=\"sepMenuPopBasis\" />\n"
"    <menuitem name=\"NewBasis\" action=\"NewBasis\" />\n"
"    <menuitem name=\"EditBasis\" action=\"EditBasis\" />\n"
"    <menuitem name=\"DeleteBasis\" action=\"DeleteBasis\" />\n"
"    <separator name=\"sepMenuPopSave\" />\n"
"    <menuitem name=\"Save\" action=\"Save\" />\n"
"    <separator name=\"sepMenuPopClose\" />\n"
"    <menuitem name=\"Close\" action=\"Close\" />\n"
"  </popup>\n"
;
/*******************************************************************************************************************************/
static GtkUIManager *newMenu(GtkWidget* win)
{
	GtkActionGroup *actionGroup = NULL;
	GtkUIManager *manager = NULL;
	GError *error = NULL;

  	manager = gtk_ui_manager_new ();
  	g_signal_connect_swapped (win, "destroy", G_CALLBACK (g_object_unref), manager);

	actionGroup = gtk_action_group_new ("GabeditEditBasisMolcasLibrary");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntries, numberOfGtkActionEntries, NULL);

  	gtk_ui_manager_insert_action_group (manager, actionGroup, 0);

  	gtk_window_add_accel_group (GTK_WINDOW (win), gtk_ui_manager_get_accel_group (manager));
	if (!gtk_ui_manager_add_ui_from_string (manager, uiMenuInfo, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	return manager;
}
/**********************************************************************************/
static void eventDispatcher(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *model;

	if (!event) return;
	if (event->window == gtk_tree_view_get_bin_window (GTK_TREE_VIEW (widget))
	    && !gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, NULL, NULL, NULL, NULL)) {
		gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)));
	}
	if(gtk_tree_view_get_path_at_pos (GTK_TREE_VIEW (widget), event->x, event->y, &path, NULL, NULL, NULL))
	{
		if(path)
		{
			model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
			gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)), path);
			sprintf(selectedRow ,"%s",gtk_tree_path_to_string(path));
			gtk_tree_model_get_iter (model, &iter, path);
			gtk_tree_path_free(path);
  			if (event->type == GDK_BUTTON_PRESS && ((GdkEventButton *) event)->button == 3)
			{
				GdkEventButton *bevent = (GdkEventButton *) event;
  				GtkUIManager *manager = GTK_UI_MANAGER(user_data);
				if(atoi(selectedRow) < 0) set_sensitive_menu(manager, TRUE, 0);
				else
				{
   					DataMolcasTree* data = NULL;
					gtk_tree_model_get (model, &iter, LIST_DATA, &data, -1);
					if(data) set_sensitive_menu(manager, FALSE, data->basisNumber);
				}
				show_menu_popup(manager, bevent->button, bevent->time);
			}
		}
	}
	GTK_WIDGET_GET_CLASS(widget)->button_press_event(widget, event);
}
/********************************************************************************/
static DataMolcasTree* newDataMolcasTree(gint atomNumber, gint basisNumber)
{
	DataMolcasTree* dataTree;
	dataTree = g_malloc(sizeof(DataMolcasTree));
	dataTree->atomNumber = atomNumber;
	dataTree->basisNumber = basisNumber;
	return  dataTree;
}
/********************************************************************************/
static void freeDataMolcasTree()
{
   	DataMolcasTree* data = NULL;
	gint i = 0;
	gchar* pathString = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	pathString = g_strdup_printf("%d", i);
	while (gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
	{
		gtk_tree_model_get (model, &iter, LIST_DATA, &data, -1);
		if(data) g_free(data);
		i++;
		g_free(pathString);
		pathString = g_strdup_printf("%d", i);
	}
	g_free(pathString);
}
/********************************************************************************/
static gboolean* getExpandInfo()
{
	gint i;
	gboolean* expandeds = NULL;
	gint nNodes = 0;

	gchar* pathString = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	nNodes = 0;
	pathString = g_strdup_printf("%d", nNodes);
	while (gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
	{
		nNodes++;
		g_free(pathString);
		pathString = g_strdup_printf("%d", nNodes);
	}
	g_free(pathString);
	if(nNodes<1) return NULL;
	expandeds = g_malloc((nNodes+1)*sizeof(gboolean));
	for(i=0;i<nNodes+1;i++) expandeds[i] = FALSE;

	i = 0;
	pathString = g_strdup_printf("%d", i);
	while (gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
	{
		GtkTreePath *path = gtk_tree_path_new_from_string  (pathString);
		expandeds[i] = FALSE;
 		expandeds[i]  =   gtk_tree_view_row_expanded(GTK_TREE_VIEW(treeView), path);
		gtk_tree_path_free(path);
		i++;
		g_free(pathString);
		pathString = g_strdup_printf("%d", i);
	}
	g_free(pathString);
	return expandeds;

}
/********************************************************************************/
static void setExpandeds(gboolean* expandeds, gchar* selected_row)
{
	gint i;
	gchar* pathString = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	if(!expandeds) return;

	i = 0;
	pathString = g_strdup_printf("%d", i);
	while (gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
	{
		if(expandeds[i])
		{
			GtkTreePath *path = gtk_tree_path_new_from_string  (pathString);
			gtk_tree_view_expand_to_path(GTK_TREE_VIEW(treeView), path);
			gtk_tree_path_free(path);
		}
		i++;
		g_free(pathString);
		pathString = g_strdup_printf("%d", i);
	}
	g_free(pathString);
	if(selected_row)
	{
			GtkTreePath *path = gtk_tree_path_new_from_string  (selected_row);
			gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeView)), path);
			gtk_tree_path_free(path);
	}
}
/********************************************************************************/
static void deleteOneBasis(GtkWidget *win, gpointer d)
{

	gint atomNumber;
	gint basisNumber;
	gint numberOfBasis;
	gint i;
	gboolean* expandeds;

	GtkTreeIter node;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
   	DataMolcasTree* data = NULL;

	if(atoi(selectedRow)<0) return;
	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;

	expandeds = getExpandInfo();


	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);
	
	if(!data) return;

	atomNumber = data->atomNumber;
	basisNumber = data->basisNumber;

	if(atomNumber<0)
		return;
	if(basisNumber<0)
		return;


	if(molcasBasis.atoms[atomNumber].basis[basisNumber].basisName)
		g_free(molcasBasis.atoms[atomNumber].basis[basisNumber].basisName);
	if(molcasBasis.atoms[atomNumber].basis[basisNumber].author)
		g_free(molcasBasis.atoms[atomNumber].basis[basisNumber].author);
	if(molcasBasis.atoms[atomNumber].basis[basisNumber].primitive)
		g_free(molcasBasis.atoms[atomNumber].basis[basisNumber].primitive);
	if(molcasBasis.atoms[atomNumber].basis[basisNumber].contraction)
		g_free(molcasBasis.atoms[atomNumber].basis[basisNumber].contraction);
	if(molcasBasis.atoms[atomNumber].basis[basisNumber].ecpType)
		g_free(molcasBasis.atoms[atomNumber].basis[basisNumber].ecpType);

	numberOfBasis = molcasBasis.atoms[atomNumber].numberOfBasis;

	for(i=basisNumber;i<numberOfBasis-1;i++)
		molcasBasis.atoms[atomNumber].basis[i] = molcasBasis.atoms[atomNumber].basis[i+1];

	molcasBasis.atoms[atomNumber].numberOfBasis--;	

	molcasBasis.atoms[atomNumber].basis = 
		g_realloc(molcasBasis.atoms[atomNumber].basis,
			molcasBasis.atoms[atomNumber].numberOfBasis*sizeof(MolcasOneBasis));

	freeDataMolcasTree();
	rafreshTreeView();
	setExpandeds(expandeds,selectedRow);
	if(expandeds) g_free(expandeds);
}
/********************************************************************************/
static void deleteBasisDlg(GtkWidget *win,gpointer d)
{

	gchar *format ="Do you want to really delete \"%s.%s\" basis for \"%s\" atom ?" ;
	gchar *t =NULL;
  	GtkTreeIter node;
   	DataMolcasTree* data = NULL;
	gint atomNumber;
	gint basisNumber;
	GtkWidget* w = NULL;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);

	if(!data) return;

	atomNumber = data->atomNumber;
	basisNumber = data->basisNumber;

	if(atomNumber<0)
		return;
	if(basisNumber<0)
		return;

	t =g_strdup_printf(format,
			molcasBasis.atoms[atomNumber].basis[basisNumber].basisName,
			molcasBasis.atoms[atomNumber].basis[basisNumber].author,
			molcasBasis.atoms[atomNumber].symbol
			);

	w = Continue_YesNo(deleteOneBasis, NULL,t);
	gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  	gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
}
/********************************************************************************/
static void deleteOneAtom(GtkWidget *win, gpointer d)
{
  	GtkTreeIter node;
   	DataMolcasTree* data = NULL;

	gint atomNumber;
	gint basisNumber;
	gint numberOfBasis;
	gint numberOfAtoms;
	gint i;
	gboolean* expandeds;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	if(atoi(selectedRow)<0) return;

	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);

	expandeds = getExpandInfo();

	if(!data) return; 
	atomNumber = data->atomNumber;
	basisNumber = data->basisNumber;

	if(atomNumber<0)
		return;
	if(basisNumber>=0)
		return;


	numberOfBasis = molcasBasis.atoms[atomNumber].numberOfBasis;
	numberOfAtoms = molcasBasis.numberOfAtoms;
	for(i=0;i<numberOfBasis;i++)
	{
		if(molcasBasis.atoms[atomNumber].basis[i].basisName)
			g_free(molcasBasis.atoms[atomNumber].basis[i].basisName);
		if(molcasBasis.atoms[atomNumber].basis[i].author)
			g_free(molcasBasis.atoms[atomNumber].basis[i].author);
		if(molcasBasis.atoms[atomNumber].basis[i].primitive)
			g_free(molcasBasis.atoms[atomNumber].basis[i].primitive);
		if(molcasBasis.atoms[atomNumber].basis[i].contraction)
			g_free(molcasBasis.atoms[atomNumber].basis[i].contraction);
		if(molcasBasis.atoms[atomNumber].basis[i].ecpType)
			g_free(molcasBasis.atoms[atomNumber].basis[i].ecpType);
	}

	if(molcasBasis.atoms[atomNumber].symbol)
		g_free(molcasBasis.atoms[atomNumber].symbol);

	for(i=atomNumber;i<numberOfAtoms-1;i++)
		molcasBasis.atoms[i] = molcasBasis.atoms[i+1];

	molcasBasis.numberOfAtoms--;	

	molcasBasis.atoms = 
		g_realloc(molcasBasis.atoms,
			molcasBasis.numberOfAtoms*sizeof(MolcasAtom));

	freeDataMolcasTree();
	rafreshTreeView();
	setExpandeds(expandeds,selectedRow);
	if(expandeds) g_free(expandeds);
}
/********************************************************************************/
static void deleteAtomDlg(GtkWidget *win,gpointer d)
{

	gchar *format ="Do you want to really delete \"%s\" atom ?" ;
	gchar *t =NULL;
  	GtkTreeIter node;
   	DataMolcasTree* data = NULL;
	gint atomNumber;
	gint basisNumber;
	GtkWidget* w = NULL;

	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);

	if(!data) return;

	atomNumber = data->atomNumber;
	basisNumber = data->basisNumber;

	if(atomNumber<0)
		return;
	if(basisNumber>=0)
		return;

	t =g_strdup_printf(format,molcasBasis.atoms[atomNumber].symbol);

	w = Continue_YesNo(deleteOneAtom, NULL,t);
	gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  	gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
}
/********************************************************************************/
static void newAtom()
{
  	GtkTreeIter node;
   	DataMolcasTree* data = NULL;
	gint atomNumber;
	gint basisNumber;
	gint numberOfAtoms;
	MolcasAtom* atoms;
	gchar* symbol = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry)));
	gboolean* expandeds;
	gint i;

	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);


	atomNumber = -1;
	basisNumber = -1;
	if(data)
	{
		atomNumber = data->atomNumber;
		basisNumber = data->basisNumber;
	}

	if(strlen(symbol)<=0) return;

	numberOfAtoms = molcasBasis.numberOfAtoms;
	atoms = molcasBasis.atoms;
	for(i=0;i<numberOfAtoms;i++)
	{
		if(strcmp(symbol,atoms[i].symbol)==0)
		{
			GtkWidget* win = Message("Sorry this atom is available","Error",TRUE);
  			gtk_window_set_modal(GTK_WINDOW(win),TRUE);
  			gtk_window_set_transient_for(GTK_WINDOW(win),GTK_WINDOW(SetWinDlg));
			return;
		}
	}
	numberOfAtoms++;
	atoms = g_realloc(atoms,numberOfAtoms*sizeof(MolcasAtom));
	atoms[numberOfAtoms-1].symbol = symbol;
	atoms[numberOfAtoms-1].numberOfBasis = 0;
	atoms[numberOfAtoms-1].basis = NULL;

	molcasBasis.atoms = atoms;
	molcasBasis.numberOfAtoms = numberOfAtoms;
	if(atomNumber>=0)
	{
		MolcasAtom t = molcasBasis.atoms[numberOfAtoms-1];
		for(i=numberOfAtoms-1;i>atomNumber;i--)
			molcasBasis.atoms[i] = molcasBasis.atoms[i-1];
		molcasBasis.atoms[atomNumber] = t; 
	}

	expandeds = getExpandInfo();
	freeDataMolcasTree();
	rafreshTreeView();
	setExpandeds(expandeds,selectedRow);
	if(expandeds) g_free(expandeds);

}
/********************************************************************************/
static void setAtom(GtkWidget *w,gpointer data)
{
  gtk_entry_set_text(GTK_ENTRY(Entry),(char *)data);
}
/********************************************************************************/
static void selectAtom(GtkWidget *w,gpointer entry0)
{
	GtkWidget* Table;
	GtkWidget* button;
	GtkWidget* frame;
	GtkWidget* FenetreTable;
	guint i;
	guint j;
        GtkStyle *button_style;
        GtkStyle *style;

	gchar*** Symb = get_periodic_table();

  FenetreTable = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(FenetreTable),TRUE);
  gtk_window_set_title(GTK_WINDOW(FenetreTable),"Select your atom");
  gtk_window_set_default_size (GTK_WINDOW(FenetreTable),(gint)(ScreenWidth*0.5),(gint)(ScreenHeight*0.4));

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

  gtk_container_add(GTK_CONTAINER(FenetreTable),frame);  
  gtk_widget_show (frame);

  Table = gtk_table_new(PERIODIC_TABLE_N_ROWS-1,PERIODIC_TABLE_N_COLUMNS,TRUE);
  gtk_container_add(GTK_CONTAINER(frame),Table);
  button_style = gtk_widget_get_style(FenetreTable); 
  
  for ( i = 0;i<PERIODIC_TABLE_N_ROWS-1;i++)
	  for ( j = 0;j<PERIODIC_TABLE_N_COLUMNS;j++)
  {
	  if(strcmp(Symb[j][i],"00"))
	  {
	  button = gtk_button_new_with_label(Symb[j][i]);
          style=set_button_style(button_style,button,Symb[j][i]);
          g_signal_connect(G_OBJECT(button), "clicked",
                            (GCallback)setAtom,(gpointer )Symb[j][i]);
          g_signal_connect_swapped(G_OBJECT(button), "clicked",
                            (GCallback)gtk_widget_destroy,GTK_OBJECT(FenetreTable));
	  gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	  }

  }
 	
  gtk_widget_show_all(FenetreTable);
  
}
/********************************************************************************/
static void newAtomDlg()
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *vboxframe;
  	GtkTreeIter node;
   	DataMolcasTree* data = NULL;
	gint atomNumber;
	gint basisNumber;
	gchar title[BSIZE];

	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);


	if(!data) return;

	atomNumber = data->atomNumber;
	basisNumber = data->basisNumber;

	sprintf(title,"New Atom");

	WinDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(WinDlg),title);
	gtk_window_set_position(GTK_WINDOW(WinDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(WinDlg),GTK_WINDOW(SetWinDlg));
	gtk_window_set_modal(GTK_WINDOW(WinDlg),TRUE);

	g_signal_connect(G_OBJECT(WinDlg),"delete_event",(GCallback)gtk_widget_destroy,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->vbox), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	hbox=create_hbox_false(vboxframe);

	Entry = create_label_entry(hbox," Atom Symbol : ",-1,-1); 
	gtk_entry_set_text(GTK_ENTRY(Entry),"H");
	gtk_editable_set_editable((GtkEditable*) Entry,FALSE);
  	Button = gtk_button_new_with_label(" Set ");
	gtk_box_pack_start (GTK_BOX(hbox), Button, TRUE, TRUE, 5);
	g_signal_connect(G_OBJECT(Button), "clicked", (GCallback)selectAtom,Entry);

	gtk_widget_realize(WinDlg);
	Button = create_button(WinDlg,"Cancel");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);

	Button = create_button(WinDlg,"OK");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)newAtom,GTK_OBJECT(WinDlg));
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
    

	gtk_widget_show_all(WinDlg);
}
/********************************************************************************/
static void newBasis()
{
  	GtkTreeIter node;
   	DataMolcasTree* data = NULL;
	gint atomNumber;
	gint basisNumber;
	gint numberOfBasis;
	MolcasOneBasis* basis;
	gchar* basisName = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryBasisName)));
	gchar* author = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryAuthor)));
	gchar* primitive = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryPrimitive)));
	gchar* contraction = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryContraction)));
	gchar* ecpType = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryEcpType)));
	gboolean* expandeds;
	gint i;

	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);


	if(!data) return;

	atomNumber = data->atomNumber;
	basisNumber = data->basisNumber;

	if(atomNumber<0)
		return;
	delete_all_spaces(basisName);
	if(strlen(basisName)<=0) return;

	numberOfBasis = molcasBasis.atoms[atomNumber].numberOfBasis;
	basis = molcasBasis.atoms[atomNumber].basis;
	for(i=0;i<numberOfBasis;i++)
	{
		if(strcmp(basisName,basis[i].basisName)==0)
		{
			Message("Sorry this basis is available","Error",TRUE);
			return;
		}
	}
	numberOfBasis++;
	basis = g_realloc(basis,numberOfBasis*sizeof(MolcasOneBasis));
	basis[numberOfBasis-1].basisName = basisName;
	if(this_is_a_backspace(author))
		basis[numberOfBasis-1].author = g_strdup("UNK");
	else
		basis[numberOfBasis-1].author =  g_strdup(author);

	if(this_is_a_backspace(primitive))
		basis[numberOfBasis-1].primitive = g_strdup("UNK");
	else
		basis[numberOfBasis-1].primitive = g_strdup(primitive);

	if(this_is_a_backspace(contraction))
		basis[numberOfBasis-1].contraction =  g_strdup("UNK");
	else
		basis[numberOfBasis-1].contraction =  g_strdup(contraction);

	if(this_is_a_backspace(ecpType))
		basis[numberOfBasis-1].ecpType =  g_strdup("UNK");
	else
		basis[numberOfBasis-1].ecpType =  g_strdup(ecpType);

	molcasBasis.atoms[atomNumber].basis = basis;
	molcasBasis.atoms[atomNumber].numberOfBasis = numberOfBasis;
	if(basisNumber>=0)
	{
		MolcasOneBasis t = molcasBasis.atoms[atomNumber].basis[numberOfBasis-1];

		for(i=numberOfBasis-1;i>basisNumber;i--)
			molcasBasis.atoms[atomNumber].basis[i] = 
			molcasBasis.atoms[atomNumber].basis[i-1];

		molcasBasis.atoms[atomNumber].basis[basisNumber] = t;
	}

	expandeds = getExpandInfo();
	freeDataMolcasTree();
	rafreshTreeView();
	setExpandeds(expandeds,selectedRow);
	if(expandeds) g_free(expandeds);

}
/********************************************************************************/
static void newBasisDlg()
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
  	GtkTreeIter node;
   	DataMolcasTree* data = NULL;
	gint atomNumber;
	gint basisNumber;
	gchar title[BSIZE];
	GtkWidget *table = NULL;
	GtkWidget *label = NULL;
	gushort i;

	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);


	if(!data)
		return;

	atomNumber = data->atomNumber;
	basisNumber = data->basisNumber;

	if(atomNumber<0 )
		return;

	sprintf(title,"New Basis for %s atom :",molcasBasis.atoms[atomNumber].symbol);

	WinDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(WinDlg),title);
	gtk_window_set_position(GTK_WINDOW(WinDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(WinDlg),GTK_WINDOW(SetWinDlg));
	gtk_window_set_modal(GTK_WINDOW(WinDlg),TRUE);

	g_signal_connect(G_OBJECT(WinDlg),"delete_event",(GCallback)gtk_widget_destroy,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->vbox), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(4,3,FALSE);
	gtk_box_pack_start (GTK_BOX (vboxframe), table, TRUE, TRUE, 0);

	i = 0;
	label = gtk_label_new(" Basis Name ");
	add_widget_table(table, label, i, 0);
	label = gtk_label_new(" : ");
	add_widget_table(table, label, i, 1);
	EntryBasisName = gtk_entry_new();
	add_widget_table(table, EntryBasisName, i, 2);

	i = 1;
	label = gtk_label_new(" Author ");
	add_widget_table(table, label, i, 0);
	label = gtk_label_new(" : ");
	add_widget_table(table, label, i, 1);
	EntryAuthor = gtk_entry_new();
	add_widget_table(table, EntryAuthor, i, 2);

	i = 2;
	label = gtk_label_new(" Primitive ");
	add_widget_table(table, label, i, 0);
	label = gtk_label_new(" : ");
	add_widget_table(table, label, i, 1);
	EntryPrimitive = gtk_entry_new();
	add_widget_table(table, EntryPrimitive, i, 2);

	i = 3;
	label = gtk_label_new(" Contraction ");
	add_widget_table(table, label, i, 0);
	label = gtk_label_new(" : ");
	add_widget_table(table, label, i, 1);
	EntryContraction = gtk_entry_new();
	add_widget_table(table, EntryContraction, i, 2);

	i = 4;
	label = gtk_label_new(" ECP Type ");
	add_widget_table(table, label, i, 0);
	label = gtk_label_new(" : ");
	add_widget_table(table, label, i, 1);
	EntryEcpType = gtk_entry_new();
	add_widget_table(table, EntryEcpType, i, 2);

	gtk_widget_realize(WinDlg);
	Button = create_button(WinDlg,"Cancel");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);

	Button = create_button(WinDlg,"OK");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)newBasis,GTK_OBJECT(WinDlg));
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
    

	gtk_widget_show_all(WinDlg);
}
/********************************************************************************/
static void editBasis()
{
  	GtkTreeIter node;
   	DataMolcasTree* data = NULL;
	gint atomNumber;
	gint basisNumber;
	MolcasOneBasis* basis;
	gchar* basisName = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryBasisName)));
	gchar* author = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryAuthor)));
	gchar* primitive = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryPrimitive)));
	gchar* contraction = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryContraction)));
	gchar* ecpType = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryEcpType)));
	gboolean* expandeds;

	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);



	if(!data) return;

	atomNumber = data->atomNumber;
	basisNumber = data->basisNumber;

	if(atomNumber<0) return;
	if(basisNumber<0) return;

	delete_all_spaces(basisName);
	if(strlen(basisName)<=0) return;

	basis = molcasBasis.atoms[atomNumber].basis;

	if(basis[basisNumber].basisName) g_free(basis[basisNumber].basisName);
	if(basis[basisNumber].author) g_free(basis[basisNumber].author);
	if(basis[basisNumber].primitive) g_free(basis[basisNumber].primitive);
	if(basis[basisNumber].contraction) g_free(basis[basisNumber].contraction);
	if(basis[basisNumber].ecpType) g_free(basis[basisNumber].ecpType);

	basis[basisNumber].basisName = basisName;
	
	if(this_is_a_backspace(author))
		basis[basisNumber].author = g_strdup("UNK");
	else
		basis[basisNumber].author =  g_strdup(author);

	if(this_is_a_backspace(primitive))
		basis[basisNumber].primitive = g_strdup("UNK");
	else
		basis[basisNumber].primitive = g_strdup(primitive);

	if(this_is_a_backspace(contraction))
		basis[basisNumber].contraction =  g_strdup("UNK");
	else
		basis[basisNumber].contraction =  g_strdup(contraction);

	if(this_is_a_backspace(ecpType))
		basis[basisNumber].ecpType =  g_strdup("UNK");
	else
		basis[basisNumber].ecpType =  g_strdup(ecpType);

	molcasBasis.atoms[atomNumber].basis = basis;

	expandeds = getExpandInfo();
	freeDataMolcasTree();
	rafreshTreeView();
	setExpandeds(expandeds,selectedRow);
	if(expandeds) g_free(expandeds);
}
/********************************************************************************/
static void editBasisDlg()
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
  	GtkTreeIter node;
   	DataMolcasTree* data = NULL;
	gint atomNumber;
	gint basisNumber;
	gchar title[BSIZE];
	GtkWidget *table = NULL;
	GtkWidget *label = NULL;
	gushort i;

	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);

	if(!data)
		return;

	atomNumber = data->atomNumber;
	basisNumber = data->basisNumber;

	if(atomNumber<0 )
		return;

	sprintf(title,"Edit Basis for %s atom :",molcasBasis.atoms[atomNumber].symbol);

	WinDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(WinDlg),title);
	gtk_window_set_position(GTK_WINDOW(WinDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(WinDlg),GTK_WINDOW(SetWinDlg));
	gtk_window_set_modal(GTK_WINDOW(WinDlg),TRUE);

	g_signal_connect(G_OBJECT(WinDlg),"delete_event",(GCallback)gtk_widget_destroy,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->vbox), frame,TRUE,TRUE,0);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(4,3,FALSE);
	gtk_box_pack_start (GTK_BOX (vboxframe), table, TRUE, TRUE, 0);

	i = 0;
	label = gtk_label_new(" Basis Name ");
	add_widget_table(table, label, i, 0);
	label = gtk_label_new(" : ");
	add_widget_table(table, label, i, 1);
	EntryBasisName = gtk_entry_new();
	if(!strstr(molcasBasis.atoms[atomNumber].basis[basisNumber].basisName,"UNK"))
		gtk_entry_set_text(GTK_ENTRY(EntryBasisName),molcasBasis.atoms[atomNumber].basis[basisNumber].basisName);
	add_widget_table(table, EntryBasisName, i, 2);

	i = 1;
	label = gtk_label_new(" Author ");
	add_widget_table(table, label, i, 0);
	label = gtk_label_new(" : ");
	add_widget_table(table, label, i, 1);
	EntryAuthor = gtk_entry_new();
	if(!strstr(molcasBasis.atoms[atomNumber].basis[basisNumber].author,"UNK"))
		gtk_entry_set_text(GTK_ENTRY(EntryAuthor),molcasBasis.atoms[atomNumber].basis[basisNumber].author);
	add_widget_table(table, EntryAuthor, i, 2);

	i = 2;
	label = gtk_label_new(" Primitive ");
	add_widget_table(table, label, i, 0);
	label = gtk_label_new(" : ");
	add_widget_table(table, label, i, 1);
	EntryPrimitive = gtk_entry_new();
	if(!strstr(molcasBasis.atoms[atomNumber].basis[basisNumber].primitive,"UNK"))
		gtk_entry_set_text(GTK_ENTRY(EntryPrimitive),molcasBasis.atoms[atomNumber].basis[basisNumber].primitive);
	add_widget_table(table, EntryPrimitive, i, 2);

	i = 3;
	label = gtk_label_new(" Contraction ");
	add_widget_table(table, label, i, 0);
	label = gtk_label_new(" : ");
	add_widget_table(table, label, i, 1);
	EntryContraction = gtk_entry_new();
	if(!strstr(molcasBasis.atoms[atomNumber].basis[basisNumber].contraction,"UNK"))
		gtk_entry_set_text(GTK_ENTRY(EntryContraction),molcasBasis.atoms[atomNumber].basis[basisNumber].contraction);
	add_widget_table(table, EntryContraction, i, 2);

	i = 4;
	label = gtk_label_new(" ECP Type ");
	add_widget_table(table, label, i, 0);
	label = gtk_label_new(" : ");
	add_widget_table(table, label, i, 1);
	EntryEcpType = gtk_entry_new();
	if(!strstr(molcasBasis.atoms[atomNumber].basis[basisNumber].ecpType,"UNK"))
		gtk_entry_set_text(GTK_ENTRY(EntryEcpType),molcasBasis.atoms[atomNumber].basis[basisNumber].ecpType);
	add_widget_table(table, EntryEcpType, i, 2);

	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,"Cancel");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);

	Button = create_button(WinDlg,"OK");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)editBasis,GTK_OBJECT(WinDlg));
	g_signal_connect_swapped(G_OBJECT(Button), "clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
    

	gtk_widget_show_all(WinDlg);
}
/*******************************************************************************************************/
static void addFeuille(GtkTreeIter *parent, MolcasOneBasis* molcasOneBasis, gint atomNumber, gint basisNumber)
{
	GtkTreeIter feuille;
	gchar* texts[lengthList];
	DataMolcasTree* dataTree;
	gint i;
	GtkTreeModel *model;
        GtkTreeStore *store;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
        store = GTK_TREE_STORE (model);

	texts[0] = g_strdup(" ");
	texts[1] = g_strdup(molcasOneBasis->basisName);
	if(strstr(molcasOneBasis->author,"UNK")) texts[2] = g_strdup(" ");
	else texts[2] = g_strdup(molcasOneBasis->author);

	if(strstr(molcasOneBasis->primitive,"UNK")) texts[3] = g_strdup(" ");
	else texts[3] = g_strdup(molcasOneBasis->primitive);

	if(strstr(molcasOneBasis->contraction,"UNK")) texts[4] = g_strdup(" ");
	else texts[4] = g_strdup(molcasOneBasis->contraction);

	if(strstr(molcasOneBasis->ecpType,"UNK")) texts[5] = g_strdup(" ");
	else texts[5] = g_strdup(molcasOneBasis->ecpType);

	dataTree = newDataMolcasTree(atomNumber,basisNumber);

	gtk_tree_store_append(store, &feuille, parent);
       	gtk_tree_store_set (store, &feuille, LIST_ATOM_SYMBOL, texts[0], -1);
       	gtk_tree_store_set (store, &feuille, LIST_BASIS_NAME, texts[1], -1);
       	gtk_tree_store_set (store, &feuille, LIST_AUTHOR, texts[2], -1);
       	gtk_tree_store_set (store, &feuille, LIST_PRIMITIVE, texts[3], -1);
       	gtk_tree_store_set (store, &feuille, LIST_CONTRACTION, texts[4], -1);
       	gtk_tree_store_set (store, &feuille, LIST_ECPTYPE, texts[5], -1);

       	gtk_tree_store_set (store, &feuille, LIST_DATA, dataTree, -1);

	for(i=0;i<lengthList;i++) g_free(texts[i]);
}
/********************************************************************************/
static GtkTreeIter addNode(gchar *text,gint atomNumber)
{
	GtkTreeIter node;
	gchar *texts[lengthList];
	DataMolcasTree* dataTree;
	gint i;
	GtkTreeModel *model;
        GtkTreeStore *store;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
        store = GTK_TREE_STORE (model);

	texts[0] = g_strdup(text);
	for(i=1;i<lengthList;i++) texts[i] = g_strdup(" ");

	gtk_tree_store_append(store, &node, NULL);
	dataTree = newDataMolcasTree(atomNumber,-1);
       	gtk_tree_store_set (store, &node, LIST_ATOM_SYMBOL, texts[0], -1);
       	gtk_tree_store_set (store, &node, LIST_BASIS_NAME, texts[1], -1);
       	gtk_tree_store_set (store, &node, LIST_AUTHOR, texts[2], -1);
       	gtk_tree_store_set (store, &node, LIST_PRIMITIVE, texts[3], -1);
       	gtk_tree_store_set (store, &node, LIST_CONTRACTION, texts[4], -1);
       	gtk_tree_store_set (store, &node, LIST_ECPTYPE, texts[5], -1);
       	gtk_tree_store_set (store, &node, LIST_DATA, dataTree, -1);

	for(i=0;i<lengthList;i++) g_free(texts[i]);

	return node;
}
/***********************************************************************/
static void addBasisList()
{
	gint i;
	GtkTreeIter node;
	MolcasOneBasis* molcasOneBasis;
	gint numberOfBasis = 0;
	gint j;
	
	for(i=0;i<molcasBasis.numberOfAtoms;i++)
	{
		node = addNode(molcasBasis.atoms[i].symbol,i);
		molcasOneBasis = molcasBasis.atoms[i].basis;
		numberOfBasis = molcasBasis.atoms[i].numberOfBasis;

		for(j=0;j<numberOfBasis;j++)
			addFeuille(&node,&molcasOneBasis[j],i,j);
	}
}
/********************************************************************************/
static void clearTreeView()
{
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
        GtkTreeStore *store = GTK_TREE_STORE (model);
	gtk_tree_store_clear(store);
}
/********************************************************************************/
static void rafreshTreeView()
{
	if(treeView == NULL) return;
	clearTreeView();
	addBasisList();
}
/***********************************************************************/
static void addTreeView(GtkWidget *win, GtkWidget *vbox)
{
	GtkWidget *scr;
	gint i;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	scr = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_container_add(GTK_CONTAINER(vbox), scr);      

	store = gtk_tree_store_new (lengthList+1, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
        model = GTK_TREE_MODEL (store);

	treeView = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeView), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeView), TRUE);

	for (i=0;i<lengthList;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, listTitles[i]);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, FALSE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (treeView), column);
	}

	/*
	set_base_style(treeView,30000,50000,60000);
	*/
	gtk_container_add(GTK_CONTAINER(scr), treeView);
}
/***********************************************************************/
void saveMolcasBasis()
{
        gchar *filename =  g_strdup_printf("%s/molcasbasis",gabedit_directory());  
	FILE* file;
	gint i;
	gint j;

	if(molcasBasis.numberOfAtoms<0)
		return;
	file = FOpen(filename,"w");
	fprintf(file,"Natoms =  %d\n",molcasBasis.numberOfAtoms);
	for(i=0;i<molcasBasis.numberOfAtoms;i++)
	{
		fprintf(file,"Atom  %s\n",molcasBasis.atoms[i].symbol);
		fprintf(file,"%d\n",molcasBasis.atoms[i].numberOfBasis);
		for(j=0;j<molcasBasis.atoms[i].numberOfBasis;j++)
		{
			fprintf(file,"%s ",molcasBasis.atoms[i].symbol);
			fprintf(file,"%s ",molcasBasis.atoms[i].basis[j].basisName);
			fprintf(file,"%s ",molcasBasis.atoms[i].basis[j].author);
			fprintf(file,"%s ",molcasBasis.atoms[i].basis[j].primitive);
			fprintf(file,"%s ",molcasBasis.atoms[i].basis[j].contraction);
			fprintf(file,"%s ",molcasBasis.atoms[i].basis[j].ecpType);
			fprintf(file,"\n");
		}
		
	}
	fclose(file);
}
/***********************************************************************/
void loadMolcasBasis()
{
        gchar *filename =  g_strdup_printf("%s/molcasbasis",gabedit_directory());  
	FILE* file = FOpen(filename,"r");
	gchar t[BSIZE];
	gchar symb[BSIZE];
	gchar basisName[BSIZE];
	gchar author[BSIZE];
	gchar primitive[BSIZE];
	gchar contraction[BSIZE];
	gchar ecpType[BSIZE];
	gchar dump[BSIZE];
	gint nbas = 0;
	gint natoms = 0;
	gchar* point = NULL;
	gint i;
	gint j;
	gint n;

	if(!file)
	{
		create_molcas_basis_file();
		file = FOpen(filename,"r");
	}
	if(!file)
	{
		printf("Sorry the molcasbasis is corrupted.\nPlease reinstall gabedit\n");
		return;
	}
    	{ char* e = fgets(t,BSIZE,file);} /* number of atoms */
	point = strstr(t,"=");
	if(!point)
	{
		freeMolcasBasis();
		return;
	}

	point = strstr(t,"=")+1;

	sscanf(point,"%d",&natoms);
	if(natoms<1 || natoms>200)
		return;

	molcasBasis.numberOfAtoms = natoms;
	molcasBasis.atoms = g_malloc(natoms*sizeof(MolcasAtom));

	for(i=0;i<natoms;i++)
	{
		if(!fgets(t,BSIZE,file))
		{
			printf("Sorry the molcasbasis is corrupted.\nPlease reinstall gabedit\n");
			freeMolcasBasis();
			break;
		}
		n = sscanf(t,"%s %s",dump,symb);
		if(n!=2)
		{
			printf("Sorry the molcasbasis is corrupted.\nPlease reinstall gabedit\n");
			freeMolcasBasis();
			return;
		}
		molcasBasis.atoms[i].symbol = g_strdup(symb);
		if(!fgets(t,BSIZE,file))
			break;
		sscanf(t,"%d",&nbas);
		molcasBasis.atoms[i].numberOfBasis = nbas;
		molcasBasis.atoms[i].basis = g_malloc(nbas*sizeof(MolcasOneBasis));

		for(j=0;j<nbas;j++)
		{
			if(!fgets(t,BSIZE,file))
			{
				printf("Sorry the molcasbasis is corrupted.\nPlease reinstall gabedit\n");
				freeMolcasBasis();
				break;
			}
			n = sscanf(t,"%s %s %s %s %s %s",symb, basisName, author, primitive, contraction, ecpType);
			if(n<6) sprintf(ecpType,"UNK");
			if(n<5) sprintf(contraction,"UNK");
			if(n<4) sprintf(primitive,"UNK");
			if(n<3) sprintf(author,"UNK");
			if(n<2)
			{
				printf("Sorry the molcasbasis is corrupted.\nPlease reinstall gabedit\n");
				freeMolcasBasis();
				return;
			}
			molcasBasis.atoms[i].basis[j].basisName = g_strdup(basisName);
			molcasBasis.atoms[i].basis[j].author = g_strdup(author);
			molcasBasis.atoms[i].basis[j].primitive = g_strdup(primitive);
			molcasBasis.atoms[i].basis[j].contraction = g_strdup(contraction);
			molcasBasis.atoms[i].basis[j].ecpType = g_strdup(ecpType);
		}
	}

	fclose(file);
}
/***********************************************************************/
void setMolcasBasisDlg()
{
	GtkWidget *Win;
	GtkWidget *vbox;
	GtkWidget *frame;
	GtkWidget *parentWindow = Fenetre;
	GtkUIManager *manager = NULL;

	Win= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_title(GTK_WINDOW(Win),"Set Molcas Basis");
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	SetWinDlg = Win;

	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)destroyDlg, NULL);
 
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (Win), vbox);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);

	gtk_container_add(GTK_CONTAINER(vbox),frame);  
	gtk_widget_show (frame);

	vbox = create_vbox(frame);
	gtk_widget_realize(Win);

	if(molcasBasis.numberOfAtoms==0) loadMolcasBasis();

	treeView = NULL;
	addTreeView(Win, vbox);
	rafreshTreeView();

	manager = newMenu(Win);
	g_signal_connect(treeView, "button_press_event", G_CALLBACK(eventDispatcher), manager);
	gtk_window_set_default_size (GTK_WINDOW(Win),(gint)(ScreenHeight*0.50),(gint)(ScreenHeight*0.80));
	gtk_widget_show_all(Win);
}
