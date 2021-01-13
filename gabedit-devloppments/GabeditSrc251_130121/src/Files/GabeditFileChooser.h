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

#ifndef __GABEDIT_GABEDITFILECHOOSER_H__
#define __GABEDIT_GABEDITFILECHOOSER_H__

#define GABEDIT_TYPE_FILE_CHOOSER            (gtk_file_chooser_get_type())
#define GABEDIT_FILE_CHOOSER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), GABEDIT_TYPE_FILE_CHOOSER,GabeditFileChooser))
#define GABEDIT_FILE_CHOOSER_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), GABEDIT_TYPE_FILE_CHOOSER, GabeditFileChooser))
#define GABEDIT_IS_FILE_CHOOSER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GABEDIT_TYPE_FILE_CHOOSER))
#define GABEDIT_IS_FILE_CHOOSER_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), GABEDIT_TYPE_FILE_CHOOSER))
#define GABEDIT_FILE_CHOOSER_GET_CLASS(obj)  (GTK_CHECK_GET_CLASS ((obj), GABEDIT_TYPE_FILE_CHOOSER, GabeditFileChooserClass))

typedef struct _GtkFileChooser     GabeditFileChooser;
typedef struct _GtkFileChooserClass     GabeditFileChooserClass;

GtkWidget* gabedit_file_chooser_new(gchar* title, GtkFileChooserAction action);
void gabedit_file_chooser_set_current_file(GabeditFileChooser *fileChooser,const gchar *fileName);
gchar* gabedit_file_chooser_get_current_file(GabeditFileChooser *fileChooser);
void gabedit_file_chooser_set_filters(GabeditFileChooser *fileChooser, gchar **patterns);
void gabedit_file_chooser_set_filter(GabeditFileChooser *fileChooser,const gchar *pattern);
void gabedit_file_chooser_show_hidden(GabeditFileChooser *fileChooser);
void gabedit_file_chooser_hide_hidden(GabeditFileChooser *fileChooser);

#endif /* __GABEDIT_GABEDITFILECHOOSER_H__ */

