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

/* reference :  Largent, RJ, Polik, WF, Schmidt, JR, J. Comput Chem. 33 (2012) 1637-1642 */

#include "../../Config.h"
#include <math.h>
#include "../Common/Global.h"
#include "../Symmetry/Element.h"

/************************************************************************************************************/
static void setERROR(Element* e, gdouble ERROR);
static void setDotTolerance(Element* e, gdouble DOT_TOLERANCE);
static void setDistance(Element* e, gdouble distance);
static double getDistance(Element* e);
static gchar* getName(Element* e);
static gint getDegree(Element* e);
static gboolean equals(Element* e, Element* other);
static gboolean eq(Element* e, gdouble d1, gdouble d2);
static gboolean eqPoint(Element* e, Point3D* p1, Point3D* p2);
static gboolean eqAxes(Element* e, Point3D* p1, Point3D* p2);
static gint getNumUniqueOperations(Element* e);
static Point3D doInversion(Element* e, Point3D* beforee);
static Point3D* getPosition(Element* e);
static Point3D* getPoint(Element* e);
static Point3D* getNormal(Element* e);
static Point3D* getAxis(Element* e);
static Point3D doReflection(Element* e, Point3D* beforee);
static Point3D doUnit(Element* e, Point3D* beforee);
static Point3D doRotation(Element* e, Point3D* beforee);
static Point3D doImproperRotation(Element* e, Point3D* beforee);
static SAtom doOperationSAtom(Element* element, SAtom* oldAtom);
static gchar* toString(Element* element);
/************************************************************************************************************/
Element newElement()
{
	Element e;
	gint i,j;
	for(i=0;i<3;i++) for(j=0;j<3;j++) e.m[i][j] = 0.0;
	for(i=0;i<3;i++) e.m[i][i] = 1.0;
	e.type = INVERSION;
	e.ERROR = 0.2;
	e.DOT_TOLERANCE =  0.017365 * 2;
	e.GABEDIT_C_INFINITY = -1;
	e.distance = -1;
	sprintf(e.name,"E");
	e.point = newPoint3D();
	e.normal = newPoint3Dxyz(0,0,1);
	e.axis = newPoint3Dxyz(0,0,1);
	e.n = 1;
	e.numUniqueOperations = 1;
/* methods */
	e.getDistance = getDistance;
	e.setDistance = setDistance;
	e.setERROR = setERROR;
	e.setDotTolerance = setDotTolerance;
	e.getNumUniqueOperations = getNumUniqueOperations;
	e.getName = getName;
	e.getDegree = getDegree;
	e.equals = equals;
	e.eqPoint = eqPoint;
	e.eqAxes = eqAxes;
	e.getPosition = getPosition;
	e.getPoint = getPoint;
	e.getNormal = getNormal;
	e.getAxis = getAxis;
	e.doOperation = doUnit;
	e.doOperationSAtom = doOperationSAtom;
	e.toString = toString;
	return e;
}
/************************************************************************************************************/
Element newElementInversion(Point3D* center)
{
	Element e = newElement();
	e.point = *center;
	e.doOperation = doInversion;
	sprintf(e.name,"i");
	e.numUniqueOperations = 1;
	return e;
}
/************************************************************************************************************/
Element newElementReflection(Point3D* center, Point3D* normal)
{
	Element e = newElement();
	e.point = *center;
	e.doOperation = doReflection;
	e.type = REFLECTION;
	sprintf(e.name,"sigma");
	e.normal = *normal;
	e.normal.unit(&e.normal);
		
	gdouble a = e.normal.x;
	gdouble b = e.normal.y;
	gdouble c = e.normal.z;
		
	e.m[0][0] = -(a * a) + b * b + c * c;
	e.m[0][1] = -2 * a * b;
	e.m[0][2] = -2 * a *c;
	e.m[1][0] = -2 * a * b;
	e.m[1][1] = a * a - b * b + c * c;
	e.m[1][2] = -2 * b * c;
	e.m[2][0] = -2 * a * c;
	e.m[2][1] = -2 * b * c;
	e.m[2][2] = a * a + b * b - c * c;
	return e;
}
/************************************************************************************************************/
Element newElementRotation(Point3D* center, Point3D* axis, gint n)
{
	Element e = newElement();
	gint i;

	e.type = ROTATION;
	e.point = *center;
	e.axis  = *axis;
	e.axis.unit(&e.axis);
	e.n = n;
	if(n == e.GABEDIT_C_INFINITY) e.distance = 0;
	if(n == -1) sprintf(e.name,"Cinf");
	else sprintf(e.name,"C%d", n);

	e.numUniqueOperations = 1;
	for(i=2; i<n; i++)
		if(!(n % 2 == 0 && i % 2 == 0) && !(n % 3 == 0 && i % 3 ==0) && !(n % 5 == 0 && i % 5 == 0)) e.numUniqueOperations++;

	/*Build up the coefficients for the rotation matrix*/
	gdouble angle = 2 * M_PI / n;
	gdouble c = cos(angle);
	gdouble s = sin(angle);
		
		
	e.m[0][0] = c + (1 - c) * e.axis.x * e.axis.x;
	e.m[0][1] = (1 - c) * e.axis.x * e.axis.y - s * e.axis.z;
	e.m[0][2] = (1 - c) * e.axis.x * e.axis.z + s * e.axis.y;
	e.m[1][0] = (1 - c) * e.axis.x * e.axis.y + s * e.axis.z;
	e.m[1][1] = c + (1 - c) * e.axis.y * e.axis.y;
	e.m[1][2] = (1 - c) * e.axis.z * e.axis.y - s * e.axis.x;
	e.m[2][0] = (1 - c) * e.axis.x * e.axis.z - s * e.axis.y;
	e.m[2][1] = (1 - c) * e.axis.y * e.axis.z + s * e.axis.x;
	e.m[2][2] = c + (1 - c) * e.axis.z * e.axis.z;

	e.doOperation = doRotation;

	return e;
}
/************************************************************************************************************/
Element newElementImproperRotation(Point3D* center, Point3D* axis, int n)
{
	Element e = newElement();
	gint i,j;

	e.point = *center;
	e.axis  = *axis;
	e.axis.unit(&e.axis);
	e.n = n;
	e.type = IMPROPERROTATION;
	sprintf(e.name,"S%d", n);

	e.numUniqueOperations = 1;
	if(n % 2 == 1)
	{
		for(i=2; i<n*2; i++) if(!(i == n) && !(i % 2 == 0)) e.numUniqueOperations++;
	}
	else
	{
		for(i=2; i<n; i++) if(i % 2 == 1 && !(n / 2 == i)) e.numUniqueOperations++;
	}

	for(i=0; i<3; i++) for(j=0; j<3; j++) e.m[i][j] = 0.0;
	for(i=0; i<3; i++) e.m[i][i] = 1.0;

	e.doOperation = doImproperRotation;

	return e;
}
/************************************************************************************************************/
void copyElement(Element* e, Element** newElement)
{
	if(*newElement) g_free(*newElement);
	*newElement = g_malloc(sizeof(Element));
	**newElement = *e;
}
/************************************************************************************************************/
static Point3D doUnit(Element* e, Point3D* beforee)
{
	return *beforee;
}
/************************************************************************************************************/
static gint getNumUniqueOperations(Element* e)
{
	return e->numUniqueOperations;
}
/************************************************************************************************************/
static void setDistance(Element* e, gdouble distance)
{
	e->distance = distance;
}
/************************************************************************************************************/
static void setERROR(Element* e, gdouble ERROR)
{
	e->ERROR = ERROR;
}
/************************************************************************************************************/
static void setDotTolerance(Element* e, gdouble DOT_TOLERANCE)
{
	e->DOT_TOLERANCE = DOT_TOLERANCE;
}
/************************************************************************************************************/
static double getDistance(Element* e)
{
	return e->distance;
}
/************************************************************************************************************/
static gchar* getName(Element* e)
{
	return e->name;
}
/************************************************************************************************************/
static gint getDegree(Element* e)
{
	return e->n;
}
/************************************************************************************************************/
static Point3D* getPosition(Element* e)
{
	return &e->point;
}
/************************************************************************************************************/
static Point3D* getPoint(Element* e)
{
	return &e->point;
}
/************************************************************************************************************/
static Point3D* getNormal(Element* e)
{
	return &e->normal;
}
/************************************************************************************************************/
static Point3D* getAxis(Element* e)
{
	return &e->axis;
}
/************************************************************************************************************/
static gboolean equals(Element* e, Element* other)
{
	if(!e || !other) return FALSE;
	return !strcmp(getName(e), getName(other)) 
		&& eqPoint(e, &e->point, &other->point) 
		&& eqAxes(e, &e->normal, &other->normal)
		&& eqAxes(e, &e->axis, &other->axis) 
		&& e->n == other->n;
}
/************************************************************************************************************/
static gboolean eq(Element* e, gdouble d1, gdouble d2)
{
	return fabs(d1 - d2) < e->ERROR;
}
/************************************************************************************************************/
static gboolean eqPoint(Element* e, Point3D* p1, Point3D* p2)
{
	return eq(e,p1->x, p2->x) && eq(e,p1->y, p2->y) && eq(e,p1->z, p2->z);
}
/************************************************************************************************************/
static gboolean eqAxes(Element* e, Point3D* p1, Point3D* p2)
{
	return 1.0 - fabs(p1->dotProd(p1,p2)) < e->DOT_TOLERANCE;
}
/************************************************************************************************************/
static Point3D doInversion(Element* e, Point3D* beforee)
{
	Point3D newPos = newPoint3D();
	newPos.x = 2 * e->point.x - beforee->x;
	newPos.y = 2 * e->point.y - beforee->y;
	newPos.z = 2 * e->point.z - beforee->z;
	return newPos;
}
/************************************************************************************************************/
static Point3D doReflection(Element* e, Point3D* beforee)
{
	Point3D oldCoords = beforee->sub(beforee,&e->point);
	Point3D newCoords = newPoint3D();
				
	newCoords.x = e->m[0][0]*oldCoords.x + e->m[0][1]*oldCoords.y + e->m[0][2]*oldCoords.z;
	newCoords.y = e->m[1][0]*oldCoords.x + e->m[1][1]*oldCoords.y + e->m[1][2]*oldCoords.z;
	newCoords.z = e->m[2][0]*oldCoords.x + e->m[2][1]*oldCoords.y + e->m[2][2]*oldCoords.z;
		
	newCoords = newCoords.add(&newCoords,&e->point);
	return newCoords;
}
/************************************************************************************************************/
static Point3D doRotation(Element* e, Point3D* beforee)
{
	if(e->n == e->GABEDIT_C_INFINITY) return *beforee;
	else
	{
		Point3D oldCoords = beforee->sub(beforee,&e->point);
		Point3D newCoords = newPoint3D();
			
		newCoords.x = e->m[0][0]*oldCoords.x + e->m[0][1]*oldCoords.y + e->m[0][2]*oldCoords.z;
		newCoords.y = e->m[1][0]*oldCoords.x + e->m[1][1]*oldCoords.y + e->m[1][2]*oldCoords.z;
		newCoords.z = e->m[2][0]*oldCoords.x + e->m[2][1]*oldCoords.y + e->m[2][2]*oldCoords.z;
			
		newCoords = newCoords.add(&newCoords,&e->point);
		return newCoords;
	}
}
/************************************************************************************************************/
static Point3D doImproperRotation(Element* e, Point3D* beforee)
{
	Point3D oldCoords = beforee->sub(beforee,&e->point);
	Point3D newCoords = newPoint3D();
		
	if(e->n==0) return newCoords;
			
	gdouble a = e->axis.x;
	gdouble b = e->axis.y;
	gdouble c = e->axis.z;
		
	gdouble angle = 2 * M_PI / e->n;
	gdouble cs = cos(angle);
	gdouble sn = sin(angle);
		
	e->m[0][0] = -(a * a) + b * b + c * c;
	e->m[0][1] = -2 * a * b;
	e->m[0][2] = -2 * a *c;
	e->m[1][0] = -2 * a * b;
	e->m[1][1] = a * a - b * b + c * c;
	e->m[1][2] = -2 * b * c;
	e->m[2][0] = -2 * a * c;
	e->m[2][1] = -2 * b * c;
	e->m[2][2] = a * a + b * b - c * c;
		
	newCoords.x = e->m[0][0]*oldCoords.x + e->m[0][1]*oldCoords.y + e->m[0][2]*oldCoords.z;
	newCoords.y = e->m[1][0]*oldCoords.x + e->m[1][1]*oldCoords.y + e->m[1][2]*oldCoords.z;
	newCoords.z = e->m[2][0]*oldCoords.x + e->m[2][1]*oldCoords.y + e->m[2][2]*oldCoords.z;

	Point3D axis = e->axis;
		
	e->m[0][0] = cs + (1 - cs) * axis.x * axis.x;
	e->m[0][1] = (1 - cs) * axis.x * axis.y - sn * axis.z;
	e->m[0][2] = (1 - cs) * axis.x * axis.z + sn * axis.y;
	e->m[1][0] = (1 - cs) * axis.x * axis.y + sn * axis.z;
	e->m[1][1] = cs + (1 - cs) * axis.y * axis.y;
	e->m[1][2] = (1 - cs) * axis.z * axis.y - sn * axis.x;
	e->m[2][0] = (1 - cs) * axis.x * axis.z - sn * axis.y;
	e->m[2][1] = (1 - cs) * axis.y * axis.z + sn * axis.x;
	e->m[2][2] = cs + (1 - cs) * axis.z * axis.z;
		
	Point3D finalCoords = newPoint3D();
		
	finalCoords.x = e->m[0][0]*newCoords.x + e->m[0][1]*newCoords.y + e->m[0][2]*newCoords.z;
	finalCoords.y = e->m[1][0]*newCoords.x + e->m[1][1]*newCoords.y + e->m[1][2]*newCoords.z;
	finalCoords.z = e->m[2][0]*newCoords.x + e->m[2][1]*newCoords.y + e->m[2][2]*newCoords.z;
		
	finalCoords = finalCoords.add(&finalCoords,&e->point);
		
	return finalCoords;
}
/************************************************************************************************************/
static SAtom doOperationSAtom(Element* e, SAtom* beforee)
{
	SAtom a = *beforee;
	Point3D oldPosition =  a.getPosition(&a);
	Point3D newPosition =  e->doOperation(e, &oldPosition);
	a.setPosition(&a, &newPosition);
	return a;
}
/************************************************************************************************************/
static gchar* toString(Element* e)
{
	static gchar tmp[1024];
	static gchar n[1024];
	sprintf(tmp, " ");
	if(e->type==ROTATION) strcat(tmp,"Rotation ");
	if(e->type==IMPROPERROTATION) strcat(tmp,"Improper Rotation ");
	if(e->type==REFLECTION) strcat(tmp,"Reflection ");
	if(e->type==INVERSION) strcat(tmp,"Inverion ");
	strcat(tmp, "Point ");
	strcat(tmp, e->point.toString(&e->point));
	strcat(tmp, "Normal ");
	strcat(tmp, e->normal.toString(&e->normal));
	strcat(tmp, "Axis ");
	strcat(tmp, e->axis.toString(&e->axis));
	sprintf(n,"%s n = %d",tmp,e->n);
	sprintf(tmp,"%s",n);
	return tmp;
}
