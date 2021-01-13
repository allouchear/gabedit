/* SetMMParameters.c */
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
#include "../Utils/UtilsInterface.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "Atom.h"
#include "Molecule.h"
#include "ForceField.h"
#include "MolecularMechanics.h"
#include "../Common/StockIcons.h"

#define NENTRYS 21
typedef enum
{
  E_S=0,
  E_T=1,
  E_M=2,
  E_P=3,
  E_T1=4,
  E_T2=5,
  E_T3=6,
  E_T4=7,
  E_F=8,
  E_R=9,
  E_A=10,
  E_C=11,
  E_D=12,
  E_E=13,
  E_B=14,
  E_N=15,
  E_BETA=16,
  E_C6  =17,
  E_C8  =18,
  E_C10 =19,
  E_DESC = 20,
} SetEntryType;
typedef enum
{
	MMNUMBERTYPE = 0,
	MMNUMBERBOND = 1,
	MMNUMBERBEND = 2,
	MMNUMBERTORSION = 3,
	MMNUMBERIMPROPER = 4,
	MMNUMBERHBOND = 5,
	MMNUMBERNONBONDED = 6,
	MMNUMBERPAIRWISE = 7,
} MMNumberLists;

static gint Factor = 7; 

#define lengthListsType			5
#define lengthListsBond			4
#define lengthListsBend			5
#define lengthListsTorsion		8
#define lengthListsImproper		7
#define lengthListsHBond		4
#define lengthListsNonBonded		3
#define lengthListsPairWise		8

static gchar *typeTitles[]={ "Type", "Symbol", "Masse", "Polarisability","Description" };
static gint typeWidths[]={8,8,10,10,10}; 

static gchar *bondTitles[]={ "Type1", "Type2", "Force", "Equilibrium Distance" };
static gint bondWidths[]  ={8,8,15,15}; 

static gchar *bendTitles[]={ "Type1", "Type2", "Type3","Force", "Equilibrium Angle" };
static gint bendWidths  []={8,8,8,15,15}; 

static gchar *torsionTitles[]={ "Type1", "Type2", "Type3","Type4","Divisor",
				"Barrier","Phase","Frequence" 
			      };
static gint torsionWidths  []={8,8,8,8,10,15,15,15}; 

static gchar *improperTitles[]={ "Type1", "Type2", "Type3","Type4","Barrier","Phase","Frequence" };
static gint improperWidths  []={8,8,8,8,15,15,15}; 

static gchar *hBondTitles[]={ "Type1", "Type2", "C(C/r**12)", "D(-D/r**10)" };
static gint hBondWidths[]  ={8,8,15,15}; 

static gchar *nonBondedTitles[]={ "Type","R","Epsilon"};
static gint nonBondedWidths[]  ={8,15,15}; 

static gchar *pairWiseTitles[]={ "Type1", "Type2", "A", "Beta","C6","C8","C10","b"};
static gint pairWiseWidths[]  ={8,8,15,15,15,15,15,15}; 

static GtkWidget *listTypes = NULL;
static GtkWidget *listBonds = NULL;
static GtkWidget *listBends = NULL;
static GtkWidget *listTorsions = NULL;
static GtkWidget *listImpropers = NULL;
static GtkWidget *listHBonds = NULL;
static GtkWidget *listNonBondeds = NULL;
static GtkWidget *listPairWises = NULL;
static GtkWidget *NoteBook = NULL;
static GtkWidget *SetWinDlg = NULL;
static GtkStyle* style1 = NULL;
static GtkStyle* style2 = NULL;


static AmberParameters* amberParameters = NULL;

static gchar rowSelectedLists[][100] ={"-1","-1","-1","-1","-1","-1","-1","-1"}; 

static GtkWidget* Entries[NENTRYS];

/********************************************************************************/
static void newDlg(GtkWidget *win,gpointer data);
static void editDlg(GtkWidget *win,gpointer data);
static void deleteDlg(GtkWidget *win,gpointer data);
static void destroyDlg(GtkWidget *win, gpointer data);
/********************************************************************************/
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
static void set_sensitive_option(GtkUIManager *manager, gchar* path, gboolean sensitive)
{
	GtkWidget *wid = gtk_ui_manager_get_widget (manager, path);
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, sensitive);
}
/**********************************************************************************/
static gboolean show_menu_popup(GtkUIManager *manager, guint button, guint32 time)
{
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuMM");
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
	if(!strcmp(name, "New")) newDlg(NULL, NULL);
	else if(!strcmp(name, "Edit")) editDlg(NULL, NULL);
	else if(!strcmp(name, "Delete")) deleteDlg(NULL, NULL);
	else if(!strcmp(name, "Save")) saveAmberParameters(); 
	else if(!strcmp(name, "Close")) destroyDlg(NULL, NULL);
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntries[] =
{
	{"New", GABEDIT_STOCK_NEW, "_New", NULL, "New", G_CALLBACK (activate_action) },
	{"Edit", GABEDIT_STOCK_SELECT, "_Edit", NULL, "Edit", G_CALLBACK (activate_action) },
	{"Delete", GABEDIT_STOCK_CUT, "Delete", NULL, "Delete", G_CALLBACK (activate_action) },
	{"Save", GABEDIT_STOCK_SAVE, "_Save", NULL, "Save", G_CALLBACK (activate_action) },
	{"Close", GABEDIT_STOCK_CLOSE, "_Close", NULL, "Close", G_CALLBACK (activate_action) },
};
static guint numberOfGtkActionEntries = G_N_ELEMENTS (gtkActionEntries);
/********************************************************************************/
/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiMenuInfo =
"  <popup name=\"MenuMM\">\n"
"    <separator name=\"sepMenuPopNew\" />\n"
"    <menuitem name=\"New\" action=\"New\" />\n"
"    <menuitem name=\"Edit\" action=\"Edit\" />\n"
"    <menuitem name=\"Delete\" action=\"Delete\" />\n"
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

	actionGroup = gtk_action_group_new ("GabeditEditMMLibrary");
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
/********************************************************************************/
static void event_dispatcher(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
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
			gint nList = gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBook));
			model = gtk_tree_view_get_model(GTK_TREE_VIEW(widget));
			gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (widget)), path);
			sprintf(rowSelectedLists[nList] ,"%s",gtk_tree_path_to_string(path));
			gtk_tree_model_get_iter (model, &iter, path);
			gtk_tree_path_free(path);
			if (event->type == GDK_2BUTTON_PRESS) editDlg(NULL, NULL);
			else
  			if (event->type == GDK_BUTTON_PRESS && ((GdkEventButton *) event)->button == 3)
			{
				GdkEventButton *bevent = (GdkEventButton *) event;
  				GtkUIManager *manager = GTK_UI_MANAGER(user_data);
				set_sensitive_option(manager, "/MenuMM/Edit", TRUE);
				set_sensitive_option(manager, "/MenuMM/Delete", TRUE);
				show_menu_popup(manager, bevent->button, bevent->time);
			}
		}
	}
	GTK_WIDGET_GET_CLASS(widget)->button_press_event(widget, event);
}
/*****************************************************************************
*  AddMenu
******************************************************************************/
static void newMenuToList(GtkWidget* list, GtkWidget* win)
{
	GtkUIManager *manager = newMenu(win);
  	g_signal_connect(G_OBJECT (list), "button_press_event", G_CALLBACK(event_dispatcher), manager);      
}
/********************************************************************************/
static void setStyles()
{
  style1 = gtk_style_copy( gtk_widget_get_default_style ());
  style2 = gtk_style_copy( gtk_widget_get_default_style ());
  
  style1->base[0].red   = 60000;
  style1->base[0].green = 60000;
  style1->base[0].blue  = 60000;

  style2->base[0].red   = 50000;
  style2->base[0].green = 50000;
  style2->base[0].blue  = 50000;
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
static gchar** getLocalListTypes(gint* nlist)
{

	gchar** t =g_malloc(amberParameters->numberOfTypes*sizeof(gchar*));
	gint i;

	*nlist = amberParameters->numberOfTypes;

	for(i=0;i<amberParameters->numberOfTypes;i++)
		t[i] = g_strdup(amberParameters->atomTypes[i].name);

	return t;
}
/********************************************************************************/
static void setAtom(GtkWidget *w,gpointer data)
{
  gtk_entry_set_text(GTK_ENTRY(Entries[E_S]),(char *)data);
}
/********************************************************************************/
static void SelectAtom(GtkWidget *w,gpointer entry0)
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
          g_signal_connect_swapped(GTK_OBJECT(button), "clicked",
                            (GCallback)gtk_widget_destroy,GTK_OBJECT(FenetreTable));
	  gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	  }

  }
 	
  gtk_widget_show_all(FenetreTable);
  
}
/**********************************************************************/
static gint getNumberType(AmberParameters* amberParameters, const gchar* type)
{
	gint i;
	gint nTypes = amberParameters->numberOfTypes;
	AmberAtomTypes* types = amberParameters->atomTypes;
	gint len = strlen(type);

	if(strcmp(type,"X")==0)
		return -1;
	for(i=0;i<nTypes;i++)
	{
		if(len == (gint)strlen(types[i].name) && strstr(types[i].name,type))
			return types[i].number;

	}
	return -2;
}
/**********************************************************************/
static gchar* getType(AmberParameters* amberParameters, gint number)
{
	gint i;
	gint nTypes = amberParameters->numberOfTypes;
	AmberAtomTypes* types = amberParameters->atomTypes;

	if(number == -1)
		return "X";
	for(i=0;i<nTypes;i++)
	{
		if(number == types[i].number)
		       return types[i].name;

	}
	return "Unk";
}
/********************************************************************************/
static void rafreshListPairWises(gint nn)
{
	gchar *texts[lengthListsPairWise];
	gint i;
	gint n = 0;
	gchar* type;
	GtkWidget* list = listPairWises;
	gint k;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        store = GTK_LIST_STORE (model);
	gtk_list_store_clear(store);

	for(i=0;i<amberParameters->numberOfPairWise;i++)
	{
		n = amberParameters->pairWiseTerms[i].numbers[0];
		type = getType(amberParameters,n);
		texts[0] = g_strdup(type);

		n = amberParameters->pairWiseTerms[i].numbers[1];
		type = getType(amberParameters,n);
		texts[1] = g_strdup(type);

		texts[2] = g_strdup_printf("%f",amberParameters->pairWiseTerms[i].a);
		texts[3] = g_strdup_printf("%f",amberParameters->pairWiseTerms[i].beta);
		texts[4] = g_strdup_printf("%f",amberParameters->pairWiseTerms[i].c6);
		texts[5] = g_strdup_printf("%f",amberParameters->pairWiseTerms[i].c8);
		texts[6] = g_strdup_printf("%f",amberParameters->pairWiseTerms[i].c10);
		texts[7] = g_strdup_printf("%f",amberParameters->pairWiseTerms[i].b);
		gtk_list_store_append(store, &iter);
		for(k=0;k<lengthListsPairWise;k++)
		{
			gtk_list_store_set (store, &iter, k, texts[k], -1);
		}
  	}
	if(nn>=0) selectRow(list, nn);
}
/********************************************************************************/
static void rafreshListNonBondeds(gint nn)
{
	gchar *texts[lengthListsNonBonded];
	gint i;
	gint n = 0;
	gchar* type;
	GtkWidget* list = listNonBondeds;
	gint k;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        store = GTK_LIST_STORE (model);
	gtk_list_store_clear(store);
	
	for(i=0;i<amberParameters->numberOfNonBonded;i++)
	{
		n = amberParameters->nonBondedTerms[i].number;
		type = getType(amberParameters,n);
		texts[0] = g_strdup(type);

		texts[1] = g_strdup_printf("%f",amberParameters->nonBondedTerms[i].r);
		texts[2] = g_strdup_printf("%f",amberParameters->nonBondedTerms[i].epsilon);
		gtk_list_store_append(store, &iter);
		for(k=0;k<lengthListsNonBonded;k++)
		{
			gtk_list_store_set (store, &iter, k, texts[k], -1);
		}
  	
  	}
	if(nn>=0) selectRow(list, nn);
}
/********************************************************************************/
static void rafreshListHBonds(gint nn)
{
	gchar *texts[lengthListsHBond];
	gint i;
	gint n = 0;
	gchar* type;
	GtkWidget* list = listHBonds;
	gint k;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        store = GTK_LIST_STORE (model);
	gtk_list_store_clear(store);
	
	for(i=0;i<amberParameters->numberOfHydrogenBonded;i++)
	{
		n = amberParameters->hydrogenBondedTerms[i].numbers[0];
		type = getType(amberParameters,n);
		texts[0] = g_strdup(type);

		n = amberParameters->hydrogenBondedTerms[i].numbers[1];
		type = getType(amberParameters,n);
		texts[1] = g_strdup(type);

		texts[2] = g_strdup_printf("%f",amberParameters->hydrogenBondedTerms[i].c);
		texts[3] = g_strdup_printf("%f",amberParameters->hydrogenBondedTerms[i].d);
		gtk_list_store_append(store, &iter);
		for(k=0;k<lengthListsHBond;k++)
		{
			gtk_list_store_set (store, &iter, k, texts[k], -1);
		}

	}
	if(nn>=0) selectRow(list, nn);
}
/**********************************************************************/
static void rafreshListImpropers(gint nn)
{
	gchar *texts[lengthListsImproper];
	gint i;
	gint n = 0;
	gchar* type;
	GtkWidget* list = listImpropers;
	gint k;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        store = GTK_LIST_STORE (model);
	gtk_list_store_clear(store);
	
	for(i=0;i<amberParameters->numberOfImproperTorsionTerms;i++)
	{
		n = amberParameters->improperTorsionTerms[i].numbers[0];
		type = getType(amberParameters,n);
		texts[0] = g_strdup(type);

		n = amberParameters->improperTorsionTerms[i].numbers[1];
		type = getType(amberParameters,n);
		texts[1] = g_strdup(type);

		n = amberParameters->improperTorsionTerms[i].numbers[2];
		type = getType(amberParameters,n);
		texts[2] = g_strdup(type);

		n = amberParameters->improperTorsionTerms[i].numbers[3];
		type = getType(amberParameters,n);
		texts[3] = g_strdup(type);

		texts[4] = g_strdup_printf("%f",amberParameters->improperTorsionTerms[i].barrier);
		texts[5] = g_strdup_printf("%f",amberParameters->improperTorsionTerms[i].phase);
		texts[6] = g_strdup_printf("%f",amberParameters->improperTorsionTerms[i].n);
		gtk_list_store_append(store, &iter);
		for(k=0;k<lengthListsImproper;k++)
		{
			gtk_list_store_set (store, &iter, k, texts[k], -1);
		}


	}
	if(nn>=0) selectRow(list, nn);
}
/**********************************************************************/
static void rafreshListTorsions(gint nn)
{
	gint i;
	gint j;
	gint n = 0;
	gchar* type;
	GtkWidget* list = listTorsions;
	gchar *texts[lengthListsTorsion];
	gint row=0;
	gint k;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        store = GTK_LIST_STORE (model);
	gtk_list_store_clear(store);

	
	for(i=0;i<amberParameters->numberOfDihedralTerms;i++)
	{
		n = amberParameters->dihedralAngleTerms[i].numbers[0];
		type = getType(amberParameters,n);
		texts[0] = g_strdup(type);

		n = amberParameters->dihedralAngleTerms[i].numbers[1];
		type = getType(amberParameters,n);
		texts[1] = g_strdup(type);

		n = amberParameters->dihedralAngleTerms[i].numbers[2];
		type = getType(amberParameters,n);
		texts[2] = g_strdup(type);

		n = amberParameters->dihedralAngleTerms[i].numbers[3];
		type = getType(amberParameters,n);
		texts[3] = g_strdup(type);

		for(j=0;j<amberParameters->dihedralAngleTerms[i].nSomme;j++)
		{
			texts[4] = g_strdup_printf("%f",amberParameters->dihedralAngleTerms[i].divisor[j]);
			texts[5] = g_strdup_printf("%f",amberParameters->dihedralAngleTerms[i].barrier[j]);
			texts[6] = g_strdup_printf("%f",amberParameters->dihedralAngleTerms[i].phase[j]);
			texts[7] = g_strdup_printf("%f",amberParameters->dihedralAngleTerms[i].n[j]);
			gtk_list_store_append(store, &iter);
			for(k=0;k<lengthListsTorsion;k++)
			{
				gtk_list_store_set (store, &iter, k, texts[k], -1);
			}
			row++;
		}
	}
	if(nn>=0) selectRow(list, nn);
}
/**********************************************************************/
static void rafreshListBends(gint nn)
{
	gchar *texts[lengthListsBend];
	gint i;
	gint n = 0;
	gchar* type;
	GtkWidget* list = listBends;
	gint k;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        store = GTK_LIST_STORE (model);
	gtk_list_store_clear(store);
	
	for(i=0;i<amberParameters->numberOfBendTerms;i++)
	{
		n = amberParameters->angleBendTerms[i].numbers[0];
		type = getType(amberParameters,n);
		texts[0] = g_strdup(type);

		n = amberParameters->angleBendTerms[i].numbers[1];
		type = getType(amberParameters,n);
		texts[1] = g_strdup(type);

		n = amberParameters->angleBendTerms[i].numbers[2];
		type = getType(amberParameters,n);
		texts[2] = g_strdup(type);

		texts[3] = g_strdup_printf("%f",amberParameters->angleBendTerms[i].forceConstant);
		texts[4] = g_strdup_printf("%f",amberParameters->angleBendTerms[i].equilibriumAngle);
		gtk_list_store_append(store, &iter);
		for(k=0;k<lengthListsBend;k++)
		{
			gtk_list_store_set (store, &iter, k, texts[k], -1);
		}
	}
	if(nn>=0) selectRow(list, nn);
}
/********************************************************************************/
static void rafreshListBonds(gint nn)
{
	gchar *texts[lengthListsBond];
	gint i;
	gint n = 0;
	gchar* type;
	GtkWidget* list = listBonds;
	gint k;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        store = GTK_LIST_STORE (model);
	gtk_list_store_clear(store);
	
	for(i=0;i<amberParameters->numberOfStretchTerms;i++)
	{
		n = amberParameters->bondStretchTerms[i].numbers[0];
		type = getType(amberParameters,n);
		texts[0] = g_strdup(type);

		n = amberParameters->bondStretchTerms[i].numbers[1];
		type = getType(amberParameters,n);
		texts[1] = g_strdup(type);

		texts[2] = g_strdup_printf("%f",amberParameters->bondStretchTerms[i].forceConstant);
		texts[3] = g_strdup_printf("%f",amberParameters->bondStretchTerms[i].equilibriumDistance);
		gtk_list_store_append(store, &iter);
		for(k=0;k<lengthListsBond;k++)
		{
			gtk_list_store_set (store, &iter, k, texts[k], -1);
		}
	}
	if(nn>=0) selectRow(list, nn);
}
/********************************************************************************/
static void rafreshListTypes(gint nn)
{
	gint i;
	gchar *texts[lengthListsType];
	GtkWidget* list = listTypes;
	gint k;
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        store = GTK_LIST_STORE (model);
	gtk_list_store_clear(store);

	for(i=0;i<amberParameters->numberOfTypes;i++)
	{
		texts[0] = g_strdup(amberParameters->atomTypes[i].name);
		texts[1] = g_strdup(amberParameters->atomTypes[i].symbol);
		texts[2] = g_strdup_printf("%f",amberParameters->atomTypes[i].masse);
		texts[3] = g_strdup_printf("%f",amberParameters->atomTypes[i].polarisability);
		texts[4] = g_strdup(amberParameters->atomTypes[i].description);
		gtk_list_store_append(store, &iter);
		for(k=0;k<lengthListsType;k++)
		{
			gtk_list_store_set (store, &iter, k, texts[k], -1);
		}
	}
	if(nn>=0) selectRow(list, nn);
}
/********************************************************************************/
static void editnewPairWiseDlg(GabeditSignalFunc f,gchar* title,gboolean sensitive)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *table;
	GtkWidget *combo;
	GtkWidget *label;
	gint i;
	gchar **tlist;
	GList *list=NULL;
	gint Nc;
	gint nlist;
	gchar* t1;
	gchar* t2;
	gint l = (gint)(ScreenHeight*0.1);

	Nc= atoi(rowSelectedLists[MMNUMBERPAIRWISE]);
	if(Nc>=0)
	{
	t1 = getType(amberParameters,amberParameters->pairWiseTerms[Nc].numbers[0]);
	t2 = getType(amberParameters,amberParameters->pairWiseTerms[Nc].numbers[1]);
	}
	else
	{
	t1 = amberParameters->atomTypes[0].name;
	t2 = amberParameters->atomTypes[0].name;
	}


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

	tlist = getLocalListTypes(&nlist);
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	table = gtk_table_new(7,4,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	label = gtk_label_new("Types");
	gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,3,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	Entries[E_T1] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T1]),t1);
	gtk_widget_set_sensitive(combo, sensitive); 

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,3,4,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	Entries[E_T2] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T2]),t2);
	gtk_widget_set_sensitive(combo, sensitive); 

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->pairWiseTerms[Nc].a);
	else
	tlist[0]=g_strdup("1.0");
	
	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(pairWiseTitles[2]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,1,2,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,1,2,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,4,1,2,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	Entries[E_A] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->pairWiseTerms[Nc].beta);
	else
	tlist[0]=g_strdup("1.0");
	
	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(pairWiseTitles[3]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,2,3,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,2,3,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,4,2,3,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	Entries[E_BETA] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->pairWiseTerms[Nc].c6);
	else
	tlist[0]=g_strdup("0.0");
	
	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(pairWiseTitles[4]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,3,4,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,3,4,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,4,3,4,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	Entries[E_C6] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->pairWiseTerms[Nc].c8);
	else
	tlist[0]=g_strdup("0.0");
	
	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(pairWiseTitles[5]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,4,5,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,4,5,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,4,4,5,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	Entries[E_C8] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->pairWiseTerms[Nc].c10);
	else
	tlist[0]=g_strdup("0.0");
	
	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(pairWiseTitles[6]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,5,6,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,5,6,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,4,5,6,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	Entries[E_C10] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->pairWiseTerms[Nc].b);
	else
	tlist[0]=g_strdup("0.0");
	
	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(pairWiseTitles[7]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,6,7,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,6,7,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,4,6,7,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	Entries[E_B] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);


	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,"Cancel");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);


	Button = create_button(WinDlg,"OK");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)f,NULL);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
	  
	gtk_widget_show_all(WinDlg);
}
/********************************************************************************/
static void editnewNonBondedDlg(GabeditSignalFunc f,gchar* title,gboolean sensitive)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *table;
	GtkWidget *combo;
	GtkWidget *label;
	gint i;
	gchar **tlist;
	GList *list=NULL;
	gint Nc;
	gint nlist;
	gchar* t1;
	gint l = (gint)(ScreenHeight*0.1);

	Nc = atoi(rowSelectedLists[MMNUMBERNONBONDED]);
	if(Nc>=0)
	t1 = getType(amberParameters,amberParameters->nonBondedTerms[Nc].number);
	else
	t1 = amberParameters->atomTypes[0].name;

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

	tlist = getLocalListTypes(&nlist);
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	table = gtk_table_new(3,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	label = gtk_label_new("Type");
	gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,3,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T1] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T1]),t1);
	gtk_widget_set_sensitive(combo, sensitive); 

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->nonBondedTerms[Nc].r);
	else
	tlist[0]=g_strdup("1.0");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(nonBondedTitles[1]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,3,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_R] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->nonBondedTerms[Nc].epsilon);
	else
	tlist[0]=g_strdup("1.0");
	
	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(nonBondedTitles[2]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,3,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_E] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);


	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,"Cancel");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);


	Button = create_button(WinDlg,"OK");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)f,NULL);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
	  
	gtk_widget_show_all(WinDlg);
}
/********************************************************************************/
static void editPairWise(GtkWidget *win)
{
	gdouble a	   = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_A])));
	gdouble beta	= atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_BETA])));
	gdouble c6	  = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_C6])));
	gdouble c8	  = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_C8])));
	gdouble c10	 = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_C10])));
	gdouble b	   = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_B])));
	gint i = atoi(rowSelectedLists[MMNUMBERPAIRWISE]);

	if(i<0)
		return;

	
	amberParameters->pairWiseTerms[i].a	  = a;
	amberParameters->pairWiseTerms[i].beta = beta;
	amberParameters->pairWiseTerms[i].c6	 = c6;
	amberParameters->pairWiseTerms[i].c8	 = c8;
	amberParameters->pairWiseTerms[i].c10	= c10;
	amberParameters->pairWiseTerms[i].b	  = b;
	rafreshListPairWises(i);

	
}
/********************************************************************************/
static void editPairWiseDlg(GtkWidget *win,gpointer data)
{
	editnewPairWiseDlg(editPairWise,"Pair Wise parameters Editor",FALSE);
}
/********************************************************************************/
static void editNonBonded(GtkWidget *win)
{
	gdouble r	      = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_R])));
	gdouble epsilon	= atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_E])));
	gint i = atoi(rowSelectedLists[MMNUMBERNONBONDED]);

	if(i<0)
		return;

	
	amberParameters->nonBondedTerms[i].r = r;
	amberParameters->nonBondedTerms[i].epsilon = epsilon;
	rafreshListNonBondeds(i);

	
}
/********************************************************************************/
static void editNonBondedDlg(GtkWidget *win,gpointer data)
{
	editnewNonBondedDlg(editNonBonded,"Non Bonded Editor",FALSE);
}
/********************************************************************************/
static void editnewHBondDlg(GabeditSignalFunc f,gchar* title,gboolean sensitive)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *table;
	GtkWidget *combo;
	GtkWidget *label;
	gint i;
	gchar **tlist;
	GList *list=NULL;
	gint Nc;
	gint nlist;
	gchar* t1;
	gchar* t2;
	gint l = (gint)(ScreenHeight*0.1);

	Nc= atoi(rowSelectedLists[MMNUMBERHBOND]);
	if(Nc>=0)
	{
	t1 = getType(amberParameters,amberParameters->hydrogenBondedTerms[Nc].numbers[0]);
	t2 = getType(amberParameters,amberParameters->hydrogenBondedTerms[Nc].numbers[1]);
	}
	else
	{
	t1 = amberParameters->atomTypes[0].name;
	t2 = amberParameters->atomTypes[0].name;
	}


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

	tlist = getLocalListTypes(&nlist);
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	table = gtk_table_new(3,4,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	label = gtk_label_new("Types");
	gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,3,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T1] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T1]),t1);
	gtk_widget_set_sensitive(combo, sensitive); 

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,3,4,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T2] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T2]),t2);
	gtk_widget_set_sensitive(combo, sensitive); 

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->hydrogenBondedTerms[Nc].c);
	else
	tlist[0]=g_strdup("1.0");
	
	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(hBondTitles[2]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,4,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_C] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->hydrogenBondedTerms[Nc].d);
	else
	tlist[0]=g_strdup("1.0");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(hBondTitles[3]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,4,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_D] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);


	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,"Cancel");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);



	Button = create_button(WinDlg,"OK");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)f,NULL);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
	  
	gtk_widget_show_all(WinDlg);
}
/********************************************************************************/
static void editHBond(GtkWidget *win)
{
	gdouble c = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_C])));
	gdouble d = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_D])));
	gint i = atoi(rowSelectedLists[MMNUMBERHBOND]);

	if(i<0)
		return;

	
	amberParameters->hydrogenBondedTerms[i].c = c;
	amberParameters->hydrogenBondedTerms[i].d = d;
	rafreshListHBonds(i);


}
/********************************************************************************/
static void editHBondDlg(GtkWidget *win,gpointer data)
{
	editnewHBondDlg(editHBond,"Edit H-Bond parameters",FALSE);
}
/********************************************************************************/
static void editnewImproperDlg(GabeditSignalFunc f,gchar* title,gboolean sensitive)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *table;
	GtkWidget *combo;
	GtkWidget *label;
	gint i;
	gchar **tlist;
	GList *list=NULL;
	gint Nc;
	gint nlist;
	gchar* t1;
	gchar* t2;
	gchar* t3;
	gchar* t4;
	gint l = (gint)(ScreenHeight*0.1);

	Nc= atoi(rowSelectedLists[MMNUMBERIMPROPER]);
	if(Nc>=0)
	{
		t1 = getType(amberParameters,amberParameters->improperTorsionTerms[Nc].numbers[0]);
		t2 = getType(amberParameters,amberParameters->improperTorsionTerms[Nc].numbers[1]);
		t3 = getType(amberParameters,amberParameters->improperTorsionTerms[Nc].numbers[2]);
		t4 = getType(amberParameters,amberParameters->improperTorsionTerms[Nc].numbers[3]);
	}
	else
	{
		t1 = amberParameters->atomTypes[0].name;
		t2 = amberParameters->atomTypes[0].name;
		t3 = amberParameters->atomTypes[0].name;
		t4 = amberParameters->atomTypes[0].name;
	}

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

	tlist = getLocalListTypes(&nlist);
	list=g_list_append(list,"X");
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	table = gtk_table_new(4,6,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	label = gtk_label_new("Types");
	gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,3,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T1] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T1]),t1);
	gtk_widget_set_sensitive(combo, sensitive); 

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,3,4,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T2] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T2]),t2);
	gtk_widget_set_sensitive(combo, sensitive); 

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,4,5,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T3] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T3]),t3);
	gtk_widget_set_sensitive(combo, sensitive); 

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,5,6,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T4] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T4]),t4);
	gtk_widget_set_sensitive(combo, sensitive); 


	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->improperTorsionTerms[Nc].barrier);
	else
	tlist[0]=g_strdup("1.0");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(improperTitles[4]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*4,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,6,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_B] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->improperTorsionTerms[Nc].phase);
	else
	tlist[0]=g_strdup("1.0");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(improperTitles[5]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*4,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,6,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_P] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->improperTorsionTerms[Nc].n);
	else
	tlist[0]=g_strdup("1.0");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(improperTitles[6]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,3,4,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,3,4,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*4,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,6,3,4,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_N] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);


	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,"Cancel");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);


	Button = create_button(WinDlg,"OK");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)f,NULL);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
	  
	gtk_widget_show_all(WinDlg);
}
/********************************************************************************/
static void editImproper(GtkWidget *win)
{
	gdouble barrier = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_B])));
	gdouble phase	 = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_P])));
	gdouble n	     = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_N])));
	gint i = atoi(rowSelectedLists[MMNUMBERIMPROPER]);

	if(i<0)
		return;

	
	amberParameters->improperTorsionTerms[i].barrier = barrier;
	amberParameters->improperTorsionTerms[i].phase	 = phase;
	amberParameters->improperTorsionTerms[i].n= n;
	rafreshListImpropers(i);

}
/********************************************************************************/
static void editImproperDlg(GtkWidget *win,gpointer data)
{
	editnewImproperDlg(editImproper,"Improper terms Editor",FALSE);
}
/********************************************************************************/
static void getIndexTorsion(gint Nc, gint* ii, gint* jj)
{
	gint i;
	gint j;
	gint row = 0;
	for(i=0;i<amberParameters->numberOfDihedralTerms;i++)
	{
		for(j=0;j<amberParameters->dihedralAngleTerms[i].nSomme;j++)
		{
			if(row == Nc)
			{
				*ii = i;
				*jj = j;
				return;
			}
			row++;
		}
	}
	*ii = -1;
	*jj = -1;

}
/********************************************************************************/
static void editnewTorsionDlg(GabeditSignalFunc f,gchar* title,gboolean sensitive)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *table;
	GtkWidget *combo;
	GtkWidget *label;
	gint i;
	gint j;
	gchar **tlist;
	GList *list=NULL;
	gint Nc;
	gint nlist;
	gchar* t1;
	gchar* t2;
	gchar* t3;
	gchar* t4;
	gint l = (gint)(ScreenHeight*0.1);

	i = atoi(rowSelectedLists[MMNUMBERTORSION]);
	getIndexTorsion(i,&Nc,&j);
	if(Nc>=0)
	{
		t1 = getType(amberParameters,amberParameters->dihedralAngleTerms[Nc].numbers[0]);
		t2 = getType(amberParameters,amberParameters->dihedralAngleTerms[Nc].numbers[1]);
		t3 = getType(amberParameters,amberParameters->dihedralAngleTerms[Nc].numbers[2]);
		t4 = getType(amberParameters,amberParameters->dihedralAngleTerms[Nc].numbers[3]);
	}
	else
	{
		  t1 = amberParameters->atomTypes[0].name;
		t2 = amberParameters->atomTypes[0].name;
		t3 = amberParameters->atomTypes[0].name;
		t4 = amberParameters->atomTypes[0].name;
	}


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

	tlist = getLocalListTypes(&nlist);
	list=g_list_append(list,"X");
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	table = gtk_table_new(5,6,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	label = gtk_label_new("Types");
	gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,3,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T1] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T1]),t1);
	gtk_widget_set_sensitive(combo, sensitive); 

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,3,4,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T2] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T2]),t2);
	gtk_widget_set_sensitive(combo, sensitive); 

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,4,5,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T3] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T3]),t3);
	gtk_widget_set_sensitive(combo, sensitive); 

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,5,6,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T4] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T4]),t4);
	gtk_widget_set_sensitive(combo, sensitive); 

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->dihedralAngleTerms[Nc].divisor[j]);
	else
	tlist[0]=g_strdup(" ");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(torsionTitles[4]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*4,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,6,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_D] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->dihedralAngleTerms[Nc].barrier[j]);
	else
	tlist[0]=g_strdup("1.0");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(torsionTitles[5]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*4,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,6,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_B] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->dihedralAngleTerms[Nc].phase[j]);
	else
	tlist[0]=g_strdup("1.0");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(torsionTitles[6]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,3,4,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,3,4,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*4,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,6,3,4,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_P] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->dihedralAngleTerms[Nc].n[j]);
	else
	tlist[0]=g_strdup("1.0");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(torsionTitles[7]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,4,5,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,4,5,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*4,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,6,4,5,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_N] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);


	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,"Cancel");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);


	Button = create_button(WinDlg,"OK");
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)f,NULL);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
	  
	gtk_widget_show_all(WinDlg);
}
/********************************************************************************/
static void editTorsion(GtkWidget *win)
{
	gdouble divisor = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_D])));
	gdouble barrier = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_B])));
	gdouble phase	 = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_P])));
	gdouble n	     = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_N])));
	gint Nc = atoi(rowSelectedLists[MMNUMBERTORSION]);
	gint i;
	gint j;

	if(Nc<0)
		return;

	getIndexTorsion(Nc,&i,&j);

	if(i<0)
		return;

	if(fabs(divisor)<1e-10)
		divisor = 1e-10;

	amberParameters->dihedralAngleTerms[i].divisor[j] = divisor;
	amberParameters->dihedralAngleTerms[i].barrier[j] = barrier;
	amberParameters->dihedralAngleTerms[i].phase[j]	 = phase;
	amberParameters->dihedralAngleTerms[i].n[j]	     = n;
	rafreshListTorsions(Nc);
}
/********************************************************************************/
static void editTorsionDlg(GtkWidget *win,gpointer data)
{
	editnewTorsionDlg(editTorsion,"Torsion editor",FALSE);
}
/********************************************************************************/
static void editnewBendDlg(GabeditSignalFunc f,gchar* title,gboolean sensitive)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *table;
	GtkWidget *combo;
	GtkWidget *label;
	gint i;
	gchar **tlist;
	GList *list=NULL;
	gint Nc;
	gint nlist;
	gchar* t1;
	gchar* t2;
	gchar* t3;
	gint l = (gint)(ScreenHeight*0.1);

	Nc= atoi(rowSelectedLists[MMNUMBERBEND]);
	if(Nc>=0)
	{
		t1 = getType(amberParameters,amberParameters->angleBendTerms[Nc].numbers[0]);
		t2 = getType(amberParameters,amberParameters->angleBendTerms[Nc].numbers[1]);
		t3 = getType(amberParameters,amberParameters->angleBendTerms[Nc].numbers[2]);
	}
	else
	{
		  t1 = amberParameters->atomTypes[0].name;
		t2 = amberParameters->atomTypes[0].name;
		t3 = amberParameters->atomTypes[0].name;
	}

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

	tlist = getLocalListTypes(&nlist);
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	table = gtk_table_new(3,5,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	label = gtk_label_new("Types");
	gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,3,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T1] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T1]),t1);
	gtk_widget_set_sensitive(combo, sensitive); 

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,3,4,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T2] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T2]),t2);
	gtk_widget_set_sensitive(combo, sensitive); 

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,4,5,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T3] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T3]),t3);
	gtk_widget_set_sensitive(combo, sensitive); 

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->angleBendTerms[Nc].forceConstant);
	else
	tlist[0]=g_strdup(" ");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(bondTitles[2]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,5,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_F] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->angleBendTerms[Nc].equilibriumAngle);
	else
	tlist[0]=g_strdup(" ");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(bondTitles[3]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,5,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_A] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);


	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,"Cancel");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);


	Button = create_button(WinDlg,"OK");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)f,NULL);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
	  
	gtk_widget_show_all(WinDlg);
}
/********************************************************************************/
static void editBend(GtkWidget *win)
{

	gdouble force = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_F])));
	gdouble angle = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_A])));
	gint i = atoi(rowSelectedLists[MMNUMBERBEND]);

	if(i<0)
		return;

	
	amberParameters->angleBendTerms[i].forceConstant = force;
	amberParameters->angleBendTerms[i].equilibriumAngle= angle;
	rafreshListBends(i);

}
/********************************************************************************/
static void editBendDlg(GtkWidget *win,gpointer data)
{
	editnewBendDlg(editBend,"Bend Editor",FALSE);

}
/********************************************************************************/
static void editnewBondDlg(GabeditSignalFunc f,gchar* title,gboolean sensitive)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *table;
	GtkWidget *combo;
	GtkWidget *label;
	gint i;
	gchar **tlist;
	GList *list=NULL;
	gint Nc;
	gint nlist;
	gchar* t1;
	gchar* t2;
	gint l = (gint)(ScreenHeight*0.1);

	Nc= atoi(rowSelectedLists[MMNUMBERBOND]);
	if(Nc>=0)
	{
		t1 = getType(amberParameters,amberParameters->bondStretchTerms[Nc].numbers[0]);
		t2 = getType(amberParameters,amberParameters->bondStretchTerms[Nc].numbers[1]);
	}
	else
	{
		  t1 = amberParameters->atomTypes[0].name;
		t2 = amberParameters->atomTypes[0].name;
	}


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

	tlist = getLocalListTypes(&nlist);
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	table = gtk_table_new(3,4,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

	label = gtk_label_new("Types");
	gtk_table_attach(GTK_TABLE(table),label,0,1,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,3,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T1] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T1]),t1);
	gtk_widget_set_sensitive(combo, sensitive); 

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l,-1);
	gtk_table_attach(GTK_TABLE(table),combo,3,4,0,1,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_T2] = GTK_BIN(combo)->child;
	gtk_entry_set_text(GTK_ENTRY(Entries[E_T2]),t2);
	gtk_widget_set_sensitive(combo, sensitive); 

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->bondStretchTerms[Nc].forceConstant);
	else
	tlist[0]=g_strdup("1.0");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(bondTitles[2]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,4,1,2,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_F] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);
	list = NULL;

	tlist=g_malloc(2*sizeof(char*));
	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->bondStretchTerms[Nc].equilibriumDistance);
	else
	tlist[0]=g_strdup("1.0");

	tlist[1]=g_strdup(" ");
	nlist=2;
	for (i=0;i<nlist;i++)
		list=g_list_append(list,tlist[i]);

	label = gtk_label_new(bondTitles[3]);
	gtk_table_attach(GTK_TABLE(table),label,0,1,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label,1,2,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);

	combo = gtk_combo_box_entry_new_text();
	gtk_combo_box_entry_set_popdown_strings(combo,list);
	gtk_widget_set_size_request(GTK_WIDGET(combo),l*2,-1);
	gtk_table_attach(GTK_TABLE(table),combo,2,4,2,3,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),1,1);
	Entries[E_R] = GTK_BIN(combo)->child;

	tlist = freeList(tlist,nlist);


	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,"Cancel");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);

	Button = create_button(WinDlg,"OK");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)f,NULL);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
	  

	gtk_widget_show_all(WinDlg);
}
/********************************************************************************/
static void editBond(GtkWidget* w)
{
	gdouble force = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_F])));
	gdouble r	   = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_R])));
	gint i = atoi(rowSelectedLists[MMNUMBERBOND]);

	if(i<0)
		return;

	
	amberParameters->bondStretchTerms[i].forceConstant = force;
	amberParameters->bondStretchTerms[i].equilibriumDistance= r;
	rafreshListBonds(i);
}
/********************************************************************************/
static void editBondDlg(GtkWidget *win,gpointer data)
{
	editnewBondDlg(editBond,"Edit Bond parameters",FALSE);
}
/********************************************************************************/
static void editType(GtkWidget* w)
{
	G_CONST_RETURN gchar *symbol = gtk_entry_get_text(GTK_ENTRY(Entries[E_S]));
	gdouble masse = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_M])));
	gdouble pol	 = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_P])));
	G_CONST_RETURN gchar* desc = (gtk_entry_get_text(GTK_ENTRY(Entries[E_DESC])));
	gint i = atoi(rowSelectedLists[MMNUMBERTYPE]);

	if(i<0)
		return;
	g_free(amberParameters->atomTypes[i].symbol);
	amberParameters->atomTypes[i].symbol = g_strdup(symbol);
	amberParameters->atomTypes[i].masse = masse;
	amberParameters->atomTypes[i].polarisability = pol;
	amberParameters->atomTypes[i].description = g_strdup(desc);
	rafreshListTypes(i);
}
/********************************************************************************/
static void newType(GtkWidget* w)
{
	G_CONST_RETURN gchar *symbol = gtk_entry_get_text(GTK_ENTRY(Entries[E_S]));
	G_CONST_RETURN gchar *name = gtk_entry_get_text(GTK_ENTRY(Entries[E_T]));
	gdouble masse = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_M])));
	gdouble pol	 = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_P])));
	G_CONST_RETURN gchar* desc = (gtk_entry_get_text(GTK_ENTRY(Entries[E_DESC])));
	gint i = atoi(rowSelectedLists[MMNUMBERTYPE]);
	gint j;
	gint number = 0;
	  
	for(j=0;j<amberParameters->numberOfTypes;j++)
	{
		if(number<amberParameters->atomTypes[j].number)
			number = amberParameters->atomTypes[j].number;
		if(!strcmp(amberParameters->atomTypes[j].name,name))
		{
			gchar* t = g_strdup_printf("The \"%s\" type is define !",name);
			GtkWidget* w = Message(t,"Error",TRUE);
			gtk_window_set_modal(GTK_WINDOW(w),TRUE);
				gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
			g_free(t);
			return;
		}
	}
	amberParameters->numberOfTypes ++;
	if(amberParameters->numberOfTypes>1)
		amberParameters->atomTypes = g_realloc(
		amberParameters->atomTypes,
		amberParameters->numberOfTypes*sizeof(AmberAtomTypes));
	else
		amberParameters->atomTypes = g_malloc(sizeof(AmberAtomTypes));

	i = amberParameters->numberOfTypes -1;
	amberParameters->atomTypes[i].name = g_strdup(name);
	amberParameters->atomTypes[i].symbol = g_strdup(symbol);
	amberParameters->atomTypes[i].number = number+1;
	amberParameters->atomTypes[i].masse = masse;
	amberParameters->atomTypes[i].polarisability = pol;
	amberParameters->atomTypes[i].description = g_strdup(desc);
	rafreshListTypes(i);
}
/********************************************************************************/
static void editnewTypeDlg(GabeditSignalFunc f,gchar* title)
{
	GtkWidget *WinDlg;
	GtkWidget *Button;
	GtkWidget *hbox;
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gint i;
	gchar **tlist;
	gint Nc;
	gint nlist;

	Nc= atoi(rowSelectedLists[MMNUMBERTYPE]);

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

	Entries[E_S] = create_label_entry(hbox,typeTitles[1],(gint)(ScreenHeight*0.1),(gint)(ScreenHeight*0.15));
	if(Nc>=0) gtk_entry_set_text(GTK_ENTRY(Entries[E_S]),amberParameters->atomTypes[Nc].symbol);
	else gtk_entry_set_text(GTK_ENTRY(Entries[E_S]),"C");
	gtk_editable_set_editable((GtkEditable*) Entries[E_S],FALSE);

	Button = gtk_button_new_with_label(" Modify ");
	gtk_box_pack_start (GTK_BOX(hbox), Button, TRUE, TRUE, 5);
	g_signal_connect(G_OBJECT(Button), "clicked", (GCallback)SelectAtom,Entries[E_S]);

	hbox=create_hbox_false(vboxframe);
	Entries[E_T] = create_label_entry(hbox,typeTitles[0],(gint)(ScreenHeight*0.1),(gint)(ScreenHeight*0.15));
	if(Nc>=0) gtk_entry_set_text(GTK_ENTRY(Entries[E_T]),amberParameters->atomTypes[Nc].name);
	else gtk_entry_set_text(GTK_ENTRY(Entries[E_T])," ");
	gtk_editable_set_editable((GtkEditable*) Entries[E_T],FALSE);

	tlist=g_malloc(2*sizeof(char*));

	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->atomTypes[Nc].masse);
	else
	tlist[0]=g_strdup("1.0");

	tlist[1]=g_strdup(" ");
	nlist=2;
	hbox=create_hbox_false(vboxframe);
	Entries[E_M] = create_label_combo(hbox,typeTitles[2],tlist,nlist,FALSE,
			(gint)(ScreenHeight*0.1),(gint)(ScreenHeight*0.15));
	gtk_editable_set_editable((GtkEditable*) Entries[E_M],TRUE);

	for(i=0;i<nlist;i++)
		g_free(tlist[i]);

	if(Nc>=0)
	tlist[0]=g_strdup_printf("%f",amberParameters->atomTypes[Nc].polarisability);
	else
	tlist[0]=g_strdup("1.0");

	tlist[1]=g_strdup(" ");
	nlist=2;
	hbox=create_hbox_false(vboxframe);
	Entries[E_P] = create_label_combo(hbox,typeTitles[3],tlist,nlist,FALSE,
			(gint)(ScreenHeight*0.1),(gint)(ScreenHeight*0.15));
	gtk_editable_set_editable((GtkEditable*) Entries[E_P],TRUE);

	for(i=0;i<nlist;i++)
		g_free(tlist[i]);

	hbox=create_hbox_false(vboxframe);
	Entries[E_DESC] = create_label_entry(hbox,typeTitles[4],(gint)(ScreenHeight*0.1),(gint)(ScreenHeight*0.15));
	if(Nc>=0) gtk_entry_set_text(GTK_ENTRY(Entries[E_DESC]),amberParameters->atomTypes[Nc].description);
	else gtk_entry_set_text(GTK_ENTRY(Entries[E_DESC])," ");
	gtk_editable_set_editable((GtkEditable*) Entries[E_DESC],TRUE);
	
	g_free(tlist);


	gtk_widget_realize(WinDlg);

	Button = create_button(WinDlg,"Cancel");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);

	Button = create_button(WinDlg,"OK");
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(WinDlg)->action_area), Button,TRUE,TRUE,0);
	g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)f, NULL);
	g_signal_connect_swapped(GTK_OBJECT(Button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(WinDlg));
	GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(Button);
	  

	gtk_widget_show_all(WinDlg);
}
/********************************************************************************/
static void editTypeDlg(GtkWidget *w,gpointer data)
{
	editnewTypeDlg(editType,"Edit a type");
	      gtk_editable_set_editable((GtkEditable*) Entries[E_T],FALSE);
}
/********************************************************************************/
static void editDlg(GtkWidget *win,gpointer data)
{
	gint nList = gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBook));
	switch(nList)
	{
		case MMNUMBERTYPE	    : 
			editTypeDlg(win,data);
			break;
		case MMNUMBERBOND	    :
			editBondDlg(win,data);
			break;
		case MMNUMBERBEND	    :
			editBendDlg(win,data);
			break;
		case MMNUMBERTORSION	 :
			editTorsionDlg(win,data);
			break;
		case MMNUMBERIMPROPER	:
			editImproperDlg(win,data);
			break;
		case MMNUMBERHBOND	   :
			editHBondDlg(win,data);
			break;
		case MMNUMBERNONBONDED :
			editNonBondedDlg(win,data);
			break;
		case MMNUMBERPAIRWISE :
			editPairWiseDlg(win,data);
			break;
	}
}
/********************************************************************************/
static void newPairWise(GtkWidget *win)
{
	G_CONST_RETURN	gchar* t1     = gtk_entry_get_text(GTK_ENTRY(Entries[E_T1]));
	G_CONST_RETURN	gchar* t2     = gtk_entry_get_text(GTK_ENTRY(Entries[E_T2]));
	gdouble a	   = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_A])));
	gdouble beta	= atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_BETA])));
	gdouble c6	  = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_C6])));
	gdouble c8	  = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_C8])));
	gdouble c10	 = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_C10])));
	gdouble b	   = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_B])));
	gint i;
	gint number1 = getNumberType(amberParameters,t1);
	gint number2 = getNumberType(amberParameters,t2);

	for(i=0;i<amberParameters->numberOfPairWise;i++)
	{
		if(
			(
			number1 == amberParameters->pairWiseTerms[i].numbers[0] &&
			number2 == amberParameters->pairWiseTerms[i].numbers[1] 
			) ||			
			(
			number1 == amberParameters->pairWiseTerms[i].numbers[1] &&
			number2 == amberParameters->pairWiseTerms[i].numbers[0] 
			) 
			)
		{
			gchar* t = g_strdup_printf("Sorry This pair wise term is define !");
			GtkWidget* w = Message(t,"Error",TRUE);
			gtk_window_set_modal(GTK_WINDOW(w),TRUE);
				gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
			g_free(t);
			return;
		}
	}

	amberParameters->numberOfPairWise++;
	if(amberParameters->numberOfPairWise>1)
		amberParameters->pairWiseTerms = g_realloc(
		amberParameters->pairWiseTerms,
		amberParameters->numberOfPairWise*sizeof(AmberPairWiseTerms));
	else
		amberParameters->pairWiseTerms = g_malloc(sizeof(AmberPairWiseTerms));

	i = amberParameters->numberOfPairWise -1;
	
	amberParameters->pairWiseTerms[i].numbers[0] = number1;
	amberParameters->pairWiseTerms[i].numbers[1] = number2;
	amberParameters->pairWiseTerms[i].a	        = a;
	amberParameters->pairWiseTerms[i].beta	     = beta;
	amberParameters->pairWiseTerms[i].c6	       = c6;
	amberParameters->pairWiseTerms[i].c8	       = c8;
	amberParameters->pairWiseTerms[i].c10	      = c10;
	amberParameters->pairWiseTerms[i].b	        = b;
	rafreshListPairWises(i);
}
/********************************************************************************/
static void newPairWiseDlg(GtkWidget *win,gpointer data)
{
	editnewPairWiseDlg(newPairWise,"New Pair Wise term ",TRUE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T1],FALSE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T2],FALSE);
}
/********************************************************************************/
static void newNonBonded(GtkWidget *win)
{
	G_CONST_RETURN gchar* t1	     = gtk_entry_get_text(GTK_ENTRY(Entries[E_T1]));
	gdouble r	     = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_R])));
	gdouble epsilon = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_E])));
	gint i = atoi(rowSelectedLists[MMNUMBERNONBONDED]);
	gint j;
	gint number = getNumberType(amberParameters,t1);
	  
	for(j=0;j<amberParameters->numberOfNonBonded;j++)
	{
		if(number == amberParameters->nonBondedTerms[j].number)
		{
			gchar* t = g_strdup_printf("The non bonded terms for \"%s\" is define !",t1);
			GtkWidget* w = Message(t,"Error",TRUE);
			gtk_window_set_modal(GTK_WINDOW(w),TRUE);
				gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
			g_free(t);
			return;
		}
	}
	amberParameters->numberOfNonBonded ++;
	if(amberParameters->numberOfNonBonded>1)
		amberParameters->nonBondedTerms = g_realloc(
		amberParameters->nonBondedTerms,
		amberParameters->numberOfNonBonded*sizeof(AmberNonBondedTerms));
	else
		amberParameters->nonBondedTerms = g_malloc(sizeof(AmberNonBondedTerms));

	i = amberParameters->numberOfNonBonded -1;
	amberParameters->nonBondedTerms[i].number	= number;
	amberParameters->nonBondedTerms[i].r	     = r;
	amberParameters->nonBondedTerms[i].epsilon = epsilon;
	rafreshListNonBondeds(i);
}
/********************************************************************************/
static void newNonBondedDlg(GtkWidget *win,gpointer data)
{
	editnewNonBondedDlg(newNonBonded,"New Non-Bonded term ",TRUE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T1],FALSE);
}
/********************************************************************************/
static void newHBond(GtkWidget *win)
{
	G_CONST_RETURN gchar *t1 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T1]));
	G_CONST_RETURN gchar *t2 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T2]));
	gdouble c = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_C])));
	gdouble d	 = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_D])));
	gint i = atoi(rowSelectedLists[MMNUMBERHBOND]);
	gint j;
	gint number1 = getNumberType(amberParameters,t1);
	gint number2 = getNumberType(amberParameters,t2);

	for(j=0;j<amberParameters->numberOfHydrogenBonded;j++)
	{
		if(
			(
			number1 == amberParameters->hydrogenBondedTerms[j].numbers[0] &&
			number2 == amberParameters->hydrogenBondedTerms[j].numbers[1] 
			) ||			
			(
			number1 == amberParameters->hydrogenBondedTerms[j].numbers[1] &&
			number2 == amberParameters->hydrogenBondedTerms[j].numbers[0] 
			) 
			)
		{
			gchar* t = g_strdup_printf("Sorry This Bond is define !");
			GtkWidget* w = Message(t,"Error",TRUE);
			gtk_window_set_modal(GTK_WINDOW(w),TRUE);
				gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
			g_free(t);
			return;
		}
	}

	amberParameters->numberOfHydrogenBonded++;
	if(amberParameters->numberOfHydrogenBonded>1)
		amberParameters->hydrogenBondedTerms = g_realloc(
		amberParameters->hydrogenBondedTerms,
		amberParameters->numberOfHydrogenBonded*sizeof(AmberHydrogenBondedTerms));
	else
		amberParameters->hydrogenBondedTerms = g_malloc(sizeof(AmberHydrogenBondedTerms));

	i = amberParameters->numberOfHydrogenBonded-1;
	amberParameters->hydrogenBondedTerms[i].numbers[0] = number1;
	amberParameters->hydrogenBondedTerms[i].numbers[1] = number2;
	amberParameters->hydrogenBondedTerms[i].c	        = c;
	amberParameters->hydrogenBondedTerms[i].d	        = d;
	rafreshListHBonds(i);
}
/********************************************************************************/
static void newHBondDlg(GtkWidget *win,gpointer data)
{
	editnewHBondDlg(newHBond,"New H-Bond",TRUE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T1],FALSE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T2],FALSE);
}
/********************************************************************************/
static void newImproper(GtkWidget *win)
{
	G_CONST_RETURN gchar *t1 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T1]));
	G_CONST_RETURN gchar *t2 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T2]));
	G_CONST_RETURN gchar *t3 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T3]));
	G_CONST_RETURN gchar *t4 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T4]));
	gdouble barrier = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_B])));
	gdouble phase	 = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_P])));
	gdouble n	    = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_N])));
	gint i = atoi(rowSelectedLists[MMNUMBERIMPROPER]);
	gint j;
	gint number1 = getNumberType(amberParameters,t1);
	gint number2 = getNumberType(amberParameters,t2);
	gint number3 = getNumberType(amberParameters,t3);
	gint number4 = getNumberType(amberParameters,t4);

	for(j=0;j<amberParameters->numberOfImproperTorsionTerms;j++)
	{
		if(
			(
			number1 == amberParameters->improperTorsionTerms[j].numbers[0] &&
			number2 == amberParameters->improperTorsionTerms[j].numbers[1] &&
			number3 == amberParameters->improperTorsionTerms[j].numbers[2] &&
			number4 == amberParameters->improperTorsionTerms[j].numbers[3] 
			) ||			
			(
			number1 == amberParameters->improperTorsionTerms[j].numbers[3] &&
			number2 == amberParameters->improperTorsionTerms[j].numbers[2] &&
			number3 == amberParameters->improperTorsionTerms[j].numbers[1] &&
			number4 == amberParameters->improperTorsionTerms[j].numbers[0] 
			) 
			)
		{
			gchar* t = g_strdup_printf("Sorry This Improper terms is define !");
			GtkWidget* w = Message(t,"Error",TRUE);
			gtk_window_set_modal(GTK_WINDOW(w),TRUE);
				gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
			g_free(t);
			return;
		}
	}

	amberParameters->numberOfImproperTorsionTerms++;
	if(amberParameters->numberOfImproperTorsionTerms>1)
		amberParameters->improperTorsionTerms = g_realloc(
		amberParameters->improperTorsionTerms,
		amberParameters->numberOfImproperTorsionTerms*sizeof(AmberImproperTorsionTerms));
	else
		amberParameters->improperTorsionTerms = g_malloc(sizeof(AmberImproperTorsionTerms));

	i = amberParameters->numberOfImproperTorsionTerms-1;
	amberParameters->improperTorsionTerms[i].numbers[0] = number1;
	amberParameters->improperTorsionTerms[i].numbers[1] = number2;
	amberParameters->improperTorsionTerms[i].numbers[2] = number3;
	amberParameters->improperTorsionTerms[i].numbers[3] = number4;
	amberParameters->improperTorsionTerms[i].barrier = barrier;
	amberParameters->improperTorsionTerms[i].phase	 = phase;
	amberParameters->improperTorsionTerms[i].n	     = n;
	rafreshListImpropers(i);

}
/********************************************************************************/
static void newImproperDlg(GtkWidget *win,gpointer data)
{
	editnewImproperDlg(newImproper,"new Improper terms",TRUE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T1],FALSE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T2],FALSE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T3],FALSE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T4],FALSE);
}
/********************************************************************************/
static void newTorsion(GtkWidget *win)
{
	G_CONST_RETURN gchar *t1 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T1]));
	G_CONST_RETURN gchar *t2 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T2]));
	G_CONST_RETURN gchar *t3 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T3]));
	G_CONST_RETURN gchar *t4 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T4]));
	gdouble divisor = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_D])));
	gdouble barrier = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_B])));
	gdouble phase	 = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_P])));
	gdouble n = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_N])));
	gint i;
	gint j;
	gint ii;
	gint jj;
	gint Nc = atoi(rowSelectedLists[MMNUMBERTORSION]);

	gint number1 = getNumberType(amberParameters,t1);
	gint number2 = getNumberType(amberParameters,t2);
	gint number3 = getNumberType(amberParameters,t3);
	gint number4 = getNumberType(amberParameters,t4);

	if(fabs(divisor)<1e-10)
		divisor = 1e-10;

	ii = -1;
	jj = -1;

	for(i=0;i<amberParameters->numberOfDihedralTerms;i++)
	{
		if(
			(
			number1 == amberParameters->dihedralAngleTerms[i].numbers[0] &&
			number2 == amberParameters->dihedralAngleTerms[i].numbers[1] &&
			number3 == amberParameters->dihedralAngleTerms[i].numbers[2] &&
			number4 == amberParameters->dihedralAngleTerms[i].numbers[3] 
			) ||			
			(
			number1 == amberParameters->dihedralAngleTerms[i].numbers[3] &&
			number2 == amberParameters->dihedralAngleTerms[i].numbers[2] &&
			number3 == amberParameters->dihedralAngleTerms[i].numbers[1] &&
			number4 == amberParameters->dihedralAngleTerms[i].numbers[0] 
			) 
			)
		{
			ii = i;
			break;
		}
	}
	if(ii == -1)
	{
		amberParameters->numberOfDihedralTerms++;

		if(amberParameters->numberOfDihedralTerms>1)
			amberParameters->dihedralAngleTerms = g_realloc(
			amberParameters->dihedralAngleTerms,
			amberParameters->numberOfDihedralTerms*sizeof(AmberDihedralAngleTerms));
		else
			amberParameters->dihedralAngleTerms = g_malloc(sizeof(AmberDihedralAngleTerms));

		i = amberParameters->numberOfDihedralTerms-1;
		amberParameters->dihedralAngleTerms[i].numbers[0] = number1;
		amberParameters->dihedralAngleTerms[i].numbers[1] = number2;
		amberParameters->dihedralAngleTerms[i].numbers[2] = number3;
		amberParameters->dihedralAngleTerms[i].numbers[3] = number4;

		amberParameters->dihedralAngleTerms[i].nSomme = 1;
		amberParameters->dihedralAngleTerms[i].divisor	  = g_malloc(sizeof(gdouble));
		amberParameters->dihedralAngleTerms[i].divisor[0] = divisor;
		amberParameters->dihedralAngleTerms[i].barrier	  = g_malloc(sizeof(gdouble));
		amberParameters->dihedralAngleTerms[i].barrier[0] = barrier;
		amberParameters->dihedralAngleTerms[i].phase	    = g_malloc(sizeof(gdouble));
		amberParameters->dihedralAngleTerms[i].phase[0]	 = phase;
		amberParameters->dihedralAngleTerms[i].n	        = g_malloc(sizeof(gdouble));
		amberParameters->dihedralAngleTerms[i].n[0]	     = n;
	}
	else
	{
		gint len;
		i = ii;
		amberParameters->dihedralAngleTerms[i].nSomme++;
		len = amberParameters->dihedralAngleTerms[i].nSomme;
		j= len -1;

		amberParameters->dihedralAngleTerms[i].divisor	  = 
			g_realloc(amberParameters->dihedralAngleTerms[i].divisor, len*sizeof(gdouble));
		amberParameters->dihedralAngleTerms[i].divisor[j] = divisor;
		amberParameters->dihedralAngleTerms[i].barrier	  = 
			g_realloc(amberParameters->dihedralAngleTerms[i].barrier, len*sizeof(gdouble));
		amberParameters->dihedralAngleTerms[i].barrier[j] = barrier;
		amberParameters->dihedralAngleTerms[i].phase	    =
			g_realloc(amberParameters->dihedralAngleTerms[i].phase, len*sizeof(gdouble));
		amberParameters->dihedralAngleTerms[i].phase[j]	 = phase;
		amberParameters->dihedralAngleTerms[i].n	        = 
			g_realloc(amberParameters->dihedralAngleTerms[i].n, len*sizeof(gdouble));
		amberParameters->dihedralAngleTerms[i].n[j]	     = n;

	}

	if(Nc>=0)
		rafreshListTorsions(Nc+1);
	else
		rafreshListTorsions(-1);
}
/********************************************************************************/
static void newTorsionDlg(GtkWidget *win,gpointer data)
{
	editnewTorsionDlg(newTorsion,"Torsion editor",TRUE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T1],FALSE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T2],FALSE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T3],FALSE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T4],FALSE);
}
/********************************************************************************/
static void newBend(GtkWidget *win)
{
	G_CONST_RETURN gchar *t1 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T1]));
	G_CONST_RETURN gchar *t2 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T2]));
	G_CONST_RETURN gchar *t3 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T3]));
	gdouble force = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_F])));
	gdouble angle	 = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_A])));
	gint i = atoi(rowSelectedLists[MMNUMBERBEND]);
	gint j;
	gint number1 = getNumberType(amberParameters,t1);
	gint number2 = getNumberType(amberParameters,t2);
	gint number3 = getNumberType(amberParameters,t3);

	for(j=0;j<amberParameters->numberOfBendTerms;j++)
	{
		if(
			(
			number1 == amberParameters->angleBendTerms[j].numbers[0] &&
			number2 == amberParameters->angleBendTerms[j].numbers[1] &&
			number3 == amberParameters->angleBendTerms[j].numbers[2] 
			) ||			
			(
			number1 == amberParameters->angleBendTerms[j].numbers[2] &&
			number2 == amberParameters->angleBendTerms[j].numbers[1] &&
			number3 == amberParameters->angleBendTerms[j].numbers[0] 
			) 
			)
		{
			gchar* t = g_strdup_printf("Sorry This Bend is define !");
			GtkWidget* w = Message(t,"Error",TRUE);
			gtk_window_set_modal(GTK_WINDOW(w),TRUE);
				gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
			g_free(t);
			return;
		}
	}

	amberParameters->numberOfBendTerms++;

	if(amberParameters->numberOfBendTerms>1)
		amberParameters->angleBendTerms = g_realloc(
		amberParameters->angleBendTerms,
		amberParameters->numberOfBendTerms*sizeof(AmberAngleBendTerms));
	else
		amberParameters->angleBendTerms = g_malloc(sizeof(AmberAngleBendTerms));

	i = amberParameters->numberOfBendTerms-1;
	amberParameters->angleBendTerms[i].numbers[0] = number1;
	amberParameters->angleBendTerms[i].numbers[1] = number2;
	amberParameters->angleBendTerms[i].numbers[2] = number3;
	amberParameters->angleBendTerms[i].forceConstant = force;
	amberParameters->angleBendTerms[i].equilibriumAngle = angle;
	rafreshListBends(i);
}
/********************************************************************************/
static void newBendDlg(GtkWidget *win,gpointer data)
{
	editnewBendDlg(newBend,"new Bend",TRUE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T1],FALSE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T2],FALSE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T3],FALSE);

}
/********************************************************************************/
static void newBond(GtkWidget* w)
{
	G_CONST_RETURN gchar *t1 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T1]));
	G_CONST_RETURN gchar *t2 = gtk_entry_get_text(GTK_ENTRY(Entries[E_T2]));
	gdouble force = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_F])));
	gdouble r	 = atof(gtk_entry_get_text(GTK_ENTRY(Entries[E_R])));
	gint i = atoi(rowSelectedLists[MMNUMBERBOND]);
	gint j;
	gint number1 = getNumberType(amberParameters,t1);
	gint number2 = getNumberType(amberParameters,t2);


	  
	for(j=0;j<amberParameters->numberOfStretchTerms;j++)
	{
		if(
			(
			number1 == amberParameters->bondStretchTerms[j].numbers[0] &&
			number2 == amberParameters->bondStretchTerms[j].numbers[1] 
			) ||			
			(
			number1 == amberParameters->bondStretchTerms[j].numbers[1] &&
			number2 == amberParameters->bondStretchTerms[j].numbers[0] 
			) 
			)
		{
			gchar* t = g_strdup_printf("Sorry This Bond is define !");
			GtkWidget* w = Message(t,"Error",TRUE);
			gtk_window_set_modal(GTK_WINDOW(w),TRUE);
				gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
			g_free(t);
			return;
		}
	}

	amberParameters->numberOfStretchTerms++;

	if(amberParameters->numberOfStretchTerms>1)
		amberParameters->bondStretchTerms = g_realloc(
		amberParameters->bondStretchTerms,
		amberParameters->numberOfStretchTerms*sizeof(AmberBondStretchTerms));
	else
		amberParameters->bondStretchTerms = g_malloc(sizeof(AmberBondStretchTerms));

	i = amberParameters->numberOfStretchTerms-1;
	amberParameters->bondStretchTerms[i].numbers[0] = number1;
	amberParameters->bondStretchTerms[i].numbers[1] = number2;
	amberParameters->bondStretchTerms[i].forceConstant = force;
	amberParameters->bondStretchTerms[i].equilibriumDistance = r;
	rafreshListBonds(i);
}

/********************************************************************************/
static void newBondDlg(GtkWidget *win,gpointer data)
{
	editnewBondDlg(newBond,"New Bond",TRUE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T1],FALSE);
	gtk_editable_set_editable((GtkEditable*) Entries[E_T2],FALSE);
}
/********************************************************************************/
static void newTypeDlg(GtkWidget *win,gpointer data)
{

	editnewTypeDlg(newType,"New type");
	  gtk_editable_set_editable((GtkEditable*) Entries[E_T],TRUE);
}
/********************************************************************************/
static void newDlg(GtkWidget *win,gpointer data)
{
	gint nList = gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBook));
	switch(nList)
	{
		case MMNUMBERTYPE	    : 
			newTypeDlg(win,data);
			break;
		case MMNUMBERBOND	    :
			newBondDlg(win,data);
			break;
		case MMNUMBERBEND	    :
			newBendDlg(win,data);
			break;
		case MMNUMBERTORSION	 :
			newTorsionDlg(win,data);
			break;
		case MMNUMBERIMPROPER	:
			newImproperDlg(win,data);
			break;
		case MMNUMBERHBOND	   :
			newHBondDlg(win,data);
			break;
		case MMNUMBERNONBONDED :
			newNonBondedDlg(win,data);
			break;
		case MMNUMBERPAIRWISE :
			newPairWiseDlg(win,data);
			break;
	}
}
/********************************************************************************/
static void deletePairWise(GtkWidget *win, gpointer data)
{

	gint j = atoi(rowSelectedLists[MMNUMBERPAIRWISE]);
	gint i;

	if(j<0)
		return;

	for(i=j;i<amberParameters->numberOfPairWise-1;i++)
	{
		amberParameters->pairWiseTerms[i] = 
			amberParameters->pairWiseTerms[i+1];
	}
	amberParameters->numberOfPairWise--;
	if(amberParameters->numberOfPairWise>0)
		amberParameters->pairWiseTerms= g_realloc(
		amberParameters->pairWiseTerms,
		amberParameters->numberOfPairWise*sizeof(AmberPairWiseTerms));
	
	rafreshListPairWises(-1);
}
/********************************************************************************/
static void deletePairWiseDlg(GtkWidget *win,gpointer data)
{

	gchar* t1;
	gchar* t2;
	
	gint i = atoi(rowSelectedLists[MMNUMBERPAIRWISE]);
	gint number1 = amberParameters->pairWiseTerms[i].numbers[0];
	gint number2 = amberParameters->pairWiseTerms[i].numbers[1];
	

	gchar *format ="Do you want to really delete the pair wise term for \"%s-%s\" type ?" ;
	gchar *t;
	

	t1 = getType(amberParameters,number1);
	t2 = getType(amberParameters,number2);
	t =g_strdup_printf(format,t1,t2);

	if(i>-1)
		Continue_YesNo(deletePairWise, NULL,t);

	g_free(t);
}
/********************************************************************************/
static void deleteNonBonded(GtkWidget *win, gpointer data)
{

	gint j = atoi(rowSelectedLists[MMNUMBERNONBONDED]);
	gint i;

	if(j<0)
		return;

	for(i=j;i<amberParameters->numberOfNonBonded-1;i++)
	{
		amberParameters->nonBondedTerms[i] = 
			amberParameters->nonBondedTerms[i+1];
	}
	amberParameters->numberOfNonBonded--;
	if(amberParameters->numberOfNonBonded>0)
		amberParameters->nonBondedTerms= g_realloc(
		amberParameters->nonBondedTerms,
		amberParameters->numberOfNonBonded*sizeof(AmberNonBondedTerms));
	
	rafreshListNonBondeds(-1);
}
/********************************************************************************/
static void deleteNonBondedDlg(GtkWidget *win,gpointer data)
{

	gchar* t1;
	
	gint i = atoi(rowSelectedLists[MMNUMBERNONBONDED]);
	gint number1 = amberParameters->nonBondedTerms[i].number;
	

	gchar *format ="Do you want to really delete the non bonded term for \"%s\" type ?" ;
	gchar *t;
	

	t1 = getType(amberParameters,number1);
	t =g_strdup_printf(format,t1);

	if(i>-1)
		Continue_YesNo(deleteNonBonded, NULL,t);

	g_free(t);
}
/********************************************************************************/
static void deleteHBond(GtkWidget *win, gpointer data)
{
	gint j = atoi(rowSelectedLists[MMNUMBERHBOND]);
	gint i;

	if(j<0)
		return;

	for(i=j;i<amberParameters->numberOfHydrogenBonded-1;i++)
	{
		amberParameters->hydrogenBondedTerms[i] = 
			amberParameters->hydrogenBondedTerms[i+1];
	}
	amberParameters->numberOfHydrogenBonded--;
	if(amberParameters->numberOfHydrogenBonded>0)
		amberParameters->hydrogenBondedTerms = g_realloc(
		amberParameters->hydrogenBondedTerms,
		amberParameters->numberOfHydrogenBonded*sizeof(AmberHydrogenBondedTerms));
	
	rafreshListHBonds(-1);
}
/********************************************************************************/
static void deleteHBondDlg(GtkWidget *win,gpointer data)
{
	gchar* t1;
	gchar* t2;

	gint i = atoi(rowSelectedLists[MMNUMBERHBOND]);
	gint number1 = amberParameters->hydrogenBondedTerms[i].numbers[0];
	gint number2 = amberParameters->hydrogenBondedTerms[i].numbers[1];

	gchar *format ="Do you want to really delete \"%s-%s\" H-bond ?" ;
	gchar *t;
	

	t1 = getType(amberParameters,number1);
	t2 = getType(amberParameters,number2);
	t =g_strdup_printf(format,t1,t2);

	if(i>-1)
		Continue_YesNo(deleteHBond, NULL,t);

	g_free(t);
}
/********************************************************************************/
static void deleteImproper(GtkWidget *win, gpointer data)
{

	gint j = atoi(rowSelectedLists[MMNUMBERIMPROPER]);
	gint i;

	if(j<0)
		return;

	for(i=j;i<amberParameters->numberOfImproperTorsionTerms-1;i++)
	{
		amberParameters->improperTorsionTerms[i] = 
			amberParameters->improperTorsionTerms[i+1];
	}
	amberParameters->numberOfImproperTorsionTerms--;
	if(amberParameters->numberOfImproperTorsionTerms>0)
		amberParameters->improperTorsionTerms = g_realloc(
		amberParameters->improperTorsionTerms,
		amberParameters->numberOfImproperTorsionTerms*sizeof(AmberImproperTorsionTerms));
	
	rafreshListImpropers(-1);
}
/********************************************************************************/
static void deleteImproperDlg(GtkWidget *win,gpointer data)
{
	gchar* t1;
	gchar* t2;
	gchar* t3;
	gchar* t4;
	gint number1;
	gint number2;
	gint number3;
	gint number4;
	gchar *format ="Do you want to really delete \"%s-%s-%s-%s\" terms ?" ;
	gchar *t;
	gint i = atoi(rowSelectedLists[MMNUMBERIMPROPER]);
	if(i<0)
		return;

	number1 = amberParameters->improperTorsionTerms[i].numbers[0];
	number2 = amberParameters->improperTorsionTerms[i].numbers[1];
	number3 = amberParameters->improperTorsionTerms[i].numbers[2];
	number4 = amberParameters->improperTorsionTerms[i].numbers[3];

	t1 = getType(amberParameters,number1);
	t2 = getType(amberParameters,number2);
	t3 = getType(amberParameters,number3);
	t4 = getType(amberParameters,number4);
	t =g_strdup_printf(format,t1,t2,t3,t4);

	if(i>-1)
		Continue_YesNo(deleteImproper, NULL,t);

	g_free(t);
}
/********************************************************************************/
static void deleteTorsion(GtkWidget *win, gpointer data)
{
	gint Nc = atoi(rowSelectedLists[MMNUMBERTORSION]);
	gint i;
	gint j;
	gint k;
	gint len;


	if(Nc<0)
		return;
	getIndexTorsion(Nc,&i,&j);
	if(i<0)
		return;
	
	len = amberParameters->dihedralAngleTerms[i].nSomme;
	
	if(len == 1)
	{
		g_free(amberParameters->dihedralAngleTerms[i].divisor);
		g_free(amberParameters->dihedralAngleTerms[i].barrier);
		g_free(amberParameters->dihedralAngleTerms[i].phase);
		g_free(amberParameters->dihedralAngleTerms[i].n);

		len = amberParameters->numberOfDihedralTerms;
		for(k=i;k<len-1;k++)
			amberParameters->dihedralAngleTerms[k] = amberParameters->dihedralAngleTerms[k+1];

		amberParameters->numberOfDihedralTerms--;
		if(amberParameters->numberOfDihedralTerms>0)
			amberParameters->dihedralAngleTerms = g_realloc(amberParameters->dihedralAngleTerms,amberParameters->numberOfDihedralTerms*sizeof(AmberDihedralAngleTerms));

	}
	else
	{
		len = amberParameters->dihedralAngleTerms[i].nSomme;
		for(k=j;k<len-1;k++)
		{
			amberParameters->dihedralAngleTerms[i].divisor[k] =
				amberParameters->dihedralAngleTerms[i].divisor[k+1];
			amberParameters->dihedralAngleTerms[i].barrier[k] =
				amberParameters->dihedralAngleTerms[i].barrier[k+1];
			amberParameters->dihedralAngleTerms[i].phase[k] =
				amberParameters->dihedralAngleTerms[i].phase[k+1];
			amberParameters->dihedralAngleTerms[i].n[k] =
				amberParameters->dihedralAngleTerms[i].n[k+1];
		}

		amberParameters->dihedralAngleTerms[i].nSomme--;
		len = amberParameters->dihedralAngleTerms[i].nSomme;

		amberParameters->dihedralAngleTerms[i].divisor	  = 
			g_realloc(amberParameters->dihedralAngleTerms[i].divisor, len*sizeof(gdouble));
		amberParameters->dihedralAngleTerms[i].barrier	  = 
			g_realloc(amberParameters->dihedralAngleTerms[i].barrier, len*sizeof(gdouble));
		amberParameters->dihedralAngleTerms[i].phase	    =
			g_realloc(amberParameters->dihedralAngleTerms[i].phase, len*sizeof(gdouble));
		amberParameters->dihedralAngleTerms[i].n	        = 
			g_realloc(amberParameters->dihedralAngleTerms[i].n, len*sizeof(gdouble));
	}

	rafreshListTorsions(-1);

}
/********************************************************************************/
static void deleteTorsionDlg(GtkWidget *win,gpointer data)
{

	gchar* t1;
	gchar* t2;
	gchar* t3;
	gchar* t4;

	gint Nc = atoi(rowSelectedLists[MMNUMBERTORSION]);

	gint number1;
	gint number2;
	gint number3;
	gint number4;
	gint i;
	gint j;
	gchar *format ="Do you want to really delete \"%s-%s-%s-%s\" torsion terms ?" ;
	gchar *t;

	if(Nc<0)
		return;

	getIndexTorsion(Nc,&i,&j);


	if(i<0)
		return;

	number1 = amberParameters->dihedralAngleTerms[i].numbers[0];
	number2 = amberParameters->dihedralAngleTerms[i].numbers[1];
	number3 = amberParameters->dihedralAngleTerms[i].numbers[2];
	number4 = amberParameters->dihedralAngleTerms[i].numbers[3];

	t1 = getType(amberParameters,number1);
	t2 = getType(amberParameters,number2);
	t3 = getType(amberParameters,number3);
	t4 = getType(amberParameters,number4);
	
	t =g_strdup_printf(format,t1,t2,t3,t4);

	if(i>-1)
		Continue_YesNo(deleteTorsion, NULL,t);

	g_free(t);
}
/********************************************************************************/
static void deleteBend(GtkWidget *win, gpointer data)
{

	gint j = atoi(rowSelectedLists[MMNUMBERBEND]);
	gint i;

	if(j<0)
		return;

	for(i=j;i<amberParameters->numberOfBendTerms-1;i++)
	{
		amberParameters->angleBendTerms[i] = 
			amberParameters->angleBendTerms[i+1];
	}
	amberParameters->numberOfBendTerms--;
	if(amberParameters->numberOfBendTerms>0)
		amberParameters->angleBendTerms= g_realloc(
		amberParameters->angleBendTerms,
		amberParameters->numberOfBendTerms*sizeof(AmberAngleBendTerms));
	
	rafreshListBends(-1);
}

/********************************************************************************/
static void deleteBendDlg(GtkWidget *win,gpointer data)
{
	gchar* t1;
	gchar* t2;
	gchar* t3;
	gint number1;
	gint number2;
	gint number3;
	gchar *format ="Do you want to really delete \"%s-%s-%s\" bend ?" ;
	gchar *t;
	gint i = atoi(rowSelectedLists[MMNUMBERBEND]);

	if(i<0)
		return;
	number1 = amberParameters->angleBendTerms[i].numbers[0];
	number2 = amberParameters->angleBendTerms[i].numbers[1];
	number3 = amberParameters->angleBendTerms[i].numbers[2];

	t1 = getType(amberParameters,number1);
	t2 = getType(amberParameters,number2);
	t3 = getType(amberParameters,number3);
	t =g_strdup_printf(format,t1,t2,t3);

	if(i>-1)
		Continue_YesNo(deleteBend, NULL,t);

	g_free(t);

}
/********************************************************************************/
static void deleteBond(GtkWidget *win, gpointer data)
{

	gint j = atoi(rowSelectedLists[MMNUMBERBOND]);
	gint i;

	if(j<0)
		return;

	for(i=j;i<amberParameters->numberOfStretchTerms-1;i++)
	{
		amberParameters->bondStretchTerms[i] = 
			amberParameters->bondStretchTerms[i+1];
	}
	amberParameters->numberOfStretchTerms--;
	if(amberParameters->numberOfStretchTerms>0)
		amberParameters->bondStretchTerms = g_realloc(
		amberParameters->bondStretchTerms,
		amberParameters->numberOfStretchTerms*sizeof(AmberBondStretchTerms));
	
	rafreshListBonds(-1);
}
/********************************************************************************/
static void deleteBondDlg(GtkWidget *win,gpointer data)
{

	gchar* t1;
	gchar* t2;
	gint number1;
	gint number2;
	gchar *format ="Do you want to really delete \"%s-%s\" bond ?" ;
	gchar *t;
	gint i = atoi(rowSelectedLists[MMNUMBERBOND]);

	if(i<0)
		return;

	number1 = amberParameters->bondStretchTerms[i].numbers[0];
	number2 = amberParameters->bondStretchTerms[i].numbers[1];

	t1 = getType(amberParameters,number1);
	t2 = getType(amberParameters,number2);
	t =g_strdup_printf(format,t1,t2);

	if(i>-1)
		Continue_YesNo(deleteBond, NULL,t);

	g_free(t);
}
/********************************************************************************/
static void deleteType(GtkWidget *win, gpointer data)
{

	gint j = atoi(rowSelectedLists[MMNUMBERTYPE]);
	gint number = amberParameters->atomTypes[j].number;
	gint i;

	j = -1;
	for(i=0;i<amberParameters->numberOfTypes;i++)
	{
		if(number == amberParameters->atomTypes[i].number)
		{
			j = i;
			break;
		}
	}
	if(j<0)
		return;

	for(i=j;i<amberParameters->numberOfTypes-1;i++)
	{
		amberParameters->atomTypes[i] = 
			amberParameters->atomTypes[i+1];
	}
	amberParameters->numberOfTypes--;
	if(amberParameters->numberOfTypes>0)
		amberParameters->atomTypes = g_realloc(
		amberParameters->atomTypes,
		amberParameters->numberOfTypes*sizeof(AmberAtomTypes));
	rafreshListTypes(j);
}
/********************************************************************************/
static gboolean typeUsed(GtkWidget *win,gpointer data)
{

	gint j = atoi(rowSelectedLists[MMNUMBERTYPE]);
	gint number = amberParameters->atomTypes[j].number;
	gint i;
	gboolean Ok = TRUE;
	
	for(i=0;i<amberParameters->numberOfStretchTerms ;i++)
	{
		if(
			number == amberParameters->bondStretchTerms[i].numbers[0] ||
			number == amberParameters->bondStretchTerms[i].numbers[1] 
		)
		{
			Ok = FALSE;
			break;
		}
	}
	if(Ok)
	for(i=0;i<amberParameters->numberOfBendTerms;i++)
	{
		if(
			number == amberParameters->angleBendTerms[i].numbers[0] ||
			number == amberParameters->angleBendTerms[i].numbers[1] ||
			number == amberParameters->angleBendTerms[i].numbers[2] 
		)
		{
			Ok = FALSE;
			break;
		}
	}
	if(Ok)
	for(i=0;i<amberParameters->numberOfDihedralTerms ;i++)
	{
		if(
			number == amberParameters->dihedralAngleTerms[i].numbers[0] ||
			number == amberParameters->dihedralAngleTerms[i].numbers[1] ||
			number == amberParameters->dihedralAngleTerms[i].numbers[2] ||
			number == amberParameters->dihedralAngleTerms[i].numbers[3] 
		)
		{
			Ok = FALSE;
			break;
		}
	}
	if(Ok)
	for(i=0;i<amberParameters->numberOfImproperTorsionTerms ;i++)
	{
		if(
			number == amberParameters->improperTorsionTerms[i].numbers[0] ||
			number == amberParameters->improperTorsionTerms[i].numbers[1] ||
			number == amberParameters->improperTorsionTerms[i].numbers[2] ||
			number == amberParameters->improperTorsionTerms[i].numbers[3] 
		)
		{
			Ok = FALSE;
			break;
		}
	}
	if(Ok)
	for(i=0;i<amberParameters->numberOfHydrogenBonded ;i++)
	{
		if(
			number == amberParameters->hydrogenBondedTerms[i].numbers[0] ||
			number == amberParameters->hydrogenBondedTerms[i].numbers[1] 
		)
		{
			Ok = FALSE;
			break;
		}
	}
	if(Ok)
	for(i=0;i<amberParameters->numberOfNonBonded;i++)
	{
		if(
			number == amberParameters->nonBondedTerms[i].number
		)
		{
			Ok = FALSE;
			break;
		}
	}
	return !Ok;
}
/********************************************************************************/
static void deleteTypeDlg(GtkWidget *win,gpointer data)
{

	gchar *format ="Do you want to really delete \"%s\" type ?" ;
	gint i = atoi(rowSelectedLists[MMNUMBERTYPE]);
	gchar* type = amberParameters->atomTypes[i].name;
	gchar *t =g_strdup_printf(format,type);

	if(!typeUsed(win,data))
		Continue_YesNo(deleteType, NULL,t);
	else
	{
		GtkWidget* w;
		gchar* t = g_strdup_printf(
			"Sorry this type is used\nPlease delete all parameters for this type beforee"
			);
		w = Message(t,"Warning",TRUE);
		gtk_window_set_modal (GTK_WINDOW (w), TRUE);
			gtk_window_set_transient_for(GTK_WINDOW(w),GTK_WINDOW(SetWinDlg));
		g_free(t);
	}
}
/********************************************************************************/
static void deleteDlg(GtkWidget *win,gpointer data)
{
	gint nList = gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBook));
	switch(nList)
	{
		case MMNUMBERTYPE	    : 
			deleteTypeDlg(win,data);
			break;
		case MMNUMBERBOND	    :
			deleteBondDlg(win,data);
			break;
		case MMNUMBERBEND	    :
			deleteBendDlg(win,data);
			break;
		case MMNUMBERTORSION	 :
			deleteTorsionDlg(win,data);
			break;
		case MMNUMBERIMPROPER	:
			deleteImproperDlg(win,data);
			break;
		case MMNUMBERHBOND	   :
			deleteHBondDlg(win,data);
			break;
		case MMNUMBERNONBONDED :
			deleteNonBondedDlg(win,data);
			break;
		case MMNUMBERPAIRWISE :
			deletePairWiseDlg(win,data);
			break;
	}
}
/********************************************************************************/
static void destroyDlg(GtkWidget *win,gpointer data)
{
	gtk_widget_destroy(SetWinDlg);
}
/********************************************************************************/
GtkWidget * newMMList(GtkWidget *vbox,gint len, gchar** titles, gint* widths, GtkWidget* win)
{
	GtkWidget *scr;
	gint i;
	gint widall=0;
	GtkWidget* list = NULL;
	GType* types = g_malloc(len*sizeof(GType));
	      GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	for(i=0;i<len;i++) types[i] = G_TYPE_STRING;

	for(i=0;i<len;i++) widall+=widths[i];

	widall=widall*Factor+90;

	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeight*0.50));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 2);

	store = gtk_list_store_newv(len, types);
	g_free(types);
	      model = GTK_TREE_MODEL (store);

	list = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (list), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (list), TRUE);

	for (i=0;i<len;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, titles[i]);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, FALSE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
		gtk_tree_view_column_set_min_width(column, widths[i]*Factor);
		gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
	}

	gtk_widget_set_size_request(list,widall*Factor,-1);
	set_base_style(list,55000,55000,55000);
	gtk_container_add(GTK_CONTAINER(scr), list);

	gtk_widget_show (list);
	newMenuToList(list, win);
	return list;
}
/***********************************************************************/
GtkWidget* AddMMListDlg(GtkWidget *NoteBook,gchar* Title,
		gint len, gchar** titles, gint* widths, GtkWidget* win)
{

	GtkWidget *frame;
	GtkWidget *LabelOnglet;
	GtkWidget *LabelMenu;
	GtkWidget *vbox;
	GtkWidget *list;

	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

	LabelOnglet = gtk_label_new(Title);
	LabelMenu = gtk_label_new(Title);
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
				frame,LabelOnglet, LabelMenu);
	g_object_set_data (G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	list = newMMList(vbox,len, titles, widths, win);
	return list;

}
/***********************************************************************/
void setMMParamatersDlg()
{
	GtkWidget *Win;
	GtkWidget *vbox;
	GtkWidget *parentWindow = Fenetre;

	amberParameters = getPointerAmberParameters();
	if(!amberParameters)
	{
		amberParameters = g_malloc(sizeof(AmberParameters));
		*amberParameters = newAmberParameters();
		setPointerAmberParameters(amberParameters);
	}
	setStyles();

	Win= gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_title(GTK_WINDOW(Win),"Set Molecular Mechanics Parameters");
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	SetWinDlg = Win;

	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)gtk_widget_destroy,NULL);
 
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (Win), vbox);

	NoteBook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX (vbox), NoteBook,TRUE, TRUE, 0);

	listTypes = AddMMListDlg(NoteBook," Types ",lengthListsType,typeTitles,typeWidths, Win);
	listBonds = AddMMListDlg(NoteBook," Bonds ",lengthListsBond,bondTitles,bondWidths, Win);
	listBends = AddMMListDlg(NoteBook," Bends ",lengthListsBend,bendTitles,bendWidths, Win);
	listTorsions = AddMMListDlg(NoteBook," Torsions ", lengthListsTorsion,torsionTitles,torsionWidths, Win);
	listImpropers = AddMMListDlg(NoteBook," Improper Torsions ", lengthListsImproper,improperTitles,improperWidths, Win);
	listHBonds = AddMMListDlg(NoteBook," Hydrogen Bonds ", lengthListsHBond,hBondTitles,hBondWidths, Win);
	listNonBondeds = AddMMListDlg(NoteBook," Non Bondeds ", lengthListsNonBonded,nonBondedTitles,nonBondedWidths, Win);
	listPairWises = AddMMListDlg(NoteBook," Pair Wise ", lengthListsPairWise,pairWiseTitles,pairWiseWidths, Win);

	rafreshListTypes(-1);
	rafreshListBonds(-1);
	rafreshListBends(-1);
	rafreshListTorsions(-1);
	rafreshListImpropers(-1);
	rafreshListHBonds(-1);
	rafreshListNonBondeds(-1);
	rafreshListPairWises(-1);
	gtk_widget_realize(Win);
	gtk_widget_show_all(Win);
}
