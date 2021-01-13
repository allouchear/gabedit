/* ConjugateGradient.c */
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
#include "ConjugateGradient.h"


static gdouble maxarg1,maxarg2;
#define FMAX(a,b) (maxarg1=(a),maxarg2=(b),(maxarg1) > (maxarg2) ?\
        (maxarg1) : (maxarg2))

static void hestenesStiefel(ConjugateGradient* conjugateGradient);
static void fletcherReeves(ConjugateGradient* conjugateGradient);
static void polakRibiere(ConjugateGradient* conjugateGradient);
static void wolfPowell(ConjugateGradient* conjugateGradient);
static gdouble lineMinimize(ConjugateGradient* conjugateGradient);
static void bracketMinimum(ConjugateGradient* conjugateGradient, gdouble pointA[], gdouble pointB[], gdouble pointC[] );
static gdouble oneDimensionalEnergy(ConjugateGradient* conjugateGradient, gdouble factor );
static gdouble inverseParabolicInterpolation(ConjugateGradient* conjugateGradient,
		gdouble pointa[], gdouble mid[], gdouble pointb[], 
		gdouble minimum[] );
/**********************************************************************/
void	runConjugateGradient(ConjugateGradient* conjugateGradient, ForceField* forceField, 
		ConjugateGradientOptions conjugateGradientOptions)
{

	gint i;
	gint minimizerOptions = conjugateGradientOptions.method;
	conjugateGradient->forceField = forceField;
	conjugateGradient->numberOfAtoms = forceField->molecule.nAtoms;
	conjugateGradient->updateFrequency = conjugateGradientOptions.updateFrequency;
	conjugateGradient->maxIterations = conjugateGradientOptions.maxIterations;
	conjugateGradient->updateNumber = 0;
	conjugateGradient->maxLine = conjugateGradientOptions.maxLines;
	conjugateGradient->epsilon = conjugateGradientOptions.gradientNorm;
	conjugateGradient->initialStep = conjugateGradientOptions.initialStep;
	conjugateGradient->rmsDeplacment = 0;
	conjugateGradient->maxDeplacment = 0;
	conjugateGradient->gradientNorm = 0;
	conjugateGradient->initialBracket = conjugateGradientOptions.initialStep;
	conjugateGradient->lastInitialBracket = 0;
	conjugateGradient->term = 0;

	for(i=0;i<3;i++)
	{
		conjugateGradient->lastGradient[i] = 
			g_malloc(conjugateGradient->numberOfAtoms*sizeof(gdouble));
		conjugateGradient->direction[i] = 
			g_malloc(conjugateGradient->numberOfAtoms*sizeof(gdouble));
	}

	conjugateGradient->temporaryMolecule = g_malloc(sizeof(Molecule));
	conjugateGradient->temporaryMolecule->nAtoms = conjugateGradient->numberOfAtoms;
	conjugateGradient->temporaryMolecule->atoms = 
		g_malloc(conjugateGradient->numberOfAtoms*sizeof(AtomMol)); 
	for (  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
	{
		conjugateGradient->temporaryMolecule->atoms[i].charge =  
			conjugateGradient->forceField->molecule.atoms[i].charge;
	}

	switch(minimizerOptions)
	{
		case 1 : hestenesStiefel(conjugateGradient);break;
		case 2 : fletcherReeves(conjugateGradient);break;
		case 3 : polakRibiere(conjugateGradient);break;
		case 4 : wolfPowell(conjugateGradient);break;
	}
}
/**********************************************************************/
void	freeConjugateGradient(ConjugateGradient* conjugateGradient)
{

	gint i;
	conjugateGradient->forceField = NULL;
	conjugateGradient->numberOfAtoms = 0;
	conjugateGradient->updateFrequency = 0;
	conjugateGradient->maxIterations = 0;
	conjugateGradient->updateNumber = 0;
	conjugateGradient->maxLine = 0;
	conjugateGradient->epsilon = 0;
	conjugateGradient->initialStep = 0;
	conjugateGradient->rmsDeplacment = 0;
	conjugateGradient->maxDeplacment = 0;
	conjugateGradient->gradientNorm = 0;
	conjugateGradient->initialBracket = 0;
	conjugateGradient->lastInitialBracket = 0;
	conjugateGradient->term = 0;

	for(i=0;i<3;i++)
	{
		if(conjugateGradient->lastGradient[i] !=NULL)
		{
			g_free(conjugateGradient->lastGradient[i]);
			conjugateGradient->lastGradient[i]= NULL;
		}
		if(conjugateGradient->direction[i] != NULL)
		{
			g_free(conjugateGradient->direction[i]);
			conjugateGradient->direction[i] = NULL; 
		}
	}

	if(conjugateGradient->temporaryMolecule != NULL)
	{
		g_free(conjugateGradient->temporaryMolecule);
		conjugateGradient->temporaryMolecule = NULL;
	}

}
/**********************************************************************/
static void fletcherReeves(ConjugateGradient* conjugateGradient)
{
	gdouble lastGradientDotGradient = 0, gradientDotGradient = 0, beta;
	gint iterations = 0;
	gint i;
	gint j;
	gdouble energy;
	gchar* str = g_strdup(" ");

	conjugateGradient->forceField->klass->calculateGradient(conjugateGradient->forceField);
	for ( i = 0; i < conjugateGradient->numberOfAtoms; i++ )
	{
		for(j=0;j<3;j++)
		{
		conjugateGradient->direction[j][ i ] = 
			-conjugateGradient->forceField->molecule.gradient[j][i];
		lastGradientDotGradient += conjugateGradient->forceField->molecule.gradient[j][i] 
					* conjugateGradient->forceField->molecule.gradient[j][i]; 
		}
	}
	gradientDotGradient = lastGradientDotGradient;
	while ( 
			( lastGradientDotGradient > conjugateGradient->epsilon ) && 
			( iterations++ < conjugateGradient->maxIterations ) 
		)
	{
		if(StopCalcul)
			break;

		lineMinimize(conjugateGradient);
		conjugateGradient->forceField->klass->calculateGradient(conjugateGradient->forceField);
		lastGradientDotGradient = gradientDotGradient;	
		gradientDotGradient = 0;	
		for (  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
			for(j=0;j<3;j++)
			{
				gradientDotGradient += 
					conjugateGradient->forceField->molecule.gradient[j][i] 
					* conjugateGradient->forceField->molecule.gradient[j][i]; 
			}
		beta = gradientDotGradient / lastGradientDotGradient;
		for (  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
		{
			for(j=0;j<3;j++)
			{
				conjugateGradient->direction[j][i] = 
					beta *  conjugateGradient->direction[j][i] - 
					conjugateGradient->forceField->molecule.gradient[j][i];
			}
		}
		if ( conjugateGradient->updateNumber >= conjugateGradient->updateFrequency )
		{
			g_free(str);
			str = g_strdup_printf(_("gradient = %f "),sqrt(gradientDotGradient)); 
			redrawMolecule(&conjugateGradient->forceField->molecule,str);
			conjugateGradient->updateNumber = 0;
		}
		conjugateGradient->updateNumber++;
	}	

	conjugateGradient->forceField->klass->calculateGradient(conjugateGradient->forceField);
	conjugateGradient->gradientNorm = 0;
	for(  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
	{
		for(j=0;j<3;j++)
		{
			conjugateGradient->gradientNorm += 
				conjugateGradient->forceField->molecule.gradient[j][i] *
				conjugateGradient->forceField->molecule.gradient[j][i]; 
		}
	}
	conjugateGradient->gradientNorm = sqrt(conjugateGradient->gradientNorm );
	energy = conjugateGradient->forceField->klass->calculateEnergyTmp
		(conjugateGradient->forceField, &conjugateGradient->forceField->molecule );

	g_free(str);
	str = g_strdup_printf(_("Gradient = %f  Energy = %f (Kcal/mol) "),
			(gdouble)conjugateGradient->gradientNorm,(gdouble)energy); 

	redrawMolecule(&conjugateGradient->forceField->molecule,str);
	g_free(str);
}
/********************************************************************************/
static void polakRibiere(ConjugateGradient* conjugateGradient)
{
	gdouble lastGradientDotGradient = 0, gradientDotGradient, beta;
	gint iterations = 0;
	gdouble energy;
	gint i;
	gint j;
	gchar* str = g_strdup(" ");

	conjugateGradient->forceField->klass->calculateGradient(conjugateGradient->forceField);

	for ( i = 0; i < conjugateGradient->numberOfAtoms; i++ )
	{
		for(j=0;j<3;j++)
		{
		conjugateGradient->direction[j][ i ] = 
		conjugateGradient->lastGradient[j][i] =
			-conjugateGradient->forceField->molecule.gradient[j][i];

		lastGradientDotGradient += conjugateGradient->forceField->molecule.gradient[j][i] 
					* conjugateGradient->forceField->molecule.gradient[j][i]; 
		}
	}
	gradientDotGradient = lastGradientDotGradient;
	while ( 
			( lastGradientDotGradient > conjugateGradient->epsilon ) && 
			( iterations++ < conjugateGradient->maxIterations ) 
		)
	{
		if(StopCalcul)
			break;

		lineMinimize(conjugateGradient);
		conjugateGradient->forceField->klass->calculateGradient(conjugateGradient->forceField);
		lastGradientDotGradient = gradientDotGradient;
		gradientDotGradient = 0;	
		beta = 0;
		for (  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
		{
			for (  j = 0; j < 3; j++ )
			{
				gradientDotGradient += 
					conjugateGradient->forceField->molecule.gradient[j][i] *
					conjugateGradient->forceField->molecule.gradient[j][i]; 
				
				beta += ( 
					conjugateGradient->forceField->molecule.gradient[j][i] - 
					conjugateGradient->lastGradient[j][i]
					)* conjugateGradient->forceField->molecule.gradient[j][i]; 
			}
		}
		beta /= lastGradientDotGradient;
		for (  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
		{
			for (  j = 0; j < 3; j++ )
			{
				conjugateGradient->direction[j][i] = 
				beta * conjugateGradient->direction[j][i] - 
				conjugateGradient->forceField->molecule.gradient[j][i];
			}
		}
		for ( i = 0; i < conjugateGradient->numberOfAtoms; i++ )
		{
			for(j=0;j<3;j++)
			{
				conjugateGradient->lastGradient[j][i] = 
				conjugateGradient->forceField->molecule.gradient[j][i];
			}
		}
		if ( conjugateGradient->updateNumber >= conjugateGradient->updateFrequency )
		{
			g_free(str);
			str = g_strdup_printf("gradient = %f ",sqrt(gradientDotGradient)); 
			redrawMolecule(&conjugateGradient->forceField->molecule,str);
			conjugateGradient->updateNumber = 0;
		}
		conjugateGradient->updateNumber++;
	}	
	conjugateGradient->forceField->klass->calculateGradient(conjugateGradient->forceField);
	conjugateGradient->gradientNorm = 0;
	for(  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
	{
		for (  j = 0; j < 3; j++ )
		{
			conjugateGradient->gradientNorm += 
				conjugateGradient->forceField->molecule.gradient[j][i] *
				conjugateGradient->forceField->molecule.gradient[j][i]; 
		}
	}
	conjugateGradient->gradientNorm = sqrt(conjugateGradient->gradientNorm );
	energy = conjugateGradient->forceField->klass->calculateEnergyTmp
		(conjugateGradient->forceField, &conjugateGradient->forceField->molecule );

	g_free(str);
	str = g_strdup_printf(_("Gradient = %f  Energy = %f (Kcal/mol) "),
			(gdouble)conjugateGradient->gradientNorm,(gdouble)energy); 
	redrawMolecule(&conjugateGradient->forceField->molecule,str);
	g_free(str);
}
/********************************************************************************/
static void hestenesStiefel(ConjugateGradient* conjugateGradient)
{
	gdouble gradientDotGradient = 1, beta, gradientDiff, denom;
	gint iterations = 0;
	gdouble energy;
	gint i;
	gint j;
	gchar* str = g_strdup(" ");

	conjugateGradient->forceField->klass->calculateGradient(conjugateGradient->forceField);
	for ( i = 0; i < conjugateGradient->numberOfAtoms; i++ )
	{
		for(j=0;j<3;j++)
		{
		conjugateGradient->direction[j][ i ] = 
		conjugateGradient->lastGradient[j][i] = 
			-conjugateGradient->forceField->molecule.gradient[j][i];
		}
	}
	while ( 
			( gradientDotGradient > conjugateGradient->epsilon ) && 
			( iterations++ < conjugateGradient->maxIterations ) 
		)
	{
		if(StopCalcul)
			break;

		lineMinimize(conjugateGradient);
		conjugateGradient->forceField->klass->calculateGradient(conjugateGradient->forceField);
		gradientDotGradient = 0;	
		for (  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
		{
			for (  j = 0; j < 3; j++ )
			{
				gradientDotGradient += 
					conjugateGradient->forceField->molecule.gradient[j][i]*
					conjugateGradient->forceField->molecule.gradient[j][i];
			}
		}
		beta = 0;
		denom = 0;
		for (  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
		{
			for (  j = 0; j < 3; j++ )
			{
				gradientDiff = 
					conjugateGradient->forceField->molecule.gradient[j][i]-
					conjugateGradient->lastGradient[j][i];

				beta += 
					conjugateGradient->forceField->molecule.gradient[j][i]*
				       	gradientDiff;
				denom += conjugateGradient->direction[j][i] * gradientDiff;
			}
		}
		if ( fabs( denom ) > 1.0e-10 )
			beta /= denom;
		else
			beta = 0;

		for (  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
		{
			for (  j = 0; j < 3; j++ )
			{
				conjugateGradient->direction[j][ i ] = 
					beta * conjugateGradient->direction[j][ i ] - 
					conjugateGradient->forceField->molecule.gradient[j][i];
			}
		}
		for ( i = 0; i < conjugateGradient->numberOfAtoms; i++ )
		{
			for(j=0;j<3;j++)
			{
				conjugateGradient->lastGradient[j][i] = 
				conjugateGradient->forceField->molecule.gradient[j][i];
			}
		}
		if ( conjugateGradient->updateNumber >= conjugateGradient->updateFrequency )
		{
			g_free(str);
			str = g_strdup_printf(_("gradient = %f "),sqrt(gradientDotGradient)); 
			redrawMolecule(&conjugateGradient->forceField->molecule,str);
			conjugateGradient->updateNumber = 0;
		}
		conjugateGradient->updateNumber++;
	}	
	conjugateGradient->forceField->klass->calculateGradient(conjugateGradient->forceField);
	conjugateGradient->gradientNorm = 0;
	for(  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
	{
		for(j=0;j<3;j++)
			conjugateGradient->gradientNorm += 
			conjugateGradient->direction[j][ i ] *
			conjugateGradient->direction[j][ i ];
	}
	conjugateGradient->gradientNorm = sqrt( conjugateGradient->gradientNorm );
	energy = conjugateGradient->forceField->klass->calculateEnergyTmp
		(conjugateGradient->forceField, &conjugateGradient->forceField->molecule );

	g_free(str);
	str = g_strdup_printf(_("Gradient = %f  Energy = %f (Kcal/mol) "),
			(gdouble)conjugateGradient->gradientNorm,(gdouble)energy); 
	redrawMolecule(&conjugateGradient->forceField->molecule,str);
	g_free(str);
}
/********************************************************************************/
static void wolfPowell(ConjugateGradient* conjugateGradient)
{
	gdouble lastGradientDotGradient = 0, gradientDotGradient, beta;
	gint iterations = 0;
	gdouble energy;
	gint i;
	gint j;
	gchar* str = g_strdup(" ");

	conjugateGradient->forceField->klass->calculateGradient(conjugateGradient->forceField);
	for ( i = 0; i < conjugateGradient->numberOfAtoms; i++ )
	{
		for(j=0;j<3;j++)
		{
		conjugateGradient->direction[j][ i ] = 
		conjugateGradient->lastGradient[j][i] =
			-conjugateGradient->forceField->molecule.gradient[j][i];

		lastGradientDotGradient += conjugateGradient->forceField->molecule.gradient[j][i] 
					* conjugateGradient->forceField->molecule.gradient[j][i]; 
		}
	}
	while ( 
			( lastGradientDotGradient > conjugateGradient->epsilon ) && 
			( iterations++ < conjugateGradient->maxIterations ) 
		)
	{
		if(StopCalcul)
			break;

		lineMinimize(conjugateGradient);
		conjugateGradient->forceField->klass->calculateGradient(conjugateGradient->forceField);
		gradientDotGradient = 0;	
		for ( i = 0; i < conjugateGradient->numberOfAtoms; i++ )
			for ( j = 0; j < 3; j++ )
			{
				gradientDotGradient += 
					conjugateGradient->forceField->molecule.gradient[j][i]*
					conjugateGradient->forceField->molecule.gradient[j][i];
			}

		beta = 0;
		for (  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
		{
			for ( j = 0; j < 3; j++ )
				beta += ( 
					conjugateGradient->forceField->molecule.gradient[j][i] - 
					conjugateGradient->lastGradient[j][i]
					)* conjugateGradient->forceField->molecule.gradient[j][i]; 
		}
		beta /= lastGradientDotGradient;
		if ( beta < 0 )
			beta = 0;
		lastGradientDotGradient = gradientDotGradient;
		for (  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
			for ( j = 0; j < 3; j++ )
				conjugateGradient->direction[j][ i ] = 
					beta * conjugateGradient->direction[j][ i ] - 
					conjugateGradient->forceField->molecule.gradient[j][i];
		
		for ( i = 0; i < conjugateGradient->numberOfAtoms; i++ )
		{
			for(j=0;j<3;j++)
			{
				conjugateGradient->lastGradient[j][i] = 
				conjugateGradient->forceField->molecule.gradient[j][i];
			}
		}
		if ( conjugateGradient->updateNumber >= conjugateGradient->updateFrequency )
		{
			g_free(str);
			str = g_strdup_printf(_("gradient = %f "),sqrt(gradientDotGradient)); 
			redrawMolecule(&conjugateGradient->forceField->molecule,str);
			conjugateGradient->updateNumber = 0;
		}
		conjugateGradient->updateNumber++;
	}	
	conjugateGradient->forceField->klass->calculateGradient(conjugateGradient->forceField);
	conjugateGradient->gradientNorm = 0;
	for(  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
	{
		for(j=0;j<3;j++)
			conjugateGradient->gradientNorm += 
			conjugateGradient->direction[j][ i ] *
			conjugateGradient->direction[j][ i ];
	}
	conjugateGradient->gradientNorm = sqrt( conjugateGradient->gradientNorm );
	energy = conjugateGradient->forceField->klass->calculateEnergyTmp
		(conjugateGradient->forceField, &conjugateGradient->forceField->molecule );

	g_free(str);
	str = g_strdup_printf(_("Gradient = %f  Energy = %f (Kcal/mol) "),
			(gdouble)conjugateGradient->gradientNorm,(gdouble)energy); 
	redrawMolecule(&conjugateGradient->forceField->molecule,str);
	g_free(str);
} 
/**********************************************************************/
static gdouble lineMinimize(ConjugateGradient* conjugateGradient)
{
	gdouble a;
	gdouble b;
	gdouble c;
	gdouble minimum=0;
	gdouble energy;
        gdouble delta = 1.0e-7;
	gint i;
	gint j;
		
	a = 0; 
	b = conjugateGradient->initialBracket;
	bracketMinimum(conjugateGradient, &a, &b, &c );
	energy = inverseParabolicInterpolation(conjugateGradient, &a, &b, &c, &minimum );
	conjugateGradient->initialBracket = minimum;

	if ( 	( fabs( conjugateGradient->initialBracket ) < delta ) || 
		( conjugateGradient->initialBracket == conjugateGradient->lastInitialBracket ) )
	{
		conjugateGradient->initialBracket = 
			rand()/(gdouble)RAND_MAX *conjugateGradient->initialStep;

		for (  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
		{
			for(j=0;j<3;j++)
			conjugateGradient->direction[j][i] = 
				-conjugateGradient->forceField->molecule.gradient[j][i];
		}
	} 
	conjugateGradient->lastInitialBracket = conjugateGradient->initialBracket;
	for (  i = 0; i <  conjugateGradient->numberOfAtoms; i++ )
	{

			for(j=0;j<3;j++)
				conjugateGradient->forceField->molecule.atoms[i].coordinates[j] += 
				conjugateGradient->direction[j][i] * conjugateGradient->initialBracket;
	}
	return( energy );
}
/********************************************************************************/
static void bracketMinimum(ConjugateGradient* conjugateGradient, gdouble pointA[], gdouble pointB[], gdouble pointC[] )
{
	static gdouble GOLDENRATIO = 1.618034;
	gdouble energyA, energyB, energyC, temp;
      	gdouble ulim, u, r, q, fu, denominator;
	gint iter = 0;

       	energyA = oneDimensionalEnergy(conjugateGradient, pointA[ 0 ] );
        energyB = oneDimensionalEnergy(conjugateGradient, pointB[ 0 ] );

        if ( energyB > energyA ) 
	{  
		temp = pointA[ 0 ];
		pointA[ 0 ] = pointB[ 0 ];
		pointB[ 0 ] = temp;
		temp = energyB;
		energyB = energyA;
		energyA = temp;  
        }
        pointC[ 0 ] = pointB[ 0 ] + GOLDENRATIO * ( pointB[ 0 ] - pointA[ 0 ] );
        energyC = oneDimensionalEnergy(conjugateGradient, pointC[ 0 ] );
        while ( energyB > energyC )
	{
		iter++;
               	r = ( pointB[ 0 ] - pointA[ 0 ] ) * ( energyB - energyC );
               	q = ( pointB[ 0 ] - pointC[ 0 ] ) * ( energyB - energyA );
		denominator = FMAX( fabs( q - r ), 1.0e-20 );
		if ( ( q - r ) < 0 )
			denominator = -denominator;
               	u = ( pointB[ 0 ] ) - ( ( pointB[ 0 ]-pointC[ 0 ] ) * q - 
			( pointB[ 0 ] - pointA[ 0 ] ) * r ) /
                       	( 2.0 * denominator );
               	ulim = pointB[ 0 ] + 100 * ( pointC[ 0 ] - pointB[ 0 ] );
               	if ( ( pointB[ 0 ] - u ) * ( u - pointC[ 0 ] ) > 0.0 )
		{
                       	fu=oneDimensionalEnergy(conjugateGradient, u );
                       	if ( fu < energyC )
			{
                               	pointA[ 0 ] = pointB[ 0 ];
                               	pointB[ 0 ] = u;
                               	energyA = energyB;
                               	energyB = fu;
                               	return;
                       	}
			else if ( fu > energyB )
			{
                               	pointC[ 0 ] = u;
                               	energyC = fu;
                               	return;
			}
                       	u = pointC[ 0 ] + GOLDENRATIO * ( pointC[ 0 ] - pointB[ 0 ] );
                       	fu = oneDimensionalEnergy(conjugateGradient, u );
               	}
		else if ( ( pointC[ 0 ] - u ) * ( u - ulim ) > 0.0 )
		{
                       	fu = oneDimensionalEnergy(conjugateGradient, u );
                       	if ( fu < energyC )
			{
				pointB[ 0 ] = pointC[ 0 ];
				pointC[ 0 ] = u;
				u = pointC[ 0 ] + GOLDENRATIO * ( pointC[ 0 ] - pointB[ 0 ] );
				energyB = energyC;
				energyC = fu;
				fu = oneDimensionalEnergy(conjugateGradient, u );
                       	}
               	}
		else if ( ( u - ulim ) * ( ulim - pointC[ 0 ] ) >= 0.0 )
		{
                       	u = ulim;
                       	fu = oneDimensionalEnergy(conjugateGradient, u );
               	}
		else
		{
                       	u = pointC[ 0 ] + GOLDENRATIO * ( pointC[ 0 ] - pointB[ 0 ] );
                       	fu = oneDimensionalEnergy(conjugateGradient, u );
               	}
		pointA[ 0 ] = pointB[ 0 ];
		pointB[ 0 ] = pointC[ 0 ];
		pointC[ 0 ] = u;
		energyA = energyB;
		energyB = energyC;
		energyC = fu;
       	}
}
/********************************************************************************/
static gdouble oneDimensionalEnergy(ConjugateGradient* conjugateGradient, gdouble factor )
{

	gint i;
	gint j;
	for (  i = 0; i < conjugateGradient->numberOfAtoms; i++ )
	{
		for(j=0;j<3;j++)
		{
			conjugateGradient->temporaryMolecule->atoms[i].coordinates[j] = 
				conjugateGradient->forceField->molecule.atoms[i].coordinates[j] + 
				factor * conjugateGradient->direction[j][i];
		}
	}
	
	return ( conjugateGradient->forceField->klass->calculateEnergyTmp(conjugateGradient->forceField,conjugateGradient->temporaryMolecule) );
}
/********************************************************************************/
static gdouble inverseParabolicInterpolation(ConjugateGradient* conjugateGradient,
		gdouble pointa[], gdouble mid[], gdouble pointb[], 
		gdouble minimum[] )
{
        gint iter;
	gint maxIterations = conjugateGradient->maxLine;
	static double CGOLD = 0.3819660;
        gdouble a,b,d=0,etemp,fu,fv,fw,fx,p,q,r,tol1,tol2,u,v,w,x,xm;
        gdouble e=0.0, tol=2.0e-4;
	gdouble pointA, pointB, midAB;
	gdouble energy;
	
	pointA = pointa[ 0 ];
	pointB = pointb[ 0 ];
	midAB = mid[ 0 ];	

        a=(pointA < pointB ? pointA : pointB);
        b=(pointA > pointB ? pointA : pointB);
        x=w=v=mid[ 0 ];
        fw=fv=fx=fu=energy=oneDimensionalEnergy(conjugateGradient,x);
        for (iter=1;iter<=maxIterations;iter++)
	{
		if(StopCalcul)
			break; 

               	xm=0.5*(a+b);
		tol1 = tol* fabs( x ) + 1.0e-10;
               	tol2=2.0*tol1;
               	if (fabs(x-xm) <= (tol2-0.5*(b-a)))
		{
                       	minimum[0]=x;
                       	return fu;
               	}
               	if (fabs(e) > tol1)
		{
                       	r=(x-w)*(fx-fv);
                       	q=(x-v)*(fx-fw);
                       	p=(x-v)*q-(x-w)*r;
                       	q=2.0*(q-r);
                       	if (q > 0.0) p = -p;
                       	q=fabs(q);
                       	etemp=e;
                       	e=d;
                       	if (fabs(p) >= fabs(0.5*q*etemp) || 
				p <= q*(a-x) || p >= q*(b-x))
                               	d=CGOLD*(e=(x >= xm ? a-x : b-x));
                       	else
			{
                               	d=p/q;
                               	u=x+d;
                               	if (u-a < tol2 || b-u < tol2)
				{
					if ( ( xm - x ) < 0 )
						d = - tol1;
					else
						d = tol1;
				}
                       	}
               	}
		else
		{
                        	d=CGOLD*(e=(x >= xm ? a-x : b-x));
               	}
		if ( fabs( d ) >= tol1 )
		{
			u = x + d;
		}
		else
		{
			if ( d >= 0 )
				u = x + tol1;
			else
				u = x - tol1;
		}
               	fu=oneDimensionalEnergy(conjugateGradient,u);
               	if (fu <= fx)
		{
                       	if (u >= x) a=x; else b=x;
			v = w;
			w = x;
			x = u;
			fv = fw;
			fw = fx;
			fx = fu;
               	}
		else
		{
                       	if (u < x) a=u; else b=u;
                       	if (fu <= fw || w == x)
			{
                               	v=w;
                               	w=u;
                               	fv=fw;
                               	fw=fu;
                       	}
			else if (fu <= fv || v == x || v == w)
			{
                               	v=u;
                               	fv=fu;
                       	}
               	}
        }
	return( fu );
}
