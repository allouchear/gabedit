/* MolcasOptimization.c */
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

#include "../../Config.h"
#include "../Common/Global.h"
#include "../Molcas/MolcasTypes.h"
#include "../Molcas/MolcasGlobal.h"
#include "../Molcas/MolcasGateWay.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/GabeditTextEdit.h"

/************************************************************************************************************/
/* 0-> Single point 
 * 1-> Optimization */
static gint typeOfCalcul[] = { 0,1};

/************************************************************************************************************/
void initMolcasOptimization()
{
	molcasOptimization.numberOfIterations = 0;  
}
/************************************************************************************************************/
void freeMolcasOptimization()
{
	static gboolean first = TRUE;

	if(first)
	{
		initMolcasOptimization();
		first = FALSE;
		return;
	}
	molcasOptimization.numberOfIterations = 0;
}
/**************************************************************************************************************************************/
static void activateRadioButton(GtkWidget *button, gpointer data)
{
	gint* type = NULL;
	GtkWidget *entryIteration = NULL;
	GtkWidget *labelIteration = NULL;
	 
	if(!GTK_IS_WIDGET(button)) return;

	type  = g_object_get_data(G_OBJECT (button), "Type");
	entryIteration = g_object_get_data(G_OBJECT (button), "EntryIteration");
	labelIteration = g_object_get_data(G_OBJECT (button), "LabelIteration");

	if(type)
	{
		if(*type == typeOfCalcul[0])
		{
			if(entryIteration) gtk_widget_set_sensitive(entryIteration, FALSE);
			if(labelIteration) gtk_widget_set_sensitive(labelIteration, FALSE);
			molcasOptimization.numberOfIterations = 0;
		}
		if(*type == typeOfCalcul[1])
		{
			if(entryIteration) gtk_widget_set_sensitive(entryIteration, TRUE);
			if(labelIteration) gtk_widget_set_sensitive(labelIteration, TRUE);
			molcasOptimization.numberOfIterations = 15;
		}
	}
}
/**************************************************************************************************************************************/
static void changedEntryMaxIterations(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gchar* tmp = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	tmp = g_strdup(entryText);
	delete_all_spaces(tmp);

	
	if(isInteger(tmp))
	{
	
		if(atoi(tmp)>0)
			molcasOptimization.numberOfIterations = atoi(tmp);
		else
		{
			molcasOptimization.numberOfIterations = 15;
			gtk_entry_set_text(GTK_ENTRY(entry),"15");
		}
	
	}
	else
	{
		molcasOptimization.numberOfIterations = 15;
		gtk_entry_set_text(GTK_ENTRY(entry),"15");
	}
	g_free(tmp);

}
/**************************************************************************************************************************************/
static GtkWidget* addRadioButtonToATable(GtkWidget* table, GtkWidget* friendButton, gchar* label, gint i, gint j, gint k)
{
	GtkWidget *newButton;

	if(friendButton)
		newButton = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (friendButton)), label);
	else
		newButton = gtk_radio_button_new_with_label( NULL, label);

	gtk_table_attach(GTK_TABLE(table),newButton,j,j+k,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);

	g_object_set_data(G_OBJECT (newButton), "Type",NULL);
	return newButton;
}
/**************************************************************************************************************************************/
void createOptimizationFrame(GtkWidget *win, GtkWidget *box)
{
	GtkWidget* button;
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget *table = gtk_table_new(2,2,FALSE);
	GtkWidget* entryIteration = gtk_entry_new();
	GtkWidget* label = gtk_label_new(_("Number of iterations : "));



	frame = gtk_frame_new (_("Type of caclul"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	button = addRadioButtonToATable(table, NULL, _("Single point"), 0, 0,1);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfCalcul[0]);
	g_object_set_data(G_OBJECT (button), "EntryIteration",entryIteration);
	g_object_set_data(G_OBJECT (button), "LabelIteration",label);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);

	button = addRadioButtonToATable(table, button, _("Optimization of geometry"), 1, 0, 1);
	g_object_set_data(G_OBJECT (button), "Type",&typeOfCalcul[1]);
	g_object_set_data(G_OBJECT (button), "EntryIteration",entryIteration);
	g_object_set_data(G_OBJECT (button), "LabelIteration",label);
	g_signal_connect(G_OBJECT(button),"clicked", G_CALLBACK(activateRadioButton),NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), FALSE);

	add_widget_table(table, label, 0, 1);

	g_signal_connect(G_OBJECT(entryIteration),"changed", G_CALLBACK(changedEntryMaxIterations),button);
	gtk_entry_set_text(GTK_ENTRY(entryIteration),"15");
	add_widget_table(table, entryIteration, 1, 1);
	gtk_widget_set_sensitive(entryIteration, FALSE);
	gtk_widget_set_sensitive(label, FALSE);

	molcasOptimization.numberOfIterations = 0;  
}
/************************************************************************************************************/
void putBeginOptimizationInTextEditor()
{
        gchar buffer[BSIZE];

	if(molcasOptimization.numberOfIterations<1) return;

	/*
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.internCommand, &molcasColorBack.internCommand, ">>> Set output override\n",-1);
	*/

	sprintf(buffer,">>> Set maxiter %d\n",molcasOptimization.numberOfIterations);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.internCommand, &molcasColorBack.internCommand, buffer, -1);

	sprintf(buffer,">>> Do while\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.internCommand, &molcasColorBack.internCommand, buffer, -1);

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n",-1);
}
/************************************************************************************************************/
void putEndOptimizationInTextEditor()
{
	/* gchar buffer[BSIZE];*/

	if(molcasOptimization.numberOfIterations<1) return;

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, &molcasColorBack.program, " &ALASKA &END\n",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, &molcasColorBack.program, "End Of Input\n\n",-1);

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, &molcasColorBack.program, " &SLAPAF &END\n",-1);
	putVariableConstantsZMatInTextEditor();
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.program, &molcasColorBack.program, "End Of Input\n\n",-1);
		                                                                                 
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.internCommand, &molcasColorBack.internCommand, ">>> EndDo\n\n",-1);

	/* this action is done automatically - all grid & molden files are copied to submit directory.*/
	/*
	sprintf(buffer,"! cp $Project.geo.molden   $MOLCAS_SUBMIT_PWD/$Project.geo.molden\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.shellCommand, &molcasColorBack.shellCommand, buffer, -1);

	sprintf(buffer,"! cp $Project.scf.molden   $MOLCAS_SUBMIT_PWD/$Project.scf.molden\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.shellCommand, &molcasColorBack.shellCommand, buffer, -1);

	sprintf(buffer,"! cp $Project.freq.molden   $MOLCAS_SUBMIT_PWD/$Project.freq.molden\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &molcasColorFore.shellCommand, &molcasColorBack.shellCommand, buffer, -1);

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, "\n\n", -1);
	*/

}
/************************************************************************************************************/
void putOptimizationInfoInTextEditor()
{
	putBeginOptimizationInTextEditor();
	putEndOptimizationInTextEditor();
}
/************************************************************************************************************/
