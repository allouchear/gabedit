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

#ifndef __GABEDIT_MOLECULARMECHANICS_H__
#define __GABEDIT_MOLECULARMECHANICS_H__

typedef struct _AmberParameters  AmberParameters;
typedef struct _AmberAtomTypes  AmberAtomTypes;
typedef struct _AmberBondStretchTerms  AmberBondStretchTerms;
typedef struct _AmberAngleBendTerms  AmberAngleBendTerms;
typedef struct _AmberDihedralAngleTerms  AmberDihedralAngleTerms;
typedef struct _AmberImproperTorsionTerms  AmberImproperTorsionTerms;
typedef struct _AmberNonBondedTerms  AmberNonBondedTerms;
typedef struct _AmberHydrogenBondedTerms  AmberHydrogenBondedTerms;
typedef struct _AmberPairWiseTerms AmberPairWiseTerms;

/************************************/
struct _AmberAtomTypes
{
	gchar* name;
	gchar* symbol;
	gint number;
	gdouble masse;
	gdouble polarisability;
	gchar* description;
};
/************************************/
struct _AmberBondStretchTerms
{
	gint numbers[2];
	gdouble equilibriumDistance;
	gdouble forceConstant;
};
/************************************/
struct _AmberAngleBendTerms
{
	gint numbers[3];
	gdouble equilibriumAngle;
	gdouble forceConstant;
};
/************************************/
struct _AmberDihedralAngleTerms
{
	gint numbers[4];
	gint nSomme;
	gdouble* divisor;
	gdouble* barrier;
	gdouble* phase;
	gdouble* n;
};
/************************************/
struct _AmberImproperTorsionTerms
{
	gint numbers[4];
	gdouble barrier;
	gdouble phase;
	gdouble n;
};
/************************************/
struct _AmberNonBondedTerms
{
	gint number;
	gdouble r;
	gdouble epsilon;
};
/************************************/
struct _AmberHydrogenBondedTerms
{
	gint numbers[2];
	gdouble c;
	gdouble d;
};
/************************************/
struct _AmberPairWiseTerms
{
	gint numbers[2];
	gdouble a;
	gdouble beta;
	gdouble c6;
	gdouble c8;
	gdouble c10;
	gdouble b;
};
/************************************/
struct _AmberParameters
{
	gint numberOfTypes;
	AmberAtomTypes* atomTypes;

	gint numberOfStretchTerms;
	AmberBondStretchTerms* bondStretchTerms;

	gint numberOfBendTerms;
	AmberAngleBendTerms* angleBendTerms;

	gint numberOfDihedralTerms;
	AmberDihedralAngleTerms* dihedralAngleTerms;

	gint numberOfImproperTorsionTerms;
	AmberImproperTorsionTerms* improperTorsionTerms;

	gint numberOfNonBonded;
	AmberNonBondedTerms* nonBondedTerms;

	gint numberOfHydrogenBonded;
	AmberHydrogenBondedTerms* hydrogenBondedTerms;

	gint numberOfPairWise;
	AmberPairWiseTerms* pairWiseTerms;
};
/************************************/
ForceField createAmberModel(GeomDef* geom,gint Natoms,ForceFieldOptions forceFieldOptions);
ForceField createPairWiseModel(GeomDef* geom,gint Natoms,ForceFieldOptions forceFieldOptions);
void loadAmberParameters();
void saveAmberParameters();
AmberParameters* getPointerAmberParameters();
void setPointerAmberParameters(AmberParameters* ptr);
AmberParameters newAmberParameters();
gchar** getListMMTypes(gint* nlist);

#endif /* __GABEDIT_MOLECULARMECHANICS_H__ */

