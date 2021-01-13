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

#ifndef __GABEDIT_FILECHOOSER_H__
#define __GABEDIT_FILECHOOSER_H__

GtkWidget* file_chooser(gpointer data,gchar* title,GabEditTypeFile type,GabEditTypeWin typewin, GtkFileChooserAction action);
GtkWidget* file_chooser_open(gpointer data,gchar* title,GabEditTypeFile type,GabEditTypeWin typewin);
GtkWidget* file_chooser_save(gpointer data,gchar* title,GabEditTypeFile type,GabEditTypeWin typewin);
void choose_file_to_open();
void choose_file_to_insert();
void choose_file_to_save();
void choose_file_to_save_end();
void Save_YesNo();
void set_entry_selected_file(GtkWidget* hbox);
GtkWidget* choose_file_to_create(gchar* title, GCallback applyFunc);

#endif /* __GABEDIT_FILECHOOSER_H__ */

