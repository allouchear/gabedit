/*Transformation.c */
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
#include <math.h>
#include "../Common/Global.h"
#include "Vector3d.h"
#include "Transformation.h"
#include "Utils.h"

#define TRACKBALLSIZE  (0.8)

/* Local function prototypes */
static gdouble tb_project_to_sphere(gdouble, gdouble, gdouble);
/****************************************************/
gdouble *v4d_pvect(V4d v1,V4d v2)
{
	gdouble* v = g_malloc(4*sizeof(gdouble));
	v[0] = v1[1] * v2[2] - v2[1] * v1[2];
	v[1] = v1[2] * v2[0] - v2[2] * v1[0];
	v[2] = v1[0] * v2[1] - v2[0] * v1[1] ;
	v[3] = (v1[3] + v2[3])/2;
	return v;
}
/****************************************************/
gdouble v4d_pscal(V4d v1,V4d v2)
{
	return v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2]+v1[3]*v2[3];
}
/****************************************************/
gdouble* v4d_scal(V4d v1,gdouble scal)
{
	gdouble* v = g_malloc(4*sizeof(gdouble));
	v[0] = v1[0] * scal;
	v[1] = v1[1] * scal;
	v[2] = v1[2] * scal;
	v[3] = v1[3] * scal;
	return v;
}
/****************************************************/
gdouble v4d_length(V4d v)
{
	return sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]+v[3]*v[3]);
}
/****************************************************/
void v4d_normal(V4d v)
{
    int i;
    gdouble len;

    len = v4d_length(v);
    for (i = 0; i < 4; i++) 
		v[i] /= len;
}
/************************************************************************/
/*
 *  Given an axis and angle, compute quaternion.
 */
void axis_to_quat(gdouble a[3], gdouble phi, gdouble q[4])
{
    v3d_normal(a);
    v3d_copy(a,q);
    v3d_scale(q,sin(phi/2.0));
    q[3] = cos(phi/2.0);
}
/************************************************************************/
void trackball(gdouble q[4],gdouble p1x,gdouble p1y,gdouble p2x,gdouble p2y)
{
    gdouble a[3]; /* Axis of rotation */
    gdouble phi;  /* how much to rotate about axis */
    gdouble p1[3];
	gdouble p2[3];
	gdouble d[3];
    gdouble t;

    if (p1x == p2x && p1y == p2y) {
        /* Zero rotation */
        v3d_zero(q);
        q[3] = 1.0;
        return;
    }

    /*
     * First, figure out z-coordinates for projection of P1 and P2 to
     * deformed sphere
     */
    v3d_set(p1,p1x,p1y,tb_project_to_sphere(TRACKBALLSIZE,p1x,p1y));
    v3d_set(p2,p2x,p2y,tb_project_to_sphere(TRACKBALLSIZE,p2x,p2y));

    /*  Now, we want the cross product of P1 and P2 */
    v3d_cross(p2,p1,a);

    /* Figure out how much to rotate around that axis. */
    v3d_sub(p1,p2,d);
    t = v3d_length(d) / (2.0*TRACKBALLSIZE);

    /* Avoid problems with out-of-control values...*/
    if (t > 1.0) t = 1.0;
    if (t < -1.0) t = -1.0;
    phi = 2.0 * asin(t);

    axis_to_quat(a,phi,q);
}

/************************************************************************/
/*
 * Project an x,y pair onto a sphere of radius r 
 * OR a hyperbolic sheet
 * if we are away from the center of the sphere.
 */
static gdouble tb_project_to_sphere(gdouble r, gdouble x, gdouble y)
{
    gdouble d, t, z;

    d = sqrt(x*x + y*y);
    if (d < r * 0.70710678118654752440) {    /* Inside sphere */
        z = sqrt(r*r - d*d);
    } else {           /* On hyperbola */
        t = r / 1.41421356237309504880;
        z = t*t / d;
    }
    return z;
}
/************************************************************************/
/*
 * Given two rotations, e1 and e2, expressed as quaternion rotations,
 * figure out the equivalent single rotation and stuff it into dest.
 *
 * This routine also normalizes the result every RENORMCOUNT times it is
 * called, to keep error from creeping in.
 *
 * NOTE: This routine is written so that q1 or q2 may be the same
 * as dest (or each other).
 */

#define RENORMCOUNT 97

void add_quats(gdouble q1[4],gdouble q2[4],gdouble dest[4])
{
    static int count=0;
    gdouble t1[4];
	gdouble t2[4];
	gdouble t3[4];
	gdouble tf[4];

    v3d_copy(q1,t1);
    v3d_scale(t1,q2[3]);

    v3d_copy(q2,t2);
    v3d_scale(t2,q1[3]);

    v3d_cross(q2,q1,t3);
    v3d_add(t1,t2,tf);
    v3d_add(t3,tf,tf);
    tf[3] = q1[3] * q2[3] - v3d_dot(q1,q2);

    dest[0] = tf[0];
    dest[1] = tf[1];
    dest[2] = tf[2];
    dest[3] = tf[3];

    if (++count > RENORMCOUNT) {
        count = 0;
        v4d_normal(dest);
    }
}
/************************************************************************/
void build_rotmatrix(gdouble m[4][4],gdouble q[4])
{
    m[0][0] = 1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]);
    m[0][1] = 2.0 * (q[0] * q[1] - q[2] * q[3]);
    m[0][2] = 2.0 * (q[2] * q[0] + q[1] * q[3]);
    m[0][3] = 0.0;

    m[1][0] = 2.0 * (q[0] * q[1] + q[2] * q[3]);
    m[1][1]= 1.0 - 2.0 * (q[2] * q[2] + q[0] * q[0]);
    m[1][2] = 2.0 * (q[1] * q[2] - q[0] * q[3]);
    m[1][3] = 0.0;

    m[2][0] = 2.0 * (q[2] * q[0] - q[1] * q[3]);
    m[2][1] = 2.0 * (q[1] * q[2] + q[0] * q[3]);
    m[2][2] = 1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]);
    m[2][3] = 0.0;

    m[3][0] = 0.0;
    m[3][1] = 0.0;
    m[3][2] = 0.0;
    m[3][3] = 1.0;
}
/************************************************************************/
/**********************************************/
/* Name: Matrix Inversion by Gauss - Jordan*/
/**************************************************/
void Pivoting(gdouble **a,gint size,gint row,gint col)
{
    gdouble temp;
    gint s; 
    for(s=0;s<size*2;s++)
    {
        temp=a[row-1][s];
        a[row-1][s]=a[row][s];
        a[row][s]=temp;
    }
}
/**************************************************/
void Trunc(gdouble **a, gint size,gdouble error)
{
	gint s,k;
	for(s=0;s<size*2;s++)
	for(k=0;k<size;k++)
	{
        if(fabs(a[k][s])<error) a[k][s]=0;
    }
}
/**************************************************/
gdouble Factor(gdouble **a,gint row,gint col)
{
    return (gdouble) (-1*(a[row][col]/a[col][col]));
}
/**************************************************/
gdouble** Inverse(gdouble **mat,gint size,gdouble error)
{
	gdouble factor = 0.0;
	gdouble **invmat = NULL;
	gint i,j,ii;
	gdouble **a = g_malloc(size*sizeof(gdouble*));
	gint done=1;
    	gdouble diagonal;
    	gdouble temp;
    	gint s;

	for(i=0;i<size;i++)
	   a[i] = g_malloc(2*size*sizeof(gdouble));

	for(i=0;i<size;i++)
	{
		for(j=0;j<size;j++)
			a[i][j] = mat[i][j];
		for(j=size;j<2*size;j++)
			a[i][j] = 0.0;
		a[i][i+size] = 1.0;
	}

	Trunc(a,size,error);
	/*
	for(i=0;i<size;i++)
	{
		for(j=0;j<2*size;j++)
		   Debug("%f ",a[i][j] );
		Debug("\n ");
	}

	Debug("erro = %f \n ",error);
	*/
    for(j=0;j<size;j++)
    {
        for(i=j+1;i<size;i++)
	{
	/*	Debug("j i %d %d %f %f\n",j,i,a[j][j],a[i][i]);*/
		if(fabs(a[j][j]) <fabs(a[j][i]))
		{
	/*		Debug("pivoting %d %d\n",i,j);*/
    			for(s=0;s<size*2;s++)
    			{
        			temp=a[i][s];
        			a[i][s]=a[j][s];
        			a[j][s]=temp;
    			}
		}
	}
    }
    /*
	Debug("\n ");
	Debug("\n ");
	for(i=0;i<size;i++)
	{
		for(j=0;j<2*size;j++)
		   Debug("%f ",a[i][j] );
		Debug("\n ");
	}
	*/

    for(j=0;j<size;j++)
    {
        for(i=j+1;i<size;i++)
		{
			if(a[j][j]!=0)
				factor=Factor(a,i,j);
			else
			{
				Pivoting(a,size,i,j+1);
				factor=Factor(a,i,j);
			}
			for(ii=0;ii<size*2;ii++)
				a[i][ii]+=factor*a[j][ii];

			Trunc(a,size,error);
		}
	}
    /*
	Debug("\n ");
	Debug("\n ");
	for(i=0;i<size;i++)
	{
		for(j=0;j<2*size;j++)
		   Debug("%f ",a[i][j] );
		Debug("\n ");
	}
	*/
	for(j=size-1;j>=0;j--)
	{
		for(i=j-1;i>=0;i--)
		{
			if(a[j][j]==0) 
				done=0;
			else 
				factor=Factor(a,i,j);

			/*for(ii=0;ii<=2*size;ii++)*/
			for(ii=0;ii<2*size;ii++)
				a[i][ii] += factor*a[j][ii];

			Trunc(a,size,error);
		}
	}
	if(done  == 0) 
		printf("ERREUR\n");
	if(done != 0)
	{
		for(i=0;i<size;i++)
		{
			diagonal=a[i][i];
			for(j=0;j<size*2;j++)
				a[i][j]/=diagonal;
		}

		invmat = g_malloc(size*sizeof(gdouble*));
		for(i=0;i<size;i++)
		{
			invmat[i] = g_malloc(size*sizeof(gdouble));
			for(j=0;j<size;j++)
				invmat[i][j] = a[i][j+size];
		}

		/*
		for(i=0;i<size;i++)
		{
			for(j=0;j<2*size;j++)
				Debug("%f ",a[i][j] );
			Debug("\n ");
		}
		*/

	}

	
	for(i=0;i<size;i++)
	   g_free(a[i]);
	
	g_free(a);
	
	return invmat;
}
/**************************************************/
gdouble** Inverse3(gdouble **mat)
{
	gdouble **invmat = NULL;
	gdouble t4,t6,t8,t10,t12,t14,t17;

	t4 = mat[0][0]*mat[1][1];     
 	t6 = mat[0][0]*mat[1][2];
      	t8 = mat[0][1]*mat[1][0];
      	t10 = mat[0][2]*mat[1][0];
      	t12 = mat[0][1]*mat[2][0];
      	t14 = mat[0][2]*mat[2][0];
      	t17 = 1/(t4*mat[2][2]-t6*mat[2][1]-t8*mat[2][2]+t10*mat[2][1]+t12*mat[1][2]-t14*mat
[1][1]);
	invmat = g_malloc(3*sizeof(gdouble*));
	invmat[0] = g_malloc(3*sizeof(gdouble));
	invmat[1] = g_malloc(3*sizeof(gdouble));
	invmat[2] = g_malloc(3*sizeof(gdouble));
      	invmat[0][0] = (mat[1][1]*mat[2][2]-mat[1][2]*mat[2][1])*t17;
      	invmat[0][1] = -(mat[0][1]*mat[2][2]-mat[0][2]*mat[2][1])*t17;
      	invmat[0][2] = -(-mat[0][1]*mat[1][2]+mat[0][2]*mat[1][1])*t17;
      	invmat[1][0] = -(mat[1][0]*mat[2][2]-mat[1][2]*mat[2][0])*t17;
      	invmat[1][1] = (mat[0][0]*mat[2][2]-t14)*t17;
      	invmat[1][2] = -(t6-t10)*t17;
      	invmat[2][0] = -(-mat[1][0]*mat[2][1]+mat[1][1]*mat[2][0])*t17;
      	invmat[2][1] = -(mat[0][0]*mat[2][1]-t12)*t17;
      	invmat[2][2] = (t4-t8)*t17;

	return invmat;
}
/**************************************************/
gboolean InverseMat3D(gdouble invmat[3][3], gdouble mat[3][3])
{
	gdouble t4,t6,t8,t10,t12,t14,t17;

	t4 = mat[0][0]*mat[1][1];     
 	t6 = mat[0][0]*mat[1][2];
      	t8 = mat[0][1]*mat[1][0];
      	t10 = mat[0][2]*mat[1][0];
      	t12 = mat[0][1]*mat[2][0];
      	t14 = mat[0][2]*mat[2][0];
      	t17 = (t4*mat[2][2]-t6*mat[2][1]-t8*mat[2][2]+t10*mat[2][1]+t12*mat[1][2]-t14*mat[1][1]);
	if(fabs(t17)<1e-12) return FALSE;
	
      	t17 = 1/t17;
      	invmat[0][0] = (mat[1][1]*mat[2][2]-mat[1][2]*mat[2][1])*t17;
      	invmat[0][1] = -(mat[0][1]*mat[2][2]-mat[0][2]*mat[2][1])*t17;
      	invmat[0][2] = -(-mat[0][1]*mat[1][2]+mat[0][2]*mat[1][1])*t17;
      	invmat[1][0] = -(mat[1][0]*mat[2][2]-mat[1][2]*mat[2][0])*t17;
      	invmat[1][1] = (mat[0][0]*mat[2][2]-t14)*t17;
      	invmat[1][2] = -(t6-t10)*t17;
      	invmat[2][0] = -(-mat[1][0]*mat[2][1]+mat[1][1]*mat[2][0])*t17;
      	invmat[2][1] = -(mat[0][0]*mat[2][1]-t12)*t17;
      	invmat[2][2] = (t4-t8)*t17;
	return TRUE;
}
/**************************************************/
