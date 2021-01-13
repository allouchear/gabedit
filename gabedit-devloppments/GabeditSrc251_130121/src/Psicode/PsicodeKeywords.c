/* PsicodeKeywords.c */
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
#include "../Psicode/PsicodeTypes.h"
#include "../Psicode/PsicodeGlobal.h"
#include "../Psicode/PsicodeMolecule.h"
#include "../Psicode/PsicodeBasis.h"
#include "../Psicode/PsicodeGuess.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"

static GtkWidget* comboSCF = NULL;
static GtkWidget* comboMethod = NULL;
static GtkWidget* comboTypeMethod = NULL;
static GtkWidget* comboExcited = NULL;
static void setPsicodeMethods();
/*************************************************************************************************************/
static gchar* listJobView[] = {
        "Single Point Energy", 
	"Equilibrium structure search", 
	"Frequencies",
	"Equilibrium structure search + Frequencies", 
};
static gchar* listJobReal[] = {
        "SP", 
	"optimize", 
        "freq", 
        "opt&freq", 
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
	"Hatree-Fock", 
	"Local and gradient corrected functionalsPsi", 
	"Hybrid functionalsPsi",
	"Meta-GGA and hybrid meta-GGA's",
	"X_LRC",
	"Second Order Many Body Perturbation Theory",
	"High-level Single Reference Methods",
	/*"Semiempirical Methods",*/
};
static gchar* listTypeMethodsReal[] = 
{ 
	"SCF", 
	"DFT", 
	"Hybrid",
	"Meta-GGA",
	"X_LRC",
	"MP2",
	"HL-SR",
	/* "SemiEmp",*/
};
static guint numberOfTypeMethods = G_N_ELEMENTS (listTypeMethodsView);
static gchar selectedTypeMethod[BSIZE]="SCF";
/*************************************************************************************************************/
static gchar* listHFMethodsView[] = 
{ 
	"HF", 
};
static gchar* listHFMethodsReal[] = 
{ 
	"SCF", 
};
static guint numberOfHFMethods = G_N_ELEMENTS (listHFMethodsView);
/*************************************************************************************************************/
static gchar* listHFDFTMethodsView[] = 
{ 
	"HFexch", 
};
static gchar* listHFDFTMethodsReal[] = 
{ 
	"XC HFexch", 
};
static guint numberOfHFDFTMethods = G_N_ELEMENTS (listHFDFTMethodsView);
/*************************************************************************************************************/
static gchar* listDFTMethodsView[] = 
{ 
	"B3_X",
	"B88_X",
	"B97-D",
	"BLYP",
	"BLYP-D",
	"BP86",
	"FT97",
	"FT97B_X",
	"FT97_C",
	"HCTH",
	"HCTH120",
	"HCTH147",
	"HCTH407",
	"LYP_C",
	"P86_C",
	"PBE",
	"PBE-D",
	"PBESOL_X",
	"PBE_C",
	"PBE_X",
	"PW91",
	"PW91_C",
	"PW91_X",
	"PW92_C",
	"PZ81_C",
	"SVWN",
	"S_X",
	"VWN3RPA_C",
	"VWN3_C",
	"VWN5RPA_C",
	"VWN5_C",
};
static gchar* listDFTMethodsReal[] = 
{ 
	"B3_X",
	"B88_X",
	"B97-D",
	"BLYP",
	"BLYP-D",
	"BP86",
	"FT97",
	"FT97B_X",
	"FT97_C",
	"HCTH",
	"HCTH120",
	"HCTH147",
	"HCTH407",
	"LYP_C",
	"P86_C",
	"PBE",
	"PBE-D",
	"PBESOL_X",
	"PBE_C",
	"PBE_X",
	"PW91",
	"PW91_C",
	"PW91_X",
	"PW92_C",
	"PZ81_C",
	"SVWN",
	"S_X",
	"VWN3RPA_C",
	"VWN3_C",
	"VWN5RPA_C",
	"VWN5_C",
};
static guint numberOfDFTMethods = G_N_ELEMENTS (listDFTMethodsView);
/*************************************************************************************************************/
static gchar* listHybridMethodsView[] = 
{ 
	"B3LYP",
	"B3LYP-D",
	"B3LYP5",
	"B3LYP5-D",
	"B97-0",
	"B97-1",
	"B97-2",
	"PBE0",
	"wB97X Parameterized Hybrid LRC B97 GGA XC Functional",
	"wB97X-D",
	"wPBE0 SR-XC Functional (HJS Model)",
	"wPBE0sol0",
};
static gchar* listHybridMethodsReal[] = 
{ 
	"B3LYP",
	"B3LYP-D",
	"B3LYP5 ",
	"B3LYP5-D",
	"B97-0",
	"B97-1",
	"B97-2",
	"PBE0",
	"wB97X",
	"wB97X-D",
	"wPBE0",
	"wPBE0sol0",
};
static guint numberOfHybridMethods = G_N_ELEMENTS (listHybridMethodsView);
/*************************************************************************************************************/
static gchar* listMetaGGAMethodsView[] = 
{ 
	"m05",
	"m05-2X",
};
static gchar* listMetaGGAMethodsReal[] = 
{ 
	"m05",
	"m05-2X",
};
static guint numberOfMetaGGAMethods = G_N_ELEMENTS (listMetaGGAMethodsView);
/*************************************************************************************************************/
static gchar* listXLRCMethodsView[] = 
{ 
	"wB88_X",
	"wB97",
	"wB97X",
	"wB97X-D",
	"wBLYP",
	"wPBE",
	"wPBE0",
	"wPBE_X",
	"wPBEsol",
	"wPBEsol0",
	"wPBEsol_X",
	"wSVWN",
	"wS_X",
};
static gchar* listXLRCMethodsReal[] = 
{ 
	"wB88_X",
	"wB97",
	"wB97X",
	"wB97X-D",
	"wBLYP",
	"wPBE",
	"wPBE0",
	"wPBE_X",
	"wPBEsol",
	"wPBEsol0",
	"wPBEsol_X",
	"wSVWN",
	"wS_X",
};
static guint numberOfXLRCMethods = G_N_ELEMENTS (listXLRCMethodsView);
/*************************************************************************************************************/
static gchar* listMP2MethodsView[] = 
{ 
	"MP2",
	"DF-MP2",
};
static gchar* listMP2MethodsReal[] = 
{ 
	"MP2",
	"DF-MP2",
};
static guint numberOfMP2Methods = G_N_ELEMENTS (listMP2MethodsView);
/*************************************************************************************************************/
static gchar* listHighLevelSingleRefMethodsView[] = 
{ 
	"CC2",
	"CCD",
	"CCSD",
	"CC3",
	"CCD(T)",
	"CCSD(T) ",
};
static gchar* listHighLevelSingleRefMethodsReal[] = 
{ 
	"CC2",
	"CCD",
	"CCSD",
	"CC3",
	"CCD(T)",
	"CCSD(T) ",
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
	"Energy convergence = 10^-6 au",
	"Energy convergence = 10^-7 au",
	"Energy convergence = 10^-8 au",
	"Energy convergence = 10^-9 au",
	"Energy convergence = 10^-10 au"
};
static gchar* listScfConvergenceReal[] = { 
	"NONE", 
	"6",
	"7",
	"8",
	"9",
	"10"

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
	"CIS",
	"TDDFT",
};
static gchar* listDFTExcitedReal[] = 
{ 
	"NONE", 
	"CIS",
	"TDDFT",
};
static guint numberOfDFTExcited = G_N_ELEMENTS (listDFTExcitedView);
/*************************************************************************************************************/
static gchar* listHLExcitedView[] = 
{ 
	"Nothing",
	"EOM",
};
static gchar* listHLExcitedReal[] = 
{ 
	"NONE", 
	"EOM",
};
static guint numberOfHLExcited = G_N_ELEMENTS (listHLExcitedView);
/*************************************************************************************************************/
static gchar selectedExcited[BSIZE]="NONE";
static gchar** listExcitedView = listNothingExcitedView;
static gchar** listExcitedReal = listNothingExcitedReal;
static guint numberOfExcited = G_N_ELEMENTS (listNothingExcitedView);
/*************************************************************************************************************/
gchar* getPsicodeExcitedMethod()
{
	return selectedExcited;
}
/*************************************************************************************************************/
gchar* getPsicodeTypeMethod()
{
	return selectedTypeMethod;
}
/*************************************************************************************************************/
static void putPsicodeExcitedInfoInTextEditor()
{
	gchar buffer[BSIZE];
	if(!strcmp(selectedExcited,"NONE"))return;
	if(!strcmp(selectedExcited,"EOM"))return;
 	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
      	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &psicodeColorFore.keyWord, &psicodeColorBack.keyWord, "tddft\n", -1);

	if(strstr(selectedExcited,"CIS")) sprintf(buffer," cis\n");
	else sprintf(buffer," rpa\n");
 	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

	sprintf(buffer," nroots %d # the number of excited states to be calculated.\n",8);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	sprintf(buffer,
			"# nosinglet\n"
			"# notriplet\n"
			"# maxvecs 1000 # it is the maximum number of trial vectors that the calculation is allowed to hold\n"
			"# thresh 1e-4\n"
			"# maxiter 100\n"
			"# freeze atomic # To exclude the atom-like core regions\n"
			"# freeze 10     #  the number of lowest-lying occupied orbitals be excluded\n"
			"# freeze virtual 5      # to freeze the top 5 virtual orbitals\n"
			);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);

	sprintf(buffer,"end #tddft\n");
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
}
/*************************************************************************************************************/
static void setPsicodeExcited()
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
	if(strcmp(selectedTypeMethod,"HFexch")==0)
	{
		listExcitedView = listDFTExcitedView;
		listExcitedReal = listDFTExcitedReal;
		numberOfExcited = numberOfDFTExcited;
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
	if(strcmp(selectedTypeMethod,"HL-SR")==0 && (!strcmp(selectedMethod,"CC2") || !strcmp(selectedMethod,"CCSD") ))
	{
		listExcitedView = listHLExcitedView;
		listExcitedReal = listHLExcitedReal;
		numberOfExcited = numberOfHLExcited;
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
	/*
{
	gboolean ri = FALSE;
	GtkWidget* wid;
	ri = !strcmp(selectedExcited,"CIS(D)");
	wid  = g_object_get_data(G_OBJECT (combobox), "LabelAuxBasis1");
	if(wid) gtk_widget_set_sensitive(wid, ri);
	wid  = g_object_get_data(G_OBJECT (combobox), "LabelAuxBasis2");
	if(wid) gtk_widget_set_sensitive(wid, ri);
	wid  = g_object_get_data(G_OBJECT (combobox), "ComboAuxBasis");
	if(wid) gtk_widget_set_sensitive(wid, ri);
}
	*/
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
GtkWidget* addPsicodeScfConvergenceToTable(GtkWidget *table, gint i)
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
/************************************************************************************************************/
/*
static void getMultiplicityName(gint multiplicity, gchar* buffer)
{
	if(multiplicity==1) sprintf(buffer,"Singlet");
	else if(multiplicity==2) sprintf(buffer,"Doublet");
	else if(multiplicity==3) sprintf(buffer,"Triplet");
	else if(multiplicity==4) sprintf(buffer,"Quartet");
	else if(multiplicity==5) sprintf(buffer,"Quintet");
	else if(multiplicity==6) sprintf(buffer,"Sextet");
	else if(multiplicity==7) sprintf(buffer,"Septet");
	else if(multiplicity==8) sprintf(buffer,"Octet");
	else sprintf(buffer,"NOpen %d",multiplicity/2);
}
*/
/*************************************************************************************************************/
static void putPsicodeSCFKeywordsInfoInTextEditor()
{
	if( !strcmp(selectedTypeMethod,"DFT") || !strcmp(selectedTypeMethod,"HFexch")|| !strcmp(selectedTypeMethod,"Hybrid") || !strcmp(selectedTypeMethod,"Meta-GGA") || !strcmp(selectedTypeMethod,"X-LRC"))
	{
		gchar buffer[BSIZE];
		sprintf(buffer,"set dft_functional ");
       		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &psicodeColorFore.keyWord, &psicodeColorBack.keyWord, buffer, -1);
		if(strcmp(selectedScfConvergence,"NONE")!=0) 
		{
			sprintf(buffer,"set scf e_convergence=%s\n",selectedScfConvergence);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
		}
		sprintf(buffer," %s\n",selectedMethod);
       		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}
	else
	{
		gchar buffer[BSIZE];
		if(strcmp(selectedScfConvergence,"NONE")!=0) 
		{
			sprintf(buffer,"set scf e_convergence=%s\n",selectedScfConvergence);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
		}
	}

	if(strcmp(selectedSCF,"DEF") || getPsicodeMultiplicity()!=1)
	{
		if( getPsicodeMultiplicity()!=1)
			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "set scf reference uhf\n",-1);
		else
		{
			if(!strcmp(selectedSCF,"TRUE")) gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "set scf reference uhf\n",-1);
			else gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "set scf reference rhf\n",-1);
		}

	}
	else if(strstr(selectedMethod,"MP2") && getPsicodeMultiplicity()!=1)
	{
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " uhf\n",-1);
	}
}
/*************************************************************************************************************/
static void putPsicodeMP2KeywordsInfoInTextEditor()
{
	if(strstr(selectedMethod,"MP2"))
	{
		if(strstr(selectedMethod,"DF"))
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "set scf_type df\n",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "set freeze_core True\n",-1);
	}
}
/*************************************************************************************************************/
static void putPsicodeMethodInfoInTextEditor()
{
	gchar buffer[BSIZE];
	gchar sMethod[BSIZE];
	sprintf(sMethod,"SCF");
        if(strcmp(selectedMethod,"SCF")) 
	{
        	if(strstr(selectedMethod,"MP2")) sprintf(sMethod,"%s",selectedMethod);
        	else if(strstr(selectedTypeMethod,"HL-SR")) sprintf(sMethod,"%s",selectedMethod);
        	else if(strstr(selectedExcited,"CIS") || strstr(selectedExcited,"TDDFT")) sprintf(sMethod,"%s","TDDFT");
		else sprintf(sMethod,"SCF");
	}
	if(!strcmp(selectedJob,"SP") && !strstr(selectedExcited,"EOM") )
	{
	sprintf(buffer,"energy(\"%s\")\n",sMethod);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &psicodeColorFore.keyWord, &psicodeColorBack.keyWord, buffer, -1);
	}
	else
	{
		gchar buffer[BSIZE];
		if(strstr(selectedJob,"opt&freq"))
		{
			
			sprintf(buffer,"optimize(\"%s\")\n",sMethod);
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &psicodeColorFore.keyWord, &psicodeColorBack.keyWord, buffer, -1);
			sprintf(buffer,"frequencies(\"%s\")\n",sMethod);
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &psicodeColorFore.keyWord, &psicodeColorBack.keyWord, buffer, -1);
		}
		else if(strstr(selectedJob,"saddle&freq"))
		{
			
			sprintf(buffer," %s","saddle");
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &psicodeColorFore.keyWord, &psicodeColorBack.keyWord, buffer, -1);

			sprintf(buffer,"\ntask %s ",sMethod);
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &psicodeColorFore.keyWord, &psicodeColorBack.keyWord, buffer, -1);

			sprintf(buffer,"frequencies(\"%s\")\n",sMethod);
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &psicodeColorFore.keyWord, &psicodeColorBack.keyWord, buffer, -1);
		}
		else if(strstr(selectedJob,"optimiz"))
		{
			sprintf(buffer,"optimize(\"%s\")\n",sMethod);
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &psicodeColorFore.keyWord, &psicodeColorBack.keyWord, buffer, -1);
		}
		else if(strstr(selectedJob,"freq"))
		{
			
			sprintf(buffer,"frequencies(\"%s\")\n",sMethod);
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &psicodeColorFore.keyWord, &psicodeColorBack.keyWord, buffer, -1);
		}
		else if(!strstr(selectedTypeMethod,"HL-SR"))
		{
			sprintf(buffer," %s ",selectedJob);
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &psicodeColorFore.keyWord, &psicodeColorBack.keyWord, buffer, -1);
		}
	}
}
/*************************************************************************************************************/
static void putPsicodeHLKeywordsInfoInTextEditor()
{
	if(!strcmp(selectedTypeMethod,"HL-SR"))
	{
		gchar buffer[BSIZE];
		if(strstr(selectedExcited,"EOM")) 
		{
			sprintf(buffer,"set roots_per_irrep [2]\n");
 			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
			sprintf(buffer,"property('eom-%s', properties=['oscillator_strength'])\n",selectedMethod);
 			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
		}
	}
}
/*************************************************************************************************************/
void putPsicodeKeywordsInfoInTextEditor()
{
	putPsicodeGuessInfoInTextEditor();
	if(!strstr(selectedTypeMethod,"SemiEmp")) putPsicodeBasisInfoInTextEditor();
	if(strstr(selectedMethod,"RI")) putPsicodeAuxBasisInTextEditor();
	if(!strcmp(selectedExcited,"CIS(D)")) putPsicodeAuxBasisInTextEditor();
	putPsicodeSCFKeywordsInfoInTextEditor();
	putPsicodeMP2KeywordsInfoInTextEditor();
	putPsicodeMethodInfoInTextEditor();
	putPsicodeHLKeywordsInfoInTextEditor();
	putPsicodeExcitedInfoInTextEditor();

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &psicodeColorFore.keyWord, &psicodeColorBack.keyWord, "\n",-1);
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
void setPsicodeSCFMethod(gboolean okRHF)
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
static void setPsicodeMethods()
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
	if(strcmp(selectedTypeMethod,"HFexch")==0)
	{
		listMethodsView = listHFDFTMethodsView;
		listMethodsReal = listHFDFTMethodsReal;
		numberOfMethods = numberOfHFDFTMethods;
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
	if(strcmp(selectedTypeMethod,"X-LRC")==0)
	{
		listMethodsView = listXLRCMethodsView;
		listMethodsReal = listXLRCMethodsReal;
		numberOfMethods = numberOfXLRCMethods;
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

	setPsicodeMethods();
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
	setPsicodeExcited();
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
void createPsicodeKeywordsFrame(GtkWidget *win, GtkWidget *box)
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
	setPsicodeMethods();
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
	setPsicodeExcited();
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
	comboBasis = addPsicodeBasisToTable(table, l+1);
	addPsicodeTypeBasisToTable(table, l, comboBasis);
	l++;
	/*------------------ aux basis -----------------------------------------*/
	l++;
	addPsicodeAuxBasisToTable(table, l, comboMethod, comboExcited);
	setPsicodeMethods(); /* For activate sensitive aux basis */
}
