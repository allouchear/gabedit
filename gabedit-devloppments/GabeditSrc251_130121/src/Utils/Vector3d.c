/* Vector3d.c */
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
/****************************************************/
void v3d_zero(V3d v)
{
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 0.0;
}
/****************************************************/
void v3d_set(V3d v, gdouble x, gdouble y, gdouble z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}
/****************************************************/
void v3d_sub(V3d src1, V3d src2, V3d dst)
{
    dst[0] = src1[0] - src2[0];
    dst[1] = src1[1] - src2[1];
    dst[2] = src1[2] - src2[2];
}
/****************************************************/
void v3d_copy(V3d v1, V3d v2)
{
    register int i;
    for (i = 0 ; i < 3 ; i++)
        v2[i] = v1[i];
}
/****************************************************/
void v3d_cross(V3d v1, V3d v2, V3d cross)
{
    V3d temp;

    temp[0] = (v1[1] * v2[2]) - (v1[2] * v2[1]);
    temp[1] = (v1[2] * v2[0]) - (v1[0] * v2[2]);
    temp[2] = (v1[0] * v2[1]) - (v1[1] * v2[0]);
    v3d_copy(temp, cross);
}

gdouble v3d_length(V3d v)
{
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}
/****************************************************/
void v3d_scale(V3d v, gdouble div)
{
    v[0] *= div;
    v[1] *= div;
    v[2] *= div;
}
/****************************************************/
void v3d_normal(V3d v)
{
    v3d_scale(v,1.0/v3d_length(v));
}
/****************************************************/
gdouble v3d_dot(V3d v1, V3d v2)
{
    return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}
/****************************************************/
void v3d_add(V3d src1, V3d src2, V3d dst)
{
    dst[0] = src1[0] + src2[0];
    dst[1] = src1[1] + src2[1];
    dst[2] = src1[2] + src2[2];
}
/****************************************************/
