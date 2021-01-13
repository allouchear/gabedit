/* UVSpectrum.c */
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
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtk.h>
#include "../Common/Global.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Files/FileChooser.h"
#include "../Common/Windows.h"
#include "../Utils/GabeditXYPlot.h"
#include "../Display/Vibration.h"
#include "SpectrumWin.h"

/************************************************************************/
static void add_oncurve_to_new_win(GtkWidget *newxyplot, XYPlotData* data, gdouble* pxmax, gdouble* pymax)
{
	gdouble *X = NULL;
	gdouble *Y = NULL;
	gint loop;
	gint i;
	X = g_malloc(data->size*sizeof(gdouble));
	Y = g_malloc(data->size*sizeof(gdouble));
	for (loop=0; loop<data->size; loop++) if(fabs(data->y[loop]) > *pymax ) *pymax = fabs(data->y[loop]);
	i=0;
	for (loop=0; loop<data->size; loop++)
	{
		if(fabs(data->x[loop])>1e-10)
		{
			X[i]= 1239.8424121/data->x[loop];
			Y[i]= data->y[loop];
			if(X[i]>*pxmax && fabs(Y[i])>*pymax*1e-2) *pxmax = X[i];
			i++;
		}
	}
       	gabedit_xyplot_add_data_conv(GABEDIT_XYPLOT(newxyplot),i, X,  Y, 1.0, GABEDIT_XYPLOT_CONV_NONE,NULL);
	if(X) g_free(X);
	if(Y) g_free(Y);
}
/************************************************************************/
static void create_uv_nm_spectrum(GtkWidget *oldWin,gpointer d)
{
	GtkWidget *newWin;
        GtkWidget* oldxyplot = g_object_get_data(G_OBJECT (oldWin), "XYPLOT");
        GtkWidget* newxyplot = NULL;
        GList* data_list = GABEDIT_XYPLOT(oldxyplot)->data_list;
	gdouble xmax = 0;
	gdouble ymax = -100;

        GList* current = NULL;
        XYPlotWinData* dataW = NULL;
        XYPlotData* data = NULL;
        if(!data_list) return;
        current=g_list_first(data_list);

	newWin = gabedit_xyplot_new_window("UV",NULL);
        newxyplot = g_object_get_data(G_OBJECT (newWin), "XYPLOT");
        for(; current != NULL; current = current->next)
        {
               	data = (XYPlotData*)current->data;
		if(!data) continue;
		if(data->size<1) continue;
		add_oncurve_to_new_win(newxyplot, data, &xmax, &ymax);
	}
	gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(newxyplot), NULL);
        gabedit_xyplot_set_range_xmin (GABEDIT_XYPLOT(newxyplot), 0.0);
        gabedit_xyplot_set_range_xmax (GABEDIT_XYPLOT(newxyplot), xmax);
	gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(newxyplot), "Wavelength(nm)");
	gabedit_xyplot_set_y_label (GABEDIT_XYPLOT(newxyplot), "Intensity");
	/* fprintf(stderr,"xmax=%f\n",xmax);*/

}
/********************************************************************************/
static void check_nm_ev_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	GtkWidget* xyplot = NULL;
	GList* data_list = NULL;
	GList* current = NULL;
	XYPlotWinData* data;
	gboolean nm = FALSE;
	GtkWidget* window = NULL;
	gdouble xmax = 0;
	gdouble ymax = 0;

	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");

	if(!data_list) return;
	window = g_object_get_data(G_OBJECT (xyplot), "Window");

	nm = gtk_toggle_button_get_active(togglebutton);
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		gint loop;
		data = (XYPlotWinData*)current->data;
		for (loop=0; loop<data->size; loop++)
		{
			if(fabs(data->x[loop])>1e-10)
			{
				if(nm) data->x[loop]= 1239.8424121/data->x[loop];
				else data->x[loop]= 1239.8424121/data->x[loop];
			}
			else
				data->x[loop]= 1e-10;
			if(xmax<data->x[loop]) xmax = data->x[loop];
			if(ymax<data->y[loop]) ymax = data->y[loop];
		}
	}
	xmax = xmax*1.5;
	ymax = ymax*2.0;
	spectrum_win_set_xmin(window, 0.0);
	spectrum_win_set_xmax(window, xmax);
	spectrum_win_set_ymax(window, ymax);
	spectrum_win_set_half_width(window, fabs(xmax/50));
	spectrum_win_reset_ymin_ymax(GTK_WIDGET(xyplot));
	if(nm)
	{
		spectrum_win_set_xlabel(window, "nm");
	}
	else
	{
		spectrum_win_set_xlabel(window, "eV");
	}
	{
		GtkWidget* nmButton = g_object_get_data(G_OBJECT (togglebutton), "nmButton");
		if(nm) gtk_widget_set_sensitive(nmButton,FALSE);
		else gtk_widget_set_sensitive(nmButton,TRUE);
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));

}
/********************************************************************************/
static void createUVSpectrumWin(gint numberOfStates, gdouble* energies, gdouble* intensities)
{
	GtkWidget* window = spectrum_win_new_with_xy(_("UV/Visible spectrum"),  numberOfStates, energies, intensities);
	GtkWidget* hbox = g_object_get_data(G_OBJECT (window), "HBoxData");
	GtkWidget* hbox2 = g_object_get_data(G_OBJECT (window), "HBoxData2");
	GtkWidget* xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
	GtkWidget* check_nm_ev = NULL;
	GtkWidget* tmp_hbox = NULL;
	GtkWidget* button = NULL;

	spectrum_win_set_half_width(window, 0.05);
	spectrum_win_set_xmin(window, 0.0);
	set_icone(window);

	if(!hbox) return;

	tmp_hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), tmp_hbox, FALSE, FALSE, 30);
	gtk_widget_show(tmp_hbox);

	check_nm_ev = gtk_check_button_new_with_label("nm<=>eV");
	gtk_box_pack_start(GTK_BOX(tmp_hbox), check_nm_ev, FALSE, FALSE, 4);
	gtk_widget_show(check_nm_ev);

	g_signal_connect(G_OBJECT(check_nm_ev), "toggled", G_CALLBACK(check_nm_ev_toggled), xyplot);
	spectrum_win_set_xlabel(window, "eV");
	spectrum_win_set_ylabel(window, _("Intensity"));

	button = create_button(window,_("nm"));
        gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
        GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
        gtk_widget_grab_default(button);
        gtk_widget_show (button);
        g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)create_uv_nm_spectrum,GTK_OBJECT(window));

	g_object_set_data(G_OBJECT (check_nm_ev), "nmButton",button);
	g_object_set_data(G_OBJECT (button), "Window",window);

}
/********************************************************************************/
static void messageErrorFreq(gchar* fileName)
{
	gchar buffer[BSIZE];
	sprintf(buffer,_("Sorry, I can not read energies from '%s' file\n"),fileName);
  	Message(buffer,_("Error"),TRUE);
}
/********************************************************************************/
/*
static void messageErrorInt(gchar* fileName)
{
	gchar buffer[BSIZE];
	sprintf(buffer,"Sorry, I can not read energies from '%s' file\n",fileName);
  	Message(buffer,"Error",TRUE);
}
*/
/********************************************************************************/
static gboolean read_gabedit_molden_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;
 	gchar t[BSIZE];
 	gboolean OK;
 	FILE *fd;
	gint ne;
	gint numberOfStates = 0;
	gdouble* energies = NULL;
	gdouble* intensities = NULL;
	gdouble a;
	gdouble b;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
 	OK=FALSE;

  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"[UV"))
		{
			OK = TRUE;
			break;
		}
	}
	if(!OK)
	{
		messageErrorFreq(FileName);
		if(fd) fclose(fd);
		return FALSE;
	}
  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"[")) break;
		if(this_is_a_backspace(t)) break;
		ne = sscanf(t,"%lf %lf",&a, &b);
		if(ne != 2)
		{
			messageErrorFreq(FileName);
			if(energies) g_free(energies);
			if(intensities) g_free(intensities);
			fclose(fd);
			return FALSE;
		}
		numberOfStates++;
		energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
		intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
		energies[numberOfStates-1] = a;
		intensities[numberOfStates-1] = b;
	}
	if(numberOfStates>0)
	{
		createUVSpectrumWin(numberOfStates, energies, intensities);
	}
	else
	{
		messageErrorFreq(FileName);
	}

	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
	fclose(fd);
	return TRUE;

}
/********************************************************************************/
static void read_gabedit_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gabedit_molden_file,
			_("Read energies and intensities from a Gabedit file"),
			GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_molden_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gabedit_molden_file,
			_("Read energies and intensities from a Molden file"),
			GABEDIT_TYPEFILE_MOLDEN,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_mpqc_file(GabeditFileChooser *SelecFile, gint response_id)
{
	return FALSE;
}
/********************************************************************************/
static void read_mpqc_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_mpqc_file,
			_("Read energies and intensities from a MPQC output file"),
			GABEDIT_TYPEFILE_MPQC,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_molpro_file(GabeditFileChooser *SelecFile, gint response_id)
{
	return FALSE;
}
/********************************************************************************/
static void read_molpro_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_molpro_file,
			_("Read energies and intensities from a Molpro output file"),
			GABEDIT_TYPEFILE_MOLPRO,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gint read_dalton_modes_MOLHES(FILE* fd, gchar *FileName)
{
	return 1;
}
/********************************************************************************/
static gboolean read_dalton_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;
 	FILE *fd;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	read_dalton_modes_MOLHES(fd, FileName);
	fclose(fd);
	return TRUE;
}
/********************************************************************************/
static void read_dalton_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_dalton_file,
			_("Read energies and intensities from a Dalton output file"),
			GABEDIT_TYPEFILE_DALTON,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_gamess_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar t[BSIZE];
 	gboolean OK;
	gint numberOfStates = 0;
	gdouble* energies = NULL;
	gdouble* intensities = NULL;
	gchar *FileName;
	gchar** allreals = NULL;
 	FILE *fd;
	gint k = 0;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,BSIZE,fd);}
	 		if ( strstr( t,"EXCITATION") && strstr( t,"STATE") && strstr( t,"OSCILLATOR") )
	  		{
    				{ char* e = fgets(t,BSIZE,fd);} /* second row of title */
    				{ char* e = fgets(t,BSIZE,fd);} /* ground state */
				OK = TRUE;
				numberOfStates = 0;
				break;
	  		}
		}
  		while(!feof(fd) )
  		{
    			{ char* e = fgets(t,BSIZE,fd);}
			if(this_is_a_backspace(t)) break;
 			allreals =gab_split (t);
			k = 0;
			/* printf("t=%s\n",t);*/
			while(allreals && allreals[k])
			{
				/*printf("k=%d %s\n",k,allreals[k]);*/
				k++;
			}
			/* printf("k=%d\n",k);*/
			if(k==8)/* Second column = symmetry */
			{
				numberOfStates++;
				energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
				intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
				energies[numberOfStates-1] = atof(allreals[3]);
				intensities[numberOfStates-1] = atof(allreals[7]);
			}
			if(k==7)/* Second column = energy in au */
			{
				numberOfStates++;
				energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
				intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
				energies[numberOfStates-1] = atof(allreals[2]);
				intensities[numberOfStates-1] = atof(allreals[6]);
			}
			g_strfreev(allreals);
			allreals = NULL;
		}
 	}while(!feof(fd));

	if(numberOfStates>0)
	{
		createUVSpectrumWin(numberOfStates, energies, intensities);
	}
	else
	{
		messageErrorFreq(FileName);
	}


	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
	fclose(fd);
	return TRUE;
	return FALSE;
}
/********************************************************************************/
static void read_gamess_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gamess_file,
			_("Read energies and intensities from a Gamess output file"),
			GABEDIT_TYPEFILE_GAMESS,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_firefly_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar t[BSIZE];
 	gboolean OK;
	gint numberOfStates = 0;
	gdouble* energies = NULL;
	gdouble* intensities = NULL;
	gchar *FileName;
	gchar** allreals = NULL;
 	FILE *fd;
	gint k = 0;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,BSIZE,fd);}
	 		if ( strstr( t," EXCITATION ENERGIES") )
	  		{
    				{ char* e = fgets(t,BSIZE,fd);}
    				{ char* e = fgets(t,BSIZE,fd);}
				OK = TRUE;
				numberOfStates = 0;
				break;
	  		}
		}
  		while(!feof(fd) )
  		{
    			{ char* e = fgets(t,BSIZE,fd);}
			if(this_is_a_backspace(t)) break;
 			allreals =gab_split (t);
			k = 0;
			while(allreals && allreals[k] && k<7) k++;
			if(k==7)
			{
				numberOfStates++;
				energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
				intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
				energies[numberOfStates-1] = atof(allreals[2]);
				intensities[numberOfStates-1] = atof(allreals[6]);
			}
			g_strfreev(allreals);
			allreals = NULL;
		}
 	}while(!feof(fd));

	if(numberOfStates>0)
	{
		createUVSpectrumWin(numberOfStates, energies, intensities);
	}
	else
	{
		messageErrorFreq(FileName);
	}


	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
	fclose(fd);
	return TRUE;
	return FALSE;
}
/********************************************************************************/
static void read_firefly_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_firefly_file,
			_("Read energies and intensities from a FireFly output file"),
			GABEDIT_TYPEFILE_FIREFLY,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_gaussian_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar t[BSIZE];
 	gboolean OK;
	gint numberOfStates = 0;
	gdouble* energies = NULL;
	gdouble* intensities = NULL;
	gchar *FileName;
 	FILE *fd;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,BSIZE,fd);}
	 		/* if ( strstr( t,"reduced masses") )*/
	 		if ( strstr( t,"Excitation energies and oscillator strengths:") )
	  		{
				OK = TRUE;
				numberOfStates = 0;
				break;
	  		}
		}
  		while(!feof(fd) )
  		{
    			{ char* e = fgets(t,BSIZE,fd);}
			if(strstr(t,"Excited State")&& strstr(t,"eV"))
			{
				char* egal = strstr(t,"eV")-10;
				numberOfStates++;
				energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
				intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
			        energies[numberOfStates-1] = atof(egal);
				egal = strstr(t,"f=")+2;
				intensities[numberOfStates-1] = atof(egal);
			}
		}
 	}while(!feof(fd));

	if(numberOfStates>0)
	{
		createUVSpectrumWin(numberOfStates, energies, intensities);
	}
	else
	{
		messageErrorFreq(FileName);
	}


	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
	fclose(fd);
	return TRUE;
}
/********************************************************************************/
static void read_gaussian_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gaussian_file,
			_("Read energies and intensities from a Gaussian output file"),
			GABEDIT_TYPEFILE_GAUSSIAN,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_qchem_file_str(gchar *fileName, gchar* strType)
{
 	gchar t[BSIZE];
 	gboolean OK;
	gint numberOfStates = 0;
	gdouble* energies = NULL;
	gdouble* intensities = NULL;
 	FILE *fd;

	if(!fileName) return FALSE;

 	fd = FOpen(fileName, "rb");
	if(!fd) return FALSE;

 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,BSIZE,fd);}
	 		if ( strstr( t,strType))
	  		{
				OK = TRUE;
				numberOfStates = 1;
				energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
				intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
				energies[numberOfStates-1] = -1.0;
				intensities[numberOfStates-1] = -1.0;
				break;
	  		}
		}
		if(!OK) break;

  		while(!feof(fd) )
  		{
    			if(!fgets(t,BSIZE,fd)) break;
	 		if ( strstr( t,"Excitation Energies") ) break;
			if(strstr(t,"Excited state")&& strstr(t,"excitation energy (eV)"))
			{
				gchar* egal = strstr(t,"=")+1;
				energies[numberOfStates-1] = atof(egal);
			}
			if(strstr(t,"Strength   :"))
			{
				gchar* egal = strstr(t,":")+1;
				intensities[numberOfStates-1] = atof(egal);
				numberOfStates++;
				energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
				intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
			}
		}
 	}while(!feof(fd));

	if(numberOfStates>1 && intensities[numberOfStates-1]>=0)
	{
		numberOfStates--;
		energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
		intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
		createUVSpectrumWin(numberOfStates, energies, intensities);
	}
	else
	{
		messageErrorFreq(fileName);
	}


	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
	fclose(fd);
	return TRUE;
}
/********************************************************************************/
static gboolean apply_qchem(GtkWidget *button,gpointer data)
{
	GtkComboBox *combo = (GtkComboBox *)data;
	GtkWidget *window = g_object_get_data (G_OBJECT (button), "Window");
	GtkTreeIter iter;
	gchar* type = NULL;
	gboolean res = FALSE;

	if (gtk_combo_box_get_active_iter (combo, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combo);
		gchar* fileName = g_object_get_data(G_OBJECT (combo), "FileName");
		gtk_tree_model_get (model, &iter, 0, &type, -1);
		if(!type) return FALSE;
		if(!fileName) return FALSE;
		res = read_qchem_file_str(fileName, type);
	}
	gtk_widget_destroy(window);
	return res;
}
/********************************************************************************************************/
static GtkWidget *add_qchem_types_combo(GtkWidget *hbox, gint nTypes, gchar** listTypes)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i;

	
	store = gtk_tree_store_new (1,G_TYPE_STRING);
	for(i=0;i<nTypes;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
        	gtk_tree_store_set (store, &iter, 0, listTypes[i], -1);
	}

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	gtk_box_pack_start (GTK_BOX (hbox), combobox, TRUE, TRUE, 1);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	return combobox;
}
/********************************************************************************/
static gboolean read_qchem_file_multiple(gchar* fileName, gint nTypes, gchar** listTypes)
{

	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* frame = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* vbox_window = NULL;
	GtkWidget* vbox = NULL;
	GtkWidget* label = NULL;
	GtkWidget* combo = NULL;
	GtkWidget* button = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Select the type of calculations"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), NULL);

	vbox_window=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox_window);
	gtk_widget_show(vbox_window);


	frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox_window), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 2);
	gtk_widget_show(hbox);

	label=gtk_label_new(
			_(
			"More types of excitation energies has been detected\n"
			"Please select one"
			)
			);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_CENTER);
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 2);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Type :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	combo = add_qchem_types_combo(hbox, nTypes, listTypes);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), nTypes-1);
	gtk_widget_show(combo); 
	g_object_set_data(G_OBJECT (window), "ComboType", combo);
	g_object_set_data(G_OBJECT (combo), "FileName", fileName);
	gtk_widget_realize(window);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_window), hbox, TRUE, FALSE, 2);
	gtk_widget_show(hbox);

	button = create_button(window,_("Cancel"));
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 5);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(window));
	gtk_widget_show_all (button);

	button = create_button(window,_("OK"));
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 5);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show_all (button);
	g_object_set_data(G_OBJECT (button), "Window", window);
	g_signal_connect(G_OBJECT(button), "clicked",G_CALLBACK(apply_qchem),(gpointer)combo);

	gtk_widget_show(window); 
	return TRUE;

}
/********************************************************************************/
static gboolean read_qchem_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;
	gchar** listTypes = NULL;
	gint nTypes = 0;
	FILE* fd;
 	gchar t[BSIZE];
	gint i;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);


 	fd = FOpen(fileName, "rb");
	if(!fd) return FALSE;
	while(!feof(fd))
	{
  		if(!fgets(t,BSIZE,fd))
 		if ( strstr( t,"Excitation Energies"))
  		{
			nTypes++;
			listTypes = g_realloc(listTypes, nTypes*sizeof(gchar*));
			for(i=0;i<strlen(t);i++)
				if(t[i]=='\n') t[i] = ' ';
			g_strstrip(t);
			listTypes[nTypes-1] = g_strdup(t);
  		}
	}
	fclose(fd);
	if(nTypes == 0) 
	{
		messageErrorFreq(fileName);
		return FALSE;
	}
	if(nTypes == 1) 
	{
		gboolean res = read_qchem_file_str(fileName, listTypes[0]);
		if(listTypes[0]) g_free(listTypes[0]);
		if(listTypes) g_free(listTypes);
		return res;
	}
	else
	{
		gboolean res = read_qchem_file_multiple(fileName, nTypes, listTypes);
		for(i=0;i<nTypes;i++)
			if(listTypes[i]) g_free(listTypes[i]);
		if(listTypes) g_free(listTypes);
		return res;
	}
	return TRUE;
}
/********************************************************************************/
static void read_qchem_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_qchem_file,
			_("Read energies and intensities from a Q-Chem output file"),
			GABEDIT_TYPEFILE_MOLPRO,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_orca_file(GabeditFileChooser *SelecFile, gint response_id)
{

	gchar *FileName;
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint n;
	gdouble ener = 0;
	gdouble itensity = 0;
	gint numberOfStates = 0;
	gdouble *energies = NULL;
	gdouble *intensities = NULL;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (strstr( t,"ABSORPTION SPECTRUM VIA TRANSITION ELECTRIC DIPOLE MOMENTS") ) OK = TRUE;
	 		if (strstr( t,"TX")  && strstr( t,"TY") && strstr( t,"TZ") && OK ){ OK = TRUE; break;}
		}
		if(!OK) break;
		numberOfStates = 0;
		if(energies) g_free(energies);
		if(intensities) g_free(intensities);
		energies = NULL;
		intensities = NULL;
    		{ char* e = fgets(t,taille,fd);}
    		{ char* e = fgets(t,taille,fd);}
  		while(!feof(fd) )
  		{
			if(!fgets(t,taille,fd)) break;
			if(atoi(t)<=0) break;
			n = sscanf(t,"%s %lf %s %lf", sdum1, &ener,sdum1, &itensity);
			if(n==4)
			{
				numberOfStates++;
				energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
				intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
				ener /=8065.54458086;
				energies[numberOfStates-1] = ener;
				intensities[numberOfStates-1] = itensity;
			}
		}
 	}while(!feof(fd));

	if(numberOfStates>0)
	{
		createUVSpectrumWin(numberOfStates, energies, intensities);
	}
	else
	{
		messageErrorFreq(FileName);
	}


	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
	fclose(fd);

	return TRUE;
}
/********************************************************************************/
static void read_orca_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_orca_file,
			_("Read energies and intensities from a Orca output file"),
			GABEDIT_TYPEFILE_ORCA,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_nwchem_tddft_file(GabeditFileChooser *SelecFile, gint response_id)
{

	gchar *FileName;
 	gchar t[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint n;
	gdouble ener = 0;
	gdouble intensity = 0;
	gint numberOfStates = 0;
	gdouble *energies = NULL;
	gdouble *intensities = NULL;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (strstr( t,"NWChem TDDFT Module") ) {OK = TRUE; break;}
		}
		if(!OK) break;
		numberOfStates = 0;
		if(energies) g_free(energies);
		if(intensities) g_free(intensities);
		energies = NULL;
		intensities = NULL;
  		while(!feof(fd) )
  		{
			if(!fgets(t,taille,fd)) break;
	 		if (!(strstr( t,"Root") && strstr( t,"eV") && strstr( t,"(") && strstr( t,")")) ) continue;
			n = sscanf( strstr( t,"(")+1,"%lf",&ener);
			if(n==1)
			{
				 while(!feof(fd) )
				{
					if(!fgets(t,taille,fd)) break;
					if(strstr(t,"Dipole Oscillator Strength"))
					{
						intensity=0.0;
						 if(strstr(t,"Spin forbidden") || 1==sscanf( strstr(t,"Dipole Oscillator Strength")+strlen("Dipole Oscillator Strength"),"%lf",&intensity))
						{
							numberOfStates++;
							energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
							intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
							energies[numberOfStates-1] = ener;
							intensities[numberOfStates-1] = intensity;
						}
						break;
						
					}
				}
			}
		}
 	}while(!feof(fd));

	if(numberOfStates>0)
	{
		OK = TRUE;
		createUVSpectrumWin(numberOfStates, energies, intensities);
	}
	else
	{
		OK = FALSE;
	}


	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
	fclose(fd);

	return OK;
}
/********************************************************************************/
static gboolean read_nwchem_eom_file(GabeditFileChooser *SelecFile, gint response_id)
{

	gchar *FileName;
 	gchar t[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint n;
	gdouble ener = 0;
	gdouble intensity = 0;
	gint numberOfStates = 0;
	gdouble *energies = NULL;
	gdouble *intensities = NULL;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (strstr( t,"Dim. of EOMCC iter. space") ) {OK = TRUE; break;}
		}
		if(!OK) break;
		numberOfStates = 0;
		if(energies) g_free(energies);
		if(intensities) g_free(intensities);
		energies = NULL;
		intensities = NULL;
  		while(!feof(fd) )
  		{
			if(!fgets(t,taille,fd)) break;
	 		if (!(strstr( t,"/ eV      ="))) continue;
			n = sscanf( strstr( t,"=")+1,"%lf",&ener);
			if(n==1)
			{
				 while(!feof(fd) )
				{
					if(!fgets(t,taille,fd)) break;
					if(strstr(t,"Oscillator Strength              "))
					{
						intensity=0.0;
						 if(strstr(t,"Spin forbidden") || 1==sscanf( strstr(t,"Oscillator Strength              ")+strlen("Oscillator Strength              "),"%lf",&intensity))
						{
							numberOfStates++;
							energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
							intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
							energies[numberOfStates-1] = ener;
							intensities[numberOfStates-1] = intensity;
						}
						break;
						
					}
				}
			}
		}
 	}while(!feof(fd));

	if(numberOfStates>0)
	{
		OK = TRUE;
		createUVSpectrumWin(numberOfStates, energies, intensities);
	}
	else
	{
		OK = FALSE;
	}


	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
	fclose(fd);

	return OK;
}
/********************************************************************************/
static gboolean read_nwchem_file(GabeditFileChooser *SelecFile, gint response_id)
{

	if(response_id != GTK_RESPONSE_OK) return FALSE;
	if(!read_nwchem_tddft_file(SelecFile, response_id)&& !read_nwchem_eom_file(SelecFile, response_id))
	{
 		gchar* FileName = gabedit_file_chooser_get_current_file(SelecFile);
 		FILE* fd = FOpen(FileName, "rb");
		if(!fd)
		{
			gchar buffer[BSIZE];
			sprintf(buffer,_("Sorry, I cannot open '%s' file\n"),FileName);
  			Message(buffer,_("Error"),TRUE);
		}
		else
		{
			messageErrorFreq(FileName);
			fclose(fd);
		}
		return FALSE;
	}

	return TRUE;
}
/********************************************************************************/
static void read_nwchem_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_nwchem_file,
			_("Read energies and intensities from a NWChem output file"),
			GABEDIT_TYPEFILE_NWCHEM,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
/*
static gboolean read_psicode_tddft_file(GabeditFileChooser *SelecFile, gint response_id)
{

	gchar *FileName;
 	gchar t[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint n;
	gdouble ener = 0;
	gdouble intensity = 0;
	gint numberOfStates = 0;
	gdouble *energies = NULL;
	gdouble *intensities = NULL;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (strstr( t,"Psicode TDDFT Module") ) {OK = TRUE; break;}
		}
		if(!OK) break;
		numberOfStates = 0;
		if(energies) g_free(energies);
		if(intensities) g_free(intensities);
		energies = NULL;
		intensities = NULL;
  		while(!feof(fd) )
  		{
			if(!fgets(t,taille,fd)) break;
	 		if (!(strstr( t,"Root") && strstr( t,"eV") && strstr( t,"(") && strstr( t,")")) ) continue;
			n = sscanf( strstr( t,"(")+1,"%lf",&ener);
			if(n==1)
			{
				 while(!feof(fd) )
				{
					if(!fgets(t,taille,fd)) break;
					if(strstr(t,"Dipole Oscillator Strength"))
					{
						intensity=0.0;
						 if(strstr(t,"Spin forbidden") || 1==sscanf( strstr(t,"Dipole Oscillator Strength")+strlen("Dipole Oscillator Strength"),"%lf",&intensity))
						{
							numberOfStates++;
							energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
							intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
							energies[numberOfStates-1] = ener;
							intensities[numberOfStates-1] = intensity;
						}
						break;
						
					}
				}
			}
		}
 	}while(!feof(fd));

	if(numberOfStates>0)
	{
		OK = TRUE;
		createUVSpectrumWin(numberOfStates, energies, intensities);
	}
	else
	{
		OK = FALSE;
	}


	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
	fclose(fd);

	return OK;
}
*/
/********************************************************************************/
static gboolean read_psicode_eom_file(GabeditFileChooser *SelecFile, gint response_id)
{

	gchar *FileName;
 	gchar t[BSIZE];
 	gchar dum[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint n;
	gdouble ener = 0;
	gdouble intensity = 0;
	gint numberOfStates = 0;
	gdouble *energies = NULL;
	gdouble *intensities = NULL;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
    			{ char* e = fgets(t,taille,fd);}
	 		if (
			strstr( t,"State") 
			&& strstr( t,"(cm") 
			&& strstr( t,"(nm)") 
			&& strstr( t,"(eV)") 
			&& strstr( t,"(au)") 
			 ) {OK = TRUE; break;}
		}
		if(!OK) break;
		numberOfStates = 0;
		if(energies) g_free(energies);
		if(intensities) g_free(intensities);
		energies = NULL;
		intensities = NULL;
  		while(!feof(fd) )
  		{
			if(!fgets(t,taille,fd)) break;
			if(this_is_a_backspace(t)) break;
			n = sscanf(t, "%s %s %lf %s %s %s %lf",dum,dum,&ener,dum,dum,dum,&intensity);
			if(n==7)
			{
				numberOfStates++;
				energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
				intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
				energies[numberOfStates-1] = ener;
				intensities[numberOfStates-1] = intensity;
			}
		}
 	}while(!feof(fd));

	if(numberOfStates>0)
	{
		OK = TRUE;
		createUVSpectrumWin(numberOfStates, energies, intensities);
	}
	else
	{
		OK = FALSE;
	}


	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
	fclose(fd);

	return OK;
}
/********************************************************************************/
static gboolean read_psicode_file(GabeditFileChooser *SelecFile, gint response_id)
{

	if(response_id != GTK_RESPONSE_OK) return FALSE;
	/* if(!read_psicode_tddft_file(SelecFile, response_id)&& !read_psicode_eom_file(SelecFile, response_id))*/
	if(!read_psicode_eom_file(SelecFile, response_id))
	{
 		gchar* FileName = gabedit_file_chooser_get_current_file(SelecFile);
 		FILE* fd = FOpen(FileName, "rb");
		if(!fd)
		{
			gchar buffer[BSIZE];
			sprintf(buffer,_("Sorry, I cannot open '%s' file\n"),FileName);
  			Message(buffer,_("Error"),TRUE);
		}
		else
		{
			messageErrorFreq(FileName);
			fclose(fd);
		}
		return FALSE;
	}

	return TRUE;
}
/********************************************************************************/
static void read_psicode_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_psicode_file,
			_("Read energies and intensities from a Psicode output file"),
			GABEDIT_TYPEFILE_NWCHEM,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_adf_file(GabeditFileChooser *SelecFile, gint response_id)
{
	return FALSE;
}
/********************************************************************************/
static void read_adf_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_adf_file,
			_("Read energies and intensities from a ADF output file"),
			GABEDIT_TYPEFILE_MOLPRO,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_sample_2columns_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar t[BSIZE];
 	gboolean OK = TRUE;
	gint numberOfStates = 0;
	gdouble* energies = NULL;
	gdouble* intensities = NULL;
	gchar *FileName;
 	FILE *fd;
	gdouble a;
	gdouble b;
	int ne = 0;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	while(!feof(fd))
	{
	 	if(!fgets(t,BSIZE,fd))break;
		ne = sscanf(t,"%lf %lf",&a,&b);
		if(ne==2)
		{
			numberOfStates++;
			energies = g_realloc(energies, numberOfStates*sizeof(gdouble));
			intensities = g_realloc(intensities, numberOfStates*sizeof(gdouble));
			energies[numberOfStates-1] = a;
			intensities[numberOfStates-1] = b;
		}
 	}

	if(numberOfStates>0)
	{
		createUVSpectrumWin(numberOfStates, energies, intensities);
	}
	else
	{
		OK = FALSE;
		messageErrorFreq(FileName);
	}


	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
	fclose(fd);
	return OK;
}
/********************************************************************************/
static void read_sample_2columns_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_sample_2columns_file,
			_("Read energies and intensities from a sample file(2columns : first = Energy(eV), second = intensity)"),
			GABEDIT_TYPEFILE_TXT,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
void createUVSpectrum(GtkWidget *parentWindow, GabEditTypeFile typeOfFile)
{
	if(typeOfFile==GABEDIT_TYPEFILE_GABEDIT) read_gabedit_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_DALTON) read_dalton_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_MOLDEN) read_molden_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_MOLPRO) read_molpro_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_GAMESS) read_gamess_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_GAUSSIAN) read_gaussian_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_ADF) read_adf_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_MPQC) read_mpqc_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_ORCA) read_orca_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_FIREFLY) read_firefly_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_QCHEM) read_qchem_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_NWCHEM) read_nwchem_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_PSICODE) read_psicode_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_TXT) read_sample_2columns_file_dlg();
}
/********************************************************************************/
