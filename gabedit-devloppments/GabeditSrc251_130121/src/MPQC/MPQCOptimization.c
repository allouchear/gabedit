/* MPQCOptimization.c */
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
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/GabeditTextEdit.h"
typedef enum
{
	SINGLEPOINT, OPTIMIZE
}ButtonTypes;
/************************************************************************************************************/
void initMPQCOptimization()
{
	mpqcOptimization.method = g_strdup("QNewtonOpt"); /* EFCOpt for trans*/
	mpqcOptimization.transitionState = FALSE;
	mpqcOptimization.modeFollowing = FALSE;
	mpqcOptimization.maxIterations = 20;
	mpqcOptimization.updateMethod = g_strdup("BFGSUpdate"); /* PowellUpdate */
	mpqcOptimization.function = g_strdup("$:mole");
	mpqcOptimization.energy = g_strdup("$:mole");
	mpqcOptimization.cartesian = TRUE;
}
/************************************************************************************************************/
void freeMPQCOptimization()
{
	if(mpqcOptimization.method ) g_free(mpqcOptimization.method);
	if(mpqcOptimization.updateMethod) g_free(mpqcOptimization.updateMethod);
	if(mpqcOptimization.function) g_free(mpqcOptimization.function);
	if(mpqcOptimization.energy) g_free(mpqcOptimization.energy);
	mpqcOptimization.method = NULL;
	mpqcOptimization.updateMethod = NULL;
	mpqcOptimization.function = NULL;
	mpqcOptimization.energy = NULL;
}
/*********************************************************************************************************/
static void activateTransButton(GtkWidget *button, gpointer data)
{
	if(!GTK_IS_WIDGET(button)) return;

	if(mpqcOptimization.method) g_free(mpqcOptimization.method);
	if(mpqcOptimization.updateMethod) g_free(mpqcOptimization.updateMethod);

	if(GTK_TOGGLE_BUTTON (button)->active)
	{
		mpqcOptimization.transitionState = TRUE;
		mpqcOptimization.modeFollowing = TRUE;
		mpqcOptimization.method = g_strdup("EFCOpt");
		mpqcOptimization.updateMethod = g_strdup("PowellUpdate");
	}
	else
	{
		mpqcOptimization.transitionState = FALSE;
		mpqcOptimization.modeFollowing = FALSE;
		mpqcOptimization.method = g_strdup("QNewtonOpt"); 
		mpqcOptimization.updateMethod = g_strdup("BFGSUpdate");
	}

}
/*********************************************************************************************************/
static void activateRadioButton(GtkWidget *button, gpointer data)
{
	gint* type = NULL;
	GtkWidget* wid = NULL;
	 
	if(!GTK_IS_WIDGET(button)) return;

	type  = g_object_get_data(G_OBJECT (button), "Type");
	if(type)
	{
		if(*type == SINGLEPOINT) mpqcMpqc.optimize = FALSE;
		if(*type == OPTIMIZE) mpqcMpqc.optimize = TRUE;
		wid = g_object_get_data(G_OBJECT (button), "LabelMaxIterations1");
		if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, mpqcMpqc.optimize);
		wid = g_object_get_data(G_OBJECT (button), "LabelMaxIterations2");
		if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, mpqcMpqc.optimize);
		wid = g_object_get_data(G_OBJECT (button), "ComboMaxIterations");
		if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, mpqcMpqc.optimize);
		wid = g_object_get_data(G_OBJECT (button), "ButtonTransition");
		if(GTK_IS_WIDGET(wid)) gtk_widget_set_sensitive(wid, mpqcMpqc.optimize);

	}
}
/**********************************************************************/
static void changedEntryMaxIterations(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;
	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	if(atoi(entryText)>0)
		mpqcOptimization.maxIterations = atoi(entryText);
	else
	{
		mpqcOptimization.maxIterations = 10;
		gtk_entry_set_text(GTK_ENTRY(entry),"10");
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
static GtkWidget *addMPQCMaxIterationsToTable(GtkWidget *table, gint i,GtkWidget *buttonSinglePoint,GtkWidget *buttonOptimization)
{
	GtkWidget* entryMaxIterations = NULL;
	GtkWidget* comboMaxIterations = NULL;
	gint nlistMaxIterations = 4;
	gchar* listMaxIterations[] = {"10","20","30","40"};
	GtkWidget *labelMaxIterations1 = NULL;
	GtkWidget *labelMaxIterations2 = NULL;

	labelMaxIterations1 = add_label_table(table,_("   Max iterations"),(gushort)i,0);
	labelMaxIterations2 = add_label_table(table,":",(gushort)i,1);
	entryMaxIterations = addComboListToATable(table, listMaxIterations, nlistMaxIterations, i, 2, 1);
	comboMaxIterations  = g_object_get_data(G_OBJECT (entryMaxIterations), "Combo");
	gtk_widget_set_sensitive(entryMaxIterations, TRUE);
	g_object_set_data(G_OBJECT (buttonSinglePoint), "LabelMaxIterations1", labelMaxIterations1);
	g_object_set_data(G_OBJECT (buttonSinglePoint), "LabelMaxIterations2", labelMaxIterations2);
	g_object_set_data(G_OBJECT (buttonSinglePoint), "ComboMaxIterations", comboMaxIterations);
	g_object_set_data(G_OBJECT (buttonOptimization), "LabelMaxIterations1", labelMaxIterations1);
	g_object_set_data(G_OBJECT (buttonOptimization), "LabelMaxIterations2", labelMaxIterations2);
	g_object_set_data(G_OBJECT (buttonOptimization), "ComboMaxIterations", comboMaxIterations);
	gtk_widget_set_sensitive(labelMaxIterations1, mpqcMpqc.optimize);
	gtk_widget_set_sensitive(labelMaxIterations2, mpqcMpqc.optimize);
	gtk_widget_set_sensitive(comboMaxIterations, mpqcMpqc.optimize);
	return comboMaxIterations;
}
/***********************************************************************************************/
void createMPQCOptimization(GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* comboMaxIterations = NULL;
	GtkWidget* entryMaxIterations = NULL;
	GtkWidget *table = NULL;
	gint i;
	GtkWidget *buttonSinglePoint = NULL;
	GtkWidget *buttonOptimization = NULL;
	GtkWidget *button = NULL;
	static ButtonTypes buttonTypes[] = {SINGLEPOINT, OPTIMIZE};

	table = gtk_table_new(3,3,FALSE);

	frame = gtk_frame_new (_("Type of calcul"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	i = 0;
	button = gtk_radio_button_new_with_label(NULL, _("Single point"));
	gtk_table_attach(GTK_TABLE(table),button,0,0+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	buttonSinglePoint = button;
	g_object_set_data(G_OBJECT (buttonSinglePoint), "Type", &buttonTypes[0]);
	g_signal_connect(G_OBJECT(buttonSinglePoint),"clicked", G_CALLBACK(activateRadioButton),NULL);
	i = 1;
	button = gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (button)), _("Optimization"));
	gtk_table_attach(GTK_TABLE(table),button,0,0+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	buttonOptimization = button;
	g_object_set_data(G_OBJECT (buttonOptimization), "Type", &buttonTypes[1]);
	g_signal_connect(G_OBJECT(buttonOptimization),"clicked", G_CALLBACK(activateRadioButton),NULL);

	button = gtk_check_button_new_with_label (_("Transition state"));
	gtk_table_attach(GTK_TABLE(table),button,1,1+2,i,i+1,
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  3,3);
	gtk_widget_set_sensitive(button, mpqcMpqc.optimize);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateTransButton),NULL);
	g_object_set_data(G_OBJECT (buttonOptimization), "ButtonTransition", button);
	g_object_set_data(G_OBJECT (buttonSinglePoint), "ButtonTransition", button);

	i = 2;
	comboMaxIterations = addMPQCMaxIterationsToTable(table, i, buttonSinglePoint, buttonOptimization);
	entryMaxIterations = GTK_BIN(comboMaxIterations)->child;
	g_signal_connect(G_OBJECT(entryMaxIterations),"changed", G_CALLBACK(changedEntryMaxIterations),NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonSinglePoint), !mpqcMpqc.optimize);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonOptimization), mpqcMpqc.optimize);
	g_object_set_data(G_OBJECT (box), "ButtonSinglePoint", buttonSinglePoint);
	g_object_set_data(G_OBJECT (box), "ButtonOptimization", buttonOptimization);

}
/************************************************************************************************************/
/************************************************************************************************************/
static void putMPQCOptimizationInTextEditor()
{
        gchar buffer[BSIZE];

	sprintf(buffer,"%c Optimizer object for the molecular geometry\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,"opt<%s>: (\n", mpqcOptimization.method);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
	sprintf(buffer,"\tfunction = %s\n", mpqcOptimization.function);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(mpqcOptimization.transitionState)
	{
		sprintf(buffer,"\ttransition_state = yes\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}
	if(mpqcOptimization.modeFollowing)
	{
		sprintf(buffer,"\tmode_following = yes\n");
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	}
	sprintf(buffer,"\tmax_iterations = %d\n",mpqcOptimization.maxIterations);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,"update<%s>: ()\n",mpqcOptimization.updateMethod);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t", -1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);

	sprintf(buffer,"convergence<MolEnergyConvergence>: (\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL,"\t", -1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
	sprintf(buffer,"\t\tenergy = %s\n", mpqcOptimization.energy);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	if(mpqcOptimization.cartesian) sprintf(buffer,"\t\tcartesian = yes\n");
	else sprintf(buffer,"\t\tcartesian = no\n");
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
void putMPQCOptimizationInfoInTextEditor()
{
	putMPQCOptimizationInTextEditor();
}
