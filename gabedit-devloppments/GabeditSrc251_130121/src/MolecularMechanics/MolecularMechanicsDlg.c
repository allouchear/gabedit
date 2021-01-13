/* MolecularMechanicsDlg.c */
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


#include <stdlib.h>
#include <math.h>
#include <glib.h>
#include "../../Config.h"
#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Constants.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/Measure.h"
#include "../Geometry/ResultsAnalise.h"
#include "../Geometry/GeomGlobal.h"
#include "../Geometry/GeomXYZ.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Files/FolderChooser.h"
#include "../Files/GabeditFolderChooser.h"
#include "../MolecularMechanics/Atom.h"
#include "../MolecularMechanics/Molecule.h"
#include "../MolecularMechanics/ForceField.h"
#include "../MolecularMechanics/MolecularMechanics.h"
#include "../MolecularMechanics/ConjugateGradient.h"
#include "../MolecularMechanics/SteepestDescent.h"
#include "../MolecularMechanics/QuasiNewton.h"
#include "../MolecularMechanics/MolecularDynamics.h"

typedef enum
{
	MMBOND = 0,
	MMBEND = 1,
	MMTORSION = 2,
	MMIMPROPER = 3,
	MMNONBOND = 4,
	MMHBOND  =5 ,
	MMCOULOMB = 6,
	PWCOULOMB = 7,
	PWVANDERWALS = 8
} MMOptions;

typedef enum
{
	GRADQUASINEWTON  = 0,
	GRADSTEEPEST  = 1,
	GRADCONJUGATE = 2,
	GRADHESTENES  = 3,
	GRADFLETCHER  = 4,
	GRADPOLAK     = 5,
	GRADWOLF      = 6
} GradientOptions;

typedef enum
{
	GRADMAXITERATIONS  = 0,
	GRADEPSILON        = 1,
	GRADMAXLINES       = 2,
	GRADINITIALSTEP    = 3,
	GRADFREQUENCY      = 4
} GradientEntries;

typedef enum
{
	TOLE = 0,
	TOLD = 1
} TOLptions;

#define NGRADENTRYS 5
#define NGRADOPTIONS 7
#define NOPTIONS1 4
#define NOPTIONS2 3
#define NOPTIONS3 2
#define NINTEGOPTIONS 3
#define NTHERMOPTIONS 4
#define NENTRYTOL 2
#define NCONSTRAINTS 3

static	GtkWidget* buttonTypesOptions[3];
static	GtkWidget* buttonMMOptions[NOPTIONS1+NOPTIONS2+NOPTIONS3];
static	GtkWidget* buttonMinimizeOptions[NGRADOPTIONS];
static	GtkWidget* buttonMDOptions[NINTEGOPTIONS];
static	GtkWidget* buttonConstraintsOptions[NCONSTRAINTS];
static	GtkWidget* buttonMDThermOptions[NTHERMOPTIONS];
static	GtkWidget* entryMinimizeOptions[NGRADENTRYS];
static	GtkWidget* frameAmber = NULL;
static	GtkWidget* framePairWise = NULL;
static 	GtkWidget* entryMDTimes[4];
static 	GtkWidget* entryMDTemperature[4];
static 	GtkWidget* entryMDStepSize;
static 	GtkWidget* entryMDRafresh;

static 	GtkWidget* entrySDFriction;
static 	GtkWidget* entrySDCollide;

static  GtkWidget* entryNumberOfGeom = NULL;
static  GtkWidget* entryFileNameGeom = NULL;
static  GtkWidget* entryFileNameTraj = NULL;
static  GtkWidget* entryFileNameProp = NULL;
static  GtkWidget* buttonSaveTraj = NULL;
static  GtkWidget* buttonSaveProp = NULL;
static  GtkWidget* buttonDirSelector = NULL;
static  GtkWidget* entryCharge = NULL;
static  GtkWidget* entrySpinMultiplicity = NULL;
static  GtkWidget* buttonCreateGaussian = NULL ;
static  GtkWidget* entryGaussianKeywords = NULL; 
static  GtkWidget* buttonCreateMopac = NULL ;
static  GtkWidget* entryMopacKeywords = NULL; 
static  GtkWidget* buttonCreateFireFly = NULL ;
static  GtkWidget* entryFireFlyKeywords = NULL; 
static  GtkWidget* buttonPostNone = NULL ;
static  GtkWidget* buttonPostMMOpt = NULL ;
static  GtkWidget* buttonPostPM6Mopac = NULL ;
static  GtkWidget* buttonPostAM1Mopac = NULL ;
static  GtkWidget* buttonPostFireFly = NULL ;

static	GtkWidget* entryTolerance[NENTRYTOL];
static	GtkWidget* buttonTolerance[NENTRYTOL];
static gint totalCharge = 0;
static gint spinMultiplicity=1;

/*********************************************************************************/
static void getMultiplicityName(gint multiplicity, gchar* buffer)
{
	if(multiplicity==1) sprintf(buffer,"Singlet");
	else if(multiplicity==2) sprintf(buffer,"Doublet");
	else if(multiplicity==3) sprintf(buffer,"Triplet");
	else if(multiplicity==4) sprintf(buffer,"Quartet");
	else if(multiplicity==5) sprintf(buffer,"Quintet");
	else if(multiplicity==6) sprintf(buffer,"Sextet");
	else sprintf(buffer,"UNKNOWN");
}
/*****************************************************************************/
static gboolean createMopacFiles(gint numberOfGeometries, ForceField** geometries, gdouble* energies, gchar* fileNamePrefix, gchar* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gint i;
	gint j;
	gint nG = 0;
	gchar* fileName = NULL;
	gchar* fileNameSH = NULL;
	gchar multiplicityStr[100];
#ifdef G_OS_WIN32
	gchar c='%';
#endif

	if(numberOfGeometries<1) return FALSE;
	if(!geometries) return FALSE;
	if(!energies) return FALSE;
	for(i=0;i<numberOfGeometries;i++) if(geometries[i]) nG++;
	if(nG<1) return FALSE;
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%sMopac.sh",fileNamePrefix);
#else
	fileNameSH = g_strdup_printf("%sMopac.bat",fileNamePrefix);
#endif
 	fileSH = fopen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"set PATH=%cPATH%c;\"%s\"\n",c,c,mopacDirectory);
#endif

	getMultiplicityName(spinMultiplicity, multiplicityStr);

	for(i=0;i<numberOfGeometries;i++)
	{
		if(!geometries[i]) continue;
 		if(fileName) g_free(fileName);
		fileName = g_strdup_printf("%s_%d.mop",fileNamePrefix,i+1);
 		file = fopen(fileName, "w");
		if(!file) return FALSE;
		fprintf(file,"* ===============================\n");
		fprintf(file,"* Input file for Mopac\n");
		fprintf(file,"* MM/SE Energy(kCal/mol) =%f\n",energies[i]);
		fprintf(file,"* ===============================\n");
		fprintf(file,"%s CHARGE=%d %s\n",keyWords,totalCharge,multiplicityStr);
		fprintf(file,"\n");
		fprintf(file,"Mopac file generated by Gabedit\n");

		for(j=0;j<geometries[i]->molecule.nAtoms;j++)
		{
		fprintf(file," %s %f %d %f %d %f %d\n", 
				geometries[i]->molecule.atoms[j].prop.symbol,
				geometries[i]->molecule.atoms[j].coordinates[0],
				1,
				geometries[i]->molecule.atoms[j].coordinates[1],
				1,
				geometries[i]->molecule.atoms[j].coordinates[2],
				1
				);
		}
		fclose(file);
		fprintf(fileSH,"%s %s\n",NameCommandMopac,fileName);
	}
	fclose(fileSH);
#ifndef G_OS_WIN32
	{
		gchar buffer[1024];
  		sprintf(buffer,"chmod u+x %s",fileNameSH);
		{int ierr = system(buffer);}
	}
#endif
 	if(fileName) g_free(fileName);
 	if(fileNameSH) g_free(fileNameSH);
	return TRUE;

}
/*****************************************************************************/
static gboolean createGaussianFiles(gint numberOfGeometries, ForceField** geometries, gdouble* energies, gchar* fileNamePrefix, gchar* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gint i;
	gint j;
	gint nG = 0;
	gchar* fileName = NULL;
	gchar* fileNameSH = NULL;

	if(numberOfGeometries<1) return FALSE;
	if(!geometries) return FALSE;
	if(!energies) return FALSE;
	for(i=0;i<numberOfGeometries;i++) if(geometries[i]) nG++;
	if(nG<1) return FALSE;
	fileNameSH = g_strdup_printf("%sGauss.sh",fileNamePrefix);
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%sGauss.sh",fileNamePrefix);
#else
	fileNameSH = g_strdup_printf("%sGauss.bat",fileNamePrefix);
#endif
 	fileSH = fopen(fileNameSH, "w");
	if(!fileSH) return FALSE;


	for(i=0;i<numberOfGeometries;i++)
	{
		if(!geometries[i]) continue;
 		if(fileName) g_free(fileName);
		fileName = g_strdup_printf("%s_%d.com",fileNamePrefix,i+1);
 		file = fopen(fileName, "w");
		if(!file) return FALSE;
		fprintf(file,"#P %s\n",keyWords);
		fprintf(file,"#  Units(Ang,Deg)\n");
		fprintf(file,"\n");
		fprintf(file,"File generated by Gabedit\n");
		fprintf(file,"MM/SE Energy(kCal/mol) = %f\n",energies[i]);
		fprintf(file,"\n");
		fprintf(file,"%d %d\n",totalCharge,spinMultiplicity);
		for(j=0;j<geometries[i]->molecule.nAtoms;j++)
		{
		fprintf(file,"%s %f %f %f\n", 
				geometries[i]->molecule.atoms[j].prop.symbol,
				geometries[i]->molecule.atoms[j].coordinates[0],
				geometries[i]->molecule.atoms[j].coordinates[1],
				geometries[i]->molecule.atoms[j].coordinates[2]
				);
		}
		fprintf(file,"\n");
		fclose(file);
		fprintf(fileSH,"%s %s\n",NameCommandGaussian,fileName);
	}
	fclose(fileSH);
#ifndef G_OS_WIN32
	{
		gchar buffer[1024];
  		sprintf(buffer,"chmod u+x %s",fileNameSH);
		{int ierr = system(buffer);}
	}
#endif
 	if(fileName) g_free(fileName);
 	if(fileNameSH) g_free(fileNameSH);
	return TRUE;

}
/*****************************************************************************/
static gboolean createFireFlyFiles(gint numberOfGeometries, ForceField** geometries, gdouble* energies, gchar* fileNamePrefix, gchar* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gint i;
	gint j;
	gint nG = 0;
	gchar* fileName = NULL;
	gchar* fileNameSH = NULL;
	gchar buffer[1024];
#ifdef G_OS_WIN32
	gchar c='%';
#endif

	if(numberOfGeometries<1) return FALSE;
	if(!geometries) return FALSE;
	if(!energies) return FALSE;
	for(i=0;i<numberOfGeometries;i++) if(geometries[i]) nG++;
	if(nG<1) return FALSE;
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%sPCGam.sh",fileNamePrefix);
#else
	fileNameSH = g_strdup_printf("%sPCGam.bat",fileNamePrefix);
#endif
 	fileSH = fopen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"set PATH=%cPATH%c;\"%s\"\n",c,c,fireflyDirectory);
#endif


	uppercase(keyWords);
	for(i=0;i<numberOfGeometries;i++)
	{
		if(!geometries[i]) continue;
 		if(fileName) g_free(fileName);
		fileName = g_strdup_printf("%sP_%d.inp",fileNamePrefix,i+1);
 		file = fopen(fileName, "w");
		if(!file) return FALSE;
		fprintf(file,"! ======================================================\n");
		fprintf(file,"!  Input file for FireFly\n"); 
		fprintf(file,"! ======================================================\n");
		if(strstr(keyWords,"RUNTYP"))
		{
			sscanf(strstr(keyWords,"RUNTYP"),"%s",buffer);
			fprintf(file," $CONTRL %s $END\n",buffer);
		}
		if(strstr(keyWords,"SCFTYP"))
		{
			sscanf(strstr(keyWords,"SCFTYP"),"%s",buffer);
			fprintf(file," $CONTRL %s $END\n",buffer);
		}
		else
		{
			if(spinMultiplicity==1)
				fprintf(file," $CONTRL SCFTYP=RHF $END\n");
			else
				fprintf(file," $CONTRL SCFTYP=UHF $END\n");
		}

		fprintf(file," $CONTRL ICHARG=%d MULT=%d $END\n",totalCharge,spinMultiplicity);
		if(strstr(keyWords,"GBASIS"))
		{
			sscanf(strstr(keyWords,"GBASIS"),"%s",buffer);
			fprintf(file," $BASIS %s $END\n",buffer);
		}
		fprintf(file," $DATA\n");
		fprintf(file,"Molecule specification\n");
		fprintf(file,"C1\n");
		for(j=0;j<geometries[i]->molecule.nAtoms;j++)
		{
			gchar* symbol = geometries[i]->molecule.atoms[j].prop.symbol;
			SAtomsProp prop = prop_atom_get(symbol);
			fprintf(file,"%s %f %f %f %f\n", 
				symbol,
				(gdouble)prop.atomicNumber,
				geometries[i]->molecule.atoms[j].coordinates[0],
				geometries[i]->molecule.atoms[j].coordinates[1],
				geometries[i]->molecule.atoms[j].coordinates[2]
				);
		}
		fprintf(file," $END\n");
		fclose(file);

#ifndef G_OS_WIN32
		if(!strcmp(NameCommandFireFly,"pcgamess") || !strcmp(NameCommandFireFly,"nohup pcgamess")||
		!strcmp(NameCommandFireFly,"firefly") || !strcmp(NameCommandFireFly,"nohup firefly"))
		{
			fprintf(fileSH,"mkdir %stmp%d\n",fileNamePrefix,i+1);
			fprintf(fileSH,"cd %stmp%d\n",fileNamePrefix,i+1);
			fprintf(fileSH,"cp %s input\n",fileName);
			fprintf(fileSH,"%s -p -o %sP_%d.log\n",NameCommandFireFly,fileNamePrefix,i+1);
			fprintf(fileSH,"cd ..\n");
			fprintf(fileSH,"mv PUNCH  %sP_%d.pun\n",fileNamePrefix,i+1);
			fprintf(fileSH,"/bin/rm -r  %stmp%d\n",fileNamePrefix,i+1);
		}
		else
			fprintf(fileSH,"%s %s",NameCommandFireFly,fileName);
#else
	 	if(!strcmp(NameCommandFireFly,"pcgamess") ||
	 	!strcmp(NameCommandFireFly,"firefly") )
		{
         		fprintf(fileSH,"mkdir %stmp%d\n",fileNamePrefix,i+1);
			addUnitDisk(fileSH, fileNamePrefix);
	 		fprintf(fileSH,"cd %stmp%d\n",fileNamePrefix,i+1);
         		fprintf(fileSH,"copy %s input\n",fileName);
         		fprintf(fileSH,"%s -p -o %sP_%d.log\n",NameCommandFireFly,fileNamePrefix,i+1);
	 		fprintf(fileSH,"cd ..\n");
         		fprintf(fileSH,"move PUNCH  %sP_%d.pun\n",fileNamePrefix,i+1);
         		fprintf(fileSH,"del /Q  %stmp%d\n",fileNamePrefix,i+1);
         		fprintf(fileSH,"rmdir  %stmp%d\n",fileNamePrefix,i+1);
		}
		else
			fprintf(fileSH,"%s %s",NameCommandFireFly,fileName);
#endif
	}
	fclose(fileSH);
#ifndef G_OS_WIN32
	{
		gchar buffer[1024];
  		sprintf(buffer,"chmod u+x %s",fileNameSH);
		{int ierr = system(buffer);}
	}
#endif
 	if(fileName) g_free(fileName);
 	if(fileNameSH) g_free(fileNameSH);
	return TRUE;

}
/*****************************************************************************/
static gboolean saveConfoGeometries(gint numberOfGeometries, ForceField** geometries, gdouble* energies, gchar* fileNameGeom)
{
	FILE* file = NULL;
	gint i;
	gint j;
	gint nG = 0;
	gint k;

	if(numberOfGeometries<1) return FALSE;
	if(!geometries) return FALSE;
	if(!energies) return FALSE;
	for(i=0;i<numberOfGeometries;i++) if(geometries[i]) nG++;
	if(nG<1) return FALSE;

 	file = fopen(fileNameGeom, "w");

	if(!file) return FALSE;

	fprintf(file,"[Gabedit Format]\n");
	fprintf(file,"[GEOCONV]\n");
	fprintf(file,"energy\n");
	for(i=0;i<numberOfGeometries;i++)
		if(geometries[i]) fprintf(file,"%f\n",energies[i]);
	fprintf(file,"max-force\n");
	for(i=0;i<numberOfGeometries;i++)
		if(geometries[i]) fprintf(file,"0.0\n");
	fprintf(file,"rms-force\n");
	for(i=0;i<numberOfGeometries;i++)
		if(geometries[i]) fprintf(file,"0.0\n");

	fprintf(file,"\n");
	fprintf(file,"[GEOMETRIES]\n");
	for(i=0;i<numberOfGeometries;i++)
	{
		if(!geometries[i]) continue;
		fprintf(file,"%d\n",geometries[i]->molecule.nAtoms);
		fprintf(file,"\n");
		for(j=0;j<geometries[i]->molecule.nAtoms;j++)
		fprintf(file," %s %f %f %f\n", 
				geometries[i]->molecule.atoms[j].prop.symbol,
				geometries[i]->molecule.atoms[j].coordinates[0],
				geometries[i]->molecule.atoms[j].coordinates[1],
				geometries[i]->molecule.atoms[j].coordinates[2]
				);
	}
	fprintf(file,"\n");
	fprintf(file,"[GEOMS]  1 \n");/* 1 for format # 1 */
	fprintf(file,"%d 2\n",nG);
	fprintf(file,"energy kcal/mol 1\n");
	fprintf(file,"deltaE K 1\n");
	k = -1;
	for(i=0;i<numberOfGeometries;i++)
	{
		if(!geometries[i]) continue;
		if(k<0) k = i;
		fprintf(file,"%f\n",energies[i]);
		if(k>=0) fprintf(file,"%f\n",(energies[i]-energies[k])*503.21892494);
		else fprintf(file,"0\n");
		fprintf(file,"%d %d %d\n",geometries[i]->molecule.nAtoms,totalCharge,spinMultiplicity);
		for(j=0;j<geometries[i]->molecule.nAtoms;j++)
		{
                        int nc = 0;
                        int k;
                        for(k=0;k<geometries[i]->molecule.nAtoms;k++)
                                if(geometries[i]->molecule.atoms[j].typeConnections&&geometries[i]->molecule.atoms[j].typeConnections[k]>0) nc++;

                        fprintf(file," %s %s %s %s %d %f %d %d %f %f %f %d ",
                                geometries[i]->molecule.atoms[j].prop.symbol,
                                geometries[i]->molecule.atoms[j].mmType,
                                geometries[i]->molecule.atoms[j].pdbType,
                                geometries[i]->molecule.atoms[j].residueName,
                                geometries[i]->molecule.atoms[j].residueNumber,
                                geometries[i]->molecule.atoms[j].charge,
                                geometries[i]->molecule.atoms[j].layer,
                                geometries[i]->molecule.atoms[j].variable,
                                geometries[i]->molecule.atoms[j].coordinates[0],
                                geometries[i]->molecule.atoms[j].coordinates[1],
                                geometries[i]->molecule.atoms[j].coordinates[2],
                                nc
                                );
                        for(k=0;k< geometries[i]->molecule.nAtoms;k++)
                        {
                                int nk =  geometries[i]->molecule.atoms[k].N-1;
                                if(geometries[i]->molecule.atoms[j].typeConnections && geometries[i]->molecule.atoms[j].typeConnections[nk]>0)
                                        fprintf(file," %d %d", nk+1, geometries[i]->molecule.atoms[j].typeConnections[nk]);
                        }
                        fprintf(file,"\n");
		}
	}
	fclose(file);
	return TRUE;

}
/*****************************************************************************/
static gboolean getEnergyMopac(gchar* fileNameOut, gdouble* energy)
{
	FILE* file = NULL;
	gchar buffer[1024];
	gchar* pdest = NULL;

 	file = fopen(fileNameOut, "rb");
	if(!file) return FALSE;
	 while(!feof(file))
	 {
		if(!fgets(buffer,BSIZE,file))break;
		pdest = strstr( buffer, " FINAL HEAT OF FORMATION");
		if(pdest) 
		{
			pdest = strstr( buffer, "=");
			if(pdest)
			{
				if(sscanf(pdest+1,"%lf",energy)==1)
				{
					fclose(file);
					return TRUE;
				}
			}
		}
	 }
	fclose(file);
	return FALSE;
}
/*****************************************************************************/
static gboolean runOneMopac(ForceField* geometry, gdouble* energy, gchar* fileNamePrefix, gchar* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gint j;
	gchar* fileNameIn = NULL;
	gchar* fileNameOut = NULL;
	gchar* fileNameSH = NULL;
	gchar multiplicityStr[100];
	gchar buffer[1024];
#ifdef G_OS_WIN32
	gchar c='%';
#endif

	if(!geometry) return FALSE;
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%sMopacOne.sh",fileNamePrefix);
#else
	fileNameSH = g_strdup_printf("%sMopacOne.bat",fileNamePrefix);
#endif
 	fileSH = fopen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"set PATH=%cPATH%c;\"%s\"\n",c,c,mopacDirectory);
#endif

	getMultiplicityName(spinMultiplicity, multiplicityStr);

	fileNameIn = g_strdup_printf("%sOne.mop",fileNamePrefix);
 	file = fopen(fileNameIn, "w");
	if(!file) 
	{
 		if(fileNameIn) g_free(fileNameIn);
 		if(fileNameOut) g_free(fileNameOut);
 		if(fileNameSH) g_free(fileNameSH);
		return FALSE;
	}
	fprintf(file,"* ===============================\n");
	fprintf(file,"* Input file for Mopac\n");
	fprintf(file,"* ===============================\n");
	fprintf(file,"%s CHARGE=%d %s\n",keyWords,totalCharge,multiplicityStr);
	fprintf(file,"\n");
	fprintf(file,"Mopac file generated by Gabedit\n");

	for(j=0;j<geometry->molecule.nAtoms;j++)
	{
	fprintf(file," %s %f %d %f %d %f %d\n", 
			geometry->molecule.atoms[j].prop.symbol,
			geometry->molecule.atoms[j].coordinates[0],
			1,
			geometry->molecule.atoms[j].coordinates[1],
			1,
			geometry->molecule.atoms[j].coordinates[2],
			1
			);
	}
	fclose(file);
#ifndef G_OS_WIN32
	fprintf(fileSH,"%s %s\n",NameCommandMopac,fileNameIn);
	fclose(fileSH);
	sprintf(buffer,"chmod u+x %s",fileNameSH);
	{int ierr = system(buffer);}
	{int ierr = system(fileNameSH);}
#else
	fprintf(fileSH,"\"%s\" \"%s\"\n",NameCommandMopac,fileNameIn);
	fclose(fileSH);
	sprintf(buffer,"\"%s\"",fileNameSH);
	{int ierr = system(buffer);}
#endif

	fileNameOut = g_strdup_printf("%sOne.out",fileNamePrefix);
	if(getEnergyMopac(fileNameOut,energy))
	{
		gchar* str = NULL;

		read_geom_from_mopac_output_file(fileNameOut, -1);
		if(strstr(keyWords,"AM1")) str = g_strdup_printf("Energy by AM1/Mopac = %f", *energy);
		else str = g_strdup_printf("Energy by PM6/Mopac = %f", *energy);
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		Waiting(1);
		if(str) g_free(str);
	}
	else
	{
 		if(fileNameIn) g_free(fileNameIn);
 		if(fileNameOut) g_free(fileNameOut);
 		if(fileNameSH) g_free(fileNameSH);
		return FALSE;
	}

 	if(fileNameIn) g_free(fileNameIn);
 	if(fileNameOut) g_free(fileNameOut);
 	if(fileNameSH) g_free(fileNameSH);
	return TRUE;
}
/*****************************************************************************/
static gboolean runMopacFiles(gint numberOfGeometries, ForceField** geometries, gdouble* energies, gchar* fileNamePrefix, gchar* keyWords)
{
	gint i;
	gint nG = 0;
	gint nM = 0;
	gchar* str = NULL;
	for(i=0;i<numberOfGeometries;i++)
	{
		if(!geometries[i]) continue;
		nG++;
		if(str) g_free(str);
		if(strstr(keyWords,"AM1"))
		str = g_strdup_printf("Minimization by AM1/Mopac of geometry n = %d... Please wait", i+1);
		else
		str = g_strdup_printf("Minimization by PM6/Mopac of geometry n = %d... Please wait", i+1);
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		if(runOneMopac(geometries[i], &energies[i], fileNamePrefix, keyWords)) 
		{
			freeMolecule(&geometries[i]->molecule);
			geometries[i]->molecule = createMolecule(geometry0,Natoms,TRUE);
			nM++;
		}
		if(StopCalcul)break;
	}
	if(str) g_free(str);
	if(nM==nG) return TRUE;
	return FALSE;

}
/*****************************************************************************/
static gboolean getEnergyFireFly(gchar* fileNameOut, gdouble* energy)
{
	FILE* file = NULL;
	gchar buffer[1024];
	gchar* pdest = NULL;
	gboolean OK = FALSE;

 	file = fopen(fileNameOut, "rb");
	if(!file) return FALSE;
	 while(!feof(file))
	 {
		if(!fgets(buffer,BSIZE,file))break;
		pdest = strstr( buffer, "HEAT OF FORMATION IS");
		if(pdest) 
		{
			pdest = strstr( buffer, "S");
			if(pdest)
			{
				if(sscanf(pdest+1,"%lf",energy)==1)
					OK = TRUE;
			}
		}
	 }
	fclose(file);
	return OK;
}
/*****************************************************************************/
static gboolean runOneFireFly(ForceField* geometry, gdouble* energy, gchar* fileNamePrefix, gchar* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gint j;
	gchar* fileNameIn = NULL;
	gchar* fileNameOut = NULL;
	gchar* fileNameSH = NULL;
	gchar multiplicityStr[100];
	gchar buffer[1024];
#ifdef G_OS_WIN32
	gchar c='%';
#endif

	if(!geometry) return FALSE;
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%sPCGOne.sh",fileNamePrefix);
#else
	fileNameSH = g_strdup_printf("%sPCGOne.bat",fileNamePrefix);
#endif
 	fileSH = fopen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"set PATH=%cPATH%c;\"%s\"\n",c,c,fireflyDirectory);
#endif

	getMultiplicityName(spinMultiplicity, multiplicityStr);

	fileNameIn = g_strdup_printf("%sOne.inp",fileNamePrefix);
 	file = fopen(fileNameIn, "w");
	if(!file) 
	{
 		if(fileNameIn) g_free(fileNameIn);
 		if(fileNameOut) g_free(fileNameOut);
 		if(fileNameSH) g_free(fileNameSH);
		return FALSE;
	}
	fprintf(file,"! ======================================================\n");
	fprintf(file,"!  Input file for FireFly\n"); 
	fprintf(file,"! ======================================================\n");
	if(strstr(keyWords,"RUNTYP"))
	{
		sscanf(strstr(keyWords,"RUNTYP"),"%s",buffer);
		fprintf(file," $CONTRL %s $END\n",buffer);
	}
	if(strstr(keyWords,"SCFTYP"))
	{
		sscanf(strstr(keyWords,"SCFTYP"),"%s",buffer);
		fprintf(file," $CONTRL %s $END\n",buffer);
	}
	else
	{
		if(spinMultiplicity==1)
			fprintf(file," $CONTRL SCFTYP=RHF $END\n");
		else
			fprintf(file," $CONTRL SCFTYP=UHF $END\n");
	}

	fprintf(file," $CONTRL ICHARG=%d MULT=%d $END\n",totalCharge,spinMultiplicity);
	if(strstr(keyWords,"GBASIS"))
	{
		sscanf(strstr(keyWords,"GBASIS"),"%s",buffer);
		fprintf(file," $BASIS %s $END\n",buffer);
	}
	fprintf(file," $DATA\n");
	fprintf(file,"Molecule specification\n");
	fprintf(file,"C1\n");
	for(j=0;j<geometry->molecule.nAtoms;j++)
	{
		gchar* symbol = geometry->molecule.atoms[j].prop.symbol;
		SAtomsProp prop = prop_atom_get(symbol);
		fprintf(file,"%s %f %f %f %f\n", 
			symbol,
			(gdouble)prop.atomicNumber,
			geometry->molecule.atoms[j].coordinates[0],
			geometry->molecule.atoms[j].coordinates[1],
			geometry->molecule.atoms[j].coordinates[2]
			);
	}
	fprintf(file," $END\n");
	fclose(file);
	fileNameOut = g_strdup_printf("%sOne.out",fileNamePrefix);
#ifndef G_OS_WIN32
	if(!strcmp(NameCommandFireFly,"pcgamess") || !strcmp(NameCommandFireFly,"nohup pcgamess")||
	!strcmp(NameCommandFireFly,"firefly") || !strcmp(NameCommandFireFly,"nohup firefly"))
	{
		fprintf(fileSH,"mkdir %stmp\n",fileNamePrefix);
		fprintf(fileSH,"cd %stmp\n",fileNamePrefix);
		fprintf(fileSH,"cp %s input\n",fileNameIn);
		fprintf(fileSH,"%s -p -o %s\n",NameCommandFireFly,fileNameOut);
		fprintf(fileSH,"cd ..\n");
		fprintf(fileSH,"rm PUNCH\n");
		fprintf(fileSH,"/bin/rm -r  %stmp\n",fileNamePrefix);
	}
	else
		fprintf(fileSH,"%s %s",NameCommandFireFly,fileNameIn);
#else
	if(!strcmp(NameCommandFireFly,"pcgamess") ||
	!strcmp(NameCommandFireFly,"firefly") )
	{
        	fprintf(fileSH,"mkdir \"%stmp\"\n",fileNamePrefix);
		addUnitDisk(fileSH, fileNamePrefix);
	 	fprintf(fileSH,"cd \"%stmp\"\n",fileNamePrefix);
         	fprintf(fileSH,"copy \"%s\" input\n",fileNameIn);
         	fprintf(fileSH,"%s -p -o \"%s\"\n",NameCommandFireFly,fileNameOut);
	 	fprintf(fileSH,"cd ..\n");
         	fprintf(fileSH,"del PUNCH\n");
         	fprintf(fileSH,"del /Q  \"%stmp\"\n",fileNamePrefix);
         	fprintf(fileSH,"rmdir  \"%stmp\"\n",fileNamePrefix);
	}
	else
		fprintf(fileSH,"%s %s",NameCommandFireFly,fileNameIn);
#endif
	fclose(fileSH);
#ifndef G_OS_WIN32
	sprintf(buffer,"chmod u+x %s",fileNameSH);
	{int ierr = system(buffer);}
	{int ierr = system(fileNameSH);}
#else
	sprintf(buffer,"\"%s\"",fileNameSH);
	{int ierr = system(buffer);}
#endif
	if(getEnergyFireFly(fileNameOut,energy))
	{
		gchar* str = NULL;

		read_geom_from_gamess_output_file(fileNameOut, -1);
		str = g_strdup_printf("Energy by FireFly = %f", *energy);
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		Waiting(1);
		if(str) g_free(str);
	}
	else
	{
 		if(fileNameIn) g_free(fileNameIn);
 		if(fileNameOut) g_free(fileNameOut);
 		if(fileNameSH) g_free(fileNameSH);
		return FALSE;
	}

 	if(fileNameIn) g_free(fileNameIn);
 	if(fileNameOut) g_free(fileNameOut);
 	if(fileNameSH) g_free(fileNameSH);
	return TRUE;
}
/*****************************************************************************/
static gboolean runFireFlyFiles(gint numberOfGeometries, ForceField** geometries, gdouble* energies, gchar* fileNamePrefix, gchar* keyWords)
{
	gint i;
	gint nG = 0;
	gint nM = 0;
	gchar* str = NULL;
	for(i=0;i<numberOfGeometries;i++)
	{
		if(!geometries[i]) continue;
		nG++;
		if(str) g_free(str);
		str = g_strdup_printf("Minimization by FireFly of geometry n = %d... Please wait", i+1);
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		if(runOneFireFly(geometries[i], &energies[i], fileNamePrefix, keyWords)) 
		{
			freeMolecule(&geometries[i]->molecule);
			geometries[i]->molecule = createMolecule(geometry0,Natoms,TRUE);
			nM++;
		}
		if(StopCalcul)break;
	}
	if(str) g_free(str);
	if(nM==nG) return TRUE;
	return FALSE;

}
/*****************************************************************************/
static gboolean testEqualDistances(gdouble* distancesI, gdouble* distancesJ, gint n, gdouble tol)
{
	gint k;
	if(!distancesI) return FALSE;
	if(!distancesJ) return FALSE;
	if(n<1) return FALSE;
	for (  k = 0; k < n; k++ )
		if(fabs(distancesI[k]-distancesJ[k])>tol) return FALSE;
	return TRUE;
}
/*****************************************************************************/
static gdouble* getDistancesBetweenAtoms(ForceField* forceField)
{
	gdouble* distances = NULL;
	gint i;
	gint j;
	gint n;
	gint k;
	if(forceField->molecule.nAtoms<1) return distances;
	n = forceField->molecule.nAtoms*(forceField->molecule.nAtoms-1)/2;
	distances = g_malloc(n*sizeof(gdouble));
	n = 0;
	for (  i = 0; i < forceField->molecule.nAtoms-1; i++ )
	for (  j = i+1; j < forceField->molecule.nAtoms; j++ )
	{
		gdouble x = forceField->molecule.atoms[i].coordinates[0]-forceField->molecule.atoms[j].coordinates[0];
		gdouble y = forceField->molecule.atoms[i].coordinates[1]-forceField->molecule.atoms[j].coordinates[1];
		gdouble z = forceField->molecule.atoms[i].coordinates[2]-forceField->molecule.atoms[j].coordinates[2];
		distances[n++] = x*x + y*y + z*z;
	}
	for(i=0;i<n-1;i++)
	{
		k = i;
		for(j=i+1;j<n;j++)
			if(distances[j]<distances[k]) k= j;
		if(k!=i)
		{
			gdouble d = distances[i];
			distances[i] = distances[k];
			distances[k] = d;
		}
	}
	return distances;
}
/*****************************************************************************/
static void removedsToEnd(gint numberOfGeometries, ForceField** geometries, gdouble* energies, gboolean* removeds)
{
	if(geometries && energies && removeds)
	{
		gint i;
		gint j;
		gint k;
		for(i=0;i<numberOfGeometries-1;i++)
		{
			if(!removeds[i]) continue;
			k = i;
			for(j=i+1;j<numberOfGeometries;j++)
				if(!removeds[j]) { k= j; break;}
			if(k!=i)
			{
				gdouble energy = energies[i];
				gboolean r = removeds[i];
				ForceField* g = geometries[i];

				energies[i] = energies[k];
				energies[k] = energy;
				geometries[i] = geometries[k];
				geometries[k] = g;
				removeds[i] = removeds[k];
				removeds[k] = r;
			}
		}
	}
}
/*****************************************************************************/
static void computeRemoveds(gint numberOfGeometries, ForceField** geometries, gdouble* energies, gboolean *removeds, 
		gdouble tolEnergy, gdouble tolDistance)
{
	gint i;
	gint j;
	gdouble* distancesI = NULL;
	gdouble* distancesJ = NULL;
	if(tolDistance<=0 && tolEnergy<=0) return;
	if(!geometries || !energies) return;
	if(numberOfGeometries<1) return;
	for(i=0;i<numberOfGeometries-1;i++)
	{
		gint n;
		if(removeds[i]) continue;
		if(tolDistance>0) distancesI =  getDistancesBetweenAtoms(geometries[i]);
		n = geometries[i]->molecule.nAtoms*(geometries[i]->molecule.nAtoms-1)/2;
		for(j=i+1;j<numberOfGeometries;j++)
		{
			if(removeds[j]) continue;
			if(tolEnergy>0 && fabs(energies[j]-energies[i])<tolEnergy && geometries[i]->molecule.nAtoms==geometries[j]->molecule.nAtoms)
			{
				if(tolDistance>0) 
				{
					distancesJ =  getDistancesBetweenAtoms(geometries[j]);
					if(testEqualDistances(distancesI, distancesJ, n, tolDistance))
						removeds[j] = TRUE;
					if(distancesJ) g_free(distancesJ);
					distancesJ = NULL;
				}
				else
					removeds[j] = TRUE;
			}
			if(tolEnergy<0 && tolDistance>0 && geometries[i]->molecule.nAtoms==geometries[j]->molecule.nAtoms)
			{
				distancesJ =  getDistancesBetweenAtoms(geometries[j]);
				if(testEqualDistances(distancesI, distancesJ, n, tolDistance))
					removeds[j] = TRUE;
				if(distancesJ) g_free(distancesJ);
				distancesJ = NULL;
			}
		}
		if(distancesI) g_free(distancesI);
		distancesI = NULL;
	}

}
/*****************************************************************************/
static void removeIdenticalGeometries(gint* nG, ForceField*** geoms, gdouble** eners, gdouble tolEnergy, gdouble tolDistance)
{
	gint i;
	gint numberOfGeometries =*nG;
	ForceField** geometries = *geoms; 
	gdouble* energies = *eners;
	gboolean* removeds = NULL;
	gint newN = 0;
	if(numberOfGeometries<1) return;
	removeds = g_malloc(numberOfGeometries*sizeof(gboolean));
	for(i=0;i<numberOfGeometries;i++) removeds[i] = FALSE;
	computeRemoveds(numberOfGeometries, geometries, energies, removeds, tolEnergy, tolDistance);
	removedsToEnd(numberOfGeometries, geometries, energies, removeds);

	for(i=0;i<numberOfGeometries;i++) 
	{
		if(removeds[i]) 
		{
			if(geometries[i]) freeForceField(geometries[i]);
		}
		else newN++;
	}
	if(newN==0) newN = 1;
	if(newN==numberOfGeometries) return;
	*nG = newN;
	*eners = g_realloc(*eners,newN*sizeof(gdouble));
	*geoms = g_realloc(*geoms,newN*sizeof(ForceField**));

	g_free(removeds);
}
/*****************************************************************************/
static void sortGeometries(gint numberOfGeometries, ForceField** geometries, gdouble* energies)
{
	if(geometries && energies)
	{
		gint i;
		gint j;
		gint k;
		for(i=0;i<numberOfGeometries-1;i++)
		{
			k = i;
			for(j=i+1;j<numberOfGeometries;j++)
				if(energies[j]<energies[k]) k= j;
			if(k!=i)
			{
				gdouble energy = energies[i];
				ForceField* g = geometries[i];

				energies[i] = energies[k];
				energies[k] = energy;
				geometries[i] = geometries[k];
				geometries[k] = g;
			}
		}
	}
}
/*****************************************************************************/
static void createPostProcessingFiles(gint numberOfGeometries, ForceField** geometries,gdouble* energies,gchar* fileNameGeom, gchar* mopacKeywords, gchar* gaussianKeywords, gchar* fireflyKeywords, gchar* message)
{
	if(!StopCalcul && mopacKeywords)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
		createMopacFiles(numberOfGeometries, geometries, energies, fileNamePrefix, mopacKeywords);
		strcat(message,fileNamePrefix);
		strcat(message,_("_*.mop\n\tFiles for a post processing by Mopac\n\n"));
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	if(!StopCalcul && gaussianKeywords)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
		createGaussianFiles(numberOfGeometries, geometries, energies, fileNamePrefix, gaussianKeywords);
		strcat(message,fileNamePrefix);
		strcat(message,_("_*.com\n\tFiles for a post processing by Gaussian\n\n"));
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	if(!StopCalcul && fireflyKeywords)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
		createFireFlyFiles(numberOfGeometries, geometries, energies, fileNamePrefix, fireflyKeywords);
		strcat(message,fileNamePrefix);
		strcat(message,_("P_*.inp\n\tFiles for a post processing by FireFly\n\n"));
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
}
/*****************************************************************************/
static void amberMolecularDynamicsConfo(GtkWidget* Win, gpointer data)
{
	ForceField forceField; 
	ForceFieldOptions forceFieldOptions;
	MolecularDynamics molecularDynamics;
	gint updateFrequency = 1;
	gdouble heatTime;
	gdouble equiTime;
	gdouble runTime;
	gdouble heatTemp; 
	gdouble equiTemp; 
	gdouble runTemp; 
	gdouble stepSize;
	MDIntegratorType integrator = VERLET;
	gchar* fileNameGeom = NULL;
	gchar* fileNameTraj = NULL;
	gchar* fileNameProp = NULL;
	gchar* mopacKeywords = NULL;
	gchar* gaussianKeywords = NULL;
	gchar* fireflyKeywords = NULL;
	gdouble friction=40;
	gdouble collide = 20;
	MDThermostatType thermostat = NONE;
	gint numberOfGeometries = 2;
	ForceField** geometries = NULL; 
	gdouble* energies = NULL;
	gboolean optMM = FALSE;
	gboolean optPM6Mopac = FALSE;
	gboolean optAM1Mopac = FALSE;
	gboolean optFireFly = FALSE;

	QuasiNewton quasiNewton;
	gboolean useConjugateGradient;
	gboolean useQuasiNewton;
	ConjugateGradientOptions conjugateGradientOptions;
	SteepestDescent steepestDescent;
	ConjugateGradient conjugateGradient;
	gint i;
	gchar message[BSIZE]="Created files :\n";
	gdouble tolEnergy = -1;
	gdouble tolDistance = -1;

	if(GTK_TOGGLE_BUTTON (buttonTolerance[TOLE])->active)
		tolEnergy = atoi(gtk_entry_get_text(GTK_ENTRY(entryTolerance[TOLE])));
	if(GTK_TOGGLE_BUTTON (buttonTolerance[TOLD])->active)
		tolDistance = atoi(gtk_entry_get_text(GTK_ENTRY(entryTolerance[TOLD])));

	forceFieldOptions.type = AMBER;
	forceFieldOptions.bondStretch = GTK_TOGGLE_BUTTON (buttonMMOptions[MMBOND])->active;
	forceFieldOptions.angleBend = GTK_TOGGLE_BUTTON (buttonMMOptions[MMBEND])->active;
	forceFieldOptions.dihedralAngle = GTK_TOGGLE_BUTTON (buttonMMOptions[MMTORSION])->active;
	forceFieldOptions.improperTorsion = GTK_TOGGLE_BUTTON (buttonMMOptions[MMIMPROPER])->active;
	forceFieldOptions.nonBonded = GTK_TOGGLE_BUTTON (buttonMMOptions[MMNONBOND])->active;
	forceFieldOptions.hydrogenBonded = GTK_TOGGLE_BUTTON (buttonMMOptions[MMHBOND])->active;
	forceFieldOptions.coulomb = GTK_TOGGLE_BUTTON (buttonMMOptions[MMCOULOMB])->active;
	forceFieldOptions.vanderWals = GTK_TOGGLE_BUTTON (buttonMMOptions[PWVANDERWALS])->active;

	forceFieldOptions.rattleConstraints = NOCONSTRAINTS;
	if(GTK_TOGGLE_BUTTON (buttonConstraintsOptions[BONDSCONSTRAINTS])->active)
			forceFieldOptions.rattleConstraints = BONDSCONSTRAINTS;
	if(GTK_TOGGLE_BUTTON (buttonConstraintsOptions[BONDSANGLESCONSTRAINTS])->active)
			forceFieldOptions.rattleConstraints = BONDSANGLESCONSTRAINTS;

	if(GTK_TOGGLE_BUTTON (buttonTypesOptions[AMBER])->active )
		forceFieldOptions.type = AMBER;
	else
	if(GTK_TOGGLE_BUTTON (buttonTypesOptions[PAIRWISE])->active )
	{
		forceFieldOptions.coulomb = GTK_TOGGLE_BUTTON (buttonMMOptions[PWCOULOMB])->active;
		forceFieldOptions.type = PAIRWISE;
	}
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;

	updateFrequency = atoi(gtk_entry_get_text(GTK_ENTRY(entryMDRafresh)));
	if(updateFrequency<0) updateFrequency = 0;

	heatTime = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTimes[0])));
	equiTime = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTimes[1])));
	runTime = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTimes[2])));

	heatTemp = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTemperature[0])));
	runTemp = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTemperature[2])));
	/* equiTemp = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTemperature[1])));*/
	equiTemp = runTemp;

	stepSize = atof(gtk_entry_get_text(GTK_ENTRY(entryMDStepSize)));


	if(GTK_TOGGLE_BUTTON (buttonMDOptions[BEEMAN])->active) integrator = BEEMAN;
	if(GTK_TOGGLE_BUTTON (buttonMDOptions[STOCHASTIC])->active) integrator = STOCHASTIC;

	if(GTK_TOGGLE_BUTTON (buttonMDThermOptions[ANDERSEN])->active) thermostat = ANDERSEN;
	if(GTK_TOGGLE_BUTTON (buttonMDThermOptions[BERENDSEN])->active) thermostat = BERENDSEN;
	if(GTK_TOGGLE_BUTTON (buttonMDThermOptions[BUSSI])->active) thermostat = BUSSI;

	if( integrator == STOCHASTIC)
		friction = atof(gtk_entry_get_text(GTK_ENTRY(entrySDFriction)));

	collide = atof(gtk_entry_get_text(GTK_ENTRY(entrySDCollide)));

	if(heatTime<0) heatTime = 1;
	if(equiTime<0) equiTime = 1;
	if(runTime<0) runTime = 1;

	if(heatTemp<0) heatTemp = 0;
	if(equiTemp<0) runTemp = 300;
	if(runTemp<0) runTemp = 300;

	if(stepSize<0) stepSize = 1.0;
	if(stepSize>5) stepSize = 5.0;

	optMM = GTK_TOGGLE_BUTTON (buttonPostMMOpt)->active;
	optPM6Mopac = GTK_TOGGLE_BUTTON (buttonPostPM6Mopac)->active;
	optAM1Mopac = GTK_TOGGLE_BUTTON (buttonPostAM1Mopac)->active;
	optFireFly = GTK_TOGGLE_BUTTON (buttonPostFireFly)->active;
	/* number for geometries */
	{
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryNumberOfGeom)));
		numberOfGeometries = atoi(tmp);
		if(numberOfGeometries<2) numberOfGeometries = 2;
		g_free(tmp);
	}
	/* fileName for geometries */
	{
		gchar* dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFileNameGeom)));
		if(!dirName) dirName = g_strdup(g_get_home_dir());
		if(dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
			fileNameGeom = g_strdup_printf("%s%s%s",dirName, G_DIR_SEPARATOR_S,tmp);
		else
			fileNameGeom = g_strdup_printf("%s%s",dirName, tmp);


		g_free(tmp);
		g_free(dirName);
	}
	if(GTK_TOGGLE_BUTTON (buttonCreateGaussian)->active)
		gaussianKeywords = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryGaussianKeywords)));
	if(GTK_TOGGLE_BUTTON (buttonCreateMopac)->active)
		mopacKeywords = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryMopacKeywords)));
	if(GTK_TOGGLE_BUTTON (buttonCreateFireFly)->active)
		fireflyKeywords = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFireFlyKeywords)));

	if(GTK_TOGGLE_BUTTON (buttonSaveTraj)->active)
	{
		gchar* dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFileNameTraj)));
		if(dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
			fileNameTraj = g_strdup_printf("%s%s%s",dirName, G_DIR_SEPARATOR_S,tmp);
		else
			fileNameTraj = g_strdup_printf("%s%s",dirName, tmp);


		g_free(tmp);
		g_free(dirName);
	}
	if(GTK_TOGGLE_BUTTON (buttonSaveProp)->active)
	{
		gchar* dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFileNameProp)));
		if(dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
			fileNameProp = g_strdup_printf("%s%s%s",dirName, G_DIR_SEPARATOR_S,tmp);
		else
			fileNameProp = g_strdup_printf("%s%s",dirName, tmp);
		g_free(tmp);
		g_free(dirName);
		/* printf("fileNameProp = %s\n", fileNameProp);*/
	}

/* Optimsation options */ 
	conjugateGradientOptions.gradientNorm = 1e-3;
	conjugateGradientOptions.maxIterations = 100;
	conjugateGradientOptions.updateFrequency = 1;
	conjugateGradientOptions.maxLines = 25;
	conjugateGradientOptions.initialStep = 0.001;
	conjugateGradientOptions.method = 1;

	useConjugateGradient = GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADCONJUGATE])->active;
	useQuasiNewton = GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADQUASINEWTON])->active;

	conjugateGradientOptions.gradientNorm  = 
		atof(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADEPSILON])));
	conjugateGradientOptions.maxIterations = 
		atoi(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADMAXITERATIONS])));
	conjugateGradientOptions.updateFrequency = 
		atoi(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADFREQUENCY])));
	conjugateGradientOptions.initialStep = 
		atof(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADINITIALSTEP])));
	conjugateGradientOptions.maxLines = 
		atoi(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADMAXLINES])));

	quasiNewton.maxIterations = 
		atoi(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADMAXITERATIONS])));
	quasiNewton.updateFrequency = 
		atoi(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADFREQUENCY])));
	quasiNewton.epsilon  = 
		atof(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADEPSILON])));
	quasiNewton.tolerence = 1e-16;  
	quasiNewton.maxLines = 
		atoi(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADMAXLINES])));
	quasiNewton.forceField = NULL;

	if(GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADHESTENES])->active)
		conjugateGradientOptions.method = 1;
	else
	if(GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADFLETCHER])->active)
		conjugateGradientOptions.method = 2;
	else
	if(GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADPOLAK])->active)
		conjugateGradientOptions.method = 3;
	else
	if(GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADWOLF])->active)
		conjugateGradientOptions.method = 4;



	gtk_widget_destroy(Win);
    	while( gtk_events_pending() ) gtk_main_iteration();

	set_sensitive_stop_button( TRUE);
	StopCalcul = FALSE;

	if(forceFieldOptions.type==AMBER)
		forceField = createAmberModel(geometry0,Natoms,forceFieldOptions);
	else
	if(forceFieldOptions.type==PAIRWISE)
		forceField = createPairWiseModel(geometry0,Natoms,forceFieldOptions);

	if(StopCalcul)
	{
		set_text_to_draw(" ");
		set_statubar_operation_str(_("Calculation canceled"));
		drawGeom();
		set_sensitive_stop_button( FALSE);
		return;
	}


	set_sensitive_stop_button( TRUE);
	{
		gint i;
		for(i=0;i<4;i++) NumSelAtoms[i] = -1;
		change_of_center(NULL,NULL);
	}

	geometries = runMolecularDynamicsConfo(&molecularDynamics, &forceField,
		updateFrequency, heatTime, equiTime, runTime, heatTemp, equiTemp, runTemp, stepSize, 
		integrator, thermostat, friction, collide, numberOfGeometries, fileNameTraj, fileNameProp);

	freeForceField(&forceField);
	if(!StopCalcul && geometries && numberOfGeometries>0)
		energies = g_malloc(numberOfGeometries*sizeof(gdouble));

	if(!StopCalcul && geometries && optMM)
	for(i=0;i<numberOfGeometries;i++)
	{
		gchar* str = NULL;
		energies[i] = 1e30;
		if(!geometries[i]) continue;
		if(str) g_free(str);
		str = g_strdup_printf("Minimization of geometry number %d ", i+1);
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		Waiting(1);
		if(str) g_free(str);


		if(StopCalcul)
		{
			set_text_to_draw(" ");
			set_statubar_operation_str(_("Calculation canceled"));
			drawGeom();
			set_sensitive_stop_button( FALSE);
			break;
		}
		if(useConjugateGradient)
		{
			set_sensitive_stop_button( TRUE);
			runConjugateGradient(&conjugateGradient, geometries[i], conjugateGradientOptions); 
			if(StopCalcul)
			{
				set_text_to_draw(" ");
				set_statubar_operation_str(_("Calculation canceled"));
				drawGeom();
			}
			set_sensitive_stop_button( FALSE);
			energies[i] = conjugateGradient.forceField->klass->calculateEnergyTmp
				(conjugateGradient.forceField, &conjugateGradient.forceField->molecule );
			freeConjugateGradient(&conjugateGradient);
		}
		else if(useQuasiNewton)
		{
			QuasiNewton tmpQuasiNewton = quasiNewton;
			set_sensitive_stop_button( TRUE);
			tmpQuasiNewton.forceField = geometries[i];
                	runQuasiNewton(&tmpQuasiNewton);
			if(StopCalcul)
			{
				set_text_to_draw(" ");
				set_statubar_operation_str(_("Calculation canceled"));
				drawGeom();
			}
			set_sensitive_stop_button( FALSE);
			energies[i] = tmpQuasiNewton.forceField->klass->calculateEnergyTmp
				(tmpQuasiNewton.forceField, &tmpQuasiNewton.forceField->molecule );
			freeQuasiNewton(&tmpQuasiNewton);

		}
		else
		{
			set_sensitive_stop_button( TRUE);
			runSteepestDescent(&steepestDescent, geometries[i],
			       	conjugateGradientOptions.updateFrequency,
			       conjugateGradientOptions.maxIterations,
			       conjugateGradientOptions.gradientNorm,
			       conjugateGradientOptions.maxLines);
			if(StopCalcul)
			{
				set_text_to_draw(" ");
				set_statubar_operation_str(_("Calculation canceled"));
				drawGeom();
			}
			set_sensitive_stop_button( FALSE);
			energies[i] = steepestDescent.forceField->klass->calculateEnergyTmp
				(steepestDescent.forceField, &steepestDescent.forceField->molecule );
			freeSteepestDescent(&steepestDescent);
		}
		set_text_to_draw(" ");
	}
	else if(!StopCalcul)
	{
		for(i=0;i<numberOfGeometries;i++)
		{
			energies[i] = 1e30;
			if(!geometries[i]) continue;
			energies[i] = geometries[i]->klass->calculateEnergyTmp
				(geometries[i], &geometries[i]->molecule );
		}

	}

	if(StopCalcul)
	{
		set_text_to_draw(" ");
		set_statubar_operation_str(_("Calculation canceled"));
		drawGeom();
	}
	set_sensitive_stop_button( FALSE);
	set_text_to_draw(" ");
	/*  sort by energies */
	if(!StopCalcul) 
	{
		sortGeometries(numberOfGeometries, geometries, energies);
		removeIdenticalGeometries(&numberOfGeometries, &geometries, &energies,tolEnergy,tolDistance);
	}
	/* printf("fileNameGeom = %s\n",fileNameGeom);*/
	if(!StopCalcul && saveConfoGeometries(numberOfGeometries, geometries, energies, fileNameGeom))
	{
		createPostProcessingFiles(numberOfGeometries, geometries,energies,fileNameGeom, mopacKeywords, gaussianKeywords, fireflyKeywords, message);
		if(!StopCalcul) read_gabedit_file_add_list(fileNameGeom);
		strcat(message,fileNameGeom);
		strcat(message,_("\n\tGeometries selected and optimized using your MM potentials"));
		strcat(message,_("\n\tTo read this file : 'Read/Gabedit file'\n\n"));
	}
	/* minimazation by mopac PM6*/
	if(!StopCalcul && optPM6Mopac)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
		if(runMopacFiles(numberOfGeometries, geometries, energies, fileNamePrefix, "PM6 XYZ") && !StopCalcul)
		{
			gchar* fileNameGeomMop = g_strdup_printf("%sMop.gab",fileNamePrefix);
			sortGeometries(numberOfGeometries, geometries, energies);
			removeIdenticalGeometries(&numberOfGeometries, &geometries, &energies,tolEnergy,tolDistance);
			if(saveConfoGeometries(numberOfGeometries, geometries, energies, fileNameGeomMop))
			{
				createPostProcessingFiles(numberOfGeometries, geometries,energies,fileNameGeomMop, mopacKeywords, gaussianKeywords, fireflyKeywords, message);
				read_gabedit_file_add_list(fileNameGeomMop);
				strcat(message,fileNameGeomMop);
				strcat(message,_("\n\tGeometries after minimization by PM6/Mopac"));
				strcat(message,_("\n\tTo read this file : 'Read/Gabedit file'\n\n"));
			}

			g_free(fileNameGeomMop);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	/* minimazation by mopac AM1*/
	if(!StopCalcul && optAM1Mopac)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
		if(runMopacFiles(numberOfGeometries, geometries, energies, fileNamePrefix, "AM1 XYZ") && !StopCalcul)
		{
			gchar* fileNameGeomMop = g_strdup_printf("%sMop.gab",fileNamePrefix);
			sortGeometries(numberOfGeometries, geometries, energies);
			removeIdenticalGeometries(&numberOfGeometries, &geometries, &energies,tolEnergy,tolDistance);
			if(saveConfoGeometries(numberOfGeometries, geometries, energies, fileNameGeomMop))
			{
				createPostProcessingFiles(numberOfGeometries, geometries,energies,fileNameGeomMop, mopacKeywords, gaussianKeywords, fireflyKeywords, message);
				read_gabedit_file_add_list(fileNameGeomMop);
				strcat(message,fileNameGeomMop);
				strcat(message,_("\n\tGeometries after minimization by AM1/Mopac"));
				strcat(message,_("\n\tTo read this file : 'Read/Gabedit file'\n\n"));
			}

			g_free(fileNameGeomMop);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	/* minimazation by FireFly AM1*/
	if(!StopCalcul && optFireFly)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
		if(runFireFlyFiles(numberOfGeometries, geometries, energies, fileNamePrefix, "RUNTYP=Optimize GBASIS=AM1") && !StopCalcul)
		{
			gchar* fileNameGeomFireFly = g_strdup_printf("%sFireFly.gab",fileNamePrefix);
			sortGeometries(numberOfGeometries, geometries, energies);
			removeIdenticalGeometries(&numberOfGeometries, &geometries, &energies,tolEnergy,tolDistance);
			if(saveConfoGeometries(numberOfGeometries, geometries, energies, fileNameGeomFireFly))
			{
				createPostProcessingFiles(numberOfGeometries, geometries,energies,fileNameGeomFireFly, mopacKeywords, gaussianKeywords, fireflyKeywords, message);
				read_gabedit_file_add_list(fileNameGeomFireFly);
				strcat(message,fileNameGeomFireFly);
				strcat(message,_("\n\tGeometries after minimization by FireFly"));
				strcat(message,_("\n\tTo read this file : 'Read/Gabedit file'\n\n"));
			}

			g_free(fileNameGeomFireFly);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}

	if(geometries)
	{
		for(i=0;i<numberOfGeometries;i++)
			if(geometries[i]) freeForceField(geometries[i]);
		g_free(geometries);
	}
	if(energies) g_free(energies);
	if(strlen(message)>20 && !StopCalcul ) Message(message,_("Info"),TRUE);

}
/*****************************************************************************/
static void amberMolecularDynamics(GtkWidget* Win, gpointer data)
{
	ForceField forceField; 
	ForceFieldOptions forceFieldOptions;
	MolecularDynamics molecularDynamics;
	gint updateFrequency = 1;
	gdouble heatTime;
	gdouble equiTime;
	gdouble runTime;
	gdouble coolTime; 
	gdouble heatTemp; 
	gdouble equiTemp; 
	gdouble runTemp; 
	gdouble coolTemp; 
	gdouble stepSize;
	MDIntegratorType integrator = VERLET;
	gchar* fileNameTraj = NULL;
	gchar* fileNameProp = NULL;
	gdouble friction=40;
	gdouble collide = 20;
	MDThermostatType thermostat = NONE;


	forceFieldOptions.type = AMBER;
	forceFieldOptions.bondStretch = GTK_TOGGLE_BUTTON (buttonMMOptions[MMBOND])->active;
	forceFieldOptions.angleBend = GTK_TOGGLE_BUTTON (buttonMMOptions[MMBEND])->active;
	forceFieldOptions.dihedralAngle = GTK_TOGGLE_BUTTON (buttonMMOptions[MMTORSION])->active;
	forceFieldOptions.improperTorsion = GTK_TOGGLE_BUTTON (buttonMMOptions[MMIMPROPER])->active;
	forceFieldOptions.nonBonded = GTK_TOGGLE_BUTTON (buttonMMOptions[MMNONBOND])->active;
	forceFieldOptions.hydrogenBonded = GTK_TOGGLE_BUTTON (buttonMMOptions[MMHBOND])->active;
	forceFieldOptions.coulomb = GTK_TOGGLE_BUTTON (buttonMMOptions[MMCOULOMB])->active;
	forceFieldOptions.vanderWals = GTK_TOGGLE_BUTTON (buttonMMOptions[PWVANDERWALS])->active;
	forceFieldOptions.rattleConstraints = NOCONSTRAINTS;
	if(GTK_TOGGLE_BUTTON (buttonConstraintsOptions[BONDSCONSTRAINTS])->active)
			forceFieldOptions.rattleConstraints = BONDSCONSTRAINTS;
	if(GTK_TOGGLE_BUTTON (buttonConstraintsOptions[BONDSANGLESCONSTRAINTS])->active)
			forceFieldOptions.rattleConstraints = BONDSANGLESCONSTRAINTS;
	if(GTK_TOGGLE_BUTTON (buttonTypesOptions[AMBER])->active )
		forceFieldOptions.type = AMBER;
	else
	if(GTK_TOGGLE_BUTTON (buttonTypesOptions[PAIRWISE])->active )
	{
		forceFieldOptions.coulomb = GTK_TOGGLE_BUTTON (buttonMMOptions[PWCOULOMB])->active;
		forceFieldOptions.type = PAIRWISE;
	}

	updateFrequency = atoi(gtk_entry_get_text(GTK_ENTRY(entryMDRafresh)));
	if(updateFrequency<0) updateFrequency = 0;

	heatTime = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTimes[0])));
	equiTime = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTimes[1])));
	runTime = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTimes[2])));
	coolTime = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTimes[3])));

	heatTemp = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTemperature[0])));
	runTemp = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTemperature[2])));
	/* equiTemp = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTemperature[1])));*/
	equiTemp = runTemp;
	coolTemp = atof(gtk_entry_get_text(GTK_ENTRY(entryMDTemperature[3])));

	stepSize = atof(gtk_entry_get_text(GTK_ENTRY(entryMDStepSize)));


	if(GTK_TOGGLE_BUTTON (buttonMDOptions[BEEMAN])->active) integrator = BEEMAN;
	if(GTK_TOGGLE_BUTTON (buttonMDOptions[STOCHASTIC])->active) integrator = STOCHASTIC;

	if(GTK_TOGGLE_BUTTON (buttonMDThermOptions[ANDERSEN])->active) thermostat = ANDERSEN;
	if(GTK_TOGGLE_BUTTON (buttonMDThermOptions[BERENDSEN])->active) thermostat = BERENDSEN;
	if(GTK_TOGGLE_BUTTON (buttonMDThermOptions[BUSSI])->active) thermostat = BUSSI;

	if( integrator == STOCHASTIC)
		friction = atof(gtk_entry_get_text(GTK_ENTRY(entrySDFriction)));

	collide = atof(gtk_entry_get_text(GTK_ENTRY(entrySDCollide)));

	if(heatTime<0) heatTime = 1;
	if(equiTime<0) equiTime = 1;
	if(runTime<0) runTime = 1;
	if(coolTime<0) coolTime = 4;

	if(heatTemp<0) heatTemp = 0;
	if(equiTemp<0) runTemp = 300;
	if(runTemp<0) runTemp = 300;
	if(coolTemp<0) coolTemp = 0;

	if(stepSize<0) stepSize = 1.0;
	if(stepSize>5) stepSize = 5.0;

	if(GTK_TOGGLE_BUTTON (buttonSaveTraj)->active)
	{
		gchar* dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFileNameTraj)));
		if(dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
			fileNameTraj = g_strdup_printf("%s%s%s",dirName, G_DIR_SEPARATOR_S,tmp);
		else
			fileNameTraj = g_strdup_printf("%s%s",dirName, tmp);


		g_free(tmp);
		g_free(dirName);
	}
	if(GTK_TOGGLE_BUTTON (buttonSaveProp)->active)
	{
		gchar* dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFileNameProp)));
		if(dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
			fileNameProp = g_strdup_printf("%s%s%s",dirName, G_DIR_SEPARATOR_S,tmp);
		else
			fileNameProp = g_strdup_printf("%s%s",dirName, tmp);
		g_free(tmp);
		g_free(dirName);
		printf("fileNameProp = %s\n", fileNameProp);
	}



	gtk_widget_destroy(Win);
    	while( gtk_events_pending() ) gtk_main_iteration();

	set_sensitive_stop_button( TRUE);
	StopCalcul = FALSE;

	if(forceFieldOptions.type==AMBER)
		forceField = createAmberModel(geometry0,Natoms,forceFieldOptions);
	else
	if(forceFieldOptions.type==PAIRWISE)
		forceField = createPairWiseModel(geometry0,Natoms,forceFieldOptions);

	if(StopCalcul)
	{
		set_text_to_draw(" ");
		set_statubar_operation_str(_("Calculation canceled"));
		drawGeom();
		set_sensitive_stop_button( FALSE);
		return;
	}


	set_sensitive_stop_button( TRUE);
	{
		gint i;
		for(i=0;i<4;i++) NumSelAtoms[i] = -1;
		change_of_center(NULL,NULL);
	}

	runMolecularDynamics(&molecularDynamics, &forceField,
		updateFrequency, heatTime, equiTime, runTime, coolTime, heatTemp, equiTemp, runTemp, coolTemp, stepSize, 
		integrator, thermostat, friction, collide, fileNameTraj, fileNameProp);

	if(StopCalcul)
	{
		set_text_to_draw(" ");
		set_statubar_operation_str(_("Calculation canceled"));
		drawGeom();
	}
	set_sensitive_stop_button( FALSE);
	set_text_to_draw(" ");
	freeForceField(&forceField);
}
/********************************************************************************/
static void AddDynamicsOptionsDlg(GtkWidget *NoteBook, GtkWidget *win)
{

	gint i;
	gint j;
	GtkWidget *frame;
	GtkWidget *LabelOnglet;
	GtkWidget *LabelMenu;
	GtkWidget *vbox;
	GtkWidget *table;
	GtkWidget* label;
	GtkWidget *hseparator = NULL;

	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

	LabelOnglet = gtk_label_new("Molecular Dynamics Options");
	LabelMenu = gtk_label_new("Molecular Dynamics Options");
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook), frame,LabelOnglet, LabelMenu);
	g_object_set_data (G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	table = gtk_table_new(20,6,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 2;
	label = gtk_label_new("Heating");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 3;
	label = gtk_label_new("Equilibrium");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 4;
	label = gtk_label_new("Production");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 5;
	label = gtk_label_new("Cooling");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 0;
	label = gtk_label_new("Time (ps)");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 2;
	entryMDTimes[0] = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryMDTimes[0]),"1.0");
	gtk_widget_set_size_request(entryMDTimes[0], 60, -1);
	gtk_table_attach(GTK_TABLE(table),entryMDTimes[0], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 3;
	entryMDTimes[1] = gtk_entry_new();
	gtk_widget_set_size_request(entryMDTimes[1], 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDTimes[1]),"1.0");
	gtk_table_attach(GTK_TABLE(table),entryMDTimes[1], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 4;
	entryMDTimes[2] = gtk_entry_new();
	gtk_widget_set_size_request(entryMDTimes[2], 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDTimes[2]),"4.0");
	gtk_table_attach(GTK_TABLE(table),entryMDTimes[2], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 5;
	entryMDTimes[3] = gtk_entry_new();
	gtk_widget_set_size_request(entryMDTimes[3], 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDTimes[3]),"4.0");
	gtk_table_attach(GTK_TABLE(table),entryMDTimes[3], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 0;
	label = gtk_label_new("Temperature (K)");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 2;
	entryMDTemperature[0] = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryMDTemperature[0]),"0.0");
	gtk_widget_set_size_request(entryMDTemperature[0], 60, -1);
	gtk_table_attach(GTK_TABLE(table),entryMDTemperature[0], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	/*
	i = 2;
	j = 3;
	entryMDTemperature[1] = gtk_entry_new();
	gtk_widget_set_size_request(entryMDTemperature[1], 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDTemperature[1]),"300.0");
	gtk_table_attach(GTK_TABLE(table),entryMDTemperature[1], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
		  */
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 4;
	entryMDTemperature[2] = gtk_entry_new();
	gtk_widget_set_size_request(entryMDTemperature[2], 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDTemperature[2]),"300.0");
	gtk_table_attach(GTK_TABLE(table),entryMDTemperature[2], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 5;
	entryMDTemperature[3] = gtk_entry_new();
	gtk_widget_set_size_request(entryMDTemperature[3], 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDTemperature[3]),"0.0");
	gtk_table_attach(GTK_TABLE(table),entryMDTemperature[3], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 3;
	j = 0;
	label = gtk_label_new("Step size (fs)");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 3;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 3;
	j = 2;
	entryMDStepSize = gtk_entry_new();
	gtk_widget_set_size_request(entryMDStepSize, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDStepSize),"1.0");
	gtk_table_attach(GTK_TABLE(table),entryMDStepSize, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 0;
	label = gtk_label_new("Screen Rafresh after");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 2;
	entryMDRafresh = gtk_entry_new();
	gtk_widget_set_size_request(entryMDRafresh, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDRafresh),"5");
	gtk_table_attach(GTK_TABLE(table),entryMDRafresh, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 3;
	label = gtk_label_new("steps");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 5;
	j = 0;
	label = gtk_label_new("Collide(ps^-1)");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 5;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 5;
	j = 2;
	entrySDCollide = gtk_entry_new();
	gtk_widget_set_size_request(entrySDCollide, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entrySDCollide),"20.0");
	gtk_table_attach(GTK_TABLE(table),entrySDCollide, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 6;
	j = 0;
	label = gtk_label_new("Friction(ps^-1)");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 6;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 6;
	j = 2;
	entrySDFriction = gtk_entry_new();
	gtk_widget_set_size_request(entrySDFriction, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entrySDFriction),"40.0");
	gtk_table_attach(GTK_TABLE(table),entrySDFriction, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	/* THERMOSTAT */
/*----------------------------------------------------------------------------------*/
	i = 3;
	j = 4;
	buttonMDThermOptions[NONE]= gtk_radio_button_new_with_label( NULL, "NVE"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDThermOptions[NONE],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDThermOptions[NONE]), TRUE);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 4;
	buttonMDThermOptions[BERENDSEN]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDThermOptions[NONE])), "Berendsen"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDThermOptions[BERENDSEN],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDThermOptions[BERENDSEN]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 5;
	j = 4;
	buttonMDThermOptions[ANDERSEN]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDThermOptions[NONE])), "Andersen"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDThermOptions[ANDERSEN],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDThermOptions[ANDERSEN]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 6;
	j = 4;
	buttonMDThermOptions[BUSSI]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDThermOptions[NONE])), "Bussi"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDThermOptions[BUSSI],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDThermOptions[BUSSI]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 7;
	j = 0;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),hseparator, j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 8;
	j = 0;
	buttonMDOptions[VERLET]= gtk_radio_button_new_with_label( NULL, "MD Trajectory via Verlet velocity Algorithm"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDOptions[VERLET],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDOptions[VERLET]), TRUE);
/*----------------------------------------------------------------------------------*/
	i = 9;
	j = 0;
	buttonMDOptions[BEEMAN]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDOptions[VERLET])), "MD Trajectory via Modified Beeman Algorithm"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDOptions[BEEMAN],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDOptions[BEEMAN]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 10;
	j = 0;
	buttonMDOptions[STOCHASTIC]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDOptions[VERLET])), "Stochastic Dynamics via Verlet velocity Algorithm"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDOptions[STOCHASTIC],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDOptions[STOCHASTIC]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 11;
	j = 0;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),hseparator, j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 12;
	j = 0;
	buttonConstraintsOptions[NOCONSTRAINTS]= gtk_radio_button_new_with_label( NULL, "No constraints"); 
	gtk_table_attach(GTK_TABLE(table),buttonConstraintsOptions[NOCONSTRAINTS],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonConstraintsOptions[NOCONSTRAINTS]), TRUE);
/*----------------------------------------------------------------------------------*/
	i = 13;
	j = 0;
	buttonConstraintsOptions[BONDSCONSTRAINTS]= gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonConstraintsOptions[NOCONSTRAINTS])), "Bond constraints"); 
	gtk_table_attach(GTK_TABLE(table),buttonConstraintsOptions[BONDSCONSTRAINTS],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonConstraintsOptions[BONDSCONSTRAINTS]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 14;
	j = 0;
	buttonConstraintsOptions[BONDSANGLESCONSTRAINTS]= gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonConstraintsOptions[NOCONSTRAINTS])), "Bond & Angle constraints"); 
	gtk_table_attach(GTK_TABLE(table),buttonConstraintsOptions[BONDSANGLESCONSTRAINTS],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonConstraintsOptions[BONDSANGLESCONSTRAINTS]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 15;
	j = 0;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),hseparator, j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 16;
	j = 0;
	buttonSaveTraj = gtk_check_button_new_with_label("Save Trajectory in "); 
	gtk_table_attach(GTK_TABLE(table),buttonSaveTraj,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonSaveTraj), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 16;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 16;
	j = 2;
	entryFileNameTraj = gtk_entry_new();
	gtk_widget_set_size_request(entryFileNameTraj, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryFileNameTraj),"traj.gab");
	gtk_table_attach(GTK_TABLE(table),entryFileNameTraj, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 17;
	j = 0;
	buttonSaveProp = gtk_check_button_new_with_label("Save Properties in "); 
	gtk_table_attach(GTK_TABLE(table),buttonSaveProp,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonSaveProp), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 17;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 17;
	j = 2;
	entryFileNameProp = gtk_entry_new();
	gtk_widget_set_size_request(entryFileNameProp, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryFileNameProp),"prop.txt");
	gtk_table_attach(GTK_TABLE(table),entryFileNameProp, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 18;
	j = 0;
	label = gtk_label_new("Folder");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 18;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 18;
	j = 2;
	buttonDirSelector =  gabedit_dir_button();
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
}
/********************************************************************************/
static void setSpinMultiplicityComboSpinMultiplicity(GtkWidget *comboSpinMultiplicity, gint spin)
{
	GtkWidget *entry = NULL;
	gchar* t = NULL;
	if(!comboSpinMultiplicity) return;
	entry = GTK_BIN (comboSpinMultiplicity)->child;
	t = g_strdup_printf("%d",spin);
	gtk_entry_set_text(GTK_ENTRY(entry),t);
	g_free(t);
}
/**********************************************************************/
static void setComboSpinMultiplicity(GtkWidget *comboSpinMultiplicity)
{
	GList *glist = NULL;
	gint i;
	gint nlist = 0;
	gchar** list = NULL;
	gint k;
	gint kinc;
	gint ne = get_number_electrons(0) - totalCharge;

	if(ne%2==0) nlist = ne/2+1;
	else nlist = (ne+1)/2;

	if(nlist<1) return;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	if(GTK_IS_WIDGET(comboSpinMultiplicity)) gtk_widget_set_sensitive(comboSpinMultiplicity, TRUE);
	if(ne%2==0) k = 1;
	else k = 2;

	kinc = 2;
	for(i=0;i<nlist;i++)
	{
		sprintf(list[i],"%d",k);
		k+=kinc;
	}

  	for(i=nlist-1;i>=0;i--) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboSpinMultiplicity, glist) ;
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboSpinMultiplicity), nlist-1);
  	g_list_free(glist);
	if( SpinMultiplicities[0]%2 == atoi(list[0])%2) setSpinMultiplicityComboSpinMultiplicity(comboSpinMultiplicity, SpinMultiplicities[0]);
	else SpinMultiplicities[0] = atoi(list[0]);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
}
/********************************************************************************/
static void setChargeComboCharge(GtkWidget *comboCharge, gint charge)
{
	GtkWidget *entry = NULL;
	gchar* t = NULL;
	if(!comboCharge) return;
	entry = GTK_BIN (comboCharge)->child;
	t = g_strdup_printf("%d",charge);
	gtk_entry_set_text(GTK_ENTRY(entry),t);
	g_free(t);
}
/********************************************************************************/
static void setComboCharge(GtkWidget *comboCharge)
{
	GList *glist = NULL;
	gint i;
	gint nlist;
	gchar** list = NULL;
	gint k;
	gint c = (gint)(0.5+get_sum_charges());
	gint ic = 0;

	nlist = get_number_electrons(0)*2-2+1;

	if(nlist<1) return;
	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++)
		list[i] = g_malloc(10*sizeof(gchar));


	sprintf(list[0],"0");
	k = 1;
	ic = 0;
	/* printf("c=%d fc = %f\n",c,get_sum_charges());*/
	for(i=1;i<nlist-1;i+=2)
	{
		sprintf(list[i],"+%d",k);
		sprintf(list[i+1],"%d",-k);
		if(c==k) ic = i;
		if(c==-k) ic = i+1;
		k += 1;
	}

  	for(i=nlist-1;i>=0;i--) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboCharge, glist) ;
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboCharge), nlist-ic-1);
	totalCharge = atoi(list[ic]);
  	g_list_free(glist);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
	totalCharge = TotalCharges[0];
	setChargeComboCharge(comboCharge, TotalCharges[0]);
}
/**********************************************************************/
static void changedEntryCharge(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	GtkWidget* comboSpinMultiplicity = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;

	totalCharge = atoi(entryText);

	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entry), "ComboSpinMultiplicity");
	if(GTK_IS_WIDGET(comboSpinMultiplicity)) setComboSpinMultiplicity(comboSpinMultiplicity);


}
/**********************************************************************/
static GtkWidget* addComboListToATable(GtkWidget* table,
		gchar** list, gint nlist, gint i, gint j, gint k)
{
	GtkWidget *entry = NULL;
	GtkWidget *combo = NULL;

	combo = create_combo_box_entry(list, nlist, TRUE, -1, -1);

	gtk_table_attach(GTK_TABLE(table),combo,j,j+k,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	entry = GTK_BIN (combo)->child;
	g_object_set_data(G_OBJECT (entry), "Combo",combo);
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeight*0.2),-1);

	return entry;
}
/**********************************************************************/
static GtkWidget *addChargeOfMoleculeToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryCharge = NULL;
	GtkWidget* comboCharge = NULL;
	gint nlistCharge = 1;
	gchar* listCharge[] = {"0"};

	add_label_table(table,"Charge",(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryCharge = addComboListToATable(table, listCharge, nlistCharge, i, 2, 1);
	comboCharge  = g_object_get_data(G_OBJECT (entryCharge), "Combo");
	gtk_widget_set_sensitive(entryCharge, FALSE);


	return comboCharge;
}
/**********************************************************************/
static GtkWidget *addSpinOfMoleculeToTable(GtkWidget *table, gint i)
{
	GtkWidget* entrySpinMultiplicity = NULL;
	GtkWidget* comboSpinMultiplicity = NULL;
	gint nlistspinMultiplicity = 1;
	gchar* listspinMultiplicity[] = {"0"};

	add_label_table(table,"Spin multiplicity",(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entrySpinMultiplicity = addComboListToATable(table, listspinMultiplicity, nlistspinMultiplicity, i, 2, 1);
	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entrySpinMultiplicity), "Combo");
	gtk_widget_set_sensitive(entrySpinMultiplicity, FALSE);

	return comboSpinMultiplicity;
}
/**********************************************************************/
static void createInfoMDFrame(GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* hboxFrame;
	GtkWidget *label = NULL;

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, 0);

	hboxFrame = gtk_hbox_new (FALSE, 3);
	gtk_widget_show (hboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), hboxFrame);
	label = gtk_label_new(
	"If \"MD Trajectory via Verlet velocity Algorithm\" is selected :\n"
	"        A molecular dynamic simulation is run using the Verlet velocity Algorithm.\n"
	"        During the production step, the user can select the Andersen or Berendsen thermostat which is then applied.\n"
	"        The Berendsen thermostat is applied for Heating, Equilibrium and Cooling steps.\n"
	"\n"
	"If \"MD Trajectory via Modified Beeman Algorithm\" is selected :\n"
	"        A molecular dynamic simulation is run using the Modified Beeman Algorithm.\n"
	"        During the production step, the user can select the Andersen or Berendsen thermostat which is then applied.\n"
	"        The Berendsen thermostat is applied for Heating, Equilibrium and Cooling steps.\n"
	"\n"
	"If \"Stochastic Dynamics via Verlet velocity Algorithm\" is selected :\n"
	"        A stochastic dynamic simulation is run using Verlet velocity Algorithm.\n"
	"        The velocities are scaled (Berendsen method)during the Heating, Equilibrium and Cooling steps.\n"
	);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hboxFrame), label, FALSE, FALSE, 5);
}
/**********************************************************************/
static void createInfoConfoFrame(GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* hboxFrame;
	GtkWidget *label = NULL;

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, FALSE, FALSE, 0);

	hboxFrame = gtk_hbox_new (FALSE, 3);
	gtk_widget_show (hboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), hboxFrame);
	label = gtk_label_new(
	"During the production stage, Gabedit selects a number geometries."
	"\nAt the end of the molecular dynamic simulation, each selected geometry is optimized."
	"\nThe very similar molecular structures can be removed."
	"\nFinally the geometries are sorted by energy and are saved in a file."
	"\nGabedit can also optimize these geometries, interactively, by Mopac or by FireFly."
	"\nGabedit can also creates input files for mopac, FireFly or Gaussian for a post processing."
	"\n\n"
	"If \"MD Trajectory via Verlet velocity Algorithm\" is selected :\n"
	"        A molecular dynamic simulation is run using the Verlet velocity Algorithm.\n"
	"        During the production step, the user can select the Andersen or Berendsen thermostat which is then applied.\n"
	"        The Berendsen thermostat is applied for Heating, Equilibrium and Cooling steps.\n"
	"\n"
	"If \"MD Trajectory via Modified Beeman Algorithm\" is selected :\n"
	"        A molecular dynamic simulation is run using the Modified Beeman Algorithm.\n"
	"        During the production step, the user can select the Andersen or Berendsen thermostat which is then applied.\n"
	"        The Berendsen thermostat is applied for Heating, Equilibrium and Cooling steps.\n"
	"\n"
	"If \"Stochastic Dynamics via Verlet velocity Algorithm\" is selected :\n"
	"        A stochastic dynamic simulation is run using Verlet velocity Algorithm.\n"
	"        The velocities are scaled (Berendsen method)during the Heating, Equilibrium and Cooling steps.\n"
	);
	gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
	gtk_widget_show (label);
	gtk_box_pack_start (GTK_BOX (hboxFrame), label, FALSE, FALSE, 5);
}
/**********************************************************************/
static void createPostProcessingFrame(GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* comboCharge = NULL;
	GtkWidget *table = NULL;
	GtkWidget *label = NULL;
	GtkWidget *sep = NULL;
	gint i;
	gint j;

	totalCharge = TotalCharges[0];
	spinMultiplicity=SpinMultiplicities[0];

	table = gtk_table_new(14,5,FALSE);

	frame = gtk_frame_new ("Post Processing");
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);
	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	label = gtk_label_new("Remove identical molecular structures");
	gtk_table_attach(GTK_TABLE(table),label, j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonTolerance[TOLE] = gtk_check_button_new_with_label("Energy tolerance(KCal/mol)"); 
	gtk_table_attach(GTK_TABLE(table),buttonTolerance[TOLE],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonTolerance[TOLE]), TRUE);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryTolerance[TOLE] = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryTolerance[TOLE]),"1e-2");

	gtk_widget_set_size_request(GTK_WIDGET(entryTolerance[TOLE]),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryTolerance[TOLE], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonTolerance[TOLD] = gtk_check_button_new_with_label("Distance tolerance(Angstrom)"); 
	gtk_table_attach(GTK_TABLE(table),buttonTolerance[TOLD],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonTolerance[TOLD]), TRUE);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryTolerance[TOLD] = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryTolerance[TOLD]),"1e-2");

	gtk_widget_set_size_request(GTK_WIDGET(entryTolerance[TOLD]),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryTolerance[TOLD], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep, j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonPostMMOpt = gtk_check_button_new_with_label("Interactive minimization by MM");
	gtk_table_attach(GTK_TABLE(table),buttonPostMMOpt,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPostMMOpt), TRUE);
	gtk_widget_show (buttonPostMMOpt);
	i++;
/*----------------------------------------------------------------------------------*/
	j = 0;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep, j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonPostNone =gtk_radio_button_new_with_label( NULL,"None inteactive post processing"); 
	gtk_table_attach(GTK_TABLE(table),buttonPostNone,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPostNone), TRUE);
	gtk_widget_show (buttonPostNone);

/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonPostPM6Mopac = gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonPostNone)),
			"Interactive minimization by Mopac(PM6)");
	gtk_table_attach(GTK_TABLE(table),buttonPostPM6Mopac,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPostPM6Mopac), FALSE);
	gtk_widget_show (buttonPostPM6Mopac);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonPostAM1Mopac = gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonPostNone)),
			"Interactive minimization by Mopac(AM1)");
	gtk_table_attach(GTK_TABLE(table),buttonPostAM1Mopac,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPostAM1Mopac), FALSE);
	gtk_widget_show (buttonPostAM1Mopac);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonPostFireFly = gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonPostNone)),
			"Interactive minimization by FireFly(AM1)");
	gtk_table_attach(GTK_TABLE(table),buttonPostFireFly,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPostFireFly), FALSE);
	gtk_widget_show (buttonPostFireFly);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	sep = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),sep, j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonCreateMopac = gtk_check_button_new_with_label("Create Mopac files.          Keywords "); 
	gtk_table_attach(GTK_TABLE(table),buttonCreateMopac,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCreateMopac), FALSE);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryMopacKeywords = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryMopacKeywords),"PM6 XYZ AUX");
	gtk_table_attach(GTK_TABLE(table),entryMopacKeywords, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonCreateGaussian = gtk_check_button_new_with_label("Create Gaussian files.     Keywords "); 
	gtk_table_attach(GTK_TABLE(table),buttonCreateGaussian,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCreateGaussian), FALSE);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryGaussianKeywords = gtk_entry_new();
	gtk_widget_set_size_request(entryGaussianKeywords, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryGaussianKeywords),"B3LYP/6-31G* Opt");
	gtk_table_attach(GTK_TABLE(table),entryGaussianKeywords, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonCreateFireFly = gtk_check_button_new_with_label("Create FireFly files.    Keywords "); 
	gtk_table_attach(GTK_TABLE(table),buttonCreateFireFly,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonCreateFireFly), FALSE);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryFireFlyKeywords = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryFireFlyKeywords),"RUNTYP=Optimize    GBASIS=AM1");
	gtk_table_attach(GTK_TABLE(table),entryFireFlyKeywords, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	comboCharge = addChargeOfMoleculeToTable(table, i);
/*----------------------------------------------------------------------------------*/
	i++;
	comboSpinMultiplicity = addSpinOfMoleculeToTable(table, i);
/*----------------------------------------------------------------------------------*/

	if(GTK_IS_COMBO_BOX(comboCharge))
		g_object_set_data(G_OBJECT (GTK_BIN(comboCharge)->child), "ComboSpinMultiplicity", comboSpinMultiplicity);
	setComboCharge(comboCharge);
	setComboSpinMultiplicity(comboSpinMultiplicity);
	entryCharge = GTK_WIDGET(GTK_BIN(comboCharge)->child);
	entrySpinMultiplicity = GTK_WIDGET(GTK_BIN(comboSpinMultiplicity)->child);
	g_signal_connect(G_OBJECT(entryCharge),"changed", G_CALLBACK(changedEntryCharge),NULL);
}
/********************************************************************************/
static void AddInfoConfo(GtkWidget *NoteBook, GtkWidget *win)
{
	GtkWidget *frame;
	GtkWidget *LabelOnglet;
	GtkWidget *LabelMenu;
	GtkWidget *vbox;

	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

	LabelOnglet = gtk_label_new("Info");
	LabelMenu = gtk_label_new("Info");
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook), frame,LabelOnglet, LabelMenu);
	g_object_set_data (G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	createInfoConfoFrame(vbox);
}
/********************************************************************************/
static void AddInfoMD(GtkWidget *NoteBook, GtkWidget *win)
{
	GtkWidget *frame;
	GtkWidget *LabelOnglet;
	GtkWidget *LabelMenu;
	GtkWidget *vbox;

	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

	LabelOnglet = gtk_label_new("Info");
	LabelMenu = gtk_label_new("Info");
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook), frame,LabelOnglet, LabelMenu);
	g_object_set_data (G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);
	createInfoMDFrame(vbox);
}
/********************************************************************************/
static void AddGeneralConfoOptionsDlg(GtkWidget *NoteBook, GtkWidget *win)
{
	gint i;
	gint j;
	GtkWidget *frame;
	GtkWidget *LabelOnglet;
	GtkWidget *LabelMenu;
	GtkWidget *vbox;
	GtkWidget *table;
	GtkWidget *label;

	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

	LabelOnglet = gtk_label_new("General");
	LabelMenu = gtk_label_new("General");
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook), frame,LabelOnglet, LabelMenu);
	g_object_set_data (G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	table = gtk_table_new(8,5,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	add_label_table(table,"Number of selected geometries",(gushort)i,(gushort)j);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 2;
	entryNumberOfGeom = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryNumberOfGeom),"10");

	gtk_widget_set_size_request(GTK_WIDGET(entryNumberOfGeom),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryNumberOfGeom, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 0;
	add_label_table(table,"Geometries saved in File",(gushort)i,(gushort)j);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 2;
	entryFileNameGeom = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryFileNameGeom),"conf.gab");
	gtk_widget_set_size_request(GTK_WIDGET(entryFileNameGeom),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryFileNameGeom, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 0;
	add_label_table(table,"Folder",(gushort)i,(gushort)j);
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 2;
	buttonDirSelector =  gabedit_dir_button();
	gtk_widget_set_size_request(GTK_WIDGET(buttonDirSelector),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 3;
	j = 0;
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_table_attach(GTK_TABLE(table),vbox,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	createPostProcessingFrame(vbox);
}
/********************************************************************************/
static void AddDynamicsConfoOptionsDlg(GtkWidget *NoteBook, GtkWidget *win)
{

	gint i;
	gint j;
	GtkWidget *frame;
	GtkWidget *LabelOnglet;
	GtkWidget *LabelMenu;
	GtkWidget *vbox;
	GtkWidget *table;
	GtkWidget* label;
	GtkWidget *hseparator = NULL;

	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

	LabelOnglet = gtk_label_new("Molecular Dynamics");
	LabelMenu = gtk_label_new("Molecular Dynamics");
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook), frame,LabelOnglet, LabelMenu);
	g_object_set_data (G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	table = gtk_table_new(20,6,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 2;
	label = gtk_label_new("Heating");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 3;
	label = gtk_label_new("Equilibrium");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 4;
	label = gtk_label_new("Production");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 0;
	label = gtk_label_new("Time (ps)");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 2;
	entryMDTimes[0] = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryMDTimes[0]),"0.0");
	gtk_widget_set_size_request(entryMDTimes[0], 60, -1);
	gtk_table_attach(GTK_TABLE(table),entryMDTimes[0], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 3;
	entryMDTimes[1] = gtk_entry_new();
	gtk_widget_set_size_request(entryMDTimes[1], 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDTimes[1]),"0.0");
	gtk_table_attach(GTK_TABLE(table),entryMDTimes[1], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 4;
	entryMDTimes[2] = gtk_entry_new();
	gtk_widget_set_size_request(entryMDTimes[2], 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDTimes[2]),"10.0");
	gtk_table_attach(GTK_TABLE(table),entryMDTimes[2], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 0;
	label = gtk_label_new("Temperature (K)");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 2;
	entryMDTemperature[0] = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryMDTemperature[0]),"0.0");
	gtk_widget_set_size_request(entryMDTemperature[0], 60, -1);
	gtk_table_attach(GTK_TABLE(table),entryMDTemperature[0], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	/*
	i = 2;
	j = 3;
	entryMDTemperature[1] = gtk_entry_new();
	gtk_widget_set_size_request(entryMDTemperature[1], 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDTemperature[1]),"1000.0");
	gtk_table_attach(GTK_TABLE(table),entryMDTemperature[1], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
		  */
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 4;
	entryMDTemperature[2] = gtk_entry_new();
	gtk_widget_set_size_request(entryMDTemperature[2], 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDTemperature[2]),"1000.0");
	gtk_table_attach(GTK_TABLE(table),entryMDTemperature[2], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 3;
	j = 0;
	label = gtk_label_new("Step size (fs)");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 3;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 3;
	j = 2;
	entryMDStepSize = gtk_entry_new();
	gtk_widget_set_size_request(entryMDStepSize, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDStepSize),"1.0");
	gtk_table_attach(GTK_TABLE(table),entryMDStepSize, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 0;
	label = gtk_label_new("Screen Rafresh after");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 2;
	entryMDRafresh = gtk_entry_new();
	gtk_widget_set_size_request(entryMDRafresh, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryMDRafresh),"5");
	gtk_table_attach(GTK_TABLE(table),entryMDRafresh, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 3;
	label = gtk_label_new("steps");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 5;
	j = 0;
	label = gtk_label_new("Collide(ps^-1)");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 5;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 5;
	j = 2;
	entrySDCollide = gtk_entry_new();
	gtk_widget_set_size_request(entrySDCollide, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entrySDCollide),"20.0");
	gtk_table_attach(GTK_TABLE(table),entrySDCollide, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 6;
	j = 0;
	label = gtk_label_new("Friction(ps^-1)");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 6;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 6;
	j = 2;
	entrySDFriction = gtk_entry_new();
	gtk_widget_set_size_request(entrySDFriction, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entrySDFriction),"40.0");
	gtk_table_attach(GTK_TABLE(table),entrySDFriction, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	/* THERMOSTAT */
/*----------------------------------------------------------------------------------*/
	i = 3;
	j = 4;
	buttonMDThermOptions[NONE]= gtk_radio_button_new_with_label( NULL, "NVE"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDThermOptions[NONE],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDThermOptions[NONE]), TRUE);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 4;
	buttonMDThermOptions[BERENDSEN]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDThermOptions[NONE])), "Berendsen"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDThermOptions[BERENDSEN],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDThermOptions[BERENDSEN]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 5;
	j = 4;
	buttonMDThermOptions[ANDERSEN]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDThermOptions[NONE])), "Andersen"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDThermOptions[ANDERSEN],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDThermOptions[ANDERSEN]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 6;
	j = 4;
	buttonMDThermOptions[BUSSI]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDThermOptions[NONE])), "Bussi"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDThermOptions[BUSSI],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDThermOptions[BUSSI]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 7;
	j = 0;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),hseparator, j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 8;
	j = 0;
	buttonMDOptions[VERLET]= gtk_radio_button_new_with_label( NULL, "MD Trajectory via Verlet velocity Algorithm"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDOptions[VERLET],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDOptions[VERLET]), TRUE);
/*----------------------------------------------------------------------------------*/
	i = 9;
	j = 0;
	buttonMDOptions[BEEMAN]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDOptions[VERLET])), "MD Trajectory via Modified Beeman Algorithm"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDOptions[BEEMAN],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDOptions[BEEMAN]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 10;
	j = 0;
	buttonMDOptions[STOCHASTIC]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDOptions[VERLET])), "Stochastic Dynamics via Verlet velocity Algorithm"); 
	gtk_table_attach(GTK_TABLE(table),buttonMDOptions[STOCHASTIC],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDOptions[STOCHASTIC]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 11;
	j = 0;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),hseparator, j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 12;
	j = 0;
	buttonConstraintsOptions[NOCONSTRAINTS]= gtk_radio_button_new_with_label( NULL, "No constraints"); 
	gtk_table_attach(GTK_TABLE(table),buttonConstraintsOptions[NOCONSTRAINTS],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonConstraintsOptions[NOCONSTRAINTS]), TRUE);
/*----------------------------------------------------------------------------------*/
	i = 13;
	j = 0;
	buttonConstraintsOptions[BONDSCONSTRAINTS]= gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonConstraintsOptions[NOCONSTRAINTS])), "Bond constraints"); 
	gtk_table_attach(GTK_TABLE(table),buttonConstraintsOptions[BONDSCONSTRAINTS],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonConstraintsOptions[BONDSCONSTRAINTS]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 14;
	j = 0;
	buttonConstraintsOptions[BONDSANGLESCONSTRAINTS]= gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonConstraintsOptions[NOCONSTRAINTS])), "Bond & Angle constraints"); 
	gtk_table_attach(GTK_TABLE(table),buttonConstraintsOptions[BONDSANGLESCONSTRAINTS],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonConstraintsOptions[BONDSANGLESCONSTRAINTS]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 15;
	j = 0;
	hseparator = gtk_hseparator_new ();
	gtk_table_attach(GTK_TABLE(table),hseparator, j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 16;
	j = 0;
	buttonSaveTraj = gtk_check_button_new_with_label("Save Trajectory in "); 
	gtk_table_attach(GTK_TABLE(table),buttonSaveTraj,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonSaveTraj), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 16;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 16;
	j = 2;
	entryFileNameTraj = gtk_entry_new();
	gtk_widget_set_size_request(entryFileNameTraj, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryFileNameTraj),"traj.gab");
	gtk_table_attach(GTK_TABLE(table),entryFileNameTraj, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 17;
	j = 0;
	buttonSaveProp = gtk_check_button_new_with_label("Save Properties in "); 
	gtk_table_attach(GTK_TABLE(table),buttonSaveProp,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonSaveProp), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 17;
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 17;
	j = 2;
	entryFileNameProp = gtk_entry_new();
	gtk_widget_set_size_request(entryFileNameProp, 60, -1);
	gtk_entry_set_text(GTK_ENTRY(entryFileNameProp),"prop.txt");
	gtk_table_attach(GTK_TABLE(table),entryFileNameProp, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
}
/*****************************************************************************/
static void amberMinimize(GtkWidget* Win, gpointer data)
{
	ForceField forceField; 
	ForceFieldOptions forceFieldOptions;
	SteepestDescent steepestDescent;
	ConjugateGradient conjugateGradient;
	QuasiNewton quasiNewton;
	gboolean useConjugateGradient;
	gboolean useQuasiNewton;
	ConjugateGradientOptions conjugateGradientOptions;

	conjugateGradientOptions.gradientNorm = 1e-3;
	conjugateGradientOptions.maxIterations = 100;
	conjugateGradientOptions.updateFrequency = 1;
	conjugateGradientOptions.maxLines = 25;
	conjugateGradientOptions.initialStep = 0.001;
	conjugateGradientOptions.method = 1;

	forceFieldOptions.type = AMBER;
	forceFieldOptions.bondStretch = GTK_TOGGLE_BUTTON (buttonMMOptions[MMBOND])->active;
	forceFieldOptions.angleBend = GTK_TOGGLE_BUTTON (buttonMMOptions[MMBEND])->active;
	forceFieldOptions.dihedralAngle = GTK_TOGGLE_BUTTON (buttonMMOptions[MMTORSION])->active;
	forceFieldOptions.improperTorsion = GTK_TOGGLE_BUTTON (buttonMMOptions[MMIMPROPER])->active;
	forceFieldOptions.nonBonded = GTK_TOGGLE_BUTTON (buttonMMOptions[MMNONBOND])->active;
	forceFieldOptions.hydrogenBonded = GTK_TOGGLE_BUTTON (buttonMMOptions[MMHBOND])->active;
	forceFieldOptions.coulomb = GTK_TOGGLE_BUTTON (buttonMMOptions[MMCOULOMB])->active;
	forceFieldOptions.vanderWals = GTK_TOGGLE_BUTTON (buttonMMOptions[PWVANDERWALS])->active;
	forceFieldOptions.rattleConstraints = NOCONSTRAINTS;

	useConjugateGradient = GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADCONJUGATE])->active;
	useQuasiNewton = GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADQUASINEWTON])->active;

	conjugateGradientOptions.gradientNorm  = 
		atof(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADEPSILON])));
	conjugateGradientOptions.maxIterations = 
		atoi(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADMAXITERATIONS])));
	conjugateGradientOptions.updateFrequency = 
		atoi(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADFREQUENCY])));
	conjugateGradientOptions.initialStep = 
		atof(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADINITIALSTEP])));
	conjugateGradientOptions.maxLines = 
		atoi(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADMAXLINES])));

	quasiNewton.maxIterations = 
		atoi(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADMAXITERATIONS])));
	quasiNewton.updateFrequency = 
		atoi(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADFREQUENCY])));
	quasiNewton.epsilon  = 
		atof(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADEPSILON])));
	quasiNewton.tolerence = 1e-16;  
	quasiNewton.maxLines = 
		atoi(gtk_entry_get_text(GTK_ENTRY(entryMinimizeOptions[GRADMAXLINES])));

	if(GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADHESTENES])->active)
		conjugateGradientOptions.method = 1;
	else
	if(GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADFLETCHER])->active)
		conjugateGradientOptions.method = 2;
	else
	if(GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADPOLAK])->active)
		conjugateGradientOptions.method = 3;
	else
	if(GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADWOLF])->active)
		conjugateGradientOptions.method = 4;

	if(GTK_TOGGLE_BUTTON (buttonTypesOptions[AMBER])->active )
		forceFieldOptions.type = AMBER;
	else
	if(GTK_TOGGLE_BUTTON (buttonTypesOptions[PAIRWISE])->active )
	{
		forceFieldOptions.coulomb = GTK_TOGGLE_BUTTON (buttonMMOptions[PWCOULOMB])->active;
		forceFieldOptions.type = PAIRWISE;
	}
	{
		gint i;
		for(i=0;i<4;i++) NumSelAtoms[i] = -1;
		change_of_center(NULL,NULL);
	}

	gtk_widget_destroy(Win);
    	while( gtk_events_pending() )
        	gtk_main_iteration();

	set_sensitive_stop_button( TRUE);
	StopCalcul = FALSE;

	if(forceFieldOptions.type==AMBER)
		forceField = createAmberModel(geometry0,Natoms,forceFieldOptions);
	else
	if(forceFieldOptions.type==PAIRWISE)
		forceField = createPairWiseModel(geometry0,Natoms,forceFieldOptions);

	if(StopCalcul)
	{
		set_text_to_draw(" ");
		set_statubar_operation_str(_("Calculation canceled"));
		drawGeom();
		set_sensitive_stop_button( FALSE);
		return;
	}

	if(useConjugateGradient)
	{
		set_sensitive_stop_button( TRUE);
		runConjugateGradient(&conjugateGradient, &forceField,
				conjugateGradientOptions); 
		if(StopCalcul)
		{
			set_text_to_draw(" ");
			set_statubar_operation_str(_("Calculation canceled"));
			drawGeom();
		}
		set_sensitive_stop_button( FALSE);
		freeConjugateGradient(&conjugateGradient);
	}
	else if(useQuasiNewton)
	{
		set_sensitive_stop_button( TRUE);
		quasiNewton.forceField = &forceField; 
                runQuasiNewton(&quasiNewton);
		if(StopCalcul)
		{
			set_text_to_draw(" ");
			set_statubar_operation_str(_("Calculation canceled"));
			drawGeom();
		}
		set_sensitive_stop_button( FALSE);
		freeQuasiNewton(&quasiNewton);

	}
	else
	{
		set_sensitive_stop_button( TRUE);
		runSteepestDescent(&steepestDescent, &forceField,
			       	conjugateGradientOptions.updateFrequency,
			       conjugateGradientOptions.maxIterations,
			       conjugateGradientOptions.gradientNorm,
			       conjugateGradientOptions.maxLines);
		if(StopCalcul)
		{
			set_text_to_draw(" ");
			set_statubar_operation_str(_("Calculation canceled"));
			drawGeom();
		}
		set_sensitive_stop_button( FALSE);
		freeSteepestDescent(&steepestDescent);
	}
	freeForceField(&forceField);
	set_text_to_draw(" ");
}
/*****************************************************************************/
void amberEnergyCalculation(GtkWidget* Win, gpointer data)
{
	gchar* str;
	ForceField forceField;
	ForceFieldOptions forceFieldOptions;
	gdouble gradientNorm = 0;
	gint i,j;

	forceFieldOptions.type = AMBER;
	forceFieldOptions.bondStretch = GTK_TOGGLE_BUTTON (buttonMMOptions[MMBOND])->active;
	forceFieldOptions.angleBend = GTK_TOGGLE_BUTTON (buttonMMOptions[MMBEND])->active;
	forceFieldOptions.dihedralAngle = GTK_TOGGLE_BUTTON (buttonMMOptions[MMTORSION])->active;
	forceFieldOptions.improperTorsion = GTK_TOGGLE_BUTTON (buttonMMOptions[MMIMPROPER])->active;
	forceFieldOptions.nonBonded = GTK_TOGGLE_BUTTON (buttonMMOptions[MMNONBOND])->active;
	forceFieldOptions.hydrogenBonded = GTK_TOGGLE_BUTTON (buttonMMOptions[MMHBOND])->active;
	forceFieldOptions.coulomb = GTK_TOGGLE_BUTTON (buttonMMOptions[MMCOULOMB])->active;
	forceFieldOptions.vanderWals = GTK_TOGGLE_BUTTON (buttonMMOptions[PWVANDERWALS])->active;
	forceFieldOptions.rattleConstraints = NOCONSTRAINTS;

	if(GTK_TOGGLE_BUTTON (buttonTypesOptions[AMBER])->active )
		forceFieldOptions.type = AMBER;
	else
	if(GTK_TOGGLE_BUTTON (buttonTypesOptions[PAIRWISE])->active )
	{
		forceFieldOptions.coulomb = GTK_TOGGLE_BUTTON (buttonMMOptions[PWCOULOMB])->active;
		forceFieldOptions.type = PAIRWISE;
	}

	gtk_widget_destroy(Win);
    	while( gtk_events_pending() )
        	gtk_main_iteration();

	set_sensitive_stop_button( TRUE);
	StopCalcul = FALSE;
	if(forceFieldOptions.type==AMBER)
		forceField = createAmberModel(geometry0,Natoms,forceFieldOptions);
	else
	if(forceFieldOptions.type==PAIRWISE)
		forceField = createPairWiseModel(geometry0,Natoms,forceFieldOptions);

	if(StopCalcul)
	{
		set_text_to_draw(" ");
		set_statubar_operation_str(_("Calculation canceled"));
		drawGeom();
		set_sensitive_stop_button( FALSE);
		return;
	}
	forceField.klass->calculateEnergy(&forceField);
	forceField.klass->calculateGradient(&forceField);

	gradientNorm = 0;
	for (  i = 0; i < forceField.molecule.nAtoms; i++ )
		for(j=0;j<3;j++)
			gradientNorm += 
			forceField.molecule.gradient[j][i]
			*forceField.molecule.gradient[j][i]; 

	str = g_strdup_printf(_("Gradient Norm  = %f energy = %f(KCal/mol)"),
		sqrt(gradientNorm),forceField.molecule.energy); 

	set_text_to_draw(str);
	set_statubar_operation_str(str);
	drawGeom();
	set_sensitive_stop_button( FALSE);
	freeForceField(&forceField);
	set_text_to_draw(" ");
	g_free(str);
}
/***********************************************************************/
void sensitive_conjugate_gradient_buttons(GtkWidget *button, gpointer data)
{
	gboolean useConjugateGradient;
	gboolean useQuasiNewton;
	gboolean Ok = FALSE;
	if(data != NULL)
		Ok = TRUE;

	useQuasiNewton = GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADQUASINEWTON])->active;
	useConjugateGradient = GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADCONJUGATE])->active;
	if(useConjugateGradient)
	{
		Ok = TRUE;
		gtk_widget_set_sensitive(buttonMinimizeOptions[GRADHESTENES], Ok);
		gtk_widget_set_sensitive(buttonMinimizeOptions[GRADFLETCHER], Ok);
		gtk_widget_set_sensitive(buttonMinimizeOptions[GRADPOLAK], Ok);
		gtk_widget_set_sensitive(buttonMinimizeOptions[GRADWOLF], Ok);

		gtk_widget_set_sensitive(entryMinimizeOptions[GRADMAXLINES], Ok);
		gtk_widget_set_sensitive(entryMinimizeOptions[GRADINITIALSTEP], Ok);
		gtk_entry_set_text(GTK_ENTRY(entryMinimizeOptions[GRADMAXLINES]),"25");
	}
	else
	if(useQuasiNewton)
	{
		Ok = FALSE;
		gtk_widget_set_sensitive(buttonMinimizeOptions[GRADHESTENES], Ok);
		gtk_widget_set_sensitive(buttonMinimizeOptions[GRADFLETCHER], Ok);
		gtk_widget_set_sensitive(buttonMinimizeOptions[GRADPOLAK], Ok);
		gtk_widget_set_sensitive(buttonMinimizeOptions[GRADWOLF], Ok);

		gtk_widget_set_sensitive(entryMinimizeOptions[GRADMAXLINES], TRUE);
		gtk_widget_set_sensitive(entryMinimizeOptions[GRADINITIALSTEP], Ok);
		gtk_entry_set_text(GTK_ENTRY(entryMinimizeOptions[GRADMAXLINES]),"25");
	}
	else
	{
		Ok = FALSE;
		gtk_widget_set_sensitive(buttonMinimizeOptions[GRADHESTENES], Ok);
		gtk_widget_set_sensitive(buttonMinimizeOptions[GRADFLETCHER], Ok);
		gtk_widget_set_sensitive(buttonMinimizeOptions[GRADPOLAK], Ok);
		gtk_widget_set_sensitive(buttonMinimizeOptions[GRADWOLF], Ok);

		gtk_widget_set_sensitive(entryMinimizeOptions[GRADMAXLINES], TRUE);
		gtk_widget_set_sensitive(entryMinimizeOptions[GRADINITIALSTEP], Ok);
		gtk_entry_set_text(GTK_ENTRY(entryMinimizeOptions[GRADMAXLINES]),"400");
	}
}
/***********************************************************************/
static void AddGradientOptionsDlg(GtkWidget *NoteBook)
{

	gint i;
	gint j;
	gint ifirst;
	GtkWidget *frame;
	GtkWidget *frame1;
	GtkWidget *LabelOnglet;
	GtkWidget *LabelMenu;
	GtkWidget *vbox;
	GtkWidget *vbox1;
	GtkWidget *hbox;
	GtkWidget *table;
	GtkWidget* label;
	GtkWidget *vseparator = gtk_vseparator_new ();
	gchar *options[NGRADOPTIONS]={
  		"Use Quasi Newton ",
  		"Use Steepest Descent ",
  		"Use Comgugate Gradient ",
  		"Hestenes-Stiefel Method",
  		"Fletcher-Reeves Method",
  		"Polak-Ribiere Method",
  		"Wolf-Powell Method"
  		};
	gchar *entrylabels[NGRADENTRYS]={
  		"Maximum Iterations",
  		"Epsilon",
  		"Maximum Line Searches",
  		"Step Size",
  		"Screen Rafresh after"
  		};
	gchar *entryvalues[NGRADENTRYS]={
  		"10000",
  		"0.0001",
  		"25",
  		"0.01",
  		"1"
  		};

	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

	LabelOnglet = gtk_label_new(_("Gradient"));
	LabelMenu = gtk_label_new(_("Gradient"));
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
			  frame,LabelOnglet, LabelMenu);
	g_object_set_data (G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	frame1 = gtk_frame_new (NULL);
	gtk_widget_show (frame1);
	gtk_container_add (GTK_CONTAINER (vbox), frame1);
	gtk_frame_set_label_align (GTK_FRAME (frame1), 0.5, 0.5);

	vbox1 = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame1), vbox1);

	table = gtk_table_new(8,8,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox1), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	buttonMinimizeOptions[GRADQUASINEWTON]= gtk_radio_button_new_with_label(
			NULL,
                       options[GRADQUASINEWTON]); 
	gtk_table_attach(GTK_TABLE(table),buttonMinimizeOptions[GRADQUASINEWTON],
			j,j+2,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADQUASINEWTON]), TRUE);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 0;
	buttonMinimizeOptions[GRADSTEEPEST]= gtk_radio_button_new_with_label(
		gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMinimizeOptions[GRADQUASINEWTON])),
                options[GRADSTEEPEST]); 
	gtk_table_attach(GTK_TABLE(table),buttonMinimizeOptions[GRADSTEEPEST],
			j,j+2,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADSTEEPEST]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 0;
	buttonMinimizeOptions[GRADCONJUGATE] = gtk_radio_button_new_with_label(
                       gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMinimizeOptions[GRADSTEEPEST])),
			options[GRADCONJUGATE]);
	gtk_table_attach(GTK_TABLE(table),buttonMinimizeOptions[GRADCONJUGATE],
			j,j+2,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADCONJUGATE]), TRUE);
/*----------------------------------------------------------------------------------*/
	i = 3;
	j = 0;
	label = gtk_label_new("    ");
	gtk_table_attach(GTK_TABLE(table),label,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 3;
	j = 1;
	buttonMinimizeOptions[GRADHESTENES] = gtk_radio_button_new_with_label(NULL,options[GRADHESTENES]);
	gtk_table_attach(GTK_TABLE(table),buttonMinimizeOptions[GRADHESTENES],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL) ,
                  (GtkAttachOptions)(GTK_SHRINK),
                  1,1);

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADHESTENES]), TRUE);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 1;
	buttonMinimizeOptions[GRADFLETCHER] = gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMinimizeOptions[GRADHESTENES])),
			options[GRADFLETCHER]);
	gtk_table_attach(GTK_TABLE(table),buttonMinimizeOptions[GRADFLETCHER],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL) ,
                  (GtkAttachOptions)(GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADFLETCHER]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 5;
	j = 1;
	buttonMinimizeOptions[GRADPOLAK] = gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMinimizeOptions[GRADHESTENES])),
			options[GRADPOLAK]);
	gtk_table_attach(GTK_TABLE(table),buttonMinimizeOptions[GRADPOLAK],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADPOLAK]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 6;
	j = 1;
	buttonMinimizeOptions[GRADWOLF] = gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMinimizeOptions[GRADHESTENES])),
			options[GRADWOLF]);
	gtk_table_attach(GTK_TABLE(table),buttonMinimizeOptions[GRADWOLF],
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADWOLF]), FALSE);


/* Entries */
	j = 3;
	i = 2;
	gtk_table_attach(GTK_TABLE(table),vseparator,
		j,j+1,i,i+5,
                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                1,1);

	ifirst = 2;

	for(i=0;i<NGRADENTRYS;i++)
	{
		j = 4;
		label = gtk_label_new(entrylabels[i]);
		gtk_label_set_justify(GTK_LABEL(label),GTK_JUSTIFY_LEFT);
		hbox = gtk_hbox_new (FALSE, 5);
		gtk_table_attach(GTK_TABLE(table),hbox,
			j,j+1,i+ifirst,i+ifirst+1,
                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                  1,1);
		gtk_widget_show (hbox);
  		gtk_box_pack_start (GTK_BOX(hbox), label, FALSE, FALSE, 5);

		j = 5;
		label = gtk_label_new(" : ");
		gtk_table_attach(GTK_TABLE(table),label,
			j,j+1,i+ifirst,i+ifirst+1,
                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                  1,1);

		j = 6;
		entryMinimizeOptions[i] = gtk_entry_new();
  		gtk_widget_set_size_request(entryMinimizeOptions[i], 60, -1);
		gtk_table_attach(GTK_TABLE(table),entryMinimizeOptions[i],
			j,j+1,i+ifirst,i+ifirst+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
		gtk_entry_set_text(GTK_ENTRY(entryMinimizeOptions[i]),entryvalues[i]);
	}
	j = 8;
	label = gtk_label_new(" steps ");
	gtk_table_attach(GTK_TABLE(table),label,
		j,j+1,GRADFREQUENCY+1,GRADFREQUENCY+2,
                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
                1,1);


	g_signal_connect(G_OBJECT(buttonMinimizeOptions[GRADCONJUGATE]),
			"clicked",
			(GCallback)sensitive_conjugate_gradient_buttons,
			(gpointer)(buttonMinimizeOptions[GRADCONJUGATE]));
	g_signal_connect(G_OBJECT(buttonMinimizeOptions[GRADSTEEPEST]),
			"clicked",
			(GCallback)sensitive_conjugate_gradient_buttons,
			NULL);
	g_signal_connect(G_OBJECT(buttonMinimizeOptions[GRADQUASINEWTON]),
			"clicked",
			(GCallback)sensitive_conjugate_gradient_buttons,
			NULL);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMinimizeOptions[GRADQUASINEWTON]), TRUE);
}
/***********************************************************************/
static void sensitiveFrames(GtkWidget *button, gpointer data)
{
	if(button == buttonTypesOptions[AMBER])
	{
		gtk_widget_set_sensitive(frameAmber, TRUE);
		gtk_widget_set_sensitive(framePairWise, FALSE);
	}
	else
	{
		gtk_widget_set_sensitive(frameAmber, FALSE);
		gtk_widget_set_sensitive(framePairWise, TRUE);
	}
}
/***********************************************************************/
static void AddMMOptionsDlg(GtkWidget *NoteBook)
{

	gint i;
	GtkWidget *frame;
	GtkWidget *frame1;
	GtkWidget *frame2;
	GtkWidget *LabelOnglet;
	GtkWidget *LabelMenu;
	GtkWidget *vbox;
	GtkWidget *vbox1;
	GtkWidget *vbox2;
	GtkWidget *hbox;
	gchar *firstColonn[NOPTIONS1]={
  		"Use bond stretch terms ",
  		"Use angle bend terms ",
  		"Use torsion terms ",
  		"Use improper torsion terms "
  		};
	gchar *secondColonn[NOPTIONS2]={
  		"Use Non bonded terms ",
		"Use H-Bonded terms ",
		"Use electrostatic charge-charge terms "
  		};
	gchar *pairWise[NOPTIONS3]={
		"Use Vander Wals attactive terms ",
		"Use electrostatic charge-charge terms "
  		};

	frame = gtk_frame_new(NULL);
	gtk_container_set_border_width(GTK_CONTAINER(frame), 5);

	LabelOnglet = gtk_label_new("MM Options");
	LabelMenu = gtk_label_new("MM Options");
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook),
			  frame,LabelOnglet, LabelMenu);
	g_object_set_data (G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 2);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	buttonTypesOptions[AMBER] =gtk_radio_button_new_with_label( NULL,"Amber model"); 
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonTypesOptions[AMBER]), TRUE);
	gtk_box_pack_start (GTK_BOX (vbox), buttonTypesOptions[AMBER], TRUE, TRUE, 0);
	gtk_widget_show (buttonTypesOptions[AMBER]);

	frame1 = gtk_frame_new (NULL);
	gtk_widget_show (frame1);
	gtk_container_add (GTK_CONTAINER (vbox), frame1);
	gtk_frame_set_label_align (GTK_FRAME (frame1), 0.5, 0.5);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_widget_show (hbox);
	gtk_container_add (GTK_CONTAINER (frame1), hbox);

	vbox1 = gtk_vbox_new (FALSE, 0);
	g_object_ref (vbox1);
	gtk_box_pack_start (GTK_BOX (hbox), vbox1, TRUE, TRUE, 0);

	for(i=0;i<NOPTIONS1;i++)
	{
		buttonMMOptions[i] = create_checkbutton(frame1,vbox1,firstColonn[i]);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMMOptions[i]), TRUE);
	}

	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMMOptions[MMIMPROPER]), FALSE);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);

	for(i=0;i<NOPTIONS2;i++)
	{
  		buttonMMOptions[NOPTIONS1+i] = create_checkbutton(frame1,vbox2,secondColonn[i]);
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMMOptions[NOPTIONS1+i]), FALSE);
	}
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMMOptions[MMNONBOND]), TRUE);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMMOptions[MMCOULOMB]), TRUE);

	buttonTypesOptions[PAIRWISE] = gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonTypesOptions[AMBER])),
			"Pair Wise approximation of energy");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonTypesOptions[PAIRWISE]), FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), buttonTypesOptions[PAIRWISE], TRUE, TRUE, 0);
	gtk_widget_show (buttonTypesOptions[PAIRWISE]);

	frame2 = gtk_frame_new (NULL);
	gtk_widget_show (frame2);
	gtk_container_add (GTK_CONTAINER (vbox), frame2);
	gtk_frame_set_label_align (GTK_FRAME (frame2), 0.5, 0.5);

	vbox2 = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox2);
	gtk_container_add (GTK_CONTAINER (frame2), vbox2);

	for(i=0;i<NOPTIONS3;i++)
	{
  		buttonMMOptions[NOPTIONS1+NOPTIONS2+i] = 
			create_checkbutton(frame2,vbox2,pairWise[i]);
		gtk_toggle_button_set_active (
				GTK_TOGGLE_BUTTON (buttonMMOptions[NOPTIONS1+NOPTIONS2+i]), TRUE);
	}
	frameAmber = frame1;
	framePairWise = frame2;
	sensitiveFrames(buttonTypesOptions[AMBER],NULL);

	g_signal_connect(G_OBJECT(buttonTypesOptions[AMBER]),
			"clicked", (GCallback)sensitiveFrames,NULL);
	g_signal_connect(G_OBJECT(buttonTypesOptions[PAIRWISE]),
			"clicked", (GCallback)sensitiveFrames,NULL);


}
/***********************************************************************/
void DestroyWinMMDlg(GtkWidget* Win, gpointer data)
{
	gtk_widget_destroy(GTK_WIDGET(Win));
}
/***********************************************************************/
void MolecularMechanicsEnergyDlg()
{
	GtkWidget *button;
	GtkWidget *Win;
	GtkWidget *NoteBook;
	GtkWidget *parentWindow = GeomDlg;

	StopCalcul = TRUE;

	Win= gtk_dialog_new ();
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_title(&GTK_DIALOG(Win)->window,"Molecular Mechanics Energy");
    	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)DestroyWinMMDlg,NULL);
 
	NoteBook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX (GTK_DIALOG(Win)->vbox), NoteBook,TRUE, TRUE, 0);

	AddMMOptionsDlg(NoteBook);
  

	gtk_widget_realize(Win);

	button = create_button(Win,"Cancel");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", G_CALLBACK(DestroyWinMMDlg),GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,"Ok");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)amberEnergyCalculation,GTK_OBJECT(Win));
	gtk_widget_show (button);


	gtk_widget_show_all(Win);
  
}
/***********************************************************************/
void MolecularMechanicsMinimizeDlg()
{
	GtkWidget *button;
	GtkWidget *Win;
	GtkWidget *NoteBook;
	GtkWidget *parentWindow = GeomDlg;

	StopCalcul = TRUE;

	Win= gtk_dialog_new ();
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_title(&GTK_DIALOG(Win)->window,"Molecular Mechanics Minimize");
    	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)DestroyWinMMDlg,NULL);
 
	NoteBook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX (GTK_DIALOG(Win)->vbox), NoteBook,TRUE, TRUE, 0);

	AddGradientOptionsDlg(NoteBook);
	AddMMOptionsDlg(NoteBook);
  

	gtk_widget_realize(Win);

	button = create_button(Win,"Cancel");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", G_CALLBACK(DestroyWinMMDlg),GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,"Ok");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)amberMinimize,GTK_OBJECT(Win));
	gtk_widget_show (button);

  


	gtk_widget_show_all(Win);
  
}
/***********************************************************************/
void MolecularMechanicsDynamicsDlg()
{
	GtkWidget *button;
	GtkWidget *Win;
	GtkWidget *NoteBook;
	GtkWidget *parentWindow = GeomDlg;

	StopCalcul = TRUE;

	Win= gtk_dialog_new ();
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_title(&GTK_DIALOG(Win)->window,"Molecular Dynamics");
    	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)gtk_widget_destroy,NULL);
 
	NoteBook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX (GTK_DIALOG(Win)->vbox), NoteBook,TRUE, TRUE, 0);

	AddDynamicsOptionsDlg(NoteBook, Win);
	AddMMOptionsDlg(NoteBook);
	AddInfoMD(NoteBook, Win);
  

	gtk_widget_realize(Win);

	button = create_button(Win,"Cancel");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,"Ok");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)amberMolecularDynamics,GTK_OBJECT(Win));
	gtk_widget_show (button);

	gtk_widget_show_all(Win);
  
}
/***********************************************************************/
void MolecularMechanicsDynamicsConfoDlg()
{
	GtkWidget *button;
	GtkWidget *Win;
	GtkWidget *NoteBook;
	GtkWidget *parentWindow = GeomDlg;

	StopCalcul = TRUE;

	Win= gtk_dialog_new ();
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	gtk_window_set_title(&GTK_DIALOG(Win)->window,"Molecular Dynamics Conformational search");
    	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)gtk_widget_destroy,NULL);
 
	NoteBook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX (GTK_DIALOG(Win)->vbox), NoteBook,TRUE, TRUE, 0);

	AddGeneralConfoOptionsDlg(NoteBook, Win);
	AddDynamicsConfoOptionsDlg(NoteBook, Win);
	AddGradientOptionsDlg(NoteBook);
	AddMMOptionsDlg(NoteBook);
	AddInfoConfo(NoteBook, Win);
  

	gtk_widget_realize(Win);

	button = create_button(Win,"Cancel");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,"Ok");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)amberMolecularDynamicsConfo,GTK_OBJECT(Win));
	gtk_widget_show (button);

	gtk_widget_show_all(Win);
  
}
/***********************************************************************/
