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

#ifndef __GABEDIT_UTILSGL_H__
#define __GABEDIT_UTILSGL_H__

#include "Vector3d.h"
#include "Transformation.h"

void glGetWorldCoords(gint x, gint y, gint height, gdouble *w);
void glGetWindowCoords(gdouble *w, gint height, gint *x);
gint glTextWidth(gchar *str);
gint glTextHeight();
/*
void glPrintWinOld(gint x, gint y, gint height, gchar *str);
void glPrintOld(gdouble x, gdouble y, gdouble z, gchar *str);
void glPrintOrthoOld(gdouble x, gdouble y, gdouble z, gchar *str, gboolean centerX, gboolean centerY);
void glInitFontsUsingOld(gchar* fontname);
void glInitFontsOld();
void glPrintScaleOld(gdouble x, gdouble y, gdouble z, gdouble scale, gchar *str);
*/
void getNormalPlanWindow(gdouble N[]);
gdouble get_alpha_opacity();
void set_alpha_opacity(gdouble a);
OpenGLOptions get_opengl_options();
void glMaterialdv(GLenum face, GLenum pname, const GLdouble*  	params);
void glLightdv(GLenum face, GLenum pname, const GLdouble* params);
void glFogdv(GLenum pname, const GLdouble* params);
void mYPerspective( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar );

void Sphere_Draw_Precision(GLdouble radius,V3d position, GLint numberOfSubdivisions);
void Sphere_Draw_Color_Precision(GLdouble radius,V3d position, V4d Specular,V4d Diffuse,V4d Ambiant, GLint numberOfSubdivisions);
void Sphere_Draw(GLdouble radius,V3d position);
void Sphere_Draw_Color(GLdouble radius,V3d position, V4d Specular,V4d Diffuse,V4d Ambiant);
GLuint Sphere_Get_List(int i,GLdouble radius,V4d Specular,V4d Diffuse,V4d Ambiant);
void Sphere_Draw_From_List(GLuint sphere,V3d position);

void Cylinder_Draw(GLdouble radius,V3d Base1Pos,V3d Base2Pos);
void Cylinder_Draw_Color(GLdouble radius,V3d Base1Pos,V3d Base2Pos, V4d Specular,V4d Diffuse,V4d Ambiant);
void Cylinder_Draw_Color_Two(GLdouble radius,V3d Base1Pos,V3d Base2Pos,
			 V4d Specular1,V4d Diffuse1,V4d Ambiant1,
			 V4d Specular2,V4d Diffuse2,V4d Ambiant2,
			GLdouble p1, GLdouble p2);
void Prism_Draw(GLdouble radius,V3d Base1Pos,V3d Base2Pos);
void Prism_Draw_Color(GLdouble radius,V3d Base1Pos,V3d Base2Pos, V4d Specular,V4d Diffuse,V4d Ambiant);
void Draw_Arrow(V3d vector, GLdouble radius,V3d origin, V4d specular,V4d diffuse,V4d ambiant, gboolean negative);
void glDeleteFontsList();

void glInitFontsUsing(gchar* fontname, PangoContext* *pft2_context);
void glInitFonts(PangoContext* *pft2_context);
void glPrintWin(gint x, gint y, gint height, gchar *str, PangoContext* ft2_context);
void glPrint(gdouble x, gdouble y, gdouble z, gchar *str, PangoContext* ft2_context);
void glPrintOrtho(gdouble x, gdouble y, gdouble z, gchar *str, gboolean centerX, gboolean centerY, PangoContext *ft2_context);
void glPrintScale(gdouble x, gdouble y, gdouble z, gdouble scale, gchar *str, PangoContext *ft2_context);


#endif /* __GABEDIT_UTILSGL_H__ */

