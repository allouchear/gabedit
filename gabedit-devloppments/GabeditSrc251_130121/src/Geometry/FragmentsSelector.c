/* FragmentsSelector.c */
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
#include <stdlib.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/PovrayUtils.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "../Common/Windows.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/PreviewGeom.h"
#include "../Geometry/FragmentsTree.h"

/********************************************************************************/
void hide_fragments_selector()
{
  	GtkWidget* FragmentsSelectorDlg = NULL;
	FragmentsSelectorDlg = g_object_get_data(G_OBJECT(GeomDlg), "FragmentsSelectorDlg");
	if( FragmentsSelectorDlg) gtk_widget_hide(FragmentsSelectorDlg);
}
/********************************************************************************/
void show_fragments_selector()
{
  	GtkWidget* FragmentsSelectorDlg = NULL;
	FragmentsSelectorDlg = g_object_get_data(G_OBJECT(GeomDlg), "FragmentsSelectorDlg");
	if( FragmentsSelectorDlg) gtk_widget_hide(FragmentsSelectorDlg);
}
/********************************************************************************/
void rafresh_fragments_selector()
{
  	GtkWidget* FragmentsSelectorDlg = NULL;
	GtkWidget* treeView = NULL;
	FragmentsSelectorDlg = g_object_get_data(G_OBJECT(GeomDlg), "FragmentsSelectorDlg");
	if( FragmentsSelectorDlg) treeView = g_object_get_data(G_OBJECT(FragmentsSelectorDlg), "TreeView");
	if( treeView) rafreshTreeView(treeView);
}
/********************************************************************************/
static void changed_entry_angle(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	gdouble a;


	t= gtk_entry_get_text(GTK_ENTRY(entry));
	a = atof(t);
	set_frag_angle(a);
}
/***************************************************************************/
static void help(GtkWidget* button, GtkWidget* FragmentsSelectorDlg)
{
	gchar* temp;
	GtkWidget* win;
	temp = g_strdup_printf(
		_(
		" * In the fragment selector :\n"
	        "       Clic to an atom (generally a H atom) connected to only one other atom. \n"
		"       This atom is colored in red.\n"
		"       The atom connected to red atom is colored in green.\n"
		"       A third atom is colored in blue.\n"
		"       To change the blue atom : \n"
		"                 select the red atom and move the mouse (button always pressed).\n"
		"                 The angle between red-green-blue atoms cannot be null\n"
	        "       Clic to red atom for unselect all.\n\n"
		" * In the Draw geometry window :\n"
	        "       Clic to an atom (generally a H atom) connected to only one other atom.\n"
		"       This atom is colored in red.\n"
		"       The atom connected to red atom is colored in green.\n"
		"       A third atom is colored in blue.\n"
		"       To change the blue atom : \n"
		"                 select the red atom and move the mouse (button always pressed).\n"
		"                 The angle between red-green-blue atoms cannot be null\n\n"
		"  After release of the left mouse button :\n"
	        "       The red atom of fragment is placed on the green atom of the old molecule\n"
	        "       the fragment is rotated so that the red, green atoms are aligned\n"
	        "       The red atoms are deleted\n"
	        "       The dihedral angle between the blue, green (of molecule), green and blue (of fragment) is set to the value given by user\n\n"
	        "       If all atoms of the fragment are unselected, then all these atoms are inserted in the molecule.\n"
		)
		 );
	win = Message(temp,_("Info"),FALSE);
	gtk_window_set_transient_for(GTK_WINDOW(win),GTK_WINDOW(FragmentsSelectorDlg));
	gtk_window_set_modal (GTK_WINDOW (win), TRUE);
	g_free(temp);
}
/********************************************************************************/
void create_window_fragments_selector(gchar* nodeNameToExpand, gchar* fragmentToSelect)
{
	GtkWidget *vboxframe;
	GtkWidget *frame;
	GtkWidget *hboxframe;
	GtkWidget *VboxWin;
  	GtkWidget* FragmentsSelectorDlg = NULL;
  	GtkWidget* drawingArea = NULL;
	GtkWidget* treeView = NULL;
	GtkWidget *table = gtk_table_new(2,4,FALSE);
	gint l, c, ncases;
	GtkWidget* entry = NULL;
	GtkWidget* button = NULL;
	gchar tmp[BSIZE];

	FragmentsSelectorDlg = g_object_get_data(G_OBJECT(GeomDlg), "FragmentsSelectorDlg");
	if(!FragmentsSelectorDlg || !GTK_IS_OBJECT(FragmentsSelectorDlg))
	{
		FragmentsSelectorDlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		g_object_set_data(G_OBJECT(GeomDlg), "FragmentsSelectorDlg", FragmentsSelectorDlg);
	}
	else
	{
		treeView = g_object_get_data(G_OBJECT(FragmentsSelectorDlg), "TreeView");
		if(fragmentToSelect) add_a_fragment(FragmentsSelectorDlg,fragmentToSelect);
		if(nodeNameToExpand) gtk_tree_selection_unselect_all (gtk_tree_view_get_selection (GTK_TREE_VIEW (treeView)));
		if(nodeNameToExpand) gtk_tree_view_collapse_all(GTK_TREE_VIEW (treeView));
		if(nodeNameToExpand) expandNodeStr(treeView, nodeNameToExpand);
		gtk_widget_show(FragmentsSelectorDlg);
		return;
	}

	VboxWin = gtk_vbox_new (TRUE, 0);
	gtk_container_add(GTK_CONTAINER(FragmentsSelectorDlg),VboxWin);
	gtk_widget_show(VboxWin);

	gtk_window_set_title(GTK_WINDOW(FragmentsSelectorDlg),_("Gabedit : Fragment Selector "));
	gtk_window_set_transient_for(GTK_WINDOW(FragmentsSelectorDlg),GTK_WINDOW(GeomDlg));
	gtk_widget_add_events(GeomDlg,GDK_KEY_RELEASE_MASK);
   

	/*gtk_window_move(GTK_WINDOW(GeomDlg),0,0);*/
	add_child(GeomDlg,FragmentsSelectorDlg, gtk_widget_destroy,_(" Frag. Sel. "));
	g_signal_connect (GTK_OBJECT (FragmentsSelectorDlg), "delete_event", G_CALLBACK (gtk_widget_hide), NULL);

	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
	gtk_box_pack_start(GTK_BOX(VboxWin), frame,TRUE,TRUE,1);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	gtk_widget_show (vboxframe);


	hboxframe = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vboxframe), hboxframe, TRUE, TRUE, 0);
	treeView = addFragmentsTreeView(hboxframe);
	gtk_widget_show_all (hboxframe);

	hboxframe = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vboxframe), hboxframe, TRUE, TRUE, 0);
	gtk_widget_show (hboxframe);

  	drawingArea = add_preview_geom(hboxframe);
	g_object_set_data(G_OBJECT(FragmentsSelectorDlg), "DrawingArea", drawingArea);
	g_object_set_data(G_OBJECT(FragmentsSelectorDlg), "TreeView", treeView);
	g_object_set_data(G_OBJECT(treeView), "DrawingArea", drawingArea);

	hboxframe = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vboxframe), hboxframe, FALSE, TRUE, 0);
	gtk_widget_show (hboxframe);
	gtk_box_pack_start (GTK_BOX (hboxframe), table, TRUE, TRUE, 0);

	/*------------------ Angle & Help -----------------------------------------*/
	l=0; 
	c = 0; 
	add_label_table(table,"Angle",l,c);
	c = 1; 
	add_label_table(table,":",l,c);
	entry = gtk_entry_new();
	c = 2; 
	ncases=1;
	sprintf(tmp,"%f",get_frag_angle());
	gtk_entry_set_text(GTK_ENTRY(entry),tmp);
	gtk_table_attach(GTK_TABLE(table),entry,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  1,1);
	c = 3;
	ncases = 1;
	button = gtk_button_new_from_stock(GTK_STOCK_HELP);
	gtk_table_attach(GTK_TABLE(table),button,c,c+ncases,l,l+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  1,1);
	gtk_widget_show_all (table);

    	gtk_widget_set_size_request(GTK_WIDGET(FragmentsSelectorDlg),(gint)(ScreenHeight*0.35),(gint)(ScreenHeight*0.85));
	

	gtk_widget_show (FragmentsSelectorDlg);
	/* fit_windows_position(GeomDlg,FragmentsSelectorDlg);*/
	if(fragmentToSelect) add_a_fragment(FragmentsSelectorDlg,fragmentToSelect);
	if(nodeNameToExpand) expandNodeStr(treeView, nodeNameToExpand);
	g_signal_connect (G_OBJECT (entry), "changed", (GCallback)changed_entry_angle, NULL);
	g_signal_connect(G_OBJECT(button), "clicked",(GCallback)help,FragmentsSelectorDlg);

}
