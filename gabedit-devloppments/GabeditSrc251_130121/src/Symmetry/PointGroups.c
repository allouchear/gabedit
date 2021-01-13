/* PointGroups.c */
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
#include "../Symmetry/PointGroups.h"

/************************************************************************************************************/
static void addPointGroup(PointGroups* pointGroups, PointGroup* pointGroup);
static void addPointGroupPos(PointGroups* pointGroups, gint pos, PointGroup* pointGroup);
static GList* getPointGroups(PointGroups* pointGroups);
static gint size(PointGroups* pointGroups);
static PointGroup get(PointGroups* pointGroups, gint i);
static PointGroup* getPointer(PointGroups* pointGroups, gint i);
static PointGroups getCopy(PointGroups* pointGroups);
static void clear(PointGroups* pointGroups);
static void addPointGroups(PointGroups* pointGroups, PointGroups* pointGroups2);
static void set(PointGroups* pointGroups, gint i, PointGroup* pointGroup);
/************************************************************************************************************/
PointGroups newPointGroups()
{
	PointGroups pointGroups;
	pointGroups.listOfPointGroups = NULL;
	pointGroups.nPointGroups = 0;
/* methods */
	pointGroups.addPointGroup = addPointGroup;
	pointGroups.addPointGroupPos = addPointGroupPos;
	pointGroups.getPointGroups = getPointGroups;
	pointGroups.size = size;
	pointGroups.get = get;
	pointGroups.getPointer = getPointer;
	pointGroups.getCopy = getCopy;
	pointGroups.clear = clear;
	pointGroups.free = clear;
	pointGroups.addPointGroups = addPointGroups;
	pointGroups.set = set;
	return pointGroups;
}
/************************************************************************************************************/
PointGroups newPointGroupsSize(gint n)
{
	PointGroups pointGroups = newPointGroups();
	gint i;
	for(i=0;i<n;i++)
	{
		PointGroup a = newPointGroupDefault();
		pointGroups.addPointGroup(&pointGroups,&a);
	}
	return pointGroups;
}
/************************************************************************************************************/
static void addPointGroup(PointGroups* pointGroups, PointGroup* pointGroup)
{
	PointGroup* a = g_malloc(sizeof(PointGroup));
	*a = *pointGroup;
	pointGroups->listOfPointGroups = g_list_append(pointGroups->listOfPointGroups, a);
	pointGroups->nPointGroups++;
}
/************************************************************************************************************/
static void addPointGroupPos(PointGroups* pointGroups, gint pos, PointGroup* pointGroup)
{
	PointGroup* a = g_malloc(sizeof(PointGroup));
	*a = *pointGroup;
	pointGroups->listOfPointGroups = g_list_insert(pointGroups->listOfPointGroups, a, pos);
	pointGroups->nPointGroups++;
}
/************************************************************************************************************/
static GList* getPointGroups(PointGroups* pointGroups)
{
	return pointGroups->listOfPointGroups;
}
/************************************************************************************************************/
static gint size(PointGroups* pointGroups)
{
	return pointGroups->nPointGroups;
}
/************************************************************************************************************/
static PointGroup get(PointGroups* pointGroups, gint i)
{
	GList* l = NULL;;
	gint j = 0;
	if(!pointGroups) return newPointGroupDefault();	
	for( l = pointGroups->listOfPointGroups; l != NULL;  l = l->next, j++)
		if(j==i) return *(PointGroup*) l->data;
	return newPointGroupDefault();	
}
/************************************************************************************************************/
static PointGroup* getPointer(PointGroups* pointGroups, gint i)
{
	GList* l = NULL;;
	gint j = 0;
	if(!pointGroups) return NULL;
	for( l = pointGroups->listOfPointGroups; l != NULL;  l = l->next, j++)
		if(j==i) return (PointGroup*) l->data;
	return NULL;
}
/************************************************************************************************************/
static PointGroups getCopy(PointGroups* pointGroups)
{
	PointGroups npointGroups = newPointGroups();	
	GList* l = NULL;;
	for( l = pointGroups->listOfPointGroups; l != NULL;  l = l->next)
		npointGroups.addPointGroup(&npointGroups, (PointGroup*) l->data);
	return npointGroups;
}
/************************************************************************************************************/
static void clear(PointGroups* pointGroups)
{
	GList* l = NULL;;
	for( l = pointGroups->listOfPointGroups; l != NULL;  l = l->next) 
	if( l->data)
	{
		PointGroup* pg = (PointGroup*) l->data;
		pg->free(pg);
		g_free(l->data);
	}
	g_list_free(pointGroups->listOfPointGroups);
	pointGroups->listOfPointGroups = NULL;
	pointGroups->nPointGroups = 0;
}
/************************************************************************************************************/
static void addPointGroups(PointGroups* pointGroups, PointGroups* pointGroups2)
{
	GList* l = NULL;;
	for( l = pointGroups2->listOfPointGroups; l != NULL;  l = l->next)
		pointGroups->addPointGroup(pointGroups, (PointGroup*) l->data);
}
/************************************************************************************************************/
static void set(PointGroups* pointGroups, gint i, PointGroup* pointGroup)
{
	GList* l = NULL;;
	gint j = 0;
	for( l = pointGroups->listOfPointGroups; l != NULL;  l = l->next, j++)
	if(j==i) 
	{
		*(PointGroup*) l->data = *pointGroup;
		return;
	}
}
/************************************************************************************************************/
