/* MopacRem.c */
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
#include <ctype.h>
#include <math.h>
#include <string.h>

#include "../../Config.h"
#include "../Common/Global.h"
#include "../Mopac/MopacTypes.h"
#include "../Mopac/MopacGlobal.h"
#include "../Mopac/MopacMolecule.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"

static GtkWidget* comboethod = NULL;
/*************************************************************************************************************/
static gchar* listJobView[] = {
        "Single Point Energy", 
	"Equilibrium structure search", 
	"Frequencies",
	"Transition structure search", 
        "Theromochemical Calculations",
        "Molecular Orbitals",
        "Atomic charges from ElectroStatic Potential",
        "Reaction path",
};
static gchar* listJobReal[] = {
        "1SCF ", 
	"XYZ ", 
        "FORCE ", 
	"TS ", 
	"THERMO ROT=1 ",
	"GRAPH VECTORS ",
	"1SCF ESP ",
	"RP",
};
static guint numberOfJobs = G_N_ELEMENTS (listJobView);
static gchar selectedJob[BSIZE]="1SCF";
/*************************************************************************************************************/
static gchar* listHamiltonianMethodsView[] = 
{ 
	"PM7", 
	"PM6", 
	"PM6-DH+", 
	"PM6-DH2", 
	"RM1", 
	"PM3", 
	"AM1", 
	"MNDO", 
};
static gchar* listHamiltonianMethodsReal[] = 
{ 
	"PM7", 
	"PM6", 
	"PM6-DH+", 
	"PM6-DH2", 
	"RM1", 
	"PM3", 
	"AM1", 
	"MNDO", 
};
static guint numberOfHamiltonianMethods = G_N_ELEMENTS (listHamiltonianMethodsView);
static gchar selectedHamiltonian[BSIZE]="PM7";
/*************************************************************************************************************/
static gchar* listSolventsView[] = 
{ 
	"Nothing", 
	"Water [H2O]",
	"AceticAcid [CH3COOH]",
	"Acetone [CH3COCH3]",
	"Acetonitrile [CH3CN]",
	"Ammonia [NH3]",
	"Aniline [C6H5NH2]",
	"Benzene [C6H6]",
	"BenzylAlcohol [C6H5CH2OH]",
	"Bromoform [CHBr3]",
	"Butanol [C4H9OH]",
	"isoButanol [(CH3)2CHCH2OH]",
	"tertButanol [(CH3)3COH]",
	"CarbonDisulfide [CS2]",
	"CarbonTetrachloride [CCl4]",
	"Chloroform [CHCl3]",
	"Cyclohexane [C6H12]",
	"Cyclohexanone [C6H10O]",
	"Dichlorobenzene [C6H4Cl2]",
	"DiethylEther [(CH3CH2)2O]",
	"Dioxane [C4H8O2]",
	"DMFA [(CH3)2NCHO]",
	"DMSO [(CH3)2SO]",
	"Ethanol [CH3CH2OH]",
	"EthylAcetate [CH3COOCH2CH3]",
	"Dichloroethane [ClCH2CH2Cl]",
	"EthyleneGlycol [HOCH2CH2OH]",
	"Formamide [HCONH2]",
	"FormicAcid [HCOOH]",
	"Glycerol [C3H8O3]",
	"HexamethylPhosphoramide [C6H18N3OP]",
	"Hexane [C6H14]",
	"Hydrazine [N2H4]",
	"Methanol [CH3OH]",
	"MethylEthylKetone [CH3CH2COCH3]",
	"Dichloromethane [CH2Cl2]",
	"Methylformamide [HCONHCH3]",
	"Methypyrrolidinone [C5H9NO]",
	"Nitrobenzene [C6H5NO2]",
	"Nitrogen [N2]",
	"Nitromethane [CH3NO2]",
	"PhosphorylChloride [POCl3]",
	"IsoPropanol [(CH3)2CHOH]",
	"Pyridine [C5H5N]",
	"Sulfolane [C4H8SO2]",
	"Tetrahydrofuran [C4H8O]",
	"Toluene [C6H5CH3]",
	"Triethylamine [(CH3CH2)3N]",
	"TrifluoroaceticAcid [CF3COOH]",
};

static gchar* listSolventsReal[] = 
{ 
	" ", 
	"EPS=78.39 RSOLV=1.3",
	"EPS=6.19 RSOLV=2.83",
	"EPS=20.7 RSOLV=3.08",
	"EPS=37.5 RSOLV=2.76",
	"EPS=16.9 RSOLV=2.24",
	"EPS=6.8  RSOLV=3.31",
	"EPS=2.3  RSOLV=3.28",
	"EPS=13.1 RSOLV=3.45",
	"EPS=4.3  RSOLV=3.26",
	"EPS=17.5 RSOLV=3.31",
	"EPS=17.9 RSOLV=3.33",
	"EPS=12.4 RSOLV=3.35",
	"EPS=2.6 RSOLV=2.88",
	"EPS=2.2 RSOLV=3.37",
	"EPS=4.8 RSOLV=3.17",
	"EPS=2.0 RSOLV=3.5",
	"EPS=15.0 RSOLV=3.46",
	"EPS=9.8 RSOLV=3.54",
	"EPS=4.34 RSOLV=3.46",
	"EPS=2.2 RSOLV=3.24",
	"EPS=37.0 RSOLV=3.13",
	"EPS=46.7 RSOLV=3.04",
	"EPS=24.55 RSOLV=2.85",
	"EPS=6.02 RSOLV=3.39",
	"EPS=10.66 RSOLV=3.15",
	"EPS=37.7 RSOLV=2.81",
	"EPS=109.5 RSOLV=2.51",
	"EPS=58.5 RSOLV=2.47",
	"EPS=42.5 RSOLV=3.07",
	"EPS=43.3 RSOLV=4.1",
	"EPS=1.88 RSOLV=3.74",
	"EPS=51.7 RSOLV=2.33",
	"EPS=32.6 RSOLV=2.53",
	"EPS=18.5 RSOLV=3.3",
	"EPS=8.9 RSOLV=2.94",
	"EPS=182.4 RSOLV=2.86",
	"EPS=33.0 RSOLV=3.36",
	"EPS=34.8 RSOLV=3.44",
	"EPS=1.45 RSOLV=2.36",
	"EPS=35.87 RSOLV=2.77",
	"EPS=13.9 RSOLV=3.33",
	"EPS=19.9 RSOLV=3.12",
	"EPS=12.4 RSOLV=3.18",
	"EPS=43.3 RSOLV=3.35",
	"EPS=7.58 RSOLV=3.18",
	"EPS=2.38 RSOLV=3.48",
	"EPS=2.44 RSOLV=3.81",
	"EPS=42.1 RSOLV=3.12",
};
static guint numberOfSolvents = G_N_ELEMENTS (listSolventsView);
static gchar selectedSolvent[BSIZE]="Nothing";
/*************************************************************************************************************/
gchar* getSeletedJobType()
{
	return selectedJob;
}
/*************************************************************************************************************/
static gchar* calculWord(gchar* view)
{
	gint i;
	for(i=0;i<numberOfJobs;i++)
	{
		if(strcmp(view,listJobView[i])==0)return listJobReal[i];
	}
	return NULL;
}
/*************************************************************************************************************/
static void putMopacJobTypeInfoInTextEditor()
{
	if(!strcmp(selectedJob,"RP")) putMopacReactionPathInTextEditor();
	else gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedJob,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
}
/*************************************************************************************************************/
static void putMopacHamiltonianInfoInTextEditor()
{
	if(strcmp(selectedHamiltonian,"NONE")==0)return;

       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedHamiltonian,-1);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
}
/*************************************************************************************************************/
static void putMopacSolventInfoInTextEditor()
{
	if(strcmp(selectedSolvent,"Nothing")==0)return;

       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedSolvent,-1);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
}
/*************************************************************************************************************/
void putMopacRemInfoInTextEditor()
{
	putMopacHamiltonianInfoInTextEditor();
	putMopacJobTypeInfoInTextEditor();
	putMopacSolventInfoInTextEditor();
}
/************************************************************************************************************/
static void traitementJobType (GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	gchar* res = NULL;
	GtkWidget* hboxRP = NULL;
	
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	else return;
	res = calculWord(data);
	if(res) sprintf(selectedJob,"%s",res);
	else  sprintf(selectedJob,"SP");
	hboxRP = g_object_get_data(G_OBJECT (combobox), "HboxReactionPath");
	if(hboxRP && GTK_IS_WIDGET(hboxRP)) gtk_widget_set_sensitive(hboxRP, strstr(data,"Reaction path")!=NULL);

}
/********************************************************************************************************/
static GtkWidget *create_list_jobtype()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfJobs;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listJobView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementJobType), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
static void traitementHamiltonian (GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	gchar* res = NULL;
	gint i;
	/* gchar* s;*/
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	else return;
	for(i=0;i<numberOfHamiltonianMethods;i++)
	{
		if(strcmp((gchar*)data,listHamiltonianMethodsView[i])==0) res = listHamiltonianMethodsReal[i];
	}
	if(res) sprintf(selectedHamiltonian,"%s",res);

}
/********************************************************************************************************/
static GtkWidget *create_list_hamiltonian_methods()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfHamiltonianMethods;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listHamiltonianMethodsView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementHamiltonian), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
static void traitementSolventType (GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	gchar* res = NULL;
	gint i;
	
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	else return;
	for(i=0;i<numberOfSolvents;i++)
	{
		if(strcmp((gchar*)data,listSolventsView[i])==0) res = listSolventsReal[i];
	}
	if(res) sprintf(selectedSolvent,"%s",res);
	else  sprintf(selectedSolvent,"Nothing");

}
/********************************************************************************************************/
static GtkWidget *create_list_solventtype()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfSolvents;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listSolventsView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementSolventType), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/*************************************************************************************************************/
void createMopacRemFrame(GtkWidget *win, GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* combo = NULL;
	gint l=0;
	gint c=0;
	gint ncases=1;
	GtkWidget *table = gtk_table_new(8,3,FALSE);

	comboethod = NULL;

	frame = gtk_frame_new (_("Job Specification:"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);
	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	/*------------------ Job Type -----------------------------------------*/
	l=0; 
	c = 0; ncases=1;
	add_label_table(table,_("Job Type"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_jobtype();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	g_object_set_data(G_OBJECT (win), "ComboJobType", combo);
	/*------------------ Hamiltonian Type -----------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("Hamiltonian"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_hamiltonian_methods();
	comboethod = combo;
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("Solvent"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_solventtype();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
}
