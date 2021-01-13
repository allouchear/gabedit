/* GabeditTextEdit.h */
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

#ifndef __GABEDIT_GABEDITTEXTEDIT_H__
#define __GABEDIT_GABEDITTEXTEDIT_H__

#define GABEDIT_TEXT                 (GtkWidget*)
GtkWidget* gabedit_text_new();
void gabedit_text_insert(GtkWidget* text, GdkFont* font, GdkColor* colorFore, GdkColor* colorBack, const gchar* str, gint l);
gint gabedit_text_get_length(GtkWidget* text);
gchar* gabedit_text_get_chars(GtkWidget* text, gint begin, gint end);
void gabedit_text_set_point(GtkWidget* text, gint char_offset);
gint gabedit_text_get_point(GtkWidget* text);
void gabedit_text_forward_delete(GtkWidget* text, gint char_offset);
void gabedit_text_set_editable(GtkWidget* text, gboolean editable);
gint gabedit_text_search(GtkWidget *text, G_CONST_RETURN gchar *str, GtkTextIter* iter, gboolean forward);
gint gabedit_text_search_all(GtkWidget *text, G_CONST_RETURN gchar *str);
gint gabedit_text_search_forward(GtkWidget *text, G_CONST_RETURN gchar *str);
gint gabedit_text_search_backward(GtkWidget *text, G_CONST_RETURN gchar *str);
void gabedit_text_goto_end(GtkWidget *text);
void gabedit_text_goto_line(GtkWidget *text, gint line);
void gabedit_text_select_all(GtkWidget* text);
void gabedit_text_cut_clipboard (GtkWidget* text);
void gabedit_text_copy_clipboard (GtkWidget* text);
void gabedit_text_paste_clipboard (GtkWidget* text);

#endif /* __GABEDIT_GABEDITTEXTEDIT_H__ */

