/*PlanesMappedDraw.c */
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
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/UtilsGL.h"
#include "../Display/ColorMap.h"

/**************************************************************************/
static ColorMap* get_colorMap()
{
	GtkWidget* handleBoxColorMapPlanesMapped = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapPlanesMapped");
	ColorMap* colorMap = g_object_get_data(G_OBJECT(handleBoxColorMapPlanesMapped),"ColorMap");

	return colorMap;
}
/********************************************************************************/
static gdouble* GetGapVector(Grid* plansgrid,gint i0,gint i1,gint numplan,gdouble gap)
{
	gdouble x1,y1,z1;
	gdouble x2,y2,z2;
	gint ip = numplan;
	gint ix=0,iy=0,iz=0;
	gint ix1=0,iy1=0,iz1=0;
	gint ix2=0,iy2=0,iz2=0;
	gint ix3,iy3,iz3;
	gint ix4,iy4,iz4;
	gint i,j,ii,jj;
	gdouble *Gap = g_malloc(3*sizeof(gdouble));
	gdouble Module;

	i = 0;
	j = 0;
	ii = plansgrid->N[i0]-1;
	jj = plansgrid->N[i1]-1;
	switch(i0)
	{
	case 0: 
		ix = i;
		ix1 = ix2 = ii;
		ix4 = ix3 = ix;
		switch(i1)
		{
			case 1 : iy = j; iz = ip; 
				 iy1 = iy4 =  iy; iy2 = iy3 = jj ;
				 iz1 = iz2 = iz3 = iz4 = iz;
				 break;
			case 2 : iy = ip;iz = j;
				 iy1 = iy2 = iy3 = iy4 = iy;
				 iz1 = iz4 = iz; iz2 = iz3 =  jj ; 
				 break;
		}
		break;
	case 1: iy = i;
		iy1 = iy2 =  ii;
		iy3 = iy4 = iy ;
		switch(i1)
		{
			case 0 : ix = j; iz = ip;
				 iz1 = iz2 = iz3 = iz4 = iz;
				 ix1 = ix4 =  ix; ix2 = ix3 = jj ;
				 break;
			case 2 : ix = ip;iz = j;
				 ix1 = ix2 = ix3 = ix4 = ix;
				 iz1 = iz4 =  iz; iz2 = iz3 = jj ;
				 break;
		}
		break;
		case 2: iz = i;
			iz1 = iz2 =  ii;
			iz3 = iz4 = iz ;
			switch(i1)
			{
				case 0 : ix = j; iy = ip;
				 iy1 = iy2 = iy3 = iy4 = iy;
				 ix1 = ix4 =  ix; ix2 = ix3 = jj ;
				 break;
			case 1 : ix = ip;iy = j;
				 ix1 = ix2 = ix3 = ix4 = ix;
				 iy1 = iy4 =  iy; iy2 = iy3 = jj ;
				 break;
			}
			break;
	}
        

	x1 = plansgrid->point[ix1][iy1][iz1].C[0] - plansgrid->point[ix][iy][iz].C[0];
	y1 = plansgrid->point[ix1][iy1][iz1].C[1] - plansgrid->point[ix][iy][iz].C[1];
	z1 = plansgrid->point[ix1][iy1][iz1].C[2] - plansgrid->point[ix][iy][iz].C[2];

	x2 = plansgrid->point[ix2][iy2][iz2].C[0] - plansgrid->point[ix1][iy1][iz1].C[0] ;
	y2 = plansgrid->point[ix2][iy2][iz2].C[1] - plansgrid->point[ix1][iy1][iz1].C[1] ;
	z2 = plansgrid->point[ix2][iy2][iz2].C[2] - plansgrid->point[ix1][iy1][iz1].C[2] ;

    	Gap[0] = (y1 * z2) - (z1 * y2);
    	Gap[1] = (z1 * x2) - (x1 * z2);
    	Gap[2] = (x1 * y2) - (y1 * x2);

	Module = sqrt(Gap[0]*Gap[0] + Gap[1]*Gap[1] +Gap[2]*Gap[2]);
	for(i=0;i<3;i++)
		Gap[i] = Gap[i]/Module*gap;

	return Gap;
}
/*********************************************************************************************************/
static void PlanDraw(Grid* plansgrid,gint i0,gint i1,gint numplan,gdouble Gap[])
{
	gdouble x;
	gdouble y;
	gdouble z;
	gint ip = numplan;
	gint ix=0,iy=0,iz=0;
	gint ix1=0,iy1=0,iz1=0;
	gint ix2=0,iy2=0,iz2=0;
	gint ix3=0,iy3=0,iz3=0;
	gint ix4,iy4,iz4;
	gint i,j,ii,jj;

	i = 0;
	j = 0;
	ii = plansgrid->N[i0]-1;
	jj = plansgrid->N[i1]-1;
	switch(i0)
	{
	case 0: 
		ix = i;
		ix1 = ix2 = ii;
		ix4 = ix3 = ix;
		switch(i1)
		{
			case 1 : iy = j; iz = ip; 
				 iy1 = iy4 =  iy; iy2 = iy3 = jj ;
				 iz1 = iz2 = iz3 = iz4 = iz;
				 break;
			case 2 : iy = ip;iz = j;
				 iy1 = iy2 = iy3 = iy4 = iy;
				 iz1 = iz4 = iz; iz2 = iz3 =  jj ; 
				 break;
		}
		break;
	case 1: iy = i;
		iy1 = iy2 =  ii;
		iy3 = iy4 = iy ;
		switch(i1)
		{
			case 0 : ix = j; iz = ip;
				 iz1 = iz2 = iz3 = iz4 = iz;
				 ix1 = ix4 =  ix; ix2 = ix3 = jj ;
				 break;
			case 2 : ix = ip;iz = j;
				 ix1 = ix2 = ix3 = ix4 = ix;
				 iz1 = iz4 =  iz; iz2 = iz3 = jj ;
				 break;
		}
		break;
		case 2: iz = i;
			iz1 = iz2 =  ii;
			iz3 = iz4 = iz ;
			switch(i1)
			{
				case 0 : ix = j; iy = ip;
				 iy1 = iy2 = iy3 = iy4 = iy;
				 ix1 = ix4 =  ix; ix2 = ix3 = jj ;
				 break;
			case 1 : ix = ip;iy = j;
				 ix1 = ix2 = ix3 = ix4 = ix;
				 iy1 = iy4 =  iy; iy2 = iy3 = jj ;
				 break;
			}
			break;
	}
        

	glLineWidth(3);
/*	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);*/
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glBegin(GL_POLYGON);
	x = plansgrid->point[ix][iy][iz].C[0] + Gap[0];
	y = plansgrid->point[ix][iy][iz].C[1] + Gap[1];
	z = plansgrid->point[ix][iy][iz].C[2] + Gap[2];
	glVertex3f(x,y,z);
	x = plansgrid->point[ix1][iy1][iz1].C[0] + Gap[0];
	y = plansgrid->point[ix1][iy1][iz1].C[1] + Gap[1];
	z = plansgrid->point[ix1][iy1][iz1].C[2] + Gap[2];
	glVertex3f(x,y,z);
	x = plansgrid->point[ix2][iy2][iz2].C[0] + Gap[0];
	y = plansgrid->point[ix2][iy2][iz2].C[1] + Gap[1];
	z = plansgrid->point[ix2][iy2][iz2].C[2] + Gap[2];
	glVertex3f(x,y,z);
	x = plansgrid->point[ix3][iy3][iz3].C[0] + Gap[0];
	y = plansgrid->point[ix3][iy3][iz3].C[1] + Gap[1];
	z = plansgrid->point[ix3][iy3][iz3].C[2] + Gap[2];
	glVertex3f(x,y,z);
	glEnd();
	glLineWidth(1.5);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
}
/*********************************************************************************************************/
static void drawAllRectangles(Grid* plansgrid,gint i0,gint i1,gint numplan,gdouble Gap[])
{
	gdouble x;
	gdouble y;
	gdouble z;
	gint ip = numplan;
	gint ix=0,iy=0,iz=0;
	gint ix1=0,iy1=0,iz1=0;
	gint ix2=0,iy2=0,iz2=0;
	gint ix3=0,iy3=0,iz3=0;
	gint ix4,iy4,iz4;
	gint i,j,ii,jj;
	ColorMap* colorMap = get_colorMap();
	V4d Diffuse  = {0.7,0.7,0.7,0.8};
	V4d Specular = {0.8,0.8,0.8,0.8 };
	V4d Ambiant  = {1.0,1.0,1.0,0.8};

	glMaterialdv(GL_FRONT_AND_BACK,GL_SPECULAR,Specular);
	glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Ambiant);
	glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,120);
	glDisable ( GL_LIGHTING ) ;
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	for(i=0;i<plansgrid->N[i0]-1;i++)
	for(j=0;j<plansgrid->N[i1]-1;j++)
	{
		ii = i+1;
		jj = j+1;
		switch(i0)
		{
		case 0: 
			ix = i;
			ix1 = ix2 = ii;
			ix4 = ix3 = ix;
			switch(i1)
			{
			case 1 : iy = j; iz = ip; 
				 iy1 = iy4 =  iy; iy2 = iy3 = jj ;
				 iz1 = iz2 = iz3 = iz4 = iz;
				 break;
			case 2 : iy = ip;iz = j;
				 iy1 = iy2 = iy3 = iy4 = iy;
				 iz1 = iz4 = iz; iz2 = iz3 =  jj ; 
				 break;
			}
			break;
		case 1: 
			iy = i;
			iy1 = iy2 =  ii;
			iy3 = iy4 = iy ;
			switch(i1)
			{
			case 0 : ix = j; iz = ip;
				 iz1 = iz2 = iz3 = iz4 = iz;
				 ix1 = ix4 =  ix; ix2 = ix3 = jj ;
				 break;
			case 2 : ix = ip;iz = j;
				 ix1 = ix2 = ix3 = ix4 = ix;
				 iz1 = iz4 =  iz; iz2 = iz3 = jj ;
				 break;
			}
			break;
		case 2: iz = i;
			iz1 = iz2 =  ii;
			iz3 = iz4 = iz ;
			switch(i1)
			{
				case 0 : ix = j; iy = ip;
				 iy1 = iy2 = iy3 = iy4 = iy;
				 ix1 = ix4 =  ix; ix2 = ix3 = jj ;
				 break;
				case 1 : ix = ip;iy = j;
				 ix1 = ix2 = ix3 = ix4 = ix;
				 iy1 = iy4 =  iy; iy2 = iy3 = jj ;
				 break;
			}
			break;
		}

		glBegin(GL_POLYGON);
		set_Color_From_colorMap(colorMap, Diffuse, plansgrid->point[ix][iy][iz].C[3]);
		glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
		glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Diffuse);
		glColor4dv(Diffuse);
		x = plansgrid->point[ix][iy][iz].C[0] + Gap[0];
		y = plansgrid->point[ix][iy][iz].C[1] + Gap[1];
		z = plansgrid->point[ix][iy][iz].C[2] + Gap[2];
		glVertex3f(x,y,z);

		set_Color_From_colorMap(colorMap, Diffuse, plansgrid->point[ix1][iy1][iz1].C[3]);
		glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
		glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Diffuse);
		glColor4dv(Diffuse);
		x = plansgrid->point[ix1][iy1][iz1].C[0] + Gap[0];
		y = plansgrid->point[ix1][iy1][iz1].C[1] + Gap[1];
		z = plansgrid->point[ix1][iy1][iz1].C[2] + Gap[2];
		glVertex3f(x,y,z);

		set_Color_From_colorMap(colorMap, Diffuse, plansgrid->point[ix2][iy2][iz2].C[3]);
		glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
		glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Diffuse);
		glColor4dv(Diffuse);
		x = plansgrid->point[ix2][iy2][iz2].C[0] + Gap[0];
		y = plansgrid->point[ix2][iy2][iz2].C[1] + Gap[1];
		z = plansgrid->point[ix2][iy2][iz2].C[2] + Gap[2];
		glVertex3f(x,y,z);

		set_Color_From_colorMap(colorMap, Diffuse, plansgrid->point[ix3][iy3][iz3].C[3]);
		glMaterialdv(GL_FRONT_AND_BACK,GL_DIFFUSE,Diffuse);
		glMaterialdv(GL_FRONT_AND_BACK,GL_AMBIENT,Diffuse);
		glColor4dv(Diffuse);
		x = plansgrid->point[ix3][iy3][iz3].C[0] + Gap[0];
		y = plansgrid->point[ix3][iy3][iz3].C[1] + Gap[1];
		z = plansgrid->point[ix3][iy3][iz3].C[2] + Gap[2];
		glVertex3f(x,y,z);
		glEnd();
	}
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	glEnable ( GL_LIGHTING ) ;
}
/*********************************************************************************************************/
GLuint PlanesMappedGenOneList(Grid* plansgrid,gint i0,gint i1,gint numplan,gdouble gap)
{
	GLuint mapslist;
	gdouble *Gap;

	if(!plansgrid) return 0;
	Gap = GetGapVector(plansgrid,i0,i1,numplan,gap);

	mapslist = glGenLists(1);
	glNewList(mapslist, GL_COMPILE);

	if(TypeBlend == GABEDIT_BLEND_YES) glEnable(GL_BLEND);
	else glDisable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,120);

 	PlanDraw(plansgrid,i0,i1,numplan,Gap);

	drawAllRectangles(plansgrid, i0, i1, numplan, Gap);

	if(TypeBlend == GABEDIT_BLEND_YES) glEnable(GL_BLEND);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glEndList();

	g_free(Gap);
	return mapslist;

}
/*********************************************************************************************************/
static void setColorMap()
{
	GtkWidget* handleBoxColorMapPlanesMapped = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapPlanesMapped");
	ColorMap* colorMap = g_object_get_data(G_OBJECT( handleBoxColorMapPlanesMapped),"ColorMap");

	if(colorMap) return;
	if(handleBoxColorMapPlanesMapped)
	{
		GtkWidget* entryLeft  = g_object_get_data(G_OBJECT(handleBoxColorMapPlanesMapped), "EntryLeft");
		GtkWidget* entryRight = g_object_get_data(G_OBJECT(handleBoxColorMapPlanesMapped), "EntryRight");
		GtkWidget* darea      = g_object_get_data(G_OBJECT(handleBoxColorMapPlanesMapped), "DrawingArea");

		colorMap = new_colorMap_min_max(-0.5,0.5);

		g_object_set_data(G_OBJECT(handleBoxColorMapPlanesMapped),"ColorMap", colorMap);
		g_object_set_data(G_OBJECT(entryLeft),"ColorMap", colorMap);
		g_object_set_data(G_OBJECT(entryRight),"ColorMap", colorMap);
		g_object_set_data(G_OBJECT(darea),"ColorMap", colorMap);
	}
}
/********************************************************************************/
void showColorMapPlanesMapped()
{
	GtkWidget* handleBoxColorMapPlanesMapped = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapPlanesMapped");
	ColorMap* colorMap = g_object_get_data(G_OBJECT( handleBoxColorMapPlanesMapped),"ColorMap");
	if(!colorMap) setColorMap();

	color_map_show(handleBoxColorMapPlanesMapped);
}
/********************************************************************************/
void hideColorMapPlanesMapped()
{
	GtkWidget* handleBoxColorMapPlanesMapped = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapPlanesMapped");
	color_map_hide(handleBoxColorMapPlanesMapped);
}
/********************************************************************************/
GLuint PlanesMappedGenLists(GLuint mapslist,Grid* plansgrid, gint i0,gint i1,gint numplan,gdouble gap)
{
	if (glIsList(mapslist) == GL_TRUE)
			glDeleteLists(mapslist,1);
	if(plansgrid && numplan>=0)
	{
		mapslist = PlanesMappedGenOneList(plansgrid, i0, i1, numplan, gap);
	}

	return mapslist;
}
/********************************************************************************/
void PlanesMappedShowLists(GLuint list)
{
	if (glIsList(list) == GL_TRUE) glCallList(list);
}
/********************************************************************************/
