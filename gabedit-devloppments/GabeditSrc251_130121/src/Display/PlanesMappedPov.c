/*PlanesMappedPov.c */
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
#include "../Display/PlanesMapped.h"
#include <unistd.h>
#include "../Display/PlanesMappedDraw.h"
#include "../Display/ColorMap.h"
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
static gchar *get_pov_mesh2(
		gdouble C1[],gdouble C2[], gdouble C3[],  gdouble C4[], 
		gdouble N1[],gdouble N2[], gdouble N3[],  gdouble N4[],
		gdouble color1[],  gdouble color2[], gdouble color3[], gdouble color4[])
{
	gchar* temp = NULL;
	gchar* t = NULL;
	gint i;
	gdouble d = 0;
	for(i=0;i<3;i++) d += (C1[i]-C2[i])*(C1[i]-C2[i]);
	if(d<1e-8) return g_strdup("\n");

	d = 0;
	for(i=0;i<3;i++) d += (C2[i]-C3[i])*(C2[i]-C3[i]);
	if(d<1e-8) return g_strdup("\n");

	d = 0;
	for(i=0;i<3;i++) d += (C3[i]-C4[i])*(C3[i]-C4[i]);
	if(d<1e-8) return g_strdup("\n");

	d = 0;
	for(i=0;i<3;i++) d += (C1[i]-C4[i])*(C1[i]-C4[i]);
	if(d<1e-8) return g_strdup("\n");


	if(TypeBlend == GABEDIT_BLEND_YES) t = g_strdup(" filter surfaceTransCoef");
	else t = g_strdup(" ");

	  temp = g_strdup_printf("mesh2\n"
				"{\n"
			           "\tvertex_vectors{ 4,\n"
			              "\t\t<%lf, %lf, %lf>\n"
			              "\t\t<%lf, %lf, %lf>\n"
			              "\t\t<%lf, %lf, %lf>\n"
			              "\t\t<%lf, %lf, %lf>\n"
				   "\t}\n"
				   "\tnormal_vectors{ 4,\n"
			              "\t\t<%lf, %lf, %lf>\n"
			              "\t\t<%lf, %lf, %lf>\n"
			              "\t\t<%lf, %lf, %lf>\n"
			              "\t\t<%lf, %lf, %lf>\n"
				   "\t}\n"
				   "\ttexture_list{ 4,\n"
				      "\t\ttexture{pigment{rgb<%lf,%lf,%lf> %s} finish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}}\n"
				      "\t\ttexture{pigment{rgb<%lf,%lf,%lf> %s} finish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}}\n"
				      "\t\ttexture{pigment{rgb<%lf,%lf,%lf> %s} finish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}}\n"
				      "\t\ttexture{pigment{rgb<%lf,%lf,%lf> %s} finish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}}\n"
				   "\t}\n"
			           "\tface_indices{2,<0,1,2> 0, 1, 2 <2,3,0> 2, 3, 0 }\n"
				    "\ttransform { myTransforms }\n"
				   "}\n",
				   C1[0], C1[1], C1[2],
				   C2[0], C2[1], C2[2],
				   C3[0], C3[1], C3[2],
				   C4[0], C4[1], C4[2],
				   N1[0], N1[1], N1[2],
				   N2[0], N2[1], N2[2],
				   N3[0], N3[1], N3[2],
				   N4[0], N4[1], N4[2],
				   color1[0], color1[1], color1[2], t,
				   color2[0], color2[1], color2[2], t,
				   color3[0], color3[1], color3[2], t,
				   color4[0], color4[1], color4[2], t
				    );
	if(t) g_free(t);

	return temp;

}
/********************************************************************************/
static void addPlanPovRay(FILE* file, Grid* plansgrid,gint i0,gint i1,gint numPlane,gdouble Gap[])
{
	gchar* temp;
	gint ip = numPlane;
	gint ix=0,iy=0,iz=0;
	gint ix1=0,iy1=0,iz1=0;
	gint ix2=0,iy2=0,iz2=0;
	gint ix3=0,iy3=0,iz3=0;
	gint ix4,iy4,iz4;
	gint i,j,ii,jj;
	gdouble C1[3];
	gdouble C2[3];
	gdouble Color[3];

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
static gdouble* GetGapVector(Grid* plansgrid,gint i0,gint i1,gint numPlane,gdouble gap)
{
	gdouble x1,y1,z1;
	gdouble x2,y2,z2;
	gint ip = numPlane;
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
/***************************************************************************************************************/
static ColorMap* get_colorMap()
{
	GtkWidget* handleBoxColorMapPlanesMapped = g_object_get_data(G_OBJECT(PrincipalWindow), "HandleboxColorMapPlanesMapped");
	ColorMap* colorMap = g_object_get_data(G_OBJECT(handleBoxColorMapPlanesMapped),"ColorMap");

	return colorMap;
}
/***************************************************************************************************************/
static void AddOnePlaneMappedPovRay(FILE* file, Grid* plansgrid, gint i0, gint i1, gint numPlane, gdouble gap)
{
	gchar* temp = NULL;
	gint ip = numPlane;
	gint ix=0,iy=0,iz=0;
	gint ix1=0,iy1=0,iz1=0;
	gint ix2=0,iy2=0,iz2=0;
	gint ix3=0,iy3=0,iz3=0;
	gint ix4,iy4,iz4;
	gint i,j,ii,jj;
	gint k;
	ColorMap* colorMap = get_colorMap();
	gdouble color1[]  = {0.7,0.7,0.7,0.8};
	gdouble color2[]  = {0.7,0.7,0.7,0.8};
	gdouble color3[]  = {0.7,0.7,0.7,0.8};
	gdouble color4[]  = {0.7,0.7,0.7,0.8};
	gdouble C1[3]; 
	gdouble C2[3]; 
	gdouble C3[3]; 
	gdouble C4[3]; 

	gdouble N1[3] = {1,1,1}; 
	gdouble N2[3] = {1,1,1}; 
	gdouble N3[3] = {1,1,1}; 
	gdouble N4[3] = {1,1,1}; 

	gdouble*Gap;
	Gap = GetGapVector(plansgrid, i0, i1, numPlane, gap);
	addPlanPovRay(file, plansgrid, i0, i1, numPlane, Gap);

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
        

	set_Color_From_colorMap(colorMap, color1, plansgrid->point[ix][iy][iz].C[3]);
	for(k=0;k<3;k++) C1[k] = plansgrid->point[ix][iy][iz].C[k] + Gap[k];

	set_Color_From_colorMap(colorMap, color2, plansgrid->point[ix1][iy1][iz1].C[3]);
	for(k=0;k<3;k++) C2[k] = plansgrid->point[ix1][iy1][iz1].C[k] + Gap[k];
	
	set_Color_From_colorMap(colorMap, color3, plansgrid->point[ix2][iy2][iz2].C[3]);
	for(k=0;k<3;k++) C3[k] = plansgrid->point[ix2][iy2][iz2].C[k] + Gap[k];

	set_Color_From_colorMap(colorMap, color4, plansgrid->point[ix3][iy3][iz3].C[3]);
	for(k=0;k<3;k++) C4[k] = plansgrid->point[ix3][iy3][iz3].C[k] + Gap[k];

	temp = get_pov_mesh2(C1, C2, C3, C4, N1, N2, N3, N4, color1,  color2, color3, color4);
	fprintf(file,"%s",temp);
	g_free(temp);
	}
	g_free(Gap);
}
/*********************************************************************************************************/
gint addPlaneMappedPovRay(Grid* plansgrid, gint i0,gint i1,gint numPlane, gdouble gap)
{
	gchar* fileName = g_strdup_printf("%s%stmp%spovrayPlanesMapped.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	FILE* file = fopen(fileName,"a");
	g_free(fileName);
	if(!file) return 1;
	if(!plansgrid) return 1;

	AddOnePlaneMappedPovRay(file, plansgrid, i0, i1, numPlane, gap);
	fclose(file);
	return 0;
}
/*********************************************************************************************************/
void deletePlanesMappedPovRayFile()
{
	gchar* fileName = g_strdup_printf("%s%stmp%spovrayPlanesMapped.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	unlink(fileName);
	g_free(fileName);
}
