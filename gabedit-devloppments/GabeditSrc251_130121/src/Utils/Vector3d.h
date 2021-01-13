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

#ifndef __GABEDIT_VECTOR3D_H__
#define __GABEDIT_VECTOR3D_H__

typedef gdouble V3d[3];
void v3d_zero(V3d v);
void v3d_set(V3d v, gdouble x, gdouble y, gdouble z);
void v3d_sub(V3d src1, V3d src2, V3d dst);
void v3d_copy(V3d v1, V3d v2);
void v3d_cross(V3d v1, V3d v2, V3d cross);
gdouble v3d_length(V3d v);
void v3d_scale(V3d v, gdouble div);
void v3d_normal(V3d v);
gdouble v3d_dot(V3d v1, V3d v2);
void v3d_add(V3d src1, V3d src2, V3d dst);

#endif /* __GABEDIT_VECTOR3D_H__ */

