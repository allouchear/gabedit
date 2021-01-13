/* QChemBasis.c */
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
#include "../QChem/QChemTypes.h"
#include "../QChem/QChemGlobal.h"
#include "../QChem/QChemLib.h"
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
/************************************************************************************************************/
void initQChemBasis()
{
	qchemBasis.name = g_strdup("3-21G");
	qchemBasis.molecule = g_strdup("$:molecule");
	qchemBasis.numberOfBasisTypes=0;
	qchemBasis.basisNames = NULL;
}
/************************************************************************************************************/
void freeQChemBasis()
{
	if(qchemBasis.name ) g_free(qchemBasis.name);
	qchemBasis.name = NULL;
	if(qchemBasis.molecule ) g_free(qchemBasis.molecule);
	qchemBasis.molecule = NULL;
	if(qchemBasis.basisNames)
	{
		gint i;
		for(i=0;i<qchemBasis.numberOfBasisTypes;i++)
			if(qchemBasis.basisNames[i])g_free(qchemBasis.basisNames[i]);

		if(qchemBasis.basisNames)g_free(qchemBasis.basisNames);
	}
	qchemBasis.numberOfBasisTypes=0;
	qchemBasis.basisNames = NULL;
}
/************************************************************************************************************/
static void setComboBasisAux(GtkWidget *comboBasis)
{
	GList *atomsList = NULL;
	GList *basisList = NULL;
	GList *basisListAux = NULL;
	GList *l = NULL;
	QChemAtom* atom = NULL;
	gchar* message = g_malloc(BSIZE*sizeof(gchar));
	gint i;

	atom = qchemMolecule.listOfAtoms;
	for(i=0; i<qchemMolecule.numberOfAtoms; i++)
	{
		atomsList = g_list_append(atomsList, atom->symbol);
		atom++;
	}
	if(atomsList) basisList = getQChemBasisListOfAtoms(atomsList,message);
	if(!basisList) basisList = g_list_append(basisList, _("Sorry, I can not obtain one type of bases for all atoms of your molecule"));
	g_list_free(atomsList);

	l = basisList;
	while( l )
	{
		if(strstr(l->data,"rimp")) basisListAux = g_list_append(basisListAux,l->data);
		if(strstr(l->data,_("Sorry"))) basisListAux = g_list_append(basisListAux,l->data);
		l = l->next;
	}

  	gtk_combo_box_entry_set_popdown_strings( comboBasis, basisListAux);
	g_list_free(basisList);
	g_free(message);
}
/************************************************************************************************************/
static void setComboBasis(GtkWidget *comboBasis)
{
	GList *atomsList = NULL;
	GList *basisList = NULL;
	QChemAtom* atom = NULL;
	gchar* message = g_malloc(BSIZE*sizeof(gchar));
	gint i;

	atom = qchemMolecule.listOfAtoms;
	for(i=0; i<qchemMolecule.numberOfAtoms; i++)
	{
		atomsList = g_list_append(atomsList, atom->symbol);
		atom++;
	}
	if(atomsList) basisList = getQChemBasisListOfAtoms(atomsList,message);
	if(!basisList) basisList = g_list_append(basisList, _("Sorry, I can not obtain one type of bases for all atoms of your molecule"));
	g_list_free(atomsList);

  	gtk_combo_box_entry_set_popdown_strings( comboBasis, basisList);
	g_free(message);
}
/************************************************************************************************************/
static void changedEntryBasis(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	if(qchemBasis.name) g_free(qchemBasis.name);
	qchemBasis.name=g_strdup(entryText);
}
/************************************************************************************************************/
static void changedEntryBasisGuess(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	if(qchemGuessWaveFunction.basis) g_free(qchemGuessWaveFunction.basis);
	qchemGuessWaveFunction.basis=g_strdup(entryText);
}
/************************************************************************************************************/
static void changedEntryAuxBasis(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	if(qchemMolecule.auxBasisName) g_free(qchemMolecule.auxBasisName);
	qchemMolecule.auxBasisName=g_strdup(entryText);
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
void addQChemBasisToTable(GtkWidget *table, gint i)
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
	setComboBasis(comboBasis);
}
/***********************************************************************************************/
void addQChemBasisGuessToTable(GtkWidget *table, gint i, GtkWidget* comboGuess)
{
	GtkWidget* entryBasisGuess = NULL;
	GtkWidget* comboBasisGuess = NULL;
	GtkWidget* label = NULL;

	gint nlistBasis = 1;
	gchar* listBasis[] = {" "};

	label = add_label_table(table,_("Basis"),(gushort)i,0);
	if(comboGuess) g_object_set_data(G_OBJECT (comboGuess), "LabelGuessBasis1", label);
	label = add_label_table(table,":",(gushort)i,1);
	if(comboGuess) g_object_set_data(G_OBJECT (comboGuess), "LabelGuessBasis2", label);
	entryBasisGuess = addComboListToATable(table, listBasis, nlistBasis, i, 2, 1);
	comboBasisGuess  = g_object_get_data(G_OBJECT (entryBasisGuess), "Combo");
	gtk_widget_set_sensitive(entryBasisGuess, FALSE);
	if(comboGuess) g_object_set_data(G_OBJECT (comboGuess), "ComboGuessBasis", comboBasisGuess);

	g_signal_connect(G_OBJECT(entryBasisGuess),"changed", G_CALLBACK(changedEntryBasisGuess),NULL);
	setComboBasis(comboBasisGuess);
}
/***********************************************************************************************/
void addQChemAuxBasisToTable(GtkWidget *table, gint i, GtkWidget* comboCorrelation)
{
	GtkWidget* entryAuxBasis = NULL;
	GtkWidget* comboAuxBasis = NULL;
	GtkWidget* label = NULL;
	gint nlistAuxBasis = 1;
	gchar* listAuxBasis[] = {" "};

	label = add_label_table(table,_("Auxiliary basis"),(gushort)i,0);
	if(comboCorrelation) g_object_set_data(G_OBJECT (comboCorrelation), "LabelAuxBasis1", label);
	label = add_label_table(table,":",(gushort)i,1);
	if(comboCorrelation) g_object_set_data(G_OBJECT (comboCorrelation), "LabelAuxBasis2", label);
	entryAuxBasis = addComboListToATable(table, listAuxBasis, nlistAuxBasis, i, 2, 1);
	comboAuxBasis  = g_object_get_data(G_OBJECT (entryAuxBasis), "Combo");
	gtk_widget_set_sensitive(entryAuxBasis, FALSE);
	if(comboCorrelation) g_object_set_data(G_OBJECT (comboCorrelation), "ComboAuxBasis", comboAuxBasis);

	g_signal_connect(G_OBJECT(entryAuxBasis),"changed", G_CALLBACK(changedEntryAuxBasis),NULL);
	setComboBasisAux(comboAuxBasis);
}
/***********************************************************************************************/
static void putQChemBasisInTextEditor()
{
        gchar buffer[BSIZE];

	sprintf(buffer," basis %s\n",qchemBasis.name);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

}
/***********************************************************************************************/
void putQChemAuxBasisInTextEditor()
{
        gchar buffer[BSIZE];

	if(!qchemMolecule.auxBasisName) return;
	sprintf(buffer," aux_basis %s\n",qchemMolecule.auxBasisName);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
}
/***********************************************************************************************/
void putQChemBasis2InTextEditor()
{
        gchar buffer[BSIZE];

	if(!qchemGuessWaveFunction.basis) return;
	sprintf(buffer," basis2 %s\n",qchemGuessWaveFunction.basis);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
}
/************************************************************************************************************/
void putQChemBasisInfoInTextEditor()
{
	putQChemBasisInTextEditor();
}
