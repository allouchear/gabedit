/* DeMonKeywords.c */
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
#include "../DeMon/DeMonTypes.h"
#include "../DeMon/DeMonGlobal.h"
#include "../DeMon/DeMonMolecule.h"
#include "../DeMon/DeMonBasis.h"
#include "../DeMon/DeMonGuess.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"

static GtkWidget* comboSCF = NULL;
static GtkWidget* combovxc = NULL;
static GtkWidget* comboMethod = NULL;
static GtkWidget* comboTypeMethod = NULL;
static GtkWidget* comboExcited = NULL;
static GtkWidget* combotight = NULL;
static GtkWidget* combogrid = NULL;
static GtkWidget* combodiis = NULL;
static GtkWidget* entryShift = NULL;
static GtkWidget* entryMixing = NULL;
static GtkWidget* entryscfmaxit = NULL;
static void setDeMonMethods();
/*************************************************************************************************************/
static gchar* listJobView[] = {
        "Single Point Energy", 
	"Geometry optimization", 
	"Frequencies",
	"Frequencies + Thermo",
	"Transition state",
	"Equilibrium structure search + Frequencies", 
	"Equilibrium structure search + Frequencies + Thermo",
	"Transition state + Frequencies", 
};
static gchar* listJobReal[] = {
        "SP", 
	"Optimization\n", 
        "Frequencies\n", 
	"Frequencies\nThermo\n",
	"OptTS\n", 
        "Optimization\nFrequencies\n", 
	"Optimization\nFrequencies\nThermo\n",
        "OptTS NumFreq\n", 
};
static guint numberOfJobs = G_N_ELEMENTS (listJobView);
static gchar selectedJob[BSIZE]="SP";
/*************************************************************************************************************/
static gchar* listSCFView[] = { "default","unrestricted", "restricted"};
static gchar* listSCFReal[] = { "DEF", "TRUE", "FALSE"};
static guint numberOfSCF = G_N_ELEMENTS (listSCFView);
static gchar selectedSCF[BSIZE]="DEF";
/*************************************************************************************************************/
static gchar* listtightView[] = { 
	"Tighten : Default",
	"No Tighten : The SCF convergence criteria will not be adjusted during an optimization. ",
};
static gchar* listtightReal[] = { 
	"NONE",
	"NOTIGHTEN",	
};
static guint numberOftight = G_N_ELEMENTS (listtightView);
static gchar selectedtight[BSIZE]="NONE";
/*************************************************************************************************************/
static gchar* listvxcView[] = { 
	"AUXIS : Default. The auxiliary function density is used for the calculation of the exchange-correlation energy and matrix elements of the potential.", 
	"BASIS : The orbital density is used for the calculation of the exchange-correlation energy and matrix elements of the potential.",
};
static gchar* listvxcReal[] = { 
	"AUXIS", 
	"BASIS",
};
static guint numberOfvxc = G_N_ELEMENTS (listvxcView);
static gchar selectedvxc[BSIZE]="AUXIS";
/*************************************************************************************************************/
static gchar* listdiisView[] = { 
	"ON : Default. The DIIS procedure is switched on.",
	"OFF : The DIIS procedure is switched off. ", 
};
static gchar* listdiisReal[] = { 
	"NONE", 
	"DIIS OFF \n",
};
static guint numberOfdiis = G_N_ELEMENTS (listdiisView);
static gchar selecteddiis[BSIZE]="NONE";
/*************************************************************************************************************/
static gchar* listTypeMethodsView[] = 
{ 
	"Local and gradient corrected functionalsDeMon", 
	"Hybrid functionalsDeMon",
	"Meta-GGA and hybrid meta-GGA's",
};
static gchar* listTypeMethodsReal[] = 
{ 
	"DFT", 
	"Hybrid",
	"Meta-GGA",
};
static guint numberOfTypeMethods = G_N_ELEMENTS (listTypeMethodsView);
static gchar selectedTypeMethod[BSIZE]="HF";
/*************************************************************************************************************/
static gchar* listHFMethodsView[] = 
{ 
	"HF", 
};
static gchar* listHFMethodsReal[] = 
{ 
	"HF", 
};
static guint numberOfHFMethods = G_N_ELEMENTS (listHFMethodsView);
/*************************************************************************************************************/

static gchar* listDFTMethodsView[] = 
{ 
	"VWN     Dirac exchange with local VWN correlation.",
	"PZ81    Dirac exchange with local PZ81 correlation.",
	"PW92    Dirac exchange with local PW92 correlation.",
	"PW86    PW86 GGA exchange with P86 GGA correlation.",
	"BLYP    B88 GGA exchange with LYP GGA correlation.",
	"OLYP    HC01 GGA exchange with LYP GGA correlation.",
	"PW91    PW91 GGA exchange and correlation.",
	"PW91SSF PW91 with full spin scaling function.",
	"PBE PBE GGA exchange and correlation.",
	"PBESSF  PBE with full spin scaling function.",
	"PBESOL  PBE GGA exchange and correlation for solids.",
	"XALPHA  X_alpha calculation."
};
static gchar* listDFTMethodsReal[] = 
{ 
	"VWN\n",
	"PZ81\n",
	"PW92\n",
	"PW86\n",
	"BLYP\n",
	"OLYP\n",
	"PW91\n",
	"PW91SSF\n",
	"PBE\n",
	"PBESSF\n",
	"PBESOL\n",
	"XALPHA\n",
};
static guint numberOfDFTMethods = G_N_ELEMENTS (listDFTMethodsView);
/*************************************************************************************************************/
static gchar* listHybridMethodsView[] = 
{ 
	"B3LYP  The popular B3LYP functional (20% HF exchange)",
        "M06L   Minnesota Functional ver.6 (local functional, 0% HF exchange) (BASIS ONLY)",
        "M06    Minnesota Functional ver.6 (Global hybrid functional, 27% HF exchange) (BASIS ONLY)",
        "M062X  Minnesota Functional ver.6 (Global hybrid functional, 54% HF exchange) (BASIS ONLY)",
        "M06HF  Minnesota Functional ver.6 (Global hybrid functional, 100% HF exchange) (BASIS ONLY)",
        "M11L   Minnesota Functional ver.11 (local functional, 0% HF exchange) (BASIS ONLY)",
        "MN12   Minnesota Functional ver.12 (local functional, 0% HF exchange) (BASIS ONLY)",
};
static gchar* listHybridMethodsReal[] = 
{ 
	"B3LYP\n",
        "M06L\n",
        "M06\n",
        "M062X\n",
        "M06HF\n",
        "M11L\n",
        "MN12\n",

};
static guint numberOfHybridMethods = G_N_ELEMENTS (listHybridMethodsView);
/*************************************************************************************************************/
static gchar* listMetaGGAMethodsView[] = 
{ 
	"PKZB 	PKZB meta-GGA exchange and correlation. (BASIS ONLY)",
	"TPSS 	TPSS meta-GGA exchange and correlation. (BASIS ONLY)",
};
static gchar* listMetaGGAMethodsReal[] = 
{ 
	"PKZB\n",
	"TPSS\n",
};
static guint numberOfMetaGGAMethods = G_N_ELEMENTS (listMetaGGAMethodsView);
/*************************************************************************************************************/
static gchar* listDHybridMethodsView[] = 
{ 
	"B2PLYP The new mixture of MP2 and DFT from Grimme",
	"RI-B2PLYP B2PLYP with RI applied to the MP2 part",
};
static gchar* listDHybridMethodsReal[] = 
{ 
	"B2PLYP",
	"RI-B2PLYP",
};
static guint numberOfDHybridMethods = G_N_ELEMENTS (listDHybridMethodsView);
/*************************************************************************************************************/
static gchar* listMP2MethodsView[] = 
{ 
	"MP2",
	"RI-MP2",
};
static gchar* listMP2MethodsReal[] = 
{ 
	"MP2",
	"RI-MP2",
};
static guint numberOfMP2Methods = G_N_ELEMENTS (listMP2MethodsView);
/*************************************************************************************************************/
static gchar* listHighLevelSingleRefMethodsView[] = 
{ 
	"CCSD  Coupled cluster singles and doubles",
	"CCSD(T) Same with perturbative triples correction",
};
static gchar* listHighLevelSingleRefMethodsReal[] = 
{ 
	"CCSD",
	"CCSD(T)",
};
static guint numberOfHighLevelSingleRefMethods = G_N_ELEMENTS (listHighLevelSingleRefMethodsView);
/*************************************************************************************************************/
static gchar* listSemiEmpMethodsView[] = 
{ 
	"AM1",
	"PM3",
};
static gchar* listSemiEmpMethodsReal[] = 
{ 
	"AM1",
	"PM3",
};
static guint numberOfSemiEmpMethods = G_N_ELEMENTS (listSemiEmpMethodsView);
/*************************************************************************************************************/
static gchar selectedMethod[BSIZE]="HF";
static gchar** listMethodsView = listHybridMethodsView;
static gchar** listMethodsReal = listHybridMethodsReal;
static guint numberOfMethods = G_N_ELEMENTS (listHybridMethodsView);
/*************************************************************************************************************/
static gchar* listScfConvergenceView[] = { 
	"Default", 
	"LooseSCF : loose SCF convergence",
	"TightSCF : tight SCF convergence",
	"VeryTightSCF :very tight SCF convergence",
};
static gchar* listScfConvergenceReal[] = { 
	"NONE", 
	"TOL=1.E-6",
	"TOL=1.E-8",
	"TOL=1.E-10",

};
static guint numberOfScfConvergence = G_N_ELEMENTS (listScfConvergenceView);
static gchar selectedScfConvergence[BSIZE]="NONE";
/*************************************************************************************************************/
static gchar* listgridView[] = { 
	"Adaptive : Default", 
	"Medium : Medium grid accuracy is requested",
	"Coarse : Coarse grid accuracy is requested.",
	"Fine   : Fine grid accuracy is requested.",
};
static gchar* listgridReal[] = { 
	"NONE", 
	"GRID FIXED MEDIUM\n",
	"GRID FIXED COARSE\n",
	"GRID FIXED FINE\n",

};
static guint numberOfgrid = G_N_ELEMENTS (listgridView);
static gchar selectedgrid[BSIZE]="NONE";
/*************************************************************************************************************/
static gchar* listNothingExcitedView[] = 
{ 
	"Nothing", 
};
static gchar* listNothingExcitedReal[] = 
{ 
	"NONE", 
};
static guint numberOfNothingExcited = G_N_ELEMENTS (listNothingExcitedView);
/*************************************************************************************************************/
static gchar* listSemiEmpExcitedView[] = 
{ 
	"Nothing",
	"CIS",
};
static gchar* listSemiEmpExcitedReal[] = 
{ 
	"NONE", 
	"CIS",
};
static guint numberOfSemiEmpExcited = G_N_ELEMENTS (listSemiEmpExcitedView);
/*************************************************************************************************************/
static gchar* listHFExcitedView[] = 
{ 
	"Nothing",
	"CIS",
	"CIS(D)",
};
static gchar* listHFExcitedReal[] = 
{ 
	"NONE", 
	"CIS",
	"CIS(D)",
};
static guint numberOfHFExcited = G_N_ELEMENTS (listHFExcitedView);
/*************************************************************************************************************/
static gchar* listDFTExcitedView[] = 
{ 
	"Nothing",
	"TD-DFT",
};
static gchar* listDFTExcitedReal[] = 
{ 
	"NONE", 
	"TD-DFT",
};
static guint numberOfDFTExcited = G_N_ELEMENTS (listDFTExcitedView);
/*************************************************************************************************************/
static gchar selectedExcited[BSIZE]="NONE";
static gchar** listExcitedView = listNothingExcitedView;
static gchar** listExcitedReal = listNothingExcitedReal;
static guint numberOfExcited = G_N_ELEMENTS (listNothingExcitedView);
/*************************************************************************************************************/
static void putDeMonExcitedInfoInTextEditor()
{
	gchar buffer[BSIZE];
	if(!strcmp(selectedExcited,"NONE"))return;
	sprintf(buffer,"Excitation ");
	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &demonColorFore.keyWord, &demonColorBack.keyWord, buffer, -1);

	/*
	sprintf(buffer,"     nroots %d # the number of excited states to be calculated.\n",8);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	sprintf(buffer,"     maxdim %d # the maximum dimension of the expansion space in the Davidson procedure.\n",30);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	*/
	sprintf(buffer,"     # TDA TammDanCoff true Tamm-Dancoff approximation for non-hybride\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
}
/*************************************************************************************************************/
static void setDeMonExcited()
{
	GtkTreeIter iter;
	GtkTreeIter iter0;
	gint i;

	if(strcmp(selectedTypeMethod,"HF")==0)
	{
		listExcitedView = listHFExcitedView;
		listExcitedReal = listHFExcitedReal;
		numberOfExcited = numberOfHFExcited;
	}
	else
	if(strcmp(selectedTypeMethod,"DFT")==0)
	{
		listExcitedView = listDFTExcitedView;
		listExcitedReal = listDFTExcitedReal;
		numberOfExcited = numberOfDFTExcited;
	}
	else
	if(strcmp(selectedTypeMethod,"Hybrid")==0)
	{
		listExcitedView = listDFTExcitedView;
		listExcitedReal = listDFTExcitedReal;
		numberOfExcited = numberOfDFTExcited;
	}
	else
	if(strcmp(selectedTypeMethod,"Meta-GGA")==0)
	{
		listExcitedView = listDFTExcitedView;
		listExcitedReal = listDFTExcitedReal;
		numberOfExcited = numberOfDFTExcited;
	}
	else
	if(strcmp(selectedTypeMethod,"D-Hybrid")==0)
	{
		listExcitedView = listDFTExcitedView;
		listExcitedReal = listDFTExcitedReal;
		numberOfExcited = numberOfDFTExcited;
	}
	else
	if(strcmp(selectedTypeMethod,"SemiEmp")==0 && strstr(selectedMethod,"/S"))
	{
		listExcitedView = listSemiEmpExcitedView;
		listExcitedReal = listSemiEmpExcitedReal;
		numberOfExcited = numberOfSemiEmpExcited;
	}
	else
	{
		listExcitedView = listNothingExcitedView;
		listExcitedReal = listNothingExcitedReal;
		numberOfExcited = numberOfNothingExcited;
	}
	
	if (comboExcited && gtk_combo_box_get_active_iter (GTK_COMBO_BOX(comboExcited), &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(comboExcited));
        	GtkTreeStore *store = GTK_TREE_STORE(model);
		gtk_tree_store_clear(store);

		for(i=0;i<numberOfExcited;i++)
		{
        		gtk_tree_store_append (store, &iter, NULL);
			if(i==0) iter0 = iter;
        		gtk_tree_store_set (store, &iter, 0, listExcitedView[i], -1);
		}
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX (comboExcited), &iter0);
	}

}
/************************************************************************************************************/
static void traitementExcited (GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	gchar* res = NULL;
	gint i;
	gboolean ri = FALSE;
	GtkWidget* wid;

	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	else return;
	for(i=0;i<numberOfExcited;i++)
	{
		if(strcmp((gchar*)data,listExcitedView[i])==0) res = listExcitedReal[i];
	}
	if(res) sprintf(selectedExcited,"%s",res);
	ri = !strcmp(selectedExcited,"CIS(D)");
	if(strstr(selectedExcited,"TD") && !strcmp(selectedTypeMethod,"D-Hybrid"))
	{
		ri = TRUE;
	}
	if(!strstr(selectedExcited,"NONE"))
	{
		wid  = g_object_get_data(G_OBJECT (combobox), "LabelAuxBasis1");
		if(wid) gtk_widget_set_sensitive(wid, ri);
		wid  = g_object_get_data(G_OBJECT (combobox), "LabelAuxBasis2");
		if(wid) gtk_widget_set_sensitive(wid, ri);
		wid  = g_object_get_data(G_OBJECT (combobox), "ComboAuxBasis");
		if(wid) gtk_widget_set_sensitive(wid, ri);
	}
}
/********************************************************************************************************/
static GtkWidget *create_list_excited()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);


	for(i=0;i<numberOfExcited;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listExcitedView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementExcited), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/********************************************************************************************************/
static void treatmenttight (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOftight;i++)
	{
		if(strcmp((gchar*)data,listtightView[i])==0) res = listtightReal[i];
	}
	if(res) sprintf(selectedtight,"%s",res);
	else  sprintf(selectedtight,"NONE");

}
/********************************************************************************************************/
static void traitementScfConvergence (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfScfConvergence;i++)
	{
		if(strcmp((gchar*)data,listScfConvergenceView[i])==0) res = listScfConvergenceReal[i];
	}
	if(res) sprintf(selectedScfConvergence,"%s",res);
	else  sprintf(selectedScfConvergence,"Default");

}
/********************************************************************************************************/
static GtkWidget *create_list_tight()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOftight;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listtightView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(treatmenttight), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
GtkWidget* addDeMontightToTable(GtkWidget *table, gint i)
{
	GtkWidget* combotight = NULL;


	add_label_table(table,_("SCF Tighten"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	combotight  = create_list_tight();

	return combotight;
}
/************************************************************************************************************/
static GtkWidget *create_list_scfconvergence()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfScfConvergence;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listScfConvergenceView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementScfConvergence), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
GtkWidget* addDeMonScfConvergenceToTable(GtkWidget *table, gint i)
{
	GtkWidget* comboScfConvergence = NULL;


	add_label_table(table,_("SCF convergence"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	comboScfConvergence  = create_list_scfconvergence();

	return comboScfConvergence;
}
/*************************************************************************************************************/
static void treatmentdiis (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfdiis;i++)
	{
		if(strcmp((gchar*)data,listdiisView[i])==0) res = listdiisReal[i];
	}
	if(res) sprintf(selecteddiis,"%s",res);
	else  sprintf(selecteddiis,"Default");

}
/********************************************************************************************************/
static GtkWidget *create_list_diis()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfdiis;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listdiisView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(treatmentdiis), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
GtkWidget* addDeMondiisToTable(GtkWidget *table, gint i)
{
	GtkWidget* combodiis = NULL;


	add_label_table(table,_("DIIS"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	combodiis  = create_list_diis();

	return combodiis;
}
/*************************************************************************************************************/
static void treatmentgrid (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfgrid;i++)
	{
		if(strcmp((gchar*)data,listgridView[i])==0) res = listgridReal[i];
	}
	if(res) sprintf(selectedgrid,"%s",res);
	else  sprintf(selectedgrid,"Default");

}
/********************************************************************************************************/
static GtkWidget *create_list_grid()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfgrid;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listgridView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(treatmentgrid), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
GtkWidget* addDeMongridToTable(GtkWidget *table, gint i)
{
	GtkWidget* combogrid = NULL;


	add_label_table(table,_("Grid"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	combogrid  = create_list_grid();

	return combogrid;
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
static void putDeMonJobTypeInfoInTextEditor()
{
	if(!strcmp(selectedJob,"SP"))return;
	else
	{
		gchar buffer[BSIZE];
		sprintf(buffer,"%s",selectedJob);
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &demonColorFore.keyWord, &demonColorBack.keyWord, buffer, -1);
	}
}
/*************************************************************************************************************/
static void putDeMonSCFKeywordsInfoInTextEditor()
{
	G_CONST_RETURN gchar *entrytext1;
	gint maxit=100;
	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &demonColorFore.keyWord, &demonColorBack.keyWord, "SCFTYP ", -1);

	if(strcmp(selectedSCF,"DEF"))
	{
		//if(!strcmp(selectedSCF,"TRUE"))
		if(strcmp(selectedSCF,"TRUE"))
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "R",-1);
		else
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "U",-1);
		if(
			!strcmp(selectedTypeMethod,"DFT")
			|| !strcmp(selectedTypeMethod,"Hybrid")
			|| !strcmp(selectedTypeMethod,"Meta-GGA")
			|| !strcmp(selectedTypeMethod,"D-Hybrid")
		)
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "KS ",-1);
		else
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "HF ",-1);

	}
	if( strcmp(selectedScfConvergence,"NONE")!=0 )
	{
		gchar buffer[BSIZE];
		sprintf(buffer,"%s ",selectedScfConvergence);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}
	if( strcmp(selectedtight,"NONE")!=0 )
	{
		gchar buffer[BSIZE];
		sprintf(buffer,"%s ",selectedtight);
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}
	entrytext1 = gtk_entry_get_text(GTK_ENTRY(entryscfmaxit));
	maxit=atoi(entrytext1);
	if (maxit==0)maxit=100;
	gchar buffer[BSIZE];
	sprintf(buffer,"MAX=%d ",maxit);
	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1); 
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
}
/************************************************************************************************************/
static void putDeMonDiisInfoInTextEditor()
{
	if( strcmp(selecteddiis,"NONE")==0 ) return;
	{
		gchar buffer[BSIZE];
		sprintf(buffer,"%s",selecteddiis);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}
}
/************************************************************************************************************/
static void putDeMonGridInfoInTextEditor()
{
	if( strcmp(selectedgrid,"NONE")==0 ) return;
	{
		gchar buffer[BSIZE];
		sprintf(buffer,"%s",selectedgrid);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}
}
/************************************************************************************************************/
static void putDeMonMethodInfoInTextEditor()
{
	gchar buffer[BSIZE];
	sprintf(buffer,"%s",selectedMethod);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &demonColorFore.keyWord, &demonColorBack.keyWord, buffer,-1);
}
/************************************************************************************************************/
static void putDeMonvxcInfoInTextEditor()
{
	gchar buffer[BSIZE];
	sprintf(buffer,"%s",selectedvxc);
	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &demonColorFore.keyWord, &demonColorBack.keyWord, "VXCTYP ", -1);
	if( strcmp(selectedvxc,"AUXIS")==0 )
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "AUXIS ",-1);
		gchar buffer[BSIZE];
		sprintf(buffer,"%s",selectedMethod);
       		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &demonColorFore.keyWord, &demonColorBack.keyWord, buffer,-1);
	}
	else
	{
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "BASIS ",-1);
		gchar buffer[BSIZE];
                sprintf(buffer,"%s",selectedMethod);
                gabedit_text_insert (GABEDIT_TEXT(text), NULL, &demonColorFore.keyWord, &demonColorBack.keyWord, buffer,-1);
	}
}
/*************************************************************************************************************/
void putDeMonShiftInfoInTextEditor()
{
	G_CONST_RETURN gchar *entrytext;
	gfloat shift=0.0;
	entrytext = gtk_entry_get_text(GTK_ENTRY(entryShift));
	shift=atof(entrytext);
	if (shift!=0)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,"SHIFT %f \n",shift);
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}
}
/*************************************************************************************************************/
void putDeMonMixingInfoInTextEditor()
{
        G_CONST_RETURN gchar *entrytext;
        gfloat mixing=-0.3;
        entrytext = gtk_entry_get_text(GTK_ENTRY(entryMixing));
        mixing=atof(entrytext);
        if (mixing!=0)
        {
                gchar buffer[BSIZE];
                sprintf(buffer,"MIXING %f \n",mixing);
                gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
        }
}
/*************************************************************************************************************/
void putDeMonKeywordsInfoInTextEditor()
{
        //gabedit_text_insert (GABEDIT_TEXT(text), NULL, &demonColorFore.keyWord, &demonColorBack.keyWord, "! ",-1);
	putDeMonJobTypeInfoInTextEditor();
	putDeMonSCFKeywordsInfoInTextEditor();
        putDeMonvxcInfoInTextEditor();
	putDeMonDiisInfoInTextEditor();
	putDeMonShiftInfoInTextEditor();
        putDeMonMixingInfoInTextEditor();
	putDeMonGridInfoInTextEditor();
	putDeMonGuessInfoInTextEditor();
	putDeMonBasisInfoInTextEditor();
	putDeMonAuxBasisInTextEditor();
	putDeMonExcitedInfoInTextEditor();
}
/************************************************************************************************************/
static void traitementJobType (GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	gchar* res = NULL;
	
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	else return;
	res = calculWord(data);
	if(res) sprintf(selectedJob,"%s",res);
	else  sprintf(selectedJob,"SP");

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
void setDeMonSCFMethod(gboolean okRHF)
{
	GtkTreeIter iter;
	GtkTreeIter iter0;
	gint i;
	/* gchar* s;*/
	if (gtk_combo_box_get_active_iter (GTK_COMBO_BOX(comboSCF), &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(comboSCF));
        	GtkTreeStore *store = GTK_TREE_STORE(model);
		gint k = 0;
		gtk_tree_store_clear(store);
		for(i=0;i<numberOfSCF;i++)
		{
			if(!okRHF)
			if(strcmp(listSCFView[i],"restricted")==0)continue;
        		gtk_tree_store_append (store, &iter, NULL);
			if(k==0) iter0 = iter;
			k++;
        		gtk_tree_store_set (store, &iter, 0, listSCFView[i], -1);
		}
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX (comboSCF), &iter0);
	}
}
/************************************************************************************************************/
static void traitementSCF (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfSCF;i++)
	{
		if(strcmp((gchar*)data,listSCFView[i])==0) res = listSCFReal[i];
	}
	if(res) sprintf(selectedSCF,"%s",res);
	else  sprintf(selectedSCF,"DEF");
	
}
/********************************************************************************************************/
static void treatmentvxc (GtkComboBox *combobox, gpointer d)
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
        for(i=0;i<numberOfvxc;i++)
        {
                if(strcmp((gchar*)data,listvxcView[i])==0) res = listvxcReal[i];
        }
        if(res) sprintf(selectedvxc,"%s",res);
        else  sprintf(selectedvxc,"Default");

}
/********************************************************************************************************/
static GtkWidget *create_list_vxc()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfvxc;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listvxcView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(treatmentvxc), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
GtkWidget* addDeMonvxcToTable(GtkWidget *table, gint i)
{
	GtkWidget* combovxc = NULL;


	add_label_table(table,_("VXCTYP"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	combovxc  = create_list_vxc();

	return combovxc;
}
/************************************************************************************************************/
static GtkWidget *create_list_scf()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfSCF;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listSCFView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementSCF), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
static void setDeMonMethods()
{
	GtkTreeIter iter;
	GtkTreeIter iter0;
	gint i;

	if(strcmp(selectedTypeMethod,"HF")==0)
	{
		listMethodsView = listHFMethodsView;
		listMethodsReal = listHFMethodsReal;
		numberOfMethods = numberOfHFMethods;
	}
	else
	if(strcmp(selectedTypeMethod,"DFT")==0)
	{
		listMethodsView = listDFTMethodsView;
		listMethodsReal = listDFTMethodsReal;
		numberOfMethods = numberOfDFTMethods;
	}
	else
	if(strcmp(selectedTypeMethod,"Hybrid")==0)
	{
		listMethodsView = listHybridMethodsView;
		listMethodsReal = listHybridMethodsReal;
		numberOfMethods = numberOfHybridMethods;
	}
	else
	if(strcmp(selectedTypeMethod,"Meta-GGA")==0)
	{
		listMethodsView = listMetaGGAMethodsView;
		listMethodsReal = listMetaGGAMethodsReal;
		numberOfMethods = numberOfMetaGGAMethods;
	}

	else
	if(strcmp(selectedTypeMethod,"D-Hybrid")==0)
	{
		listMethodsView = listDHybridMethodsView;
		listMethodsReal = listDHybridMethodsReal;
		numberOfMethods = numberOfDHybridMethods;
	}
	else
	if(strcmp(selectedTypeMethod,"MP2")==0)
	{
		listMethodsView = listMP2MethodsView;
		listMethodsReal = listMP2MethodsReal;
		numberOfMethods = numberOfMP2Methods;
	}
	else
	if(strcmp(selectedTypeMethod,"HL-SR")==0)
	{
		listMethodsView = listHighLevelSingleRefMethodsView;
		listMethodsReal = listHighLevelSingleRefMethodsReal;
		numberOfMethods = numberOfHighLevelSingleRefMethods;
	}
	else
	if(strcmp(selectedTypeMethod,"SemiEmp")==0)
	{
		listMethodsView = listSemiEmpMethodsView;
		listMethodsReal = listSemiEmpMethodsReal;
		numberOfMethods = numberOfSemiEmpMethods;
	}
	else
	{
		listMethodsView = listHFMethodsView;
		listMethodsReal = listHFMethodsReal;
		numberOfMethods = numberOfHFMethods;
	}
	
	if (comboMethod && gtk_combo_box_get_active_iter (GTK_COMBO_BOX(comboMethod), &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(comboMethod));
        	GtkTreeStore *store = GTK_TREE_STORE(model);
		gtk_tree_store_clear(store);

		for(i=0;i<numberOfMethods;i++)
		{
        		gtk_tree_store_append (store, &iter, NULL);
			if(i==0) iter0 = iter;
        		gtk_tree_store_set (store, &iter, 0, listMethodsView[i], -1);
		}
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX (comboMethod), &iter0);
	}

}
/************************************************************************************************************/
static void traitementTypeMethod (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfTypeMethods;i++)
	{
		if(strcmp((gchar*)data,listTypeMethodsView[i])==0) res = listTypeMethodsReal[i];
	}
	if(res) sprintf(selectedTypeMethod,"%s",res);
	else  sprintf(selectedSCF,"HF");

	setDeMonMethods();
}
/********************************************************************************************************/
static GtkWidget *create_list_type_methods()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfTypeMethods;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listTypeMethodsView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementTypeMethod), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/*************************************************************************************************************/
static void traitementMethod (GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	gchar* res = NULL;
	gint i;
	GtkWidget* wid;
	gboolean ri = FALSE;

	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	else return;
	for(i=0;i<numberOfMethods;i++)
	{
		if(strcmp((gchar*)data,listMethodsView[i])==0) res = listMethodsReal[i];
	}
	if(res) sprintf(selectedMethod,"%s",res);
	else  sprintf(selectedMethod,"HF");
	if(strstr(selectedMethod,"RI")) ri = TRUE;

	ri = TRUE;// activate it in all cases

	wid  = g_object_get_data(G_OBJECT (combobox), "LabelAuxBasis1");
	if(wid) gtk_widget_set_sensitive(wid, ri);
	wid  = g_object_get_data(G_OBJECT (combobox), "LabelAuxBasis2");
	if(wid) gtk_widget_set_sensitive(wid, ri);
	wid  = g_object_get_data(G_OBJECT (combobox), "ComboAuxBasis");
	if(wid) gtk_widget_set_sensitive(wid, ri);
	setDeMonExcited();
}
/********************************************************************************************************/
static GtkWidget *create_list_methods()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);


	for(i=0;i<numberOfMethods;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listMethodsView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementMethod), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
void createDeMonKeywordsFrame(GtkWidget *win, GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* comboBasis;
	GtkWidget* sep;
	GtkWidget* combo = NULL;
	GtkWidget* comboScfConvergence  = NULL;
	gint l=0;
	gint c=0;
	gint ncases=1;
	GtkWidget *table = gtk_table_new(10,3,FALSE);

	comboMethod = NULL;
	comboTypeMethod = NULL;
	comboExcited = NULL;

	listMethodsView = listHybridMethodsView;
	listMethodsReal = listHybridMethodsReal;
	numberOfMethods = numberOfHybridMethods;

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
	/*------------------ SCF Type -----------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("SCF Type"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_scf();
	comboSCF=combo;
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------ SCF Tighten -----------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("SCF Tighten"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combotight  = create_list_tight();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combotight,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------ SCF Max Iter ------------------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("SCF Max Iterations"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	entryscfmaxit = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryscfmaxit),"100");
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),entryscfmaxit,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
		  2,2);
	/*------------------ SCF Convergence -----------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("SCF Convergence"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	comboScfConvergence  = create_list_scfconvergence();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),comboScfConvergence,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);

	/*------------------ VXCTYP ------------------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("VXCTYP"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combovxc = create_list_vxc();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combovxc,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
		  2,2);
	/*------------------ DIIS ------------------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("DIIS"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combodiis = create_list_diis();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combodiis,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
		  2,2);
	/*------------------ Grid ------------------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("Grid"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combogrid = create_list_grid();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combogrid,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
		  2,2);
	/*------------------ Shift ------------------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("Shift"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	entryShift = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryShift),"0.0");
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),entryShift,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
		  2,2);
        /*------------------ MIXING ------------------------------------------------*/
        l++;
        c = 0; ncases=1;
        add_label_table(table,_("Mixing"),l,c);
        c = 1; ncases=1;
        add_label_table(table,":",l,c);
        entryMixing = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(entryMixing),"-0.3");
        c = 2; ncases=1;
        gtk_table_attach(GTK_TABLE(table),entryMixing,c,c+ncases,l,l+1,
                (GtkAttachOptions)      (GTK_FILL | GTK_EXPAND),
                (GtkAttachOptions)      (GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------ Method Type -----------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("Type of method"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_type_methods();
	comboTypeMethod = combo;
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------  Method -----------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("Method"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_methods();
	comboMethod = combo;
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	setDeMonMethods();
	/*------------------  Excited states -----------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("Excited states"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_excited();
	comboExcited = combo;
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	setDeMonExcited();
	/*------------------ separator -----------------------------------------*/
	l++;
	sep = gtk_hseparator_new ();;
	c = 0; ncases=3;
	gtk_table_attach(GTK_TABLE(table),sep,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------ basis -----------------------------------------*/
	l++;
	comboBasis = addDeMonBasisToTable(table, l+1);
	addDeMonTypeBasisToTable(table, l, comboBasis);
	l++;
	/*------------------ aux basis -----------------------------------------*/
	l++;
	addDeMonAuxBasisToTable(table, l, comboMethod, comboExcited);
	setDeMonMethods(); /* For activate sensitive aux basis */
}
