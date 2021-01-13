/* NWChemKeywords.c */
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
#include "../NWChem/NWChemTypes.h"
#include "../NWChem/NWChemGlobal.h"
#include "../NWChem/NWChemMolecule.h"
#include "../NWChem/NWChemBasis.h"
#include "../NWChem/NWChemGuess.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"

static GtkWidget* comboSCF = NULL;
static GtkWidget* comboMethod = NULL;
static GtkWidget* comboTypeMethod = NULL;
static GtkWidget* comboExcited = NULL;
static void setNWChemMethods();
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
	"optimize", 
        "freq", 
	"saddle", 
        "opt&freq", 
        "saddle&freq", 
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
	"Hatree-Fock using DFT module", 
	"Local and gradient corrected functionalsNWC", 
	"Hybrid functionalsNWC",
	"Meta-GGA and hybrid meta-GGA's",
	"Second Order Many Body Perturbation Theory",
	"High-level Single Reference Methods",
	"Semiempirical Methods",
};
static gchar* listTypeMethodsReal[] = 
{ 
	"SCF", 
	"HFexch", 
	"DFT", 
	"Hybrid",
	"Meta-GGA",
	"MP2",
	"HL-SR",
	"SemiEmp",
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
	"Slater Slater Exchange only functional",
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
};
static gchar* listDFTMethodsReal[] = 
{ 
	"XC slater",
	"XC slater vwn_5",
	"XC vwn_5",
	"XC vwn_3",
	"XC pw91lda",
	"XC becke88 perdew86",
	"XC becke88 lyp",
	"XC op lyp",
	"XC gill96 lyp",
	"XC xperdew91 lyp",
	"XC perdew91",
	"XC mpw91 perdew91",
	"XC mpw91 lyp",
	"XC xpbe96 cpbe96",
};
static guint numberOfDFTMethods = G_N_ELEMENTS (listDFTMethodsView);
/*************************************************************************************************************/
static gchar* listHybridMethodsView[] = 
{ 
	"B3LYP The popular B3LYP functional (20% HF exchange)",
	"PBE0 One parameter hybrid version of PBE",
	"beckehandh",
	"acm",
	"becke97",
	"becke97-1",
	"becke97-2",
	"becke97-3",
	"becke98",
	"mpw1k",
};
static gchar* listHybridMethodsReal[] = 
{ 
	"XC B3LYP",
	"XC PBE0",
	"XC beckehandh",
	"XC acm",
	"XC becke97",
	"XC becke97-1",
	"XC becke97-2",
	"XC becke97-3",
	"XC becke98",
	"XC mpw1k",
};
static guint numberOfHybridMethods = G_N_ELEMENTS (listHybridMethodsView);
/*************************************************************************************************************/
static gchar* listMetaGGAMethodsView[] = 
{ 
	"xpkzb99 cpkzb99",
	"xtpss03 ctpss03",
	"cpw6b95 cpwb6k",
	"xm05 cm05",
	"xm05-2x cm05-2x",
	"xvsxc cvsxc",
	"xm06-L cm06-L",
	"xm06-hf cm06-hf",
	"xm06 cm06",
	"xm06-2x cm06-2x",
	"xctpssh",
	"bb1k",
	"mpw1b95",
	"mpwb1k",
	"pw6b95",
	"pwb6k",
	"m05",
	"vsxc",
	"m06-hf",
	"m06",
	"m06-2x",
};
static gchar* listMetaGGAMethodsReal[] = 
{ 
	"XC xpkzb99 cpkzb99",
	"XC xtpss03 ctpss03",
	"XC cpw6b95 cpwb6k",
	"XC xm05 cm05",
	"XC xm05-2x cm05-2x",
	"XC xvsxc cvsxc",
	"XC xm06-L cm06-L",
	"XC xm06-hf cm06-hf",
	"XC xm06 cm06",
	"XC xm06-2x cm06-2x",
	"XC xctpssh",
	"XC bb1k",
	"XC mpw1b95",
	"XC mpwb1k",
	"XC pw6b95",
	"XC pwb6k",
	"XC m05",
	"XC vsxc",
	"XC m06-hf",
	"XC m06",
	"XC m06-2x",
};
static guint numberOfMetaGGAMethods = G_N_ELEMENTS (listMetaGGAMethodsView);
/*************************************************************************************************************/
static gchar* listMP2MethodsView[] = 
{ 
	"MP2",
	"RIMP2",
};
static gchar* listMP2MethodsReal[] = 
{ 
	"MP2",
	"RIMP2",
};
static guint numberOfMP2Methods = G_N_ELEMENTS (listMP2MethodsView);
/*************************************************************************************************************/
static gchar* listHighLevelSingleRefMethodsView[] = 
{ 
	"LCCD",
	"CCD",
	"CCSD",
	"LCCSD",
	"CCSDT",
	"CCSDTQ",
	"CCSD(T) ",
	"CCSD[T]",
	"CISD",
	"QCISD",
	"CISDT",
	"CISDTQ",
	"MBPT2",
	"MBPT3",
	"MBPT4",
};
static gchar* listHighLevelSingleRefMethodsReal[] = 
{ 
	"LCCD",
	"CCD",
	"CCSD",
	"LCCSD",
	"CCSDT",
	"CCSDTQ",
	"CCSD(T) ",
	"CCSD[T]",
	"CISD",
	"QCISD",
	"CISDT",
	"CISDTQ",
	"MBPT2",
	"MBPT3",
	"MBPT4",
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
	"Energy convergence = 10-6 au",
	"Energy convergence = 10-7 au",
	"Energy convergence = 10-8 au",
	"Energy convergence = 10-9 au",
	"Energy convergence = 10-10 au"
};
static gchar* listScfConvergenceReal[] = { 
	"NONE", 
	"1e-6",
	"1e-7",
	"1e-8",
	"1e-9",
	"1e-10"

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
gchar* getNWChemExcitedMethod()
{
	return selectedExcited;
}
/*************************************************************************************************************/
gchar* getNWChemTypeMethod()
{
	return selectedTypeMethod;
}
/*************************************************************************************************************/
static void putNWChemExcitedInfoInTextEditor()
{
	gchar buffer[BSIZE];
	if(!strcmp(selectedExcited,"NONE"))return;
	if(!strcmp(selectedExcited,"EOM"))return;
 	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
      	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "tddft\n", -1);

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
static void setNWChemExcited()
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
	if(strcmp(selectedTypeMethod,"HL-SR")==0&& (!strcmp(selectedMethod,"CCSD")||!strcmp(selectedMethod,"CCSDT")||!strcmp(selectedMethod,"CCSDTQ")))
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
GtkWidget* addNWChemScfConvergenceToTable(GtkWidget *table, gint i)
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
/*************************************************************************************************************/
static void putNWChemSCFKeywordsInfoInTextEditor()
{
	if( !strcmp(selectedTypeMethod,"DFT") || !strcmp(selectedTypeMethod,"HFexch")|| !strcmp(selectedTypeMethod,"Hybrid") || !strcmp(selectedTypeMethod,"Meta-GGA"))
	{
		gchar buffer[BSIZE];
		gint multiplicity = getNWChemMultiplicity();
       		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "dft\n", -1);
		sprintf(buffer," mult %d",multiplicity);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
		if(strcmp(selectedScfConvergence,"NONE")!=0) 
		{
			sprintf(buffer," iterations %d\n",30);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
			sprintf(buffer," convergence energy %s\n",selectedScfConvergence);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
		}
		sprintf(buffer," %s\n",selectedMethod);
       		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}
	else
	{
		gchar buffer[BSIZE];
		gint multiplicity = getNWChemMultiplicity();
       		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "scf\n", -1);
		getMultiplicityName(multiplicity, buffer);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
		if(strcmp(selectedScfConvergence,"NONE")!=0) 
		{
			sprintf(buffer," maxiter %d\n",30);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
			sprintf(buffer," thresh %s\n",selectedScfConvergence);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
		}
	}

	if(strcmp(selectedSCF,"DEF"))
	{
		if(!strcmp(selectedSCF,"TRUE")) gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " uhf\n",-1);
		else gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " rhf\n",-1);

	}
	else if(strstr(selectedMethod,"MP2") && getNWChemMultiplicity()!=1)
	{
		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " uhf\n",-1);
	}
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "end\n", -1);
}
/*************************************************************************************************************/
static void putNWChemMP2KeywordsInfoInTextEditor()
{
	if(strstr(selectedMethod,"MP2"))
	{
       		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "mp2\n", -1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " freeze core atomic\n",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "# freeze virtual 5\n",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "# tight\n",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "# print\n",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "# noprint\n",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "# riapprox V\n",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "# riapprox SVS\n",-1);
       		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "end\n", -1);
	}
}
/*************************************************************************************************************/
static void putNWChemMethodInfoInTextEditor()
{
	gchar buffer[BSIZE];
	gchar sMethod[BSIZE];
	sprintf(sMethod,"SCF");
        if(strcmp(selectedMethod,"SCF")) 
	{
        	if(strstr(selectedMethod,"MP2")) sprintf(sMethod,"%s",selectedMethod);
        	else if(strstr(selectedTypeMethod,"HL-SR")) sprintf(sMethod,"%s","tce");
        	else if(strstr(selectedExcited,"CIS") || strstr(selectedExcited,"TDDFT")) sprintf(sMethod,"%s","TDDFT");
		else sprintf(sMethod,"DFT");
	}
	sprintf(buffer,"\ntask %s ",sMethod);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);
	if(!strcmp(selectedJob,"SP"))return;
	else
	{
		gchar buffer[BSIZE];
		if(strstr(selectedJob,"opt&freq"))
		{
			
			sprintf(buffer," %s","optimize");
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);

			sprintf(buffer,"\ntask %s ",sMethod);
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);

			sprintf(buffer," %s ","freq");
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);
		}
		else if(strstr(selectedJob,"saddle&freq"))
		{
			
			sprintf(buffer," %s","saddle");
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);

			sprintf(buffer,"\ntask %s ",sMethod);
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);

			sprintf(buffer," %s ","freq");
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);
		}
		else
		{
			sprintf(buffer," %s ",selectedJob);
       			gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, buffer, -1);
		}
	}
}
/*************************************************************************************************************/
static void putNWChemHLKeywordsInfoInTextEditor()
{
	if(!strcmp(selectedTypeMethod,"HL-SR"))
	{
		gchar buffer[BSIZE];
       		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "tce\n", -1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " scf\n",-1);
		sprintf(buffer," %s\n",selectedMethod);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
		sprintf(buffer," dipole\n");
 		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
		if(strstr(selectedExcited,"EOM")) 
		{
			sprintf(buffer," nroots 4\n");
 			gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
		}
		sprintf(buffer, 
			"# maxiter 100\n"
			"# freeze atomic # To exclude the atom-like core regions\n"
			"# freeze 10     #  the number of lowest-lying occupied orbitals be excluded\n"
			"# freeze virtual 5      # to freeze the top 5 virtual orbitals\n"
			);
 		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
       		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "end\n", -1);
	}
}
/*************************************************************************************************************/
void putNWChemKeywordsInfoInTextEditor()
{
	putNWChemGuessInfoInTextEditor();
	if(!strstr(selectedTypeMethod,"SemiEmp")) putNWChemBasisInfoInTextEditor();
	if(strstr(selectedMethod,"RI")) putNWChemAuxBasisInTextEditor();
	if(!strcmp(selectedExcited,"CIS(D)")) putNWChemAuxBasisInTextEditor();
	putNWChemSCFKeywordsInfoInTextEditor();
	putNWChemMP2KeywordsInfoInTextEditor();
	putNWChemHLKeywordsInfoInTextEditor();
	putNWChemExcitedInfoInTextEditor();
	putNWChemMethodInfoInTextEditor();

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "\n",-1);
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
void setNWChemSCFMethod(gboolean okRHF)
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
static void setNWChemMethods()
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

	setNWChemMethods();
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
	setNWChemExcited();
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
void createNWChemKeywordsFrame(GtkWidget *win, GtkWidget *box)
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
	setNWChemMethods();
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
	setNWChemExcited();
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
	comboBasis = addNWChemBasisToTable(table, l+1);
	addNWChemTypeBasisToTable(table, l, comboBasis);
	l++;
	/*------------------ aux basis -----------------------------------------*/
	l++;
	addNWChemAuxBasisToTable(table, l, comboMethod, comboExcited);
	setNWChemMethods(); /* For activate sensitive aux basis */
}
