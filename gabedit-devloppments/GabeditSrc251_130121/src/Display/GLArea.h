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

#ifndef __GABEDIT_GLAREA_H__
#define __GABEDIT_GLAREA_H__

void resetBeginNegative();
gdouble getScaleBall();
gdouble getScaleStick();
gboolean getShowOneSurface();
gboolean getShowBox();
void setShowBox(gboolean c);
void setScaleBall(gdouble a);
void setScaleStick(gdouble a);
void setShowOneSurface(gboolean a);
gint getOptCol();
void setOptCol(gint i);
void build_rings(gint size, gboolean showMessage);
void delete_rings_all();
gboolean get_show_symbols();
gboolean get_show_numbers();
void set_show_symbols(gboolean ac);
void set_show_numbers(gboolean ac);
gint get_background_color(guchar color[]);
void getQuat(gdouble q[]);
gboolean  sourceIsOn(gint numsSource);
void  setAnimateContours(gboolean anim);
void  setAnimatePlanesMapped(gboolean anim);
void  get_orgin_molecule(gdouble orig[]);
void  get_camera_values(gdouble* zn, gdouble* zf, gdouble* angle, gdouble* aspect, gboolean* persp);
void set_camera_values(gdouble zn, gdouble zf, gdouble zo, gboolean persp);
gboolean get_light(gint num,gdouble v[]);
gchar**  get_light_position(gint num);
void set_light_position(gint num,gdouble v[]);
void set_default_operation();
void set_operation_type(gint i);
void set_light_on_off(gint i);
void add_void_maps();
void add_maps(gint ii0, gint ii1, gint inumplan, gdouble igap, gboolean newGrid);
void set_contours_values_from_plane(gdouble minv,gdouble maxv,gint N,gdouble gap, gboolean linear);
void add_void_contours();
void set_contours_values(gint N,gdouble* cvalues,gint ii0,gint ii1,gint inumplan,gdouble gap);
void add_objects_for_new_grid();
void add_surface();
void set_background_optcolor(gint i);
gboolean NewGLArea(GtkWidget *widget);
void Define_Iso(gdouble isovale);
void Define_Grid();
gint glarea_rafresh(GtkWidget *widget);
void free_grid_all();
void free_iso_all();
void free_surfaces_all();
void free_planes_mapped_all();
void free_contours_all();
void free_objects_all();
void rafresh_window_orb();
void rotationAboutAnAxis(GtkWidget *widget, gdouble phi, gint axe);
gint redrawGL2PS();

#endif /* __GABEDIT_GLAREA_H__ */

