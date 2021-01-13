/* GInterfaceLink.c */
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "GaussGlobal.h"

/*****************************************************************/
static void gene_one_entry2(GtkWidget *b,gpointer data)
{
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext;
  gchar *t;
  entry=(GtkWidget *)data;
  entrytext = gtk_entry_get_text(GTK_ENTRY(entry));

  if(!StrLink)
  	StrLink = g_strdup_printf("%s\n",entrytext);
  else
  {
	t = g_strdup(StrLink);
  	StrLink = g_strdup_printf("%s%s\n",t,entrytext);
	g_free(t);
  }
}
/*****************************************************************/
static void gene_one_entry1(GtkWidget *b,gpointer data)
{
  gchar *t;
  if(!StrLink)
  	StrLink = g_strdup_printf("%s",(char*)data);
  else
  {
	t = g_strdup(StrLink);
  	StrLink = g_strdup_printf("%s%s",t,(char*)data);
	g_free(t);
  }
}
/*****************************************************************/
static void c_one_entry (GtkWidget *bframe,gchar *titre,gchar *tlabel,gchar *mode,gchar *set,guint del)
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget *entry;
  GtkWidget *Wins =  GTK_WIDGET(g_object_get_data (G_OBJECT (bframe), "Window"));
  
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_title(GTK_WINDOW(fp),titre);
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);
  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Wins));
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  add_child(Wins,fp,gtk_widget_destroy,tlabel);
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)delete_child,NULL);

  vboxall = create_vbox(fp);
  frame = gtk_frame_new (titre);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
   hbox1 = create_hbox(vboxframe);
  label = gtk_label_new (tlabel);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox1), label, TRUE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (hbox1), entry, FALSE, TRUE, 0);
  gtk_entry_set_text(GTK_ENTRY(entry),set);

  hbox2 = create_hbox(vboxall);
  gtk_widget_realize(fp);

  button = create_button(fp,_("Cancel"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(fp,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(gene_one_entry1),(gpointer)mode);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(gene_one_entry2),(gpointer)entry);
  if (!strcmp((char *)titre,_("The checkpoint file")) )
  {
  	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_show),GTK_OBJECT(CheckButtons[7]));
  	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_show),GTK_OBJECT(CheckButtons[8]));
  }

  if(del) g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(bframe));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));
   
  gtk_widget_show_all(fp);
}
/*****************************************************************/
static void sorry(GtkWidget *bframe,gpointer data)
{
 char *temp;
 temp=g_malloc(100);
  sprintf(temp,_("Sorry, the button of \"%s\" is not active"),(char*)data);
   Message(temp," Warning ",TRUE);
   gtk_widget_hide(bframe);
 g_free(temp);
}
/*****************************************************************/
static void Traite_Link_Option(GtkWidget *bframe,gpointer data)
{
  if (!strcmp((char *)data,_("Dynamic memory")) )
   c_one_entry(bframe,(char *)data,_("Memory size : "),"%Mem=","4MW",1);
  else
  if (!strcmp((char *)data,_("Read-Write file")) )
   c_one_entry(bframe,(char *)data,_("File Name : "), "%RWF=","rwffile",1);
  else 
  if (!strcmp((char *)data,_("2E derivative file")) )
   c_one_entry(bframe,(char *)data,_("File Name : "), "%d2I=","d2intfile",1);
  else 
  if (!strcmp((char *)data,_("The checkpoint file")) )
   c_one_entry(bframe,(char *)data,_("File Name : "),"%Chk=","chkfile",1);
  else 
  if (!strcmp((char *)data,_("2E integral file")) )
   c_one_entry(bframe,(char *)data,_("File Name : "),"%Int=","intfile",1);
  else 
  if (!strcmp((char *)data,"") )
    sorry(bframe,data);
  else 
    sorry(bframe,data);
}
/*****************************************************************/
void create_button_link(GtkWidget *w,GtkWidget *Wins)
{
	GtkWidget* Table;
	GtkWidget* button;
	guint i;
	guint j;
        guint ColonneT=3; 
        guint LigneT=2; 
	char *LabelButton[3][2]={
        {N_("Dynamic memory"),
         N_("Read-Write file"),
        },
        {N_("The checkpoint file"),
	 N_("2E derivative file"),
         } ,
	{_("2E integral file"),
         "00"
         }
	};

  StrLink = NULL;

  Table = gtk_table_new(LigneT,ColonneT,TRUE);
  gtk_container_add(GTK_CONTAINER(w),Table);
  
  for ( i = 0;i<LigneT;i++)
	  for ( j = 0;j<ColonneT;j++)
  {
    if(strcmp(LabelButton[j][i],"00"))
          {
	  button = gtk_button_new_with_label(LabelButton[j][i]);
  	  g_object_set_data(G_OBJECT (button), "Window", Wins);
          g_signal_connect(G_OBJECT(button), "clicked",(GCallback)Traite_Link_Option,(gpointer )LabelButton[j][i]);

	  gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
          gtk_widget_show (button);
         }

  }
  gtk_widget_show (Table);
 	
}
