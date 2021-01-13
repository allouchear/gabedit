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
#include "../Utils/Constants.h"
#include "../Common/Global.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Common/StockIcons.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/DrawGeom.h"

/************************************************************************************************************/
typedef struct _DataSelectionTree DataSelectionTree;
struct _DataSelectionTree
{
	gchar type[100];
	gchar name[100];
	gint number;
	gboolean checked;
};
typedef enum
{
  LIST_TYPE = 0,
  LIST_NAME,
  LIST_CHECK,
  VISIBLE_COLUMN,
  LIST_DATA /* column for data, this column is not visible */
}ListColumnsTypes;
static gchar* typeSymbols ="Symbols";
static gchar* typeMM ="MM Type";
static gchar* typePDB ="MM PDB";
static gchar* typeResidues ="Residues";
static gchar* typeResidueNumbers ="Residue#";
/********************************************************************************/
static void eventDispatcher(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	GtkTreePath *path;
	GtkTreeIter iter;
	GtkTreeModel *model;
	gchar selectedRow[100];

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
			/* printf("%s\n",selectedRow);*/
		}
	}
	GTK_WIDGET_GET_CLASS(widget)->button_press_event(widget, event);
}
/********************************************************************************/
static DataSelectionTree* newDataSelectionTree(gchar* type, gchar* name, gint number)
{
	DataSelectionTree* dataTree;
	dataTree = g_malloc(sizeof(DataSelectionTree));
	sprintf(dataTree->type,"UNK");
	if(type) sprintf(dataTree->type,"%s",type);
	sprintf(dataTree->name,"UNK");
	if(type) sprintf(dataTree->name,"%s",name);
	dataTree->number = number;
	dataTree->checked = FALSE;
	return  dataTree;
}
/********************************************************************************/
static void freeDataSelectionTree(GtkWidget *treeView)
{
   	DataSelectionTree* data = NULL;
	gint i = 0;
	gint j = 0;
	gchar* pathString = NULL;
	gchar* pathStringJ = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	pathString = g_strdup_printf("%d", i);
	while (gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
	{
		gtk_tree_model_get (model, &iter, LIST_DATA, &data, -1);
		if(data) g_free(data);
		j = 0;
		pathStringJ = g_strdup_printf("%d:%d", i,j);
		while (gtk_tree_model_get_iter_from_string (model, &iter, pathStringJ) == TRUE)
		{
			gtk_tree_model_get (model, &iter, LIST_DATA, &data, -1);
			if(data) g_free(data);
			j++;
			g_free(pathStringJ);
			pathStringJ = g_strdup_printf("%d:%d", i,j);
		}
		g_free(pathStringJ);
		i++;
		g_free(pathString);
		pathString = g_strdup_printf("%d", i);
	}
	g_free(pathString);
}
/********************************************************************************/
static void apply(GtkWidget *treeView)
{
   	DataSelectionTree* data = NULL;
	gint i = 0;
	gint j = 0;
	gint k = 0;
	gchar* pathString = NULL;
	gchar* pathStringJ = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	NFatoms = 0;
	if(NumFatoms) g_free(NumFatoms);

	NumFatoms = g_malloc(Natoms*sizeof(gint));
	k = 0;

	pathString = g_strdup_printf("%d", i);
	while (gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
	{
		/* printf("path %s\n",pathString);*/
		j = 0;
		pathStringJ = g_strdup_printf("%d:%d", i,j);
		while (gtk_tree_model_get_iter_from_string (model, &iter, pathStringJ) == TRUE)
		{
			/* printf("pathJ %s\n",pathStringJ);*/
			gtk_tree_model_get (model, &iter, LIST_DATA, &data, -1);
			if(data && data->checked)
			{
				gint ia;
				for (ia=0;ia<(gint)Natoms;ia++)
				{
					if(!geometry[ia].show) continue;
					if((!strcmp(data->type,typeSymbols) && !strcmp(geometry[ia].Prop.symbol,data->name))
					||(!strcmp(data->type,typeMM) && !strcmp(geometry[ia].mmType,data->name))
					||(!strcmp(data->type,typePDB) && !strcmp(geometry[ia].pdbType,data->name))
					||(!strcmp(data->type,typeResidues) && !strcmp(geometry[ia].Residue,data->name))
					||(!strcmp(data->type,typeResidueNumbers) && !strcmp(geometry[ia].Residue,data->name) && geometry[ia].ResidueNumber==data->number))
					{
						gint ki;
						gboolean sel = FALSE;
						for(ki=0;ki<k;ki++)
						if(NumFatoms[ki] == geometry[ia].N) { sel = TRUE; break;}
						if(!sel) NumFatoms[k++]= geometry[ia].N;
					}
				}
				/* printf("%s %s %d cheked = %d\n",data->type, data->name,data->number, data->checked);*/
			}
			j++;
			g_free(pathStringJ);
			pathStringJ = g_strdup_printf("%d:%d", i,j);
		}
		g_free(pathStringJ);
		i++;
		g_free(pathString);
		pathString = g_strdup_printf("%d", i);
	}
	g_free(pathString);
	NFatoms = k;
	if(k<1)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms=NULL;
	}
	else
	{
		NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	}
	drawGeom();
}
/********************************************************************************/
static void selUnselType(GtkWidget *treeView, gint sel, gint i)
{
	/* sel = 1 => select All
	*      = 0 => unselect all
	*      = 2 => invert selection
	*/
   	DataSelectionTree* data = NULL;
	gint j = 0;
	gchar* pathString = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
	gint column = 2;

	j = 0;
	pathString = g_strdup_printf("%d:%d", i,j);
	while (gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
	{
		/* printf("pathJ %s\n",pathString);*/
		gtk_tree_model_get (model, &iter, LIST_DATA, &data, -1);

		if(data)
		{
			if(sel == 1 ) data->checked = TRUE;
			if(sel == 0 ) data->checked = FALSE;
			if(sel == 2 ) data->checked = !data->checked;
			gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,  data->checked, -1);
		}
		j++;
		g_free(pathString);
		pathString = g_strdup_printf("%d:%d", i,j);
	}
	g_free(pathString);
}
/********************************************************************************/
/* sel = 1 => select All
*      = 0 => unselect all
*      = 2 => invert selection
*/
/*
static void selUnselAll(GtkWidget *treeView, gint sel)
{
   	DataSelectionTree* data = NULL;
	gint i = 0;
	gint j = 0;
	gchar* pathString = NULL;
	gchar* pathStringJ = NULL;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
	gint column = 2;

	pathString = g_strdup_printf("%d", i);
	while (gtk_tree_model_get_iter_from_string (model, &iter, pathString) == TRUE)
	{
		j = 0;
		pathStringJ = g_strdup_printf("%d:%d", i,j);
		while (gtk_tree_model_get_iter_from_string (model, &iter, pathStringJ) == TRUE)
		{
			gtk_tree_model_get (model, &iter, LIST_DATA, &data, -1);

			if(data)
			{
				if(sel == 1 ) data->checked = TRUE;
				if(sel == 0 ) data->checked = FALSE;
				if(sel == 2 ) data->checked = !data->checked;
				gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column,  data->checked, -1);
			}
			j++;
			g_free(pathStringJ);
			pathStringJ = g_strdup_printf("%d:%d", i,j);
		}
		g_free(pathStringJ);
		i++;
		g_free(pathString);
		pathString = g_strdup_printf("%d", i);
	}
	g_free(pathString);
}
*/
/********************************************************************************/
/*
static void invertSelection(GtkWidget *treeView)
{
	selUnselAll(treeView, 2);
}
*/
/********************************************************************************/
static void destroyDlg(GtkWidget* win, gpointer data)
{
	GtkWidget* treeView = g_object_get_data(G_OBJECT(win),"TreeView");
	if(treeView) freeDataSelectionTree(treeView);
	gtk_widget_destroy(win);
}
/********************************************************************************/
/*
static gboolean* getExpandInfo(GtkWidget *treeView)
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
*/
/********************************************************************************/
/*
static void setExpandeds(GtkWidget *treeView, gboolean* expandeds, gchar* selected_row)
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
*/
/********************************************************************************/
static void toggledRow (GtkCellRendererToggle *cell, gchar *path_string, gpointer data)
{
	GtkWidget *treeView = g_object_get_data(G_OBJECT(cell),"TreeView");
	gint column = GPOINTER_TO_INT(data);
	GtkTreeIter iter;
	GtkTreePath *path = gtk_tree_path_new_from_string (path_string);
	gboolean value = FALSE;
   	DataSelectionTree* dataTree = NULL;
	GtkTreeModel *model;

	if(column<2) return;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));

	gtk_tree_model_get_iter (model, &iter, path);
	gtk_tree_model_get (model, &iter, column, &value, LIST_DATA, &dataTree, -1);

	value = !value;
	gtk_tree_store_set (GTK_TREE_STORE (model), &iter, column, value, -1);

	gtk_tree_path_free (path);

	if(data)
	{
		dataTree->checked = value;
		/* printf("%s %s %d cheked = %d\n",dataTree->type, dataTree->name,dataTree->number, dataTree->checked);*/
	}
	if(!strstr(path_string,":")) selUnselType(treeView, value, atoi(path_string));
}
/********************************************************************************/
static void addFeuille(GtkWidget *treeView, GtkTreeIter *parent, gchar* type, gchar* name, gint number)
{
	GtkTreeIter feuille;
	DataSelectionTree* dataTree;
	GtkTreeModel *model;
        GtkTreeStore *store;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
        store = GTK_TREE_STORE (model);

	dataTree = newDataSelectionTree(type,name,number);

	gtk_tree_store_append(store, &feuille, parent);
       	gtk_tree_store_set (store, &feuille, LIST_TYPE, " ", -1);
	if(number<0) gtk_tree_store_set (store, &feuille, LIST_NAME, name, -1);
	else
	{
		gchar* txt = g_strdup_printf("%s%d",name,number+1);
		gtk_tree_store_set (store, &feuille, LIST_NAME, txt, -1);
	}
       	gtk_tree_store_set (store, &feuille, LIST_CHECK, FALSE, -1);
       	gtk_tree_store_set (store, &feuille, LIST_DATA, dataTree, -1);
       	gtk_tree_store_set (store, &feuille, VISIBLE_COLUMN, TRUE, -1);
}
/********************************************************************************/
static GtkTreeIter addNode(GtkWidget *treeView, gchar *text)
{
	GtkTreeIter node;
	DataSelectionTree* dataTree;
	GtkTreeModel *model;
        GtkTreeStore *store;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
        store = GTK_TREE_STORE (model);

	gtk_tree_store_append(store, &node, NULL);
	dataTree = newDataSelectionTree(text," ",-1);
       	gtk_tree_store_set (store, &node, LIST_TYPE, text, -1);
       	gtk_tree_store_set (store, &node, LIST_NAME, " ", -1);
       	gtk_tree_store_set (store, &node, LIST_CHECK, FALSE, -1);
       	gtk_tree_store_set (store, &node, VISIBLE_COLUMN, TRUE, -1);
       	gtk_tree_store_set (store, &node, LIST_DATA, dataTree, -1);

	return node;
}
/********************************************************************************/
gchar** getListResidueNumbers(gint* nlist, gint**nums)
{

	gchar** t = NULL;
	gint* num = NULL;
	
	gint i;
	gint j;

	*nlist = 0;
	if(Natoms<1) return NULL;

	t = g_malloc(Natoms*sizeof(gchar*));
	num = g_malloc(Natoms*sizeof(gint));
	*nlist = 1;
	t[*nlist-1] =  g_strdup(geometry0[0].Residue);
	num[*nlist-1] =  geometry0[0].ResidueNumber;


	for(i=1;i<Natoms;i++)
	{
		gboolean inList = FALSE;
		for(j=0;j<*nlist;j++)
		{
			if(!strcmp(geometry0[i].Residue, t[j]) && geometry0[i].ResidueNumber== num[j])
			{
				inList = TRUE;
				break;
			}
		}
		if(!inList)
		{
			(*nlist)++;
			t[*nlist-1] =  g_strdup(geometry0[i].Residue);
			num[*nlist-1] =  geometry0[i].ResidueNumber;
		}
	}
	t = g_realloc(t,*nlist*sizeof(gchar*));
	*nums = num;

	return t;
}
/********************************************************************************/
static void sortList(gchar** strs, gint nlist)
{
	gint i;
	gint j;
	gint k;

	for(i=0;i<nlist-1;i++)
	{
		k = i;
		for(j=i+1;j<nlist;j++)
			if(strcmp(strs[k],strs[j])>0) k = j;
		if(i != k)
		{
			gchar* tmp = strs[i];
			strs[i] = strs[k];
			strs[k] = tmp;
		}
	}
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
gchar** getListResidues(gint* nlist)
{

	gchar** t = NULL;
	
	gint i;
	gint j;

	*nlist = 0;
	if(Natoms<1) return NULL;

	t = g_malloc(Natoms*sizeof(gchar*));
	*nlist = 1;
	t[*nlist-1] =  g_strdup(geometry0[0].Residue);


	for(i=1;i<Natoms;i++)
	{
		gboolean inList = FALSE;
		for(j=0;j<*nlist;j++)
		{
			if(!strcmp(geometry0[i].Residue, t[j]))
			{
				inList = TRUE;
				break;
			}
		}
		if(!inList)
		{
			(*nlist)++;
			t[*nlist-1] =  g_strdup(geometry0[i].Residue);
		}
	}
	t = g_realloc(t,*nlist*sizeof(gchar*));
	sortList(t,*nlist);

	return t;
}
/********************************************************************************/
gchar** getListGeomMMTypes(gint* nlist)
{

	gchar** t = NULL;
	
	gint i;
	gint j;

	*nlist = 0;
	if(Natoms<1) return NULL;

	t = g_malloc(Natoms*sizeof(gchar*));
	*nlist = 1;
	t[*nlist-1] =  g_strdup(geometry0[0].mmType);


	for(i=1;i<Natoms;i++)
	{
		gboolean inList = FALSE;
		for(j=0;j<*nlist;j++)
		{
			if(!strcmp(geometry0[i].mmType, t[j]))
			{
				inList = TRUE;
				break;
			}
		}
		if(!inList)
		{
			(*nlist)++;
			t[*nlist-1] =  g_strdup(geometry0[i].mmType);
		}
	}
	t = g_realloc(t,*nlist*sizeof(gchar*));
	sortList(t,*nlist);

	return t;
}
/********************************************************************************/
gchar** getListPDBTypesFromGeom(gint* nlist)
{

	gchar** t = NULL;
	
	gint i;
	gint j;

	*nlist = 0;
	if(Natoms<1) return NULL;

	t = g_malloc(Natoms*sizeof(gchar*));
	*nlist = 1;
	t[*nlist-1] =  g_strdup(geometry0[0].pdbType);


	for(i=1;i<Natoms;i++)
	{
		gboolean inList = FALSE;
		for(j=0;j<*nlist;j++)
		{
			if(!strcmp(geometry0[i].pdbType, t[j]))
			{
				inList = TRUE;
				break;
			}
		}
		if(!inList)
		{
			(*nlist)++;
			t[*nlist-1] =  g_strdup(geometry0[i].pdbType);
		}
	}
	t = g_realloc(t,*nlist*sizeof(gchar*));
	sortList(t,*nlist);

	return t;
}
/********************************************************************************/
gchar** getListSymbols(gint* nlist)
{

	gchar** t = NULL;
	
	gint i;
	gint j;

	*nlist = 0;
	if(Natoms<1) return NULL;

	t = g_malloc(Natoms*sizeof(gchar*));
	*nlist = 1;
	t[*nlist-1] =  g_strdup(geometry0[0].Prop.symbol);


	for(i=1;i<Natoms;i++)
	{
		gboolean inList = FALSE;
		for(j=0;j<*nlist;j++)
		{
			if(!strcmp(geometry0[i].Prop.symbol, t[j]))
			{
				inList = TRUE;
				break;
			}
		}
		if(!inList)
		{
			(*nlist)++;
			t[*nlist-1] =  g_strdup(geometry0[i].Prop.symbol);
		}
	}
	t = g_realloc(t,*nlist*sizeof(gchar*));
	sortList(t,*nlist);

	return t;
}
/********************************************************************************/
static void addSymbolsList(GtkWidget *treeView)
{
	gint i;
	GtkTreeIter node;
	gint n = 0;
	gchar** list = getListSymbols(&n);
	gchar* type = typeSymbols;
	if(!list) return;
	node = addNode(treeView,type);
	for(i=0;i<n;i++)
	{
		if(list[i]) addFeuille(treeView, &node, type, list[i],-1);
		if(list[i]) g_free(list[i]);
	}
	g_free(list);
	{
		GtkTreePath *path = gtk_tree_path_new_from_string  ("0");
		gtk_tree_view_expand_to_path(GTK_TREE_VIEW(treeView), path);
		gtk_tree_path_free(path);
	}
}
/***********************************************************************/
static void addMMList(GtkWidget *treeView)
{
	gint i;
	GtkTreeIter node;
	gint n = 0;
	gchar** list = getListGeomMMTypes(&n);
	gchar* type = typeMM;
	if(!list) return;
	node = addNode(treeView,type);
	for(i=0;i<n;i++)
	{
		if(list[i]) addFeuille(treeView, &node, type, list[i],-1);
		if(list[i]) g_free(list[i]);
	}
	g_free(list);
}
/***********************************************************************/
static void addPDBList(GtkWidget *treeView)
{
	gint i;
	GtkTreeIter node;
	gint n = 0;
	gchar** list = getListPDBTypesFromGeom(&n);
	gchar* type = typePDB;
	if(!list) return;
	node = addNode(treeView,type);
	for(i=0;i<n;i++)
	{
		if(list[i]) addFeuille(treeView, &node, type, list[i],-1);
		if(list[i]) g_free(list[i]);
	}
	g_free(list);
}
/***********************************************************************/
static void addResidueList(GtkWidget *treeView)
{
	gint i;
	GtkTreeIter node;
	gint n = 0;
	gchar** list = getListResidues(&n);
	gchar* type = typeResidues;
	if(!list) return;
	node = addNode(treeView,type);
	for(i=0;i<n;i++)
	{
		if(list[i]) addFeuille(treeView, &node, type, list[i],-1);
		if(list[i]) g_free(list[i]);
	}
	g_free(list);
}
/********************************************************************************/
static void addResidueNumbersList(GtkWidget *treeView)
{
	gint i;
	GtkTreeIter node;
	gint n = 0;
	gint* num = NULL;
	gchar** list = getListResidueNumbers(&n,&num);
	gchar* type = typeResidueNumbers;
	if(!list) return;
	node = addNode(treeView,type);
	for(i=0;i<n;i++)
	{
		if(list[i]) addFeuille(treeView, &node, type, list[i],num[i]);
		if(list[i]) g_free(list[i]);
	}
	g_free(list);
	g_free(num);
}
/***********************************************************************/
static void addAllList(GtkWidget *treeView)
{
	addSymbolsList(treeView);
	addMMList(treeView);
	addPDBList(treeView);
	addResidueList(treeView);
	addResidueNumbersList(treeView);
}
/********************************************************************************/
static void clearTreeView(GtkWidget *treeView)
{
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeView));
        GtkTreeStore *store = GTK_TREE_STORE (model);
	gtk_tree_store_clear(store);
}
/********************************************************************************/
static void rafreshTreeView(GtkWidget *treeView)
{
	if(treeView == NULL) return;
	clearTreeView(treeView);
	addAllList(treeView);
}
/***********************************************************************/
static GtkWidget *addTreeView(GtkWidget *win, GtkWidget *vbox)
{
	GtkWidget *scr;
	gint i;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget *treeView = NULL;
	gchar *listTitles[]={ N_("Type"), N_("Name"), N_(" ") };
	static gint listWidths[]={15,15,8};
	gint lengthList = 3;
	gint Factor = 7;


	gint widall=0;
	
	for(i=0;i<lengthList;i++) widall += listWidths[i];

	widall = widall*Factor+10; 

	scr = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_container_add(GTK_CONTAINER(vbox), scr);     
	gtk_widget_set_size_request(scr,widall,-1);

	store = gtk_tree_store_new (lengthList+2, 
			G_TYPE_STRING, G_TYPE_STRING,  /* atom symbol and basis name */
			G_TYPE_BOOLEAN,               /* Check */
			G_TYPE_BOOLEAN, /* it is not visible, used for render others columns visible  or not */
			G_TYPE_POINTER); /* it is not visible, = data row */

        model = GTK_TREE_MODEL (store);

	treeView = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeView), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeView), FALSE);

	for (i=0;i<2;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, listTitles[i]);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, FALSE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
		gtk_tree_view_column_set_min_width(column, listWidths[i]*Factor);
		gtk_tree_view_append_column (GTK_TREE_VIEW (treeView), column);
	}
	for (i=2;i<lengthList;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, listTitles[i]);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		renderer = gtk_cell_renderer_toggle_new ();
		gtk_tree_view_column_pack_start (column, renderer, FALSE);
		gtk_tree_view_column_set_attributes (column, renderer, "active", i, "visible", VISIBLE_COLUMN,NULL);
		gtk_tree_view_column_set_min_width(column, listWidths[i]*Factor);
		gtk_tree_view_append_column (GTK_TREE_VIEW (treeView), column);
		g_object_set_data(G_OBJECT(renderer),"TreeView",treeView);
		g_signal_connect (renderer, "toggled", G_CALLBACK (toggledRow), GINT_TO_POINTER(i));
	}
	set_base_style(treeView,30000,50000,60000);

	gtk_container_add(GTK_CONTAINER(scr), treeView);
	return treeView;
}
/***********************************************************************/
void selectionDlg()
{
	GtkWidget *Win;
	GtkWidget *vbox;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *parentWindow = GeomDlg;
	GtkWidget *treeView = NULL;
	GtkWidget *button = NULL;

	Win= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_title(GTK_WINDOW(Win),_("Selection"));
	/* gtk_window_set_modal (GTK_WINDOW (Win), TRUE);*/

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

	treeView = NULL;
	treeView = addTreeView(Win, vbox);
	rafreshTreeView(treeView);

	g_signal_connect(treeView, "button_press_event", G_CALLBACK(eventDispatcher), NULL);
	g_object_set_data(G_OBJECT(Win),"TreeView",treeView);

	hbox = create_hbox_false(vbox);
	gtk_widget_realize(Win);

	/*
	button = create_button(Win,_(" Invert "));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 1);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)invertSelection,GTK_OBJECT(treeView));
	gtk_widget_show (button);
	*/

	button = create_button(Win,_("Close"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 1);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,_("Apply"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 1);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply, GTK_OBJECT(treeView));

	button = create_button(Win,_("OK"));
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 1);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);

	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply, GTK_OBJECT(treeView));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(Win));


	gtk_window_set_default_size (GTK_WINDOW(Win), -1, (gint)(ScreenHeight*0.85));
	gtk_widget_show_all(Win);
	/* fit_windows_position(GeomDlg,Win);*/
}
/********************************************************************************/
void SelectAllAtoms()
{
	gint i;
	gint k;
	if(Natoms<1) return;
	NFatoms = Natoms;
	NumFatoms = g_malloc((NFatoms)*sizeof(gint));
	k = 0;
	for (i=0;i<(gint)Natoms;i++)
		if(geometry[i].show) NumFatoms[k++] = geometry[i].N;
	NFatoms = k;
	if(k<1)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms=NULL;
	}
	else
	{
		NFatoms = k;
		NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	}

	drawGeom();
}
/*****************************************************************************/
static gboolean if_selected(gint Num)
{
	gint j;
	if(NFatoms<1 || !NumFatoms )
		return FALSE;

	for(j=0;j<(gint)NFatoms;j++)
		if(NumFatoms[j] == (gint)geometry[Num].N)
			return TRUE;
	return FALSE;
}
/********************************************************************************/
void InvertSelectionOfAtoms()
{
	gint i;
	gint n = 0;
	gint* num = NULL;
	gint k = 0;
	if(Natoms<1) return;
	n = Natoms - NFatoms;
	if(n<=0)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms = NULL;
		RebuildGeom=TRUE;
		drawGeom();
		return;
	}
	else
		num = g_malloc(n*sizeof(gint));

	k = 0;
	for (i=0;i<(gint)Natoms;i++)
		if(!if_selected(i) && geometry[i].show) 
		{
			num[k] = geometry[i].N;
			k++;
			if(k>=n) break;
		}

	n = k;
	if(n>0) num = g_realloc(num,n*sizeof(gint));
	if(NumFatoms) g_free(NumFatoms);
	NumFatoms = num;
	NFatoms = n;
	RebuildGeom=TRUE;
	drawGeom();
}
/********************************************************************************/
void unSelectAllAtoms()
{
	if(Natoms<1) return;
	if(NFatoms<1) return;
	NFatoms = 0;
	if(NumFatoms) g_free(NumFatoms);
	NumFatoms = NULL;
	drawGeom();
}
/*****************************************************************************/
void SelectLayerAtoms(GabEditLayerType layer)
{
	gint i;
	gint k = 0;
	if(Natoms<1) return;
	NFatoms = 0;
	if(NumFatoms) g_free(NumFatoms);

	NumFatoms = g_malloc(Natoms*sizeof(gint));
	k = 0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		if(layer==geometry[i].Layer)
			NumFatoms[k++]= geometry[i].N;
	}
	NFatoms = k;
	if(k<1)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms=NULL;
	}
	else
	{
		NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	}
	drawGeom();
}
/********************************************************************************/
void SelectFixedVariableAtoms(gboolean variable)
{
	gint i;
	gint k = 0;
	if(Natoms<1) return;
	NFatoms = 0;
	if(NumFatoms) g_free(NumFatoms);

	NumFatoms = g_malloc(Natoms*sizeof(gint));
	k = 0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		if(variable==geometry[i].Variable)
			NumFatoms[k++]= geometry[i].N;
	}
	NFatoms = k;
	if(k<1)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms=NULL;
	}
	else
	{
		NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	}
	drawGeom();
}
/********************************************************************************/
void SelectFirstResidue()
{
	gint i;
	gint k = 0;
	if(Natoms<1) return;
	NFatoms = 0;
	if(NumFatoms) g_free(NumFatoms);

	NumFatoms = g_malloc(Natoms*sizeof(gint));
	k = 0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		if(0==geometry[i].ResidueNumber)
			NumFatoms[k++]= geometry[i].N;
	}
	NFatoms = k;
	if(k<1)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms=NULL;
	}
	else
	{
		NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	}
	drawGeom();
}
/********************************************************************************/
void SelectLastResidue()
{
	gint i;
	gint k = 0;
	gint rMax = 0;
	if(Natoms<1) return;

	for (i=1;i<(gint)Natoms;i++)
	{
		if(rMax<geometry[i].ResidueNumber)
			rMax = geometry[i].ResidueNumber;
	}
	NFatoms = 0;
	if(NumFatoms) g_free(NumFatoms);

	NumFatoms = g_malloc(Natoms*sizeof(gint));
	k = 0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		if(rMax==geometry[i].ResidueNumber)
			NumFatoms[k++]= geometry[i].N;
	}
	NFatoms = k;
	if(k<1)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms=NULL;
	}
	else
	{
		NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	}
	drawGeom();
}
/********************************************************************************/
static void selectResidueByName(GtkWidget* button, GtkWidget* entry)
{
	gint i;
	gint k = 0;
	G_CONST_RETURN gchar *rName;


	if(Natoms<1) return;
	rName = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(rName)<1) return;

	NFatoms = 0;
	if(NumFatoms) g_free(NumFatoms);

	NumFatoms = g_malloc(Natoms*sizeof(gint));
	k = 0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		if(!strcmp(geometry[i].Residue,rName))
			NumFatoms[k++]= geometry[i].N;
	}
	NFatoms = k;
	if(k<1)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms=NULL;
	}
	else
	{
		NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	}
	drawGeom();
}
/********************************************************************************/
void selectResidueByNameDlg()
{
	GtkWidget *winDlg;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint n=0;
	gchar** t = NULL;
  
	winDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Select by Residue name"));
	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(GeomDlg));

	add_child(GeomDlg,winDlg,gtk_widget_destroy,_(" Sel. Res. "));
	g_signal_connect(G_OBJECT(winDlg),"delete_event",(GCallback)delete_child,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	hbox=create_hbox_false(vboxframe);
	n=0;
	t = getListResidues(&n);
	entry = create_label_combo(hbox,_(" Residue Name : "),t,n, TRUE,-1,-1);
	gtk_editable_set_editable((GtkEditable*) entry,TRUE);
	if(t) freeList(t,n);

	gtk_widget_realize(winDlg);

	button = create_button(winDlg,_("Cancel"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	button = create_button(winDlg,_("OK"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)selectResidueByName,entry);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
    

	gtk_widget_show_all(winDlg);
}
/********************************************************************************/
static void selectResidueByNumber(GtkWidget* button, GtkWidget* entry)
{
	gint i;
	gint k = 0;
	G_CONST_RETURN gchar *rName;
	gint n = 1;


	if(Natoms<1) return;
	rName = gtk_entry_get_text(GTK_ENTRY(entry));
	n = atoi(rName);

	NFatoms = 0;
	if(NumFatoms) g_free(NumFatoms);

	NumFatoms = g_malloc(Natoms*sizeof(gint));
	k = 0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		if(geometry[i].ResidueNumber == n-1)
			NumFatoms[k++]= geometry[i].N;
	}
	NFatoms = k;
	if(k<1)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms=NULL;
	}
	else
	{
		NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	}
	drawGeom();
}
/********************************************************************************/
void selectResidueByNumberDlg()
{
	GtkWidget *winDlg;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint n=0;
	gchar** t = NULL;
  
	winDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Select by Residue number"));
	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(GeomDlg));

	add_child(GeomDlg,winDlg,gtk_widget_destroy,_(" Sel. Res. "));
	g_signal_connect(G_OBJECT(winDlg),"delete_event",(GCallback)delete_child,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	hbox=create_hbox_false(vboxframe);
	n=1;
	t = g_malloc(2*sizeof(gchar*));
	t[0] = g_strdup("1");
	t[1] = NULL;
	entry = create_label_combo(hbox,_(" Residue Number : "),t,n, TRUE,-1,-1);
	gtk_editable_set_editable((GtkEditable*) entry,TRUE);
	if(t) freeList(t,n);

	gtk_widget_realize(winDlg);

	button = create_button(winDlg,_("Cancel"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	button = create_button(winDlg,_("OK"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)selectResidueByNumber,entry);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
    

	gtk_widget_show_all(winDlg);
}
/********************************************************************************/
static void selectAtomsByMMType(GtkWidget* button, GtkWidget* entry)
{
	gint i;
	gint k = 0;
	G_CONST_RETURN gchar *rName;


	if(Natoms<1) return;
	rName = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(rName)<1) return;

	NFatoms = 0;
	if(NumFatoms) g_free(NumFatoms);

	NumFatoms = g_malloc(Natoms*sizeof(gint));
	k = 0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		if(!strcmp(geometry[i].mmType,rName))
			NumFatoms[k++]= geometry[i].N;
	}
	NFatoms = k;
	if(k<1)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms=NULL;
	}
	else
	{
		NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	}
	drawGeom();
}
/********************************************************************************/
void selectAtomsByMMTypeDlg()
{
	GtkWidget *winDlg;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint n=0;
	gchar** t = NULL;
  
	winDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Select by Type"));
	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(GeomDlg));

	add_child(GeomDlg,winDlg,gtk_widget_destroy,_(" Sel. Res. "));
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
	gtk_editable_set_editable((GtkEditable*) entry,TRUE);
	if(t) freeList(t,n);

	gtk_widget_realize(winDlg);

	button = create_button(winDlg,_("Cancel"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	button = create_button(winDlg,_("OK"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)selectAtomsByMMType,entry);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
    

	gtk_widget_show_all(winDlg);
}
/********************************************************************************/
static void selectAtomsByPDBType(GtkWidget* button, GtkWidget* entry)
{
	gint i;
	gint k = 0;
	G_CONST_RETURN gchar *rName;


	if(Natoms<1) return;
	rName = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(rName)<1) return;

	NFatoms = 0;
	if(NumFatoms) g_free(NumFatoms);

	NumFatoms = g_malloc(Natoms*sizeof(gint));
	k = 0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		if(!strcmp(geometry[i].pdbType,rName))
			NumFatoms[k++]= geometry[i].N;
	}
	NFatoms = k;
	if(k<1)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms=NULL;
	}
	else
	{
		NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	}
	drawGeom();
}
/********************************************************************************/
void selectAtomsByPDBTypeDlg()
{
	GtkWidget *winDlg;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint n=0;
	gchar** t = NULL;
  
	winDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Select by PDB Type"));
	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(GeomDlg));

	add_child(GeomDlg,winDlg,gtk_widget_destroy,_(" Sel. Res. "));
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
	gtk_editable_set_editable((GtkEditable*) entry,TRUE);
	if(t) freeList(t,n);

	gtk_widget_realize(winDlg);

	button = create_button(winDlg,_("Cancel"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	button = create_button(winDlg,_("OK"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)selectAtomsByPDBType,entry);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
    

	gtk_widget_show_all(winDlg);
}
/********************************************************************************/
static void selectAtomsBySymbol(GtkWidget* button, GtkWidget* entry)
{
	gint i;
	gint k = 0;
	G_CONST_RETURN gchar *rName;


	if(Natoms<1) return;
	rName = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(rName)<1) return;

	NFatoms = 0;
	if(NumFatoms) g_free(NumFatoms);

	NumFatoms = g_malloc(Natoms*sizeof(gint));
	k = 0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		if(!strcmp(geometry[i].Prop.symbol,rName))
			NumFatoms[k++]= geometry[i].N;
	}
	NFatoms = k;
	if(k<1)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms=NULL;
	}
	else
	{
		NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	}
	drawGeom();
}
/********************************************************************************/
void selectAtomsBySymbolDlg()
{
	GtkWidget *winDlg;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint n=0;
	gchar** t = NULL;
  
	winDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Select by Symbol"));
	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(GeomDlg));

	add_child(GeomDlg,winDlg,gtk_widget_destroy,_(" Sel. Res. "));
	g_signal_connect(G_OBJECT(winDlg),"delete_event",(GCallback)delete_child,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	hbox=create_hbox_false(vboxframe);
	n=0;
	t = getListSymbols(&n);
	entry = create_label_combo(hbox,_(" Symbol : "),t,n, TRUE,-1,-1);
	gtk_editable_set_editable((GtkEditable*) entry,TRUE);
	if(t) freeList(t,n);

	gtk_widget_realize(winDlg);

	button = create_button(winDlg,_("Cancel"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	button = create_button(winDlg,_("OK"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)selectAtomsBySymbol,entry);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
    

	gtk_widget_show_all(winDlg);
}
/********************************************************************************/
void selectAtomsByChargeValues(gboolean positive)
{
	gint i;
	gint k = 0;

	if(Natoms<1) return;
	NFatoms = 0;
	if(NumFatoms) g_free(NumFatoms);

	NumFatoms = g_malloc(Natoms*sizeof(gint));
	k = 0;
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		if(
			(geometry[i].Charge>0 && positive) ||
			(geometry[i].Charge<0 && !positive)
		)
			NumFatoms[k++]= geometry[i].N;
	}
	NFatoms = k;
	if(k<1)
	{
		NFatoms = 0;
		if(NumFatoms) g_free(NumFatoms);
		NumFatoms=NULL;
	}
	else
	{
		NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	}
	drawGeom();
}
/********************************************************************************/
static void selectAtomsBySphere(GtkWidget* button, GtkWidget* entry)
{
	gint i;
	gint j;
	gint k = 0;
	G_CONST_RETURN gchar *rName;
	gdouble r = 1.0;
	gint* nums = NULL;


	if(Natoms<1) return;
	if(NFatoms<1) return;
	rName = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(rName)<1) return;
	r = atof(rName)*ANG_TO_BOHR;
	r = r*r;

	nums = g_malloc(Natoms*sizeof(gint));
	for (k=0;k<(gint)NFatoms;k++) nums[k] = NumFatoms[k];
		
	for (i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		for(j=0;j<k;j++) if(nums[j] == geometry[i].N) break;
		if(j!=k) continue;/* already selected */
		for(j=0;j<NFatoms;j++)
		{
			gint nj = 0;
			for(nj=0;nj<Natoms;nj++)
				if(geometry[nj].N==NumFatoms[j]) break;
			if(nj<Natoms)
			{
				gdouble x = geometry[i].X-geometry[nj].X;
				gdouble y = geometry[i].Y-geometry[nj].Y;
				gdouble z = geometry[i].Z-geometry[nj].Z;
				if(x*x+y*y+z*z<r)
				{
					nums[k++]= geometry[i].N;
					break;
				}
			}
		}
	}
	if(NumFatoms) g_free(NumFatoms);
	NFatoms = k;
	NumFatoms = nums;
	NumFatoms = g_realloc(NumFatoms,NFatoms*sizeof(gint));
	drawGeom();
}
/********************************************************************************/
void selectAtomsBySphereDlg()
{
	GtkWidget *winDlg;
	GtkWidget *button;
	GtkWidget *hbox;
	GtkWidget *entry;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint n=0;
	gchar** t = NULL;

	if(Natoms<1) return;
	if(NFatoms<1) 
	{
		return;
	}
  
	winDlg = gtk_dialog_new();
	gtk_window_set_title(GTK_WINDOW(winDlg),_("Select by sphere"));
	gtk_window_set_position(GTK_WINDOW(winDlg),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(winDlg),GTK_WINDOW(GeomDlg));

	add_child(GeomDlg,winDlg,gtk_widget_destroy,_(" Sel. Res. "));
	g_signal_connect(G_OBJECT(winDlg),"delete_event",(GCallback)delete_child,NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

	gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	hbox=create_hbox_false(vboxframe);
	n=1;
	t = g_malloc(2*sizeof(gchar));
	t[0] = g_strdup_printf("2.0");
	t[1] = NULL;
	entry = create_label_combo(hbox,_(" Radius(Ang) : "),t,n, TRUE,-1,-1);
	gtk_editable_set_editable((GtkEditable*) entry,TRUE);
	if(t) freeList(t,n);

	gtk_widget_realize(winDlg);

	button = create_button(winDlg,_("Cancel"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);

	button = create_button(winDlg,_("OK"));
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(winDlg)->action_area), button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)selectAtomsBySphere,entry);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child,GTK_OBJECT(winDlg));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);

	gtk_widget_show_all(winDlg);
}
