/* GeomDraw */
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
#include "GlobalOrb.h"
#include "../Display/UtilsOrb.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/UtilsGL.h"
#include "../Utils/Constants.h"
#include "../Utils/HydrogenBond.h"
#include "../Utils/Utils.h"
#include "BondsOrb.h"
#include "RingsOrb.h"

/************************************************************************/
static void draw_space(int i, gdouble g)
{
	int k;
	V4d Specular = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse  = {0.0f,0.0f,0.0f,1.0f};
	V4d Ambiant  = {0.2f,0.2f,0.2f,1.0f};
	OpenGLOptions openGLOptions = get_opengl_options();
	  
	Specular[0] = GeomOrb[i].Prop.color.red/(gdouble)65535;
	Specular[1] = GeomOrb[i].Prop.color.green/(gdouble)65535;
	Specular[2] = GeomOrb[i].Prop.color.blue/(gdouble)65535;
	for(k=0;k<3;k++) Diffuse[k] = Specular[k]*0.8;
	for(k=0;k<3;k++) Ambiant[k] = Specular[k]*0.8;
	for(k=0;k<3;k++) Specular[k] = 1.0;

	Sphere_Draw_Color_Precision(GeomOrb[i].Prop.vanDerWaalsRadii*g,GeomOrb[i].C,Specular,Diffuse,Ambiant, (GLint)openGLOptions.numberOfSubdivisionsSphere*2);

}
/************************************************************************/
static void draw_ball(int i,GLdouble scal)
{
	int k;
	V4d Specular = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse  = {0.0f,0.0f,0.0f,1.0f};
	V4d Ambiant  = {0.0f,0.0f,0.0f,1.0f};
	GLdouble g = scal*GeomOrb[i].Prop.radii;
	  
	Specular[0] = GeomOrb[i].Prop.color.red/(gdouble)65535;
	Specular[1] = GeomOrb[i].Prop.color.green/(gdouble)65535;
	Specular[2] = GeomOrb[i].Prop.color.blue/(gdouble)65535;
	for(k=0;k<3;k++) Diffuse[k] = Specular[k]*0.8;
	for(k=0;k<3;k++) Ambiant[k] = Specular[k]*0.5;
	for(k=0;k<3;k++) Specular[k] = 0.8;
	for(k=0;k<3;k++) Ambiant[k] = 0.0;

	Sphere_Draw_Color(g,GeomOrb[i].C,Specular,Diffuse,Ambiant);

}
/************************************************************************/
static void draw_hbond(int i,int j,GLdouble scal)
{
	
	int k;
	GLdouble g;
	V4d Specular1 = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse1  = {0.0f,0.0f,0.0f,1.0f};
	V4d Ambiant1  = {0.0f,0.0f,0.0f,1.0f};
	V4d Specular2 = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse2  = {0.0f,0.0f,0.0f,1.0f};
	V4d Ambiant2  = {0.0f,0.0f,0.0f,1.0f};
	GLdouble aspect = scal;
	GLdouble p1;
	GLdouble p2;
     	gdouble A[3];
     	gdouble B[3];
     	gdouble K[3];
     	static gint n = 10;
	gint kbreak;
	
	if(GeomOrb[i].Prop.radii<GeomOrb[j].Prop.radii) g = GeomOrb[i].Prop.radii*aspect;
	else g = GeomOrb[j].Prop.radii*aspect;
	  
	Specular1[0] = GeomOrb[i].Prop.color.red/(gdouble)65535;
	Specular1[1] = GeomOrb[i].Prop.color.green/(gdouble)65535;
	Specular1[2] = GeomOrb[i].Prop.color.blue/(gdouble)65535;

	Specular2[0] = GeomOrb[j].Prop.color.red/(gdouble)65535;
	Specular2[1] = GeomOrb[j].Prop.color.green/(gdouble)65535;
	Specular2[2] = GeomOrb[j].Prop.color.blue/(gdouble)65535;

	for(k=0;k<3;k++)
	{
		Diffuse1[k] = Specular1[k]*0.8;
		Diffuse2[k] = Specular2[k]*0.8;
	}
	for(k=0;k<3;k++)
	{
		Ambiant1[k] = Specular1[k]*0.5;
		Ambiant2[k] = Specular2[k]*0.5;
	}

	p1 = GeomOrb[i].Prop.covalentRadii+GeomOrb[i].Prop.radii;
	p2 = GeomOrb[j].Prop.covalentRadii+GeomOrb[j].Prop.radii;

	kbreak = (gint)(p1*n/(p1+p2));

	for(k=0;k<3;k++) K[k] =(GeomOrb[j].C[k]-GeomOrb[i].C[k])/(n*5.0/4);
	for(k=0;k<3;k++) A[k] =GeomOrb[i].C[k];
	for(i=0;i<n;i++)
	{
     		for(k=0;k<3;k++) B[k] = A[k] + K[k];
		if(i<=kbreak) Cylinder_Draw_Color(g,A,B,Specular1,Diffuse1,Ambiant1);
		else Cylinder_Draw_Color(g,A,B,Specular2,Diffuse2,Ambiant2);
     		for(k=0;k<3;k++) A[k] = B[k]+K[k]/4;
     }
}
/************************************************************************/
static void draw_wireframe(int i,int j, int line)
{
	
	int k;
	V4d Specular1 = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse1  = {0.0f,0.0f,0.0f,1.0f};
	V4d Ambiant1  = {0.0f,0.0f,0.0f,1.0f};
	V4d Specular2 = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse2  = {0.0f,0.0f,0.0f,1.0f};
	V4d Ambiant2  = {0.0f,0.0f,0.0f,1.0f};
	V3d Center;
	GLdouble p1;
	GLdouble p2;
	GLdouble p;
		  
	glLineWidth(line);
	Specular1[0] = GeomOrb[i].Prop.color.red/(gdouble)65535;
	Specular1[1] = GeomOrb[i].Prop.color.green/(gdouble)65535;
	Specular1[2] = GeomOrb[i].Prop.color.blue/(gdouble)65535;

	Specular2[0] = GeomOrb[j].Prop.color.red/(gdouble)65535;
	Specular2[1] = GeomOrb[j].Prop.color.green/(gdouble)65535;
	Specular2[2] = GeomOrb[j].Prop.color.blue/(gdouble)65535;

	for(k=0;k<3;k++)
	{
		Diffuse1[k] = Specular1[k]*0.99;
		Diffuse2[k] = Specular2[k]*0.99;
	}
	for(k=0;k<3;k++)
	{
		Ambiant1[k] = Specular1[k]*0.9;
		Ambiant2[k] = Specular2[k]*0.9;
		Ambiant1[k] = Specular1[k];
		Ambiant2[k] = Specular2[k];
	}
	for(k=0;k<3;k++)
	{
		Specular1[k] = 0;
		Specular2[k] = 0;

		Diffuse1[k] = 0.1;
		Diffuse2[k] = 0.1;
	}
	p1 = GeomOrb[i].Prop.covalentRadii+GeomOrb[i].Prop.radii;
	p2 = GeomOrb[j].Prop.covalentRadii+GeomOrb[j].Prop.radii;
	p = p1 + p2;

	Center[0] = (GeomOrb[i].C[0]*p2 + GeomOrb[j].C[0]*p1)/p;
	Center[1] = (GeomOrb[i].C[1]*p2 + GeomOrb[j].C[1]*p1)/p;
	Center[2] = (GeomOrb[i].C[2]*p2 + GeomOrb[j].C[2]*p1)/p;


	glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular1);
	glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse1);
	glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant1);
	glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,50);
	
	glBegin(GL_LINES);
	glVertex3f(GeomOrb[i].C[0],GeomOrb[i].C[1],GeomOrb[i].C[2]);
	glVertex3f(Center[0],Center[1],Center[2]);
	glEnd();


	glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular2);
	glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse2);
	glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant2);
	glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,50);

	glBegin(GL_LINES);
	glVertex3f(Center[0],Center[1],Center[2]);
	glVertex3f(GeomOrb[j].C[0],GeomOrb[j].C[1],GeomOrb[j].C[2]);
	glEnd();
	glLineWidth(1);

}
/************************************************************************/
static void draw_bond(int i,int j,GLdouble scal, GabEditBondType bondType)
{
	
	int k;
	GLdouble g;
	V4d Specular1 = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse1  = {0.0f,0.0f,0.0f,1.0f};
	V4d Ambiant1  = {0.0f,0.0f,0.0f,1.0f};
	V4d Specular2 = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse2  = {0.0f,0.0f,0.0f,1.0f};
	V4d Ambiant2  = {0.0f,0.0f,0.0f,1.0f};
	GLdouble aspect = scal;
	GLdouble p1;
	GLdouble p2;
	
	/*
	if(GeomOrb[i].Prop.radii<GeomOrb[j].Prop.radii) g = GeomOrb[i].Prop.radii*aspect;
	else g = GeomOrb[j].Prop.radii*aspect;
	*/
	g = 0.8*aspect;
	  
	Specular1[0] = GeomOrb[i].Prop.color.red/(gdouble)65535;
	Specular1[1] = GeomOrb[i].Prop.color.green/(gdouble)65535;
	Specular1[2] = GeomOrb[i].Prop.color.blue/(gdouble)65535;

	Specular2[0] = GeomOrb[j].Prop.color.red/(gdouble)65535;
	Specular2[1] = GeomOrb[j].Prop.color.green/(gdouble)65535;
	Specular2[2] = GeomOrb[j].Prop.color.blue/(gdouble)65535;

	for(k=0;k<3;k++)
	{
		Diffuse1[k] = Specular1[k]*0.8;
		Diffuse2[k] = Specular2[k]*0.8;
	}
	for(k=0;k<3;k++)
	{
		Ambiant1[k] = Specular1[k]*0.5;
		Ambiant2[k] = Specular2[k]*0.5;
	}

	for(k=0;k<3;k++)
	{
		Ambiant1[k] = 0.1;
		Ambiant2[k] = 0.1;
	}
	for(k=0;k<3;k++)
	{
		Specular1[k] = 0.8;
		Specular2[k] = 0.8;
	}

	p1 = GeomOrb[i].Prop.covalentRadii+GeomOrb[i].Prop.radii;
	p2 = GeomOrb[j].Prop.covalentRadii+GeomOrb[j].Prop.radii;

	if(      bondType == GABEDIT_BONDTYPE_SINGLE ||
		( !ShowMultiBondsOrb && 
		 (bondType == GABEDIT_BONDTYPE_DOUBLE || bondType == GABEDIT_BONDTYPE_TRIPLE)
		 )
	    )
		Cylinder_Draw_Color_Two(g,GeomOrb[i].C,GeomOrb[j].C,
				Specular1,Diffuse1,Ambiant1,
				Specular2,Diffuse2,Ambiant2,
				p1,p2);

	else
	if(bondType == GABEDIT_BONDTYPE_DOUBLE && ShowMultiBondsOrb)
	{
	  	V3d vScal = {g/aspect*0.3,g/aspect*0.3,g/aspect*0.3};
		V3d C1;
		V3d C2;
		V3d cross;
		V3d sub;
		V3d CRing;
		getCentreRing(i,j, CRing);
		v3d_sub(CRing, GeomOrb[i].C, C1);
		v3d_sub(CRing, GeomOrb[j].C, C2);
		v3d_cross(C1, C2, cross);
		v3d_sub(GeomOrb[i].C, GeomOrb[j].C, sub);
		v3d_cross(cross, sub, vScal);
		if(v3d_dot(vScal,vScal)!=0)
		{
			v3d_normal(vScal);
			v3d_scale(vScal, g/aspect*0.3);
		}
		else
			 getvScaleBond(g/aspect*0.3*2, C1,C2, vScal);

		for(k=0;k<3;k++) C1[k] = GeomOrb[i].C[k]-vScal[k];
		for(k=0;k<3;k++) C2[k] = GeomOrb[j].C[k]-vScal[k];
		Cylinder_Draw_Color_Two(g/2,C1,C2, Specular1,Diffuse1,Ambiant1, Specular2,Diffuse2,Ambiant2, p1,p2);
		for(k=0;k<3;k++) C1[k] = GeomOrb[i].C[k]+vScal[k];
		for(k=0;k<3;k++) C2[k] = GeomOrb[j].C[k]+vScal[k];
		Cylinder_Draw_Color_Two(g/2,C1,C2, Specular1,Diffuse1,Ambiant1, Specular2,Diffuse2,Ambiant2, p1,p2);
	}
	else
	if(bondType == GABEDIT_BONDTYPE_TRIPLE && ShowMultiBondsOrb)
	{
	  	V3d vScal = {g/aspect*0.3,g/aspect*0.3,g/aspect*0.3};
		V3d C1;
		V3d C2;
		V3d cross;
		V3d sub;
		V3d CRing;
		getCentreRing(i,j, CRing);
		v3d_sub(CRing, GeomOrb[i].C, C1);
		v3d_sub(CRing, GeomOrb[j].C, C2);
		v3d_cross(C1, C2, cross);
		v3d_sub(GeomOrb[i].C, GeomOrb[j].C, sub);
		v3d_cross(cross, sub, vScal);
		if(v3d_dot(vScal,vScal)!=0)
		{
			v3d_normal(vScal);
			v3d_scale(vScal, g/aspect*0.3*2);
		}
		else
			 getvScaleBond(g/aspect*0.3*2*2, C1,C2, vScal);

		for(k=0;k<3;k++) C1[k] = GeomOrb[i].C[k]-vScal[k];
		for(k=0;k<3;k++) C2[k] = GeomOrb[j].C[k]-vScal[k];
		Cylinder_Draw_Color_Two(g/2,C1,C2, Specular1,Diffuse1,Ambiant1, Specular2,Diffuse2,Ambiant2, p1,p2);
		for(k=0;k<3;k++) C1[k] = GeomOrb[i].C[k];
		for(k=0;k<3;k++) C2[k] = GeomOrb[j].C[k];
		Cylinder_Draw_Color_Two(g/2,C1,C2, Specular1,Diffuse1,Ambiant1, Specular2,Diffuse2,Ambiant2, p1,p2);
		for(k=0;k<3;k++) C1[k] = GeomOrb[i].C[k]+vScal[k];
		for(k=0;k<3;k++) C2[k] = GeomOrb[j].C[k]+vScal[k];
		Cylinder_Draw_Color_Two(g/2,C1,C2, Specular1,Diffuse1,Ambiant1, Specular2,Diffuse2,Ambiant2, p1,p2);
	}
}
/************************************************************************/
static gint GeomDrawBallStick(gdouble scaleBall, gdouble scaleStick)
{
	gint i;
	gint j;
	GList* list = NULL;
	for(i = 0;i<nCenters;i++) 
	{
		if(!ShowHAtomOrb && strcmp("H",GeomOrb[i].Symb)==0) continue;
		draw_ball(i,scaleBall);
	}
	for(list=BondsOrb;list!=NULL;list=list->next)
	{
		BondType* data=(BondType*)list->data;
		i = data->n1;
		j = data->n2;
		if(!ShowHAtomOrb && (strcmp("H",GeomOrb[i].Symb)==0 || strcmp("H",GeomOrb[j].Symb)==0)) continue;
		if(data->bondType == GABEDIT_BONDTYPE_HYDROGEN)
			draw_hbond(i,j,1.0/10.0*scaleStick);
		else
			draw_bond(i,j,1.0/3.0*scaleStick, data->bondType);
	}
  	return TRUE;
}
/************************************************************************/
static gint GeomDrawSpaceFill(gdouble scaleBall)
{
	int i;
	  for(i = 0;i<nCenters;i++)
	  {
		if(!ShowHAtomOrb && strcmp("H",GeomOrb[i].Symb)==0) continue;
		draw_space(i,scaleBall);
	  }

  return TRUE;
}
/************************************************************************/
static void draw_ball_for_stick(int i, GLdouble g)
{
	int k;
	V4d Specular = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse  = {0.0f,0.0f,0.0f,1.0f};
	V4d Ambiant  = {0.0f,0.0f,0.0f,1.0f};
	  
	Specular[0] = GeomOrb[i].Prop.color.red/(gdouble)65535;
	Specular[1] = GeomOrb[i].Prop.color.green/(gdouble)65535;
	Specular[2] = GeomOrb[i].Prop.color.blue/(gdouble)65535;
	for(k=0;k<3;k++) Diffuse[k] = Specular[k]*0.8;
	for(k=0;k<3;k++) Ambiant[k] = Specular[k]*0.5;
	for(k=0;k<3;k++) Specular[k] = 0.8;
	for(k=0;k<3;k++) Ambiant[k] = 0.1;

	Sphere_Draw_Color(g,GeomOrb[i].C,Specular,Diffuse,Ambiant);

}
/************************************************************************/
static void draw_bond_for_stick(int i,int j,GLdouble g, GabEditBondType bondType)
{
	
	int k;
	V4d Specular1 = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse1  = {0.0f,0.0f,0.0f,1.0f};
	V4d Ambiant1  = {0.0f,0.0f,0.0f,1.0f};
	V4d Specular2 = {1.0f,1.0f,1.0f,1.0f};
	V4d Diffuse2  = {0.0f,0.0f,0.0f,1.0f};
	V4d Ambiant2  = {0.0f,0.0f,0.0f,1.0f};
	GLdouble p1;
	GLdouble p2;
	GLdouble aspect = g;
	
	  
	Specular1[0] = GeomOrb[i].Prop.color.red/(gdouble)65535;
	Specular1[1] = GeomOrb[i].Prop.color.green/(gdouble)65535;
	Specular1[2] = GeomOrb[i].Prop.color.blue/(gdouble)65535;

	Specular2[0] = GeomOrb[j].Prop.color.red/(gdouble)65535;
	Specular2[1] = GeomOrb[j].Prop.color.green/(gdouble)65535;
	Specular2[2] = GeomOrb[j].Prop.color.blue/(gdouble)65535;

	for(k=0;k<3;k++)
	{
		Diffuse1[k] = Specular1[k]*0.8;
		Diffuse2[k] = Specular2[k]*0.8;
	}
	for(k=0;k<3;k++)
	{
		Ambiant1[k] = Specular1[k]*0.5;
		Ambiant2[k] = Specular2[k]*0.5;
	}
	for(k=0;k<3;k++)
	{
		Ambiant1[k] = 0;
		Ambiant2[k] = 0;
	}
	for(k=0;k<3;k++)
	{
		Specular1[k] = 0.8;
		Specular2[k] = 0.8;
	}
	p1 = GeomOrb[i].Prop.covalentRadii+GeomOrb[i].Prop.radii;
	p2 = GeomOrb[j].Prop.covalentRadii+GeomOrb[j].Prop.radii;

	Cylinder_Draw_Color_Two(g,GeomOrb[i].C,GeomOrb[j].C,
				Specular1,Diffuse1,Ambiant1,
				Specular2,Diffuse2,Ambiant2,
				p1,p2);

	if(      bondType == GABEDIT_BONDTYPE_SINGLE ||
		( !ShowMultiBondsOrb && 
		 (bondType == GABEDIT_BONDTYPE_DOUBLE || bondType == GABEDIT_BONDTYPE_TRIPLE)
		 )
	    )
		Cylinder_Draw_Color_Two(g,GeomOrb[i].C,GeomOrb[j].C,
				Specular1,Diffuse1,Ambiant1,
				Specular2,Diffuse2,Ambiant2,
				p1,p2);
	else
	if(bondType == GABEDIT_BONDTYPE_DOUBLE && ShowMultiBondsOrb)
	{
	  	V3d vScal = {g/aspect*0.35,g/aspect*0.35,g/aspect*0.35};
		V3d C1;
		V3d C2;
		V3d cross;
		V3d sub;
		V3d CRing;
		getCentreRing(i,j, CRing);
		v3d_sub(CRing, GeomOrb[i].C, C1);
		v3d_sub(CRing, GeomOrb[j].C, C2);
		v3d_cross(C1, C2, cross);
		v3d_sub(GeomOrb[i].C, GeomOrb[j].C, sub);
		v3d_cross(cross, sub, vScal);
		if(v3d_dot(vScal,vScal)!=0)
		{
			v3d_normal(vScal);
			v3d_scale(vScal, g/aspect*0.35);
		}
		else
			 getvScaleBond(g/aspect*0.35*2, C1,C2, vScal);

		for(k=0;k<3;k++) C1[k] = GeomOrb[i].C[k];
		for(k=0;k<3;k++) C2[k] = GeomOrb[j].C[k];
		Cylinder_Draw_Color_Two(g,C1,C2, Specular1,Diffuse1,Ambiant1, Specular2,Diffuse2,Ambiant2, p1,p2);
		for(k=0;k<3;k++) C1[k] = GeomOrb[i].C[k]-vScal[k];
		for(k=0;k<3;k++) C2[k] = GeomOrb[j].C[k]-vScal[k];
		Cylinder_Draw_Color_Two(g/2,C1,C2, Specular1,Diffuse1,Ambiant1, Specular2,Diffuse2,Ambiant2, p1,p2);
	}
	else
	if(bondType == GABEDIT_BONDTYPE_TRIPLE && ShowMultiBondsOrb)
	{
	  	V3d vScal = {g/aspect*0.35,g/aspect*0.35,g/aspect*0.35};
		V3d C1;
		V3d C2;
		V3d cross;
		V3d sub;
		V3d CRing;
		getCentreRing(i,j, CRing);
		v3d_sub(CRing, GeomOrb[i].C, C1);
		v3d_sub(CRing, GeomOrb[j].C, C2);
		v3d_cross(C1, C2, cross);
		v3d_sub(GeomOrb[i].C, GeomOrb[j].C, sub);
		v3d_cross(cross, sub, vScal);
		if(v3d_dot(vScal,vScal)!=0)
		{
			v3d_normal(vScal);
			v3d_scale(vScal, g/aspect*0.35);
		}
		else
			 getvScaleBond(g/aspect*0.35*2, C1,C2, vScal);

		for(k=0;k<3;k++) C1[k] = GeomOrb[i].C[k]-vScal[k];
		for(k=0;k<3;k++) C2[k] = GeomOrb[j].C[k]-vScal[k];
		Cylinder_Draw_Color_Two(g/2,C1,C2, Specular1,Diffuse1,Ambiant1, Specular2,Diffuse2,Ambiant2, p1,p2);
		for(k=0;k<3;k++) C1[k] = GeomOrb[i].C[k];
		for(k=0;k<3;k++) C2[k] = GeomOrb[j].C[k];
		Cylinder_Draw_Color_Two(g,C1,C2, Specular1,Diffuse1,Ambiant1, Specular2,Diffuse2,Ambiant2, p1,p2);
		for(k=0;k<3;k++) C1[k] = GeomOrb[i].C[k]+vScal[k];
		for(k=0;k<3;k++) C2[k] = GeomOrb[j].C[k]+vScal[k];
		Cylinder_Draw_Color_Two(g/2,C1,C2, Specular1,Diffuse1,Ambiant1, Specular2,Diffuse2,Ambiant2, p1,p2);
	}

}
/************************************************************************/
static gint GeomDrawStick(gdouble scaleStick)
{
	int i;
	int j;
	GLdouble g = 0.2;
	GList* list = NULL;
	for(i = 0;i<nCenters;i++) 
	{
		if(!ShowHAtomOrb && strcmp("H",GeomOrb[i].Symb)==0) continue;
		draw_ball_for_stick(i, g*scaleStick);
	}

	for(list=BondsOrb;list!=NULL;list=list->next)
	{
		BondType* data=(BondType*)list->data;
		i = data->n1;
		j = data->n2;
		if(!ShowHAtomOrb && (strcmp("H",GeomOrb[i].Symb)==0 || strcmp("H",GeomOrb[j].Symb)==0)) continue;
		if(data->bondType == GABEDIT_BONDTYPE_HYDROGEN)
			draw_hbond(i,j,0.07*scaleStick);
		else
			draw_bond_for_stick(i, j, g*scaleStick,  data->bondType);
	}
	return TRUE;
}
/************************************************************************/
static gint GeomDrawWireFrame(gdouble scaleBall)
{
	int i;
	int j;
	gboolean* Ok = NULL;
	GList* list = NULL;

	if(nCenters>0) Ok = g_malloc(nCenters*sizeof(gboolean));
	for(i = 0;i<nCenters;i++) Ok[i] = FALSE;

	for(list=BondsOrb;list!=NULL;list=list->next)
	{
		BondType* data=(BondType*)list->data;
		i = data->n1;
		j = data->n2;
		if(!ShowHAtomOrb && (strcmp("H",GeomOrb[i].Symb)==0 || strcmp("H",GeomOrb[j].Symb)==0)) continue;
		if(data->bondType == GABEDIT_BONDTYPE_HYDROGEN)
			draw_wireframe(i,j,1);
		else
			 draw_wireframe(i,j, 2);
		Ok[i] = TRUE;
		Ok[j] = TRUE;
	}
	for(i = 0;i<nCenters;i++) 
	{
		if(!ShowHAtomOrb && strcmp("H",GeomOrb[i].Symb)==0) continue;
		if(!Ok[i]) draw_ball(i,0.2*scaleBall);
	}
	if(Ok) g_free(Ok);
	glLineWidth(1);

	return TRUE;
}
/************************************************************************/
static gint GeomDraw(gdouble scaleBall, gdouble scaleStick)
{
	switch(TypeGeom)
	{
	case GABEDIT_TYPEGEOM_BALLSTICK : return GeomDrawBallStick(scaleBall, scaleStick);
	case GABEDIT_TYPEGEOM_STICK : return GeomDrawStick(scaleStick);
	case GABEDIT_TYPEGEOM_SPACE: return GeomDrawSpaceFill(scaleBall);
	case GABEDIT_TYPEGEOM_WIREFRAME : return GeomDrawWireFrame(scaleBall);
	default: return GeomDrawBallStick(scaleBall, scaleStick);
	}
  return FALSE;
}
/*****************************************************************************/
static void gl_build_box()
{	
	guint i;
	guint j;
    	gdouble rayon;
	gint ni, nj;
	gchar tmp[BSIZE];
	gint nTv = 0;
	gint iTv[3] = {-1,-1,-1};
	V3d Base1Pos;
	V3d Base2Pos;
	V4d Specular = {1.0f,1.0f,1.0f,1.0f};
        V4d Diffuse  = {1.0f,1.0f,1.0f,1.0f};
        V4d Ambiant  = {1.0f,1.0f,1.0f,1.0f};
	gdouble radius = 0.1;
	gdouble Tv[3][3];
	gdouble O[3]={0,0,0};


	for(i=0;i<nCenters;i++)
	{
		sprintf(tmp,"%s",GeomOrb[i].Symb);
		uppercase(tmp);
		if(!strcmp(tmp,"TV")) { iTv[nTv]= i; nTv++;}
	}
	if(nTv<2) return;
	for(i=0;i<nTv;i++)
		for(j=0;j<3;j++) Tv[i][j]=GeomOrb[iTv[i]].C[j]-O[j];

	for(i=0;i<3;i++) Base1Pos[i] = O[i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[0][i];
	Cylinder_Draw_Color(radius, Base1Pos, Base2Pos, Specular, Diffuse, Ambiant);

	for(i=0;i<3;i++) Base1Pos[i] = O[i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[1][i];
	Cylinder_Draw_Color(radius, Base1Pos, Base2Pos, Specular, Diffuse, Ambiant);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[0][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[1][i];
	Cylinder_Draw_Color(radius, Base1Pos, Base2Pos, Specular, Diffuse, Ambiant);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[1][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[0][i];
	Cylinder_Draw_Color(radius, Base1Pos, Base2Pos, Specular, Diffuse, Ambiant);
	if(nTv<3) return;

	for(i=0;i<3;i++) Base1Pos[i] = O[i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[2][i];
	Cylinder_Draw_Color(radius, Base1Pos, Base2Pos, Specular, Diffuse, Ambiant);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[0][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[2][i];
	Cylinder_Draw_Color(radius, Base1Pos, Base2Pos, Specular, Diffuse, Ambiant);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[2][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[0][i];
	Cylinder_Draw_Color(radius, Base1Pos, Base2Pos, Specular, Diffuse, Ambiant);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[1][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[2][i];
	Cylinder_Draw_Color(radius, Base1Pos, Base2Pos, Specular, Diffuse, Ambiant);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[2][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[1][i];
	Cylinder_Draw_Color(radius, Base1Pos, Base2Pos, Specular, Diffuse, Ambiant);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[0][i]+Tv[2][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[1][i];
	Cylinder_Draw_Color(radius, Base1Pos, Base2Pos, Specular, Diffuse, Ambiant);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[0][i]+Tv[1][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[2][i];
	Cylinder_Draw_Color(radius, Base1Pos, Base2Pos, Specular, Diffuse, Ambiant);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[1][i]+Tv[2][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[0][i];
	Cylinder_Draw_Color(radius, Base1Pos, Base2Pos, Specular, Diffuse, Ambiant);

}
/************************************************************************/
GLuint GeomGenList(GLuint geomlist, gdouble scaleBall, gdouble scaleStick,gboolean showBox)
{
	if(!GeomOrb) return 0;
	if (glIsList(geomlist) == GL_TRUE) glDeleteLists(geomlist,1);
	geomlist = glGenLists(1);
	glNewList(geomlist, GL_COMPILE);
	GeomDraw(scaleBall, scaleStick);
	if(showBox) gl_build_box();
	glEndList();
	return geomlist;
}
/************************************************************************/
void GeomShowList(GLuint geomlist)
{
	if(TypeGeom==GABEDIT_TYPEGEOM_NO) return;
	if (glIsList(geomlist) == GL_TRUE) glCallList(geomlist);
}
/************************************************************************/


