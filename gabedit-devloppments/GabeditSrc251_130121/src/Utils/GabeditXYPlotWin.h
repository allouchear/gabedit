/* GabeditXYPlotWin.h */
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

#ifndef __GABEDIT_GABEDITXYPLOTWIN_H__
#define __GABEDIT_GABEDITXYPLOTWIN_H__



#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include "GabeditXYPlot.h"

typedef enum
{
  GABEDIT_CONV_TYPE_NONE,
  GABEDIT_CONV_TYPE_LORENTZ,
  GABEDIT_CONV_TYPE_GAUSS,
} GabeditConvType;

typedef struct 
{
  gdouble *x, *y;
  guint size;
  GabeditXYPlotDataStyle style;
  guint point_width, line_width;
  GdkColor point_color, line_color;
  GdkLineStyle line_style;
  XYPlotData* dataPeaks;
  XYPlotData* dataCurve;
  gboolean showDataPeaks;
  gboolean showDataCurve;
  gdouble xmin;
  gdouble xmax;
  gdouble ymin;
  gdouble ymax;
  gdouble halfWidth;
  GabeditConvType convType;
  gdouble scaleX;
}XYPlotWinData;

void gabedit_xyplotwin_add_data(GtkWidget *winXYPlot, gint size, gdouble* x, gdouble* y);
void gabedit_xyplotwin_autorange(GtkWidget *winXYPlot);
GtkWidget*  gabedit_xyplotwin_new (gchar* title);
GtkWidget* gabedit_xyplotwin_new_with_xy(gchar* title,  gint size, gdouble* x, gdouble* y);
void gabedit_xyplotwin_relect_x(GtkWidget* window, gboolean active);
void gabedit_xyplotwin_relect_y(GtkWidget* window, gboolean active);
void gabedit_xyplotwin_set_half_width(GtkWidget* window, gdouble value);
void gabedit_xyplotwin_set_xmin(GtkWidget* window, gdouble value);
void gabedit_xyplotwin_set_xmax(GtkWidget* window, gdouble value);
void gabedit_xyplotwin_set_ymin(GtkWidget* window, gdouble value);
void gabedit_xyplotwin_set_ymax(GtkWidget* window, gdouble value);
GtkWidget*  gabedit_xyplotwin_new_testing (gchar* title);

#endif /* __GABEDIT_GABEDITXYPLOTWIN_H__ */
