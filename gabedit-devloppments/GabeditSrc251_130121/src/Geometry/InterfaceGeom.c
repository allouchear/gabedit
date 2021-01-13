/* InterfaceGeom.c */
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
#include "../Geometry/InterfaceGeom.h"
#include "../Gaussian/GaussGlobal.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomXYZ.h"
#include "../Geometry/GeomZmatrix.h"
#include "../Geometry/ResultsAnalise.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Common/Windows.h"
#include "../Files/FileChooser.h"

static GtkWidget *FrameGeom;
/*************************************************************************/
void MessageGeom(gchar* message,gchar* type,gboolean center)
{
	GtkWidget* Dialogue = Message(message,type,center);
  	gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(Fenetre));
	if(WindowGeom)
  		gtk_window_set_transient_for(GTK_WINDOW(Dialogue),GTK_WINDOW(WindowGeom));
}
/*************************************************************************/
void read_mopac_input_file(GabeditFileChooser *gabeditFileChooser, gint response_id)
{
	gchar *fileName=NULL;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(gabeditFileChooser);
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		Message(_("Sorry\n No file selected"),_("Warning"),TRUE);
		return ;
	}
	read_geom_in_mopac_input(fileName);
}
/*************************************************************************/
void read_mpqc_input_file(GabeditFileChooser *gabeditFileChooser, gint response_id)
{
	gchar *fileName=NULL;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(gabeditFileChooser);
	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		Message(_("Sorry\n No file selected"),_("Warning"),TRUE);
		return ;
	}
	read_geom_in_mpqc_input(fileName);
}
/*************************************************************************/
void read_gauss_input_file(GabeditFileChooser *gabeditFileChooser, gint response_id)
{
	gchar *NomFichier=NULL;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(gabeditFileChooser);
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		Message(_("Sorry\n No file selected"),_("Warning"),TRUE);
		return ;
	}
	
	read_geom_in_gauss_input(NomFichier);
}
/*************************************************************************/
void read_molcas_input_file(GabeditFileChooser *gabeditFileChooser, gint response_id)
{
	gchar *NomFichier=NULL;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(gabeditFileChooser);
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		Message(_("Sorry\n No file selected"),_("Warning"),TRUE);
		return ;
	}
	read_geom_in_molcas_input(NomFichier);
}
/*************************************************************************/
void read_molpro_input_file(GabeditFileChooser *gabeditFileChooser, gint response_id)
{
	gchar *NomFichier=NULL;

	if(response_id != GTK_RESPONSE_OK) return;
	NomFichier = gabedit_file_chooser_get_current_file(gabeditFileChooser);
	if ((!NomFichier) || (strcmp(NomFichier,"") == 0))
	{
		Message(_("Sorry\n No file selected"),_("Warning"),TRUE);
		return ;
	}
	read_geom_in_molpro_input(NomFichier);
}
/*************************************************************************/
void selc_all_input_file(gchar *data)
{
	GtkWidget *gabeditFileChooser;
	gchar* patternsfiles[] = {"*.com","*.inp","*.mop","*",NULL};
	gchar* temp = NULL;


	gabeditFileChooser = gabedit_file_chooser_new(data, GTK_FILE_CHOOSER_ACTION_OPEN);
	gabedit_file_chooser_hide_hidden(GABEDIT_FILE_CHOOSER(gabeditFileChooser));

	if(WindowGeom) gtk_window_set_transient_for(GTK_WINDOW(gabeditFileChooser),GTK_WINDOW(WindowGeom));
	else gtk_window_set_transient_for(GTK_WINDOW(gabeditFileChooser),GTK_WINDOW(Fenetre));
	gtk_window_set_modal (GTK_WINDOW (gabeditFileChooser), TRUE);
	gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(gabeditFileChooser),patternsfiles);
	if( !strcmp(data,_("Read Geometry from a Mopac input file")) )
	{
		temp = g_strdup_printf("%s.mop",fileopen.projectname);
		gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(gabeditFileChooser),"*.mop");
	}
	else
		temp = g_strdup_printf("%s.com",fileopen.projectname);
	if(!temp) temp = g_strdup_printf("dump.com");
	if(lastdirectory)
	{
		gchar* t = g_strdup_printf("%s%s%s",lastdirectory,G_DIR_SEPARATOR_S,temp);
		gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),t);
		g_free(t);
		g_free(temp);
	}
	else
	{
		if(fileopen.localdir)
		{
			gchar* t = g_strdup_printf("%s%s%s",fileopen.localdir,G_DIR_SEPARATOR_S,temp);
			gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),t);
			g_free(t);
			g_free(temp);
		}
		else
		{
			gabedit_file_chooser_set_current_file(GABEDIT_FILE_CHOOSER(gabeditFileChooser),temp);
			g_free(temp);
		}
	}

	if( !strcmp(data,_("Read Geometry from a Gaussian input file")) )
  		g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (read_gauss_input_file), GTK_OBJECT(gabeditFileChooser));
	else
	if( !strcmp(data,_("Read Geometry from a Molcas input file")) )
		g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (read_molcas_input_file), GTK_OBJECT(gabeditFileChooser));
  	else
	if( !strcmp(data,_("Read Geometry from a Mopac input file")) )
		g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (read_mopac_input_file), GTK_OBJECT(gabeditFileChooser));
  	else
	if( !strcmp(data,_("Read Geometry from a MPQC input file")) )
		g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (read_mpqc_input_file), GTK_OBJECT(gabeditFileChooser));
	else
		g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (read_molpro_input_file), GTK_OBJECT(gabeditFileChooser));

	g_signal_connect (gabeditFileChooser, "response",  G_CALLBACK (gtk_widget_destroy), GTK_OBJECT(gabeditFileChooser));
	gtk_widget_show(gabeditFileChooser);
}
/*************************************************************************/
void create_geomXYZ_interface (GabEditTypeFileGeom readfile)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  /*GtkWidget *vseparator;*/
  
  MethodeGeom = GEOM_IS_XYZ;

  if( HboxGeom != NULL) 
      gtk_widget_destroy(GTK_WIDGET(HboxGeom) ) ;
  hbox =create_hbox_true(geominter->vbox);
  HboxGeom= hbox;
  FrameGeom = create_frame(geominter->window,hbox,geominter->frametitle);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (FrameGeom), vbox);

  hbox =create_hbox_true(vbox);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 5);
  gtk_widget_show (vbox);


  label = gtk_label_new (_(" XYZ Editor  "));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);

  create_geomXYZ_list(vbox,readfile);

  /* vseparator = create_vseparator (hbox);*/
  create_vseparator (hbox);
  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 5);

  label = gtk_label_new (_(" VARIABLES Editor "));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);
  create_variablesXYZ_list(vbox,readfile);

  gtk_widget_show_all (FrameGeom);
	
}
/*************************************************************************/
void create_geom_interface (GabEditTypeFileGeom readfile)
{
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *label;
  /* GtkWidget *vseparator;*/

  MethodeGeom = GEOM_IS_ZMAT;
  if( HboxGeom != NULL) 
      gtk_widget_destroy(GTK_WIDGET(HboxGeom) ) ;
  hbox =create_hbox_true(geominter->vbox);
  
  HboxGeom = hbox;
  FrameGeom = create_frame(geominter->window,hbox,geominter->frametitle);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_container_add (GTK_CONTAINER (FrameGeom), vbox);

  hbox =create_hbox_true(vbox);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 5);

  label = gtk_label_new (_(" Z-MATRIX Editor "));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);

  create_geom_list(vbox,readfile);

  /* vseparator = create_vseparator (hbox);*/
  create_vseparator (hbox);

  vbox = gtk_vbox_new (FALSE, 0);
  gtk_widget_show (vbox);
  gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 5);

  label = gtk_label_new (_(" VARIABLES Editor "));
  gtk_widget_show (label);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 2);

  create_variables_list(vbox,readfile);

  gtk_widget_show_all (FrameGeom);
}
/********************************************************************************************************/
static void traite_geom(GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	gchar* data = NULL;
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	if(!data) return;
	if (!strcmp((char*)data, _("New Z-Matrix geometry")))
	create_geom_interface (GABEDIT_TYPEFILEGEOM_NEW);
	else
	if (!strcmp((char*)data, _("Read Geometry from a Gaussian Z-Matrix file")))
	create_geom_interface (GABEDIT_TYPEFILEGEOM_GAUSSIAN_ZMATRIX);
	else
	if (!strcmp((char*)data, _("Read Geometry from a Mopac Z-Matrix file")))
	create_geom_interface (GABEDIT_TYPEFILEGEOM_MOPAC_ZMATRIX);
	else
	if (!strcmp((char*)data, _("New XYZ geometry")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_NEW);
	else
	if (!strcmp((char*)data, _("Read Geometry from a XYZ file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_XYZ);
	else
	if (!strcmp((char*)data, _("Read Geometry from a MOL2 file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_MOL2);
	else
	if (!strcmp((char*)data, _("Read Geometry from a Tinker file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_TINKER);
	else
	if (!strcmp((char*)data, _("Read Geometry from a PDB file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_PDB);
	else
	if (!strcmp((char*)data, _("Read Geometry from a Hyperchem file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_HIN);
	else
	if (!strcmp((char*)data, _("Read the first geometry from a Dalton output file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_DALTONFIRST);
	else
	if (!strcmp((char*)data, _("Read the last geometry from a Dalton output file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_DALTONLAST);
	else
	if (!strcmp((char*)data, _("Read the first geometry from a Molcas output file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_MOLCASOUTFIRST);
	else
	if (!strcmp((char*)data, _("Read the last geometry from a Molcas output file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_MOLCASOUTLAST);
	else
	if (!strcmp((char*)data, _("Read the first geometry from a Molpro output file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_MOLPROOUTFIRST);
	else
	if (!strcmp((char*)data, _("Read the last geometry from a Molpro output file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_MOLPROOUTLAST);
	else
	if (!strcmp((char*)data, _("Read the first geometry from a MPQC output file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_MPQCOUTFIRST);
	else
	if (!strcmp((char*)data, _("Read the last geometry from a MPQC output file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_MPQCOUTLAST);
	else
	if (!strcmp((char*)data, _("Read the first geometry from a Gaussian output file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_GAUSSOUTFIRST);
	else
	if (!strcmp((char*)data, _("Read the last geometry from a Gaussian output file")))
	create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_GAUSSOUTLAST);
	else
	if (!strcmp((char*)data, _("Read Geometry from a Gaussian input file")))
	{
		selc_all_input_file(data);
	}
	else
	if (!strcmp((char*)data, _("Read Geometry from a Molpro input file")))
	{
		selc_all_input_file(data);
	}
	else
	if (!strcmp((char*)data, _("Read Geometry from a MPQC input file")))
	{
		selc_all_input_file(data);
	}
	else
	if (!strcmp((char*)data, _("Read Geometry from a Molcas input file")))
	{
		selc_all_input_file(data);
	}
}
/********************************************************************************************************/
/* only with 2.6.x
static gboolean is_separator (GtkTreeModel *model, GtkTreeIter  *iter, gpointer  data)
{
	gchar* d = NULL;
	gtk_tree_model_get (model, iter, 0, &d, -1);

	return (d == NULL);
}
*/
/********************************************************************************************************/
GtkWidget *create_geom(GtkWidget*win,GtkWidget *frame)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
  	GtkWidget *hbox;
  	GtkWidget *vbox;
  	GtkWidget *hseparator;

	vbox =create_vbox(frame);
	VboxM[NM]=vbox;
	hbox =create_hbox_false(vbox);

	store = gtk_tree_store_new (1,G_TYPE_STRING);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Select a option"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("New Z-Matrix geometry"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("New XYZ geometry"), -1);

	/* separator */
	/* only with 2.6.x
        gtk_tree_store_append (store, &iter, NULL);
	gtk_tree_store_set (store, &iter, 0, NULL, -1);
	*/

        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read Geometry from a XYZ file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read Geometry from a MOL2 file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read Geometry from a Tinker file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read Geometry from a PDB file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read Geometry from a Hyperchem file"), -1);
        gtk_tree_store_append (store, &iter, NULL);

        gtk_tree_store_set (store, &iter, 0, _("Read Geometry from a Gaussian Z-Matrix file"), -1);
        gtk_tree_store_append (store, &iter, NULL);

	/* separator */
	/* only with 2.6.x
        gtk_tree_store_append (store, &iter, NULL);
	gtk_tree_store_set (store, &iter, 0, NULL, -1);
	*/

        gtk_tree_store_set (store, &iter, 0, _("Read the first geometry from a Dalton output file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read the last geometry from a Dalton output file"), -1);
        gtk_tree_store_append (store, &iter, NULL);

	/* separator */
	/* only with 2.6.x
        gtk_tree_store_append (store, &iter, NULL);
	gtk_tree_store_set (store, &iter, 0, NULL, -1);
	*/

        gtk_tree_store_set (store, &iter, 0, _("Read Geometry from a Gaussian input file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read the first geometry from a Gaussian output file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read the last geometry from a Gaussian output file"), -1);
        gtk_tree_store_append (store, &iter, NULL);

	/* separator */
	/* only with 2.6.x
        gtk_tree_store_append (store, &iter, NULL);
	gtk_tree_store_set (store, &iter, 0, NULL, -1);
	*/

        gtk_tree_store_set (store, &iter, 0, _("Read Geometry from a Molcas input file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read the first geometry from a Molcas output file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read the last geometry from a Molcas output file"), -1);
        gtk_tree_store_append (store, &iter, NULL);

	/* separator */
	/* only with 2.6.x
        gtk_tree_store_append (store, &iter, NULL);
	gtk_tree_store_set (store, &iter, 0, NULL, -1);
	*/

        gtk_tree_store_set (store, &iter, 0, _("Read Geometry from a Molpro input file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read the first geometry from a Molpro output file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read the last geometry from a Molpro output file"), -1);
        gtk_tree_store_append (store, &iter, NULL);

	/* separator */
	/* only with 2.6.x
        gtk_tree_store_append (store, &iter, NULL);
	gtk_tree_store_set (store, &iter, 0, NULL, -1);
	*/

        gtk_tree_store_set (store, &iter, 0, _("Read Geometry from a MPQC input file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read the first geometry from a MPQC output file"), -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, _("Read the last geometry from a MPQC output file"), -1);

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traite_geom), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);
	/* only with 2.6.x
	gtk_combo_box_set_row_separator_func (GTK_COMBO_BOX (combobox), is_separator, NULL, NULL);
	*/

  	gtk_box_pack_start (GTK_BOX (hbox), combobox, TRUE, TRUE, 10);

	create_units_option(hbox,_("  Units : "));

	hseparator = gtk_hseparator_new ();
	gtk_widget_show (hseparator);
	gtk_box_pack_start (GTK_BOX (vbox), hseparator, FALSE, FALSE, 1);

	return combobox;
}
/*************************************************************************/
void geom_is_open()
{
     GeomIsOpen = TRUE;
     TypeGeomOpen = 1;
}
/*************************************************************************/
void geom_is_not_open()
{
     GeomIsOpen = FALSE;
}
/*************************************************************************/
void destroy_geometry(GtkWidget *win)
{
     geom_is_not_open();
     TypeGeomOpen = 0;
     gtk_widget_destroy(win);
     WindowGeom = NULL;
}
/*************************************************************************/
void edit_geometry()
{
  GtkWidget *hbox;
  GtkWidget *BoiteV;
  GtkWidget *Frame;
  GtkWidget *FrameType;
  GtkWidget *button;
  GtkWidget *Window;
  GtkWidget *combobox;

  if( GeomIsOpen)
  {
     if(WindowGeom != NULL)
     {
        gtk_widget_hide(WindowGeom);
        gtk_widget_show(WindowGeom);
     }
     return;
  }
  geominter=g_malloc(sizeof(GeomInter));
  geominter->window= gtk_frame_new(NULL);
  HboxGeom = NULL;
  WindowGeom = NULL;

  geom_is_open();

  Window= gtk_dialog_new ();
  gtk_window_set_title(&GTK_DIALOG(Window)->window,_("Geometry Editor"));
  gtk_window_set_transient_for(GTK_WINDOW(Window),GTK_WINDOW(Fenetre));
  gtk_widget_realize (Window);
  init_child(Window,destroy_geometry,_(" Geometry Editor "));
  g_signal_connect(G_OBJECT(Window),"delete_event",(GCallback)destroy_children,NULL);

  Frame = geominter->window;
  gtk_container_set_border_width (GTK_CONTAINER (Frame), 10);
  gtk_container_add(GTK_CONTAINER(GTK_DIALOG(Window)->vbox), Frame);
  
  BoiteV = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(Frame), BoiteV);

  hbox =create_hbox_false(BoiteV);
  FrameType = create_frame(Frame,hbox,_("TYPE"));
  
  combobox = create_geom(FrameGeom,FrameType);
  geominter->vbox=BoiteV;
  geominter->frametitle=g_strdup(_("GEOMETRY"));

  gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(Window)->action_area), FALSE);
  gtk_widget_realize(Window);
  button = create_button(Window,_("Close"));
  gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Window)->action_area), button, FALSE, TRUE , 5);
  g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)destroy_children,GTK_OBJECT(Window));

 if(GeomXYZ != NULL && MethodeGeom == GEOM_IS_XYZ)
     create_geomXYZ_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
 if(Geom != NULL && MethodeGeom == GEOM_IS_ZMAT)
     create_geom_interface (GABEDIT_TYPEFILEGEOM_UNKNOWN);
  gtk_widget_show_all (Window);
  WindowGeom = Window;
  gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
}
/********************************************************************************************************/
static void traite_units_option(GtkComboBox *combobox, gpointer d)
{
	GtkTreeIter iter;
	guint OldUnits = Units;
	gchar* data = NULL;
	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		gtk_tree_model_get (model, &iter, 0, &data, -1);
	}
	if(!data) return;
	if (!strcmp((char*)data, _("Angstrom"))) Units = 1;
	else if (!strcmp((char*)data, _("Bohr"))) Units = 0;

	if(GeomDrawingArea != NULL && OldUnits != Units) rafresh_drawing();
}
/********************************************************************************************************/
void create_units_option(GtkWidget *hbox,gchar *tlabel)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	GtkWidget *label;

	label = gtk_label_new (tlabel);
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 2);
	gtk_widget_show (label);

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	if(Units == 1 )
	{
        	gtk_tree_store_append (store, &iter, NULL);
        	gtk_tree_store_set (store, &iter, 0, _("Angstrom"), -1);
        	gtk_tree_store_append (store, &iter, NULL);
        	gtk_tree_store_set (store, &iter, 0, _("Bohr"), -1);
	}
	else
	{
        	gtk_tree_store_append (store, &iter, NULL);
        	gtk_tree_store_set (store, &iter, 0, _("Bohr"), -1);
        	gtk_tree_store_append (store, &iter, NULL);
        	gtk_tree_store_set (store, &iter, 0, _("Angstrom"), -1);
	}


        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	g_signal_connect (G_OBJECT(combobox), "changed", G_CALLBACK(traite_units_option), NULL);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);
	gtk_widget_set_size_request(GTK_WIDGET(combobox), -1, 25 );
  	gtk_box_pack_start (GTK_BOX (hbox), combobox, TRUE, TRUE, 10);
        gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
}
/*************************************************************************/
static void read_file(GabeditFileChooser *selecFile, gint response_id)
{
	gchar *fileName;
	GabEditTypeFile fileType = GABEDIT_TYPEFILE_UNKNOWN;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(selecFile);
	gtk_widget_hide(GTK_WIDGET(selecFile));
	while( gtk_events_pending() ) gtk_main_iteration();

	fileType = get_type_file(fileName);
	if(fileType == GABEDIT_TYPEFILE_HIN) read_hin_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_XYZ) read_XYZ_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GABEDIT) read_gabedit_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MPQC) read_last_mpqc_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_DALTON) read_last_dalton_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GAUSSIAN) read_last_gaussian_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOLPRO) read_last_molpro_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GAMESS) read_last_gamess_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_FIREFLY) read_last_gamess_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOLCAS) read_last_molcas_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOLPRO) read_last_molpro_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_ORCA) read_last_orca_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_QCHEM) read_last_qchem_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_NWCHEM) read_last_nwchem_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOPAC) read_last_mopac_output_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOPAC_AUX) read_last_mopac_aux_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOPAC_SCAN) read_geometries_conv_mopac_scan(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOPAC_IRC) read_geometries_conv_mopac_irc(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOL2) read_mol2_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_TINKER) read_tinker_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_PDB) read_pdb_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GZMAT) read_ZMatrix_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MZMAT) read_ZMatrix_mopac_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GAUSSIANINPUT) read_gauss_input_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOLCASINPUT) read_molcas_input_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOLPROINPUT) read_molpro_input_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MOPACINPUT) read_mopac_input_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_MPQCINPUT) read_mpqc_input_file(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GAMESSIRC) read_geometries_irc_gamess(selecFile, response_id);
	else if(fileType == GABEDIT_TYPEFILE_GAUSSIAN_FCHK) read_fchk_gaussian_file(selecFile, response_id);
	else 
	{
		Message(
			_("Sorry, I cannot read this file\n")
			,_("Error"),TRUE);
	}
}
/********************************************************************************/
void read_geom_any_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_file, _("Read geometries"), GABEDIT_TYPEFILE_UNKNOWN,GABEDIT_TYPEWIN_ORB);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
