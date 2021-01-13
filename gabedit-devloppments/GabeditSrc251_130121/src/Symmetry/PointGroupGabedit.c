/* PointGroup.c */
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
#include "../Symmetry/PointGroupGabedit.h"

/************************************************************************************************************/
static gchar* getName(PointGroup* pointGroup);
static gchar* toString(PointGroup* pointGroup);
static gboolean isCubic(PointGroup* pointGroup);
static gdouble uniquenessThreshold(PointGroup* pointGroup, gdouble tolerance);
static void findUniqueAtoms(PointGroup* pointGroup, gdouble tolerance);
static SMolecule getUniqueAtoms(PointGroup* pointGroup, gdouble tolerance);
static SMolecule getMolecule(PointGroup* pointGroup);
static Elements getElements(PointGroup* pointGroup);
static void setDistance(PointGroup* pointGroup, gdouble distance);
static void setNumExtraElements(PointGroup* pointGroup, gint extra);
static void setNumMissingElements( PointGroup* pointGroup, gint missing);
static gdouble getDistance(PointGroup* pointGroup);
static gint getNumExtraElements(PointGroup* pointGroup);
static gint getNumMissingElements(PointGroup* pointGroup);
static void adjustUniqueAtomsToElements(PointGroup* pointGroup, gdouble tolerance);
static SMolecule createSymmetrizedMolecule(PointGroup* pointGroup, gdouble tolerance);
static Elements symmetrizeIcosahedralElements(PointGroup* pointGroup, Element* primaryRotation, gboolean isIh);
static Elements symmetrizeOctahedralElements(PointGroup* pointGroup, Element* primaryRotation, gboolean isOh);
static Elements symmetrizeTetrahedralElements(PointGroup* pointGroup, Element* primaryRotation, gint numRefs);
static SMolecule symmetrizeMolecule(PointGroup* pointGroup, gdouble tolerance);
static gchar** getListOfGroups(PointGroup* pointGroup);
static void freePointGroup(PointGroup* pointGroup);
/************************************************************************************************************/
PointGroup newPointGroupDefault()
{
	PointGroup pointGroup;
	pointGroup.symmetrizeTolerance = 0.15;
	pointGroup.molecule = newSMolecule();
	pointGroup.uniqueMolecule = newSMolecule();
	sprintf(pointGroup.name,"C1");
	pointGroup.elements = newElements();
	pointGroup.distance = -1.0;
	pointGroup.isCubicGroup = FALSE;
	pointGroup.numExtraElements = 0;
	pointGroup.numMissingElements = 0;


/* methods */
	pointGroup.getName = getName;
	pointGroup.toString = toString;
	pointGroup.isCubic = isCubic;
	pointGroup.findUniqueAtoms = findUniqueAtoms;
	pointGroup.getUniqueAtoms  = getUniqueAtoms;
	pointGroup.getMolecule  = getMolecule;
	pointGroup.getElements  = getElements;
	pointGroup.setDistance = setDistance;
	pointGroup.setNumExtraElements = setNumExtraElements;
	pointGroup.setNumMissingElements = setNumMissingElements;
	pointGroup.getDistance = getDistance;
	pointGroup.getNumExtraElements = getNumExtraElements;
	pointGroup.getNumMissingElements = getNumMissingElements;
	pointGroup.adjustUniqueAtomsToElements = adjustUniqueAtomsToElements;
	pointGroup.createSymmetrizedMolecule = createSymmetrizedMolecule;
	pointGroup.symmetrizeMolecule = symmetrizeMolecule;
	pointGroup.getListOfGroups = getListOfGroups;
	pointGroup.clear = freePointGroup;
	pointGroup.free = freePointGroup;

	return pointGroup;
}
/************************************************************************************************************/
PointGroup newPointGroup(Elements* elements, SMolecule* mol, gchar* groupName)
{
	PointGroup pointGroup = newPointGroupDefault();
	if(mol) pointGroup.molecule = mol->getCopy(mol);
	if(elements) pointGroup.elements = elements->getCopy(elements);
	sprintf(pointGroup.name,"%s", groupName);
	pointGroup.isCubicGroup = isCubic(&pointGroup);

	return pointGroup;
}
/************************************************************************************************************/
static gchar* getName(PointGroup* pointGroup)
{
	return pointGroup->name;
}
/************************************************************************************************************/
static gchar* toString(PointGroup* pointGroup)
{
	return pointGroup->name;
}
/************************************************************************************************************/
static gboolean isCubic(PointGroup* pointGroup)
{
	if(!strcmp(pointGroup->name,"Th")) return TRUE;
	if(!strcmp(pointGroup->name,"T")) return TRUE;
	if(!strcmp(pointGroup->name,"Td")) return TRUE;
	if(!strcmp(pointGroup->name,"O")) return TRUE;
	if(!strcmp(pointGroup->name,"Oh")) return TRUE;
	if(!strcmp(pointGroup->name,"I")) return TRUE;
	if(!strcmp(pointGroup->name,"Ih")) return TRUE;
	return FALSE;
}
/************************************************************************************************************/
static gdouble uniquenessThreshold(PointGroup* pointGroup, gdouble tolerance)
{
	if(pointGroup->elements.size(&pointGroup->elements)>0 )
	{
		Element e = pointGroup->elements.get(&pointGroup->elements,0);
		if(e.ERROR > tolerance*2.0) return e.ERROR;
	}

	return tolerance*2.0;
}
/************************************************************************************************************/
static void findUniqueAtoms(PointGroup* pointGroup, gdouble tolerance)
{
	gint i,j;
	pointGroup->uniqueMolecule = newSMolecule();
	SMolecule generatedAtoms = newSMolecule();
	SMolecule* mol = &pointGroup->molecule;
	SMolecule* gmol = &generatedAtoms;
	Elements* els = &pointGroup->elements;
	GList* listOfElements = els->getElements(els);
	GList* l;
	gint n;
	GList* listOfAtoms = mol->getAtoms(mol);
	gint atomsFinishedGenerating = 0;

	mol->setSymmetryUniqueAll(mol, TRUE);
	mol->setSymmetryUnique(mol,0,TRUE);
        for(i=0; i<mol->size(mol); i++)
	{
		SAtom* a = mol->get(mol,i);
		if(a->isSymmetryUnique)
		{
                        gmol->addAtom(gmol, a);
			for(j= atomsFinishedGenerating; j<gmol->size(gmol); j++)
			{
				for(l = listOfElements; l != NULL; l = l->next)
				{
					Element* elem = (Element*) l->data;
					SAtom* b = gmol->get(gmol,j);
					SAtom startAtom = elem->doOperationSAtom(elem, b);
					for(n=0; n<elem->getDegree(elem); n++)
					{
						SAtom closestAtom = startAtom.findClosestAtom(&startAtom, listOfAtoms);
						
						if(startAtom.distance(&startAtom, &closestAtom) < uniquenessThreshold(pointGroup,tolerance) 
							&& !gmol->contains(gmol, &closestAtom, tolerance)
							&& (n ==0 || mol->indexOf(mol,&closestAtom, tolerance/1000) != mol->indexOf(mol,&startAtom, tolerance/1000))
						)
						{
								gint index = mol->indexOf(mol,&closestAtom, tolerance); 
                                                                closestAtom.setSymmetryUnique(&closestAtom,FALSE);
                                                                gmol->addAtom(gmol,&closestAtom);
								mol->set(mol,index,&closestAtom);
								
						}
						startAtom = closestAtom;
					}
				}
				atomsFinishedGenerating++;
			}
		}
	}
        for(i=0; i<gmol->size(gmol); i++)
	{
		SAtom* a = gmol->get(gmol,i);
		if(a->isSymmetryUnique)
		{
                                pointGroup->uniqueMolecule.addAtom(&pointGroup->uniqueMolecule, a);
		}
        }
}
/************************************************************************************************************/
static SMolecule getUniqueAtoms(PointGroup* pointGroup, gdouble tolerance)
{
	SMolecule* mol = &pointGroup->uniqueMolecule;
	if(mol->size(mol)<1) pointGroup->findUniqueAtoms(pointGroup, tolerance);
	return mol->getCopy(&pointGroup->uniqueMolecule);
}
/************************************************************************************************************/
static SMolecule getMolecule(PointGroup* pointGroup)
{
	SMolecule* mol = &pointGroup->molecule;
	return mol->getCopy(mol);
}
/************************************************************************************************************/
static Elements getElements(PointGroup* pointGroup)
{
	Elements* els = &pointGroup->elements;
	return els->getCopy(els);
}
/************************************************************************************************************/
static void setDistance(PointGroup* pointGroup, gdouble distance)
{
	pointGroup->distance = distance;
}
/************************************************************************************************************/
static void setNumExtraElements(PointGroup* pointGroup, gint extra)
{
	pointGroup->numExtraElements = extra;
}
/************************************************************************************************************/
static void setNumMissingElements( PointGroup* pointGroup, gint missing)
{
	pointGroup->numMissingElements = missing;
}
/************************************************************************************************************/
static gdouble getDistance(PointGroup* pointGroup)
{
	return pointGroup->distance;
}
/************************************************************************************************************/
static gint getNumExtraElements(PointGroup* pointGroup)
{
	return pointGroup->numExtraElements;
}
/************************************************************************************************************/
static gint getNumMissingElements(
PointGroup* pointGroup)
{
	return pointGroup->numMissingElements;
}
/************************************************************************************************************/
static void adjustUniqueAtomsToElements(PointGroup* pointGroup, gdouble tolerance)
{
	SMolecule* umol = &pointGroup->uniqueMolecule;
	Elements* els = &pointGroup->elements;
	GList* listOfElements = els->getElements(els);
	GList* l;
	gint i;

        for(i=0; i<umol->size(umol); i++)
	{
		SAtom* atom = umol->get(umol,i);
		for(l = listOfElements; l != NULL; l = l->next)
		{
			Element* elem = (Element*) l->data;
			if(elem->type==ROTATION)
			{
				Point3D atomPos = atom->getPosition(atom);
				elem->getPoint(elem); elem->getAxis(elem);
				Point3D closestPoint = newPoint3D();
				closestPoint = closestPoint.closestPointOnAxis(&atomPos, elem->getPoint(elem), elem->getAxis(elem));
				if(atomPos.distance(&atomPos, &closestPoint) < uniquenessThreshold(pointGroup, tolerance))
					atom->setPosition(atom, &closestPoint);
			}
			else if(elem->type==REFLECTION)
			{
				Point3D atomPos = atom->getPosition(atom);
				Point3D closestPoint = newPoint3D();
				closestPoint = closestPoint.closestPointInPlane(&atomPos, elem->getPoint(elem), elem->getNormal(elem));
				if(atomPos.distance(&atomPos, &closestPoint) < uniquenessThreshold(pointGroup, tolerance))
					atom->setPosition(atom, &closestPoint);
			}
			else if(elem->type == INVERSION)
			{
				Point3D atomPos = atom->getPosition(atom);
				if(atomPos.distance(&atomPos, elem->getPoint(elem)) < uniquenessThreshold(pointGroup, tolerance))
					atom->setPosition(atom, elem->getPoint(elem));
			}
		}
	}
}
/************************************************************************************************************/
static SMolecule createSymmetrizedMolecule(PointGroup* pointGroup, gdouble tolerance)
{
	gint i;
	SMolecule* mol = &pointGroup->molecule;
	SMolecule newMolecule = newSMolecule(mol->size(mol));
	SMolecule* umol = &pointGroup->uniqueMolecule;
	Elements* els = &pointGroup->elements;
	GList* listOfElements = els->getElements(els);
	GList* l;
	GList* listOfAtoms = mol->getAtoms(mol);
	HashMapSAtoms orderMap = newHashMapSAtoms();
	SMolecule finalMolecule = newSMolecule();


	newMolecule.addSMolecule(&newMolecule, &pointGroup->uniqueMolecule);
		
        for(i=0; i<umol->size(umol); i++)
	{
		SAtom* atom = umol->get(umol,i);
		orderMap.put(&orderMap, mol->indexOf(mol,atom, 2*tolerance), atom);
	}
	for(l = listOfElements; l != NULL; l = l->next)
	{
		Element* elem = (Element*) l->data;
		for(i=0; i<newMolecule.size(&newMolecule); i++)
		{
			SAtom atom = *(newMolecule.get(&newMolecule,i));
			gint degree = elem->getDegree(elem);
			gint n;

			if(degree > 1) degree--;
			for(n=0; n<degree; n++)
			{
				SAtom newAtom = elem->doOperationSAtom(elem,&atom);
				GList* listOfNewAtoms = newMolecule.getAtoms(&newMolecule);
				SAtom closestAtom = newAtom.findClosestAtom(&newAtom, listOfNewAtoms);
				if(newAtom.distance(&newAtom, &closestAtom) > 2*elem->ERROR)
				{
					SAtom closestOriginalAtom = newAtom.findClosestAtom(&newAtom, listOfAtoms);
					gint index;
					newAtom.setSymmetryUnique(&newAtom, FALSE);
					newMolecule.addAtom(&newMolecule, &newAtom);
					index = mol->indexOf(mol,&closestOriginalAtom, 2*tolerance);
					orderMap.put(&orderMap, index, &newAtom);
				}
				atom = newAtom;
				if(newMolecule.size(&newMolecule)>=mol->size(mol)) break;
			}
		}
		if(newMolecule.size(&newMolecule)>=mol->size(mol)) break;
	}
	finalMolecule = newSMolecule();
	for(i=0; i<newMolecule.size(&newMolecule); i++)
	{
		SAtom* a = (newMolecule.get(&newMolecule,i));
		if(a != NULL) finalMolecule.addAtom(&finalMolecule,a);
	}

	if(finalMolecule.size(&finalMolecule) != mol->size(mol))
	{
		printf("Error : the number of generated atoms does not match the initial molecule\n");
	}

	return finalMolecule;
}
/************************************************************************************************************/
static Elements symmetrizeIcosahedralElements(PointGroup* pointGroup, Element* primaryRotation, gboolean isIh)
{
	Elements symmetrizedElements = newElements();
		
	Point3D* center = primaryRotation->getPoint(primaryRotation);
	Point3D t = newPoint3Dxyz(0, 0, 1);
	Point3D* paxis =  primaryRotation->getAxis(primaryRotation);
	Point3D rotAxis = t.crossProd(&t,paxis);
		
	gdouble overallAngle = M_PI / 10;
	gdouble vertexAngle = (63.4349488 / 180) * M_PI;
	gdouble faceAngle = (M_PI - vertexAngle) / 2;
	gdouble c3Angle = (20.905157439 / 180) * M_PI;

	Elements* els = &pointGroup->elements;
	GList* listOfElements = els->getElements(els);
	GList* l;
	gint i;
	Point3D O = newPoint3D();

	for(l = listOfElements; l != NULL; l = l->next)
	{
		Element* elem = (Element*) l->data;
		Element* firstRot = elem;
		if(elem->type==ROTATION)
		{
			Point3D* axis = firstRot->getAxis(firstRot);
			gdouble dotprod = axis->dotProd(axis, paxis);
			if(fabs(cos(vertexAngle) - dotprod) < 0.1 && firstRot->getDegree(firstRot) == 5) rotAxis = axis->crossProd(axis, paxis);
		}
	}
		
	rotAxis.unit(&rotAxis);
		
	symmetrizedElements.addElementRotation(&symmetrizedElements, center, paxis, 5);

	if(isIh)
	{
		symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, paxis, 10);
	}
		
	for(i=0; i<primaryRotation->getDegree(primaryRotation)*2; i++)
	{
		if(i % 4 == 0)
		{
			Point3D newAxis = paxis->rotate(paxis, &O , &rotAxis, faceAngle + c3Angle);					
			symmetrizedElements.addElementRotation(&symmetrizedElements,center, &newAxis, 3);
			if(isIh) symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center,  &newAxis, 6);

			newAxis = paxis->rotate(paxis, &O , &rotAxis, faceAngle);					
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 2);
			if(isIh) symmetrizedElements.addElementReflection(&symmetrizedElements,center, &newAxis);

			newAxis = paxis->rotate(paxis, &O , &rotAxis, faceAngle + c3Angle);					
			symmetrizedElements.addElementRotation(&symmetrizedElements,center, &newAxis, 3);
			if(isIh) symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, &newAxis, 6);

			newAxis = paxis->rotate(paxis, &O , &rotAxis, 2*faceAngle);					
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 5);
			if(isIh) symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, &newAxis, 10);

			newAxis = paxis->rotate(paxis, &O , &rotAxis, 2*faceAngle + vertexAngle / 2);					
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 2);
			if(isIh) symmetrizedElements.addElementReflection(&symmetrizedElements,center, &newAxis);
		}
		else if(i % 4 == 2)
		{
			Point3D newAxis = paxis->rotate(paxis, &O , &rotAxis, vertexAngle / 2);					
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 2);
			if(isIh) symmetrizedElements.addElementReflection(&symmetrizedElements,center, &newAxis);

			newAxis = paxis->rotate(paxis, &O , &rotAxis, vertexAngle);
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 5);
			if(isIh) symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, &newAxis, 10);

			newAxis = paxis->rotate(paxis, &O , &rotAxis, vertexAngle + faceAngle - c3Angle);
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 3);
			if(isIh) symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, &newAxis, 6);

			newAxis = paxis->rotate(paxis, &O , &rotAxis, vertexAngle + faceAngle);					
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 2);
			if(isIh) symmetrizedElements.addElementReflection(&symmetrizedElements,center, &newAxis);

			newAxis = paxis->rotate(paxis, &O , &rotAxis, vertexAngle + faceAngle + c3Angle);
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 3);
			if(isIh) symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, &newAxis, 6);
		}
		else
		{
			Point3D newAxis = paxis->rotate(paxis, &O , &rotAxis, M_PI / 2);					
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 2);
			if(isIh) symmetrizedElements.addElementReflection(&symmetrizedElements,center, &newAxis);
		}
		rotAxis = rotAxis.rotate(&rotAxis, &O, paxis, overallAngle);
	}
	symmetrizedElements.addElementInversion(&symmetrizedElements, center);
		
	return symmetrizedElements;
}
/************************************************************************************************************/
static Elements symmetrizeOctahedralElements(PointGroup* pointGroup, Element* primaryRotation, gboolean isOh)
{
	Elements symmetrizedElements = newElements();
		
	Point3D* center = primaryRotation->getPoint(primaryRotation);
	Point3D t = newPoint3Dxyz(0, 0, 1);
	Point3D* paxis =  primaryRotation->getAxis(primaryRotation);
	Point3D rotAxis = t.crossProd(&t,paxis);
		
	gdouble overallAngle = M_PI / 4;

	Elements* els = &pointGroup->elements;
	GList* listOfElements = els->getElements(els);
	GList* l;
	gint i;
	Point3D O = newPoint3D();
	for(l = listOfElements; l != NULL; l = l->next)
	{
		Element* elem = (Element*) l->data;
		Element* firstRot = elem;
		if(elem->type==ROTATION)
		{
			Point3D* axis = firstRot->getAxis(firstRot);
			gdouble dotprod = axis->dotProd(axis, paxis);
			if(dotprod>0.1 && dotprod < 0.9 && firstRot->getDegree(firstRot) == 2) rotAxis = axis->crossProd(axis, paxis);
		}
	}
	rotAxis.unit(&rotAxis);
		
	symmetrizedElements.addElementRotation(&symmetrizedElements, center, paxis, 2);
	if(isOh)
	{
		symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, paxis, 4);
		symmetrizedElements.addElementReflection(&symmetrizedElements, center, paxis);
	}
	for(i=0; i<primaryRotation->getDegree(primaryRotation)*2; i++)
	{
		if(i % 2 == 0)
		{
			gdouble rotAngle = M_PI / 4;
			Point3D newAxis = paxis->rotate(paxis, &O , &rotAxis, M_PI / 2);					
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 2);
			if(isOh) symmetrizedElements.addElementReflection(&symmetrizedElements, center, &newAxis);

			newAxis = paxis->rotate(paxis, &O , &rotAxis, 2*rotAngle);					
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 2);
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 4);
			if(isOh) 
			{
				symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, &newAxis, 4);
				symmetrizedElements.addElementReflection(&symmetrizedElements, center, &newAxis);
			}
			newAxis = paxis->rotate(paxis, &O , &rotAxis, 3*rotAngle);
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 2);
			if(isOh) symmetrizedElements.addElementReflection(&symmetrizedElements, center, &newAxis);
		}
		else
		{
			gdouble rotAngleC3 = (109.47122064 / 180) * M_PI / 2;
			gdouble rotAngleC2 = M_PI / 2;
			gdouble rotAngle2C3 = ((90 + (90 - 109.47122064 / 2) )/ 180) * M_PI;

			Point3D newAxis = paxis->rotate(paxis, &O , &rotAxis, rotAngleC3);
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 3);
			if(isOh) symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, &newAxis, 6);

			newAxis = paxis->rotate(paxis, &O , &rotAxis, rotAngleC2);
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 2);
			if(isOh) symmetrizedElements.addElementReflection(&symmetrizedElements, center, &newAxis);

			newAxis = paxis->rotate(paxis, &O , &rotAxis, rotAngle2C3);
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 3);
			if(isOh) symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, &newAxis, 6);
		}
		rotAxis = rotAxis.rotate(&rotAxis, &O, paxis, overallAngle);
	}
	symmetrizedElements.addElementInversion(&symmetrizedElements, center);
	return symmetrizedElements;

}
/************************************************************************************************************/
static Elements symmetrizeTetrahedralElements(PointGroup* pointGroup, Element* pRot, gint numRefs)
{
	Elements symmetrizedElements = newElements();
	Element* primaryRotation = NULL;
	copyElement(pRot,&primaryRotation);
		
	Point3D* center = primaryRotation->getPoint(primaryRotation);
	Point3D t = newPoint3Dxyz(0, 0, 1);
	Point3D* paxis =  primaryRotation->getAxis(primaryRotation);
	Point3D rotAxis = t.crossProd(&t,paxis);

	GList* c3s = NULL;
	GList* c3si;
	GList* c3sj;
		
	gdouble overallAngle = M_PI / 4;
		
	Elements* els = &pointGroup->elements;
	GList* listOfElements = els->getElements(els);
	GList* l;
	gint i;
	Point3D O = newPoint3D();
	Point3D* p;

	for(l = listOfElements; l != NULL; l = l->next)
	{
		Element* elem = (Element*) l->data;
		Element* firstRot = elem;
		if(elem->type==ROTATION)
		{
			if(firstRot->getDegree(firstRot) == 2)
			{
				Point3D* axis = firstRot->getAxis(firstRot);
				gdouble dotprod = axis->dotProd(axis, paxis);
				if(primaryRotation->getDegree(primaryRotation) == 3)
				{
					copyElement(firstRot, &primaryRotation);
				}
				else if(fabs(dotprod)<0.1) 
				{
					paxis =  primaryRotation->getAxis(primaryRotation);
					rotAxis = axis->crossProd(axis, paxis);
					break;
				}
			}
		}
	}
	rotAxis.unit(&rotAxis);
	symmetrizedElements.addElementRotation(&symmetrizedElements, center, paxis, 2);
	if(numRefs == 6) symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, paxis, 4);

	for(i=0; i<primaryRotation->getDegree(primaryRotation)*2; i++)
	{
		if(i % 2 == 0)
		{
			gdouble rotAngle = M_PI / 2;
			Point3D newAxis = paxis->rotate(paxis, &O , &rotAxis, rotAngle);					
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 2);
			if(numRefs == 3) symmetrizedElements.addElementReflection(&symmetrizedElements, center, &newAxis);
			else if(numRefs == 6) symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, &newAxis, 4);
		}
		else
		{
			gdouble rotC3Angle2 = (125.2643896827 / 180) * M_PI;
			gdouble rotC3Angle1 = M_PI / 2 - (rotC3Angle2 - M_PI / 2);
			Point3D newAxis = paxis->rotate(paxis, &O , &rotAxis, rotC3Angle1);
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 3);
			if(numRefs == 3) symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, &newAxis, 6);

			p = g_malloc(sizeof(Point3D));
			*p = newAxis;
			c3s = g_list_append(c3s,p);

			newAxis = paxis->rotate(paxis, &O , &rotAxis, rotC3Angle2);
			symmetrizedElements.addElementRotation(&symmetrizedElements, center, &newAxis, 3);
			if(numRefs == 3) symmetrizedElements.addElementImproperRotation(&symmetrizedElements, center, &newAxis, 6);

			p = g_malloc(sizeof(Point3D));
			*p = newAxis;
			c3s = g_list_append(c3s,p);
		}
		rotAxis = rotAxis.rotate(&rotAxis, &O, paxis, overallAngle);
	}
	if(numRefs == 3) symmetrizedElements.addElementInversion(&symmetrizedElements, center);
	else  if(numRefs == 6)
	{
		gint i,j;
		for( i=0, c3si = c3s; c3si != NULL;  c3si = c3si->next, i++)
		for( j=0, c3sj = c3s; c3sj != NULL;  c3sj = c3sj->next, j++)
		if(j>=i)
		{
			Point3D* pi = (Point3D*)(c3si->data);
			Point3D* pj = (Point3D*)(c3sj->data);
			Point3D cp = pi->crossProd(pi,pj);
			Element ref = newElementReflection(center, &cp);
			Point3D* normal = ref.getNormal(&ref);
			if(!normal->equals(normal, &O)) symmetrizedElements.addElement(&symmetrizedElements, &ref);
		}
	}
	for( c3si = c3s; c3si != NULL;  c3si = c3si->next) if( c3si->data) g_free(c3si->data);
	g_list_free(c3s);

	return symmetrizedElements;
}
/************************************************************************************************************/
static SMolecule symmetrizeMolecule(PointGroup* pointGroup, gdouble tolerance)
{
	Elements oldElements = pointGroup->elements.getCopy(&pointGroup->elements);
	Elements idealElements = newElements();

	GList* l;
	gint i;
	SMolecule* mol = &pointGroup->molecule;
	Element startElement;
	gchar* name = pointGroup->getName(pointGroup);
	pointGroup->isCubicGroup = isCubic(pointGroup);

	if(pointGroup->elements.size(&pointGroup->elements) == 0) return *mol;
	startElement = pointGroup->elements.get(&pointGroup->elements,0);

	for(l = pointGroup->elements.getElements(&pointGroup->elements) ; l != NULL; l = l->next)
	{
		Element* elem = (Element*) l->data;
		if(elem->getDegree(elem)> startElement.getDegree(&startElement))
		if(!pointGroup->isCubicGroup || elem->type==ROTATION) startElement = *elem;
	}
	idealElements.addElement(&idealElements, &startElement);
	oldElements.remove(&oldElements, &startElement);
	if(!strcmp(name,"T") || !strcmp(name,"Th")  || !strcmp(name,"Td"))
	{
		gint numReflections = 0;
		if(!strcmp(name,"Th")) numReflections = 3;
		if(!strcmp(name,"Td")) numReflections = 6;
		idealElements.clear(&idealElements);
		idealElements = symmetrizeTetrahedralElements(pointGroup, &startElement, numReflections);
		oldElements.clear(&oldElements);
	}
	else if(!strcmp(name,"O")  || !strcmp(name,"Oh"))
	{
		idealElements.clear(&idealElements);
		idealElements = symmetrizeOctahedralElements(pointGroup, &startElement, !strcmp(name,"Oh"));
		oldElements.clear(&oldElements);
	}
	else if(!strcmp(name,"I")  || !strcmp(name,"Ih"))
	{
		idealElements.clear(&idealElements);
		idealElements = symmetrizeIcosahedralElements(pointGroup, &startElement, !strcmp(name,"Ih"));
		oldElements.clear(&oldElements);
	}
	else if(strstr(name,"inf"))
	{
		idealElements.clear(&idealElements);
		idealElements = pointGroup->elements.getCopy(&pointGroup->elements);
		oldElements.clear(&oldElements);
	}
	else
	{
		if(startElement.getDegree(&startElement) > 1)
		{
			Point3D* rotAxis = NULL;
			Point3D* refAxis = NULL;		
			Element* primaryRotation = &startElement;
			Point3D* center = primaryRotation->getPoint(primaryRotation);
			gdouble maxAngle = (2 * M_PI / primaryRotation->getDegree(primaryRotation));
			if(primaryRotation->type==ROTATION) maxAngle /= 2;
			for(i=0;i< oldElements.size(&oldElements); i++)
			{
				Element e = oldElements.get(&oldElements, i);
				Element* elem = &e;
				if(elem->type==INVERSION) 
				{
					idealElements.addElementInversion(&idealElements, center);
					oldElements.remove(&oldElements, elem);
					i--;
				}
				else
				{
					Point3D* currentAxis = NULL;
					Point3D* otherAxis = NULL;
					Point3D* newAxis = NULL;
					gdouble dotprod = 0;
					if(elem->type==ROTATION || elem->type==IMPROPERROTATION) 
					{
						newAxis = elem->getAxis(elem);
						Point3D* paxis = primaryRotation->getAxis(primaryRotation);
						dotprod = newAxis->dotProd(newAxis, paxis);
						currentAxis = rotAxis;
						otherAxis = refAxis;
					}
					else if(elem->type==REFLECTION) 
					{
						newAxis = elem->getNormal(elem);
						Point3D* paxis = primaryRotation->getAxis(primaryRotation);
						dotprod = newAxis->dotProd(newAxis, paxis);
						currentAxis = refAxis;
						otherAxis = rotAxis;
					}
					if(fabs(1.0 - fabs(dotprod)) < pointGroup->symmetrizeTolerance)
					{
						Point3D* paxis = primaryRotation->getAxis(primaryRotation);
						if(elem->type==ROTATION) idealElements.addElementRotation(&idealElements, center, paxis, elem->getDegree(elem));
						else if(elem->type==IMPROPERROTATION) idealElements.addElementImproperRotation(&idealElements, center, paxis, elem->getDegree(elem));
						else if(elem->type==REFLECTION) idealElements.addElementReflection(&idealElements, center, paxis);
						oldElements.remove(&oldElements, elem);
						i--;
					}
					else if(fabs(dotprod) < pointGroup->symmetrizeTolerance)
					{
						if(currentAxis == NULL)
						{
							currentAxis = newAxis->copy(newAxis);
							Point3D* paxis = primaryRotation->getAxis(primaryRotation);
							Point3D rotPoint = center->add(center,currentAxis);
							gdouble angle = M_PI / 2 - currentAxis->angleBetween(currentAxis, paxis);
							Point3D cp = currentAxis->crossProd(currentAxis, paxis);
							
							rotPoint = rotPoint.rotate(&rotPoint, center, &cp, -angle);
							currentAxis = currentAxis->copyPoint(rotPoint.sub(&rotPoint, center));
							
							if(otherAxis != NULL)
							{
								gdouble currentAngle = currentAxis->angleBetween(currentAxis, otherAxis);
								gdouble correctAngle = (gint)(0.5+currentAngle/(maxAngle/2))*(maxAngle/2);
								gdouble angleOffset = correctAngle - currentAngle;
								
								rotPoint = center->add(center,currentAxis);
								rotPoint = rotPoint.rotate(&rotPoint, center, paxis, angleOffset);
								currentAxis = currentAxis->copyPoint(rotPoint.sub(&rotPoint, center));
							}
						}
						else
						{
							Point3D* paxis = primaryRotation->getAxis(primaryRotation);
							Point3D caxis = *currentAxis;
							caxis = caxis.add(&caxis, center);
							caxis = caxis.rotate(&caxis, center, paxis, maxAngle);
							caxis = caxis.sub(&caxis, center);
							if(currentAxis) g_free(currentAxis);
							currentAxis = currentAxis->copy(&caxis);
						}
						
						if(elem->type==ROTATION) 
						{
							idealElements.addElementRotation(&idealElements, center, currentAxis, elem->getDegree(elem));
							rotAxis = currentAxis;
							refAxis = otherAxis;
						}
						else if(elem->type==IMPROPERROTATION)
						{
							idealElements.addElementImproperRotation(&idealElements, center, currentAxis, elem->getDegree(elem));
							rotAxis = currentAxis;
							refAxis = otherAxis;
						}
						if(elem->type==REFLECTION) 
						{
							idealElements.addElementReflection(&idealElements, center, currentAxis);
							refAxis = currentAxis;
							rotAxis = otherAxis;
						}
						oldElements.remove(&oldElements, elem);
						i--;
					}
				}
			}
		}
	}
	idealElements.print(&idealElements);

	pointGroup->elements.clear(&pointGroup->elements);
	pointGroup->elements = idealElements.getCopy(&idealElements);

	pointGroup->findUniqueAtoms(pointGroup, tolerance);
	pointGroup->adjustUniqueAtomsToElements(pointGroup,tolerance);
	SMolecule symMol = createSymmetrizedMolecule(pointGroup, tolerance);
	return symMol;
} 
/************************************************************************************************************/
static gchar** getListOfGroups(PointGroup* pointGroup)
{
	static gchar* list[] = {
	"C1" ,"Cs" ,"Ci" ,"C2" ,"C3" ,"C4" ,"C5" ,"C6" ,"C7" ,"C8" ,"D2" ,"D3" ,"D4" ,"D5" ,"D6" ,"C2v" ,"C3v",
	"C4v" ,"C5v" ,"C6v" ,"D2d" ,"D3d" ,"D4d" ,"D5d" ,"D6d" ,"C2h" ,"C3h" ,"C4h" ,"C5h" ,"C6h" ,"D2h" ,"D3h",
	"D4h" ,"D5h" ,"D6h" ,"S4" ,"S6" ,"S8" ,"T" ,"Th" ,"Td" ,"O" ,"Oh" ,"I" ,"Ih" ,"Cinfv" ,"Dinfh", NULL
	};
	return list;
}
/************************************************************************************************************/
static void freePointGroup(PointGroup* pointGroup)
{
	pointGroup->molecule.free(&pointGroup->molecule);
	pointGroup->uniqueMolecule.free(&pointGroup->uniqueMolecule);
	pointGroup->elements.free(&pointGroup->elements);
}
