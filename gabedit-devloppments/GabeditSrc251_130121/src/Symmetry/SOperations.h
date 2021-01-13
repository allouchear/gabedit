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

#ifndef __GABEDIT_OPERATIONS_H__
#define __GABEDIT_OPERATIONS_H__

#include "../Utils/Point3D.h"
#include "../Symmetry/SOperation.h"

typedef struct _SOperations  SOperations;

struct _SOperations
{
	GList* listOfSOperations;
	gint nSOperations;
	gchar str[1024];
/* methods */
	void (*addSOperation) (SOperations* operations, SOperation* operation);
	void (*addOperation) (SOperations* operations, gint number, gchar* type, gint degree);
	GList* (*getSOperations) (SOperations* operations);
	gint (*size) (SOperations* operations);
	SOperation (*get) (SOperations* operations, gint i);
	SOperations (*copy) (SOperations* operations);
	void (*clear) (SOperations* operations);
	void (*addSOperations) (SOperations* operations, SOperations* operations2);
	void (*set) (SOperations* operations, gint i, SOperation* operation);
	gchar* (*toString) (SOperations* operations);
};

SOperations newSOperations();
SOperations newSOperationsSize(gint n);
SOperations getSOperationsForOneGroup(gchar* groupName);

#endif /* __GABEDIT_OPERATIONS_H__ */

