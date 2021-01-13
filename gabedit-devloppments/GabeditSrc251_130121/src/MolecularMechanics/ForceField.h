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

#ifndef __GABEDIT_FORCEFIELD_H__
#define __GABEDIT_FORCEFIELD_H__

#define STRETCHDIM 		4 /* a1 a2 Force Re */
#define BENDDIM 		5 /* a1 a2 a3 Force Theta0 */
#define DIHEDRALDIM 		8 /*a1 a2 a3 a4 Idiv Pk Phase Pn */
#define IMPROPERDIHEDRALDIM	7 /*a1 a2 a3 a4 Pk Phase Pn */
#define NONBONDEDDIM 		5 /* a1 a2 Aij Bij CoulombFactor */
#define HYDROGENBONDEDDIM 	4 /* a1 a2 Cij Dij */
#define RATTLEDIM	        3 /* a1 a2 r12 */

#define PAIRWISEDIM 	8 /* a1 a2 A Beta C6 C8 C10 b  : 
			     potential = A*exp(-Beta*r)-Somme C2n*f2n/r**(2*n) + Zi*Zj/r 
			     f2n = 1 - exp(-b*r)*somme((b*r)**k/k!,k=0..2*n)
			   */

typedef struct _ForceField  ForceField;
typedef struct _ForceFieldClass  ForceFieldClass;
typedef struct _ForceFieldOptions  ForceFieldOptions;

typedef enum
{
  AMBER,
  UFF,
  PAIRWISE
} ForceFieldTypes;

typedef enum
{
  NOCONSTRAINTS = 0,
  BONDSCONSTRAINTS = 1,
  BONDSANGLESCONSTRAINTS = 2
} ForceFieldConstraints;

struct _ForceFieldOptions
{
	ForceFieldTypes type;
	
	gboolean bondStretch;/* For Amber */
	gboolean angleBend;/* For Amber */
	gboolean dihedralAngle;/* For Amber */
	gboolean improperTorsion;/* For Amber */
	gboolean nonBonded;/* For Amber */
	gboolean hydrogenBonded;/* For Amber */
	gboolean coulomb; /* For Amber and Pair-Wise */
	gboolean vanderWals; /* For Ionic */
	ForceFieldConstraints rattleConstraints;/*  rattle constraints */
};
struct _ForceField
{
	Molecule molecule;
	ForceFieldClass* klass;
	gint numberOfStretchTerms;
	gint numberOfBendTerms;
	gint numberOfDihedralTerms;
	gint numberOfImproperTorsionTerms;
	gint numberOfNonBonded;
	gint numberOfHydrogenBonded;
	gint numberOfRattleConstraintsTerms;

	gdouble* bondStretchTerms[STRETCHDIM];
	gdouble* angleBendTerms[BENDDIM];
	gdouble* dihedralAngleTerms[DIHEDRALDIM];
	gdouble* improperTorsionTerms[IMPROPERDIHEDRALDIM];
	gdouble* nonBondedTerms[NONBONDEDDIM];
	gdouble* hydrogenBondedTerms[HYDROGENBONDEDDIM];

	gint numberOfPairWise;
	gdouble* pairWiseTerms[PAIRWISEDIM];

	gdouble* rattleConstraintsTerms[RATTLEDIM];

	ForceFieldOptions options;
};
struct _ForceFieldClass
{
	void (*calculateGradient)(ForceField* forceField);
	void (*calculateEnergy)(ForceField* forceField);
	gdouble (*calculateEnergyTmp)(ForceField* forceField,Molecule* m);
};


ForceField newForceField();
void freeForceField(ForceField* forceField);
ForceField copyForceField(ForceField* forceField);

#endif /* __GABEDIT_FORCEFIELD_H__ */

