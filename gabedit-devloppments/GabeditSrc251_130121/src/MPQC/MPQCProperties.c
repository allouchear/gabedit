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
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
typedef enum
{
	PRINTNPA, PRINTNAO, FREQUENCIES
}ButtonTypes;
/**************************************************************/
static void activateButton(GtkWidget *button, gpointer data)
{
	gint* type = NULL;
	gboolean activate = FALSE;
	 
	if(!GTK_IS_WIDGET(button)) return;

	if(GTK_TOGGLE_BUTTON (button)->active) activate = TRUE;
	type  = g_object_get_data(G_OBJECT (button), "Type");
	if(type)
	{
		if(*type == PRINTNPA) mpqcMole.printNPA = activate;
		if(*type == PRINTNAO) mpqcMole.printNAO = activate;
		if(*type == FREQUENCIES) mpqcMpqc.frequencies = activate;
	}
}
/******************************************************************************/
void createMPQCProperties(GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget *table = NULL;
	gint i;
	GtkWidget *buttonPrintNPA = NULL;
	GtkWidget *buttonPrintNAO = NULL;
	GtkWidget *buttonFrequencies = NULL;
	static ButtonTypes buttonTypes[] = {PRINTNPA, PRINTNAO, FREQUENCIES};

	table = gtk_table_new(3,1,FALSE);

	frame = gtk_frame_new ("Properties");
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	i = 0;
	buttonPrintNPA = gtk_check_button_new_with_label (_("Print the natural population analysis "));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPrintNPA), mpqcMole.printNPA);
	add_widget_table(table, buttonPrintNPA, (gushort)i, 0);
	g_object_set_data(G_OBJECT (buttonPrintNPA), "Type", &buttonTypes[PRINTNPA]);
	g_signal_connect(G_OBJECT(buttonPrintNPA),"clicked", G_CALLBACK(activateButton),NULL);
	i = 1;
	buttonPrintNAO = gtk_check_button_new_with_label (_("Print the natural atomic orbitals"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPrintNAO), mpqcMole.printNAO);
	add_widget_table(table, buttonPrintNAO, (gushort)i, 0);
	g_object_set_data(G_OBJECT (buttonPrintNAO), "Type", &buttonTypes[PRINTNAO]);
	g_signal_connect(G_OBJECT(buttonPrintNAO),"clicked", G_CALLBACK(activateButton),NULL);
	i = 2;
	buttonFrequencies = gtk_check_button_new_with_label (_("Calcul Frequencies"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonFrequencies), mpqcMpqc.frequencies);
	add_widget_table(table, buttonFrequencies, (gushort)i, 0);
	g_object_set_data(G_OBJECT (buttonFrequencies), "Type", &buttonTypes[FREQUENCIES]);
	g_signal_connect(G_OBJECT(buttonFrequencies),"clicked", G_CALLBACK(activateButton),NULL);

}
/*******************************************************************************************/
static void putMPQCFrequenciesInTextEditor()
{
        gchar buffer[BSIZE];

	sprintf(buffer,"%c vibrational frequency input\n",'%');
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,"freq<MolecularFrequencies>: (\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
	sprintf(buffer,"\tmolecule = $:molecule\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);
	sprintf(buffer,"\tdebug = 1\n");
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer, -1);

	sprintf(buffer,")\n");
	gabedit_text_insert (GABEDIT_TEXT(text), NULL, &mpqcColorFore.keyWord, &mpqcColorBack.keyWord, buffer, -1);
}
/*******************************************************************************************/
void putMPQCPropertiesInfoInTextEditor()
{
	putMPQCFrequenciesInTextEditor();
}
