/* FolderChooser.c */
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
#include <GL/gl.h>
#include <GL/glu.h>
#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Common/Windows.h"
#include "../Common/Exit.h"
#include "../Display/UtilsOrb.h"
#include "../Files/GabeditFolderChooser.h"

/************************************************************************************************************/
GtkWidget* selectionOfDir(gpointer data, gchar* title, GabEditTypeWin typewin) 
{
	GtkWidget *folderChooser;
	GCallback *func = (GCallback *)data;
	gchar* lastDir = get_last_directory();

	if(title) folderChooser = gabedit_folder_chooser_new(title);
	else folderChooser = gabedit_folder_chooser_new(_("Select a folder"));

	if(lastDir)
	{
  		gabedit_folder_chooser_set_current_folder( GABEDIT_FOLDER_CHOOSER(folderChooser),lastDir);
	}
  	if(typewin==GABEDIT_TYPEWIN_ORB)
	{
  		add_glarea_child(folderChooser,_("Dir selector"));
	}
	else
	{
  		add_button_windows(_(" Dir selector "),folderChooser);
  		g_signal_connect(G_OBJECT(folderChooser), "delete_event",(GCallback)destroy_button_windows,NULL);
  		g_signal_connect(G_OBJECT(folderChooser), "delete_event",G_CALLBACK(gtk_widget_destroy),NULL);
	}

	g_signal_connect (folderChooser, "response", G_CALLBACK (func), NULL);

	if(typewin==GABEDIT_TYPEWIN_ORB)
		g_signal_connect_swapped (folderChooser, "response", G_CALLBACK (delete_child), GTK_OBJECT(folderChooser));
	else
		g_signal_connect_swapped (folderChooser, "response", G_CALLBACK (destroy_button_windows), GTK_OBJECT(folderChooser));
	g_signal_connect_swapped (folderChooser, "response", G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(folderChooser));

	if(typewin==GABEDIT_TYPEWIN_ORB)
		g_signal_connect_swapped (folderChooser, "close", G_CALLBACK (delete_child), GTK_OBJECT(folderChooser));
	else
		g_signal_connect_swapped (folderChooser, "close", G_CALLBACK (destroy_button_windows), GTK_OBJECT(folderChooser));
	g_signal_connect_swapped (folderChooser, "close", G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(folderChooser));

	gtk_widget_show(folderChooser);

	return folderChooser;
}
