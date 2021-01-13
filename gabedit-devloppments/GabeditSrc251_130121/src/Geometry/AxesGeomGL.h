
#ifndef __GABEDIT_AXISGEOMGL_H__
#define __GABEDIT_AXISGEOMGL_H__

#ifdef DRAWGEOMGL
void getAxesGeomProperties(gboolean* show, gboolean* negative, gdouble origin[], gdouble* radius, gdouble* scal, gdouble xColor[], gdouble yColor[], gdouble zColor[]);
void initAxesGeom();
void save_axes_geom_properties();
void read_axes_geom_properties();
void set_axes_geom_dialog ();
gboolean testShowAxesGeom();
void showAxesGeom();
void hideAxesGeom();
void gl_build_axes(gdouble* position);
void showLabelAxesGeom(gboolean ortho, gdouble* position, PangoContext *ft2_context);
#endif

#endif /* __GABEDIT_AXISGEOMGL_H__ */

