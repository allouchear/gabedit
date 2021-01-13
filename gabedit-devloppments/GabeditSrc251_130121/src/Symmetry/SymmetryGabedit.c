/* Symmetry.c */
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
#include "../Symmetry/SymmetryGabedit.h"

/************************************************************************************************************/
static Elements getElements(Symmetry* symmetry);
static PointGroups getPointGroups(Symmetry* symmetry);
static SMolecule getMolecule(Symmetry* symmetry);
static void initSymmetry(Symmetry* symmetry);
static PointGroups findAllPointGroups(Symmetry* symmetry);
static void findProperRotationAxes(Symmetry* symmetry);
static void findImproperRotationAxes(Symmetry* symmetry);
static void findReflectionPlanes(Symmetry* symmetry);
static void findSymmetryElements(Symmetry* symmetry);
static void findPointGroups(Symmetry* symmetry);
static void findInversionCenter(Symmetry* symmetry);
static Point3D findCenterOfMass(SMolecule* mol);
static void findPrincipalAxes(SMolecule* mol, Point3D centerOfMass, gdouble momentsOutput[], Point3D axesOutput[]);
static  gboolean addElement(Symmetry* symmetry, Element* elem);
static gdouble testSymmetryElement(Symmetry* symmetry, Element* elem);
static gboolean isInertiallyAllowed(Symmetry* symmetry, Point3D* axis);
static void printPointGroupResults(Symmetry* symmetry);
static void getPrincipalAxisRotations(Symmetry* symmetry);
static void getAtomRotations(Symmetry* symmetry);
static void getMidpointRotations(Symmetry* symmetry);
static void getFaceRotations(Symmetry* symmetry);
static void getRotationNormals(Symmetry* symmetry);
static void getMidpointNormals(Symmetry* symmetry);
static void findReflectionPlanes(Symmetry* symmetry);
static gchar* findSinglePointGroup(Symmetry* symmetry);
static void printElementResults(Symmetry* symmetry);
static SMolecule getUniqueMolecule(Symmetry* symmetry);
static SMolecule getSymmetrizeMolecule(Symmetry* symmetry);
static void freeSymmetry(Symmetry* symmetry);
static void setMaxDegree(Symmetry* symmetry, gint maxDegree);
static void setMomentTolerance(Symmetry* symmetry, gdouble tolerance);
static gchar* getGroupName(Symmetry* symmetry);
/************************************************************************************************************/
static void initSymmetry(Symmetry* symmetry)
{
	
	gint i;
	symmetry->GAUSSIAN_SYMMETRY_TOLERANCE = 1e-6;
	symmetry->MAX_DEGREE = 6;
	symmetry->DEFAULT_TOLERANCE = 0.1;
	symmetry->MOMENT_TOLERANCE = 0.1;
	symmetry->DOT_TOLERANCE = 0.017365 * 2;
	symmetry->GABEDIT_T_APOTHEM = 0.3333333333;
	
	symmetry->elements = newElements();
	symmetry->molecule = newSMolecule();
	symmetry->pointGroups = newPointGroups();
	symmetry->rotations = newElements();
	symmetry->primaryAxis = NULL;

	symmetry->tolerance = symmetry->DEFAULT_TOLERANCE;
	
	symmetry->centerOfMass = newPoint3D();
	for(i=0;i<3;i++)
	{
		symmetry->principalAxes[i] = newPoint3D();
		symmetry->principalMoments[i] = 0;
	}
	symmetry->degeneracy = 1;
	symmetry->getElements = getElements;
	symmetry->getMolecule = getMolecule;
	symmetry->getPointGroups = getPointGroups;
	symmetry->findAllPointGroups = findAllPointGroups;
	symmetry->findProperRotationAxes = findProperRotationAxes;
	symmetry->findImproperRotationAxes = findImproperRotationAxes;
	symmetry->findReflectionPlanes = findReflectionPlanes;
	symmetry->findSymmetryElements = findSymmetryElements;
	symmetry->findPointGroups = findPointGroups;
	symmetry->findInversionCenter = findInversionCenter;
	symmetry->printPointGroupResults = printPointGroupResults;
	symmetry->findSinglePointGroup = findSinglePointGroup;
	symmetry->printElementResults = printElementResults;
	symmetry->getUniqueMolecule=getUniqueMolecule;
	symmetry->getSymmetrizeMolecule=getSymmetrizeMolecule;
	symmetry->free=freeSymmetry;
	symmetry->clear=freeSymmetry;
	symmetry->setMaxDegree=setMaxDegree;
	symmetry->setMomentTolerance= setMomentTolerance;
	symmetry->getGroupName = getGroupName;

}
/************************************************************************************************************/
Symmetry newSymmetry(SMolecule* mol,gdouble tolerance)
{
	Symmetry symmetry;
	initSymmetry(&symmetry);
	if(tolerance<=0) tolerance = 0.1;
	symmetry.tolerance = tolerance;
	symmetry.molecule = mol->getCopy(mol);

	return symmetry;
}
/************************************************************************************************************/
static PointGroups findAllPointGroups(Symmetry* symmetry)
{
	SMolecule* mol = &symmetry->molecule;
	if(mol->size(mol) < 2) return newPointGroups();

	symmetry->findSymmetryElements(symmetry);	
	symmetry->printElementResults(symmetry);
	symmetry->findPointGroups(symmetry);
	return symmetry->pointGroups;
}
/************************************************************************************************************/
/**
 * Calls delegate methods for calculating the physical properties of the molecule
 * and searching for the individual symmetry elements.
*/
static void findSymmetryElements(Symmetry* symmetry)
{
	Point3D tempPrincipalAxes[3];
	gboolean isLinear = FALSE;
	gint i;
	SMolecule* mol = &symmetry->molecule;
	gdouble minMoment;
	gdouble maxMoment;
	gdouble momentDiff;

	symmetry->elements = newElements();
	symmetry->centerOfMass = findCenterOfMass(mol);
	findPrincipalAxes(mol, symmetry->centerOfMass, symmetry->principalMoments, tempPrincipalAxes);
	for(i=0; i<3; i++)
	{
		symmetry->principalAxes[i] = newPoint3Dxyz(tempPrincipalAxes[i].x, tempPrincipalAxes[i].y, tempPrincipalAxes[i].z);
	}
		
	symmetry->degeneracy = 1;
	minMoment = fmin(symmetry->principalMoments[0], fmin(symmetry->principalMoments[1], symmetry->principalMoments[2]));
	maxMoment = fmax(symmetry->principalMoments[0], fmax(symmetry->principalMoments[1], symmetry->principalMoments[2]));
	momentDiff = (maxMoment - minMoment) / maxMoment;
	if(momentDiff < symmetry->MOMENT_TOLERANCE) symmetry->degeneracy = 3;
	else if(fabs(symmetry->principalMoments[0] - symmetry->principalMoments[1]) / maxMoment < symmetry->MOMENT_TOLERANCE || 
		fabs(symmetry->principalMoments[1] - symmetry->principalMoments[2]) / maxMoment < symmetry->MOMENT_TOLERANCE || 
		fabs(symmetry->principalMoments[0] - symmetry->principalMoments[2]) / maxMoment < symmetry->MOMENT_TOLERANCE)
	{
		symmetry->degeneracy = 2;
		if(minMoment < symmetry->MOMENT_TOLERANCE)
		{
			gint minPos = symmetry->principalMoments[1] == minMoment ? 1 : symmetry->principalMoments[2] == minMoment ? 2 : 0;
			Element infRot = newElementRotation(&symmetry->centerOfMass, &symmetry->principalAxes[minPos], -1);
			gdouble distance = 0;
			gint i;
			for(i = 0; i<mol->size(mol); i++)
			{
				SAtom* atom = mol->get(mol,i);
				Point3D pos = atom->getPosition(atom);
				Point3D* rot = infRot.getPoint(&infRot);
				Point3D* axis = infRot.getAxis(&infRot);
				Point3D closP = pos.closestPointOnAxis(&pos, rot, axis);
				distance += pos.distance(&pos, &closP);
			}
			distance /= mol->size(mol);
			infRot.setDistance(&infRot,distance);
			symmetry->elements.addElement(&symmetry->elements, &infRot);
			isLinear = TRUE;
		}
	}
		
	symmetry->rotations = newElements();
		
	symmetry->findInversionCenter(symmetry);
		
	if(!isLinear)
	{
		symmetry->findProperRotationAxes(symmetry);	
		symmetry->findImproperRotationAxes(symmetry);
		symmetry->findReflectionPlanes(symmetry);
	}
}
/************************************************************************************************************/
static Point3D findCenterOfMass(SMolecule* mol)
{
	gdouble totalMass = 0;
	gdouble comX = 0;
	gdouble comY = 0;
	gdouble comZ = 0;
	gint i;
	for(i = 0; i<mol->size(mol); i++)
	{
		SAtom* a = mol->get(mol,i);
		gdouble m = a->getMass(a);
		comX += a->getPosition(a).x * m;
		comY += a->getPosition(a).y * m;
		comZ += a->getPosition(a).z * m;
		totalMass += m;
	}
	return newPoint3Dxyz(comX/totalMass, comY/totalMass, comZ/totalMass);
}
/************************************************************************************************************/
static void findInversionCenter(Symmetry* symmetry)
{
	Element inv = newElementInversion(&symmetry->centerOfMass);
	testSymmetryElement(symmetry, &inv);
	addElement(symmetry, &inv);
}
/************************************************************************************************************/
#define EPSILON 1.0E-12
#define SQU(x,y,z) ((x)*(x) + (y)*(y) + (z)*(z))
/* Jacobi diagonalisation of 3x3 symmetric matrix */
/* matrix mat stored like   0 3 5    
                              1 4
                                2   */
static void jacobi(gdouble *mat, gdouble evec[3][3])
{
  
	gdouble t,s,u;
	gdouble a;
	evec[0][1] = evec[0][2] = evec[1][0] = 0.0;  /* unity matrix */
	evec[1][2] = evec[2][0] = evec[2][1] = 0.0;  /* unity matrix */
	evec[0][0] = evec[1][1] = evec[2][2] = 1.0;

	/* do jacobi sweep */
	while(SQU(mat[3],mat[4],mat[5]) > EPSILON)
	{
		/* set mat[3] to zero */
		if (mat[3]*mat[3] > EPSILON)
		{
			t = (mat[1]-mat[0])/(2.*mat[3]);
	 		t = (t>0) ? 1./(t+sqrt(t*t+1.)) : -1./(-t+sqrt(t*t+1.));
	 		s = t/(sqrt(t*t+1.));
	  		u = s*t/(s+t);

	  		mat[0] -= t*mat[3];
	  		mat[1] += t*mat[3];
	  		a = mat[5];
	  		mat[5] -= s*(mat[4]+u*mat[5]);
	  		mat[4] += s*(  a   -u*mat[4]);
	  		mat[3] = 0.;
	  
	 		a = evec[0][0];
	  		evec[0][0] -= s*(evec[0][1]+u*evec[0][0]);
	  		evec[0][1] += s*(   a   -u*evec[0][1]);

	  		a = evec[1][0];
	  		evec[1][0] -= s*(evec[1][1]+u*evec[1][0]);
	  		evec[1][1] += s*(   a   -u*evec[1][1]);

	 		a = evec[2][0];
	  		evec[2][0] -= s*(evec[2][1]+u*evec[2][0]);
	  		evec[2][1] += s*(   a   -u*evec[2][1]);
		}

		/* set mat[5] to zero */
		if (mat[5]*mat[5] > EPSILON)
		{
			t = (mat[2]-mat[0])/(2.*mat[5]);
	  		t = (t>0) ? 1./(t+sqrt(t*t+1.)) : -1./(-t+sqrt(t*t+1.));
	  		s = t/(sqrt(t*t+1.));
	  		u = s*t/(s+t);

	  		mat[0] -= t*mat[5];
	  		mat[2] += t*mat[5];
	  		a = mat[3];
	  		mat[3] -= s*(mat[4]+u*mat[3]);
	  		mat[4] += s*(  a   -u*mat[4]);
	  		mat[5] = 0.;
	  
	  		a = evec[0][0];
	  		evec[0][0] -= s*(evec[0][2]+u*evec[0][0]);
	  		evec[0][2] += s*(   a   -u*evec[0][2]);

	  		a = evec[1][0];
	  		evec[1][0] -= s*(evec[1][2]+u*evec[1][0]);
	  		evec[1][2] += s*(   a   -u*evec[1][2]);

	  		a = evec[2][0];
	  		evec[2][0] -= s*(evec[2][2]+u*evec[2][0]);
	  		evec[2][2] += s*(   a   -u*evec[2][2]);
		}

		/* set mat[4] to zero */
		if (mat[4]*mat[4] > EPSILON)
		{
	  		t = (mat[2]-mat[1])/(2.*mat[4]);
	  		t = (t>0) ? 1./(t+sqrt(t*t+1.)) : -1./(-t+sqrt(t*t+1.));
	  		s = t/(sqrt(t*t+1.));
	  		u = s*t/(s+t);

	  		mat[1] -= t*mat[4];
	  		mat[2] += t*mat[4];
	  		a = mat[3];
	  		mat[3] -= s*(mat[5]+u*mat[3]);
	  		mat[5] += s*(  a   -u*mat[5]);
	  		mat[4] = 0.;

	  		a = evec[0][1];
	  		evec[0][1] -= s*(evec[0][2]+u*evec[0][1]);
	  		evec[0][2] += s*(   a   -u*evec[0][2]);

	  		a = evec[1][1];
	  		evec[1][1] -= s*(evec[1][2]+u*evec[1][1]);
	  		evec[1][2] += s*(   a   -u*evec[1][2]);

	  		a = evec[2][1];
	  		evec[2][1] -= s*(evec[2][2]+u*evec[2][1]);
	  		evec[2][2] += s*(   a   -u*evec[2][2]);
		}
	}
}
/************************************************************************************************************/
static void swap(gint i,gint j,gdouble* mat, gdouble vecs[3][3])
{
	gint k;
	gdouble t;
	t = mat[i];
	mat[i] = mat[j];
	mat[j] = t;
	for(k=0;k<3;k++)
	{
		t = vecs[k][i];
		vecs[k][i] = vecs[k][j];
		vecs[k][j] = t;
	}
}
/********************************************************************************************************************************************/
static void findPrincipalAxes(SMolecule* mol, Point3D centerOfMass, gdouble inertialMoment[], Point3D axesOutput[])
{
	gint i;
	gdouble mat[6];
	gdouble axes[3][3];
	for(i=0;i<6;i++) mat[i]=0.0;

	/* build up inertial tensor */
	for(i = 0; i<mol->size(mol); i++)
	{
		SAtom* atom = mol->get(mol,i);
		Point3D a = atom->getPosition(atom);
		gdouble m = atom->getMass(atom);
		
		mat[0] += m*(pow(a.y-centerOfMass.y,2)+pow(a.z-centerOfMass.z,2));
		mat[1] += m*(pow(a.z-centerOfMass.z,2)+pow(a.x-centerOfMass.x,2));
		mat[2] += m*(pow(a.x-centerOfMass.x,2)+pow(a.y-centerOfMass.y,2));
		mat[3] -= m*(a.x-centerOfMass.x)*(a.y-centerOfMass.y);
		mat[4] -= m*(a.y-centerOfMass.y)*(a.z-centerOfMass.z);
		mat[5] -= m*(a.x-centerOfMass.x)*(a.z-centerOfMass.z);
	}
	jacobi(mat,axes);/* diagonalize tensor */
	/* sort eigenvalues */
	if (mat[0]<mat[1]) swap(0,1,mat,axes);
	if (mat[1]<mat[2]) swap(1,2,mat,axes);
	if (mat[0]<mat[1]) swap(0,1,mat,axes);

	inertialMoment[0] = mat[0];
	inertialMoment[1] = mat[1];
	inertialMoment[2] = mat[2];

	for(i = 0; i<3; i++)
	{
		axesOutput[i].x = axes[0][i];
		axesOutput[i].y = axes[1][i];
		axesOutput[i].z = axes[2][i];
	}
}
/************************************************************************************************************/
static Elements getElements(Symmetry* symmetry)
{
	return symmetry->elements;
}
/************************************************************************************************************/
static PointGroups getPointGroups(Symmetry* symmetry)
{
	return symmetry->pointGroups;
}
/************************************************************************************************************/
static SMolecule getMolecule(Symmetry* symmetry)
{
	return symmetry->molecule;
}
/************************************************************************************************************/
static  gboolean addElement(Symmetry* symmetry, Element* elem)
{
	Elements* elements = &symmetry->elements;
	if(elem->getDistance(elem) <  symmetry->tolerance)
	{
		gboolean found = FALSE;
		gint i;
		for(i=0; i<elements->size(elements); i++)
		 {
			Element check = elements->get(elements,i);
			if(elem->equals(elem,&check))
			{
				found = TRUE;
				if(elem->getDistance(elem) < check.getDistance(&check)) {
					elements->remove(elements, &check);
					elements->addElement(elements, elem);
					return TRUE;
				}
			}
		}
		if(found == FALSE)
		{
			elements->addElement(elements, elem);
			return TRUE;
		}
	}
	if(!elements->contains(elements, elem, symmetry->tolerance) && elem->getDistance(elem) < symmetry->tolerance)
	{
		elements->addElement(elements, elem);
		return TRUE;
	}
	return FALSE;
}
/************************************************************************************************************/
static void printElementResults(Symmetry* symmetry)
{
	Elements* elements = &symmetry->elements;
	if(elements->size(elements) > 0)
	{
		gint i;
		/* printf("\nELEMENTS FOUND:\n");*/
		printf("\nELEMENTS FOUND: ");
		for(i=0; i<elements->size(elements); i++)
		{
			Element elem = elements->get(elements,i);
			printf("%s ", elem.getName(&elem));
			/* printf("\t%s with a rating of %f\n", elem.getName(&elem), elem.getDistance(&elem));*/
			/* printf("\t%s with a rating of %f\n", elem.toString(&elem), elem.getDistance(&elem));*/
		}
		printf("\n\n");
	}
	else printf("\n--- NO ELEMENTS FOUND! --- \n");
}
/************************************************************************************************************/
static void printPointGroupResults(Symmetry* symmetry)
{
	PointGroup finalGroup = symmetry->pointGroups.get(&symmetry->pointGroups,0);
	gchar* exact = "===========================";
	if(finalGroup.getNumExtraElements(&finalGroup) == 0)
		printf("%s\n%2d %s : %f \n%s\n",exact, 1, finalGroup.getName(&finalGroup), finalGroup.getDistance(&finalGroup), exact);
	else
		printf("\n%2d %s : %f \n", 1, finalGroup.getName(&finalGroup), finalGroup.getDistance(&finalGroup));
		
	{
		gint i;
		for(i=1; i<symmetry->pointGroups.size(&symmetry->pointGroups); i++)
		{
			finalGroup = symmetry->pointGroups.get(&symmetry->pointGroups,i);
			printf("%2d %s : %f \n", i+1, finalGroup.getName(&finalGroup), finalGroup.getDistance(&finalGroup));
		}
	}
	printf("\n");
}
/************************************************************************************************************/
static gdouble testSymmetryElement(Symmetry* symmetry, Element* elem)
{
	gdouble totalDist = 0;
	gdouble subTotalDist = 0;
	gint numOperations = 1;
	gint i;
	SMolecule* mol = &symmetry->molecule;
	GList* atoms = mol->getAtoms(mol);

	if(elem->getDegree(elem) > 1) numOperations = elem->getDegree(elem) - 1;
	for(i = 0; i < numOperations; i++)
	{
		gint j;
		for(j = 0; j<mol->size(mol); j++)
		{
			SAtom* beforee = mol->get(mol,j);
			SAtom after = elem->doOperationSAtom(elem,beforee);
			SAtom closestAtom = after.findClosestAtom(&after, atoms);
			gdouble shortestDist = after.distance(&after,&closestAtom);
			gdouble dist = 1.0;
			if (elem->type == INVERSION)
			{
				Point3D p = closestAtom.getPosition(&closestAtom);
				dist = p.distance(&p,&symmetry->centerOfMass);
			}
			else if (elem->type == REFLECTION)
			{
				Point3D ca = closestAtom.getPosition(&closestAtom);
				Point3D* pe = elem->getPoint(elem);
				Point3D* ne = elem->getNormal(elem);
				Point3D closestPoint = ca.closestPointInPlane(&ca, pe, ne); 
				dist = ca.distance(&ca,&closestPoint);
			}
			else if (elem->type == ROTATION)
			{
				Point3D ca = closestAtom.getPosition(&closestAtom);
				Point3D* pe = elem->getPoint(elem);
				Point3D* ae = elem->getAxis(elem);
				Point3D closestPoint = ca.closestPointOnAxis(&ca, pe, ae);
				dist = ca.distance(&ca,&closestPoint);
			}
			if(dist > 1) shortestDist /= dist;
			subTotalDist = fmax(subTotalDist, shortestDist);
		}
		if(subTotalDist > symmetry->tolerance)
		{
			elem->setDistance(elem,subTotalDist);
			return subTotalDist;
		}
		totalDist += subTotalDist;
	}
	totalDist /= numOperations;
	elem->setDistance(elem,totalDist);
	return totalDist;
}
/************************************************************************************************************/
static gboolean isInertiallyAllowed(Symmetry* symmetry, Point3D* axis)
{
	gint degeneracy = symmetry->degeneracy;
	Point3D point = *axis;

	if(degeneracy == 3) return TRUE;
	else
	{
		gdouble dotproducts[3];
		gdouble maxDotprod = 0;
		gdouble minDotprod = 1;
		Point3D checkPoint = point;
		gint i;
		checkPoint.unit(&checkPoint);
		for(i=0; i<3; i++)
		{
			dotproducts[i] = fabs(checkPoint.dotProd(&checkPoint,&symmetry->principalAxes[i]));
			if(dotproducts[i] > maxDotprod) maxDotprod = dotproducts[i];
			if(dotproducts[i] < minDotprod) minDotprod = dotproducts[i];
		}
		if(fabs(minDotprod) < symmetry->DOT_TOLERANCE)
		{
			if(degeneracy == 2) return TRUE;
			else if(fabs(1.0 - maxDotprod) < symmetry->DOT_TOLERANCE) return TRUE;
		}
		return FALSE;
	}
}
/************************************************************************************************************/
static void findPointGroups(Symmetry* symmetry)
{
	SMolecule* mol = &symmetry->molecule;
	symmetry->pointGroups = newPointGroups();
	PointGroup testGrp = newPointGroup(NULL,NULL,"C1");
	gchar** listOfGroups = testGrp.getListOfGroups(&testGrp);
	gint j;
	Elements* elements = &symmetry->elements;
	SOperations operations;
	GList* l;
	GList* listOp;
	GList* listE;
	PointGroup ptGrp;
	gdouble ptGrpDistance = 0;

	for(j = 0; listOfGroups[j] != NULL; j++)
	{
		Elements foundElements = elements->getCopy(elements);
		Elements finalElements = newElements();
		gint missingElements = 0;
		gint extraElements = 0;
		gint totalOperations = 0;
		Element* groupPrimaryAxis = NULL;
		gboolean groupPrimaryAxisSet = FALSE;

		if(symmetry->primaryAxis) copyElement(symmetry->primaryAxis, &groupPrimaryAxis);

		testGrp = newPointGroup(NULL,NULL,listOfGroups[j]);
		operations = getSOperationsForOneGroup(listOfGroups[j]);

		if(testGrp.isCubic(&testGrp))
		{
			groupPrimaryAxis = NULL;
			groupPrimaryAxisSet = TRUE;
		}
		listOp = operations.getSOperations(&operations);
		for(l = listOp; l != NULL; l = l->next)
		{
			SOperation testOp = *(SOperation*)l->data;
			gboolean moreElements = TRUE;
			gint count = 1;
			gint num = testOp.getNumber(&testOp);
			gboolean coincidentWithPrimary = TRUE;
				
			while(count <= num && moreElements)
			{
				Element* bestElement = NULL;
				gint i;

				moreElements = FALSE;
				for(i=0; i<foundElements.size(&foundElements); i++)
				{
					Element checkElement = foundElements.get(&foundElements,i);
					if(!strcmp(checkElement.getName(&checkElement),testOp.getElementType(&testOp)))
					{
						if(bestElement == NULL || checkElement.getDistance(&checkElement) < bestElement->getDistance(bestElement))
						{
							if(groupPrimaryAxis != NULL)
							{
								Point3D* checkAxis = NULL;
									
								if(checkElement.type == ROTATION) checkAxis = checkElement.getAxis(&checkElement);
								else if(checkElement.type == REFLECTION) checkAxis  = checkElement.getNormal(&checkElement);
								
								if(checkAxis != NULL)
								{
									if(coincidentWithPrimary)
									 {
										Point3D* paxis = groupPrimaryAxis->getAxis(groupPrimaryAxis);
										if(1 - fabs(checkAxis->dotProd(checkAxis, paxis)) < symmetry->DOT_TOLERANCE)
											copyElement(&checkElement, &bestElement);
									}
									else
									{
										Point3D* paxis = groupPrimaryAxis->getAxis(groupPrimaryAxis);
										if(fabs(checkAxis->dotProd(checkAxis, paxis)) < symmetry->DOT_TOLERANCE)
											copyElement(&checkElement, &bestElement);
									}
								}
								else copyElement(&checkElement, &bestElement);
							}
							else copyElement(&checkElement, &bestElement);
						}
					}
				}
				if(bestElement != NULL)
				{
					if(bestElement->type==ROTATION && !groupPrimaryAxisSet)
					{
						copyElement(bestElement, &groupPrimaryAxis);
						groupPrimaryAxisSet = TRUE;
					}
					foundElements.remove(&foundElements, bestElement);
					finalElements.addElement(&finalElements, bestElement);
					count += bestElement->getNumUniqueOperations(bestElement);
					totalOperations += bestElement->getNumUniqueOperations(bestElement);
					moreElements = TRUE;
					g_free(bestElement);
				}
				else if(coincidentWithPrimary)
				{
					coincidentWithPrimary = FALSE;
					moreElements = TRUE;
				}
				if(moreElements == FALSE) missingElements += num - count + 1;
			}
		}
		if(groupPrimaryAxis) g_free(groupPrimaryAxis);

		listE = foundElements.getElements(&foundElements);
		for(l = listE; l != NULL; l = l->next)
		{
			Element* e = (Element*) l->data;
			extraElements += e->getNumUniqueOperations(e);
		}
		ptGrp = newPointGroup(&finalElements, mol, listOfGroups[j]);
		
		ptGrpDistance = 0;
		listE = finalElements.getElements(&finalElements);
		for(l = listE; l != NULL; l = l->next)
		{
			Element* e = (Element*) l->data;
			ptGrpDistance += e->getDistance(e);
		}
		ptGrp.setDistance(&ptGrp, ptGrpDistance/(finalElements.size(&finalElements) == 0 ? 1 : finalElements.size(&finalElements)));
		ptGrp.setNumExtraElements(&ptGrp, extraElements);
		ptGrp.setNumMissingElements(&ptGrp, missingElements);
		
		if(missingElements == 0)
		{
			gint index = 0;
			gint i;
			PointGroups* pgs = &symmetry->pointGroups;
			for(i=0; i<pgs->size(pgs); i++)
			{
				PointGroup pg = pgs->get(pgs,i);
				gint ne = pg.getNumExtraElements(&pg);
				if(extraElements > ne) index++;
				else if(extraElements == ne) {
					if(ptGrp.getDistance(&ptGrp) > pg.getDistance(&pg)) index++;
				}
				else break;
			}
			pgs->addPointGroupPos(pgs,index, &ptGrp);
		}	
	}
	symmetry->printPointGroupResults(symmetry); 
}
/************************************************************************************************************/
static void findProperRotationAxes(Symmetry* symmetry)
{
	gint degeneracy = symmetry->degeneracy;
	symmetry->rotations = newElements();
	getPrincipalAxisRotations(symmetry);
	getAtomRotations(symmetry);
	getMidpointRotations(symmetry);
	if(degeneracy == 3) getFaceRotations(symmetry);
	if(degeneracy != 3)
	{
		if(symmetry->primaryAxis == NULL)
		{
			GList* listE = symmetry->elements.getElements(&symmetry->elements);
			GList* l;
			for(l = listE; l != NULL; l = l->next)
			{
				Element* rot = (Element*) l->data;
				if(symmetry->primaryAxis == NULL || rot->getDegree(rot) > symmetry->primaryAxis->getDegree(symmetry->primaryAxis))
				{
					copyElement(rot, &symmetry->primaryAxis);
				}
			}
		}
	}
	else symmetry->primaryAxis = NULL;
}
/************************************************************************************************************/
static void getPrincipalAxisRotations(Symmetry* symmetry)
{
	gint i,n;
	Element rot;
	for(i=0; i<3; i++)
	{
		Point3D pAxis = symmetry->principalAxes[i];
		for(n=2; n<= symmetry->MAX_DEGREE; n++)
		{
			rot = newElementRotation(&symmetry->centerOfMass, &pAxis, n);
			testSymmetryElement(symmetry, &rot);
			if(addElement(symmetry, &rot))
			{
				if(symmetry->primaryAxis == NULL || rot.getDegree(&rot) > symmetry->primaryAxis->getDegree(symmetry->primaryAxis))
				{
					copyElement(&rot, &symmetry->primaryAxis);
				}
				symmetry->rotations.addElement(&symmetry->rotations, &rot);
			}
		}
		symmetry->rotations.addElementRotation(&symmetry->rotations, &symmetry->centerOfMass, &pAxis, 1);
	}
}
/************************************************************************************************************/
static void getAtomRotations(Symmetry* symmetry)
{
	SMolecule* mol = &symmetry->molecule;
	gint i,n;
	Element rot;

	for(i=0;i<mol->size(mol); i++)
	{
		SAtom* atom = mol->get(mol,i);
		Point3D pos = atom->getPosition(atom);
		Point3D axis = pos.sub(&pos, &symmetry->centerOfMass);
		if(isInertiallyAllowed(symmetry, &axis))
		{
			for(n=2; n<= symmetry->MAX_DEGREE; n++)
			{
				rot = newElementRotation(&symmetry->centerOfMass, &axis, n);
				testSymmetryElement(symmetry, &rot);
				if(addElement(symmetry, &rot))
				{
					if(symmetry->rotations.contains(&symmetry->rotations, &rot, symmetry->tolerance))
						symmetry->rotations.set(&symmetry->rotations, symmetry->rotations.indexOf(&symmetry->rotations, &rot), &rot);
					else
						symmetry->rotations.addElement(&symmetry->rotations, &rot);
				}
			}
		}
	}
}
/************************************************************************************************************/
static void getMidpointRotations(Symmetry* symmetry)
{
	SMolecule* mol = &symmetry->molecule;
	gint nAtoms = mol->size(mol);
	gint i,j, n;
	gint degeneracy = symmetry->degeneracy;
	Element rot;

	int nR = 0;
	for(i=0; i<nAtoms-1; i++)
	{
		SAtom* from = mol->get(mol,i);
		Point3D posfrom = from->getPosition(from);
		for(j=i+1; j<nAtoms; j++)
		{
			SAtom* to = mol->get(mol,j);
			if(to->getNumber(to) == from->getNumber(from) && (degeneracy != 3 || to->distance(to,from) < 5))
			{
				Point3D pos = to->getPosition(to);
				Point3D midPoint = pos.add(&pos,&posfrom);
				midPoint = midPoint.mult(&midPoint, 0.5);
				Point3D midAxis = midPoint.sub(&midPoint, &symmetry->centerOfMass);
				if(isInertiallyAllowed(symmetry, &midAxis))
				{
					for(n=2; n<=symmetry->MAX_DEGREE; n+=2)
					{
						rot = newElementRotation(&symmetry->centerOfMass, &midAxis, n);
						testSymmetryElement(symmetry, &rot);
						if(addElement(symmetry, &rot))
						{
							if(symmetry->rotations.contains(&symmetry->rotations, &rot, symmetry->tolerance))
							{
								symmetry->rotations.set(&symmetry->rotations, symmetry->rotations.indexOf(&symmetry->rotations, &rot), &rot);
							}
							else
							{
								symmetry->rotations.addElement(&symmetry->rotations, &rot);
								nR++;
							}
						}
					}
				}
			}	
		}
	}
}
/************************************************************************************************************/
static void getFaceRotations(Symmetry* symmetry)
{
	GList* listR = symmetry->rotations.getElements(&symmetry->rotations);
	GList* l;
	gint n[] = {3, 5};
	gint i,j,k;
	gint factor;
	gint c2sSize = 0;
	Element elemi;
	Element elemj;
	Point3D* axisi;
	Point3D* axisj;
	Point3D* axisk;
	Point3D distVector;
	Point3D midAxis;
	Element rot;
	Point3D* rot1;
	Point3D* rot2;
	gint polygon; 
	gint nlength = sizeof(n)/sizeof(n[0]);
		
	Elements c2s = newElements();
	for(l = listR; l != NULL; l = l->next)
	{
		Element* rot = (Element*) l->data;
		if(rot->getDegree(rot) == 2) c2s.addElement(&c2s, rot);
	}
	c2sSize = c2s.size(&c2s);
		
	if(c2sSize == 3)
	{
		for(i=0; i<c2sSize-1; i++)
		{
			elemi = c2s.get(&c2s,i);
			axisi = elemi.getAxis(&elemi);
			for(j=i; j<c2sSize; j++)
			{
				elemj = c2s.get(&c2s,j);
				axisj = elemj.getAxis(&elemj);
				for(factor=-1; factor<2; factor+=2)
				{
					Point3D midpoint = axisi->add(axisi, axisj);
					midpoint = midpoint.mult(&midpoint, 0.5*factor);
					for(k=0; k<c2sSize; k++)
					{
						Element elem;
						elem = c2s.get(&c2s,k);
						axisk = elem.getAxis(&elem);
						distVector = midpoint.sub(&midpoint, axisk);
						distVector = distVector.mult(&distVector, symmetry->GABEDIT_T_APOTHEM);
						midAxis = midpoint.sub(&midpoint, &distVector);
						rot = newElementRotation(&symmetry->centerOfMass, &midAxis, 3);
						testSymmetryElement(symmetry,&rot);
						if(addElement(symmetry, &rot))
						{
							if(symmetry->rotations.contains(&symmetry->rotations, &rot, symmetry->tolerance))
								symmetry->rotations.set(&symmetry->rotations, symmetry->rotations.indexOf(&symmetry->rotations, &rot), &rot);
							else
								symmetry->rotations.addElement(&symmetry->rotations, &rot);
						}
					}
				}
			}
		}
	} 
	else
	{
		for(i=0; i<c2sSize-1; i++)
		{
			elemi = c2s.get(&c2s,i);
			rot1 = elemi.getAxis(&elemi);
			for(j=i+1; j<c2sSize; j++)
			{
				elemj = c2s.get(&c2s,j);
				rot2 = elemj.getAxis(&elemj);
				for(polygon=0; polygon<nlength; polygon++)
				{
					Point3D c12 = rot1->crossProd(rot1,rot2);
					Element rot = newElementRotation(&symmetry->centerOfMass, &c12, n[polygon]);
					testSymmetryElement(symmetry, &rot);
					if(addElement(symmetry, &rot))
					{
						if(symmetry->rotations.contains(&symmetry->rotations, &rot, symmetry->tolerance))
							symmetry->rotations.set(&symmetry->rotations, symmetry->rotations.indexOf(&symmetry->rotations, &rot), &rot);
						else
							symmetry->rotations.addElement(&symmetry->rotations, &rot);
					}
				}
			}
		}
	}
}
/************************************************************************************************************/
static void findImproperRotationAxes(Symmetry* symmetry)
{
	gint n;
	GList* listR = symmetry->rotations.getElements(&symmetry->rotations);
	GList* l;
	for(l = listR; l != NULL; l = l->next)
	{
		Element* rot = (Element*) l->data;
		for(n=1; n<=2; n++)
		{
			gint degree = rot->getDegree(rot) * n;
			if(degree > 2)
			{
				Point3D* point = rot->getPoint(rot);
				Point3D* axis = rot->getAxis(rot);
				Element impRot = newElementImproperRotation(point, axis, degree);
				testSymmetryElement(symmetry, &impRot);
				addElement(symmetry, &impRot);
			}
		}
	}
}
/************************************************************************************************************/
static void findReflectionPlanes(Symmetry* symmetry)
{
	gboolean isTetrahedral = TRUE;
	gint degeneracy = symmetry->degeneracy;
	GList* listR = symmetry->rotations.getElements(&symmetry->rotations);
	GList* l;
	if(degeneracy == 3)
	{
		for(l = listR; l != NULL; l = l->next)
		{
			Element* elem = (Element*) l->data;
			if(!strcmp(elem->getName(elem),"S6"))
			{
				isTetrahedral = FALSE;
				break;
			}
		}
		if(!isTetrahedral)
		{
			gint i;
			for(i=0; i<symmetry->rotations.size(&symmetry->rotations); i++)
		 	{
				Element elem = symmetry->rotations.get(&symmetry->rotations,i);
				if(elem.getDegree(&elem) != 2)
				{
					symmetry->rotations.remove(&symmetry->rotations,&elem);
					i--;
				}
			}
		}
	}
	
	getRotationNormals(symmetry);
	
	if(degeneracy != 3 || isTetrahedral) getMidpointNormals(symmetry);
}
/************************************************************************************************************/
static void getRotationNormals(Symmetry* symmetry)
{
	GList* listR = symmetry->rotations.getElements(&symmetry->rotations);
	GList* l;
	for(l = listR; l != NULL; l = l->next)
	{
		Element* elem = (Element*) l->data;
		Point3D* point = elem->getPoint(elem);
		Point3D* axis = elem->getAxis(elem);
		Element ref = newElementReflection(point, axis);
		testSymmetryElement(symmetry, &ref);
		addElement(symmetry, &ref);
	}
}
/************************************************************************************************************/
static void getMidpointNormals(Symmetry* symmetry)
{
	SMolecule* mol = &symmetry->molecule;
	gint nAtoms = mol->size(mol);
	gint i,j;
	gint degeneracy = symmetry->degeneracy;

	for(i=0; i<nAtoms-1; i++)
	{
		SAtom* from = mol->get(mol,i);
		Point3D fromp = from->getPosition(from);
		fromp = fromp.sub(&fromp, &symmetry->centerOfMass);

		for(j=i+1; j<nAtoms; j++)
		{
			SAtom* to = mol->get(mol,j);
			if(to->getNumber(to) == from->getNumber(from) && (degeneracy != 3 || to->distance(to,from) < 5))
			{
				Point3D axis;
				Point3D crossProd;
				Point3D midPoint;
				Point3D top = to->getPosition(to);
				top = top.sub(&top, &symmetry->centerOfMass);
				midPoint = top.add(&top,&fromp);
				midPoint = midPoint.mult(&midPoint, 0.5);
				axis = fromp.crossProd(&fromp,&top);
				crossProd = axis.crossProd(&axis, &midPoint);


				if(isInertiallyAllowed(symmetry, &crossProd))
				{
					if(crossProd.length(&crossProd) > 0.001)
					{
						Element ref = newElementReflection(&symmetry->centerOfMass, &crossProd);
						testSymmetryElement(symmetry, &ref);
						addElement(symmetry, &ref);
					}
				}					
			}	
		}
	}
}
/************************************************************************************************************/
static gchar* findSinglePointGroup(Symmetry* symmetry)
{
	static gchar pointGroupGuess[100]; 
	gdouble GUESS_TOLERANCE = 0.1;
	Point3D tempPrincipalAxes[3];
	SMolecule* mol = &symmetry->molecule;
	gint nAtoms = mol->size(mol);
	gdouble minMoment;
	gdouble maxMoment;
	gdouble momentDiff;
	Element* primaryAxis = NULL;
	gint perpAxes = 0;
	gint perpPlanes = 0;
	gint i,j;
	Point3D orthogAxis;
	Element ref;
	Element rot;
	gint size;

	symmetry->tolerance = GUESS_TOLERANCE;
	sprintf(pointGroupGuess,"C1");
	symmetry->elements = newElements();
		
	if(nAtoms == 1)
	{ 
		
		sprintf(pointGroupGuess,"R3");
		return pointGroupGuess;
	}
		
	symmetry->centerOfMass = findCenterOfMass(mol);
	findPrincipalAxes(mol, symmetry->centerOfMass, symmetry->principalMoments, tempPrincipalAxes);

		
	minMoment = fmin(symmetry->principalMoments[0], fmin(symmetry->principalMoments[1], symmetry->principalMoments[2]));
	maxMoment = fmax(symmetry->principalMoments[0], fmax(symmetry->principalMoments[1], symmetry->principalMoments[2]));
	momentDiff = (maxMoment - minMoment) / maxMoment;

	if(minMoment < symmetry->MOMENT_TOLERANCE)
	{
		gint index = 0;
		Point3D refAxis;
		Element ref;

		if(symmetry->principalMoments[1] == minMoment) index = 1;
		else if(symmetry->principalMoments[2] == minMoment) index = 2;

		refAxis = newPoint3D(tempPrincipalAxes[index].x, tempPrincipalAxes[index].y, tempPrincipalAxes[index].z);
		ref = newElementReflection(&symmetry->centerOfMass, &refAxis);
		testSymmetryElement(symmetry, &ref);
		sprintf(pointGroupGuess,"Cinfv");
		if(addElement(symmetry, &ref)) sprintf(pointGroupGuess,"Dinfh");
	
		return pointGroupGuess;
	}
	if(momentDiff < symmetry->MOMENT_TOLERANCE) return "cubic";
		
	primaryAxis = NULL;
	for(i=0; i<3; i++)
	{
		Point3D pAxis = newPoint3Dxyz(tempPrincipalAxes[i].x, tempPrincipalAxes[i].y, tempPrincipalAxes[i].z);
		gint n;
		symmetry->principalAxes[i] = pAxis;
		for(n=2; n<= symmetry->MAX_DEGREE; n++)
		{
			rot = newElementRotation(&symmetry->centerOfMass, &pAxis, n);
			testSymmetryElement(symmetry, &rot);				
			if(addElement(symmetry, &rot) && (primaryAxis == NULL || rot.getDegree(&rot) > primaryAxis->getDegree(primaryAxis)))
			{
				copyElement(&rot, &primaryAxis);
			}
		}
		Element ref = newElementReflection(&symmetry->centerOfMass, &pAxis);
		testSymmetryElement(symmetry, &ref);
		addElement(symmetry, &ref);
	}
		
	if(primaryAxis == NULL)
	{
		if(symmetry->elements.size(&symmetry->elements) == 1)
		{
			Element elem = symmetry->elements.get(&symmetry->elements,0);
			if(elem.type==REFLECTION) return "Cs";
		}
		else if(symmetry->elements.size(&symmetry->elements) == 0)
		{
			Element inv = newElementInversion(&symmetry->centerOfMass);
			testSymmetryElement(symmetry, &inv);
			if(addElement(symmetry, &inv)) return "Ci";
		}
	}
	else
	{
		gint i;
		for(i=0; i<symmetry->elements.size(&symmetry->elements); i++)
		{
			Element elem = symmetry->elements.get(&symmetry->elements,i);
			if(elem.type == ROTATION)
			{
				Point3D* axis = elem.getAxis(&elem);
				Point3D* paxis = primaryAxis->getAxis(primaryAxis);
				if(!axis->equals(axis, paxis))
				{
					symmetry->elements.remove(&symmetry->elements,&elem);
					i--;
				}
				else
				{
				}
			}
			else
			{
				symmetry->elements.remove(&symmetry->elements,&elem);
				i--;
			}
		}
			
		perpAxes = 0;
		perpPlanes = 0;
			
		for(i=0; i<nAtoms-1; i++)
		{
			SAtom* from = mol->get(mol,i);
			Point3D fromp = from->getPosition(from);
			Point3D atomAxis = fromp.sub(&fromp, &symmetry->centerOfMass);
			Point3D* paxis = primaryAxis->getAxis(primaryAxis);
			gdouble dotprod = atomAxis.dotProd(&atomAxis, paxis);

	
			if(fabs(dotprod) < GUESS_TOLERANCE)
			{
				rot = newElementRotation(&symmetry->centerOfMass, &atomAxis, 2);
				size = symmetry->elements.size(&symmetry->elements);
				testSymmetryElement(symmetry, &rot);
				if(addElement(symmetry, &rot) && size != symmetry->elements.size(&symmetry->elements)) perpAxes++;
			}
				
			orthogAxis = atomAxis.crossProd(&atomAxis, paxis);
			ref = newElementReflection(&symmetry->centerOfMass, &orthogAxis);
			testSymmetryElement(symmetry, &ref);
			size = symmetry->elements.size(&symmetry->elements);
			if(addElement(symmetry, &ref) && size != symmetry->elements.size(&symmetry->elements)) perpPlanes++;
				
			for(j=i+1; j<nAtoms; j++)
			{
				SAtom* to = mol->get(mol,j);
				if(to->getNumber(to) == from->getNumber(from))
				{
					Point3D midPoint;
					Point3D midAxis;
					Point3D top = to->getPosition(to);
					top = top.sub(&top, &symmetry->centerOfMass);
					midPoint = top.add(&top,&fromp);
					midPoint = midPoint.mult(&midPoint, 0.5);
					midAxis = midPoint.sub(&midPoint, &symmetry->centerOfMass);
					dotprod = midAxis.dotProd(&midAxis, paxis);
					if(fabs(dotprod) < GUESS_TOLERANCE)
					{
						rot = newElementRotation(&symmetry->centerOfMass, &midAxis, 2);
						testSymmetryElement(symmetry, &rot);
						size = symmetry->elements.size(&symmetry->elements);
						if(addElement(symmetry, &rot) && size != symmetry->elements.size(&symmetry->elements)) perpAxes++;
					}
					orthogAxis = midAxis.crossProd(&midAxis, paxis);
					ref = newElementReflection(&symmetry->centerOfMass, &orthogAxis);
					testSymmetryElement(symmetry, &ref);
					size = symmetry->elements.size(&symmetry->elements);
					if(addElement(symmetry, &ref) && size != symmetry->elements.size(&symmetry->elements)) perpPlanes++;
				}
			}
		}	
			
		/*Check for dihedral planes orthogonal to principal axes */
		for(i=0; i<3; i++)
		{
			Point3D* paxis = primaryAxis->getAxis(primaryAxis);
			if(!paxis->equals(paxis, &symmetry->principalAxes[i]))
			{
			Point3D refAxis = paxis->crossProd(paxis, &symmetry->principalAxes[i]);
			ref = newElementReflection(&symmetry->centerOfMass, &refAxis);
			testSymmetryElement(symmetry, &ref);
			size = symmetry->elements.size(&symmetry->elements);
			if(addElement(symmetry, &ref) && size != symmetry->elements.size(&symmetry->elements)) perpPlanes++;
			}
		}
			
		if(perpAxes == primaryAxis->getDegree(primaryAxis)) {
			sprintf(pointGroupGuess,"D%d", primaryAxis->getDegree(primaryAxis));
		}
		else {
			sprintf(pointGroupGuess,"C%d", primaryAxis->getDegree(primaryAxis));
		}
			
		ref = newElementReflection(&symmetry->centerOfMass, primaryAxis->getAxis(primaryAxis));
		testSymmetryElement(symmetry, &ref);
		if(addElement(symmetry, &ref))
		{
			strcat(pointGroupGuess,"h");
			return pointGroupGuess;
		}
			
		if(perpPlanes == primaryAxis->getDegree(primaryAxis))
		{
			if(strstr(pointGroupGuess,"D"))
			{
				strcat(pointGroupGuess,"d");
				return pointGroupGuess;
			}
			else
			{
				strcat(pointGroupGuess,"v");
				return pointGroupGuess;
			}
		}
		else if(perpPlanes == 0)
		{
			gint n;
			for(n=4; n<10; n+=2)
			{
				Element imp = newElementImproperRotation(&symmetry->centerOfMass, primaryAxis->getAxis(primaryAxis), n);
				testSymmetryElement(symmetry,&imp);
				if(addElement(symmetry, &imp))
				{
					sprintf(pointGroupGuess,"S%d",imp.getDegree(&imp));
					return pointGroupGuess;
				}
			}
		}
	}
	
	return pointGroupGuess;
}
/************************************************************************************************************/
static SMolecule getUniqueMolecule(Symmetry* symmetry)
{
	PointGroup finalGroup = symmetry->pointGroups.get(&symmetry->pointGroups,0);
	SMolecule molUnique = finalGroup.getUniqueAtoms(&finalGroup, symmetry->tolerance);
	return molUnique;
}
/************************************************************************************************************/
static SMolecule getSymmetrizeMolecule(Symmetry* symmetry)
{
	PointGroup* finalGroup = symmetry->pointGroups.getPointer(&symmetry->pointGroups,0);
	finalGroup->molecule.print(&finalGroup->molecule);

	SMolecule smol = finalGroup->symmetrizeMolecule(finalGroup, symmetry->tolerance);
	smol.print(&smol);
	return smol;
}
/************************************************************************************************************/
static void freeSymmetry(Symmetry* symmetry)
{
	symmetry->elements.free(&symmetry->elements);
	symmetry->molecule.free(&symmetry->molecule);
	symmetry->rotations.free(&symmetry->rotations);
	symmetry->pointGroups.free(&symmetry->pointGroups);
	
}
/************************************************************************************************************/
static void setMaxDegree(Symmetry* symmetry, gint maxDegree)
{
	symmetry->MAX_DEGREE = maxDegree;
}
/************************************************************************************************************/
static void setMomentTolerance(Symmetry* symmetry, gdouble tolerance)
{
	symmetry->MOMENT_TOLERANCE = tolerance;
}
/************************************************************************************************************/
static gchar* getGroupName(Symmetry* symmetry)
{
	if( symmetry->pointGroups.size(&symmetry->pointGroups)<1) return g_strdup("C1");
	PointGroup finalGroup = symmetry->pointGroups.get(&symmetry->pointGroups,0);
	return g_strdup(finalGroup.getName(&finalGroup));
}
