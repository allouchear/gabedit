/* FireFlyBasis.c */
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
#include "../FireFly/FireFlyTypes.h"
#include "../FireFly/FireFlyGlobal.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"

static GtkWidget *buttonDiffuseL = NULL;
static GtkWidget *buttonDiffuseS = NULL;
static GtkWidget *basisFrame = NULL;
static GtkWidget *comboBasisType = NULL;
/*************************************************************************************************************/
static gchar* listBasisSetView[] = { 
                            "MINI", "MIDI", "STO-2G", "STO-3G", "STO-4G", "STO-5G", "STO-6G",
                            "3-21G", "6-21G", "4-31G", "5-31G", "6-31G", "6-311G", "Double Zeta Valance",
                            "Dunning/Hay DZ", "Triple Zeta Valance", "Mclean/Chandler", "SBKJC Valance",
                            "Hay/Wadt Valance"
};
static gchar* listBasisSetReal[] = { 
	"GBASIS=MINI", "GBASIS=MIDI", "GBASIS=STO NGAUSS=2", "GBASIS=STO NGAUSS=3", "GBASIS=STO NGAUSS=4", 
	"GBASIS=STO NGAUSS=5", "GBASIS=STO NGAUSS=6", 
	"GBASIS=N21 NGAUSS=3", "GBASIS=N21 NGAUSS=6",
	"GBASIS=N31 NGAUSS=4", "GBASIS=N31 NGAUSS=5",
	"GBASIS=N31 NGAUSS=6", "GBASIS=N311 NGAUSS=6",
	"GBASIS=DZV", "GBASIS=DH", "GBASIS=TZV", "GBASIS=MC",
	"GBASIS=SBKJC",
	"GBASIS=HW"

};
static guint numberOfBasisSet = G_N_ELEMENTS (listBasisSetView);
static gchar selectedBasisSet[BSIZE]="MINI";
/*************************************************************************************************************/
static gchar* listBasisTypeView[] = {  N_("Default"),"Cartezian", "Spherical" };
static gchar* listBasisTypeReal[] = { "NONE","d5=.false.", "d5=.true." };
static guint numberOfBasisType = G_N_ELEMENTS (listBasisTypeView);
static gchar selectedBasisType[BSIZE]="NONE";
/*************************************************************************************************************/
static gchar* listBasisDPolView[] = {  "0","1", "2","3"};
static gchar* listBasisDPolReal[] = {  "NONE","NDFUNC=1", "NDFUNC=2","NDFUNC=3" };
static guint numberOfBasisDPol = G_N_ELEMENTS (listBasisDPolView);
static gchar selectedBasisDPol[BSIZE]="NONE";
/*************************************************************************************************************/
static gchar* listBasisFPolView[] = {  "0","1"};
static gchar* listBasisFPolReal[] = {  "NONE","NFFUNC=1"};
static guint numberOfBasisFPol = G_N_ELEMENTS (listBasisFPolView);
static gchar selectedBasisFPol[BSIZE]="NONE";
/*************************************************************************************************************/
static gchar* listBasisPPolView[] = {  "0","1", "2","3"};
static gchar* listBasisPPolReal[] = {  "NONE","NPFUNC=1", "NPFUNC=2","NPFUNC=3" };
static guint numberOfBasisPPol = G_N_ELEMENTS (listBasisPPolView);
static gchar selectedBasisPPol[BSIZE]="NONE";
/*************************************************************************************************************/
static gchar* listBasisPolarView[] = {  N_("Default"),"Pople", "Pople N311","Dunning","Huzinaga","Hondo7"};
static gchar* listBasisPolarReal[] = {  "NONE","POLAR=POPLE", "POLAR=POPN311","POLAR=DUNNING","POLAR=HUZINAGA","POLAR=HONDO7" };
static guint numberOfBasisPolar = G_N_ELEMENTS (listBasisPolarView);
static gchar selectedBasisPolar[BSIZE]="NONE";
/*************************************************************************************************************/
void initFireFlyBasisFrame()
{
	basisFrame = NULL;
}
/*************************************************************************************************************/
void setSensitiveFireFlyBasisFrame(gboolean sensitive)
{
	if(!basisFrame) return;
	gtk_widget_set_sensitive(basisFrame, sensitive);
}
/*************************************************************************************************************/
static void putFireFlyBasisDiffuseFunctions()
{
	if(!GTK_TOGGLE_BUTTON (buttonDiffuseL)->active 
	   && !GTK_TOGGLE_BUTTON (buttonDiffuseS)->active) return;
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$BASIS",-1);
	if(GTK_TOGGLE_BUTTON (buttonDiffuseL)->active)
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " DIFFSP=.TRUE.",-1);
	if(GTK_TOGGLE_BUTTON (buttonDiffuseS)->active)
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " DIFFS=.TRUE.",-1);

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);
}
/************************************************************************************************************/
static void putFireFlyBasisPolInfoInTextEditor()
{
	gchar* s=NULL;
	if(
		strcmp(selectedBasisDPol,"NONE")==0 
	     && strcmp(selectedBasisPPol,"NONE")==0 
	     && strcmp(selectedBasisPPol,"NONE")==0
	     && strcmp(selectedBasisPolar,"NONE")==0
	 ) return;
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$BASIS",-1);
	s=selectedBasisDPol;
	if(strcmp(s,"NONE")!=0)
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, s,-1);
	}
	s=selectedBasisFPol;
	if(strcmp(s,"NONE")!=0)
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, s,-1);
	}
	s=selectedBasisPPol;
	if(strcmp(s,"NONE")!=0)
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, s,-1);
	}
	s=selectedBasisPolar;
	if(strcmp(s,"NONE")!=0)
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, s,-1);
	}
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);
}
/*************************************************************************************************************/
static void putFireFlyBasisSetInfoInTextEditor()
{
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$BASIS",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedBasisSet,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);
	if(strcmp(selectedBasisSet,"GBASIS=SBKJC")==0)
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$CONTRL",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ECP=SBKJC",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);
	}
	if(strcmp(selectedBasisSet,"GBASIS=HW")==0)
	{
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$CONTRL",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ECP=HW",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);
	}
}
/*************************************************************************************************************/
static void putFireFlyBasisTypeInfoInTextEditor()
{
	if(strcmp(selectedBasisType,"NONE")==0)return;
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$CONTRL",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, selectedBasisType,-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &fireflyColorFore.keyWord, &fireflyColorBack.keyWord, "$END\n",-1);
}
/*************************************************************************************************************/
void putFireFlyBasisInfoInTextEditor()
{
	putFireFlyBasisSetInfoInTextEditor();
	putFireFlyBasisTypeInfoInTextEditor();
	putFireFlyBasisPolInfoInTextEditor();
	putFireFlyBasisDiffuseFunctions();
}
/************************************************************************************************************/
static void setFireFlyBasisType()
{
	GtkTreeIter iter;

	if (!comboBasisType) return;
	if (!gtk_combo_box_get_active_iter (GTK_COMBO_BOX(comboBasisType), &iter)) return;
	if (comboBasisType && GTK_IS_COMBO_BOX(comboBasisType))
	{
		if(strstr(selectedBasisSet,"CC") || strstr(selectedBasisSet,"PC") )
			gtk_combo_box_set_active(GTK_COMBO_BOX(comboBasisType), 2);
		else
			gtk_combo_box_set_active(GTK_COMBO_BOX(comboBasisType), 0);
	}
}
/************************************************************************************************************/
static void traitementBasisSet (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfBasisSet;i++)
	{
		if(strcmp((gchar*)data,listBasisSetView[i])==0) res = listBasisSetReal[i];
	}
	if(res) sprintf(selectedBasisSet,"%s",res);
	else  sprintf(selectedBasisSet,"MINI");
	setFireFlyBasisType();
	
	/* for(s=selectedBasisSet;*s != 0;s++) *s = toupper(*s);*/
}
/********************************************************************************************************/
static GtkWidget *create_list_basisset()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfBasisSet;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listBasisSetView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementBasisSet), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
static void traitementBasisType (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfBasisType;i++)
	{
		if(strcmp((gchar*)data,listBasisTypeView[i])==0) res = listBasisTypeReal[i];
	}
	if(res) sprintf(selectedBasisType,"%s",res);
	else  sprintf(selectedBasisType,"NONE");
	
	/* for(s=selectedBasisType;*s != 0;s++) *s = toupper(*s);*/
}
/********************************************************************************************************/
static GtkWidget *create_list_basistype()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfBasisType;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listBasisTypeView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementBasisType), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
static void traitementBasisDPol (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfBasisDPol;i++)
	{
		if(strcmp((gchar*)data,listBasisDPolView[i])==0) res = listBasisDPolReal[i];
	}
	if(res) sprintf(selectedBasisDPol,"%s",res);
	else  sprintf(selectedBasisDPol,"NONE");
	
	/* for(s=selectedBasisDPol;*s != 0;s++) *s = toupper(*s);*/
}
/********************************************************************************************************/
static GtkWidget *create_list_BasisDPol()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfBasisDPol;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listBasisDPolView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementBasisDPol), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
static void traitementBasisFPol (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfBasisFPol;i++)
	{
		if(strcmp((gchar*)data,listBasisFPolView[i])==0) res = listBasisFPolReal[i];
	}
	if(res) sprintf(selectedBasisFPol,"%s",res);
	else  sprintf(selectedBasisFPol,"NONE");
	
	/* for(s=selectedBasisFPol;*s != 0;s++) *s = toupper(*s);*/
}
/********************************************************************************************************/
static GtkWidget *create_list_BasisFPol()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfBasisFPol;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listBasisFPolView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementBasisFPol), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
static void traitementBasisPPol (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfBasisPPol;i++)
	{
		if(strcmp((gchar*)data,listBasisPPolView[i])==0) res = listBasisPPolReal[i];
	}
	if(res) sprintf(selectedBasisPPol,"%s",res);
	else  sprintf(selectedBasisPPol,"NONE");
	
	/* for(s=selectedBasisPPol;*s != 0;s++) *s = toupper(*s);*/
}
/********************************************************************************************************/
static GtkWidget *create_list_BasisPPol()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfBasisPPol;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listBasisPPolView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementBasisPPol), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
static void traitementBasisPolar (GtkComboBox *combobox, gpointer d)
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
	for(i=0;i<numberOfBasisPolar;i++)
	{
		if(strcmp((gchar*)data,listBasisPolarView[i])==0) res = listBasisPolarReal[i];
	}
	if(res) sprintf(selectedBasisPolar,"%s",res);
	else  sprintf(selectedBasisPolar,"NONE");
	
	/* for(s=selectedBasisPolar;*s != 0;s++) *s = toupper(*s);*/
}
/********************************************************************************************************/
static GtkWidget *create_list_BasisPolar()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfBasisPolar;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listBasisPolarView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementBasisPolar), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
void createFireFlyBasisFrame(GtkWidget *win, GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* sep;
	GtkWidget* combo = NULL;
	gint l=0;
	gint c=0;
	gint ncases=1;
	GtkWidget *table = gtk_table_new(9,3,FALSE);

	buttonDiffuseL = NULL;
	buttonDiffuseS = NULL;
	comboBasisType = NULL;

	frame = gtk_frame_new (_("Basis"));
	basisFrame = frame;
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);
	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	/*------------------ Basis set -----------------------------------------*/
	l=0; 
	c = 0; ncases=1;
	add_label_table(table,_("Basis set"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_basisset();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------ Basis set -----------------------------------------*/
	l++; 
	c = 0; ncases=1;
	add_label_table(table,_("Type"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_basistype();
	comboBasisType = combo;
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
	/*------------------ #D heavy atom polarization functions ------------*/
	l++; 
	c = 0; ncases=1;
	add_label_table(table,_("#D heavy atom polarization functions"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_BasisDPol();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------ #F heavy atom polarization functions ------------*/
	l++; 
	c = 0; ncases=1;
	add_label_table(table,_("#F heavy atom polarization functions"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_BasisFPol();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------ ##light atom polarization functions ------------*/
	l++; 
	c = 0; ncases=1;
	add_label_table(table,_("#light atom polarization functions"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_BasisPPol();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------ Polar ------------*/
	l++; 
	c = 0; ncases=1;
	add_label_table(table,_("Polar"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_BasisPolar();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------ Diifuse L ------------*/
	l++; 
	c = 0; ncases=3;
	buttonDiffuseL = gtk_check_button_new_with_label (_("Diffuse L-shell on heavy atoms"));
	gtk_table_attach(GTK_TABLE(table),buttonDiffuseL,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonDiffuseL), FALSE);
	/*------------------ Diifuse S ------------*/
	l++; 
	c = 0; ncases=3;
	buttonDiffuseS = gtk_check_button_new_with_label (_("Diffuse s-shell on hydrogens "));
	gtk_table_attach(GTK_TABLE(table),buttonDiffuseS,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonDiffuseS), FALSE);
}
