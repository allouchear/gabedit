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

#ifndef __GABEDIT_LABELS_H__
#define  __GABEDIT_LABELS_H__

void init_labels_font();
gboolean get_labels_ortho();
void set_labels_ortho(gboolean o);
gboolean get_show_symbols();
void set_show_symbols(gboolean ac);
gboolean get_show_numbers();
void set_show_numbers(gboolean ac);
gboolean get_show_charges();
void set_show_charges(gboolean ac);
void showLabelSymbolsNumbersCharges(PangoContext *ft2_context);
gboolean get_show_distances();
void set_show_distances(gboolean ac);
void showLabelDistances(PangoContext *ft2_context);
gboolean get_show_dipole();
void set_show_dipole(gboolean ac);
void showLabelDipole(PangoContext *ft2_context);
gboolean get_show_axes();
void set_show_axes(gboolean ac);
void showLabelAxes(PangoContext *ft2_context);
void showLabelPrincipalAxes(PangoContext *ft2_context);
void showLabelTitle(gint width,gint height, PangoContext *ft2_context);
gchar* get_label_title();
void set_label_title(gchar* str, gint x, gint y);
void set_title_dlg();
#endif /*   __GABEDIT_LABELS_H__ */
