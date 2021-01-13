/* UtilsCairo.c */
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cairo-pdf.h>
#include <cairo-ps.h>
#include <cairo-svg.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>
#include <glib.h>


/**********************************************************************************/
#define SCALE(i) (i / 65535.)
/**********************************************************************************/
void gabedit_cairo_string(cairo_t* cr, GtkWidget* parent, PangoFontDescription *font_desc, GdkGC* gc , gint x, gint y, G_CONST_RETURN gchar* str, gboolean centerX, gboolean centerY)
{
	int width  = 0;
	int height = 0;
	PangoStyle style = pango_font_description_get_style(font_desc);;
	cairo_font_slant_t cairoStyle = CAIRO_FONT_SLANT_NORMAL;
	G_CONST_RETURN gchar *name = pango_font_description_get_family (font_desc);
	PangoWeight weight = pango_font_description_get_weight(font_desc);
	cairo_font_weight_t cairoWeight = PANGO_WEIGHT_NORMAL;
	gint fontSize = pango_font_description_get_size (font_desc)/PANGO_SCALE;
	GdkGCValues values;
	GdkColormap *colormap;
	GdkColor color;

	cairo_save (cr); /* stack-pen-size */
	gdk_gc_get_values(gc, &values);
   	colormap  = gdk_drawable_get_colormap(parent->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);

	switch(style)
	{
		case PANGO_STYLE_NORMAL:  cairoStyle = CAIRO_FONT_SLANT_NORMAL;break;
		case PANGO_STYLE_OBLIQUE: cairoStyle = CAIRO_FONT_SLANT_OBLIQUE;break;
		case PANGO_STYLE_ITALIC : cairoStyle = CAIRO_FONT_SLANT_ITALIC;break;
	}
	switch(weight)
	{
		case PANGO_WEIGHT_THIN: 
		case PANGO_WEIGHT_BOOK : 
		case PANGO_WEIGHT_MEDIUM: 
		case PANGO_WEIGHT_ULTRAHEAVY: 
		case PANGO_WEIGHT_ULTRALIGHT: 
		case PANGO_WEIGHT_LIGHT: 
		case PANGO_WEIGHT_NORMAL: 
			cairoWeight = PANGO_WEIGHT_NORMAL;
			break;
		case PANGO_WEIGHT_SEMIBOLD: 
		case PANGO_WEIGHT_BOLD: 
		case PANGO_WEIGHT_ULTRABOLD: 
		case PANGO_WEIGHT_HEAVY: 
			cairoWeight = CAIRO_FONT_WEIGHT_BOLD;
	}

	height = fontSize;
	width = fontSize*strlen(str);
	if(centerX) x -= width/4;
	if(centerY) y += height/4;
	cairo_select_font_face (cr, name, cairoStyle, cairoWeight);
	gdk_cairo_set_source_color (cr, &color);
        cairo_set_font_size (cr, fontSize);
	cairo_move_to (cr, x, y);
	cairo_show_text (cr, str);
	cairo_stroke (cr);
	cairo_restore (cr); /* stack-pen-size */
}
/*****************************************************************************/
void gabedit_cairo_triangle(cairo_t *cr,  GtkWidget* parent, GdkGC* gc,
		gdouble x1,gdouble y1,gdouble x2,gdouble y2, gdouble x3,gdouble y3)
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
	cairo_save (cr); /* stack-pen-size */
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
   	colormap  = gdk_drawable_get_colormap(parent->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);
	if( ndash != 0) cairo_set_dash (cr, dashes, ndash, offset);

	r = SCALE(color.red);
	g = SCALE(color.green);
	b = SCALE(color.blue);
	cairo_set_line_width (cr, values.line_width);

	cairo_move_to (cr, x1, y1);
	cairo_line_to (cr, x2, y2);
	cairo_rel_line_to (cr, x3,y3);
	cairo_rel_line_to (cr, x1,y1);
	cairo_close_path (cr);
	cairo_set_source_rgb (cr, r, g, b);
	cairo_fill_preserve (cr);
	cairo_set_source_rgb (cr, 0, 0, 0);

	cairo_stroke (cr);
	cairo_restore (cr); /* stack-pen-size */
}
/*****************************************************************************/
void gabedit_cairo_line(cairo_t *cr,  GtkWidget* parent, GdkGC* gc, gdouble x1,gdouble y1,gdouble x2,gdouble y2)
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
	cairo_save (cr); /* stack-pen-size */
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
   	colormap  = gdk_drawable_get_colormap(parent->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);
	if( ndash != 0) cairo_set_dash (cr, dashes, ndash, offset);

	r = SCALE(color.red);
	g = SCALE(color.green);
	b = SCALE(color.blue);
	cairo_set_source_rgba (cr, r, g, b, 1.0);

	cairo_set_line_width (cr, values.line_width);
	cairo_move_to (cr, x1, y1);
	cairo_line_to (cr, x2, y2);
	cairo_stroke (cr);
	cairo_restore (cr); /* stack-pen-size */
}
/*****************************************************************************/
void gabedit_cairo_arc(cairo_t *cr,  GtkWidget* parent, GdkGC* gc,
		gint xc,gint yc,gint rayon, gdouble angle1, gdouble angle2, gdouble scale1, gdouble scale2)
{
	GdkGCValues values;
	GdkColor color;
	double r,g,b;
	GdkColormap *colormap;
	if(!cr) return;
	if(!gc) return;
	cairo_save (cr); /* stack-pen-size */
	cairo_translate(cr,xc,yc);
	cairo_scale(cr,scale1,scale2);
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
   	colormap  = gdk_drawable_get_colormap(parent->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);
	r = SCALE(color.red);
	g = SCALE(color.green);
	b = SCALE(color.blue);
	cairo_set_source_rgba (cr, r, g, b, 1.0);
	cairo_set_line_width (cr, values.line_width);
	cairo_arc (cr, 0, 0, rayon, angle1, angle2);
	/*if(values.fill==GDK_SOLID) cairo_fill (cr);*/
	cairo_stroke (cr);
	cairo_restore (cr); /* stack-pen-size */
}
/*****************************************************************************/
void gabedit_cairo_cercle(cairo_t *cr,  GtkWidget* parent, GdkGC* gc,
		gint xc,gint yc,gint rayon)
{
	GdkGCValues values;
	GdkColor color;
	double r,g,b;
	GdkColormap *colormap;
	if(!cr) return;
	if(!gc) return;
	cairo_save (cr); /* stack-pen-size */
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
   	colormap  = gdk_drawable_get_colormap(parent->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);
	r = SCALE(color.red);
	g = SCALE(color.green);
	b = SCALE(color.blue);
	cairo_set_source_rgba (cr, r, g, b, 1.0);

	cairo_set_line_width (cr, values.line_width);
	cairo_arc (cr, xc, yc, rayon, 0, 2 * M_PI);
	if(values.fill==GDK_SOLID) cairo_fill (cr);
	cairo_stroke (cr);
	cairo_restore (cr); /* stack-pen-size */
}
/*****************************************************************************/
void gabedit_cairo_line_gradient(cairo_t *cr,  GtkWidget* parent, GdkGC* gc, GdkColor color1,  GdkColor color2, gdouble x1,gdouble y1,gdouble x2,gdouble y2)
{
	GdkGCValues values;
	double r1,g1,b1;
	double r2,g2,b2;
	cairo_pattern_t *pat;

	if(!cr) return;
	if(!gc) return;
	cairo_save (cr); /* stack-pen-size */
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


	r1 = SCALE(color1.red);
	g1 = SCALE(color1.green);
	b1 = SCALE(color1.blue);

	r2 = SCALE(color2.red);
	g2 = SCALE(color2.green);
	b2 = SCALE(color2.blue);

	pat = cairo_pattern_create_linear (x1,y1, x2,y2);
	cairo_pattern_add_color_stop_rgba (pat, 0, r1, g1, b1, 1);
	cairo_pattern_add_color_stop_rgba (pat, 0.5, r2, g2, b2, 1);
	cairo_set_source (cr, pat);

	cairo_set_line_width (cr, values.line_width);
	cairo_move_to (cr, x1, y1);
	cairo_line_to (cr, x2, y2);
	cairo_stroke (cr);
	cairo_restore (cr); /* stack-pen-size */
	cairo_pattern_destroy (pat);
}
/*****************************************************************************/
void gabedit_cairo_cercle_gradient(cairo_t *cr,  GtkWidget* parent, GdkGC* gc,
		gint xc,gint yc,gint rayon)
{
	GdkGCValues values;
	GdkColor color;
	double r,g,b;
	GdkColormap *colormap;
	cairo_pattern_t *pat;
	if(!cr) return;
	if(!gc) return;
	cairo_save (cr); /* stack-pen-size */
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
   	colormap  = gdk_drawable_get_colormap(parent->window);
        gdk_colormap_query_color(colormap, values.foreground.pixel,&color);
	r = SCALE(color.red);
	g = SCALE(color.green);
	b = SCALE(color.blue);
	pat = cairo_pattern_create_radial (xc+rayon/2, yc, rayon/4, xc,  yc, rayon);
	cairo_pattern_add_color_stop_rgba (pat, 0, 1, 1, 1, 1);
	cairo_pattern_add_color_stop_rgba (pat, 1, r, g, b, 1);
	cairo_set_source (cr, pat);
	cairo_set_line_width (cr, values.line_width);
	cairo_arc (cr, xc, yc, rayon, 0, 2 * M_PI);
	if(values.fill==GDK_SOLID) cairo_fill (cr);
	cairo_stroke (cr);
	cairo_restore (cr); /* stack-pen-size */
	cairo_pattern_destroy (pat);
}
