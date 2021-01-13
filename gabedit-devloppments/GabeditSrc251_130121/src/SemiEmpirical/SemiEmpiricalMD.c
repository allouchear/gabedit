/* SemiEmpiricalMD.c  */
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
#include "AtomSE.h"
#include "MoleculeSE.h"
#include "SemiEmpiricalModel.h"
#include "SemiEmpiricalMD.h"


/*********************************************************************************/
static void initMD(SemiEmpiricalMD* seMD, gdouble temperature, gdouble stepSize, MDIntegratorType integratorType, MDThermostatType thermostat, gdouble friction, gdouble collide, gchar* fileNameTraj, gchar* fileNameProp, gint numberOfRunSteps);
static void berendsen(SemiEmpiricalMD* seMD);
static void bussi(SemiEmpiricalMD* seMD);
static void andersen(SemiEmpiricalMD* seMD);
static void rescaleVelocities(SemiEmpiricalMD* seMD);
static void computeEnergies(SemiEmpiricalMD* seMD);
static void applyOneStep(SemiEmpiricalMD* seMD);
static void applyVerlet(SemiEmpiricalMD* seMD);
static void applyBeeman(SemiEmpiricalMD* seMD);
static void applyStochastic(SemiEmpiricalMD* seMD);
static gdouble	maxwel(gdouble masse, gdouble temperature);
static void newProperties(SemiEmpiricalMD* seMD, gchar* comments);
static void saveProperties(SemiEmpiricalMD* seMD, gint iStep0, gint iStep, gchar* comments);
static void saveTrajectory(SemiEmpiricalMD* seMD, gint iStep);
static void getRandVect(gdouble len, gdouble V[]);
static gdouble getEKin(SemiEmpiricalMD* seMD);
static gdouble getKelvin(SemiEmpiricalMD* seMD);
static gdouble drandom();
static gdouble normal();
/**********************************************************************/
SemiEmpiricalModel**    runSemiEmpiricalMDConfo(
		SemiEmpiricalMD* seMD, SemiEmpiricalModel* seModel, 
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
	SemiEmpiricalModel** geometries = NULL;
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

	if(seModel->molecule.nAtoms<1) return NULL;
	if(numberOfGeometries<2) return NULL;
	geometries = g_malloc(numberOfGeometries*sizeof(SemiEmpiricalModel*));
	for(i=0;i<numberOfGeometries;i++) geometries[i] = NULL;

	seMD->seModel = seModel;
	seMD->numberOfAtoms = seModel->molecule.nAtoms;
	seMD->updateFrequency = updateFrequency;

	currentTemp = heatTemperature/2;
	
	numberOfHeatSteps = heatTime/stepSize*1000;
	numberOfEquiSteps = equiTime/stepSize*1000;; 
	numberOfRunSteps = runTime/stepSize*1000;; 


	currentTemp = heatTemperature;
	if(numberOfHeatSteps==0) currentTemp = equiTemperature; 
	if(numberOfHeatSteps==0 && numberOfEquiSteps==0 ) currentTemp = runTemperature; 

	initMD(seMD,currentTemp,stepSize,integratorType, thermostat, friction, collide, fileNameTraj, fileNameProp, numberOfRunSteps);
	seMD->seModel->klass->calculateGradient(seMD->seModel);
	computeEnergies(seMD);

	iSel =-1;
	if((i+1)%stepSel==0 && (iSel+1)<numberOfGeometries)
	{
		if(str) g_free(str);
		str = g_strdup_printf(_("Geometry selected Potential energy =  %0.4f"), seMD->potentialEnergy);
		redrawMoleculeSE(&seMD->seModel->molecule,str);
		iSel++;
		geometries[iSel] = g_malloc(sizeof(SemiEmpiricalModel));
		*geometries[iSel] = copySemiEmpiricalModel(seMD->seModel);
		Waiting(1);
	}

	seMD->temperature = heatTemperature;
	rescaleVelocities(seMD);

	currentTemp = heatTemperature;
	n0 = 0;
	newProperties(seMD," ");
	/*newProperties(seMD," ----> Heating");*/
	for (i = 0; i < numberOfHeatSteps; i++ )
	{
		seMD->temperature = currentTemp;
		applyOneStep(seMD);
		currentTemp = heatTemperature + ( runTemperature - heatTemperature ) *
				( ( gdouble )( i + 1 )/ numberOfHeatSteps );
		seMD->temperature = currentTemp;
		rescaleVelocities(seMD);
		if(StopCalcul) break;
		if (++updateNumber >= seMD->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Heating: %0.2f fs, T = %0.2f K T(t) = %0.2f Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					seMD->kelvin, 
					seMD->kineticEnergy,
					seMD->potentialEnergy,
					seMD->totalEnergy
					);
			redrawMoleculeSE(&seMD->seModel->molecule,str);
			updateNumber = 0;
		}
		saveProperties(seMD, n0+i+1, i+1," Heating");
	}

	currentTemp = equiTemperature;
	seMD->temperature = currentTemp;
	rescaleVelocities(seMD);
	if(StopCalcul) numberOfEquiSteps =0;
	if(StopCalcul) numberOfRunSteps =0;
	updateNumber = seMD->updateFrequency;
	n0 += numberOfHeatSteps;
	/* newProperties(seMD," ----> Equilibrium");*/
	for (i = 0; i < numberOfEquiSteps; i++ )
	{
		seMD->temperature = currentTemp;
		applyOneStep(seMD);
		seMD->temperature = currentTemp;
		rescaleVelocities(seMD);
		if(StopCalcul) break;
		if (++updateNumber >= seMD->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Equilibrium: %0.2f fs, T = %0.2f K  T(t) = %0.2f K Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					seMD->kelvin, 
					seMD->kineticEnergy,
					seMD->potentialEnergy,
					seMD->totalEnergy
					);
			redrawMoleculeSE(&seMD->seModel->molecule,str);
			updateNumber = 0;
		}
		saveProperties(seMD, n0+i+1, i+1, " Equilibrium");
	}
	updateNumber = seMD->updateFrequency;

	currentTemp = runTemperature;
	seMD->temperature = currentTemp;
	rescaleVelocities(seMD);
	if(StopCalcul) numberOfRunSteps =0;
	updateNumber = seMD->updateFrequency;
	n0 += numberOfEquiSteps;
	/* newProperties(seMD," ----> Runing");*/
	if(str) g_free(str);
	str = g_strdup_printf(_("Geometry selected Potential energy =  %0.4f"), seMD->potentialEnergy);
	redrawMoleculeSE(&seMD->seModel->molecule,str);
	if(numberOfGeometries>2) stepSel = numberOfRunSteps/numberOfGeometries;
	else stepSel = numberOfRunSteps;
	/* printf("Isel = %d\n",stepSel);*/
	for (i = 0; i < numberOfRunSteps; i++ )
	{
		seMD->temperature = currentTemp;
		applyOneStep(seMD);
		if(seMD->thermostat == ANDERSEN) andersen(seMD);
		if(seMD->thermostat == BERENDSEN) berendsen(seMD);
		if(seMD->thermostat == BUSSI) bussi(seMD);
		if(StopCalcul) break;
		if (++updateNumber >= seMD->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Running: %0.2f fs, T = %0.2f K  T(t) = %0.2f K Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					seMD->kelvin, 
					seMD->kineticEnergy,
					seMD->potentialEnergy,
					seMD->totalEnergy
					);
			redrawMoleculeSE(&seMD->seModel->molecule,str);
			updateNumber = 0;
			saveTrajectory(seMD, i+1);
		}
		if((i+1)%stepSel==0 && (iSel+1)<numberOfGeometries)
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("Geometry selected Potential energy =  %0.4f"), seMD->potentialEnergy);
			redrawMoleculeSE(&seMD->seModel->molecule,str);
			iSel++;
			geometries[iSel] = g_malloc(sizeof(SemiEmpiricalModel));
			*geometries[iSel] = copySemiEmpiricalModel(seMD->seModel);
			Waiting(1);
		}
		saveProperties(seMD, n0+i+1, i+1," Running");
	}
	if(iSel<numberOfGeometries-1)
	{
		if(str) g_free(str);
		str = g_strdup_printf(_("Geometry selected Potential energy =  %0.4f"), seMD->potentialEnergy);
		redrawMoleculeSE(&seMD->seModel->molecule,str);
		iSel++;
		geometries[iSel] = g_malloc(sizeof(SemiEmpiricalModel));
		*geometries[iSel] = copySemiEmpiricalModel(seMD->seModel);
		Waiting(1);
	}

	updateNumber = seMD->updateFrequency;
	n0 += numberOfRunSteps;

	seMD->seModel->klass->calculateGradient(seMD->seModel);
        gradientNorm = 0;
	for (i = 0; i < seMD->numberOfAtoms; i++)
		for ( j = 0; j < 3; j++)
                        gradientNorm += 
				seMD->seModel->molecule.gradient[j][i] * 
				seMD->seModel->molecule.gradient[j][i]; 

        gradientNorm = sqrt( gradientNorm );
	if(str) g_free(str);
	str = g_strdup_printf(_("End of MD Simulation. Gradient = %f Ekin = %f (Kcal/mol) EPot =  %0.4f ETot =  %0.4f T(t) = %0.2f"),
			(gdouble)gradientNorm,
			seMD->kineticEnergy,
			seMD->potentialEnergy,
			seMD->totalEnergy,
			seMD->kelvin 
			); 
	redrawMoleculeSE(&seMD->seModel->molecule,str);
	g_free(str);
	if(seMD->fileTraj)fclose(seMD->fileTraj);
	if(seMD->fileProp)fclose(seMD->fileProp);
	freeSemiEmpiricalMD(seMD);
	return geometries;
}
/**********************************************************************/
void	runSemiEmpiricalMD(
		SemiEmpiricalMD* seMD, SemiEmpiricalModel* seModel, 
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

	if(seModel->molecule.nAtoms<1) return;

	seMD->seModel = seModel;
	seMD->numberOfAtoms = seModel->molecule.nAtoms;
	seMD->updateFrequency = updateFrequency;

	currentTemp = heatTemperature/2;
	
	numberOfHeatSteps = heatTime/stepSize*1000;
	numberOfEquiSteps = equiTime/stepSize*1000;; 
	numberOfRunSteps = runTime/stepSize*1000;; 
	numberOfCoolSteps = coolTime/stepSize*1000;;


	currentTemp = heatTemperature;
	if(numberOfHeatSteps==0) currentTemp = equiTemperature; 
	if(numberOfHeatSteps==0 && numberOfEquiSteps==0 ) currentTemp = runTemperature; 
	if(numberOfHeatSteps==0 && numberOfEquiSteps==0 && numberOfRunSteps==0 ) currentTemp = coolTemperature; 

	initMD(seMD,currentTemp,stepSize,integratorType, thermostat, friction, collide, fileNameTraj, fileNameProp, numberOfRunSteps);
	seMD->seModel->klass->calculateGradient(seMD->seModel);

	seMD->temperature = heatTemperature;
	rescaleVelocities(seMD);

	currentTemp = heatTemperature;
	n0 = 0;
	newProperties(seMD," ");
	/*newProperties(seMD," ----> Heating");*/
	for (i = 0; i < numberOfHeatSteps; i++ )
	{
		seMD->temperature = currentTemp;
		applyOneStep(seMD);
		currentTemp = heatTemperature + ( runTemperature - heatTemperature ) *
				( ( gdouble )( i + 1 )/ numberOfHeatSteps );
		seMD->temperature = currentTemp;
		rescaleVelocities(seMD);
		if(StopCalcul) break;
		if (++updateNumber >= seMD->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Heating: %0.2f fs, T = %0.2f K T(t) = %0.2f Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					seMD->kelvin, 
					seMD->kineticEnergy,
					seMD->potentialEnergy,
					seMD->totalEnergy
					);
			redrawMoleculeSE(&seMD->seModel->molecule,str);
			updateNumber = 0;
		}
		saveProperties(seMD, n0+i+1, i+1," Heating");
	}

	currentTemp = equiTemperature;
	seMD->temperature = currentTemp;
	rescaleVelocities(seMD);
	if(StopCalcul) numberOfEquiSteps =0;
	if(StopCalcul) numberOfRunSteps =0;
	if(StopCalcul) numberOfCoolSteps =0;
	updateNumber = seMD->updateFrequency;
	n0 += numberOfHeatSteps;
	/* newProperties(seMD," ----> Equilibrium");*/
	for (i = 0; i < numberOfEquiSteps; i++ )
	{
		seMD->temperature = currentTemp;
		applyOneStep(seMD);
		seMD->temperature = currentTemp;
		rescaleVelocities(seMD);
		if(StopCalcul) break;
		if (++updateNumber >= seMD->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Equilibrium: %0.2f fs, T = %0.2f K  T(t) = %0.2f K Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					seMD->kelvin, 
					seMD->kineticEnergy,
					seMD->potentialEnergy,
					seMD->totalEnergy
					);
			redrawMoleculeSE(&seMD->seModel->molecule,str);
			updateNumber = 0;
		}
		saveProperties(seMD, n0+i+1, i+1, " Equilibrium");
	}
	updateNumber = seMD->updateFrequency;

	currentTemp = runTemperature;
	seMD->temperature = currentTemp;
	rescaleVelocities(seMD);
	if(StopCalcul) numberOfRunSteps =0;
	if(StopCalcul) numberOfCoolSteps =0;
	updateNumber = seMD->updateFrequency;
	n0 += numberOfEquiSteps;
	/* newProperties(seMD," ----> Runing");*/
	for (i = 0; i < numberOfRunSteps; i++ )
	{
		seMD->temperature = currentTemp;
		applyOneStep(seMD);
		if(seMD->thermostat == ANDERSEN) andersen(seMD);
		if(seMD->thermostat == BERENDSEN) berendsen(seMD);
		if(seMD->thermostat == BUSSI) bussi(seMD);
		if(StopCalcul) break;
		if (++updateNumber >= seMD->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Running: %0.2f fs, T = %0.2f K  T(t) = %0.2f K Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					seMD->kelvin, 
					seMD->kineticEnergy,
					seMD->potentialEnergy,
					seMD->totalEnergy
					);
			redrawMoleculeSE(&seMD->seModel->molecule,str);
			updateNumber = 0;
			saveTrajectory(seMD, i+1);
		}
		saveProperties(seMD, n0+i+1, i+1," Running");
	}

	updateNumber = seMD->updateFrequency;
	if(StopCalcul) numberOfCoolSteps =0;
	n0 += numberOfRunSteps;
	/* newProperties(seMD," ----> Cooling");*/
	for (i = 0; i < numberOfCoolSteps; i++ )
	{
		currentTemp = runTemperature - ( runTemperature - coolTemperature ) * 
				( ( gdouble )( i + 1 )/ numberOfCoolSteps );
		seMD->temperature = currentTemp;
		rescaleVelocities(seMD);
		seMD->temperature = currentTemp;
		applyOneStep(seMD);
		if(StopCalcul) break;
		if (++updateNumber >= seMD->updateFrequency )
		{
			if(str) g_free(str);
			str = g_strdup_printf(_("MD Cooling: %0.2f fs, T = %0.2f K T(t) = %0.2f K Kin = %0.4f Pot =  %0.4f Tot =  %0.4f"), 
					i*stepSize, currentTemp, 
					seMD->kelvin, 
					seMD->kineticEnergy,
					seMD->potentialEnergy,
					seMD->totalEnergy
					);
			redrawMoleculeSE(&seMD->seModel->molecule,str);
			updateNumber = 0;
		}
		saveProperties(seMD, n0+i+1, i+1," Cooling");
	}
	seMD->seModel->klass->calculateGradient(seMD->seModel);
        gradientNorm = 0;
	for (i = 0; i < seMD->numberOfAtoms; i++)
		for ( j = 0; j < 3; j++)
                        gradientNorm += 
				seMD->seModel->molecule.gradient[j][i] * 
				seMD->seModel->molecule.gradient[j][i]; 

        gradientNorm = sqrt( gradientNorm );
	if(str) g_free(str);
	str = g_strdup_printf(_("End of MD Simulation. Gradient = %f Ekin = %f (Kcal/mol) EPot =  %0.4f ETot =  %0.4f T(t) = %0.2f"),
			(gdouble)gradientNorm,
			seMD->kineticEnergy,
			seMD->potentialEnergy,
			seMD->totalEnergy,
			seMD->kelvin 
			); 
	redrawMoleculeSE(&seMD->seModel->molecule,str);
	g_free(str);
	if(seMD->fileTraj)fclose(seMD->fileTraj);
	if(seMD->fileProp)fclose(seMD->fileProp);
	freeSemiEmpiricalMD(seMD);
}
/*********************************************************************************/
static void initSD(SemiEmpiricalMD* seMD, gdouble friction)
{
	/* gdouble fsInAKMA = 1.0/sqrt(1e-10*1e-10*1.6605655e-27*6.022045e23/4184.0)/1e15;*/
	static gdouble fsInAKMA = 0.020454828110640;
	gint i;


	seMD->friction = friction/(fsInAKMA)/1000;

	seMD->positionFriction = NULL;
	seMD->velocityFriction = NULL;
	seMD->accelarationFriction = NULL;
	seMD->gamma = NULL;
	seMD->positionRandom = NULL;
	seMD->velocityRandom = NULL;

	if(seMD->integratorType != STOCHASTIC) return;

	seMD->positionFriction = g_malloc(seMD->numberOfAtoms *sizeof(gdouble)); 
	seMD->velocityFriction = g_malloc(seMD->numberOfAtoms *sizeof(gdouble)); 
	seMD->accelarationFriction = g_malloc(seMD->numberOfAtoms *sizeof(gdouble)); 
	seMD->gamma = g_malloc(seMD->numberOfAtoms *sizeof(gdouble)); 

	seMD->positionRandom = g_malloc(seMD->numberOfAtoms *sizeof(gdouble*)); 
	for(i=0;i<seMD->numberOfAtoms;i++)
		seMD->positionRandom[i] = g_malloc(3*sizeof(gdouble));

	seMD->velocityRandom = g_malloc(seMD->numberOfAtoms *sizeof(gdouble*)); 
	for(i=0;i<seMD->numberOfAtoms;i++)
		seMD->velocityRandom[i] = g_malloc(3*sizeof(gdouble));

}
/*********************************************************************************/
/*
static void printTranslation(SemiEmpiricalMD* seMD)
{
	gdouble vtot[3] = {0,0,0};
	gint i;
	gint j;
	gdouble mass = 1.0;
	for ( j = 0; j < 3; j++)
		vtot[j] = 0;
	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		mass = seMD->seModel->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
		{
			vtot[j] += mass*seMD->velocity[i][j];
		}
	}
	printf("Trans velocity = %f %f %f\n",vtot[0], vtot[1], vtot[2]);
}
*/
/*********************************************************************************/
static void removeTranslation(SemiEmpiricalMD* seMD)
{
	gdouble vtot[3] = {0,0,0};
	gint i;
	gint j;
	gdouble mass = 1.0;
	gdouble totMass = 0.0;
	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		mass = seMD->seModel->molecule.atoms[i].prop.masse;
		totMass += mass;
		for ( j = 0; j < 3; j++)
		{
			vtot[j] += mass*seMD->velocity[i][j];
		}
	}

	for ( j = 0; j < 3; j++)
		vtot[j] /= totMass;

	for ( i = 0; i < seMD->numberOfAtoms; i++)
		for ( j = 0; j < 3; j++)
			seMD->velocity[i][j] -= vtot[j];
	/* check */
	/*
	for ( j = 0; j < 3; j++)
		vtot[j] = 0;
	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		mass = seMD->seModel->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
		{
			vtot[j] += mass*seMD->velocity[i][j];
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
/*
static void printRotation(SemiEmpiricalMD* seMD)
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
	AtomSE* atoms = seMD->seModel->molecule.atoms;


	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		mass = seMD->seModel->molecule.atoms[i].prop.masse;
		totMass += mass;
		for ( j = 0; j < 3; j++)
			cm[j] += mass*atoms[i].coordinates[j];
		for ( j = 0; j < 3; j++)
			vtot[j] += mass*seMD->velocity[i][j];
	}


	for ( j = 0; j < 3; j++)
		cm[j] /= totMass;
	for ( j = 0; j < 3; j++)
		vtot[j] /= totMass;

	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		mass = seMD->seModel->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
			L[j] += (
				atoms[i].coordinates[(j+1)%3]*seMD->velocity[i][(j+2)%3]
			      - atoms[i].coordinates[(j+2)%3]*seMD->velocity[i][(j+1)%3]
			      )*mass;
	}
	for ( j = 0; j < 3; j++)
		L[j] -= (
			cm[(j+1)%3]*vtot[(j+2)%3]
		      - cm[(j+2)%3]*vtot[(j+1)%3]
			      )*totMass;

	for ( k = 0; k < 3; k++)
	for ( j = 0; j < 3; j++)
		tensor[k][j] = 0;
	xx = 0;
	yy = 0;
	zz = 0;
	xy = 0;
	xz = 0;
	yz = 0;
	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		mass = seMD->seModel->molecule.atoms[i].prop.masse;
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
		printf("Angular velocity = %f %f %f\n",vAng[0], vAng[1], vAng[2]);
	}
}
*/
/*********************************************************************************/
static void removeRotation(SemiEmpiricalMD* seMD)
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
	AtomSE* atoms = seMD->seModel->molecule.atoms;


	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		mass = seMD->seModel->molecule.atoms[i].prop.masse;
		totMass += mass;
		for ( j = 0; j < 3; j++)
			cm[j] += mass*atoms[i].coordinates[j];
		for ( j = 0; j < 3; j++)
			vtot[j] += mass*seMD->velocity[i][j];
	}


	for ( j = 0; j < 3; j++)
		cm[j] /= totMass;
	for ( j = 0; j < 3; j++)
		vtot[j] /= totMass;

	/*   compute the angular momentum  */
	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		mass = seMD->seModel->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
			L[j] += (
				atoms[i].coordinates[(j+1)%3]*seMD->velocity[i][(j+2)%3]
			      - atoms[i].coordinates[(j+2)%3]*seMD->velocity[i][(j+1)%3]
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
	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		mass = seMD->seModel->molecule.atoms[i].prop.masse;
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
		for ( i = 0; i < seMD->numberOfAtoms; i++)
		{
			for ( j = 0; j < 3; j++)
				cdel[j] = atoms[i].coordinates[j]-cm[j];
			for ( j = 0; j < 3; j++)
				seMD->velocity[i][j] += 
					cdel[(j+1)%3]*vAng[(j+2)%3]-
					cdel[(j+2)%3]*vAng[(j+1)%3];
		}
	}
	/*   check  */
	/*
	for ( j = 0; j < 3; j++)
		L[j] = 0;
	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		mass = seMD->seModel->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
			L[j] += (
				atoms[i].coordinates[(j+1)%3]*seMD->velocity[i][(j+2)%3]
			      - atoms[i].coordinates[(j+2)%3]*seMD->velocity[i][(j+1)%3]
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
static void removeTranslationAndRotation(SemiEmpiricalMD* seMD)
{
	removeTranslation(seMD);
	removeRotation(seMD);
}
/*********************************************************************************/
static void initMD(SemiEmpiricalMD* seMD, gdouble temperature, gdouble stepSize, MDIntegratorType integratorType, MDThermostatType thermostat, gdouble friction, gdouble collide, gchar* fileNameTraj, gchar* fileNameProp, gint numberOfRunSteps)
{
	gint i;
	gint j;
	/* gdouble fsInAKMA = 1.0/sqrt(1e-10*1e-10*1.6605655e-27*6.022045e23/4184.0)/1e15;*/
	static gdouble fsInAKMA = 0.020454828110640;
	gdouble dt = stepSize * fsInAKMA;

	seMD->collide = collide;
	seMD->potentialEnergy = 0;
	seMD->kineticEnergy = 0;
	seMD->totalEnergy = 0;
	seMD->kelvin = 0;
	seMD->temperature = temperature;
	seMD->thermostat = NONE;

	seMD->integratorType = integratorType;
	seMD->thermostat = thermostat;
	seMD->fileTraj = NULL;
	seMD->fileProp = NULL;

	seMD->velocity = g_malloc(seMD->numberOfAtoms *sizeof(gdouble*)); 
	for(i=0;i<seMD->numberOfAtoms;i++)
		seMD->velocity[i] = g_malloc(3*sizeof(gdouble));

	seMD->a = g_malloc(seMD->numberOfAtoms *sizeof(gdouble*)); 
	for(i=0;i<seMD->numberOfAtoms;i++)
		seMD->a[i] = g_malloc(3*sizeof(gdouble));

	seMD->aold = NULL;
	if(seMD->integratorType==BEEMAN)
	{
		seMD->aold = g_malloc(seMD->numberOfAtoms *sizeof(gdouble*)); 
		for(i=0;i<seMD->numberOfAtoms;i++)
			seMD->aold[i] = g_malloc(3*sizeof(gdouble));
	}
	seMD->coordinatesOld = NULL;
	seMD->moved = NULL;
	seMD->update = NULL;
	if(seMD->seModel->constraints!=NOCONSTRAINTS)
	{
		seMD->coordinatesOld = g_malloc(seMD->numberOfAtoms *sizeof(gdouble*)); 
		for(i=0;i<seMD->numberOfAtoms;i++)
			seMD->coordinatesOld[i] = g_malloc(3*sizeof(gdouble));
		seMD->moved = g_malloc(seMD->numberOfAtoms *sizeof(gboolean)); 
		seMD->update = g_malloc(seMD->numberOfAtoms *sizeof(gboolean)); 

	}
	if(fileNameTraj)
	{
 		seMD->fileTraj = FOpen(fileNameTraj, "w");
		if(seMD->fileTraj != NULL)
		{
			fprintf(seMD->fileTraj,"[Gabedit Format]\n");
			fprintf(seMD->fileTraj,"\n");
			fprintf(seMD->fileTraj,"[MD]\n");
			if(seMD->updateFrequency>0) numberOfRunSteps/=seMD->updateFrequency;
			fprintf(seMD->fileTraj," %d\n",numberOfRunSteps);
		}
	}
	if(fileNameProp)
	{
 		seMD->fileProp = FOpen(fileNameProp, "w");
	}

	srand ( (unsigned)time (NULL));
	
	seMD->dt = dt;
	seMD->dt_2 = dt/2.0;
	seMD->dt2_2 = dt*dt/2;;
	seMD->dt_8 = dt/8.0;
	seMD->dt2_8 = dt*dt/8.0;

	initSD(seMD, friction);


	seMD->seModel->klass->calculateGradient(seMD->seModel);
	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		gdouble m = seMD->seModel->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
			seMD->a[i][j] = -seMD->seModel->molecule.gradient[j][i]/m;
		if(seMD->aold)
			for ( j = 0; j < 3; j++)
				seMD->aold[i][j]  = seMD->a[i][j];
	}

	if(temperature<=0)
	{
		for ( i = 0; i < seMD->numberOfAtoms; i++)
			for ( j = 0; j < 3; j++)
				seMD->velocity[i][j] = 0.0;
	}
	else
	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		if(seMD->seModel->molecule.atoms[i].variable)
		{
			gdouble speed = maxwel(seMD->seModel->molecule.atoms[i].prop.masse,temperature);
			getRandVect(speed, seMD->velocity[i]);
		}
		else
		{
			for ( j = 0; j < 3; j++)
				seMD->velocity[i][j] = 0.0;
		}
	}
        seMD->nvariables = 0;
	for ( i = 0; i < seMD->numberOfAtoms; i++)
		if(seMD->seModel->molecule.atoms[i].variable) seMD->nvariables +=1;
        if(seMD->nvariables==0) 
	{
		seMD->nvariables= seMD->numberOfAtoms;
		for ( i = 0; i < seMD->numberOfAtoms; i++)
			seMD->seModel->molecule.atoms[i].variable = TRUE;
		if(temperature>0)
		for ( i = 0; i < seMD->numberOfAtoms; i++)
		{
			gdouble speed = maxwel(seMD->seModel->molecule.atoms[i].prop.masse,temperature);
			getRandVect(speed, seMD->velocity[i]);
		}
	}
        seMD->nfree = 3* seMD->nvariables-seMD->seModel->numberOfRattleConstraintsTerms;
        removeTranslationAndRotation(seMD);
        if(seMD->nvariables==seMD->numberOfAtoms) seMD->nfree -=6;
        if(seMD->nvariables==seMD->numberOfAtoms-1) seMD->nfree -=3;
        if(seMD->nvariables==seMD->numberOfAtoms-2) seMD->nfree -=1;
        if( seMD->nfree<1)
	{ 
		StopCalcul = TRUE;
        	seMD->nfree = 1;
	}

	removeTranslationAndRotation(seMD);
}
/*********************************************************************************/
static void rescaleVelocities(SemiEmpiricalMD* seMD)
{
	berendsen(seMD);
}
/*********************************************************************************/
static void berendsen(SemiEmpiricalMD* seMD)
{
	gint i;
	gint j;
	static gdouble fsInAKMA = 0.020454828110640;
	gdouble ekin = 0;
	gdouble kelvin = 0;
	gint nfree = seMD->nfree;
	static gdouble Kb = 1.9871914e-3;
	gdouble scale = 1.0;
	gdouble dt = seMD->dt;
	gdouble tautemp = 1.0/(seMD->collide)*1000*fsInAKMA;
	gdouble masse = 1.0;
	if(seMD->temperature<=0) return;
	if(nfree<1) return;
	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		masse = seMD->seModel->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
			ekin += seMD->velocity[i][j]*seMD->velocity[i][j]*
				masse;
	}
	/*
	ekin /= 2;
	kelvin = 2* ekin / ( nfree * Kb);
	*/
	kelvin = ekin / ( nfree * Kb);
	/* if(tautemp>dt) tautemp = dt;*/
	scale = sqrt(1.0 + (dt/tautemp)*(seMD->temperature/kelvin-1.0));
	/* printf("temp = %f kelvin = %f scale = %f\n",seMD->temperature, kelvin, scale);*/
	for ( i = 0; i < seMD->numberOfAtoms; i++)
		if(seMD->seModel->molecule.atoms[i].variable)
		for ( j = 0; j < 3; j++)
			seMD->velocity[i][j] *= scale;
	removeTranslationAndRotation(seMD);
}
/*********************************************************************************/
static void andersen(SemiEmpiricalMD* seMD)
{
	gint i;
	/* gdouble fsInAKMA = 1.0/sqrt(1e-10*1e-10*1.6605655e-27*6.022045e23/4184.0)/1e15;*/
	static gdouble fsInAKMA = 0.020454828110640;
	gdouble tau = 1.0/seMD->collide*1000*fsInAKMA; /* in fs */
	gdouble rate;
	static gdouble Kb = 1.9871914e-3;
	if(seMD->temperature<=0) return;
	if(seMD->numberOfAtoms<1) return;

	rate = seMD->dt / tau;
	rate /= pow(seMD->nvariables,2.0/3.0);

	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		gdouble trial = drandom();
		if(seMD->seModel->molecule.atoms[i].variable)
		if(trial<rate)
		{
/*
			gdouble speed = maxwel(
					seMD->seModel->molecule.atoms[i].prop.masse,
					seMD->temperature
					);
			getRandVect(speed, seMD->velocity[i]);
*/
			double speed = sqrt(Kb* seMD->temperature/seMD->seModel->molecule.atoms[i].prop.masse);
                	double pnorm = normal();
			seMD->velocity[i][0] = pnorm*speed;
                	pnorm = normal();
			seMD->velocity[i][1] = pnorm*speed;
                	pnorm = normal();
			seMD->velocity[i][2] = pnorm*speed;
		}
	}
}
/*********************************************************************************/
static void bussi(SemiEmpiricalMD* seMD)
{
        static gdouble fsInAKMA = 0.020454828110640;
        gint nfree = seMD->nfree;
        static gdouble Kb = 1.9871914e-3;
        gdouble scale = 1.0;
        gdouble dt = seMD->dt;
        gdouble tautemp = 1.0/(seMD->collide)*1000*fsInAKMA;
        gdouble c = exp(-dt/tautemp);
        gdouble ekin = getEKin(seMD);
        gdouble kelvin = 2*ekin / ( nfree * Kb);
        gdouble d = (1.0-c) * (seMD->temperature/kelvin) / (nfree);
        gdouble r = normal ();
        gdouble si = 0.0;
        gdouble s = 0.0;
        gint i,j;
        if(seMD->temperature<=0) return;
        if(nfree<1) return;
        for(i=0;i<nfree-1;i++)
        {
            si = normal ();
            s += si*si;
        }
        scale = c + (s+r*r)*d + 2.0*r*sqrt(c*d);
        scale = sqrt(scale);
        if (r+sqrt(c/d)<0)  scale = -scale;
        for ( i = 0; i < seMD->numberOfAtoms; i++)
		if(seMD->seModel->molecule.atoms[i].variable)
                for ( j = 0; j < 3; j++)
                        seMD->velocity[i][j] *= scale;
        removeTranslationAndRotation(seMD);
}
/*********************************************************************************/
static void newAccelaration(SemiEmpiricalMD* seMD)
{
	gint i;
	gint j;
	seMD->seModel->klass->calculateGradient(seMD->seModel);
	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		gdouble m = seMD->seModel->molecule.atoms[i].prop.masse;
		if(seMD->aold)
			for ( j = 0; j < 3; j++)
				seMD->aold[i][j]  = seMD->a[i][j];

		for ( j = 0; j < 3; j++)
			seMD->a[i][j] = -seMD->seModel->molecule.gradient[j][i]/m;
	}
}
/*********************************************************************************/
static void computeEnergies(SemiEmpiricalMD* seMD)
{
	seMD->kineticEnergy = getEKin(seMD);
	seMD->potentialEnergy = seMD->seModel->molecule.energy;
	seMD->totalEnergy = seMD->kineticEnergy + seMD->potentialEnergy;
	seMD->kelvin = getKelvin(seMD);
}
/*********************************************************************************/
static void applyOneStep(SemiEmpiricalMD* seMD)
{
	if(seMD->integratorType==VERLET) applyVerlet(seMD);
	else if(seMD->integratorType==BEEMAN) applyBeeman(seMD);
	else applyStochastic(seMD);
	computeEnergies(seMD);
	/*
	printTranslation(seMD);
	printRotation(seMD);
	*/
	/* removeTranslationAndRotation(seMD);*/

}
/*********************************************************************************/
static void applyRattleFirstPortion(SemiEmpiricalMD* semiEmpiricalMD)
{
	gint i;
	gint k;
	gint maxIter = 100;
	gdouble omega = 1.2; 
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
	MoleculeSE* m = &semiEmpiricalMD->seModel->molecule;
	SemiEmpiricalModel* seModel = semiEmpiricalMD->seModel;
	gdouble deltaMax = 0;

	if(seModel->constraints==NOCONSTRAINTS) return;
	for (i = 0; i < semiEmpiricalMD->numberOfAtoms; i++)
	{
			semiEmpiricalMD->moved[i] = semiEmpiricalMD->seModel->molecule.atoms[i].variable;
			semiEmpiricalMD->update[i] = FALSE;
	}
	maxIter *= semiEmpiricalMD->seModel->numberOfRattleConstraintsTerms;
	do{
		nIter++;
		done=TRUE;
		deltaMax = 0;
		for (i = 0; i < semiEmpiricalMD->seModel->numberOfRattleConstraintsTerms; i++)
		{
			a1 = (gint)semiEmpiricalMD->seModel->rattleConstraintsTerms[0][i];
			a2 = (gint)semiEmpiricalMD->seModel->rattleConstraintsTerms[1][i];
			if( !semiEmpiricalMD->moved[a1] && !semiEmpiricalMD->moved[a2] ) continue;
			r2ij = 0;
			for (k=0;k<3;k++)
			{
				d = m->atoms[a2].coordinates[k]-m->atoms[a1].coordinates[k];
				r2ij +=d*d;
			}
			delta = semiEmpiricalMD->seModel->rattleConstraintsTerms[2][i]-r2ij;
			if(deltaMax<fabs(delta)) deltaMax = fabs(delta);
			if(fabs(delta)<=tolerance) continue;
			done = FALSE;
			semiEmpiricalMD->update[a1] = TRUE;
			semiEmpiricalMD->update[a2] = TRUE;
			/* here : rattle image for PBC, not yet implemented */
			dot = 0;
			for (k=0;k<3;k++)
			{
				d = m->atoms[a2].coordinates[k]-m->atoms[a1].coordinates[k];
				dot +=d*(semiEmpiricalMD->coordinatesOld[a2][k]-semiEmpiricalMD->coordinatesOld[a1][k]);
			}
			invMass1 = 1/m->atoms[a1].prop.masse;
			invMass2 = 1/m->atoms[a2].prop.masse;
		        term = omega*delta / (2.0*(invMass1+invMass2)*dot);
			for (k=0;k<3;k++)
			{
				terms[k] = (semiEmpiricalMD->coordinatesOld[a2][k]-semiEmpiricalMD->coordinatesOld[a1][k])*term;
			}
			for (k=0;k<3;k++) m->atoms[a1].coordinates[k] -= terms[k]*invMass1;
			for (k=0;k<3;k++) m->atoms[a2].coordinates[k] += terms[k]*invMass2;

			invMass1 /= semiEmpiricalMD->dt;
			invMass2 /= semiEmpiricalMD->dt;
			for (k=0;k<3;k++) semiEmpiricalMD->velocity[a1][k] -= terms[k]*invMass1;
			for (k=0;k<3;k++) semiEmpiricalMD->velocity[a2][k] += terms[k]*invMass2;
		}
		for (i = 0; i < semiEmpiricalMD->numberOfAtoms; i++)
		{
			semiEmpiricalMD->moved[i] = semiEmpiricalMD->update[i];
			semiEmpiricalMD->update[i] = FALSE;
		}
	}while(!done && nIter<maxIter);
	if(nIter>=maxIter && deltaMax>tolerance*10)
	{
		printf(_("Rattle first portion : Warning, distance constraints not satisfied\n"));
	}
	for (i = 0; i < semiEmpiricalMD->numberOfAtoms; i++)
		if(!m->atoms[i].variable) 
		{
			for (k=0;k<3;k++) m->atoms[i].coordinates[k] =  semiEmpiricalMD->coordinatesOld[i][k];
			for (k=0;k<3;k++) semiEmpiricalMD->velocity[i][k] = 0;
		}

}
/*********************************************************************************/
static void applyRattleSecondPortion(SemiEmpiricalMD* semiEmpiricalMD)
{
	gint i;
	gint k;
	gint maxIter = 100;
	gdouble omega = 1.2;
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
	MoleculeSE* m = &semiEmpiricalMD->seModel->molecule;
	SemiEmpiricalModel* seModel = semiEmpiricalMD->seModel;
	gdouble deltaMax = 0;

	if(seModel->constraints==NOCONSTRAINTS) return;
	tolerance /= semiEmpiricalMD->dt;
	for (i = 0; i < semiEmpiricalMD->numberOfAtoms; i++)
	{
			semiEmpiricalMD->moved[i] = semiEmpiricalMD->seModel->molecule.atoms[i].variable;
			semiEmpiricalMD->update[i] = FALSE;
	}
	maxIter *= semiEmpiricalMD->seModel->numberOfRattleConstraintsTerms;
	do{
		nIter++;
		done=TRUE;
		deltaMax = 0;
		for (i = 0; i < semiEmpiricalMD->seModel->numberOfRattleConstraintsTerms; i++)
		{
			a1 = (gint)semiEmpiricalMD->seModel->rattleConstraintsTerms[0][i];
			a2 = (gint)semiEmpiricalMD->seModel->rattleConstraintsTerms[1][i];
			r2ij = semiEmpiricalMD->seModel->rattleConstraintsTerms[2][i];
			if( !semiEmpiricalMD->moved[a1] && !semiEmpiricalMD->moved[a2] ) continue;
			/* here : rattle image for PBC, not yet implemented */
			dot = 0;
			for (k=0;k<3;k++)
			{
				d = m->atoms[a2].coordinates[k]-m->atoms[a1].coordinates[k];
				dot +=d*(semiEmpiricalMD->velocity[a2][k]-semiEmpiricalMD->velocity[a1][k]);
			}
			invMass1 = 1/semiEmpiricalMD->seModel->molecule.atoms[a1].prop.masse;
			invMass2 = 1/semiEmpiricalMD->seModel->molecule.atoms[a2].prop.masse;
		        term = -dot / ((invMass1+invMass2)*r2ij);
			if(deltaMax<fabs(term)) deltaMax = fabs(term);
			if(fabs(term)<=tolerance) continue;
			done = FALSE;
			semiEmpiricalMD->update[a1] = TRUE;
			semiEmpiricalMD->update[a2] = TRUE;
		        term *= omega;

			for (k=0;k<3;k++)
			{
				d = m->atoms[a2].coordinates[k]-m->atoms[a1].coordinates[k];
				terms[k] = d*term;
			}
			for (k=0;k<3;k++) semiEmpiricalMD->velocity[a1][k] -= terms[k]*invMass1;
			for (k=0;k<3;k++) semiEmpiricalMD->velocity[a2][k] += terms[k]*invMass2;
		}
		for (i = 0; i < semiEmpiricalMD->numberOfAtoms; i++)
		{
			semiEmpiricalMD->moved[i] = semiEmpiricalMD->update[i];
			semiEmpiricalMD->update[i] = FALSE;
		}
	}while(!done && nIter<maxIter);
	if(nIter>=maxIter && deltaMax>tolerance*10)
	{
		printf(_("Rattle second portion : Warning, velocity constraints not satisfied\n"));
	}
	for (i = 0; i < semiEmpiricalMD->numberOfAtoms; i++)
			if(!m->atoms[i].variable) for (k=0;k<3;k++) semiEmpiricalMD->velocity[i][k] = 0.0;
}
/*********************************************************************************/
static void applyVerlet(SemiEmpiricalMD* seMD)
{
	gint i;
	gint j;

	if(seMD->seModel->constraints!=NOCONSTRAINTS)
	for (i = 0; i < seMD->numberOfAtoms; i++)
		for ( j = 0; j < 3; j++)
				seMD->coordinatesOld[i][j]= seMD->seModel->molecule.atoms[i].coordinates[j];

	for (i = 0; i < seMD->numberOfAtoms; i++)
	{
		if(!seMD->seModel->molecule.atoms[i].variable) continue;
		for ( j = 0; j < 3; j++)
		{
			seMD->seModel->molecule.atoms[i].coordinates[j] += 
				seMD->velocity[i][j] * seMD->dt +
				seMD->a[i][j]*seMD->dt2_2;	
		}
		for ( j = 0; j < 3; j++)
			seMD->velocity[i][j] += seMD->a[i][j] * seMD->dt_2;
	}

	if(seMD->seModel->constraints!=NOCONSTRAINTS) applyRattleFirstPortion(seMD);

	newAccelaration(seMD);

	for (i = 0; i < seMD->numberOfAtoms; i++)
		if(seMD->seModel->molecule.atoms[i].variable)
		for ( j = 0; j < 3; j++)
			seMD->velocity[i][j] += seMD->a[i][j] * seMD->dt_2;
	if(seMD->seModel->constraints!=NOCONSTRAINTS) applyRattleSecondPortion(seMD);
}
/*********************************************************************************/
static void applyBeeman(SemiEmpiricalMD* seMD)
{
	gint i;
	gint j;
	gdouble terms[3];

	if(seMD->seModel->constraints!=NOCONSTRAINTS)
	for (i = 0; i < seMD->numberOfAtoms; i++)
		for ( j = 0; j < 3; j++)
				seMD->coordinatesOld[i][j]= seMD->seModel->molecule.atoms[i].coordinates[j];

	for (i = 0; i < seMD->numberOfAtoms; i++)
	{
		if(!seMD->seModel->molecule.atoms[i].variable) continue;

		for ( j = 0; j < 3; j++)
			terms[j] = 5.0*seMD->a[i][j]-seMD->aold[i][j];

		for ( j = 0; j < 3; j++)
		{
			seMD->seModel->molecule.atoms[i].coordinates[j] += 
				seMD->velocity[i][j] * seMD->dt +
				terms[j]*seMD->dt2_8;	
		}
		for ( j = 0; j < 3; j++)
			seMD->velocity[i][j] += terms[j] * seMD->dt_8;
	}

	if(seMD->seModel->constraints!=NOCONSTRAINTS) applyRattleFirstPortion(seMD);

	newAccelaration(seMD);

	for (i = 0; i < seMD->numberOfAtoms; i++)
		if(seMD->seModel->molecule.atoms[i].variable)
		for ( j = 0; j < 3; j++)
			seMD->velocity[i][j] += (3.0*seMD->a[i][j]+seMD->aold[i][j]) * seMD->dt_8;
	if(seMD->seModel->constraints!=NOCONSTRAINTS) applyRattleSecondPortion(seMD);
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
static void newProperties(SemiEmpiricalMD* seMD, gchar* comments)
{
	if( seMD->fileProp == NULL) return;
	fprintf(seMD->fileProp,"time0(fs)\ttime(fs)\tTotal Energy(Kcal/mol)\tPotential Energy(kcal/mol) Kinetic Energy(Kcal/mol)\tT(t) (K)\tTaver(K)\tsigma(T)(K)");
	if(comments) fprintf(seMD->fileProp,"%s\n", comments);
	else fprintf(seMD->fileProp,"\n");
}
/*********************************************************************************/
static void saveProperties(SemiEmpiricalMD* seMD, gint iStep0, gint iStep, gchar* comments)
{
	/* gdouble fsInAKMA = 1.0/sqrt(1e-10*1e-10*1.6605655e-27*6.022045e23/4184.0)/1e15;*/
	static gdouble fsInAKMA = 0.020454828110640;
	gdouble dt = seMD->dt/(fsInAKMA);
	static gdouble Ttot = 0;
	static gdouble T2tot = 0;
	gdouble Taver = 0;
	gdouble T2aver = 0;
	

	if( seMD->fileProp == NULL) return;
	if(iStep==1)
	{
			Ttot = 0;
			T2tot = 0;
	}
	Ttot += seMD->kelvin;
	T2tot += seMD->kelvin*seMD->kelvin;
	Taver = Ttot/iStep;
	T2aver = T2tot/iStep;


	fprintf(seMD->fileProp,"%f\t%f\t%f\t\t%f\t\t%f\t%f\t%f\t%f", 
			(iStep0)*dt, 
			(iStep)*dt, 
			seMD->totalEnergy,
			seMD->potentialEnergy,
			seMD->kineticEnergy,
			seMD->kelvin,
			Taver,
			sqrt(fabs(T2aver-Taver*Taver))
			 );
	if(comments) fprintf(seMD->fileProp,"%s\n", comments);
	else fprintf(seMD->fileProp,"\n");
}
/*********************************************************************************/
static void saveTrajectory(SemiEmpiricalMD* seMD, gint iStep)
{
	/* gdouble fsInAKMA = 1.0/sqrt(1e-10*1e-10*1.6605655e-27*6.022045e23/4184.0)/1e15;*/
	static gdouble fsInAKMA = 0.020454828110640;
	gdouble dt = seMD->dt/(fsInAKMA);
	gint i;
	if( seMD->fileTraj == NULL) return;

	fprintf(seMD->fileTraj," %d %f %f %f %f nAtoms time(fs) TotalEnery(Kcal/mol) Kinetic Potential\n", 
			seMD->numberOfAtoms,
			 (iStep)*dt, 
			seMD->totalEnergy,
			seMD->kineticEnergy,
			seMD->potentialEnergy
			 );
	fprintf(seMD->fileTraj," %s\n", "Coord in Ang, Velocity in AKMA, time in fs");

	for (i = 0; i < seMD->numberOfAtoms; i++)
	{
		fprintf(seMD->fileTraj," %s %f %f %f %f %f %f %f %s %s %s %d %d\n", 
				seMD->seModel->molecule.atoms[i].prop.symbol,
				seMD->seModel->molecule.atoms[i].coordinates[0],
				seMD->seModel->molecule.atoms[i].coordinates[1],
				seMD->seModel->molecule.atoms[i].coordinates[2],
				seMD->velocity[i][0],
				seMD->velocity[i][1],
				seMD->velocity[i][2],
				seMD->seModel->molecule.atoms[i].charge,
				seMD->seModel->molecule.atoms[i].mmType,
				seMD->seModel->molecule.atoms[i].pdbType,
				seMD->seModel->molecule.atoms[i].residueName,
				seMD->seModel->molecule.atoms[i].residueNumber,
				seMD->seModel->molecule.atoms[i].variable
				);
	}
}

/**********************************************************************/
void	freeSemiEmpiricalMD(SemiEmpiricalMD* seMD)
{

	seMD->seModel = NULL;
	seMD->numberOfAtoms = 0;
	seMD->updateFrequency = 0;
	if(seMD->velocity)
	{
		gint i;
		for(i=0;i<seMD->numberOfAtoms;i++)
			if(seMD->velocity[i]) g_free(seMD->velocity[i]);
		g_free(seMD->velocity);
	}
	if(seMD->a)
	{
		gint i;
		for(i=0;i<seMD->numberOfAtoms;i++)
			if(seMD->a[i]) g_free(seMD->a[i]);
		g_free(seMD->a);
	}
	if(seMD->aold)
	{
		gint i;
		for(i=0;i<seMD->numberOfAtoms;i++)
			if(seMD->aold[i]) g_free(seMD->aold[i]);
		g_free(seMD->aold);
	}
}
/********************************************************************************/
static gdouble getEKin(SemiEmpiricalMD* seMD)
{
	gdouble ekin = 0;
	gint i;
	gint j;
	gdouble masse;
	for ( i = 0; i < seMD->numberOfAtoms; i++)
	{
		masse = seMD->seModel->molecule.atoms[i].prop.masse;
		for ( j = 0; j < 3; j++)
			ekin += seMD->velocity[i][j]*seMD->velocity[i][j]*
				masse;
	}
	return ekin/2;
}
/********************************************************************************/
static gdouble getKelvin(SemiEmpiricalMD* seMD)
{
	gint nfree = seMD->nfree;
	static gdouble Kb = 1.9871914e-3;
	if(nfree<1) return 0;
	return 2*getEKin(seMD) / ( nfree * Kb);
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
static void getsFrictionalAndRandomForce(SemiEmpiricalMD* seMD)
{
	gdouble* gamma = seMD->gamma;
	gdouble* positionFriction = seMD->positionFriction;
	gdouble* velocityFriction = seMD->velocityFriction;
	gdouble* accelarationFriction = seMD->accelarationFriction;
	gdouble** positionRandom = seMD->positionRandom;
	gdouble** velocityRandom = seMD->velocityRandom;
	gdouble dt = seMD->dt;
	
	gint n = seMD->numberOfAtoms;

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
        	gamma[i] = seMD->friction;

	/* printf(" friction = %f\n", seMD->friction);*/
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
               		ktm = Kb * seMD->temperature / seMD->seModel->molecule.atoms[i].prop.masse;
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
static void applyStochastic(SemiEmpiricalMD* seMD)
{
	gdouble* positionFriction = seMD->positionFriction;
	gdouble* velocityFriction = seMD->velocityFriction;
	gdouble* accelarationFriction = seMD->accelarationFriction;
	gdouble** positionRandom = seMD->positionRandom;
	gdouble** velocityRandom = seMD->velocityRandom;
	gdouble**v = seMD->velocity;
	gdouble**a = seMD->a;
	
	gint n = seMD->numberOfAtoms;
	gint i;
	gint j;
	AtomSE* atoms = seMD->seModel->molecule.atoms;

	getsFrictionalAndRandomForce(seMD);

	if(seMD->seModel->constraints!=NOCONSTRAINTS)
	for (i = 0; i < seMD->numberOfAtoms; i++)
		for ( j = 0; j < 3; j++)
				seMD->coordinatesOld[i][j]= seMD->seModel->molecule.atoms[i].coordinates[j];

	for(i=0;i<n;i++)
	{
		if(!seMD->seModel->molecule.atoms[i].variable) continue;
		for(j=0;j<3;j++)
			atoms[i].coordinates[j] += v[i][j]*velocityFriction[i] + a[i][j]*accelarationFriction[i] + positionRandom[i][j];
		for(j=0;j<3;j++)
			v[i][j] = v[i][j]*positionFriction[i] + 0.5*a[i][j]*velocityFriction[i];
	}

	if(seMD->seModel->constraints!=NOCONSTRAINTS) applyRattleFirstPortion(seMD);

	newAccelaration(seMD);

	for (i = 0; i < n; i++)
		if(seMD->seModel->molecule.atoms[i].variable)
		for ( j = 0; j < 3; j++)
			v[i][j] += 0.5*a[i][j]*velocityFriction[i] + velocityRandom[i][j];
	if(seMD->seModel->constraints!=NOCONSTRAINTS) applyRattleSecondPortion(seMD);
	computeEnergies(seMD);
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
