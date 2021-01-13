/* MInterfaceGeom.c */
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

#include "MInterfaceGeom.h"
#include "../Common/Global.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/GeomZmatrix.h"
#include "../Gaussian/GaussGlobal.h"
#include "../Utils/UtilsInterface.h"
#include "../Geometry/InterfaceGeom.h"

/************************************************************************************************************/
void AjoutePageGeom(GtkWidget *NoteBook,GeomS *geom)
{
  GtkWidget *hbox;
  GtkWidget *FrameType;
  GtkWidget *Frame;
  GtkWidget *LabelOnglet;
  GtkWidget *LabelMenu;
  GtkWidget *BoiteV;
  GtkWidget *window1;
  GtkWidget *vbox1;
  GtkWidget *hbox1;
  GtkWidget *label2;
  GtkWidget *label3;
  GtkWidget *hseparator2;
  GtkWidget *combo2;
  GtkWidget *combo_entry2;
  GtkWidget *combo3;
  GtkWidget *combo_entry3;
  GtkWidget *combobox;
  GList *combo2_items = NULL;
  GList *combo3_items = NULL;


  geom_is_open();
  geominter=g_malloc(sizeof(GeomInter));
  geominter->window= gtk_frame_new(NULL);
  HboxGeom = NULL;


  Frame =geominter->window;
  gtk_container_set_border_width(GTK_CONTAINER(Frame), 10);



  LabelOnglet = gtk_label_new(_("Geometry"));
  LabelMenu = gtk_label_new(_("Geometry"));
  gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
                                Frame,
                                LabelOnglet, LabelMenu);
 
  BoiteV = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(Frame), BoiteV);

  window1=Frame;
  vbox1 = BoiteV;
  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (vbox1), hbox1, FALSE, FALSE, 0);


  label2 = gtk_label_new (_(" Symmetry : "));
  gtk_widget_show (label2);
  gtk_box_pack_start (GTK_BOX (hbox1), label2, FALSE, FALSE, 0);

  combo2 = gtk_combo_box_entry_new_text ();
  gtk_widget_set_size_request(GTK_WIDGET(combo2),100,-1);
  gtk_widget_show (combo2);
  gtk_box_pack_start (GTK_BOX (hbox1), combo2, FALSE, FALSE, 0);
  combo2_items = g_list_append (combo2_items, _("default"));
  combo2_items = g_list_append (combo2_items, "NOSYM");
  combo2_items = g_list_append (combo2_items, "Z");
  combo2_items = g_list_append (combo2_items, "XY");
  combo2_items = g_list_append (combo2_items, "XYZ");
  combo2_items = g_list_append (combo2_items, "X,Y");
  combo2_items = g_list_append (combo2_items, "XY,Z");
  combo2_items = g_list_append (combo2_items, "XZ,YZ");
  combo2_items = g_list_append (combo2_items, "X,Y,Z");
  gtk_combo_box_entry_set_popdown_strings ( (combo2), combo2_items);
  g_list_free (combo2_items);

  combo_entry2 = GTK_BIN (combo2)->child;
  gtk_widget_show (combo_entry2);


  label3 = gtk_label_new (_("         Options :    "));
  gtk_widget_show (label3);
  gtk_box_pack_start (GTK_BOX (hbox1), label3, FALSE, FALSE, 0);

  combo3 = gtk_combo_box_entry_new_text ();
  gtk_widget_set_size_request(GTK_WIDGET(combo3),100,-1);
  gtk_widget_show (combo3);
  gtk_box_pack_start (GTK_BOX (hbox1), combo3, FALSE, FALSE, 0);
  combo3_items = g_list_append (combo3_items, _("none"));
  combo3_items = g_list_append (combo3_items, "MASS");
  combo3_items = g_list_append (combo3_items, "CHARGE");
  combo3_items = g_list_append (combo3_items, "NOORIENT");
  combo3_items = g_list_append (combo3_items, "PLANEXZ");
  combo3_items = g_list_append (combo3_items, "PLANEYZ");
  combo3_items = g_list_append (combo3_items, "PLANEXY");
  gtk_combo_box_entry_set_popdown_strings ( (combo3), combo3_items);
  g_list_free (combo3_items);

  combo_entry3 = GTK_BIN (combo3)->child;
  gtk_widget_show (combo_entry3);

  hseparator2 = gtk_hseparator_new ();
  gtk_widget_show (hseparator2);
  gtk_box_pack_start (GTK_BOX (BoiteV), hseparator2, FALSE, FALSE, 0);

  hbox =create_hbox_false(BoiteV);
  FrameType = create_frame(window1,hbox,_("TYPE"));
  
  combobox = create_geom(window1,FrameType);

  geominter->vbox=BoiteV;
  geominter->frametitle=g_strdup(_("GEOMETRY"));

   geom->Symentry=combo_entry2;
   geom->Optentry=combo_entry3;

 if(GeomXYZ != NULL && MethodeGeom == GEOM_IS_XYZ)
     create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
 if(Geom != NULL && MethodeGeom == GEOM_IS_ZMAT)
     create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
 gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
}

