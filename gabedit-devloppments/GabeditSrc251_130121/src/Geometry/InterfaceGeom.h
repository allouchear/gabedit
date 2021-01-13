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

#ifndef __GABEDIT_INTERFACEGEOM_H__
#define __GABEDIT_INTERFACEGEOM_H__

void MessageGeom(gchar* message,gchar* type,gboolean center);
void create_geom_interface (GabEditTypeFileGeom readfile);
GtkWidget* create_geom(GtkWidget*win,GtkWidget *frame);
void geom_is_open();
void geom_is_not_open();
void destroy_geometry(GtkWidget *win);
void edit_geometry();
void create_units_option(GtkWidget *,gchar *);
void selc_all_input_file(gchar *data);
void create_geomXYZ_interface (GabEditTypeFileGeom readfile);
void read_geom_any_file_dlg();

#endif /* __GABEDIT_INTERFACEGEOM_H__ */

