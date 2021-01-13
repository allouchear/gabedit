/* DOSSpectrum.c */
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

/********************************************************************************/
static void createDOSSpectrumWin(gint numberOfStates, gdouble* energies, gdouble* intensities)
{
	GtkWidget* window = spectrum_win_new_with_xy(_("Density of states"),  numberOfStates, energies, intensities);
	GtkWidget* hbox = g_object_get_data(G_OBJECT (window), "HBoxData");
	GabeditXYPlot *xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");

	spectrum_win_set_half_width(window, 0.5);
	set_icone(window);

	if(!hbox) return;

	spectrum_win_set_xlabel(window, "eV");
	spectrum_win_set_ylabel(window, _("Intensity"));

	if(!xyplot || !G_IS_OBJECT(xyplot)) return;
	gabedit_xyplot_set_autorange(xyplot,NULL);

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
static gboolean read_gabedit_file(GabeditFileChooser *SelecFile, gint response_id)
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
	gdouble b = 1;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
 	OK=FALSE;

  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"[DOS"))
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
		createDOSSpectrumWin(numberOfStates, energies, intensities);
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
 	file_chooser_open(read_gabedit_file,
			_("Read energies from a Gabedit file"),
			GABEDIT_TYPEFILE_GABEDIT,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
static gboolean read_sample_1column_file(GabeditFileChooser *SelecFile, gint response_id)
{
 	gchar t[BSIZE];
 	gboolean OK = TRUE;
	gint numberOfStates = 0;
	gdouble* energies = NULL;
	gdouble* intensities = NULL;
	gchar *FileName;
 	FILE *fd;
	gdouble a;
	gdouble b = 1;
	int ne = 0;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	FileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(FileName, "rb");
	if(!fd) return FALSE;

 	while(!feof(fd))
	{
	 	if(!fgets(t,BSIZE,fd))break;
		ne = sscanf(t,"%lf",&a);
		if(ne==1)
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
		createDOSSpectrumWin(numberOfStates, energies, intensities);
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
static void read_sample_1column_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_sample_1column_file,
			_("Read energies from a sample file(1 column : Energy(eV))"),
			GABEDIT_TYPEFILE_TXT,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
/********************************************************************************/
void createDOSSpectrum(GtkWidget *parentWindow, GabEditTypeFile typeOfFile)
{
	if(typeOfFile==GABEDIT_TYPEFILE_GABEDIT) read_gabedit_file_dlg();
	if(typeOfFile==GABEDIT_TYPEFILE_TXT) read_sample_1column_file_dlg();
}
/********************************************************************************/
void createDOSSpectrumFromEnergiesTable(GtkWidget *parentWindow, gdouble* auEnergies, gint nEnergies)
{
	gint numberOfStates = nEnergies;
	gdouble* energies = NULL;
	gdouble* intensities = NULL;
	gint j;
	if(numberOfStates<1)
	{
		GtkWidget* mess=Message(_("The number of states should be > 0"),_("Error"),TRUE);
    		gtk_window_set_modal (GTK_WINDOW (mess), TRUE);
		return;
	}
	energies = g_malloc(numberOfStates*sizeof(gdouble));
	intensities = g_malloc(numberOfStates*sizeof(gdouble));

	for (j=0; j < numberOfStates; j++)
	{
		energies[j] = auEnergies[j]*AUTOEV;
		intensities[j] = 1;
	}
	createDOSSpectrumWin(numberOfStates, energies, intensities);
	if(energies) g_free(energies);
	if(intensities) g_free(intensities);
}
