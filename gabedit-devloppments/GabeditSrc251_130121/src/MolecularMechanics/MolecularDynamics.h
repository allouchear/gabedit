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

#ifndef __GABEDIT_MOLECULARDYNAMICS_H__
#define __GABEDIT_MOLECULARDYNAMICS_H__

typedef enum
{
  VERLET = 0,
  BEEMAN = 1,
  STOCHASTIC = 2
} MDIntegratorType;
typedef enum
{
  NONE = 0,
  ANDERSEN = 1,
  BERENDSEN = 2,
  BUSSI = 3
} MDThermostatType;

typedef struct _MolecularDynamics  MolecularDynamics;

struct _MolecularDynamics
{
	ForceField* forceField;
	gint numberOfAtoms;
	gint updateFrequency;
	gdouble** coordinatesOld; 
	gdouble** velocity; 
	gdouble** a; 
	gdouble** aold; 
	gboolean* moved;
	gboolean* update;
	gdouble dt;
	gdouble dt_2;
	gdouble dt2_2;
	gdouble dt_8;
	gdouble dt2_8;
	MDIntegratorType integratorType;
	gdouble potentialEnergy;
	gdouble kineticEnergy;
	gdouble totalEnergy;
	gdouble kelvin;


	gdouble* positionFriction;
	gdouble* velocityFriction;
	gdouble* accelarationFriction;
	gdouble** positionRandom;
	gdouble** velocityRandom;
	gdouble* gamma;
	gdouble friction;
	gdouble temperature;
	gdouble collide;
	MDThermostatType thermostat;

	FILE* fileTraj;
	FILE* fileProp;
	gint nfree;
	gint nvariables;
};
void	freeMolecularDynamics(MolecularDynamics* molecularDynamics);
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
		);
ForceField**	runMolecularDynamicsConfo(
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
		);
#endif /* __GABEDIT_MOLECULARDYNAMICS_H__ */

