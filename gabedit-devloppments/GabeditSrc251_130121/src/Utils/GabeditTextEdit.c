/* GabeditTextEdit.c */
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
#include "../Common/GabeditType.h"

/************************************************************************************************************/
GtkWidget* gabedit_text_new()
{
	GtkWidget* text = gtk_text_view_new();
	GtkTextBuffer *buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	/* GtkTextTag *found_text_tag = gtk_text_buffer_create_tag (buffer, NULL, "foreground", "red", NULL);*/
	GtkTextTag *found_text_tag = gtk_text_buffer_create_tag (buffer, NULL, "background", "red", NULL);
	g_object_set_data(G_OBJECT(text),"FoundTextTag",found_text_tag);
	return text;
}
/************************************************************************************************************/
void gabedit_text_insert(GtkWidget* text, GdkFont* font, GdkColor* colorFore, GdkColor* colorBack, const gchar* str, gint l)
{
		GtkTextBuffer *buffer;
		GtkTextIter match_start;
		GtkTextIter match_end;
		gint char_offset  = 0;
		if(!GTK_IS_TEXT_VIEW (text)) return;
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
		gtk_text_buffer_get_iter_at_mark (buffer, &match_start, gtk_text_buffer_get_mark (buffer,"insert"));
		char_offset = gtk_text_iter_get_offset(&match_start);
		gtk_text_buffer_insert_at_cursor (buffer, str, l);
		gtk_text_buffer_get_iter_at_mark (buffer, &match_end, gtk_text_buffer_get_mark (buffer,"insert"));
		gtk_text_buffer_get_iter_at_mark (buffer, &match_start, gtk_text_buffer_get_mark (buffer,"insert"));
		gtk_text_iter_set_offset(&match_start, char_offset );

		if(colorFore)
		{
			GtkTextTag *tag = gtk_text_buffer_create_tag (buffer, NULL, "foreground_gdk", colorFore, NULL);
			gtk_text_buffer_apply_tag (buffer, tag, &match_start, &match_end);
			/* gtk_text_buffer_remove_tag (buffer,  tag, &match_start, &match_end);*/
		}
		if(colorBack)
		{
			GtkTextTag *tag = gtk_text_buffer_create_tag (buffer, NULL, "background_gdk", colorBack, NULL);
			gtk_text_buffer_apply_tag (buffer, tag, &match_start, &match_end);
			/* gtk_text_buffer_remove_tag (buffer,  tag, &match_start, &match_end);*/
		}
}
/************************************************************************************************************/
gint gabedit_text_get_length(GtkWidget* text)
{
		GtkTextBuffer *buffer;
		if(!GTK_IS_TEXT_VIEW (text)) return 0;
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
		return gtk_text_buffer_get_char_count  (buffer);
}
/************************************************************************************************************/
gchar* gabedit_text_get_chars(GtkWidget* text, gint b, gint e)
{
		GtkTextBuffer *buffer;
		GtkTextIter start;
		GtkTextIter end;
		GtkTextIter iterStart;
		GtkTextIter iterEnd;

		if(!GTK_IS_TEXT_VIEW (text)) return NULL;

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
		gtk_text_buffer_get_bounds (buffer, &start, &end);

		if(b>=0) gtk_text_buffer_get_iter_at_offset(buffer,&iterStart,b);
		else iterStart = start;

		if(e>=0) gtk_text_buffer_get_iter_at_offset(buffer,&iterEnd,e);
		else iterEnd = end;

		return gtk_text_iter_get_text(&iterStart, &iterEnd);
}
/************************************************************************************************************/
void gabedit_text_set_point(GtkWidget* text, gint char_offset)
{
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;
	GtkTextIter iter;

	if(!GTK_IS_TEXT_VIEW (text)) return;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	if(!buffer) return;
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	if(char_offset>=0) gtk_text_buffer_get_iter_at_offset(buffer,&iter,char_offset);
	else iter = end;

	gtk_text_buffer_place_cursor(buffer, &iter);
}
/************************************************************************************************************/
gint gabedit_text_get_point(GtkWidget* text)
{

	return 0;
}
/************************************************************************************************************/
void gabedit_text_forward_delete(GtkWidget* text, gint char_offset)
{
	GtkTextBuffer *buffer;
	GtkTextIter start;
	GtkTextIter end;
	GtkTextIter iterBegin;
	GtkTextIter iterEnd;

	if(!GTK_IS_TEXT_VIEW (text)) return;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	if(!buffer) return;
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	gtk_text_buffer_get_iter_at_mark (buffer, &iterBegin, gtk_text_buffer_get_mark (buffer,"insert"));
	if(char_offset>=0)
	{
		gint char_offset_begin = gtk_text_iter_get_offset(&iterBegin);
		gint char_offset_end = char_offset_begin + char_offset;
		iterEnd = end;
		gtk_text_iter_set_offset(&iterEnd, char_offset_end );
	}
	else
	iterEnd = end;

	gtk_text_buffer_delete(buffer, &iterBegin,&iterEnd);
}
/************************************************************************************************************/
void gabedit_text_set_editable(GtkWidget* text, gboolean editable)
{
	if(!GTK_IS_TEXT_VIEW (text)) return;
	gtk_text_view_set_editable((GtkTextView *)text, editable);
}
/************************************************************************************************************/
gint gabedit_text_search(GtkWidget *text, G_CONST_RETURN gchar *str, GtkTextIter* iter, gboolean forward)
{
	GtkTextTag *found_text_tag;
	GtkTextIter start;
	GtkTextIter end;
	GtkTextBuffer *buffer;
	GtkTextIter match_start;
	GtkTextIter match_end;

	if(!str) return -1;
	if(*str == '\0') return -1;

	if(!GTK_IS_TEXT_VIEW (text)) return -1;

	found_text_tag = (GtkTextTag *)g_object_get_data(G_OBJECT(text),"FoundTextTag");
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	gtk_text_buffer_remove_tag (buffer,  found_text_tag, &start, &end);

	if(!gtk_text_iter_in_range(iter, &start, &end)) return -1;

	if(forward)
	{
		gint i = 0;
		GtkTextIter posBegin;
		GtkTextIter posEnd;
		while(gtk_text_iter_forward_search (iter, str, GTK_TEXT_SEARCH_VISIBLE_ONLY | GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL))
		{
			gtk_text_buffer_apply_tag (buffer, found_text_tag, &match_start, &match_end);
			if(i==0)
			{
			       	posBegin = match_start;
			       	posEnd = match_end;
			}
			i++;
			iter = &match_end;
		}
		if(i>0)
		{
			gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW (text), &posBegin, 0.05, FALSE, 0.5, 0.5);
			gtk_text_buffer_place_cursor(buffer, &posEnd);
		}

		return i;
	}
	else
	{
		gint i = 0;
		GtkTextIter posBegin;
		GtkTextIter posEnd;
		while(gtk_text_iter_backward_search (iter, str, GTK_TEXT_SEARCH_VISIBLE_ONLY | GTK_TEXT_SEARCH_TEXT_ONLY, &match_start, &match_end, NULL))
		{
			gtk_text_buffer_apply_tag (buffer, found_text_tag, &match_start, &match_end);
			if(i==0)
			{
			       	posBegin = match_start;
			       	posEnd = match_end;
			}
			i++;
			iter = &match_start;
		}
		if(i>0)
		{
			gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW (text), &posBegin, 0.05, FALSE, 0.5, 0.5);
			gtk_text_buffer_place_cursor(buffer, &posBegin);
		}
		return i;
	}
	return -1;

}
/************************************************************************************************************/
gint gabedit_text_search_all(GtkWidget *text, G_CONST_RETURN gchar *str)
{
	GtkTextIter start;
	GtkTextIter end;
	GtkTextBuffer *buffer;

	if(!GTK_IS_TEXT_VIEW (text)) return -1;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	return gabedit_text_search(text, str, &start, TRUE);
}
/************************************************************************************************************/
gint gabedit_text_search_forward(GtkWidget *text, G_CONST_RETURN gchar *str)
{
	GtkTextIter iter;
	GtkTextBuffer *buffer;

	if(!GTK_IS_TEXT_VIEW (text)) return -1;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_mark (buffer,"insert"));
	return gabedit_text_search(text, str, &iter, TRUE);
}
/************************************************************************************************************/
gint gabedit_text_search_backward(GtkWidget *text, G_CONST_RETURN gchar *str)
{
	GtkTextIter iter;
	GtkTextBuffer *buffer;

	if(!GTK_IS_TEXT_VIEW (text)) return -1;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_text_buffer_get_iter_at_mark (buffer, &iter, gtk_text_buffer_get_mark (buffer,"insert"));
	return gabedit_text_search(text, str, &iter, FALSE);
}
/************************************************************************************************************/
void gabedit_text_goto_end(GtkWidget *text)
{
	
	GtkTextIter start;
	GtkTextIter end;
	GtkTextBuffer *buffer;

	if(!GTK_IS_TEXT_VIEW (text)) return;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_text_buffer_get_bounds (buffer, &start, &end);

	gtk_text_buffer_place_cursor(buffer, &end);
	gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(text),gtk_text_buffer_get_insert(buffer));

}
/************************************************************************************************************/
void gabedit_text_goto_line(GtkWidget *text, gint line)
{
	GtkTextIter iter;
	GtkTextBuffer *buffer;

	if(!GTK_IS_TEXT_VIEW (text)) return;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_text_buffer_get_iter_at_line(buffer, &iter, (gint)line);
	gtk_text_buffer_place_cursor(buffer, &iter);
	gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(text),gtk_text_buffer_get_insert(buffer));
}
/************************************************************************************************************/
void gabedit_text_select_all(GtkWidget* text)
{
	GtkTextBuffer *buffer;
	GtkTextIter start_iter;
	GtkTextIter end_iter;

	if(!GTK_IS_TEXT_VIEW (text)) return;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	gtk_text_buffer_get_bounds (buffer, &start_iter, &end_iter);
	gtk_text_buffer_select_range (buffer, &start_iter, &end_iter);
}
/************************************************************************************************************/
void gabedit_text_cut_clipboard (GtkWidget* text)
{
	GtkTextBuffer *buffer;
	if(!GTK_IS_TEXT_VIEW (text)) return;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	GtkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text), GDK_SELECTION_CLIPBOARD);
  
	gtk_text_buffer_cut_clipboard (buffer, clipboard, GTK_TEXT_VIEW(text)->editable);
	gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(text), gtk_text_buffer_get_mark (buffer, "insert"));
}
/************************************************************************************************************/
void gabedit_text_copy_clipboard (GtkWidget* text)
{
	GtkTextBuffer *buffer;
	if(!GTK_IS_TEXT_VIEW (text)) return;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	GtkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text), GDK_SELECTION_CLIPBOARD);
  
	gtk_text_buffer_copy_clipboard (buffer, clipboard);
	gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(text), gtk_text_buffer_get_mark (buffer, "insert"));
}

/************************************************************************************************************/
void gabedit_text_paste_clipboard (GtkWidget* text)
{
	GtkTextBuffer *buffer;
	if(!GTK_IS_TEXT_VIEW (text)) return;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	GtkClipboard *clipboard = gtk_widget_get_clipboard (GTK_WIDGET (text), GDK_SELECTION_CLIPBOARD);
  
	gtk_text_buffer_paste_clipboard (buffer, clipboard, NULL, GTK_TEXT_VIEW(text)->editable);
	gtk_text_view_scroll_mark_onscreen (GTK_TEXT_VIEW(text), gtk_text_buffer_get_mark (buffer, "insert"));
}
