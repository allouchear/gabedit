/* NWChemBasis.c */
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
#include "../NWChem/NWChemTypes.h"
#include "../NWChem/NWChemGlobal.h"
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
	"Pople with one diffuse function on non-hydrogen atoms", 
	"Pople with one diffuse function on all atoms", 
	"Dunning basis sets", 
	"Jensen Basis Sets", 
	"Miscellenous and Specialized Basis Sets", 
};
static gchar* listTypeBasisReal[] = 
{ 
	"Pople", 
	"PopleDiffuseNonHydrogen", 
	"PopleDiffuseAllAtoms", 
	"Dunning", 
	"Jensen", 
	"Miscellenous", 
};
static guint numberOfTypeBasis = G_N_ELEMENTS (listTypeBasisView);
/*************************************************************************************************************/
static gchar* listPopleBasisView[] = 
{ 
	"6-31G Pople 6-31G and its modifications",
	"6-311G Pople 6-311G and its modifications",

	"3-21G*  3-21G plus one polarisation function all non-hydrogens atoms",
	"6-31G* 6-31G plus one polarisation function all non-hydrogens atoms",
	"6-311G* 6-311G plus one polarisation function all non-hydrogens atoms",

	"3-21G**  3-21G plus one polarisation function all atoms",
	"6-31G** 6-31G plus one polarisation function all atoms",
	"6-311G** 6-311G plus one polarisation function all atoms",

	"3-21G(2d)  3-21G plus two polarisation functions all non-hydrogens atoms",
	"6-31G(2d) 6-31G plus two polarisation functions all non-hydrogens atoms",
	"6-311G(2d) 6-311G plus two polarisation functions all non-hydrogens atoms",

	"3-21G(2d,2p)  3-21G plus two polarisation functions all atoms",
	"6-31G(2d,2p) 6-31G plus two polarisation functions all atoms",
	"6-311G(2d,2p) 6-311G plus two polarisation functions all atoms",

	"3-21G(2df)  3-21G plus three polarisation functions all non-hydrogens atoms",
	"6-31G(2df) 6-31G plus three polarisation functions all non-hydrogens atoms",
	"6-311G(2df) 6-311G plus three polarisation functions all non-hydrogens atoms",

	"3-21G(2df,2pd)  3-21G plus three polarisation functions all atoms",
	"6-31G(2df,2pd) 6-31G plus three polarisation functions all atoms",
	"6-311G(2df,2pd) 6-311G plus three polarisation functions all atoms",

	"3-21G(3df)  3-21G plus four polarisation functions all non-hydrogens atoms",
	"6-31G(3df) 6-31G plus four polarisation functions all non-hydrogens atoms",
	"6-311G(3df) 6-311G plus four polarisation functions all non-hydrogens atoms",

	"3-21G(3df,3pd)  3-21G plus four polarisation functions all atoms",
	"6-31G(3df,3pd) 6-31G plus four polarisation functions all atoms",
	"6-311G(3df,3pd) 6-311G plus four polarisation functions all atoms",
};
static gchar* listPopleBasisReal[] = 
{ 
	"6-31G",
	"6-311G",

	"3-21G*",
	"6-31G*",
	"6-311G*",

	"3-21G**",
	"6-31G**",
	"6-311G**",

	"3-21G(2d)",
	"6-31G(2d)",
	"6-311G(2d)",

	"3-21G(2d,2p)",
	"6-31G(2d,2p)",
	"6-311G(2d,2p)",

	"3-21G(2df)",
	"6-31G(2df)",
	"6-311G(2df)",

	"3-21G(2df,2pd)",
	"6-31G(2df,2pd)",
	"6-311G(2df,2pd)",

	"3-21G(3df)",
	"6-31G(3df)",
	"6-311G(3df)",

	"3-21G(3df,3pd)",
	"6-31G(3df,3pd)",
	"6-311G(3df,3pd)",
};
static guint numberOfPopleBasis = G_N_ELEMENTS (listPopleBasisView);
/*************************************************************************************************************/
static gchar* listPopleDiffuseNonHydrogenBasisView[] = 
{ 
	"3-21+G  3-21G plus diffuse functions on all non-hydrogens atoms",
	"6-31+G 6-31G plus diffuse functions on all non-hydrogens atoms",
	"6-311+G 6-311G plus diffuse functions on all non-hydrogens atoms",

	"3-21+G*  3-21G + diff. non-hydrogens + 1 pol. non-hydrogens",
	"6-31+G* 6-31G + diff. non-hydrogens + 1 pol. non-hydrogens",
	"6-311+G* 6-311G + diff. non-hydrogens + 1 pol. non-hydrogens",

	"3-21+G**  3-21G + diff. non-hydrogens + 1 pol. ",
	"6-31+G** 6-31G + diff. non-hydrogens + 1 pol. ",
	"6-311+G** 6-311G + diff. non-hydrogens + 1 pol. ",

	"3-21+G(2d)  3-21G + diff. non-hydrogens + 2 pol. non-hydrogens",
	"6-31+G(2d) 6-31G + diff. non-hydrogens + 2 pol. non-hydrogens",
	"6-311+G(2d) 6-311G + diff. non-hydrogens + 2 pol. non-hydrogens",

	"3-21+G(2d,2p)  3-21G + diff. non-hydrogens + 2 pol.",
	"6-31+G(2d,2p) 6-31G + diff. non-hydrogens + 2 pol.",
	"6-311+G(2d,2p) 6-311G + diff. non-hydrogens + 2 pol.",

	"3-21+G(2df)  3-21G + diff. non-hydrogens + 3 pol. non-hydrogens",
	"6-31+G(2df) 6-31G + diff. non-hydrogens + 3 pol. non-hydrogens",
	"6-311+G(2df) 6-311G + diff. non-hydrogens + 3 pol. non-hydrogens",

	"3-21+G(2df,2pd)  3-21G + diff. non-hydrogens + 3 pol.",
	"6-31+G(2df,2pd) 6-31G + diff. non-hydrogens + 3 pol.",
	"6-311+G(2df,2pd) 6-311G + diff. non-hydrogens + 3 pol.",

	"3-21+G(3df)  3-21G + diff. non-hydrogens + 4 pol. non-hydrogens",
	"6-31+G(3df) 6-31G + diff. non-hydrogens + 4 pol. non-hydrogens",
	"6-311+G(3df) 6-311G + diff. non-hydrogens + 4 pol. non-hydrogens",

	"3-21+G(3df,3pd)  3-21G + diff. non-hydrogens + 4 pol.",
	"6-31+G(3df,3pd) 6-31G + diff. non-hydrogens + 4 pol.",
	"6-311+G(3df,3pd) 6-311G + diff. non-hydrogens + 4 pol.",
};
static gchar* listPopleDiffuseNonHydrogenBasisReal[] = 
{ 
	"3-21+G",
	"6-31+G",
	"6-311+G",

	"3-21+G*",
	"6-31+G*",
	"6-311+G*",

	"3-21+G**",
	"6-31+G**",
	"6-311+G**",

	"3-21+G(2d)",
	"6-31+G(2d)",
	"6-311+G(2d)",

	"3-21+G(2d,2p)",
	"6-31+G(2d,2p)",
	"6-311+G(2d,2p)",

	"3-21+G(2df)",
	"6-31+G(2df)",
	"6-311+G(2df)",

	"3-21+G(2df,2pd)",
	"6-31+G(2df,2pd)",
	"6-311+G(2df,2pd)",

	"3-21+G(3df)",
	"6-31+G(3df)",
	"6-311+G(3df)",

	"3-21+G(3df,3pd)",
	"6-31+G(3df,3pd)",
	"6-311+G(3df,3pd)",
};
static guint numberOfPopleDiffuseNonHydrogenBasis = G_N_ELEMENTS (listPopleDiffuseNonHydrogenBasisView);
/*************************************************************************************************************/
static gchar* listPopleDiffuseAllAtomsBasisView[] = 
{ 
	"3-21++G  3-21G plus diffuse functions on all atoms",
	"6-31++G 6-31G plus diffuse functions on all atoms",
	"6-311++G 6-311G plus diffuse functions on all atoms",

	"3-21++G*  3-21G + diff. + 1 pol. non-hydrogens",
	"6-31++G* 6-31G + diff. + 1 pol. non-hydrogens",
	"6-311++G* 6-311G + diff. + 1 pol. non-hydrogens",

	"3-21++G**  3-21G + diff. + 1 pol. ",
	"6-31++G** 6-31G + diff. + 1 pol. ",
	"6-311++G** 6-311G + diff. + 1 pol. ",

	"3-21++G(2d)  3-21G + diff. + 2 pol. non-hydrogens",
	"6-31++G(2d) 6-31G + diff. + 2 pol. non-hydrogens",
	"6-311++G(2d) 6-311G + diff. + 2 pol. non-hydrogens",

	"3-21++G(2d,2p)  3-21G + diff. + 2 pol.",
	"6-31++G(2d,2p) 6-31G + diff. + 2 pol.",
	"6-311++G(2d,2p) 6-311G + diff. + 2 pol.",

	"3-21++G(2df)  3-21G + diff. + 3 pol. non-hydrogens",
	"6-31++G(2df) 6-31G + diff. + 3 pol. non-hydrogens",
	"6-311++G(2df) 6-311G + diff. + 3 pol. non-hydrogens",

	"3-21++G(2df,2pd)  3-21G + diff. + 3 pol.",
	"6-31++G(2df,2pd) 6-31G + diff. + 3 pol.",
	"6-311++G(2df,2pd) 6-311G + diff. + 3 pol.",

	"3-21++G(3df)  3-21G + diff. + 4 pol. non-hydrogens",
	"6-31++G(3df) 6-31G + diff. + 4 pol. non-hydrogens",
	"6-311++G(3df) 6-311G + diff. + 4 pol. non-hydrogens",

	"3-21++G(3df,3pd)  3-21G + diff. + 4 pol.",
	"6-31++G(3df,3pd) 6-31G + diff. + 4 pol.",
	"6-311++G(3df,3pd) 6-311G + diff. + 4 pol.",

};
static gchar* listPopleDiffuseAllAtomsBasisReal[] = 
{ 
	"3-21++G",
	"6-31++G",
	"6-311++G",

	"3-21++G*",
	"6-31++G*",
	"6-311++G*",

	"3-21++G**",
	"6-31++G**",
	"6-311++G**",

	"3-21++G(2d)",
	"6-31++G(2d)",
	"6-311++G(2d)",

	"3-21++G(2d,2p)",
	"6-31++G(2d,2p)",
	"6-311++G(2d,2p)",

	"3-21++G(2df)",
	"6-31++G(2df)",
	"6-311++G(2df)",

	"3-21++G(2df,2pd)",
	"6-31++G(2df,2pd)",
	"6-311++G(2df,2pd)",

	"3-21++G(3df)",
	"6-31++G(3df)",
	"6-311++G(3df)",

	"3-21++G(3df,3pd)",
	"6-31++G(3df,3pd)",
	"6-311++G(3df,3pd)",
};
static guint numberOfPopleDiffuseAllAtomsBasis = G_N_ELEMENTS (listPopleDiffuseAllAtomsBasisView);
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
	"cc-pVDZ",
	"Aug-cc-pVDZ",
	"cc-pVTZ",
	"Aug-cc-pVTZ",
	"cc-pVQZ",
	"Aug-cc-pVQZ",
	"cc-pV5Z",
	"Aug-cc-pV5Z",
	"cc-pV6Z",
	"Aug-cc-pV6Z",
};
static guint numberOfDunningBasis = G_N_ELEMENTS (listDunningBasisView);
/*************************************************************************************************************/
static gchar* listJensenBasisView[] = 
{ 
	"PC-1 Polarization consistent basis sets (H-Ar) optimized for DFT",
	"PC-2 double zeta polarization consistent basis sets (H-Ar) optimized for DFT",
	"PC-3 triple zeta polarization consistent basis sets (H-Ar) optimized for DFT",
	"PC-4 quadruple zeta polarization consistent basis sets (H-Ar) optimized for DFT",
	"Aug-PC-1 PC-1 with augmentations by diffuse functions",
	"Aug-PC-2 PC-2 with augmentations by diffuse functions",
	"Aug-PC-3 PC-3 with augmentations by diffuse functions",
	"Aug-PC-4 PC-4 with augmentations by diffuse functions",
};
static gchar* listJensenBasisReal[] = 
{ 
	"PC-1",
	"PC-2",
	"PC-3",
	"PC-4",
	"Aug-PC-1",
	"Aug-PC-2",
	"Aug-PC-3",
	"Aug-PC-4",
};
static guint numberOfJensenBasis = G_N_ELEMENTS (listJensenBasisView);
/*************************************************************************************************************/
static gchar* listMiscellenousBasisView[] = 
{ 
	"IGLO-II Kutzelniggs basis set for NMR and EPR calculations",
	"IGLO-III Kutzelniggs basis set for NMR and EPR calculations (accurate)",

};
static gchar* listMiscellenousBasisReal[] = 
{ 
	"IGLO-II",
	"IGLO-III",
};
static guint numberOfMiscellenousBasis = G_N_ELEMENTS (listMiscellenousBasisView);
/*************************************************************************************************************/
static gchar* listAuxBasisView[] = 
{ 
	"cc-pVDZ-RI",
	"cc-pVTZ-RI",
	"cc-pVQZ-RI",
	"cc-pV5Z-RI",
	"cc-pV6Z-RI",
	"pCVDZ-RI",
};
static gchar* listAuxBasisReal[] = 
{ 
	"cc-pVDZ-RI",
	"cc-pVTZ-RI",
	"cc-pVQZ-RI",
	"cc-pV5Z-RI",
	"cc-pV6Z-RI",
	"pCVDZ-RI",
};
static guint numberOfAuxBasis = G_N_ELEMENTS (listAuxBasisView);
/*************************************************************************************************************/
static void setNWChemBasis(GtkWidget* comboBasis)
{
	gint i;

	if(strcmp(selectedTypeBasis, "Pople")==0)
	{
		listBasisView = listPopleBasisView;
		listBasisReal = listPopleBasisReal;
		numberOfBasis = numberOfPopleBasis;
	}
	else if(strcmp(selectedTypeBasis, "PopleDiffuseNonHydrogen")==0)
	{
		listBasisView = listPopleDiffuseNonHydrogenBasisView;
		listBasisReal = listPopleDiffuseNonHydrogenBasisReal;
		numberOfBasis = numberOfPopleDiffuseNonHydrogenBasis;
	}
	else if(strcmp(selectedTypeBasis, "PopleDiffuseAllAtoms")==0)
	{
		listBasisView = listPopleDiffuseAllAtomsBasisView;
		listBasisReal = listPopleDiffuseAllAtomsBasisReal;
		numberOfBasis = numberOfPopleDiffuseAllAtomsBasis;
	}
	else if(strcmp(selectedTypeBasis, "Dunning")==0)
	{
		listBasisView = listDunningBasisView;
		listBasisReal = listDunningBasisReal;
		numberOfBasis = numberOfDunningBasis;
	}
	else if(strcmp(selectedTypeBasis, "Jensen")==0)
	{
		listBasisView = listJensenBasisView;
		listBasisReal = listJensenBasisReal;
		numberOfBasis = numberOfJensenBasis;
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

	if(comboBasis) setNWChemBasis(comboBasis);
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
		if(i==3) iter0 = iter;
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
void initNWChemBasis()
{
	nwchemBasis.name = g_strdup("3-21G");
	nwchemBasis.molecule = g_strdup("$:molecule");
	nwchemBasis.numberOfBasisTypes=0;
	nwchemBasis.basisNames = NULL;
}
/************************************************************************************************************/
void freeNWChemBasis()
{
	if(nwchemBasis.name ) g_free(nwchemBasis.name);
	nwchemBasis.name = NULL;
	if(nwchemBasis.molecule ) g_free(nwchemBasis.molecule);
	nwchemBasis.molecule = NULL;
	if(nwchemBasis.basisNames)
	{
		gint i;
		for(i=0;i<nwchemBasis.numberOfBasisTypes;i++)
			if(nwchemBasis.basisNames[i])g_free(nwchemBasis.basisNames[i]);

		if(nwchemBasis.basisNames)g_free(nwchemBasis.basisNames);
	}
	nwchemBasis.numberOfBasisTypes=0;
	nwchemBasis.basisNames = NULL;
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

	if(nwchemBasis.name) g_free(nwchemBasis.name);
	for(i=0;i<numberOfBasis;i++)
	{
		if(strcmp((gchar*)entryText,listBasisView[i])==0) res = listBasisReal[i];
	}
	if(res) nwchemBasis.name=g_strdup(res);
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

	if(nwchemMolecule.auxBasisName) g_free(nwchemMolecule.auxBasisName);
	for(i=0;i<numberOfAuxBasis;i++)
	{
		if(strcmp((gchar*)entryText,listAuxBasisView[i])==0) res = listAuxBasisReal[i];
	}
	if(res) nwchemMolecule.auxBasisName=g_strdup(res);
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
void addNWChemTypeBasisToTable(GtkWidget *table, gint i, GtkWidget* comboBasis)
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
GtkWidget* addNWChemBasisToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryBasis = NULL;
	GtkWidget* comboBasis = NULL;
	gint nlistBasis = 1;
	gchar* listBasis[] = {" "};


	add_label_table(table,_("Basis"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryBasis = addComboListToATable(table, listBasis, nlistBasis, i, 2, 1);
	comboBasis  = g_object_get_data(G_OBJECT (entryBasis), "Combo");
	gtk_widget_set_sensitive(entryBasis, FALSE);

	g_signal_connect(G_OBJECT(entryBasis),"changed", G_CALLBACK(changedEntryBasis),NULL);
	return comboBasis;
}
/***********************************************************************************************/
void addNWChemAuxBasisToTable(GtkWidget *table, gint i, GtkWidget* comboMethod, GtkWidget* comboExcited)
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
	gtk_widget_set_sensitive(entryAuxBasis, FALSE);
	if(comboMethod) g_object_set_data(G_OBJECT (comboMethod), "ComboAuxBasis", comboAuxBasis);
	if(comboExcited) g_object_set_data(G_OBJECT (comboExcited), "ComboAuxBasis", comboAuxBasis);

	g_signal_connect(G_OBJECT(entryAuxBasis),"changed", G_CALLBACK(changedEntryAuxBasis),NULL);
	nwchemMolecule.auxBasisName=g_strdup(listAuxBasisReal[0]);
}
/***********************************************************************************************/
static void putNWChemBasisInTextEditor()
{
        gchar buffer[BSIZE];

       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "\nbasis  \"ao basis\" spherical\n", -1);
	sprintf(buffer," * library %s ",nwchemBasis.name);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "\nend\n", -1);

}
/***********************************************************************************************/
void putNWChemAuxBasisInTextEditor()
{
        gchar buffer[BSIZE];

	if(!nwchemMolecule.auxBasisName) return;
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "\nbasis  \"ri-mp2 basis\" spherical\n", -1);
	sprintf(buffer," * library %s",nwchemMolecule.auxBasisName);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
       	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &nwchemColorFore.keyWord, &nwchemColorBack.keyWord, "\nend\n", -1);
}
/***********************************************************************************************/
void putNWChemBasisInfoInTextEditor()
{
	putNWChemBasisInTextEditor();
}
