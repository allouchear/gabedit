/* PlanesMapped.c */
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
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "../Display/GlobalOrb.h"
#include "../Display/Orbitals.h"
#include "../Display/GLArea.h"
#include "../Display/UtilsOrb.h"
#include "../Utils/UtilsInterface.h"
#include "../Display/PlanesMapped.h"
#include "../Utils/Utils.h"

/********************************************************************************/
static void apply_maps(GtkWidget *Win,gpointer data)
{
	GtkWidget** Entries =(GtkWidget**)g_object_get_data(G_OBJECT (Win), "Entries");
	gint type = *((gint*)g_object_get_data(G_OBJECT (Win), "Type"));
	G_CONST_RETURN gchar* temp;
	gint i0=0;
	gint i1=1;
	gint numPlane = -1;
	gint pvalue = 0;
	gdouble gap = 0;
	
        temp	= gtk_entry_get_text(GTK_ENTRY(Entries[0])); 
	pvalue = atoi(temp);

        if(!get_a_float(Entries[1],&gap,_("Error : The projection value should be float."))) return;
	numPlane = pvalue-1;
	if(numPlane<0 || numPlane>grid->N[type]) numPlane = grid->N[type]/2;
	switch(type)
	{
		case 0 : i0 = 1;i1 = 2;break; /* plane YZ */
		case 1 : i0 = 0;i1 = 2;break; /* plane XZ */
		case 2 : i0 = 0;i1 = 1;break; /* plane XY */
	}

	add_maps(i0, i1, numPlane, gap, FALSE);
  	delete_child(Win);
  	/*gtk_widget_destroy(Win);*/
}
/********************************************************************************/
static GtkWidget *create_maps_frame( GtkWidget *vboxall,gchar* title,gint type)
{
	GtkWidget *frame;
	GtkWidget *combo;
	GtkWidget *vboxframe;
	GtkWidget *hseparator;
	GtkWidget **Entries = (GtkWidget **)g_malloc(2*sizeof(GtkWidget *));
	gushort i;
	GtkWidget *Table;
	gchar** listvalues;
	static gint itype = 0;
	gchar      *strLabelPlane = NULL;
	
	itype = type;
	strLabelPlane = g_strdup(_(" Plane number "));
	listvalues = g_malloc(grid->N[type]*sizeof(gchar*));
	for(i=0;i<grid->N[type];i++)
	{
		listvalues[i] = g_strdup_printf("%d",i+1);
	}

	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	Table = gtk_table_new(3,4,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	i = 0;
	add_label_at_table(Table,strLabelPlane,i,(gushort)0,GTK_JUSTIFY_LEFT);
	add_label_at_table(Table," : ",i,(gushort)1,GTK_JUSTIFY_LEFT);
	combo = create_combo_box_entry(listvalues,grid->N[type], FALSE,-1,-1);
	Entries[0] = GTK_BIN(combo)->child;
	add_widget_table(Table,combo,(gushort)0,(gushort)2);

	i=1;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(Table),hseparator,0,4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  3,3);

	i = 2;
	add_label_at_table(Table, _("Projection "),i,(gushort)0,GTK_JUSTIFY_LEFT);
	add_label_at_table(Table," : ",i,(gushort)1,GTK_JUSTIFY_LEFT);
	Entries[1] = gtk_entry_new ();
	add_widget_table(Table,Entries[1],(gushort)i,(gushort)2);
	add_label_at_table(Table,_(" left if <0, right if >0. and nothing if 0 "),i,(gushort)3,GTK_JUSTIFY_LEFT);

	g_object_set_data(G_OBJECT (frame), "Entries",Entries);
	g_object_set_data(G_OBJECT (GTK_OBJECT(GTK_COMBO_BOX(combo))), "Entries",Entries);
	g_object_set_data(G_OBJECT (GTK_OBJECT(GTK_COMBO_BOX(combo))), "Type",&itype);
	g_object_set_data(G_OBJECT(Entries[0]), "ComboList",GTK_OBJECT(GTK_COMBO_BOX(combo)));
	gtk_entry_set_text(GTK_ENTRY(Entries[0]),listvalues[grid->N[type]/2]);
	gtk_entry_set_text(GTK_ENTRY(Entries[1]),"0.0");

	g_free(strLabelPlane);
	for(i=0;i<grid->N[type];i++)
	{
		if(listvalues[i])
		       g_free(listvalues[i]);
	}
	g_free(listvalues);

	gtk_widget_show_all(frame);
  
  	return frame;
}

/**************************************************************/
void create_maps(gchar* title,gint type)
{
  GtkWidget *Win;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *button;
  GtkWidget** Entries;
  static gint itype;


  if(!grid)
  {
	  if(!CancelCalcul) Message(_("Grid not defined "),_("Error"),TRUE);
	  return;
  }
  itype = type;
  
  Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Win),title);
  gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
  gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
  gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

  add_glarea_child(Win," Maps ");

  vboxall = create_vbox(Win);
  vboxwin = vboxall;

  frame = create_maps_frame(vboxall,"Maps",type);
  Entries = (GtkWidget**) g_object_get_data(G_OBJECT (frame), "Entries");
  g_object_set_data(G_OBJECT (Win), "Entries",Entries);
  g_object_set_data(G_OBJECT (Win), "Type",&itype);


  hbox = create_hbox_false(vboxwin);
  gtk_widget_realize(Win);

  button = create_button(Win,_("Cancel"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, GTK_OBJECT(Win));
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(Win));
  gtk_widget_show (button);

  button = create_button(Win,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_maps,GTK_OBJECT(Win));
  

  gtk_widget_show_all (Win);
}
/********************************************************************************/
static void apply_maps_plane(GtkWidget *Win,gpointer data)
{
			
	GtkWidget* entry =(GtkWidget*)g_object_get_data(G_OBJECT (Win), "Entry");
	gdouble gap;
	
        if(!get_a_float(entry, &gap, _("Error : The projection value should be float."))) return;
	add_maps(0, 1, 0,  gap, TRUE);
	glarea_rafresh(GLArea);
  	delete_child(Win);
}
/********************************************************************************/
static void destroyWin(GtkWidget *Win)
{

	gtk_widget_destroy(Win);
	reDrawPlaneMappedPlane = TRUE;
}
/********************************************************************************/
static GtkWidget *create_maps_frame_plane( GtkWidget *vboxall,gchar* title)
{
	GtkWidget *frame;
	GtkWidget *vboxframe;
	GtkWidget *entry = NULL;
	gushort i;
	GtkWidget *Table;

	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	Table = gtk_table_new(1,4,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	i = 0;
	add_label_at_table(Table, _("Projection "),i,(gushort)0,GTK_JUSTIFY_LEFT);
	add_label_at_table(Table," : ",i,(gushort)1,GTK_JUSTIFY_LEFT);
	entry = gtk_entry_new ();
	add_widget_table(Table, entry, (gushort)i, (gushort)2);
	add_label_at_table(Table,_(" left if <0, right if >0. and nothing if 0"),i,(gushort)3,GTK_JUSTIFY_LEFT);
	gtk_entry_set_text(GTK_ENTRY(entry),"0.0");

	g_object_set_data(G_OBJECT (frame), "Entry",entry);
	gtk_widget_show_all(frame);
  
  	return frame;
}
/********************************************************************************/
void create_maps_plane(gchar* title)
{
  GtkWidget *Win;
  GtkWidget *frame;
  GtkWidget *hbox;
  GtkWidget *vboxall;
  GtkWidget *vboxwin;
  GtkWidget *button;
  GtkWidget* entry;


  if(!gridPlaneForPlanesMapped)
  {
	  Message(_("Grid in plane is not defined "),_("Error"),TRUE);
	  return;
  }
  /* Principal Window */
  Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(Win),title);
  gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
  gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
  gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
  gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

  add_child(PrincipalWindow,Win,destroyWin,"Plane Colorcoded.");
  g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)delete_child,NULL);

  vboxall = create_vbox(Win);
  vboxwin = vboxall;

  frame = create_maps_frame_plane(vboxall,"Maps");
  entry = (GtkWidget*) g_object_get_data(G_OBJECT (frame), "Entry");
  g_object_set_data(G_OBJECT (Win), "Entry",entry);

  /* buttons box */
  hbox = create_hbox_false(vboxwin);
  gtk_widget_realize(Win);

  button = create_button(Win,_("Cancel"));
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, GTK_OBJECT(Win));
  gtk_widget_show (button);

  button = create_button(Win,_("OK"));
  gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  gtk_widget_show (button);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_maps_plane,GTK_OBJECT(Win));
  

  /* Show all */
  gtk_widget_show_all (Win);
}
/********************************************************************************/
