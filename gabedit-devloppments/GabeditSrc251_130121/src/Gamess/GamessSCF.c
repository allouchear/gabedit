/* GamessGuess.c */
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
#include "../Gamess/GamessTypes.h"
#include "../Gamess/GamessGlobal.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Common/Windows.h"
#include "../Utils/Constants.h"

static GtkWidget *buttonDirectSCF = NULL;
static GtkWidget *buttonChangeFock = NULL;
static GtkWidget *buttonUHFNO = NULL;
static GtkWidget *scfFrame = NULL;
/*************************************************************************************************************/
void initGamessSCFFrame()
{
	scfFrame = NULL;
}
/*************************************************************************************************************/
void setSensitiveGamessSCFFrame(gboolean sensitive)
{
	if(!scfFrame) return;
	gtk_widget_set_sensitive(scfFrame, sensitive);
}
/*************************************************************************************************************/
static void putGamessSCFOptionsInfoInTextEditor()
{
	if(!GTK_TOGGLE_BUTTON (buttonDirectSCF)->active 
	   && !GTK_TOGGLE_BUTTON (buttonChangeFock)->active
	   && !GTK_TOGGLE_BUTTON (buttonUHFNO)->active
	   ) return;
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$SCF",-1);
	if(GTK_TOGGLE_BUTTON (buttonDirectSCF)->active)
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " DIRSCF=.TRUE.",-1);
	if(GTK_TOGGLE_BUTTON (buttonChangeFock)->active)
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " FDIFF=.TRUE.",-1);
	if(GTK_TOGGLE_BUTTON (buttonUHFNO)->active)
        	gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " UHFNOS=.TRUE.",-1);

        gabedit_text_insert (GABEDIT_TEXT(text), NULL, NULL, NULL, " ",-1);
        gabedit_text_insert (GABEDIT_TEXT(text), NULL, &gamessColorFore.keyWord, &gamessColorBack.keyWord, "$END\n",-1);
}
/************************************************************************************************************/
void putGamessSCFInfoInTextEditor()
{
	putGamessSCFOptionsInfoInTextEditor();

}
/************************************************************************************************************/
void createGamessSCFFrame(GtkWidget *win, GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	gint l=0;
	gint c=0;
	gint ncases=1;
	GtkWidget *table = gtk_table_new(3,1,FALSE);

	buttonDirectSCF = NULL;
	buttonChangeFock = NULL;
	buttonUHFNO = NULL;

	frame = gtk_frame_new (_("SCF options"));
	scfFrame = frame;
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);
	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	/*------------------ DIrectSCF -----------------------------------------*/
	l=0; 
	c = 0; ncases=1;
	buttonDirectSCF = gtk_check_button_new_with_label (_("Direct SCF"));
	gtk_table_attach(GTK_TABLE(table),buttonDirectSCF,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonDirectSCF), FALSE);
	/*------------------ ChangeFock ----------------------------------------*/
	l++; 
	c = 0; ncases=1;
	buttonChangeFock = gtk_check_button_new_with_label (_("Compute only change in Fock matrix"));
	gtk_table_attach(GTK_TABLE(table),buttonChangeFock,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonChangeFock), FALSE);
	/*------------------ UHFNO ----------------------------------------*/
	l++; 
	c = 0; ncases=1;
	buttonUHFNO = gtk_check_button_new_with_label (_("Generate UHF Natural Orbitals"));
	gtk_table_attach(GTK_TABLE(table),buttonUHFNO,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonUHFNO), FALSE);
}
