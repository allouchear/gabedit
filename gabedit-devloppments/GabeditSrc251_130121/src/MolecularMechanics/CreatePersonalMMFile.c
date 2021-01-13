/* CreatePersonalMMFile.c */
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
#include <stdio.h>
#include <math.h>

#include "../Common/Global.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Utils.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "Atom.h"
#include "Molecule.h"
#include "ForceField.h"
#include "MolecularMechanics.h"

static gchar atomTypesTitle[]       = "Begin  INPUT FOR ATOM TYPES, MASSE AND POLARISABILITIES";
static gchar bondStretchTitle[]     = "Begin INPUT FOR BOND LENGTH PARAMETERS";
static gchar angleBendTitle[]       = "Begin INPUT FOR BOND ANGLE PARAMETERS";
static gchar hydrogenBondedTitle[]  = "Begin INPUT FOR H-BOND 10-12 POTENTIAL PARAMETERS";
static gchar improperTorsionTitle[] ="Begin INPUT FOR IMPROPER DIHEDRAL PARAMETERS";
static gchar nonBondedTitle[]       ="Begin INPUT FOR THE NON-BONDED 6-12 POTENTIAL PARAMETERS";
static gchar dihedralAngleTitle[]   = "Begin INPUT FOR DIHEDRAL PARAMETERS";
static gchar pairWiseTitle[]        = "Begin INPUT FOR PAIR WISE PARAMETERS";

/**********************************************************************/
static void saveParametersPairWise(FILE* file,AmberParameters* amberParameters)
{
	gint n;
	fprintf(file,"%s\n",pairWiseTitle);

	for(n=0;n<amberParameters->numberOfPairWise;n++)
	{
		fprintf(file,"%d\t%d\t%8.4f\t%8.4f\t%8.4f\t%8.4f\t%8.4f\t%8.4f\n",
				amberParameters->pairWiseTerms[n].numbers[0]+1,
				amberParameters->pairWiseTerms[n].numbers[1]+1,
				amberParameters->pairWiseTerms[n].a,
				amberParameters->pairWiseTerms[n].beta,
				amberParameters->pairWiseTerms[n].c6,
				amberParameters->pairWiseTerms[n].c8,
				amberParameters->pairWiseTerms[n].c10,
				amberParameters->pairWiseTerms[n].b
				);
	}
	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveParametersBond(FILE* file,AmberParameters* amberParameters)
{
	gint i;
	fprintf(file,"%s\n",bondStretchTitle);

	for(i=0;i<amberParameters->numberOfStretchTerms;i++)
	{
		fprintf(file,"%d\t%d\t%6.1f\t%7.4f\n",
			amberParameters->bondStretchTerms[i].numbers[0]+1,
			amberParameters->bondStretchTerms[i].numbers[1]+1,
			amberParameters->bondStretchTerms[i].forceConstant,
			amberParameters->bondStretchTerms[i].equilibriumDistance
			);
	}
	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveParametersBend(FILE* file,AmberParameters* amberParameters)
{
	gint n;
	fprintf(file,"%s\n",angleBendTitle);

	for(n=0;n<amberParameters->numberOfBendTerms;n++)
	{
		fprintf(file,"%d\t%d\t%d\t%6.3f\t%6.2f\n",
				amberParameters->angleBendTerms[n].numbers[0]+1,
				amberParameters->angleBendTerms[n].numbers[1]+1,
				amberParameters->angleBendTerms[n].numbers[2]+1,
				amberParameters->angleBendTerms[n].forceConstant,
				amberParameters->angleBendTerms[n].equilibriumAngle
				);
	}

	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveParametersHydrogenBonded(FILE* file,AmberParameters* amberParameters)
{
	gint n;
	fprintf(file,"%s\n",hydrogenBondedTitle);
	for(n=0;n<amberParameters->numberOfHydrogenBonded;n++)
	{
			fprintf(file,"%d\t%d\t%8.1f\t%8.1f\n",
				amberParameters->hydrogenBondedTerms[n].numbers[0]+1,
				amberParameters->hydrogenBondedTerms[n].numbers[1]+1,
				amberParameters->hydrogenBondedTerms[n].c,
				amberParameters->hydrogenBondedTerms[n].d
				);
	}

	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveParametersimproperTorsion(FILE* file,AmberParameters* amberParameters)
{
	gint n;
	fprintf(file,"%s\n",improperTorsionTitle);
	for(n=0;n<amberParameters->numberOfImproperTorsionTerms;n++)
	{
		fprintf(file,"%d\t%d\t%d\t%d\t%6.3f\t%6.2f\t%4.1f\n",
				amberParameters->improperTorsionTerms[n].numbers[0]+1,
				amberParameters->improperTorsionTerms[n].numbers[1]+1,
				amberParameters->improperTorsionTerms[n].numbers[2]+1,
				amberParameters->improperTorsionTerms[n].numbers[3]+1,
				amberParameters->improperTorsionTerms[n].barrier,
				amberParameters->improperTorsionTerms[n].phase,
				amberParameters->improperTorsionTerms[n].n
				);
	}

	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveParametersNonBonded(FILE* file,AmberParameters* amberParameters)
{
	gint n;
	fprintf(file,"%s\n",nonBondedTitle);

	for(n=0;n<amberParameters->numberOfNonBonded;n++)
	{
		fprintf(file,"%d\t%8.4f\t%8.4f\n",
				amberParameters->nonBondedTerms[n].number+1,
				amberParameters->nonBondedTerms[n].r,
				amberParameters->nonBondedTerms[n].epsilon
				);
	}


	
		fprintf(file,"End\n");
}


/**********************************************************************/
static void saveParametersDihedralAngle(FILE* file,AmberParameters* amberParameters)
{
	gint n;
	gdouble nn;

	fprintf(file,"%s\n",dihedralAngleTitle);
	for(n=0;n<amberParameters->numberOfDihedralTerms;n++)
	{
		gint j;
		for(j=0;j<amberParameters->dihedralAngleTerms[n].nSomme;j++)
		{
			if(j==amberParameters->dihedralAngleTerms[n].nSomme-1)
				nn = amberParameters->dihedralAngleTerms[n].n[j];
			else
				nn = -amberParameters->dihedralAngleTerms[n].n[j];
		
			fprintf(file,"%d\t%d\t%d\t%d\t%4.1f\t%6.3f\t%6.2f\t%4.1f\n",
				amberParameters->dihedralAngleTerms[n].numbers[0]+1,
				amberParameters->dihedralAngleTerms[n].numbers[1]+1,
				amberParameters->dihedralAngleTerms[n].numbers[2]+1,
				amberParameters->dihedralAngleTerms[n].numbers[3]+1,
				amberParameters->dihedralAngleTerms[n].divisor[j],
				amberParameters->dihedralAngleTerms[n].barrier[j],
				amberParameters->dihedralAngleTerms[n].phase[j],
				nn

				);
		}
	}
	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveParametersAtomTypes(FILE* file,AmberParameters* amberParameters)
{
	gint i;
	fprintf(file,"%s\n",atomTypesTitle);

	for(i=0;i<amberParameters->numberOfTypes;i++)
	{
		fprintf(file,"%s\t%s\t%d\t%6.3f\t%6.3f\t%s\n",
			amberParameters->atomTypes[i].name,
			amberParameters->atomTypes[i].symbol,
			amberParameters->atomTypes[i].number+1,
			amberParameters->atomTypes[i].masse,
			amberParameters->atomTypes[i].polarisability,
			amberParameters->atomTypes[i].description
			);
	}

	fprintf(file,"End\n");
}

/**********************************************************************/
static void saveParametersTitle(FILE* file)
{
	fprintf(file,"Begin Title\n");
	fprintf(file,"\tAtom Types         : Ty(Type) Symbol Numero Masse(C12 UMA) Polarisablities(Ang**3) \n");
	fprintf(file,"\tBond Length        : N1-N2 Force(Kcal/mol/A**2) Re\n");
	fprintf(file,"\tBond Angle         : N1-N2-N3 Force(Kcal/mol/rad**2) Angle(Deg) \n");
	fprintf(file,"\tDihedral           : N1-N2-N3-N4 Idiv Pk Phase(Deg) Pn \n");
	fprintf(file,"\t                     E = Pk/Idiv*(1 + cos(P,*Phi - Phase)\n");
	fprintf(file,"\t                     Pk = Barrier/2 Kcal/mol\n");
	fprintf(file,"\t                     Idiv barrier is divised by Idiv\n");
	fprintf(file,"\t                     Pn = periodicity fo the torional barrier\n");
	fprintf(file,"\t                     if Pn<0 the tosional potential is \n");
	fprintf(file,"\t                     assumed to have more than one term\n");
	fprintf(file,"\t                     if Ni=0 => N is a number for any one Type\n");
	fprintf(file,"\tImproper Dihedral  : N1-N2-N3-N4 Idiv Pk Phase(Deg) Pn \n");
	fprintf(file,"\tH-Bond             : N1-N2   A(coef. 1/r**12)  B(coef. -B/r**10)\n");
	fprintf(file,"\tPair Wise          :N1-N2  A(kcal/mol)  B(Ang-1) C6(kcal*Ang**6) C8 C10 b(Ang-1)\n");
	fprintf(file,"\t                   V(r) = A*exp(-B*r) -Somme(C2n*f2n/r**(2*n))[n=3,4,5]\n");
	fprintf(file,"\t                   f2n = 1- exp(-b*r)*Somme((b*r)**k/k!,k=1..2*n)\n");
	fprintf(file,"End\n");
}
/**********************************************************************/
static void saveParameters(AmberParameters* amberParameters,gchar* filename)
{
	FILE* file;
	file = FOpen(filename,"w");
	if(!file)
	{
		printf("I can not open \"%s\" file\n",filename);
		return;
	}
	saveParametersTitle(file);
	saveParametersAtomTypes(file,amberParameters);
	saveParametersBond(file,amberParameters);
	saveParametersBend(file,amberParameters);
	saveParametersDihedralAngle(file,amberParameters);
	saveParametersimproperTorsion(file,amberParameters);
	saveParametersHydrogenBonded(file,amberParameters);
	saveParametersNonBonded(file,amberParameters);
	saveParametersPairWise(file,amberParameters);
	fclose(file);
}
/**********************************************************************/
void createPersonalParametersFile(AmberParameters* amberParameters)
{
	gchar* filename = g_strdup_printf("%s%sPersonalMM.prm",gabedit_directory(), G_DIR_SEPARATOR_S);
	saveParameters(amberParameters,filename);
	g_free(filename);
}
