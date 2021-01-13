/* MolecularMechanics.c */
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
#ifdef ENABLE_OMP
#include <omp.h>
#endif

#include "../Common/Global.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "Atom.h"
#include "Molecule.h"
#include "ForceField.h"
#include "MolecularMechanics.h"
#include "LoadMMParameters.h"
#include "CreatePersonalMMFile.h"
#include "CreateMolecularMechanicsFile.h"

static AmberParameters* staticAmberParameters = NULL;


/* static void calculateGradientNumericAmber(ForceField* forceField);*/
static void calculateGradientAmber(ForceField* forceField);
static void calculateEnergyAmber(ForceField* forceField);
static gdouble calculateEnergyTmpAmber(ForceField* forceField,Molecule* m);

/**********************************************************************/
AmberParameters newAmberParameters()
{
	AmberParameters amberParameters;

	amberParameters.numberOfTypes = 0;
	amberParameters.atomTypes = NULL;

	amberParameters.numberOfStretchTerms = 0;
	amberParameters.bondStretchTerms = NULL;

	amberParameters.numberOfBendTerms = 0;
	amberParameters.angleBendTerms = NULL;

	amberParameters.numberOfDihedralTerms = 0;
	amberParameters.dihedralAngleTerms = NULL;

	amberParameters.numberOfImproperTorsionTerms = 0;
	amberParameters.improperTorsionTerms = NULL;

	amberParameters.numberOfNonBonded = 0;
	amberParameters.nonBondedTerms = NULL;

	amberParameters.numberOfHydrogenBonded = 0;
	amberParameters.hydrogenBondedTerms = NULL;

	amberParameters.numberOfPairWise = 0;
	amberParameters.pairWiseTerms = NULL;


	return amberParameters;
	
}
/**********************************************************************/
static void freeAmberParameters(AmberParameters* amberParameters)
{
	gint i;

	for(i=0;i<amberParameters->numberOfTypes;i++)
		if(amberParameters->atomTypes[i].name)
			g_free(amberParameters->atomTypes[i].name);

	for(i=0;i<amberParameters->numberOfTypes;i++)
		if(amberParameters->atomTypes[i].name)
			g_free(amberParameters->atomTypes[i].description);

	amberParameters->numberOfTypes = 0;
	if(amberParameters->atomTypes )
		g_free(amberParameters->atomTypes );
	amberParameters->atomTypes = NULL;

	amberParameters->numberOfStretchTerms = 0;
	if(amberParameters->bondStretchTerms)
		g_free(amberParameters->bondStretchTerms);
	amberParameters->bondStretchTerms = NULL;

	amberParameters->numberOfBendTerms = 0;
	if(amberParameters->angleBendTerms)
		g_free(amberParameters->angleBendTerms);
	amberParameters->angleBendTerms = NULL;

	for(i=0;i<amberParameters->numberOfDihedralTerms;i++)
	{
		if(amberParameters->dihedralAngleTerms[i].divisor)
			g_free(amberParameters->dihedralAngleTerms[i].divisor);
		if(amberParameters->dihedralAngleTerms[i].barrier)
			g_free(amberParameters->dihedralAngleTerms[i].barrier);
		if(amberParameters->dihedralAngleTerms[i].phase)
			g_free(amberParameters->dihedralAngleTerms[i].phase);
		if(amberParameters->dihedralAngleTerms[i].n)
			g_free(amberParameters->dihedralAngleTerms[i].n);

	}

	amberParameters->numberOfDihedralTerms = 0;
	if(amberParameters->dihedralAngleTerms)
		g_free(amberParameters->dihedralAngleTerms);
	amberParameters->dihedralAngleTerms = NULL;

	amberParameters->numberOfImproperTorsionTerms = 0;
	if(amberParameters->improperTorsionTerms)
		g_free(amberParameters->improperTorsionTerms);
	amberParameters->improperTorsionTerms = NULL;

	amberParameters->numberOfNonBonded = 0;
	if(amberParameters->nonBondedTerms)
		g_free(amberParameters->nonBondedTerms);
	amberParameters->nonBondedTerms = NULL;

	amberParameters->numberOfHydrogenBonded = 0;
	if(amberParameters->hydrogenBondedTerms)
		g_free(amberParameters->hydrogenBondedTerms);
	amberParameters->hydrogenBondedTerms = NULL;
}
/**********************************************************************/
static gint getNumberType(AmberParameters* amberParameters, gchar* type)
{
	gint i;
	gint nTypes = amberParameters->numberOfTypes;
	AmberAtomTypes* types = amberParameters->atomTypes;
	gint len = strlen(type);

	if(strcmp(type,"X")==0)
		return -1;
	for(i=0;i<nTypes;i++)
	{
		if(len == (gint)strlen(types[i].name) && strstr(types[i].name,type))
			return types[i].number;

	}
	return -2;
}
/**********************************************************************/
static ForceField newAmberModel()
{
	ForceField forceField = newForceField();

	forceField.klass->calculateGradient = calculateGradientAmber;
	/*forceField.klass->calculateGradient = calculateGradientNumericAmber;*/
	forceField.klass->calculateEnergy = calculateEnergyAmber;
	forceField.klass->calculateEnergyTmp = calculateEnergyTmpAmber;

	forceField.options.type = AMBER;
	forceField.options.coulomb = TRUE;
	forceField.options.hydrogenBonded = TRUE;
	forceField.options.improperTorsion = TRUE;

	return forceField;

}
/**********************************************************************/
static ForceField newPairWiseModel()
{
	ForceField forceField = newForceField();

	forceField.klass->calculateGradient = calculateGradientAmber;
	forceField.klass->calculateEnergy = calculateEnergyAmber;
	forceField.klass->calculateEnergyTmp = calculateEnergyTmpAmber;

	forceField.options.type = PAIRWISE;

	forceField.options.coulomb = TRUE;
	forceField.options.vanderWals = TRUE;

	forceField.options.bondStretch = FALSE;
	forceField.options.angleBend = FALSE;
	forceField.options.dihedralAngle = FALSE;
	forceField.options.improperTorsion = FALSE;
	forceField.options.nonBonded = FALSE;
	forceField.options.hydrogenBonded = FALSE;


	return forceField;

}
/**********************************************************************/
static gboolean isIonic(gchar* mmType)
{
	if(!strcmp(mmType,"Li")) return TRUE;
	if(!strcmp(mmType,"Na")) return TRUE;
	if(!strcmp(mmType,"K")) return TRUE;
	if(!strcmp(mmType,"Rb")) return TRUE;
	if(!strcmp(mmType,"Cs")) return TRUE;
	if(!strcmp(mmType,"Ca")) return TRUE;
	if(!strcmp(mmType,"Sr")) return TRUE;
	if(!strcmp(mmType,"Ba")) return TRUE;
	if(!strcmp(mmType,"Zn")) return TRUE;
	if(!strcmp(mmType,"IB")) return TRUE;
	if(!strcmp(mmType,"Cl")) return TRUE;
	return FALSE;
}
/**********************************************************************/
static gboolean getStretchParameters(	AmberParameters* amberParameters,
								gint a1Type, gint a2Type, 
								gdouble* forceConstant,gdouble* equilibriumDistance)
{
	gint i;
	forceConstant[0] = 0.0;
	equilibriumDistance[0] = 0.0;

	if(a1Type>a2Type)
	{
		gint t;
		t = a1Type;
		a1Type = a2Type;
		a2Type = t;
	}

	for(i=0;i<amberParameters->numberOfStretchTerms;i++)
	{
		if(
			a1Type == amberParameters->bondStretchTerms[i].numbers[0]  &&
			a2Type == amberParameters->bondStretchTerms[i].numbers[1] 
		)
		{
			forceConstant[0]       = amberParameters->bondStretchTerms[i].forceConstant;
			equilibriumDistance[0] = amberParameters->bondStretchTerms[i].equilibriumDistance;
			return TRUE;
		}
	}
	return FALSE;
}
/**********************************************************************/
static gboolean getBendParameters(AmberParameters* amberParameters,gint a1Type, gint a2Type, gint a3Type,
	       	gdouble* forceConstant, gdouble* equilibriumAngle)
{
	gint i;
	forceConstant[0] = 0.0;
	equilibriumAngle[0] = 0.0;

	if(a1Type>a3Type)
	{
		gint t;
		t = a1Type;
		a1Type = a3Type;
		a3Type = t;
	}

	for(i=0;i<amberParameters->numberOfBendTerms;i++)
	{
		if(
			a1Type == amberParameters->angleBendTerms[i].numbers[0]  &&
			a2Type == amberParameters->angleBendTerms[i].numbers[1]  &&
			a3Type == amberParameters->angleBendTerms[i].numbers[2] 
		)
		{
			forceConstant[0]       = amberParameters->angleBendTerms[i].forceConstant;
			equilibriumAngle[0]    = amberParameters->angleBendTerms[i].equilibriumAngle;
			return TRUE;
		}
	}
	return FALSE;
}
/**********************************************************************/
static gboolean getHydrogenBondedParameters(AmberParameters* amberParameters, gint a1Type, gint a2Type, gdouble c[], gdouble d[] )
{
	gint i;
	AmberAtomTypes* types = amberParameters->atomTypes;


	c[0] = 0.0;
	d[0] = 0.0;

	if(types[a2Type].name[0]=='H')
	{
		gint t = a1Type;
		a1Type = a2Type;
		a2Type = t;
	}

	for(i=0;i<amberParameters->numberOfHydrogenBonded;i++)
	{
		if(
			a1Type == amberParameters->hydrogenBondedTerms[i].numbers[0] &&
			a2Type == amberParameters->hydrogenBondedTerms[i].numbers[1]
		  )
		{
			c[0]    = amberParameters->hydrogenBondedTerms[i].c;
			d[0]    = amberParameters->hydrogenBondedTerms[i].d;
			return TRUE;
		}
	}
	return FALSE;
}
/**********************************************************************/
static gboolean getNonBondedParameters(AmberParameters* amberParameters, gint atomType, gdouble* r, gdouble* epsilon )
{

	gint i;
	r[0] = 1.0;
	epsilon[0] = 0.0;
	
	for(i=0;i<amberParameters->numberOfNonBonded;i++)
	{
		if(
			atomType == amberParameters->nonBondedTerms[i].number
		  )
		{
			r[0]       = amberParameters->nonBondedTerms[i].r;
			epsilon[0]    = amberParameters->nonBondedTerms[i].epsilon;
			/*printf("r = %f eps = %f\n",r[0],epsilon[0]);*/
			return TRUE;
		}
	}

	return FALSE;
}
/**********************************************************************/
static gboolean getPairWiseParameters(AmberParameters* amberParameters,
	       	gint a1Type, gint a2Type,
		gdouble* a, gdouble* beta,
	       	gdouble* c6, gdouble* c8, gdouble* c10, gdouble* b)
{

	gint i;

	a[0]    = 0.0;
	beta[0] = 1.0;
	c6[0]   = 0.0;
	c8[0]   = 0.0;
	c10[0]   = 0.0;
	b[0]    = 1.0;
	for(i=0;i<amberParameters->numberOfPairWise;i++)
	{
		if(
			(
			a1Type == amberParameters->pairWiseTerms[i].numbers[0] &&
			a2Type == amberParameters->pairWiseTerms[i].numbers[1] 
			) ||
			(
			a1Type == amberParameters->pairWiseTerms[i].numbers[1] &&
			a2Type == amberParameters->pairWiseTerms[i].numbers[0]
			)

		  )
		{
			a[0]    = amberParameters->pairWiseTerms[i].a;
			beta[0]    = amberParameters->pairWiseTerms[i].beta;
			c6[0]    = amberParameters->pairWiseTerms[i].c6;
			c8[0]    = amberParameters->pairWiseTerms[i].c8;
			c10[0]    = amberParameters->pairWiseTerms[i].c10;
			b[0]    = amberParameters->pairWiseTerms[i].b;
			return TRUE;
		}
	}

	return FALSE;
}
/**********************************************************************/
static gboolean getImproperTorsionParameters( AmberParameters* amberParameters,
		gint a1Type, gint a2Type, gint a3Type, gint a4Type,
	       	gdouble* forceConstant, gdouble* equilibriumAngle, gdouble* terms
		)
{
	gint i;
	forceConstant[0] = 0.0;
	equilibriumAngle[0] = 0.0;
	terms[0] = 0.0;

	if(a1Type>a4Type)
	{
		gint t;
		t = a1Type;
		a1Type = a4Type;
		a4Type = t;
		
		t = a2Type;
		a2Type = a3Type;
		a3Type = t;

	}


	for(i=0;i<amberParameters->numberOfImproperTorsionTerms;i++)
	{
		if(
			a1Type == amberParameters->improperTorsionTerms[i].numbers[0] &&
			a2Type == amberParameters->improperTorsionTerms[i].numbers[1] &&
			a3Type == amberParameters->improperTorsionTerms[i].numbers[2] &&
			a4Type == amberParameters->improperTorsionTerms[i].numbers[3]
		  )
		{
			forceConstant[0]    = amberParameters->improperTorsionTerms[i].barrier;
			equilibriumAngle[0]    = amberParameters->improperTorsionTerms[i].phase;
			terms[0]     = amberParameters->improperTorsionTerms[i].n;
			return TRUE;
		}
	}

	return FALSE;
}
/**********************************************************************/
static gint getNumberDihedralParameters( AmberParameters* amberParameters,
		gint a1Type, gint a2Type, gint a3Type, gint a4Type,
		gint *n)
{
	gint i;
	gint a1Typet;
	gint a2Typet;
	gint a3Typet;
	gint a4Typet;
	gboolean btype;
	gboolean Ok;
	gint types[4];
	gint k;

	*n = 0;

	a1Typet = a4Type;
	a2Typet = a3Type;
	a3Typet = a2Type;
	a4Typet = a1Type;

	/* Je cherche d'abord sans les -1 */
	for(i=0;i<amberParameters->numberOfDihedralTerms;i++)
	{

		types[0] = a1Type;
		types[1] = a2Type;
		types[2] = a3Type;
		types[3] = a4Type;

		Ok = TRUE;
		for(k=0;k<4;k++)
		{
			btype = (types[k] == amberParameters->dihedralAngleTerms[i].numbers[k]);
			if(!btype)
			{
				Ok = FALSE;
				break;
			}
		}
		if(!Ok)
		{
			types[0] = a1Typet;
			types[1] = a2Typet;
			types[2] = a3Typet;
			types[3] = a4Typet;
			Ok = TRUE;
			for(k=0;k<4;k++)
			{
				btype = (types[k] == amberParameters->dihedralAngleTerms[i].numbers[k]);
				if(!btype)
				{
					Ok = FALSE;
					break;
				}
			}
		}

			 
		if(Ok)
		{
			*n =i;
			return amberParameters->dihedralAngleTerms[i].nSomme;
		}
	}
	/* Je cherche d'abord avec les -1 */
	for(i=0;i<amberParameters->numberOfDihedralTerms;i++)
	{

		types[0] = a1Type;
		types[1] = a2Type;
		types[2] = a3Type;
		types[3] = a4Type;

		Ok = TRUE;
		for(k=0;k<4;k++)
		{
			btype = 
			(amberParameters->dihedralAngleTerms[i].numbers[k] == -1) || 
			(types[k] == amberParameters->dihedralAngleTerms[i].numbers[k]);
			if(!btype)
			{
				Ok = FALSE;
				break;
			}
		}
		if(!Ok)
		{
			types[0] = a1Typet;
			types[1] = a2Typet;
			types[2] = a3Typet;
			types[3] = a4Typet;
			Ok = TRUE;
			for(k=0;k<4;k++)
			{
				btype = 
				(amberParameters->dihedralAngleTerms[i].numbers[k] == -1) || 
				(types[k] == amberParameters->dihedralAngleTerms[i].numbers[k]);
				if(!btype)
				{
					Ok = FALSE;
					break;
				}
			}
		}

			 
		if(Ok)
		{
			*n =i;
			return amberParameters->dihedralAngleTerms[i].nSomme;
		}
	}

	return 0;
}

/**********************************************************************/
static gboolean canHydrogenBond(AmberParameters* amberParameters, gint a1Type, gint a2Type )
{
	AmberAtomTypes* types = amberParameters->atomTypes;

	if( a1Type>-1 && a2Type>-1)
	if(
		types[a1Type].name[0] == 'H' || 
		types[a2Type].name[0] == 'H' 
	  )
			return TRUE;

	return FALSE;
}
/**********************************************************************/
static void setRattleConstraintsParameters(ForceField* forceField)
{
	gint i;
	gint j;
	gint k;
	gint a1,a2,a3;
	gdouble r2;
	gdouble d;
	Molecule* m = &forceField->molecule;
	gint numberOfRattleConstraintsTerms = 0;
	gdouble* rattleConstraintsTerms[RATTLEDIM];

	forceField->numberOfRattleConstraintsTerms = 0;
	for( i=0; i<RATTLEDIM;i++) forceField->rattleConstraintsTerms[i] = NULL;

	if(m->nAtoms<1) return;

	if(forceField->options.rattleConstraints==NOCONSTRAINTS) return;
	numberOfRattleConstraintsTerms = m->numberOf2Connections;
	if(forceField->options.rattleConstraints==BONDSANGLESCONSTRAINTS) 
		numberOfRattleConstraintsTerms += m->numberOf3Connections;

	if(numberOfRattleConstraintsTerms<1) return;
	for( i=0; i<RATTLEDIM;i++)
       		rattleConstraintsTerms[i] = g_malloc(numberOfRattleConstraintsTerms*sizeof(gdouble));


	/* 1=a1, 2=a2, 3=r2a1a2 */
	/* RATTLEDIM 	3 */
	j = 0;
	for ( i = 0; i < m->numberOf2Connections; i++)
	{
    		while( gtk_events_pending() ) gtk_main_iteration();
		if(StopCalcul) break;
		a1 = m->connected2[0][i];
		a2 = m->connected2[1][i];
		if(!m->atoms[a1].variable &&!m->atoms[a2].variable) continue;
		r2 = 0;
		for (k=0;k<3;k++)
		{
			d = m->atoms[a1].coordinates[k]-m->atoms[a2].coordinates[k];
			r2 +=d*d;
		}
		rattleConstraintsTerms[0][j] = a1;
		rattleConstraintsTerms[1][j] = a2;
		rattleConstraintsTerms[2][j] = r2;
		j++;
	}
	if(forceField->options.rattleConstraints==BONDSANGLESCONSTRAINTS)
	{
		gint a1p, a2p;
		gint* nConnections = NULL;
		gint* nAngles = NULL;
       		nConnections = g_malloc(m->nAtoms*sizeof(gint));
       		nAngles = g_malloc(m->nAtoms*sizeof(gint));
		for ( i = 0; i < m->nAtoms; i++)
		{
			nConnections[i] = 0;
			nAngles[i] = 0;
		}
		for ( i = 0; i < m->nAtoms; i++)
		if(m->atoms[i].typeConnections)
		{
			for ( k = 0; k < m->nAtoms; k++)
				if(i!=k && m->atoms[i].typeConnections[m->atoms[k].N-1]>0) nConnections[i]++;
			/* printf("%d %s nCon=%d\n",i,m->atoms[i].mmType,nConnections[i]);*/
		}
		for ( i = 0; i < m->numberOf3Connections; i++)
		{
    			while( gtk_events_pending() ) gtk_main_iteration();
			if(StopCalcul) break;
			a1 = m->connected3[0][i];
			a2 = m->connected3[1][i];
			a3 = m->connected3[2][i];
			if(!m->atoms[a1].variable &&!m->atoms[a3].variable) continue;
			if(nAngles[a2]>=2*nConnections[a2]-3) continue;
			for (k=0;k<j;k++)
			{
				a1p = (gint)rattleConstraintsTerms[0][k];
				a2p = (gint)rattleConstraintsTerms[1][k];
				if(a1p==a1 && a2p==a3) break;
				if(a1p==a3 && a2p==a1) break;
			}
			if(k!=j) continue;

			nAngles[a2]++;
			r2 = 0;
			for (k=0;k<3;k++)
			{
				d = m->atoms[a1].coordinates[k]-m->atoms[a3].coordinates[k];
				r2 +=d*d;
			}
			rattleConstraintsTerms[0][j] = a1;
			rattleConstraintsTerms[1][j] = a3;
			rattleConstraintsTerms[2][j] = r2;
			j++;
		}
		/*
		for ( i = 0; i < m->nAtoms; i++)
		{
			printf("%d %s nAngle = %d 2*nCon-3=%d\n",i,m->atoms[i].mmType,nAngles[i],2*nConnections[i]-3);
		}
		*/
       		if(nConnections) g_free(nConnections);
       		if(nAngles) g_free(nAngles);
	}

	if(j<1)
	{
		numberOfRattleConstraintsTerms=0;
		for( i=0; i<RATTLEDIM;i++)
		{
       			g_free(rattleConstraintsTerms[i]);
       			rattleConstraintsTerms[i] = NULL;
		}
	}
	else if(numberOfRattleConstraintsTerms!=j)
	{
		numberOfRattleConstraintsTerms=j;
		for( i=0; i<RATTLEDIM;i++)
		{
       			rattleConstraintsTerms[i] = 
				g_realloc(rattleConstraintsTerms[i],numberOfRattleConstraintsTerms*sizeof(gdouble));
		}
	}
	forceField->numberOfRattleConstraintsTerms = numberOfRattleConstraintsTerms;
	for( i=0; i<RATTLEDIM;i++)
       		forceField->rattleConstraintsTerms[i] = rattleConstraintsTerms[i]; 

	printf(_("number Of RattleConstraintsTerms = %d\n"), forceField->numberOfRattleConstraintsTerms);
	printf(_("number free degrees = %d\n"), 3*m->nAtoms-6-forceField->numberOfRattleConstraintsTerms);
	/*
	for ( i = 0; i < forceField->numberOfRattleConstraintsTerms; i++)
	{
			a1 = (gint)rattleConstraintsTerms[0][i];
			a2 = (gint)rattleConstraintsTerms[1][i];
			r2 = rattleConstraintsTerms[2][i];
			printf("%d  %d %s %s r2= %f\n",
				a1,a2,
				m->atoms[a1].mmType,
				m->atoms[a2].mmType,
				r2);
	}
	*/
}
/**********************************************************************/
static void setStretchParameters(AmberParameters* amberParameters,ForceField* forceField,gint* atomTypes)
{
	gint i;
	gint a1,a2;
	gint a1Type, a2Type;
	gdouble forceConstant, equilibriumDistance;
	Molecule* m = &forceField->molecule;
	gint numberOfStretchTerms = 0;
	gdouble* bondStretchTerms[STRETCHDIM];

	numberOfStretchTerms = m->numberOf2Connections;
	for( i=0; i<STRETCHDIM;i++)
       		bondStretchTerms[i] = g_malloc(numberOfStretchTerms*sizeof(gdouble));

	/* 1=a1, 2=a2, 3=Force, 4=Re */
	/* STRETCHDIM 	4 */
	for ( i = 0; i < numberOfStretchTerms; i++ )
	{
    		while( gtk_events_pending() )
        		gtk_main_iteration();
		if(StopCalcul)
			break;
		a1 = m->connected2[0][i];
		a2 = m->connected2[1][i];
		a1Type = atomTypes[a1];
		a2Type = atomTypes[a2];
		
		if ( ! ( getStretchParameters(amberParameters, a1Type, a2Type,&forceConstant,&equilibriumDistance ) ) )
		{
			gchar l1 = m->atoms[a1].mmType[0];
			gchar l2 = m->atoms[a2].mmType[0];
			printf( _("**** couldn't find stretch parameters for %s-%s(%d-%d) "), 
				m->atoms[a1].mmType,m->atoms[a2].mmType,a1Type, a2Type);

			forceConstant = 310;
			equilibriumDistance = 1.525;
			if(l1==l2)
			{
				forceConstant = 415;
				equilibriumDistance = 1.5;
			}
			else
			if((l1=='C' && l2=='H' ) || (l1=='H' && l2=='C' ))
			{
				forceConstant = 340;
				equilibriumDistance = 1.09;
			}
			else
			if((l1=='C' && l2=='O' ) || (l1=='O' && l2=='C' ))
			{
				forceConstant = 570;
				equilibriumDistance = 1.229;
			}
			else
			if((l1=='C' && l2=='N' ) || (l1=='N' && l2=='C' ))
			{
				forceConstant = 490;
				equilibriumDistance = 1.335;
			}
			if(isIonic( m->atoms[a1].mmType) || isIonic( m->atoms[a2].mmType))
			{
				forceConstant = 0;
			}
			printf( _("-> I set  force to %f and equilibrium distance to %f\n"),
					forceConstant,equilibriumDistance);

		}

		bondStretchTerms[0][i] = a1;
		bondStretchTerms[1][i] = a2;
		bondStretchTerms[2][i] = forceConstant;
		bondStretchTerms[3][i] = equilibriumDistance;
	}

	forceField->numberOfStretchTerms = numberOfStretchTerms;
	for( i=0; i<STRETCHDIM;i++)
       		forceField->bondStretchTerms[i] = bondStretchTerms[i]; 
}
/**********************************************************************/
static void setBendParameters(AmberParameters* amberParameters,ForceField* forceField,gint* atomTypes)
{
	gint i;
	gint a1,a2,a3;
	gint a1Type, a2Type, a3Type;
	Molecule* m = &forceField->molecule;
	gint numberOfBendTerms = 0;
	gdouble* angleBendTerms[BENDDIM];
	gdouble forceConstant, equilibriumAngle;

	numberOfBendTerms =  m->numberOf3Connections;
	for( i=0; i<BENDDIM;i++)
		angleBendTerms[i] =  g_malloc(numberOfBendTerms*sizeof(gdouble)); 

	/* 5 terms 1=a1, 2=a2, 3=a3, 4=Force, 5=angle */
	/* BENDDIM 5 */
	for ( i = 0; i < numberOfBendTerms; i++ )
	{
    		while( gtk_events_pending() )
        		gtk_main_iteration();
		if(StopCalcul)
			break;
		a1 = m->connected3[0][i];
		a2 = m->connected3[1][i];
		a3 = m->connected3[2][i];
		a1Type = atomTypes[a1];
		a2Type = atomTypes[a2];
		a3Type = atomTypes[a3];

		if ( ! ( getBendParameters(amberParameters, a1Type, a2Type, a3Type,&forceConstant,&equilibriumAngle ) ) )
		{
			gchar l1 = m->atoms[a1].mmType[0];
			gchar l2 = m->atoms[a2].mmType[0];
			gchar l3 = m->atoms[a3].mmType[0];
			printf(_("**** couldn't find bend parameters for %s-%s-%s "),
			m->atoms[a1].mmType,m->atoms[a2].mmType,m->atoms[a3].mmType);
			forceConstant = 60.0;
			equilibriumAngle = 115.0;
			if(!strcmp(m->atoms[a2].mmType,"CT"))
			{
				forceConstant = 50.0;
				equilibriumAngle = 109.0;
			}
			else
			if(l1=='H' || l2=='H' || l3=='H')
			{
				forceConstant = 50.0;
				equilibriumAngle = 120.0;
			}
			if(isIonic( m->atoms[a1].mmType) || isIonic( m->atoms[a2].mmType) ||  isIonic( m->atoms[a3].mmType))
			{
				forceConstant = 0;
			}
			printf(_("-> I set force to %f and equilibrium angle to %f\n"), forceConstant, equilibriumAngle);
		}

		angleBendTerms[0][i] = a1;
		angleBendTerms[1][i] = a2;
		angleBendTerms[2][i] = a3;
		angleBendTerms[3][i] = forceConstant;
		angleBendTerms[4][i] = equilibriumAngle;
	}

	forceField-> numberOfBendTerms = numberOfBendTerms;
	for( i=0; i<BENDDIM;i++)
       		forceField->angleBendTerms[i] = angleBendTerms[i]; 

}
/**********************************************************************/
static void setDihedralParameters(AmberParameters* amberParameters,ForceField* forceField,gint* atomTypes)
{
	gint i;
	gint j;
	gint k;
	gint l;
	gint a1,a2,a3,a4;
	gint a1Type, a2Type, a3Type,a4Type;
	Molecule* m = &forceField->molecule;
	gdouble* dihedralAngleTerms[DIHEDRALDIM];
	gint numberOfDihedralTerms = 0;
	gint dim;

	/*  8 terms 1=a1, 2=a2, 3=a3, 4=a4, 5=Idiv, 6=Pk, 7=Phase, 8=Pn */
	/*  DIHEDRALDIM	8 */

	for( i=0; i<DIHEDRALDIM;i++)
		dihedralAngleTerms[i] =  g_malloc(4*m->numberOf4Connections*sizeof(gdouble)); 

	numberOfDihedralTerms = 0;

	for (  i = 0; i < m->numberOf4Connections; i++ )
	{
    		while( gtk_events_pending() )
        		gtk_main_iteration();
		if(StopCalcul)
			break;
		a1 = m->connected4[0][i];
		a2 = m->connected4[1][i];
		a3 = m->connected4[2][i];
		a4 = m->connected4[3][i];

		a1Type = atomTypes[a1];
		a2Type = atomTypes[a2];
		a3Type = atomTypes[a3];
		a4Type = atomTypes[a4];

		dim = getNumberDihedralParameters(amberParameters, a1Type, a2Type, a3Type, a4Type,&k);
		if(dim>0)
		{
			for(j=0;j<dim;j++)
			{
				dihedralAngleTerms[0][numberOfDihedralTerms] = a1;
				dihedralAngleTerms[1][numberOfDihedralTerms] = a2;
				dihedralAngleTerms[2][numberOfDihedralTerms] = a3;
				dihedralAngleTerms[3][numberOfDihedralTerms] = a4;
				dihedralAngleTerms[4][numberOfDihedralTerms] = 
					amberParameters->dihedralAngleTerms[k].divisor[j];
				dihedralAngleTerms[5][numberOfDihedralTerms] = 
					amberParameters->dihedralAngleTerms[k].barrier[j];
				dihedralAngleTerms[6][numberOfDihedralTerms] = 
					amberParameters->dihedralAngleTerms[k].phase[j];
				dihedralAngleTerms[7][numberOfDihedralTerms] = 
					amberParameters->dihedralAngleTerms[k].n[j];

				numberOfDihedralTerms++;
				if(numberOfDihedralTerms>4*m->numberOf4Connections)
				{
					for( l=0; l<DIHEDRALDIM;l++)
					{
						dihedralAngleTerms[l] =  
						g_realloc(dihedralAngleTerms[l],numberOfDihedralTerms*sizeof(gdouble)); 
					}

				}
			}
		}		
	}

	forceField-> numberOfDihedralTerms = numberOfDihedralTerms;
	for( i=0; i<DIHEDRALDIM;i++)
       		forceField->dihedralAngleTerms[i] = dihedralAngleTerms[i]; 
}
/**********************************************************************/
static void setImproperTorionParameters(AmberParameters* amberParameters, ForceField* forceField,gint* atomTypes)
{
	gint i;
	gint a1,a2,a3,a4;
	gint a1Type, a2Type, a3Type,a4Type;
	Molecule* m = &forceField->molecule;
	gdouble forceConstant, equilibriumAngle, terms;
	gint numberOfImproperTorsionTerms = 0;
	gdouble* improperTorsionTerms[IMPROPERDIHEDRALDIM];

	/*  8 terms 1=a1, 2=a2, 3=a3, 4=a4, 5=Idiv, 6=Pk, 7=Phase, 8=Pn */
	/*  IMPROPERDIHEDRALDIM	8 */

	numberOfImproperTorsionTerms = m->numberOf4Connections;

	for( i=0; i<IMPROPERDIHEDRALDIM;i++)
		improperTorsionTerms[i] =  
			g_malloc(m->numberOf4Connections*sizeof(gdouble)); 

	for (  i = 0; i < numberOfImproperTorsionTerms; i++ )
	{
    		while( gtk_events_pending() )
        		gtk_main_iteration();
		if(StopCalcul)
			break;
		a1 = m->connected4[0][i];
		a2 = m->connected4[1][i];
		a3 = m->connected4[2][i];
		a4 = m->connected4[3][i];

		a1Type = atomTypes[a1];
		a2Type = atomTypes[a2];
		a3Type = atomTypes[a3];
		a4Type = atomTypes[a4];

		getImproperTorsionParameters(amberParameters, a1Type, a2Type, a3Type,a4Type, 
				&forceConstant, &equilibriumAngle, &terms );

		improperTorsionTerms[0][i] = a1;
		improperTorsionTerms[1][i] = a2;
		improperTorsionTerms[2][i] = a3;
		improperTorsionTerms[3][i] = a4;
		improperTorsionTerms[4][i] = forceConstant;
		improperTorsionTerms[5][i] = equilibriumAngle;
		improperTorsionTerms[6][i] = terms;
	}
	forceField-> numberOfImproperTorsionTerms = numberOfImproperTorsionTerms;
	for( i=0; i<IMPROPERDIHEDRALDIM;i++)
       		forceField->improperTorsionTerms[i] = improperTorsionTerms[i]; 

}
/**********************************************************************/
static void setHydrogenBondedParameters(AmberParameters* amberParameters,ForceField* forceField,gint* atomTypes)
{
	gint numberOfHydrogenBonded = 0;
	gint i;
	gint a1,a2;
	gint a1Type,a2Type;
	Molecule* m = &forceField->molecule;
	gdouble C, D;
	gdouble* hydrogenBondedTerms[HYDROGENBONDEDDIM];

	for( i=0; i<HYDROGENBONDEDDIM;i++)
		hydrogenBondedTerms[i] =  g_malloc(m->numberOfNonBonded*sizeof(gdouble));

	for ( i = 0; i < m->numberOfNonBonded; i++ )
	{
    		while( gtk_events_pending() )
        		gtk_main_iteration();
		if(StopCalcul)
			break;
		a1 = m->nonBonded[0][i];
		a2 = m->nonBonded[1][i];

		a1Type = atomTypes[a1];
		a2Type = atomTypes[a2];
		/* printf("a1 = %d a2 = %d %s %s\n",a1,a2, amberParameters->atomTypes[a1Type].name, amberParameters->atomTypes[a2Type].name);*/

		if ( canHydrogenBond( amberParameters, a1Type, a2Type ) )
		{ 
			getHydrogenBondedParameters(amberParameters, a1Type, a2Type, &C, &D );
			hydrogenBondedTerms[0][numberOfHydrogenBonded] = a1;
			hydrogenBondedTerms[1][numberOfHydrogenBonded] = a2;
			hydrogenBondedTerms[2][numberOfHydrogenBonded] = C;
			hydrogenBondedTerms[3][numberOfHydrogenBonded] = D;
			numberOfHydrogenBonded++;
		}
	}

	if(numberOfHydrogenBonded==0)
		for( i=0; i<HYDROGENBONDEDDIM;i++)
		{
			g_free(hydrogenBondedTerms[i]);
			hydrogenBondedTerms[i] = NULL;
		}
	else
		for( i=0; i<HYDROGENBONDEDDIM;i++)
		{
			hydrogenBondedTerms[i] = 
				g_realloc(hydrogenBondedTerms[i],numberOfHydrogenBonded*sizeof(gdouble));
		}

	forceField-> numberOfHydrogenBonded = numberOfHydrogenBonded;
	for( i=0; i<HYDROGENBONDEDDIM;i++)
       		forceField->hydrogenBondedTerms[i] = hydrogenBondedTerms[i]; 

}
/**********************************************************************/
static void setNonBondedParameters(AmberParameters* amberParameters, ForceField* forceField,gint* atomTypes)
{
	gint numberOfNonBonded = 0;
	gint i;
	gint a1,a2,a4;
	gint a1Type,a2Type,a4Type;
	Molecule* m = &forceField->molecule;
	gdouble equilibriumDistance, epsilon;
	gdouble epsilonProduct;
	gdouble ri, rj;
	gdouble Aij, Bij;
	gdouble* nonBondedTerms[NONBONDEDDIM];
	gboolean useHydrogenBonded = forceField->options.hydrogenBonded;

	/* 5 terms 1=a1, 2=a2, 3=Aij, 4=Bij, 5=Coulomb Factor */
	/* NONBONDEDDIM 5 */
	for( i=0; i<NONBONDEDDIM;i++)
		nonBondedTerms[i] =  
			g_malloc((m->numberOfNonBonded+m->numberOf4Connections)*sizeof(gdouble)); 

	for ( i = 0; i < m->numberOfNonBonded; i++ )
	{
    		while( gtk_events_pending() )
        		gtk_main_iteration();
		if(StopCalcul)
			break;
		a1 = m->nonBonded[0][i];
		a2 = m->nonBonded[1][i];
/*
		if(a1==a2)
		{
			printf("Erreur non bonded\n");
			break;
		}
*/


		a1Type = atomTypes[a1];
		a2Type = atomTypes[a2];

		if ( !useHydrogenBonded || !canHydrogenBond(amberParameters, a1Type, a2Type ) )
		{ 
			if ( ! ( getNonBondedParameters(amberParameters, a1Type, &equilibriumDistance, &epsilon ) ) )
				printf(_("**** couldn't find non bonded parameters for %s \n"),m->atoms[a1].mmType);
		
			epsilonProduct = sqrt(fabs(epsilon));
			ri = equilibriumDistance;
			/*printf("r1 = %f eps1 = %f\n",equilibriumDistance,epsilon);*/

			getNonBondedParameters(amberParameters, a2Type, &equilibriumDistance, &epsilon );
			/*printf("r2 = %f eps2 = %f\n",equilibriumDistance,epsilon);*/
			epsilonProduct *= sqrt(fabs(epsilon));
			rj = equilibriumDistance;
			Bij = ( ri + rj ) * ( ri + rj );
			Bij = Bij * Bij * Bij;
			Aij = Bij * Bij * epsilonProduct;
			Bij *= epsilonProduct * 2.0;

			nonBondedTerms[0][numberOfNonBonded] = a1;
			nonBondedTerms[1][numberOfNonBonded] = a2;
			nonBondedTerms[2][numberOfNonBonded] = Aij;
			nonBondedTerms[3][numberOfNonBonded] = Bij;
			nonBondedTerms[4][numberOfNonBonded] = 1.0;
			numberOfNonBonded++;
                }
	}

	/* now 1/2 non bonded */
	for (  i = 0; i < m->numberOf4Connections; i++ )
	{
    		while( gtk_events_pending() )
        		gtk_main_iteration();
		if(StopCalcul)
			break;
		a1 = m->connected4[0][i];
		a4 = m->connected4[3][i];
/*
		if(a1==a4)
		{
			printf("Erreur a1=a4\n");
			break;
		}
*/

		a1Type = atomTypes[a1];
		a4Type = atomTypes[a4];

		epsilonProduct = 0;
		ri = 0;
		rj = 0;
	        if ( getNonBondedParameters(amberParameters, a1Type, &equilibriumDistance, &epsilon ) )
		{
	        	epsilonProduct = sqrt(fabs(epsilon));
	        	ri = equilibriumDistance;
			/*printf("r1 = %f eps1 = %f\n",equilibriumDistance,epsilon);*/
		}
		else
		{
			epsilonProduct = 0;
		}

	        if ( getNonBondedParameters( amberParameters, a4Type, &equilibriumDistance, &epsilon ) )
		{
	        	epsilonProduct *= sqrt(fabs(epsilon));
	        	rj = equilibriumDistance;
			/*printf("r2 = %f eps2 = %f\n",equilibriumDistance,epsilon);*/
		}
		else
		{
			epsilonProduct = 0;
		}

	       	Bij = ( ri + rj ) * ( ri + rj );
	       	Bij = Bij * Bij * Bij;
	       	Aij = Bij * Bij * epsilonProduct / 2.0;
	       	Bij *= epsilonProduct;

		/*
			Aij = 0;
			Bij = 0;
		*/

		nonBondedTerms[0][numberOfNonBonded] = a1;
		nonBondedTerms[1][numberOfNonBonded] = a4;
		nonBondedTerms[2][numberOfNonBonded] = Aij;
		nonBondedTerms[3][numberOfNonBonded] = Bij;
		nonBondedTerms[4][numberOfNonBonded] = 1.0/(gdouble)1.2;
		numberOfNonBonded++;
	}
	if(numberOfNonBonded==0)
		for( i=0; i<NONBONDEDDIM;i++)
		{
			g_free(nonBondedTerms[i]);
			nonBondedTerms[i] = NULL;
		}
	else
		for( i=0; i<NONBONDEDDIM;i++)
			nonBondedTerms[i] = 
				g_realloc(nonBondedTerms[i],numberOfNonBonded*sizeof(gdouble));

	forceField-> numberOfNonBonded = numberOfNonBonded;
	for( i=0; i<NONBONDEDDIM;i++)
       		forceField->nonBondedTerms[i] = nonBondedTerms[i]; 
}
/**********************************************************************/
static void setPairWiseParameters(AmberParameters* amberParameters, ForceField* forceField,gint* atomTypes)
{
	gint numberOfPairWise = 0;
	gint i;
	gint j;
	gint a1,a2;
	gint a1Type,a2Type;
	Molecule* m = &forceField->molecule;
	gdouble a, beta, c6, c8, c10, b;
	gdouble* pairWiseTerms[PAIRWISEDIM];

	numberOfPairWise = m->nAtoms*(m->nAtoms-1)/2;

	/* PAIRWISEDIM 8 */
	for( i=0; i<PAIRWISEDIM;i++)
		pairWiseTerms[i] =  
			g_malloc((numberOfPairWise)*sizeof(gdouble)); 

	numberOfPairWise = 0;
	for ( i = 0; i < m->nAtoms; i++ )
	for ( j = i+1; j < m->nAtoms; j++ )
	{
    		while( gtk_events_pending() )
        		gtk_main_iteration();
		if(StopCalcul)
			break;

		a1 = i;
		a2 = j;

		a1Type = atomTypes[a1];
		a2Type = atomTypes[a2];

		if ( ! ( getPairWiseParameters(amberParameters, a1Type,a2Type,&a, &beta,&c6,&c8, &c10,&b) ) )
				printf( _("**** couldn't find pair wise parameters for %s-%s\n"),
					m->atoms[a1].mmType, m->atoms[a2].mmType);
		
			pairWiseTerms[0][numberOfPairWise] = a1;
			pairWiseTerms[1][numberOfPairWise] = a2;
			pairWiseTerms[2][numberOfPairWise] = a;
			pairWiseTerms[3][numberOfPairWise] = beta;
			pairWiseTerms[4][numberOfPairWise] = c6;
			pairWiseTerms[5][numberOfPairWise] = c8;
			pairWiseTerms[6][numberOfPairWise] = c10;
			pairWiseTerms[7][numberOfPairWise] = b;
			numberOfPairWise++;
	}

	if(numberOfPairWise==0)
		for( i=0; i<PAIRWISEDIM;i++)
		{
			g_free(pairWiseTerms[i]);
			pairWiseTerms[i] = NULL;
		}
	else
		for( i=0; i<PAIRWISEDIM;i++)
			pairWiseTerms[i] = 
				g_realloc(pairWiseTerms[i],numberOfPairWise*sizeof(gdouble));

	forceField-> numberOfPairWise = numberOfPairWise;
	for( i=0; i<PAIRWISEDIM;i++)
       		forceField->pairWiseTerms[i] = pairWiseTerms[i]; 
}
/**********************************************************************/
static void setAtomTypes(AmberParameters* amberParameters,ForceField* forceField,gint* atomTypes)
{
	Molecule* m = &forceField->molecule;
	gint nAtoms = m->nAtoms;
	gint i;
	for(i=0;i<nAtoms;i++)
	{ 
		/* printf("Atom %s=",m->atoms[i].mmType); */
		atomTypes[i] = getNumberType(amberParameters, m->atoms[i].mmType);
		/*
		{
			gint j;
			gint nTypes = amberParameters->numberOfTypes;
			AmberAtomTypes* types = amberParameters->atomTypes;
			gchar* type = m->atoms[i].mmType;
			gint len = strlen(type);

			if(strcmp(type,"X")==0)
				printf("-1\n");
			for(j=0;j<nTypes;j++)
			{
					if(len == (gint)strlen(types[j].name) && 
						strstr(types[j].name,type))
						printf(" %d \n",types[j].number);
			}
		}
		*/
	}
	
}
/**********************************************************************/
static void setAmberParameters(ForceField* forceField)
{
	Molecule* m = &forceField->molecule;
	gint* atomTypes = g_malloc(m->nAtoms*sizeof(gint));
	AmberParameters amberParameters;



	if(staticAmberParameters && staticAmberParameters->numberOfTypes >0 )
		amberParameters = *staticAmberParameters;
	else
	{
		gchar* persoFileName = 
		g_strdup_printf("%s%sPersonalMM.prm",gabedit_directory(), G_DIR_SEPARATOR_S);

		gchar* defaultFileName = 
		g_strdup_printf("%s%sMolecularMechanics.prm",gabedit_directory(), G_DIR_SEPARATOR_S);

		amberParameters =  newAmberParameters();
		if(!readAmberParameters(&amberParameters,persoFileName))
			if(!readAmberParameters(&amberParameters,persoFileName))
			{
				g_free(persoFileName);
				g_free(defaultFileName);
				return;
			}

		staticAmberParameters = g_malloc(sizeof(AmberParameters));
		*staticAmberParameters = amberParameters;

		g_free(persoFileName);
		g_free(defaultFileName);

	}

	setAtomTypes(&amberParameters,forceField,atomTypes);
    	while( gtk_events_pending() )
        	gtk_main_iteration();
	if(StopCalcul)
	{
		return;
	}

    	while( gtk_events_pending() ) gtk_main_iteration();
	if(forceField->options.bondStretch) setStretchParameters(&amberParameters,forceField,atomTypes);
	if(StopCalcul) return;
    	while( gtk_events_pending() ) gtk_main_iteration();

	if(forceField->options.angleBend) setBendParameters(&amberParameters,forceField,atomTypes);
	if(StopCalcul) return;
    	while( gtk_events_pending() ) gtk_main_iteration();

	if(forceField->options.dihedralAngle) setDihedralParameters(&amberParameters, forceField,atomTypes);
	if(StopCalcul) return;
    	while( gtk_events_pending() ) gtk_main_iteration();

	if(forceField->options.improperTorsion) setImproperTorionParameters(&amberParameters,forceField,atomTypes);
	if(StopCalcul) return;
    	while( gtk_events_pending() ) gtk_main_iteration();

	if(forceField->options.hydrogenBonded) setHydrogenBondedParameters(&amberParameters,forceField,atomTypes);
	if(StopCalcul) return;
    	while( gtk_events_pending() ) gtk_main_iteration();
	
	if(forceField->options.nonBonded) setNonBondedParameters(&amberParameters,forceField,atomTypes);
	if(StopCalcul) return;
    	while( gtk_events_pending() ) gtk_main_iteration();

	if(forceField->options.rattleConstraints!=NOCONSTRAINTS) setRattleConstraintsParameters(forceField);
	if(StopCalcul) return;
    	while( gtk_events_pending() ) gtk_main_iteration();
	

	/*
	freeAmberParameters(&amberParameters);
	*/
}
/**********************************************************************/
static void setAllPairWiseParameters(ForceField* forceField)
{
	Molecule* m = &forceField->molecule;
	gint* atomTypes = g_malloc(m->nAtoms*sizeof(gint));
	AmberParameters amberParameters;



	if(staticAmberParameters && staticAmberParameters->numberOfTypes >0 )
		amberParameters = *staticAmberParameters;
	else
	{
		gchar* persoFileName = 
		g_strdup_printf("%s%sPersonalMM.prm",gabedit_directory(), G_DIR_SEPARATOR_S);

		gchar* defaultFileName = 
		g_strdup_printf("%s%sMolecularMechanics.prm",gabedit_directory(), G_DIR_SEPARATOR_S);

		amberParameters =  newAmberParameters();
		if(!readAmberParameters(&amberParameters,persoFileName))
			if(!readAmberParameters(&amberParameters,persoFileName))
			{
				g_free(persoFileName);
				g_free(defaultFileName);
				return;
			}

		staticAmberParameters = g_malloc(sizeof(AmberParameters));
		*staticAmberParameters = amberParameters;

		g_free(persoFileName);
		g_free(defaultFileName);

	}
	

	setAtomTypes(&amberParameters,forceField,atomTypes);
	
    	while( gtk_events_pending() )
        	gtk_main_iteration();
	if(StopCalcul)
	{
		return;
	}

    	while( gtk_events_pending() )
        	gtk_main_iteration();

		
	setPairWiseParameters(&amberParameters,forceField,atomTypes);
	if(StopCalcul) return;
	while( gtk_events_pending() ) gtk_main_iteration();

	if(forceField->options.rattleConstraints!=NOCONSTRAINTS) setRattleConstraintsParameters(forceField);
	if(StopCalcul) return;
    	while( gtk_events_pending() ) gtk_main_iteration();

	/*
	freeAmberParameters(&amberParameters);
	*/
}
/**********************************************************************/
static void calculateGradientBondAmber(ForceField* forceField)
{
	gint i;
	gint ai, aj;
	AtomMol atomi,atomj;
	gdouble rijx, rijy, rijz, forceConstant, equilibriumDistance, term;
	gdouble forceix, forceiy, forceiz;
	gdouble bondLength;
	Molecule* m = &forceField->molecule;
	gdouble* bondStretchTerms[STRETCHDIM];
	gint numberOfStretchTerms = forceField->numberOfStretchTerms;

	for( i=0; i<STRETCHDIM;i++)
       		bondStretchTerms[i] = forceField->bondStretchTerms[i];

#ifdef ENABLE_OMP
#pragma omp parallel for private(i,ai,aj,forceConstant, equilibriumDistance,atomi,atomj,rijx,rijy,rijz,bondLength,term,forceix,forceiy,forceiz) 
#endif
	for ( i = 0; i < numberOfStretchTerms; i++ )
	{
		ai = (gint)bondStretchTerms[0][i];
		aj = (gint)bondStretchTerms[1][i];
		forceConstant = bondStretchTerms[2][i];
		equilibriumDistance = bondStretchTerms[3][i];
		atomi =  m->atoms[ai];
		atomj =  m->atoms[aj];

		rijx = atomi.coordinates[0]  - atomj.coordinates[0];	
		rijy = atomi.coordinates[1]  - atomj.coordinates[1];	
		rijz = atomi.coordinates[2]  - atomj.coordinates[2];	

		bondLength = sqrt( rijx * rijx + rijy * rijy + rijz * rijz );

		if ( bondLength < 1.0e-10 ) 
			bondLength = 1.0e-10;

		term = - 2*forceConstant * ( bondLength - equilibriumDistance ) / bondLength;
		forceix = term * rijx;
		forceiy = term * rijy;
		forceiz = term * rijz;
#ifdef ENABLE_OMP
#pragma omp critical
#endif
		{
		m->gradient[0][ai] -= forceix;
		m->gradient[1][ai] -= forceiy;
		m->gradient[2][ai] -= forceiz;
		
		m->gradient[0][aj] += forceix;
		m->gradient[1][aj] += forceiy;
		m->gradient[2][aj] += forceiz;
		}
	} 
}
/**********************************************************************/
static void calculateGradientBendAmber(ForceField* forceField)
{
	gint i;

	Molecule* m = &forceField->molecule;
	gdouble* angleBendTerms[BENDDIM];
	static gdouble	D2R = 1.0/57.29577951308232090712;
	gint numberOfBendTerms = forceField->numberOfBendTerms;

	for( i=0; i<BENDDIM;i++)
		angleBendTerms[i] = forceField->angleBendTerms[i]; 

#ifdef ENABLE_OMP
#pragma omp parallel for private(i)
#endif
	for ( i = 0; i < numberOfBendTerms; i++ )
	{
		gint ai, aj, ak;
		AtomMol atomi,atomj,atomk;
		gdouble term;
		gdouble thetaDeg, thetaRad, cosTheta;
		gdouble denominator, absTheta;
		gdouble delta = 1e-10;

		gdouble rijx, rijy, rijz;
		gdouble rkjx, rkjy, rkjz;
		gdouble rij2, rij, rkj2, rkj,rij3, rkj3;
		gdouble denominatori, denominatork;

		gdouble forceix, forceiy, forceiz;
		gdouble forcejx, forcejy, forcejz;
		gdouble forcekx, forceky, forcekz;

		gdouble rijDotrkj;
		gdouble term2ix, term2iy, term2iz;
		gdouble term2jx, term2jy, term2jz;
		gdouble term2kx, term2ky, term2kz;

		ai = (gint)angleBendTerms[0][i];
		aj = (gint)angleBendTerms[1][i];
		ak = (gint)angleBendTerms[2][i];
		atomi =  m->atoms[ai];
		atomj =  m->atoms[aj];
		atomk =  m->atoms[ak];

		thetaDeg = getAngle(&atomi, &atomj, &atomk);
		thetaRad = thetaDeg * D2R;
		absTheta = fabs( thetaDeg );
		cosTheta = cos( thetaRad );

		if ( ( absTheta > delta ) && ( absTheta < 180.0 - delta ) )
		{
			/*denominator = sqrt( 1 - cosTheta * cosTheta );*/
			denominator = sin(thetaRad);
			
			if ( denominator < 1.0e-10 )
			{
				printf("cut denominator\n");
				denominator = 1.0e-10;
			}

			term = 2*angleBendTerms[3][i] * (thetaDeg - angleBendTerms[4][i]) / denominator;
			term *= D2R;

			rijx = atomi.coordinates[0] - atomj.coordinates[0];
			rijy = atomi.coordinates[1] - atomj.coordinates[1];
			rijz = atomi.coordinates[2] - atomj.coordinates[2];

			rkjx = atomk.coordinates[0] - atomj.coordinates[0];
			rkjy = atomk.coordinates[1] - atomj.coordinates[1];
			rkjz = atomk.coordinates[2] - atomj.coordinates[2];

			rij2 = rijx * rijx + rijy * rijy + rijz * rijz;
			rij = sqrt( rij2 );

			rkj2 = rkjx * rkjx + rkjy * rkjy + rkjz * rkjz;
			rkj = sqrt( rkj2 );

			rijDotrkj = rijx * rkjx + rijy * rkjy + rijz * rkjz;

			rij3 = rij2 * rij;
			rkj3 = rkj2 * rkj;

			denominatori = rij3 * rkj;
			if ( denominatori < 1.0e-10 )
			{
				printf("cut denominatori\n");
				denominatori = 1.0e-10;
			}

			denominatork = rij * rkj3;
			if ( denominatork < 1.0e-10 )
			{
				printf("cut denominatork\n");
				denominatork = 1.0e-10;
			}
			
			term2ix = ( rij2 * rkjx - rijDotrkj * rijx ) / denominatori;
			term2iy = ( rij2 * rkjy - rijDotrkj * rijy ) / denominatori;
			term2iz = ( rij2 * rkjz - rijDotrkj * rijz ) / denominatori;
			
			term2kx = ( rkj2 * rijx - rijDotrkj * rkjx ) / denominatork;
			term2ky = ( rkj2 * rijy - rijDotrkj * rkjy ) / denominatork;
			term2kz = ( rkj2 * rijz - rijDotrkj * rkjz ) / denominatork;
			
			term2jx = - term2ix - term2kx;
			term2jy = - term2iy - term2ky;
			term2jz = - term2iz - term2kz;
			
			forceix = term * term2ix;
			forceiy = term * term2iy;
			forceiz = term * term2iz;
			
			forcejx = term * term2jx;
			forcejy = term * term2jy;
			forcejz = term * term2jz;
			
			forcekx = term * term2kx;
			forceky = term * term2ky;
			forcekz = term * term2kz;
			
#ifdef ENABLE_OMP
#pragma omp critical
#endif
			{
			m->gradient[0][ai] -= forceix;
			m->gradient[1][ai] -= forceiy;
			m->gradient[2][ai] -= forceiz;
			
			m->gradient[0][aj] -= forcejx;
			m->gradient[1][aj] -= forcejy;
			m->gradient[2][aj] -= forcejz;
			
			m->gradient[0][ak] -= forcekx;
			m->gradient[1][ak] -= forceky;
			m->gradient[2][ak] -= forcekz;
			}
		}
	} 
}
/**********************************************************************/
static void calculateGradientDihedralAmber(ForceField* forceField)
{

	gint i;

	Molecule* m = &forceField->molecule;
	gdouble* dihedralAngleTerms[DIHEDRALDIM];
	static gdouble	D2R = 1.0/57.29577951308232090712;
	gint numberOfDihedralTerms = forceField->numberOfDihedralTerms;

	for(i=0;i<DIHEDRALDIM;i++)
		dihedralAngleTerms[i] = forceField->dihedralAngleTerms[i];

#ifdef ENABLE_OMP
#pragma omp parallel for private(i) 
#endif
	for (  i = 0; i < numberOfDihedralTerms; i++ )
	{
		gint ai, aj, ak, al;
		AtomMol atomi,atomj,atomk,atoml;
		gint j;

		gdouble rjix, rjiy, rjiz;
		gdouble rkjx, rkjy, rkjz;
		gdouble rkix, rkiy, rkiz;
		gdouble rljx, rljy, rljz;
		gdouble rlkx, rlky, rlkz;

		gdouble forceix, forceiy, forceiz;
		gdouble forcejx, forcejy, forcejz;
		gdouble forcekx, forceky, forcekz;
		gdouble forcelx, forcely, forcelz;

		gdouble rkj;
		gdouble xt, yt, zt;
		gdouble xu, yu, zu;
		gdouble xtu, ytu, ztu;
		gdouble rt2, ru2, rtru;
		gdouble cosine1, sine1, cosineN, sineN, cosold, sinold;
		gdouble cosPhase, sinPhase;
		gdouble dedxt, dedyt, dedzt;
		gdouble dedxu, dedyu, dedzu;
		gdouble dedphi;
		gint n;
		gdouble vn;

		ai = (gint)dihedralAngleTerms[0][i];
		aj = (gint)dihedralAngleTerms[1][i];
		ak = (gint)dihedralAngleTerms[2][i];
		al = (gint)dihedralAngleTerms[3][i];

		atomi =  m->atoms[ai];
		atomj =  m->atoms[aj];
		atomk =  m->atoms[ak];
		atoml =  m->atoms[al];

		rjix = atomj.coordinates[0] - atomi.coordinates[0];
		rjiy = atomj.coordinates[1] - atomi.coordinates[1];
		rjiz = atomj.coordinates[2] - atomi.coordinates[2];

		rkjx = atomk.coordinates[0] - atomj.coordinates[0];
		rkjy = atomk.coordinates[1] - atomj.coordinates[1];
		rkjz = atomk.coordinates[2] - atomj.coordinates[2];

		rlkx = atoml.coordinates[0] - atomk.coordinates[0];
		rlky = atoml.coordinates[1] - atomk.coordinates[1];
		rlkz = atoml.coordinates[2] - atomk.coordinates[2];


		xt = rjiy*rkjz - rkjy*rjiz;
		yt = rjiz*rkjx - rkjz*rjix;
		zt = rjix*rkjy - rkjx*rjiy;

		xu = rkjy*rlkz - rlky*rkjz;
		yu = rkjz*rlkx - rlkz*rkjx;
		zu = rkjx*rlky - rlkx*rkjy;

		xtu = yt*zu - yu*zt;
		ytu = zt*xu - zu*xt;
		ztu = xt*yu - xu*yt;

		rt2 = xt*xt + yt*yt + zt*zt;
		ru2 = xu*xu + yu*yu + zu*zu;

		rtru = sqrt(rt2 * ru2);

		rkj = sqrt(rkjx*rkjx + rkjy*rkjy + rkjz*rkjz);
		cosine1 = 1.0;
		sine1   = 0.0;

		if (rtru <1e-10) rtru = 1e-10;
		if (rt2 <1e-10) rt2 = 1e-10;
		if (ru2 <1e-10) ru2 = 1e-10;

		cosine1 = (xt*xu + yt*yu + zt*zu) / rtru;
		sine1 = (rkjx*xtu + rkjy*ytu + rkjz*ztu) / (rkj*rtru);

		n = (gint)dihedralAngleTerms[7][i];
		cosPhase = cos(D2R*dihedralAngleTerms[6][i]);
		sinPhase = sin(D2R*dihedralAngleTerms[6][i]);
		vn = dihedralAngleTerms[5][i]/dihedralAngleTerms[4][i];

/*
     compute the multiple angle trigonometry and the phase terms
*/
		
		cosineN = cosine1;
		sineN   = sine1;

		for(j=2;j<=n;j++)
		{
		   cosold = cosineN;
		   sinold = sineN;
		   cosineN = cosine1*cosold - sine1*sinold;
		   sineN   = cosine1*sinold + sine1*cosold;
		}

		dedphi = vn*n*(cosineN*sinPhase-sineN*cosPhase);

/*
     chain rule terms for first derivative components
*/

		rkix = atomk.coordinates[0] - atomi.coordinates[0];
		rkiy = atomk.coordinates[1] - atomi.coordinates[1];
		rkiz = atomk.coordinates[2] - atomi.coordinates[2];

		rljx = atoml.coordinates[0] - atomj.coordinates[0];
		rljy = atoml.coordinates[1] - atomj.coordinates[1];
		rljz = atoml.coordinates[2] - atomj.coordinates[2];

		dedxt = dedphi * (yt*rkjz - rkjy*zt) / (rt2*rkj);
		dedyt = dedphi * (zt*rkjx - rkjz*xt) / (rt2*rkj);
		dedzt = dedphi * (xt*rkjy - rkjx*yt) / (rt2*rkj);

		dedxu = -dedphi * (yu*rkjz - rkjy*zu) / (ru2*rkj);
		dedyu = -dedphi * (zu*rkjx - rkjz*xu) / (ru2*rkj);
		dedzu = -dedphi * (xu*rkjy - rkjx*yu) / (ru2*rkj);
/*

     compute first derivative components for this angle
*/

		forceix = rkjz*dedyt - rkjy*dedzt;
		forceiy = rkjx*dedzt - rkjz*dedxt;
		forceiz = rkjy*dedxt - rkjx*dedyt;

		forcejx = rkiy*dedzt - rkiz*dedyt + rlkz*dedyu - rlky*dedzu;
		forcejy = rkiz*dedxt - rkix*dedzt + rlkx*dedzu - rlkz*dedxu;
		forcejz = rkix*dedyt - rkiy*dedxt + rlky*dedxu - rlkx*dedyu;

		forcekx = rjiz*dedyt - rjiy*dedzt + rljy*dedzu - rljz*dedyu;
		forceky = rjix*dedzt - rjiz*dedxt + rljz*dedxu - rljx*dedzu;
		forcekz = rjiy*dedxt - rjix*dedyt + rljx*dedyu - rljy*dedxu;

		forcelx = rkjz*dedyu - rkjy*dedzu;
		forcely = rkjx*dedzu - rkjz*dedxu;
		forcelz = rkjy*dedxu - rkjx*dedyu;

#ifdef ENABLE_OMP
#pragma omp critical
#endif
		{
		m->gradient[0][ai] += forceix;
		m->gradient[1][ai] += forceiy;
		m->gradient[2][ai] += forceiz;

		m->gradient[0][aj] += forcejx;
		m->gradient[1][aj] += forcejy;
		m->gradient[2][aj] += forcejz;

		m->gradient[0][ak] += forcekx;
		m->gradient[1][ak] += forceky;
		m->gradient[2][ak] += forcekz;

		m->gradient[0][al] += forcelx;
		m->gradient[1][al] += forcely;
		m->gradient[2][al] += forcelz;
		}
	}
}
/**********************************************************************/
static void calculateGradientImproperTorsion(ForceField* forceField)
{
}
/**********************************************************************/
static void calculateGradientNonBondedAmber(ForceField* forceField)
{
	gint i;

	gboolean useCoulomb = forceField->options.coulomb;
	Molecule* m = &forceField->molecule;
	gdouble* nonBondedTerms[NONBONDEDDIM];
	gint numberOfNonBonded = forceField->numberOfNonBonded;

	for(i=0;i<NONBONDEDDIM;i++)
		nonBondedTerms[i] = forceField->nonBondedTerms[i];

	/* non-bonded part */
#ifdef ENABLE_OMP
#pragma omp parallel for private(i) 
#endif
	for (  i = 0; i < numberOfNonBonded; i++ )
	{
		gint ai, aj;
		AtomMol atomi,atomj;

		gdouble rijx, rijy, rijz;

		gdouble forceix, forceiy, forceiz;
		gdouble forcejx, forcejy, forcejz;

		gdouble permittivityScale = 1, permittivity = 1;
		gdouble coulombFactor, factorNonBonded;
		gdouble rij2, rij;
		gdouble rij3;
		gdouble chargei, chargej, coulombTerm;
		gdouble Aij, Bij, rij6, rij7, rij14, rij8;
		gdouble  term1, term2, term3;

		coulombFactor = 332.05382 / ( permittivity * permittivityScale );

		ai       = (gint)nonBondedTerms[0][i];
		aj       = (gint)nonBondedTerms[1][i];
		Aij      = nonBondedTerms[2][i];
		Bij      = nonBondedTerms[3][i];
		factorNonBonded   = nonBondedTerms[4][i];

		atomi =  m->atoms[ai];
		atomj =  m->atoms[aj];

		chargei = atomi.charge;
		chargej = atomj.charge;


		rijx = atomi.coordinates[0] - atomj.coordinates[0];
		rijy = atomi.coordinates[1] - atomj.coordinates[1];
		rijz = atomi.coordinates[2] - atomj.coordinates[2];

		rij2 = rijx * rijx + rijy * rijy + rijz * rijz;
		if ( rij2 < 1.0e-2 )
			rij2 = 1.0e-2;	

		rij = sqrt( rij2 );
		rij3 = rij2 * rij;
		rij6 = rij3 * rij3;
		rij7 = rij6 * rij;
		rij8 = rij7 * rij;
		rij14 = rij7 * rij7;
		if(useCoulomb)
			coulombTerm = ( chargei * chargej * coulombFactor*factorNonBonded ) / rij3;
		else
			coulombTerm = 0.0;

		
		/*printf("coulombTerm = %f\n",coulombTerm);*/
		
		term1 = 12 * Aij / rij14;
		term2 = 6 * Bij / rij8;
		term3 = term1 - term2 + coulombTerm;
		forceix = term3 * rijx;
		forceiy = term3 * rijy;
		forceiz = term3 * rijz;
		forcejx = - forceix;
		forcejy = - forceiy;
		forcejz = - forceiz;
#ifdef ENABLE_OMP
#pragma omp critical
#endif
		{
		m->gradient[0][ai] -= forceix;
		m->gradient[1][ai] -= forceiy;
		m->gradient[2][ai] -= forceiz;
		m->gradient[0][aj] -= forcejx;
		m->gradient[1][aj] -= forcejy;
		m->gradient[2][aj] -= forcejz;
		}
	}  
}
/*********************************************************************/
static void calculateGradientHydrogenBondedAmber(ForceField* forceField)
{
	gint i;


	Molecule* m = &forceField->molecule;
	gdouble* hydrogenBondedTerms[HYDROGENBONDEDDIM];
	gint numberOfHydrogenBonded =  forceField->numberOfHydrogenBonded;

	for(i=0;i<HYDROGENBONDEDDIM;i++)
		hydrogenBondedTerms[i] = forceField->hydrogenBondedTerms[i];

	/* Hydrogen-bonded part */
#ifdef ENABLE_OMP
#pragma omp parallel for private(i) 
#endif
	for (  i = 0; i < numberOfHydrogenBonded; i++ )
	{
		gint ai, aj;
		AtomMol atomi,atomj;

		gdouble rijx, rijy, rijz;

		gdouble forceix, forceiy, forceiz;
		gdouble forcejx, forcejy, forcejz;

		gdouble Cij, Dij, rij2,  rij4, rij8, rij12, rij14;
		gdouble  term1, term2, term3;

		ai = (gint)hydrogenBondedTerms[0][i];
		aj = (gint)hydrogenBondedTerms[1][i];
		Cij = hydrogenBondedTerms[2][i];
		Dij = hydrogenBondedTerms[3][i];

		atomi =  m->atoms[ai];
		atomj =  m->atoms[aj];

		rijx = atomi.coordinates[0] - atomj.coordinates[0];
		rijy = atomi.coordinates[1] - atomj.coordinates[1];
		rijz = atomi.coordinates[2] - atomj.coordinates[2];

		rij2 = rijx * rijx + rijy * rijy + rijz * rijz;

		if ( rij2 < 1.0e-2 )
			rij2 = 1.0e-2;	

		rij4 = rij2 * rij2;
		rij8 = rij4 * rij4;
		rij12 = rij8 * rij4;
		rij14 = rij12 * rij2;
		term1 = Cij / rij14;
		term2 = Dij / rij12;
		term3 = term1 - term2;
		forceix = term3 * rijx;
		forceiy = term3 * rijy;
		forceiz = term3 * rijz;
		forcejx = - forceix;
		forcejy = - forceiy;
		forcejz = - forceiz;
#ifdef ENABLE_OMP
#pragma omp critical
#endif
		{
		m->gradient[0][ai] -= forceix;
		m->gradient[1][ai] -= forceiy;
		m->gradient[2][ai] -= forceiz;
		m->gradient[0][aj] -= forcejx;
		m->gradient[1][aj] -= forcejy;
		m->gradient[2][aj] -= forcejz;
		}
	}
}
/**********************************************************************/
static void calculateGradientPairWise(ForceField* forceField)
{
	gint i;
	gint ai, aj;
	AtomMol atomi,atomj;

	gdouble rijx, rijy, rijz;

	gdouble forceix, forceiy, forceiz;
	gdouble forcejx, forcejy, forcejz;

	gdouble permittivityScale = 1, permittivity = 1;
	gdouble coulombFactor;
	gdouble rij2, rij;
	gdouble rij3;
	gdouble chargei, chargej, coulombTerm;
	gdouble rij6, rij7, rij8, rij9, rij10, rij11, rij12;
	gdouble  term1, term6, term8, term10, termAll;
	gdouble A, Beta, C6, C8, C10,b;
	gdouble s, sp, fact, br, brk, ebr;
	gint n, k;

	gboolean useCoulomb = forceField->options.coulomb;
	gboolean useVanderWals = forceField->options.vanderWals;
	Molecule* m = &forceField->molecule;
	gdouble* pairWiseTerms[PAIRWISEDIM];
	gint numberOfPairWise = forceField->numberOfPairWise;

	for(i=0;i<PAIRWISEDIM;i++)
		pairWiseTerms[i] = forceField->pairWiseTerms[i];

	/* non-bonded part */
	coulombFactor = 332.05382 / ( permittivity * permittivityScale );
	for (  i = 0; i < numberOfPairWise; i++ )
	{
		ai       = (gint)pairWiseTerms[0][i];
		aj       = (gint)pairWiseTerms[1][i];
		A        = pairWiseTerms[2][i];
		Beta     = pairWiseTerms[3][i];
		C6       = pairWiseTerms[4][i];
		C8       = pairWiseTerms[5][i];
		C10      = pairWiseTerms[6][i];
		b        = pairWiseTerms[7][i];

		atomi =  m->atoms[ai];
		atomj =  m->atoms[aj];

		chargei = atomi.charge;
		chargej = atomj.charge;


		rijx = atomi.coordinates[0] - atomj.coordinates[0];
		rijy = atomi.coordinates[1] - atomj.coordinates[1];
		rijz = atomi.coordinates[2] - atomj.coordinates[2];

		rij2 = rijx * rijx + rijy * rijy + rijz * rijz;
		if ( rij2 < 1.0e-8 ) rij2 = 1.0e-8;	

		rij = sqrt( rij2 );
		rij3 = rij2 * rij;
		rij6 = rij3 * rij3;
		rij7 = rij6 * rij;
		rij8 = rij7 * rij;
		rij9 = rij8 * rij;
		rij10 = rij9 * rij;
		rij11 = rij10 * rij;
		rij12 = rij11 * rij;
		if(useCoulomb) coulombTerm = ( chargei * chargej * coulombFactor ) / rij3;
		else coulombTerm = 0.0;
		
		/* printf("A = %f Beta = %f qi = %f qj = %f rij = %f\n",A,Beta,chargei,chargej,rij);*/
		
		/*term1 = -A*Beta/rij*exp(-Beta*rij);*/
		term1 = A*Beta/rij*exp(-Beta*rij);

		br = b*rij;
		ebr = exp(-b*rij);

		term6 =   0.0;
		if(useVanderWals && fabs(C6)>1e-12)
		{
			fact = 1.0;
			s = 1.0;
			n = 3;
			brk = 1.0;
			for(k=1;k<2*n;k++)
			{
				fact *= k;
				brk *= br;
				s += brk/fact;
			}
			sp = s*b;
			fact *=2*n;
			brk *= br;
			s += brk/fact;
			term6 =   b*C6*ebr*s/rij7
				-(2*n)*C6*(1-ebr*s)/rij8
				-C6*ebr/rij7*sp;
		}
		term8 =   0.0;
		if(useVanderWals && fabs(C8)>1e-12)
		{
			fact = 1.0;
			s = 1.0;
			n = 4;
			brk = 1.0;
			for(k=1;k<2*n;k++)
			{
				fact *= k;
				brk *= br;
				s += brk/fact;
			}
			sp = s*b;
			fact *=2*n;
			brk *= br;
			s += brk/fact;
			term8 =   b*C8*ebr*s/rij9
				-(2*n)*C8*(1-ebr*s)/rij10
				-C8*ebr/rij9*sp;
		}

		term10 =   0.0;
		if(useVanderWals && fabs(C10)>1e-12)
		{
			fact = 1.0;
			s = 1.0;
			n = 5;
			brk = 1.0;
			for(k=1;k<2*n;k++)
			{
				fact *= k;
				brk *= br;
				s += brk/fact;
			}
			sp = s*b;

			fact *=2*n;
			brk *= br;
			s += brk/fact;
			term10 =   b*C10*ebr*s/rij11
				-(2*n)*C10*(1-ebr*s)/rij12
				-C10*ebr/rij11*sp;
		}

		//termAll = term1 - term6 - term8 - term10 + coulombTerm;
		termAll = term1 + term6 + term8 + term10 + coulombTerm;


		forceix = termAll * rijx;
		forceiy = termAll * rijy;
		forceiz = termAll * rijz;
		forcejx = - forceix;
		forcejy = - forceiy;
		forcejz = - forceiz;
		{
		m->gradient[0][ai] -= forceix;
		m->gradient[1][ai] -= forceiy;
		m->gradient[2][ai] -= forceiz;
		m->gradient[0][aj] -= forcejx;
		m->gradient[1][aj] -= forcejy;
		m->gradient[2][aj] -= forcejz;
		}
	}  
}
/**********************************************************************/
static void calculateGradientAmber(ForceField* forceField)
{
	gint i;
	gint j;
	Molecule* m = &forceField->molecule;

	for(j=0;j<3;j++)
		for( i=0; i<m->nAtoms;i++)
			m->gradient[j][i] = 0.0;

	calculateGradientBondAmber(forceField);
	if(StopCalcul) return;
	calculateGradientBendAmber(forceField);
	if(StopCalcul) return;
	calculateGradientDihedralAmber(forceField);
	if(StopCalcul) return;
	calculateGradientImproperTorsion(forceField);
	if(StopCalcul) return;
	calculateGradientNonBondedAmber(forceField);
	if(StopCalcul) return;
	calculateGradientHydrogenBondedAmber(forceField);
	/*
	printf("Before grad pairwise\n");
	for( i=0; i<m->nAtoms;i++) for(j=0;j<3;j++) printf(" i = %d j = %d g = %f\n",i,j,m->gradient[j][i]);
	*/
	if(StopCalcul) return;
	calculateGradientPairWise(forceField);
	if(StopCalcul) return;
	/*
	printf("After grad pairwise\n");
	for( i=0; i<m->nAtoms;i++) for(j=0;j<3;j++) printf(" i = %d j = %d g = %f\n",i,j,m->gradient[j][i]);
	*/
	for( i=0; i<m->nAtoms;i++)
	{
		if(!m->atoms[i].variable)
			for(j=0;j<3;j++)
				m->gradient[j][i] = 0.0;
	}
}
/**********************************************************************/
/*
static void calculateGradientNumericAmber(ForceField* forceField)
{
	gint i;
	gint j;
	Molecule* m = &forceField->molecule;
	gdouble h=0.0001;
	gdouble E1;
	gdouble E2;

	for(j=0;j<3;j++)
		for( i=0; i<m->nAtoms;i++)
		{
    			while( gtk_events_pending() )
        			gtk_main_iteration();
			if(StopCalcul)
				return;
			m->atoms[i].coordinates[j] += h;
			E1 = calculateEnergyTmpAmber(forceField,&m);
			m->atoms[i].coordinates[j] -= h+h;
			E2 = calculateEnergyTmpAmber(forceField,&m);
			m->atoms[i].coordinates[j] += h;
			m->gradient[j][i] = (E1-E2)/2/h;
		}


}
*/
/**********************************************************************/
static gdouble calculateEnergyBondAmber(ForceField* forceField,Molecule* molecule)
{
	gint i;
	gint ai, aj;
	AtomMol atomi,atomj;
	gdouble rijx, rijy, rijz, forceConstant, equilibriumDistance, term;

	Molecule* m = molecule;
	gdouble* bondStretchTerms[STRETCHDIM];
	gint numberOfStretchTerms = forceField->numberOfStretchTerms;
	gdouble energy = 0.0;
	gdouble bondLength;

	for( i=0; i<STRETCHDIM;i++)
       		bondStretchTerms[i] = forceField->bondStretchTerms[i];


#ifdef ENABLE_OMP
#pragma omp parallel for private(i,ai,aj,forceConstant, equilibriumDistance,atomi,atomj,rijx,rijy,rijz,bondLength,term) reduction(+:energy)
#endif
	for (  i = 0; i < numberOfStretchTerms; i++ )
	{
		ai = (gint)bondStretchTerms[0][i];
		aj = (gint)bondStretchTerms[1][i];
		forceConstant = bondStretchTerms[2][i];
		equilibriumDistance = bondStretchTerms[3][i];
		atomi =  m->atoms[ai];
		atomj =  m->atoms[aj];
		

		rijx = atomi.coordinates[0]  - atomj.coordinates[0];	
		rijy = atomi.coordinates[1]  - atomj.coordinates[1];	
		rijz = atomi.coordinates[2]  - atomj.coordinates[2];	

		bondLength = sqrt( rijx * rijx + rijy * rijy + rijz * rijz );
		term = bondLength - equilibriumDistance;

		energy += ( forceConstant ) * term * term;

	} 
	return energy;
}
/**********************************************************************/
static gdouble calculateEnergyBendAmber(ForceField* forceField,Molecule* molecule)
{
	gint i;
	gint ai, aj, ak;
	AtomMol atomi,atomj, atomk;
	gdouble thetaDeg;
	gdouble term;
	gdouble energy = 0.0;
	static gdouble D2RxD2R = 1/( RAD_TO_DEG*RAD_TO_DEG);

	Molecule* m = molecule;
	gdouble* angleBendTerms[BENDDIM];
	gint numberOfBendTerms = forceField->numberOfBendTerms;

	for( i=0; i<BENDDIM;i++)
		angleBendTerms[i] = forceField->angleBendTerms[i]; 

#ifdef ENABLE_OMP
#pragma omp parallel for private(i,ai,aj,ak,atomi,atomj,atomk,thetaDeg,term) reduction(+:energy)
#endif
	for (  i = 0; i < numberOfBendTerms; i++ )
	{
		ai = (gint)angleBendTerms[0][i];
		aj = (gint)angleBendTerms[1][i];
		ak = (gint)angleBendTerms[2][i];
		atomi =  m->atoms[ai];
		atomj =  m->atoms[aj];
		atomk =  m->atoms[ak];

		thetaDeg = getAngle(&atomi, &atomj, &atomk);

		term = thetaDeg - angleBendTerms[4][i];
		term *= term * angleBendTerms[3][i];
		term *= D2RxD2R;

		energy += term;
	
		/*
		printf("f =%f t0 = %f  t= %f e= %f\n",
			angleBendTerms[3][i],
			angleBendTerms[4][i],
			thetaDeg,
			energy);
		*/
	

	} 
	return energy;
}
/**********************************************************************/
static gdouble calculateEnergyDihedralAmber(ForceField* forceField,Molecule* molecule)
{
	gint i;
	gint ai, aj, ak, al;
	AtomMol atomi,atomj, atomk, atoml;
	gdouble phiDeg;
	Molecule* m = molecule;
	gdouble* dihedralAngleTerms[DIHEDRALDIM];
	gint numberOfDihedralTerms = forceField->numberOfDihedralTerms;
	gdouble energy = 0.0;
	static gdouble	D2R = 1.0/57.29577951308232090712;

	for(i=0;i<DIHEDRALDIM;i++)
		dihedralAngleTerms[i] = forceField->dihedralAngleTerms[i];

#ifdef ENABLE_OMP
#pragma omp parallel for private(i,ai,aj,ak,al,atomi,atomj,atomk,atoml,phiDeg) reduction(+:energy)
#endif
	for (  i = 0; i < numberOfDihedralTerms; i++ )
	{
		ai = (gint)dihedralAngleTerms[0][i];
		aj = (gint)dihedralAngleTerms[1][i];
		ak = (gint)dihedralAngleTerms[2][i];
		al = (gint)dihedralAngleTerms[3][i];
		atomi =  m->atoms[ai];
		atomj =  m->atoms[aj];
		atomk =  m->atoms[ak];
		atoml =  m->atoms[al];

		phiDeg = getTorsion(  &atomi ,&atomj, &atomk, &atoml);

		energy += dihedralAngleTerms[5][i]/dihedralAngleTerms[4][i] * 
		( 1.0 + cos( D2R*(dihedralAngleTerms[7][i] * phiDeg - dihedralAngleTerms[6][i] )) );
	}
	return energy;
}
/**********************************************************************/
static gdouble calculateEnergyImproperTorsionAmber(ForceField* forceField,Molecule* molecule)
{
	gdouble energy = 0.0;
	return energy;
}
/**********************************************************************/
static gdouble calculateEnergyfNonBondedAmber(ForceField* forceField,Molecule* molecule)
{
	gint i;
	gint ai, aj;
	AtomMol atomi,atomj;
	gdouble rij2, rij6, rij12, coulombTerm, factorNonBonded;
	gdouble rijx, rijy, rijz;
	gdouble chargei, chargej, Aij, Bij, rij;
	gdouble permittivityScale = 1, permittivity = 1;
	gdouble coulombFactor;
	Molecule* m = molecule;
	gdouble* nonBondedTerms[NONBONDEDDIM];
	gint numberOfNonBonded = forceField->numberOfNonBonded;
	gboolean useCoulomb = forceField->options.coulomb;
	gdouble energy = 0.0;

	for(i=0;i<NONBONDEDDIM;i++)
		nonBondedTerms[i] = forceField->nonBondedTerms[i];

	/* now for non-bonded term */
	coulombFactor = 332.05382 / ( permittivity * permittivityScale );
	/*printf("number of Non Bonded terms = %d\n",numberOfNonBonded);*/
#ifdef ENABLE_OMP
#pragma omp parallel for private(i,ai,aj,Aij,Bij,factorNonBonded,atomi,atomj,chargei,chargej,rijx,rijy,rijz,rij2,rij,rij6,rij12,coulombTerm) reduction(+:energy)
#endif
	for (  i = 0; i < numberOfNonBonded; i++ )
	{
		ai     = (gint)nonBondedTerms[0][i];
		aj     = (gint)nonBondedTerms[1][i];
		Aij    = nonBondedTerms[2][i];
		Bij    = nonBondedTerms[3][i];
		factorNonBonded = nonBondedTerms[4][i];

		atomi =  m->atoms[ai];
		atomj =  m->atoms[aj];

		chargei = atomi.charge;
		chargej = atomj.charge;


		rijx = atomi.coordinates[0] - atomj.coordinates[0];
		rijy = atomi.coordinates[1] - atomj.coordinates[1];
		rijz = atomi.coordinates[2] - atomj.coordinates[2];

		rij2 = rijx * rijx + rijy * rijy + rijz * rijz;
		rij = sqrt( rij2 );
		rij6 = rij2 * rij2 * rij2;
		rij12 = rij6 * rij6;

		if(useCoulomb)
			coulombTerm = ( chargei * chargej * coulombFactor*factorNonBonded ) / rij;
		else
			coulombTerm = 0.0;

		energy += Aij / rij12 - Bij / rij6 + coulombTerm;

		/*
		printf("A =%f B = %f  r= %f e= %f\n",
			Aij,Bij ,rij,energy);
		*/

	}  
	/* printf("Non Bonded energy = %f\n",energy);*/
	return energy;
}
/**********************************************************************/
static gdouble calculateEnergyHydrogenBondedAmber(ForceField* forceField,Molecule* molecule)
{
	gint i;
	gint ai, aj;
	AtomMol atomi,atomj;
	gdouble rij2, rij6, rij12;
	gdouble rijx, rijy, rijz;
	gdouble rij4, rij10;
	gdouble Cij, Dij;
	Molecule* m = molecule;
	gdouble* hydrogenBondedTerms[HYDROGENBONDEDDIM];
	gint numberOfHydrogenBonded =  forceField->numberOfHydrogenBonded;
	gdouble energy = 0.0;

	for(i=0;i<HYDROGENBONDEDDIM;i++)
		hydrogenBondedTerms[i] = forceField->hydrogenBondedTerms[i];

	/* Hydrogen-bonded term */
#ifdef ENABLE_OMP
#pragma omp parallel for private(i,ai,aj,Cij,Dij,atomi,atomj,rijx,rijy,rijz,rij2,rij4,rij6,rij10,rij12) reduction(+:energy)
#endif
	for (  i = 0; i < numberOfHydrogenBonded; i++ )
	{
		ai = (gint)hydrogenBondedTerms[0][i];
		aj = (gint)hydrogenBondedTerms[1][i];
		Cij = hydrogenBondedTerms[2][i];
		Dij = hydrogenBondedTerms[3][i];

		atomi =  m->atoms[ai];
		atomj =  m->atoms[aj];

		rijx = atomi.coordinates[0] - atomj.coordinates[0];
		rijy = atomi.coordinates[1] - atomj.coordinates[1];
		rijz = atomi.coordinates[2] - atomj.coordinates[2];

		rij2 = rijx * rijx + rijy * rijy + rijz * rijz;
		if ( rij2 < 1.0e-2 )
		{
			printf("i = %d j = %d\n",ai,aj);
			rij2 = 1.0e-2;	
		}
		rij4 = rij2 * rij2;
		rij6 = rij4 * rij2;
		rij10 = rij6 * rij4;
		rij12 = rij10 * rij2;

		energy += Cij / rij12 - Dij / rij10;
/*
		printf("C =%f D = %f  r= %f e= %f\n",
			Cij,Dij ,sqrt(rij2),energy);
*/

	}  

	return energy;		
}
/**********************************************************************/
static gdouble calculateEnergyPairWise(ForceField* forceField,Molecule* molecule)
{
	gint i;
	gint ai, aj;
	AtomMol atomi,atomj;
	gdouble rij2, rij6, rij8, rij10;
	gdouble coulombTerm;
	gdouble rijx, rijy, rijz;
	gdouble chargei, chargej, rij;
	gdouble permittivityScale = 1, permittivity = 1;
	gdouble coulombFactor;
	Molecule* m = molecule;
	gdouble* pairWiseTerms[PAIRWISEDIM];
	gint numberOfPairWise = forceField->numberOfPairWise;
	gboolean useCoulomb = forceField->options.coulomb;
	gboolean useVanderWals = forceField->options.vanderWals;
	gdouble energy = 0.0;
	gdouble A, Beta;
	gdouble  B6, B8, B10;
	gdouble c6, c8, c10, b;

	for(i=0;i<PAIRWISEDIM;i++)
		pairWiseTerms[i] = forceField->pairWiseTerms[i];

	/* now for non-bonded term */
	coulombFactor = 332.05382/ ( permittivity * permittivityScale );
	/* printf("number of Non Bonded terms = %d\n",numberOfPairWise);*/
	for (  i = 0; i < numberOfPairWise; i++ )
	{
		ai     = (gint)pairWiseTerms[0][i];
		aj     = (gint)pairWiseTerms[1][i];
		A      = pairWiseTerms[2][i];
		Beta   = pairWiseTerms[3][i];
		c6     = pairWiseTerms[4][i];
		c8     = pairWiseTerms[5][i];
		c10    = pairWiseTerms[6][i];
		b      = pairWiseTerms[7][i];

		atomi =  m->atoms[ai];
		atomj =  m->atoms[aj];

		chargei = atomi.charge;
		chargej = atomj.charge;


		rijx = atomi.coordinates[0] - atomj.coordinates[0];
		rijy = atomi.coordinates[1] - atomj.coordinates[1];
		rijz = atomi.coordinates[2] - atomj.coordinates[2];

		rij2 = rijx * rijx + rijy * rijy + rijz * rijz;
		//if(rij2<1e-2) rij = 1e-2;

		rij = sqrt( rij2 );
		rij6 = rij2 * rij2 * rij2;
		rij8 = rij6* rij2;
		rij10 = rij8 * rij2;

		if(useCoulomb)
			coulombTerm = ( chargei * chargej * coulombFactor ) / rij;
		else
			coulombTerm = 0.0;

		B6  = 0;
		B8  = 0;
		B10 = 0;
		/* printf("A = %f Beta = %f qi = %f qj = %f rij = %f\n",A,Beta,chargei,chargej,rij);*/
		if(useVanderWals)
		{
			gdouble fact = 1.0;
			gdouble s = 1.0;
			gdouble br = b*rij;
			gdouble brk = 1.0;
			gint k;

			if(fabs(c6)>1e-12)
			{
				for(k=1;k<=2*3;k++)
				{
					fact *= k;
					brk *= br;
					s += brk/fact;
				}
				B6 = c6*(1-exp(-br)*s);
			}

			if(fabs(c8)>1e-12)
			{
				fact = 1.0;
				s = 1.0;
				br = b*rij;
				brk = 1.0;
				for(k=1;k<=2*4;k++)
				{
					fact *= k;
					brk *= br;
					s += brk/fact;
				}
				B8 = c8*(1-exp(-br)*s);
			}

			if(fabs(c10)>1e-12)
			{
				fact = 1.0;
				s = 1.0;
				br = b*rij;
				brk = 1.0;
				for(k=1;k<=2*5;k++)
				{
					fact *= k;
					brk *= br;
					s += brk/fact;
				}
				B10 = c10*(1-exp(-br)*s);
			}
		}
					


		energy += A*exp(-Beta*rij)
			- B6 / rij6 
			- B8 / rij8 
			- B10 / rij10 
			+ coulombTerm;
	}  
	return energy;
}

/**********************************************************************/
static void calculateEnergyAmber(ForceField* forceField)
{
	Molecule* m = &forceField->molecule;

	forceField->molecule.energy =  calculateEnergyTmpAmber(forceField,m);
}
/**********************************************************************/
static gdouble calculateEnergyTmpAmber(ForceField* forceField,Molecule* molecule)
{
	gdouble energy = 0.0;

	energy +=calculateEnergyBondAmber(forceField,molecule);
	energy +=calculateEnergyBendAmber(forceField,molecule);
	energy +=calculateEnergyDihedralAmber(forceField,molecule);
	energy +=calculateEnergyImproperTorsionAmber(forceField,molecule);
	energy +=calculateEnergyfNonBondedAmber(forceField,molecule);
	energy +=calculateEnergyHydrogenBondedAmber(forceField,molecule);
	energy +=calculateEnergyPairWise(forceField,molecule);

	return energy;		
}
/**********************************************************************/
ForceField createAmberModel
(GeomDef* geom, gint Natoms,ForceFieldOptions forceFieldOptions)
{
	ForceField forceField = newAmberModel();
	

	forceField.molecule = createMolecule(geom,Natoms,TRUE);
	
	forceField.options = forceFieldOptions;

	set_text_to_draw(_("Setting of Parameters ..."));
	set_statubar_operation_str(_("Setting of Parameters ..."));
	drawGeom();
    	while( gtk_events_pending() )
        	gtk_main_iteration();

	setAmberParameters(&forceField);
	drawGeom();
    	while( gtk_events_pending() )
        	gtk_main_iteration();

	return forceField;
}
/**********************************************************************/
ForceField createPairWiseModel
(GeomDef* geom,gint Natoms,ForceFieldOptions forceFieldOptions)
{
	ForceField forceField = newPairWiseModel();
	

	forceField.molecule = createMolecule(geom,Natoms,TRUE);
	
	forceField.options = forceFieldOptions;

	forceField.options.bondStretch = FALSE;
	forceField.options.angleBend = FALSE;
	forceField.options.dihedralAngle = FALSE;
	forceField.options.improperTorsion = FALSE;
	forceField.options.nonBonded = FALSE;
	forceField.options.hydrogenBonded = FALSE;


	set_text_to_draw(_("Setting of Parameters ..."));
	set_statubar_operation_str(_("Setting of Parameters ..."));
	drawGeom();
    	while( gtk_events_pending() )
        	gtk_main_iteration();

	setAllPairWiseParameters(&forceField);
	drawGeom();
    	while( gtk_events_pending() )
        	gtk_main_iteration();

	return forceField;
}
/**********************************************************************/
void loadAmberParameters()
{
	AmberParameters amberParameters;

	gchar* persoFileName = 
	g_strdup_printf("%s%sPersonalMM.prm",gabedit_directory(), G_DIR_SEPARATOR_S);

	gchar* defaultFileName = 
	g_strdup_printf("%s%sMolecularMechanics.prm",gabedit_directory(), G_DIR_SEPARATOR_S);

	if(staticAmberParameters != NULL)
		freeAmberParameters(staticAmberParameters);

	amberParameters =  newAmberParameters();

	if(!readAmberParameters(&amberParameters,persoFileName))
		if(!readAmberParameters(&amberParameters,defaultFileName))
		{
			createMMFile();
			if(!readAmberParameters(&amberParameters,defaultFileName))
			{
				g_free(persoFileName);
				g_free(defaultFileName);
				return;
			}
		}

	staticAmberParameters = g_malloc(sizeof(AmberParameters));
	*staticAmberParameters = amberParameters;

	g_free(persoFileName);
	g_free(defaultFileName);
}
/**********************************************************************/
void saveAmberParameters()
{
	createPersonalParametersFile(staticAmberParameters);
}
/**********************************************************************/
AmberParameters* getPointerAmberParameters()
{
	return staticAmberParameters;
}
/**********************************************************************/
void setPointerAmberParameters(AmberParameters* ptr)
{
	staticAmberParameters = ptr;
}
/********************************************************************************/
gchar** getListMMTypes(gint* nlist)
{

	gchar** t = NULL;
	
	gint i;

	*nlist = 0;

	if(!staticAmberParameters || staticAmberParameters->numberOfTypes<=0)
		return NULL;
	
	t = g_malloc(staticAmberParameters->numberOfTypes*sizeof(gchar*));
	*nlist = staticAmberParameters->numberOfTypes;

	for(i=0;i<staticAmberParameters->numberOfTypes;i++)
		t[i] = g_strdup(staticAmberParameters->atomTypes[i].name);

	return t;
}
