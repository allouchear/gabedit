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

#ifndef __GABEDIT_POINT3D_H__
#define __GABEDIT_POINT3D_H__

typedef struct _Point3D  Point3D;

struct _Point3D
{
	gdouble x;
	gdouble y;
	gdouble z;
	gchar str[1024];
	gdouble (*length)(Point3D* point);
	gboolean (*unit)(Point3D* point);
	Point3D (*add)(Point3D* point, Point3D* pointToAdd);
	Point3D (*sub)(Point3D* point, Point3D* pointToSub);
	Point3D (*mult)(Point3D* point, gdouble factor);
	Point3D (*crossProd)(Point3D* point, Point3D* other);
	gdouble (*dotProd)(Point3D* point, Point3D* point2);
	gdouble (*angleBetween)(Point3D* point, Point3D* point2);
	Point3D (*rotate)(Point3D* point, Point3D* rotCenter, Point3D* rotateAxis, gdouble angle);
	gboolean (*equals)(Point3D* point, Point3D* point2);
	Point3D (*closestPointInPlane)(Point3D* point, Point3D* planePoint, Point3D* normal);
	Point3D (*closestPointOnAxis)(Point3D* point, Point3D* axisPoint, Point3D* axis);
	gdouble (*distance)(Point3D* point, Point3D* other);
	Point3D* (*copy)(Point3D* point);
	Point3D* (*copyPoint)(Point3D point);
	gchar* (*toString)(Point3D* point);
};

Point3D newPoint3D();
Point3D newPoint3Dv(gdouble v[3]);
Point3D newPoint3Dxyz(gdouble x, gdouble y,gdouble z);

#endif /* __GABEDIT_POINT3D_H__ */

