/* DeMonBasis.c */
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

#include "../../Config.h"
#include "../Common/Global.h"
#include "../DeMon/DeMonTypes.h"
#include "../DeMon/DeMonGlobal.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomConversion.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"

static gchar selectedTypeBasis[BSIZE]="";
static gchar** listBasisReal = NULL;
static gchar** listBasisView = NULL;
static gint numberOfBasis = 0;
/*************************************************************************************************************/
static gchar* listTypeBasisView[] = 
{ 
	"Pople Style basis sets", 
	"Pople with one diffuse function", 
	"Dunning basis sets", 
	"Miscellenous and Specialized Basis Sets", 
};
static gchar* listTypeBasisReal[] = 
{ 
	"Pople", 
	"PopleDiffuseAllAtoms", 
	"Dunning", 
	"Miscellenous", 
};
static guint numberOfTypeBasis = G_N_ELEMENTS (listTypeBasisView);
/*************************************************************************************************************/
static gchar* listPopleBasisView[] = 
{ 
	"DZVP 	   H-Xe LDA double zeta polarizationa basis set",
	"DZVP-GGA  H-Xe GGA double zeta polarizationa basis set",
	"DZVP2 	   Be-F, Al-Ar, Sc-Zn   Modified DZVP basis set.",
	"TZVP 	   H, Li, C-F, Si-Cl  LDA triple zeta polarization basis set.",
	"TZVP-GGA  Sc-Cu              GGA triple zeta polarization basis set. ",
	"TZVP-FIP1 H, C-F, Na, S, Cl, Cu TZVP with field-induced polarization",
	"TZVP-FIP2 H, C-F, Na, S, Cl, Cu for  alpha, beta (FIP1) and gamma (FIP2) calculations",
	"DZV      H,C 	LDA double zeta basis set.",
	"DZV-GGA  H,C 	GGA double zeta basis set.",
};
static gchar* listPopleBasisReal[] = 
{ 
	"(DZVP)",
	"(DZVP-GGA)",
	"(DZVP2)",
	"(TZVP)",
	"(TZVP-GGA)",
	"(TZVP-FIP1)",
	"(TZVP-FIP2)",
	"(DZV)",
	"(DZV-GGA)",
};
static guint numberOfPopleBasis = G_N_ELEMENTS (listPopleBasisView);
/*************************************************************************************************************/
/*************************************************************************************************************/
static gchar* listDunningBasisView[] = 
{ 
	"cc-pVDZ Dunning correlation concisistent polarized double zeta",
	"Aug-cc-pVDZ Same but including diffuse functions",
	"cc-pVTZ Dunning correlation concisistent polarized triple zeta",
	"Aug-cc-pVTZ Same but including diffuse functions(g-functions deleted!)",
	"cc-pVQZ Dunning correlation concisistent polarized quadruple zeta",
	"Aug-cc-pVQZ with diffuse functions",
	"cc-pV5Z Dunning correlation concisistent polarized quintuple zeta",
	"Aug-cc-pV5Z with diffuse functions",
	"cc-pV6Z Dunning correlation concisistent polarized sextuple zeta",
	"Aug-cc-pV6Z ... with diffuse functions",
};
static gchar* listDunningBasisReal[] = 
{ 
	"(cc-pVDZ)",
	"(Aug-cc-pVDZ)",
	"(cc-pVTZ)",
	"(Aug-cc-pVTZ)",
	"(cc-pVQZ)",
	"(Aug-cc-pVQZ)",
	"(cc-pV5Z)",
	"(Aug-cc-pV5Z)",
	"(cc-pV6Z)",
	"(Aug-cc-pV6Z)",
};
static guint numberOfDunningBasis = G_N_ELEMENTS (listDunningBasisView);
/*************************************************************************************************************/
static gchar* listMiscellenousBasisView[] = 
{ 
	"EPR-III 	H-F 	EPR basis set",
	"IGLO-II 	H, Li, B-F, Si 	NMR basis set",
	"IGLO-III 	H, B-F, Si, Cr, Fe 	NMR basis set",
	"STO-3G 	H-Ar 	STO-3G basis set for testing only",
	"SAD 	H, C-F 	Sadlej FIP basis set",
	"LIC 	H-Ne 	Lie-Clementi basis set",
	"WACHTERS 	Sc-Cu 	Wachters basis set without f functions",
	"DZ-ANO 	H-Zn 	Double zeta ANO basis set from Roos",
	"ECP|SD 	Valence basis for SD ECPs",
	"RECP|SD 	Valence basis for SD RECPs",
	"QECP|SD 	Valence basis for SD QECPs",
	"ECP|LANL2DZ 	Valence basis for LANL ECPs",
	"QECP|LANL2DZ 	Valence basis for LANL QECPs",
	"ECP|HW 	K-Cu 	Hay-Wadt basis for LANL ECPs",
	"QECP|HW 	Rb-Ag, Cs-La, Hf-Au 	Hay-Wadt basis for LANL QECPs",
	"MCP|LK 	Valence basis for LK MCPs",
	"RMCP|LK 	Valence basis for LK RMCPs",
	"XAS-I 	Li-F 	XAS augmentation basis for first row",
	"XAS-II 	Na-Cl 	XAS augmentation basis for second row"
};
static gchar* listMiscellenousBasisReal[] = 
{ 
	"(EPR-III)",
	"(IGLO-II)",
	"(IGLO-III)",
	"(STO-3G)",
	"(SAD)",
	"(LIC)",
	"(WACHTERS)",
	"(DZ-ANO)",
	"(ECP|SD)",
	"(RECP|SD)",
	"(QECP|SD)",
	"(ECP|LANL2DZ)",
	"(QECP|LANL2DZ)",
	"(ECP|HW)",
	"(QECP|HW)",
	"(MCP|LK)",
	"(RMCP|LK)",
	"(XAS-I)",
	"(XAS-II)",
};
static guint numberOfMiscellenousBasis = G_N_ELEMENTS (listMiscellenousBasisView);
/*************************************************************************************************************/
static gchar* listAuxBasisView[] = 
{ 
	"GEN-A2*",
	"A2 auxiliary function",
        "GEN-A2",
	"GEN-A2**",
	"GEN-A3",
	"GEN-A3*",
	"GEN-A3**",
	"GEN-A4",
	"GEN-A4*",
	"GEN-A4**"
};
static gchar* listAuxBasisReal[] = 
{ 
	"(GEN-A2*)",
	"(A2)",
	"(GEN-A2)",
	"(GEN-A2**)",
	"(GEN-A3)",
	"(GEN-A3*)",
	"(GEN-A3**)",
	"(GEN-A4)",
	"(GEN-A4*)",
	"(GEN-A4**)",
};
static guint numberOfAuxBasis = G_N_ELEMENTS (listAuxBasisView);
/*************************************************************************************************************/
static void setDeMonBasis(GtkWidget* comboBasis)
{
	gint i;

	if(strcmp(selectedTypeBasis, "Pople")==0)
	{
		listBasisView = listPopleBasisView;
		listBasisReal = listPopleBasisReal;
		numberOfBasis = numberOfPopleBasis;
	}
	else if(strcmp(selectedTypeBasis, "Dunning")==0)
	{
		listBasisView = listDunningBasisView;
		listBasisReal = listDunningBasisReal;
		numberOfBasis = numberOfDunningBasis;
	}
	else 
	{
		listBasisView = listMiscellenousBasisView;
		listBasisReal = listMiscellenousBasisReal;
		numberOfBasis = numberOfMiscellenousBasis;
	}
	
	if (comboBasis && GTK_IS_COMBO_BOX(comboBasis))
	{
		GtkTreeModel * model = NULL;
		model = gtk_combo_box_get_model(GTK_COMBO_BOX(comboBasis));
		gtk_list_store_clear(GTK_LIST_STORE(model));

		for(i=0;i<numberOfBasis;i++)
  			gtk_combo_box_append_text (GTK_COMBO_BOX (comboBasis), listBasisView[i]);
  		gtk_combo_box_set_active(GTK_COMBO_BOX (comboBasis), 0);
	}

}
/************************************************************************************************************/
static void traitementTypeBasis (GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	gchar* res = NULL;
	gint i;
	GtkWidget* comboBasis = NULL;
	/* gchar* s;*/
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	else return;
	for(i=0;i<numberOfTypeBasis;i++)
	{
		if(strcmp((gchar*)data,listTypeBasisView[i])==0) res = listTypeBasisReal[i];
	}
	if(res) sprintf(selectedTypeBasis,"%s",res);
	comboBasis = g_object_get_data(G_OBJECT (combobox), "ComboBasis");

	if(comboBasis) setDeMonBasis(comboBasis);
}
/********************************************************************************************************/
static GtkWidget *create_list_type_basis(GtkWidget *comboBasis)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfTypeBasis;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listTypeBasisView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_set_data(G_OBJECT (combobox), "ComboBasis",comboBasis);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementTypeBasis), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
void initDeMonBasis()
{
	demonBasis.name = g_strdup("3-21G");
	demonBasis.molecule = g_strdup("$:molecule");
	demonBasis.numberOfBasisTypes=0;
	demonBasis.basisNames = NULL;
}
/************************************************************************************************************/
void freeDeMonBasis()
{
	if(demonBasis.name ) g_free(demonBasis.name);
	demonBasis.name = NULL;
	if(demonBasis.molecule ) g_free(demonBasis.molecule);
	demonBasis.molecule = NULL;
	if(demonBasis.basisNames)
	{
		gint i;
		for(i=0;i<demonBasis.numberOfBasisTypes;i++)
			if(demonBasis.basisNames[i])g_free(demonBasis.basisNames[i]);

		if(demonBasis.basisNames)g_free(demonBasis.basisNames);
	}
	demonBasis.numberOfBasisTypes=0;
	demonBasis.basisNames = NULL;
}
/************************************************************************************************************/
static void changedEntryBasis(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gchar* res = NULL;
	gint i;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	if(demonBasis.name) g_free(demonBasis.name);
	for(i=0;i<numberOfBasis;i++)
	{
		if(strcmp((gchar*)entryText,listBasisView[i])==0) res = listBasisReal[i];
	}
	if(res) demonBasis.name=g_strdup(res);
}
/************************************************************************************************************/
static void changedEntryAuxBasis(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gint i;
	gchar* res = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	if(demonMolecule.auxBasisName) g_free(demonMolecule.auxBasisName);
	for(i=0;i<numberOfAuxBasis;i++)
	{
		if(strcmp((gchar*)entryText,listAuxBasisView[i])==0) res = listAuxBasisReal[i];
	}
	if(res) demonMolecule.auxBasisName=g_strdup(res);
}
/***********************************************************************************************/
static GtkWidget* addComboListToATable(GtkWidget* table,
		gchar** list, gint nlist, gint i, gint j, gint k)
{
	GtkWidget *entry = NULL;
	GtkWidget *combo = NULL;

	combo = create_combo_box_entry(list, nlist, TRUE, -1, -1);

	gtk_table_attach(GTK_TABLE(table),combo,j,j+k,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	entry = GTK_BIN (combo)->child;
	g_object_set_data(G_OBJECT (entry), "Combo",combo);

	return entry;
}
/***********************************************************************************************/
void addDeMonTypeBasisToTable(GtkWidget *table, gint i, GtkWidget* comboBasis)
{
	GtkWidget* combo = NULL;

	add_label_table(table,_("Type"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	combo  = create_list_type_basis(comboBasis);
	gtk_table_attach(GTK_TABLE(table),combo,2,2+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
}
/***********************************************************************************************/
GtkWidget* addDeMonBasisToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryBasis = NULL;
	GtkWidget* comboBasis = NULL;
	gint nlistBasis = 1;
	gchar* listBasis[] = {" "};


	add_label_table(table,_("Basis"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryBasis = addComboListToATable(table, listBasis, nlistBasis, i, 2, 1);
	comboBasis  = g_object_get_data(G_OBJECT (entryBasis), "Combo");
	gtk_widget_set_sensitive(entryBasis, TRUE);
	gtk_editable_set_editable(GTK_EDITABLE(entryBasis),FALSE);

	g_signal_connect(G_OBJECT(entryBasis),"changed", G_CALLBACK(changedEntryBasis),NULL);
	return comboBasis;
}
/***********************************************************************************************/
void addDeMonAuxBasisToTable(GtkWidget *table, gint i, GtkWidget* comboMethod, GtkWidget* comboExcited)
{
	GtkWidget* entryAuxBasis = NULL;
	GtkWidget* comboAuxBasis = NULL;
	GtkWidget* label = NULL;

	label = add_label_table(table,_("Auxiliary basis"),(gushort)i,0);
	if(comboMethod) g_object_set_data(G_OBJECT (comboMethod), "LabelAuxBasis1", label);
	if(comboExcited) g_object_set_data(G_OBJECT (comboExcited), "LabelAuxBasis1", label);
	label = add_label_table(table,":",(gushort)i,1);
	if(comboMethod) g_object_set_data(G_OBJECT (comboMethod), "LabelAuxBasis2", label);
	if(comboExcited) g_object_set_data(G_OBJECT (comboExcited), "LabelAuxBasis2", label);
	entryAuxBasis = addComboListToATable(table, listAuxBasisView, numberOfAuxBasis, i, 2, 1);
	comboAuxBasis  = g_object_get_data(G_OBJECT (entryAuxBasis), "Combo");
	gtk_widget_set_sensitive(entryAuxBasis, TRUE);
	gtk_editable_set_editable(GTK_EDITABLE(entryAuxBasis),FALSE);
	if(comboMethod) g_object_set_data(G_OBJECT (comboMethod), "ComboAuxBasis", comboAuxBasis);
	if(comboExcited) g_object_set_data(G_OBJECT (comboExcited), "ComboAuxBasis", comboAuxBasis);

	g_signal_connect(G_OBJECT(entryAuxBasis),"changed", G_CALLBACK(changedEntryAuxBasis),NULL);
	demonMolecule.auxBasisName=g_strdup(listAuxBasisReal[0]);
}
/***********************************************************************************************/
static void putDeMonBasisInTextEditor()
{
        gchar buffer[BSIZE];

	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &demonColorFore.keyWord, &demonColorBack.keyWord, "Basis ",-1);
	sprintf(buffer,"%s\n",demonBasis.name);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

}
/***********************************************************************************************/
void putDeMonAuxBasisInTextEditor()
{
        gchar buffer[BSIZE];

	if(!demonMolecule.auxBasisName) return;
        if(strcmp(demonMolecule.auxBasisName, "None") != 0)
        { 
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &demonColorFore.keyWord, &demonColorBack.keyWord, "Auxis ",-1); 
        sprintf(buffer,"%s\n",demonMolecule.auxBasisName);        
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
        }
}
/***********************************************************************************************/
void putDeMonBasisInfoInTextEditor()
{
	putDeMonBasisInTextEditor();
}
