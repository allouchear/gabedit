/* Interpolation.c */
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
#include <math.h>
#include <stdlib.h>

/********************************************************************************/
void computBicubCoef(
	gdouble z[], 
	gdouble dzdx[], 
	gdouble dzdy[], 
	gdouble d2zdxdy[], 
	gdouble dx, gdouble dy,
	gdouble **c)
{
	static gint Ainv[16][16]={
		{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0},
		{-3,0,0,3,0,0,0,0,-2,0,0,-1,0,0,0,0},
		{2,0,0,-2,0,0,0,0,1,0,0,1,0,0,0,0},
		{0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0},
		{0,0,0,0,-3,0,0,3,0,0,0,0,-2,0,0,-1},
		{0,0,0,0,2,0,0,-2,0,0,0,0,1,0,0,1},
		{-3,3,0,0,-2,-1,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,-3,3,0,0,-2,-1,0,0},
		{9,-9,9,-9,6,3,-3,-6,6,-6,-3,3,4,2,1,2},
		{-6,6,-6,6,-4,-2,2,4,-3,3,3,-3,-2,-1,-1,-2},
		{2,-2,0,0,1,1,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,2,-2,0,0,1,1,0,0},
		{-6,6,-6,6,-3,-3,3,3,-4,4,2,-2,-2,-2,-1,-1},
		{4,-4,4,-4,2,2,-2,-2,2,-2,-2,2,1,1,1,1}
	};
	gint l,k,j,i;
	gdouble s,dxdy,c1D[16],x[16];

	dxdy=dx*dy;
	for (i=0;i<4;i++)
	{
		x[i]=z[i];
		x[i+4]=dzdx[i]*dx;
		x[i+8]=dzdy[i]*dy;
		x[i+12]=d2zdxdy[i]*dxdy;
	}
	for (i=0;i<16;i++)
	{
		s=0.0;
		for (k=0;k<16;k++) s += Ainv[i][k]*x[k];
		c1D[i]=s;
	}
	l=0;
	for (i=0;i<4;i++)
		for (j=0;j<4;j++) c[i][j]=c1D[l++];
}
/********************************************************************************/
void biCubicInterpolation(
	gdouble z[], 
	gdouble dzdx[], 
	gdouble dzdy[], 
	gdouble d2zdxdy[], 
	gdouble xl, gdouble xu, 
	gdouble yl, gdouble yu, 
	gdouble x, gdouble y, 
	gdouble *pz, gdouble *pdzdx, gdouble *pdzdy,
	gdouble *pd2zdx2, gdouble *pd2zdy2, gdouble *pd2zdxdy
	)
{
	int i;
	gdouble t,u,dx,dy;
	static gdouble **c = NULL;

	if(!c) 
	{
		c = g_malloc(4*sizeof(gdouble*));
		for(i=0;i<4;i++) c[i] = g_malloc(4*sizeof(gdouble));
	}
	if(!pz) return;

	dx=xu-xl;
	dy=yu-yl;
	computBicubCoef(z,dzdx,dzdy,d2zdxdy,dx,dy,c);
	if (xu == xl || yu == yl) 
	{
		printf("xUpper == xLower or yUpper = yLower in biCubicInterpolation");
		return;
	}
	t=(x-xl)/dx;
	u=(y-yl)/dy;
	*pz=0.0;
	if(pdzdx) *pdzdx=0.0;
	if(pdzdy) *pdzdy=0.0;
	for (i=3;i>=0;i--) {
		*pz=t*(*pz)+((c[i][3]*u+c[i][2])*u+c[i][1])*u+c[i][0];
		if(pdzdy) *pdzdy=t*(*pdzdy)+(3.0*c[i][3]*u+2.0*c[i][2])*u+c[i][1];
		if(pdzdx) *pdzdx=u*(*pdzdx)+(3.0*c[3][i]*t+2.0*c[2][i])*t+c[1][i];
	}
	if(pdzdx) *pdzdx /= dx;
	if(pdzdx) *pdzdy /= dy;
	if(pd2zdx2)
	{
		*pd2zdx2 =0;
		gint i;
		gint j;
		for(i=2;i<=3;i++)
		for(j=0;j<=3;j++)
			*pd2zdx2 += c[i][j]*i*(i-1)*pow(t,i-2)*pow(u,j);
		*pd2zdx2 /= dx*dx;
	}
	if(pd2zdy2)
	{
		*pd2zdy2 =0;
		gint i;
		gint j;
		for(i=0;i<=3;i++)
		for(j=2;j<=3;j++)
			*pd2zdy2 += c[i][j]*j*(j-1)*pow(t,i)*pow(u,j-2);
		*pd2zdy2 /= dy*dy;
	}
	if(pd2zdxdy)
	{
		*pd2zdxdy =0;
		gint i;
		gint j;
		for(i=1;i<=3;i++)
		for(j=1;j<=3;j++)
			*pd2zdxdy += c[i][j]*i*j*pow(t,i-1)*pow(u,j-1);
		*pd2zdxdy /= dx*dy;
	}
}
