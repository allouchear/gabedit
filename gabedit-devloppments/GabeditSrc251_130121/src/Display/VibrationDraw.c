/* VibrationDraw.c */
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
#include "../Display/GlobalOrb.h"
#include "../Display/UtilsOrb.h"
#include "../Display/GeomDraw.h"
#include "../Geometry/GeomGlobal.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/Constants.h"
#include "../Utils/UtilsGL.h"

#define Deg_Rad 180.0/PI
#include "../Display/Vibration.h"

#define Deg_Rad 180.0/PI

/************************************************************************/
static void rotated_vector(V3d v)
{
	V3d vz={0.0,0.0,1.0};
	V3d	vert;
	gdouble angle;


	v3d_cross(vz,v,vert);
	angle = acos(v3d_dot(vz,v)/v3d_length(v))*Deg_Rad;
	  
	if(fabs(angle)<1e-6)
		return;
	if(fabs(angle-180)<1e-6)
		glRotated(angle, 1.0, 0.0, 0.0);
	else
	glRotated(angle, vert[0],vert[1],vert[2]);

}
/************************************************************************/
static void draw_prism(GLdouble radius,V3d Base1Pos,V3d Base2Pos,
		V4d Specular,V4d Diffuse,V4d Ambiant)
{
		V3d Direction;
		double lengt;
		GLUquadricObj *obj;

		glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
		glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
		glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant);
		glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,50);

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
static void draw_vector(
		gdouble x0, gdouble y0, gdouble z0,
		gdouble x1, gdouble y1, gdouble z1,
		gdouble radius 
		)
{
	V4d Specular = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse  = {0.0f,0.0f,1.0f,1.0f};
	V4d Ambiant  = {0.0f,0.0f,0.1f,1.0f};
	V3d Base1Pos  = {x0, y0, z0};
	V3d Base2Pos  = {x1, y1, z1};
	V3d Center;
	V3d Direction;
	double lengt;

	Direction[0] = Base2Pos[0]-Base1Pos[0];
	Direction[1] = Base2Pos[1]-Base1Pos[1];
	Direction[2] = Base2Pos[2]-Base1Pos[2];
	lengt = v3d_length(Direction);
	if(lengt<1e-6) return;
	if(radius<0.01) return;

	Direction[0] /= lengt;
	Direction[1] /= lengt;
	Direction[2] /= lengt;

	Center[0] = Base2Pos[0];
	Center[1] = Base2Pos[1];
	Center[2] = Base2Pos[2];

	Base2Pos[0] += Direction[0]*2*radius;
	Base2Pos[1] += Direction[1]*2*radius;
	Base2Pos[2] += Direction[2]*2*radius;


	/*
	if(lengt>2*radius)
	{
		p2 = 2*radius;
		p1 = lengt-p2;
		p = lengt;
	}
	else
	{
		p2 = lengt/2;
		p1 = p2;
		p = lengt;
	}

	Center[0] = (Base1Pos[0]*p2 + Base2Pos[0]*p1)/p;
	Center[1] = (Base1Pos[1]*p2 + Base2Pos[1]*p1)/p;
	Center[2] = (Base1Pos[2]*p2 + Base2Pos[2]*p1)/p;
	*/


	Cylinder_Draw_Color(radius/2,Base1Pos,Center,Specular,Diffuse,Ambiant);
	Diffuse[2] *=0.9;
	Ambiant[2] *=0.9;
	Diffuse[1] =0.9;

	/* draw_prism(radius,Center,Base2Pos,Specular,Diffuse,Ambiant);*/
	draw_prism(radius/1.5,Center,Base2Pos,Specular,Diffuse,Ambiant);
}
/************************************************************************/
static void draw_vectors()
{
	gint m = rowSelected;
	gint j;
	gdouble x0, y0, z0;
	gdouble x1, y1, z1;

	if(m<0) return;

	for(j=0;j<nCenters;j++)
	{
		if(
			vibration.modes[m].vectors[0][j]*vibration.modes[m].vectors[0][j]+
			vibration.modes[m].vectors[1][j]*vibration.modes[m].vectors[1][j]+
			vibration.modes[m].vectors[2][j]*vibration.modes[m].vectors[2][j]
			<vibration.threshold*vibration.threshold
		)continue;
		x0 = vibration.geometry[j].coordinates[0];
		x1 = x0 + 2*vibration.scal*vibration.modes[m].vectors[0][j];

		y0 = vibration.geometry[j].coordinates[1];
		y1 = y0 + 2*vibration.scal*vibration.modes[m].vectors[1][j];

		z0 = vibration.geometry[j].coordinates[2];
		z1 = z0 + 2*vibration.scal*vibration.modes[m].vectors[2][j];
		draw_vector(x0,y0,z0,x1,y1,z1,vibration.radius);
	}
}
/************************************************************************/
GLuint VibGenList(GLuint viblist)
{
	if (glIsList(viblist) == GL_TRUE)
		glDeleteLists(viblist,1);

        viblist = glGenLists(1);
	glNewList(viblist, GL_COMPILE);

	draw_vectors();

	glEndList();
	return viblist;
}
/************************************************************************/
void VibShowList(GLuint viblist)
{
	if(TypeGeom==GABEDIT_TYPEGEOM_NO)
		return;
	if(!ShowVibration)
		return;
	if (glIsList(viblist) == GL_TRUE) 
			glCallList(viblist);

}
/************************************************************************/

