/*Matrix3D.c */
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
#include "../Utils/Constants.h"

/* Matrix 3x4 */

void ScalMat3D(gdouble** M, gdouble f)
{
	gint i;
	gint j;

	for(i=0;i<3;i++)
	for(j=0;j<4;j++)
		M[i][j] *= f;
		
}
void ScalMat3D_3(gdouble** M, gdouble fx,gdouble fy, gdouble fz)
{
	gint i;
	gint j;
	gdouble T[3] = { fx, fy, fz };

	for(i=0;i<3;i++)
	for(j=0;j<4;j++)
	{
		M[i][j] *= T[i];
		
	}
}
/** Translate the origin */
void TranslatMat3D(gdouble** M, gdouble x,gdouble y, gdouble z)
{
	gdouble T[3] = { x, y, z };
	gint i;

	for(i=0;i<3;i++)
		M[i][3] += T[i];
}
/** rotate theta degrees about the y axis */
void RotMat3D(gdouble** M,gdouble theta,gint iaxe)
{
	gdouble ct;
	gdouble st;
	gdouble N[3][4];
	gint i;
	gint j;
	gint axes[2] = { 0, 2};
	gdouble st2[2];

	theta *= (PI / 180);
	ct = cos(theta);
	st = sin(theta);

	switch(iaxe)
	{
		case 0 : axes[0] = 1;axes[1] =2; break;
		case 1 : axes[0] = 0;axes[1] =2; break;
		case 2 : axes[0] = 1;axes[1] =0; break;
	}
	/*
	axes[0] = (iaxe + 2)%3;
	axes[1] = (iaxe + 1)%3;
	*/

	st2[0] = st;
	st2[1] =-st;

	for(i=0;i<2;i++)
	for(j=0;j<4;j++)
		N[axes[i]][j] = (gdouble)(M[axes[i]][j]*ct + M[axes[1-i]][j]*st2[i]);

	for(i=0;i<2;i++)
	for(j=0;j<4;j++)
		M[axes[i]][j] = N[axes[i]][j];
}
/** Reinitialize to the unit matrix */
void UnitMat3D(gdouble** M)
{
	gint i;
	gint j;

	for(i=0;i<3;i++)
	for(j=0;j<4;j++)
		M[i][j] = 0.0;

	for(i=0;i<3;i++)
		M[i][i] = 1.0;
		
}
void TransformMat3D(gdouble** M,gdouble** v, int nvert)
{
	gint i;
	gint j;
	gdouble x;
	gdouble y;
	gdouble z;

	for (i = 0;i<nvert ;i++)
	{
	    x = v[i][0];
	    y = v[i][1];
	    z = v[i][2];
	    for(j=0;j<3;j++)
	    {
	    	v[i][j] = x * M[j][0] + y * M[j][1] + z * M[j][2] + M[j][3];
	    }

	}
}
void InverseTransformMat3D(gdouble** M,gdouble** v, int nvert)
{
	gint i;
	gdouble x;
	gdouble y;
	gdouble z;

	gdouble lxx = M[0][0], lxy = M[0][1], lxz = M[0][2], lxo = M[0][3];
	gdouble lyx = M[1][0], lyy = M[1][1], lyz = M[1][2], lyo = M[1][3];
	gdouble lzx = M[2][0], lzy = M[2][1], lzz = M[2][2], lzo = M[2][3];
	gdouble d = lxx*lyy*lzz-lxx*lyz*lzy-lyx*lxy*lzz+lyx*lxz*lzy+lzx*lxy*lyz-lzx*lxz*lyy;

	if ( d != 0 )
		return;

	for (i = nvert * 3; (i -= 3) >= 0;)
	{
	        x = v[i][0] - lxo;
	        y = v[i][1] - lyo;
	        z = v[i][2] - lzo;

	        v[i][0] = (x*(lyy*lzz-lyx*lzy)-y*(lxy*lzz-lxz*lzy)+z*(lxy*lyz-lxz*lyy))/d;
	        v[i][1] = (-x*(lyx*lzz-lyx*lzy)+y*(lxx*lzz-lxz*lzx)-z*(lxx*lyz-lxz*lyx))/d;
	        v[i][2] = (x*(lyx*lzy-lyy*lzx)-y*(lxx*lzy-lxy*lzx)+z*(lxx*lyy-lxy*lyx))/d;
	}
}
