/* ForceField.c */
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
#include <stdlib.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "Atom.h"
#include "Molecule.h"
#include "ForceField.h"
void create_GeomXYZ_from_draw_grometry();

/**********************************************************************/
ForceField newForceField()
{
	gint i;
	ForceField forceField;

	forceField.molecule = newMolecule();

	forceField.klass = g_malloc(sizeof(ForceFieldClass));
	forceField.klass->calculateGradient = NULL;
	forceField.klass->calculateEnergy = NULL;
	forceField.klass->calculateEnergyTmp = NULL;


	forceField.numberOfRattleConstraintsTerms = 0;
	forceField.numberOfStretchTerms = 0;
	forceField.numberOfBendTerms = 0;
	forceField.numberOfDihedralTerms = 0;
	forceField.numberOfImproperTorsionTerms = 0;
	forceField.numberOfNonBonded = 0;
	forceField.numberOfHydrogenBonded = 0;

	for(i=0;i<STRETCHDIM;i++)
		forceField.bondStretchTerms[i] = NULL;
	for(i=0;i<BENDDIM;i++)
		forceField.angleBendTerms[i] = NULL;
	for(i=0;i<DIHEDRALDIM;i++)
		forceField.dihedralAngleTerms[i] = NULL;
	for(i=0;i<IMPROPERDIHEDRALDIM;i++)
		forceField.improperTorsionTerms[i] = NULL;
	for(i=0;i<NONBONDEDDIM;i++)
		forceField.nonBondedTerms[i] = NULL;
	for(i=0;i<HYDROGENBONDEDDIM;i++)
		forceField.hydrogenBondedTerms[i] = NULL;

	for(i=0;i<RATTLEDIM;i++)
		forceField.rattleConstraintsTerms[i] = NULL;

	forceField.numberOfPairWise = 0;
	for(i=0;i<PAIRWISEDIM;i++)
		forceField.pairWiseTerms[i] = NULL;

	forceField.options.type = AMBER;
	forceField.options.coulomb = TRUE;
	forceField.options.hydrogenBonded = TRUE;
	forceField.options.improperTorsion = TRUE;
	forceField.options.vanderWals = TRUE;
	forceField.options.rattleConstraints = NOCONSTRAINTS;
	return forceField;

}
/**********************************************************************/
void freeForceField(ForceField* forceField)
{

	gint i;
	freeMolecule(&forceField->molecule);

	if(forceField->klass != NULL)
	{
		g_free(forceField->klass);
		forceField->klass = NULL;
	}
	for(i=0;i<STRETCHDIM;i++)
		if(forceField->bondStretchTerms[i] !=NULL)
		{
			g_free(forceField->bondStretchTerms[i]);
			forceField->bondStretchTerms[i] = NULL;
		}
	for(i=0;i<BENDDIM;i++)
		if(forceField->angleBendTerms[i] != NULL)
		{
			g_free(forceField->angleBendTerms[i]);
			forceField->angleBendTerms[i] = NULL;
		}
	for(i=0;i<DIHEDRALDIM;i++)
		if(forceField->dihedralAngleTerms[i] != NULL)
		{
			g_free(forceField->dihedralAngleTerms[i]);
			forceField->dihedralAngleTerms[i] = NULL;
		}
	for(i=0;i<IMPROPERDIHEDRALDIM;i++)
		if(forceField->improperTorsionTerms[i] != NULL)
		{
			g_free(forceField->improperTorsionTerms[i]);
			forceField->improperTorsionTerms[i] = NULL;
		}
	for(i=0;i<NONBONDEDDIM;i++)
		if(forceField->nonBondedTerms[i] != NULL)
		{
			g_free(forceField->nonBondedTerms[i]);
			forceField->nonBondedTerms[i] = NULL;
		}
	for(i=0;i<HYDROGENBONDEDDIM;i++)
		if(forceField->hydrogenBondedTerms[i] != NULL)
		{
			g_free(forceField->hydrogenBondedTerms[i]);
			forceField->hydrogenBondedTerms[i] = NULL;
		}
	for(i=0;i<RATTLEDIM;i++)
		if(forceField->rattleConstraintsTerms[i] != NULL)
		{
			g_free(forceField->rattleConstraintsTerms[i]);
			forceField->rattleConstraintsTerms[i] = NULL;
		}

	forceField->numberOfStretchTerms = 0;
	forceField->numberOfBendTerms = 0;
	forceField->numberOfDihedralTerms = 0;
	forceField->numberOfImproperTorsionTerms = 0;
	forceField->numberOfNonBonded = 0;
	forceField->numberOfHydrogenBonded = 0;
	forceField->numberOfRattleConstraintsTerms = 0;

	for(i=0;i<PAIRWISEDIM;i++)
		if(forceField->pairWiseTerms[i] != NULL)
		{
			g_free(forceField->pairWiseTerms[i]);
			forceField->pairWiseTerms[i] = NULL;
		}
	forceField->numberOfPairWise = 0;
}
/*****************************************************************************/
ForceField copyForceField(ForceField* f)
{
	gint i;
	gint j;
	gint k;
	ForceField forceField = newForceField();

	forceField.molecule = copyMolecule(&f->molecule);

	/* already in newForceField */
	/* forceField.klass = g_malloc(sizeof(ForceFieldClass));*/
	forceField.klass->calculateGradient = f->klass->calculateGradient;
	forceField.klass->calculateEnergy = f->klass->calculateEnergy;
	forceField.klass->calculateEnergyTmp = f->klass->calculateEnergyTmp;


	forceField.numberOfStretchTerms = f->numberOfStretchTerms;
	forceField.numberOfBendTerms = f->numberOfBendTerms;
	forceField.numberOfDihedralTerms = f->numberOfDihedralTerms;
	forceField.numberOfImproperTorsionTerms = f->numberOfImproperTorsionTerms;
	forceField.numberOfNonBonded = f->numberOfNonBonded;
	forceField.numberOfHydrogenBonded = f->numberOfHydrogenBonded;

	k = forceField.numberOfStretchTerms;
	if(k>0)
	for(i=0;i<STRETCHDIM;i++)
	{
		forceField.bondStretchTerms[i] = g_malloc(k*sizeof(gdouble));
		for(j=0;j<k;j++) forceField.bondStretchTerms[i][j] = f->bondStretchTerms[i][j];
	}

	k = forceField.numberOfBendTerms;
	if(k>0)
	for(i=0;i<BENDDIM;i++)
	{
		forceField.angleBendTerms[i] = g_malloc(k*sizeof(gdouble));
		for(j=0;j<k;j++) forceField.angleBendTerms[i][j] = f->angleBendTerms[i][j];
	}


	k = forceField.numberOfDihedralTerms;
	if(k>0)
	for(i=0;i<DIHEDRALDIM;i++)
	{
		forceField.dihedralAngleTerms[i] = g_malloc(k*sizeof(gdouble));
		for(j=0;j<k;j++) forceField.dihedralAngleTerms[i][j] = f->dihedralAngleTerms[i][j];
	}

	k = forceField.numberOfImproperTorsionTerms;
	if(k>0)
	for(i=0;i<IMPROPERDIHEDRALDIM;i++)
	{
		forceField.improperTorsionTerms[i] = g_malloc(k*sizeof(gdouble));
		for(j=0;j<k;j++) forceField.improperTorsionTerms[i][j] = f->improperTorsionTerms[i][j];
	}

	k = forceField.numberOfNonBonded;
	if(k>0)
	for(i=0;i<NONBONDEDDIM;i++)
	{
		forceField.nonBondedTerms[i] = g_malloc(k*sizeof(gdouble));
		for(j=0;j<k;j++) forceField.nonBondedTerms[i][j] = f->nonBondedTerms[i][j];
	}

	k = forceField.numberOfHydrogenBonded;
	if(k>0)
	for(i=0;i<HYDROGENBONDEDDIM;i++)
	{
		forceField.hydrogenBondedTerms[i] = g_malloc(k*sizeof(gdouble));
		for(j=0;j<k;j++) forceField.hydrogenBondedTerms[i][j] = f->hydrogenBondedTerms[i][j];
	}

	k = forceField.numberOfRattleConstraintsTerms;
	if(k>0)
	for(i=0;i<RATTLEDIM;i++)
	{
		forceField.rattleConstraintsTerms[i] = g_malloc(k*sizeof(gdouble));
		for(j=0;j<k;j++) forceField.rattleConstraintsTerms[i][j] = f->rattleConstraintsTerms[i][j];
	}

	k = forceField.numberOfPairWise = f->numberOfPairWise;
	if(k>0)
	for(i=0;i<PAIRWISEDIM;i++)
	{
		forceField.pairWiseTerms[i] = g_malloc(k*sizeof(gdouble));
		for(j=0;j<k;j++) forceField.pairWiseTerms[i][j] = f->pairWiseTerms[i][j];
	}

	forceField.options.type = f->options.type;
	forceField.options.coulomb = f->options.coulomb;
	forceField.options.hydrogenBonded = f->options.hydrogenBonded;
	forceField.options.improperTorsion = f->options.improperTorsion;
	forceField.options.vanderWals = f->options.vanderWals;
	forceField.options.bondStretch = f->options.bondStretch;
	forceField.options.angleBend = f->options.angleBend;
	forceField.options.dihedralAngle = f->options.dihedralAngle;
	forceField.options.nonBonded = f->options.nonBonded;
	forceField.options.rattleConstraints = f->options.rattleConstraints;

	return forceField;

}
