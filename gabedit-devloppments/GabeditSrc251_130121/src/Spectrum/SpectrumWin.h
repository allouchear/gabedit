/* SpectrumWin.h */
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

#ifndef __GABEDIT_SPECTRUMWIN_H__
#define __GABEDIT_SPECTRUMWIN_H__



#include <gdk/gdk.h>
#include <gtk/gtkwidget.h>
#include "../Utils/GabeditXYPlot.h"

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
  guint point_size;
  guint line_width;
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
  gdouble scaleY;
  gdouble shiftX;
  gboolean ymaxToOne;
}XYPlotWinData;

void spectrum_win_add_data(GtkWidget *winSpectrum, gint size, gdouble* x, gdouble* y);
void spectrum_win_remove_data(GtkWidget *winSpectrum);
void spectrum_win_autorange(GtkWidget *winSpectrum);
GtkWidget*  spectrum_win_new (gchar* title);
GtkWidget* spectrum_win_new_with_xy(gchar* title,  gint size, gdouble* x, gdouble* y);
void spectrum_win_relect_x(GtkWidget* winSpectrum, gboolean active);
void spectrum_win_relect_y(GtkWidget* winSpectrum, gboolean active);
void spectrum_win_set_half_width(GtkWidget* winSpectrum, gdouble value);
void spectrum_win_set_xmin(GtkWidget* winSpectrum, gdouble value);
void spectrum_win_set_xmax(GtkWidget* winSpectrum, gdouble value);
void spectrum_win_set_ymin(GtkWidget* winSpectrum, gdouble value);
void spectrum_win_set_ymax(GtkWidget* winSpectrum, gdouble value);
void spectrum_win_reset_ymin_ymax(GtkWidget *xyplot);
void spectrum_win_set_xlabel(GtkWidget* winSpectrum, gchar* label);
void spectrum_win_set_ylabel(GtkWidget* winSpectrum, gchar* label);
XYPlotData* spectrum_win_get_dataCurve(GtkWidget *winSpectrum);

#endif /* __GABEDIT_SPECTRUMWIN_H__ */
