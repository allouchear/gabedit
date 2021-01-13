/* GabeditContoursPlot.h */
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

#ifndef __GABEDIT_GABEDITContoursPLOT_H__
#define __GABEDIT_GABEDITContoursPLOT_H__



#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>

G_BEGIN_DECLS

typedef enum
{
  GABEDIT_ContoursPLOT_HMAJOR_GRID,
  GABEDIT_ContoursPLOT_HMINOR_GRID,
  GABEDIT_ContoursPLOT_VMAJOR_GRID,
  GABEDIT_ContoursPLOT_VMINOR_GRID
} GabeditContoursPlotGrid;

typedef enum
{
  GABEDIT_ContoursPLOT_CONV_NONE,
  GABEDIT_ContoursPLOT_CONV_LORENTZ,
  GABEDIT_ContoursPLOT_CONV_GAUSS
} GabeditContoursPlotConvType;


#define GABEDIT_TYPE_ContoursPLOT                 (gabedit_contoursplot_get_type ())
#define GABEDIT_ContoursPLOT(obj)                 (G_TYPE_CHECK_INSTANCE_CAST ((obj), GABEDIT_TYPE_ContoursPLOT, GabeditContoursPlot))
#define GABEDIT_ContoursPLOT_CLASS(klass)         (G_TYPE_CHECK_CLASS_CAST ((klass), GABEDIT_TYPE_ContoursPLOT, GabeditContoursPlotClass))
#define GABEDIT_IS_ContoursPLOT(obj)              (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GABEDIT_TYPE_ContoursPLOT))
#define GABEDIT_IS_ContoursPLOT_CLASS(klass)      (G_TYPE_CHECK_CLASS_TYPE ((klass), GABEDIT_TYPE_ContoursPLOT))
#define GABEDIT_ContoursPLOT_GET_CLASS(obj)       (G_TYPE_INSTANCE_GET_CLASS ((obj), GABEDIT_TYPE_ContoursPLOT, GabeditContoursPlotClass))

typedef struct 
{
	gint xi;
	gint yi;
	gint width;
	gint height;
	gdouble x;
	gdouble y;
	gdouble angle;
	gchar* str;
	PangoLayout* pango;
}ContoursPlotObjectText;

typedef struct 
{
	gint x1i;
	gint y1i;
	gint x2i;
	gint y2i;
	gdouble x1;
	gdouble y1;
	gdouble x2;
	gdouble y2;
  	gint width;
  	gint arrow_size;
  	GdkColor color;
  	GdkLineStyle style;
}ContoursPlotObjectLine;

typedef struct 
{
	gdouble x; /* realtive x : 0(left) 1(right) window*/
	gdouble y;
	gdouble width;
	gdouble height;
	gint xi;
	gint yi;
	gint widthi;
	gint heighti;
	gchar* fileName;
	cairo_surface_t *image;
}ContoursPlotObjectImage;

typedef struct 
{
	gdouble value;
	gint size;
	gdouble* x;
	gdouble* y;
	gint* index;
  	gchar point_str[100];
  	PangoLayout* point_pango;
  	gint point_size;
  	gint point_width, point_height;
  	gint line_width;
  	GdkColor point_color, line_color;
  	GdkLineStyle line_style;
	gchar* label;
	gdouble xlabel;
	gdouble ylabel;
}ContourData;

typedef struct 
{
  gdouble *zValues;
  gint xsize;
  gint ysize;
  gdouble xmin, xmax;
  gdouble ymin, ymax;
  gdouble zmin, zmax;
  gint nContours;
  ContourData* contours;
}ContoursPlotData;

typedef struct _OneColorValue
{
	gdouble color[3];
	gdouble value;
}OneColorValue;

typedef struct 
{
	gint numberOfColors;
	OneColorValue* colorValue;
}ColorMap;

typedef struct 
{
	gint type;
	gdouble colors[3][3];
	ColorMap* colorMap;
}ContoursColorMap;

typedef struct _GabeditContoursPlot        GabeditContoursPlot;
typedef struct _GabeditContoursPlotClass   GabeditContoursPlotClass;

struct _GabeditContoursPlot
{
  GtkWidget widget;
  gdouble xmin, xmax, ymin, ymax; 

  gboolean show_label_contours;
  
  ContoursColorMap colorsMap;
  GdkPixmap *plotting_area; 
  GdkPixmap *old_area; 
  cairo_t *cairo_widget; 
  cairo_t *cairo_area; 
  cairo_t *cairo_export; 
  GdkRectangle plotting_rect;
  gint x_legends_digits; 
  gint y_legends_digits; 

  gint length_ticks; 
  gint hmajor_ticks, hminor_ticks; 
  gint vmajor_ticks, vminor_ticks; 
  gdouble d_hmajor, d_hminor; 
  gdouble d_vmajor, d_vminor; 

  gint left_margins, right_margins, top_margins, bottom_margins;
  
  gdouble d_hlegend, d_vlegend; 
  
  GdkGC *back_gc; 
  GdkGC *fore_gc; 
  
  PangoLayout **h_legends;
  PangoLayout **v_legends;

  gchar** h_legends_str;
  gchar** v_legends_str;
  
  gint x_legends_width;
  gint x_legends_height;
  gint y_legends_width;
  gint y_legends_height;

  gchar *h_label_str;
  gchar *v_label_str;
  PangoLayout *h_label;
  PangoLayout *v_label;
  gint h_label_width;
  gint h_label_height;
  gint v_label_width;
  gint v_label_height;

  gint colormap_width;
  gint colormap_height;
  gint colormap_nlegends;
  PangoLayout **colormap_legends;
  gchar** colormap_legends_str;
  gboolean show_colormap;
  
  GdkGC *hmajor_grid_gc;
  GdkGC *hminor_grid_gc;
  GdkGC *vmajor_grid_gc;
  GdkGC *vminor_grid_gc;

  gboolean hmajor_grid;
  gboolean hminor_grid;
  gboolean vmajor_grid;
  gboolean vminor_grid; 

  gboolean show_left_legends;
  gboolean show_right_legends;
  gboolean show_top_legends;
  gboolean show_bottom_legends;

  gboolean reflect_x;
  gboolean reflect_y;

  GList *data_list;
  
  GdkGC *data_gc;
  GdkGC *lines_gc;

  gint mouse_button; 
  
  gboolean mouse_zoom_enabled;
  gboolean mouse_distance_enabled;
  
  gint mouse_zoom_button;
  gint mouse_distance_button;
  
  GdkPoint zoom_point;
  GdkPoint distance_point;
  GdkPoint object_begin_point;
  GdkPoint object_end_point;
  
  GdkRectangle zoom_rect;
  GdkRectangle distance_rect;

  gboolean mouse_displace_enabled;
  gint mouse_displace_button;
  GdkPoint move_point;

  gboolean wheel_zoom_enabled;
  gdouble wheel_zoom_factor;

  gboolean mouse_autorange_enabled;
  gint mouse_autorange_button;
  gint font_size;
  gboolean double_click;

  gboolean shift_key_pressed;
  gboolean control_key_pressed;

  gboolean dashed_negative_contours;

  gboolean d_key_pressed;

  gint nObjectsText;
  ContoursPlotObjectText* objectsText;
  gint selected_objects_text_num;
  gboolean t_key_pressed;

  gint nObjectsLine;
  ContoursPlotObjectLine* objectsLine;
  gint selected_objects_line_num;
  gint selected_objects_line_type;
  gboolean l_key_pressed;

  gint nObjectsImage;
  ContoursPlotObjectImage* objectsImage;
  gint selected_objects_image_num;
  gboolean i_key_pressed;

  gboolean r_key_pressed;
};

struct _GabeditContoursPlotClass
{
  GtkWidgetClass parent_class;
};

GtkWidget*     gabedit_contoursplot_new ();
GType          gabedit_contoursplot_get_type ();
void gabedit_contoursplot_set_range (GabeditContoursPlot *contoursplot, gdouble xmin, gdouble xmax, gdouble ymin, gdouble ymax);    
void gabedit_contoursplot_set_range_xmin (GabeditContoursPlot *contoursplot, gdouble xmin);
void gabedit_contoursplot_set_range_xmax (GabeditContoursPlot *contoursplot, gdouble xmax);
void gabedit_contoursplot_set_range_ymin (GabeditContoursPlot *contoursplot, gdouble ymin);
void gabedit_contoursplot_set_range_ymax (GabeditContoursPlot *contoursplot, gdouble ymax);
void gabedit_contoursplot_set_autorange (GabeditContoursPlot *contoursplot, ContoursPlotData *data);    
void gabedit_contoursplot_get_range (GabeditContoursPlot *contoursplot, gdouble *xmin, gdouble *xmax, gdouble *ymin, gdouble *ymax);
gboolean gabedit_contoursplot_get_point (GabeditContoursPlot *contoursplot, gint x, gint y, gdouble *xv, gdouble *yv);
gboolean gabedit_contoursplot_get_point_control(GabeditContoursPlot *contoursplot, gint x, gint y, gint width, gint height, gdouble angle, gdouble *xv, gdouble *yv );
void gabedit_contoursplot_set_ticks (GabeditContoursPlot *contoursplot, gint hmajor, gint hminor, gint vmajor, gint vminor, gint length);
void gabedit_contoursplot_set_ticks_hmajor (GabeditContoursPlot *contoursplot, gint hmajor);
void gabedit_contoursplot_set_ticks_hminor (GabeditContoursPlot *contoursplot, gint hminor);
void gabedit_contoursplot_set_ticks_vmajor (GabeditContoursPlot *contoursplot, gint vmajor);
void gabedit_contoursplot_set_ticks_vminor (GabeditContoursPlot *contoursplot, gint vminor);
void gabedit_contoursplot_set_ticks_length (GabeditContoursPlot *contoursplot, gint length);
void gabedit_contoursplot_get_ticks (GabeditContoursPlot *contoursplot, gint *hmajor, gint *hminor, gint *vmajor, gint *vminor, gint* length);
void gabedit_contoursplot_set_margins_left (GabeditContoursPlot *contoursplot, gint left);
void gabedit_contoursplot_set_margins_right (GabeditContoursPlot *contoursplot, gint right);
void gabedit_contoursplot_set_margins_top (GabeditContoursPlot *contoursplot, gint top);
void gabedit_contoursplot_set_margins_bottom (GabeditContoursPlot *contoursplot, gint bottom);
void gabedit_contoursplot_set_x_legends_digits (GabeditContoursPlot *contoursplot, gint digits);
void gabedit_contoursplot_set_y_legends_digits (GabeditContoursPlot *contoursplot, gint digits);
gint gabedit_contoursplot_get_x_legends_digits (GabeditContoursPlot *contoursplot);
gint gabedit_contoursplot_get_y_legends_digits (GabeditContoursPlot *contoursplot);
void gabedit_contoursplot_set_background_color (GabeditContoursPlot *contoursplot, GdkColor color); 
void gabedit_contoursplot_set_grids_attributes (GabeditContoursPlot *contoursplot, GabeditContoursPlotGrid grid, GdkColor color, gint line_width, GdkLineStyle line_style);
void gabedit_contoursplot_get_grids_attributes (GabeditContoursPlot *contoursplot, GabeditContoursPlotGrid grid, GdkColor *color, gint *line_width, GdkLineStyle *line_style);
void gabedit_contoursplot_enable_grids (GabeditContoursPlot *contoursplot, GabeditContoursPlotGrid grid, gboolean enable);
void gabedit_contoursplot_add_data (GabeditContoursPlot *contoursplot, ContoursPlotData *data);
void gabedit_contoursplot_remove_data (GabeditContoursPlot *contoursplot, ContoursPlotData *data);
void gabedit_contoursplot_add_data_peaks(GabeditContoursPlot *contoursplot, gint numberOfPoints, gdouble* X,  gdouble* Y, GdkColor color);
void gabedit_contoursplot_configure_mouse_zoom (GabeditContoursPlot *contoursplot, gboolean enabled, gint button);
void gabedit_contoursplot_configure_mouse_distance (GabeditContoursPlot *contoursplot, gboolean enabled, gint button);
void gabedit_contoursplot_configure_wheel_zoom (GabeditContoursPlot *contoursplot, gboolean enabled, gdouble factor);
void gabedit_contoursplot_configure_mouse_displace (GabeditContoursPlot *contoursplot, gboolean enabled, gint button);
void gabedit_contoursplot_configure_mouse_autorange (GabeditContoursPlot *contoursplot, gboolean enabled, gint button);
void gabedit_contoursplot_save_image(GabeditContoursPlot *contoursplot, gchar *fileName, gchar* type);
void gabedit_contoursplot_reflect_x (GabeditContoursPlot *contoursplot, gboolean reflection);
void gabedit_contoursplot_reflect_y (GabeditContoursPlot *contoursplot, gboolean reflection);
void gabedit_contoursplot_show_left_legends (GabeditContoursPlot *contoursplot, gboolean show);
void gabedit_contoursplot_show_right_legends (GabeditContoursPlot *contoursplot, gboolean show);
void gabedit_contoursplot_show_top_legends (GabeditContoursPlot *contoursplot, gboolean show);
void gabedit_contoursplot_show_bottom_legends (GabeditContoursPlot *contoursplot, gboolean show);
void gabedit_contoursplot_set_font (GabeditContoursPlot *contoursplot, gchar* fontName);
void gabedit_contoursplot_set_x_label (GabeditContoursPlot *contoursplot, G_CONST_RETURN gchar* str);
void gabedit_contoursplot_set_y_label (GabeditContoursPlot *contoursplot, G_CONST_RETURN gchar* str);
GtkWidget* gabedit_contoursplot_new_window(gchar* title, GtkWidget*parent);
void gabedit_contoursplot_help();

G_END_DECLS

#endif /* __GABEDIT_GABEDITContoursPLOT_H__ */
