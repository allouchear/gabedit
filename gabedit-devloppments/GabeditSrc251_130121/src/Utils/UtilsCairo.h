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

#ifndef __GABEDIT_UTILSCAIRO_H__
#define __GABEDIT_UTILSCAIRO_H__

void gabedit_cairo_string(cairo_t* cr, GtkWidget* parent, PangoFontDescription *font_desc, GdkGC* gc , gint x, gint y, G_CONST_RETURN gchar* str, gboolean centerX, gboolean centerY);
void gabedit_cairo_triangle(cairo_t *cr,  GtkWidget* parent, GdkGC* gc,
		gdouble x1,gdouble y1,gdouble x2,gdouble y2, gdouble x3,gdouble y3);
void gabedit_cairo_line(cairo_t *cr,  GtkWidget* parent, GdkGC* gc, gdouble x1,gdouble y1,gdouble x2,gdouble y2);
void gabedit_cairo_arc(cairo_t *cr,  GtkWidget* parent, GdkGC* gc, gint xc,gint yc,gint rayon, gdouble angle1, gdouble angle2 , gdouble scale1, gdouble scale2);
void gabedit_cairo_cercle(cairo_t *cr,  GtkWidget* parent, GdkGC* gc, gint xc,gint yc,gint rayon);
void gabedit_cairo_line_gradient(cairo_t *cr,  GtkWidget* parent, GdkGC* gc, GdkColor color1,  GdkColor color2, gdouble x1,gdouble y1,gdouble x2,gdouble y2);
void gabedit_cairo_cercle_gradient(cairo_t *cr,  GtkWidget* parent, GdkGC* gc, gint xc,gint yc,gint rayon);

#endif /* __GABEDIT_UTILSCAIRO_H__ */

