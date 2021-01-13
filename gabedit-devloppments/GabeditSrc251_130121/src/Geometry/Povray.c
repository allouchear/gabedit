/* Povray.c */
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
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Geometry/InterfaceGeom.h"
#include "../Utils/Utils.h"
#include "../Utils/PovrayUtils.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Vector3d.h"
#include "../Utils/Constants.h"
#include "../Utils/Transformation.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/Measure.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Common/Windows.h"

#define PRECISON_CYLINDER 0.001

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
static gchar *get_pov_matrix_transformation()
{
	gdouble q[4];
	gdouble m[4][4];
	gchar* temp;

	getQuatGeom(q);
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
/*****************************************************************************/
static gdouble get_epaisseur(gint i, gint j)
{
	gdouble factorstick = get_factorstick();
        gdouble ei = 1.0/3.0*factorstick;
        gdouble ej = 1.0/3.0*factorstick;
        gdouble e = 1.0/3.0*factorstick;
	gdouble sl = 4.5;
	gdouble sm = 2;
	if(geometry[i].Layer == LOW_LAYER) ei /= sl;
	if(geometry[i].Layer == MEDIUM_LAYER) ei /= sm;
	if(geometry[j].Layer == LOW_LAYER) ej /= sl;
	if(geometry[j].Layer == MEDIUM_LAYER) ej /= sm;
	e = (ei<ej)?ei:ej;
	return e;
}
/*****************************************************************************/
static gdouble get_rayon(gint i)
{
        gdouble rayon;
	gdouble factorball = get_factorball();
	gdouble sl = 4.5;
	gdouble sm = 2;
        if ( !stick_mode() && geometry[i].Layer != LOW_LAYER )
        { 
		if(space_fill_mode()) rayon =(geometry[i].Prop.vanDerWaalsRadii*factorball);
		else rayon =(geometry[i].Prop.radii*factorball);
		if(geometry[i].Layer == LOW_LAYER) rayon /= sl;
		if(geometry[i].Layer == MEDIUM_LAYER) rayon /= sm;
	}
	else rayon = get_epaisseur(i,i);
	if(rayon<0.01) rayon = 0.01;
	return rayon;
}
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
static XYZRC get_prop_center(gint Num)
{
        XYZRC PropCenter;

	PropCenter.C[0]=geometry[Num].X;
	PropCenter.C[1]=geometry[Num].Y;
	PropCenter.C[2]=geometry[Num].Z;
	PropCenter.C[3]=geometry[Num].Prop.radii;

	PropCenter.P.Colors[0]=(gdouble)(geometry[Num].Prop.color.red/65535.0);
	PropCenter.P.Colors[1]=(gdouble)(geometry[Num].Prop.color.green/65535.0);
	PropCenter.P.Colors[2]=(gdouble)(geometry[Num].Prop.color.blue/65535.0);

        return  PropCenter;
}
/********************************************************************************/
static XYZRC get_prop_dipole(gint i)
{
        XYZRC PropCenter;

	PropCenter.C[0]=dipole[i][0];
	PropCenter.C[1]=dipole[i][1];
	PropCenter.C[2]=dipole[i][2];
	PropCenter.C[3]=geometry[0].Prop.radii*get_factor()/4*get_factorstick();

	if(i<NDIVDIPOLE-1)
	{
		PropCenter.P.Colors[0]=0;
		PropCenter.P.Colors[1]=0;
		PropCenter.P.Colors[2]=(gdouble)(50000.0/65535.0);
	}
	else
	{
		PropCenter.P.Colors[0]=(gdouble)(50000.0/65535.0);
		PropCenter.P.Colors[1]=0;
		PropCenter.P.Colors[2]=0;
	}

        return  PropCenter;
}
/********************************************************************************/
static gchar *get_pov_cone_dipole()
{
     XYZRC C1 = get_prop_dipole(NDIVDIPOLE-NDIVDIPOLE/5);
     XYZRC C2 = get_prop_dipole(NDIVDIPOLE-1);
     gdouble ep = C1.C[3]*2;
     if(degenerated_cylinder(C1.C, C2.C)) return g_strdup(" ");
     gchar* temp = g_strdup_printf(
		"cone\n"
		"{\n"
		"\t<%14.6f,%14.6f,%14.6f>,%14.6f\n"
		"\t<%14.6f,%14.6f,%14.6f>,0.0 \n"
		"\ttexture { finish { Dull } }\n"
		"\tpigment { rgb<%14.6f,%14.6f,%14.6f> }\n}\n",
		C1.C[0],C1.C[1],C1.C[2],ep,
		C2.C[0],C2.C[1],C2.C[2],
		C2.P.Colors[0],C2.P.Colors[1],C2.P.Colors[2]
		);
     return temp;
}
/********************************************************************************/
static gchar *get_pov_ball(gint num, gdouble scale)
{
     gchar *temp;
     XYZRC Center = get_prop_center(num);
     temp = g_strdup_printf(
		"sphere\n"
		"{\n"
		"\t<%14.6f,%14.6f,%14.6f> %14.6f\n"
		"\ttexture { finish { Dull } }\n"
		"\tpigment { rgb<%14.6f,%14.6f,%14.6f> }\n}\n"
		,
		Center.C[0],Center.C[1],Center.C[2],Center.C[3]*get_factorball()*scale,
		Center.P.Colors[0], Center.P.Colors[1], Center.P.Colors[2]
		);
     return temp;
}
/********************************************************************************/
static gchar *get_pov_ball_for_stick(gint num, gdouble scale)
{
     gchar *temp;
     XYZRC Center = get_prop_center(num);
     temp = g_strdup_printf(
		"sphere\n"
		"{\n"
		"\t<%14.6f,%14.6f,%14.6f> %14.6f\n"
		"\ttexture { finish { Dull } }\n"
		"\tpigment { rgb<%14.6f,%14.6f,%14.6f> }\n}\n"
		,
		Center.C[0],Center.C[1],Center.C[2],scale*get_rayon(num),
		Center.P.Colors[0], Center.P.Colors[1], Center.P.Colors[2]
		);
     return temp;
}
/********************************************************************************/
static gdouble get_min(gint k)
{
     gdouble min=0;
     gint i=0;

     if(k==0)
     {
     	min = geometry[0].X;
     	for(i=1;i<(gint)Natoms;i++) if(min>geometry[i].X) min = geometry[i].X;
     }
     else
     {
     	if(k==1)
     	{
     		min = geometry[0].Y;
     		for(i=1;i<(gint)Natoms;i++) if(min>geometry[i].Y) min = geometry[i].Y;
     	}
     	else
     	{
     		min = geometry[0].Z;
     		for(i=1;i<(gint)Natoms;i++) if(min>geometry[i].Z) min = geometry[i].Z;
     	}
     }
     
     min -=100;
    return min;
}
/********************************************************************************/
static gchar *get_pov_cylingre(gdouble C1[],gdouble C2[],gdouble Colors[],gdouble ep)
{
     gchar* temp = g_strdup_printf(
		"cylinder\n"
		"{\n"
		"\t<%14.6f,%14.6f,%14.6f>,\n"
		"\t<%14.6f,%14.6f,%14.6f> \n"
		"\t%14.6f\n"
		"\ttexture { finish { Dull } }\n"
		"\tpigment { rgb<%14.6f,%14.6f,%14.6f> }\n}\n",
		C1[0],C1[1],C1[2],
		C2[0],C2[1],C2[2],
		ep,
		Colors[0],Colors[1],Colors[2]
		);
	return temp;

}
/********************************************************************************/
static gchar *get_pov_stick_dipole()
{
     gchar *temp;
     XYZRC Center1;
     XYZRC Center2;
     gdouble ep;

     Center1 = get_prop_dipole(0);
     Center2 = get_prop_dipole(NDIVDIPOLE-NDIVDIPOLE/5);
     ep = Center1.C[3];
     if(degenerated_cylinder(Center1.C, Center2.C)) return g_strdup(" ");

 
      temp = get_pov_cylingre(Center1.C,Center2.C,Center1.P.Colors,ep);
      return temp;
}
/********************************************************************************/
static gchar *get_pov_one_stick_for_ball(gint i,gint j)
{
     gchar *temp;
     gchar *temp1;
     gchar *temp2;
     XYZRC Center1;
     XYZRC Center2;
     gint l;
     gdouble ep;
     gdouble poid1;
     gdouble poid2;
     gdouble poid;
     gdouble C[3];
     gint nc = get_connection_type(i,j);

     if(nc<1) return " ";

     if(!getShowMultipleBonds()) nc = 1;

     Center1 = get_prop_center(i);
     Center2 = get_prop_center(j);
 
     ep =get_epaisseur(i,j);
 
     poid1 = geometry[i].Prop.covalentRadii+geometry[i].Prop.radii;
     poid2 = geometry[j].Prop.covalentRadii+geometry[j].Prop.radii;
     poid = poid1 + poid2 ;

     if(nc==3)
     {
	gchar* t;
/*
  	V3d vScal = {ep*0.5,ep*0.5,ep*0.5};
	gdouble C1[3];
	gdouble C2[3];
	V3d cros;
	V3d sub;
	V3d C0={0,0,0};
	gdouble C10[3];
	gdouble C20[3];
	gdouble CC1[3];
	gdouble CC2[3];
	for(l=0;l<3;l++) CC1[l] = Center1.C[l];
	for(l=0;l<3;l++) CC2[l] = Center2.C[l];
	v3d_sub(C0, CC1, C10);
	v3d_sub(C0, CC2, C20);
	v3d_cross(C10, C20, cros);
	v3d_sub(CC1, CC2, sub);
	v3d_cross(cros, sub, vScal);
	if(v3d_dot(vScal,vScal)!=0)
	{
		v3d_normal(vScal);
		v3d_scale(vScal, ep*0.5);
	}
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
	t = temp;
      	temp = g_strdup_printf("%s%s%s",t,temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
      	g_free(t);

	for(l=0;l<3;l++) C1[l] = Center1.C[l]+vScal[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l]+vScal[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep/3);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep/3);
	t = temp;
      	temp = g_strdup_printf("%s%s%s",t,temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
      	g_free(t);
*/
		V3d C0;
		gdouble r = ep;
		gdouble C11[3];
		gdouble C12[3];
		gdouble C21[3];
		gdouble C22[3];
		gdouble C31[3];
		gdouble C32[3];
		gdouble C[3];
		gdouble rs[3];
		gdouble Ci[3];
		gdouble Cj[3];
		gint type = 1;
		if(geometry[i].Layer == LOW_LAYER || geometry[j].Layer == LOW_LAYER) type = 0;
		getOptimalCiCj(i, j, Ci, Cj,C0);
		getPositionsRadiusBond3(r, C0, Ci, Cj, C11, C12,  C21,  C22, C31, C32, rs, type);

     		for(l=0;l<3;l++) C[l] =(C11[l]*poid2+C12[l]*poid1)/poid;
      		temp1 = get_pov_cylingre(C11,C,Center1.P.Colors,rs[0]);
      		temp2 = get_pov_cylingre(C,C12,Center2.P.Colors,rs[0]);
      		temp = g_strdup_printf("%s%s",temp1,temp2);
      		g_free(temp1);
      		g_free(temp2);

     		for(l=0;l<3;l++) C[l] =(C21[l]*poid2+C22[l]*poid1)/poid;
      		temp1 = get_pov_cylingre(C21,C,Center1.P.Colors,rs[1]);
      		temp2 = get_pov_cylingre(C,C22,Center2.P.Colors,rs[1]);
		t = temp;
      		temp = g_strdup_printf("%s%s%s",t,temp1,temp2);
      		g_free(temp1);
      		g_free(temp2);
      		g_free(t);

     		for(l=0;l<3;l++) C[l] =(C31[l]*poid2+C32[l]*poid1)/poid;
      		temp1 = get_pov_cylingre(C31,C,Center1.P.Colors,rs[2]);
      		temp2 = get_pov_cylingre(C,C32,Center2.P.Colors,rs[2]);
		t = temp;
      		temp = g_strdup_printf("%s%s%s",t,temp1,temp2);
      		g_free(temp1);
      		g_free(temp2);
      		g_free(t);

     }
     else
     if(nc==2)
     {
	gchar* t;
	gdouble r = ep;
	gdouble C11[3];
	gdouble C12[3];
	gdouble C21[3];
	gdouble C22[3];
	gdouble C[3];
	V3d C0;
	gdouble rs[3];
	gint type = 1;
	if(geometry[i].Layer == LOW_LAYER || geometry[j].Layer == LOW_LAYER) type = 0;

	getOptimalCiCj(i, j, Center1.C, Center2.C, C0);
	getPositionsRadiusBond2(r, C0, Center1.C, Center2.C, C11, C12,  C21,  C22, rs, type);

     	for(l=0;l<3;l++) C[l] =(C11[l]*poid2+C12[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C11,C,Center1.P.Colors,rs[0]);
      	temp2 = get_pov_cylingre(C,C12,Center2.P.Colors,rs[0]);
      	temp = g_strdup_printf("%s%s",temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);

     	for(l=0;l<3;l++) C[l] =(C21[l]*poid2+C22[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C21,C,Center1.P.Colors,rs[1]);
      	temp2 = get_pov_cylingre(C,C22,Center2.P.Colors,rs[1]);
	t = temp;
      	temp = g_strdup_printf("%s%s%s",t,temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
      	g_free(t);

     }
     else
     {
     	for(l=0;l<3;l++) C[l] =(Center1.C[l]*poid2+Center2.C[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(Center1.C,C,Center1.P.Colors,ep);
      	temp2 = get_pov_cylingre(C,Center2.C,Center2.P.Colors,ep);
      	temp = g_strdup_printf("%s%s",temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
     }

      return temp;
}
/********************************************************************************/
static gchar *get_pov_one_stick(gint i,gint j)
{
     gchar *temp;
     gchar *temp1;
     gchar *temp2;
     XYZRC Center1;
     XYZRC Center2;
     gint l;
     gdouble ep;
     gdouble poid1;
     gdouble poid2;
     gdouble poid;
     gdouble C[3];
     gint nc = get_connection_type(i,j);

     if(nc<1) return " ";
     if(!getShowMultipleBonds()) nc = 1;

     Center1 = get_prop_center(i);
     Center2 = get_prop_center(j);
     get_num_min_rayonIJ(i,j);
 
     ep =get_epaisseur(i,j);
 
     poid1 = geometry[i].Prop.covalentRadii+geometry[i].Prop.radii;
     poid2 = geometry[j].Prop.covalentRadii+geometry[j].Prop.radii;
     poid = poid1 + poid2 ;

     if(nc==3)
     {
	gchar* t;
/*
  	V3d vScal = {ep,ep,ep};
	gdouble C1[3];
	gdouble C2[3];
	gdouble C12[3];
	V3d cros;
	V3d sub;
	V3d C0={0,0,0};
	gdouble C10[3];
	gdouble C20[3];
	gdouble CC1[3];
	gdouble CC2[3];
	for(l=0;l<3;l++) CC1[l] = Center1.C[l];
	for(l=0;l<3;l++) CC2[l] = Center2.C[l];
	v3d_sub(C0, CC1, C10);
	v3d_sub(C0, CC2, C20);
	v3d_cross(C10, C20, cros);
	v3d_sub(CC1, CC2, sub);
	v3d_cross(cros, sub, vScal);
	if(v3d_dot(vScal,vScal)!=0)
	{
		v3d_normal(vScal);
		v3d_scale(vScal, ep*2);
	}
	for(l=0;l<3;l++) C1[l] = Center1.C[l]-vScal[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l]-vScal[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;

	v3d_sub(CC1, CC2, C12);
	if(v3d_dot(C12,C12)!=0)
	{
		v3d_normal(C12);
	}
	for(l=0;l<3;l++) C1[l] -= C12[l]*ep;
	for(l=0;l<3;l++) C2[l] += C12[l]*ep;

      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep/2);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep/2);
      	temp = g_strdup_printf("%s%s",temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);


	for(l=0;l<3;l++) C1[l] = Center1.C[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep);
	t = temp;
      	temp = g_strdup_printf("%s%s%s",t,temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
      	g_free(t);

	for(l=0;l<3;l++) C1[l] = Center1.C[l]+vScal[l];
	for(l=0;l<3;l++) C2[l] = Center2.C[l]+vScal[l];
     	for(l=0;l<3;l++) C[l] =(C1[l]*poid2+C2[l]*poid1)/poid;

	for(l=0;l<3;l++) C1[l] -= C12[l]*ep;
	for(l=0;l<3;l++) C2[l] += C12[l]*ep;

      	temp1 = get_pov_cylingre(C1,C,Center1.P.Colors,ep/2);
      	temp2 = get_pov_cylingre(C,C2,Center2.P.Colors,ep/2);
	t = temp;
      	temp = g_strdup_printf("%s%s%s",t,temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
      	g_free(t);
*/
		V3d C0;
		gdouble r = ep;
		gdouble C11[3];
		gdouble C12[3];
		gdouble C21[3];
		gdouble C22[3];
		gdouble C31[3];
		gdouble C32[3];
		gdouble C[3];
		gdouble rs[3];
		gdouble Ci[3];
		gdouble Cj[3];
		gint type = 0;
		getOptimalCiCj(i, j, Ci, Cj,C0);
		getPositionsRadiusBond3(r, C0, Ci, Cj, C11, C12,  C21,  C22, C31, C32, rs, type);

     		for(l=0;l<3;l++) C[l] =(C11[l]*poid2+C12[l]*poid1)/poid;
      		temp1 = get_pov_cylingre(C11,C,Center1.P.Colors,rs[0]);
      		temp2 = get_pov_cylingre(C,C12,Center2.P.Colors,rs[0]);
      		temp = g_strdup_printf("%s%s",temp1,temp2);
      		g_free(temp1);
      		g_free(temp2);

     		for(l=0;l<3;l++) C[l] =(C21[l]*poid2+C22[l]*poid1)/poid;
      		temp1 = get_pov_cylingre(C21,C,Center1.P.Colors,rs[1]);
      		temp2 = get_pov_cylingre(C,C22,Center2.P.Colors,rs[1]);
		t = temp;
      		temp = g_strdup_printf("%s%s%s",t,temp1,temp2);
      		g_free(temp1);
      		g_free(temp2);
      		g_free(t);

     		for(l=0;l<3;l++) C[l] =(C31[l]*poid2+C32[l]*poid1)/poid;
      		temp1 = get_pov_cylingre(C31,C,Center1.P.Colors,rs[2]);
      		temp2 = get_pov_cylingre(C,C32,Center2.P.Colors,rs[2]);
		t = temp;
      		temp = g_strdup_printf("%s%s%s",t,temp1,temp2);
      		g_free(temp1);
      		g_free(temp2);
      		g_free(t);

     }
     else
     if(nc==2)
     {
	gchar* t;
	gdouble r = ep;
	gdouble C11[3];
	gdouble C12[3];
	gdouble C21[3];
	gdouble C22[3];
	gdouble C[3];
	gdouble rs[3];
	gint type = 0;
	V3d C0;
	getOptimalCiCj(i, j, Center1.C, Center2.C, C0);
	getPositionsRadiusBond2(r, C0, Center1.C, Center2.C, C11, C12,  C21,  C22, rs, type);

     	for(l=0;l<3;l++) C[l] =(C11[l]*poid2+C12[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C11,C,Center1.P.Colors,rs[0]);
      	temp2 = get_pov_cylingre(C,C12,Center2.P.Colors,rs[0]);
      	temp = g_strdup_printf("%s%s",temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);

     	for(l=0;l<3;l++) C[l] =(C21[l]*poid2+C22[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(C21,C,Center1.P.Colors,rs[1]);
      	temp2 = get_pov_cylingre(C,C22,Center2.P.Colors,rs[1]);
	t = temp;
      	temp = g_strdup_printf("%s%s%s",t,temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
      	g_free(t);
     }
     else
     {
     	for(l=0;l<3;l++) C[l] =(Center1.C[l]*poid2+Center2.C[l]*poid1)/poid;
      	temp1 = get_pov_cylingre(Center1.C,C,Center1.P.Colors,ep);
      	temp2 = get_pov_cylingre(C,Center2.C,Center2.P.Colors,ep);
      	temp = g_strdup_printf("%s%s",temp1,temp2);
      	g_free(temp1);
      	g_free(temp2);
     }

      return temp;
}
/********************************************************************************/
static gchar *get_pov_one_hbond(gint i,gint j)
{
     gchar *temp;
     XYZRC Center1;
     XYZRC Center2;
     gint l;
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
     gdouble aspect = 0.3;

     if( !hbond_connections(i,j)) return " ";

     Center1 = get_prop_center(i);
     Center2 = get_prop_center(j);
 
     if(geometry[i].Prop.radii<geometry[j].Prop.radii) ep = geometry[i].Prop.radii*aspect;
     else ep = geometry[j].Prop.radii*aspect;

 
     poid1 = geometry[i].Prop.covalentRadii+geometry[i].Prop.radii;
     poid2 = geometry[j].Prop.covalentRadii+geometry[j].Prop.radii;
     poid = poid1 + poid2 ;

     ibreak = (gint)(poid1*n/poid);
     ibreak = n/2;

     for(l=0;l<3;l++) K[l] =(Center2.C[l]-Center1.C[l])/(n);
     for(l=0;l<3;l++) A[l] =Center1.C[l];
     temp = NULL;
     for(i=0;i<n;i++)
     {
     	for(l=0;l<3;l++) B[l] = A[l] + K[l];
	if(i%2==0)
	{
		if(i<=ibreak) temp1 =  get_pov_cylingre(A,B,Center1.P.Colors,ep/2);
		else temp1 =  get_pov_cylingre(A,B,Center2.P.Colors,ep/2);
		dump = temp;
		if(dump)
		{
     			temp = g_strdup_printf("%s%s",dump,temp1);
			g_free(dump);
		}
		else temp = g_strdup_printf("%s",temp1);
		g_free(temp1);
		temp1 = NULL;
	}
     	for(l=0;l<3;l++) A[l] = B[l];
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

	get_camera_values_drawgeom(&zn, &zf, &angle, &aspect, &perspective);
	get_orgin_molecule_drawgeom(origin);
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
	/* if(!perspective) fov = angle;*/
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
         			"{\n",
				 x,y,z,color);
	return temp;           
}
/********************************************************************************/
static gchar *get_pov_light_sources()
{
     gchar *temp;
     gchar *dum1;
     gchar *dum2;
     gdouble Ymax;
     gint i=0;
/* calcul of Ymax*/

     Ymax = geometry[0].Y;
     for(i=1;i<(gint)Natoms;i++)
		if(Ymax<geometry[i].Y)
			Ymax = geometry[i].Y;
     
     Ymax =Ymax*2+100;
     printf("Ymax = %f\n",Ymax);
     /* dum1 = get_pov_light_source("// LIGHT 1\n","0.8*White",0,Ymax/10,Ymax);*/
     dum1 = g_strdup(" ");
     dum2 = get_pov_light_source("// LIGHT 1\n","0.8*White",0,0,Ymax);
     temp = g_strdup_printf("%s %s",dum1,dum2);
     g_free(dum1);
     g_free(dum2);
     return temp;
}
/********************************************************************************/
static gchar *get_pov_atoms(gdouble scal)
{
	gdouble factorball = get_factorball();
     	gchar *temp=NULL;
     	gchar *tempold=NULL;
     	gchar *t=NULL;
     	gint i=0;
     	temp = g_strdup( "// ATOMS \n");
	for(i=0;i<(gint)Natoms;i++)
	{
 		gdouble r = scal*get_rayon(i)/(geometry[i].Prop.radii*factorball);
		if(!geometry[i].show) continue;
		tempold = temp;
		t =get_pov_ball(i,r);
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
static gchar *get_pov_atoms_for_stick(gdouble scal)
{
     	gchar *temp=NULL;
     	gchar *tempold=NULL;
     	gchar *t=NULL;
     	gint i=0;
     	temp = g_strdup( "// ATOMS \n");
	for(i=0;i<(gint)Natoms;i++)
	{
		if(!geometry[i].show) continue;
		tempold = temp;
		t =get_pov_ball_for_stick(i,scal);
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
static gchar *get_pov_bonds(gboolean ballstick)
{
     gchar *temp = NULL;
     gint i,j;
     gchar* t;
     gchar* tempold;
     gboolean* Ok = NULL;
     if(Natoms>0) Ok = g_malloc(Natoms*sizeof(gboolean));
     for(i=0;i<(gint)Natoms;i++) Ok[i] = FALSE;

     temp = g_strdup( "// BONDS \n");
     for(i=0;i<(gint)(Natoms-1);i++)
     {
		if(!geometry[i].show) continue;
	for(j=i+1;j<(gint)Natoms;j++)
		if(get_connection_type(i,j)>0 && geometry[i].show && geometry[j].show)
	 	{
			Ok[i] = TRUE;
			Ok[j] = TRUE;
			tempold = temp;
			if(ballstick) t =get_pov_one_stick_for_ball(i,j);
			else t =get_pov_one_stick(i,j);
			if(tempold)
                	{
                        	temp = g_strdup_printf("%s%s",tempold,t);
                        	g_free(tempold);
                	}
                	else temp = g_strdup_printf("%s",t); 
	 	}
     		else
		{
			if(hbond_connections(i,j) && geometry[i].show && geometry[j].show)
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
		}
     }
     for(i=0;i<(gint)(Natoms-1);i++)
	if(!Ok[i] && geometry[i].show)
	{
		tempold = temp;
		t =get_pov_ball(i, 0.5);
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
static gchar *get_pov_dipole()
{
	gchar* t1;
	gchar* t2;
	gchar* t = NULL;

     	t = g_strdup( "// Dipole \n");
	t1 = t;
	t2 = get_pov_stick_dipole();
	t = g_strdup_printf("%s%s",t1,t2);
	g_free(t1);
	g_free(t2);
	t1 = t;
	t2 = get_pov_cone_dipole();
	t = g_strdup_printf("%s%s",t1,t2);
	g_free(t1);
	g_free(t2);
	return t;
}
/*****************************************************************************/
static gchar* get_pov_box()
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
	gdouble O[3];
     	gchar *temp;
     	gchar *temp_all;
     	XYZRC Center1;
     	XYZRC Center2;
	gdouble Orig[3]= {0,0,0};
	gchar* t;

	if(!testShowBoxGeom()) return g_strdup("\n");
	for(i=0;i<Natoms;i++)
	{
		sprintf(tmp,"%s",geometry[i].Prop.symbol);
		uppercase(tmp);
		if(!strcmp(tmp,"TV")) { iTv[nTv]= i; nTv++;}
	}
	if(nTv<2) return g_strdup("\n");
	get_origine_molecule_drawgeom(Orig);
	for(i=0;i<3;i++) O[i] = -Orig[i];
	for(i=0;i<nTv;i++)
	{
		Tv[i][0] = geometry[iTv[i]].X-O[0];
		Tv[i][1] = geometry[iTv[i]].Y-O[1];
		Tv[i][2] = geometry[iTv[i]].Z-O[2];
	}
	for(i=0;i<3;i++) Base1Pos[i] = O[i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[0][i];
        temp_all = get_pov_cylingre(Base1Pos,Base2Pos,Specular,radius);

	for(i=0;i<3;i++) Base1Pos[i] = O[i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[1][i];
        temp = get_pov_cylingre(Base1Pos,Base2Pos,Specular,radius);
	t = temp_all; temp_all = g_strdup_printf("%s%s",t,temp); g_free(temp); g_free(t);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[0][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[1][i];
        temp = get_pov_cylingre(Base1Pos,Base2Pos,Specular,radius);
	t = temp_all; temp_all = g_strdup_printf("%s%s",t,temp); g_free(temp); g_free(t);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[1][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[0][i];
        temp = get_pov_cylingre(Base1Pos,Base2Pos,Specular,radius);
	t = temp_all; temp_all = g_strdup_printf("%s%s",t,temp); g_free(temp); g_free(t);
	if(nTv<3) return temp_all;

	for(i=0;i<3;i++) Base1Pos[i] = O[i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[2][i];
        temp = get_pov_cylingre(Base1Pos,Base2Pos,Specular,radius);
	t = temp_all; temp_all = g_strdup_printf("%s%s",t,temp); g_free(temp); g_free(t);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[0][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[2][i];
        temp = get_pov_cylingre(Base1Pos,Base2Pos,Specular,radius);
	t = temp_all; temp_all = g_strdup_printf("%s%s",t,temp); g_free(temp); g_free(t);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[2][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[0][i];
        temp = get_pov_cylingre(Base1Pos,Base2Pos,Specular,radius);
	t = temp_all; temp_all = g_strdup_printf("%s%s",t,temp); g_free(temp); g_free(t);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[1][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[2][i];
        temp = get_pov_cylingre(Base1Pos,Base2Pos,Specular,radius);
	t = temp_all; temp_all = g_strdup_printf("%s%s",t,temp); g_free(temp); g_free(t);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[2][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[1][i];
        temp = get_pov_cylingre(Base1Pos,Base2Pos,Specular,radius);
	t = temp_all; temp_all = g_strdup_printf("%s%s",t,temp); g_free(temp); g_free(t);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[0][i]+Tv[2][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[1][i];
        temp = get_pov_cylingre(Base1Pos,Base2Pos,Specular,radius);
	t = temp_all; temp_all = g_strdup_printf("%s%s",t,temp); g_free(temp); g_free(t);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[0][i]+Tv[1][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[2][i];
        temp = get_pov_cylingre(Base1Pos,Base2Pos,Specular,radius);
	t = temp_all; temp_all = g_strdup_printf("%s%s",t,temp); g_free(temp); g_free(t);

	for(i=0;i<3;i++) Base1Pos[i] = O[i]+Tv[1][i]+Tv[2][i];
	for(i=0;i<3;i++) Base2Pos[i] = Base1Pos[i]+Tv[0][i];
        temp = get_pov_cylingre(Base1Pos,Base2Pos,Specular,radius);
	t = temp_all; temp_all = g_strdup_printf("%s%s",t,temp); g_free(temp); g_free(t);

      return temp_all;

}
/********************************************************************************/
	
/********************************************************************************/
static gchar* export_to_povray(gchar* fileName)
{
  gchar *temp;
  gdouble xmin;
  gdouble ymin;
  gdouble zmin;
  FILE *fd=NULL;
  gchar* message = NULL;

 fd = FOpen(fileName, "w");
 if(fd)
 {
	temp =get_pov_epilogue();
	fprintf(fd,"%s",temp);
	g_free(temp);

	temp = get_pov_matrix_transformation();
	fprintf(fd,"%s",temp);
	g_free(temp);

	temp =get_pov_camera();
	fprintf(fd,"%s",temp);
	g_free(temp);
	temp = get_pov_light_sources();
	fprintf(fd,"%s",temp);
	g_free(temp);

	xmin = get_min(0);
	ymin = get_min(1);
	zmin = get_min(2);
	temp = get_pov_background(xmin, ymin,zmin);
	fprintf(fd,"%s",temp);
	g_free(temp);

     	if(Natoms<1)
	{
		fclose(fd);
	 	message = g_strdup_printf(_("\nSorry, The number of atoms should be >0\n"));
		return message;
	}
	temp = get_pov_begin_molecule();
	fprintf(fd,"%s",temp);
	g_free(temp);

	if( !stick_mode())
	{
		temp = get_pov_atoms(1.0); 
		fprintf(fd,"%s",temp);
		g_free(temp);
	}
	else
	{
		temp = get_pov_atoms_for_stick(get_factorstick()); 
		fprintf(fd,"%s",temp);
		g_free(temp);
	}
	temp = get_pov_bonds( !stick_mode()); 
	fprintf(fd,"%s",temp);
	g_free(temp);
        if(Ddef && dipole_mode())
	{
		temp = get_pov_dipole();
		fprintf(fd,"%s",temp);
		g_free(temp);
	}

	temp = get_pov_box();
	fprintf(fd,"%s",temp);
	g_free(temp);

	temp = get_pov_end_molecule();
	fprintf(fd,"%s",temp);
	g_free(temp);
	
 	fclose(fd);
 }
 else
 {
	 message = g_strdup_printf(_("\nSorry, I cannot create the %s file\n"),fileName);
 }
	return message;
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
  		Message(_("\nI can not create cmd file\n "),_("Error"),TRUE);   
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
		gchar* message = export_to_povray(fileNamePOV);
		if(message)
		{
    			GtkWidget *m = Message(message,_("Error"),TRUE);
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
					if(GeomDrawingArea->allocation.width)
					{
						width =  GeomDrawingArea->allocation.width;
						height = GeomDrawingArea->allocation.height;
					}
					gtk_widget_hide(Win);
					while( gtk_events_pending() ) gtk_main_iteration();
					{int ierr = system(fileNameCMD);}
					create_images_window (parent,fileNameIMG, width, height);
				}
				else
				{
					gchar* t = g_strdup_printf(
						_(
						"\n2 files was created :\n"
						" -\"%s\" a povray input file\n"
						" -\"%s\" a batch file for run povray\n")
						,fileNamePOV,fileNameCMD);
					GtkWidget* winDlg = Message(t,_("Info"),TRUE);
					gtk_window_set_modal (GTK_WINDOW (winDlg), FALSE);
					g_free(t);
				}
			}
			else
			{
				gchar* t = g_strdup_printf(_("\nSorry, I cannot create the %s file\n"),fileNameCMD);
				GtkWidget* winDlg = Message(t,_("Info"),TRUE);
				gtk_window_set_modal (GTK_WINDOW (winDlg), FALSE);
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
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (buttonDirSelector), g_getenv("PWD"));
	gtk_widget_set_size_request(GTK_WIDGET(buttonDirSelector),(gint)(ScreenHeight*0.2),-1);
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

	if(GeomDrawingArea)
	{
		width =  GeomDrawingArea->allocation.width;
		height = GeomDrawingArea->allocation.height;
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
void exportPOVGeomDlg(GtkWidget *parentWindow)
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
