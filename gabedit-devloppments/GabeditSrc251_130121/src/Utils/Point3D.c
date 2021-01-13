/* Point3D.c */
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
#include "../Utils/Point3D.h"

/************************************************************************************************************/
static gdouble length(Point3D* p);
static gboolean unit(Point3D* p);
static Point3D add(Point3D* p, Point3D* other);
static Point3D sub(Point3D* p, Point3D* other);
static Point3D mult(Point3D* p, gdouble factor);
static Point3D crossProd(Point3D* p, Point3D* other);
static gdouble dotProd(Point3D* p, Point3D* other);
static gdouble distance(Point3D* p, Point3D* other);
static gdouble angleBetween(Point3D* p, Point3D* other);
static Point3D rotate(Point3D* p, Point3D* rotCenter, Point3D* rotateAxis, gdouble angle);
static gboolean equals(Point3D* p, Point3D* other);
static Point3D closestPointInPlane(Point3D* point, Point3D* planePoint, Point3D* normal);
static Point3D closestPointOnAxis(Point3D* point, Point3D* axisPoint, Point3D* paxis);
static gchar* toString(Point3D* p);
static Point3D* copy(Point3D* p);
static Point3D* copyPoint(Point3D p);
/************************************************************************************************************/
Point3D newPoint3D()
{
	Point3D p;
	p.x = 0;
	p.y = 0;
	p.z = 0;
	sprintf(p.str," ");
	p.length = length;
	p.unit = unit;
	p.add = add;
	p.sub = sub;
	p.mult = mult;
	p.copy = copy;
	p.copyPoint = copyPoint;
	p.crossProd = crossProd;
	p.dotProd = dotProd;
	p.angleBetween = angleBetween;
	p.rotate = rotate;
	p.equals = equals;
	p.closestPointInPlane = closestPointInPlane;
	p.closestPointOnAxis = closestPointOnAxis;
	p.distance = distance;
	p.toString = toString;
	return p;
};
/************************************************************************************************************/
Point3D newPoint3Dxyz(gdouble x, gdouble y, gdouble z)
{
	Point3D p = newPoint3D();
	p.x = x;
	p.y = y;
	p.z = z;
	return p;
}
/************************************************************************************************************/
static gdouble length(Point3D* p)
{
	return sqrt(p->x*p->x + p->y*p->y + p->z*p->z);
}
/************************************************************************************************************/
static gboolean unit(Point3D* p)
{
	gdouble len = length(p);
	if(len != 0)
	{
		p->x /= len;
		p->y /= len;
		p->z /= len;
		return TRUE;
	}
	return FALSE;
}
/************************************************************************************************************/
static Point3D add(Point3D* p, Point3D* other)
{
	return newPoint3Dxyz(p->x + other->x, p->y + other->y, p->z + other->z);
}
/************************************************************************************************************/
static  Point3D sub(Point3D* p, Point3D* other)
{
	return newPoint3Dxyz(p->x - other->x, p->y - other->y, p->z - other->z);
}
/************************************************************************************************************/
static Point3D mult(Point3D* p, gdouble factor)
{
	return newPoint3Dxyz(p->x *factor, p->y * factor, p->z * factor);
}
/************************************************************************************************************/
static Point3D crossProd(Point3D* p, Point3D* other)
{
	Point3D result = newPoint3D();
	result.x = other->z*p->y - other->y*p->z;
	result.y = other->x*p->z - other->z*p->x;
	result.z = other->y*p->x - other->x*p->y;
	return result;
}
/************************************************************************************************************/
static Point3D* copy(Point3D* p)
{
	Point3D* newP = g_malloc(sizeof(Point3D));
	*newP = *p;
	return newP;
}
/************************************************************************************************************/
static Point3D* copyPoint(Point3D p)
{
	return copy(&p);
}
/************************************************************************************************************/
static gdouble dotProd(Point3D* p, Point3D* other)
{
		return other->x*p->x + other->y*p->y + other->z*p->z;
}
/************************************************************************************************************/
static gdouble distance(Point3D* p, Point3D* other)
{
	gdouble dx = other->x-p->x;
	gdouble dy = other->y-p->y;
	gdouble dz = other->z-p->z;
	return sqrt(dx*dx+dy*dy+dz*dz);
}
/************************************************************************************************************/
static gdouble angleBetween(Point3D* p, Point3D* other)
{
	return acos(dotProd(p,other)/(length(p)*length(other)));
}
/************************************************************************************************************/
static Point3D rotate(Point3D* p, Point3D* rotCenter, Point3D* rotateAxis, gdouble angle)
{
		Point3D oldCoords = *p;
		Point3D newCoords = newPoint3D();
		Point3D rotAxis = *rotateAxis;
		unit(&rotAxis);
		
		oldCoords = sub(&oldCoords, rotCenter);
		
		gdouble c = cos(angle);
		gdouble s = sin(angle);
		
		gdouble m[4][4];
		
		m[0][0] = c + (1 - c) * rotAxis.x * rotAxis.x;
		m[0][1] = (1 - c) * rotAxis.x * rotAxis.y - s * rotAxis.z;
		m[0][2] = (1 - c) * rotAxis.x * rotAxis.z + s * rotAxis.y;
		m[1][0] = (1 - c) * rotAxis.x * rotAxis.y + s * rotAxis.z;
		m[1][1] = c + (1 - c) * rotAxis.y * rotAxis.y;
		m[1][2] = (1 - c) * rotAxis.z * rotAxis.y - s * rotAxis.x;
		m[2][0] = (1 - c) * rotAxis.x * rotAxis.z - s * rotAxis.y;
		m[2][1] = (1 - c) * rotAxis.y * rotAxis.z + s * rotAxis.x;
		m[2][2] = c + (1 - c) * rotAxis.z * rotAxis.z;
			
		newCoords.x = m[0][0]*oldCoords.x + m[0][1]*oldCoords.y + m[0][2]*oldCoords.z;
		newCoords.y = m[1][0]*oldCoords.x + m[1][1]*oldCoords.y + m[1][2]*oldCoords.z;
		newCoords.z = m[2][0]*oldCoords.x + m[2][1]*oldCoords.y + m[2][2]*oldCoords.z;
		
		newCoords = add(&newCoords, rotCenter);
		
		return newCoords;
}
/************************************************************************************************************/
static gboolean equals(Point3D* p, Point3D* other)
{
	if( !p || !other) return FALSE;
	if(distance(p, other) < 0.00001) return TRUE;
	return FALSE;
}
/************************************************************************************************************/
static Point3D closestPointInPlane(Point3D* point, Point3D* planePoint, Point3D* normal)
{
	gdouble d = -(normal->x * planePoint->x + normal->y * planePoint->y + normal->z * planePoint->z);
	gdouble t = -((normal->x * point->x + normal->y * point->y + normal->z * point->z + d)/
					(normal->x*normal->x + normal->y*normal->y + normal->z*normal->z));
	return newPoint3Dxyz(point->x + normal->x * t, point->y + normal->y * t, point->z + normal->z*t);
}
/************************************************************************************************************/
static Point3D closestPointOnAxis(Point3D* point, Point3D* axisPoint, Point3D* paxis)
{
		Point3D axis = newPoint3Dxyz(paxis->x, paxis->y, paxis->z);
		unit(&axis);
		Point3D s =  sub(point,axisPoint);
		Point3D m =  mult(&axis, dotProd(&s,&axis)); 
		return add(axisPoint, &m);
}
/************************************************************************************************************/
static gchar* toString(Point3D* point)
{
	sprintf(point->str,"(%f, %f, %f)", point->x, point->y, point->z);
	return point->str;
}
