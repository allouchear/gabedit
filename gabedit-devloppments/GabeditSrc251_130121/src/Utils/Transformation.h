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

#ifndef __GABEDIT_TRANSFORMATION_H__
#define __GABEDIT_TRANSFORMATION_H__

typedef gdouble V4d[4];
gdouble *v4d_pvect(V4d v1,V4d v2);
gdouble v4d_pscal(V4d v1,V4d v2);
gdouble* v4d_scal(V4d v1,gdouble scal);
gdouble v4d_length(V4d v);
void v4d_normal(V4d v);
void trackball(gdouble q[4],gdouble p1x,gdouble p1y,gdouble p2x,gdouble p2y);
void add_quats(gdouble q1[4],gdouble q2[4],gdouble dest[4]);
void build_rotmatrix(gdouble m[4][4],gdouble q[4]);
gdouble** Inverse(gdouble **mat,gint size,gdouble error);
gdouble** Inverse3(gdouble **mat);
gboolean InverseMat3D(gdouble invmat[3][3], gdouble mat[3][3]);

#endif /* __GABEDIT_TRANSFORMATION_H__ */

