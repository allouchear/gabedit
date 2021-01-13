/* QChemGuess.c */
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
#include "../QChem/QChemTypes.h"
#include "../QChem/QChemGlobal.h"
#include "../QChem/QChemBasis.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"

static GtkWidget *buttonMixed = NULL;
static GtkWidget *guessFrame = NULL;
/*************************************************************************************************************/
static gchar* listGuessMethodView[] = { 
	"Default", 
	"Diagonalize core Hamiltonian", 
	"Superposition of atomic density",
	"Apply generalized Wolfsberg-Helmholtz approximation",
	"Basis set projection (basis2)",
};
static gchar* listGuessMethodReal[] = { 
	"NONE", 
	"core",
	"SAD",
	"GWH",
	"BASIS2",

};
static guint numberOfGuessMethods = G_N_ELEMENTS (listGuessMethodView);
static gchar selectedGuessMethod[BSIZE]="NONE";
/*************************************************************************************************************/
void initQChemGuessFrame()
{
	guessFrame = NULL;
	buttonMixed = NULL;
}
/*************************************************************************************************************/
void setSensitiveQChemGuessMixed(gboolean sensitive)
{
	if(!buttonMixed) return;
	gtk_widget_set_sensitive(buttonMixed, sensitive);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMixed), FALSE);
}
/*************************************************************************************************************/
static void putQChemGuessOptionsInfoInTextEditor()
{
	if(GTK_TOGGLE_BUTTON (buttonMixed)->active)
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " scf_guess_mix     5\n",-1);

}
/************************************************************************************************************/
static void putQChemGuessMethodInfoInTextEditor()
{
	if( strcmp(selectedGuessMethod,"NONE")==0 ) return;
	if( strcmp(selectedGuessMethod,"BASIS2")==0 )
	{
		putQChemBasis2InTextEditor();
	}
	else
	{
		gchar buffer[BSIZE];
		sprintf(buffer," scf_guess     %s\n",selectedGuessMethod);
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, buffer,-1);
	}
}
/*************************************************************************************************************/
void putQChemGuessInfoInTextEditor()
{
	putQChemGuessMethodInfoInTextEditor();
	putQChemGuessOptionsInfoInTextEditor();

}
/************************************************************************************************************/
static void traitementGuessMethod (GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	gchar* res = NULL;
	gint i;
	gboolean basis2 = FALSE;
	GtkWidget* wid;

	
	/* gchar* s;*/
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	for(i=0;i<numberOfGuessMethods;i++)
	{
		if(strcmp((gchar*)data,listGuessMethodView[i])==0) res = listGuessMethodReal[i];
	}
	if(res) sprintf(selectedGuessMethod,"%s",res);
	else  sprintf(selectedGuessMethod,"Default");

	if(strstr(selectedGuessMethod,"BASIS2")) basis2 = TRUE;
	wid  = g_object_get_data(G_OBJECT (combobox), "LabelGuessBasis1");
	if(wid) gtk_widget_set_sensitive(wid, basis2);
	wid  = g_object_get_data(G_OBJECT (combobox), "LabelGuessBasis2");
	if(wid) gtk_widget_set_sensitive(wid, basis2);
	wid  = g_object_get_data(G_OBJECT (combobox), "ComboGuessBasis");
	if(wid) gtk_widget_set_sensitive(wid, basis2);
	
	/* for(s=selectedGuessMethod;*s != 0;s++) *s = toupper(*s);*/
}
/********************************************************************************************************/
static GtkWidget *create_list_guessmethods()
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;
        GtkTreeIter iter0;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<numberOfGuessMethods;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
		if(i==0) iter0 = iter;
        	gtk_tree_store_set (store, &iter, 0, listGuessMethodView[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	/*
	gtk_combo_box_set_add_tearoffs (GTK_COMBO_BOX (combobox), TRUE);
	*/
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traitementGuessMethod), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	gtk_combo_box_set_active_iter(GTK_COMBO_BOX (combobox), &iter0);

	return combobox;
}
/************************************************************************************************************/
void createQChemGuessFrame(GtkWidget *win, GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* sep;
	GtkWidget* combo = NULL;
	gint l=0;
	gint c=0;
	gint ncases=1;
	GtkWidget *table = gtk_table_new(4,3,FALSE);

	buttonMixed = NULL;

	frame = gtk_frame_new (_("Mo Guess"));
	guessFrame = frame;
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);
	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	/*------------------ Guess Method -----------------------------------------*/
	l=0; 
	c = 0; ncases=1;
	add_label_table(table,_("Initial Guess"),l,c);
	c = 1; ncases=1;
	add_label_table(table,":",l,c);
	combo = create_list_guessmethods();
	c = 2; ncases=1;
	gtk_table_attach(GTK_TABLE(table),combo,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	/*------------------ Guess basis -----------------------------------------*/
	l++;
	addQChemBasisGuessToTable(table, l, combo);
	traitementGuessMethod(GTK_COMBO_BOX(combo),NULL); /* for activate sensitivity of basis2 */
	/*------------------ separator -----------------------------------------*/
	l++;
	sep = gtk_hseparator_new ();
	c = 0; ncases=3;
	gtk_table_attach(GTK_TABLE(table),sep,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);

	/*------------------ Mix ------------*/
	l++; 
	c = 0; ncases=3;
	buttonMixed = gtk_check_button_new_with_label (_("Mixed alpha and beta orbitals"));
	gtk_table_attach(GTK_TABLE(table),buttonMixed,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMixed), FALSE);
	setSensitiveQChemGuessMixed(FALSE);
}
