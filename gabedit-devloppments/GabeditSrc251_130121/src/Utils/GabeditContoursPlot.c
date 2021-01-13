/* GabeditContoursPlot.c */
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

#include "GabeditContoursPlot.h"
#include "Interpolation.h"

#define ContoursPLOT_DEFAULT_SIZE 300
#define BSIZE 1024
#define SCALE(i) (i / 65535.)
#define SCALE2(i) (i * 65535.)

typedef enum
{
  JDX_TYPE_ContoursDATA,
  JDX_TYPE_ContoursPOINTS,
  JDX_TYPE_ContoursTABLE,
  JDX_TYPE_UNKNOWN
} JDXType;

/****************************************************************************************/
static void gabedit_contoursplot_cairo_string(cairo_t* cr, GtkWidget *widget, GdkGC* gc, gint x, gint y, G_CONST_RETURN gchar* str, gboolean centerX, gboolean centerY, gdouble angle);
static void gabedit_contoursplot_cairo_line(cairo_t* cr, GtkWidget *widget, GdkGC* gc, gdouble x1,gdouble y1,gdouble x2,gdouble y2);
static void gabedit_contoursplot_cairo_lines(cairo_t *cr,  GtkWidget* widget, GdkGC* gc, GdkPoint* points, gint size);
static void gabedit_contoursplot_cairo_rectangle(cairo_t* cr, GtkWidget *widget, GdkGC* gc, gboolean fill, gdouble x1,gdouble y1,gdouble w,gdouble h);
/* static void gabedit_contoursplot_cairo_cercle(cairo_t* cr, GtkWidget *widget, GdkGC* gc, gint xc,gint yc,gint rayon);*/

static void contoursplot_cairo_string(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gint x, gint y, G_CONST_RETURN gchar* str, gboolean centerX, gboolean centerY, gdouble angle);
static void contoursplot_cairo_image(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget *widget, gint x, gint y, gint w, gint h, cairo_surface_t *image);
static void contoursplot_cairo_line(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gdouble x1,gdouble y1,gdouble x2,gdouble y2);
static void contoursplot_cairo_lines(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, GdkPoint* points, gint size);
static void contoursplot_cairo_rectangle(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gboolean fill, gdouble x1,gdouble y1,gdouble w,gdouble h);
/* static void contoursplot_cairo_cercle(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gint xc,gint yc,gint rayon);*/

static void gabedit_contoursplot_class_init (GabeditContoursPlotClass    *klass);
static void gabedit_contoursplot_init (GabeditContoursPlot         *contoursplot);
static void gabedit_contoursplot_destroy (GtkObject        *object);
static void gabedit_contoursplot_realize (GtkWidget        *widget);
static void gabedit_contoursplot_size_request (GtkWidget      *widget, GtkRequisition *requisition);
static void gabedit_contoursplot_size_allocate (GtkWidget     *widget, GtkAllocation *allocation);
static gint gabedit_contoursplot_expose (GtkWidget        *widget, GdkEventExpose   *event);
static gint gabedit_contoursplot_button_press (GtkWidget   *widget, GdkEventButton   *event);
static gint gabedit_contoursplot_button_release (GtkWidget *widget, GdkEventButton   *event);
static gint gabedit_contoursplot_motion_notify (GtkWidget  *widget, GdkEventMotion   *event);
static gint gabedit_contoursplot_key_press(GtkWidget* widget, GdkEventKey *event);
static gint gabedit_contoursplot_key_release(GtkWidget* widget, GdkEventKey *event);
static gint gabedit_contoursplot_scroll (GtkWidget        *widget, GdkEventScroll   *event);
static void gabedit_contoursplot_style_set (GtkWidget      *widget, GtkStyle       *previous_style);
static gint gabedit_contoursplot_grab(GtkWidget* widget, GdkEventCrossing* event);
static void gabedit_contoursplot_cairo_layout(cairo_t* cr, gdouble x, gdouble y, PangoLayout *layout, gboolean centerX, gboolean centerY, gdouble angle) ;
static void contoursplot_enable_grids (GabeditContoursPlot *contoursplot, GabeditContoursPlotGrid grid, gboolean enable);
static void contoursplot_show_colormap (GabeditContoursPlot *contoursplot, gboolean show);
static void contoursplot_show_label_contours (GabeditContoursPlot *contoursplot, gboolean show);
static void contoursplot_build_position_label_contours (GabeditContoursPlot *contoursplot);
static void reset_label_contours(GtkWidget* contoursplot, ContoursPlotData* data);
static void contoursplot_dashed_negative_contours (GabeditContoursPlot *contoursplot, gboolean dashed);
static void contoursplot_show_left_legends (GabeditContoursPlot *contoursplot, gboolean show);
static void contoursplot_show_right_legends (GabeditContoursPlot *contoursplot, gboolean show);
static void contoursplot_show_top_legends (GabeditContoursPlot *contoursplot, gboolean show);
static void contoursplot_show_bottom_legends (GabeditContoursPlot *contoursplot, gboolean show);
static void contoursplot_reflect_x (GabeditContoursPlot *contoursplot, gboolean enable);
static void contoursplot_reflect_y (GabeditContoursPlot *contoursplot, gboolean enable);
static gint gabedit_contoursplot_draw (GtkWidget *widget);
static void set_old_area(GtkWidget *widget, GabeditContoursPlot *contoursplot);

/****************************************************************************************/
static GtkWidgetClass *parent_class = NULL; /* TO DELETE */
/****************************************************************************************/
static void contoursplot_calculate_sizes (GabeditContoursPlot *contoursplot);
static void value2pixel(GabeditContoursPlot *contoursplot, gdouble xv, gdouble yv, gint *x, gint *y);
static void pixel2value(GabeditContoursPlot *contoursplot, gint xp, gint yp, gdouble *x, gdouble *y);
static void contoursplot_calculate_legends_sizes(GabeditContoursPlot *contoursplot);
static void contoursplot_build_legends(GabeditContoursPlot *contoursplot);
static void contoursplot_free_legends(GabeditContoursPlot *contoursplot);
static void contoursplot_build_points_contour(GabeditContoursPlot *contoursplot, ContourData *contour);
static void contoursplot_calculate_colormap_sizes(GabeditContoursPlot *contoursplot);
static void contoursplot_build_colormap_legends(GabeditContoursPlot *contoursplot);
static gint get_distance_M_AB(GabeditContoursPlot *contoursplot,gint xM, gint yM, gint ixA, gint iyA, gint ixB, gint iyB);
static void saveAsGabeditDlg(GtkWidget* contoursplot);
static void readAGabeditDlg(GtkWidget* contoursplot);
static void reset_theme(GtkWidget *widget, gint line_width, GdkColor* foreColor, GdkColor* backColor );
static void set_theme_publication(GtkWidget *widget);
static void set_theme_green_black(GtkWidget *widget);
static void set_theme_dialog(GtkWidget* widget);
static PangoLayout* get_pango_str(GabeditContoursPlot *contoursplot, G_CONST_RETURN gchar* txt);

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
static void contoursplot_message(gchar* message)
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
		gdouble arrow_length,
		gdouble start_x, gdouble start_y, 
		gdouble end_x, gdouble end_y, 
		gdouble* x1, gdouble* y1, 
		gdouble* x2, gdouble* y2
		)
{
	gdouble angle = atan2 (end_y - start_y, end_x - start_x) + M_PI;
	arrow_degrees = arrow_degrees/180.0*M_PI;

	*x1 = end_x + arrow_length * cos(angle - arrow_degrees);
	*y1 = end_y + arrow_length * sin(angle - arrow_degrees);
	*x2 = end_x + arrow_length * cos(angle + arrow_degrees);
	*y2 = end_y + arrow_length * sin(angle + arrow_degrees);
}
/**********************************************************************************/
static void colormap_free(ColorMap* colorMap)
{
	if(!colorMap) return;
	if(!colorMap->colorValue)
	{

		colorMap->numberOfColors = 0;
		return;
	}
	g_free(colorMap->colorValue);
	colorMap->numberOfColors = 0;
}
/******************************************************************************************************************************/
static void colormap_alloc(ColorMap* colorMap, gint numberOfColors)
{
	if(colorMap->colorValue)
		colormap_free(colorMap);

	if(numberOfColors<1) numberOfColors = 1;
	colorMap->numberOfColors = numberOfColors;
	colorMap->colorValue = g_malloc(numberOfColors*sizeof(OneColorValue));
}
/******************************************************************************************************************************/
static ColorMap new_colorMap(gdouble color[], gdouble value)
{
	gint c;
	ColorMap colorMap = {0,NULL};
	colormap_alloc(&colorMap, 1);
	for(c=0; c<3; c++)
		colorMap.colorValue[0].color[c] = color[c];
	colorMap.colorValue[0].value = value;
	return colorMap;
}
/******************************************************************************************************************************/
static void add_to_colorMap(ColorMap* colorMap, gdouble color[], gdouble value)
{
	gint i;
	gint c;
	OneColorValue* colorValue = NULL;
	colorValue = g_malloc((colorMap->numberOfColors+1)*sizeof(OneColorValue));
	for(i=0; i<colorMap->numberOfColors; i++)
		colorValue[i] = colorMap->colorValue[i];

	for(c=0;c<3;c++)
		colorValue[colorMap->numberOfColors].color[c] = color[c];
	colorValue[colorMap->numberOfColors].value = value;
	g_free( colorMap->colorValue);
	colorMap->colorValue = colorValue;
	colorMap->numberOfColors++;
}
/******************************************************************************************************************************/
static void sort_colorMap(ColorMap* colorMap)
{
	gint i;
	gint j;
	gint k;
	OneColorValue tmp;
	for(i=0;i<colorMap->numberOfColors-1;i++)
	{
		k = i;
		for(j=i+1;j<colorMap->numberOfColors;j++)
			if(colorMap->colorValue[k].value>colorMap->colorValue[j].value)
				k = j;
		if(k != i)
		{
			tmp = colorMap->colorValue[i];
			colorMap->colorValue[i] = colorMap->colorValue[k];
			colorMap->colorValue[k] = tmp;
		}
	}
}
/******************************************************************************************************************************/
static ColorMap*  new_colorMap_min_max_multicolor(gdouble minValue, gdouble maxValue)
{
	gint i;
	gint j;
	gint k;
	gdouble h = 0;
	gint ns = 4;
	gint nc = 20;
	gint n = nc *ns+1;
	gdouble color[3];
	ColorMap* colorMap;

	h = (maxValue-minValue)/(n-1) ;
	color[0] = 1.0; color[1] = 0.0; color[2] = 0.0; 

	colorMap = g_malloc(sizeof(ColorMap));
	*colorMap = new_colorMap(color, minValue);
	k = 1;
	for(j=0;j<ns;j++)
	for(i=0;i<nc;i++)
	{
		
		if(j==0) color[1] += 1.0/nc;
		if(j==1) color[0] -= 1.0/nc;
		if(j==2) color[2] += 1.0/nc;
		if(j==3) color[1] -= 1.0/nc;
		add_to_colorMap(colorMap, color, minValue +k*h);
		k++;
	}
	sort_colorMap(colorMap);
	return colorMap;
}
/******************************************************************************************************************************/
static ColorMap*  new_colorMap_min_max_unicolor(gdouble minValue, gdouble maxValue, gdouble Color[])
{
	gdouble color[3];
	ColorMap* colorMap;
	gint c;

	for(c=0;c<3;c++) color[c] = Color[c];
	colorMap = g_malloc(sizeof(ColorMap));
	*colorMap = new_colorMap(color, minValue);
	add_to_colorMap(colorMap, color, maxValue );
	sort_colorMap(colorMap);
	return colorMap;
}
/******************************************************************************************************************************/
static ColorMap*  new_colorMap_min_max_2colors(gdouble minValue, gdouble maxValue, gdouble Color1[], gdouble Color2[])
{
	gdouble color[3];
	gdouble color1[3];
	gdouble color2[3];
	ColorMap* colorMap;
	gint c;

	gdouble max1;
	gdouble max2;

	for(c=0;c<3;c++) color1[c] = Color1[c];
	for(c=0;c<3;c++) color2[c] = Color2[c];

	max1 = color1[0];
	if(max1<color1[1]) max1 = color1[1];
	if(max1<color1[2]) max1 = color1[2];
	if(max1<1e-3)
	{
		color1[0] = 1.0;
		color1[1] = 1.0;
		color1[2] = 1.0;
		max1 = 1.0;
	}
	max2 = color2[0];
	if(max2<color2[1]) max2 = color2[1];
	if(max2<color2[2]) max2 = color2[2];
	if(max2<1e-3)
	{
		color2[0] = 1.0;
		color2[1] = 1.0;
		color2[2] = 1.0;
		max2 = 1.0;
	}

	color[0] = 0; color[1] = 0; color[2] = 0; 
	for(c=0;c<3;c++) color1[c] /= max1;
	for(c=0;c<3;c++) color2[c] /= max2;
	colorMap = g_malloc(sizeof(ColorMap));
	*colorMap = new_colorMap(color1, minValue);
	add_to_colorMap(colorMap, color, minValue+(maxValue-minValue)/2 );
	add_to_colorMap(colorMap, color2, maxValue );

	sort_colorMap(colorMap);
	return colorMap;
}
/************************************************************************************************************/
static ColorMap*  new_colorMap_min_max(gdouble minValue, gdouble maxValue, gint colorMapType, gdouble colorMapColors[3][3])
{
	if(colorMapType == 1) return new_colorMap_min_max_multicolor(minValue,maxValue);
	else if(colorMapType == 2)
	return new_colorMap_min_max_2colors(minValue,maxValue, colorMapColors[0], colorMapColors[1]);
	else return new_colorMap_min_max_unicolor(minValue,maxValue, colorMapColors[2]);
}
/******************************************************************************************************************************/
static void  set_Color_From_colorMap(ColorMap* colorMap, gdouble color[], gdouble value)
{
	gint i = 0;
	gint k = 0;
	gint c;
	gdouble l,l1,l2;
	if(colorMap->numberOfColors<1)
	{
		for(c=0;c<3;c++)
			color[c] = 0.5;
		return;
	}
	if(value<=colorMap->colorValue[i].value)
	{
		for(c=0;c<3;c++)
			color[c] = colorMap->colorValue[i].color[c];
		return;
	}
	i = colorMap->numberOfColors-1;
	if(value>=colorMap->colorValue[i].value)
	{
		for(c=0;c<3;c++)
			color[c] = colorMap->colorValue[i].color[c];
		return;
	}

	k = 0;
	for(i=1;i<colorMap->numberOfColors-1;i++)
	{
		if(value>=colorMap->colorValue[i].value && value<=colorMap->colorValue[i+1].value)
		{
			k = i;
			break;
		}
	}
	l = colorMap->colorValue[k+1].value-colorMap->colorValue[k].value;
	l1 = value-colorMap->colorValue[k].value;
	l2 = l-l1;
	if(l>0 && l1>=0 && l2>=0)
	{
		l1 = l1/l;
		l2 = l2/l;
		for(c=0;c<3;c++) 
			color[c] = colorMap->colorValue[k].color[c]*l2+colorMap->colorValue[k+1].color[c]*l1;
	}
	else
	for(c=0;c<3;c++)
		color[c] = colorMap->colorValue[k].color[c];
}
/**********************************************************************************/
static void gabedit_contoursplot_cairo_image(cairo_t* cr, GtkWidget *widget, gint x, gint y, gint w, gint h, cairo_surface_t *image)
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
static void gabedit_contoursplot_cairo_string(cairo_t* cr, GtkWidget *widget, GdkGC* gc, gint x, gint y, G_CONST_RETURN gchar* str, gboolean centerX, gboolean centerY, gdouble angle)
{
	PangoLayout* pango = get_pango_str(GABEDIT_ContoursPLOT(widget), str);
	gabedit_contoursplot_cairo_layout(cr, x, y, pango, centerX,  centerY, angle) ;
	g_object_unref(G_OBJECT(pango));
}
/*****************************************************************************/
static void gabedit_contoursplot_cairo_line(cairo_t *cr,  GtkWidget* widget, GdkGC* gc, gdouble x1,gdouble y1,gdouble x2,gdouble y2)
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
static void gabedit_contoursplot_cairo_lines(cairo_t *cr,  GtkWidget* widget, GdkGC* gc, GdkPoint* points, gint size)
{
	gint i;
	for(i=0;i<size-1;i++)
		gabedit_contoursplot_cairo_line(cr, widget, gc, points[i].x, points[i].y, points[i+1].x, points[i+1].y);
}
/*****************************************************************************/
static void gabedit_contoursplot_cairo_rectangle(cairo_t *cr,  GtkWidget* widget, GdkGC* gc, gboolean fill, gdouble x1,gdouble y1,gdouble w,gdouble h)
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
/*****************************************************************************/
/*
static void gabedit_contoursplot_cairo_cercle(cairo_t *cr,  GtkWidget* widget, GdkGC* gc,
		gint xc,gint yc,gint rayon)
{
	GdkGCValues values;
	GdkColor color;
	double r,g,b;
	GdkColormap *colormap;
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
   	colormap  = gdk_window_get_colormap(widget->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);
	r = SCALE(color.red);
	g = SCALE(color.green);
	b = SCALE(color.blue);
	cairo_set_source_rgba (cr, r, g, b, 1.0);
	if(values.line_width<1) values.line_width = 1;

	cairo_set_line_width (cr, values.line_width);
	cairo_arc (cr, xc, yc, rayon, 0, 2 * M_PI);
	if(values.fill==GDK_SOLID) cairo_fill (cr);
	cairo_stroke (cr);
	cairo_restore (cr); 
}
*/
/**********************************************************************************/
static void gabedit_contoursplot_cairo_layout(cairo_t* cr, gdouble x, gdouble y, PangoLayout *layout, gboolean centerX, gboolean centerY, gdouble angle) 
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
static void contoursplot_cairo_image(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget *widget, gint x, gint y, gint w, gint h, cairo_surface_t *image)
{
	gabedit_contoursplot_cairo_image(cr,  widget, x, y, w, h, image);
	if(contoursplot->cairo_export)
		gabedit_contoursplot_cairo_image(contoursplot->cairo_export,  widget, x, y, w, h, image);
}
/****************************************************************************************/
static void contoursplot_cairo_string(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gint x, gint y, G_CONST_RETURN gchar* str, gboolean centerX, gboolean centerY, gdouble angle)
{

	gabedit_contoursplot_cairo_string(cr,  widget, gc, x, y, str, centerX, centerY, angle);
	if(contoursplot->cairo_export)
		gabedit_contoursplot_cairo_string(contoursplot->cairo_export,  widget, gc,  x, y, str, centerX, centerY, angle);
}
/****************************************************************************************/
static void contoursplot_cairo_line(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gdouble x1,gdouble y1,gdouble x2,gdouble y2)
{
	gabedit_contoursplot_cairo_line(cr,  widget, gc, x1, y1, x2, y2);
	if(contoursplot->cairo_export)
		gabedit_contoursplot_cairo_line(contoursplot->cairo_export,  widget, gc,  x1, y1, x2, y2);
}
/****************************************************************************************/
static void contoursplot_cairo_rectangle(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gboolean fill, gdouble x1,gdouble y1,gdouble x2,gdouble y2)
{
	gabedit_contoursplot_cairo_rectangle(cr,  widget, gc, fill, x1, y1, x2, y2);
	if(contoursplot->cairo_export)
		gabedit_contoursplot_cairo_rectangle(contoursplot->cairo_export,  widget, gc,  fill, x1, y1, x2, y2);
}
/****************************************************************************************/
static void contoursplot_cairo_lines(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, GdkPoint* points, gint size)
{
	gabedit_contoursplot_cairo_lines(cr,  widget, gc, points, 2);
	if(contoursplot->cairo_export && size>0)
	{
		GdkPoint* pointsScal = g_malloc(size*sizeof(GdkPoint));
		gint i;
		if(!pointsScal) return;
		for(i=0;i<size;i++)
		{
			pointsScal[i].x = points[i].x + contoursplot->plotting_rect.x;
			pointsScal[i].y = points[i].y + contoursplot->plotting_rect.y;
		}
		gabedit_contoursplot_cairo_lines(contoursplot->cairo_export,  widget, gc, pointsScal, 2);
		if(pointsScal) g_free(pointsScal);
	}
}
/****************************************************************************************/
/*
static void contoursplot_cairo_cercle(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gint xc,gint yc,gint rayon)
{
	gabedit_contoursplot_cairo_cercle(cr,  widget, gc, xc, yc, rayon);
	if(contoursplot->cairo_export)
		gabedit_contoursplot_cairo_cercle(contoursplot->cairo_export,  widget, gc, xc, yc, rayon);
}
*/
/****************************************************************************************/
static void contoursplot_cairo_layout(GabeditContoursPlot *contoursplot, cairo_t* cr,   gdouble x, gdouble y, PangoLayout *layout, gboolean centerX, gboolean centerY, gdouble angle)
{
	gabedit_contoursplot_cairo_layout(cr, x, y, layout, centerX, centerY, angle);
	if(contoursplot->cairo_export)
	{
		if(cr == contoursplot->cairo_area)
		{
			x += contoursplot->plotting_rect.x;
			y += contoursplot->plotting_rect.y;
		}
		gabedit_contoursplot_cairo_layout(contoursplot->cairo_export,  x, y, layout, centerX, centerY, angle);
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
static void destroy_contoursplot_window(GtkWidget* contoursplot)
{
	GtkWidget* parentWindow;
	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
	if(parentWindow) gtk_object_destroy (GTK_OBJECT(parentWindow));
}
/****************************************************************************************/
static void destroy_data_dialog(GtkWidget* win, GdkEvent  *event, gpointer user_data)
{
  	gtk_object_destroy (GTK_OBJECT(win));
}
/********************************************************************************/
static gboolean remove_all_data(GtkWidget* contoursplot, gint response_id)
{
	if(response_id != GTK_RESPONSE_YES) return FALSE;
	if(!contoursplot) return FALSE; 
	if(GABEDIT_ContoursPLOT(contoursplot)->data_list)
	{
		g_list_foreach(GABEDIT_ContoursPLOT(contoursplot)->data_list, (GFunc)g_free, NULL);
		g_list_free(GABEDIT_ContoursPLOT(contoursplot)->data_list);
		GABEDIT_ContoursPLOT(contoursplot)->data_list = NULL;
	}
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	return TRUE;
}
/********************************************************************************/
static void remove_all_data_dlg(GtkWidget* contoursplot)
{
	GtkWidget* parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
	GtkWidget* dialog = gtk_message_dialog_new (GTK_WINDOW(parentWindow),
			GTK_DIALOG_DESTROY_WITH_PARENT,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
		       _("Are you sure to remove all data ?")
			);
	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_NO);
	g_signal_connect_swapped(dialog, "response", G_CALLBACK (remove_all_data), contoursplot);
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
/****************************************************************************************/
static void get_interpolatedValueData(ContoursPlotData* data, gdouble x, gdouble y, gdouble* zv, gdouble* pdzdx, gdouble* pdzdy, gdouble* pd2zdx2, gdouble* pd2zdy2, gdouble* pd2zdxdy)
{
	gint xsize = data->xsize;
	gint ysize = data->ysize;
	gdouble dx = (data->xmax-data->xmin)/(xsize-1);
	gdouble dy = (data->ymax-data->ymin)/(ysize-1);
	gint ix = (gint)((x-data->xmin)/dx);
	gint iy = (gint)((y-data->ymin)/dy);
	gint ix1 = ix+1;
	gint iy1 = iy+1;
	gint ixp = (ix+1)%xsize;
	gint ixm = (ix-1+xsize)%xsize;
	gint ix1p = (ix1+1)%xsize;
	gint ix1m = (ix1-1+xsize)%xsize;
	gint iyp = (iy+1)%ysize;
	gint iym = (iy-1+ysize)%ysize;
	gint iy1p = (iy1+1)%ysize;
	gint iy1m = (iy1-1+ysize)%ysize;
	gdouble xl = data->xmin+ix*dx;
	gdouble xu = data->xmin+(ix+1)*dx;
	gdouble yl = data->ymin+iy*dy;
	gdouble yu = data->ymin+(iy+1)*dy;
	gdouble* zValues = data->zValues;
	gdouble z[4] = { zValues[ix*ysize+iy], zValues[ix1*ysize+iy], zValues[ix1*ysize+iy1], zValues[ix*ysize+iy1]};
	gdouble dzdx[4] = {
	  (zValues[ixp*ysize+iy] - zValues[ixm*ysize+iy])/dx/2, 
	  (zValues[ix1p*ysize+iy] - zValues[ix1m*ysize+iy])/dx/2, 
	  (zValues[ix1p*ysize+iy1] - zValues[ix1m*ysize+iy1])/dx/2, 
	  (zValues[ixp*ysize+iy1] - zValues[ixm*ysize+iy1])/dx/2
	};
	gdouble dzdy[4] = { 
	  (zValues[ix*ysize+iyp] - zValues[ix*ysize+iym])/dy/2, 
	  (zValues[ix1*ysize+iyp] - zValues[ix1*ysize+iym])/dy/2, 
	  (zValues[ix1*ysize+iy1p] - zValues[ix1*ysize+iy1m])/dy/2, 
	  (zValues[ix*ysize+iy1p] - zValues[ix*ysize+iy1m])/dy/2
	};
	gdouble d2zdxdy[4] = { 
	  (zValues[ixp*ysize+iyp] + zValues[ixm*ysize+iym]-zValues[ixp*ysize+iym]-zValues[ixm*ysize+iyp])/dx/dy/4, 
	  (zValues[ix1p*ysize+iyp] + zValues[ix1m*ysize+iym]-zValues[ix1p*ysize+iym]-zValues[ix1m*ysize+iyp])/dx/dy/4, 
	  (zValues[ix1p*ysize+iy1p] + zValues[ix1m*ysize+iy1m]-zValues[ix1p*ysize+iy1m]-zValues[ix1m*ysize+iy1p])/dx/dy/4, 
	  (zValues[ixp*ysize+iy1p] + zValues[ixm*ysize+iy1m]-zValues[ixp*ysize+iy1m]-zValues[ixm*ysize+iy1p])/dx/dy/4
	};
	*zv = 0;
	biCubicInterpolation( z, dzdx, dzdy, d2zdxdy, xl, xu, yl,  yu,  x,  y, zv, pdzdx,pdzdy, pd2zdx2, pd2zdy2, pd2zdxdy);

}
/****************************************************************************************/
static void get_interpolatedValue(GtkWidget* contoursplot, ContoursPlotData* data, gdouble x, gdouble y, gdouble* zv, gdouble* pdzdx, gdouble* pdzdy, gdouble* pd2zdx2, gdouble* pd2zdy2, gdouble* pd2zdxdy)
{

	if(!GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap) return;
	if(!zv) return;
	if(data)
	{
		get_interpolatedValueData(data, x, y, zv, pdzdx, pdzdy, pd2zdx2, pd2zdy2, pd2zdxdy);
	}
	else
	{
		GList *current_node;
		current_node=g_list_last(GABEDIT_ContoursPLOT(contoursplot)->data_list);
		data=(ContoursPlotData*)current_node->data;  
		get_interpolatedValueData(data, x, y, zv, pdzdx, pdzdy, pd2zdx2, pd2zdy2, pd2zdxdy);
	}
}
/****************************************************************************************/
static void reset_contour_lines_styles(GtkWidget* contoursplot, ContoursPlotData* data)
{

	gint loop;
	if(!GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap) return;
	if(data)
	{
		for(loop=0;loop<data->nContours; loop++) 
		{
			gdouble value = data->contours[loop].value;
			data->contours[loop].line_style = GDK_LINE_SOLID;
			if(GABEDIT_ContoursPLOT(contoursplot)->dashed_negative_contours && value<0) 
				data->contours[loop].line_style = GDK_LINE_ON_OFF_DASH;
		}
	}
	else
	{
		GList *current_node;
		current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
		for (; current_node!=NULL; current_node=current_node->next)
		{
			data=(ContoursPlotData*)current_node->data;  
			for(loop=0;loop<data->nContours; loop++) 
			{
				gdouble value = data->contours[loop].value;
				data->contours[loop].line_style = GDK_LINE_SOLID;
				if(GABEDIT_ContoursPLOT(contoursplot)->dashed_negative_contours && value<0) 
					data->contours[loop].line_style = GDK_LINE_ON_OFF_DASH;
			}
		}
	}
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
static void reset_contour_colors(GtkWidget* contoursplot, ContoursPlotData* data)
{

	gint loop;
	if(!GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap) return;
	if(data)
	{
		for(loop=0;loop<data->nContours; loop++) 
		{
			gdouble value = data->contours[loop].value;
			gdouble color[3];
			set_Color_From_colorMap(GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap, color, value);
			data->contours[loop].line_color.red=(gushort)(color[0]*65535);
			data->contours[loop].line_color.green=(gushort)(color[1]*65535);
			data->contours[loop].line_color.blue=(gushort)(color[2]*65535);
		}
	}
	else
	{
		GList *current_node;
		current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
		for (; current_node!=NULL; current_node=current_node->next)
		{
			data=(ContoursPlotData*)current_node->data;  
			for(loop=0;loop<data->nContours; loop++) 
			{
				gdouble value = data->contours[loop].value;
				gdouble color[3];
				set_Color_From_colorMap(GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap, color, value);
				data->contours[loop].line_color.red=(gushort)(color[0]*65535);
				data->contours[loop].line_color.green=(gushort)(color[1]*65535);
				data->contours[loop].line_color.blue=(gushort)(color[2]*65535);
			}
		}
	}
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
static void contoursplot_build_points_contour(GabeditContoursPlot *contoursplot, ContourData *contour)
{
	GtkWidget *widget;
	if (contoursplot == NULL) return;
	if(contoursplot->data_list == NULL) return;
  
	widget=GTK_WIDGET(contoursplot);
	if(contour->point_pango != NULL) g_object_unref(G_OBJECT(contour->point_pango));
	contour->point_pango = gtk_widget_create_pango_layout (widget, contour->point_str);
	if(contour->point_pango)
	{
		PangoRectangle logical_rect;
		pango_layout_set_justify(contour->point_pango,TRUE);
		pango_layout_set_alignment(contour->point_pango,PANGO_ALIGN_CENTER);
		pango_layout_set_alignment(contour->point_pango,PANGO_ALIGN_LEFT);
		pango_layout_set_markup(contour->point_pango, contour->point_str, -1);
		gchar fontName[100];
		PangoAttrList *attr_list;
		PangoFontDescription *font_desc = NULL;
		sprintf(fontName,"sans %d",2+contour->point_size*2);
		font_desc = pango_font_description_from_string (fontName);
    		pango_layout_set_font_description (contour->point_pango, font_desc);  
		pango_layout_context_changed(contour->point_pango);
		pango_layout_get_pixel_extents(contour->point_pango, NULL, &logical_rect);
    		pango_layout_get_size(contour->point_pango, &(contour->point_width), &(contour->point_height));
		contour->point_width/=PANGO_SCALE;
		contour->point_height/=PANGO_SCALE;
		/* printf("%d %d\n",logical_rect.x,logical_rect.y);*/
		contour->point_width =logical_rect.width;
		contour->point_height =logical_rect.height;
		pango_font_description_free (font_desc);
		/* set color */
		attr_list = pango_layout_get_attributes (contour->point_pango);
		if (attr_list == NULL) attr_list = pango_attr_list_new ();
		else pango_attr_list_ref (attr_list);
		pango_attr_list_insert(attr_list,
				pango_attr_foreground_new(
					contour->point_color.red, 
					contour->point_color.green, 
					contour->point_color.blue)
				);
		pango_layout_set_attributes (contour->point_pango, attr_list);
		pango_attr_list_unref (attr_list);
	}
}
/**************************************************************/
static void getzMinzMax(GtkWidget* contoursplot, ContoursPlotData* data, gdouble* zmin, gdouble* zmax) 
{
	*zmin = 0;
	*zmax = 0;
	if(data)
	{
		*zmin=data->zmin;
		*zmax=data->zmax;
	}
	if(contoursplot && GABEDIT_ContoursPLOT(contoursplot)->data_list)
	{
		gint ib = 0;
		GList *current_node;
		current_node=g_list_last(GABEDIT_ContoursPLOT(contoursplot)->data_list);
		if(current_node)
		{
			ContoursPlotData* data = (ContoursPlotData*)current_node->data;  
			if(ib==0)
			{
				*zmin=data->zmin;
				*zmax=data->zmax;
			}
			else
			{
				ib = 1;
				if(*zmin>data->zmin) *zmin=data->zmin;
				if(*zmax<data->zmax) *zmax=data->zmax;
			}
		}
	}
}
/**************************************************************/
static void get_one_point(gdouble v1, gdouble v2, gdouble x1, gdouble y1, gdouble x2, gdouble y2, gdouble* x, gdouble* y)
{
	gdouble u1 = x1;
	gdouble u2 = x2;
	if(v1==0) *x = u1;
	else *x = u1 - v1 *(u2-u1)/(v2-v1);
	u1 = y1;
	u2 = y2;
	if(v1==0) *y = u1;
	else *y = u1 - v1 *(u2-u1)/(v2-v1);
}
/**************************************************************/
static gdouble get_1r2(gdouble x1, gdouble y1, gdouble x2, gdouble y2)
{
	gdouble r2 = (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
	if(r2<1e-14) return 1e14;
	return 1/r2;
}
/**************************************************************/
ContourData get_contour_point(GtkWidget* contoursplot, ContoursPlotData *data, gdouble value)
{
	gint i;
	gint j;
	gint k;
	gdouble v1;
	gdouble v2;
	gint ix=0,iy=0;
	gint ix1=0,iy1=0;
	gint ix2=0,iy2=0;
	gint ix3=0,iy3=0;
	gint ix4=0,iy4=0;
	gdouble x1,x2,y1,y2,x,y;
	gdouble dx, dy;
	gdouble xmin,xmax,ymin,ymax;
	gint xsize,ysize;
	gdouble* zValues;
	ContourData contour;
	gint red = rand()%60000;
	gint green = rand()%60000;
	gint blue = rand()%60000;
	gdouble color[3];

	contour.value = value;
	contour.size = 0;
	contour.x = NULL;
	contour.y = NULL;
	contour.index = NULL;
	xmin = data->xmin;
	xmax = data->xmax;
	ymin = data->ymin;
	ymax = data->ymax;
	xsize = data->xsize;
	ysize = data->ysize;
	dx = (xmax-xmin)/(xsize-1);
	dy = (ymax-ymin)/(ysize-1);
	zValues = data->zValues;

	for( i = 0; i<xsize-1;i++)
	{
		for( j = 0; j<ysize-1;j++)
		{
			ix = i;
			ix1 = ix2 = ix + 1;
			ix4 = ix3 = ix;
			iy = j; 
			iy1 = iy4 =  iy; 
			iy2 = iy3 = iy+1 ;
        
			k = -1;
			
			v1 = zValues[ix*ysize+iy]-value,
			v2 = zValues[ix1*ysize+iy1]-value;
			if( v1*v2 <=0 )
			{
				k++;
				x1 = xmin+ix*dx;
				y1 = ymin+iy*dy;
				x2 = xmin+ix1*dx;
				y2 = ymin+iy1*dy;
				get_one_point(v1, v2, x1, y1, x2, y2, &x, &y);
				contour.size++;
				contour.x = g_realloc(contour.x,contour.size*sizeof(gdouble));
				contour.y = g_realloc(contour.y,contour.size*sizeof(gdouble));
				contour.index = g_realloc(contour.index,contour.size*sizeof(gint));
				contour.x[contour.size-1]=x;
				contour.y[contour.size-1]=y;
				contour.index[contour.size-1]=i*ysize+j;

			}
			v1 = v2;
			v2 = zValues[ix2*ysize+iy2]-value;
			if( v1*v2 <=0 )
			{
				k++;
				x1 = xmin+ix1*dx;
				y1 = ymin+iy1*dy;
				x2 = xmin+ix2*dx;
				y2 = ymin+iy2*dy;
				get_one_point(v1, v2, x1, y1, x2, y2, &x, &y);
				contour.size++;
				contour.x = g_realloc(contour.x,contour.size*sizeof(gdouble));
				contour.y = g_realloc(contour.y,contour.size*sizeof(gdouble));
				contour.index = g_realloc(contour.index,contour.size*sizeof(gint));
				contour.x[contour.size-1]=x;
				contour.y[contour.size-1]=y;
				contour.index[contour.size-1]=i*ysize+j;
			}
			v1 = v2;
			v2 = zValues[ix3*ysize+iy3]-value;
			if( v1*v2 <=0 )
			{
				k++;
				x1 = xmin+ix2*dx;
				y1 = ymin+iy2*dy;
				x2 = xmin+ix3*dx;
				y2 = ymin+iy3*dy;
				get_one_point(v1, v2, x1, y1, x2, y2, &x, &y);
				contour.size++;
				contour.x = g_realloc(contour.x,contour.size*sizeof(gdouble));
				contour.y = g_realloc(contour.y,contour.size*sizeof(gdouble));
				contour.index = g_realloc(contour.index,contour.size*sizeof(gint));
				contour.x[contour.size-1]=x;
				contour.y[contour.size-1]=y;
				contour.index[contour.size-1]=i*ysize+j;
			}
			v1 = v2;
			v2 = zValues[ix4*ysize+iy4]-value;

			if( v1*v2 <=0 )
			{
				k++;
				x1 = xmin+ix3*dx;
				y1 = ymin+iy3*dy;
				x2 = xmin+ix4*dx;
				y2 = ymin+iy4*dy;
				get_one_point(v1, v2, x1, y1, x2, y2, &x, &y);
				contour.size++;
				contour.x = g_realloc(contour.x,contour.size*sizeof(gdouble));
				contour.y = g_realloc(contour.y,contour.size*sizeof(gdouble));
				contour.index = g_realloc(contour.index,contour.size*sizeof(gint));
				contour.x[contour.size-1]=x;
				contour.y[contour.size-1]=y;
				contour.index[contour.size-1]=i*ysize+j;
			}
		}
	}
	sprintf(contour.point_str,"+");
	contour.point_pango = NULL;
	contoursplot_build_points_contour(GABEDIT_ContoursPLOT(contoursplot), &contour);

	contour.point_size = 0;
	contour.line_width=2;
	contour.point_color.red=red; 
	contour.point_color.green=green; 
	contour.point_color.blue=blue; 

	contour.line_color.red=green;
	contour.line_color.green=red;
	contour.line_color.blue=blue;
	if(GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap)
	{
		set_Color_From_colorMap(GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap, color, value);
		contour.line_color.red=(gushort)(color[0]*65535);
		contour.line_color.green=(gushort)(color[1]*65535);
		contour.line_color.blue=(gushort)(color[2]*65535);
	}

	contour.line_style=GDK_LINE_SOLID;
	if(GABEDIT_ContoursPLOT(contoursplot)->dashed_negative_contours && value<0) contour.line_style = GDK_LINE_ON_OFF_DASH;

	contour.label = g_malloc( (sizeof(gchar) * (GABEDIT_ContoursPLOT(contoursplot)->x_legends_digits+1))); 
	snprintf(contour.label, GABEDIT_ContoursPLOT(contoursplot)->x_legends_digits+1, "%lf", value);
	contour.label[GABEDIT_ContoursPLOT(contoursplot)->x_legends_digits]='\0';
	contour.xlabel = 0;
	contour.ylabel = 0;
	if(contour.size>0)
	{
		gint i = 0;
		gdouble vmin = get_1r2(contour.x[i],contour.y[i],xmin,ymin)
			       +get_1r2(contour.x[i],contour.y[i],xmin,ymax)
			       +get_1r2(contour.x[i],contour.y[i],xmax,ymin)
			       +get_1r2(contour.x[i],contour.y[i],xmax,ymax);

		contour.xlabel = contour.x[0];
		contour.ylabel = contour.y[0];
		for(i=1;i<contour.size;i++)
		{
			gdouble v = get_1r2(contour.x[i],contour.y[i],xmin,ymin)
			       +get_1r2(contour.x[i],contour.y[i],xmin,ymax)
			       +get_1r2(contour.x[i],contour.y[i],xmax,ymin)
			       +get_1r2(contour.x[i],contour.y[i],xmax,ymax);

			if(v<vmin)
			{
				contour.xlabel = contour.x[i];
				contour.ylabel = contour.y[i];
				vmin = v;
			}
		}
	}
	return contour;

}
/****************************************************************************************/
static void set_label_one_contour(ContoursPlotData* data, gint n)
{
	gint i;
	gint j;
	gdouble vmin = 0;
	gdouble xmin, xmax, ymin, ymax;
	gdouble dx, dy;
	gint k;
	gint N = 10;
	if(!data) return;
	if(data->contours[n].size<1) return;
	data->contours[n].xlabel = data->contours[n].x[0];
	data->contours[n].ylabel = data->contours[n].y[0];
	xmin = data->xmin;
	xmax = data->xmax;
	ymin = data->ymin;
	ymax = data->ymax;
	dx = (xmax-xmin)/(N-1);
	dy = (ymax-ymin)/(N-1);
	i = 0;
	vmin = 0;
	for(k=0;k<N; k++) 
	{
		gdouble x;
		gdouble y;
		x = xmin + dx*k;
		y = ymin;
		vmin += get_1r2(data->contours[n].x[i],data->contours[n].y[i],x,y);
		y = ymax;
		vmin += get_1r2(data->contours[n].x[i],data->contours[n].y[i],x,y);
		x = xmin;
		y = ymin + dy*k;
		vmin += get_1r2(data->contours[n].x[i],data->contours[n].y[i],x,y);
		x = xmax;
		y = ymin + dy*k;
		vmin += get_1r2(data->contours[n].x[i],data->contours[n].y[i],x,y);
	}
	for(j=0;j<n; j++) 
	{
		vmin += get_1r2( data->contours[n].x[i],data->contours[n].y[i],
				data->contours[j].xlabel,data->contours[j].ylabel);
	}

	data->contours[n].xlabel = data->contours[n].x[0];
	data->contours[n].ylabel = data->contours[n].y[0];
	for(i=1;i<data->contours[n].size;i++)
	{
		gdouble v = 0;
		for(k=0;k<N; k++) 
		{
			gdouble x;
			gdouble y;
			x = xmin + dx*k;
			y = ymin;
			v += get_1r2(data->contours[n].x[i],data->contours[n].y[i],x,y);
			y = ymax;
			v += get_1r2(data->contours[n].x[i],data->contours[n].y[i],x,y);
			x = xmin;
			y = ymin + dy*k;
			v += get_1r2(data->contours[n].x[i],data->contours[n].y[i],x,y);
			x = xmax;
			y = ymin + dy*k;
			v += get_1r2(data->contours[n].x[i],data->contours[n].y[i],x,y);
		}
		for(j=0;j<n; j++) 
			v += get_1r2( data->contours[n].x[i],data->contours[n].y[i],
			data->contours[j].xlabel,data->contours[j].ylabel);

		if(v<vmin)
		{
			data->contours[n].xlabel = data->contours[n].x[i];
			data->contours[n].ylabel = data->contours[n].y[i];
			vmin = v;
		}
	}
}
/****************************************************************************************/
static void reset_label_contours(GtkWidget* contoursplot, ContoursPlotData* data)
{

	gint n;
	if(!GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap) return;
	if(data)
	{
		for(n=0;n<data->nContours; n++) 
			set_label_one_contour(data, n);
	}
	else
	{
		GList *current_node;
		current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
		for (; current_node!=NULL; current_node=current_node->next)
		{
			data=(ContoursPlotData*)current_node->data;  
			for(n=0;n<data->nContours; n++) 
				set_label_one_contour(data, n);
		}
	}
}
/**************************************************************/
static void free_one_contour(ContourData* contour)
{
	if(contour->x) g_free(contour->x);
	if(contour->y) g_free(contour->y);
	if(contour->index) g_free(contour->index);
	if(contour->point_pango) g_object_unref(G_OBJECT(contour->point_pango));
	contour->x = NULL;
	contour->y = NULL;
	contour->index = NULL;
	contour->point_pango = NULL;
}
/**************************************************************/
static void add_one_contour(GtkWidget* contoursplot, ContoursPlotData *data, gdouble value)
{
	ContourData  contour;
	if(!data) return;
	contour = get_contour_point(contoursplot, data, value);
	if(contour.size<1) return;
	data->nContours++;
	data->contours = g_realloc(data->contours,data->nContours*sizeof(ContourData));
	data->contours[data->nContours-1] = contour;
}
/**********************************************/
static void delete_last_spaces(gchar* str)
{
	gchar *s;

	if(str == NULL)
		return;

	if (!*str)
		return;
	for (s = str + strlen (str) - 1; s >= str && isspace ((guchar)*s); s--)
		*s = '\0';
}
/**********************************************/
static void delete_first_spaces(gchar* str)
{
	gchar *start;
	gint i;
	gint lenSpace = 0;

	if(str == NULL) return;
	if (!*str) return;

	for (start = str; *start && isspace (*start); start++)lenSpace++;

	for(i=0;i<(gint)(strlen(str)-lenSpace);i++)
		str[i] = str[i+lenSpace];
	str[strlen(str)-lenSpace] = '\0';
}
/**********************************************/
/*
static void delete_all_spaces(gchar* str)
{
	gint i;
	gint j;
	gboolean Ok = FALSE;

	delete_last_spaces(str);
	delete_first_spaces(str);
	while(!Ok)
	{
		Ok = TRUE;
		for(i=0;i<(gint)strlen(str);i++)
		{
			if(isspace(str[i]))
			{
				Ok = FALSE;
				for(j=i;j<(gint)strlen(str);j++)
				{
					str[j] = str[j+1];
				}
				break;
			}
		}
	}
}
*/
/*************************************************************************************/
static gboolean testascii(char c)
{
	switch ( c )
	{
	case	'0':
	case	'1':
	case	'2':
	case	'3':
	case	'4':
	case	'5':
	case	'6':
	case	'7':
	case	'8':
	case	'9':
	case	'.':
	case	'e':
	case	'E':
	case	'+':
	case	'-':return TRUE;
	}
	return FALSE;
}
/*************************************************************************************/
static gboolean this_is_a_real(gchar *t)
{
	gint i;
	for(i=0;i<strlen(t);i++)
		if(!testascii(t[i]) ) return FALSE;
	if(t[0] =='e' || t[0] =='E' ) return FALSE;
	return TRUE;

}
/****************************************************************************************/
static void delete_contours(GtkWidget* contoursplot, ContoursPlotData* data)
{

	gint loop;
	if(!GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap) return;
	if(data)
	{
		for(loop=0;loop<data->nContours; loop++) 
			free_one_contour(&data->contours[loop]);
		if(data->contours) g_free(data->contours);
		data->nContours = 0;
		data->contours = NULL;
	}
	else
	{
		GList *current_node;
		current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
		for (; current_node!=NULL; current_node=current_node->next)
		{
			data=(ContoursPlotData*)current_node->data;  
			for(loop=0;loop<data->nContours; loop++) 
				free_one_contour(&data->contours[loop]);
			if(data->contours) g_free(data->contours);
			data->nContours = 0;
			data->contours = NULL;
		}
	}
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}

/********************************************************************************/
static void Message(gchar* message, gchar* title)
{
	GtkWidget* dialog = NULL;
	if(!message) return;
	if(!title) return;
	if(strstr(title,"Error"))
	dialog = gtk_message_dialog_new_with_markup (NULL,
                 GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
	        "%s",
		message);
	else
	dialog = gtk_message_dialog_new_with_markup (NULL,
                 GTK_DIALOG_DESTROY_WITH_PARENT,
		GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			   "%s",
		message);

	if(dialog)
	{
		gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);
	}
}
/********************************************************************************/
static gboolean get_a_float(GtkWidget* Entry,gdouble* value, gchar* errorMessage)
{
	G_CONST_RETURN gchar* temp;
	gchar* t = NULL;
        temp	= gtk_entry_get_text(GTK_ENTRY(Entry)); 
	if(temp&& strlen(temp)>0)
	{
		t = g_strdup(temp);
		delete_first_spaces(t);
		delete_last_spaces(t);
	}
	else
	{
		Message(errorMessage,"Error");
		return FALSE;
	}
	if(t && !this_is_a_real(t))
	{
		Message(errorMessage,"Error");
		g_free(t);
		return FALSE;
	}
	if(t) g_free(t);
	*value = atof(temp);
	return TRUE;
}
/********************************************************************************/
static void add_widget_table(GtkWidget *Table,GtkWidget *wid,gushort line,gushort colonne)
{

	gtk_table_attach(GTK_TABLE(Table),wid,colonne,colonne+1,line,line+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  3,3);

}
/********************************************************************************/
static GtkWidget *add_label_at_table(GtkWidget *Table,gchar *label,gushort line,gushort colonne,GtkJustification just)
{
	GtkWidget *Label;
	GtkWidget *hbox = gtk_hbox_new(0,FALSE);
	
	Label = gtk_label_new (label);
   	gtk_label_set_justify(GTK_LABEL(Label),just);
	if(just ==GTK_JUSTIFY_CENTER) 
		gtk_box_pack_start (GTK_BOX (hbox), Label, TRUE, TRUE, 0);
	else
		gtk_box_pack_start (GTK_BOX (hbox), Label, FALSE, FALSE, 0);
	
	add_widget_table(Table,hbox,line,colonne);

	return Label;
}
/********************************************************************************/
static void apply_contours(GtkWidget *window,gpointer data)
{
	GtkWidget* entryNumber =(GtkWidget*)g_object_get_data(G_OBJECT (window), "EntryNumber");
	GtkWidget* entryMin =(GtkWidget*)g_object_get_data(G_OBJECT (window), "EntryMin");
	GtkWidget* entryMax =(GtkWidget*)g_object_get_data(G_OBJECT (window), "EntryMax");
	GtkWidget* linearButton =(GtkWidget*)g_object_get_data(G_OBJECT (window), "LinearButton");
	GtkWidget* contoursplot = (GtkWidget* )g_object_get_data(G_OBJECT (window), "ContoursPlot");
	G_CONST_RETURN gchar* temp;
	gint i;
	gint N;
	gdouble min;
	gdouble max;
	gdouble* values = NULL;
	gdouble step = 0;
	
        temp	= gtk_entry_get_text(GTK_ENTRY(entryNumber)); 
	N = atoi(temp);
	if(N<=0)
	{
		Message(_("Error : The number of points should be a positive integer. "),_("Error"));
		return;
	}
        if(!get_a_float(entryMin,&min,_("Error : The minimal value should be float.") )) return;
        if(!get_a_float(entryMax,&max,_("Error : The maximal value should be float."))) return;
	if( max<=min)
	{
		Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"));
		return;
	}

	values = g_malloc(N*sizeof(gdouble));

	if(GTK_TOGGLE_BUTTON (linearButton)->active)
	{
		step = (max-min)/(N-1);
		for(i=0;i<N;i++) values[i] = min + i*step;
	}
	else
	{
		gdouble e = exp(1.0);
		step = (1.0)/(N-1);
		for(i=0;i<N;i++) values[i] = min+(max-min)*log(step*i*(e-1)+1);
	}
	/* Debug("Begin set_contours_values\n");*/
	if(contoursplot && GABEDIT_ContoursPLOT(contoursplot)->data_list)
	{
		GList *current_node;
		current_node=g_list_last(GABEDIT_ContoursPLOT(contoursplot)->data_list);
		if(current_node)
		{
			ContoursPlotData* data = (ContoursPlotData*)current_node->data;  
			gint i;
			for(i=0;i<N;i++)
			{
				add_one_contour(contoursplot, data, values[i]);
			}
		}
	}
	reset_label_contours(contoursplot, NULL);
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	/* Debug("End set_contours_values\n");*/
	if(values) g_free(values);
	
  	gtk_widget_destroy(window);
}
/********************************************************************************/
static GtkWidget *create_contours_frame(GtkWidget* contoursplot, GtkWidget *window, GtkWidget *vboxall,gchar* title, ContoursPlotData* data)
{
	GtkWidget *frame;
	GtkWidget *LabelMin = NULL;
	GtkWidget *LabelMax = NULL;
	GtkWidget *vboxframe;
	GtkWidget *hseparator;
	GtkWidget *linearButton;
	GtkWidget *logButton;
	GtkWidget *entryNumber = NULL;
	GtkWidget *entryMin = NULL;
	GtkWidget *entryMax = NULL;
	gushort i;
	gushort j;
	GtkWidget *Table;
	gdouble min = 0;
	gdouble max = 0;
	gchar t[100];
	
	getzMinzMax(contoursplot, data, &min, &max);
	frame = gtk_frame_new (title);
	gtk_container_set_border_width (GTK_CONTAINER (frame), 5);
	gtk_container_add (GTK_CONTAINER (vboxall), frame);
	gtk_widget_show (frame);

	vboxframe=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vboxframe);
	Table = gtk_table_new(5,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vboxframe),Table);

	i = 0;
	j = 0;
	add_label_at_table(Table,_(" Numbre of contours "),i,j,GTK_JUSTIFY_LEFT);
	j++;
	add_label_at_table(Table," : ",i,j,GTK_JUSTIFY_LEFT);
	j++;
	entryNumber = gtk_entry_new ();
	add_widget_table(Table,entryNumber,i,j);
	gtk_entry_set_text(GTK_ENTRY(entryNumber),"10");

	i = 1;
	j = 0;
	LabelMin = add_label_at_table(Table,_(" Min iso-value "),i,j,GTK_JUSTIFY_LEFT);
	j++;
	add_label_at_table(Table," : ",i,j,GTK_JUSTIFY_LEFT);
	j++;
	entryMin = gtk_entry_new ();
	add_widget_table(Table,entryMin,i,j);
	sprintf(t,"%f",min);
	gtk_entry_set_text(GTK_ENTRY(entryMin),t);

	i = 2;
	j = 0;
	LabelMax = add_label_at_table(Table,_(" Max iso-value "),i,j,GTK_JUSTIFY_LEFT);
	j++;
	add_label_at_table(Table," : ",i,j,GTK_JUSTIFY_LEFT);
	j++;
	entryMax = gtk_entry_new ();
	add_widget_table(Table,entryMax,i,j);
	sprintf(t,"%f",max);
	gtk_entry_set_text(GTK_ENTRY(entryMax),t);


	i=3;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(Table),hseparator,0,3,i,i+1,
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
                  3,3);

	i=5;
	linearButton = gtk_radio_button_new_with_label( NULL,_("Linear scale"));
	add_widget_table(Table, linearButton,i,0);
	logButton = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (linearButton)), _("logarithmic scale")); 
	gtk_table_attach(GTK_TABLE(Table), logButton,1,3,i,i+1,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND) ,
		  (GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		  3,3);

	g_object_set_data(G_OBJECT (window), "EntryNumber",entryNumber);
	g_object_set_data(G_OBJECT (window), "EntryMin",entryMin);
	g_object_set_data(G_OBJECT (window), "EntryMax",entryMax);
	g_object_set_data(G_OBJECT (window), "LinearButton",linearButton);
	g_object_set_data(G_OBJECT (window), "LogButton",logButton);

	gtk_widget_show_all(frame);
  
  	return frame;
}

/**************************************************************/
static void create_contours(GtkWidget* contoursplot, ContoursPlotData* data)
{
	GtkWidget *window;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vboxall;
	GtkWidget *vboxwin;
	GtkWidget *button;

  	g_return_if_fail (contoursplot != NULL);
  	g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
	if(GABEDIT_ContoursPLOT(contoursplot)->data_list == NULL)
	{
		Message(_("No data available :\n You should read a file"),_("Error"));
		return;
	}
  	g_return_if_fail (GABEDIT_ContoursPLOT(contoursplot)->data_list != NULL);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window),_("Create contours"));
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	gtk_container_set_border_width (GTK_CONTAINER (window), 5);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)contoursplot);

	vboxall=gtk_vbox_new(FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), vboxall);
	vboxwin = vboxall;

	frame = create_contours_frame(contoursplot, window,vboxall,_("Contours"), data);
	g_object_set_data(G_OBJECT (window), "ContoursPlot",contoursplot);

	/* buttons box */
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vboxwin), hbox, TRUE, FALSE, 2);
	gtk_widget_realize(window);

	button = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)gtk_widget_destroy,GTK_OBJECT(window));
	gtk_widget_show (button);

	button = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_box_pack_start (GTK_BOX( hbox), button, TRUE, TRUE, 3);
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	gtk_widget_show (button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked",(GCallback)apply_contours,GTK_OBJECT(window));

	/* Show all */
	gtk_widget_show_all (window);
}
/************************************************************************************************************/
static void reset_color_map_min_max(GtkWidget* contoursplot,gdouble min, gdouble max)
{
	ColorMap* newColorMap = NULL;

	newColorMap = new_colorMap_min_max(min, max, GABEDIT_ContoursPLOT(contoursplot)->colorsMap.type,
			GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colors);
	colormap_free(GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap);
	g_free(GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap);
	GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap = newColorMap;
	contoursplot_build_colormap_legends(GABEDIT_ContoursPLOT(contoursplot));
}
/************************************************************************************************************/
static void reset_color_map(GtkWidget* contoursplot, ContoursPlotData *data)
{
	if(!data && GABEDIT_ContoursPLOT(contoursplot)->data_list)
	{
		GList *current_node;
		current_node=g_list_last(GABEDIT_ContoursPLOT(contoursplot)->data_list);
		if(current_node) data = (ContoursPlotData*)current_node->data;  
	}
	if(!data) return;
	reset_color_map_min_max(contoursplot, data->zmin, data->zmax);
}
/********************************************************************************/
static void add_new_data(GtkWidget* contoursplot, 
		gint xsize, gdouble xmin, gdouble xmax,
		gint ysize, gdouble ymin, gdouble ymax,
		gdouble* zValues)
{
	if(xsize>0 && ysize>0)
	{
		gint loop;
		ContoursPlotData *data = g_malloc(sizeof(ContoursPlotData));
		data->xsize=xsize;
		data->xmin=xmin;
		data->xmax=xmax;
		data->ysize=xsize;
		data->ymin=ymin;
		data->ymax=ymax;
		data->nContours=0;
		data->contours=NULL;

		data->zValues = g_malloc(xsize*ysize*sizeof(gdouble)); 

		data->zmin=zValues[0];
		data->zmax=zValues[0];
		for(loop = 0; loop<data->xsize*data->ysize;loop++)
		{
			data->zValues[loop] = zValues[loop];
			if(data->zmin>zValues[loop]) data->zmin=zValues[loop];
			if(data->zmax<zValues[loop]) data->zmax=zValues[loop];
		}

		gabedit_contoursplot_add_data (GABEDIT_ContoursPLOT(contoursplot), data);
		gabedit_contoursplot_set_autorange(GABEDIT_ContoursPLOT(contoursplot), NULL);
		reset_color_map(contoursplot, data);
	}
}
/****************************************************************************************/
static void set_object_text_pixels(GabeditContoursPlot *contoursplot, ContoursPlotObjectText* objectText)
{
	if(!objectText) return;
	value2pixel(contoursplot, objectText->x,objectText->y, &objectText->xi, &objectText->yi);
	objectText->yi=contoursplot->plotting_rect.height-objectText->yi;
	pango_layout_get_size(objectText->pango, &objectText->width, &objectText->height);
	objectText->width/=PANGO_SCALE;
	objectText->height/=PANGO_SCALE;
	/* objectText->yi -= objectText->height/2;*/
	objectText->xi += contoursplot->plotting_rect.x;
	objectText->yi += contoursplot->plotting_rect.y;
}
/****************************************************************************************/
static PangoLayout* get_pango_str(GabeditContoursPlot *contoursplot, G_CONST_RETURN gchar* txt)
{
	gchar *str = NULL;
	GdkGCValues values;
	GdkColormap *colormap;
	GdkColor color;
	gchar* rgb = NULL;
	PangoLayout* pango;

	if(!txt) return NULL;

	pango = gtk_widget_create_pango_layout (GTK_WIDGET(contoursplot), txt);
	pango_layout_set_alignment(pango,PANGO_ALIGN_LEFT);

	gdk_gc_get_values(contoursplot->fore_gc, &values);
   	colormap  = gdk_window_get_colormap(GTK_WIDGET(contoursplot)->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);
	rgb = g_strdup_printf("#%02x%02x%02x", color.red >> 8, color.green >> 8, color.blue >> 8);
	str = g_strconcat("<span foreground='", rgb, "'>",txt, "</span>", NULL);
	pango_layout_set_markup(pango, str, -1);
	g_free(str);
	return pango;
}
/****************************************************************************************/
static void set_object_text_pango(GabeditContoursPlot *contoursplot, ContoursPlotObjectText* objectText)
{
	if(!objectText) return;
  	objectText->pango = get_pango_str(contoursplot, objectText->str);
}
/****************************************************************************************/
static void set_object_text(GabeditContoursPlot *contoursplot, ContoursPlotObjectText* objectText, gdouble x, gdouble y, gdouble angle, G_CONST_RETURN gchar* str)
{
	if(!str) return;
	objectText->x = x;
	objectText->y = y;
	objectText->angle = angle;
	objectText->str = g_strdup(str);
	set_object_text_pango(contoursplot, objectText);
	set_object_text_pixels(contoursplot, objectText);
}
/****************************************************************************************/
static void add_object_text(GabeditContoursPlot *contoursplot, gdouble x, gdouble y, gdouble angle, G_CONST_RETURN gchar* str)
{
	gint i;
	if(!str) return;
	contoursplot->nObjectsText++;
	if(contoursplot->nObjectsText==1) contoursplot->objectsText = g_malloc(sizeof(ContoursPlotObjectText));
	else contoursplot->objectsText = g_realloc(contoursplot->objectsText,contoursplot->nObjectsText*sizeof(ContoursPlotObjectText));
	i = contoursplot->nObjectsText-1;
	set_object_text(contoursplot, &contoursplot->objectsText[i],  x,  y, angle, str);
}
/****************************************************************************************/
static void reset_object_text_pixels(GabeditContoursPlot *contoursplot)
{
	gint i;
	if(!contoursplot) return;
	for(i=0;i<contoursplot->nObjectsText;i++)
		set_object_text_pixels(contoursplot, &contoursplot->objectsText[i]);
}
/****************************************************************************************/
static void reset_object_text_pango(GabeditContoursPlot *contoursplot)
{
	gint i;
	if(!contoursplot) return;
	for(i=0;i<contoursplot->nObjectsText;i++)
		set_object_text_pango(contoursplot, &contoursplot->objectsText[i]);
}
/****************************************************************************************/
static gint get_object_text_num(GabeditContoursPlot *contoursplot, gint xi, gint yi)
{
	gint i;
	gint x,y;
	if(!contoursplot) return -1;
	for(i=0;i<contoursplot->nObjectsText;i++)
	{
		gdouble cosa = cos(contoursplot->objectsText[i].angle);
		gdouble sina = sin(contoursplot->objectsText[i].angle);
		gdouble xx = xi-contoursplot->objectsText[i].xi;
		gdouble yy = yi-contoursplot->objectsText[i].yi;
		x = contoursplot->objectsText[i].xi+xx*cosa-yy*sina;
		y = contoursplot->objectsText[i].yi+xx*sina+yy*cosa;
		if(x>=contoursplot->objectsText[i].xi && y>=contoursplot->objectsText[i].yi
		&& x<=contoursplot->objectsText[i].xi+contoursplot->objectsText[i].width 
		&& y<=contoursplot->objectsText[i].yi+contoursplot->objectsText[i].height)
			return i;
	}
	return -1;
}
/****************************************************************************************/
static void delete_object_text(GtkWidget *widget, gint i)
{
	gint j;
	GabeditContoursPlot *contoursplot = NULL;
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	contoursplot = GABEDIT_ContoursPLOT(widget);
	if(i<0||i>=contoursplot->nObjectsText) return;
	if(contoursplot->objectsText[i].str) g_free(contoursplot->objectsText[i].str);
	if(contoursplot->objectsText[i].pango) g_object_unref(G_OBJECT(contoursplot->objectsText[i].pango));
	for(j=i;j<contoursplot->nObjectsText-1;j++)
		contoursplot->objectsText[j] = contoursplot->objectsText[j+1];
	contoursplot->nObjectsText--;
	if(contoursplot->nObjectsText<1) 
	{
		contoursplot->nObjectsText = 0;
		if(contoursplot->objectsText) g_free(contoursplot->objectsText);
		contoursplot->objectsText = NULL;
	}
	else
	{
		contoursplot->objectsText = g_realloc(contoursplot->objectsText,contoursplot->nObjectsText*sizeof(ContoursPlotObjectText));
	}

}
/****************************************************************************************/
static void delete_objects_text(GtkWidget *widget)
{
	GabeditContoursPlot *contoursplot = NULL;
	gint i;
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	contoursplot = GABEDIT_ContoursPLOT(widget);
	for(i=0;i<contoursplot->nObjectsText;i++)
	{
		if(contoursplot->objectsText[i].str) g_free(contoursplot->objectsText[i].str);
		if(contoursplot->objectsText[i].pango) g_object_unref(G_OBJECT(contoursplot->objectsText[i].pango));
	}
	contoursplot->nObjectsText = 0;
	if(contoursplot->objectsText) g_free(contoursplot->objectsText);
	contoursplot->objectsText = NULL;
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/********************************************************************************/
static void activate_entry_object_text(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* tlabel;
	G_CONST_RETURN gchar* tangle;
	GtkWidget* contoursplot = NULL;
	GtkWidget* window = NULL;
	GtkWidget* entry_label = NULL;
	GtkWidget* entry_angle = NULL;
	ContoursPlotObjectText* objectText = NULL;


	if(!entry) return;
	if(!GTK_IS_WIDGET(entry)) return;
	if(!user_data || !G_IS_OBJECT(user_data)) return;

	contoursplot = GTK_WIDGET(user_data);
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
		add_object_text(GABEDIT_ContoursPLOT(contoursplot), objectText->x, objectText->y, angle, tlabel);
		g_free(objectText);
	}
	else
	{
		gdouble angle = 0;
		if(tangle && strlen(tangle)>0) angle = atof(tangle)/180.0*M_PI;
		if(objectText->str) g_free(objectText->str);
		if(objectText->pango) g_object_unref(G_OBJECT(objectText->pango));
		set_object_text(GABEDIT_ContoursPLOT(contoursplot), objectText,  objectText->x, objectText->y, angle, tlabel);
	}
	if(window)gtk_widget_destroy(window);
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
static void add_set_object_text_dialog(GtkWidget* contoursplot, gint i, gdouble x, gdouble y)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* parentWindow = NULL;
	GtkWidget* frame = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* vbox_frame = NULL;
	GtkWidget* label = NULL;
	GtkWidget* entry_label = NULL;
	GtkWidget* entry_angle = NULL;
	ContoursPlotObjectText* objectText = NULL;

	if(i>-1 && i<GABEDIT_ContoursPLOT(contoursplot)->nObjectsText)
	{
		objectText = &GABEDIT_ContoursPLOT(contoursplot)->objectsText[i];
	}
	else
	{
		objectText = g_malloc(sizeof(ContoursPlotObjectText));
		objectText->x = x;
		objectText->y = y;
		objectText->str = NULL;
		objectText->pango = NULL;
	}

	gtk_window_set_title (GTK_WINDOW (window), _("Set label"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)contoursplot);
	
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
	if(i>-1 && i<GABEDIT_ContoursPLOT(contoursplot)->nObjectsText)
	{
		gtk_entry_set_text(GTK_ENTRY(entry_label),GABEDIT_ContoursPLOT(contoursplot)->objectsText[i].str);
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
	g_signal_connect (G_OBJECT (entry_label), "activate", (GCallback)activate_entry_object_text, contoursplot);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_frame), hbox, FALSE, FALSE, 2);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Angle : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_angle = gtk_entry_new();
	if(i>-1 && i<GABEDIT_ContoursPLOT(contoursplot)->nObjectsText)
	{
		gchar* tmp = g_strdup_printf("%f", GABEDIT_ContoursPLOT(contoursplot)->objectsText[i].angle/M_PI*180.0);
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
	g_signal_connect (G_OBJECT (entry_angle), "activate", (GCallback)activate_entry_object_text, contoursplot);

	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
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
	GABEDIT_ContoursPLOT(widget)->t_key_pressed = TRUE;
}
/****************************************************************************************/
static void set_object_line_pixels(GabeditContoursPlot *contoursplot, ContoursPlotObjectLine* objectLine)
{
	if(!objectLine) return;
	value2pixel(contoursplot, objectLine->x1,objectLine->y1, &objectLine->x1i, &objectLine->y1i);
	objectLine->y1i=contoursplot->plotting_rect.height-objectLine->y1i;
	objectLine->x1i += contoursplot->plotting_rect.x;
	objectLine->y1i += contoursplot->plotting_rect.y;
	value2pixel(contoursplot, objectLine->x2,objectLine->y2, &objectLine->x2i, &objectLine->y2i);
	objectLine->y2i=contoursplot->plotting_rect.height-objectLine->y2i;
	objectLine->x2i += contoursplot->plotting_rect.x;
	objectLine->y2i += contoursplot->plotting_rect.y;
}
/****************************************************************************************/
static void set_object_line(GabeditContoursPlot *contoursplot, ContoursPlotObjectLine* objectLine, 
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
	set_object_line_pixels(contoursplot, objectLine);
}
/****************************************************************************************/
static void add_object_line(GabeditContoursPlot *contoursplot, 
		gdouble x1, gdouble y1, 
		gdouble x2, gdouble y2
		)
{
  	gint width = 1;
  	gint arrow_size = 0;
  	GdkColor color;
  	GdkLineStyle style = GDK_LINE_SOLID;
	gint i;
	contoursplot->nObjectsLine++;
	if(contoursplot->nObjectsLine==1) contoursplot->objectsLine = g_malloc(sizeof(ContoursPlotObjectLine));
	else contoursplot->objectsLine = g_realloc(contoursplot->objectsLine,contoursplot->nObjectsLine*sizeof(ContoursPlotObjectLine));
	i = contoursplot->nObjectsLine-1;

	if(contoursplot->nObjectsLine==1)
	{
		GdkGCValues values;
		GdkColormap *colormap;
		gdk_gc_get_values(contoursplot->lines_gc, &values);
   		colormap  = gdk_window_get_colormap(GTK_WIDGET(contoursplot)->window);
        	gdk_colormap_query_color(colormap, values.foreground.pixel,&color);
		style = values.line_style;
		width = values.line_width;
	}
	else
	{
		color = contoursplot->objectsLine[contoursplot->nObjectsLine-2].color;
		style = contoursplot->objectsLine[contoursplot->nObjectsLine-2].style;
		width = contoursplot->objectsLine[contoursplot->nObjectsLine-2].width;
		arrow_size = contoursplot->objectsLine[contoursplot->nObjectsLine-2].arrow_size;
	}

	set_object_line(contoursplot, &contoursplot->objectsLine[i],  x1,  y1, x2, y2,
			width,
			arrow_size,
			color,
			style);
}
/****************************************************************************************/
static void reset_object_line_pixels(GabeditContoursPlot *contoursplot)
{
	gint i;
	if(!contoursplot) return;
	for(i=0;i<contoursplot->nObjectsLine;i++)
		set_object_line_pixels(contoursplot, &contoursplot->objectsLine[i]);
}
/****************************************************************************************/
static gint get_object_line_num(GabeditContoursPlot *contoursplot, gint xi, gint yi)
{
	gint i;
	if(!contoursplot) return -1;
	for(i=0;i<contoursplot->nObjectsLine;i++)
	{
		gint d = get_distance_M_AB(contoursplot,xi,yi, 
				contoursplot->objectsLine[i].x1i,contoursplot->objectsLine[i].y1i,
				contoursplot->objectsLine[i].x2i,contoursplot->objectsLine[i].y2i);
		if(d<5 || d<contoursplot->objectsLine[i].width) return i;
	}
	return -1;
}
/****************************************************************************************/
static void delete_object_line(GtkWidget *widget, gint i)
{
	gint j;
	GabeditContoursPlot *contoursplot = NULL;
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	contoursplot = GABEDIT_ContoursPLOT(widget);
	if(i<0||i>=contoursplot->nObjectsLine) return;
	for(j=i;j<contoursplot->nObjectsLine-1;j++)
		contoursplot->objectsLine[j] = contoursplot->objectsLine[j+1];
	contoursplot->nObjectsLine--;
	if(contoursplot->nObjectsLine<1) 
	{
		contoursplot->nObjectsLine = 0;
		if(contoursplot->objectsLine) g_free(contoursplot->objectsLine);
		contoursplot->objectsLine = NULL;
	}
	else
	{
		contoursplot->objectsLine = g_realloc(contoursplot->objectsLine,contoursplot->nObjectsLine*sizeof(ContoursPlotObjectLine));
	}

}
/****************************************************************************************/
static void delete_objects_line(GtkWidget *widget)
{
	GabeditContoursPlot *contoursplot = NULL;
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	contoursplot = GABEDIT_ContoursPLOT(widget);
	contoursplot->nObjectsLine = 0;
	if(contoursplot->objectsLine) g_free(contoursplot->objectsLine);
	contoursplot->objectsLine = NULL;
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
static void begin_insert_objects_line(GtkWidget *widget)
{
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	GABEDIT_ContoursPLOT(widget)->l_key_pressed = TRUE;
}
/****************************************************************************************/
static void spin_line_width_changed_value_object(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContoursPlotObjectLine* objectLine = g_object_get_data(G_OBJECT(spinbutton),"ObjectLine");
  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
		if(objectLine) objectLine->width = gtk_spin_button_get_value(spinbutton);
		else
		{
			gint i;
			for (i=0; i<GABEDIT_ContoursPLOT (contoursplot)->nObjectsLine;i++)
			{
				GABEDIT_ContoursPLOT (contoursplot)->objectsLine[i].width = gtk_spin_button_get_value(spinbutton);
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/****************************************************************************************/
static void spin_line_color_changed_value_object(GtkColorButton  *colorbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContoursPlotObjectLine* objectLine = g_object_get_data(G_OBJECT(colorbutton),"ObjectLine");
		GdkColor c;
		gtk_color_button_get_color (colorbutton, &c);

  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

		if(objectLine) objectLine->color = c;
		else
		{
			gint i;
			for (i=0; i<GABEDIT_ContoursPLOT (contoursplot)->nObjectsLine;i++)
			{
				GABEDIT_ContoursPLOT (contoursplot)->objectsLine[i].color = c;
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
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
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContoursPlotObjectLine* objectLine = g_object_get_data(G_OBJECT(combobox),"ObjectLine");
  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

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
			for (i=0; i<GABEDIT_ContoursPLOT (contoursplot)->nObjectsLine;i++)
			{
				if (!strcmp(d,"Solid") ) {  GABEDIT_ContoursPLOT (contoursplot)->objectsLine[i].style = GDK_LINE_SOLID; }
				else if (!strcmp(d,"On-Off dashed") ) {GABEDIT_ContoursPLOT (contoursplot)->objectsLine[i].style = GDK_LINE_ON_OFF_DASH; }
				else if (!strcmp(d,"Double dashed") ) { GABEDIT_ContoursPLOT (contoursplot)->objectsLine[i].style = GDK_LINE_DOUBLE_DASH;}
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/****************************************************************************************/
static void spin_arrow_size_changed_value_object(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContoursPlotObjectLine* objectLine = g_object_get_data(G_OBJECT(spinbutton),"ObjectLine");

  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

		if(objectLine) objectLine->arrow_size = gtk_spin_button_get_value(spinbutton);
		else
		{
			gint i;
			for (i=0; i<GABEDIT_ContoursPLOT (contoursplot)->nObjectsLine;i++)
			{
				GABEDIT_ContoursPLOT (contoursplot)->objectsLine[i].arrow_size = gtk_spin_button_get_value(spinbutton);
			}
		}
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
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
static void set_object_line_dialog(GabeditContoursPlot* contoursplot, gint i)
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
	ContoursPlotObjectLine* objectLine = NULL;
	GdkLineStyle line_style =  GDK_LINE_SOLID;

	if(i>=0 && i<=contoursplot->nObjectsLine-1) objectLine = &contoursplot->objectsLine[i];
	else return;
	line_style =  objectLine->style;

	gtk_window_set_title (GTK_WINDOW (window), _("Set line options"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)contoursplot);

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
	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(spin_line_width_changed_value_object), contoursplot);

	g_object_set_data(G_OBJECT (button), "ObjectLine", objectLine);
	g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(spin_line_color_changed_value_object), contoursplot);

	g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(combo_line_style_changed_value_object), contoursplot);
	g_object_set_data(G_OBJECT (combo), "ObjectLine", objectLine);

	g_object_set_data(G_OBJECT (spin_arrow), "ObjectLine", objectLine);
	g_signal_connect(G_OBJECT(spin_arrow), "value-changed", G_CALLBACK(spin_arrow_size_changed_value_object), contoursplot);


	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
	/* list_utf8();*/
	

}
/**************************************************************************************************/
static void set_object_image_relative(GabeditContoursPlot *contoursplotplot, ContoursPlotObjectImage* objectImage)
{
	gdouble f = (gdouble)GTK_WIDGET(contoursplotplot)->allocation.width;
  	gint xi=objectImage->xi-contoursplotplot->plotting_rect.x;
  	gint yi=contoursplotplot->plotting_rect.y+contoursplotplot->plotting_rect.height-objectImage->yi; 
	if(f>(gdouble)GTK_WIDGET(contoursplotplot)->allocation.height) f = (gdouble)GTK_WIDGET(contoursplotplot)->allocation.height;
	objectImage->width = (gdouble)objectImage->widthi/f;
	objectImage->height = (gdouble)objectImage->heighti/f;
	/*
	objectImage->x = (gdouble)objectImage->xi/f;
	objectImage->y = (gdouble)objectImage->yi/f;
	*/
        pixel2value(contoursplotplot, xi, yi, &objectImage->x, &objectImage->y);
	/* printf("contoursplot = %f %f\n",objectImage->x, objectImage->y);*/
}
/**************************************************************************************************/
static void set_object_image_pixels(GabeditContoursPlot *contoursplotplot, ContoursPlotObjectImage* objectImage)
{
	gint xi,yi;
	gdouble f = (gdouble)GTK_WIDGET(contoursplotplot)->allocation.width;
	if(f>(gdouble)GTK_WIDGET(contoursplotplot)->allocation.height) f = (gdouble)GTK_WIDGET(contoursplotplot)->allocation.height;
	objectImage->widthi = (gint)(objectImage->width*f);
	objectImage->heighti = (gint)(objectImage->height*f);
	/*
	objectImage->xi = (gint)(objectImage->x*f);
	objectImage->yi = (gint)(objectImage->y*f);
	*/

       	value2pixel(contoursplotplot, objectImage->x, objectImage->y, &xi, &yi);
       	objectImage->xi = xi;
	objectImage->yi = yi;
	objectImage->yi=contoursplotplot->plotting_rect.height-objectImage->yi;
	objectImage->xi += contoursplotplot->plotting_rect.x;
	objectImage->yi += contoursplotplot->plotting_rect.y;
}
/****************************************************************************************/
static void set_object_image(GabeditContoursPlot *contoursplotplot, ContoursPlotObjectImage* objectImage, gint xi, gint yi, gint w, gint h)
{
	objectImage->xi = xi;
	objectImage->yi = yi;
	objectImage->widthi = w;
	objectImage->heighti = h;
	set_object_image_relative(contoursplotplot, objectImage);
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
static ContoursPlotObjectImage get_object_image(GabeditContoursPlot *contoursplotplot, gint xi, gint yi, G_CONST_RETURN gchar* fileName)
{
	gint w;
	gint h;
	gint nw;
	gint nh;
	gdouble fw = 1;
	gdouble fh = 1;
	gdouble f = 1;
	ContoursPlotObjectImage objectImage;
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

	nw = contoursplotplot->plotting_rect.width-xi;
	nh = contoursplotplot->plotting_rect.height-yi;
	if(nw<w && nw>0) fw = nw/(gdouble)w;
	if(nh<h&&nh>0) fh = nh/(gdouble)h;
	f = fw;
	if(f>fh) f = fh;

	objectImage.widthi = (gint)(w*f);
	objectImage.heighti = (gint)(h*f);
	set_object_image_relative(contoursplotplot, &objectImage);

	/* printf("w = %d h = %d\n",w, h);*/
	return objectImage;
}
/****************************************************************************************/
static void add_object_image(GabeditContoursPlot *contoursplotplot, gint xi, gint yi, gint width, gint height, G_CONST_RETURN gchar* fileName)
{
	ContoursPlotObjectImage objectImage = get_object_image(contoursplotplot, xi, yi, fileName);
	if(!objectImage.image) return;
	contoursplotplot->nObjectsImage++;
	if(contoursplotplot->nObjectsImage==1) contoursplotplot->objectsImage = g_malloc(sizeof(ContoursPlotObjectImage));
	else contoursplotplot->objectsImage = g_realloc(contoursplotplot->objectsImage,contoursplotplot->nObjectsImage*sizeof(ContoursPlotObjectImage));
	if(width>0 && height>0)
	{
		objectImage.widthi=width;
		objectImage.heighti=height;
	}
	contoursplotplot->objectsImage[contoursplotplot->nObjectsImage-1] = objectImage;
}
/****************************************************************************************/
static gint get_object_image_num(GabeditContoursPlot *contoursplotplot, gint xi, gint yi)
{
	gint i;
	if(!contoursplotplot) return -1;
	for(i=0;i<contoursplotplot->nObjectsImage;i++)
	{
		if(xi>=(gint)contoursplotplot->objectsImage[i].xi && yi>=contoursplotplot->objectsImage[i].yi
		&& xi<=contoursplotplot->objectsImage[i].xi+contoursplotplot->objectsImage[i].widthi 
		&& yi<=contoursplotplot->objectsImage[i].yi+contoursplotplot->objectsImage[i].heighti)
			return i;
	}
	return -1;
}
/****************************************************************************************/
static void delete_object_image(GtkWidget *widget, gint i)
{
	gint j;
	GabeditContoursPlot *contoursplotplot = NULL;
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	contoursplotplot = GABEDIT_ContoursPLOT(widget);
	if(i<0||i>=contoursplotplot->nObjectsImage) return;
	if(contoursplotplot->objectsImage[i].fileName) g_free(contoursplotplot->objectsImage[i].fileName);
	if(contoursplotplot->objectsImage[i].image) cairo_surface_destroy (contoursplotplot->objectsImage[i].image);
	for(j=i;j<contoursplotplot->nObjectsImage-1;j++)
		contoursplotplot->objectsImage[j] = contoursplotplot->objectsImage[j+1];
	contoursplotplot->nObjectsImage--;
	if(contoursplotplot->nObjectsImage<1) 
	{
		contoursplotplot->nObjectsImage = 0;
		if(contoursplotplot->objectsImage) g_free(contoursplotplot->objectsImage);
		contoursplotplot->objectsImage = NULL;
	}
	else
	{
		contoursplotplot->objectsImage = g_realloc(contoursplotplot->objectsImage,contoursplotplot->nObjectsImage*sizeof(ContoursPlotObjectImage));
	}

}
/****************************************************************************************/
static void delete_objects_image(GtkWidget *widget)
{
	GabeditContoursPlot *contoursplotplot = NULL;
	gint i;
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	contoursplotplot = GABEDIT_ContoursPLOT(widget);
	for(i=0;i<contoursplotplot->nObjectsImage;i++)
	{
		if(contoursplotplot->objectsImage[i].fileName) g_free(contoursplotplot->objectsImage[i].fileName);
		if(contoursplotplot->objectsImage[i].image) cairo_surface_destroy (contoursplotplot->objectsImage[i].image);
	}
	contoursplotplot->nObjectsImage = 0;
	if(contoursplotplot->objectsImage) g_free(contoursplotplot->objectsImage);
	contoursplotplot->objectsImage = NULL;
	gtk_widget_queue_draw(GTK_WIDGET(contoursplotplot));
}
/********************************************************************************/
static gboolean read_image_png(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
	GtkWidget* contoursplotplot = NULL;
	ContoursPlotObjectImage* objectImage = NULL;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gtk_file_chooser_get_filename(filesel);
	contoursplotplot = g_object_get_data(G_OBJECT (filesel), "ContoursPLOT");
	objectImage = g_object_get_data(G_OBJECT(filesel),"ObjectImage");
	if(objectImage && !objectImage->image)
	{
		add_object_image(GABEDIT_ContoursPLOT(contoursplotplot), objectImage->xi, objectImage->yi, -1, -1, fileName);
		g_free(objectImage);
	}
	gtk_widget_queue_draw(GTK_WIDGET(contoursplotplot));
	return TRUE;

}
/********************************************************************************/
static void activate_entry_object_image(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	GtkWidget* contoursplotplot = NULL;
	GtkWidget* window = NULL;
	ContoursPlotObjectImage* objectImage = NULL;


	if(!GTK_IS_WIDGET(entry)) return;
	if(!user_data || !G_IS_OBJECT(user_data)) return;

	contoursplotplot = GTK_WIDGET(user_data);
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
	gtk_widget_queue_draw(GTK_WIDGET(contoursplotplot));
}
/********************************************************************************/
static void add_set_object_image_dialog(GtkWidget* contoursplotplot, gint i, gint xi, gint yi)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.png","*",NULL}; 
	GtkWidget* filesel= NULL;
	ContoursPlotObjectImage* objectImage = NULL;

	if(i>-1 && i<GABEDIT_ContoursPLOT(contoursplotplot)->nObjectsImage)
	{
		GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		GtkWidget* frame = NULL;
		GtkWidget* hbox = NULL;
		GtkWidget* vbox_frame = NULL;
		GtkWidget* label = NULL;
		GtkWidget* entry_label = NULL;

		objectImage = &GABEDIT_ContoursPLOT(contoursplotplot)->objectsImage[i];

		gtk_window_set_title (GTK_WINDOW (window), _("Scale image"));
		gtk_container_set_border_width (GTK_CONTAINER (window), 10);
		gtk_window_set_modal (GTK_WINDOW (window), TRUE);

		g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)contoursplotplot);
	
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
		g_signal_connect (G_OBJECT (entry_label), "activate", (GCallback)activate_entry_object_image, contoursplotplot);

		parentWindow = get_parent_window(GTK_WIDGET(contoursplotplot));
		if(parentWindow)
		{
			gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
		}
		gtk_widget_show(window); 
	}
	else if(i==-1)
	{
		objectImage = g_malloc(sizeof(ContoursPlotObjectImage));
		objectImage->xi = xi;
		objectImage->yi = yi;
		objectImage->fileName = NULL;
		objectImage->image = NULL;
		parentWindow = get_parent_window(GTK_WIDGET(contoursplotplot));
		filesel= new_file_chooser_open(parentWindow, 
				(GCallback *)read_image_png, 
				_("Read image from a png file"), 
				patternsfiles);
		gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
		g_object_set_data(G_OBJECT (filesel), "ContoursPLOT", contoursplotplot);
		g_object_set_data(G_OBJECT(filesel),"ObjectImage", objectImage);
	}
	else
		add_object_image(GABEDIT_ContoursPLOT(contoursplotplot), xi, yi, -1, -1, NULL);

}
/****************************************************************************************/
static void reset_object_image_pixels(GabeditContoursPlot *contoursplotplot)
{
	gint i;
	if(!contoursplotplot) return;
	for(i=0;i<contoursplotplot->nObjectsImage;i++)
		set_object_image_pixels(contoursplotplot, &contoursplotplot->objectsImage[i]);
}
/****************************************************************************************/
/*
static void reset_object_image_relative(GabeditContoursPlot *contoursplotplot)
{
	gint i;
	if(!contoursplotplot) return;
	for(i=0;i<contoursplotplot->nObjectsImage;i++)
		set_object_image_relative(contoursplotplot, &contoursplotplot->objectsImage[i]);
}
*/
/****************************************************************************************/
static void insert_objects_image_from_clipboard(GtkWidget *widget)
{
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	add_object_image(GABEDIT_ContoursPLOT(widget), 0, 0, -1, -1, NULL);
        gtk_widget_queue_draw(widget);
}
/****************************************************************************************/
static void begin_insert_objects_image(GtkWidget *widget)
{
	if(!widget) return;
	if(!GTK_IS_WIDGET(widget)) return;
	GABEDIT_ContoursPLOT(widget)->i_key_pressed = TRUE;
}
/********************************************************************************/
static gboolean read_data(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
 	gboolean OK;
 	FILE *fd;
	gint xsize = 0;
	gint ysize = 0;
	gdouble xmin = 0;
	gdouble ymin = 0;
	gdouble xmax = 0;
	gdouble ymax = 0;
	gdouble* zValues = NULL;
	GtkWidget* contoursplot = NULL;
	gint size = 0;
	gint i;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gtk_file_chooser_get_filename(filesel);
	contoursplot = g_object_get_data(G_OBJECT (filesel), "ContoursPLOT");

 	fd = fopen(fileName, "rb");
 	OK=FALSE;
	if(!fd)
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
	if(fd)
	{
		if(3==fscanf(fd,"%d %lf %lf",&xsize,&xmin,&xmax))
		{
			if(3==fscanf(fd,"%d %lf %lf",&ysize,&ymin,&ymax)) OK = TRUE;
		}
	}
	size = xsize*ysize;
	if(size<1)
	{
		GtkWidget* dialog = NULL;
		gchar* tmp = g_strdup_printf(_("Sorry, I cannot read %s file"),fileName);
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
	if(OK) zValues = g_malloc(size*sizeof(gdouble));
	if(!zValues)
	{
		GtkWidget* dialog = NULL;
		gchar* tmp = g_strdup_printf(_("Sorry, I cannot read %s file"),fileName);
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
	for(i=0;OK && i<xsize*ysize;i++)
		if(1!=fscanf(fd,"%lf",&zValues[i])) OK = FALSE;

	if(!OK)
	{
		GtkWidget* parentWidget = get_parent_window(GTK_WIDGET(contoursplot));
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
	else 
	{
		add_new_data(contoursplot, xsize, xmin, xmax, ysize, ymin, ymax, zValues);
		if(GABEDIT_ContoursPLOT(contoursplot)->data_list)
		{
			GList *current_node;
			current_node=g_list_last(GABEDIT_ContoursPLOT(contoursplot)->data_list);
			if(current_node)
			{
				ContoursPlotData* data = (ContoursPlotData*)current_node->data;  
				create_contours(contoursplot,data);
			}
		}
	}

	if(zValues) g_free(zValues);
	fclose(fd);
	return TRUE;

}
/********************************************************************************/
static void read_data_dlg(GtkWidget* contoursplot)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.txt","*",NULL}; 
	GtkWidget* filesel= NULL;

	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
	filesel= new_file_chooser_open(parentWindow, 
			(GCallback *)read_data, 
			_("Read data from an ASCII grid file(xsize, xmin, xmax, ysize, ymin, ymax, Zvalues)"), 
			patternsfiles);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	g_object_set_data(G_OBJECT (filesel), "ContoursPLOT", contoursplot);
}
/********************************************************************************/
static gboolean save_data(GtkFileChooser *filesel, gint response_id)
{
	gchar *fileName;
 	FILE *file;
	ContoursPlotData* data = NULL;
	gint loop;

	if(response_id != GTK_RESPONSE_OK) return FALSE;
 	fileName = gtk_file_chooser_get_filename(filesel);
	data = g_object_get_data(G_OBJECT (filesel), "CurentData");
	if(!data) return FALSE;

 	file = fopen(fileName, "w");

	fprintf(file, "%d %f %f\n",data->xsize, data->xmin, data->xmax);
	fprintf(file, "%d %f %f\n",data->ysize, data->ymin, data->ymax);
	for(loop=0;loop<data->xsize*data->ysize; loop++)
		fprintf(file, "%f\n",data->zValues[loop]);
	fclose(file);
	return TRUE;

}
/********************************************************************************/
static void auto_range_activate(GtkWidget* buttonAutoRange, GtkWidget* contoursplot)
{
	ContoursPlotData* data = NULL;
	data = g_object_get_data(G_OBJECT (buttonAutoRange), "CurentData");
	if(contoursplot) gabedit_contoursplot_set_autorange(GABEDIT_ContoursPLOT(contoursplot), data);
}
/********************************************************************************/
static void save_data_dlg(GtkWidget* buttonSave, GtkWidget* contoursplot)
{
	GtkWidget* parentWindow = NULL;
	gchar* patternsfiles[] = {"*.txt","*",NULL}; 
	GtkWidget* filesel= NULL;
	gchar* tmp = NULL;
	gpointer* data = NULL;

	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
	filesel= new_file_chooser_save(parentWindow, 
			(GCallback *)save_data, 
			_("Save data in an ASCII grid file(xsize, xmin, xmax, ysize, ymin, ymax, Zvalues)"), 
			patternsfiles);
	gtk_window_set_modal (GTK_WINDOW (filesel), TRUE);
	data = g_object_get_data(G_OBJECT (buttonSave), "CurentData");
	g_object_set_data(G_OBJECT (filesel), "CurentData", data);
	tmp = g_strdup_printf("%s%s%s",g_get_current_dir(),G_DIR_SEPARATOR_S,"contoursplotData.txt");
	gtk_file_chooser_set_filename ((GtkFileChooser *)filesel, tmp);
	if(tmp) g_free(tmp);
	gtk_file_chooser_set_current_name ((GtkFileChooser *)filesel, "contoursplotData.txt");
}
/********************************************************************************/
static gboolean remove_data(GtkWidget* buttonRemove, gint response_id)
{
	ContoursPlotData* data = NULL;
	GtkWidget*window = NULL;
	GtkWidget*contoursplot = NULL;

	if(response_id != GTK_RESPONSE_YES) return FALSE;
	data = g_object_get_data(G_OBJECT (buttonRemove), "CurentData");
	window = g_object_get_data(G_OBJECT (buttonRemove), "Window");
	contoursplot = g_object_get_data(G_OBJECT (buttonRemove), "ContoursPLOT");
	if(contoursplot && data) 
	{
		gabedit_contoursplot_remove_data(GABEDIT_ContoursPLOT(contoursplot), data);
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
	if(window) gtk_widget_destroy(window);
	return TRUE;
}
/********************************************************************************/
static void remove_data_dlg(GtkWidget* buttonRemove, GtkWidget* contoursplot)
{
	GtkWidget* parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
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
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		gint fontSize = gtk_spin_button_get_value(spinbutton);
		gchar tmp[100];
		sprintf(tmp,"sans %d",fontSize);
		gabedit_contoursplot_set_font (GABEDIT_ContoursPLOT(contoursplot), tmp);
		reset_object_text_pango(GABEDIT_ContoursPLOT(contoursplot));
	}
}
/****************************************************************************************/
static void add_font_size_frame(GtkWidget* hbox, GtkWidget* contoursplot)
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

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_font_size), GABEDIT_ContoursPLOT(contoursplot)->font_size);
	g_signal_connect(G_OBJECT(spin_font_size), "value-changed", G_CALLBACK(spin_font_changed_value), contoursplot);
}
/****************************************************************************************/
static void set_font_size_dialog(GtkWidget* contoursplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* hbox = NULL;
	GtkWidget* parentWindow = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set font size"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)contoursplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);

	add_font_size_frame(hbox, contoursplot);


	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
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
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		gabedit_contoursplot_set_x_legends_digits(GABEDIT_ContoursPLOT(contoursplot), gtk_spin_button_get_value(spinbutton));
	}
}
/****************************************************************************************/
static void spin_y_digits_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		gabedit_contoursplot_set_y_legends_digits(GABEDIT_ContoursPLOT(contoursplot), gtk_spin_button_get_value(spinbutton));
	}
}
/****************************************************************************************/
static void add_digits_frame(GtkWidget* hbox, GtkWidget* contoursplot)
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

	label=gtk_label_new(" X ");
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	x_spin_digits=gtk_spin_button_new_with_range(0, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), x_spin_digits, TRUE, FALSE, 2);
	gtk_widget_show(x_spin_digits);

	label=gtk_label_new(" Y ");
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 
	
	y_spin_digits=gtk_spin_button_new_with_range(0, 30, 1);
	gtk_box_pack_start(GTK_BOX(hbox), y_spin_digits, TRUE, FALSE, 2);
	gtk_widget_show(y_spin_digits);


	gtk_spin_button_set_value(GTK_SPIN_BUTTON(x_spin_digits), GABEDIT_ContoursPLOT(contoursplot)->x_legends_digits);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(y_spin_digits), GABEDIT_ContoursPLOT(contoursplot)->y_legends_digits);

	g_signal_connect(G_OBJECT(x_spin_digits), "value-changed", G_CALLBACK(spin_x_digits_changed_value), contoursplot);
	g_signal_connect(G_OBJECT(y_spin_digits), "value-changed", G_CALLBACK(spin_y_digits_changed_value), contoursplot);
}
/****************************************************************************************/
static void set_digits_dialog(GtkWidget* contoursplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* hbox = NULL;
	GtkWidget* parentWindow = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set digits"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)contoursplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);
	add_digits_frame(hbox, contoursplot);


	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
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
	GtkWidget* contoursplot = NULL;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	contoursplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	gabedit_contoursplot_set_x_label (GABEDIT_ContoursPLOT(contoursplot), t);
}
/********************************************************************************/
static void activate_entry_ylabel(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	GtkWidget* contoursplot = NULL;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	contoursplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	gabedit_contoursplot_set_y_label (GABEDIT_ContoursPLOT(contoursplot), t);
}
/****************************************************************************************/
static void add_labels_frame(GtkWidget* hbox, GtkWidget* contoursplot)
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

	label=gtk_label_new("X : ");
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	
	entry_x_label = gtk_entry_new();
	gtk_widget_set_size_request(entry_x_label,100,-1);
	if(GABEDIT_ContoursPLOT(contoursplot)->h_label_str)
		gtk_entry_set_text(GTK_ENTRY(entry_x_label),GABEDIT_ContoursPLOT(contoursplot)->h_label_str);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_x_label),"");
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_x_label, FALSE, FALSE, 2);
	gtk_widget_show(entry_x_label);

	label=gtk_label_new("Y : ");
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	
	entry_y_label = gtk_entry_new();
	gtk_widget_set_size_request(entry_y_label,100,-1);
	if(GABEDIT_ContoursPLOT(contoursplot)->v_label_str)
		gtk_entry_set_text(GTK_ENTRY(entry_y_label),GABEDIT_ContoursPLOT(contoursplot)->v_label_str);
	else
		gtk_entry_set_text(GTK_ENTRY(entry_y_label),"");
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_y_label, FALSE, FALSE, 2);
	gtk_widget_show(entry_y_label);

	g_signal_connect (G_OBJECT (entry_x_label), "activate", (GCallback)activate_entry_xlabel, contoursplot);
	g_signal_connect (G_OBJECT (entry_y_label), "activate", (GCallback)activate_entry_ylabel, contoursplot);
}
/****************************************************************************************/
static void set_labels_dialog(GtkWidget* contoursplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* parentWindow = NULL;
	GtkWidget* hbox = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set labels"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)contoursplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);

	add_labels_frame(hbox, contoursplot);


	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
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
	GtkWidget* contoursplot = NULL;
	gdouble xmin;
	gdouble ymin;
	gdouble xmax;
	gdouble ymax;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	contoursplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	a = atof(t);
	gabedit_contoursplot_get_range (GABEDIT_ContoursPLOT(contoursplot), &xmin, &xmax, &ymin, &ymax);

	if(a>= xmax)
	{
		gchar* tmp = g_strdup_printf("%0.3f",xmin);
		gtk_entry_set_text(GTK_ENTRY(entry),tmp);
		g_free(tmp);
		return;
	}
	gabedit_contoursplot_set_range_xmin (GABEDIT_ContoursPLOT(contoursplot), a);
}
/********************************************************************************/
static void activate_entry_xmax(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	gdouble a;
	GtkWidget* contoursplot = NULL;
	gdouble xmin;
	gdouble ymin;
	gdouble xmax;
	gdouble ymax;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	contoursplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	a = atof(t);
	gabedit_contoursplot_get_range (GABEDIT_ContoursPLOT(contoursplot), &xmin, &xmax, &ymin, &ymax);

	if(a<=xmin)
	{
		gchar* tmp = g_strdup_printf("%0.3f",xmax);
		gtk_entry_set_text(GTK_ENTRY(entry),tmp);
		g_free(tmp);
		return;
	}
	gabedit_contoursplot_set_range_xmax (GABEDIT_ContoursPLOT(contoursplot), a);
}
/********************************************************************************/
static void activate_entry_ymin(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	gdouble a;
	GtkWidget* contoursplot = NULL;
	gdouble xmin;
	gdouble ymin;
	gdouble xmax;
	gdouble ymax;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	contoursplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	a = atof(t);
	gabedit_contoursplot_get_range (GABEDIT_ContoursPLOT(contoursplot), &xmin, &xmax, &ymin, &ymax);

	if(a>= ymax)
	{
		gchar* tmp = g_strdup_printf("%0.3f",ymin);
		gtk_entry_set_text(GTK_ENTRY(entry),tmp);
		g_free(tmp);
		return;
	}
	gabedit_contoursplot_set_range_ymin (GABEDIT_ContoursPLOT(contoursplot), a);
}
/********************************************************************************/
static void activate_entry_ymax(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	gdouble a;
	GtkWidget* contoursplot = NULL;
	gdouble xmin;
	gdouble ymin;
	gdouble xmax;
	gdouble ymax;


	if(!user_data || !G_IS_OBJECT(user_data)) return;

	contoursplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	a = atof(t);
	gabedit_contoursplot_get_range (GABEDIT_ContoursPLOT(contoursplot), &xmin, &xmax, &ymin, &ymax);

	if(a<= ymin)
	{
		gchar* tmp = g_strdup_printf("%0.3f",ymax);
		gtk_entry_set_text(GTK_ENTRY(entry),tmp);
		g_free(tmp);
		return;
	}
	gabedit_contoursplot_set_range_ymax (GABEDIT_ContoursPLOT(contoursplot), a);
}
/****************************************************************************************/
static void add_ranges_frame(GtkWidget* hbox, GtkWidget* contoursplot)
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
	sprintf(tmp,"%0.3f",GABEDIT_ContoursPLOT(contoursplot)->xmin);
	gtk_entry_set_text(GTK_ENTRY(entry_x_min),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_x_min, TRUE, FALSE, 2);
	gtk_widget_show(entry_x_min);

	label=gtk_label_new(_("X Max: "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	
	entry_x_max = gtk_entry_new();
	gtk_widget_set_size_request(entry_x_max,50,-1);
	sprintf(tmp,"%0.3f",GABEDIT_ContoursPLOT(contoursplot)->xmax);
	gtk_entry_set_text(GTK_ENTRY(entry_x_max),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_x_max, TRUE, FALSE, 2);
	gtk_widget_show(entry_x_max);

	label=gtk_label_new(_("Y Min: "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	
	entry_y_min = gtk_entry_new();
	gtk_widget_set_size_request(entry_y_min,50,-1);
	sprintf(tmp,"%0.3f",GABEDIT_ContoursPLOT(contoursplot)->ymin);
	gtk_entry_set_text(GTK_ENTRY(entry_y_min),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_y_min, TRUE, FALSE, 2);
	gtk_widget_show(entry_y_min);

	label=gtk_label_new(_("Y Max: "));
	gtk_box_pack_start(GTK_BOX(hbox_frame), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 
	
	entry_y_max = gtk_entry_new();
	gtk_widget_set_size_request(entry_y_max,50,-1);
	sprintf(tmp,"%0.3f",GABEDIT_ContoursPLOT(contoursplot)->ymax);
	gtk_entry_set_text(GTK_ENTRY(entry_y_max),tmp);
	gtk_box_pack_start(GTK_BOX(hbox_frame), entry_y_max, TRUE, FALSE, 2);
	gtk_widget_show(entry_y_max);

	g_signal_connect (G_OBJECT (entry_x_min), "activate", (GCallback)activate_entry_xmin, contoursplot);
	g_signal_connect (G_OBJECT (entry_x_max), "activate", (GCallback)activate_entry_xmax, contoursplot);
	g_signal_connect (G_OBJECT (entry_y_min), "activate", (GCallback)activate_entry_ymin, contoursplot);
	g_signal_connect (G_OBJECT (entry_y_max), "activate", (GCallback)activate_entry_ymax, contoursplot);
}
/****************************************************************************************/
static void set_ranges_dialog(GtkWidget* contoursplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* parentWindow = NULL;
	GtkWidget* hbox = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set ranges"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)contoursplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);

	add_ranges_frame(hbox, contoursplot);


	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
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
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_set_ticks_hminor(GABEDIT_ContoursPLOT(contoursplot), gtk_spin_button_get_value(spinbutton));	
	}
}
/****************************************************************************************/
static void spin_hmajor_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_set_ticks_hmajor(GABEDIT_ContoursPLOT(contoursplot), gtk_spin_button_get_value(spinbutton));
	}
}
/****************************************************************************************/
static void spin_vminor_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_set_ticks_vminor(GABEDIT_ContoursPLOT(contoursplot), gtk_spin_button_get_value(spinbutton));
	}
}
/****************************************************************************************/
static void spin_vmajor_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_set_ticks_vmajor(GABEDIT_ContoursPLOT(contoursplot), gtk_spin_button_get_value(spinbutton));	
	}
}
/****************************************************************************************/
static void spin_length_ticks_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_set_ticks_length(GABEDIT_ContoursPLOT(contoursplot), gtk_spin_button_get_value(spinbutton));	
	}
}
/****************************************************************************************/
static void add_ticks_frame(GtkWidget* hbox, GtkWidget* contoursplot)
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

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_hmajor), GABEDIT_ContoursPLOT(contoursplot)->hmajor_ticks);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_hminor), GABEDIT_ContoursPLOT(contoursplot)->hminor_ticks);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_vmajor), GABEDIT_ContoursPLOT(contoursplot)->vmajor_ticks);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_vminor), GABEDIT_ContoursPLOT(contoursplot)->vminor_ticks);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_length), GABEDIT_ContoursPLOT(contoursplot)->length_ticks);

	g_signal_connect(G_OBJECT(spin_hmajor), "value-changed", G_CALLBACK(spin_hmajor_changed_value), contoursplot);
	g_signal_connect(G_OBJECT(spin_hminor), "value-changed", G_CALLBACK(spin_hminor_changed_value), contoursplot);
	g_signal_connect(G_OBJECT(spin_vmajor), "value-changed", G_CALLBACK(spin_vmajor_changed_value), contoursplot);
	g_signal_connect(G_OBJECT(spin_vminor), "value-changed", G_CALLBACK(spin_vminor_changed_value), contoursplot);
	g_signal_connect(G_OBJECT(spin_length), "value-changed", G_CALLBACK(spin_length_ticks_changed_value), contoursplot);
}
/****************************************************************************************/
static void set_ticks_dialog(GtkWidget* contoursplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* hbox = NULL;
	GtkWidget* parentWindow = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set ticks"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)contoursplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);

	add_ticks_frame(hbox, contoursplot);


	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
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
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_set_margins_right(GABEDIT_ContoursPLOT(contoursplot), gtk_spin_button_get_value(spinbutton));	
	}
}
/****************************************************************************************/
static void spin_left_margins_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_set_margins_left(GABEDIT_ContoursPLOT(contoursplot), gtk_spin_button_get_value(spinbutton));
	}
}
/****************************************************************************************/
static void spin_bottom_margins_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_set_margins_bottom(GABEDIT_ContoursPLOT(contoursplot), gtk_spin_button_get_value(spinbutton));
	}
}
/****************************************************************************************/
static void spin_top_margins_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_set_margins_top(GABEDIT_ContoursPLOT(contoursplot), gtk_spin_button_get_value(spinbutton));	
	}
}
/****************************************************************************************/
static void add_margins_frame(GtkWidget* hbox, GtkWidget* contoursplot)
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

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_left_margins), GABEDIT_ContoursPLOT(contoursplot)->left_margins);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_right_margins), GABEDIT_ContoursPLOT(contoursplot)->right_margins);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_top_margins), GABEDIT_ContoursPLOT(contoursplot)->top_margins);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_bottom_margins), GABEDIT_ContoursPLOT(contoursplot)->bottom_margins);

	g_signal_connect(G_OBJECT(spin_left_margins), "value-changed", G_CALLBACK(spin_left_margins_changed_value), contoursplot);
	g_signal_connect(G_OBJECT(spin_right_margins), "value-changed", G_CALLBACK(spin_right_margins_changed_value), contoursplot);
	g_signal_connect(G_OBJECT(spin_top_margins), "value-changed", G_CALLBACK(spin_top_margins_changed_value), contoursplot);
	g_signal_connect(G_OBJECT(spin_bottom_margins), "value-changed", G_CALLBACK(spin_bottom_margins_changed_value), contoursplot);
}
/****************************************************************************************/
static void set_margins_dialog(GtkWidget* contoursplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* hbox = NULL;
	GtkWidget* parentWindow = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set margins"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)contoursplot);
	
	hbox=gtk_hbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);

	add_margins_frame(hbox, contoursplot);


	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
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
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_reflect_x (GABEDIT_ContoursPLOT(contoursplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void reflect_y_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_reflect_y (GABEDIT_ContoursPLOT(contoursplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void add_reflexion_frame(GtkWidget* hbox, GtkWidget* contoursplot)
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

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(reflect_x_button), GABEDIT_ContoursPLOT(contoursplot)->reflect_x);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(reflect_y_button), GABEDIT_ContoursPLOT(contoursplot)->reflect_y);
	
	g_signal_connect (G_OBJECT (reflect_x_button), "toggled", (GCallback)reflect_x_toggled, contoursplot);
	g_signal_connect (G_OBJECT (reflect_y_button), "toggled", (GCallback)reflect_y_toggled, contoursplot);
}
/****************************************************************************************/
static void show_left_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_show_left_legends (GABEDIT_ContoursPLOT(contoursplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void show_right_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_show_right_legends (GABEDIT_ContoursPLOT(contoursplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void show_top_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_show_top_legends (GABEDIT_ContoursPLOT(contoursplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void show_bottom_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_show_bottom_legends (GABEDIT_ContoursPLOT(contoursplot), gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void add_legends_frame(GtkWidget* hbox, GtkWidget* contoursplot)
{
	GtkWidget* frame = NULL;
	GtkWidget* hbox_frame = NULL;
	GtkWidget* show_left;
	GtkWidget* show_right;
	GtkWidget* show_top;
	GtkWidget* show_bottom;

	frame=gtk_frame_new("Legends");
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

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_left), GABEDIT_ContoursPLOT(contoursplot)->show_left_legends);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_right), GABEDIT_ContoursPLOT(contoursplot)->show_right_legends);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_top), GABEDIT_ContoursPLOT(contoursplot)->show_top_legends);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(show_bottom), GABEDIT_ContoursPLOT(contoursplot)->show_bottom_legends);
	
	g_signal_connect (G_OBJECT (show_left), "toggled", (GCallback)show_left_toggled, contoursplot);
	g_signal_connect (G_OBJECT (show_right), "toggled", (GCallback)show_right_toggled, contoursplot);
	g_signal_connect (G_OBJECT (show_top), "toggled", (GCallback)show_top_toggled, contoursplot);
	g_signal_connect (G_OBJECT (show_bottom), "toggled", (GCallback)show_bottom_toggled, contoursplot);
}
/****************************************************************************************/
static void h_major_grids_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_HMAJOR_GRID, gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void h_minor_grids_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_HMINOR_GRID, gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void v_major_grids_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_VMAJOR_GRID, gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void v_minor_grids_toggled(GtkToggleButton *togglebutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
	GtkWidget* contoursplot = GTK_WIDGET(user_data);
	gabedit_contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_VMINOR_GRID, gtk_toggle_button_get_active(togglebutton));
	}
}
/****************************************************************************************/
static void add_grid_frame(GtkWidget* hbox, GtkWidget* contoursplot)
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

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(h_major), GABEDIT_ContoursPLOT(contoursplot)->hmajor_grid);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(h_minor), GABEDIT_ContoursPLOT(contoursplot)->hminor_grid);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(v_major), GABEDIT_ContoursPLOT(contoursplot)->vmajor_grid);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(v_minor), GABEDIT_ContoursPLOT(contoursplot)->vminor_grid);
	
	g_signal_connect (G_OBJECT (h_major), "toggled", (GCallback)h_major_grids_toggled, contoursplot);
	g_signal_connect (G_OBJECT (h_minor), "toggled", (GCallback)h_minor_grids_toggled, contoursplot);
	g_signal_connect (G_OBJECT (v_major), "toggled", (GCallback)v_major_grids_toggled, contoursplot);
	g_signal_connect (G_OBJECT (v_minor), "toggled", (GCallback)v_minor_grids_toggled, contoursplot);
}
/****************************************************************************************/
static void set_all_dialog(GtkWidget* contoursplot)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* parentWindow = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* vbox = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)contoursplot);
	
	vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_widget_show(vbox);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 1);
	add_ranges_frame(hbox, contoursplot);
	gtk_widget_show(hbox); 

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 1);
	add_margins_frame(hbox, contoursplot);
	gtk_widget_show(hbox); 

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 1);
	add_labels_frame(hbox, contoursplot);
	add_font_size_frame(hbox, contoursplot);
	add_digits_frame(hbox, contoursplot);
	gtk_widget_show(hbox); 

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 1);
	add_ticks_frame(hbox, contoursplot);
	gtk_widget_show(hbox); 

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 1);
	add_reflexion_frame(hbox, contoursplot);
	add_legends_frame(hbox, contoursplot);
	add_grid_frame(hbox, contoursplot);
	gtk_widget_show(hbox); 


	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
}
/****************************************************************************************/
static void gabedit_contoursplot_save_gabedit_lines(GabeditContoursPlot *contoursplot, FILE* file)
{
	gint i;
	fprintf(file,"lines %d\n", contoursplot->nObjectsLine);
	for(i=0;i<contoursplot->nObjectsLine;i++)
	{
		fprintf(file,"%lf %lf %lf %lf %d %d %lf %lf %lf %d\n", 
				contoursplot->objectsLine[i].x1, 
				contoursplot->objectsLine[i].y1, 
				contoursplot->objectsLine[i].x2, 
				contoursplot->objectsLine[i].y2, 
				contoursplot->objectsLine[i].width, 
				contoursplot->objectsLine[i].arrow_size, 
				SCALE(contoursplot->objectsLine[i].color.red),
				SCALE(contoursplot->objectsLine[i].color.green),
				SCALE(contoursplot->objectsLine[i].color.blue),
				contoursplot->objectsLine[i].style
		       ); 
	}
}
/****************************************************************************************/
static void gabedit_contoursplot_save_gabedit_texts(GabeditContoursPlot *contoursplot, FILE* file)
{
	gint i;
	fprintf(file,"texts %d\n", contoursplot->nObjectsText);
	for(i=0;i<contoursplot->nObjectsText;i++)
	{
		fprintf(file,"%lf %lf %lf\n", 
				contoursplot->objectsText[i].x, 
				contoursplot->objectsText[i].y,
				contoursplot->objectsText[i].angle
		       ); 
		fprintf(file,"%s\n", contoursplot->objectsText[i].str);
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
static void gabedit_contoursplot_save_gabedit_images(GabeditContoursPlot *contoursplot, FILE* file, gchar* fileName)
{
	gint i;
	gchar *fn;
	fprintf(file,"images %d\n", contoursplot->nObjectsImage);
	for(i=0;i<contoursplot->nObjectsImage;i++)
	{
		fprintf(file,"%lf %lf %lf %lf\n", 
				contoursplot->objectsImage[i].x, 
				contoursplot->objectsImage[i].y,
				contoursplot->objectsImage[i].width,
				contoursplot->objectsImage[i].height
		       ); 
		fn = get_filename_png(fileName, i);
		fprintf(file,"%s\n", fn);
		cairo_surface_write_to_png (contoursplot->objectsImage[i].image,fn);
		g_free(fn);
	}
}
/****************************************************************************************/
static void gabedit_contoursplot_save_gabedit_parameters(GabeditContoursPlot *contoursplot, FILE* file)
{
	GdkColor foreColor;
	GdkColor backColor;
	GdkGCValues gc_values;
	GdkColormap *colormap;

	colormap  = gdk_window_get_colormap(GTK_WIDGET(contoursplot)->window);
	gdk_gc_get_values(contoursplot->back_gc, &gc_values);
       	gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&backColor);
	gdk_gc_get_values(contoursplot->fore_gc, &gc_values);
       	gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&foreColor);

	fprintf(file,"theme %d %d %d %d %d %d %d\n", gc_values.line_width,
			(gint)foreColor.red, (gint)foreColor.green,(gint)foreColor.blue,
			(gint)backColor.red, (gint)backColor.green,(gint)backColor.blue);
	fprintf(file,"digits %d %d\n", contoursplot->x_legends_digits,  contoursplot->y_legends_digits);
	fprintf(file,"ticks %d %d %d %d %d\n",  contoursplot->length_ticks,  contoursplot->hmajor_ticks, contoursplot->hminor_ticks,  contoursplot->vmajor_ticks, contoursplot->vminor_ticks);
	fprintf(file,"margins %d %d %d %d\n",  contoursplot->left_margins,  contoursplot->top_margins, contoursplot->right_margins,  contoursplot->bottom_margins);
	fprintf(file,"grid %d %d %d %d\n", contoursplot->hmajor_grid, contoursplot->hminor_grid, contoursplot->vmajor_grid, contoursplot->vminor_grid); 
	fprintf(file,"legends %d %d %d %d\n", contoursplot->show_left_legends, contoursplot->show_right_legends, contoursplot->show_top_legends, contoursplot->show_bottom_legends);
	fprintf(file,"axes %d %d\n",  contoursplot->reflect_x, contoursplot->reflect_y);
	fprintf(file,"fonts %d\n", contoursplot->font_size);
	fprintf(file,"minmax %lf %lf %lf %lf\n",  contoursplot->xmin, contoursplot->xmax, contoursplot->ymin, contoursplot->ymax);
	if(contoursplot->h_label_str) 
	{
		fprintf(file,"hlabel 1\n");
		fprintf(file,"%s\n",  contoursplot->h_label_str);
	}
	else fprintf(file,"hlabel 0\n");
	if(contoursplot->v_label_str) 
	{
		fprintf(file,"vlabel 1\n");
		fprintf(file,"%s\n",  contoursplot->v_label_str);
	}
	else fprintf(file,"vlabel 0\n");

}
/****************************************************************************************/
static void gabedit_contoursplot_save_gabedit_contour(ContourData* contour, FILE* file)
{
	gint i;
	fprintf(file,"contourline %lf %lf %lf %d %d\n",
		SCALE(contour->line_color.red),
		SCALE(contour->line_color.green),
		SCALE(contour->line_color.blue),
		contour->line_width,
		contour->line_style);
	fprintf(file,"contourpoint %lf %lf %lf %d %s\n",
		SCALE(contour->point_color.red),
		SCALE(contour->point_color.green),
		SCALE(contour->point_color.blue),
		contour->point_size,
		contour->point_str);
	fprintf(file,"size %d\n", contour->size);
	fprintf(file,"value %lf\n", contour->value);
	if(contour->label) fprintf(file,"label 1 %lf %lf %s\n", contour->xlabel, contour->ylabel, contour->label);
	else fprintf(file,"label 0 0 0\n");
	for(i=0;i<contour->size;i++)
		fprintf(file,"%lf %lf %d\n", contour->x[i],contour->y[i],contour->index[i]);

}
/****************************************************************************************/
static void gabedit_contoursplot_save_gabedit_data(ContoursPlotData* data, FILE* file)
{
	gint i;
	fprintf(file, "datax %d %lf %lf\n",data->xsize, data->xmin, data->xmax);
	fprintf(file, "datay %d %lf %lf\n",data->ysize, data->ymin, data->ymax);
	fprintf(file, "dataz %lf %lf\n", data->zmin, data->zmax);
	for(i=0;i<data->xsize*data->ysize; i++)
		fprintf(file, "%f\n",data->zValues[i]);
	fprintf(file, "ncontours %d\n",data->nContours);
	for(i=0;i<data->nContours; i++)
		gabedit_contoursplot_save_gabedit_contour(&data->contours[i], file);
}
/****************************************************************************************/
static void gabedit_contoursplot_save_gabedit_datas(GabeditContoursPlot *contoursplot, FILE* file)
{
	gint nDatas = 0;
	ContoursPlotData* data;
	GList *current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
	for (; current_node!=NULL; current_node=current_node->next) nDatas++;
	fprintf(file,"ndatas %d\n",nDatas);
	if(nDatas<1) return;
	current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
	for (; current_node!=NULL; current_node=current_node->next)
	{
		data=(ContoursPlotData*)current_node->data;  
		gabedit_contoursplot_save_gabedit_data(data, file);
	}
}
/****************************************************************************************/
static void gabedit_contoursplot_save_gabedit(GtkWidget* contoursplot, gchar* fileName)
{
 	FILE* file = fopen(fileName, "w");
	if(!file) return;
	fprintf(file,"[Gabedit format]\n");
	fprintf(file,"[ContoursPLOT]\n");
	gabedit_contoursplot_save_gabedit_parameters(GABEDIT_ContoursPLOT(contoursplot), file);
	gabedit_contoursplot_save_gabedit_texts(GABEDIT_ContoursPLOT(contoursplot), file);
	gabedit_contoursplot_save_gabedit_lines(GABEDIT_ContoursPLOT(contoursplot), file);
	gabedit_contoursplot_save_gabedit_images(GABEDIT_ContoursPLOT(contoursplot), file,fileName);
	gabedit_contoursplot_save_gabedit_datas(GABEDIT_ContoursPLOT(contoursplot), file);
	fclose(file);
}
/**************************************************************************/
static void saveAsGabedit(GtkFileChooser *SelecFile, gint response_id)
{       
	gchar *fileName;
	GtkWidget* contoursplot = g_object_get_data (G_OBJECT (SelecFile), "ContoursPLOT");

 	if(response_id != GTK_RESPONSE_OK) return;
	if(!GABEDIT_IS_ContoursPLOT(contoursplot)) return;
 	fileName = gtk_file_chooser_get_filename(SelecFile);
	gabedit_contoursplot_save_gabedit(contoursplot, fileName);
}
/********************************************************************************/
static void saveAsGabeditDlg(GtkWidget* contoursplot)
{
	GtkFileFilter *filter;
	GtkWidget *dialog;
	GtkWidget *parentWindow;

	dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", GTK_FILE_CHOOSER_ACTION_SAVE, "file-system-backend", "gtk+", "select-multiple", FALSE, NULL);
	gtk_window_set_title (GTK_WINDOW (dialog), "Save as Gabedit format");
	gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_SAVE, GTK_RESPONSE_OK, NULL);
	filter = gtk_file_filter_new ();
	
	gtk_file_filter_set_name (filter, "*.gab");
	gtk_file_filter_add_pattern (filter, "*.gab");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	g_signal_connect (dialog, "response",  G_CALLBACK (saveAsGabedit),GTK_OBJECT(dialog));
	g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy),GTK_OBJECT(dialog));
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog),"contoursplot.gab");

	g_object_set_data (G_OBJECT (dialog), "ContoursPLOT",contoursplot);
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
	if(parentWindow) gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parentWindow));
	gtk_widget_show(dialog);
}
/****************************************************************************************/
static gboolean gabedit_contoursplot_read_gabedit_lines(GabeditContoursPlot *contoursplot, FILE* file)
{
	gint i;
	gchar t[BSIZE];
	gchar tmp[BSIZE];
	gint style;
	if(contoursplot->objectsLine)
	{
		g_free(contoursplot->objectsLine);
		contoursplot->objectsLine = NULL;
	}
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d\n", tmp, &contoursplot->nObjectsLine);
	if(contoursplot->nObjectsLine>0)
		contoursplot->objectsLine = g_malloc(contoursplot->nObjectsLine*sizeof(ContoursPlotObjectLine));

	for(i=0;i<contoursplot->nObjectsLine;i++)
	{
		gdouble r,g,b;
    		if(!fgets(t,BSIZE,file))
		{
			contoursplot->nObjectsLine= 0;
			g_free(contoursplot->objectsLine);
			contoursplot->objectsLine = NULL;
			contoursplot->nObjectsLine = 0;
		       	return FALSE;
		}
		sscanf(t,"%lf %lf %lf %lf %d %d %lf %lf %lf %d\n", 
				&contoursplot->objectsLine[i].x1, 
				&contoursplot->objectsLine[i].y1, 
				&contoursplot->objectsLine[i].x2, 
				&contoursplot->objectsLine[i].y2, 
				&contoursplot->objectsLine[i].width, 
				&contoursplot->objectsLine[i].arrow_size, 
				&r,&g,&b,
				&style
		       ); 
		contoursplot->objectsLine[i].style = style;
		contoursplot->objectsLine[i].color.red=SCALE2(r);
		contoursplot->objectsLine[i].color.green=SCALE2(g);
		contoursplot->objectsLine[i].color.blue=SCALE2(b);
	}
	reset_object_line_pixels(contoursplot);
	return TRUE;
}
/****************************************************************************************/
static gboolean gabedit_contoursplot_read_gabedit_texts(GabeditContoursPlot *contoursplot, FILE* file)
{
	gint i;
	gchar t[BSIZE];
	gchar tmp[BSIZE];
	if(contoursplot->objectsText)
	{
		for(i=0;i<contoursplot->nObjectsText;i++)
		{
			if(contoursplot->objectsText[i].str) g_free(contoursplot->objectsText[i].str);
			if(contoursplot->objectsText[i].pango) g_object_unref(contoursplot->objectsText[i].pango);
		}
		g_free(contoursplot->objectsText);
		contoursplot->objectsText = NULL;
		contoursplot->nObjectsText = 0;
	}
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d\n", tmp, &contoursplot->nObjectsText);
	if(contoursplot->nObjectsText>0)
	{
		contoursplot->objectsText = g_malloc(contoursplot->nObjectsText*sizeof(ContoursPlotObjectText));
		for(i=0;i<contoursplot->nObjectsText;i++) contoursplot->objectsText[i].str = NULL;
	}
	for(i=0;i<contoursplot->nObjectsText;i++)
	{
    		if(!fgets(t,BSIZE,file))
		{
			for(i=0;i<contoursplot->nObjectsText;i++) 
				if(contoursplot->objectsText[i].str) g_free(contoursplot->objectsText[i].str);
			contoursplot->nObjectsText= 0;
			g_free(contoursplot->objectsText);
			contoursplot->objectsText = NULL;
		       	return FALSE;
		}
		sscanf(t,"%lf %lf %lf", &contoursplot->objectsText[i].x, &contoursplot->objectsText[i].y, &contoursplot->objectsText[i].angle); 
    		if(fgets(t,BSIZE,file))
		{
			gint j;
			for(j=0;j<strlen(t);j++) if(t[j]=='\n') t[j] = ' ';
			contoursplot->objectsText[i].str= g_strdup(t);
		}
		else contoursplot->objectsText[i].str = g_strdup(" ");
	}
	for(i=0;i<contoursplot->nObjectsText;i++)
	{
		set_object_text_pango(contoursplot, &contoursplot->objectsText[i]);
		set_object_text_pixels(contoursplot, &contoursplot->objectsText[i]);
	}
	return TRUE;
}
/****************************************************************************************/
static gboolean gabedit_contoursplot_read_gabedit_images(GabeditContoursPlot *contoursplot, FILE* file)
{
	gint i;
	gchar t[BSIZE];
	gchar tmp[BSIZE];
	if(contoursplot->objectsImage)
	{
		for(i=0;i<contoursplot->nObjectsImage;i++)
		{
			if(contoursplot->objectsImage[i].fileName) g_free(contoursplot->objectsImage[i].fileName);
			if(contoursplot->objectsImage[i].image) cairo_surface_destroy (contoursplot->objectsImage[i].image);
		}
		g_free(contoursplot->objectsImage);
		contoursplot->objectsImage = NULL;
		contoursplot->nObjectsImage = 0;
	}
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d\n", tmp, &contoursplot->nObjectsImage);
	if(contoursplot->nObjectsImage>0)
	{
		contoursplot->objectsImage = g_malloc(contoursplot->nObjectsImage*sizeof(ContoursPlotObjectImage));
		for(i=0;i<contoursplot->nObjectsImage;i++) contoursplot->objectsImage[i].fileName = NULL;
		for(i=0;i<contoursplot->nObjectsImage;i++) contoursplot->objectsImage[i].image = NULL;
	}
	for(i=0;i<contoursplot->nObjectsImage;i++)
	{
		gdouble x,y,w,h;
    		if(!fgets(t,BSIZE,file))
		{
			for(i=0;i<contoursplot->nObjectsImage;i++) 
				if(contoursplot->objectsImage[i].fileName) g_free(contoursplot->objectsImage[i].fileName);
			contoursplot->nObjectsImage= 0;
			g_free(contoursplot->objectsImage);
			contoursplot->objectsImage = NULL;
		       	return FALSE;
		}
		sscanf(t,"%lf %lf %lf %lf", &x, &y, &w, &h);
		contoursplot->objectsImage[i].x = x;
		contoursplot->objectsImage[i].y = y;
		contoursplot->objectsImage[i].width = w;
		contoursplot->objectsImage[i].height = h;
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
			contoursplot->objectsImage[i].fileName= g_strdup(t);
		}
		else contoursplot->objectsImage[i].fileName = g_strdup(" ");
	}
	for(i=0;i<contoursplot->nObjectsImage;i++)
	{
		contoursplot->objectsImage[i].image = cairo_image_surface_create_from_png (contoursplot->objectsImage[i].fileName);
		if(!contoursplot->objectsImage[i].image)
		{
			printf("I cannot read %s png file\n",contoursplot->objectsImage[i].fileName);
		}
	}
	reset_object_image_pixels(contoursplot);
	return TRUE;
}
/****************************************************************************************/
static gboolean gabedit_contoursplot_read_gabedit_parameters(GabeditContoursPlot *contoursplot, FILE* file)
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
	sscanf(t,"%s %d %d", tmp, &contoursplot->x_legends_digits,  &contoursplot->y_legends_digits);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d %d %d %d %d",  tmp, &contoursplot->length_ticks,  &contoursplot->hmajor_ticks, &contoursplot->hminor_ticks,  &contoursplot->vmajor_ticks, &contoursplot->vminor_ticks);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d %d %d %d",  tmp, &contoursplot->left_margins,  &contoursplot->top_margins, &contoursplot->right_margins,  &contoursplot->bottom_margins);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d %d %d %d", tmp, &contoursplot->hmajor_grid, &contoursplot->hminor_grid, &contoursplot->vmajor_grid, &contoursplot->vminor_grid); 
	gabedit_contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_HMAJOR_GRID, contoursplot->hmajor_grid);
	gabedit_contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_HMINOR_GRID, contoursplot->hminor_grid);
	gabedit_contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_VMAJOR_GRID, contoursplot->vmajor_grid); 
	gabedit_contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_VMINOR_GRID, contoursplot->vminor_grid);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d %d %d %d", tmp, &contoursplot->show_left_legends, &contoursplot->show_right_legends, &contoursplot->show_top_legends, &contoursplot->show_bottom_legends);
	gabedit_contoursplot_show_left_legends (GABEDIT_ContoursPLOT(contoursplot), contoursplot->show_left_legends);
	gabedit_contoursplot_show_right_legends (GABEDIT_ContoursPLOT(contoursplot), contoursplot->show_right_legends);
	gabedit_contoursplot_show_top_legends (GABEDIT_ContoursPLOT(contoursplot), contoursplot->show_top_legends);
	gabedit_contoursplot_show_bottom_legends (GABEDIT_ContoursPLOT(contoursplot), contoursplot->show_bottom_legends);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d %d",  tmp, &contoursplot->reflect_x, &contoursplot->reflect_y);
	gabedit_contoursplot_reflect_x (GABEDIT_ContoursPLOT(contoursplot), contoursplot->reflect_x);
	gabedit_contoursplot_reflect_y (GABEDIT_ContoursPLOT(contoursplot), contoursplot->reflect_y);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d", tmp, &contoursplot->font_size);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %lf %lf %lf %lf", tmp, &contoursplot->xmin, &contoursplot->xmax, &contoursplot->ymin, &contoursplot->ymax);

	if(contoursplot->h_label_str) g_free(contoursplot->h_label_str);
	if(contoursplot->v_label_str) g_free(contoursplot->v_label_str);
	contoursplot->h_label_str = NULL;
	contoursplot->v_label_str = NULL;
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d",tmp,&i); 
	if(i!=0)
	{
		gint j;
    		if(!fgets(t,BSIZE,file)) return FALSE;
		for(j=0;j<strlen(t);j++) if(t[j]=='\n') t[j] = ' ';
		contoursplot->h_label_str = g_strdup(t);
	}
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d",tmp,&i); 
	if(i!=0)
	{
		gint j;
    		if(!fgets(t,BSIZE,file)) return FALSE;
		for(j=0;j<strlen(t);j++) if(t[j]=='\n') t[j] = ' ';
		contoursplot->v_label_str = g_strdup(t);
	}
	contoursplot_build_legends(contoursplot);
	contoursplot_calculate_legends_sizes(contoursplot);
	contoursplot_calculate_sizes(contoursplot);
	reset_theme(GTK_WIDGET(contoursplot), line_width, &foreColor, &backColor);
	return TRUE;

}
/****************************************************************************************/
static gboolean gabedit_contoursplot_read_gabedit_contour(ContourData* contour, FILE* file)
{
	gint i;
	gchar tmp[BSIZE];
	gchar t[BSIZE];
	gdouble r,g,b;
	gint style;
	contour->size = 0;
	contour->x = NULL;
	contour->y = NULL;
	contour->index = NULL;
	contour->point_pango = NULL;
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %lf %lf %lf %d %d",
		tmp,
		&r,&g,&b,
		&contour->line_width,
		&style);
	contour->line_style = style;
	contour->line_color.red = SCALE2(r);
	contour->line_color.green = SCALE2(g);
	contour->line_color.blue = SCALE2(b);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %lf %lf %lf %d %s",
		tmp,
		&r,&g,&b,
		&contour->point_size,
		contour->point_str);
	contour->point_color.red = SCALE2(r);
	contour->point_color.green = SCALE2(g);
	contour->point_color.blue = SCALE2(b);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d", tmp, &contour->size);
	if(contour->size<1) return TRUE;
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %lf", tmp, &contour->value);

    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d",tmp,&i); 
	contour->label = NULL;
	contour->xlabel = 0;
	contour->ylabel = 0;
	if(i!=0)
	{
		contour->label = g_strdup(t);
		sscanf(t,"%s %d %lf %lf %s",tmp,&i,&contour->xlabel, &contour->ylabel, contour->label);
	}
	contour->x = g_malloc(contour->size*sizeof(gdouble));
	contour->y = g_malloc(contour->size*sizeof(gdouble));
	contour->index = g_malloc(contour->size*sizeof(gint));
	for(i=0;i<contour->size;i++)
	{
    		if(!fgets(t,BSIZE,file)) 
		{
			contour->size = 0;
			if(contour->x) g_free(contour->x);
			if(contour->y) g_free(contour->y);
			if(contour->index) g_free(contour->index);
			contour->x = NULL;
			contour->y = NULL;
			contour->index = NULL;
			return FALSE;
		}

		sscanf(t,"%lf %lf %d", &contour->x[i],&contour->y[i], &contour->index[i]);
	}
	return TRUE;
}
/****************************************************************************************/
static gboolean gabedit_contoursplot_read_gabedit_data(ContoursPlotData* data, FILE* file)
{
	gint i;
	gchar tmp[BSIZE];
	gchar t[BSIZE];
	gboolean Ok = TRUE;
	data->xsize = 0;
	data->ysize = 0;
	data->zValues = NULL;
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t, "%s %d %lf %lf",tmp, &data->xsize, &data->xmin, &data->xmax);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t, "%s %d %lf %lf",tmp, &data->ysize, &data->ymin, &data->ymax);
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t, "%s %lf %lf",tmp, &data->zmin, &data->zmax);
	if( data->xsize <1) return FALSE;
	if( data->ysize <1) return FALSE;
	data->zValues = g_malloc(data->xsize*data->ysize*sizeof(gdouble));
	for(i=0;i<data->xsize*data->ysize; i++)
	{
    		if(!fgets(t,BSIZE,file)) return FALSE;
		sscanf(t, "%lf",&data->zValues[i]);
	}
	data->nContours = 0;
	data->contours = NULL;
    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t, "%s %d\n",tmp, &data->nContours);
	if(data->nContours<1) return TRUE;
	data->contours = g_malloc(data->nContours*sizeof(ContourData));

	for(i=0;i<data->nContours; i++)
	{
		Ok = gabedit_contoursplot_read_gabedit_contour(&data->contours[i], file);
		if(!Ok) break;
	}
	if(!Ok)
	{
		g_free(data->contours);
		data->contours = NULL;
		data->nContours = 0;
	}
	return Ok;
}
/****************************************************************************************/
static gboolean gabedit_contoursplot_read_gabedit_datas(GabeditContoursPlot *contoursplot, FILE* file)
{
	gint nDatas = 0;
	gchar tmp[BSIZE];
	gchar t[BSIZE];
	gint i;

    	if(!fgets(t,BSIZE,file)) return FALSE;
	sscanf(t,"%s %d",tmp, &nDatas);
	if(nDatas<1) return TRUE;
	if(GABEDIT_ContoursPLOT(contoursplot)->data_list)
	{
		g_list_foreach(GABEDIT_ContoursPLOT(contoursplot)->data_list, (GFunc)g_free, NULL);
		g_list_free(GABEDIT_ContoursPLOT(contoursplot)->data_list);
		GABEDIT_ContoursPLOT(contoursplot)->data_list = NULL;
	}
	for(i=0;i<nDatas;i++)
	{
		ContoursPlotData *data = g_malloc(sizeof(ContoursPlotData));
		if(!gabedit_contoursplot_read_gabedit_data(data, file)) return FALSE;
		if(data&& data->xsize>0 && data->ysize>0 && data->zValues)
		{
			gabedit_contoursplot_add_data (GABEDIT_ContoursPLOT(contoursplot), data);
		}
	}
	return TRUE;
}
/****************************************************************************************/
static gboolean gabedit_contoursplot_read_gabedit(GtkWidget* contoursplot, gchar* fileName)
{
	gchar t[BSIZE];
 	FILE* file = fopen(fileName, "rb");
	if(!file) return FALSE;
    	if(!fgets(t,BSIZE,file)) return FALSE;
	uppercase(t);
	if(!strstr(t,"[GABEDIT FORMAT]"))
	{
		contoursplot_message(_("This is not a Gabedit file\n"));
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
		if(strstr(t,"[ContoursPLOT]"))
		{
			delete_objects_text(contoursplot);
			delete_objects_line(contoursplot);
			delete_contours(contoursplot,NULL);
			if(GABEDIT_ContoursPLOT(contoursplot)->data_list)
			{
				g_list_foreach(GABEDIT_ContoursPLOT(contoursplot)->data_list, (GFunc)g_free, NULL);
				g_list_free(GABEDIT_ContoursPLOT(contoursplot)->data_list);
				GABEDIT_ContoursPLOT(contoursplot)->data_list = NULL;
			}
			gabedit_contoursplot_read_gabedit_parameters(GABEDIT_ContoursPLOT(contoursplot), file);
			gabedit_contoursplot_read_gabedit_texts(GABEDIT_ContoursPLOT(contoursplot), file);
			gabedit_contoursplot_read_gabedit_lines(GABEDIT_ContoursPLOT(contoursplot), file);
			gabedit_contoursplot_read_gabedit_images(GABEDIT_ContoursPLOT(contoursplot), file);
			gabedit_contoursplot_read_gabedit_datas(GABEDIT_ContoursPLOT(contoursplot), file);
			reset_color_map(contoursplot, NULL);
			fclose(file);
			return TRUE;
		}
	}
	contoursplot_message(_("I can not read the ContoursPlot data\n"));
	fclose(file);
	return FALSE;
}
/**************************************************************************/
static void readAGabedit(GtkFileChooser *SelecFile, gint response_id)
{       
	gchar *fileName;
	GtkWidget* contoursplot = g_object_get_data (G_OBJECT (SelecFile), "ContoursPLOT");

 	if(response_id != GTK_RESPONSE_OK) return;
	if(!GABEDIT_IS_ContoursPLOT(contoursplot)) return;
 	fileName = gtk_file_chooser_get_filename(SelecFile);
	gabedit_contoursplot_read_gabedit(contoursplot, fileName);
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/********************************************************************************/
static void readAGabeditDlg(GtkWidget* contoursplot)
{
	GtkFileFilter *filter;
	GtkWidget *dialog;
	GtkWidget *parentWindow;

	dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", GTK_FILE_CHOOSER_ACTION_OPEN, "file-system-backend", "gtk+", "select-multiple", FALSE, NULL);
	gtk_window_set_title (GTK_WINDOW (dialog), "Read a Gabedit file");
	gtk_dialog_add_buttons (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_OK, NULL);

	filter = gtk_file_filter_new ();
	
	gtk_file_filter_set_name (filter, "*.gab");
	gtk_file_filter_add_pattern (filter, "*.gab");
	gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog), filter);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
	g_signal_connect (dialog, "response",  G_CALLBACK (readAGabedit),GTK_OBJECT(dialog));
	g_signal_connect (dialog, "response", G_CALLBACK (gtk_widget_destroy),GTK_OBJECT(dialog));

	g_object_set_data (G_OBJECT (dialog), "ContoursPLOT",contoursplot);
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
	if(parentWindow) gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parentWindow));
	gtk_widget_show(dialog);
}
/*************************************************************************************/
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
static void copyImageToClipBoard(GtkWidget* contoursplot)
{       
	if(!GABEDIT_IS_ContoursPLOT(contoursplot)) return;
	gtk_widget_hide(contoursplot);
	gtk_widget_show(contoursplot);
	while( gtk_events_pending() ) gtk_main_iteration();

	gabedit_contoursplot_save_image(GABEDIT_ContoursPLOT(contoursplot), NULL, NULL);
}
/**************************************************************************/
static void saveImage(GtkFileChooser *SelecFile, gint response_id)
{       
	gchar *fileName;
	GtkWidget* contoursplot = g_object_get_data (G_OBJECT (SelecFile), "ContoursPLOT");
	gchar* type = g_object_get_data (G_OBJECT (SelecFile), "ImageType");

 	if(response_id != GTK_RESPONSE_OK) return;

 	fileName = gtk_file_chooser_get_filename(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
	Waiting(1.0);

	if(!GABEDIT_IS_ContoursPLOT(contoursplot)) return;
	gtk_widget_hide(contoursplot);
	gtk_widget_show(contoursplot);
	while( gtk_events_pending() ) gtk_main_iteration();

	gabedit_contoursplot_save_image(GABEDIT_ContoursPLOT(contoursplot), fileName, type);
}
/********************************************************************************/
static void saveImageDlg(GtkWidget* contoursplot, gchar* type)
{
	GtkFileFilter *filter;
	GtkWidget *dialog;
	gchar* tmp = NULL;
	GtkWidget *parentWindow;

	dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", GTK_FILE_CHOOSER_ACTION_SAVE, "file-system-backend", "gtk+", "select-multiple", FALSE, NULL);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Save Contours plot"));
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
		tmp = g_strdup_printf("contoursplot.png");
	else
		tmp = g_strdup_printf("contoursplot.%s",type);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER (dialog),tmp);
	g_free(tmp);

	g_object_set_data (G_OBJECT (dialog), "ContoursPLOT",contoursplot);
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
	if(parentWindow) gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parentWindow));
	gtk_widget_show(dialog);
}
/*****************************************************************************/
static void gabedit_contoursplot_export(GabeditContoursPlot *contoursplot, gchar* fileName, gchar* fileType)
{
	GtkWidget* widget = NULL;
  	g_return_if_fail (contoursplot != NULL);
  	g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  	g_return_if_fail (fileName != NULL);
  	g_return_if_fail (fileType != NULL);
	widget = GTK_WIDGET(contoursplot);
	if(!strcmp(fileType,"pdf"))
	{
		
		cairo_surface_t *surface;
		surface = cairo_pdf_surface_create(fileName, widget->allocation.width, widget->allocation.height);
		contoursplot->cairo_export = cairo_create(surface);
		gabedit_contoursplot_draw (widget);
		cairo_show_page(contoursplot->cairo_export);
		cairo_surface_destroy(surface);
		cairo_destroy(contoursplot->cairo_export);
		contoursplot->cairo_export = NULL;
		return;
	}
	else
	if(!strcmp(fileType,"ps"))
	{
		
		cairo_surface_t *surface;
		surface = cairo_ps_surface_create(fileName, widget->allocation.width, widget->allocation.height);
		contoursplot->cairo_export = cairo_create(surface);
		gabedit_contoursplot_draw (widget);
		cairo_show_page(contoursplot->cairo_export);
		cairo_surface_destroy(surface);
		cairo_destroy(contoursplot->cairo_export);
		contoursplot->cairo_export = NULL;
		return;
	}
	else
	if(!strcmp(fileType,"eps"))
	{
		
		cairo_surface_t *surface;
		surface = cairo_ps_surface_create(fileName, widget->allocation.width, widget->allocation.height);
		cairo_ps_surface_set_eps(surface, TRUE);
		contoursplot->cairo_export = cairo_create(surface);
		gabedit_contoursplot_draw (widget);
		cairo_show_page(contoursplot->cairo_export);
		cairo_surface_destroy(surface);
		cairo_destroy(contoursplot->cairo_export);
		contoursplot->cairo_export = NULL;
		return;
	}
	else
	if(!strcmp(fileType,"svg"))
	{
		
		cairo_surface_t *surface;
		surface = cairo_svg_surface_create(fileName, widget->allocation.width, widget->allocation.height);
		contoursplot->cairo_export = cairo_create(surface);
		gabedit_contoursplot_draw (widget);
		cairo_show_page(contoursplot->cairo_export);
		cairo_surface_destroy(surface);
		cairo_destroy(contoursplot->cairo_export);
		contoursplot->cairo_export = NULL;
		return;
	}
}
/**************************************************************************/
static void exportImage(GtkFileChooser *SelecFile, gint response_id)
{       
	gchar *fileName;
	GtkWidget* contoursplot = g_object_get_data (G_OBJECT (SelecFile), "ContoursPLOT");
	gchar* type = g_object_get_data (G_OBJECT (SelecFile), "ImageType");

 	if(response_id != GTK_RESPONSE_OK) return;

 	fileName = gtk_file_chooser_get_filename(SelecFile);
	gtk_widget_hide(GTK_WIDGET(SelecFile));
	while( gtk_events_pending() ) gtk_main_iteration();
	Waiting(1.0);

	if(!GABEDIT_IS_ContoursPLOT(contoursplot)) return;
	gtk_widget_hide(contoursplot);
	gtk_widget_show(contoursplot);
	while( gtk_events_pending() ) gtk_main_iteration();

	gabedit_contoursplot_export(GABEDIT_ContoursPLOT(contoursplot), fileName, type);
}
/********************************************************************************/
static void exportImageDlg(GtkWidget* contoursplot, gchar* type)
{
	GtkFileFilter *filter;
	GtkWidget *dialog;
	gchar* tmp = NULL;
	GtkWidget *parentWindow;

	dialog = g_object_new (GTK_TYPE_FILE_CHOOSER_DIALOG, "action", GTK_FILE_CHOOSER_ACTION_SAVE, "file-system-backend", "gtk+", "select-multiple", FALSE, NULL);
	gtk_window_set_title (GTK_WINDOW (dialog), _("Export Contours plot"));
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

	g_object_set_data (G_OBJECT (dialog), "ContoursPLOT",contoursplot);
	gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
	if(parentWindow) gtk_window_set_transient_for(GTK_WINDOW(dialog),GTK_WINDOW(parentWindow));
	gtk_widget_show(dialog);
}
/********************************************************************************/
static void activate_entry_label_contour_str(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	GtkWidget* contoursplot = NULL;
	ContourData* contour = NULL;


	if(!user_data || !G_IS_OBJECT(user_data)) return;
	contoursplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	contour = g_object_get_data(G_OBJECT (entry), "CurentContour");
	if(contour->label) g_free(contour->label);
	contour->label = g_strdup(t);
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/********************************************************************************/
static void activate_entry_label_contour_x(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	GtkWidget* contoursplot = NULL;
	ContourData* contour = NULL;


	if(!user_data || !G_IS_OBJECT(user_data)) return;
	contoursplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	contour = g_object_get_data(G_OBJECT (entry), "CurentContour");
	contour->xlabel = atof(t);
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/********************************************************************************/
static void activate_entry_label_contour_y(GtkWidget *entry, gpointer user_data)
{
	G_CONST_RETURN gchar* t;
	GtkWidget* contoursplot = NULL;
	ContourData* contour = NULL;


	if(!user_data || !G_IS_OBJECT(user_data)) return;
	contoursplot = GTK_WIDGET(user_data);
	t= gtk_entry_get_text(GTK_ENTRY(entry));
	contour = g_object_get_data(G_OBJECT (entry), "CurentContour");
	contour->ylabel = atof(t);
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
static void spin_line_width_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContourData* contour = g_object_get_data(G_OBJECT (spinbutton), "CurentContour");
  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
		if(contour) contour->line_width = gtk_spin_button_get_value(spinbutton);
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/****************************************************************************************/
static void spin_line_color_changed_value(GtkColorButton  *colorbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContourData* contour = g_object_get_data(G_OBJECT (colorbutton), "CurentContour");
		GdkColor c;
		gtk_color_button_get_color (colorbutton, &c);

  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
		
		if(contour) contour->line_color = c; 
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
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
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContourData* contour = g_object_get_data(G_OBJECT (combobox), "CurentContour");

  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

		gtk_tree_model_get (model, &iter, 0, &d, -1);
		if(!d) return;
		if(contour)
		{
		if (!strcmp(d,"Solid") ) {  contour->line_style = GDK_LINE_SOLID; }
		else if (!strcmp(d,"On-Off dashed") ) {contour->line_style = GDK_LINE_ON_OFF_DASH; }
		else if (!strcmp(d,"Double dashed") ) { contour->line_style = GDK_LINE_DOUBLE_DASH;}
		}
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/****************************************************************************************/
static void spin_point_size_changed_value(GtkSpinButton *spinbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContourData* contour = g_object_get_data(G_OBJECT (spinbutton), "CurentContour");

  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

		if(contour) 
		{
			contour->point_size = gtk_spin_button_get_value(spinbutton);
			contoursplot_build_points_contour(GABEDIT_ContoursPLOT(contoursplot), contour);
		}
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
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
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContourData* contour = g_object_get_data(G_OBJECT (combobox), "CurentContour");

  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

		gtk_tree_model_get (model, &iter, 0, &d, -1);
		if(!d) return;
		if(contour)
		{
			sprintf(contour->point_str,"%s",d);
			contoursplot_build_points_contour(GABEDIT_ContoursPLOT(contoursplot), contour);
		}
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/****************************************************************************************/
static void spin_point_color_changed_value(GtkColorButton  *colorbutton, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContourData* contour = g_object_get_data(G_OBJECT (colorbutton), "CurentContour");
		GdkColor c;

  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

		gtk_color_button_get_color (colorbutton, &c);
		
		if(contour)
		{
			contour->point_color = c; 
			contoursplot_build_points_contour(GABEDIT_ContoursPLOT(contoursplot), contour);
		}
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/********************************************************************************************************/
static GtkWidget *add_point_types_combo(GtkWidget *hbox, ContourData* contour)
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
		"<span>&#9651;</span>", "<span>&#9650;</span>"};
	gint n = G_N_ELEMENTS (list);

	store = gtk_tree_store_new (1,G_TYPE_STRING);

	for(i=0;i<n;i++)
	{
        	gtk_tree_store_append (store, &iter, NULL);
        	gtk_tree_store_set (store, &iter, 0, list[i], -1);
		if(!strcmp(list[i],contour->point_str)) k = i;
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
static void activate_entry_scale_z(GtkEntry *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContoursPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gint loop;
		gdouble a;
		G_CONST_RETURN gchar* t;

  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		a = evaluate_formula(t);
		if(data)
		{
			for(loop=0;loop<data->xsize*data->ysize; loop++) data->zValues[loop] *= a;
			for(loop=0;loop<data->nContours; loop++) 
			{
				double b = data->contours[loop].value *a;
				free_one_contour(&data->contours[loop]);
				data->contours[loop] = get_contour_point(contoursplot, data, b);
			}
		}
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(ContoursPlotData*)current_node->data;  
				for(loop=0;loop<data->xsize*data->ysize; loop++) data->zValues[loop] *= a;
				for(loop=0;loop<data->nContours; loop++) 
				{
					double b = data->contours[loop].value*a;
					free_one_contour(&data->contours[loop]);
					data->contours[loop] = get_contour_point(contoursplot, data, b);
				}
			}
		}
		gtk_entry_set_text(GTK_ENTRY(entry),"1.0");
		gtk_editable_set_position(GTK_EDITABLE(entry),3);
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/****************************************************************************************/
static void activate_entry_scale_x(GtkEntry *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContoursPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gint loop,i;
		gdouble a;
		G_CONST_RETURN gchar* t;

  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		a = evaluate_formula(t);
		if(data)
		{
			data->xmin *= a;
			data->xmax *= a;
			for(loop=0;loop<data->nContours; loop++) 
				for(i=0;i<data->contours[loop].size; i++) data->contours[loop].x[i] *= a;
		}
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(ContoursPlotData*)current_node->data;  
				data->xmin *= a;
				data->xmax *= a;
				for(loop=0;loop<data->nContours; loop++) 
					for(i=0;i<data->contours[loop].size; i++) data->contours[loop].x[i] *= a;
			}
		}
		gtk_entry_set_text(GTK_ENTRY(entry),"1.0");
		gtk_editable_set_position(GTK_EDITABLE(entry),3);
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/****************************************************************************************/
static void activate_entry_scale_y(GtkEntry *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContoursPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gdouble a;
		gint loop,i;
		G_CONST_RETURN gchar* t;
  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		a = evaluate_formula(t);
		if(data)
		{
			data->ymin *= a;
			data->ymax *= a;
			for(loop=0;loop<data->nContours; loop++) 
				for(i=0;i<data->contours[loop].size; i++) data->contours[loop].y[i] *= a;
		}
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(ContoursPlotData*)current_node->data;  
				data->ymin *= a;
				data->ymax *= a;
				for(loop=0;loop<data->nContours; loop++) 
					for(i=0;i<data->contours[loop].size; i++) data->contours[loop].y[i] *= a;
			}
		}
		gtk_entry_set_text(GTK_ENTRY(entry),"1.0");
		gtk_editable_set_position(GTK_EDITABLE(entry),3);
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/****************************************************************************************/
static void activate_entry_shift_z(GtkEntry *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContoursPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gint loop;
		gdouble a;
		G_CONST_RETURN gchar* t;
  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		a = evaluate_formula(t);
		if(data)
		{
			for(loop=0;loop<data->xsize*data->ysize; loop++) data->zValues[loop] += a;
			for(loop=0;loop<data->nContours; loop++) 
			{
				double b = data->contours[loop].value+a;
				free_one_contour(&data->contours[loop]);
				data->contours[loop] = get_contour_point(contoursplot, data, b);
			}
		}
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(ContoursPlotData*)current_node->data;  
				for(loop=0;loop<data->xsize*data->ysize; loop++) data->zValues[loop] += a;
				for(loop=0;loop<data->nContours; loop++) 
				{
					double b = data->contours[loop].value+a;
					free_one_contour(&data->contours[loop]);
					data->contours[loop] = get_contour_point(contoursplot, data, b);
				}
			}
		}
		gtk_entry_set_text(GTK_ENTRY(entry),"0.0");
		gtk_editable_set_position(GTK_EDITABLE(entry),3);
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/****************************************************************************************/
static void activate_entry_shift_x(GtkEntry *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContoursPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gdouble a;
		gint i,loop;
		G_CONST_RETURN gchar* t;
  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		a = evaluate_formula(t);
		if(data)
		{
			data->xmin += a;
			data->xmax += a;
			for(loop=0;loop<data->nContours; loop++) 
				for(i=0;i<data->contours[loop].size; i++) data->contours[loop].x[i] += a;
		}
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(ContoursPlotData*)current_node->data;  
				data->xmin += a;
				data->xmax += a;
				for(loop=0;loop<data->nContours; loop++) 
					for(i=0;i<data->contours[loop].size; i++) data->contours[loop].x[i] += a;
			}
		}
		gtk_entry_set_text(GTK_ENTRY(entry),"0.0");
		gtk_editable_set_position(GTK_EDITABLE(entry),3);
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/****************************************************************************************/
static void activate_entry_shift_y(GtkEntry *entry, gpointer user_data)
{
	if(user_data && G_IS_OBJECT(user_data))
	{
		GtkWidget* contoursplot = GTK_WIDGET(user_data);
		ContoursPlotData* data = g_object_get_data(G_OBJECT (entry), "CurentData");
		gdouble a;
		gint i,loop;
		G_CONST_RETURN gchar* t;
  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
		t= gtk_entry_get_text(GTK_ENTRY(entry));
		if(!t) return;
		a = evaluate_formula(t);
		if(data)
		{
			data->ymin += a;
			data->ymax += a;
			for(loop=0;loop<data->nContours; loop++) 
				for(i=0;i<data->contours[loop].size; i++) data->contours[loop].y[i] += a;
		}
		else
		{
			GList *current_node;
			current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
			for (; current_node!=NULL; current_node=current_node->next)
			{
				data=(ContoursPlotData*)current_node->data;  
				data->ymin += a;
				data->ymax += a;
				for(loop=0;loop<data->nContours; loop++) 
					for(i=0;i<data->contours[loop].size; i++) data->contours[loop].y[i] += a;
			}
		}
		gtk_entry_set_text(GTK_ENTRY(entry),"0.0");
		gtk_editable_set_position(GTK_EDITABLE(entry),3);
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/****************************************************************************************/
static void add_contour_data_dialog(GtkWidget* window, GtkWidget* vbox_window, 
		GabeditContoursPlot* contoursplot, ContoursPlotData* data, ContourData* contour)
{
	gint i,j;
	GtkWidget* frame = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* vbox_frame = NULL;
	GtkWidget* table = NULL;
	GtkWidget* hbox1 = NULL;
	GtkWidget* label = NULL;
	GtkWidget* combo = NULL;
	GtkWidget* combo_point = NULL;
	GtkWidget* spin = NULL;
	GtkWidget* button = NULL;
	GtkWidget* spin_point = NULL;
	GtkWidget* button_point = NULL;
	GtkWidget* entry_label_contour_str = NULL;
	GtkWidget* entry_label_contour_x = NULL;
	GtkWidget* entry_label_contour_y = NULL;
	gchar t[100];

	if(!contour) return;
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
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), contour->line_width);
	gtk_box_pack_start(GTK_BOX(hbox), spin, TRUE, FALSE, 2);
	gtk_widget_show(spin);
	g_object_set_data(G_OBJECT (window), "SpinLineWidth", spin);

	label=gtk_label_new(_("Line type :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	combo = add_line_types_combo(hbox);
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
	if(contour->line_style == GDK_LINE_SOLID) gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 0);
	else if(contour->line_style == GDK_LINE_ON_OFF_DASH) gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 1);
	else if(contour->line_style == GDK_LINE_DOUBLE_DASH) gtk_combo_box_set_active (GTK_COMBO_BOX (combo), 2);
	gtk_widget_show(combo); 
	g_object_set_data(G_OBJECT (window), "ComboLineType", combo);

	label=gtk_label_new(_("Line color :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	button = gtk_color_button_new_with_color (&contour->line_color);
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
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_point), contour->point_size);
	gtk_box_pack_start(GTK_BOX(hbox), spin_point, TRUE, FALSE, 2);
	gtk_widget_show(spin_point);
	g_object_set_data(G_OBJECT (window), "SpinPointWidth", spin_point);

	label=gtk_label_new(_("Point type :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	combo_point = add_point_types_combo(hbox, contour);
	gtk_widget_show(combo_point); 
	g_object_set_data(G_OBJECT (window), "ComboPointType", combo_point);

	label=gtk_label_new(_("Point color :"));
	gtk_box_pack_start(GTK_BOX(hbox), label, TRUE, FALSE, 2);
	gtk_widget_show(label); 

	button_point = gtk_color_button_new_with_color (&contour->point_color);
	gtk_box_pack_start(GTK_BOX(hbox), button_point, TRUE, FALSE, 2);
	gtk_widget_show(button_point);
	g_object_set_data(G_OBJECT (window), "ColorButton", button_point);

	frame = gtk_frame_new(_("Set label"));
	gtk_box_pack_start(GTK_BOX(vbox_window), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	vbox_frame=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox_frame);
	gtk_widget_show(vbox_frame);

	table = gtk_table_new(3,3,FALSE);
	gtk_container_add(GTK_CONTAINER(vbox_frame),table);

	i = 0;
	j = 0;
	add_label_at_table(table,_(" Label "),i,j,GTK_JUSTIFY_LEFT);
	j++;
	add_label_at_table(table," : ",i,j,GTK_JUSTIFY_LEFT);
	j++;
	entry_label_contour_str = gtk_entry_new ();
	add_widget_table(table,entry_label_contour_str,i,j);
	if(contour->label) sprintf(t,"%s",contour->label);
	else t[0] = '\0';
	gtk_entry_set_text(GTK_ENTRY(entry_label_contour_str),t);

	i = 1;
	j = 0;
	add_label_at_table(table,_(" X label "),i,j,GTK_JUSTIFY_LEFT);
	j++;
	add_label_at_table(table," : ",i,j,GTK_JUSTIFY_LEFT);
	j++;
	entry_label_contour_x = gtk_entry_new ();
	add_widget_table(table,entry_label_contour_x,i,j);
	sprintf(t,"%0.10lf",contour->xlabel);
	gtk_entry_set_text(GTK_ENTRY(entry_label_contour_x),t);

	i = 2;
	j = 0;
	add_label_at_table(table,_(" X label "),i,j,GTK_JUSTIFY_LEFT);
	j++;
	add_label_at_table(table," : ",i,j,GTK_JUSTIFY_LEFT);
	j++;
	entry_label_contour_y = gtk_entry_new ();
	add_widget_table(table,entry_label_contour_y,i,j);
	sprintf(t,"%0.10lf",contour->ylabel);
	gtk_entry_set_text(GTK_ENTRY(entry_label_contour_y),t);
	gtk_widget_show_all(table); 

	gtk_widget_show(window); 

	g_object_set_data(G_OBJECT (spin), "CurentData", data);
	g_object_set_data(G_OBJECT (spin), "CurentContour", contour);
	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(spin_line_width_changed_value), contoursplot);
	g_object_set_data(G_OBJECT (button), "CurentData", data);
	g_object_set_data(G_OBJECT (button), "CurentContour", contour);
	g_signal_connect(G_OBJECT(button), "color-set", G_CALLBACK(spin_line_color_changed_value), contoursplot);
	g_signal_connect(G_OBJECT(combo), "changed", G_CALLBACK(combo_line_style_changed_value), contoursplot);
	g_object_set_data(G_OBJECT (combo), "CurentData", data);
	g_object_set_data(G_OBJECT (combo), "CurentContour", contour);

	g_signal_connect(G_OBJECT(combo_point), "changed", G_CALLBACK(combo_point_style_changed_value), contoursplot);
	g_object_set_data(G_OBJECT (combo_point), "CurentData", data);
	g_object_set_data(G_OBJECT (combo_point), "CurentContour", contour);

	g_object_set_data(G_OBJECT (spin_point), "CurentData", data);
	g_object_set_data(G_OBJECT (spin_point), "CurentContour", contour);
	g_signal_connect(G_OBJECT(spin_point), "value-changed", G_CALLBACK(spin_point_size_changed_value), contoursplot);
	g_object_set_data(G_OBJECT (button_point), "CurentData", data);
	g_object_set_data(G_OBJECT (button_point), "CurentContour", contour);
	g_signal_connect(G_OBJECT(button_point), "color-set", G_CALLBACK(spin_point_color_changed_value), contoursplot);

	g_object_set_data(G_OBJECT (entry_label_contour_str), "CurentData", data);
	g_object_set_data(G_OBJECT (entry_label_contour_str), "CurentContour", contour);
	g_object_set_data(G_OBJECT (entry_label_contour_x), "CurentData", data);
	g_object_set_data(G_OBJECT (entry_label_contour_x), "CurentContour", contour);
	g_object_set_data(G_OBJECT (entry_label_contour_y), "CurentData", data);
	g_object_set_data(G_OBJECT (entry_label_contour_y), "CurentContour", contour);
	g_signal_connect (G_OBJECT (entry_label_contour_str), "activate", (GCallback)activate_entry_label_contour_str, contoursplot);
	g_signal_connect (G_OBJECT (entry_label_contour_x), "activate", (GCallback)activate_entry_label_contour_x, contoursplot);
	g_signal_connect (G_OBJECT (entry_label_contour_y), "activate", (GCallback)activate_entry_label_contour_y, contoursplot);
}
/****************************************************************************************/
static void set_data_dialog(GabeditContoursPlot* contoursplot, ContoursPlotData* data,ContourData* contour)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* frame = NULL;
	GtkWidget* hbox = NULL;
	GtkWidget* vbox = NULL;
	GtkWidget* label = NULL;
	GtkWidget* buttonSave = NULL;
	GtkWidget* buttonRemove = NULL;
	GtkWidget* buttonAutoRanges = NULL;
	GtkWidget* buttonAutoRangesAll = NULL;
	GtkWidget* parentWindow = NULL;
	GtkWidget* vbox_window = NULL;
	GtkWidget* entry_scale_x = NULL;
	GtkWidget* entry_scale_y = NULL;
	GtkWidget* entry_scale_z = NULL;
	GtkWidget* entry_shift_x = NULL;
	GtkWidget* entry_shift_y = NULL;
	GtkWidget* entry_shift_z = NULL;

	gtk_window_set_title (GTK_WINDOW (window), _("Set data options"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(destroy_data_dialog), (gpointer)contoursplot);

	vbox_window=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(window), vbox_window);
	gtk_widget_show(vbox_window);

	add_contour_data_dialog(window, vbox_window, contoursplot, data, contour);

	frame = gtk_frame_new(NULL);
	gtk_box_pack_start(GTK_BOX(vbox_window), frame, TRUE, FALSE, 2);
	gtk_widget_show(frame);

	vbox=gtk_vbox_new(FALSE, 0);
	gtk_container_add(GTK_CONTAINER(frame), vbox);
	gtk_widget_show(vbox);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 2);
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

	label=gtk_label_new(_("Scale Z : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_scale_z = gtk_entry_new();
	gtk_widget_set_size_request(entry_scale_z,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_scale_z),"1.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_scale_z, FALSE, FALSE, 2);
	gtk_widget_show(entry_scale_z);

	hbox=gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, FALSE, 2);
	gtk_widget_show(hbox);

	label=gtk_label_new(_("Shift  X : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_shift_x = gtk_entry_new();
	gtk_widget_set_size_request(entry_shift_x,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_shift_x),"0.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_shift_x, FALSE, FALSE, 2);
	gtk_widget_show(entry_shift_x);

	label=gtk_label_new(_("Shift  Y : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 


	entry_shift_y = gtk_entry_new();
	gtk_widget_set_size_request(entry_shift_y,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_shift_y),"0.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_shift_y, FALSE, FALSE, 2);
	gtk_widget_show(entry_shift_y);

	label=gtk_label_new(_("Shift  Z : "));
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 2);
	gtk_widget_show(label); 

	entry_shift_z = gtk_entry_new();
	gtk_widget_set_size_request(entry_shift_z,50,-1);
	gtk_entry_set_text(GTK_ENTRY(entry_shift_z),"0.0");
	gtk_box_pack_start(GTK_BOX(hbox), entry_shift_z, FALSE, FALSE, 2);
	gtk_widget_show(entry_shift_z);

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

	buttonRemove = gtk_button_new_with_label (_("Remove"));
	gtk_box_pack_start(GTK_BOX(hbox), buttonRemove, TRUE, TRUE, 4);
	gtk_widget_show (buttonRemove);



	g_object_set_data(G_OBJECT (buttonSave), "CurentData", data);
	g_signal_connect(G_OBJECT(buttonSave), "clicked", G_CALLBACK(save_data_dlg), contoursplot);

	g_object_set_data(G_OBJECT (buttonRemove), "CurentData", data);
	g_object_set_data(G_OBJECT (buttonRemove), "Window", window);
	g_object_set_data(G_OBJECT (buttonRemove), "ContoursPLOT", contoursplot);
	g_signal_connect(G_OBJECT(buttonRemove), "clicked", G_CALLBACK(remove_data_dlg), contoursplot);

	g_object_set_data(G_OBJECT (buttonAutoRanges), "CurentData", data);
	g_signal_connect(G_OBJECT(buttonAutoRanges), "clicked", G_CALLBACK(auto_range_activate), contoursplot);

	g_object_set_data(G_OBJECT (buttonAutoRangesAll), "CurentData", NULL);
	g_signal_connect(G_OBJECT(buttonAutoRangesAll), "clicked", G_CALLBACK(auto_range_activate), contoursplot);

	g_object_set_data(G_OBJECT (entry_scale_x), "CurentData", data);
	g_object_set_data(G_OBJECT (entry_scale_y), "CurentData", data);
	g_object_set_data(G_OBJECT (entry_shift_x), "CurentData", data);
	g_object_set_data(G_OBJECT (entry_shift_y), "CurentData", data);
	g_signal_connect (G_OBJECT (entry_scale_x), "activate", (GCallback)activate_entry_scale_x, contoursplot);
	g_signal_connect (G_OBJECT (entry_scale_y), "activate", (GCallback)activate_entry_scale_y, contoursplot);
	g_signal_connect (G_OBJECT (entry_scale_z), "activate", (GCallback)activate_entry_scale_z, contoursplot);
	g_signal_connect (G_OBJECT (entry_shift_x), "activate", (GCallback)activate_entry_shift_x, contoursplot);
	g_signal_connect (G_OBJECT (entry_shift_y), "activate", (GCallback)activate_entry_shift_y, contoursplot);
	g_signal_connect (G_OBJECT (entry_shift_z), "activate", (GCallback)activate_entry_shift_z, contoursplot);

	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
	/* list_utf8();*/
	

}
/*********************************************************************************************************************/
static void applyColorMapOptions(GtkWidget *dialogWindow, gpointer data)
{
	GtkWidget* buttonMultiColor;
	GtkWidget* button2Colors;
	GtkWidget* buttonUniColor;
	GtkWidget* selectorUniColor;
	GtkWidget* selector2Colors1;
	GtkWidget* selector2Colors2;
	GabeditContoursPlot* contoursplot;
	GtkWidget* entryMin;
	GtkWidget* entryMax;
	GtkWidget* entryNLabels;
	gdouble min = -1;
	gdouble max = 1;
	gint N = 0;
	G_CONST_RETURN gchar *temp = NULL;
	 
	if(!GTK_IS_WIDGET(dialogWindow)) return;
	buttonMultiColor = g_object_get_data(G_OBJECT (dialogWindow), "ButtonMultiColor");
	button2Colors = g_object_get_data(G_OBJECT (dialogWindow), "Button2Colors");
	buttonUniColor = g_object_get_data(G_OBJECT (dialogWindow), "ButtonUniColor");
	selectorUniColor = g_object_get_data(G_OBJECT (dialogWindow), "SelectorUniColor");
	selector2Colors1 = g_object_get_data(G_OBJECT (dialogWindow), "Selector2Colors1");
	selector2Colors2 = g_object_get_data(G_OBJECT (dialogWindow), "Selector2Colors2");
	entryMin =(GtkWidget*)g_object_get_data(G_OBJECT (dialogWindow), "EntryMin");
	entryMax =(GtkWidget*)g_object_get_data(G_OBJECT (dialogWindow), "EntryMax");
	entryNLabels =(GtkWidget*)g_object_get_data(G_OBJECT (dialogWindow), "EntryNLabels");
	contoursplot = (GabeditContoursPlot* )g_object_get_data(G_OBJECT (dialogWindow), "ContoursPlot");
	if(!entryNLabels) return;
        temp	= gtk_entry_get_text(GTK_ENTRY(entryNLabels)); 
	N = atoi(temp);
	if(N<=0)
	{
		Message(_("Error : The number of labels should be a positive integer. "),_("Error"));
		return;
	}
        if(!get_a_float(entryMin,&min,_("Error : The minimal value should be float."))) return;
        if(!get_a_float(entryMax,&max,_("Error : The maximal value should be float."))) return;
	if( max<=min)
	{
		Message(_("Error :  The minimal value should be smaller than the maximal value "),_("Error"));
		return;
	}

	if(!buttonMultiColor) return;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(buttonMultiColor)))
	{

  		contoursplot->colorsMap.type=1;
	}
	else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button2Colors)) && selector2Colors1 && selector2Colors2)
	{
		GdkColor color;
  		contoursplot->colorsMap.type=2;
		gtk_color_button_get_color (GTK_COLOR_BUTTON(selector2Colors1), &color);
		contoursplot->colorsMap.colors[0][0] = color.red/65535.0;
		contoursplot->colorsMap.colors[0][1] = color.green/65535.0;
		contoursplot->colorsMap.colors[0][2] = color.blue/65535.0;
		gtk_color_button_get_color (GTK_COLOR_BUTTON(selector2Colors2), &color);
		contoursplot->colorsMap.colors[1][0] = color.red/65535.0;
		contoursplot->colorsMap.colors[1][1] = color.green/65535.0;
		contoursplot->colorsMap.colors[1][2] = color.blue/65535.0;

	}
	else if(selectorUniColor)
	{
		GdkColor color;
  		contoursplot->colorsMap.type=3;
		gtk_color_button_get_color (GTK_COLOR_BUTTON(selectorUniColor), &color);
		contoursplot->colorsMap.colors[2][0] = color.red/65535.0;
		contoursplot->colorsMap.colors[2][1] = color.green/65535.0;
		contoursplot->colorsMap.colors[2][2] = color.blue/65535.0;
	}
  	contoursplot->colormap_nlegends = N;
	reset_color_map_min_max(GTK_WIDGET(contoursplot), min, max);
	reset_contour_colors(GTK_WIDGET(contoursplot), NULL);
}
/*********************************************************************************************************************/
static GtkWidget* addRadioButtonColorMapToATable(GtkWidget* table, GtkWidget* friendButton, gchar* label, gint i, gint j, gint k)
{
	GtkWidget *newButton;

	if(friendButton)
		newButton = gtk_radio_button_new_with_label( gtk_radio_button_get_group (GTK_RADIO_BUTTON (friendButton)), label);
	else
		newButton = gtk_radio_button_new_with_label( NULL, label);

	gtk_table_attach(GTK_TABLE(table),newButton,j,j+k,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);

	g_object_set_data(G_OBJECT (newButton), "Type",NULL);
	return newButton;
}
/*********************************************************************************************************************/
static void createColorMapOptionsFrame(GtkWidget* dialogWindow, GtkWidget *box)
{
	GtkWidget* button;
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget *table = gtk_table_new(3,3,TRUE);
	gint i;
	GtkWidget *selector;
	GdkColor color;
	GabeditContoursPlot* contoursplot = NULL;

	if(!GTK_IS_WIDGET(dialogWindow)) return;
	contoursplot = (GabeditContoursPlot* )g_object_get_data(G_OBJECT (dialogWindow), "ContoursPlot");

	color.red = 65535;
	color.green = 65535;
	color.blue = 65535;
	frame = gtk_frame_new (_("Color mapping type"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	i = 0;
	button =  NULL;
	button = addRadioButtonColorMapToATable(table, button, _("Multi color"), i, 0,1);
	if(contoursplot->colorsMap.type == 1) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	g_object_set_data(G_OBJECT (dialogWindow), "ButtonMultiColor",button);

	i = 1;
	button = addRadioButtonColorMapToATable(table, button, _("2 colors"), i, 0,1);
	if(contoursplot->colorsMap.type == 2) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);
	g_object_set_data(G_OBJECT (dialogWindow), "Button2Colors",button);

	color.red = (gushort)(contoursplot->colorsMap.colors[0][0]*65535);
	color.green = (gushort)(contoursplot->colorsMap.colors[0][1]*65535);
	color.blue = (gushort)(contoursplot->colorsMap.colors[0][2]*65535);
	selector = gtk_color_button_new_with_color (&color);
	gtk_table_attach(GTK_TABLE(table),selector,1,2,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	g_object_set_data(G_OBJECT (dialogWindow), "Selector2Colors1",selector);

	color.red = (gushort)(contoursplot->colorsMap.colors[1][0]*65535);
	color.green = (gushort)(contoursplot->colorsMap.colors[1][1]*65535);
	color.blue = (gushort)(contoursplot->colorsMap.colors[1][2]*65535);
	selector = gtk_color_button_new_with_color (&color);
	gtk_table_attach(GTK_TABLE(table),selector,2,3,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	g_object_set_data(G_OBJECT (dialogWindow), "Selector2Colors2",selector);

	i = 2;
	button = addRadioButtonColorMapToATable(table, button, _("Unicolor"), i, 0,1);
	if(contoursplot->colorsMap.type == 3) gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (button), TRUE);

	color.red = (gushort)(contoursplot->colorsMap.colors[2][0]*65535);
	color.green = (gushort)(contoursplot->colorsMap.colors[2][1]*65535);
	color.blue = (gushort)(contoursplot->colorsMap.colors[2][2]*65535);
	selector = gtk_color_button_new_with_color (&color);
	gtk_table_attach(GTK_TABLE(table),selector,1,2,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
                  3,3);
	g_object_set_data(G_OBJECT (dialogWindow), "SelectorUniColor",selector);
	g_object_set_data(G_OBJECT (dialogWindow), "ButtonUniColor",button);

}
/****************************************************************************************************/
static void createColorMapOptionsWindow(GtkWidget* contoursplot)
{
	GtkWidget *dialogWindow = NULL;
	GtkWidget *button;
	GtkWidget *frame;
	GtkWidget *hbox;
	GtkWidget *vboxframe;
	gchar t[BSIZE];
	gint i,j;
	GtkWidget *table;
	GtkWidget* entryMin;
	GtkWidget* entryMax;
	GtkWidget* entryNLabels;
	gdouble min = -1;
	gdouble max = 1;
  	ColorMap* colorMap = NULL;

	colorMap =GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap;
	if(colorMap && colorMap->numberOfColors>0)
	{
    		min = colorMap->colorValue[0].value;
    		max = colorMap->colorValue[colorMap->numberOfColors-1].value;
	}
	 
	dialogWindow = gtk_dialog_new();
	gtk_widget_realize(GTK_WIDGET(dialogWindow));
	sprintf(t, _("Color Mapping options"));
	gtk_window_set_title(GTK_WINDOW(dialogWindow),t);

	gtk_window_set_modal (GTK_WINDOW (dialogWindow), TRUE);
	gtk_window_set_position(GTK_WINDOW(dialogWindow),GTK_WIN_POS_CENTER);

	g_signal_connect(G_OBJECT(dialogWindow), "delete_event", (GCallback)gtk_widget_destroy, NULL);
	g_object_set_data(G_OBJECT (dialogWindow), "ContoursPlot",contoursplot);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (GTK_WIDGET(GTK_DIALOG(dialogWindow)->vbox)), frame, TRUE, TRUE, 3);

	vboxframe = gtk_vbox_new (FALSE, 1);
	gtk_widget_show (vboxframe);
	gtk_container_add (GTK_CONTAINER (frame), vboxframe);

	hbox = gtk_hbox_new (FALSE, 1);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vboxframe), hbox, TRUE, TRUE, 0);

	createColorMapOptionsFrame(dialogWindow,hbox);

	hbox = gtk_hbox_new (FALSE, 1);
	gtk_widget_show (hbox);
	gtk_box_pack_start (GTK_BOX (vboxframe), hbox, TRUE, TRUE, 0);

	table = gtk_table_new(3,3,FALSE);
	gtk_box_pack_start (GTK_BOX (hbox), table, TRUE, TRUE, 3);

	i = 0;
	j = 0;
	add_label_at_table(table,_(" Min iso-value "),i,j,GTK_JUSTIFY_LEFT);
	j++;
	add_label_at_table(table," : ",i,j,GTK_JUSTIFY_LEFT);
	j++;
	entryMin = gtk_entry_new ();
	add_widget_table(table,entryMin,i,j);
	sprintf(t,"%f",min);
	gtk_entry_set_text(GTK_ENTRY(entryMin),t);

	i = 1;
	j = 0;
	add_label_at_table(table,_(" Max iso-value "),i,j,GTK_JUSTIFY_LEFT);
	j++;
	add_label_at_table(table," : ",i,j,GTK_JUSTIFY_LEFT);
	j++;
	entryMax = gtk_entry_new ();
	add_widget_table(table,entryMax,i,j);
	sprintf(t,"%f",max);
	gtk_entry_set_text(GTK_ENTRY(entryMax),t);

	i = 2;
	j = 0;
	add_label_at_table(table,_(" # of labels "),i,j,GTK_JUSTIFY_LEFT);
	j++;
	add_label_at_table(table," : ",i,j,GTK_JUSTIFY_LEFT);
	j++;
	entryNLabels = gtk_entry_new ();
	add_widget_table(table,entryNLabels,i,j);
	if(GABEDIT_ContoursPLOT(contoursplot)->colormap_nlegends>0) 
		sprintf(t,"%d",GABEDIT_ContoursPLOT(contoursplot)->colormap_nlegends);
	else sprintf(t,"20");
	gtk_entry_set_text(GTK_ENTRY(entryNLabels),t);

	g_object_set_data(G_OBJECT (dialogWindow), "EntryMin",entryMin);
	g_object_set_data(G_OBJECT (dialogWindow), "EntryMax",entryMax);
	g_object_set_data(G_OBJECT (dialogWindow), "EntryNLabels",entryNLabels);

	gtk_box_set_homogeneous (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), TRUE);

	button = gtk_button_new_from_stock (GTK_STOCK_CLOSE);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), button, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(dialogWindow));

	button = gtk_button_new_from_stock (GTK_STOCK_APPLY);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), button, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)applyColorMapOptions, GTK_OBJECT(dialogWindow));

	button = gtk_button_new_from_stock (GTK_STOCK_OK);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(dialogWindow)->action_area), button, FALSE, TRUE, 5);	
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_widget_grab_default(button);
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)applyColorMapOptions, GTK_OBJECT(dialogWindow));
	g_signal_connect_swapped(G_OBJECT(button), "clicked", (GCallback)gtk_widget_destroy, GTK_OBJECT(dialogWindow));
	

	gtk_widget_show_all(dialogWindow);
}
/****************************************************************************************/
static void toggle_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	gboolean enable = gtk_toggle_action_get_active (GTK_TOGGLE_ACTION (action));
	GtkWidget* contoursplot = g_object_get_data(G_OBJECT (action), "ContoursPLOT");
	if(!contoursplot) return;
	if(!strcmp(name,"HGridShowMajor")) 
		contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_HMAJOR_GRID, enable);
	else if(!strcmp(name,"HGridShowMinor")) 
		contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_HMINOR_GRID, enable);
	else if(!strcmp(name,"VGridShowMajor"))
		contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_VMAJOR_GRID, enable);
	else if(!strcmp(name,"VGridShowMinor"))
		contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_VMINOR_GRID, enable);
	else if(!strcmp(name,"ShowColorMap"))
		contoursplot_show_colormap (GABEDIT_ContoursPLOT(contoursplot), enable);
	else if(!strcmp(name,"ShowLabelContours"))
		contoursplot_show_label_contours (GABEDIT_ContoursPLOT(contoursplot), enable);
	else if(!strcmp(name,"DashedNegativeContours"))
		contoursplot_dashed_negative_contours (GABEDIT_ContoursPLOT(contoursplot), enable);
	else if(!strcmp(name,"LegendShowLeft"))
		contoursplot_show_left_legends (GABEDIT_ContoursPLOT(contoursplot), enable);
	else if(!strcmp(name,"LegendShowRight"))
		contoursplot_show_right_legends (GABEDIT_ContoursPLOT(contoursplot), enable);
	else if(!strcmp(name,"LegendShowTop"))
		contoursplot_show_top_legends (GABEDIT_ContoursPLOT(contoursplot), enable);
	else if(!strcmp(name,"LegendShowBottom"))
		contoursplot_show_bottom_legends (GABEDIT_ContoursPLOT(contoursplot), enable);
	else if(!strcmp(name,"DirectionReflectX"))
		contoursplot_reflect_x (GABEDIT_ContoursPLOT(contoursplot), enable);
	else if(!strcmp(name,"DirectionReflectY"))
		contoursplot_reflect_y (GABEDIT_ContoursPLOT(contoursplot), enable);
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
	{ "DirectionReflectX", NULL, N_("_X reflect"), NULL, "X reflect", G_CALLBACK (toggle_action), FALSE },
	{ "DirectionReflectY", NULL, N_("_Y reflect"), NULL, "Y reflect", G_CALLBACK (toggle_action), FALSE },
	{ "ShowColorMap", NULL, N_("show _colormap"), NULL, "show colormap", G_CALLBACK (toggle_action), FALSE },
	{ "ShowLabelContours", NULL, N_("show _label contours"), NULL, "show label contours", G_CALLBACK (toggle_action), FALSE },
	{ "DashedNegativeContours", NULL, N_("dashed negative contours"), NULL, "dashed negative contours", G_CALLBACK (toggle_action), FALSE },
};
/*--------------------------------------------------------------------------------------------------------------------*/
static void init_toggle_entries (GtkWidget* contoursplot)
{
	contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_HMAJOR_GRID, gtkActionToggleEntries[0].is_active);
	contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_HMINOR_GRID, gtkActionToggleEntries[1].is_active);
	contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_VMAJOR_GRID, gtkActionToggleEntries[2].is_active);
	contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_VMINOR_GRID, gtkActionToggleEntries[3].is_active);
	contoursplot_show_left_legends (GABEDIT_ContoursPLOT(contoursplot), gtkActionToggleEntries[4].is_active);
	contoursplot_show_right_legends (GABEDIT_ContoursPLOT(contoursplot), gtkActionToggleEntries[5].is_active);
	contoursplot_show_top_legends (GABEDIT_ContoursPLOT(contoursplot), gtkActionToggleEntries[6].is_active);
	contoursplot_show_bottom_legends (GABEDIT_ContoursPLOT(contoursplot), gtkActionToggleEntries[7].is_active);

	contoursplot_reflect_x (GABEDIT_ContoursPLOT(contoursplot), gtkActionToggleEntries[8].is_active);
	contoursplot_reflect_y (GABEDIT_ContoursPLOT(contoursplot), gtkActionToggleEntries[9].is_active);
	contoursplot_show_colormap (GABEDIT_ContoursPLOT(contoursplot), gtkActionToggleEntries[10].is_active);
	contoursplot_show_label_contours (GABEDIT_ContoursPLOT(contoursplot), gtkActionToggleEntries[11].is_active);
	contoursplot_dashed_negative_contours (GABEDIT_ContoursPLOT(contoursplot), gtkActionToggleEntries[12].is_active);
}
static gint numberOfGtkActionToggleEntries = G_N_ELEMENTS (gtkActionToggleEntries);
/*********************************************************************************************************************/
static void activate_action (GtkAction *action)
{
	const gchar *name = gtk_action_get_name (action);
	GtkWidget* contoursplot = g_object_get_data(G_OBJECT (action), "ContoursPLOT");
	if(!strcmp(name,"SetTicks")) { set_ticks_dialog(contoursplot); }
	if(!strcmp(name,"SetMargins")) { set_margins_dialog(contoursplot); }
	if(!strcmp(name,"SetRanges")) { set_ranges_dialog(contoursplot); }
	if(!strcmp(name,"SetLabels")) { set_labels_dialog(contoursplot); }
	if(!strcmp(name,"SetDigits")) { set_digits_dialog(contoursplot); }
	if(!strcmp(name,"SetFontSize")) { set_font_size_dialog(contoursplot); }
	if(!strcmp(name,"SetAutoRanges")) { gabedit_contoursplot_set_autorange(GABEDIT_ContoursPLOT(contoursplot), NULL); }
	if(!strcmp(name,"DataRead")) { read_data_dlg(contoursplot); }
	if(!strcmp(name,"DataRemoveAll")) { remove_all_data_dlg(contoursplot); }
	if(!strcmp(name,"ContoursSet")) {  create_contours(contoursplot,NULL);}
	if(!strcmp(name,"ContoursDelete")) {  delete_contours(contoursplot,NULL);}
	if(!strcmp(name,"ContoursBuildLabelPositions")) {  
		contoursplot_build_position_label_contours(GABEDIT_ContoursPLOT(contoursplot));
	}
	if(!strcmp(name,"ColorMapSet")) { createColorMapOptionsWindow(contoursplot);}
	if(!strcmp(name,"ObjectsInsertText")) { begin_insert_objects_text(contoursplot); }
	if(!strcmp(name,"ObjectsDeleteTexts")) { delete_objects_text(contoursplot); }
	if(!strcmp(name,"ObjectsInsertLine")) { begin_insert_objects_line(contoursplot); }
	if(!strcmp(name,"ObjectsDeleteLines")) { delete_objects_line(contoursplot); }
	if(!strcmp(name,"ObjectsInsertImage")) { begin_insert_objects_image(contoursplot); }
	if(!strcmp(name,"ObjectsInsertImageClip")) { insert_objects_image_from_clipboard(contoursplot); }
	if(!strcmp(name,"ObjectsDeleteImages")) { delete_objects_image(contoursplot); }
	if(!strcmp(name,"ThemesPublication")) { set_theme_publication(contoursplot); }
	if(!strcmp(name,"ThemesGreenBlack")) { set_theme_green_black(contoursplot); }
	if(!strcmp(name,"ThemesOther")) { set_theme_dialog(contoursplot); }
	if(!strcmp(name,"ScreenCaptureBMP")) {  saveImageDlg(contoursplot, "bmp");}
	if(!strcmp(name,"ScreenCaptureJPEG")) {  saveImageDlg(contoursplot, "jpeg");}
	if(!strcmp(name,"ScreenCapturePNG")) {  saveImageDlg(contoursplot, "png");}
	if(!strcmp(name,"ScreenCaptureTPNG")) {  saveImageDlg(contoursplot, "tpng");}
	if(!strcmp(name,"ScreenCaptureTIF")) {  saveImageDlg(contoursplot, "tif");}
	if(!strcmp(name,"ScreenCaptureClipBoard")) {  copyImageToClipBoard(contoursplot); }
	if(!strcmp(name,"ExportSVG")) {  exportImageDlg(contoursplot, "svg");}
	if(!strcmp(name,"ExportPDF")) {  exportImageDlg(contoursplot, "pdf");}
	if(!strcmp(name,"ExportPS")) {  exportImageDlg(contoursplot, "ps");}
	if(!strcmp(name,"ExportEPS")) {  exportImageDlg(contoursplot, "eps");}
	if(!strcmp(name,"Read")) {  readAGabeditDlg(contoursplot);}
	if(!strcmp(name,"Save")) {  saveAsGabeditDlg(contoursplot);}
	if(!strcmp(name,"Help")) {  gabedit_contoursplot_help();}
	if(!strcmp(name,"Close")) {  destroy_contoursplot_window(contoursplot);}
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
	{"SetAutoRanges", NULL, N_("_Auto ranges"), NULL, "Auto ranges", G_CALLBACK (activate_action) },
	{"Render", NULL, N_("_Render")},
	{"RenderGrid", NULL, N_("_Grid")},
	{"RenderLegends", NULL, N_("_Legends")},
	{"RenderDirections", NULL, N_("_Directions")},
	{"Data", NULL, N_("_Data")},
	{"DataRead", NULL, N_("_Read data from an ASCII grid file"), NULL, "Read data from an ASCII grid file", G_CALLBACK (activate_action) },
	{"DataSaveAll", NULL, N_("_Save all data in an ascii grid file"), NULL, "Save all data in an ascii file", G_CALLBACK (activate_action) },
	{"DataRemoveAll", NULL, N_("_Remove all data"), NULL, "Remove all data", G_CALLBACK (activate_action) },

	{"Contours", NULL, N_("_Contours")},
	{"ContoursSet", NULL, N_("_Set contours values"), NULL, "Set contours values", G_CALLBACK (activate_action) },
	{"ContoursBuildLabelPositions", NULL, N_("Reset _label positions"), NULL, "Reset label positions", G_CALLBACK (activate_action) },
	{"ContoursDelete", NULL, N_("_Delete all"), NULL, "Delete all", G_CALLBACK (activate_action) },

	{"ColorMap", NULL, N_("Color_Map")},
	{"ColorMapSet", NULL, N_("_Set colors map"), NULL, "Set colors map", G_CALLBACK (activate_action) },

	{"Objects", NULL, N_("_Objects")},
	{"ObjectsInsertText", NULL, N_("Insert a _text(Click in window)"), NULL, "Insert a text", G_CALLBACK (activate_action) },
	{"ObjectsDeleteTexts", NULL, N_("Delete all _texts"), NULL, "Delete all texts", G_CALLBACK (activate_action) },
	{"ObjectsInsertLine", NULL, N_("Insert a _line(Click in window and move)"), NULL, "Insert a line", G_CALLBACK (activate_action) },
	{"ObjectsDeleteLines", NULL, N_("Delete all _lines"), NULL, "Delete all lines", G_CALLBACK (activate_action) },
	{"ObjectsInsertImage", NULL, N_("Insert an _image(Click in window)"), NULL, "Insert an image", G_CALLBACK (activate_action) },
	{"ObjectsInsertImageClip", NULL, N_("Insert an _image from clipboard"), NULL, "Insert an image", G_CALLBACK (activate_action) },
	{"ObjectsDeleteImages", NULL, N_("Delete all _images"), NULL, "Delete all images", G_CALLBACK (activate_action) },

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
	{"Export", NULL, "_Export"},
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
static void add_data_to_actions(GtkUIManager *manager, GtkWidget   *contoursplot)
{
	GtkAction* action = NULL;
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Set/SetTicks");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Set/Margins");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Set/SetRanges");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Set/SetLabels");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Set/SetDigits");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Set/SetFontSize");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Set/SetAutoRanges");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderGrid/HGridShowMajor");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderGrid/HGridShowMinor");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderGrid/VGridShowMajor");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderGrid/VGridShowMinor");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderLegends/LegendShowLeft");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderLegends/LegendShowRight");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderLegends/LegendShowTop");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderLegends/LegendShowBottom");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderDirections/DirectionReflectX");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderDirections/DirectionReflectY");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/ShowColorMap");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/ShowLabelContours");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/DashedNegativeContours");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Data/DataRead");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Data/DataSaveAll");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Data/DataRemoveAll");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Contours/ContoursSet");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Contours/ContoursDelete");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Contours/ContoursBuildLabelPositions");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/ColorMap/ColorMapSet");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Objects/ObjectsDeleteTexts");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Objects/ObjectsInsertText");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Objects/ObjectsDeleteLines");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Objects/ObjectsInsertLine");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Objects/ObjectsInsertImage");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Objects/ObjectsInsertImageClip");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Objects/ObjectsDeleteImages");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Themes/ThemesPublication");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Themes/ThemesGreenBlack");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Themes/ThemesOther");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);



	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/ScreenCapture/ScreenCaptureBMP");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/ScreenCapture/ScreenCaptureJPEG");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/ScreenCapture/ScreenCapturePNG");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/ScreenCapture/ScreenCaptureTPNG");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/ScreenCapture/ScreenCaptureTIF");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/ScreenCapture/ScreenCaptureClipBoard");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Export/ExportSVG");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Export/ExportPDF");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Export/ExportPS");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Export/ExportEPS");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);
	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Read");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Save");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);


	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Help");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

	action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Close");
	if(action) g_object_set_data(G_OBJECT (action), "ContoursPLOT", contoursplot);

}
/********************************************************************************/
static const gchar *uiMenuInfo =
"  <popup name=\"MenuContoursPlot\">\n"
"    <separator name=\"sepMenuPopSet\" />\n"
"    <menu name=\"Set\" action=\"Set\">\n"
"      <menuitem name=\"SetTicks\" action=\"SetTicks\" />\n"
"      <menuitem name=\"SetMargins\" action=\"SetMargins\" />\n"
"      <menuitem name=\"SetRanges\" action=\"SetRanges\" />\n"
"      <menuitem name=\"SetLabels\" action=\"SetLabels\" />\n"
"      <menuitem name=\"SetDigits\" action=\"SetDigits\" />\n"
"      <menuitem name=\"SetFontSize\" action=\"SetFontSize\" />\n"
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
"      </menu>\n"
"      <menu name=\"RenderDirections\" action=\"RenderDirections\">\n"
"        <menuitem name=\"DirectionReflectX\" action=\"DirectionReflectX\" />\n"
"        <menuitem name=\"DirectionReflectY\" action=\"DirectionReflectY\" />\n"
"      </menu>\n"
"        <separator name=\"sepColorMap\" />\n"
"        <menuitem name=\"ShowColorMap\" action=\"ShowColorMap\" />\n"
"        <menuitem name=\"ShowLabelContours\" action=\"ShowLabelContours\" />\n"
"        <menuitem name=\"DashedNegativeContours\" action=\"DashedNegativeContours\" />\n"
"    </menu>\n"
"    <separator name=\"sepData\" />\n"
"    <menu name=\"Data\" action=\"Data\">\n"
"        <menuitem name=\"DataRead\" action=\"DataRead\" />\n"
"        <separator name=\"sepDataSaveAll\" />\n"
"        <menuitem name=\"DataSaveAll\" action=\"DataSaveAll\" />\n"
"        <separator name=\"sepDataRemoveAll\" />\n"
"        <menuitem name=\"DataRemoveAll\" action=\"DataRemoveAll\" />\n"
"    </menu>\n"
"    <separator name=\"sepContours\" />\n"
"    <menu name=\"Contours\" action=\"Contours\">\n"
"      <menuitem name=\"ContoursSet\" action=\"ContoursSet\" />\n"
"      <menuitem name=\"ContoursBuildLabelPositions\" action=\"ContoursBuildLabelPositions\" />\n"
"      <menuitem name=\"ContoursDelete\" action=\"ContoursDelete\" />\n"
"    </menu>\n"
"    <separator name=\"sepColorMaps\" />\n"
"    <menu name=\"ColorMap\" action=\"ColorMap\">\n"
"      <menuitem name=\"ColorMapSet\" action=\"ColorMapSet\" />\n"
"    </menu>\n"
"    <separator name=\"sepObjects\" />\n"
"    <menu name=\"Objects\" action=\"Objects\">\n"
"      <menuitem name=\"ObjectsInsertText\" action=\"ObjectsInsertText\" />\n"
"      <menuitem name=\"ObjectsDeleteTexts\" action=\"ObjectsDeleteTexts\" />\n"
"      <menuitem name=\"ObjectsInsertLine\" action=\"ObjectsInsertLine\" />\n"
"      <menuitem name=\"ObjectsDeleteLines\" action=\"ObjectsDeleteLines\" />\n"
"      <menuitem name=\"ObjectsInsertImage\" action=\"ObjectsInsertImage\" />\n"
"      <menuitem name=\"ObjectsInsertImageClip\" action=\"ObjectsInsertImageClip\" />\n"
"      <menuitem name=\"ObjectsDeleteImages\" action=\"ObjectsDeleteImages\" />\n"
"    </menu>\n"
"    <separator name=\"sepThemes\" />\n"
"    <menu name=\"Themes\" action=\"Themes\">\n"
"      <menuitem name=\"ThemesPublication\" action=\"ThemesPublication\" />\n"
"      <menuitem name=\"ThemesGreenBlack\" action=\"ThemesGreenBlack\" />\n"
"      <menuitem name=\"ThemesOther\" action=\"ThemesOther\" />\n"
"    </menu>\n"
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
"  <toolbar action=\"ToolbarContoursPlot\">\n"
"      <toolitem name=\"SetTicks\" action=\"SetTicks\" />\n"
"      <toolitem name=\"SetMargins\" action=\"SetMargins\" />\n"
"      <toolitem name=\"SetRanges\" action=\"SetRanges\" />\n"
"      <toolitem name=\"ContoursSet\" action=\"ContoursSet\" />\n"
"      <toolitem name=\"ColorMapSet\" action=\"ColorMapSet\" />\n"
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

	actionGroup = gtk_action_group_new ("GabeditContoursPlotActions");
	gtk_action_group_set_translation_domain(actionGroup,GETTEXT_PACKAGE);
	gtk_action_group_add_actions (actionGroup, gtkActionEntries, numberOfGtkActionEntries, NULL);
	gtk_action_group_add_toggle_actions (actionGroup, gtkActionToggleEntries, numberOfGtkActionToggleEntries, NULL);
  	gtk_ui_manager_insert_action_group (merge, actionGroup, 0);

	if(box) g_signal_connect (merge, "add_widget", G_CALLBACK (add_widget), box);
	if (!gtk_ui_manager_add_ui_from_string (merge, uiMenuInfo, -1, &error))
	{
		g_message (_("building menus of contoursplot failed: %s"), error->message);
		g_error_free (error);
	}
	g_object_set_data(G_OBJECT (parent), "Manager", merge);
	add_data_to_actions(merge,parent);
	init_toggle_entries (parent);
}
/****************************************************************************************/
GType gabedit_contoursplot_get_type ()
{

	static GType contoursplot_type = 0;
	if (!contoursplot_type)
       	{
             static const GTypeInfo contoursplot_info =
            {
	           sizeof (GabeditContoursPlotClass),
	           NULL,           /* base_init */
	           NULL,           /* base_finalize */
	           (GClassInitFunc) gabedit_contoursplot_class_init,
	           NULL,           /* class_finalize */
	           NULL,           /* class_data */
	           sizeof (GabeditContoursPlot),
	           0,             /* n_preallocs */
	           (GInstanceInitFunc) gabedit_contoursplot_init,
           };
	   contoursplot_type = g_type_register_static (GTK_TYPE_WIDGET, "GabeditContoursPlot", &contoursplot_info, 0);
	}
	return contoursplot_type;
}
/****************************************************************************************/
static void gabedit_contoursplot_class_init (GabeditContoursPlotClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  object_class = (GtkObjectClass*) class;
  widget_class = (GtkWidgetClass*) class;

  parent_class = g_type_class_peek_parent (class);

  object_class->destroy = gabedit_contoursplot_destroy;

  widget_class->realize = gabedit_contoursplot_realize;
  widget_class->expose_event = gabedit_contoursplot_expose;
  widget_class->size_request = gabedit_contoursplot_size_request;
  widget_class->size_allocate = gabedit_contoursplot_size_allocate;
  widget_class->button_press_event = gabedit_contoursplot_button_press;
  widget_class->button_release_event = gabedit_contoursplot_button_release;
  widget_class->motion_notify_event = gabedit_contoursplot_motion_notify;
  widget_class->scroll_event = gabedit_contoursplot_scroll;
  widget_class->style_set = gabedit_contoursplot_style_set;
  widget_class->key_press_event = gabedit_contoursplot_key_press;
  widget_class->key_release_event = gabedit_contoursplot_key_release;
  widget_class->enter_notify_event = gabedit_contoursplot_grab;
  widget_class->leave_notify_event = gabedit_contoursplot_grab;
}
/****************************************************************************************/
static void gabedit_contoursplot_init (GabeditContoursPlot *contoursplot)
{
  gint i,j;
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

  contoursplot->colorsMap.type=1;
  for(i=0;i<3;i++)
  	for(j=0;j<3;j++)
  		contoursplot->colorsMap.colors[i][j] = 1.0;
 for(j=0;j<3;j++)
 		contoursplot->colorsMap.colors[2][j] = 0.0;
  contoursplot->colorsMap.colorMap = NULL;

  contoursplot->show_label_contours = FALSE;
     
  contoursplot->xmin=-1.0;
  contoursplot->xmax=1.0;
  contoursplot->ymin=-1.0;
  contoursplot->ymax=1.0;

  contoursplot->left_margins =0;
  contoursplot->top_margins =0;
  contoursplot->right_margins =0;
  contoursplot->bottom_margins =0;
    
  contoursplot->x_legends_digits=8;
  contoursplot->y_legends_digits=8;

  contoursplot->hmajor_ticks=11;
  contoursplot->hminor_ticks=3;
  contoursplot->vmajor_ticks=11;
  contoursplot->vminor_ticks=3;
  contoursplot->length_ticks=3;
  
  contoursplot->hmajor_grid=TRUE;
  contoursplot->hminor_grid=TRUE;
  contoursplot->vmajor_grid=TRUE;
  contoursplot->vminor_grid=TRUE;  

  contoursplot->show_left_legends = TRUE;
  contoursplot->show_right_legends = FALSE;
  contoursplot->show_top_legends = FALSE;
  contoursplot->show_bottom_legends  = TRUE;

  contoursplot->show_colormap = FALSE;
  contoursplot->colormap_nlegends = 20;
  contoursplot->colormap_width = 0;
  contoursplot->colormap_height = 0;
  contoursplot->colormap_legends = 0;
  contoursplot->colormap_legends_str = 0;

  contoursplot->reflect_x  = FALSE;
  contoursplot->reflect_y  = FALSE;

  contoursplot->data_list=NULL;

  contoursplot->mouse_zoom_enabled=TRUE;
  contoursplot->mouse_zoom_button = 2;
  contoursplot->mouse_displace_enabled=FALSE;
  contoursplot->wheel_zoom_enabled=FALSE;
  contoursplot->mouse_autorange_enabled=FALSE;
  contoursplot->mouse_autorange_button = 1;
  contoursplot->mouse_distance_enabled=TRUE;
  contoursplot->mouse_distance_button = 1;

  contoursplot->h_label_str = NULL;
  contoursplot->v_label_str = NULL;
  contoursplot->h_label = NULL;
  contoursplot->v_label = NULL;
  contoursplot->h_label_width = 0;
  contoursplot->h_label_height = 0;
  contoursplot->v_label_width = 0;
  contoursplot->v_label_height = 0;
  contoursplot->font_size = 12;
  contoursplot->cairo_widget = NULL;
  contoursplot->cairo_area = NULL;
  contoursplot->cairo_export = NULL;
  contoursplot->shift_key_pressed = FALSE;
  contoursplot->control_key_pressed = FALSE;

  contoursplot->dashed_negative_contours = FALSE;
  contoursplot->d_key_pressed = FALSE;

  contoursplot->nObjectsText = 0;
  contoursplot->objectsText = NULL;
  contoursplot->t_key_pressed = FALSE;
  contoursplot->selected_objects_text_num = -1;

  contoursplot->nObjectsLine = 0;
  contoursplot->objectsLine = NULL;
  contoursplot->l_key_pressed = FALSE;

  contoursplot->nObjectsLine = 0;
  contoursplot->objectsLine = NULL;
  contoursplot->l_key_pressed = FALSE;
  contoursplot->selected_objects_image_num = -1;


  contoursplot->selected_objects_line_num = -1;
  contoursplot->selected_objects_line_type = -1;
  contoursplot->object_begin_point.x=-1;
  contoursplot->object_begin_point.y=-1;
  contoursplot->object_end_point.x=-1;
  contoursplot->object_end_point.y=-1;
  contoursplot->r_key_pressed = FALSE;
}
/****************************************************************************************/
GtkWidget* gabedit_contoursplot_new ()
{
  GabeditContoursPlot *contoursplot;

  contoursplot = g_object_new (GABEDIT_TYPE_ContoursPLOT, NULL);
  
  return GTK_WIDGET (contoursplot);
}
/****************************************************************************************/
static void gabedit_contoursplot_destroy (GtkObject *object)
{
  GabeditContoursPlot *contoursplot;

  g_return_if_fail (object != NULL);
  g_return_if_fail (G_IS_OBJECT (object));
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (object));

  contoursplot = GABEDIT_ContoursPLOT (object);

  if (contoursplot->plotting_area && G_IS_OBJECT(contoursplot->plotting_area))
  {
    g_object_unref(G_OBJECT(contoursplot->plotting_area));
    contoursplot->plotting_area = NULL;
  }
  if (contoursplot->old_area && G_IS_OBJECT(contoursplot->old_area))
  {
    g_object_unref(G_OBJECT(contoursplot->old_area));
    contoursplot->old_area = NULL;
  }
  if (contoursplot->cairo_area)
  {
    cairo_destroy (contoursplot->cairo_area);
    contoursplot->cairo_area = NULL;
  }
  if (contoursplot->cairo_widget)
  {
    cairo_destroy (contoursplot->cairo_widget);
    contoursplot->cairo_widget = NULL;
  }

  if (contoursplot->back_gc && G_IS_OBJECT(contoursplot->back_gc))
  {
    g_object_unref(contoursplot->back_gc);
    contoursplot->back_gc = NULL;
  }
  if (contoursplot->fore_gc && G_IS_OBJECT(contoursplot->fore_gc))
  {
    g_object_unref(contoursplot->fore_gc);
    contoursplot->fore_gc = NULL;
  }

  if (contoursplot->data_gc &&  G_IS_OBJECT(contoursplot->data_gc))
  {
    g_object_unref(contoursplot->data_gc);
    contoursplot->data_gc = NULL;
  }

  if (contoursplot->lines_gc &&  G_IS_OBJECT(contoursplot->lines_gc))
  {
    g_object_unref(contoursplot->lines_gc);
    contoursplot->lines_gc = NULL;
  }


  if (contoursplot->hmajor_grid_gc && G_IS_OBJECT(contoursplot->hmajor_grid_gc))
  {
    g_object_unref(contoursplot->hmajor_grid_gc);
    contoursplot->hmajor_grid_gc = NULL;
  }

  if (contoursplot->hminor_grid_gc && G_IS_OBJECT(contoursplot->hminor_grid_gc))
  {
    g_object_unref(contoursplot->hminor_grid_gc);
    contoursplot->hminor_grid_gc = NULL;
  }

  if (contoursplot->vmajor_grid_gc && G_IS_OBJECT(contoursplot->vmajor_grid_gc))
  {
    g_object_unref(contoursplot->vmajor_grid_gc);
    contoursplot->vmajor_grid_gc = NULL;
  }

  if (contoursplot->vminor_grid_gc && G_IS_OBJECT(contoursplot->vminor_grid_gc))
  {
    g_object_unref(contoursplot->vminor_grid_gc);
    contoursplot->vminor_grid_gc = NULL;
  }

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    (* GTK_OBJECT_CLASS (parent_class)->destroy) (object);
  
  gtk_object_destroy (object);
}
/****************************************************************************************/
static gint contoursplot_get_font_size (GtkWidget* widget, PangoFontDescription* font_desc)
{
	gint font_size = 8;
	if(!font_desc) return font_size;
	font_size = pango_font_description_get_size (font_desc) / PANGO_SCALE;
	return font_size;
}
/********************************************************************************/
static gint gabedit_contoursplot_grab(GtkWidget* widget, GdkEventCrossing* event)
{
	if(!event) return FALSE;
	switch(event->type)
	{
	      	case GDK_ENTER_NOTIFY: 
		      	gtk_grab_add(GTK_WIDGET (widget));
			break;
		case GDK_LEAVE_NOTIFY:
			{
				GabeditContoursPlot *contoursplot = NULL;
				if(widget && GABEDIT_IS_ContoursPLOT (widget))
					contoursplot = GABEDIT_ContoursPLOT (widget);
		      		gtk_grab_remove(GTK_WIDGET (widget));
				if(contoursplot)
				{
  					contoursplot->shift_key_pressed = FALSE;
  					contoursplot->control_key_pressed = FALSE;
				}
			}
			break;
		 default:
			break;
	}
	return TRUE;
}
/********************************************************************************/
static gint gabedit_contoursplot_key_press(GtkWidget* widget, GdkEventKey *event)
{
	GabeditContoursPlot *contoursplot;
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GABEDIT_IS_ContoursPLOT (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	contoursplot = GABEDIT_ContoursPLOT (widget);

	if((event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R) )
  		contoursplot->shift_key_pressed = TRUE;
	if((event->keyval == GDK_Control_L || event->keyval == GDK_Control_R) )
  		contoursplot->control_key_pressed = TRUE;
	if((event->keyval == GDK_Alt_L || event->keyval == GDK_Alt_L) )
  		contoursplot->control_key_pressed = TRUE;

	if((event->keyval == GDK_d || event->keyval == GDK_D) )
  		contoursplot->d_key_pressed = TRUE;

	if((event->keyval == GDK_c || event->keyval == GDK_C) )
	{
		if(contoursplot->control_key_pressed) 
			copyImageToClipBoard(widget);

	}
	if((event->keyval == GDK_v || event->keyval == GDK_V) )
	{
		if(contoursplot->control_key_pressed) 
		{
			add_object_image(contoursplot, 0, 0, -1, -1, NULL);
    			gtk_widget_queue_draw(widget);
		}
	}

	if((event->keyval == GDK_t || event->keyval == GDK_T) )
  		contoursplot->t_key_pressed = TRUE;
	if((event->keyval == GDK_l || event->keyval == GDK_L) )
  		contoursplot->l_key_pressed = TRUE;
	if((event->keyval == GDK_i || event->keyval == GDK_I) )
  		contoursplot->i_key_pressed = TRUE;
	if((event->keyval == GDK_r || event->keyval == GDK_R) )
  		contoursplot->r_key_pressed = TRUE;
	return TRUE;
}
/********************************************************************************/
static gint gabedit_contoursplot_key_release(GtkWidget* widget, GdkEventKey *event)
{
	GabeditContoursPlot *contoursplot;
	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GABEDIT_IS_ContoursPLOT (widget), FALSE);
	g_return_val_if_fail (event != NULL, FALSE);

	contoursplot = GABEDIT_ContoursPLOT (widget);

	if((event->keyval == GDK_Shift_L || event->keyval == GDK_Shift_R) )
  		contoursplot->shift_key_pressed = FALSE;
	if((event->keyval == GDK_Control_L || event->keyval == GDK_Control_R) )
  		contoursplot->control_key_pressed = FALSE;
	if((event->keyval == GDK_Alt_L || event->keyval == GDK_Alt_R) )
  		contoursplot->control_key_pressed = FALSE;
	if((event->keyval == GDK_d || event->keyval == GDK_D) )
  		contoursplot->d_key_pressed = FALSE;
	if((event->keyval == GDK_t || event->keyval == GDK_T) )
  		contoursplot->t_key_pressed = FALSE;
	if((event->keyval == GDK_l || event->keyval == GDK_L) )
  		contoursplot->l_key_pressed = FALSE;
	if((event->keyval == GDK_i || event->keyval == GDK_I) )
  		contoursplot->i_key_pressed = FALSE;
	if((event->keyval == GDK_r || event->keyval == GDK_R) )
  		contoursplot->r_key_pressed = FALSE;
	return TRUE;
}
/****************************************************************************************/
static void gabedit_contoursplot_realize (GtkWidget *widget)
{
  GabeditContoursPlot *contoursplot;
  GdkWindowAttr attributes;
  gint attributes_mask;
  GdkGCValues gc_values;
  GdkGCValuesMask gc_values_mask;
  GdkColor black;
  GdkColor white;
  GdkColormap *colormap;
     
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (widget));

  GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
  contoursplot = GABEDIT_ContoursPLOT (widget);

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
  contoursplot->back_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
  
  gc_values.foreground=black;
  gc_values.line_style=GDK_LINE_SOLID;
  gc_values.line_width=2;
  gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
  contoursplot->fore_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
  
  gc_values.foreground=black;
  contoursplot->hmajor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
  contoursplot->vmajor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
  
  gc_values.foreground=black;
  gc_values.line_style=GDK_LINE_ON_OFF_DASH;
  gc_values.line_width=1;
  gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
  contoursplot->hminor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
  contoursplot->vminor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);

  contoursplot->data_gc=gdk_gc_new (widget->window);

  gc_values.foreground=black;
  gc_values.line_style=GDK_LINE_SOLID;
  gc_values.line_width=2;
  gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
  contoursplot->lines_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);

  contoursplot->font_size =  contoursplot_get_font_size (widget, widget->style->font_desc);

  /* Create the initial legends*/
  contoursplot->v_legends_str = NULL;
  contoursplot->h_legends_str = NULL;
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_legends_sizes(contoursplot);

  contoursplot->colormap_legends_str = NULL;
  contoursplot_build_colormap_legends(contoursplot);
  contoursplot_calculate_colormap_sizes(contoursplot);

  contoursplot_calculate_sizes(contoursplot);
  add_toolbar_and_popup_menu(widget, NULL);
}
/****************************************************************************************/
static void reset_theme(GtkWidget *widget, gint line_width, GdkColor* foreColor, GdkColor* backColor )
{
	GabeditContoursPlot *contoursplot;
	GdkGCValues gc_values;
	GdkGCValuesMask gc_values_mask;
	GdkColormap *colormap;
		 
	g_return_if_fail (widget != NULL);
	g_return_if_fail (GABEDIT_IS_ContoursPLOT (widget));

	contoursplot = GABEDIT_ContoursPLOT (widget);

	colormap=gdk_drawable_get_colormap(widget->window); 

	gdk_colormap_alloc_color (colormap, backColor, FALSE, TRUE);
	gdk_colormap_alloc_color (colormap, foreColor, FALSE, TRUE);

	gtk_widget_modify_bg (widget, GTK_STATE_NORMAL, backColor);
	gtk_widget_modify_fg (widget, GTK_STATE_NORMAL, foreColor);

	if (contoursplot->back_gc && G_IS_OBJECT(contoursplot->back_gc))
	{
		g_object_unref(contoursplot->back_gc);
		contoursplot->back_gc = NULL;
	}
	if (contoursplot->fore_gc && G_IS_OBJECT(contoursplot->fore_gc))
	{
		g_object_unref(contoursplot->fore_gc);
		contoursplot->fore_gc = NULL;
	}

	if (contoursplot->data_gc &&	G_IS_OBJECT(contoursplot->data_gc))
	{
		g_object_unref(contoursplot->data_gc);
		contoursplot->data_gc = NULL;
	}


	if (contoursplot->hmajor_grid_gc && G_IS_OBJECT(contoursplot->hmajor_grid_gc))
	{
		g_object_unref(contoursplot->hmajor_grid_gc);
		contoursplot->hmajor_grid_gc = NULL;
	}

	if (contoursplot->hminor_grid_gc && G_IS_OBJECT(contoursplot->hminor_grid_gc))
	{
		g_object_unref(contoursplot->hminor_grid_gc);
		contoursplot->hminor_grid_gc = NULL;
	}

	if (contoursplot->vmajor_grid_gc && G_IS_OBJECT(contoursplot->vmajor_grid_gc))
	{
		g_object_unref(contoursplot->vmajor_grid_gc);
		contoursplot->vmajor_grid_gc = NULL;
	}

	if (contoursplot->vminor_grid_gc && G_IS_OBJECT(contoursplot->vminor_grid_gc))
	{
		g_object_unref(contoursplot->vminor_grid_gc);
		contoursplot->vminor_grid_gc = NULL;
	}
		
	gc_values.foreground=*backColor;
	gc_values.line_style=GDK_LINE_SOLID;
	gc_values.line_width=line_width;
	gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
	contoursplot->back_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);

	gc_values.foreground=*foreColor;
	gc_values.line_style=GDK_LINE_SOLID;
	gc_values.line_width=line_width;
	gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
	contoursplot->fore_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
	
	gc_values.foreground=*foreColor;
	contoursplot->hmajor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
	contoursplot->vmajor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
	
	gc_values.foreground=*foreColor;
	gc_values.line_style=GDK_LINE_ON_OFF_DASH;
	if(line_width/2>0) gc_values.line_width=line_width/2;
	else gc_values.line_width=line_width;
	gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
	contoursplot->hminor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
	contoursplot->vminor_grid_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);

	contoursplot->data_gc=gdk_gc_new (widget->window);

	if (contoursplot->nObjectsLine<1)
	{
		if (contoursplot->lines_gc &&	G_IS_OBJECT(contoursplot->lines_gc))
		{
			g_object_unref(contoursplot->lines_gc);
			contoursplot->lines_gc = NULL;
		}
		gc_values.foreground=*foreColor;
		gc_values.line_style=GDK_LINE_SOLID;
		gc_values.line_width=line_width;
		gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
		contoursplot->lines_gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
	}

	contoursplot_build_legends(contoursplot);
	contoursplot_calculate_legends_sizes(contoursplot);
	contoursplot_calculate_sizes(contoursplot);
	reset_object_text_pango(contoursplot);
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
		GabeditContoursPlot* contoursplot = NULL;
		GdkColormap *colormap;

  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (widget));
		contoursplot = GABEDIT_ContoursPLOT(widget);
   		colormap  = gdk_window_get_colormap(widget->window);

		gdk_gc_get_values(contoursplot->back_gc, &gc_values);
        	gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&backColor);
		gdk_gc_get_values(contoursplot->fore_gc, &gc_values);
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
		GabeditContoursPlot* contoursplot = NULL;
		GdkColormap *colormap;

  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (widget));
		contoursplot = GABEDIT_ContoursPLOT(widget);
   		colormap  = gdk_window_get_colormap(widget->window);
		gtk_color_button_get_color (colorbutton, &backColor);
		gdk_gc_get_values(contoursplot->fore_gc, &gc_values);
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
		GabeditContoursPlot* contoursplot = NULL;
		GdkColormap *colormap;

  		g_return_if_fail (GABEDIT_IS_ContoursPLOT (widget));
		contoursplot = GABEDIT_ContoursPLOT(widget);
   		colormap  = gdk_window_get_colormap(widget->window);
		gtk_color_button_get_color (colorbutton, &foreColor);
		gdk_gc_get_values(contoursplot->back_gc, &gc_values);
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
	GabeditContoursPlot* contoursplot = NULL;
	GdkColormap *colormap;

  	g_return_if_fail (GABEDIT_IS_ContoursPLOT (widget));
	contoursplot = GABEDIT_ContoursPLOT(widget);
   	colormap  = gdk_window_get_colormap(widget->window);

	gdk_gc_get_values(contoursplot->back_gc, &gc_values);
        gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&backColor);

	gdk_gc_get_values(contoursplot->fore_gc, &gc_values);
        gdk_colormap_query_color(colormap, gc_values.foreground.pixel,&foreColor);
	line_width=gc_values.line_width;

	gtk_window_set_title (GTK_WINDOW (window), _("Set theme"));
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_modal (GTK_WINDOW (window), TRUE);

	g_signal_connect(G_OBJECT(window), "delete_event", G_CALLBACK(gtk_widget_destroy), (gpointer)contoursplot);

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

	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(theme_line_width_changed_value), contoursplot);
	g_signal_connect(G_OBJECT(buttonBack), "color-set", G_CALLBACK(theme_back_color_changed_value), contoursplot);
	g_signal_connect(G_OBJECT(buttonFore), "color-set", G_CALLBACK(theme_fore_color_changed_value), contoursplot);

	parentWindow = get_parent_window(GTK_WIDGET(contoursplot));
	if(parentWindow)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parentWindow));
	}
	gtk_widget_show(window); 
}
/****************************************************************************************/
static void gabedit_contoursplot_size_request (GtkWidget *widget, GtkRequisition *requisition)
{
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (widget));
  g_return_if_fail (requisition != NULL);

  requisition->width = ContoursPLOT_DEFAULT_SIZE;
  requisition->height = ContoursPLOT_DEFAULT_SIZE;
}
/****************************************************************************************/
static void gabedit_contoursplot_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
  GabeditContoursPlot *contoursplot;

  g_return_if_fail (widget != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (widget));
  g_return_if_fail (allocation != NULL);

  widget->allocation = *allocation;
  contoursplot=GABEDIT_ContoursPLOT(widget);  

  if (GTK_WIDGET_REALIZED (widget))
    gdk_window_move_resize (widget->window,
			allocation->x, 
			allocation->y,
			allocation->width, 
			allocation->height);
    
  contoursplot_calculate_sizes(contoursplot);
}
/****************************************************************************************/
static void draw_points(GtkWidget *widget, GabeditContoursPlot *contoursplot, ContoursPlotData* data)
{
	gint i;
	gint x, y;
	GdkRectangle rect;
	int n;

	rect.x=0;
	rect.y=0;
	rect.width=widget->allocation.width;
	rect.height=widget->allocation.height;

	for (n=0; n<data->nContours; n++)
	{
		gdk_gc_set_rgb_fg_color (contoursplot->data_gc, &data->contours[n].point_color);
		for (i=0; i<data->contours[n].size; i++)
		{
		if(data->contours[n].point_size<1) continue;
        	value2pixel(contoursplot, data->contours[n].x[i], data->contours[n].y[i], &x, &y);
        	y=contoursplot->plotting_rect.height-y;                   
		x -= data->contours[n].point_width/2;
		y -= data->contours[n].point_height/2;
		if(data->contours[n].point_pango)
		contoursplot_cairo_layout(contoursplot, contoursplot->cairo_area,  
                        x,
			y,
			data->contours[n].point_pango,FALSE,FALSE,0);
		}
	}
}
/****************************************************************************************/
static void draw_rectangle_gradient(GabeditContoursPlot *contoursplot, cairo_t* cr, gint x1, gint y1, gint x2, gint y2, gdouble c1[], gdouble c2[], gboolean vertical, gdouble alpha)
{
	cairo_pattern_t *pat;
	gint w = 0;
	gint h = 0;
	gdouble col1[3];
	gdouble col2[3];
	gint k;

	for(k=0;k<3;k++) col1[k] = c1[k];
	for(k=0;k<3;k++) col2[k] = c2[k];

	if(vertical)
	{
		if(x2<x1)
		{
			gint t = x1;
			x1 = x2;
			x2 = t;
		}
		if(y2<y1)
		{
			gint t = y1;
			y1 = y2;
			y2 = t;
			for(k=0;k<3;k++)
			{
				gdouble t = col1[k];
				col1[k] = col2[k];
				col2[k] = t;
			}
		}
	}
	else
	{
		if(y2<y1)
		{
			gint t = y1;
			y1 = y2;
			y2 = t;
		}
		if(x2<x1)
		{
			gint t = x1;
			x1 = x2;
			x2 = t;
			for(k=0;k<3;k++)
			{
				gdouble t = col1[k];
				col1[k] = col2[k];
				col2[k] = t;
			}
		}
	}
	w = x2-x1;
	h = y2-y1;

	/*
	printf("x1 = %d x2 = %d y1 = %d y2 = %d\n",x1,x2,y1,y2);
	printf("Col1 %f %f %f\n",col1[0], col1[1], col1[2]);
	printf("Col2 %f %f %f\n",col2[0], col2[1], col2[2]);
	*/
	if(vertical) pat = cairo_pattern_create_linear (x1,y1, x1,y2);
	else pat = cairo_pattern_create_linear (x1,y1, x2,y1);
	cairo_pattern_add_color_stop_rgba (pat, 0, col1[0], col1[1], col1[2],alpha);
	cairo_pattern_add_color_stop_rgba (pat, 1, col2[0], col2[1], col2[2],alpha);

	cairo_rectangle(cr, x1, y1, w, h);
	cairo_set_source(cr, pat);
	cairo_fill(cr);
	cairo_stroke (cr);

	if(contoursplot->cairo_export)
	{
		cairo_t* cr =contoursplot->cairo_export;
		cairo_rectangle(cr, x1, y1, w, h);
		cairo_set_source(cr, pat);
		cairo_fill(cr);
		cairo_stroke (cr);
	}
	/*
	cairo_restore (cr);
	*/
	cairo_pattern_destroy (pat);
}
/****************************************************************************************/
/*
static void draw_rectangle_bilinear(GabeditContoursPlot *contoursplot, 
		gint x1, gint y1, 
		gint x2, gint y2, 
		gdouble c1[], gdouble c2[], 
		gdouble c3[], gdouble c4[]
		)
{
	draw_rectangle_gradient(contoursplot, contoursplot->cairo_area, x1, y1, x2, y2, c1, c2, FALSE, 1.0);
	draw_rectangle_gradient(contoursplot, contoursplot->cairo_area, x1, y1, x2, y2, c2, c3, TRUE, 0.5);
	draw_rectangle_gradient(contoursplot, contoursplot->cairo_area, x1, y1, x2, y2, c3, c4, FALSE, 0.5);
	draw_rectangle_gradient(contoursplot, contoursplot->cairo_area, x1, y1, x2, y2, c1, c4, TRUE, 0.5);
}
*/
/****************************************************************************************/
static void draw_lines(GtkWidget *widget, GabeditContoursPlot *contoursplot, ContoursPlotData* data)
{
	GdkPoint* points;
	gint i;
	int n;

	points=(GdkPoint*)g_malloc( (sizeof(GdkPoint) * 2));
                                             
	for (n=0; n<data->nContours; n++)
	{
		gdk_gc_set_rgb_fg_color (contoursplot->data_gc, &data->contours[n].line_color);
		gdk_gc_set_line_attributes (contoursplot->data_gc, 
				data->contours[n].line_width, data->contours[n].line_style, 
				GDK_CAP_ROUND, GDK_JOIN_MITER);
		for (i=0; i<data->contours[n].size-1; i++)
		{
			if(data->contours[n].line_width<1) continue;
		if(data->contours[n].index[i] != data->contours[n].index[i+1]) continue;
		value2pixel(contoursplot, data->contours[n].x[i], data->contours[n].y[i], 
				(gint *)&points[0].x, (gint *)&points[0].y);
		points[0].y=contoursplot->plotting_rect.height-points[0].y;                   

		value2pixel(contoursplot, data->contours[n].x[i+1], data->contours[n].y[i+1], 
				(gint *)&points[1].x, (gint *)&points[1].y);
		points[1].y=contoursplot->plotting_rect.height-points[1].y;                   
		contoursplot_cairo_lines(contoursplot, contoursplot->cairo_area,  widget, contoursplot->data_gc, points, 2);
		}
	}
	g_free(points);
}
/****************************************************************************************/
static void draw_zoom_rectangle(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	if (!contoursplot->mouse_zoom_enabled || contoursplot->mouse_button != contoursplot->mouse_zoom_button) return;
	gdk_draw_rectangle(widget->window,
			contoursplot->fore_gc,
			FALSE,
			contoursplot->zoom_rect.x,
			contoursplot->zoom_rect.y,
			contoursplot->zoom_rect.width,
			contoursplot->zoom_rect.height);
}
/****************************************************************************************/
static void draw_object_line_gdk(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint arrow_size = 0;
	gint i = -1;
	if (contoursplot->object_begin_point.x<0 || contoursplot->object_begin_point.y<0) return;
	if (contoursplot->object_end_point.x<0 || contoursplot->object_end_point.y<0) return;
	i = contoursplot->selected_objects_line_num;
	if(i<0 && contoursplot->nObjectsLine>0) i = contoursplot->nObjectsLine-1;

	if(i>-1)
	{
		gdk_gc_set_rgb_fg_color (contoursplot->lines_gc, &contoursplot->objectsLine[i].color);
		gdk_gc_set_line_attributes (contoursplot->lines_gc, 
			contoursplot->objectsLine[i].width, 
			/* contoursplot->objectsLine[i].style, */
			GDK_LINE_ON_OFF_DASH,
			GDK_CAP_ROUND, 
			GDK_JOIN_MITER);
		arrow_size = contoursplot->objectsLine[i].arrow_size;
	}
	gdk_draw_line(widget->window, contoursplot->lines_gc,
			contoursplot->object_begin_point.x,
			contoursplot->object_begin_point.y,
			contoursplot->object_end_point.x,
			contoursplot->object_end_point.y
		     );
	if(arrow_size>0)
	{
		gdouble x1, y1, x2, y2;
		calc_arrow_vertexes(30.0, arrow_size*5.0,
		(gdouble)contoursplot->object_begin_point.x,
		(gdouble)contoursplot->object_begin_point.y,
		(gdouble)contoursplot->object_end_point.x,
		(gdouble)contoursplot->object_end_point.y,
		&x1, &y1, 
		&x2, &y2
		);
		gdk_draw_line(widget->window, contoursplot->lines_gc,
			(gint)x1,
			(gint)y1,
			contoursplot->object_end_point.x,
			contoursplot->object_end_point.y
		     );
		gdk_draw_line(widget->window, contoursplot->lines_gc,
			(gint)x2,
			(gint)y2,
			contoursplot->object_end_point.x,
			contoursplot->object_end_point.y
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
static void draw_rotated_rectangle(GtkWidget *widget, GabeditContoursPlot* contoursplot,
		gint x, gint y, gint w, gint h, gdouble angle)
{
	static GdkPoint* points = NULL;
	if(!points) points = g_malloc(4*sizeof(GdkPoint));
	get_rotated_rectangle( x, y, w, h, angle,
		&points[0].x, &points[0].y,
		&points[1].x, &points[1].y,
		&points[2].x, &points[2].y,
		&points[3].x, &points[3].y);
	gdk_draw_polygon(widget->window, contoursplot->fore_gc, FALSE, points, 4);
}
/****************************************************************************************/
static void draw_selected_objects_text_rectangle(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i = contoursplot->selected_objects_text_num;
	gdouble angle = 0;
	if(i<0 || i>=contoursplot->nObjectsText) return;
	if (contoursplot->mouse_zoom_enabled && contoursplot->mouse_button == contoursplot->mouse_zoom_button) return;
	/*
	gdk_draw_rectangle(widget->window,
			contoursplot->fore_gc,
			FALSE,
			contoursplot->objectsText[i].xi,
			contoursplot->objectsText[i].yi,
			contoursplot->objectsText[i].width,
			contoursplot->objectsText[i].height);
			*/
	angle = contoursplot->objectsText[i].angle;
	draw_rotated_rectangle(widget, contoursplot,
			contoursplot->objectsText[i].xi,
			contoursplot->objectsText[i].yi,
			contoursplot->objectsText[i].width,
			contoursplot->objectsText[i].height,
			angle);
}
/****************************************************************************************/
static void draw_colormap(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i;
	GdkRectangle rect;
	gint xlabel = 0;
	gint xlines = 0;
	gdouble dy = 0;
	gint ll = 1;
	ColorMap* colorMap;
	GdkColor color;
	gdouble C[3];
	gdouble value;
	gdouble vmin;
	gdouble dv;
	gint x1, y1, x2, y2;
	gdouble col1[3]={0,0,0};
	gdouble col2[3]={0,0,0};


	if (!contoursplot->show_colormap) return;
	colorMap =GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap;

  	if(!colorMap) return;
	if(colorMap->numberOfColors<2) return;
	rect.x=0;
	rect.y=0;
	rect.width=widget->allocation.width;
	rect.height=widget->allocation.height;
	xlabel = contoursplot->plotting_rect.x+contoursplot->plotting_rect.width+2;
	xlabel += contoursplot->colormap_width/2;
	ll = contoursplot->colormap_width/2-4;
	if (contoursplot->y_legends_digits!=0 && contoursplot->show_right_legends) 
	{
		xlabel += contoursplot->y_legends_width+4;
		if (contoursplot->v_label && !contoursplot->show_left_legends && contoursplot->show_right_legends)
			xlabel += contoursplot->v_label_height;
	}
	xlines = xlabel - 2 -ll;

	if(contoursplot->colormap_nlegends<2) dy = 0;
	else dy = contoursplot->colormap_height/(gdouble)(contoursplot->colormap_nlegends-1);

    	vmin = colorMap->colorValue[0].value;
    	dv = (colorMap->colorValue[colorMap->numberOfColors-1].value-colorMap->colorValue[0].value)/(contoursplot->colormap_nlegends-1);
	x1 = xlines;
	y1 = contoursplot->plotting_rect.y+contoursplot->plotting_rect.height;
	for(i=0; i<contoursplot->colormap_nlegends;i++)
	{
		contoursplot_cairo_string(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
                       xlabel,
		       contoursplot->plotting_rect.y+contoursplot->plotting_rect.height-i*dy,
				contoursplot->colormap_legends_str[i], FALSE, TRUE,0);

      		value = vmin + dv*i;
		set_Color_From_colorMap(colorMap, C, value);
		color.red=(gushort)(C[0]*65535);
		color.green=(gushort)(C[1]*65535);
		color.blue=(gushort)(C[2]*65535);
		gdk_gc_set_rgb_fg_color (contoursplot->data_gc, &color);
		gdk_gc_set_line_attributes (contoursplot->data_gc, 2, GDK_LINE_SOLID, GDK_CAP_ROUND, GDK_JOIN_MITER);
		contoursplot_cairo_line(contoursplot, contoursplot->cairo_widget, GTK_WIDGET(contoursplot), contoursplot->data_gc, 
                       		xlines, contoursplot->plotting_rect.y+contoursplot->plotting_rect.height-i*dy,
                       		xlines+ll, contoursplot->plotting_rect.y+contoursplot->plotting_rect.height-i*dy
				);
		if(i==0)
		{
			int k;
			for(k=0;k<3;k++) col1[k] = C[k];
			x1 = xlines;
		        y1 = contoursplot->plotting_rect.y+contoursplot->plotting_rect.height-i*dy;
		}
		else
		{
			int k;
			for(k=0;k<3;k++) col2[k] = C[k];
			x2 = xlines+ll;
		        y2 = contoursplot->plotting_rect.y+contoursplot->plotting_rect.height-i*dy;
			/* that not works */
			draw_rectangle_gradient(contoursplot, contoursplot->cairo_widget, x1, y1, x2, y2, col1, col2,TRUE,1.0);
			for(k=0;k<3;k++) col1[k] = col2[k];
			x1 = xlines;
			y1 = y2;
		}
	}
}
/****************************************************************************************/
static void draw_selected_objects_image_rectangle(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i = contoursplot->selected_objects_image_num;
	if(i<0 || i>=contoursplot->nObjectsImage) return;
	if (contoursplot->mouse_zoom_enabled && contoursplot->mouse_button == contoursplot->mouse_zoom_button) return;
	gdk_draw_rectangle(widget->window,
			contoursplot->fore_gc,
			FALSE,
			contoursplot->objectsImage[i].xi,
			contoursplot->objectsImage[i].yi,
			contoursplot->objectsImage[i].widthi,
			contoursplot->objectsImage[i].heighti);
}
/****************************************************************************************/
static void draw_distance_line(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	PangoLayout *playout;
	GdkRectangle rect;
	gchar begin[100]=" ";
	gchar end[100] = " ";
	gdouble x1,y1,x2,y2;
	gint x,y;


	if (!contoursplot->mouse_distance_enabled || contoursplot->mouse_button != contoursplot->mouse_distance_button) return;

	x=contoursplot->distance_rect.x;
	y=contoursplot->distance_rect.y;
	if(gabedit_contoursplot_get_point(GABEDIT_ContoursPLOT(contoursplot), x, y, &x1, &y1))
		sprintf(begin,"(%f ; %f)",x1,y1);
	x = contoursplot->distance_rect.x+contoursplot->distance_rect.width;
	y = contoursplot->distance_rect.y+contoursplot->distance_rect.height;
	if(gabedit_contoursplot_get_point(GABEDIT_ContoursPLOT(contoursplot), x, y, &x2, &y2))
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
			contoursplot->distance_rect.x,
			contoursplot->distance_rect.y,
			playout);
	if (playout) g_object_unref(G_OBJECT(playout));
	playout=gtk_widget_create_pango_layout (widget, end);
	gtk_paint_layout (widget->style, widget->window, 
			GTK_STATE_NORMAL, FALSE, 
			&rect, widget, NULL, 
			contoursplot->distance_rect.x+contoursplot->distance_rect.width,
			contoursplot->distance_rect.y+contoursplot->distance_rect.height,
			playout);
	if (playout) g_object_unref(G_OBJECT(playout));
	gdk_draw_line(widget->window,
			contoursplot->fore_gc,
			contoursplot->distance_rect.x,
			contoursplot->distance_rect.y,
			contoursplot->distance_rect.x+contoursplot->distance_rect.width,
			contoursplot->distance_rect.y+contoursplot->distance_rect.height);

}
/****************************************************************************************/
static void draw_hminor_ticks(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i;
	gint j;
	if (contoursplot->hmajor_ticks<2 || contoursplot->hminor_ticks == 0 ) return;
	for (i=0; i < contoursplot->hmajor_ticks-1; i++)
	for (j=1; j <= contoursplot->hminor_ticks; j++)
	{
		if (contoursplot->show_bottom_legends)
        	contoursplot_cairo_line(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
			contoursplot->plotting_rect.x+i*contoursplot->d_hminor*(contoursplot->hminor_ticks+1)+j*contoursplot->d_hminor, 
			contoursplot->plotting_rect.y+contoursplot->plotting_rect.height,
			contoursplot->plotting_rect.x+i*contoursplot->d_hminor*(contoursplot->hminor_ticks+1)+j*contoursplot->d_hminor,
			contoursplot->plotting_rect.y+contoursplot->plotting_rect.height+contoursplot->length_ticks);
      		if (contoursplot->show_top_legends)
        	contoursplot_cairo_line(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
			contoursplot->plotting_rect.x+i*contoursplot->d_hminor*(contoursplot->hminor_ticks+1)+j*contoursplot->d_hminor, 
			contoursplot->plotting_rect.y,
			contoursplot->plotting_rect.x+i*contoursplot->d_hminor*(contoursplot->hminor_ticks+1)+j*contoursplot->d_hminor,
			contoursplot->plotting_rect.y-contoursplot->length_ticks);
      }
}
/****************************************************************************************/
static void draw_hmajor_ticks_and_xlegends(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i;
	GdkRectangle rect;

	rect.x=0;
	rect.y=0;
	rect.width=widget->allocation.width;
	rect.height=widget->allocation.height;
  
	if (contoursplot->hmajor_ticks>1)
	for (i=0; i < contoursplot->hmajor_ticks; i++)
	{
		gint l1 = i;
		if(contoursplot->reflect_x) l1 = contoursplot->hmajor_ticks-i-1;
		if (contoursplot->show_bottom_legends)
        	contoursplot_cairo_line(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
			contoursplot->plotting_rect.x+i*contoursplot->d_hmajor, 
			contoursplot->plotting_rect.y+contoursplot->plotting_rect.height,
			contoursplot->plotting_rect.x+i*contoursplot->d_hmajor,
			contoursplot->plotting_rect.y+contoursplot->plotting_rect.height+contoursplot->length_ticks*2);
		if (contoursplot->show_top_legends)
        	contoursplot_cairo_line(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
			contoursplot->plotting_rect.x+i*contoursplot->d_hmajor, 
			contoursplot->plotting_rect.y,
			contoursplot->plotting_rect.x+i*contoursplot->d_hmajor,
			contoursplot->plotting_rect.y-contoursplot->length_ticks*2);

		if (contoursplot->x_legends_digits!=0 && contoursplot->show_bottom_legends)
		contoursplot_cairo_string(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
			contoursplot->plotting_rect.x+i*contoursplot->d_hmajor-0.5*contoursplot->x_legends_width,
			contoursplot->plotting_rect.y+contoursplot->plotting_rect.height+contoursplot->length_ticks*2,
				contoursplot->h_legends_str[l1], FALSE, FALSE,0);
		if (contoursplot->x_legends_digits!=0 && contoursplot->show_top_legends)
		contoursplot_cairo_string(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
                        contoursplot->plotting_rect.x+i*contoursplot->d_hmajor-0.5*contoursplot->x_legends_width,
			contoursplot->plotting_rect.y-contoursplot->length_ticks*2-contoursplot->x_legends_height,
				contoursplot->h_legends_str[l1], FALSE, FALSE,0);
	}
	if ( contoursplot->h_label && contoursplot->x_legends_digits!=0 && contoursplot->show_bottom_legends)
	{
		contoursplot_cairo_layout(contoursplot, contoursplot->cairo_widget,  
                        contoursplot->plotting_rect.x+0.5*contoursplot->plotting_rect.width,
			contoursplot->plotting_rect.y+contoursplot->plotting_rect.height+contoursplot->length_ticks*2+contoursplot->x_legends_height,
			contoursplot->h_label,TRUE,FALSE,0);
	}
	if (contoursplot->h_label  && contoursplot->x_legends_digits!=0 && contoursplot->show_top_legends && !contoursplot->show_bottom_legends)
	{
		contoursplot_cairo_layout(contoursplot, contoursplot->cairo_widget,  
                        contoursplot->plotting_rect.x+0.5*contoursplot->plotting_rect.width,
			contoursplot->plotting_rect.y-contoursplot->x_legends_height-contoursplot->length_ticks*2-contoursplot->h_label_height,
			contoursplot->h_label,TRUE,FALSE,0);
	}
}

/****************************************************************************************/
static void contoursplot_cairo_line_grid(GabeditContoursPlot *contoursplot, cairo_t* cr, GtkWidget* widget, GdkGC* gc, gdouble x1,gdouble y1,gdouble x2,gdouble y2)
{
	gabedit_contoursplot_cairo_line(cr,  widget, gc, x1, y1, x2, y2);
	if(contoursplot->cairo_export)
	{
		x1+= contoursplot->plotting_rect.x;
		x2+= contoursplot->plotting_rect.x;
		y1+= contoursplot->plotting_rect.y;
		y2+= contoursplot->plotting_rect.y;
		gabedit_contoursplot_cairo_line(contoursplot->cairo_export,  widget, gc,  x1, y1, x2, y2);
	}
}
/****************************************************************************************/
static void draw_hminor_grid(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i;
	gint j;
	if (contoursplot->hminor_grid && (contoursplot->vmajor_ticks > 1) && (contoursplot->vminor_ticks != 0))
	for (i=0; i < contoursplot->vmajor_ticks-1; i++)
	for (j=1; j <= contoursplot->vminor_ticks; j++)
	{
        	contoursplot_cairo_line_grid(contoursplot, contoursplot->cairo_area, widget, 
			contoursplot->hminor_grid_gc,
			0,
			i*contoursplot->d_vmajor+j*contoursplot->d_vminor,
			contoursplot->plotting_rect.width,
			i*contoursplot->d_vmajor+j*contoursplot->d_vminor);
      }
}
/****************************************************************************************/
static void draw_hmajor_grid(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i;
 
	if (contoursplot->hmajor_grid && (contoursplot->vmajor_ticks > 1) )
	for (i=1; i < (contoursplot->vmajor_ticks-1); i++)
        	contoursplot_cairo_line_grid(contoursplot, contoursplot->cairo_area, widget, 
			contoursplot->hmajor_grid_gc,
			0,
			i*contoursplot->d_vmajor,
			contoursplot->plotting_rect.width,
			i*contoursplot->d_vmajor);
}
/****************************************************************************************/
static void draw_vminor_ticks(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i;
	gint j;

	if ( (contoursplot->vmajor_ticks > 1) && (contoursplot->vminor_ticks != 0) )
	for (i=0; i < contoursplot->vmajor_ticks-1; i++)
	for (j=1; j <= contoursplot->vminor_ticks; j++)
	{
		if (contoursplot->show_left_legends)
        	contoursplot_cairo_line(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
			contoursplot->plotting_rect.x, 
			contoursplot->plotting_rect.y+i*contoursplot->d_vminor*(contoursplot->vminor_ticks+1)+j*contoursplot->d_vminor,
			contoursplot->plotting_rect.x-contoursplot->length_ticks,
			contoursplot->plotting_rect.y+i*contoursplot->d_vminor*(contoursplot->vminor_ticks+1)+j*contoursplot->d_vminor);       
      		if (contoursplot->show_right_legends)
        	contoursplot_cairo_line(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
			contoursplot->plotting_rect.x+contoursplot->plotting_rect.width, 
			contoursplot->plotting_rect.y+i*contoursplot->d_vminor*(contoursplot->vminor_ticks+1)+j*contoursplot->d_vminor,
			contoursplot->plotting_rect.x+contoursplot->plotting_rect.width+contoursplot->length_ticks,
			contoursplot->plotting_rect.y+i*contoursplot->d_vminor*(contoursplot->vminor_ticks+1)+j*contoursplot->d_vminor);       
	}
}
/****************************************************************************************/
static void draw_vmajor_ticks_and_ylegends(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i;
	GdkRectangle rect;

	rect.x=0;
	rect.y=0;
	rect.width=widget->allocation.width;
	rect.height=widget->allocation.height;

	if (contoursplot->vmajor_ticks>1)
	for (i=0; i < contoursplot->vmajor_ticks; i++)
	{
		gint l1 = i;
		if(contoursplot->reflect_y) l1 = contoursplot->vmajor_ticks-i-1;
		if (contoursplot->show_left_legends)
        	contoursplot_cairo_line(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
			contoursplot->plotting_rect.x, 
			contoursplot->plotting_rect.y+i*contoursplot->d_vmajor,
			contoursplot->plotting_rect.x-contoursplot->length_ticks*2,
			contoursplot->plotting_rect.y+i*contoursplot->d_vmajor);
		if (contoursplot->show_right_legends)
        	contoursplot_cairo_line(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
			contoursplot->plotting_rect.x+contoursplot->plotting_rect.width, 
			contoursplot->plotting_rect.y+i*contoursplot->d_vmajor,
			contoursplot->plotting_rect.x+contoursplot->plotting_rect.width+contoursplot->length_ticks*2,
			contoursplot->plotting_rect.y+i*contoursplot->d_vmajor);
    
		if (contoursplot->y_legends_digits!=0 && contoursplot->show_left_legends)
		contoursplot_cairo_string(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
                        contoursplot->plotting_rect.x-contoursplot->y_legends_width-contoursplot->length_ticks*2-1,
			contoursplot->plotting_rect.y+contoursplot->plotting_rect.height-i*contoursplot->d_vmajor,
				contoursplot->v_legends_str[l1], FALSE, TRUE,0);
		if (contoursplot->y_legends_digits!=0 && contoursplot->show_right_legends)
		contoursplot_cairo_string(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->fore_gc,
                        contoursplot->plotting_rect.x+contoursplot->plotting_rect.width+contoursplot->length_ticks*2,
			contoursplot->plotting_rect.y+contoursplot->plotting_rect.height-i*contoursplot->d_vmajor,
				contoursplot->v_legends_str[l1], FALSE, TRUE,0);
	}
	if (contoursplot->v_label && contoursplot->y_legends_digits!=0 && contoursplot->show_left_legends)
	{
		contoursplot_cairo_layout(contoursplot, contoursplot->cairo_widget,  
                        contoursplot->plotting_rect.x-contoursplot->y_legends_width-contoursplot->length_ticks*2-contoursplot->v_label_height,
			contoursplot->plotting_rect.y+0.5*contoursplot->plotting_rect.height,
			contoursplot->v_label,FALSE,TRUE,-M_PI/2);
	}
	if (contoursplot->v_label && contoursplot->y_legends_digits!=0 && contoursplot->show_right_legends && !contoursplot->show_left_legends)
	{
		contoursplot_cairo_layout(contoursplot, contoursplot->cairo_widget,  
                        contoursplot->plotting_rect.x+contoursplot->plotting_rect.width+contoursplot->y_legends_width+contoursplot->length_ticks*2,
			contoursplot->plotting_rect.y+0.5*contoursplot->plotting_rect.height,
			contoursplot->v_label,FALSE,TRUE,-M_PI/2);
	}
}
/****************************************************************************************/
static void draw_vminor_grid(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i;
	gint j;
	if (contoursplot->vminor_grid && (contoursplot->hmajor_ticks > 1) && (contoursplot->hminor_ticks != 0) )
	for (i=0; i < contoursplot->hmajor_ticks-1; i++)
	for (j=1; j <= contoursplot->hminor_ticks; j++)
        contoursplot_cairo_line_grid(contoursplot, contoursplot->cairo_area, widget, 
			contoursplot->vminor_grid_gc,
			i*contoursplot->d_hmajor+j*contoursplot->d_hminor, 
			0,
			i*contoursplot->d_hmajor+j*contoursplot->d_hminor,
			contoursplot->plotting_rect.height);
}
/****************************************************************************************/
static void draw_vmajor_grid(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i;
	if (contoursplot->vmajor_grid && (contoursplot->hmajor_ticks > 1))
	for (i=1; i < (contoursplot->hmajor_ticks-1); i++)  
        contoursplot_cairo_line_grid(contoursplot, contoursplot->cairo_area, widget, 
			contoursplot->vmajor_grid_gc,
			i*contoursplot->d_hmajor, 
			0,
			i*contoursplot->d_hmajor,
			contoursplot->plotting_rect.height);
}
/****************************************************************************************/
static void draw_objects_text(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i;
	for (i=0; i < contoursplot->nObjectsText; i++)  
	if(contoursplot->objectsText[i].pango)
	{
		/*
    		if ( 
		!(
		(contoursplot->objectsText[i].xi > contoursplot->plotting_rect.x) && 
         	(contoursplot->objectsText[i].xi < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         	(contoursplot->objectsText[i].yi > contoursplot->plotting_rect.y) && 
         	(contoursplot->objectsText[i].yi < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) 
		)
		) continue;
		*/
		contoursplot_cairo_layout(contoursplot, contoursplot->cairo_widget,  
                        contoursplot->objectsText[i].xi,
                        contoursplot->objectsText[i].yi,
			contoursplot->objectsText[i].pango,FALSE,FALSE,-contoursplot->objectsText[i].angle);
	}
}
/****************************************************************************************/
static void draw_objects_line(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i;
	for (i=0; i < contoursplot->nObjectsLine; i++)  
	{
		/*
    		if ( 
		!(
		(contoursplot->objectsLine[i].x1i > contoursplot->plotting_rect.x) && 
         	(contoursplot->objectsLine[i].x1i < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         	(contoursplot->objectsLine[i].y1i > contoursplot->plotting_rect.y) && 
         	(contoursplot->objectsLine[i].y1i < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) 
		)
		) continue;
    		if ( 
		!(
		(contoursplot->objectsLine[i].x2i > contoursplot->plotting_rect.x) && 
         	(contoursplot->objectsLine[i].x2i < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         	(contoursplot->objectsLine[i].y2i > contoursplot->plotting_rect.y) && 
         	(contoursplot->objectsLine[i].y2i < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) 
		)
		) continue;
		*/
		/* HERE change gc vlaues */
		gdouble x1, x2, y1, y2;
		gdk_gc_set_rgb_fg_color (contoursplot->lines_gc, &contoursplot->objectsLine[i].color);
		gdk_gc_set_line_attributes (contoursplot->lines_gc, 
			contoursplot->objectsLine[i].width, 
			contoursplot->objectsLine[i].style, 
			GDK_CAP_ROUND, 
			GDK_JOIN_MITER);

        	contoursplot_cairo_line(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->lines_gc,
			contoursplot->objectsLine[i].x1i,
			contoursplot->objectsLine[i].y1i,
			contoursplot->objectsLine[i].x2i,
			contoursplot->objectsLine[i].y2i);
		if(contoursplot->objectsLine[i].arrow_size<1) continue;
		calc_arrow_vertexes(30.0, contoursplot->objectsLine[i].arrow_size*5.0,
		(gdouble)contoursplot->objectsLine[i].x1i, 
		(gdouble)contoursplot->objectsLine[i].y1i, 
		(gdouble)contoursplot->objectsLine[i].x2i, 
		(gdouble)contoursplot->objectsLine[i].y2i, 
		&x1, &y1, 
		&x2, &y2
		);
        	contoursplot_cairo_line(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->lines_gc,
			(gint)x1,
			(gint)y1,
			contoursplot->objectsLine[i].x2i,
			contoursplot->objectsLine[i].y2i);
        	contoursplot_cairo_line(contoursplot, contoursplot->cairo_widget, widget, 
			contoursplot->lines_gc,
			(gint)x2,
			(gint)y2,
			contoursplot->objectsLine[i].x2i,
			contoursplot->objectsLine[i].y2i);
	}
}
/****************************************************************************************/
static void draw_objects_image(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint i;
	for (i=0; i < contoursplot->nObjectsImage; i++)  
	if(contoursplot->objectsImage[i].image)
	{
		/*
    		if ( 
		!(
		(contoursplot->objectsImage[i].xi > contoursplot->plotting_rect.x) && 
         	(contoursplot->objectsImage[i].xi < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         	(contoursplot->objectsImage[i].yi > contoursplot->plotting_rect.y) && 
         	(contoursplot->objectsImage[i].yi < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) 
		)
		) continue;
		*/
		contoursplot_cairo_image(contoursplot, contoursplot->cairo_widget, widget, 
				contoursplot->objectsImage[i].xi , contoursplot->objectsImage[i].yi, 
				contoursplot->objectsImage[i].widthi, contoursplot->objectsImage[i].heighti, 
				contoursplot->objectsImage[i].image);
	}
}
/****************************************************************************************/
static void clean_borders(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	if(contoursplot->cairo_export)
	{
		contoursplot_cairo_rectangle(contoursplot, contoursplot->cairo_export,  widget, contoursplot->back_gc, 
			TRUE,
			0,
			0,
			contoursplot->plotting_rect.x,
			widget->allocation.height);
		contoursplot_cairo_rectangle(contoursplot, contoursplot->cairo_export,  widget, contoursplot->back_gc, 
			TRUE,
			0,
			0,
			widget->allocation.width,
			contoursplot->plotting_rect.y
			);
		contoursplot_cairo_rectangle(contoursplot, contoursplot->cairo_export,  widget, contoursplot->back_gc, 
			TRUE,
			0,
			contoursplot->plotting_rect.y+contoursplot->plotting_rect.height,
			widget->allocation.width,
			widget->allocation.height-contoursplot->plotting_rect.y-contoursplot->plotting_rect.height
			);
		contoursplot_cairo_rectangle(contoursplot, contoursplot->cairo_export,  widget, contoursplot->back_gc, 
			TRUE,
			contoursplot->plotting_rect.x+contoursplot->plotting_rect.width,
			0,
			widget->allocation.width-contoursplot->plotting_rect.x-contoursplot->plotting_rect.width,
			widget->allocation.height
			);
	}
}
/****************************************************************************************/
static void draw_borders(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	contoursplot_cairo_rectangle(contoursplot, contoursplot->cairo_widget,  widget,
			contoursplot->fore_gc,
			FALSE,
			contoursplot->plotting_rect.x,
			contoursplot->plotting_rect.y,
			contoursplot->plotting_rect.width,
			contoursplot->plotting_rect.height);
}
/****************************************************************************************/
static void draw_background(GtkWidget *widget, GabeditContoursPlot *contoursplot)
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
	contoursplot_cairo_rectangle(contoursplot, contoursplot->cairo_area,  widget, contoursplot->back_gc, 
			TRUE,
			0,
			0,
			contoursplot->plotting_rect.width,
			contoursplot->plotting_rect.height);
	if(contoursplot->cairo_export)
	{
	contoursplot_cairo_rectangle(contoursplot, contoursplot->cairo_export,  widget, contoursplot->back_gc, 
			TRUE,
			contoursplot->plotting_rect.x,
			contoursplot->plotting_rect.y,
			contoursplot->plotting_rect.width,
			contoursplot->plotting_rect.height);
	}

}
/****************************************************************************************/
static void draw_label_contours(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gint n;
	GList *current_node;
	gint x,y;
	PangoLayout* pango = NULL;
	ContoursPlotData *data = NULL;
	gint width;
	gint height;

	current_node=g_list_first(GABEDIT_ContoursPLOT(contoursplot)->data_list);
	for (; current_node!=NULL; current_node=current_node->next)
	{
		data=(ContoursPlotData*)current_node->data;  
		for(n=0;n<data->nContours; n++) 
		{
	  		gchar *str = NULL;

			if(!data->contours[n].label) continue;
			if(strlen(data->contours[n].label)<1) continue;
	  		str = g_strconcat("<span>", data->contours[n].label, "</span>", NULL);
        		value2pixel(contoursplot, data->contours[n].xlabel, data->contours[n].ylabel, &x, &y);
        		y=contoursplot->plotting_rect.height-y;                   
			pango = gtk_widget_create_pango_layout (GTK_WIDGET(contoursplot), data->contours[n].label);
			pango_layout_set_alignment(pango,PANGO_ALIGN_LEFT);
	  		pango_layout_set_markup(pango, str, -1);
	  		g_free(str);
			pango_layout_get_size(pango, &width, &height);
		        width/=PANGO_SCALE;
			height/=PANGO_SCALE;
			y -= height/2;

			contoursplot_cairo_layout(
			contoursplot, 
			contoursplot->cairo_area,  
                        x,
			y,
			pango,FALSE,FALSE,0);
			g_object_unref(G_OBJECT(pango));
		}
	}
}
/**************************************************************/
/*
static void draw_plan_color_coded(GtkWidget* contoursplot, ContoursPlotData *data)
{
	gint i;
	gint j;
	gint k;
	gdouble v1,v2,v3,v4;
	gint ix1=0,iy1=0;
	gint ix2=0,iy2=0;
	gdouble x1,x2,y1,y2;
	gdouble dx, dy;
	gdouble xmin,xmax,ymin,ymax;
	gint xsize,ysize;
	gdouble* zValues;
	gdouble c1[3];
	gdouble c2[3];
	gdouble c3[3];
	gdouble c4[3];
	gint x1p;
	gint y1p;
	gint x2p;
	gint y2p;

	xmin = data->xmin;
	xmax = data->xmax;
	ymin = data->ymin;
	ymax = data->ymax;
	xsize = data->xsize;
	ysize = data->ysize;
	dx = (xmax-xmin)/(xsize-1);
	dy = (ymax-ymin)/(ysize-1);
	zValues = data->zValues;

	for( i = 0; i<xsize-1;i++)
	{
		for( j = 0; j<ysize-1;j++)
		{
			ix1 = i;
			iy1 = j;
			ix2 = i+1;
			iy2 = j+1;
        
			v1 = zValues[ix1*ysize+iy1],
			v2 = zValues[ix1*ysize+iy2];
			v3 = zValues[ix2*ysize+iy2];
			v4 = zValues[ix2*ysize+iy1];
			for(k=0;k<3;k++) c1[k] = rand()/(gdouble)RAND_MAX;
			for(k=0;k<3;k++) c2[k] = rand()/(gdouble)RAND_MAX;
			for(k=0;k<3;k++) c3[k] = rand()/(gdouble)RAND_MAX;
			for(k=0;k<3;k++) c4[k] = rand()/(gdouble)RAND_MAX;

			if(GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap)
			{
				set_Color_From_colorMap(GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap, c1, v1);
				set_Color_From_colorMap(GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap, c2, v2);
				set_Color_From_colorMap(GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap, c3, v3);
				set_Color_From_colorMap(GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap, c4, v4);
			}
			x1 = xmin+dx*i;
			y1 = ymin+dy*j;
			x2 = xmin+dx*(i+1);
			y2 = ymin+dy*(j+1);
			value2pixel(GABEDIT_ContoursPLOT(contoursplot), x1, y1, &x1p, &y1p);
			value2pixel(GABEDIT_ContoursPLOT(contoursplot), x2, y2, &x2p, &y2p);
			draw_rectangle_bilinear(GABEDIT_ContoursPLOT(contoursplot), x1p, y1p, x2p, y2p, c1, c2, c3, c4);
		}
	}
}
*/
/****************************************************************************************/
static void draw_data(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	ContoursPlotData *data = NULL;
	GList *current = NULL;

	if(contoursplot->data_list)
	for(current=g_list_first(contoursplot->data_list); current!=NULL; current=current->next)
	{
		data=(ContoursPlotData*)current->data;
		/* draw_plan_color_coded(widget, data);*/
		draw_lines(widget, contoursplot, data);
		draw_points(widget, contoursplot, data);
		if(contoursplot->show_label_contours) 
			draw_label_contours(widget, contoursplot);
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
static void set_old_area(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	if (contoursplot->old_area!=NULL) g_object_unref(G_OBJECT(contoursplot->old_area));
	contoursplot->old_area=NULL;

	if (GTK_WIDGET_REALIZED(widget) &&  widget->window) 
	{
		contoursplot->old_area=gdk_pixmap_copy(widget->window);
	}
}
/****************************************************************************************/
static void draw_plotting_area(GtkWidget *widget, GabeditContoursPlot *contoursplot)
{
	gdk_draw_drawable (widget->window, 
			contoursplot->back_gc, 
			contoursplot->plotting_area, 
			0, 
			0, 
			contoursplot->plotting_rect.x, 
			contoursplot->plotting_rect.y, 
			contoursplot->plotting_rect.width, 
			contoursplot->plotting_rect.height);

}
/****************************************************************************************/
static gint gabedit_contoursplot_draw (GtkWidget *widget)
{
	GabeditContoursPlot *contoursplot;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GTK_IS_WIDGET(widget), FALSE);
	if(!widget->window) return FALSE;
	if(!GDK_IS_DRAWABLE (widget->window)) return FALSE;
	g_return_val_if_fail (GABEDIT_IS_ContoursPLOT (widget), FALSE);

	contoursplot=GABEDIT_ContoursPLOT(widget);
	if(!contoursplot->h_legends_str) return FALSE;

	if (contoursplot->cairo_widget)
	{
    		cairo_destroy (contoursplot->cairo_widget);
    		contoursplot->cairo_widget = gdk_cairo_create (widget->window);
	}
	if (contoursplot->cairo_area)
	{
    		cairo_destroy (contoursplot->cairo_area);
    		contoursplot->cairo_area = gdk_cairo_create (contoursplot->plotting_area);
	}

	draw_background(widget, contoursplot);
	draw_hminor_grid(widget, contoursplot);
	draw_hmajor_grid(widget, contoursplot);
	draw_vminor_grid(widget, contoursplot);
	draw_vmajor_grid(widget, contoursplot);

	draw_data(widget, contoursplot);

	clean_borders(widget, contoursplot);/* for export */
	draw_borders(widget, contoursplot);

	draw_hminor_ticks(widget, contoursplot);
	draw_hmajor_ticks_and_xlegends(widget, contoursplot);
	draw_vminor_ticks(widget, contoursplot);
	draw_vmajor_ticks_and_ylegends(widget, contoursplot);

	draw_plotting_area(widget, contoursplot);
 
	draw_colormap(widget, contoursplot);
	draw_objects_image(widget, contoursplot);
	draw_objects_line(widget, contoursplot);
	draw_objects_text(widget, contoursplot);
  	set_old_area(widget, contoursplot);

	return 0;
}
/****************************************************************************************/
static gint gabedit_contoursplot_expose (GtkWidget *widget, GdkEventExpose *event)
{
	GabeditContoursPlot *contoursplot;

	g_return_val_if_fail (widget != NULL, FALSE);
	g_return_val_if_fail (GABEDIT_IS_ContoursPLOT (widget), FALSE);
	/* g_return_val_if_fail (event != NULL, FALSE);*/

	if (event && event->count > 0) return FALSE;
	contoursplot=GABEDIT_ContoursPLOT(widget);

	if (contoursplot->mouse_zoom_enabled && contoursplot->mouse_button == contoursplot->mouse_zoom_button)
	{
		gint width;
		gint height;
		gdk_drawable_get_size(contoursplot->old_area, &width, &height);
		gdk_draw_drawable (widget->window, 
		contoursplot->back_gc, 
		contoursplot->old_area, 
		0, 
		0, 
		0, 
		0, 
		width, 
		height
		);
		draw_zoom_rectangle(widget, contoursplot);
		return TRUE;
	}
	if (contoursplot->mouse_distance_enabled && contoursplot->mouse_button == contoursplot->mouse_distance_button)
	{
		gint width;
		gint height;
		gdk_drawable_get_size(contoursplot->old_area, &width, &height);
		gdk_draw_drawable (widget->window, 
		contoursplot->back_gc, 
		contoursplot->old_area, 
		0, 
		0, 
		0, 
		0, 
		width, 
		height
		);
		draw_distance_line(widget, contoursplot);
		return TRUE;
	}
	if (contoursplot->selected_objects_text_num>-1)
	{
		gint width;
		gint height;
		gdk_drawable_get_size(contoursplot->old_area, &width, &height);
		gdk_draw_drawable (widget->window, 
		contoursplot->back_gc, 
		contoursplot->old_area, 
		0, 
		0, 
		0, 
		0, 
		width, 
		height
		);
		draw_selected_objects_text_rectangle(widget, contoursplot);
		return TRUE;
	}
	if (contoursplot->selected_objects_image_num>-1)
	{
		gint width;
		gint height;
		gdk_drawable_get_size(contoursplot->old_area, &width, &height);
		gdk_draw_drawable (widget->window, 
		contoursplot->back_gc, 
		contoursplot->old_area, 
		0, 
		0, 
		0, 
		0, 
		width, 
		height
		);
		draw_selected_objects_image_rectangle(widget, contoursplot);
		return TRUE;
	}
	if (contoursplot->object_begin_point.x>-1)
	{
		gint width;
		gint height;
		gdk_drawable_get_size(contoursplot->old_area, &width, &height);
		gdk_draw_drawable (widget->window, 
		contoursplot->back_gc, 
		contoursplot->old_area, 
		0, 
		0, 
		0, 
		0, 
		width, 
		height
		);
		draw_object_line_gdk(widget, contoursplot);
		return TRUE;
	}
	gabedit_contoursplot_draw (widget);
	return 0;
}
/****************************************************************************************/
static gint get_distance_M_AB(GabeditContoursPlot *contoursplot,gint xM, gint yM, gint ixA, gint iyA, gint ixB, gint iyB)
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
static gint gabedit_contoursplot_double_click (GtkWidget *widget, GdkEventButton *event)
{
  	ContoursPlotData *data = NULL; 
  	ContourData *contour = NULL; 
	GList* current = NULL;
	gint x;
	gint y;
	gboolean OK = FALSE;
	GabeditContoursPlot *contoursplot = NULL;
	gint loop;
	gint xOld;
	gint yOld;
	gint n;
	gint indexOld = -1;
	gint index;
	gint i;

  	contoursplot=GABEDIT_ContoursPLOT(widget);
	/* test on objects label */
	i = get_object_text_num(contoursplot, event->x, event->y);
	if(i>-1 && i<contoursplot->nObjectsText)
	{
		contoursplot->selected_objects_text_num = -1;
		add_set_object_text_dialog(GTK_WIDGET(contoursplot), i, 0,0);
		return TRUE;
	}
	/* test on objects line */
	i = get_object_line_num(contoursplot, event->x, event->y);
	if(i>-1 && i<contoursplot->nObjectsLine)
	{
		contoursplot->selected_objects_line_num = -1;
		contoursplot->selected_objects_line_type = -1;
		contoursplot->object_begin_point.x = -1;
		contoursplot->object_begin_point.y = -1;
		contoursplot->object_end_point.x = -1;
		contoursplot->object_end_point.y = -1;
		set_object_line_dialog(contoursplot, i);
		return TRUE;
	}
	/* test on objects image */
	i = get_object_image_num(contoursplot, event->x, event->y);
	if(i>-1 && i<contoursplot->nObjectsImage)
	{
		contoursplot->selected_objects_image_num = -1;
		contoursplot->object_begin_point.x = -1;
		contoursplot->object_begin_point.y = -1;
		contoursplot->object_end_point.x = -1;
		contoursplot->object_end_point.y = -1;
		add_set_object_image_dialog(GTK_WIDGET(contoursplot), i, 0,0);
		return TRUE;
	}
    	if ( 
		!(
		(event->x > contoursplot->plotting_rect.x) && 
         	(event->x < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         	(event->y > contoursplot->plotting_rect.y) && 
         	(event->y < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) 
	)
	) 
	{
		set_all_dialog(widget);
		return TRUE;
	}

	if(!(contoursplot->data_list)) return TRUE;
	current=g_list_first(contoursplot->data_list);
	for(; current != NULL; current = current->next)
	{
		data = (ContoursPlotData*)current->data;
		xOld = 0;
		yOld = 0;
		for (n=0; !OK && n<data->nContours; n++)
		for(loop=0;loop<data->contours[n].size;loop++)
		{
			contour = &data->contours[n];

			index = data->contours[n].index[loop];
    			value2pixel(contoursplot, data->contours[n].x[loop], data->contours[n].y[loop], &x, &y);
			y=contoursplot->plotting_rect.height-y;
			x += contoursplot->plotting_rect.x;
			y += contoursplot->plotting_rect.y;
			if(loop ==0 && abs(x-event->x)<5 && abs(y-event->y)<5)
			{
				OK = TRUE;
				break;
			}
			if(loop >0 && indexOld==index && get_distance_M_AB(contoursplot,(gint)event->x, (gint)event->y,  (gint)xOld, (gint)yOld, (gint)x, (gint)y)<5)
			{
				OK = TRUE;
				break;
			}
			xOld = x;
			yOld = y;
			indexOld = index;
		}
		if(OK) break;
	}
	if(OK)
	{
		set_data_dialog(contoursplot,data,contour);
	}
	return OK;
}
/*********************************************************************************************************************/
static gboolean popuo_menu(GtkWidget* widget, guint button, guint32 time)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (widget), "Manager");
	GtkWidget *menu = gtk_ui_manager_get_widget (manager, "/MenuContoursPlot");
	if (GTK_IS_MENU (menu)) 
	{
		gtk_menu_popup (GTK_MENU (menu), NULL, NULL, NULL, NULL, button, time);
		return TRUE;
	}
	else 
	{
		g_message (_("popup menu of contoursplot failed"));
	}
	return FALSE;
}
/****************************************************************************************/
static gint gabedit_contoursplot_button_press (GtkWidget *widget, GdkEventButton *event)
{
  GabeditContoursPlot *contoursplot;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GABEDIT_IS_ContoursPLOT (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  contoursplot = GABEDIT_ContoursPLOT (widget);
  if(event->type == GDK_2BUTTON_PRESS)
  {
	  contoursplot->double_click = TRUE;
	  return gabedit_contoursplot_double_click(widget,event);
  }
  if(event->button == 3)
  {
	  return popuo_menu(widget, event->button, event->time);
  }

  if (contoursplot->mouse_zoom_enabled && (event->button == contoursplot->mouse_zoom_button || (event->button == 1 && contoursplot->control_key_pressed )) )
    if ( (event->x > contoursplot->plotting_rect.x) && 
         (event->x < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         (event->y > contoursplot->plotting_rect.y) && 
         (event->y < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) ){
      contoursplot->mouse_button=contoursplot->mouse_zoom_button;
    
      contoursplot->zoom_point.x=event->x;
      contoursplot->zoom_point.y=event->y;
	/*set_old_area(widget, contoursplot);*/
  }
  if (contoursplot->mouse_distance_enabled && (event->button == contoursplot->mouse_distance_button && contoursplot->shift_key_pressed ) )
    if ( (event->x > contoursplot->plotting_rect.x) && 
         (event->x < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         (event->y > contoursplot->plotting_rect.y) && 
         (event->y < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) ){
      contoursplot->mouse_button=contoursplot->mouse_distance_button;
    
      contoursplot->distance_point.x=event->x;
      contoursplot->distance_point.y=event->y;
	/*set_old_area(widget, contoursplot);*/
  }

  if (contoursplot->mouse_displace_enabled && (event->button == contoursplot->mouse_displace_button))
    if ( (event->x > contoursplot->plotting_rect.x) && 
         (event->x < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         (event->y > contoursplot->plotting_rect.y) && 
         (event->y < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) ){
      contoursplot->mouse_button=event->button;
    
      contoursplot->move_point.x=event->x;
      contoursplot->move_point.y=event->y;
  } 
  if (contoursplot->r_key_pressed && !contoursplot->control_key_pressed){
	  	gint i;
		contoursplot->r_key_pressed = FALSE;
		i = get_object_text_num(contoursplot, event->x, event->y);
		if(i>-1) delete_object_text(GTK_WIDGET(contoursplot), i);
		else
		{
			i = get_object_line_num(contoursplot, event->x, event->y);
			if(i>-1) delete_object_line(GTK_WIDGET(contoursplot), i);
			else
			{
				i = get_object_image_num(contoursplot, event->x, event->y);
				if(i>-1) delete_object_image(GTK_WIDGET(contoursplot), i);
			}
		}
	}
  if (
	!contoursplot->t_key_pressed && 
	!contoursplot->l_key_pressed && 
	!contoursplot->i_key_pressed && 
	!contoursplot->shift_key_pressed && 
	!contoursplot->control_key_pressed && 
	event->button != contoursplot->mouse_zoom_button)
	  /*
    if ( (event->x > contoursplot->plotting_rect.x) && 
         (event->x < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         (event->y > contoursplot->plotting_rect.y) && 
         (event->y < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) )
	 */
    {

	gdouble X, Y;
	gchar txt[BSIZE];
	PangoLayout *playout;
	gboolean gp = gabedit_contoursplot_get_point(GABEDIT_ContoursPLOT(contoursplot), event->x, event->y, &X, &Y);
	gint i;

	sprintf(txt,"%s"," ");
	if(gp && !contoursplot->d_key_pressed) sprintf(txt,"(%f ; %f)",X,Y);
	else if(gp && contoursplot->d_key_pressed && 
             	(event->x > contoursplot->plotting_rect.x) && 
         	(event->x < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         	(event->y > contoursplot->plotting_rect.y) && 
         	(event->y < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) )
	{
		gdouble z = 0;
		gdouble dzdx = 0;
		gdouble dzdy = 0;
		gdouble d2zdx2 = 0;
		gdouble d2zdy2 = 0;
		gdouble d2zdxdy = 0;
		gdouble evals[2] = {0,0};
		gdouble evecs[2][2] = {{0,0},{0,0}};
		gdouble delta;
		gdouble d = 0;
		gdouble precision = 1e-10;


		get_interpolatedValue(GTK_WIDGET(contoursplot), NULL, X, Y, &z, &dzdx, &dzdy,
				&d2zdx2, &d2zdy2, &d2zdxdy);

		delta = (d2zdx2-d2zdy2)*(d2zdx2-d2zdy2)+4*d2zdxdy*d2zdxdy;
		delta = sqrt(delta);
		evals[0] = ((d2zdx2+d2zdy2)+delta)/2;
		evals[1] = ((d2zdx2+d2zdy2)-delta)/2;

		evecs[0][0] = 1.0;
		evecs[0][1] = 0.0;
		evecs[1][0] = 0.0;
		evecs[1][1] = 1.0;
		d = sqrt(d2zdxdy*d2zdxdy+(d2zdx2-evals[0])*(d2zdx2-evals[0]));
		if(d>1e-10)
		{
			evecs[0][0] = d2zdxdy/d;
			evecs[0][1] = (d2zdx2-evals[0])/d;
		}
		d = sqrt(d2zdxdy*d2zdxdy+(d2zdx2-evals[1])*(d2zdx2-evals[1]));
		if(d>precision && fabs(evecs[0][1])>precision)
		{
			evecs[1][0] = d2zdxdy/d;
			evecs[1][1] = -evecs[0][0]*evecs[1][0]/evecs[0][1];
		}
		else if(d>precision && fabs(evecs[0][0])>precision)
		{
			evecs[1][1] = (d2zdx2-evals[1])/d;
			evecs[1][0] = -evecs[0][1]*evecs[1][1]/evecs[0][0];
		}
		sprintf(txt,
				"(x=%f ; y=%f ; z=%f)\n(dz/dx=%f ; dz/dy=%f)\n(d2z/dx2=%f)\n(d2z/dy2=%f)\n(d2z/dxdy=%f)"
				"\n(EigenHess=%f : %f %f )"
				"\n(EigenHess=%f : %f %f )",
				X,Y,z,dzdx, dzdy,
				d2zdx2, d2zdy2, d2zdxdy,
				evals[0], evecs[0][0], evecs[0][1],
				evals[1], evecs[1][0], evecs[1][1]
				);
		if(evals[0]>=0)
		gdk_draw_line(widget->window,
			contoursplot->fore_gc,
			event->x-(gint)(evecs[0][0]*contoursplot->plotting_rect.width/10),
			event->y-(gint)(evecs[0][1]*contoursplot->plotting_rect.height/10),
			event->x+(gint)(evecs[0][0]*contoursplot->plotting_rect.width/10),
			event->y+(gint)(evecs[0][1]*contoursplot->plotting_rect.height/10));

		if(evals[1]>=0)
		gdk_draw_line(widget->window,
			contoursplot->fore_gc,
			event->x-(gint)(evecs[1][0]*contoursplot->plotting_rect.width/10),
			event->y-(gint)(evecs[1][1]*contoursplot->plotting_rect.height/10),
			event->x+(gint)(evecs[1][0]*contoursplot->plotting_rect.width/10),
			event->y+(gint)(evecs[1][1]*contoursplot->plotting_rect.height/10));
		if(evals[0]<0)
		{
  			GdkGC* gc;
  			GdkGCValues gc_values;
  			GdkGCValuesMask gc_values_mask;
  			GdkColor col;
			GdkColormap *colormap;

			col.red = 65000;
  			col.green = 0;
			col.blue = 0;

			gc_values.foreground=col;
			gc_values.line_style=GDK_LINE_SOLID;
			gc_values.line_width=2;
			gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
   			colormap  = gdk_window_get_colormap(widget->window);
			gdk_colormap_alloc_color (colormap, &col, FALSE, TRUE);
			gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
			 gdk_gc_set_foreground(gc,&col);
			gdk_draw_line(widget->window,gc,
			event->x-(gint)(evecs[0][0]*contoursplot->plotting_rect.width/10),
			event->y-(gint)(evecs[0][1]*contoursplot->plotting_rect.height/10),
			event->x+(gint)(evecs[0][0]*contoursplot->plotting_rect.width/10),
			event->y+(gint)(evecs[0][1]*contoursplot->plotting_rect.height/10));
			g_object_unref(gc);
		}
		if(evals[1]<0)
		{
  			GdkGC* gc;
  			GdkGCValues gc_values;
  			GdkGCValuesMask gc_values_mask;
  			GdkColor col;
			GdkColormap *colormap;

			col.red = 65000;
  			col.green = 0;
			col.blue = 0;

			gc_values.foreground=col;
			gc_values.line_style=GDK_LINE_SOLID;
			gc_values.line_width=2;
			gc_values_mask=GDK_GC_FOREGROUND | GDK_GC_LINE_STYLE | GDK_GC_LINE_WIDTH;
   			colormap  = gdk_window_get_colormap(widget->window);
			gdk_colormap_alloc_color (colormap, &col, FALSE, TRUE);
			gc=gdk_gc_new_with_values (widget->window, &gc_values, gc_values_mask);
			 gdk_gc_set_foreground(gc,&col);
			gdk_draw_line(widget->window,gc,
			event->x-(gint)(evecs[1][0]*contoursplot->plotting_rect.width/10),
			event->y-(gint)(evecs[1][1]*contoursplot->plotting_rect.height/10),
			event->x+(gint)(evecs[1][0]*contoursplot->plotting_rect.width/10),
			event->y+(gint)(evecs[1][1]*contoursplot->plotting_rect.height/10));
			g_object_unref(gc);
		}
			
	}
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
	i = get_object_text_num(contoursplot, event->x, event->y);
	if(i>-1 && i<contoursplot->nObjectsText) contoursplot->selected_objects_text_num = i;
	else contoursplot->selected_objects_text_num = -1;
	if(contoursplot->selected_objects_text_num>-1) 
	{
		/*set_old_area(widget, contoursplot);*/
    		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
	if(contoursplot->selected_objects_text_num<0)
	{
		i = get_object_line_num(contoursplot, event->x, event->y);
		contoursplot->selected_objects_line_num = -1;
		contoursplot->selected_objects_line_type = -1;
		if(i>-1 && i<contoursplot->nObjectsLine) 
		{
			gdouble xx,yy,d;
			contoursplot->selected_objects_line_num = i;
			contoursplot->selected_objects_line_type = 0;
      			contoursplot->object_begin_point.x=contoursplot->objectsLine[i].x1i;
      			contoursplot->object_begin_point.y=contoursplot->objectsLine[i].y1i;
      			contoursplot->object_end_point.x=contoursplot->objectsLine[i].x2i;
      			contoursplot->object_end_point.y=contoursplot->objectsLine[i].y2i;
			xx = event->x-contoursplot->objectsLine[i].x1i;
			yy = event->y-contoursplot->objectsLine[i].y1i;
			d = xx*xx+yy*yy;
			xx = event->x-contoursplot->objectsLine[i].x2i;
			yy = event->y-contoursplot->objectsLine[i].y2i;
			if(d<xx*xx+yy*yy) contoursplot->selected_objects_line_type = 1;
			/*set_old_area(widget, contoursplot);*/
    			gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
		}
		
	}
	if(contoursplot->selected_objects_text_num<0 && contoursplot->selected_objects_line_num<0)
	{
		i = get_object_image_num(contoursplot, event->x, event->y);
		if(i>-1 && i<contoursplot->nObjectsImage) contoursplot->selected_objects_image_num = i;
		else contoursplot->selected_objects_image_num = -1;
		if(contoursplot->selected_objects_image_num>-1) 
		{
			gdouble xx = -event->x+contoursplot->objectsImage[i].xi+contoursplot->objectsImage[i].widthi;
			gdouble yy = -event->y+contoursplot->objectsImage[i].yi+contoursplot->objectsImage[i].heighti;
			if(xx*xx+yy*yy<
			(contoursplot->objectsImage[i].widthi*contoursplot->objectsImage[i].widthi+
			contoursplot->objectsImage[i].heighti*contoursplot->objectsImage[i].heighti)/100
			)
			{
      				contoursplot->object_begin_point.x=-1;
      				contoursplot->object_begin_point.y=-1;
      				contoursplot->object_end_point.x=(gint)fabs(xx);
      				contoursplot->object_end_point.y=(gint)fabs(yy);
			}
			else
			{
      				contoursplot->object_begin_point.x=event->x-contoursplot->objectsImage[i].xi;
      				contoursplot->object_begin_point.y=event->y-contoursplot->objectsImage[i].yi;
      				contoursplot->object_end_point.x=-1;
      				contoursplot->object_end_point.y=-1;
			}
			set_old_area(widget, contoursplot);
    			gtk_widget_queue_draw(GTK_WIDGET(contoursplot));  
		}
	}

  } 
  if (contoursplot->t_key_pressed && !contoursplot->control_key_pressed)
	  /*
    if ( (event->x > contoursplot->plotting_rect.x) && 
         (event->x < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         (event->y > contoursplot->plotting_rect.y) && 
         (event->y < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) )
	 */
	 {

	gdouble X, Y;

	if(gabedit_contoursplot_get_point(GABEDIT_ContoursPLOT(contoursplot), event->x, event->y, &X, &Y))
	{
		contoursplot->t_key_pressed = FALSE;
		add_set_object_text_dialog(GTK_WIDGET(contoursplot), -1, X, Y);
	}

  } 
  if (contoursplot->l_key_pressed && !contoursplot->control_key_pressed)
	  /*
    if ( (event->x > contoursplot->plotting_rect.x) && 
         (event->x < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         (event->y > contoursplot->plotting_rect.y) && 
         (event->y < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) )
	 */
    {

	/*set_old_area(widget, contoursplot);*/
	contoursplot->l_key_pressed = FALSE;
      	contoursplot->object_begin_point.x=event->x;
      	contoursplot->object_begin_point.y=event->y;
      	contoursplot->object_end_point.x=-1;
      	contoursplot->object_end_point.y=-1;

  } 
  if (contoursplot->i_key_pressed && !contoursplot->control_key_pressed)
	  /*
    if ( (event->x > contoursplot->plotting_rect.x) && 
         (event->x < (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) && 
         (event->y > contoursplot->plotting_rect.y) && 
         (event->y < (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height)) )
	 */
    {
		contoursplot->i_key_pressed = FALSE;
		add_set_object_image_dialog(GTK_WIDGET(contoursplot), -1, event->x, event->y);
	}


  if ( contoursplot->mouse_autorange_enabled && 
      (event->button == contoursplot->mouse_autorange_button) )
    gabedit_contoursplot_set_autorange(contoursplot, NULL);

   contoursplot->double_click = FALSE;
  
  return FALSE;
}
/****************************************************************************************/
static gint gabedit_contoursplot_button_release (GtkWidget *widget, GdkEventButton *event)
{
  GabeditContoursPlot *contoursplot;
  gdouble xmin, xmax, ymin, ymax;
  gint xleft, ytop, xright, ybottom; 
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GABEDIT_IS_ContoursPLOT (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  contoursplot = GABEDIT_ContoursPLOT (widget);

  contoursplot->selected_objects_text_num=-1;
   if (contoursplot->selected_objects_image_num>-1)
   {
	contoursplot->object_begin_point.x=-1;
	contoursplot->object_begin_point.y=-1;
	contoursplot->object_end_point.x=-1;
	contoursplot->object_end_point.y=-1;
  	contoursplot->selected_objects_image_num=-1;
   }
   if (contoursplot->selected_objects_line_num>-1)
   {
	contoursplot->object_begin_point.x=-1;
	contoursplot->object_begin_point.y=-1;
	contoursplot->object_end_point.x=-1;
	contoursplot->object_end_point.y=-1;
  	contoursplot->selected_objects_line_num=-1;
  	contoursplot->selected_objects_line_type=-1;
   }

/* Zoom */
   if ( !contoursplot->double_click && contoursplot->mouse_zoom_enabled && 
       (contoursplot->mouse_button == contoursplot->mouse_zoom_button)){

    xleft=contoursplot->zoom_rect.x-contoursplot->plotting_rect.x;
    xright=contoursplot->zoom_rect.x+contoursplot->zoom_rect.width-contoursplot->plotting_rect.x;
    ybottom=contoursplot->plotting_rect.y+contoursplot->plotting_rect.height-contoursplot->zoom_rect.y-contoursplot->zoom_rect.height;
    ytop=contoursplot->plotting_rect.y+contoursplot->plotting_rect.height-contoursplot->zoom_rect.y;
        
    pixel2value(contoursplot, xleft, ybottom, &xmin, &ymin);
    pixel2value(contoursplot, xright, ytop, &xmax, &ymax);

    if(contoursplot->reflect_x)
    {
	    gdouble a = xmin;
	    xmin = xmax;
	    xmax = a;
    }
    if(contoursplot->reflect_y)
    {
	    gdouble a = ymin;
	    ymin = ymax;
	    ymax = a;
    }
  
    gabedit_contoursplot_set_range(contoursplot, xmin, xmax, ymin, ymax);
    
    contoursplot->mouse_button=0;
  }
/* draw object line */
   if ( !contoursplot->double_click && contoursplot->object_begin_point.x>-1) {

	gdouble X1, Y1;
	gdouble X2, Y2;
	if(gabedit_contoursplot_get_point(GABEDIT_ContoursPLOT(contoursplot), event->x, event->y, &X2, &Y2)
	&& gabedit_contoursplot_get_point(GABEDIT_ContoursPLOT(contoursplot), contoursplot->object_begin_point.x, contoursplot->object_begin_point.y, &X1, &Y1))
	{
		contoursplot->l_key_pressed = FALSE;
		add_object_line(contoursplot, X1, Y1, X2, Y2);
	}
	contoursplot->object_begin_point.x = -1;
   }
/* distance */
   if ( !contoursplot->double_click && contoursplot->mouse_distance_enabled && 
       (contoursplot->mouse_button == contoursplot->mouse_distance_button)){

    xleft=contoursplot->distance_rect.x-contoursplot->plotting_rect.x;
    xright=contoursplot->distance_rect.x+contoursplot->distance_rect.width-contoursplot->plotting_rect.x;
    ybottom=contoursplot->plotting_rect.y+contoursplot->plotting_rect.height-contoursplot->distance_rect.y-contoursplot->distance_rect.height;
    ytop=contoursplot->plotting_rect.y+contoursplot->plotting_rect.height-contoursplot->distance_rect.y;
        
    pixel2value(contoursplot, xleft, ybottom, &xmin, &ymin);
    pixel2value(contoursplot, xright, ytop, &xmax, &ymax);

    if(contoursplot->reflect_x)
    {
	    gdouble a = xmin;
	    xmin = xmax;
	    xmax = a;
    }
    if(contoursplot->reflect_y)
    {
	    gdouble a = ymin;
	    ymin = ymax;
	    ymax = a;
    }
    {
	GtkWidget* dialog = NULL;
	gchar* tmp = NULL;
	tmp = g_strdup_printf("dX = %f ; dY = %f",xmax-xmin,ymax-ymin);
	dialog = gtk_message_dialog_new_with_markup (NULL,
		           GTK_DIALOG_DESTROY_WITH_PARENT,
		           GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
			   "%s",
			   tmp);
       gtk_dialog_run (GTK_DIALOG (dialog));
       gtk_widget_destroy (dialog);
       g_free(tmp);
    }
  
    /* gabedit_contoursplot_set_range(contoursplot, xmin, xmax, ymin, ymax);*/
    
    gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
    contoursplot->mouse_button=0;
  }
   else
    gtk_widget_queue_draw(GTK_WIDGET(contoursplot));

  if (contoursplot->mouse_displace_enabled && 
      (event->button == contoursplot->mouse_displace_button) && 
      (contoursplot->mouse_button == contoursplot->mouse_displace_button) ){
    contoursplot->mouse_button=0;
  }
    
  return FALSE;
}
/****************************************************************************************/
static gint gabedit_contoursplot_motion_notify (GtkWidget *widget, GdkEventMotion *event)
{
  GabeditContoursPlot *contoursplot;
  gint x, y;
  gdouble px, py, mx, my;
  
  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GABEDIT_IS_ContoursPLOT (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  contoursplot = GABEDIT_ContoursPLOT (widget);

  x = event->x;
  y = event->y;
  
  if (event->is_hint || (event->window != widget->window))
      gdk_window_get_pointer (widget->window, (gint *)&x, (gint *)&y, NULL);

  if (contoursplot->selected_objects_text_num>-1) 
  {
	gint i = contoursplot->selected_objects_text_num;
	ContoursPlotObjectText* objectText = &GABEDIT_ContoursPLOT(contoursplot)->objectsText[i];
	gabedit_contoursplot_get_point_control (contoursplot, x, y, objectText->width, objectText->height, objectText->angle, &mx, &my);
	set_object_text(contoursplot, objectText,  mx,my, objectText->angle, objectText->str);
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
  }
  else if (contoursplot->selected_objects_line_num>-1) 
  {
	gint i = contoursplot->selected_objects_line_num;
	gdouble x1=0, y1=0, x2=0, y2=0;
	ContoursPlotObjectLine* objectLine = &GABEDIT_ContoursPLOT(contoursplot)->objectsLine[i];
	if(contoursplot->selected_objects_line_type==0)
	{
      		contoursplot->object_end_point.x=x;
      		contoursplot->object_end_point.y=y;
		gabedit_contoursplot_get_point_control (contoursplot, contoursplot->object_begin_point.x, contoursplot->object_begin_point.y,  objectLine->width,objectLine->width, 0, &x1, &y1);
		gabedit_contoursplot_get_point_control (contoursplot, contoursplot->object_end_point.x, contoursplot->object_end_point.y,  objectLine->width,objectLine->width, 0, &x2, &y2);
	}
	else if(contoursplot->selected_objects_line_type==1)
	{
      		contoursplot->object_begin_point.x=x;
      		contoursplot->object_begin_point.y=y;
		gabedit_contoursplot_get_point_control (contoursplot, contoursplot->object_begin_point.x, contoursplot->object_begin_point.y,  objectLine->width,objectLine->width, 0, &x1, &y1);
		gabedit_contoursplot_get_point_control (contoursplot, contoursplot->object_end_point.x, contoursplot->object_end_point.y,  objectLine->width,objectLine->width, 0, &x2, &y2);
	}
	set_object_line(contoursplot, objectLine, 
		x1, y1, 
		x2, y2,
  		objectLine->width,
  		objectLine->arrow_size,
  		objectLine->color,
  		objectLine->style
		);
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
  }
  else if (contoursplot->selected_objects_image_num>-1) 
  {
	gint i = contoursplot->selected_objects_image_num;
	ContoursPlotObjectImage* objectImage = &GABEDIT_ContoursPLOT(contoursplot)->objectsImage[i];
	if(contoursplot->object_begin_point.x>0)
	{
		gint dx,dy;
		x -= contoursplot->object_begin_point.x;
		y -= contoursplot->object_begin_point.y;
		if(x<0) x = 0;
		if(y<0) y = 0;
		dx = x+objectImage->widthi-widget->allocation.width;
		if(dx>0) x-=dx;
		dy = y+objectImage->heighti-widget->allocation.height;
		if(dy>0) y-=dy;
		set_object_image(contoursplot, objectImage,  x, y, objectImage->widthi, objectImage->heighti);
	}
	if(contoursplot->object_end_point.x>0)
	{
		gint w,h;
		x += contoursplot->object_end_point.x;
		y += contoursplot->object_end_point.y;
		if(x<objectImage->xi) x = objectImage->xi+20;
		if(y<objectImage->yi) y = objectImage->yi+20;
		if(x>widget->allocation.width) x = widget->allocation.width;
		if(y>widget->allocation.height) y = widget->allocation.height;
		w = (gint)(x-objectImage->xi);
		h = (gint)(y-objectImage->yi);
		set_object_image(contoursplot, objectImage,  objectImage->xi, objectImage->yi, w, h);
	}

	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));  
  }
  else if (contoursplot->object_begin_point.x>-1)
  {
      	contoursplot->object_end_point.x=x;
      	contoursplot->object_end_point.y=y;
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
  }

  /* Zoom */
  if (contoursplot->mouse_zoom_enabled && 
      (contoursplot->mouse_button == contoursplot->mouse_zoom_button) ){
    if (x < contoursplot->plotting_rect.x)
      x=contoursplot->plotting_rect.x;
       
    if(x > (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) 
      x=(contoursplot->plotting_rect.x + contoursplot->plotting_rect.width);

    if (y < contoursplot->plotting_rect.y)
      y=contoursplot->plotting_rect.y;

    if (y > (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height))
      y=(contoursplot->plotting_rect.y + contoursplot->plotting_rect.height);

    if (contoursplot->zoom_point.x < x){
      contoursplot->zoom_rect.x=contoursplot->zoom_point.x;
      contoursplot->zoom_rect.width=x-contoursplot->zoom_point.x;
    }else{
      contoursplot->zoom_rect.x=x;
      contoursplot->zoom_rect.width=contoursplot->zoom_point.x-x;
    }

    if (contoursplot->zoom_point.y < y){
      contoursplot->zoom_rect.y=contoursplot->zoom_point.y;
      contoursplot->zoom_rect.height=y-contoursplot->zoom_point.y;
    }else{
      contoursplot->zoom_rect.y=y;
      contoursplot->zoom_rect.height=contoursplot->zoom_point.y-y;
    }
    
    gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
  }
/* distance between 2 points */
  if (contoursplot->mouse_distance_enabled && 
      (contoursplot->mouse_button == contoursplot->mouse_distance_button) ){
    if (x < contoursplot->plotting_rect.x)
      x=contoursplot->plotting_rect.x;
       
    if(x > (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) 
      x=(contoursplot->plotting_rect.x + contoursplot->plotting_rect.width);

    if (y < contoursplot->plotting_rect.y)
      y=contoursplot->plotting_rect.y;

    if (y > (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height))
      y=(contoursplot->plotting_rect.y + contoursplot->plotting_rect.height);

      contoursplot->distance_rect.x=contoursplot->distance_point.x;
      contoursplot->distance_rect.width=x-contoursplot->distance_point.x;

      contoursplot->distance_rect.y=contoursplot->distance_point.y;
      contoursplot->distance_rect.height=y-contoursplot->distance_point.y;
    
    gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
  }

  if (contoursplot->mouse_displace_enabled && (contoursplot->mouse_button==contoursplot->mouse_displace_button))
    if ( (x > contoursplot->plotting_rect.x) && 
         (x < (contoursplot->plotting_rect.width + contoursplot->plotting_rect.x)) &&
         (y > contoursplot->plotting_rect.y) && 
         (y < (contoursplot->plotting_rect.height + contoursplot->plotting_rect.y)) ){
      pixel2value(contoursplot, contoursplot->move_point.x, contoursplot->move_point.y, &px, &py);
      pixel2value(contoursplot, x, y, &mx, &my);
    
      gabedit_contoursplot_set_range(contoursplot, contoursplot->xmin+(px-mx), contoursplot->xmax+(px-mx), contoursplot->ymin-(py-my), contoursplot->ymax-(py-my)); 
      contoursplot->move_point.x=x;
      contoursplot->move_point.y=y;
  }

  return FALSE;
}
/****************************************************************************************/
static gint gabedit_contoursplot_scroll (GtkWidget *widget, GdkEventScroll   *event)
{
  GabeditContoursPlot *contoursplot;
  gdouble x_range = 0.0, y_range = 0.0, 
          xmin = 0.0, xmax = 0.0, 
          ymin = 0.0, ymax = 0.0;

  g_return_val_if_fail (widget != NULL, FALSE);
  g_return_val_if_fail (GABEDIT_IS_ContoursPLOT (widget), FALSE);
  g_return_val_if_fail (event != NULL, FALSE);

  contoursplot=GABEDIT_ContoursPLOT(widget);

  if (contoursplot->wheel_zoom_enabled){
    if (event->direction==GDK_SCROLL_UP){
      x_range=contoursplot->wheel_zoom_factor*(contoursplot->xmax-contoursplot->xmin);
      y_range=contoursplot->wheel_zoom_factor*(contoursplot->ymax-contoursplot->ymin);
    }

    if (event->direction==GDK_SCROLL_DOWN){
      x_range=(contoursplot->xmax-contoursplot->xmin)/contoursplot->wheel_zoom_factor;
      y_range=(contoursplot->ymax-contoursplot->ymin)/contoursplot->wheel_zoom_factor;
    }
 
    xmin=contoursplot->xmin+((contoursplot->xmax-contoursplot->xmin)/2.0)-(x_range/2.0);
    xmax=contoursplot->xmin+((contoursplot->xmax-contoursplot->xmin)/2.0)+(x_range/2.0);
     
    ymin=contoursplot->ymin+((contoursplot->ymax-contoursplot->ymin)/2.0)-(y_range/2.0);
    ymax=contoursplot->ymin+((contoursplot->ymax-contoursplot->ymin)/2.0)+(y_range/2.0);

    gabedit_contoursplot_set_range(contoursplot, xmin, xmax, ymin, ymax);
  }  
  return TRUE;
}
/****************************************************************************************/
static void gabedit_contoursplot_style_set (GtkWidget *widget, GtkStyle *previous_style)
{
  GabeditContoursPlot *contoursplot;
  
  g_return_if_fail (widget != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (widget));
  
  contoursplot = GABEDIT_ContoursPLOT (widget);
  
  contoursplot_calculate_legends_sizes(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_set_range (GabeditContoursPlot *contoursplot, gdouble xmin, gdouble xmax, gdouble ymin, gdouble ymax)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if (xmin < xmax){
    contoursplot->xmin=xmin;
    contoursplot->xmax=xmax;
  }
  
  if (ymin < ymax){
    contoursplot->ymin=ymin;
    contoursplot->ymax=ymax;
  }
   
  contoursplot_free_legends(contoursplot);
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_set_range_xmin (GabeditContoursPlot *contoursplot, gdouble xmin)
{
	gabedit_contoursplot_set_range(contoursplot, xmin,  contoursplot->xmax, contoursplot->ymin , contoursplot->ymax);
}
/****************************************************************************************/
void gabedit_contoursplot_set_range_xmax (GabeditContoursPlot *contoursplot, gdouble xmax)
{
	gabedit_contoursplot_set_range(contoursplot, contoursplot->xmin,  xmax, contoursplot->ymin , contoursplot->ymax);
}
/****************************************************************************************/
void gabedit_contoursplot_set_range_ymin (GabeditContoursPlot *contoursplot, gdouble ymin)
{
	gabedit_contoursplot_set_range(contoursplot, contoursplot->xmin,  contoursplot->xmax, ymin , contoursplot->ymax);
}
/****************************************************************************************/
void gabedit_contoursplot_set_range_ymax (GabeditContoursPlot *contoursplot, gdouble ymax)
{
	gabedit_contoursplot_set_range(contoursplot, contoursplot->xmin,  contoursplot->xmax, contoursplot->ymin , ymax);
}
/****************************************************************************************/
void gabedit_contoursplot_set_autorange (GabeditContoursPlot *contoursplot, ContoursPlotData *data)
{
  gdouble xmax, xmin, ymax, ymin;
  ContoursPlotData *current_data; 
  GList *current_node; 
 
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
   
  xmax=1.0;
  xmin=-1.0;
  ymax=1.0;
  ymin=-1.0;

  if (data!=NULL){ 
    xmax=data->xmax;
    xmin=data->xmin;
    ymax=data->ymin;
    ymin=data->ymax;
  }

  if ( (data == NULL) && contoursplot->data_list ){
    current_node=g_list_first(contoursplot->data_list);
    current_data=(ContoursPlotData*)current_node->data;  
    xmin=current_data->xmin;
    xmax=current_data->xmax; 
    ymin=current_data->ymin;
    ymax=current_data->ymax;
    for (; current_node!=NULL; current_node=current_node->next)
    {
        current_data=(ContoursPlotData*)current_node->data;  
        if (xmin > current_data->xmin) xmin=current_data->xmin;
        if (xmax < current_data->xmax) xmax=current_data->xmax;
        if (ymin > current_data->ymin) ymin=current_data->ymin;
        if (ymax < current_data->ymax) ymax=current_data->ymax;
      }
    }

  /*
  {
  gdouble dx,dy;
  dx=fabs(xmax-xmin);
  dy=fabs(ymax-ymin);
  xmin=xmin-dx*0.1;
  xmax=xmax+dx*0.1;
  ymin=ymin-dy*0.1;
  ymax=ymax+dy*0.1;
  }
  */
  
  contoursplot->xmin=xmin;
  contoursplot->xmax=xmax;
  contoursplot->ymin=ymin;
  contoursplot->ymax=ymax;

  contoursplot_free_legends(contoursplot);
  contoursplot_build_legends(contoursplot);  
  contoursplot_calculate_sizes(contoursplot);

  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_get_range (GabeditContoursPlot *contoursplot, gdouble *xmin, gdouble *xmax, gdouble *ymin, gdouble *ymax)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if (xmin!=NULL)
    *xmin=contoursplot->xmin;
  if (xmax!=NULL)
    *xmax=contoursplot->xmax;
  if (ymax!=NULL)
    *ymax=contoursplot->ymax;
  if (ymin!=NULL)
    *ymin=contoursplot->ymin;
}
/****************************************************************************************/
gboolean gabedit_contoursplot_get_point (GabeditContoursPlot *contoursplot, gint x, gint y, gdouble *xv, gdouble *yv)
{
  g_return_val_if_fail (contoursplot != NULL, FALSE);
  g_return_val_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot),FALSE);
  
  /*
  if ((x < contoursplot->plotting_rect.x) || 
      (x > (contoursplot->plotting_rect.x + contoursplot->plotting_rect.width)) || 
      (y < contoursplot->plotting_rect.y) || 
      (y > (contoursplot->plotting_rect.y + contoursplot->plotting_rect.height))) {
    *xv=*yv=0;
    return FALSE;
  }
  */
  
  x=x-contoursplot->plotting_rect.x;
  y=contoursplot->plotting_rect.y+contoursplot->plotting_rect.height-y; 
  
  pixel2value(contoursplot, x, y, xv, yv);
  return TRUE;
}
/****************************************************************************************/
gboolean gabedit_contoursplot_get_point_control(GabeditContoursPlot *contoursplot, gint x, gint y, gint width, gint height, gdouble angle, gdouble *xv, gdouble *yv )
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
	GtkWidget *widget = GTK_WIDGET(contoursplot);


	g_return_val_if_fail (contoursplot != NULL, FALSE);
	g_return_val_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot),FALSE);
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

	gabedit_contoursplot_get_point (contoursplot, x, y, xv, yv);
	xmin = xmax = *xv;
	ymin = ymax = *yv;
	for(i=0;i<4;i++)
	{
		gabedit_contoursplot_get_point (contoursplot, a[i], b[i], &xa, &ya);
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
void gabedit_contoursplot_set_ticks (GabeditContoursPlot *contoursplot, gint hmajor, gint hminor, gint vmajor, gint vminor, gint length)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  contoursplot_free_legends(contoursplot);

  contoursplot->hmajor_ticks=hmajor;
  contoursplot->hminor_ticks=hminor;
  contoursplot->vmajor_ticks=vmajor;
  contoursplot->vminor_ticks=vminor;
  contoursplot->length_ticks=length;
    
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_set_ticks_hmajor (GabeditContoursPlot *contoursplot, gint hmajor)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  contoursplot_free_legends(contoursplot);

  contoursplot->hmajor_ticks=hmajor;
    
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_set_ticks_hminor (GabeditContoursPlot *contoursplot, gint hminor)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  contoursplot_free_legends(contoursplot);

  contoursplot->hminor_ticks=hminor;
    
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_set_ticks_vmajor (GabeditContoursPlot *contoursplot, gint vmajor)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  contoursplot_free_legends(contoursplot);

  contoursplot->vmajor_ticks=vmajor;
    
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_set_ticks_vminor (GabeditContoursPlot *contoursplot, gint vminor)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  contoursplot_free_legends(contoursplot);

  contoursplot->vminor_ticks=vminor;
    
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_set_ticks_length (GabeditContoursPlot *contoursplot, gint length)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  contoursplot_free_legends(contoursplot);

  contoursplot->length_ticks=length;
    
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_get_ticks (GabeditContoursPlot *contoursplot, gint *hmajor, gint *hminor, gint *vmajor, gint *vminor, gint* length)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if (hmajor!=NULL)
    *hmajor=contoursplot->hmajor_ticks;
  if (hminor!=NULL)
    *hminor=contoursplot->hminor_ticks;
  if (vmajor!=NULL)
    *vmajor=contoursplot->vmajor_ticks;
  if (vminor!=NULL)
    *vminor=contoursplot->vminor_ticks;
  if (length!=NULL)
    *length=contoursplot->length_ticks;
}
/****************************************************************************************/
void gabedit_contoursplot_set_margins_left (GabeditContoursPlot *contoursplot, gint left)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  contoursplot_free_legends(contoursplot);

  contoursplot->left_margins=left;
    
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));  
}
/****************************************************************************************/
void gabedit_contoursplot_set_margins_right (GabeditContoursPlot *contoursplot, gint right)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  contoursplot_free_legends(contoursplot);

  contoursplot->right_margins=right;
    
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));  
}
/****************************************************************************************/
void gabedit_contoursplot_set_margins_top (GabeditContoursPlot *contoursplot, gint top)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  contoursplot_free_legends(contoursplot);

  contoursplot->top_margins=top;
    
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));  
}
/****************************************************************************************/
void gabedit_contoursplot_set_margins_bottom (GabeditContoursPlot *contoursplot, gint bottom)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  contoursplot_free_legends(contoursplot);

  contoursplot->bottom_margins=bottom;
    
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));  
}
/****************************************************************************************/
void gabedit_contoursplot_set_x_legends_digits (GabeditContoursPlot *contoursplot, gint digits)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if (digits != contoursplot->x_legends_digits){
    contoursplot_free_legends(contoursplot);
  
    contoursplot->x_legends_digits=digits;
  
    contoursplot_build_legends(contoursplot);
    contoursplot_calculate_legends_sizes(contoursplot);
    contoursplot_calculate_sizes(contoursplot);
    gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
  }
}
/****************************************************************************************/
void gabedit_contoursplot_set_y_legends_digits (GabeditContoursPlot *contoursplot, gint digits)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if (digits != contoursplot->y_legends_digits){
    contoursplot_free_legends(contoursplot);
  
    contoursplot->y_legends_digits=digits;
  
    contoursplot_build_legends(contoursplot);
    contoursplot_calculate_legends_sizes(contoursplot);
    contoursplot_calculate_sizes(contoursplot);
    gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
  }
}
/****************************************************************************************/
gint gabedit_contoursplot_get_x_legends_digits (GabeditContoursPlot *contoursplot)
{
  g_return_val_if_fail (contoursplot != NULL, 0);
  g_return_val_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot), 0);
  
  return(contoursplot->x_legends_digits);
}
/****************************************************************************************/
gint gabedit_contoursplot_get_y_legends_digits (GabeditContoursPlot *contoursplot)
{
  g_return_val_if_fail (contoursplot != NULL, 0);
  g_return_val_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot), 0);
  
  return(contoursplot->y_legends_digits);
}
/****************************************************************************************/
void gabedit_contoursplot_set_background_color (GabeditContoursPlot *contoursplot, GdkColor color)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

  gdk_gc_set_rgb_fg_color(contoursplot->back_gc, &color);

  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_set_grids_attributes (GabeditContoursPlot *contoursplot, 
			GabeditContoursPlotGrid grid, 
			GdkColor color, 
			gint line_width, 
			GdkLineStyle line_style)
{
  GdkGC *gc;

  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  switch (grid){
    case GABEDIT_ContoursPLOT_HMAJOR_GRID:
      gc=contoursplot->hmajor_grid_gc;  
    break;
    case GABEDIT_ContoursPLOT_HMINOR_GRID:
      gc=contoursplot->hminor_grid_gc;
    break;
    case GABEDIT_ContoursPLOT_VMAJOR_GRID:
      gc=contoursplot->vmajor_grid_gc;
    break;
    case GABEDIT_ContoursPLOT_VMINOR_GRID:
      gc=contoursplot->vminor_grid_gc;
    break;
    default:
      return;
  }
  
  gdk_gc_set_rgb_fg_color(gc, &color);
  gdk_gc_set_line_attributes(gc, line_width, line_style, 0 ,0);
  
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_get_grids_attributes (GabeditContoursPlot *contoursplot, 
			GabeditContoursPlotGrid grid, 
			GdkColor *color, 
			gint *line_width, 
			GdkLineStyle *line_style)
{
  GdkGC *gc;
  GdkGCValues gc_values;

  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  g_return_if_fail (color != NULL);
  g_return_if_fail (line_width != NULL);
  g_return_if_fail (line_style != NULL);
  
  switch (grid){
    case GABEDIT_ContoursPLOT_HMAJOR_GRID:
      gc=contoursplot->hmajor_grid_gc;  
    break;
    case GABEDIT_ContoursPLOT_HMINOR_GRID:
      gc=contoursplot->hminor_grid_gc;
    break;
    case GABEDIT_ContoursPLOT_VMAJOR_GRID:
      gc=contoursplot->vmajor_grid_gc;
    break;
    case GABEDIT_ContoursPLOT_VMINOR_GRID:
      gc=contoursplot->vminor_grid_gc;
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
static void contoursplot_enable_grids (GabeditContoursPlot *contoursplot, GabeditContoursPlotGrid grid, gboolean enable)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  switch (grid){
    case GABEDIT_ContoursPLOT_HMAJOR_GRID:
      contoursplot->hmajor_grid=enable;  
    break;
    case GABEDIT_ContoursPLOT_HMINOR_GRID:
      contoursplot->hminor_grid=enable;  
    break;
    case GABEDIT_ContoursPLOT_VMAJOR_GRID:
      contoursplot->vmajor_grid=enable;  
    break;
    case GABEDIT_ContoursPLOT_VMINOR_GRID:
      contoursplot->vminor_grid=enable;  
    break;
    default:
      return;
  }
  
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_enable_grids (GabeditContoursPlot *contoursplot, GabeditContoursPlotGrid grid, gboolean enable)
{

	GtkUIManager *manager = g_object_get_data(G_OBJECT (contoursplot), "Manager");
	GtkAction* action = NULL;
	if(grid==GABEDIT_ContoursPLOT_HMAJOR_GRID)
		action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderGrid/HGridShowMajor");
	if(grid==GABEDIT_ContoursPLOT_HMINOR_GRID)
		action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderGrid/HGridShowMinor");
	if(grid==GABEDIT_ContoursPLOT_VMAJOR_GRID)
		action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderGrid/VGridShowMajor");
	if(grid==GABEDIT_ContoursPLOT_VMINOR_GRID)
		action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderGrid/VGridShowMinor");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
void gabedit_contoursplot_add_data(GabeditContoursPlot *contoursplot, ContoursPlotData *data)
{
	g_return_if_fail (contoursplot != NULL);
	g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
	g_return_if_fail (data != NULL);
  
	if (g_list_find (contoursplot->data_list, (gpointer)data)==NULL)
	{
		gint i;
		contoursplot->data_list=g_list_append(contoursplot->data_list, (gpointer) data);
		for(i=0;i<data->nContours;i++)
		contoursplot_build_points_contour(GABEDIT_ContoursPLOT(contoursplot), &data->contours[i]);
	}
}
/****************************************************************************************/
void gabedit_contoursplot_remove_data(GabeditContoursPlot *contoursplot, ContoursPlotData *data)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  g_return_if_fail (data != NULL);

  if (g_list_find (contoursplot->data_list, (gpointer)data)!=NULL){
    contoursplot->data_list=g_list_remove_all (contoursplot->data_list, (gpointer) data);
  }
}

/****************************************************************************************/
void gabedit_contoursplot_configure_mouse_zoom(GabeditContoursPlot *contoursplot, gboolean enabled, gint button)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

  contoursplot->mouse_zoom_enabled=enabled;
  contoursplot->mouse_zoom_button=button;
}
/****************************************************************************************/
void gabedit_contoursplot_configure_mouse_distance(GabeditContoursPlot *contoursplot, gboolean enabled, gint button)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

  contoursplot->mouse_distance_enabled=enabled;
  contoursplot->mouse_distance_button=button;
}
/****************************************************************************************/
void gabedit_contoursplot_configure_wheel_zoom(GabeditContoursPlot *contoursplot, gboolean enabled, gdouble factor)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

  contoursplot->wheel_zoom_enabled=enabled;
  contoursplot->wheel_zoom_factor=factor;
}
/****************************************************************************************/
void gabedit_contoursplot_configure_mouse_displace(GabeditContoursPlot *contoursplot, gboolean enabled, gint button)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

  contoursplot->mouse_displace_enabled=enabled;
  contoursplot->mouse_displace_button=button;
}
/****************************************************************************************/
void gabedit_contoursplot_configure_mouse_autorange(GabeditContoursPlot *contoursplot, gboolean enabled, gint button)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));

  contoursplot->mouse_autorange_enabled=enabled;
  contoursplot->mouse_autorange_button=button;
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
static void writeBMP(GabeditContoursPlot *contoursplot, gchar *fileName)
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

	drawable = GTK_WIDGET(contoursplot);

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
		{ int it = fwrite(rgbtmp,1,1,file);}
  	}

  	fclose(file);
  	g_free(rgbbuf);
}
/****************************************************************************************/
static void writeTransparentPNG(GabeditContoursPlot *contoursplot, gchar *fileName)
{       
	GtkWidget* widget = GTK_WIDGET(contoursplot);
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
		gdk_gc_get_values(contoursplot->back_gc, &gc_values);
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
void gabedit_contoursplot_save_image(GabeditContoursPlot *contoursplot, gchar *fileName, gchar* type)
{       
	GtkWidget* widget = GTK_WIDGET(contoursplot);
	int width;
	int height;
	GError *error = NULL;
	GdkPixbuf  *pixbuf = NULL;

	if(fileName && type && !strcmp(type,"bmp"))
	{
		writeBMP(contoursplot, fileName);
		return;
	}
	if(fileName && type && !strcmp(type,"tpng"))
	{
		writeTransparentPNG(contoursplot, fileName);
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
static void contoursplot_calculate_sizes (GabeditContoursPlot *contoursplot)
{
  GtkWidget *widget;
  gint left = 10;
  gint right = 10;
  gint top = 5;
  gint bottom = 5;
  
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  widget = GTK_WIDGET(contoursplot);
  left += 0.75*((gdouble)contoursplot->y_legends_width);
  right += 0.75*((gdouble)contoursplot->y_legends_width);
  top += 0.5*((gdouble)contoursplot->x_legends_height);
  bottom += 0.5*((gdouble)contoursplot->x_legends_height);
  if(contoursplot->show_left_legends) left = 10+1.5*((gdouble)contoursplot->y_legends_width)+1.0*((gdouble)contoursplot->v_label_height);
  if(contoursplot->show_right_legends) right = 10+1.5*((gdouble)contoursplot->y_legends_width)+1.0*((gdouble)contoursplot->v_label_height);
  if(contoursplot->show_top_legends) top = 5+1.5*((gdouble)contoursplot->x_legends_height)+1.0*((gdouble)contoursplot->h_label_height);
  if(contoursplot->show_bottom_legends) bottom = 5+1.5*((gdouble)contoursplot->x_legends_height)+1.0*((gdouble)contoursplot->h_label_height);
  if(contoursplot->show_colormap) right += (gdouble)contoursplot->colormap_width;
  if (contoursplot->v_label && !contoursplot->show_left_legends && contoursplot->show_right_legends) right += contoursplot->v_label_height;

  left += (gint)(contoursplot->left_margins/100.0*widget->allocation.width);
  right += (gint)(contoursplot->right_margins/100.0*widget->allocation.width);
  top += (gint)(contoursplot->top_margins/100.0*widget->allocation.height);
  bottom += (gint)(contoursplot->bottom_margins/100.0*widget->allocation.height);

  contoursplot->plotting_rect.x = left;
  contoursplot->plotting_rect.y = top;
  
 
  contoursplot->plotting_rect.width=widget->allocation.width  -left - right;

  contoursplot->plotting_rect.height=widget->allocation.height  - top - bottom ;
  
  if (contoursplot->hmajor_ticks>1){
    contoursplot->d_hmajor=((gdouble)contoursplot->plotting_rect.width)/((gdouble)contoursplot->hmajor_ticks-1.0);
    contoursplot->d_hminor=((gdouble)contoursplot->d_hmajor)/((gdouble)contoursplot->hminor_ticks+1.0);
  }
  
  if (contoursplot->vmajor_ticks>1){
    contoursplot->d_vmajor=((gdouble)contoursplot->plotting_rect.height)/((gdouble)contoursplot->vmajor_ticks-1.0);
    contoursplot->d_vminor=((gdouble)contoursplot->d_vmajor)/((gdouble)contoursplot->vminor_ticks+1.0);
  }

  /* Creating the plotting area (everytime the plotting area's size is changed, the size of the pismap
     must be changed too, so, a new one, with the right size is created*/
  if (contoursplot->plotting_area!=NULL) g_object_unref(G_OBJECT(contoursplot->plotting_area));
  if (contoursplot->cairo_widget!=NULL) cairo_destroy (contoursplot->cairo_widget);
  if (contoursplot->cairo_area!=NULL) cairo_destroy (contoursplot->cairo_area);

  if (GTK_WIDGET_REALIZED(widget)) 
  {
    contoursplot->plotting_area=gdk_pixmap_new(widget->window, contoursplot->plotting_rect.width, contoursplot->plotting_rect.height, -1);
    contoursplot->cairo_area = gdk_cairo_create (contoursplot->plotting_area);
    contoursplot->cairo_widget = gdk_cairo_create (widget->window);
  }
  contoursplot->colormap_height = contoursplot->plotting_rect.height;
  reset_object_text_pixels(contoursplot);
  reset_object_line_pixels(contoursplot);
}
/****************************************************************************************/
static void value2pixel(GabeditContoursPlot *contoursplot, gdouble xv, gdouble yv, gint *x, gint *y)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  *x=(xv-contoursplot->xmin)*((gdouble)contoursplot->plotting_rect.width)/(contoursplot->xmax-contoursplot->xmin);
  if(contoursplot->reflect_x) *x = contoursplot->plotting_rect.width - *x;

  *y=(yv-contoursplot->ymin)*((gdouble)contoursplot->plotting_rect.height)/(contoursplot->ymax-contoursplot->ymin);
  if(contoursplot->reflect_y) *y = contoursplot->plotting_rect.height - *y;
}
/****************************************************************************************/
static void pixel2value(GabeditContoursPlot *contoursplot, gint xp, gint yp, gdouble *x, gdouble *y)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  gdouble xxp = (gdouble)xp;
  gdouble yyp = (gdouble)yp;
  if(contoursplot->reflect_x)  xxp = contoursplot->plotting_rect.width-xxp;
  if(contoursplot->reflect_y)  yyp = contoursplot->plotting_rect.height-yyp;

    
  *x=contoursplot->xmin+((gdouble)xxp)*(contoursplot->xmax-contoursplot->xmin)/((gdouble)contoursplot->plotting_rect.width);
  *y=contoursplot->ymin+((gdouble)yyp)*(contoursplot->ymax-contoursplot->ymin)/((gdouble)contoursplot->plotting_rect.height);
}
/****************************************************************************************/
static void contoursplot_calculate_colormap_sizes(GabeditContoursPlot *contoursplot)
{
  GtkWidget *widget;
    
  g_return_if_fail (contoursplot != NULL);
  
  widget=GTK_WIDGET(contoursplot);

  if (contoursplot->colormap_legends)
  {
    	pango_layout_set_font_description (contoursplot->colormap_legends[0], widget->style->font_desc);  
    	pango_layout_get_size(contoursplot->colormap_legends[0], &(contoursplot->colormap_width), &(contoursplot->colormap_height));
	contoursplot->colormap_width /=PANGO_SCALE;
	contoursplot->colormap_height /=PANGO_SCALE;
	contoursplot->colormap_width *=2;
  	contoursplot->colormap_height = contoursplot->plotting_rect.height;
  }
} 
/****************************************************************************************/
static void contoursplot_build_colormap_legends(GabeditContoursPlot *contoursplot)
{
  GtkWidget *widget;
  gint loop;
  ColorMap* colorMap = NULL;
  gint nlegends;
  gdouble vmin;
  gdouble dv;
  gdouble value;


  g_return_if_fail (contoursplot != NULL);
  
  widget=GTK_WIDGET(contoursplot);

  colorMap =GABEDIT_ContoursPLOT(contoursplot)->colorsMap.colorMap;
  if(!colorMap) return;
  if(colorMap->numberOfColors<2)return;

  nlegends = GABEDIT_ContoursPLOT(contoursplot)->colormap_nlegends;

  contoursplot->colormap_legends=g_new(PangoLayout*, nlegends);
  if(contoursplot->colormap_legends_str) g_free(contoursplot->colormap_legends_str);
    contoursplot->colormap_legends_str=g_malloc( (sizeof(gchar*) * nlegends)); 
    vmin = colorMap->colorValue[0].value;
    dv = (colorMap->colorValue[colorMap->numberOfColors-1].value-colorMap->colorValue[0].value)/(nlegends-1);
    for (loop=0; loop<nlegends; loop++)
    {
      contoursplot->colormap_legends_str[loop]=g_malloc( (sizeof(gchar) * (contoursplot->y_legends_digits+1))); 
      value = vmin + dv*loop;
      snprintf(contoursplot->colormap_legends_str[loop], contoursplot->y_legends_digits+1, "%lf", value);
      contoursplot->colormap_legends_str[loop][contoursplot->y_legends_digits]='\0';
      /* printf("str[%d]=%s\n",loop, contoursplot->colormap_legends_str[loop]);*/
      contoursplot->colormap_legends[loop]=gtk_widget_create_pango_layout (widget, contoursplot->colormap_legends_str[loop]); 
    }
    contoursplot->colormap_nlegends=nlegends; 
  contoursplot_calculate_colormap_sizes(contoursplot);
} 
/****************************************************************************************/
static void contoursplot_calculate_legends_sizes(GabeditContoursPlot *contoursplot)
{
  GtkWidget *widget;
    
  g_return_if_fail (contoursplot != NULL);
  
  widget=GTK_WIDGET(contoursplot);

  if (contoursplot->x_legends_digits==0 && contoursplot->y_legends_digits==0){
    contoursplot->x_legends_width=0;
    contoursplot->x_legends_height=0;
    contoursplot->y_legends_width=0;
    contoursplot->y_legends_height=0;
    return;
  }

  if (contoursplot->h_legends)
  {
    pango_layout_set_font_description (contoursplot->h_legends[0], widget->style->font_desc);  
    pango_layout_get_size(contoursplot->h_legends[0], &(contoursplot->x_legends_width), &(contoursplot->x_legends_height));
  }
  if (contoursplot->v_legends)
  {
	gint i;
    	pango_layout_set_font_description (contoursplot->v_legends[0], widget->style->font_desc);  
    	pango_layout_get_size(contoursplot->v_legends[0], &(contoursplot->y_legends_width), &(contoursplot->y_legends_height));
    	for (i=1; i<contoursplot->vmajor_ticks; i++)
    	{
		gint w,h;
    		pango_layout_set_font_description (contoursplot->v_legends[i], widget->style->font_desc);  
    		pango_layout_get_size(contoursplot->v_legends[i], &w,&h); 
		if(w>contoursplot->y_legends_width) contoursplot->y_legends_width=w;
    	}
  }

  contoursplot->x_legends_width/=PANGO_SCALE;
  contoursplot->x_legends_height/=PANGO_SCALE;

  contoursplot->y_legends_width/=PANGO_SCALE;
  contoursplot->y_legends_height/=PANGO_SCALE;

  contoursplot->h_label_width = 0;
  contoursplot->h_label_height =0;
  if (contoursplot->h_label)
  {
    	pango_layout_set_font_description (contoursplot->h_label, widget->style->font_desc);  
    	pango_layout_get_size(contoursplot->h_label, &(contoursplot->h_label_width), &(contoursplot->h_label_height));
	contoursplot->h_label_width /=PANGO_SCALE;
	contoursplot->h_label_height /=PANGO_SCALE;
  }
  contoursplot->v_label_width = 0;
  contoursplot->v_label_height =0;
  if (contoursplot->v_label)
  {
    	pango_layout_set_font_description (contoursplot->v_label, widget->style->font_desc);  
    	pango_layout_get_size(contoursplot->v_label, &(contoursplot->v_label_width), &(contoursplot->v_label_height));
	contoursplot->v_label_width /=PANGO_SCALE;
	contoursplot->v_label_height /=PANGO_SCALE;
  }
} 
/****************************************************************************************/
static void contoursplot_build_legends(GabeditContoursPlot *contoursplot)
{
  GtkWidget *widget;
  gint loop;

  g_return_if_fail (contoursplot != NULL);
  
  widget=GTK_WIDGET(contoursplot);

  if ( (contoursplot->hmajor_ticks > 1) && (contoursplot->x_legends_digits != 0)){
    contoursplot->d_hlegend=fabs(contoursplot->xmax-contoursplot->xmin)/((gdouble)contoursplot->hmajor_ticks-1.0);

    contoursplot->h_legends=g_new(PangoLayout*, contoursplot->hmajor_ticks);
    if(contoursplot->h_legends_str) g_free(contoursplot->h_legends_str);
    contoursplot->h_legends_str = g_malloc( (sizeof(gchar*) * contoursplot->hmajor_ticks ) );
    for (loop=0; loop<contoursplot->hmajor_ticks; loop++){
      contoursplot->h_legends_str[loop] = g_malloc( (sizeof(gchar) * (contoursplot->x_legends_digits+1)) );
      snprintf(contoursplot->h_legends_str[loop] , contoursplot->x_legends_digits+1, "%lf", contoursplot->xmin + (gdouble)loop*contoursplot->d_hlegend);
      contoursplot->h_legends_str[loop] [contoursplot->x_legends_digits]='\0';
      contoursplot->h_legends[loop]=gtk_widget_create_pango_layout (widget, contoursplot->h_legends_str[loop]); 
    }
  }

  if ((contoursplot->vmajor_ticks > 1) && (contoursplot->y_legends_digits != 0) ){  
    contoursplot->d_vlegend=fabs(contoursplot->ymax-contoursplot->ymin)/((gdouble)contoursplot->vmajor_ticks-1.0);
 
    contoursplot->v_legends=g_new(PangoLayout*, contoursplot->vmajor_ticks);
    if(contoursplot->v_legends_str) g_free(contoursplot->v_legends_str);
    contoursplot->v_legends_str=g_malloc( (sizeof(gchar*) * contoursplot->vmajor_ticks)); 
    for (loop=0; loop<contoursplot->vmajor_ticks; loop++){
      contoursplot->v_legends_str[loop]=g_malloc( (sizeof(gchar) * (contoursplot->y_legends_digits+1))); 
      snprintf(contoursplot->v_legends_str[loop], contoursplot->y_legends_digits+1, "%lf", contoursplot->ymin + (gdouble)loop*contoursplot->d_vlegend);
      contoursplot->v_legends_str[loop][contoursplot->y_legends_digits]='\0';
      contoursplot->v_legends[loop]=gtk_widget_create_pango_layout (widget, contoursplot->v_legends_str[loop]); 
    }
  }
  contoursplot->h_label = get_pango_str(contoursplot, contoursplot->h_label_str);
  contoursplot->v_label = get_pango_str(contoursplot, contoursplot->v_label_str);
  contoursplot_calculate_legends_sizes(contoursplot);
} 
/****************************************************************************************/
static void contoursplot_free_legends(GabeditContoursPlot *contoursplot)
{
  gint loop;

  g_return_if_fail (contoursplot != NULL);
    
  if (contoursplot->h_legends!=NULL){
    for (loop=0; loop<contoursplot->hmajor_ticks; loop++)
      if (contoursplot->h_legends[loop]!=NULL)
        g_object_unref(G_OBJECT(contoursplot->h_legends[loop]));
    g_free(contoursplot->h_legends);
    contoursplot->h_legends=NULL;    
  }

  if (contoursplot->v_legends!=NULL){
    for (loop=0; loop<contoursplot->vmajor_ticks; loop++)
      if (contoursplot->v_legends[loop]!=NULL)
        g_object_unref(G_OBJECT(contoursplot->v_legends[loop]));
    g_free(contoursplot->v_legends);
    contoursplot->v_legends=NULL;    
  }  
  if (contoursplot->v_legends_str!=NULL) 
  {
    	for (loop=0; loop<contoursplot->vmajor_ticks; loop++)
		g_free(contoursplot->v_legends_str[loop]);
	 g_free(contoursplot->v_legends_str);
  }
  if (contoursplot->h_legends_str!=NULL)
  {
    	for (loop=0; loop<contoursplot->hmajor_ticks; loop++)
		g_free(contoursplot->h_legends_str[loop]);
	  g_free(contoursplot->h_legends_str);
  }
  contoursplot->v_legends_str = NULL;
  contoursplot->h_legends_str = NULL;
}
/****************************************************************************************/
static void contoursplot_reflect_x (GabeditContoursPlot *contoursplot, gboolean reflection)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if(contoursplot->reflect_x == reflection) return; 

  contoursplot->reflect_x = reflection;
  reset_object_text_pixels(contoursplot);
  reset_object_line_pixels(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_reflect_x (GabeditContoursPlot *contoursplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (contoursplot), "Manager");
	GtkAction* action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderDirections/DirectionReflectX");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
static void contoursplot_reflect_y (GabeditContoursPlot *contoursplot, gboolean reflection)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if(contoursplot->reflect_y == reflection) return; 

  contoursplot->reflect_y = reflection;
  reset_object_text_pixels(contoursplot);
  reset_object_line_pixels(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_reflect_y (GabeditContoursPlot *contoursplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (contoursplot), "Manager");
	GtkAction* action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderDirections/DirectionReflectY");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
static void contoursplot_show_label_contours (GabeditContoursPlot *contoursplot, gboolean show)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if(contoursplot->show_label_contours == show) return; 

  contoursplot->show_label_contours = show; 

  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
static void contoursplot_build_position_label_contours (GabeditContoursPlot *contoursplot)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  reset_label_contours(GTK_WIDGET(contoursplot), NULL);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
static void contoursplot_show_colormap (GabeditContoursPlot *contoursplot, gboolean show)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if(contoursplot->show_colormap == show) return; 

  contoursplot->show_colormap = show; 
  contoursplot_build_colormap_legends(contoursplot);
  contoursplot_calculate_colormap_sizes(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
static void contoursplot_dashed_negative_contours (GabeditContoursPlot *contoursplot, gboolean dashed)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if(contoursplot->dashed_negative_contours == dashed) return; 

  contoursplot->dashed_negative_contours = dashed;
  reset_contour_lines_styles(GTK_WIDGET(contoursplot), NULL);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
static void contoursplot_show_left_legends (GabeditContoursPlot *contoursplot, gboolean show)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if(contoursplot->show_left_legends == show) return; 

  contoursplot->show_left_legends = show; 
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_legends_sizes(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_show_left_legends (GabeditContoursPlot *contoursplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (contoursplot), "Manager");
	GtkAction* action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderLegends/LegendShowLeft");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
static void contoursplot_show_right_legends (GabeditContoursPlot *contoursplot, gboolean show)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if(contoursplot->show_right_legends == show) return; 

  contoursplot->show_right_legends = show; 
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_legends_sizes(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_show_right_legends (GabeditContoursPlot *contoursplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (contoursplot), "Manager");
	GtkAction* action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderLegends/LegendShowRight");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
static void contoursplot_show_top_legends (GabeditContoursPlot *contoursplot, gboolean show)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if(contoursplot->show_top_legends == show) return; 

  contoursplot->show_top_legends = show; 
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_legends_sizes(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
  gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_show_top_legends (GabeditContoursPlot *contoursplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (contoursplot), "Manager");
	GtkAction* action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderLegends/LegendShowTop");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
static void contoursplot_show_bottom_legends (GabeditContoursPlot *contoursplot, gboolean show)
{
  g_return_if_fail (contoursplot != NULL);
  g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
  
  if(contoursplot->show_bottom_legends == show) return; 

  contoursplot->show_bottom_legends = show; 
  contoursplot_build_legends(contoursplot);
  contoursplot_calculate_legends_sizes(contoursplot);
  contoursplot_calculate_sizes(contoursplot);
   gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_show_bottom_legends (GabeditContoursPlot *contoursplot, gboolean enable)
{
	GtkUIManager *manager = g_object_get_data(G_OBJECT (contoursplot), "Manager");
	GtkAction* action = gtk_ui_manager_get_action (manager, "/MenuContoursPlot/Render/RenderLegends/LegendShowBottom");
	if(action) gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),enable);
}
/****************************************************************************************/
void gabedit_contoursplot_set_font (GabeditContoursPlot *contoursplot, gchar* fontName)
{
	GtkWidget* widget = NULL;
	g_return_if_fail (contoursplot != NULL);
	g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
	PangoFontDescription *font_desc = pango_font_description_from_string (fontName);

	widget=GTK_WIDGET(contoursplot);

	if(font_desc)
	{
		contoursplot->font_size =  contoursplot_get_font_size (widget, font_desc);
		gtk_widget_modify_font (widget, font_desc);  
  		contoursplot_build_legends(contoursplot);
  		contoursplot_calculate_legends_sizes(contoursplot);
  		contoursplot_calculate_sizes(contoursplot);
		gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
	}
}
/****************************************************************************************/
void gabedit_contoursplot_set_x_label (GabeditContoursPlot *contoursplot, G_CONST_RETURN gchar* str)
{
	GtkWidget* widget = NULL;
	g_return_if_fail (contoursplot != NULL);
	g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
	widget=GTK_WIDGET(contoursplot);

  	if(contoursplot->h_label_str) g_free(contoursplot->h_label_str);
	if(str && strlen(str)>0)
	{
		contoursplot->h_label_str = g_strdup(str);
	}
	else
	{
		contoursplot->h_label_str = NULL;
	}

  	contoursplot_build_legends(contoursplot);
  	contoursplot_calculate_legends_sizes(contoursplot);
  	contoursplot_calculate_sizes(contoursplot);
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
void gabedit_contoursplot_set_y_label (GabeditContoursPlot *contoursplot, G_CONST_RETURN gchar* str)
{
	GtkWidget* widget = NULL;
	g_return_if_fail (contoursplot != NULL);
	g_return_if_fail (GABEDIT_IS_ContoursPLOT (contoursplot));
	widget=GTK_WIDGET(contoursplot);

  	if(contoursplot->v_label_str) g_free(contoursplot->v_label_str);
	if(str && strlen(str)>0)
	{
		contoursplot->v_label_str = g_strdup(str);
	}
	else
		contoursplot->v_label_str = NULL;

  	contoursplot_build_legends(contoursplot);
  	contoursplot_calculate_legends_sizes(contoursplot);
  	contoursplot_calculate_sizes(contoursplot);
	gtk_widget_queue_draw(GTK_WIDGET(contoursplot));
}
/****************************************************************************************/
static gboolean contoursplot_motion_notify_event(GtkWidget *contoursplot, GdkEventMotion *event, gpointer user_data)
{
	double xv, yv;
	int x, y;
	char str[50];
	int context_id;
	GtkWidget* statusbar = g_object_get_data(G_OBJECT (contoursplot), "StatusBar");

	x=event->x;
	y=event->y;

	if (event->is_hint || (event->window != contoursplot->window))
		gdk_window_get_pointer (contoursplot->window, &x, &y, NULL);

	if(gabedit_contoursplot_get_point(GABEDIT_ContoursPLOT(contoursplot), x, y, &xv, &yv))
		snprintf(str, 50, "Mouse position: %f, %f", xv, yv);
	else
		sprintf(str, " ");
	context_id=gtk_statusbar_get_context_id (GTK_STATUSBAR(statusbar), "mouse position");
	gtk_statusbar_push (GTK_STATUSBAR(statusbar), context_id, str);
    
	return TRUE;
}
/****************************************************************************************/
GtkWidget* gabedit_contoursplot_new_window(gchar* title, GtkWidget*parent)
{
	GtkWidget* window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	GtkWidget* table = NULL;
	GtkWidget* box = NULL;
	GtkWidget* frame_contoursplot = NULL;
	GtkWidget* contoursplot = NULL;

	GtkWidget *statusbar = NULL;

	gtk_window_set_title (GTK_WINDOW (window), title);
	gtk_signal_connect (GTK_OBJECT (window), "delete_event", G_CALLBACK (gtk_widget_destroy), NULL);
	gtk_container_set_border_width (GTK_CONTAINER (window), 10);
	gtk_window_set_default_size (GTK_WINDOW(window),2*gdk_screen_width()/3,2*gdk_screen_height()/3);
	
	table=gtk_table_new(3, 1, FALSE);
	gtk_container_add(GTK_CONTAINER(window), table);
	gtk_widget_show(table);
  
	frame_contoursplot=gtk_frame_new("Contours GabeditPlot");
	gtk_table_attach(GTK_TABLE(table), frame_contoursplot, 0, 1, 0, 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
	gtk_widget_show(frame_contoursplot);
  
	contoursplot = gabedit_contoursplot_new();
	gtk_container_add(GTK_CONTAINER(frame_contoursplot), contoursplot);
	gtk_widget_show (contoursplot);

	g_object_set_data (G_OBJECT (window), "ContoursPLOT",contoursplot);

	box=gtk_hbox_new(FALSE,0);
	gtk_table_attach(GTK_TABLE(table), box, 0, 1, 1, 2, GTK_FILL, GTK_FILL, 5, 5);
	gtk_widget_show (box);
	g_object_set_data(G_OBJECT (contoursplot), "Box", box);

	statusbar=gtk_statusbar_new();
	gtk_table_attach(GTK_TABLE(table), statusbar, 0, 1, 2, 3, GTK_FILL, GTK_FILL, 5, 5);
	gtk_widget_show (statusbar);
	g_object_set_data(G_OBJECT (contoursplot), "StatusBar", statusbar);

	if(parent) gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parent));
	gtk_widget_show (window);

        gabedit_contoursplot_set_range(GABEDIT_ContoursPLOT(contoursplot),  0.0,  10,  0,  20);
	gabedit_contoursplot_set_x_legends_digits(GABEDIT_ContoursPLOT(contoursplot), 5);
	gabedit_contoursplot_set_y_legends_digits(GABEDIT_ContoursPLOT(contoursplot), 5);

	gabedit_contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_HMAJOR_GRID, FALSE);
	gabedit_contoursplot_enable_grids (GABEDIT_ContoursPLOT(contoursplot), GABEDIT_ContoursPLOT_VMAJOR_GRID, FALSE);
	gabedit_contoursplot_show_top_legends (GABEDIT_ContoursPLOT(contoursplot),FALSE);
	gabedit_contoursplot_show_right_legends (GABEDIT_ContoursPLOT(contoursplot),FALSE);

	g_signal_connect_after(G_OBJECT(contoursplot), "motion-notify-event", G_CALLBACK(contoursplot_motion_notify_event), NULL);

	g_object_set_data(G_OBJECT (contoursplot), "Window", window);
	g_object_set_data(G_OBJECT (window), "Box", box);

	gabedit_contoursplot_set_font (GABEDIT_ContoursPLOT(contoursplot), "sans 12");

	return window;
}
/****************************************************************************************/
void gabedit_contoursplot_help()
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
			"\t Left button + Shift key : compute distance between 2 points\n"
			"\t d key : compute f and its dervivatives, using a bicubic interpolation\n"
			"\t Left button + t key : insert a text\n"
			"\t Left button + l key : insert a line (arrow)\n"
			"\t Left button + i key : insert an image (arrow)\n"
			"\t Left button + r key : remove selected image/text/line\n"
			"\t Left button, double click : select a data to change\n"
			"\n\n"
			"Popup menu :\n"
			"\t Set : ticks, margins, ranges, X and Y labels, digits, font size, auto ranges\n"
			"\t Render : grids, directions, legends, colormap\n"
			"\t Data : read data from a grid ASCII file (txt)\n"
			"\t        Save all data at a txt file\n"
			"\t        Remove all all\n"
			"\t Contours : set values or delete contours\n"
			"\t ColorMap : set colormap\n"
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
