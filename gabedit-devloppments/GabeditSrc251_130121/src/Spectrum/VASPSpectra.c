/* VASPSpectra.c */
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
/********************************************************************************/
static void g_list_free_all (GList * list, GDestroyNotify free_func)
{
    g_list_foreach (list, (GFunc) free_func, NULL);
    g_list_free (list);
} 
/********************************************************************************/
static GdkColor get_fore_color(GabeditXYPlot *xyplot)
{
        GdkGCValues values;
        GdkColormap *colormap;
        GdkColor color;

        gdk_gc_get_values(xyplot->fore_gc, &values);
        colormap  = gdk_window_get_colormap(GTK_WIDGET(xyplot)->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);
        return color;
}
/********************************************************************************/
static void free_list(GList* list)
{
	/*
	GList* current = NULL;
	if(!list) return;
	current = g_list_first(list);
	for(; current != NULL; current = current->next)
	{
		gdouble* data = (gdouble*)current->data;
		if(data) g_free(data);
	}
	g_list_free(list);
	*/
	g_list_free_all(list, g_free);
	
}
/********************************************************************************/
static void add_new_data(GabeditXYPlot* xyplot, GList* X, GList* Y, gchar* color)
{
	gint numberOfPoints = 0;
	if(X && Y)
	{
		GList* current = NULL;
		current = g_list_first(X);
		for(; current != NULL; current = current->next) numberOfPoints++;
	}
	//printf("Np=%d\n",numberOfPoints);
		
	if(numberOfPoints>0)
	{
		GList* cx = g_list_first(X);
		GList* cy = g_list_first(Y);
		gint loop;
		XYPlotData *data = g_malloc(sizeof(XYPlotData));
		GdkColor c = get_fore_color(GABEDIT_XYPLOT(xyplot));
		if(color) gdk_color_parse (color, &c);

		data->size=numberOfPoints;
		data->x = g_malloc(numberOfPoints*sizeof(gdouble)); 
		data->y = g_malloc(numberOfPoints*sizeof(gdouble)); 

		loop = 0;
		for(; cx != NULL; cx = cx->next, cy = cy->next) 
		{
			data->x[loop] = *(gdouble*)(cx->data);
			data->y[loop] = *(gdouble*)(cy->data);
			loop++;
		}

		sprintf(data->point_str,"+");
		data->point_pango = NULL;
		gabedit_xyplot_add_data (GABEDIT_XYPLOT(xyplot), data);

		data->point_size=0;
		data->line_width=2;
		data->point_color.red=c.red; 
		data->point_color.green=c.green; 
		data->point_color.blue=c.blue; 

		data->line_color.red=c.green;
		data->line_color.green=c.red;
		data->line_color.blue=c.blue;
		data->line_style=GDK_LINE_SOLID;
		gabedit_xyplot_add_data (GABEDIT_XYPLOT(xyplot), data);
		gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(xyplot), NULL);
	}
}
/********************************************************************************/
static void createVASPSpectraWin(gchar* title, GList* X, GList* Y, GList* Z, gchar* xlabel, gchar* ylabel)
{
	GtkWidget* window = gabedit_xyplot_new_window(title, NULL);
	GabeditXYPlot *xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
	set_icone(window);

	if(!xyplot || !G_IS_OBJECT(xyplot)) return;
	if(X && Y) add_new_data(xyplot, X,  Y,"red");
	if(X && Z) add_new_data(xyplot, X,  Z,"blue");
	if(!X) fprintf(stderr,"attention X = NULL\n");
	if(xlabel) gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), xlabel);
	if(ylabel) gabedit_xyplot_set_y_label (GABEDIT_XYPLOT(xyplot), ylabel);
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
/* Ref : J. Mater. Chem. C, 2015, 3, 8625-8633 */
/********************************************************************************/
static  GList* computeAbsorptionCoefficient(GList* listEnergies, GList* listEpsiReal, GList* listEpsiImag)
{
	GList* listAlpha = NULL;
	GList* ce = listEnergies; 
	GList* cr = listEpsiReal; 
	GList* ci = listEpsiImag; 
	gdouble eVTocm1=1.0/(AUTOEV)*AU_TO_CM1;
	gdouble fact=eVTocm1*sqrt(2.0);
	for(; ce && cr && ci ; ce = ce->next, cr = cr->next, ci = ci->next) 
	{
		gdouble e = *(gdouble*)(ce->data);
		gdouble r = *(gdouble*)(cr->data);
		gdouble i = *(gdouble*)(ci->data);
		gdouble alpha=fact*sqrt(fabs(sqrt(r*r+i*i)-r));
		gdouble* data = g_malloc(sizeof(gdouble));
		*data = alpha;
		listAlpha=g_list_append(listAlpha, (gpointer) data);
	}
	return listAlpha;
}
/********************************************************************************/
static  GList* computeRefractiveIndex(GList* listEpsiReal, GList* listEpsiImag)
{
	GList* listIndex = NULL;
	GList* cr = listEpsiReal; 
	GList* ci = listEpsiImag; 
	gdouble fact=1.0/sqrt(2.0);
	for(; cr && ci ; cr = cr->next, ci = ci->next) 
	{
		gdouble r = *(gdouble*)(cr->data);
		gdouble i = *(gdouble*)(ci->data);
		gdouble n=fact*sqrt(fabs(sqrt(r*r+i*i)+r));
		gdouble* data = g_malloc(sizeof(gdouble));
		*data = n;
		listIndex=g_list_append(listIndex, (gpointer) data);
	}
	return listIndex;
}
/********************************************************************************/
static  GList* computeExtinctionCoefficient(GList* listEpsiReal, GList* listEpsiImag)
{
	GList* listExtinctionCoefficient = NULL;
	GList* cr = listEpsiReal; 
	GList* ci = listEpsiImag; 
	gdouble fact=1.0/sqrt(2.0);
	for(; cr && ci ; cr = cr->next, ci = ci->next) 
	{
		gdouble r = *(gdouble*)(cr->data);
		gdouble i = *(gdouble*)(ci->data);
		gdouble k=fact*sqrt(fabs(sqrt(r*r+i*i)-r));
		gdouble* data = g_malloc(sizeof(gdouble));
		*data = k;
		listExtinctionCoefficient=g_list_append(listExtinctionCoefficient, (gpointer) data);
	}
	return listExtinctionCoefficient;
}
/********************************************************************************/
static  GList* computeReflectivity(GList* listn, GList* listk)
{
	GList* listR = NULL;
	GList* cn = listn; 
	GList* ck = listk; 
	for(; cn && ck ; cn = cn->next, ck = ck->next) 
	{
		gdouble n = *(gdouble*)(cn->data);
		gdouble k = *(gdouble*)(ck->data);
		gdouble R=((n-1)*(n-1)+k*k)/((n+1)*(n+1)+k*k);
		gdouble* data = g_malloc(sizeof(gdouble));
		*data = R;
		listR=g_list_append(listR, (gpointer) data);
	}
	return listR;
}
/********************************************************************************/
static  GList* computeConductivityReal(GList* listEnergies, GList* listEpsiReal, GList* listEpsiImag)
{
	GList* list = NULL;
	GList* ce = listEnergies; 
	GList* cr = listEpsiReal; 
	GList* ci = listEpsiImag; 
	gdouble eVTofsm1=0.241804;
	gdouble fact=eVTofsm1/2.0;
	for(; ce && cr && ci ; ce = ce->next, cr = cr->next, ci = ci->next) 
	{
		gdouble e = *(gdouble*)(ce->data);
		gdouble r = *(gdouble*)(cr->data);
		gdouble i = *(gdouble*)(ci->data);
		gdouble sigma=fact*e*i;
		gdouble* data = g_malloc(sizeof(gdouble));
		*data = sigma;
		list=g_list_append(list, (gpointer) data);
	}
	return list;
}
static  GList* computeConductivityImag(GList* listEnergies, GList* listEpsiReal, GList* listEpsiImag)
{
	GList* list = NULL;
	GList* ce = listEnergies; 
	GList* cr = listEpsiReal; 
	GList* ci = listEpsiImag; 
	gdouble eVTofsm1=0.241804;
	gdouble fact=eVTofsm1/2.0;
	for(; ce && cr && ci ; ce = ce->next, cr = cr->next, ci = ci->next) 
	{
		gdouble e = *(gdouble*)(ce->data);
		gdouble r = *(gdouble*)(cr->data);
		gdouble i = *(gdouble*)(ci->data);
		gdouble sigma=fact*e*(1-r);
		gdouble* data = g_malloc(sizeof(gdouble));
		*data = sigma;
		list=g_list_append(list, (gpointer) data);
	}
	return list;
}
/********************************************************************************/
static  GList* computeEnergyLossFunction(GList* listEpsiReal, GList* listEpsiImag)
{
	GList* list = NULL;
	GList* cr = listEpsiReal; 
	GList* ci = listEpsiImag; 
	for(; cr && ci ; cr = cr->next, ci = ci->next) 
	{
		gdouble r = *(gdouble*)(cr->data);
		gdouble i = *(gdouble*)(ci->data);
		gdouble m=(r*r+i*i);
		gdouble L=(m==0)?0:i/m;
		gdouble* data = g_malloc(sizeof(gdouble));
		*data = L;
		list=g_list_append(list, (gpointer) data);
	}
	return list;
}
/********************************************************************************/
static gboolean read_lists_vasp_xml_file(FILE* fd, gchar* tag, gint direction, GList** plistEnergies, GList** plistEpsi)
{
	GList* listEnergies = *plistEnergies;
	GList* listEpsi = *plistEpsi; 
 	gchar t[BSIZE];
 	gboolean OK = TRUE;
	gdouble e, exx, eyy, ezz, exy, exz, eyz;
	gint numberOfPoints = 0;
        if(!goToStr(fd, tag))
        {
		gchar buffer[BSIZE];
		sprintf(buffer,_("I cannot read dielectric function from the VASP xml file\nChech your file\n"));
  		Message(buffer,_("Error"),TRUE);
                return FALSE;
        }

  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"<r>"))
		{
//			printf("%s\n",(strstr(t,"<r>")+strlen("<r>")));
			if(7==sscanf(strstr(t,"<r>")+strlen("<r>"),"%lf %lf %lf %lf %lf %lf %lf",
			&e, &exx, &eyy, &ezz, &exy, &exz, &eyz))
			{
				double* data;
				data = g_malloc(sizeof(gdouble));
				*data = e;
				listEnergies=g_list_append(listEnergies, (gpointer) data);
				data = g_malloc(sizeof(gdouble));
				*data = exx;
				if(direction == 1) *data = eyy;
				if(direction == 2) *data = ezz;
				if(direction == 3) *data = exy;
				if(direction == 4) *data = exz;
				if(direction == 5) *data = eyz;
				if(direction == 6) *data = (exx+eyy+ezz)/3;
				listEpsi=g_list_append(listEpsi, (gpointer) data);
				numberOfPoints++;
			}
			else
			{
				OK = FALSE;
				break;
			}
		}
		if(strstr(t,"</set>"))
		{
			OK = TRUE;
			break;
		}
	}
	*plistEnergies = listEnergies;
	*plistEpsi = listEpsi; 
	return OK;
}
/********************************************************************************/
static gboolean read_dielectric_vasp_xml_file(gchar* fileName, GtkWidget* entry_options, GtkWidget* entry_directions, GtkWidget** toggle_functions)
{
 	gchar t[BSIZE];
 	gboolean OK;
 	FILE *fd;
	gint ne;
	gint numberOfPoints = 0;
	gdouble* energies = NULL;
	gdouble* intensities = NULL;
	gdouble a;
	gdouble b = 1;
	GList* listEnergies = NULL;
	GList* listEpsiImag = NULL;
	GList* listEpsiReal = NULL;
	GList* listAlpha = NULL;
	GList* listRefractiveIndex = NULL;
	GList* listExtinctionCoefficient = NULL;
	GList* listReflectivity = NULL;
	GList* listSigmaReal = NULL;
	GList* listSigmaImag = NULL;
	GList* listEnergyLoss = NULL;
	gchar* option = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry_options)));
	G_CONST_RETURN gchar* tdirection = gtk_entry_get_text(GTK_ENTRY(entry_directions));
	gint direction = 0;
	gchar* directions[] = {"XX","YY","ZZ","XY","XZ","YZ","XYZ"};
	gint nDirections = sizeof (directions)/sizeof(directions[0]);
	gint i;

	for(i=0;i<nDirections;i++) if(!strcmp(tdirection,directions[i])) direction = i;

 	fd = FOpen(fileName, "rb");
 	OK=FALSE;

        if(!goToStr(fd, option))
        {
		gchar buffer[BSIZE];
		if(fd) fclose(fd);
		sprintf(buffer,_("I cannot read dielectric function from the VASP xml file\nCheck your '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
		g_free(option);
                return FALSE;
        }
	g_free(option);
	OK = read_lists_vasp_xml_file(fd,  "<imag", direction, &listEnergies, &listEpsiImag);
	if(!OK) messageErrorFreq(fileName);
	if(OK)
	{
		GList* listEnergies = NULL;
		OK = read_lists_vasp_xml_file(fd,  "<real", direction, &listEnergies, &listEpsiReal);
		if(OK) createVASPSpectraWin("Dielectric function",listEnergies, listEpsiReal,listEpsiImag,"Frequency(eV)","Dielectric function");
		else messageErrorFreq(fileName);
		free_list(listEnergies);
	}
	//gchar* functions[] = {"Absorption coefficient","Refractive index","Extinction coefficient","Reflectivity","Conductivity","Energy loss"};
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_functions[0])))
	{
		listAlpha = computeAbsorptionCoefficient(listEnergies, listEpsiReal, listEpsiImag);
		if(listAlpha) createVASPSpectraWin("Absorption coefficient",listEnergies, listAlpha,NULL,"Frequency(eV)","Absorption function(cm<sup>-1</sup>)");
	}
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_functions[1])))
	{
		listRefractiveIndex = computeRefractiveIndex(listEpsiReal, listEpsiImag);
		if(listRefractiveIndex) createVASPSpectraWin("Refractive index",listEnergies, listRefractiveIndex,NULL,"Frequency(eV)","Refractive index");
	}
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_functions[2])))
	{
		listExtinctionCoefficient = computeExtinctionCoefficient(listEpsiReal, listEpsiImag);
		if(listExtinctionCoefficient) createVASPSpectraWin("Extinction coefficient",listEnergies, listExtinctionCoefficient,NULL,"Frequency(eV)","Extinction coefficient");
	}
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_functions[3])))
	{
		listReflectivity = computeReflectivity(listRefractiveIndex, listExtinctionCoefficient);
		if(listExtinctionCoefficient) createVASPSpectraWin("Reflectivity",listEnergies, listReflectivity,NULL,"Frequency(eV)","Reflectivity");
	}
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_functions[4])))
	{
		listSigmaReal = computeConductivityReal(listEnergies, listEpsiReal, listEpsiImag);
		listSigmaImag = computeConductivityImag(listEnergies, listEpsiReal, listEpsiImag);
		if(listSigmaReal && listSigmaImag) createVASPSpectraWin("Conductivity",listEnergies, listSigmaReal,listSigmaImag,"Frequency(eV)","Conductivity(1/fs)");
	}
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(toggle_functions[5])))
	{
		listEnergyLoss = computeEnergyLossFunction(listEpsiReal, listEpsiImag);
		if(listEnergyLoss) createVASPSpectraWin("Energy loss",listEnergies, listEnergyLoss,NULL,"Frequency(eV)","Loss function");
	}

	free_list(listEnergies);
	free_list(listEpsiReal);
	free_list(listEpsiImag);
	free_list(listAlpha);
	free_list(listRefractiveIndex);
	free_list(listExtinctionCoefficient);
	free_list(listReflectivity);
	free_list(listEnergyLoss);
	free_list(listSigmaReal);
	free_list(listSigmaImag);
	fclose(fd);
	return TRUE;

}

/********************************************************************************/
static gboolean apply_dielectric_vasp(GtkWidget *window,gpointer data)
{
	if(!window) printf("window = NULL\n");
	gint direction = 0;
	gchar* fileName  = (gchar*)g_object_get_data(G_OBJECT(window), "FileName");
	GtkWidget* entry_options= g_object_get_data(G_OBJECT (window), "EntryOptions");
	GtkWidget* entry_directions= g_object_get_data(G_OBJECT (window), "EntryDirections");
	if(!entry_options) printf("entry_options= = NULL\n");
	GtkWidget** toggle_functions =  g_object_get_data(G_OBJECT (window), "ToggleFunctions");
	if(!toggle_functions) printf("toggle_functions= = NULL\n");

	read_dielectric_vasp_xml_file(fileName, entry_options, entry_directions, toggle_functions);

	gtk_widget_destroy(window);
	return TRUE;
}
/********************************************************************************/
static gboolean create_dielectric_vasp_xml_win_dlg(gchar* fileName, GList* lists)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* table = NULL;
	GtkWidget* tmp_label = NULL;
	GtkWidget* vbox = NULL;
	gint i;
	gint nOptions = 0;
	gchar** listOptions = NULL;
	GtkWidget* entry_options = NULL;
	GtkWidget* hseparator = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* vbox_window = NULL;
	GtkWidget* frame = NULL;
	GList* cOption = NULL;
	gchar* functions[] = {"Absorption coefficient","Refractive index","Extinction coefficient","Reflectivity","Conductivity","Energy loss"};
	gint nFunctions = sizeof (functions)/sizeof(functions[0]);
	gchar* directions[] = {"XX","YY","ZZ","XY","XZ","YZ","XYZ"};
	gchar** listDirections = NULL;
	gint nDirections = sizeof (directions)/sizeof(directions[0]);
	GtkWidget* entry_directions = NULL;
	GtkWidget* button;
	gint il = 0;

	GtkWidget** toggle_functions = g_malloc(nFunctions*sizeof(GtkWidget*));


	nOptions = 0;
        for(cOption = g_list_first(lists); cOption != NULL; cOption = cOption->next) nOptions++;
	if(nOptions<1) return FALSE;
	listOptions = g_malloc(nOptions*sizeof(gchar*));
	i = 0;
        for(cOption = g_list_first(lists); cOption != NULL; cOption = cOption->next)
        {
               gchar* data = (gchar*)cOption->data;
               listOptions[i] = g_strdup(data);
               i++;
        }

	listDirections = g_malloc(nDirections*sizeof(gchar*));
	for(i=0;i<nDirections;i++) listDirections[i] = g_strdup(directions[i]);
	
	gtk_window_set_title (GTK_WINDOW (window), "VASP dielectric function");
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", G_CALLBACK (gtk_widget_destroy), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);


        vbox_window=gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(window), vbox_window);
        gtk_widget_show(vbox_window);


        frame = gtk_frame_new(NULL);
        gtk_box_pack_start(GTK_BOX(vbox_window), frame, TRUE, FALSE, 2);
        gtk_widget_show(frame);

        vbox=gtk_vbox_new(FALSE, 0);
        gtk_container_add(GTK_CONTAINER(frame), vbox);
        gtk_widget_show(vbox);
	
	table=gtk_table_new(8, 3, FALSE);
	gtk_container_add(GTK_CONTAINER(vbox), table);
	gtk_widget_show(table);

	il = 0;
	entry_options = create_label_combo_in_table(table,_("Type"),listOptions, nOptions, TRUE,-1,600,il);
	gtk_widget_show(entry_options);

	il++;
	entry_directions = create_label_combo_in_table(table,_("Direction"), listDirections, nDirections, FALSE,-1,600,il);
	gtk_widget_show(entry_directions);

	il++;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table), hseparator, 0, 3, il, il+1, (GtkAttachOptions)(GTK_FILL|GTK_EXPAND), (GtkAttachOptions)(GTK_FILL|GTK_SHRINK), 1, 1);
	gtk_widget_show(hseparator);


	for(i=0; i<nFunctions;i++)
        {
		il++;
                toggle_functions[i] = gtk_check_button_new_with_label(_(functions[i]));
		gtk_table_attach(GTK_TABLE(table), toggle_functions[i], 0, 3, il, il+1, GTK_FILL, GTK_FILL, 1, 1);
                gtk_widget_show(toggle_functions[i]);
        }
	gtk_widget_show_all(table);

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
	g_signal_connect_swapped(G_OBJECT(button), "clicked",G_CALLBACK(apply_dielectric_vasp),GTK_OBJECT(window));
	gtk_widget_show (window);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_functions[0]), TRUE);

	g_object_set_data(G_OBJECT (window), "EntryOptions", entry_options);
	g_object_set_data(G_OBJECT (window), "EntryDirections", entry_directions);
	g_object_set_data(G_OBJECT (window), "ToggleFunctions", toggle_functions);
	g_object_set_data(G_OBJECT (window), "FileName", g_strdup(fileName));

	if(listOptions) for(i=0; i<nOptions;i++) if(listOptions[i]) g_free(listOptions[i]);
	if(listOptions) g_free(listOptions);
	if(listDirections) for(i=0; i<nDirections;i++) if(listDirections[i]) g_free(listDirections[i]);
	if(listDirections) g_free(listDirections);

	return TRUE;
}
/********************************************************************************/
static gboolean read_dielectric_vasp_xml_file_dlg(GabeditFileChooser *SelecFile, gint response_id)
{
	gchar *fileName;
 	gchar t[BSIZE];
 	FILE *fd;
	GList* lists= NULL;
	gchar* pos = NULL;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gabedit_file_chooser_get_current_file(SelecFile);

 	fd = FOpen(fileName, "rb");
	while(!feof(fd))
        {
                if(!fgets(t,BSIZE,fd)) break;
                if(strstr(t,"<dielectricfunction"))
                {
			if(strstr(t,"<dielectricfunction>"))
			{
				lists=g_list_append(lists, (gpointer) g_strdup("<dielectricfunction>"));
			}
			else
			{
				gint i;
				for(i=0;i<strlen(t);i++) if(t[i]=='\n') t[i] = '\0';
				pos = NULL;
				if(strstr(t,"=")) pos = strstr(t,"=")+1;
				for(i=strlen(t)-1;i>=0;i--) { if(t[i]=='>') t[i] = '\0'; break;}
				if(pos) lists=g_list_append(lists, (gpointer) g_strdup(pos));
			}
		}
	}
	if(lists==NULL)
        {
		gchar buffer[BSIZE];
		if(fd) fclose(fd);
		sprintf(buffer,_("I cannot read dielectric function from the VASP xml file\nCheck your '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
                return FALSE;
        }
	fclose(fd);
	create_dielectric_vasp_xml_win_dlg(fileName,lists);
	free_list(lists);
	return TRUE;

}
/********************************************************************************/
void read_vasp_xml_file_dlg()
{
	GtkWidget* filesel = 
 	file_chooser_open(read_dielectric_vasp_xml_file_dlg,
			_("Read dielectric function from a vasp.xml file"),
			GABEDIT_TYPEFILE_XML,GABEDIT_TYPEWIN_OTHER);

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
}
