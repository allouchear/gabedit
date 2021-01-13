/* ViewOrb.c */
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
#include "GlobalOrb.h"
#include "UtilsOrb.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "GLArea.h"
#include "StatusOrb.h"
#include "ColorMap.h"

/*************************************************************************/
int view_orb(GtkWidget* Parent,int argc, char **argv)
{
	GtkWidget* vboxwin;
	GtkWidget* hboxwin;
	GtkWidget* handleBoxColorMapGrid;
	GtkWidget* handleBoxColorMapContours;
	GtkWidget* handleBoxColorMapPlanesMapped;

	static gboolean first = TRUE;


	init_dipole();
	if(!first)
	{
		if(PrincipalWindow)
		{
			gtk_widget_hide(GTK_WIDGET(PrincipalWindow));
			gtk_widget_show(GTK_WIDGET(PrincipalWindow));
			/* if(argc>1) read_any_file(argv[1]);*/
			return 0;
		}
	}
	/* initialisation */
	initialise_global_orbitals_variables();
	read_opengl_file();

	/* Create new top level window. */
	PrincipalWindow = gtk_window_new( GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(PrincipalWindow), _("Gabedit : Orbitals/Density/Vibration"));
	 gtk_container_set_reallocate_redraws (GTK_CONTAINER (PrincipalWindow), TRUE);
  	gtk_window_set_default_size (GTK_WINDOW(PrincipalWindow),(gint)(ScreenWidthD*0.5),(gint)(ScreenHeightD*0.69));
	gtk_container_set_border_width(GTK_CONTAINER(PrincipalWindow), 1);
	init_child(PrincipalWindow,gtk_widget_destroy," Draw Dens. Orb. ");
	/* g_signal_connect(G_OBJECT(PrincipalWindow),"delete_event",(GCallback)close_window_orb,NULL);*/
	g_signal_connect(G_OBJECT(PrincipalWindow), "delete-event",G_CALLBACK(gtk_widget_hide_on_delete), NULL);
	gtk_window_set_transient_for(GTK_WINDOW(PrincipalWindow),GTK_WINDOW(Parent));

	vboxwin = create_vbox(PrincipalWindow);
	gtk_widget_realize(PrincipalWindow);
	hboxwin = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX(vboxwin), hboxwin, TRUE, TRUE, 2);
	gtk_widget_show (hboxwin);

	
	if (!NewGLArea(hboxwin)) return 0;

	create_status_bar_orb(vboxwin);
	create_status_progress_bar_orb(vboxwin);

	handleBoxColorMapGrid = create_color_map_show(vboxwin,NULL," Grid ");
	g_object_set_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapGrid ", handleBoxColorMapGrid);

	handleBoxColorMapContours = create_color_map_show(vboxwin,NULL, "Contours ");
	g_object_set_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapContours", handleBoxColorMapContours);

	handleBoxColorMapPlanesMapped = create_color_map_show(vboxwin,NULL," Planes colorcoded");
	g_object_set_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapPlanesMapped", handleBoxColorMapPlanesMapped);

	set_icone(PrincipalWindow);
	gtk_widget_show(GTK_WIDGET(PrincipalWindow));
	color_map_hide(handleBoxColorMapGrid);
	color_map_hide(handleBoxColorMapContours);
	color_map_hide(handleBoxColorMapPlanesMapped);
	/* if(argc>1) read_any_file(argv[1]);*/

	first = FALSE;

	gtk_window_move(GTK_WINDOW(PrincipalWindow),0,0);
	InitializeAll();
	/*printCoefZlm();*/

	return 0;
}
