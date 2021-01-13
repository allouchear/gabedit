/* MolcasBasis.c */
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
#include <unistd.h>
#include "../Common/Global.h"
#include "../Molcas/MolcasTypes.h"
#include "../Molcas/MolcasGlobal.h"
#include "../Molcas/MolcasVariables.h"
#include "../Molcas/MolcasOptimization.h"
#include "../Molcas/MolcasGateWay.h"
#include "../Molcas/MolcasScf.h"
#include "../Molcas/MolcasLib.h"
#include "../Molcas/MolcasBasis.h"
#include "../Utils/UtilsInterface.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Constants.h"


/************************************************************************************************************/
typedef enum
{
  ATOMLIST_SYMBOL = 0,
  ATOMLIST_DATA,
  ATOMLIST_NUMBER_OF_COLUMNS,
} MolCasAtomsList;

typedef enum
{
  BASISLIST_NAME = 0,
  BASISLIST_DATA,
  BASISLIST_NUMBER_OF_COLUMNS,
} MolCasBAsisList;
/************************************************************************************************************/
static GtkWidget *labelInfo = NULL;
static	GtkWidget *listOfAtoms = NULL;
static	GtkWidget *listOfBasis = NULL;
static	GtkWidget *Wins = NULL;
static	GtkWidget *hBasisBox = NULL;
static	GtkWidget *vBoxPaned = NULL;
static gint numberOfSelectedAtom = -1;
/************************************************************************************************************/
static void selectRow(GtkWidget* list, gint row)
{
	GtkTreePath *path;
	gchar* tmp = g_strdup_printf("%d",row);

	path = gtk_tree_path_new_from_string  (tmp);
	g_free(tmp);
	gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (list)), path);
	gtk_tree_path_free(path);
}
/************************************************************************************************************/
static void detroyBasisWindow()
{
	destroy(Wins,NULL);
	Wins = NULL;
	labelInfo = NULL;
	listOfAtoms = NULL;
	listOfBasis = NULL;
	hBasisBox = NULL;
	vBoxPaned = NULL;
	numberOfSelectedAtom = -1;
}
/************************************************************************************************************/
static gint setMolcasMoleculeBasis(gint numAtom)
{
	GtkTreeModel *model;
	GtkTreeIter  iter;
	gchar* base;
	gchar* pathString = g_strdup_printf("%d",numAtom);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(listOfAtoms));
	if(!gtk_tree_model_get_iter_from_string (model, &iter, pathString)) return 1;
	gtk_tree_model_get (model, &iter, ATOMLIST_DATA, &base, -1);

	if(numAtom<0) return 0;
	if(molcasMolecule.numberOfDifferentKindsOfAtoms<1) return 0;

	if(molcasMolecule.basis[numAtom]) g_free(molcasMolecule.basis[numAtom]);
	molcasMolecule.basis[numAtom] = g_strdup(base);

	resetNumberOfValenceElectrons();
	return 0;
}
/************************************************************************************************************/
static gchar* getErrorMessage()
{
	gchar *errfile= g_strdup_printf("%s%stmp%serrfile",gabedit_directory(), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S);
	gchar *terr = NULL;
	gchar *dump = NULL;
	gint taille = BSIZE;
	gchar t[BSIZE];
	FILE *fd;
	fd = FOpen(errfile, "r");
	if(!fd) 
	{
		g_free(errfile);
		return NULL;
	}
  	while(!feof(fd))
  	{
    		if(!fgets(t,taille, fd)) break;
               	dump = terr;
		if(!terr) terr = g_strdup_printf("%s",t);
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
	g_free(errfile);
	return terr;
}
/********************************************************************************/
gboolean addBasis(GtkWidget *list,char *fileName)
{
	gchar *texts[1];
	gchar t[BSIZE];
	FILE *fd;
	gboolean OK = FALSE;
	gchar* data;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        store = GTK_LIST_STORE (model);

	fd = FOpen(fileName, "r");

	if(!fd) return FALSE;

	while(!feof(fd))
	{
		if(!fgets(t,BSIZE,fd))break;
		if(strlen(t)<1) continue;
		str_delete_n(t);
		delete_last_spaces(t);
		delete_first_spaces(t);
		texts[0] =g_strdup(t);
		data =g_strdup(t);
		gtk_list_store_append(store, &iter);
		gtk_list_store_set (store, &iter, BASISLIST_NAME, texts[0], BASISLIST_DATA, (gpointer) data, -1);
	}
	fclose(fd);
	OK = TRUE;
	return OK;
}
/********************************************************************************/
static GtkWidget *createWidgetListOfBasis(gchar* fileName)
{
	GtkWidget *scrollWin = NULL;
  	GtkWidget *vbox;
  	gchar *title[1];
  	gboolean creationOfNewHBox = FALSE;
 	GtkWidget *listOfBasis = NULL;
        GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

     	title[0] = g_strdup(_("List of available basis"));
   	if(!hBasisBox)
   	{
  		creationOfNewHBox = TRUE;
  		hBasisBox = gtk_hbox_new(TRUE, 0);
  		gtk_box_pack_start (GTK_BOX (vBoxPaned), hBasisBox, TRUE, TRUE, 2);
   	}

  	vbox = gtk_vbox_new(FALSE, 0);
  	gtk_box_pack_start (GTK_BOX (hBasisBox), vbox, TRUE, TRUE, 2);
	gtk_widget_show(vbox);

    	scrollWin = gtk_scrolled_window_new(NULL, NULL);
    	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    	gtk_container_add(GTK_CONTAINER(vbox), scrollWin);

	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER); /* the second column is not visible, used for data */
        model = GTK_TREE_MODEL (store);

	listOfBasis = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (listOfBasis), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (listOfBasis), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (listOfBasis), TRUE);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, title[0]);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_attributes (column, renderer, "text", BASISLIST_NAME, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (listOfBasis), column);

	gtk_widget_set_size_request(listOfBasis,(gint)(ScreenHeight*0.4),-1);
    	gtk_container_add(GTK_CONTAINER(scrollWin), listOfBasis);


    	if(!addBasis(listOfBasis,fileName) && creationOfNewHBox ) gtk_widget_destroy(hBasisBox);
   	return (listOfBasis);
}
/********************************************************************************/
gboolean createListOfBasis(gint numAtom)
{
 	gboolean OK = TRUE;
 	gchar *terr;
#ifndef G_OS_WIN32
 	gchar *dump;
#endif
	gchar t[BSIZE];
 	FILE *fd;
 	gchar *outfile= g_strdup_printf("%s%stmp%soutfile",gabedit_directory(), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S);
 	gchar *errfile= g_strdup_printf("%s%stmp%serrfile",gabedit_directory(), G_DIR_SEPARATOR_S, G_DIR_SEPARATOR_S);

	if(numAtom > -1)
  		getMolcasBasisList(molcasMolecule.symbol[numAtom],outfile,errfile);
	else 
		return FALSE;

	
 	terr = NULL;
 	fd = FOpen(outfile, "r");
 	if(fd)
 	{
 		terr=fgets(t,BSIZE,fd);
 		fclose(fd);
 	}
 	if(terr!=NULL)
     	listOfBasis = createWidgetListOfBasis(outfile);
 	else
    	OK = FALSE;
	
#ifdef G_OS_WIN32
	unlink (errfile);
#else
	dump = g_strdup_printf("rm %s&",errfile);
	{int ierr = system(dump);}
	g_free(dump);
#endif
	g_free(errfile);
	g_free(outfile);
 	return OK;
}
/********************************************************************************/
static void selectionABasis(GtkTreeSelection *selection, gpointer data)
{
	GtkTreeModel *model;
	GtkTreeModel *modelBasis;
	GtkTreeIter  iter;
	GtkTreeIter  iterBasis;
	gchar* pathString = g_strdup_printf("%d",numberOfSelectedAtom);
	gchar* base = NULL;
	gchar* oldBasis = NULL;
	if(numberOfSelectedAtom<0) return;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(listOfAtoms));
	if(!gtk_tree_model_get_iter_from_string (model, &iter, pathString)) return;
	gtk_tree_model_get (model, &iter, ATOMLIST_DATA, &oldBasis, -1);
	g_free(oldBasis);
	g_free(pathString);

	if (!gtk_tree_selection_get_selected (selection, &modelBasis, &iterBasis)) return;
	gtk_tree_model_get (modelBasis, &iterBasis, BASISLIST_DATA, &base, -1);

       	gtk_list_store_set (GTK_LIST_STORE (model), &iter, ATOMLIST_DATA, (gpointer)(gpointer) g_strdup(base), -1);

	setMolcasMoleculeBasis(numberOfSelectedAtom);
}
/************************************************************************************************************/
void definelistOfAtoms(GtkWidget* list)
{
	gint i;
	gchar* basisName;
	gchar *texts[1];
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        store = GTK_LIST_STORE (model);

	for (i=0 ; i<molcasMolecule.numberOfDifferentKindsOfAtoms ; i++)
  	{
   		texts[0]=g_strdup(molcasMolecule.symbol[i]);
    		if(*(texts[0]) != 'X' )
    		{
    			basisName = g_strdup_printf("%s.sto-3g....",molcasMolecule.symbol[i]);
			gtk_list_store_append(store, &iter);
       			gtk_list_store_set (store, &iter, ATOMLIST_SYMBOL, texts[0], ATOMLIST_DATA, basisName, -1);
   		}
  	}
}
/********************************************************************************/
static void selectAnAtoms(GtkTreeSelection *selection, gpointer data)
{
	gchar *terr = NULL;
  	/* gint i;*/
	gboolean Ok = TRUE;

	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar* pathString;
	gint row;
	/*
	GtkTreePath* path;
	*/
	if (!gtk_tree_selection_get_selected (selection, &model, &iter)) return;

	pathString =gtk_tree_model_get_string_from_iter(model, &iter);
	/*
	path = gtk_tree_model_get_path (model,&iter);
	*/
	row = atoi(pathString);
  	numberOfSelectedAtom = row;
	terr = g_strdup_printf(_("Atom n %d is selected "), row);
	gtk_widget_show(labelInfo);
  	gtk_label_set_text( GTK_LABEL(labelInfo),terr);
	g_free(terr);
	if(hBasisBox) gtk_widget_destroy(hBasisBox);
	hBasisBox = NULL;
	Ok = createListOfBasis(numberOfSelectedAtom);
	if(Ok)
	{
		GtkTreeSelection *select;
		gtk_widget_show_all(hBasisBox);
		select = gtk_tree_view_get_selection (GTK_TREE_VIEW (listOfBasis));
		gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
		g_signal_connect (G_OBJECT (select), "changed", G_CALLBACK (selectionABasis), NULL);
		gtk_widget_hide(labelInfo);
		/*
  		gtk_label_set_text( GTK_LABEL(labelInfo),"Please select your basis");
		*/
		if(numberOfSelectedAtom>-1) selectRow(listOfBasis,0);
	}
	else
	{
		terr = getErrorMessage();
		if(terr)
		{
			gtk_widget_show(labelInfo);
			gtk_label_set_text( GTK_LABEL(labelInfo),terr);
			g_free(terr);
		}
	}
}
/************************************************************************************************************/
GtkWidget* createMolcasBasisWindow()
{
	GtkWidget *button;
	GtkWidget *dlgWin = NULL;
	GtkWidget *frame = NULL;
	GtkWidget *vBox = NULL;
	GtkWidget *hBox = NULL;
	GtkWidget *hPaned = NULL;
	GtkWidget *scrollWin = NULL;
	gchar *titleForAtoms[] = {"Atoms"};
        GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;

	if(molcasMolecule.numberOfAtoms <1)
	{
		dlgWin = Message(_("You must initially define your geometry."),_("Error"),TRUE);
		return dlgWin;
	}


	dlgWin= gtk_dialog_new ();
	gtk_window_set_position(GTK_WINDOW(dlgWin),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(dlgWin),GTK_WINDOW(Fenetre));
	gtk_window_set_title(&GTK_DIALOG(dlgWin)->window,_("Molcas Basis"));

	init_child(dlgWin, detroyBasisWindow ,_(" Molcas Basis "));
	g_signal_connect(G_OBJECT(dlgWin),"delete_event",(GCallback)destroy_children,NULL);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (GTK_BOX( GTK_DIALOG(dlgWin)->vbox)), frame, TRUE, TRUE, 5);

	gtk_widget_realize(dlgWin);

	vBox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vBox);

	hPaned = gtk_hpaned_new();
	gtk_box_pack_start (GTK_BOX (vBox), hPaned,TRUE,TRUE, 0);

	vBoxPaned = gtk_vbox_new(FALSE, 0);
	gtk_paned_add2(GTK_PANED(hPaned), vBoxPaned);

	hBox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vBoxPaned), hBox, FALSE, FALSE, 0);

	labelInfo = gtk_label_new (_("Please select a atom "));
	gtk_widget_show (labelInfo);
	gtk_box_pack_start (GTK_BOX (vBoxPaned), labelInfo, FALSE, FALSE, 0);

	scrollWin = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollWin),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);

	gtk_paned_add1(GTK_PANED(hPaned), scrollWin);
    

	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER); /* the second column is not visible, used for data */
        model = GTK_TREE_MODEL (store);

	listOfAtoms = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (listOfAtoms), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (listOfAtoms), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (listOfAtoms), TRUE);

	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, titleForAtoms[0]);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_set_attributes (column, renderer, "text", ATOMLIST_SYMBOL, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (listOfAtoms), column);

	definelistOfAtoms(listOfAtoms);


	gtk_widget_set_size_request(listOfAtoms,(gint)(ScreenHeight*0.1),(gint)(ScreenHeight*0.5)); 
	gtk_container_add(GTK_CONTAINER(scrollWin), listOfAtoms);


	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (listOfAtoms));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	g_signal_connect (G_OBJECT (select), "changed", G_CALLBACK (selectAnAtoms), NULL);

	button = create_button(dlgWin,_("Cancel"));
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(destroy_children),GTK_OBJECT(dlgWin));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_show (button);
	/*
	for(i=0;i<NHBOX;i++) BoiteHP[i] = NULL;
	for(i=0;i<NORB+1;i++) BoiteListeBaseAll[i] = NULL ;
	*/
	button = create_button(dlgWin,"OK");
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(dlgWin)->action_area), button, FALSE, TRUE, 5);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(destroy_children),GTK_OBJECT(dlgWin));

	gtk_widget_show_all(dlgWin);
	Wins = dlgWin;
	gtk_widget_set_size_request(Wins,-1 ,(gint)(ScreenHeight*0.6)); 
	if(molcasMolecule.numberOfAtoms>0) selectRow(listOfAtoms,0);
	return dlgWin;
}
