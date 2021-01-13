/* SAtom.c */
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
#include "../Symmetry/SAtom.h"

/************************************************************************************************************/
static gchar* getSymbol(SAtom* atom);
static gint getNumber(SAtom* atom);
static gdouble getMass(SAtom* atom);
static Point3D getPosition(SAtom* atom);
static void setPosition(SAtom* atom, Point3D* newPos);
static void setSymmetryUnique(SAtom* atom, gboolean isUnique);
static gdouble distance(SAtom* atom, SAtom* other);
static SAtom findClosestAtom(SAtom* atom, GList* closeAtoms);
static gchar* toString(SAtom* atom);
static gboolean equals(SAtom* atom1, SAtom* atom2, gdouble tol);
/************************************************************************************************************/
SAtom newSAtom()
{
	SAtom e;
	e.position = newPoint3D();
	e.number = 1;
	sprintf(e.symbol,"H");
	e.mass = 1.007900;
	e.isSymmetryUnique = TRUE;
/* methods */
	e.getSymbol = getSymbol;
	e.getNumber = getNumber;
	e.getMass = getMass;
	e.getPosition = getPosition;
	e.setPosition = setPosition;
	e.setSymmetryUnique = setSymmetryUnique;
	e.findClosestAtom = findClosestAtom;
	e.distance = distance;
	e.toString = toString;
	e.equals = equals;
	return e;
}
/************************************************************************************************************/
SAtom newSAtomsnm(gchar* symbol, gint number, gdouble mass)
{
	SAtom e;
	e = newSAtom();
	e.number = number;
	if(symbol) sprintf(e.symbol,"%s",symbol);
	e.mass = mass;
	e.isSymmetryUnique = TRUE;
	return e;
}
/************************************************************************************************************/
SAtom newSAtomxyz(gchar* symbol, gint number, gdouble mass, gdouble x, gdouble y, gdouble z)
{
	SAtom e;
	e = newSAtomsnm(symbol, number, mass);
	e.position = newPoint3Dxyz(x,y,z);
	return e;
}
/************************************************************************************************************/
static gchar* getSymbol(SAtom* atom)
{
	return atom->symbol;
}
/************************************************************************************************************/
static gint getNumber(SAtom* atom)
{
	return atom->number;
}
/************************************************************************************************************/
static gdouble getMass(SAtom* atom)
{
	return atom->number;
}
/************************************************************************************************************/
static Point3D getPosition(SAtom* atom)
{
	return atom->position;
}
/************************************************************************************************************/
static void setPosition(SAtom* atom, Point3D* newPos)
{
	atom->position = *newPos;
}
/************************************************************************************************************/
static void setSymmetryUnique(SAtom* atom, gboolean isUnique)
{
	atom->isSymmetryUnique = isUnique;
}
/************************************************************************************************************/
static gdouble distance(SAtom* atom, SAtom* other)
{
	Point3D position = atom->getPosition(atom);
	Point3D otherPos = other->getPosition(other);
	return position.distance(&position,&otherPos);
}
/************************************************************************************************************/
static SAtom findClosestAtom(SAtom* atom, GList* closeAtoms)
{
	SAtom* closestAtom = NULL;
        Point3D thisPos, testPos;
        gdouble dx,dy,dz,distSq;
        gdouble shortestDistanceSq = 1E40;
	GList* list;
	SAtom* testAtom;

        thisPos = atom->getPosition(atom);
	for(list = closeAtoms; list != NULL; list = list->next)
	{
		testAtom = (SAtom*)(list->data);
		testPos = testAtom->getPosition(testAtom);
                dx = thisPos.x - testPos.x;
                dy = thisPos.y - testPos.y;
                dz = thisPos.z - testPos.z;
                distSq = dx*dx+dy*dy+dz*dz;

                if(atom->getNumber(atom) == testAtom->getNumber(testAtom) && atom!=testAtom &&
                (closestAtom == NULL || distSq < shortestDistanceSq))
		{
			closestAtom = testAtom;
			shortestDistanceSq = distSq;
		}
	}
        if(closestAtom != NULL)
	{
                        return *closestAtom;
	}
        return *atom;
}
/************************************************************************************************************/
static gchar* toString(SAtom* atom)
{
	gchar tmp[10];
	sprintf(tmp," ");
	if(atom->isSymmetryUnique) sprintf(tmp,"*");
	sprintf(atom->str,"%s%s at %s", tmp, atom->getSymbol(atom), atom->position.toString(&atom->position));
	return atom->str;
}
/************************************************************************************************************/
static gboolean eq(gdouble d1, gdouble d2, gdouble tol)
{
	return fabs(d1 - d2) < tol;
}
/************************************************************************************************************/
static gboolean eqPoint(Point3D* p1, Point3D* p2, gdouble tol)
{
	return eq(p1->x, p2->x, tol) && eq(p1->y, p2->y,tol) && eq(p1->z, p2->z,tol);
}
/************************************************************************************************************/
static gboolean equals(SAtom* atom1, SAtom* atom2, gdouble tol)
{
	if(!atom1 || !atom2) return FALSE;
	return !strcmp(getSymbol(atom1), getSymbol(atom2)) 
		&& eqPoint(&atom1->position, &atom2->position, tol) 
		&& atom1->number == atom2->number
		&& fabs(atom1->mass - atom2->mass)<1e-10;
}
