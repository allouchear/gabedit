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

#ifndef __GABEDIT_POINTGROUPS_H__
#define __GABEDIT_POINTGROUPS_H__

#include "../Utils/Point3D.h"
#include "../Symmetry/PointGroupGabedit.h"

typedef struct _PointGroups  PointGroups;

struct _PointGroups
{
	GList* listOfPointGroups;
	gint nPointGroups;
/* methods */
	void (*addPointGroup) (PointGroups* pointGroups, PointGroup* pointGroup);
	void (*addPointGroupPos) (PointGroups* pointGroups, gint pos, PointGroup* pointGroup);
	GList* (*getPointGroups) (PointGroups* pointGroups);
	gint (*size) (PointGroups* pointGroups);
	PointGroup (*get) (PointGroups* pointGroups, gint i);
	PointGroup* (*getPointer) (PointGroups* pointGroups, gint i);
	PointGroups (*getCopy) (PointGroups* pointGroups);
	void (*clear) (PointGroups* pointGroups);
	void (*free) (PointGroups* pointGroups);
	void (*addPointGroups) (PointGroups* pointGroups, PointGroups* pointGroups2);
	void (*set) (PointGroups* pointGroups, gint i, PointGroup* pointGroup);
};

PointGroups newPointGroups();
PointGroups newPointGroupsSize(gint n);

#endif /* __GABEDIT_POINTGROUPS_H__ */

