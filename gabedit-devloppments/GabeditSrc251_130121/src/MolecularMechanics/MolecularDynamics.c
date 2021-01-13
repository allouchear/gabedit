/* MolecularDynamics.c  */
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
#include <time.h>

#include "../Common/Global.h"
#include "../Utils/AtomsProp.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Utils/Utils.h"
#include "Atom.h"
#include "Molecule.h"
#include "ForceField.h"
#include "MolecularDynamics.h"


/*********************************************************************************/
static void initMD(MolecularDynamics* molecularDynamics, gdouble temperature, gdouble stepSize, MDIntegratorType integratorType, MDThermostatType thermostat, gdouble friction, gdouble collide, gchar* fileNameTraj, gchar* fileNameProp, gint numberOfRunSteps);
static void berendsen(MolecularDynamics* molecularDynamics);
static void andersen(MolecularDynamics* molecularDynamics);
static void bussi(MolecularDynamics* molecularDynamics);
static void rescaleVelocities(MolecularDynamics* molecularDynamics);
static void computeEnergies(MolecularDynamics* molecularDynamics);
static void applyOneStep(MolecularDynamics* molecularDynamics);
static void applyVerlet(MolecularDynamics* molecularDynamics);
static void applyBeeman(MolecularDynamics* molecularDynamics);
static void applyStochastic(MolecularDynamics* molecularDynamics);
static gdouble	maxwel(gdouble masse, gdouble temperature);
static void newProperties(MolecularDynamics* molecularDynamics, gchar* comments);
static void saveProperties(MolecularDynamics* molecularDynamics, gint iStep0, gint iStep, gchar* comments);
static void saveTrajectory(MolecularDynamics* molecularDynamics, gint iStep);
static void getRandVect(gdouble len, gdouble V[]);
static gdouble getEKin(MolecularDynamics* molecularDynamics);
static gdouble getKelvin(MolecularDynamics* molecularDynamics);
static gdouble drandom();
static gdouble normal();
/**********************************************************************/
ForceField**    runMolecularDynamicsConfo(
		MolecularDynamics* molecularDynamics, ForceField* forceField, 
		gint updateFrequency, 
		gdouble heatTime, gdouble equiTime, gdouble runTime,
		gdouble heatTemperature, gdouble equiTemperature, gdouble runTemperature,
		gdouble stepSize,
		MDIntegratorType integratorType,
		MDThermostatType thermostat,
		gdouble friction,
		gdouble collide,
		gint numberOfGeometries,
		gchar* fileNameTraj,
		gchar* fileNameProp
		)
{
	gint i;
	gint j;
	gchar* str = NULL;
        gdouble gradientNorm = 0;
	gint numberOfHeatSteps = 0;
	gint numberOfEquiSteps = 0;
	gint numberOfRunSteps = 0;
	gdouble currentTemp;
	gint updateNumber = 0;
	gint n0 = 0;
	ForceField** geometries = NULL;
	gint iSel = 0;
	gint stepSel = 1;
	/* 
	 *  physical constants in SI units
	 *   ------------------------------
	 *      Kb = 1.380662 E-23 J/K
	 *      Na = 6.022045 E23  1/mol
	 *      e = 1.6021892 E-19 C
	 *      eps = 8.85418782 E-12 F/m
	 *                       
	 *      1 Kcal = 4184.0 J
	 *      1 amu = 1.6605655 E-27 Kg
	 *      1 A = 1.0 E-10 m
	 *                                       
	 *       Internally, AKMA units are used:
	 *                                        
	 *       timeFactor = SQRT ( ( 1A )**2 * 1amu * Na  / 1Kcal )
	 *       kBoltzmann = Na *Kb  / 1 Kcal
	*/ 

	/* printf("basname = %s\n",g_path_get_basename(fileNameTraj));*/

	if(forceField->molecule.nAtoms<1) return NULL;
	if(numberOfGeometries<2) return NULL;
	geometries = g_malloc(numberOfGeometries*sizeof(ForceField*));
	for(i=0;i<numberOfGeometries;i++) geometries[i] = NULL;

	molecularDynamics->forceField = forceField;
	molecularDynamics->numberOfAtoms = forceField->molecule.nAtoms;
	molecularDynamics->updateFrequency = updateFrequency;

	currentTemp = heatTemperature/2;
	
	numberOfHeatSteps = heatTime/stepSize*1000;
	numberOfEquiSteps = equiTime/stepSize*1000;; 
	numberOfRunSteps = runTime/stepSize*1000;; 


	currentTemp = heatTemperature;
	if(numberOfHeatSteps==0) currentTemp = equiTemperature; 
	if(numberOfHeatSteps==0 && numberOfEquiSteps==0 ) currentTemp = runTemperature; 

	initMD(molecularDynamics,currentTemp,stepSize, integratorType, thermostat, friction, collide, fileNameTraj, fileNameProp, numberOfRunSteps);
	molecularDynamics->forceField->klass->calculateGradient(molecularDynamics->forceField);
	computeEnergies(molecularDynamics);

	iSel = -1;
	{
		if(str) g_free(str);
		str = g_strdup_printf("Geometry selected Potential energy =  %0.4f", molecularDynamics->potentialEnergy);
		redrawMolecule(&molecularDynamics->forceField->molecule,str);
		iSel++;
		geometries[iSel] = g_malloc(sizeof(ForceField));
		*geometries[iSel] = copyForceField(molecularDynamics->forceField);
		Waiting(1);
	}

	molecularDynamics->temperature = heatTemperature;
	rescaleVelocities(molecularDynamics);

	currentTemp = heatTemperature;
	n0 = 0;
	newProperties(molecularDynamics," ");
	/*newProperties(molecularDynamics," ----> Heating");*/
	for (i = 0; i < numberOfHeatSteps; i++ )
	{
		molecularDynamics->temperature = currentTemp;
		applyOneStep(molecularDynamics);
		currentTemp = heatTemperature + ( runTemperature - heatTemperature ) *
				( ( gdouble )( i + 1 )/ numberOfHeatSteps );
		molecularDynamics->temperature = currentTemp;
		rescaleVelocities(molecularDynamics);
		if(StopCalcul) break;
		if (++updateNumber >= molecularDynamics->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Heating: %0.2f fs, T = %0.2f K T(t) = %0.2f Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					molecularDynamics->kelvin, 
					molecularDynamics->kineticEnergy,
					molecularDynamics->potentialEnergy,
					molecularDynamics->totalEnergy
					);
			redrawMolecule(&molecularDynamics->forceField->molecule,str);
			updateNumber = 0;
		}
		saveProperties(molecularDynamics, n0+i+1, i+1," Heating");
	}

	currentTemp = equiTemperature;
	molecularDynamics->temperature = currentTemp;
	rescaleVelocities(molecularDynamics);
	if(StopCalcul) numberOfEquiSteps =0;
	if(StopCalcul) numberOfRunSteps =0;
	updateNumber = molecularDynamics->updateFrequency;
	n0 += numberOfHeatSteps;
	/* newProperties(molecularDynamics," ----> Equilibrium");*/
	for (i = 0; i < numberOfEquiSteps; i++ )
	{
		molecularDynamics->temperature = currentTemp;
		applyOneStep(molecularDynamics);
		molecularDynamics->temperature = currentTemp;
		rescaleVelocities(molecularDynamics);
		if(StopCalcul) break;
		if (++updateNumber >= molecularDynamics->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Equilibrium: %0.2f fs, T = %0.2f K  T(t) = %0.2f K Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					molecularDynamics->kelvin, 
					molecularDynamics->kineticEnergy,
					molecularDynamics->potentialEnergy,
					molecularDynamics->totalEnergy
					);
			redrawMolecule(&molecularDynamics->forceField->molecule,str);
			updateNumber = 0;
		}
		saveProperties(molecularDynamics, n0+i+1, i+1, " Equilibrium");
	}
	updateNumber = molecularDynamics->updateFrequency;

	currentTemp = runTemperature;
	molecularDynamics->temperature = currentTemp;
	rescaleVelocities(molecularDynamics);
	if(StopCalcul) numberOfRunSteps =0;
	updateNumber = molecularDynamics->updateFrequency;
	n0 += numberOfEquiSteps;
	/* newProperties(molecularDynamics," ----> Runing");*/
	if(str) g_free(str);
	str = g_strdup_printf(_("Geometry selected Potential energy =  %0.4f"), molecularDynamics->potentialEnergy);
	redrawMolecule(&molecularDynamics->forceField->molecule,str);
	if(numberOfGeometries>2) stepSel = numberOfRunSteps/(numberOfGeometries-1);
	else stepSel = numberOfRunSteps;
	/* printf("Isel = %d\n",stepSel);*/
	for (i = 0; i < numberOfRunSteps; i++ )
	{
		molecularDynamics->temperature = currentTemp;
		applyOneStep(molecularDynamics);
		if(molecularDynamics->thermostat == ANDERSEN) andersen(molecularDynamics);
		if(molecularDynamics->thermostat == BERENDSEN) berendsen(molecularDynamics);
		if(molecularDynamics->thermostat == BUSSI) bussi(molecularDynamics);
		if(StopCalcul) break;
		if (++updateNumber >= molecularDynamics->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Running: %0.2f fs, T = %0.2f K  T(t) = %0.2f K Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					molecularDynamics->kelvin, 
					molecularDynamics->kineticEnergy,
					molecularDynamics->potentialEnergy,
					molecularDynamics->totalEnergy
					);
			redrawMolecule(&molecularDynamics->forceField->molecule,str);
			updateNumber = 0;
			saveTrajectory(molecularDynamics, i+1);
		}
		if((i+1)%stepSel==0 && (iSel+1)<numberOfGeometries)
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("Geometry selected Potential energy =  %0.4f"), molecularDynamics->potentialEnergy);
			redrawMolecule(&molecularDynamics->forceField->molecule,str);
			iSel++;
			geometries[iSel] = g_malloc(sizeof(ForceField));
			*geometries[iSel] = copyForceField(molecularDynamics->forceField);
			Waiting(1);
		}
		saveProperties(molecularDynamics, n0+i+1, i+1," Running");
	}
	if(iSel<numberOfGeometries-1)
	{
		if(str) g_free(str);
		str = g_strdup_printf(_("Geometry selected Potential energy =  %0.4f"), molecularDynamics->potentialEnergy);
		redrawMolecule(&molecularDynamics->forceField->molecule,str);
		iSel++;
		geometries[iSel] = g_malloc(sizeof(ForceField));
		*geometries[iSel] = copyForceField(molecularDynamics->forceField);
		Waiting(1);
	}

	updateNumber = molecularDynamics->updateFrequency;
	n0 += numberOfRunSteps;

	molecularDynamics->forceField->klass->calculateGradient(molecularDynamics->forceField);
        gradientNorm = 0;
	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
		for ( j = 0; j < 3; j++)
                        gradientNorm += 
				molecularDynamics->forceField->molecule.gradient[j][i] * 
				molecularDynamics->forceField->molecule.gradient[j][i]; 

        gradientNorm = sqrt( gradientNorm );
	if(str) g_free(str);
	str = g_strdup_printf(_("End of MD Simulation. Gradient = %f Ekin = %f (Kcal/mol) EPot =  %0.4f ETot =  %0.4f T(t) = %0.2f"),
			(gdouble)gradientNorm,
			molecularDynamics->kineticEnergy,
			molecularDynamics->potentialEnergy,
			molecularDynamics->totalEnergy,
			molecularDynamics->kelvin 
			); 
	redrawMolecule(&molecularDynamics->forceField->molecule,str);
	g_free(str);
	if(molecularDynamics->fileTraj)fclose(molecularDynamics->fileTraj);
	if(molecularDynamics->fileProp)fclose(molecularDynamics->fileProp);
	freeMolecularDynamics(molecularDynamics);
	return geometries;
}
/**********************************************************************/
void	runMolecularDynamics(
		MolecularDynamics* molecularDynamics, ForceField* forceField, 
		gint updateFrequency, 
		gdouble heatTime, gdouble equiTime, gdouble runTime, gdouble coolTime, 
		gdouble heatTemperature, gdouble equiTemperature, gdouble runTemperature, gdouble coolTemperature, 
		gdouble stepSize,
		MDIntegratorType integratorType,
		MDThermostatType thermostat,
		gdouble friction,
		gdouble collide,
		gchar* fileNameTraj,
		gchar* fileNameProp
		)
{
	gint i;
	gint j;
	gchar* str = NULL;
        gdouble gradientNorm = 0;
	gint numberOfHeatSteps = 0;
	gint numberOfEquiSteps = 0;
	gint numberOfRunSteps = 0;
	gint numberOfCoolSteps = 0;
	gdouble currentTemp;
	gint updateNumber = 0;
	gint n0 = 0;
	/* 
	 *  physical constants in SI units
	 *   ------------------------------
	 *      Kb = 1.380662 E-23 J/K
	 *      Na = 6.022045 E23  1/mol
	 *      e = 1.6021892 E-19 C
	 *      eps = 8.85418782 E-12 F/m
	 *                       
	 *      1 Kcal = 4184.0 J
	 *      1 amu = 1.6605655 E-27 Kg
	 *      1 A = 1.0 E-10 m
	 *                                       
	 *       Internally, AKMA units are used:
	 *                                        
	 *       timeFactor = SQRT ( ( 1A )**2 * 1amu * Na  / 1Kcal )
	 *       kBoltzmann = Na *Kb  / 1 Kcal
	*/ 

	/* printf("basname = %s\n",g_path_get_basename(fileNameTraj));*/

	if(forceField->molecule.nAtoms<1) return;

	molecularDynamics->forceField = forceField;
	molecularDynamics->numberOfAtoms = forceField->molecule.nAtoms;
	molecularDynamics->updateFrequency = updateFrequency;

	currentTemp = heatTemperature/2;
	
	numberOfHeatSteps = heatTime/stepSize*1000;
	numberOfEquiSteps = equiTime/stepSize*1000;; 
	numberOfRunSteps = runTime/stepSize*1000;; 
	numberOfCoolSteps = coolTime/stepSize*1000;;


	currentTemp = heatTemperature;
	if(numberOfHeatSteps==0) currentTemp = equiTemperature; 
	if(numberOfHeatSteps==0 && numberOfEquiSteps==0 ) currentTemp = runTemperature; 
	if(numberOfHeatSteps==0 && numberOfEquiSteps==0 && numberOfRunSteps==0 ) currentTemp = coolTemperature; 

	initMD(molecularDynamics,currentTemp,stepSize, integratorType, thermostat, friction, collide, fileNameTraj, fileNameProp, numberOfRunSteps);
	molecularDynamics->forceField->klass->calculateGradient(molecularDynamics->forceField);

	molecularDynamics->temperature = heatTemperature;
	rescaleVelocities(molecularDynamics);

	currentTemp = heatTemperature;
	n0 = 0;
	newProperties(molecularDynamics," ");
	/*newProperties(molecularDynamics," ----> Heating");*/
	for (i = 0; i < numberOfHeatSteps; i++ )
	{
		molecularDynamics->temperature = currentTemp;
		applyOneStep(molecularDynamics);
		currentTemp = heatTemperature + ( runTemperature - heatTemperature ) *
				( ( gdouble )( i + 1 )/ numberOfHeatSteps );
		molecularDynamics->temperature = currentTemp;
		rescaleVelocities(molecularDynamics);
		if(StopCalcul) break;
		if (++updateNumber >= molecularDynamics->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Heating: %0.2f fs, T = %0.2f K T(t) = %0.2f Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					molecularDynamics->kelvin, 
					molecularDynamics->kineticEnergy,
					molecularDynamics->potentialEnergy,
					molecularDynamics->totalEnergy
					);
			redrawMolecule(&molecularDynamics->forceField->molecule,str);
			updateNumber = 0;
		}
		saveProperties(molecularDynamics, n0+i+1, i+1," Heating");
	}

	currentTemp = equiTemperature;
	molecularDynamics->temperature = currentTemp;
	rescaleVelocities(molecularDynamics);
	if(StopCalcul) numberOfEquiSteps =0;
	if(StopCalcul) numberOfRunSteps =0;
	if(StopCalcul) numberOfCoolSteps =0;
	updateNumber = molecularDynamics->updateFrequency;
	n0 += numberOfHeatSteps;
	/* newProperties(molecularDynamics," ----> Equilibrium");*/
	for (i = 0; i < numberOfEquiSteps; i++ )
	{
		molecularDynamics->temperature = currentTemp;
		applyOneStep(molecularDynamics);
		molecularDynamics->temperature = currentTemp;
		rescaleVelocities(molecularDynamics);
		if(StopCalcul) break;
		if (++updateNumber >= molecularDynamics->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Equilibrium: %0.2f fs, T = %0.2f K  T(t) = %0.2f K Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					molecularDynamics->kelvin, 
					molecularDynamics->kineticEnergy,
					molecularDynamics->potentialEnergy,
					molecularDynamics->totalEnergy
					);
			redrawMolecule(&molecularDynamics->forceField->molecule,str);
			updateNumber = 0;
		}
		saveProperties(molecularDynamics, n0+i+1, i+1, " Equilibrium");
	}
	updateNumber = molecularDynamics->updateFrequency;

	currentTemp = runTemperature;
	molecularDynamics->temperature = currentTemp;
	rescaleVelocities(molecularDynamics);
	if(StopCalcul) numberOfRunSteps =0;
	if(StopCalcul) numberOfCoolSteps =0;
	updateNumber = molecularDynamics->updateFrequency;
	n0 += numberOfEquiSteps;
	/* newProperties(molecularDynamics," ----> Runing");*/
	for (i = 0; i < numberOfRunSteps; i++ )
	{
		molecularDynamics->temperature = currentTemp;
		applyOneStep(molecularDynamics);
		if(molecularDynamics->thermostat == ANDERSEN) andersen(molecularDynamics);
		if(molecularDynamics->thermostat == BERENDSEN) berendsen(molecularDynamics);
		if(molecularDynamics->thermostat == BUSSI) bussi(molecularDynamics);
		if(StopCalcul) break;
		if (++updateNumber >= molecularDynamics->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Running: %0.2f fs, T = %0.2f K  T(t) = %0.2f K Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					molecularDynamics->kelvin, 
					molecularDynamics->kineticEnergy,
					molecularDynamics->potentialEnergy,
					molecularDynamics->totalEnergy
					);
			redrawMolecule(&molecularDynamics->forceField->molecule,str);
			updateNumber = 0;
			saveTrajectory(molecularDynamics, i+1);
		}
		saveProperties(molecularDynamics, n0+i+1, i+1," Running");
	}

	updateNumber = molecularDynamics->updateFrequency;
	if(StopCalcul) numberOfCoolSteps =0;
	n0 += numberOfRunSteps;
	/* newProperties(molecularDynamics," ----> Cooling");*/
	for (i = 0; i < numberOfCoolSteps; i++ )
	{
		currentTemp = runTemperature - ( runTemperature - coolTemperature ) * 
				( ( gdouble )( i + 1 )/ numberOfCoolSteps );
		molecularDynamics->temperature = currentTemp;
		rescaleVelocities(molecularDynamics);
		molecularDynamics->temperature = currentTemp;
		applyOneStep(molecularDynamics);
		if(StopCalcul) break;
		if (++updateNumber >= molecularDynamics->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Cooling: %0.2f fs, T = %0.2f K T(t) = %0.2f K Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					molecularDynamics->kelvin, 
					molecularDynamics->kineticEnergy,
					molecularDynamics->potentialEnergy,
					molecularDynamics->totalEnergy
					);
			redrawMolecule(&molecularDynamics->forceField->molecule,str);
			updateNumber = 0;
		}
		saveProperties(molecularDynamics, n0+i+1, i+1," Cooling");
	}
	molecularDynamics->forceField->klass->calculateGradient(molecularDynamics->forceField);
        gradientNorm = 0;
	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
		for ( j = 0; j < 3; j++)
                        gradientNorm += 
				molecularDynamics->forceField->molecule.gradient[j][i] * 
				molecularDynamics->forceField->molecule.gradient[j][i]; 

        gradientNorm = sqrt( gradientNorm );
	if(str) g_free(str);
	str = g_strdup_printf(_("End of MD Simulation. Gradient = %f Ekin = %f (Kcal/mol) EPot =  %0.4f ETot =  %0.4f T(t) = %0.2f"),
			(gdouble)gradientNorm,
			molecularDynamics->kineticEnergy,
			molecularDynamics->potentialEnergy,
			molecularDynamics->totalEnergy,
			molecularDynamics->kelvin 
			); 
	redrawMolecule(&molecularDynamics->forceField->molecule,str);
	g_free(str);
	if(molecularDynamics->fileTraj)fclose(molecularDynamics->fileTraj);
	if(molecularDynamics->fileProp)fclose(molecularDynamics->fileProp);
	freeMolecularDynamics(molecularDynamics);
}
/*********************************************************************************/
static void removeTranslation(MolecularDynamics* molecularDynamics)
{
	gdouble vtot[3] = {0,0,0};
	gint i;
	gint j;
	gdouble mass = 1.0;
	gdouble totMass = 0.0;
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		mass = molecularDynamics->forceField->molecule.atoms[i].prop.masse;
		totMass += mass;
		for ( j = 0; j < 3; j++)
		{
			vtot[j] += mass*molecularDynamics->velocity[i][j];
		}
	}

	for ( j = 0; j < 3; j++)
		vtot[j] /= totMass;

	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
		for ( j = 0; j < 3; j++)
			molecularDynamics->velocity[i][j] -= vtot[j];
	/* check */
	/*
	for ( j = 0; j < 3; j++)
		vtot[j] = 0;
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		mass = molecularDynamics->forceField->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
		{
			vtot[j] += mass*molecularDynamics->velocity[i][j];
		}
	}
	printf("Trans velocity = %f %f %f\n",vtot[0], vtot[1], vtot[2]);
	*/
}
/**************************************************/
static gboolean InverseTensor(gdouble mat[3][3],gdouble invmat[3][3])
{
	gdouble t4,t6,t8,t10,t12,t14,t17;
	gdouble d = 0;

	t4 = mat[0][0]*mat[1][1];     
 	t6 = mat[0][0]*mat[1][2];
      	t8 = mat[0][1]*mat[1][0];
      	t10 = mat[0][2]*mat[1][0];
      	t12 = mat[0][1]*mat[2][0];
      	t14 = mat[0][2]*mat[2][0];
      	d =(t4*mat[2][2]-t6*mat[2][1]-t8*mat[2][2]+t10*mat[2][1]+t12*mat[1][2]-t14*mat[1][1]);
	if(d == 0) 
	{
      		invmat[0][0] = 0;
      		invmat[0][1] = 0;
      		invmat[0][2] = 0;
      		invmat[1][0] = 0;
      		invmat[1][1] = 0;
      		invmat[1][2] = 0;
      		invmat[2][0] = 0;
      		invmat[2][1] = 0;
      		invmat[2][2] = 0;
		return FALSE;
	}
      	t17 = 1/d;
      	invmat[0][0] = (mat[1][1]*mat[2][2]-mat[1][2]*mat[2][1])*t17;
      	invmat[0][1] = -(mat[0][1]*mat[2][2]-mat[0][2]*mat[2][1])*t17;
      	invmat[0][2] = -(-mat[0][1]*mat[1][2]+mat[0][2]*mat[1][1])*t17;
      	invmat[1][0] = -(mat[1][0]*mat[2][2]-mat[1][2]*mat[2][0])*t17;
      	invmat[1][1] = (mat[0][0]*mat[2][2]-t14)*t17;
      	invmat[1][2] = -(t6-t10)*t17;
      	invmat[2][0] = -(-mat[1][0]*mat[2][1]+mat[1][1]*mat[2][0])*t17;
      	invmat[2][1] = -(mat[0][0]*mat[2][1]-t12)*t17;
      	invmat[2][2] = (t4-t8)*t17;

	return TRUE;
}
/*********************************************************************************/
static void removeRotation(MolecularDynamics* molecularDynamics)
{
	gdouble vtot[3] = {0,0,0};
	gdouble cm[3] = {0,0,0};
	gdouble L[3] = {0,0,0};
	gint i;
	gint j;
	gint k;
	gdouble mass = 1.0;
	gdouble totMass = 0.0;
	gdouble cdel[3];
	gdouble vAng[3]={0,0,0};
	gdouble tensor[3][3];
	gdouble invTensor[3][3];
        gdouble xx, xy,xz,yy,yz,zz;
	/* find the center of mass coordinates  and total velocity*/
	AtomMol* atoms = molecularDynamics->forceField->molecule.atoms;


	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		mass = molecularDynamics->forceField->molecule.atoms[i].prop.masse;
		totMass += mass;
		for ( j = 0; j < 3; j++)
			cm[j] += mass*atoms[i].coordinates[j];
		for ( j = 0; j < 3; j++)
			vtot[j] += mass*molecularDynamics->velocity[i][j];
	}


	for ( j = 0; j < 3; j++)
		cm[j] /= totMass;
	for ( j = 0; j < 3; j++)
		vtot[j] /= totMass;

	/*   compute the angular momentum  */
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		mass = molecularDynamics->forceField->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
			L[j] += (
				atoms[i].coordinates[(j+1)%3]*molecularDynamics->velocity[i][(j+2)%3]
			      - atoms[i].coordinates[(j+2)%3]*molecularDynamics->velocity[i][(j+1)%3]
			      )*mass;
	}
	for ( j = 0; j < 3; j++)
		L[j] -= (
			cm[(j+1)%3]*vtot[(j+2)%3]
		      - cm[(j+2)%3]*vtot[(j+1)%3]
			      )*totMass;

	/* calculate and invert the inertia tensor */
	for ( k = 0; k < 3; k++)
	for ( j = 0; j < 3; j++)
		tensor[k][j] = 0;
	xx = 0;
	yy = 0;
	zz = 0;
	xy = 0;
	xz = 0;
	yz = 0;
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		mass = molecularDynamics->forceField->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
			cdel[j] = atoms[i].coordinates[j]-cm[j];
		xx +=  cdel[0]*cdel[0]*mass;
		xy +=  cdel[0]*cdel[1]*mass;
		xz +=  cdel[0]*cdel[2]*mass;
		yy +=  cdel[1]*cdel[1]*mass;
		yz +=  cdel[1]*cdel[2]*mass;
		zz +=  cdel[2]*cdel[2]*mass;
	}
	tensor[0][0] = yy+zz;
	tensor[1][0] = -xy;
	tensor[2][0] = -xz;
	tensor[0][1] = -xy;
	tensor[1][1] = xx+zz;
	tensor[2][1] = -yz;
	tensor[0][2] = -xz;
	tensor[1][2] = -yz;
	tensor[2][2] = xx+yy;
	if(InverseTensor(tensor,invTensor))
	{
		for ( j = 0; j < 3; j++)
		{
			vAng[j] = 0;
			for ( k = 0; k < 3; k++)
				vAng[j] += invTensor[j][k]*L[k];
		}
		/*  eliminate any rotation about the system center of mass */
		for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
		{
			for ( j = 0; j < 3; j++)
				cdel[j] = atoms[i].coordinates[j]-cm[j];
			for ( j = 0; j < 3; j++)
				molecularDynamics->velocity[i][j] += 
					cdel[(j+1)%3]*vAng[(j+2)%3]-
					cdel[(j+2)%3]*vAng[(j+1)%3];
		}
	}
	/*   check  */
	/*
	for ( j = 0; j < 3; j++)
		L[j] = 0;
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		mass = molecularDynamics->forceField->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
			L[j] += (
				atoms[i].coordinates[(j+1)%3]*molecularDynamics->velocity[i][(j+2)%3]
			      - atoms[i].coordinates[(j+2)%3]*molecularDynamics->velocity[i][(j+1)%3]
			      )*mass;
	}
	for ( j = 0; j < 3; j++)
	{
		vAng[j] = 0;
		for ( k = 0; k < 3; k++)
			vAng[j] += invTensor[j][k]*L[k];
	}
	printf("Angular velocity = %f %f %f\n",vAng[0], vAng[1], vAng[2]);
	*/

}
/*********************************************************************************/
static void removeTranslationAndRotation(MolecularDynamics* molecularDynamics)
{
	removeTranslation(molecularDynamics);
	removeRotation(molecularDynamics);
}
/*********************************************************************************/
static void initSD(MolecularDynamics* molecularDynamics, gdouble friction)
{
	/* gdouble fsInAKMA = 1.0/sqrt(1e-10*1e-10*1.6605655e-27*6.022045e23/4184.0)/1e15;*/
	static gdouble fsInAKMA = 0.020454828110640;
	gint i;


	molecularDynamics->friction = friction/(fsInAKMA)/1000;

	molecularDynamics->positionFriction = NULL;
	molecularDynamics->velocityFriction = NULL;
	molecularDynamics->accelarationFriction = NULL;
	molecularDynamics->gamma = NULL;
	molecularDynamics->positionRandom = NULL;
	molecularDynamics->velocityRandom = NULL;

	if(molecularDynamics->integratorType != STOCHASTIC) return;

	molecularDynamics->positionFriction = g_malloc(molecularDynamics->numberOfAtoms *sizeof(gdouble)); 
	molecularDynamics->velocityFriction = g_malloc(molecularDynamics->numberOfAtoms *sizeof(gdouble)); 
	molecularDynamics->accelarationFriction = g_malloc(molecularDynamics->numberOfAtoms *sizeof(gdouble)); 
	molecularDynamics->gamma = g_malloc(molecularDynamics->numberOfAtoms *sizeof(gdouble)); 

	molecularDynamics->positionRandom = g_malloc(molecularDynamics->numberOfAtoms *sizeof(gdouble*)); 
	for(i=0;i<molecularDynamics->numberOfAtoms;i++)
		molecularDynamics->positionRandom[i] = g_malloc(3*sizeof(gdouble));

	molecularDynamics->velocityRandom = g_malloc(molecularDynamics->numberOfAtoms *sizeof(gdouble*)); 
	for(i=0;i<molecularDynamics->numberOfAtoms;i++)
		molecularDynamics->velocityRandom[i] = g_malloc(3*sizeof(gdouble));

}
/*********************************************************************************/
static void initMD(MolecularDynamics* molecularDynamics, gdouble temperature, gdouble stepSize, MDIntegratorType integratorType, MDThermostatType thermostat, gdouble friction, gdouble collide, gchar* fileNameTraj, gchar* fileNameProp, gint numberOfRunSteps)
{
	gint i;
	gint j;
	/* gdouble fsInAKMA = 1.0/sqrt(1e-10*1e-10*1.6605655e-27*6.022045e23/4184.0)/1e15;*/
	static gdouble fsInAKMA = 0.020454828110640;
	gdouble dt = stepSize * fsInAKMA;

	molecularDynamics->collide = collide;
	molecularDynamics->potentialEnergy = 0;
	molecularDynamics->kineticEnergy = 0;
	molecularDynamics->totalEnergy = 0;
	molecularDynamics->kelvin = 0;
	molecularDynamics->temperature = temperature;
	molecularDynamics->thermostat = NONE;

	molecularDynamics->integratorType = integratorType;
	molecularDynamics->thermostat = thermostat;
	molecularDynamics->fileTraj = NULL;
	molecularDynamics->fileProp = NULL;

	molecularDynamics->velocity = g_malloc(molecularDynamics->numberOfAtoms *sizeof(gdouble*)); 
	for(i=0;i<molecularDynamics->numberOfAtoms;i++)
		molecularDynamics->velocity[i] = g_malloc(3*sizeof(gdouble));

	molecularDynamics->a = g_malloc(molecularDynamics->numberOfAtoms *sizeof(gdouble*)); 
	for(i=0;i<molecularDynamics->numberOfAtoms;i++)
		molecularDynamics->a[i] = g_malloc(3*sizeof(gdouble));

	molecularDynamics->aold = NULL;
	if(molecularDynamics->integratorType==BEEMAN)
	{
		molecularDynamics->aold = g_malloc(molecularDynamics->numberOfAtoms *sizeof(gdouble*)); 
		for(i=0;i<molecularDynamics->numberOfAtoms;i++)
			molecularDynamics->aold[i] = g_malloc(3*sizeof(gdouble));
	}
	molecularDynamics->coordinatesOld = NULL;
	molecularDynamics->moved = NULL;
	molecularDynamics->update = NULL;
	if(molecularDynamics->forceField->options.rattleConstraints!=NOCONSTRAINTS)
	{
		molecularDynamics->coordinatesOld = g_malloc(molecularDynamics->numberOfAtoms *sizeof(gdouble*)); 
		for(i=0;i<molecularDynamics->numberOfAtoms;i++)
			molecularDynamics->coordinatesOld[i] = g_malloc(3*sizeof(gdouble));
		molecularDynamics->moved = g_malloc(molecularDynamics->numberOfAtoms *sizeof(gboolean)); 
		molecularDynamics->update = g_malloc(molecularDynamics->numberOfAtoms *sizeof(gboolean)); 

	}

	if(fileNameTraj)
	{
 		molecularDynamics->fileTraj = fopen(fileNameTraj, "w");
		if(molecularDynamics->fileTraj != NULL)
		{
			fprintf(molecularDynamics->fileTraj,"[Gabedit Format]\n");
			fprintf(molecularDynamics->fileTraj,"\n");
			fprintf(molecularDynamics->fileTraj,"[MD]\n");
			if(molecularDynamics->updateFrequency>0) numberOfRunSteps/=molecularDynamics->updateFrequency;
			fprintf(molecularDynamics->fileTraj," %d\n",numberOfRunSteps);
		}
	}
	if(fileNameProp)
	{
 		molecularDynamics->fileProp = fopen(fileNameProp, "w");
	}

	srand ( (unsigned)time (NULL));
	
	molecularDynamics->dt = dt;
	molecularDynamics->dt_2 = dt/2.0;
	molecularDynamics->dt2_2 = dt*dt/2;;
	molecularDynamics->dt_8 = dt/8.0;
	molecularDynamics->dt2_8 = dt*dt/8.0;

	initSD(molecularDynamics, friction);


	molecularDynamics->forceField->klass->calculateGradient(molecularDynamics->forceField);
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		gdouble m = molecularDynamics->forceField->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++) molecularDynamics->a[i][j] = -molecularDynamics->forceField->molecule.gradient[j][i]/m;
		if(molecularDynamics->aold) 
			for ( j = 0; j < 3; j++) molecularDynamics->aold[i][j]  = molecularDynamics->a[i][j];
		if(molecularDynamics->coordinatesOld)
		       	for ( j = 0; j < 3; j++) molecularDynamics->coordinatesOld[i][j]  = molecularDynamics->forceField->molecule.atoms[i].coordinates[j];
		if(molecularDynamics->moved) molecularDynamics->moved[i] = FALSE; 
		if(molecularDynamics->update) molecularDynamics->update[i] = FALSE; 
	}

	if(temperature<=0)
	{
		for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
			for ( j = 0; j < 3; j++)
				molecularDynamics->velocity[i][j] = 0.0;
	}
	else
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		if(molecularDynamics->forceField->molecule.atoms[i].variable)
		{
			gdouble speed = maxwel(molecularDynamics->forceField->molecule.atoms[i].prop.masse,temperature);
			getRandVect(speed, molecularDynamics->velocity[i]);
		}
		else
			for( j = 0; j < 3; j++) molecularDynamics->velocity[i][j] = 0.0;
	}
        molecularDynamics->nvariables = 0;
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
		if(molecularDynamics->forceField->molecule.atoms[i].variable) molecularDynamics->nvariables +=1;
        if(molecularDynamics->nvariables==0)
	{
        	molecularDynamics->nvariables= molecularDynamics->numberOfAtoms;
		for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
		{
			molecularDynamics->forceField->molecule.atoms[i].variable = TRUE;
			if(temperature>0)
			{
				gdouble speed = maxwel(molecularDynamics->forceField->molecule.atoms[i].prop.masse,temperature);
				getRandVect(speed, molecularDynamics->velocity[i]);
			}
		}
	}
        molecularDynamics->nfree = 3* molecularDynamics->nvariables-molecularDynamics->forceField->numberOfRattleConstraintsTerms;
        removeTranslationAndRotation(molecularDynamics);
        if(molecularDynamics->nvariables==molecularDynamics->numberOfAtoms) molecularDynamics->nfree -=6;
        if(molecularDynamics->nvariables==molecularDynamics->numberOfAtoms-1) molecularDynamics->nfree -=3;
        if(molecularDynamics->nvariables==molecularDynamics->numberOfAtoms-2) molecularDynamics->nfree -=1;
	printf("nfree =%d\n",molecularDynamics->nfree);
	if( molecularDynamics->nfree<1)  { 
		StopCalcul=TRUE;
	}
        if( molecularDynamics->nfree<1)  molecularDynamics->nfree = 1;

	removeTranslationAndRotation(molecularDynamics);
}
/*********************************************************************************/
static void rescaleVelocities(MolecularDynamics* molecularDynamics)
{
	berendsen(molecularDynamics);
}
/*********************************************************************************/
static void berendsen(MolecularDynamics* molecularDynamics)
{
	gint i;
	gint j;
	static gdouble fsInAKMA = 0.020454828110640;
	gdouble ekin = 0;
	gdouble kelvin = 0;
        gint nfree = molecularDynamics->nfree;
	static gdouble Kb = 1.9871914e-3;
	gdouble scale = 1.0;
	gdouble dt = molecularDynamics->dt;
	gdouble tautemp = 1.0/(molecularDynamics->collide)*1000*fsInAKMA;
	gdouble masse = 1.0;
	if(molecularDynamics->temperature<=0) return;
	if(nfree<1) return;
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		masse = molecularDynamics->forceField->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
			ekin += molecularDynamics->velocity[i][j]*molecularDynamics->velocity[i][j]*
				masse;
	}
	/*
	ekin /= 2;
	kelvin = 2* ekin / ( nfree * Kb);
	*/
	kelvin = ekin / ( nfree * Kb);
	/* if(tautemp>dt) tautemp = dt;*/
	scale = sqrt(1.0 + (dt/tautemp)*(molecularDynamics->temperature/kelvin-1.0));
	/* printf("temp = %f kelvin = %f scale = %f\n",molecularDynamics->temperature, kelvin, scale);*/
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		if(molecularDynamics->forceField->molecule.atoms[i].variable)
		for ( j = 0; j < 3; j++)
			molecularDynamics->velocity[i][j] *= scale;
	}
	removeTranslationAndRotation(molecularDynamics);
}
/*********************************************************************************/
static void andersen(MolecularDynamics* molecularDynamics)
{
	gint i;
	/* gdouble fsInAKMA = 1.0/sqrt(1e-10*1e-10*1.6605655e-27*6.022045e23/4184.0)/1e15;*/
	static gdouble fsInAKMA = 0.020454828110640;
	gdouble tau = 1.0/molecularDynamics->collide*1000*fsInAKMA; /* in fs */
	gdouble rate;
	static gdouble Kb = 1.9871914e-3;
	if(molecularDynamics->temperature<=0) return;
	if(molecularDynamics->numberOfAtoms<1) return;

	rate = molecularDynamics->dt / tau;
	rate /= pow(molecularDynamics->nvariables,2.0/3.0);

	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		gdouble trial = drandom();
		if(molecularDynamics->forceField->molecule.atoms[i].variable)
		if(trial<rate)
		{
/*
			gdouble speed = maxwel(
					molecularDynamics->forceField->molecule.atoms[i].prop.masse,
					molecularDynamics->temperature
					);
			getRandVect(speed, molecularDynamics->velocity[i]);
*/
			double speed = sqrt(Kb* molecularDynamics->temperature/molecularDynamics->forceField->molecule.atoms[i].prop.masse);
                	double pnorm = normal();
			molecularDynamics->velocity[i][0] = pnorm*speed;
                	pnorm = normal();
			molecularDynamics->velocity[i][1] = pnorm*speed;
                	pnorm = normal();
			molecularDynamics->velocity[i][2] = pnorm*speed;
		}
	}
}
/*********************************************************************************/
static void bussi(MolecularDynamics* molecularDynamics)
{
        static gdouble fsInAKMA = 0.020454828110640;
        gint nfree = molecularDynamics->nfree;
        static gdouble Kb = 1.9871914e-3;
        gdouble scale = 1.0;
        gdouble dt = molecularDynamics->dt;
        gdouble tautemp = 1.0/(molecularDynamics->collide)*1000*fsInAKMA;
        gdouble c = exp(-dt/tautemp);
        gdouble ekin = getEKin(molecularDynamics);
        gdouble kelvin = 2*ekin / ( nfree * Kb);
        gdouble d = (1.0-c) * (molecularDynamics->temperature/kelvin) / (nfree);
        gdouble r = normal ();
        gdouble si = 0.0;
        gdouble s = 0.0;
        gint i,j;
        if(molecularDynamics->temperature<=0) return;
        if(nfree<1) return;
        for(i=0;i<nfree-1;i++)
        {
            si = normal ();
            s += si*si;
        }
        scale = c + (s+r*r)*d + 2.0*r*sqrt(c*d);
        scale = sqrt(scale);
        if (r+sqrt(c/d)<0)  scale = -scale;
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
		if(molecularDynamics->forceField->molecule.atoms[i].variable)
		for ( j = 0; j < 3; j++)
			molecularDynamics->velocity[i][j] *= scale;
	removeTranslationAndRotation(molecularDynamics);
}
/*********************************************************************************/
static void newAccelaration(MolecularDynamics* molecularDynamics)
{
	gint i;
	gint j;
	molecularDynamics->forceField->klass->calculateGradient(molecularDynamics->forceField);
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		gdouble m = molecularDynamics->forceField->molecule.atoms[i].prop.masse;
		if(molecularDynamics->aold)
			for ( j = 0; j < 3; j++)
				molecularDynamics->aold[i][j]  = molecularDynamics->a[i][j];

		for ( j = 0; j < 3; j++)
			molecularDynamics->a[i][j] = -molecularDynamics->forceField->molecule.gradient[j][i]/m;
	}
}
/*********************************************************************************/
static void computeEnergies(MolecularDynamics* molecularDynamics)
{
	molecularDynamics->kineticEnergy = getEKin(molecularDynamics);
	molecularDynamics->potentialEnergy = molecularDynamics->forceField->klass->calculateEnergyTmp(
		      molecularDynamics->forceField,&molecularDynamics->forceField->molecule);
	molecularDynamics->totalEnergy = molecularDynamics->kineticEnergy + molecularDynamics->potentialEnergy;
	molecularDynamics->kelvin = getKelvin(molecularDynamics);
}
/*********************************************************************************/
static void applyOneStep(MolecularDynamics* molecularDynamics)
{
	if(molecularDynamics->integratorType==VERLET) applyVerlet(molecularDynamics);
	else if(molecularDynamics->integratorType==BEEMAN) applyBeeman(molecularDynamics);
	else applyStochastic(molecularDynamics);
	computeEnergies(molecularDynamics);
}
/*********************************************************************************/
static void applyRattleFirstPortion(MolecularDynamics* molecularDynamics)
{
	gint i;
	gint k;
	gint maxIter = 1000;
	gdouble omega = 1.0; 
	gdouble tolerance = 1e-6;
	gboolean done = FALSE;
	gint nIter = 0;
	gint a1 = 0;
	gint a2 = 0;
	gdouble r2ij;
	gdouble dot;
	gdouble invMass1;
	gdouble invMass2;
	gdouble delta;
	gdouble term = 0;
	gdouble terms[3];
	gdouble d;
	Molecule* m = &molecularDynamics->forceField->molecule;
	ForceField* forceField = molecularDynamics->forceField;
	gdouble deltaMax = 0;

	if(forceField->options.rattleConstraints==NOCONSTRAINTS) return;
	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		molecularDynamics->moved[i] = molecularDynamics->forceField->molecule.atoms[i].variable;
		molecularDynamics->update[i] = FALSE;
	}
	/* maxIter *= molecularDynamics->forceField->numberOfRattleConstraintsTerms;*/
	do{
		nIter++;
		done=TRUE;
		deltaMax = 0;
		for (i = 0; i < molecularDynamics->forceField->numberOfRattleConstraintsTerms; i++)
		{
			a1 = (gint)molecularDynamics->forceField->rattleConstraintsTerms[0][i];
			a2 = (gint)molecularDynamics->forceField->rattleConstraintsTerms[1][i];
			if( !molecularDynamics->moved[a1] && !molecularDynamics->moved[a2] ) continue;
			r2ij = 0;
			for (k=0;k<3;k++)
			{
				d = m->atoms[a2].coordinates[k]-m->atoms[a1].coordinates[k];
				r2ij +=d*d;
			}
			delta = molecularDynamics->forceField->rattleConstraintsTerms[2][i]-r2ij;
			/* if(fabs(delta)<=tolerance) continue;*/
			if(r2ij>0 && fabs(delta/r2ij)<=tolerance) continue;
			if(deltaMax<fabs(delta)) deltaMax = fabs(delta);
			done = FALSE;
			molecularDynamics->update[a1] = TRUE;
			molecularDynamics->update[a2] = TRUE;
			/* here : rattle image for PBC, not yet implemented */
			dot = 0;
			for (k=0;k<3;k++)
			{
				d = m->atoms[a2].coordinates[k]-m->atoms[a1].coordinates[k];
				dot +=d*(molecularDynamics->coordinatesOld[a2][k]-molecularDynamics->coordinatesOld[a1][k]);
			}
			invMass1 = 1/m->atoms[a1].prop.masse;
			invMass2 = 1/m->atoms[a2].prop.masse;
		        term = omega*delta / (2.0*(invMass1+invMass2)*dot);
			for (k=0;k<3;k++)
			{
				terms[k] = (molecularDynamics->coordinatesOld[a2][k]-molecularDynamics->coordinatesOld[a1][k])*term;
			}
			for (k=0;k<3;k++) m->atoms[a1].coordinates[k] -= terms[k]*invMass1;
			for (k=0;k<3;k++) m->atoms[a2].coordinates[k] += terms[k]*invMass2;

			invMass1 /= molecularDynamics->dt;
			invMass2 /= molecularDynamics->dt;
			for (k=0;k<3;k++) molecularDynamics->velocity[a1][k] -= terms[k]*invMass1;
			for (k=0;k<3;k++) molecularDynamics->velocity[a2][k] += terms[k]*invMass2;
		}
		for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
		{
			molecularDynamics->moved[i] = molecularDynamics->update[i];
			molecularDynamics->update[i] = FALSE;
		}
	}while(!done && nIter<maxIter);
	if(nIter>=maxIter && deltaMax>tolerance*10)
	{
		printf(_("Rattle first portion : Warning, distance constraints not satisfied\n"));
		/*
		for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
		{
			printf("atom#%d\n",i);
			printf("Old coord\n");
			for (k=0;k<3;k++) printf("%f ",molecularDynamics->coordinatesOld[i][k]);
			printf("\n");
			printf("New coord\n");
			for (k=0;k<3;k++) printf("%f ",m->atoms[i].coordinates[k]);
			printf("\n");
		}
		exit(1);
		*/
		for (i = 0; i < molecularDynamics->forceField->numberOfRattleConstraintsTerms; i++)
		{
			a1 = (gint)molecularDynamics->forceField->rattleConstraintsTerms[0][i];
			a2 = (gint)molecularDynamics->forceField->rattleConstraintsTerms[1][i];
			r2ij = 0;
			for (k=0;k<3;k++)
			{
				d = m->atoms[a2].coordinates[k]-m->atoms[a1].coordinates[k];
				r2ij +=d*d;
			}
			delta = molecularDynamics->forceField->rattleConstraintsTerms[2][i]-r2ij;
			printf("%d %d %s %s r2ij=%f r2Old=%f delta=%f\n",
			a1,a2,
			molecularDynamics->forceField->molecule.atoms[a1].mmType,
			molecularDynamics->forceField->molecule.atoms[a2].mmType,
			r2ij, molecularDynamics->forceField->rattleConstraintsTerms[2][i],delta);
		}
		StopCalcul=TRUE;
	}
	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
		if(!m->atoms[i].variable) 
		{
			for (k=0;k<3;k++) m->atoms[i].coordinates[k] =  molecularDynamics->coordinatesOld[i][k];
			for (k=0;k<3;k++) molecularDynamics->velocity[i][k] = 0;
		}

}
/*********************************************************************************/
static void applyRattleSecondPortion(MolecularDynamics* molecularDynamics)
{
	gint i;
	gint k;
	gint maxIter = 1000;
	gdouble omega = 1.0;
	gdouble tolerance = 1e-6; 
	gboolean done = FALSE;
	gint nIter = 0;
	gint a1 = 0;
	gint a2 = 0;
	gdouble r2ij;
	gdouble dot;
	gdouble invMass1;
	gdouble invMass2;
	gdouble term = 0;
	gdouble terms[3];
	gdouble d;
	Molecule* m = &molecularDynamics->forceField->molecule;
	ForceField* forceField = molecularDynamics->forceField;
	gdouble deltaMax = 0;

	if(forceField->options.rattleConstraints==NOCONSTRAINTS) return;
	tolerance /= molecularDynamics->dt;
	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		molecularDynamics->moved[i] = molecularDynamics->forceField->molecule.atoms[i].variable;
		molecularDynamics->update[i] = FALSE;
	}
	/* maxIter *= molecularDynamics->forceField->numberOfRattleConstraintsTerms;*/
	do{
		nIter++;
		done=TRUE;
		deltaMax = 0;
		for (i = 0; i < molecularDynamics->forceField->numberOfRattleConstraintsTerms; i++)
		{
			a1 = (gint)molecularDynamics->forceField->rattleConstraintsTerms[0][i];
			a2 = (gint)molecularDynamics->forceField->rattleConstraintsTerms[1][i];
			r2ij = molecularDynamics->forceField->rattleConstraintsTerms[2][i];
			if( !molecularDynamics->moved[a1] && !molecularDynamics->moved[a2] ) continue;
			/* here : rattle image for PBC, not yet implemented */
			dot = 0;
			for (k=0;k<3;k++)
			{
				d = m->atoms[a2].coordinates[k]-m->atoms[a1].coordinates[k];
				dot +=d*(molecularDynamics->velocity[a2][k]-molecularDynamics->velocity[a1][k]);
			}
			invMass1 = 1/molecularDynamics->forceField->molecule.atoms[a1].prop.masse;
			invMass2 = 1/molecularDynamics->forceField->molecule.atoms[a2].prop.masse;
		        term = -dot / ((invMass1+invMass2)*r2ij);
			if(fabs(term)<=tolerance) continue;
			/* if(fabs(dot/r2ij)<=tolerance) continue;*/
			if(deltaMax<fabs(term)) deltaMax = fabs(term);

			done = FALSE;
			molecularDynamics->update[a1] = TRUE;
			molecularDynamics->update[a2] = TRUE;
		        term *= omega;

			for (k=0;k<3;k++)
			{
				d = m->atoms[a2].coordinates[k]-m->atoms[a1].coordinates[k];
				terms[k] = d*term;
			}
			for (k=0;k<3;k++) molecularDynamics->velocity[a1][k] -= terms[k]*invMass1;
			for (k=0;k<3;k++) molecularDynamics->velocity[a2][k] += terms[k]*invMass2;
		}
		for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
		{
			molecularDynamics->moved[i] = molecularDynamics->update[i];
			molecularDynamics->update[i] = FALSE;
		}
	}while(!done && nIter<maxIter);
	if(nIter>=maxIter && deltaMax>tolerance*10)
	{
		printf(_("Rattle second portion : Warning, velocity constraints not satisfied\n"));
		for (i = 0; i < molecularDynamics->forceField->numberOfRattleConstraintsTerms; i++)
		{
			a1 = (gint)molecularDynamics->forceField->rattleConstraintsTerms[0][i];
			a2 = (gint)molecularDynamics->forceField->rattleConstraintsTerms[1][i];
			r2ij = 0;
			for (k=0;k<3;k++)
			{
				d = m->atoms[a2].coordinates[k]-m->atoms[a1].coordinates[k];
				r2ij +=d*d;
			}
			dot = 0;
			for (k=0;k<3;k++)
			{
				d = m->atoms[a2].coordinates[k]-m->atoms[a1].coordinates[k];
				dot +=d*(molecularDynamics->velocity[a2][k]-molecularDynamics->velocity[a1][k]);
			}
			invMass1 = 1/molecularDynamics->forceField->molecule.atoms[a1].prop.masse;
			invMass2 = 1/molecularDynamics->forceField->molecule.atoms[a2].prop.masse;
		        term = -dot / ((invMass1+invMass2)*r2ij);
			printf("%d %d %s %s r2ij=%f r2Old=%f term=%f\n",
			a1,a2,
			molecularDynamics->forceField->molecule.atoms[a1].mmType,
			molecularDynamics->forceField->molecule.atoms[a2].mmType,
			r2ij, molecularDynamics->forceField->rattleConstraintsTerms[2][i],term);
		}
		StopCalcul=TRUE;
	}
	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
		if(!m->atoms[i].variable) for (k=0;k<3;k++) molecularDynamics->velocity[i][k] = 0;
}
/*********************************************************************************/
static void applyVerlet(MolecularDynamics* molecularDynamics)
{
	gint i;
	gint j;

	if(molecularDynamics->forceField->options.rattleConstraints!=NOCONSTRAINTS)
	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
		for ( j = 0; j < 3; j++)
				molecularDynamics->coordinatesOld[i][j]= molecularDynamics->forceField->molecule.atoms[i].coordinates[j];

	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		if(!molecularDynamics->forceField->molecule.atoms[i].variable) continue;

		for ( j = 0; j < 3; j++)
		{
			molecularDynamics->forceField->molecule.atoms[i].coordinates[j] += 
				molecularDynamics->velocity[i][j] * molecularDynamics->dt +
				molecularDynamics->a[i][j]*molecularDynamics->dt2_2;	
		}
		for ( j = 0; j < 3; j++)
			molecularDynamics->velocity[i][j] += molecularDynamics->a[i][j] * molecularDynamics->dt_2;
	}

	if(molecularDynamics->forceField->options.rattleConstraints!=NOCONSTRAINTS) applyRattleFirstPortion(molecularDynamics);

	newAccelaration(molecularDynamics);

	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		if(!molecularDynamics->forceField->molecule.atoms[i].variable) continue;
		for ( j = 0; j < 3; j++)
			molecularDynamics->velocity[i][j] += molecularDynamics->a[i][j] * molecularDynamics->dt_2;
	}
	if(molecularDynamics->forceField->options.rattleConstraints!=NOCONSTRAINTS) applyRattleSecondPortion(molecularDynamics);
}
/*********************************************************************************/
static void applyBeeman(MolecularDynamics* molecularDynamics)
{
	gint i;
	gint j;
	gdouble terms[3];

	if(molecularDynamics->forceField->options.rattleConstraints!=NOCONSTRAINTS)
	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
		for ( j = 0; j < 3; j++)
				molecularDynamics->coordinatesOld[i][j]= molecularDynamics->forceField->molecule.atoms[i].coordinates[j];


	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		if(!molecularDynamics->forceField->molecule.atoms[i].variable) continue;
		for ( j = 0; j < 3; j++)
			terms[j] = 5.0*molecularDynamics->a[i][j]-molecularDynamics->aold[i][j];

		for ( j = 0; j < 3; j++)
		{
			molecularDynamics->forceField->molecule.atoms[i].coordinates[j] += 
				molecularDynamics->velocity[i][j] * molecularDynamics->dt +
				terms[j]*molecularDynamics->dt2_8;	
		}
		for ( j = 0; j < 3; j++)
			molecularDynamics->velocity[i][j] += terms[j] * molecularDynamics->dt_8;
	}

	if(molecularDynamics->forceField->options.rattleConstraints!=NOCONSTRAINTS) applyRattleFirstPortion(molecularDynamics);

	newAccelaration(molecularDynamics);

	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		if(!molecularDynamics->forceField->molecule.atoms[i].variable) continue;
		for ( j = 0; j < 3; j++)
			molecularDynamics->velocity[i][j] += (3.0*molecularDynamics->a[i][j]+molecularDynamics->aold[i][j]) * molecularDynamics->dt_8;
	}

	if(molecularDynamics->forceField->options.rattleConstraints!=NOCONSTRAINTS) applyRattleSecondPortion(molecularDynamics);
}
/**********************************************************************/
static gdouble erfinv( gdouble y )
{
	static gdouble a[] = {0,  0.886226899, -1.645349621,  0.914624893, -0.140543331 };
	static gdouble b[] = {0, -2.118377725,  1.442710462, -0.329097515,  0.012229801 };
	static gdouble c[] = {0, -1.970840454, -1.624906493,  3.429567803,  1.641345311 };
	static gdouble d[] = {0,  3.543889200,  1.637067800 };
	gdouble x=1e100, z;
  
	if ( y < -1. ) return x;
	if ( y >  1. ) return x;
	if ( y >= -.7 )
	{
		if ( y <= .7 )
		{
			z = y*y;
			x = y * (((a[4]*z+a[3])*z+a[2])*z+a[1]) /
			  ((((b[4]*z+b[3])*z+b[2])*z+b[1])*z+1);
		}
		else if ( y < 1 )
		{
			z = sqrt(-log((1-y)/2));
			x = (((c[4]*z+c[3])*z+c[2])*z+c[1]) / ((d[2]*z+d[1])*z+1);
		}
	}
	else
	{
  		z = sqrt(-log((1+y)/2));
  		x = -(((c[4]*z+c[3])*z+c[2])*z+c[1]) / ((d[2]*z+d[1])*z+1);
	}
	return x;
}
/**********************************************************************/
static void getRandVect(gdouble len, gdouble V[])
{
	gdouble l = 0;
	gint j;
	for(j=0;j<3;j++)
	{
		V [j] = drandom();
		l += V[j]*V[j];
	}
	
	if(l<=0) return;
	l = sqrt(l);
	for(j=0;j<3;j++)
		V [j] *= len/l;
}
/**********************************************************************/
static gdouble	maxwel(gdouble masse, gdouble temperature)
{
	/* 
	 *  physical constants in SI units
	 *   ------------------------------
	 *      Kb = 1.380662 E-23 J/K
	 *      Na = 6.022045 E23  1/mol
	 *      e = 1.6021892 E-19 C
	 *      eps = 8.85418782 E-12 F/m
	 *                       
	 *      1 Kcal = 4184.0 J
	 *      1 amu = 1.6605655 E-27 Kg
	 *      1 A = 1.0 E-10 m
	 *                                       
	 *       Internally, AKMA units are used:
	 *       KBOLTZ = Na *Kb  / 1 Kcal
	 */
	/* gdouble Kb = 6.022045e23*1.380662e-23/4184.0;*/
	gdouble Kb = 1.9871914e-3;
	gdouble beta = sqrt(masse / (2.0*Kb*temperature));
	gdouble rho;
	gdouble xs, ys, zs;
	rho = drandom();
	xs = erfinv(rho)/beta;
	rho = drandom();
	ys = erfinv(rho)/beta;
	rho = drandom();
	zs = erfinv(rho)/beta;

	return sqrt(xs*xs+ys*ys+zs*zs);

}
/*********************************************************************************/
static void newProperties(MolecularDynamics* molecularDynamics, gchar* comments)
{
	if( molecularDynamics->fileProp == NULL) return;
	fprintf(molecularDynamics->fileProp,"time0(fs)\ttime(fs)\tTotal Energy(Kcal/mol)\tPotential Energy(kcal/mol) Kinetic Energy(Kcal/mol)\tT(t) (K)\tTaver(K)\tsigma(T)(K)");
	if(comments) fprintf(molecularDynamics->fileProp,"%s\n", comments);
	else fprintf(molecularDynamics->fileProp,"\n");
}
/*********************************************************************************/
static void saveProperties(MolecularDynamics* molecularDynamics, gint iStep0, gint iStep, gchar* comments)
{
	/* gdouble fsInAKMA = 1.0/sqrt(1e-10*1e-10*1.6605655e-27*6.022045e23/4184.0)/1e15;*/
	static gdouble fsInAKMA = 0.020454828110640;
	gdouble dt = molecularDynamics->dt/(fsInAKMA);
	static gdouble Ttot = 0;
	static gdouble T2tot = 0;
	gdouble Taver = 0;
	gdouble T2aver = 0;
	

	if( molecularDynamics->fileProp == NULL) return;
	if(iStep==1)
	{
			Ttot = 0;
			T2tot = 0;
	}
	Ttot += molecularDynamics->kelvin;
	T2tot += molecularDynamics->kelvin*molecularDynamics->kelvin;
	Taver = Ttot/iStep;
	T2aver = T2tot/iStep;


	fprintf(molecularDynamics->fileProp,"%f\t%f\t%f\t\t%f\t\t%f\t%f\t%f\t%f", 
			(iStep0)*dt, 
			(iStep)*dt, 
			molecularDynamics->totalEnergy,
			molecularDynamics->potentialEnergy,
			molecularDynamics->kineticEnergy,
			molecularDynamics->kelvin,
			Taver,
			sqrt(fabs(T2aver-Taver*Taver))
			 );
	if(comments) fprintf(molecularDynamics->fileProp,"%s\n", comments);
	else fprintf(molecularDynamics->fileProp,"\n");
}
/*********************************************************************************/
static void saveTrajectory(MolecularDynamics* molecularDynamics, gint iStep)
{
	/* gdouble fsInAKMA = 1.0/sqrt(1e-10*1e-10*1.6605655e-27*6.022045e23/4184.0)/1e15;*/
	static gdouble fsInAKMA = 0.020454828110640;
	gdouble dt = molecularDynamics->dt/(fsInAKMA);
	gint i;
	if( molecularDynamics->fileTraj == NULL) return;

	fprintf(molecularDynamics->fileTraj," %d %f %f %f %f nAtoms time(fs) TotalEnery(Kcal/mol) Kinetic Potential\n", 
			molecularDynamics->numberOfAtoms,
			 (iStep)*dt, 
			molecularDynamics->totalEnergy,
			molecularDynamics->kineticEnergy,
			molecularDynamics->potentialEnergy
			 );
	fprintf(molecularDynamics->fileTraj," %s\n", "Coord in Ang, Velocity in AKMA, time in fs");

	for (i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		fprintf(molecularDynamics->fileTraj," %s %f %f %f %f %f %f %f %s %s %s %d %d\n", 
				molecularDynamics->forceField->molecule.atoms[i].prop.symbol,
				molecularDynamics->forceField->molecule.atoms[i].coordinates[0],
				molecularDynamics->forceField->molecule.atoms[i].coordinates[1],
				molecularDynamics->forceField->molecule.atoms[i].coordinates[2],
				molecularDynamics->velocity[i][0],
				molecularDynamics->velocity[i][1],
				molecularDynamics->velocity[i][2],
				molecularDynamics->forceField->molecule.atoms[i].charge,
				molecularDynamics->forceField->molecule.atoms[i].mmType,
				molecularDynamics->forceField->molecule.atoms[i].pdbType,
				molecularDynamics->forceField->molecule.atoms[i].residueName,
				molecularDynamics->forceField->molecule.atoms[i].residueNumber,
				molecularDynamics->forceField->molecule.atoms[i].variable
				);
	}
}

/**********************************************************************/
void	freeMolecularDynamics(MolecularDynamics* molecularDynamics)
{

	molecularDynamics->forceField = NULL;
	molecularDynamics->numberOfAtoms = 0;
	molecularDynamics->updateFrequency = 0;
	if(molecularDynamics->velocity)
	{
		gint i;
		for(i=0;i<molecularDynamics->numberOfAtoms;i++)
			if(molecularDynamics->velocity[i]) g_free(molecularDynamics->velocity[i]);
		g_free(molecularDynamics->velocity);
	}
	if(molecularDynamics->a)
	{
		gint i;
		for(i=0;i<molecularDynamics->numberOfAtoms;i++)
			if(molecularDynamics->a[i]) g_free(molecularDynamics->a[i]);
		g_free(molecularDynamics->a);
	}
	if(molecularDynamics->aold)
	{
		gint i;
		for(i=0;i<molecularDynamics->numberOfAtoms;i++)
			if(molecularDynamics->aold[i]) g_free(molecularDynamics->aold[i]);
		g_free(molecularDynamics->aold);
	}
	if(molecularDynamics->coordinatesOld)
	{
		gint i;
		for(i=0;i<molecularDynamics->numberOfAtoms;i++)
			if(molecularDynamics->coordinatesOld[i]) g_free(molecularDynamics->coordinatesOld[i]);
		g_free(molecularDynamics->coordinatesOld);
	}
	if(molecularDynamics->moved) g_free(molecularDynamics->moved);
	if(molecularDynamics->update) g_free(molecularDynamics->update);
}
/********************************************************************************/
static gdouble getEKin(MolecularDynamics* molecularDynamics)
{
	gdouble ekin = 0;
	gint i;
	gint j;
	gdouble masse;
	for ( i = 0; i < molecularDynamics->numberOfAtoms; i++)
	{
		masse = molecularDynamics->forceField->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
			ekin += molecularDynamics->velocity[i][j]*molecularDynamics->velocity[i][j]*
				masse;
	}
	return ekin/2;
}
/********************************************************************************/
static gdouble getKelvin(MolecularDynamics* molecularDynamics)
{
        gint nfree = molecularDynamics->nfree;
	static gdouble Kb = 1.9871914e-3;
	if(nfree<1) return 0;
	return 2*getEKin(molecularDynamics) / ( nfree * Kb);
}
/********************************************************************************/
/*
     literature references:

     M. P. Allen, "Brownian Dynamics Simulation of a Chemical
     Reaction in Solution", Molecular Physics, 40, 1073-1087 (1980)

     F. Guarnieri and W. C. Still, "A Rapidly Convergent Simulation
     Method: Mixed Monte Carlo / Stochastic Dynamics", Journal of
     Computational Chemistry, 15, 1302-1310 (1994)
*/
/*********************************************************************************/
static void getsFrictionalAndRandomForce(MolecularDynamics* molecularDynamics)
{
	gdouble* gamma = molecularDynamics->gamma;
	gdouble* positionFriction = molecularDynamics->positionFriction;
	gdouble* velocityFriction = molecularDynamics->velocityFriction;
	gdouble* accelarationFriction = molecularDynamics->accelarationFriction;
	gdouble** positionRandom = molecularDynamics->positionRandom;
	gdouble** velocityRandom = molecularDynamics->velocityRandom;
	gdouble dt = molecularDynamics->dt;
	
	gint n = molecularDynamics->numberOfAtoms;

	gint i;
	gint j;
	gdouble gdt;
	gdouble egdt;
	gdouble ktm = 0;
	gdouble pterm;
	gdouble vterm;
        gdouble psig;
        gdouble vsig;
        gdouble rho;
        gdouble rhoc;
	gdouble pnorm;
	gdouble vnorm;
	static gdouble Kb = 1.9871914e-3;

	for(i=0;i<n;i++)
        	gamma[i] = molecularDynamics->friction;

	/* printf(" friction = %f\n", molecularDynamics->friction);*/
	for(i=0;i<n;i++)
	{
		gdt = gamma[i] * dt;
		/* printf("gdt = %f\n",gdt);*/
		if (gdt <= 0.0)
		{
               		positionFriction[i] = 1.0;
			velocityFriction[i] = dt;
			accelarationFriction[i] = 0.5 * dt * dt;
			for(j=0;j<3;j++)
			{
                  		positionRandom[i][j] = 0.0;
                  		velocityRandom[i][j] = 0.0;
			}
		}
            	else
		{
			/* analytical expressions when friction coefficient is large */
               		if (gdt>=0.05)
			{
                  		egdt = exp(-gdt);
                  		positionFriction[i] = egdt;
                  		velocityFriction[i] = (1.0-egdt) / gamma[i];
                  		accelarationFriction[i] = (dt-velocityFriction[i]) / gamma[i];
                  		pterm = 2.0*gdt - 3.0 + (4.0-egdt)*egdt;
                  		vterm = 1.0 - egdt*egdt;
                  		rho = (1.0-egdt)*(1.0-egdt) / sqrt(pterm*vterm);
			}
			/* use seriess expansions when friction coefficient is small */
			else
			{
                  		gdouble gdt2 = gdt * gdt;
                  		gdouble gdt3 = gdt * gdt2;
                  		gdouble gdt4 = gdt2 * gdt2;
                  		gdouble gdt5 = gdt2 * gdt3;
                  		gdouble gdt6 = gdt3 * gdt3;
                  		gdouble gdt7 = gdt3 * gdt4;
                  		gdouble gdt8 = gdt4 * gdt4;
                  		gdouble gdt9 = gdt4 * gdt5;
                  		accelarationFriction[i] = (gdt2/2.0 - gdt3/6.0 + gdt4/24.0
                               	- gdt5/120.0 + gdt6/720.0
                               	- gdt7/5040.0 + gdt8/40320.0
                               	- gdt9/362880.0) / gamma[i]/gamma[i];
                  		velocityFriction[i] = dt - gamma[i]*accelarationFriction[i];
                  		positionFriction[i] = 1.0 - gamma[i]*velocityFriction[i];
                  		pterm = 2.0*gdt3/3.0 - gdt4/2.0
                            	+ 7.0*gdt5/30.0 - gdt6/12.0
                            	+ 31.0*gdt7/1260.0 - gdt8/160.0
                            	+ 127.0*gdt9/90720.0;
                  		vterm = 2.0*gdt - 2.0*gdt2 + 4.0*gdt3/3.0
                            	- 2.0*gdt4/3.0 + 4.0*gdt5/15.0
                            	- 4.0*gdt6/45.0 + 8.0*gdt7/315.0
                            	- 2.0*gdt8/315.0 + 4.0*gdt9/2835.0;
                  		rho = sqrt(3.0) * (0.5 - 3.0*gdt/16.0
                            	- 17.0*gdt2/1280.0
                            	+ 17.0*gdt3/6144.0
                            	+ 40967.0*gdt4/34406400.0
                            	- 57203.0*gdt5/275251200.0
                            	- 1429487.0*gdt6/13212057600.0);
			}
               		ktm = Kb * molecularDynamics->temperature / molecularDynamics->forceField->molecule.atoms[i].prop.masse;
               		psig = sqrt(ktm*pterm) / gamma[i];
               		vsig = sqrt(ktm*vterm);
               		rhoc = sqrt(1.0 - rho*rho);
			for(j=0;j<3;j++)
			{
                		pnorm = normal();
             			vnorm = normal ();
				positionRandom[i][j] = psig * pnorm;
                  		velocityRandom[i][j] = vsig * (rho*pnorm+rhoc*vnorm);
			}
		}
	}
}
/*********************************************************************************/
static void applyStochastic(MolecularDynamics* molecularDynamics)
{
	gdouble* positionFriction = molecularDynamics->positionFriction;
	gdouble* velocityFriction = molecularDynamics->velocityFriction;
	gdouble* accelarationFriction = molecularDynamics->accelarationFriction;
	gdouble** positionRandom = molecularDynamics->positionRandom;
	gdouble** velocityRandom = molecularDynamics->velocityRandom;
	gdouble**v = molecularDynamics->velocity;
	gdouble**a = molecularDynamics->a;
	
	gint n = molecularDynamics->numberOfAtoms;
	gint i;
	gint j;
	AtomMol* atoms = molecularDynamics->forceField->molecule.atoms;

	getsFrictionalAndRandomForce(molecularDynamics);

	if(molecularDynamics->forceField->options.rattleConstraints!=NOCONSTRAINTS)
	for (i = 0; i < n; i++)
		for ( j = 0; j < 3; j++)
				molecularDynamics->coordinatesOld[i][j]= molecularDynamics->forceField->molecule.atoms[i].coordinates[j];


	for(i=0;i<n;i++)
	{
		if(!molecularDynamics->forceField->molecule.atoms[i].variable) continue;
		for(j=0;j<3;j++)
			atoms[i].coordinates[j] += v[i][j]*velocityFriction[i] + a[i][j]*accelarationFriction[i] + positionRandom[i][j];
		for(j=0;j<3;j++)
			v[i][j] = v[i][j]*positionFriction[i] + 0.5*a[i][j]*velocityFriction[i];
	}
	if(molecularDynamics->forceField->options.rattleConstraints!=NOCONSTRAINTS) applyRattleFirstPortion(molecularDynamics);
	newAccelaration(molecularDynamics);

	for (i = 0; i < n; i++)
	{
		if(!molecularDynamics->forceField->molecule.atoms[i].variable) continue;
		for ( j = 0; j < 3; j++)
			v[i][j] += 0.5*a[i][j]*velocityFriction[i] + velocityRandom[i][j];
	}
	if(molecularDynamics->forceField->options.rattleConstraints!=NOCONSTRAINTS) applyRattleSecondPortion(molecularDynamics);
	computeEnergies(molecularDynamics);
}
/*********************************************************************************/
static gdouble drandom()
{
	return (rand()/(gdouble)RAND_MAX);
}
/*********************************************************************************/
/*     "normal" generates a random number from a normal Gaussian
     distribution with a mean of zero and a variance of one
*/
static gdouble normal()
{
	gdouble v1,v2,rsq;
	gdouble factor;
	static gdouble store;
	static gboolean compute = TRUE;

	if (compute)
	{
		do{
         		v1 = 2.0 * drandom()  - 1.0;
         		v2 = 2.0 * drandom () - 1.0;
         		rsq = v1*v1 + v2*v2;
		}while(rsq >= 1.0);
		compute = FALSE;
		factor = sqrt(-2.0*log(rsq)/rsq);
		store = v1 * factor;
		return v2 * factor;
      }
/*     use the second random value computed at the last call */
      else
      {
		compute = TRUE;
		return store;
      }
}
/*********************************************************************************/
