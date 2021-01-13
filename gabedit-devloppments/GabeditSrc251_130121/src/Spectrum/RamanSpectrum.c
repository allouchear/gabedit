/* RamanSpectrum.c */
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
#include <gtk/gtk.h>
#include <gdk/gdk.h>
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
			X[i]= 10000.0/data->x[loop];
			Y[i]= data->y[loop];
			if(X[i]>*pxmax && fabs(Y[i])>*pymax*1e-2) *pxmax = X[i];
			i++;
		}
	}
       	gabedit_xyplot_add_data_conv(GABEDIT_XYPLOT(newxyplot),i, X,  Y, 1.0, GABEDIT_XYPLOT_CONV_NONE,NULL);
	if(X) g_free(X);
	if(Y) g_free(Y);
}
/**********************************************************************************************/
static void create_microm_spectrum(GtkWidget *oldWin,gpointer d)
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

	newWin = gabedit_xyplot_new_window("Raman",NULL);
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
	gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(newxyplot), "&#181;m");
	gabedit_xyplot_set_y_label (GABEDIT_XYPLOT(newxyplot), "Intensity");
	/* fprintf(stderr,"xmax=%f\n",xmax);*/

}
/**********************************************************************************************/
static void check_microm_cmm1_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	GtkWidget* xyplot = NULL;
	GList* data_list = NULL;
	GList* current = NULL;
	XYPlotWinData* data;
	gboolean microm = FALSE;
	GtkWidget* window = NULL;
	gdouble xmax = 0;
	gdouble ymax = 0;

	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");

	if(!data_list) return;
	window = g_object_get_data(G_OBJECT (xyplot), "Window");

	microm = gtk_toggle_button_get_active(togglebutton);
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		gint loop;
		data = (XYPlotWinData*)current->data;
		for (loop=0; loop<data->size; loop++)
		{
			if(data->x[loop]!=0)
			{
				if(microm) data->x[loop]= 10000.0/data->x[loop];
				else data->x[loop]= 10000.0/data->x[loop];
			}
			else
				data->x[loop]= 1.0e-10;
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
	if(microm)
		spectrum_win_set_xlabel(window,"&#181;m");
	else
		spectrum_win_set_xlabel(window,"cm<sup>-1</sup>");
	{
		GtkWidget* micromButton = g_object_get_data(G_OBJECT (togglebutton), "micromButton");
		if(microm) gtk_widget_set_sensitive(micromButton,FALSE);
		else gtk_widget_set_sensitive(micromButton,TRUE);
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));

}
/********************************************************************************/
static void reflectXY(GtkWidget *buttonReflect, gpointer user_data)
{
	GtkWidget* xyplot = NULL;
	GList* data_list = NULL;
	GList* current = NULL;
	XYPlotWinData* data;
	gboolean microm = FALSE;
	gdouble xmax = 0;
	gdouble ymax = 0;
	gboolean rx;
	gboolean ry;

	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");

	if(!data_list) return;

	gabedit_xyplot_get_reflects (GABEDIT_XYPLOT (xyplot),&rx, &ry);

	gabedit_xyplot_reflect_x (GABEDIT_XYPLOT(xyplot), !rx);
        gabedit_xyplot_reflect_y (GABEDIT_XYPLOT(xyplot), !ry);

	gtk_widget_queue_draw(GTK_WIDGET(xyplot));

}
/********************************************************************************/
static GtkWidget* createRamanSpectrumWin(gint numberOfFrequencies, gdouble* frequencies, gdouble* intensities)
{
	GtkWidget* window = spectrum_win_new_with_xy(_("Raman spectrum"),  numberOfFrequencies, frequencies, intensities);
	GtkWidget* hbox = g_object_get_data(G_OBJECT (window), "HBoxData");
	GtkWidget* hbox2 = g_object_get_data(G_OBJECT (window), "HBoxData2");
	GtkWidget* xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
	GtkWidget* check_microm_cmm1 = NULL;
	GtkWidget* tmp_hbox = NULL;
	GtkWidget* buttonReflect = NULL;
	GtkWidget* button = NULL;

	spectrum_win_relect_x(window, TRUE);
	spectrum_win_relect_y(window, TRUE);
	spectrum_win_set_half_width(window, 20.0);
	spectrum_win_set_xmin(window, 0.0);
	set_icone(window);


	if(!hbox) return window;

	tmp_hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(hbox), tmp_hbox, FALSE, FALSE, 30);
	gtk_widget_show(tmp_hbox);

	check_microm_cmm1 = gtk_check_button_new_with_label("microm<=>cm-1");
	gtk_box_pack_start(GTK_BOX(tmp_hbox), check_microm_cmm1, FALSE, FALSE, 4);
	gtk_widget_show(check_microm_cmm1);

	g_signal_connect(G_OBJECT(check_microm_cmm1), "toggled", G_CALLBACK(check_microm_cmm1_toggled), xyplot);
	gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), "cm<sup>-1</sup>");

	g_signal_connect(G_OBJECT(check_microm_cmm1), "toggled", G_CALLBACK(check_microm_cmm1_toggled), xyplot);
	spectrum_win_set_xlabel(window, "cm<sup>-1</sup>");
	buttonReflect = create_button(window,_("Reflect"));
	gtk_box_pack_start(GTK_BOX(hbox2), buttonReflect, FALSE, FALSE, 4);
        g_signal_connect(G_OBJECT(buttonReflect), "clicked", G_CALLBACK(reflectXY), xyplot);
	gtk_widget_show(buttonReflect);

	button = create_button(window,_("microm"));
        gtk_box_pack_start (GTK_BOX( hbox2), button, TRUE, TRUE, 3);
        GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
        gtk_widget_grab_default(button);
        gtk_widget_show (button);
        g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)create_microm_spectrum,GTK_OBJECT(window));

	g_object_set_data(G_OBJECT (check_microm_cmm1), "micromButton",button);
	g_object_set_data(G_OBJECT (button), "Window",window);



	return window;
}
/********************************************************************************/
static void messageErrorFreq(gchar* fileName)
{
	gchar buffer[BSIZE];
	sprintf(buffer,_("Sorry, I can not read frequencies from '%s' file\n"),fileName);
  	Message(buffer,_("Error"),TRUE);
}
/********************************************************************************/
static void messageErrorInt(gchar* fileName)
{
	gchar buffer[BSIZE];
	sprintf(buffer,_("Sorry, I can not read frequencies from '%s' file\n"),fileName);
  	Message(buffer,_("Error"),TRUE);
}
/********************************************************************************/
static gboolean read_gabedit_molden_file(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *FileName;
 	gchar t[BSIZE];
 	gboolean OK;
 	FILE *fd;
	gint i;
	gint ne;
	gint numberOfFrequencies = 0;
	gdouble* frequencies = NULL;
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
		if(strstr(t,"[FREQ"))
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
		ne = sscanf(t,"%lf",&a);
		if(ne != 1)
		{
			messageErrorFreq(FileName);
			if(frequencies) g_free(frequencies);
			if(intensities) g_free(intensities);
			fclose(fd);
			return FALSE;
		}
		numberOfFrequencies++;
		frequencies = g_realloc(frequencies, numberOfFrequencies*sizeof(gdouble));
		intensities = g_realloc(intensities, numberOfFrequencies*sizeof(gdouble));
		frequencies[numberOfFrequencies-1] = a;
		intensities[numberOfFrequencies-1] = 0;
	}
	if(numberOfFrequencies<1)
	{
		messageErrorFreq(FileName);
		if(frequencies) g_free(frequencies);
		if(intensities) g_free(intensities);
		fclose(fd);
		return FALSE;
	}
	OK = FALSE;
	if(strstr(t,"[INT"))OK = TRUE;
  	while(!feof(fd) && !OK)
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"[INT"))
		{
			OK = TRUE;
			break;
		}
	}
	if(!OK)
	{
		messageErrorInt(FileName);
		if(fd) fclose(fd);
		return FALSE;
	}
	for(i=0;i<numberOfFrequencies;i++)
	{
    		if(!fgets(t,BSIZE,fd))
		{
			messageErrorInt(FileName);
			if(frequencies) g_free(frequencies);
			if(intensities) g_free(intensities);
			fclose(fd);
			return FALSE;
		}
		ne = sscanf(t,"%lf %lf",&b,&a);
		if(ne != 2)
		{
			messageErrorInt(FileName);
			if(frequencies) g_free(frequencies);
			if(intensities) g_free(intensities);
			fclose(fd);
			return FALSE;
		}
		intensities[i] = a;
	}

	createRamanSpectrumWin(numberOfFrequencies, frequencies, intensities);



	if(frequencies) g_free(frequencies);
	if(intensities) g_free(intensities);
	fclose(fd);
	return TRUE;

}
/********************************************************************************/
static void read_gabedit_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gabedit_molden_file,
			_("Read frequencies and intensities from a Gabedit file"),
			GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static void read_molden_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gabedit_molden_file,
			_("Read frequencies and intensities from a Molden file"),
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
			_("Read frequencies and intensities from a MPQC output file"),
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
			_("Read frequencies and intensities from a Molpro output file"),
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
			_("Read last frequencies and intensities from a Dalton output file"),
			GABEDIT_TYPEFILE_DALTON,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_gamess_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar t[BSIZE];
 	gboolean OK;
	gint i;
	gint nf;
	gint nir;
	gint nfMax = 5;
	gdouble freq[5];
	gdouble ir[5];
 	gchar* sdum[5*2];
 	gchar* tmp;
	gint numberOfFrequencies = 0;
	gdouble* frequencies = NULL;
	gdouble* intensities = NULL;
	gchar *FileName;
 	FILE *fd;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");

 	OK=FALSE;
 	while(!feof(fd))
	{
		if(!fgets(t,BSIZE,fd))break;
	 	if ( strstr( t,"FREQUENCY:") )
	  	{
			OK = TRUE;
			break;
	  	}
	}

	if(!OK) 
	{
		messageErrorFreq(FileName);
		if(frequencies) g_free(frequencies);
		if(intensities) g_free(intensities);
		return FALSE;
	}
	for(i=0;i<nfMax*2;i++) sdum[i] = g_malloc(BSIZE*sizeof(gchar));

  	while(!feof(fd))
  	{
		gint nfi=0;
		if(!strstr( t,"FREQUENCY:"))
		{
			if(!fgets(t,BSIZE,fd)) break;
			continue;
		}

		tmp = strstr(t,":")+1;
		for(i=0;i<nfMax*2;i++) sprintf(sdum[i]," ");
		nfi = sscanf(tmp,"%s %s %s %s %s %s %s %s %s %s", sdum[0],sdum[1],sdum[2],sdum[3],sdum[4],
				sdum[5],sdum[6],sdum[7],sdum[8],sdum[9]
				);
		if(nfi<1)
		{
			for(i=0;i<nfMax*2;i++) g_free(sdum[i]);
			messageErrorFreq(FileName);
			if(frequencies) g_free(frequencies);
			if(intensities) g_free(intensities);
			return FALSE;
		}
		nf = 0;
		for(i=0;i<nfi;)
		{
			if(strstr(sdum[i+1],"I"))
			{
				freq[nf] = -atof(sdum[i]);
				i+=2;
			}
			else
			{
				freq[nf] = atof(sdum[i]);
				i+=1;
			}
			nf++;
		}
		nir=-1;
		while(fgets(t,BSIZE,fd) && strstr(t,":")) /* REDUCED MASS: IR INTENSITY: RAMAN ACTIVITY: Depol,... backspace */
		{
			if(strstr(t,"RAMAN"))
			{
				tmp =  strstr(t,":")+1;
				nir = sscanf(tmp,"%s %s %s %s %s", sdum[0],sdum[1],sdum[2],sdum[3],sdum[4]);
			}
		}
		if(nf!=nir)
		{
			for(i=0;i<nfMax*2;i++) g_free(sdum[i]);
			messageErrorFreq(FileName);
			if(frequencies) g_free(frequencies);
			if(intensities) g_free(intensities);
			return FALSE;
		}
		for(i=0;i<nf;i++) ir[i] = atof(sdum[i]);

		for(i=0;i<nf;i++)
		{
			numberOfFrequencies++;
			frequencies = g_realloc(frequencies, numberOfFrequencies*sizeof(gdouble));
			intensities = g_realloc(intensities, numberOfFrequencies*sizeof(gdouble));
			frequencies[numberOfFrequencies-1] = freq[i];
			intensities[numberOfFrequencies-1] = ir[i];
		}
		if(!fgets(t,BSIZE,fd)) break;
	}
	if(numberOfFrequencies>0)
	{
		createRamanSpectrumWin(numberOfFrequencies, frequencies, intensities);
	}
	else
	{
		messageErrorFreq(FileName);
	}


	if(frequencies) g_free(frequencies);
	if(intensities) g_free(intensities);
	fclose(fd);
	return TRUE;
}
/********************************************************************************/
static void read_gamess_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gamess_file,
			_("Read last frequencies and intensities from a Gamess output file"),
			GABEDIT_TYPEFILE_GAMESS,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_gaussian_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gchar sdum2[BSIZE];
 	gchar sdum3[BSIZE];
	gint nf;
	gint i;
	gdouble freq[3] = {0,0,0};
	gdouble RamanIntensity[3] = {0,0,0};
	gint numberOfFrequencies = 0;
	gdouble* frequencies = NULL;
	gdouble* intensities = NULL;
	gchar *FileName;
 	FILE *fd;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	do 
 	{
 		while(!feof(fd))
		{
    			if(!feof(fd)) { char* e = fgets(t,BSIZE,fd);}
	 		/* if ( strstr( t,"reduced masses") )*/
	 		if ( strstr( t,"and normal coordinates:") )
	  		{
				numberOfFrequencies = 0;
				break;
	  		}
		}
  		while(!feof(fd) )
  		{
    			if(!feof(fd)) { char* e = fgets(t,BSIZE,fd);}
			if(!strstr(t,"Frequencies --"))continue;
			if(this_is_a_backspace(t)) break;
			nf = sscanf(t,"%s %s %lf %lf %lf", sdum1,sdum2, &freq[0],&freq[1],&freq[2]);
			nf -=2;
			if(nf<=0 || nf>3) break;
			while(!feof(fd))
			{
    				if(!feof(fd)) { char* e = fgets(t,BSIZE,fd);}
				if(strstr(t,"Raman"))
				{
					sscanf(t,"%s %s %s %lf %lf %lf", sdum1,sdum2, sdum3, &RamanIntensity[0],&RamanIntensity[1],&RamanIntensity[2]);
					break;
				}
			}
			for(i=0;i<nf;i++)
			{
				numberOfFrequencies++;
				frequencies = g_realloc(frequencies, numberOfFrequencies*sizeof(gdouble));
				intensities = g_realloc(intensities, numberOfFrequencies*sizeof(gdouble));
				frequencies[numberOfFrequencies-1] = freq[i];
				intensities[numberOfFrequencies-1] = RamanIntensity[i];
			}
		}
 	}while(!feof(fd));

	if(numberOfFrequencies>0)
	{
		createRamanSpectrumWin(numberOfFrequencies, frequencies, intensities);
	}
	else
	{
		messageErrorFreq(FileName);
	}


	if(frequencies) g_free(frequencies);
	if(intensities) g_free(intensities);
	fclose(fd);
	return TRUE;
}
/********************************************************************************/
static void read_gaussian_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_gaussian_file,
			_("Read last frequencies and intensities from a Gaussian output file"),
			GABEDIT_TYPEFILE_GAUSSIAN,GABEDIT_TYPEWIN_OTHER);

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
			_("Read last frequencies and intensities from a ADF output file"),
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
	gdouble freq = 0;
	gdouble RamanIntensity = 0;
	gint numberOfFrequencies = 0;
	gdouble *frequencies = NULL;
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
    			if(!feof(fd)) { char* e = fgets(t,BSIZE,fd);}
	 		if (strstr( t,"RAMAN SPECTRUM") ) OK = TRUE;
	 		if (strstr( t,"Activity")  && strstr( t,"Depolarization") && OK ){ OK = TRUE; break;}
		}
		if(!OK) break;
		numberOfFrequencies = 0;
		if(frequencies) g_free(frequencies);
		if(intensities) g_free(intensities);
		frequencies = NULL;
		intensities = NULL;
    		if(!feof(fd)) { char* e = fgets(t,BSIZE,fd);}
  		while(!feof(fd) )
  		{
			if(!fgets(t,taille,fd)) break;
			if(atoi(t)<=0) break;
			n = sscanf(t,"%s %lf %lf", sdum1, &freq,&RamanIntensity);
			if(n==3)
			{
				numberOfFrequencies++;
				frequencies = g_realloc(frequencies, numberOfFrequencies*sizeof(gdouble));
				intensities = g_realloc(intensities, numberOfFrequencies*sizeof(gdouble));
				frequencies[numberOfFrequencies-1] = freq;
				intensities[numberOfFrequencies-1] = RamanIntensity;
			}
		}
 	}while(!feof(fd));

	if(numberOfFrequencies>0)
	{
		createRamanSpectrumWin(numberOfFrequencies, frequencies, intensities);
	}
	else
	{
		messageErrorFreq(FileName);
	}


	if(frequencies) g_free(frequencies);
	if(intensities) g_free(intensities);
	fclose(fd);

	return TRUE;
}
/********************************************************************************/
static void read_orca_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_orca_file,
			_("Read last frequencies and intensities from a Orca output file"),
			GABEDIT_TYPEFILE_QCHEM,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_sample_2columns_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar t[BSIZE];
 	gboolean OK = TRUE;
	gint numberOfFrequencies = 0;
	gdouble* frequencies = NULL;
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
			numberOfFrequencies++;
			frequencies = g_realloc(frequencies, numberOfFrequencies*sizeof(gdouble));
			intensities = g_realloc(intensities, numberOfFrequencies*sizeof(gdouble));
			frequencies[numberOfFrequencies-1] = a;
			intensities[numberOfFrequencies-1] = b;
		}
 	}

	if(numberOfFrequencies>0)
	{
		createRamanSpectrumWin(numberOfFrequencies, frequencies, intensities);
	}
	else
	{
		OK = FALSE;
		messageErrorFreq(FileName);
	}


	if(frequencies) g_free(frequencies);
	if(intensities) g_free(intensities);
	fclose(fd);
	return OK;
}
/********************************************************************************/
static void read_sample_2columns_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_sample_2columns_file,
			_("Read frequencies and intensities from a sample file(2columns : first = Energy(cm-1), second = intensity)"),
			GABEDIT_TYPEFILE_TXT,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_qchem_file(GabeditFileChooser *SelecFile, gint response_id)
{

	gchar *FileName;
	gint i;
 	gchar t[BSIZE];
 	gchar sdum1[BSIZE];
 	gchar sdum2[BSIZE];
 	gboolean OK;
 	FILE *fd;
 	guint taille=BSIZE;
	gint idum;
	gint nf;
	gdouble freq[3] = {0,0,0};
	gdouble RamanIntensity[3] = {0,0,0};
	gint numberOfFrequencies = 0;
	gdouble *frequencies = NULL;
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
    			if(!feof(fd)) { char* e = fgets(t,BSIZE,fd);}
	 		if (strstr( t,"VIBRATIONAL ANALYSIS") ) OK = TRUE;
	 		if (strstr( t,"Mode:") && OK ){ OK = TRUE; break;}
		}
		if(!OK) break;
  		while(!feof(fd) )
  		{
			if(!strstr(t,"Mode:")) break;
			nf = sscanf(t,"%s %d %d %d",sdum1,&idum,&idum,&idum);
			nf--;
			if(nf<0 || nf>3) break;

			if(!fgets(t,taille,fd)) break;
			sscanf(t,"%s %lf %lf %lf", sdum1, &freq[0],&freq[1],&freq[2]);
			while(!feof(fd))
			{
    				if(!feof(fd)) { char* e = fgets(t,BSIZE,fd);}
				if(strstr(t,"Raman Intens"))
				{
					sscanf(t,"%s %s %lf %lf %lf", sdum1,sdum2, &RamanIntensity[0],&RamanIntensity[1],&RamanIntensity[2]);
					break;
				}
			}
			for(i=0;i<nf;i++)
			{
				numberOfFrequencies++;
				frequencies = g_realloc(frequencies, numberOfFrequencies*sizeof(gdouble));
				intensities = g_realloc(intensities, numberOfFrequencies*sizeof(gdouble));
				frequencies[numberOfFrequencies-1] = freq[i];
				intensities[numberOfFrequencies-1] = RamanIntensity[i];
			}
			if(!strstr(t,"X      Y      Z"))
			while(!feof(fd))
			{
    				if(!feof(fd)) { char* e = fgets(t,BSIZE,fd);}
				if(strstr(t,"Mode:")) break; /* Mode: or END */
			}
		}
 	}while(!feof(fd));

	if(numberOfFrequencies>0)
	{
		createRamanSpectrumWin(numberOfFrequencies, frequencies, intensities);
	}
	else
	{
		messageErrorFreq(FileName);
	}


	if(frequencies) g_free(frequencies);
	if(intensities) g_free(intensities);
	fclose(fd);

	return TRUE;
}
/********************************************************************************/
static void read_qchem_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_qchem_file,
			_("Read last frequencies and intensities from a Q-Chem output file"),
			GABEDIT_TYPEFILE_QCHEM,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
void createRamanSpectrum(GtkWidget *parentWindow, GabEditTypeFile typeOfFile)
{
	if(typeOfFile==GABEDIT_TYPEFILE_GABEDIT) read_gabedit_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_DALTON) read_dalton_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_MOLDEN) read_molden_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_MOLPRO) read_molpro_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_ORCA) read_orca_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_GAMESS) read_gamess_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_FIREFLY) read_gamess_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_QCHEM) read_qchem_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_GAUSSIAN) read_gaussian_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_ADF) read_adf_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_MPQC) read_mpqc_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_TXT) read_sample_2columns_file_dlg();
}
/********************************************************************************/
void createRamanSpectrumFromVibration(GtkWidget *parentWindow, Vibration ibration)
{
	gint numberOfFrequencies = vibration.numberOfFrequencies;
	gdouble* frequencies = NULL;
	gdouble* intensities = NULL;
	gint j;
	if(numberOfFrequencies<1)
	{
		GtkWidget* mess=Message(_("Sorry, For draw spectrum, Please read a file beforee"),_("Error"),TRUE);
    		gtk_window_set_modal (GTK_WINDOW (mess), TRUE);
		return;
	}
	frequencies = g_malloc(numberOfFrequencies*sizeof(gdouble));
	intensities = g_malloc(numberOfFrequencies*sizeof(gdouble));

	for (j=0; j < vibration.numberOfFrequencies; j++)
	{
		frequencies[j] = vibration.modes[j].frequence;
		intensities[j] = vibration.modes[j].RamanIntensity;
	}
	createRamanSpectrumWin(numberOfFrequencies, frequencies, intensities);
	if(frequencies) g_free(frequencies);
	if(intensities) g_free(intensities);
}
