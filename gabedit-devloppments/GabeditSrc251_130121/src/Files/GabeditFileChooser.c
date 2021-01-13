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
#include <gtk/gtk.h>
#include "GabeditFileChooser.h"

/********************************************************************************/
GtkWidget* gabedit_file_chooser_new(gchar* title, GtkFileChooserAction action)
{
	gboolean multiple = FALSE;
	GtkWidget *dialog;
	dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", action, "file-system-backend", "gtk+", "select-multiple", multiple, NULL);
	if(title) gtk_window_set_title (GTK_WINDOW (dialog), title);
	if (action == GTK_FILE_CHOOSER_ACTION_OPEN)
	{
		gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	}
	else
	{
		gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
	}
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	return GTK_WIDGET(dialog);

}
/*************************************************************************************/
void gabedit_file_chooser_set_current_file(GabeditFileChooser *fileChooser,const gchar *fileName)
{

	GtkFileChooserAction action;
	g_return_if_fail (fileChooser != NULL);
	g_return_if_fail (GABEDIT_IS_FILE_CHOOSER(fileChooser));
	g_return_if_fail (fileName != NULL);

	action = gtk_file_chooser_get_action((GtkFileChooser *)fileChooser);
	gtk_file_chooser_set_filename ((GtkFileChooser *)fileChooser, fileName);
	if(action == GTK_FILE_CHOOSER_ACTION_SAVE)
	{
		gchar* tmp = g_path_get_basename(fileName);
		gtk_file_chooser_set_current_name ((GtkFileChooser *)fileChooser, tmp);
		if(tmp) g_free(tmp);
	}
}
/********************************************************************************/
gchar* gabedit_file_chooser_get_current_file(GabeditFileChooser *fileChooser)
{
	return gtk_file_chooser_get_filename((GtkFileChooser *)fileChooser);
}
/*****************************************************************************************/
void gabedit_file_chooser_set_filters(GabeditFileChooser *fileChooser,gchar **patterns)
{
	GtkFileFilter *filter;
	GtkFileFilter *filter0 = NULL;
	gint n = 0;

	g_return_if_fail (fileChooser != NULL);
	g_return_if_fail (GABEDIT_IS_FILE_CHOOSER(fileChooser));
	g_return_if_fail (patterns != NULL);
	while(patterns[n])
	{
		filter = gtk_file_filter_new ();
		gtk_file_filter_set_name (filter, patterns[n]);
		gtk_file_filter_add_pattern (filter, patterns[n]);
		gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fileChooser), filter);
		if(n==0) filter0 = filter;
		n++;
	}
	if(filter0)gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (fileChooser), filter0);
}
/*************************************************************************************/
void gabedit_file_chooser_set_filter(GabeditFileChooser *fileChooser, const gchar *pattern)
{
	GtkFileFilter *filter;

	g_return_if_fail (fileChooser != NULL);
	g_return_if_fail (GABEDIT_IS_FILE_CHOOSER(fileChooser));
	g_return_if_fail (pattern != NULL);

	filter = gtk_file_filter_new ();
	gtk_file_filter_set_name (filter, pattern);
	gtk_file_filter_add_pattern (filter, pattern);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fileChooser), filter);
	gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (fileChooser), filter);
}
/*************************************************************************************/
void gabedit_file_chooser_show_hidden(GabeditFileChooser *fileChooser)
{
	
	/*
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER (fileChooser), TRUE);
	*/
}
/*************************************************************************************/
void gabedit_file_chooser_hide_hidden(GabeditFileChooser *fileChooser)
{
	/*
	gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER (fileChooser), FALSE);
	*/
}
/*************************************************************************************/
