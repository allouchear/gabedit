/* TextEdit.c */
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
#include <string.h>

#include "Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/GabeditTextEdit.h"
#include "../Geometry/ResultsAnalise.h"
#include "Windows.h"

#define COUPER            7
#define COPIER            8
#define COLLER            9
#define SELECTALL         10
#define FIND              11

#define DIML 10
/********************************************************************************/
gchar *liste[DIML];
guint nliste = 1;
void find_text_win();
/******************************************************************************/
static void change_liste(G_CONST_RETURN gchar *str)
{
	gboolean OK;
	guint i;
	OK=FALSE;
	for(i=0;i<nliste;i++)
		if(!strcmp(str,liste[i]) )
		{
			OK=TRUE;
			break;
		}
	if(!OK) {
		if(nliste<DIML)
		     nliste++;
		for(i=nliste-1;i>0;i--)
				 liste[i]=liste[i-1];
		liste[0]=g_strdup(str);
	}

}
/********************************************************************************/
static void find_text_forward(GtkWidget *wid,gpointer entry)
{
	GtkWidget* view;
	GtkWidget* label;
	gchar* t;
	G_CONST_RETURN gchar *str;
	guint i;
	gint n;
	str = gtk_entry_get_text(GTK_ENTRY(entry));
	change_liste(str);
	i=gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBookText));
	view = text;
	if(i==1)view = textresult;

  	label = g_object_get_data(G_OBJECT (entry), "Label");
	gtk_label_set_text(GTK_LABEL(label)," ");

	n = gabedit_text_search_forward(view, str);

	if(n>0) t = g_strdup_printf(_("      %d strings found and marked in red"),n);
	else t = g_strdup_printf(_("      no strings found"));
	gtk_label_set_text(GTK_LABEL(label),t);
	g_free(t);
}
/********************************************************************************/
static void find_text_backward(GtkWidget *wid,gpointer entry)
{
	GtkWidget* view;
	GtkWidget* label;
	gchar* t;
	G_CONST_RETURN gchar *str;
	guint i;
	gint n;
	str = gtk_entry_get_text(GTK_ENTRY(entry));
	change_liste(str);
	i=gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBookText));
	view = text;
	if(i==1)view = textresult;

  	label = g_object_get_data(G_OBJECT (entry), "Label");
	gtk_label_set_text(GTK_LABEL(label)," ");

	n = gabedit_text_search_backward(view, str);

	if(n>0) t = g_strdup_printf(_("      %d strings found and marked in red"),n);
	else t = g_strdup_printf(_("      no strings found"));
	gtk_label_set_text(GTK_LABEL(label),t);
	g_free(t);
}
/*****************************************************************************************/
static void find_text_all(GtkWidget *wid,gpointer entry)
{
	GtkWidget* view;
	GtkWidget* label;
	gchar* t;
	G_CONST_RETURN gchar *str;
	guint i;
	gint n;
	str = gtk_entry_get_text(GTK_ENTRY(entry));
	change_liste(str);
	i=gtk_notebook_get_current_page(GTK_NOTEBOOK(NoteBookText));
	view = text;
	if(i==1)view = textresult;

  	label = g_object_get_data(G_OBJECT (entry), "Label");
	gtk_label_set_text(GTK_LABEL(label)," ");

	n = gabedit_text_search_all(view, str);

	if(n>0) t = g_strdup_printf(_("      %d strings found and marked in red"),n);
	else t = g_strdup_printf(_("      no strings found"));
	gtk_label_set_text(GTK_LABEL(label),t);
	g_free(t);
}
/*****************************************************************************************/
GtkWidget *create_combo_box_entry_liste_find(GtkWidget* Window,GtkWidget* hbox,gchar *lname,gchar **liste,int n)
{
  int i;
  GtkWidget *label;
  GtkWidget *combo;
  GtkWidget *combo_entry;
  GList *combo_items = NULL;
  label = gtk_label_new (lname);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE,FALSE, 2);

  combo = gtk_combo_box_entry_new_text ();
  gtk_widget_set_size_request(combo, (gint)(ScreenHeight*0.150), -1);
  gtk_widget_show (combo);
  gtk_box_pack_start (GTK_BOX (hbox), combo, TRUE, TRUE, 2);
  for (i=0;i<n;i++) combo_items = g_list_append (combo_items, liste[i]);
  gtk_combo_box_entry_set_popdown_strings (combo, combo_items);
  g_list_free (combo_items);

  combo_entry = GTK_BIN (combo)->child;
  gtk_widget_show (combo_entry);
  gtk_entry_set_text (GTK_ENTRY (combo_entry), liste[0]);
  return combo_entry;
 }
/*****************************************************************************************/
void find_text_win()
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *entry;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *hseparator;
  
  if(nliste==1)liste[0]=g_strdup("");
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Fenetre));
  gtk_window_set_title(GTK_WINDOW(fp),"Search");
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Fenetre));
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  g_signal_connect(G_OBJECT(fp), "delete_event", (GCallback)destroy_button_windows, NULL);
  g_signal_connect(G_OBJECT(fp), "delete_event", (GCallback)gtk_widget_destroy, NULL);

  vboxall = create_vbox(fp);
  frame = gtk_frame_new (_("FIND"));
  gtk_container_set_border_width (GTK_CONTAINER (frame), 2);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  hbox1 = gtk_hbox_new (FALSE, 5);
  g_object_ref (hbox1);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vboxframe), hbox1, FALSE, FALSE, 2);
  entry= create_combo_box_entry_liste_find(fp,hbox1,_(" String to find :"),liste,nliste);


  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_box_pack_start (GTK_BOX (vboxframe), hseparator, FALSE, FALSE, 1);

  label = gtk_label_new(" ");
  gtk_box_pack_start (GTK_BOX (vboxframe), label, FALSE, FALSE, 4);
  g_object_set_data(G_OBJECT (entry), "Label", label);

  hbox2 = gtk_hbox_new (TRUE, 0);
  g_object_ref (hbox2);
  gtk_widget_show (hbox2);
  gtk_box_pack_start (GTK_BOX (vboxall), hbox2, FALSE, FALSE, 4);
  
  button = gtk_button_new_with_label (_("All"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(find_text_all),(gpointer)entry);

  button = gtk_button_new_with_label (_("Forward"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(find_text_forward),(gpointer)entry);

  button = gtk_button_new_with_label (_("Backward"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(find_text_backward),(gpointer)entry);


  button = gtk_button_new_with_label ("Close");
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button),"clicked",
                       (GCallback)destroy_button_windows,
                       GTK_OBJECT(fp));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",
                     G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(fp));

  gtk_widget_show (button);
   
  add_button_windows(_(" Find "),fp);
  gtk_widget_show_all(fp);
}
/*****************************************************************************************/
GtkWidget *AddBarResult(GtkWidget *Frame)
{
  GtkWidget *hboxframe;
  GtkWidget *frame;
  GtkWidget *vbox;

/* For result NoteBook */
  hboxframe = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hboxframe);
  gtk_container_add(GTK_CONTAINER(Frame),hboxframe);

  vbox = gtk_vbox_new (TRUE, 0);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX(hboxframe), vbox, TRUE, TRUE, 2);

  frame = gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(frame), 2);
  gtk_container_add(GTK_CONTAINER(vbox),frame);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX(hboxframe), vbox, FALSE, FALSE, 2);

  create_bar_result(vbox);
  
  return frame;

}
/*****************************************************************************************/
void AjoutePageNotebook(char *label,GtkWidget **TextP)
{
  GtkWidget *scrolledwindow;
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkStyle *stylered;
  GtkStyle *styledef;
  

  Frame = gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 2);

  LabelOnglet = gtk_label_new(label);
  LabelMenu = gtk_label_new(label);
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBookText),
                                Frame,
                                LabelOnglet, LabelMenu);

  g_object_set_data(G_OBJECT (Frame), "Frame", Frame);

  if(strcmp(label,_("Data")) )
        Frame = AddBarResult(Frame);    
  scrolledwindow = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow);
  gtk_container_add (GTK_CONTAINER (Frame), scrolledwindow);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER,GTK_POLICY_AUTOMATIC );

  *TextP = gabedit_text_new ();
  set_tab_size (*TextP, 8);
  /* gabedit_text_set_word_wrap (GABEDIT_TEXT(*TextP), FALSE);*/
  
/*
  GABEDIT_TEXT(*TextP)->default_tab_width=7;
*/

  g_object_ref (*TextP);
  g_object_set_data_full (G_OBJECT (Frame), "text", *TextP, (GDestroyNotify) g_object_unref);
  gtk_widget_show (*TextP);
  gtk_container_add (GTK_CONTAINER (scrolledwindow), *TextP);
  g_object_set_data(G_OBJECT (*TextP), "LabelOnglet", LabelOnglet);
  if( !strcmp(label,_("Data")) )
  {
	static guint start_pos = 0;
	static guint end_pos = 0;

        styledef =  gtk_style_copy(LabelOnglet->style); 
        stylered =  gtk_style_copy(LabelOnglet->style); 
        stylered->fg[0].red=65535;
        stylered->fg[0].green=0;
        stylered->fg[0].blue=0;
	g_object_set_data(G_OBJECT (*TextP), "StyleDef", styledef);
  	g_object_set_data(G_OBJECT (*TextP), "StyleRed", stylered);
  	g_object_set_data(G_OBJECT (*TextP), "StartPos", &start_pos);
  	g_object_set_data(G_OBJECT (*TextP), "EndPos", &end_pos);
  }
}
/*****************************************************************************************/
void view_result_calcul(GtkWidget *noteb,gpointer d)
{
  int numpage;
  numpage = gtk_notebook_get_current_page ((GtkNotebook *)noteb);
  if(numpage == 0)
    get_result();
}
/*****************************************************************************************/
void set_imodif(GtkWidget *wid, gpointer data)
{
 	data_modify(TRUE);
}     
/*****************************************************************************************/
void cree_text_notebook()
{
  GtkWidget *hbox;

  hbox =create_hbox_false(vboxtexts);

  iedit=1;
  NoteBookText = gtk_notebook_new();
  g_object_set_data_full (G_OBJECT (NoteBookText), "Fenetre", Fenetre, (GDestroyNotify) g_object_unref);
  g_signal_connect(G_OBJECT(NoteBookText),"switch_page", (GCallback)view_result_calcul,NULL);

  gtk_box_pack_start(GTK_BOX (vboxtexts), NoteBookText,TRUE, TRUE, 0);
  
  AjoutePageNotebook(_("Data"),&text);
  gabedit_text_set_editable (GABEDIT_TEXT (text), TRUE);
  set_font (text,FontsStyleData.fontname);
  set_base_style(text,FontsStyleData.BaseColor.red ,FontsStyleData.BaseColor.green ,FontsStyleData.BaseColor.blue);
  set_text_style(text,FontsStyleData.TextColor.red ,FontsStyleData.TextColor.green ,FontsStyleData.TextColor.blue);

  AjoutePageNotebook(_("Result"),&textresult);
  gabedit_text_set_editable (GABEDIT_TEXT (textresult), TRUE);
  set_font (textresult,FontsStyleResult.fontname);
  set_base_style(textresult,FontsStyleResult.BaseColor.red ,FontsStyleResult.BaseColor.green ,FontsStyleResult.BaseColor.blue);
  set_text_style(textresult,FontsStyleResult.TextColor.red ,FontsStyleResult.TextColor.green ,FontsStyleResult.TextColor.blue);

  {
	GtkTextBuffer *buffer;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (text));
	if(buffer) g_signal_connect(G_OBJECT(buffer),"changed", (GCallback)set_imodif,NULL);
  }
}
/*****************************************************************************************/
