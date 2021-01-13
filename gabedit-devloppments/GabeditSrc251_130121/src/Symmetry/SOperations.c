/* SOperations.c */
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
#include "../Symmetry/SOperations.h"

/************************************************************************************************************/
static void addOperation(SOperations* operations, gint number, gchar* type, gint degree);
static void addSOperation(SOperations* operations, SOperation* operation);
static GList* getSOperations(SOperations* operations);
static gint size(SOperations* operations);
static SOperation get(SOperations* operations, gint i);
static SOperations copy(SOperations* operations);
static void clear(SOperations* operations);
static void addSOperations(SOperations* operations, SOperations* operations2);
static void set(SOperations* operations, gint i, SOperation* operation);
static gchar* toString(SOperations* operations);
/************************************************************************************************************/
SOperations newSOperations()
{
	SOperations operations;
	operations.listOfSOperations = NULL;
	operations.nSOperations = 0;
	sprintf(operations.str," ");
/* methods */
	operations.addOperation = addOperation;
	operations.addSOperation = addSOperation;
	operations.getSOperations = getSOperations;
	operations.size = size;
	operations.get = get;
	operations.copy = copy;
	operations.clear = clear;
	operations.addSOperations = addSOperations;
	operations.set = set;
	operations.toString = toString;
	return operations;
}
/************************************************************************************************************/
SOperations newSOperationsSize(gint n)
{
	SOperations operations = newSOperations();
	gint i;
	for(i=0;i<n;i++)
	{
		SOperation a = newSOperation(1,"E",1);
		operations.addSOperation(&operations,&a);
	}
	return operations;
}
/************************************************************************************************************/
SOperations getSOperationsForOneGroup(gchar* groupName)
{
	SOperations operations = newSOperations();
	if(!strcmp(groupName,"Cs"))
	{
		operations.addOperation(&operations,1,"sigma",1);
	}
	else if(!strcmp(groupName,"Ci"))
	{
		operations.addOperation(&operations,1,"i",1);
	}
	else if(!strcmp(groupName,"C2"))
	{
		operations.addOperation(&operations,1,"C",2);
	}
	else if(!strcmp(groupName,"C3"))
	{
		operations.addOperation(&operations,2,"C",3);
	}
	else if(!strcmp(groupName,"C4"))
	{
		operations.addOperation(&operations,2,"C",4);
		operations.addOperation(&operations,1,"C",2);
	}
	else if(!strcmp(groupName,"C5"))
	{
		operations.addOperation(&operations,4,"C",5);
	}
	else if(!strcmp(groupName,"C6"))
	{
		operations.addOperation(&operations,2,"C",6);
		operations.addOperation(&operations,2,"C",3);
		operations.addOperation(&operations,1,"C",2);
	}
	else if(!strcmp(groupName,"C7"))
	{
		operations.addOperation(&operations,6,"C",7);
	}
	else if(!strcmp(groupName,"C8"))
	{
		operations.addOperation(&operations,4,"C",8);
		operations.addOperation(&operations,2,"C",4);
		operations.addOperation(&operations,1,"C",2);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,1,"sigma",1);
	}
	else if(!strcmp(groupName,"D2"))
	{
		operations.addOperation(&operations,3,"C",2);
	}
	else if(!strcmp(groupName,"D3"))
	{
		operations.addOperation(&operations,2,"C",3);
		operations.addOperation(&operations,3,"C",2);
	}
	else if(!strcmp(groupName,"D4"))
	{
		operations.addOperation(&operations,2,"C",4);
		operations.addOperation(&operations,5,"C",2);
	}
	else if(!strcmp(groupName,"D5"))
	{
		operations.addOperation(&operations,4,"C",5);
		operations.addOperation(&operations,5,"C",2);
	}
	else if(!strcmp(groupName,"D6"))
	{
		operations.addOperation(&operations,2,"C",6);
		operations.addOperation(&operations,2,"C",3);
		operations.addOperation(&operations,7,"C",2);
	}
	else if(!strcmp(groupName,"C2v"))
	{
		operations.addOperation(&operations,1,"C",2);
		operations.addOperation(&operations,2,"sigma",1);
	}
	else if(!strcmp(groupName,"C3v"))
	{
		operations.addOperation(&operations,2,"C",3);
		operations.addOperation(&operations,3,"sigma",1);
	}
	else if(!strcmp(groupName,"C4v"))
	{
		operations.addOperation(&operations,2,"C",4);
		operations.addOperation(&operations,1,"C",2);
		operations.addOperation(&operations,4,"sigma",1);
	}
	else if(!strcmp(groupName,"C5v"))
	{
		operations.addOperation(&operations,4,"C",5);
		operations.addOperation(&operations,5,"sigma",1);
	}
	else if(!strcmp(groupName,"C6v"))
	{
		operations.addOperation(&operations,2,"C",6);
		operations.addOperation(&operations,2,"C",3);
		operations.addOperation(&operations,1,"C",2);
		operations.addOperation(&operations,6,"sigma",1);
	}
	else if(!strcmp(groupName,"D2d"))
	{
		operations.addOperation(&operations,3,"C",2);
		operations.addOperation(&operations,2,"S",4);
		operations.addOperation(&operations,2,"sigma",1);
	}
	else if(!strcmp(groupName,"D3d"))
	{
		operations.addOperation(&operations,2,"C",3);
		operations.addOperation(&operations,3,"C",2);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,2,"S",6);
		operations.addOperation(&operations,3,"sigma",1);
	}
	else if(!strcmp(groupName,"D4d"))
	{
		operations.addOperation(&operations,2,"C",4);
		operations.addOperation(&operations,5,"C",2);
		operations.addOperation(&operations,4,"S",8);
		operations.addOperation(&operations,4,"sigma",1);
	}
	else if(!strcmp(groupName,"D5d"))
	{
		operations.addOperation(&operations,4,"C",5);
		operations.addOperation(&operations,5,"C",2);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,4,"S",10);
		operations.addOperation(&operations,5,"sigma",1);
	}
	else if(!strcmp(groupName,"D6d"))
	{
		operations.addOperation(&operations,2,"C",6);
		operations.addOperation(&operations,2,"C",3);
		operations.addOperation(&operations,7,"C",2);
		operations.addOperation(&operations,4,"S",12);
		operations.addOperation(&operations,2,"S",4);
		operations.addOperation(&operations,6,"sigma",1);
	}
	else if(!strcmp(groupName,"C2h"))
	{
		operations.addOperation(&operations,1,"C",2);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,1,"sigma",1);
	}
	else if(!strcmp(groupName,"C3h"))
	{
		operations.addOperation(&operations,2,"C",3);
		operations.addOperation(&operations,1,"sigma",1);
		operations.addOperation(&operations,2,"S",3);
	}
	else if(!strcmp(groupName,"C4h"))
	{
		operations.addOperation(&operations,2,"C",4);
		operations.addOperation(&operations,1,"C",2);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,2,"S",4);
		operations.addOperation(&operations,1,"sigma",1);
	}
	else if(!strcmp(groupName,"C5h"))
	{
		operations.addOperation(&operations,4,"C",5);
		operations.addOperation(&operations,4,"S",5);
		operations.addOperation(&operations,1,"sigma",1);
	}
	else if(!strcmp(groupName,"C6h"))
	{
		operations.addOperation(&operations,2,"C",6);
		operations.addOperation(&operations,2,"C",3);
		operations.addOperation(&operations,1,"C",2);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,2,"S",3);
		operations.addOperation(&operations,2,"S",6);
		operations.addOperation(&operations,1,"sigma",1);
	}
	else if(!strcmp(groupName,"D2h"))
	{
		operations.addOperation(&operations,3,"C",2);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,3,"sigma",1);
	}
	else if(!strcmp(groupName,"D3h"))
	{
		operations.addOperation(&operations,2,"C",3);
		operations.addOperation(&operations,3,"C",2);
		operations.addOperation(&operations,2,"S",3);
		operations.addOperation(&operations,4,"sigma",1);
	}
	else if(!strcmp(groupName,"D4h"))
	{
		operations.addOperation(&operations,2,"C",4);
		operations.addOperation(&operations,5,"C",2);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,1,"S",4);
		operations.addOperation(&operations,5,"sigma",1);
	}
	else if(!strcmp(groupName,"D5h"))
	{
		operations.addOperation(&operations,4,"C",5);
		operations.addOperation(&operations,5,"C",2);
		operations.addOperation(&operations,4,"S",5);
		operations.addOperation(&operations,6,"sigma",1);
	}
	else if(!strcmp(groupName,"D6h"))
	{
		operations.addOperation(&operations,2,"C",6);
		operations.addOperation(&operations,2,"C",3);
		operations.addOperation(&operations,7,"C",2);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,2,"S",3);
		operations.addOperation(&operations,2,"S",6);
		operations.addOperation(&operations,7,"sigma",1);
	}
	else if(!strcmp(groupName,"S4"))
	{
		operations.addOperation(&operations,1,"C",2);
		operations.addOperation(&operations,2,"S",4);
	}
	else if(!strcmp(groupName,"S6"))
	{
		operations.addOperation(&operations,2,"C",3);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,2,"S",6);
	}
	else if(!strcmp(groupName,"S8"))
	{
		operations.addOperation(&operations,2,"C",4);
		operations.addOperation(&operations,1,"C",2);
		operations.addOperation(&operations,4,"S",8);
	}
	else if(!strcmp(groupName,"T"))
	{
		operations.addOperation(&operations,8,"C",3);
		operations.addOperation(&operations,3,"C",2);
	}
	else if(!strcmp(groupName,"Th"))
	{
		operations.addOperation(&operations,8,"C",3);
		operations.addOperation(&operations,3,"C",2);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,8,"S",6);
		operations.addOperation(&operations,3,"sigma",1);
	}
	else if(!strcmp(groupName,"Td"))
	{
		operations.addOperation(&operations,8,"C",3);
		operations.addOperation(&operations,3,"C",2);
		operations.addOperation(&operations,6,"S",4);
		operations.addOperation(&operations,6,"sigma",1);
	}
	else if(!strcmp(groupName,"O"))
	{
		operations.addOperation(&operations,6,"C",4);
		operations.addOperation(&operations,8,"C",3);
		operations.addOperation(&operations,9,"C",2);
	}
	else if(!strcmp(groupName,"Oh"))
	{
		operations.addOperation(&operations,6,"C",4);
		operations.addOperation(&operations,8,"C",3);
		operations.addOperation(&operations,9,"C",2);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,8,"S",6);
		operations.addOperation(&operations,6,"S",4);
		operations.addOperation(&operations,9,"sigma",1);
	}
	else if(!strcmp(groupName,"I"))
	{
		operations.addOperation(&operations,24,"C",5);
		operations.addOperation(&operations,20,"C",3);
		operations.addOperation(&operations,15,"C",2);
	}
	else if(!strcmp(groupName,"Ih"))
	{
		operations.addOperation(&operations,24,"C",5);
		operations.addOperation(&operations,20,"C",3);
		operations.addOperation(&operations,15,"C",2);
		operations.addOperation(&operations,1,"i",1);
		operations.addOperation(&operations,24,"S",10);
		operations.addOperation(&operations,20,"S",6);
		operations.addOperation(&operations,15,"sigma",1);
	}

	else if(!strcmp(groupName,"Cinfv"))
	{
		operations.addOperation(&operations,1,"C",-1);
	}
	else if(!strcmp(groupName,"Dinfh"))
	{
		operations.addOperation(&operations,1,"C",-1);
		operations.addOperation(&operations,1,"i",1);
	}
	return operations;
}
/************************************************************************************************************/
static void addOperation(SOperations* operations, gint number, gchar* type, gint degree)
{
	SOperation* a = g_malloc(sizeof(SOperation));
	*a = newSOperation(number, type, degree);
	operations->listOfSOperations = g_list_append(operations->listOfSOperations, a);
	operations->nSOperations++;
}
/************************************************************************************************************/
static void addSOperation(SOperations* operations, SOperation* operation)
{
	SOperation* a = g_malloc(sizeof(SOperation));
	*a = *operation;
	operations->listOfSOperations = g_list_append(operations->listOfSOperations, a);
	operations->nSOperations++;
}
/************************************************************************************************************/
static GList* getSOperations(SOperations* operations)
{
	return operations->listOfSOperations;
}
/************************************************************************************************************/
static gint size(SOperations* operations)
{
	return operations->nSOperations;
}
/************************************************************************************************************/
static SOperation get(SOperations* operations, gint i)
{
	GList* l = NULL;;
	gint j = 0;
	if(!operations) return newSOperation(1,"E",1);	
	for( l = operations->listOfSOperations; l != NULL;  l = l->next, j++)
		if(j==i) return *(SOperation*) l->data;
	return  newSOperation(1,"E",1);
}
/************************************************************************************************************/
static SOperations copy(SOperations* operations)
{
	SOperations noperations = newSOperations();	
	GList* l = NULL;;
	for( l = operations->listOfSOperations; l != NULL;  l = l->next)
		noperations.addSOperation(&noperations, (SOperation*) l->data);
	return noperations;
}
/************************************************************************************************************/
static void clear(SOperations* operations)
{
	GList* l = NULL;;
	for( l = operations->listOfSOperations; l != NULL;  l = l->next) if( l->data) g_free(l->data);
	g_list_free(operations->listOfSOperations);
	operations->listOfSOperations = NULL;
	operations->nSOperations = 0;
}
/************************************************************************************************************/
static void addSOperations(SOperations* operations, SOperations* operations2)
{
	GList* l = NULL;;
	for( l = operations2->listOfSOperations; l != NULL;  l = l->next)
		operations->addSOperation(operations, (SOperation*) l->data);
}
/************************************************************************************************************/
static void set(SOperations* operations, gint i, SOperation* operation)
{
	GList* l = NULL;;
	gint j = 0;
	for( l = operations->listOfSOperations; l != NULL;  l = l->next, j++)
	if(j==i) 
	{
		*(SOperation*) l->data = *operation;
		return;
	}
}
/************************************************************************************************************/
static gchar* toString(SOperations* operations)
{
	sprintf(operations->str," ");
	GList* l = NULL;;
	for( l = operations->listOfSOperations; l != NULL;  l = l->next)
	{
		SOperation* o =(SOperation*) l->data;
		strcat(operations->str, o->toString(o));
	}
	return operations->str;
}
