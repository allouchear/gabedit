/* UtilsGL.c */
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
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <gtk/gtk.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <gtk/gtkgl.h>

#include <pango/pangoft2.h>

/*
#ifndef G_OS_WIN32
#include <GL/glx.h>
#include <pango/pangox.h>
#include <gdk/gdkx.h>
#else
#include <pango/pangowin32.h>
#endif
*/


#include "../Common/Global.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/Constants.h"
#include "UtilsGL.h"
#include "../../gl2ps/gl2ps.h"
/* transformation/projection matrices */
static GLint viewport[4];
static GLdouble mvmatrix[16];
static GLdouble projmatrix[16];
static gint glFontsize=10;
static gint fontOffset=-1;
static gint charWidth=0;
static gint charHeight=0;

/*********************************************************************************************/
/* window to real space conversion primitive */
void glGetWorldCoords(gint x, gint y, gint height, gdouble *w)
{
	gint i;
	GLdouble r[3];
	GLfloat winX, winY, winZ;

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	winZ = 0.0;
	/*glReadPixels( x, (gint)(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );*/

	gluUnProject( winX, winY, winZ, mvmatrix, projmatrix, viewport, &r[0], &r[1], &r[2]);

	for(i=0;i<3;i++) w[i] = r[i];
}
/*********************************************************************************************/
/* real space to window conversion primitive */
void glGetWindowCoords(gdouble *w, gint height, gint *x)
{
	GLdouble r[3];

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

	gluProject(w[0], w[1], w[2], mvmatrix, projmatrix, viewport, &r[0], &r[1], &r[2]);

	x[0] = r[0];
	x[1] = height - r[1] - 1;
}
/*********************************************************************************************/
gint glTextWidth(gchar *str)
{
	return(strlen(str) * glFontsize);
}
/*********************************************************************************************/
gint glTextHeight()
{
	if(charHeight>0) return charHeight;
	return(glFontsize);
}
/*********************************************************************************************/
/* print at a window position */
/*
void glPrintWinOld(gint x, gint y, gint height, gchar *str)
{
	gdouble w[3];

	glGetWorldCoords(x, y, height, w);
	glRasterPos3f(w[0], w[1], w[2]); 
	gl2psText(str, "Times-Roman", glFontsize);

	glListBase(fontOffset);
	glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
}
*/
/*********************************************************************************************/
/* print at a world position */
/*
void glPrintOld(gdouble x, gdouble y, gdouble z, gchar *str)
{
	glRasterPos3f(x,y,z); 
	gl2psText(str, "Times-Roman", glFontsize);

	glListBase(fontOffset);
	glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
}
*/
/*********************************************************************************************/
/*
void glPrintOrthoOld(gdouble x, gdouble y, gdouble z, gchar *str, gboolean centerX, gboolean centerY)
{
	gdouble w[] = {x, y, z};
	gint xy[] = {0, 0};
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetWindowCoords(w, viewport[3], xy);
	glPushMatrix();
	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); glLoadIdentity();
	gluOrtho2D(0, viewport[2], 0, viewport[3]);

	
	if(centerX) xy[0] -= charWidth*strlen(str)/2;
	if(centerY) xy[1] += charHeight/4;
	glPrintWinOld(xy[0], xy[1], viewport[3], str);
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	glPopMatrix();
}
*/
/*********************************************************************************************/
void glDeleteFontsList()
{
	glDeleteLists(fontOffset,256);
}
/*********************************************************************************************/
/*********************************************************************************************/
/* pango fonts for OpenGL  */
/*
void glInitFontsOld()
{
	glInitFontsUsingOld(FontsStyleLabel.fontname);
}
*/
/*********************************************************************************************/
/* pango fonts for OpenGL  */
void glInitFonts(PangoContext* *pft2_context)
{
	/*if (fontOffset >=0) return;*/
	glInitFontsUsing(FontsStyleLabel.fontname, pft2_context);
}
/*********************************************************************************************/
/* get a World coordinates from scene coordinates */
void glGetWorldCoordsFromSceneCoords(gdouble VScene[], gdouble VWorld[])
{
	GLdouble mvMatrix[4][4];
	gdouble** trMatrix;
	gdouble** invMatrix;
	gint i;
	gint j;
	glGetDoublev(GL_MODELVIEW_MATRIX, &mvMatrix[0][0]);
	trMatrix = g_malloc(3*sizeof(gdouble*));
	for(i=0;i<3;i++) trMatrix[i] = g_malloc(3*sizeof(gdouble));

	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			trMatrix[j][i] = mvMatrix[i][j];

	invMatrix = Inverse3(trMatrix);
	for(i=0;i<3;i++) g_free(trMatrix[i]);
	g_free(trMatrix);

	if(invMatrix != NULL)
	{
		for(i=0;i<3;i++)
		{
			VWorld[i] = 0;
			for(j=0;j<3;j++)
			{
				VWorld[i] += invMatrix[i][j]*VScene[j];
			}
		}
		for(i=0;i<3;i++) g_free(invMatrix[i]);
		g_free(invMatrix);
	}
	else
	{
		for(i=0;i<3;i++) VWorld[i] = 0;
		VWorld[2] = 1;
	}
}
/*********************************************************************************************/
/* get a normal vector to plan of window */
void getNormalPlanWindow(gdouble N[])
{
	gdouble VScene[3]={0,0,1};

	glGetWorldCoordsFromSceneCoords(VScene,N);

}
/*********************************************************************************************/
/* print at a world position  after scaling of scal in scene coordinates*/
/*
void glPrintScaleOld(gdouble x, gdouble y, gdouble z, gdouble scale, gchar *str)
{
	gdouble VScene[]={0,0,0};
	gdouble VWorld[]={x,y,z};
	GLdouble mvMatrix[4][4];
	gdouble** trMatrix;
	gdouble** invMatrix;
	gint i;
	gint j;
	glGetDoublev(GL_MODELVIEW_MATRIX, &mvMatrix[0][0]);
	trMatrix = g_malloc(3*sizeof(gdouble*));
	for(i=0;i<3;i++) trMatrix[i] = g_malloc(3*sizeof(gdouble));

	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			trMatrix[j][i] = mvMatrix[i][j];

	for(i=0;i<3;i++)
	{
		VScene[i] = 0;
		for(j=0;j<3;j++)
		{
			VScene[i] += trMatrix[i][j]*VWorld[j];
		}
	}
	VScene[2] += scale;

	invMatrix = Inverse3(trMatrix);
	for(i=0;i<3;i++) g_free(trMatrix[i]);
	g_free(trMatrix);

	if(invMatrix != NULL)
	{
		for(i=0;i<3;i++)
		{
			VWorld[i] = 0;
			for(j=0;j<3;j++)
			{
				VWorld[i] += invMatrix[i][j]*VScene[j];
			}
		}
		for(i=0;i<3;i++) g_free(invMatrix[i]);
		g_free(invMatrix);
	}
	else
	{
		VWorld[0] = x;
		VWorld[1] = y;
		VWorld[2] = z;
	}
	glRasterPos3f(VWorld[0],VWorld[1],VWorld[2]); 
	gl2psText(str, "Times-Roman", glFontsize);
	glListBase(fontOffset);
	glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);
}
*/
/*************************************************************************************/
OpenGLOptions get_opengl_options()
{
	return openGLOptions;
}
/**********************************************/
void glMaterialdv(GLenum face, GLenum pname, const GLdouble*  	params)
{
	GLfloat p[4] = {params[0],params[1],params[2], params[3]};
	glMaterialfv(face, pname, p);
}
/**********************************************/
void glLightdv(GLenum face, GLenum pname, const GLdouble* params)
{
	GLfloat p[4] = {params[0],params[1],params[2], params[3]};
	glLightfv(face, pname, p);
}

/**********************************************/
void glFogdv(GLenum pname, const GLdouble* params)
{
	GLfloat p[4] = {params[0],params[1],params[2], params[3]};
	glFogfv(pname, p);
}
/*********************************************************************************************/
void mYPerspective( GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
	GLdouble xmin, xmax, ymin, ymax;

	ymax = zNear * tan( fovy * PI / 360.0 );
	ymin = -ymax;

	xmin = ymin * aspect;
	xmax = ymax * aspect;

	glFrustum( xmin, xmax, ymin, ymax, zNear, zFar );
}
/* Sphere.c */
/************************************************************************************************************/
void Sphere_Draw_Precision(GLdouble radius,V3d position, GLint numberOfSubdivisions)
{
	GLUquadricObj *obj;
	
	glPushMatrix();
	glTranslated(position[0],position[1],position[2]);
    	obj = gluNewQuadric();
	gluQuadricNormals(obj, GL_SMOOTH);
	gluQuadricDrawStyle(obj, GLU_FILL);
	gluSphere(obj, radius, numberOfSubdivisions*2, numberOfSubdivisions);

	gluDeleteQuadric(obj);
	glTranslated(-position[0],-position[1],-position[2]);
	glPopMatrix(); 
}
/************************************************************************************************************/
void Sphere_Draw_Color_Precision(GLdouble radius,V3d position, V4d Specular,V4d Diffuse,V4d Ambiant, GLint numberOfSubdivisions)
{
	static GLdouble emission[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
	glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
	glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant);
	glMaterialdv(GL_FRONT, GL_EMISSION, emission);
	glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,100);
	Sphere_Draw_Precision(radius,position, numberOfSubdivisions);
}
/************************************************************************************************************/
void Sphere_Draw(GLdouble radius,V3d position)
{
	GLUquadricObj *obj;
	OpenGLOptions openGLOptions = get_opengl_options();
	
	glPushMatrix();
	glTranslated(position[0],position[1],position[2]);
    	obj = gluNewQuadric();
	gluQuadricNormals(obj, GL_SMOOTH);
	gluQuadricDrawStyle(obj, GLU_FILL);
	gluSphere(obj, radius, (GLint)openGLOptions.numberOfSubdivisionsSphere, (GLint)openGLOptions.numberOfSubdivisionsSphere);

	gluDeleteQuadric(obj);
	glTranslated(-position[0],-position[1],-position[2]);
	glPopMatrix(); 
}
/************************************************************************************************************/
void Sphere_Draw_Color(GLdouble radius,V3d position, V4d Specular,V4d Diffuse,V4d Ambiant)
{
	static GLdouble emission[] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
	glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
	glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant);
	glMaterialdv(GL_FRONT, GL_EMISSION, emission);
	glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,100);
	Sphere_Draw(radius,position);

}
/************************************************************************************************************/
GLuint Sphere_Get_List(int i,GLdouble radius,V4d Specular,V4d Diffuse,V4d Ambiant)
{
	
	GLuint sphere;
	V3d position={0,0,0};
	
	
    sphere = glGenLists(i);
	glNewList(sphere, GL_COMPILE);
	Sphere_Draw_Color(radius,position,Specular,Diffuse,Ambiant);
	glEndList();
	return sphere;
}
/************************************************************************************************************/
void Sphere_Draw_From_List(GLuint sphere,V3d position)
{
	glPushMatrix();
	glTranslated(position[0],position[1],position[2]);
	if (glIsList(sphere) == GL_TRUE) 
		glCallList(sphere);
	else
		printf("list error Sphere.c file\n");

	
	glPopMatrix();
}
/* Cylinder */
/************************************************************************/
void rotated_vector(V3d v)
{
	V3d vz={0.0,0.0,1.0};
	V3d	vert;
	gdouble angle;


	v3d_cross(vz,v,vert);
	angle = acos(v3d_dot(vz,v)/v3d_length(v))*RAD_TO_DEG;
	  
	if(fabs(angle)<1e-6)
		return;
	if(fabs(angle-180)<1e-6)
		glRotated(angle, 1.0, 0.0, 0.0);
	else
	glRotated(angle, vert[0],vert[1],vert[2]);

}
/************************************************************************/
void Cylinder_Draw(GLdouble radius,V3d Base1Pos,V3d Base2Pos)
{
		V3d Direction;
		OpenGLOptions openGLOptions = get_opengl_options();
		GLUquadricObj *obj;
		glPushMatrix();
		glTranslated(Base1Pos[0],Base1Pos[1],Base1Pos[2]);
		Direction[0] = Base2Pos[0]-Base1Pos[0];
		Direction[1] = Base2Pos[1]-Base1Pos[1];
		Direction[2] = Base2Pos[2]-Base1Pos[2];

		rotated_vector(Direction);
		obj = gluNewQuadric();
		gluQuadricNormals(obj, GL_SMOOTH);
		gluQuadricDrawStyle(obj, GLU_FILL);
		gluCylinder (obj,radius,radius,v3d_length(Direction),(GLint)openGLOptions.numberOfSubdivisionsCylindre,1);
		gluDeleteQuadric(obj);
		glPopMatrix(); 
}

/************************************************************************/
void Cylinder_Draw_Color(GLdouble radius,V3d Base1Pos,V3d Base2Pos,
			 V4d Specular,V4d Diffuse,V4d Ambiant)
{
	glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
	glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
	glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant);
	glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,50);
	Cylinder_Draw(radius,Base1Pos,Base2Pos);
}
/************************************************************************/
void Cylinder_Draw_Color_Two(GLdouble radius,V3d Base1Pos,V3d Base2Pos,
			 V4d Specular1,V4d Diffuse1,V4d Ambiant1,
			 V4d Specular2,V4d Diffuse2,V4d Ambiant2,
			GLdouble p1,GLdouble p2)
{
	V3d Center;
	GLdouble p = p1 + p2;
	Center[0] = (Base1Pos[0]*p2 + Base2Pos[0]*p1)/p;
	Center[1] = (Base1Pos[1]*p2 + Base2Pos[1]*p1)/p;
	Center[2] = (Base1Pos[2]*p2 + Base2Pos[2]*p1)/p;
	Cylinder_Draw_Color(radius,Base1Pos,Center,Specular1,Diffuse1,Ambiant1);
	Cylinder_Draw_Color(radius,Center,Base2Pos,Specular2,Diffuse2,Ambiant2);
}
/************************************************************************/
void Prism_Draw(GLdouble radius,V3d Base1Pos,V3d Base2Pos)
{
		V3d Direction;
		double lengt;
		GLUquadricObj *obj;
		glPushMatrix();
		glTranslated(Base1Pos[0],Base1Pos[1],Base1Pos[2]);
		Direction[0] = Base2Pos[0]-Base1Pos[0];
		Direction[1] = Base2Pos[1]-Base1Pos[1];
		Direction[2] = Base2Pos[2]-Base1Pos[2];
		lengt = v3d_length(Direction);

		rotated_vector(Direction);
		obj = gluNewQuadric();
		gluQuadricNormals(obj, GL_SMOOTH);
		gluQuadricDrawStyle(obj, GLU_FILL);
		gluCylinder (obj,radius,radius/5,lengt,10,10);
		gluDeleteQuadric(obj);
		glPopMatrix(); 
}

/************************************************************************/
void Prism_Draw_Color(GLdouble radius,V3d Base1Pos,V3d Base2Pos,
			 V4d Specular,V4d Diffuse,V4d Ambiant)
{
	glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
	glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
	glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant);
	glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,50);
	Prism_Draw(radius,Base1Pos,Base2Pos);
}
/***************************************************************************************************************/
void Draw_Arrow(V3d vector, GLdouble radius,V3d origin, V4d specular,V4d diffuse,V4d ambiant, gboolean negative)
{
	V3d top;
	V3d bottom;
	V3d center;
	double lengt;
	gint i;
	V4d diffuseFleche;
	V4d ambiantFleche;

	bottom[0] = origin[0];
	bottom[1] = origin[1];
	bottom[2] = origin[2];
	if(negative)
	{
		bottom[0] -= vector[0];
		bottom[1] -= vector[1];
		bottom[2] -= vector[2];
	}


	top[0] = origin[0] + vector[0];
	top[1] = origin[1] + vector[1];
	top[2] = origin[2] + vector[2];

	lengt = v3d_length(vector);

	if(radius<0.1) radius = 0.1;

	if(negative) lengt *=2;

	center[0] = top[0];
	center[1] = top[1];
	center[2] = top[2];

	top[0] += (top[0]-bottom[0])/lengt*2*radius;
	top[1] += (top[1]-bottom[1])/lengt*2*radius;
	top[2] += (top[2]-bottom[2])/lengt*2*radius;


	Cylinder_Draw_Color(radius/2,bottom,center,specular,diffuse,ambiant);
	for(i=0;i<3;i++)
	{
		diffuseFleche[i] = diffuse[i] *0.6;
		ambiantFleche[i] = ambiant[i] *0.6;
	}
	diffuseFleche[3] = diffuse[3];
	ambiantFleche[3] = ambiant[3];

	Prism_Draw_Color(radius/1.5,center,top,specular,diffuseFleche,ambiantFleche);
}
/*********************************************************************************************/
/* pango fonts for OpenGL  */
void glInitFontsUsing(gchar* fontname, PangoContext* *pft2_context)
{
	OpenGLOptions openGlOptions = get_opengl_options();
	/*if (fontOffset >=0) return;*/

	/* fprintf(stderr,"FontName = %s\n",fontname);*/
	fontOffset = glGenLists(256);
	if (fontOffset && openGlOptions.activateText==1)
	{ 
		//static PangoContext *ft2_context = NULL;

		PangoFontDescription *pfd = pango_font_description_from_string(fontname);
		if (pfd)
		{
			glFontsize = pango_font_description_get_size(pfd) / PANGO_SCALE;
			/* fprintf(stderr,"PANGO_SCALE = %d\n",PANGO_SCALE);*/
			if(!*pft2_context) *pft2_context = pango_ft2_get_context (10*glFontsize, 10*glFontsize);
			pango_context_set_font_description (*pft2_context, pfd);
			charWidth = glFontsize / PANGO_SCALE;
			charHeight = glFontsize / PANGO_SCALE;
			pango_font_description_free(pfd); 
		}
	}
}
/*********************************************************************************************/
void gl_pango_ft2_render_layout (PangoLayout *layout)
{
	PangoRectangle logical_rect;
	FT_Bitmap bitmap;
	GLvoid *pixels;
	guint32 *p;
	GLfloat color[4];
	guint32 rgb;
	GLfloat a;
	guint8 *row, *row_end;
	int i;

	pango_layout_get_extents (layout, NULL, &logical_rect);
	if (logical_rect.width == 0 || logical_rect.height == 0)
  		return;

	bitmap.rows = PANGO_PIXELS (logical_rect.height);
	bitmap.width = PANGO_PIXELS (logical_rect.width);
	bitmap.pitch = bitmap.width;
	bitmap.buffer = g_malloc (bitmap.rows * bitmap.width);
	bitmap.num_grays = 256;
	bitmap.pixel_mode = ft_pixel_mode_grays;

	memset (bitmap.buffer, 0, bitmap.rows * bitmap.width);
	pango_ft2_render_layout (&bitmap, layout, PANGO_PIXELS (-logical_rect.x), 0);

	pixels = g_malloc (bitmap.rows * bitmap.width * 4);
	p = (guint32 *) pixels;

	glGetFloatv (GL_CURRENT_COLOR, color);
#if !defined(GL_VERSION_1_2) && G_BYTE_ORDER == G_LITTLE_ENDIAN
	  rgb =	((guint32) (color[0] * 255.0))        |
        	(((guint32) (color[1] * 255.0)) << 8)  |
        	(((guint32) (color[2] * 255.0)) << 16);
#else
  	rgb = 	(((guint32) (color[0] * 255.0)) << 24) |
        	(((guint32) (color[1] * 255.0)) << 16) |
        	(((guint32) (color[2] * 255.0)) << 8);
#endif
	a = color[3];

	row = bitmap.buffer + bitmap.rows * bitmap.width; /* past-the-end */
	row_end = bitmap.buffer;      /* beginning */

	if (a == 1.0)
	{
		do
        	{
          		row -= bitmap.width;
          		for (i = 0; i < bitmap.width; i++)
#if !defined(GL_VERSION_1_2) && G_BYTE_ORDER == G_LITTLE_ENDIAN
            			*p++ = rgb | (((guint32) row[i]) << 24);
#else
            			*p++ = rgb | ((guint32) row[i]);
#endif
        	}
      		while (row != row_end);
    	}
	else
    	{
      		do
        	{
          		row -= bitmap.width;
          		for (i = 0; i < bitmap.width; i++)
#if !defined(GL_VERSION_1_2) && G_BYTE_ORDER == G_LITTLE_ENDIAN
            			*p++ = rgb | (((guint32) (a * row[i])) << 24);
#else
            			*p++ = rgb | ((guint32) (a * row[i]));
#endif
        	}while (row != row_end);
	}

	glPixelStorei (GL_UNPACK_ALIGNMENT, 4);

	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if !defined(GL_VERSION_1_2)
	glDrawPixels (bitmap.width, bitmap.rows, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
#else
	glDrawPixels (bitmap.width, bitmap.rows, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, pixels);
#endif
	glDisable (GL_BLEND);
	g_free (bitmap.buffer);
	g_free (pixels);
}
/*********************************************************************************************/
/* print at a window position */
void glPrintWin(gint x, gint y, gint height, gchar *str, PangoContext *ft2_context)
{
	PangoLayout *layout;
	PangoRectangle logical_rect;

	/* Text layout */
	layout = pango_layout_new (ft2_context);
	pango_layout_set_width (layout, PANGO_SCALE * strlen(str)*height);
	pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);

	gdouble w[3];

	glGetWorldCoords(x, y, height, w);
	glRasterPos3f(w[0], w[1], w[2]); 
	gl2psText(str, "Times-Roman", glFontsize);

	pango_layout_set_text (layout, str, -1);


	glListBase(fontOffset);
	glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);

	gl_pango_ft2_render_layout (layout);

	g_object_unref (G_OBJECT (layout));
}
/*********************************************************************************************/
void glPrintOrtho(gdouble x, gdouble y, gdouble z, gchar *str, gboolean centerX, gboolean centerY, PangoContext *ft2_context)
{
	gdouble w[] = {x, y, z};
	gint xy[] = {0, 0};
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetWindowCoords(w, viewport[3], xy);
	glPushMatrix();
	glLoadIdentity();
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix(); glLoadIdentity();
	gluOrtho2D(0, viewport[2], 0, viewport[3]);

	
	if(centerX) xy[0] -= charWidth*strlen(str)/2;
	if(centerY) xy[1] += charHeight/4;
	glPrintWin(xy[0], xy[1], viewport[3], str, ft2_context);
	
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	glPopMatrix();
}
/*********************************************************************************************/
/* print at a world position */
void glPrint(gdouble x, gdouble y, gdouble z, gchar *str, PangoContext *ft2_context)
{

	PangoLayout *layout;
	PangoRectangle logical_rect;

	/* Text layout */
	layout = pango_layout_new (ft2_context);
	pango_layout_set_width (layout, PANGO_SCALE * strlen(str)*glFontsize);
	pango_layout_set_alignment (layout, PANGO_ALIGN_CENTER);

	gdouble w[3];

	glRasterPos3f(x,y,z); 
	gl2psText(str, "Times-Roman", glFontsize);

	pango_layout_set_text (layout, str, -1);


	glListBase(fontOffset);
	glCallLists(strlen(str), GL_UNSIGNED_BYTE, str);

	gl_pango_ft2_render_layout (layout);

	g_object_unref (G_OBJECT (layout));
}
/*********************************************************************************************/
/* print at a world position  after scaling of scal in scene coordinates*/
void glPrintScale(gdouble x, gdouble y, gdouble z, gdouble scale, gchar *str, PangoContext *ft2_context)
{
	gdouble VScene[]={0,0,0};
	gdouble VWorld[]={x,y,z};
	GLdouble mvMatrix[4][4];
	gdouble** trMatrix;
	gdouble** invMatrix;
	gint i;
	gint j;
	glGetDoublev(GL_MODELVIEW_MATRIX, &mvMatrix[0][0]);
	trMatrix = g_malloc(3*sizeof(gdouble*));
	for(i=0;i<3;i++) trMatrix[i] = g_malloc(3*sizeof(gdouble));

	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
			trMatrix[j][i] = mvMatrix[i][j];

	for(i=0;i<3;i++)
	{
		VScene[i] = 0;
		for(j=0;j<3;j++)
		{
			VScene[i] += trMatrix[i][j]*VWorld[j];
		}
	}
	VScene[2] += scale;

	invMatrix = Inverse3(trMatrix);
	for(i=0;i<3;i++) g_free(trMatrix[i]);
	g_free(trMatrix);

	if(invMatrix != NULL)
	{
		for(i=0;i<3;i++)
		{
			VWorld[i] = 0;
			for(j=0;j<3;j++)
			{
				VWorld[i] += invMatrix[i][j]*VScene[j];
			}
		}
		for(i=0;i<3;i++) g_free(invMatrix[i]);
		g_free(invMatrix);
	}
	else
	{
		VWorld[0] = x;
		VWorld[1] = y;
		VWorld[2] = z;
	}
	glPrint(VWorld[0],VWorld[1],VWorld[2], str, ft2_context);
}
