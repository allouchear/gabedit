/*ContoursDraw.c */
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
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Display/Contours.h"
#include "../Display/ColorMap.h"
#include "../Display/TriangleDraw.h"

static gboolean dottedNegtaiveContours = FALSE;
/**************************************************************************/
void set_dotted_negative_contours(gboolean dotted)
{
	dottedNegtaiveContours = dotted;
}
/**************************************************************************/
gboolean get_dotted_negative_contours()
{
	return dottedNegtaiveContours;
}
/**************************************************************************/
static ColorMap* get_colorMap_contours()
{
	GtkWidget* handleBoxColorMapContours = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapContours");
	ColorMap* colorMap = g_object_get_data(G_OBJECT(handleBoxColorMapContours),"ColorMap");

	return colorMap;
}
/********************************************************************************/
gdouble* GetGapVector(Grid* plansgrid,gint i0,gint i1,gint numplan,gdouble gap)
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
void PlanDraw(Grid* plansgrid,gint i0,gint i1,gint numplan,gdouble Gap[])
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
static void ContoursDraw(Contours contours,gdouble Gap[], gdouble value)
{
	gint i;
	gint j;
	gint k = 0;
	gint n=0;
	gint N[2] = {contours.N[0],contours.N[1]};
	PointsContour** pointscontour= contours.pointscontour;
	gdouble x, y, z;

	glLineWidth(0.5);
	/*for(i=1;i<N[0]-2;i++)*/
	for(i=0;i<N[0]-1;i++)
	{
		/*for(j=1;j<N[1]-2;j++)*/
		for(j=0;j<N[1]-1;j++)
		{
	/*			Debug("N(%d,%d) = %d\n",i,j,pointscontour[i][j].N);*/
				for(n=0;n<pointscontour[i][j].N-1;n++)
				{
					k++;
					if(dottedNegtaiveContours && value<0) 
					{
						glEnable (GL_LINE_STIPPLE);
						/*glLineStipple (1, 0x0101);*/   /*  dotted   */
						/*glLineStipple (1, 0x00FF);*/   /*  dashed   */
						/*glLineStipple (1, 0x1C47);*/   /*  dash/dot/dash   */
						/* glLineStipple (2, 0xAAAA);  */
						glLineStipple (3, 0x5555);
					}
					glBegin(GL_LINES);
					/*
					glVertex4fv(pointscontour[i][j].point[n].C);
					glVertex4fv(pointscontour[i][j].point[n+1].C);
					*/
					x = pointscontour[i][j].point[n].C[0] + Gap[0];
					y = pointscontour[i][j].point[n].C[1] + Gap[1];
					z = pointscontour[i][j].point[n].C[2] + Gap[2];
					glVertex3f(x,y,z);
					x = pointscontour[i][j].point[n+1].C[0] + Gap[0];
					y = pointscontour[i][j].point[n+1].C[1] + Gap[1];
					z = pointscontour[i][j].point[n+1].C[2] + Gap[2];
					glVertex3f(x,y,z);
					glEnd();
					if(dottedNegtaiveContours && value<0) glDisable (GL_LINE_STIPPLE);
				}

		}
	}
	glLineWidth(1);
	/*
	Debug("N0 = %d\n",N[0]);
	Debug("N1 = %d\n",N[1]);
	Debug("Nlignes = %d\n",k);
	*/
}
/*********************************************************************************************************/
GLuint ContoursPlanGenOneList(Grid* plansgrid,gint Ncontours,gdouble*values,gint i0,gint i1,gint numplan,gdouble gap)
{
	GLuint contourslist;
	Contours contours;
	gint i;
	gdouble *Gap;
	ColorMap* colorMap = get_colorMap_contours();

	if(!plansgrid) return 0;

	Gap = GetGapVector(plansgrid,i0,i1,numplan,gap);

	/* Debug("Gap  = %lf %lf %lf \n",Gap[0],Gap[1],Gap[2]);*/
	contourslist = glGenLists(1);
	glNewList(contourslist, GL_COMPILE);

	glLineWidth(1);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);

	if(TypeBlend == GABEDIT_BLEND_YES)
		glEnable(GL_BLEND);
	else
		glDisable(GL_BLEND);

	/*la couleur de l'objet va etre (1-alpha_de_l_objet) * couleur du fond et (le_reste * couleur originale) */
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	{
		V4d Color  = {0.7,0.7,0.7,0.8};

		glDisable ( GL_LIGHTING ) ;
		glMateriali(GL_FRONT_AND_BACK,GL_SHININESS,120);
/*		
 *		glColor4f(0.7,0.7,0.7,0.8);
 *      	glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT);
 *	 	glEnable(GL_COLOR_MATERIAL);
*/
		if(Ncontours==1 && values[0]>0) get_color_surface(0,Color);
		if(Ncontours==1 && values[0]<0) get_color_surface(1,Color);
		if(Ncontours==1 && values[0]==0) get_color_surface(2,Color);

		if(Ncontours>1) PlanDraw(plansgrid,i0,i1,numplan,Gap);
		/* glDisable(GL_COLOR_MATERIAL);*/
		glDisable(GL_BLEND);
		glEnable(GL_LINE_SMOOTH); 
		for(i=0;i<Ncontours;i++)
		{
			if(Ncontours>1) set_Color_From_colorMap(colorMap, Color, values[i]);
			/* printf("i = %d vale = %f\n",i,values[i]);*/

			glColor4dv(Color);
			contours = get_contours(plansgrid,values[i],i0,i1,numplan);
			/* printf("End get_contours\n");*/
			ContoursDraw(contours,Gap,values[i]);
			/* printf("End ContoursDraw\n");*/
			contour_point_free(contours);				
			/* printf("End contour_point_free\n");*/
		}
		glEnable ( GL_LIGHTING ) ;
	}
	if(TypeBlend == GABEDIT_BLEND_YES)
	{
		glEnable(GL_BLEND);
	}
	glLineWidth(1);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);

	glEndList();

	g_free(Gap);
	return contourslist;

}
/********************************************************************************/
static void setColorMap()
{
	GtkWidget* handleBoxColorMapContours = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapContours");
	ColorMap* colorMap = g_object_get_data(G_OBJECT( handleBoxColorMapContours),"ColorMap");

	if(colorMap) return;
	if(handleBoxColorMapContours)
	{
		GtkWidget* entryLeft  = g_object_get_data(G_OBJECT(handleBoxColorMapContours), "EntryLeft");
		GtkWidget* entryRight = g_object_get_data(G_OBJECT(handleBoxColorMapContours), "EntryRight");
		GtkWidget* darea      = g_object_get_data(G_OBJECT(handleBoxColorMapContours), "DrawingArea");

		colorMap = new_colorMap_min_max(-0.5,0.5);

		g_object_set_data(G_OBJECT(handleBoxColorMapContours),"ColorMap", colorMap);
		g_object_set_data(G_OBJECT(entryLeft),"ColorMap", colorMap);
		g_object_set_data(G_OBJECT(entryRight),"ColorMap", colorMap);
		g_object_set_data(G_OBJECT(darea),"ColorMap", colorMap);
	}
}
/********************************************************************************/
void showColorMapContours()
{
	GtkWidget* handleBoxColorMapContours = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapContours");
	ColorMap* colorMap = g_object_get_data(G_OBJECT( handleBoxColorMapContours),"ColorMap");
	if(!colorMap) setColorMap();

	color_map_show(handleBoxColorMapContours);
}
/********************************************************************************/
void hideColorMapContours()
{
	GtkWidget* handleBoxColorMapContours = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapContours");
	color_map_hide(handleBoxColorMapContours);
}
/********************************************************************************/
GLuint ContoursGenLists(GLuint contourslist,Grid* plansgrid,gint Ncontours,gdouble* values,gint i0,gint i1,gint numplan,gdouble gap)
{
	if (glIsList(contourslist) == GL_TRUE)
			glDeleteLists(contourslist,1);
	if(Ncontours>0)
	{
		contourslist = ContoursPlanGenOneList(plansgrid,Ncontours,values,i0,i1,numplan,gap);
	}

	return contourslist;
}
/********************************************************************************/
void ContoursShowLists(GLuint list)
{
	if (glIsList(list) == GL_TRUE) 
			glCallList(list);
}
/********************************************************************************/
