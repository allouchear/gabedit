/* SOperation.c */
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
#include "../Symmetry/SOperation.h"

/* methods */
/************************************************************************************************************/
static gint getNumber(SOperation* operation);
static gchar* getElementType(SOperation* operation);
static gint getDegree(SOperation* operation);
static gchar* toString(SOperation* operation);
/************************************************************************************************************/
SOperation newSOperation(gint number, gchar* type, gint degree)
{
	SOperation o;
	o.number = number;
	o.degree = degree;
	sprintf(o.str," ");
	sprintf(o.elementType,"E");
	if(type) sprintf(o.elementType,"%s",type);
	if(degree!=1) 
	{
		gchar n[10];
		sprintf(n,"%d",degree);
		if(degree<0) sprintf(n,"inf");
		strcat(o.elementType,n);
	}
/* methods */
	o.getNumber = getNumber;
	o.getDegree = getDegree;
	o.getElementType = getElementType;
	o.toString = toString;
	return o;
}
/************************************************************************************************************/
static gchar* getElementType(SOperation* operation)
{
	return operation->elementType;
}
/************************************************************************************************************/
static gint getNumber(SOperation* operation)
{
	return operation->number;
}
/************************************************************************************************************/
static gint getDegree(SOperation* operation)
{
	return operation->degree;
}
/************************************************************************************************************/
static gchar* toString(SOperation* operation)
{
	gchar n[10];
	gchar m[10];

	sprintf(n," ");
	if(operation->number>1) sprintf(n," %d",operation->number);

	sprintf(m," ");
	if(operation->degree>1) sprintf(m,"%d ",operation->degree);
	if(operation->degree<0) sprintf(m,"inf ");

	sprintf(operation->str,"%s%s%s",n, operation->elementType, m);

	return operation->str;
}
