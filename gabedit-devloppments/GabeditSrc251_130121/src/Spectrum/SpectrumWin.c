/* SpectrumWin.c */
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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>
#include "../Common/Global.h"
#include "../Utils/GabeditXYPlot.h"
#include "SpectrumWin.h"

/****************************************************************************************/
static gdouble get_ymax(XYPlotData* dataCurve)
{
	gdouble ymax = 1;
	gint loop;
	if(dataCurve->size<1) return 1.0;
	ymax = dataCurve->y[0];
	for (loop=1; loop<dataCurve->size; loop++)
		if(ymax<dataCurve->y[loop]) ymax = dataCurve->y[loop];
	return ymax;
}
/****************************************************************************************/
static gdouble get_ymin(XYPlotData* dataCurve)
{
	gdouble ymin = 1;
	gint loop;
	if(dataCurve->size<1) return 0.0;
	ymin = dataCurve->y[0];
	for (loop=1; loop<dataCurve->size; loop++)
		if(ymin<dataCurve->y[loop]) ymin = dataCurve->y[loop];
	return ymin;
}
/****************************************************************************************/
static void set_ymax_to_one(XYPlotData* dataCurve, XYPlotData* dataPeaks)
{
	gdouble ymax = 0;
	gint loop;
	if(dataCurve->size<1) return;
	ymax = dataCurve->y[0];
	for (loop=1; loop<dataCurve->size; loop++)
		if(ymax<dataCurve->y[loop]) ymax = dataCurve->y[loop];
	if(ymax!=0)
	for (loop=0; loop<dataCurve->size; loop++)
		dataCurve->y[loop] /= ymax;
	if(ymax!=0)
	for (loop=0; loop<dataPeaks->size; loop++)
		dataPeaks->y[loop] /= ymax;
}
/****************************************************************************************/
static void build_data_xyplot_curve_noconv(XYPlotWinData* winData, XYPlotData* dataCurve)
{
	gint loop;
	gint line_width = winData->line_width;
	gint point_size = winData->point_size;
	GdkColor line_color = winData->line_color;
	GdkColor point_color = winData->point_color;
	
	if(dataCurve->x && dataCurve->y)
	{
		line_width = dataCurve->line_width;
		point_size = dataCurve->point_size;
		line_color = dataCurve->line_color;
		point_color = dataCurve->point_color;
	}

	dataCurve->size=winData->size;
	if(dataCurve->size>0)
	{
		if(dataCurve->x) g_free(dataCurve->x);
		if(dataCurve->y) g_free(dataCurve->y);
		dataCurve->x = (gdouble*)g_malloc(sizeof(gdouble)*dataCurve->size);
		dataCurve->y = (gdouble*)g_malloc(sizeof(gdouble)*dataCurve->size);
	}
	
	for (loop=0; loop<dataCurve->size; loop++){
		dataCurve->x[loop]=winData->x[loop]*winData->scaleX+winData->shiftX;
		dataCurve->y[loop]=winData->y[loop]*winData->scaleY;
	}
	sprintf(dataCurve->point_str,"+");
	dataCurve->point_size=point_size;
	dataCurve->line_width=line_width;
	dataCurve->point_color=point_color; 
	dataCurve->line_color=line_color; 

	dataCurve->line_style=winData->line_style; 

}
/********************************************************************************/
static gdouble lorentzianLineshape(gdouble rel_offset)
{
	return 1.0 / (1.0 + rel_offset * rel_offset);
}
/********************************************************************************/
static gdouble gaussianLineshape(gdouble rel_offset)
{
  gdouble nln2 = -log(2.0);
  return exp(nln2 * rel_offset * rel_offset);
}
/****************************************************************************************/
static void build_data_xyplot_curve_withconv(XYPlotWinData* winData, XYPlotData* dataCurve)
{
	gint i;
	gint j;
	gdouble (*lineshape)(gdouble);
	gint line_width = winData->line_width;
	gint point_size = winData->point_size;
	GdkColor line_color = winData->line_color;
	GdkColor point_color = winData->point_color;
	gint n = 0;
        gdouble xx = winData->xmin;
        gdouble h0 = winData->halfWidth/20;
	
	if(dataCurve->x && dataCurve->y)
	{
		line_width = dataCurve->line_width;
		point_size = dataCurve->point_size;
		line_color = dataCurve->line_color;
		point_color = dataCurve->point_color;
	}

	if(winData->convType==GABEDIT_CONV_TYPE_LORENTZ)
	       lineshape = lorentzianLineshape;
	else
	       lineshape = gaussianLineshape;


	
	if(dataCurve->size>0 && winData->size)
	{
		if(dataCurve->x) g_free(dataCurve->x);
		if(dataCurve->y) g_free(dataCurve->y);
		dataCurve->x = NULL;
		dataCurve->y = NULL;
	}

	dataCurve->size=0;
        xx = winData->xmin;
        h0 = winData->halfWidth/10;
	n = (gint)((winData->xmax-winData->xmin)/h0+0.5)+winData->size;
	if(n>0) dataCurve->x = (gdouble*)g_malloc(sizeof(gdouble)*n);
	if(winData->size>0 && n>0)
	do
	{
		gdouble dmin = 0.0;
		gdouble d = 0.0;
		gint jmin = 0;
		for (j=0; j < winData->size; j++)
		{
			gdouble center = (gdouble) winData->x[j]*winData->scaleX+winData->shiftX;
			
			d = fabs(xx - center);
			if(d<dmin || j==0) 
			{
				jmin = j;
				dmin = d;
			}
		}
		dataCurve->x[dataCurve->size] = xx;
		if(dmin<h0)
		{
			if(xx< winData->x[jmin]*winData->scaleX+winData->shiftX)
			{
				xx = (gdouble) winData->x[jmin]*winData->scaleX+winData->shiftX;
				dataCurve->x[dataCurve->size] = xx;
				xx += h0+1e-8;
			}
			else
			{
				xx = (gdouble) winData->x[jmin]*winData->scaleX+winData->shiftX;
				dataCurve->x[dataCurve->size] = xx;
				xx += h0+1e-8;
			}
		}
		else
		{
			if(dmin> 5*winData->halfWidth) xx += h0+dmin/5;
			else xx += h0;
		}

		dataCurve->size++;
	}while(xx<winData->xmax && dataCurve->size<n);

	if(dataCurve->size>0) 
	{
		dataCurve->x = (gdouble*)g_realloc(dataCurve->x,sizeof(gdouble)*dataCurve->size);
		dataCurve->y = (gdouble*)g_malloc(sizeof(gdouble)*dataCurve->size);
	}

	if(winData->size>0)
	for (i=0; i < dataCurve->size; i++)
	{
		gdouble yy = 0.0;
		for (j=0; j < winData->size; j++)
		{
			gdouble center = (gdouble) winData->x[j]*winData->scaleX+winData->shiftX;
			gdouble rel_offset = (dataCurve->x[i] - center) / winData->halfWidth;
			yy += winData->y[j]*lineshape(rel_offset)*winData->scaleY;
		}
		dataCurve->y[i] = yy;
	}

	sprintf(dataCurve->point_str,"+");
	dataCurve->point_size=point_size;
	dataCurve->line_width=line_width;
	dataCurve->point_color=point_color; 

	dataCurve->line_color=line_color; 

	dataCurve->line_style=winData->line_style; 


}
/****************************************************************************************/
static void build_data_xyplot_curve(XYPlotWinData* winData, XYPlotData* dataCurve)
{
	switch(winData->convType)
	{
		case GABEDIT_CONV_TYPE_NONE :
			build_data_xyplot_curve_noconv(winData, dataCurve);
			break;
		case GABEDIT_CONV_TYPE_LORENTZ :
		case GABEDIT_CONV_TYPE_GAUSS :
			build_data_xyplot_curve_withconv(winData, dataCurve);
	}

}
/****************************************************************************************/
static void build_data_xyplot_peaks(XYPlotWinData* winData, XYPlotData* dataPeaks)
{
	gint loop;
	gint line_width = winData->line_width;
	gint point_size = winData->point_size;
	GdkColor line_color = winData->line_color;
	GdkColor point_color = winData->point_color;
	
	line_color.red *=0.9; 
	line_color.green *=0.9; 
	line_color.blue *=0.9; 
	point_color.red *=0.9; 
	point_color.green *=0.9; 
	point_color.blue *=0.9; 
	if(dataPeaks->x && dataPeaks->y)
	{
		line_width = dataPeaks->line_width;
		point_size = dataPeaks->point_size;
		line_color = dataPeaks->line_color;
		point_color = dataPeaks->point_color;
	}

	dataPeaks->size=3*winData->size+2;
	if(dataPeaks->size>0)
	{
		if(dataPeaks->x) g_free(dataPeaks->x);
		if(dataPeaks->y) g_free(dataPeaks->y);
		dataPeaks->x = (gdouble*)g_malloc(sizeof(gdouble)*dataPeaks->size);
		dataPeaks->y = (gdouble*)g_malloc(sizeof(gdouble)*dataPeaks->size);
	}
	
     
	dataPeaks->x[0]=winData->xmin;
	/* dataPeaks->y[0]=winData->ymin;*/
	dataPeaks->y[0]=0;
	dataPeaks->x[dataPeaks->size-1]=winData->xmax;
	/* dataPeaks->y[dataPeaks->size-1]=winData->ymin;*/
	dataPeaks->y[dataPeaks->size-1]=0;
	for (loop=0; loop<winData->size; loop++){
		gint iold = loop*3+1;
		gdouble xx = winData->x[loop]*winData->scaleX+winData->shiftX;
		dataPeaks->x[iold]=xx;
		/* dataPeaks->y[iold]=winData->ymin;*/
		dataPeaks->y[iold]=0;

		dataPeaks->x[iold+1]=xx;
		dataPeaks->y[iold+1]=winData->y[loop]*winData->scaleY;

		dataPeaks->x[iold+2]=xx;
		/* dataPeaks->y[iold+2]=winData->ymin;*/
		dataPeaks->y[iold+2]=0;
	}

	sprintf(dataPeaks->point_str,"+");
	dataPeaks->point_size=point_size;
	dataPeaks->line_width=line_width;
	dataPeaks->point_color=point_color; 

	dataPeaks->line_color=line_color; 

	dataPeaks->line_style=winData->line_style; 
}
/****************************************************************************************/
static void build_data_xyplot(XYPlotWinData* winData)
{
	XYPlotData* dataPeaks = NULL;
	XYPlotData* dataCurve = NULL;
	if(!winData->showDataPeaks && !winData->showDataCurve) return;
	if(!winData->dataPeaks)
	{
		winData->dataPeaks = (XYPlotData*)g_malloc(sizeof(XYPlotData));
		winData->dataPeaks->size = 0;
		winData->dataPeaks->x = NULL;
		winData->dataPeaks->y = NULL;
	}
	if(!winData->dataCurve) 
	{
		winData->dataCurve = (XYPlotData*)g_malloc(sizeof(XYPlotData));
		winData->dataCurve->size = 0;
		winData->dataCurve->x = NULL;
		winData->dataCurve->y = NULL;
	}

	dataPeaks = winData->dataPeaks;
	dataCurve = winData->dataCurve;

	build_data_xyplot_curve(winData, dataCurve);
	build_data_xyplot_peaks(winData, dataPeaks);
	if(winData->ymaxToOne) 
		set_ymax_to_one(dataCurve, dataPeaks);
}
/****************************************************************************************/
static XYPlotWinData* get_win_data(GabeditXYPlot *xyplot, gint size, gdouble* x, gdouble* y)
{
	XYPlotWinData* winData = (XYPlotWinData*)g_malloc(sizeof(XYPlotWinData));
	gint loop;
	winData->showDataPeaks = FALSE;
	winData->showDataCurve = TRUE;
	winData->dataPeaks = NULL;
	winData->dataCurve = NULL;
	

	winData->size=size;
	winData->x = NULL;
	winData->y = NULL;
	gabedit_xyplot_get_range (GABEDIT_XYPLOT(xyplot), &(winData->xmin), &(winData->xmax), &(winData->ymin), &(winData->ymax));
	if(size>0)
	{
		winData->x = (gdouble*) g_malloc(sizeof(gdouble)*size);
		winData->y = (gdouble*) g_malloc(sizeof(gdouble)*size);
		for(loop=0;loop<size;loop++)
		{
			winData->x[loop] = x[loop];
			winData->y[loop] = y[loop];
		}
	}
	winData->point_size=0;
	winData->line_width=1;

	winData->point_color.red=0;
	winData->point_color.green=0;
	winData->point_color.blue=0;

	winData->line_color.red=65000;
	winData->line_color.green=0;
	winData->line_color.blue=0;

 	winData->line_style=GDK_LINE_SOLID;

 	winData->halfWidth = fabs(winData->xmax-winData->xmin)/30;
 	winData->convType = GABEDIT_CONV_TYPE_LORENTZ;
 	winData->scaleX = 1;
 	winData->scaleY = 1;
 	winData->shiftX = 0;
 	winData->ymaxToOne = FALSE;


	build_data_xyplot(winData);

	return winData;

}
/****************************************************************************************/
static void gabedit_xyplot_add_windata(GabeditXYPlot *xyplot, XYPlotWinData* data)
{
	GList* data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");
  	if (!data_list || g_list_find (data_list, (gpointer)data)==NULL)
	{
		data_list=g_list_append(data_list, (gpointer) data);
		g_object_set_data(G_OBJECT (xyplot), "DataList", data_list);
	}
}
/****************************************************************************************/
static void gabedit_xyplot_show_curve_peaks(GabeditXYPlot *xyplot)
{
	GList* data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");
	GList* current = NULL;
	XYPlotWinData* data;
	if(!data_list) return;
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		if(data->showDataPeaks) gabedit_xyplot_add_data(xyplot, data->dataPeaks);
		if(data->showDataCurve) gabedit_xyplot_add_data(xyplot, data->dataCurve);
	}
}
/****************************************************************************************/
static void gabedit_xyplot_refresh_dat(GabeditXYPlot *xyplot)
{
	GList* data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");
	GList* current = NULL;
	XYPlotWinData* data = NULL;
	if(!data_list) return;
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		if(data->convType!=GABEDIT_CONV_TYPE_NONE)
		{
			gabedit_xyplot_get_range (GABEDIT_XYPLOT(xyplot), &(data->xmin), &(data->xmax), &(data->ymin), &(data->ymax));
			build_data_xyplot(data);
		}
	}
}
/****************************************************************************************/
static void toggle_no_convolution_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	GtkWidget* xyplot = NULL;
	GList* data_list = NULL;
	GList* current = NULL;
	XYPlotWinData* data;


	if(!user_data || !G_IS_OBJECT(user_data)) return;
	if(!gtk_toggle_button_get_active(togglebutton)) return;

	xyplot = GTK_WIDGET(user_data);
	data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");

	if(!data_list) return;
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		if(data->convType!=GABEDIT_CONV_TYPE_NONE)
		{
			data->convType=GABEDIT_CONV_TYPE_NONE;
			build_data_xyplot(data);
		}
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/********************************************************************************/
static void toggle_lorentzian_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	GtkWidget* xyplot = NULL;
	GList* data_list = NULL;
	GList* current = NULL;
	XYPlotWinData* data;


	if(!user_data || !G_IS_OBJECT(user_data)) return;
	if(!gtk_toggle_button_get_active(togglebutton)) return;

	xyplot = GTK_WIDGET(user_data);
	data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");

	if(!data_list) return;
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		if(data->convType!=GABEDIT_CONV_TYPE_LORENTZ)
		{
			data->convType=GABEDIT_CONV_TYPE_LORENTZ;
			build_data_xyplot(data);
		}
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/********************************************************************************/
static void toggle_gaussian_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	GtkWidget* xyplot = NULL;
	GList* data_list = NULL;
	GList* current = NULL;
	XYPlotWinData* data;


	if(!user_data || !G_IS_OBJECT(user_data)) return;
	if(!gtk_toggle_button_get_active(togglebutton)) return;

	xyplot = GTK_WIDGET(user_data);
	data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");

	if(!data_list) return;
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		if(data->convType!=GABEDIT_CONV_TYPE_GAUSS)
		{
			data->convType=GABEDIT_CONV_TYPE_GAUSS;
			build_data_xyplot(data);
		}
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/********************************************************************************/
static void toggle_show_peaks_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	GtkWidget* xyplot = NULL;
	GList* data_list = NULL;
	GList* current = NULL;
	XYPlotWinData* data;
	gboolean showPeaks = FALSE;

	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");

	if(!data_list) return;

	showPeaks = gtk_toggle_button_get_active(togglebutton);
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		gabedit_xyplot_remove_data(GABEDIT_XYPLOT(xyplot), (gpointer)data->dataPeaks);
		if(data->showDataPeaks!=showPeaks)
		{
			data->showDataPeaks=showPeaks;
			build_data_xyplot(data);
			if(data->showDataPeaks)
				gabedit_xyplot_add_data(GABEDIT_XYPLOT(xyplot), (gpointer)data->dataPeaks);
		}
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));

}
/********************************************************************************/
static void toggle_ymax_to_one_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	GtkWidget* xyplot = NULL;
	GList* data_list = NULL;
	GList* current = NULL;
	XYPlotWinData* data;
	gboolean ymaxToOne = FALSE;
	gdouble ymin = 0;
	gdouble ymax = 0;
	gdouble a;

	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");

	if(!data_list) return;

	ymaxToOne = gtk_toggle_button_get_active(togglebutton);
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		gabedit_xyplot_remove_data(GABEDIT_XYPLOT(xyplot), (gpointer)data->dataCurve);
		if(data->ymaxToOne!=ymaxToOne)
		{
			data->ymaxToOne=ymaxToOne;
			build_data_xyplot(data);
			gabedit_xyplot_add_data(GABEDIT_XYPLOT(xyplot), (gpointer)data->dataCurve);
			a =get_ymin(data->dataCurve);
			if(ymin>a) ymin = a;
			a =get_ymax(data->dataCurve);
			if(ymax<a) ymax = a;

		}
	}
	if(ymaxToOne)
	{
		ymin = 0.0;
		ymax = 1.0;
	}
	{
		gabedit_xyplot_set_range_ymin (GABEDIT_XYPLOT(xyplot), ymin);
		gabedit_xyplot_set_range_ymax (GABEDIT_XYPLOT(xyplot), ymax);
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));

}
/****************************************************************************************/
static gboolean xyplot_motion_notify_event(GtkWidget *xyplot, GdkEventMotion *event, gpointer user_data)
{
	double xv, yv;
	int x, y;
	char str[50];
	int context_id;
	GtkWidget* statusbar = g_object_get_data(G_OBJECT (xyplot), "StatusBar");

	x=event->x;
	y=event->y;

	if (event->is_hint || (event->window != xyplot->window))
		gdk_window_get_pointer (xyplot->window, &x, &y, NULL);

	if(gabedit_xyplot_get_point(GABEDIT_XYPLOT(xyplot), x, y, &xv, &yv))
	snprintf(str, 50, _("Mouse position: %lf, %lf"), xv, yv);
	else
	sprintf(str, " ");
	context_id=gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "mouse position");
	gtk_statusbar_push (GTK_STATUSBAR(statusbar), context_id, str);
    
	return TRUE;
}
/********************************************************************************/
static void activate_entry_xmin(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	gdouble a;
	GtkWidget* xyplot = NULL;
	gdouble xmin;
	gdouble ymin;
	gdouble xmax;
	gdouble ymax;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	a = atof(t);
	gabedit_xyplot_get_range (GABEDIT_XYPLOT(xyplot), &xmin, &xmax, &ymin, &ymax);

	if(a>= xmax)
	{
		gchar* tmp = g_strdup_printf("%0.3f",xmin);
		gtk_entry_set_text(GTK_ENTRY(entry),tmp);
		g_free(tmp);
		return;
	}
	gabedit_xyplot_set_range_xmin (GABEDIT_XYPLOT(xyplot), a);
	gabedit_xyplot_refresh_dat(GABEDIT_XYPLOT(xyplot));
}
/********************************************************************************/
static void activate_entry_xmax(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	gdouble a;
	GtkWidget* xyplot = NULL;
	gdouble xmin;
	gdouble ymin;
	gdouble xmax;
	gdouble ymax;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	a = atof(t);
	gabedit_xyplot_get_range (GABEDIT_XYPLOT(xyplot), &xmin, &xmax, &ymin, &ymax);

	if(a<=xmin)
	{
		gchar* tmp = g_strdup_printf("%0.3f",xmax);
		gtk_entry_set_text(GTK_ENTRY(entry),tmp);
		g_free(tmp);
		return;
	}
	gabedit_xyplot_set_range_xmax (GABEDIT_XYPLOT(xyplot), a);
	gabedit_xyplot_refresh_dat(GABEDIT_XYPLOT(xyplot));
}
/********************************************************************************/
static void activate_entry_half_width(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	gdouble a;
	GtkWidget* xyplot = NULL;
	gdouble xmin;
	gdouble ymin;
	gdouble xmax;
	gdouble ymax;
	GList* data_list = NULL;
	GList* current = NULL;
	XYPlotWinData* data;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	a = atof(t);
	gabedit_xyplot_get_range (GABEDIT_XYPLOT(xyplot), &xmin, &xmax, &ymin, &ymax);

	if(a<= 0)
	{
		gchar* tmp = g_strdup_printf("%0.3f",fabs(xmax-xmin)/30);
		gtk_entry_set_text(GTK_ENTRY(entry),tmp);
		g_free(tmp);
		return;
	}
	data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");
	if(!data_list) return;
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		data->halfWidth = a;
		if(data->convType!=GABEDIT_CONV_TYPE_NONE)
		{
			build_data_xyplot(data);
		}
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/********************************************************************************/
static void activate_entry_scale_x(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	gdouble a;
	GtkWidget* xyplot = NULL;
	gdouble xmin;
	gdouble ymin;
	gdouble xmax;
	gdouble ymax;
	GList* data_list = NULL;
	GList* current = NULL;
	XYPlotWinData* data;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	a = atof(t);
	gabedit_xyplot_get_range (GABEDIT_XYPLOT(xyplot), &xmin, &xmax, &ymin, &ymax);

	if(a<= 0)
	{
		gchar* tmp = g_strdup_printf("%0.3f",1.0);
		gtk_entry_set_text(GTK_ENTRY(entry),tmp);
		g_free(tmp);
		return;
	}
	data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");
	if(!data_list) return;
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		data->scaleX = a;
		build_data_xyplot(data);
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/********************************************************************************/
static void activate_entry_scale_y(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	gdouble a;
	GtkWidget* xyplot = NULL;
	gdouble xmin;
	gdouble ymin;
	gdouble xmax;
	gdouble ymax;
	GList* data_list = NULL;
	GList* current = NULL;
	XYPlotWinData* data;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	a = atof(t);
	gabedit_xyplot_get_range (GABEDIT_XYPLOT(xyplot), &xmin, &xmax, &ymin, &ymax);

	if(a<= 0)
	{
		gchar* tmp = g_strdup_printf("%0.3f",1.0);
		gtk_entry_set_text(GTK_ENTRY(entry),tmp);
		g_free(tmp);
		return;
	}
	data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");
	if(!data_list) return;
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		data->scaleY = a;
		build_data_xyplot(data);
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/********************************************************************************/
static void activate_entry_shift_x(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	gdouble a;
	GtkWidget* xyplot = NULL;
	gdouble xmin;
	gdouble ymin;
	gdouble xmax;
	gdouble ymax;
	GList* data_list = NULL;
	GList* current = NULL;
	XYPlotWinData* data;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	a = atof(t);
	gabedit_xyplot_get_range (GABEDIT_XYPLOT(xyplot), &xmin, &xmax, &ymin, &ymax);

	data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");
	if(!data_list) return;
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		data->shiftX = a;
		build_data_xyplot(data);
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/*************************************************************************************/
static void gabedit_xyplot_autorange(GabeditXYPlot *xyplot)
{
	GList* data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");
	GList* current = NULL;
	XYPlotWinData* data;
	GtkWidget *entry_half_width = g_object_get_data(G_OBJECT (xyplot), "EntryHalfWidth");
	GtkWidget *entry_scale_x = g_object_get_data(G_OBJECT (xyplot), "EntryScaleX");
	GtkWidget *entry_scale_y = g_object_get_data(G_OBJECT (xyplot), "EntryScaleY");
	GtkWidget *entry_shift_x = g_object_get_data(G_OBJECT (xyplot), "EntryShiftX");
	GtkWidget*entry_x_min = g_object_get_data(G_OBJECT (xyplot), "EntryXMin");
	GtkWidget*entry_x_max = g_object_get_data(G_OBJECT (xyplot), "EntryXMax");

	if(!data_list) return;
	current=g_list_first(data_list);
	if(!current) return;
	data = (XYPlotWinData*)current->data;
	if(data->dataCurve) gabedit_xyplot_set_autorange (xyplot, data->dataCurve);
	if(data->dataPeaks) gabedit_xyplot_set_autorange (xyplot, data->dataPeaks);
	if(data->convType!=GABEDIT_CONV_TYPE_NONE && (data->dataCurve ||data->dataPeaks ))
	{
		gchar tmp[100];
		gabedit_xyplot_get_range (GABEDIT_XYPLOT(xyplot), &(data->xmin), &(data->xmax), &(data->ymin), &(data->ymax));

		sprintf(tmp,"%0.3f",data->xmin);
		gtk_entry_set_text(GTK_ENTRY(entry_x_min),tmp);
		sprintf(tmp,"%0.3f",data->xmax);
		gtk_entry_set_text(GTK_ENTRY(entry_x_max),tmp);

		gabedit_xyplot_set_range_ymin (GABEDIT_XYPLOT(xyplot), data->ymin);
		gabedit_xyplot_set_range_ymin (GABEDIT_XYPLOT(xyplot), data->ymax);

		data->halfWidth = fabs(data->xmax-data->xmin)/30;

		build_data_xyplot(data);

		sprintf(tmp,"%0.3f",data->halfWidth);
		gtk_entry_set_text(GTK_ENTRY(entry_half_width),tmp);

		sprintf(tmp,"%0.3f",data->scaleX);
		gtk_entry_set_text(GTK_ENTRY(entry_scale_x),tmp);

		sprintf(tmp,"%0.3f",data->scaleY);
		gtk_entry_set_text(GTK_ENTRY(entry_scale_y),tmp);

		sprintf(tmp,"%0.3f",data->shiftX);
		gtk_entry_set_text(GTK_ENTRY(entry_shift_x),tmp);

	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/****************************************************************************************/
void spectrum_win_add_data(GtkWidget *winXYPlot, gint size, gdouble* x, gdouble* y)
{
	GabeditXYPlot *xyplot = g_object_get_data(G_OBJECT (winXYPlot), "XYPLOT");
	XYPlotWinData* winData = NULL;
	if(!xyplot || !G_IS_OBJECT(xyplot)) return;
	winData = get_win_data(xyplot, size, x, y);
	gabedit_xyplot_add_windata(xyplot, winData);
	gabedit_xyplot_refresh_dat(GABEDIT_XYPLOT(xyplot));
	gabedit_xyplot_show_curve_peaks(GABEDIT_XYPLOT(xyplot));
}
/****************************************************************************************/
void spectrum_win_autorange(GtkWidget *winXYPlot)
{
	GabeditXYPlot *xyplot = g_object_get_data(G_OBJECT (winXYPlot), "XYPLOT");
	if(!xyplot || !G_IS_OBJECT(xyplot)) return;
	gabedit_xyplot_autorange(xyplot);
}
/****************************************************************************************/
static void free_data(GtkWidget *window)
{
	GtkWidget* xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
	GList* data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");
	GList* current = NULL;
	XYPlotWinData* data = NULL;
	if(!data_list) return;
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		if(data)
		{
			if(data->dataCurve) gabedit_xyplot_remove_data(GABEDIT_XYPLOT(xyplot), (gpointer)data->dataCurve);
			if(data->dataPeaks) gabedit_xyplot_remove_data(GABEDIT_XYPLOT(xyplot), (gpointer)data->dataPeaks);
			if(data->dataCurve && data->dataCurve->x) g_free(data->dataCurve->x);
			if(data->dataCurve && data->dataCurve->y) g_free(data->dataCurve->y);
			if(data->dataCurve) g_free(data->dataCurve);
			if(data->dataPeaks && data->dataPeaks->x) g_free(data->dataPeaks->x);
			if(data->dataPeaks && data->dataPeaks->y) g_free(data->dataPeaks->y);
			if(data->dataPeaks) g_free(data->dataPeaks);
			if(data->x) g_free(data->x);
			if(data->y) g_free(data->y);
		}
		g_free(data);
	}
	g_object_set_data(G_OBJECT (xyplot), "DataList",NULL);
}
/****************************************************************************************/
void spectrum_win_remove_data(GtkWidget *window)
{
	free_data(window);
}
/****************************************************************************************/
static void destroy_spectrum_win(GtkWidget *window, gpointer data)
{
	free_data(window);
	gtk_widget_destroy(window);
}
/****************************************************************************************/
GtkWidget* spectrum_win_new(gchar* title)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* table1 = NULL;
	GtkWidget* table2 = NULL;
	GtkWidget* frame_xyplot = NULL;
	GtkWidget* xyplot = NULL;
	GtkWidget* tmp_label = NULL;
	GtkWidget* vbox = NULL;

	GtkWidget* entry_x_min = NULL;
	GtkWidget* entry_x_max = NULL;

	GtkWidget *statusbar = NULL;

	GtkWidget *frame_set_data = NULL;
	GtkWidget *toggle_no_convolution = NULL;
	GtkWidget *toggle_lorentzian = NULL;
	GtkWidget *toggle_gaussian = NULL;
	GtkWidget *toggle_show_peaks = NULL;
	GtkWidget *toggle_ymax_to_one = NULL;
	GtkWidget *entry_half_width = NULL;
	GtkWidget *entry_scale_x = NULL;
	GtkWidget *entry_scale_y = NULL;
	GtkWidget *entry_shift_x = NULL;
	GtkWidget* hbox_data = NULL;
	GtkWidget* first_hbox = NULL;

	GList* data_list = NULL;

	gdouble xmin = 0;
	gdouble xmax = 10;

	gchar tmp[100];
	
	gtk_window_set_title (GTK_WINDOW (window), title);
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", G_CALLBACK (destroy_spectrum_win), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	
	table1=gtk_table_new(5, 3, FALSE);
	gtk_container_add(GTK_CONTAINER(window), table1);
	gtk_widget_show(table1);
  
	frame_xyplot=gtk_frame_new("XY GabeditPlot");
	gtk_table_attach(GTK_TABLE(table1), frame_xyplot, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_widget_show(frame_xyplot);
  
	xyplot = gabedit_xyplot_new();
	gtk_container_add(GTK_CONTAINER(frame_xyplot), xyplot);
	gtk_widget_show (xyplot);

	g_object_set_data (G_OBJECT (window), "XYPLOT",xyplot);

	frame_set_data=gtk_frame_new(_("Set data"));
	gtk_table_attach(GTK_TABLE(table1), frame_set_data, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 0, 5);
	gtk_widget_show(frame_set_data);

	table2=gtk_table_new(2, 5, FALSE);
	gtk_container_add(GTK_CONTAINER(frame_set_data), table2);
	gtk_widget_show(table2);

	hbox_data = gtk_hbox_new(FALSE,2);
	gtk_table_attach(GTK_TABLE(table2), hbox_data, 0, 1, 0, 1, GTK_FILL, GTK_FILL, 2, 2);
	gtk_widget_show(hbox_data);

	first_hbox = hbox_data;

	toggle_no_convolution = gtk_radio_button_new_with_label( NULL,_("No convolution"));
	gtk_box_pack_start(GTK_BOX(hbox_data), toggle_no_convolution, FALSE, FALSE, 2);
	gtk_widget_show(toggle_no_convolution); 

	toggle_lorentzian = gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (toggle_no_convolution)),_("Lorentzian lineshape") );
	gtk_box_pack_start(GTK_BOX(hbox_data), toggle_lorentzian, FALSE, FALSE, 2);
	gtk_widget_show(toggle_lorentzian); 

	toggle_gaussian = gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (toggle_no_convolution)),_("Gaussian lineshape") );
	gtk_box_pack_start(GTK_BOX(hbox_data), toggle_gaussian, FALSE, FALSE, 2);
	gtk_widget_show(toggle_gaussian); 

	toggle_show_peaks = gtk_check_button_new_with_label(_("Show peaks"));
	gtk_box_pack_start(GTK_BOX(hbox_data), toggle_show_peaks, FALSE, FALSE, 2);
	gtk_widget_show(toggle_show_peaks);

	toggle_ymax_to_one = gtk_check_button_new_with_label(_("Set ymax=1"));
	gtk_box_pack_start(GTK_BOX(hbox_data), toggle_ymax_to_one, FALSE, FALSE, 2);
	gtk_widget_show(toggle_ymax_to_one);

	hbox_data = gtk_hbox_new(FALSE,2);
	gtk_table_attach(GTK_TABLE(table2), hbox_data, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 2, 2);
	gtk_widget_show(hbox_data);

	tmp_label=gtk_label_new(_("X Min: "));
	gtk_box_pack_start(GTK_BOX(hbox_data), tmp_label, FALSE, FALSE, 2);
	gtk_widget_show(tmp_label); 
	
	entry_x_min = gtk_entry_new();
	gtk_widget_set_size_request(entry_x_min,50,-1);
	sprintf(tmp,"%0.3f",xmin);
	gtk_entry_set_text(GTK_ENTRY(entry_x_min),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_data), entry_x_min, FALSE, FALSE, 2);
	gtk_widget_show(entry_x_min);

	tmp_label=gtk_label_new(_("X Max: "));
	gtk_box_pack_start(GTK_BOX(hbox_data), tmp_label, FALSE, FALSE, 2);
	gtk_widget_show(tmp_label); 
	
	entry_x_max = gtk_entry_new();
	gtk_widget_set_size_request(entry_x_max,50,-1);
	sprintf(tmp,"%0.3f",xmax);
	gtk_entry_set_text(GTK_ENTRY(entry_x_max),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_data), entry_x_max, FALSE, FALSE, 2);
	gtk_widget_show(entry_x_max);

	tmp_label=gtk_label_new(_("Half-Width : "));
	gtk_box_pack_start(GTK_BOX(hbox_data), tmp_label, FALSE, FALSE, 2);
	gtk_widget_show(tmp_label); 

	entry_half_width = gtk_entry_new();
	gtk_widget_set_size_request(entry_half_width,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_half_width),"0.25");
	gtk_box_pack_start(GTK_BOX(hbox_data), entry_half_width, FALSE, FALSE, 2);
	gtk_widget_show(entry_half_width);

	tmp_label=gtk_label_new(_("Scale X : "));
	gtk_box_pack_start(GTK_BOX(hbox_data), tmp_label, FALSE, FALSE, 2);
	gtk_widget_show(tmp_label); 

	entry_scale_x = gtk_entry_new();
	gtk_widget_set_size_request(entry_scale_x,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_scale_x),"1.0");
	gtk_box_pack_start(GTK_BOX(hbox_data), entry_scale_x, FALSE, FALSE, 2);
	gtk_widget_show(entry_scale_x);

	tmp_label=gtk_label_new(_("Scale Y : "));
	gtk_box_pack_start(GTK_BOX(hbox_data), tmp_label, FALSE, FALSE, 2);
	gtk_widget_show(tmp_label); 

	entry_scale_y = gtk_entry_new();
	gtk_widget_set_size_request(entry_scale_y,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_scale_y),"1.0");
	gtk_box_pack_start(GTK_BOX(hbox_data), entry_scale_y, FALSE, FALSE, 2);
	gtk_widget_show(entry_scale_y);

	tmp_label=gtk_label_new(_("Shift X : "));
	gtk_box_pack_start(GTK_BOX(hbox_data), tmp_label, FALSE, FALSE, 2);
	gtk_widget_show(tmp_label); 

	entry_shift_x = gtk_entry_new();
	gtk_widget_set_size_request(entry_shift_x,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_shift_x),"0.0");
	gtk_box_pack_start(GTK_BOX(hbox_data), entry_shift_x, FALSE, FALSE, 2);
	gtk_widget_show(entry_shift_x);


	g_object_set_data(G_OBJECT (xyplot), "DataList", data_list);

	statusbar=gtk_statusbar_new();
	gtk_table_attach(GTK_TABLE(table1), statusbar, 0, 1, 3, 4, GTK_FILL, GTK_FILL, 2, 2);
	gtk_widget_show (statusbar);
	g_object_set_data(G_OBJECT (xyplot), "StatusBar", statusbar);

	vbox = gtk_vbox_new(FALSE,2);
	gtk_table_attach(GTK_TABLE(table1), vbox, 0, 1, 4, 5, GTK_FILL, GTK_FILL, 2, 2);
	gtk_widget_show(vbox);


        gabedit_xyplot_set_range(GABEDIT_XYPLOT(xyplot),  0.0,  10,  0,  20);

	gabedit_xyplot_set_x_legends_digits(GABEDIT_XYPLOT(xyplot), 5);
	gabedit_xyplot_set_y_legends_digits(GABEDIT_XYPLOT(xyplot), 5);

	g_signal_connect (G_OBJECT (entry_x_min), "activate", (GCallback)activate_entry_xmin, xyplot);
	g_signal_connect (G_OBJECT (entry_x_max), "activate", (GCallback)activate_entry_xmax, xyplot);

	g_signal_connect(G_OBJECT(toggle_no_convolution), "toggled", G_CALLBACK(toggle_no_convolution_toggled), xyplot);
	g_signal_connect(G_OBJECT(toggle_lorentzian), "toggled", G_CALLBACK(toggle_lorentzian_toggled), xyplot);
	g_signal_connect(G_OBJECT(toggle_gaussian), "toggled", G_CALLBACK(toggle_gaussian_toggled), xyplot);
	g_signal_connect (G_OBJECT (entry_half_width), "activate", (GCallback)activate_entry_half_width, xyplot);
	g_signal_connect (G_OBJECT (entry_scale_x), "activate", (GCallback)activate_entry_scale_x, xyplot);
	g_signal_connect (G_OBJECT (entry_scale_y), "activate", (GCallback)activate_entry_scale_y, xyplot);
	g_signal_connect (G_OBJECT (entry_shift_x), "activate", (GCallback)activate_entry_shift_x, xyplot);
	g_signal_connect(G_OBJECT(toggle_show_peaks), "toggled", G_CALLBACK(toggle_show_peaks_toggled), xyplot);

	g_signal_connect(G_OBJECT(toggle_ymax_to_one), "toggled", G_CALLBACK(toggle_ymax_to_one_toggled), xyplot);


	g_signal_connect_after(G_OBJECT(xyplot), "motion-notify-event", G_CALLBACK(xyplot_motion_notify_event), NULL);

	gtk_widget_show (window);


	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_show_peaks), FALSE);
	toggle_show_peaks_toggled(GTK_TOGGLE_BUTTON(toggle_show_peaks), xyplot);

	g_object_set_data(G_OBJECT (xyplot), "EntryHalfWidth", entry_half_width);
	g_object_set_data(G_OBJECT (xyplot), "EntryScaleX", entry_scale_x);
	g_object_set_data(G_OBJECT (xyplot), "EntryScaleY", entry_scale_y);
	g_object_set_data(G_OBJECT (xyplot), "EntryShiftX", entry_shift_x);
	g_object_set_data(G_OBJECT (xyplot), "EntryXMin", entry_x_min);
	g_object_set_data(G_OBJECT (xyplot), "EntryXMax", entry_x_max);
	g_object_set_data(G_OBJECT (xyplot), "HBoxData", first_hbox);
	g_object_set_data(G_OBJECT (xyplot), "HBoxData2", hbox_data);
	g_object_set_data(G_OBJECT (xyplot), "VBox", vbox);
	g_object_set_data(G_OBJECT (xyplot), "Window", window);
	g_object_set_data(G_OBJECT (window), "HBoxData", first_hbox);
	g_object_set_data(G_OBJECT (window), "HBoxData2", hbox_data);
	g_object_set_data(G_OBJECT (window), "VBox", vbox);
	g_object_set_data(G_OBJECT (window), "NoConvolutionButton", toggle_no_convolution);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle_lorentzian), TRUE);
	toggle_lorentzian_toggled((GtkToggleButton*)toggle_lorentzian,xyplot);

	gabedit_xyplot_set_font (GABEDIT_XYPLOT(xyplot), "sans 10");
	gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(Fenetre));


	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_HMAJOR_GRID, FALSE);
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_VMAJOR_GRID, FALSE);

	gabedit_xyplot_reflect_x (GABEDIT_XYPLOT(xyplot), FALSE);
	gabedit_xyplot_reflect_y (GABEDIT_XYPLOT(xyplot), FALSE);

  
	return window;
}
/****************************************************************************************/
void spectrum_win_relect_x(GtkWidget* winSpectrum, gboolean active)
{
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (winSpectrum), "XYPLOT");
	gabedit_xyplot_reflect_x (GABEDIT_XYPLOT(xyplot), active);
}
/****************************************************************************************/
void spectrum_win_relect_y(GtkWidget* winSpectrum, gboolean active)
{
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (winSpectrum), "XYPLOT");
	gabedit_xyplot_reflect_y (GABEDIT_XYPLOT(xyplot), active);
}
/****************************************************************************************/
void spectrum_win_set_half_width(GtkWidget* winSpectrum, gdouble value)
{
	gchar tmp[100];
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (winSpectrum), "XYPLOT");
	GtkWidget *entry_half_width = g_object_get_data(G_OBJECT (xyplot), "EntryHalfWidth");
	sprintf(tmp,"%0.3f",value);
	gtk_entry_set_text(GTK_ENTRY(entry_half_width),tmp);
	activate_entry_half_width(entry_half_width, xyplot);
}
/****************************************************************************************/
void spectrum_win_set_xmin(GtkWidget* winSpectrum, gdouble value)
{
	gchar tmp[100];
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (winSpectrum), "XYPLOT");
	GtkWidget *entry_x_min = g_object_get_data(G_OBJECT (xyplot), "EntryXMin");
	sprintf(tmp,"%0.3f",value);
	gtk_entry_set_text(GTK_ENTRY(entry_x_min),tmp);
	activate_entry_xmin(entry_x_min, xyplot);
}
/****************************************************************************************/
void spectrum_win_set_xmax(GtkWidget* winSpectrum, gdouble value)
{
	gchar tmp[100];
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (winSpectrum), "XYPLOT");
	GtkWidget *entry_x_max = g_object_get_data(G_OBJECT (xyplot), "EntryXMax");
	sprintf(tmp,"%0.3f",value);
	gtk_entry_set_text(GTK_ENTRY(entry_x_max),tmp);
	activate_entry_xmax(entry_x_max, xyplot);
}
/****************************************************************************************/
void spectrum_win_set_ymin(GtkWidget* winSpectrum, gdouble value)
{
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (winSpectrum), "XYPLOT");
	gabedit_xyplot_set_range_ymin (GABEDIT_XYPLOT(xyplot), value);
}
/****************************************************************************************/
void spectrum_win_set_ymax(GtkWidget* winSpectrum, gdouble value)
{
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (winSpectrum), "XYPLOT");
	gabedit_xyplot_set_range_ymax (GABEDIT_XYPLOT(xyplot), value);
}
/****************************************************************************************/
void spectrum_win_set_xlabel(GtkWidget* winSpectrum, gchar* label)
{
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (winSpectrum), "XYPLOT");
	if(!xyplot) printf("ERROR xyplot =0\n");
	gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), label);
}
/****************************************************************************************/
void spectrum_win_set_ylabel(GtkWidget* winSpectrum, gchar* label)
{
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (winSpectrum), "XYPLOT");
	gabedit_xyplot_set_y_label (GABEDIT_XYPLOT(xyplot), label);
}
/****************************************************************************************/
void spectrum_win_reset_ymin_ymax(GtkWidget *xyplot)
{
	GList* data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");
	GList* current = NULL;
	XYPlotWinData* data = NULL;
	if(!data_list) return;
	current=g_list_first(data_list);
	if(current) data = (XYPlotWinData*)current->data;
	if(data && data->ymaxToOne)
	{
		gabedit_xyplot_set_range_ymin (GABEDIT_XYPLOT(xyplot), 0.0);
		gabedit_xyplot_set_range_ymax (GABEDIT_XYPLOT(xyplot), 1.0);
	}
}
/****************************************************************************************/
/*
GtkWidget* spectrum_win_new_testing(gchar* title)
{
	GtkWidget* window = spectrum_win_new(title);
	gint size = 20;
	gint i;
	gdouble x[20];
	gdouble y[20];
	for(i=0;i<size;i++)
	{
		x[i] = i;
		y[i] = rand()/(gdouble)RAND_MAX*size;
	}
	spectrum_win_add_data(winSpectrum, size, x, y);
	spectrum_win_autorange(window);
	return window;
}
*/
/****************************************************************************************/
GtkWidget* spectrum_win_new_with_xy(gchar* title,  gint size, gdouble* x, gdouble* y)
{
	GtkWidget* winSpectrum = spectrum_win_new(title);
	spectrum_win_add_data(winSpectrum, size, x, y);
	spectrum_win_autorange(winSpectrum);
	return winSpectrum;
}
/****************************************************************************************/
XYPlotData* spectrum_win_get_dataCurve(GtkWidget *winSpectrum)
{
	GtkWidget* xyplot = g_object_get_data(G_OBJECT (winSpectrum), "XYPLOT");
	GList* data_list = g_object_get_data(G_OBJECT (xyplot), "DataList");
	GList* current = NULL;
	XYPlotWinData* data = NULL;
	if(!data_list) return NULL;
	current=g_list_first(data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotWinData*)current->data;
		if(data && data->dataCurve) return data->dataCurve;
	}
	return NULL;
}
