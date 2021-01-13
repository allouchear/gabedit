/* BuildNanoTube.c */
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
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Geometry/MenuToolBarGeom.h"

static GtkWidget* Entries[4];
static gint entryWidth = 40;

/********************************************************************************/
static void build_nanotube(GtkWidget *w,gpointer data)
{
	G_CONST_RETURN gchar *te;
	/* refrence : Phys. Rev. B, 47, 5485 (1993) */
	
      	gint n=10; /*          First integer of the pair (n,m), which defines the tube chirality.*/
      	gint m=7; /*           Second integer of the pair (n,m), which defines the tube chirality.*/
	gdouble b = 1.422; /*  Carbon-carbon bond length in graphene sheet, in Angstroms */
	gint ncells = 2; /*  Number of unit cells to be generated */

	gint hcd; /* the higher common divisor of both n and m */
	gint L;
	gdouble Rm;
	gdouble radius; /* tube radius */
	gint dR; 
	gint Nc; /* the number of two-atom unit cells in the translational cell */
	gdouble phi;
	gint p1;
	gdouble t;
	gint p2;
	gdouble alpha;
	gdouble h;
	gdouble* coord[4];
	gint i;

	te = gtk_entry_get_text(GTK_ENTRY(Entries[0]));
	n = atoi(te);
	if(n<=0) n = 10;
	te = gtk_entry_get_text(GTK_ENTRY(Entries[1]));
	m = atoi(te);
	if(m<0) m = 0;
	te = gtk_entry_get_text(GTK_ENTRY(Entries[2]));
	b = atof(te);
	if(b<=0) b = 1.422;
	te = gtk_entry_get_text(GTK_ENTRY(Entries[3]));
	ncells = atoi(te);
	if(ncells<1) ncells = 1;

	unselect_all_atoms();
	if(Units==0) b *= ANG_TO_BOHR;

	if(m==0) hcd = n;
	else
	{
		for(hcd = m; hcd>=1; hcd--)
			if(n%hcd ==0 && m%hcd==0) break;
	}
	L = m*m+n*m+n*n;
	Rm = b*sqrt(3.0*L);
	radius = Rm/2.0/PI;
	if((n-m) % (3*hcd)==0)
		dR = 3*hcd;
	else dR = hcd;

	Nc = 2*L/dR;

	if(Nc<1) return;

	for(i=0;i<4;i++)
		coord[i] = g_malloc(2*Nc*ncells*sizeof(gdouble));
	phi = (PI*(m+n))/L;
	t = (b*(m-n)*(m-n))/(2*L);
	p1 = 0;
	p2 = 0;
	for(p1=0; p1<=n; p1++)
	{
		gdouble pp2;
		pp2 = (gdouble)(hcd+p1*m)/n;
		if(fabs(pp2 -(gint)pp2)<1e-10)
		{
			p2 = (gint)pp2;
			break;
		}
	}

	alpha = PI*(m*(2*p2+p1)+n*(2*p1+p2))/L;
	h = (3*hcd*b)/(2*sqrt(1.0*L));

	/* Set the first atom position in the two-atom unit cell */
	coord[0][0] = radius;
	coord[1][0] = 0.0;
	coord[2][0] = 0.0;
	coord[3][0] = 0.0;

	/* Set the second atom position in the two-atom unit cell */
	coord[0][1] = radius*cos(phi);
	coord[1][1] = radius*sin(phi);
	coord[2][1] = t;
	coord[3][1] = coord[3][0]+phi;
	/* Complete the tubule helical motif */
	for(i=2; i<2*hcd; i++)
	{
		coord[0][i] = radius*cos(coord[3][i-2]+(2*PI)/hcd);
		coord[1][i] = radius*sin(coord[3][i-2]+(2*PI)/hcd);
		coord[2][i] = coord[2][i-2];
		coord[3][i] = coord[3][i-2]+(2*PI)/hcd;
	}
	/* Complete the translational unit cell */
	for(i=2*hcd; i<2*Nc*ncells; i++)
	{
		coord[0][i] = radius*cos((coord[3][i-(2*hcd)])+alpha);
		coord[1][i] = radius*sin((coord[3][i-(2*hcd)])+alpha);
		coord[2][i] = coord[2][i-(2*hcd)]+h;
		coord[3][i] = coord[3][i-(2*hcd)]+alpha;
	}

	init_dipole();
 	if(GeomXYZ) freeGeomXYZ();
 	if(VariablesXYZ) freeVariablesXYZ(VariablesXYZ);

	NcentersXYZ = 2*Nc*ncells;
	GeomXYZ=g_malloc(NcentersXYZ*sizeof(GeomXYZAtomDef));

	for(i=0;i<NcentersXYZ;i++)
	{
		GeomXYZ[i].Nentry=NUMBER_LIST_XYZ;
		GeomXYZ[i].Symb=g_strdup("C");
		GeomXYZ[i].mmType=g_strdup("CA");
		GeomXYZ[i].pdbType=g_strdup("CA");
		GeomXYZ[i].Residue=g_strdup("TUB");
		GeomXYZ[i].ResidueNumber=0;
		GeomXYZ[i].X=g_strdup_printf("%f",coord[0][i]);
		GeomXYZ[i].Y=g_strdup_printf("%f",coord[1][i]);
		GeomXYZ[i].Z=g_strdup_printf("%f",coord[2][i]);
		GeomXYZ[i].Charge=g_strdup("0.0");
    		GeomXYZ[i].Layer=g_strdup(" ");
    		GeomXYZ[i].typeConnections=NULL;
  	}
        MethodeGeom = GEOM_IS_XYZ;
	for(i=0;i<4;i++)
		g_free(coord[i]);

 	if(GeomIsOpen) create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
  	if(GeomDrawingArea != NULL) rafresh_drawing();
	activate_rotation();
}
/********************************************************************************/
void build_nanotube_dlg()
{
  GtkWidget *Dlg;
  GtkWidget *Button;
  GtkWidget *frame;
  GtkWidget *vboxframe;
  GtkWidget *table;
  GtkWidget *label;
  gint i = 0;
  
  Dlg = gtk_dialog_new();
  gtk_window_set_title(GTK_WINDOW(Dlg),_("Build Natotube molecule"));
  gtk_window_set_position(GTK_WINDOW(Dlg),GTK_WIN_POS_CENTER);
  gtk_window_set_transient_for(GTK_WINDOW(Dlg),GTK_WINDOW(GeomDlg));


  add_child(GeomDlg,Dlg,gtk_widget_destroy,_(" Build lin. mol. "));

  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)delete_child,NULL);
  g_signal_connect(G_OBJECT(Dlg),"delete_event",(GCallback)gtk_widget_destroy,NULL);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
  gtk_container_set_border_width (GTK_CONTAINER (frame), 10);
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dlg)->vbox), frame,TRUE,TRUE,0);
  gtk_widget_show (frame);

  vboxframe = create_vbox(frame);

   table = gtk_table_new(3,8,FALSE);
   gtk_box_pack_start (GTK_BOX (vboxframe), table, TRUE, TRUE, 3);

   i = 0;
   label = gtk_label_new(_("Chirality"));
   gtk_table_attach(GTK_TABLE(table),label,0,1,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
   label = gtk_label_new(":");
   gtk_table_attach(GTK_TABLE(table),label,1,2,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
   label = gtk_label_new("(");
   gtk_table_attach(GTK_TABLE(table),label,2,3,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

   Entries[0] = gtk_entry_new();
   gtk_widget_set_size_request(GTK_WIDGET(Entries[0]),entryWidth,-1);
   gtk_entry_set_text(GTK_ENTRY(Entries[0]),"10");
   gtk_table_attach(GTK_TABLE(table),Entries[0],3,4,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

   label = gtk_label_new(",");
   gtk_table_attach(GTK_TABLE(table),label,4,5,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

   Entries[1] = gtk_entry_new();
   gtk_widget_set_size_request(GTK_WIDGET(Entries[1]),entryWidth,-1);
   gtk_entry_set_text(GTK_ENTRY(Entries[1]),"0");
   gtk_table_attach(GTK_TABLE(table),Entries[1],5,6,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

   label = gtk_label_new(")");
   gtk_table_attach(GTK_TABLE(table),label,6,7,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

   label = gtk_label_new(_(" see PRB,47,5485"));
   gtk_table_attach(GTK_TABLE(table),label,7,8,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

   i = 1;
   label = gtk_label_new(_("C-C bond length"));
   gtk_table_attach(GTK_TABLE(table),label,0,1,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
   label = gtk_label_new(":");
   gtk_table_attach(GTK_TABLE(table),label,1,2,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

   Entries[2] = gtk_entry_new();
   gtk_widget_set_size_request(GTK_WIDGET(Entries[2]),entryWidth*2,-1);
   gtk_entry_set_text(GTK_ENTRY(Entries[2]),"1.422");
   gtk_table_attach(GTK_TABLE(table),Entries[2],2,7,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

   label = gtk_label_new(_("Angstroms"));
   gtk_table_attach(GTK_TABLE(table),label,7,8,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

   i = 2;
   label = gtk_label_new(_("Number of translational unit cells"));
   gtk_table_attach(GTK_TABLE(table),label,0,1,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);
   label = gtk_label_new(":");
   gtk_table_attach(GTK_TABLE(table),label,1,2,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

   Entries[3] = gtk_entry_new();
   gtk_widget_set_size_request(GTK_WIDGET(Entries[3]),entryWidth,-1);
   gtk_entry_set_text(GTK_ENTRY(Entries[3]),"1");
   gtk_table_attach(GTK_TABLE(table),Entries[3],2,8,i,i+1,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,(GtkAttachOptions)(GTK_FILL|GTK_SHRINK),1,1);

  gtk_widget_realize(Dlg);
  /* The "Cancel" button */
  Button = create_button(Dlg,_("Cancel"));
  gtk_box_pack_start( GTK_BOX(GTK_DIALOG(Dlg)->action_area), Button,TRUE,TRUE,0);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)delete_child,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);

  /* The "OK" button */
  Button = create_button(Dlg,_("OK"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(Dlg)->action_area), Button,TRUE,TRUE,0);
  g_signal_connect(G_OBJECT(Button), "clicked",(GCallback)build_nanotube,NULL);
  g_signal_connect_swapped(G_OBJECT(Button), "clicked",(GCallback)delete_child,GTK_OBJECT(Dlg));
  GTK_WIDGET_SET_FLAGS(Button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(Button);
    
  gtk_widget_show_all(GTK_DIALOG(Dlg)->vbox);
  gtk_widget_show_all(GTK_DIALOG(Dlg)->action_area);
  gtk_widget_show_now(Dlg);

  /* fit_windows_position(GeomDlg, Dlg);*/
}

