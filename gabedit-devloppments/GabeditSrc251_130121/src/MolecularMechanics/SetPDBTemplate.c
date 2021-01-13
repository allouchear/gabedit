/* SetPDBTemplate.c */
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
#include <stdio.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Utils.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../MolecularMechanics/Atom.h"
#include "../MolecularMechanics/Molecule.h"
#include "../MolecularMechanics/ForceField.h"
#include "../MolecularMechanics/MolecularMechanics.h"
#include "../Utils/UtilsInterface.h"
#include "../MolecularMechanics/PDBTemplate.h"
#include "../MolecularMechanics/LoadPDBTemplate.h"
#include "../MolecularMechanics/CreateDefaultPDBTpl.h"
#include "../Common/StockIcons.h"

#define NENTRYS 4
typedef enum
{
  E_RESIDUE=0,
  E_PDBTYPE,
  E_MMTYPE,
  E_CHARGE

} SetEntryType;

static GtkWidget* Entries[NENTRYS];

static gdouble labelWidth = 0.10;
static gdouble entryWidth = 0.15;

#define lengthListsType			4

static gchar *typeTitles[]={ "Residue", "PDB Type", "MM Type", "Charge" };

static	PDBTemplate* pdbTemplate = NULL;

static GtkWidget *SetWinDlg = NULL;
static GtkWidget *pdbTreeView = NULL;

typedef struct _DataTplTree DataTplTree;
struct _DataTplTree
{
	gint residueNumber;
	gint typeNumber;
};
static void rafreshTreeView();
typedef enum
{
  LIST_RESIDUE = 0,
  LIST_PDB_TYPE,
  LIST_MM_TYPE,
  LIST_CHARGE,
  LIST_DATA /* column for data, this column is not visible */
}ListColumnsTypes;
static gchar selectedRow[100] = "-1";
/********************************************************************************/
typedef enum
{
	MENU_NEW_RESIDUE =0,
	MENU_DELETE_RESIDUE,
	MENU_NEW_TYPE,
	MENU_EDIT_TYPE,
	MENU_DELETE_TYPE,
	MENU_SAVE,
	MENU_CLOSE
} EnumMenuType;
/********************************************************************************/
static void freeDataTplTree()
{
   	DataTplTree* data = NULL;
	gint i = 0;
	gchar* pathString = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));

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
gboolean* getExpandInfo()
{
	gint i;
	gboolean* expandeds = NULL;
	gint nNodes = 0;

	gchar* pathString = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));

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
 		expandeds[i] =  gtk_tree_view_row_expanded(GTK_TREE_VIEW(pdbTreeView), path);
		gtk_tree_path_free(path);
		i++;
		g_free(pathString);
		pathString = g_strdup_printf("%d", i);
	}
	g_free(pathString);
	return expandeds;
}
/********************************************************************************/
void setExpandeds(gboolean* expandeds, gchar* selected_row)
{
	gint i;
	gchar* pathString = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));

	if(!expandeds) return;

	i = 0;
	pathString = g_strdup_printf("%d", i);
	while (gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
	{
		if(expandeds[i])
		{
			GtkTreePath *path = gtk_tree_path_new_from_string  (pathString);
			gtk_tree_view_expand_to_path(GTK_TREE_VIEW(pdbTreeView), path);
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
			gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (pdbTreeView)), path);
			gtk_tree_path_free(path);
	}
}
/********************************************************************************/
static void destroyDlg(GtkWidget* win, gpointer data)
{
	freeDataTplTree();
	gtk_widget_destroy(win);
}
/********************************************************************************/
static DataTplTree* newDataTplTree(gint residueNumber, gint typeNumber)
{
	DataTplTree* dataTplTree;
	dataTplTree = g_malloc(sizeof(DataTplTree));
	dataTplTree->residueNumber = residueNumber;
	dataTplTree->typeNumber = typeNumber;
	return  dataTplTree;
}
/********************************************************************************/
static void newResidue(GtkWidget* w)
{
	gint residueNumber;
	gint typeNumber;
	PDBResidueTemplate* residueTemplates;
	gint numberOfResidues = 0;
	gchar *residueName;
	gboolean* expandeds;
	GtkTreeIter node;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));
   	DataTplTree* data = NULL;

	if(atoi(selectedRow)<0) return;
	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);

	expandeds = getExpandInfo();
	if(!data)
		return;

	residueNumber = data->residueNumber;
	typeNumber = data->typeNumber;

	if(typeNumber>=0)
		return;
	if(residueNumber<0)
		return;

	residueName = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entries[E_RESIDUE])));
	delete_all_spaces(residueName);
	numberOfResidues = pdbTemplate->numberOfResidues;
	residueTemplates = pdbTemplate->residueTemplates;
	numberOfResidues++;

	if(residueTemplates)
		residueTemplates = 
			g_realloc(residueTemplates,numberOfResidues*sizeof(PDBResidueTemplate));
	else
		residueTemplates = g_malloc(sizeof(PDBResidueTemplate));

	residueTemplates[numberOfResidues-1].residueName = g_strdup(residueName);
	residueTemplates[numberOfResidues-1].typeTemplates = NULL;
	residueTemplates[numberOfResidues-1].numberOfTypes = 0;

	pdbTemplate->residueTemplates = residueTemplates;
	pdbTemplate->numberOfResidues = numberOfResidues;

	freeDataTplTree();
	rafreshTreeView();
	setExpandeds(expandeds,selectedRow);

	g_free(expandeds);
	g_free(residueName);
	gtk_widget_destroy(w);
}
/********************************************************************************/
static void editType(GtkWidget* w)
{
	gint residueNumber;
	gint typeNumber;
	PDBTypeTemplate* typeTemplates; 
	G_CONST_RETURN gchar *mmType = gtk_entry_get_text(GTK_ENTRY(Entries[E_MMTYPE]));
	G_CONST_RETURN gchar *charge = gtk_entry_get_text(GTK_ENTRY(Entries[E_CHARGE]));
	gboolean* expandeds;
	GtkTreeIter node;
	GtkTreeModel *model;
   	DataTplTree* data = NULL;

	if(!isFloat(charge))
	{
		gchar t[] = "The value for 'charge' must be a number.";
		GtkWidget* w = Message(t,"Error",TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
		return;
	}

	if(atoi(selectedRow)<0) return;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));
	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);

	expandeds = getExpandInfo();

	if(!data) return;

	residueNumber = data->residueNumber;
	typeNumber = data->typeNumber;

	if(typeNumber<0)
		return;

	typeTemplates = pdbTemplate->residueTemplates[residueNumber].typeTemplates;
	if(typeTemplates[typeNumber].mmType)
		g_free(typeTemplates[typeNumber].mmType);
	typeTemplates[typeNumber].mmType = g_strdup(mmType);
	typeTemplates[typeNumber].charge = atof(charge);

	freeDataTplTree();
	rafreshTreeView();
	setExpandeds(expandeds,selectedRow);

	g_free(expandeds);
	gtk_widget_destroy(w);
}
/********************************************************************************/
static gboolean availableType(gint residueNumber,gchar*  pdbType)
{
	PDBTypeTemplate* typeTemplates; 
	gint numberOfTypes;
	gint i;

	typeTemplates = pdbTemplate->residueTemplates[residueNumber].typeTemplates;
	numberOfTypes =  pdbTemplate->residueTemplates[residueNumber].numberOfTypes;
	for(i=0;i<numberOfTypes;i++)
	{
		if(!strcmp(pdbType,typeTemplates[i].pdbType))
			return TRUE;
	}
	return FALSE;
}
/********************************************************************************/
static void newType(GtkWidget* w)
{
	gint residueNumber;
	gint typeNumber;
	PDBTypeTemplate* typeTemplates; 
	gint numberOfTypes;
	G_CONST_RETURN gchar *pdbType0 = gtk_entry_get_text(GTK_ENTRY(Entries[E_PDBTYPE]));
	gchar *pdbType = NULL;
	G_CONST_RETURN gchar *mmType  = gtk_entry_get_text(GTK_ENTRY(Entries[E_MMTYPE]));
	G_CONST_RETURN gchar *charge  = gtk_entry_get_text(GTK_ENTRY(Entries[E_CHARGE]));
	gboolean* expandeds;
	GtkTreeModel *model;
   	DataTplTree* data = NULL;
	GtkTreeIter node;

	if(!isFloat(charge))
	{
		gchar* t = _("The value for 'charge' must be a number.");
		GtkWidget* w = Message(t,"Error",TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
		return;
	}

	if(atoi(selectedRow)<0) return;
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));
	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);

	expandeds = getExpandInfo();

	if(!data) return;

	residueNumber = data->residueNumber;
	typeNumber = data->typeNumber;

	if(residueNumber<0)
		return;

	typeTemplates = pdbTemplate->residueTemplates[residueNumber].typeTemplates;
	numberOfTypes =  pdbTemplate->residueTemplates[residueNumber].numberOfTypes;

	pdbType = g_strdup(pdbType0);
	delete_all_spaces(pdbType);
	if(availableType(residueNumber,pdbType))
	{
		gchar* t = g_strdup_printf(_("Sorry, I can not add this pdb type\n%s is available."),pdbType);
		GtkWidget* w = Message(t,_("Error"),TRUE);
		gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  		gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
		g_free(t);
		return;
	}
	numberOfTypes++;
	if(typeTemplates)
		typeTemplates = g_realloc(typeTemplates,numberOfTypes*sizeof(PDBTypeTemplate));
	else
		typeTemplates = g_malloc(sizeof(PDBTypeTemplate));
	typeTemplates[numberOfTypes-1].pdbType = g_strdup(pdbType);
	typeTemplates[numberOfTypes-1].mmType = g_strdup(mmType);
	typeTemplates[numberOfTypes-1].charge = atof(charge);
	pdbTemplate->residueTemplates[residueNumber].typeTemplates = typeTemplates;
	pdbTemplate->residueTemplates[residueNumber].numberOfTypes = numberOfTypes;

	freeDataTplTree();
	rafreshTreeView();
	setExpandeds(expandeds,selectedRow);

	g_free(pdbType);
	g_free(expandeds);
	gtk_widget_destroy(w);
}
/********************************************************************************/
static void editnewDlg(GabeditSignalFunc f,gchar* title, gboolean newResidue)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint i;
	gchar **tlist;
	gint Nc;
	gint residueNumber;
	gint typeNumber;
	PDBTypeTemplate* typeTemplates; 
	gint numberOfTypes;
	GtkTreeIter node;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));
   	DataTplTree* data = NULL;

	if(atoi(selectedRow)<0) return;
	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);
	
	if(!data) return;

	residueNumber = data->residueNumber;
	typeNumber = data->typeNumber;

	if(typeNumber>=0 && newResidue) return;
	if(residueNumber<0 && !newResidue) return;

	typeTemplates = pdbTemplate->residueTemplates[residueNumber].typeTemplates;
	numberOfTypes = pdbTemplate->residueTemplates[residueNumber].numberOfTypes;

	Nc= atoi(selectedRow);

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

	Entries[E_RESIDUE] = create_label_entry(hbox,_("Residue Name : "),
		  (gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	if(Nc>=0 && !newResidue)
		gtk_entry_set_text(GTK_ENTRY(Entries[E_RESIDUE]),
				pdbTemplate->residueTemplates[residueNumber].residueName);
	else
		gtk_entry_set_text(GTK_ENTRY(Entries[E_RESIDUE]),"");

	if(newResidue)
		gtk_editable_set_editable((GtkEditable*) Entries[E_RESIDUE],TRUE);
	else
		gtk_editable_set_editable((GtkEditable*) Entries[E_RESIDUE],FALSE);

	if(!newResidue)
	{
		gint n = 0;

		hbox=create_hbox_false(vboxframe);
		Entries[E_PDBTYPE] = create_label_entry(hbox,_("PDB Type : "),
				(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
		if(Nc>=0 && typeNumber>=0)
			gtk_entry_set_text(GTK_ENTRY(Entries[E_PDBTYPE]),
				typeTemplates[typeNumber].pdbType);
		else
			gtk_entry_set_text(GTK_ENTRY(Entries[E_PDBTYPE]),"");

		gtk_editable_set_editable((GtkEditable*) Entries[E_PDBTYPE],TRUE);

		tlist=getListMMTypes(&n);

		hbox=create_hbox_false(vboxframe);
		if(n!=0)
		{
			Entries[E_MMTYPE] = create_label_combo(hbox,_("MM Type :"),tlist,n,
			TRUE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
		}
		else
			Entries[E_MMTYPE] = create_label_entry(hbox,_("MM Type :"),
			(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
		if(Nc>=0 && typeNumber>=0)
			gtk_entry_set_text(GTK_ENTRY(Entries[E_MMTYPE]),
				typeTemplates[typeNumber].mmType);
		else
			gtk_entry_set_text(GTK_ENTRY(Entries[E_MMTYPE]),"C");

		if(n !=0)
			gtk_editable_set_editable((GtkEditable*) Entries[E_MMTYPE],FALSE);
		else
			gtk_editable_set_editable((GtkEditable*) Entries[E_MMTYPE],TRUE);

  		for(i=0;i<n;i++)
	  		g_free(tlist[i]);

		hbox=create_hbox_false(vboxframe);
		Entries[E_CHARGE] = create_label_entry(hbox,_("Charge : "),
				(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
		if(Nc>=0 && typeNumber>=0)
		{
			gchar* t = g_strdup_printf("%f",typeTemplates[typeNumber].charge);
			gtk_entry_set_text(GTK_ENTRY(Entries[E_CHARGE]),t); 
			g_free(t);
		}
		else
			gtk_entry_set_text(GTK_ENTRY(Entries[E_CHARGE]),"0.0");

		gtk_editable_set_editable((GtkEditable*) Entries[E_CHARGE],TRUE);
	}


	gtk_widget_realize(WinDlg);
	Button = create_button(WinDlg,_("Cancel"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);

	Button = create_button(WinDlg,_("OK"));
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)f,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
    

	gtk_widget_show_all(WinDlg);
}
/********************************************************************************/
static void deleteOneResidue(GtkWidget *win, gpointer d)
{
	gint residueNumber;
	gint typeNumber;
	PDBTypeTemplate* typeTemplates; 
	gint numberOfTypes;
	gint i;
	gboolean* expandeds;
	GtkTreeIter node;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));
   	DataTplTree* data = NULL;

	if(atoi(selectedRow)<0) return;
	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);
	if(!data) return;

	expandeds = getExpandInfo();


	residueNumber = data->residueNumber;
	typeNumber = data->typeNumber;

	if(typeNumber>=0)
		return;
	if(residueNumber<0)
		return;

	typeTemplates = pdbTemplate->residueTemplates[residueNumber].typeTemplates;
	numberOfTypes = pdbTemplate->residueTemplates[residueNumber].numberOfTypes;

	for(i=0;i<numberOfTypes;i++)
	{
		if(typeTemplates[i].pdbType)
			g_free(typeTemplates[i].pdbType);
		if(typeTemplates[i].mmType)
			g_free(typeTemplates[i].mmType);
	}
	if(typeTemplates)
		g_free(typeTemplates);


	for(i=residueNumber;i<pdbTemplate->numberOfResidues;i++)
		pdbTemplate->residueTemplates[i] = pdbTemplate->residueTemplates[i+1];

	pdbTemplate->numberOfResidues--;

	pdbTemplate->residueTemplates = 
		g_realloc(pdbTemplate->residueTemplates,
			pdbTemplate->numberOfResidues*sizeof(PDBTypeTemplate));

	freeDataTplTree();
	rafreshTreeView();
	setExpandeds(expandeds,selectedRow);
	g_free(expandeds);
}
/********************************************************************************/
static void deleteResidueDlg(GtkWidget *win,gpointer d)
{

	gchar *format =_("Do you want to really delete \"%s\" residue ?") ;
	gchar *t =NULL;
	gint residueNumber;
	gint typeNumber;
	GtkWidget* w = NULL;
	GtkTreeIter node;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));
   	DataTplTree* data = NULL;

	if(atoi(selectedRow)<0) return;
	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);

	if(!data) return;

	residueNumber = data->residueNumber;
	typeNumber = data->typeNumber;

	if(typeNumber>=0)
		return;
	if(residueNumber<0)
		return;

	t =g_strdup_printf(format,pdbTemplate->residueTemplates[residueNumber].residueName);

	w = Continue_YesNo(deleteOneResidue, NULL,t);
	gtk_window_set_modal(GTK_WINDOW(w),TRUE);
  	gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
}
/********************************************************************************/
static void deleteOneType(GtkWidget *win,gpointer d)
{
	gint residueNumber;
	gint typeNumber;
	PDBTypeTemplate* typeTemplates; 
	gint numberOfTypes;
	gint i;
	gboolean* expandeds;
	GtkTreeIter node;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));
   	DataTplTree* data = NULL;

	if(atoi(selectedRow)<0) return;
	if(!gtk_tree_model_get_iter_from_string (model, &node, selectedRow)) return;
	gtk_tree_model_get (model, &node, LIST_DATA, &data, -1);

	expandeds = getExpandInfo();

	if(!data) return;

	residueNumber = data->residueNumber;
	typeNumber = data->typeNumber;

	if(typeNumber<0 || residueNumber<0)
		return;
	typeTemplates = pdbTemplate->residueTemplates[residueNumber].typeTemplates;
	numberOfTypes = pdbTemplate->residueTemplates[residueNumber].numberOfTypes;

	/*printf("typeNumber = %d residueNumber = %d\n",typeNumber, residueNumber);*/
	if(typeTemplates[typeNumber].pdbType)
		g_free(typeTemplates[typeNumber].pdbType);
	if(typeTemplates[typeNumber].mmType)
		g_free(typeTemplates[typeNumber].mmType);

	for(i=typeNumber;i<numberOfTypes-1;i++)
		typeTemplates[i] = typeTemplates[i+1];

	numberOfTypes--;
	typeTemplates = g_realloc(typeTemplates,numberOfTypes*sizeof(PDBTypeTemplate));

	pdbTemplate->residueTemplates[residueNumber].typeTemplates = typeTemplates ;
	pdbTemplate->residueTemplates[residueNumber].numberOfTypes = numberOfTypes;
	

	/*printf("End getExp\n");*/
	freeDataTplTree();
	rafreshTreeView();
	setExpandeds(expandeds,selectedRow);
	g_free(expandeds);
}
/********************************************************************************/
static void Traitement(guint Operation)
{
	switch(Operation)
	{
		case MENU_NEW_RESIDUE :
			editnewDlg(newResidue,_("New Residue"),TRUE);
			gtk_editable_set_editable((GtkEditable*) Entries[E_RESIDUE],TRUE);
			break;
		case MENU_DELETE_RESIDUE :
			deleteResidueDlg(NULL,NULL);
			break;
		case MENU_EDIT_TYPE :
			editnewDlg(editType,_("Edit Type"),FALSE);
			gtk_editable_set_editable((GtkEditable*) Entries[E_RESIDUE],FALSE);
			gtk_editable_set_editable((GtkEditable*) Entries[E_PDBTYPE],FALSE);
			gtk_editable_set_editable((GtkEditable*) Entries[E_CHARGE],TRUE);
			break;
		case MENU_NEW_TYPE :
			editnewDlg(newType,_("New Type"),FALSE);
			gtk_editable_set_editable((GtkEditable*) Entries[E_RESIDUE],FALSE);
			gtk_editable_set_editable((GtkEditable*) Entries[E_PDBTYPE],TRUE);
			gtk_editable_set_editable((GtkEditable*) Entries[E_CHARGE],TRUE);
			break;
		case MENU_DELETE_TYPE :
			deleteOneType(NULL,NULL);
			break;
		case MENU_SAVE :
			 savePersonalPDBTpl(NULL);
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
static void set_sensitive_menu(GtkUIManager *manager, gboolean norow, gint typeNumber)
{
	gint i;
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuPDBTpl");
	static gchar* paths[] = {
		"/MenuPDBTpl/NewResidue", "/MenuPDBTpl/DeleteResidue",
		"/MenuPDBTpl/NewType","/MenuPDBTpl/EditType", "/MenuPDBTpl/DeleteType",
		"/MenuPDBTpl/Save", "/MenuPDBTpl/Close" };

	static gboolean sensitive[] = {TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, TRUE};
	static gint o = MENU_NEW_RESIDUE;
	static gint n = MENU_CLOSE-MENU_NEW_RESIDUE;
	if (!GTK_IS_MENU (menu)) return;
	if(norow)
	{
		sensitive[MENU_NEW_RESIDUE-o] = TRUE;
		sensitive[MENU_DELETE_RESIDUE-o] = FALSE;
		sensitive[MENU_NEW_TYPE-o] = FALSE;
		sensitive[MENU_EDIT_TYPE-o] = FALSE;
		sensitive[MENU_DELETE_TYPE-o] = FALSE;
		sensitive[MENU_SAVE-o] = TRUE;
		sensitive[MENU_CLOSE-o] = TRUE;
		for(i=0;i<n;i++) set_sensitive_option(manager, paths[i], sensitive[i]);
	}
	else
	{
		if(typeNumber<0)
		{
			sensitive[MENU_NEW_RESIDUE-o] = TRUE;
			sensitive[MENU_DELETE_RESIDUE-o] = TRUE;
			sensitive[MENU_NEW_TYPE-o] = TRUE;
			sensitive[MENU_EDIT_TYPE-o] = FALSE;
			sensitive[MENU_DELETE_TYPE-o] = FALSE;
			sensitive[MENU_SAVE-o] = TRUE;
			sensitive[MENU_CLOSE-o] = TRUE;
			for(i=0;i<n;i++) set_sensitive_option(manager, paths[i], sensitive[i]);
		}
		else
		if(typeNumber>=0)
		{
			sensitive[MENU_NEW_RESIDUE-o] = FALSE;
			sensitive[MENU_DELETE_RESIDUE-o] = FALSE;
			sensitive[MENU_NEW_TYPE-o] = TRUE;
			sensitive[MENU_EDIT_TYPE-o] = TRUE;
			sensitive[MENU_DELETE_TYPE-o] = TRUE;
			sensitive[MENU_SAVE-o] = TRUE;
			sensitive[MENU_CLOSE-o] = TRUE;
			for(i=0;i<n;i++) set_sensitive_option(manager, paths[i], sensitive[i]);
		}
	}
}
/**********************************************************************************/
static gboolean show_menu_popup(GtkUIManager *manager, guint button, guint32 time)
{
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuPDBTpl");
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
	if(!strcmp(name, "NewResidue")) Traitement(MENU_NEW_RESIDUE);
	else if(!strcmp(name, "DeleteResidue")) Traitement(MENU_DELETE_RESIDUE);
	else if(!strcmp(name, "NewType")) Traitement(MENU_NEW_TYPE);
	else if(!strcmp(name, "EditType")) Traitement(MENU_EDIT_TYPE);
	else if(!strcmp(name, "DeleteType")) Traitement(MENU_DELETE_TYPE);
	else if(!strcmp(name, "Save")) Traitement(MENU_SAVE);
	else if(!strcmp(name, "Close")) Traitement(MENU_CLOSE);
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntries[] =
{
	{"NewResidue", GABEDIT_STOCK_NEW, N_("New _residue"), NULL, "New residue", G_CALLBACK (activate_action) },
	{"DeleteResidue", GABEDIT_STOCK_CUT, N_("_Delete selected residue"), NULL, "Delete selected residue", G_CALLBACK (activate_action) },
	{"NewType", GABEDIT_STOCK_NEW, N_("New _type"), NULL, "New type", G_CALLBACK (activate_action) },
	{"EditType", GABEDIT_STOCK_SELECT, N_("_Edit type"), NULL, "Edit type", G_CALLBACK (activate_action) },
	{"DeleteType", GABEDIT_STOCK_CUT, N_("_Delete selected type"), NULL, "Delete selected type", G_CALLBACK (activate_action) },
	{"Save", GABEDIT_STOCK_SAVE, N_("_Save"), NULL, "Save", G_CALLBACK (activate_action) },
	{"Close", GABEDIT_STOCK_CLOSE, N_("_Close"), NULL, "Close", G_CALLBACK (activate_action) },
};
static guint numberOfGtkActionEntries = G_N_ELEMENTS (gtkActionEntries);
/********************************************************************************/
/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiMenuInfo =
"  <popup name=\"MenuPDBTpl\">\n"
"    <separator name=\"sepMenuPopResidue\" />\n"
"    <menuitem name=\"NewResidue\" action=\"NewResidue\" />\n"
"    <menuitem name=\"DeleteResidue\" action=\"DeleteResidue\" />\n"
"    <separator name=\"sepMenuPopType\" />\n"
"    <menuitem name=\"NewType\" action=\"NewType\" />\n"
"    <menuitem name=\"EditType\" action=\"EditType\" />\n"
"    <menuitem name=\"DeleteType\" action=\"DeleteType\" />\n"
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

	actionGroup = gtk_action_group_new ("GabeditEditTypePDBTplLibrary");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntries, numberOfGtkActionEntries, NULL);

  	gtk_ui_manager_insert_action_group (manager, actionGroup, 0);

  	gtk_window_add_accel_group (GTK_WINDOW (win), gtk_ui_manager_get_accel_group (manager));
	if (!gtk_ui_manager_add_ui_from_string (manager, uiMenuInfo, -1, &error))
	{
		g_message (_("building menus failed: %s"), error->message);
		g_error_free (error);
	}
	return manager;
}
/*******************************************************************************************************************************/
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
   					DataTplTree* data = NULL;
					gtk_tree_model_get (model, &iter, LIST_DATA, &data, -1);
					if(data) set_sensitive_menu(manager, FALSE, data->typeNumber);
				}
				show_menu_popup(manager, bevent->button, bevent->time);
			}
		}
	}
	GTK_WIDGET_GET_CLASS(widget)->button_press_event(widget, event);
}
/*****************************************************************************************************************/
static void addFeuille(GtkTreeIter *parent, PDBTypeTemplate* typeTemplates, gint residueNumber, gint typeNumber)
{
	GtkTreeIter feuille;
	gchar* texts[lengthListsType];
	gchar* pdbType = typeTemplates->pdbType;
	gchar* mmType  = typeTemplates->mmType;
	gdouble charge = typeTemplates->charge;
	DataTplTree* dataTplTree;
	gint i;
	GtkTreeModel *model;
        GtkTreeStore *store;

	texts[0] = g_strdup(" ");
	texts[1] = g_strdup(pdbType);
	texts[2] = g_strdup(mmType);
	texts[3] = g_strdup_printf("%f",charge);

	dataTplTree = newDataTplTree(residueNumber,typeNumber);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));
        store = GTK_TREE_STORE (model);

	gtk_tree_store_append(store, &feuille, parent);
       	gtk_tree_store_set (store, &feuille, LIST_RESIDUE, texts[0], -1);
       	gtk_tree_store_set (store, &feuille, LIST_PDB_TYPE, texts[1], -1);
       	gtk_tree_store_set (store, &feuille, LIST_MM_TYPE, texts[2], -1);
       	gtk_tree_store_set (store, &feuille, LIST_CHARGE, texts[3], -1);
       	gtk_tree_store_set (store, &feuille, LIST_DATA, dataTplTree, -1);

	for(i=0;i<lengthListsType;i++) g_free(texts[i]);
}

/********************************************************************************/
static GtkTreeIter addNode(gchar *text,gint residueNumber)
{
	gchar *texts[lengthListsType];
	DataTplTree* dataTplTree;
	GtkTreeIter node;
	gint i;
	GtkTreeModel *model;
        GtkTreeStore *store;

	texts[0] = g_strdup(text);
	texts[1] = g_strdup(" ");
	texts[2] = g_strdup(" ");
	texts[3] = g_strdup(" ");

	uppercase(texts[0]);
	dataTplTree = newDataTplTree(residueNumber,-1);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));
        store = GTK_TREE_STORE (model);

	texts[0] = g_strdup(text);

	gtk_tree_store_append(store, &node, NULL);
       	gtk_tree_store_set (store, &node, LIST_RESIDUE, texts[0], -1);
       	gtk_tree_store_set (store, &node, LIST_PDB_TYPE, texts[1], -1);
       	gtk_tree_store_set (store, &node, LIST_MM_TYPE, texts[2], -1);
       	gtk_tree_store_set (store, &node, LIST_CHARGE, texts[3], -1);
       	gtk_tree_store_set (store, &node, LIST_DATA, dataTplTree, -1);

	for(i=0;i<lengthListsType;i++) g_free(texts[i]);

	return node;
}
/***********************************************************************/
static void addResidueList()
{
	gint i;
	GtkTreeIter node;
	PDBTypeTemplate* typeTemplates = NULL;
	gint numberOfTypes = 0;
	gint j;
	
	for(i=0;i<pdbTemplate->numberOfResidues;i++)
	{
		node = addNode(pdbTemplate->residueTemplates[i].residueName,i);
		typeTemplates = pdbTemplate->residueTemplates[i].typeTemplates;
		numberOfTypes = pdbTemplate->residueTemplates[i].numberOfTypes;

		for(j=0;j<numberOfTypes;j++)
		{
			addFeuille(&node,&typeTemplates[j],i,j);
		}

	}
}
/********************************************************************************/
static void clearTreeView()
{
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(pdbTreeView));
        GtkTreeStore *store = GTK_TREE_STORE (model);
	gtk_tree_store_clear(store);
}
/********************************************************************************/
static void rafreshTreeView()
{
	if(pdbTreeView == NULL) return;
	clearTreeView();
	addResidueList();
}
/********************************************************************************/
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

	store = gtk_tree_store_new (lengthListsType+1, G_TYPE_STRING, G_TYPE_STRING,  G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
        model = GTK_TREE_MODEL (store);

	pdbTreeView = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (pdbTreeView), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (pdbTreeView), TRUE);

	for (i=0;i<lengthListsType;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, typeTitles[i]);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, FALSE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (pdbTreeView), column);
	}

	/*
	set_base_style(pdbTreeView,30000,50000,60000);
	*/
	gtk_container_add(GTK_CONTAINER(scr), pdbTreeView);
}
/***********************************************************************/
void setPDBTemplateDlg()
{
	GtkWidget *Win;
	GtkWidget *vbox;
	GtkWidget *frame;
	GtkWidget *parentWindow = Fenetre;
	GtkUIManager *manager = NULL;

	pdbTemplate  = getPointerPDBTemplate();
	if(!pdbTemplate  )
	{
		pdbTemplate = g_malloc(sizeof(PDBTemplate));
		pdbTemplate->numberOfResidues = 0;
		pdbTemplate->residueTemplates = NULL;
		setPointerPDBTemplate(pdbTemplate);
	}

	Win= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_title(GTK_WINDOW(Win),_("Set PDB Template"));
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

	pdbTreeView = NULL;
	addTreeView(Win,vbox);
	rafreshTreeView();

	manager = newMenu(Win);
	g_signal_connect(pdbTreeView, "button_press_event", G_CALLBACK(eventDispatcher), manager);
	gtk_window_set_default_size (GTK_WINDOW(Win),(gint)(ScreenWidth*0.30),(gint)(ScreenHeight*0.80));
	gtk_widget_show_all(Win);
}
