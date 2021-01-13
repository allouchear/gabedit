/* Status.c */
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

#include "../../Config.h"
#include "../Common/Global.h"
#include "../Utils/Utils.h"
#include "../Geometry/ResultsAnalise.h"
#include "../Common/Status.h"
/* extern *.h */
gboolean stopDownLoad;
/********************************************************************************/

static GtkWidget *ProgressBar = NULL;
static GtkWidget *ProgressTable = NULL;
static GtkWidget *ButtonCancel = NULL;
/********************************************************************************/
void stop_download()
{
	stopDownLoad = TRUE;
}
/********************************************************************************/
void show_progress_connection()
{
#ifdef G_OS_WIN32
#else
	if(fileopen.netWorkProtocol == GABEDIT_NETWORK_FTP_RSH)
		gtk_widget_show(ButtonCancel);
	else
		gtk_widget_hide(ButtonCancel);
#endif
	set_sensitive_remote_frame(FALSE);
	/* gtk_widget_show(ProgressBar);*/
	gtk_widget_show(ProgressTable);
	stopDownLoad = FALSE;
	set_sensitive_remote_frame(FALSE);
}
/********************************************************************************/
void hide_progress_connection()
{
	/*gtk_widget_hide(ProgressBar);*/
	gtk_widget_hide(ProgressTable);
	stopDownLoad = TRUE;
	set_sensitive_remote_frame(TRUE);
}
/********************************************************************************/
gint progress_connection(gdouble scal,gchar* str,gboolean reset)
{

	gdouble new_val;

	if(reset)
	{
		new_val = 0;
		gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (ProgressBar), new_val);
		while( gtk_events_pending() ) gtk_main_iteration();

		return TRUE;
	}
	else
    		new_val = gtk_progress_bar_get_fraction( GTK_PROGRESS_BAR(ProgressBar) ) + scal;

	if (new_val > 1) new_val = 1.0;

	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (ProgressBar), new_val);
    	while( gtk_events_pending() ) gtk_main_iteration();

    return TRUE;
}
/********************************************************************************/
static GtkWidget *create_progress_bar(GtkWidget *box)
{
	GtkWidget *pbar;
	GtkWidget *button;
	GtkWidget *table;

	table = gtk_table_new(1,4,FALSE);
	gtk_box_pack_start (GTK_BOX(box), table, FALSE, TRUE, 2);
	gtk_widget_show (table);

	pbar = gtk_progress_bar_new ();

	gtk_table_attach(GTK_TABLE(table),pbar,0,2,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);


  	button = gtk_button_new_with_label(_(" Cancel "));
	ButtonCancel = button;
  	gtk_widget_show_all (button);
	gtk_table_attach(GTK_TABLE(table),button,2,3,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			1,1);

  	gtk_widget_show_all (table);
	ProgressTable = table;
	g_signal_connect(G_OBJECT(button), "clicked", (GCallback)stop_download,NULL);  
	stopDownLoad = FALSE;

	return pbar;
}
/********************************************************************************/
void create_status_progress_connection_bar(GtkWidget* vbox)
{
	ProgressBar = create_progress_bar(vbox);
}
/********************************************************************************/
