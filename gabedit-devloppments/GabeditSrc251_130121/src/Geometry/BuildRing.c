/* BuildRing.c */
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Geometry/MenuToolBarGeom.h"

static GtkWidget* Entries[3];
static GtkWidget* FenetreTable = NULL;

/********************************************************************************/
static void build_ring_molecule(GtkWidget *w,gpointer data)
{
	G_CONST_RETURN gchar *t;
	gint Nc;
	gdouble Distance;
	gdouble Ro;
	G_CONST_RETURN gchar* Symb;
	gint j;
	gdouble angle;
	gdouble X;
	gdouble Y;

	Symb = gtk_entry_get_text(GTK_ENTRY(Entries[0]));
	t = gtk_entry_get_text(GTK_ENTRY(Entries[1]));
	Nc = atoi(t);
	t = gtk_entry_get_text(GTK_ENTRY(Entries[2]));
	Distance = atof(t);

	unselect_all_atoms();
	if(Units==0)
		Distance *= ANG_TO_BOHR;


	if(Nc<1)
		return;
	angle = (360.0/Nc)*DEG_TO_RAD;
	Ro = sqrt(2*(1-cos(angle) )); 
	Ro = Distance/Ro; 

	init_dipole();
 	if(GeomXYZ)
   		freeGeomXYZ();
 	if(VariablesXYZ)
		freeVariablesXYZ(VariablesXYZ);


	NcentersXYZ = Nc;
	GeomXYZ=g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));

	for(j=0;j<Nc;j++)
	{
		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(Symb);
		GeomXYZ[j].mmType=g_strdup(Symb);
		GeomXYZ[j].pdbType=g_strdup(Symb);
		GeomXYZ[j].Residue=g_strdup(Symb);
		GeomXYZ[j].ResidueNumber=0;
		X = Ro*cos(j*angle);
		Y = Ro*sin(j*angle);

		GeomXYZ[j].X=g_strdup_printf("%f",X);
		GeomXYZ[j].Y=g_strdup_printf("%f",Y);
		GeomXYZ[j].Z=g_strdup("0.0");
    		GeomXYZ[j].Layer=g_strdup(" ");
		GeomXYZ[j].Charge=g_strdup("0.0");
		GeomXYZ[j].typeConnections=NULL;
  	}
        MethodeGeom = GEOM_IS_XYZ;

 	if(GeomIsOpen)
		create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
  	if(GeomDrawingArea != NULL)
		rafresh_drawing();

	activate_rotation();
}
/********************************************************************************/
static void set_entry_distance(GtkWidget *w,gpointer data)
{
  
  SAtomsProp P = prop_atom_get((gchar*)data);
  gchar* t = g_strdup_printf("%f",0.95*(P.covalentRadii+P.covalentRadii)*BOHR_TO_ANG);
  gtk_entry_set_text(GTK_ENTRY(Entries[2]),t);
  g_free(t);
  g_free(P.name);
  g_free(P.symbol);
}
/********************************************************************************/
static void set_atom(GtkWidget *w,gpointer data)
{
  gtk_entry_set_text(GTK_ENTRY(Entries[0]),(char *)data);
  gtk_editable_set_editable((GtkEditable*) Entries[0],FALSE);
  gtk_widget_destroy(FenetreTable);
  set_entry_distance(NULL,data);
}
/********************************************************************************/
static void select_atom(GtkWidget *w,gpointer entry0)
{
	GtkWidget* Table;
	GtkWidget* button;
	GtkWidget* frame;
	guint i;
	guint j;
        GtkStyle *button_style;
        GtkStyle *style;

	gchar*** Symb = get_periodic_table();
  
  FenetreTable = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(FenetreTable),TRUE);
  gtk_window_set_title(GTK_WINDOW(FenetreTable),_("Select your atom"));
  gtk_window_set_default_size (GTK_WINDOW(FenetreTable),(gint)(ScreenWidth*0.5),(gint)(ScreenHeight*0.4));

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

  gtk_container_add(GTK_CONTAINER(FenetreTable),frame);  
  gtk_widget_show (frame);

  Table = gtk_table_new(PERIODIC_TABLE_N_ROWS-1,PERIODIC_TABLE_N_COLUMNS,TRUE);
  gtk_container_add(GTK_CONTAINER(frame),Table);
  button_style = gtk_widget_get_style(FenetreTable); 
  
  for ( i = 0;i<PERIODIC_TABLE_N_ROWS-1;i++)
	  for ( j = 0;j<PERIODIC_TABLE_N_COLUMNS;j++)
  {
	  if(strcmp(Symb[j][i],"00"))
	  {
	  button = gtk_button_new_with_label(Symb[j][i]);
          style=set_button_style(button_style,button,Symb[j][i]);
          g_signal_connect(G_OBJECT(button), "clicked",
                            (GCallback)set_atom,(gpointer )Symb[j][i]);
	  gtk_table_attach(GTK_TABLE(Table),button,j,j+1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
	  }

  }
 	
  gtk_widget_show_all(FenetreTable);
  
}
/**********************************************************************/
void build_ring_molecule_dlg()
{
  GtkWidget *Dlg;
  GtkWidget *Button;
  GtkWidget *hbox;
  GtkWidget *frame;
  GtkWidget *vboxframe;
  gchar** tlist;
  gint nlist = 12;
  gint i = 0;
  gchar* Symb = "C";
  gchar* t = NULL;
  SAtomsProp P = prop_atom_get(Symb);
  
  Dlg = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dlg),_("Build Ring molecule"));
  gtk_window_set_position(GTK_WINDOW(Dlg),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dlg),GTK_WINDOW(GeomDlg));


  add_child(GeomDlg,Dlg,gtk_widget_destroy,_(" Build Ring mol. "));

  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)delete_child,NULL);
  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)gtk_widget_destroy,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dlg)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);


  /* The Atom Entry */
  hbox=create_hbox_false(vboxframe);
  Entries[0] = create_label_entry(hbox,_("Atom                       : "),(gint)(ScreenHeight*0.2),(gint)(ScreenHeight*0.15));
  gtk_entry_set_text(GTK_ENTRY(Entries[0]),Symb);
  gtk_editable_set_editable((GtkEditable*) Entries[0],FALSE);

  Button = gtk_button_new_with_label(_(" Set "));
  gtk_box_pack_start (GTK_BOX(hbox), Button, TRUE, TRUE, 5);
  g_signal_connect(G_OBJECT(Button), "clicked", (GCallback)select_atom,Entries[0]);

  /* The Number of atoms Entry */
  tlist = g_malloc(nlist*sizeof(gchar*));
  for(i=0;i<nlist;i++)
	  tlist[i] = g_strdup_printf("%d",i+3);
  hbox=create_hbox_false(vboxframe);
  Entries[1] = create_label_combo(hbox,_("Number of atoms            : "),tlist,nlist,TRUE,(gint)(ScreenHeight*0.2),(gint)(ScreenHeight*0.15));

  for(i=0;i<nlist;i++)
	  g_free(tlist[i]);
  g_free(tlist);

  /* The Distance between atoms */
  hbox = create_hbox_false(vboxframe);
  t = g_strdup_printf("%f",0.95*(P.covalentRadii+P.covalentRadii)*BOHR_TO_ANG);
  Entries[2] = create_label_entry(hbox,_("Distance between atoms(Ang) : "),(gint)(ScreenHeight*0.2),(gint)(ScreenHeight*0.15));
  gtk_entry_set_text(GTK_ENTRY(Entries[2]),t);
  g_free(t);


  gtk_widget_realize(Dlg);
  /* The "Cancel" button */
  Button = create_button(Dlg,_("Cancel"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dlg)->action_area), Button,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)delete_child,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);

  /* The "OK" button */
  Button = create_button(Dlg,_("OK"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->action_area), Button,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)build_ring_molecule,NULL);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)delete_child,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);

  gtk_widget_show_all(GTK_DIALOG(Dlg)->vbox);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->action_area);
  gtk_widget_show_now(Dlg);

  /*fit_windows_position(GeomDlg, Dlg);*/
}

