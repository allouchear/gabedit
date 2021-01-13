/* MPQCGuessWaveFunction.c */
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
#include "../MPQC/MPQCBasis.h"
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
#include "../Utils/AtomsProp.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Symmetry/MoleculeSymmetry.h"
#include "../Symmetry/MoleculeSymmetryInterface.h"
/************************************************************************************************************/
void initMPQCGuessWaveFunction()
{
	mpqcGuessWaveFunction.method = g_strdup("CLHF");
	mpqcGuessWaveFunction.functional = g_strdup("B3LYP");
	mpqcGuessWaveFunction.molecule = g_strdup("$:molecule");
	mpqcGuessWaveFunction.basis = g_strdup("STO-3G");
	mpqcGuessWaveFunction.memory = g_strdup("16000000");
	mpqcGuessWaveFunction.totalCharge = 0;
	mpqcGuessWaveFunction.spinMultiplicity = 1;
	mpqcGuessWaveFunction.fromHCore = FALSE;
}
/************************************************************************************************************/
void freeMPQCGuessWaveFunction()
{
	if(mpqcGuessWaveFunction.method ) g_free(mpqcGuessWaveFunction.method);
	if(mpqcGuessWaveFunction.functional ) g_free(mpqcGuessWaveFunction.functional);
	if(mpqcGuessWaveFunction.molecule  ) g_free(mpqcGuessWaveFunction.molecule );
	if(mpqcGuessWaveFunction.basis ) g_free(mpqcGuessWaveFunction.basis);
	if(mpqcGuessWaveFunction.memory ) g_free(mpqcGuessWaveFunction.memory);
	mpqcGuessWaveFunction.method = NULL;
	mpqcGuessWaveFunction.functional = NULL;
	mpqcGuessWaveFunction.molecule  = NULL;
	mpqcGuessWaveFunction.basis = NULL;
	mpqcGuessWaveFunction.memory = NULL;
}
/************************************************************************************************************/
static void setLabelMethod(GtkWidget* labelMethod, gchar* method)
{
	static GList *glistMethod = NULL;
	static GList *glistLabel = NULL;
	GList *glistM = NULL;
	GList *glistL = NULL;
	if(!glistMethod)
	{
  		glistMethod = g_list_append(glistMethod,"CLHF");
  		glistLabel = g_list_append(glistLabel,"Closed Shell Hartree-Fock");

  		glistMethod = g_list_append(glistMethod,"HSOSHF");
  		glistLabel = g_list_append(glistLabel,"Restricted Open Shell Hartree-Fock");

  		glistMethod = g_list_append(glistMethod,"CLKS");
  		glistLabel = g_list_append(glistLabel,"Closed Shell Kohn-Sham");

  		glistMethod = g_list_append(glistMethod,"HSOSKS");
  		glistLabel = g_list_append(glistLabel,"Restricted Open Shell Kohn-Sham ");

  		glistMethod = g_list_append(glistMethod,"UHF");
  		glistLabel = g_list_append(glistLabel,"Unrestricted Hartree-Fock");

  		glistMethod = g_list_append(glistMethod,"UKS");
  		glistLabel = g_list_append(glistLabel,"Unrestricted Kohn-Sham ");

  		glistMethod = g_list_append(glistMethod,"MBPT2");
  		glistLabel = g_list_append(glistLabel,"MP2 closed shell");

  		glistMethod = g_list_append(glistMethod,"MBPT2_R12/A");
  		glistLabel = g_list_append(glistLabel,"MP2 + R12 correlation closed shell");

  		glistMethod = g_list_append(glistMethod,"MBPT2_R12/A'");
  		glistLabel = g_list_append(glistLabel,"MP2 + R12 correlation closed shell");

  		glistMethod = g_list_append(glistMethod,"ZAPT2");
  		glistLabel = g_list_append(glistLabel,"MP2 open shell");

  		glistMethod = g_list_append(glistMethod,"OPT2(2)");
  		glistLabel = g_list_append(glistLabel,"MP2 open shell");
	}
	glistM = glistMethod;
	glistL = glistLabel;
	while(glistM != NULL && glistL!=NULL)
	{
		if(strcmp((gchar*)glistM->data,method)==0)
		{
			gtk_label_set_text(GTK_LABEL(labelMethod),(gchar*)glistL->data);
			break;
		}
		glistM = glistM->next;
		glistL = glistL->next;
	}

}
/************************************************************************************************************/
static void changedEntryMethod(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	GtkWidget* labelMethod = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	if(mpqcGuessWaveFunction.method) g_free(mpqcGuessWaveFunction.method);
	mpqcGuessWaveFunction.method=g_strdup(entryText);

	labelMethod  = g_object_get_data(G_OBJECT (entry), "LabelMethod");
	if(GTK_IS_WIDGET(labelMethod)) setLabelMethod(labelMethod, mpqcGuessWaveFunction.method);
}
/************************************************************************************************************/
static void setComboSpinMultiplicity(GtkWidget *comboSpinMultiplicity)
{
	GList *glist = NULL;
	gint i;
	gint nlist = 0;
	gchar** list = NULL;
	gint k;
	gint kinc;
	gint ne = mpqcMolecule.numberOfValenceElectrons - mpqcGuessWaveFunction.totalCharge;

	if(ne%2==0)
		nlist = ne/2+1;
	else
		nlist = (ne+1)/2;

	if(nlist<1) return;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	if(GTK_IS_WIDGET(comboSpinMultiplicity)) gtk_widget_set_sensitive(comboSpinMultiplicity, TRUE);
	if(ne%2==0)
		k = 1;
	else
		k = 2;

	kinc = 2;
	for(i=0;i<nlist;i++)
	{
		sprintf(list[i],"+%d",k);
		k+=kinc;
	}

  	for(i=0;i<nlist;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboSpinMultiplicity, glist) ;
  	g_list_free(glist);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
}
/********************************************************************************/
static void setComboCharge(GtkWidget *comboCharge)
{
	GList *glist = NULL;
	gint i;
	gint nlist;
	gchar** list = NULL;
	gint k;

	nlist = mpqcMolecule.numberOfValenceElectrons*2-2+1;

	if(nlist<1) return;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	sprintf(list[0],"0");
	k = 1;
	for(i=1;i<nlist-1;i+=2)
	{
		sprintf(list[i],"+%d",k);
		sprintf(list[i+1],"%d",-k);
		k += 1;
	}

  	for(i=0;i<nlist;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboCharge, glist) ;
  	g_list_free(glist);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
}
/************************************************************************************************************/
static void setComboMethod(GtkWidget *comboMethod)
{
	GList *glist = NULL;
	GtkWidget* entrySpinMultiplicity = NULL;
	G_CONST_RETURN gchar* entryText = NULL;
	int multiplicity;

	entrySpinMultiplicity  = g_object_get_data(G_OBJECT (comboMethod), "EntrySpinMultiplicity");
	if(!(GTK_IS_WIDGET(entrySpinMultiplicity))) return;

	 
	entryText = gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity));
	if(strlen(entryText)<1)return;
	multiplicity = atoi(entryText);

	if(multiplicity==1)
	{
  		glist = g_list_append(glist,"CLHF");
  		glist = g_list_append(glist,"HSOSHF");
		/*
  		glist = g_list_append(glist,"CLKS");
  		glist = g_list_append(glist,"HSOSKS");
		*/
  		glist = g_list_append(glist,"UHF");
		/*
  		glist = g_list_append(glist,"UKS");
		*/
	}
	else
	{
  		glist = g_list_append(glist,"UHF");
  		glist = g_list_append(glist,"HSOSHF");
		/*
  		glist = g_list_append(glist,"UKS");
  		glist = g_list_append(glist,"HSOSKS");
		*/
	}
  	gtk_combo_box_entry_set_popdown_strings( comboMethod, glist) ;
}
/**********************************************************************/
static void changedEntrySpinMultiplicity(GtkWidget *entry, gpointer data)
{
	GtkWidget* comboMethod;
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	mpqcGuessWaveFunction.spinMultiplicity=atoi(entryText);
	if(mpqcGuessWaveFunction.method) g_free(mpqcGuessWaveFunction.method);
	if(mpqcGuessWaveFunction.spinMultiplicity==1) mpqcGuessWaveFunction.method = g_strdup("CLHF");
	else mpqcGuessWaveFunction.method = g_strdup("UHF");

	mpqcGuessWaveFunction.spinMultiplicity=mpqcGuessWaveFunction.spinMultiplicity;
	if(mpqcGuessWaveFunction.method) g_free(mpqcGuessWaveFunction.method);
	if(mpqcGuessWaveFunction.spinMultiplicity==1) mpqcGuessWaveFunction.method = g_strdup("CLHF");
	else mpqcGuessWaveFunction.method = g_strdup("UHF");
	comboMethod = g_object_get_data(G_OBJECT (entry), "ComboMethod");
	if(GTK_IS_WIDGET(comboMethod)) setComboMethod(comboMethod);
}
/**********************************************************************/
static void changedEntryCharge(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* labelNumberOfElectrons = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	mpqcGuessWaveFunction.totalCharge = atoi(entryText);
	mpqcGuessWaveFunction.totalCharge = mpqcGuessWaveFunction.totalCharge;

	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entry), "ComboSpinMultiplicity");
	if(GTK_IS_WIDGET(comboSpinMultiplicity)) setComboSpinMultiplicity(comboSpinMultiplicity);

	labelNumberOfElectrons = g_object_get_data(G_OBJECT (entry), "LabelNumberOfElectrons");

	if(GTK_IS_WIDGET(labelNumberOfElectrons))
	{
		gint ne = mpqcMolecule.numberOfValenceElectrons - mpqcGuessWaveFunction.totalCharge;
		gchar buffer[BSIZE];
		sprintf(buffer, _("Number of electrons = %d"),ne);
		gtk_label_set_text(GTK_LABEL(labelNumberOfElectrons),buffer);
	}
}
/**********************************************************************/
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
static GtkWidget *addMPQCChargeToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryCharge = NULL;
	GtkWidget* comboCharge = NULL;
	gint nlistCharge = 1;
	gchar* listCharge[] = {"0"};
	GtkWidget* labelNumberOfElectrons = NULL;

	add_label_table(table,"Charge",(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryCharge = addComboListToATable(table, listCharge, nlistCharge, i, 2, 1);
	comboCharge  = g_object_get_data(G_OBJECT (entryCharge), "Combo");
	gtk_widget_set_sensitive(entryCharge, FALSE);

	labelNumberOfElectrons = add_label_table(table," ",(gushort)i,3);

	g_object_set_data(G_OBJECT (entryCharge), "LabelNumberOfElectrons", labelNumberOfElectrons);
	g_signal_connect(G_OBJECT(entryCharge),"changed", G_CALLBACK(changedEntryCharge),NULL);
	setComboCharge(comboCharge);
	return comboCharge;
}
/***********************************************************************************************/
static GtkWidget *addMPQCSpinGuessToTable(GtkWidget *table, gint i)
{
	GtkWidget* entrySpinMultiplicity = NULL;
	GtkWidget* comboSpinMultiplicity = NULL;
	gint nlistspinMultiplicity = 1;
	gchar* listspinMultiplicity[] = {"0"};

	add_label_table(table,_("Spin multiplicity"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entrySpinMultiplicity = addComboListToATable(table, listspinMultiplicity, nlistspinMultiplicity, i, 2, 1);
	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entrySpinMultiplicity), "Combo");
	gtk_widget_set_sensitive(entrySpinMultiplicity, FALSE);

	g_signal_connect(G_OBJECT(entrySpinMultiplicity),"changed", G_CALLBACK(changedEntrySpinMultiplicity),NULL);
	setComboSpinMultiplicity(comboSpinMultiplicity);
	return comboSpinMultiplicity;
}
/***********************************************************************************************/
static void addMPQCMethodToTable(GtkWidget *table, gint i, GtkWidget *comboSpinMultiplicity)
{
	GtkWidget* entryMethod = NULL;
	GtkWidget* comboMethod = NULL;
	gint nlistMethod = 1;
	gchar* listMethod[] = {"CLHF"};
	GtkWidget* labelMethod = NULL;
	GtkWidget* entrySpinMultiplicity = NULL;

	entrySpinMultiplicity  = GTK_BIN (comboSpinMultiplicity)->child;

	add_label_table(table, _("Method"),(gushort) i, 0);
	add_label_table(table, ":",(gushort) i, 1);
	entryMethod = addComboListToATable(table, listMethod, nlistMethod, i, 2, 1);
	comboMethod  = g_object_get_data(G_OBJECT (entryMethod), "Combo");
	gtk_widget_set_sensitive(entryMethod, FALSE);
	labelMethod = add_label_table(table, " ",(gushort) i, 3);

	g_object_set_data(G_OBJECT (entryMethod), "ComboSpinMultiplicity", comboSpinMultiplicity);
	g_object_set_data(G_OBJECT (entryMethod), "LabelMethod", labelMethod);
	g_object_set_data(G_OBJECT (entryMethod), "EntrySpinMultiplicity", entrySpinMultiplicity);
	g_object_set_data(G_OBJECT (comboMethod), "EntrySpinMultiplicity", entrySpinMultiplicity);
	g_object_set_data(G_OBJECT (entrySpinMultiplicity), "ComboMethod", comboMethod);
	g_signal_connect(G_OBJECT(entryMethod),"changed", G_CALLBACK(changedEntryMethod),NULL);
	setComboMethod(comboMethod);
}
/***********************************************************************************************/
void coreButtonClicked(GtkWidget *button, gpointer data)
{
	GtkWidget* vboxTable = NULL;
	if(!GTK_IS_BUTTON(button))return;
	vboxTable = g_object_get_data(G_OBJECT (button), "VboxTable");
  	if(GTK_TOGGLE_BUTTON (button)->active)
	{
		if(GTK_IS_WIDGET(vboxTable)) gtk_widget_set_sensitive(vboxTable, FALSE);
		/* if(GTK_IS_WIDGET(vboxTable)) gtk_widget_hide(vboxTable);*/
		
		mpqcGuessWaveFunction.fromHCore = TRUE;
	}
	else
	{
		if(GTK_IS_WIDGET(vboxTable)) gtk_widget_set_sensitive(vboxTable, TRUE);
		mpqcGuessWaveFunction.fromHCore = FALSE;
		/* if(GTK_IS_WIDGET(vboxTable)) gtk_widget_show(vboxTable);*/
	}
}
/***********************************************************************************************/
void createMPQCGuess(GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* vboxTable;
	GtkWidget* hboxButton;
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* comboCharge = NULL;
	GtkWidget *table = NULL;
	GtkWidget *checkButton = NULL;
	gint i;

	table = gtk_table_new(5,4,FALSE);

	frame = gtk_frame_new (_("Parameters for computing the guess wave function"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	hboxButton = gtk_hbox_new(TRUE, 5);
	gtk_box_pack_start (GTK_BOX (vboxFrame), hboxButton, FALSE, FALSE, 5);

	checkButton = gtk_check_button_new_with_label (_("Starting from core Hamiltonian guess."));
	gtk_box_pack_start (GTK_BOX (hboxButton), checkButton, TRUE, TRUE, 5);

	vboxTable = gtk_vbox_new (FALSE, 3);
	gtk_box_pack_start (GTK_BOX (vboxFrame), vboxTable, TRUE, TRUE, 3);

	gtk_box_pack_start (GTK_BOX (vboxTable), table, TRUE, TRUE, 0);

	i = 0;
	comboCharge = addMPQCChargeToTable(table, i);
	i = 1;
	comboSpinMultiplicity = addMPQCSpinGuessToTable(table, i);

	if(GTK_IS_COMBO_BOX(comboCharge))
		g_object_set_data(G_OBJECT (GTK_BIN(comboCharge)->child), "ComboSpinMultiplicity", comboSpinMultiplicity);

	i = 2;
	addMPQCMethodToTable(table, i, comboSpinMultiplicity);
	i = 3;
	addMPQCBasisGuessToTable(table,i);

	g_object_set_data(G_OBJECT (checkButton), "VboxTable", vboxTable);
	g_signal_connect(G_OBJECT(checkButton),"clicked", G_CALLBACK(coreButtonClicked),NULL);
}
/**************************************************************************************************************************************/
static void putMPQCGuessWaveInTextEditor()
{
        gchar buffer[BSIZE];

	sprintf(buffer,"%c Guess wave function\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,"guess_wavefunction<%s>: (\n", mpqcGuessWaveFunction.method);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
	sprintf(buffer,"\ttotal_charge = %d\n", mpqcGuessWaveFunction.totalCharge);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	if(!strstr(mpqcGuessWaveFunction.method,"CL"))
	{
		sprintf(buffer,"\tmultiplicity = %d\n", mpqcGuessWaveFunction.spinMultiplicity);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}
	sprintf(buffer,"\tmolecule = %s\n", mpqcGuessWaveFunction.molecule);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,"\tmemory = %s\n", mpqcGuessWaveFunction.memory);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,"basis<GaussianBasisSet>: (\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t", -1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
	sprintf(buffer,"\t\tmolecule = %s\n", mpqcGuessWaveFunction.molecule);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,"\t\tname = \"%s\"\n", mpqcGuessWaveFunction.basis);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,")\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t", -1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);

	sprintf(buffer,")\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
	sprintf(buffer,"%c----------------------------------------------------------------\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
}
/**************************************************************************************************************************************/
/************************************************************************************************************/
void putMPQCGuessWaveFunctionInfoInTextEditor()
{
	if(!mpqcGuessWaveFunction.fromHCore)
		putMPQCGuessWaveInTextEditor();
}
