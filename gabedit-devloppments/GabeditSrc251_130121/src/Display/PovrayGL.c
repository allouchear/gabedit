/* PovrayGL.c */
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
#include "../Common/Global.h"
#include "GlobalOrb.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/GeomGlobal.h"
#include "../Files/FileChooser.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Transformation.h"
#include "../Utils/UtilsGL.h"
#include "../Utils/Utils.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Utils/HydrogenBond.h"
#include "../Utils/PovrayUtils.h"
#include "../Display/GLArea.h"
#include "../Display/GeomDraw.h"
#include "../Display/TriangleDraw.h"
#include "../Display/AxisGL.h"
#include "../Display/PrincipalAxisGL.h"
#include "../Display/ColorMap.h"
#include "../Display/BondsOrb.h"
#include "../Display/RingsOrb.h"
#include "../Display/Vibration.h"

#include <unistd.h>

#define STICKSIZE 0.2
#define STICKSIZEBALL 0.7


typedef struct _RGB
{
	/* Red Green Blue */
 	gdouble Colors[3];
}RGB;

typedef struct _XYZRC
{
	/* X=C[0], Y=C[1], Z =C[2] Radius=C[3] */
 	gdouble C[4];
	/* RGB Colors */
	RGB P;
}XYZRC;

/********************************************************************************/
static gboolean degenerated_cylinder(gdouble*  v1, gdouble* v2)
{
	gdouble d = 0;
	gint i;
	for(i=0;i<3;i++)
		d += (v1[i]-v2[i])*(v1[i]-v2[i]);
	if(d<PRECISON_CYLINDER) return TRUE;
	return FALSE;
}
/********************************************************************************/
static XYZRC get_prop_center(gint Num, gdouble scale)
{
        XYZRC PropCenter;
	gint i;

	for(i=0;i<3;i++) PropCenter.C[i]=GeomOrb[Num].C[i];
	if(TypeGeom==GABEDIT_TYPEGEOM_SPACE)
		PropCenter.C[3]=GeomOrb[Num].Prop.vanDerWaalsRadii*scale;
	else
		PropCenter.C[3]=GeomOrb[Num].Prop.radii*scale;

	PropCenter.P.Colors[0]=(gdouble)(GeomOrb[Num].Prop.color.red/65535.0);
	PropCenter.P.Colors[1]=(gdouble)(GeomOrb[Num].Prop.color.green/65535.0);
	PropCenter.P.Colors[2]=(gdouble)(GeomOrb[Num].Prop.color.blue/65535.0);

        return  PropCenter;
}
/********************************************************************************/
static XYZRC get_base_dipole()
{
        XYZRC PropCenter;
	gint i;

	for(i=0;i<3;i++) PropCenter.C[i]= 0.0;
	PropCenter.C[3] = Dipole.radius;
	for(i=0;i<3;i++) PropCenter.P.Colors[i]=Dipole.color[i]/65535.0;

        return  PropCenter;
}
/********************************************************************************/
static XYZRC get_tete_dipole()
{
        XYZRC PropCenter;
	gint i;
	static GLdouble f = 2;

	for(i=0;i<3;i++) PropCenter.C[i]= f*Dipole.value[i];
	PropCenter.C[3] = Dipole.radius;
	for(i=0;i<3;i++) 
	{
		PropCenter.P.Colors[i]=Dipole.color[i]/65535.0;
		PropCenter.P.Colors[i]*=0.6;
	}
	PropCenter.P.Colors[1] = PropCenter.P.Colors[2];

        return  PropCenter;
}
/********************************************************************************/
static gdouble get_min(gint k)
{
     gdouble min;
     gint i=0;
     min = GeomOrb[0].C[k];
     for(i=1;i<(gint)nCenters;i++)
		if(min>GeomOrb[i].C[k])
			min = GeomOrb[i].C[k];
     
     min -=20;
     return min;
}
/********************************************************************************/
static gint get_num_min_rayonIJ(gint i, gint j)
{
	if(GeomOrb[i].Prop.radii<GeomOrb[j].Prop.radii) return i;
	return j;
}
/********************************************************************************/
static gchar *get_pov_vibarrow(
		gdouble x0, gdouble y0, gdouble z0,
		gdouble x1, gdouble y1, gdouble z1,
		gdouble radius, gint i
		)

{
     XYZRC C1;
     XYZRC C2;
     gdouble ep = radius/2;
     gchar* temp = NULL;

     C1.C[0] = x0;
     C1.C[1] = y0;
     C1.C[2] = z0;
     C1.C[3] = radius/2;
     C1.P.Colors[0]=0;
     C1.P.Colors[1]=0;
     C1.P.Colors[2]=1;

     C2.C[0] = x1;
     C2.C[1] = y1;
     C2.C[2] = z1;
     C2.C[3] = radius/2;
     C2.P.Colors[0]=0.0;
     C2.P.Colors[1]=0.9;
     C2.P.Colors[2]=0.9;


     if(!degenerated_cylinder(C1.C,C2.C))
     temp = g_strdup_printf(
		"// Vib\n"
		"#declare CBas1_A%d = <%lf, %lf, %lf>;\n"
		"#declare CBas2_A%d = <%lf, %lf, %lf>;\n"
		"#declare Col1_A%d = <%lf, %lf, %lf>;\n"
		"#declare Col2_A%d = <%lf, %lf, %lf>;\n"
		"object\n"
		"{\n"
		"\tarrow (CBas1_A%d, CBas2_A%d, %lf, Col1_A%d, Col2_A%d)\n"
		"}\n",
		i, C1.C[0], C1.C[1], C1.C[2],
		i, C2.C[0], C2.C[1], C2.C[2],
		i,C1.P.Colors[0],C1.P.Colors[1],C1.P.Colors[2],
		i,C2.P.Colors[0],C2.P.Colors[1],C2.P.Colors[2],
		i,i,ep,i,i
		);
     else temp = g_strdup(" ");
     return temp;
}
/********************************************************************************/
static gchar *get_pov_vibration()
{
     	gchar *temp=NULL;
     	gchar *tempold=NULL;
     	gchar *t=NULL;
	gint m = rowSelected;
	gint j;
	gdouble x0, y0, z0;
	gdouble x1, y1, z1;

	if(!ShowVibration || m<0) return g_strdup( " ");

     	temp = g_strdup( "// Vibration arrows \n");
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
		tempold = temp;
		t = get_pov_vibarrow(x0,y0,z0,x1,y1,z1,vibration.radius,j);
		if(t)
		{
			if(tempold)
			{
				temp = g_strdup_printf("%s%s",tempold,t);
				g_free(tempold);
			}
			else temp = g_strdup_printf("%s",t);
		  	g_free(t);
		}
	}
	return temp;
}
/********************************************************************************/
static gchar *get_pov_dipole()
{
     XYZRC C1 = get_base_dipole();
     XYZRC C2 = get_tete_dipole();
     gdouble ep = C1.C[3]/2;
     gchar* temp = NULL;

     if(!degenerated_cylinder(C1.C,C2.C))
     temp = g_strdup_printf(
		"// Dipole\n"
		"#declare CDipole1 = <%lf, %lf, %lf>;\n"
		"#declare CDipole2 = <%lf, %lf, %lf>;\n"
		"#declare ColDipole1 = <%lf, %lf, %lf>;\n"
		"#declare ColDipole2 = <%lf, %lf, %lf>;\n"
		"object\n"
		"{\n"
		"\tarrow (CDipole1, CDipole2, %lf, ColDipole1, ColDipole2)\n"
		"}\n",
		C1.C[0],C1.C[1],C1.C[2],
		C2.C[0],C2.C[1],C2.C[2],
		C1.P.Colors[0],C1.P.Colors[1],C1.P.Colors[2],
		C2.P.Colors[0],C2.P.Colors[1],C2.P.Colors[2],
		ep
		);
     else temp = g_strdup(" ");
     return temp;
}
/********************************************************************************/
static gchar *get_pov_xyz_axes()
{
	gboolean show;
	gboolean negative;
	gdouble origin[3];
	gdouble originX[3];
	gdouble originY[3];
	gdouble originZ[3];
	gdouble radius;
	gdouble scale;
	gdouble xColor[3];
	gdouble yColor[3];
	gdouble zColor[3];
	gdouble vectorX[3]  = {1,0,0};
	gdouble vectorY[3]  = {0,1,0};
	gdouble vectorZ[3]  = {0,0,1};
	gint i;
	gchar* temp; 

	getAxisProperties(&show, &negative, origin, &radius, &scale,  xColor, yColor, zColor);
	if(!show) return NULL;

	for(i=0;i<3;i++) vectorX[i] *= scale;
	for(i=0;i<3;i++) vectorY[i] *= scale;
	for(i=0;i<3;i++) vectorZ[i] *= scale;

	for(i=0;i<3;i++) originX[i] = origin[i];
	for(i=0;i<3;i++) originY[i] = origin[i];
	for(i=0;i<3;i++) originZ[i] = origin[i];


	if(negative)
	{
		for(i=0;i<3;i++) originX[i] -=vectorX[i];
		for(i=0;i<3;i++) originY[i] -=vectorY[i];
		for(i=0;i<3;i++) originZ[i] -=vectorZ[i];

		for(i=0;i<3;i++) vectorX[i] = originX[i]+2*vectorX[i];
		for(i=0;i<3;i++) vectorY[i] = originY[i]+2*vectorY[i];
		for(i=0;i<3;i++) vectorZ[i] = originZ[i]+2*vectorZ[i];
	}
	else
	{
		for(i=0;i<3;i++) vectorX[i] += originX[i];
		for(i=0;i<3;i++) vectorY[i] += originY[i];
		for(i=0;i<3;i++) vectorZ[i] += originZ[i];
	}

	temp = g_strdup_printf(
		"// XYZ Axes\n"
		"#declare COriginAxes1 = <%lf, %lf, %lf>;\n"
		"#declare COriginAxes2 = <%lf, %lf, %lf>;\n"
		"#declare COriginAxes3 = <%lf, %lf, %lf>;\n"
		"#declare CTeteAxes1 = <%lf, %lf, %lf>;\n"
		"#declare CTeteAxes2 = <%lf, %lf, %lf>;\n"
		"#declare CTeteAxes3 = <%lf, %lf, %lf>;\n"
		"#declare ColorAxes1 = <%lf, %lf, %lf>;\n"
		"#declare ColorConeAxes1 = <%lf, %lf, %lf>;\n"
		"#declare ColorAxes2 = <%lf, %lf, %lf>;\n"
		"#declare ColorConeAxes2 = <%lf, %lf, %lf>;\n"
		"#declare ColorAxes3 = <%lf, %lf, %lf>;\n"
		"#declare ColorConeAxes3 = <%lf, %lf, %lf>;\n"
		"#declare AxesRadius = %lf;\n"
		"object\n"
		"{\n"
		"\tarrow (COriginAxes1, CTeteAxes1, AxesRadius, ColorAxes1, ColorConeAxes1)\n"
		"}\n"
		"object\n"
		"{\n"
		"\tarrow (COriginAxes2, CTeteAxes2, AxesRadius, ColorAxes2, ColorConeAxes2)\n"
		"}\n"
		"object\n"
		"{\n"
		"\tarrow (COriginAxes3, CTeteAxes3, AxesRadius, ColorAxes3, ColorConeAxes3)\n"
		"}\n",
		originX[0],originX[1],originX[2],
		originY[0],originY[1],originY[2],
		originZ[0],originZ[1],originZ[2],
		vectorX[0],vectorX[1],vectorX[2],
		vectorY[0],vectorY[1],vectorY[2],
		vectorZ[0],vectorZ[1],vectorZ[2],
		xColor[0],xColor[1],xColor[2],
		xColor[0]/2,xColor[1]/2,xColor[2]/2,
		yColor[0],yColor[1],yColor[2],
		yColor[0]/2,yColor[1]/2,yColor[2]/2,
		zColor[0],zColor[1],zColor[2],
		zColor[0]/2,zColor[1]/2,zColor[2]/2,
		radius/2
		);
     return temp;
}
/********************************************************************************/
static gchar *get_pov_principal_axes()
{
	gboolean show;
	gboolean negative;
	gboolean def;
	gdouble origin[3];
	gdouble firstOrigin[3];
	gdouble secondOrigin[3];
	gdouble thirdOrigin[3];
	gdouble radius;
	gdouble scale;
	gdouble firstColor[3];
	gdouble secondColor[3];
	gdouble thirdColor[3];
	gdouble firstVector[3]  = {1,0,0};
	gdouble secondVector[3]  = {0,1,0};
	gdouble thirdVector[3]  = {0,0,1};
	gint i;
	gchar* temp = NULL;

	getPrincipalAxisProperties(&show,&negative, &def, origin, &radius, &scale,
		firstVector,secondVector,thirdVector,
		firstColor,secondColor, thirdColor);
	if(!def || !show) return NULL;

	for(i=0;i<3;i++) firstVector[i] *= scale;
	for(i=0;i<3;i++) secondVector[i] *= scale;
	for(i=0;i<3;i++) thirdVector[i] *= scale;

	for(i=0;i<3;i++) firstOrigin[i] = origin[i];
	for(i=0;i<3;i++) secondOrigin[i] = origin[i];
	for(i=0;i<3;i++) thirdOrigin[i] = origin[i];
	if(negative)
	{
		for(i=0;i<3;i++) firstOrigin[i] -=firstVector[i];
		for(i=0;i<3;i++) secondOrigin[i] -=secondVector[i];
		for(i=0;i<3;i++) thirdOrigin[i] -=thirdVector[i];

		for(i=0;i<3;i++) firstVector[i] = 2*firstVector[i]+firstOrigin[i];
		for(i=0;i<3;i++) secondVector[i] = 2*secondVector[i]+secondOrigin[i];
		for(i=0;i<3;i++) thirdVector[i] = 2*thirdVector[i]+thirdOrigin[i];
	}
	else
	{
		for(i=0;i<3;i++) firstVector[i] = firstVector[i]+firstOrigin[i];
		for(i=0;i<3;i++) secondVector[i] = secondVector[i]+secondOrigin[i];
		for(i=0;i<3;i++) thirdVector[i] = thirdVector[i]+thirdOrigin[i];
	}

	temp = g_strdup_printf(
		"// Principal Axes\n"
		"#declare COriginPrincipalAxe1 = <%lf, %lf, %lf>;\n"
		"#declare COriginPrincipalAxe2 = <%lf, %lf, %lf>;\n"
		"#declare COriginPrincipalAxe3 = <%lf, %lf, %lf>;\n"
		"#declare CTetePrincipalAxe1 = <%lf, %lf, %lf>;\n"
		"#declare CTetePrincipalAxe2 = <%lf, %lf, %lf>;\n"
		"#declare CTetePrincipalAxe3 = <%lf, %lf, %lf>;\n"
		"#declare ColorPrincipalAxe1 = <%lf, %lf, %lf>;\n"
		"#declare ColorConePrincipalAxe1 = <%lf, %lf, %lf>;\n"
		"#declare ColorPrincipalAxe2 = <%lf, %lf, %lf>;\n"
		"#declare ColorConePrincipalAxe2 = <%lf, %lf, %lf>;\n"
		"#declare ColorPrincipalAxe3 = <%lf, %lf, %lf>;\n"
		"#declare ColorConePrincipalAxe3 = <%lf, %lf, %lf>;\n"
		"#declare PrincipalAxeRadius = %lf;\n"
		"object\n"
		"{\n"
		"\tarrow (COriginPrincipalAxe1, CTetePrincipalAxe1, PrincipalAxeRadius, ColorPrincipalAxe1, ColorConePrincipalAxe1)\n"
		"}\n"
		"object\n"
		"{\n"
		"\tarrow (COriginPrincipalAxe2, CTetePrincipalAxe2, PrincipalAxeRadius, ColorPrincipalAxe2, ColorConePrincipalAxe2)\n"
		"}\n"
		"object\n"
		"{\n"
		"\tarrow (COriginPrincipalAxe3, CTetePrincipalAxe3, PrincipalAxeRadius, ColorPrincipalAxe3, ColorConePrincipalAxe3)\n"
		"}\n",
		firstOrigin[0],firstOrigin[1],firstOrigin[2],
		secondOrigin[0],secondOrigin[1],secondOrigin[2],
		thirdOrigin[0],thirdOrigin[1],thirdOrigin[2],
		firstVector[0],firstVector[1],firstVector[2],
		secondVector[0],secondVector[1],secondVector[2],
		thirdVector[0],thirdVector[1],thirdVector[2],
		firstColor[0],firstColor[1],firstColor[2],
		firstColor[0]/2,firstColor[1]/2,firstColor[2]/2,
		secondColor[0],secondColor[1],secondColor[2],
		secondColor[0]/2,secondColor[1]/2,secondColor[2]/2,
		thirdColor[0],thirdColor[1],thirdColor[2],
		thirdColor[0]/2,thirdColor[1]/2,thirdColor[2]/2,
		radius/2
		);
     return temp;
}
/********************************************************************************/
static gchar *get_pov_ball(gint num, gdouble scale)
{
     gchar *temp;
     XYZRC Center = get_prop_center(num,1.0);
     temp = g_strdup_printf(
		"sphere\n"
		"{\n"
		"\t<%14.6f,%14.6f,%14.6f> %14.6f\n"
		"\ttexture\n"
		"\t{\n"
		"\t\tpigment { rgb<%14.6f,%14.6f,%14.6f> }\n"
		"\t\tfinish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}\n"
		"\t}\n"
		"}\n"
		,
		Center.C[0],Center.C[1],Center.C[2],Center.C[3]*scale,
		Center.P.Colors[0], Center.P.Colors[1], Center.P.Colors[2]
		);
     return temp;
}
/********************************************************************************/
static gchar *get_pov_cylingre(gdouble C1[],gdouble C2[],gdouble Colors[],gdouble ep)
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
		"\t%14.6f\n"
		"\ttexture\n"
		"\t{\n"
		"\t\tpigment { rgb<%14.6f,%14.6f,%14.6f> }\n"
		"\t\tfinish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}\n"
		"\t}\n"
		"}\n",
		C1[0],C1[1],C1[2],
		C2[0],C2[1],C2[2],
		ep,
		Colors[0],Colors[1],Colors[2]
		);
	return temp;

}
/********************************************************************************/
static gchar *get_pov_one_stick(gint i,gint j, GabEditBondType bondType)
{
     gchar *temp;
     gchar *temp1;
     gchar *temp2;
     XYZRC Center1;
     XYZRC Center2;
     gint l;
     /* gint k;*/
     gdouble ep;
     gdouble poid1;
     gdouble poid2;
     gdouble poid;
     gdouble C[3];
     V3d CC1;
     V3d CC2;

     Center1 = get_prop_center(i, 1.0);
     Center2 = get_prop_center(j, 1.0);
 
     /*
     k =get_num_min_rayonIJ(i,j);
     if(k==i) ep = Center1.C[3];
     else ep = Center2.C[3];
     */
     ep =  STICKSIZEBALL;

     /* if(TypeGeom == GABEDIT_TYPEGEOM_STICK ) ep /=4;*/
     if(TypeGeom == GABEDIT_TYPEGEOM_STICK ) ep =STICKSIZE;
     else ep/=2;
     if(TypeGeom==GABEDIT_TYPEGEOM_WIREFRAME ) ep =STICKSIZE/2;

     ep *=  getScaleStick();
 
     poid1 = GeomOrb[i].Prop.covalentRadii+GeomOrb[i].Prop.radii;
     poid2 = GeomOrb[j].Prop.covalentRadii+GeomOrb[j].Prop.radii;
     poid = poid1 + poid2 ;
     for(l=0;l<3;l++) CC1[l] =Center1.C[l];
     for(l=0;l<3;l++) CC2[l] =Center2.C[l];
     if(bondType == GABEDIT_BONDTYPE_SINGLE || !ShowMultiBondsOrb)
     {
     	for(l=0;l<3;l++) C[l] =(Center1.C[l]*poid2+Center2.C[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(Center1.C,C,Center1.P.Colors,ep);
      	temp2 = get_pov_cylingre(C,Center2.C,Center2.P.Colors,ep);
      	temp = g_strdup_printf("%s%s",temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
     }
     else if(bondType == GABEDIT_BONDTYPE_DOUBLE && TypeGeom == GABEDIT_TYPEGEOM_STICK)
     {
	gchar* temp3;
  	V3d vScal = {ep*1.5,ep*1.5,ep*1.5};
	gdouble C1[3];
	gdouble C2[3];
	V3d cros;
	V3d sub;
	V3d CRing;
	gdouble C10[3];
	gdouble C20[3];
	getCentreRing(i,j, CRing);
	v3d_sub(CRing, CC1, C10);
	v3d_sub(CRing, CC2, C20);
	v3d_cross(C10, C20, cros);
	v3d_sub(CC1, CC2, sub);
	v3d_cross(cros, sub, vScal);
	if(v3d_dot(vScal,vScal)!=0)
	{
		v3d_normal(vScal);
		v3d_scale(vScal, ep*1.5);
	}
	else
		 getvScaleBond(ep*1.5*2, C10,C20, vScal);

	for(l=0;l<3;l++) C1[l] = Center1.C[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep);
      	temp = g_strdup_printf("%s%s",temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
	for(l=0;l<3;l++) C1[l] = Center1.C[l]-vScal[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l]-vScal[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep/3);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep/3);
	temp3 = temp;
      	temp = g_strdup_printf("%s%s%s",temp3,temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
      	g_free(temp3);
     }
     else if(bondType == GABEDIT_BONDTYPE_TRIPLE  && TypeGeom == GABEDIT_TYPEGEOM_STICK)
     {
	gchar* temp3;
  	V3d vScal = {ep*1.5,ep*1.5,ep*1.5};
	gdouble C1[3];
	gdouble C2[3];
	V3d cros;
	V3d sub;
	V3d CRing;
	gdouble C10[3];
	gdouble C20[3];
	getCentreRing(i,j, CRing);
	v3d_sub(CRing, CC1, C10);
	v3d_sub(CRing, CC2, C20);
	v3d_cross(C10, C20, cros);
	v3d_sub(CC1, CC2, sub);
	v3d_cross(cros, sub, vScal);
	if(v3d_dot(vScal,vScal)!=0)
	{
		v3d_normal(vScal);
		v3d_scale(vScal, ep*1.5);
	}
	else
		 getvScaleBond(ep*1.5*2, C10,C20, vScal);

	for(l=0;l<3;l++) C1[l] = Center1.C[l]-vScal[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l]-vScal[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep/3);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep/3);
      	temp = g_strdup_printf("%s%s",temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
	for(l=0;l<3;l++) C1[l] = Center1.C[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep);
	temp3 = temp;
      	temp = g_strdup_printf("%s%s%s",temp3,temp1,temp2);
      	g_free(temp3);
      	g_free(temp1);
      	g_free(temp2);
	for(l=0;l<3;l++) C1[l] = Center1.C[l]+vScal[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l]+vScal[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep/3);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep/3);
	temp3 = temp;
      	temp = g_strdup_printf("%s%s%s",temp3,temp1,temp2);
      	g_free(temp3);
      	g_free(temp1);
      	g_free(temp2);
     }
     else if(bondType == GABEDIT_BONDTYPE_DOUBLE)
     {
	gchar* temp3;
  	V3d vScal = {ep*0.5,ep*0.5,ep*0.5};
	gdouble C1[3];
	gdouble C2[3];
	V3d cros;
	V3d sub;
	V3d CRing;
	gdouble C10[3];
	gdouble C20[3];
	getCentreRing(i,j, CRing);
	v3d_sub(CRing, CC1, C10);
	v3d_sub(CRing, CC2, C20);
	v3d_cross(C10, C20, cros);
	v3d_sub(CC1, CC2, sub);
	v3d_cross(cros, sub, vScal);
	if(v3d_dot(vScal,vScal)!=0)
	{
		v3d_normal(vScal);
		v3d_scale(vScal, ep*0.5);
	}
	else
		 getvScaleBond(ep*0.5*2, C10,C20, vScal);

	for(l=0;l<3;l++) C1[l] = Center1.C[l]-vScal[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l]-vScal[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep/3);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep/3);
      	temp = g_strdup_printf("%s%s",temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
	for(l=0;l<3;l++) C1[l] = Center1.C[l]+vScal[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l]+vScal[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep/3);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep/3);
	temp3 = temp;
      	temp = g_strdup_printf("%s%s%s",temp3,temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
      	g_free(temp3);
     }
     else if(bondType == GABEDIT_BONDTYPE_TRIPLE)
     {
	gchar* temp3;
  	V3d vScal = {ep*0.5,ep*0.5,ep*0.5};
	gdouble C1[3];
	gdouble C2[3];
	V3d cros;
	V3d sub;
	V3d CRing;
	gdouble C10[3];
	gdouble C20[3];
	getCentreRing(i,j, CRing);
	v3d_sub(CRing, CC1, C10);
	v3d_sub(CRing, CC2, C20);
	v3d_cross(C10, C20, cros);
	v3d_sub(CC1, CC2, sub);
	v3d_cross(cros, sub, vScal);
	if(v3d_dot(vScal,vScal)!=0)
	{
		v3d_normal(vScal);
		v3d_scale(vScal, ep*0.5*2);
	}
	else
		 getvScaleBond(ep*0.5*2*2, C10,C20, vScal);

	for(l=0;l<3;l++) C1[l] = Center1.C[l]-vScal[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l]-vScal[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep/3);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep/3);
      	temp = g_strdup_printf("%s%s",temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
	for(l=0;l<3;l++) C1[l] = Center1.C[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep/3);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep/3);
	temp3 = temp;
      	temp = g_strdup_printf("%s%s%s",temp3,temp1,temp2);
      	g_free(temp3);
      	g_free(temp1);
      	g_free(temp2);
	for(l=0;l<3;l++) C1[l] = Center1.C[l]+vScal[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l]+vScal[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep/3);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep/3);
	temp3 = temp;
      	temp = g_strdup_printf("%s%s%s",temp3,temp1,temp2);
      	g_free(temp3);
      	g_free(temp1);
      	g_free(temp2);
     }
     else temp = g_strdup_printf(" ");
     return temp;
}
/********************************************************************************/
static gchar *get_pov_one_hbond(gint i,gint j)
{
     gchar *temp;
     XYZRC Center1;
     XYZRC Center2;
     gint l;
     gint k;
     gdouble ep;
     gdouble poid1;
     gdouble poid2;
     gdouble poid;
     gint n = 10;
     gdouble A[3];
     gdouble B[3];
     gdouble K[3];
     gchar *dump;
     gchar *temp1;
     gint ibreak;

     Center1 = get_prop_center(i, 1.0);
     Center2 = get_prop_center(j, 1.0);
     k =get_num_min_rayonIJ(i,j);
 
     if(k==i) ep = Center1.C[3];
     else ep = Center2.C[3];

     if(TypeGeom == GABEDIT_TYPEGEOM_STICK  ) ep /=4;
     else ep/=2;
     if(TypeGeom==GABEDIT_TYPEGEOM_WIREFRAME ) ep /=4;

 
     poid1 = GeomOrb[i].Prop.covalentRadii+GeomOrb[i].Prop.radii;
     poid2 = GeomOrb[j].Prop.covalentRadii+GeomOrb[j].Prop.radii;
     poid = poid1 + poid2 ;

     ibreak = (gint)(poid1*n/poid);

     for(l=0;l<3;l++) K[l] =(Center2.C[l]-Center1.C[l])/(n*5/3);
     for(l=0;l<3;l++) A[l] =Center1.C[l];
     temp = NULL;
     for(i=0;i<n;i++)
     {
     	for(l=0;l<3;l++) B[l] = A[l] + K[l];
	if(i<=ibreak)
		temp1 =  get_pov_cylingre(A,B,Center1.P.Colors,ep/4.0);
	else
		temp1 =  get_pov_cylingre(A,B,Center2.P.Colors,ep/4.0);
	dump = temp;
	if(dump)
	{
     		temp = g_strdup_printf("%s%s",dump,temp1);
		g_free(dump);
	}
	else temp = g_strdup_printf("%s",temp1);
	g_free(temp1);
	temp1 = NULL;
     	for(l=0;l<3;l++) A[l] = B[l]+K[l]/2;
     }

     if(temp1) g_free(temp1);
     return temp;
}
/********************************************************************************/
static gchar *get_pov_epilogue()
{
     gchar *temp;
     temp = g_strdup(
	 "// ****This file was generated by Gabedit ****\n\n"
	 "#include \"colors.inc\"\n"
	 "#include \"textures.inc\"\n"
	 "#include \"shapes.inc\"\n"
	 "#include \"stones1.inc\"\n\n"
	);
     return temp;
}
/********************************************************************************/
static gchar *get_pov_declare_finish_option()
{
	gchar* temp;

	temp = g_strdup_printf(
	 "// finish options\n"
	 "#declare ambientCoef = 0.2;\n"
	 "#declare diffuseCoef = 0.8;\n"
	 "#declare specularCoef = 0.8;\n"
	 "\n"
	);
	return temp;
}
/********************************************************************************/
static gchar *get_pov_declare_surface_options()
{
	gchar* temp;
	gdouble ep;
	/* compute the min radius of atoms*/
	ep = 10000;
	SAtomsProp prop = prop_atom_get("H");
	ep = prop.radii;
	ep /= 64;
	if(ep<1e-3) ep = 1e-3;

	temp = g_strdup_printf(
	 "// transparency coeffition\n"
	 "#declare surfaceTransCoef = %lf;\n"
	 "// wire frame radius of cylinder\n"
	 "#declare wireFrameCylinderRadius = %lf;\n"
	 "\n\n",
	 get_alpha_opacity(),
	 ep
	);
	return temp;
}
/********************************************************************************/
static gchar *get_pov_matrix_transformation()
{
	gdouble q[4];
	gdouble m[4][4];
	gchar* temp;

	getQuat(q);
	build_rotmatrix(m,q);
	temp = g_strdup_printf(
	 "// Rotation matrix\n"
	 "#declare myTransforms = transform {\n"
	 "matrix <%lf, %lf ,%lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf ,%lf>\n"
	 "}\n\n",
	 m[0][0],m[0][1],m[0][2],
	 m[1][0],m[1][1],m[1][2],
	 m[2][0],m[2][1],m[2][2],
	 m[3][0],m[3][1],m[3][2]
	);
	return temp;
}
/********************************************************************************/
static gchar *get_pov_declare_arrow()
{
     gchar *temp;
     temp = g_strdup(
	"// arrow\n"
	"#macro arrow (P1, P2, r, C1, C2) \n"
	"\tunion {\n"
	"\t\tcylinder\n"
	"\t\t{\n"
	"\t\t\tP1,P2,r\n"
	"\t\t\ttexture\n"
	"\t\t\t{\n"
	"\t\t\t\tpigment { rgb C1}\n"
	"\t\t\t\tfinish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}\n"
	"\t\t\t}\n"
	"\t\t}\n"
	"\t\tcone\n"
	"\t\t{\n"
	"\t\t\tP2,r*2.0/1.5,P2+(P2-P1)*4*r/vlength(P2-P1),0\n"
	"\t\t\ttexture\n"
	"\t\t\t{\n"
	"\t\t\t\tpigment { rgb C2}\n"
	"\t\t\t\tfinish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}\n"
	"\t\t\t}\n"
	"\t\t}\n"
	"\t\tno_shadow\n"
	"\t}\n"
	"#end\n"
	 "\n"
		    );
     return temp;
}
/********************************************************************************/
static gchar *get_pov_declare_threeCylinders_wireframe()
{
     gchar *temp;
     temp = g_strdup(
	"// TriangleCylinders\n"
	"#macro threeCylinders ( P1, P2, P3, r) \n"
	"\tunion {\n"
	"\t\t#if (vlength(P1-P2)>0)\n"
	"\t\t\tcylinder { P1 , P2, r}\n"
	"\t\t#end\n"
	"\t\t#if (vlength(P2-P3)>0)\n"
	"\t\t\tcylinder { P2 , P3, r}\n"
	"\t\t#end\n"
	"\t\t#if (vlength(P1-P3)>0)\n"
	"\t\t\tcylinder { P3 , P1, r}\n"
	"\t\t#end\n"
	"\t}\n"
	"#end\n"
	 "\n"
		    );
     return temp;
}
/********************************************************************************/
static gchar *get_pov_declare_threeCylinders_wireframe_color()
{
     gchar *temp;
     temp = g_strdup(
	"// TriangleCylindersColor\n"
	"#macro threeCylindersColor ( P1, P2, P3, C1, C2, C3, r) \n"
	"\tunion {\n"
	"\t\t#if (vlength(P1-P2)>0)\n"
	"\t\t\tcylinder { P1 , P2, r  texture {pigment { rgb C1}} finish {ambient ambientCoef diffuse diffuseCoef specular specularCoef} }\n"
	"\t\t#end\n"
	"\t\t#if (vlength(P2-P3)>0)\n"
	"\t\t\tcylinder { P2 , P3, r  texture {pigment { rgb C2}} finish {ambient ambientCoef diffuse diffuseCoef specular specularCoef} }\n"
	"\t\t#end\n"
	"\t\t#if (vlength(P1-P3)>0)\n"
	"\t\t\tcylinder { P3 , P1, r  texture {pigment { rgb C3}} finish {ambient ambientCoef diffuse diffuseCoef specular specularCoef} }\n"
	"\t\t#end\n"
	"\t}\n"
	"#end\n"
	 "\n"
		    );
     return temp;
}
/********************************************************************************/
static gchar *get_pov_camera()
{
	gchar *temp;
	gdouble f = 5;
	gdouble position = 10;
	gdouble zn, zf, angle;
	gboolean perspective;
	gdouble aspect = 1.0;
	gdouble H = 100;
	gdouble W = 100;
	gdouble fov = 0;
	gdouble d = 0;
	gdouble origin[3];

	get_camera_values(&zn, &zf, &angle, &aspect, &perspective);
	get_orgin_molecule(origin);
	fov = angle;

	position = zf/2;
	f = 0;

	d = zf-zn;
	if(d !=0)
	{
		H = 2*d*tan(PI/360*angle);
		W = aspect*H;
		fov = 360/PI*atan(W/2/d);
	}
	if(fov<0) fov = 360+fov;
	if(fov>180) fov = 179.99;


     
	if(perspective)
	temp = g_strdup_printf(
	 "// CAMERA\n"
	 "camera\n"
	 "{\n"
	 "\tright     %0.14f *x\n"
	  "\tup        y\n"
	  "\tdirection -z\n"
	  "\tangle %0.14f\n"
	  "\tlocation  < 0.000000, 0.00000, %0.14f >\n"
	  "\tlook_at   < 0.000000, 0.00000, %0.14f >\n"
	  "\ttranslate < %0.14f , %0.14f , 0.000000 >\n"
	  "}\n\n",
	   aspect,
	   fov,
	   position,f,
	   -origin[0],
	   -origin[1]
	);
	else
	temp = g_strdup_printf(
	 "// CAMERA\n"
	 "camera\n"
 	 "{\torthographic\n"
	 "\tright     %0.14f *x\n"
	  "\tup        y\n"
	  "\tdirection -z\n"
	  "\tlocation  < 0.000000, 0.00000, %14.8f >\n"
	 "\t scale     %0.14f\n"
	  "\ttranslate < %0.14f , %0.14f , 0.000000 >\n"
	  "}\n\n",
	   aspect,
	   position,
	   angle,
	   -origin[0],
	   -origin[1]
	   );
     return temp;
}
/********************************************************************************/
static gchar *get_pov_light_source(gchar* title,gchar* color,gdouble x,gdouble y, gdouble z)
{
	gchar *temp;
     	temp = g_strdup_printf("%s%s\t<%10.6f,%10.6f,%10.6f>\n\tcolor %s\n}\n",
				title,
         			"light_source\n"
         			"{\n\n",
				 x,y,z,color);
	return temp;           
}
/********************************************************************************/
static gchar *get_pov_light_sources()
{
     gchar *temp;
     gchar* dump1;
     gchar* dump2;
     gchar* dump3;
     gdouble v[3];
/* calcul of Ymax*/

      if(get_light(0,v)) dump1 = get_pov_light_source("// LIGHT 1\n","0.6*White",v[0],v[1],v[2]);
      else dump1 = g_strdup(" ");
      if(get_light(1,v)) dump2 = get_pov_light_source("// LIGHT 2\n","0.6*White",v[0],v[1],v[2]);
      else dump2 = g_strdup(" ");
      if(get_light(2,v)) dump3 = get_pov_light_source("// LIGHT 3\n","0.6*White",v[0],v[1],v[2]);
      else dump3 = g_strdup(" ");

      temp = g_strdup_printf("%s %s %s",dump1, dump2,dump3);
      g_free(dump1);
      g_free(dump2);
      g_free(dump3);

     return temp;
}
/********************************************************************************/
static gchar *get_pov_begin_molecule()
{
     gchar *temp;
     temp = g_strdup( "\n# declare molecule = union {\n");
     return temp;
}
/********************************************************************************/
static gchar *get_pov_end_molecule()
{
     gchar *temp;
     temp = g_strdup("transform { myTransforms }\n}\n\nobject {molecule}");
     return temp;
}
/********************************************************************************/
static gchar *get_pov_atoms()
{
     	gchar *temp=NULL;
     	gchar *tempold=NULL;
     	gchar *t=NULL;
     	gint i=0;
     	temp = g_strdup( "// ATOMS \n");
	for(i=0;i<(gint)nCenters;i++)
	{
		if(!ShowHAtomOrb && strcmp("H",GeomOrb[i].Symb)==0) continue;
		tempold = temp;
		t =get_pov_ball(i,1.0*getScaleBall());
		if(tempold)
		{
			temp = g_strdup_printf("%s%s",tempold,t);
			g_free(tempold);
		}
		else
			temp = g_strdup_printf("%s",t);
		if(t)
		  g_free(t);
	}

     return temp;
}
/********************************************************************************/
static gchar *get_pov_ball_for_stick(gint num, gdouble radius)
{
     gchar *temp;
     XYZRC Center = get_prop_center(num,1.0);
     temp = g_strdup_printf(
		"sphere\n"
		"{\n"
		"\t<%14.6f,%14.6f,%14.6f> %14.6f\n"
		"\ttexture\n"
		"\t{\n"
		"\t\tpigment { rgb<%14.6f,%14.6f,%14.6f> }\n"
		"\t\tfinish {ambient ambientCoef diffuse diffuseCoef specular specularCoef}\n"
		"\t}\n"
		"}\n"
		,
		Center.C[0],Center.C[1],Center.C[2], radius,
		Center.P.Colors[0], Center.P.Colors[1], Center.P.Colors[2]
		);
     return temp;
}
/********************************************************************************/
static gchar *get_pov_atoms_for_stick()
{
     	gchar *temp=NULL;
     	gchar *tempold=NULL;
     	gchar *t=NULL;
     	gint i=0;
     	temp = g_strdup( "// ATOMS \n");
	for(i=0;i<(gint)nCenters;i++)
	{
		if(!ShowHAtomOrb && strcmp("H",GeomOrb[i].Symb)==0) continue;
		tempold = temp;
		if(TypeGeom==GABEDIT_TYPEGEOM_WIREFRAME)
			t =get_pov_ball_for_stick(i,STICKSIZE/2*getScaleStick());
		else
			t =get_pov_ball_for_stick(i,STICKSIZE*getScaleStick());
		if(tempold)
		{
			temp = g_strdup_printf("%s%s",tempold,t);
			g_free(tempold);
		}
		else
			temp = g_strdup_printf("%s",t);
		if(t)
		  g_free(t);
	}

     return temp;
}
/********************************************************************************/
static gchar *get_pov_bonds()
{
     gchar *temp = NULL;
     gint i,j;
     gchar* t;
     gchar* tempold;
     gboolean* Ok = NULL;
     GList* list = NULL;
     if(nCenters>0) Ok = g_malloc(nCenters*sizeof(gboolean));
     for(i=0;i<(gint)nCenters;i++) Ok[i] = FALSE;

     temp = g_strdup( "// BONDS \n");
     for(list=BondsOrb;list!=NULL;list=list->next)
     {
	BondType* data=(BondType*)list->data;
	i = data->n1;
	j = data->n2;
	if(!ShowHAtomOrb && (strcmp("H",GeomOrb[i].Symb)==0 || strcmp("H",GeomOrb[j].Symb)==0)) continue;
	if(data->bondType == GABEDIT_BONDTYPE_HYDROGEN)
	{
		tempold = temp;
		t =get_pov_one_hbond(i,j);
		if(tempold)
               	{
               		temp = g_strdup_printf("%s%s",tempold,t);
                       	g_free(tempold);
                }
                else temp = g_strdup_printf("%s",t); 
	}
	else
	{
		Ok[i] = TRUE;
		Ok[j] = TRUE;
		tempold = temp;
		t =get_pov_one_stick(i,j,data->bondType);
		if(tempold)
               	{
                       	temp = g_strdup_printf("%s%s",tempold,t);
                       	g_free(tempold);
               	}
               	else temp = g_strdup_printf("%s",t); 
	}
     }

     for(i=0;i<(gint)nCenters;i++)
     if(!Ok[i])
     {
		if(!ShowHAtomOrb && strcmp("H",GeomOrb[i].Symb)==0) continue;
		tempold = temp;
		t =get_pov_ball(i, 0.5*getScaleBall());
		if(tempold)
		{
			temp = g_strdup_printf("%s%s",tempold,t);
			g_free(tempold);
		}
		else
			temp = g_strdup_printf("%s",t);
		if(t)
		  g_free(t);
      }
     if(Ok) g_free(Ok);
     return temp;
}
/********************************************************************************/
static void save_pov_surfaces(FILE* file)
{
	gchar* fileName = g_strdup_printf("%s%stmp%spovraySurfaces.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	gchar* fileNameLast = g_strdup_printf("%s%stmp%spovrayLastSurface.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	FILE* fileSurfaces = fopen(fileName,"rb");
	FILE* fileLastSurface = fopen(fileNameLast,"rb");
	gchar* t = g_malloc(BSIZE*sizeof(gchar));

	g_free(fileName);
	g_free(fileNameLast);
	if(!fileSurfaces)
	{
		g_free(t);
		return;
	}
	while(!feof(fileSurfaces))
	{
		if(!fgets(t, BSIZE,fileSurfaces)) break;
		fprintf(file,"%s",t);
	}
	fclose(fileSurfaces);

	if(!fileLastSurface)
	{
		g_free(t);
		return;
	}
	while(!feof(fileLastSurface))
	{
		if(!fgets(t, BSIZE,fileLastSurface)) break;
		fprintf(file,"%s",t);
	}
	fclose(fileLastSurface);
	g_free(t);
}
/********************************************************************************/
static void save_pov_contours(FILE* file)
{
	gchar* fileName = g_strdup_printf("%s%stmp%spovrayContours.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	FILE* fileContours = fopen(fileName,"rb");
	gchar* t = g_malloc(BSIZE*sizeof(gchar));

	g_free(fileName);
	if(!fileContours)
	{
		g_free(t);
		return;
	}
	while(!feof(fileContours))
	{
		if(!fgets(t, BSIZE,fileContours)) break;
		fprintf(file,"%s",t);
	}
	fclose(fileContours);
}
/********************************************************************************/
static void save_pov_planes_mapped(FILE* file)
{
	gchar* fileName = g_strdup_printf("%s%stmp%spovrayPlanesMapped.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	FILE* filePlanesMapped = fopen(fileName,"rb");
	gchar* t = g_malloc(BSIZE*sizeof(gchar));

	g_free(fileName);
	if(!filePlanesMapped)
	{
		g_free(t);
		return;
	}
	while(!feof(filePlanesMapped))
	{
		if(!fgets(t, BSIZE,filePlanesMapped)) break;
		fprintf(file,"%s",t);
	}
	fclose(filePlanesMapped);
}
/********************************************************************************/
static void save_pov_rings(FILE* file)
{
	gchar* fileName = g_strdup_printf("%s%stmp%spovrayRings.pov",gabedit_directory(),G_DIR_SEPARATOR_S,G_DIR_SEPARATOR_S);
	FILE* fileRings = fopen(fileName,"rb");
	gchar* t = g_malloc(BSIZE*sizeof(gchar));

	g_free(fileName);
	if(!fileRings)
	{
		g_free(t);
		return;
	}
	while(!feof(fileRings))
	{
		if(!fgets(t, BSIZE,fileRings)) break;
		fprintf(file,"%s",t);
	}
	fclose(fileRings);
}
/********************************************************************************/
static gchar* create_povray_file(gchar* fileName, gboolean saveCamera, gboolean newCamera)
{
	FILE* file;
	gchar* temp;
	static gchar message[1024];
	static gchar* camera = NULL;
	static gchar* background = NULL;
	static gchar* light = NULL;


	if(!GLArea)
	{
		sprintf(message,"Sorry\n OpenGl window is not open");
		return message;
	}
 	if ((!fileName) || (strcmp(fileName,"") == 0))
	{
		sprintf(message,"Sorry\n Problem with the name of file");
	       	return message;
	}

	file = FOpen(fileName, "w");
 	if (!file)
	{
		sprintf(message,"Sorry\n I can not open %s file ",fileName);
	       	return message;
	}
	
	temp =get_pov_epilogue();
	fprintf(file,"%s",temp);
	g_free(temp);
	temp = get_pov_matrix_transformation();
	fprintf(file,"%s",temp);
	g_free(temp);

	temp = get_pov_declare_finish_option();
	fprintf(file,"%s",temp);
	g_free(temp);

	temp = get_pov_declare_arrow();
	fprintf(file,"%s",temp);
	g_free(temp);

	if(newCamera)
	{
		temp =get_pov_camera();
		fprintf(file,"%s",temp);
		if(saveCamera)
		{
			if(camera) g_free(camera);
			camera = g_strdup(temp);
		}
		g_free(temp);
	}
	else
	{
		if(camera) fprintf(file,"%s",camera);
		else
		{
			printf("Erreur : No new camera et camera =NULL\n");
			temp =get_pov_camera();
			fprintf(file,"%s",temp);
			g_free(temp);
		}
	}

	if(newCamera)
	{
		temp = get_pov_light_sources();
		fprintf(file,"%s",temp);
		if(saveCamera)
		{
			if(light) g_free(light);
			light = g_strdup(temp);
		}
		g_free(temp);
	}
	else
	{
		if(light) fprintf(file,"%s",light);
		else
		{
			printf("Erreur : No new camera et camera =NULL\n");
			temp = get_pov_light_sources();
			fprintf(file,"%s",temp);
			g_free(temp);
		}
	}

	if(newCamera)
	{
		gdouble xmin = get_min(0);
		gdouble ymin = get_min(1);
		gdouble zmin = get_min(2);
 		temp = get_pov_background(xmin,ymin,zmin);
		fprintf(file,"%s",temp);
		if(saveCamera)
		{
			if(background) g_free(background);
			background = g_strdup(temp);
		}
		g_free(temp);
	}
	else
	{
		if(background) fprintf(file,"%s",background);
		else
		{
			gdouble xmin = get_min(0);
			gdouble ymin = get_min(1);
			gdouble zmin = get_min(2);
			printf("Erreur : No new camera and background =NULL\n");
 			temp = get_pov_background(xmin,ymin,zmin);
			fprintf(file,"%s",temp);
			g_free(temp);
		}
	}

     	if(nCenters<1)
	{
		fclose(file);
		return NULL;
	}

	temp = get_pov_begin_molecule();
	fprintf(file,"%s",temp);
	g_free(temp);

	if(TypeGeom==GABEDIT_TYPEGEOM_BALLSTICK || TypeGeom==GABEDIT_TYPEGEOM_SPACE)
	{
		temp = get_pov_atoms(); 
		fprintf(file,"%s",temp);
		g_free(temp);
	}
	else if(TypeGeom==GABEDIT_TYPEGEOM_STICK || TypeGeom==GABEDIT_TYPEGEOM_WIREFRAME)
	{
		temp = get_pov_atoms_for_stick(); 
		fprintf(file,"%s",temp);
		g_free(temp);
	}
	if(TypeGeom==GABEDIT_TYPEGEOM_BALLSTICK || TypeGeom==GABEDIT_TYPEGEOM_STICK || TypeGeom==GABEDIT_TYPEGEOM_WIREFRAME)
	{
		temp = get_pov_bonds(); 
		fprintf(file,"%s",temp);
		g_free(temp);
	}
	if(Dipole.def && ShowDipoleOrb)
	{
			temp = get_pov_dipole();
			fprintf(file,"%s",temp);
			g_free(temp);
	}
	temp = get_pov_xyz_axes();
	if(temp)
	{
		fprintf(file,"%s",temp);
		g_free(temp);
	}
	temp = get_pov_principal_axes();
	if(temp)
	{
		fprintf(file,"%s",temp);
		g_free(temp);
	}
	temp = get_pov_vibration();
	if(temp)
	{
		fprintf(file,"%s",temp);
		g_free(temp);
	}

	temp = get_pov_end_molecule();
	fprintf(file,"%s",temp);
	g_free(temp);

	temp = get_pov_declare_surface_options();
	fprintf(file,"%s",temp);
	g_free(temp);

	temp = get_pov_declare_threeCylinders_wireframe();
	fprintf(file,"%s",temp);
	g_free(temp);

	temp = get_pov_declare_threeCylinders_wireframe_color();
	fprintf(file,"%s",temp);
	g_free(temp);

	save_pov_surfaces(file);
	save_pov_contours(file);
	save_pov_planes_mapped(file);
	save_pov_rings(file);
		
 	fclose(file);
	return NULL;
}
/********************************************************************************/
static void save_povray_file(GabeditFileChooser *w , gint response_id)
{
	gchar *fileName;
	gchar* message = NULL;
    	GtkWidget* m;

 	if(response_id != GTK_RESPONSE_OK) return;
	if(!GLArea) return;
 	fileName = gabedit_file_chooser_get_current_file(w);
	message = create_povray_file(fileName, FALSE, TRUE);
	if(message)
	{
    		m = Message(message,"Error",TRUE);
		gtk_window_set_modal (GTK_WINDOW (m), TRUE);
	}
}
/**************************************************************************/
gchar* new_pov(gchar* dirname, int i)
{
	gchar* fileName = g_strdup_printf("%s%sgab%d.pov",dirname,G_DIR_SEPARATOR_S,i);
	gchar* message;
	if(i==1)
	{
		gint j;
		for(j=0;j<100;j++)
		{
			gchar* filestoDelete = g_strdup_printf("%s%sgab%d.pov",dirname,G_DIR_SEPARATOR_S,j);
			unlink(filestoDelete);
			g_free(filestoDelete);

		}
		message = create_povray_file(fileName,TRUE,TRUE);
	}
	else
		message = create_povray_file(fileName,FALSE,FALSE);

	g_free(fileName);
	return message;
}
/********************************************************************************/
void create_save_povray_orb(GtkWidget* Win)
{
	gchar* title = g_strdup_printf("Create a povray file");
  	gchar* patternsfiles[] = {"*","*.pov",NULL};
	gchar* fileName = g_strdup_printf("gabedit.pov");
	gchar* filter = g_strdup_printf("*.pov");

	GtkWidget* win = choose_file_to_create(title, G_CALLBACK(save_povray_file));
	gabedit_file_chooser_set_filters(GABEDIT_FILE_CHOOSER(win), patternsfiles);
	gabedit_file_chooser_set_filter(GABEDIT_FILE_CHOOSER(win),filter);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(win),fileName);
	g_free(title);
	g_free(fileName);
	g_free(filter);
}
/********************************************************************************/
static void create_images_window (GtkWidget* parent, gchar* fileName, gint width, gint height)
{
	GtkWidget *window;
	GtkWidget *scrolled_window;
	GtkWidget *table;
	GtkWidget *vbox;

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	vbox = create_vbox(window);
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_widget_destroyed), &window);

	gtk_window_set_title (GTK_WINDOW (window), fileName);
	gtk_container_set_border_width (GTK_CONTAINER (window), 0);


	scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_container_set_border_width (GTK_CONTAINER (scrolled_window), 1);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_box_pack_start (GTK_BOX (vbox), scrolled_window, TRUE, TRUE, 0);
	gtk_widget_show (scrolled_window);

	table = gtk_table_new (1, 1, FALSE);
	gtk_table_set_row_spacings (GTK_TABLE (table), 1);
	gtk_table_set_col_spacings (GTK_TABLE (table), 1);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (scrolled_window), table);
	gtk_container_set_focus_hadjustment (GTK_CONTAINER (table),
					   gtk_scrolled_window_get_hadjustment (GTK_SCROLLED_WINDOW (scrolled_window)));
	gtk_container_set_focus_vadjustment (GTK_CONTAINER (table),
					   gtk_scrolled_window_get_vadjustment (GTK_SCROLLED_WINDOW (scrolled_window)));
	gtk_widget_show (table);

	{
		GtkWidget* image = gtk_image_new_from_file (fileName);
		if(image) gtk_table_attach(GTK_TABLE(table),image, 0,0+1,0,0+1,
			              (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
			              (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
			              1,1);

		gtk_widget_show (image);
	}
	gtk_window_set_default_size (GTK_WINDOW (window), width+30, height+30);
	gtk_widget_realize(window);
	gtk_widget_show (window);
	gtk_window_set_position(GTK_WINDOW(window),GTK_WIN_POS_CENTER);
	if(parent)
	{
		gtk_window_set_transient_for(GTK_WINDOW(window),GTK_WINDOW(parent));
	}
}
/*****************************************************************************/
static gboolean create_cmd_pov(G_CONST_RETURN gchar* command, gchar* fileNameCMD, gchar* fileNamePov, gchar* fileNameIMG)
{
	gchar* commandStr = g_strdup(command);
        FILE* fcmd = NULL;
  	fcmd = FOpen(fileNameCMD, "w");
	if(!fcmd)
	{
  		Message("\nI can not create cmd file\n ","Error",TRUE);   
		return FALSE;
	}
#ifndef G_OS_WIN32
	fprintf(fcmd,"#!/bin/sh\n");
	fprintf(fcmd,"rm %s\n",fileNameIMG);
	fprintf(fcmd,"%s +I%s +O%s\n",commandStr, fileNamePov, fileNameIMG);
#else
	/* fprintf(fcmd,"setlocal\n");*/
	fprintf(fcmd,"set PATH=\"%s\";%cPATH%c\n",povrayDirectory,'%','%');
	fprintf(fcmd,"del %s\n",fileNameIMG);
	fprintf(fcmd,"%s +I%s +O%s\n",commandStr, fileNamePov, fileNameIMG);
	/* fprintf(fcmd,"endlocal");*/
#endif
	fclose(fcmd);
#ifndef G_OS_WIN32
	{
		gchar buffer[BSIZE];
  		sprintf(buffer,"chmod u+x %s",fileNameCMD);
		{int ierr = system(buffer);}
	}
#endif
	if(commandStr) g_free(commandStr);
	return TRUE;
}
/*****************************************************************************/
static void exportPOVRay(GtkWidget* Win, gboolean runPovray)
{
	gchar* fileNamePOV = NULL;
	gchar* fileNameIMG = NULL;
	gchar* fileNameCMD = NULL;
	GtkWidget *entryFileName = g_object_get_data(G_OBJECT (Win), "EntryFileName");
	GtkWidget *buttonDirSelector =g_object_get_data(G_OBJECT (Win), "ButtonDirSelector");
	GtkWidget *entryCommand = g_object_get_data(G_OBJECT (Win), "EntryCommand");
	GtkWidget *parent = g_object_get_data(G_OBJECT (Win), "ParentWindow");
	/* fileName */
	if(entryFileName && buttonDirSelector )
	{
		gchar* dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFileName)));
		gint l = 0;
		gint i;
		if(tmp) l = strlen(tmp);
		for(i=l-1;i>=1;i--) if(tmp[i]=='.') tmp[i]='\0';
		if(dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
		{
			fileNamePOV = g_strdup_printf("%s%s%s.pov",dirName, G_DIR_SEPARATOR_S,tmp);
			fileNameIMG = g_strdup_printf("%s%s%s.png",dirName, G_DIR_SEPARATOR_S,tmp);
#ifndef G_OS_WIN32
			fileNameCMD = g_strdup_printf("%s%s%s.cmd",dirName, G_DIR_SEPARATOR_S,tmp);
#else
			fileNameCMD = g_strdup_printf("%s%s%s.bat",dirName, G_DIR_SEPARATOR_S,tmp);
#endif
		}
		else
		{
			fileNamePOV = g_strdup_printf("%s%s.pov",dirName, tmp);
			fileNameIMG = g_strdup_printf("%s%s.png",dirName, tmp);
#ifndef G_OS_WIN32
			fileNameCMD = g_strdup_printf("%s%s.cmd",dirName, tmp);
#else
			fileNameCMD = g_strdup_printf("%s%s.bat",dirName, tmp);
#endif
		}
		g_free(tmp);
		g_free(dirName);
	}
	if(fileNamePOV)
	{
		applyPovrayOptions(NULL,NULL);
		gchar* message = create_povray_file(fileNamePOV, FALSE, TRUE);
		if(message)
		{
    			GtkWidget *m = Message(message,"Error",TRUE);
			gtk_window_set_modal (GTK_WINDOW (m), TRUE);
		}
		else
		{
			G_CONST_RETURN gchar* command = gtk_entry_get_text(GTK_ENTRY(entryCommand));
			if(create_cmd_pov(command, fileNameCMD, fileNamePOV, fileNameIMG))
			{
				 if(runPovray)
				{
					gint width = 500;
					gint height = 500;
					if(GLArea)
					{
						width =  GLArea->allocation.width;
						height = GLArea->allocation.height;
					}
					gtk_widget_hide(Win);
					while( gtk_events_pending() ) gtk_main_iteration();
					{ int ierr = system(fileNameCMD);}
					create_images_window (parent,fileNameIMG, width, height);
				}
				else
				{
					gchar* t = g_strdup_printf(
						"\n2 files was created :\n"
						" -\"%s\" a povray input file\n"
						" -\"%s\" a batch file for run povray\n",
						fileNamePOV,fileNameCMD);
					GtkWidget* winDlg = Message(t,"Info",TRUE);
					gtk_window_set_modal (GTK_WINDOW (winDlg), TRUE);
					g_free(t);
				}
			}
			else
			{
				gchar* t = g_strdup_printf("\nSorry, I cannot create the %s file\n",fileNameCMD);
				GtkWidget* winDlg = Message(t,"Info",TRUE);
				gtk_window_set_modal (GTK_WINDOW (winDlg), TRUE);
				g_free(t);
			}
		}
	}
	gtk_widget_destroy(Win);
}
/*****************************************************************************/
static void savePOVRay(GtkWidget* Win, gpointer data)
{
	gboolean runPovray = FALSE;
	exportPOVRay(Win, runPovray);
}
/*****************************************************************************/
static void runPOVRay(GtkWidget* Win, gpointer data)
{
	gboolean runPovray = TRUE;
	exportPOVRay(Win, runPovray);
}
/**********************************************************************/
static void AddPOVRayLocationDlg(GtkWidget *box, GtkWidget *Win)
{
	gint i = 0;
	gint j = 0;
	GtkWidget *label;
	GtkWidget *buttonDirSelector;
	GtkWidget *entryFileName;
	GtkWidget *table;

	table = gtk_table_new(2,3,FALSE);
	gtk_box_pack_start (GTK_BOX (box), table, TRUE, TRUE, 0);

	i++;
	j = 0;
	add_label_table(table,"Folder",(gushort)i,(gushort)j);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	buttonDirSelector =  gabedit_dir_button();
	/* gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (buttonDirSelector), g_getenv("PWD"));*/
	gtk_widget_set_size_request(GTK_WIDGET(buttonDirSelector),(gint)(ScreenHeightD*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,"File name",(gushort)i,(gushort)j);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryFileName = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),entryFileName,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_entry_set_text(GTK_ENTRY(entryFileName),"gabeditPOV");
	g_object_set_data(G_OBJECT (Win), "EntryFileName",entryFileName);
	g_object_set_data(G_OBJECT (Win), "ButtonDirSelector",buttonDirSelector);
}
/************************************************************************************************************/
static void AddPOVRayRunDlg(GtkWidget *box, GtkWidget *Win)
{
	gint i = 0;
	gint j = 0;
	GtkWidget *entryCommand;
	GtkWidget *table;
	GtkWidget* label;
	gint width = 500;
	gint height = 500;
	gchar* tmp = NULL;

	if(GLArea)
	{
		width =  GLArea->allocation.width;
		height = GLArea->allocation.height;
	}

	table = gtk_table_new(2,3,FALSE);
	gtk_box_pack_start (GTK_BOX (box), table, TRUE, TRUE, 0);

	i = 0;
	j = 0;
	label = gtk_label_new ("Command for run povray : ");
	gtk_table_attach(GTK_TABLE(table),label,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);

	j++;
	entryCommand = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table),entryCommand,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  1,1);
	tmp = g_strdup_printf("%s +W%d +H%d",NameCommandPovray, width,height);
	gtk_entry_set_text(GTK_ENTRY(entryCommand),tmp);
	g_free(tmp);
	g_object_set_data(G_OBJECT (Win), "EntryCommand",entryCommand);
    	gtk_widget_set_size_request(GTK_WIDGET(entryCommand),400,-1);
}
/**********************************************************************/
void exportPOVDlg(GtkWidget *parentWindow)
{
	GtkWidget *button;
	GtkWidget *Win;
	gchar* title = "POV Ray export";
	GtkWidget *hseparator = NULL;

	Win= gtk_dialog_new ();
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_title(&GTK_DIALOG(Win)->window,title);
    	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)gtk_widget_destroy,NULL);
 
	createPOVBackgroundFrame(GTK_WIDGET (GTK_DIALOG(Win)->vbox));
	AddPOVRayLocationDlg(GTK_WIDGET (GTK_DIALOG(Win)->vbox), Win);

	hseparator = gtk_hseparator_new ();
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->vbox), hseparator, TRUE, TRUE, 0);
	AddPOVRayRunDlg(GTK_WIDGET (GTK_DIALOG(Win)->vbox), Win);
  

	gtk_widget_realize(Win);

	button = create_button(Win,"Cancel");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,"Save");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)savePOVRay,GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,"Run PovRay");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPOVRay,GTK_OBJECT(Win));
	gtk_widget_show (button);

	gtk_widget_show_all(Win);
	g_object_set_data(G_OBJECT (Win), "ParentWindow",parentWindow);
}
