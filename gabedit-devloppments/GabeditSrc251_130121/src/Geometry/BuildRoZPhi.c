/* BuildRoZPhi.c */
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
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Geometry/MenuToolBarGeom.h"

static GtkWidget* Entries[6];
static GtkWidget* WinTable = NULL;
static gboolean First = TRUE;

/********************************************************************************/
static void build_rozphi_molecule(GtkWidget *w,gpointer data)
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
	gdouble Z;
	gint N0=0;
	gdouble phi=0;


	if(First)
	{
		unselect_all_atoms();
		if(GeomXYZ)
   			freeGeomXYZ();
		if(VariablesXYZ)
			freeVariablesXYZ(VariablesXYZ);
		NcentersXYZ = 0;

	}
	N0 = NcentersXYZ;
		
	First = FALSE;
	Symb = gtk_entry_get_text(GTK_ENTRY(Entries[0]));
	t = gtk_entry_get_text(GTK_ENTRY(Entries[1]));
	Nc = atoi(t);
	t = gtk_entry_get_text(GTK_ENTRY(Entries[2]));
	Distance = atof(t);
	if(Nc==1)
		Distance = 0.0;

	t = gtk_entry_get_text(GTK_ENTRY(Entries[4]));
	Z = atof(t);

	t = gtk_entry_get_text(GTK_ENTRY(Entries[5]));
	phi = atof(t);

	if(Units==0)
	{
		Distance *= ANG_TO_BOHR;
		Z *= ANG_TO_BOHR;
	}


	if(Nc<1)
		return;
	angle = (360.0/Nc)*DEG_TO_RAD;
	Ro = sqrt(2*(1-cos(angle) )); 
	Ro = Distance/Ro; 

	phi *= DEG_TO_RAD;
	init_dipole();


	NcentersXYZ += Nc;
	if(!GeomXYZ)
		GeomXYZ=g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));
	else
		GeomXYZ=g_realloc(GeomXYZ,NcentersXYZ*sizeof(GeomXYZAtomDef));

	for(j=N0;j<N0+Nc;j++)
	{
		GeomXYZ[j].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[j].Symb=g_strdup(Symb);
		GeomXYZ[j].mmType=g_strdup(Symb);
		GeomXYZ[j].pdbType=g_strdup(Symb);
		GeomXYZ[j].Residue=g_strdup(Symb);
		GeomXYZ[j].ResidueNumber=0;
		X = Ro*cos(j*angle+phi);
		Y = Ro*sin(j*angle+phi);

		GeomXYZ[j].X=g_strdup_printf("%f",X);
		GeomXYZ[j].Y=g_strdup_printf("%f",Y);
		GeomXYZ[j].Z=g_strdup_printf("%f",Z);
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
static void set_ro_entry (GtkWidget *w,gpointer data)
{
	G_CONST_RETURN gchar* t0;
	gchar* t;
	gdouble Distance;
	gdouble Ro; 
	gint Nc;
	gdouble angle;
	t0 = gtk_entry_get_text(GTK_ENTRY(Entries[2]));
	Distance = atof(t0);

	t0 = gtk_entry_get_text(GTK_ENTRY(Entries[1]));
	Nc = atoi(t0);

	if(Nc<1) return;
	angle = (360.0/Nc)*DEG_TO_RAD;
	Ro = sqrt(2*(1-cos(angle) )); 
	Ro = Distance/Ro; 
	t = g_strdup_printf("%f",Ro);
	if(Nc>1)
		gtk_entry_set_text(GTK_ENTRY(Entries[3]),t);
	else
		gtk_entry_set_text(GTK_ENTRY(Entries[3]),"0.0");

	g_free(t);
	
}
/********************************************************************************/
static void set_atom(GtkWidget *w,gpointer data)
{
  gtk_entry_set_text(GTK_ENTRY(Entries[0]),(char *)data);
  gtk_editable_set_editable((GtkEditable*) Entries[0],FALSE);
  gtk_widget_destroy(WinTable);
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
  
  WinTable = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_modal(GTK_WINDOW(WinTable),TRUE);
  gtk_window_set_title(GTK_WINDOW(WinTable),_("Select your atom"));
  gtk_window_set_default_size (GTK_WINDOW(WinTable),(gint)(ScreenWidth*0.5),(gint)(ScreenHeight*0.4));

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);

  gtk_container_add(GTK_CONTAINER(WinTable),frame);  
  gtk_widget_show (frame);

  Table = gtk_table_new(PERIODIC_TABLE_N_ROWS-1,PERIODIC_TABLE_N_COLUMNS,TRUE);
  gtk_container_add(GTK_CONTAINER(frame),Table);
  button_style = gtk_widget_get_style(WinTable); 
  
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
 	
  gtk_widget_show_all(WinTable);
  
}
/**********************************************************************/
void build_rozphi_molecule_dlg()
{
  GtkWidget *Dlg;
  GtkWidget *Button;
  GtkWidget *frame;
  GtkWidget *vboxframe;
  gint i = 0;
  gchar* Symb = "C";
  gchar* t = NULL;
  SAtomsProp P = prop_atom_get(Symb);
  GtkWidget *Table;
  GtkWidget *Label;
  gint nligne = 6;
  gchar* tlabel[]={
	N_("Atom"),
	N_("Number of atoms"),
	N_("Distance between atoms(Ang)"),
	N_("Distance betwenn an atom to center of Fragment(Ang)"),
	N_("Z of plan of Fragment(Ang)"),
	N_("Angle betwenn X axis and the first atom of Fragment(Deg.)")
	};
  gchar* tentry[]={
	"C",
	"2",
	"2.0",
	"2.0",
	"0.0",
	"0.0"
	};



  First = TRUE;
  Dlg = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dlg),_("Build a molecule with an axis symmetry"));
  gtk_window_set_position(GTK_WINDOW(Dlg),GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (Dlg), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(Dlg),GTK_WINDOW(GeomDlg));


  add_child(GeomDlg,Dlg,gtk_widget_destroy,_(" Build sym. mol. "));

  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)delete_child,NULL);
  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)gtk_widget_destroy,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
   gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->vbox), frame,TRUE,TRUE,0);

  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

  Table = gtk_table_new(6,4,FALSE);
  gtk_container_add(GTK_CONTAINER(vboxframe),Table);
  

  for(i=0;i<nligne;i++)
  {
	  Label = gtk_label_new(tlabel[i]);
	  gtk_table_attach(GTK_TABLE(Table),Label,0,1,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
		  1,1);
	  Label = gtk_label_new(":");
	  gtk_table_attach(GTK_TABLE(Table),Label,1,2,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
		  1,1);

	  Entries[i] = gtk_entry_new();
	  gtk_table_attach(GTK_TABLE(Table),Entries[i],2,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
		  1,1);
	  gtk_entry_set_text(GTK_ENTRY(Entries[i]),tentry[i]);

  }
  Button = gtk_button_new_with_label(_(" Set "));
  g_signal_connect(G_OBJECT(Button), "clicked", (GCallback)select_atom,Entries[0]);
  gtk_table_attach(GTK_TABLE(Table),Button,3,4,0,1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);
  gtk_editable_set_editable((GtkEditable*) Entries[0],FALSE);

  gtk_editable_set_editable((GtkEditable*) Entries[3],FALSE);
  g_signal_connect(G_OBJECT(Entries[1]), "changed", (GCallback)set_ro_entry,Entries[3]);
  g_signal_connect(G_OBJECT(Entries[2]), "changed", (GCallback)set_ro_entry,Entries[3]);

  t = g_strdup_printf("%f",0.95*(P.covalentRadii+P.covalentRadii)*BOHR_TO_ANG);
  gtk_entry_set_text(GTK_ENTRY(Entries[2]),t);
  g_free(t);

  Button = gtk_button_new_with_label(_("  Insert  "));
  g_signal_connect(G_OBJECT(Button), "clicked", (GCallback)build_rozphi_molecule,NULL);
  gtk_table_attach(GTK_TABLE(Table),Button,3,4,2,4,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  1,1);

	gtk_widget_show_all(Table);

  /* The "Close" button */
  gtk_widget_realize(Dlg);
  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Dlg)->action_area), FALSE);
  Button = create_button(Dlg,_("Close"));
  gtk_box_pack_end (GTK_BOX(GTK_DIALOG(Dlg)->action_area), Button, FALSE, TRUE, 5);  
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)delete_child,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);

  gtk_widget_show_all(GTK_DIALOG(Dlg)->vbox);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->action_area);
  gtk_widget_show_now(Dlg);

  /* fit_windows_position(GeomDlg, Dlg);*/
}

