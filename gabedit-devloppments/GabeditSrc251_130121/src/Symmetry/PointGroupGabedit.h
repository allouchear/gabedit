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

#ifndef __GABEDIT_POINTGROUP_H__
#define __GABEDIT_POINTGROUP_H__

#include "../Utils/Point3D.h"
#include "../Symmetry/SOperations.h"
#include "../Symmetry/SMolecule.h"
#include "../Symmetry/Elements.h"
#include "../Symmetry/HashMapSAtoms.h"

typedef struct _PointGroup  PointGroup;

struct _PointGroup
{
	gdouble symmetrizeTolerance;
	SMolecule molecule;
	SMolecule uniqueMolecule;
	Elements elements;
	gchar name[10];
	gdouble distance;
	gboolean isCubicGroup;
	gint numExtraElements;
	gint numMissingElements;

/* methods */
	gchar* (*getName) (PointGroup* pointGroup);
	gchar* (*toString) (PointGroup* pointGroup);
	gboolean (*isCubic) (PointGroup* pointGroup);
	void (*findUniqueAtoms)(PointGroup* pointGroup, gdouble tolerance);
	SMolecule (*getUniqueAtoms)(PointGroup* pointGroup, gdouble tolerance);
	SMolecule (*getMolecule)(PointGroup* pointGroup);
	Elements (*getElements)(PointGroup* pointGroup);
	void (*setDistance)(PointGroup* pointGroup, gdouble distance);
	void (*setNumExtraElements)(PointGroup* pointGroup, gint extra);
	void (*setNumMissingElements)( PointGroup* pointGroup, gint missing);
	gdouble (*getDistance)(PointGroup* pointGroup);
	gint (*getNumExtraElements)(PointGroup* pointGroup);
	gint (*getNumMissingElements)(PointGroup* pointGroup);
	void (*adjustUniqueAtomsToElements)(PointGroup* pointGroup, gdouble tolerance);
	SMolecule (*createSymmetrizedMolecule)(PointGroup* pointGroup, gdouble tolerance);
	SMolecule (*symmetrizeMolecule)(PointGroup* pointGroup, gdouble tolerance);
	gchar** (*getListOfGroups)(PointGroup* pointGroup);
	void (*clear)(PointGroup* pointGroup);
	void (*free)(PointGroup* pointGroup);
};

PointGroup newPointGroupDefault();
PointGroup newPointGroup(Elements* elements, SMolecule* mol, gchar* groupName);

#endif /* __GABEDIT_POINTGROUP_H__ */

