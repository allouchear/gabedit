/* MPQCBasis.c */
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
#include "../MPQC/MPQCTypes.h"
#include "../MPQC/MPQCGlobal.h"
#include "../MPQC/MPQCLib.h"
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
void initMPQCBasis()
{
	mpqcBasis.name = g_strdup("3-21G");
	mpqcBasis.molecule = g_strdup("$:molecule");
	mpqcBasis.numberOfBasisTypes=0;
	mpqcBasis.basisNames = NULL;
}
/************************************************************************************************************/
void freeMPQCBasis()
{
	if(mpqcBasis.name ) g_free(mpqcBasis.name);
	mpqcBasis.name = NULL;
	if(mpqcBasis.molecule ) g_free(mpqcBasis.molecule);
	mpqcBasis.molecule = NULL;
	if(mpqcBasis.basisNames)
	{
		gint i;
		for(i=0;i<mpqcBasis.numberOfBasisTypes;i++)
			if(mpqcBasis.basisNames[i])g_free(mpqcBasis.basisNames[i]);

		if(mpqcBasis.basisNames)g_free(mpqcBasis.basisNames);
	}
	mpqcBasis.numberOfBasisTypes=0;
	mpqcBasis.basisNames = NULL;
}
/************************************************************************************************************/
static void setComboBasis(GtkWidget *comboBasis)
{
	GList *atomsList = NULL;
	GList *basisList = NULL;
	MPQCAtom* atom = NULL;
	gchar* message = g_malloc(BSIZE*sizeof(gchar));
	gint i;

	atom = mpqcMolecule.listOfAtoms;
	for(i=0; i<mpqcMolecule.numberOfAtoms; i++)
	{
		atomsList = g_list_append(atomsList, atom->symbol);
		atom++;
	}
	if(atomsList) basisList = getMPQCBasisListOfAtoms(atomsList,message);
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

	if(mpqcBasis.name) g_free(mpqcBasis.name);
	mpqcBasis.name=g_strdup(entryText);
}
/************************************************************************************************************/
static void changedEntryBasisGuess(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	if(mpqcGuessWaveFunction.basis) g_free(mpqcGuessWaveFunction.basis);
	mpqcGuessWaveFunction.basis=g_strdup(entryText);
}
/************************************************************************************************************/
static void changedEntryAuxBasis(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	if(mpqcMole.auxBasisName) g_free(mpqcMole.auxBasisName);
	mpqcMole.auxBasisName=g_strdup(entryText);
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
void addMPQCBasisToTable(GtkWidget *table, gint i)
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
void addMPQCBasisGuessToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryBasisGuess = NULL;
	GtkWidget* comboBasisGuess = NULL;

	gint nlistBasis = 1;
	gchar* listBasis[] = {" "};

	add_label_table(table,_("Basis"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryBasisGuess = addComboListToATable(table, listBasis, nlistBasis, i, 2, 1);
	comboBasisGuess  = g_object_get_data(G_OBJECT (entryBasisGuess), "Combo");
	gtk_widget_set_sensitive(entryBasisGuess, FALSE);

	g_signal_connect(G_OBJECT(entryBasisGuess),"changed", G_CALLBACK(changedEntryBasisGuess),NULL);
	setComboBasis(comboBasisGuess);
}
/***********************************************************************************************/
void addMPQCAuxBasisToTable(GtkWidget *table, gint i, GtkWidget* comboMethod)
{
	GtkWidget* entryAuxBasis = NULL;
	GtkWidget* comboAuxBasis = NULL;
	GtkWidget* label = NULL;
	gint nlistAuxBasis = 1;
	gchar* listAuxBasis[] = {" "};
	GtkWidget* entryMethod = NULL;

	if(GTK_IS_COMBO_BOX(comboMethod)) entryMethod = GTK_BIN(comboMethod)->child;


	label = add_label_table(table,_("Auxiliary basis"),(gushort)i,0);
	if(entryMethod) g_object_set_data(G_OBJECT (entryMethod), "LabelAuxBasis1", label);
	label = add_label_table(table,":",(gushort)i,1);
	if(entryMethod) g_object_set_data(G_OBJECT (entryMethod), "LabelAuxBasis2", label);
	entryAuxBasis = addComboListToATable(table, listAuxBasis, nlistAuxBasis, i, 2, 1);
	comboAuxBasis  = g_object_get_data(G_OBJECT (entryAuxBasis), "Combo");
	gtk_widget_set_sensitive(entryAuxBasis, FALSE);
	if(entryMethod) g_object_set_data(G_OBJECT (entryMethod), "ComboAuxBasis", comboAuxBasis);

	g_signal_connect(G_OBJECT(entryAuxBasis),"changed", G_CALLBACK(changedEntryAuxBasis),NULL);
	setComboBasis(comboAuxBasis);
}
/***********************************************************************************************/
static void putMPQCBasisInTextEditor()
{
        gchar buffer[BSIZE];

	sprintf(buffer,"%c Basis set specification\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,"basis<GaussianBasisSet>: (\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);

	sprintf(buffer,"\tmolecule = %s\n", mpqcBasis.molecule);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,"\tname = \"%s\"\n", mpqcBasis.name);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,")\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);

	sprintf(buffer,"%c----------------------------------------------------------------\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
}
/************************************************************************************************************/
void putMPQCBasisInfoInTextEditor()
{
	putMPQCBasisInTextEditor();
}
