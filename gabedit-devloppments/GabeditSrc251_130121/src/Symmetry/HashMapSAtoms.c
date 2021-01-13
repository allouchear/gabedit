/* HashMapSAtoms.c */
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
#include "../Symmetry/HashMapSAtoms.h"

/************************************************************************************************************/
static void put(HashMapSAtoms* map, gint index, SAtom* atom);
static gint size(HashMapSAtoms* map);
static SAtom* get(HashMapSAtoms* map, gint i);
static void clear(HashMapSAtoms* map);
/************************************************************************************************************/
HashMapSAtoms newHashMapSAtoms()
{
	HashMapSAtoms map;
	map.listOfAtoms = NULL;
	map.listOfIndex = NULL;
	map.nAtoms = 0;
/* methods */
	map.put = put;
	map.get = get;
	map.clear = clear;
	map.size = size;
	return map;
}
/************************************************************************************************************/
static void put(HashMapSAtoms* map, gint index, SAtom* atom)
{
	SAtom* a = g_malloc(sizeof(SAtom));
	gint* i = g_malloc(sizeof(gint));
	*a = *atom;
	map->listOfAtoms = g_list_append(map->listOfAtoms, a);
	*i = index;
	map->listOfIndex = g_list_append(map->listOfIndex, i);
	map->nAtoms++;
}
/************************************************************************************************************/
static gint size(HashMapSAtoms* map)
{
	return map->nAtoms;
}
/************************************************************************************************************/
static SAtom* get(HashMapSAtoms* map, gint i)
{
	GList* la = NULL;
	GList* li = NULL;
	if(!map) return NULL;
	for( la = map->listOfAtoms, li=map->listOfIndex ; la != NULL && li != NULL;  la = la->next, li = li->next)
	{
		if(*(gint*)(li->data)==i) return (SAtom*) la->data;
	}
	return NULL;
}
/************************************************************************************************************/
static void clear(HashMapSAtoms* map)
{
	GList* l = NULL;
	for( l = map->listOfAtoms; l != NULL;  l = l->next) if( l->data) g_free(l->data);
	for( l = map->listOfIndex; l != NULL;  l = l->next) if( l->data) g_free(l->data);
	g_list_free(map->listOfAtoms);
	g_list_free(map->listOfIndex);
	map->listOfAtoms = NULL;
	map->listOfIndex = NULL;
	map->nAtoms = 0;
}
/************************************************************************************************************/
