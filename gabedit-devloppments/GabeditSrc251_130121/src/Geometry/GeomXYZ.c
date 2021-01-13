/* GGeomXYZ.c */
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
#include <ctype.h>
#include <math.h>
#include <gtk/gtk.h>
#include <glib/gprintf.h>

#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Common/Help.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Geometry/GeomGlobal.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/UtilsVASP.h"
#include "../Crystallography/UtilsCIF.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/GeomZmatrix.h"
#include "../Files/ListeFiles.h"
#include "../Common/Windows.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Gaussian/Gaussian.h"
#include "../Molpro/Molpro.h"
#include "../Geometry/GeomConversion.h"
#include "../Common/StockIcons.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/ResultsAnalise.h"
#include "../Geometry/OpenBabel.h"
#include "../Geometry/SelectionDlg.h"
#include "../MolecularMechanics/PDBTemplate.h"
#include "../MolecularMechanics/CalculTypesAmber.h"

#ifdef G_OS_WIN32
#include <fcntl.h>
#include <io.h>
#else 
#include <unistd.h>
#endif

#define MAXNAME 6
#define MAXATOMTYPE 4
#define MAXRESIDUENAME 4
#define MAXSYMBOL 2

typedef enum
{
  E_NUMBER=0,
  E_SYMBOL,
  E_MMTYPE,
  E_PDBTYPE,
  E_RESIDUE,
  E_X,
  E_Y,
  E_Z,
  E_CHARGE,
  E_LAYER
} GabeditEntryType;
static GtkWidget *FenetreTable;

static gint  LineSelectedV;
static GtkWidget *listv=NULL;
static GtkWidget *EntryV[2];
static gint LineSelected;
static GtkWidget *list = NULL;
static GtkWidget *Entry[NUMBER_LIST_XYZ];
static gboolean DestroyDialog;
static gint  NCr;
static gint LineSelectedOld = -1;
static gdouble labelWidth = 0.15;
static gdouble entryWidth = 0.20;
static	gint rowInserted = -1;

/*
static GtkTargetEntry row_targets[] = { { "GTK_TREE_MODEL_ROW", GTK_TARGET_SAME_APP, 0} };
*/

static void TransXYZConstXVar();
static void TransXYZConstYVar();
static void TransXYZConstZVar();
static void get_position(guint i,gdouble Pos[]);
gchar** getListMMTypes(gint* nlist);
gchar** getListPDBTypes(gchar* residueName, gint* nlist);
/********************************************************************************/
static gboolean connected(guint i,guint j);
static void DialogueAdd();
static void DialogueEdit();
static void DialogueDelete();
void create_window_save_xyzmol2tinkerpdbhin();
static void DialogueTransInVar();
static void TransXYZConstVar();
static void MultiByA0();
static void DivideByA0();
static void OriginToCenter();
static void sort_GeomXYZ();
static void clearList(GtkWidget* myList);
static void removeFromList(GtkWidget* myList, gint ligne);
static void insertToList(GtkWidget* myList, gint ligne, gchar* texts[], gint nColumns);
static void appendToList(GtkWidget* myList, gchar* texts[], gint nColumns);
static void append_list();
static gboolean TestVariablesXYZCreated(gchar *NewName,gint j);
static gint testav(gchar *t);
static gint read_gabedit_geoms_file(gchar* fileName, gint geometryNumber);
void read_geom_from_vasp_xml_file(gchar *fileName, gint numgeom);
void read_geom_from_cif_file(gchar *fileName, gboolean applySymOp);
static void trans_coordXYZ_geom(gchar T, GeomXYZAtomDef* myGeomXYZ, gint i, VariablesXYZDef* myVariablesXYZ, gint iv);
static void GeomXYZ_Change_Unit(gboolean toang);
/********************************************************************************/
static gint get_layer(gchar* layer)
{
	if(strstr(layer,"Low")) return LOW_LAYER;
	else if(strstr(layer,"Medium")) return MEDIUM_LAYER;
	else return HIGH_LAYER;
}
/*************************************************************************************/
static guint get_number_of_electrons(guint type)
{
/* 
   type = 1 : Medium and High
   type = 2 : High
   type = other : All
*/
   guint i;
   guint Ne=0;
   SAtomsProp Atom;
   for(i=0;i<NcentersXYZ;i++)
   {
       Atom = prop_atom_get(GeomXYZ[i].Symb);
       switch (type)
       {
      	case 1 : if(get_layer(GeomXYZ[i].Layer)==HIGH_LAYER ||  
				 get_layer(GeomXYZ[i].Layer)==MEDIUM_LAYER) Ne += Atom.atomicNumber;
		 break;
       	case 2 : if(get_layer(GeomXYZ[i].Layer)==HIGH_LAYER) Ne += Atom.atomicNumber;
		 break;
       	default : Ne += Atom.atomicNumber;
        }
	g_free(Atom.name);
	g_free(Atom.symbol);
   }
   return Ne;
} 
/************************************************************************************************************/
static gint get_number_of_model_connections()
{
	gint i;
	gint j;
	gint nc = 0;
	gint NC = NcentersXYZ;
	if(NcentersXYZ<1) return 0;	
    	for(i=0;i<NC;i++)
	{
		if(get_layer(GeomXYZ[i].Layer)==MEDIUM_LAYER || get_layer(GeomXYZ[i].Layer)==LOW_LAYER) continue;
    		for(j=0;j<NC;j++)
		{
			gint nj = j;
			if(i==j) continue;
			if(get_layer(GeomXYZ[j].Layer)==MEDIUM_LAYER || get_layer(GeomXYZ[j].Layer)==LOW_LAYER) continue;
			if(GeomXYZ[i].typeConnections[nj]>0) nc++;
		}
	}
	return nc;
}
/************************************************************************************************************/
static gint get_number_of_inter_connections()
{
	gint i;
	gint j;
	gint nc = 0;
	gint NC = NcentersXYZ;
    	for(i=0;i<NC;i++)
	{
		if(get_layer(GeomXYZ[i].Layer)==HIGH_LAYER || get_layer(GeomXYZ[i].Layer)==LOW_LAYER) continue;
    		for(j=0;j<NC;j++)
		{
			gint nj = j;
			if(i==j) continue;
			if(get_layer(GeomXYZ[j].Layer)==HIGH_LAYER || get_layer(GeomXYZ[j].Layer)==LOW_LAYER) continue;
			if(GeomXYZ[i].typeConnections[nj]>0) nc++;
		}
	}
	return nc;
}
/************************************************************************************************************/
static void reset_spin_of_electrons()
{
        gint i;
        guint NumberElectrons[3];
        guint SpinElectrons[3];
	gint n = 1;
	gint nL = 0;
	gint nM = 0;
	gint nH = 0;

	if(NcentersXYZ<1) return;	

        NumberElectrons[2]= get_number_of_electrons(2);
        NumberElectrons[1]= get_number_of_electrons(1);
        NumberElectrons[0]= get_number_of_electrons(0);


        for(i=0;i<3;i++)
		SpinElectrons[i]=0;
    	for(i=0;i<NcentersXYZ;i++)
	{
		if(get_layer(GeomXYZ[i].Layer)==LOW_LAYER) nL =1;
		if(get_layer(GeomXYZ[i].Layer)==MEDIUM_LAYER) nM =1;
		if(get_layer(GeomXYZ[i].Layer)==HIGH_LAYER) nH =1;
	}
	n = nH + nM + nL;
        if(n==3)
	{
        	NumberElectrons[2] += get_number_of_model_connections();
        	NumberElectrons[1] += get_number_of_inter_connections();
	}
        if(n==2)
	{
        	NumberElectrons[1] += get_number_of_model_connections();
	}

        for(i=0;(guint)i<n;i++)
        	if((NumberElectrons[i]-TotalCharges[i])%2==0)
			SpinElectrons[i]=1;
                else
			SpinElectrons[i]=2;

        for(i=0;(guint)i<n;i++)
	{
		if(SpinMultiplicities[i]%2 != SpinElectrons[i]%2)
			SpinMultiplicities[i] = SpinElectrons[i];
	}
}
/*****************************************************************************/
static void reset_charges_multiplicities_XYZ()
{
	gint i;
	if(Natoms<1) return;	
	for(i=0;i<3;i++)
		TotalCharges[i] = 0;
	reset_spin_of_electrons();

}
/************************************************************************************************************/
void compute_dipole_using_charges_of_xyz_geom()
{
	gdouble charge;
	gdouble x;
	gdouble y;
	gdouble z;
	gint i;

	Dipole.def = TRUE;
	Dipole.value[0] = 0;
	Dipole.value[1] = 0;
	Dipole.value[2] = 0;
	for(i=0;i<NcentersXYZ;i++)
	{
		charge = atof(GeomXYZ[i].Charge);
		if(!test(GeomXYZ[i].X)) x = get_value_variableXYZ(GeomXYZ[i].X);
		else x = atof(GeomXYZ[i].X);
		if(!test(GeomXYZ[i].Y)) y = get_value_variableXYZ(GeomXYZ[i].Y);
		else y = atof(GeomXYZ[i].Y);
		if(!test(GeomXYZ[i].Z)) z = get_value_variableXYZ(GeomXYZ[i].Z);
		else z = atof(GeomXYZ[i].Z);
		Dipole.value[0] += charge*x;
		Dipole.value[1] += charge*y;
		Dipole.value[2] += charge*z;
	}
	if(Units == 1 ) 
		for(i=0;i<3;i++) Dipole.value[i] *= ANG_TO_BOHR;
	for(i=0;i<3;i++)
	              Dipole.value[i] *= AUTODEB;
}
/********************************************************************************/
static void calculMMTypes(gboolean withCharge)
{
	gint i;
	MethodeGeom = GEOM_IS_XYZ;
	define_geometry();
	reset_all_connections();
	calculTypesAmber(geometry,NcentersXYZ);
	for(i=0;i<(gint)NcentersXYZ;i++)
	{
		gint in = geometry[i].N-1;
		GeomXYZ[in].mmType = g_strdup(geometry[i].mmType);
		if(withCharge) GeomXYZ[in].Charge = g_strdup_printf("%f",geometry[i].Charge);
	}
}
/********************************************************************************/
static void set_sensitive_option(GtkUIManager *manager, gchar* path, gboolean sensitive)
{
	GtkWidget *wid = gtk_ui_manager_get_widget (manager, path);
	if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, sensitive);
}
/**********************************************************************************/
static gboolean show_menu_popup(GtkUIManager *manager, gchar* menuName, guint button, guint32 time)
{
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, menuName);
	if (GTK_IS_MENU (menu)) 
	{
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, time);
		return TRUE;
	}
	return FALSE;
}
/*********************************************************************************************************************/
static void activate_action_xyz_geom (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);

	if(!strcmp(name, "Edit")) DialogueEdit();
	else if(!strcmp(name, "New")) DialogueAdd();
	else if(!strcmp(name, "Delete")) DialogueDelete();
	else if(!strcmp(name, "Draw")) draw_geometry(NULL, NULL);
	else if(!strcmp(name, "Save")) create_window_save_xyzmol2tinkerpdbhin(); 
	else if(!strcmp(name, "All")) DialogueTransInVar(); 
	else if(!strcmp(name, "AllX")) TransXYZConstXVar(); 
	else if(!strcmp(name, "AllY")) TransXYZConstYVar(); 
	else if(!strcmp(name, "AllZ")) TransXYZConstZVar(); 
	else if(!strcmp(name, "One")) TransXYZConstVar(); 
	else if(!strcmp(name, "MultiplyBya0")) MultiByA0(); 
	else if(!strcmp(name, "DivideBya0")) DivideByA0(); 
	else if(!strcmp(name, "Center")) OriginToCenter(); 
	else if(!strcmp(name, "Sort")) sort_GeomXYZ(); 
	else if(!strcmp(name, "ToZmat")) conversion_xyz_to_zmat(); 
	else if(!strcmp(name, "ToZmatConn")) conversion_xyz_to_zmat_using_connections(); 
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntriesXYZGeom[] =
{
	{"Edit", NULL, N_("_Edit"), NULL, "Edit", G_CALLBACK (activate_action_xyz_geom) },
	{"New", GABEDIT_STOCK_NEW, N_("_New"), NULL, "New", G_CALLBACK (activate_action_xyz_geom) },
	{"Delete", GABEDIT_STOCK_CUT, N_("_Delete"), NULL, "Delete", G_CALLBACK (activate_action_xyz_geom) },
	{"Draw", GABEDIT_STOCK_DRAW, N_("D_raw"), NULL, "Draw", G_CALLBACK (activate_action_xyz_geom) },
	{"Save", GABEDIT_STOCK_SAVE, N_("_Save"), NULL, "Save", G_CALLBACK (activate_action_xyz_geom) },
	{"All", NULL, N_("_All=>"), NULL, "All=>", G_CALLBACK (activate_action_xyz_geom) },
	{"AllX", NULL,N_("All _X=>"), NULL, "All X=>", G_CALLBACK (activate_action_xyz_geom) },
	{"AllY", NULL,N_("All _Y=>"), NULL, "All Y=>", G_CALLBACK (activate_action_xyz_geom) },
	{"AllZ", NULL,N_("All _Z=>"), NULL, "All Z=>", G_CALLBACK (activate_action_xyz_geom) },
	{"One", NULL,N_("_One=>"), NULL, "One=>", G_CALLBACK (activate_action_xyz_geom) },
	{"MultiplyBya0", GABEDIT_STOCK_A0P, N_("M_ultiply by a0"), NULL, "Multiply by a0", G_CALLBACK (activate_action_xyz_geom) },
	{"DivideBya0", GABEDIT_STOCK_A0D, N_("D_ivide by a0"), NULL, "D_ivide by a0", G_CALLBACK (activate_action_xyz_geom) },
	{"Center", NULL, N_("_Center"), NULL, "Center", G_CALLBACK (activate_action_xyz_geom) },
	{"Sort", NULL, N_("_Sort"), NULL, "Sort", G_CALLBACK (activate_action_xyz_geom) },
	{"ToZmat", NULL, N_("to _Zmat"), NULL, "to _Zmat", G_CALLBACK (activate_action_xyz_geom) },
	{"ToZmatConn", NULL, N_("to _Zmat using connections"), NULL, "to _Zmat using conn.", G_CALLBACK (activate_action_xyz_geom) },
};
static guint numberOfGtkActionEntriesXYZGeom = G_N_ELEMENTS (gtkActionEntriesXYZGeom);
/********************************************************************************/
/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiMenuXYZGeomInfo =
"  <popup name=\"MenuXYZGeom\">\n"
"    <separator name=\"sepMenuPopNew\" />\n"
"    <menuitem name=\"Edit\" action=\"Edit\" />\n"
"    <menuitem name=\"New\" action=\"New\" />\n"
"    <menuitem name=\"Delete\" action=\"Delete\" />\n"
"    <separator name=\"sepMenuPopDraw\" />\n"
"    <menuitem name=\"Draw\" action=\"Draw\" />\n"
"    <separator name=\"sepMenuPopSave\" />\n"
"    <menuitem name=\"Save\" action=\"Save\" />\n"
"    <separator name=\"sepMenuPopSave\" />\n"
"    <menuitem name=\"Save\" action=\"Save\" />\n"
"    <separator name=\"sepMenuPopAll\" />\n"
"    <menuitem name=\"All\" action=\"All\" />\n"
"    <menuitem name=\"AllX\" action=\"AllX\" />\n"
"    <menuitem name=\"AllY\" action=\"AllY\" />\n"
"    <menuitem name=\"AllZ\" action=\"AllZ\" />\n"
"    <menuitem name=\"One\" action=\"One\" />\n"
"    <separator name=\"sepMenuPopMul\" />\n"
"    <menuitem name=\"MultiplyBya0\" action=\"MultiplyBya0\" />\n"
"    <menuitem name=\"DivideBya0\" action=\"DivideBya0\" />\n"
"    <separator name=\"sepMenuPopCent\" />\n"
"    <menuitem name=\"Center\" action=\"Center\" />\n"
"    <menuitem name=\"Sort\" action=\"Sort\" />\n"
"    <separator name=\"sepMenuPopZmat\" />\n"
"    <menuitem name=\"ToZmat\" action=\"ToZmat\" />\n"
"    <menuitem name=\"ToZmatConn\" action=\"ToZmatConn\" />\n"
"  </popup>\n"
;
/*******************************************************************************************************************************/
static GtkUIManager *newMenuXYZGeom(GtkWidget* win)
{
	GtkActionGroup *actionGroup = NULL;
	GtkUIManager *manager = NULL;
	GError *error = NULL;
	static gchar* menuName = "/MenuXYZGeom";

  	manager = gtk_ui_manager_new ();
  	g_signal_connect_swapped (win, "destroy", G_CALLBACK (g_object_unref), manager);

	actionGroup = gtk_action_group_new ("GabeditListOfProject");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntriesXYZGeom, numberOfGtkActionEntriesXYZGeom, NULL);

  	gtk_ui_manager_insert_action_group (manager, actionGroup, 0);

  	gtk_window_add_accel_group (GTK_WINDOW (win), gtk_ui_manager_get_accel_group (manager));
	if (!gtk_ui_manager_add_ui_from_string (manager, uiMenuXYZGeomInfo, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	g_object_set_data(G_OBJECT(manager),"MenuName", menuName);
	return manager;
}
/********************************************************************************/
static void DialogueAddV();
static void DialogueEditV();
static void DialogueDeleteV();
static void DialogueTransInConst();
static void TransXYZVarConst();
/*********************************************************************************************************************/
static void activate_action_xyz_variables (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	if(!strcmp(name, "Edit")) DialogueEditV();
	else if(!strcmp(name, "New")) DialogueAddV();
	else if(!strcmp(name, "Delete")) DialogueDeleteV();
	else if(!strcmp(name, "All")) DialogueTransInConst(); 
	else if(!strcmp(name, "One")) TransXYZVarConst(); 
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntriesXYZVariables[] =
{
	{"Edit", NULL, "_Edit", NULL, "Edit", G_CALLBACK (activate_action_xyz_variables) },
	{"New", GABEDIT_STOCK_NEW, "_New", NULL, "New", G_CALLBACK (activate_action_xyz_variables) },
	{"Delete", GABEDIT_STOCK_CUT, "_Delete", NULL, "Delete", G_CALLBACK (activate_action_xyz_variables) },
	{"All", NULL, "<=_All", NULL, "<=All", G_CALLBACK (activate_action_xyz_variables) },
	{"One", NULL, "<=_On", NULL, "<=One", G_CALLBACK (activate_action_xyz_variables) },
};
static guint numberOfGtkActionEntriesXYZVariables = G_N_ELEMENTS (gtkActionEntriesXYZVariables);
/********************************************************************************/
/* XML description of the menus for the test app.  The parser understands
 * a subset of the Bonobo UI XML format, and uses GMarkup for parsing */
static const gchar *uiMenuXYZVariablesInfo =
"  <popup name=\"MenuXYZVariables\">\n"
"    <separator name=\"sepMenuPopNew\" />\n"
"    <menuitem name=\"Edit\" action=\"Edit\" />\n"
"    <menuitem name=\"New\" action=\"New\" />\n"
"    <menuitem name=\"Delete\" action=\"Delete\" />\n"
"    <separator name=\"sepMenuPopAll\" />\n"
"    <menuitem name=\"All\" action=\"All\" />\n"
"    <menuitem name=\"One\" action=\"One\" />\n"
"  </popup>\n"
;
/*******************************************************************************************************************************/
static GtkUIManager *newMenuXYZVariables(GtkWidget* win)
{
	GtkActionGroup *actionGroup = NULL;
	GtkUIManager *manager = NULL;
	GError *error = NULL;
	static gchar* menuName = "/MenuXYZVariables";

  	manager = gtk_ui_manager_new ();
  	g_signal_connect_swapped (win, "destroy", G_CALLBACK (g_object_unref), manager);

	actionGroup = gtk_action_group_new ("GabeditListOfProject");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntriesXYZVariables, numberOfGtkActionEntriesXYZVariables, NULL);

  	gtk_ui_manager_insert_action_group (manager, actionGroup, 0);

  	gtk_window_add_accel_group (GTK_WINDOW (win), gtk_ui_manager_get_accel_group (manager));
	if (!gtk_ui_manager_add_ui_from_string (manager, uiMenuXYZVariablesInfo, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
	}
	g_object_set_data(G_OBJECT(manager),"MenuName", menuName);
	return manager;
}
/********************************************************************************/
static void row_inserted(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer data)
{
	rowInserted = atoi(gtk_tree_path_to_string(path));
}
/********************************************************************************/
static void row_deleted(GtkTreeModel *model, GtkTreePath *path, gpointer data)
{
	gint rowDeleted = atoi(gtk_tree_path_to_string(path));
	gchar* tmp;
	GList *glist = NULL;
	GList *g = NULL;
	GeomXYZAtomDef* tmpGeomXYZ;
	gint i;
	gint k;
	gint nd;
	gint j;

	if(rowInserted>(gint)NcentersXYZ || rowDeleted>(gint)NcentersXYZ || rowInserted<0 || rowDeleted<0 || rowInserted == rowDeleted) return;	

	for(i=0;i<NcentersXYZ; i++) glist = g_list_append(glist, GINT_TO_POINTER(i));

	nd = rowDeleted+NcentersXYZ;
	if(rowInserted<rowDeleted) nd--;
	glist = g_list_insert(glist, GINT_TO_POINTER(nd), rowInserted);
	if(rowInserted<rowDeleted) glist = g_list_remove(glist, GINT_TO_POINTER(rowDeleted-1));
	if(rowInserted>rowDeleted) glist = g_list_remove(glist, GINT_TO_POINTER(rowDeleted));



	tmpGeomXYZ=g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));
	i = 0;
	g = glist;
	while(g)
	{
		k = GPOINTER_TO_INT(g->data);
		if(k>=NcentersXYZ)
		{
			k -= NcentersXYZ;
		}
		tmpGeomXYZ[i].Symb = g_strdup(GeomXYZ[k].Symb);
		tmpGeomXYZ[i].mmType = g_strdup(GeomXYZ[k].mmType);
		tmpGeomXYZ[i].pdbType = g_strdup(GeomXYZ[k].pdbType);
  		tmpGeomXYZ[i].Residue  = g_strdup(GeomXYZ[k].Residue);
		tmpGeomXYZ[i].X = g_strdup(GeomXYZ[k].X);
		tmpGeomXYZ[i].Y = g_strdup(GeomXYZ[k].Y);
		tmpGeomXYZ[i].Z = g_strdup(GeomXYZ[k].Z);
		tmpGeomXYZ[i].Charge = g_strdup(GeomXYZ[k].Charge);
		tmpGeomXYZ[i].Layer = g_strdup(GeomXYZ[k].Layer);
		tmpGeomXYZ[i].typeConnections = g_malloc(NcentersXYZ*sizeof(gint));
		for(j=0; j<NcentersXYZ; j++) tmpGeomXYZ[i].typeConnections[j] = GeomXYZ[k].typeConnections[j];
		g = g->next;
		i++;
	}

  	g_list_free(glist);
	for(i=0;i<NcentersXYZ;i++)
	{
		g_free(GeomXYZ[i].Symb);
		g_free(GeomXYZ[i].mmType);
		g_free(GeomXYZ[i].pdbType);
		g_free(GeomXYZ[i].Residue);
		g_free(GeomXYZ[i].X);
		g_free(GeomXYZ[i].Y);
		g_free(GeomXYZ[i].Z);
		g_free(GeomXYZ[i].Charge);
		g_free(GeomXYZ[i].Layer);
		if(GeomXYZ[i].typeConnections) g_free(GeomXYZ[i].typeConnections);
	}
	g_free(GeomXYZ);
	GeomXYZ = tmpGeomXYZ;
	
   	clearList(list);
	append_list();

	LineSelected = rowInserted;
	if(rowInserted>rowDeleted) LineSelected--;
	tmp = g_strdup_printf("%d", LineSelected);
	path = gtk_tree_path_new_from_string  (tmp);
	g_free(tmp);
	gtk_tree_selection_select_path  (gtk_tree_view_get_selection (GTK_TREE_VIEW (list)), path);
	gtk_tree_path_free(path);
	NSA[0] = LineSelected+1;
	NSA[1] = NSA[2] = NSA[3] =-1;
 	if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void event_dispatcher(GtkWidget *widget, GdkEventButton *event, gpointer user_data)
{

	GtkTreePath *path;
	gint row = -1;
  	GtkUIManager *manager = NULL;
	gchar* menuName = NULL;

	if (!event) return;
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
  	manager = GTK_UI_MANAGER(user_data);
	menuName = g_object_get_data(G_OBJECT(manager),"MenuName");
	if(strstr(menuName,"Geom"))
	{
  		LineSelected = row;
		NSA[0] = LineSelected+1;
		NSA[1] = NSA[2] = NSA[3] =-1;
	}
	else
	{
  		LineSelectedV = row;
		LineSelectedOld = row;
	}
 	if(GeomDrawingArea != NULL) rafresh_drawing();
    	if (event->type == GDK_2BUTTON_PRESS)
	{

		if(strstr(menuName,"Geom")) DialogueEdit();
		else DialogueEditV();
	}

  	if (event->type == GDK_BUTTON_PRESS && ((GdkEventButton *) event)->button == 3)
	{
		gchar* pathEdit = NULL;
		gchar* pathDelete = NULL;
		gchar* pathOne = NULL;
		gchar* pathAll = NULL;
		GdkEventButton *bevent = (GdkEventButton *) event;

		if(menuName) pathEdit = g_strdup_printf("%s/Edit",menuName);
		if(menuName) pathDelete = g_strdup_printf("%s/Delete",menuName);
		if(menuName) pathOne = g_strdup_printf("%s/One",menuName);
		if(menuName) pathAll = g_strdup_printf("%s/All",menuName);
		if(pathEdit)
		{
			if(row<0) set_sensitive_option(manager, pathEdit, FALSE);
			else set_sensitive_option(manager, pathEdit, TRUE);
			g_free(pathEdit);
		}
		if(pathOne)
		{
			if(row<0) set_sensitive_option(manager, pathOne, FALSE);
			else set_sensitive_option(manager, pathOne, TRUE);
			g_free(pathOne);
		}
		if(pathDelete)
		{
			if(strstr(menuName,"Geom"))
			{
				if(NcentersXYZ <1) set_sensitive_option(manager, pathDelete, FALSE);
				else set_sensitive_option(manager, pathDelete, TRUE);
			}
			else
			if(strstr(menuName,"Variable"))
			{
				if(NVariablesXYZ <1) set_sensitive_option(manager, pathDelete, FALSE);
				else set_sensitive_option(manager, pathDelete, TRUE);
			}
			g_free(pathDelete);
		}
		if(pathAll)
		{
			if(strstr(menuName,"Geom"))
			{
				if(NcentersXYZ <1) set_sensitive_option(manager, pathAll, FALSE);
				else set_sensitive_option(manager, pathAll, TRUE);
			}
			else
			if(strstr(menuName,"Variable"))
			{
				if(NVariablesXYZ <1) set_sensitive_option(manager, pathAll, FALSE);
				else set_sensitive_option(manager, pathAll, TRUE);
			}
			g_free(pathAll);
		}
		show_menu_popup(manager, menuName, bevent->button, bevent->time);
	}
	GTK_WIDGET_GET_CLASS(widget)->button_press_event(widget, event);
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
/*****************************************************************************/
void set_coord(guint i,gdouble V[])
{
         if(!test(GeomXYZ[i].X))
                 V[0] = get_value_variableXYZ(GeomXYZ[i].X);
         else
                 V[0] = atof(GeomXYZ[i].X);
         if(!test(GeomXYZ[i].Y))
                 V[1] = get_value_variableXYZ(GeomXYZ[i].Y);
         else
                 V[1] = atof(GeomXYZ[i].Y);
         if(!test(GeomXYZ[i].Z))
                 V[2] = get_value_variableXYZ(GeomXYZ[i].Z);
         else
                 V[2] = atof(GeomXYZ[i].Z);
         if(Units==1)
         {
              V[0] /= BOHR_TO_ANG;
              V[1] /= BOHR_TO_ANG;
              V[2] /= BOHR_TO_ANG;
         }
}
/************************************************************************/
static void set_multiple_bonds()
{
	gint* nBonds = NULL;
	gint i;
	gint j;
	if(NcentersXYZ<1) return;
	nBonds = g_malloc(NcentersXYZ*sizeof(gint));

	for(i=0;i<(gint)NcentersXYZ;i++) nBonds[i] = 0;
	for(i=0;i<(gint)NcentersXYZ;i++)
		for(j=i+1;j<(gint)NcentersXYZ;j++)
			 if(GeomXYZ[i].typeConnections && GeomXYZ[i].typeConnections[j]!=0) 
			 {
				 nBonds[i] += 1;
				 nBonds[j] += 1;
			 }
	for(i=0;i<(gint)NcentersXYZ;i++)
	{
		SAtomsProp Prop_i = prop_atom_get(GeomXYZ[i].Symb);
		if(!GeomXYZ[i].typeConnections) continue;
		for(j=i+1;j<(gint)NcentersXYZ;j++)
		{
			SAtomsProp Prop_j;
			if(GeomXYZ[i].typeConnections[j]==0) continue;
			Prop_j = prop_atom_get(GeomXYZ[j].Symb);
			if(
		 	nBonds[i] < Prop_i.maximumBondValence &&
		 	nBonds[j] < Prop_j.maximumBondValence 
			)
			{
				GeomXYZ[i].typeConnections[j] = 2;
				if(GeomXYZ[j].typeConnections) GeomXYZ[j].typeConnections[i] = 2;
				nBonds[i] += 1;
				nBonds[j] += 1;
			}
			g_free(Prop_j.name);
			g_free(Prop_j.symbol);
		}
		g_free(Prop_i.name);
		g_free(Prop_i.symbol);
	}
	for(i=0;i<(gint)NcentersXYZ;i++)
	{
		SAtomsProp Prop_i = prop_atom_get(GeomXYZ[i].Symb);
		if(!GeomXYZ[i].typeConnections) continue;
		for(j=i+1;j<(gint)NcentersXYZ;j++)
		{
			SAtomsProp Prop_j;
			if(GeomXYZ[i].typeConnections[j]==0) continue;
			Prop_j = prop_atom_get(GeomXYZ[j].Symb);
			if(
		 	nBonds[i] < Prop_i.maximumBondValence &&
		 	nBonds[j] < Prop_j.maximumBondValence 
			)
			{
				GeomXYZ[i].typeConnections[j] = 3;
				if(GeomXYZ[j].typeConnections) GeomXYZ[j].typeConnections[i] = 3;
				nBonds[i] += 1;
				nBonds[j] += 1;
			}
			g_free(Prop_j.name);
			g_free(Prop_j.symbol);
		}
		g_free(Prop_i.name);
		g_free(Prop_i.symbol);
	}
	g_free(nBonds);
}
/*****************************************************************************/
void reset_connections_XYZ()
{
	gint i,j;
	for(i=0;i<NcentersXYZ;i++)
	{
		if(GeomXYZ[i].typeConnections) g_free(GeomXYZ[i].typeConnections);
		GeomXYZ[i].typeConnections = g_malloc(NcentersXYZ*sizeof(gint));
		for(j=0;j<NcentersXYZ;j++) GeomXYZ[i].typeConnections[j] = 0;
		for(j=0;j<NcentersXYZ;j++) 
			if(i!=j && connected(i,j)) GeomXYZ[i].typeConnections[j]=1;
	}
	set_multiple_bonds();
}
/*****************************************************************************/
gboolean connecteds(guint i,guint j)
{
  gdouble distance;
  gdouble dif[3];
  guint k;
  gdouble V1[3];
  gdouble V2[3];
  gdouble vd1;
  gdouble vd2;
  SAtomsProp Prop;

  if(GeomXYZ[i].typeConnections)
  {
	  if(GeomXYZ[i].typeConnections[j]>0) return TRUE;
	  else return FALSE;
  }
  if(GeomXYZ[j].typeConnections)
  {
	  if(GeomXYZ[j].typeConnections[i]>0) return TRUE;
	  else return FALSE;
  }
  set_coord(i,V1);
  set_coord(j,V2);

  dif[0] = V1[0]-V2[0];
  dif[1] = V1[1]-V2[1];
  dif[2] = V1[2]-V2[2];

  distance = 0;
  for (k=0;k<3;k++)
	distance +=dif[k]*dif[k];
  distance = sqrt(distance);

  Prop = prop_atom_get(GeomXYZ[i].Symb);
  vd1 = Prop.covalentRadii;
  g_free(Prop.symbol);

  Prop = prop_atom_get(GeomXYZ[j].Symb);
  vd2 = Prop.covalentRadii;
  g_free(Prop.name);
  g_free(Prop.symbol);

  if(distance<(vd1+vd2))
	return TRUE;
  else 
	return FALSE;
}
/********************************************************************************/
static void clearList(GtkWidget* myList)
{
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(myList));
        GtkListStore *store = GTK_LIST_STORE (model);
	if(myList == list) g_signal_handlers_disconnect_by_func (model, row_deleted, NULL);
	gtk_list_store_clear(store);
	if(myList == list) g_signal_connect_after(G_OBJECT (model), "row_deleted", G_CALLBACK(row_deleted), NULL);      
}
/********************************************************************************/
static void removeFromList(GtkWidget* myList, gint ligne)
{
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;
	gchar* tmp;

	if(ligne<0) return;

	tmp = g_strdup_printf("%d",ligne);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(myList));
        store = GTK_LIST_STORE (model);

	if(myList == list) g_signal_handlers_disconnect_by_func (model, row_deleted, NULL);
	if(gtk_tree_model_get_iter_from_string (model, &iter, tmp))
	{
		gtk_list_store_remove(store, &iter);
	}
	g_free(tmp);
	if(myList == list) g_signal_connect_after(G_OBJECT (model), "row_deleted", G_CALLBACK(row_deleted), NULL);      



}
/********************************************************************************/
static void changeNameVariableInGeometry(gchar* oldName, gchar* newName)
{
	gint i;
	gint k=-1;
	for(i=0;i<NcentersXYZ;i++)
	{
  		if(!strcmp(GeomXYZ[i].X,oldName))
		{
			if(GeomXYZ[i].X) g_free(GeomXYZ[i].X);
			GeomXYZ[i].X =  g_strdup(newName);
			k = 0;
		}
  		if(!strcmp(GeomXYZ[i].Y,oldName))
		{
			if(GeomXYZ[i].Y) g_free(GeomXYZ[i].Y);
			GeomXYZ[i].Y =  g_strdup(newName);
			k = 0;
		}
  		if(!strcmp(GeomXYZ[i].Z,oldName))
		{
			if(GeomXYZ[i].Z) g_free(GeomXYZ[i].Z);
			GeomXYZ[i].Z =  g_strdup(newName);
			k=0;
		}
	}
	if(k==0)
	{
		clearList(list);
		append_list();
	}
}
/********************************************************************************/
static void editedVariable (GtkCellRendererText *cell, gchar  *path_string,
		    gchar *new_text, gpointer data)
{
	GtkTreeModel *model = GTK_TREE_MODEL (data);
	GtkTreeIter iter;
	GtkTreePath *path = NULL;
	gint numCol = 0;
	gint Nc = -1;
	numCol = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell),"NumColumn"));
	gchar* oldName = NULL;
	if(numCol==0)
	{
		Nc = -1;
  		if(!variable_name_valid(new_text))
  		{
			show_forbidden_characters();
      			return;
  		} 
        	if ( !strcmp(new_text, "")) return;
	  	Nc = atoi(path_string);
  		if(TestVariablesXYZCreated(new_text,Nc) )
  		{
			MessageGeom(_("Sorry a other variable have any Name !\n"),_("Error"),TRUE);
      			return;
		} 
	}
	if(numCol==1)
	{
  		if(!test(new_text))
		{
			gchar* message=g_strdup_printf(_("Sorry %s is not a number \n"),new_text);
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
			return;
  		}
        	if ( !strcmp(new_text, "")) return;
	  	Nc = atoi(path_string);
  		if(test(new_text) && !testpointeE(new_text) )
			new_text=g_strdup_printf("%s.0",new_text);
	}
	if(Nc<0)return;
	if(numCol==0)
	{
		oldName = VariablesXYZ[Nc].Name;
  		VariablesXYZ[Nc].Name=g_strdup(new_text);
	}
	if(numCol==1)
	{
		if(VariablesXYZ[Nc].Value) g_free(VariablesXYZ[Nc].Value);
  		VariablesXYZ[Nc].Value=g_strdup(new_text);
	}

	path = gtk_tree_path_new_from_string (path_string);
	gtk_tree_model_get_iter (model, &iter, path);
	gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);

	gtk_tree_path_free (path);
	if(numCol==0 && oldName) 
	{
		changeNameVariableInGeometry(oldName, new_text);
		g_free(oldName);
	}
	if(numCol==1 && GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void editedGeom (GtkCellRendererText *cell, gchar  *path_string,
		    gchar *new_text, gpointer data)
{
	GtkTreeModel *model = GTK_TREE_MODEL (data);
	GtkTreeIter iter;
	GtkTreePath *path = NULL;
	gint numCol = 0;
	gint Nc = -1;
	if(!new_text) return;
	numCol = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(cell),"NumColumn"));
	if(numCol==0) 
	{
		gint i;
		gint j;
		gint oldNum = atoi(path_string);
		gint newNum = atoi(new_text)-1;
		GeomXYZAtomDef g;
		if(newNum<0 || newNum>=NcentersXYZ)return;
		if(newNum==oldNum)return;
		/* swap oldNum and newNum */
		g = GeomXYZ[oldNum];
		GeomXYZ[oldNum] = GeomXYZ[newNum];
		GeomXYZ[newNum] = g;
  		for (i = 0; i <(gint)NcentersXYZ; i++)
  		{
			if(!GeomXYZ[i].typeConnections) continue;
			j = GeomXYZ[i].typeConnections[oldNum];
			GeomXYZ[i].typeConnections[oldNum] = GeomXYZ[i].typeConnections[newNum];
			GeomXYZ[i].typeConnections[newNum] = j;
  		}

   		clearList(list);
		append_list();
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
	/* symbol */
	if(numCol==1)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
		new_text[0]=toupper(new_text[0]);
		if(nc>1)new_text[1]=tolower(new_text[1]);
		if(!test_atom_define(new_text))
		{
			gchar* message=g_strdup_printf(_("Sorry %s is not a symbol for an atom \n"),new_text);
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
			return;
		}
	  	Nc = atoi(path_string);
  		if(GeomXYZ[Nc].Symb) g_free(GeomXYZ[Nc].Symb);
		GeomXYZ[Nc].Symb = g_strdup(new_text);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
	/* MM Type */
	if(numCol==2)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
	  	Nc = atoi(path_string);
  		if(GeomXYZ[Nc].mmType) g_free(GeomXYZ[Nc].mmType);
		GeomXYZ[Nc].mmType = g_strdup(new_text);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
	/* PDB Type */
	if(numCol==3)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
	  	Nc = atoi(path_string);
  		if(GeomXYZ[Nc].pdbType) g_free(GeomXYZ[Nc].pdbType);
		GeomXYZ[Nc].pdbType = g_strdup(new_text);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
	/* Residue Name  */
	if(numCol==4)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
	  	Nc = atoi(path_string);
  		if(strcmp(GeomXYZ[Nc].Residue,new_text))
  		{
  			if(GeomXYZ[Nc].Residue) g_free(GeomXYZ[Nc].Residue);
			GeomXYZ[Nc].Residue = g_strdup(new_text);
	  		gint k;
	  		GeomXYZ[Nc].ResidueNumber = -1; 
	  		for(k=0;k<(gint)NcentersXYZ;k++)
	  		{
		  		if(Nc != k && !strcmp(GeomXYZ[Nc].Residue,GeomXYZ[k].Residue))
		  		{
			  		GeomXYZ[Nc].ResidueNumber = GeomXYZ[k].ResidueNumber;
			  		break;
		  		}
	  		}
	  		if(GeomXYZ[Nc].ResidueNumber == -1)
	  		{
	  			for(k=0;k<(gint)NcentersXYZ;k++)
	  			{
		  			if(GeomXYZ[Nc].ResidueNumber<GeomXYZ[k].ResidueNumber)
			  			GeomXYZ[Nc].ResidueNumber = GeomXYZ[k].ResidueNumber;
	  			}
	  			GeomXYZ[Nc].ResidueNumber += 1;
	  		}
  		}
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
	/* X, Y or Z  */
	if(numCol==5 || numCol==6 || numCol==7)
	{
		gint nc = strlen(new_text);
		if(nc<1)return;
	  	Nc = atoi(path_string);
		if(testav(new_text)<-1)
		{	
			gchar* message=g_strdup_printf(_("Sorry\n %s \nis not a number \nand is not a variable "),new_text);
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
        		return;
		}
  		if(test(new_text) && !testpointeE(new_text) )
			new_text=g_strdup_printf("%s.0",new_text);
		if(numCol==5)
		{
  			if(GeomXYZ[Nc].X) g_free(GeomXYZ[Nc].X);
			GeomXYZ[Nc].X = g_strdup(new_text);
		}
		if(numCol==6)
		{
  			if(GeomXYZ[Nc].Y) g_free(GeomXYZ[Nc].Y);
			GeomXYZ[Nc].Y = g_strdup(new_text);
		}
		if(numCol==7)
		{
  			if(GeomXYZ[Nc].Z) g_free(GeomXYZ[Nc].Z);
			GeomXYZ[Nc].Z = g_strdup(new_text);
		}
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
	/* Charge  */
	if(numCol==8)
	{
		gint nc = strlen(new_text);
		gchar* txt = NULL;
		if(nc<1)return;
	  	Nc = atoi(path_string);
  		if(!test(new_text))
		{
			gchar* message=g_strdup_printf(_("Sorry %s is not a number \n"),new_text);
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
			return;
  		}
  		if(test(new_text) && !testpointeE(new_text) )
			txt=g_strdup_printf("%s.0",new_text);
		else
			txt=g_strdup_printf("%s",new_text);
  		if(GeomXYZ[Nc].Charge) g_free(GeomXYZ[Nc].Charge);
		GeomXYZ[Nc].Charge = g_strdup(new_text);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, txt, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		if(txt) g_free(txt);
		return;
	}
	/* Layer  */
	if(numCol==9)
	{
		gint nc = strlen(new_text);
		gint i;
		if(nc<1)return;
		new_text[0] = toupper(new_text[0]);
		for(i=1;i<nc;i++) new_text[i] = tolower(new_text[i]);
	  	Nc = atoi(path_string);
  		if(!(!strcmp(new_text,"High") 
			|| !strcmp(new_text,"Medium")
			||!strcmp(new_text,"Low")
			|| !strcmp(new_text," ")))
		{
			gchar* message=g_strdup_printf(_("Sorry  The layer should be High, Medium, Low or one space \n"));
			MessageGeom(message,_("Error"),TRUE);
			g_free(message);
			return;
  		}
  		if(GeomXYZ[Nc].Layer) g_free(GeomXYZ[Nc].Layer);
		GeomXYZ[Nc].Layer = g_strdup(new_text);
		path = gtk_tree_path_new_from_string (path_string);
		gtk_tree_model_get_iter (model, &iter, path);
		gtk_list_store_set (GTK_LIST_STORE (model), &iter, 2*numCol, new_text, -1);
		gtk_tree_path_free (path);
		if(GeomDrawingArea != NULL) rafresh_drawing();
		return;
	}
}
/********************************************************************************/
static void insertToList(GtkWidget* myList, gint ligne, gchar* texts[], gint nColumns)
{
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;
	gint k;

	if(ligne<0) ligne = 0;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(myList));
        store = GTK_LIST_STORE (model);

	if(myList == list) g_signal_handlers_disconnect_by_func (model, row_inserted, NULL);
	gtk_list_store_insert(store, &iter, ligne);
	for(k=0;k<nColumns;k++)
	{
		gboolean ed = TRUE;
		/*if(k==0 && myList == list) ed=FALSE;*/
       		gtk_list_store_set (store, &iter, k+k, texts[k],k+k+1,ed, -1);
		g_free(texts[k]);
	}
	if(myList == list) g_signal_connect_after(G_OBJECT (model), "row_inserted", G_CALLBACK(row_inserted), NULL);      
}
/********************************************************************************/
static void appendToList(GtkWidget* myList, gchar* texts[], gint nColumns)
{
	GtkTreeModel *model;
        GtkListStore *store;
	GtkTreeIter  iter;
	gint k;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(myList));
        store = GTK_LIST_STORE (model);

	if(myList == list) g_signal_handlers_disconnect_by_func (model, row_inserted, NULL);
	gtk_list_store_append(store, &iter);
	for(k=0;k<nColumns;k++)
	{
		gboolean ed = TRUE;
		/* if(k==0 && myList == list) ed=FALSE;*/
       		gtk_list_store_set (store, &iter, k+k, texts[k],k+k+1,ed, -1);
		g_free(texts[k]);
	}
	if(myList == list) g_signal_connect_after(G_OBJECT (model), "row_inserted", G_CALLBACK(row_inserted), NULL);      
}
/********************************************************************************/
static void append_list()
{
	gchar *texts[NUMBER_LIST_XYZ];
	guint Nc;
	gint k;
        GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(list));
        GtkListStore *store = GTK_LIST_STORE (model);
  
        model = GTK_TREE_MODEL (store);
	g_signal_handlers_disconnect_by_func (model, row_inserted, NULL);

	for(Nc=0;Nc<NcentersXYZ;Nc++)
	{
 	 	texts[E_NUMBER] = g_strdup_printf("%d",Nc+1);
  		texts[E_SYMBOL] = g_strdup(GeomXYZ[Nc].Symb);
  		texts[E_RESIDUE] = g_strdup(GeomXYZ[Nc].Residue);
  		texts[E_MMTYPE] = g_strdup(GeomXYZ[Nc].mmType);
  		texts[E_PDBTYPE] = g_strdup(GeomXYZ[Nc].pdbType);
  		texts[E_X] = g_strdup(GeomXYZ[Nc].X);
  		texts[E_Y] = g_strdup(GeomXYZ[Nc].Y);
  		texts[E_Z] = g_strdup(GeomXYZ[Nc].Z);
  		texts[E_CHARGE] = g_strdup(GeomXYZ[Nc].Charge);
  		texts[E_LAYER] = g_strdup(GeomXYZ[Nc].Layer);

        	gtk_list_store_append (store, &iter);
		for(k=0;k<NUMBER_LIST_XYZ;k++)
		{
			gboolean ed = TRUE;
			/* if(k==0) ed=FALSE;*/
       			gtk_list_store_set (store, &iter, k+k, texts[k],k+k+1,ed, -1);
			g_free(texts[k]);
		}
	}
  	g_signal_connect_after(G_OBJECT (model), "row_inserted", G_CALLBACK(row_inserted), NULL);      
}
/*****************************************************************************/
static gdouble get_distance_xyz(gint i,gint j)
{
        guint k;
        gchar *serr;
        gdouble res;
	Point A;
	Point B;
	gdouble Pos[3];

        if(i>=(gint)NcentersXYZ || j >=(gint)NcentersXYZ || i<0 || j<0)
        {
           return -1000; 
        }
	get_position(i,Pos);
	for(k=0;k<3;k++)
		A.C[k] = Pos[k];
	get_position(j,Pos);
	for(k=0;k<3;k++)
		B.C[k] = Pos[k];
   	serr = get_distance_points(A,B,FALSE);
	res = atof(serr);
	g_free(serr);
        return res;
}
/*****************************************************************************/
static void sort_GeomXYZ()
{
  gint i;
  gint j;
  gint k;
  GeomXYZAtomDef t;
  gint* oldNum = NULL;
  gint* N = NULL;


  if (NcentersXYZ <= 0) return ;
  oldNum = g_malloc(NcentersXYZ*sizeof(gint));
  for (i = 0; i <(gint)NcentersXYZ; i++) oldNum[i] = i;

  N = g_malloc(NcentersXYZ*sizeof(gint));
  for (i = 0; i <(gint)NcentersXYZ; i++) N[i] = 0;

  for (i = 1; i <(gint)NcentersXYZ-1; i++)
  {
	k = i;
  	for (j = i+1; j <(gint)NcentersXYZ; j++)
  	if( get_distance_xyz(j,i-1)<get_distance_xyz(k,i-1))
		k = j; 
	if(k!=i)
	{
		t= GeomXYZ[i];
		GeomXYZ[i] = GeomXYZ[k];
		GeomXYZ[k] = t;
		j = oldNum[i];
		oldNum[i] = oldNum[k];
		oldNum[k] = j;
	}
  }
  for (i = 0; i <(gint)NcentersXYZ; i++)
  {
	if(!GeomXYZ[i].typeConnections) continue;
  	for (j = 0; j <(gint)NcentersXYZ; j++) N[j] = GeomXYZ[i].typeConnections[j];
  	for (j = 0; j <(gint)NcentersXYZ; j++) GeomXYZ[i].typeConnections[j]=N[oldNum[j]];
  }
  if(N) g_free(N);
  if(oldNum) g_free(oldNum);

  if(GeomIsOpen)
  {
   	clearList(list);
   	append_list();
  }
   if(GeomDrawingArea != NULL)
         draw_geometry(NULL,NULL);
}
/*****************************************************************************/
static void sortGeomXYZByResidueNumber()
{
  gint i;
  gint j;
  gint k;
  GeomXYZAtomDef t;
  gint* oldNum = NULL;
  gint* N = NULL;

  if (NcentersXYZ <= 0) return ;
  oldNum = g_malloc(NcentersXYZ*sizeof(gint));
  for (i = 0; i <(gint)NcentersXYZ; i++) oldNum[i] = i;

  N = g_malloc(NcentersXYZ*sizeof(gint));
  for (i = 0; i <(gint)NcentersXYZ; i++) N[i] = 0;

  for (i = 0; i <(gint)NcentersXYZ-1; i++)
  {
	k = i;
  	for (j = i+1; j <(gint)NcentersXYZ; j++)
  	if( GeomXYZ[j].ResidueNumber<GeomXYZ[k].ResidueNumber)
		k = j; 
	if(k!=i)
	{
		t= GeomXYZ[i];
		GeomXYZ[i] = GeomXYZ[k];
		GeomXYZ[k] = t;
		j = oldNum[i];
		oldNum[i] = oldNum[k];
		oldNum[k] = j;
	}
  }
  for (i = 0; i <(gint)NcentersXYZ; i++)
  {
	if(!GeomXYZ[i].typeConnections) continue;
  	for (j = 0; j <(gint)NcentersXYZ; j++) N[j] = GeomXYZ[i].typeConnections[j];
  	for (j = 0; j <(gint)NcentersXYZ; j++) GeomXYZ[i].typeConnections[j]=N[oldNum[j]];
  }
  if(N) g_free(N);
  if(oldNum) g_free(oldNum);
  if(GeomIsOpen)
  {
   	clearList(list);
   	append_list();
  }
   if(GeomDrawingArea != NULL)
         draw_geometry(NULL,NULL);
}
/********************************************************************************/
void append_VariablesXYZ_in_list()
{
	gchar *texts[2];
	guint i;
	gint k;
        GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(listv));
        GtkListStore *store = GTK_LIST_STORE (model);
  
        model = GTK_TREE_MODEL (store);

	for(i=0;i<NVariablesXYZ;i++)
	{
  		texts[0] = g_strdup(VariablesXYZ[i].Name);
  		texts[1] = g_strdup(VariablesXYZ[i].Value);

        	gtk_list_store_append (store, &iter);
		for(k=0;k<2;k++)
		{
        		gtk_list_store_set (store, &iter, k+k, texts[k],k+k+1,TRUE, -1);
			g_free(texts[k]);
		}
	}
}
/********************************************************************************/
void FreeGeomXYZ(GeomXYZAtomDef* GeomXYZtemp, VariablesXYZDef* VariablesXYZtemp, gint Ncent, gint Nvar)
{
 gint i;
 if(GeomXYZtemp)
 {
 	for(i=0;i<Ncent;i++)
 	{
	if(GeomXYZtemp[i].Symb)
   		g_free(GeomXYZtemp[i].Symb);
	if(GeomXYZtemp[i].mmType)
   		g_free(GeomXYZtemp[i].mmType);
	if(GeomXYZtemp[i].pdbType)
   		g_free(GeomXYZtemp[i].pdbType);
	if(GeomXYZtemp[i].X)
   		g_free(GeomXYZtemp[i].X);
	if(GeomXYZtemp[i].Y)
   		g_free(GeomXYZtemp[i].Y);
	if(GeomXYZtemp[i].Z)
   		g_free(GeomXYZtemp[i].Z);
	if(GeomXYZtemp[i].Charge)
   		g_free(GeomXYZtemp[i].Charge);
	if(GeomXYZtemp[i].Layer)
   		g_free(GeomXYZtemp[i].Layer);
	if(GeomXYZtemp[i].typeConnections)
   		g_free(GeomXYZtemp[i].typeConnections);
 	}
 	g_free(GeomXYZtemp);
 }
 if(VariablesXYZtemp)
 {
 	for(i=0;i<Nvar;i++)
 	{
   		g_free(VariablesXYZtemp[i].Name);
   		g_free(VariablesXYZtemp[i].Value);
 	}
   	g_free(VariablesXYZtemp);
 }
 init_dipole();
}
/********************************************************************************/
void freeGeomXYZ()
{
 guint i;
 for(i=0;i<NcentersXYZ;i++)
 {
   g_free(GeomXYZ[i].Symb);
   g_free(GeomXYZ[i].mmType);
   g_free(GeomXYZ[i].pdbType);
   g_free(GeomXYZ[i].Residue);
   g_free(GeomXYZ[i].X);
   g_free(GeomXYZ[i].Y);
   g_free(GeomXYZ[i].Z);
   g_free(GeomXYZ[i].Charge);
   g_free(GeomXYZ[i].Layer);
   if(GeomXYZ[i].typeConnections) g_free(GeomXYZ[i].typeConnections);
 }
 g_free(GeomXYZ);
 GeomXYZ = NULL;
 NcentersXYZ = 0;
 init_dipole();
}
/********************************************************************************/
void freeVariablesXYZ()
{
 guint i;
 for(i=0;i<NVariablesXYZ;i++)
 {
   g_free(VariablesXYZ[i].Name);
   g_free(VariablesXYZ[i].Value);
 }
 g_free(VariablesXYZ);
 VariablesXYZ = NULL;
 NVariablesXYZ = 0;
}
/********************************************************************************/
static void destroy_dialogue(GtkWidget *win,gpointer d)
{
 if(DestroyDialog)
 {
         delete_child(win);
 }
}
/********************************************************************************/
void compute_volume_molecule()
{
	gint i,j;
	gdouble X;
	gdouble Y;
	gdouble Z;
	gdouble vol = 0;
	gdouble r =0;
	gdouble rmax =0;
	gdouble C[3]={0,0,0};
	gint nAtoms = 0;
	gdouble volNonH = 18.0;
	gint nAtomsNonH = 0;


	if(NcentersXYZ>1)
	/*
	for(i=0;i<NcentersXYZ;i++) 
	{
		if(!strcmp(GeomXYZ[i].Symb, "TV") || !strcmp(GeomXYZ[i].Symb, "Tv")) continue;
         	if(!test(GeomXYZ[i].X)) X = get_value_variableXYZ(GeomXYZ[i].X); else X = atof(GeomXYZ[i].X);
		if(!test(GeomXYZ[i].Y)) Y = get_value_variableXYZ(GeomXYZ[i].Y); else Y = atof(GeomXYZ[i].Y);
		if(!test(GeomXYZ[i].Z)) Z = get_value_variableXYZ(GeomXYZ[i].Z); else Z = atof(GeomXYZ[i].Z);
		C[0] = X;
		C[1] = Y;
		C[2] = Z;
		nAtoms++;
		if(strcmp(GeomXYZ[i].Symb, "H"))  nAtomsNonH++;
		for(j=i+1;j<NcentersXYZ;j++) 
		{
			if(!strcmp(GeomXYZ[j].Symb, "TV") || !strcmp(GeomXYZ[j].Symb, "Tv")) continue;
         		if(!test(GeomXYZ[j].X)) X = get_value_variableXYZ(GeomXYZ[j].X); else X = atof(GeomXYZ[j].X);
			if(!test(GeomXYZ[j].Y)) Y = get_value_variableXYZ(GeomXYZ[j].Y); else Y = atof(GeomXYZ[j].Y);
			if(!test(GeomXYZ[j].Z)) Z = get_value_variableXYZ(GeomXYZ[j].Z); else Z = atof(GeomXYZ[j].Z);
			r = (C[0]-X)*(C[0]-X)+(C[1]-Y)*(C[1]-Y)+(C[2]-Z)*(C[2]-Z);
			if(r>rmax) rmax = r;
		}
	}
	rmax = sqrt(rmax)/2;
	*/
	for(i=0;i<NcentersXYZ;i++) 
	{
		if(!strcmp(GeomXYZ[i].Symb, "TV") || !strcmp(GeomXYZ[i].Symb, "Tv")) continue;
         	if(!test(GeomXYZ[i].X)) X = get_value_variableXYZ(GeomXYZ[i].X); else X = atof(GeomXYZ[i].X);
		if(!test(GeomXYZ[i].Y)) Y = get_value_variableXYZ(GeomXYZ[i].Y); else Y = atof(GeomXYZ[i].Y);
		if(!test(GeomXYZ[i].Z)) Z = get_value_variableXYZ(GeomXYZ[i].Z); else Z = atof(GeomXYZ[i].Z);
		C[0] += X;
		C[1] += Y;
		C[2] += Z;
		nAtoms++;
		if(strcmp(GeomXYZ[i].Symb, "H"))  nAtomsNonH++;
	}
	if(nAtoms>1)
	{
		for(i=0;i<3;i++) C[i] /= nAtoms;
		for(i=0;i<NcentersXYZ;i++) 
		{
			if(!strcmp(GeomXYZ[i].Symb, "TV") || !strcmp(GeomXYZ[i].Symb, "Tv")) continue;
         		if(!test(GeomXYZ[i].X)) X = get_value_variableXYZ(GeomXYZ[i].X); else X = atof(GeomXYZ[i].X);
			if(!test(GeomXYZ[i].Y)) Y = get_value_variableXYZ(GeomXYZ[i].Y); else Y = atof(GeomXYZ[i].Y);
			if(!test(GeomXYZ[i].Z)) Z = get_value_variableXYZ(GeomXYZ[i].Z); else Z = atof(GeomXYZ[i].Z);
			r = (C[0]-X)*(C[0]-X)+(C[1]-Y)*(C[1]-Y)+(C[2]-Z)*(C[2]-Z);
			if(r>rmax) rmax = r;
		}
		rmax = sqrt(rmax);
	}

	if(nAtoms<2)
	{
		gchar* t = g_strdup_printf(_("Sorry,\nI cannot compute volume with only %d atoms"),nAtoms);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	if(Units==0) rmax *= BOHR_TO_ANG;

	{
		gchar* t = g_strdup_printf(_(
		"rmax = %lf Angstrom\nVolume = 4/3 pi rmax^3 = %0.6lf Angstrom^3\n\n"
		"Volume by Crystallographers (18 ang^3 by # of a non H atom) = %0.6lf Angstrom^3\n\n"
		" 18 ang^3 * # of atoms = %0.6lf Angstrom^3\n"
		),
		rmax,4*M_PI*rmax*rmax*rmax/3.0,
		volNonH*nAtomsNonH,
		volNonH*nAtoms
		);
		Message(t,_("Info"),TRUE);
		g_free(t);
		return;
	}
}
/********************************************************************************/
void compute_volume_cell()
{
	gint i;
	gdouble X;
	gdouble Y;
	gdouble Z;
	gdouble TV[3][3];
	gdouble cross[3];
	gdouble vol = 0;
	gint nTV = 0;

	if(NcentersXYZ<1) return;
	for(i=0;i<NcentersXYZ;i++) 
		if(!strcmp(GeomXYZ[i].Symb, "TV") || !strcmp(GeomXYZ[i].Symb, "Tv")) 
		{
        	 	if(!test(GeomXYZ[i].X)) X = get_value_variableXYZ(GeomXYZ[i].X); else X = atof(GeomXYZ[i].X);
			if(!test(GeomXYZ[i].Y)) Y = get_value_variableXYZ(GeomXYZ[i].Y); else Y = atof(GeomXYZ[i].Y);
			if(!test(GeomXYZ[i].Z)) Z = get_value_variableXYZ(GeomXYZ[i].Z); else Z = atof(GeomXYZ[i].Z);
			if(Units==0) { X *= BOHR_TO_ANG; Y *= BOHR_TO_ANG; Z *= BOHR_TO_ANG; }
			TV[nTV][0] = X;
			TV[nTV][1] = Y;
			TV[nTV][2] = Z;
			nTV++;
		}
	if(nTV<3)
	{
		gchar* t = g_strdup_printf(_("Sorry,\nI cannot compute volume with only %d Tv vectors"),nTV);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return;
	}

	for(i=0;i<3;i++) cross[i] = TV[0][(i+1)%3]*TV[1][(i+2)%3]-TV[0][(i+2)%3]*TV[1][(i+1)%3];
	for(i=0;i<3;i++) vol += cross[i]*TV[2][i];
	{
		gchar* t = g_strdup_printf(_("Volume = %0.8lf Angstrom^3"),fabs(vol));
		Message(t,_("Info"),TRUE);
		g_free(t);
		return;
	}
	
}
/********************************************************************************/
Crystal* get_crystal_from_GeomXYZ()
{ 
	Crystal* crystal = g_malloc(sizeof(Crystal));
	gint i;
	gint nAtoms = NcentersXYZ;

	initCrystal(crystal);

	if(nAtoms<1) return crystal;

	/* fprintf(stderr,"Units=%d\n",Units);*/
	for(i=0;i<nAtoms;i++)
	{
		gdouble X,Y,Z;
		CrystalloAtom* a = g_malloc(sizeof(CrystalloAtom));

        	if(!test(GeomXYZ[i].X)) X = get_value_variableXYZ(GeomXYZ[i].X); else X = atof(GeomXYZ[i].X);
		if(!test(GeomXYZ[i].Y)) Y = get_value_variableXYZ(GeomXYZ[i].Y); else Y = atof(GeomXYZ[i].Y);
		if(!test(GeomXYZ[i].Z)) Z = get_value_variableXYZ(GeomXYZ[i].Z); else Z = atof(GeomXYZ[i].Z);
		if(Units==0) { X *= (BOHR_TO_ANG); Y *= (BOHR_TO_ANG); Z *= (BOHR_TO_ANG); }
		a->C[0] = X;
		a->C[1] = Y;
		a->C[2] = Z;


		a->charge = atof(GeomXYZ[i].Charge);
		a->residueNumber = GeomXYZ[i].ResidueNumber;

		sprintf(a->symbol,"%s",GeomXYZ[i].Symb);
		sprintf(a->mmType,"%s",GeomXYZ[i].mmType);
		sprintf(a->pdbType,"%s",GeomXYZ[i].pdbType);
		sprintf(a->residueName,"%s",GeomXYZ[i].Residue);

		crystal->atoms=g_list_append(crystal->atoms, (gpointer) a);
	}
	return crystal;
}
/********************************************************************************/
void save_cif_file_no_add_list(const gchar* FileName, gboolean allAtoms)
{
	FILE *file;
	Crystal* crystal = NULL;
	gint nAtoms = NcentersXYZ;
	gchar* tmp = NULL;
	gdouble symprec = 1e-4;
	gint i;
	gint nTv=0;

	if(nAtoms<1) return;
	for(i=0;i<nAtoms;i++) if(!strcmp(GeomXYZ[i].Symb, "TV") || !strcmp(GeomXYZ[i].Symb, "Tv")) nTv++;
	if(nTv<3)
	{
		gchar* t = g_strdup_printf(_("Sorry,\nI cannot create the cif file, only %d Tv vectors are available"),nTv);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	crystal = get_crystal_from_GeomXYZ();
	if(!crystal) return;

	file = FOpen(FileName, "w");
	if(file == NULL)
	{
		gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),FileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	tmp = crystalloGetCIF(crystal, symprec, !allAtoms);
	if(tmp)
	{
		fprintf(file,"%s",tmp);
		g_free(tmp);
	}
	fclose(file);
	freeCrystal(crystal);
	g_free(crystal);
}
/********************************************************************************/
void save_poscar_direct_file_no_add_list(const gchar* FileName)
{
	gint i;
	gint j;
	FILE *fd;
	gdouble X;
	gdouble Y;
	gdouble Z;
	gchar *temp  = NULL;
	gint nTV = 0;
	gint nTypes = 0;
	gint* types = NULL;
	gboolean ok;
	gchar** listTypes = NULL;
	gint* nListTypes = NULL;
	Crystal* crystal = NULL;
	gint nAtoms = NcentersXYZ;

	if(nAtoms<1) return;
	for(i=0;i<nAtoms;i++) if(!strcmp(GeomXYZ[i].Symb, "TV") || !strcmp(GeomXYZ[i].Symb, "Tv")) nTV++;
	if(nTV<3)
	{
		gchar* t = g_strdup_printf(_("Sorry,\nI cannot create the poscar file, only %d Tv vectors are available"),nTV);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	crystal = get_crystal_from_GeomXYZ();
	if(!crystal) return;
	crystalloCartnToFract(crystal->atoms);

	//printf("nTv = %d\n",nTV);
	types = g_malloc(nAtoms*sizeof(gint));
	for(i=0;i<nAtoms;i++) types[i] = -1;

	nTypes = 0;
	for(i=0;i<nAtoms;i++)
	{
		if(!strcmp(GeomXYZ[i].Symb, "TV") || !strcmp(GeomXYZ[i].Symb, "Tv")) continue;
		ok = TRUE;
		for(j=0;j<i;j++)
		{
			if(!strcmp(GeomXYZ[i].Symb,GeomXYZ[j].Symb))
			{
				types[i]= types[j];
				ok = FALSE;
				break;
			}
		}
		if(ok)
		{
			types[i]= nTypes;
			nTypes++;
		}
	}
	if(nTypes<1) return;
	//printf("nTypes = %d\n",nTypes);
	listTypes = g_malloc(nTypes*sizeof(gchar*));
	nListTypes = g_malloc(nTypes*sizeof(gint));
	for(i=0;i<nTypes;i++) listTypes[i] = NULL;
	for(i=0;i<nTypes;i++) nListTypes[i] = 0;
	for(i=0;i<nAtoms;i++)
	{
		if(types[i]==-1) continue;
		nListTypes[types[i]]++;
		if(!listTypes[types[i]]) listTypes[types[i]] = g_strdup(GeomXYZ[i].Symb);
	}

	temp = get_suffix_name_file(FileName);
	FileName = g_strdup_printf("%s",temp);
	g_free(temp);
	fd = FOpen(FileName, "w");
	if(fd == NULL)
	{
		gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),FileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	for(i=0;i<nTypes;i++) fprintf(fd,"%s%d ",listTypes[i],nListTypes[i]);
	fprintf(fd," : POSCAR (Direct) generated by Gabedit\n");
	fprintf(fd,"%6s %0.10lf\n"," ",1.0);
	for(i=0;i<NcentersXYZ;i++)
	{
		if(types[i]!=-1) continue;
        	if(!test(GeomXYZ[i].X)) X = get_value_variableXYZ(GeomXYZ[i].X); else X = atof(GeomXYZ[i].X);
		if(!test(GeomXYZ[i].Y)) Y = get_value_variableXYZ(GeomXYZ[i].Y); else Y = atof(GeomXYZ[i].Y);
		if(!test(GeomXYZ[i].Z)) Z = get_value_variableXYZ(GeomXYZ[i].Z); else Z = atof(GeomXYZ[i].Z);
		if(Units==0) { X *= BOHR_TO_ANG; Y *= BOHR_TO_ANG; Z *= BOHR_TO_ANG; }
		fprintf(fd,"     %20.10f  %20.10f  %20.10f\n",X,Y,Z);
	}
	fprintf(fd,"%6s"," ");
	for(i=0;i<nTypes;i++) fprintf(fd,"%6s ",listTypes[i]);
	fprintf(fd,"\n");
	fprintf(fd,"%6s"," ");
	for(i=0;i<nTypes;i++) fprintf(fd,"%6d ",nListTypes[i]);
	fprintf(fd,"\n");
	fprintf(fd,"Direct\n");
	for(j=0;j<nTypes;j++) 
	{
	      	GList *l = NULL;
		i=-1;
        	for(l = g_list_first(crystal->atoms); l != NULL; l = l->next)
        	{
			i++;
                	CrystalloAtom* crystalloAtom = (CrystalloAtom*)l->data;
                	if(strstr(crystalloAtom->symbol,"Tv") || strstr(crystalloAtom->symbol,"TV")) continue;
			if(types[i]==-1) continue;
			if(strcmp(crystalloAtom->symbol,listTypes[j]))continue;
			fprintf(fd,"%6s  %20.10f  %20.10f  %20.10f %s\n"," ",crystalloAtom->C[0], crystalloAtom->C[1],crystalloAtom->C[2], crystalloAtom->symbol);
        	}
	}
		
	fclose(fd);
	for(i=0;i<nTypes;i++) if(listTypes[i]) g_free(listTypes[i]);
	if(listTypes) g_free(listTypes);
	if(nListTypes) g_free(nListTypes);
	if(types) g_free(types);
	freeCrystal(crystal);
	g_free(crystal);
}
/********************************************************************************/
void save_poscar_file_no_add_list(const gchar* FileName)
{
	gint i;
	gint j;
	FILE *fd;
	gdouble X;
	gdouble Y;
	gdouble Z;
	gchar *temp  = NULL;
	gint nTV = 0;
	gint nTypes = 0;
	gint* types = NULL;
	gboolean ok;
	gchar** listTypes = NULL;
	gint* nListTypes = NULL;

	if(NcentersXYZ<1) return;
	for(i=0;i<NcentersXYZ;i++) if(!strcmp(GeomXYZ[i].Symb, "TV") || !strcmp(GeomXYZ[i].Symb, "Tv")) nTV++;
	if(nTV<3)
	{
		gchar* t = g_strdup_printf(_("Sorry,\nI cannot create the poscar file, only %d Tv vectors are available"),nTV);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	//printf("nTv = %d\n",nTV);
	types = g_malloc(NcentersXYZ*sizeof(gint));
	for(i=0;i<NcentersXYZ;i++) types[i] = -1;

	nTypes = 0;
	for(i=0;i<NcentersXYZ;i++)
	{
		if(!strcmp(GeomXYZ[i].Symb, "TV") || !strcmp(GeomXYZ[i].Symb, "Tv")) continue;
		ok = TRUE;
		for(j=0;j<i;j++)
		{
			if(!strcmp(GeomXYZ[i].Symb,GeomXYZ[j].Symb))
			{
				types[i]= types[j];
				ok = FALSE;
				break;
			}
		}
		if(ok)
		{
			types[i]= nTypes;
			nTypes++;
		}
	}
	if(nTypes<1) return;
	//printf("nTypes = %d\n",nTypes);
	listTypes = g_malloc(nTypes*sizeof(gchar*));
	nListTypes = g_malloc(nTypes*sizeof(gint));
	for(i=0;i<nTypes;i++) listTypes[i] = NULL;
	for(i=0;i<nTypes;i++) nListTypes[i] = 0;
	for(i=0;i<NcentersXYZ;i++)
	{
		if(types[i]==-1) continue;
		nListTypes[types[i]]++;
		if(!listTypes[types[i]]) listTypes[types[i]] = g_strdup(GeomXYZ[i].Symb);
	}

	temp = get_suffix_name_file(FileName);
	FileName = g_strdup_printf("%s",temp);
	g_free(temp);
	fd = FOpen(FileName, "w");
	if(fd == NULL)
	{
		gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),FileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	for(i=0;i<nTypes;i++) fprintf(fd,"%s%d ",listTypes[i],nListTypes[i]);
	fprintf(fd," : POSCAR generated by Gabedit\n");
	fprintf(fd,"%6s %0.10lf\n"," ",1.0);
	for(i=0;i<NcentersXYZ;i++)
	{
		if(types[i]!=-1) continue;
        	 if(!test(GeomXYZ[i].X)) X = get_value_variableXYZ(GeomXYZ[i].X); else X = atof(GeomXYZ[i].X);
		if(!test(GeomXYZ[i].Y)) Y = get_value_variableXYZ(GeomXYZ[i].Y); else Y = atof(GeomXYZ[i].Y);
		if(!test(GeomXYZ[i].Z)) Z = get_value_variableXYZ(GeomXYZ[i].Z); else Z = atof(GeomXYZ[i].Z);
		if(Units==0) { X *= BOHR_TO_ANG; Y *= BOHR_TO_ANG; Z *= BOHR_TO_ANG; }
		fprintf(fd,"     %20.10f  %20.10f  %20.10f\n",X,Y,Z);
	}
	fprintf(fd,"%6s"," ");
	for(i=0;i<nTypes;i++) fprintf(fd,"%6s ",listTypes[i]);
	fprintf(fd,"\n");
	fprintf(fd,"%6s"," ");
	for(i=0;i<nTypes;i++) fprintf(fd,"%6d ",nListTypes[i]);
	fprintf(fd,"\n");
	fprintf(fd,"Cartesian\n");
	for(j=0;j<nTypes;j++) 
	{
		for(i=0;i<NcentersXYZ;i++)
		{
			if(types[i]==-1) continue;
			if(strcmp(GeomXYZ[i].Symb,listTypes[j]))continue;
        	 	if(!test(GeomXYZ[i].X)) X = get_value_variableXYZ(GeomXYZ[i].X); else X = atof(GeomXYZ[i].X);
			if(!test(GeomXYZ[i].Y)) Y = get_value_variableXYZ(GeomXYZ[i].Y); else Y = atof(GeomXYZ[i].Y);
			if(!test(GeomXYZ[i].Z)) Z = get_value_variableXYZ(GeomXYZ[i].Z); else Z = atof(GeomXYZ[i].Z);
			if(Units==0) { X *= BOHR_TO_ANG; Y *= BOHR_TO_ANG; Z *= BOHR_TO_ANG; }
			fprintf(fd,"%6s  %20.10f  %20.10f  %20.10f\n"," ",X,Y,Z);
		}
	}
		
/*
	for(i=0;i<NcentersXYZ;i++)
	{
		if(types[i]==-1) continue;
        	 if(!test(GeomXYZ[i].X)) X = get_value_variableXYZ(GeomXYZ[i].X); else X = atof(GeomXYZ[i].X);
		if(!test(GeomXYZ[i].Y)) Y = get_value_variableXYZ(GeomXYZ[i].Y); else Y = atof(GeomXYZ[i].Y);
		if(!test(GeomXYZ[i].Z)) Z = get_value_variableXYZ(GeomXYZ[i].Z); else Z = atof(GeomXYZ[i].Z);
		if(Units==0) { X *= BOHR_TO_ANG; Y *= BOHR_TO_ANG; Z *= BOHR_TO_ANG; }
		fprintf(fd,"%6s  %20.10f  %20.10f  %20.10f\n"," ",X,Y,Z);
	}
*/
	fclose(fd);
	for(i=0;i<nTypes;i++) if(listTypes[i]) g_free(listTypes[i]);
	if(listTypes) g_free(listTypes);
	if(nListTypes) g_free(nListTypes);
	if(types) g_free(types);
}
/********************************************************************************/
void save_xyz_file_no_add_list(const gchar* FileName)
{
	guint i;
	FILE *fd;
	gdouble X;
	gdouble Y;
	gdouble Z;
	gchar *temp  = NULL;

	temp = get_suffix_name_file(FileName);
	FileName = g_strdup_printf("%s.xyz",temp);
	g_free(temp);
	fd = FOpen(FileName, "w");
	if(fd == NULL)
	{
		gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),FileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	fprintf(fd,"%d\n",NcentersXYZ);
	fprintf(fd,"XYZ file generated by gabedit : coordinates in Angstrom\n");
	for(i=0;i<NcentersXYZ;i++)
	{
        	 if(!test(GeomXYZ[i].X)) X = get_value_variableXYZ(GeomXYZ[i].X);
		else X = atof(GeomXYZ[i].X);

		if(!test(GeomXYZ[i].Y)) Y = get_value_variableXYZ(GeomXYZ[i].Y);
		else Y = atof(GeomXYZ[i].Y);

		if(!test(GeomXYZ[i].Z)) Z = get_value_variableXYZ(GeomXYZ[i].Z);
		else Z = atof(GeomXYZ[i].Z);

		if(Units==0)
		{
              		X *= BOHR_TO_ANG;
              		Y *= BOHR_TO_ANG;
              		Z *= BOHR_TO_ANG;
		}
		fprintf(fd,"%s  %20.10f  %20.10f  %20.10f\n",GeomXYZ[i].Symb,X,Y,Z);
	}
	fclose(fd);
}
/********************************************************************************/
void save_xyz_file(const gchar* FileName)
{
 guint i;
 FILE *fd;
 gdouble X;
 gdouble Y;
 gdouble Z;
 gchar *projectname = NULL;
 gchar *datafile = NULL;
 gchar *localdir = NULL;
 gchar *remotehost  = NULL;
 gchar *remoteuser  = NULL;
 gchar *remotepass  = NULL;
 gchar *remotedir  = NULL;
 gchar *temp  = NULL;

 temp = get_suffix_name_file(FileName);
 FileName = g_strdup_printf("%s.xyz",temp);
 g_free(temp);
 fd = FOpen(FileName, "w");
 if(fd == NULL)
 {
	gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),FileName);
	Message(t,_("Error"),TRUE);
	g_free(t);
	return;
 }
 fprintf(fd,"%d\n",NcentersXYZ);
 fprintf(fd,"XYZ file generated by gabedit : coordinates in Angstrom\n");
 for(i=0;i<NcentersXYZ;i++)
 {
         if(!test(GeomXYZ[i].X))
                 X = get_value_variableXYZ(GeomXYZ[i].X);
         else
                 X = atof(GeomXYZ[i].X);
         if(!test(GeomXYZ[i].Y))
                 Y = get_value_variableXYZ(GeomXYZ[i].Y);
         else
                 Y = atof(GeomXYZ[i].Y);
         if(!test(GeomXYZ[i].Z))
                 Z = get_value_variableXYZ(GeomXYZ[i].Z);
         else
                 Z = atof(GeomXYZ[i].Z);
         if(Units==0)
         {
              X *= BOHR_TO_ANG;
              Y *= BOHR_TO_ANG;
              Z *= BOHR_TO_ANG;
         }

  	g_fprintf(fd,"%s  %20.10f  %20.10f  %20.10f\n",GeomXYZ[i].Symb,X,Y,Z);
   }
  fprintf(fd," \n");

  fclose(fd);
  datafile = get_name_file(FileName);
  temp = get_suffix_name_file(FileName);
  projectname = get_name_file(temp);
  localdir = get_name_dir(temp);
  if(lastdirectory)
	g_free(lastdirectory);
  lastdirectory = g_strdup(localdir);
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_XYZ],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_XYZ, NULL, defaultNetWorkProtocol);
  g_free(temp);
  g_free(datafile);
  g_free(projectname);
  g_free(localdir);
}
/************************************************************************************/
void save_geometry_cif_allatoms_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	save_cif_file_no_add_list(FileName,TRUE);
}
/************************************************************************************/
void save_geometry_cif_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	save_cif_file_no_add_list(FileName,FALSE);
}
/************************************************************************************/
void save_geometry_poscar_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	save_poscar_file_no_add_list(FileName);
}
/************************************************************************************/
void save_geometry_poscar_direct_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	save_poscar_direct_file_no_add_list(FileName);
}
/************************************************************************************/
void save_geometry_xyz_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	 save_xyz_file(FileName);
}

/********************************************************************************/
void save_xyz_file_entry(GtkWidget* entry)
{
	G_CONST_RETURN gchar *FileName;

	FileName = gtk_entry_get_text(GTK_ENTRY(entry));
	if ((!FileName) || (strcmp(FileName,"") == 0))
		return ;
	 save_xyz_file(FileName);
}
/********************************************************************************/
void save_mol_file(const gchar* FileName)
{
 guint i;
 gint j;
 gint n;
 FILE *fd;
 gdouble X;
 gdouble Y;
 gdouble Z;
 gchar *projectname = NULL;
 gchar *datafile = NULL;
 gchar *localdir = NULL;
/*
 gchar *remotehost  = NULL;
 gchar *remoteuser  = NULL;
 gchar *remotepass  = NULL;
 gchar *remotedir  = NULL;
*/
 gchar *temp  = NULL;

 temp = get_suffix_name_file(FileName);
 FileName = g_strdup_printf("%s.mol",temp);
 g_free(temp);
 fd = FOpen(FileName, "w");
 if(fd == NULL)
 {
	gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),FileName);
	Message(t,_("Error"),TRUE);
	g_free(t);
	return;
 }
 n = 0;
 for(i=0;i<NcentersXYZ;i++)
        if(GeomXYZ[i].typeConnections)
 	for(j=i+1;j<NcentersXYZ;j++)
        	if(GeomXYZ[i].typeConnections[j]) n++;
 fprintf(fd," Molecule\n");
  fprintf(fd," GENERATED BY GABEDIT %d.%d.%d\n",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
  temp = get_time_str();
  if(temp) fprintf(fd," Time = %s",temp);
  else fprintf(fd,"\n");
	
 fprintf(fd,"%3d%3d  0  0  0  0999 V2000\n",NcentersXYZ,n);
 for(i=0;i<NcentersXYZ;i++)
 {
         if(!test(GeomXYZ[i].X))
                 X = get_value_variableXYZ(GeomXYZ[i].X);
         else
                 X = atof(GeomXYZ[i].X);
         if(!test(GeomXYZ[i].Y))
                 Y = get_value_variableXYZ(GeomXYZ[i].Y);
         else
                 Y = atof(GeomXYZ[i].Y);
         if(!test(GeomXYZ[i].Z))
                 Z = get_value_variableXYZ(GeomXYZ[i].Z);
         else
                 Z = atof(GeomXYZ[i].Z);
         if(Units==0)
         {
              X *= BOHR_TO_ANG;
              Y *= BOHR_TO_ANG;
              Z *= BOHR_TO_ANG;
         }
  	fprintf(fd,"%10.4f%10.4f%10.4f %-3s%2d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d\n",
		X,Y,Z,GeomXYZ[i].Symb,0,0,0,0,0,0,0,0,0,0,0,0);
   }
  n = 0;
 for(i=0;i<NcentersXYZ;i++)
 {
        if(GeomXYZ[i].typeConnections)
 	for(j=i+1;j<NcentersXYZ;j++)
 	{
        	if(GeomXYZ[i].typeConnections[j])
		{
  			/*fprintf(fd," %5d  %5d %5d 0     0  0\n", i+1, j+1, GeomXYZ[i].typeConnections[j]);*/
  			fprintf(fd,"%3d%3d%3d  0  0  0  0\n", i+1, j+1, GeomXYZ[i].typeConnections[j]);
			n++;
		}

 	}
 }
  fprintf(fd,"M  END\n");
  fclose(fd);
  datafile = get_name_file(FileName);
  temp = get_suffix_name_file(FileName);
  projectname = get_name_file(temp);
  localdir = get_name_dir(temp);
  if(lastdirectory)
	g_free(lastdirectory);
  lastdirectory = g_strdup(localdir);
  /* CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_XYZ],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_XYZ,NULL, defaultNetWorkProtocol);*/
  g_free(temp);
  g_free(datafile);
  g_free(projectname);
  g_free(localdir);
}
/************************************************************************************/
void save_geometry_mol_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	 save_mol_file(FileName);
}
/********************************************************************************/
void save_mol2_file(const gchar* FileName)
{
 guint i;
 gint j;
 gint n;
 FILE *fd;
 gdouble X;
 gdouble Y;
 gdouble Z;
 gchar *projectname = NULL;
 gchar *datafile = NULL;
 gchar *localdir = NULL;
 gchar *remotehost  = NULL;
 gchar *remoteuser  = NULL;
 gchar *remotepass  = NULL;
 gchar *remotedir  = NULL;
 gchar *temp  = NULL;

 temp = get_suffix_name_file(FileName);
 FileName = g_strdup_printf("%s.mol2",temp);
 g_free(temp);
 fd = FOpen(FileName, "w");
 if(fd == NULL)
 {
	gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),FileName);
	Message(t,_("Error"),TRUE);
	g_free(t);
	return;
 }
 n = 0;
 for(i=0;i<NcentersXYZ;i++)
        if(GeomXYZ[i].typeConnections)
 	for(j=i+1;j<NcentersXYZ;j++)
        	if(GeomXYZ[i].typeConnections[j]) n++;
 fprintf(fd,"@<TRIPOS>MOLECULE\n");
 fprintf(fd,"MOL2  : Made in Gabedit. mol2 file\n");
 fprintf(fd," %10d %10d %10d\n",NcentersXYZ,n,1);
 fprintf(fd," SMALL\n");
 fprintf(fd," NO_CHARGES\n");
 fprintf(fd," ****\n");
 fprintf(fd," ****\n");
 fprintf(fd,"@<TRIPOS>ATOM\n");
 for(i=0;i<NcentersXYZ;i++)
 {
         if(!test(GeomXYZ[i].X))
                 X = get_value_variableXYZ(GeomXYZ[i].X);
         else
                 X = atof(GeomXYZ[i].X);
         if(!test(GeomXYZ[i].Y))
                 Y = get_value_variableXYZ(GeomXYZ[i].Y);
         else
                 Y = atof(GeomXYZ[i].Y);
         if(!test(GeomXYZ[i].Z))
                 Z = get_value_variableXYZ(GeomXYZ[i].Z);
         else
                 Z = atof(GeomXYZ[i].Z);
         if(Units==0)
         {
              X *= BOHR_TO_ANG;
              Y *= BOHR_TO_ANG;
              Z *= BOHR_TO_ANG;
         }

  	fprintf(fd,"%7d%1s%-6s%12.4f%10.4f%10.4f%1s%-5s%4d%1s %-8s%10.4f\n",
		i+1,"",GeomXYZ[i].Symb,X,Y,Z,"",GeomXYZ[i].mmType,GeomXYZ[i].ResidueNumber+1,"",GeomXYZ[i].Residue,atof(GeomXYZ[i].Charge));
   }
  fprintf(fd,"@<TRIPOS>BOND\n");
  n = 0;
 for(i=0;i<NcentersXYZ;i++)
 {
        if(GeomXYZ[i].typeConnections)
 	for(j=i+1;j<NcentersXYZ;j++)
 	{
        	if(GeomXYZ[i].typeConnections[j])
		{
  			fprintf(fd,"%6d%6d%6d%3s%2d\n",n+1, i+1, j+1, "",GeomXYZ[i].typeConnections[j]);
			n++;
		}

 	}
 }
  fprintf(fd,"@<TRIPOS>SUBSTRUCTURE\n");
  fprintf(fd,"    1 RES1       1\n");

  fclose(fd);
  datafile = get_name_file(FileName);
  temp = get_suffix_name_file(FileName);
  projectname = get_name_file(temp);
  localdir = get_name_dir(temp);
  if(lastdirectory)
	g_free(lastdirectory);
  lastdirectory = g_strdup(localdir);
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_MOL2],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_MOL2,NULL, defaultNetWorkProtocol);
  g_free(temp);
  g_free(datafile);
  g_free(projectname);
  g_free(localdir);
}
/************************************************************************************/
void save_geometry_mol2_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	 save_mol2_file(FileName);
}
/********************************************************************************/
void save_mol2_file_entry(GtkWidget* entry)
{
	G_CONST_RETURN gchar *FileName;

 	FileName = gtk_entry_get_text(GTK_ENTRY(entry));
 	if ((!FileName) || (strcmp(FileName,"") == 0))
		return;

	save_mol2_file(FileName);
}
/********************************************************************************/
static gint testav(gchar *t)
{
  guint i;
	if(!test(t) ){
	for(i=0;i<NVariablesXYZ;i++)
  		if (!strcmp(t,VariablesXYZ[i].Name) || (  t[0] == '-' && !strcmp(t+1,VariablesXYZ[i].Name)))
         		return i;
        
         return -2;
        }
        else
         return -1;
}
/********************************************************************************/
static void ChangeVariablesXYZUseds()
{
	guint i;
	guint j;
	gboolean k;
    
        for (i=0;i<NVariablesXYZ;i++)
        {
           k=FALSE;
           for (j=0;j<NcentersXYZ;j++)
           {
		if (!strcmp(VariablesXYZ[i].Name, GeomXYZ[j].X) || (GeomXYZ[j].X[0] == '-' && !strcmp(VariablesXYZ[i].Name, GeomXYZ[j].X+1)))
                {
                	k=TRUE;
             		break;
                }
		if (!strcmp(VariablesXYZ[i].Name, GeomXYZ[j].Y) || (GeomXYZ[j].Y[0] == '-' && !strcmp(VariablesXYZ[i].Name, GeomXYZ[j].Y+1)))
                {
                	k=TRUE;
             		break;
                }
		if (!strcmp(VariablesXYZ[i].Name, GeomXYZ[j].Z) || (GeomXYZ[j].Z[0] == '-' && !strcmp(VariablesXYZ[i].Name, GeomXYZ[j].Z+1)))
                {
                	k=TRUE;
             		break;
		}
           }
        VariablesXYZ[i].Used=k;
        }
}
/********************************************************************************/
void AllocationVariableXYZ()
{
   if(VariablesXYZ != NULL)
    VariablesXYZ = g_realloc(VariablesXYZ,NVariablesXYZ*sizeof(VariablesXYZDef));
   else
    VariablesXYZ = g_malloc(NVariablesXYZ*sizeof(VariablesXYZDef));
}
/********************************************************************************/
void show_geom_in_list(guint i)
{
	gchar *texts[NUMBER_LIST_XYZ];
	texts[E_NUMBER] = g_strdup_printf("%d",i+1);
	texts[E_SYMBOL] = g_strdup(GeomXYZ[i].Symb);
	texts[E_RESIDUE] = g_strdup(GeomXYZ[i].Residue);
	texts[E_MMTYPE] = g_strdup(GeomXYZ[i].mmType);
	texts[E_PDBTYPE] = g_strdup(GeomXYZ[i].pdbType);
	texts[E_X] = g_strdup(GeomXYZ[i].X);
	texts[E_Y] = g_strdup(GeomXYZ[i].Y);
	texts[E_Z] = g_strdup(GeomXYZ[i].Z);
	texts[E_CHARGE] = g_strdup(GeomXYZ[i].Charge);
	texts[E_LAYER] = g_strdup(GeomXYZ[i].Layer);

	removeFromList(list, i);
	insertToList(list, i, texts, NUMBER_LIST_XYZ);
}
/********************************************************************************/
guint  ModifyCoordXYZ(gchar T,guint numC,guint numV )
{
 if(T == 'X')
 {
     if(!strcmp(GeomXYZ[numC].X,VariablesXYZ[numV].Name))
        GeomXYZ[numC].X =g_strdup(VariablesXYZ[numV].Value);
 return 1;
 }
 if(T == 'Y')
 {
     if(!strcmp(GeomXYZ[numC].Y,VariablesXYZ[numV].Name))
        GeomXYZ[numC].Y =g_strdup(VariablesXYZ[numV].Value);
 return 1;
 }
 if(T == 'Z')
 {
     if(!strcmp(GeomXYZ[numC].Z,VariablesXYZ[numV].Name))
        GeomXYZ[numC].Z =g_strdup(VariablesXYZ[numV].Value);
 return 1;
 }
 return 0;
}
/********************************************************************************/
void OneVariableToConstXYZ(guint num)
{
   guint i;
   guint k;
   if(!VariablesXYZ[num].Used)
       return;
   for(i=0;i<NcentersXYZ;i++)
   {
     k = 0;
     k += ModifyCoordXYZ('X',i,num);
     k += ModifyCoordXYZ('Y',i,num);
     k += ModifyCoordXYZ('Z',i,num);
     if(k>0) show_geom_in_list(i);
   }
   for(i=num;i<NVariablesXYZ-1;i++)
   {
	 VariablesXYZ[i].Name = g_strdup(VariablesXYZ[i+1].Name);
	 VariablesXYZ[i].Value = g_strdup(VariablesXYZ[i+1].Value);
	 VariablesXYZ[i].Used = VariablesXYZ[i+1].Used;
   }
   NVariablesXYZ--;
   if(NVariablesXYZ>0)
   	AllocationVariableXYZ();
   else
	freeVariablesXYZ();

/*
   ChangeVariablesXYZUseds();
*/

   removeFromList(listv, num);
}
/********************************************************************************/
void trans_allVariables_to_Constants()
{
   guint numV;
   guint numC;
   guint k;
   guint i;
   guint NRem=0;
   gboolean *Rem;
   VariablesXYZDef *VXYZ;
   Rem = g_malloc(NVariablesXYZ*sizeof(gboolean));
   VXYZ = g_malloc(NVariablesXYZ*sizeof(VariablesXYZDef));
   
   for(numV=0;numV<NVariablesXYZ;numV++)
   {
    Rem[numV] = FALSE;
    if(VariablesXYZ[numV].Used)
    {
      for(numC=0;numC<NcentersXYZ;numC++)
      {
        k = 0;
        k += ModifyCoordXYZ('X',numC,numV);
        k += ModifyCoordXYZ('Y',numC,numV);
        k += ModifyCoordXYZ('Z',numC,numV);
      }
      NRem++;
      Rem[numV] = TRUE;
      VariablesXYZ[numV].Used = FALSE;
    } 
   }
   for(i=0;i<NVariablesXYZ;i++)
   {
    VXYZ[i].Name = g_strdup(VariablesXYZ[i].Name);
    VXYZ[i].Value = g_strdup(VariablesXYZ[i].Value);
    VXYZ[i].Used = VariablesXYZ[i].Used;
   }
   k=NVariablesXYZ;
   freeVariablesXYZ();
   NVariablesXYZ = k - NRem;
   VariablesXYZ = g_malloc(NVariablesXYZ*sizeof(VariablesXYZDef));
   numV =-1; 
   for(i=0;i<k;i++)
   {
    if(!Rem[i]) 
    {
      numV++;
      VariablesXYZ[numV].Name = g_strdup(VXYZ[i].Name);
      VariablesXYZ[numV].Value = g_strdup(VXYZ[i].Value);
      VariablesXYZ[numV].Used = VXYZ[i].Used;
    }
   }

   clearList(listv);
   append_VariablesXYZ_in_list();

   clearList(list);
   append_list();

   for(i=0;i<NVariablesXYZ;i++)
   {
     g_free(VXYZ[i].Name);
     g_free(VXYZ[i].Value);
   }
   g_free(VXYZ);
   g_free(Rem);
   if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
void AddVariableXYZ(gchar *NameV,gchar *ValueV, gboolean rafresh)
{
   gchar *texts[2];
   NVariablesXYZ++;
   AllocationVariableXYZ();
   VariablesXYZ[NVariablesXYZ-1].Name  = g_strdup(NameV);
   VariablesXYZ[NVariablesXYZ-1].Value = g_strdup(ValueV);
   texts[0] = g_strdup(NameV);
   texts[1] = g_strdup(ValueV);
   if(rafresh) appendToList(listv, texts, 2);
}
/********************************************************************************/
static gboolean reset_variableXYZ_value(gchar *NameV,gchar *ValueV, gboolean rafresh)
{
	gint i=testav(NameV);
	if(i>=0)
	{
   		if(VariablesXYZ[i].Value) g_free(VariablesXYZ[i].Value);
		VariablesXYZ[i].Value = g_strdup(ValueV);
		if(rafresh) 
		{
			gchar* texts[2] = { VariablesXYZ[i].Name, VariablesXYZ[i].Value};
  			removeFromList(listv, i);
  			insertToList(listv, i, texts, 2);
		}
		return TRUE;
	}
	return FALSE;
}
/********************************************************************************/
void trans_coordXYZ(gchar T,guint i, gboolean rafresh)
{
 gdouble V;
 gchar *NameV;
 gchar *ValueV;
 gint j;
 gint k;
 gboolean OK;

 V = atof(GeomXYZ[i].X);
 if( T == 'Y' )
    V = atof(GeomXYZ[i].Y);
 if( T == 'Z' )
    V = atof(GeomXYZ[i].Z);
 NameV = g_strdup_printf("%c%s%d",T,GeomXYZ[i].Symb,i+1);
 k = i+2;
 OK = FALSE;
 while(!OK)
 {
	OK = TRUE;
 	for(j=0;j<(gint)NVariablesXYZ;j++)
 	{
		 if(strcmp(NameV,VariablesXYZ[j].Name )==0)
	 	{
 			NameV = g_strdup_printf("%c%s%d",T,GeomXYZ[i].Symb,k);
			k++;
			OK = FALSE;
			break;
	 	}
 	}
 }
 ValueV = g_strdup_printf("%f",V);
 if(!reset_variableXYZ_value(NameV,ValueV,  rafresh))
 	AddVariableXYZ(NameV,ValueV,rafresh);
 if( T == 'X' )
    GeomXYZ[i].X=g_strdup(NameV);
 if( T == 'Y' )
    GeomXYZ[i].Y=g_strdup(NameV);
 if( T == 'Z' )
    GeomXYZ[i].Z=g_strdup(NameV);
}
/********************************************************************************/
void set_variable_one_atom_in_GeomXYZ(gint i)
{
	if(test(GeomXYZ[i].X)) trans_coordXYZ('X',(guint)i,FALSE);
	if(test(GeomXYZ[i].Y)) trans_coordXYZ('Y',(guint)i,FALSE);
	if(test(GeomXYZ[i].Z)) trans_coordXYZ('Z',(guint)i,FALSE);

	ChangeVariablesXYZUseds();
}
/********************************************************************************/
void trans_OneGeomXYZ_to_variables(guint i)
{
  if(test(GeomXYZ[i].X))
	trans_coordXYZ('X',i,TRUE);
  if(test(GeomXYZ[i].Y))
	trans_coordXYZ('Y',i,TRUE);
  if(test(GeomXYZ[i].Z))
	trans_coordXYZ('Z',i,TRUE);

 show_geom_in_list(i);
 ChangeVariablesXYZUseds();
}
/********************************************************************************/
void trans_allGeomXYZ_to_variables()
{
 guint i;

  if(NcentersXYZ <1 ) return;
  for(i=0;i<NcentersXYZ;i++)
	trans_OneGeomXYZ_to_variables(i);
  if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void DialogueTransInVar()
{
  GtkWidget *Dialogue;
  GtkWidget *Label;
  GtkWidget *Bouton;
  GtkWidget *frame, *vboxframe;


  if (NcentersXYZ <1)
  {
    	MessageGeom(_(" Sorry No Center  !"),_("Error"),TRUE);
    	return ;
  }

  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),_("Tansform all constants in Variables"));
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));
  gtk_window_set_modal (GTK_WINDOW (Dialogue), TRUE);

  add_child(WindowGeom,Dialogue,gtk_widget_destroy,_(" Question "));
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  gtk_widget_realize(Dialogue);
  Label = create_label_with_pixmap(Dialogue,_("\nAre you sure to transform\n all constants in variables? \n"),_(" Question "));

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  gtk_box_pack_start(GTK_BOX(vboxframe), Label,TRUE,TRUE,0);

  Bouton = create_button(Dialogue,_("No"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
 
  Bouton = create_button(Dialogue,_("Yes"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)trans_allGeomXYZ_to_variables, NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
    

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
static void DialogueTransInConst()
{
  GtkWidget *Dialogue;
  GtkWidget *Label;
  GtkWidget *Bouton;
  GtkWidget *frame, *vboxframe;


  if (NcentersXYZ <1)
  {
    	MessageGeom(_(" Sorry No Center  !"),_("Error"),TRUE);
    	return ;
  }

  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),"Tansform all variables in constants ");
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));
  gtk_window_set_modal (GTK_WINDOW (Dialogue), TRUE);

  add_child(WindowGeom,Dialogue,gtk_widget_destroy,_(" Question "));
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  gtk_widget_realize(Dialogue);
  Label = create_label_with_pixmap(Dialogue,_("\nAre you sure to transform\nall variables in constants? \n"),_(" Question "));
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  gtk_box_pack_start(GTK_BOX(vboxframe), Label,TRUE,TRUE,0);

  Bouton = create_button(Dialogue,_("No"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
 
  Bouton = create_button(Dialogue,_("Yes"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)trans_allVariables_to_Constants, NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
    

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
static void TransXYZConstVar()
{
 gint Nc;
 Nc=LineSelected;
 if(Nc<0)
  {
	MessageGeom(_("Sorry No line selected"),_("Error"),TRUE);
    	return;
  }
  trans_OneGeomXYZ_to_variables((guint)Nc);   
 LineSelected = -1;
 if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void TransXYZConstXVar()
{
	gint i;
	if (NcentersXYZ <1) return ;
	for(i=0;i<NcentersXYZ;i++)
	{
  		if(test(GeomXYZ[i].X)) trans_coordXYZ('X',i,TRUE);
		show_geom_in_list(i);
	}
 	ChangeVariablesXYZUseds();
	if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void TransXYZConstYVar()
{
	gint i;
	if (NcentersXYZ <1) return ;
	for(i=0;i<NcentersXYZ;i++)
	{
  		if(test(GeomXYZ[i].Y)) trans_coordXYZ('Y',i,TRUE);
		show_geom_in_list(i);
	}
 	ChangeVariablesXYZUseds();
	if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void TransXYZConstZVar()
{
	gint i;
	if (NcentersXYZ <1) return ;
	for(i=0;i<NcentersXYZ;i++)
	{
  		if(test(GeomXYZ[i].Z)) trans_coordXYZ('Z',i,TRUE);
		show_geom_in_list(i);
	}
 	ChangeVariablesXYZUseds();
	if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void TransXYZVarConst()
{
 gint Nc;
 Nc=LineSelectedV;
 if(Nc<0)
  {
	MessageGeom(_("Sorry\n No variable selected"),_("Error"),TRUE);
    	return;
  }
 OneVariableToConstXYZ((guint)Nc);
 LineSelectedV = -1;
 if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
static void set_entry_XYZ()
{
   SAtomsProp Atom[3];
   gdouble r;
   gdouble Coord[3];
   gdouble angle1=60.0*DEG_TO_RAD;
   gdouble angle2=60.0*DEG_TO_RAD;
   gchar *strdump;
   G_CONST_RETURN gchar *Atomdump;

      angle1= (gdouble)rand()/RAND_MAX*60*DEG_TO_RAD;
      angle2= (gdouble)rand()/RAND_MAX*60*DEG_TO_RAD;

      if(NcentersXYZ==0)
      {
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_X]),"0.0");
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_Y]),"0.0");
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_Z]),"0.0");
      }
      else
      {
        Atomdump =gtk_entry_get_text(GTK_ENTRY(Entry[E_SYMBOL]));
   	Atom[0] = prop_atom_get(Atomdump);
   	Atom[1] = prop_atom_get(GeomXYZ[NcentersXYZ-1].Symb);
        r = Atom[0].covalentRadii+Atom[1].covalentRadii;
        r *=0.8;
        if(Units==1)
		r*=BOHR_TO_ANG;
        if(test(GeomXYZ[NcentersXYZ-1].X))
        Coord[0] = atof(GeomXYZ[NcentersXYZ-1].X) 
		   + r * sin(angle1)*cos(angle2);
        else
        Coord[0] = get_value_variableXYZ(GeomXYZ[NcentersXYZ-1].X) 
		+ r * sin(angle1)*cos(angle2);

        if(test(GeomXYZ[NcentersXYZ-1].Y))
        Coord[1] = atof(GeomXYZ[NcentersXYZ-1].Y) + r * sin(angle1)*sin(angle2);
        else
        Coord[1] = get_value_variableXYZ(GeomXYZ[NcentersXYZ-1].Y) 
		  + r * sin(angle1)*sin(angle2);

        if(test(GeomXYZ[NcentersXYZ-1].Z))
        Coord[2] = atof(GeomXYZ[NcentersXYZ-1].Z) 
	          + r * cos(angle1);
        else
        Coord[2] = get_value_variableXYZ(GeomXYZ[NcentersXYZ-1].Z) 
	          + r * cos(angle1);

        strdump = g_strdup_printf("%f", Coord[0]);
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_X]),strdump);
        strdump = g_strdup_printf("%f", Coord[1]);
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_Y]),strdump);
        strdump = g_strdup_printf("%f", Coord[2]);
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_Z]),strdump);
        g_free(strdump);
      }

}
/********************************************************************************/
static void SetAtom(GtkWidget *w,gpointer data)
{
  gtk_entry_set_text(GTK_ENTRY(Entry[E_SYMBOL]),(char *)data);
  gtk_entry_set_text(GTK_ENTRY(Entry[E_MMTYPE]),(char *)data);
  gtk_entry_set_text(GTK_ENTRY(Entry[E_PDBTYPE]),(char *)data);
  gtk_editable_set_editable((GtkEditable*) Entry[E_SYMBOL],FALSE);
  gtk_widget_destroy(FenetreTable);
  set_entry_XYZ();
}
/********************************************************************************/
static void SelectAtom(GtkWidget *w,gpointer entry0)
{
	GtkWidget* Table;
	GtkWidget* button;
	GtkWidget* frame;
	guint i;
	guint j;
        GtkStyle *button_style;
        /* GtkStyle *style;*/

	gchar*** Symb = get_periodic_table();

  FenetreTable = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(FenetreTable),TRUE);
  gtk_window_set_title(GTK_WINDOW(FenetreTable),_("Select your atom"));
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
          set_button_style(button_style,button,Symb[j][i]);
          g_signal_connect(G_OBJECT(button), "clicked",
                            (GCallback)SetAtom,(gpointer )Symb[j][i]);
	  gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	  }

  }
 	
  gtk_widget_show_all(FenetreTable);
  
}
/********************************************************************************/
static void DelAtom(GtkWidget *w,gpointer data)
{
    
  	NcentersXYZ--;
	if((gint)NcentersXYZ>-1)
	{
		GeomXYZ=g_realloc(GeomXYZ,NcentersXYZ*sizeof(GeomXYZAtomDef));
  		removeFromList(list, NcentersXYZ);
	}
	else
 		freeGeomXYZ();

 	ChangeVariablesXYZUseds();
  if(GeomDrawingArea != NULL)
       rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();
}
/********************************************************************************/
static void addAtom(GtkWidget *w,gpointer Entree)
{
  gchar *texts[NUMBER_LIST_XYZ];
  gchar *message;
  gint i;
  gint k;

  DestroyDialog = TRUE;
  texts[E_NUMBER] = g_strdup_printf("%d",NcentersXYZ+1);
  texts[E_SYMBOL] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_SYMBOL])));
  texts[E_MMTYPE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_MMTYPE])));
  texts[E_PDBTYPE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_PDBTYPE])));
  texts[E_RESIDUE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_RESIDUE])));
  for (i=E_X;i<=E_Z;i++)
  {
  	texts[i] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[i])));
  	if (!texts[i] || !strcmp(texts[i], ""))
        {
		MessageGeom(_("Sorry a Entry text is void !\n"),_("Error"),TRUE);
  		DestroyDialog = FALSE;
        	return;
        }
	k=testav(texts[i]);
	if(k<-1)
	{	
		message=g_strdup_printf(_("Sorry\n %s \nis not a number \nand is not a variable "),texts[i]);
		MessageGeom(message,_("Error"),TRUE);
  		DestroyDialog = FALSE;
        	return;
	}
  }

  texts[E_CHARGE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_CHARGE])));

  if(iprogram == PROG_IS_MOLPRO)
  	texts[E_LAYER] = g_strdup(" ");
  else
  	texts[E_LAYER] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_LAYER])));

    NcentersXYZ++;
    if(GeomXYZ==NULL)
	  GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
    else
	GeomXYZ=g_realloc(GeomXYZ,NcentersXYZ*sizeof(GeomXYZAtomDef));
  for(i=0; i<NcentersXYZ; i++) GeomXYZ[i].typeConnections = NULL;
  for(i=E_X;i<=E_Z;i++)
  	if(test(texts[i]) && !testpointeE(texts[i]) )
		texts[i]=g_strdup_printf("%s.0",texts[i]);

  GeomXYZ[NcentersXYZ-1].Nentry=NUMBER_LIST_XYZ;
  GeomXYZ[NcentersXYZ-1].Symb  = g_strdup(texts[E_SYMBOL]);
  GeomXYZ[NcentersXYZ-1].mmType  = g_strdup(texts[E_MMTYPE]);
  GeomXYZ[NcentersXYZ-1].pdbType  = g_strdup(texts[E_PDBTYPE]);
  GeomXYZ[NcentersXYZ-1].Residue  = g_strdup(texts[E_RESIDUE]);
  GeomXYZ[NcentersXYZ-1].X     = g_strdup(texts[E_X]);
  GeomXYZ[NcentersXYZ-1].Y     = g_strdup(texts[E_Y]);
  GeomXYZ[NcentersXYZ-1].Z     = g_strdup(texts[E_Z]);
  GeomXYZ[NcentersXYZ-1].Charge = g_strdup(texts[E_CHARGE]);
  GeomXYZ[NcentersXYZ-1].Layer = g_strdup(texts[E_LAYER]);
  GeomXYZ[NcentersXYZ-1].typeConnections = NULL;
  if(NcentersXYZ==1)
	  GeomXYZ[NcentersXYZ-1].ResidueNumber  = 0;
  else
  {
	  gint k;
	  GeomXYZ[NcentersXYZ-1].ResidueNumber = 0; 
	  for(k=0;k<(gint)NcentersXYZ-1;k++)
	  {
		  if(GeomXYZ[NcentersXYZ-1].ResidueNumber<GeomXYZ[k].ResidueNumber)
			  GeomXYZ[NcentersXYZ-1].ResidueNumber = GeomXYZ[k].ResidueNumber;
	  }
	  GeomXYZ[NcentersXYZ-1].ResidueNumber += 1;
  }
  ChangeVariablesXYZUseds();

  appendToList(list, texts, NUMBER_LIST_XYZ);

  if(GeomDrawingArea != NULL)
       rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();

}
/********************************************************************************/
static void EditAtom(GtkWidget *w,gpointer Entree)
{
  gchar *texts[NUMBER_LIST_XYZ];
  gchar *message;
  gint i;
  gint Nc;
  gint k;
  gboolean oldResidue = FALSE;

  DestroyDialog = TRUE;
  Nc = LineSelected;
  if(Nc<0)
	  Nc = LineSelectedOld;
  texts[E_NUMBER] = g_strdup_printf("%d",Nc+1);
  texts[E_SYMBOL] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_SYMBOL])));
  texts[E_RESIDUE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_RESIDUE])));
  texts[E_MMTYPE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_MMTYPE])));
  texts[E_PDBTYPE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_PDBTYPE])));

  for (i=E_X;i<=E_Z;i++)
  {
  	texts[i] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[i])));
  	if (!texts[i] || !strcmp(texts[i], ""))
        {
		message=g_strdup_printf(_("Sorry\n a entry is void "));
		MessageGeom(message,_("Error"),TRUE);
  		DestroyDialog = FALSE;
        	return;
        }
	k=testav(texts[i]);
	if(k<-1)
	{	
		message=g_strdup_printf(_("Sorry\n %s \nis not a number \nand is not a variable "),texts[i]);
		MessageGeom(message,_("Error"),TRUE);
  		DestroyDialog = FALSE;
        return;
	}
  }
  texts[E_CHARGE] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_CHARGE])));

  if(iprogram != PROG_IS_MOLPRO)
  	texts[E_LAYER] = g_strdup(gtk_entry_get_text(GTK_ENTRY(Entry[E_LAYER])));
  else
  	texts[E_LAYER] = g_strdup(" ");

  Nc = LineSelected;
  if(Nc<0)
	  Nc = LineSelectedOld;
  if(Nc>-1) 
  {

  for(i=E_X;i<=E_Z;i++)
  	if(test(texts[i]) && !testpointeE(texts[i]) )
		texts[i]=g_strdup_printf("%s.0",texts[i]);

  if(!strcmp(GeomXYZ[Nc].Residue,texts[E_RESIDUE]))
	  oldResidue = TRUE;
  GeomXYZ[Nc].Nentry=NUMBER_LIST_XYZ;
  GeomXYZ[Nc].Symb  = g_strdup(texts[E_SYMBOL]);
  GeomXYZ[Nc].mmType  = g_strdup(texts[E_MMTYPE]);
  GeomXYZ[Nc].pdbType  = g_strdup(texts[E_PDBTYPE]);
  GeomXYZ[Nc].Residue  = g_strdup(texts[E_RESIDUE]);
  GeomXYZ[Nc].X     = g_strdup(texts[E_X]);
  GeomXYZ[Nc].Y     = g_strdup(texts[E_Y]);
  GeomXYZ[Nc].Z     = g_strdup(texts[E_Z]);
  GeomXYZ[Nc].Charge = g_strdup(texts[E_CHARGE]);
  GeomXYZ[Nc].Layer = g_strdup(texts[E_LAYER]);
  if(!oldResidue)
  {
	  gint k;
	  GeomXYZ[Nc].ResidueNumber = -1; 
	  for(k=0;k<(gint)NcentersXYZ;k++)
	  {
		  if(Nc != k && !strcmp(GeomXYZ[Nc].Residue,GeomXYZ[k].Residue))
		  {
			  GeomXYZ[Nc].ResidueNumber = GeomXYZ[k].ResidueNumber;
			  break;
		  }
	  }
	  if(GeomXYZ[Nc].ResidueNumber == -1)
	  {
	  	for(k=0;k<(gint)NcentersXYZ;k++)
	  	{
		  	if(GeomXYZ[Nc].ResidueNumber<GeomXYZ[k].ResidueNumber)
			  	GeomXYZ[Nc].ResidueNumber = GeomXYZ[k].ResidueNumber;
	  	}
	  	GeomXYZ[Nc].ResidueNumber += 1;
	  }
  }
  
  removeFromList(list, Nc);
  insertToList(list, Nc, texts, NUMBER_LIST_XYZ);

  ChangeVariablesXYZUseds();
  if(GeomDrawingArea != NULL)
       rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();
  }
  else
	MessageGeom(_("Sorry No line selected"),_("Error"),TRUE);

  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();
    
  
}
/********************************************************************************/
static gchar **get_list_variablesXYZ()
{
  guint i;
  gchar **tlist;
  gint nl = NVariablesXYZ*2+1;

  tlist=g_malloc(nl*sizeof(char*));

  tlist[0] = g_strdup("");
  for (i=0;i<NVariablesXYZ;i++)
    tlist[i+1] = g_strdup(VariablesXYZ[i].Name);
  for (i=NVariablesXYZ;i<2*NVariablesXYZ;i++)
    tlist[i+1] = g_strdup_printf("-%s",VariablesXYZ[i-NVariablesXYZ].Name);
 
  return tlist;
}
/********************************************************************************/
static void freelistvariablesXYZ(gchar **tlist)
{
  guint i;
  guint nl = NVariablesXYZ*2+1;

  for (i=0;i<nl;i++)
    g_free(tlist[i]);
 
  g_free(tlist);
}


/********************************************************************************/
static void DialogueAdd(GtkWidget *w,gpointer data)
{
  GtkWidget *Dialogue;
  GtkWidget *Bouton;
  GtkWidget *hbox;
  GtkWidget *frame, *vboxframe;
  gint nlist;
  gchar *tlabel[]={	" ",	
	  		N_("Atom Symbol : "), 
	  		N_("MM Type : "),
	  		N_("PDB Type : "),
	  		N_("Residue Type : "),
			" X : ", 
			" Y : ", 
			" Z : ", 
			N_(" Charge : "),N_(" Layer : ")
  		};
  gint i;
  gchar **tlist;
  gchar **tlistvar;
  
  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),_("New Center"));
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));

  add_child(WindowGeom,Dialogue,gtk_widget_destroy,_(" New Center "));
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);


  hbox=create_hbox_false(vboxframe);
  Entry[E_SYMBOL] = create_label_entry(hbox,tlabel[E_SYMBOL],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  if(NcentersXYZ==0)
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_SYMBOL]),"H");
  else
  	gtk_entry_set_text(GTK_ENTRY(Entry[E_SYMBOL]),GeomXYZ[NcentersXYZ-1].Symb);
  gtk_editable_set_editable((GtkEditable*) Entry[E_SYMBOL],FALSE);

  Bouton = gtk_button_new_with_label(_(" Set "));
  gtk_box_pack_start (GTK_BOX(hbox), Bouton, TRUE, TRUE, 5);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)SelectAtom,
                     Entry[E_SYMBOL]);

  hbox=create_hbox_false(vboxframe);
  {
	gint n=0;
	gchar** t = getListMMTypes(&n);
	if(n!=0)
	{
		Entry[E_MMTYPE] = create_label_combo(hbox,tlabel[E_MMTYPE],t,n,
		TRUE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}
	else
	{
		Entry[E_MMTYPE] = create_label_entry(hbox,tlabel[E_MMTYPE],
		(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}

	if(t)
		freeList(t,n);

	if(NcentersXYZ==0)
	{
  		gtk_entry_set_text(GTK_ENTRY(Entry[E_MMTYPE]),"H");
	}
	else
	{
		gtk_entry_set_text(GTK_ENTRY(Entry[E_MMTYPE]),GeomXYZ[NcentersXYZ-1].mmType);
	}
  }
  hbox=create_hbox_false(vboxframe);
  {
	gint n=0;
	gchar** t = getListPDBTypes("UNK", &n);
	if(n!=0)
	{
		Entry[E_PDBTYPE] = create_label_combo(hbox,tlabel[E_PDBTYPE],t,n,
		TRUE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}
	else
	{
		Entry[E_PDBTYPE] = create_label_entry(hbox,tlabel[E_PDBTYPE],
		(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}

	if(t) freeList(t,n);

	if(NcentersXYZ==0)
	{
  		gtk_entry_set_text(GTK_ENTRY(Entry[E_PDBTYPE]),"H");
	}
	else
	{
		gtk_entry_set_text(GTK_ENTRY(Entry[E_PDBTYPE]),GeomXYZ[NcentersXYZ-1].pdbType);
	}
  }
  hbox=create_hbox_false(vboxframe);
  Entry[E_RESIDUE] = create_label_entry(hbox,tlabel[E_RESIDUE],
		(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));

  if(NcentersXYZ==0)
 	gtk_entry_set_text(GTK_ENTRY(Entry[E_RESIDUE])," ");
  else
	gtk_entry_set_text(GTK_ENTRY(Entry[E_RESIDUE]),GeomXYZ[NcentersXYZ-1].Residue);

  tlistvar  =get_list_variablesXYZ();

  for (i=E_X;i<=E_Z;i++)
  {
  	hbox=create_hbox_false(vboxframe);
  	Entry[i] = create_label_combo(hbox,tlabel[i],tlistvar,
			2*NVariablesXYZ+1,TRUE,
			(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
   
  }
  freelistvariablesXYZ(tlistvar);
  set_entry_XYZ();

  hbox=create_hbox_false(vboxframe);
  Entry[E_CHARGE] = create_label_entry(hbox,tlabel[E_CHARGE],
		  (gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  gtk_entry_set_text(GTK_ENTRY(Entry[E_CHARGE]),"0.0");

  tlist=g_malloc(4*sizeof(char*));
  
  tlist[0]=g_strdup(" ");
  tlist[1]=g_strdup("High");
  tlist[2]=g_strdup("Medium");
  tlist[3]=g_strdup("Low");
  nlist=4;
  if(NcentersXYZ<3)
	nlist=1;
  
  hbox=create_hbox_false(vboxframe);
  if(iprogram != PROG_IS_MOLPRO)
  	Entry[E_LAYER] = create_label_combo(hbox,tlabel[E_LAYER],tlist,nlist,FALSE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));

  gtk_widget_realize(Dialogue);

  Bouton = create_button(Dialogue,_("Cancel"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);

  Bouton = create_button(Dialogue,_("OK"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked",(GCallback)addAtom,Entry[E_SYMBOL]);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)destroy_dialogue,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
    

  gtk_widget_show_all(Dialogue);
  g_free(tlist);
}

/********************************************************************************/
static void DialogueEdit()
{
  GtkWidget *Dialogue;
  GtkWidget *Bouton;
  GtkWidget *hbox;
  GtkWidget *frame, *vboxframe;
  gint nlist;

  gchar *tlabel[]={	" ",	
	  		N_("Atom Symbol : "), 
	  		N_("MM Type : "),
	  		N_("PDB Type : "),
	  		N_("Residue Type : "),
			" X : ", 
			" Y : ", 
			" Z : ", 
			N_(" Charge : "),N_(" Layer : ")
  		};
  gint i;
  gchar **tlist;
  gchar **tlistvar;
  gint Nc;
  Nc=LineSelected;
  if(Nc<0 ) {
  	if(NcentersXYZ<1 ) 
   		MessageGeom(_("Create center beforee \n"),_("Error"),TRUE);
        else
   		MessageGeom(_("Please Select your center \n"),_("Warning"),TRUE);
   return;
  }

  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),_("Edit Center"));
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));

  add_child(WindowGeom,Dialogue,gtk_widget_destroy,_(" Edit Center "));
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);


  hbox=create_hbox_false(vboxframe);
  Entry[E_SYMBOL] = create_label_entry(hbox,tlabel[E_SYMBOL],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  gtk_entry_set_text(GTK_ENTRY(Entry[E_SYMBOL]),GeomXYZ[Nc].Symb);
  gtk_editable_set_editable((GtkEditable*) Entry[E_SYMBOL],FALSE);

  Bouton = gtk_button_new_with_label(_(" Set "));
  gtk_box_pack_start (GTK_BOX(hbox), Bouton, TRUE, TRUE, 5);
  g_signal_connect(G_OBJECT(Bouton), "clicked", (GCallback)SelectAtom,
                     Entry[E_SYMBOL]);

  hbox=create_hbox_false(vboxframe);
  {
	gint n=0;
	gchar** t = getListMMTypes(&n);
	if(n!=0)
	{
		Entry[E_MMTYPE] = create_label_combo(hbox,tlabel[E_MMTYPE],t,n,
		TRUE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}
	else
	{
		Entry[E_MMTYPE] = create_label_entry(hbox,tlabel[E_MMTYPE],
		(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}

	if(t) freeList(t,n);
	gtk_entry_set_text(GTK_ENTRY(Entry[E_MMTYPE]),GeomXYZ[Nc].mmType);
  }
  hbox=create_hbox_false(vboxframe);
  {
	gint n=0;
	gchar** t = getListPDBTypes(GeomXYZ[Nc].Residue, &n);
	if(n!=0)
	{
		Entry[E_PDBTYPE] = create_label_combo(hbox,tlabel[E_PDBTYPE],t,n,
		TRUE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}
	else
	{
		Entry[E_PDBTYPE] = create_label_entry(hbox,tlabel[E_PDBTYPE],
		(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
	}

	if(t) freeList(t,n);
	gtk_entry_set_text(GTK_ENTRY(Entry[E_PDBTYPE]),GeomXYZ[Nc].pdbType);
  }
  hbox=create_hbox_false(vboxframe);
  Entry[E_RESIDUE] = create_label_entry(hbox,tlabel[E_RESIDUE],
		(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));

  if(NcentersXYZ==0)
 	gtk_entry_set_text(GTK_ENTRY(Entry[E_RESIDUE])," ");
  else
	gtk_entry_set_text(GTK_ENTRY(Entry[E_RESIDUE]),GeomXYZ[Nc].Residue);


  tlistvar  =get_list_variablesXYZ();

  for (i=E_X;i<=E_Z;i++)
  {
  	hbox=create_hbox_false(vboxframe);
  	if(i==E_X)tlistvar[0] = g_strdup(GeomXYZ[Nc].X);
  	if(i==E_Y)tlistvar[0] = g_strdup(GeomXYZ[Nc].Y);
  	if(i==E_Z)tlistvar[0] = g_strdup(GeomXYZ[Nc].Z);
  	Entry[i] = create_label_combo(hbox,tlabel[i],tlistvar,
			2*NVariablesXYZ+1,TRUE,
			(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  }
  freelistvariablesXYZ(tlistvar);
  gtk_entry_set_text(GTK_ENTRY(Entry[E_X]),GeomXYZ[Nc].X);
  gtk_entry_set_text(GTK_ENTRY(Entry[E_Y]),GeomXYZ[Nc].Y);
  gtk_entry_set_text(GTK_ENTRY(Entry[E_Z]),GeomXYZ[Nc].Z);

  hbox=create_hbox_false(vboxframe);
  Entry[E_CHARGE] = create_label_entry(hbox,tlabel[E_CHARGE],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  gtk_entry_set_text(GTK_ENTRY(Entry[E_CHARGE]),GeomXYZ[Nc].Charge);

  tlist=g_malloc(4*sizeof(char*));
  tlist[0]=g_strdup(" ");
  tlist[1]=g_strdup("High");
  tlist[2]=g_strdup("Medium");
  tlist[3]=g_strdup("Low");
  nlist=4;
  if(LineSelected<3)
	nlist=1;
  
  hbox=create_hbox_false(vboxframe);
  if(iprogram != PROG_IS_MOLPRO)
  {
    Entry[E_LAYER] = create_label_combo(hbox,tlabel[E_LAYER],tlist,nlist,FALSE,(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
    gtk_entry_set_text(GTK_ENTRY(Entry[E_LAYER]),GeomXYZ[Nc].Layer);  
  }
  gtk_widget_realize(Dialogue);

  Bouton = create_button(Dialogue,_("Cancel"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);

  Bouton = create_button(Dialogue,_("OK"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked",(GCallback)EditAtom,Entry[E_SYMBOL]);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)destroy_dialogue,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
    

  gtk_widget_show_all(Dialogue);
  g_free(tlist);
}
/********************************************************************************/
static void DialogueDelete()
{
  GtkWidget *Dialogue;
  GtkWidget *Label;
  GtkWidget *Bouton;
  GtkWidget *frame, *vboxframe;


  if (NcentersXYZ <1)
  {
    MessageGeom(_(" No Center to delete !"),_("Info"),TRUE);
    return ;
  }

  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),_("Delete Center"));
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));
  gtk_window_set_modal (GTK_WINDOW (Dialogue), TRUE);

  add_child(WindowGeom,Dialogue,gtk_widget_destroy,_(" Delete Center "));
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  gtk_widget_realize(Dialogue);
  Label = create_label_with_pixmap(Dialogue,_("\nAre you sure to delete \nthe last center?\n"),_("Question"));
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  gtk_box_pack_start(GTK_BOX(vboxframe), Label,TRUE,TRUE,0);

  Bouton = create_button(Dialogue,_("No"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
 
  Bouton = create_button(Dialogue,_("Yes"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked",(GCallback)DelAtom, NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
    

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
static gchar* get_symb_atom(gchar* symb)
{
	gchar* S;
	gchar Forbidden[]={'0','1','2','3','4','5','6','7','8','9'};

	if(strlen(symb)==1)
		S = g_strdup(symb);
	else
	{
		gint Ok = 1;
		gint i;
		for(i=0;i<10;i++)
			if(symb[1]== Forbidden[i])
			{
				Ok =0;
				break;
			}
		if(Ok==1)
		{
			S = g_strdup_printf("%c%c",toupper(symb[0]),symb[1]);
		}
		else
			S = g_strdup_printf("%c",toupper(symb[0]));
	}
	if(!test_atom_define(S))
	{
		S = g_strdup_printf("%c",toupper(symb[0]));
		if(!test_atom_define(S))  S = g_strdup("H");
	}


	return S;
}
/********************************************************************************/
static gint get_connections_one_atom_hin(gchar* t, gint nAtoms, gint* connections)
{
	gint k;
	gint nc;
	gint nj;
	gchar** split = NULL;
	gint nA = 0;
	gint type = 1;
	gint ibeg = 11;
	for(k=0;k<nAtoms;k++) connections[k] = 0;
	split = gab_split(t);
	nA = 0;
	while(split && split[nA]!=NULL) nA++;
	if(nA<ibeg)
	{
		g_strfreev(split);
		return 0;
	}
	nc = atoi(split[ibeg-1]);
	for(k=0;k<2*nc;k+=2) 
	{
		if(!split[ibeg+k]) break;
		if(!split[ibeg+k+1]) break;
		nj = atoi(split[ibeg+k]);
		type = 1;
		if(strstr(split[ibeg+k+1],"d"))type = 2;
		if(strstr(split[ibeg+k+1],"D"))type = 2;
		if(strstr(split[ibeg+k+1],"t"))type = 3;
		if(strstr(split[ibeg+k+1],"T"))type = 3;
		connections[nj-1] = type;
	}

	g_strfreev(split);

	return 1;
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
		uppercase(dump);
		if(!strcmp(dump,"ATOM")) (*natoms)++;
		if(!strcmp(dump,"RES")) (*nresidues)++;
	}
}
/*************************************************************************************/
static gboolean read_atom_hin_file(FILE* file,gchar* listFields[], gint nAtoms, gint* connections, gint* nc)
{
	guint taille = BSIZE;
	gchar t[BSIZE];
	gchar dump[BSIZE];
	gint i;

    	if(!fgets(t,taille,file)) return FALSE;
    	sscanf(t,"%s",dump);
	uppercase(dump);
	if(strcmp(dump,"ATOM")!=0)
	{
		if(strcmp(dump,"RES")==0)
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
		*nc = get_connections_one_atom_hin(t,  nAtoms, connections);
	}
	for(i=0;i<6;i++)
	{
		delete_last_spaces(listFields[i]);
		delete_first_spaces(listFields[i]);
	}
	return TRUE;

}
/*************************************************************************************/
void read_hin_file_no_add_list(gchar *NomFichier)
{
	FILE *fd;
	int natoms;
	int nresidues;
	gchar *listFields[8];
	gint i;
	gint j;
	gchar resName[BSIZE] = "U";
	gint taille = BSIZE;
	gint ncAll =0;

	for(i=0;i<8;i++) listFields[i]=g_malloc(taille*sizeof(gchar));

	fd = FOpen(NomFichier, "rb");
 	if(fd == NULL)
	{
		gchar* t;
		t = g_strdup_printf(_("Sorry\n I can not open \"%s\" file"),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	read_hin_numbers_of_atoms(fd,&natoms, &nresidues);

	if(natoms <1)
	{
		gchar* t;
		t = g_strdup_printf(_("Sorry\n I can read \"%s\" file"),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}

 	if(GeomXYZ) freeGeomXYZ();
 	if(VariablesXYZ) freeVariablesXYZ();
	init_dipole();
	GeomXYZ=g_malloc(natoms*sizeof(GeomXYZAtomDef));
  	for(i=0; i<natoms; i++) GeomXYZ[i].typeConnections = g_malloc(natoms*sizeof(gint));
	NcentersXYZ = natoms;
 	NVariablesXYZ = 0;

	fseek(fd, 0L, SEEK_SET);
	j = 0;
	i = -1; /* number residu */
	while(!feof(fd))
	{
		gint nc = 0;
		if(!read_atom_hin_file(fd,listFields, natoms,GeomXYZ[j].typeConnections,&nc))continue;
		if(strcmp(listFields[0],"Unknown")==0)
		{
			sprintf(resName,"%s",listFields[1]);
			i++;
			continue;
		}
		ncAll += nc;
		/* 0 -> Atom Type PDB Style*/
		/* 1 -> Atom Symbol*/
		/* 2 -> Atom Type Amber*/
		/* 3 -> Atom Charge*/
		/* 4 -> x*/
		/* 5 -> y*/
		/* 6 -> z*/
		/* printf("%s %s %s %s %s %s %s \n",listFields[0],listFields[1],listFields[2],listFields[3],listFields[4],listFields[5],listFields[6]);*/
		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb= g_strdup(listFields[1]);

		if(strcmp(listFields[0],"-")==0) GeomXYZ[j].pdbType= g_strdup(listFields[1]);
		else GeomXYZ[j].pdbType= g_strdup(listFields[1]);
			
		if(strcmp(listFields[2],"**")==0) GeomXYZ[j].mmType= g_strdup(listFields[1]);
		else GeomXYZ[j].mmType= g_strdup(listFields[2]);

		GeomXYZ[j].Residue= g_strdup(resName);
		GeomXYZ[j].ResidueNumber= i;

		if(Units==1)
		{
			GeomXYZ[j].X=g_strdup(listFields[4]);
			GeomXYZ[j].Y=g_strdup(listFields[5]);
			GeomXYZ[j].Z=g_strdup(listFields[6]);
		}
		else
		{
			GeomXYZ[j].X=g_strdup(ang_to_bohr(listFields[4]));
			GeomXYZ[j].Y=g_strdup(ang_to_bohr(listFields[5]));
			GeomXYZ[j].Z=g_strdup(ang_to_bohr(listFields[6]));
		}
		GeomXYZ[j].Charge=g_strdup(listFields[3]);

		GeomXYZ[j].Layer=g_strdup(" ");
		j++;
		if(j>=natoms)break;
	}
	if(natoms!=j)
	{
 		if(GeomXYZ) freeGeomXYZ();
 		if(VariablesXYZ) freeVariablesXYZ();
		NcentersXYZ = 0;
 		NVariablesXYZ = 0;
	}

	fclose(fd);
	for(i=0;i<8;i++)
		g_free(listFields[i]);
	if(ncAll==0)
	{
  		for(i=0; i<natoms; i++) if(GeomXYZ[i].typeConnections) g_free(GeomXYZ[i].typeConnections);
  		for(i=0; i<natoms; i++) GeomXYZ[i].typeConnections = NULL;
	}

	if(GeomIsOpen)
		create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
	else
	if(GeomDrawingArea == NULL)
	{
		/*
		switch(iprogram)
		{
			case PROG_IS_MOLPRO : insert_molpro(2); break;
			case PROG_IS_GAUSS : insert_gaussian(2);
			break;
			default : edit_geometry();
		}
		*/
		create_window_drawing();
        }
	
	if(GeomDrawingArea != NULL)
		rafresh_drawing();
}
/********************************************************************************/
void read_hin_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *NomFichier;
	gchar *projectname = NULL;
	gchar *datafile = NULL;
	gchar *localdir = NULL;
	gchar *remotehost  = NULL;
	gchar *remoteuser  = NULL;
	gchar *remotepass  = NULL;
	gchar *remotedir  = NULL;
	gchar *temp  = NULL;

 	if(response_id != GTK_RESPONSE_OK) return;

 	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}

	read_hin_file_no_add_list(NomFichier);

	datafile = get_name_file(NomFichier);
	temp = get_suffix_name_file(NomFichier);
	projectname = get_name_file(temp);
	localdir = get_name_dir(temp);
	if(lastdirectory) g_free(lastdirectory);
	lastdirectory = g_strdup(localdir);
	CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_HIN],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_HIN, NULL, defaultNetWorkProtocol);
	g_free(temp);
	g_free(datafile);
	g_free(projectname);
	g_free(localdir);
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
/*************************************************************************************/
static gint read_gabedit_file_all_geoms(gchar *fileName)
{
	gint res = 0;
	gint nG = get_number_geoms_gabedit(fileName);
	if(nG<0)
	{
		gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),fileName);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return 1;
	}
	if(nG<1)
	{
		gchar* t = g_strdup_printf(_("Sorry,\n No geometry in %s file"),fileName);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return 1;
	}
	if(nG==1)
	{
		res = read_gabedit_geoms_file(fileName,  1);
	}
	else
	{
		find_energy_gabedit(fileName);
	}
	if(res==0)
	{
		if(GeomIsOpen) create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
		else if(GeomDrawingArea == NULL) create_window_drawing();
		if(GeomDrawingArea != NULL) rafresh_drawing();
	}
	return res;
}
/*************************************************************************************/
void read_gabedit_file_no_add_list(gchar *fileName)
{
	if(read_gabedit_file_all_geoms(fileName) != 0)
	{

	}
}
/*************************************************************************************/
static void add_gabedit_file_to_list(G_CONST_RETURN gchar *fileName)
{
	gchar *projectname = NULL;
	gchar *datafile = NULL;
	gchar *localdir = NULL;
	gchar *remotehost  = NULL;
	gchar *remoteuser  = NULL;
	gchar *remotepass  = NULL;
	gchar *remotedir  = NULL;
	gchar *temp  = NULL;

	datafile = get_name_file(fileName);
	temp = get_suffix_name_file(fileName);
	projectname = get_name_file(temp);
	localdir = get_name_dir(temp);
	if(lastdirectory) g_free(lastdirectory);
	lastdirectory = g_strdup(localdir);
	CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_GABEDIT],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_GABEDIT, NULL, defaultNetWorkProtocol);
	g_free(temp);
	g_free(datafile);
	g_free(projectname);
	g_free(localdir);
}
/*************************************************************************************/
void read_gabedit_file_add_list(gchar *fileName)
{
	if(read_gabedit_file_all_geoms(fileName) != 0) return ;
	add_gabedit_file_to_list(fileName);
}
/********************************************************************************/
void read_gabedit_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar* fileName = NULL;
 	if(response_id != GTK_RESPONSE_OK) return;

 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_gabedit_file_add_list(fileName);
}
/********************************************************************************/
void read_poscar_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar* fileName = NULL;
 	if(response_id != GTK_RESPONSE_OK) return;

 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_poscar_file(fileName);
}
/********************************************************************************/
void read_cif_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gboolean applySymOp = TRUE;
	gchar* fileName = NULL;
 	if(response_id != GTK_RESPONSE_OK) return;

 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_cif_file(fileName, applySymOp);
}
/********************************************************************************/
void read_cif_file_nosym(GabeditFileChooser *SelecFile, gint response_id)
{
	gboolean applySymOp = FALSE;
	gchar* fileName = NULL;
 	if(response_id != GTK_RESPONSE_OK) return;

 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_cif_file(fileName, applySymOp);
}
/********************************************************************************/
static void save_in_xyz_and_conversion(GtkWidget *wid,gpointer data)
{
	gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar *inputFileName =  g_strdup_printf("%s%stmp%sgeom.xyz",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	G_CONST_RETURN gchar* outputFileName;
	gchar outputFileType[100];
	gchar inputFileType[] ="xyz" ;
	GtkWidget* entryFileName = NULL;
	GtkWidget* entryFileType = NULL;
	G_CONST_RETURN gchar* t0;
	gchar* t;
	gchar* strout;
	gchar* strerr;

	GtkWidget *Win = (GtkWidget *)data;
        if(!this_is_an_object((GtkObject*)Win)) return;

	entryFileName = g_object_get_data (G_OBJECT (Win), "EntryFileName");
	entryFileType = g_object_get_data (G_OBJECT (Win), "EntryCombo");
	
        if(!this_is_an_object((GtkObject*)entryFileName)) return;
        if(!this_is_an_object((GtkObject*)entryFileType)) return;

	save_xyz_file_no_add_list(inputFileName);

	outputFileName = gtk_entry_get_text(GTK_ENTRY(entryFileName)); 

	t0 = gtk_entry_get_text(GTK_ENTRY(entryFileType)); 
	sscanf(t0,"%s",outputFileType);
	if(strstr(t0,"Automatic")) t = g_strdup_printf("%s %s -O %s", babelCommand, inputFileName, outputFileName);
	else t = g_strdup_printf("%s -i%s %s -o%s -O %s", babelCommand, inputFileType, inputFileName, outputFileType, outputFileName);

	run_local_command(fout,ferr,t,FALSE);
	strout = cat_file(fout,FALSE);
	strerr = cat_file(ferr,FALSE);
  	if(strout)
	{
 		Message(strout, "Info", TRUE);
		g_free(strout);
	}
  	if(strerr)
	{
 		Message(strerr, "Error", TRUE);
		g_free(strerr);
	}
	unlink(fout);
	unlink(ferr);
	unlink(inputFileName);
	g_free(fout);
	g_free(ferr);
	g_free(t);
	g_free(inputFileName);
}
/********************************************************************************/
void create_babel_save_dialogue()
{
	GtkWidget* okButton = create_babel_read_save_dialogue(FALSE);
	GtkWidget* win = g_object_get_data (G_OBJECT (okButton), "Window");

	g_signal_connect(G_OBJECT(okButton), "clicked", G_CALLBACK(save_in_xyz_and_conversion),win);
	g_signal_connect_swapped(G_OBJECT(okButton), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(win));
}
/******************************************************************************/
static void save_atom_hin_file(FILE* file, 
		gchar*name, gint atomNumber,
		gchar* atomPDBType, gchar* atomMMType, gchar* residueName,
		gint residueNumber,
		gdouble x, gdouble y, gdouble z,
		gdouble occupancy,
		gdouble temperature,
		gchar* symbol,
		gdouble charge,
		gint N,
		gint* connection,
		gint* connectionType
		)
{
	gint i;
        fprintf(file,"%s %d ",name,atomNumber);
        fprintf(file,"%s ",atomPDBType);
        fprintf(file,"%s ",symbol); 
        fprintf(file,"%s - ",atomMMType); 
        fprintf(file,"%f ",charge); 
        fprintf(file,"%f ",x); 
        fprintf(file,"%f ",y); 
        fprintf(file,"%f ",z); 
	if(N>0)
	{
        	fprintf(file,"%d ",N); 
		for(i=0;i<N;i++)
		{
			if(connectionType[i]==3) fprintf(file,"%d t ",connection[i]); 
			else if(connectionType[i]==2) fprintf(file,"%d d ",connection[i]); 
			else fprintf(file,"%d s ",connection[i]); 
		}
	}
        fprintf(file,"\n"); 
}
/*****************************************************************************/
gboolean save_hin_file(G_CONST_RETURN gchar* FileName)
{
	gint i;
	gint j;
	gint k;
	FILE* fd;
	gdouble X;
	gdouble Y;
	gdouble Z;
	gchar *projectname = NULL;
	gchar *datafile = NULL;
	gchar *localdir = NULL;
	gchar *remotehost  = NULL;
	gchar *remoteuser  = NULL;
	gchar *remotepass  = NULL;
	gchar *remotedir  = NULL;
	gchar *temp  = NULL;
	gint* connection = NULL;
	gint* connectionType = NULL;
	gint N;

	if(NcentersXYZ<1) return TRUE;

	temp = get_suffix_name_file(FileName);
	FileName = g_strdup_printf("%s.hin",temp);
	g_free(temp);
	fd = FOpen(FileName, "w");


	if(fd == NULL)
	{
		gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),FileName);
		Message(t,_("Error"),TRUE);
		g_free(t);
		return FALSE;
	}

 	sortGeomXYZByResidueNumber();


	fprintf(fd,"forcefield Amber99\n");
	fprintf(fd,"sys 0 0 1\n");
	fprintf(fd,"view 40 0.1272 55 15 0.247224 0.3713666 0.8949677 -0.8641704 0.5022867 0.0302929 -0.4382806 -0.7808937 0.4451014 6.191 0.64575 -54.754\n");
	fprintf(fd,"seed -1108\n");
	fprintf(fd,"mol 1\n");
	if(NcentersXYZ>0 && strcmp(GeomXYZ[0].Residue,"U")!=0)
		fprintf(fd,"res 1 %s 1 - - \n",GeomXYZ[0].Residue);

	connection = g_malloc(NcentersXYZ*sizeof(gint));
	connectionType = g_malloc(NcentersXYZ*sizeof(gint));

	j = 1;
	for(i=0;i<(gint)NcentersXYZ;i++)
	{
		N = 0;
		if(GeomXYZ[i].typeConnections)
		for(k=0;k<(gint)NcentersXYZ;k++)
		{
			if(i==k) continue;
			if(GeomXYZ[i].typeConnections[k]>0)
			{
				connection[N] = k+1;
				connectionType[N] = GeomXYZ[i].typeConnections[k];
				N++;
			}
		}

		if(i>0&& strcmp(GeomXYZ[i].Residue,GeomXYZ[i-1].Residue)!=0)
		{
			fprintf(fd,"endres %d\n",j);
			/* if(i!=(gint)(NcentersXYZ-1)) */
				fprintf(fd,"res %d %s %d - - \n",j+1,GeomXYZ[i].Residue,j+1);
			j++;
		}
		if(!test(GeomXYZ[i].X)) X = get_value_variableXYZ(GeomXYZ[i].X);
		else X = atof(GeomXYZ[i].X);
		if(!test(GeomXYZ[i].Y)) Y = get_value_variableXYZ(GeomXYZ[i].Y);
		else Y = atof(GeomXYZ[i].Y);
		if(!test(GeomXYZ[i].Z)) Z = get_value_variableXYZ(GeomXYZ[i].Z);
		else Z = atof(GeomXYZ[i].Z);
		if(Units==0)
		{
			X *= BOHR_TO_ANG;
			Y *= BOHR_TO_ANG;
			Z *= BOHR_TO_ANG;
		}
		save_atom_hin_file(fd,"ATOM",i+1,GeomXYZ[i].pdbType, GeomXYZ[i].mmType,GeomXYZ[i].Residue,
		GeomXYZ[i].ResidueNumber, X,Y,Z,
		1.0, 300.0, GeomXYZ[i].Symb, atof(GeomXYZ[i].Charge),N,connection, connectionType);
	}
	if(NcentersXYZ>0 && strcmp(GeomXYZ[NcentersXYZ-1].Residue,"U")!=0)
		fprintf(fd,"endres %d\n",j);
	fprintf(fd,"endmol 1\n");
	fclose(fd);
	g_free(connection);
	g_free(connectionType);

	datafile = get_name_file(FileName);
	temp = get_suffix_name_file(FileName);
	projectname = get_name_file(temp);
	localdir = get_name_dir(temp);
	if(lastdirectory)
		g_free(lastdirectory);
	lastdirectory = g_strdup(localdir);
	CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_HIN],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_HIN, NULL, defaultNetWorkProtocol);
	g_free(temp);
	g_free(datafile);
	g_free(projectname);
	g_free(localdir);
	return TRUE;
}
/************************************************************************************/
void save_geometry_hin_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	 save_hin_file(FileName);
}
/********************************************************************************/
void save_hin_file_entry(GtkWidget* entry)
{
	G_CONST_RETURN gchar *FileName;

	FileName = gtk_entry_get_text(GTK_ENTRY(entry));
	if ((!FileName) || (strcmp(FileName,"") == 0))
		return ;
	 save_hin_file(FileName);
}
/*****************************************************************************/
static gboolean save_cchemi_file(G_CONST_RETURN gchar* fileNameGeom)
{
	FILE* file = NULL;
	gint j;
	gint k;
	gdouble X,Y,Z;
	gint nc = 0;
	gint* connection = NULL;
	gint* connectionType = NULL;
	gint ct;
	gboolean allFixed = TRUE;

	if(NcentersXYZ<1) return FALSE;
	if(!GeomXYZ) return FALSE;

 	file = fopen(fileNameGeom, "w");

	if(!file) return FALSE;
	fprintf(file,"#RunType = Energy, Optimization, MD, MDConfo, REMDConfo, RDCONFO, OptFreq\n");
	fprintf(file,"RunType=MDConfo\n");
	fprintf(file,"#Model = MM , Mopac , Orca or FireFly\n");
	fprintf(file,"Model=MM\n");
	fprintf(file,"#QMKeys=PM6\n");
	fprintf(file,"QMKeys=AM1\n");
	fprintf(file,"mopacCommand=mopac\n");
	fprintf(file,"orcaCommand=orca\n");
	fprintf(file,"fireflyCommand=firefly\n");
	fprintf(file,"\n");
	fprintf(file,"#Confo\n");
	fprintf(file,"gaussianCommand=g09\n");
	fprintf(file,"fireflyCommand=firefly\n");
	fprintf(file,"numberOfGeometries=500\n");
	fprintf(file,"tolEnergy=0.1\n");
	fprintf(file,"tolDistance=0.1\n");
	fprintf(file,"ConfoOptMM=TRUE\n");
	fprintf(file,"ConfoOptMopac=TRUE\n");
	fprintf(file,"ConfoOptMopacMethod=PM6 GNORM=0.001\n");
	fprintf(file,"ConfoOptFireFly=FALSE\n");
	fprintf(file,"# remove # if post processing required\n");
	fprintf(file,"#mopacKeywordsPost=PM6\n");
	fprintf(file,"gaussianKeywordsPost=B3LYP/6-31G*\n");
	fprintf(file,"#fireflyKeywordsPost=AM1\n");
	fprintf(file,"\n");
	fprintf(file,"#MM\n");
	fprintf(file,"# AMBER, UFF(not implemented), PAIRWISE\n");
	fprintf(file,"ForceFieldType=0\n");
	fprintf(file,"ForceFieldUseBond=TRUE\n");
	fprintf(file,"ForceFieldUseBend=TRUE\n");
	fprintf(file,"ForceFieldUseStrBend=FALSE\n");
	fprintf(file,"ForceFieldUseDihedral=TRUE\n");
	fprintf(file,"ForceFieldUseOutOfPlane=FALSE\n");
	fprintf(file,"ForceFieldUseImproper=FALSE\n");
	fprintf(file,"ForceFieldUseHydrogenBonded=FALSE\n");
	fprintf(file,"ForceFieldUsecoulomb=TRUE\n");
	fprintf(file,"#ForceFieldChargesType=ACKH2\n");
	fprintf(file,"#ForceFieldChargesType=ACKH2-BEGIN\n");
	fprintf(file,"#ForceFieldChargesType=EEM\n");
	fprintf(file,"#ForceFieldChargesType=EEM-BEGIN\n");
	fprintf(file,"ForceFieldVanderWals=6-12\n");
	fprintf(file,"#ForceFieldVanderWals=7-14\n");
	fprintf(file,"#  NOCONSTRAINTS = 0, BONDSCONSTRAINTS = 1, BONDSANGLESCONSTRAINTS = 2\n");
	fprintf(file,"ForceFieldConstraints=1\n");
	fprintf(file,"\n");
	fprintf(file,"#MD\n");
	fprintf(file,"updateFrequency=5\n");
	fprintf(file,"#Time in ps\n");
	fprintf(file,"heatTime = 0.1\n");
	fprintf(file,"equiTime = 0.1\n");
	fprintf(file,"runTime = 2.0\n");
	fprintf(file,"coolTime = 0\n");
	fprintf(file,"timeExchange = 0.01\n");
	fprintf(file,"heatTemp = 0\n");
	fprintf(file,"runTemp = 400\n");
	fprintf(file,"runTempMax = 700\n");
	fprintf(file,"nTemperatures = 10\n");
	fprintf(file,"#in fs\n");
	fprintf(file,"stepSize = 0.5\n");
	fprintf(file,"#  VERLET = 0, BEEMAN = 1, STOCHASTIC = 2\n");
	fprintf(file,"integrator = 0\n");
	fprintf(file,"#  NONE = 0, ANDERSEN = 1, BERENDSEN = 2, BUSSI = 3, NOSEHOOVER = 4\n");
	fprintf(file,"thermostat = 0\n");
	fprintf(file,"friction=40\n");
	fprintf(file,"collide = 20\n");
	fprintf(file,"\n");
	fprintf(file,"#QuasiNewton\n");
	fprintf(file,"useQuasiNewton = TRUE\n");
	fprintf(file,"quasiNewtonMaxIterations = 20000\n");
	fprintf(file,"quasiNewtonUpdateFrequency = 100\n");
	fprintf(file,"quasiNewtonEpsilon  = 0.0001\n");
	fprintf(file,"quasiNewtonTolerence = 1e-16\n");
	fprintf(file,"quasiNewtonMaxLines =  25\n");
	fprintf(file,"\n");
	fprintf(file,"#ConjugateGradient\n");
	fprintf(file,"useConjugateGradient = FALSE\n");
	fprintf(file,"conjugateGradientGradientNorm = 1e-3\n");
	fprintf(file,"conjugateGradientMaxIterations = 100\n");
	fprintf(file,"conjugateGradientUpdateFrequency = 1\n");
	fprintf(file,"conjugateGradientMaxLines = 25\n");
	fprintf(file,"conjugateGradientInitialStep = 0.001\n");
	fprintf(file,"# 1 : Hestenes Stiefel,  2 : Fletcher Reeves, 3 : Polak Ribiere, 4 : Wolf Powell\n");
	fprintf(file,"conjugateGradientMethod = 1\n");
	fprintf(file,"\n");
	fprintf(file,"#Geometry, nAtoms, charge, spin multiplicity. For each atom : symbol, MMType, pdbType, residueName, numResidue, charge, layer, x(Ang),y,z, nconn, num1, type1, num2, type2,...\n");
	fprintf(file,"Geometry\n");
	fprintf(file,"%d %d %d\n",Natoms,TotalCharges[0],SpinMultiplicities[0]);

	connection = g_malloc(NcentersXYZ*sizeof(gint));
	connectionType = g_malloc(NcentersXYZ*sizeof(gint));

	for(j=0;j<NcentersXYZ;j++)
	if( !test(GeomXYZ[j].X) || !test(GeomXYZ[j].Y) || !test(GeomXYZ[j].Z) ) {allFixed = FALSE;break;}

	for(j=0;j<NcentersXYZ;j++)
	{
		int variable = 0;
         	if( !test(GeomXYZ[j].X) || !test(GeomXYZ[j].Y) || !test(GeomXYZ[j].Z) ) variable = 1;
         	if(!test(GeomXYZ[j].X)) X = get_value_variableXYZ(GeomXYZ[j].X);
         	else X = atof(GeomXYZ[j].X);
         	if(!test(GeomXYZ[j].Y)) Y = get_value_variableXYZ(GeomXYZ[j].Y);
         	else Y = atof(GeomXYZ[j].Y);
         	if(!test(GeomXYZ[j].Z)) Z = get_value_variableXYZ(GeomXYZ[j].Z);
         	else Z = atof(GeomXYZ[j].Z);
		if(allFixed) variable = 1;
         	if(Units==0)
         	{
              		X *= BOHR_TO_ANG;
              		Y *= BOHR_TO_ANG;
              		Z *= BOHR_TO_ANG;
         	}
		nc = 0;
		if(GeomXYZ[j].typeConnections)
		for(k=0;k<(gint)NcentersXYZ;k++)
		{
			if(j==k) continue;
			ct = GeomXYZ[j].typeConnections[k];
			if( ct!=0)
			{
				connection[nc] = k+1;
				connectionType[nc] = ct;
				nc++;
			}
		}

		fprintf(file," %s %s %s %s %d %f %d %d %f %f %f ", 
				GeomXYZ[j].Symb,
				GeomXYZ[j].mmType,
				GeomXYZ[j].pdbType,
				GeomXYZ[j].Residue,
				GeomXYZ[j].ResidueNumber,
				atof(GeomXYZ[j].Charge),
				get_layer(GeomXYZ[j].Layer),
				variable,
				X,
				Y,
				Z
				);
		fprintf(file," %d ", nc);
		for(k=0;k<nc;k++)
		fprintf(file," %d %d", connection[k], connectionType[k]);
		fprintf(file,"\n");

	}

	fclose(file);
	return TRUE;
}
/*****************************************************************************/
static gboolean save_gabedit_file(G_CONST_RETURN gchar* fileNameGeom)
{
	FILE* file = NULL;
	gint j;
	gint k;
	gdouble X,Y,Z;
	gint nc = 0;
	gint* connection = NULL;
	gint* connectionType = NULL;
	gint ct;
	gint form = 1;

	if(NcentersXYZ<1) return FALSE;
	if(!GeomXYZ) return FALSE;

 	file = fopen(fileNameGeom, "w");

	if(!file) return FALSE;

	fprintf(file,"[Gabedit Format]\n");
	fprintf(file,"[GEOMS] %d\n",form);
	fprintf(file,"1 1\n");
	fprintf(file,"ENERGY UNK 1\n");
	fprintf(file,"0.0\n");
	fprintf(file,"%d %d %d\n",Natoms,TotalCharges[0],SpinMultiplicities[0]);

	connection = g_malloc(NcentersXYZ*sizeof(gint));
	connectionType = g_malloc(NcentersXYZ*sizeof(gint));

	for(j=0;j<NcentersXYZ;j++)
	{
		int variable = 0;
         	if(!test(GeomXYZ[j].X)) X = get_value_variableXYZ(GeomXYZ[j].X);
         	else X = atof(GeomXYZ[j].X);
         	if(!test(GeomXYZ[j].Y)) Y = get_value_variableXYZ(GeomXYZ[j].Y);
         	else Y = atof(GeomXYZ[j].Y);
         	if(!test(GeomXYZ[j].Z)) Z = get_value_variableXYZ(GeomXYZ[j].Z);
         	else Z = atof(GeomXYZ[j].Z);
         	if( !test(GeomXYZ[j].X) || !test(GeomXYZ[j].Y) || !test(GeomXYZ[j].Z) ) variable = 1;
         	if(Units==0)
         	{
              		X *= BOHR_TO_ANG;
              		Y *= BOHR_TO_ANG;
              		Z *= BOHR_TO_ANG;
         	}
		nc = 0;
		if(GeomXYZ[j].typeConnections)
		for(k=0;k<(gint)NcentersXYZ;k++)
		{
			if(j==k) continue;
			ct = GeomXYZ[j].typeConnections[k];
			if( ct!=0)
			{
				connection[nc] = k+1;
				connectionType[nc] = ct;
				nc++;
			}
		}

		fprintf(file," %s %s %s %s %d %f %d %d %f %f %f ", 
				GeomXYZ[j].Symb,
				GeomXYZ[j].mmType,
				GeomXYZ[j].pdbType,
				GeomXYZ[j].Residue,
				GeomXYZ[j].ResidueNumber,
				atof(GeomXYZ[j].Charge),
				get_layer(GeomXYZ[j].Layer),
				variable,
				X,
				Y,
				Z
				);
		fprintf(file," %d ", nc);
		for(k=0;k<nc;k++)
		fprintf(file," %d %d", connection[k], connectionType[k]);
		fprintf(file,"\n");

	}

	fclose(file);
	add_gabedit_file_to_list(fileNameGeom);
	return TRUE;
}
/************************************************************************************/
void save_geometry_gabedit_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	 save_gabedit_file(FileName);
}
/************************************************************************************/
void save_geometry_cchemi_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	 save_cchemi_file(FileName);
}
/*****************************************************************************************/
static void conversion_to_xyz_and_read(GtkWidget *wid,gpointer data)
{
	gchar *fout =  g_strdup_printf("%s%stmp%sfout",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar *ferr =  g_strdup_printf("%s%stmp%sferr",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar *outputFileName =  g_strdup_printf("%s%stmp%sgeom.xyz",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	G_CONST_RETURN gchar* inputFileName;
	gchar inputFileType[100];
	gchar outputFileType[] ="xyz" ;
	GtkWidget* entryFileName = NULL;
	GtkWidget* entryFileType = NULL;
	G_CONST_RETURN gchar* t0;
	gchar* t;
	gchar* strout;
	gchar* strerr;

	GtkWidget *Win = (GtkWidget *)data;
        if(!this_is_an_object((GtkObject*)Win)) return;

	entryFileName = g_object_get_data (G_OBJECT (Win), "EntryFileName");
	entryFileType = g_object_get_data (G_OBJECT (Win), "EntryCombo");
	
        if(!this_is_an_object((GtkObject*)entryFileName)) return;
        if(!this_is_an_object((GtkObject*)entryFileType)) return;

	inputFileName = gtk_entry_get_text(GTK_ENTRY(entryFileName)); 

	t0 = gtk_entry_get_text(GTK_ENTRY(entryFileType)); 
	sscanf(t0,"%s",inputFileType);
	if(strstr(t0,"Automatic")) t = g_strdup_printf("%s %s -O %s", babelCommand, inputFileName, outputFileName);
	else t = g_strdup_printf("%s -i%s %s -o%s -O %s", babelCommand, inputFileType, inputFileName, outputFileType, outputFileName);

	unlink(fout);
	unlink(ferr);
	/* printf("babelcommand = %s\n",t);*/
	run_local_command(fout,ferr,t,FALSE);
	strout = cat_file(fout,FALSE);
	strerr = cat_file(ferr,FALSE);
  	if(strout)
	{
 		Message(strout, "Info", TRUE);
		g_free(strout);
	}
  	if(strerr)
	{
 		Message(strerr, "Error", TRUE);
		g_free(strerr);
	}
	/*else*/
	{
		MethodeGeom = GEOM_IS_XYZ;
		read_XYZ_file_no_add_list(outputFileName);
	}
	unlink(fout);
	unlink(ferr);
	unlink(outputFileName);
	g_free(fout);
	g_free(ferr);
	g_free(t);
	g_free(outputFileName);
}
/********************************************************************************/
void create_babel_read_dialogue()
{
	GtkWidget* okButton = create_babel_read_save_dialogue(TRUE);
	GtkWidget* win = g_object_get_data (G_OBJECT (okButton), "Window");

	g_signal_connect(G_OBJECT(okButton), "clicked", G_CALLBACK(conversion_to_xyz_and_read),win);
	g_signal_connect_swapped(G_OBJECT(okButton), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(win));
}
/*************************************************************************************/
void read_charges_from_gromacs_topology_file(gchar *NomFichier)
{
	gchar *fileNameTOP = NULL;
	FILE* file;
	gchar* str = NULL;
	gchar** split = NULL;
	gint i,j;

	fileNameTOP = get_suffix_name_file(NomFichier);
	fileNameTOP = g_strdup_printf("%s.top",fileNameTOP);
	file = FOpen(fileNameTOP, "rb");
	if(fileNameTOP) g_free(fileNameTOP);
 	if(file == NULL) return;
	str = g_malloc(BSIZE*sizeof(gchar));
	fseek(file, 0L, SEEK_SET);

	while(!feof(file))
	{
    		if(!fgets(str,BSIZE,file)) break;
		if(strstr(str,"[ atoms ]"))
		{
			break;
		}
	}

	j = -1;
	while(!feof(file))
	{
    		if(!fgets(str,BSIZE,file)) break;
		if(strlen(str)>0 && str[0] ==';') continue;
		if(strstr(str,"[")) break;
		split = NULL;
		split = gab_split(str);
		i = 0;
		if(split)
		for(i=0;i<8;i++)
		{
			if(split[i]==NULL) break;
			if(i==6 && split[i]) break;
		}
		if(i==6)
		{
			j++;
			GeomXYZ[j].Charge=g_strdup(split[6]);
		}
		if(split) g_strfreev(split);
		if(j>=NcentersXYZ)  break;
	}
	fclose(file);
}
/*************************************************************************************/
static gboolean read_atom_pdb_file(gchar* line,gchar* listFields[])
{
	gint i;
	gint k = 0;
	if(strlen(line)<54)
		return FALSE;

	/* 0 -> Atom Type */
	k = 0;
	for(i=0;i<MAXATOMTYPE;i++)
		listFields[k][i] = line[13+i-1];
	listFields[k][MAXATOMTYPE] = '\0';
	if(isdigit(listFields[k][0]))
	{
		gchar c0 = listFields[k][0];
		for(i=0;i<MAXATOMTYPE-1;i++)
			listFields[k][i] = listFields[k][i+1];
		listFields[k][MAXATOMTYPE-1] = c0;
	}
	/* 1-> Residue Name */
	k = 1;
	for(i=0;i<MAXRESIDUENAME;i++)
		listFields[k][i] = line[17+i-1];
	listFields[k][MAXRESIDUENAME] = '\0';

	/* 2-> Residue Number */
	k = 2;
	for(i=0;i<4;i++)
		listFields[k][i] = line[23+i-1];
	listFields[k][4] = '\0';
	/* 3-> x */
	k = 3;
	for(i=0;i<8;i++)
		listFields[k][i] = line[31+i-1];
	listFields[k][8] = '\0';

	/* 4-> y */
	k = 4;
	for(i=0;i<8;i++)
		listFields[k][i] = line[39+i-1];
	listFields[k][8] = '\0';

	/* 5-> z */
	k = 5;
	for(i=0;i<8;i++)
		listFields[k][i] = line[47+i-1];
	listFields[k][8] = '\0';

	/* 6-> Symbol */
	k = 6;
	if(strlen(line)>=78)
	{
		for(i=0;i<2;i++)
		{
			listFields[k][i] = line[76+i];
		}
		listFields[k][2] = '\0';
		if(listFields[k][1]==' ')
			listFields[k][1] = '\0';
		if(listFields[k][0]==' ')
			listFields[k][0] = '\0';
	}
	else
		listFields[k][0] = '\0';
	/* 7-> Charge */
	k = 7;
	if(strlen(line)>=80)
	{
		for(i=0;i<(gint)strlen(line)-79+1;i++)
			listFields[k][i] = line[79+i-1];

		listFields[k][strlen(line)-79+1] = '\0';

		if(listFields[k][strlen(line)-79]=='\n')
			listFields[k][strlen(line)-79]='\0';

	}
	else
		listFields[k][0] = '\0';

	for(i=0;i<8;i++)
	{
		delete_last_spaces(listFields[i]);
		delete_first_spaces(listFields[i]);
	}
	return TRUE;

}
/********************************************************************************/
gint get_connections_one_connect_pdb(gchar* t)
{
	gint k;
	gint ni;
	gint nj;
	gchar** split = NULL;
	gint nA = 0;
	split = gab_split(t);
	nA = 0;
	while(split && split[nA]!=NULL) nA++;
	if(nA<3)
	{
		g_strfreev(split);
		return 0;
	}
	ni = atoi(split[1])-1;
	if(ni<0 || ni>NcentersXYZ-1) 
	{
		g_strfreev(split);
		return 0;
	}
	GeomXYZ[ni].typeConnections = g_malloc(NcentersXYZ*sizeof(gint));
	for(k=0;k<NcentersXYZ;k++) GeomXYZ[ni].typeConnections[k] = 0;
	for(k=0;k<nA-2;k++) 
	{
		if(!split[2+k]) break;
		nj = atoi(split[2+k])-1;
		if(nj<0 || nj>NcentersXYZ-1) continue;
		GeomXYZ[ni].typeConnections[nj] = 1;
	}

	g_strfreev(split);

	return 1;
}
/*************************************************************************************/
void read_pdb_file_no_add_list(gchar *NomFichier)
{
	gchar *t;
	gchar *listFields[8];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j;
	gdouble charge;

	for(i=0;i<8;i++)
		listFields[i]=g_malloc(taille*sizeof(gchar));


	fd = FOpen(NomFichier, "rb");
 	if(fd == NULL)
	{
		t = g_strdup_printf(_("Sorry\n I can not open \"%s\" file"),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		return;
	}
	t=g_malloc(taille);
	

 	if(GeomXYZ)
   		freeGeomXYZ();
 	if(VariablesXYZ)
		freeVariablesXYZ();
	init_dipole();
	GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
  	GeomXYZ[0].typeConnections = NULL;

	j=0;
	while(!feof(fd))
	{
    		if(!fgets(t,taille,fd)) break;
    		sscanf(t,"%s",listFields[0]);
		if(strcmp(listFields[0],"HETATM")!=0 && strcmp(listFields[0],"ATOM")!=0)
			continue;
		if(!strcmp(t,"END"))
			break;
		if(!read_atom_pdb_file(t,listFields))
			continue;
		/* 0 -> Atom Type  1-> Residue Name  2-> Residue Number 
		 * 3-> x  4-> y  5-> z  6-> Symbol 7-> Charge */
		GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
  		GeomXYZ[j].typeConnections = NULL;
		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		if(strlen(listFields[6])<=0)
			GeomXYZ[j].Symb= get_symb_atom(listFields[0]);
		else
			GeomXYZ[j].Symb= g_strdup(listFields[6]);
			
		GeomXYZ[j].pdbType= g_strdup(listFields[0]);
		GeomXYZ[j].Residue= g_strdup(listFields[1]);
		GeomXYZ[j].ResidueNumber= atoi(listFields[2])-1;
		GeomXYZ[j].mmType= getMMTypeFromPDBTpl(GeomXYZ[j].Residue,GeomXYZ[j].pdbType,&charge);

		if(Units==1)
		{
			GeomXYZ[j].X=g_strdup(listFields[3]);
			GeomXYZ[j].Y=g_strdup(listFields[4]);
			GeomXYZ[j].Z=g_strdup(listFields[5]);
		}
		else
		{
			GeomXYZ[j].X=g_strdup(ang_to_bohr(listFields[3]));
			GeomXYZ[j].Y=g_strdup(ang_to_bohr(listFields[4]));
			GeomXYZ[j].Z=g_strdup(ang_to_bohr(listFields[5]));
		}
		if(strlen(listFields[7])<=0)
		{
			GeomXYZ[j].Charge=g_strdup_printf("%f",charge);
		}
		else
			GeomXYZ[j].Charge=g_strdup(listFields[7]);

		GeomXYZ[j].Layer=g_strdup(" ");
		j++;
	}
	fseek(fd, 0L, SEEK_SET);
	NcentersXYZ = j;
 	NVariablesXYZ = 0;
	while(!feof(fd))
	{
    		if(!fgets(t,taille,fd)) break;
    		sscanf(t,"%s",listFields[0]);
		if(strcmp(listFields[0],"CONNECT")!=0) continue;
		if(!strcmp(t,"END")) break;
		if(!read_atom_pdb_file(t,listFields)) continue;
		get_connections_one_connect_pdb(t);
	}
	
	fclose(fd);
	/* multiple bond not define in a pdb file */
	set_multiple_bonds();

	g_free(t);
	for(i=0;i<8;i++)
		g_free(listFields[i]);
	read_charges_from_gromacs_topology_file(NomFichier);
	
	if(GeomIsOpen)
	{
		create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
	}
	else
	if(GeomDrawingArea == NULL)
	{
		/*
		switch(iprogram)
		{
			case PROG_IS_MOLPRO : 
				insert_molpro(2);
			break;
			case PROG_IS_GAUSS : 
				insert_gaussian(2);
			break;
			default :
				edit_geometry();
		}
		*/
		create_window_drawing();
        }
	if(GeomDrawingArea != NULL) rafresh_drawing();
 	if(NcentersXYZ==0)
	{
		t = g_strdup_printf(_("Sorry\n I can read \"%s\" file"),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		return;
	}
}
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
/******************************************************************************/
gboolean save_pdb_file(G_CONST_RETURN gchar* FileName)
{
 guint i;
 guint j;
 FILE* fd;
 gdouble X;
 gdouble Y;
 gdouble Z;
 gchar *projectname = NULL;
 gchar *datafile = NULL;
 gchar *localdir = NULL;
 gchar *remotehost  = NULL;
 gchar *remoteuser  = NULL;
 gchar *remotepass  = NULL;
 gchar *remotedir  = NULL;
 gchar *temp  = NULL;
 gint* connection = NULL;
 gint N;

 temp = get_suffix_name_file(FileName);
 FileName = g_strdup_printf("%s.pdb",temp);
 g_free(temp);
 fd = FOpen(FileName, "w");

 if(fd == NULL)
 {
	gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),FileName);
	Message(t,_("Error"),TRUE);
	g_free(t);
	return FALSE;
 }
 sortGeomXYZByResidueNumber();

  fprintf(fd,"HEADER    PROTEIN\n");
  fprintf(fd,"COMPND    UNNAMED\n");
  temp = get_time_str();
  if(temp)
  	fprintf(fd,"AUTHOR    GENERATED BY GABEDIT %d.%d.%d at %s",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION,temp);
  else
  	fprintf(fd,"AUTHOR    GENERATED BY GABEDIT %d.%d.%d\n",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);


 for(i=0;i<NcentersXYZ;i++)
 {
         if(!test(GeomXYZ[i].X))
                 X = get_value_variableXYZ(GeomXYZ[i].X);
         else
                 X = atof(GeomXYZ[i].X);
         if(!test(GeomXYZ[i].Y))
                 Y = get_value_variableXYZ(GeomXYZ[i].Y);
         else
                 Y = atof(GeomXYZ[i].Y);
         if(!test(GeomXYZ[i].Z))
                 Z = get_value_variableXYZ(GeomXYZ[i].Z);
         else
                 Z = atof(GeomXYZ[i].Z);
         if(Units==0)
         {
              X *= BOHR_TO_ANG;
              Y *= BOHR_TO_ANG;
              Z *= BOHR_TO_ANG;
         }
	save_atom_pdb_file(fd,"ATOM",i+1,GeomXYZ[i].pdbType,GeomXYZ[i].Residue,
		GeomXYZ[i].ResidueNumber+1, X,Y,Z,
		1.0, 300.0, GeomXYZ[i].Symb, atof(GeomXYZ[i].Charge));
   }
  connection = g_malloc(NcentersXYZ*sizeof(gint));
  for(i=0;i<NcentersXYZ;i++)
  {
	N = 0;
	for(j=0;j<NcentersXYZ;j++)
	{
		if(i==j) continue;
		if(GeomXYZ[i].typeConnections && GeomXYZ[i].typeConnections[j]>0)
		{
			connection[N] = j+1;
			N++;
		}
	}
	if(N>0)
	{
		gint len = 0;
		fprintf(fd,"CONNECT %4d ",i+1);
		len +=  6 + 1 +4;
		fprintf(fd,"%4d",connection[0]);
		len +=  4;
		for(j=1;j<(guint)N;j++)
		{
			fprintf(fd,"%5d",connection[j]);
			len += 5;
		}
		if(len<72) for(j=1;j<72-len;j++) fprintf(fd," ");

		fprintf(fd,"\n");
	}
  }
  fprintf(fd,"MASTER        0    0    0    0    0    0    0    0 ");
  fprintf(fd,"%4d    0 %4d    0\n",NcentersXYZ,NcentersXYZ);          
  fprintf(fd,"END\n");
  fclose(fd);
  g_free(connection);

  datafile = get_name_file(FileName);
  temp = get_suffix_name_file(FileName);
  projectname = get_name_file(temp);
  localdir = get_name_dir(temp);
  if(lastdirectory)
	g_free(lastdirectory);
  lastdirectory = g_strdup(localdir);
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_PDB],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_PDB, NULL, defaultNetWorkProtocol);
  g_free(temp);
  g_free(datafile);
  g_free(projectname);
  g_free(localdir);
  return TRUE;
}
/************************************************************************************/
void save_geometry_pdb_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	 save_pdb_file(FileName);
}

/********************************************************************************/
void save_pdb_file_entry(GtkWidget* entry)
{
	G_CONST_RETURN gchar *FileName;

	FileName = gtk_entry_get_text(GTK_ENTRY(entry));
	if ((!FileName) || (strcmp(FileName,"") == 0))
		return ;
	 save_pdb_file(FileName);
}
/********************************************************************************/
void get_charges_from_gamess_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


	/* printf("NAtoms = %d\n",N);*/
	for(i=0;i<N;i++) GeomXYZ[i].Charge = g_strdup("0.0");

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		if(!fgets(t,taille,fd)) break;
    		pdest = strstr( t, "TOTAL MULLIKEN AND LOWDIN ATOMIC POPULATIONS");

		if(pdest)
		{
    			if(!fgets(t,taille,fd)) break;
			for(i=0;i<N;i++)
			{
    				if(!fgets(t,taille,fd)) break;
				if(sscanf(t,"%s %s %s %s %s %s",dump, dump ,dump, dump, dump, d)==6)
				{
					g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup(d);
				}
				else break;
			}
			break;
		}
	}
}
/********************************************************************************/
gboolean get_npa_charges_from_turbomole_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


	/* printf("NAtoms = %d\n",N);*/

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		if(!fgets(t,taille,fd)) break;
    		pdest = strstr( t, "Summary of Natural Population Analysis:");

		if(pdest)
		{
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd)) break;
				if(strstr(t,"Atom") && strstr(t,"Charge") && strstr(t,"Core")) break;
			}
			if(!(strstr(t,"Atom") && strstr(t,"Charge") && strstr(t,"Core"))) return FALSE;
    			if(!fgets(t,taille,fd)) break;
			for(i=0;i<N;i++)
			{
    				if(!fgets(t,taille,fd)) break;
				if(sscanf(t,"%s %s %s",dump, dump, d)==3)
				{
					g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup(d);
				}
				else return FALSE;
			}
			return TRUE;
		}
	}
	return FALSE;
}
/********************************************************************************/
gboolean get_fit_charges_from_turbomole_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


	/* printf("NAtoms = %d\n",N);*/

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		if(!fgets(t,taille,fd)) break;
    		pdest = strstr( t, "charges resulting from fit:");

		if(pdest)
		{
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd)) break;
				if(strstr(t,"atom") && strstr(t,"radius/au") && strstr(t,"charge")) break;
			}
			if(!(strstr(t,"atom") && strstr(t,"radius/au") && strstr(t,"charge"))) return FALSE;
    			if(!fgets(t,taille,fd)) break;
			for(i=0;i<N;i++)
			{
    				if(!fgets(t,taille,fd)) break;
				if(sscanf(t,"%s %s %s %s",dump, dump, dump, d)==4)
				{
					g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup(d);
				}
				else return FALSE;
			}
			return TRUE;
		}
	}
	return FALSE;
}
/********************************************************************************/
void get_charges_from_turbomole_output_file(FILE* fd,gint N)
{
	gint i;
	long long int pos = ftell(fd);
	for(i=0;i<N;i++) GeomXYZ[i].Charge = g_strdup("0.0");
	if(get_fit_charges_from_turbomole_output_file(fd,N)) return;
	fseek(fd, pos, SEEK_SET);
	if(get_npa_charges_from_turbomole_output_file(fd,N)) return;
	
}
/********************************************************************************/
void get_charges_from_gaussian_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;
	gint ngrad=0;


	/* fprintf(stderr,"I try to read charges from gaussian output file\n");*/
	for(i=0;i<N;i++)
		GeomXYZ[i].Charge = g_strdup("0.0");

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Total atomic charges");
		if(!pdest) /* Gaussian 03 */
    			pdest = strstr( t, "atomic charges");
		if(!pdest) /* Gaussian 09D */
    			pdest = strstr( t, "Mulliken charges and spin densities:");
		if(!pdest) /* Gaussian 09D */
    			pdest = strstr( t, "Mulliken charges:");

		if(pdest)
		{
    			if(!feof(fd))
    				{ char* e = fgets(t,taille,fd);}
			else
				break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd))
    					{ char* e = fgets(t,taille,fd);}
				else
					break;
				if(sscanf(t,"%s %s %s",dump,dump,d)==3)
				{
					g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup(d);
				}
			}
			break;
		}
		else
		{
          		pdest = strstr( t, "GradGradGrad" );
			if(pdest)
			{
				ngrad++;
			}
			if(ngrad>2)
				break;
		}

	}
}
/********************************************************************************/
void get_natural_charges_from_gaussian_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;
	gint ngrad =0;



  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Summary of Natural Population Analysis:");
		if(!pdest) /* Gaussian 03 */
    			pdest = strstr( t, "Summary of Natural Population Analysis:");

		if(pdest)
		{
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;
			if(!strstr(t,"Natural Population"))break;
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;
			if(!strstr(t,"Natural"))break;
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;
			if(!strstr(t,"Charge"))break;
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;
			if(!strstr(t,"-------------"))break;

			for(i=0;i<N;i++)
			{
				if(GeomXYZ[i].Charge) g_free(GeomXYZ[i].Charge);
				GeomXYZ[i].Charge = g_strdup("0.0");
			}

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else break;
				if(sscanf(t,"%s %s %s",dump,dump,d)==3)
				{
					g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup(d);
				}
			}
			break;
		}
		else
		{
          		pdest = strstr( t, "GradGradGrad" );
			if(pdest)
			{
				ngrad++;
			}
			if(ngrad>2)
				break;
		}

	}
}
/********************************************************************************/
void get_esp_charges_from_gaussian_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;
	gint ngrad=0;


  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Charges from ESP fit");
		if(!pdest) /* Gaussian 03 */
    			pdest = strstr( t, "harges from ESP");

		if(pdest)
		{
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;
    			if(!feof(fd)) { char* e = fgets(t,taille,fd);}
			else break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else break;
				if(sscanf(t,"%s %s %s",dump,dump,d)==3)
				{
					g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup(d);
				}
			}
			break;
		}
		else
		{
          		pdest = strstr( t, "GradGradGrad" );
			if(pdest)
			{
				ngrad++;
			}
			if(ngrad>2)
				break;
		}

	}
}
/********************************************************************************/
static void read_molden_gabedit_geom_conv_file(gchar* fileName, gint geometryNumber, GabEditTypeFile type)
{
	gchar t[BSIZE];
	gchar AtomCoord[5][BSIZE];
	FILE *fd;
	guint taille=BSIZE;
	gint i;
	gint l;
	gint numgeom;
	gchar *pdest;
	gint nn;

	fd = FOpen(fileName, "rb");
	if(fd ==NULL)
	{
		sprintf(t,"Sorry\nI can not open %s  file ",fileName);
		MessageGeom(t," Error ",TRUE);
		return;
	}
	numgeom =0;
	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
		pdest = strstr( t, "[GEOMETRIES]");
		if(pdest && strstr(t,"ZMAT"))
		{
			if(type == GABEDIT_TYPEFILE_MOLDEN)
				sprintf(t,_("Sorry\nMolden file with ZMAT coordinate is not supported by Gabedit"));
			if(type == GABEDIT_TYPEFILE_GABEDIT)
				sprintf(t,_("Sorry\nGabedit file with ZMAT coordinate is not supported by Gabedit"));

			MessageGeom(t,_("Error"),TRUE);
			return;
		}	
 		if (pdest)
		{
			while(!feof(fd))
			{
				if(!fgets(t,taille,fd))break;

				str_delete_n(t);
				delete_last_spaces(t);
				delete_first_spaces(t);
				if(!isInteger(t))break;
               			numgeom++;
				if(numgeom == geometryNumber)
				{
					nn = atoi(t);
					if(nn<1)break;
					GeomXYZ=g_malloc(nn*sizeof(GeomXYZAtomDef));
  					for(i=0; i<nn; i++) GeomXYZ[i].typeConnections = NULL;
					if(!fgets(t,taille,fd))break;
					for(i=0; i<nn; i++)
					{
						if(!fgets(t,taille,fd))break;
    						sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
						AtomCoord[0][0]=toupper(AtomCoord[0][0]);
						l=strlen(AtomCoord[0]);
						if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

						GeomXYZ[i].Nentry=NUMBER_LIST_XYZ;
						GeomXYZ[i].Symb=g_strdup(AtomCoord[0]);
						GeomXYZ[i].mmType=g_strdup(GeomXYZ[i].Symb);
						GeomXYZ[i].pdbType=g_strdup(GeomXYZ[i].Symb);
						GeomXYZ[i].Residue=g_strdup(GeomXYZ[i].Symb);
						GeomXYZ[i].ResidueNumber=0;
						if(Units == 0 )
						{
							GeomXYZ[i].X=g_strdup(ang_to_bohr(AtomCoord[1]));
							GeomXYZ[i].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
							GeomXYZ[i].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
						}
						else
						{
							GeomXYZ[i].X=g_strdup(AtomCoord[1]);
							GeomXYZ[i].Y=g_strdup(AtomCoord[2]);
							GeomXYZ[i].Z=g_strdup(AtomCoord[3]);
						}
						GeomXYZ[i].Charge=g_strdup("0.0");
						GeomXYZ[i].Layer=g_strdup(" ");
					}
 					NcentersXYZ = nn;
					break;
				}
				else
				{
					nn = atoi(t);
					if(!fgets(t,taille,fd)) break;
					for(i=0; i<nn; i++)
						if(!fgets(t,taille,fd))break;
				}
			}
		}
	}

	fclose(fd);
 	calculMMTypes(FALSE);
	if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	{
   		clearList(list);
		append_list();
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomDrawingArea != NULL)
		rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS && GeomIsOpen)
		set_spin_of_electrons();
}
/********************************************************************************/
gint get_connections_one_atom_gabedit(gchar* t, gint nAtoms, gint ibeg, gint* connections)
{
	gint k;
	gint nc;
	gint nj;
	gchar** split = NULL;
	gint nA = 0;
	/* gint ibeg = 12;*/
	for(k=0;k<nAtoms;k++) connections[k] = 0;
	split = gab_split(t);
	nA = 0;
	while(split && split[nA]!=NULL) nA++;
	if(nA<ibeg)
	{
		g_strfreev(split);
		return 0;
	}
	nc = atoi(split[ibeg-1]);
	for(k=0;k<2*nc;k+=2) 
	{
		if(!split[ibeg+k]) break;
		if(!split[ibeg+k+1]) break;
		nj = atoi(split[ibeg+k]);
		connections[nj-1] = atoi(split[ibeg+k+1]);
	}

	g_strfreev(split);

	return 1;
}
/********************************************************************************/
static gint read_gabedit_geoms_file(gchar* fileName, gint geometryNumber)
{
#define SZ 50
	gboolean OK;
	gchar t[BSIZE];
	gchar symbol[SZ];
	gchar mmType[SZ];
	gchar pdbType[SZ];
	gchar residueName[SZ];
	gchar dum[SZ];
	gdouble X,Y,Z;
	gdouble charge;
	gint layer;
	FILE *fd;
	guint taille=BSIZE;
	gint i;
	gint j;
	gint k;
	gchar *pdest = NULL;
	gint nGeometries = 0;
	gint nLabels = 0;
	gint nAtoms=0;
	gint l;
	gint nc = 0;
	gint Nvar = 0;
	gint form = 1;

	fd = FOpen(fileName, "rb");
	if(fd == NULL) 
	{
		sprintf(t,_("Sorry\nI can not open %s  file "),fileName);
		MessageGeom(t,_("Error"),TRUE);
		return 1;
	}
	OK=FALSE;
	Nvar = 0;
	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
		pdest = strstr( t, "[GEOMS]");
 		if (pdest)
		{
			if(sscanf(t,"%s %d",dum,&form)!=2) form = 0;
			if(!fgets(t,taille,fd))break;
			sscanf(t,"%d %d",&nGeometries, &nLabels);
			if(nGeometries<geometryNumber)break;
			for(i=0;i<nLabels;i++)
				if(!fgets(t,taille,fd))break;
			if(i!=nLabels) break;
			for(j=0;j<geometryNumber-1;j++)
			{
				for(i=0;i<nLabels;i++) if(!fgets(t,taille,fd))break;
				if(i!=nLabels) break;
				if(!fgets(t,taille,fd))break;
				sscanf(t,"%d",&nAtoms);
				if(nAtoms<1)break;
				for(i=0; i<nAtoms; i++)
					if(!fgets(t,taille,fd))break;
				if(i!=nAtoms) break;
			}
			if(j!=geometryNumber-1) break;
			for(i=0;i<nLabels;i++) if(!fgets(t,taille,fd))break;
			if(i!=nLabels) break;
			if(!fgets(t,taille,fd))break;
			sscanf(t,"%d %d %d",&nAtoms, &TotalCharges[0], &SpinMultiplicities[0]);
			if(nAtoms<1)break;

			if(nAtoms>0) 
			{
				GeomXYZ=g_malloc(nAtoms*sizeof(GeomXYZAtomDef));
				for(i=0; i<nAtoms; i++) GeomXYZ[i].typeConnections = g_malloc(nAtoms*sizeof(gint));
			}
			else GeomXYZ= NULL;
			for(i=0; i<nAtoms; i++)
			{
				GeomXYZ[i].Symb=NULL;
				GeomXYZ[i].mmType=NULL;
				GeomXYZ[i].pdbType=NULL;
				GeomXYZ[i].Residue=NULL;
				GeomXYZ[i].Charge=NULL;
				GeomXYZ[i].Layer=NULL;
			}
			nc = 0;
			for(i=0; i<nAtoms; i++)
			{
				int variable = 0;
				gint ibeg = 12;
				if(!fgets(t,taille,fd))break;
				if(form==0)
				{
    					k = sscanf(t,"%s %s %s %s %d %lf %d %lf %lf %lf",
						symbol,mmType,pdbType,residueName, 
						&GeomXYZ[i].ResidueNumber,
						&charge,&layer,&X,&Y,&Z);
					k++;
					variable = TRUE;
					ibeg = 11;
				}
				else
    				k = sscanf(t,"%s %s %s %s %d %lf %d %d %lf %lf %lf",
						symbol,mmType,pdbType,residueName, 
						&GeomXYZ[i].ResidueNumber,
						&charge,&layer,&variable,&X,&Y,&Z);
				if(k!=11) 
				{
					for(j=0; j<i; j++)
					{
					   if(GeomXYZ[i].Symb) g_free(GeomXYZ[i].Symb);	
					   if(GeomXYZ[i].mmType) g_free(GeomXYZ[i].mmType);	
					   if(GeomXYZ[i].pdbType) g_free(GeomXYZ[i].pdbType);	
					   if(GeomXYZ[i].Residue) g_free(GeomXYZ[i].Residue);	
					   if(GeomXYZ[i].Charge) g_free(GeomXYZ[i].Charge);	
					   if(GeomXYZ[i].Layer) g_free(GeomXYZ[i].Layer);	
					}
					for(i=0; i<nAtoms; i++) if(GeomXYZ[i].typeConnections)g_free(GeomXYZ[i].typeConnections);
					if(GeomXYZ) g_free(GeomXYZ);
  					if(VariablesXYZ) g_free(VariablesXYZ);
 					NcentersXYZ = 0;
					NVariablesXYZ = 0;
					fclose(fd);
					if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ) clearList(list);
					return -1;
				}
				symbol[0]=toupper(symbol[0]);
				l=strlen(symbol);
				if (l==2) symbol[1]=tolower(symbol[1]);

				GeomXYZ[i].Nentry=NUMBER_LIST_XYZ;
				GeomXYZ[i].Symb=g_strdup(symbol);
				GeomXYZ[i].mmType=g_strdup(mmType);
				GeomXYZ[i].pdbType=g_strdup(pdbType);
				GeomXYZ[i].Residue=g_strdup(residueName);
				if(Units == 0 )
				{
					GeomXYZ[i].X=g_strdup_printf("%f",ANG_TO_BOHR*X);
					GeomXYZ[i].Y=g_strdup_printf("%f",ANG_TO_BOHR*Y);
					GeomXYZ[i].Z=g_strdup_printf("%f",ANG_TO_BOHR*Z);
				}
				else
				{
					GeomXYZ[i].X=g_strdup_printf("%f",X);
					GeomXYZ[i].Y=g_strdup_printf("%f",Y);
					GeomXYZ[i].Z=g_strdup_printf("%f",Z);
				}
				if(variable!=0)
				{
					Nvar +=3;
  					if(Nvar==3) VariablesXYZ = g_malloc(Nvar*sizeof(VariablesXYZDef));
  					else VariablesXYZ = g_realloc(VariablesXYZ, Nvar*sizeof(VariablesXYZDef));
					trans_coordXYZ_geom('X', GeomXYZ, i, VariablesXYZ,  Nvar-3);
					trans_coordXYZ_geom('Y', GeomXYZ, i, VariablesXYZ,  Nvar-2);
					trans_coordXYZ_geom('Z', GeomXYZ, i, VariablesXYZ,  Nvar-1);
				}
				GeomXYZ[i].Charge=g_strdup_printf("%f",charge);
				if(layer==0) GeomXYZ[i].Layer=g_strdup("Low");
				if(layer==1) GeomXYZ[i].Layer=g_strdup("Medium");
				if(layer==2) GeomXYZ[i].Layer=g_strdup(" ");
    				nc += get_connections_one_atom_gabedit(t, nAtoms, ibeg, GeomXYZ[i].typeConnections);
			}
			NVariablesXYZ =Nvar;
 			NcentersXYZ = nAtoms;
			OK = TRUE;
			break;
		}
	}

	fclose(fd);
	/* printf("nAtoms = %d\n",nAtoms);*/
	if(!OK) 
	{
		if(GeomXYZ)
		for(i=0; i<nAtoms; i++) if(GeomXYZ[i].typeConnections)g_free(GeomXYZ[i].typeConnections);
		return 3;
	}
	if(nc<1 && GeomXYZ)
		for(i=0; i<nAtoms; i++) 
		{
			if(GeomXYZ[i].typeConnections)g_free(GeomXYZ[i].typeConnections);
			GeomXYZ[i].typeConnections = NULL;
		}
	if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	{
   		clearList(list);
		append_list();
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomDrawingArea != NULL) 
	{
		rafresh_drawing();
	}
	if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
	return 0;
#undef SZ
}
/********************************************************************************/
void read_geom_from_molden_geom_conv_file(gchar *fileName, gint geometryNumber)
{
	read_molden_gabedit_geom_conv_file(fileName, geometryNumber, GABEDIT_TYPEFILE_MOLDEN);

}
/********************************************************************************/
void read_geom_from_gabedit_geom_conv_file(gchar *fileName, gint geometryNumber)
{
	gint res = read_gabedit_geoms_file(fileName,  geometryNumber);
	/* printf("geometryNumber=%d res = %d\n",geometryNumber, res);*/
	if(res == 1) return;
	if(res == 0) return;
	if(res == -1) return;
	read_molden_gabedit_geom_conv_file(fileName, geometryNumber, GABEDIT_TYPEFILE_GABEDIT);
}
/********************************************************************************/
void get_charges_from_mpqc_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
	gint i;
	gboolean OK = FALSE;


	for(i=0;i<N;i++) GeomXYZ[i].Charge = g_strdup("0.0");

	fseek(fd, 0L, SEEK_SET);
	OK = FALSE;
  	while(!feof(fd) )
	{
    		if(!fgets(t,taille,fd))break;
    		if(strstr(t, "Natural Population Analysis"))
		{
			OK = TRUE;
			break;
		}
	}
	if(!OK) return;

	OK = FALSE;
  	while(!feof(fd) )
	{
    		if(!fgets(t,taille,fd))break;
    		if(strstr(t, "atom") && strstr(t, "charge"))
		{
			OK = TRUE;
			break;
		}
	}
	if(!OK) return;
	for(i=0;i<N && !feof(fd) ;i++)
	{
		if(!fgets(t,taille,fd)) break;
		if(sscanf(t,"%s %s %s",dump,dump,d)==3)
		{
			g_free(GeomXYZ[i].Charge);
			GeomXYZ[i].Charge = g_strdup(d);
		}
	}
	if(i==N) return;
	for(i=0;i<N;i++)
	{
		g_free(GeomXYZ[i].Charge);
		GeomXYZ[i].Charge = g_strdup("0.0");
	}

	return;
}
/********************************************************************************/
void read_geom_from_mpqc_output_file(gchar *fileName, gint numGeometry)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint idummy;
	guint i;
	gint j=0;
	gint l;
	guint numGeom;
	gdouble tmp;


	t=g_malloc(taille);
	fd = FOpen(fileName, "rb");
	if(fd ==NULL)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),fileName);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));

	numGeom = 0;
	do 
	{
		gboolean unitOfOutAng = FALSE;
		OK=FALSE;
		while(!feof(fd))
		{
			if(!fgets(t,taille,fd)) break;
			if (strstr(t,"<Molecule>"))
			{
				gboolean OkUnit = FALSE;
				do{
		 			if(!fgets(t,taille,fd))break;
					if(strstr(t,"unit"))
					{
						OkUnit = TRUE;
						if(strstr(t,"angstrom"))unitOfOutAng=TRUE;
						break;
					};
					
				}while(!feof(fd));
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
	 		MessageGeom(t,_("Error"),TRUE);
	 		g_free(t);
			for(i=0;i<5;i++) g_free(AtomCoord[i]);
	 		return;
		}
		if(!OK) break;
		OK = FALSE;
		while(!feof(fd) )
		{
	   		if(!fgets(t,taille,fd))break;
			if ( !(strstr(t,"atoms") && strstr(t,"geometry"))) continue;
			OK = TRUE;
			break;
		}
		if(!OK)
		{
	 		g_free(t);
	 		t = g_strdup_printf(_("Sorry\nI can read Geometry from %s  file "),fileName);
	 		MessageGeom(t,_("Error"),TRUE);
	 		g_free(t);
			for(i=0;i<5;i++) g_free(AtomCoord[i]);
	 		return;
		}

		j=-1;
		while(!feof(fd) )
		{
	   		if(!fgets(t,taille,fd))break;
	   		if(strstr(t,"}"))break;
			j++;

			if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
			else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
  			GeomXYZ[j].typeConnections = NULL;

			for(i=0;i<strlen(t);i++) if(t[i]=='[' || t[i] ==']') t[i]=' ';
			sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
			for(i=1;i<=3;i++)
			{
				tmp = atof(AtomCoord[i]);
				sprintf(AtomCoord[i],"%f",tmp);
			}

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);

			l=strlen(AtomCoord[0]);

			if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

			GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
			GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);
			GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
			GeomXYZ[j].ResidueNumber=0;

			if(!unitOfOutAng)
			{
				if(Units == 1)
				{
					GeomXYZ[j].X=g_strdup(bohr_to_ang(AtomCoord[1]));
					GeomXYZ[j].Y=g_strdup(bohr_to_ang(AtomCoord[2]));
					GeomXYZ[j].Z=g_strdup(bohr_to_ang(AtomCoord[3]));
				}
				else
				{
					GeomXYZ[j].X=g_strdup(AtomCoord[1]);
					GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
					GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
				}
			}
			else
			{
				if(Units != 1)
				{
					GeomXYZ[j].X=g_strdup(bohr_to_ang(AtomCoord[1]));
					GeomXYZ[j].Y=g_strdup(bohr_to_ang(AtomCoord[2]));
					GeomXYZ[j].Z=g_strdup(bohr_to_ang(AtomCoord[3]));
				}
				else
				{
					GeomXYZ[j].X=g_strdup(AtomCoord[1]);
					GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
					GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
				}
			}

			GeomXYZ[j].Charge=g_strdup("0.0");
			GeomXYZ[j].Layer=g_strdup(" ");
		}

		NcentersXYZ = j+1;
		if(numGeometry<0) continue;
		if(OK) break;
	}while(!feof(fd));


	if(numGeometry<0) get_charges_from_mpqc_output_file(fd,NcentersXYZ);

	fclose(fd);
 	calculMMTypes(FALSE);
	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);

	if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	{
   		clearList(list);
		append_list();
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void read_first_mpqc_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_mpqc_output_file(fileName,1);
}
/********************************************************************************/
void read_last_mpqc_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_mpqc_output_file(fileName,-1);
}
/********************************************************************************/
void read_geom_from_wfx_file(gchar *fileName)
{
	gint k;
	gint j;
	gchar t[BSIZE];
	gchar* C[3] ={ NULL, NULL,NULL};
	gchar** symbols = NULL;
	gdouble* coordinates = NULL;
	gint nSymbols,nCoordinates;
	FILE* file = FOpen(fileName, "rb");
	if(file ==NULL)
	{
		sprintf(t,"Sorry\nI can not open %s  file ",fileName);
		MessageGeom(t," Error ",TRUE);
		return;
	}
	symbols = get_one_block_from_wfx_file(file,"Nuclear Name",&nSymbols);
	//fprintf(stderr,"nSymb = %d\n",nSymbols);
	if(!symbols || nSymbols<1)
	{
		sprintf(t,"Sorry\nI cannot read symbol from %s  file ",fileName);
		MessageGeom(t," Error ",TRUE);
		fclose(file);
		if(symbols) free_one_string_table(symbols, nSymbols);
		return;
	}
	rewind(file);
	coordinates = get_one_block_real_from_wfx_file(file,"Nuclear Cartesian Coordinates",&nCoordinates);
	//fprintf(stderr,"nC = %d\n",nCoordinates);
	if(!coordinates || nCoordinates<3)
	{
		sprintf(t,"Sorry\nI cannot read coordinates from %s  file ",fileName);
		MessageGeom(t," Error ",TRUE);
		fclose(file);
		if(symbols) free_one_string_table(symbols, nSymbols);
		if(coordinates) g_free(coordinates);
		return;
	}
	if(nCoordinates!=3*nSymbols)
	{
		sprintf(t,"Sorry\nI cannot read geometry from %s  file",fileName);
		MessageGeom(t," Error ",TRUE);
		fclose(file);
		if(symbols) free_one_string_table(symbols, nSymbols);
		if(coordinates) g_free(coordinates);
		return;
	}
	if(GeomXYZ==NULL) GeomXYZ=g_malloc(nSymbols*sizeof(GeomXYZAtomDef));
	else GeomXYZ=g_realloc(GeomXYZ,nSymbols*sizeof(GeomXYZAtomDef));
	for(k=0;k<3;k++) C[k] = g_malloc(BSIZE*sizeof(gchar));
	for(j=0;j<nSymbols;j++)
	{
		gint l;
   		GeomXYZ[j].typeConnections = NULL;
		for(k=0;k<3;k++) sprintf(C[k],"%0.10e",coordinates[3*j+k]);
		//fprintf(stderr,"%s %s %s\n", C[0], C[1], C[2]); 

		for(k=0;k<(gint)strlen(symbols[j]);k++) if(isdigit(symbols[j][k])) symbols[j][k] = ' ';
		delete_all_spaces(symbols[j]);

		symbols[j][0]=toupper(symbols[j][0]);
		l=strlen(symbols[j]); 
		if (l==2) symbols[j][1]=tolower(symbols[j][1]);
		if(l==1)sprintf(t,"%c",symbols[j][0]);
		else sprintf(t,"%c%c",symbols[j][0],symbols[j][1]);
		//fprintf(stderr,"t=%s\n", t);

		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(t);
		GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].ResidueNumber=0;
		if(Units == 0 )
		{
			GeomXYZ[j].X=g_strdup(C[0]);
			GeomXYZ[j].Y=g_strdup(C[1]);
			GeomXYZ[j].Z=g_strdup(C[2]);
		}
		else
		{
			GeomXYZ[j].X=g_strdup(bohr_to_ang(C[0]));
			GeomXYZ[j].Y=g_strdup(bohr_to_ang(C[1]));
			GeomXYZ[j].Z=g_strdup(bohr_to_ang(C[2]));
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
		//fprintf(stderr,"%s %s %s %s\n", GeomXYZ[j].Symb, GeomXYZ[j].X, GeomXYZ[j].Y, GeomXYZ[j].Z);
	}
	NcentersXYZ = nSymbols;
	for(k=0;k<3;k++) if(C[k]) g_free(C[k]);
	fclose(file);
	if(symbols) free_one_string_table(symbols, nSymbols);
	if(coordinates) g_free(coordinates);
 	calculMMTypes(FALSE);
	if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	{
		clearList(list);
		append_list();
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void read_wfx_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_wfx_file(NomFichier);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_first_vasp_xml_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_vasp_xml_file(fileName,1);
}
/********************************************************************************/
void read_last_vasp_xml_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_vasp_xml_file(fileName,-1);
}
/********************************************************************************/
void read_geom_conv_from_dalton_output_file(gchar *NomFichier, gint numgeometry)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j=0;
	gint l;
	guint numgeom;
	gchar dum[100];
	gint kk;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
	t=g_malloc(taille);
	fd = FOpen(NomFichier, "rb");
	if(fd ==NULL)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	numgeom = 0;
	do 
	{
		OK=FALSE;
		while(!feof(fd)){
    			{ char* e = fgets(t,taille,fd);}
			if(strstr(t,"Next geometry") || strstr(t,"Final geometry"))
			{
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
 				numgeom++;
				if((gint)numgeom == numgeometry ) { OK = TRUE; break; }
				if(numgeometry<0 ) { OK = TRUE; break; }
	  		}
		}
		if(!OK && (numgeom == 0) ){
			g_free(t);
			t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
			MessageGeom(t,_("Error"),TRUE);
			g_free(t);
			return;
		}
		if(!OK)break;

		j=-1;
		while(!feof(fd) )
		{
    			{ char* e = fgets(t,taille,fd);}
			if ( !strcmp(t,"\n"))
			{
				get_dipole_from_dalton_output_file(fd);
				break;
			}
			j++;

			if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
			else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
  			GeomXYZ[j].typeConnections = NULL;

			kk = sscanf(t,"%s %s %s %s %s",AtomCoord[0],AtomCoord[1], AtomCoord[2],AtomCoord[3], dum);
			if(kk==5) sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);
			{
				gint k;
				for(k=0;k<(gint)strlen(AtomCoord[0]);k++) if(isdigit(AtomCoord[0][k])) AtomCoord[0][k] = ' ';
				delete_all_spaces(AtomCoord[0]);
			}

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
			l=strlen(AtomCoord[0]);
			if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
			GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);
			GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
			GeomXYZ[j].ResidueNumber=0;
			if(Units == 1 )
			{
				GeomXYZ[j].X=g_strdup(bohr_to_ang(AtomCoord[1]));
				GeomXYZ[j].Y=g_strdup(bohr_to_ang(AtomCoord[2]));
				GeomXYZ[j].Z=g_strdup(bohr_to_ang(AtomCoord[3]));
			}
			else
			{
				GeomXYZ[j].X=g_strdup(AtomCoord[1]);
				GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
				GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
			}
			GeomXYZ[j].Charge=g_strdup("0.0");
			GeomXYZ[j].Layer=g_strdup(" ");
		}

		NcentersXYZ = j+1;
		if(OK && numgeometry>=0) break;
	}while(!feof(fd));

	fclose(fd);
 	calculMMTypes(FALSE);
	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	{
   		clearList(list);
		append_list();
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void read_geom_from_dalton_output_file(gchar *NomFichier, gint numgeometry)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j=0;
	gint l;
	guint numgeom;
	gchar dum[100];
	gint kk;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
	t=g_malloc(taille);
	fd = FOpen(NomFichier, "rb");
	if(fd ==NULL)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	numgeom = 0;
	do 
	{
		OK=FALSE;
		while(!feof(fd)){
    			{ char* e = fgets(t,taille,fd);}
			if ( strstr(t,"geometry (au)"))
			{
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
 				numgeom++;
				if((gint)numgeom == numgeometry ) { OK = TRUE; break; }
				if(numgeometry<0 ) { OK = TRUE; break; }
	  		}
		}
		if(!OK && (numgeom == 0) ){
			g_free(t);
			t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
			MessageGeom(t,_("Error"),TRUE);
			g_free(t);
			return;
		}
		if(!OK)break;

		j=-1;
		while(!feof(fd) )
		{
    			{ char* e = fgets(t,taille,fd);}
			if ( !strcmp(t,"\n"))
			{
				get_dipole_from_dalton_output_file(fd);
				break;
			}
			j++;

			if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
			else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
  			GeomXYZ[j].typeConnections = NULL;

			kk = sscanf(t,"%s %s %s %s %s",AtomCoord[0],AtomCoord[1], AtomCoord[2],AtomCoord[3], dum);
			if(kk==5) sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);
			{
				gint k;
				for(k=0;k<(gint)strlen(AtomCoord[0]);k++) if(isdigit(AtomCoord[0][k])) AtomCoord[0][k] = ' ';
				delete_all_spaces(AtomCoord[0]);
			}

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
			l=strlen(AtomCoord[0]);
			if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
			GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);
			GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
			GeomXYZ[j].ResidueNumber=0;
			if(Units == 1 )
			{
				GeomXYZ[j].X=g_strdup(bohr_to_ang(AtomCoord[1]));
				GeomXYZ[j].Y=g_strdup(bohr_to_ang(AtomCoord[2]));
				GeomXYZ[j].Z=g_strdup(bohr_to_ang(AtomCoord[3]));
			}
			else
			{
				GeomXYZ[j].X=g_strdup(AtomCoord[1]);
				GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
				GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
			}
			GeomXYZ[j].Charge=g_strdup("0.0");
			GeomXYZ[j].Layer=g_strdup(" ");
		}

		NcentersXYZ = j+1;
		if(OK && numgeometry>=0) break;
	}while(!feof(fd));

	fclose(fd);
 	calculMMTypes(FALSE);
	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	{
   		clearList(list);
		append_list();
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void read_first_dalton_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_dalton_output_file(fileName,1);
}
/********************************************************************************/
void read_last_dalton_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_dalton_output_file(fileName,-1);
}
/********************************************************************************/
void read_geom_conv_from_gamess_output_file(gchar *NomFichier, gint numgeometry)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j=0;
	gint l;
	guint numgeom;
	gchar dum[100];
	gint uni=1;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
	t=g_malloc(taille);
	fd = FOpen(NomFichier, "rb");
	if(fd ==NULL)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	numgeom = 0;
	do 
	{
		OK=FALSE;
		while(!feof(fd)){
    			{ char* e = fgets(t,taille,fd);}
			if ( strstr(t,"COORDINATES OF ALL ATOMS ARE (ANGS)"))
			{
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
 				numgeom++;
				uni=1;
				if((gint)numgeom == numgeometry ) { OK = TRUE; break; }
				if(numgeometry<0 ) { OK = TRUE; break; }
	  		}
		}
		if(!OK && (numgeom == 0) ){
			g_free(t);
			t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
			MessageGeom(t,_("Error"),TRUE);
			g_free(t);
			return;
		}
		if(!OK)break;

		j=-1;
		while(!feof(fd) )
		{
    			{ char* e = fgets(t,taille,fd);}
			if ( !strcmp(t,"\n") || !strcmp(t,"\r\n"))
			{
				get_dipole_from_gamess_output_file(fd);
				break;
			}
			j++;

			if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
			else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
  			GeomXYZ[j].typeConnections = NULL;

			sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);
			{
				gint k;
				for(k=0;k<(gint)strlen(AtomCoord[0]);k++) if(isdigit(AtomCoord[0][k])) AtomCoord[0][k] = ' ';
				delete_all_spaces(AtomCoord[0]);
			}

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
			l=strlen(AtomCoord[0]);
			if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
			/* GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);*/
			GeomXYZ[j].Symb=get_symbol_using_z(atoi(dum));
			GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
			GeomXYZ[j].ResidueNumber=0;
			if(Units != uni )
			{
				if(Units==1)
				{
				GeomXYZ[j].X=g_strdup(bohr_to_ang(AtomCoord[1]));
				GeomXYZ[j].Y=g_strdup(bohr_to_ang(AtomCoord[2]));
				GeomXYZ[j].Z=g_strdup(bohr_to_ang(AtomCoord[3]));
				}
				else
				{
				GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
				GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
				GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
				}
			}
			else
			{
				GeomXYZ[j].X=g_strdup(AtomCoord[1]);
				GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
				GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
			}
			GeomXYZ[j].Charge=g_strdup("0.0");
			GeomXYZ[j].Layer=g_strdup(" ");
		}

		NcentersXYZ = j+1;
		if(OK && numgeometry>=0) break;
	}while(!feof(fd));

	fclose(fd);
 	calculMMTypes(FALSE);
	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	{
   		clearList(list);
		append_list();
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void get_charges_from_aimall_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


	/* printf("NAtoms = %d\n",N);*/
	for(i=0;i<N;i++) GeomXYZ[i].Charge = g_strdup("0.0");

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		if(!fgets(t,taille,fd)) break;
    		pdest = strstr( t, "Atom A          q(A)");
		if(pdest)
		{
    			if(!fgets(t,taille,fd)) break;
			for(i=0;i<N;i++)
			{
    				if(!fgets(t,taille,fd)) break;
				if(sscanf(t,"%s %s",dump, d)==2)
				{
					g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup(d);
				}
				else break;
			}
			break;
		}
	}
}
/********************************************************************************/
void read_geom_from_aimall_file(gchar *NomFichier)
{
	gchar *t;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j=0;
	gint l;
	gchar dum[100];
	gint uni=0;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
	t=g_malloc(taille);
	fd = FOpen(NomFichier, "rb");
	if(fd ==NULL)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
		if ( strstr(t,"Atom      Charge                X                  Y                  Z"))
		{
    			{ char* e = fgets(t,taille,fd);}
			break; 
  		}
	}

	j=-1;
	while(!feof(fd) )
	{
    		{ char* e = fgets(t,taille,fd);}
		if(this_is_a_backspace(t))
		{
			get_charges_from_aimall_file(fd,j+1);
			break;
		}
		j++;

		if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
		else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));

  		GeomXYZ[j].typeConnections = NULL;
		sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);
		{
			gint k;
			for(k=0;k<(gint)strlen(AtomCoord[0]);k++) if(isdigit(AtomCoord[0][k])) AtomCoord[0][k] = ' ';
			delete_all_spaces(AtomCoord[0]);
		}
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
		l=strlen(AtomCoord[0]);
		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=get_symbol_using_z(atoi(dum));
		GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
		GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
		GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
		GeomXYZ[j].ResidueNumber=0;
		if(Units != uni )
		{
			if(Units==1)
			{
				GeomXYZ[j].X=g_strdup(bohr_to_ang(AtomCoord[1]));
				GeomXYZ[j].Y=g_strdup(bohr_to_ang(AtomCoord[2]));
				GeomXYZ[j].Z=g_strdup(bohr_to_ang(AtomCoord[3]));
			}
			else
			{
				GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
				GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
				GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
			}
		}
		else
		{
			GeomXYZ[j].X=g_strdup(AtomCoord[1]);
			GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
			GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
	}

	NcentersXYZ = j+1;
	fclose(fd);
 	calculMMTypes(FALSE);
	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	{
   		clearList(list);
		append_list();
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
static void get_charge_and_multilicity(FILE* fd)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];

  	while(!feof(fd) )
	{
    		if(!fgets(t,taille,fd)) break;
		if ( strstr(t,"CHARGE OF MOLECULE") && strstr(t,"="))
		{
			gchar* p = strstr(t,"=")+1;
			TotalCharges[0] = atoi(p);
		}
		if ( strstr(t,"SPIN MULTIPLICITY  ") && strstr(t,"="))
		{
			gchar* p = strstr(t,"=")+1;
			SpinMultiplicities[0] = atoi(p);
			break;
		}
	}
}
/********************************************************************************/
void read_geom_from_gamess_output_file(gchar *NomFichier, gint numgeometry)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j=0;
	gint l;
	guint numgeom;
	gchar dum[100];
	gint uni=1;
 	long geomposok = 0;


	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
	t=g_malloc(taille);
#ifdef G_OS_WIN32 
 	fd = FOpen(NomFichier, "rb");
#else
	fd = FOpen(NomFichier, "rb");
#endif
	if(fd ==NULL)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	numgeom = 0;
	do 
	{
		OK=FALSE;
		while(!feof(fd)){
    			{ char* e = fgets(t,taille,fd);}
			if ( numgeometry==1 && strstr(t,"COORDINATES (BOHR)"))
			{
    				{ char* e = fgets(t,taille,fd);}
 				numgeom++;
				uni=0;
				if((gint)numgeom == numgeometry ) { OK = TRUE; break; }
	  		}
			if ( strstr(t,"COORDINATES OF ALL ATOMS ARE (ANGS)"))
			{
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
 				numgeom++;
				uni=1;
				if((gint)numgeom == numgeometry ) { OK = TRUE; break; }
				if(numgeometry<0 ) { OK = TRUE; break; }
	  		}
		}
		if(!OK && (numgeom == 0) ){
			g_free(t);
			t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
			MessageGeom(t,_("Error"),TRUE);
			g_free(t);
			return;
		}
		if(!OK)break;

		j=-1;
		while(!feof(fd) )
		{
    			{ char* e = fgets(t,taille,fd);}
			if ( !strcmp(t,"\n")) break;
			if ( !strcmp(t,"\r\n")) break;
			j++;

			if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
			else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
  			GeomXYZ[j].typeConnections = NULL;

			sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);
			{
				gint k;
				for(k=0;k<(gint)strlen(AtomCoord[0]);k++) if(isdigit(AtomCoord[0][k])) AtomCoord[0][k] = ' ';
				delete_all_spaces(AtomCoord[0]);
			}

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
			l=strlen(AtomCoord[0]);
			if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
			/* GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);*/
			GeomXYZ[j].Symb=get_symbol_using_z(atoi(dum));
			GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
			GeomXYZ[j].ResidueNumber=0;
			if(Units != uni )
			{
				if(Units==1)
				{
				GeomXYZ[j].X=g_strdup(bohr_to_ang(AtomCoord[1]));
				GeomXYZ[j].Y=g_strdup(bohr_to_ang(AtomCoord[2]));
				GeomXYZ[j].Z=g_strdup(bohr_to_ang(AtomCoord[3]));
				}
				else
				{
				GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
				GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
				GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
				}
			}
			else
			{
				GeomXYZ[j].X=g_strdup(AtomCoord[1]);
				GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
				GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
			}
			GeomXYZ[j].Charge=g_strdup("0.0");
			GeomXYZ[j].Layer=g_strdup(" ");
		}

		NcentersXYZ = j+1;
		if(OK && numgeometry>=0) break;
		if(numgeometry<0) geomposok = ftell(fd);
	}while(!feof(fd));
	if ( NcentersXYZ >0 )
	{
		if(numgeometry<0) fseek(fd, geomposok, SEEK_SET);
		geomposok = ftell(fd);
		get_charges_from_gamess_output_file(fd,NcentersXYZ);
		get_dipole_from_gamess_output_file(fd);
		fseek(fd, geomposok, SEEK_SET);
		get_charge_and_multilicity(fd);
	}

	fclose(fd);
 	calculMMTypes(FALSE);
	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	{
   		clearList(list);
		append_list();
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void read_geom_from_gamess_irc_file(gchar *NomFichier, gint numgeometry)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j=0;
	gint l;
	guint numgeom;
	gchar dum[100];
	gint uni=1;
 	/* long geomposok = 0;*/



	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
	t=g_malloc(taille);
#ifdef G_OS_WIN32 
 	fd = FOpen(NomFichier, "rb");
#else
	fd = FOpen(NomFichier, "rb");
#endif
	if(fd ==NULL)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	numgeom = 0;
	do 
	{
		OK=FALSE;
		while(!feof(fd)){
    			{ char* e = fgets(t,taille,fd);}
			if (strstr(t,"CARTESIAN COORDINATES (BOHR)"))
			{
 				numgeom++;
				uni=0;
				if((gint)numgeom == numgeometry ) { OK = TRUE; break; }
	  		}
		}
		if(!OK && (numgeom == 0) ){
			g_free(t);
			t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
			MessageGeom(t,_("Error"),TRUE);
			g_free(t);
			return;
		}
		if(!OK)break;

		j=-1;
		while(!feof(fd) )
		{
			gdouble rdum = 0;
    			{ char* e = fgets(t,taille,fd);}
			if ( !strcmp(t,"GRADIENT")) break;
			if(2!=sscanf(t,"%s %lf",AtomCoord[0],&rdum)) break;
			j++;

			if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
			else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
  			GeomXYZ[j].typeConnections = NULL;

			sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);
			{
				gint k;
				for(k=0;k<(gint)strlen(AtomCoord[0]);k++) if(isdigit(AtomCoord[0][k])) AtomCoord[0][k] = ' ';
				delete_all_spaces(AtomCoord[0]);
			}

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
			l=strlen(AtomCoord[0]);
			if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
			/* GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);*/
			GeomXYZ[j].Symb=get_symbol_using_z(atoi(dum));
			GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
			GeomXYZ[j].ResidueNumber=0;
			if(Units != uni )
			{
				if(Units==1)
				{
				GeomXYZ[j].X=g_strdup(bohr_to_ang(AtomCoord[1]));
				GeomXYZ[j].Y=g_strdup(bohr_to_ang(AtomCoord[2]));
				GeomXYZ[j].Z=g_strdup(bohr_to_ang(AtomCoord[3]));
				}
				else
				{
				GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
				GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
				GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
				}
			}
			else
			{
				GeomXYZ[j].X=g_strdup(AtomCoord[1]);
				GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
				GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
			}
			GeomXYZ[j].Charge=g_strdup("0.0");
			GeomXYZ[j].Layer=g_strdup(" ");
		}

		NcentersXYZ = j+1;
		if(OK && numgeometry>=0) break;
		// if(numgeometry<0) geomposok = ftell(fd);*/
	}while(!feof(fd));
	if ( NcentersXYZ >0 )
	{
		reset_charges_multiplicities_XYZ();
	}

	fclose(fd);
 	calculMMTypes(FALSE);
	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	{
   		clearList(list);
		append_list();
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void read_first_gamess_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_gamess_output_file(fileName,1);
}
/********************************************************************************/
void read_last_gamess_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_gamess_output_file(fileName,-1);
}
/********************************************************************************/
void read_last_irc_gamess_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_gamess_irc_file(fileName,-1);
}
/********************************************************************************/
void read_aimall_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_aimall_file(fileName);
}
/********************************************************************************/
void read_geom_from_turbomole_output_file(gchar *fileName, gint numgeometry)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j=0;
	gint l;
	guint numgeom;
	gint uni=1;
 	long geomposok = 0;


	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
	t=g_malloc(taille*sizeof(gchar));
 	fd = FOpen(fileName, "rb");
	if(fd ==NULL)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),fileName);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	numgeom = 0;
	do 
	{
		OK=FALSE;
		while(!feof(fd)){
    			{ char* e = fgets(t,taille,fd);}
			if ( strstr(t,"Atomic coordinate, charge and isotop information"))
			{
				while(!feof(fd)){
    				{ char* e = fgets(t,taille,fd);}
				if(strstr(t,"atomic") && strstr(t, "coordinates") && strstr(t,"atom") && strstr(t,"shells") && strstr(t,"charge")) break;
				}
				if(!(strstr(t,"atomic") && strstr(t, "coordinates") && strstr(t,"atom") && strstr(t,"shells") && strstr(t,"charge"))) {OK = FALSE; break;}
 				numgeom++;
				uni=0;
				if((gint)numgeom == numgeometry ) { OK = TRUE; break; }
				if(numgeometry<0 ) { OK = TRUE; break; }
	  		}
		}
		if(!OK && (numgeom == 0) ){
			g_free(t);
			t = g_strdup_printf(_("Sorry\nI can not open %s  file "),fileName);
			MessageGeom(t,_("Error"),TRUE);
			g_free(t);
			return;
		}
		if(!OK)break;

		j=-1;
		while(!feof(fd) )
		{
    			{ char* e = fgets(t,taille,fd);}
			if ( !strcmp(t,"\n")) break;
			if ( !strcmp(t,"\r\n")) break;
			j++;

			if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
			else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
  			GeomXYZ[j].typeConnections = NULL;

			sscanf(t,"%s %s %s %s",AtomCoord[1], AtomCoord[2],AtomCoord[3], AtomCoord[0]);
			{
				gint k;
				for(k=0;k<(gint)strlen(AtomCoord[0]);k++) if(isdigit(AtomCoord[0][k])) AtomCoord[0][k] = ' ';
				delete_all_spaces(AtomCoord[0]);
			}

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
			l=strlen(AtomCoord[0]);
			if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
			GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);
			/* GeomXYZ[j].Symb=get_symbol_using_z(atoi(dum));*/
			GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
			GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
			GeomXYZ[j].ResidueNumber=0;
			if(Units != uni )
			{
				if(Units==1)
				{
				GeomXYZ[j].X=g_strdup(bohr_to_ang(AtomCoord[1]));
				GeomXYZ[j].Y=g_strdup(bohr_to_ang(AtomCoord[2]));
				GeomXYZ[j].Z=g_strdup(bohr_to_ang(AtomCoord[3]));
				}
				else
				{
				GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
				GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
				GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
				}
			}
			else
			{
				GeomXYZ[j].X=g_strdup(AtomCoord[1]);
				GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
				GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
			}
			GeomXYZ[j].Charge=g_strdup("0.0");
			GeomXYZ[j].Layer=g_strdup(" ");
		}

		NcentersXYZ = j+1;
		if(OK && numgeometry>=0) break;
		if(numgeometry<0) geomposok = ftell(fd);
	}while(!feof(fd));
	if ( NcentersXYZ >0 )
	{
		if(numgeometry<0) fseek(fd, geomposok, SEEK_SET);
		geomposok = ftell(fd);
		get_charges_from_turbomole_output_file(fd,NcentersXYZ);
		fseek(fd, geomposok, SEEK_SET);
		get_dipole_from_turbomole_output_file(fd);
		/*
		fseek(fd, geomposok, SEEK_SET);
		get_charge_and_multilicity(fd);
		*/
	}

	fclose(fd);
 	calculMMTypes(FALSE);
	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	{
   		clearList(list);
		append_list();
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void read_first_turbomole_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_turbomole_output_file(fileName,1);
}
/********************************************************************************/
void read_last_turbomole_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_turbomole_output_file(fileName,-1);
}
/********************************************************************************/
void read_geom_from_cif_file(gchar *fileName, gboolean applySymOp)
{
	gint k;
	gchar* error = NULL;
	Crystal crystal;
	gint nAtoms = 0;
	GList* l;

	error = read_geometry_from_cif_file(&crystal, fileName, applySymOp);
        if(!error) for(l = g_list_first(crystal.atoms); l != NULL; l = l->next) nAtoms++;
	if(error || nAtoms<1)
	{
		gchar* t = g_strdup_printf(_("Sorry\nI cannot read the geometry from %s file "),fileName);
		if(error)
		{
			gchar* tmp = t;
			t = g_strdup_printf(_("%s\n%s\n"),tmp, error);
			g_free(tmp);
		}
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		freeCrystal(&crystal);
		return;
	}
	init_dipole();
	if(GeomXYZ) freeGeomXYZ();
	if(VariablesXYZ) freeVariablesXYZ();


	NcentersXYZ = nAtoms;
	GeomXYZ = g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));

	k=0;
	/* fprintf(stderr,"read cif Units=%d\n",Units);*/
        for(l = g_list_first(crystal.atoms); l != NULL; l = l->next)
	{
		CrystalloAtom* a = (CrystalloAtom*)l->data;
		gdouble X,Y,Z;

    		GeomXYZ[k].Nentry=NUMBER_LIST_XYZ;
    		GeomXYZ[k].Symb=g_strdup(a->symbol);
    		GeomXYZ[k].mmType=g_strdup(a->mmType);
    		GeomXYZ[k].pdbType=g_strdup(a->pdbType);
    		GeomXYZ[k].Residue=g_strdup(a->residueName);
    		GeomXYZ[k].ResidueNumber=0;
    		X=a->C[0];
    		Y=a->C[1];
    		Z=a->C[2];
		if(Units==0) { X /= (BOHR_TO_ANG); Y /= (BOHR_TO_ANG); Z /= (BOHR_TO_ANG); }
    		GeomXYZ[k].X=g_strdup_printf("%0.10lf",X);
    		GeomXYZ[k].Y=g_strdup_printf("%0.10lf",Y);
    		GeomXYZ[k].Z=g_strdup_printf("%0.10lf",Z);
    		GeomXYZ[k].Charge=g_strdup_printf("%0.5lf",a->charge);
    		GeomXYZ[k].Layer=g_strdup(" ");
    		GeomXYZ[k].typeConnections=NULL;
		k++;
	}
	reset_origine_molecule_drawgeom();
	//calculMMTypes(TRUE);
	if(GeomIsOpen) create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
	if(GeomDrawingArea != NULL) rafresh_drawing();

	freeCrystal(&crystal);
}
/********************************************************************************/
void read_geom_from_vasp_xml_file(gchar *fileName, gint numgeom)
{
	gchar** symbols = NULL;
	gdouble* positions[3]= { NULL, NULL, NULL};
	gint nAtoms = 0;
	gdouble* X;
	gdouble* Y;
	gdouble* Z;
	gint k;

	nAtoms = read_geometry_vasp_xml_file(fileName, numgeom, &symbols, positions);
	if(nAtoms<1)
	{
		gchar* t = g_strdup_printf(_("Sorry\nI can not the geometry number %d from %s file "),numgeom, fileName);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	init_dipole();
	if(GeomXYZ) freeGeomXYZ();
	if(VariablesXYZ) freeVariablesXYZ();
	NcentersXYZ = nAtoms;
	GeomXYZ = g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));
	X = positions[0];
	Y = positions[1];
	Z = positions[2];
	for(k=0;k<nAtoms;k++)
	{
    		GeomXYZ[k].Nentry=NUMBER_LIST_XYZ;
    		GeomXYZ[k].Symb=g_strdup(symbols[k]);
    		GeomXYZ[k].mmType=g_strdup(symbols[k]);
    		GeomXYZ[k].pdbType=g_strdup(symbols[k]);
    		GeomXYZ[k].Residue=g_strdup(symbols[k]);
    		GeomXYZ[k].ResidueNumber=0;
    		GeomXYZ[k].X=g_strdup_printf("%0.10f",X[k]);
    		GeomXYZ[k].Y=g_strdup_printf("%0.10f",Y[k]);
    		GeomXYZ[k].Z=g_strdup_printf("%0.10f",Z[k]);
    		GeomXYZ[k].Charge=g_strdup("0.0");
    		GeomXYZ[k].Layer=g_strdup(" ");
    		GeomXYZ[k].typeConnections=NULL;
	}
	calculMMTypes(TRUE);
	if(GeomIsOpen) create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
	if(GeomDrawingArea != NULL) rafresh_drawing();

        if(X) g_free(X);
        if(Y) g_free(Y);
        if(Z) g_free(Z);
	for(k=0;k<nAtoms;k++) if(symbols[k]) g_free(symbols[k]);
	if(symbols) g_free(symbols);
}
/********************************************************************************/
void read_geom_from_poscar_file(gchar *fileName)
{
	gchar *t;
	gboolean OK;
	FILE *file;
	gint i;
	gint j;
	gint k;
	gint n = 0;
	gdouble factor = 1.0;
	gdouble TV[3][3];
	gchar** strsplit1 = NULL;
	gchar** strsplit2 = NULL;
	gint nmax = 0;
	gint nmax2 = 0;
	gchar** types = NULL;
	gchar** symbols = NULL;
	gint* ntypes = NULL;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble* Z = NULL;
	gboolean direct = FALSE;
	gint nAtoms = 0;


	file = FOpen(fileName, "rb");
	if(!file)
	{
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),fileName);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	OK = TRUE;
	t=g_malloc(BSIZE*sizeof(gchar));
	if(!fgets(t,BSIZE,file)) OK = FALSE; //comment
	if(OK && !fgets(t,BSIZE,file)) OK = FALSE; //global factor
	if(OK && 1!=sscanf(t,"%lf",&factor)) OK = FALSE;
	if(OK) for(i=0;i<3;i++)
	{
		if(OK && !fgets(t,BSIZE,file)){ OK = FALSE; break;}
		if(OK && 3!=sscanf(t,"%lf %lf %lf",&TV[i][0], &TV[i][1], &TV[i][2])) {OK = FALSE; break;}
	}
	if(OK && fgets(t,BSIZE,file))
	{
		deleteLastSpaces(t);
		strsplit1 = gab_split(t);
		while(strsplit1 && strsplit1[nmax]) nmax++;
		if(nmax>0 && fgets(t,BSIZE,file)) 
		{
			deleteLastSpaces(t);
			strsplit2 =  gab_split(t);
		}
		while(strsplit2 && strsplit2[nmax2]) nmax2++;
		if(nmax<1 || nmax != nmax2)
		{
			if(t)g_free(t);
			t = g_strdup_printf(_("Sorry\nI can not the symbol of atoms from %s file,\nadd these after the cell definition beforee the number of each type"),fileName);
			MessageGeom(t,_("Error"),TRUE);
			g_free(t);
			if(strsplit1) g_strfreev(strsplit1);
			if(strsplit2) g_strfreev(strsplit2);
			return;
		}

	}
	types = g_malloc(nmax*sizeof(gchar*));
	i = 0; while(strsplit1 && strsplit1[i]) { types[i] = g_strdup(strsplit1[i]); i++;}
	ntypes = g_malloc(nmax*sizeof(gint));
	i = 0; while(strsplit2 && strsplit2[i]) { ntypes[i] = atoi(strsplit2[i]); i++;}
	nAtoms = 0;
	//printf("nmax = %d\n",nmax);
	//for(i=0;i<nmax;i++) printf("ntype = %d\n",ntypes[i]);

	for(i=0;i<nmax;i++) nAtoms += ntypes[i];
	if(nAtoms<1) OK = FALSE;
	nAtoms += 3; // TV
	//printf("nAtoms = %d\n",nAtoms);
	
	if(OK) 
	{
		OK = FALSE;
		while(!feof(file))
		{
			if(!fgets(t,BSIZE,file)){ break;}
			delete_last_spaces(t);
			delete_first_spaces(t);
			t[0]=toupper(t[0]);
			if(t[0]=='D') { direct = TRUE; OK = TRUE; break;}
			if(t[0] == 'C') { OK = TRUE; break; }
		}
	}
	//printf("direct = %d\n",direct);
	if(OK)
	{
		X = g_malloc(nAtoms*sizeof(gdouble));
		Y = g_malloc(nAtoms*sizeof(gdouble));
		Z = g_malloc(nAtoms*sizeof(gdouble));
		symbols = g_malloc(nAtoms*sizeof(gchar*));
		for(i=0;i<nAtoms;i++) symbols[i] = NULL;
		k = 0;
		for(i=0;i<nmax;i++)
		for(j=0;j<ntypes[i];j++)
		{
			symbols[k] = g_strdup(types[i]);
			symbols[k][0]=toupper(symbols[k][0]);
          		if (strlen(symbols[k])==2) symbols[k][1]=tolower(symbols[k][1]);
          		if (strlen(symbols[k])==3) symbols[k][2]=tolower(symbols[k][2]);
			if(!fgets(t,BSIZE,file)){ OK = FALSE; break;}
			if(3!=sscanf(t,"%lf %lf %lf",&X[k], &Y[k], &Z[k])) {OK = FALSE; break;}
			k++;
		}
	}
	if(OK && Units!=1) factor *=  ANG_TO_BOHR;
	if(OK) for(k=0;k<3;k++) for(j=0;j<3;j++) TV[k][j] *= factor;
	if(OK&& !direct)
	{
		for(k=0;k<nAtoms-3;k++) X[k] *= factor;
		for(k=0;k<nAtoms-3;k++) Y[k] *= factor;
		for(k=0;k<nAtoms-3;k++) Z[k] *= factor;
	}
	if(OK&& direct)
	{
		gdouble* nX = g_malloc(nAtoms*sizeof(gdouble));
		gdouble* nY = g_malloc(nAtoms*sizeof(gdouble));
		gdouble* nZ = g_malloc(nAtoms*sizeof(gdouble));
		for(k=0;k<nAtoms-3;k++)  nX[k] =  0.0;
		for(k=0;k<nAtoms-3;k++)  nY[k] =  0.0;
		for(k=0;k<nAtoms-3;k++)  nZ[k] =  0.0;

		for(k=0;k<nAtoms-3;k++) 
		{
			while(X[k]<0) X[k]+=1;
			while(Y[k]<0) Y[k]+=1;
			while(Z[k]<0) Z[k]+=1;
			while(X[k]>=1) X[k]-=1;
			while(Y[k]>=1) Y[k]-=1;
			while(Z[k]>=1) Z[k]-=1;
			if(fabs(X[k]-1.0)<1e-10)  X[k]=0;
			if(fabs(Y[k]-1.0)<1e-10)  Y[k]=0;
			if(fabs(Z[k]-1.0)<1e-10)  Z[k]=0;
		}

		for(k=0;k<nAtoms-3;k++) 
		{
			nX[k] = X[k]*TV[0][0]+Y[k]*TV[1][0]+Z[k]*TV[2][0];
			nY[k] = X[k]*TV[0][1]+Y[k]*TV[1][1]+Z[k]*TV[2][1];
			nZ[k] = X[k]*TV[0][2]+Y[k]*TV[1][2]+Z[k]*TV[2][2];
		}
		g_free(X);
		g_free(Y);
		g_free(Z);
		X = nX;
		Y = nY;
		Z = nZ;
	}
	
	if(OK)
	{
		init_dipole();
		if(GeomXYZ) freeGeomXYZ();
		if(VariablesXYZ) freeVariablesXYZ();
		NcentersXYZ = nAtoms;
		GeomXYZ = g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));
		for(k=0;k<nAtoms-3;k++)
		{
    			GeomXYZ[k].Nentry=NUMBER_LIST_XYZ;
    			GeomXYZ[k].Symb=g_strdup(symbols[k]);
    			GeomXYZ[k].mmType=g_strdup(symbols[k]);
    			GeomXYZ[k].pdbType=g_strdup(symbols[k]);
    			GeomXYZ[k].Residue=g_strdup(symbols[k]);
    			GeomXYZ[k].ResidueNumber=0;
    			GeomXYZ[k].X=g_strdup_printf("%0.10f",X[k]);
    			GeomXYZ[k].Y=g_strdup_printf("%0.10f",Y[k]);
    			GeomXYZ[k].Z=g_strdup_printf("%0.10f",Z[k]);
    			GeomXYZ[k].Charge=g_strdup("0.0");
    			GeomXYZ[k].Layer=g_strdup(" ");
    			GeomXYZ[k].typeConnections=NULL;
		}
		for(k=nAtoms-3;k<nAtoms;k++)
		{
    			GeomXYZ[k].Nentry=NUMBER_LIST_XYZ;
    			GeomXYZ[k].Symb=g_strdup("Tv");
    			GeomXYZ[k].mmType=g_strdup("Tv");
    			GeomXYZ[k].pdbType=g_strdup("Tv");
    			GeomXYZ[k].Residue=g_strdup("Tv");
    			GeomXYZ[k].ResidueNumber=0;
    			GeomXYZ[k].X=g_strdup_printf("%0.10f",TV[k-nAtoms+3][0]);
    			GeomXYZ[k].Y=g_strdup_printf("%0.10f",TV[k-nAtoms+3][1]);
    			GeomXYZ[k].Z=g_strdup_printf("%0.10f",TV[k-nAtoms+3][2]);
    			GeomXYZ[k].Charge=g_strdup("0.0");
    			GeomXYZ[k].Layer=g_strdup(" ");
    			GeomXYZ[k].typeConnections=NULL;
		}
	}
	fclose(file);
	g_free(t);
	calculMMTypes(TRUE);
	if(GeomIsOpen) create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
	if(GeomDrawingArea != NULL) rafresh_drawing();

	if(strsplit1) g_strfreev(strsplit1);
	if(strsplit2) g_strfreev(strsplit2);
}
/********************************************************************************/
void read_geom_from_xyz_file(gchar *fileName, gint numGeom)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *file;
	guint taille=BSIZE;
	gint i;
	gint j;
	gint l;
	gint n = 0;


	file = FOpen(fileName, "rb");
	if(!file)
	{
		t = g_strdup_printf(_("Sorry\nI can not open %s  file "),fileName);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	OK = FALSE;
	j = 0;
	t=g_malloc(taille);
	while(!feof(file))
	{
		j++;
		if(j==numGeom){OK = TRUE; break;}
		if(!fgets(t,BSIZE,file))break;
		n=atoi(t);
		if(n<1)break;
		if(!fgets(t,BSIZE,file))break;
		for(i=0;i<n;i++) if(!fgets(t,BSIZE,file))break;
		if(i!=n) break;
	}
	if(!OK)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\n I can not read geometry number %d for %s file"),numGeom,fileName);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	if(OK && fgets(t,BSIZE,file))
	{
		n=atoi(t);
		if(n<1) OK = FALSE;
		if(!fgets(t,BSIZE,file)) OK = FALSE;/* title */
	}
	else OK = FALSE;
	if(OK)
	{
		init_dipole();
		if(GeomXYZ) freeGeomXYZ();
		if(VariablesXYZ) freeVariablesXYZ();
		NcentersXYZ = n;
		GeomXYZ = g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));
		for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
		for(j=0;j<n;j++)
		{
			if(!fgets(t,BSIZE,file)) { OK = FALSE;break;}
    			sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
			delete_first_spaces(AtomCoord[0]);
			delete_last_spaces(AtomCoord[0]);
			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    			GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
    			GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);
    			GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
    			GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
    			GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
    			GeomXYZ[j].ResidueNumber=0;
    			if(Units==1)
    			{
    				GeomXYZ[j].X=g_strdup(AtomCoord[1]);
    				GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
    				GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
    			}
    			else
    			{
    				GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
    				GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
    				GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
    			}
    			GeomXYZ[j].Charge=g_strdup("0.0");
    			GeomXYZ[j].Layer=g_strdup(" ");
    			GeomXYZ[j].typeConnections=NULL;
		}
		if(!OK) 
		{
			NcentersXYZ = 0;
			if(GeomXYZ) freeGeomXYZ();
		}
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
	}
	fclose(file);
	g_free(t);
	calculMMTypes(TRUE);
	if(GeomIsOpen) create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
	if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
void read_geom_from_gaussian_file(gchar *NomFichier, gint numgeometry)
{
 gchar *t;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 guint idummy;
 guint i;
 gint j=0;
 gint l;
 gint numgeom;
 gchar *pdest;
 guint itype=0;
 gchar* strStandard = "Standard orientation:";
 gchar* strInput = "Input orientation:";
 gchar* strOther = "orientation:";
 gchar* strSearch = strOther;

	/* fprintf(stderr,"DEBUG numGeom = %d\n",numgeometry);*/

 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(gchar));
  
 t=g_malloc(taille*sizeof(gchar));
 fd = FOpen(NomFichier, "rb");
 if(fd ==NULL)
 {
  g_free(t);
  t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
  MessageGeom(t,_("Error"),TRUE);
  g_free(t);
  return;
 }
 while(!feof(fd))
 {
	  if(!fgets(t,taille,fd))break;
          if(strstr( t, strStandard))
	  {
		  strSearch = strStandard;
		  break;
	  }
          if(strstr( t, strInput))
		  strSearch = strInput;
 }
 rewind(fd);
 numgeom =0;
 do 
 {
 OK=FALSE;
 while(!feof(fd)){
    	{ char* e = fgets(t,taille,fd);}
	  if(strstr(t,"Charge =") && strstr(t,"Multiplicity ="))
	  {
		  gchar* p = strstr(t,"Charge =")+8;
		  TotalCharges[0] = atoi(p);
		  p = strstr(t,"Multiplicity =")+14;
		  SpinMultiplicities[0] = atoi(p);
	  }
	 if ( strstr( t, strSearch))
	  {
    		{ char* e = fgets(t,taille,fd);}
    		{ char* e = fgets(t,taille,fd);}
    		{ char* e = fgets(t,taille,fd);}
                if(strstr( t, "Type" ))
			itype=1;
                else
			itype=0;
    		{ char* e = fgets(t,taille,fd);}
                numgeom++;
                if(numgeom == numgeometry )
		{
			OK = TRUE;
	  		break;
		}
	  }
        }
 if(!OK && (numgeom == 0) )
 {
  	g_free(t);
  	t = g_strdup_printf(_("Sorry\nI can not read geometry in  %s  file "),NomFichier);
  	MessageGeom(t,_("Error"),TRUE);
  	g_free(t);
  	return;
    }
  if(!OK)break;

  j=-1;
  while(!feof(fd) )
  {
    { char* e = fgets(t,taille,fd);}
    if ( strstr( t, "----------------------------------" ))
    {
	long geomposok = ftell(fd);
      	get_dipole_from_gaussian_output_file(fd);
	fseek(fd, geomposok, SEEK_SET);
	get_charges_from_gaussian_output_file(fd,j+1);
	get_natural_charges_from_gaussian_output_file(fd,j+1);
	fseek(fd, geomposok, SEEK_SET);
	get_esp_charges_from_gaussian_output_file(fd,j+1);
      	break;
    }
    j++;

    if(GeomXYZ==NULL)
	GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
    else
    	GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
    GeomXYZ[j].typeConnections = NULL;

    if(itype==0)
    sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
    else
    sscanf(t,"%d %s %d %s %s %s",&idummy,AtomCoord[0],&idummy,AtomCoord[1],AtomCoord[2],AtomCoord[3]);
	AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 l=strlen(AtomCoord[0]);
          if (l==2)
	 	AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
    GeomXYZ[j].Symb=g_strdup(symb_atom_get((guint)atoi(AtomCoord[0])));
    GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
    GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
    GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
    GeomXYZ[j].ResidueNumber=0;
    if(Units == 0 )
    {
     GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
     GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
     GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
    }
    else
    {
     GeomXYZ[j].X=g_strdup(AtomCoord[1]);
     GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
     GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
    }
    GeomXYZ[j].Charge=g_strdup("0.0");
    GeomXYZ[j].Layer=g_strdup(" ");
  }

 NcentersXYZ = j+1;
	/* fprintf(stderr,"DEBUG Ncenters = %d\n",NcentersXYZ);*/
 if(OK)
   break;
 }while(!feof(fd));

 fclose(fd);
 g_free(t);
 calculMMTypes(FALSE);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
 {
   	clearList(list);
	append_list();
 }
 MethodeGeom = GEOM_IS_XYZ;
 if(GeomDrawingArea != NULL)
	rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS && GeomIsOpen)
 	set_spin_of_electrons();
}
/********************************************************************************/
void read_geom_from_molpro_file(gchar *NomFichier, gint numgeometry)
{
 gchar *t;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 guint idummy;
 guint i;
 gint j=0;
 gint l;
 guint numgeom;

 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(char));
  

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd ==NULL)
 {
  g_free(t);
  t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
  MessageGeom(t,_("Error"),TRUE);
  g_free(t);
  return;
 }
 numgeom = 0;
 do 
 {
 OK=FALSE;
 while(!feof(fd)){
    	{ char* e = fgets(t,taille,fd);}
	 if ( !strcmp(t," ATOMIC COORDINATES\n"))
	  {
    		{ char* e = fgets(t,taille,fd);}
    		{ char* e = fgets(t,taille,fd);}
    		{ char* e = fgets(t,taille,fd);}
 		numgeom++;
                if((gint)numgeom == numgeometry )
		{
			OK = TRUE;
	  		break;
		}
	  }
        }
 if(!OK && (numgeom == 0) ){
  	g_free(t);
  	t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
  	MessageGeom(t,_("Error"),TRUE);
  	g_free(t);
  	return;
    }
 if(!OK)break;

  j=-1;
  while(!feof(fd) )
  {
    { char* e = fgets(t,taille,fd);}
    if ( !strcmp(t,"\n"))
    {
 	get_dipole_from_molpro_output_file(fd);
      	break;
    }
    j++;

    if(GeomXYZ==NULL)
	GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
    else
    	GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
    GeomXYZ[j].typeConnections = NULL;
    sscanf(t,"%d %s %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[1], AtomCoord[2],AtomCoord[3]);

	{
		gint k;
		for(k=0;k<(gint)strlen(AtomCoord[0]);k++) if(isdigit(AtomCoord[0][k])) AtomCoord[0][k] = ' ';
		delete_all_spaces(AtomCoord[0]);
	}

	AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 l=strlen(AtomCoord[0]);
          if (l==2)
	 	AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
    GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);
    GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
    GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
    GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
    GeomXYZ[j].ResidueNumber=0;
    if(Units == 1 )
    {
     GeomXYZ[j].X=g_strdup(bohr_to_ang(AtomCoord[1]));
     GeomXYZ[j].Y=g_strdup(bohr_to_ang(AtomCoord[2]));
     GeomXYZ[j].Z=g_strdup(bohr_to_ang(AtomCoord[3]));
    }
    else
    {
     GeomXYZ[j].X=g_strdup(AtomCoord[1]);
     GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
     GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
    }
    GeomXYZ[j].Charge=g_strdup("0.0");
    GeomXYZ[j].Layer=g_strdup(" ");
  }

 NcentersXYZ = j+1;
 if(OK)
   break;
 }while(!feof(fd));

 fclose(fd);
 g_free(t);
 calculMMTypes(FALSE);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
 {
   	clearList(list);
	append_list();
 }
 MethodeGeom = GEOM_IS_XYZ;
 if(GeomDrawingArea != NULL)
	rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS && GeomIsOpen)
 	set_spin_of_electrons();
}
/********************************************************************************/
void read_last_gaussian_file(GabeditFileChooser *SelecFile , gint response_id)
{
 gchar *t;
 gchar *NomFichier;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 guint idummy;
 guint i;
 gint j=0;
 gint l;
 guint numgeom;
 gchar *pdest;
 guint itype=0;
 gchar* strInput = "Input orientation:";
 gchar* strStand = "Standard orientation:";
 gchar* strSearch = strInput;

 if(response_id != GTK_RESPONSE_OK) return;
 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(char));
 NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
  
 if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
 {
	MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
    	return ;
 }

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd ==NULL)
 {
  	MessageGeom(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  	return;
 }
 while(!feof(fd)){
	  if(!fgets(t,taille,fd))break;
	  if(strstr(t,strStand))
	  {
		  strSearch = strStand;
		  break;
	  }
 }
 fseek(fd, 0, SEEK_SET);
 numgeom =1;
 do 
 {
 OK=FALSE;
 while(!feof(fd)){
    	{ char* e = fgets(t,taille,fd);}
	  if(strstr(t,"Charge =") && strstr(t,"Multiplicity ="))
	  {
		  gchar* p = strstr(t,"Charge =")+8;
		  TotalCharges[0] = atoi(p);
		  p = strstr(t,"Multiplicity =")+14;
		  SpinMultiplicities[0] = atoi(p);
	  }
	 if ( strstr( t, strSearch))
	  {
    		{ char* e = fgets(t,taille,fd);}
    		{ char* e = fgets(t,taille,fd);}
    		{ char* e = fgets(t,taille,fd);}
                if(strstr( t, "Type" ))
			itype=1;
                else
			itype=0;
    		{ char* e = fgets(t,taille,fd);}
                numgeom++;
                OK = TRUE;
	  	break;
	  }
        }
 if(!OK && (numgeom == 1) ){
  	MessageGeom(_("Sorry\nI can not read this file"),_("Error"),TRUE);
	return;
    }
 if(!OK)break;

  j=-1;
  while(!feof(fd) )
  {
   { char* e = fgets(t,taille,fd);}
    if (strstr( t, "----------------------------------" ))
    {
	long geomposok = ftell(fd);
      	get_dipole_from_gaussian_output_file(fd);
	fseek(fd, geomposok, SEEK_SET);
	get_charges_from_gaussian_output_file(fd,j+1);
	get_natural_charges_from_gaussian_output_file(fd,j+1);
	fseek(fd, geomposok, SEEK_SET);
	get_esp_charges_from_gaussian_output_file(fd,j+1);
	fseek(fd, geomposok, SEEK_SET);
      	break;
    }
    j++;

    if(GeomXYZ==NULL)
	GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
    else
    	GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
    GeomXYZ[j].typeConnections = NULL;

    if(itype==0)
    sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
    else
    sscanf(t,"%d %s %d %s %s %s",&idummy,AtomCoord[0],&idummy,AtomCoord[1],AtomCoord[2],AtomCoord[3]);
	AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 l=strlen(AtomCoord[0]);
          if (l==2)
	 	AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
    GeomXYZ[j].Symb=g_strdup(symb_atom_get((guint)atoi(AtomCoord[0])));
    GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
    GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
    GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
    GeomXYZ[j].ResidueNumber=0;
    if(Units == 0 )
    {
     GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
     GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
     GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
    }
    else
    {
     GeomXYZ[j].X=g_strdup(AtomCoord[1]);
     GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
     GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
    }
    GeomXYZ[j].Charge=g_strdup("0.0");
    GeomXYZ[j].Layer=g_strdup(" ");
  }

 NcentersXYZ = j+1;
 }while(!feof(fd));

 fclose(fd);
 calculMMTypes(FALSE);

 g_free(t);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if(GeomIsOpen)
 {
 	append_list();
 }
 if(GeomDrawingArea != NULL)
	rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();
  set_last_directory(NomFichier);
}
/********************************************************************************/
void read_first_gaussian_file(GabeditFileChooser *SelecFile, gint response_id)
{
 gchar *t;
 gchar *NomFichier;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 guint idummy;
 guint i;
 gint j=0;
 gint l;
 gchar *pdest;
 guint itype=0;
 gchar* strInput = "Input orientation:";
 gchar* strStand = "Standard orientation:";
 gchar* strSearch = strInput;

 if(response_id != GTK_RESPONSE_OK) return;

 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(char));
 NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
  
 if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
 {
	MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
    return ;
 }

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd ==NULL)
 {
  MessageGeom(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  return;
 }
 while(!feof(fd)){
	  if(!fgets(t,taille,fd))break;
	  if(strstr(t,strStand))
	  {
		  strSearch = strStand;
		  break;
	  }
 }
 fseek(fd, 0, SEEK_SET);
 OK=FALSE;
 while(!feof(fd)){
   	{ char* e = fgets(t,taille,fd);}
	  if(strstr(t,"Charge =") && strstr(t,"Multiplicity ="))
	  {
		  gchar* p = strstr(t,"Charge =")+8;
		  TotalCharges[0] = atoi(p);
		  p = strstr(t,"Multiplicity =")+14;
		  SpinMultiplicities[0] = atoi(p);
	  }
	 if (strstr( t, strSearch ) )
	  {
   		{ char* e = fgets(t,taille,fd);}
   		{ char* e = fgets(t,taille,fd);}
   		{ char* e = fgets(t,taille,fd);}
                if(strstr( t, "Type" ))
			itype=1;
                else
			itype=0;

   		{ char* e = fgets(t,taille,fd);}
                OK = TRUE;
	  	break;
	  }
        }
 if(!OK){
  	MessageGeom(_("Sorry\nI can not read this file"),_("Error"),TRUE);
	return;
    }

  j=-1;
  while(!feof(fd) )
  {
    { char* e = fgets(t,taille,fd);}
    if (strstr( t, "----------------------------------" ))
    {
	long geomposok = ftell(fd);
      	get_dipole_from_gaussian_output_file(fd);
	fseek(fd, geomposok, SEEK_SET);
	get_charges_from_gaussian_output_file(fd,j+1);
	get_natural_charges_from_gaussian_output_file(fd,j+1);
	fseek(fd, geomposok, SEEK_SET);
	get_esp_charges_from_gaussian_output_file(fd,j+1);
      	break;
    }
    j++;
    if(GeomXYZ==NULL)GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
    else
    GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
    GeomXYZ[j].typeConnections = NULL;

    if(itype==0)
    sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
    else
    sscanf(t,"%d %s %d %s %s %s",&idummy,AtomCoord[0],&idummy,AtomCoord[1],AtomCoord[2],AtomCoord[3]);
	AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 l=strlen(AtomCoord[0]);
          if (l==2)
	 	AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
    GeomXYZ[j].Symb=g_strdup(symb_atom_get((guint)atoi(AtomCoord[0])));
    GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
    GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
    GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
    GeomXYZ[j].ResidueNumber=0;
    if(Units == 0 )
    {
     GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
     GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
     GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
    }
    else
    {
     GeomXYZ[j].X=g_strdup(AtomCoord[1]);
     GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
     GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
    }
    GeomXYZ[j].Charge=g_strdup("0.0");
    GeomXYZ[j].Layer=g_strdup(" ");
  }

 fclose(fd);
 calculMMTypes(FALSE);
 NcentersXYZ = j+1;
 g_free(t);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if(GeomIsOpen)
 {
 	append_list();
 }

 if(GeomDrawingArea != NULL)
	rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();
  set_last_directory(NomFichier);
}
/********************************************************************************/
void read_fchk_gaussian_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *fileName;
	FILE *file;
	gint i,j;
	gint *z = NULL;
	gdouble* coords = NULL;
	gdouble* charges = NULL;
	gdouble* dipole = NULL;
	gint n;

	if(response_id != GTK_RESPONSE_OK) return;

	fileName = gabedit_file_chooser_get_current_file(SelecFile);
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
    		return ;
	}
	file = FOpen(fileName, "rb");
	if(file ==NULL)
	{
  		MessageGeom(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  		return;
	}

	i = get_one_int_from_fchk_gaussian_file(file,"Multiplicity ");
	if(i<1)
	{
  		MessageGeom(_("Sorry\nI can not the multiplicity from this file"),_("Error"),TRUE);
		return;
	}
	rewind(file);
	j = get_one_int_from_fchk_gaussian_file(file,"Number of atoms ");
	if(j<1)
	{
  		MessageGeom(_("Sorry\nI can not the number of atoms from this file"),_("Error"),TRUE);
		return;
	}

	SpinMultiplicities[0] = i;
	rewind(file);
	z = get_array_int_from_fchk_gaussian_file(file, "Atomic numbers ", &n);
	if(n!=j)
	{
  		MessageGeom(_("Sorry\nI can not read the atomic numbers from this file"),_("Error"),TRUE);
		return;
	}
	rewind(file);
	coords = get_array_real_from_fchk_gaussian_file(file, "Current cartesian coordinates  ", &n);
	if(n!=3*j)
	{
  		MessageGeom(_("Sorry\nI can not read the current cartesian coordinates from this file"),_("Error"),TRUE);
		return;
	}
	rewind(file);
	TotalCharges[0] = get_one_int_from_fchk_gaussian_file(file,"Charge ");
	NcentersXYZ = j;

    	GeomXYZ=g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));
	for(j=0;j<NcentersXYZ;j++)
	{
    		GeomXYZ[j].typeConnections = NULL;
		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(symb_atom_get(z[j]));
		GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].ResidueNumber=0;
		if(Units == 0 )
		{
			GeomXYZ[j].X=g_strdup_printf("%0.20lf",coords[j*3]);
			GeomXYZ[j].Y=g_strdup_printf("%0.20lf",coords[j*3+1]);
			GeomXYZ[j].Z=g_strdup_printf("%0.20lf",coords[j*3+2]);
		}
    		else
    		{
			GeomXYZ[j].X=g_strdup_printf("%0.20lf",coords[j*3]*BOHR_TO_ANG);
			GeomXYZ[j].Y=g_strdup_printf("%0.20lf",coords[j*3+1]*BOHR_TO_ANG);
			GeomXYZ[j].Z=g_strdup_printf("%0.20lf",coords[j*3+2]*BOHR_TO_ANG);
    		}
    		GeomXYZ[j].Charge=g_strdup("0.0");
    		GeomXYZ[j].Layer=g_strdup(" ");
	}
	if(z) g_free(z);
	if(coords) g_free(coords);
	z = NULL;
	coords = NULL;
	rewind(file);
	charges = get_array_real_from_fchk_gaussian_file(file, "NPA Charges ", &n);
	if(n==NcentersXYZ && charges)
	{
		for(j=0;j<NcentersXYZ;j++)
    			GeomXYZ[j].Charge = g_strdup_printf("%14.8f",charges[j]);

	}
	else
	{
		rewind(file);
		charges = get_array_real_from_fchk_gaussian_file(file, "ESP Charges  ", &n);
		if(n==NcentersXYZ && charges) 
		{
			for(j=0;j<NcentersXYZ;j++)
    				GeomXYZ[j].Charge = g_strdup_printf("%14.8f",charges[j]);
		}
		else
		{
			rewind(file);
			charges = get_array_real_from_fchk_gaussian_file(file, "Mulliken Charges  ", &n);
			if(n==NcentersXYZ && charges) 
			{
				for(j=0;j<NcentersXYZ;j++)
    					GeomXYZ[j].Charge = g_strdup_printf("%14.8f",charges[j]);
			}
			if(charges) g_free(charges);
		}
	}
	rewind(file);
	dipole = get_array_real_from_fchk_gaussian_file(file, "Dipole Moment  ", &n);
	init_dipole();
	if(n==3)
	{
		for(i=0;i<3;i++) Dipole.value[i] = dipole[i] / AUTODEB;
		Dipole.def = TRUE;

	}
	if(dipole) g_free(dipole);
 	fclose(file);
 	calculMMTypes(FALSE);
	if(GeomIsOpen) append_list();
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS) set_spin_of_electrons();
	set_last_directory(fileName);
}
/********************************************************************************/
void read_last_molcas_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar t[BSIZE];
	gchar *NomFichier;
	gboolean OK;
	gchar symb[100];
	gchar x[100];
	gchar y[100];
	gchar z[100];
	FILE *file;
	guint idummy;
	gint i;
	gint j=0;
	gint l;
	gint numgeom = 0;

 	if(response_id != GTK_RESPONSE_OK) return;
 	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}

	file = FOpen(NomFichier, "rb");
	if(file ==NULL)
	{
		MessageGeom(_("Sorry\nI can not open this file"),_("Error"),TRUE);
		return;
	}
	numgeom = 0;
	do{
	OK=FALSE;
	while(!feof(file))
	{
    		{ char* e = fgets(t,BSIZE,file);}
		if (strstr(t," Cartesian coordinates:"))
		{
    			{ char* e = fgets(t,BSIZE,file);}
    			{ char* e = fgets(t,BSIZE,file);}
    			{ char* e = fgets(t,BSIZE,file);}
			numgeom++;
                	OK = TRUE;
	  		break;
	 	}
	}
	if(!OK && numgeom==0)
	{
  		MessageGeom(_("Sorry\nI can not read this file"),_("Error"),TRUE);
		return;
	}
	if(!OK) break;

	j=-1;
	while(!feof(file) )
	{
    		{ char* e = fgets(t,BSIZE,file);}
		if (strstr(t,"-----------------------------------------"))
		{
			/*
			get_dipole_from_molcas_output_file(file);
			*/
			break;
		}
		j++;
		if(GeomXYZ==NULL)GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
		else
			GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
    		GeomXYZ[j].typeConnections = NULL;

		sscanf(t,"%d %s %s %s %s",&idummy,symb,x,y,z);

		for(i=0;i<(gint)strlen(symb);i++)
			if(isdigit(symb[i])) symb[i] = ' ';
		delete_all_spaces(symb);

		symb[0]=toupper(symb[0]);
		l=strlen(symb);
		for(i=1;i<l;i++)
	 		symb[i]=tolower(symb[i]);

		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(symb);
		GeomXYZ[j].mmType=g_strdup(symb);
		GeomXYZ[j].pdbType=g_strdup(symb);
		GeomXYZ[j].Residue=g_strdup(symb);
		GeomXYZ[j].ResidueNumber=0;
		if(Units == 1 )
		{
			GeomXYZ[j].X=g_strdup(bohr_to_ang(x));
			GeomXYZ[j].Y=g_strdup(bohr_to_ang(y));
			GeomXYZ[j].Z=g_strdup(bohr_to_ang(z));
		}
		else
		{
			GeomXYZ[j].X=g_strdup(x);
			GeomXYZ[j].Y=g_strdup(y);
			GeomXYZ[j].Z=g_strdup(z);
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
	}
	NcentersXYZ = j+1;
	}while(!feof(file));
	fclose(file);
 	calculMMTypes(FALSE);

	if(GeomIsOpen)
	{
		append_list();
	}
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS) set_spin_of_electrons();
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_last_molpro_file(GabeditFileChooser *SelecFile , gint response_id)
{
 gchar *t;
 gchar *NomFichier;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 guint idummy;
 guint i;
 gint j=0;
 gint l;
 guint numgeom;

 if(response_id != GTK_RESPONSE_OK) return;
 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(char));
 NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
  
 if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
 {
	MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
	return ;
 }

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd ==NULL)
 {
  MessageGeom(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  return;
 }
 numgeom =1;
 do 
 {
 OK=FALSE;
 while(!feof(fd)){
    	{ char* e = fgets(t,BSIZE,fd);}
	 if ( !strcmp(t," ATOMIC COORDINATES\n"))
	  {
    		{ char* e = fgets(t,BSIZE,fd);}
    		{ char* e = fgets(t,BSIZE,fd);}
    		{ char* e = fgets(t,BSIZE,fd);}
 		numgeom++;
                OK = TRUE;
	  	break;
	  }
        }
 if(!OK && (numgeom == 1) ){
  	MessageGeom(_("Sorry\nI can not read this file"),_("Error"),TRUE);
	return;
    }
 if(!OK)break;

  j=-1;
  while(!feof(fd) )
  {
    { char* e = fgets(t,BSIZE,fd);}
    if ( !strcmp(t,"\n"))
    {
 	get_dipole_from_molpro_output_file(fd);
      	break;
    }
    j++;

    if(GeomXYZ==NULL)
	GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
    else
    	GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
   GeomXYZ[j].typeConnections = NULL;
    sscanf(t,"%d %s %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[1],
AtomCoord[2],AtomCoord[3]);

	{
		gint k;
		for(k=0;k<(gint)strlen(AtomCoord[0]);k++) if(isdigit(AtomCoord[0][k])) AtomCoord[0][k] = ' ';
		delete_all_spaces(AtomCoord[0]);
	}

	AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 l=strlen(AtomCoord[0]);
          if (l==2)
	 	AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
    GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);
    GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
    GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
    GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
    GeomXYZ[j].ResidueNumber=0;
    if(Units == 1 )
    {
     GeomXYZ[j].X=g_strdup(bohr_to_ang(AtomCoord[1]));
     GeomXYZ[j].Y=g_strdup(bohr_to_ang(AtomCoord[2]));
     GeomXYZ[j].Z=g_strdup(bohr_to_ang(AtomCoord[3]));
    }
    else
    {
     GeomXYZ[j].X=g_strdup(AtomCoord[1]);
     GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
     GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
    }
    GeomXYZ[j].Charge=g_strdup("0.0");
    GeomXYZ[j].Layer=g_strdup(" ");
  }

 NcentersXYZ = j+1;
 }while(!feof(fd));

 fclose(fd);
 calculMMTypes(FALSE);
 g_free(t);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if(GeomIsOpen)
 {
 	append_list();
 }
 if(GeomDrawingArea != NULL)
	rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();
  
  set_last_directory(NomFichier);
}
/********************************************************************************/
void get_charges_from_qchem_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;
	gint ngrad=0;


	for(i=0;i<N;i++)
		GeomXYZ[i].Charge = g_strdup("0.0");

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Mulliken Net Atomic Charges");

		if(pdest)
		{
			gboolean OK = FALSE;
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd)) break;
				if(strstr(t,"----------------"))
				{
					OK = TRUE;
					break;
				}
			}
			if(!OK) break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else break;
				if(sscanf(t,"%s %s %s",dump,dump,d)==3)
				{
					g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup(d);
				}
			}
			break;
		}
		else
		{
          		pdest = strstr( t, "GradGradGrad" );
			if(pdest)
			{
				ngrad++;
			}
			if(ngrad>2)
				break;
		}

	}
}
/********************************************************************************/
void read_geom_from_qchem_file(gchar *NomFichier, gint numgeometry)
{
	gchar *t;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint idummy;
	guint i;
	gint j=0;
	gint l;
	gint numgeom;
	gchar *pdest;
	long int geomposok = 0;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	 
	t=g_malloc(taille*sizeof(gchar));
	fd = FOpen(NomFichier, "rb");
	if(fd ==NULL)
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
	 	MessageGeom(t,_("Error"),TRUE);
	 	g_free(t);
	 	return;
	}
	numgeom =0;
	 while(!feof(fd))
	 {
		if(!fgets(t,taille,fd))break;
		pdest = NULL;
		if(( strstr( t,"Atom")  || strstr( t,"ATOM")) && strstr( t,"X") && strstr( t,"Y") && strstr( t,"Z")) 
		{
			if(strstr( t,"Atom")) if(!fgets(t,taille,fd))break; /* ---- ligne */
			pdest = t;
		}
		if ( pdest )
		{
			numgeom++;
			geomposok = ftell(fd);
			if(numgeom == numgeometry )
			{
				break;
			}
		}
	 }
	 if(numgeom == 0)
	 {
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not read geometry in	%s file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	  }
	j=-1;
	fseek(fd, geomposok, SEEK_SET);
	while(!feof(fd) )
	{
		if(!fgets(t,taille,fd))break;
		if (atoi(t)<1)
		{
			geomposok = ftell(fd);
			get_charges_from_qchem_output_file(fd,j+1);
			fseek(fd, geomposok, SEEK_SET);
 			get_dipole_from_qchem_output_file(fd);
			break;
		}
		j++;
		if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
		else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
   		GeomXYZ[j].typeConnections = NULL;

		sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
		l=strlen(AtomCoord[0]); 
		if(isdigit(AtomCoord[0][1]))l=1;
		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
		if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);

		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(t);
		GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].ResidueNumber=0;
		if(Units == 0 )
		{
			GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
			GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
			GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
		}
		else
		{
			GeomXYZ[j].X=g_strdup(AtomCoord[1]);
			GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
			GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
	  }
	NcentersXYZ = j+1;

	 fclose(fd);
 	 calculMMTypes(FALSE);
	 g_free(t);
	 for(i=0;i<5;i++) g_free(AtomCoord[i]);
	 if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	 {
	 	clearList(list);
		append_list();
	 }
	 MethodeGeom = GEOM_IS_XYZ;
	 if(GeomDrawingArea != NULL) rafresh_drawing();
	 if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void get_charges_from_nwchem_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar d3[BSIZE];
  	gchar d4[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


	for(i=0;i<N;i++)
		GeomXYZ[i].Charge = g_strdup("0.0");

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Atom       Charge   Shell Charges");

		if(pdest)
		{
			gboolean OK = FALSE;
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd)) break;
				if(strstr(t,"----------------"))
				{
					OK = TRUE;
					break;
				}
			}
			if(!OK) break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else break;
				if(sscanf(t,"%s %s %s %s",d,d,d3,d4)==4)
				{
					g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup_printf("%f",atof(d3)-atof(d4));
				}
			}
			break;
		}
	}
}
/********************************************************************************/
void read_geom_from_nwchem_file(gchar *NomFichier, gint numgeometry)
{
	gchar *t;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint idummy;
	guint i;
	gint j=0;
	gint l;
	gint numgeom;
	gchar *pdest;
	long int geomposok = 0;
	gboolean ang = FALSE;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	 
	t=g_malloc(taille*sizeof(gchar));
	fd = FOpen(NomFichier, "rb");
	if(fd ==NULL)
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
	 	MessageGeom(t,_("Error"),TRUE);
	 	g_free(t);
	 	return;
	}
	numgeom =0;
	 while(!feof(fd))
	 {
		if(!fgets(t,taille,fd))break;
		if(strstr(t,"Output coordinates in angstroms ")) ang = TRUE;
		if(strstr(t,"Output coordinates in a.u.")) ang = FALSE;
		pdest = strstr( t, "Charge          X              Y              Z");
		if(pdest) 
		{
			if(!fgets(t,taille,fd))break;
			pdest = strstr( t, "--------------");
		}
		if ( pdest )
		{
			numgeom++;
			geomposok = ftell(fd);
			if(numgeom == numgeometry )
			{
				break;
			}
		}
	 }
	 if(numgeom == 0)
	 {
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not read geometry in	%s file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	  }
	j=-1;
	fseek(fd, geomposok, SEEK_SET);
	while(!feof(fd) )
	{
		if(!fgets(t,taille,fd))break;
		if(this_is_a_backspace(t))
		{
			geomposok = ftell(fd);
			get_charges_from_nwchem_output_file(fd,j+1);
			fseek(fd, geomposok, SEEK_SET);
 			get_dipole_from_nwchem_output_file(fd);
			break;
		}
		j++;
		if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
		else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
   		GeomXYZ[j].typeConnections = NULL;

		sscanf(t,"%d %s %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[4],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
		l=strlen(AtomCoord[0]); 
		if(isdigit(AtomCoord[0][1]))l=1;
		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
		if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);

		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(t);
		GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].ResidueNumber=0;

		if(!ang )
		{
			AtomCoord[1] = bohr_to_ang(AtomCoord[1]);
			AtomCoord[2] = bohr_to_ang(AtomCoord[2]);
			AtomCoord[3] = bohr_to_ang(AtomCoord[3]);
		}

		if(Units == 0 )
		{
			GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
			GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
			GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
		}
		else
		{
			GeomXYZ[j].X=g_strdup(AtomCoord[1]);
			GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
			GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
	  }
	NcentersXYZ = j+1;

	 fclose(fd);
 	 calculMMTypes(FALSE);
	 g_free(t);
	 for(i=0;i<5;i++) g_free(AtomCoord[i]);
	 if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	 {
	 	clearList(list);
		append_list();
	 }
	 MethodeGeom = GEOM_IS_XYZ;
	 if(GeomDrawingArea != NULL) rafresh_drawing();
	 if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void get_charges_from_psicode_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar d3[BSIZE];
  	gchar d4[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


	for(i=0;i<N;i++)
		GeomXYZ[i].Charge = g_strdup("0.0");

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		pdest = strstr( t, "Atom       Charge   Shell Charges");

		if(pdest)
		{
			gboolean OK = FALSE;
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd)) break;
				if(strstr(t,"----------------"))
				{
					OK = TRUE;
					break;
				}
			}
			if(!OK) break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else break;
				if(sscanf(t,"%s %s %s %s",d,d,d3,d4)==4)
				{
					g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup_printf("%f",atof(d3)-atof(d4));
				}
			}
			break;
		}
	}
}
/********************************************************************************/
void read_geom_from_psicode_file(gchar *NomFichier, gint numgeometry)
{
	gchar *t;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint idummy;
	guint i;
	gint j=0;
	gint l;
	gint numgeom;
	gchar *pdest;
	long int geomposok = 0;
	gboolean ang = TRUE;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	 
	t=g_malloc(taille*sizeof(gchar));
	fd = FOpen(NomFichier, "rb");
	if(fd ==NULL)
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
	 	MessageGeom(t,_("Error"),TRUE);
	 	g_free(t);
	 	return;
	}
	numgeom =0;
	 while(!feof(fd))
	 {
		if(!fgets(t,taille,fd))break;
		pdest = strstr( t, "Geometry (in Angstrom),");
		if(pdest) 
		{
			if(!fgets(t,taille,fd))break;
			if(!fgets(t,taille,fd))break;
			pdest = strstr( t, "Center              X                  Y                   Z");
		}

		if(pdest) 
		{
			if(!fgets(t,taille,fd))break;
			pdest = strstr( t, "--------------");
		}
		if ( pdest )
		{
			numgeom++;
			geomposok = ftell(fd);
			if(numgeom == numgeometry )
			{
				break;
			}
		}
	 }
	 if(numgeom == 0)
	 {
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not read geometry in	%s file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	  }
	j=-1;
	fseek(fd, geomposok, SEEK_SET);
	while(!feof(fd) )
	{
		if(!fgets(t,taille,fd))break;
		if(this_is_a_backspace(t))
		{
			geomposok = ftell(fd);
			get_charges_from_psicode_output_file(fd,j+1);
			fseek(fd, geomposok, SEEK_SET);
 			get_dipole_from_psicode_output_file(fd);
			break;
		}
		j++;
		if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
		else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
   		GeomXYZ[j].typeConnections = NULL;

		sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
		l=strlen(AtomCoord[0]); 
		if(isdigit(AtomCoord[0][1]))l=1;
		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
		if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);

		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(t);
		GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].ResidueNumber=0;

		if(!ang )
		{
			AtomCoord[1] = bohr_to_ang(AtomCoord[1]);
			AtomCoord[2] = bohr_to_ang(AtomCoord[2]);
			AtomCoord[3] = bohr_to_ang(AtomCoord[3]);
		}

		if(Units == 0 )
		{
			GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
			GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
			GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
		}
		else
		{
			GeomXYZ[j].X=g_strdup(AtomCoord[1]);
			GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
			GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
	  }
	NcentersXYZ = j+1;

	 fclose(fd);
 	 calculMMTypes(FALSE);
	 g_free(t);
	 for(i=0;i<5;i++) g_free(AtomCoord[i]);
	 if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	 {
	 	clearList(list);
		append_list();
	 }
	 MethodeGeom = GEOM_IS_XYZ;
	 if(GeomDrawingArea != NULL) rafresh_drawing();
	 if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void get_esp_charges_from_mopac_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


	/* printf("NAtoms = %d\n",N);*/
	/* for(i=0;i<N;i++) GeomXYZ[i].Charge = g_strdup("0.0");*/

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		if(!fgets(t,taille,fd)) break;
    		pdest = strstr( t, "ELECTROSTATIC POTENTIAL CHARGES");

		if(pdest)
		{
    			if(!fgets(t,taille,fd)) break;
    			if(!fgets(t,taille,fd)) break;
			for(i=0;i<N;i++)
			{
    				if(!fgets(t,taille,fd)) break;
				if(sscanf(t,"%s %s %s",dump, dump , d)==3)
				{
					if(GeomXYZ[i].Charge) g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup(d);
				}
				else break;
			}
			break;
		}
	}
}
/********************************************************************************/
void get_charge_and_multiplicity_from_mopac_output_file(FILE* fd)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];

	TotalCharges[0] = 0;
	SpinMultiplicities[0] =1;

  	while(!feof(fd) )
	{
    		if(!fgets(t,taille,fd)) break;
		if ( strstr(t,"CHARGE ON SYSTEM") && strstr(t,"="))
		{
			gchar* p = strstr(t,"=")+1;
			TotalCharges[0] = atoi(p);
		}
		if ( strstr(t,"SPIN STATE DEFINED AS A"))
		{
			if ( strstr(t,"SINGLET")) SpinMultiplicities[0] = 1;
			if ( strstr(t,"DOUBLET")) SpinMultiplicities[0] = 2;
			if ( strstr(t,"TRIPLET")) SpinMultiplicities[0] = 3;
			if ( strstr(t,"QUARTET")) SpinMultiplicities[0] = 4;
			if ( strstr(t,"QUINTET")) SpinMultiplicities[0] = 5;
			if ( strstr(t,"SEXTET")) SpinMultiplicities[0] = 6;
		}
	}
}
/********************************************************************************/
void get_charges_from_mopac_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


	/* printf("NAtoms = %d\n",N);*/
	for(i=0;i<N;i++) GeomXYZ[i].Charge = g_strdup("0.0");

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		if(!fgets(t,taille,fd)) break;
    		pdest = strstr( t, "NET ATOMIC CHARGES");

		if(pdest)
		{
    			if(!fgets(t,taille,fd)) break;
    			if(!fgets(t,taille,fd)) break;
			for(i=0;i<N;i++)
			{
    				if(!fgets(t,taille,fd)) break;
				if(sscanf(t,"%s %s %s",dump, dump , d)==3)
				{
					g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup(d);
				}
				else break;
			}
			break;
		}
	}
}
/********************************************************************************/
void read_geom_from_mopac_output_file(gchar *NomFichier, gint numgeometry)
{
	gchar *t;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint idummy;
	guint i;
	gint j=0;
	gint l;
	gint numgeom;
	gchar *pdest;
	long int geomposok = 0;
        gchar tmp1[100];
        gchar tmp2[100];

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	 
	t=g_malloc(taille*sizeof(gchar));
	fd = FOpen(NomFichier, "rb");
	if(fd ==NULL)
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
	 	MessageGeom(t,_("Error"),TRUE);
	 	g_free(t);
	 	return;
	}
	numgeom =0;
	 while(!feof(fd))
	 {
		if(!fgets(t,taille,fd))break;
		pdest = strstr( t, "ATOM   CHEMICAL          X               Y               Z");
		if(pdest) 
		{
			if(!fgets(t,taille,fd)) {pdest=0;break;}
			if(!fgets(t,taille,fd)) {pdest=0;break;}
		}
		if ( pdest )
		{
			numgeom++;
			geomposok = ftell(fd);
			if(numgeom == numgeometry )
			{
				break;
			}
		}
	 }
	 if(numgeom == 0)
	 {
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not read geometry in	%s file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	  }
	j=-1;
	fseek(fd, geomposok, SEEK_SET);
	while(!feof(fd) )
	{
		gint ii;
		if(!fgets(t,taille,fd))break;
		if(this_is_a_backspace(t) || sscanf(t,"%d",&ii)==0)
		{
			if(numgeometry<0)
			{
				fseek(fd, 0, SEEK_SET);
				get_charges_from_mopac_output_file(fd,j+1);
				fseek(fd, 0, SEEK_SET);
				get_esp_charges_from_mopac_output_file(fd,j+1);
				fseek(fd, 0, SEEK_SET);
 				get_dipole_from_mopac_output_file(fd);
				fseek(fd, 0, SEEK_SET);
 				get_charge_and_multiplicity_from_mopac_output_file(fd);
			}
			break;
		}
		j++;
		if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
		else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
   		GeomXYZ[j].typeConnections = NULL;
		for(ii=0;ii<strlen(t);ii++) if(t[ii]=='*') t[ii]=' ';
		sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);

		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
		l=strlen(AtomCoord[0]); 
		if(isdigit(AtomCoord[0][1]))l=1;
		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
		if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);

		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(t);
		GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].ResidueNumber=0;
		if(Units == 0 )
		{
			GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
			GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
			GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
		}
		else
		{
			GeomXYZ[j].X=g_strdup(AtomCoord[1]);
			GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
			GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
	  }
	NcentersXYZ = j+1;
	if(NcentersXYZ>0)
	{
		fseek(fd, 0, SEEK_SET);
 		get_charge_and_multiplicity_from_mopac_output_file(fd);
	}

	 fclose(fd);
 	 calculMMTypes(FALSE);
	 g_free(t);
	 for(i=0;i<5;i++) g_free(AtomCoord[i]);
	 if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	 {
	 	clearList(list);
		append_list();
	 }
	 MethodeGeom = GEOM_IS_XYZ;
	 if(GeomDrawingArea != NULL) rafresh_drawing();
	 if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void read_XYZ_from_mopac_irc_output_file(gchar *FileName, gint numGeom)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[10];
	FILE *fd;
	guint taille=BSIZE;
	guint i,l;
	GeomXYZAtomDef* GeomXYZtemp=NULL;
	gint Ncent = 0;
	gint Nvar = 0;
	VariablesXYZDef* VariablesXYZtemp=NULL;
	gchar symb[20];
	gchar type[20];
	gchar charge[20];
	gint k1;
	gint j;

	for(i=0;i<10;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	fd = FOpen(FileName, "rb");
	if(fd == NULL)
	{
		t = g_strdup_printf(_("Sorry\n I can not open \"%s\" file"),FileName); 
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	t=g_malloc(taille);
	OK = TRUE;
	i = 0;
  	while(!feof(fd) )    
  	{
 		if(!fgets(t, taille, fd))break;
		if(
			strstr(t,"POTENTIAL") && 
			strstr(t,"LOST") &&
			strstr(t,"TOTAL")
		) 
		{
			i++;
			if(i==numGeom)break;
		}
	}
	if(i==0) OK = FALSE;
  	while(!feof(fd) && OK )    
  	{
 		if(!fgets(t, taille, fd))OK = FALSE;
		if(
		strstr(t,"ATOM")&&
		strstr(t,"X")&&
		strstr(t,"Y")&&
		strstr(t,"Z")) break;
	}
	if(!OK)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\n I can not read geometry from \"%s\" file"),FileName); 
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	Ncent=0;
	GeomXYZtemp = NULL;
	
	j=0;
	while(!feof(fd) )
	{
		if(!fgets(t,taille,fd))break;
		if(this_is_a_backspace(t)) break;
                i = sscanf(t,"%d %s %s %s %s",
				&k1,AtomCoord[0],
				AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		if(i==0) break;
    		if( i== 5)
                {
                        Ncent = j+1;
			GeomXYZtemp=g_realloc(GeomXYZtemp,Ncent*sizeof(GeomXYZAtomDef));
    			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			GeomXYZtemp[j].Nentry=NUMBER_LIST_XYZ;
			get_symb_type_charge(AtomCoord[0],symb,type,charge);
			{
				gint k;
				for(k=0;k<(gint)strlen(symb);k++) if(isdigit(symb[k])) symb[k] = ' ';
				delete_all_spaces(symb);
			}

    			GeomXYZtemp[j].Symb=g_strdup(symb);
			GeomXYZtemp[j].mmType=g_strdup(type);
			GeomXYZtemp[j].pdbType=g_strdup(type);
			GeomXYZtemp[j].Charge=g_strdup(charge);

    			GeomXYZtemp[j].Residue=g_strdup("DUM");
    			GeomXYZtemp[j].ResidueNumber=0;
    			GeomXYZtemp[j].X=g_strdup(AtomCoord[1]);
    			GeomXYZtemp[j].Y=g_strdup(AtomCoord[2]);
    			GeomXYZtemp[j].Z=g_strdup(AtomCoord[3]);
    			GeomXYZtemp[j].Layer=g_strdup(" ");
    			GeomXYZtemp[j].typeConnections = NULL;
			j++;
		}
        }
	fseek(fd, 0L, SEEK_SET);
	if(j==0) OK = FALSE;
	else get_charge_and_multiplicity_from_mopac_output_file(fd);
	fclose(fd);

	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if( !OK || Ncent <1 )
	{
		FreeGeomXYZ(GeomXYZtemp,VariablesXYZtemp,Ncent, Nvar);
		MessageGeom(_("Sorry\n I can not read geometry in Mopac input file"),_("Error"),TRUE);
		return;
	}
	if(GeomXYZ) freeGeomXYZ();
	if(VariablesXYZ) freeVariablesXYZ();
	GeomXYZ = GeomXYZtemp;
	NcentersXYZ = Ncent;
	NVariablesXYZ = Nvar;
	VariablesXYZ = VariablesXYZtemp;
	MethodeGeom = GEOM_IS_XYZ;
	calculMMTypes(FALSE);
	if( Units== 0 ) GeomXYZ_Change_Unit(FALSE);
	if(GeomIsOpen) create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

	if(GeomDrawingArea != NULL) rafresh_drawing();
	set_last_directory(FileName);
}
/********************************************************************************/
void read_geom_from_mopac_irc_output_file(gchar *FileName, gint numGeom)
{
	if(zmat_mopac_irc_output_file(FileName))
	{
		read_Zmat_from_mopac_irc_output_file(FileName, numGeom);
	}
	else 
		read_XYZ_from_mopac_irc_output_file(FileName, numGeom);
}
/********************************************************************************/
void read_XYZ_from_mopac_scan_output_file(gchar *FileName, gint numGeom)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[10];
	FILE *fd;
	guint taille=BSIZE;
	guint i,l;
	GeomXYZAtomDef* GeomXYZtemp=NULL;
	gint Ncent = 0;
	gint Nvar = 0;
	VariablesXYZDef* VariablesXYZtemp=NULL;
	gchar symb[20];
	gchar type[20];
	gchar charge[20];
	gint k1,k2,k3;
	gint j;

	for(i=0;i<10;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	fd = FOpen(FileName, "rb");
	if(fd == NULL)
	{
		t = g_strdup_printf(_("Sorry\n I can not open \"%s\" file"),FileName); 
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	t=g_malloc(taille);
	OK = TRUE;
	i = 0;
  	while(!feof(fd) )    
  	{
 		if(!fgets(t, taille, fd))break;
		if(
			strstr(t,"VARIABLE") && 
			strstr(t,"FUNCTION")
		) 
		{
			i++;
			if(i==numGeom)break;
		}
	}
	if(i==0) OK = FALSE;
 	if(OK && !fgets(t, taille, fd))OK = FALSE; /* Values */
  	while(!feof(fd) && OK )    
  	{
 		if(!fgets(t, taille, fd))OK = FALSE;
		if(t[0] != '*') break; /* Keywords */
	}
	if(!fgets(t,taille,fd)) OK = FALSE;
	if(!fgets(t,taille,fd)) OK = FALSE;
	if(!OK)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\n I can not read geometry from \"%s\" file"),FileName); 
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	}
	Ncent=0;
	GeomXYZtemp = NULL;
	
	j=0;
	while(!feof(fd) )
	{
		if(!fgets(t,taille,fd))break;
		if(this_is_a_backspace(t)) break;
                i = sscanf(t,"%s %s %d %s %d %s %d", AtomCoord[0],AtomCoord[1],&k1,AtomCoord[2],&k2,AtomCoord[3],&k3);
		if(i==0) break;
    		if( i== 7)
                {
			gint newVar = 0;

                        Ncent = j+1;
			GeomXYZtemp=g_realloc(GeomXYZtemp,Ncent*sizeof(GeomXYZAtomDef));
    			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			GeomXYZtemp[j].Nentry=NUMBER_LIST_XYZ;
			get_symb_type_charge(AtomCoord[0],symb,type,charge);
			{
				gint k;
				for(k=0;k<(gint)strlen(symb);k++) if(isdigit(symb[k])) symb[k] = ' ';
				delete_all_spaces(symb);
			}

    			GeomXYZtemp[j].Symb=g_strdup(symb);
			GeomXYZtemp[j].mmType=g_strdup(type);
			GeomXYZtemp[j].pdbType=g_strdup(type);
			GeomXYZtemp[j].Charge=g_strdup(charge);

    			GeomXYZtemp[j].Residue=g_strdup("DUM");
    			GeomXYZtemp[j].ResidueNumber=0;
    			GeomXYZtemp[j].X=g_strdup(AtomCoord[1]);
    			GeomXYZtemp[j].Y=g_strdup(AtomCoord[2]);
    			GeomXYZtemp[j].Z=g_strdup(AtomCoord[3]);
    			GeomXYZtemp[j].Layer=g_strdup(" ");
    			GeomXYZtemp[j].typeConnections = NULL;
			if(k1==1) newVar++;
			if(k2==1) newVar++;
			if(k3==1) newVar++;
			if(newVar>0)
			{
				gint n = Nvar+newVar;
  				if(n==1)
  					VariablesXYZtemp = g_malloc(n*sizeof(VariablesXYZDef));
  				else
  					VariablesXYZtemp = g_realloc(VariablesXYZtemp,n*sizeof(VariablesXYZDef));
			}
			newVar = 0;
			if(k1==1)
			{
				newVar++;
				trans_coordXYZ_geom('X', GeomXYZtemp, j, VariablesXYZtemp,  Nvar+newVar-1);
			}
			if(k2==1)
			{
				newVar++;
				trans_coordXYZ_geom('Y', GeomXYZtemp, j, VariablesXYZtemp,  Nvar+newVar-1);
			}
			if(k3==1)
			{
				newVar++;
				trans_coordXYZ_geom('Z', GeomXYZtemp, j, VariablesXYZtemp,  Nvar+newVar-1);
			}
			Nvar += newVar;
			j++;
		}
        }
	fseek(fd, 0L, SEEK_SET);
	if(j==0) OK = FALSE;
	else get_charge_and_multiplicity_from_mopac_output_file(fd);
	fclose(fd);

	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if( !OK || Ncent <1 )
	{
		FreeGeomXYZ(GeomXYZtemp,VariablesXYZtemp,Ncent, Nvar);
		MessageGeom(_("Sorry\n I can not read geometry in Mopac input file"),_("Error"),TRUE);
		return;
	}
	if(GeomXYZ) freeGeomXYZ();
	if(VariablesXYZ) freeVariablesXYZ();
	GeomXYZ = GeomXYZtemp;
	NcentersXYZ = Ncent;
	NVariablesXYZ = Nvar;
	VariablesXYZ = VariablesXYZtemp;
	MethodeGeom = GEOM_IS_XYZ;
	calculMMTypes(FALSE);
	if( Units== 0 ) GeomXYZ_Change_Unit(FALSE);
	if(GeomIsOpen) create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

	if(GeomDrawingArea != NULL) rafresh_drawing();
	set_last_directory(FileName);
}
/********************************************************************************/
void read_geom_from_mopac_scan_output_file(gchar *FileName, gint numGeom)
{
	if(zmat_mopac_scan_output_file(FileName))
		read_Zmat_from_mopac_scan_output_file(FileName, numGeom);
	else 
		read_XYZ_from_mopac_scan_output_file(FileName, numGeom);
}
/********************************************************************************/
void get_charge_and_multiplicity_from_mopac_aux_file(FILE* fd)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];

	TotalCharges[0] = 0;
	SpinMultiplicities[0] =1;

  	while(!feof(fd) )
	{
    		if(!fgets(t,taille,fd)) break;
		if ( strstr(t,"KEYWORDS="))
		{
			if(strstr(t,"CHARGE="))
			{
				gchar* p = strstr(t,"CHARGE=")+7;
				TotalCharges[0] = atoi(p);
			}
			if ( strstr(t,"SINGLET")) SpinMultiplicities[0] = 1;
			if ( strstr(t,"DOUBLET")) SpinMultiplicities[0] = 2;
			if ( strstr(t,"TRIPLET")) SpinMultiplicities[0] = 3;
			if ( strstr(t,"QUARTET")) SpinMultiplicities[0] = 4;
			if ( strstr(t,"QUINTET")) SpinMultiplicities[0] = 5;
			if ( strstr(t,"SEXTET")) SpinMultiplicities[0] = 6;
		}
	}
}
/********************************************************************************/
void get_charges_from_mopac_aux_file(FILE* fd,gint N)
{
	gchar** elements = NULL;
	gint n;
	gint i;

	for(i=0;i<N;i++)
		GeomXYZ[i].Charge = g_strdup("0.0");
	elements = get_one_block_from_aux_mopac_file(fd, "ATOM_CHARGES[",  &n);

	if(!elements) return;
	if(n<N)
	{
		free_one_string_table(elements, n);
		return;
	}
	for(i=0;i<N;i++)
	{
		g_free(GeomXYZ[i].Charge);
		GeomXYZ[i].Charge = g_strdup(elements[i]);
	}
	free_one_string_table(elements, n);
}
/********************************************************************************/
gint get_translation_vectors_from_mopac_aux_file(FILE* fd,gint N)
{
	gchar** elements = NULL;
	gint n;
	gint i,j,k;
	gint nTv = 0;
	gdouble Tv[3][3];

	elements = get_one_block_from_aux_mopac_file(fd, "TRANS_VECTS",  &n);
	if(!elements) return 0;
	nTv = n/3;
	if(nTv<1) { free_one_string_table(elements, n); return 0; }
	k = 0;
	for(i=0;i<nTv;i++)
	{
		for(j=0;j<3;j++)
		Tv[i][j]=atof(elements[k++]);
	}

	/*
	printf("nTv=%d\n",nTv);
	printf("Tv0=%f %f %f\n",Tv[0][0],Tv[0][1],Tv[0][2]);
	*/

	if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
	else GeomXYZ=g_realloc(GeomXYZ,(N+nTv)*sizeof(GeomXYZAtomDef));
	j = N;
	for(i=0;i<nTv;i++)
	{
   		GeomXYZ[j].typeConnections = NULL;
		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup("Tv");
		GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].ResidueNumber=0;
		if(Units == 0 )
		{
			GeomXYZ[j].X=g_strdup_printf("%0.14f",Tv[i][0]*BOHR_TO_ANG);
			GeomXYZ[j].Y=g_strdup_printf("%0.14f",Tv[i][1]*BOHR_TO_ANG);
			GeomXYZ[j].Z=g_strdup_printf("%0.14f",Tv[i][2]*BOHR_TO_ANG);
		}
		else
		{
			GeomXYZ[j].X=g_strdup_printf("%0.14f",Tv[i][0]);
			GeomXYZ[j].Y=g_strdup_printf("%0.14f",Tv[i][1]);
			GeomXYZ[j].Z=g_strdup_printf("%0.14f",Tv[i][2]);
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
		j++;
	}
	free_one_string_table(elements, n);
	return nTv;
}
/********************************************************************************/
void read_geom_from_mopac_aux_file(gchar *NomFichier, gint numgeometry)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j=0;
	gint l;
	gint numgeom;
	gchar *pdest;
	long int geomposok = 0;
	gint nElements = 0;
	gchar** elements = NULL;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	 
	t=g_malloc(taille*sizeof(gchar));
	fd = FOpen(NomFichier, "rb");
	if(fd ==NULL)
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
	 	MessageGeom(t,_("Error"),TRUE);
	 	g_free(t);
	 	return;
	}
	numgeom =0;
	OK=FALSE;
	elements = get_one_block_from_aux_mopac_file(fd, "ATOM_EL[",  &nElements);
	if(elements) OK = TRUE;
	 if(!OK) 
	 {
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not read the atom symbols in %s  file "),NomFichier);
	 	MessageGeom(t,_("Error"),TRUE);
	 	g_free(t);
	 	return;
	 }
	 while(!feof(fd))
	 {
		if(!fgets(t,taille,fd))break;
		if(numgeometry<0)
			pdest = strstr( t, "ATOM_X_OPT:ANGSTROMS");
		else
			pdest = strstr( t, "ATOM_X_UPDATED:ANGSTROMS");
		if ( pdest )
		{
			numgeom++;
			geomposok = ftell(fd);
			if(numgeom == numgeometry )
			{
				OK = TRUE;
				break;
			}
			if(numgeometry<0)
			{
				OK = TRUE;
			}
		}
	 }
	 if(numgeom == 0)
	 {
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not read geometry in	%s file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		free_one_string_table(elements, nElements);
		return;
	  }
	j=-1;
	fseek(fd, geomposok, SEEK_SET);
	while(!feof(fd) )
	{
		if(!fgets(t,taille,fd))break;
		if(strstr( t, "[")
		  || strstr(t,"HEAT_OF_FORM_UPDATED")
		  || strstr(t,"TRANS_VECTS")
		  || strstr( t, "####################################")
		  ||this_is_a_backspace(t))
		{
			if(numgeometry<0)
			{
				fseek(fd, geomposok, SEEK_SET);
				get_charges_from_mopac_aux_file(fd,j+1);
				if(j+1>0)
				{
					NcentersXYZ = j+1;
					compute_dipole_using_charges_of_xyz_geom();
				}
				/* Only module of dipole is available in aux mopac file*/
			}
			break;
		}
		if(j+1>nElements)break;
		j++;
		if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
		else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
   		GeomXYZ[j].typeConnections = NULL;

		sscanf(t,"%s %s %s",AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		if(j<nElements) sprintf(AtomCoord[0],"%s",elements[j]);
		else sprintf(AtomCoord[0],"X");
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
		l=strlen(AtomCoord[0]); 
		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
		if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);

		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(t);
		GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].ResidueNumber=0;
		if(Units == 0 )
		{
			GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
			GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
			GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
		}
		else
		{
			GeomXYZ[j].X=g_strdup(AtomCoord[1]);
			GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
			GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
	  }
	if(j+1>0) 
	{
		fseek(fd, geomposok, SEEK_SET);
		j += get_translation_vectors_from_mopac_aux_file(fd,j+1);
		fseek(fd, 0, SEEK_SET);
		get_charge_and_multiplicity_from_mopac_aux_file(fd);
	}
	 fclose(fd);
	 NcentersXYZ = j+1;
 	 calculMMTypes(FALSE);
	 g_free(t);
	 for(i=0;i<5;i++) g_free(AtomCoord[i]);
	 if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	 {
	 	clearList(list);
		append_list();
	 }
	 MethodeGeom = GEOM_IS_XYZ;
	 if(GeomDrawingArea != NULL) rafresh_drawing();
	 if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
	free_one_string_table(elements, nElements);
}
/********************************************************************************/
void get_charges_from_orca_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


	for(i=0;i<N;i++)
		GeomXYZ[i].Charge = g_strdup("0.0");

  	while(!feof(fd) )
	{
    		pdest = NULL;
    		{ char* e = fgets(t,taille,fd);}
		if(strstr(t,"GEOMETRY OPTIMIZATION CYCLE")) break;
    		pdest = strstr( t, "MULLIKEN ATOMIC CHARGES");
		if(pdest)
		{
			gboolean OK = FALSE;
  			while(!feof(fd) )
			{
    				if(!fgets(t,taille,fd)) break;
				if(strstr(t,"----------------"))
				{
					OK = TRUE;
					break;
				}
			}
			if(!OK) break;

			for(i=0;i<N;i++)
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
				else break;
				if(sscanf(t,"%s %s %s %s",dump,dump,dump,d)==4)
				{
					g_free(GeomXYZ[i].Charge);
					GeomXYZ[i].Charge = g_strdup(d);
				}
			}
			break;
		}
	}
}
/********************************************************************************/
void read_geom_from_orca_file(gchar *NomFichier, gint numgeometry)
{
	gchar *t;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j=0;
	gint l;
	gint numgeom;
	gchar *pdest;
	long int geomposok = 0;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	 
	t=g_malloc(taille*sizeof(gchar));
	fd = FOpen(NomFichier, "rb");
	if(fd ==NULL)
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
	 	MessageGeom(t,_("Error"),TRUE);
	 	g_free(t);
	 	return;
	}
	numgeom =0;
	 while(!feof(fd))
	 {
		if(!fgets(t,taille,fd))break;
		pdest = strstr( t, "CARTESIAN COORDINATES (ANGSTROEM)");
		if(pdest) 
		{
			if(!fgets(t,taille,fd))break;
			pdest = strstr( t, "---------------------------------");
		}
		if ( pdest )
		{
			numgeom++;
			geomposok = ftell(fd);
			if(numgeom == numgeometry )
			{
				break;
			}
		}
	 }
	 if(numgeom == 0)
	 {
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not read geometry in	%s file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	  }
	j=-1;
	fseek(fd, geomposok, SEEK_SET);
	while(!feof(fd) )
	{
		if(!fgets(t,taille,fd))break;
		pdest = strstr( t, "----------------------------------" );
		if (pdest || this_is_a_backspace(t))
		{
			geomposok = ftell(fd);
			get_charges_from_orca_output_file(fd,j+1);
			fseek(fd, geomposok, SEEK_SET);
 			get_dipole_from_orca_output_file(fd);
			break;
		}
		j++;
		if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
		else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
   		GeomXYZ[j].typeConnections = NULL;

		sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
		l=strlen(AtomCoord[0]); 
		if(isdigit(AtomCoord[0][1]))l=1;
		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
		if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);
		/* if(!strcmp(t,"-")) sprintf(t,"%c",'X');*/
		if(!strcmp(t,"-")) { j--;continue;}

		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(t);
		GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].ResidueNumber=0;
		if(Units == 0 )
		{
			GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
			GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
			GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
		}
		else
		{
			GeomXYZ[j].X=g_strdup(AtomCoord[1]);
			GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
			GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
	  }
	NcentersXYZ = j+1;

	 fclose(fd);
 	 calculMMTypes(FALSE);
	 g_free(t);
	 for(i=0;i<5;i++) g_free(AtomCoord[i]);
	 if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	 {
	 	clearList(list);
		append_list();
	 }
	 MethodeGeom = GEOM_IS_XYZ;
	 if(GeomDrawingArea != NULL) rafresh_drawing();
	 if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
}
/********************************************************************************/
void read_first_orca_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_orca_file(NomFichier, 1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_last_orca_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_orca_file(NomFichier, -1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_first_nwchem_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_nwchem_file(NomFichier, 1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_last_nwchem_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_nwchem_file(NomFichier, -1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_first_psicode_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_psicode_file(NomFichier, 1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_last_psicode_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_psicode_file(NomFichier, -1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_first_qchem_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_qchem_file(NomFichier, 1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_last_qchem_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_qchem_file(NomFichier, -1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_first_mopac_output_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_mopac_output_file(NomFichier, 1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_last_mopac_output_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_mopac_output_file(NomFichier, -1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_last_mopac_aux_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_mopac_aux_file(NomFichier, -1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
gint get_translation_vectors_from_vasp_file(FILE* fd,gint N)
{
	gchar** elements = NULL;
	gint n;
	gint i,j,k;
	gint nTv = 0;
	gdouble Tv[3][3];
	gchar t[BSIZE];
	gint size=BSIZE;
	gchar* pdest = NULL;

	for(i=0;i<3;i++)
	{
                if(!fgets(t,size,fd))break;
		k = sscanf(t,"%lf %lf %lf",&Tv[i][0],&Tv[i][1],&Tv[i][2]);
		if(k!=3) break;
		nTv++;
	}
	if(nTv<1) { return 0; }
	/*
	printf("nTv=%d\n",nTv);
	printf("Tv0=%f %f %f\n",Tv[0][0],Tv[0][1],Tv[0][2]);
	*/

	if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
	else GeomXYZ=g_realloc(GeomXYZ,(N+nTv)*sizeof(GeomXYZAtomDef));
	j = N;
	for(i=0;i<nTv;i++)
	{
   		GeomXYZ[j].typeConnections = NULL;
		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup("Tv");
		GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].ResidueNumber=0;
		if(Units == 0 )
		{
			GeomXYZ[j].X=g_strdup_printf("%0.14f",Tv[i][0]*BOHR_TO_ANG);
			GeomXYZ[j].Y=g_strdup_printf("%0.14f",Tv[i][1]*BOHR_TO_ANG);
			GeomXYZ[j].Z=g_strdup_printf("%0.14f",Tv[i][2]*BOHR_TO_ANG);
		}
		else
		{
			GeomXYZ[j].X=g_strdup_printf("%0.14f",Tv[i][0]);
			GeomXYZ[j].Y=g_strdup_printf("%0.14f",Tv[i][1]);
			GeomXYZ[j].Z=g_strdup_printf("%0.14f",Tv[i][2]);
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
		j++;
	}
	return nTv;
}
/********************************************************************************/
void get_charges_from_vasp_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;

	for(i=0;i<N;i++) GeomXYZ[i].Charge = g_strdup("0.0");
	/* TO DO */
	return;

}
/********************************************************************************/
void read_geom_from_vasp_file(gchar *NomFichier, gint numgeometry)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	guint k;
	gint j=0;
	gint l;
	gint numgeom;
	gchar *pdest;
	long int geomposok = 0;
	long int latticeposok = -1;
	gint ntypes = 0;
	gchar** symbols = NULL;
	gint* nntypes = NULL;
	gchar told[20];
	gchar t1[20];
	gchar t2[20];
	gchar t3[BSIZE];
	gchar** listTypes = NULL;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));
	 
	t=g_malloc(taille*sizeof(gchar));
	fd = FOpen(NomFichier, "rb");
	if(fd ==NULL)
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not open %s  file "),NomFichier);
	 	MessageGeom(t,_("Error"),TRUE);
	 	g_free(t);
	 	return;
	}
	/* number atom types */
	OK=FALSE;

	ntypes = 0;
	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
		pdest = strstr( t, "INCAR:");
		if(pdest)
		{
			
			geomposok = ftell(fd);
			while(!feof(fd))
			{
				if(!fgets(t,taille,fd))break;
				if(strstr( t, "POTCAR:")) 
				{
					gboolean ok = TRUE;
					for(j=0;j<ntypes;j++) if(!strcmp(t,listTypes[j])) { ok = FALSE; break;}
					if(!ok) break;
					ntypes++;
					listTypes = g_realloc(listTypes,ntypes*sizeof(gchar*));
					listTypes[ntypes-1] = g_strdup(t);
				}
				else break;
			}
			break;
		}
	}
	if(listTypes) for(j=0;j<ntypes;j++) if(listTypes[j]) g_free(listTypes[j]);
	if(listTypes) g_free(listTypes);
	listTypes = NULL;
	
	if(ntypes<1)
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not read the type of atoms from %s file "),NomFichier);
	 	MessageGeom(t,_("Error"),TRUE);
	 	g_free(t);
	 	return;
	}
	fseek(fd, geomposok, SEEK_SET);
	symbols=g_malloc(ntypes*sizeof(gchar*)); 
	nntypes=g_malloc(ntypes*sizeof(gint)); 
	/* printf("ntypes = %d\n",ntypes);*/
	for(i=0;i<ntypes;i++) { symbols[i]=NULL; nntypes[i]=0;}
	for(i=0;i<ntypes;i++) 
	{
		if(!fgets(t,taille,fd))break;
		if(3==sscanf(t,"%s %s %s",t1,t2,t3))
		{
			if(strlen(t3)>2) t3[2]='\0';
			symbols[i]=g_strdup(t3);
			/* printf("symb =%s\n",symbols[i]);*/
		}
	}
	/*printf("Ok end symbols\n");*/
	OK = TRUE;
	for(i=0;i<ntypes;i++) { if(symbols[i]==NULL) {OK = FALSE; break; }}
	if(!OK)
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not read the type of atoms from %s file, check your file ! "),NomFichier);
	 	MessageGeom(t,_("Error"),TRUE);
	 	g_free(t);
		g_free(symbols);
		g_free(nntypes);
	 	return;
	}

	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
		if(strstr( t, "ions per type ="))
		{
			gchar** split = NULL;
			gchar* tt = strstr(t,"ions per type =")+strlen("ions per type =");
			gint nA=0;
			split = gab_split(tt);
			while(split && split[nA]!=NULL) nA++;
			if(nA==ntypes) for(i=0;i<ntypes;i++) nntypes[i] = atoi(split[i]);
			g_strfreev(split);
			break;
		}
	}
	OK = TRUE;
	for(i=0;i<ntypes;i++) { if(nntypes[i]==0) {OK = FALSE; break; }}
	if(!OK)
	{
	 	g_free(t);
	 	t = g_strdup_printf(_("Sorry\nI can not read the number of atoms by type from %s file, check your file ! "),NomFichier);
	 	MessageGeom(t,_("Error"),TRUE);
	 	g_free(t);
		g_free(symbols);
		g_free(nntypes);
	 	return;
	}
	/* printf("Ok end nntypes\n");*/
	
	numgeom =0;
	OK=FALSE;
	 while(!feof(fd))
	 {
		if(!fgets(t,taille,fd))break;
                if(strstr(t, "direct lattice vectors")) latticeposok = ftell(fd);
		pdest = strstr( t, "POSITION");
		if(strstr( t, "POSITION") && strstr( t, "TOTAL-FORCE"))
		{
			if(!fgets(t,taille,fd))break;
			pdest = strstr( t, "---------------------------------");
		}
		if (pdest)
		{
			numgeom++;
			geomposok = ftell(fd);
			if(numgeom == numgeometry )
			{
				OK = TRUE;
				break;
			}
			if(numgeometry<0)
			{
				OK = TRUE;
			}
		}
	 }
	 if(numgeom == 0)
	 {
		g_free(t);
		t = g_strdup_printf(_("Sorry\nI can not read geometry in %s file "),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		g_free(t);
		return;
	  }
	j=-1;
	fseek(fd, geomposok, SEEK_SET);
	for(i=0;i<ntypes;i++)
	for(k=0;k<nntypes[i];k++)
	{
		j++;
		if(!fgets(t,taille,fd))break;
		if(GeomXYZ==NULL) GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
		else GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
   		GeomXYZ[j].typeConnections = NULL;

		sscanf(t,"%s %s %s",AtomCoord[1],AtomCoord[2],AtomCoord[3]);

		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(symbols[i]);
		GeomXYZ[j].mmType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].pdbType=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].Residue=g_strdup(GeomXYZ[j].Symb);
		GeomXYZ[j].ResidueNumber=0;
		if(Units == 0 )
		{
			GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
			GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
			GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
		}
		else
		{
			GeomXYZ[j].X=g_strdup(AtomCoord[1]);
			GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
			GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
	}
	/* printf("Ok end geom j = %d\n",j);*/
	geomposok = ftell(fd);
	get_charges_from_vasp_output_file(fd,j+1);
	fseek(fd, geomposok, SEEK_SET);
 	get_dipole_from_vasp_output_file(fd);
	if(latticeposok>-1)
	{
		fseek(fd, latticeposok, SEEK_SET);
		/* printf("%ld %ld\n",geomposok,latticeposok);*/
		j += get_translation_vectors_from_vasp_file(fd,j+1);
	}
	NcentersXYZ = j+1;

	fclose(fd);
 	calculMMTypes(FALSE);
	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if(GeomIsOpen && MethodeGeom == GEOM_IS_XYZ)
	{
		clearList(list);
		append_list();
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS && GeomIsOpen) set_spin_of_electrons();
	g_free(symbols);
	g_free(nntypes);
}
/********************************************************************************/
void read_first_vasp_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_vasp_file(NomFichier, 1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_last_vasp_file(GabeditFileChooser *SelecFile , gint response_id)
{
	gchar *NomFichier;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
	
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}
	read_geom_from_vasp_file(NomFichier, -1);
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_first_molcas_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar t[BSIZE];
	gchar *NomFichier;
	gboolean OK;
	gchar symb[100];
	gchar x[100];
	gchar y[100];
	gchar z[100];
	FILE *file;
	guint idummy;
	gint i;
	gint j=0;
	gint l;

 	if(response_id != GTK_RESPONSE_OK) return;
 	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}

	file = FOpen(NomFichier, "rb");
	if(file ==NULL)
	{
		MessageGeom(_("Sorry\nI can not open this file"),_("Error"),TRUE);
		return;
	}
	OK=FALSE;
	while(!feof(file))
	{
    		{ char* e = fgets(t,BSIZE,file);}
		if (strstr(t," Cartesian coordinates:"))
		{
    			{ char* e = fgets(t,BSIZE,file);}
    			{ char* e = fgets(t,BSIZE,file);}
    			{ char* e = fgets(t,BSIZE,file);}
                	OK = TRUE;
	  		break;
	 	}
	}
	if(!OK)
	{
  		MessageGeom(_("Sorry\nI can not read this file"),_("Error"),TRUE);
		return;
	}

	j=-1;
	while(!feof(file) )
	{
    		{ char* e = fgets(t,BSIZE,file);}
		if (strstr(t,"-----------------------------------------"))
		{
			/*
			get_dipole_from_molcas_output_file(file);
			*/
			break;
		}
		j++;
		if(GeomXYZ==NULL)GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
		else
			GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
   		GeomXYZ[j].typeConnections = NULL;

		sscanf(t,"%d %s %s %s %s",&idummy,symb,x,y,z);

		for(i=0;i<(gint)strlen(symb);i++)
			if(isdigit(symb[i])) symb[i] = ' ';
		delete_all_spaces(symb);

		symb[0]=toupper(symb[0]);
		l=strlen(symb);
		for(i=1;i<l;i++)
	 		symb[i]=tolower(symb[i]);

		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(symb);
		GeomXYZ[j].mmType=g_strdup(symb);
		GeomXYZ[j].pdbType=g_strdup(symb);
		GeomXYZ[j].Residue=g_strdup(symb);
		GeomXYZ[j].ResidueNumber=0;
		if(Units == 1 )
		{
			GeomXYZ[j].X=g_strdup(bohr_to_ang(x));
			GeomXYZ[j].Y=g_strdup(bohr_to_ang(y));
			GeomXYZ[j].Z=g_strdup(bohr_to_ang(z));
		}
		else
		{
			GeomXYZ[j].X=g_strdup(x);
			GeomXYZ[j].Y=g_strdup(y);
			GeomXYZ[j].Z=g_strdup(z);
		}
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].Layer=g_strdup(" ");
	}
	fclose(file);
 	calculMMTypes(FALSE);
	NcentersXYZ = j+1;

	if(GeomIsOpen)
	{
		append_list();
	}
	if(GeomDrawingArea != NULL) rafresh_drawing();
	if(iprogram == PROG_IS_GAUSS) set_spin_of_electrons();
	set_last_directory(NomFichier);
}
/********************************************************************************/
void read_first_molpro_file(GabeditFileChooser *SelecFile, gint response_id)
{
 gchar *t;
 gchar *NomFichier;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 guint idummy;
 guint i;
 gint j=0;
 gint l;

 if(response_id != GTK_RESPONSE_OK) return;

 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(char));
 NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
  
 if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
 {
	MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
    return ;
 }

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 if(fd ==NULL)
 {
  MessageGeom(_("Sorry\nI can not open this file"),_("Error"),TRUE);
  return;
 }
 OK=FALSE;
 while(!feof(fd)){
    	  { char* e = fgets(t,taille,fd);}
	 if ( !strcmp(t," ATOMIC COORDINATES\n"))
	  {
    	  	{ char* e = fgets(t,taille,fd);}
    	  	{ char* e = fgets(t,taille,fd);}
    	  	{ char* e = fgets(t,taille,fd);}
                OK = TRUE;
	  	break;
	  }
        }
 if(!OK){
  	MessageGeom(_("Sorry\nI can not read this file"),_("Error"),TRUE);
	return;
    }

  j=-1;
  while(!feof(fd) )
  {
    { char* e = fgets(t,taille,fd);}
    if ( !strcmp(t,"\n"))
    {
 	get_dipole_from_molpro_output_file(fd);
      	break;
    }
    j++;
    if(GeomXYZ==NULL)GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
    else
    GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
    GeomXYZ[j].typeConnections = NULL;

    sscanf(t,"%d %s %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[1],AtomCoord[2],AtomCoord[3]);

	{
		gint k;
		for(k=0;k<(gint)strlen(AtomCoord[0]);k++) if(isdigit(AtomCoord[0][k])) AtomCoord[0][k] = ' ';
		delete_all_spaces(AtomCoord[0]);
	}

	AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 l=strlen(AtomCoord[0]);
          if (l==2)
	 	AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
    GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);
    GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
    GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
    GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
    GeomXYZ[j].ResidueNumber=0;
    if(Units == 1 )
    {
     GeomXYZ[j].X=g_strdup(bohr_to_ang(AtomCoord[1]));
     GeomXYZ[j].Y=g_strdup(bohr_to_ang(AtomCoord[2]));
     GeomXYZ[j].Z=g_strdup(bohr_to_ang(AtomCoord[3]));
    }
    else
    {
     GeomXYZ[j].X=g_strdup(AtomCoord[1]);
     GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
     GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
    }
    GeomXYZ[j].Charge=g_strdup("0.0");
    GeomXYZ[j].Layer=g_strdup(" ");
  }

 fclose(fd);
 calculMMTypes(FALSE);
 NcentersXYZ = j+1;
 g_free(t);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if(GeomIsOpen)
 	append_list();
 if(GeomDrawingArea != NULL)
	rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();
 set_last_directory(NomFichier);
}
/********************************************************************************/
static void GeomXYZ_Change_Unit(gboolean toang)
{
 guint i;
  for(i=0;i<NcentersXYZ;i++)
  {
        if(test(GeomXYZ[i].X))
        {
        	if(toang)
		GeomXYZ[i].X=bohr_to_ang(GeomXYZ[i].X);
 		else
		GeomXYZ[i].X=ang_to_bohr(GeomXYZ[i].X);
        }
        if(test(GeomXYZ[i].Y))
        {
        	if(toang)
		GeomXYZ[i].Y=bohr_to_ang(GeomXYZ[i].Y);
 		else
		GeomXYZ[i].Y=ang_to_bohr(GeomXYZ[i].Y);
        }
        if(test(GeomXYZ[i].Z))
        {
        	if(toang)
		GeomXYZ[i].Z=bohr_to_ang(GeomXYZ[i].Z);
 		else
		GeomXYZ[i].Z=ang_to_bohr(GeomXYZ[i].Z);
        }
  }
/*  Variables */
  for(i=0;i<NVariablesXYZ;i++)
  {
        if(toang)
		VariablesXYZ[i].Value =bohr_to_ang( VariablesXYZ[i].Value);
 	else
		VariablesXYZ[i].Value =ang_to_bohr( VariablesXYZ[i].Value);
  }
}
/*************************************************************************************/
void read_XYZ_from_gamess_input_file(gchar *fileName)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	gint i;
	gint j;
	gint l;
	GeomXYZAtomDef* GeomXYZtemp=NULL;
	gint Ncent = 0;
	VariablesXYZDef* VariablesXYZtemp=NULL;
	gchar symb[BSIZE];
	gchar type[BSIZE];
	gchar charge[BSIZE];
	gboolean unitAng = TRUE;
 
	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));

	t=g_malloc(taille);
	fd = FOpen(fileName, "rb");
	OK=TRUE;
	if(fd!=NULL)
	{
		while(!feof(fd) && OK )
		{
    			if(!fgets(t,taille,fd))
			{
				OK = FALSE;
				break;
			};
			if(strstr(t,"molecule") && strstr(t,":"))
			{
				if(strstr(t,"<") && strstr(t,">"))
				{
					while(!feof(fd) && OK )
					{
    						if(!fgets(t,taille,fd))
						{
							OK = FALSE;
							break;
						};
						if(strstr(t,"unit"))
							if(strstr(t,"bohr")) unitAng=FALSE;
						if (strstr(t,"atoms") && strstr(t,"geometry")) break;
					}
				}
				break;
			}
		}
		if(OK) GeomXYZtemp=g_malloc(sizeof(GeomXYZAtomDef));
		j=-1;
		while(!feof(fd) && OK )
		{
    			j++;
    			{ char* e = fgets(t,taille,fd);}
			if(this_is_a_backspace(t)) break;
			if(strstr(t,"}")) break;
    			i = sscanf(t,"%s ",AtomCoord[0]);
                	if(i != 1)
			{
				OK = FALSE;
				break;
			}
			for(i=0;i<(gint)strlen(t);i++) if(t[i]=='[' || t[i] ==']') t[i]=' ';
               		i = sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);

    			if( i==4)
                	{
                        	Ncent = j+1;
				GeomXYZtemp=g_realloc(GeomXYZtemp,Ncent*sizeof(GeomXYZAtomDef));
    				AtomCoord[0][0]=toupper(AtomCoord[0][0]);
    				l=strlen(AtomCoord[0]);
      				if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    				GeomXYZtemp[j].Nentry=NUMBER_LIST_XYZ;
				get_symb_type_charge(AtomCoord[0],symb,type,charge);

    				GeomXYZtemp[j].Symb=g_strdup(symb);
				GeomXYZtemp[j].mmType=g_strdup(type);
				GeomXYZtemp[j].pdbType=g_strdup(type);
				GeomXYZtemp[j].Charge=g_strdup(charge);

    				GeomXYZtemp[j].Residue=g_strdup("DUM");
    				GeomXYZtemp[j].ResidueNumber=0;
    				GeomXYZtemp[j].X=g_strdup_printf("%f",atof(AtomCoord[1]));
    				GeomXYZtemp[j].Y=g_strdup_printf("%f",atof(AtomCoord[2]));
    				GeomXYZtemp[j].Z=g_strdup_printf("%f",atof(AtomCoord[3]));

    				GeomXYZtemp[j].Layer=g_strdup(" ");
    				GeomXYZtemp[j].typeConnections = NULL;
			}
               		else OK = FALSE;
  		}
		fclose(fd);
	}
	else OK = FALSE;

	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if( !OK || Ncent <1 )
	{
   		FreeGeomXYZ(GeomXYZtemp,VariablesXYZtemp,Ncent, 0);
   		MessageGeom(_("Sorry\n I can not read geometry from your Gamess input file"),_("Error"),TRUE);
		return;
	}
	if(GeomXYZ) freeGeomXYZ();
	if(VariablesXYZ) freeVariablesXYZ();

	GeomXYZ = GeomXYZtemp;
	NcentersXYZ = Ncent;
	NVariablesXYZ = 0;
	VariablesXYZ = VariablesXYZtemp;
	MethodeGeom = GEOM_IS_XYZ;
 	calculMMTypes(FALSE);

	if( unitAng && Units== 0 ) GeomXYZ_Change_Unit(FALSE);
	else if( !unitAng && Units== 1 ) GeomXYZ_Change_Unit(TRUE);

	if(GeomIsOpen) create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

	if(GeomDrawingArea != NULL) rafresh_drawing();
	set_last_directory(fileName);
}
/*************************************************************************************/
void read_XYZ_from_mpqc_input_file(gchar *fileName)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	gint i;
	gint j;
	gint l;
	GeomXYZAtomDef* GeomXYZtemp=NULL;
	gint Ncent = 0;
	VariablesXYZDef* VariablesXYZtemp=NULL;
	gchar symb[BSIZE];
	gchar type[BSIZE];
	gchar charge[BSIZE];
	gboolean unitAng = TRUE;
 
	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));

	t=g_malloc(taille);
	fd = FOpen(fileName, "rb");
	OK=TRUE;
	if(fd!=NULL)
	{
		while(!feof(fd) && OK )
		{
    			if(!fgets(t,taille,fd))
			{
				OK = FALSE;
				break;
			};
			if(strstr(t,"molecule") && strstr(t,":"))
			{
				if(strstr(t,"<") && strstr(t,">"))
				{
					while(!feof(fd) && OK )
					{
    						if(!fgets(t,taille,fd))
						{
							OK = FALSE;
							break;
						};
						if(strstr(t,"unit"))
							if(strstr(t,"bohr")) unitAng=FALSE;
						if (strstr(t,"atoms") && strstr(t,"geometry")) break;
					}
				}
				break;
			}
		}
		if(OK) GeomXYZtemp=g_malloc(sizeof(GeomXYZAtomDef));
		j=-1;
		while(!feof(fd) && OK )
		{
    			j++;
    			{ char* e = fgets(t,taille,fd);}
			if(this_is_a_backspace(t)) break;
			if(strstr(t,"}")) break;
    			i = sscanf(t,"%s ",AtomCoord[0]);
                	if(i != 1)
			{
				OK = FALSE;
				break;
			}
			for(i=0;i<(gint)strlen(t);i++) if(t[i]=='[' || t[i] ==']') t[i]=' ';
               		i = sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);

    			if( i==4)
                	{
                        	Ncent = j+1;
				GeomXYZtemp=g_realloc(GeomXYZtemp,Ncent*sizeof(GeomXYZAtomDef));
    				AtomCoord[0][0]=toupper(AtomCoord[0][0]);
    				l=strlen(AtomCoord[0]);
      				if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    				GeomXYZtemp[j].Nentry=NUMBER_LIST_XYZ;
				get_symb_type_charge(AtomCoord[0],symb,type,charge);

    				GeomXYZtemp[j].Symb=g_strdup(symb);
				GeomXYZtemp[j].mmType=g_strdup(type);
				GeomXYZtemp[j].pdbType=g_strdup(type);
				GeomXYZtemp[j].Charge=g_strdup(charge);

    				GeomXYZtemp[j].Residue=g_strdup("DUM");
    				GeomXYZtemp[j].ResidueNumber=0;
    				GeomXYZtemp[j].X=g_strdup_printf("%f",atof(AtomCoord[1]));
    				GeomXYZtemp[j].Y=g_strdup_printf("%f",atof(AtomCoord[2]));
    				GeomXYZtemp[j].Z=g_strdup_printf("%f",atof(AtomCoord[3]));

    				GeomXYZtemp[j].Layer=g_strdup(" ");
    				GeomXYZtemp[j].typeConnections = NULL;
			}
               		else OK = FALSE;
  		}
		fclose(fd);
	}
	else OK = FALSE;

	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if( !OK || Ncent <1 )
	{
   		FreeGeomXYZ(GeomXYZtemp,VariablesXYZtemp,Ncent, 0);
   		MessageGeom(_("Sorry\n I can not read geometry from your MPQC input file"),_("Error"),TRUE);
		return;
	}
	if(GeomXYZ) freeGeomXYZ();
	if(VariablesXYZ) freeVariablesXYZ();

	GeomXYZ = GeomXYZtemp;
	NcentersXYZ = Ncent;
	NVariablesXYZ = 0;
	VariablesXYZ = VariablesXYZtemp;
	MethodeGeom = GEOM_IS_XYZ;
 	calculMMTypes(FALSE);

	if( unitAng && Units== 0 ) GeomXYZ_Change_Unit(FALSE);
	else if( !unitAng && Units== 1 ) GeomXYZ_Change_Unit(TRUE);

	if(GeomIsOpen) create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

	if(GeomDrawingArea != NULL) rafresh_drawing();
	set_last_directory(fileName);
}
/*************************************************************************************/
void read_XYZ_from_molpro_input_file(gchar *NomFichier, FilePosTypeGeom InfoFile )
{
 gchar *t;
 gchar *t1;
 gchar *t2;
 gchar *t3;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 guint i;
 gint j;
 gint l;
 gint Kvar=0;
 gboolean Uvar=FALSE;
 GeomXYZAtomDef* GeomXYZtemp=NULL;
 gint Ncent = 0;
 gint Nvar = 0;
 VariablesXYZDef* VariablesXYZtemp=NULL;
 
 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(char));
 

 t=g_malloc(taille);
/* Read Geomery */
 fd = FOpen(NomFichier, "rb");
 OK=TRUE;
 if(fd!=NULL)
 {
	for(i=0;(gint)i<InfoFile.numline;i++) { char* e = fgets(t,taille,fd);}
	GeomXYZtemp=g_malloc(sizeof(GeomXYZAtomDef));
    	{ char* e = fgets(t,taille,fd);}
  	Ncent=atoi(t);
  	if(Ncent<1)
  	{
		MessageGeom(_("Sorry\n i can not read XYZ geometry in molpro input file"),_("Error"),TRUE);
		return;
  	}
	init_dipole();
    	{ char* e = fgets(t,taille,fd);}
	GeomXYZtemp=g_malloc(Ncent*sizeof(GeomXYZAtomDef));
	for(j = 0;j<Ncent ;j++)
        {
    		GeomXYZtemp[j].Symb=NULL;
    		GeomXYZtemp[j].mmType=NULL;
    		GeomXYZtemp[j].pdbType=NULL;
    		GeomXYZtemp[j].X=NULL;
    		GeomXYZtemp[j].Y=NULL;
    		GeomXYZtemp[j].Z=NULL;
    		GeomXYZtemp[j].Charge=NULL;
    		GeomXYZtemp[j].Layer=NULL;
    		GeomXYZtemp[j].typeConnections = NULL;
        }
	Kvar = 0;
	for(j = 0;j<Ncent && OK ;j++)
        {
    		{ char* e = fgets(t,taille,fd);}
		i = sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]) ;
    		if( i != EOF && i == 4 )
                {
			DeleteLastChar(AtomCoord[0]);
			DeleteLastChar(AtomCoord[1]);
			DeleteLastChar(AtomCoord[2]);

			{
				gint k;
				for(k=0;k<(gint)strlen(AtomCoord[0]);k++) if(isdigit(AtomCoord[0][k])) AtomCoord[0][k] = ' ';
				delete_all_spaces(AtomCoord[0]);
			}
    			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
    			l=strlen(AtomCoord[0]);
      			if (l==2)
	 			AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    			GeomXYZtemp[j].Nentry=NUMBER_LIST_XYZ;
    			GeomXYZtemp[j].Symb=g_strdup(AtomCoord[0]);
    			GeomXYZtemp[j].mmType=g_strdup(AtomCoord[0]);
    			GeomXYZtemp[j].pdbType=g_strdup(AtomCoord[0]);
    			GeomXYZtemp[j].Residue=g_strdup(AtomCoord[0]);
    			GeomXYZtemp[j].ResidueNumber=0;
                        if(!test(AtomCoord[1]) || !test(AtomCoord[2]) || !test(AtomCoord[3]))
                              Uvar = TRUE;
                        if(!test(AtomCoord[1])) 
				Kvar++;
                        if(!test(AtomCoord[2])) 
				Kvar++;
                        if(!test(AtomCoord[3])) 
				Kvar++;
    			GeomXYZtemp[j].X=g_strdup(AtomCoord[1]);
    			GeomXYZtemp[j].Y=g_strdup(AtomCoord[2]);
    			GeomXYZtemp[j].Z=g_strdup(AtomCoord[3]);

    			GeomXYZtemp[j].Charge=g_strdup("0.0");
    			GeomXYZtemp[j].Layer=g_strdup(" ");
		}
               else
                {
                 OK = FALSE;
                 break;
                }
  	}
  fclose(fd);
 }
 else
      OK = FALSE;

/* Read Variables */
 if(OK && Uvar )
 {
	Nvar  = 0;
 	fd = FOpen(NomFichier, "rb");
 	if(fd!=NULL)
 	{
               	t2= g_strdup("Variables");
               	uppercase(t2);
		while( !feof(fd) )
		{
    			{ char* e = fgets(t,taille,fd);}
                	t1 = g_strdup(t);
               		uppercase(t1);
			t3 = strstr(t1,t2);
			if(t3 != NULL)
				break;
			g_free(t1);
		}
		g_free(t2);
		while( !feof(fd) )
		{
    			{ char* e = fgets(t,taille,fd);}
                	i = sscanf(t,"%s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2]);
    			if( i != EOF && i == 3)
                	{
				Nvar++;
				if(Nvar == 1)
					VariablesXYZtemp=g_malloc(sizeof(VariablesXYZDef));
                                else
					VariablesXYZtemp=g_realloc(VariablesXYZtemp,Nvar*sizeof(VariablesXYZDef));
				DeleteLastChar(AtomCoord[2]);
				VariablesXYZtemp[Nvar-1].Name = g_strdup(AtomCoord[0]);
				VariablesXYZtemp[Nvar-1].Value = g_strdup(AtomCoord[2]);
				VariablesXYZtemp[Nvar-1].Used = TRUE;
			}
			else 
				break;
		}
		
        fclose(fd);      
	}
 }
 g_free(t);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if( !OK || Ncent <1 || Nvar < Kvar)
 {
 	FreeGeomXYZ(GeomXYZtemp,VariablesXYZtemp,Ncent, Nvar);
   	MessageGeom(_("Sorry\n I can not read geometry in Molpro input file"),_("Error"),TRUE);
   	return;
 }
 if(GeomXYZ)
   freeGeomXYZ();
 if(VariablesXYZ)
	freeVariablesXYZ();
 GeomXYZ = GeomXYZtemp;
 NcentersXYZ = Ncent;
 NVariablesXYZ = Nvar;
 VariablesXYZ = VariablesXYZtemp;
 MethodeGeom = GEOM_IS_XYZ;
 calculMMTypes(FALSE);
 if( InfoFile.units== 1 && Units== 0 )
 	GeomXYZ_Change_Unit(FALSE);
 else
 if( InfoFile.units== 0 && Units== 1 )
 	GeomXYZ_Change_Unit(TRUE);
 if(GeomIsOpen)
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

 if(GeomDrawingArea != NULL)
	rafresh_drawing();
 set_last_directory(NomFichier);
}
/*************************************************************************************/
void read_XYZ_from_demon_input_file(gchar *NomFichier, FilePosTypeGeom InfoFile )
{
 gchar *t;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 gint i;
 gint j;
 gint l;
 gboolean Uvar=FALSE;
 GeomXYZAtomDef* GeomXYZtemp=NULL;
 gint Ncent = 0;
 gint Nvar = 0;
 VariablesXYZDef* VariablesXYZtemp=NULL;
 gchar symb[BSIZE];
 gchar type[BSIZE];
 gchar charge[BSIZE];

 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 
 t=g_malloc(taille*sizeof(gchar));
 fd = FOpen(NomFichier, "rb");
 OK=TRUE;
 if(fd!=NULL)
 {
	for(i=0;i<InfoFile.numline-1;i++) { char* e = fgets(t,taille,fd);}
	GeomXYZtemp=g_malloc(sizeof(GeomXYZAtomDef));
  	j=-1;
  	while(!feof(fd) && OK )
  	{
    		j++;
    		if(!fgets(t,taille,fd))break;
		deleteFirstSpaces(t);
                if(t[0] == '#') continue;
                if(t[0] == '\n') break;
                for(i=0;i<(gint)strlen(t);i++) if(t[i] != ' ') break;
                if(i == (gint)strlen(t)-1) break;
    		i = sscanf(t,"%s ",AtomCoord[0]);
                //fprintf(stderr,"%s\n",t);
                if(i != 1)
		{
			OK = FALSE;
			break;
		}
		uppercase(AtomCoord[0]);
                if(strstr(AtomCoord[0],"VARIA") ) 
                {
			Uvar = TRUE;
			break;
                }
                i = sscanf(t,"%s %s %s %s %s",AtomCoord[0],AtomCoord[4],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
    		if( i!= 5 ) 
		{
                	i = sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		}
    		if( i== 5 || i== 4)
                {

                        Ncent = j+1;
			GeomXYZtemp=g_realloc(GeomXYZtemp,Ncent*sizeof(GeomXYZAtomDef));
    			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
			if(isdigit(AtomCoord[0][0]) || atoi(AtomCoord[0])<0)
			{
				gchar* sy = get_symbol_using_z(atoi(AtomCoord[0]));
				sprintf(AtomCoord[0],"%s",sy);
				g_free(sy);
			}
    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			GeomXYZtemp[j].Nentry=NUMBER_LIST_XYZ;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
			{
				gint k;
				for(k=0;k<(gint)strlen(symb);k++) if(isdigit(symb[k])) symb[k] = ' ';
				delete_all_spaces(symb);
			}

    			GeomXYZtemp[j].Symb=g_strdup(symb);
			GeomXYZtemp[j].mmType=g_strdup(type);
			GeomXYZtemp[j].pdbType=g_strdup(type);
			GeomXYZtemp[j].Charge=g_strdup(charge);

    			GeomXYZtemp[j].Residue=g_strdup("DUM");
    			GeomXYZtemp[j].ResidueNumber=0;
                        if(!test(AtomCoord[1]) || !test(AtomCoord[2]) || !test(AtomCoord[3]))
                              Uvar = TRUE;
    			GeomXYZtemp[j].X=g_strdup(AtomCoord[1]);
    			GeomXYZtemp[j].Y=g_strdup(AtomCoord[2]);
    			GeomXYZtemp[j].Z=g_strdup(AtomCoord[3]);

			if(strstr(t,"Low")) GeomXYZtemp[j].Layer=g_strdup("Low");
			else if(strstr(t,"Med")) GeomXYZtemp[j].Layer=g_strdup("Medium");
			else GeomXYZtemp[j].Layer=g_strdup(" ");
			GeomXYZtemp[j].typeConnections = NULL;
		}
               else
                {
                 OK = FALSE;
                }
  	}
/* Variables */
  Nvar=0;
  while(!feof(fd) && Uvar && OK )
  {
    	if(!fgets(t,taille,fd))break;
	deleteFirstSpaces(t);
       if(t[0] == '#') continue;
       if(t[0] == '\n') continue;
  	Nvar++;
  	if(Nvar==1) VariablesXYZtemp = g_malloc(Nvar*sizeof(VariablesXYZDef));
  	else VariablesXYZtemp = g_realloc(VariablesXYZtemp,Nvar*sizeof(VariablesXYZDef));
  	i = sscanf(t,"%s %s",AtomCoord[0],AtomCoord[1]);
	//fprintf(stderr,"ilue=%d\n",i);
        if( i == 2)
        {
		for(j=0;j<Ncent;j++) 
		{
			sprintf(t,"%s%d",GeomXYZtemp[j].Symb,j+1);
			//fprintf(stderr,"%s<>%s\n", t,AtomCoord[0]);
			if(!strcmp(t,AtomCoord[0]))
			{
				//fprintf(stderr,"Okkkkkkkkkkkkkkkkkkkkkkkkkk\n");
  				VariablesXYZtemp[Nvar-1].Name=g_strdup_printf("%s%s",AtomCoord[1],AtomCoord[0]);
				if(strstr(AtomCoord[1],"X") || strstr(AtomCoord[1],"x"))
				{
					VariablesXYZtemp[Nvar-1].Value= g_strdup_printf("%s",GeomXYZtemp[j].X); 
					GeomXYZtemp[j].X=g_strdup(VariablesXYZtemp[Nvar-1].Name); 
				}
				if(strstr(AtomCoord[1],"Y") || strstr(AtomCoord[1],"y"))
				{
					VariablesXYZtemp[Nvar-1].Value= g_strdup_printf("%s",GeomXYZtemp[j].Y); 
					GeomXYZtemp[j].Y=g_strdup(VariablesXYZtemp[Nvar-1].Name); 
				}
				if(strstr(AtomCoord[1],"Z") || strstr(AtomCoord[1],"z"))
				{
					VariablesXYZtemp[Nvar-1].Value= g_strdup_printf("%s",GeomXYZtemp[j].Z); 
					GeomXYZtemp[j].Z=g_strdup(VariablesXYZtemp[Nvar-1].Name); 
				}
  				VariablesXYZtemp[Nvar-1].Used=TRUE;
                		OK = TRUE;
				break;
			}
		}
         }
         else 
	 {
		Nvar--;
  		VariablesXYZtemp = g_realloc(VariablesXYZtemp,Nvar*sizeof(VariablesXYZDef));
                OK = FALSE;
		break;
	 }
  }
/* end while variables */
  fclose(fd);
 }
 else
      OK = FALSE;

 g_free(t);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if( !OK || Ncent <1 )
 {
   FreeGeomXYZ(GeomXYZtemp,VariablesXYZtemp,Ncent, Nvar);
   MessageGeom(_("Sorry\n I can not read geometry in DeMon input file"),_("Error"),TRUE);
   return;
 }
 if(GeomXYZ)
	freeGeomXYZ();
 if(VariablesXYZ)
	freeVariablesXYZ();
 GeomXYZ = GeomXYZtemp;
 NcentersXYZ = Ncent;
 NVariablesXYZ = Nvar;
 VariablesXYZ = VariablesXYZtemp;
 MethodeGeom = GEOM_IS_XYZ;
 if( InfoFile.units== 1 && Units== 0 )
 	GeomXYZ_Change_Unit(FALSE);
 else
 if( InfoFile.units== 0 && Units== 1 )
 	GeomXYZ_Change_Unit(TRUE);
 if(GeomIsOpen)
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

 if(GeomDrawingArea != NULL)
	rafresh_drawing();
 set_last_directory(NomFichier);
}
/*************************************************************************************/
void read_XYZ_from_gauss_input_file(gchar *NomFichier, FilePosTypeGeom InfoFile )
{
 gchar *t;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 gint i;
 gint j;
 gint l;
 gboolean Uvar=FALSE;
 GeomXYZAtomDef* GeomXYZtemp=NULL;
 gint Ncent = 0;
 gint Nvar = 0;
 VariablesXYZDef* VariablesXYZtemp=NULL;
 gchar symb[BSIZE];
 gchar type[BSIZE];
 gchar charge[BSIZE];

 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 
 t=g_malloc(taille*sizeof(gchar));
 fd = FOpen(NomFichier, "rb");
 OK=TRUE;
 if(fd!=NULL)
 {
	for(i=0;i<InfoFile.numline-1;i++) { char* e = fgets(t,taille,fd);}
	GeomXYZtemp=g_malloc(sizeof(GeomXYZAtomDef));
  	j=-1;
  	while(!feof(fd) && OK )
  	{
    		j++;
    		if(!fgets(t,taille,fd))break;
                if(t[0] == '\n') break;
                for(i=0;i<(gint)strlen(t);i++) if(t[i] != ' ') break;
                if(i == (gint)strlen(t)-1) break;
    		i = sscanf(t,"%s ",AtomCoord[0]);
                if(i != 1)
		{
			OK = FALSE;
			break;
		}
                if( !strcmp(AtomCoord[0],"Variables") ) 
                {
			Uvar = TRUE;
			break;
                }
                i = sscanf(t,"%s %s %s %s %s",AtomCoord[0],AtomCoord[4],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
    		if( i!= 5 ) 
		{
                	i = sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		}
    		if( i== 5 || i== 4)
                {

                        Ncent = j+1;
			GeomXYZtemp=g_realloc(GeomXYZtemp,Ncent*sizeof(GeomXYZAtomDef));
    			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
			if(isdigit(AtomCoord[0][0]) || atoi(AtomCoord[0])<0)
			{
				gchar* sy = get_symbol_using_z(atoi(AtomCoord[0]));
				sprintf(AtomCoord[0],"%s",sy);
				g_free(sy);
			}
    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			GeomXYZtemp[j].Nentry=NUMBER_LIST_XYZ;

			get_symb_type_charge(AtomCoord[0],symb,type,charge);
			{
				gint k;
				for(k=0;k<(gint)strlen(symb);k++) if(isdigit(symb[k])) symb[k] = ' ';
				delete_all_spaces(symb);
			}

    			GeomXYZtemp[j].Symb=g_strdup(symb);
			GeomXYZtemp[j].mmType=g_strdup(type);
			GeomXYZtemp[j].pdbType=g_strdup(type);
			GeomXYZtemp[j].Charge=g_strdup(charge);

    			GeomXYZtemp[j].Residue=g_strdup("DUM");
    			GeomXYZtemp[j].ResidueNumber=0;
                        if(!test(AtomCoord[1]) || !test(AtomCoord[2]) || !test(AtomCoord[3]))
                              Uvar = TRUE;
    			GeomXYZtemp[j].X=g_strdup(AtomCoord[1]);
    			GeomXYZtemp[j].Y=g_strdup(AtomCoord[2]);
    			GeomXYZtemp[j].Z=g_strdup(AtomCoord[3]);

			if(strstr(t,"Low")) GeomXYZtemp[j].Layer=g_strdup("Low");
			else if(strstr(t,"Med")) GeomXYZtemp[j].Layer=g_strdup("Medium");
			else GeomXYZtemp[j].Layer=g_strdup(" ");
			GeomXYZtemp[j].typeConnections = NULL;
		}
               else
                {
                 OK = FALSE;
                }
  	}
/* Variables */
  Nvar=0;
  while(!feof(fd) && Uvar && OK )
  {
    	{ char* e = fgets(t,taille,fd);}
        OK=TRUE;
        for(i=0;i<(gint)strlen(t)-1;i++)
 		if ( (int)t[i] != (int)' ' )
                OK=FALSE;
        if(OK)
        {
		OK = TRUE; 
		break;
        }
 	else
	{
  	Nvar++;
  	if(Nvar==1)
  		VariablesXYZtemp = g_malloc(Nvar*sizeof(VariablesXYZDef));
  	else
  		VariablesXYZtemp = g_realloc(VariablesXYZtemp,Nvar*sizeof(VariablesXYZDef));
  	i = sscanf(t,"%s %s",AtomCoord[0],AtomCoord[1]);
        if( i == 2)
        {
  		VariablesXYZtemp[Nvar-1].Name=g_strdup(AtomCoord[0]);
  		VariablesXYZtemp[Nvar-1].Value=g_strdup(AtomCoord[1]);
  		VariablesXYZtemp[Nvar-1].Used=TRUE;
                OK = TRUE;
         }
         else 
	 {
		Nvar--;
  		VariablesXYZtemp = g_realloc(VariablesXYZtemp,Nvar*sizeof(VariablesXYZDef));
                OK = FALSE;
		break;
	 }
 	 }
  }
/* end while variables */
  fclose(fd);
 }
 else
      OK = FALSE;

 g_free(t);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if( !OK || Ncent <1 )
 {
   FreeGeomXYZ(GeomXYZtemp,VariablesXYZtemp,Ncent, Nvar);
   MessageGeom(_("Sorry\n I can not read geometry in Gaussian input file"),_("Error"),TRUE);
   return;
 }
 if(GeomXYZ)
	freeGeomXYZ();
 if(VariablesXYZ)
	freeVariablesXYZ();
 GeomXYZ = GeomXYZtemp;
 NcentersXYZ = Ncent;
 NVariablesXYZ = Nvar;
 VariablesXYZ = VariablesXYZtemp;
 MethodeGeom = GEOM_IS_XYZ;
 if( InfoFile.units== 1 && Units== 0 )
 	GeomXYZ_Change_Unit(FALSE);
 else
 if( InfoFile.units== 0 && Units== 1 )
 	GeomXYZ_Change_Unit(TRUE);
 if(GeomIsOpen)
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

 if(GeomDrawingArea != NULL)
	rafresh_drawing();
 set_last_directory(NomFichier);
}
/*************************************************************************************/
void read_XYZ_from_nwchem_input_file(gchar *NomFichier)
{
	gchar *t;
	gboolean OK;
	gboolean icharge;
	gchar *AtomCoord[5];
	FILE *file;
	guint taille=BSIZE;
	gint i;
	gint j;
	gint l;
	gint k;
	gboolean Uvar=FALSE;
	GeomXYZAtomDef* GeomXYZtemp=NULL;
	gint Ncent = 0;
	gint Nvar = 0;
	VariablesXYZDef* VariablesXYZtemp=NULL;
	gchar symb[BSIZE];
	gchar type[BSIZE];
	gchar charge[BSIZE];
	gint globalCharge, mult;
 
 

	file = FOpen(NomFichier, "rb");
	OK=TRUE;
 	if(file==NULL)
	{
   		MessageGeom(_("Sorry\n I can not read geometry in NWChem input file"),_("Error"),TRUE);
   		return;
	}
	t=g_malloc(taille*sizeof(gchar));
	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));

	 while(!feof(file))
	 {
		if(!fgets(t,taille,file))
		{
			OK = FALSE;
			break;
		}
		uppercase(t);
		if(strstr(t,"GEOMETRY"))
		{
			OK = TRUE;
			break;
		}
	 }
	 if(OK) GeomXYZtemp=g_malloc(sizeof(GeomXYZAtomDef));
	j=-1;
 	while(!feof(file) && OK )
  	{
    		j++;
    		if(!fgets(t,taille,file))break;
		uppercase(t);
		if(strstr(t,"ZMATRIX"))
		{
			g_free(GeomXYZtemp);
   			MessageGeom(_("Sorry\n I can not zmatrix geometry from a NWChem input file"),_("Error"),TRUE);
			return;
		}

		if(strstr(t,"END")) break;
		if(strstr(t,"SYMMETRY")) { j--; continue;}
                for(i=0;i<(gint)strlen(t);i++) if(t[i] != ' ') break;
                if(i<=(gint)strlen(t) && t[i] == '*') break;
		for(k=0;k<(gint)strlen(t);k++) if(t[k]=='{' || t[k]=='}') t[k] = ' ';
                i = sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
    		if(i== 4)
                {

                        Ncent = j+1;
			GeomXYZtemp=g_realloc(GeomXYZtemp,Ncent*sizeof(GeomXYZAtomDef));
    			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			GeomXYZtemp[j].Nentry=NUMBER_LIST_XYZ;
			get_symb_type_charge(AtomCoord[0],symb,type,charge);
			{
				gint k;
				for(k=0;k<(gint)strlen(symb);k++) if(isdigit(symb[k])) symb[k] = ' ';
				delete_all_spaces(symb);
			}

    			GeomXYZtemp[j].Symb=g_strdup(symb);
			GeomXYZtemp[j].mmType=g_strdup(type);
			GeomXYZtemp[j].pdbType=g_strdup(type);
			GeomXYZtemp[j].Charge=g_strdup(charge);

    			GeomXYZtemp[j].Residue=g_strdup("DUM");
    			GeomXYZtemp[j].ResidueNumber=0;
                        if(!test(AtomCoord[1]) || !test(AtomCoord[2]) || !test(AtomCoord[3])) Uvar = TRUE;
    			GeomXYZtemp[j].X=g_strdup(AtomCoord[1]);
    			GeomXYZtemp[j].Y=g_strdup(AtomCoord[2]);
    			GeomXYZtemp[j].Z=g_strdup(AtomCoord[3]);

    			GeomXYZtemp[j].Layer=g_strdup(" ");
    			GeomXYZtemp[j].typeConnections = NULL;
		}
               else
                 OK = FALSE;
  	}
/* Variables */
	Nvar=0;
	fseek(file, 0L, SEEK_SET);
	if(Uvar)
	{
		OK =FALSE;
  		while(!feof(file) && Uvar)
  		{
 			if(!fgets(t,taille,file)) break;
			uppercase(t);
			if(strstr(t,"VARIABLE")) 
			{
				OK = TRUE;
				break;
			}
  		}
  	}
	while(!feof(file) && Uvar && OK )
	{
		uppercase(t);
		for(k=0;k<(gint)strlen(t);k++) if(t[k]=='=') t[k] = ' ';
        	for(j=0;j<Ncent;j++)
		{
			gchar* co[3] = {GeomXYZtemp[j].X,GeomXYZtemp[j].Y,GeomXYZtemp[j].Z};
        		for(k=0;k<3;k++)
			if(!test(co[k]))
			{
				gchar* b = strstr(t,co[k]);
				if(b) 
				{
					b = b+strlen(co[k])+1;
  					Nvar++;
  					if(Nvar==1) VariablesXYZtemp = g_malloc(Nvar*sizeof(VariablesXYZDef));
  					else VariablesXYZtemp = g_realloc(VariablesXYZtemp,Nvar*sizeof(VariablesXYZDef));
  					VariablesXYZtemp[Nvar-1].Name=g_strdup(co[k]);
  					VariablesXYZtemp[Nvar-1].Value=g_strdup_printf("%f",atof(b));
  					VariablesXYZtemp[Nvar-1].Used=TRUE;
				}
			}
		}
		if(strstr(t,"END")) break;
    		{ char* e = fgets(t,taille,file);}
  	}
/* end while variables */
	fseek(file, 0L, SEEK_SET);
	icharge = FALSE;
	globalCharge = 0;
	mult = 0;
	while(OK && !feof(file))
	{
		if(!fgets(t,taille,file)) break;
		uppercase(t);
		if(strstr(t,"CHARGE") && 2==sscanf(t,"%s %d",AtomCoord[0],&globalCharge) )
		{
			icharge = TRUE;
			continue;
		}
		if(strstr(t,"SINGLET")) mult=1;
		if(strstr(t,"DOUBLET")) mult=2;
		if(strstr(t,"TRIPLET")) mult=3;
		if(strstr(t,"QUARTET")) mult=4;
		if(strstr(t,"QUINTET")) mult=5;
		if(strstr(t,"SEXTET")) mult=6;
		if(strstr(t,"SEPTET")) mult=7;
		if(strstr(t,"OCTET")) mult=8;
		if(strlen(t)>5 && strstr(t,"MULT") && sscanf(strstr(t,"MULT")+4,"%d",&k) && k>0 ) mult=k;
		if(strlen(t)>6 && strstr(t,"NOPEN") && sscanf(strstr(t,"NOOPEN")+5,"%d",&k) && k>0 ) mult=2*k;
		if(mult>0 && icharge) break;
	}
	fclose(file);
	g_free(t);
	if(OK)
 	{
		TotalCharges[0] = globalCharge;
		if(mult>0) SpinMultiplicities[0] = mult;
		else reset_spin_of_electrons();
 	}

	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if( !OK || Ncent <1 )
	{
   		FreeGeomXYZ(GeomXYZtemp,VariablesXYZtemp,Ncent, Nvar);
   		MessageGeom(_("Sorry\n I can not read geometry in NWChem input file"),_("Error"),TRUE);
   		return;
 	}
	if(GeomXYZ) freeGeomXYZ();
	if(VariablesXYZ) freeVariablesXYZ();
	GeomXYZ = GeomXYZtemp;
 	NcentersXYZ = Ncent;
 	NVariablesXYZ = Nvar;
 	VariablesXYZ = VariablesXYZtemp;
 	MethodeGeom = GEOM_IS_XYZ;
 	calculMMTypes(FALSE);
 	if( Units== 0 ) GeomXYZ_Change_Unit(FALSE);
 	if(GeomIsOpen)
		create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

 	if(GeomDrawingArea != NULL)
		rafresh_drawing();
 	set_last_directory(NomFichier);
}
/*************************************************************************************/
void read_XYZ_from_psicode_input_file(gchar *NomFichier)
{
	gchar *t;
	gboolean OK;
	gboolean icharge;
	gchar *AtomCoord[5];
	FILE *file;
	guint taille=BSIZE;
	gint i;
	gint j;
	gint l;
	gint k;
	gboolean Uvar=FALSE;
	GeomXYZAtomDef* GeomXYZtemp=NULL;
	gint Ncent = 0;
	gint Nvar = 0;
	VariablesXYZDef* VariablesXYZtemp=NULL;
	gchar symb[BSIZE];
	gchar type[BSIZE];
	gchar charge[BSIZE];
	gint globalCharge, mult;
 
 

	/* printf("Coucou\n");*/
	file = FOpen(NomFichier, "rb");
	OK=TRUE;
 	if(file==NULL)
	{
		gchar* buffer = g_strdup_printf(_("Sorry\n I can not read geometry in Psicode input file %s"), NomFichier);
   		MessageGeom(buffer,_("Error"),TRUE);
		g_free(buffer);
   		return;
	}
	t=g_malloc(taille*sizeof(gchar));
	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));

	 while(!feof(file))
	 {
		if(!fgets(t,taille,file))
		{
			OK = FALSE;
			break;
		}
		uppercase(t);
		/*printf("t=%s\n", t);*/
		if(strstr(t,"MOLECULE"))
		{
			OK = TRUE;
			break;
		}
	 }
	if(fgets(t,taille,file) && 2==sscanf(t,"%d %d",&globalCharge,&mult)) OK = TRUE;
	else OK = FALSE;

	/*printf("charge = %d mult = %d\n", globalCharge, mult);*/
	 if(OK) GeomXYZtemp=g_malloc(sizeof(GeomXYZAtomDef));
	j=-1;
 	while(!feof(file) && OK )
  	{
    		j++;
    		if(!fgets(t,taille,file))break;
		uppercase(t);
                if(strstr(t,"SYMMETRY")) continue;
		if(strstr(t,"ZMATRIX"))
		{
			g_free(GeomXYZtemp);
   			MessageGeom(_("Sorry\n I can not zmatrix geometry from a Psicode input file"),_("Error"),TRUE);
			return;
		}

		if(strstr(t,"}")) break;
		if(this_is_a_backspace(t)) break;
                for(i=0;i<(gint)strlen(t);i++) if(t[i] != ' ' && t[i]!= '\t') break;
                if(i<=(gint)strlen(t) && t[i] == '#') continue;
                i = sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
	/* printf("t=%s\n", t);*/
    		if(i== 4)
                {

                        Ncent = j+1;
			GeomXYZtemp=g_realloc(GeomXYZtemp,Ncent*sizeof(GeomXYZAtomDef));
    			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			GeomXYZtemp[j].Nentry=NUMBER_LIST_XYZ;
			get_symb_type_charge(AtomCoord[0],symb,type,charge);
			{
				gint k;
				for(k=0;k<(gint)strlen(symb);k++) if(isdigit(symb[k])) symb[k] = ' ';
				delete_all_spaces(symb);
			}

    			GeomXYZtemp[j].Symb=g_strdup(symb);
			GeomXYZtemp[j].mmType=g_strdup(type);
			GeomXYZtemp[j].pdbType=g_strdup(type);
			GeomXYZtemp[j].Charge=g_strdup(charge);

    			GeomXYZtemp[j].Residue=g_strdup("DUM");
    			GeomXYZtemp[j].ResidueNumber=0;
                        if(!test(AtomCoord[1]) || !test(AtomCoord[2]) || !test(AtomCoord[3])) Uvar = TRUE;
    			GeomXYZtemp[j].X=g_strdup(AtomCoord[1]);
    			GeomXYZtemp[j].Y=g_strdup(AtomCoord[2]);
    			GeomXYZtemp[j].Z=g_strdup(AtomCoord[3]);

    			GeomXYZtemp[j].Layer=g_strdup(" ");
    			GeomXYZtemp[j].typeConnections = NULL;
		}
               else
                 OK = FALSE;
  	}
/* Variables */
	Nvar=0;
	while(!feof(file) && Uvar && OK )
	{
		uppercase(t);
		for(k=0;k<(gint)strlen(t);k++) if(t[k]=='=') t[k] = ' ';
        	for(j=0;j<Ncent;j++)
		{
			gchar* co[3] = {GeomXYZtemp[j].X,GeomXYZtemp[j].Y,GeomXYZtemp[j].Z};
        		for(k=0;k<3;k++)
			if(!test(co[k]))
			{
				gchar* b = strstr(t,co[k]);
				if(b) 
				{
					b = b+strlen(co[k])+1;
  					Nvar++;
  					if(Nvar==1) VariablesXYZtemp = g_malloc(Nvar*sizeof(VariablesXYZDef));
  					else VariablesXYZtemp = g_realloc(VariablesXYZtemp,Nvar*sizeof(VariablesXYZDef));
  					VariablesXYZtemp[Nvar-1].Name=g_strdup(co[k]);
  					VariablesXYZtemp[Nvar-1].Value=g_strdup_printf("%f",atof(b));
  					VariablesXYZtemp[Nvar-1].Used=TRUE;
				}
			}
		}
		if(strstr(t,"}")) break;
    		{ char* e = fgets(t,taille,file);}
  	}
/* end while variables */
	fclose(file);
	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if( !OK || Ncent <1 )
	{
   		FreeGeomXYZ(GeomXYZtemp,VariablesXYZtemp,Ncent, Nvar);
   		MessageGeom(_("Sorry\n I can not read geometry in Psicode input file"),_("Error"),TRUE);
   		return;
 	}
	TotalCharges[0] = globalCharge;
	SpinMultiplicities[0]=mult;
	if(GeomXYZ) freeGeomXYZ();
	if(VariablesXYZ) freeVariablesXYZ();
	GeomXYZ = GeomXYZtemp;
 	NcentersXYZ = Ncent;
 	NVariablesXYZ = Nvar;
 	VariablesXYZ = VariablesXYZtemp;
 	MethodeGeom = GEOM_IS_XYZ;
 	calculMMTypes(FALSE);
 	if( Units== 0 ) GeomXYZ_Change_Unit(FALSE);
 	if(GeomIsOpen)
		create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

 	if(GeomDrawingArea != NULL)
		rafresh_drawing();
 	set_last_directory(NomFichier);
}
/*************************************************************************************/
void read_XYZ_from_orca_input_file(gchar *NomFichier)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	FILE *file;
	guint taille=BSIZE;
	gint i;
	gint j;
	gint l;
	gint k;
	gboolean Uvar=FALSE;
	GeomXYZAtomDef* GeomXYZtemp=NULL;
	gint Ncent = 0;
	gint Nvar = 0;
	VariablesXYZDef* VariablesXYZtemp=NULL;
	gchar symb[BSIZE];
	gchar type[BSIZE];
	gchar charge[BSIZE];
	gint globalCharge, mult;
 
 

	file = FOpen(NomFichier, "rb");
	OK=TRUE;
 	if(file==NULL)
	{
   		MessageGeom(_("Sorry\n I can not read geometry in Orca input file"),_("Error"),TRUE);
   		return;
	}
	t=g_malloc(taille*sizeof(gchar));
	for(i=0;i<5;i++)
		AtomCoord[i]=g_malloc(taille*sizeof(char));

	 while(!feof(file))
	 {
		if(!fgets(t,taille,file))
		{
			OK = FALSE;
			break;
		}
		uppercase(t);
		if(strstr(t,"* XYZ") && 4==sscanf(t,"%s %s %d %d",AtomCoord[0],AtomCoord[1],&globalCharge, &mult) )
		{
			OK = TRUE;
			break;
		}
	 }
	 if(OK) GeomXYZtemp=g_malloc(sizeof(GeomXYZAtomDef));
	j=-1;
 	while(!feof(file) && OK )
  	{
    		j++;
    		{ char* e = fgets(t,taille,file);}
                for(i=0;i<(gint)strlen(t);i++) if(t[i] != ' ') break;
                if(i<=(gint)strlen(t) && t[i] == '*') break;
		for(k=0;k<(gint)strlen(t);k++) if(t[k]=='{' || t[k]=='}') t[k] = ' ';
                i = sscanf(t,"%s %s %s %s %s",AtomCoord[0],AtomCoord[4],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
    		if( i!= 5 ) 
		{
                	i = sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		}
    		if( i== 5 || i== 4)
                {

                        Ncent = j+1;
			GeomXYZtemp=g_realloc(GeomXYZtemp,Ncent*sizeof(GeomXYZAtomDef));
    			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			if(!strcmp(AtomCoord[0],"Da")) sprintf(AtomCoord[0],"X");

    			GeomXYZtemp[j].Nentry=NUMBER_LIST_XYZ;
			get_symb_type_charge(AtomCoord[0],symb,type,charge);
			{
				gint k;
				for(k=0;k<(gint)strlen(symb);k++) if(isdigit(symb[k])) symb[k] = ' ';
				delete_all_spaces(symb);
			}

    			GeomXYZtemp[j].Symb=g_strdup(symb);
			GeomXYZtemp[j].mmType=g_strdup(type);
			GeomXYZtemp[j].pdbType=g_strdup(type);
			GeomXYZtemp[j].Charge=g_strdup(charge);

    			GeomXYZtemp[j].Residue=g_strdup("DUM");
    			GeomXYZtemp[j].ResidueNumber=0;
                        if(!test(AtomCoord[1]) || !test(AtomCoord[2]) || !test(AtomCoord[3]))
                              Uvar = TRUE;
    			GeomXYZtemp[j].X=g_strdup(AtomCoord[1]);
    			GeomXYZtemp[j].Y=g_strdup(AtomCoord[2]);
    			GeomXYZtemp[j].Z=g_strdup(AtomCoord[3]);

    			GeomXYZtemp[j].Layer=g_strdup(" ");
    			GeomXYZtemp[j].typeConnections = NULL;
		}
               else
                 OK = FALSE;
  	}
/* Variables */
	Nvar=0;
	fseek(file, 0L, SEEK_SET);
	if(Uvar)
	{
		OK =FALSE;
  		while(!feof(file) && Uvar)
  		{
 			if(!fgets(t,taille,file)) break;
			uppercase(t);
			if(strstr(t,"PARAS")) 
			{
				OK = TRUE;
				break;
			}
  		}
  	}
	while(!feof(file) && Uvar && OK )
	{
		uppercase(t);
		for(k=0;k<(gint)strlen(t);k++) if(t[k]=='=') t[k] = ' ';
        	for(j=0;j<Ncent;j++)
		{
			gchar* co[3] = {GeomXYZtemp[j].X,GeomXYZtemp[j].Y,GeomXYZtemp[j].Z};
        		for(k=0;k<3;k++)
			if(!test(co[k]))
			{
				gchar* b = strstr(t,co[k]);
				if(b) 
				{
					b = b+strlen(co[k])+1;
  					Nvar++;
  					if(Nvar==1) VariablesXYZtemp = g_malloc(Nvar*sizeof(VariablesXYZDef));
  					else VariablesXYZtemp = g_realloc(VariablesXYZtemp,Nvar*sizeof(VariablesXYZDef));
  					VariablesXYZtemp[Nvar-1].Name=g_strdup(co[k]);
  					VariablesXYZtemp[Nvar-1].Value=g_strdup_printf("%f",atof(b));
  					VariablesXYZtemp[Nvar-1].Used=TRUE;
				}
			}
		}
		if(strstr(t,"END")) break;
    		{ char* e = fgets(t,taille,file);}
  	}
/* end while variables */
	fclose(file);
	if(OK)
 	{
		TotalCharges[0] = globalCharge;
		SpinMultiplicities[0] = mult;
 	}

	g_free(t);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if( !OK || Ncent <1 )
	{
   		FreeGeomXYZ(GeomXYZtemp,VariablesXYZtemp,Ncent, Nvar);
   		MessageGeom(_("Sorry\n I can not read geometry in Orca input file"),_("Error"),TRUE);
   		return;
 	}
	if(GeomXYZ) freeGeomXYZ();
	if(VariablesXYZ) freeVariablesXYZ();
	GeomXYZ = GeomXYZtemp;
 	NcentersXYZ = Ncent;
 	NVariablesXYZ = Nvar;
 	VariablesXYZ = VariablesXYZtemp;
 	MethodeGeom = GEOM_IS_XYZ;
 	calculMMTypes(FALSE);
 	if( Units== 0 ) GeomXYZ_Change_Unit(FALSE);
 	if(GeomIsOpen)
		create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

 	if(GeomDrawingArea != NULL)
		rafresh_drawing();
 	set_last_directory(NomFichier);
}
/*************************************************************************************/
void read_XYZ_from_qchem_input_file(gchar *NomFichier)
{
 gchar *t;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 gint i;
 gint j;
 gint l;
 gboolean Uvar=FALSE;
 GeomXYZAtomDef* GeomXYZtemp=NULL;
 gint Ncent = 0;
 gint Nvar = 0;
 VariablesXYZDef* VariablesXYZtemp=NULL;
 gchar symb[BSIZE];
 gchar type[BSIZE];
 gchar charge[BSIZE];
 
 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(char));
 

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");

 OK=TRUE;
 if(fd!=NULL)
 {
	 while(!feof(fd))
	 {
		if(!fgets(t,taille,fd))
		{
			OK = FALSE;
			break;
		}
		uppercase(t);
		if(strstr(t,"$MOLECULE"))
		{
    			{ char* e = fgets(t,taille,fd);} /* charge and spin */
			OK = TRUE;
			break;
		}
	 }
	 if(OK) GeomXYZtemp=g_malloc(sizeof(GeomXYZAtomDef));
  	j=-1;
  	while(!feof(fd) && OK )
  	{
    		j++;
    		{ char* e = fgets(t,taille,fd);}
		if(strstr(t,"$")) break;
                if(t[0] == '\n') break;
                for(i=0;i<(gint)strlen(t);i++)
                   if(t[i] != ' ') break;

                if(i == (gint)strlen(t)-1) break;

    		i = sscanf(t,"%s ",AtomCoord[0]);
                if(i != 1)
		{
			OK = FALSE;
			break;
		}
                if( !strcmp(AtomCoord[0],"Variables") ) 
                {
			Uvar = TRUE;
			break;
                }
                i = sscanf(t,"%s %s %s %s %s",AtomCoord[0],AtomCoord[4],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
    		if( i!= 5 ) 
		{
                	i = sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
		}
    		if( i== 5 || i== 4)
                {

                        Ncent = j+1;
			GeomXYZtemp=g_realloc(GeomXYZtemp,Ncent*sizeof(GeomXYZAtomDef));
    			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
			if(isdigit(AtomCoord[0][0]))
			{
				gchar* sy = get_symbol_using_z(atoi(AtomCoord[0]));
				sprintf(AtomCoord[0],"%s",sy);
				g_free(sy);
			}
    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			GeomXYZtemp[j].Nentry=NUMBER_LIST_XYZ;
			get_symb_type_charge(AtomCoord[0],symb,type,charge);
			{
				gint k;
				for(k=0;k<(gint)strlen(symb);k++) if(isdigit(symb[k])) symb[k] = ' ';
				delete_all_spaces(symb);
			}

    			GeomXYZtemp[j].Symb=g_strdup(symb);
			GeomXYZtemp[j].mmType=g_strdup(type);
			GeomXYZtemp[j].pdbType=g_strdup(type);
			GeomXYZtemp[j].Charge=g_strdup(charge);

    			GeomXYZtemp[j].Residue=g_strdup("DUM");
    			GeomXYZtemp[j].ResidueNumber=0;
                        if(!test(AtomCoord[1]) || !test(AtomCoord[2]) || !test(AtomCoord[3]))
                              Uvar = TRUE;
    			GeomXYZtemp[j].X=g_strdup(AtomCoord[1]);
    			GeomXYZtemp[j].Y=g_strdup(AtomCoord[2]);
    			GeomXYZtemp[j].Z=g_strdup(AtomCoord[3]);

    			GeomXYZtemp[j].Layer=g_strdup(" ");
    			GeomXYZtemp[j].typeConnections = NULL;
		}
               else
                {
                 OK = FALSE;
                }
  	}
/* Variables */
  Nvar=0;
  while(!feof(fd) && Uvar && OK )
  {
    	{ char* e = fgets(t,taille,fd);}
        OK=TRUE;
        for(i=0;i<(gint)strlen(t)-1;i++)
 		if ( (int)t[i] != (int)' ' )
                OK=FALSE;
        if(OK || strstr(t,"$"))
        {
		OK = TRUE; /* sortie normale*/
		break;
        }
 	else
	{
  	Nvar++;
  	if(Nvar==1)
  		VariablesXYZtemp = g_malloc(Nvar*sizeof(VariablesXYZDef));
  	else
  		VariablesXYZtemp = g_realloc(VariablesXYZtemp,Nvar*sizeof(VariablesXYZDef));
  	i = sscanf(t,"%s %s",AtomCoord[0],AtomCoord[1]);
        if( i == 2)
        {
  		VariablesXYZtemp[Nvar-1].Name=g_strdup(AtomCoord[0]);
  		VariablesXYZtemp[Nvar-1].Value=g_strdup(AtomCoord[1]);
  		VariablesXYZtemp[Nvar-1].Used=TRUE;
                OK = TRUE;
         }
         else 
	 {
		Nvar--;
  		VariablesXYZtemp = g_realloc(VariablesXYZtemp,Nvar*sizeof(VariablesXYZDef));
                OK = FALSE;
		break;
	 }
 	 }
  }
/* end while variables */
  fclose(fd);
 }
 else
      OK = FALSE;

 g_free(t);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if( !OK || Ncent <1 )
 {
   FreeGeomXYZ(GeomXYZtemp,VariablesXYZtemp,Ncent, Nvar);
   MessageGeom(_("Sorry\n I can not read geometry in QChem input file"),_("Error"),TRUE);
   return;
 }
 if(GeomXYZ)
	freeGeomXYZ();
 if(VariablesXYZ)
	freeVariablesXYZ();
 GeomXYZ = GeomXYZtemp;
 NcentersXYZ = Ncent;
 NVariablesXYZ = Nvar;
 VariablesXYZ = VariablesXYZtemp;
 MethodeGeom = GEOM_IS_XYZ;
 calculMMTypes(FALSE);
 if( Units== 0 ) GeomXYZ_Change_Unit(FALSE);
 if(GeomIsOpen)
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

 if(GeomDrawingArea != NULL)
	rafresh_drawing();
 set_last_directory(NomFichier);
}
/********************************************************************************/
static void trans_coordXYZ_geom(gchar T, GeomXYZAtomDef* myGeomXYZ, gint i, VariablesXYZDef* myVariablesXYZ, gint iv)
{
 gdouble V;
 gchar *NameV;
 gchar *ValueV;

 V = atof(myGeomXYZ[i].X);
 if( T == 'Y' ) V = atof(myGeomXYZ[i].Y);
 if( T == 'Z' ) V = atof(myGeomXYZ[i].Z);
 NameV = g_strdup_printf("%c%s%d",T,myGeomXYZ[i].Symb,i+1);
 ValueV = g_strdup_printf("%f",V);
 if( T == 'X' ) { g_free(myGeomXYZ[i].X); myGeomXYZ[i].X=g_strdup(NameV);}
 if( T == 'Y' ) { g_free(myGeomXYZ[i].Y); myGeomXYZ[i].Y=g_strdup(NameV);}
 if( T == 'Z' ) { g_free(myGeomXYZ[i].Z); myGeomXYZ[i].Z=g_strdup(NameV);}
 myVariablesXYZ[iv].Name  = g_strdup(NameV);
 myVariablesXYZ[iv].Value = g_strdup(ValueV);
}
/********************************************************************************/
static void get_charge_and_multiplicity_from_mopac_input_file(FILE* fd)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
	gint i;

	TotalCharges[0] = 0;
	SpinMultiplicities[0] =1;

	fseek(fd, 0L, SEEK_SET);
  	while(!feof(fd) )    
  	{
 		if(!fgets(t, taille, fd)) return;
		if(t[0] !='*') break;
	}
  	for(i=0;i<2;i++)
	{
		uppercase(t);
		if ( strstr(t,"CHARGE") && strstr(t,"="))
		{
			gchar* p = strstr(t,"=")+1;
			TotalCharges[0] = atoi(p);
		}
		if ( strstr(t,"SINGLET")) SpinMultiplicities[0] = 1;
		if ( strstr(t,"DOUBLET")) SpinMultiplicities[0] = 2;
		if ( strstr(t,"TRIPLET")) SpinMultiplicities[0] = 3;
		if ( strstr(t,"QUARTET")) SpinMultiplicities[0] = 4;
		if ( strstr(t,"QUINTET")) SpinMultiplicities[0] = 5;
		if ( strstr(t,"SEXTET")) SpinMultiplicities[0] = 6;
    		if(!fgets(t,taille,fd)) break;
	}
}
/*************************************************************************************/
void read_XYZ_from_mopac_input_file(gchar *NomFichier)
{
 gchar *t;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 gint i;
 gint j;
 gint l;
 /*gboolean Uvar=FALSE;*/
 GeomXYZAtomDef* GeomXYZtemp=NULL;
 gint Ncent = 0;
 gint Nvar = 0;
 VariablesXYZDef* VariablesXYZtemp=NULL;
 gchar symb[BSIZE];
 gchar type[BSIZE];
 gchar charge[BSIZE];
 gint k1,k2,k3;
 
 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(gchar));
 

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");

 OK=TRUE;
 j = -1;
 if(fd!=NULL)
 {
  	while(!feof(fd) )    
  	{
 		if(!fgets(t, taille, fd)) {OK = FALSE; break;}
		if(t[0] !='*') break;
	}
 	if(!fgets(t, taille, fd)) OK = FALSE; 
 	if(!fgets(t, taille, fd)) OK = FALSE; 

	 if(OK) GeomXYZtemp=g_malloc(sizeof(GeomXYZAtomDef));
  	j=-1;
  	while(!feof(fd) && OK )
  	{
    		j++;
		if(!fgets(t,taille,fd)) break;
		if(this_is_a_backspace(t)) break;
                if(t[0] == '0') break;
                for(i=0;i<(gint)strlen(t);i++)
                   if(t[i] != ' ') break;
                if(i == (gint)strlen(t)-1) break;
    		i = sscanf(t,"%s ",AtomCoord[0]);
                if(i != 1)
		{
			OK = FALSE;
			break;
		}
                i = sscanf(t,"%s %s %d %s %d %s %d",
				AtomCoord[0],
				AtomCoord[1],&k1,AtomCoord[2],&k2,AtomCoord[3],&k3);
    		if( i== 7)
                {
			gint newVar = 0;

                        Ncent = j+1;
			GeomXYZtemp=g_realloc(GeomXYZtemp,Ncent*sizeof(GeomXYZAtomDef));
    			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
    			l=strlen(AtomCoord[0]);
      			if (l>=2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);

    			GeomXYZtemp[j].Nentry=NUMBER_LIST_XYZ;
			get_symb_type_charge(AtomCoord[0],symb,type,charge);
			{
				gint k;
				for(k=0;k<(gint)strlen(symb);k++) if(isdigit(symb[k])) symb[k] = ' ';
				delete_all_spaces(symb);
			}

    			GeomXYZtemp[j].Symb=g_strdup(symb);
			GeomXYZtemp[j].mmType=g_strdup(type);
			GeomXYZtemp[j].pdbType=g_strdup(type);
			GeomXYZtemp[j].Charge=g_strdup(charge);

    			GeomXYZtemp[j].Residue=g_strdup("DUM");
    			GeomXYZtemp[j].ResidueNumber=0;
    			GeomXYZtemp[j].X=g_strdup(AtomCoord[1]);
    			GeomXYZtemp[j].Y=g_strdup(AtomCoord[2]);
    			GeomXYZtemp[j].Z=g_strdup(AtomCoord[3]);
    			GeomXYZtemp[j].Layer=g_strdup(" ");
    			GeomXYZtemp[j].typeConnections = NULL;
                        /* if(k1 ==1 || k2 == 1 || k3==1) Uvar = TRUE;*/
			if(k1==1) newVar++;
			if(k2==1) newVar++;
			if(k3==1) newVar++;
			if(newVar>0)
			{
				gint n = Nvar+newVar;
  				if(n==1)
  					VariablesXYZtemp = g_malloc(n*sizeof(VariablesXYZDef));
  				else
  					VariablesXYZtemp = g_realloc(VariablesXYZtemp,n*sizeof(VariablesXYZDef));
			}
			newVar = 0;
			if(k1==1)
			{
				newVar++;
				trans_coordXYZ_geom('X', GeomXYZtemp, j, VariablesXYZtemp,  Nvar+newVar-1);
			}
			if(k2==1)
			{
				newVar++;
				trans_coordXYZ_geom('Y', GeomXYZtemp, j, VariablesXYZtemp,  Nvar+newVar-1);
			}
			if(k3==1)
			{
				newVar++;
				trans_coordXYZ_geom('Z', GeomXYZtemp, j, VariablesXYZtemp,  Nvar+newVar-1);
			}
			Nvar += newVar;

		}
               else
               {
                 OK = FALSE;
               }
  	}
 }
 if(j<0) OK = FALSE;
 else get_charge_and_multiplicity_from_mopac_input_file(fd);
 fclose(fd);

 g_free(t);
 for(i=0;i<5;i++) g_free(AtomCoord[i]);
 if( !OK || Ncent <1 )
 {
   FreeGeomXYZ(GeomXYZtemp,VariablesXYZtemp,Ncent, Nvar);
   MessageGeom(_("Sorry\n I can not read geometry in Mopac input file"),_("Error"),TRUE);
   return;
 }
 if(GeomXYZ) freeGeomXYZ();
 if(VariablesXYZ) freeVariablesXYZ();
 GeomXYZ = GeomXYZtemp;
 NcentersXYZ = Ncent;
 NVariablesXYZ = Nvar;
 VariablesXYZ = VariablesXYZtemp;
 MethodeGeom = GEOM_IS_XYZ;
 calculMMTypes(FALSE);
 if( Units== 0 ) GeomXYZ_Change_Unit(FALSE);
 if(GeomIsOpen) create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);

 if(GeomDrawingArea != NULL) rafresh_drawing();
 set_last_directory(NomFichier);
}
/*************************************************************************************/
static gint get_pdbtype_charge_from_tinker_amber_key_file(
		gchar* NomFichier,
		gchar***types, 
		gchar***residues, 
		gdouble**charges)
{
	FILE *file;
	gint n = 0;
	gchar* tstr = NULL;
	gchar sdum0[100];
	gchar sdum1[100];
	gchar sdum2[100];
	gchar sdum3[500];
	gchar res[100];
	gint idum;
	gint k;
	gchar** pdbTypes = NULL;
	gchar** residueNames = NULL;
	gdouble* chargesValue = NULL;
	gdouble ddum;
	gint i;
	gchar *fileNameKey = NULL;
	gchar *fileNameParameters = NULL;

	*charges = chargesValue;
	*types = pdbTypes;
	*residues = residueNames;

	fileNameKey = get_suffix_name_file(NomFichier);
	fileNameKey = g_strdup_printf("%s.key",fileNameKey);
	/* printf("Key file = %s\n", fileNameKey);*/
	file = FOpen(fileNameKey, "rb");
	if(fileNameKey) g_free(fileNameKey);
 	if(file == NULL) return 0;
	tstr = g_malloc(BSIZE*sizeof(gchar));
	sprintf(sdum1,"NO");
	while(!feof(file))
	{
    		if(!fgets(tstr,BSIZE,file)) break;
    		k = sscanf(tstr,"%s %s",sdum1,sdum3);
		if(k!=2) continue;
		if(!strstr(sdum1,"parameters")) continue;
		if(strstr(sdum3,".prm")) fileNameParameters =  g_strdup(sdum3);
		else fileNameParameters =  g_strdup_printf("%s.prm",sdum3);
	}
	fclose(file);
	/* printf("param file = %s\n", fileNameParameters);*/
	if(!fileNameParameters) return 0;

	file = FOpen(fileNameParameters, "rb");
 	if(file == NULL) return 0;

	fseek(file, 0L, SEEK_SET);
	/* get the number of pdb type */
	n = 0;
	while(!feof(file))
	{
    		if(!fgets(tstr,BSIZE,file)) break;
    		k = sscanf(tstr,"%s %d %s %s",sdum1,&idum,sdum2,sdum3);
		if(k!=4) continue;
		if(idum<1) continue;
		if(!strstr(sdum1,"biotyp")) continue;
		n++;
		if(idum>n) n = idum;
	}
	if(n==0)
	{
		fclose(file);
		return 0;
	}
	pdbTypes = g_malloc(n*sizeof(gchar*));
	residueNames = g_malloc(n*sizeof(gchar*));
	chargesValue = g_malloc(n*sizeof(gdouble));
	for(i=0;i<n;i++)
	{
		pdbTypes[i] = NULL;
		residueNames[i] = NULL;
		chargesValue[i] = 0.0;
	}
	fseek(file, 0L, SEEK_SET);
	/* set pdb type, residueNames */
	while(!feof(file))
	{
    		if(!fgets(tstr,BSIZE,file)) break;
		if(strstr(tstr,"N-Terminal") || strstr(tstr,"C-Terminal")) 
		{
			k = sscanf(tstr,"%s %d %s %s %s",sdum1,&idum,sdum2,sdum0,sdum3);
			k--;
		}
		else k = sscanf(tstr,"%s %d %s %s",sdum1,&idum,sdum2,sdum3);
		if(k!=4) continue;
		if(idum<1) continue;
		if(!strstr(sdum1,"biotyp")) continue;
		if(idum>n) continue;
		i = idum-1;
		pdbTypes[i] = g_strdup(sdum2);
		sprintf(res,"UNK");
		k = 0;
		for(i=0;i<strlen(sdum3);i++)
		{
			if(sdum3[i]=='"') continue;
			res[k++] = toupper(sdum3[i]);
			if(k>2)break;
		}
		if(strstr(sdum3,"C-Terminus"))sprintf(res,"NME");
		else if(strstr(sdum3,"N-Terminus"))sprintf(res,"ACE");
		else if(strstr(sdum3,"C-Terminal"))sprintf(res,"CTE");
		else if(strstr(sdum3,"Adenosine"))sprintf(res,"DA");
		else if(strstr(sdum3,"Guanosine"))sprintf(res,"DG");
		else if(strstr(sdum3,"Cytidine"))sprintf(res,"DC");
		else if(strstr(sdum3,"Uridine"))sprintf(res,"DU");
		else if(strstr(sdum3,"Deoxyadenosine"))sprintf(res,"DA");
		else if(strstr(sdum3,"Deoxyguanosine"))sprintf(res,"DG");
		else if(strstr(sdum3,"Deoxycytidine"))sprintf(res,"DC");
		else if(strstr(sdum3,"Deoxythymidine"))sprintf(res,"DT");
		else if(strstr(sdum3,"5'") && strstr(sdum3,"RNA"))sprintf(res,"RG");
		else if(strstr(sdum3,"3'") && strstr(sdum3,"RNA"))sprintf(res,"RG");
		else if(strstr(sdum3,"5'") && strstr(sdum3,"DNA"))sprintf(res,"DT");
		else if(strstr(sdum3,"3'") && strstr(sdum3,"DNA"))sprintf(res,"DT");
		i =  idum-1;
		residueNames[i] = g_strdup(res);
	}
	for(i=0;i<n;i++)
	{
		if(!pdbTypes[i])  pdbTypes[i] = g_strdup("UNK");
		if(!residueNames[i])  residueNames[i] = g_strdup("UNK");
	}
	fseek(file, 0L, SEEK_SET);
	/* set charge */
	while(!feof(file))
	{
    		if(!fgets(tstr,BSIZE,file)) break;
    		k = sscanf(tstr,"%s %d %lf",sdum1,&idum,&ddum);
		if(k!=3) continue;
		if(idum<1) continue;
		if(!strstr(sdum1,"charge")) continue;
		if(idum>n) continue;
		i = idum-1;
		chargesValue[i] = ddum;
	}
	fclose(file);
	*charges = chargesValue;
	*types = pdbTypes;
	*residues = residueNames;
	/*
	for(i=0;i<n;i++)
		printf("%s %s %f\n",pdbTypes[i], residueNames[i] , chargesValue[i]);
		*/
	return n;
}
/*************************************************************************************/
void read_mol2_tinker_file_no_add_list(gchar *NomFichier,gchar*type)
{
	gchar *t;
	gboolean OK;
	gchar *AtomCoord[5];
	gchar *Type;
	FILE *fd;
	guint taille=BSIZE;
	guint i;
	gint j;
	gint Nc = -1;
	gchar** pdbTypes = NULL;
	gchar** residueNames = NULL;
	gdouble* charges = NULL;
	gint npdb = 0;
	gint ipdb = 0;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));


	fd = FOpen(NomFichier, "rb");
 	if(fd == NULL)
	{
		t = g_strdup_printf(_("Sorry\n I can not open \"%s\" file"),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		return;
	}
	t=g_malloc(taille);
	
	if(!strcmp(type,"MOL2"))
	{
		Type = g_strdup("<TRIPOS>ATOM");
 		OK=FALSE;
	}
	else
	{
		Type =  NULL;
 		OK=TRUE;
	}

  	while(Type && !feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
		if(strstr(t,Type))
		{
			OK = TRUE;
			break;
		}
	}
	if(!OK)
	{
		g_free(t);
		t = g_strdup_printf(_("Sorry\n \"%s\" is not at MOL2 format"),NomFichier);
		MessageGeom(t,_("Error"),TRUE);
		if(Type)
			g_free(Type);
		return;
	}
	if(!Type)
	{
    		{ char* e = fgets(t,taille,fd);}
		Nc=atoi(t);
		init_dipole();
		if(Nc<1)
		{
			OK=FALSE;
			NcentersXYZ=0;
			t = g_strdup_printf(_("Sorry\n \"%s\" is not at Tinker format"),NomFichier);
			MessageGeom(t,_("Error"),TRUE);
			return;
		}
		npdb = get_pdbtype_charge_from_tinker_amber_key_file
			(NomFichier, &pdbTypes, &residueNames, &charges);
	}
 	if(GeomXYZ) freeGeomXYZ();
 	if(VariablesXYZ) freeVariablesXYZ();
	init_dipole();
	GeomXYZ=g_malloc(sizeof(GeomXYZAtomDef));
	j=0;
    	GeomXYZ[j].typeConnections = NULL;
	while(!feof(fd)  && OK )
	{
		if(Nc>0 && j>Nc)
			break;
    		if(!fgets(t,taille,fd))
			break;
		if(atoi(t)<=0)
			break;
		GeomXYZ=g_realloc(GeomXYZ,(j+1)*sizeof(GeomXYZAtomDef));
		if(type)
		{
			gchar rname[100];
			gchar charge[100];
			gint k;
			ipdb = 0;
			GeomXYZ[j].ResidueNumber=0;
    			k = sscanf(t,"%d %s %s %s %s %s %d %s %s",&i,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3],AtomCoord[4],&GeomXYZ[j].ResidueNumber,
					rname,charge);
			GeomXYZ[j].Residue=NULL;
			GeomXYZ[j].Charge=NULL;
			if(k>=8) GeomXYZ[j].Residue=g_strdup(rname);
			if(k>=9) GeomXYZ[j].Charge=g_strdup_printf("%f",atof(charge));
		}
		else
		{
    			sscanf(t,"%d %s %s %s %s %d",&i,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3],&ipdb);
			GeomXYZ[j].ResidueNumber=0;
		}
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
    		GeomXYZ[j].typeConnections = NULL;

		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb= get_symb_atom(AtomCoord[0]);
		GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
		GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
		if(ipdb>0 && npdb>0 && ipdb<npdb)
		{
			GeomXYZ[j].Residue = g_strdup(residueNames[ipdb-1]);
			/* experimental */
			if(j==0) GeomXYZ[j].ResidueNumber = 0;
			else
			{
				if(strcmp(GeomXYZ[j].Residue,GeomXYZ[j-1].Residue))
					GeomXYZ[j].ResidueNumber = GeomXYZ[j-1].ResidueNumber+1;
				else
					GeomXYZ[j].ResidueNumber = GeomXYZ[j-1].ResidueNumber;
			}
			GeomXYZ[j].Charge=g_strdup_printf("%f",charges[ipdb-1]);
		}
		else
		{
			if(!GeomXYZ[j].Residue) GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
			if(!GeomXYZ[j].Charge) GeomXYZ[j].Charge=g_strdup("0.0");
		}
		if(Units==1)
		{
			GeomXYZ[j].X=g_strdup(AtomCoord[1]);
			GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
			GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
		}
		else
		{
			GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
			GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
			GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
		}
		GeomXYZ[j].Layer=g_strdup(" ");
		j++;
	}
	for(i=0;i<npdb;i++)
	{
		if(pdbTypes && pdbTypes[i]) g_free(pdbTypes[i]);
		if(residueNames && residueNames[i]) g_free(residueNames[i]);
	}
	NcentersXYZ = j;
	/* Read Bonds from MOL2 */
	if(type && !strcmp(type,"MOL2"))
	{
		if(!strstr(t,"<TRIPOS>BOND"))
		{
			fseek(fd, 0L, SEEK_SET);
  			while(!feof(fd))
			{
    				{ char* e = fgets(t,taille,fd);}
				if(strstr(t,"<TRIPOS>BOND"))
				{
					OK = TRUE;
					break;
				}
			}
		} else OK = TRUE;
		if(OK)
		{
			for(i=0;i<NcentersXYZ;i++) 
				GeomXYZ[i].typeConnections = g_malloc(NcentersXYZ*sizeof(gint));
			for(i=0;i<NcentersXYZ;i++) 
				for(j=0;j<NcentersXYZ;j++) 
					GeomXYZ[i].typeConnections[j] = 0;
		}
  		while(OK && !feof(fd))
		{
			gint a,i,j,d;
			gint k;
    			{ char* e = fgets(t,taille,fd);}
    			k = sscanf(t,"%d %d %d %d",&a,&i,&j,&d);
			if(k<=3 &&strstr(t,"ar"))
			{
				d = 1;
				k = 4;
			}
			if(k!=4) break;
			i--;
			j--;
			if(i>=NcentersXYZ || i<0) break;
			if(j>=NcentersXYZ || j<0) break;
			if(d<0) break;
			GeomXYZ[i].typeConnections[j] = d;
			GeomXYZ[j].typeConnections[i] = d;
		}
	}

	if(residueNames) g_free(residueNames);
	if(pdbTypes) g_free(pdbTypes);
	if(charges) g_free(charges);
	fclose(fd);

	g_free(t);
	for(i=0;i<5;i++)
		g_free(AtomCoord[i]);
	if(GeomIsOpen)
	{
		create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
	}
	else
	if(GeomDrawingArea == NULL)
	{
		/*
		switch(iprogram)
		{
			case PROG_IS_MOLPRO : 
				insert_molpro(2);
			break;
			case PROG_IS_GAUSS : 
				insert_gaussian(2);
			break;
			default :
				edit_geometry();
		}
		*/
		create_window_drawing();
        }
	if(GeomDrawingArea != NULL)
		rafresh_drawing();
}
/*****************************************************************************/
static void get_position(guint i,gdouble Pos[])
{
         if(!test(GeomXYZ[i].X))
                 Pos[0] = get_value_variableXYZ(GeomXYZ[i].X);
         else
                 Pos[0] = atof(GeomXYZ[i].X);
         if(!test(GeomXYZ[i].Y))
                 Pos[1] = get_value_variableXYZ(GeomXYZ[i].Y);
         else
                 Pos[1] = atof(GeomXYZ[i].Y);
         if(!test(GeomXYZ[i].Z))
                 Pos[2] = get_value_variableXYZ(GeomXYZ[i].Z);
         else
                 Pos[2] = atof(GeomXYZ[i].Z);
}
/*****************************************************************************/
static gboolean connected(guint i,guint j)
{
  gdouble distance;
  gdouble dif[3];
  gdouble Pos1[3];
  gdouble Pos2[3];
  guint k;
  SAtomsProp Prop;
  gdouble d;

  get_position(i,Pos1);
  get_position(j,Pos2);
  for (k=0;k<3;k++)
  	dif[k] = Pos1[k] - Pos2[k];
  distance = 0;
  for (k=0;k<3;k++)
	distance +=dif[k]*dif[k];
  distance = sqrt(distance);

  
  Prop = prop_atom_get(GeomXYZ[i].Symb);
  d = Prop.covalentRadii;
  Prop = prop_atom_get(GeomXYZ[j].Symb);
  d += Prop.covalentRadii;
  if(Units == 1)
	  d *= BOHR_TO_ANG;
  if(distance<d)
	return TRUE;
  else 
	return FALSE;
}
/********************************************************************************/
void save_tinker_file(G_CONST_RETURN gchar* FileName)
{
 guint i;
 guint j;
 FILE *fd;
 gdouble X;
 gdouble Y;
 gdouble Z;
 gchar *projectname = NULL;
 gchar *datafile = NULL;
 gchar *localdir = NULL;
 gchar *remotehost  = NULL;
 gchar *remoteuser  = NULL;
 gchar *remotepass  = NULL;
 gchar *remotedir  = NULL;
 gchar *temp  = NULL;

 temp = get_suffix_name_file(FileName);
 FileName = g_strdup_printf("%s.tnk",temp);
 g_free(temp);
 fd = FOpen(FileName, "w");
 if(fd == NULL)
 {
	gchar* t = g_strdup_printf(_("Sorry,\n I can not open %s file"),FileName);
	Message(t,_("Error"),TRUE);
	g_free(t);
	return;
 }
 fprintf(fd," %d    Gabedit generated tinker .xyz (mm2 param.)\n",NcentersXYZ);

 for(i=0;i<NcentersXYZ;i++)
 {
         if(!test(GeomXYZ[i].X))
                 X = get_value_variableXYZ(GeomXYZ[i].X);
         else
                 X = atof(GeomXYZ[i].X);
         if(!test(GeomXYZ[i].Y))
                 Y = get_value_variableXYZ(GeomXYZ[i].Y);
         else
                 Y = atof(GeomXYZ[i].Y);
         if(!test(GeomXYZ[i].Z))
                 Z = get_value_variableXYZ(GeomXYZ[i].Z);
         else
                 Z = atof(GeomXYZ[i].Z);
         if(Units==0)
         {
              X *= BOHR_TO_ANG;
              Y *= BOHR_TO_ANG;
              Z *= BOHR_TO_ANG;
         }
  	fprintf(fd,"%5d %s  %20.10f  %20.10f  %20.10f 1 ",
		i+1,GeomXYZ[i].Symb,X,Y,Z);
	for(j=0;j<NcentersXYZ;j++)
		if(i != j && connected(i,j))
  			fprintf(fd," %5d ",j+1);

  	fprintf(fd,"\n");

   }

  fclose(fd);
  datafile = get_name_file(FileName);
  temp = get_suffix_name_file(FileName);
  projectname = get_name_file(temp);
  localdir = get_name_dir(temp);
  if(lastdirectory)
	g_free(lastdirectory);
  lastdirectory = g_strdup(localdir);
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_TINKER],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_TINKER, NULL, defaultNetWorkProtocol);
  g_free(temp);
  g_free(datafile);
  g_free(projectname);
  g_free(localdir);
}
/************************************************************************************/
void save_geometry_tinker_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *FileName;

 	if(response_id != GTK_RESPONSE_OK) return;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	 save_tinker_file(FileName);
}
/********************************************************************************/
void save_tinker_file_entry(GtkWidget* entry)
{
	G_CONST_RETURN gchar *FileName;

	FileName = gtk_entry_get_text(GTK_ENTRY(entry));
	if ((!FileName) || (strcmp(FileName,"") == 0))
		return ;
	 save_tinker_file(FileName);

}
/********************************************************************************/
void read_mol_file_no_add_list(G_CONST_RETURN  gchar *NomFichier)
{
	gchar *t;
	gchar *AtomCoord[5];
	FILE *fd;
	guint taille=BSIZE;
	gint i;
	gint j;
	gint l;
	gint nAtoms=0;
	gint nBonds;

	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(gchar));

	t=g_malloc(taille);
	fd = FOpen(NomFichier, "rb");
	if(fd!=NULL)
	{
    		{ char* e = fgets(t,taille,fd);}
    		{ char* e = fgets(t,taille,fd);}
    		{ char* e = fgets(t,taille,fd);}
    		{ char* e = fgets(t,taille,fd);}

		sscanf(t,"%d %d", &nAtoms, &nBonds);
		if(nAtoms<1)
		{
			MessageGeom(_("Sorry\n this is not a mol file"),_("Error"),TRUE);
			return;
		}
	}
	init_dipole();
 	if(GeomXYZ) freeGeomXYZ();
 	if(VariablesXYZ) freeVariablesXYZ();
	NcentersXYZ = nAtoms;
	GeomXYZ=g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));
    	for(j=0;j<NcentersXYZ;j++) 
	{
		GeomXYZ[j].typeConnections = NULL;
		GeomXYZ[j].Symb=NULL;
		GeomXYZ[j].mmType=NULL;
		GeomXYZ[j].pdbType=NULL;
		GeomXYZ[j].Residue=NULL;
    		GeomXYZ[j].Charge=NULL;
    		GeomXYZ[j].Layer=NULL;
	}
	for(j=0;j<nAtoms;j++)
	{
  		if(!fgets(t,taille,fd))break;
    		sscanf(t,"%s %s %s %s",AtomCoord[1],AtomCoord[2],AtomCoord[3],AtomCoord[0]);
		delete_first_spaces(AtomCoord[0]);
		delete_last_spaces(AtomCoord[0]);
		AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 	l=strlen(AtomCoord[0]);
          	if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);
		GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
		GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
		GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
    		GeomXYZ[j].ResidueNumber=0;
   	 	if(Units==1)
    		{
    			GeomXYZ[j].X=g_strdup(AtomCoord[1]);
    			GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
    			GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
    		}
    		else
    		{
    			GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
    			GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
    			GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
    		}
    		GeomXYZ[j].Charge=g_strdup("0.0");
    		GeomXYZ[j].Layer=g_strdup(" ");
	}
	if(j!=nAtoms)
	{
		MessageGeom(_("Sorry\n I cannot read this file"),_("Error"),TRUE);
 		if(GeomXYZ) freeGeomXYZ();
		fclose(fd);
		return;
	}
	if(nBonds>0)
	{
		gint k;
    		for(j=0;j<NcentersXYZ;j++) 
		{
			GeomXYZ[j].typeConnections = g_malloc(NcentersXYZ*sizeof(gint));
    			for(i=0;i<NcentersXYZ;i++) GeomXYZ[j].typeConnections[i] = 0;
		}
		for(k=0;k<nBonds;k++)
		{
			gint d;
  			if(!fgets(t,taille,fd))break;
    			if(3!=sscanf(t,"%d %d %d",&i,&j,&d)) break;
			if(i>NcentersXYZ) break;
			if(j>NcentersXYZ) break;
			i--;
			j--;
			if(i<0) break;
			if(j<0) break;
			if(d<0) break;
    			GeomXYZ[i].typeConnections[j] = d;
    			GeomXYZ[j].typeConnections[i] = d;
		}
	}
	fclose(fd);
	g_free(t);
	calculMMTypes(FALSE);
	for(i=0;i<5;i++) g_free(AtomCoord[i]);
	if(GeomIsOpen) create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
	else if(GeomDrawingArea == NULL) create_window_drawing();
	if(GeomDrawingArea != NULL) rafresh_drawing();
}
/********************************************************************************/
void read_XYZ_file_no_add_list(G_CONST_RETURN  gchar *NomFichier)
{
 gchar *t;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 guint i;
 gint j;
 gint l;
 gint Nc;

 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(char));

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 OK=TRUE;
 if(fd!=NULL)
 {
  { char* e = fgets(t,taille,fd);}
  Nc=atoi(t);
  if(Nc<1)
  {
	OK=FALSE;
	MessageGeom(_("Sorry\n this is not a XYZ file"),_("Error"),TRUE);
	return;
  }
  if(OK)
   {
	init_dipole();
 	if(GeomXYZ)
   		freeGeomXYZ();
 	if(VariablesXYZ)
		freeVariablesXYZ();
	NcentersXYZ = Nc;
	GeomXYZ=g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));
    	for(j=0;j<NcentersXYZ;j++) GeomXYZ[j].typeConnections = NULL;
   }
  { char* e = fgets(t,taille,fd);}
  j=-1;
  while(!feof(fd) && OK && (j<(gint)NcentersXYZ))
  {
    j++;
    if(j>=(gint)NcentersXYZ)
	break;

    if(!fgets(t,taille,fd))
    {
	    if(j-1 != (gint)NcentersXYZ)
	    {
	    	NcentersXYZ = j -1;
	    }
	    break;
    };
    sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
	delete_first_spaces(AtomCoord[0]);
	delete_last_spaces(AtomCoord[0]);
	AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 l=strlen(AtomCoord[0]);
          if (l==2)
	 	AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
    GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);
    GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
    GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
    GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
    GeomXYZ[j].ResidueNumber=0;
    if(Units==1)
    {
    GeomXYZ[j].X=g_strdup(AtomCoord[1]);
    GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
    GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
    }
    else
    {
    GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
    GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
    GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
    }
    GeomXYZ[j].Charge=g_strdup("0.0");
    GeomXYZ[j].Layer=g_strdup(" ");
  }
  fclose(fd);
 }
 g_free(t);
 calculMMTypes(FALSE);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if(GeomIsOpen)
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
  else
  if(GeomDrawingArea == NULL)
  {
	/*
	switch(iprogram)
       	 {
		case PROG_IS_MOLPRO : 
			insert_molpro(2);
			break;
		case PROG_IS_GAUSS : 
			insert_gaussian(2);
			break;
	        default :
			edit_geometry();
       }
	*/
	create_window_drawing();
  }
 if(GeomDrawingArea != NULL) rafresh_drawing();
}
/*************************************************************************************/
void create_GeomXYZ_from_draw_grometry()
{
	gint j;
	gint jj;
	gboolean toSort = FALSE;
	gint iHigh = -1;
	gint i;
	gint* numOrd = NULL;
	gdouble Orig[3];
	gdouble X,Y,Z;
	gint nMaxCentersToVar = 500;

 	if(GeomXYZ) freeGeomXYZ();
 	if(VariablesXYZ) freeVariablesXYZ();

	get_origine_molecule_drawgeom(Orig);

	init_dipole();
	NcentersXYZ = Natoms;
	/*
	if(Natoms<1)
		return;

	GeomXYZ=g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));
	*/
	if(Natoms>0) GeomXYZ=g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));
	if(Natoms>0) numOrd=g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));
	for(i=0;i<(gint)NcentersXYZ;i++)
	{
		numOrd[i] = i;
		for(j=0;j<(gint)NcentersXYZ;j++)
    		if(geometry0[j].N-1==i)
		{ 
			numOrd[i] = j; 
			break;
		}
	}

	copy_connections(geometry0, geometry, Natoms);

	for(j=0;j<(gint)NcentersXYZ;j++)
	{
		jj = numOrd[j]; 
    		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
    		GeomXYZ[j].Symb=g_strdup(geometry0[jj].Prop.symbol);
    		GeomXYZ[j].mmType=g_strdup(geometry0[jj].mmType);
    		GeomXYZ[j].pdbType=g_strdup(geometry0[jj].pdbType);
    		GeomXYZ[j].Residue=g_strdup(geometry0[jj].Residue);
    		GeomXYZ[j].ResidueNumber=geometry0[jj].ResidueNumber;
    		GeomXYZ[j].typeConnections = g_malloc(NcentersXYZ*sizeof(gint));
		for(i=0;i<NcentersXYZ;i++) GeomXYZ[j].typeConnections[i] = get_connection_type(jj,numOrd[i]);
		X = geometry0[jj].X+Orig[0];
		Y = geometry0[jj].Y+Orig[1];
		Z = geometry0[jj].Z+Orig[2];
    		if(Units==1)
    		{
    			GeomXYZ[j].X=g_strdup_printf("%0.6f",X*BOHR_TO_ANG);
    			GeomXYZ[j].Y=g_strdup_printf("%0.6f",Y*BOHR_TO_ANG);
    			GeomXYZ[j].Z=g_strdup_printf("%0.6f",Z*BOHR_TO_ANG);
    		}
    		else
    		{
    			GeomXYZ[j].X=g_strdup_printf("%0.6f",X);
    			GeomXYZ[j].Y=g_strdup_printf("%0.6f",Y);
    			GeomXYZ[j].Z=g_strdup_printf("%0.6f",Z);
		}
    		GeomXYZ[j].Charge=g_strdup_printf("%0.6f",geometry0[jj].Charge);
		/* this part is very consomming if Ncentres > nMaxCentersToVar */
		if(NcentersXYZ<nMaxCentersToVar && geometry0[jj].Variable) set_variable_one_atom_in_GeomXYZ(j);
		if(geometry0[jj].Layer==LOW_LAYER) 
		{
			GeomXYZ[j].Layer=g_strdup("Low");
			toSort = TRUE;
		}
		else if(geometry0[jj].Layer==MEDIUM_LAYER) 
		{
			GeomXYZ[j].Layer=g_strdup("Medium");
			toSort = TRUE;
		}
		else 
		{
			if(iHigh<0) iHigh = j;
			GeomXYZ[j].Layer=g_strdup(" ");
		}
		/* if(j%50==0) fprintf(stderr,"DEBUG j=%d\n",j);*/
    	}
	if(numOrd) g_free(numOrd);
	numOrd = NULL;
	if(toSort && iHigh>=0)
	{
		GeomXYZAtomDef t;
		gint i;
		gint j;
		gint k;
		gint* oldNum = NULL;
		gint* N = NULL;

		oldNum = g_malloc(NcentersXYZ*sizeof(gint));
		for (i = 0; i <(gint)NcentersXYZ; i++) oldNum[i] = i;

		N = g_malloc(NcentersXYZ*sizeof(gint));
		for (i = 0; i <(gint)NcentersXYZ; i++) N[i] = 0;
		
		/* sorting Hight, Medium, Low */
		if(iHigh != 0)
		{
			t= GeomXYZ[0];
			GeomXYZ[0] = GeomXYZ[iHigh];
			GeomXYZ[iHigh] = t;

			j = oldNum[0];
			oldNum[0]=oldNum[iHigh];
			oldNum[iHigh]=j;
		}
  		for (i = 0; i <(gint)NcentersXYZ-1; i++)
 		{
			if(strcmp(GeomXYZ[i].Layer," ")==0) continue;
			k = i;
  			for (j = i+1; j <(gint)NcentersXYZ; j++)
  				if( strcmp(GeomXYZ[j].Layer," ")==0) {k = j; break;} 
			if(k!=i)
			{
				t= GeomXYZ[i];
				GeomXYZ[i] = GeomXYZ[k];
				GeomXYZ[k] = t;

				j = oldNum[i];
				oldNum[i]=oldNum[k];
				oldNum[k]=j;
			}
  		}
  		for (i = 0; i <(gint)NcentersXYZ-1; i++)
 		{
			if(strcmp(GeomXYZ[i].Layer," ")==0) continue;
			if(strcmp(GeomXYZ[i].Layer,"Medium")==0) continue;
			k = i;
  			for (j = i+1; j <(gint)NcentersXYZ; j++)
  				if( strcmp(GeomXYZ[j].Layer,"Medium")==0) {k = j; break;} 
			if(k!=i)
			{
				t= GeomXYZ[i];
				GeomXYZ[i] = GeomXYZ[k];
				GeomXYZ[k] = t;
				j = oldNum[i];
				oldNum[i]=oldNum[k];
				oldNum[k]=j;
			}
  		}
  		for (i = 0; i <(gint)NcentersXYZ; i++)
  		{
			if(!GeomXYZ[i].typeConnections) continue;
  			for (j = 0; j <(gint)NcentersXYZ; j++) N[j] = GeomXYZ[i].typeConnections[j];
  			for (j = 0; j <(gint)NcentersXYZ; j++) GeomXYZ[i].typeConnections[j]=N[oldNum[j]];
  		}
  		if(N) g_free(N);
  		if(oldNum) g_free(oldNum);
	}
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomIsOpen)
	{
		create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
		if(toSort) unSelectAllAtoms();
	}
}
/********************************************************************************/
void read_mol2_file(GabeditFileChooser *SelecFile , gint response_id)
{
 gchar *NomFichier;
 gchar *projectname = NULL;
 gchar *datafile = NULL;
 gchar *localdir = NULL;
 gchar *remotehost  = NULL;
 gchar *remoteuser  = NULL;
 gchar *remotepass  = NULL;
 gchar *remotedir  = NULL;
 gchar *temp  = NULL;

 if(response_id != GTK_RESPONSE_OK) return;

 NomFichier = gabedit_file_chooser_get_current_file(SelecFile);

 if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
 {
	MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
    return ;
 }

   read_mol2_tinker_file_no_add_list(NomFichier,"MOL2");

  datafile = get_name_file(NomFichier);
  temp = get_suffix_name_file(NomFichier);
  projectname = get_name_file(temp);
  localdir = get_name_dir(temp);
  if(lastdirectory)
	g_free(lastdirectory);
  lastdirectory = g_strdup(localdir);
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_MOL2],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_MOL2, NULL, defaultNetWorkProtocol);
  g_free(temp);
  g_free(datafile);
  g_free(projectname);
  g_free(localdir);
}
/********************************************************************************/
void read_tinker_file(GabeditFileChooser *SelecFile, gint response_id)
{
 gchar *NomFichier;
 gchar *projectname = NULL;
 gchar *datafile = NULL;
 gchar *localdir = NULL;
 gchar *remotehost  = NULL;
 gchar *remoteuser  = NULL;
 gchar *remotepass  = NULL;
 gchar *remotedir  = NULL;
 gchar *temp  = NULL;

 if(response_id != GTK_RESPONSE_OK) return;
 NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
  
 if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
 {
	MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
    return ;
 }

   read_mol2_tinker_file_no_add_list(NomFichier,"TINKER");

  datafile = get_name_file(NomFichier);
  temp = get_suffix_name_file(NomFichier);
  projectname = get_name_file(temp);
  localdir = get_name_dir(temp);
  if(lastdirectory)
	g_free(lastdirectory);
  lastdirectory = g_strdup(localdir);
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_TINKER],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_TINKER, NULL, defaultNetWorkProtocol);
  g_free(temp);
  g_free(datafile);
  g_free(projectname);
  g_free(localdir);
  
 
}
/********************************************************************************/
void read_pdb_file(GabeditFileChooser *SelecFile, gint response_id)
{
 gchar *NomFichier;
 gchar *projectname = NULL;
 gchar *datafile = NULL;
 gchar *localdir = NULL;
 gchar *remotehost  = NULL;
 gchar *remoteuser  = NULL;
 gchar *remotepass  = NULL;
 gchar *remotedir  = NULL;
 gchar *temp  = NULL;

 if(response_id != GTK_RESPONSE_OK) return;

 NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
  
 if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
 {
	MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
    return ;
 }

   read_pdb_file_no_add_list(NomFichier);

  datafile = get_name_file(NomFichier);
  temp = get_suffix_name_file(NomFichier);
  projectname = get_name_file(temp);
  localdir = get_name_dir(temp);
  if(lastdirectory)
	g_free(lastdirectory);
  lastdirectory = g_strdup(localdir);
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_PDB],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_PDB, NULL, defaultNetWorkProtocol);
  g_free(temp);
  g_free(datafile);
  g_free(projectname);
  g_free(localdir);
  
 
}
/********************************************************************************/
void read_mol_file(GabeditFileChooser *SelecFile, gint  response_id)
{
	gchar *NomFichier;
	gchar *projectname = NULL;
	gchar *datafile = NULL;
	gchar *localdir = NULL;
/*
	gchar *remotehost  = NULL;
	gchar *remoteuser  = NULL;
	gchar *remotepass  = NULL;
	gchar *remotedir  = NULL;
*/
	gchar *temp  = NULL;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
  
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
		return ;
	}

	read_mol_file_no_add_list(NomFichier);

	datafile = get_name_file(NomFichier);
	temp = get_suffix_name_file(NomFichier);
	projectname = get_name_file(temp);
	localdir = get_name_dir(temp);
	if(lastdirectory) g_free(lastdirectory);
	lastdirectory = g_strdup(localdir);
	/* CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_XYZ],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_XYZ, NULL, defaultNetWorkProtocol);*/
	g_free(temp);
	g_free(datafile);
	g_free(projectname);
	g_free(localdir);
}
/********************************************************************************/
 void read_XYZ_file(GabeditFileChooser *SelecFile, gint  response_id)
{
 gchar *t;
 gchar *NomFichier;
 gboolean OK;
 gchar *AtomCoord[5];
 FILE *fd;
 guint taille=BSIZE;
 guint i;
 gint j;
 gint l;
 gchar *projectname = NULL;
 gchar *datafile = NULL;
 gchar *localdir = NULL;
 gchar *remotehost  = NULL;
 gchar *remoteuser  = NULL;
 gchar *remotepass  = NULL;
 gchar *remotedir  = NULL;
 gchar *temp  = NULL;

 if(response_id != GTK_RESPONSE_OK) return;
 for(i=0;i<5;i++)
	AtomCoord[i]=g_malloc(taille*sizeof(char));
 NomFichier = gabedit_file_chooser_get_current_file(SelecFile);
  
 if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
 {
	MessageGeom(_("Sorry\n No file selected"),_("Error"),TRUE);
    return ;
 }

 t=g_malloc(taille);
 fd = FOpen(NomFichier, "rb");
 OK=TRUE;
 if(fd!=NULL)
 {
  { char* e = fgets(t,taille,fd);}
  NcentersXYZ=atoi(t);
  if(NcentersXYZ<1)
  {
	OK=FALSE;
	NcentersXYZ=0;
	MessageGeom(_("Sorry\n this is not a XYZ file"),_("Error"),TRUE);

	return;
  }
  if(OK)
   {
	init_dipole();
	GeomXYZ=g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));
    	for(j=0;j<NcentersXYZ;j++) GeomXYZ[j].typeConnections = NULL;
   }
  { char* e = fgets(t,taille,fd);}
  j=-1;
  while(!feof(fd) && OK && (j<(gint)NcentersXYZ))
  {
    j++;
    if(j>=(gint)NcentersXYZ)
	break;
    if(!fgets(t,taille,fd))
    {
	    if(j-1 != (gint)NcentersXYZ)
	    {
	    	NcentersXYZ = j -1;
	    }
	    break;
    };

    delete_first_spaces(t);
    sscanf(t,"%s %s %s %s",AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);
	AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 l=strlen(AtomCoord[0]);
          if (l==2)
	 	AtomCoord[0][1]=tolower(AtomCoord[0][1]);
    GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
    GeomXYZ[j].Symb=g_strdup(AtomCoord[0]);
    GeomXYZ[j].mmType=g_strdup(AtomCoord[0]);
    GeomXYZ[j].pdbType=g_strdup(AtomCoord[0]);
    GeomXYZ[j].Residue=g_strdup(AtomCoord[0]);
    GeomXYZ[j].ResidueNumber=0;
    if(Units==1)
    {
    GeomXYZ[j].X=g_strdup(AtomCoord[1]);
    GeomXYZ[j].Y=g_strdup(AtomCoord[2]);
    GeomXYZ[j].Z=g_strdup(AtomCoord[3]);
    }
    else
    {
    GeomXYZ[j].X=g_strdup(ang_to_bohr(AtomCoord[1]));
    GeomXYZ[j].Y=g_strdup(ang_to_bohr(AtomCoord[2]));
    GeomXYZ[j].Z=g_strdup(ang_to_bohr(AtomCoord[3]));
    }
    GeomXYZ[j].Charge=g_strdup("0.0");
    GeomXYZ[j].Layer=g_strdup(" ");
  }
  fclose(fd);
 }
 g_free(t);
 calculMMTypes(FALSE);
 for(i=0;i<5;i++)
	g_free(AtomCoord[i]);
 if(GeomIsOpen)
 	append_list();
 if(GeomDrawingArea != NULL)
	rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();

  datafile = get_name_file(NomFichier);
  temp = get_suffix_name_file(NomFichier);
  projectname = get_name_file(temp);
  localdir = get_name_dir(temp);
  if(lastdirectory)
	g_free(lastdirectory);
  lastdirectory = g_strdup(localdir);
  CreeFeuille(treeViewProjects, noeud[GABEDIT_TYPENODE_XYZ],projectname,datafile,localdir,remotehost,remoteuser,remotepass,remotedir,GABEDIT_TYPENODE_XYZ, NULL, defaultNetWorkProtocol);
  g_free(temp);
  g_free(datafile);
  g_free(projectname);
  g_free(localdir);
  
 
}
/********************************************************************************/
void put_geomXYZ_in_list()
{

 if(GeomXYZ != NULL )
	append_list();
 if(GeomDrawingArea != NULL)
	rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();
}
/********************************************************************************/
void selc_cif_file_nosym()
{
  GtkWidget *SelecFile=NULL;
  gchar* patternscif[] = {"*.cif","*.mmcif","*",NULL};

  SelecFile = gabedit_file_chooser_new(_("Read geometry from a CIF file (don\'t apply symm)"), GTK_FILE_CHOOSER_ACTION_OPEN);
  gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternscif);

  if(lastdirectory)
  {
	gchar* t = g_strdup_printf("%s%sdump.xyz",lastdirectory,G_DIR_SEPARATOR_S);
	gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(SelecFile),t);
	g_free(t);
  }

  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(SelecFile));
  if(WindowGeom) gtk_window_set_transient_for(GTK_WINDOW(SelecFile),GTK_WINDOW(WindowGeom));
  else gtk_window_set_transient_for(GTK_WINDOW(SelecFile),GTK_WINDOW(Fenetre));
  gtk_window_set_modal (GTK_WINDOW (SelecFile), TRUE);

  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_cif_file_nosym), GTK_OBJECT(SelecFile));

  g_signal_connect_swapped(SelecFile, "response", G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(SelecFile));
  g_signal_connect_swapped(SelecFile, "close", G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(SelecFile));
  gtk_widget_show(SelecFile);
}
/********************************************************************************/
void selc_XYZ_file(GabEditTypeFileGeom itype)
{
  GtkWidget *SelecFile=NULL;
  gchar* patternsxyz[] = {"*.xyz","*",NULL};
  gchar* patternsmol2[] = {"*.mol2","*",NULL};
  gchar* patternspdb[] = {"*.pdb","*",NULL};
  gchar* patternshin[] = {"*.hin","*",NULL};
  gchar* patternsaimall[] = {"*.sum","*",NULL};
  gchar* patternstnk[] = {"*.tnk","*",NULL};
  gchar* patternslog[] = {"*.log","*",NULL};
  gchar* patternsout[] = {"*.out","*.log","*",NULL};
  gchar* patternsaux[] = {"*.aux","*",NULL};
  gchar* patternsgab[] = {"*.gab","*",NULL};
  gchar* patternsmol[] = {"*.mol","*",NULL};
  gchar* patternsirc[] = {"*.irc","*",NULL};
  gchar* patternsfchk[] = {"*.fchk","*",NULL};
  gchar* patternsposcar[] = {"POS*","*",NULL};
  gchar* patternsxml[] = {"*.xml","*",NULL};
  gchar* patternsall[] = {"*",NULL};
  gchar* patternscif[] = {"*.cif","*",NULL};
  gchar* patternswfx[] = {"*.wfx","*",NULL};


  switch(itype){
  case GABEDIT_TYPEFILEGEOM_NEW : return;
	   break;
  case GABEDIT_TYPEFILEGEOM_XYZ :
	   SelecFile = gabedit_file_chooser_new(_("Read XYZ file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsxyz);
	   break;
  case GABEDIT_TYPEFILEGEOM_MOL :
	   SelecFile = gabedit_file_chooser_new(_("Read Mol file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsmol);
	   break;
  case GABEDIT_TYPEFILEGEOM_DALTONFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a dalton output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_DALTONLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a dalton output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_FIREFLYFIRST : 
  case GABEDIT_TYPEFILEGEOM_GAMESSFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a Gamess output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternslog);
	   break;
  case GABEDIT_TYPEFILEGEOM_FIREFLYLAST : 
  case GABEDIT_TYPEFILEGEOM_GAMESSLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a Gamess output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternslog);
	   break;
  case GABEDIT_TYPEFILEGEOM_GAMESSIRC : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a Gamess IRC file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsirc);
	   break;
  case GABEDIT_TYPEFILEGEOM_TURBOMOLEFIRST : 
  case GABEDIT_TYPEFILEGEOM_TURBOMOLELAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a Turbomole output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_GAUSSOUTFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a gaussian output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternslog);
	   break;
  case GABEDIT_TYPEFILEGEOM_GAUSSOUTLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a gaussian output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternslog);
	   break;
  case GABEDIT_TYPEFILEGEOM_GAUSSIAN_FCHK : 
	   SelecFile = gabedit_file_chooser_new(_("Read the geometry from a gaussian fchk file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsfchk);
	   break;
  case GABEDIT_TYPEFILEGEOM_MOLCASOUTFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a molcas output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_MOLCASOUTLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a molcas output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_MOLPROOUTFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a molpro output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_MOLPROOUTLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a molpro output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_MOL2 : 
	   SelecFile = gabedit_file_chooser_new(_("Read MOL2 file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsmol2);
	   break;
  case GABEDIT_TYPEFILEGEOM_MPQCOUTFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a MPQC output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_MPQCOUTLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a MPQC output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_VASPXMLFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a VASP/xml file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsxml);
	   break;
  case GABEDIT_TYPEFILEGEOM_VASPXMLLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a VASP/xml file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsxml);
	   break;

  case GABEDIT_TYPEFILEGEOM_ORCAOUTFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a ORCA output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_ORCAOUTLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a ORCA output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;

  case GABEDIT_TYPEFILEGEOM_VASPOUTFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a VASP output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsall);
	   break;
  case GABEDIT_TYPEFILEGEOM_VASPOUTLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a VASP output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsall);
	   break;

  case GABEDIT_TYPEFILEGEOM_NWCHEMOUTFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a NWCHEM output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_NWCHEMOUTLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a NWCHEM output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_PSICODEOUTFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a Psicode output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_PSICODEOUTLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a Psicode output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_QCHEMOUTFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a Q-Chem output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_QCHEMOUTLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a Q-Chem output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_MOPACOUTFIRST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the first geometry from a Mopac output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_MOPACOUTLAST : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a Mopac output file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_MOPACAUX : 
	   SelecFile = gabedit_file_chooser_new(_("Read the last geometry from a Mopac aux file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsaux);
	   break;
  case GABEDIT_TYPEFILEGEOM_WFX : 
	   SelecFile = gabedit_file_chooser_new(_("Read the geometry from a WFX file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternswfx);
	   break;
  case GABEDIT_TYPEFILEGEOM_TINKER :
	   SelecFile = gabedit_file_chooser_new(_("Read Tinker file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternstnk);
	   break;
  case GABEDIT_TYPEFILEGEOM_PDB :
	   SelecFile = gabedit_file_chooser_new(_("Read pdb file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternspdb);
	   break;
  case GABEDIT_TYPEFILEGEOM_HIN :
	   SelecFile = gabedit_file_chooser_new(_("Read hyperchem file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternshin);
	   break;
  case GABEDIT_TYPEFILEGEOM_AIMALL :
	   SelecFile = gabedit_file_chooser_new(_("Read AIMAll file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsaimall);
	   break;
  case GABEDIT_TYPEFILEGEOM_GABEDIT :
	   SelecFile = gabedit_file_chooser_new(_("Read Gabedit file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsgab);
	   break;
  case GABEDIT_TYPEFILEGEOM_MOPACOUTSCAN :
	   SelecFile = gabedit_file_chooser_new(_("Read geometries from a Mopac scan output"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsout);
	   break;
  case GABEDIT_TYPEFILEGEOM_VASPPOSCAR :
	   SelecFile = gabedit_file_chooser_new(_("Read geometries from a POSCAR/VASP file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternsposcar);
	   break;
  case GABEDIT_TYPEFILEGEOM_CIF :
	   SelecFile = gabedit_file_chooser_new(_("Read geometries from a CIF file"), GTK_FILE_CHOOSER_ACTION_OPEN);
   	   gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(SelecFile),patternscif);
	   break;
  case GABEDIT_TYPEFILEGEOM_MOLDEN : return;
  case GABEDIT_TYPEFILEGEOM_DALTONIN : return;
  case GABEDIT_TYPEFILEGEOM_FIREFLYIN : return;
  case GABEDIT_TYPEFILEGEOM_GAMESSIN : return;
  case GABEDIT_TYPEFILEGEOM_GAUSSIN : return;
  case GABEDIT_TYPEFILEGEOM_MOLCASIN : return;
  case GABEDIT_TYPEFILEGEOM_MOLPROIN : return;
  case GABEDIT_TYPEFILEGEOM_MPQCIN : return;
  case GABEDIT_TYPEFILEGEOM_ORCAIN : return;
  case GABEDIT_TYPEFILEGEOM_VASPIN : return;
  case GABEDIT_TYPEFILEGEOM_NWCHEMIN : return;
  case GABEDIT_TYPEFILEGEOM_QCHEMIN : return;
  case GABEDIT_TYPEFILEGEOM_MOPACIN : return;
  case GABEDIT_TYPEFILEGEOM_GAUSSIAN_ZMATRIX : return;
  case GABEDIT_TYPEFILEGEOM_MOPAC_ZMATRIX : return;
  case GABEDIT_TYPEFILEGEOM_UNKNOWN : return;
  case GABEDIT_TYPEFILEGEOM_PSICODEIN : return;
  }
  if(lastdirectory)
  {
	gchar* t = g_strdup_printf("%s%sdump.xyz",lastdirectory,G_DIR_SEPARATOR_S);
	gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(SelecFile),t);
	g_free(t);
  }

  gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(SelecFile));
  if(WindowGeom) gtk_window_set_transient_for(GTK_WINDOW(SelecFile),GTK_WINDOW(WindowGeom));
  else gtk_window_set_transient_for(GTK_WINDOW(SelecFile),GTK_WINDOW(Fenetre));
  gtk_window_set_modal (GTK_WINDOW (SelecFile), TRUE);


  switch(itype){
  case GABEDIT_TYPEFILEGEOM_XYZ : 
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_XYZ_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_MOL : 
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_mol_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_DALTONFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_dalton_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_DALTONLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_dalton_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_FIREFLYFIRST :
  case GABEDIT_TYPEFILEGEOM_GAMESSFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_gamess_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_GAMESSLAST :
  case GABEDIT_TYPEFILEGEOM_FIREFLYLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_gamess_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_GAMESSIRC :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_irc_gamess_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_GAUSSOUTFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_gaussian_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_GAUSSOUTLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_gaussian_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_TURBOMOLEFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_turbomole_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_TURBOMOLELAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_turbomole_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_GAUSSIAN_FCHK :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_fchk_gaussian_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_MOLCASOUTFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_molcas_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_MOLCASOUTLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_molcas_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_MOLPROOUTFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_molpro_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_MOLPROOUTLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_molpro_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_MPQCOUTFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_mpqc_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_MPQCOUTLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_mpqc_file), GTK_OBJECT(SelecFile)); break;

  case GABEDIT_TYPEFILEGEOM_VASPXMLFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_vasp_xml_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_VASPXMLLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_vasp_xml_file), GTK_OBJECT(SelecFile)); break;

  case GABEDIT_TYPEFILEGEOM_ORCAOUTFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_orca_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_ORCAOUTLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_orca_file), GTK_OBJECT(SelecFile)); break;

  case GABEDIT_TYPEFILEGEOM_VASPOUTFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_vasp_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_VASPOUTLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_vasp_file), GTK_OBJECT(SelecFile)); break;

  case GABEDIT_TYPEFILEGEOM_QCHEMOUTFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_qchem_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_QCHEMOUTLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_qchem_file), GTK_OBJECT(SelecFile)); break;

  case GABEDIT_TYPEFILEGEOM_NWCHEMOUTFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_nwchem_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_NWCHEMOUTLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_nwchem_file), GTK_OBJECT(SelecFile)); break;

  case GABEDIT_TYPEFILEGEOM_PSICODEOUTFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_psicode_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_PSICODEOUTLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_psicode_file), GTK_OBJECT(SelecFile)); break;


  case GABEDIT_TYPEFILEGEOM_MOPACOUTFIRST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_first_mopac_output_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_MOPACOUTLAST :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_mopac_output_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_MOPACAUX :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_last_mopac_aux_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_WFX :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_wfx_file), GTK_OBJECT(SelecFile)); break;

  case GABEDIT_TYPEFILEGEOM_MOL2 :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_mol2_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_TINKER :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_tinker_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_PDB :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_pdb_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_HIN :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_hin_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_AIMALL :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_aimall_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_GABEDIT :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_gabedit_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_MOPACOUTSCAN :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_geometries_conv_mopac_scan), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_VASPPOSCAR :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_poscar_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_CIF :
	  g_signal_connect (SelecFile, "response",  G_CALLBACK (read_cif_file), GTK_OBJECT(SelecFile)); break;
  case GABEDIT_TYPEFILEGEOM_NEW : 
  case GABEDIT_TYPEFILEGEOM_MOLDEN : 
  case GABEDIT_TYPEFILEGEOM_GAUSSIN : 
  case GABEDIT_TYPEFILEGEOM_DALTONIN : 
  case GABEDIT_TYPEFILEGEOM_FIREFLYIN : 
  case GABEDIT_TYPEFILEGEOM_GAMESSIN : 
  case GABEDIT_TYPEFILEGEOM_MOLCASIN : 
  case GABEDIT_TYPEFILEGEOM_MOLPROIN : 
  case GABEDIT_TYPEFILEGEOM_MPQCIN : 
  case GABEDIT_TYPEFILEGEOM_ORCAIN : 
  case GABEDIT_TYPEFILEGEOM_VASPIN : 
  case GABEDIT_TYPEFILEGEOM_NWCHEMIN : 
  case GABEDIT_TYPEFILEGEOM_QCHEMIN : 
  case GABEDIT_TYPEFILEGEOM_MOPACIN : 
  case GABEDIT_TYPEFILEGEOM_GAUSSIAN_ZMATRIX : 
  case GABEDIT_TYPEFILEGEOM_MOPAC_ZMATRIX : 
  case GABEDIT_TYPEFILEGEOM_PSICODEIN : 
  case GABEDIT_TYPEFILEGEOM_UNKNOWN : return;
  }
  g_signal_connect_swapped(SelecFile, "response", G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(SelecFile));
  g_signal_connect_swapped(SelecFile, "close", G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(SelecFile));
  gtk_widget_show(SelecFile);
}
/********************************************************************************/
void save_xyzmol2tinkerpdbhin_file(GtkWidget* win,gpointer data)
{
	GtkWidget * ButtonXYZ = g_object_get_data(G_OBJECT(win),"ButtonXYZ");
	GtkWidget * ButtonMol2 = g_object_get_data(G_OBJECT(win),"ButtonMol2");
	GtkWidget * ButtonPDB = g_object_get_data(G_OBJECT(win),"ButtonPDB");
	GtkWidget * ButtonHIN = g_object_get_data(G_OBJECT(win),"ButtonHIN");
	GtkWidget * entry = g_object_get_data(G_OBJECT(win),"Entry");
	GtkWidget * buttonDirSelector = g_object_get_data(G_OBJECT(win),"ButtonDirSelector");
	G_CONST_RETURN gchar *entrytext;
	gchar *dirName;
	gchar *fileName;

  	entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
  	dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
  	fileName = get_dir_file_name(dirName,entrytext);
	if ((!fileName) || (strcmp(fileName,"") == 0)) return ;
	if (GTK_TOGGLE_BUTTON (ButtonXYZ)->active)
	{
	 	save_xyz_file(fileName);
	}
	 else
	 if (GTK_TOGGLE_BUTTON (ButtonMol2)->active)
	 {
		save_mol2_file(fileName);
	 }
	 else
	 if (GTK_TOGGLE_BUTTON (ButtonPDB)->active)
	 {
		 save_pdb_file(fileName);
	 }
	 if (GTK_TOGGLE_BUTTON (ButtonHIN)->active)
	 {
		 save_hin_file(fileName);
	 }
	 else
	 {
		 save_tinker_file(fileName);
	 }
}
/********************************************************************************/
static void reset_extended_file(gpointer data,G_CONST_RETURN gchar* ext)
{
	GtkWidget* entry = GTK_WIDGET(data);
	G_CONST_RETURN gchar* entrytext = gtk_entry_get_text(GTK_ENTRY(entry)); 
	gchar*	temp = get_filename_without_ext(entrytext);
	gchar*	t = g_strdup_printf("%s.%s",temp,ext);
	gtk_entry_set_text(GTK_ENTRY(entry),t);
	g_free(t);
	g_free(temp);
}
/********************************************************************************/
static void reset_extended_xyz_file(GtkWidget* b,gpointer data)
{
	reset_extended_file(data,"xyz");
}
/********************************************************************************/
static void reset_extended_mol2_file(GtkWidget* b,gpointer data)
{
	reset_extended_file(data,"mol2");
}
/********************************************************************************/
static void reset_extended_tinker_file(GtkWidget* b,gpointer data)
{
	reset_extended_file(data,"tnk");
}
/********************************************************************************/
static void reset_extended_pdb_file(GtkWidget* b,gpointer data)
{
	reset_extended_file(data,"pdb");
}
/********************************************************************************/
static void reset_extended_hin_file(GtkWidget* b,gpointer data)
{
	reset_extended_file(data,"hin");
}
/********************************************************************************/
void create_window_save_xyzmol2tinkerpdbhin()
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *buttonDirSelector;
  GtkWidget *ButtonXYZ;
  GtkWidget *ButtonMol2;
  GtkWidget *ButtonTinker;
  GtkWidget *ButtonPDB;
  GtkWidget *ButtonHIN;
  GtkWidget *entry;
  GtkWidget *Win = WindowGeom;
  gchar      *labelt = g_strdup(" File  : ");
  gchar      *titre=g_strdup("Save in xyz/mol2/tinker/Hyperchem file");
  gchar* fileName  = g_strdup_printf("%s.xyz",fileopen.projectname);

  if(NcentersXYZ<1)
  {
    MessageGeom(_(" Sorry No Center  !"),_("Error"),TRUE);
    return;
  }
  if(!Win)
	  Win = Fenetre;

  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(fp),titre);
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Fenetre));

  add_child(Win,fp,gtk_widget_destroy,_(" Save XYZ "));

  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)delete_child,NULL);
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)gtk_widget_destroy,NULL);

  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);
  vboxall = create_vbox(fp);

  
  frame = gtk_frame_new ("Type of file");
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  hbox = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vboxframe), hbox, FALSE, FALSE, 5);
  gtk_widget_show (hbox);
  ButtonXYZ = gtk_radio_button_new_with_label( NULL,"XYZ " );
  gtk_box_pack_start (GTK_BOX (hbox), ButtonXYZ, FALSE, FALSE, 5);
  gtk_widget_show (ButtonXYZ);
  ButtonMol2 = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonXYZ)), "Mol2"); 
   gtk_box_pack_start (GTK_BOX (hbox), ButtonMol2, FALSE, FALSE, 5);
   gtk_widget_show (ButtonMol2);
  ButtonTinker = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonXYZ)), "Tinker"); 
   gtk_box_pack_start (GTK_BOX (hbox), ButtonTinker, FALSE, FALSE, 5);
   gtk_widget_show (ButtonTinker);

  hbox = gtk_hbox_new(FALSE, 10);
  gtk_box_pack_start (GTK_BOX (vboxframe), hbox, FALSE, FALSE, 5);


   ButtonPDB = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonXYZ)), "PDB"); 
   gtk_box_pack_start (GTK_BOX (hbox), ButtonPDB, FALSE, FALSE, 5);
   gtk_widget_show (ButtonPDB);

   ButtonHIN = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonXYZ)), "Hyperchem"); 
   gtk_box_pack_start (GTK_BOX (hbox), ButtonHIN, FALSE, FALSE, 5);
   gtk_widget_show (ButtonHIN);
   create_hseparator(vboxframe); 
 


  create_table_browser(Win,vboxframe);
  entry = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"EntryFileName"));
  gtk_entry_set_text(GTK_ENTRY(entry),fileName);
  buttonDirSelector = (GtkWidget*)(g_object_get_data(G_OBJECT(Win),"ButtonDirSelector"));
  if(fileopen.localdir && strcmp(fileopen.localdir,"NoName")!=0) gtk_file_chooser_set_current_folder( GTK_FILE_CHOOSER(buttonDirSelector), fileopen.localdir);

  create_hseparator(vboxframe); 
  g_signal_connect(G_OBJECT(ButtonXYZ),"clicked",(GCallback)reset_extended_xyz_file,(gpointer)(entry));
  g_signal_connect(G_OBJECT(ButtonMol2),"clicked",(GCallback)reset_extended_mol2_file,(gpointer)(entry));
  g_signal_connect(G_OBJECT(ButtonTinker),"clicked",(GCallback)reset_extended_tinker_file,(gpointer)(entry));
  g_signal_connect(G_OBJECT(ButtonPDB),"clicked",(GCallback)reset_extended_pdb_file,(gpointer)(entry));
  g_signal_connect(G_OBJECT(ButtonHIN),"clicked",(GCallback)reset_extended_hin_file,(gpointer)(entry));

  g_object_set_data(G_OBJECT (fp), "ButtonXYZ",ButtonXYZ);
  g_object_set_data(G_OBJECT (fp), "ButtonMol2",ButtonMol2);
  g_object_set_data(G_OBJECT (fp), "ButtonTinker",ButtonTinker);
  g_object_set_data(G_OBJECT (fp), "ButtonPDB",ButtonPDB);
  g_object_set_data(G_OBJECT (fp), "ButtonHIN",ButtonHIN);
  g_object_set_data(G_OBJECT (fp), "Entry",entry);
  g_object_set_data(G_OBJECT (fp), "ButtonDirSelector",buttonDirSelector);
  
  create_hseparator(vboxall); 
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start (GTK_BOX (vboxall), hbox, FALSE, FALSE, 5);
  gtk_widget_realize(fp);


  button = create_button(fp,_("OK"));
  gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, FALSE, 3);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(save_xyzmol2tinkerpdbhin_file),GTK_OBJECT(fp));
  g_signal_connect_swapped(G_OBJECT(button),"clicked",(GCallback)delete_child,GTK_OBJECT(fp));

  button = create_button(fp,_("Cancel"));
  gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, FALSE, 3);
  g_signal_connect_swapped(G_OBJECT(button),"clicked",(GCallback)delete_child,GTK_OBJECT(fp));
  gtk_widget_show (button);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);


  g_free(labelt);
  g_free(fileName);
   
  gtk_widget_show_all(fp);
}


/********************************************************************************/
void create_geomXYZ_list(GtkWidget *vbox, GabEditTypeFileGeom readfile)
{
        GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	GtkWidget *scr;
	guint i;
	guint Factor=7;
	guint widall=0;
	gchar *titres[NUMBER_LIST_XYZ]={
  			"Nr",
	  		" Symbol ", 
	  		" MM Type  ",
	  		" PDB Type ",
	  		" Residue ",
			" X ", 
			" Y ", 
			" Z ", 
			" Charge "," Layer    "
  		};
	gint width[NUMBER_LIST_XYZ]={4,6,8,8,8,10,10,10,10,10 }; 
	GtkUIManager *manager = NULL;

  
	if(iprogram == PROG_IS_MOLPRO) NCr = NUMBER_LIST_XYZ-1;
	else NCr = NUMBER_LIST_XYZ;

	NSA[0] = -1;
	NSA[1] = NSA[2] = NSA[3] =-1;
	MethodeGeom = GEOM_IS_XYZ;
	if(GeomXYZ!=NULL && readfile != GABEDIT_TYPEFILEGEOM_UNKNOWN)
	{
 		freeGeomXYZ();
		if(VariablesXYZ) freeVariablesXYZ();
	}

	LineSelected=-1;
	if(readfile != GABEDIT_TYPEFILEGEOM_NEW) selc_XYZ_file(readfile);
	else NcentersXYZ=0;
	for(i=0;(gint)i<NCr;i++) widall+=width[i];
	widall=widall*Factor+60;

	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeight*0.4));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 2);

	store = gtk_list_store_new (NUMBER_LIST_XYZ*2,
		       	G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN, 
			G_TYPE_STRING, G_TYPE_BOOLEAN, 
			G_TYPE_STRING, G_TYPE_BOOLEAN,
		       	G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN,
		       	G_TYPE_STRING, G_TYPE_BOOLEAN,
			G_TYPE_STRING, G_TYPE_BOOLEAN
			);
        model = GTK_TREE_MODEL (store);

	list = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (list), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (list), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (list), TRUE);
	for (i=0;(gint)i<NCr;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, titres[i]);
		gtk_tree_view_column_set_min_width(column, width[i]*Factor);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i+i, "editable",i+i+1,NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (list), column);
		g_object_set_data(G_OBJECT(renderer),"NumColumn", GINT_TO_POINTER(i));
		g_signal_connect (renderer, "edited", G_CALLBACK (editedGeom), model);
	}
	gtk_container_add(GTK_CONTAINER(scr),list);
	/* Drag and Drop */
	/*
	gtk_tree_view_enable_model_drag_source (GTK_TREE_VIEW (list), GDK_BUTTON1_MASK, row_targets, G_N_ELEMENTS (row_targets), GDK_ACTION_MOVE);
	gtk_tree_view_enable_model_drag_dest (GTK_TREE_VIEW (list), row_targets, G_N_ELEMENTS (row_targets), GDK_ACTION_MOVE);
	*/

  
  
	set_base_style(list,55000,55000,55000);
	gtk_widget_show (list);

	if(WindowGeom) manager = newMenuXYZGeom(WindowGeom);
	else manager = newMenuXYZGeom(Fenetre);
  	g_signal_connect(G_OBJECT (model), "row_deleted", G_CALLBACK(row_deleted), NULL);      
  	g_signal_connect(G_OBJECT (model), "row_inserted", G_CALLBACK(row_inserted), NULL);      

	if(GeomXYZ != NULL) put_geomXYZ_in_list();
  	g_signal_connect(G_OBJECT (list), "button_press_event", G_CALLBACK(event_dispatcher), manager);


}
/********************************************************************************/
static void DelVariable(GtkWidget *w,gpointer data)
{
   guint i,imin;
   gchar *message;
    
        if(LineSelectedV<0)
              LineSelectedV=NVariablesXYZ-1;
	if(VariablesXYZ[LineSelectedV].Used) {
        message=g_strdup_printf(_("Sorry\n %s \n is used in Geometry"),VariablesXYZ[LineSelectedV].Name);
	MessageGeom(message,_("Error"),TRUE);
        return;
        }
	if(NVariablesXYZ>0)
	{
		imin=LineSelectedV;
   		removeFromList(listv, LineSelectedV);
  		for(i=imin;i<NVariablesXYZ-1;i++)
		{
			VariablesXYZ[i].Name=g_strdup(VariablesXYZ[i+1].Name);
			VariablesXYZ[i].Value=g_strdup(VariablesXYZ[i+1].Value);
			VariablesXYZ[i].Used=VariablesXYZ[i+1].Used;
		}
  		NVariablesXYZ--;
                if(NVariablesXYZ>0) 
	 	 VariablesXYZ=g_realloc(VariablesXYZ,NVariablesXYZ*sizeof(VariablesXYZDef));
		else
			freeVariablesXYZ();
	} 
        LineSelectedV=-1;
}
/********************************************************************************/
static gboolean TestVariablesXYZCreated(gchar *NewName,gint j)
{
	gint i;
	gboolean k;
    
        k=FALSE;
        for (i=0;i<(gint)NVariablesXYZ;i++)
        {
		if (j==i)continue;
		if (!strcmp(VariablesXYZ[i].Name, NewName) )
                {
                	k=TRUE;
   			break;
                }
        }
 return k;
}
/********************************************************************************/
static void EditVariable(GtkWidget *w,gpointer Entree)
{
  gchar *texts[2];
  gchar *message;
  gint Nc;
  
  DestroyDialog = TRUE;
  texts[0] = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryV[0])));
  texts[1] = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryV[1])));
  if(!variable_name_valid(texts[0]))
  {
	show_forbidden_characters();
  	DestroyDialog = FALSE;
      	return;
  } 
  if ( strcmp(texts[0], "") && strcmp(texts[1], "") )
  {
  	Nc=LineSelectedV;
	if(Nc<0)
	  Nc = LineSelectedOld;
  }
   else
     {
	MessageGeom(_("Sorry a Entry text is void !\n"),_("Error"),TRUE);
  	DestroyDialog = FALSE;
      	return;
     }

  if(TestVariablesXYZCreated(texts[0],Nc) )
  {
	MessageGeom(_("Sorry a other variable have any Name !\n"),_("Error"),TRUE);
  	DestroyDialog = FALSE;
      	return;
  } 

  if(!test(texts[1]))
  {
	message=g_strdup_printf(_("Sorry %s is not a number \n"),texts[1]);
	MessageGeom(message,_("Error"),TRUE);
  	DestroyDialog = FALSE;
  }
  else
  {
  if(test(texts[1]) && !testpointeE(texts[1]) )
		texts[1]=g_strdup_printf("%s.0",texts[1]);
  	VariablesXYZ[Nc].Name=g_strdup(texts[0]);
  	VariablesXYZ[Nc].Value=g_strdup(texts[1]);

   	removeFromList(listv, Nc);
	insertToList(listv, Nc, texts, 2);
  }
}
/********************************************************************************/
static void DialogueDeleteV()
{
  GtkWidget *Dialogue;
  GtkWidget *Label;
  GtkWidget *Bouton;
  GtkWidget *frame;
  GtkWidget *vboxframe;
  gchar *message;

  if (NVariablesXYZ <1)
  {
    MessageGeom(_(" No Variable to delet !"),_("Error"),TRUE);
    return ;
  }
    
  if(LineSelectedV<0)
         LineSelectedV=NVariablesXYZ-1;
  if(VariablesXYZ[LineSelectedV].Used) {
        message=g_strdup_printf(_("Sorry\n %s \n is used in Geometry"),VariablesXYZ[LineSelectedV].Name);
	MessageGeom(message,_("Error"),TRUE);
        return;
        }

  Dialogue = gtk_dialog_new();
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_title(GTK_WINDOW(Dialogue),"Delete Variable");
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));
  gtk_window_set_modal (GTK_WINDOW (Dialogue), TRUE);

  add_child(WindowGeom,Dialogue,gtk_widget_destroy," Delete Variable ");
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  gtk_widget_realize(Dialogue);
  message = g_strdup_printf("\nAre you sure to delete the variable\n %s\n?",VariablesXYZ[LineSelectedV].Name);
  Label = create_label_with_pixmap(Dialogue,message," Question ");
  gtk_box_pack_start(GTK_BOX(vboxframe), Label,TRUE,TRUE,0);
 
  Bouton = create_button(Dialogue,"No");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);

  Bouton = create_button(Dialogue,"Yes");
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked",(GCallback)DelVariable, NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked", (GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
    

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
static void DialogueEditV()
{
  GtkWidget *Dialogue;
  GtkWidget *Bouton;
  GtkWidget *hbox;
  GtkWidget *frame, *vboxframe;
  gint Nc;
  gchar *tlabel[]={" Name : ", " Value : "};
  
  Nc=LineSelectedV;
  if(Nc<0 ) {
  	if(NVariablesXYZ<1)
   	MessageGeom(_("Create variable beforee \n"),_("Warning"),TRUE);
       else
   	MessageGeom(_("Please Select your variable \n"),_("Warning"),TRUE);
   return;
  }
  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),_("Edit Variable"));
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(Fenetre));
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));

  add_child(WindowGeom,Dialogue,gtk_widget_destroy,_(" Edit Variable "));
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

    frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);


  hbox=create_hbox_false(vboxframe);
  EntryV[0] = create_label_entry(hbox,tlabel[0],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  gtk_entry_set_text(GTK_ENTRY(EntryV[0]),VariablesXYZ[Nc].Name);
  if(VariablesXYZ[Nc].Used) 
	gtk_editable_set_editable((GtkEditable*) EntryV[0],FALSE);

  hbox=create_hbox_false(vboxframe);
  EntryV[1] = create_label_entry(hbox,tlabel[1],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  gtk_entry_set_text(GTK_ENTRY(EntryV[1]),VariablesXYZ[Nc].Value);

  gtk_widget_realize(Dialogue);
  Bouton = create_button(Dialogue,_("Cancel"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);

  Bouton = create_button(Dialogue,_("OK"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked",(GCallback)EditVariable,NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)destroy_dialogue,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
    

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
static void AddVariable(GtkWidget *w,gpointer Entree)
{
  gchar *texts[2];
  gchar *message;
  
  texts[0] = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryV[0])));
  texts[1] = g_strdup(gtk_entry_get_text(GTK_ENTRY(EntryV[1])));
  DestroyDialog = TRUE;
  if(TestVariablesXYZCreated(texts[0],-1))
  {
	MessageGeom(_("Sorry a other variable have any Name !\n"),_("Error"),TRUE);
  	DestroyDialog = FALSE;
      	return;
  } 
  if(!variable_name_valid(texts[0]))
  {
	show_forbidden_characters();
  	DestroyDialog = FALSE;
      	return;
  } 
  if ( strcmp(texts[0], "") && strcmp(texts[1], "") )
  	NVariablesXYZ++;
   else
     {
   	MessageGeom(_("Sorry a Entry text is void !\n"),_("Error"),TRUE);
  	DestroyDialog = FALSE;
      	return;
     }
  if(VariablesXYZ==NULL)
	  VariablesXYZ=g_malloc(sizeof(VariablesXYZDef));
  else
	VariablesXYZ=g_realloc(VariablesXYZ,NVariablesXYZ*sizeof(VariablesXYZDef));

  if(test(texts[1]) && !testpointeE(texts[1]) )
		texts[1]=g_strdup_printf("%s.0",texts[1]);
  VariablesXYZ[NVariablesXYZ-1].Name=g_strdup(texts[0]);
  VariablesXYZ[NVariablesXYZ-1].Value=g_strdup(texts[1]);
  VariablesXYZ[NVariablesXYZ-1].Used=FALSE;
  if(!test(texts[1]))
  {
	message=g_strdup_printf(_("Sorry %s is not a number \n"),texts[1]);
  	DestroyDialog = FALSE;
	MessageGeom(message,_("Error"),TRUE);
	NVariablesXYZ--;
	if(NVariablesXYZ>0)
	VariablesXYZ=g_realloc(VariablesXYZ,NVariablesXYZ*sizeof(VariablesXYZDef));
        else
	VariablesXYZ=NULL;
  }
  else
  	appendToList(listv, texts, 2);
}
/********************************************************************************/
static void DialogueAddV()
{
  GtkWidget *Dialogue;
  GtkWidget *Bouton;
  GtkWidget *hbox;
  GtkWidget *frame, *vboxframe;
  gchar *tlabel[]={" Name : ", " Value : "};
  
  Dialogue = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dialogue),"New Variable");
  gtk_window_set_position(GTK_WINDOW(Dialogue),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));

  add_child(WindowGeom,Dialogue,gtk_widget_destroy," New Variable ");
  g_signal_connect(G_OBJECT(Dialogue),"delete_event",(GCallback)delete_child,NULL);

    frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);

  g_object_ref (frame);
  g_object_set_data_full(G_OBJECT (Dialogue), "frame",
	  frame,(GDestroyNotify) g_object_unref);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dialogue)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);


  hbox=create_hbox_false(vboxframe);
  EntryV[0] = create_label_entry(hbox,tlabel[0],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));
  hbox=create_hbox_false(vboxframe);
  EntryV[1] = create_label_entry(hbox,tlabel[1],(gint)(ScreenHeight*labelWidth),(gint)(ScreenHeight*entryWidth));

  gtk_widget_realize(Dialogue);

  Bouton = create_button(Dialogue,_("Cancel"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)delete_child,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);

  Bouton = create_button(Dialogue,_("OK"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dialogue)->action_area), Bouton,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Bouton), "clicked",(GCallback)AddVariable,NULL);
  g_signal_connect_swapped(G_OBJECT(Bouton), "clicked",(GCallback)destroy_dialogue,GTK_OBJECT(Dialogue));
  GTK_WIDGET_SET_FLAGS(Bouton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Bouton);
    

  gtk_widget_show_all(Dialogue);
}
/********************************************************************************/
void create_variablesXYZ_list(GtkWidget *vbox,guint itype)
{
	GtkWidget *scr;
	guint i;
	guint Factor=7;
	guint widall=0;
	gchar *titres[2]={	" Name "," Value "};
	gint width[2]={10,10 };
	GtkUIManager *manager;

        GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
 
	if(itype!=GABEDIT_TYPEFILEGEOM_UNKNOWN) freeVariablesXYZ();

	LineSelectedV=-1;
	for(i=0;i<2;i++) widall+=width[i];
	widall=widall*Factor+50;
  
	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeight*0.4));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); 
	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 2);

	store = gtk_list_store_new (4, G_TYPE_STRING, G_TYPE_BOOLEAN,G_TYPE_STRING,G_TYPE_BOOLEAN);
        model = GTK_TREE_MODEL (store);

	listv = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (listv), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (listv), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (listv), TRUE);
	for (i=0;(gint)i<2;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, titres[i]);
		gtk_tree_view_column_set_min_width(column, width[i]*Factor);
		gtk_tree_view_column_set_reorderable(column, TRUE);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i+i, "editable",i+i+1,NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (listv), column);
		g_object_set_data(G_OBJECT(renderer),"NumColumn", GINT_TO_POINTER(i));
		g_signal_connect (renderer, "edited", G_CALLBACK (editedVariable), model);
	}
	gtk_container_add(GTK_CONTAINER(scr),listv);

	set_base_style(listv,58000,58000,58000);
	gtk_widget_show (listv);

	if(WindowGeom) manager = newMenuXYZVariables(WindowGeom);
	else manager = newMenuXYZVariables(Fenetre);


	if(GeomXYZ != NULL && VariablesXYZ != NULL) append_VariablesXYZ_in_list();
	g_signal_connect(G_OBJECT (listv), "button_press_event", G_CALLBACK(event_dispatcher), manager);
}
/********************************************************************************/
static void multi_by_factor(gdouble factor)
{
  gint i;

  for (i=0;(guint)i<NcentersXYZ;i++)
  {
     if(test(GeomXYZ[i].X))
  	GeomXYZ[i].X = g_strdup_printf("%f",factor*atof(GeomXYZ[i].X));
     if(test(GeomXYZ[i].Y))
 	 GeomXYZ[i].Y = g_strdup_printf("%f",factor*atof(GeomXYZ[i].Y));
     if(test(GeomXYZ[i].Z))
 	 GeomXYZ[i].Z = g_strdup_printf("%f",factor*atof(GeomXYZ[i].Z));
  }

  for (i=0;(guint)i<NVariablesXYZ;i++)
   VariablesXYZ[i].Value = g_strdup_printf("%f",factor*atof(VariablesXYZ[i].Value));

  clearList(list);
  append_list();

  clearList(listv);
  append_VariablesXYZ_in_list();

  if(GeomDrawingArea != NULL)
       rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();
}
/********************************************************************************/
static void MultiByA0()
{
 multi_by_factor(BOHR_TO_ANG);
}
/********************************************************************************/
static void DivideByA0()
{
 multi_by_factor(ANG_TO_BOHR);
}
/********************************************************************************/
static void OriginToCenter()
{
  guint i;
  guint j;
  gdouble X0=0.0;
  gdouble Y0=0.0;
  gdouble Z0=0.0;

  for (i=0;i<NcentersXYZ;i++)
  {
         if(!test(GeomXYZ[i].X))
                 X0 += get_value_variableXYZ(GeomXYZ[i].X);
         else
                 X0 += atof(GeomXYZ[i].X);
         if(!test(GeomXYZ[i].Y))
                 Y0 += get_value_variableXYZ(GeomXYZ[i].Y);
         else
                 Y0 += atof(GeomXYZ[i].Y);
         if(!test(GeomXYZ[i].Z))
                 Z0 += get_value_variableXYZ(GeomXYZ[i].Z);
         else
                 Z0 += atof(GeomXYZ[i].Z);
  }
  X0 /=NcentersXYZ;
  Y0 /=NcentersXYZ;
  Z0 /=NcentersXYZ;

  for (i=0;i<NcentersXYZ;i++)
  {
         if(!test(GeomXYZ[i].X))
         {
                 j = get_num_variableXYZ(GeomXYZ[i].X);
  		VariablesXYZ[j].Value = g_strdup_printf("%f",atof(VariablesXYZ[j].Value)-X0);
         }
         else
  		GeomXYZ[i].X = g_strdup_printf("%f",atof(GeomXYZ[i].X)-X0);
         if(!test(GeomXYZ[i].Y))
         {
                 j = get_num_variableXYZ(GeomXYZ[i].Y);
  		VariablesXYZ[j].Value = g_strdup_printf("%f",atof(VariablesXYZ[j].Value)-Y0);
         }
         else
  		GeomXYZ[i].Y = g_strdup_printf("%f",atof(GeomXYZ[i].Y)-Y0);
         if(!test(GeomXYZ[i].Z))
         {
                 j = get_num_variableXYZ(GeomXYZ[i].Z);
  		VariablesXYZ[j].Value = g_strdup_printf("%f",atof(VariablesXYZ[j].Value)-Z0);
         }
         else
  		GeomXYZ[i].Z = g_strdup_printf("%f",atof(GeomXYZ[i].Z)-Z0);
  }

  clearList(list);
  append_list();

  clearList(listv);
  append_VariablesXYZ_in_list();

  if(GeomDrawingArea != NULL)
       rafresh_drawing();
  if(iprogram == PROG_IS_GAUSS)
 	set_spin_of_electrons();
}
/********************************************************************************/
static void scale_cell(GtkWidget* fp,gpointer data)
{
	GtkWidget** entrys = (GtkWidget**)data;
	G_CONST_RETURN gchar* tentry[3];
	gint i,j;
	gdouble fact[3]={1.0,1.0,1.0};
	gint nTV = 0;

	for(i=0;i<3;i++) tentry[i] = gtk_entry_get_text(GTK_ENTRY(entrys[i]));
	for(i=0;i<3;i++) fact[i] = atof(tentry[i]);

	for(i=0;i<NcentersXYZ;i++) 
		if(!strcmp(GeomXYZ[i].Symb, "TV") || !strcmp(GeomXYZ[i].Symb, "Tv")) 
		{
			
			gdouble f = fact[nTV];
         		if(!test(GeomXYZ[i].X))
         		{
                 			j = get_num_variableXYZ(GeomXYZ[i].X);
  					VariablesXYZ[j].Value = g_strdup_printf("%f",atof(VariablesXYZ[j].Value)*f);
         		}
			else GeomXYZ[i].X = g_strdup_printf("%f",atof(GeomXYZ[i].X)*f);
         		if(!test(GeomXYZ[i].Y))
         		{
                 			j = get_num_variableXYZ(GeomXYZ[i].Y);
  					VariablesXYZ[j].Value = g_strdup_printf("%f",atof(VariablesXYZ[j].Value)*f);
         		}
			else GeomXYZ[i].Y = g_strdup_printf("%f",atof(GeomXYZ[i].Y)*f);
         		if(!test(GeomXYZ[i].Z))
         		{
                 			j = get_num_variableXYZ(GeomXYZ[i].Z);
  					VariablesXYZ[j].Value = g_strdup_printf("%f",atof(VariablesXYZ[j].Value)*f);
         		}
			else GeomXYZ[i].Z = g_strdup_printf("%f",atof(GeomXYZ[i].Z)*f);

			nTV++;
			if(nTV>=3) break;
		}

	for(i=0;i<3;i++) gtk_entry_set_text(GTK_ENTRY(entrys[i]),"1.0");
  	if(GeomDrawingArea != NULL) rafresh_drawing();
}
/*********************************************************************/
GtkWidget* scale_cell_dialog ()
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox;
  GtkWidget *button;
  GtkWidget *label;
  GtkStyle *style;
  static GtkWidget* entrys[3];
  static GdkColor color;
  gchar* tlabel[3]={"a : ","b : ","c : "};
  gint i;

  /* principal Window */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),_("Scal box"));
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)gtk_widget_destroy,NULL);

  vboxall = create_vbox(fp);
  frame = gtk_frame_new (_("Scaling factors"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  for(i=0;i<3;i++)
  {
	hbox = create_hbox(vboxframe);
	label = gtk_label_new (tlabel[i]);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, FALSE, 0);

	entrys[i] = gtk_entry_new ();
	gtk_widget_show (entrys[i]);
	gtk_box_pack_start (GTK_BOX (hbox), entrys[i], FALSE, TRUE, 0);

	gtk_entry_set_text(GTK_ENTRY(entrys[i]),"1.0");
  }
  hbox = create_hbox(vboxall);

  button = create_button(Fenetre,_("Close"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(fp));

  button = create_button(Fenetre,_("Apply"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(scale_cell),(gpointer)entrys);
  gtk_widget_show (button);

  button = create_button(Fenetre,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(scale_cell),(gpointer)entrys);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(fp));
  gtk_widget_show (button);
  gtk_widget_show_all(fp);
  return fp;
}
