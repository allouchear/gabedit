/* Exit.c */
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


#include <stdlib.h>
#include "../../Config.h"
#include "../Common/Global.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Utils/HydrogenBond.h"
#include "../Files/ListeFiles.h"
#include "../MolecularMechanics/Atom.h"
#include "../MolecularMechanics/Molecule.h"
#include "../MolecularMechanics/ForceField.h"
#include "../MolecularMechanics/MolecularMechanics.h"
#include "../MolecularMechanics/PDBTemplate.h"
#include "../Geometry/PersonalFragments.h"
#include "../Utils/AtomsProp.h"
#include "../Molpro/MolproBasisLibrary.h"
#include "../Molcas/MolcasBasisLibrary.h"
#include "../MPQC/MPQCBasisLibrary.h"

void create_color_surfaces_file();
void create_opengl_file();
void save_axis_properties();
void save_principal_axis_properties();

typedef enum
{
	SAVELISTFILES = 0,
	SAVEHOSTSFILE,
	SAVEMMPARAMETERS,
	SAVEPDBTEMPLATE,
	SAVEFRAGMENT,
	SAVEATOMPROP,
	SAVEMOLPROBASIS,
	SAVEMOLCASBASIS,
	SAVEMPQCBASIS,
} SaveOptions;

#define NOPTIONS 9

static	GtkWidget* buttonOptions[NOPTIONS];

/********************************************************************************/
static void ExitGabedit(GtkWidget *widget, gchar *data)
{
	if(GTK_TOGGLE_BUTTON (buttonOptions[SAVELISTFILES])->active)
 		parse_liste_files();

	if(GTK_TOGGLE_BUTTON (buttonOptions[SAVEHOSTSFILE])->active)
        	create_hosts_file();

	if(GTK_TOGGLE_BUTTON (buttonOptions[SAVEMMPARAMETERS])->active)
		saveAmberParameters();

	if(GTK_TOGGLE_BUTTON (buttonOptions[SAVEPDBTEMPLATE])->active)
		savePersonalPDBTpl(NULL);

	if(GTK_TOGGLE_BUTTON (buttonOptions[SAVEFRAGMENT])->active)
		savePersonalFragments(NULL);

	if(GTK_TOGGLE_BUTTON (buttonOptions[SAVEATOMPROP])->active)
		save_atoms_prop();
	if(GTK_TOGGLE_BUTTON (buttonOptions[SAVEMOLPROBASIS])->active)
		saveMolproBasis();
	if(GTK_TOGGLE_BUTTON (buttonOptions[SAVEMOLCASBASIS])->active)
		saveMolcasBasis();
	if(GTK_TOGGLE_BUTTON (buttonOptions[SAVEMPQCBASIS])->active)
		saveMPQCBasis();

	create_commands_file();
	create_network_file();
	create_fonts_file();

	create_color_surfaces_file();
	create_opengl_file();
	create_drawmolecule_file();

	save_axis_properties();
#ifdef DRAWGEOMGL
	save_axes_geom_properties();
#endif
	save_principal_axis_properties();
	save_HBonds_properties();
	gtk_main_quit();
}
/***********************************************************************/
static void AddFrame(GtkWidget *box)
{

	gint i;
	GtkWidget *frame;
	GtkWidget *vbox;
	gchar *options[NOPTIONS]={
  		_("Save list of recent projects"),
  		_("Save list of recent host systems"),
  		_("Save the Molecular mechanics parameters"),
  		_("Save the PDB Templates parameters"),
  		_("Save the Personal Fragments"),
  		_("Save the atoms properties"),
  		_("Save the list of Molpro basis"),
  		_("Save the list of Molcas basis"),
  		_("Save the list of MPQC basis")
  		};

	frame = gtk_frame_new("Action");
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);
	gtk_container_add (GTK_CONTAINER (box), frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	for(i=0;i<NOPTIONS;i++)
	{
		buttonOptions[i] = create_checkbutton(frame,vbox,options[i]);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonOptions[i]), TRUE);
	}
}
/***********************************************************************/
void exit_gabedit(GtkWidget* Win, gpointer data)
{
	gtk_widget_destroy(GTK_WIDGET(Win));
}
/***********************************************************************/
void ExitDlg(GtkWidget* w, gpointer data)
{
	GtkWidget *button;
	GtkWidget *Win;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *parentWindow = Fenetre;

	Win= gtk_dialog_new ();
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_title(&GTK_DIALOG(Win)->window,"Exit");
    	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	gtk_button_box_set_layout (GTK_BUTTON_BOX (GTK_DIALOG(Win)->action_area), GTK_BUTTONBOX_END);
	gtk_box_set_homogeneous(GTK_BOX( GTK_DIALOG(Win)->action_area), TRUE);

	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)gtk_widget_destroy,NULL);
 
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_box_pack_start(GTK_BOX (GTK_DIALOG(Win)->vbox), vbox,TRUE, TRUE, 0);

	gtk_widget_realize(Win);
	label = create_label_with_pixmap(Win,_("Are you sure you want to exit?"),_("Question"));  
	gtk_box_pack_start (GTK_BOX(vbox), label, TRUE, TRUE, 5);

	AddFrame(vbox);


	button = create_button(Win,"No");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,"Yes");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)ExitGabedit,GTK_OBJECT(Win));
	gtk_widget_grab_default(button);
	gtk_widget_show (button);

  

	gtk_widget_show_all(Win);
  
}
/***********************************************************************/
