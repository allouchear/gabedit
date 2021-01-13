/* QChemRem.c */
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
#include "../QChem/QChemTypes.h"
#include "../QChem/QChemGlobal.h"
#include "../QChem/QChemMolecule.h"
#include "../QChem/QChemBasis.h"
#include "../QChem/QChemGuess.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"

static GtkWidget* entrySCFIterations = NULL;
static GtkWidget* comboSCF = NULL;
static GtkWidget* comboCorrelationMethod = NULL;
static GtkWidget* comboExchangeMethod = NULL;
static void setQChemCorrelationMethods();
/*************************************************************************************************************/
static gchar* listJobView[] = {
        "Single Point Energy", 
	"Equilibrium structure search", 
	"Frequencies",
	"Transition structure search", 
        "NMR Chemical Shift",
        "Intrinsic reaction pathway",
	"Forces", 
};
static gchar* listJobReal[] = {
        "SP", 
	"OPT", 
        "FREQ", 
	"TS", 
	"NMR",
	"RPATH",
	"FORCE"
};
static guint numberOfJobs = G_N_ELEMENTS (listJobView);
static gchar selectedJob[BSIZE]="SP";
/*************************************************************************************************************/
static gchar* listSCFView[] = { "default","unrestricted", "restricted"};
static gchar* listSCFReal[] = { "DEF", "TRUE", "FALSE"};
static guint numberOfSCF = G_N_ELEMENTS (listSCFView);
static gchar selectedSCF[BSIZE]="DEF";
/*************************************************************************************************************/
static gchar* listExchangeMethodsView[] = 
{ 
	"Hatree-Fock", 
	"Slater", 
	"Becke",
	"Gill96, Gill 1996",
	"GG99, Gilbert and Gill 1999 ",
	"Becke(EDF1), Becke (uses EDF1 parameters) ",
	"PW91, Perdew 91",
	"B3PW91,  hybrid Becke-HF-PW91",
	"B3LYP,  hybrid Becke-HF-LYP",
	"B3LYP5,  Original hybrid Becke-HF-LYP(using VWN5)",
	"EDF1",
	"EDF2",
	"BMK",
};
static gchar* listExchangeMethodsReal[] = 
{ 
	"HF", 
	"Slater", 
	"Becke",
	"Gill96",
	"GG99 ",
	"Becke(EDF1)"
	"PW91",
	"B3PW91",
	"B3LYP",
	"B3LYP5",
	"EDF1",
	"EDF2",
	"BMK",
};
static guint numberOfExchangeMethods = G_N_ELEMENTS (listExchangeMethodsView);
static gchar selectedExchangeMethod[BSIZE]="HF";
/*************************************************************************************************************/
static gchar* listCorrelationHybridMethodsView[] = 
{ 
	" ", 
};
static gchar* listCorrelationHybridMethodsReal[] = 
{ 
	"NONE", 
};
static guint numberOfCorrelationHybridMethods = G_N_ELEMENTS (listCorrelationHybridMethodsView);
/*************************************************************************************************************/
static gchar* listCorrelationDFTMethodsView[] = 
{ 
	"No correlation", 
	"VWN, Vosko-Wilk-Nusair parameterization",
	"LYP, Lee-Yang-Parr",
	"PW91, Perdew 91",
	"LYP(EDF1), parameterization",
	"Perdew86, Perdew 1986",
	"PZ81, Perdew-Zunger 1981",
	"Wigner",
};
static gchar* listCorrelationDFTMethodsReal[] = 
{ 
	"NONE", 
	"VWN",
	"LYP",
	"PW91",
	"LYP(EDF1)",
	"Perdew86",
	"PZ81",
	"Wigner",
};
static guint numberOfCorrelationDFTMethods = G_N_ELEMENTS (listCorrelationDFTMethodsView);
/*************************************************************************************************************/
static gchar* listCorrelationWFMethodsView[] = 
{ 
	"No correlation", 
	"MP2",
	"RIMP2",
	"Local MP2",
	"RILMP2",
	"ZAPT2",
	"SOSMP2",
	"MOSMP2",
	"MP3",
	"MP4",
	"MP4SDQ",
	"PP(2)",
	"IP",
	"RIP",
	"CCD",
	"CCD(2)",
	"CCSD",
	"CCSD(T)",
	"CCSD(2)",
	"QCISD",
	"QCISD(T)",
	"OD",
	"OD(T)",
	"OD(2)",
	"VOD",
	"VOD(2)",
	"QCCD",
	"VQCCD",
};
static gchar* listCorrelationWFMethodsReal[] = 
{ 
	"NONE", 
	"MP2",
	"RIMP2",
	"LMP2",
	"RILMP2",
	"ZAPT2",
	"SOSMP2",
	"MOSMP2",
	"MP3",
	"MP4",
	"MP4SDQ",
	"PP(2)",
	"IP",
	"RIP",
	"CCD",
	"CCD(2)",
	"CCSD",
	"CCSD(T)",
	"CCSD(2)",
	"QCISD",
	"QCISD(T)",
	"OD",
	"OD(T)",
	"OD(2)",
	"VOD",
	"VOD(2)",
	"QCCD",
	"VQCCD",
};
static guint numberOfCorrelationWFMethods = G_N_ELEMENTS (listCorrelationWFMethodsView);
/*************************************************************************************************************/
static gchar selectedCorrelationMethod[BSIZE]="NONE";
static gchar** listCorrelationMethodsView = listCorrelationHybridMethodsView;
static gchar** listCorrelationMethodsReal = listCorrelationHybridMethodsReal;
static guint numberOfCorrelationMethods = G_N_ELEMENTS (listCorrelationHybridMethodsView);
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
static void putQChemJobTypeInfoInTextEditor()
{
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " JobTyp  ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedJob,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
}
/*************************************************************************************************************/
static void putQChemSCFRemInfoInTextEditor()
{
  	G_CONST_RETURN gchar *entrytext;
	gint maxit=20;
	if(strcmp(selectedSCF,"DEF"))
	{
		if(!strcmp(selectedSCF,"TRUE"))
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " unrestricted  true\n",-1);
		else
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " unrestricted  false\n",-1);

	}
  	entrytext = gtk_entry_get_text(GTK_ENTRY(entrySCFIterations));
	maxit=atoi(entrytext);
	if(maxit==0)maxit=50;
	if(maxit!=50)
	{
        	gchar buffer[BSIZE];
		sprintf(buffer," max_scf_cycles   %d\n",maxit);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}

}
/*************************************************************************************************************/
static void putQChemExchangeInfoInTextEditor()
{
	if(strcmp(selectedExchangeMethod,"NONE")==0)return;
	else
	{
		gchar buffer[BSIZE];
		sprintf(buffer," exchange   %s\n",selectedExchangeMethod);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}
}
/*************************************************************************************************************/
static void putQChemCorrelationInfoInTextEditor()
{
	if(strcmp(selectedCorrelationMethod,"NONE")==0)return;
	else
	{
		gchar buffer[BSIZE];
		sprintf(buffer," correlation   %s\n",selectedCorrelationMethod);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}
}
/*************************************************************************************************************/
void putQChemRemInfoInTextEditor()
{
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &qchemColorFore.keyWord, &qchemColorBack.keyWord, "$rem\n",-1);
	putQChemJobTypeInfoInTextEditor();
	putQChemSCFRemInfoInTextEditor();
	putQChemExchangeInfoInTextEditor();
	putQChemCorrelationInfoInTextEditor();
	putQChemGuessInfoInTextEditor();
	putQChemBasisInfoInTextEditor();
	if(strstr(selectedCorrelationMethod,"RI")) putQChemAuxBasisInTextEditor();
	if(strstr(selectedCorrelationMethod,"OSMP2")) putQChemAuxBasisInTextEditor();
	if(strstr(selectedCorrelationMethod,"PP(2)")) putQChemAuxBasisInTextEditor();
	if(!strcmp(selectedCorrelationMethod,"IP")) putQChemAuxBasisInTextEditor();

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &qchemColorFore.keyWord, &qchemColorBack.keyWord, "$end\n\n",-1);
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
void setQChemSCFMethod(gboolean okRHF)
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
	
	if(entrySCFIterations)
		gtk_widget_set_sensitive(entrySCFIterations, TRUE);

	if(strstr(selectedSCF,"TRUE")) 
		setSensitiveQChemGuessMixed(TRUE);
	else
		setSensitiveQChemGuessMixed(FALSE);
	/* for(s=selectedSCF;*s != 0;s++) *s = toupper(*s);*/
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
static void setQChemCorrelationMethods()
{
	GtkTreeIter iter;
	GtkTreeIter iter0;
	gint i;

	if(strcmp(selectedExchangeMethod,"NONE")==0 || strstr(selectedExchangeMethod,"hybrid"))
	{
		listCorrelationMethodsView = listCorrelationHybridMethodsView;
		listCorrelationMethodsReal = listCorrelationHybridMethodsReal;
		numberOfCorrelationMethods = numberOfCorrelationHybridMethods;
	}
	else
	if(strcmp(selectedExchangeMethod,"HF")==0)
	{
		listCorrelationMethodsView = listCorrelationWFMethodsView;
		listCorrelationMethodsReal = listCorrelationWFMethodsReal;
		numberOfCorrelationMethods = numberOfCorrelationWFMethods;
	}
	else
	{
		listCorrelationMethodsView = listCorrelationDFTMethodsView;
		listCorrelationMethodsReal = listCorrelationDFTMethodsReal;
		numberOfCorrelationMethods = numberOfCorrelationDFTMethods;
	}
	
	if (comboCorrelationMethod && gtk_combo_box_get_active_iter (GTK_COMBO_BOX(comboCorrelationMethod), &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(comboCorrelationMethod));
        	GtkTreeStore *store = GTK_TREE_STORE(model);
		gtk_tree_store_clear(store);

		for(i=0;i<numberOfCorrelationMethods;i++)
		{
        		gtk_tree_store_append (store, &iter, NULL);
			if(i==0) iter0 = iter;
        		gtk_tree_store_set (store, &iter, 0, listCorrelationMethodsView[i], -1);
		}
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX (comboCorrelationMethod), &iter0);
	}

}
/************************************************************************************************************/
static void traitementExchangeMethod (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfExchangeMethods;i++)
	{
		if(strcmp((gchar*)data,listExchangeMethodsView[i])==0) res = listExchangeMethodsReal[i];
	}
	if(res) sprintf(selectedExchangeMethod,"%s",res);
	else  sprintf(selectedSCF,"NONE");

	setQChemCorrelationMethods();
}
/********************************************************************************************************/
static GtkWidget *create_list_exchange_methods()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfExchangeMethods;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listExchangeMethodsView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementExchangeMethod), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/*************************************************************************************************************/
static void traitementCorrMethod (GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	gchar* res = NULL;
	gint i;
	GtkWidget* wid;
	gboolean rimp2 = FALSE;

	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	else return;
	for(i=0;i<numberOfCorrelationMethods;i++)
	{
		if(strcmp((gchar*)data,listCorrelationMethodsView[i])==0) res = listCorrelationMethodsReal[i];
	}
	if(res) sprintf(selectedCorrelationMethod,"%s",res);
	else  sprintf(selectedCorrelationMethod,"NONE");
	if(strstr(selectedCorrelationMethod,"RILMP2")) rimp2 = TRUE;
	if(strstr(selectedCorrelationMethod,"RIMP2")) rimp2 = TRUE;
	if(strstr(selectedCorrelationMethod,"OSMP2")) rimp2 = TRUE;
	if(strstr(selectedCorrelationMethod,"PP(2)")) rimp2 = TRUE;
	if(!strcmp(selectedCorrelationMethod,"IP")) rimp2 = TRUE;
	wid  = g_object_get_data(G_OBJECT (combobox), "LabelAuxBasis1");
	if(wid) gtk_widget_set_sensitive(wid, rimp2);
	wid  = g_object_get_data(G_OBJECT (combobox), "LabelAuxBasis2");
	if(wid) gtk_widget_set_sensitive(wid, rimp2);
	wid  = g_object_get_data(G_OBJECT (combobox), "ComboAuxBasis");
	if(wid) gtk_widget_set_sensitive(wid, rimp2);
}
/********************************************************************************************************/
static GtkWidget *create_list_correlation_methods()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);


	for(i=0;i<numberOfCorrelationMethods;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listCorrelationMethodsView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementCorrMethod), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
void createQChemRemFrame(GtkWidget *win, GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* sep;
	GtkWidget* combo = NULL;
	gint l=0;
	gint c=0;
	gint ncases=1;
	GtkWidget *table = gtk_table_new(8,3,FALSE);

	entrySCFIterations = NULL;
	comboCorrelationMethod = NULL;
	comboExchangeMethod = NULL;

	listCorrelationMethodsView = listCorrelationHybridMethodsView;
	listCorrelationMethodsReal = listCorrelationHybridMethodsReal;
	numberOfCorrelationMethods = numberOfCorrelationHybridMethods;

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

	l++;
	c = 0; ncases=1;
	add_label_table(table,_("Max # SCF iterations"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	entrySCFIterations = gtk_entry_new ();
	gtk_entry_set_text(GTK_ENTRY(entrySCFIterations),"30");
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),entrySCFIterations,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);

	/*------------------ Exchange Type -----------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("Exchange Type"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_exchange_methods();
	comboExchangeMethod = combo;
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------ Correlation Method -----------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("Correlation Method"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_correlation_methods();
	comboCorrelationMethod = combo;
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	setQChemCorrelationMethods();
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
	addQChemBasisToTable(table, l);
	/*------------------ aux basis -----------------------------------------*/
	l++;
	addQChemAuxBasisToTable(table, l, comboCorrelationMethod);
	setQChemCorrelationMethods(); /* For activate sensitive aux basis */
}
