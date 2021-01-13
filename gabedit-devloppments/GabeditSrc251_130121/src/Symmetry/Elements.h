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

#ifndef __GABEDIT_ELEMENTS_H__
#define __GABEDIT_ELEMENTS_H__

#include "../Utils/Point3D.h"
#include "../Symmetry/Element.h"

typedef struct _Elements  Elements;

struct _Elements
{
	GList* listOfElements;
	gint nElements;
/* methods */
	void (*addElement) (Elements* elements, Element* element);
	void (*addElementRotation) (Elements* elements, Point3D* center, Point3D* axis, gint n);
	void (*addElementImproperRotation) (Elements* elements, Point3D* center, Point3D* axis, gint n);
	void (*addElementReflection) (Elements* elements, Point3D* center, Point3D* axis );
	void (*addElementInversion) (Elements* elements, Point3D* center);
	GList* (*getElements) (Elements* elements);
	gint (*size) (Elements* elements);
	Element (*get) (Elements* elements, gint i);
	Elements (*getCopy) (Elements* elements);
	void (*clear) (Elements* elements);
	void (*free) (Elements* elements);
	void (*addElements) (Elements* elements, Elements* elements2);
	void (*set) (Elements* elements, gint i, Element* element);
	void (*remove) (Elements* elements, Element* element);
	gboolean (*contains)(Elements* elements, Element* elemnt, gdouble tol);
	gint (*indexOf)(Elements* elements, Element* elemnt);
	void (*print) (Elements* elements);
};

Elements newElements();
Elements newElementsSize(gint n);

#endif /* __GABEDIT_ELEMENTS_H__ */

