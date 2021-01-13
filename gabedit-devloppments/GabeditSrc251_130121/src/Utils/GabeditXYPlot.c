/* GabeditXYPlot.c */
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

#define GETTEXT_PACKAGE "gabedit"
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <cairo-svg.h>

#include "GabeditXYPlot.h"

#define XYPLOT_DEFAULT_SIZE 300
#define BSIZE 1024
#define SCALE(i) (i / 65535.)
#define SCALE2(i) (i * 65535.)

typedef enum
{
  JDX_TYPE_XYDATA,
  JDX_TYPE_XYPOINTS,
  JDX_TYPE_XYTABLE,
  JDX_TYPE_UNKNOWN
} JDXType;

/****************************************************************************************/
static gint get_distance_M_AB(GabeditXYPlot *xyplot,gint xM, gint yM, gint ixA, gint iyA, gint ixB, gint iyB);
static void gabedit_xyplot_cairo_string(cairo_t* cr, GtkWidget *widget, GdkGC* gc, gint x, gint y, G_CONST_RETURN gchar* str, gboolean centerX, gboolean centerY, gdouble angle);
static void gabedit_xyplot_cairo_line(cairo_t* cr, GtkWidget *widget, GdkGC* gc, gdouble x1,gdouble y1,gdouble x2,gdouble y2);
static void gabedit_xyplot_cairo_lines(cairo_t *cr,  GtkWidget* widget, GdkGC* gc, GdkPoint* points, gint size);
static void gabedit_xyplot_cairo_rectangle(cairo_t* cr, GtkWidget *widget, GdkGC* gc, gboolean fill, gdouble x1,gdouble y1,gdouble w,gdouble h);
/* static void gabedit_xyplot_cairo_cercle(cairo_t* cr, GtkWidget *widget, GdkGC* gc, gint xc,gint yc,gint rayon);*/

static void xyplot_cairo_string(GabeditXYPlot *xyplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gint x, gint y, G_CONST_RETURN gchar* str, gboolean centerX, gboolean centerY, gdouble angle);
static void xyplot_cairo_line(GabeditXYPlot *xyplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gdouble x1,gdouble y1,gdouble x2,gdouble y2);
static void xyplot_cairo_lines(GabeditXYPlot *xyplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, GdkPoint* points, gint size);
static void xyplot_cairo_rectangle(GabeditXYPlot *xyplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gboolean fill, gdouble x1,gdouble y1,gdouble w,gdouble h);
/* static void xyplot_cairo_cercle(GabeditXYPlot *xyplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gint xc,gint yc,gint rayon);*/

static void gabedit_xyplot_class_init (GabeditXYPlotClass    *klass);
static void gabedit_xyplot_init (GabeditXYPlot         *xyplot);
static void gabedit_xyplot_destroy (GtkObject        *object);
static void gabedit_xyplot_realize (GtkWidget        *widget);
static void gabedit_xyplot_size_request (GtkWidget      *widget, GtkRequisition *requisition);
static void gabedit_xyplot_size_allocate (GtkWidget     *widget, GtkAllocation *allocation);
static gint gabedit_xyplot_expose (GtkWidget        *widget, GdkEventExpose   *event);
static gint gabedit_xyplot_button_press (GtkWidget   *widget, GdkEventButton   *event);
static gint gabedit_xyplot_button_release (GtkWidget *widget, GdkEventButton   *event);
static gint gabedit_xyplot_motion_notify (GtkWidget  *widget, GdkEventMotion   *event);
static gint gabedit_xyplot_key_press(GtkWidget* widget, GdkEventKey *event);
static gint gabedit_xyplot_key_release(GtkWidget* widget, GdkEventKey *event);
static gint gabedit_xyplot_scroll (GtkWidget        *widget, GdkEventScroll   *event);
static void gabedit_xyplot_style_set (GtkWidget      *widget, GtkStyle       *previous_style);
static gint gabedit_xyplot_grab(GtkWidget* widget, GdkEventCrossing* event);
static void xyplot_enable_grids (GabeditXYPlot *xyplot, GabeditXYPlotGrid grid, gboolean enable);
static void xyplot_show_left_legends (GabeditXYPlot *xyplot, gboolean show);
static void xyplot_show_right_legends (GabeditXYPlot *xyplot, gboolean show);
static void xyplot_show_top_legends (GabeditXYPlot *xyplot, gboolean show);
static void xyplot_show_bottom_legends (GabeditXYPlot *xyplot, gboolean show);
static void xyplot_show_rectangle_legends (GabeditXYPlot *xyplot, gboolean show);
static void xyplot_reflect_x (GabeditXYPlot *xyplot, gboolean enable);
static void xyplot_reflect_y (GabeditXYPlot *xyplot, gboolean enable);
static void saveAsGabeditDlg(GtkWidget* xyplot);
static void readAGabeditDlg(GtkWidget* xyplot);
static void reset_theme(GtkWidget *widget, gint line_width, GdkColor* foreColor, GdkColor* backColor );
static void set_theme_publication(GtkWidget *widget);
static void set_theme_green_black(GtkWidget *widget);
static void set_theme_dialog(GtkWidget* widget);
static void gabedit_xyplot_cairo_layout(cairo_t* cr, gdouble x, gdouble y, PangoLayout *layout, gboolean centerX, gboolean centerY, gdouble angle);

/****************************************************************************************/
static GtkWidgetClass *parent_class = NULL; /* TO DELETE */
/****************************************************************************************/
static void xyplot_calculate_sizes (GabeditXYPlot *xyplot);
static void value2pixel(GabeditXYPlot *xyplot, gdouble xv, gdouble yv, gint *x, gint *y);
static void pixel2value(GabeditXYPlot *xyplot, gint xp, gint yp, gdouble *x, gdouble *y);
static void xyplot_calculate_legends_sizes(GabeditXYPlot *xyplot);
static void xyplot_build_legends(GabeditXYPlot *xyplot);
static void xyplot_free_legends(GabeditXYPlot *xyplot);
static void xyplot_build_points_data(GabeditXYPlot *xyplot, XYPlotData *data);
static PangoLayout* get_pango_str(GabeditXYPlot *xyplot, G_CONST_RETURN gchar* txt);
static void xyplot_curve_noconv(GabeditXYPlot *xyplot, gint numberOfPoints, gdouble* X,  gdouble* Y, GdkColor* color);

/****************************************************************************************/
static void uppercase(gchar *str)
{
  while( *str != '\0')
  {
    if (isalpha((gint)*str))
      if (islower((gint)*str))
        *str = toupper((gint)*str);
    str ++;
  }
}
/****************************************************************************************/
static void build_linear_x(gdouble* x0, gdouble* y0, gint n, gdouble**X, gdouble** Y, gint N)
{
	
	gdouble dx = 0;
	gdouble* x = NULL;
	gdouble* y = NULL;
	gint i,j;
	*X= x;
	*Y= y;
	if(n<1||N<2) return;
	x =  g_malloc(N*sizeof(gdouble)); 
	y =  g_malloc(N*sizeof(gdouble)); 
	dx = (x0[n-1]-x0[0])/(N-1);

	x[0] = x0[0];
	for(i=1;i<N; i++)  x[i] = x[i-1]+dx;
	y[0] = y0[0];
	for(i=1;i<N; i++)  
	{
		for(j=n-1;j>=0; j--)  if(x[i]>x0[j]) break;
		if(j==n-1) y[i] = y0[j];
		else y[i] = (y0[j+1]-y0[j])/(x0[j+1]-x0[j])*(x[i]-x0[j])+y0[j];
	}
	*X = x;
	*Y = y;
}
/****************************************************************************************/
static void xyplot_message(gchar* message)
{
	GtkWidget* dialog = NULL;
	dialog = gtk_message_dialog_new_with_markup (NULL,
		           GTK_DIALOG_DESTROY_WITH_PARENT,
		           GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			   "%s",
			   message);
       	gtk_dialog_run (GTK_DIALOG (dialog));
       	gtk_widget_destroy (dialog);
}
/**********************************************************************************/
static void calc_arrow_vertexes(
		gdouble arrow_degrees,
		gdouble arrow_lenght,
		gdouble start_x, gdouble start_y, 
		gdouble end_x, gdouble end_y, 
		gdouble* x1, gdouble* y1, 
		gdouble* x2, gdouble* y2
		)
{
	gdouble angle = atan2 (end_y - start_y, end_x - start_x) + M_PI;
	arrow_degrees = arrow_degrees/180.0*M_PI;

	*x1 = end_x + arrow_lenght * cos(angle - arrow_degrees);
	*y1 = end_y + arrow_lenght * sin(angle - arrow_degrees);
	*x2 = end_x + arrow_lenght * cos(angle + arrow_degrees);
	*y2 = end_y + arrow_lenght * sin(angle + arrow_degrees);
}

/**********************************************************************************/
static void gabedit_xyplot_cairo_image(cairo_t* cr, GtkWidget *widget, gint x, gint y, gint w, gint h, cairo_surface_t *image)
{
	gint ow = 1;
	gint oh = 1;
	if(!image) return;
	if(x<0||y<0) return;
	/* printf("x = %d y = %d w = %d h = %d\n",x,y,w,h);*/
	ow = cairo_image_surface_get_width (image);
	oh = cairo_image_surface_get_height (image);

	cairo_save (cr); 

	cairo_translate (cr, x,y);
	cairo_scale (cr, (gdouble)w/ow, (gdouble)h/oh);
	cairo_set_source_surface (cr, image, 0, 0);
	cairo_paint (cr);

	cairo_stroke (cr);
	cairo_restore (cr); 
}
/**********************************************************************************/
static void gabedit_xyplot_cairo_string(cairo_t* cr, GtkWidget *widget, GdkGC* gc, gint x, gint y, G_CONST_RETURN gchar* str, gboolean centerX, gboolean centerY, gdouble angle)
{
	PangoLayout* pango = get_pango_str(GABEDIT_XYPLOT(widget), str);
	gabedit_xyplot_cairo_layout(cr, x, y, pango, centerX,  centerY, angle) ;
	g_object_unref(G_OBJECT(pango));
}
/*****************************************************************************/
static void gabedit_xyplot_cairo_line(cairo_t *cr,  GtkWidget* widget, GdkGC* gc, gdouble x1,gdouble y1,gdouble x2,gdouble y2)
{
	GdkGCValues values;
	GdkColor color;
	double r,g,b;
	GdkColormap *colormap;
	double dashes[] = {5.0,  /* ink */
		           5.0,  /* skip */
			   10.0,  /* ink */
			   5.0   /* skip*/
	};
	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = -5.0;


	if(!cr) return;
	if(!gc) return;
	cairo_save (cr); 
	gdk_gc_get_values(gc, &values);

	switch(values.cap_style)
	{
		case GDK_CAP_NOT_LAST:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT); break;
		case GDK_CAP_BUTT:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT); break;
		case GDK_CAP_ROUND:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND); break;
		case GDK_CAP_PROJECTING:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE); break;
		default:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT); break;
	}
	switch(values.join_style)
	{
		case GDK_JOIN_MITER:
			cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);break;
		case GDK_JOIN_ROUND :
			cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);break;
		case GDK_JOIN_BEVEL :
			cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);break;
		default:
			cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);break;
	}
	switch(values.line_style)
	{
		case GDK_LINE_SOLID : ndash = 0; break;
		case GDK_LINE_ON_OFF_DASH : ndash = 2; break;
		case GDK_LINE_DOUBLE_DASH : break;
		default  : ndash = 0;
	}
   	colormap  = gdk_window_get_colormap(widget->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);

	if( ndash != 0) cairo_set_dash (cr, dashes, ndash, offset);

	r = SCALE(color.red);
	g = SCALE(color.green);
	b = SCALE(color.blue);
	cairo_set_source_rgba (cr, r, g, b, 1.0);
	if(values.line_width<1) values.line_width = 1;

	cairo_set_line_width (cr, values.line_width);
	cairo_move_to (cr, x1, y1);
	cairo_line_to (cr, x2, y2);
	cairo_stroke (cr);
	cairo_restore (cr); 
}
/*****************************************************************************/
static void gabedit_xyplot_cairo_lines(cairo_t *cr,  GtkWidget* widget, GdkGC* gc, GdkPoint* points, gint size)
{
	gint i;
	for(i=0;i<size-1;i++)
		gabedit_xyplot_cairo_line(cr, widget, gc, points[i].x, points[i].y, points[i+1].x, points[i+1].y);
}
/*****************************************************************************/
static void gabedit_xyplot_cairo_rectangle(cairo_t *cr,  GtkWidget* widget, GdkGC* gc, gboolean fill, gdouble x1,gdouble y1,gdouble w,gdouble h)
{
	GdkGCValues values;
	GdkColor color;
	double r,g,b;
	GdkColormap *colormap;
	double dashes[] = {5.0,  /* ink */
		           5.0,  /* skip */
			   10.0,  /* ink */
			   5.0   /* skip*/
	};
	int    ndash  = sizeof (dashes)/sizeof(dashes[0]);
	double offset = 0.0;


	if(!cr) return;
	if(!gc) return;
	cairo_save (cr); 
	gdk_gc_get_values(gc, &values);

	switch(values.cap_style)
	{
		case GDK_CAP_NOT_LAST:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT); break;
		case GDK_CAP_BUTT:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT); break;
		case GDK_CAP_ROUND:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND); break;
		case GDK_CAP_PROJECTING:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE); break;
		default:
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT); break;
	}
	switch(values.join_style)
	{
		case GDK_JOIN_MITER:
			cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);break;
		case GDK_JOIN_ROUND :
			cairo_set_line_join (cr, CAIRO_LINE_JOIN_ROUND);break;
		case GDK_JOIN_BEVEL :
			cairo_set_line_join (cr, CAIRO_LINE_JOIN_BEVEL);break;
		default:
			cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);break;
	}
	switch(values.line_style)
	{
		case GDK_LINE_SOLID : ndash = 0; break;
		case GDK_LINE_ON_OFF_DASH : ndash = 2; break;
		case GDK_LINE_DOUBLE_DASH : break;
		default  : ndash = 0;
	}
   	colormap  = gdk_window_get_colormap(widget->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);

	if( ndash != 0) cairo_set_dash (cr, dashes, ndash, offset);

	r = SCALE(color.red);
	g = SCALE(color.green);
	b = SCALE(color.blue);
	cairo_set_source_rgba (cr, r, g, b, 1.0);

	cairo_move_to (cr, x1, y1);
	cairo_rel_line_to (cr, w, 0);
	cairo_rel_line_to (cr, 0, h);
	cairo_rel_line_to (cr, -w, 0);
	cairo_close_path (cr);
	if(values.line_width<1) values.line_width = 1;
	if(fill) cairo_fill(cr);
	else cairo_set_line_width (cr, values.line_width);
	cairo_stroke (cr);
	cairo_restore (cr); 
}
/**********************************************************************************/
static void gabedit_xyplot_cairo_layout(cairo_t* cr, gdouble x, gdouble y, PangoLayout *layout, gboolean centerX, gboolean centerY, gdouble angle) 
{
	cairo_save (cr); 
	if(centerX || centerY)
	{
		gint w;
		gint h;
		pango_layout_get_size(layout,&w,&h);
		if(centerX) x-=w/2/PANGO_SCALE*cos(angle)-h/2/PANGO_SCALE*sin(angle);
		if(centerY) y-=w/2/PANGO_SCALE*sin(angle)+h/2/PANGO_SCALE*cos(angle);
	}
	cairo_move_to(cr, x,y);
	cairo_rotate(cr, angle);
	
	pango_cairo_show_layout(cr,layout);
	cairo_stroke (cr);
	cairo_restore (cr); 
}
/****************************************************************************************/
static void xyplot_cairo_image(GabeditXYPlot *xyplot, cairo_t* cr, GtkWidget *widget, gint x, gint y, gint w, gint h, cairo_surface_t *image)
{
	gabedit_xyplot_cairo_image(cr,  widget, x, y, w, h, image);
	if(xyplot->cairo_export)
		gabedit_xyplot_cairo_image(xyplot->cairo_export,  widget, x, y, w, h, image);
}
/****************************************************************************************/
static void xyplot_cairo_string(GabeditXYPlot *xyplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gint x, gint y, G_CONST_RETURN gchar* str, gboolean centerX, gboolean centerY, gdouble angle)
{

	gabedit_xyplot_cairo_string(cr,  widget, gc, x, y, str, centerX, centerY, angle);
	if(xyplot->cairo_export)
		gabedit_xyplot_cairo_string(xyplot->cairo_export,  widget, gc,  x, y, str, centerX, centerY, angle);
}
/****************************************************************************************/
static void xyplot_cairo_line(GabeditXYPlot *xyplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gdouble x1,gdouble y1,gdouble x2,gdouble y2)
{
	gabedit_xyplot_cairo_line(cr,  widget, gc, x1, y1, x2, y2);
	if(xyplot->cairo_export)
		gabedit_xyplot_cairo_line(xyplot->cairo_export,  widget, gc,  x1, y1, x2, y2);
}
/****************************************************************************************/
static void xyplot_cairo_rectangle(GabeditXYPlot *xyplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gboolean fill, gdouble x1,gdouble y1,gdouble x2,gdouble y2)
{
	gabedit_xyplot_cairo_rectangle(cr,  widget, gc, fill, x1, y1, x2, y2);
	if(xyplot->cairo_export)
		gabedit_xyplot_cairo_rectangle(xyplot->cairo_export,  widget, gc,  fill, x1, y1, x2, y2);
}
/****************************************************************************************/
static void xyplot_cairo_lines(GabeditXYPlot *xyplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, GdkPoint* points, gint size)
{
	gabedit_xyplot_cairo_lines(cr,  widget, gc, points, 2);
	if(xyplot->cairo_export && size>0)
	{
		GdkPoint* pointsScal = g_malloc(size*sizeof(GdkPoint));
		gint i;
		if(!pointsScal) return;
		for(i=0;i<size;i++)
		{
			pointsScal[i].x = points[i].x + xyplot->plotting_rect.x;
			pointsScal[i].y = points[i].y + xyplot->plotting_rect.y;
		}
		gabedit_xyplot_cairo_lines(xyplot->cairo_export,  widget, gc, pointsScal, 2);
		if(pointsScal) g_free(pointsScal);
	}
}
/****************************************************************************************/
/*
static void xyplot_cairo_cercle(GabeditXYPlot *xyplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gint xc,gint yc,gint rayon)
{
	gabedit_xyplot_cairo_cercle(cr,  widget, gc, xc, yc, rayon);
	if(xyplot->cairo_export)
		gabedit_xyplot_cairo_cercle(xyplot->cairo_export,  widget, gc, xc, yc, rayon);
}
*/
/****************************************************************************************/
static void xyplot_cairo_layout(GabeditXYPlot *xyplot, cairo_t* cr,   gdouble x, gdouble y, PangoLayout *layout, gboolean centerX, gboolean centerY, gdouble angle)
{
	gabedit_xyplot_cairo_layout(cr, x, y, layout, centerX, centerY, angle);
	if(xyplot->cairo_export)
	{
		if(cr == xyplot->cairo_area)
		{
			x += xyplot->plotting_rect.x;
			y += xyplot->plotting_rect.y;
		}
		gabedit_xyplot_cairo_layout(xyplot->cairo_export,  x, y, layout, centerX, centerY, angle);
	}
}
/****************************************************************************************/
/*
static void list_utf8()
{
	gchar tmp[10000];
	gchar t[10000];
	GtkWidget* dialog = NULL;
	gint i=32;
	sprintf(tmp,"%d(<span>&#%d;</span>)  ",i,i);
	
	for(i=9600;i<9700;i++)
	{

		sprintf(t,"%s",tmp);
		sprintf(tmp,"%s %d(<span>&#%d;</span>)  ",t,i,i);
	}
	dialog = gtk_message_dialog_new_with_markup (NULL,
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_INFO,
			GTK_BUTTONS_YES_NO,
			tmp
			);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}
*/
/****************************************************************************************/
static GtkWidget* get_parent_window(GtkWidget* widget)
{
	GtkWidget* parent = NULL;
	GtkWidget* child = widget;
	do
	{
		parent = gtk_widget_get_parent(child);
		child = parent;
	}
	while( parent && !GTK_IS_WINDOW(parent));
	return parent;
}
/****************************************************************************************/
static void destroy_xyplot_window(GtkWidget* xyplot)
{
	GtkWidget* parentWindow;
	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow) gtk_object_destroy (GTK_OBJECT(parentWindow));
}
/****************************************************************************************/
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
/****************************************************************************************/
static void destroy_data_dialog(GtkWidget* win, GdkEvent  *event, gpointer user_data)
{
  	gtk_object_destroy (GTK_OBJECT(win));
}
/********************************************************************************/
static gboolean remove_all_data(GtkWidget* xyplot, gint response_id)
{
	if(response_id != GTK_RESPONSE_YES) return FALSE;
	if(!xyplot) return FALSE; 
	if(GABEDIT_XYPLOT(xyplot)->data_list)
	{
		g_list_foreach(GABEDIT_XYPLOT(xyplot)->data_list, (GFunc)g_free, NULL);
		g_list_free(GABEDIT_XYPLOT(xyplot)->data_list);
		GABEDIT_XYPLOT(xyplot)->data_list = NULL;
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	return TRUE;
}
/********************************************************************************/
static void remove_all_data_dlg(GtkWidget* xyplot)
{
	GtkWidget* parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(parentWindow),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
		       _("Are you sure to remove all data ?")
			);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_NO);
	g_signal_connect_swapped(dialog, "response", G_CALLBACK (remove_all_data), xyplot);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}
/*****************************************************************************************/
static void file_chooser_set_filters(GtkFileChooser *fileChooser,gchar **patterns)
{
	GtkFileFilter *filter;
	GtkFileFilter *filter0 = NULL;
	gint n = 0;

	g_return_if_fail (fileChooser != NULL);
	g_return_if_fail (GTK_IS_FILE_CHOOSER(fileChooser));
	g_return_if_fail (patterns != NULL);
	while(patterns[n])
	{
		filter = gtk_file_filter_new ();
		gtk_file_filter_set_name (filter, patterns[n]);
		gtk_file_filter_add_pattern (filter, patterns[n]);
		gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (fileChooser), filter);
		if(n==0) filter0 = filter;
		n++;
	}
	if(filter0)gtk_file_chooser_set_filter (GTK_FILE_CHOOSER (fileChooser), filter0);
}
/********************************************************************************/
static GtkWidget* new_file_chooser_open(GtkWidget* parentWindow, GCallback *func, gchar* title, gchar** patternsfiles)
{
	GtkWidget* filesel = NULL;
	filesel = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", GTK_FILE_CHOOSER_ACTION_OPEN, "file-system-backend", "gtk+", "select-multiple", FALSE, NULL);
	gtk_window_set_title (GTK_WINDOW (filesel), title);
	gtk_dialog_add_buttons (GTK_DIALOG (filesel), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (filesel), GTK_RESPONSE_OK);

	if(parentWindow) gtk_window_set_transient_for(GTK_WINDOW(filesel),GTK_WINDOW(parentWindow));

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_signal_connect (filesel, "response",  G_CALLBACK (func),GTK_OBJECT(filesel));
	g_signal_connect_after (filesel, "response", G_CALLBACK (gtk_widget_destroy),GTK_OBJECT(filesel));
	file_chooser_set_filters(GTK_FILE_CHOOSER(filesel),patternsfiles);
	gtk_widget_show(filesel);
	return filesel;
}
/********************************************************************************/
static GtkWidget* new_file_chooser_save(GtkWidget* parentWindow, GCallback *func, gchar* title, gchar** patternsfiles)
{
	GtkWidget* filesel = NULL;
	filesel = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", GTK_FILE_CHOOSER_ACTION_SAVE, "file-system-backend", "gtk+", "select-multiple", FALSE, NULL);
	gtk_window_set_title (GTK_WINDOW (filesel), title);
	gtk_dialog_add_buttons (GTK_DIALOG (filesel), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
	gtk_dialog_set_default_response (GTK_DIALOG (filesel), GTK_RESPONSE_OK);

	if(parentWindow) gtk_window_set_transient_for(GTK_WINDOW(filesel),GTK_WINDOW(parentWindow));

	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_signal_connect (filesel, "response",  G_CALLBACK (func),GTK_OBJECT(filesel));
	g_signal_connect_after (filesel, "response", G_CALLBACK (gtk_widget_destroy),GTK_OBJECT(filesel));
	file_chooser_set_filters(GTK_FILE_CHOOSER(filesel),patternsfiles);
	gtk_widget_show(filesel);
	return filesel;
}
/********************************************************************************/
static void add_new_data(GtkWidget* xyplot, gint numberOfPoints, gdouble* X,  gdouble* Y)
{
	if(numberOfPoints>0)
	{
		gint loop;
		XYPlotData *data = g_malloc(sizeof(XYPlotData));
		gint red = 0;
		gint green = 0;
		gint blue = 0;
		GdkColor c = get_fore_color(GABEDIT_XYPLOT(xyplot));

		red = c.red;
		green = c.green;
		blue = c.blue;

		data->size=numberOfPoints;
		data->x = g_malloc(numberOfPoints*sizeof(gdouble)); 
		data->y = g_malloc(numberOfPoints*sizeof(gdouble)); 
		for(loop = 0; loop<data->size;loop++)
		{
			data->x[loop] = X[loop];
			data->y[loop] = Y[loop];
		}

		sprintf(data->point_str,"+");
		data->point_pango = NULL;
		xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), data);

		data->point_size=0;
		data->line_width=2;
		data->point_color.red=red; 
		data->point_color.green=green; 
		data->point_color.blue=blue; 

		data->line_color.red=green;
		data->line_color.green=red;
		data->line_color.blue=blue;
		data->line_style=GDK_LINE_SOLID;
		gabedit_xyplot_add_data (GABEDIT_XYPLOT(xyplot), data);
		gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(xyplot), NULL);
	}
}
/********************************************************************************/
static void add_new_data_peaks(GtkWidget* xyplot, gint numberOfPoints, gdouble* X,  gdouble* Y)
{
	if(numberOfPoints>0)
	{
		gint loop;
		XYPlotData *data = g_malloc(sizeof(XYPlotData));
		gdouble xmin = 0;
		gdouble xmax = 0;
		gint red = 0;
		gint green = 0;
		gint blue = 0;
		GdkColor c = get_fore_color(GABEDIT_XYPLOT(xyplot));

		red = c.red;
		green = c.green;
		blue = c.blue;

		data->size=3*numberOfPoints+2;
		data->x = g_malloc(data->size*sizeof(gdouble)); 
		data->y = g_malloc(data->size*sizeof(gdouble)); 

		xmin = X[0];
		xmax = X[0];
		for(loop = 1; loop<numberOfPoints;loop++)
		{
			if(xmin>X[loop]) xmin = X[loop];
			if(xmax<X[loop]) xmax = X[loop];
		}

		data->x[0]=xmin;
		data->y[0]=0;
		data->x[data->size-1]=xmax;
		data->y[data->size-1]=0;
		for (loop=0; loop<numberOfPoints; loop++){
			gint iold = loop*3+1;
			data->x[iold]=X[loop];
			data->y[iold]=0;

			data->x[iold+1]=X[loop];
			data->y[iold+1]=Y[loop];

			data->x[iold+2]=X[loop];
			data->y[iold+2]=0;
		}

		sprintf(data->point_str,"+");
		data->point_pango = NULL;
		xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), data);

		data->point_size=0;
		data->line_width=2;
		data->point_color.red=red; 
		data->point_color.green=green; 
		data->point_color.blue=blue; 

		data->line_color.red=green;
		data->line_color.green=red;
		data->line_color.blue=blue;
		data->line_style=GDK_LINE_SOLID;
		gabedit_xyplot_add_data (GABEDIT_XYPLOT(xyplot), data);
		gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(xyplot), NULL);
	}
}
/****************************************************************************************/
static void set_object_text_pixels(GabeditXYPlot *xyplot, XYPlotObjectText* objectText)
{
	if(!objectText) return;
	value2pixel(xyplot, objectText->x,objectText->y, &objectText->xi, &objectText->yi);
	objectText->yi=xyplot->plotting_rect.height-objectText->yi;
	pango_layout_get_size(objectText->pango, &objectText->width, &objectText->height);
	objectText->width/=PANGO_SCALE;
	objectText->height/=PANGO_SCALE;
	/* objectText->yi -= objectText->height/2;*/
	objectText->xi += xyplot->plotting_rect.x;
	objectText->yi += xyplot->plotting_rect.y;
}
/****************************************************************************************/
static PangoLayout* get_pango_str(GabeditXYPlot *xyplot, G_CONST_RETURN gchar* txt)
{
	gchar *str = NULL;
	GdkGCValues values;
	GdkColormap *colormap;
	GdkColor color;
	gchar* rgb = NULL;
	PangoLayout* pango;

	if(!txt) return NULL;

	pango = gtk_widget_create_pango_layout (GTK_WIDGET(xyplot), txt);
	pango_layout_set_alignment(pango,PANGO_ALIGN_LEFT);

	gdk_gc_get_values(xyplot->fore_gc, &values);
   	colormap  = gdk_window_get_colormap(GTK_WIDGET(xyplot)->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);
	rgb = g_strdup_printf("#%02x%02x%02x", color.red >> 8, color.green >> 8, color.blue >> 8);
	str = g_strconcat("<span foreground='", rgb, "'>",txt, "</span>", NULL);
	pango_layout_set_markup(pango, str, -1);
	g_free(str);
	return pango;
}
/****************************************************************************************/
static void set_object_text_pango(GabeditXYPlot *xyplot, XYPlotObjectText* objectText)
{
	if(!objectText) return;
  	objectText->pango = get_pango_str(xyplot, objectText->str);
}
/****************************************************************************************/
static void set_object_text(GabeditXYPlot *xyplot, XYPlotObjectText* objectText, gdouble x, gdouble y, gdouble angle, G_CONST_RETURN gchar* str)
{
	if(!str) return;
	objectText->x = x;
	objectText->y = y;
	objectText->angle = angle;
	objectText->str = g_strdup(str);
	set_object_text_pango(xyplot, objectText);
	set_object_text_pixels(xyplot, objectText);
}
/****************************************************************************************/
static void add_object_text(GabeditXYPlot *xyplot, gdouble x, gdouble y, gdouble angle, G_CONST_RETURN gchar* str)
{
	gint i;
	if(!str) return;
	xyplot->nObjectsText++;
	if(xyplot->nObjectsText==1) xyplot->objectsText = g_malloc(sizeof(XYPlotObjectText));
	else xyplot->objectsText = g_realloc(xyplot->objectsText,xyplot->nObjectsText*sizeof(XYPlotObjectText));
	i = xyplot->nObjectsText-1;
	set_object_text(xyplot, &xyplot->objectsText[i],  x,  y, angle , str);
}
/****************************************************************************************/
static void reset_object_text_pixels(GabeditXYPlot *xyplot)
{
	gint i;
	if(!xyplot) return;
	for(i=0;i<xyplot->nObjectsText;i++)
		set_object_text_pixels(xyplot, &xyplot->objectsText[i]);
}
/****************************************************************************************/
static void reset_object_text_pango(GabeditXYPlot *xyplot)
{
	gint i;
	if(!xyplot) return;
	for(i=0;i<xyplot->nObjectsText;i++)
		set_object_text_pango(xyplot, &xyplot->objectsText[i]);
}
/****************************************************************************************/
static gint get_object_text_num(GabeditXYPlot *xyplot, gint xi, gint yi)
{
	gint i;
	gint x,y;
	if(!xyplot) return -1;

	for(i=0;i<xyplot->nObjectsText;i++)
	{
		gdouble cosa = cos(xyplot->objectsText[i].angle);
		gdouble sina = sin(xyplot->objectsText[i].angle);
		gdouble xx = xi-xyplot->objectsText[i].xi;
		gdouble yy = yi-xyplot->objectsText[i].yi;
		x = xyplot->objectsText[i].xi+xx*cosa-yy*sina;
		y = xyplot->objectsText[i].yi+xx*sina+yy*cosa;
		if(x>=xyplot->objectsText[i].xi && y>=xyplot->objectsText[i].yi
		&& x<=xyplot->objectsText[i].xi+xyplot->objectsText[i].width 
		&& y<=xyplot->objectsText[i].yi+xyplot->objectsText[i].height)
			return i;
	}
	return -1;
}
/****************************************************************************************/
static void delete_object_text(GtkWidget *widget, gint i)
{
	gint j;
	GabeditXYPlot *xyplot = NULL;
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	xyplot = GABEDIT_XYPLOT(widget);
	if(i<0||i>=xyplot->nObjectsText) return;
	if(xyplot->objectsText[i].str) g_free(xyplot->objectsText[i].str);
	if(xyplot->objectsText[i].pango) g_object_unref(G_OBJECT(xyplot->objectsText[i].pango));
	for(j=i;j<xyplot->nObjectsText-1;j++)
		xyplot->objectsText[j] = xyplot->objectsText[j+1];
	xyplot->nObjectsText--;
	if(xyplot->nObjectsText<1) 
	{
		xyplot->nObjectsText = 0;
		if(xyplot->objectsText) g_free(xyplot->objectsText);
		xyplot->objectsText = NULL;
	}
	else
	{
		xyplot->objectsText = g_realloc(xyplot->objectsText,xyplot->nObjectsText*sizeof(XYPlotObjectText));
	}

}
/****************************************************************************************/
static void delete_objects_text(GtkWidget *widget)
{
	GabeditXYPlot *xyplot = NULL;
	gint i;
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	xyplot = GABEDIT_XYPLOT(widget);
	for(i=0;i<xyplot->nObjectsText;i++)
	{
		if(xyplot->objectsText[i].str) g_free(xyplot->objectsText[i].str);
		if(xyplot->objectsText[i].pango) g_object_unref(G_OBJECT(xyplot->objectsText[i].pango));
	}
	xyplot->nObjectsText = 0;
	if(xyplot->objectsText) g_free(xyplot->objectsText);
	xyplot->objectsText = NULL;
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/********************************************************************************/
static void activate_entry_object_text(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* tlabel;
	G_CONST_RETURN gchar* tangle;
	GtkWidget* xyplot = NULL;
	GtkWidget* window = NULL;
	GtkWidget* entry_label = NULL;
	GtkWidget* entry_angle = NULL;
	XYPlotObjectText* objectText = NULL;


	if(!entry) return;
	if(!GTK_IS_WIDGET(entry)) return;
	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	entry_label = g_object_get_data(G_OBJECT(entry),"EntryLabel");
	entry_angle = g_object_get_data(G_OBJECT(entry),"EntryAngle");

	if(!GTK_IS_WIDGET(entry_label)) return;
	if(!GTK_IS_WIDGET(entry_angle)) return;

	tlabel= gtk_entry_get_text(GTK_ENTRY(entry_label));
	tangle= gtk_entry_get_text(GTK_ENTRY(entry_angle));
	window = g_object_get_data(G_OBJECT(entry),"Window");
	objectText = g_object_get_data(G_OBJECT(entry),"ObjectText");
	/* t is destroyed with window */
	if(!objectText) 
	{
		if(window)gtk_widget_destroy(window);
		return;
	}
	if(!objectText->str)
	{
		gdouble angle = 0;
		if(tangle && strlen(tangle)>0) angle = atof(tangle)/180.0*M_PI;
		add_object_text(GABEDIT_XYPLOT(xyplot), objectText->x, objectText->y, angle, tlabel);
		g_free(objectText);
	}
	else
	{
		gdouble angle = 0;
		if(tangle && strlen(tangle)>0) angle = atof(tangle)/180.0*M_PI;
		if(objectText->str) g_free(objectText->str);
		if(objectText->pango) g_object_unref(G_OBJECT(objectText->pango));
		set_object_text(GABEDIT_XYPLOT(xyplot), objectText,  objectText->x, objectText->y, angle, tlabel);
	}
	if(window)gtk_widget_destroy(window);
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/****************************************************************************************/
static void add_set_object_text_dialog(GtkWidget* xyplot, gint i, gdouble x, gdouble y)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* parentWindow = NULL;
	GtkWidget* frame = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* vbox_frame = NULL;
	GtkWidget* label = NULL;
	GtkWidget* entry_label = NULL;
	GtkWidget* entry_angle = NULL;
	XYPlotObjectText* objectText = NULL;

	if(i>-1 && i<GABEDIT_XYPLOT(xyplot)->nObjectsText)
	{
		objectText = &GABEDIT_XYPLOT(xyplot)->objectsText[i];
	}
	else
	{
		objectText = g_malloc(sizeof(XYPlotObjectText));
		objectText->x = x;
		objectText->y = y;
		objectText->str = NULL;
		objectText->pango = NULL;
	}

	gtk_window_set_title (GTK_WINDOW (window), _("Set label"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)xyplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);

	frame=gtk_frame_new(NULL);
	gtk_container_add(GTK_CONTAINER(hbox), frame);
	gtk_widget_show(frame);

	vbox_frame=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox_frame);
	gtk_widget_show(vbox_frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_frame), hbox, FALSE, FALSE, 2);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Label : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_label = gtk_entry_new();
	/* gtk_widget_set_size_request(entry_label,100,-1);*/
	if(i>-1 && i<GABEDIT_XYPLOT(xyplot)->nObjectsText)
	{
		gtk_entry_set_text(GTK_ENTRY(entry_label),GABEDIT_XYPLOT(xyplot)->objectsText[i].str);
	}
	else 
		gtk_entry_set_text(GTK_ENTRY(entry_label),"<span foreground='blue' font_desc='20'>Blue text</span> is <i>cool</i>!");
	gtk_box_pack_start(GTK_BOX(hbox), entry_label, TRUE, TRUE, 2);
	gtk_widget_show(entry_label); 
	
	g_object_set_data(G_OBJECT(entry_label),"ObjectText", objectText);

	label=gtk_label_new(
			_(
			"You can use the Pango Text Attribute Markup Language\n"
			"Example : <span foreground='blue' font_desc='Sans 20'>Blue text</span> is <i>cool</i>!\n"
			"\nFor insert a special character : control shift u + code UTF8\n"
			"Examples : \n"
			" control shift u + 03B1 for alpha (greek)\n"
			" control shift u + 03B2 for beta (greek)\n"
			" control shift u + 03A3 for cap Sigma (greek)\n"
			" See http://www.utf8-chartable.de\n"
			)
			);
	gtk_widget_show(label); 
	gtk_box_pack_start(GTK_BOX(vbox_frame), label, FALSE, FALSE, 2);


	g_object_set_data(G_OBJECT(entry_label),"Window", window);
	g_signal_connect (G_OBJECT (entry_label), "activate", (GCallback)activate_entry_object_text, xyplot);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_frame), hbox, FALSE, FALSE, 2);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Angle : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_angle = gtk_entry_new();
	if(i>-1 && i<GABEDIT_XYPLOT(xyplot)->nObjectsText)
	{
		gchar* tmp = g_strdup_printf("%f", GABEDIT_XYPLOT(xyplot)->objectsText[i].angle/M_PI*180.0);
		gtk_entry_set_text(GTK_ENTRY(entry_angle),tmp);
		g_free(tmp);
	}
	else gtk_entry_set_text(GTK_ENTRY(entry_angle),"0.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_angle, TRUE, TRUE, 2);
	gtk_widget_show(entry_angle); 
	
	g_object_set_data(G_OBJECT(entry_angle),"ObjectText", objectText);
	g_object_set_data(G_OBJECT(entry_angle),"Window", window);
	g_object_set_data(G_OBJECT(entry_angle),"EntryLabel", entry_label);
	g_object_set_data(G_OBJECT(entry_angle),"EntryAngle", entry_angle);
	g_object_set_data(G_OBJECT(entry_label),"EntryLabel", entry_label);
	g_object_set_data(G_OBJECT(entry_label),"EntryAngle", entry_angle);
	g_signal_connect (G_OBJECT (entry_angle), "activate", (GCallback)activate_entry_object_text, xyplot);

	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
}
/****************************************************************************************/
static void begin_insert_objects_text(GtkWidget *widget)
{
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	GABEDIT_XYPLOT(widget)->t_key_pressed = TRUE;
}
/****************************************************************************************/
static void set_object_line_pixels(GabeditXYPlot *xyplot, XYPlotObjectLine* objectLine)
{
	if(!objectLine) return;
	value2pixel(xyplot, objectLine->x1,objectLine->y1, &objectLine->x1i, &objectLine->y1i);
	objectLine->y1i=xyplot->plotting_rect.height-objectLine->y1i;
	objectLine->x1i += xyplot->plotting_rect.x;
	objectLine->y1i += xyplot->plotting_rect.y;
	value2pixel(xyplot, objectLine->x2,objectLine->y2, &objectLine->x2i, &objectLine->y2i);
	objectLine->y2i=xyplot->plotting_rect.height-objectLine->y2i;
	objectLine->x2i += xyplot->plotting_rect.x;
	objectLine->y2i += xyplot->plotting_rect.y;
}
/****************************************************************************************/
static void set_object_line(GabeditXYPlot *xyplot, XYPlotObjectLine* objectLine, 
		gdouble x1, gdouble y1, 
		gdouble x2, gdouble y2,
  		gint width,
  		gint arrow_size,
  		GdkColor color,
  		GdkLineStyle style
		)
{
	if(!objectLine) return;
	objectLine->x1 = x1;
	objectLine->y1 = y1;
	objectLine->x2 = x2;
	objectLine->y2 = y2;
	objectLine->width = width;
	objectLine->arrow_size = arrow_size;
	objectLine->color = color;
	objectLine->style = style;
	set_object_line_pixels(xyplot, objectLine);
}
/****************************************************************************************/
static void add_object_line(GabeditXYPlot *xyplot, 
		gdouble x1, gdouble y1, 
		gdouble x2, gdouble y2
		)
{
  	gint width = 1;
  	gint arrow_size = 0;
  	GdkColor color;
  	GdkLineStyle style = GDK_LINE_SOLID;
	gint i;
	xyplot->nObjectsLine++;
	if(xyplot->nObjectsLine==1) xyplot->objectsLine = g_malloc(sizeof(XYPlotObjectLine));
	else xyplot->objectsLine = g_realloc(xyplot->objectsLine,xyplot->nObjectsLine*sizeof(XYPlotObjectLine));
	i = xyplot->nObjectsLine-1;

	if(xyplot->nObjectsLine==1)
	{
		GdkGCValues values;
		GdkColormap *colormap;
		gdk_gc_get_values(xyplot->lines_gc, &values);
   		colormap  = gdk_window_get_colormap(GTK_WIDGET(xyplot)->window);
        	gdk_colormap_query_color(colormap, values.foreground.pixel,&color);
		style = values.line_style;
		width = values.line_width;
	}
	else
	{
		color = xyplot->objectsLine[xyplot->nObjectsLine-2].color;
		style = xyplot->objectsLine[xyplot->nObjectsLine-2].style;
		width = xyplot->objectsLine[xyplot->nObjectsLine-2].width;
		arrow_size = xyplot->objectsLine[xyplot->nObjectsLine-2].arrow_size;
	}

	set_object_line(xyplot, &xyplot->objectsLine[i],  x1,  y1, x2, y2,
			width,
			arrow_size,
			color,
			style);
}
/****************************************************************************************/
static void reset_object_line_pixels(GabeditXYPlot *xyplot)
{
	gint i;
	if(!xyplot) return;
	for(i=0;i<xyplot->nObjectsLine;i++)
		set_object_line_pixels(xyplot, &xyplot->objectsLine[i]);
}
/****************************************************************************************/
static gint get_object_line_num(GabeditXYPlot *xyplot, gint xi, gint yi)
{
	gint i;
	if(!xyplot) return -1;
	for(i=0;i<xyplot->nObjectsLine;i++)
	{
		gint d = get_distance_M_AB(xyplot,xi,yi, 
				xyplot->objectsLine[i].x1i,xyplot->objectsLine[i].y1i,
				xyplot->objectsLine[i].x2i,xyplot->objectsLine[i].y2i);
		if(d<5 || d<xyplot->objectsLine[i].width) return i;
	}
	return -1;
}
/****************************************************************************************/
static void delete_object_line(GtkWidget *widget, gint i)
{
	gint j;
	GabeditXYPlot *xyplot = NULL;
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	xyplot = GABEDIT_XYPLOT(widget);
	if(i<0||i>=xyplot->nObjectsLine) return;
	for(j=i;j<xyplot->nObjectsLine-1;j++)
		xyplot->objectsLine[j] = xyplot->objectsLine[j+1];
	xyplot->nObjectsLine--;
	if(xyplot->nObjectsLine<1) 
	{
		xyplot->nObjectsLine = 0;
		if(xyplot->objectsLine) g_free(xyplot->objectsLine);
		xyplot->objectsLine = NULL;
	}
	else
	{
		xyplot->objectsLine = g_realloc(xyplot->objectsLine,xyplot->nObjectsLine*sizeof(XYPlotObjectLine));
	}

}
/****************************************************************************************/
static void delete_objects_line(GtkWidget *widget)
{
	GabeditXYPlot *xyplot = NULL;
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	xyplot = GABEDIT_XYPLOT(widget);
	xyplot->nObjectsLine = 0;
	if(xyplot->objectsLine) g_free(xyplot->objectsLine);
	xyplot->objectsLine = NULL;
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/****************************************************************************************/
static void begin_insert_objects_line(GtkWidget *widget)
{
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	GABEDIT_XYPLOT(widget)->l_key_pressed = TRUE;
}
/****************************************************************************************/
static void spin_line_width_changed_value_object(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotObjectLine* objectLine = g_object_get_data(G_OBJECT(spinbutton),"ObjectLine");
  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
		if(objectLine) objectLine->width = gtk_spin_button_get_value(spinbutton);
		else
		{
			gint i;
			for (i=0; i<GABEDIT_XYPLOT (xyplot)->nObjectsLine;i++)
			{
				GABEDIT_XYPLOT (xyplot)->objectsLine[i].width = gtk_spin_button_get_value(spinbutton);
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/****************************************************************************************/
static void spin_line_color_changed_value_object(GtkColorButton  *colorbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotObjectLine* objectLine = g_object_get_data(G_OBJECT(colorbutton),"ObjectLine");
		GdkColor c;
		gtk_color_button_get_color (colorbutton, &c);

  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

		if(objectLine) objectLine->color = c;
		else
		{
			gint i;
			for (i=0; i<GABEDIT_XYPLOT (xyplot)->nObjectsLine;i++)
			{
				GABEDIT_XYPLOT (xyplot)->objectsLine[i].color = c;
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/********************************************************************************************************/
static void combo_line_style_changed_value_object(GtkComboBox *combobox, gpointer user_data)
{
	GtkTreeIter iter;
	gchar* d = NULL;

	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotObjectLine* objectLine = g_object_get_data(G_OBJECT(combobox),"ObjectLine");
  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

		gtk_tree_model_get (model, &iter, 0, &d, -1);
		if(!d) return;
		if(objectLine)
		{
		if (!strcmp(d,"Solid") ) {  objectLine->style = GDK_LINE_SOLID; }
		else if (!strcmp(d,"On-Off dashed") ) {objectLine->style = GDK_LINE_ON_OFF_DASH; }
		else if (!strcmp(d,"Double dashed") ) { objectLine->style = GDK_LINE_DOUBLE_DASH;}
		}
		else
		{
			gint i;
			for (i=0; i<GABEDIT_XYPLOT (xyplot)->nObjectsLine;i++)
			{
				if (!strcmp(d,"Solid") ) {  GABEDIT_XYPLOT (xyplot)->objectsLine[i].style = GDK_LINE_SOLID; }
				else if (!strcmp(d,"On-Off dashed") ) {GABEDIT_XYPLOT (xyplot)->objectsLine[i].style = GDK_LINE_ON_OFF_DASH; }
				else if (!strcmp(d,"Double dashed") ) { GABEDIT_XYPLOT (xyplot)->objectsLine[i].style = GDK_LINE_DOUBLE_DASH;}
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/****************************************************************************************/
static void spin_arrow_size_changed_value_object(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotObjectLine* objectLine = g_object_get_data(G_OBJECT(spinbutton),"ObjectLine");

  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

		if(objectLine) objectLine->arrow_size = gtk_spin_button_get_value(spinbutton);
		else
		{
			gint i;
			for (i=0; i<GABEDIT_XYPLOT (xyplot)->nObjectsLine;i++)
			{
				GABEDIT_XYPLOT (xyplot)->objectsLine[i].arrow_size = gtk_spin_button_get_value(spinbutton);
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/********************************************************************************************************/
static GtkWidget *add_line_types_combo_object(GtkWidget *hbox)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Solid", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "On-Off dashed", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Double dashed", -1);

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	gtk_box_pack_start (GTK_BOX (hbox), combobox, TRUE, TRUE, 1);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	return combobox;
}
/****************************************************************************************/
static void set_object_line_dialog(GabeditXYPlot* xyplot, gint i)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* frame = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* hbox1 = NULL;
	GtkWidget* label = NULL;
	GtkWidget* combo = NULL;
	GtkWidget* spin = NULL;
	GtkWidget* button = NULL;
	GtkWidget* spin_arrow = NULL;
	GtkWidget* parentWindow = NULL;
	GtkWidget* vbox_window = NULL;
	XYPlotObjectLine* objectLine = NULL;
	GdkLineStyle line_style =  GDK_LINE_SOLID;

	if(i>=0 && i<=xyplot->nObjectsLine-1) objectLine = &xyplot->objectsLine[i];
	else return;
	line_style =  objectLine->style;

	gtk_window_set_title (GTK_WINDOW (window), _("Set line options"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)xyplot);

	vbox_window=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox_window);
	gtk_widget_show(vbox_window);

	hbox1=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_window), hbox1, TRUE, FALSE, 2);
	gtk_widget_show(hbox1);

	frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(hbox1), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Line width :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin = gtk_spin_button_new_with_range(0, 10, 1);
	if(objectLine) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), objectLine->width);
	else gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), 1);

	gtk_box_pack_start(GTK_BOX(hbox), spin, TRUE, FALSE, 2);
	gtk_widget_show(spin);
	g_object_set_data(G_OBJECT (window), "SpinLineWidth", spin);

	label=gtk_label_new(_("Line type :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	combo = add_line_types_combo_object(hbox);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
	if(objectLine) 
	{
		GdkLineStyle line_style =  objectLine->style;
		if(line_style == GDK_LINE_SOLID) gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
		else if(line_style == GDK_LINE_ON_OFF_DASH) gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 1);
		else if(line_style == GDK_LINE_DOUBLE_DASH) gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 2);
	}
	gtk_widget_show(combo); 
	g_object_set_data(G_OBJECT (window), "ComboLineType", combo);

	label=gtk_label_new(_("Line color :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	button = gtk_color_button_new_with_color (&objectLine->color);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, FALSE, 2);
	gtk_widget_show(button);
	g_object_set_data(G_OBJECT (window), "ColorButton", button);

	label=gtk_label_new(_("Arrow size :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin_arrow = gtk_spin_button_new_with_range(0, 30, 1);

	if(objectLine) 
		gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_arrow), objectLine->arrow_size);
	gtk_box_pack_start(GTK_BOX(hbox), spin_arrow, TRUE, FALSE, 2);
	gtk_widget_show(spin_arrow);
	g_object_set_data(G_OBJECT (window), "SpinArrowSize", spin_arrow);


	g_object_set_data(G_OBJECT (spin), "ObjectLine", objectLine);
	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(spin_line_width_changed_value_object), xyplot);

	g_object_set_data(G_OBJECT (button), "ObjectLine", objectLine);
	g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(spin_line_color_changed_value_object), xyplot);

	g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(combo_line_style_changed_value_object), xyplot);
	g_object_set_data(G_OBJECT (combo), "ObjectLine", objectLine);

	g_object_set_data(G_OBJECT (spin_arrow), "ObjectLine", objectLine);
	g_signal_connect(G_OBJECT(spin_arrow), "value-changed", G_CALLBACK(spin_arrow_size_changed_value_object), xyplot);


	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
	/* list_utf8();*/
	

}
/**************************************************************************************************/
static void set_object_image_relative(GabeditXYPlot *xyplot, XYPlotObjectImage* objectImage)
{
	gdouble f = (gdouble)GTK_WIDGET(xyplot)->allocation.width;
  	gint xi=objectImage->xi-xyplot->plotting_rect.x;
  	gint yi=xyplot->plotting_rect.y+xyplot->plotting_rect.height-objectImage->yi; 
	if(f>(gdouble)GTK_WIDGET(xyplot)->allocation.height) f = (gdouble)GTK_WIDGET(xyplot)->allocation.height;
	objectImage->width = (gdouble)objectImage->widthi/f;
	objectImage->height = (gdouble)objectImage->heighti/f;
	/*
	objectImage->x = (gdouble)objectImage->xi/f;
	objectImage->y = (gdouble)objectImage->yi/f;
	*/
        pixel2value(xyplot, xi, yi, &objectImage->x, &objectImage->y);
	/* printf("xy = %f %f\n",objectImage->x, objectImage->y);*/
}
/**************************************************************************************************/
static void set_object_image_pixels(GabeditXYPlot *xyplot, XYPlotObjectImage* objectImage)
{
	gint xi,yi;
	gdouble f = (gdouble)GTK_WIDGET(xyplot)->allocation.width;
	if(f>(gdouble)GTK_WIDGET(xyplot)->allocation.height) f = (gdouble)GTK_WIDGET(xyplot)->allocation.height;
	objectImage->widthi = (gint)(objectImage->width*f);
	objectImage->heighti = (gint)(objectImage->height*f);
	/*
	objectImage->xi = (gint)(objectImage->x*f);
	objectImage->yi = (gint)(objectImage->y*f);
	*/

       	value2pixel(xyplot, objectImage->x, objectImage->y, &xi, &yi);
       	objectImage->xi = xi;
	objectImage->yi = yi;
	objectImage->yi=xyplot->plotting_rect.height-objectImage->yi;
	objectImage->xi += xyplot->plotting_rect.x;
	objectImage->yi += xyplot->plotting_rect.y;
}
/****************************************************************************************/
static void set_object_image(GabeditXYPlot *xyplot, XYPlotObjectImage* objectImage, gint xi, gint yi, gint w, gint h)
{
	objectImage->xi = xi;
	objectImage->yi = yi;
	objectImage->widthi = w;
	objectImage->heighti = h;
	set_object_image_relative(xyplot, objectImage);
}
/**************************************************************************/
static cairo_surface_t* get_image_from_clipboard()
{       
	cairo_surface_t *surface = NULL;
	GtkClipboard * clipboard = NULL;
	GdkPixbuf * pixbuf = NULL;

	clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
	if(!clipboard) return NULL;
	pixbuf = gtk_clipboard_wait_for_image(clipboard);

	if(pixbuf)
	{
		gint width;
                gint height;
		gint stride;
		gint x,y;
		gint nChannels;
		guchar *p, *pixels;
      		gint red, green, blue;
      		gfloat alpha;

		if (!gdk_pixbuf_get_has_alpha (pixbuf))
      		{
            		GdkPixbuf* newPixbuf = gdk_pixbuf_add_alpha (pixbuf, FALSE, 255, 255, 255);
			if (newPixbuf != pixbuf)
			{
				g_object_unref(pixbuf);
				pixbuf = newPixbuf;
			}
		}
		width = gdk_pixbuf_get_width (pixbuf);
                height = gdk_pixbuf_get_height (pixbuf);
		stride = gdk_pixbuf_get_rowstride(pixbuf);
		nChannels = gdk_pixbuf_get_n_channels (pixbuf);
		pixels = gdk_pixbuf_get_pixels (pixbuf);
			
		for (y = 0; y < height; y++)
      		{
            		for (x = 0; x < width; x++)
            		{
                  		p = pixels + y * stride + x * nChannels;
                  		alpha = (gfloat) p[3] / 255;
                  		red = p[0] * alpha;
                  		green = p[1] * alpha;
                  		blue = p[2] * alpha;
                  		p[0] = blue;
                  		p[1] = green;
                  		p[2] = red;
            		}
      		}

		surface = cairo_image_surface_create_for_data(
					pixels,
					CAIRO_FORMAT_ARGB32,
                                       	width,
                                       	height,
                                       	stride);
		/*g_object_unref(pixbuf);*/
	}
	return surface;
}
/**************************************************************************************************/
static XYPlotObjectImage get_object_image(GabeditXYPlot *xyplot, gint xi, gint yi, G_CONST_RETURN gchar* fileName)
{
	gint w;
	gint h;
	gint nw;
	gint nh;
	gdouble fw = 1;
	gdouble fh = 1;
	gdouble f = 1;
	XYPlotObjectImage objectImage;
	objectImage.widthi=0;
	objectImage.heighti=0;
	objectImage.xi=xi;
	objectImage.yi=yi;
	if(fileName) objectImage.fileName = g_strdup(fileName);
	else objectImage.fileName = NULL;
	objectImage.image=NULL;
	if(fileName) 
	{
		objectImage.image = cairo_image_surface_create_from_png (objectImage.fileName);
	}
	else
	{
		objectImage.image = get_image_from_clipboard();
	}
        if(!objectImage.image) return objectImage;
        w = cairo_image_surface_get_width (objectImage.image);
	h = cairo_image_surface_get_height (objectImage.image);

	nw = xyplot->plotting_rect.width-xi;
	nh = xyplot->plotting_rect.height-yi;
	if(nw<w && nw>0) fw = nw/(gdouble)w;
	if(nh<h&&nh>0) fh = nh/(gdouble)h;
	f = fw;
	if(f>fh) f = fh;

	objectImage.widthi = (gint)(w*f);
	objectImage.heighti = (gint)(h*f);
	set_object_image_relative(xyplot, &objectImage);

	/* printf("w = %d h = %d\n",w, h);*/
	return objectImage;
}
/****************************************************************************************/
static void add_object_image(GabeditXYPlot *xyplot, gint xi, gint yi, gint width, gint height, G_CONST_RETURN gchar* fileName)
{
	XYPlotObjectImage objectImage = get_object_image(xyplot, xi, yi, fileName);
	if(!objectImage.image) return;
	xyplot->nObjectsImage++;
	if(xyplot->nObjectsImage==1) xyplot->objectsImage = g_malloc(sizeof(XYPlotObjectImage));
	else xyplot->objectsImage = g_realloc(xyplot->objectsImage,xyplot->nObjectsImage*sizeof(XYPlotObjectImage));
	if(width>0 && height>0)
	{
		objectImage.widthi=width;
		objectImage.heighti=height;
	}
	xyplot->objectsImage[xyplot->nObjectsImage-1] = objectImage;
}
/****************************************************************************************/
static gint get_object_image_num(GabeditXYPlot *xyplot, gint xi, gint yi)
{
	gint i;
	if(!xyplot) return -1;
	for(i=0;i<xyplot->nObjectsImage;i++)
	{
		if(xi>=(gint)xyplot->objectsImage[i].xi && yi>=xyplot->objectsImage[i].yi
		&& xi<=xyplot->objectsImage[i].xi+xyplot->objectsImage[i].widthi 
		&& yi<=xyplot->objectsImage[i].yi+xyplot->objectsImage[i].heighti)
			return i;
	}
	return -1;
}
/****************************************************************************************/
static void delete_object_image(GtkWidget *widget, gint i)
{
	gint j;
	GabeditXYPlot *xyplot = NULL;
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	xyplot = GABEDIT_XYPLOT(widget);
	if(i<0||i>=xyplot->nObjectsImage) return;
	if(xyplot->objectsImage[i].fileName) g_free(xyplot->objectsImage[i].fileName);
	if(xyplot->objectsImage[i].image) cairo_surface_destroy (xyplot->objectsImage[i].image);
	for(j=i;j<xyplot->nObjectsImage-1;j++)
		xyplot->objectsImage[j] = xyplot->objectsImage[j+1];
	xyplot->nObjectsImage--;
	if(xyplot->nObjectsImage<1) 
	{
		xyplot->nObjectsImage = 0;
		if(xyplot->objectsImage) g_free(xyplot->objectsImage);
		xyplot->objectsImage = NULL;
	}
	else
	{
		xyplot->objectsImage = g_realloc(xyplot->objectsImage,xyplot->nObjectsImage*sizeof(XYPlotObjectImage));
	}

}
/****************************************************************************************/
static void delete_objects_image(GtkWidget *widget)
{
	GabeditXYPlot *xyplot = NULL;
	gint i;
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	xyplot = GABEDIT_XYPLOT(widget);
	for(i=0;i<xyplot->nObjectsImage;i++)
	{
		if(xyplot->objectsImage[i].fileName) g_free(xyplot->objectsImage[i].fileName);
		if(xyplot->objectsImage[i].image) cairo_surface_destroy (xyplot->objectsImage[i].image);
	}
	xyplot->nObjectsImage = 0;
	if(xyplot->objectsImage) g_free(xyplot->objectsImage);
	xyplot->objectsImage = NULL;
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/********************************************************************************/
static gboolean read_image_png(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
	GtkWidget* xyplot = NULL;
	XYPlotObjectImage* objectImage = NULL;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gtk_file_chooser_get_filename(filesel);
	xyplot = g_object_get_data(G_OBJECT (filesel), "XYPLOT");
	objectImage = g_object_get_data(G_OBJECT(filesel),"ObjectImage");
	if(objectImage && !objectImage->image)
	{
		add_object_image(GABEDIT_XYPLOT(xyplot), objectImage->xi, objectImage->yi, -1, -1, fileName);
		g_free(objectImage);
	}
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	return TRUE;

}
/********************************************************************************/
static void activate_entry_object_image(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	GtkWidget* xyplot = NULL;
	GtkWidget* window = NULL;
	XYPlotObjectImage* objectImage = NULL;


	if(!GTK_IS_WIDGET(entry)) return;
	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	if(!entry) return;
	window = g_object_get_data(G_OBJECT(entry),"Window");
	objectImage = g_object_get_data(G_OBJECT(entry),"ObjectImage");
	/* t is destroyed with window */
	if(!objectImage) 
	{
		if(window)gtk_widget_destroy(window);
		return;
	}
	else
	{
		objectImage->widthi =(gint)(fabs(atof(t))*cairo_image_surface_get_width (objectImage->image));
		objectImage->heighti =(gint)(fabs(atof(t))*cairo_image_surface_get_height (objectImage->image));
	}
	if(window)gtk_widget_destroy(window);
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/********************************************************************************/
static void add_set_object_image_dialog(GtkWidget* xyplot, gint i, gint xi, gint yi)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.png","*",NULL}; 
	GtkWidget* filesel= NULL;
	XYPlotObjectImage* objectImage = NULL;

	if(i>-1 && i<GABEDIT_XYPLOT(xyplot)->nObjectsImage)
	{
		GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		GtkWidget* frame = NULL;
		GtkWidget* hbox = NULL;
		GtkWidget* vbox_frame = NULL;
		GtkWidget* label = NULL;
		GtkWidget* entry_label = NULL;

		objectImage = &GABEDIT_XYPLOT(xyplot)->objectsImage[i];

		gtk_window_set_title (GTK_WINDOW (window), _("Scale image"));
		gtk_container_set_border_width (GTK_CONTAINER (window), 10);
		gtk_window_set_modal (GTK_WINDOW (window), TRUE);

		g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)xyplot);
	
		hbox=gtk_hbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(window), hbox);
		gtk_widget_show(hbox);

		frame=gtk_frame_new(NULL);
		gtk_container_add(GTK_CONTAINER(hbox), frame);
		gtk_widget_show(frame);

		vbox_frame=gtk_vbox_new(FALSE, 0);
		gtk_container_add(GTK_CONTAINER(frame), vbox_frame);
		gtk_widget_show(vbox_frame);

		hbox=gtk_hbox_new(FALSE, 0);
		gtk_box_pack_start(GTK_BOX(vbox_frame), hbox, FALSE, FALSE, 2);
		gtk_widget_show(hbox);

		label=gtk_label_new(_("Factor (new size/orginal size) : "));
		gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
		gtk_widget_show(label); 

		entry_label = gtk_entry_new();
		/* gtk_widget_set_size_request(entry_label,100,-1);*/

		gtk_entry_set_text(GTK_ENTRY(entry_label),"0.5");

		gtk_box_pack_start(GTK_BOX(hbox), entry_label, TRUE, TRUE, 2);
		gtk_widget_show(entry_label); 
	
		g_object_set_data(G_OBJECT(entry_label),"ObjectImage", objectImage);

		g_object_set_data(G_OBJECT(entry_label),"Window", window);
		g_signal_connect (G_OBJECT (entry_label), "activate", (GCallback)activate_entry_object_image, xyplot);

		parentWindow = get_parent_window(GTK_WIDGET(xyplot));
		if(parentWindow)
		{
			gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
		}
		gtk_widget_show(window); 
	}
	else if(i==-1)
	{
		objectImage = g_malloc(sizeof(XYPlotObjectImage));
		objectImage->xi = xi;
		objectImage->yi = yi;
		objectImage->fileName = NULL;
		objectImage->image = NULL;
		parentWindow = get_parent_window(GTK_WIDGET(xyplot));
		filesel= new_file_chooser_open(parentWindow, 
				(GCallback *)read_image_png, 
				_("Read image from a png file"), 
				patternsfiles);
		gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
		g_object_set_data(G_OBJECT (filesel), "XYPLOT", xyplot);
		g_object_set_data(G_OBJECT(filesel),"ObjectImage", objectImage);
	}
	else
		add_object_image(GABEDIT_XYPLOT(xyplot), xi, yi, -1, -1, NULL);

}
/****************************************************************************************/
static void reset_object_image_pixels(GabeditXYPlot *xyplot)
{
	gint i;
	if(!xyplot) return;
	for(i=0;i<xyplot->nObjectsImage;i++)
		set_object_image_pixels(xyplot, &xyplot->objectsImage[i]);
}
/****************************************************************************************/
/*
static void reset_object_image_relative(GabeditXYPlot *xyplot)
{
	gint i;
	if(!xyplot) return;
	for(i=0;i<xyplot->nObjectsImage;i++)
		set_object_image_relative(xyplot, &xyplot->objectsImage[i]);
}
*/
/****************************************************************************************/
static void insert_objects_image_from_clipboard(GtkWidget *widget)
{
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	add_object_image(GABEDIT_XYPLOT(widget), 0, 0, -1, -1, NULL);
        gtk_widget_queue_draw(widget);
}
/****************************************************************************************/
static void begin_insert_objects_image(GtkWidget *widget)
{
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	GABEDIT_XYPLOT(widget)->i_key_pressed = TRUE;
}
/********************************************************************************/
static gchar** my_strsplit(gchar *str)
{
	gchar** strsplit= g_malloc(sizeof(gchar*));
	gint n=0;
	gchar* t=str;
	gchar p[BSIZE];
	while(*t!='\n' && *t !='\0')
	{
		if(*t!=' ')
		{
			n++;
			strsplit= g_realloc(strsplit,(n+1)*sizeof(gchar*));
			sscanf(t,"%s",p);
			strsplit[n-1]= g_strdup(p);
			while(*t!=' ')
			{
				t++;
				if(*t =='\n' || *t =='\0')
					break;
			}

		}
		else
		{
			while(*t ==' ' )
			{
				t++;
				if(*t =='\n' || *t =='\0')
					break;
			}
		}
	}
	strsplit[n]= NULL;
	return strsplit;
}
/********************************************************************************/
static gboolean read_data_xy1yncolumns(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
 	gchar t[BSIZE];
 	FILE *fd;
	GtkWidget* xyplot = NULL;
	gchar** strsplit;
	gchar* str = NULL;
	gint* numberOfPoints = NULL;
	gdouble** X = NULL;
	gdouble** Y = NULL;
	gint nmax = 0;
	gint i;
	gint k;
	gdouble x=0;
	gboolean doPeaks = FALSE;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gtk_file_chooser_get_filename(filesel);
	xyplot = g_object_get_data(G_OBJECT (filesel), "XYPLOT");
	doPeaks = GPOINTER_TO_INT(g_object_get_data(G_OBJECT (filesel), "DoPeaks"));

 	fd = fopen(fileName, "rb");

    	if(fgets(t,BSIZE,fd) && strcmp(t,"\n"))
	{
		for(i=0;i<strlen(t);i++)
		{
			if(t[i]==',')t[i]=' ';
			if(t[i]=='\t')t[i]=' ';
		}
		strsplit = my_strsplit(t);
		while(strsplit && strsplit[nmax]) nmax++;
		nmax--;
		if(nmax>0)
		{
			Y = g_malloc(nmax*sizeof(gdouble*));
			X = g_malloc(nmax*sizeof(gdouble*));
			numberOfPoints = g_malloc(nmax*sizeof(gint));
			for(k=0;k<nmax;k++)
			{
				numberOfPoints[k] = 1;
				X[k] = g_malloc(1*sizeof(gdouble));
				X[k][0] = atof(strsplit[0]);
				Y[k] = g_malloc(1*sizeof(gdouble));
				Y[k][0] = atof(strsplit[k+1]);
			}
		}
		if(strsplit) g_strfreev(strsplit);
	}
  	while(nmax>0 && !feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(!strcmp(t,"\n")) break;
		for(i=0;i<strlen(t);i++)
		{
			if(t[i]==',')t[i]=' ';
			if(t[i]=='\t')t[i]=' ';
		}
		strsplit = my_strsplit(t);
		if(!strsplit) break;
		if(!strsplit[0]) break;
		if(!strsplit[1]) break;
		i = 0;
		do{
			str=strsplit[i];
			if(!str) break;
			i++;
			if(i==1) {
				x = atof(str);
				continue;
			}
			k = i-2;
			if(k>=nmax) break;
			numberOfPoints[k]++;
			X[k] = g_realloc(X[k], numberOfPoints[k]*sizeof(gdouble));
			Y[k] = g_realloc(Y[k], numberOfPoints[k]*sizeof(gdouble));
			X[k][numberOfPoints[k]-1] = x;
			Y[k][numberOfPoints[k]-1] = atof(str);
		}while(str!=NULL);

		g_strfreev(strsplit);
	}
	if(nmax>0)
	{
		for(k=0;k<nmax;k++)
		{
			if(numberOfPoints[k]>0 && X[k] && Y[k])
			{
				if(doPeaks) add_new_data_peaks(xyplot, numberOfPoints[k], X[k],  Y[k]);
				else add_new_data(xyplot, numberOfPoints[k], X[k],  Y[k]);
			}
			if(X[k]) g_free(X[k]);
			if(Y[k]) g_free(Y[k]);
		}
	}
	else
	{
		GtkWidget* parentWidget = get_parent_window(GTK_WIDGET(xyplot));
		GtkWindow* parentWindow = NULL;
		if(parentWidget) parentWindow = GTK_WINDOW(parentWidget);
		GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(parentWindow),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
			       _("Error reading file '%s'"),
				fileName);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}

	if(numberOfPoints) g_free(numberOfPoints);
	if(X) g_free(X);
	if(Y) g_free(Y);
	fclose(fd);
	return TRUE;

}
/********************************************************************************/
static void read_data_xy1yncolumns_dlg(GtkWidget* xyplot, gboolean doPeaks)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.txt","*",NULL}; 
	GtkWidget* filesel= NULL;

	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	filesel= new_file_chooser_open(parentWindow, 
			(GCallback *)read_data_xy1yncolumns, 
			_("Read data from an ASCII X.Y1..Yn file(x, y1, y2,...,yn)"), 
			patternsfiles);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_object_set_data(G_OBJECT (filesel), "XYPLOT", xyplot);
	g_object_set_data(G_OBJECT (filesel), "DoPeaks",  GINT_TO_POINTER(doPeaks));
}
/********************************************************************************/
static gboolean this_is_a_backspace(gchar *st)
{
        gint i;
        for(i=0;i<(gint)strlen(st);i++)
        	if(st[i] != ' ' && st[i] !='\n' && st[i] !='\r')
                	return FALSE;
        return TRUE;
}   
/********************************************************************************/
static gboolean read_data_1column(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
 	gchar t[BSIZE];
 	gboolean OK;
 	FILE *fd;
	gint ne;
	gint numberOfPoints = 0;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble a;
	gdouble b;
	GtkWidget* xyplot = NULL;
	gint nData= 0;
	gboolean doPeaks = FALSE;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gtk_file_chooser_get_filename(filesel);
	xyplot = g_object_get_data(G_OBJECT (filesel), "XYPLOT");
	doPeaks = GPOINTER_TO_INT(g_object_get_data(G_OBJECT (filesel), "DoPeaks"));

 	fd = fopen(fileName, "rb");
 	OK=FALSE;

  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(this_is_a_backspace(t))
		{
			if(numberOfPoints==0) continue;
			add_new_data(xyplot, numberOfPoints, X,  Y);
			numberOfPoints = 0;
			if(X) g_free(X);
			if(Y) g_free(Y);
			X = NULL;
			Y = NULL;
			nData ++;
		}
		ne = sscanf(t,"%lf",&b);
		if(ne==1)
		{
			numberOfPoints++;
			a = numberOfPoints;
			X = g_realloc(X, numberOfPoints*sizeof(gdouble));
			Y = g_realloc(Y, numberOfPoints*sizeof(gdouble));
			X[numberOfPoints-1] = a;
			Y[numberOfPoints-1] = b;
		}
	}
	if(numberOfPoints>0)
	{
		if(doPeaks) add_new_data_peaks(xyplot, numberOfPoints, X,  Y);
		else add_new_data(xyplot, numberOfPoints, X,  Y);
	}
	else if(nData == 0)
	{
		GtkWidget* parentWidget = get_parent_window(GTK_WIDGET(xyplot));
		GtkWindow* parentWindow = NULL;
		if(parentWidget) parentWindow = GTK_WINDOW(parentWidget);
		GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(parentWindow),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
			       _("Error reading file '%s'"),
				fileName);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}

	if(X) g_free(X);
	if(Y) g_free(Y);
	fclose(fd);
	return TRUE;

}
/********************************************************************************/
static void read_data_1column_dlg(GtkWidget* xyplot, gboolean doPeaks)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.txt","*",NULL}; 
	GtkWidget* filesel= NULL;

	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	filesel= new_file_chooser_open(parentWindow, 
			(GCallback *)read_data_1column, 
			_("Read data from an ASCII Y file(1 column(Y), X=#ligne)"), 
			patternsfiles);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_object_set_data(G_OBJECT (filesel), "XYPLOT", xyplot);
	g_object_set_data(G_OBJECT (filesel), "DoPeaks",  GINT_TO_POINTER(doPeaks));
}
/********************************************************************************/
static gboolean read_data_2columns(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
 	gchar t[BSIZE];
 	gboolean OK;
 	FILE *fd;
	gint ne;
	gint numberOfPoints = 0;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gdouble a;
	gdouble b;
	GtkWidget* xyplot = NULL;
	gint nData= 0;
	gboolean doPeaks = FALSE;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gtk_file_chooser_get_filename(filesel);
	xyplot = g_object_get_data(G_OBJECT (filesel), "XYPLOT");
	doPeaks = GPOINTER_TO_INT(g_object_get_data(G_OBJECT (filesel), "DoPeaks"));

 	fd = fopen(fileName, "rb");
 	OK=FALSE;

  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(this_is_a_backspace(t))
		{
			if(numberOfPoints==0) continue;
			add_new_data(xyplot, numberOfPoints, X,  Y);
			numberOfPoints = 0;
			if(X) g_free(X);
			if(Y) g_free(Y);
			X = NULL;
			Y = NULL;
			nData ++;
		}
		ne = sscanf(t,"%lf %lf",&a, &b);
		if(ne==2)
		{
			numberOfPoints++;
			X = g_realloc(X, numberOfPoints*sizeof(gdouble));
			Y = g_realloc(Y, numberOfPoints*sizeof(gdouble));
			X[numberOfPoints-1] = a;
			Y[numberOfPoints-1] = b;
		}
	}
	if(numberOfPoints>0)
	{
		if(doPeaks) add_new_data_peaks(xyplot, numberOfPoints, X,  Y);
		else add_new_data(xyplot, numberOfPoints, X,  Y);
	}
	else if(nData == 0)
	{
		GtkWidget* parentWidget = get_parent_window(GTK_WIDGET(xyplot));
		GtkWindow* parentWindow = NULL;
		if(parentWidget) parentWindow = GTK_WINDOW(parentWidget);
		GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(parentWindow),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
			       _("Error reading file '%s'"),
				fileName);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}

	if(X) g_free(X);
	if(Y) g_free(Y);
	fclose(fd);
	return TRUE;

}
/********************************************************************************/
static void read_data_2columns_dlg(GtkWidget* xyplot, gboolean doPeaks)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.txt","*",NULL}; 
	GtkWidget* filesel= NULL;

	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	filesel= new_file_chooser_open(parentWindow, 
			(GCallback *)read_data_2columns, 
			_("Read data from an ASCII XY file(2 columns)"), 
			patternsfiles);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_object_set_data(G_OBJECT (filesel), "XYPLOT", xyplot);
	g_object_set_data(G_OBJECT (filesel), "DoPeaks",  GINT_TO_POINTER(doPeaks));
}
/********************************************************************************/
static gboolean save_data_2columns(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
 	FILE *file;
	XYPlotData* data = NULL;
	gint loop;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gtk_file_chooser_get_filename(filesel);
	data = g_object_get_data(G_OBJECT (filesel), "CurentData");
	if(!data) return FALSE;

 	file = fopen(fileName, "w");

	for(loop=0;loop<data->size; loop++)
	{
		fprintf(file, "%f %f\n",data->x[loop], data->y[loop]);
	}
	fclose(file);
	return TRUE;

}
/********************************************************************************/
static gboolean save_all_data_2columns(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
 	FILE *file;
	XYPlotData* data = NULL;
	gint loop;
	GList *current = NULL;
	GList *data_list = NULL;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gtk_file_chooser_get_filename(filesel);

	data_list = g_object_get_data(G_OBJECT (filesel), "DataList");


 	file = fopen(fileName, "w");

	if(data_list)
	for(current=g_list_first(data_list); current!=NULL; current=current->next)
	{
		data=(XYPlotData*)current->data;
		if(!data) continue;
		for(loop=0;loop<data->size; loop++)
			fprintf(file, "%f %f\n",data->x[loop], data->y[loop]);
		fprintf(file, "\n");
  	}
	fclose(file);
	return TRUE;

}
/********************************************************************************/
static gint get_data_jdx_type(FILE* fd, gdouble* scaleX, gdouble* scaleY, gdouble* deltaX)
{
 	gchar t[BSIZE];
	*scaleX = 1;
	*scaleY = 1;
	*deltaX = 0;
  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"##XFACTOR="))
		{
			gchar* e = strstr(t,"=")+1;
			if(e) *scaleX = atof(e);
		}
		if(strstr(t,"##YFACTOR="))
		{
			gchar* e = strstr(t,"=")+1;
			if(e) *scaleY = atof(e);
		}
		if(strstr(t,"##DELTAX="))
		{
			gchar* e = strstr(t,"=")+1;
			if(e) *deltaX = atof(e);
		}
		if(strstr(t,"##XYDATA="))
		{
			return JDX_TYPE_XYDATA;
		}
		if(strstr(t,"##XYPOINTS="))
		{
			return JDX_TYPE_XYPOINTS;
		}
		if(strstr(t,"##PEAK TABLE=") || strstr(t,"##PEAKTABLE="))
		{
			return JDX_TYPE_XYTABLE;
		}
	}
	return JDX_TYPE_UNKNOWN;

}
/********************************************************************************/
static gboolean read_data_jdx_xypoints(FILE* fd, GtkWidget*xyplot, gdouble scaleX, gdouble scaleY, gchar* fileName )
{
 	gchar t[BSIZE];
	gint numberOfPoints = 0;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gchar** strsplit;
	gchar* str1 = NULL;
	gchar* str2 = NULL;
	gint i;

  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(!strcmp(t,"\n")) break;
		if(strstr(t,"##END")) break;
		for(i=0;i<strlen(t);i++)
		{
			if(t[i]==',')t[i]=' ';
			if(t[i]=='\t')t[i]=' ';
		}
		strsplit = my_strsplit(t);
		if(!strsplit) break;
		if(!strsplit[0]) break;
		if(!strsplit[1]) break;
		i = 0;
		do{
			str1=strsplit[i];
			if(!str1) break;
			str2=strsplit[i+1];
			if(!str2) break;
			i++;
			numberOfPoints++;
			X = g_realloc(X, numberOfPoints*sizeof(gdouble));
			Y = g_realloc(Y, numberOfPoints*sizeof(gdouble));
			X[numberOfPoints-1] = atof(str1)*scaleX;
			Y[numberOfPoints-1] = atof(str2)*scaleY;
		}while(str2!=NULL);

		g_strfreev(strsplit);
	}
	if(numberOfPoints>0)
	{
		add_new_data(xyplot, numberOfPoints, X,  Y);
	}
	else
	{
		GtkWidget* parentWidget = get_parent_window(GTK_WIDGET(xyplot));
		GtkWindow* parentWindow = NULL;
		if(parentWidget) parentWindow = GTK_WINDOW(parentWidget);
		GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(parentWindow),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
			       _("Error reading file '%s'"),
				fileName);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}

	if(X) g_free(X);
	if(Y) g_free(Y);
	return TRUE;

}
/********************************************************************************/
static gboolean read_data_jdx_xydata(FILE* fd, GtkWidget*xyplot , gdouble scaleX, gdouble scaleY, gdouble deltaX , gchar* fileName)
{
 	gchar t[BSIZE];
	gint numberOfPoints = 0;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gchar** strsplit;
	gchar* str = NULL;
	gdouble x=0;
	gint i;

  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(!strcmp(t,"\n")) break;
		if(strstr(t,"##END")) break;
		for(i=0;i<strlen(t);i++)
		{
			if(t[i]==',')t[i]=' ';
			if(t[i]=='\t')t[i]=' ';
		}
		strsplit = my_strsplit(t);
		if(!strsplit) break;
		if(!strsplit[0]) break;
		if(!strsplit[1]) break;
		i = 0;
		do{
			str=strsplit[i];
			if(!str) break;
			i++;
			if(i==1) {
				x = atof(str);
				continue;
			}
			if(i>2) x += deltaX;
			numberOfPoints++;
			X = g_realloc(X, numberOfPoints*sizeof(gdouble));
			Y = g_realloc(Y, numberOfPoints*sizeof(gdouble));
			X[numberOfPoints-1] = x*scaleX;
			Y[numberOfPoints-1] = atof(str)*scaleY;
		}while(str!=NULL);

		g_strfreev(strsplit);
	}
	if(numberOfPoints>0)
	{
		add_new_data(xyplot, numberOfPoints, X,  Y);
	}
	else
	{
		GtkWidget* parentWidget = get_parent_window(GTK_WIDGET(xyplot));
		GtkWindow* parentWindow = NULL;
		if(parentWidget) parentWindow = GTK_WINDOW(parentWidget);
		GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(parentWindow),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
			       _("Error reading file '%s'"),
				fileName);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}

	if(X) g_free(X);
	if(Y) g_free(Y);
	return TRUE;

}
/********************************************************************************/
static gboolean read_data_jdx_xytable(FILE* fd, GtkWidget*xyplot , gdouble scaleX, gdouble scaleY, gchar* fileName)
{
 	gchar t[BSIZE];
	gint numberOfPoints = 0;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gchar** strsplit;
	gchar* str = NULL;
	gdouble x=0;
	gint i;

  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(!strcmp(t,"\n")) break;
		if(strstr(t,"##END")) break;
		for(i=0;i<strlen(t);i++)
		{
			if(t[i]==',')t[i]=' ';
			if(t[i]=='\t')t[i]=' ';
		}
		strsplit = my_strsplit(t);
		if(!strsplit) break;
		if(!strsplit[0]) break;
		if(!strsplit[1]) break;
		i = 0;
		do{
			str=strsplit[i];
			if(!str) break;
			i++;
			if(i%2==1) x = atof(str);
			else 
			{
				gint i0  = numberOfPoints;
				numberOfPoints += 3;
				X = g_realloc(X, numberOfPoints*sizeof(gdouble));
				Y = g_realloc(Y, numberOfPoints*sizeof(gdouble));
				X[i0] = x*scaleX;
				Y[i0] = 0;
				X[i0+1] = x*scaleX;
				Y[i0+1] = atof(str)*scaleY;
				X[i0+2] = x*scaleX;
				Y[i0+2] = 0;
			}
		}while(str!=NULL);

		g_strfreev(strsplit);
	}
	if(numberOfPoints>0)
	{
		add_new_data(xyplot, numberOfPoints, X,  Y);
	}
	else
	{
		GtkWidget* parentWidget = get_parent_window(GTK_WIDGET(xyplot));
		GtkWindow* parentWindow = NULL;
		if(parentWidget) parentWindow = GTK_WINDOW(parentWidget);
		GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(parentWindow),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
			       _("Error reading file '%s'"),
				fileName);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}

	if(X) g_free(X);
	if(Y) g_free(Y);
	return TRUE;

}
/********************************************************************************/
static gboolean read_data_jdx(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
 	FILE *fd;
	GtkWidget* xyplot = NULL;
	gint type = 0;
	gdouble scaleX;
	gdouble scaleY;
	gdouble deltaX;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gtk_file_chooser_get_filename(filesel);
	xyplot = g_object_get_data(G_OBJECT (filesel), "XYPLOT");

 	fd = fopen(fileName, "rb");
	type = get_data_jdx_type(fd, &scaleX, &scaleY, &deltaX);
	if(type==JDX_TYPE_XYPOINTS) read_data_jdx_xypoints(fd, xyplot, scaleX, scaleY, fileName);
	if(type==JDX_TYPE_XYDATA) read_data_jdx_xydata(fd,xyplot, scaleX, scaleY,deltaX, fileName);
	if(type==JDX_TYPE_XYTABLE) read_data_jdx_xytable(fd,xyplot, scaleX, scaleY, fileName);
	if(type==JDX_TYPE_UNKNOWN)
	{
		GtkWidget* parentWidget = get_parent_window(GTK_WIDGET(xyplot));
		GtkWindow* parentWindow = NULL;
		if(parentWidget) parentWindow = GTK_WINDOW(parentWidget);
		GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(parentWindow),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
			       _("Error reading file '%s', The type of data in this file is unknown"),
				fileName);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}
	fclose(fd);
	return TRUE;

}
/********************************************************************************/
static void read_data_jdx_dlg(GtkWidget* xyplot)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.jdx","*",NULL}; 
	GtkWidget* filesel= NULL;

	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	filesel= new_file_chooser_open(parentWindow, 
			(GCallback *)read_data_jdx, 
			_("Read data from a jdx file"),
			patternsfiles);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_object_set_data(G_OBJECT (filesel), "XYPLOT", xyplot);
}
/*************************************************************************************/
static void set_font (GtkWidget *view, gchar *fontname)
{
        GtkStyle *style;
  	PangoFontDescription *font_desc;
 
	if(!GTK_IS_WIDGET(view)) return;
        style = gtk_style_copy (gtk_widget_get_style (view));
  	font_desc = pango_font_description_from_string (fontname);

	if (font_desc)
	{
		/*
		pango_font_description_free (style->font_desc);
		*/
		style->font_desc = font_desc;
	}
 
        gtk_widget_set_style (GTK_WIDGET(view), style);
 
        g_object_unref (style);
}         
/********************************************************************************/
static GtkWidget* create_popup_win(gchar* label)
{
	GtkWidget *MainFrame;
	GtkWidget *Label;
	GtkWidget *hbox = gtk_hbox_new(0,FALSE);
	

	MainFrame = gtk_window_new (GTK_WINDOW_POPUP);

	/* center it on the screen*/
        gtk_window_set_position(GTK_WINDOW (MainFrame), GTK_WIN_POS_MOUSE);

	/* set up key and mound button press to hide splash screen*/

        gtk_widget_add_events(MainFrame,
                              GDK_BUTTON_PRESS_MASK|
                              GDK_BUTTON_RELEASE_MASK|
                              GDK_KEY_PRESS_MASK);

        gtk_widget_realize(MainFrame);
	Label = gtk_label_new(label);
   	gtk_label_set_justify(GTK_LABEL(Label),GTK_JUSTIFY_LEFT);
 	set_font (Label,"helvetica bold 24");  
	gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 0);
        gtk_widget_show(Label);
        gtk_widget_show(hbox);
        gtk_container_add (GTK_CONTAINER (MainFrame), hbox);
        gtk_widget_show(MainFrame);

	/* force it to draw now.*/
	gdk_flush();

	/* go into main loop, processing events.*/
        while(gtk_events_pending()) gtk_main_iteration();
	return MainFrame;
}
/****************************************************************************************/
static void build_fourier(GtkWidget* buttonDFT, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		gdouble scale = 1.0;
		gdouble temperature = 1.0;
		gdouble xmax = 6000.0;
		G_CONST_RETURN gchar* t;
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (xyplot), "CurentData");
		GtkWidget* parentWindow = ( GtkWidget*) g_object_get_data(G_OBJECT (buttonDFT), "ParentWindow");
		GtkWidget* entry_scale = ( GtkWidget*) g_object_get_data(G_OBJECT (xyplot), "Entry_scale");
		GtkWidget* entry_temperature = ( GtkWidget*) g_object_get_data(G_OBJECT (xyplot), "Entry_temperature");
		GtkWidget* entry_xmax = ( GtkWidget*) g_object_get_data(G_OBJECT (xyplot), "Entry_xmax");
		GtkWidget* popup = NULL;
		GtkComboBox *combo_correction = ( GtkComboBox*) g_object_get_data(G_OBJECT (xyplot), "ComboCorrection");
		GtkTreeIter iter;
		gchar* correction = NULL;
		GtkToggleButton *normalize_button = ( GtkToggleButton*) g_object_get_data(G_OBJECT (xyplot), "NormalizeDFTButton");

  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  		g_return_if_fail (GTK_IS_ENTRY (entry_scale));
  		g_return_if_fail (GTK_IS_ENTRY (entry_temperature));
  		g_return_if_fail (GTK_IS_ENTRY (entry_xmax));

		if(parentWindow) gtk_widget_hide (GTK_WIDGET(parentWindow));
        	while(gtk_events_pending()) gtk_main_iteration();
		popup = create_popup_win(_("Please wait"));

		t= gtk_entry_get_text(GTK_ENTRY(entry_scale));
		scale = atof(t);
		if(scale<1e-10) { printf("Warning I set scale to %f\n",1.0); scale = 1.0;}
		t= gtk_entry_get_text(GTK_ENTRY(entry_temperature));
		temperature = atof(t);
		if(temperature<1e-10) { printf("Warning I set temperature to %f\n",300.0); temperature = 300.0;}
		t= gtk_entry_get_text(GTK_ENTRY(entry_xmax));
		xmax = atof(t);

		if (gtk_combo_box_get_active_iter (combo_correction, &iter))
		{
			GtkTreeModel* model = gtk_combo_box_get_model(combo_correction);
			gtk_tree_model_get (model, &iter, 0, &correction, -1);
		}

		if(data && data->size>2)
		{
			gdouble* X = g_malloc(data->size*sizeof(gdouble)); 
			gdouble* Y = g_malloc(data->size*sizeof(gdouble)); 
			gint i;
		        GtkWidget* window = NULL;
        		GtkWidget* xyplot = NULL;
			int n = 0;
			gint N = data->size;
			gdouble* x = NULL;
			gdouble* y = NULL;

			x = data->x;
			y = data->y;
			/*
			printf("dx1 %f\n", fabs((x[N-1]-x[0])/(N-1)));
			printf("dx2 %f\n", fabs(x[1]-x[0]));
			*/
			if(fabs(fabs((x[N-1]-x[0])/(N-1))-fabs(x[1]-x[0]))>1e-10)
			{
				printf("x values are not linear. Linearisation of x\n");
				build_linear_x(data->x, data->y, data->size, &x, &y, N);
			}
	
			for(i=0;i<N; i++) 
			{
				gint k;
				double arg;
      				X[i] = 0;
      				Y[i] = 0;
      				arg = 2.0 * M_PI * (double)i / (double)N;
				if(xmax>0 && i*1.0/N*scale/fabs((x[N-1]-x[0])/(N-1))>xmax) break;
      				for (k=0;k<N;k++)
				{
         				gdouble cosarg = cos(k * arg);
         				gdouble sinarg = sin(k * arg);
         				X[i] += (y[k] * cosarg);
         				Y[i] += (y[k] * sinarg);
      				}
				n++;
   			}
			for(i=0;i<n; i++)  Y[i] = sqrt( X[i]* X[i]+Y[i]*Y[i]);
			/* time in fs, freq in cm-1 */
			for(i=0;i<n; i++)  X[i] = i*1.0/N*scale/fabs((x[N-1]-x[0])/(N-1));
			if(x && x!= data->x) g_free(x);
			if(y && y!=data->y) g_free(y);
			if(n>0) { X = g_realloc(X,n*sizeof(gdouble));  Y = g_realloc(Y,n*sizeof(gdouble));}
			if(correction)
			{
				/*gdouble conv = 1/scale*47992.36961128; 10^15Hz->Kelvin*/
				gdouble conv = 1.43877505;/* cm-1=>Kelvin*/
				gdouble fac = conv/temperature;
				gdouble fac2 = fac/2;
				if(strstr(correction,"Classic")) for(i=0;i<n; i++)  Y[i] *=  X[i]*(1-exp(-X[i]*fac));
				else if(strstr(correction,"Kubo")) for(i=0;i<n; i++)  Y[i] *= X[i]*tanh(X[i]*fac2); 
				else if(strstr(correction,"Harmonic")) for(i=0;i<n; i++)  Y[i] *=  X[i]*(X[i]*fac); 
				else if(strstr(correction,"Schofield")) for(i=0;i<n; i++)  Y[i] *= X[i]*(1-exp(-X[i]*fac))*exp(X[i]*fac2);

				if(gtk_toggle_button_get_active(normalize_button))
				{
					gdouble norm = 0;
					for(i=0;i<n; i++)  norm+=Y[i];
					if(norm==0) norm = 1;
					norm = 1.0/norm;
					for(i=0;i<n; i++)  Y[i] *=  norm;
				}
			}
			

			window = gabedit_xyplot_new_window(_("DFT"),NULL);
			xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
			xyplot_curve_noconv(GABEDIT_XYPLOT(xyplot), n, X,  Y, NULL);
			gabedit_xyplot_set_range_xmin (GABEDIT_XYPLOT(xyplot), 0.0);
			gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), "cm<sup>-1</sup>");
		}
		if(parentWindow) gtk_object_destroy (GTK_OBJECT(parentWindow));
		if(popup) gtk_object_destroy(GTK_OBJECT(popup));
	}
}
/********************************************************************************/
static gboolean get_one_data_jMRUI(FILE*fd, gint idata, gint numberOfPoints, gdouble*Y)
{
 	gchar t[BSIZE];
	gboolean Ok = FALSE;
	gint k = 0;
	gdouble dum;
  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"Signal"))
		{
			k = atoi(strstr(t,"Signal")+1);
			if(k==idata)
			{
				Ok = TRUE;
				break;
			}
		}
	}
	if(!Ok) return FALSE;
	for(k = 0;k<numberOfPoints;k++)
	{
    		if(!fgets(t,BSIZE,fd)) break;
		sscanf(t,"%lf %lf %lf",&dum, &dum, &Y[k]);
	}
	if(k!=numberOfPoints)
		return FALSE;
	return TRUE;
}
/********************************************************************************/
static gboolean read_data_jMRUI(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
 	FILE *fd;
	GtkWidget* xyplot = NULL;


 	gchar t[BSIZE];
	gint numberOfPoints = 0;
	gdouble* X = NULL;
	gdouble* Y = NULL;
	gint i;
	gdouble dx = -1;
	gint nData = 0;
	gdouble f = -1;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gtk_file_chooser_get_filename(filesel);
	xyplot = g_object_get_data(G_OBJECT (filesel), "XYPLOT");

 	fd = fopen(fileName, "rb");
	if(!fd) return FALSE;

  	while(!feof(fd))
  	{
    		if(!fgets(t,BSIZE,fd)) break;
		if(strstr(t,"PointsInDataset:"))
		{
			numberOfPoints = atoi(strstr(t,":")+1);
			if(dx>0 && nData>0 && f>0) break;
		}
		if(strstr(t,"SamplingInterval:"))
		{
			dx = atof(strstr(t,":")+1);
			if(numberOfPoints!=0 && nData>0 && f>0) break;
		}
		if(strstr(t,"DatasetsInFile:"))
		{
			nData = atoi(strstr(t,":")+1);
			if(numberOfPoints!=0 && dx>0 && f>0) break;
		}
		if(strstr(t,"TransmitterFrequency:"))
		{
			f = atof(strstr(t,":")+1)/1e6;
			if(numberOfPoints!=0 && dx>0 && nData>0) break;
		}
	}
	if(numberOfPoints<1 || nData<1 || dx<=0)
	{
		GtkWidget* parentWidget = get_parent_window(GTK_WIDGET(xyplot));
		GtkWindow* parentWindow = NULL;
		if(parentWidget) parentWindow = GTK_WINDOW(parentWidget);
		GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(parentWindow),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
			       _("Error reading file '%s'"),
				fileName);
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}

	X = g_realloc(X, numberOfPoints*sizeof(gdouble));
	Y = g_realloc(Y, numberOfPoints*sizeof(gdouble));

	dx = 1/dx/f/numberOfPoints*1000;

	for(i=0;i<numberOfPoints;i++)
		X[i] = (dx*i);

	for(i=0;i<nData;i++)
	{
		if(get_one_data_jMRUI(fd,i,numberOfPoints, Y))
			add_new_data(xyplot, numberOfPoints, X,  Y);
	}


	if(X) g_free(X);
	if(Y) g_free(Y);
	return TRUE;

}
/********************************************************************************/
static void read_data_jMRUI_dlg(GtkWidget* xyplot)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.txt","*",NULL}; 
	GtkWidget* filesel= NULL;

	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	filesel= new_file_chooser_open(parentWindow, 
			(GCallback *)read_data_jMRUI, 
			_("Read data from a jMRUI Text file"), 
			patternsfiles);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_object_set_data(G_OBJECT (filesel), "XYPLOT", xyplot);
}
/********************************************************************************/
static void auto_range_activate(GtkWidget* buttonAutoRange, GtkWidget* xyplot)
{
	XYPlotData* data = NULL;
	data = g_object_get_data(G_OBJECT (buttonAutoRange), "CurentData");
	if(xyplot) gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(xyplot), data);
}
/********************************************************************************/
static void set_max_1_activate(GtkWidget* buttonMax1, GtkWidget* xyplot)
{
	XYPlotData* data = NULL;
	gdouble max = 1;
	gint i;
	if(!buttonMax1) return;
	data = g_object_get_data(G_OBJECT (buttonMax1), "CurentData");
	if(!data || data->size<1) return;
	max = fabs(data->y[0]);
	for(i=1;i<data->size;i++) if(max< fabs(data->y[i])) max =  fabs(data->y[i]);
	for(i=0;i<data->size;i++) data->y[i] /= max;
	if(xyplot) 
	{
		gdouble max = (data->y[0]);
		gdouble min = (data->y[0]);
		for(i=1;i<data->size;i++) if(max< (data->y[i])) max =  (data->y[i]);
		for(i=1;i<data->size;i++) if(min> (data->y[i])) min =  (data->y[i]);

		gabedit_xyplot_set_range_ymax (GABEDIT_XYPLOT(xyplot), max);
		gabedit_xyplot_set_range_ymin (GABEDIT_XYPLOT(xyplot), min);
	}
	if(xyplot) gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	
}
/********************************************************************************/
static void save_data_2columns_dlg(GtkWidget* buttonSave, GtkWidget* xyplot)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.txt","*",NULL}; 
	GtkWidget* filesel= NULL;
	gchar* tmp = NULL;
	gpointer* data = NULL;

	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	filesel= new_file_chooser_save(parentWindow, 
			(GCallback *)save_data_2columns, 
			_("Save data in an ASCII XY file(2 columns)"), 
			patternsfiles);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	data = g_object_get_data(G_OBJECT (buttonSave), "CurentData");
	g_object_set_data(G_OBJECT (filesel), "CurentData", data);
	tmp = g_strdup_printf("%s%s%s",g_get_current_dir(),G_DIR_SEPARATOR_S,"xyplotData.txt");
	gtk_file_chooser_set_filename ((GtkFileChooser *)filesel, tmp);
	if(tmp) g_free(tmp);
	gtk_file_chooser_set_current_name ((GtkFileChooser *)filesel, "xyplotData.txt");
}
/********************************************************************************/
static void save_all_data_2columns_dlg(GtkWidget* xyplot)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.txt","*",NULL}; 
	GtkWidget* filesel= NULL;
	gchar* tmp = NULL;

	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	filesel= new_file_chooser_save(parentWindow, 
			(GCallback *)save_all_data_2columns, 
			_("Save all data in an ASCII XY file(2 columns)"), 
			patternsfiles);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_object_set_data(G_OBJECT (filesel), "DataList", GABEDIT_XYPLOT(xyplot)->data_list);
	tmp = g_strdup_printf("%s%s%s",g_get_current_dir(),G_DIR_SEPARATOR_S,"xyplotData.txt");
	gtk_file_chooser_set_filename ((GtkFileChooser *)filesel, tmp);
	if(tmp) g_free(tmp);
	gtk_file_chooser_set_current_name ((GtkFileChooser *)filesel, "xyplotData.txt");
}
/********************************************************************************/
static gboolean remove_data(GtkWidget* buttonRemove, gint response_id)
{
	XYPlotData* data = NULL;
	GtkWidget*window = NULL;
	GtkWidget*xyplot = NULL;

	if(response_id != GTK_RESPONSE_YES) return FALSE;
	data = g_object_get_data(G_OBJECT (buttonRemove), "CurentData");
	window = g_object_get_data(G_OBJECT (buttonRemove), "Window");
	xyplot = g_object_get_data(G_OBJECT (buttonRemove), "XYPLOT");
	if(xyplot && data) 
	{
		gabedit_xyplot_remove_data(GABEDIT_XYPLOT(xyplot), data);
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
	if(window) gtk_widget_destroy(window);
	return TRUE;
}
/********************************************************************************/
static void remove_data_dlg(GtkWidget* buttonRemove, GtkWidget* xyplot)
{
	GtkWidget* parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(parentWindow),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
		       _("Are you sure to delete this data ?")
			);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_NO);
	g_signal_connect_swapped(dialog, "response", G_CALLBACK (remove_data), buttonRemove);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}
/****************************************************************************************/
static void spin_font_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		gint fontSize = gtk_spin_button_get_value(spinbutton);
		gchar tmp[100];
		sprintf(tmp,"sans %d",fontSize);
		gabedit_xyplot_set_font (GABEDIT_XYPLOT(xyplot), tmp);
		reset_object_text_pango(GABEDIT_XYPLOT(xyplot));
	}
}
/****************************************************************************************/
static void add_font_size_frame(GtkWidget* hbox, GtkWidget* xyplot)
{
	GtkWidget* frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* spin_font_size = NULL;

	frame=gtk_frame_new(_("Font size"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 2);
	gtk_widget_show(frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox_frame);
	gtk_widget_show(hbox_frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(hbox_frame), hbox);
	gtk_widget_show(hbox);

	spin_font_size=gtk_spin_button_new_with_range(8, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), spin_font_size, TRUE, FALSE, 2);
	gtk_widget_show(spin_font_size);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_font_size), GABEDIT_XYPLOT(xyplot)->font_size);
	g_signal_connect(G_OBJECT(spin_font_size), "value-changed", G_CALLBACK(spin_font_changed_value), xyplot);
}
/****************************************************************************************/
static void set_font_size_dialog(GtkWidget* xyplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* hbox = NULL;
	GtkWidget* parentWindow = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set font size"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)xyplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);

	add_font_size_frame(hbox, xyplot);


	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
}
/****************************************************************************************/
static void spin_x_digits_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		gabedit_xyplot_set_x_legends_digits(GABEDIT_XYPLOT(xyplot), gtk_spin_button_get_value(spinbutton));
	}
}
/****************************************************************************************/
static void spin_y_digits_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		gabedit_xyplot_set_y_legends_digits(GABEDIT_XYPLOT(xyplot), gtk_spin_button_get_value(spinbutton));
	}
}
/****************************************************************************************/
static void add_digits_frame(GtkWidget* hbox, GtkWidget* xyplot)
{
	GtkWidget* frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* label = NULL;
	GtkWidget* x_spin_digits = NULL;
	GtkWidget* y_spin_digits = NULL;

	frame=gtk_frame_new(_("Digits"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 2);
	gtk_widget_show(frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox_frame);
	gtk_widget_show(hbox_frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(hbox_frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_(" X "));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	x_spin_digits=gtk_spin_button_new_with_range(0, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), x_spin_digits, TRUE, FALSE, 2);
	gtk_widget_show(x_spin_digits);

	label=gtk_label_new(_(" Y "));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	y_spin_digits=gtk_spin_button_new_with_range(0, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), y_spin_digits, TRUE, FALSE, 2);
	gtk_widget_show(y_spin_digits);


	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_spin_digits), GABEDIT_XYPLOT(xyplot)->x_legends_digits);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_spin_digits), GABEDIT_XYPLOT(xyplot)->y_legends_digits);

	g_signal_connect(G_OBJECT(x_spin_digits), "value-changed", G_CALLBACK(spin_x_digits_changed_value), xyplot);
	g_signal_connect(G_OBJECT(y_spin_digits), "value-changed", G_CALLBACK(spin_y_digits_changed_value), xyplot);
}
/****************************************************************************************/
static void set_digits_dialog(GtkWidget* xyplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* hbox = NULL;
	GtkWidget* parentWindow = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set digits"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)xyplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);
	add_digits_frame(hbox, xyplot);


	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
}
/********************************************************************************/
static void activate_entry_xlabel(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	GtkWidget* xyplot = NULL;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), t);
}
/********************************************************************************/
static void activate_entry_ylabel(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	GtkWidget* xyplot = NULL;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	xyplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	gabedit_xyplot_set_y_label (GABEDIT_XYPLOT(xyplot), t);
}
/****************************************************************************************/
static void add_labels_frame(GtkWidget* hbox, GtkWidget* xyplot)
{
	GtkWidget* frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* label = NULL;
	GtkWidget* entry_x_label = NULL;
	GtkWidget* entry_y_label = NULL;

	frame=gtk_frame_new(_("Labels"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 2);
	gtk_widget_show(frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox_frame);
	gtk_widget_show(hbox_frame);

	label=gtk_label_new(_("X : "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	
	entry_x_label = gtk_entry_new();
	gtk_widget_set_size_request(entry_x_label,100,-1);
	if(GABEDIT_XYPLOT(xyplot)->h_label_str)
		gtk_entry_set_text(GTK_ENTRY(entry_x_label),GABEDIT_XYPLOT(xyplot)->h_label_str);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_x_label),"");
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_x_label, FALSE, FALSE, 2);
	gtk_widget_show(entry_x_label);

	label=gtk_label_new(_("Y : "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	
	entry_y_label = gtk_entry_new();
	gtk_widget_set_size_request(entry_y_label,100,-1);
	if(GABEDIT_XYPLOT(xyplot)->v_label_str)
		gtk_entry_set_text(GTK_ENTRY(entry_y_label),GABEDIT_XYPLOT(xyplot)->v_label_str);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_y_label),"");
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_y_label, FALSE, FALSE, 2);
	gtk_widget_show(entry_y_label);

	g_signal_connect (G_OBJECT (entry_x_label), "activate", (GCallback)activate_entry_xlabel, xyplot);
	g_signal_connect (G_OBJECT (entry_y_label), "activate", (GCallback)activate_entry_ylabel, xyplot);
}
/****************************************************************************************/
static void set_labels_dialog(GtkWidget* xyplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* parentWindow = NULL;
	GtkWidget* hbox = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set labels"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)xyplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);

	add_labels_frame(hbox, xyplot);


	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
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
}
/********************************************************************************/
static void activate_entry_ymin(GtkWidget *entry, gpointer user_data)
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

	if(a>= ymax)
	{
		gchar* tmp = g_strdup_printf("%0.3f",ymin);
		gtk_entry_set_text(GTK_ENTRY(entry),tmp);
		g_free(tmp);
		return;
	}
	gabedit_xyplot_set_range_ymin (GABEDIT_XYPLOT(xyplot), a);
}
/********************************************************************************/
static void activate_entry_ymax(GtkWidget *entry, gpointer user_data)
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

	if(a<= ymin)
	{
		gchar* tmp = g_strdup_printf("%0.3f",ymax);
		gtk_entry_set_text(GTK_ENTRY(entry),tmp);
		g_free(tmp);
		return;
	}
	gabedit_xyplot_set_range_ymax (GABEDIT_XYPLOT(xyplot), a);
}
/****************************************************************************************/
static void add_ranges_frame(GtkWidget* hbox, GtkWidget* xyplot)
{
	GtkWidget* frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* label = NULL;
	GtkWidget* entry_x_min = NULL;
	GtkWidget* entry_x_max = NULL;
	GtkWidget* entry_y_min = NULL;
	GtkWidget* entry_y_max = NULL;
	gchar tmp[100];

	frame=gtk_frame_new(_("Ranges"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 2);
	gtk_widget_show(frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox_frame);
	gtk_widget_show(hbox_frame);

	label=gtk_label_new(_("X Min: "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_x_min = gtk_entry_new();
	gtk_widget_set_size_request(entry_x_min,50,-1);
	sprintf(tmp,"%0.3f",GABEDIT_XYPLOT(xyplot)->xmin);
	gtk_entry_set_text(GTK_ENTRY(entry_x_min),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_x_min, TRUE, FALSE, 2);
	gtk_widget_show(entry_x_min);

	label=gtk_label_new(_("X Max: "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	
	entry_x_max = gtk_entry_new();
	gtk_widget_set_size_request(entry_x_max,50,-1);
	sprintf(tmp,"%0.3f",GABEDIT_XYPLOT(xyplot)->xmax);
	gtk_entry_set_text(GTK_ENTRY(entry_x_max),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_x_max, TRUE, FALSE, 2);
	gtk_widget_show(entry_x_max);

	label=gtk_label_new(_("Y Min: "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	
	entry_y_min = gtk_entry_new();
	gtk_widget_set_size_request(entry_y_min,50,-1);
	sprintf(tmp,"%0.3f",GABEDIT_XYPLOT(xyplot)->ymin);
	gtk_entry_set_text(GTK_ENTRY(entry_y_min),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_y_min, TRUE, FALSE, 2);
	gtk_widget_show(entry_y_min);

	label=gtk_label_new(_("Y Max: "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	
	entry_y_max = gtk_entry_new();
	gtk_widget_set_size_request(entry_y_max,50,-1);
	sprintf(tmp,"%0.3f",GABEDIT_XYPLOT(xyplot)->ymax);
	gtk_entry_set_text(GTK_ENTRY(entry_y_max),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_y_max, TRUE, FALSE, 2);
	gtk_widget_show(entry_y_max);

	g_signal_connect (G_OBJECT (entry_x_min), "activate", (GCallback)activate_entry_xmin, xyplot);
	g_signal_connect (G_OBJECT (entry_x_max), "activate", (GCallback)activate_entry_xmax, xyplot);
	g_signal_connect (G_OBJECT (entry_y_min), "activate", (GCallback)activate_entry_ymin, xyplot);
	g_signal_connect (G_OBJECT (entry_y_max), "activate", (GCallback)activate_entry_ymax, xyplot);
}
/****************************************************************************************/
static void set_ranges_dialog(GtkWidget* xyplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* parentWindow = NULL;
	GtkWidget* hbox = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set ranges"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)xyplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);

	add_ranges_frame(hbox, xyplot);


	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
}
/****************************************************************************************/
static void spin_hminor_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_set_ticks_hminor(GABEDIT_XYPLOT(xyplot), gtk_spin_button_get_value(spinbutton));	
	}
}
/****************************************************************************************/
static void spin_hmajor_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_set_ticks_hmajor(GABEDIT_XYPLOT(xyplot), gtk_spin_button_get_value(spinbutton));
	}
}
/****************************************************************************************/
static void spin_vminor_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_set_ticks_vminor(GABEDIT_XYPLOT(xyplot), gtk_spin_button_get_value(spinbutton));
	}
}
/****************************************************************************************/
static void spin_vmajor_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_set_ticks_vmajor(GABEDIT_XYPLOT(xyplot), gtk_spin_button_get_value(spinbutton));	
	}
}
/****************************************************************************************/
static void spin_length_ticks_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_set_ticks_length(GABEDIT_XYPLOT(xyplot), gtk_spin_button_get_value(spinbutton));	
	}
}
/****************************************************************************************/
static void add_ticks_frame(GtkWidget* hbox, GtkWidget* xyplot)
{
	GtkWidget* frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* label = NULL;
	GtkWidget* spin_hmajor = NULL;
	GtkWidget* spin_hminor = NULL;
	GtkWidget* spin_vmajor = NULL;
	GtkWidget* spin_vminor = NULL;
	GtkWidget* spin_length = NULL;

	frame=gtk_frame_new(_("Ticks"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 2);
	gtk_widget_show(frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox_frame);
	gtk_widget_show(hbox_frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(hbox_frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("X Major: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin_hmajor=gtk_spin_button_new_with_range(0, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), spin_hmajor, TRUE, FALSE, 2);
	gtk_widget_show(spin_hmajor);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(hbox_frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Y Major: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin_vmajor=gtk_spin_button_new_with_range(0, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), spin_vmajor, TRUE, FALSE, 2);
	gtk_widget_show(spin_vmajor);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(hbox_frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("X Minor: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin_hminor=gtk_spin_button_new_with_range(0, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), spin_hminor, TRUE, FALSE, 2);
	gtk_widget_show(spin_hminor);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(hbox_frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Y Minor: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin_vminor=gtk_spin_button_new_with_range(0, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), spin_vminor, TRUE, FALSE, 2);
	gtk_widget_show(spin_vminor);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(hbox_frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Length: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin_length=gtk_spin_button_new_with_range(3, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), spin_length, TRUE, FALSE, 2);
	gtk_widget_show(spin_length);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_hmajor), GABEDIT_XYPLOT(xyplot)->hmajor_ticks);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_hminor), GABEDIT_XYPLOT(xyplot)->hminor_ticks);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_vmajor), GABEDIT_XYPLOT(xyplot)->vmajor_ticks);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_vminor), GABEDIT_XYPLOT(xyplot)->vminor_ticks);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_length), GABEDIT_XYPLOT(xyplot)->length_ticks);

	g_signal_connect(G_OBJECT(spin_hmajor), "value-changed", G_CALLBACK(spin_hmajor_changed_value), xyplot);
	g_signal_connect(G_OBJECT(spin_hminor), "value-changed", G_CALLBACK(spin_hminor_changed_value), xyplot);
	g_signal_connect(G_OBJECT(spin_vmajor), "value-changed", G_CALLBACK(spin_vmajor_changed_value), xyplot);
	g_signal_connect(G_OBJECT(spin_vminor), "value-changed", G_CALLBACK(spin_vminor_changed_value), xyplot);
	g_signal_connect(G_OBJECT(spin_length), "value-changed", G_CALLBACK(spin_length_ticks_changed_value), xyplot);
}
/****************************************************************************************/
static void set_ticks_dialog(GtkWidget* xyplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* hbox = NULL;
	GtkWidget* parentWindow = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set ticks"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)xyplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);

	add_ticks_frame(hbox, xyplot);


	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
}
/****************************************************************************************/
static void spin_right_margins_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_set_margins_right(GABEDIT_XYPLOT(xyplot), gtk_spin_button_get_value(spinbutton));	
	}
}
/****************************************************************************************/
static void spin_left_margins_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_set_margins_left(GABEDIT_XYPLOT(xyplot), gtk_spin_button_get_value(spinbutton));
	}
}
/****************************************************************************************/
static void spin_bottom_margins_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_set_margins_bottom(GABEDIT_XYPLOT(xyplot), gtk_spin_button_get_value(spinbutton));
	}
}
/****************************************************************************************/
static void spin_top_margins_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_set_margins_top(GABEDIT_XYPLOT(xyplot), gtk_spin_button_get_value(spinbutton));	
	}
}
/****************************************************************************************/
static void add_margins_frame(GtkWidget* hbox, GtkWidget* xyplot)
{
	GtkWidget* frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* label = NULL;
	GtkWidget* spin_left_margins = NULL;
	GtkWidget* spin_right_margins = NULL;
	GtkWidget* spin_top_margins = NULL;
	GtkWidget* spin_bottom_margins = NULL;

	frame=gtk_frame_new(_("Margins"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 2);
	gtk_widget_show(frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox_frame);
	gtk_widget_show(hbox_frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(hbox_frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Left: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin_left_margins=gtk_spin_button_new_with_range(0, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), spin_left_margins, TRUE, FALSE, 2);
	gtk_widget_show(spin_left_margins);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(hbox_frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Top: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin_top_margins=gtk_spin_button_new_with_range(0, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), spin_top_margins, TRUE, FALSE, 2);
	gtk_widget_show(spin_top_margins);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(hbox_frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Right: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin_right_margins=gtk_spin_button_new_with_range(0, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), spin_right_margins, TRUE, FALSE, 2);
	gtk_widget_show(spin_right_margins);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(hbox_frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Bottom: "));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin_bottom_margins=gtk_spin_button_new_with_range(0, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), spin_bottom_margins, TRUE, FALSE, 2);
	gtk_widget_show(spin_bottom_margins);

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_left_margins), GABEDIT_XYPLOT(xyplot)->left_margins);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_right_margins), GABEDIT_XYPLOT(xyplot)->right_margins);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_top_margins), GABEDIT_XYPLOT(xyplot)->top_margins);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_bottom_margins), GABEDIT_XYPLOT(xyplot)->bottom_margins);

	g_signal_connect(G_OBJECT(spin_left_margins), "value-changed", G_CALLBACK(spin_left_margins_changed_value), xyplot);
	g_signal_connect(G_OBJECT(spin_right_margins), "value-changed", G_CALLBACK(spin_right_margins_changed_value), xyplot);
	g_signal_connect(G_OBJECT(spin_top_margins), "value-changed", G_CALLBACK(spin_top_margins_changed_value), xyplot);
	g_signal_connect(G_OBJECT(spin_bottom_margins), "value-changed", G_CALLBACK(spin_bottom_margins_changed_value), xyplot);
}
/****************************************************************************************/
static void set_margins_dialog(GtkWidget* xyplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* hbox = NULL;
	GtkWidget* parentWindow = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set margins"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)xyplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);

	add_margins_frame(hbox, xyplot);


	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
}
/****************************************************************************************/
static void reflect_x_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_reflect_x (GABEDIT_XYPLOT(xyplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void reflect_y_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_reflect_y (GABEDIT_XYPLOT(xyplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void add_reflexion_frame(GtkWidget* hbox, GtkWidget* xyplot)
{
	GtkWidget* frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* reflect_x_button;
	GtkWidget* reflect_y_button;

	frame=gtk_frame_new(_("Axes"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 2);
	gtk_widget_show(frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox_frame);
	gtk_widget_show(hbox_frame);

	reflect_x_button = gtk_check_button_new_with_label (_("Reflect X"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), reflect_x_button, FALSE, FALSE, 2);
	gtk_widget_show(reflect_x_button); 

	reflect_y_button = gtk_check_button_new_with_label (_("Reflect Y"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), reflect_y_button, FALSE, FALSE, 2);
	gtk_widget_show(reflect_y_button); 

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(reflect_x_button), GABEDIT_XYPLOT(xyplot)->reflect_x);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(reflect_y_button), GABEDIT_XYPLOT(xyplot)->reflect_y);
	
	g_signal_connect (G_OBJECT (reflect_x_button), "toggled", (GCallback)reflect_x_toggled, xyplot);
	g_signal_connect (G_OBJECT (reflect_y_button), "toggled", (GCallback)reflect_y_toggled, xyplot);
}
/****************************************************************************************/
static void show_left_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_show_left_legends (GABEDIT_XYPLOT(xyplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void show_right_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_show_right_legends (GABEDIT_XYPLOT(xyplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void show_top_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_show_top_legends (GABEDIT_XYPLOT(xyplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void show_bottom_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_show_bottom_legends (GABEDIT_XYPLOT(xyplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void show_rectangle_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_show_rectangle_legends (GABEDIT_XYPLOT(xyplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void add_legends_frame(GtkWidget* hbox, GtkWidget* xyplot)
{
	GtkWidget* frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* show_left;
	GtkWidget* show_right;
	GtkWidget* show_top;
	GtkWidget* show_bottom;
	GtkWidget* show_rectangle;

	frame=gtk_frame_new(_("Legends"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 2);
	gtk_widget_show(frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox_frame);
	gtk_widget_show(hbox_frame);

	show_left = gtk_check_button_new_with_label (_("Left"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), show_left, FALSE, FALSE, 2);
	gtk_widget_show(show_left); 

	show_right = gtk_check_button_new_with_label (_("Right"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), show_right, FALSE, FALSE, 2);
	gtk_widget_show(show_right); 

	show_top = gtk_check_button_new_with_label (_("Top"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), show_top, FALSE, FALSE, 2);
	gtk_widget_show(show_top); 

	show_bottom = gtk_check_button_new_with_label (_("Bottom"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), show_bottom, FALSE, FALSE, 2);
	gtk_widget_show(show_bottom); 

	show_rectangle = gtk_check_button_new_with_label (_("Rectangle"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), show_rectangle, FALSE, FALSE, 2);
	gtk_widget_show(show_rectangle); 

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_left), GABEDIT_XYPLOT(xyplot)->show_left_legends);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_right), GABEDIT_XYPLOT(xyplot)->show_right_legends);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_top), GABEDIT_XYPLOT(xyplot)->show_top_legends);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_bottom), GABEDIT_XYPLOT(xyplot)->show_bottom_legends);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_rectangle), GABEDIT_XYPLOT(xyplot)->show_rectangle_legends);
	
	g_signal_connect (G_OBJECT (show_left), "toggled", (GCallback)show_left_toggled, xyplot);
	g_signal_connect (G_OBJECT (show_right), "toggled", (GCallback)show_right_toggled, xyplot);
	g_signal_connect (G_OBJECT (show_top), "toggled", (GCallback)show_top_toggled, xyplot);
	g_signal_connect (G_OBJECT (show_bottom), "toggled", (GCallback)show_bottom_toggled, xyplot);
	g_signal_connect (G_OBJECT (show_rectangle), "toggled", (GCallback)show_rectangle_toggled, xyplot);
}
/****************************************************************************************/
static void h_major_grids_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_HMAJOR_GRID, gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void h_minor_grids_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_HMINOR_GRID, gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void v_major_grids_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_VMAJOR_GRID, gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void v_minor_grids_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* xyplot = GTK_WIDGET(user_data);
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_VMINOR_GRID, gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void add_grid_frame(GtkWidget* hbox, GtkWidget* xyplot)
{
	GtkWidget* frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* h_major;
	GtkWidget* h_minor;
	GtkWidget* v_major;
	GtkWidget* v_minor;

	frame=gtk_frame_new(_("Grid"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 2);
	gtk_widget_show(frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox_frame);
	gtk_widget_show(hbox_frame);

	h_major = gtk_check_button_new_with_label (_("H major"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), h_major, FALSE, FALSE, 2);
	gtk_widget_show(h_major); 

	v_major = gtk_check_button_new_with_label (_("V major"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), v_major, FALSE, FALSE, 2);
	gtk_widget_show(v_major); 

	h_minor = gtk_check_button_new_with_label (_("H minor"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), h_minor, FALSE, FALSE, 2);
	gtk_widget_show(h_minor); 

	v_minor = gtk_check_button_new_with_label (_("V minor"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), v_minor, FALSE, FALSE, 2);
	gtk_widget_show(v_minor); 

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(h_major), GABEDIT_XYPLOT(xyplot)->hmajor_grid);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(h_minor), GABEDIT_XYPLOT(xyplot)->hminor_grid);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(v_major), GABEDIT_XYPLOT(xyplot)->vmajor_grid);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(v_minor), GABEDIT_XYPLOT(xyplot)->vminor_grid);
	
	g_signal_connect (G_OBJECT (h_major), "toggled", (GCallback)h_major_grids_toggled, xyplot);
	g_signal_connect (G_OBJECT (h_minor), "toggled", (GCallback)h_minor_grids_toggled, xyplot);
	g_signal_connect (G_OBJECT (v_major), "toggled", (GCallback)v_major_grids_toggled, xyplot);
	g_signal_connect (G_OBJECT (v_minor), "toggled", (GCallback)v_minor_grids_toggled, xyplot);
}
/********************************************************************************/
static void activate_entry_xmincut(GtkWidget *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gint loop;
		gdouble a;
		G_CONST_RETURN gchar* t;
  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		a = atof(t);
		if(data)
		{
			int n = 0;
			for(loop=0;loop<data->size; loop++) if(data->x[loop]>=a) n++;
			if(n!=data->size && n>0)
			{
				gdouble* X = g_malloc(n*sizeof(gdouble));
				gdouble* Y = g_malloc(n*sizeof(gdouble));
				n= 0;
				for(loop=0;loop<data->size; loop++) 
				if(data->x[loop]>=a)
				{ 
					X[n] = data->x[loop]; 
					Y[n] = data->y[loop]; 
					n++;
				}
				if(data->x) g_free(data->x);
				if(data->y) g_free(data->y);
				data->x = X;
				data->y = Y;
				data->size  = n;
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/********************************************************************************/
static void activate_entry_xmaxcut(GtkWidget *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gint loop;
		gdouble a;
		G_CONST_RETURN gchar* t;
  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		a = atof(t);
		if(data)
		{
			int n = 0;
			for(loop=0;loop<data->size; loop++) if(data->x[loop]<=a) n++;
			if(n!=data->size && n>0)
			{
				gdouble* X = g_malloc(n*sizeof(gdouble));
				gdouble* Y = g_malloc(n*sizeof(gdouble));
				n= 0;
				for(loop=0;loop<data->size; loop++) 
				if(data->x[loop]<=a)
				{ 
					X[n] = data->x[loop]; 
					Y[n] = data->y[loop]; 
					n++;
				}
				if(data->x) g_free(data->x);
				if(data->y) g_free(data->y);
				data->x = X;
				data->y = Y;
				data->size  = n;
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/********************************************************************************/
static void add_cut_data_frame(GtkWidget* hbox, GtkWidget* xyplot, XYPlotData *data)
{
	GtkWidget* frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* label = NULL;
	GtkWidget* entry_x_min = NULL;
	GtkWidget* entry_x_max = NULL;
	gchar tmp[100];

	frame=gtk_frame_new(_("Cut data"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 2);
	gtk_widget_show(frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox_frame);
	gtk_widget_show(hbox_frame);

	label=gtk_label_new(_("X Min: "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_x_min = gtk_entry_new();
	gtk_widget_set_size_request(entry_x_min,80,-1);
	sprintf(tmp,"%0.3f",GABEDIT_XYPLOT(xyplot)->xmin);
	gtk_entry_set_text(GTK_ENTRY(entry_x_min),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_x_min, TRUE, FALSE, 2);
	gtk_widget_show(entry_x_min);

	label=gtk_label_new(_(" X Max: "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	
	entry_x_max = gtk_entry_new();
	gtk_widget_set_size_request(entry_x_max,80,-1);
	sprintf(tmp,"%0.3f",GABEDIT_XYPLOT(xyplot)->xmax);
	gtk_entry_set_text(GTK_ENTRY(entry_x_max),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_x_max, TRUE, FALSE, 2);
	gtk_widget_show(entry_x_max);

	g_object_set_data(G_OBJECT (entry_x_min), "CurentData", data);
	g_object_set_data(G_OBJECT (entry_x_max), "CurentData", data);
	g_signal_connect (G_OBJECT (entry_x_min), "activate", (GCallback)activate_entry_xmincut, xyplot);
	g_signal_connect (G_OBJECT (entry_x_max), "activate", (GCallback)activate_entry_xmaxcut, xyplot);
}
/********************************************************************************/
static void activate_entry_x_linear(GtkWidget *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gint N;
		G_CONST_RETURN gchar* t;
  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		N = atoi(t);
		if(data && N>1)
		{
			gdouble* x  = NULL;
			gdouble* y  = NULL;
			build_linear_x(data->x, data->y, data->size, &x, &y, N);
			g_free(data->x);
			g_free(data->y);
			data->x = x;
			data->y = y;
			data->size = N;
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/********************************************************************************/
static void add_x_linear_data_frame(GtkWidget* hbox, GtkWidget* xyplot, XYPlotData *data)
{
	GtkWidget* frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* label = NULL;
	GtkWidget* entry_x_linear = NULL;
	gchar tmp[100];

	frame=gtk_frame_new(_("X Linearization"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, FALSE, 2);
	gtk_widget_show(frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox_frame);
	gtk_widget_show(hbox_frame);

	label=gtk_label_new(_("N points : "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_x_linear = gtk_entry_new();
	gtk_widget_set_size_request(entry_x_linear,80,-1);

	if(data) sprintf(tmp,"%d", 2*data->size);
	else sprintf(tmp,"%d",100);

	gtk_entry_set_text(GTK_ENTRY(entry_x_linear),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_x_linear, TRUE, FALSE, 2);
	gtk_widget_show(entry_x_linear);

	g_object_set_data(G_OBJECT (entry_x_linear), "CurentData", data);
	g_signal_connect (G_OBJECT (entry_x_linear), "activate", (GCallback)activate_entry_x_linear, xyplot);
}
/********************************************************************************************************/
static GtkWidget *add_combo_correction(GtkWidget *hbox)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Fourier : no correction", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Harmonic (w*beta hbar w)", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Classic : w ( 1 - exp(-beta hbar w) )", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Kubo : w tanh(beta hbar w/2) ", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Schofield : w ( 1 - exp(-beta hbar w) ) exp(beta hbar w/2)", -1);

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
static void add_fourier_data_frame(GtkWidget* hbox, GtkWidget* xyplot, XYPlotData *data, GtkWidget* parentWindow)
{
	GtkWidget* frame = NULL;
	GtkWidget* vbox_frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* label = NULL;
	GtkWidget* entry_scale = NULL;
	GtkWidget* entry_xmax = NULL;
	GtkWidget* entry_temperature = NULL;
	GtkWidget* buttonDFT = NULL;
	GtkWidget* combo = NULL;
	GtkWidget* normalize_button = NULL;
	gchar tmp[100];

	frame=gtk_frame_new(_("Fourier transformation"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, TRUE, 2);
	gtk_widget_show(frame);

	vbox_frame=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox_frame);
	gtk_widget_show(vbox_frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_frame), hbox_frame, FALSE, FALSE, 2);
	gtk_widget_show(hbox_frame);

	label=gtk_label_new(_("Scale X : "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_scale = gtk_entry_new();
	gtk_widget_set_size_request(entry_scale,100,-1);
	sprintf(tmp,"%0.3f",33356.40951982);
	gtk_entry_set_text(GTK_ENTRY(entry_scale),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_scale, FALSE, FALSE, 2);
	gtk_widget_show(entry_scale);
	g_object_set_data(G_OBJECT ( xyplot), "Entry_scale", entry_scale);

	label=gtk_label_new(_("X max : "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_xmax = gtk_entry_new();
	gtk_widget_set_size_request(entry_xmax,100,-1);
	sprintf(tmp,"%0.3f",6000.0);
	gtk_entry_set_text(GTK_ENTRY(entry_xmax),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_xmax, FALSE, FALSE, 2);
	gtk_widget_show(entry_xmax);
	g_object_set_data(G_OBJECT ( xyplot), "Entry_xmax", entry_xmax);


	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_frame), hbox_frame, FALSE, FALSE, 2);
	gtk_widget_show(hbox_frame);

	label=gtk_label_new(_("Correction :"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	combo = add_combo_correction(hbox_frame);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
	gtk_widget_show(combo); 
	g_object_set_data(G_OBJECT (xyplot), "ComboCorrection", combo);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_frame), hbox_frame, FALSE, FALSE, 2);
	gtk_widget_show(hbox_frame);

	label=gtk_label_new(_(" Temperature (K) : "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	
	entry_temperature = gtk_entry_new();
	gtk_widget_set_size_request(entry_temperature,100,-1);
	sprintf(tmp,"%0.3f",300.0);
	gtk_entry_set_text(GTK_ENTRY(entry_temperature),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_temperature, FALSE, FALSE, 2);
	gtk_widget_show(entry_temperature);
	g_object_set_data(G_OBJECT ( xyplot), "Entry_temperature", entry_temperature);

	normalize_button = gtk_check_button_new_with_label (_("Normalize"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), normalize_button, FALSE, FALSE, 2);
	gtk_widget_show(normalize_button); 
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(normalize_button), TRUE);
	g_object_set_data(G_OBJECT ( xyplot), "NormalizeDFTButton", normalize_button);


	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_frame), hbox_frame, FALSE, FALSE, 2);
	gtk_widget_show(hbox_frame);
	buttonDFT = gtk_button_new_with_label (_("DFT"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), buttonDFT, TRUE, TRUE, 4);
	gtk_widget_show (buttonDFT);

	g_object_set_data(G_OBJECT ( xyplot), "CurentData", data);
	g_object_set_data(G_OBJECT (buttonDFT), "ParentWindow", parentWindow);

        g_signal_connect(G_OBJECT(buttonDFT), "clicked", G_CALLBACK(build_fourier), xyplot);

}
/****************************************************************************************/
static void set_all_dialog(GtkWidget* xyplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* parentWindow = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* vbox = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)xyplot);
	
	vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show(vbox);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 1);
	add_ranges_frame(hbox, xyplot);
	gtk_widget_show(hbox); 

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 1);
	add_margins_frame(hbox, xyplot);
	gtk_widget_show(hbox); 

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 1);
	add_labels_frame(hbox, xyplot);
	add_font_size_frame(hbox, xyplot);
	add_digits_frame(hbox, xyplot);
	gtk_widget_show(hbox); 

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 1);
	add_ticks_frame(hbox, xyplot);
	gtk_widget_show(hbox); 


	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 1);
	add_reflexion_frame(hbox, xyplot);
	add_legends_frame(hbox, xyplot);
	add_grid_frame(hbox, xyplot);
	gtk_widget_show(hbox); 


	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
}
/****************************************************************************************/
static void gabedit_xyplot_save_gabedit_lines(GabeditXYPlot *xyplot, FILE* file)
{
	gint i;
	fprintf(file,"lines %d\n", xyplot->nObjectsLine);
	for(i=0;i<xyplot->nObjectsLine;i++)
	{
		fprintf(file,"%lf %lf %lf %lf %d %d %lf %lf %lf %d\n", 
				xyplot->objectsLine[i].x1, 
				xyplot->objectsLine[i].y1, 
				xyplot->objectsLine[i].x2, 
				xyplot->objectsLine[i].y2, 
				xyplot->objectsLine[i].width, 
				xyplot->objectsLine[i].arrow_size, 
				SCALE(xyplot->objectsLine[i].color.red),
				SCALE(xyplot->objectsLine[i].color.green),
				SCALE(xyplot->objectsLine[i].color.blue),
				xyplot->objectsLine[i].style
		       ); 
	}
}
/****************************************************************************************/
static void gabedit_xyplot_save_gabedit_texts(GabeditXYPlot *xyplot, FILE* file)
{
	gint i;
	fprintf(file,"texts %d\n", xyplot->nObjectsText);
	for(i=0;i<xyplot->nObjectsText;i++)
	{
		fprintf(file,"%lf %lf %lf\n", 
				xyplot->objectsText[i].x, 
				xyplot->objectsText[i].y,
				xyplot->objectsText[i].angle
		       ); 
		fprintf(file,"%s\n", xyplot->objectsText[i].str);
	}
}
/*************************************************************************************/
static gchar *get_suffix_name_file(const gchar* allname)
{
	gchar *filename= g_path_get_basename(allname);
	gchar *dirname= g_path_get_dirname(allname);
	gchar *temp= g_strdup(filename);
	gint len=strlen(filename);
	gint i;
	gchar* name = NULL;

	if(!allname || strlen(allname)<1) return g_strdup("error");
	filename= g_path_get_basename(allname);
	dirname= g_path_get_dirname(allname);
	temp= g_strdup(filename);
	len=strlen(filename);

	for(i=len;i>0;i--)
	if(temp[i]=='.')
	{
		temp[i] = '\0';
		break;
	}
	name = g_strdup_printf("%s%s%s",dirname,G_DIR_SEPARATOR_S,temp);
	if(temp) g_free(temp);
	if(dirname) g_free(dirname);
	if(filename) g_free(filename);

	if(strcmp(name,".")==0) name = g_strdup(g_get_current_dir());
   
	return name;
}
/*************************************************************************************/
static gchar *get_filename_png(const gchar* allname, gint i)
{
	gchar *bas = get_suffix_name_file(allname);
	if(bas)
	{
		gchar* fn = g_strdup_printf("%s_%d.png",bas,i);
		g_free(bas);
		return fn;
	}
	return g_strdup_printf("%d.png",i);
}
/****************************************************************************************/
static void gabedit_xyplot_save_gabedit_images(GabeditXYPlot *xyplot, FILE* file, gchar* fileName)
{
	gint i;
	gchar *fn;
	fprintf(file,"images %d\n", xyplot->nObjectsImage);
	for(i=0;i<xyplot->nObjectsImage;i++)
	{
		fprintf(file,"%lf %lf %lf %lf\n", 
				xyplot->objectsImage[i].x, 
				xyplot->objectsImage[i].y,
				xyplot->objectsImage[i].width,
				xyplot->objectsImage[i].height
		       ); 
		fn = get_filename_png(fileName, i);
		fprintf(file,"%s\n", fn);
		cairo_surface_write_to_png (xyplot->objectsImage[i].image,fn);
		g_free(fn);
	}
}
/****************************************************************************************/
static void gabedit_xyplot_save_gabedit_parameters(GabeditXYPlot *xyplot, FILE* file)
{
	GdkColor foreColor;
	GdkColor backColor;
	GdkGCValues gc_values;
	GdkColormap *colormap;

	colormap  = gdk_window_get_colormap(GTK_WIDGET(xyplot)->window);
	gdk_gc_get_values(xyplot->back_gc, &gc_values);
       	gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&backColor);
	gdk_gc_get_values(xyplot->fore_gc, &gc_values);
       	gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&foreColor);

	fprintf(file,"theme %d %d %d %d %d %d %d\n", gc_values.line_width,
			(gint)foreColor.red, (gint)foreColor.green,(gint)foreColor.blue,
			(gint)backColor.red, (gint)backColor.green,(gint)backColor.blue);
	fprintf(file,"digits %d %d\n", xyplot->x_legends_digits,  xyplot->y_legends_digits);
	fprintf(file,"ticks %d %d %d %d %d\n",  xyplot->length_ticks,  xyplot->hmajor_ticks, xyplot->hminor_ticks,  xyplot->vmajor_ticks, xyplot->vminor_ticks);
	fprintf(file,"margins %d %d %d %d\n",  xyplot->left_margins,  xyplot->top_margins, xyplot->right_margins,  xyplot->bottom_margins);
	fprintf(file,"grid %d %d %d %d\n", xyplot->hmajor_grid, xyplot->hminor_grid, xyplot->vmajor_grid, xyplot->vminor_grid); 
	fprintf(file,"legends %d %d %d %d %d\n", xyplot->show_left_legends, xyplot->show_right_legends, xyplot->show_top_legends, xyplot->show_bottom_legends,xyplot->show_rectangle_legends);
	fprintf(file,"axes %d %d\n",  xyplot->reflect_x, xyplot->reflect_y);
	fprintf(file,"fonts %d\n", xyplot->font_size);
	fprintf(file,"minmax %lf %lf %lf %lf\n",  xyplot->xmin, xyplot->xmax, xyplot->ymin, xyplot->ymax);
	if(xyplot->h_label_str) 
	{
		fprintf(file,"hlabel 1\n");
		fprintf(file,"%s\n",  xyplot->h_label_str);
	}
	else fprintf(file,"hlabel 0\n");
	if(xyplot->v_label_str) 
	{
		fprintf(file,"vlabel 1\n");
		fprintf(file,"%s\n",  xyplot->v_label_str);
	}
	else fprintf(file,"vlabel 0\n");

}
/****************************************************************************************/
static void gabedit_xyplot_save_gabedit_data(XYPlotData* data, FILE* file)
{
	gint i;
	fprintf(file,"dataline %lf %lf %lf %d %d\n",
		SCALE(data->line_color.red),
		SCALE(data->line_color.green),
		SCALE(data->line_color.blue),
		data->line_width,
		data->line_style);
	fprintf(file,"datapoint %lf %lf %lf %d %s\n",
		SCALE(data->point_color.red),
		SCALE(data->point_color.green),
		SCALE(data->point_color.blue),
		data->point_size,
		data->point_str);
	fprintf(file,"size %d\n", data->size);
	for(i=0;i<data->size;i++)
		fprintf(file,"%lf %lf\n", data->x[i],data->y[i]);
}
/****************************************************************************************/
static void gabedit_xyplot_save_gabedit_datas(GabeditXYPlot *xyplot, FILE* file)
{
	gint nDatas = 0;
	XYPlotData* data;
	GList *current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
	for (; current_node!=NULL; current_node=current_node->next) nDatas++;
	fprintf(file,"ndatas %d\n",nDatas);
	if(nDatas<1) return;
	current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
	for (; current_node!=NULL; current_node=current_node->next)
	{
		data=(XYPlotData*)current_node->data;  
		gabedit_xyplot_save_gabedit_data(data, file);
	}
}
/****************************************************************************************/
static void gabedit_xyplot_save_gabedit(GtkWidget* xyplot, gchar* fileName)
{
 	FILE* file = fopen(fileName, "w");
	if(!file) 
	{
		GtkWidget* dialog = NULL;
		gchar* tmp = g_strdup_printf(_("Sorry, I cannot open %s file"),fileName);
		dialog = gtk_message_dialog_new_with_markup (NULL,
		           GTK_DIALOG_DESTROY_WITH_PARENT,
		           GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			   "%s",tmp);
       		gtk_dialog_run (GTK_DIALOG (dialog));
       		gtk_widget_destroy (dialog);
       		g_free(tmp);
		return;
	}
	fprintf(file,"[Gabedit format]\n");
	fprintf(file,"[XYPLOT]\n");
	gabedit_xyplot_save_gabedit_parameters(GABEDIT_XYPLOT(xyplot), file);
	gabedit_xyplot_save_gabedit_texts(GABEDIT_XYPLOT(xyplot), file);
	gabedit_xyplot_save_gabedit_lines(GABEDIT_XYPLOT(xyplot), file);
	gabedit_xyplot_save_gabedit_images(GABEDIT_XYPLOT(xyplot), file,fileName);
	gabedit_xyplot_save_gabedit_datas(GABEDIT_XYPLOT(xyplot), file);
	fclose(file);
}
/**************************************************************************/
static void saveAsGabedit(GtkFileChooser *SelecFile, gint response_id)
{       
	gchar *fileName;
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (SelecFile), "XYPLOT");

 	if(response_id != GTK_RESPONSE_OK) return;
	if(!GABEDIT_IS_XYPLOT(xyplot)) return;
 	fileName = gtk_file_chooser_get_filename(SelecFile);
	gabedit_xyplot_save_gabedit(xyplot, fileName);
}
/********************************************************************************/
static void saveAsGabeditDlg(GtkWidget* xyplot)
{
	GtkFileFilter *filter;
	GtkWidget *dialog;
	GtkWidget *parentWindow;

	dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", GTK_FILE_CHOOSER_ACTION_SAVE, "file-system-backend", "gtk+", "select-multiple", FALSE, NULL);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Save as Gabedit format"));
	gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
	filter = gtk_file_filter_new ();
	
	gtk_file_filter_set_name (filter, "*.gab");
	gtk_file_filter_add_pattern (filter, "*.gab");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	g_signal_connect (dialog, "response",  G_CALLBACK (saveAsGabedit),GTK_OBJECT(dialog));
	g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy),GTK_OBJECT(dialog));
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog),"xyplot.gab");

	g_object_set_data (G_OBJECT (dialog), "XYPLOT",xyplot);
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow) gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parentWindow));
	gtk_widget_show(dialog);
}
/****************************************************************************************/
static gboolean gabedit_xyplot_read_gabedit_lines(GabeditXYPlot *xyplot, FILE* file)
{
	gint i;
	gchar t[BSIZE];
	gchar tmp[BSIZE];
	gint style;
	if(xyplot->objectsLine)
	{
		g_free(xyplot->objectsLine);
		xyplot->objectsLine = NULL;
	}
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d\n", tmp, &xyplot->nObjectsLine);
	if(xyplot->nObjectsLine>0)
		xyplot->objectsLine = g_malloc(xyplot->nObjectsLine*sizeof(XYPlotObjectLine));

	for(i=0;i<xyplot->nObjectsLine;i++)
	{
		gdouble r,g,b;
    		if(!fgets(t,BSIZE,file))
		{
			xyplot->nObjectsLine= 0;
			g_free(xyplot->objectsLine);
			xyplot->objectsLine = NULL;
			xyplot->nObjectsLine = 0;
		       	return FALSE;
		}
		sscanf(t,"%lf %lf %lf %lf %d %d %lf %lf %lf %d\n", 
				&xyplot->objectsLine[i].x1, 
				&xyplot->objectsLine[i].y1, 
				&xyplot->objectsLine[i].x2, 
				&xyplot->objectsLine[i].y2, 
				&xyplot->objectsLine[i].width, 
				&xyplot->objectsLine[i].arrow_size, 
				&r,&g,&b,
				&style
		       ); 
		xyplot->objectsLine[i].style = style;
		xyplot->objectsLine[i].color.red=SCALE2(r);
		xyplot->objectsLine[i].color.green=SCALE2(g);
		xyplot->objectsLine[i].color.blue=SCALE2(b);
	}
	reset_object_line_pixels(xyplot);
	return TRUE;
}
/****************************************************************************************/
static gboolean gabedit_xyplot_read_gabedit_texts(GabeditXYPlot *xyplot, FILE* file)
{
	gint i;
	gchar t[BSIZE];
	gchar tmp[BSIZE];
	if(xyplot->objectsText)
	{
		for(i=0;i<xyplot->nObjectsText;i++)
		{
			if(xyplot->objectsText[i].str) g_free(xyplot->objectsText[i].str);
			if(xyplot->objectsText[i].pango) g_object_unref(xyplot->objectsText[i].pango);
		}
		g_free(xyplot->objectsText);
		xyplot->objectsText = NULL;
		xyplot->nObjectsText = 0;
	}
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d\n", tmp, &xyplot->nObjectsText);
	if(xyplot->nObjectsText>0)
	{
		xyplot->objectsText = g_malloc(xyplot->nObjectsText*sizeof(XYPlotObjectText));
		for(i=0;i<xyplot->nObjectsText;i++) xyplot->objectsText[i].str = NULL;
	}
	for(i=0;i<xyplot->nObjectsText;i++)
	{
    		if(!fgets(t,BSIZE,file))
		{
			for(i=0;i<xyplot->nObjectsText;i++) 
				if(xyplot->objectsText[i].str) g_free(xyplot->objectsText[i].str);
			xyplot->nObjectsText= 0;
			g_free(xyplot->objectsText);
			xyplot->objectsText = NULL;
		       	return FALSE;
		}
		sscanf(t,"%lf %lf %lf", &xyplot->objectsText[i].x, &xyplot->objectsText[i].y,&xyplot->objectsText[i].angle); 
    		if(fgets(t,BSIZE,file))
		{
			gint j;
			for(j=0;j<strlen(t);j++) if(t[j]=='\n') t[j] = ' ';
			for(j=0;j<strlen(t);j++) if(t[j]=='\r') t[j] = ' ';
			xyplot->objectsText[i].str= g_strdup(t);
			for(j=strlen(t);j>=0;j--)   if(t[j]==' ') t[j] = '\0';
		}
		else xyplot->objectsText[i].str = g_strdup(" ");
	}
	for(i=0;i<xyplot->nObjectsText;i++)
	{
		set_object_text_pango(xyplot, &xyplot->objectsText[i]);
		set_object_text_pixels(xyplot, &xyplot->objectsText[i]);
	}
	return TRUE;
}
/****************************************************************************************/
static gboolean gabedit_xyplot_read_gabedit_images(GabeditXYPlot *xyplot, FILE* file)
{
	gint i;
	gchar t[BSIZE];
	gchar tmp[BSIZE];
	if(xyplot->objectsImage)
	{
		for(i=0;i<xyplot->nObjectsImage;i++)
		{
			if(xyplot->objectsImage[i].fileName) g_free(xyplot->objectsImage[i].fileName);
			if(xyplot->objectsImage[i].image) cairo_surface_destroy (xyplot->objectsImage[i].image);
		}
		g_free(xyplot->objectsImage);
		xyplot->objectsImage = NULL;
		xyplot->nObjectsImage = 0;
	}
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d\n", tmp, &xyplot->nObjectsImage);
	if(xyplot->nObjectsImage>0)
	{
		xyplot->objectsImage = g_malloc(xyplot->nObjectsImage*sizeof(XYPlotObjectImage));
		for(i=0;i<xyplot->nObjectsImage;i++) xyplot->objectsImage[i].fileName = NULL;
		for(i=0;i<xyplot->nObjectsImage;i++) xyplot->objectsImage[i].image = NULL;
	}
	for(i=0;i<xyplot->nObjectsImage;i++)
	{
		gdouble x,y,w,h;
    		if(!fgets(t,BSIZE,file))
		{
			for(i=0;i<xyplot->nObjectsImage;i++) 
				if(xyplot->objectsImage[i].fileName) g_free(xyplot->objectsImage[i].fileName);
			xyplot->nObjectsImage= 0;
			g_free(xyplot->objectsImage);
			xyplot->objectsImage = NULL;
		       	return FALSE;
		}
		sscanf(t,"%lf %lf %lf %lf", &x, &y, &w, &h);
		xyplot->objectsImage[i].x = x;
		xyplot->objectsImage[i].y = y;
		xyplot->objectsImage[i].width = w;
		xyplot->objectsImage[i].height = h;
    		if(fgets(t,BSIZE,file))
		{
			gint j;
			gint len;
			len = strlen(t);
			for(j=0;j<strlen(t);j++) 
			{
				if(t[j]=='\n') t[j] = ' ';
				if(t[j]=='\r') t[j] = ' ';
			}
			len = strlen(t);
			for(j=len;j>=0;j--)   if(t[j]==' ') t[j] = '\0';
			xyplot->objectsImage[i].fileName= g_strdup(t);
		}
		else xyplot->objectsImage[i].fileName = g_strdup(" ");
	}
	for(i=0;i<xyplot->nObjectsImage;i++)
	{
		xyplot->objectsImage[i].image = cairo_image_surface_create_from_png (xyplot->objectsImage[i].fileName);
		if(!xyplot->objectsImage[i].image)
		{
			printf("I cannot read %s png file\n",xyplot->objectsImage[i].fileName);
		}
	}
	reset_object_image_pixels(xyplot);
	return TRUE;
}
/****************************************************************************************/
static gboolean gabedit_xyplot_read_gabedit_parameters(GabeditXYPlot *xyplot, FILE* file)
{
	gchar tmp[BSIZE];
	gchar t[BSIZE];
	gint i;
	gint r1, g1, b1;
	gint r2, g2, b2;
	GdkColor foreColor;
	GdkColor backColor;
	gint line_width;

    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d %d %d %d %d %d %d", tmp, &line_width, &r1, &g1, &b1, &r2, &g2, &b2);
	foreColor.red = (gushort)r1;
	foreColor.green = (gushort)g1;
	foreColor.blue = (gushort)b1;
	backColor.red = (gushort)r2;
	backColor.green = (gushort)g2;
	backColor.blue = (gushort)b2;

    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d %d", tmp, &xyplot->x_legends_digits,  &xyplot->y_legends_digits);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d %d %d %d %d",  tmp, &xyplot->length_ticks,  &xyplot->hmajor_ticks, &xyplot->hminor_ticks,  &xyplot->vmajor_ticks, &xyplot->vminor_ticks);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d %d %d %d",  tmp, &xyplot->left_margins,  &xyplot->top_margins, &xyplot->right_margins,  &xyplot->bottom_margins);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d %d %d %d", tmp, &xyplot->hmajor_grid, &xyplot->hminor_grid, &xyplot->vmajor_grid, &xyplot->vminor_grid); 
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_HMAJOR_GRID, xyplot->hmajor_grid);
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_HMINOR_GRID, xyplot->hminor_grid);
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_VMAJOR_GRID, xyplot->vmajor_grid); 
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_VMINOR_GRID, xyplot->vminor_grid);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d %d %d %d %d", tmp, &xyplot->show_left_legends, &xyplot->show_right_legends, &xyplot->show_top_legends, &xyplot->show_bottom_legends,&xyplot->show_rectangle_legends);
	gabedit_xyplot_show_left_legends (GABEDIT_XYPLOT(xyplot), xyplot->show_left_legends);
	gabedit_xyplot_show_right_legends (GABEDIT_XYPLOT(xyplot), xyplot->show_right_legends);
	gabedit_xyplot_show_top_legends (GABEDIT_XYPLOT(xyplot), xyplot->show_top_legends);
	gabedit_xyplot_show_bottom_legends (GABEDIT_XYPLOT(xyplot), xyplot->show_bottom_legends);
	gabedit_xyplot_show_rectangle_legends (GABEDIT_XYPLOT(xyplot), xyplot->show_rectangle_legends);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d %d",  tmp, &xyplot->reflect_x, &xyplot->reflect_y);
	gabedit_xyplot_reflect_x (GABEDIT_XYPLOT(xyplot), xyplot->reflect_x);
	gabedit_xyplot_reflect_y (GABEDIT_XYPLOT(xyplot), xyplot->reflect_y);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d", tmp, &xyplot->font_size);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %lf %lf %lf %lf", tmp, &xyplot->xmin, &xyplot->xmax, &xyplot->ymin, &xyplot->ymax);

	if(xyplot->h_label_str) g_free(xyplot->h_label_str);
	if(xyplot->v_label_str) g_free(xyplot->v_label_str);
	xyplot->h_label_str = NULL;
	xyplot->v_label_str = NULL;
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d",tmp,&i); 
	if(i!=0)
	{
		gint j;
    		if(!fgets(t,BSIZE,file)) return FALSE;
		for(j=0;j<strlen(t);j++) if(t[j]=='\n') t[j] = '\0';
		for(j=0;j<strlen(t);j++) if(t[j]=='\r') t[j] = '\0';
		xyplot->h_label_str = g_strdup(t);
	}
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d",tmp,&i); 
	if(i!=0)
	{
		gint j;
    		if(!fgets(t,BSIZE,file)) return FALSE;
		for(j=0;j<strlen(t);j++) if(t[j]=='\n') t[j] = '\0';
		for(j=0;j<strlen(t);j++) if(t[j]=='\r') t[j] = '\0';
		xyplot->v_label_str = g_strdup(t);
	}
	xyplot_build_legends(xyplot);
	xyplot_calculate_legends_sizes(xyplot);
	xyplot_calculate_sizes(xyplot);
	reset_theme(GTK_WIDGET(xyplot), line_width, &foreColor, &backColor);
	return TRUE;

}
/****************************************************************************************/
static gboolean gabedit_xyplot_read_gabedit_data(XYPlotData* data, FILE* file)
{
	gint i;
	gchar tmp[BSIZE];
	gchar t[BSIZE];
	gdouble r,g,b;
	gint style;
	data->size = 0;
	data->x = NULL;
	data->y = NULL;
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %lf %lf %lf %d %d",
		tmp,
		&r,&g,&b,
		&data->line_width,
		&style);
	data->line_style = style;
	data->line_color.red = SCALE2(r);
	data->line_color.green = SCALE2(g);
	data->line_color.blue = SCALE2(b);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %lf %lf %lf %d %s",
		tmp,
		&r,&g,&b,
		&data->point_size,
		data->point_str);
	data->point_color.red = SCALE2(r);
	data->point_color.green = SCALE2(g);
	data->point_color.blue = SCALE2(b);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d", tmp, &data->size);
	if(data->size<1) return TRUE;
	data->x = g_malloc(data->size*sizeof(gdouble));
	data->y = g_malloc(data->size*sizeof(gdouble));
	for(i=0;i<data->size;i++)
	{
    		if(!fgets(t,BSIZE,file)) 
		{
			data->size = 0;
			if(data->x) g_free(data->x);
			if(data->y) g_free(data->y);
			data->x = NULL;
			data->y = NULL;
			return FALSE;
		}

		sscanf(t,"%lf %lf", &data->x[i],&data->y[i]);
	}
	return TRUE;
}
/****************************************************************************************/
static gboolean gabedit_xyplot_read_gabedit_datas(GabeditXYPlot *xyplot, FILE* file)
{
	gint nDatas = 0;
	gchar tmp[BSIZE];
	gchar t[BSIZE];
	gint i;

    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d",tmp, &nDatas);
	if(nDatas<1) return TRUE;
	if(GABEDIT_XYPLOT(xyplot)->data_list)
	{
		g_list_foreach(GABEDIT_XYPLOT(xyplot)->data_list, (GFunc)g_free, NULL);
		g_list_free(GABEDIT_XYPLOT(xyplot)->data_list);
		GABEDIT_XYPLOT(xyplot)->data_list = NULL;
	}
	for(i=0;i<nDatas;i++)
	{
		XYPlotData *data = g_malloc(sizeof(XYPlotData));
		if(!gabedit_xyplot_read_gabedit_data(data, file)) return FALSE;
		xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), data);
		gabedit_xyplot_add_data (GABEDIT_XYPLOT(xyplot), data);
	}
	return TRUE;
}
/****************************************************************************************/
static gboolean gabedit_xyplot_read_gabedit(GtkWidget* xyplot, gchar* fileName)
{
	gchar t[BSIZE];
 	FILE* file = fopen(fileName, "rb");
	if(!file) 
	{
		GtkWidget* dialog = NULL;
		gchar* tmp = g_strdup_printf(_("Sorry, I cannot open %s file"),fileName);
		dialog = gtk_message_dialog_new_with_markup (NULL,
		           GTK_DIALOG_DESTROY_WITH_PARENT,
		           GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			   "%s",
			   tmp);
       		gtk_dialog_run (GTK_DIALOG (dialog));
       		gtk_widget_destroy (dialog);
       		g_free(tmp);
		return FALSE;
	}
    	if(!fgets(t,BSIZE,file)) return FALSE;
	uppercase(t);
	if(!strstr(t,"[GABEDIT FORMAT]"))
	{
		xyplot_message(_("This is not a Gabedit file\n"));
		fclose(file);
		return FALSE;
	}
	while(!feof(file))
	{
    		if(!fgets(t,BSIZE,file)) 
		{
			fclose(file);
			return FALSE;
		}
		if(strstr(t,"[XYPLOT]"))
		{
			gabedit_xyplot_read_gabedit_parameters(GABEDIT_XYPLOT(xyplot), file);
			gabedit_xyplot_read_gabedit_texts(GABEDIT_XYPLOT(xyplot), file);
			gabedit_xyplot_read_gabedit_lines(GABEDIT_XYPLOT(xyplot), file);
			gabedit_xyplot_read_gabedit_images(GABEDIT_XYPLOT(xyplot), file);
			gabedit_xyplot_read_gabedit_datas(GABEDIT_XYPLOT(xyplot), file);
			fclose(file);
			return TRUE;
		}
	}
	xyplot_message(_("I can not read the XYPlot data\n"));
	fclose(file);
	return FALSE;
}
/**************************************************************************/
static void readAGabedit(GtkFileChooser *SelecFile, gint response_id)
{       
	gchar *fileName;
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (SelecFile), "XYPLOT");

 	if(response_id != GTK_RESPONSE_OK) return;
	if(!GABEDIT_IS_XYPLOT(xyplot)) return;
 	fileName = gtk_file_chooser_get_filename(SelecFile);
	gabedit_xyplot_read_gabedit(xyplot, fileName);
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));
}
/********************************************************************************/
static void readAGabeditDlg(GtkWidget* xyplot)
{
	GtkFileFilter *filter;
	GtkWidget *dialog;
	GtkWidget *parentWindow;

	dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", GTK_FILE_CHOOSER_ACTION_OPEN, "file-system-backend", "gtk+", "select-multiple", FALSE, NULL);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Read a Gabedit file"));
	gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);

	filter = gtk_file_filter_new ();
	
	gtk_file_filter_set_name (filter, "*.gab");
	gtk_file_filter_add_pattern (filter, "*.gab");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	g_signal_connect (dialog, "response",  G_CALLBACK (readAGabedit),GTK_OBJECT(dialog));
	g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy),GTK_OBJECT(dialog));

	g_object_set_data (G_OBJECT (dialog), "XYPLOT",xyplot);
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow) gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parentWindow));
	gtk_widget_show(dialog);
}
/********************************************************************************/
static void Waiting(gdouble tsecond)
{
        GTimer *timer;
        gdouble elaps;
        gulong m ;
	
        timer =g_timer_new( );
	g_timer_start( timer );
	g_timer_reset( timer );
        do{
		elaps = g_timer_elapsed( timer,&m);
        }while(elaps<tsecond);
 	g_timer_destroy(timer);
}
/**************************************************************************/
static void copyImageToClipBoard(GtkWidget* xyplot)
{       
	if(!GABEDIT_IS_XYPLOT(xyplot)) return;
	gtk_widget_hide(xyplot);
	gtk_widget_show(xyplot);
	while( gtk_events_pending() ) gtk_main_iteration();

	gabedit_xyplot_save_image(GABEDIT_XYPLOT(xyplot), NULL, NULL);
}
/**************************************************************************/
static void saveImage(GtkFileChooser *SelecFile, gint response_id)
{       
	gchar *fileName;
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (SelecFile), "XYPLOT");
	gchar* type = g_object_get_data (G_OBJECT (SelecFile), "ImageType");

 	if(response_id != GTK_RESPONSE_OK) return;

 	fileName = gtk_file_chooser_get_filename(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
	Waiting(1.0);

	if(!GABEDIT_IS_XYPLOT(xyplot)) return;
	gtk_widget_hide(xyplot);
	gtk_widget_show(xyplot);
	while( gtk_events_pending() ) gtk_main_iteration();

	gabedit_xyplot_save_image(GABEDIT_XYPLOT(xyplot), fileName, type);
}
/********************************************************************************/
static void saveImageDlg(GtkWidget* xyplot, gchar* type)
{
	GtkFileFilter *filter;
	GtkWidget *dialog;
	gchar* tmp = NULL;
	GtkWidget *parentWindow;

	dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", GTK_FILE_CHOOSER_ACTION_SAVE, "file-system-backend", "gtk+", "select-multiple", FALSE, NULL);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Save XY plot"));
	gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
	filter = gtk_file_filter_new ();
	
	tmp = g_strdup_printf("*.%s",type);
	gtk_file_filter_set_name (filter, tmp);
	gtk_file_filter_add_pattern (filter, tmp);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
	g_free(tmp);


	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	g_signal_connect (dialog, "response",  G_CALLBACK (saveImage),GTK_OBJECT(dialog));
	g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy),GTK_OBJECT(dialog));
	g_object_set_data(G_OBJECT (dialog), "ImageType", type);
	if(!strcmp(type,"tpng"))
		tmp = g_strdup_printf("xyplot.png");
	else
		tmp = g_strdup_printf("xyplot.%s",type);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog),tmp);
	g_free(tmp);

	g_object_set_data (G_OBJECT (dialog), "XYPLOT",xyplot);
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow) gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parentWindow));
	gtk_widget_show(dialog);
}
/*****************************************************************************/
static void force_expose(GtkWidget* widget)
{
	gtk_widget_hide(widget);
	gtk_widget_show(widget);
	while( gtk_events_pending() ) gtk_main_iteration();
}
/*****************************************************************************/
static void gabedit_xyplot_export(GabeditXYPlot *xyplot, gchar* fileName, gchar* fileType)
{
	GtkWidget* widget = NULL;
  	g_return_if_fail (xyplot != NULL);
  	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  	g_return_if_fail (fileName != NULL);
  	g_return_if_fail (fileType != NULL);
	widget = GTK_WIDGET(xyplot);
	if(!strcmp(fileType,"pdf"))
	{
		
		cairo_surface_t *surface;
		surface = cairo_pdf_surface_create(fileName, widget->allocation.width, widget->allocation.height);
		xyplot->cairo_export = cairo_create(surface);
		force_expose(widget);
		cairo_show_page(xyplot->cairo_export);
		cairo_surface_destroy(surface);
		cairo_destroy(xyplot->cairo_export);
		xyplot->cairo_export = NULL;
		return;
	}
	else
	if(!strcmp(fileType,"ps"))
	{
		
		cairo_surface_t *surface;
		surface = cairo_ps_surface_create(fileName, widget->allocation.width, widget->allocation.height);
		xyplot->cairo_export = cairo_create(surface);
		force_expose(widget);
		cairo_show_page(xyplot->cairo_export);
		cairo_surface_destroy(surface);
		cairo_destroy(xyplot->cairo_export);
		xyplot->cairo_export = NULL;
		return;
	}
	else
	if(!strcmp(fileType,"eps"))
	{
		
		cairo_surface_t *surface;
		surface = cairo_ps_surface_create(fileName, widget->allocation.width, widget->allocation.height);
		cairo_ps_surface_set_eps(surface, TRUE);
		xyplot->cairo_export = cairo_create(surface);
		force_expose(widget);
		cairo_show_page(xyplot->cairo_export);
		cairo_surface_destroy(surface);
		cairo_destroy(xyplot->cairo_export);
		xyplot->cairo_export = NULL;
		return;
	}
	else
	if(!strcmp(fileType,"svg"))
	{
		
		cairo_surface_t *surface;
		surface = cairo_svg_surface_create(fileName, widget->allocation.width, widget->allocation.height);
		xyplot->cairo_export = cairo_create(surface);
		force_expose(widget);
		cairo_show_page(xyplot->cairo_export);
		cairo_surface_destroy(surface);
		cairo_destroy(xyplot->cairo_export);
		xyplot->cairo_export = NULL;
		return;
	}
}
/**************************************************************************/
static void exportImage(GtkFileChooser *SelecFile, gint response_id)
{       
	gchar *fileName;
	GtkWidget* xyplot = g_object_get_data (G_OBJECT (SelecFile), "XYPLOT");
	gchar* type = g_object_get_data (G_OBJECT (SelecFile), "ImageType");

 	if(response_id != GTK_RESPONSE_OK) return;

 	fileName = gtk_file_chooser_get_filename(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
	Waiting(1.0);

	if(!GABEDIT_IS_XYPLOT(xyplot)) return;
	gtk_widget_hide(xyplot);
	gtk_widget_show(xyplot);
	while( gtk_events_pending() ) gtk_main_iteration();

	gabedit_xyplot_export(GABEDIT_XYPLOT(xyplot), fileName, type);
}
/********************************************************************************/
static void exportImageDlg(GtkWidget* xyplot, gchar* type)
{
	GtkFileFilter *filter;
	GtkWidget *dialog;
	gchar* tmp = NULL;
	GtkWidget *parentWindow;

	dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", GTK_FILE_CHOOSER_ACTION_SAVE, "file-system-backend", "gtk+", "select-multiple", FALSE, NULL);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Export XY plot"));
	gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
	filter = gtk_file_filter_new ();
	
	tmp = g_strdup_printf("*.%s",type);
	gtk_file_filter_set_name (filter, tmp);
	gtk_file_filter_add_pattern (filter, tmp);
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);
	g_free(tmp);


	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	g_signal_connect (dialog, "response",  G_CALLBACK (exportImage),GTK_OBJECT(dialog));
	g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy),GTK_OBJECT(dialog));
	g_object_set_data(G_OBJECT (dialog), "ImageType", type);
	tmp = g_strdup_printf("gabeditExport.%s",type);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog),tmp);
	g_free(tmp);

	g_object_set_data (G_OBJECT (dialog), "XYPLOT",xyplot);
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow) gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parentWindow));
	gtk_widget_show(dialog);
}
/****************************************************************************************/
static void spin_line_width_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (spinbutton), "CurentData");
  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
		if(data) data->line_width = gtk_spin_button_get_value(spinbutton);
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(XYPlotData*)current_node->data;  
				data->line_width = gtk_spin_button_get_value(spinbutton);
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/****************************************************************************************/
static void spin_line_color_changed_value(GtkColorButton  *colorbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (colorbutton), "CurentData");
		GdkColor c;
		gtk_color_button_get_color (colorbutton, &c);

  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
		
		if(data)
			data->line_color = c; 
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(XYPlotData*)current_node->data;  
				data->line_color = c; 
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/********************************************************************************************************/
static void combo_line_style_changed_value (GtkComboBox *combobox, gpointer user_data)
{
	GtkTreeIter iter;
	gchar* d = NULL;

	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (combobox), "CurentData");

  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

		gtk_tree_model_get (model, &iter, 0, &d, -1);
		if(!d) return;
		if(data)
		{
		if (!strcmp(d,"Solid") ) {  data->line_style = GDK_LINE_SOLID; }
		else if (!strcmp(d,"On-Off dashed") ) {data->line_style = GDK_LINE_ON_OFF_DASH; }
		else if (!strcmp(d,"Double dashed") ) { data->line_style = GDK_LINE_DOUBLE_DASH;}
		}
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(XYPlotData*)current_node->data;  
				if (!strcmp(d,"Solid") ) {  data->line_style = GDK_LINE_SOLID; }
				else if (!strcmp(d,"On-Off dashed") ) {data->line_style = GDK_LINE_ON_OFF_DASH; }
				else if (!strcmp(d,"Double dashed") ) { data->line_style = GDK_LINE_DOUBLE_DASH;}
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/****************************************************************************************/
static void spin_point_size_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (spinbutton), "CurentData");

  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));


		if(data) 
		{
			data->point_size = gtk_spin_button_get_value(spinbutton);
			xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), data);
		}
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(XYPlotData*)current_node->data;  
				data->point_size = gtk_spin_button_get_value(spinbutton);
				xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), data);
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/********************************************************************************************************/
static void combo_point_style_changed_value (GtkComboBox *combobox, gpointer user_data)
{
	GtkTreeIter iter;
	gchar* d = NULL;

	if (gtk_combo_box_get_active_iter (combobox, &iter))
	{
		GtkTreeModel* model = gtk_combo_box_get_model(combobox);
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (combobox), "CurentData");

  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

		gtk_tree_model_get (model, &iter, 0, &d, -1);
		if(!d) return;
		if(data)
		{
			sprintf(data->point_str,"%s",d);
			xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), data);
		}
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(XYPlotData*)current_node->data;  
				sprintf(data->point_str,"%s",d);
				xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), data);
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/****************************************************************************************/
static void spin_point_color_changed_value(GtkColorButton  *colorbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (colorbutton), "CurentData");
		GdkColor c;

  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

		gtk_color_button_get_color (colorbutton, &c);
		
		if(data)
		{
			data->point_color = c; 
			xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), data);
		}
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(XYPlotData*)current_node->data;  
				data->point_color = c; 
				xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), data);
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/********************************************************************************************************/
static GtkWidget *add_point_types_combo(GtkWidget *hbox, XYPlotData* data)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;
	gint i = 0;
	gint k = 0;
	/* 0x2605=star, 25C6=BLACK DIAMOND,  0x25B2= black triangle, 0x25B3=white triangle */
	/* long int x =0x2605; printf("%d\n",x) in decimal*/
	gchar* list [] = {"<span>&#9733;</span>","+","x",
		"<span>&#9675;</span>", "<span>&#9679;</span>",
		"<span>&#9688;</span>", "<span>&#9689;</span>",
		"<span>&#9670;</span>", "<span>&#9674;</span>",
		"<span>&#8226;</span>",
		"<span><b>&#9788;</b></span>",
		"<span>&#9651;</span>", "<span>&#9650;</span>",
		"."
		};
	gint n = G_N_ELEMENTS (list);

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<n;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
        	gtk_tree_store_set (store, &iter, 0, list[i], -1);
		if(!strcmp(list[i],data->point_str)) k = i;
	}
	

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	gtk_box_pack_start (GTK_BOX (hbox), combobox, TRUE, TRUE, 1);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "markup", 0, NULL);

	gtk_widget_show(combobox); 
	gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), k);

	return combobox;
}
/********************************************************************************************************/
static GtkWidget *add_line_types_combo(GtkWidget *hbox)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Solid", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "On-Off dashed", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Double dashed", -1);

        model = GTK_TREE_MODEL (store);
	combobox = gtk_combo_box_new_with_model (model);
	g_object_unref (model);
	gtk_box_pack_start (GTK_BOX (hbox), combobox, TRUE, TRUE, 1);
	renderer = gtk_cell_renderer_text_new ();
	gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
	gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combobox), renderer, "text", 0, NULL);

	return combobox;
}
/****************************************************************************************/
static gdouble evaluate_formula(G_CONST_RETURN gchar* t)
{
	gdouble a = 1.0;
	gchar* pos = strstr(t,"/");
	if(pos && pos[1]!='\0') a = atof(t)/atof(pos+1);
	else
	{
		pos = strstr(t,"*");
		if(pos && pos[1]!='\0') a = atof(t)*atof(pos+1);
		else a = atof(t);
	}
	return a;
}
/****************************************************************************************/
static void activate_entry_scale_x(GtkEntry *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gint loop;
		gdouble a;
		G_CONST_RETURN gchar* t;

  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		a = evaluate_formula(t);
		if(data)
			for(loop=0;loop<data->size; loop++) data->x[loop] *= a;
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(XYPlotData*)current_node->data;  
				for(loop=0;loop<data->size; loop++) data->x[loop] *= a;
			}
		}
		gtk_entry_set_text(GTK_ENTRY(entry),"1.0");
		gtk_editable_set_position(GTK_EDITABLE(entry),3);
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/****************************************************************************************/
static void activate_entry_scale_y(GtkEntry *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gint loop;
		gdouble a;
		G_CONST_RETURN gchar* t;
  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		a = evaluate_formula(t);
		if(data)
			for(loop=0;loop<data->size; loop++) data->y[loop] *= a;
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(XYPlotData*)current_node->data;  
				for(loop=0;loop<data->size; loop++) data->y[loop] *= a;
			}
		}
		gtk_entry_set_text(GTK_ENTRY(entry),"1.0");
		gtk_editable_set_position(GTK_EDITABLE(entry),3);
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/****************************************************************************************/
static void activate_entry_shift_x(GtkEntry *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gint loop;
		gdouble a;
		G_CONST_RETURN gchar* t;
  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		a = evaluate_formula(t);
		if(data)
			for(loop=0;loop<data->size; loop++) data->x[loop] += a;
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(XYPlotData*)current_node->data;  
				for(loop=0;loop<data->size; loop++) data->x[loop] += a;
			}
		}
		gtk_entry_set_text(GTK_ENTRY(entry),"0.0");
		gtk_editable_set_position(GTK_EDITABLE(entry),3);
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/****************************************************************************************/
static void activate_entry_shift_y(GtkEntry *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gint loop;
		gdouble a;
		G_CONST_RETURN gchar* t;
  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		a = evaluate_formula(t);
		if(data)
			for(loop=0;loop<data->size; loop++) data->y[loop] += a;
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(XYPlotData*)current_node->data;  
				for(loop=0;loop<data->size; loop++) data->y[loop] += a;
			}
		}
		gtk_entry_set_text(GTK_ENTRY(entry),"0.0");
		gtk_editable_set_position(GTK_EDITABLE(entry),3);
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));
	}
}
/****************************************************************************************/
static void build_filter(GtkWidget* buttonFilter, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		gdouble sigma = 10.0;
		G_CONST_RETURN gchar* t;
		GtkWidget* xyplot = GTK_WIDGET(user_data);
		XYPlotData* data = g_object_get_data(G_OBJECT (xyplot), "CurentData");
		GtkWidget* parentWindow = ( GtkWidget*) g_object_get_data(G_OBJECT (buttonFilter), "ParentWindow");
		GtkWidget* entry_sigma = ( GtkWidget*) g_object_get_data(G_OBJECT (xyplot), "EntrySigma");
		GtkWidget* popup = NULL;
		GtkComboBox *combo_filter = ( GtkComboBox*) g_object_get_data(G_OBJECT (xyplot), "ComboFilter");
		GtkTreeIter iter;
		gchar* filter = NULL;

  		g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  		g_return_if_fail (GTK_IS_ENTRY (entry_sigma));

		if(parentWindow) gtk_widget_hide (GTK_WIDGET(parentWindow));
        	while(gtk_events_pending()) gtk_main_iteration();
		popup = create_popup_win(_("Please wait"));

		if (gtk_combo_box_get_active_iter (combo_filter, &iter))
		{
			GtkTreeModel* model = gtk_combo_box_get_model(combo_filter);
			gtk_tree_model_get (model, &iter, 0, &filter, -1);
		}
		if(filter && strstr(filter,"Gaussian")) 
		{
			t= gtk_entry_get_text(GTK_ENTRY(entry_sigma));
			sigma = atof(t);
			if(sigma==0) sigma = 1;
			sigma *=0.5;
		}
		if(data && data->size>2 &&  data->x &&  data->y)
		{
			gdouble* X = g_malloc(data->size*sizeof(gdouble)); 
			gdouble* Y = g_malloc(data->size*sizeof(gdouble)); 
			gint i;
		        GtkWidget* window = NULL;
        		GtkWidget* xyplot = NULL;
			gint N = data->size;
			gdouble xmin;
			gdouble xmax;
			gdouble* x = NULL;
			gdouble* y = NULL;
			gdouble xm = 0.0;

			x = data->x;
			y = data->y;
			xmin = x[0];
			xmax = x[0];
			for(i=0;i<N; i++) 
			{
         			X[i] = x[i];
         			Y[i] = y[i];
				if(xmin>x[i]) xmin = x[i];
				if(xmax<x[i]) xmax = x[i];
   			}
			xmax = (xmax-xmin);
			xm = xmax/2;
			if(filter)
			{
				gint n = 3;
				gdouble alpha = 0.16;
				gdouble a[5] = {(1-alpha)/2,0.5,alpha/2,0.0,0.0};/* Blackman coefs */
				for(i=0;i<N; i++)  X[i] = (X[i]-xmin-xm)/xmax;
				if(strstr(filter,"Nuttall")&& !strstr(filter,"-"))
				{
					a[0] =  0.355768;
					a[1] =  0.487396;
					a[2] =  0.144232;
					a[3] =  0.012604;
					n = 4;
				}
				if(strstr(filter,"Blackman–Harris"))
				{
					a[0] = 0.35875 ;
					a[1] = 0.48829 ;
					a[2] = 0.14128 ;
					a[3] = 0.01168 ;
					n = 4;
				}
				if(strstr(filter,"Blackman–Nuttall"))
				{
					a[0] =  0.3635819;
					a[1] =  0.4891775;
					a[2] =  0.1365995;
					a[3] =  0.0106411;
					n = 4;
				}
				if(strstr(filter,"Flat-Top"))
				{
					a[0] =  1.0;
					a[1] =  1.93;
					a[2] =  1.29;
					a[3] =  0.388;
					a[4] =  0.032;
					n = 5;
				}
				if(strstr(filter,"Gaussian")) for(i=0;i<N; i++)  Y[i] = Y[i]*exp(-X[i]*X[i]*sigma);
				else
				{
				/*	gdouble norm = 0;*/
				     for(i=0;i<N; i++)  
				     {
					gdouble c = a[0];
					gdouble b = X[i];
					gint j;
				        for(j=1;j<n; j++)  c += a[j]*cos(2*M_PI*b*j);
					Y[i] = Y[i]*c;
					/* norm+= c;*/
				     }
					/* printf("norm = %lf\n",norm);*/
				}
				for(i=0;i<N; i++)  X[i] = X[i]*xmax+xmin+xm;
			}
			
			window = gabedit_xyplot_new_window(_("Filter"),NULL);
			xyplot = g_object_get_data(G_OBJECT (window), "XYPLOT");
			xyplot_curve_noconv(GABEDIT_XYPLOT(xyplot), N, X,  Y, NULL);
			gabedit_xyplot_set_range_xmin (GABEDIT_XYPLOT(xyplot), 0.0);
			gabedit_xyplot_set_x_label (GABEDIT_XYPLOT(xyplot), " ");
		}
		if(parentWindow) gtk_object_destroy (GTK_OBJECT(parentWindow));
		if(popup) gtk_object_destroy(GTK_OBJECT(popup));
	}
}
/********************************************************************************************************/
static GtkWidget *add_combo_filter(GtkWidget *hbox)
{
        GtkTreeIter iter;
        GtkTreeStore *store;
	GtkTreeModel *model;
	GtkWidget *combobox;
	GtkCellRenderer *renderer;

	store = gtk_tree_store_new (1,G_TYPE_STRING);

        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Gaussian : exp(-0.5 sigma (t/tmax)^2)", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Blackman", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Nuttall", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Blackman–Harris", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Blackman–Nuttall", -1);
        gtk_tree_store_append (store, &iter, NULL);
        gtk_tree_store_set (store, &iter, 0, "Flat-Top", -1);

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
static void add_filter_data_frame(GtkWidget* hbox, GtkWidget* xyplot, XYPlotData *data, GtkWidget* parentWindow)
{
	GtkWidget* frame = NULL;
	GtkWidget* vbox_frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* label = NULL;
	GtkWidget* entry_sigma = NULL;
	GtkWidget* buttonFilter = NULL;
	GtkWidget* combo = NULL;
	gchar tmp[100];

	frame=gtk_frame_new(_("Filter"));
	gtk_box_pack_start(GTK_BOX(hbox), frame, FALSE, TRUE, 2);
	gtk_widget_show(frame);

	vbox_frame=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox_frame);
	gtk_widget_show(vbox_frame);

	hbox_frame=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_frame), hbox_frame, FALSE, FALSE, 2);
	gtk_widget_show(hbox_frame);

	label=gtk_label_new(_("Type :"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	combo = add_combo_filter(hbox_frame);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
	gtk_widget_show(combo); 
	g_object_set_data(G_OBJECT (xyplot), "ComboFilter", combo);

	/* g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(NULL), xyplot);*/

	label=gtk_label_new(_("Sigma : "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	g_object_set_data(G_OBJECT ( xyplot), "LabelSigma", label);

	entry_sigma = gtk_entry_new();
	gtk_widget_set_size_request(entry_sigma,80,-1);
	sprintf(tmp,"%0.2f",10.0);
	gtk_entry_set_text(GTK_ENTRY(entry_sigma),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_sigma, FALSE, FALSE, 2);
	gtk_widget_show(entry_sigma);
	g_object_set_data(G_OBJECT ( xyplot), "EntrySigma", entry_sigma);

	buttonFilter = gtk_button_new_with_label (_("Apply"));
	gtk_box_pack_start(GTK_BOX(hbox_frame), buttonFilter, TRUE, TRUE, 4);
	gtk_widget_show (buttonFilter);

	g_object_set_data(G_OBJECT ( xyplot), "CurentData", data);
	g_object_set_data(G_OBJECT (buttonFilter), "ParentWindow", parentWindow);

        g_signal_connect(G_OBJECT(buttonFilter), "clicked", G_CALLBACK(build_filter), xyplot);

}
/****************************************************************************************/
static void set_data_dialog(GabeditXYPlot* xyplot, XYPlotData* data)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* frame = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* hbox1 = NULL;
	GtkWidget* label = NULL;
	GtkWidget* combo = NULL;
	GtkWidget* combo_point = NULL;
	GtkWidget* spin = NULL;
	GtkWidget* button = NULL;
	GtkWidget* spin_point = NULL;
	GtkWidget* button_point = NULL;
	GtkWidget* buttonSave = NULL;
	GtkWidget* buttonRemove = NULL;
	GtkWidget* buttonMax1 = NULL;
	GtkWidget* buttonAutoRanges = NULL;
	GtkWidget* buttonAutoRangesAll = NULL;
	GtkWidget* parentWindow = NULL;
	GtkWidget* vbox_window = NULL;
	GtkWidget* entry_scale_x = NULL;
	GtkWidget* entry_scale_y = NULL;
	GtkWidget* entry_shift_x = NULL;
	GtkWidget* entry_shift_y = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set data options"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(destroy_data_dialog), (gpointer)xyplot);

	vbox_window=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox_window);
	gtk_widget_show(vbox_window);

	hbox1=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_window), hbox1, TRUE, FALSE, 2);
	gtk_widget_show(hbox1);

	frame = gtk_frame_new(_("Set line type"));
	gtk_box_pack_start(GTK_BOX(hbox1), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Line width :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin = gtk_spin_button_new_with_range(0, 10, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), data->line_width);
	gtk_box_pack_start(GTK_BOX(hbox), spin, TRUE, FALSE, 2);
	gtk_widget_show(spin);
	g_object_set_data(G_OBJECT (window), "SpinLineWidth", spin);

	label=gtk_label_new(_("Line type :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	combo = add_line_types_combo(hbox);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
	if(data->line_style == GDK_LINE_SOLID) gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
	else if(data->line_style == GDK_LINE_ON_OFF_DASH) gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 1);
	else if(data->line_style == GDK_LINE_DOUBLE_DASH) gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 2);
	gtk_widget_show(combo); 
	g_object_set_data(G_OBJECT (window), "ComboLineType", combo);

	label=gtk_label_new(_("Line color :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	button = gtk_color_button_new_with_color (&data->line_color);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, FALSE, 2);
	gtk_widget_show(button);
	g_object_set_data(G_OBJECT (window), "ColorButton", button);

	frame = gtk_frame_new(_("Set point type"));
	gtk_box_pack_start(GTK_BOX(vbox_window), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Point size :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin_point = gtk_spin_button_new_with_range(0, 30, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_point), data->point_size);
	gtk_box_pack_start(GTK_BOX(hbox), spin_point, TRUE, FALSE, 2);
	gtk_widget_show(spin_point);
	g_object_set_data(G_OBJECT (window), "SpinPointWidth", spin_point);

	label=gtk_label_new(_("Point type :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	combo_point = add_point_types_combo(hbox, data);
	gtk_widget_show(combo_point); 
	g_object_set_data(G_OBJECT (window), "ComboPointType", combo_point);

	label=gtk_label_new(_("Point color :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	button_point = gtk_color_button_new_with_color (&data->point_color);
	gtk_box_pack_start(GTK_BOX(hbox), button_point, TRUE, FALSE, 2);
	gtk_widget_show(button_point);
	g_object_set_data(G_OBJECT (window), "ColorButton", button_point);


	frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox_window), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_widget_show(hbox);


	label=gtk_label_new(_("Scale X : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_scale_x = gtk_entry_new();
	gtk_widget_set_size_request(entry_scale_x,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_scale_x),"1.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_scale_x, FALSE, FALSE, 2);
	gtk_widget_show(entry_scale_x);

	label=gtk_label_new(_("Scale Y : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_scale_y = gtk_entry_new();
	gtk_widget_set_size_request(entry_scale_y,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_scale_y),"1.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_scale_y, FALSE, FALSE, 2);
	gtk_widget_show(entry_scale_y);

	label=gtk_label_new(_("Shift X : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_shift_x = gtk_entry_new();
	gtk_widget_set_size_request(entry_shift_x,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_shift_x),"0.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_shift_x, FALSE, FALSE, 2);
	gtk_widget_show(entry_shift_x);

	label=gtk_label_new(_("Shift Y : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 


	entry_shift_y = gtk_entry_new();
	gtk_widget_set_size_request(entry_shift_y,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_shift_y),"0.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_shift_y, FALSE, FALSE, 2);
	gtk_widget_show(entry_shift_y);

	frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox_window), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_widget_show(hbox);

	buttonAutoRanges = gtk_button_new_with_label (_("Auto ranges"));
	gtk_box_pack_start(GTK_BOX(hbox), buttonAutoRanges, TRUE, TRUE, 4);
	gtk_widget_show (buttonAutoRanges);

	buttonAutoRangesAll = gtk_button_new_with_label (_("Auto ranges all"));
	gtk_box_pack_start(GTK_BOX(hbox), buttonAutoRangesAll, TRUE, TRUE, 4);
	gtk_widget_show (buttonAutoRangesAll);


	buttonSave = gtk_button_new_with_label (_("Save"));
	gtk_box_pack_start(GTK_BOX(hbox), buttonSave, TRUE, TRUE, 4);
	gtk_widget_show (buttonSave);

	buttonMax1 = gtk_button_new_with_label (_("Max=1"));
	gtk_box_pack_start(GTK_BOX(hbox), buttonMax1, TRUE, TRUE, 4);
	gtk_widget_show (buttonMax1);

	buttonRemove = gtk_button_new_with_label (_("Remove"));
	gtk_box_pack_start(GTK_BOX(hbox), buttonRemove, TRUE, TRUE, 4);
	gtk_widget_show (buttonRemove);



	g_object_set_data(G_OBJECT (spin), "CurentData", data);
	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(spin_line_width_changed_value), xyplot);
	g_object_set_data(G_OBJECT (button), "CurentData", data);
	g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(spin_line_color_changed_value), xyplot);
	g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(combo_line_style_changed_value), xyplot);
	g_object_set_data(G_OBJECT (combo), "CurentData", data);

	g_signal_connect(G_OBJECT(combo_point), "changed", G_CALLBACK(combo_point_style_changed_value), xyplot);
	g_object_set_data(G_OBJECT (combo_point), "CurentData", data);

	g_object_set_data(G_OBJECT (spin_point), "CurentData", data);
	g_signal_connect(G_OBJECT(spin_point), "value-changed", G_CALLBACK(spin_point_size_changed_value), xyplot);
	g_object_set_data(G_OBJECT (button_point), "CurentData", data);
	g_signal_connect(G_OBJECT(button_point), "color-set", G_CALLBACK(spin_point_color_changed_value), xyplot);


	g_object_set_data(G_OBJECT (buttonSave), "CurentData", data);
	g_signal_connect(G_OBJECT(buttonSave), "clicked", G_CALLBACK(save_data_2columns_dlg), xyplot);

	g_object_set_data(G_OBJECT (buttonRemove), "CurentData", data);
	g_object_set_data(G_OBJECT (buttonRemove), "Window", window);
	g_object_set_data(G_OBJECT (buttonRemove), "XYPLOT", xyplot);
	g_signal_connect(G_OBJECT(buttonRemove), "clicked", G_CALLBACK(remove_data_dlg), xyplot);

	g_object_set_data(G_OBJECT (buttonAutoRanges), "CurentData", data);
	g_signal_connect(G_OBJECT(buttonAutoRanges), "clicked", G_CALLBACK(auto_range_activate), xyplot);

	g_object_set_data(G_OBJECT (buttonMax1), "CurentData", data);
	g_signal_connect(G_OBJECT(buttonMax1), "clicked", G_CALLBACK(set_max_1_activate), xyplot);

	g_object_set_data(G_OBJECT (buttonAutoRangesAll), "CurentData", NULL);
	g_signal_connect(G_OBJECT(buttonAutoRangesAll), "clicked", G_CALLBACK(auto_range_activate), xyplot);

	g_object_set_data(G_OBJECT (entry_scale_x), "CurentData", data);
	g_object_set_data(G_OBJECT (entry_scale_y), "CurentData", data);
	g_object_set_data(G_OBJECT (entry_shift_x), "CurentData", data);
	g_object_set_data(G_OBJECT (entry_shift_y), "CurentData", data);
	g_signal_connect (G_OBJECT (entry_scale_x), "activate", (GCallback)activate_entry_scale_x, xyplot);
	g_signal_connect (G_OBJECT (entry_scale_y), "activate", (GCallback)activate_entry_scale_y, xyplot);
	g_signal_connect (G_OBJECT (entry_shift_x), "activate", (GCallback)activate_entry_shift_x, xyplot);
	g_signal_connect (G_OBJECT (entry_shift_y), "activate", (GCallback)activate_entry_shift_y, xyplot);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_window), hbox, TRUE, FALSE, 2);
	gtk_widget_show(hbox);
	add_cut_data_frame(hbox, GTK_WIDGET(xyplot), data);

	add_x_linear_data_frame(hbox, GTK_WIDGET(xyplot), data);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_window), hbox, TRUE, FALSE, 2);
	gtk_widget_show(hbox);
	add_filter_data_frame(hbox, GTK_WIDGET(xyplot), data, window);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_window), hbox, TRUE, FALSE, 2);
	gtk_widget_show(hbox);
	add_fourier_data_frame(hbox, GTK_WIDGET(xyplot), data, window);

	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
	/* list_utf8();*/
	

}
/****************************************************************************************/
static void set_all_data_dlg(GtkWidget* xyplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* frame = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* hbox1 = NULL;
	GtkWidget* label = NULL;
	GtkWidget* combo = NULL;
	GtkWidget* combo_point = NULL;
	GtkWidget* spin = NULL;
	GtkWidget* button = NULL;
	GtkWidget* spin_point = NULL;
	GtkWidget* button_point = NULL;
	GtkWidget* buttonAutoRangesAll = NULL;
	GtkWidget* parentWindow = NULL;
	GtkWidget* vbox_window = NULL;
	GtkWidget* entry_scale_x = NULL;
	GtkWidget* entry_scale_y = NULL;
	GtkWidget* entry_shift_x = NULL;
	GtkWidget* entry_shift_y = NULL;

	GList *current_node;
	XYPlotData* data=NULL;

  	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

	current_node=g_list_first(GABEDIT_XYPLOT(xyplot)->data_list);
	if(!current_node) return;
	data=(XYPlotData*)current_node->data;  
	if(!data) return;

	gtk_window_set_title (GTK_WINDOW (window), _("Set all data"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(destroy_data_dialog), (gpointer)xyplot);

	vbox_window=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox_window);
	gtk_widget_show(vbox_window);

	hbox1=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_window), hbox1, TRUE, FALSE, 2);
	gtk_widget_show(hbox1);

	frame = gtk_frame_new(_("Set line type"));
	gtk_box_pack_start(GTK_BOX(hbox1), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Line width :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin = gtk_spin_button_new_with_range(0, 10, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), data->line_width);
	gtk_box_pack_start(GTK_BOX(hbox), spin, TRUE, FALSE, 2);
	gtk_widget_show(spin);
	g_object_set_data(G_OBJECT (window), "SpinLineWidth", spin);

	label=gtk_label_new(_("Line type :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	combo = add_line_types_combo(hbox);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
	if(data->line_style == GDK_LINE_SOLID) gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
	else if(data->line_style == GDK_LINE_ON_OFF_DASH) gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 1);
	else if(data->line_style == GDK_LINE_DOUBLE_DASH) gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 2);
	gtk_widget_show(combo); 
	g_object_set_data(G_OBJECT (window), "ComboLineType", combo);

	label=gtk_label_new(_("Line color :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	button = gtk_color_button_new_with_color (&data->line_color);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, FALSE, 2);
	gtk_widget_show(button);
	g_object_set_data(G_OBJECT (window), "ColorButton", button);

	frame = gtk_frame_new(_("Set point type"));
	gtk_box_pack_start(GTK_BOX(vbox_window), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Point size :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin_point = gtk_spin_button_new_with_range(0, 30, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_point), data->point_size);
	gtk_box_pack_start(GTK_BOX(hbox), spin_point, TRUE, FALSE, 2);
	gtk_widget_show(spin_point);
	g_object_set_data(G_OBJECT (window), "SpinPointWidth", spin_point);

	label=gtk_label_new(_("Point type :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	combo_point = add_point_types_combo(hbox, data);
	gtk_widget_show(combo_point); 
	g_object_set_data(G_OBJECT (window), "ComboPointType", combo_point);

	label=gtk_label_new(_("Point color :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	button_point = gtk_color_button_new_with_color (&data->point_color);
	gtk_box_pack_start(GTK_BOX(hbox), button_point, TRUE, FALSE, 2);
	gtk_widget_show(button_point);
	g_object_set_data(G_OBJECT (window), "ColorButton", button_point);


	frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox_window), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_widget_show(hbox);


	label=gtk_label_new(_("Scale X : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_scale_x = gtk_entry_new();
	gtk_widget_set_size_request(entry_scale_x,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_scale_x),"1.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_scale_x, FALSE, FALSE, 2);
	gtk_widget_show(entry_scale_x);

	label=gtk_label_new(_("Scale Y : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_scale_y = gtk_entry_new();
	gtk_widget_set_size_request(entry_scale_y,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_scale_y),"1.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_scale_y, FALSE, FALSE, 2);
	gtk_widget_show(entry_scale_y);

	label=gtk_label_new(_("Shift X : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_shift_x = gtk_entry_new();
	gtk_widget_set_size_request(entry_shift_x,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_shift_x),"0.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_shift_x, FALSE, FALSE, 2);
	gtk_widget_show(entry_shift_x);

	label=gtk_label_new(_("Shift Y : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 


	entry_shift_y = gtk_entry_new();
	gtk_widget_set_size_request(entry_shift_y,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_shift_y),"0.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_shift_y, FALSE, FALSE, 2);
	gtk_widget_show(entry_shift_y);

	frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox_window), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_widget_show(hbox);

	buttonAutoRangesAll = gtk_button_new_with_label (_("Auto ranges all"));
	gtk_box_pack_start(GTK_BOX(hbox), buttonAutoRangesAll, TRUE, TRUE, 4);
	gtk_widget_show (buttonAutoRangesAll);



	g_object_set_data(G_OBJECT (spin), "CurentData", NULL);
	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(spin_line_width_changed_value), xyplot);
	g_object_set_data(G_OBJECT (button), "CurentData", NULL);
	g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(spin_line_color_changed_value), xyplot);
	g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(combo_line_style_changed_value), xyplot);
	g_object_set_data(G_OBJECT (combo), "CurentData", NULL);

	g_signal_connect(G_OBJECT(combo_point), "changed", G_CALLBACK(combo_point_style_changed_value), xyplot);
	g_object_set_data(G_OBJECT (combo_point), "CurentData", NULL);

	g_object_set_data(G_OBJECT (spin_point), "CurentData", NULL);
	g_signal_connect(G_OBJECT(spin_point), "value-changed", G_CALLBACK(spin_point_size_changed_value), xyplot);
	g_object_set_data(G_OBJECT (button_point), "CurentData", NULL);
	g_signal_connect(G_OBJECT(button_point), "color-set", G_CALLBACK(spin_point_color_changed_value), xyplot);


	g_object_set_data(G_OBJECT (buttonAutoRangesAll), "CurentData", NULL);
	g_signal_connect(G_OBJECT(buttonAutoRangesAll), "clicked", G_CALLBACK(auto_range_activate), xyplot);

	g_object_set_data(G_OBJECT (entry_scale_x), "CurentData", NULL);
	g_object_set_data(G_OBJECT (entry_scale_y), "CurentData", NULL);
	g_object_set_data(G_OBJECT (entry_shift_x), "CurentData", NULL);
	g_object_set_data(G_OBJECT (entry_shift_y), "CurentData", NULL);
	g_signal_connect (G_OBJECT (entry_scale_x), "activate", (GCallback)activate_entry_scale_x, xyplot);
	g_signal_connect (G_OBJECT (entry_scale_y), "activate", (GCallback)activate_entry_scale_y, xyplot);
	g_signal_connect (G_OBJECT (entry_shift_x), "activate", (GCallback)activate_entry_shift_x, xyplot);
	g_signal_connect (G_OBJECT (entry_shift_y), "activate", (GCallback)activate_entry_shift_y, xyplot);

	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
	/* list_utf8();*/
	

}
/*********************************************************************************************************************/
static void toggle_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	gboolean enable = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
	GtkWidget* xyplot = g_object_get_data(G_OBJECT (action), "XYPLOT");
	if(!xyplot) return;
	if(!strcmp(name,"HGridShowMajor")) 
		xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_HMAJOR_GRID, enable);
	else if(!strcmp(name,"HGridShowMinor")) 
		xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_HMINOR_GRID, enable);
	else if(!strcmp(name,"VGridShowMajor"))
		xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_VMAJOR_GRID, enable);
	else if(!strcmp(name,"VGridShowMinor"))
		xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_VMINOR_GRID, enable);
	else if(!strcmp(name,"LegendShowLeft"))
		xyplot_show_left_legends (GABEDIT_XYPLOT(xyplot), enable);
	else if(!strcmp(name,"LegendShowRight"))
		xyplot_show_right_legends (GABEDIT_XYPLOT(xyplot), enable);
	else if(!strcmp(name,"LegendShowTop"))
		xyplot_show_top_legends (GABEDIT_XYPLOT(xyplot), enable);
	else if(!strcmp(name,"LegendShowBottom"))
		xyplot_show_bottom_legends (GABEDIT_XYPLOT(xyplot), enable);
	else if(!strcmp(name,"LegendShowRectangle"))
		xyplot_show_rectangle_legends (GABEDIT_XYPLOT(xyplot), enable);
	else if(!strcmp(name,"DirectionReflectX"))
		xyplot_reflect_x (GABEDIT_XYPLOT(xyplot), enable);
	else if(!strcmp(name,"DirectionReflectY"))
		xyplot_reflect_y (GABEDIT_XYPLOT(xyplot), enable);
}
/*--------------------------------------------------------------------------------------------------------------------*/
static GtkToggleActionEntry gtkActionToggleEntries[] =
{
	{ "HGridShowMajor", NULL, N_("show _H major"), NULL, "show H major", G_CALLBACK (toggle_action), TRUE },
	{ "HGridShowMinor", NULL, N_("show _H minor"), NULL, "show H minor", G_CALLBACK (toggle_action), FALSE },
	{ "VGridShowMajor", NULL, N_("show _V major"), NULL, "show V major", G_CALLBACK (toggle_action), TRUE },
	{ "VGridShowMinor", NULL, N_("show _V minor"), NULL, "show V minor", G_CALLBACK (toggle_action), FALSE },
	{ "LegendShowLeft", NULL, N_("show _left"), NULL, "show left", G_CALLBACK (toggle_action), TRUE },
	{ "LegendShowRight", NULL, N_("show _right"), NULL, "show right", G_CALLBACK (toggle_action), TRUE },
	{ "LegendShowTop", NULL, N_("show _top"), NULL, "show top", G_CALLBACK (toggle_action), TRUE },
	{ "LegendShowBottom", NULL, N_("show _bottom"), NULL, "show bottom", G_CALLBACK (toggle_action), TRUE },
	{ "LegendShowRectangle", NULL, N_("show _rectangle"), NULL, "show rectangle", G_CALLBACK (toggle_action), TRUE },
	{ "DirectionReflectX", NULL, N_("_X reflect"), NULL, "X reflect", G_CALLBACK (toggle_action), FALSE },
	{ "DirectionReflectY", NULL, N_("_Y reflect"), NULL, "Y reflect", G_CALLBACK (toggle_action), FALSE },
};
/*--------------------------------------------------------------------------------------------------------------------*/
static void init_toggle_entries (GtkWidget* xyplot)
{
	xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_HMAJOR_GRID, gtkActionToggleEntries[0].is_active);
	xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_HMINOR_GRID, gtkActionToggleEntries[1].is_active);
	xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_VMAJOR_GRID, gtkActionToggleEntries[2].is_active);
	xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_VMINOR_GRID, gtkActionToggleEntries[3].is_active);
	xyplot_show_left_legends (GABEDIT_XYPLOT(xyplot), gtkActionToggleEntries[4].is_active);
	xyplot_show_right_legends (GABEDIT_XYPLOT(xyplot), gtkActionToggleEntries[5].is_active);
	xyplot_show_top_legends (GABEDIT_XYPLOT(xyplot), gtkActionToggleEntries[6].is_active);
	xyplot_show_bottom_legends (GABEDIT_XYPLOT(xyplot), gtkActionToggleEntries[7].is_active);
	xyplot_show_rectangle_legends (GABEDIT_XYPLOT(xyplot), gtkActionToggleEntries[8].is_active);

	xyplot_reflect_x (GABEDIT_XYPLOT(xyplot), gtkActionToggleEntries[9].is_active);
	xyplot_reflect_y (GABEDIT_XYPLOT(xyplot), gtkActionToggleEntries[10].is_active);
}
static gint numberOfGtkActionToggleEntries = G_N_ELEMENTS (gtkActionToggleEntries);
/*********************************************************************************************************************/
static void activate_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	GtkWidget* xyplot = g_object_get_data(G_OBJECT (action), "XYPLOT");
	if(!strcmp(name,"SetTicks")) { set_ticks_dialog(xyplot); }
	if(!strcmp(name,"SetMargins")) { set_margins_dialog(xyplot); }
	if(!strcmp(name,"SetRanges")) { set_ranges_dialog(xyplot); }
	if(!strcmp(name,"SetLabels")) { set_labels_dialog(xyplot); }
	if(!strcmp(name,"SetDigits")) { set_digits_dialog(xyplot); }
	if(!strcmp(name,"SetFontSize")) { set_font_size_dialog(xyplot); }
	if(!strcmp(name,"SetAll")) { set_all_dialog(xyplot); }
	if(!strcmp(name,"SetAutoRanges")) { gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(xyplot), NULL); }
	if(!strcmp(name,"DataRead2Columns")) { read_data_2columns_dlg(xyplot,FALSE); }
	if(!strcmp(name,"DataRead2ColumnsPeaks")) { read_data_2columns_dlg(xyplot,TRUE); }
	if(!strcmp(name,"DataRead1Column")) { read_data_1column_dlg(xyplot,FALSE); }
	if(!strcmp(name,"DataRead1ColumnPeaks")) { read_data_1column_dlg(xyplot,TRUE); }
	if(!strcmp(name,"DataReadXY1YnColumns")) { read_data_xy1yncolumns_dlg(xyplot,FALSE); }
	if(!strcmp(name,"DataReadXY1YnColumnsPeaks")) { read_data_xy1yncolumns_dlg(xyplot,TRUE); }
	if(!strcmp(name,"DataReadJDX")) { read_data_jdx_dlg(xyplot); }
	if(!strcmp(name,"DataReadJMRUI")) { read_data_jMRUI_dlg(xyplot); }
	if(!strcmp(name,"DataSaveAll")) { save_all_data_2columns_dlg(xyplot); }
	if(!strcmp(name,"DataRemoveAll")) { remove_all_data_dlg(xyplot); }
	if(!strcmp(name,"DataChangeAll")) { set_all_data_dlg(xyplot); }
	if(!strcmp(name,"ObjectsInsertText")) { begin_insert_objects_text(xyplot); }
	if(!strcmp(name,"ObjectsInsertImage")) { begin_insert_objects_image(xyplot); }
	if(!strcmp(name,"ObjectsInsertImageClip")) { insert_objects_image_from_clipboard(xyplot);}
	if(!strcmp(name,"ThemesPublication")) { set_theme_publication(xyplot); }
	if(!strcmp(name,"ThemesGreenBlack")) { set_theme_green_black(xyplot); }
	if(!strcmp(name,"ThemesOther")) { set_theme_dialog(xyplot); }
	if(!strcmp(name,"ObjectsDeleteTexts")) { delete_objects_text(xyplot); }
	if(!strcmp(name,"ObjectsDeleteImage")) { delete_objects_image(xyplot); }
	if(!strcmp(name,"ObjectsInsertLine")) { begin_insert_objects_line(xyplot); }
	if(!strcmp(name,"ObjectsDeleteLines")) { delete_objects_line(xyplot); }
	if(!strcmp(name,"ScreenCaptureBMP")) {  saveImageDlg(xyplot, "bmp");}
	if(!strcmp(name,"ScreenCaptureJPEG")) {  saveImageDlg(xyplot, "jpeg");}
	if(!strcmp(name,"ScreenCapturePNG")) {  saveImageDlg(xyplot, "png");}
	if(!strcmp(name,"ScreenCaptureTPNG")) {  saveImageDlg(xyplot, "tpng");}
	if(!strcmp(name,"ScreenCaptureTIF")) {  saveImageDlg(xyplot, "tif");}
	if(!strcmp(name,"ScreenCaptureClipBoard")) {  copyImageToClipBoard(xyplot); }
	if(!strcmp(name,"ExportSVG")) {  exportImageDlg(xyplot, "svg");}
	if(!strcmp(name,"ExportPDF")) {  exportImageDlg(xyplot, "pdf");}
	if(!strcmp(name,"ExportPS")) {  exportImageDlg(xyplot, "ps");}
	if(!strcmp(name,"ExportEPS")) {  exportImageDlg(xyplot, "eps");}
	if(!strcmp(name,"Read")) {  readAGabeditDlg(xyplot);}
	if(!strcmp(name,"Save")) {  saveAsGabeditDlg(xyplot);}
	if(!strcmp(name,"Help")) {  gabedit_xyplot_help();}
	if(!strcmp(name,"Close")) {  destroy_xyplot_window(xyplot);}
}
/*--------------------------------------------------------------------*/
static GtkActionEntry gtkActionEntries[] =
{
	{"Set", NULL, N_("_Set")},
	{"SetTicks", NULL, N_("_Ticks"), NULL, "Ticks", G_CALLBACK (activate_action) },
	{"SetMargins", NULL, N_("_Margins"), NULL, "Margins", G_CALLBACK (activate_action) },
	{"SetRanges", NULL, N_("_Ranges"), NULL, "Ranges", G_CALLBACK (activate_action) },
	{"SetLabels", NULL, N_("_Labels"), NULL, "Labels", G_CALLBACK (activate_action) },
	{"SetDigits", NULL, N_("_Digits"), NULL, "Digits", G_CALLBACK (activate_action) },
	{"SetFontSize", NULL, N_("_Font size"), NULL, "Font size", G_CALLBACK (activate_action) },
	{"SetAll", NULL, N_("_All"), NULL, "All", G_CALLBACK (activate_action) },
	{"SetAutoRanges", NULL, N_("_Auto ranges"), NULL, "Auto ranges", G_CALLBACK (activate_action) },
	{"Render", NULL, N_("_Render")},
	{"RenderGrid", NULL, N_("_Grid")},
	{"RenderLegends", NULL, N_("_Legends")},
	{"RenderDirections", NULL, N_("_Directions")},
	{"Data", NULL, N_("_Data")},
	{"DataAdd", NULL, N_("_Add Data")},
	{"DataRead2Columns", NULL, N_("_Read data from an ASCII XY file(2 columns)"), NULL, "Read data from an ASCII XY file(2 columns)", G_CALLBACK (activate_action) },
	{"DataRead1Column", NULL, N_("_Read data from an ASCII Y file(1 column)"), NULL, "Read data from an ASCII Y file(1 column)", G_CALLBACK (activate_action) },
	{"DataReadXY1YnColumns", NULL, N_("_Read data from an ASCII X.Y1..Yn file(x, y1, y2,...,yn)"), NULL, "Read data from an ASCII XY file(2 columns)", G_CALLBACK (activate_action) },
	{"DataReadJDX", NULL, N_("_Read data from a JDX file"), NULL, "Read data from a JDX file", G_CALLBACK (activate_action) },
	{"DataReadJMRUI", NULL, N_("_Read data from a jMRUI text file"), NULL, "Read data from a jMRUI text file", G_CALLBACK (activate_action) },
	{"DataRead2ColumnsPeaks", NULL, N_("_Read data from an ASCII XY file(2 columns) and draw peaks"), NULL, "Read data from an ASCII XY file(2 columns) and draw peaks", G_CALLBACK (activate_action) },
	{"DataRead1ColumnPeaks", NULL, N_("_Read data from an ASCII Y file(1 column) and draw peaks"), NULL, "Read data from an ASCII Y file(1 column) and draw peaks", G_CALLBACK (activate_action) },
	{"DataReadXY1YnColumnsPeaks", NULL, N_("_Read data from an ASCII X.Y1..Yn file(x, y1, y2,...,yn) and draw peaks"), NULL, "Read data from an ASCII XY file(2 columns) and draw peaks", G_CALLBACK (activate_action) },
	{"DataSaveAll", NULL, N_("_Save all data in an ascii XY file(2columns)"), NULL, "Save all data in an ascii file(2columns)", G_CALLBACK (activate_action) },
	{"DataRemoveAll", NULL, N_("_Remove all data"), NULL, "Remove all data", G_CALLBACK (activate_action) },
	{"DataChangeAll", NULL, N_("_Change all"), NULL, "change all", G_CALLBACK (activate_action) },
	{"Objects", NULL, N_("_Objects")},
	{"ObjectsInsertText", NULL, N_("Insert a _text(Click in window)"), NULL, "Insert a text", G_CALLBACK (activate_action) },
	{"ObjectsDeleteTexts", NULL, N_("Delete all _texts"), NULL, "Delete all texts", G_CALLBACK (activate_action) },
	{"ObjectsInsertLine", NULL, N_("Insert a _line(Click in window and move)"), NULL, "Insert a line", G_CALLBACK (activate_action) },
	{"ObjectsDeleteLines", NULL, N_("Delete all _lines"), NULL, "Delete all lines", G_CALLBACK (activate_action) },
	{"ObjectsInsertImage", NULL, N_("Insert an _image from a png file(Click in window)"), NULL, "Insert an image", G_CALLBACK (activate_action) },
	{"ObjectsInsertImageClip", NULL, N_("Insert an _image from clipboard"), NULL, "Insert an image", G_CALLBACK (activate_action) },
	{"ObjectsDeleteImage", NULL, N_("Delete all _images"), NULL, "Delete all images", G_CALLBACK (activate_action) },
	{"Themes", NULL, N_("_Themes")},
	{"ThemesPublication", NULL, N_("_Publication"), NULL, "Publication", G_CALLBACK (activate_action) },
	{"ThemesGreenBlack", NULL, N_("_Green&black"), NULL, "Green&black", G_CALLBACK (activate_action) },
	{"ThemesOther", NULL, N_("_Other"), NULL, "Other", G_CALLBACK (activate_action) },
	{"ScreenCapture", NULL, N_("_Screen Capture")},
	{"ScreenCaptureBMP", NULL, N_("_BMP format"), NULL, "BMP format", G_CALLBACK (activate_action) },
	{"ScreenCaptureJPEG", NULL, N_("_JPEG format"), NULL, "JPEG format", G_CALLBACK (activate_action) },
	{"ScreenCapturePNG", NULL, N_("_PNG format"), NULL, "PNG format", G_CALLBACK (activate_action) },
	{"ScreenCaptureTPNG", NULL, N_("_Transparent PNG format"), NULL, "Transparent PNG format", G_CALLBACK (activate_action) },
	{"ScreenCaptureTIF", NULL, N_("_TIF format"), NULL, "TIF format", G_CALLBACK (activate_action) },
	{"ScreenCaptureClipBoard", NULL, N_("_Copy to clipboard"), NULL, "Copy to clipboard", G_CALLBACK (activate_action) },
	{"Export", NULL, N_("_Export")},
	{"ExportSVG", NULL, N_("Export _SVG format"), NULL, "SVG format", G_CALLBACK (activate_action) },
	{"ExportPDF", NULL, N_("Export p_df format"), NULL, "PDF format", G_CALLBACK (activate_action) },
	{"ExportPS", NULL, N_("Export _postscript format"), NULL, "PS format", G_CALLBACK (activate_action) },
	{"ExportEPS", NULL, N_("Export _Encapsuled postscript format"), NULL, "EPS format", G_CALLBACK (activate_action) },
	{"Read", GTK_STOCK_OPEN, N_("_Read"), NULL, "Read", G_CALLBACK (activate_action) },
	{"Save", GTK_STOCK_SAVE, N_("_Save"), NULL, "Save", G_CALLBACK (activate_action) },
	{"Help", GTK_STOCK_HELP, N_("_Help"), NULL, "Help", G_CALLBACK (activate_action) },
	{"Close", GTK_STOCK_CLOSE, N_("_Close"), NULL, "Close", G_CALLBACK (activate_action) },
};


static gint numberOfGtkActionEntries = G_N_ELEMENTS (gtkActionEntries);
/********************************************************************************/
static void add_data_to_actions(GtkUIManager *manager, GtkWidget   *xyplot)
{
	GtkAction* action = NULL;
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Set/SetTicks");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Set/SetMargins");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Set/SetRanges");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Set/SetLabels");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Set/SetDigits");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Set/SetFontSize");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Set/SetAll");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Set/SetAutoRanges");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderGrid/HGridShowMajor");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderGrid/HGridShowMinor");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderGrid/VGridShowMajor");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderGrid/VGridShowMinor");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderLegends/LegendShowLeft");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderLegends/LegendShowRight");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderLegends/LegendShowTop");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderLegends/LegendShowBottom");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderLegends/LegendShowRectangle");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderDirections/DirectionReflectX");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderDirections/DirectionReflectY");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Data/DataAdd/DataRead2Columns");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Data/DataAdd/DataRead2ColumnsPeaks");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Data/DataAdd/DataRead1Column");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Data/DataAdd/DataRead1ColumnPeaks");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Data/DataAdd/DataReadXY1YnColumns");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Data/DataAdd/DataReadXY1YnColumnsPeaks");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Data/DataAdd/DataReadJDX");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Data/DataAdd/DataReadJMRUI");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Data/DataSaveAll");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Data/DataRemoveAll");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Data/DataChangeAll");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Objects/ObjectsDeleteTexts");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Objects/ObjectsInsertText");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Objects/ObjectsInsertImage");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Objects/ObjectsInsertImageClip");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Objects/ObjectsDeleteImage");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Themes/ThemesPublication");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Themes/ThemesGreenBlack");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Themes/ThemesOther");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Objects/ObjectsDeleteLines");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Objects/ObjectsInsertLine");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/ScreenCapture/ScreenCaptureBMP");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/ScreenCapture/ScreenCaptureJPEG");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/ScreenCapture/ScreenCapturePNG");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/ScreenCapture/ScreenCaptureTPNG");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/ScreenCapture/ScreenCaptureTIF");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);
	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/ScreenCapture/ScreenCaptureClipBoard");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Export/ExportSVG");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Export/ExportPDF");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);


	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Export/ExportPS");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Export/ExportEPS");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Read");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Save");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Help");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

	action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Close");
	if(action) g_object_set_data(G_OBJECT (action), "XYPLOT", xyplot);

}
/********************************************************************************/
static const gchar *uiMenuInfo =
"  <popup name=\"MenuXYPlot\">\n"
"    <separator name=\"sepMenuPopSet\" />\n"
"    <menu name=\"Set\" action=\"Set\">\n"
"      <menuitem name=\"SetTicks\" action=\"SetTicks\" />\n"
"      <menuitem name=\"SetMargins\" action=\"SetMargins\" />\n"
"      <menuitem name=\"SetRanges\" action=\"SetRanges\" />\n"
"      <menuitem name=\"SetLabels\" action=\"SetLabels\" />\n"
"      <menuitem name=\"SetDigits\" action=\"SetDigits\" />\n"
"      <menuitem name=\"SetFontSize\" action=\"SetFontSize\" />\n"
"      <menuitem name=\"SetAll\" action=\"SetAll\" />\n"
"      <separator name=\"sepSetAutoRanges\" />\n"
"      <menuitem name=\"SetAutoRanges\" action=\"SetAutoRanges\" />\n"
"    </menu>\n"
"    <separator name=\"sepRender\" />\n"
"    <menu name=\"Render\" action=\"Render\">\n"
"      <menu name=\"RenderGrid\" action=\"RenderGrid\">\n"
"        <menuitem name=\"HGridShowMajor\" action=\"HGridShowMajor\" />\n"
"        <menuitem name=\"HGridShowMinor\" action=\"HGridShowMinor\" />\n"
"        <menuitem name=\"VGridShowMajor\" action=\"VGridShowMajor\" />\n"
"        <menuitem name=\"VGridShowMinor\" action=\"VGridShowMinor\" />\n"
"      </menu>\n"
"      <menu name=\"RenderLegends\" action=\"RenderLegends\">\n"
"        <menuitem name=\"LegendShowLeft\" action=\"LegendShowLeft\" />\n"
"        <menuitem name=\"LegendShowRight\" action=\"LegendShowRight\" />\n"
"        <menuitem name=\"LegendShowTop\" action=\"LegendShowTop\" />\n"
"        <menuitem name=\"LegendShowBottom\" action=\"LegendShowBottom\" />\n"
"        <menuitem name=\"LegendShowRectangle\" action=\"LegendShowRectangle\" />\n"
"      </menu>\n"
"      <menu name=\"RenderDirections\" action=\"RenderDirections\">\n"
"        <menuitem name=\"DirectionReflectX\" action=\"DirectionReflectX\" />\n"
"        <menuitem name=\"DirectionReflectY\" action=\"DirectionReflectY\" />\n"
"      </menu>\n"
"    </menu>\n"
"    <separator name=\"sepData\" />\n"
"    <menu name=\"Data\" action=\"Data\">\n"
"      <menu name=\"DataAdd\" action=\"DataAdd\">\n"
"        <menuitem name=\"DataRead2Columns\" action=\"DataRead2Columns\" />\n"
"        <menuitem name=\"DataReadXY1YnColumns\" action=\"DataReadXY1YnColumns\" />\n"
"        <menuitem name=\"DataRead1Column\" action=\"DataRead1Column\" />\n"
"        <menuitem name=\"DataReadJDX\" action=\"DataReadJDX\" />\n"
"        <menuitem name=\"DataReadJMRUI\" action=\"DataReadJMRUI\" />\n"
"        <separator name=\"sepPeaks\" />\n"
"        <menuitem name=\"DataRead2ColumnsPeaks\" action=\"DataRead2ColumnsPeaks\" />\n"
"        <menuitem name=\"DataRead1ColumnPeaks\" action=\"DataRead1ColumnPeaks\" />\n"
"        <menuitem name=\"DataReadXY1YnColumnsPeaks\" action=\"DataReadXY1YnColumnsPeaks\" />\n"
"      </menu>\n"
"        <separator name=\"sepDataSaveAll\" />\n"
"        <menuitem name=\"DataSaveAll\" action=\"DataSaveAll\" />\n"
"        <separator name=\"sepDataRemoveAll\" />\n"
"        <menuitem name=\"DataRemoveAll\" action=\"DataRemoveAll\" />\n"
"        <separator name=\"sepDataChangeAll\" />\n"
"        <menuitem name=\"DataChangeAll\" action=\"DataChangeAll\" />\n"
"    </menu>\n"
"    <separator name=\"sepObjects\" />\n"
"    <menu name=\"Objects\" action=\"Objects\">\n"
"      <menuitem name=\"ObjectsInsertText\" action=\"ObjectsInsertText\" />\n"
"      <menuitem name=\"ObjectsDeleteTexts\" action=\"ObjectsDeleteTexts\" />\n"
"      <menuitem name=\"ObjectsInsertLine\" action=\"ObjectsInsertLine\" />\n"
"      <menuitem name=\"ObjectsDeleteLines\" action=\"ObjectsDeleteLines\" />\n"
"      <menuitem name=\"ObjectsInsertImage\" action=\"ObjectsInsertImage\" />\n"
"      <menuitem name=\"ObjectsInsertImageClip\" action=\"ObjectsInsertImageClip\" />\n"
"      <menuitem name=\"ObjectsDeleteImage\" action=\"ObjectsDeleteImage\" />\n"
"    </menu>\n"
"    <separator name=\"sepThemes\" />\n"
"    <menu name=\"Themes\" action=\"Themes\">\n"
"      <menuitem name=\"ThemesPublication\" action=\"ThemesPublication\" />\n"
"      <menuitem name=\"ThemesGreenBlack\" action=\"ThemesGreenBlack\" />\n"
"      <menuitem name=\"ThemesOther\" action=\"ThemesOther\" />\n"
"    </menu>\n"
"    <separator name=\"sepScreenCapture\" />\n"
"    <separator name=\"sepScreenCapture\" />\n"
"    <menu name=\"ScreenCapture\" action=\"ScreenCapture\">\n"
"      <menuitem name=\"ScreenCaptureBMP\" action=\"ScreenCaptureBMP\" />\n"
"      <menuitem name=\"ScreenCaptureJPEG\" action=\"ScreenCaptureJPEG\" />\n"
"      <menuitem name=\"ScreenCapturePNG\" action=\"ScreenCapturePNG\" />\n"
"      <menuitem name=\"ScreenCaptureTPNG\" action=\"ScreenCaptureTPNG\" />\n"
"      <menuitem name=\"ScreenCaptureTIF\" action=\"ScreenCaptureTIF\" />\n"
"      <menuitem name=\"ScreenCaptureClipBoard\" action=\"ScreenCaptureClipBoard\" />\n"
"    </menu>\n"
"    <separator name=\"sepExport\" />\n"
"    <menu name=\"Export\" action=\"Export\">\n"
"      <menuitem name=\"ExportSVG\" action=\"ExportSVG\" />\n"
"      <menuitem name=\"ExportPDF\" action=\"ExportPDF\" />\n"
"      <menuitem name=\"ExportPS\" action=\"ExportPS\" />\n"
"      <menuitem name=\"ExportEPS\" action=\"ExportEPS\" />\n"
"    </menu>\n"
"    <separator name=\"sepRead\" />\n"
"      <menuitem name=\"Read\" action=\"Read\" />\n"
"    <separator name=\"sepSave\" />\n"
"      <menuitem name=\"Save\" action=\"Save\" />\n"
"    <separator name=\"sepHelp\" />\n"
"      <menuitem name=\"Help\" action=\"Help\" />\n"
"    <separator name=\"sepClose\" />\n"
"      <menuitem name=\"Close\" action=\"Close\" />\n"
"  </popup>\n"
"  <toolbar action=\"ToolbarXYPlot\">\n"
"      <toolitem name=\"SetTicks\" action=\"SetTicks\" />\n"
"      <toolitem name=\"SetMargins\" action=\"SetMargins\" />\n"
"      <toolitem name=\"SetRanges\" action=\"SetRanges\" />\n"
"      <toolitem name=\"ScreenCaptureBMP\" action=\"ScreenCaptureBMP\" />\n"
"      <toolitem name=\"ScreenCaptureJPEG\" action=\"ScreenCaptureJPEG\" />\n"
"      <toolitem name=\"ScreenCapturePNG\" action=\"ScreenCapturePNG\" />\n"
"      <toolitem name=\"ScreenCaptureTPNG\" action=\"ScreenCaptureTPNG\" />\n"
"      <toolitem name=\"ScreenCaptureTIF\" action=\"ScreenCaptureTIF\" />\n"
"      <toolitem name=\"ScreenCaptureClipBoard\" action=\"ScreenCaptureClipBoard\" />\n"
"  </toolbar>\n"
;
/*****************************************************************************************/
static void add_widget (GtkUIManager *merge, GtkWidget   *widget, GtkContainer *container)
{
	GtkWidget *handlebox;

	if (!GTK_IS_TOOLBAR (widget))  return;

	handlebox =gtk_handle_box_new ();
	g_object_ref (handlebox);
  	gtk_handle_box_set_handle_position  (GTK_HANDLE_BOX(handlebox),GTK_POS_TOP);
	/*   GTK_SHADOW_NONE,  GTK_SHADOW_IN,  GTK_SHADOW_OUT, GTK_SHADOW_ETCHED_IN, GTK_SHADOW_ETCHED_OUT */
	gtk_handle_box_set_shadow_type(GTK_HANDLE_BOX(handlebox),GTK_SHADOW_OUT);
	gtk_box_pack_start (GTK_BOX (container), handlebox, FALSE, FALSE, 0);

	if (GTK_IS_TOOLBAR (widget)) 
	{
		GtkToolbar *toolbar;
		toolbar = GTK_TOOLBAR (widget);
		gtk_toolbar_set_show_arrow (toolbar, TRUE);
		gtk_toolbar_set_style(toolbar, GTK_TOOLBAR_ICONS);
		gtk_toolbar_set_orientation(toolbar,  GTK_ORIENTATION_VERTICAL);
	}
	gtk_widget_show (widget);
	gtk_container_add (GTK_CONTAINER (handlebox), widget);
	gtk_widget_show (handlebox);
}
/*************************************************************************************************/
static void add_toolbar_and_popup_menu(GtkWidget* parent, GtkWidget* box)
{
	GtkActionGroup *actionGroup = NULL;
	GtkUIManager *merge = NULL;
	GError *error = NULL;

  	merge = gtk_ui_manager_new ();
  	g_signal_connect_swapped (parent, "destroy", G_CALLBACK (g_object_unref), merge);

	actionGroup = gtk_action_group_new ("GabeditXYPlotActions");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntries, numberOfGtkActionEntries, NULL);
	gtk_action_group_add_toggle_actions (actionGroup, gtkActionToggleEntries, numberOfGtkActionToggleEntries, NULL);
  	gtk_ui_manager_insert_action_group (merge, actionGroup, 0);

	if(box) g_signal_connect (merge, "add_widget", G_CALLBACK (add_widget), box);
	if (!gtk_ui_manager_add_ui_from_string (merge, uiMenuInfo, -1, &error))
	{
		g_message (_("building menus of xyplot failed: %s"), error->message);
		g_error_free (error);
	}
	g_object_set_data(G_OBJECT (parent), "Manager", merge);
	add_data_to_actions(merge,parent);
	init_toggle_entries (parent);
}
/****************************************************************************************/
GType gabedit_xyplot_get_type ()
{

	static GType xyplot_type = 0;
	if (!xyplot_type)
       	{
             static const GTypeInfo xyplot_info =
            {
	           sizeof (GabeditXYPlotClass),
	           NULL,           /* base_init */
	           NULL,           /* base_finalize */
	           (GClassInitFunc) gabedit_xyplot_class_init,
	           NULL,           /* class_finalize */
	           NULL,           /* class_data */
	           sizeof (GabeditXYPlot),
	           0,             /* n_preallocs */
	           (GInstanceInitFunc) gabedit_xyplot_init,
           };
	   xyplot_type = g_type_register_static (GTK_TYPE_WIDGET, "GabeditXYPlot", &xyplot_info, 0);
	}
	return xyplot_type;
}
/****************************************************************************************/
static void gabedit_xyplot_class_init (GabeditXYPlotClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;

  parent_class = g_type_class_peek_parent (class);

  object_class->destroy = gabedit_xyplot_destroy;

  widget_class->realize = gabedit_xyplot_realize;
  widget_class->expose_event = gabedit_xyplot_expose;
  widget_class->size_request = gabedit_xyplot_size_request;
  widget_class->size_allocate = gabedit_xyplot_size_allocate;
  widget_class->button_press_event = gabedit_xyplot_button_press;
  widget_class->button_release_event = gabedit_xyplot_button_release;
  widget_class->motion_notify_event = gabedit_xyplot_motion_notify;
  widget_class->scroll_event = gabedit_xyplot_scroll;
  widget_class->style_set = gabedit_xyplot_style_set;
  widget_class->key_press_event = gabedit_xyplot_key_press;
  widget_class->key_release_event = gabedit_xyplot_key_release;
  widget_class->enter_notify_event = gabedit_xyplot_grab;
  widget_class->leave_notify_event = gabedit_xyplot_grab;
}
/****************************************************************************************/
static void gabedit_xyplot_init (GabeditXYPlot *xyplot)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
     
  xyplot->xmin=-1.0;
  xyplot->xmax=1.0;
  xyplot->ymin=-1.0;
  xyplot->ymax=1.0;
    
  xyplot->x_legends_digits=8;
  xyplot->y_legends_digits=8;

  xyplot->hmajor_ticks=11;
  xyplot->hminor_ticks=3;
  xyplot->vmajor_ticks=11;
  xyplot->vminor_ticks=3;
  xyplot->length_ticks=3;

  xyplot->left_margins =0;
  xyplot->top_margins =0;
  xyplot->right_margins =0;
  xyplot->bottom_margins =0;
  
  xyplot->hmajor_grid=TRUE;
  xyplot->hminor_grid=TRUE;
  xyplot->vmajor_grid=TRUE;
  xyplot->vminor_grid=TRUE;  

  xyplot->show_left_legends = TRUE;
  xyplot->show_right_legends = FALSE;
  xyplot->show_top_legends = FALSE;
  xyplot->show_bottom_legends  = TRUE;
  xyplot->show_rectangle_legends  = TRUE;

  xyplot->reflect_x  = FALSE;
  xyplot->reflect_y  = FALSE;

  xyplot->data_list=NULL;

  xyplot->mouse_zoom_enabled=TRUE;
  xyplot->mouse_zoom_button = 2;
  xyplot->mouse_displace_enabled=FALSE;
  xyplot->wheel_zoom_enabled=FALSE;
  xyplot->mouse_autorange_enabled=FALSE;
  xyplot->mouse_autorange_button = 1;
  xyplot->mouse_distance_enabled=TRUE;
  xyplot->mouse_distance_button = 1;

  xyplot->h_label_str = NULL;
  xyplot->v_label_str = NULL;
  xyplot->h_label = NULL;
  xyplot->v_label = NULL;
  xyplot->h_label_width = 0;
  xyplot->h_label_height = 0;
  xyplot->v_label_width = 0;
  xyplot->v_label_height = 0;
  xyplot->font_size = 12;
  xyplot->cairo_widget = NULL;
  xyplot->cairo_area = NULL;
  xyplot->cairo_export = NULL;
  xyplot->shift_key_pressed = FALSE;
  xyplot->control_key_pressed = FALSE;

  xyplot->nObjectsText = 0;
  xyplot->objectsText = NULL;
  xyplot->t_key_pressed = FALSE;
  xyplot->selected_objects_text_num = -1;

  xyplot->nObjectsLine = 0;
  xyplot->objectsLine = NULL;
  xyplot->l_key_pressed = FALSE;
  xyplot->selected_objects_line_num = -1;
  xyplot->selected_objects_line_type = -1;

  xyplot->nObjectsImage = 0;
  xyplot->objectsImage = NULL;
  xyplot->i_key_pressed = FALSE;
  xyplot->selected_objects_image_num = -1;

  xyplot->object_begin_point.x=-1;
  xyplot->object_begin_point.y=-1;
  xyplot->object_end_point.x=-1;
  xyplot->object_end_point.y=-1;

  xyplot->r_key_pressed = FALSE;
}
/****************************************************************************************/
GtkWidget* gabedit_xyplot_new ()
{
  GabeditXYPlot *xyplot;

  xyplot = g_object_new (GABEDIT_TYPE_XYPLOT, NULL);
  
  return GTK_WIDGET (xyplot);
}
/****************************************************************************************/
static void gabedit_xyplot_destroy (GtkObject *object)
{
  GabeditXYPlot *xyplot;

  g_return_if_fail (object != NULL);
  g_return_if_fail (G_IS_OBJECT (object));
  g_return_if_fail (GABEDIT_IS_XYPLOT (object));

  xyplot = GABEDIT_XYPLOT (object);

  if (xyplot->plotting_area && G_IS_OBJECT(xyplot->plotting_area))
  {
    g_object_unref(G_OBJECT(xyplot->plotting_area));
    xyplot->plotting_area = NULL;
  }
  if (xyplot->old_area && G_IS_OBJECT(xyplot->old_area))
  {
    g_object_unref(G_OBJECT(xyplot->old_area));
    xyplot->old_area = NULL;
  }
  if (xyplot->cairo_area)
  {
    cairo_destroy (xyplot->cairo_area);
    xyplot->cairo_area = NULL;
  }
  if (xyplot->cairo_widget)
  {
    cairo_destroy (xyplot->cairo_widget);
    xyplot->cairo_widget = NULL;
  }

  if (xyplot->back_gc && G_IS_OBJECT(xyplot->back_gc))
  {
    g_object_unref(xyplot->back_gc);
    xyplot->back_gc = NULL;
  }
  if (xyplot->fore_gc && G_IS_OBJECT(xyplot->fore_gc))
  {
    g_object_unref(xyplot->fore_gc);
    xyplot->fore_gc = NULL;
  }

  if (xyplot->data_gc &&  G_IS_OBJECT(xyplot->data_gc))
  {
    g_object_unref(xyplot->data_gc);
    xyplot->data_gc = NULL;
  }

  if (xyplot->lines_gc &&  G_IS_OBJECT(xyplot->lines_gc))
  {
    g_object_unref(xyplot->lines_gc);
    xyplot->lines_gc = NULL;
  }

  if (xyplot->hmajor_grid_gc && G_IS_OBJECT(xyplot->hmajor_grid_gc))
  {
    g_object_unref(xyplot->hmajor_grid_gc);
    xyplot->hmajor_grid_gc = NULL;
  }

  if (xyplot->hminor_grid_gc && G_IS_OBJECT(xyplot->hminor_grid_gc))
  {
    g_object_unref(xyplot->hminor_grid_gc);
    xyplot->hminor_grid_gc = NULL;
  }

  if (xyplot->vmajor_grid_gc && G_IS_OBJECT(xyplot->vmajor_grid_gc))
  {
    g_object_unref(xyplot->vmajor_grid_gc);
    xyplot->vmajor_grid_gc = NULL;
  }

  if (xyplot->vminor_grid_gc && G_IS_OBJECT(xyplot->vminor_grid_gc))
  {
    g_object_unref(xyplot->vminor_grid_gc);
    xyplot->vminor_grid_gc = NULL;
  }
  if (xyplot->objectsLine)
  {
	g_free(xyplot->objectsLine); 
    	xyplot->objectsLine = NULL;
  }
  if (xyplot->objectsText)
  {
	  gint i;
	  for(i=0;i<xyplot->nObjectsText;i++)
	  {
		if(xyplot->objectsText[i].str) g_free(xyplot->objectsText[i].str);
		if(xyplot->objectsText[i].pango) g_object_unref(G_OBJECT(xyplot->objectsText[i].pango));
	  }
	g_free(xyplot->objectsText); 
    	xyplot->objectsText = NULL;
  }

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
  
  gtk_object_destroy (object);
}
/****************************************************************************************/
static gint xyplot_get_font_size (GtkWidget* widget, PangoFontDescription* font_desc)
{
	gint font_size = 8;
	if(!font_desc) return font_size;
	font_size = pango_font_description_get_size (font_desc) / PANGO_SCALE;
	return font_size;
}
/********************************************************************************/
static gint gabedit_xyplot_grab(GtkWidget* widget, GdkEventCrossing* event)
{
	if(!event) return FALSE;
	switch(event->type)
	{
	      	case GDK_ENTER_NOTIFY: 
		      	gtk_grab_add(GTK_WIDGET (widget));
			break;
		case GDK_LEAVE_NOTIFY:
			{
				GabeditXYPlot *xyplot = NULL;
				if(widget && GABEDIT_IS_XYPLOT (widget))
					xyplot = GABEDIT_XYPLOT (widget);
		      		gtk_grab_remove(GTK_WIDGET (widget));
				if(xyplot)
				{
  					xyplot->shift_key_pressed = FALSE;
  					xyplot->control_key_pressed = FALSE;
				}
			}
			break;
		 default:
			break;
	}
	return TRUE;
}
/********************************************************************************/
static gint gabedit_xyplot_key_press(GtkWidget* widget, GdkEventKey *event)
{
	GabeditXYPlot *xyplot;
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GABEDIT_IS_XYPLOT (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	xyplot = GABEDIT_XYPLOT (widget);

	if((event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R) )
  		xyplot->shift_key_pressed = TRUE;
	if((event->keyval == GDK_Control_L || event->keyval == GDK_Control_R) )
  		xyplot->control_key_pressed = TRUE;
	if((event->keyval == GDK_Alt_L || event->keyval == GDK_Alt_L) )
  		xyplot->control_key_pressed = TRUE;

	if((event->keyval == GDK_c || event->keyval == GDK_C) )
	{
		if(xyplot->control_key_pressed) 
			copyImageToClipBoard(widget);

	}
	if((event->keyval == GDK_v || event->keyval == GDK_V) )
	{
		if(xyplot->control_key_pressed) 
		{
			add_object_image(xyplot, 0, 0, -1, -1, NULL);
    			gtk_widget_queue_draw(widget);
		}
	}

	if((event->keyval == GDK_t || event->keyval == GDK_T) )
  		xyplot->t_key_pressed = TRUE;
	if((event->keyval == GDK_l || event->keyval == GDK_L) )
  		xyplot->l_key_pressed = TRUE;
	if((event->keyval == GDK_i || event->keyval == GDK_I) )
  		xyplot->i_key_pressed = TRUE;
	if((event->keyval == GDK_r || event->keyval == GDK_R) )
  		xyplot->r_key_pressed = TRUE;
	return TRUE;
}
/********************************************************************************/
static gint gabedit_xyplot_key_release(GtkWidget* widget, GdkEventKey *event)
{
	GabeditXYPlot *xyplot;
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GABEDIT_IS_XYPLOT (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	xyplot = GABEDIT_XYPLOT (widget);

	if((event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R) )
  		xyplot->shift_key_pressed = FALSE;
	if((event->keyval == GDK_Control_L || event->keyval == GDK_Control_R) )
  		xyplot->control_key_pressed = FALSE;
	if((event->keyval == GDK_Alt_L || event->keyval == GDK_Alt_R) )
  		xyplot->control_key_pressed = FALSE;
	if((event->keyval == GDK_T || event->keyval == GDK_t) )
  		xyplot->t_key_pressed = FALSE;
	if((event->keyval == GDK_l || event->keyval == GDK_L) )
  		xyplot->l_key_pressed = FALSE;
	if((event->keyval == GDK_r || event->keyval == GDK_R) )
  		xyplot->r_key_pressed = FALSE;
	return TRUE;
}
/****************************************************************************************/
static void gabedit_xyplot_realize (GtkWidget *widget)
{
  GabeditXYPlot *xyplot;
  GdkWindowAttr attributes;
  gint attributes_mask;
  GdkGCValues gc_values;
  GdkGCValuesMask gc_values_mask;
  GdkColor black;
  GdkColor white;
  GdkColormap *colormap;
     
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  xyplot = GABEDIT_XYPLOT (widget);

  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = widget->allocation.width;
  attributes.height = widget->allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.event_mask = 
	(  gtk_widget_get_events (widget) |
	GDK_VISIBILITY_NOTIFY_MASK|
	GDK_EXPOSURE_MASK | 
	GDK_LEAVE_NOTIFY_MASK | GDK_ENTER_NOTIFY_MASK |
	GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK |
	GDK_FOCUS_CHANGE_MASK |
	GDK_LEAVE_NOTIFY_MASK |
	GDK_POINTER_MOTION_MASK | GDK_POINTER_MOTION_HINT_MASK | 
	GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);

  attributes.visual = gtk_widget_get_visual (widget);
  attributes.colormap = gtk_widget_get_colormap (widget);

  attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
  widget->window = gdk_window_new (widget->parent->window, &attributes, attributes_mask);

  widget->style = gtk_style_attach (widget->style, widget->window);

  gdk_window_set_user_data (widget->window, widget);

  gtk_style_set_background (widget->style, widget->window, GTK_STATE_ACTIVE);
       
  colormap=gdk_drawable_get_colormap(widget->window); 

  black.red = 0;
  black.green = 0;
  black.blue = 0;
  gdk_colormap_alloc_color (colormap, &black, FALSE, TRUE);

  white.red = 65535;
  white.green = 65535;
  white.blue = 65535;
  gdk_colormap_alloc_color (colormap, &white, FALSE, TRUE);


  gtk_widget_modify_bg (widget, GTK_STATE_NORMAL, &white);
  gtk_widget_modify_fg (widget, GTK_STATE_NORMAL, &black);
    
  gc_values.foreground=white;
  gc_values.line_style=GDK_LINE_SOLID;
  gc_values.line_width=1;
  gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
  xyplot->back_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);

  gc_values.foreground=black;
  gc_values.line_style=GDK_LINE_SOLID;
  gc_values.line_width=2;
  gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
  xyplot->fore_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
  
  gc_values.foreground=black;
  xyplot->hmajor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
  xyplot->vmajor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
  
  gc_values.foreground=black;
  gc_values.line_style=GDK_LINE_ON_OFF_DASH;
  gc_values.line_width=1;
  gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
  xyplot->hminor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
  xyplot->vminor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);

  xyplot->data_gc=gdk_gc_new (widget->window);

  gc_values.foreground=black;
  gc_values.line_style=GDK_LINE_SOLID;
  gc_values.line_width=2;
  gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
  xyplot->lines_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);

  xyplot->font_size =  xyplot_get_font_size (widget, widget->style->font_desc);

  /* Create the initial legends*/
  xyplot->v_legends_str = NULL;
  xyplot->h_legends_str = NULL;
  xyplot_build_legends(xyplot);
  xyplot_calculate_legends_sizes(xyplot);
  xyplot_calculate_sizes(xyplot);
  add_toolbar_and_popup_menu(widget, NULL);
}
/****************************************************************************************/
static void reset_theme(GtkWidget *widget, gint line_width, GdkColor* foreColor, GdkColor* backColor )
{
	GabeditXYPlot *xyplot;
	GdkGCValues gc_values;
	GdkGCValuesMask gc_values_mask;
	GdkColormap *colormap;
		 
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GABEDIT_IS_XYPLOT (widget));

	xyplot = GABEDIT_XYPLOT (widget);

	colormap=gdk_drawable_get_colormap(widget->window); 

	gdk_colormap_alloc_color (colormap, backColor, FALSE, TRUE);
	gdk_colormap_alloc_color (colormap, foreColor, FALSE, TRUE);

	gtk_widget_modify_bg (widget, GTK_STATE_NORMAL, backColor);
	gtk_widget_modify_fg (widget, GTK_STATE_NORMAL, foreColor);

	if (xyplot->back_gc && G_IS_OBJECT(xyplot->back_gc))
	{
		g_object_unref(xyplot->back_gc);
		xyplot->back_gc = NULL;
	}
	if (xyplot->fore_gc && G_IS_OBJECT(xyplot->fore_gc))
	{
		g_object_unref(xyplot->fore_gc);
		xyplot->fore_gc = NULL;
	}

	if (xyplot->data_gc &&	G_IS_OBJECT(xyplot->data_gc))
	{
		g_object_unref(xyplot->data_gc);
		xyplot->data_gc = NULL;
	}


	if (xyplot->hmajor_grid_gc && G_IS_OBJECT(xyplot->hmajor_grid_gc))
	{
		g_object_unref(xyplot->hmajor_grid_gc);
		xyplot->hmajor_grid_gc = NULL;
	}

	if (xyplot->hminor_grid_gc && G_IS_OBJECT(xyplot->hminor_grid_gc))
	{
		g_object_unref(xyplot->hminor_grid_gc);
		xyplot->hminor_grid_gc = NULL;
	}

	if (xyplot->vmajor_grid_gc && G_IS_OBJECT(xyplot->vmajor_grid_gc))
	{
		g_object_unref(xyplot->vmajor_grid_gc);
		xyplot->vmajor_grid_gc = NULL;
	}

	if (xyplot->vminor_grid_gc && G_IS_OBJECT(xyplot->vminor_grid_gc))
	{
		g_object_unref(xyplot->vminor_grid_gc);
		xyplot->vminor_grid_gc = NULL;
	}
		
	gc_values.foreground=*backColor;
	gc_values.line_style=GDK_LINE_SOLID;
	gc_values.line_width=line_width;
	gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
	xyplot->back_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);

	gc_values.foreground=*foreColor;
	gc_values.line_style=GDK_LINE_SOLID;
	gc_values.line_width=line_width;
	gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
	xyplot->fore_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
	
	gc_values.foreground=*foreColor;
	xyplot->hmajor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
	xyplot->vmajor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
	
	gc_values.foreground=*foreColor;
	gc_values.line_style=GDK_LINE_ON_OFF_DASH;
	if(line_width/2>0) gc_values.line_width=line_width/2;
	else gc_values.line_width=line_width;
	gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
	xyplot->hminor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
	xyplot->vminor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);

	xyplot->data_gc=gdk_gc_new (widget->window);

	if (xyplot->nObjectsLine<1)
	{
		if (xyplot->lines_gc &&	G_IS_OBJECT(xyplot->lines_gc))
		{
			g_object_unref(xyplot->lines_gc);
			xyplot->lines_gc = NULL;
		}
		gc_values.foreground=*foreColor;
		gc_values.line_style=GDK_LINE_SOLID;
		gc_values.line_width=line_width;
		gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
		xyplot->lines_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
	}

	xyplot_build_legends(xyplot);
	xyplot_calculate_legends_sizes(xyplot);
	xyplot_calculate_sizes(xyplot);
	reset_object_text_pango(xyplot);
}
/****************************************************************************************/
static void set_theme_publication(GtkWidget *widget)
{
	GdkColor black;
	GdkColor white;
	gint line_width = 2;
	black.red = 0;
	black.green = 0;
	black.blue = 0;

	white.red = 65535;
	white.green = 65535;
	white.blue = 65535;
	reset_theme(widget, line_width, &black, &white);
}
/****************************************************************************************/
static void set_theme_green_black(GtkWidget *widget)
{
	GdkColor black;
	GdkColor green;
	gint line_width = 2;
	black.red = 0;
	black.green = 0;
	black.blue = 0;

	green.red = 0;
	green.green = 65535;
	green.blue = 0;
	reset_theme(widget, line_width, &green, &black);
}
/****************************************************************************************/
static void theme_line_width_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* widget = GTK_WIDGET(user_data);
		GdkColor foreColor;
		GdkColor backColor;
		GdkGCValues gc_values;
		gint line_width = 1;
		GabeditXYPlot* xyplot = NULL;
		GdkColormap *colormap;

  		g_return_if_fail (GABEDIT_IS_XYPLOT (widget));
		xyplot = GABEDIT_XYPLOT(widget);
   		colormap  = gdk_window_get_colormap(widget->window);

		gdk_gc_get_values(xyplot->back_gc, &gc_values);
        	gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&backColor);
		gdk_gc_get_values(xyplot->fore_gc, &gc_values);
        	gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&foreColor);
		line_width=gc_values.line_width;
		line_width = gtk_spin_button_get_value(spinbutton);
		reset_theme(widget, line_width, &foreColor, &backColor);
	}
}
/****************************************************************************************/
static void theme_back_color_changed_value(GtkColorButton  *colorbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* widget = GTK_WIDGET(user_data);
		GdkColor foreColor;
		GdkColor backColor;
		GdkGCValues gc_values;
		gint line_width = 1;
		GabeditXYPlot* xyplot = NULL;
		GdkColormap *colormap;

  		g_return_if_fail (GABEDIT_IS_XYPLOT (widget));
		xyplot = GABEDIT_XYPLOT(widget);
   		colormap  = gdk_window_get_colormap(widget->window);
		gtk_color_button_get_color (colorbutton, &backColor);
		gdk_gc_get_values(xyplot->fore_gc, &gc_values);
        	gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&foreColor);
		line_width=gc_values.line_width;
		reset_theme(widget, line_width, &foreColor, &backColor);

	}
}
/********************************************************************************************************/
static void theme_fore_color_changed_value(GtkColorButton  *colorbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* widget = GTK_WIDGET(user_data);
		GdkColor foreColor;
		GdkColor backColor;
		GdkGCValues gc_values;
		gint line_width = 1;
		GabeditXYPlot* xyplot = NULL;
		GdkColormap *colormap;

  		g_return_if_fail (GABEDIT_IS_XYPLOT (widget));
		xyplot = GABEDIT_XYPLOT(widget);
   		colormap  = gdk_window_get_colormap(widget->window);
		gtk_color_button_get_color (colorbutton, &foreColor);
		gdk_gc_get_values(xyplot->back_gc, &gc_values);
        	gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&backColor);
		line_width=gc_values.line_width;
		reset_theme(widget, line_width, &foreColor, &backColor);
	}
}
/********************************************************************************************************/
static void set_theme_dialog(GtkWidget* widget)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* frame = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* hbox1 = NULL;
	GtkWidget* label = NULL;
	GtkWidget* spin = NULL;
	GtkWidget* button = NULL;
	GtkWidget* buttonBack = NULL;
	GtkWidget* buttonFore = NULL;
	GtkWidget* parentWindow = NULL;
	GtkWidget* vbox_window = NULL;
	GdkColor foreColor;
	GdkColor backColor;
	GdkGCValues gc_values;
	gint line_width = 1;
	GabeditXYPlot* xyplot = NULL;
	GdkColormap *colormap;

  	g_return_if_fail (GABEDIT_IS_XYPLOT (widget));
	xyplot = GABEDIT_XYPLOT(widget);
   	colormap  = gdk_window_get_colormap(widget->window);

	gdk_gc_get_values(xyplot->back_gc, &gc_values);
        gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&backColor);

	gdk_gc_get_values(xyplot->fore_gc, &gc_values);
        gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&foreColor);
	line_width=gc_values.line_width;

	gtk_window_set_title (GTK_WINDOW (window), _("Set theme"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)xyplot);

	vbox_window=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox_window);
	gtk_widget_show(vbox_window);

	hbox1=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_window), hbox1, TRUE, FALSE, 2);
	gtk_widget_show(hbox1);

	frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(hbox1), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), hbox);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Line width :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	spin = gtk_spin_button_new_with_range(0, 10, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), line_width);
	gtk_box_pack_start(GTK_BOX(hbox), spin, TRUE, FALSE, 2);
	gtk_widget_show(spin);
	g_object_set_data(G_OBJECT (window), "SpinLineWidth", spin);

	label=gtk_label_new(_("Background color :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	button = gtk_color_button_new_with_color (&backColor);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, FALSE, 2);
	gtk_widget_show(button);
	g_object_set_data(G_OBJECT (window), "BackColorButton", button);
	buttonBack = button;

	label=gtk_label_new(_("Foreground color :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	button = gtk_color_button_new_with_color (&foreColor);
	gtk_box_pack_start(GTK_BOX(hbox), button, TRUE, FALSE, 2);
	gtk_widget_show(button);
	g_object_set_data(G_OBJECT (window), "ForeColorButton", button);
	buttonFore = button;

	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(theme_line_width_changed_value), xyplot);
	g_signal_connect(G_OBJECT(buttonBack), "color-set", G_CALLBACK(theme_back_color_changed_value), xyplot);
	g_signal_connect(G_OBJECT(buttonFore), "color-set", G_CALLBACK(theme_fore_color_changed_value), xyplot);

	parentWindow = get_parent_window(GTK_WIDGET(xyplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
}
/****************************************************************************************/
static void gabedit_xyplot_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (widget));
  g_return_if_fail (requisition != NULL);

  requisition->width = XYPLOT_DEFAULT_SIZE;
  requisition->height = XYPLOT_DEFAULT_SIZE;
}
/****************************************************************************************/
static void gabedit_xyplot_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
  GabeditXYPlot *xyplot;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
  xyplot=GABEDIT_XYPLOT(widget);  

  if (GTK_WIDGET_REALIZED (widget))
    gdk_window_move_resize (widget->window,
			allocation->x, 
			allocation->y,
			allocation->width, 
			allocation->height);
    
  xyplot_calculate_sizes(xyplot);
}
/****************************************************************************************/
static void draw_points(GtkWidget *widget, GabeditXYPlot *xyplot, XYPlotData* data)
{
	gint i;
	gint x, y;
	GdkRectangle rect;

	if ( data->point_size<1 ) return;

	rect.x=0;
	rect.y=0;
	rect.width=widget->allocation.width;
	rect.height=widget->allocation.height;
	gdk_gc_set_rgb_fg_color (xyplot->data_gc, &data->point_color);

	for (i=0; i<data->size; i++)
		/*
        if ((data->x[i] < xyplot->xmax) && 
            (data->x[i] > xyplot->xmin) && 
            (data->y[i] < xyplot->ymax) && 
            (data->y[i] > xyplot->ymin))
	    */
	{
        	value2pixel(xyplot, data->x[i], data->y[i], &x, &y);
        	y=xyplot->plotting_rect.height-y;                   
		x -= data->point_width/2;
		y -= data->point_height/2;
		if(data->point_pango)
		xyplot_cairo_layout(xyplot, xyplot->cairo_area,  
                        x,
			y,
			data->point_pango,FALSE,FALSE,0);
	}
}
/****************************************************************************************/
static void draw_lines(GtkWidget *widget, GabeditXYPlot *xyplot, XYPlotData* data)
{
	GdkPoint* points;
	gint i;
	gboolean begin = TRUE;
	gint k,l;

	if (data->line_width<1) return;
	points=(GdkPoint*)g_malloc( (sizeof(GdkPoint) * 2));
	gdk_gc_set_rgb_fg_color (xyplot->data_gc, &data->line_color);
	gdk_gc_set_line_attributes (xyplot->data_gc, data->line_width, data->line_style, GDK_CAP_ROUND, GDK_JOIN_MITER);
                                             
	k = 0;
	l = 0;
	for (i=0; i<data->size; i++)
		/*
        if ((data->x[i] < xyplot->xmax) && 
            (data->x[i] > xyplot->xmin) && 
            (data->y[i] < xyplot->ymax) && 
            (data->y[i] > xyplot->ymin))
	    */
	{
		l = i;
		value2pixel(xyplot, data->x[i], data->y[i], (gint *)&points[1].x, (gint *)&points[1].y);
		points[1].y=xyplot->plotting_rect.height-points[1].y;                   
		if(begin)
		{
			points[0] = points[1];
			begin = FALSE;
			k = i;
			continue;
		}
		if(k+1 ==l) 
		{
			xyplot_cairo_lines(xyplot, xyplot->cairo_area,  widget, xyplot->data_gc, points, 2);
		}
		k = l;
		points[0] = points[1];
	}
	g_free(points);
}
/****************************************************************************************/
static void draw_zoom_rectangle(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	if (!xyplot->mouse_zoom_enabled || xyplot->mouse_button != xyplot->mouse_zoom_button) return;
	gdk_draw_rectangle(widget->window,
			xyplot->fore_gc,
			FALSE,
			xyplot->zoom_rect.x,
			xyplot->zoom_rect.y,
			xyplot->zoom_rect.width,
			xyplot->zoom_rect.height);
}
/****************************************************************************************/
static void draw_object_line_gdk(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint arrow_size = 0;
	gint i = -1;
	if (xyplot->object_begin_point.x<0 || xyplot->object_begin_point.y<0) return;
	if (xyplot->object_end_point.x<0 || xyplot->object_end_point.y<0) return;
	i = xyplot->selected_objects_line_num;
	if(i<0 && xyplot->nObjectsLine>0) i = xyplot->nObjectsLine-1;

	if(i>-1)
	{
		gdk_gc_set_rgb_fg_color (xyplot->lines_gc, &xyplot->objectsLine[i].color);
		gdk_gc_set_line_attributes (xyplot->lines_gc, 
			xyplot->objectsLine[i].width, 
			/* xyplot->objectsLine[i].style, */
			GDK_LINE_ON_OFF_DASH,
			GDK_CAP_ROUND, 
			GDK_JOIN_MITER);
		arrow_size = xyplot->objectsLine[i].arrow_size;
	}
	gdk_draw_line(widget->window, xyplot->lines_gc,
			xyplot->object_begin_point.x,
			xyplot->object_begin_point.y,
			xyplot->object_end_point.x,
			xyplot->object_end_point.y
		     );
	if(arrow_size>0)
	{
		gdouble x1, y1, x2, y2;
		calc_arrow_vertexes(30.0, arrow_size*5.0,
		(gdouble)xyplot->object_begin_point.x,
		(gdouble)xyplot->object_begin_point.y,
		(gdouble)xyplot->object_end_point.x,
		(gdouble)xyplot->object_end_point.y,
		&x1, &y1, 
		&x2, &y2
		);
		gdk_draw_line(widget->window, xyplot->lines_gc,
			(gint)x1,
			(gint)y1,
			xyplot->object_end_point.x,
			xyplot->object_end_point.y
		     );
		gdk_draw_line(widget->window, xyplot->lines_gc,
			(gint)x2,
			(gint)y2,
			xyplot->object_end_point.x,
			xyplot->object_end_point.y
		     );
	}
}
/****************************************************************************************/
static void get_rotated_rectangle(
		gint x, gint y, gint w, gint h, gdouble angle,
		gint* x1, gint* y1, 
		gint* x2, gint* y2, 
		gint* x3, gint* y3, 
		gint* x4, gint* y4) 
{
	gdouble a = -angle;
	gdouble cosa = cos(a); 
	gdouble sina = sin(a); 
	*x1 = x; *y1 = y; 
	*x2 = (gint)(x+w*cosa); *y2 = (gint)(y+w*sina); 
	*x3 = x+(gint)(w*cosa-h*sina); *y3 = (gint)(y+w*sina+h*cosa); 
	*x4 = (gint)(x-h*sina); *y4 = (gint)(y+h*cosa); 
}
/*****************************************************************************************************************/
static void draw_rotated_rectangle(GtkWidget *widget, GabeditXYPlot* xyplot,
		gint x, gint y, gint w, gint h, gdouble angle)
{
	static GdkPoint* points = NULL;
	if(!points) points = g_malloc(4*sizeof(GdkPoint));
	get_rotated_rectangle( x, y, w, h, angle,
		&points[0].x, &points[0].y,
		&points[1].x, &points[1].y,
		&points[2].x, &points[2].y,
		&points[3].x, &points[3].y);
	gdk_draw_polygon(widget->window, xyplot->fore_gc, FALSE, points, 4);
}
/****************************************************************************************/
static void draw_selected_objects_text_rectangle(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i = xyplot->selected_objects_text_num;
	gdouble angle = 0;
	if(i<0 || i>=xyplot->nObjectsText) return;
	if (xyplot->mouse_zoom_enabled && xyplot->mouse_button == xyplot->mouse_zoom_button) return;
	/*
	gdk_draw_rectangle(widget->window,
			xyplot->fore_gc,
			FALSE,
			xyplot->objectsText[i].xi,
			xyplot->objectsText[i].yi,
			xyplot->objectsText[i].width,
			xyplot->objectsText[i].height);
			*/
	angle = xyplot->objectsText[i].angle;
	draw_rotated_rectangle(widget, xyplot,
			xyplot->objectsText[i].xi,
			xyplot->objectsText[i].yi,
			xyplot->objectsText[i].width,
			xyplot->objectsText[i].height,
			angle);
}
/****************************************************************************************/
static void draw_selected_objects_image_rectangle(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i = xyplot->selected_objects_image_num;
	if(i<0 || i>=xyplot->nObjectsImage) return;
	if (xyplot->mouse_zoom_enabled && xyplot->mouse_button == xyplot->mouse_zoom_button) return;
	gdk_draw_rectangle(widget->window,
			xyplot->fore_gc,
			FALSE,
			xyplot->objectsImage[i].xi,
			xyplot->objectsImage[i].yi,
			xyplot->objectsImage[i].widthi,
			xyplot->objectsImage[i].heighti);
}
/****************************************************************************************/
static void draw_distance_line(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	PangoLayout *playout;
	GdkRectangle rect;
	gchar begin[100]=" ";
	gchar end[100] = " ";
	gdouble x1,y1,x2,y2;
	gint x,y;


	if (!xyplot->mouse_distance_enabled || xyplot->mouse_button != xyplot->mouse_distance_button) return;

	x=xyplot->distance_rect.x;
	y=xyplot->distance_rect.y;
	if(gabedit_xyplot_get_point(GABEDIT_XYPLOT(xyplot), x, y, &x1, &y1))
		sprintf(begin,"(%f ; %f)",x1,y1);
	x = xyplot->distance_rect.x+xyplot->distance_rect.width;
	y = xyplot->distance_rect.y+xyplot->distance_rect.height;
	if(gabedit_xyplot_get_point(GABEDIT_XYPLOT(xyplot), x, y, &x2, &y2))
		sprintf(end,"(%f ; %f)",x2,y2);
	rect.x=0; 
	rect.y=0; 
	rect.width=widget->allocation.width;
	rect.height=widget->allocation.height;
	playout=gtk_widget_create_pango_layout (widget, begin);
	if(!playout) return;
	gtk_paint_layout (widget->style, widget->window, 
			GTK_STATE_NORMAL, FALSE, 
			&rect, widget, NULL, 
			xyplot->distance_rect.x,
			xyplot->distance_rect.y,
			playout);
	if (playout) g_object_unref(G_OBJECT(playout));
	playout=gtk_widget_create_pango_layout (widget, end);
	gtk_paint_layout (widget->style, widget->window, 
			GTK_STATE_NORMAL, FALSE, 
			&rect, widget, NULL, 
			xyplot->distance_rect.x+xyplot->distance_rect.width,
			xyplot->distance_rect.y+xyplot->distance_rect.height,
			playout);
	if (playout) g_object_unref(G_OBJECT(playout));
	gdk_draw_line(widget->window,
			xyplot->fore_gc,
			xyplot->distance_rect.x,
			xyplot->distance_rect.y,
			xyplot->distance_rect.x+xyplot->distance_rect.width,
			xyplot->distance_rect.y+xyplot->distance_rect.height);

}
/****************************************************************************************/
static void draw_hminor_ticks(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i;
	gint j;
	if (xyplot->hmajor_ticks<2 || xyplot->hminor_ticks == 0 ) return;
	for (i=0; i < xyplot->hmajor_ticks-1; i++)
	for (j=1; j <= xyplot->hminor_ticks; j++)
	{
		if (xyplot->show_bottom_legends)
        	xyplot_cairo_line(xyplot, xyplot->cairo_widget, widget, 
			xyplot->fore_gc,
			xyplot->plotting_rect.x+i*xyplot->d_hminor*(xyplot->hminor_ticks+1)+j*xyplot->d_hminor, 
			xyplot->plotting_rect.y+xyplot->plotting_rect.height,
			xyplot->plotting_rect.x+i*xyplot->d_hminor*(xyplot->hminor_ticks+1)+j*xyplot->d_hminor,
			xyplot->plotting_rect.y+xyplot->plotting_rect.height+xyplot->length_ticks);
      		if (xyplot->show_top_legends)
        	xyplot_cairo_line(xyplot, xyplot->cairo_widget, widget, 
			xyplot->fore_gc,
			xyplot->plotting_rect.x+i*xyplot->d_hminor*(xyplot->hminor_ticks+1)+j*xyplot->d_hminor, 
			xyplot->plotting_rect.y,
			xyplot->plotting_rect.x+i*xyplot->d_hminor*(xyplot->hminor_ticks+1)+j*xyplot->d_hminor,
			xyplot->plotting_rect.y-xyplot->length_ticks);
      }
}
/****************************************************************************************/
static void draw_hmajor_ticks_and_xlegends(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i;
	GdkRectangle rect;

	rect.x=0;
	rect.y=0;
	rect.width=widget->allocation.width;
	rect.height=widget->allocation.height;
  
	if (xyplot->hmajor_ticks>1)
	for (i=0; i < xyplot->hmajor_ticks; i++)
	{
		gint l1 = i;
		if(xyplot->reflect_x) l1 = xyplot->hmajor_ticks-i-1;
		if (xyplot->show_bottom_legends)
        	xyplot_cairo_line(xyplot, xyplot->cairo_widget, widget, 
			xyplot->fore_gc,
			xyplot->plotting_rect.x+i*xyplot->d_hmajor, 
			xyplot->plotting_rect.y+xyplot->plotting_rect.height,
			xyplot->plotting_rect.x+i*xyplot->d_hmajor,
			xyplot->plotting_rect.y+xyplot->plotting_rect.height+xyplot->length_ticks*2);
		if (xyplot->show_top_legends)
        	xyplot_cairo_line(xyplot, xyplot->cairo_widget, widget, 
			xyplot->fore_gc,
			xyplot->plotting_rect.x+i*xyplot->d_hmajor, 
			xyplot->plotting_rect.y,
			xyplot->plotting_rect.x+i*xyplot->d_hmajor,
			xyplot->plotting_rect.y-xyplot->length_ticks*2);

		if (xyplot->x_legends_digits!=0 && xyplot->show_bottom_legends)
		xyplot_cairo_string(xyplot, xyplot->cairo_widget, widget, xyplot->fore_gc,
			xyplot->plotting_rect.x+i*xyplot->d_hmajor-0.5*xyplot->x_legends_width,
			xyplot->plotting_rect.y+xyplot->plotting_rect.height+xyplot->length_ticks*2,
				xyplot->h_legends_str[l1], FALSE, FALSE,0);
		if (xyplot->x_legends_digits!=0 && xyplot->show_top_legends)
		xyplot_cairo_string(xyplot, xyplot->cairo_widget, widget, xyplot->fore_gc,
                        xyplot->plotting_rect.x+i*xyplot->d_hmajor-0.5*xyplot->x_legends_width,
			xyplot->plotting_rect.y-xyplot->length_ticks*2-xyplot->x_legends_height,
				xyplot->h_legends_str[l1], FALSE, FALSE,0);
	}
	if ( xyplot->h_label && xyplot->x_legends_digits!=0 && xyplot->show_bottom_legends)
	{
		xyplot_cairo_layout(xyplot, xyplot->cairo_widget,  
                        xyplot->plotting_rect.x+0.5*xyplot->plotting_rect.width,
			xyplot->plotting_rect.y+xyplot->plotting_rect.height+xyplot->length_ticks*2+xyplot->x_legends_height,
			xyplot->h_label,TRUE,FALSE,0);
	}
	if (xyplot->h_label  && xyplot->x_legends_digits!=0 && xyplot->show_top_legends && !xyplot->show_bottom_legends)
	{
		xyplot_cairo_layout(xyplot, xyplot->cairo_widget,  
                        xyplot->plotting_rect.x+0.5*xyplot->plotting_rect.width,
			xyplot->plotting_rect.y-xyplot->x_legends_height-xyplot->length_ticks*2-xyplot->h_label_height,
			xyplot->h_label,TRUE,FALSE,0);
	}
}

/****************************************************************************************/
static void xyplot_cairo_line_grid(GabeditXYPlot *xyplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gdouble x1,gdouble y1,gdouble x2,gdouble y2)
{
	gabedit_xyplot_cairo_line(cr,  widget, gc, x1, y1, x2, y2);
	if(xyplot->cairo_export)
	{
		x1+= xyplot->plotting_rect.x;
		x2+= xyplot->plotting_rect.x;
		y1+= xyplot->plotting_rect.y;
		y2+= xyplot->plotting_rect.y;
		gabedit_xyplot_cairo_line(xyplot->cairo_export,  widget, gc,  x1, y1, x2, y2);
	}
}
/****************************************************************************************/
static void draw_hminor_grid(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i;
	gint j;
	if (xyplot->hminor_grid && (xyplot->vmajor_ticks > 1) && (xyplot->vminor_ticks != 0))
	for (i=0; i < xyplot->vmajor_ticks-1; i++)
	for (j=1; j <= xyplot->vminor_ticks; j++)
	{
        	xyplot_cairo_line_grid(xyplot, xyplot->cairo_area, widget, 
			xyplot->hminor_grid_gc,
			0, 
			i*xyplot->d_vmajor+j*xyplot->d_vminor,
			xyplot->plotting_rect.width,
			i*xyplot->d_vmajor+j*xyplot->d_vminor);
      }
}
/****************************************************************************************/
static void draw_hmajor_grid(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i;
 
	if (xyplot->hmajor_grid && (xyplot->vmajor_ticks > 1) )
	for (i=1; i < (xyplot->vmajor_ticks-1); i++)
        	xyplot_cairo_line_grid(xyplot, xyplot->cairo_area, widget, 
			xyplot->hmajor_grid_gc,
			0, 
			i*xyplot->d_vmajor,
			xyplot->plotting_rect.width,
			i*xyplot->d_vmajor);
}
/****************************************************************************************/
static void draw_vminor_ticks(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i;
	gint j;

	if ( (xyplot->vmajor_ticks > 1) && (xyplot->vminor_ticks != 0) )
	for (i=0; i < xyplot->vmajor_ticks-1; i++)
	for (j=1; j <= xyplot->vminor_ticks; j++)
	{
		if (xyplot->show_left_legends)
        	xyplot_cairo_line(xyplot, xyplot->cairo_widget, widget, 
			xyplot->fore_gc,
			xyplot->plotting_rect.x, 
			xyplot->plotting_rect.y+i*xyplot->d_vminor*(xyplot->vminor_ticks+1)+j*xyplot->d_vminor,
			xyplot->plotting_rect.x-xyplot->length_ticks,
			xyplot->plotting_rect.y+i*xyplot->d_vminor*(xyplot->vminor_ticks+1)+j*xyplot->d_vminor);       
      		if (xyplot->show_right_legends)
        	xyplot_cairo_line(xyplot, xyplot->cairo_widget, widget, 
			xyplot->fore_gc,
			xyplot->plotting_rect.x+xyplot->plotting_rect.width, 
			xyplot->plotting_rect.y+i*xyplot->d_vminor*(xyplot->vminor_ticks+1)+j*xyplot->d_vminor,
			xyplot->plotting_rect.x+xyplot->plotting_rect.width+xyplot->length_ticks,
			xyplot->plotting_rect.y+i*xyplot->d_vminor*(xyplot->vminor_ticks+1)+j*xyplot->d_vminor);       
	}
}
/****************************************************************************************/
static void draw_vmajor_ticks_and_ylegends(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i;
	GdkRectangle rect;

	rect.x=0;
	rect.y=0;
	rect.width=widget->allocation.width;
	rect.height=widget->allocation.height;

	if (xyplot->vmajor_ticks>1)
	for (i=0; i < xyplot->vmajor_ticks; i++)
	{
		gint l1 = i;
		if(xyplot->reflect_y) l1 = xyplot->vmajor_ticks-i-1;
		if (xyplot->show_left_legends)
        	xyplot_cairo_line(xyplot, xyplot->cairo_widget, widget, 
			xyplot->fore_gc,
			xyplot->plotting_rect.x, 
			xyplot->plotting_rect.y+i*xyplot->d_vmajor,
			xyplot->plotting_rect.x-xyplot->length_ticks*2,
			xyplot->plotting_rect.y+i*xyplot->d_vmajor);
		if (xyplot->show_right_legends)
        	xyplot_cairo_line(xyplot, xyplot->cairo_widget, widget, 
			xyplot->fore_gc,
			xyplot->plotting_rect.x+xyplot->plotting_rect.width, 
			xyplot->plotting_rect.y+i*xyplot->d_vmajor,
			xyplot->plotting_rect.x+xyplot->plotting_rect.width+xyplot->length_ticks*2,
			xyplot->plotting_rect.y+i*xyplot->d_vmajor);
    
		if (xyplot->y_legends_digits!=0 && xyplot->show_left_legends)
		xyplot_cairo_string(xyplot, xyplot->cairo_widget, widget, xyplot->fore_gc,
                        xyplot->plotting_rect.x-xyplot->y_legends_width-xyplot->length_ticks*2-2,
			xyplot->plotting_rect.y+xyplot->plotting_rect.height-i*xyplot->d_vmajor,
				xyplot->v_legends_str[l1], FALSE, TRUE,0);
		if (xyplot->y_legends_digits!=0 && xyplot->show_right_legends)
		xyplot_cairo_string(xyplot, xyplot->cairo_widget, widget, xyplot->fore_gc,
                        xyplot->plotting_rect.x+xyplot->plotting_rect.width+xyplot->length_ticks*2+2,
			xyplot->plotting_rect.y+xyplot->plotting_rect.height-i*xyplot->d_vmajor,
				xyplot->v_legends_str[l1], FALSE, TRUE,0);
	}
	if (xyplot->v_label && xyplot->y_legends_digits!=0 && xyplot->show_left_legends)
	{
		xyplot_cairo_layout(xyplot, xyplot->cairo_widget,  
                        xyplot->plotting_rect.x-xyplot->y_legends_width-xyplot->length_ticks*2-xyplot->v_label_height,
			xyplot->plotting_rect.y+0.5*xyplot->plotting_rect.height,
			xyplot->v_label,FALSE,TRUE,-M_PI/2);
	}
	if (xyplot->v_label && xyplot->y_legends_digits!=0 && xyplot->show_right_legends && !xyplot->show_left_legends)
	{
		xyplot_cairo_layout(xyplot, xyplot->cairo_widget,  
                        xyplot->plotting_rect.x+xyplot->plotting_rect.width+xyplot->y_legends_width+xyplot->length_ticks*2,
			xyplot->plotting_rect.y+0.5*xyplot->plotting_rect.height,
			xyplot->v_label,FALSE,TRUE,-M_PI/2);
	}
}
/****************************************************************************************/
static void draw_vminor_grid(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i;
	gint j;
	if (xyplot->vminor_grid && (xyplot->hmajor_ticks > 1) && (xyplot->hminor_ticks != 0) )
	for (i=0; i < xyplot->hmajor_ticks-1; i++)
	for (j=1; j <= xyplot->hminor_ticks; j++)
        xyplot_cairo_line_grid(xyplot, xyplot->cairo_area, widget, 
			xyplot->vminor_grid_gc,
			i*xyplot->d_hmajor+j*xyplot->d_hminor, 
			0,
			i*xyplot->d_hmajor+j*xyplot->d_hminor,
			xyplot->plotting_rect.height);
}
/****************************************************************************************/
static void draw_vmajor_grid(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i;
	if (xyplot->vmajor_grid && (xyplot->hmajor_ticks > 1))
	for (i=1; i < (xyplot->hmajor_ticks-1); i++)  
        xyplot_cairo_line_grid(xyplot, xyplot->cairo_area, widget, 
			xyplot->vmajor_grid_gc,
			i*xyplot->d_hmajor, 
			0,
			i*xyplot->d_hmajor,
			xyplot->plotting_rect.height);
}
/****************************************************************************************/
static void draw_objects_text(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i;
	for (i=0; i < xyplot->nObjectsText; i++)  
	if(xyplot->objectsText[i].pango)
	{
		/*
    		if ( 
		!(
		(xyplot->objectsText[i].xi > xyplot->plotting_rect.x) && 
         	(xyplot->objectsText[i].xi < (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) && 
         	(xyplot->objectsText[i].yi > xyplot->plotting_rect.y) && 
         	(xyplot->objectsText[i].yi < (xyplot->plotting_rect.y + xyplot->plotting_rect.height)) 
		)
		) continue;
		*/
		xyplot_cairo_layout(xyplot, xyplot->cairo_widget,  
                        xyplot->objectsText[i].xi,
                        xyplot->objectsText[i].yi,
			xyplot->objectsText[i].pango,FALSE,FALSE,-xyplot->objectsText[i].angle);
	}
}
/****************************************************************************************/
static void draw_objects_line(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i;
	for (i=0; i < xyplot->nObjectsLine; i++)  
	{
		/*
    		if ( 
		!(
		(xyplot->objectsLine[i].x1i > xyplot->plotting_rect.x) && 
         	(xyplot->objectsLine[i].x1i < (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) && 
         	(xyplot->objectsLine[i].y1i > xyplot->plotting_rect.y) && 
         	(xyplot->objectsLine[i].y1i < (xyplot->plotting_rect.y + xyplot->plotting_rect.height)) 
		)
		) continue;
    		if ( 
		!(
		(xyplot->objectsLine[i].x2i > xyplot->plotting_rect.x) && 
         	(xyplot->objectsLine[i].x2i < (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) && 
         	(xyplot->objectsLine[i].y2i > xyplot->plotting_rect.y) && 
         	(xyplot->objectsLine[i].y2i < (xyplot->plotting_rect.y + xyplot->plotting_rect.height)) 
		)
		) continue;
		*/
		/* HERE change gc vlaues */
		gdouble x1, x2, y1, y2;
		gdk_gc_set_rgb_fg_color (xyplot->lines_gc, &xyplot->objectsLine[i].color);
		gdk_gc_set_line_attributes (xyplot->lines_gc, 
			xyplot->objectsLine[i].width, 
			xyplot->objectsLine[i].style, 
			GDK_CAP_ROUND, 
			GDK_JOIN_MITER);

        	xyplot_cairo_line(xyplot, xyplot->cairo_widget, widget, 
			xyplot->lines_gc,
			xyplot->objectsLine[i].x1i,
			xyplot->objectsLine[i].y1i,
			xyplot->objectsLine[i].x2i,
			xyplot->objectsLine[i].y2i);
		if(xyplot->objectsLine[i].arrow_size<1) continue;
		calc_arrow_vertexes(30.0, xyplot->objectsLine[i].arrow_size*5.0,
		(gdouble)xyplot->objectsLine[i].x1i, 
		(gdouble)xyplot->objectsLine[i].y1i, 
		(gdouble)xyplot->objectsLine[i].x2i, 
		(gdouble)xyplot->objectsLine[i].y2i, 
		&x1, &y1, 
		&x2, &y2
		);
        	xyplot_cairo_line(xyplot, xyplot->cairo_widget, widget, 
			xyplot->lines_gc,
			(gint)x1,
			(gint)y1,
			xyplot->objectsLine[i].x2i,
			xyplot->objectsLine[i].y2i);
        	xyplot_cairo_line(xyplot, xyplot->cairo_widget, widget, 
			xyplot->lines_gc,
			(gint)x2,
			(gint)y2,
			xyplot->objectsLine[i].x2i,
			xyplot->objectsLine[i].y2i);
	}
}
/****************************************************************************************/
static void draw_objects_image(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gint i;
	for (i=0; i < xyplot->nObjectsImage; i++)  
	if(xyplot->objectsImage[i].image)
	{
		/*
    		if ( 
		!(
		(xyplot->objectsImage[i].xi > xyplot->plotting_rect.x) && 
         	(xyplot->objectsImage[i].xi < (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) && 
         	(xyplot->objectsImage[i].yi > xyplot->plotting_rect.y) && 
         	(xyplot->objectsImage[i].yi < (xyplot->plotting_rect.y + xyplot->plotting_rect.height)) 
		)
		) continue;
		*/
		xyplot_cairo_image(xyplot, xyplot->cairo_widget, widget, 
				xyplot->objectsImage[i].xi , xyplot->objectsImage[i].yi, 
				xyplot->objectsImage[i].widthi, xyplot->objectsImage[i].heighti, 
				xyplot->objectsImage[i].image);
	}
}
/****************************************************************************************/
static void clean_borders(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	if(xyplot->cairo_export)
	{
		xyplot_cairo_rectangle(xyplot, xyplot->cairo_export,  widget, xyplot->back_gc, 
			TRUE,
			0,
			0,
			xyplot->plotting_rect.x,
			widget->allocation.height);
		xyplot_cairo_rectangle(xyplot, xyplot->cairo_export,  widget, xyplot->back_gc, 
			TRUE,
			0,
			0,
			widget->allocation.width,
			xyplot->plotting_rect.y
			);
		xyplot_cairo_rectangle(xyplot, xyplot->cairo_export,  widget, xyplot->back_gc, 
			TRUE,
			0,
			xyplot->plotting_rect.y+xyplot->plotting_rect.height,
			widget->allocation.width,
			widget->allocation.height-xyplot->plotting_rect.y-xyplot->plotting_rect.height
			);
		xyplot_cairo_rectangle(xyplot, xyplot->cairo_export,  widget, xyplot->back_gc, 
			TRUE,
			xyplot->plotting_rect.x+xyplot->plotting_rect.width,
			0,
			widget->allocation.width-xyplot->plotting_rect.x-xyplot->plotting_rect.width,
			widget->allocation.height
			);
	}
}
/****************************************************************************************/
static void draw_borders(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gboolean rectangle = xyplot->show_rectangle_legends;
	if(rectangle)
	xyplot_cairo_rectangle(
			xyplot, xyplot->cairo_widget,  widget,
	                xyplot->fore_gc,
			FALSE,
			xyplot->plotting_rect.x,
			xyplot->plotting_rect.y,
			xyplot->plotting_rect.width,
			xyplot->plotting_rect.height);
	else
	{
  		if(xyplot->show_left_legends)
		{
			gdouble x1 = xyplot->plotting_rect.x;
			gdouble y1 = xyplot->plotting_rect.y;
			gdouble x2 = xyplot->plotting_rect.x;
			gdouble y2 = xyplot->plotting_rect.y+xyplot->plotting_rect.height;
			xyplot_cairo_line( xyplot, xyplot->cairo_widget,  widget, xyplot->fore_gc, x1, y1, x2, y2);
		}
		if(xyplot->show_right_legends)
		{
			gdouble x1 = xyplot->plotting_rect.x+xyplot->plotting_rect.width;
			gdouble y1 = xyplot->plotting_rect.y;
			gdouble x2 = xyplot->plotting_rect.x+xyplot->plotting_rect.width;
			gdouble y2 = xyplot->plotting_rect.y+xyplot->plotting_rect.height;
			xyplot_cairo_line( xyplot, xyplot->cairo_widget,  widget, xyplot->fore_gc, x1, y1, x2, y2);
		}
		if(xyplot->show_top_legends)
		{
			gdouble x1 = xyplot->plotting_rect.x;
			gdouble y1 = xyplot->plotting_rect.y;
			gdouble x2 = xyplot->plotting_rect.x+xyplot->plotting_rect.width;
			gdouble y2 = xyplot->plotting_rect.y;
			xyplot_cairo_line( xyplot, xyplot->cairo_widget,  widget, xyplot->fore_gc, x1, y1, x2, y2);
		}
		if(xyplot->show_bottom_legends)
		{
			gdouble x1 = xyplot->plotting_rect.x;
			gdouble y1 = xyplot->plotting_rect.y+xyplot->plotting_rect.height;
			gdouble x2 = xyplot->plotting_rect.x+xyplot->plotting_rect.width;
			gdouble y2 = xyplot->plotting_rect.y+xyplot->plotting_rect.height;
			xyplot_cairo_line( xyplot, xyplot->cairo_widget,  widget, xyplot->fore_gc, x1, y1, x2, y2);
		}

	}
}
/****************************************************************************************/
static void draw_background(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	GdkRectangle rect;

	rect.x=0;
	rect.y=0;
	rect.width=widget->allocation.width;
	rect.height=widget->allocation.height;

	gtk_paint_flat_box (widget->style,
			widget->window,
			GTK_STATE_NORMAL,
			GTK_SHADOW_NONE,
			&rect,
			widget,
			NULL, 
			0, 
			0, 
			-1,
			-1);
  
	/* Filling the plotting area*/
	xyplot_cairo_rectangle(xyplot, xyplot->cairo_area,  widget, xyplot->back_gc, 
			TRUE,
			0,
			0,
			xyplot->plotting_rect.width,
			xyplot->plotting_rect.height);
	if(xyplot->cairo_export)
	{
	xyplot_cairo_rectangle(xyplot, xyplot->cairo_export,  widget, xyplot->back_gc, 
			TRUE,
			xyplot->plotting_rect.x,
			xyplot->plotting_rect.y,
			xyplot->plotting_rect.width,
			xyplot->plotting_rect.height);
	}

}
/****************************************************************************************/
static void draw_data(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	XYPlotData *data = NULL;
	GList *current = NULL;

	if(xyplot->data_list)
	for(current=g_list_first(xyplot->data_list); current!=NULL; current=current->next)
	{
		data=(XYPlotData*)current->data;
		draw_lines(widget, xyplot, data);
		draw_points(widget, xyplot, data);
  	}
  
}
/****************************************************************************************/
static GdkPixmap* gdk_pixmap_copy(GdkPixmap *pixmap)
{
	GdkPixmap *pixmap_out;
	GdkGC *gc;
	gint width, height, depth;

	g_return_val_if_fail(pixmap != NULL, NULL);

	gdk_drawable_get_size(pixmap, &width, &height);
	depth = gdk_drawable_get_depth(pixmap);
	pixmap_out = gdk_pixmap_new(NULL, width, height, depth);
	gc = gdk_gc_new(pixmap);
	gdk_draw_drawable(pixmap_out, gc, pixmap, 0, 0, 0, 0, width, height);
	g_object_unref(gc);
	return pixmap_out;
}
/****************************************************************************************/
static void set_old_area(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	if (xyplot->old_area!=NULL) g_object_unref(G_OBJECT(xyplot->old_area));
	xyplot->old_area=NULL;

	if (GTK_WIDGET_REALIZED(widget) &&  widget->window) 
	{
		xyplot->old_area=gdk_pixmap_copy(widget->window);
	}
}
/****************************************************************************************/
static void draw_plotting_area(GtkWidget *widget, GabeditXYPlot *xyplot)
{
	gdk_draw_drawable (widget->window, 
			xyplot->back_gc, 
			xyplot->plotting_area, 
			0, 
			0, 
			xyplot->plotting_rect.x, 
			xyplot->plotting_rect.y, 
			xyplot->plotting_rect.width, 
			xyplot->plotting_rect.height);

}
/****************************************************************************************/
static gint gabedit_xyplot_expose (GtkWidget *widget, GdkEventExpose *event)
{
	GabeditXYPlot *xyplot;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GABEDIT_IS_XYPLOT (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	if (event->count > 0) return FALSE;
	xyplot=GABEDIT_XYPLOT(widget);

	if (xyplot->mouse_zoom_enabled && xyplot->mouse_button == xyplot->mouse_zoom_button)
	{
		gint width;
		gint height;
		gdk_drawable_get_size(xyplot->old_area, &width, &height);
		gdk_draw_drawable (widget->window, 
		xyplot->back_gc, 
		xyplot->old_area, 
		0, 
		0, 
		0, 
		0, 
		width, 
		height
		);
		draw_zoom_rectangle(widget, xyplot);
		return TRUE;
	}
	if (xyplot->mouse_distance_enabled && xyplot->mouse_button == xyplot->mouse_distance_button)
	{
		gint width;
		gint height;
		gdk_drawable_get_size(xyplot->old_area, &width, &height);
		gdk_draw_drawable (widget->window, 
		xyplot->back_gc, 
		xyplot->old_area, 
		0, 
		0, 
		0, 
		0, 
		width, 
		height
		);
		draw_distance_line(widget, xyplot);
		return TRUE;
	}
	if (xyplot->selected_objects_text_num>-1)
	{
		gint width;
		gint height;
		gdk_drawable_get_size(xyplot->old_area, &width, &height);
		gdk_draw_drawable (widget->window, 
		xyplot->back_gc, 
		xyplot->old_area, 
		0, 
		0, 
		0, 
		0, 
		width, 
		height
		);
		draw_selected_objects_text_rectangle(widget, xyplot);
		return TRUE;
	}
	if (xyplot->selected_objects_image_num>-1)
	{
		gint width;
		gint height;
		gdk_drawable_get_size(xyplot->old_area, &width, &height);
		gdk_draw_drawable (widget->window, 
		xyplot->back_gc, 
		xyplot->old_area, 
		0, 
		0, 
		0, 
		0, 
		width, 
		height
		);
		draw_selected_objects_image_rectangle(widget, xyplot);
		return TRUE;
	}
	if (xyplot->object_begin_point.x>-1)
	{
		gint width;
		gint height;
		gdk_drawable_get_size(xyplot->old_area, &width, &height);
		gdk_draw_drawable (widget->window, 
		xyplot->back_gc, 
		xyplot->old_area, 
		0, 
		0, 
		0, 
		0, 
		width, 
		height
		);
		draw_object_line_gdk(widget, xyplot);
		return TRUE;
	}
    
	if (xyplot->cairo_widget)
	{
    		cairo_destroy (xyplot->cairo_widget);
    		xyplot->cairo_widget = gdk_cairo_create (widget->window);
	}
	if (xyplot->cairo_area)
	{
    		cairo_destroy (xyplot->cairo_area);
    		xyplot->cairo_area = gdk_cairo_create (xyplot->plotting_area);
	}

	draw_background(widget, xyplot);

	draw_vmajor_grid(widget, xyplot);
	draw_hminor_grid(widget, xyplot);
	draw_hmajor_grid(widget, xyplot);
	draw_vminor_grid(widget, xyplot);

	draw_data(widget, xyplot);

	clean_borders(widget, xyplot);/* for export */
	draw_borders(widget, xyplot);


	draw_hminor_ticks(widget, xyplot);
	draw_hmajor_ticks_and_xlegends(widget, xyplot);
	draw_vminor_ticks(widget, xyplot);
	draw_vmajor_ticks_and_ylegends(widget, xyplot);

	draw_plotting_area(widget, xyplot);

 
	/* draw_zoom_rectangle(widget, xyplot);*/
	/* draw_distance_line(widget, xyplot);*/
        
	draw_objects_image(widget, xyplot);
	draw_objects_line(widget, xyplot);
	draw_objects_text(widget, xyplot);

	return 0;
}
/****************************************************************************************/
static gint get_distance_M_AB(GabeditXYPlot *xyplot,gint xM, gint yM, gint ixA, gint iyA, gint ixB, gint iyB)
{
	gdouble xA = ixA;
	gdouble yA = iyA;
	gdouble xB = ixB;
	gdouble yB = iyB;
	gdouble xAB = xB -xA;
	gdouble yAB = yB -yA;
	gdouble AB2 = xAB*xAB + yAB*yAB;
	gdouble xMA;
	gdouble yMA;
	gdouble MA2;
	gdouble xMB;
	gdouble yMB;
	gdouble MB2;
	gdouble xC;
	gdouble yC;
	gdouble xMC;
	gdouble yMC;
	gdouble MC2;

	do{
		xMA = xA -xM;
		yMA = yA -yM;
		MA2 = xMA*xMA + yMA*yMA;
		xMB = xB -xM;
		yMB = yB -yM;
		MB2 = xMB*xMB + yMB*yMB;
		xC = (xA + xB)/2.0;
		yC = (yA + yB)/2.0;
		xMC = xC - xM;
		yMC = yC - yM;
		MC2 = xMC*xMC + yMC*yMC;
		if(MA2>MB2)
		{
			if(MC2<MA2)
			{
				xA = xC;
				yA = yC;
			}
			else
			{
				xB = xC;
				yB = yC;
			}
		}
		else
		{
			if(MC2<MB2)
			{
				xB = xC;
				yB = yC;
			}
			else
			{
				xA = xC;
				yA = yC;
			}
		}
		xAB = xB -xA;
		yAB = yB -yA;
		AB2 = xAB*xAB + yAB*yAB;
	}while(AB2>1);
	return (gint)sqrt(MC2);
}
/****************************************************************************************/
static gint gabedit_xyplot_double_click (GtkWidget *widget, GdkEventButton *event)
{
  	XYPlotData *data; 
	GList* current = NULL;
	gint x;
	gint y;
	gboolean OK = FALSE;
	GabeditXYPlot *xyplot = NULL;
	gint loop;
	gint xOld;
	gint yOld;
	gint i = 0;

  	xyplot=GABEDIT_XYPLOT(widget);
	/* test on objects label */
	i = get_object_text_num(xyplot, event->x, event->y);
	if(i>-1 && i<xyplot->nObjectsText)
	{
		xyplot->selected_objects_text_num = -1;
		add_set_object_text_dialog(GTK_WIDGET(xyplot), i, 0,0);
		return TRUE;
	}
	/* test on objects line */
	i = get_object_line_num(xyplot, event->x, event->y);
	if(i>-1 && i<xyplot->nObjectsLine)
	{
		xyplot->selected_objects_line_num = -1;
		xyplot->selected_objects_line_type = -1;
		xyplot->object_begin_point.x = -1;
		xyplot->object_begin_point.y = -1;
		xyplot->object_end_point.x = -1;
		xyplot->object_end_point.y = -1;
		set_object_line_dialog(xyplot, i);
		return TRUE;
	}
	/* test on objects image */
	i = get_object_image_num(xyplot, event->x, event->y);
	if(i>-1 && i<xyplot->nObjectsImage)
	{
		xyplot->selected_objects_image_num = -1;
		xyplot->object_begin_point.x = -1;
		xyplot->object_begin_point.y = -1;
		xyplot->object_end_point.x = -1;
		xyplot->object_end_point.y = -1;
		add_set_object_image_dialog(GTK_WIDGET(xyplot), i, 0,0);
		return TRUE;
	}
    	if ( 
		!(
		(event->x > xyplot->plotting_rect.x) && 
         	(event->x < (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) && 
         	(event->y > xyplot->plotting_rect.y) && 
         	(event->y < (xyplot->plotting_rect.y + xyplot->plotting_rect.height)) 
	)
	) 
	{
		set_all_dialog(widget);
		return TRUE;
	}

	if(!(xyplot->data_list)) return TRUE;
	current=g_list_first(xyplot->data_list);
	for(; current != NULL; current = current->next)
	{
		data = (XYPlotData*)current->data;
		xOld = 0;
		yOld = 0;
		for(loop=0;loop<data->size;loop++)
		{
    			value2pixel(xyplot, data->x[loop], data->y[loop], &x, &y);
			y=xyplot->plotting_rect.height-y;
			x += xyplot->plotting_rect.x;
			y += xyplot->plotting_rect.y;
			if(loop ==0 && abs(x-event->x)<5 && abs(y-event->y)<5)
			{
				OK = TRUE;
				break;
			}
			if(loop >0 && get_distance_M_AB(xyplot,(gint)event->x, (gint)event->y,  (gint)xOld, (gint)yOld, (gint)x, (gint)y)<5)
			{
				OK = TRUE;
				break;
			}
			xOld = x;
			yOld = y;
		}
		if(OK) break;
	}
	if(OK)
	{
		set_data_dialog(xyplot,data);
	}
	return OK;
}
/*********************************************************************************************************************/
static gboolean popuo_menu(GtkWidget* widget, guint button, guint32 time)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (widget), "Manager");
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuXYPlot");
	if (GTK_IS_MENU (menu)) 
	{
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, time);
		return TRUE;
	}
	else 
	{
		g_message (_("popup menu of xyplot failed"));
	}
	return FALSE;
}
/****************************************************************************************/
static gint gabedit_xyplot_button_press (GtkWidget *widget, GdkEventButton *event)
{
  GabeditXYPlot *xyplot;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GABEDIT_IS_XYPLOT (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  xyplot = GABEDIT_XYPLOT (widget);
  if(event->type == GDK_2BUTTON_PRESS)
  {
	  xyplot->double_click = TRUE;
	  return gabedit_xyplot_double_click(widget,event);
  }
  if(event->button == 3)
  {
	  return popuo_menu(widget, event->button, event->time);
  }

  if (xyplot->mouse_zoom_enabled && (event->button == xyplot->mouse_zoom_button || (event->button == 1 && xyplot->control_key_pressed )) )
    if ( (event->x > xyplot->plotting_rect.x) && 
         (event->x < (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) && 
         (event->y > xyplot->plotting_rect.y) && 
         (event->y < (xyplot->plotting_rect.y + xyplot->plotting_rect.height)) ){
      xyplot->mouse_button=xyplot->mouse_zoom_button;
    
      xyplot->zoom_point.x=event->x;
      xyplot->zoom_point.y=event->y;
	set_old_area(widget, xyplot);
  }
  if (xyplot->mouse_distance_enabled && (event->button == xyplot->mouse_distance_button && xyplot->shift_key_pressed ) )
    if ( (event->x > xyplot->plotting_rect.x) && 
         (event->x < (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) && 
         (event->y > xyplot->plotting_rect.y) && 
         (event->y < (xyplot->plotting_rect.y + xyplot->plotting_rect.height)) ){
      xyplot->mouse_button=xyplot->mouse_distance_button;
    
      xyplot->distance_point.x=event->x;
      xyplot->distance_point.y=event->y;
	set_old_area(widget, xyplot);
  }

  if (xyplot->mouse_displace_enabled && (event->button == xyplot->mouse_displace_button))
    if ( (event->x > xyplot->plotting_rect.x) && 
         (event->x < (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) && 
         (event->y > xyplot->plotting_rect.y) && 
         (event->y < (xyplot->plotting_rect.y + xyplot->plotting_rect.height)) ){
      xyplot->mouse_button=event->button;
    
      xyplot->move_point.x=event->x;
      xyplot->move_point.y=event->y;
  } 
  if (xyplot->r_key_pressed && !xyplot->control_key_pressed){
	  	gint i;
		xyplot->r_key_pressed = FALSE;
		i = get_object_text_num(xyplot, event->x, event->y);
		if(i>-1) delete_object_text(GTK_WIDGET(xyplot), i);
		else
		{
			i = get_object_line_num(xyplot, event->x, event->y);
			if(i>-1) delete_object_line(GTK_WIDGET(xyplot), i);
			else
			{
				i = get_object_image_num(xyplot, event->x, event->y);
				if(i>-1) delete_object_image(GTK_WIDGET(xyplot), i);
			}
		}
	}
  if (
	 !xyplot->t_key_pressed && 
	 !xyplot->l_key_pressed && 
	 !xyplot->i_key_pressed && 
	 !xyplot->shift_key_pressed && 
	 !xyplot->control_key_pressed && 
	 event->button != xyplot->mouse_zoom_button
   )

	  /*
    if ( (event->x > xyplot->plotting_rect.x) && 
         (event->x < (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) && 
         (event->y > xyplot->plotting_rect.y) && 
         (event->y < (xyplot->plotting_rect.y + xyplot->plotting_rect.height)) )
	 */
    {

	gdouble X, Y;
	gchar txt[BSIZE];
	PangoLayout *playout;
	gint i;

	if(gabedit_xyplot_get_point(GABEDIT_XYPLOT(xyplot), event->x, event->y, &X, &Y))
		sprintf(txt,"(%f ; %f)",X,Y);
	playout=gtk_widget_create_pango_layout (widget, txt);
	if(playout)
	{
		GdkRectangle rect;
		rect.x=0; 
		rect.y=0; 
		rect.width=widget->allocation.width;
		rect.height=widget->allocation.height;
		gtk_paint_layout (widget->style, widget->window, 
			GTK_STATE_NORMAL, FALSE, 
			&rect, widget, NULL, 
			event->x,
			event->y,
			playout);
		g_object_unref(G_OBJECT(playout));
	}
	i = get_object_text_num(xyplot, event->x, event->y);
	if(i>-1 && i<xyplot->nObjectsText) xyplot->selected_objects_text_num = i;
	else xyplot->selected_objects_text_num = -1;
	if(xyplot->selected_objects_text_num>-1) 
	{
		set_old_area(widget, xyplot);
    		gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
	}
	if(xyplot->selected_objects_text_num<0)
	{
		i = get_object_line_num(xyplot, event->x, event->y);
		xyplot->selected_objects_line_num = -1;
		xyplot->selected_objects_line_type = -1;
		if(i>-1 && i<xyplot->nObjectsLine) 
		{
			gdouble xx,yy,d;
			xyplot->selected_objects_line_num = i;
			xyplot->selected_objects_line_type = 0;
      			xyplot->object_begin_point.x=xyplot->objectsLine[i].x1i;
      			xyplot->object_begin_point.y=xyplot->objectsLine[i].y1i;
      			xyplot->object_end_point.x=xyplot->objectsLine[i].x2i;
      			xyplot->object_end_point.y=xyplot->objectsLine[i].y2i;
			xx = event->x-xyplot->objectsLine[i].x1i;
			yy = event->y-xyplot->objectsLine[i].y1i;
			d = xx*xx+yy*yy;
			xx = event->x-xyplot->objectsLine[i].x2i;
			yy = event->y-xyplot->objectsLine[i].y2i;
			if(d<xx*xx+yy*yy) xyplot->selected_objects_line_type = 1;
			set_old_area(widget, xyplot);
    			gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
		}
		
	}
	if(xyplot->selected_objects_text_num<0 && xyplot->selected_objects_line_num<0)
	{
		i = get_object_image_num(xyplot, event->x, event->y);
		if(i>-1 && i<xyplot->nObjectsImage) xyplot->selected_objects_image_num = i;
		else xyplot->selected_objects_image_num = -1;
		if(xyplot->selected_objects_image_num>-1) 
		{
			gdouble xx = -event->x+xyplot->objectsImage[i].xi+xyplot->objectsImage[i].widthi;
			gdouble yy = -event->y+xyplot->objectsImage[i].yi+xyplot->objectsImage[i].heighti;
			if(xx*xx+yy*yy<
			(xyplot->objectsImage[i].widthi*xyplot->objectsImage[i].widthi+
			xyplot->objectsImage[i].heighti*xyplot->objectsImage[i].heighti)/100
			)
			{
      				xyplot->object_begin_point.x=-1;
      				xyplot->object_begin_point.y=-1;
      				xyplot->object_end_point.x=(gint)fabs(xx);
      				xyplot->object_end_point.y=(gint)fabs(yy);
			}
			else
			{
      				xyplot->object_begin_point.x=event->x-xyplot->objectsImage[i].xi;
      				xyplot->object_begin_point.y=event->y-xyplot->objectsImage[i].yi;
      				xyplot->object_end_point.x=-1;
      				xyplot->object_end_point.y=-1;
			}
			set_old_area(widget, xyplot);
    			gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
		}
	}

  } 
  if (xyplot->t_key_pressed && !xyplot->control_key_pressed)
	  /*
    if ( (event->x > xyplot->plotting_rect.x) && 
         (event->x < (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) && 
         (event->y > xyplot->plotting_rect.y) && 
         (event->y < (xyplot->plotting_rect.y + xyplot->plotting_rect.height)) )
	 */
    {

	gdouble X, Y;

	if(gabedit_xyplot_get_point(GABEDIT_XYPLOT(xyplot), event->x, event->y, &X, &Y))
	{
		xyplot->t_key_pressed = FALSE;
		add_set_object_text_dialog(GTK_WIDGET(xyplot), -1, X, Y);
	}

  } 
  if (xyplot->l_key_pressed && !xyplot->control_key_pressed)
	  /*
    if ( (event->x > xyplot->plotting_rect.x) && 
         (event->x < (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) && 
         (event->y > xyplot->plotting_rect.y) && 
         (event->y < (xyplot->plotting_rect.y + xyplot->plotting_rect.height)) )
	 */
    {

	set_old_area(widget, xyplot);
	xyplot->l_key_pressed = FALSE;
      	xyplot->object_begin_point.x=event->x;
      	xyplot->object_begin_point.y=event->y;
      	xyplot->object_end_point.x=-1;
      	xyplot->object_end_point.y=-1;

  } 
  if (xyplot->i_key_pressed && !xyplot->control_key_pressed)
	  /*
    if ( (event->x > xyplot->plotting_rect.x) && 
         (event->x < (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) && 
         (event->y > xyplot->plotting_rect.y) && 
         (event->y < (xyplot->plotting_rect.y + xyplot->plotting_rect.height)) )
	 */
    {
		xyplot->i_key_pressed = FALSE;
		add_set_object_image_dialog(GTK_WIDGET(xyplot), -1, event->x, event->y);
	}
  if ( xyplot->mouse_autorange_enabled && 
      (event->button == xyplot->mouse_autorange_button) )
    gabedit_xyplot_set_autorange(xyplot, NULL);

   xyplot->double_click = FALSE;
  
  return FALSE;
}
/****************************************************************************************/
static gint gabedit_xyplot_button_release (GtkWidget *widget, GdkEventButton *event)
{
  GabeditXYPlot *xyplot;
  gdouble xmin, xmax, ymin, ymax;
  gint xleft, ytop, xright, ybottom; 
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GABEDIT_IS_XYPLOT (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  xyplot = GABEDIT_XYPLOT (widget);

  xyplot->selected_objects_text_num=-1;

   if (xyplot->selected_objects_image_num>-1)
   {
	xyplot->object_begin_point.x=-1;
	xyplot->object_begin_point.y=-1;
	xyplot->object_end_point.x=-1;
	xyplot->object_end_point.y=-1;
  	xyplot->selected_objects_image_num=-1;
   }
   if (xyplot->selected_objects_line_num>-1)
   {
	xyplot->object_begin_point.x=-1;
	xyplot->object_begin_point.y=-1;
	xyplot->object_end_point.x=-1;
	xyplot->object_end_point.y=-1;
  	xyplot->selected_objects_line_num=-1;
  	xyplot->selected_objects_line_type=-1;
   }

/* Zoom */
   if ( !xyplot->double_click && xyplot->mouse_zoom_enabled && 
       (xyplot->mouse_button == xyplot->mouse_zoom_button)){

    xleft=xyplot->zoom_rect.x-xyplot->plotting_rect.x;
    xright=xyplot->zoom_rect.x+xyplot->zoom_rect.width-xyplot->plotting_rect.x;
    ybottom=xyplot->plotting_rect.y+xyplot->plotting_rect.height-xyplot->zoom_rect.y-xyplot->zoom_rect.height;
    ytop=xyplot->plotting_rect.y+xyplot->plotting_rect.height-xyplot->zoom_rect.y;
        
    pixel2value(xyplot, xleft, ybottom, &xmin, &ymin);
    pixel2value(xyplot, xright, ytop, &xmax, &ymax);

    if(xyplot->reflect_x)
    {
	    gdouble a = xmin;
	    xmin = xmax;
	    xmax = a;
    }
    if(xyplot->reflect_y)
    {
	    gdouble a = ymin;
	    ymin = ymax;
	    ymax = a;
    }
  
    gabedit_xyplot_set_range(xyplot, xmin, xmax, ymin, ymax);
    
    xyplot->mouse_button=0;
  }
/* draw object line */
   if ( !xyplot->double_click && xyplot->object_begin_point.x>-1) {

	gdouble X1, Y1;
	gdouble X2, Y2;
	if(gabedit_xyplot_get_point(GABEDIT_XYPLOT(xyplot), event->x, event->y, &X2, &Y2)
	&& gabedit_xyplot_get_point(GABEDIT_XYPLOT(xyplot), xyplot->object_begin_point.x, xyplot->object_begin_point.y, &X1, &Y1))
	{
		xyplot->l_key_pressed = FALSE;
		add_object_line(xyplot, X1, Y1, X2, Y2);
	}
	xyplot->object_begin_point.x = -1;
   }
/* distance */
   if ( !xyplot->double_click && xyplot->mouse_distance_enabled && 
       (xyplot->mouse_button == xyplot->mouse_distance_button)){

    xleft=xyplot->distance_rect.x-xyplot->plotting_rect.x;
    xright=xyplot->distance_rect.x+xyplot->distance_rect.width-xyplot->plotting_rect.x;
    ybottom=xyplot->plotting_rect.y+xyplot->plotting_rect.height-xyplot->distance_rect.y-xyplot->distance_rect.height;
    ytop=xyplot->plotting_rect.y+xyplot->plotting_rect.height-xyplot->distance_rect.y;
        
    pixel2value(xyplot, xleft, ybottom, &xmin, &ymin);
    pixel2value(xyplot, xright, ytop, &xmax, &ymax);

    if(xyplot->reflect_x)
    {
	    gdouble a = xmin;
	    xmin = xmax;
	    xmax = a;
    }
    if(xyplot->reflect_y)
    {
	    gdouble a = ymin;
	    ymin = ymax;
	    ymax = a;
    }
    {
	GtkWidget* dialog = NULL;
	gchar* tmp = g_strdup_printf("dX = %f ; dY = %f",xmax-xmin,ymax-ymin);
	dialog = gtk_message_dialog_new_with_markup (NULL,
		           GTK_DIALOG_DESTROY_WITH_PARENT,
		           GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			   "%s",
			   tmp);
       gtk_dialog_run (GTK_DIALOG (dialog));
       gtk_widget_destroy (dialog);
       g_free(tmp);
    }
  
    /* gabedit_xyplot_set_range(xyplot, xmin, xmax, ymin, ymax);*/
    
    gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
    xyplot->mouse_button=0;
  }
   else
    gtk_widget_queue_draw(GTK_WIDGET(xyplot));  

  if (xyplot->mouse_displace_enabled && 
      (event->button == xyplot->mouse_displace_button) && 
      (xyplot->mouse_button == xyplot->mouse_displace_button) ){
    xyplot->mouse_button=0;
  }
    
  return FALSE;
}
/****************************************************************************************/
static gint gabedit_xyplot_motion_notify (GtkWidget *widget, GdkEventMotion *event)
{
  GabeditXYPlot *xyplot;
  gint x, y;
  gdouble px, py, mx, my;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GABEDIT_IS_XYPLOT (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  xyplot = GABEDIT_XYPLOT (widget);

  x = event->x;
  y = event->y;
  
  if (event->is_hint || (event->window != widget->window))
      gdk_window_get_pointer (widget->window, (gint *)&x, (gint *)&y, NULL);

  if (xyplot->selected_objects_text_num>-1) 
  {
	gint i = xyplot->selected_objects_text_num;
	XYPlotObjectText* objectText = &GABEDIT_XYPLOT(xyplot)->objectsText[i];
	gabedit_xyplot_get_point_control (xyplot, x, y, objectText->width, objectText->height, objectText->angle, &mx, &my);
	set_object_text(xyplot, objectText,  mx,my, objectText->angle, objectText->str);
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
  }
  else if (xyplot->selected_objects_line_num>-1) 
  {
	gint i = xyplot->selected_objects_line_num;
	gdouble x1=0, y1=0, x2=0, y2=0;
	XYPlotObjectLine* objectLine = &GABEDIT_XYPLOT(xyplot)->objectsLine[i];
	if(xyplot->selected_objects_line_type==0)
	{
      		xyplot->object_end_point.x=x;
      		xyplot->object_end_point.y=y;
		gabedit_xyplot_get_point_control (xyplot, xyplot->object_begin_point.x, xyplot->object_begin_point.y, 
				objectLine->width,objectLine->width, 0,  &x1, &y1);
		gabedit_xyplot_get_point_control (xyplot, xyplot->object_end_point.x, xyplot->object_end_point.y, 
				objectLine->width,objectLine->width, 0, &x2, &y2);
	}
	else if(xyplot->selected_objects_line_type==1)
	{
      		xyplot->object_begin_point.x=x;
      		xyplot->object_begin_point.y=y;
		gabedit_xyplot_get_point_control (xyplot, xyplot->object_begin_point.x, xyplot->object_begin_point.y, 
				objectLine->width,objectLine->width, 0, &x1, &y1);
		gabedit_xyplot_get_point_control (xyplot, xyplot->object_end_point.x, xyplot->object_end_point.y, 
				objectLine->width,objectLine->width, 0, &x2, &y2);
	}
	set_object_line(xyplot, objectLine, 
		x1, y1, 
		x2, y2,
  		objectLine->width,
  		objectLine->arrow_size,
  		objectLine->color,
  		objectLine->style
		);
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
  }
  else if (xyplot->selected_objects_image_num>-1) 
  {
	gint i = xyplot->selected_objects_image_num;
	XYPlotObjectImage* objectImage = &GABEDIT_XYPLOT(xyplot)->objectsImage[i];
	if(xyplot->object_begin_point.x>0)
	{
		gint dx,dy;
		x -= xyplot->object_begin_point.x;
		y -= xyplot->object_begin_point.y;
		if(x<0) x = 0;
		if(y<0) y = 0;
		dx = x+objectImage->widthi-widget->allocation.width;
		if(dx>0) x-=dx;
		dy = y+objectImage->heighti-widget->allocation.height;
		if(dy>0) y-=dy;
		set_object_image(xyplot, objectImage,  x, y, objectImage->widthi, objectImage->heighti);
	}
	if(xyplot->object_end_point.x>0)
	{
		gint w,h;
		x += xyplot->object_end_point.x;
		y += xyplot->object_end_point.y;
		if(x<objectImage->xi) x = objectImage->xi+20;
		if(y<objectImage->yi) y = objectImage->yi+20;
		if(x>widget->allocation.width) x = widget->allocation.width;
		if(y>widget->allocation.height) y = widget->allocation.height;
		w = (gint)(x-objectImage->xi);
		h = (gint)(y-objectImage->yi);
		set_object_image(xyplot, objectImage,  objectImage->xi, objectImage->yi, w, h);
	}

	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
  }
  else if (xyplot->object_begin_point.x>-1)
  {
      	xyplot->object_end_point.x=x;
      	xyplot->object_end_point.y=y;
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
  }

  /* Zoom */
  if (xyplot->mouse_zoom_enabled && 
      (xyplot->mouse_button == xyplot->mouse_zoom_button) ){
    if (x < xyplot->plotting_rect.x)
      x=xyplot->plotting_rect.x;
       
    if(x > (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) 
      x=(xyplot->plotting_rect.x + xyplot->plotting_rect.width);

    if (y < xyplot->plotting_rect.y)
      y=xyplot->plotting_rect.y;

    if (y > (xyplot->plotting_rect.y + xyplot->plotting_rect.height))
      y=(xyplot->plotting_rect.y + xyplot->plotting_rect.height);

    if (xyplot->zoom_point.x < x){
      xyplot->zoom_rect.x=xyplot->zoom_point.x;
      xyplot->zoom_rect.width=x-xyplot->zoom_point.x;
    }else{
      xyplot->zoom_rect.x=x;
      xyplot->zoom_rect.width=xyplot->zoom_point.x-x;
    }

    if (xyplot->zoom_point.y < y){
      xyplot->zoom_rect.y=xyplot->zoom_point.y;
      xyplot->zoom_rect.height=y-xyplot->zoom_point.y;
    }else{
      xyplot->zoom_rect.y=y;
      xyplot->zoom_rect.height=xyplot->zoom_point.y-y;
    }
    
    gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
  }
/* distance between 2 points */
  if (xyplot->mouse_distance_enabled && 
      (xyplot->mouse_button == xyplot->mouse_distance_button) ){
    if (x < xyplot->plotting_rect.x)
      x=xyplot->plotting_rect.x;
       
    if(x > (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) 
      x=(xyplot->plotting_rect.x + xyplot->plotting_rect.width);

    if (y < xyplot->plotting_rect.y)
      y=xyplot->plotting_rect.y;

    if (y > (xyplot->plotting_rect.y + xyplot->plotting_rect.height))
      y=(xyplot->plotting_rect.y + xyplot->plotting_rect.height);

      xyplot->distance_rect.x=xyplot->distance_point.x;
      xyplot->distance_rect.width=x-xyplot->distance_point.x;

      xyplot->distance_rect.y=xyplot->distance_point.y;
      xyplot->distance_rect.height=y-xyplot->distance_point.y;
    
    gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
  }

  if (xyplot->mouse_displace_enabled && (xyplot->mouse_button==xyplot->mouse_displace_button))
    if ( (x > xyplot->plotting_rect.x) && 
         (x < (xyplot->plotting_rect.width + xyplot->plotting_rect.x)) &&
         (y > xyplot->plotting_rect.y) && 
         (y < (xyplot->plotting_rect.height + xyplot->plotting_rect.y)) ){
      pixel2value(xyplot, xyplot->move_point.x, xyplot->move_point.y, &px, &py);
      pixel2value(xyplot, x, y, &mx, &my);
    
      gabedit_xyplot_set_range(xyplot, xyplot->xmin+(px-mx), xyplot->xmax+(px-mx), xyplot->ymin-(py-my), xyplot->ymax-(py-my)); 
      xyplot->move_point.x=x;
      xyplot->move_point.y=y;
  }

  return FALSE;
}
/****************************************************************************************/
static gint gabedit_xyplot_scroll (GtkWidget *widget, GdkEventScroll   *event)
{
  GabeditXYPlot *xyplot;
  gdouble x_range = 0.0, y_range = 0.0, 
          xmin = 0.0, xmax = 0.0, 
          ymin = 0.0, ymax = 0.0;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GABEDIT_IS_XYPLOT (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  xyplot=GABEDIT_XYPLOT(widget);

  if (xyplot->wheel_zoom_enabled){
    if (event->direction==GDK_SCROLL_UP){
      x_range=xyplot->wheel_zoom_factor*(xyplot->xmax-xyplot->xmin);
      y_range=xyplot->wheel_zoom_factor*(xyplot->ymax-xyplot->ymin);
    }

    if (event->direction==GDK_SCROLL_DOWN){
      x_range=(xyplot->xmax-xyplot->xmin)/xyplot->wheel_zoom_factor;
      y_range=(xyplot->ymax-xyplot->ymin)/xyplot->wheel_zoom_factor;
    }
 
    xmin=xyplot->xmin+((xyplot->xmax-xyplot->xmin)/2.0)-(x_range/2.0);
    xmax=xyplot->xmin+((xyplot->xmax-xyplot->xmin)/2.0)+(x_range/2.0);
     
    ymin=xyplot->ymin+((xyplot->ymax-xyplot->ymin)/2.0)-(y_range/2.0);
    ymax=xyplot->ymin+((xyplot->ymax-xyplot->ymin)/2.0)+(y_range/2.0);

    gabedit_xyplot_set_range(xyplot, xmin, xmax, ymin, ymax);
  }  
  return TRUE;
}
/****************************************************************************************/
static void gabedit_xyplot_style_set (GtkWidget *widget, GtkStyle *previous_style)
{
  GabeditXYPlot *xyplot;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (widget));
  
  xyplot = GABEDIT_XYPLOT (widget);
  
  xyplot_calculate_legends_sizes(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_range (GabeditXYPlot *xyplot, gdouble xmin, gdouble xmax, gdouble ymin, gdouble ymax)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if (xmin < xmax){
    xyplot->xmin=xmin;
    xyplot->xmax=xmax;
  }
  
  if (ymin < ymax){
    xyplot->ymin=ymin;
    xyplot->ymax=ymax;
  }
   
  xyplot_free_legends(xyplot);
  xyplot_build_legends(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_range_xmin (GabeditXYPlot *xyplot, gdouble xmin)
{
	gabedit_xyplot_set_range(xyplot, xmin,  xyplot->xmax, xyplot->ymin , xyplot->ymax);
}
/****************************************************************************************/
void gabedit_xyplot_set_range_xmax (GabeditXYPlot *xyplot, gdouble xmax)
{
	gabedit_xyplot_set_range(xyplot, xyplot->xmin,  xmax, xyplot->ymin , xyplot->ymax);
}
/****************************************************************************************/
void gabedit_xyplot_set_range_ymin (GabeditXYPlot *xyplot, gdouble ymin)
{
	gabedit_xyplot_set_range(xyplot, xyplot->xmin,  xyplot->xmax, ymin , xyplot->ymax);
}
/****************************************************************************************/
void gabedit_xyplot_set_range_ymax (GabeditXYPlot *xyplot, gdouble ymax)
{
	gabedit_xyplot_set_range(xyplot, xyplot->xmin,  xyplot->xmax, xyplot->ymin , ymax);
}
/****************************************************************************************/
void gabedit_xyplot_set_autorange (GabeditXYPlot *xyplot, XYPlotData *data)
{
  gint loop, loop2;
  gdouble xmax, xmin, ymax, ymin;
  gdouble dx, dy; 
  XYPlotData *current_data; 
  GList *current_node; 
 
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
   
  xmax=1.0;
  xmin=-1.0;
  ymax=1.0;
  ymin=-1.0;

  if (data!=NULL){ 
    xmax=data->x[0];
    xmin=data->x[0];
    ymax=data->y[0];
    ymin=data->y[0];
    for (loop=1; loop<data->size; loop++){
      if (data->x[loop] > xmax)
        xmax=data->x[loop];
      if (data->x[loop] < xmin)
        xmin=data->x[loop];
      if (data->y[loop] > ymax)
        ymax=data->y[loop];
      if (data->y[loop] < ymin)
        ymin=data->y[loop];
    }
  }

  if ( (data == NULL) && xyplot->data_list ){
    current_node=g_list_first(xyplot->data_list);
    current_data=(XYPlotData*)current_node->data;  
    xmin=current_data->x[0];
    xmax=current_data->x[0]; 
    ymin=current_data->y[0];
    ymax=current_data->y[0];
    for (; current_node!=NULL; current_node=current_node->next)
    {
        current_data=(XYPlotData*)current_node->data;  
      for (loop2=0; loop2<current_data->size; loop2++){
        if (xmin > current_data->x[loop2])
          xmin=current_data->x[loop2];
        if (xmax < current_data->x[loop2])
          xmax=current_data->x[loop2];
        if (ymin > current_data->y[loop2])
          ymin=current_data->y[loop2];
        if (ymax < current_data->y[loop2])
          ymax=current_data->y[loop2];
      }
    }
  }

  dx=fabs(xmax-xmin);
  dy=fabs(ymax-ymin);
  xmin=xmin-dx*0.1;
  xmax=xmax+dx*0.1;
  ymin=ymin-dy*0.1;
  ymax=ymax+dy*0.1;
  
  xyplot->xmin=xmin;
  xyplot->xmax=xmax;
  xyplot->ymin=ymin;
  xyplot->ymax=ymax;

  xyplot_free_legends(xyplot);
  xyplot_build_legends(xyplot);  
  xyplot_calculate_sizes(xyplot);

  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_get_reflects (GabeditXYPlot *xyplot, gboolean* rx, gboolean* ry)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if (rx!=NULL) *rx=xyplot->reflect_x;
  if (ry!=NULL) *ry=xyplot->reflect_y;
}
/****************************************************************************************/
void gabedit_xyplot_get_range (GabeditXYPlot *xyplot, gdouble *xmin, gdouble *xmax, gdouble *ymin, gdouble *ymax)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if (xmin!=NULL)
    *xmin=xyplot->xmin;
  if (xmax!=NULL)
    *xmax=xyplot->xmax;
  if (ymax!=NULL)
    *ymax=xyplot->ymax;
  if (ymin!=NULL)
    *ymin=xyplot->ymin;
}
/****************************************************************************************/
gboolean gabedit_xyplot_get_point (GabeditXYPlot *xyplot, gint x, gint y, gdouble *xv, gdouble *yv)
{
  g_return_val_if_fail (xyplot != NULL, FALSE);
  g_return_val_if_fail (GABEDIT_IS_XYPLOT (xyplot),FALSE);
  
  /*
  if ((x < xyplot->plotting_rect.x) || 
      (x > (xyplot->plotting_rect.x + xyplot->plotting_rect.width)) || 
      (y < xyplot->plotting_rect.y) || 
      (y > (xyplot->plotting_rect.y + xyplot->plotting_rect.height))) {
    *xv=*yv=0;
    return FALSE;
  }
  */
  
  x=x-xyplot->plotting_rect.x;
  y=xyplot->plotting_rect.y+xyplot->plotting_rect.height-y; 
  
  pixel2value(xyplot, x, y, xv, yv);
  return TRUE;
}
/****************************************************************************************/
gboolean gabedit_xyplot_get_point_control(GabeditXYPlot *xyplot, gint x, gint y, gint width, gint height, gdouble angle, gdouble *xv, gdouble *yv )
{
	gdouble xa, ya;
	gdouble xmin,xmax,ymin,ymax;
	gint a[4],b[4];
	gint i;
	gint ixmin,ixmax,iymin,iymax;
	gdouble alpha = angle;
	gdouble l;
	gint lcos;
	gint lsin;
	gint wcos;
	gint wsin;
	gint hcos;
	gint hsin;
	GtkWidget *widget = GTK_WIDGET(xyplot);


	g_return_val_if_fail (xyplot != NULL, FALSE);
	g_return_val_if_fail (GABEDIT_IS_XYPLOT (xyplot),FALSE);
	if(width>0) alpha -= atan((gdouble)height/width);
	l = sqrt(width*width+height*height);
	lcos = (gint)(l*cos(alpha));
	lsin = (gint)(l*sin(alpha));
	wcos = (gint)(width*cos(angle));
	wsin = (gint)(width*sin(angle));
	hcos = (gint)(height*cos(angle-M_PI/2));
	hsin = (gint)(height*sin(angle-M_PI/2));

	ixmin = 0;
	if(ixmin<-lcos) ixmin = -lcos;
	if(ixmin<-wcos) ixmin = -wcos;
	if(ixmin<-hcos) ixmin = -hcos;

	iymin = 0;
	if(iymin<lsin) iymin = lsin;
	if(iymin<wsin) iymin = wsin;
	if(iymin<hsin) iymin = hsin;

	ixmax = 0;
	if(ixmax<lcos) ixmax = lcos;
	if(ixmax<wcos) ixmax = wcos;
	if(ixmax<hcos) ixmax = hcos;
	ixmax = widget->allocation.width-ixmax;

	iymax = 0;
	if(iymax<-lsin) iymax = -lsin;
	if(iymax<-wsin) iymax = -wsin;
	if(iymax<-hsin) iymax = -hsin;
	iymax = widget->allocation.height-iymax;

	/* printf("iminmax = %d %d %d %d\n",ixmin,ixmax,iymin,iymax);*/
	a[0] = ixmin;
	a[1] = ixmax;
	a[2] = ixmax;
	a[3] = ixmin;
	b[0] = iymin;
	b[1] = iymin;
	b[2] = iymax;
	b[3] = iymax;

	gabedit_xyplot_get_point (xyplot, x, y, xv, yv);
	xmin = xmax = *xv;
	ymin = ymax = *yv;
	for(i=0;i<4;i++)
	{
		gabedit_xyplot_get_point (xyplot, a[i], b[i], &xa, &ya);
		if(i==0)
		{
			xmin = xa;
			xmax = xa;
			ymin = ya;
			ymax = ya;
		}
		else
		{
			if(xmin>xa) xmin = xa;
			if(ymin>ya) ymin = ya;
			if(xmax<xa) xmax = xa;
			if(ymax<ya) ymax = ya;
		}
	}
	/* printf("xmax ymax = %f %f\n",xmax,ymax);*/

	if(*xv<xmin) *xv = xmin;
	if(*yv<ymin) *yv = ymin;
	if(*xv>xmax) *xv = xmax;
	if(*yv>ymax) *yv = ymax;
	return TRUE;
}
/****************************************************************************************/
void gabedit_xyplot_set_ticks (GabeditXYPlot *xyplot, gint hmajor, gint hminor, gint vmajor, gint vminor, gint length)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  xyplot_free_legends(xyplot);

  xyplot->hmajor_ticks=hmajor;
  xyplot->hminor_ticks=hminor;
  xyplot->vmajor_ticks=vmajor;
  xyplot->vminor_ticks=vminor;
  xyplot->length_ticks=length;
    
  xyplot_build_legends(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_ticks_hmajor (GabeditXYPlot *xyplot, gint hmajor)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  xyplot_free_legends(xyplot);

  xyplot->hmajor_ticks=hmajor;
    
  xyplot_build_legends(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_ticks_hminor (GabeditXYPlot *xyplot, gint hminor)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  xyplot_free_legends(xyplot);

  xyplot->hminor_ticks=hminor;
    
  xyplot_build_legends(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_ticks_vmajor (GabeditXYPlot *xyplot, gint vmajor)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  xyplot_free_legends(xyplot);

  xyplot->vmajor_ticks=vmajor;
    
  xyplot_build_legends(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_ticks_vminor (GabeditXYPlot *xyplot, gint vminor)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  xyplot_free_legends(xyplot);

  xyplot->vminor_ticks=vminor;
    
  xyplot_build_legends(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_ticks_length(GabeditXYPlot *xyplot, gint length)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  xyplot_free_legends(xyplot);

  xyplot->length_ticks=length;
    
  xyplot_build_legends(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_get_ticks (GabeditXYPlot *xyplot, gint *hmajor, gint *hminor, gint *vmajor, gint *vminor, gint* length)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if (hmajor!=NULL)
    *hmajor=xyplot->hmajor_ticks;
  if (hminor!=NULL)
    *hminor=xyplot->hminor_ticks;
  if (vmajor!=NULL)
    *vmajor=xyplot->vmajor_ticks;
  if (vminor!=NULL)
    *vminor=xyplot->vminor_ticks;
  if (length!=NULL)
    *length=xyplot->length_ticks;
}
/****************************************************************************************/
void gabedit_xyplot_set_margins_left (GabeditXYPlot *xyplot, gint left)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  xyplot_free_legends(xyplot);

  xyplot->left_margins=left;
    
  xyplot_build_legends(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_margins_right (GabeditXYPlot *xyplot, gint right)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  xyplot_free_legends(xyplot);

  xyplot->right_margins=right;
    
  xyplot_build_legends(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_margins_top (GabeditXYPlot *xyplot, gint top)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  xyplot_free_legends(xyplot);

  xyplot->top_margins=top;
    
  xyplot_build_legends(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_margins_bottom (GabeditXYPlot *xyplot, gint bottom)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  xyplot_free_legends(xyplot);

  xyplot->bottom_margins=bottom;
    
  xyplot_build_legends(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_x_legends_digits (GabeditXYPlot *xyplot, gint digits)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if (digits != xyplot->x_legends_digits){
    xyplot_free_legends(xyplot);
  
    xyplot->x_legends_digits=digits;
  
    xyplot_build_legends(xyplot);
    xyplot_calculate_legends_sizes(xyplot);
    xyplot_calculate_sizes(xyplot);
    gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
  }
}
/****************************************************************************************/
void gabedit_xyplot_set_y_legends_digits (GabeditXYPlot *xyplot, gint digits)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if (digits != xyplot->y_legends_digits){
    xyplot_free_legends(xyplot);
  
    xyplot->y_legends_digits=digits;
  
    xyplot_build_legends(xyplot);
    xyplot_calculate_legends_sizes(xyplot);
    xyplot_calculate_sizes(xyplot);
    gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
  }
}
/****************************************************************************************/
gint gabedit_xyplot_get_x_legends_digits (GabeditXYPlot *xyplot)
{
  g_return_val_if_fail (xyplot != NULL, 0);
  g_return_val_if_fail (GABEDIT_IS_XYPLOT (xyplot), 0);
  
  return(xyplot->x_legends_digits);
}
/****************************************************************************************/
gint gabedit_xyplot_get_y_legends_digits (GabeditXYPlot *xyplot)
{
  g_return_val_if_fail (xyplot != NULL, 0);
  g_return_val_if_fail (GABEDIT_IS_XYPLOT (xyplot), 0);
  
  return(xyplot->y_legends_digits);
}
/****************************************************************************************/
void gabedit_xyplot_set_background_color (GabeditXYPlot *xyplot, GdkColor color)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

  gdk_gc_set_rgb_fg_color(xyplot->back_gc, &color);

  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_grids_attributes (GabeditXYPlot *xyplot, 
			GabeditXYPlotGrid grid, 
			GdkColor color, 
			gint line_width, 
			GdkLineStyle line_style)
{
  GdkGC *gc;

  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  switch (grid){
    case GABEDIT_XYPLOT_HMAJOR_GRID:
      gc=xyplot->hmajor_grid_gc;  
    break;
    case GABEDIT_XYPLOT_HMINOR_GRID:
      gc=xyplot->hminor_grid_gc;
    break;
    case GABEDIT_XYPLOT_VMAJOR_GRID:
      gc=xyplot->vmajor_grid_gc;
    break;
    case GABEDIT_XYPLOT_VMINOR_GRID:
      gc=xyplot->vminor_grid_gc;
    break;
    default:
      return;
  }
  
  gdk_gc_set_rgb_fg_color(gc, &color);
  gdk_gc_set_line_attributes(gc, line_width, line_style, 0 ,0);
  
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_get_grids_attributes (GabeditXYPlot *xyplot, 
			GabeditXYPlotGrid grid, 
			GdkColor *color, 
			gint *line_width, 
			GdkLineStyle *line_style)
{
  GdkGC *gc;
  GdkGCValues gc_values;

  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  g_return_if_fail (color != NULL);
  g_return_if_fail (line_width != NULL);
  g_return_if_fail (line_style != NULL);
  
  switch (grid){
    case GABEDIT_XYPLOT_HMAJOR_GRID:
      gc=xyplot->hmajor_grid_gc;  
    break;
    case GABEDIT_XYPLOT_HMINOR_GRID:
      gc=xyplot->hminor_grid_gc;
    break;
    case GABEDIT_XYPLOT_VMAJOR_GRID:
      gc=xyplot->vmajor_grid_gc;
    break;
    case GABEDIT_XYPLOT_VMINOR_GRID:
      gc=xyplot->vminor_grid_gc;
    break;
    default:
      return;
  }
  
  gdk_gc_get_values(gc, &gc_values);
  
  *color=gc_values.foreground;
  *line_width=gc_values.line_width;
  *line_style=gc_values.line_style;
}
/****************************************************************************************/
static void xyplot_enable_grids (GabeditXYPlot *xyplot, GabeditXYPlotGrid grid, gboolean enable)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  switch (grid){
    case GABEDIT_XYPLOT_HMAJOR_GRID:
      xyplot->hmajor_grid=enable;  
    break;
    case GABEDIT_XYPLOT_HMINOR_GRID:
      xyplot->hminor_grid=enable;  
    break;
    case GABEDIT_XYPLOT_VMAJOR_GRID:
      xyplot->vmajor_grid=enable;  
    break;
    case GABEDIT_XYPLOT_VMINOR_GRID:
      xyplot->vminor_grid=enable;  
    break;
    default:
      return;
  }
  
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_enable_grids (GabeditXYPlot *xyplot, GabeditXYPlotGrid grid, gboolean enable)
{

	GtkUIManager *manager = g_object_get_data(G_OBJECT (xyplot), "Manager");
	GtkAction* action = NULL;
	if(!manager) return;
	if(grid==GABEDIT_XYPLOT_HMAJOR_GRID)
		action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderGrid/HGridShowMajor");
	if(grid==GABEDIT_XYPLOT_HMINOR_GRID)
		action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderGrid/HGridShowMinor");
	if(grid==GABEDIT_XYPLOT_VMAJOR_GRID)
		action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderGrid/VGridShowMajor");
	if(grid==GABEDIT_XYPLOT_VMINOR_GRID)
		action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderGrid/VGridShowMinor");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
void gabedit_xyplot_add_data(GabeditXYPlot *xyplot, XYPlotData *data)
{
	g_return_if_fail (xyplot != NULL);
	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
	g_return_if_fail (data != NULL);
  
	if (g_list_find (xyplot->data_list, (gpointer)data)==NULL)
	{
		xyplot->data_list=g_list_append(xyplot->data_list, (gpointer) data);
		data->point_pango = NULL;
		xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), data);
	}
}
/****************************************************************************************/
void gabedit_xyplot_remove_data(GabeditXYPlot *xyplot, XYPlotData *data)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  g_return_if_fail (data != NULL);

  if (g_list_find (xyplot->data_list, (gpointer)data)!=NULL){
    xyplot->data_list=g_list_remove_all (xyplot->data_list, (gpointer) data);
  }
}

/****************************************************************************************/
void gabedit_xyplot_configure_mouse_zoom(GabeditXYPlot *xyplot, gboolean enabled, gint button)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

  xyplot->mouse_zoom_enabled=enabled;
  xyplot->mouse_zoom_button=button;
}
/****************************************************************************************/
void gabedit_xyplot_configure_mouse_distance(GabeditXYPlot *xyplot, gboolean enabled, gint button)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

  xyplot->mouse_distance_enabled=enabled;
  xyplot->mouse_distance_button=button;
}
/****************************************************************************************/
void gabedit_xyplot_configure_wheel_zoom(GabeditXYPlot *xyplot, gboolean enabled, gdouble factor)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

  xyplot->wheel_zoom_enabled=enabled;
  xyplot->wheel_zoom_factor=factor;
}
/****************************************************************************************/
void gabedit_xyplot_configure_mouse_displace(GabeditXYPlot *xyplot, gboolean enabled, gint button)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

  xyplot->mouse_displace_enabled=enabled;
  xyplot->mouse_displace_button=button;
}
/****************************************************************************************/
void gabedit_xyplot_configure_mouse_autorange(GabeditXYPlot *xyplot, gboolean enabled, gint button)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

  xyplot->mouse_autorange_enabled=enabled;
  xyplot->mouse_autorange_button=button;
}
/********************************************************************************/
static guchar *get_rgb_image(GtkWidget* drawable)
{
	gdouble fac=255.0/65535.0;
	GdkColormap *colormap;
  	gint height;
  	gint width;
	gint32 pixel;
  	GdkImage* image = NULL;
	GdkVisual *v;
	gint8 component;
	gint k=0;
	gint x;
	gint y;
	gint i;
	guchar* rgbbuf=NULL;

	
	colormap = gdk_window_get_colormap(GTK_WIDGET(drawable)->window);
  	height = drawable->allocation.height;
  	width = drawable->allocation.width;
	rgbbuf=(guchar *) g_malloc(3*width*height*sizeof(guchar));

  	if(!colormap) return NULL;
	if(!rgbbuf) return NULL;
	
	image = gdk_image_get(drawable->window,0,0,width,height);

	v = gdk_colormap_get_visual(colormap);

	switch(v->type)
	{
		case GDK_VISUAL_STATIC_GRAY:
		case GDK_VISUAL_GRAYSCALE:
		case GDK_VISUAL_STATIC_COLOR:
		case GDK_VISUAL_PSEUDO_COLOR:
		for(y=height-1;y>=0;y--)
		for(x=0;x<(gint)width;x++)
		{
			pixel = gdk_image_get_pixel(image, x, y);
			rgbbuf[k] = (guchar)(colormap->colors[pixel].red*fac);
			rgbbuf[k+1] =(guchar) (colormap->colors[pixel].green*fac);
			rgbbuf[k+2] =(guchar) (colormap->colors[pixel].blue*fac);
			k+=3;
		}
		break;

	    case GDK_VISUAL_TRUE_COLOR:
		for(y=height-1;y>=0;y--)
		for(x=0;x<(gint)width;x++)
		{
			pixel = gdk_image_get_pixel(image, x, y);
	      		component = 0;
	      		for (i = 24; i < 32; i += v->red_prec)
				component |= ((pixel & v->red_mask) << (32 - v->red_shift - v->red_prec)) >> i;
	      		rgbbuf[k] = (guchar)(component);
	      		
			component = 0;
	      		for (i = 24; i < 32; i += v->green_prec)
				component |= ((pixel & v->green_mask) << (32 - v->green_shift - v->green_prec)) >> i;
	      		rgbbuf[k+1] = (guchar)(component);
	      		component = 0;
	      		for (i = 24; i < 32; i += v->blue_prec)
				component |= ((pixel & v->blue_mask) << (32 - v->blue_shift - v->blue_prec)) >> i;
	      		rgbbuf[k+2] = (guchar)(component);
	      		k += 3;
		}
	       break;
	    case GDK_VISUAL_DIRECT_COLOR:
		for(y=height-1;y>=0;y--)
		for(x=0;x<(gint)width;x++)
		{
			pixel = gdk_image_get_pixel(image, x, y);
	      		component = colormap->colors[((pixel & v->red_mask) << (32 - v->red_shift - v->red_prec)) >> 24].red;
	      		rgbbuf[k] = (guchar)(component*fac);

      			component = colormap->colors[((pixel & v->green_mask) << (32 - v->green_shift - v->green_prec)) >> 24].green;
      			rgbbuf[k+1] = (guchar)(component*fac);
      			component = colormap->colors[((pixel & v->blue_mask) << (32 - v->blue_shift - v->blue_prec)) >> 24].blue;
      			rgbbuf[k+2] = (guchar)(component*fac);
      			k += 3;
		}
	       break;
	       default :
	       {
		        g_free(rgbbuf);
		      	return NULL;
	       }
	}
	return rgbbuf;
}
/********************************************************************************/
static void WLSBL(int val,char* arr)
{
    arr[0] = (char) (val&0xff);
    arr[1] = (char) ((val>>8) &0xff);
    arr[2] = (char) ((val>>16)&0xff);
    arr[3] = (char) ((val>>24)&0xff);
}
/**************************************************************************/
static void writeBMP(GabeditXYPlot *xyplot, gchar *fileName)
{       
  	guchar rgbtmp[3];
  	int pad;
	char bmp_header[]=
	{ 'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0,
  	40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0, 0,0,0,0, 0,0,0,0,
  	0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0 };
	GtkWidget *drawable;

	FILE *file;
	int i;
	int j;
	int width;
	int height;
	guchar *rgbbuf;

        file = fopen(fileName,"wb");

        if (!file) return;

	drawable = GTK_WIDGET(xyplot);

	rgbbuf = get_rgb_image(drawable);
	if (!rgbbuf) {
	    	fclose(file);
            	return;
	}
	width =  drawable->allocation.width;
	height = drawable->allocation.height;


/* The number of bytes on a screenline should be wholly devisible by 4 */

  	pad = (width*3)%4;
  	if (pad) pad = 4 - pad;

  	WLSBL((int) (3*width+pad)*height+54,bmp_header+2);
  	WLSBL((int) width,bmp_header+18);
  	WLSBL((int) height,bmp_header+22);
  	WLSBL((int) 3*width*height,bmp_header+34);

  	{int it = fwrite(bmp_header,1,54,file);}

  	for (i=0;i<height;i++)
	{
    		for (j=0;j<width;j++)
		{
			rgbtmp[0] = rgbbuf[(j+width*i)*3+2];
			rgbtmp[1] = rgbbuf[(j+width*i)*3+1];
			rgbtmp[2] = rgbbuf[(j+width*i)*3+0];
			{int it = fwrite(rgbtmp,3,1,file);}
    		}
    	rgbtmp[0] = (char) 0;
    	for (j=0;j<pad;j++) 
		{int it = fwrite(rgbtmp,1,1,file);}
  	}

  	fclose(file);
  	g_free(rgbbuf);
}
/****************************************************************************************/
static void writeTransparentPNG(GabeditXYPlot *xyplot, gchar *fileName)
{       
	GtkWidget* widget = GTK_WIDGET(xyplot);
	int width;
	int height;
	GError *error = NULL;
	GdkPixbuf  *pixbuf = NULL;
	width =  widget->allocation.width;
	height = widget->allocation.height;
	pixbuf = gdk_pixbuf_get_from_drawable(NULL, widget->window, NULL, 0, 0, 0, 0, width, height);
	if(pixbuf)
	{
		GdkPixbuf  *pixbufNew = NULL;
		guchar color[3] = {255, 255, 255};
		GdkColor c;
		GdkGCValues gc_values;

		GdkColormap *colormap;
   		colormap  = gdk_window_get_colormap(widget->window);
		gdk_gc_get_values(xyplot->back_gc, &gc_values);
        	gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&c);

		color[0] = (guchar)(SCALE(c.red)*255);
		color[1] = (guchar)(SCALE(c.green)*255);
		color[2] = (guchar)(SCALE(c.blue)*255);

		pixbufNew = gdk_pixbuf_add_alpha(pixbuf, TRUE, color[0], color[1], color[2]);
		if(pixbufNew) gdk_pixbuf_save(pixbufNew, fileName, "png", &error, NULL);
		else gdk_pixbuf_save(pixbuf, fileName, "png", &error, NULL);
	 	g_object_unref (pixbuf);
	 	g_object_unref (pixbufNew);
	}
}
/****************************************************************************************/
void gabedit_xyplot_save_image(GabeditXYPlot *xyplot, gchar *fileName, gchar* type)
{       
	GtkWidget* widget = GTK_WIDGET(xyplot);
	int width;
	int height;
	GError *error = NULL;
	GdkPixbuf  *pixbuf = NULL;

	if(fileName && type && !strcmp(type,"bmp"))
	{
		writeBMP(xyplot, fileName);
		return;
	}
	if(fileName && type && !strcmp(type,"tpng"))
	{
		writeTransparentPNG(xyplot, fileName);
		return;
	}

	width =  widget->allocation.width;
	height = widget->allocation.height;
	pixbuf = gdk_pixbuf_get_from_drawable(NULL, widget->window, NULL, 0, 0, 0, 0, width, height);
	if(pixbuf)
	{
		if(!fileName)
		{
			GtkClipboard * clipboard;
			clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
			if(clipboard)
			{
				gtk_clipboard_clear(clipboard);
				gtk_clipboard_set_image(clipboard, pixbuf);
			}
		}
		else 
		{
			if(type && strstr(type,"j") && strstr(type,"g") )
			gdk_pixbuf_save(pixbuf, fileName, type, &error, "quality", "100", NULL);
			else if(type && strstr(type,"png"))
			gdk_pixbuf_save(pixbuf, fileName, type, &error, "compression", "5", NULL);
			else if(type && (strstr(type,"tif") || strstr(type,"tiff")))
			gdk_pixbuf_save(pixbuf, fileName, "tiff", &error, "compression", "1", NULL);
			else
			gdk_pixbuf_save(pixbuf, fileName, type, &error, NULL);
		}
	 	g_object_unref (pixbuf);
	}
}
/****************************************************************************************/
static void xyplot_calculate_sizes (GabeditXYPlot *xyplot)
{
  GtkWidget *widget;
  gint left = 10;
  gint right = 10;
  gint top = 5;
  gint bottom = 5;
  
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  widget = GTK_WIDGET(xyplot);
  left += 0.75*((gdouble)xyplot->y_legends_width);
  right += 0.75*((gdouble)xyplot->y_legends_width);
  top += 0.5*((gdouble)xyplot->x_legends_height);
  bottom += 0.5*((gdouble)xyplot->x_legends_height);
  if(xyplot->show_left_legends) left = xyplot->length_ticks*4+1.5*((gdouble)xyplot->y_legends_width)+1.0*((gdouble)xyplot->v_label_height);
  if(xyplot->show_right_legends) right = xyplot->length_ticks*4+1.5*((gdouble)xyplot->y_legends_width)+1.0*((gdouble)xyplot->v_label_height);
  if(xyplot->show_top_legends) top = xyplot->length_ticks*2+1.5*((gdouble)xyplot->x_legends_height)+1.0*((gdouble)xyplot->h_label_height);
  if(xyplot->show_bottom_legends) bottom = xyplot->length_ticks*2+1.5*((gdouble)xyplot->x_legends_height)+1.0*((gdouble)xyplot->h_label_height);

  left += (gint)(xyplot->left_margins/100.0*widget->allocation.width);
  right += (gint)(xyplot->right_margins/100.0*widget->allocation.width);
  top += (gint)(xyplot->top_margins/100.0*widget->allocation.height);
  bottom += (gint)(xyplot->bottom_margins/100.0*widget->allocation.height);

  xyplot->plotting_rect.x = left;
  xyplot->plotting_rect.y = top;
  
 
  xyplot->plotting_rect.width=widget->allocation.width  -left - right;

  xyplot->plotting_rect.height=widget->allocation.height  - top - bottom ;
  
  if (xyplot->hmajor_ticks>1){
    xyplot->d_hmajor=((gdouble)xyplot->plotting_rect.width)/((gdouble)xyplot->hmajor_ticks-1.0);
    xyplot->d_hminor=((gdouble)xyplot->d_hmajor)/((gdouble)xyplot->hminor_ticks+1.0);
  }
  
  if (xyplot->vmajor_ticks>1){
    xyplot->d_vmajor=((gdouble)xyplot->plotting_rect.height)/((gdouble)xyplot->vmajor_ticks-1.0);
    xyplot->d_vminor=((gdouble)xyplot->d_vmajor)/((gdouble)xyplot->vminor_ticks+1.0);
  }

  /* Creating the plotting area (everytime the plotting area's size is changed, the size of the pismap
     must be changed too, so, a new one, with the right size is created*/
  if (xyplot->plotting_area!=NULL) g_object_unref(G_OBJECT(xyplot->plotting_area));
  if (xyplot->cairo_widget!=NULL) cairo_destroy (xyplot->cairo_widget);
  if (xyplot->cairo_area!=NULL) cairo_destroy (xyplot->cairo_area);

  if (GTK_WIDGET_REALIZED(widget)) 
  {
    xyplot->plotting_area=gdk_pixmap_new(widget->window, xyplot->plotting_rect.width, xyplot->plotting_rect.height, -1);
    xyplot->cairo_area = gdk_cairo_create (xyplot->plotting_area);
    xyplot->cairo_widget = gdk_cairo_create (widget->window);
  }
  reset_object_text_pixels(xyplot);
  reset_object_line_pixels(xyplot);
  reset_object_image_pixels(xyplot);
}
/****************************************************************************************/
static void value2pixel(GabeditXYPlot *xyplot, gdouble xv, gdouble yv, gint *x, gint *y)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  *x=(xv-xyplot->xmin)*((gdouble)xyplot->plotting_rect.width)/(xyplot->xmax-xyplot->xmin);
  if(xyplot->reflect_x) *x = xyplot->plotting_rect.width - *x;

  *y=(yv-xyplot->ymin)*((gdouble)xyplot->plotting_rect.height)/(xyplot->ymax-xyplot->ymin);
  if(xyplot->reflect_y) *y = xyplot->plotting_rect.height - *y;
}
/****************************************************************************************/
static void pixel2value(GabeditXYPlot *xyplot, gint xp, gint yp, gdouble *x, gdouble *y)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  gdouble xxp = (gdouble)xp;
  gdouble yyp = (gdouble)yp;
  if(xyplot->reflect_x)  xxp = xyplot->plotting_rect.width-xxp;
  if(xyplot->reflect_y)  yyp = xyplot->plotting_rect.height-yyp;

    
  *x=xyplot->xmin+((gdouble)xxp)*(xyplot->xmax-xyplot->xmin)/((gdouble)xyplot->plotting_rect.width);
  *y=xyplot->ymin+((gdouble)yyp)*(xyplot->ymax-xyplot->ymin)/((gdouble)xyplot->plotting_rect.height);
}
/****************************************************************************************/
static void xyplot_calculate_legends_sizes(GabeditXYPlot *xyplot)
{
  GtkWidget *widget;
    
  g_return_if_fail (xyplot != NULL);
  
  widget=GTK_WIDGET(xyplot);

  if (xyplot->x_legends_digits==0 && xyplot->y_legends_digits==0){
    xyplot->x_legends_width=0;
    xyplot->x_legends_height=0;
    xyplot->y_legends_width=0;
    xyplot->y_legends_height=0;
    return;
  }

  if (xyplot->h_legends)
  {
    pango_layout_set_font_description (xyplot->h_legends[0], widget->style->font_desc);  
    pango_layout_get_size(xyplot->h_legends[0], &(xyplot->x_legends_width), &(xyplot->x_legends_height));
  }
  if (xyplot->v_legends)
  {
	  gint i;
	pango_layout_set_font_description (xyplot->v_legends[0], widget->style->font_desc);  
	pango_layout_get_size(xyplot->v_legends[0], &(xyplot->y_legends_width), &(xyplot->y_legends_height));
    	for (i=1; i<xyplot->vmajor_ticks; i++)
    	{
		gint w,h;
    		pango_layout_set_font_description (xyplot->v_legends[i], widget->style->font_desc);  
    		pango_layout_get_size(xyplot->v_legends[i], &w,&h); 
		if(w>xyplot->y_legends_width) xyplot->y_legends_width=w;
    	}
  }

  xyplot->x_legends_width/=PANGO_SCALE;
  xyplot->x_legends_height/=PANGO_SCALE;
  xyplot->y_legends_width/=PANGO_SCALE;
  xyplot->y_legends_height/=PANGO_SCALE;

  xyplot->h_label_width = 0;
  xyplot->h_label_height =0;
  if (xyplot->h_label)
  {
    	pango_layout_set_font_description (xyplot->h_label, widget->style->font_desc);  
    	pango_layout_get_size(xyplot->h_label, &(xyplot->h_label_width), &(xyplot->h_label_height));
	xyplot->h_label_width /=PANGO_SCALE;
	xyplot->h_label_height /=PANGO_SCALE;
  }
  xyplot->v_label_width = 0;
  xyplot->v_label_height =0;
  if (xyplot->v_label)
  {
    	pango_layout_set_font_description (xyplot->v_label, widget->style->font_desc);  
    	pango_layout_get_size(xyplot->v_label, &(xyplot->v_label_width), &(xyplot->v_label_height));
	xyplot->v_label_width /=PANGO_SCALE;
	xyplot->v_label_height /=PANGO_SCALE;
  }
} 
/****************************************************************************************/
static void xyplot_build_legends(GabeditXYPlot *xyplot)
{
  GtkWidget *widget;
  gint loop;

  g_return_if_fail (xyplot != NULL);
  
  widget=GTK_WIDGET(xyplot);

  if ( (xyplot->hmajor_ticks > 1) && (xyplot->x_legends_digits != 0)){
    xyplot->d_hlegend=fabs(xyplot->xmax-xyplot->xmin)/((gdouble)xyplot->hmajor_ticks-1.0);

    xyplot->h_legends=g_new(PangoLayout*, xyplot->hmajor_ticks);
    if(xyplot->h_legends_str) g_free(xyplot->h_legends_str);
    xyplot->h_legends_str = g_malloc( (sizeof(gchar*) * xyplot->hmajor_ticks ) );
    for (loop=0; loop<xyplot->hmajor_ticks; loop++){
      xyplot->h_legends_str[loop] = g_malloc( (sizeof(gchar) * (xyplot->x_legends_digits+1)) );
      snprintf(xyplot->h_legends_str[loop] , xyplot->x_legends_digits+1, "%lf", xyplot->xmin + (gdouble)loop*xyplot->d_hlegend);
      xyplot->h_legends_str[loop] [xyplot->x_legends_digits]='\0';
      xyplot->h_legends[loop]=gtk_widget_create_pango_layout (widget, xyplot->h_legends_str[loop]); 
    }
  }

  if ((xyplot->vmajor_ticks > 1) && (xyplot->y_legends_digits != 0) ){  
    xyplot->d_vlegend=fabs(xyplot->ymax-xyplot->ymin)/((gdouble)xyplot->vmajor_ticks-1.0);
 
    xyplot->v_legends=g_new(PangoLayout*, xyplot->vmajor_ticks);
    if(xyplot->v_legends_str) g_free(xyplot->v_legends_str);
    xyplot->v_legends_str=g_malloc( (sizeof(gchar*) * xyplot->vmajor_ticks)); 
    for (loop=0; loop<xyplot->vmajor_ticks; loop++){
      xyplot->v_legends_str[loop]=g_malloc( (sizeof(gchar) * (xyplot->y_legends_digits+1))); 
      snprintf(xyplot->v_legends_str[loop], xyplot->y_legends_digits+1, "%lf", xyplot->ymin + (gdouble)loop*xyplot->d_vlegend);
      xyplot->v_legends_str[loop][xyplot->y_legends_digits]='\0';
      xyplot->v_legends[loop]=gtk_widget_create_pango_layout (widget, xyplot->v_legends_str[loop]); 
    }
  }
  xyplot->h_label = get_pango_str(xyplot, xyplot->h_label_str);
  xyplot->v_label = get_pango_str(xyplot, xyplot->v_label_str);

  xyplot_calculate_legends_sizes(xyplot);
} 
/****************************************************************************************/
static void xyplot_free_legends(GabeditXYPlot *xyplot)
{
  gint loop;

  g_return_if_fail (xyplot != NULL);
    
  if (xyplot->h_legends!=NULL){
    for (loop=0; loop<xyplot->hmajor_ticks; loop++)
      if (xyplot->h_legends[loop]!=NULL)
        g_object_unref(G_OBJECT(xyplot->h_legends[loop]));
    g_free(xyplot->h_legends);
    xyplot->h_legends=NULL;    
  }

  if (xyplot->v_legends!=NULL){
    for (loop=0; loop<xyplot->vmajor_ticks; loop++)
      if (xyplot->v_legends[loop]!=NULL)
        g_object_unref(G_OBJECT(xyplot->v_legends[loop]));
    g_free(xyplot->v_legends);
    xyplot->v_legends=NULL;    
  }  
  if (xyplot->v_legends_str!=NULL) 
  {
    	for (loop=0; loop<xyplot->vmajor_ticks; loop++)
		g_free(xyplot->v_legends_str[loop]);
	 g_free(xyplot->v_legends_str);
  }
  if (xyplot->h_legends_str!=NULL)
  {
    	for (loop=0; loop<xyplot->hmajor_ticks; loop++)
		g_free(xyplot->h_legends_str[loop]);
	  g_free(xyplot->h_legends_str);
  }
  xyplot->v_legends_str = NULL;
  xyplot->h_legends_str = NULL;
}
/****************************************************************************************/
static void xyplot_build_points_data(GabeditXYPlot *xyplot, XYPlotData *data)
{
	GtkWidget *widget;
	if (xyplot == NULL) return;
	if(xyplot->data_list == NULL) return;
  
	widget=GTK_WIDGET(xyplot);
	if(data->point_pango != NULL) g_object_unref(G_OBJECT(data->point_pango));
	data->point_pango = gtk_widget_create_pango_layout (widget, data->point_str);
	if(data->point_pango)
	{
		PangoRectangle logical_rect;
		pango_layout_set_justify(data->point_pango,TRUE);
		pango_layout_set_alignment(data->point_pango,PANGO_ALIGN_CENTER);
		pango_layout_set_alignment(data->point_pango,PANGO_ALIGN_LEFT);
		pango_layout_set_markup(data->point_pango, data->point_str, -1);
		gchar fontName[100];
		PangoAttrList *attr_list;
		PangoFontDescription *font_desc = NULL;
		sprintf(fontName,"sans %d",2+data->point_size*2);
		font_desc = pango_font_description_from_string (fontName);
    		pango_layout_set_font_description (data->point_pango, font_desc);  
		pango_layout_context_changed(data->point_pango);
		pango_layout_get_pixel_extents(data->point_pango, NULL, &logical_rect);
    		pango_layout_get_size(data->point_pango, &(data->point_width), &(data->point_height));
		data->point_width/=PANGO_SCALE;
		data->point_height/=PANGO_SCALE;
		/* printf("%d %d\n",logical_rect.x,logical_rect.y);*/
		data->point_width =logical_rect.width;
		data->point_height =logical_rect.height;
		pango_font_description_free (font_desc);
		/* set color */
		attr_list = pango_layout_get_attributes (data->point_pango);
		if (attr_list == NULL) attr_list = pango_attr_list_new ();
		else pango_attr_list_ref (attr_list);
		pango_attr_list_insert(attr_list,
				pango_attr_foreground_new(
					data->point_color.red, 
					data->point_color.green, 
					data->point_color.blue)
				);
		pango_layout_set_attributes (data->point_pango, attr_list);
		pango_attr_list_unref (attr_list);

	}
}
/****************************************************************************************/
static void xyplot_reflect_x (GabeditXYPlot *xyplot, gboolean reflection)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if(xyplot->reflect_x == reflection) return; 

  xyplot->reflect_x = reflection;
  reset_object_text_pixels(xyplot);
  reset_object_line_pixels(xyplot);
  reset_object_image_pixels(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_reflect_x (GabeditXYPlot *xyplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (xyplot), "Manager");
	GtkAction* action = NULL;
	if(manager) action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderDirections/DirectionReflectX");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
static void xyplot_reflect_y (GabeditXYPlot *xyplot, gboolean reflection)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if(xyplot->reflect_y == reflection) return; 

  xyplot->reflect_y = reflection;
  reset_object_text_pixels(xyplot);
  reset_object_line_pixels(xyplot);
  reset_object_image_pixels(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_reflect_y (GabeditXYPlot *xyplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (xyplot), "Manager");
	GtkAction* action = NULL;
	if(manager) action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderDirections/DirectionReflectY");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
static void xyplot_show_left_legends (GabeditXYPlot *xyplot, gboolean show)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if(xyplot->show_left_legends == show) return; 

  xyplot->show_left_legends = show; 
  xyplot_build_legends(xyplot);
  xyplot_calculate_legends_sizes(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_show_left_legends (GabeditXYPlot *xyplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (xyplot), "Manager");
	GtkAction* action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderLegends/LegendShowLeft");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
static void xyplot_show_right_legends (GabeditXYPlot *xyplot, gboolean show)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if(xyplot->show_right_legends == show) return; 

  xyplot->show_right_legends = show; 
  xyplot_build_legends(xyplot);
  xyplot_calculate_legends_sizes(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_show_right_legends (GabeditXYPlot *xyplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (xyplot), "Manager");
	GtkAction* action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderLegends/LegendShowRight");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
static void xyplot_show_top_legends (GabeditXYPlot *xyplot, gboolean show)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if(xyplot->show_top_legends == show) return; 

  xyplot->show_top_legends = show; 
  xyplot_build_legends(xyplot);
  xyplot_calculate_legends_sizes(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_show_top_legends (GabeditXYPlot *xyplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (xyplot), "Manager");
	GtkAction* action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderLegends/LegendShowTop");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
static void xyplot_show_bottom_legends (GabeditXYPlot *xyplot, gboolean show)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if(xyplot->show_bottom_legends == show) return; 

  xyplot->show_bottom_legends = show; 
  xyplot_build_legends(xyplot);
  xyplot_calculate_legends_sizes(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_show_bottom_legends (GabeditXYPlot *xyplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (xyplot), "Manager");
	GtkAction* action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderLegends/LegendShowBottom");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
static void xyplot_show_rectangle_legends (GabeditXYPlot *xyplot, gboolean show)
{
  g_return_if_fail (xyplot != NULL);
  g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
  
  if(xyplot->show_rectangle_legends == show) return; 

  xyplot->show_rectangle_legends = show; 
  xyplot_build_legends(xyplot);
  xyplot_calculate_legends_sizes(xyplot);
  xyplot_calculate_sizes(xyplot);
  gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_show_rectangle_legends (GabeditXYPlot *xyplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (xyplot), "Manager");
	GtkAction* action = gtk_ui_manager_get_action (manager, "/MenuXYPlot/Render/RenderLegends/LegendShowRectangle");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
void gabedit_xyplot_set_font (GabeditXYPlot *xyplot, gchar* fontName)
{
	GtkWidget* widget = NULL;
	g_return_if_fail (xyplot != NULL);
	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
	PangoFontDescription *font_desc = pango_font_description_from_string (fontName);

	widget=GTK_WIDGET(xyplot);

	if(font_desc)
	{
		xyplot->font_size =  xyplot_get_font_size (widget, font_desc);
		gtk_widget_modify_font (widget, font_desc);  
  		xyplot_build_legends(xyplot);
  		xyplot_calculate_legends_sizes(xyplot);
  		xyplot_calculate_sizes(xyplot);
		gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
	}
}
/****************************************************************************************/
void gabedit_xyplot_set_x_label (GabeditXYPlot *xyplot, G_CONST_RETURN gchar* str)
{
	GtkWidget* widget = NULL;
	g_return_if_fail (xyplot != NULL);
	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
	widget=GTK_WIDGET(xyplot);

  	if(xyplot->h_label_str) g_free(xyplot->h_label_str);
	if(str && strlen(str)>0)
	{
		xyplot->h_label_str = g_strdup(str);
	}
	else
	{
		xyplot->h_label_str = NULL;
	}

  	xyplot_build_legends(xyplot);
  	xyplot_calculate_legends_sizes(xyplot);
  	xyplot_calculate_sizes(xyplot);
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_y_label (GabeditXYPlot *xyplot, G_CONST_RETURN gchar* str)
{
	GtkWidget* widget = NULL;
	g_return_if_fail (xyplot != NULL);
	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
	widget=GTK_WIDGET(xyplot);

  	if(xyplot->v_label_str) g_free(xyplot->v_label_str);
	if(str && strlen(str)>0)
	{
		xyplot->v_label_str = g_strdup(str);
	}
	else
		xyplot->v_label_str = NULL;

  	xyplot_build_legends(xyplot);
  	xyplot_calculate_legends_sizes(xyplot);
  	xyplot_calculate_sizes(xyplot);
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
		snprintf(str, 50, "Mouse position: %f, %f", xv, yv);
	else
		sprintf(str, " ");
	context_id=gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "mouse position");
	gtk_statusbar_push (GTK_STATUSBAR(statusbar), context_id, str);
    
	return TRUE;
}
/****************************************************************************************/
GtkWidget* gabedit_xyplot_new_window(gchar* title, GtkWidget*parent)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* table = NULL;
	GtkWidget* box = NULL;
	GtkWidget* frame_xyplot = NULL;
	GtkWidget* xyplot = NULL;

	GtkWidget *statusbar = NULL;

	gtk_window_set_title (GTK_WINDOW (window), title);
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", G_CALLBACK (gtk_widget_destroy), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 2);
	gtk_window_set_default_size (GTK_WINDOW(window),2*gdk_screen_width()/3,2*gdk_screen_height()/3);
	
	table=gtk_table_new(3, 1, FALSE);
	gtk_container_add(GTK_CONTAINER(window), table);
	gtk_widget_show(table);
  
	frame_xyplot=gtk_frame_new("XY GabeditPlot");
	gtk_table_attach(GTK_TABLE(table), frame_xyplot, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_widget_show(frame_xyplot);
  
	xyplot = gabedit_xyplot_new();
	gtk_container_add(GTK_CONTAINER(frame_xyplot), xyplot);
	gtk_widget_show (xyplot);

	g_object_set_data (G_OBJECT (window), "XYPLOT",xyplot);

	box=gtk_hbox_new(FALSE,0);
	gtk_table_attach(GTK_TABLE(table), box, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 5, 5);
	gtk_widget_show (box);
	g_object_set_data(G_OBJECT (xyplot), "Box", box);

	statusbar=gtk_statusbar_new();
	gtk_table_attach(GTK_TABLE(table), statusbar, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 5, 5);
	gtk_widget_show (statusbar);
	g_object_set_data(G_OBJECT (xyplot), "StatusBar", statusbar);

	if(parent) gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parent));
	gtk_widget_show (window);

        gabedit_xyplot_set_range(GABEDIT_XYPLOT(xyplot),  0.0,  10,  0,  20);
	gabedit_xyplot_set_x_legends_digits(GABEDIT_XYPLOT(xyplot), 5);
	gabedit_xyplot_set_y_legends_digits(GABEDIT_XYPLOT(xyplot), 5);



	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_HMAJOR_GRID, FALSE);
	gabedit_xyplot_enable_grids (GABEDIT_XYPLOT(xyplot), GABEDIT_XYPLOT_VMAJOR_GRID, FALSE);
	gabedit_xyplot_show_top_legends (GABEDIT_XYPLOT(xyplot),FALSE);
	gabedit_xyplot_show_right_legends (GABEDIT_XYPLOT(xyplot),FALSE);

	g_signal_connect_after(G_OBJECT(xyplot), "motion-notify-event", G_CALLBACK(xyplot_motion_notify_event), NULL);

	g_object_set_data(G_OBJECT (xyplot), "Window", window);
	g_object_set_data(G_OBJECT (window), "Box", box);

	gabedit_xyplot_set_font (GABEDIT_XYPLOT(xyplot), "sans 12");

	return window;
}
/****************************************************************************************/
void gabedit_xyplot_add_data_peaks(GabeditXYPlot *xyplot, gint numberOfPoints, gdouble* X,  gdouble* Y, GdkColor* color)
{
	g_return_if_fail (xyplot != NULL);
	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));
	g_return_if_fail (X != NULL);
	g_return_if_fail (Y != NULL);
	if(numberOfPoints>0)
	{
		gint loop;
		XYPlotData *data = g_malloc(sizeof(XYPlotData));
		gint red = 0;
		gint green = 0;
		gint blue = 0;
		gdouble xmin = 0;
		gdouble xmax = 0;

		if(color)
		{
			red = (color->red);
			green = (color->green);
			blue = (color->blue);
		}
		else
		{
			GdkColor c = get_fore_color(xyplot);
			red = c.red;
			green = c.green;
			blue = c.blue;
		}

		data->size=3*numberOfPoints+2;
		data->x = g_malloc(data->size*sizeof(gdouble)); 
		data->y = g_malloc(data->size*sizeof(gdouble)); 

		xmin = X[0];
		xmax = X[0];
		for(loop = 1; loop<numberOfPoints;loop++)
		{
			if(xmin>X[loop]) xmin = X[loop];
			if(xmax<X[loop]) xmax = X[loop];
		}

		data->x[0]=xmin;
		data->y[0]=0;
		data->x[data->size-1]=xmax;
		data->y[data->size-1]=0;
		for (loop=0; loop<numberOfPoints; loop++){
			gint iold = loop*3+1;
			data->x[iold]=X[loop];
			data->y[iold]=0;

			data->x[iold+1]=X[loop];
			data->y[iold+1]=Y[loop];

			data->x[iold+2]=X[loop];
			data->y[iold+2]=0;
		}

		sprintf(data->point_str,"+");
		data->point_pango = NULL;
		xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), data);

		data->point_size=0;
		data->line_width=2;
		data->point_color.red=green; 
		data->point_color.green=red; 
		data->point_color.blue=blue; 

		data->line_color.red=red;
		data->line_color.green=green;
		data->line_color.blue=blue;
		data->line_style=GDK_LINE_SOLID;
		gabedit_xyplot_add_data (GABEDIT_XYPLOT(xyplot), data);
		gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(xyplot), NULL);
	}
}
/****************************************************************************************/
static void xyplot_curve_noconv(GabeditXYPlot *xyplot, gint numberOfPoints, gdouble* X,  gdouble* Y, GdkColor* color)
{
	gint loop;
	gint red = 0;
	gint green = 0;
	gint blue = 0;
	XYPlotData *data = g_malloc(sizeof(XYPlotData));

	if(color)
	{
		red = (color->red);
		green = (color->green);
		blue = (color->blue);
	}
	else
	{
		GdkColor c = get_fore_color(xyplot);
		red = c.red;
		green = c.green;
		blue = c.blue;
	}
	
	data->size=numberOfPoints;
	if(data->size>0)
	{
		data->x = g_malloc(data->size*sizeof(gdouble)); 
		data->y = g_malloc(data->size*sizeof(gdouble)); 
	}
	
	for (loop=0; loop<data->size; loop++){
		data->x[loop]=X[loop];
		data->y[loop]=Y[loop];
	}
	sprintf(data->point_str,"+");

	data->point_size=0;
	data->line_width=2;
	data->point_color.red=red; 
	data->point_color.green=green; 
	data->point_color.blue=blue; 

	data->line_color.red=green;
	data->line_color.green=red;
	data->line_color.blue=blue;
	data->line_style=GDK_LINE_SOLID;
	gabedit_xyplot_add_data (GABEDIT_XYPLOT(xyplot), data);
	gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(xyplot), NULL);

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
static void build_data_xyplot_curve_withconv(GabeditXYPlot *xyplot, gint numberOfPoints, gdouble* X,  gdouble* Y, gdouble halfWidth, gdouble (*lineshape)(gdouble), GdkColor* color)
{
	gint i;
	gint j;
	gint n = 0;
        gdouble xx ;
        gdouble h0 = halfWidth/20;
	gint red = 0;
	gint green = 0;
	gint blue = 0;
	XYPlotData *data = g_malloc(sizeof(XYPlotData));
	gdouble xmin = 0;
	gdouble xmax = 0;

	if(color)
	{
		red = (color->red);
		green = (color->green);
		blue = (color->blue);
	}
	else
	{
		GdkColor c = get_fore_color(xyplot);
		red = c.red;
		green = c.green;
		blue = c.blue;
	}
	
	data->size=0;
	data->point_size=0;
	data->line_width=2;
	data->point_color.red=green; 
	data->point_color.green=red; 
	data->point_color.blue=blue; 

	data->line_color.red=red;
	data->line_color.green=green;
	data->line_color.blue=blue;
	data->line_style=GDK_LINE_SOLID;
	
	data->x = NULL;
	data->y = NULL;

	if(numberOfPoints<1||!X||!Y) return;
	
	xmin = X[0];
	xmax = X[0];
	for (i=0; i<numberOfPoints; i++){
		if(X[i]<xmin) xmin = X[i];
		if(X[i]>xmax) xmax = X[i];
	}

        xmin -= 10*halfWidth;
        xmax += 10*halfWidth;
        xx = xmin;
        h0 = halfWidth/10;
	n = (gint)((xmax-xmin)/h0+0.5)+numberOfPoints;
	if(n>0) data->x = (gdouble*)g_malloc(sizeof(gdouble)*n);
	if(numberOfPoints>0 && n>0)
	do
	{
		gdouble dmin = 0.0;
		gdouble d = 0.0;
		gint jmin = 0;
		for (j=0; j < numberOfPoints; j++)
		{
			gdouble center = (gdouble) X[j];
			d = fabs(xx - center);
			if(d<dmin || j==0) 
			{
				jmin = j;
				dmin = d;
			}
		}
		data->x[data->size] = xx;
		if(dmin<h0)
		{
			if(xx< X[jmin])
			{
				xx = (gdouble) X[jmin];
				data->x[data->size] = xx;
				xx += h0+1e-8;
			}
			else
			{
				xx = (gdouble) X[jmin];
				data->x[data->size] = xx;
				xx += h0+1e-8;
			}
		}
		else
		{
			if(dmin> 5*halfWidth) xx += h0+dmin/5;
			else xx += h0;
		}
		data->size++;
	}while(xx<xmax && data->size<n);

	if(data->size>0) 
	{
		data->x = (gdouble*)g_realloc(data->x,sizeof(gdouble)*data->size);
		data->y = (gdouble*)g_malloc(sizeof(gdouble)*data->size);
	}

	if(numberOfPoints>0)
	for (i=0; i < data->size; i++)
	{
		gdouble yy = 0.0;
		for (j=0; j < numberOfPoints; j++)
		{
			gdouble center = (gdouble) X[j];
			gdouble rel_offset = (data->x[i] - center) / halfWidth;
			yy += Y[j]*lineshape(rel_offset);
		}
		data->y[i] = yy;
	}

	sprintf(data->point_str,"+");
	data->point_pango = NULL;
	xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), data);
	gabedit_xyplot_add_data (GABEDIT_XYPLOT(xyplot), data);
	gabedit_xyplot_set_autorange(GABEDIT_XYPLOT(xyplot), NULL);
}
/****************************************************************************************/
void gabedit_xyplot_add_data_conv(GabeditXYPlot *xyplot, gint numberOfPoints, gdouble* X,  gdouble* Y, gdouble halfWidth,GabeditXYPlotConvType convType, GdkColor* color)
{
	switch(convType)
	{
		case GABEDIT_XYPLOT_CONV_NONE :
			xyplot_curve_noconv(xyplot, numberOfPoints, X,  Y, color);
			break;
		case GABEDIT_XYPLOT_CONV_LORENTZ :
			build_data_xyplot_curve_withconv(xyplot, numberOfPoints, X,  Y, halfWidth, lorentzianLineshape, color);
			break;
		case GABEDIT_XYPLOT_CONV_GAUSS :
			build_data_xyplot_curve_withconv(xyplot, numberOfPoints, X,  Y, halfWidth, gaussianLineshape, color);
	}

}
/****************************************************************************************/
void gabedit_xyplot_help()
{
	GtkWidget* dialog = NULL;
	GtkWidget *label, *button;
	GtkWidget *frame, *vboxframe;
	GtkWidget* hbox;

	gchar* tmp = g_strdup_printf(
			_(
			"Mouse buttons and Keys :\n"
			"\t Right button : popup menu\n"
			"\t Middle button : zoom\n"
			"\t Left button + Control key : zoom\n"
			"\t Left button + t key : insert a text\n"
			"\t Left button + l key : insert a line (arrow)\n"
			"\t Left button + i key : insert an image from a png file\n"
			"\t Left button + r key : remove selected image/text/line\n"
			"\t Ctrl + V (Alt + V) : insert an image from clipboard\n"
			"\t Left button + Shift key : compute distance between 2 points\n"
			"\t Left button, double click : select a data to change\n"
			"\t Left button clicked on bottom right corner + move : change the width/height for an image\n"
			"\n\n"
			"Popup menu :\n"
			"\t Set : ticks, margins, ranges, X and Y labels, digits, font size, auto ranges\n"
			"\t Render : grids, directions, legends\n"
			"\t Data : read data from multiple format(txt 2 or more, JDX, jMRUI)\n"
			"\t        A backspace ligne in a txt file is interpreted as a beginning new data\n"
			"\t        Save all data at a txt file\n"
			"\t        Remove all all\n"
			"\t        change data (scale, shift, ...)\n"
			"\t Objects : for insert/delete texts, lines or images\n"
			"\t Screen capture : BMP, JPEG, PNG, Transparent PNG, TIF\n"
			"\t Export image : SVG, PS, EPS and PDF (this is a real export, not a capture)\n"
			"\t Read : read all (parameters, data, texts, lines, images) from a Gabedit file\n"
			"\t Save : save all (parameters, data, texts, lines, images) in Gabedit file\n"
			"\t Help : for obtain this window (You guessed :))\n"
			"\t Close : very simple :)\n"
			)
			);

	dialog = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dialog));

	gtk_window_set_title(GTK_WINDOW(dialog),_("Help"));
	g_signal_connect(G_OBJECT(dialog), "delete_event", (GCallback)gtk_widget_destroy, NULL);
	frame = gtk_frame_new (NULL);
	gtk_frame_set_shadow_type( GTK_FRAME(frame),GTK_SHADOW_ETCHED_OUT);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_box_pack_start( GTK_BOX(GTK_DIALOG(dialog)->vbox), frame,TRUE,TRUE,0);

	gtk_widget_show (frame);
  	vboxframe = gtk_vbox_new (FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vboxframe);
    	label = gtk_label_new(tmp);
  	gtk_widget_show (label);
   	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
  	hbox = gtk_hbox_new (FALSE, 5);
  	gtk_box_pack_start (GTK_BOX (vboxframe), hbox, FALSE, FALSE, 5);
  	gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 5);
  	gtk_widget_show (hbox);
    
	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(dialog)->action_area), FALSE);
	button = gtk_button_new_from_stock (GTK_STOCK_OK);
	gtk_box_pack_end (GTK_BOX( GTK_DIALOG(dialog)->action_area), button, FALSE, TRUE, 5);  
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",
                          (GCallback)gtk_widget_destroy,
                          GTK_OBJECT(dialog));

	gtk_widget_show_all(dialog);
       gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
       g_free(tmp);
}
/****************************************************************************************/
void gabedit_xyplot_set_data_line_width (GabeditXYPlot *xyplot, gdouble line_width)
{
	g_return_if_fail (xyplot != NULL);
	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

	if(line_width<0) line_width = 0;
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
	XYPlotData *current_data; 
	GList *current_node; 
 
	if (xyplot->data_list ){
		current_node=g_list_first(xyplot->data_list);
		current_data=(XYPlotData*)current_node->data;  
		for (; current_node!=NULL; current_node=current_node->next)
		{
        		current_data=(XYPlotData*)current_node->data;  
			current_data->line_width = line_width;
		}
	}
	xyplot_free_legends(xyplot);
	xyplot_build_legends(xyplot);  
	xyplot_calculate_sizes(xyplot);

	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_data_point_size (GabeditXYPlot *xyplot, gdouble point_size)
{
	g_return_if_fail (xyplot != NULL);
	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

	if(point_size<0) point_size = 0;
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
	XYPlotData *current_data; 
	GList *current_node; 
 
	if (xyplot->data_list ){
		current_node=g_list_first(xyplot->data_list);
		current_data=(XYPlotData*)current_node->data;  
		for (; current_node!=NULL; current_node=current_node->next)
		{
        		current_data=(XYPlotData*)current_node->data;  
			current_data->point_size = point_size;
			xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), current_data);
		}
	}
	xyplot_free_legends(xyplot);
	xyplot_build_legends(xyplot);  
	xyplot_calculate_sizes(xyplot);

	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_data_point_type (GabeditXYPlot *xyplot, gchar c)
{
	g_return_if_fail (xyplot != NULL);
	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
	XYPlotData *current_data; 
	GList *current_node; 
 
	if (xyplot->data_list ){
		current_node=g_list_first(xyplot->data_list);
		current_data=(XYPlotData*)current_node->data;  
		for (; current_node!=NULL; current_node=current_node->next)
		{
        		current_data=(XYPlotData*)current_node->data;  
			sprintf(current_data->point_str,"%c",c);
			xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), current_data);
		}
	}
	xyplot_free_legends(xyplot);
	xyplot_build_legends(xyplot);  
	xyplot_calculate_sizes(xyplot);

	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_data_point_color (GabeditXYPlot *xyplot, gdouble red, gdouble green, gdouble blue)
{
	g_return_if_fail (xyplot != NULL);
	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
	XYPlotData *current_data; 
	GList *current_node; 
 
	if (xyplot->data_list ){
		current_node=g_list_first(xyplot->data_list);
		current_data=(XYPlotData*)current_node->data;  
		for (; current_node!=NULL; current_node=current_node->next)
		{
        		current_data=(XYPlotData*)current_node->data;  
			current_data->point_color.red = SCALE2(red);
			current_data->point_color.green = SCALE2(green);
			current_data->point_color.blue = SCALE2(blue);
			xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), current_data);
		}
	}
	xyplot_free_legends(xyplot);
	xyplot_build_legends(xyplot);  
	xyplot_calculate_sizes(xyplot);

	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/********************************************************************************/
void gabedit_xyplot_add_new_data(GtkWidget* xyplot, gint numberOfPoints, gdouble* X,  gdouble* Y)
{
	add_new_data(xyplot, numberOfPoints, X,  Y);
}
/****************************************************************************************/
void gabedit_xyplot_set_last_data_line_width (GabeditXYPlot *xyplot, gdouble line_width)
{
	g_return_if_fail (xyplot != NULL);
	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

	if(line_width<0) line_width = 0;
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
	XYPlotData *current_data; 
	GList *current_node; 
 
	if (xyplot->data_list ){
		current_node=g_list_first(xyplot->data_list);
		current_data=(XYPlotData*)current_node->data;  
		for (; current_node!=NULL; current_node=current_node->next)
		{
        		current_data=(XYPlotData*)current_node->data;  
		}
		current_data->line_width = line_width;
	}
	xyplot_free_legends(xyplot);
	xyplot_build_legends(xyplot);  
	xyplot_calculate_sizes(xyplot);

	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_set_last_data_point_size (GabeditXYPlot *xyplot, gdouble point_size)
{
	g_return_if_fail (xyplot != NULL);
	g_return_if_fail (GABEDIT_IS_XYPLOT (xyplot));

	if(point_size<0) point_size = 0;
	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
	XYPlotData *current_data; 
	GList *current_node; 
 
	if (xyplot->data_list ){
		current_node=g_list_first(xyplot->data_list);
		current_data=(XYPlotData*)current_node->data;  
		for (; current_node!=NULL; current_node=current_node->next)
		{
        		current_data=(XYPlotData*)current_node->data;  
		}
		current_data->point_size = point_size;
		xyplot_build_points_data(GABEDIT_XYPLOT(xyplot), current_data);
	}
	xyplot_free_legends(xyplot);
	xyplot_build_legends(xyplot);  
	xyplot_calculate_sizes(xyplot);

	gtk_widget_queue_draw(GTK_WIDGET(xyplot));  
}
/****************************************************************************************/
void gabedit_xyplot_add_object_text (GabeditXYPlot *xyplot, gdouble x, gdouble y, gdouble angle, G_CONST_RETURN gchar* str)
{
	add_object_text(GABEDIT_XYPLOT(xyplot),  x,  y,  angle, str);
}
