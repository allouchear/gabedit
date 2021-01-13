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

#ifndef __GABEDIT_ELEMENT_H__
#define __GABEDIT_ELEMENT_H__

#include "../Utils/Point3D.h"
#include "../Symmetry/SAtom.h"

typedef struct _Element  Element;

typedef enum
{
  INVERSION,
  ROTATION,
  IMPROPERROTATION,
  REFLECTION,
} ElementType;

struct _Element
{
	gdouble m[3][3];
	ElementType type;
	gdouble ERROR;
	gdouble DOT_TOLERANCE;
	gdouble distance;
	gchar name[10];
	gint degree;
	Point3D point;
	Point3D normal;
	Point3D axis;
	gint n;
	gint numUniqueOperations;
	gint GABEDIT_C_INFINITY;

/* methods */
	Point3D (*doOperation)(Element* element, Point3D* oldPoint);
	SAtom (*doOperationSAtom)(Element* element, SAtom* oldAtom);
	gdouble (*getDistance)(Element* element);
	void (*setDistance)(Element* element, gdouble distance);
	void (*setERROR)(Element* element, gdouble ERROR);
	void (*setDotTolerance)(Element* element, gdouble DOT_TOLERANCE);
	gint (*getNumUniqueOperations)(Element* element);
	gchar* (*getName)(Element* element);
	gint (*getDegree)(Element* element);
	gboolean (*equals)(Element* element1, Element* element2);
	gboolean (*eqPoint)(Element* e, Point3D* p1, Point3D* p2);
	gboolean (*eqAxes)(Element* e, Point3D* p1, Point3D* p2);
	Point3D* (*getPosition)(Element* element);
	Point3D* (*getPoint)(Element* element);
	Point3D* (*getNormal)(Element* element);
	Point3D* (*getAxis)(Element* element);
	gchar* (*toString)(Element* element);
};

Element newElement();
Element newElementInversion(Point3D* center);
Element newElementReflection(Point3D* center, Point3D* normal);
Element newElementRotation(Point3D* center, Point3D* axis, gint n);
Element newElementImproperRotation(Point3D* center, Point3D* axis, int n);
void copyElement(Element* element, Element** newElement);

#endif /* __GABEDIT_ELEMENT_H__ */

