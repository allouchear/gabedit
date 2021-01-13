/* Elements.c */
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

/* reference :  Largent, RJ, Polik, WF, Schmidt, JR, J. Comput Chem. 33 (2012) 1637-1642 */

#include "../../Config.h"
#include <math.h>
#include "../Common/Global.h"
#include "../Symmetry/Elements.h"

/************************************************************************************************************/
static void addElement(Elements* elements, Element* element);
static GList* getElements(Elements* elements);
static gint size(Elements* elements);
static Element get(Elements* elements, gint i);
static Elements getCopy(Elements* elements);
static void clear(Elements* elements);
static void printElements(Elements* elements);
static void addElements(Elements* elements, Elements* elements2);
static void set(Elements* elements, gint i, Element* element);
static void removeElement(Elements* elements, Element* element);
static void addElementRotation(Elements* elements, Point3D* center, Point3D* axis, gint n);
static void addElementImproperRotation(Elements* elements, Point3D* center, Point3D* axis, gint n);
static void addElementReflection(Elements* elements, Point3D* center, Point3D* axis );
static void addElementInversion(Elements* elements, Point3D* center);
static gboolean contains(Elements* mol, Element* element, gdouble tol);
static gint indexOf(Elements* elements, Element* elem);
/************************************************************************************************************/
Elements newElements()
{
	Elements elements;
	elements.listOfElements = NULL;
	elements.nElements = 0;
/* methods */
	elements.addElement = addElement;
	elements.getElements = getElements;
	elements.size = size;
	elements.get = get;
	elements.getCopy = getCopy;
	elements.clear = clear;
	elements.free = clear;
	elements.print = printElements;
	elements.addElements = addElements;
	elements.set = set;
	elements.contains = contains;
	elements.remove = removeElement;
	elements.addElementRotation = addElementRotation;
	elements.addElementImproperRotation = addElementImproperRotation ;
	elements.addElementReflection = addElementReflection ;
	elements.addElementInversion = addElementInversion ;
	elements.indexOf = indexOf;
	return elements;
}
/************************************************************************************************************/
Elements newElementsSize(gint n)
{
	Elements elements = newElements();
	gint i;
	for(i=0;i<n;i++)
	{
		Element a = newElement();
		elements.addElement(&elements,&a);
	}
	return elements;
}
/************************************************************************************************************/
static void addElement(Elements* elements, Element* element)
{
	Element* a = g_malloc(sizeof(Element));
	*a = *element;
	elements->listOfElements = g_list_append(elements->listOfElements, a);
	elements->nElements++;
}
/************************************************************************************************************/
static void addElementRotation(Elements* elements, Point3D* center, Point3D* axis, gint n)
{
	Element e = newElementRotation(center, axis, n);
	elements->addElement(elements, &e);
}
/************************************************************************************************************/
static void addElementImproperRotation(Elements* elements, Point3D* center, Point3D* axis, gint n)
{
	Element e = newElementImproperRotation(center, axis, n);
	elements->addElement(elements, &e);
}
/************************************************************************************************************/
static void addElementReflection(Elements* elements, Point3D* center, Point3D* axis )
{
	Element e = newElementReflection(center, axis);
	elements->addElement(elements, &e);
}
/************************************************************************************************************/
static void addElementInversion(Elements* elements, Point3D* center)
{
	Element e = newElementInversion(center);
	elements->addElement(elements, &e);
}
/************************************************************************************************************/
static GList* getElements(Elements* elements)
{
	return elements->listOfElements;
}
/************************************************************************************************************/
static gint size(Elements* elements)
{
	return elements->nElements;
}
/************************************************************************************************************/
static Element get(Elements* elements, gint i)
{
	GList* l = NULL;;
	gint j = 0;
	if(!elements) return newElement();	
	for( l = elements->listOfElements; l != NULL;  l = l->next, j++)
		if(j==i) return *(Element*) l->data;
	return newElement();	
}
/************************************************************************************************************/
static Elements getCopy(Elements* elements)
{
	Elements nelements = newElements();	
	GList* l = NULL;;
	for( l = elements->listOfElements; l != NULL;  l = l->next)
		nelements.addElement(&nelements, (Element*) l->data);
	return nelements;
}
/************************************************************************************************************/
static void clear(Elements* elements)
{
	GList* l = NULL;;
	if(!elements->listOfElements) return;
	int i =0;
	for( l = elements->listOfElements; l != NULL;  l = l->next)
	{ 
		i++; 
		if( l->data) g_free(l->data);
	}
	g_list_free(elements->listOfElements);
	elements->listOfElements = NULL;
	elements->nElements = 0;
}
/************************************************************************************************************/
static void addElements(Elements* elements, Elements* elements2)
{
	GList* l = NULL;;
	for( l = elements2->listOfElements; l != NULL;  l = l->next)
		elements->addElement(elements, (Element*) l->data);
}
/************************************************************************************************************/
static void set(Elements* elements, gint i, Element* element)
{
	GList* l = NULL;;
	gint j = 0;
	for( l = elements->listOfElements; l != NULL;  l = l->next, j++)
	if(j==i) 
	{
		*(Element*) l->data = *element;
		return;
	}
}
/************************************************************************************************************/
static void removeElement(Elements* elements, Element* element)
{
	GList* l = NULL;;
	for( l = elements->listOfElements; l != NULL;  l = l->next)
	{
		if(l->data && element->equals(element,(Element*) l->data))
		{
			elements->listOfElements = g_list_remove(elements->listOfElements, (gpointer) l->data);
			elements->nElements--;
			return;
		}
	}
}
/************************************************************************************************************/
static gboolean contains(Elements* elements, Element* element, gdouble tol)
{
	GList* l = NULL;
	Element e = *element;
	e.setDotTolerance(&e, tol);
	for( l = elements->listOfElements; l != NULL;  l = l->next)
	{
		Element* a= (Element*) l->data;
		if(e.equals(&e,a)) return TRUE;
	}
	return FALSE;
}
/************************************************************************************************************/
static gint indexOf(Elements* elements, Element* elem)
{
	GList* l = NULL;
	gint j = 0;
	if(!elements) return -1;
	for( l = elements->listOfElements; l != NULL;  l = l->next, j++)
	{
		Element* a = l->data;
		if(a->equals(a,elem)) return j;
	}
	return -1;
}
/************************************************************************************************************/
static void printElements(Elements* elements)
{
        if(elements && elements->size(elements) > 0)
        {
                gint i;
                printf("\nELEMENTS FOUND:");
                for(i=0; i<elements->size(elements); i++)
                {
                        Element elem = elements->get(elements,i);
                        printf("%s ", elem.getName(&elem));
                        /* printf("\t%s with a rating of %f\n", elem.getName(&elem), elem.getDistance(&elem));*/
                        /*printf("\t%s with a rating of %f\n", elem.toString(&elem), elem.getDistance(&elem));*/
                }
                printf("\n\n");
        }
        else printf("\n--- NO ELEMENTS FOUND! --- \n");
}

