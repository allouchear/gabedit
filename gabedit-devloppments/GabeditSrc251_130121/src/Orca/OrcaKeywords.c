/* OrcaKeywords.c */
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
#include "../Orca/OrcaTypes.h"
#include "../Orca/OrcaGlobal.h"
#include "../Orca/OrcaMolecule.h"
#include "../Orca/OrcaBasis.h"
#include "../Orca/OrcaGuess.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"

static GtkWidget* comboSCF = NULL;
static GtkWidget* comboMethod = NULL;
static GtkWidget* comboTypeMethod = NULL;
static GtkWidget* comboExcited = NULL;
static void setOrcaMethods();
/*************************************************************************************************************/
static gchar* listJobView[] = {
        "Single Point Energy", 
	"Equilibrium structure search", 
	"Frequencies",
	"Transition state",
	"Equilibrium structure search + Frequencies", 
	"Transition state + Frequencies", 
};
static gchar* listJobReal[] = {
        "SP", 
	"Opt", 
        "NumFreq", 
	"OptTS", 
        "Opt NumFreq", 
        "OptTS NumFreq", 
};
static guint numberOfJobs = G_N_ELEMENTS (listJobView);
static gchar selectedJob[BSIZE]="SP";
/*************************************************************************************************************/
static gchar* listSCFView[] = { "default","unrestricted", "restricted"};
static gchar* listSCFReal[] = { "DEF", "TRUE", "FALSE"};
static guint numberOfSCF = G_N_ELEMENTS (listSCFView);
static gchar selectedSCF[BSIZE]="DEF";
/*************************************************************************************************************/
static gchar* listTypeMethodsView[] = 
{ 
	"Hartree-Fock", 
	"Local and gradient corrected functionalsOrca", 
	"Hybrid functionalsOrca",
	"Meta-GGA and hybrid meta-GGA's",
	"Perturbatively corrected double hybrid functional",
	"Second Order Many Body Perturbation Theory",
	"High-level Single Reference Methods",
	"Semiempirical Methods",
};
static gchar* listTypeMethodsReal[] = 
{ 
	"HF", 
	"DFT", 
	"Hybrid",
	"Meta-GGA",
	"D-Hybrid",
	"MP2",
	"HL-SR",
	"SemiEmp",
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
	"HFS Hartree-Fock-Slater Exchange only functional",
	"LDA Local density approximation",
	"VWN5 Vosko-Wilk-Nusair local density approx",
	"VWN3 Vosko-Wilk-Nusair local density approx.",
	"PWLDA Perdew-Wang parameterization of LDA",
	"BP86 Becke '88 exchange and Perdew '86 correlation",
	"BLYP Becke '88 exchange and Lee-Yang-Parr correlation",
	"OLYP Handy's 'optimal' exchange and Lee-Yang-Parr correlation",
	"GLYP Gill's '96 exchange and Lee-Yang-Parr correlation",
	"XLYP The Xu and Goddard exchange and Lee-Yang-Parr correlation",
	"PW91 Perdew-Wang '91 GGA functional",
	"mPWPW Modified PW exchange and PW correlation",
	"mPWLYP Modified PW exchange and LYP correlation",
	"PBE Perdew-Burke-Erzerhoff GGA functional",
	"RPBE 'Modified' PBE",
	"REVPBE 'Revised' PBE",
	"PWP Perdew-Wang '91 exchange and Perdew 86 correlation"
};
static gchar* listDFTMethodsReal[] = 
{ 
	"HFS",
	"LDA",
	"VWN5",
	"VWN3",
	"PWLDA",
	"BP86",
	"BLYP",
	"OLYP",
	"GLYP",
	"XLYP",
	"PW91",
	"mPWPW",
	"mPWLYP",
	"PBE",
	"RPBE",
	"REVPBE",
	"PWP",
};
static guint numberOfDFTMethods = G_N_ELEMENTS (listDFTMethodsView);
/*************************************************************************************************************/
static gchar* listHybridMethodsView[] = 
{ 
	"B1LYP The one-parameter hybrid Becke'88 exchange and LYP correlation (25% HF exchange)",
	"B3LYP The popular B3LYP functional (20% HF exchange)",
	"O3LYP The Handy hybrid functional",
	"X3LYP The Xu and Goddard hybrid functional",
	"B1P The one parameter hybrid version of BP86",
	"B3P The three parameter hybrid version of BP86",
	"B3PW The three parameter hybrid version of PW91",
	"PW1PW One parameter hybrid version of PW91",
	"mPW1PW One parameter hybrid version of mPWPW",
	"mPW1LYP One parameter hybrid version of mPWLYP",
	"PBE0 One parameter hybrid version of PBE",
};
static gchar* listHybridMethodsReal[] = 
{ 
	"B1LYP",
	"B3LYP",
	"O3LYP",
	"X3LYP",
	"B1P",
	"B3P",
	"B3PW",
	"PW1PW",
	"mPW1PW",
	"mPW1LYP",
	"PBE0",
};
static guint numberOfHybridMethods = G_N_ELEMENTS (listHybridMethodsView);
/*************************************************************************************************************/
static gchar* listMetaGGAMethodsView[] = 
{ 
	"TPSS The TPSS meta-GGA functional",
	"TPSSh The hybrid version of TPSS",
	"TPSS0 A 25% exchange version of TPSShi",
};
static gchar* listMetaGGAMethodsReal[] = 
{ 
	"TPSS",
	"TPSSh",
	"TPSS0",
};
static guint numberOfMetaGGAMethods = G_N_ELEMENTS (listMetaGGAMethodsView);
/*************************************************************************************************************/
static gchar* listDHybridMethodsView[] = 
{ 
	"B2PLYP The new mixture of MP2 and DFT from Grimme",
	"RI-B2PLYP B2PLYP with RI applied to the MP2 part",
	"B2PLYP-D B2PLYP with Van der Waals correction",
	"RI-B2PLYP RIJONX The same but with RI also applied in the SCF part",
	"mPW2PLYP mPW exchange instead of B88 (also with RI and RIJONX as above for B2PYLP)",
	"mPW is supposed to improve on weak interactions",
	"B2GP-PLYP Gershom Martin's 'general purpose' reparameterization",
	"B2K-PLYP Gershom Martin's 'kinetic' reparameterization",
	"B2T-PLYP Gershom Martin's 'thermochemistry' reparameterization",
};
static gchar* listDHybridMethodsReal[] = 
{ 
	"B2PLYP",
	"RI-B2PLYP",
	"B2PLYP-D",
	"RI-B2PLYP",
	"mPW2PLYP",
	"mPW",
	"B2GP-PLYP",
	"B2K-PLYP",
	"B2T-PLYP",
};
static guint numberOfDHybridMethods = G_N_ELEMENTS (listDHybridMethodsView);
/*************************************************************************************************************/
static gchar* listMP2MethodsView[] = 
{ 
	"MP2",
	"RI-MP2",
	"SCS-MP2 Spin-component scaled MP2",
	"RI-SCS-MP2 Spin-component scaled RI-MP2",
};
static gchar* listMP2MethodsReal[] = 
{ 
	"MP2",
	"RI-MP2",
	"SCS-MP2",
	"RI-SCS-MP2",
};
static guint numberOfMP2Methods = G_N_ELEMENTS (listMP2MethodsView);
/*************************************************************************************************************/
static gchar* listHighLevelSingleRefMethodsView[] = 
{ 
	"CCSD  Coupled cluster singles and doubles",
	"CCSD(T) Same with perturbative triples correction",
	"QCISD Quadratic Configuration interaction",
	"QCISD(T) Same with perturbative triples correction",
	"CPF/1 Coupled pair functional",
	"NCPF/1 A 'new' modified coupled pair functional",
	"CEPA/1 Coupled electron pair approximation",
	"NCEPA/1 The CEPA analogue of NCPF/1",
	"MP3 MP3 energies",
	"SCS-MP3 Grimme's refined version of MP3",
};
static gchar* listHighLevelSingleRefMethodsReal[] = 
{ 
	"CCSD",
	"CCSD(T)",
	"QCISD",
	"QCISD(T)",
	"CPF/1",
	"NCPF/1",
	"CEPA/1",
	"NCEPA/1",
	"MP3",
	"SCS-MP3 ",
};
static guint numberOfHighLevelSingleRefMethods = G_N_ELEMENTS (listHighLevelSingleRefMethodsView);
/*************************************************************************************************************/
static gchar* listSemiEmpMethodsView[] = 
{ 
	"ZINDO/S",
	"ZINDO/1",
	"ZINDO/2",
	"NDDO/1",
	"NDDO/2",
	"MNDO",
	"AM1",
	"PM3",
};
static gchar* listSemiEmpMethodsReal[] = 
{ 
	"ZINDO/S",
	"ZINDO/1",
	"ZINDO/2",
	"NDDO/1",
	"NDDO/2",
	"MNDO",
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
	"TightSCF : tight SCF convergence",
	"LooseSCF : loose SCF convergence",
	"SloppySCF: sloppy SCF convergence",
	"StrongSCF: strong SCF convergence",
	"VeryTightSCF :very tight SCF convergence",
	"ScfConv6 : energy convergence check and ETol=10-6",
	"ScfConv7 : energy convergence check and ETol=10-7",
	"ScfConv8 : energy convergence check and ETol=10-8",
	"ScfConv9 : energy convergence check and ETol=10-9",
	"ScfConv10: energy convergence check and ETol=10-10",
};
static gchar* listScfConvergenceReal[] = { 
	"NONE", 
	"TightSCF",
	"LooseSCF",
	"SloppySCF",
	"StrongSCF",
	"VeryTightSCF",
	"ScfConv6",
	"ScfConv7",
	"ScfConv8",
	"ScfConv9",
	"ScfConv10",

};
static guint numberOfScfConvergence = G_N_ELEMENTS (listScfConvergenceView);
static gchar selectedScfConvergence[BSIZE]="NONE";
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
static void putOrcaExcitedInfoInTextEditor()
{
	gchar buffer[BSIZE];
	if(!strcmp(selectedExcited,"NONE"))return;
	if(strstr(selectedExcited,"CIS")) sprintf(buffer,"%ccis\n",'%');
	else sprintf(buffer,"%ctddft\n",'%');
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

	sprintf(buffer,"     nroots %d # the number of excited states to be calculated.\n",8);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	sprintf(buffer,"     maxdim %d # the maximum dimension of the expansion space in the Davidson procedure.\n",30);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	if(!strcmp(selectedExcited,"CIS(D)") || !strcmp(selectedTypeMethod,"D-Hybrid"))
	{
		sprintf(buffer, "     dcorr 1\n");
       		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}
	sprintf(buffer,
			"     # dcorr n\n"
			"     # n=1-4. The meaning of the four algorithms\n"
			"     # algorithm 1 Is perhaps the best for small systems. May use a\n"
			"     #             lot of disk space\n"
			"     # algorithm 2 Stores less integrals\n"
			"     # algorithm 3 Is good if the system is large and only a few\n"
			"     #             states are to be made. Safes disk and main memory.\n"
			"     # algorithm 4 Uses only transformed RI integrals. May be the\n"
			"     #             fastest for large systems and a larger number of states\n"
			);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

	if(!strcmp(selectedTypeMethod,"DFT"))
	{
		sprintf(buffer,"     # TammDanCoff true Tamm-Dancoff approximation for non-hybride\n");
       		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}
	sprintf(buffer,"     # Triplets true : do triplets states\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	sprintf(buffer,"     # EWin -3,100  (orbital energy window in Eh)\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	sprintf(buffer,"     # Etol 1e-3  the required convergence of the energies of the excited states (in Eh)\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	sprintf(buffer,"     # Rtol 1e-5  required convergence on the norm of the residual vectors.\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

	sprintf(buffer,"     # DoQuad true  Quadrupole contributions (and magnetic dipole contributions)\n");
	sprintf(buffer,"                   # essential for metal edges. For ligand edges, the contributions are much smaller.\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

	if(!strcmp(selectedExcited,"CIS")) sprintf(buffer," end #cis\n");
	else sprintf(buffer," end #tddft\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
}
/*************************************************************************************************************/
static void setOrcaExcited()
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
GtkWidget* addOrcaScfConvergenceToTable(GtkWidget *table, gint i)
{
	GtkWidget* comboScfConvergence = NULL;


	add_label_table(table,_("SCF convergence"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	comboScfConvergence  = create_list_scfconvergence();

	return comboScfConvergence;
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
static void putOrcaJobTypeInfoInTextEditor()
{
	if(!strcmp(selectedJob,"SP"))return;
	else
	{
		gchar buffer[BSIZE];
		sprintf(buffer," %s ",selectedJob);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}
}
/*************************************************************************************************************/
static void putOrcaSCFKeywordsInfoInTextEditor()
{
	if(strcmp(selectedSCF,"DEF"))
	{
		//if(!strcmp(selectedSCF,"TRUE"))
		if(strcmp(selectedSCF,"TRUE"))
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " R",-1);
		else
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " U",-1);
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
		sprintf(buffer,"  %s ",selectedScfConvergence);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}
}
/*************************************************************************************************************/
static void putOrcaMethodInfoInTextEditor()
{
	gchar buffer[BSIZE];
	sprintf(buffer," %s ",selectedMethod);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
}
/*************************************************************************************************************/
static void putOrcaPrintBasisInTextEditor()
{
	gchar buffer[BSIZE];
	sprintf(buffer,"\n!");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &orcaColorFore.keyWord, &orcaColorBack.keyWord, buffer,-1);
	sprintf(buffer," PrintBasis");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
}
/*************************************************************************************************************/
static void putOrcaOrbitalsInTextEditor()
{
	gchar buffer[BSIZE];
	sprintf(buffer,"%coutput\n",'%');
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &orcaColorFore.keyWord, &orcaColorBack.keyWord, buffer,-1);
	sprintf(buffer,"     print[p_mos] 1\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	sprintf(buffer," end #output\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
}
/*************************************************************************************************************/
void putOrcaKeywordsInfoInTextEditor()
{
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &orcaColorFore.keyWord, &orcaColorBack.keyWord, "! ",-1);
	putOrcaJobTypeInfoInTextEditor();
	putOrcaSCFKeywordsInfoInTextEditor();
	putOrcaMethodInfoInTextEditor();
	putOrcaPrintBasisInTextEditor();
	putOrcaGuessInfoInTextEditor();
	if(!strstr(selectedTypeMethod,"SemiEmp")) putOrcaBasisInfoInTextEditor();
	if(strstr(selectedMethod,"RI")) putOrcaAuxBasisInTextEditor();
	if(!strcmp(selectedExcited,"CIS(D)") || !strcmp(selectedTypeMethod,"D-Hybrid")) putOrcaAuxBasisInTextEditor();

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &orcaColorFore.keyWord, &orcaColorBack.keyWord, "\n",-1);
	putOrcaExcitedInfoInTextEditor();
	putOrcaOrbitalsInTextEditor();
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
void setOrcaSCFMethod(gboolean okRHF)
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
static void setOrcaMethods()
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

	setOrcaMethods();
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
	wid  = g_object_get_data(G_OBJECT (combobox), "LabelAuxBasis1");
	if(wid) gtk_widget_set_sensitive(wid, ri);
	wid  = g_object_get_data(G_OBJECT (combobox), "LabelAuxBasis2");
	if(wid) gtk_widget_set_sensitive(wid, ri);
	wid  = g_object_get_data(G_OBJECT (combobox), "ComboAuxBasis");
	if(wid) gtk_widget_set_sensitive(wid, ri);
	setOrcaExcited();
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
void createOrcaKeywordsFrame(GtkWidget *win, GtkWidget *box)
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
	setOrcaMethods();
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
	setOrcaExcited();
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
	comboBasis = addOrcaBasisToTable(table, l+1);
	addOrcaTypeBasisToTable(table, l, comboBasis);
	l++;
	/*------------------ aux basis -----------------------------------------*/
	l++;
	addOrcaAuxBasisToTable(table, l, comboMethod, comboExcited);
	setOrcaMethods(); /* For activate sensitive aux basis */
}
