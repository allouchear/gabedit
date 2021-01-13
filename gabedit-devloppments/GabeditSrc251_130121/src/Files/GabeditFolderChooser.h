/**********************************************************************************************************
Copyright (c) 2002-2013 Abdul-Rahman Allouche. All rights reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation dir (the Gabedit), to deal in the Software without restriction, including without limitation
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

#ifndef __GABEDIT_GABEDITFOLDERCHOOSER_H__
#define __GABEDIT_GABEDITFOLDERCHOOSER_H__

#define GABEDIT_TYPE_FOLDER_CHOOSER            (gtk_file_chooser_get_type())
#define GABEDIT_FOLDER_CHOOSER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GABEDIT_TYPE_FOLDER_CHOOSER,GabeditFolderChooser))
#define GABEDIT_FOLDER_CHOOSER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), GABEDIT_TYPE_FOLDER_CHOOSER, GabeditFolderChooser))
#define GABEDIT_IS_FOLDER_CHOOSER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GABEDIT_TYPE_FOLDER_CHOOSER))
#define GABEDIT_IS_FOLDER_CHOOSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), GABEDIT_TYPE_FOLDER_CHOOSER))
#define GABEDIT_FOLDER_CHOOSER_GET_CLASS(obj)  (G_TYPE_CHECK_GET_CLASS ((obj), GABEDIT_TYPE_FOLDER_CHOOSER, GabeditFolderChooserClass))

typedef struct _GtkFileChooser     GabeditFolderChooser;
typedef struct _GtkFileChooserClass     GabeditFolderChooserClass;

GtkWidget* gabedit_folder_chooser_new(gchar* title);
void gabedit_folder_chooser_set_current_folder(GabeditFolderChooser *folderChooser,const gchar *folderName);
gchar* gabedit_folder_chooser_get_current_folder(GabeditFolderChooser *folderChooser);
void gabedit_folder_chooser_set_filters(GabeditFolderChooser *folderChooser,gchar **patterns);

#endif /* __GABEDIT_GABEDITFOLDERCHOOSER_H__ */

