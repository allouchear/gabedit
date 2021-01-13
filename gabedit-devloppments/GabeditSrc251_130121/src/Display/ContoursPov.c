/*ContoursPov.c */
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
#include "../Utils/Vector3d.h"
#include "../Utils/Utils.h"
#include "../Utils/Transformation.h"
#include "../Display/Contours.h"
#include <unistd.h>
#include "../Display/ContoursDraw.h"
#include "../Display/ColorMap.h"
#include "../Display/TriangleDraw.h"
/********************************************************************************/
static gchar *get_pov_cylingre(gdouble C1[],gdouble C2[],gdouble Colors[], gdouble coef)
{
     gchar* temp = NULL;
     gint i;
     gdouble d = 0;
     for(i=0;i<3;i++) d += (C1[i]-C2[i])*(C1[i]-C2[i]);
     if(d<1e-8) return g_strdup("\n");

     	temp = g_strdup_printf(
		"cylinder\n"
		"{\n"
		"\t<%14.6f,%14.6f,%14.6f>,\n"
		"\t<%14.6f,%14.6f,%14.6f> \n"
		"\twireFrameCylinderRadius*%lf\n"
		"\ttexture\n"
		"\t{\n"
		"\t\tpigment { rgb<%14.6f,%14.6f,%14.6f> }\n"
		"\t\tfinish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}\n"
		"\t}\n"
		"\ttransform { myTransforms }\n"
		"}\n",
		C1[0],C1[1],C1[2],
		C2[0],C2[1],C2[2],
		coef,
		Colors[0],Colors[1],Colors[2]
		);
	return temp;

}
/********************************************************************************/
static void addPlanPovRay(FILE* file, Grid* plansgrid,gint i0,gint i1,gint numplan,gdouble Gap[], gdouble Color[])
{
	gchar* temp;
	gint ip = numplan;
	gint ix=0,iy=0,iz=0;
	gint ix1=0,iy1=0,iz1=0;
	gint ix2=0,iy2=0,iz2=0;
	gint ix3=0,iy3=0,iz3=0;
	gint ix4,iy4,iz4;
	gint i,j,ii,jj;
	gdouble C1[3];
	gdouble C2[3];

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
        

	for(i=0;i<3;i++) Color[i] = 1.0;
	for(i=0;i<3;i++) C1[i] = plansgrid->point[ix][iy][iz].C[i] + Gap[i];
	for(i=0;i<3;i++) C2[i] = plansgrid->point[ix1][iy1][iz1].C[i] + Gap[i];
	temp = get_pov_cylingre(C1,C2,Color, 1.0);
	fprintf(file,"%s",temp);
	g_free(temp);

	for(i=0;i<3;i++) C1[i] = C2[i];
	for(i=0;i<3;i++) C2[i] = plansgrid->point[ix2][iy2][iz2].C[i] + Gap[i];
	temp = get_pov_cylingre(C1,C2,Color, 1.0);
	fprintf(file,"%s",temp);
	g_free(temp);

	for(i=0;i<3;i++) C1[i] = C2[i];
	for(i=0;i<3;i++) C2[i] = plansgrid->point[ix3][iy3][iz3].C[i] + Gap[i];
	temp = get_pov_cylingre(C1,C2,Color, 1.0);
	fprintf(file,"%s",temp);
	g_free(temp);

	for(i=0;i<3;i++) C1[i] = plansgrid->point[ix][iy][iz].C[i] + Gap[i];
	temp = get_pov_cylingre(C1,C2,Color, 1.0);
	fprintf(file,"%s",temp);
	g_free(temp);


}
/********************************************************************************/
static void AddContoursPovRay(FILE* file,Contours contours,gdouble Gap[], gdouble Color[])
{
	gint i;
	gint j;
	gint l;
	gint n=0;
	gint N[2] = {contours.N[0],contours.N[1]};
	PointsContour** pointscontour= contours.pointscontour;
	gdouble C1[3];
	gdouble C2[3];
	gchar* temp;

	for(i=1;i<N[0]-2;i++)
	{
		for(j=1;j<N[1]-2;j++)
		{
				for(n=0;n<pointscontour[i][j].N-1;n++)
				{
					for(l=0;l<3;l++) C1[l] = pointscontour[i][j].point[n].C[l] + Gap[l];
					for(l=0;l<3;l++) C2[l] = pointscontour[i][j].point[n+1].C[l] + Gap[l];
					temp = get_pov_cylingre(C1,C2,Color, 1.0);
					fprintf(file,"%s",temp);
					g_free(temp);
				}

		}
	}
}
/**************************************************************************/
static ColorMap* get_colorMap_contours()
{
	GtkWidget* handleBoxColorMapContours = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapContours");
	ColorMap* colorMap = g_object_get_data(G_OBJECT(handleBoxColorMapContours),"ColorMap");

	return colorMap;
}
/*********************************************************************************************************/
static gint addOneContoursPovRay(FILE* file, Grid* plansgrid,gint Ncontours,gdouble*values,gint i0,gint i1,gint numplan,gdouble gap)
{
	Contours contours;
	gint i;
	gdouble *Gap;
	V4d Color = {0.7,0.7,0.7};
	ColorMap* colorMap = get_colorMap_contours();

	if(!plansgrid) return 1;
	Gap = GetGapVector(plansgrid,i0,i1,numplan,gap);


	if(Ncontours==1 && values[0]>0) get_color_surface(0,Color);
	if(Ncontours==1 && values[0]<0) get_color_surface(1,Color);
	if(Ncontours==1 && values[0]==0) get_color_surface(2,Color);

	if(Ncontours>1) 
	{
		addPlanPovRay(file, plansgrid,i0,i1,numplan,Gap, Color);
		Color[2] = 0.8;
	}

	for(i=0;i<Ncontours;i++)
	{
		if(Ncontours>1) set_Color_From_colorMap(colorMap, Color, values[i]);
		contours = get_contours(plansgrid,values[i],i0,i1,numplan);
		AddContoursPovRay(file,contours,Gap,Color);
		contour_point_free(contours);
	}
	g_free(Gap);
	return 0;

}
/********************************************************************************/
gint addContoursPovRay(Grid* plansgrid,gint Ncontours,gdouble* values,gint i0,gint i1,gint numplan,gdouble gap)
{
	gchar* fileName = g_strdup_printf("%s%stmp%spovrayContours.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	FILE* file = fopen(fileName,"a");
	g_free(fileName);
	if(!file) return 1;
	if(Ncontours>0)
	{
		addOneContoursPovRay(file, plansgrid,Ncontours,values,i0,i1,numplan,gap);
		fclose(file);
		return 0;
	}
	else return 1;
	return 1;
}
/********************************************************************************/
void deleteContoursPovRayFile()
{
	gchar* fileName = g_strdup_printf("%s%stmp%spovrayContours.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	unlink(fileName);
	g_free(fileName);
}
