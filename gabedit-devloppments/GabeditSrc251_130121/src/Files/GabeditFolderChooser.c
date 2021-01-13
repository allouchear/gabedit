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
#include "GabeditFolderChooser.h"

/********************************************************************************/
GtkWidget* gabedit_folder_chooser_new(gchar* title)
{
	gboolean multiple = FALSE;
	GtkWidget *dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", action, "file-system-backend", "gtk+", "select-multiple", multiple, NULL);
	if(title) gtk_window_set_title (GTK_WINDOW (dialog), title);
	gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	return GTK_WIDGET(dialog);

}
/*************************************************************************************/
void gabedit_folder_chooser_set_current_folder(GabeditFolderChooser *folderChooser,const gchar *folderName)
{

	g_return_if_fail (folderChooser != NULL);
	g_return_if_fail (GABEDIT_IS_FOLDER_CHOOSER(folderChooser));
	g_return_if_fail (folderName != NULL);
	gtk_file_chooser_set_filename ((GtkFileChooser *)folderChooser, folderName);
	/*
	gtk_file_chooser_set_current_folder ((GtkFileChooser *)folderChooser, folderName);
	*/
}
/********************************************************************************/
gchar* gabedit_folder_chooser_get_current_folder(GabeditFolderChooser *folderChooser)
{
	return gtk_file_chooser_get_current_folder((GtkFileChooser *)folderChooser);
}
/*****************************************************************************************/
void gabedit_folder_chooser_set_filters(GabeditFolderChooser *folderChooser,gchar **patterns)
{
	GtkFileFilter *filter;
	gint n = 0;

	g_return_if_fail (folderChooser != NULL);
	g_return_if_fail (GABEDIT_IS_FOLDER_CHOOSER(folderChooser));
	g_return_if_fail (patterns != NULL);
	while(patterns[n])
	{
		filter = gtk_file_filter_new ();
		gtk_file_filter_set_name (filter, patterns[n]);
		gtk_file_filter_add_pattern (filter, patterns[n]);
		gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (folderChooser), filter);
		n++;
	}
}
/*************************************************************************************/
