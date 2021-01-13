/* GamessRunControl.c */
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
#include "../Gamess/GamessTypes.h"
#include "../Gamess/GamessGlobal.h"
#include "../Gamess/GamessMolecule.h"
#include "../Gamess/GamessBasis.h"
#include "../Gamess/GamessGuess.h"
#include "../Gamess/GamessSCF.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"

static GtkWidget* entrySCFIterations = NULL;
static GtkWidget* comboSCF = NULL;
static GtkWidget* comboCorrMethod = NULL;
static GtkWidget* comboCorrType = NULL;
static void setGamessCorrType();
/*************************************************************************************************************/
static gchar* listRunView[] = {
        N_("Single Point Energy"), N_("Equilibrium geometry"), N_("Transition State"), N_("Frequencies"),
        "Gradient", "Trudge", "IRC", "VSCF", "DRC", "GlobOp", "OptFMO", "GradExtr",
        "Surface", "Drop", "Raman", "NMR", "Morokuma", "Transitn", "FField", "TDHF", "MakeFP"
};
static gchar* listRunReal[] = {
        "Energy", "Optimize", "Sadpoint", "Hessian",
        "Gradient", "Trudge", "IRC", "VSCF", "DRC", "GlobOp", "OptFMO", "GradExtr",
        "Surface", "Drop", "Raman", "NMR", "Morokuma", "Transitn", "FField", "TDHF", "MakeFP"
};
static guint numberOfRuns = G_N_ELEMENTS (listRunView);
static gchar selectedRun[BSIZE]="ENERGY";
/*************************************************************************************************************/
static gchar* listSCFView[] = { "RHF", "UHF", "ROHF", "GVB", "MCSCF", "MNDO", "AM1", "PM3", "None" };
static gchar* listSCFReal[] = { "RHF", "UHF", "ROHF", "GVB", "MCSCF", "MNDO", "AM1", "PM3", "NONE" };
static guint numberOfSCF = G_N_ELEMENTS (listSCFView);
static gchar selectedSCF[BSIZE]="RHF";
/*************************************************************************************************************/
static gchar* listCorrTypeView[] = { N_("None"), N_("Density Functional Theory"), N_("MP2"), N_("Coupled-Cluster")};
static gchar* listCorrTypeReal[] = { "NONE", "DFTTYP", "MPLEVL=2","CCTYP"};
static guint numberOfCorrType = G_N_ELEMENTS (listCorrTypeView);
static gchar selectedCorrType[BSIZE]="NONE";
/*************************************************************************************************************/
static gchar* listNoneView[] = { N_("None")};
static gchar* listNoneReal[] = { "NONE"};
static guint numberOfNone = G_N_ELEMENTS (listNoneView);

static gchar* listMP2View[] = { N_("MP2")};
static gchar* listMP2Real[] = { "MP2"};
static guint numberOfMP2 = G_N_ELEMENTS (listMP2View);

static gchar* listDFTView[] = { 
	N_("B3LYP"),
	N_("BHHLYP (HF and BECKE exchange + LYP Correlation)"),

	N_("BLYP (Becke exchange + LYP Correlation)"), 
	N_("BVWN (Becke exchange + VWN5 Correlation)"), 
	N_("BOP (Becke exchange + OP Correlation)"), 

	N_("GLYP (GILL exchange + LYP Correlation)"), 
	N_("GVWN (GILL exchange + VWN5 Correlation)"), 
	N_("GOP (GILL exchange + OP Correlation)"), 

	N_("PBELYP (PBE exchange + LYP Correlation)"), 
	N_("PBEVWN (PBE exchange + VWN5 Correlation)"), 
	N_("PBEOP (PBE exchange + OP Correlation)"), 

	N_("SLYP (Slater exchange + LYP Correlation)"), 
	N_("SVWN (Slater exchange + VWN5 Correlation)"),
	N_("SOP (Slater exchange + OP Correlation)"), 

	N_("SLATER (Exchange)"), 
	N_("BECKE 1988 (Exchange)"),
	N_("GILL  1996 (Exchange)"), 
	N_("PBE (Exchange only)"),

	N_("VWN (VWN5 Correlation)"), 
	N_("LYP (Correlation)"),
	N_("OP (Correlation)")
};
static gchar* listDFTReal[] = {
	"B3LYP", "BHHLYP", "BLYP", "BVWN", "BOP", "GLYP", "GVWN", "GOP", "PBELYP", "PBEVWN", "PBEOP", 
	"SLYP", "SVWN", "SOP", "SLATER", "BECKE", "GILL", "PBE", "VWN", "LYP", "OP",
};
static guint numberOfDFT = G_N_ELEMENTS (listDFTView);

static gchar* listCCView[] = { "LCCD", "CCD", "CCSD", "CCSD(T)","R-CC","CR-CC","CCSD(TQ)","CR-CC(Q)","EOM-CCSD","CR-EOM" };
static gchar* listCCReal[] = { "LCCD", "CCD", "CCSD", "CCSD(T)","R-CC","CR-CC","CCSD(TQ)","CR-CC(Q)","EOM-CCSD","CR-EOM" };
static guint numberOfCC = G_N_ELEMENTS (listCCView);

static guint numberOfCorrMethods = 0;
static gchar** listCorrMethodsView = NULL;
static gchar** listCorrMethodsReal = NULL;
static gchar selectedCorrMethod[BSIZE]="NONE";
/*************************************************************************************************************/
static gchar* listLocalizedView[] = { N_("None"), "Foster-Boys", "Edmiston-Ruedenberg","Pipek-Mezey"};
static gchar* listLocalizedReal[] = { "NONE", "BOYS", "RUEDNBRG","POP" };
static guint numberOfLocalized = G_N_ELEMENTS (listLocalizedView);
static gchar selectedLocalized[BSIZE]="NONE";
/*************************************************************************************************************/
static gchar* listEXEView[] = { N_("Normal Run"), N_("Check"), N_("Debug")};
static gchar* listEXEReal[] = { "NORMAL", "CHECK", "DEBUG" };
static guint numberOfEXE = G_N_ELEMENTS (listEXEView);
static gchar selectedEXE[BSIZE]="NORMAL";
/*************************************************************************************************************/
gboolean gamessSemiEmperical()
{
	if(
		strcmp(selectedSCF,"MNDO")==0 ||
		strcmp(selectedSCF,"AM1")==0 ||
		strcmp(selectedSCF,"PM3")==0
	  )return TRUE;
	else return FALSE;
}
/*************************************************************************************************************/
static gchar* calculWord(gchar* view)
{
	gint i;
	for(i=0;i<numberOfRuns;i++)
	{
		if(strcmp(view,listRunView[i])==0)return listRunReal[i];
	}
	return NULL;
}
/*************************************************************************************************************/
static void putGamessRunTypeInfoInTextEditor()
{

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$SYSTEM",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " MWORDS=20 ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$END\n",-1);

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$CONTRL",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " RUNTYP=",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedRun,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$END\n",-1);

	if(!strcmp(selectedRun,"Optimize") || !strcmp(selectedRun,"Sadpoint"))
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$STATPT",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "  OptTol=1e-5 NStep=500 ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$END\n",-1);
	}
	if(!strcmp(selectedRun,"IRC"))
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$IRC",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "  NPOINT=10 ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$END\n",-1);
	}
	if(!strcmp(selectedRun,"Raman") || !strcmp(selectedRun,"Sadpoint") || !strcmp(selectedRun,"IRC"))
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, 
			_("----> Put here the $HESS card.\n      You can obtain it from your old frequecncies calculation(.pun or .irc file)\n"),-1);
	}
}
/*************************************************************************************************************/
static void putGamessSCFControlInfoInTextEditor()
{
  	G_CONST_RETURN gchar *entrytext;
	gint maxit=20;
	if(strcmp(selectedSCF,"NONE")==0)return;
	
	if(strcmp(selectedSCF,"MNDO")==0 || strcmp(selectedSCF,"AM1")==0 || strcmp(selectedSCF,"PM3")==0)
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$BASIS",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " GBASIS=",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedSCF,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$END\n",-1);
		if(getGamessMultiplicity()!=1)
		{
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$CONTRL",-1);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " SCFTYP=ROHF ",-1);
        		gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$END\n",-1);
		}
	}
	else
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$CONTRL",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " SCFTYP=",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedSCF,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$END\n",-1);
	}
  	entrytext = gtk_entry_get_text(GTK_ENTRY(entrySCFIterations));
	maxit=atoi(entrytext);
	if(maxit==0)maxit=30;
	if(maxit!=30)
	{
        	gchar buffer[BSIZE];
		sprintf(buffer," MAXIT=%d ",maxit);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$CONTRL",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$END\n",-1);
	}

}
/*************************************************************************************************************/
static void putGamessCorrelationInfoInTextEditor()
{
	if(strcmp(selectedCorrType,"NONE")==0)return;
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
	if(strcmp(selectedCorrType,"DFTTYP")==0)
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$DFT",-1);
	else
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$CONTRL",-1);

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);

	if(strcmp(selectedCorrType,"MPLEVL=2")==0)
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedCorrType,-1);
	if(strcmp(selectedCorrType,"CCTYP")==0)
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " CCTYP=",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedCorrMethod,-1);
	}
	if(strcmp(selectedCorrType,"DFTTYP")==0)
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " DFTTYP=",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedCorrMethod,-1);
	}
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$END\n",-1);
}
/*************************************************************************************************************/
static void putGamessLocalizedInfoInTextEditor()
{
	if(strcmp(selectedLocalized,"NONE")==0)return;
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$CONTRL",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " LOCAL=",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedLocalized,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$END\n",-1);
}
/*************************************************************************************************************/
static void putGamessEXEInfoInTextEditor()
{
	if(strcmp(selectedEXE,"NORMAL")==0)return;
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$CONTRL",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " EXETYP=",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedEXE,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$END\n",-1);
}
/*************************************************************************************************************/
void putGamessControlInfoInTextEditor()
{
	putGamessRunTypeInfoInTextEditor();
	putGamessSCFControlInfoInTextEditor();
	putGamessCorrelationInfoInTextEditor();
	putGamessLocalizedInfoInTextEditor();
	putGamessEXEInfoInTextEditor();
}
/************************************************************************************************************/
static void traitementRunType (GtkComboBox *combobox, gpointer d)
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
	if(res) sprintf(selectedRun,"%s",res);
	else  sprintf(selectedRun,"Energy");
	setGamessCorrType();

}
/********************************************************************************************************/
static GtkWidget *create_list_runtype()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfRuns;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listRunView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementRunType), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
void setGamessSCFMethod(gboolean okRHF)
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
			if(strcmp(listSCFView[i],"RHF")==0)continue;
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
	gboolean sm=FALSE;
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
	else  sprintf(selectedSCF,"RHF");
	
	if(entrySCFIterations)
	{
		if(strcmp(selectedSCF,"NONE")==0 && entrySCFIterations) 
			gtk_widget_set_sensitive(entrySCFIterations, FALSE);
		else 
			gtk_widget_set_sensitive(entrySCFIterations, TRUE);
	}
	sm = gamessSemiEmperical();
	setSensitiveGamessBasisFrame(!sm);
	setSensitiveGamessSCFFrame(!sm);
	setSensitiveGamessGuessFrame(!sm);
	setGamessCorrType();
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
static void setGamessCorrMethod()
{
	GtkTreeIter iter;
	GtkTreeIter iter0;
	gint i;

	if(strcmp(selectedCorrType,"MPLEVL=2")==0)
	{
		numberOfCorrMethods = numberOfMP2;
		listCorrMethodsView = listMP2View;
		listCorrMethodsReal = listMP2Real;
	}
	else
	if(strcmp(selectedCorrType,"DFTTYP")==0)
	{
		numberOfCorrMethods = numberOfDFT;
		listCorrMethodsView = listDFTView;
		listCorrMethodsReal = listDFTReal;
	}
	else
	if(strcmp(selectedCorrType,"CCTYP")==0)
	{
		numberOfCorrMethods = numberOfCC;
		listCorrMethodsView = listCCView;
		listCorrMethodsReal = listCCReal;
	}
	else
	{
		numberOfCorrMethods = numberOfNone;
		listCorrMethodsView = listNoneView;
		listCorrMethodsReal = listNoneReal;
	}
	
	if (comboCorrMethod && gtk_combo_box_get_active_iter (GTK_COMBO_BOX(comboCorrMethod), &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(comboCorrMethod));
        	GtkTreeStore *store = GTK_TREE_STORE(model);
		gtk_tree_store_clear(store);

		for(i=0;i<numberOfCorrMethods;i++)
		{
        		gtk_tree_store_append (store, &iter, NULL);
			if(i==0) iter0 = iter;
        		gtk_tree_store_set (store, &iter, 0, listCorrMethodsView[i], -1);
		}
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX (comboCorrMethod), &iter0);
	}
}
/************************************************************************************************************/
static void setGamessCorrType()
{
	GtkTreeIter iter;
	GtkTreeIter iter0;
	gint i;
	gboolean sm = gamessSemiEmperical();

	if (comboCorrType && GTK_IS_COMBO_BOX(comboCorrType))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(GTK_COMBO_BOX(comboCorrType));
        	GtkTreeStore *store = GTK_TREE_STORE(model);
		gtk_tree_store_clear(store);

		for(i=0;i<numberOfCorrType;i++)
		{
			if(strcmp(selectedSCF,"RHF")==0 ||strcmp(selectedSCF,"NONE")==0 )
			{
				if(
				strcmp(selectedRun,"Energy")!=0 && 
				strcmp(selectedRun,"Trudge")!=0 && 
				strcmp(selectedRun,"Surface")!=0 && 
				strcmp(selectedRun,"FField")!=0 && 
				strcmp(listCorrTypeReal[i],"CCTYP")==0
						) continue;
				else
				{
        				gtk_tree_store_append (store, &iter, NULL);
					if(i==0) iter0 = iter;
        				gtk_tree_store_set (store, &iter, 0, listCorrTypeView[i], -1);
				}
			}
			else
			{
				if( strcmp(listCorrTypeReal[i],"CCTYP")==0) continue;
				else 
				{
        				gtk_tree_store_append (store, &iter, NULL);
					if(i==0) iter0 = iter;
					gtk_tree_store_set (store, &iter, 0, listCorrTypeView[i], -1);
				}
			}
			if(i==0 && sm) break;


		}
		gtk_combo_box_set_active_iter(GTK_COMBO_BOX (comboCorrType), &iter0);
	}
	setGamessCorrMethod();
}
/************************************************************************************************************/
static void traitementCorrType (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfCorrType;i++)
	{
		if(strcmp((gchar*)data,listCorrTypeView[i])==0) res = listCorrTypeReal[i];
	}
	if(res) sprintf(selectedCorrType,"%s",res);
	else  sprintf(selectedSCF,"NONE");

	setGamessCorrMethod();
}
/********************************************************************************************************/
static GtkWidget *create_list_corrtype()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfCorrType;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listCorrTypeView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementCorrType), NULL);
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

	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	else return;
	for(i=0;i<numberOfCorrMethods;i++)
	{
		if(strcmp((gchar*)data,listCorrMethodsView[i])==0) res = listCorrMethodsReal[i];
	}
	if(res) sprintf(selectedCorrMethod,"%s",res);
	else  sprintf(selectedSCF,"NONE");
}
/********************************************************************************************************/
static GtkWidget *create_list_corrmethod()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);


	for(i=0;i<numberOfCorrMethods;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listCorrMethodsView[i], -1);
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
static void traitementLocalized (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfLocalized;i++)
	{
		if(strcmp((gchar*)data,listLocalizedView[i])==0) res = listLocalizedReal[i];
	}
	if(res) sprintf(selectedLocalized,"%s",res);
	else  sprintf(selectedSCF,"NONE");
	/* for(s=selectedLocalized;*s != 0;s++) *s = toupper(*s);*/
}
/********************************************************************************************************/
static GtkWidget *create_list_localized()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfLocalized;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listLocalizedView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementLocalized), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
static void traitementEXE (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfEXE;i++)
	{
		if(strcmp((gchar*)data,listEXEView[i])==0) res = listEXEReal[i];
	}
	if(res) sprintf(selectedEXE,"%s",res);
	else  sprintf(selectedSCF,"NORMAL");
	/* for(s=selectedEXE;*s != 0;s++) *s = toupper(*s);*/
}
/********************************************************************************************************/
static GtkWidget *create_list_exe()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfEXE;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listEXEView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementEXE), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
void createGamessControlFrame(GtkWidget *win, GtkWidget *box)
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
	comboCorrMethod = NULL;
	comboCorrType = NULL;

	numberOfCorrMethods = numberOfNone;
	listCorrMethodsView = listNoneView;
	listCorrMethodsReal = listNoneReal;

	frame = gtk_frame_new (_("Control"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);
	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	/*------------------ Run Type -----------------------------------------*/
	l=0; 
	c = 0; ncases=1;
	add_label_table(table,_("Run Type"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_runtype();
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

	/*------------------ Correlation Type -----------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("Correlation Type"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_corrtype();
	comboCorrType = combo;
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
	combo = create_list_corrmethod();
	comboCorrMethod = combo;
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	setGamessCorrType();
	/*------------------ Localized Type -----------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("Localized Type"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_localized();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);

	/*------------------ separator -----------------------------------------*/
	l++;
	sep = gtk_hseparator_new ();;
	c = 0; ncases=3;
	gtk_table_attach(GTK_TABLE(table),sep,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------ EXE Type -----------------------------------------*/
	l++;
	c = 0; ncases=1;
	add_label_table(table,_("EXE Type"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_exe();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
}
