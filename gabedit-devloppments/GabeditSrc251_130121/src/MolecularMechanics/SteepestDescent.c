/* SteepestDescent.c  */
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
#include "SteepestDescent.h"


static void Minimize(SteepestDescent* steepestDescent);
/**********************************************************************/
void	runSteepestDescent(
		SteepestDescent* steepestDescent, ForceField* forceField, 
		gint updateFrequency, gint maxIterations, gdouble epsilon,
		gint maxLines
		)
{

	steepestDescent->forceField = forceField;
	steepestDescent->numberOfAtoms = forceField->molecule.nAtoms;
	steepestDescent->updateFrequency = updateFrequency;
	steepestDescent->maxIterations = maxIterations;
	steepestDescent->updateNumber = 0;
	steepestDescent->epsilon = epsilon;
	steepestDescent->rmsDeplacment = 0;
	steepestDescent->maxDeplacment = 0;
	steepestDescent->gradientNorm = 0;
	steepestDescent->maxLines=maxLines;

	Minimize(steepestDescent);
}
/**********************************************************************/
void	freeSteepestDescent(SteepestDescent* steepestDescent)
{

	steepestDescent->forceField = NULL;
	steepestDescent->numberOfAtoms = 0;
	steepestDescent->updateFrequency = 0;
	steepestDescent->maxIterations = 0;
	steepestDescent->updateNumber = 0;
	steepestDescent->epsilon = 0;
	steepestDescent->rmsDeplacment = 0;
	steepestDescent->maxDeplacment = 0;
	steepestDescent->gradientNorm = 0;
	steepestDescent->maxLines=0;

}
/**********************************************************************/
static void Minimize(SteepestDescent* steepestDescent)
{
	gdouble energy;
	gint iteration = 0;
	gdouble lastGradientNorm = 1;
	gdouble term = 1;
	gchar* str = g_strdup(" ");
	gint i;
	gint j;
	gdouble f0,f1;
	gint ii;
	gdouble fg;

	steepestDescent->updateNumber = 0;

	steepestDescent->forceField->klass->calculateGradient(steepestDescent->forceField);

	steepestDescent->gradientNorm = 0;
	for (  i = 0; i < steepestDescent->numberOfAtoms; i++ )
		for(j=0;j<3;j++)
			steepestDescent->gradientNorm += 
				steepestDescent->forceField->molecule.gradient[j][i] *
				steepestDescent->forceField->molecule.gradient[j][i]; 

	lastGradientNorm = sqrt( steepestDescent->gradientNorm );

	while( 
			( lastGradientNorm > steepestDescent->epsilon ) && 
			( iteration++ < steepestDescent->maxIterations )
	     )
	{
		if(StopCalcul)
			break;

		steepestDescent->forceField->klass->calculateGradient(steepestDescent->forceField);
		steepestDescent->gradientNorm = 0;
		for (  i = 0; i < steepestDescent->numberOfAtoms; i++ )
			for(j=0;j<3;j++)
				steepestDescent->gradientNorm += 
					steepestDescent->forceField->molecule.gradient[j][i] *
					steepestDescent->forceField->molecule.gradient[j][i]; 

		steepestDescent->gradientNorm = sqrt( steepestDescent->gradientNorm );
		
	
		if(steepestDescent->gradientNorm<1e-12)
			break;

        	f0 = steepestDescent->forceField->klass->calculateEnergyTmp(
		      steepestDescent->forceField,&steepestDescent->forceField->molecule);
		term = 0;
		fg = 1.0;
		if(steepestDescent->gradientNorm>1)
			fg = 1.0/steepestDescent->gradientNorm;

		for(ii=steepestDescent->maxLines;ii>=1;ii--)
		{
    			while( gtk_events_pending() )
        			gtk_main_iteration();
			if(StopCalcul)
				break;
			term = ii*0.01;
			for (  i = 0; i < steepestDescent->numberOfAtoms; i++ )
			{
				for(j=0;j<3;j++)
					steepestDescent->forceField->molecule.atoms[i].coordinates[j]-=
					fg*term*steepestDescent->forceField->molecule.gradient[j][i]; 
			}

        		f1 = steepestDescent->forceField->klass->calculateEnergyTmp(
		      	steepestDescent->forceField,&steepestDescent->forceField->molecule);
			if(f1<f0)
				break;
			for (  i = 0; i < steepestDescent->numberOfAtoms; i++ )
			{
				for(j=0;j<3;j++)
					steepestDescent->forceField->molecule.atoms[i].coordinates[ j ] += 
					fg*term*steepestDescent->forceField->molecule.gradient[j][i]; 
			}
		}
		if(ii<=1)
			break;
		lastGradientNorm = steepestDescent->gradientNorm;

		if ( steepestDescent->updateNumber++ >= steepestDescent->updateFrequency )
		{
			g_free(str);
			str = g_strdup_printf(_("Gradient = %f "),(gdouble)steepestDescent->gradientNorm); 
			redrawMolecule(&steepestDescent->forceField->molecule,str);
			steepestDescent->updateNumber = 0;
		}
	}

	steepestDescent->forceField->klass->calculateGradient(steepestDescent->forceField);
	steepestDescent->gradientNorm = 0;
	for (  i = 0; i < steepestDescent->numberOfAtoms; i++ )
		for(j=0;j<3;j++)
			steepestDescent->gradientNorm += 
				steepestDescent->forceField->molecule.gradient[j][i] *
				steepestDescent->forceField->molecule.gradient[j][i]; 

	steepestDescent->gradientNorm = sqrt( steepestDescent->gradientNorm );
        energy = steepestDescent->forceField->klass->calculateEnergyTmp(
		      steepestDescent->forceField,&steepestDescent->forceField->molecule);
	g_free(str);
	str = g_strdup_printf(_("Gradient = %f  Energy = %f (Kcal/mol)"),
			(gdouble)steepestDescent->gradientNorm,(gdouble)energy); 

	redrawMolecule(&steepestDescent->forceField->molecule,str);
	g_free(str);
}
/********************************************************************************/
