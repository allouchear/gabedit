/* MInterfaceComm.c */
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
#include "../Utils/GabeditTextEdit.h"
#include "../Geometry/GeomGlobal.h"
#include "MInterfaceComm.h"

static GtkWidget *combo_entry1;
static GtkWidget *text1;
static GtkWidget *text2;
static  GtkWidget *Window;
static  GtkWidget *VboxOption;
static  GtkWidget *FrameShow;
static  GtkWidget *Wins;
static int begin = 1;

/********************************************************************************************************/
static void gene_ref(GtkWidget *b,gpointer data)
{
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext;
  entry=(GtkWidget *)data;
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "Ref",-1);
  entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, ",",-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, entrytext,-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "; ",-1);
}
/********************************************************************************************************/
static void gene_nstate(GtkWidget *b,gpointer data)
{
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext;
  entry=(GtkWidget *)data;
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "State",-1);
  entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, ",",-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, entrytext,-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "; ",-1);
}
/********************************************************************************************************/
static void gene_guess(GtkWidget *b,gpointer data)
{
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext;
  entry=(GtkWidget *)data;
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "Start",-1);
  entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, ",",-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, entrytext,-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "; ",-1);
}
/********************************************************************************************************/
static void gene_entry(GtkWidget *b,gpointer data)
{
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext;
  entry=(GtkWidget *)data;
  entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, entrytext,-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "; ",-1);
}
/********************************************************************************************************/
static void c_restrictexcitation(GtkWidget *bframe,guint del)
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *entry;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *button;
  gchar *liste[3];
  gchar *titre=_("Restriction of classes of excitations");

  int nliste = 3;
  liste[0]=g_strdup("NOPAIR");
  liste[1]=g_strdup("NOSINGLE");
  liste[2]=g_strdup("NOEXC");
  /* Fenetre principale */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),titre);
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Wins));
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  add_child(Wins,fp,gtk_widget_destroy,titre);
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)delete_child,NULL);


  vboxall = create_vbox(fp);
  frame = gtk_frame_new (titre);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  hbox1 = create_hbox(vboxframe);
  entry= create_combo_box_entry_liste(Window,hbox1," Select : ",liste,nliste);

  hbox2 = create_hbox(vboxall);
  gtk_widget_realize(Window);

  button = create_button(Window,_("Cancel"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(Window,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(gene_entry),(gpointer)entry);
  if(del) g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(bframe));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));

   
  gtk_widget_show_all(fp);
}
/********************************************************************************************************/
static void c_type_orb(GtkWidget *bframe,guint del)
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *entry;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *button;
  gchar *liste[3];
  int nliste = 3;
  gchar *titre=_("Type of orbital to save");

  liste[0]=g_strdup("NatOrb");
  liste[1]=g_strdup("CanOrb");
  liste[2]=g_strdup("LocOrb");

  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),titre);
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Wins));
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  add_child(Wins,fp,gtk_widget_destroy,titre);
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)delete_child,NULL);


  vboxall = create_vbox(fp);
  frame = gtk_frame_new (titre);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  hbox1 = create_hbox(vboxframe);
  entry= create_combo_box_entry_liste(Window,hbox1,_(" Type of robital :"),liste,nliste);
  
  hbox2 = create_hbox(vboxall);
  gtk_widget_realize(Window);

  button = create_button(Window,_("Cancel"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(Window,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(gene_entry),(gpointer)entry);
  if(del) g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(bframe));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));

   
  gtk_widget_show_all(fp);
}
/********************************************************************************************************/
static void c_ref(GtkWidget *bframe,guint del)
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *entry;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *button;
  gchar *liste[8];
  int nliste = 8;
  gchar *titre =_("Additional reference symmetries");
  liste[0]=g_strdup("1");
  liste[1]=g_strdup("2");
  liste[2]=g_strdup("3");
  liste[3]=g_strdup("4");
  liste[4]=g_strdup("5");
  liste[5]=g_strdup("6");
  liste[6]=g_strdup("7");
  liste[7]=g_strdup("8");
  /* Fenetre principale */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),titre);
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Wins));
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  add_child(Wins,fp,gtk_widget_destroy,titre);
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)delete_child,NULL);

  vboxall = create_vbox(fp);
  frame = gtk_frame_new (titre);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  hbox1 = create_hbox(vboxframe);
  entry= create_combo_box_entry_liste(Window,hbox1,_("Additional reference symmetries  :"),liste,nliste);
  
  hbox2 = create_hbox(vboxall);
  gtk_widget_realize(Window);

  button = create_button(Window,_("Cancel"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(Window,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(gene_ref),(gpointer)entry);
  if(del) g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(bframe));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));

   
  gtk_widget_show_all(fp);
}
/********************************************************************************************************/
static void c_nstate(GtkWidget *bframe,guint del)
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *entry;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *button;
  gchar *liste[5];
  int nliste = 5;
  gchar *titre = _("Number of states");

 
  liste[0]=g_strdup("1");
  liste[1]=g_strdup("2");
  liste[2]=g_strdup("3");
  liste[3]=g_strdup("4");
  liste[4]=g_strdup("5");
  /* Fenetre principale */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),titre);
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Wins));
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  add_child(Wins,fp,gtk_widget_destroy,titre);
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)delete_child,NULL);

  vboxall = create_vbox(fp);
  frame = gtk_frame_new (titre);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  hbox1 = create_hbox(vboxframe);
  entry= create_combo_box_entry_liste(Window,hbox1,_(" Number of states :"),liste,nliste);
  
  hbox2 = create_hbox(vboxall);
  gtk_widget_realize(Window);

  button = create_button(Window,_("Cancel"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(Window,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(gene_nstate),(gpointer)entry);
  if(del) g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(bframe));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));

   
  gtk_widget_show_all(fp);
}
/********************************************************************************************************/
static void c_initial_guess (GtkWidget *bframe)
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *entry;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *button;
  gchar *liste[2];
  gchar *titre =_("Initial orbital guess");

  int nliste = 2;
  liste[0]=g_strdup("H0");
  liste[1]=g_strdup("ATDEN");
  
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),titre);
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Wins));
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  add_child(Wins,fp,gtk_widget_destroy,titre);
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)delete_child,NULL);

  vboxall = create_vbox(fp);
  frame = gtk_frame_new (titre);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  hbox1 = create_hbox(vboxframe);
  entry= create_combo_box_entry_liste(Window,hbox1,_(" The type of the initial orbital guess :"),liste,nliste);
  
  hbox2 = create_hbox(vboxall);
  gtk_widget_realize(Window);

  button = create_button(Window,_("Cancel"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(Window,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(gene_guess),(gpointer)entry);
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(bframe));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));

   
  gtk_widget_show_all(fp);
}
/********************************************************************************************************/
static void gene_S_orb_mode(GtkWidget *b,gpointer data)
{
  if (!strcmp((char *)data,"DENSITY") )
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "Start,DENSITY=",-1);
  else
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, (char*)data,-1);
  if (strcmp((char *)data,"DENSITY") )
  	gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, ",",-1);
}
/********************************************************************************************************/
static void gene_S_orb_num(GtkWidget *b,gpointer data)
{
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext;
  entryall=(GtkWidget **)data;
  entry=entryall[1];
  entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, entrytext,-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, ".",-1);

  entry=entryall[0];
  entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, entrytext,-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "; ",-1);
}
/********************************************************************************************************/
static void gene_orb_mode(GtkWidget *b,gpointer data)
{
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, (char*)data,-1);
}
/********************************************************************************************************/
static void gene_orb_num(GtkWidget *b,gpointer data)
{
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext;
  guint i;
  entryall=(GtkWidget **)data;
  for (i=0;i<8;i++)
  {
  entry=entryall[i];
  entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, ",",-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, entrytext,-1);
  }
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "; ",-1);
}
/********************************************************************************************************/
static void gene_one_entry2(GtkWidget *b,gpointer data)
{
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext;
  entry=(GtkWidget *)data;
  entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, entrytext,-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "; ",-1);
}
/********************************************************************************************************/
static void gene_one_entry1(GtkWidget *b,gpointer data)
{
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, (char*)data,-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, ",",-1);
}
/********************************************************************************************************/
static void c_one_entry (GtkWidget *bframe,gchar *titre,gchar *mode,gchar *set,guint del)
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
  
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),titre);
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Wins));
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  add_child(Wins,fp,gtk_widget_destroy,titre);
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)delete_child,NULL);

  vboxall = create_vbox(fp);
  frame = gtk_frame_new (titre);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
   hbox1 = create_hbox(vboxframe);
  label = gtk_label_new (titre);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox1), label, TRUE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (hbox1), entry, FALSE, TRUE, 0);
  gtk_entry_set_text(GTK_ENTRY(entry),set);

  hbox2 = create_hbox(vboxall);
  gtk_widget_realize(Window);

  button = create_button(Window,_("Cancel"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(Window,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(gene_one_entry1),(gpointer)mode);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(gene_one_entry2),(gpointer)entry);
  if(del) g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(bframe));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));

   
  gtk_widget_show_all(fp);
}
/********************************************************************************************************/
static void c_SS_orb (GtkWidget *bframe,gchar *titre,gchar *mode)
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget **entry;
  gchar      *labelt[2];
  guint i;
  entry=g_malloc(2*sizeof(GtkWidget *));
  for (i=0;i<2;i++)
        labelt[i]=g_malloc(100);
  sprintf(labelt[0],_("File number : "));
  sprintf(labelt[1],_("Record number: "));
  /* Fenetre principale */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),titre);
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Wins));
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  add_child(Wins,fp,gtk_widget_destroy,titre);
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)delete_child,NULL);


  vboxall = create_vbox(fp);
  frame = gtk_frame_new (titre);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  for (i=0;i<2;i++)
  {
    hbox1 = create_hbox(vboxframe);
  label = gtk_label_new (labelt[i]);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox1), label, TRUE, FALSE, 0);

  entry[i] = gtk_entry_new ();
  gtk_widget_show (entry[i]);
  gtk_box_pack_start (GTK_BOX (hbox1), entry[i], FALSE, TRUE, 0);
  }
  
  hbox2 = create_hbox(vboxall);
  gtk_widget_realize(fp);

  button = create_button(fp,_("Cancel"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(fp,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(gene_S_orb_mode),(gpointer)mode);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(gene_S_orb_num),(gpointer)entry);
  g_signal_connect_swapped(G_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(bframe));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));


  for (i=1;i<2;i++)
        g_free(labelt[i]);
   
  gtk_widget_show_all(fp);
}
/********************************************************************************************************/
static void c_orb (GtkWidget *bframe,gchar *titre,gchar *mode)
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *button;
  GtkWidget *label;
  GtkWidget **entry;
  gchar      *Sym[8];
  guint i;
  entry=g_malloc(8*sizeof(GtkWidget *));
  for (i=0;i<8;i++)
  {
        Sym[i]=g_malloc(100);
  	sprintf(Sym[i],_("In symmetry number %d : "),i+1);
  }
  /* Fenetre principale */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),titre);
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Wins));
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  add_child(Wins,fp,gtk_widget_destroy,titre);
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)delete_child,NULL);

  vboxall = create_vbox(fp);
  frame = gtk_frame_new (titre);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  for (i=0;i<8;i++)
  {
    hbox1 = create_hbox(vboxframe);
  label = gtk_label_new (Sym[i]);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox1), label, TRUE, FALSE, 0);

  entry[i] = gtk_entry_new ();
  gtk_widget_show (entry[i]);
  gtk_box_pack_start (GTK_BOX (hbox1), entry[i], FALSE, TRUE, 0);
  }
  
  hbox2 = create_hbox(vboxall);
  gtk_widget_realize(fp);

  button = create_button(fp,_("Cancel"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(fp,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(gene_orb_mode),(gpointer)mode);
  g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(gene_orb_num),(gpointer)entry);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(bframe));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));


  for (i=1;i<8;i++) g_free(Sym[i]);
   
  gtk_widget_show_all(fp);
}
/********************************************************************************************************/
static void gene_wf(GtkWidget *b,gpointer data)
{
  GtkWidget **entryall;
  GtkWidget *entry;
  G_CONST_RETURN gchar *entrytext;
  guint i;
  entryall=(GtkWidget **)data;
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "wf",-1);
  for (i=0;i<3;i++)
  {
  entry=entryall[i];
  entrytext = gtk_entry_get_text(GTK_ENTRY(entry));
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, ",",-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, entrytext,-1);
  }
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "; ",-1);
}
/********************************************************************************************************/
static void cw_wave (GtkWidget *bframe,guint del)
{
  GtkWidget *fp;
  GtkWidget *frame;
  GtkWidget **entry;
  GtkWidget *vboxall;
  GtkWidget *vboxframe;
  GtkWidget *hbox1;
  GtkWidget *hbox2;
  GtkWidget *button;
  gchar *listene[1];
  int nlistene = 1;
  gchar *listespin[7];
  int nlistespin = 7;
  gchar *listeirre[8];
  int nlisteiree = 8;
  gchar *titre=_("Wave function symmetry");

  entry=g_malloc(3*sizeof(GtkWidget*));
  listene[0]=g_strdup_printf("%d",Nelectrons);


  if(Nelectrons%2 == 0 && SpinMultiplicities[0]%2==0)
  	listespin[0] = g_strdup_printf("%d",SpinMultiplicities[0]);
  else if(Nelectrons%2 == 1 && SpinMultiplicities[0]%2==1)
  	listespin[0] = g_strdup_printf("%d",SpinMultiplicities[0]);
  else
  {
  	if(Nelectrons%2 == 0) listespin[0]=g_strdup("0");
  	else listespin[0]=g_strdup("1");
  }

  listespin[1]=g_strdup("0");
  listespin[2]=g_strdup("1");
  listespin[3]=g_strdup("2");
  listespin[4]=g_strdup("3");
  listespin[5]=g_strdup("4");
  listespin[6]=g_strdup("5");

  listeirre[0]=g_strdup("1");
  listeirre[1]=g_strdup("2");
  listeirre[2]=g_strdup("3");
  listeirre[3]=g_strdup("4");
  listeirre[4]=g_strdup("5");
  listeirre[5]=g_strdup("6");
  listeirre[6]=g_strdup("7");
  listeirre[7]=g_strdup("8");
  /* Fenetre principale */
  fp = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(fp),TRUE);
  gtk_window_set_title(GTK_WINDOW(fp),titre);
  gtk_container_set_border_width (GTK_CONTAINER (fp), 5);

  gtk_window_set_transient_for(GTK_WINDOW(fp),GTK_WINDOW(Wins));
  gtk_window_set_position(GTK_WINDOW(fp),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (fp), TRUE);

  add_child(Wins,fp,gtk_widget_destroy,titre);
  g_signal_connect(G_OBJECT(fp),"delete_event",(GCallback)delete_child,NULL);

  vboxall = create_vbox(fp);
  frame = gtk_frame_new (titre);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
  gtk_container_add (GTK_CONTAINER (vboxall), frame);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);
  hbox1 = create_hbox(vboxframe);
  entry[0]= create_combo_box_entry_liste(Window,hbox1,_("Number of electrons :"),listene,nlistene);
  hbox1 = create_hbox(vboxframe);
  entry[1]= create_combo_box_entry_liste(Window,hbox1,_("Number of the irreducible representation :"),listeirre,nlisteiree);
  hbox1 = create_hbox(vboxframe);
  entry[2]= create_combo_box_entry_liste(Window,hbox1,_("2*Spin :"),listespin,nlistespin);
  
  hbox2 = create_hbox(vboxall);
  gtk_widget_realize(fp);

  button = create_button(fp,_("Cancel"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));
  gtk_widget_show (button);

  button = create_button(fp,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
  gtk_widget_show (button);
  g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(gene_wf),(gpointer)entry);
  if(del) g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(bframe));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(delete_child),GTK_OBJECT(fp));

   
  gtk_widget_show_all(fp);
}
/********************************************************************************************************/
static void Traite_ci_Option(GtkWidget *bframe,gpointer data)
{
 char *temp;
 temp=g_malloc(100);
  if (!strcmp((char *)data,_("Defining the state symmetry")) )
   cw_wave(bframe,1);
  else
  if (!strcmp((char *)data,_("Defining the occupied orbitals")) )
   c_orb(bframe,_("Defining the occupied orbitals"),"Occ");
  else
  if (!strcmp((char *)data,_("Defining the closed-shell orbitals")) )
   c_orb(bframe,_("Defining the closed-shell orbitals"),"Closed");
  else
  if (!strcmp((char *)data,_("Defining the Frozen-core orbitals")) )
   c_orb(bframe,_("Defining the Frozen-core orbitals"),"Core");
  else
  if (!strcmp((char *)data,_("Saving the natural orbitals")) )
   c_SS_orb(bframe,_("Saving the natural orbital"),"Natorb");
  else
  if (!strcmp((char *)data,_("Defining the number of states in the present symmetry")) )
   c_nstate(bframe,1);
  else
  if (!strcmp((char *)data,_("Defining the orbitals")) )
   c_SS_orb(bframe,_("Defining the orbitals"),"Orbit");
  else
  if (!strcmp((char *)data,_("Additional reference symmetries")) )
     c_ref(bframe,0);
  else
  if (!strcmp((char *)data,_("Restriction of classes of excitations")) )
  c_restrictexcitation(bframe,1);
  else 
  {
  sprintf(temp,_("Sorry, the button of \"%s\" is not active"),(char*)data);
   Message(temp,_("Warning"),TRUE);
   gtk_widget_hide(bframe);
  }
 g_free(temp);
}
/********************************************************************************************************/
static void Traite_multi_Option(GtkWidget *bframe,gpointer data)
{
 char *temp;
 temp=g_malloc(100);
  if (!strcmp((char *)data,_("Defining the state symmetry")) )
   cw_wave(bframe,0);
  else
  if (!strcmp((char *)data,_("Defining the occupied orbitals")) )
   c_orb(bframe,_("Defining the occupied orbitals"),"Occ");
  else
  if (!strcmp((char *)data,_("Defining the closed-shell orbitals")) )
   c_orb(bframe,_("Defining the closed-shell orbitals"),"Closed");
  else
  if (!strcmp((char *)data,_("Defining the Frozen-core orbitals")) )
   c_orb(bframe,_("Defining the Frozen-core orbitals"),"Core");
  else
  if (!strcmp((char *)data,_("Saving the final orbitals")) )
   c_SS_orb(bframe,_("Saving the final orbital"),"Orbital");
  else
  if (!strcmp((char *)data,_("Defining the starting guess")) )
   c_SS_orb(bframe,_("Defining the starting guess"),"Start");
  else
  if (!strcmp((char *)data,_("Defining the number of states in the present symmetry")) )
   c_nstate(bframe,0);
  else
  if (!strcmp((char *)data,_("Specifying weights in state-averaged calculations")) )
   c_one_entry(bframe,_("Specifying weights in state-averaged calculations "),"Weights","1,1,1,1",0);
  else
  if (!strcmp((char *)data,_("Type of orbitals to save")) )
     c_type_orb(bframe,1);
  else 
  {
  sprintf(temp,_("Sorry, the button of \"%s\" is not active"),(char*)data);
   Message(temp,_("Warning"),TRUE);
   gtk_widget_hide(bframe);
  }
 g_free(temp);
}
static void Traite_hf_Option(GtkWidget *bframe,gpointer data)
{
 char *temp;
 temp=g_malloc(100);
  if (!strcmp((char *)data,_("Defining the wavefunction")) )
   cw_wave(bframe,1);
  else
  if (!strcmp((char *)data,_("Specifying closed-shell orbitals")) )
   c_orb(bframe,_("Specifying closed-shell orbitals"),"Closed");
  else
  if (!strcmp((char *)data,_("Defining the number of occupied orbitals in each symmetry")) )
   c_orb(bframe,_("Defining the number of occupied orbitals in each symmetry"),"Occ");
  else
  if (!strcmp((char *)data,_("Saving the final orbital")) )
   c_SS_orb(bframe,_("Saving the final orbital"),"Save");
  else
  if (!strcmp((char *)data,_("Starting with previous orbitals")) )
   c_SS_orb(bframe,_("Starting with previous orbitals"),"Start");
  else
  if (!strcmp((char *)data,_("Starting with a previous density matrix")) )
   c_SS_orb(bframe,_("Starting with a previous density matrix"),"DENSITY");
  else
  if (!strcmp((char *)data,_("Initial orbital guess")) )
   c_initial_guess(bframe);
  else 
  {
  sprintf(temp,_("Sorry, the button of \"%s\" is not active"),(char*)data);
   Message(temp,_("Warning"),TRUE);
   gtk_widget_hide(bframe);
  }
 g_free(temp);
}
/********************************************************************************************************/
static void button_fcioption(GtkWidget *w)
{
	GtkWidget* Table;
	GtkWidget* button;
	guint i;
	guint j;
        guint ColonneT=2; 
        guint LigneT=3; 
	char *LabelButton[2][3]={
        {_("Defining the orbitals"),
         _("Defining the closed-shell orbitals"),
         _("Defining the state symmetry"),
        },
        {_("Defining the occupied orbitals"),
         _("Defining the Frozen-core orbitals"),
         "00"
         }
	};

  Table = gtk_table_new(LigneT,ColonneT,TRUE);
  gtk_container_add(GTK_CONTAINER(w),Table);
  
  for ( i = 0;i<LigneT;i++)
	  for ( j = 0;j<ColonneT;j++)
  {
    if(strcmp(LabelButton[j][i],"00"))
          {
	  button = gtk_button_new_with_label(LabelButton[j][i]);

          g_signal_connect(G_OBJECT(button), "clicked",
                            (GCallback)Traite_ci_Option,(gpointer )LabelButton[j][i]);

	  gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
          gtk_widget_show (button);
         }

  }
  gtk_widget_show (Table);
 	
}
/********************************************************************************************************/
static void button_cioption(GtkWidget *w)
{
	GtkWidget* Table;
	GtkWidget* button;
	guint i;
	guint j;
        guint ColonneT=2; 
        guint LigneT=6; 
	char *LabelButton[2][6]={
        {_("Defining the occupied orbitals"),
         _("Defining the closed-shell orbitals"),
         _("Defining the state symmetry"),
         _("Additional reference symmetries"),
         "00",
         "00"
        },
        {_("Defining the Frozen-core orbitals"),
         _("Defining the orbitals"),
         _("Defining the number of states in the present symmetry"),
         _("Restriction of classes of excitations"),
         _("Saving the natural orbitals"),
         "00"
         }
	};

  Table = gtk_table_new(LigneT,ColonneT,TRUE);
  gtk_container_add(GTK_CONTAINER(w),Table);
  
  for ( i = 0;i<LigneT;i++)
	  for ( j = 0;j<ColonneT;j++)
  {
    if(strcmp(LabelButton[j][i],"00"))
          {
	  button = gtk_button_new_with_label(LabelButton[j][i]);

          g_signal_connect(G_OBJECT(button), "clicked",
                            (GCallback)Traite_ci_Option,(gpointer )LabelButton[j][i]);

	  gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
          gtk_widget_show (button);
         }

  }
  gtk_widget_show (Table);
 	
}
/********************************************************************************************************/
static void button_multioption(GtkWidget *w)
{
	GtkWidget* Table;
	GtkWidget* button;
	guint i;
	guint j;
        guint ColonneT=2; 
        guint LigneT=6; 
	char *LabelButton[2][6]={
        {_("Defining the occupied orbitals"),
         _("Defining the closed-shell orbitals"),
         _("Defining the state symmetry"),
         "00",
         _("Type of orbitals to save"),
         "00"
        },
        {_("Defining the Frozen-core orbitals"),
         _("Defining the starting guess"),
         _("Defining the number of states in the present symmetry"),
         _("Specifying weights in state-averaged calculations"),
         _("Saving the final orbitals"),
         "00"
         }
	};

  Table = gtk_table_new(LigneT,ColonneT,TRUE);
  gtk_container_add(GTK_CONTAINER(w),Table);
  
  for ( i = 0;i<LigneT;i++)
	  for ( j = 0;j<ColonneT;j++)
  {
    if(strcmp(LabelButton[j][i],"00"))
          {
	  button = gtk_button_new_with_label(LabelButton[j][i]);

          g_signal_connect(G_OBJECT(button), "clicked",
                            (GCallback)Traite_multi_Option,(gpointer )LabelButton[j][i]);

	  gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
          gtk_widget_show (button);
         }

  }
  gtk_widget_show (Table);
 	
}
/********************************************************************************************************/
static void button_hfoption(GtkWidget *w)
{
	GtkWidget* Table;
	GtkWidget* button;
	guint i;
	guint j;
        guint ColonneT=2; 
        guint LigneT=4; 
	char *LabelButton[2][4]={
                {_("Defining the wavefunction"),
                 _("Specifying closed-shell orbitals"),
                 _("Starting with previous orbitals"),
                 _("Starting with a previous density matrix")},
                {_("Defining the number of occupied orbitals in each symmetry"),
                 _("Saving the final orbital"),
                 _("Initial orbital guess"),
                 "00" }
		};

  Table = gtk_table_new(LigneT,ColonneT,TRUE);
  gtk_container_add(GTK_CONTAINER(w),Table);
  
  for ( i = 0;i<LigneT;i++)
	  for ( j = 0;j<ColonneT;j++)
  {
    if(strcmp(LabelButton[j][i],"00"))
          {
	  button = gtk_button_new_with_label(LabelButton[j][i]);

          g_signal_connect(G_OBJECT(button), "clicked",
                            (GCallback)Traite_hf_Option,(gpointer )LabelButton[j][i]);

	  gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
          gtk_widget_show (button);
         }

  }
  gtk_widget_show (Table);
 	
}
/********************************************************************************************************/
void create_frame_option (gchar *titre)
{
  GtkWidget *frame;

  frame = gtk_frame_new (titre);
  gtk_widget_show (frame);
  gtk_box_pack_start (GTK_BOX (VboxOption), frame, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
 FrameShow=frame;
}
/********************************************************************************************************/
static void command_select(GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	if(FrameShow !=NULL) gtk_widget_destroy(FrameShow);
	if (!strcmp((char *)data,"hf") ) { create_frame_option(_("HF Options")); button_hfoption(FrameShow); }
	else if (!strcmp((char *)data,"uhf") ) { create_frame_option(_("UHF Options")); button_hfoption(FrameShow); }
	else if (!strcmp((char *)data,"multi") ) { create_frame_option(_("Multi Options")); button_multioption(FrameShow); }
	else if (!strcmp((char *)data,"ci") ) { create_frame_option(_("CI Options")); button_cioption(FrameShow); }
	else if (!strcmp((char *)data,"fci") ) { create_frame_option(_("FCI Options")); button_fcioption(FrameShow); }
 
	if (strcmp((char *)data,_("Please select the type of calculation")) )
	{
		if(!begin) gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "}\n",-1);
		gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "{ ",-1);
		gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, (char *)data,-1);
		gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "; ",-1);
		begin = 0;
	}
}
/********************************************************************************************************/
static GtkWidget *CreateListeComm(GtkWidget* BoiteVP)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;

	GtkWidget *hbox1;
	GtkWidget *window1;

	window1 = Window;
	hbox1 = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox1);
	gtk_box_pack_start (GTK_BOX (BoiteVP), hbox1, FALSE, TRUE, 10);

	store = gtk_tree_store_new (1,G_TYPE_STRING);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Please select the type of calculation"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "hf", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "multi", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "ci", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "fci", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "cisd", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "qci", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "ccsd", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "ccsd(t)", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "rccsd", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "rccsd(t)", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "rccsd-t", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "uccsd", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "uccsd(t)", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "uccsd-t", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "ks", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "uks", -1);

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	gtk_box_pack_start (GTK_BOX (hbox1), combobox, TRUE, TRUE, 1);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(command_select), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	return combobox;
}
/********************************************************************************************************/
void AddToComm(GtkWidget *w)
{
  G_CONST_RETURN gchar *ModeEntry;
  gchar *Option;

  ModeEntry = gtk_entry_get_text(GTK_ENTRY(combo_entry1));
  Option=  gabedit_text_get_chars(text1, 0, -1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, ModeEntry,-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, ";",-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, Option,-1);
  gabedit_text_insert (GABEDIT_TEXT(text2), NULL, NULL, NULL, "\n",-1);

}
/********************************************************************************************************/
void AjoutePageComm(GtkWidget* Win,GtkWidget *NoteBook,CommS *comm)
{
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *window1;
  GtkWidget *vbox1;
  GtkWidget *frame2;
  GtkWidget *scrolledwindow2;
  GtkWidget *combobox;
  Wins = Win;

  begin = 1;

  Frame = gtk_frame_new(NULL);
  gtk_widget_set_size_request(GTK_WIDGET(Frame), 600, 400);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);

  LabelOnglet = gtk_label_new(_("Commands"));
  LabelMenu = gtk_label_new(_("Commands"));
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),Frame,LabelOnglet, LabelMenu);

  window1 = Frame;
  Window = window1;
  g_object_set_data (G_OBJECT (window1), "window1", window1);

  vbox1 =create_vbox(window1);

  VboxOption=vbox1 ;
  combobox = CreateListeComm(vbox1);
  FrameShow=NULL;

  frame2 = gtk_frame_new (_("View all commands generated by gabedit"));
  gtk_widget_show (frame2);
  gtk_box_pack_start (GTK_BOX (vbox1), frame2, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame2), 5);

  scrolledwindow2 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow2);
  gtk_container_add (GTK_CONTAINER (frame2), scrolledwindow2);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow2), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

  text2 = gabedit_text_new ();
  gtk_widget_show (text2);
  gtk_container_add (GTK_CONTAINER (scrolledwindow2), text2);
  gabedit_text_set_editable (GABEDIT_TEXT (text2), FALSE);
  comm->TextComm=text2;
  gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
}

