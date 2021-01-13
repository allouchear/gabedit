/* CaptureOrbitals.c */
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
#include "../Display/GlobalOrb.h"
#include "../Utils/Vector3d.h"
#include "../Display/GLArea.h"
#include "../Display/Orbitals.h"
#include "../Display/OrbitalsMolpro.h"
#include "../Display/OrbitalsGamess.h"
#include "../Display/OrbitalsQChem.h"
#include "../Display/GeomOrbXYZ.h"
#include "../Display/BondsOrb.h"
#include "../Display/UtilsOrb.h"
#include "../Display/TriangleDraw.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Files/FileChooser.h"
#include "../Common/Windows.h"
#include "../Display/Vibration.h"
#include "../Display/ContoursPov.h"
#include "../Display/PlanesMappedPov.h"
#include "../Display/LabelsGL.h"
#include "../Display/Images.h"

#define WIDTHSCR 0.3

/********************************************************************************/
static void resize_image(GtkWidget *widget, gint width, gint height)
{
	GdkPixbuf *pixbuf =	gtk_image_get_pixbuf(GTK_IMAGE(widget));
	if (pixbuf == NULL) return;
	pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, height, GDK_INTERP_BILINEAR);
	gtk_image_set_from_pixbuf(GTK_IMAGE(widget), pixbuf);
}
/********************************************************************************/
static void get_image_size(GtkWidget *widget, gint* width, gint* height)
{
		GdkPixbuf *pixbuf =	gtk_image_get_pixbuf(GTK_IMAGE(widget));
		*width = 0;
		*height = 0;
		if (pixbuf == NULL) return;
		*width = gdk_pixbuf_get_width (pixbuf);
		*height = gdk_pixbuf_get_height (pixbuf);
}
/********************************************************************************/
static void export_slides_file(GabeditFileChooser *SelecFile, gint response_id)
{       
 	gchar *fileName;
	GtkWidget *window = NULL;
	gchar* type = NULL;

	if(response_id != GTK_RESPONSE_OK) return;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry\n No selected file"),_("Error"),TRUE);
    		return ;
 	}
	window = g_object_get_data (G_OBJECT (SelecFile), "Window");
	type = g_object_get_data (G_OBJECT (SelecFile), "Type");

	if(!window) return;
	if(!type) return;

	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
	gabedit_save_image(window, fileName, type);
} 
/********************************************************************************/
static void export_png_file_dlg(GtkWidget *window, gpointer data)
{       
	static gchar* type ="png";
	GtkWidget* chooser = file_chooser_save(export_slides_file,"Capture slides in png file",GABEDIT_TYPEFILE_PNG,GABEDIT_TYPEWIN_ORB);
	g_object_set_data (G_OBJECT (chooser), "Window",window);
	g_object_set_data (G_OBJECT (chooser), "Type",type);
	gtk_window_set_modal (GTK_WINDOW (chooser), TRUE);
}
/********************************************************************************/
static void export_jpeg_file_dlg(GtkWidget *window, gpointer data)
{       
	static gchar* type ="jpeg";
	GtkWidget* chooser = file_chooser_save(export_slides_file,"capture slides in jpeg file",GABEDIT_TYPEFILE_JPEG,GABEDIT_TYPEWIN_ORB);
	g_object_set_data (G_OBJECT (chooser), "Window",window);
	g_object_set_data (G_OBJECT (chooser), "Type",type);
	gtk_window_set_modal (GTK_WINDOW (chooser), TRUE);
}
/********************************************************************************/
static void create_images_window (gint n, gint* numOrbs,gchar* prefix,gchar* title, gdouble scal, gint nCols)
{
	GtkWidget *window;
	GtkWidget *scrolled_window;
	GtkWidget *table;
	GtkWidget *hbox;
	GtkWidget *button;
	gint nRows = n;
	GtkWidget *vbox;
	gint i;
	gint j;
	gint k;
	gint iw=100;

	if(nCols<1) nCols = 1;
	nRows = n/nCols+1;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	vbox = create_vbox(window);
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);

	gtk_window_set_title (GTK_WINDOW (window), title);
	gtk_container_set_border_width (GTK_CONTAINER (window), 0);


	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 10);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
	gtk_widget_show (scrolled_window);

	table = gtk_table_new (nRows, nCols, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 1);
	gtk_table_set_col_spacings (GTK_TABLE (table), 1);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window), table);
	gtk_container_set_focus_hadjustment (GTK_CONTAINER (table),
					   gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (scrolled_window)));
	gtk_container_set_focus_vadjustment (GTK_CONTAINER (table),
					   gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (scrolled_window)));
	gtk_widget_show (table);

	k = 0;
	for (i = 0; i < nRows; i++)
	{
		for (j = 0; j < nCols; j++)
		{
		gchar* fileName = g_strdup_printf("%s_%d.png",prefix,numOrbs[k++]);
		GtkWidget* image = gtk_image_new_from_file (fileName);
		gint width;
		gint height;
		gint h = (gint)(ScreenHeightD*scal);
		gint w = h;
		get_image_size(image, &width, &height);
		w = (gint)(width*h*1.0/height);

		iw = w;

		if(image) gtk_table_attach(GTK_TABLE(table),image, j,j+1,i,i+1,
			              (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
			              (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
			              1,1);

		gtk_widget_show (image);

		if(image && height>h) resize_image(image,  w,  h);
		else iw = width;
		if(k>=n)break;
	 	}
		if(k>=n)break;
	}

	iw = (iw+15)*nCols;
	if(iw>(gint)(0.95*ScreenWidthD)) iw = (gint)(0.95*ScreenWidthD);
	gtk_window_set_default_size (GTK_WINDOW (window), iw, (gint)(ScreenHeightD*0.9));

	hbox = create_hbox_false(vbox);
	gtk_widget_realize(window);

	button = create_button(window,"Screen capture PNG");
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)export_png_file_dlg,G_OBJECT(table));

	button = create_button(window,"Screen capture JPEG");
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)export_jpeg_file_dlg,G_OBJECT(table));

	gtk_widget_show (window);
}

/********************************************************************************/
static gint* get_num_of_selected_orbitals(GtkWidget *gtklist, gint* n)
{
	gint* numOrbs = NULL;

	*n = 0;
	if (gtklist == NULL) return NULL; 
	if(!GTK_IS_TREE_VIEW(gtklist)) return NULL;
	{
		GtkTreeSelection *selection;
 		GtkTreeModel *model;
		GList *selected_rows = NULL;
		GList *row;
		GtkTreePath *path = NULL;
		gint* indices = NULL;
		gint i = 0;

		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW(gtklist));
		if(selection) selected_rows = gtk_tree_selection_get_selected_rows (selection, &model);
		*n = gtk_tree_selection_count_selected_rows(selection);
		if(*n<1) return numOrbs;
		numOrbs = g_malloc(*n*sizeof(gint));

		i =0;
		for (row = g_list_first (selected_rows); row != NULL; row = g_list_next (row))
		{
				path = (GtkTreePath *)(row->data);
				indices = gtk_tree_path_get_indices(path); 
				numOrbs[i++] = indices[0];
				if(i>=*n) break;
		}
	}
	return numOrbs;
}
/********************************************************************************/
static void apply_capture_orbitals(GtkWidget *Win,gpointer data)
{
	GtkWidget** entriestmp = NULL;
	G_CONST_RETURN gchar* temp;
	gchar* dump;
	gint i;
	gint j;
	GridLimits limitstmp;
	gint NumPointstmp[3];
	GtkWidget *entries[3][6];
	gdouble V[3][3];
	GtkWidget* buttonDirSelector = g_object_get_data (G_OBJECT (Win), "ButtonDirSelector");
	gchar* dirName = NULL;
	GtkWidget* entryIsoValue = g_object_get_data (G_OBJECT (Win), "EntryIsoValue");
	gdouble isovalue = 0.1;
	GtkWidget* alphaList = g_object_get_data (G_OBJECT (Win), "AlphaList");
	GtkWidget* betaList = g_object_get_data (G_OBJECT (Win), "BetaList");
	gint* numAlphaOrbs = NULL;
	gint* numBetaOrbs = NULL;
	gint nAlpha = 0;
	gint nBeta = 0;
	gint iAlpha = -1;
	gint iBeta = -1;
	GtkWidget* numberButton = g_object_get_data (G_OBJECT (Win), "NumberButton");
	GtkWidget* energyButton = g_object_get_data (G_OBJECT (Win), "EnergyButton");
	GtkWidget* symmetryButton = g_object_get_data (G_OBJECT (Win), "SymmetryButton");
	GtkWidget* occButton = g_object_get_data (G_OBJECT (Win), "OccButton");
	GtkWidget* homoLumoButton = g_object_get_data (G_OBJECT (Win), "HomoLumoButton");
	GtkWidget* entryPrefix=g_object_get_data (G_OBJECT (Win), "EntryPrefix");
	GtkWidget* columnSpinButton = g_object_get_data (G_OBJECT (Win), "ColumnSpinButton");
	GtkWidget* scaleSpinButton = g_object_get_data (G_OBJECT (Win), "ScaleSpinButton");
	gboolean showNumberButton = FALSE;
	gboolean showEnergyButton = FALSE;
	gboolean showSymmetryButton = FALSE;
	gboolean showOccButton = FALSE;
	gboolean showHomoLumoButton = FALSE;
	gchar* prefix = NULL;
	gdouble scale = 0.2;
	gint nCols = 3;

	if(GTK_IS_WIDGET(Win))
	{
		entriestmp = (GtkWidget **)g_object_get_data(G_OBJECT (Win), "Entries");
	}
	else return;

	if(entriestmp==NULL) return;

	for(i=0;i<3;i++)
	for(j=0;j<6;j++)
		entries[i][j] = entriestmp[i*6+j];
	
	for(i=0;i<3;i++)
	{
		for(j=3;j<5;j++)
		{
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				limitstmp.MinMax[j-3][i] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : one entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
        	temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][5])); 
		NumPointstmp[i] = atoi(temp);
		if(NumPointstmp[i] <=2)
		{
			GtkWidget* message = Message(_("Error : The number of points should be > 2. "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		
	}

	for(i=0;i<3;i++)
	{
		if( limitstmp.MinMax[0][i]> limitstmp.MinMax[1][i])
		{
			GtkWidget* message = Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
	}
	for(i=0;i<3;i++)
	{
		for(j=0;j<3;j++)
		{
			V[i][j] = 0;
        		temp	= gtk_entry_get_text(GTK_ENTRY(entries[i][j])); 
			dump = NULL;
			if(temp && strlen(temp)>0)
			{
				dump = g_strdup(temp);
				delete_first_spaces(dump);
				delete_last_spaces(dump);
			}

			if(dump && strlen(dump)>0 && this_is_a_real(dump))
			{
				V[i][j] = atof(dump);
			}
			else
			{
				GtkWidget* message = Message(_("Error : one entry is not a float "),_("Error"),TRUE);
  				gtk_window_set_modal (GTK_WINDOW (message), TRUE);
				return;
			}
			if(dump) g_free(dump);
		}
	}
        
	for(i=0;i<3;i++)
	{
		gdouble norm = 0.0;
		for(j=0;j<3;j++)
			norm += V[i][j]*V[i][j];
		if(fabs(norm)<1e-8)
		{
			GtkWidget* message = Message(_("Error : the norm is equal to 0 "),_("Error"),TRUE);
  			gtk_window_set_modal (GTK_WINDOW (message), TRUE);
			return;
		}
		for(j=0;j<3;j++)
			V[i][j] /= sqrt(norm);
	}
	for(j=0;j<3;j++) originOfCube[j] = 0;
	for(j=0;j<3;j++) firstDirection[j] = V[0][j];
	for(j=0;j<3;j++) secondDirection[j] = V[1][j];
	for(j=0;j<3;j++) thirdDirection[j] = V[2][j];

	for(i=0;i<3;i++)
	{
		NumPoints[i] =NumPointstmp[i] ; 
		for(j=0;j<2;j++)
			limits.MinMax[j][i] =limitstmp.MinMax[j][i]; 
	}
	if(buttonDirSelector)
	{
		dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
		if(!dirName) dirName = g_strdup(g_get_current_dir());
		if(dirName && dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
		{
			gchar* tmp = dirName;
			dirName = g_strdup_printf("%s%s",tmp, G_DIR_SEPARATOR_S);
			g_free(tmp);
		}
	}
	if(entryIsoValue)
	{
        	temp = gtk_entry_get_text(GTK_ENTRY(entryIsoValue));
		if(temp) isovalue = atof(temp);
	}


	/* printf("DirName = %s\n",dirName);*/
	numAlphaOrbs = get_num_of_selected_orbitals(alphaList, &nAlpha);
	numBetaOrbs = get_num_of_selected_orbitals(betaList, &nBeta);
	/*
	printf("Selected alpha orbitals : ");
	for(i=0;i<nAlpha;i++)
		printf("%d ",numAlphaOrbs[i]);
	printf("\n");
	printf("Selected beta orbitals : ");
	for(i=0;i<nBeta;i++)
		printf("%d ",numBetaOrbs[i]);
	printf("\n");
	*/
	if(GTK_IS_WIDGET(numberButton)) showNumberButton = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(numberButton));
	if(GTK_IS_WIDGET(energyButton)) showEnergyButton = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(energyButton));
	if(GTK_IS_WIDGET(symmetryButton)) showSymmetryButton = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(symmetryButton));
	if(GTK_IS_WIDGET(occButton)) showOccButton = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(occButton));
	if(GTK_IS_WIDGET(homoLumoButton)) showHomoLumoButton = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(homoLumoButton));
	if(entryPrefix) prefix	= g_strdup(gtk_entry_get_text(GTK_ENTRY(entryPrefix))); 
	scale = gtk_spin_button_get_value (GTK_SPIN_BUTTON(scaleSpinButton));
	nCols = (gint)gtk_spin_button_get_value (GTK_SPIN_BUTTON(columnSpinButton));

	delete_child(Win);

	TypeSelOrb = 1;
	TypeGrid = GABEDIT_TYPEGRID_ORBITAL;
	for(i=0;i<nAlpha;i++)
	{
		gchar* fileName = NULL;
		gint ii = numAlphaOrbs[i];
		gchar* pLabel = NULL;
		gchar* nLabel = NULL;
		gchar* eLabel = NULL;
		gchar* sLabel = NULL;
		gchar* oLabel = NULL;
		gchar* hLabel = NULL;
		gchar* title = NULL;
		 if(CancelCalcul) break;
		fileName = g_strdup_printf("%sgabOrbAlpha_%d.png",dirName,numAlphaOrbs[i]);
		if(prefix && strlen(prefix)>0) pLabel = g_strdup_printf("%s ",prefix);
		else pLabel = g_strdup(" ");
		if(showNumberButton) nLabel = g_strdup_printf(" n=%d ",ii+1);
		else nLabel =  g_strdup(" ");
		if(showEnergyButton) eLabel = g_strdup_printf(" E=%0.6e ",EnerAlphaOrbitals[ii]);
		else eLabel =  g_strdup(" ");
		if(showOccButton) oLabel = g_strdup_printf(" Occ=%0.3f ",OccAlphaOrbitals[ii]);
		else oLabel =  g_strdup(" ");
		if(showSymmetryButton) sLabel = g_strdup_printf(" Sym=%s ",SymAlphaOrbitals[ii]);
		else sLabel =  g_strdup(" ");
		if(showHomoLumoButton) 
		{
			gint nh = ii+1-NAlphaOcc;
			if(nh==0) hLabel = g_strdup_printf("Homo");
			else if(nh<0) hLabel = g_strdup_printf("Homo%d",nh);
			else 
			{
				if(nh==1) hLabel = g_strdup_printf("Lumo");
				else hLabel = g_strdup_printf("Lumo+%d",nh-1);
			}

		}
		else hLabel =  g_strdup(" ");

		title = g_strdup_printf("%s%s%s%s%s%s",pLabel,hLabel,nLabel,eLabel,oLabel,sLabel);

		free_surfaces_all();
		NumSelOrb = numAlphaOrbs[i];
		Define_Grid();
		Define_Iso(fabs(isovalue));
		set_label_title(title,0,0);
		glarea_rafresh(GLArea);
		while( gtk_events_pending() ) gtk_main_iteration();
		gabedit_save_image_gl(GLArea, fileName, "png",NULL);
		g_free(fileName);
		g_free(title);
		g_free(pLabel);
		g_free(nLabel);
		g_free(eLabel);
		g_free(sLabel);
		g_free(oLabel);
		g_free(hLabel);
	}
	iAlpha = i;
	
	if(iAlpha != nAlpha) nBeta = 0;
	TypeSelOrb = 2;
	for(i=0;i<nBeta;i++)
	{
		gchar* fileName = NULL;
		gint ii = numBetaOrbs[i];
		gchar* pLabel = NULL;
		gchar* nLabel = NULL;
		gchar* eLabel = NULL;
		gchar* sLabel = NULL;
		gchar* oLabel = NULL;
		gchar* hLabel = NULL;
		gchar* title = NULL;
		 if(CancelCalcul) break;
		fileName = g_strdup_printf("%sgabOrbBeta_%d.png",dirName,numBetaOrbs[i]);
		if(prefix && strlen(prefix)>0) pLabel = g_strdup_printf("%s ",prefix);
		else pLabel = g_strdup(" ");
		if(showNumberButton) nLabel = g_strdup_printf(" n=%d ",ii+1);
		else nLabel =  g_strdup(" ");
		if(showEnergyButton) eLabel = g_strdup_printf(" E=%0.6e ",EnerBetaOrbitals[ii]);
		else eLabel =  g_strdup(" ");
                if(showOccButton) oLabel = g_strdup_printf(" Occ=%0.3f ",OccBetaOrbitals[ii]);
                else oLabel =  g_strdup(" ");
		if(showSymmetryButton) sLabel = g_strdup_printf(" Sym=%s ",SymBetaOrbitals[ii]);
		else sLabel =  g_strdup(" ");
		if(showHomoLumoButton) 
		{
			gint nh = ii+1-NAlphaOcc;
			if(nh==0) hLabel = g_strdup_printf("Homo");
			else if(nh<0) hLabel = g_strdup_printf("Homo%d",nh);
			else 
			{
				if(nh==1) hLabel = g_strdup_printf("Lumo");
				else hLabel = g_strdup_printf("Lumo+%d",nh-1);
			}

		}
		else hLabel =  g_strdup(" ");

		title = g_strdup_printf("%s%s%s%s%s%s",pLabel,hLabel,nLabel,eLabel,oLabel,sLabel);

		free_surfaces_all();
		NumSelOrb = numBetaOrbs[i];
		Define_Grid();
		Define_Iso(fabs(isovalue));
		set_label_title(title,0,0);
		glarea_rafresh(GLArea);
		while( gtk_events_pending() ) gtk_main_iteration();
		gabedit_save_image_gl(GLArea, fileName, "png",NULL);
		g_free(fileName);
		g_free(title);
		g_free(pLabel);
		g_free(nLabel);
		g_free(eLabel);
		g_free(sLabel);
		g_free(oLabel);
		g_free(hLabel);
	}
	iBeta = i;
	if(nAlpha>0 && iAlpha==nAlpha)
	{
		gchar* prefix =  g_strdup_printf("%sgabOrbAlpha",dirName);
		create_images_window (nAlpha, numAlphaOrbs,prefix,_("Alpha orbitals"),scale,nCols);
		g_free(prefix);
	}
	if(nBeta>0 && iBeta==nBeta)
	{
		gchar* prefix =  g_strdup_printf("%sgabOrbBeta",dirName);
		create_images_window (nBeta, numBetaOrbs,prefix,_("Beta Orbitals"),scale,nCols);
		g_free(prefix);
	}
	set_label_title(NULL,0,0);
	if(numAlphaOrbs) g_free(numAlphaOrbs);
	if(numBetaOrbs) g_free(numBetaOrbs);
	if(dirName) g_free(dirName);
	if(prefix) g_free(prefix);
	if(CancelCalcul) CancelCalcul = FALSE;
}
/********************************************************************************/
static GtkWidget *create_folder_frame( GtkWidget *vboxall,gchar* title)
{
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gushort i;
	gushort j;
	GtkWidget *table;
	GtkWidget *buttonDirSelector;
	GtkWidget *label;

	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(1,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	label = gtk_label_new(_("Folder"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 2;
	buttonDirSelector =  gabedit_dir_button();
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_widget_show_all(frame);
	g_object_set_data (G_OBJECT (frame), "ButtonDirSelector",buttonDirSelector);

  	return frame;
}
/********************************************************************************/
static GtkWidget *create_isovalue_frame( GtkWidget *vboxall,gchar* title)
{
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gushort i;
	gushort j;
	GtkWidget *table;
	GtkWidget *entryIsoValue;
	GtkWidget *label;

	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(1,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	label = gtk_label_new(_("Isovalue"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 2;
	entryIsoValue =  gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryIsoValue),"0.1");
	gtk_table_attach(GTK_TABLE(table),entryIsoValue,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_widget_show_all(frame);
	g_object_set_data (G_OBJECT (frame), "EntryIsoValue",entryIsoValue);

  	return frame;
}
/********************************************************************************/
static GtkWidget *create_labels_frame( GtkWidget *vboxall,gchar* title)
{
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gushort i;
	gushort j;
	GtkWidget *table;
	GtkWidget *numberButton;
	GtkWidget *energyButton;
	GtkWidget *symmetryButton;
	GtkWidget *occButton;
	GtkWidget *homoLumoButton;
	GtkWidget *label;
	GtkWidget *entryPrefix;

	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(1,8,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	numberButton = gtk_check_button_new_with_label (_("Number"));
	gtk_table_attach(GTK_TABLE(table),numberButton, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 1;
	energyButton = gtk_check_button_new_with_label (_("Energy"));
	gtk_table_attach(GTK_TABLE(table),energyButton, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 2;
	symmetryButton = gtk_check_button_new_with_label (_("Symbol"));
	gtk_table_attach(GTK_TABLE(table),symmetryButton, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 3;
	occButton  = gtk_check_button_new_with_label (_("Occ. #"));
	gtk_table_attach(GTK_TABLE(table),occButton, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 4;
	homoLumoButton = gtk_check_button_new_with_label (_("Homo/Lumo number"));
	gtk_table_attach(GTK_TABLE(table),homoLumoButton, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 5;
	label = gtk_label_new(_("prefix"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 6;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 7;
	entryPrefix =  gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryPrefix),"");
	gtk_table_attach(GTK_TABLE(table),entryPrefix,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);

	gtk_widget_show_all(frame);
	g_object_set_data (G_OBJECT (frame), "EntryPrefix",entryPrefix);
	g_object_set_data (G_OBJECT (frame), "NumberButton",numberButton);
	g_object_set_data (G_OBJECT (frame), "EnergyButton",energyButton);
	g_object_set_data (G_OBJECT (frame), "SymmetryButton",symmetryButton);
	g_object_set_data (G_OBJECT (frame), "OccButton",occButton);
	g_object_set_data (G_OBJECT (frame), "HomoLumoButton",homoLumoButton);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(numberButton), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(energyButton), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(symmetryButton), FALSE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(homoLumoButton), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(occButton), FALSE);

  	return frame;
}
/********************************************************************************/
static GtkWidget *create_slides_frame( GtkWidget *vboxall,gchar* title)
{
	GtkWidget *frame;
	GtkWidget *vboxframe;
	gushort i;
	gushort j;
	GtkWidget *table;
	GtkWidget *scaleSpinButton;
	GtkWidget *columnSpinButton;
	GtkWidget *label;

	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe = create_vbox(frame);
	table = gtk_table_new(1,6,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),table);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	label = gtk_label_new(_("Number of slides by row"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 2;
	columnSpinButton =  gtk_spin_button_new_with_range (1, 100, 1);
	gtk_table_attach(GTK_TABLE(table),columnSpinButton,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (columnSpinButton), 3);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 3;
	label = gtk_label_new(_("Slide size/Screen size"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  4,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 4;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 5;
	scaleSpinButton =  gtk_spin_button_new_with_range (0.1, 1.0, 0.1);
	gtk_table_attach(GTK_TABLE(table),scaleSpinButton,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (scaleSpinButton), 0.25);


	gtk_widget_show_all(frame);
	g_object_set_data (G_OBJECT (frame), "ColumnSpinButton",columnSpinButton);
	g_object_set_data (G_OBJECT (frame), "ScaleSpinButton",scaleSpinButton);

  	return frame;
}
/********************************************************************************/
static GtkWidget* new_gtk_list_orbitals(gint N,gdouble* Energies,gdouble* Occ,gchar** sym, gint* widall)
{
	gint i;
	gint j;
	GtkWidget* gtklist = NULL;
	gint *Width = NULL;
	gint NlistTitle = 4;
	gchar* Titles[] = {"Nr","Energy","Occ.","Sym."};
	gchar* List[4];
	GtkListStore *store;
	GtkTreeModel *model;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;
	GtkTreeIter iter;
	GType* types;
  
	Width = g_malloc(NlistTitle*sizeof(gint));

	for (j=0;j<NlistTitle;j++) Width[j] = strlen(Titles[j]);

	types = g_malloc(NlistTitle*sizeof(GType));
	for (i=0;i<NlistTitle;i++) types[i] = G_TYPE_STRING;
  	store = gtk_list_store_newv (NlistTitle, types);
	g_free(types);
	model = GTK_TREE_MODEL (store);


	Width[0] = (gint)(Width[0]*10);
	Width[1] = (gint)(Width[1]*12);
	Width[2] = (gint)(Width[2]*8);
	Width[3] = (gint)(Width[3]*14);

	*widall = 0;
	for (j=0;j<NlistTitle;j++) *widall += Width[j];
	*widall += 60;

	gtklist = gtk_tree_view_new_with_model (model);
	gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (gtklist), TRUE);
	gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (gtklist), TRUE);
	gtk_tree_view_set_reorderable(GTK_TREE_VIEW (gtklist), FALSE);

	for (i=0;i<NlistTitle;i++)
	{
		column = gtk_tree_view_column_new ();
		gtk_tree_view_column_set_title (column, Titles[i]);
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_column_pack_start (column, renderer, TRUE);
		gtk_tree_view_column_set_min_width(column, Width[i]);
		gtk_tree_view_column_set_attributes (column, renderer, "text", i, NULL);
		gtk_tree_view_append_column (GTK_TREE_VIEW (gtklist), column);
	}
  	g_free( Width);
  
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtklist));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_MULTIPLE);

	for(i=0;i<N;i++)
	{
		if(strcmp(sym[i],"DELETED")==0)continue;
		List[0] = g_strdup_printf("%i",i+1);
		List[1] = g_strdup_printf("%lf",Energies[i]);
		List[2] = g_strdup_printf("%lf",Occ[i]);
		List[3] = g_strdup(sym[i]);

		gtk_list_store_append(store, &iter);
		for(j=0;j<4;j++) gtk_list_store_set (store, &iter, j, List[j], -1);

		for(j=0;j<4;j++) g_free(List[j]);
	}
	return gtklist;
 
}
/********************************************************************************/
static GtkWidget* new_alpha_list(GtkWidget *hboxall)
{
	GtkWidget *frame;
	GtkWidget *scr;
	GtkWidget *vbox;
	GtkWidget *gtklist;
	gint i;
	gint N;
	gdouble* Energies;
	gdouble* Occ;
	gchar** sym;
	static gint type = 1;
	gint widall = 0;

	N = NAlphaOrb;
	Energies = g_malloc(N*sizeof(gdouble));
	Occ = g_malloc(N*sizeof(gdouble));
	sym = g_malloc(N*sizeof(gchar*));

	for(i=0;i<N;i++)
	{
		Energies[i] = EnerAlphaOrbitals[i];
		Occ[i] = OccAlphaOrbitals[i];
		sym[i] = g_strdup(SymAlphaOrbitals[i]);
	}

	gtklist = new_gtk_list_orbitals(N,Energies,Occ,sym,&widall);
	g_object_set_data(G_OBJECT (gtklist), "Type",&type);
  	frame = gtk_frame_new (_("Alpha Orbitals"));
  	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_box_pack_start (GTK_BOX (hboxall), frame, TRUE, TRUE, 0);
  	gtk_widget_show (frame);
  	vbox = create_vbox(frame);
  	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeightD*WIDTHSCR));
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 1);
  	gtk_container_add(GTK_CONTAINER(scr),gtklist);

	set_base_style(gtklist,55000,55000,55000);


	for(i=0;i<N;i++) g_free(sym[i]);
	g_free(Energies);
	g_free(Occ);
	g_free(sym);

	g_object_set_data(G_OBJECT (hboxall), "AlphaList",gtklist);

	return frame;

}
/********************************************************************************/
static GtkWidget* new_beta_list(GtkWidget *hboxall)
{
	GtkWidget *frame;
	GtkWidget *scr;
	GtkWidget *vbox;
	GtkWidget *gtklist;
	gint i;
	gint N;
	gdouble* Energies;
	gdouble* Occ;
	gchar** sym;
	static gint type = 2;
	gint widall = 0;

	N = NBetaOrb;
	Energies = g_malloc(N*sizeof(gdouble));
	Occ = g_malloc(N*sizeof(gdouble));
	sym = g_malloc(N*sizeof(gchar*));

	for(i=0;i<N;i++)
	{
		Energies[i] = EnerBetaOrbitals[i];
		Occ[i] = OccBetaOrbitals[i];
		sym[i] = g_strdup(SymBetaOrbitals[i]);
	}

	gtklist = new_gtk_list_orbitals(N,Energies,Occ,sym,&widall);
	g_object_set_data(G_OBJECT (gtklist), "Type",&type);
  	frame = gtk_frame_new (_("Beta Orbitals"));
  	gtk_container_set_border_width (GTK_CONTAINER (frame), 1);
  	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_box_pack_start (GTK_BOX (hboxall), frame, TRUE, TRUE, 0);     
  	gtk_widget_show (frame);
  	vbox = create_vbox(frame);
  	scr=gtk_scrolled_window_new(NULL,NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scr),GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_size_request(scr,widall,(gint)(ScreenHeightD*WIDTHSCR));
  	gtk_box_pack_start(GTK_BOX (vbox), scr,TRUE, TRUE, 1);
  	gtk_container_add(GTK_CONTAINER(scr),gtklist);
	set_base_style(gtklist,55000,55000,55000);
  	gtk_widget_show (scr);
  	gtk_widget_show (gtklist);

	for(i=0;i<N;i++) g_free(sym[i]);
	g_free(Energies);
	g_free(Occ);
	g_free(sym);
	g_object_set_data(G_OBJECT (hboxall), "BetaList",gtklist);
	return frame;
}
/********************************************************************************/
static GtkWidget *create_orbitals_list( GtkWidget *vboxall)
{
	GtkWidget *hbox;
	hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vboxall), hbox, TRUE, TRUE, 0); 
	new_alpha_list(hbox);
	new_beta_list(hbox);
	return hbox;
}
/********************************************************************************/
void capture_orbitals_dlg()
{
	GtkWidget *Win;
	GtkWidget *frame;
	GtkWidget *frameFolder;
	GtkWidget *buttonDirSelector;
	GtkWidget *frameIsoValue;
	GtkWidget *entryIsoValue;
	GtkWidget *alphaList;
	GtkWidget *betaList;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget** entries;
	GtkWidget *numberButton;
	GtkWidget *energyButton;
	GtkWidget *symmetryButton;
	GtkWidget *occButton;
	GtkWidget *homoLumoButton;
	GtkWidget *frameLabels;
	GtkWidget *entryPrefix;
	GtkWidget *frameSlides;
	GtkWidget *scaleSpinButton;
	GtkWidget *columnSpinButton;

	if(!GeomOrb)
	{
		Message(_("Sorry, Please load a file beforee\n"),_("Error"),TRUE);
		return;
	}
	if(!CoefAlphaOrbitals)
	{
		Message(_("Sorry, Please load the MO beforee\n"),_("Error"),TRUE);
		return;
	}

	if(!AOAvailable &&(TypeGrid == GABEDIT_TYPEGRID_DDENSITY || TypeGrid == GABEDIT_TYPEGRID_ADENSITY))
	{
		Message(_("Sorry, No atomic orbitals available.\nPlease use a gabedit file for load : \n"
		  "Geometry, Molecular and Atomic Orbitals\n"),_("Error"),TRUE);
		return;
	}
	
	Win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(Win),_("Slideshow orbital"));
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (Win), 5);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(PrincipalWindow));
	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	add_glarea_child(Win,"Grid ");

	vboxall = create_vbox(Win);
	vboxwin = vboxall;
	frame = create_grid_frame(vboxall,_("Box & Grid"));
	entries = (GtkWidget**) g_object_get_data (G_OBJECT (frame), "Entries");
	g_object_set_data (G_OBJECT (Win), "Entries",entries);

	frameFolder = create_folder_frame(vboxall,NULL);
	buttonDirSelector = g_object_get_data (G_OBJECT (frameFolder), "ButtonDirSelector");
	g_object_set_data (G_OBJECT (Win), "ButtonDirSelector",buttonDirSelector);

	frameIsoValue = create_isovalue_frame(vboxall,NULL);
	entryIsoValue = g_object_get_data (G_OBJECT (frameIsoValue), "EntryIsoValue");
	g_object_set_data (G_OBJECT (Win), "EntryIsoValue",entryIsoValue);


	frameLabels = create_labels_frame(vboxall,_("Labels"));
	numberButton = g_object_get_data (G_OBJECT (frameLabels), "NumberButton");
	energyButton = g_object_get_data (G_OBJECT (frameLabels), "EnergyButton");
	symmetryButton = g_object_get_data (G_OBJECT (frameLabels), "SymmetryButton");
	occButton = g_object_get_data (G_OBJECT (frameLabels), "OccButton");
	homoLumoButton = g_object_get_data (G_OBJECT (frameLabels), "HomoLumoButton");
	entryPrefix=g_object_get_data (G_OBJECT (frameLabels), "EntryPrefix");
	g_object_set_data (G_OBJECT (Win), "NumberButton",numberButton);
	g_object_set_data (G_OBJECT (Win), "EnergyButton",energyButton);
	g_object_set_data (G_OBJECT (Win), "SymmetryButton",symmetryButton);
	g_object_set_data (G_OBJECT (Win), "OccButton",occButton);
	g_object_set_data (G_OBJECT (Win), "EntryPrefix",entryPrefix);
	g_object_set_data (G_OBJECT (Win), "HomoLumoButton",homoLumoButton);

	frameSlides = create_slides_frame(vboxall,_("Slides"));
	columnSpinButton = g_object_get_data (G_OBJECT (frameSlides), "ColumnSpinButton");
	scaleSpinButton = g_object_get_data (G_OBJECT (frameSlides), "ScaleSpinButton");
	g_object_set_data (G_OBJECT (Win), "ColumnSpinButton",columnSpinButton);
	g_object_set_data (G_OBJECT (Win), "ScaleSpinButton",scaleSpinButton);

	hbox = gtk_hbox_new (TRUE, 0);
	gtk_box_pack_start (GTK_BOX (vboxall), hbox, TRUE, TRUE, 0); 
	label = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(label), "<span foreground=\"#FF0000\"><big>Use mouse + the Ctrl key (or the shift key) to select several orbitals</big></span>\n");
	gtk_box_pack_start (GTK_BOX (hbox), label, TRUE, TRUE, 0); 

	hbox = create_orbitals_list(vboxall);
	alphaList = g_object_get_data (G_OBJECT (hbox), "AlphaList");
	g_object_set_data (G_OBJECT (Win), "AlphaList",alphaList);
	betaList = g_object_get_data (G_OBJECT (hbox), "BetaList");
	g_object_set_data (G_OBJECT (Win), "BetaList",betaList);
   

	hbox = create_hbox_false(vboxwin);
	gtk_widget_realize(Win);

	button = create_button(Win,"OK");
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_capture_orbitals,G_OBJECT(Win));

	button = create_button(Win,"Cancel");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_end (GTK_BOX( hbox), button, FALSE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)delete_child, G_OBJECT(Win));
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,G_OBJECT(Win));
	gtk_widget_show (button);

  

	gtk_widget_show_all (Win);
}
