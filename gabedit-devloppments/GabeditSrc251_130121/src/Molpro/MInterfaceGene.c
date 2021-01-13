/* MInterfaceGene.c */
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
#include <gtk/gtk.h>

#include "../Common/Global.h"
#include "MInterfaceGene.h"
#include "../Utils/UtilsInterface.h"

static  GtkWidget *entryC;
static  GtkWidget *entryX;
static  GtkWidget *entryY;
static  GtkWidget *entryZ;

  GtkWidget *checkbutton[32];

  GtkWidget *ButtonRad1;
  GtkWidget *ButtonRad2;

  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *window1;
  GtkWidget *label1;
  GtkWidget *entry20;
  GtkWidget *entry21;
  GtkWidget *frame1;
  GtkWidget *vboxo;
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *entry1;
  GtkWidget *entry2;
  GtkWidget *entry3;
  GtkWidget *entry4;
  GtkWidget *label5;
  GtkWidget *entry5;
  GtkWidget *entry6;
  GtkWidget *entry7;
  GtkWidget *vbox2;
  GtkWidget *entry8;
  GtkWidget *hbox5;
  GtkWidget *entry9;
  GtkWidget *entry10;
  GtkWidget *entry11;
  GtkWidget *entry12;
  GtkWidget *entry13;
  GtkWidget *entry14;

/************************************************************************************************************/
static void  Set_Entry1(GtkWidget *b,gpointer d)
{
  if(entryC != NULL ) gtk_widget_set_sensitive(GTK_WIDGET(entryC),TRUE); 
  if(entryX != NULL ) gtk_widget_set_sensitive(GTK_WIDGET(entryX),FALSE); 
  if(entryY != NULL ) gtk_widget_set_sensitive(GTK_WIDGET(entryY),FALSE); 
  if(entryZ != NULL ) gtk_widget_set_sensitive(GTK_WIDGET(entryZ),FALSE); 
}
/************************************************************************************************************/
static void  Set_Entry2(GtkWidget *b,gpointer d)
{
  if(entryC != NULL ) gtk_widget_set_sensitive(GTK_WIDGET(entryC),FALSE); 
  if(entryX != NULL ) gtk_widget_set_sensitive(GTK_WIDGET(entryX),TRUE); 
  if(entryY != NULL ) gtk_widget_set_sensitive(GTK_WIDGET(entryY),TRUE); 
  if(entryZ != NULL ) gtk_widget_set_sensitive(GTK_WIDGET(entryZ),TRUE); 
}
/************************************************************************************************************/
/*
static void add_separator_to_hbox(GtkWidget *win,GtkWidget *hbox)
{
  GtkWidget *vseparator;
  vseparator = gtk_vseparator_new ();
  gtk_widget_show (vseparator);
  gtk_box_pack_start (GTK_BOX (hbox), vseparator, FALSE, FALSE, 10);
}
*/
/************************************************************************************************************/
static void add_separator_to_vbox(GtkWidget *win,GtkWidget *vbox)
{
  GtkWidget *hseparator;
  hseparator = gtk_hseparator_new ();
  gtk_widget_show (hseparator);
  gtk_box_pack_start (GTK_BOX (vbox), hseparator, FALSE, FALSE, 0);
}
/************************************************************************************************************/
static GtkWidget *create_label_and_entry_in_hbox_false(GtkWidget *win,GtkWidget *hbox,gchar *tlabel,gchar *tset,gint len)
{
  GtkWidget *entry;
  GtkWidget *label;

  label = gtk_label_new (tlabel);
  gtk_widget_set_size_request(GTK_WIDGET(label),len,-1);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, TRUE, 0);

  entry = gtk_entry_new ();
  gtk_entry_set_text(GTK_ENTRY(entry),tset);
  gtk_widget_set_size_request(GTK_WIDGET(entry),60,-1);
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, TRUE, 0);

  return entry;
}
/************************************************************************************************************/
static GtkWidget *create_label_and_entry_in_hbox(GtkWidget *win,GtkWidget *hbox,gchar *tlabel,gchar *tset)
{
  GtkWidget *entry;
  GtkWidget *label;

  label = gtk_label_new (tlabel);
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, FALSE, 0);

  entry = gtk_entry_new ();
  gtk_entry_set_text(GTK_ENTRY(entry),tset);
  gtk_widget_show (entry);
  gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, TRUE, 0);

  return entry;
}
/************************************************************************************************************/
static GtkWidget *create_hbox_in_Vbox(GtkWidget *win,GtkWidget *vbox)
{
  GtkWidget *hbox;
  hbox = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);

  return hbox;
}
/************************************************************************************************************/
static GtkWidget *create_hboxlabelentry(GtkWidget *win,GtkWidget *vbox,gchar *tlabel,gchar *tset)
{
  GtkWidget *hbox;
  GtkWidget *entry;
  hbox = create_hbox_in_Vbox(win,vbox);
  entry = create_label_and_entry_in_hbox(win,hbox,tlabel,tset);
  return entry;
}
/************************************************************************************************************/
static void  create_epilogue(GtkWidget *Frame)
{
  GtkWidget *label1;
  GtkWidget *frame1;
  GtkWidget *vboxo;
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *label5;
  GtkWidget *window1;
  GtkWidget *hbox5;
  window1 = Frame;
  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (window1), vbox1);

  hbox1 = gtk_hbox_new (FALSE, 10);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

  label1 = gtk_label_new ("Title :");
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (hbox1), label1, FALSE, FALSE, 0);

  entry20 = gtk_entry_new ();
  gtk_widget_show (entry20);
  gtk_box_pack_start (GTK_BOX (hbox1), entry20, TRUE, TRUE, 0);
  gtk_entry_set_text(GTK_ENTRY(entry20),_("Input file generated by gabedit"));

  add_separator_to_vbox( window1,vbox1);

  hbox5 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox5);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox5, FALSE, FALSE, 0);
  label5 = gtk_label_new (_("Memory size(in words) : "));
  gtk_widget_show (label5);
  gtk_box_pack_start (GTK_BOX (hbox5), label5, FALSE, FALSE, 0);

  entry21 = gtk_entry_new ();
  gtk_entry_set_text(GTK_ENTRY(entry21),"4000000");
  gtk_widget_show (entry21);
  gtk_box_pack_start (GTK_BOX (hbox5), entry21, FALSE, TRUE, 0);

  add_separator_to_vbox( window1,vbox1);


  window1 = vbox1;
  frame1 = gtk_frame_new (_("Global Thresholds "));
  gtk_widget_show (frame1);
  gtk_container_add (GTK_CONTAINER (window1), frame1);
  gtk_frame_set_label_align (GTK_FRAME (frame1), 0.5, 0.5);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (frame1), hbox1);

  vboxo = vbox1;
  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 0);

  entry1=create_hboxlabelentry(window1,vbox1,"Zero    : ","1.d-12");
  entry2=create_hboxlabelentry(window1,vbox1,"Oneint  : ","1.d-12");
  entry3=create_hboxlabelentry(window1,vbox1,"Twoint  : ","1.d-12");
  entry4=create_hboxlabelentry(window1,vbox1,"Prefac  : ","1.d-14");
  entry5=create_hboxlabelentry(window1,vbox1,"Locali  : ","1.d-8");

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 0);
  entry6=create_hboxlabelentry(window1,vbox1,"  Eorder  : ","1.d-4");
  entry7=create_hboxlabelentry(window1,vbox1,"  Energy  : ","1.d-6");
  entry8=create_hboxlabelentry(window1,vbox1,"  Gradiant: ","1.d-2");
  entry9=create_hboxlabelentry(window1,vbox1,"  Step    : ","1.d-3");
  entry10=create_hboxlabelentry(window1,vbox1,"  Orbital : ","1.d-5");

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 0);

  entry11=create_hboxlabelentry(window1,vbox1,"  Civec   : ","1.d-5");
  entry12=create_hboxlabelentry(window1,vbox1,"  Coeff   : ","1.d-4");
  entry13=create_hboxlabelentry(window1,vbox1,"  Printci : ","0.05");
  entry14=create_hboxlabelentry(window1,vbox1,"  Punchci : ","99");

}
/************************************************************************************************************/
static void  create_gprint(GtkWidget *Frame)
{
  guint i;
  GtkWidget *frame1;
  GtkWidget *vbox1;
  GtkWidget *vbox2;
  GtkWidget *hbox1;
  GtkWidget *window1;
  gchar *tGP1[7]={
  		_("Print basis information "),
  		_("Print CI vector in MCSCF"),
  		_("Print reference CSFs and their coefficients in CI "),
  		_("Print detailed I/O information "),
  		_("Print orbitals in SCF and MCSCF "),
  		_("Print bond angle information"),
  		_("Print information for singles in CI, CCSD ")
  		};
  gchar *tGP2[7]={
		_("Print bond distances "),
		_("Print p-space configurations"),
		_("Print variables each time they are set or changed "),
		_("Print information for pairs in CI, CCSD "),
		_("Print microiterations in MCSCF and CI"),
		_("Print detailed CPU information "),
                _("Print pair list in CI, CCSD")
  		};

  window1 = Frame;
  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (window1), vbox1);


  window1 = vbox1;
  frame1 = gtk_frame_new (_("Global Print Options"));
  gtk_widget_show (frame1);
  gtk_container_add (GTK_CONTAINER (window1), frame1);
  gtk_frame_set_label_align (GTK_FRAME (frame1), 0.5, 0.5);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_container_add (GTK_CONTAINER (frame1), hbox1);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 0);

  for(i=0;i<7;i++)
  	checkbutton[i] = create_checkbutton(window1,vbox1,tGP1[i]);

  /*
  GTK_TOGGLE_BUTTON (checkbutton[0])->active=TRUE;
  GTK_TOGGLE_BUTTON (checkbutton[4])->active=TRUE;
  */
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton[0]), TRUE);
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbutton[4]), TRUE);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox2, TRUE, TRUE, 0);

  for(i=0;i<7;i++)
  	checkbutton[i+7] = create_checkbutton(window1,vbox2,tGP2[i]);
}
/************************************************************************************************************/
static void  create_oneop(GtkWidget *Frame)
{
  guint i;
  GtkWidget *frame1;
  GtkWidget *vbox1;
  GtkWidget *vbox2;
  GtkWidget *hbox1;
  GtkWidget *window1;
  gchar *tO1[6]={
		_("Overlap "),
		_("Kinetic energy"),
		_("Potential energy "),
		_("Delta function "),
		_("Delta4 function "),
		_("One-electron Darwin term")
		};
  gchar *tO2[6]={
		_("Mass-velocity term "),
		_("Total Cowan-Griffin Relativistic correction"),
		_("Products of angular momentum operators "),
		_("Dipole moments"),
		_("Second moment "),
		_("Quadrupole moments and R2 ")
		};
  gchar *tO3[6]={
		_("Electric field"),
		_("Field gradients "),
		_("Angular momentum operators"),
		_("Velocity"),
		_("Spin-orbit operators"),
		_("ECP spin-orbit operators")
		};

  window1 = Frame;
  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (window1), vbox1);

  window1 = vbox1;
  frame1 = gtk_frame_new (_("One-electron operators and expectation values"));
  gtk_widget_show (frame1);
  gtk_container_add (GTK_CONTAINER (window1), frame1);
  gtk_frame_set_label_align (GTK_FRAME (frame1), 0.5, 0.5);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_container_add (GTK_CONTAINER (frame1), vbox1);

  add_separator_to_vbox( window1,vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

   ButtonRad1 = gtk_radio_button_new_with_label( NULL,_("Compute with origin at "));
   gtk_box_pack_start (GTK_BOX (hbox1), ButtonRad1, FALSE, FALSE, 0);
   gtk_widget_show (ButtonRad1);

    entryC = create_label_and_entry_in_hbox_false(window1,hbox1,_("Center Number : "),"1",(gint)(ScreenHeight*0.1));

   g_signal_connect(G_OBJECT (ButtonRad1), "clicked", G_CALLBACK(Set_Entry1), NULL);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);

   ButtonRad2 = gtk_radio_button_new_with_label(
                       gtk_radio_button_get_group (GTK_RADIO_BUTTON (ButtonRad1)),
                       _("Compute with origin at ")); 
   gtk_box_pack_start (GTK_BOX (hbox1), ButtonRad2, FALSE, FALSE, 0);
   gtk_widget_show (ButtonRad2);
   gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (ButtonRad2), TRUE);
    entryX = create_label_and_entry_in_hbox_false(window1,hbox1," X : ","0.0",(gint)(ScreenHeight*0.1));
    entryY = create_label_and_entry_in_hbox_false(window1,hbox1," Y : ","0.0",(gint)(ScreenHeight*0.05));
    entryZ = create_label_and_entry_in_hbox_false(window1,hbox1," Z : ","0.0",(gint)(ScreenHeight*0.05));
   g_signal_connect(G_OBJECT (ButtonRad2), "clicked", G_CALLBACK(Set_Entry2), NULL);
   Set_Entry2(NULL,NULL);

  add_separator_to_vbox( window1,vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, TRUE, TRUE, 0);

  vbox1 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox1);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox1, TRUE, TRUE, 0);

  for(i=0;i<6;i++)
  	checkbutton[i+14] = create_checkbutton(window1,vbox1,tO1[i]);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox2, TRUE, TRUE, 0);

  for(i=0;i<6;i++)
  	checkbutton[i+20] = create_checkbutton(window1,vbox2,tO2[i]);

  vbox2 = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox2);
  gtk_box_pack_start (GTK_BOX (hbox1), vbox2, TRUE, TRUE, 0);

  for(i=0;i<6;i++)
  	checkbutton[i+26] = create_checkbutton(window1,vbox2,tO3[i]);
}
/************************************************************************************************************/
static void Add_page_gene(GtkWidget *NoteBook,gchar * tNote,guint j)
{
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *window1;
  
  Frame = gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);

  LabelOnglet = gtk_label_new(tNote);
  LabelMenu = gtk_label_new(tNote);
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
                                Frame,
                                LabelOnglet, LabelMenu);

  window1 = Frame;
  g_object_set_data (G_OBJECT (window1), "window1", window1);
  if(j==0)create_epilogue(Frame);
  if(j==1)create_gprint(Frame);
  if(j==2)create_oneop(Frame);


}
/************************************************************************************************************/
static void create_notebook_gene(GtkWidget *frame)
{
  GtkWidget *vbox; 
  GtkWidget *NoteBook; 

  vbox =create_vbox(frame);
  NoteBook = gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX (vbox), NoteBook,FALSE, FALSE, 0);
  Add_page_gene(NoteBook,_("  Title/Global Thresholds "),0); 
  Add_page_gene(NoteBook,_("  Global Print Options "),1); 
  Add_page_gene(NoteBook,_("  One-electron operators and expectation values "),2); 
}
/************************************************************************************************************/
void AjoutePageGene(GtkWidget *NoteBook,GeneS *gen)
{

  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *window1;


  entryC=NULL;
  entryX=NULL;
  entryY=NULL;
  entryZ=NULL;
  Frame = gtk_frame_new(NULL);
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 20);

  LabelOnglet = gtk_label_new(_("General"));
  LabelMenu = gtk_label_new(_("General"));
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook), Frame, LabelOnglet, LabelMenu);

  window1 = Frame;
  g_object_set_data (G_OBJECT (window1), "window1", window1);
  create_notebook_gene(Frame);

  gen->title=entry20;
  gen->memory=entry21;

  gen->gth_zero=entry1 ; 
  gen->gth_oneint=entry2 ; 
  gen->gth_twoint=entry3 ; 
  gen->gth_prefac=entry4 ; 
  gen->gth_locali=entry5 ; 
  gen->gth_eorder=entry6 ; 
  gen->gth_energy=entry7 ; 
  gen->gth_gradiant=entry8 ; 
  gen->gth_step=entry9 ; 
  gen->gth_orbital=entry10 ; 
  gen->gth_civec=entry11; 
  gen->gth_coeff=entry12; 
  gen->gth_printci=entry13; 
  gen->gth_punchci=entry14; 

  gen->basis=checkbutton[0];
  gen->civector=checkbutton[1];
  gen->ref=checkbutton[2];
  gen->io=checkbutton[3];
  gen->orbital=checkbutton[4];
  gen->angles=checkbutton[5];
  gen->cs=checkbutton[6];
  gen->distance=checkbutton[7];
  gen->pspace=checkbutton[8];
  gen->variable=checkbutton[9];
  gen->cp=checkbutton[10];
  gen->micro=checkbutton[11];
  gen->cpu=checkbutton[12];
  gen->pairs=checkbutton[13];

    gen->RadCenter=ButtonRad1;
    gen->entryC=entryC;
    gen->entryX=entryX;
    gen->entryY=entryY;
    gen->entryZ=entryZ;

    gen->Overlap=checkbutton[14];
    gen->Kinetic=checkbutton[15];
    gen->Potential=checkbutton[16];
    gen->Delta=checkbutton[17];
    gen->Delta4=checkbutton[18];
    gen->Darwin=checkbutton[19];

    gen->MassVelocity=checkbutton[20];
    gen->CowanGriffin=checkbutton[21];
    gen->ProductsMO=checkbutton[22];
    gen->Dipole=checkbutton[23];
    gen->Second=checkbutton[24];
    gen->Quadrupole=checkbutton[25];

    gen->Electric=checkbutton[26];
    gen->Field=checkbutton[27];
    gen->AngularMO=checkbutton[28];
    gen->Velocity=checkbutton[29];
    gen->SpinOrbit=checkbutton[30];
    gen->ECP=checkbutton[31];
}

