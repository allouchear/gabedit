/* SemiEmpiricalDlg.c */
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
#include "AtomSE.h"
#include "MoleculeSE.h"
#include "SemiEmpiricalModel.h"
#include "SemiEmpirical.h"
#include "SemiEmpiricalMD.h"
#include "SemiEmpiricalDlg.h"

typedef enum
{
	TOLE = 0,
	TOLD = 1
} TOLptions;

#define NINTEGOPTIONS 3
#define NTHERMOPTIONS 4

#define NENTRYTOL 2

#define NCONSTRAINTS 3

static	GtkWidget* buttonConstraintsOptions[NCONSTRAINTS];

static	GtkWidget* entryTolerance[NENTRYTOL];
static	GtkWidget* buttonTolerance[NENTRYTOL];

static	GtkWidget* buttonMDOptions[NINTEGOPTIONS];
static	GtkWidget* buttonMDThermOptions[NTHERMOPTIONS];
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
static  GtkWidget* entryOpenBabelKeywords = NULL; 
static  GtkWidget* entryOpenBabelPotential = NULL; 
static  GtkWidget* entryGenericKeywords = NULL; 
static  GtkWidget* entryGenericPotential = NULL; 
static  GtkWidget* buttonPostNone = NULL ;
static  GtkWidget* buttonPostOpt = NULL ;
static  GtkWidget* buttonMopac = NULL ;
static  GtkWidget* buttonFireFly = NULL ;
static  GtkWidget* buttonOpenBabel = NULL ;
static  GtkWidget* buttonGeneric = NULL ;
static  GtkWidget* entryMopacMethod = NULL;
static  GtkWidget* entryFireFlyMethod = NULL;
static  GtkWidget* entryOpenBabelMethod = NULL;
static  GtkWidget* entryGenericMethod = NULL;
static  GtkWidget* entryMopacHamiltonianSparkle = NULL;
static  GtkWidget* entryMopacHamiltonian = NULL;
static  GtkWidget* entryAddMopacKeywords = NULL;
static  GtkWidget* entryOrcaHamiltonian = NULL;
static  GtkWidget* entryAddOrcaKeywords = NULL;
static  GtkWidget* entryAddOpenBabelKeywords = NULL;
static  gchar* genericProgName = NULL;
static gint totalCharge = 0;
static gint spinMultiplicity=1;

static gint numberOfPointsRP[2] = {10,10};
static gdouble stepValueRP[2] = {0.1,0.1};
static gchar typeRP[2][100] = {"Bond","Nothing"};
static gint atomRP[2] = {1,0};

static  GtkWidget* entryFileName = NULL;

/*********************************************************************************/
static void addMopacOptions(GtkWidget *box, gchar* type);
static void addOrcaOptions(GtkWidget *box, gchar* type);
static void addMopacSparkleOptions(GtkWidget *box, gchar* type);
static void addOpenBabelOptions(GtkWidget *box, gchar* type);
static gboolean runOneGeneric(MoleculeSE* mol, char* fileNamePrefix, char* keyWords, char* genericCommand);
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
static gboolean getEnergyMopac(gchar* fileNameOut, gdouble* energy)
{
	FILE* file = NULL;
	gchar buffer[1024];
	gchar* pdest = NULL;

 	file = FOpen(fileNameOut, "rb");
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
/*************************************************************************************************************/
static gint getRealNumberXYZVariables()
{
	gint k=0;
	gint i;
        for(i=0;i<NcentersXYZ;i++)
	{
		if(!test(GeomXYZ[i].X))k++;
		if(!test(GeomXYZ[i].Y))k++;
		if(!test(GeomXYZ[i].Z))k++;
	}
	return k;
}
/*************************************************************************************************************/
static void setValueVariableXYZ(gchar* source, gchar* value, gint* k)
{
	if(!test(source)) 
	{
		sprintf(value,"%f ", get_value_variableXYZ(source));
		*k = 1;
	}
	else
	{
		sprintf(value,"%s ",source);
		*k = 0;
	}
}
/*************************************************************************************************************/
static void setValueVariableZmat(gchar* source, gchar* value, gint* k)
{
	if(!test(source)) 
	{
		sprintf(value,"%f ", get_value_variableZmat(source));
		*k = 1;
	}
	else
	{
		sprintf(value,"%s ",source);
		*k = 0;
	}
}
/*************************************************************************************************************/
static gint getRealNumberZmatVariables()
{
	gint k=0;
	gint i;
        for(i=0;i<NcentersZmat;i++)
	{
        	if(Geom[i].Nentry>NUMBER_ENTRY_0 && !test(Geom[i].R)) k++;
        	if(Geom[i].Nentry>NUMBER_ENTRY_R && !test(Geom[i].Angle)) k++;
        	if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE && !test(Geom[i].Dihedral)) k++;
	}
	return k;
}
/*************************************************************************************************************/
static gboolean putMopacMoleculeInFile(FILE* file, gboolean RP)
{
        gchar buffer[BSIZE];
        gchar b1[20];
        gchar b2[20];
        gchar b3[20];
        gchar con[20];
	gint i;
	gint k1 = 0;
	gint k2 = 0;
	gint k3 = 0;
	gint nvar = 0;
	gint k;

	if(MethodeGeom==GEOM_IS_XYZ && NcentersXYZ<1) return FALSE;
	if(MethodeGeom!=GEOM_IS_XYZ && NcentersZmat<1) return FALSE;
	if(MethodeGeom==GEOM_IS_XYZ)
		nvar  = getRealNumberXYZVariables();
	else
		nvar  = getRealNumberZmatVariables();

	if(MethodeGeom==GEOM_IS_XYZ)
      		for (i=0;i<NcentersXYZ;i++)
		{
			setValueVariableXYZ(GeomXYZ[i].X, b1, &k1);
			setValueVariableXYZ(GeomXYZ[i].Y, b2, &k2);
			setValueVariableXYZ(GeomXYZ[i].Z, b3, &k3);
			if(nvar==0) { k1 = 1; k2 = 1; k3 = 1;}
			if(RP)
			{
				for(k=0;k<2;k++)
				{
					if(!strcmp(typeRP[k],"X") && i==atomRP[k]-1) k1 = -1;
					if(!strcmp(typeRP[k],"Y") && i==atomRP[k]-1) k2 = -1;
					if(!strcmp(typeRP[k],"Z") && i==atomRP[k]-1) k3 = -1;
				}
			}

			sprintf(buffer,"%s  %s %d %s %d %s %d\n",GeomXYZ[i].Symb,
					b1, k1,
					b2, k2,
					b3, k3
					);
        		fprintf(file, "%s",buffer);
		}
	else
	{
        	for(i=0;i<NcentersZmat;i++)
        	{
        		if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE)
			{
				setValueVariableZmat(Geom[i].R, b1, &k1);
				setValueVariableZmat(Geom[i].Angle, b2, &k2);
				setValueVariableZmat(Geom[i].Dihedral, b3, &k3);
				sprintf(con,"%s %s %s", Geom[i].NR, Geom[i].NAngle, Geom[i].NDihedral);
				if(nvar==0) { k1 = 1; k2 = 1; k3 = 1;}

				if(RP)
				{
					for(k=0;k<2;k++)
					{
						if(!strcmp(typeRP[k],"Bond") && i==atomRP[k]-1) k1 = -1;
						if(!strcmp(typeRP[k],"Angle") && i==atomRP[k]-1) k2 = -1;
						if(!strcmp(typeRP[k],"Dihedral") && i==atomRP[k]-1) k3 = -1;
					}
				}

				sprintf(buffer,"%s  %s %d %s %d %s %d %s\n",
						Geom[i].Symb,
						b1, k1,
						b2, k2,
						b3, k3,
						con
						);
        			fprintf(file, "%s",buffer);
			}
			else
        		if(Geom[i].Nentry>NUMBER_ENTRY_R)
			{
				setValueVariableZmat(Geom[i].R, b1, &k1);
				setValueVariableZmat(Geom[i].Angle, b2, &k2);
				sprintf(b3,"0.0"); k3 = 0;
				sprintf(con,"%s %s %s", Geom[i].NR, Geom[i].NAngle, "0");

				if(nvar==0) { k1 = 1; k2 = 1;}
				if(RP)
				{
					for(k=0;k<2;k++)
					{
						if(!strcmp(typeRP[k],"Bond") && i==atomRP[k]-1) k1 = -1;
						if(!strcmp(typeRP[k],"Angle") && i==atomRP[k]-1) k2 = -1;
					}
				}
				sprintf(buffer,"%s  %s %d %s %d %s %d %s\n",
						Geom[i].Symb,
						b1, k1,
						b2, k2,
						b3, k3,
						con
						);

        			fprintf(file, "%s",buffer);
			}
			else
        		if(Geom[i].Nentry>NUMBER_ENTRY_0)
			{
				setValueVariableZmat(Geom[i].R, b1, &k1);
				sprintf(b2,"0.0"); k2 = 0;
				sprintf(b3,"0.0"); k3 = 0;
				sprintf(con,"%s %s %s", Geom[i].NR, "0", "0");

				if(nvar==0) { k1 = 1;}
				if(RP)
				for(k=0;k<2;k++)
					if(!strcmp(typeRP[k],"Bond") && i==atomRP[k]-1) k1 = -1;

				sprintf(buffer,"%s  %s %d %s %d %s %d %s\n",
						Geom[i].Symb,
						b1, k1,
						b2, k2,
						b3, k3,
						con
						);
        			fprintf(file, "%s",buffer);
			}
			else
			{
				sprintf(b1,"0.0"); k1 = 0;
				sprintf(b2,"0.0"); k2 = 0;
				sprintf(b3,"0.0"); k3 = 0;
				sprintf(con,"%s %s %s", "0.0", "0", "0");
				sprintf(buffer,"%s  %s %d %s %d %s %d %s\n",
						Geom[i].Symb,
						b1, k1,
						b2, k2,
						b3, k3,
						con
						);
        			fprintf(file, "%s",buffer);
			}
        	}
	}
	return TRUE;
}
/*****************************************************************************/
static gboolean runOneMopac(gchar* fileNamePrefix, gchar* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gint j;
	gchar* fileNameIn = NULL;
	gchar* fileNameOut = NULL;
	gchar* fileNameSH = NULL;
	gchar multiplicityStr[100];
	gchar buffer[1024];
	gdouble energy = 0;
#ifdef G_OS_WIN32
	gchar c='%';
#endif

	if(!geometry0) return FALSE;
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%sMopacOne.sh",fileNamePrefix);
#else
	fileNameSH = g_strdup_printf("%sMopacOne.bat",fileNamePrefix);
#endif
 	fileSH = FOpen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"@echo off\n");
	fprintf(fileSH,"set PATH=%cPATH%c;\"%s\"\n",c,c,mopacDirectory);
#endif

	getMultiplicityName(spinMultiplicity, multiplicityStr);

	fileNameIn = g_strdup_printf("%sOne.mop",fileNamePrefix);
 	file = FOpen(fileNameIn, "w");
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
	if(spinMultiplicity>1)
	fprintf(file,"%s UHF CHARGE=%d %s\n",keyWords,totalCharge,multiplicityStr);
	else
	fprintf(file,"%s CHARGE=%d %s\n",keyWords,totalCharge,multiplicityStr);
	fprintf(file,"\n");
	fprintf(file,"Mopac file generated by Gabedit\n");

  	if(!putMopacMoleculeInFile(file, NULL!=strstr(keyWords,"POINT")))
	for(j=0;j<(gint)Natoms;j++)
	{
	fprintf(file," %s %f %d %f %d %f %d\n", 
			geometry0[j].Prop.symbol,
			geometry0[j].X*BOHR_TO_ANG,
			geometry0[j].Variable,
			geometry0[j].Y*BOHR_TO_ANG,
			geometry0[j].Variable,
			geometry0[j].Z*BOHR_TO_ANG,
			geometry0[j].Variable
			);
	}
	fclose(file);
	{
		gchar* str = NULL;
		if(strstr(keyWords,"XYZ") && strstr(keyWords,"PM7")) str = g_strdup_printf("Minimization by PM7/Mopac ... Please wait");
		else if(strstr(keyWords,"XYZ") && strstr(keyWords,"PM6-DH2")) str = g_strdup_printf("Minimization by PM6-DH2/Mopac ... Please wait");
		else if(strstr(keyWords,"XYZ") && strstr(keyWords,"PM6-DH+")) str = g_strdup_printf("Minimization by PM6-DH+/Mopac ... Please wait");
		else if(strstr(keyWords,"SPARKLE") && strstr(keyWords,"PM6")) str = g_strdup_printf("Minimization by Sparkle/PM6/Mopac ... Please wait");
		else if(strstr(keyWords,"SPARKLE") && strstr(keyWords,"AM1")) str = g_strdup_printf("Minimization by Sparkle/AM1/Mopac ... Please wait");
		else if(strstr(keyWords,"SPARKLE") && strstr(keyWords,"PM3")) str = g_strdup_printf("Minimization by Sparkle/PM3/Mopac ... Please wait");
		else if(strstr(keyWords,"XYZ") && strstr(keyWords,"PM6")) str = g_strdup_printf("Minimization by PM6/Mopac ... Please wait");
		else if(strstr(keyWords,"XYZ") && strstr(keyWords,"AM1")) str = g_strdup_printf("Minimization by AM1/Mopac ... Please wait");
		else if(strstr(keyWords,"ESP") && strstr(keyWords,"PM6-DH2")) str = g_strdup_printf("ESP charges from PM6-DH2/Mopac ... Please wait");
		else if(strstr(keyWords,"ESP") && strstr(keyWords,"PM6-DH+")) str = g_strdup_printf("ESP charges from PM6-DH+/Mopac ... Please wait");
		else if(strstr(keyWords,"ESP") && strstr(keyWords,"PM6")) str = g_strdup_printf("ESP charges from PM6/Mopac ... Please wait");
		else if(strstr(keyWords,"ESP") && strstr(keyWords,"AM1")) str = g_strdup_printf("ESP charges from AM1/Mopac ... Please wait");
		else if(strstr(keyWords,"ESP") && strstr(keyWords,"PM7")) str = g_strdup_printf("ESP charges from PM7/Mopac ... Please wait");
		else if(strstr(keyWords,"POINT")) str = g_strdup_printf("Reaction path by Mopac ... Please wait");
		else if(strstr(keyWords,"PM6-DH2")) str = g_strdup_printf("Computing of energy by PM6-DH2/Mopac .... Please wait");
		else if(strstr(keyWords,"PM6-DH+")) str = g_strdup_printf("Computing of energy by PM6-DH+/Mopac .... Please wait");
		else if(strstr(keyWords,"PM6")) str = g_strdup_printf("Computing of energy by PM6/Mopac .... Please wait");
		else if(strstr(keyWords,"PM7")) str = g_strdup_printf("Computing of energy by PM7/Mopac .... Please wait");
		else str = g_strdup_printf("Computing of energy by AM1/Mopac .... Please wait");
		set_text_to_draw(str);
		if(str) g_free(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
	}
#ifndef G_OS_WIN32
	fprintf(fileSH,"%s %s\n",NameCommandMopac,fileNameIn);
	fclose(fileSH);
	sprintf(buffer,"chmod u+x %s",fileNameSH);
	{int ierr= system(buffer);}
	{int ierr = system(fileNameSH);}
#else
	fprintf(fileSH,"\"%s\" \"%s\"\n",NameCommandMopac,fileNameIn);
	fclose(fileSH);
	sprintf(buffer,"\"%s\"",fileNameSH);
	{int ierr= system(buffer);}
#endif

	fileNameOut = g_strdup_printf("%sOne.out",fileNamePrefix);
	if(strstr(keyWords,"POINT")==NULL)
	{
		if(getEnergyMopac(fileNameOut,&energy))
		{
		gchar* str = NULL;

		read_geom_from_mopac_output_file(fileNameOut, -1);
		str = g_strdup_printf("Energy by Mopac = %f", energy);
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		Waiting(1);
		if(str) g_free(str);
		}
		else
		{
		gchar* str = NULL;
		str = g_strdup_printf(
				_(
				"Sorry, I cannot read the output file : %s "
				" ; Check also the installation of Mopac..."
				),
				fileNameOut
				);
		set_text_to_draw(str);
		if(str) g_free(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
 		if(fileNameIn) g_free(fileNameIn);
 		if(fileNameOut) g_free(fileNameOut);
 		if(fileNameSH) g_free(fileNameSH);
		return FALSE;
		}
	}

 	if(fileNameIn) g_free(fileNameIn);
 	if(fileNameOut) g_free(fileNameOut);
 	if(fileNameSH) g_free(fileNameSH);
	return TRUE;
}
/*****************************************************************************/
static gboolean getEnergyFireFly(gchar* fileNameOut, gdouble* energy)
{
	FILE* file = NULL;
	gchar buffer[1024];
	gchar* pdest = NULL;
	gboolean OK = FALSE;

 	file = FOpen(fileNameOut, "rb");
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
/*************************************************************************************************************/
static void putFireFlyMoleculeXYZFixed(FILE* file)
{
	gint i,k,l;
	gint nvar = 0;

        if(Natoms<2)return;
	nvar = 0;
        for(i=0;i<Natoms;i++)
		if(geometry[i].Variable) nvar+=3;
	/* printf("nvar = %d\n",nvar);*/
	if(nvar==3*Natoms) return;
	if(nvar==0) return;

        fprintf(file," ");
        fprintf(file, "$STATPT\n");
        fprintf (file,"   IFREEZ(1)=");

	l = 0;
        for(i=0;i<Natoms;i++)
	{
		if(!geometry[i].Variable)
		{
			l++;
			k = i*3+1;
			fprintf(file,"%d, %d, %d ",k,k+1,k+2);
			if(l%10==0) fprintf(file,"\n");
		}
	}
	fprintf(file,"\n ");
        fprintf (file, "$END\n");
}
/*****************************************************************************/
static gboolean runOneFireFly(gchar* fileNamePrefix, gchar* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gint j;
	gchar* fileNameIn = NULL;
	gchar* fileNameOut = NULL;
	gchar* fileNameSH = NULL;
	gchar multiplicityStr[100];
	gchar buffer[1024];
	gdouble energy = 0;
#ifdef G_OS_WIN32
	gchar c='%';
#endif

	if(!geometry0) return FALSE;
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%sPCGOne.sh",fileNamePrefix);
#else
	fileNameSH = g_strdup_printf("%sPCGOne.bat",fileNamePrefix);
#endif
 	fileSH = FOpen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"@echo off\n");
	fprintf(fileSH,"set PATH=%cPATH%c;\"%s\"\n",c,c,fireflyDirectory);
#endif

	getMultiplicityName(spinMultiplicity, multiplicityStr);

	fileNameIn = g_strdup_printf("%sOne.inp",fileNamePrefix);
 	file = FOpen(fileNameIn, "w");
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
	if(strstr(keyWords,"Optimize"))
	{
        	fprintf(file, " $STATPT OptTol=1e-4 NStep=500 $END\n");
	}
	if(strstr(keyWords,"Optimize"))
	{
		putFireFlyMoleculeXYZFixed(file);
	}
	fprintf(file," $DATA\n");
	fprintf(file,"Molecule specification\n");
	fprintf(file,"C1\n");
	for(j=0;j<(gint)Natoms;j++)
	{
		gchar* symbol = geometry0[j].Prop.symbol;
		SAtomsProp prop = prop_atom_get(symbol);
		fprintf(file,"%s %f %f %f %f\n", 
			symbol,
			(gdouble)prop.atomicNumber,
			geometry0[j].X*BOHR_TO_ANG,
			geometry0[j].Y*BOHR_TO_ANG,
			geometry0[j].Z*BOHR_TO_ANG
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
         	fprintf(fileSH,"del PUNCH 2> nul\n");
         	fprintf(fileSH,"del /Q  \"%stmp\"\n",fileNamePrefix);
         	fprintf(fileSH,"rmdir  \"%stmp\"\n",fileNamePrefix);
	}
	else
		fprintf(fileSH,"%s %s",NameCommandFireFly,fileNameIn);
#endif
	fclose(fileSH);
	{
		gchar* str = NULL;
		if(strstr(keyWords,"Optimiz")) str = g_strdup_printf("Minimization by AM1/FireFly ... Please wait");
		else str = g_strdup_printf("Computing of energy by AM1/FireFly .... Please wait");
		set_text_to_draw(str);
		if(str) g_free(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
	}
#ifndef G_OS_WIN32
	sprintf(buffer,"chmod u+x %s",fileNameSH);
	{int ierr= system(buffer);}
	{int ierr = system(fileNameSH);}
#else
	sprintf(buffer,"\"%s\"",fileNameSH);
	{int ierr= system(buffer);}
#endif
	if(getEnergyFireFly(fileNameOut,&energy))
	{
		gchar* str = NULL;

		str = g_strdup_printf("Energy by FireFly = %f", energy);
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		Waiting(1);
		if(str) g_free(str);
	}
	else
	{
		gchar* str = NULL;
		str = g_strdup_printf(
				_(
				"Sorry, I cannot read the output file :  %s"
				" ; Check also the installation of FireFly...")
				,
				fileNameOut
				);
		set_text_to_draw(str);
		if(str) g_free(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
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
static gboolean getEnergyOpenBabel(gchar* fileNameOut, gdouble* energy)
{
        FILE* file = NULL;
        char buffer[1024];
        char* pdest = NULL;
        char* energyTag = "FINAL ENERGY:";

        file = fopen(fileNameOut, "r");
        if(!file) return FALSE;
         while(!feof(file))
         {
                if(!fgets(buffer,BSIZE,file))break;
                pdest = strstr( buffer, energyTag);
                if(pdest &&sscanf(pdest+strlen(energyTag)+1,"%lf",energy)==1)
                {
                        fclose(file);
                        if(strstr(pdest,"kJ")) *energy /= KCALTOKJ;
                        return TRUE;
                }
         }
        fclose(file);
	return FALSE;
}
/*****************************************************************************/
static gboolean saveGeometry(MoleculeSE* molecule, double energy, char* fileNameGeom)
{
        gboolean Ok = FALSE;
        double oldEnergy = molecule->energy;
        molecule->energy = energy;
        Ok = saveMoleculeSE(molecule,fileNameGeom);
        molecule->energy = oldEnergy;
        return Ok;
}
/*************************************************************************************************************/
static gboolean runOneOpenBabel(MoleculeSE* mol, gchar* fileNamePrefix, gchar* NameCommandOpenBabel)
{
	FILE* fileSH = NULL;
	char* fileNameIn = NULL;
	char* fileNameOut = NULL;
	char* fileNameSH = NULL;
	char buffer[1024];
	double energy;
	MoleculeSE molecule;
	gboolean newMolSE = FALSE;
#ifdef G_OS_WIN32
	char c='%';
#endif

	if(!mol)
	{
		molecule = createMoleculeSE(geometry0,(gint)Natoms, totalCharge, spinMultiplicity, TRUE);
		mol = &molecule;
		newMolSE = TRUE;
	}
	/*
	else
	{
      		gint j;
		for(j=0;j<mol->nAtoms;j++)
		{
		gchar* symbol = mol->atoms[j].prop.symbol;
		SAtomsProp prop = prop_atom_get(symbol);
		fprintf(stderr,"%s %s %s %f %f %f %f\n", 
			symbol,
			mol->atoms[j].pdbType, mol->atoms[j].mmType,
			(gdouble)prop.atomicNumber,
			mol->atoms[j].coordinates[0],
			mol->atoms[j].coordinates[1],
			mol->atoms[j].coordinates[2]
			);
		}
	}
	*/

	if(mol->nAtoms<1) return FALSE;
#ifndef G_OS_WIN32
	fileNameSH =g_strdup_printf("%sOne.sh",fileNamePrefix);
#else
	fileNameSH =g_strdup_printf("%sOne.bat",fileNamePrefix);
#endif
 	fileSH = fopen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"@echo off\n");
#endif

	fileNameIn =g_strdup_printf("%sOne.hin",fileNamePrefix);
	fileNameOut =g_strdup_printf("%sOne.out",fileNamePrefix);

	if(!saveMoleculeSEHIN(mol, fileNameIn))
	{
 		if(fileNameIn) free(fileNameIn);
 		if(fileNameOut) free(fileNameOut);
 		if(fileNameSH) free(fileNameSH);
		if(mol && newMolSE) freeMoleculeSE(mol);
		return FALSE;
	}
#ifndef G_OS_WIN32
	fprintf(fileSH,"#!/bin/bash\n");
	fprintf(fileSH,"export PATH=$PATH:%s\n",openbabelDirectory);
	fprintf(fileSH,"export BABEL_DATADIR=%s\n",openbabelDirectory);
	if(!strstr(NameCommandOpenBabel,"obgradient") || !strstr(NameCommandOpenBabel,"obopt"))
	{
		fprintf(fileSH,"%s %s > %s\n",NameCommandOpenBabel,fileNameIn,fileNameOut);
		fprintf(fileSH,"exit\n");
	}
	else 
	{
		if(!strstr( NameCommandOpenBabel,"obopt")) 
		{
			char** ssplit = NULL;
			int nA = 0;
			int i;
			ssplit = gab_split(NameCommandOpenBabel);
			while(ssplit && ssplit[nA]!=NULL) nA++;
			fprintf(fileSH,"%s ", "obopt");
			for(i=1;i<nA;i++) fprintf(fileSH,"%s ",  ssplit[i]);
			fprintf(fileSH," %s > %s 2>/dev/null", fileNameIn, fileNameOut);
			gab_strfreev(ssplit);
		}
		else fprintf(fileSH,"%s %s > %s 2>/dev/null", NameCommandOpenBabel, fileNameIn, fileNameOut);
	}
#else
	if(strstr(openbabelDirectory,"\"")) 
	{
		fprintf(fileSH,"set PATH=%s;%cPATH%c\n",openbabelDirectory,'%','%');
		fprintf(fileSH,"set BABEL_DATADIR=%s\n",openbabelDirectory);
	}
	else 
	{
		fprintf(fileSH,"set PATH=\"%s\";%cPATH%c\n",openbabelDirectory,'%','%');
		fprintf(fileSH,"set BABEL_DATADIR=%s\n",openbabelDirectory);
	}
	if(!strstr(NameCommandOpenBabel,"obgradient") || !strstr(NameCommandOpenBabel,"obopt"))
	{
		fprintf(fileSH,"%s %s > %s\n",NameCommandOpenBabel,fileNameIn,fileNameOut);
		fprintf(fileSH,"exit\n");
	}
	else
	{
		if(!strstr( NameCommandOpenBabel,"obopt")) 
		{
			char** ssplit = NULL;
			int nA = 0;
			int i;
			ssplit = gab_split(NameCommandOpenBabel);
			while(ssplit && ssplit[nA]!=NULL) nA++;
			fprintf(fileSH,"%s ", "obopt");
			for(i=1;i<nA;i++) fprintf(fileSH,"%s ",  ssplit[i]);
			fprintf(fileSH," %s > %s 2>/dev/null", fileNameIn, fileNameOut);
			gab_strfreev(ssplit);
		}
		else fprintf(fileSH,"%s %s > %s", NameCommandOpenBabel, fileNameIn, fileNameOut);
	}
#endif
	fclose(fileSH);
#ifndef G_OS_WIN32
	/*
	sprintf(buffer,"cat %s",fileNameSH);
	system(buffer);
	sprintf(buffer,"cat %s",fileNameIn);
	system(buffer);
	*/



	sprintf(buffer,"chmod u+x %s",fileNameSH);
	system(buffer);
	system(fileNameSH);
#else
	sprintf(buffer,"\"%s\"",fileNameSH);
	system(buffer);
#endif
	if(getEnergyOpenBabel(fileNameOut,&energy))
	{
		printf("Energy by OpenBabel = %f\n", energy);
		readGeomMoleculeSEFromOpenBabelOutputFile(mol, fileNameOut, -1);
		mol->energy = energy;
		if(strstr( NameCommandOpenBabel,"obopt"))
		{
			char* str =g_strdup_printf("%s.gab",fileNamePrefix);
			saveGeometry(mol, energy, str);
			read_geom_from_gabedit_geom_conv_file(str, 1);
			if(str) free(str);
			/* str = g_strdup_printf("Energy by OpenBabel = %f", energy);*/
			str = g_strdup_printf("Gradient = %f Energy by OpenBabel = %f",getGradientNormMoleculeSE(mol), energy);
			set_text_to_draw(str);
			drawGeom();
    			while( gtk_events_pending() ) gtk_main_iteration();
			Waiting(1);
			/*
			printf("----------------------------------------- \n");
			printf("Optimized geometry saved in %s file\n",str);
			printf("----------------------------------------- \n");
			*/
			if(str) free(str);
		}
		else
		{
			gchar* str = NULL;
			str = g_strdup_printf("Energy by OpenBabel = %f", energy);
			set_text_to_draw(str);
			drawGeom();
    			while( gtk_events_pending() ) gtk_main_iteration();
			Waiting(1);
			if(str) g_free(str);
			/*
			char* str =g_strdup_printf("%s.gab",fileNamePrefix);
			saveGeometry(mol, energy, str);
			printf("----------------------------------------- \n");
			printf("Geometry saved in %s file\n",str);
			printf("----------------------------------------- \n");
			if(str) free(str);
			*/
		}
	}
	else
	{
		gchar* str = NULL;
		str = g_strdup_printf(
				_(
				"Sorry, I cannot read the output file :  %s"
				" ; Check also the installation of OpenBabel...")
				,
				fileNameOut
				);
		set_text_to_draw(str);
		if(str) g_free(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		return FALSE;
	}

 	if(fileNameIn) free(fileNameIn);
 	if(fileNameOut) free(fileNameOut);
 	if(fileNameSH) free(fileNameSH);
	if(mol && newMolSE) freeMoleculeSE(mol);
	return TRUE;
}
/*****************************************************************************/
static gboolean getEnergyOrca(gchar* fileNameOut, gdouble* energy)
{
	FILE* file = NULL;
	gchar buffer[1024];
	gchar* pdest = NULL;
	gchar* energyTag = "FINAL SINGLE POINT ENERGY";

 	file = FOpen(fileNameOut, "rb");
	if(!file) return FALSE;
	 while(!feof(file))
	 {
		if(!fgets(buffer,BSIZE,file))break;
		pdest = strstr( buffer, energyTag);
		if(pdest &&sscanf(pdest+strlen(energyTag)+1,"%lf",energy)==1)
		{
			fclose(file);
			*energy *=627.50944796;
			return TRUE;
		}
	 }
	fclose(file);
	return FALSE;
}
/*****************************************************************************/
static gboolean runOneOrca(gchar* fileNamePrefix, gchar* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gchar* fileNameIn = NULL;
	gchar* fileNameOut = NULL;
	gchar* fileNameSH = NULL;
	gchar multiplicityStr[100];
	gchar buffer[1024];
	gdouble energy = 0;
	gint i;
	gint nV;

	if(!geometry0) return FALSE;
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%sOne.sh",fileNamePrefix);
#else
	fileNameSH = g_strdup_printf("%sOne.bat",fileNamePrefix);
#endif
 	fileSH = FOpen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"@echo off\n");
#endif

	getMultiplicityName(spinMultiplicity, multiplicityStr);

	fileNameIn = g_strdup_printf("%sOne.inp",fileNamePrefix);
 	file = FOpen(fileNameIn, "w");
	if(!file) 
	{
 		if(fileNameIn) g_free(fileNameIn);
 		if(fileNameOut) g_free(fileNameOut);
 		if(fileNameSH) g_free(fileNameSH);
		return FALSE;
	}
	fprintf(file,"# ======================================================\n");
	fprintf(file,"#  Orca input file made in Gabedit\n"); 
	fprintf(file,"# ======================================================\n");
	fprintf(file,"! %s\n",keyWords);
	if(MethodeGeom==GEOM_IS_XYZ)
	{
		fprintf(file,"* xyz %d   %d\n",totalCharge,spinMultiplicity);
      		for (i=0;i<NcentersXYZ;i++)
		{
			gchar X[100];
			gchar Y[100];
			gchar Z[100];
			sprintf(X,"%s",GeomXYZ[i].X);
  			/* if(!test(GeomXYZ[i].X)) sprintf(X,"{%s}",GeomXYZ[i].X);*/
  			if(!test(GeomXYZ[i].X)) sprintf(X," %lf ", get_value_variableXYZ(GeomXYZ[i].X));
			sprintf(Y,"%s",GeomXYZ[i].Y);
  			/* if(!test(GeomXYZ[i].Y)) sprintf(Y,"{%s}",GeomXYZ[i].Y);*/
  			if(!test(GeomXYZ[i].Y)) sprintf(Y," %lf ", get_value_variableXYZ(GeomXYZ[i].Y));
			sprintf(Z,"%s",GeomXYZ[i].Z);
  			/* if(!test(GeomXYZ[i].Z)) sprintf(Z,"{%s}",GeomXYZ[i].Z);*/
  			if(!test(GeomXYZ[i].Z)) sprintf(Z," %lf ", get_value_variableXYZ(GeomXYZ[i].Z));

			fprintf(file," %s  %s %s %s\n",GeomXYZ[i].Symb, X,Y,Z);
		}
		fprintf(file,"*\n");
		nV = 0;
        	if(NVariablesXYZ>0)
        	for(i=0;i<NVariablesXYZ;i++)
        	{
        		if(VariablesXYZ[i].Used)
			{
				/* fprintf(file,"%cparams \n",'%');*/
				nV++;
				break;
			}
        	}
		/*
        	for(i=0;i<NVariablesXYZ;i++)
        	{
        		if(VariablesXYZ[i].Used)
			{
  				fprintf(file," %s  %s\n",VariablesXYZ[i].Name,VariablesXYZ[i].Value);
			}
        	}
		if(nV>0) fprintf(file," end #params\n");
		*/
		if(nV>0) 
		{
			fprintf(file,"%cgeom Constraints\n",'%');
      			for (i=0;i<NcentersXYZ;i++)
			{
  				if(!test(GeomXYZ[i].X) || !test(GeomXYZ[i].Y) || !test(GeomXYZ[i].Z)) 
				{
					fprintf(file,"  {C %d C}\n",i);
				}
			}
			fprintf(file," end #Constraints\n");
			fprintf(file," invertConstraints true\n");
			fprintf(file," end #geom\n");
		}
	}
	else
	{
		fprintf(file,"* int %d   %d\n",totalCharge,spinMultiplicity);
        	for(i=0;i<NcentersZmat;i++)
        	{
        		if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE)
			{
				gchar R[100];
				gchar A[100];
				gchar D[100];
				sprintf(R,"%s",Geom[i].R);
  				/* if(!test(Geom[i].R)) sprintf(R,"{%s}",Geom[i].R);*/
  				if(!test(Geom[i].R)) sprintf(R," %lf ",get_value_variableZmat(Geom[i].R));
				sprintf(A,"%s",Geom[i].Angle);
  				/* if(!test(Geom[i].Angle)) sprintf(A,"{%s}",Geom[i].Angle);*/
  				if(!test(Geom[i].Angle)) sprintf(A," %lf ",get_value_variableZmat(Geom[i].Angle));
				sprintf(D,"%s",Geom[i].Dihedral);
  				/* if(!test(Geom[i].Dihedral)) sprintf(D,"{%s}",Geom[i].Dihedral);*/
  				if(!test(Geom[i].Dihedral)) sprintf(D," %lf ",get_value_variableZmat(Geom[i].Dihedral));

				fprintf(file," %s %s %s %s %s %s %s\n",
						Geom[i].Symb,
						Geom[i].NR,
						Geom[i].NAngle,
						Geom[i].NDihedral,
						R,A,D);
			}
			else
        		if(Geom[i].Nentry>NUMBER_ENTRY_R)
			{
				gchar R[100];
				gchar A[100];
				sprintf(R,"%s",Geom[i].R);
  				/* if(!test(Geom[i].R)) sprintf(R,"{%s}",Geom[i].R);*/
  				if(!test(Geom[i].R)) sprintf(R," %lf ",get_value_variableZmat(Geom[i].R));
				sprintf(A,"%s",Geom[i].Angle);
  				/* if(!test(Geom[i].Angle)) sprintf(A,"{%s}",Geom[i].Angle);*/
  				if(!test(Geom[i].Angle)) sprintf(A," %lf ",get_value_variableZmat(Geom[i].Angle));
				/*
				fprintf(file," %s  %s %s %s %s\n",
						Geom[i].Symb,
						Geom[i].NR,R,
						Geom[i].NAngle,A
						);
				*/
				fprintf(file," %s %s %s %s %s %s %s\n",
						Geom[i].Symb,
						Geom[i].NR,
						Geom[i].NAngle,
						"0",
						R,A,"0.0");
			}
			else
        		if(Geom[i].Nentry>NUMBER_ENTRY_0)
			{
				gchar R[100];
				sprintf(R,"%s",Geom[i].R);
  				if(!test(Geom[i].R)) sprintf(R," %lf ",get_value_variableZmat(Geom[i].R));
  				/* if(!test(Geom[i].R)) sprintf(R,"{%s}",Geom[i].R);*/
				/* fprintf(file," %s  %s %s\n", Geom[i].Symb, Geom[i].NR,R);*/
				fprintf(file," %s %s %s %s %s %s %s\n",
						Geom[i].Symb,
						Geom[i].NR,
						"0",
						"0",
						R,"0.0","0.0");
			}
			else
			{
				//fprintf(file," %s \n", Geom[i].Symb);
				fprintf(file," %s %s %s %s %s %s %s\n",
						Geom[i].Symb,
						"0",
						"0",
						"0",
						"0.0","0.0","0.0");
			}
        	}
		fprintf(file,"*\n");
		nV = 0;
        	if(NVariables>0)
        	for(i=0;i<NVariables;i++)
        	{
        		if(Variables[i].Used)
			{
				/* fprintf(file,"%cparams \n",'%');*/
				nV++;
				break;
			}
        	}
		/*
        	for(i=0;i<NVariables;i++)
        	{
        		if(Variables[i].Used)
			{
  				fprintf(file," %s  %s\n",Variables[i].Name,Variables[i].Value);
			}
        	}
		if(nV>0)
        		fprintf(file," end #params\n");
		*/
		if(nV>0) 
		{
			fprintf(file,"%cgeom Constraints\n",'%');
      			for (i=0;i<NcentersZmat;i++)
			{
  				if(Geom[i].Nentry>=NUMBER_ENTRY_R && !test(Geom[i].R)) 
				{
					fprintf(file,"  {B %d %d C}\n",atoi(Geom[i].NR)-1,i);
				}
  				if(Geom[i].Nentry>=NUMBER_ENTRY_ANGLE && !test(Geom[i].Angle)) 
				{
					fprintf(file,"  {A %d %d %d C}\n",
							atoi(Geom[i].NAngle)-1,
							atoi(Geom[i].NR)-1,i);
				}
  				if(Geom[i].Nentry>NUMBER_ENTRY_ANGLE && !test(Geom[i].Dihedral)) 
				{
					fprintf(file,"  {D %d %d %d %d C}\n",
							atoi(Geom[i].NDihedral)-1, 
							atoi(Geom[i].NAngle)-1,
							atoi(Geom[i].NR)-1,i);
				}
			}
			fprintf(file," end #Constraints\n");
			fprintf(file," invertConstraints true\n");
			fprintf(file," end #geom\n");
		}
	}

	fclose(file);
	fileNameOut = g_strdup_printf("%sOne.out",fileNamePrefix);
#ifndef G_OS_WIN32
	if(!strcmp(NameCommandOrca,"orca") || !strcmp(NameCommandOrca,"nohup orca"))
	{
		fprintf(fileSH,"%s %s > %s\n",NameCommandOrca,fileNameIn,fileNameOut);
		fprintf(fileSH,"exit\n");
	}
	else
		fprintf(fileSH,"%s %s",NameCommandOrca,fileNameIn);
#else
	 if(!strcmp(NameCommandOrca,"orca") )
	{
		if(strstr(orcaDirectory,"\"")) fprintf(fileSH,"set PATH=%s;%cPATH%c\n",orcaDirectory,'%','%');
		else fprintf(fileSH,"set PATH=\"%s\";%cPATH%c\n",orcaDirectory,'%','%');
		fprintf(fileSH,"%s %s > %s\n",NameCommandOrca,fileNameIn,fileNameOut);
		fprintf(fileSH,"exit\n");
	}
	else
		fprintf(fileSH,"%s %s",NameCommandOrca,fileNameIn);
#endif
	fclose(fileSH);
	{
		gchar* str = NULL;
		if(strstr(keyWords,"Opt")) str = g_strdup_printf("Minimization by Orca ... Please wait");
		else str = g_strdup_printf("Computing of energy by Orca .... Please wait");
		set_text_to_draw(str);
		if(str) g_free(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
	}
#ifndef G_OS_WIN32
	sprintf(buffer,"chmod u+x %s",fileNameSH);
	{int ierr= system(buffer);}
	{int ierr = system(fileNameSH);}
#else
	sprintf(buffer,"\"%s\"",fileNameSH);
	{int ierr= system(buffer);}
#endif
	if(getEnergyOrca(fileNameOut,&energy))
	{
		gchar* str = NULL;

		str = g_strdup_printf("Energy by Orca = %f", energy);
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		Waiting(1);
		if(str) g_free(str);
	}
	else
	{
		gchar* str = NULL;
		str = g_strdup_printf(
				_(
				"Sorry, I cannot read the output file :  %s"
				" ; Check also the installation of Orca..."
				),
				fileNameOut
				);
		set_text_to_draw(str);
		if(str) g_free(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
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
static void runGeneric(MoleculeSE* mol, char* fileName, char* keys, char* genericCommand)
{
	if(keys && genericCommand)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneGeneric(mol, fileNamePrefix, keys, genericCommand))
		{
		}
		if(fileNamePrefix) free(fileNamePrefix);
	}
}
/*****************************************************************************/
static void runSemiEmpirical(GtkWidget* Win, gpointer data, gchar* type, gchar* keys)
{
	gchar* fileName = NULL;
	/* fileName */
	{
		gchar* dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFileName)));
		if(dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
			fileName = g_strdup_printf("%s%s%s",dirName, G_DIR_SEPARATOR_S,tmp);
		else
			fileName = g_strdup_printf("%s%s",dirName, tmp);

		g_free(tmp);
		g_free(dirName);
	}

	gtk_widget_destroy(Win);
    	while( gtk_events_pending() ) gtk_main_iteration();

	{
		gint i;
		for(i=0;i<4;i++) NumSelAtoms[i] = -1;
		change_of_center(NULL,NULL);
	}

	if(!strcmp(type,"PM6DH2MopacEnergy"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "PM6-DH2 1SCF"))
		{
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"PM6DH+MopacEnergy"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "PM6-DH+ 1SCF"))
		{
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else
	if(!strcmp(type,"PM6MopacEnergy"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "PM6 1SCF"))
		{
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else
	if(!strcmp(type,"PM7MopacEnergy"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "PM7 1SCF"))
		{
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"PM6DH2MopacOptimize"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "PM6-DH2 XYZ AUX"))
		{
			gchar* fileOut = g_strdup_printf("%sOne.aux",fileNamePrefix);
			find_energy_mopac_aux(fileOut);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"PM6DH+MopacOptimize"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "PM6-DH+ XYZ AUX"))
		{
			gchar* fileOut = g_strdup_printf("%sOne.aux",fileNamePrefix);
			find_energy_mopac_aux(fileOut);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"PM6MopacOptimize"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "PM6 XYZ AUX"))
		{
			gchar* fileOut = g_strdup_printf("%sOne.aux",fileNamePrefix);
			find_energy_mopac_aux(fileOut);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"PM7MopacOptimize"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "PM7 XYZ AUX"))
		{
			gchar* fileOut = g_strdup_printf("%sOne.aux",fileNamePrefix);
			find_energy_mopac_aux(fileOut);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"PM6DH2MopacESP"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "PM6-DH2 1SCF ESP"))
		{
			gchar* fileOut = g_strdup_printf("%sOne.out",fileNamePrefix);
			read_geom_from_mopac_output_file(fileOut, -1);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"PM6DH+MopacESP"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "PM6-DH+ 1SCF ESP"))
		{
			gchar* fileOut = g_strdup_printf("%sOne.out",fileNamePrefix);
			read_geom_from_mopac_output_file(fileOut, -1);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"PM6MopacESP"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "PM6 1SCF ESP"))
		{
			gchar* fileOut = g_strdup_printf("%sOne.out",fileNamePrefix);
			read_geom_from_mopac_output_file(fileOut, -1);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"PM7MopacESP"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "PM7 1SCF ESP"))
		{
			gchar* fileOut = g_strdup_printf("%sOne.out",fileNamePrefix);
			read_geom_from_mopac_output_file(fileOut, -1);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"AM1MopacEnergy"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "AM1 1SCF"))
		{
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"AM1MopacOptimize"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "AM1 XYZ AUX"))
		{
			gchar* fileOut = g_strdup_printf("%sOne.aux",fileNamePrefix);
			find_energy_mopac_aux(fileOut);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"AM1MopacESP"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, "AM1 1SCF ESP"))
		{
			gchar* fileOut = g_strdup_printf("%sOne.out",fileNamePrefix);
			read_geom_from_mopac_output_file(fileOut, -1);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"MopacEnergy") && keys)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, keys))
		{
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"MopacOptimize") && keys)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, keys))
		{
			gchar* fileOut = g_strdup_printf("%sOne.aux",fileNamePrefix);
			find_energy_mopac_aux(fileOut);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
        else if(!strcmp(type,"MopacOptimizeSparkle") && keys)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, keys))
		{
			gchar* fileOut = g_strdup_printf("%sOne.aux",fileNamePrefix);
			find_energy_mopac_aux(fileOut);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"MopacESP") && keys)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, keys))
		{
			gchar* fileOut = g_strdup_printf("%sOne.out",fileNamePrefix);
			read_geom_from_mopac_output_file(fileOut, -1);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"MopacScan") && keys)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneMopac(fileNamePrefix, keys))
		{
			gchar* fileOut = g_strdup_printf("%sOne.out",fileNamePrefix);
			find_energy_mopac_scan_output(fileOut);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"OrcaEnergy") && keys)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneOrca(fileNamePrefix, keys))
		{
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"OrcaOptimize") && keys)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneOrca(fileNamePrefix, keys))
		{
			gchar* fileOut = g_strdup_printf("%sOne.out",fileNamePrefix);
			find_energy_orca_output(fileOut);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"AM1FireFlyEnergy"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneFireFly(fileNamePrefix, "RUNTYP=Energy GBASIS=AM1"))
		{
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"AM1FireFlyOptimize"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneFireFly(fileNamePrefix, "RUNTYP=Optimize GBASIS=AM1"))
		{
			gchar* fileOut = g_strdup_printf("%sOne.out",fileNamePrefix);
			find_energy_gamess_output_heat(fileOut);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"OpenBabelEnergy"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		printf("Keys = %s\n",keys);
		if(runOneOpenBabel(NULL,fileNamePrefix,keys))
		{
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"OpenBabelOptimize"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		if(runOneOpenBabel(NULL,fileNamePrefix, keys))
		{
			gchar* fileOut = g_strdup_printf("%sOne.out",fileNamePrefix);
			find_energy_gamess_output_heat(fileOut);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"GenericEnergy"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		printf("Keys = %s\n",keys);
		if(runOneGeneric(NULL,fileNamePrefix,"Energy",keys))
		{
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
	else if(!strcmp(type,"GenericOptimize"))
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileName);
		printf("Keys = %s\n",keys);
		if(runOneGeneric(NULL,fileNamePrefix, "Opt",keys))
		{
			gchar* fileOut = g_strdup_printf("%sOne.out",fileNamePrefix);
			find_energy_gamess_output_heat(fileOut);
			if(fileOut) g_free(fileOut);
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
	}
}
/*****************************************************************************/
static void runAM1FireFlyEnergy(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "AM1FireFlyEnergy",NULL);
}
/*****************************************************************************/
static void runAM1FireFlyOptimize(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "AM1FireFlyOptimize",NULL);
}
/*****************************************************************************/
static void runOpenBabelEnergy(GtkWidget* Win, gpointer data)
{
	G_CONST_RETURN gchar* potential = gtk_entry_get_text(GTK_ENTRY(entryOpenBabelPotential));
	G_CONST_RETURN gchar* options   = gtk_entry_get_text(GTK_ENTRY(entryAddOpenBabelKeywords));
	gchar* keys = NULL;
	/*
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	*/
	TotalCharges[0] = 0;
	SpinMultiplicities[0] = 1;
	if(potential && (options && strlen(options) >1) ) keys = g_strdup_printf("obgradient -ff %s %s",potential, options);
	else if(potential) keys = g_strdup_printf("obgradient -ff %s",potential);
	else keys = g_strdup_printf("obgradient -ff MMFF94");

	runSemiEmpirical(Win, data, "OpenBabelEnergy",keys);
	if(keys) g_free(keys);
}
/*****************************************************************************/
static void runOpenBabelOptimize(GtkWidget* Win, gpointer data)
{
	G_CONST_RETURN gchar* potential = gtk_entry_get_text(GTK_ENTRY(entryOpenBabelPotential));
	G_CONST_RETURN gchar* options   = gtk_entry_get_text(GTK_ENTRY(entryAddOpenBabelKeywords));
	gchar* keys = NULL;
	/*
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	*/
	TotalCharges[0] = 0;
	SpinMultiplicities[0] = 1;
	if(potential && (options && strlen(options) >1) ) keys = g_strdup_printf("obopt -ff %s %s",potential, options);
	else if(potential) keys = g_strdup_printf("obopt -ff %s",potential);
	else keys = g_strdup_printf("obopt -ff MMFF94");
	runSemiEmpirical(Win, data, "OpenBabelOptimize",keys);
	if(keys) g_free(keys);
}
/*****************************************************************************/
static void runGenericEnergy(GtkWidget* Win, gpointer data)
{
	G_CONST_RETURN gchar* potential = gtk_entry_get_text(GTK_ENTRY(entryGenericPotential));
	gchar* keys = NULL;
	/*
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	*/
	TotalCharges[0] = 0;
	SpinMultiplicities[0] = 1;
	if(potential) keys = g_strdup_printf("%s",potential);
	else keys = g_strdup_printf("generic");

	if(genericProgName) g_free(genericProgName);
	genericProgName = g_strdup(keys);
	runSemiEmpirical(Win, data, "GenericEnergy",keys);
	if(keys) g_free(keys);
}
/*****************************************************************************/
static void runGenericOptimize(GtkWidget* Win, gpointer data)
{
	G_CONST_RETURN gchar* potential = gtk_entry_get_text(GTK_ENTRY(entryGenericPotential));
	gchar* keys = NULL;
	/*
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	*/
	TotalCharges[0] = 0;
	SpinMultiplicities[0] = 1;
	if(potential) keys = g_strdup_printf("%s",potential);
	else keys = g_strdup_printf("generic");
	runSemiEmpirical(Win, data, "GenericOptimize",keys);
	if(keys) g_free(keys);
}
/*****************************************************************************/
static void runPM6DH2MopacEnergy(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "PM6DH2MopacEnergy",NULL);
}
/*****************************************************************************/
static void runPM6DH2MopacOptimize(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "PM6DH2MopacOptimize",NULL);
}
/*****************************************************************************/
static void runPM6DH2MopacESP(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "PM6DH2MopacESP",NULL);
}
/*****************************************************************************/
static void runPM6DHpMopacEnergy(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "PM6DH+MopacEnergy",NULL);
}
/*****************************************************************************/
static void runPM6DHpMopacOptimize(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "PM6DH+MopacOptimize",NULL);
}
/*****************************************************************************/
static void runPM6DHpMopacESP(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "PM6DH+MopacESP",NULL);
}
/*****************************************************************************/
static void runPM6MopacEnergy(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "PM6MopacEnergy",NULL);
}
/*****************************************************************************/
static void runPM6MopacOptimize(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "PM6MopacOptimize",NULL);
}
/*****************************************************************************/
static void runPM6MopacESP(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "PM6MopacESP",NULL);
}
/*****************************************************************************/
static void runPM7MopacEnergy(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "PM7MopacEnergy",NULL);
}
/*****************************************************************************/
static void runPM7MopacOptimize(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "PM7MopacOptimize",NULL);
}
/*****************************************************************************/
static void runPM7MopacESP(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "PM7MopacESP",NULL);
}
/*****************************************************************************/
static void runAM1MopacEnergy(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "AM1MopacEnergy",NULL);
}
/*****************************************************************************/
static void runAM1MopacOptimize(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "AM1MopacOptimize",NULL);
}
/*****************************************************************************/
static void runAM1MopacESP(GtkWidget* Win, gpointer data)
{
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "AM1MopacESP",NULL);
}
/*****************************************************************************/
static gboolean getEnergyGeneric(char* fileNameOut, double* energy)
{
	FILE* file = NULL;
	char buffer[1024];
	int i;
 	file = fopen(fileNameOut, "r");
	if(!file) return FALSE;
	if(!fgets(buffer,BSIZE,file)) { fclose(file); return FALSE;}/* first line for energy in Hartree*/

	for(i=0;i<strlen(buffer);i++) if(buffer[i]=='D' || buffer[i]=='d') buffer[i] ='E';
	if(sscanf(buffer,"%lf",energy)==1)
	{
		fclose(file);
		*energy *=AUTOKCAL;
		return TRUE;
	}
	fclose(file);
	return FALSE;
}
/*****************************************************************************/
static gboolean runOneGeneric(MoleculeSE* mol, char* fileNamePrefix, char* keyWords, char* genericCommand)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gchar* fileNameIn = NULL;
	gchar* fileNameOut = NULL;
	gchar* fileNameSH = NULL;
	gchar multiplicityStr[100];
	gchar buffer[1024];
	gdouble energy = 0;
	gint type = 0;
	gboolean newMolSE = FALSE;
	MoleculeSE molecule;
#ifdef OS_WIN32
	char c='%';
#endif

	if(!mol)
	{
		molecule = createMoleculeSE(geometry0,(gint)Natoms, totalCharge, spinMultiplicity, TRUE);
		mol = &molecule;
		newMolSE = TRUE;
	}

	if(!mol) return FALSE;
	if(mol->nAtoms<1) return FALSE;
#ifndef OS_WIN32
	fileNameSH = g_strdup_printf("%sGenericOne.sh",fileNamePrefix);
#else
	fileNameSH = g_strdup_printf("%sGenericOne.bat",fileNamePrefix);
#endif
 	fileSH = fopen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef OS_WIN32
	fprintf(fileSH,"@echo off\n");
#endif

	fileNameIn = g_strdup_printf("%sOne.inp",fileNamePrefix);
	fileNameOut = g_strdup_printf("%sOne.out",fileNamePrefix);

 	file = fopen(fileNameIn, "w");
	if(!file) 
	{
 		if(fileNameIn) free(fileNameIn);
 		if(fileNameOut) free(fileNameOut);
 		if(fileNameSH) free(fileNameSH);
		return FALSE;
	}
	if(strstr(keyWords,"Opt")) type = 2;
	if(strstr(keyWords,"ENGRAD")) type = 1;
	fprintf(file,"%d\n",type);
	addMoleculeSEToFile(mol,file);
	fclose(file);
	{
		char* str = NULL;
		if(type==2) str = g_strdup_printf("Minimization by Generic/%s ... Please wait",genericCommand);
		else str = g_strdup_printf("Energy by Generic/%s ... Please wait",genericCommand);
		/* printf("%s\n",str);*/
		set_text_to_draw(str);
		if(str) g_free(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
	}
#ifndef OS_WIN32
	fprintf(fileSH,"%s %s %s",genericCommand,fileNameIn,fileNameOut);
	fclose(fileSH);
	sprintf(buffer,"chmod u+x %s",fileNameSH);
	system(buffer);
	system(fileNameSH);
#else
	fprintf(fileSH,"\"%s\" \"%s\" \"%s\"",genericCommand,fileNameIn,fileNameOut);
	fclose(fileSH);
	sprintf(buffer,"\"%s\"",fileNameSH);
	system(buffer);
#endif

	if(getEnergyGeneric(fileNameOut,&energy))
	{
		gchar* str = NULL;
		str = g_strdup_printf("Energy by %s = %f", genericCommand,energy);
		/* printf("%s\n",str);*/
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		if(str) g_free(str);
		if(strstr(keyWords,"Opt"))
		{
			gchar* str = g_strdup_printf("%s.gab",fileNamePrefix);
			readGeometryFromGenericOutputFile(mol,fileNameOut);
			saveGeometry(mol, energy, str);
			read_geom_from_gabedit_geom_conv_file(str, 1);
			if(str) g_free(str);
			/* str = g_strdup_printf("Energy by OpenBabel = %f", energy);*/
			str = g_strdup_printf("Energy by %s = %f", genericCommand,energy);
			set_text_to_draw(str);
			drawGeom();
    			while( gtk_events_pending() ) gtk_main_iteration();
			Waiting(1);
		}
	}
	else
	{
		gchar* str = NULL;
		str = g_strdup_printf(
				(
				"Sorry, I cannot read the output file : %s "
				" ; Check also the installation of Generic..."
				),
				fileNameOut
				);
		/* printf(str);*/
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		if(str) g_free(str);
 		if(fileNameIn) free(fileNameIn);
 		if(fileNameOut) free(fileNameOut);
 		if(fileNameSH) free(fileNameSH);
		if(mol && newMolSE) freeMoleculeSE(mol);
		return FALSE;
	}

 	if(fileNameIn) free(fileNameIn);
 	if(fileNameOut) free(fileNameOut);
 	if(fileNameSH) free(fileNameSH);
	if(mol && newMolSE) freeMoleculeSE(mol);
	return TRUE;
}
/*****************************************************************************/
static void runMopacEnergy(GtkWidget* Win, gpointer data)
{

	G_CONST_RETURN gchar* model = gtk_entry_get_text(GTK_ENTRY(entryMopacHamiltonian));
	G_CONST_RETURN gchar* addKeys = gtk_entry_get_text(GTK_ENTRY(entryAddMopacKeywords));
	gchar* keys = g_strdup_printf("%s 1SCF %s",model,addKeys);
	/* printf("addKeys = %s\n",addKeys);*/
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "MopacEnergy", keys);
	if(keys) g_free(keys);
}
/*****************************************************************************/
static void runMopacOptimize(GtkWidget* Win, gpointer data)
{
	G_CONST_RETURN gchar* model = gtk_entry_get_text(GTK_ENTRY(entryMopacHamiltonian));
	G_CONST_RETURN gchar* addKeys = gtk_entry_get_text(GTK_ENTRY(entryAddMopacKeywords));
	gchar* keys = g_strdup_printf("%s XYZ AUX %s",model,addKeys);
	/* printf("addKeys = %s\n",addKeys);*/
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "MopacOptimize",keys);
	if(keys) g_free(keys);
}
/*****************************************************************************/
static void runMopacOptimizeSparkle(GtkWidget* Win, gpointer data)
{
	G_CONST_RETURN gchar* model = gtk_entry_get_text(GTK_ENTRY(entryMopacHamiltonianSparkle));
	G_CONST_RETURN gchar* addKeys = gtk_entry_get_text(GTK_ENTRY(entryAddMopacKeywords));
	gchar* keys = g_strdup_printf("%s SPARKLE XYZ AUX %s",model,addKeys);
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "MopacOptimizeSparkle",keys);
	if(keys) g_free(keys);
}
/*****************************************************************************/
static void runMopacESP(GtkWidget* Win, gpointer data)
{
	G_CONST_RETURN gchar* model = gtk_entry_get_text(GTK_ENTRY(entryMopacHamiltonian));
	G_CONST_RETURN gchar* addKeys = gtk_entry_get_text(GTK_ENTRY(entryAddMopacKeywords));
	gchar* keys = g_strdup_printf("%s ESP %s",model,addKeys);
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "MopacESP",keys);
	if(keys) g_free(keys);
}
/*****************************************************************************/
static void runMopacScan(GtkWidget* Win, gpointer data)
{
	G_CONST_RETURN gchar* model = gtk_entry_get_text(GTK_ENTRY(entryMopacHamiltonian));
	G_CONST_RETURN gchar* addKeys = gtk_entry_get_text(GTK_ENTRY(entryAddMopacKeywords));
	gchar buffer[BSIZE];
	gchar* keys = NULL;
	sprintf(buffer,"STEP=%g POINT=%d ",stepValueRP[0],numberOfPointsRP[0]);
	if(strcmp(typeRP[1],"Nothing"))
		sprintf(buffer,"STEP1=%g POINT1=%d STEP2=%g POINT2=%d",
				stepValueRP[0],numberOfPointsRP[0],
				stepValueRP[1],numberOfPointsRP[1]
				);
	keys = g_strdup_printf("%s 1SCF %s %s",model,buffer,addKeys);
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "MopacScan",keys);
	if(keys) g_free(keys);
}
/*****************************************************************************/
static void runOrcaEnergy(GtkWidget* Win, gpointer data)
{

	G_CONST_RETURN gchar* model = gtk_entry_get_text(GTK_ENTRY(entryOrcaHamiltonian));
	G_CONST_RETURN gchar* addKeys = gtk_entry_get_text(GTK_ENTRY(entryAddOrcaKeywords));
	gchar* keys = g_strdup_printf("%s %s",model,addKeys);
	/* printf("addKeys = %s\n",addKeys);*/
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "OrcaEnergy", keys);
	if(keys) g_free(keys);
}
/*****************************************************************************/
static void runOrcaOptimize(GtkWidget* Win, gpointer data)
{
	G_CONST_RETURN gchar* model = gtk_entry_get_text(GTK_ENTRY(entryOrcaHamiltonian));
	G_CONST_RETURN gchar* addKeys = gtk_entry_get_text(GTK_ENTRY(entryAddOrcaKeywords));
	gchar* keys = g_strdup_printf("%s Opt %s",model,addKeys);
	/* printf("addKeys = %s\n",addKeys);*/
	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	runSemiEmpirical(Win, data, "OrcaOptimize",keys);
	if(keys) g_free(keys);
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
/********************************************************************************/
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

  	//for(i=nlist-1;i>=0;i--) glist = g_list_append(glist,list[i]);
  	for(i=0;i<nlist;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboSpinMultiplicity, glist) ;
	gtk_combo_box_set_active(GTK_COMBO_BOX(comboSpinMultiplicity), nlist-1);
  	g_list_free(glist);
	SpinMultiplicities[0] = atoi(list[0]);
	if( SpinMultiplicities[0]%2 == atoi(list[0])%2) setSpinMultiplicityComboSpinMultiplicity(comboSpinMultiplicity, SpinMultiplicities[0]);

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

  	//for(i=nlist-1;i>=0;i--) glist = g_list_append(glist,list[i]);
  	for(i=0;i<nlist;i++) glist = g_list_append(glist,list[i]);

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
	gtk_widget_set_size_request(GTK_WIDGET(entry),(gint)(ScreenHeight*0.1),-1);

	return entry;
}
/**********************************************************************/
static GtkWidget *addChargeOfMoleculeToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryCharge = NULL;
	GtkWidget* comboCharge = NULL;
	gint nlistCharge = 1;
	gchar* listCharge[] = {"0"};

	add_label_table(table,_("Charge"),(gushort)i,0);
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

	add_label_table(table,_("Spin multiplicity"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entrySpinMultiplicity = addComboListToATable(table, listspinMultiplicity, nlistspinMultiplicity, i, 2, 1);
	comboSpinMultiplicity  = g_object_get_data(G_OBJECT (entrySpinMultiplicity), "Combo");
	gtk_widget_set_sensitive(entrySpinMultiplicity, FALSE);

	return comboSpinMultiplicity;
}
/**********************************************************************/
static void addChargeSpin(GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* comboSpinMultiplicity = NULL;
	GtkWidget* comboCharge = NULL;
	GtkWidget *table = NULL;
	gint i;

	totalCharge = TotalCharges[0];
	spinMultiplicity=SpinMultiplicities[0];

	table = gtk_table_new(2,5,FALSE);

	frame = gtk_frame_new (_("Charge & Multiplicity"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);
	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);
/*----------------------------------------------------------------------------------*/
	i = 0;
	comboCharge = addChargeOfMoleculeToTable(table, i);
/*----------------------------------------------------------------------------------*/
	i = 1;
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
/************************************************************************************************************/
static void setComboReactionPathVariableType(GtkWidget *comboReactionPathVariableType, gboolean nothing)
{
	GList *glist = NULL;
	gint i;
	gchar* listXYZ[] = {"Nothing","X","Y","Z"};
	gchar* listZMatrix[] = {"Nothing","Bond","Angle","Dihedral"};
	gchar** list = NULL;
	gint iBegin = 0;
	gint iEnd = 3;

	if(!nothing) iBegin = 1;
	if(MethodeGeom==GEOM_IS_XYZ) list = listXYZ;
	else list = listZMatrix;

	if(MethodeGeom!=GEOM_IS_XYZ)
	{
		if(NcentersZmat<=2) iEnd-=2;
		else if(NcentersZmat<=3) iEnd--;
	}

  	for(i=iBegin;i<=iEnd;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboReactionPathVariableType, glist) ;
  	g_list_free(glist);
}
/********************************************************************************/
static void setComboReactionPathAtoms(GtkWidget *comboReactionPathAtoms)
{
	GList *glist = NULL;
	gint i;
	gint nlist;
	gchar** list = NULL;
	GtkWidget* entry = NULL;
	gchar* typeRP   = NULL;
	gint iEnd;

	if(!comboReactionPathAtoms) return;
	entry = GTK_BIN (comboReactionPathAtoms)->child;
	if(!entry) return;
	typeRP   = g_object_get_data(G_OBJECT (entry), "TypeRP");
	if(!typeRP) return;

	nlist = NcentersXYZ;
	if(MethodeGeom!=GEOM_IS_XYZ) nlist = NcentersZmat;
	iEnd = nlist-1;
	if(nlist<1) return;

	list = g_malloc(nlist*sizeof(gchar*));
	if(!list) return;
	for(i=0;i<nlist;i++) list[i] = NULL;
	if(!strcmp(typeRP,"X") || !strcmp(typeRP,"Y") || !strcmp(typeRP,"Z"))
	{
		iEnd = nlist-1;
		for(i=0;i<=iEnd;i++)
			list[i] = g_strdup_printf("%d",i+1);
	}
	else if(!strcmp(typeRP,"Bond"))
	{
		iEnd = nlist-2;
		for(i=0;i<=iEnd;i++)
		{
			gint a = i+1;
			gint b = atoi(Geom[a].NR)-1;
			list[i] = g_strdup_printf("%d [%s%d-%s%d]",a+1,Geom[a].Symb,a+1,Geom[b].Symb,b+1);
		}
	}
	else if(!strcmp(typeRP,"Angle"))
	{
		iEnd = nlist-3;
		for(i=0;i<=iEnd;i++)
		{
			gint a = i+2;
			gint b = atoi(Geom[a].NR)-1;
			gint c = atoi(Geom[a].NAngle)-1;
			list[i] = g_strdup_printf("%d [%s%d-%s%d-%s%d]",a+1, 
					Geom[a].Symb,a+1,
					Geom[b].Symb,b+1,
					Geom[c].Symb,c+1);
		}
	}
	else if(!strcmp(typeRP,"Dihedral"))
	{
		iEnd = nlist-4;
		for(i=0;i<=iEnd;i++)
		{
			gint a = i+3;
			gint b = atoi(Geom[a].NR)-1;
			gint c = atoi(Geom[a].NAngle)-1;
			gint d = atoi(Geom[a].NDihedral)-1;
			list[i] = g_strdup_printf("%d [%s%d-%s%d-%s%d-%s%d]",a+1, 
					Geom[a].Symb,a+1,
					Geom[b].Symb,b+1,
					Geom[c].Symb,c+1,
					Geom[d].Symb,d+1
					);
		}
	}
	else 
	{
		iEnd = 0;
		list[0] = g_strdup_printf(" ");
	}

  	for(i=0;i<=iEnd;i++) glist = g_list_append(glist,list[i]);

  	gtk_combo_box_entry_set_popdown_strings( comboReactionPathAtoms, glist) ;
  	g_list_free(glist);
	if(list)
	{
		for(i=0;i<nlist;i++) if(list[i]) g_free(list[i]);
		g_free(list);
	}
}
/**********************************************************************/
static void changedEntryReactionPathType(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gchar* typeRP;
	GtkWidget* entryStep = NULL;
	GtkWidget* entryNbPoints = NULL;
	GtkWidget* comboAtoms = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;
	typeRP   = g_object_get_data(G_OBJECT (entry), "TypeRP");
	if(!typeRP) return;
	sprintf(typeRP,"%s",entryText);
	entryStep = g_object_get_data(G_OBJECT (entry), "EntryStep");
	entryNbPoints =	g_object_get_data(G_OBJECT (entry), "EntryNbPoints");
	if (!strcmp(typeRP,"X") || !strcmp(typeRP,"Y") || !strcmp(typeRP,"Z") || !strcmp(typeRP,"Bond"))
	{
		if(entryStep) gtk_entry_set_text(GTK_ENTRY(entryStep),"0.1");
		if(entryNbPoints) gtk_entry_set_text(GTK_ENTRY(entryNbPoints),"20");
	}
	else
	{
		if(entryStep) gtk_entry_set_text(GTK_ENTRY(entryStep),"1.0");
		if(entryNbPoints) gtk_entry_set_text(GTK_ENTRY(entryNbPoints),"360");
	}
	comboAtoms = g_object_get_data(G_OBJECT (entry), "ComboAtoms");
	if(comboAtoms)setComboReactionPathAtoms(comboAtoms);
}
/**********************************************************************/
static void changedEntryReactionPathAtoms(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gint* atomRP = NULL;
	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;
	atomRP   = g_object_get_data(G_OBJECT (entry), "AtomRP");
	if(!atomRP) return;
	sscanf(entryText,"%d",atomRP);
}
/**********************************************************************/
static void changedEntryReactionPathStep(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gdouble* stepValue = NULL;

	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;
	stepValue = g_object_get_data(G_OBJECT (entry), "StepValueRP");
	if(!stepValue) return;
	*stepValue = atof(entryText);
}
/**********************************************************************/
static void changedEntryReactionPathNbPoints(GtkWidget *entry, gpointer data)
{
	G_CONST_RETURN gchar* entryText = NULL;
	gint* numberOfPoints = NULL;

	 
	if(!GTK_IS_WIDGET(entry)) return;

	entryText = gtk_entry_get_text(GTK_ENTRY(entry));
	if(strlen(entryText)<1)return;
	if(atoi(entryText)<1) return;
	numberOfPoints = g_object_get_data(G_OBJECT (entry), "NumberOfPointsRP");
	if(!numberOfPoints) return;
	*numberOfPoints = atoi(entryText);
}
/***********************************************************************************************/
static GtkWidget *addMopacReactionPathVariableToTable(GtkWidget *table, gint i,gint j)
{
	GtkWidget* entry = NULL;
	GtkWidget* combo = NULL;
	gint nlist = 1;
	gchar* list[] = {"Bond"};

	entry = addComboListToATable(table, list, nlist, i, j, 1);
	combo  = g_object_get_data(G_OBJECT (entry), "Combo");
	gtk_widget_set_sensitive(entry, FALSE);
	g_signal_connect(G_OBJECT(entry),"changed", G_CALLBACK(changedEntryReactionPathType),NULL);

	return combo;
}
/***********************************************************************************************/
static GtkWidget *addMopacReactionPathAtomsToTable(GtkWidget *table, gint i, gint j)
{
	GtkWidget* entry = NULL;
	GtkWidget* combo = NULL;
	gint nlist = 1;
	gchar* list[] = {"1-2"};

	entry = addComboListToATable(table, list, nlist, i, j, 1);
	combo  = g_object_get_data(G_OBJECT (entry), "Combo");
	gtk_widget_set_sensitive(entry, FALSE);

	g_signal_connect(G_OBJECT(entry),"changed", G_CALLBACK(changedEntryReactionPathAtoms),NULL);
	return combo;
}
/***********************************************************************************************/
static void createReactionPathFrame(GtkWidget *box)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* label;
	GtkWidget* comboVariableType[2] = {NULL,NULL}; /* X, Y, Z, R, Angle or Dihedral */
	GtkWidget* comboAtoms[2] = {NULL,NULL}; /* n1-n2-n3-n4 (Symb1-Symb2-Symb3-Symb4)*/
	GtkWidget* entryNbPoints[2] = {NULL,NULL};
	GtkWidget* entryStep[2] = {NULL,NULL};
	GtkWidget *table = NULL;
	gint i;
	gint j;
	gint k;

	table = gtk_table_new(3,5,FALSE);

	sprintf(typeRP[1],"Nothing");
	if(MethodeGeom==GEOM_IS_XYZ) sprintf(typeRP[0],"X");
	else sprintf(typeRP[0],"Bond");

	numberOfPointsRP[0] = 10;
	numberOfPointsRP[1] = 10;
	stepValueRP[0] = 0.1;
	stepValueRP[1] = 0.1;

	frame = gtk_frame_new (_("Reaction path"));
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);

	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);

	i = 0; j = 1;
	label = gtk_label_new(_("Type"));
	gtk_table_attach(GTK_TABLE(table),label,j,j+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	i = 0; j = 2;
	label = gtk_label_new(_("Atoms"));
	gtk_table_attach(GTK_TABLE(table),label,j,j+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	i = 0; j = 3;
	label = gtk_label_new(_("Nb points"));
	gtk_table_attach(GTK_TABLE(table),label,j,j+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	i = 0; j = 4;
	label = gtk_label_new(_("Step value"));
	gtk_table_attach(GTK_TABLE(table),label,j,j+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);

	i = 1; 
	j = 0;
	label = gtk_label_new(_("First"));
	gtk_table_attach(GTK_TABLE(table),label,j,j+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	j = 1;
	comboVariableType[0] = addMopacReactionPathVariableToTable(table, i,j);
	j = 2;
	comboAtoms[0] = addMopacReactionPathAtomsToTable(table, i,j);
	j = 3;
	entryNbPoints[0] = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entryNbPoints[0]),(gint)(ScreenHeight*0.1),-1);
	gtk_table_attach(GTK_TABLE(table),entryNbPoints[0],j,j+1,i,i+1,
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  2,2);
	j = 4;
	entryStep[0] = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entryStep[0]),(gint)(ScreenHeight*0.1),-1);
	gtk_table_attach(GTK_TABLE(table),entryStep[0],j,j+1,i,i+1,
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  2,2);
	i = 2; 
	j = 0;
	label = gtk_label_new(_("Second"));
	gtk_table_attach(GTK_TABLE(table),label,j,j+1,i,i+1,
		(GtkAttachOptions)	(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)	(GTK_FILL | GTK_SHRINK),
                  2,2);
	j = 1;
	comboVariableType[1] = addMopacReactionPathVariableToTable(table, i,j);
	j = 2;
	comboAtoms[1] = addMopacReactionPathAtomsToTable(table, i,j);
	j = 3;
	entryNbPoints[1] = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entryNbPoints[1]),(gint)(ScreenHeight*0.1),-1);
	gtk_table_attach(GTK_TABLE(table),entryNbPoints[1],j,j+1,i,i+1,
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  2,2);
	j = 4;
	entryStep[1] = gtk_entry_new ();
	gtk_widget_set_size_request(GTK_WIDGET(entryStep[1]),(gint)(ScreenHeight*0.1),-1);
	gtk_table_attach(GTK_TABLE(table),entryStep[1],j,j+1,i,i+1,
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
		(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
                  2,2);

	for(k=0;k<2;k++) gtk_entry_set_text(GTK_ENTRY(entryStep[k]),"0.1");
	for(k=0;k<2;k++) gtk_entry_set_text(GTK_ENTRY(entryNbPoints[k]),"20");
	for(k=0;k<2;k++)
	if(GTK_IS_COMBO_BOX(comboVariableType[k]))
	{
		g_object_set_data(G_OBJECT (GTK_BIN(comboVariableType[k])->child), "ComboAtoms", comboAtoms[k]);
		g_object_set_data(G_OBJECT (GTK_BIN(comboVariableType[k])->child), "TypeRP", typeRP[k]);
		g_object_set_data(G_OBJECT (GTK_BIN(comboVariableType[k])->child), "EntryStep", entryStep[k]);
		g_object_set_data(G_OBJECT (GTK_BIN(comboVariableType[k])->child), "EntryNbPoints", entryNbPoints[k]);
	}
	for(k=0;k<2;k++)
	if(GTK_IS_COMBO_BOX(comboAtoms[k]))
	{
		g_object_set_data(G_OBJECT (GTK_BIN(comboAtoms[k])->child), "EntryStep", entryStep[k]);
		g_object_set_data(G_OBJECT (GTK_BIN(comboAtoms[k])->child), "EntryNbPoints", entryNbPoints[k]);
		g_object_set_data(G_OBJECT (GTK_BIN(comboAtoms[k])->child), "AtomRP", &atomRP[k]);
		g_object_set_data(G_OBJECT (GTK_BIN(comboAtoms[k])->child), "TypeRP", typeRP[k]);
	}
	for(k=0;k<2;k++) g_object_set_data(G_OBJECT (entryNbPoints[k]), "NumberOfPointsRP", &numberOfPointsRP[k]);
	for(k=0;k<2;k++) g_object_set_data(G_OBJECT (entryStep[k]), "StepValueRP", &stepValueRP[k]);

	for(k=0;k<2;k++) g_signal_connect(G_OBJECT(entryStep[k]),"changed", G_CALLBACK(changedEntryReactionPathStep),NULL);
	for(k=0;k<2;k++) g_signal_connect(G_OBJECT(entryNbPoints[k]),"changed", G_CALLBACK(changedEntryReactionPathNbPoints),NULL);

	for(k=0;k<2;k++) setComboReactionPathVariableType(comboVariableType[k],k>0);
	for(k=0;k<2;k++) setComboReactionPathAtoms(comboAtoms[k]);
}
/**********************************************************************/
static void addGenericOptions(GtkWidget *box, gchar* type)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* comboGenericPotential = NULL;
	GtkWidget* label = NULL;
	GtkWidget *table = NULL;
	gint i;
	gint j;

	table = gtk_table_new(2,5,FALSE);

	frame = gtk_frame_new (NULL);
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
	add_label_table(table,_("Name of your generic program"),(gushort)i,(gushort)j);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryGenericPotential = gtk_entry_new();
	gtk_widget_set_size_request(GTK_WIDGET(entryGenericPotential),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryGenericPotential, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	if(!genericProgName) genericProgName = g_strdup("myGenericProgram");
	gtk_entry_set_text(GTK_ENTRY(entryGenericPotential),genericProgName);
/*----------------------------------------------------------------------------------*/
}
/************************************************************************************************************/
static void AddOptionsDlg(GtkWidget *NoteBook, GtkWidget *win,gchar* type)
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

	LabelOnglet = gtk_label_new(_("General"));
	LabelMenu = gtk_label_new(_("General"));
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook), frame,LabelOnglet, LabelMenu);
	g_object_set_data (G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	table = gtk_table_new(4,5,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	i=-1;
/*----------------------------------------------------------------------------------*/
	if(!strstr(type,"OpenBabel"))
	{
		i++;
		j = 0;
		vbox = gtk_vbox_new (FALSE, 0);
		gtk_table_attach(GTK_TABLE(table),vbox,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
		addChargeSpin(vbox);
	}
/*----------------------------------------------------------------------------------*/
	if(strstr(type,"Generic"))
	{
		i++;
		j = 0;
		vbox = gtk_vbox_new (FALSE, 0);
		gtk_table_attach(GTK_TABLE(table),vbox,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
		addGenericOptions(vbox, type);
	}
	else if(strstr(type,"OpenBabel"))
	{
		i++;
		j = 0;
		vbox = gtk_vbox_new (FALSE, 0);
		gtk_table_attach(GTK_TABLE(table),vbox,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
		addOpenBabelOptions(vbox, type);
	}
	else if(strstr(type,"Orca"))
	{
		i++;
		j = 0;
		vbox = gtk_vbox_new (FALSE, 0);
		gtk_table_attach(GTK_TABLE(table),vbox,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
		addOrcaOptions(vbox, type);
	}
	else if(strstr(type,"Mopac") && strstr(type,"Sparkle"))
	{
		i++;
		j = 0;
		vbox = gtk_vbox_new (FALSE, 0);
		gtk_table_attach(GTK_TABLE(table),vbox, j,j+3,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
		addMopacSparkleOptions(vbox, type);
	}
	else
	if(strstr(type,"Mopac") && !strstr(type,"PM6") && !strstr(type,"AM1") && !strstr(type,"PM7"))
	{
		i++;
		j = 0;
		vbox = gtk_vbox_new (FALSE, 0);
		gtk_table_attach(GTK_TABLE(table),vbox,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
		addMopacOptions(vbox, type);
	}
	else if(strstr(type,"MopacScanAM1") || strstr(type,"MopacScanPM6") || strstr(type,"MopacScanPM6DH2") || strstr(type,"MopacScanPM6DH+") || strstr(type,"MopacScanPM7") )
	{
		i++;
		j = 0;
		vbox = gtk_vbox_new (FALSE, 0);
		gtk_table_attach(GTK_TABLE(table),vbox,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
		addMopacOptions(vbox, type);
	}
	if(strstr(type,"MopacScan"))
	{
		i++;
		j = 0;
		vbox = gtk_vbox_new (FALSE, 0);
		gtk_table_attach(GTK_TABLE(table),vbox,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
		createReactionPathFrame(vbox);
	}
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("Working Folder"),(gushort)i,(gushort)j);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	buttonDirSelector =  gabedit_dir_button();
	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER(buttonDirSelector), g_get_current_dir());
	gtk_widget_set_size_request(GTK_WIDGET(buttonDirSelector),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),buttonDirSelector,
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("File name"),(gushort)i,(gushort)j);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryFileName = gtk_entry_new();
	if(strstr(type,"AM1")) gtk_entry_set_text(GTK_ENTRY(entryFileName),"am1");
	if(strstr(type,"PM6")) gtk_entry_set_text(GTK_ENTRY(entryFileName),"pm6");
	if(strstr(type,"PM7")) gtk_entry_set_text(GTK_ENTRY(entryFileName),"pm7");
	else if(strstr(type,"Mopac")) gtk_entry_set_text(GTK_ENTRY(entryFileName),"mopacFile");
	else if(strstr(type,"FireFly")) gtk_entry_set_text(GTK_ENTRY(entryFileName),"fireflyFile");
	else if(strstr(type,"Orca")) gtk_entry_set_text(GTK_ENTRY(entryFileName),"orcaFile");
	else gtk_entry_set_text(GTK_ENTRY(entryFileName),"myFile");
	gtk_widget_set_size_request(GTK_WIDGET(entryFileName),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryFileName, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);

/*----------------------------------------------------------------------------------*/
}
/**********************************************************************/
static GtkWidget *addMopacHamiltonianToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryMopacHamiltonian = NULL;
	GtkWidget* comboMopacHamiltonian = NULL;
	gint nlistHamiltonian = 7;
	gchar* listHamiltonian[] = {"PM7","PM6","PM6-DH+","PM6-DH2","AM1","RM1","PM3","MNDO"};

	add_label_table(table,_("Model"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryMopacHamiltonian = addComboListToATable(table, listHamiltonian, nlistHamiltonian, i, 2, 1);
	comboMopacHamiltonian  = g_object_get_data(G_OBJECT (entryMopacHamiltonian), "Combo");
	gtk_widget_set_sensitive(entryMopacHamiltonian, FALSE);


	return comboMopacHamiltonian;
}
/**********************************************************************/
static void addMopacOptions(GtkWidget *box, gchar* type)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* comboMopacHamiltonian = NULL;
	GtkWidget* label = NULL;
	GtkWidget *table = NULL;
	gint i;
	gint j;

	table = gtk_table_new(2,5,FALSE);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);
	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);
/*----------------------------------------------------------------------------------*/
	i = 0;
	comboMopacHamiltonian = addMopacHamiltonianToTable(table, i);
	entryMopacHamiltonian = GTK_WIDGET(GTK_BIN(comboMopacHamiltonian)->child);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("Additional keywords"),(gushort)i,(gushort)j);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryAddMopacKeywords = gtk_entry_new();
	if(strstr(type,"Opt")) gtk_entry_set_text(GTK_ENTRY(entryAddMopacKeywords),"GNORM=0.01");
	else if(strstr(type,"Gradient")) gtk_entry_set_text(GTK_ENTRY(entryAddMopacKeywords),"GNORM=0.01");
	else if(strstr(type,"Scan")) gtk_entry_set_text(GTK_ENTRY(entryAddMopacKeywords),"GEO-OK EPS=78.39 RSOLV=1.3");
	else gtk_entry_set_text(GTK_ENTRY(entryAddMopacKeywords),"BONDS");
	gtk_widget_set_size_request(GTK_WIDGET(entryAddMopacKeywords),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryAddMopacKeywords, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	if(strstr(type,"MopacScanAM1")) gtk_entry_set_text(GTK_ENTRY(entryMopacHamiltonian),"AM1");
/*----------------------------------------------------------------------------------*/
}
/**********************************************************************/
static GtkWidget *addMopacHamiltonianSparkleToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryMopacHamiltonianSparkle = NULL;
	GtkWidget* comboMopacHamiltonianSparkle = NULL;
	gint nlistHamiltonianSparkle = 4;
	gchar* listHamiltonianSparkle[] = {"AM1","PM7","PM6","PM3"};

	add_label_table(table,_("Model"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryMopacHamiltonianSparkle = addComboListToATable(table, listHamiltonianSparkle, nlistHamiltonianSparkle, i, 2, 1);
	comboMopacHamiltonianSparkle  = g_object_get_data(G_OBJECT (entryMopacHamiltonianSparkle), "Combo");
	gtk_widget_set_sensitive(entryMopacHamiltonianSparkle, FALSE);

	return comboMopacHamiltonianSparkle;
}
/**********************************************************************/
static void addMopacSparkleOptions(GtkWidget *box, gchar* type)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* comboMopacHamiltonianSparkle = NULL;
	GtkWidget* label = NULL;
	GtkWidget *table = NULL;
	gint i;
	gint j;

	table = gtk_table_new(2,5,FALSE);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);
	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);
/*----------------------------------------------------------------------------------*/
	i = 0;
	comboMopacHamiltonianSparkle = addMopacHamiltonianSparkleToTable(table, i);
	entryMopacHamiltonianSparkle = GTK_WIDGET(GTK_BIN(comboMopacHamiltonianSparkle)->child);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("Additional keywords"),(gushort)i,(gushort)j);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryAddMopacKeywords = gtk_entry_new();
	if(strstr(type,"Opt")) gtk_entry_set_text(GTK_ENTRY(entryAddMopacKeywords),"GNORM=0.01");
	else if(strstr(type,"Gradient")) gtk_entry_set_text(GTK_ENTRY(entryAddMopacKeywords),"GNORM=0.01");
	else if(strstr(type,"Scan")) gtk_entry_set_text(GTK_ENTRY(entryAddMopacKeywords),"GEO-OK EPS=78.39 RSOLV=1.3");
	else gtk_entry_set_text(GTK_ENTRY(entryAddMopacKeywords),"BONDS");
	gtk_widget_set_size_request(GTK_WIDGET(entryAddMopacKeywords),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryAddMopacKeywords, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	if(strstr(type,"MopacScanAM1")) gtk_entry_set_text(GTK_ENTRY(entryMopacHamiltonian),"AM1");
/*----------------------------------------------------------------------------------*/
}
/**********************************************************************/
static GtkWidget *addOrcaHamiltonianToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryOrcaHamiltonian = NULL;
	GtkWidget* comboOrcaHamiltonian = NULL;
	gint nlistHamiltonian = 2;
	gchar* listHamiltonian[] = {"PM3","AM1"};

	add_label_table(table,_("Model"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryOrcaHamiltonian = addComboListToATable(table, listHamiltonian, nlistHamiltonian, i, 2, 1);
	comboOrcaHamiltonian  = g_object_get_data(G_OBJECT (entryOrcaHamiltonian), "Combo");
	gtk_widget_set_sensitive(entryOrcaHamiltonian, FALSE);


	return comboOrcaHamiltonian;
}
/**********************************************************************/
static void addOrcaOptions(GtkWidget *box, gchar* type)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* comboOrcaHamiltonian = NULL;
	GtkWidget* label = NULL;
	GtkWidget *table = NULL;
	gint i;
	gint j;

	table = gtk_table_new(2,5,FALSE);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);
	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);
/*----------------------------------------------------------------------------------*/
	i = 0;
	comboOrcaHamiltonian = addOrcaHamiltonianToTable(table, i);
	entryOrcaHamiltonian = GTK_WIDGET(GTK_BIN(comboOrcaHamiltonian)->child);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("Additional keywords"),(gushort)i,(gushort)j);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryAddOrcaKeywords = gtk_entry_new();
	gtk_widget_set_size_request(GTK_WIDGET(entryAddOrcaKeywords),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryAddOrcaKeywords, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_entry_set_text(GTK_ENTRY(entryOrcaHamiltonian),"PM3");
/*----------------------------------------------------------------------------------*/
}
/**********************************************************************/
static GtkWidget *addOpenBabelPotentialToTable(GtkWidget *table, gint i)
{
	GtkWidget* entryOpenBabelPotential = NULL;
	GtkWidget* comboOpenBabelPotential = NULL;
	gint nlistPotential = 4;
	gchar* listPotential[] = {"MMFF94","MMFF94s","UFF", "GHEMICAL"};

	add_label_table(table,_("Model"),(gushort)i,0);
	add_label_table(table,":",(gushort)i,1);
	entryOpenBabelPotential = addComboListToATable(table, listPotential, nlistPotential, i, 2, 1);
	comboOpenBabelPotential  = g_object_get_data(G_OBJECT (entryOpenBabelPotential), "Combo");
	gtk_widget_set_sensitive(entryOpenBabelPotential, FALSE);

	return comboOpenBabelPotential;
}
/**********************************************************************/
static void addOpenBabelOptions(GtkWidget *box, gchar* type)
{
	GtkWidget* frame;
	GtkWidget* vboxFrame;
	GtkWidget* comboOpenBabelPotential = NULL;
	GtkWidget* label = NULL;
	GtkWidget *table = NULL;
	gint i;
	gint j;

	table = gtk_table_new(2,5,FALSE);

	frame = gtk_frame_new (NULL);
	gtk_widget_show (frame);
	gtk_box_pack_start (GTK_BOX (box), frame, TRUE, TRUE, 3);
	gtk_frame_set_label_align (GTK_FRAME (frame), 0.5, 0.5);

	vboxFrame = gtk_vbox_new (FALSE, 3);
	gtk_widget_show (vboxFrame);
	gtk_container_add (GTK_CONTAINER (frame), vboxFrame);
	gtk_box_pack_start (GTK_BOX (vboxFrame), table, TRUE, TRUE, 0);
/*----------------------------------------------------------------------------------*/
	i = 0;
	comboOpenBabelPotential = addOpenBabelPotentialToTable(table, i);
	entryOpenBabelPotential = GTK_WIDGET(GTK_BIN(comboOpenBabelPotential)->child);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	add_label_table(table,_("Additional options"),(gushort)i,(gushort)j);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryAddOpenBabelKeywords = gtk_entry_new();
	gtk_widget_set_size_request(GTK_WIDGET(entryAddOpenBabelKeywords),(gint)(ScreenHeight*0.2),-1);
	gtk_table_attach(GTK_TABLE(table),entryAddOpenBabelKeywords, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_entry_set_text(GTK_ENTRY(entryOpenBabelPotential),"MMFF94");
/*----------------------------------------------------------------------------------*/
}
/***********************************************************************/
void semiEmpiricalDlg(gchar* type)
{
	GtkWidget *button;
	GtkWidget *Win;
	GtkWidget *NoteBook;
	GtkWidget *parentWindow = GeomDlg;

	StopCalcul = TRUE;

	Win= gtk_dialog_new ();
	gtk_window_set_position(GTK_WINDOW(Win),GTK_WIN_POS_CENTER);
	gtk_window_set_transient_for(GTK_WINDOW(Win),GTK_WINDOW(parentWindow));
	if(type) 
	{
		gchar* title = g_strdup_printf(_("%s calculation"),type);
		gtk_window_set_title(&GTK_DIALOG(Win)->window,title);
		g_free(title);
	}
    	gtk_window_set_modal (GTK_WINDOW (Win), TRUE);

	g_signal_connect(G_OBJECT(Win),"delete_event",(GCallback)gtk_widget_destroy,NULL);
 
	NoteBook = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX (GTK_DIALOG(Win)->vbox), NoteBook,TRUE, TRUE, 0);

	AddOptionsDlg(NoteBook, Win,type);
  

	gtk_widget_realize(Win);

	button = create_button(Win,"Cancel");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", G_CALLBACK(gtk_widget_destroy),GTK_OBJECT(Win));
	gtk_widget_show (button);

	button = create_button(Win,"Ok");
	GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
	gtk_box_pack_start (GTK_BOX( GTK_DIALOG(Win)->action_area), button, TRUE, TRUE, 0);
	if(!strcmp(type,"AM1FireFlyEnergy"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runAM1FireFlyEnergy,GTK_OBJECT(Win));
	else if(!strcmp(type,"AM1FireFlyOptimize"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runAM1FireFlyOptimize,GTK_OBJECT(Win));

	else if(!strcmp(type,"OpenBabelEnergy"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runOpenBabelEnergy,GTK_OBJECT(Win));
	else if(!strcmp(type,"OpenBabelOptimize"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runOpenBabelOptimize,GTK_OBJECT(Win));

	else if(!strcmp(type,"GenericEnergy"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runGenericEnergy,GTK_OBJECT(Win));
	else if(!strcmp(type,"GenericOptimize"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runGenericOptimize,GTK_OBJECT(Win));

	else if(!strcmp(type,"PM6DH2MopacEnergy"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPM6DH2MopacEnergy,GTK_OBJECT(Win));
	else if(!strcmp(type,"PM6DH2MopacOptimize"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPM6DH2MopacOptimize,GTK_OBJECT(Win));
	else if(!strcmp(type,"PM6DH2MopacESP"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPM6DH2MopacESP,GTK_OBJECT(Win));
	else if(!strcmp(type,"PM6DH+MopacEnergy"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPM6DHpMopacEnergy,GTK_OBJECT(Win));
	else if(!strcmp(type,"PM6DH+MopacOptimize"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPM6DHpMopacOptimize,GTK_OBJECT(Win));
	else if(!strcmp(type,"PM6DH+MopacESP"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPM6DHpMopacESP,GTK_OBJECT(Win));

	else if(!strcmp(type,"PM6MopacEnergy"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPM6MopacEnergy,GTK_OBJECT(Win));
	else if(!strcmp(type,"PM6MopacOptimize"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPM6MopacOptimize,GTK_OBJECT(Win));
	else if(!strcmp(type,"PM6MopacESP"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPM6MopacESP,GTK_OBJECT(Win));

	else if(!strcmp(type,"PM7MopacEnergy"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPM7MopacEnergy,GTK_OBJECT(Win));
	else if(!strcmp(type,"PM7MopacOptimize"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPM7MopacOptimize,GTK_OBJECT(Win));
	else if(!strcmp(type,"PM7MopacESP"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runPM7MopacESP,GTK_OBJECT(Win));

	else if(!strcmp(type,"AM1MopacEnergy"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runAM1MopacEnergy,GTK_OBJECT(Win));
	else if(!strcmp(type,"AM1MopacOptimize"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runAM1MopacOptimize,GTK_OBJECT(Win));
	else if(!strcmp(type,"AM1MopacESP"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runAM1MopacESP,GTK_OBJECT(Win));
	else if(!strcmp(type,"MopacEnergy"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runMopacEnergy,GTK_OBJECT(Win));
	else if(!strcmp(type,"MopacOptimize"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runMopacOptimize,GTK_OBJECT(Win));
	else if(!strcmp(type,"MopacOptimizeSparkle"))
                g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runMopacOptimizeSparkle,GTK_OBJECT(Win));
	else if(!strcmp(type,"MopacESP"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runMopacESP,GTK_OBJECT(Win));
	else if(strstr(type,"MopacScan")!=NULL)
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runMopacScan,GTK_OBJECT(Win));
	else if(!strcmp(type,"OrcaEnergy"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runOrcaEnergy,GTK_OBJECT(Win));
	else if(!strcmp(type,"OrcaOptimize"))
		g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)runOrcaOptimize,GTK_OBJECT(Win));

	gtk_widget_show (button);

	gtk_widget_show_all(Win);
}
/*****************************************************************************/
static gboolean saveConfoGeometries(gint numberOfGeometries, SemiEmpiricalModel** geometries, gdouble* energies, gchar* fileNameGeom)
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

 	file = FOpen(fileNameGeom, "w");

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
	fprintf(file,"[GEOMS] 1\n"); /* for format # 1 */
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
		fprintf(file,"%d %d %d\n",geometries[i]->molecule.nAtoms,
		geometries[i]->molecule.totalCharge, geometries[i]->molecule.spinMultiplicity);
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
/*
		fprintf(file," %s %s %s %s %d %f %d %f %f %f\n", 
				geometries[i]->molecule.atoms[j].prop.symbol,
				geometries[i]->molecule.atoms[j].mmType,
				geometries[i]->molecule.atoms[j].pdbType,
				geometries[i]->molecule.atoms[j].residueName,
				geometries[i]->molecule.atoms[j].residueNumber,
				geometries[i]->molecule.atoms[j].charge,
				geometries[i]->molecule.atoms[j].layer,
				geometries[i]->molecule.atoms[j].coordinates[0],
				geometries[i]->molecule.atoms[j].coordinates[1],
				geometries[i]->molecule.atoms[j].coordinates[2]
				);
*/
		}
	}
	fclose(file);
	return TRUE;

}
/*****************************************************************************/
static gboolean runOneOptMopac(SemiEmpiricalModel* geom, gdouble* energy, gchar* fileNamePrefix, gchar* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gint j;
	gchar* fileNameIn = NULL;
	gchar* fileNameOut = NULL;
	gchar* fileNameSH = NULL;
	gchar multiplicityStr[100];
	gchar buffer[1024];
	*energy = 0;
#ifdef G_OS_WIN32
	gchar c='%';
#endif

	if(!geom) return FALSE;
	if(geom->molecule.nAtoms<1) return FALSE;
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%sMopacOne.sh",fileNamePrefix);
#else
	fileNameSH = g_strdup_printf("%sMopacOne.bat",fileNamePrefix);
#endif
 	fileSH = FOpen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"set PATH=%cPATH%c;\"%s\"\n",c,c,mopacDirectory);
#endif

	getMultiplicityName(spinMultiplicity, multiplicityStr);

	fileNameIn = g_strdup_printf("%sOne.mop",fileNamePrefix);
 	file = FOpen(fileNameIn, "w");
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
	if(spinMultiplicity>1)
	fprintf(file,"%s UHF CHARGE=%d %s\n",keyWords,totalCharge,multiplicityStr);
	else
	fprintf(file,"%s CHARGE=%d %s\n",keyWords,totalCharge,multiplicityStr);
	fprintf(file,"\n");
	fprintf(file,"Mopac file generated by Gabedit\n");

	for(j=0;j<geom->molecule.nAtoms;j++)
	{
	fprintf(file," %s %f %d %f %d %f %d\n", 
			geom->molecule.atoms[j].prop.symbol,
			geom->molecule.atoms[j].coordinates[0],
			geom->molecule.atoms[j].variable,
			geom->molecule.atoms[j].coordinates[1],
			geom->molecule.atoms[j].variable,
			geom->molecule.atoms[j].coordinates[2],
			geom->molecule.atoms[j].variable
			);
	}
	fclose(file);
	{
		gchar* str = NULL;
		if(strstr(keyWords,"XYZ") && strstr(keyWords,"PM7")) str = g_strdup_printf("Minimization by PM7/Mopac ... Please wait");
		else if(strstr(keyWords,"XYZ") && strstr(keyWords,"PM6-DH2")) str = g_strdup_printf("Minimization by PM6-DH2/Mopac ... Please wait");
		else if(strstr(keyWords,"XYZ") && strstr(keyWords,"PM6-DH+")) str = g_strdup_printf("Minimization by PM6-DH+/Mopac ... Please wait");
		else if(strstr(keyWords,"SPARKLE") && strstr(keyWords,"PM6")) str = g_strdup_printf("Minimization by Sparkle/PM6/Mopac ... Please wait");
		else if(strstr(keyWords,"SPARKLE") && strstr(keyWords,"AM1")) str = g_strdup_printf("Minimization by Sparkle/AM1/Mopac ... Please wait");
		else if(strstr(keyWords,"SPARKLE") && strstr(keyWords,"PM3")) str = g_strdup_printf("Minimization by Sparkle/PM3/Mopac ... Please wait");
		else if(strstr(keyWords,"XYZ") && strstr(keyWords,"PM6")) str = g_strdup_printf("Minimization by PM6/Mopac ... Please wait");
		else if(strstr(keyWords,"XYZ") && strstr(keyWords,"AM1")) str = g_strdup_printf("Minimization by AM1/Mopac ... Please wait");
		else if(strstr(keyWords,"ESP") && strstr(keyWords,"PM6-DH2")) str = g_strdup_printf("ESP charges from PM6-DH2/Mopac ... Please wait");
		else if(strstr(keyWords,"ESP") && strstr(keyWords,"PM6-DH+")) str = g_strdup_printf("ESP charges from PM6-DH+/Mopac ... Please wait");
		else if(strstr(keyWords,"ESP") && strstr(keyWords,"PM6")) str = g_strdup_printf("ESP charges from PM6/Mopac ... Please wait");
		else if(strstr(keyWords,"ESP") && strstr(keyWords,"PM7")) str = g_strdup_printf("ESP charges from PM7/Mopac ... Please wait");
		else if(strstr(keyWords,"ESP") && strstr(keyWords,"AM1")) str = g_strdup_printf("ESP charges from AM1/Mopac ... Please wait");
		else if(strstr(keyWords,"PM6-DH2")) str = g_strdup_printf("Computing of energy by PM6-DH2/Mopac .... Please wait");
		else if(strstr(keyWords,"PM6-DH+")) str = g_strdup_printf("Computing of energy by PM6-DH+/Mopac .... Please wait");
		else if(strstr(keyWords,"PM6")) str = g_strdup_printf("Computing of energy by PM6/Mopac .... Please wait");
		else if(strstr(keyWords,"PM7")) str = g_strdup_printf("Computing of energy by PM7/Mopac .... Please wait");
		else str = g_strdup_printf("Computing of energy by AM1/Mopac .... Please wait");
		set_text_to_draw(str);
		if(str) g_free(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
	}
#ifndef G_OS_WIN32
	fprintf(fileSH,"%s %s\n",NameCommandMopac,fileNameIn);
	fclose(fileSH);
	sprintf(buffer,"chmod u+x %s",fileNameSH);
	{int ierr= system(buffer);}
	{int ierr = system(fileNameSH);}
#else
	fprintf(fileSH,"\"%s\" \"%s\"\n",NameCommandMopac,fileNameIn);
	fclose(fileSH);
	sprintf(buffer,"\"%s\"",fileNameSH);
	{int ierr= system(buffer);}
#endif

	fileNameOut = g_strdup_printf("%sOne.out",fileNamePrefix);
	if(getEnergyMopac(fileNameOut,energy))
	{
		gchar* str = NULL;

		read_geom_from_mopac_output_file(fileNameOut, -1);
		str = g_strdup_printf("Energy by Mopac = %f", *energy);
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		Waiting(1);
		if(str) g_free(str);
	}
	else
	{
		printf("I cannot read energy = from %s file\n",fileNameOut);
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
static gboolean runMopacFiles(gint numberOfGeometries, SemiEmpiricalModel** geometries, gdouble* energies, gchar* fileNamePrefix, gchar* keyWords)
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
		str = g_strdup_printf("Minimization by Mopac of geometry n = %d... Please wait", i+1);
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		if(runOneOptMopac(geometries[i], &energies[i], fileNamePrefix, keyWords)) 
		{
			freeMoleculeSE(&geometries[i]->molecule);
			/*
			geometries[i]->molecule = createMoleculeSE(geometry0,Natoms,
			geometries[i]->molecule.totalCharge,
			geometries[i]->molecule.spinMultiplicity);
			*/
			geometries[i]->molecule = createFromGeomXYZMoleculeSE(
			geometries[i]->molecule.totalCharge,
			geometries[i]->molecule.spinMultiplicity,TRUE);
			nM++;
		}
		if(StopCalcul) break;
	}
	if(str) g_free(str);
	if(nM==nG) return TRUE;
	return FALSE;

}
/*****************************************************************************/
static gboolean runOneOptGeneric(SemiEmpiricalModel* geom, double* energy, char* fileNamePrefix, char* keyWords, char* genericCommand)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	char* fileNameIn = NULL;
	char* fileNameOut = NULL;
	char* fileNameSH = NULL;
	char buffer[1024];
	int type = 0;
	MoleculeSE* mol = &geom->molecule;
	*energy = 0;
#ifdef OS_WIN32
	char c='%';
#endif

	if(!geom) return FALSE;
	if(geom->molecule.nAtoms<1) return FALSE;
#ifndef OS_WIN32
	fileNameSH = g_strdup_printf("%sGeneOne.sh",fileNamePrefix);
#else
	fileNameSH = g_strdup_printf("%sGeneOne.bat",fileNamePrefix);
#endif
 	fileSH = fopen(fileNameSH, "w");
	if(!fileSH) return FALSE;

	fileNameIn = g_strdup_printf("%sOne.inp",fileNamePrefix);
	fileNameOut = g_strdup_printf("%sOne.out",fileNamePrefix);

 	file = fopen(fileNameIn, "w");
	if(!file) 
	{
 		if(fileNameIn) free(fileNameIn);
 		if(fileNameOut) free(fileNameOut);
 		if(fileNameSH) free(fileNameSH);
		return FALSE;
	}
 	file = fopen(fileNameIn, "w");
	if(!file) 
	{
 		if(fileNameIn) free(fileNameIn);
 		if(fileNameOut) free(fileNameOut);
 		if(fileNameSH) free(fileNameSH);
		return FALSE;
	}
	if(strstr(keyWords,"Opt")) type = 2;
	if(strstr(keyWords,"ENGRAD")) type = 1;
	fprintf(file,"%d\n",type);
	addMoleculeSEToFile(mol,file);
	fclose(file);
	{
		char* str = NULL;
		if(strstr(keyWords,"OPT")) str = g_strdup_printf("Minimization by Generic/%s ... Please wait",genericCommand);
		else str = g_strdup_printf("Computing of energy by Generic/%s .... Please wait",genericCommand);
		/*printf(str);*/
		set_text_to_draw(str);
		if(str) g_free(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
	}
#ifndef OS_WIN32
	fprintf(fileSH,"%s %s %s",genericCommand,fileNameIn,fileNameOut);
	fclose(fileSH);
	sprintf(buffer,"chmod u+x %s",fileNameSH);
	system(buffer);
	system(fileNameSH);
#else
	fprintf(fileSH,"\"%s\" \"%s\" \"%s\"",genericCommand,fileNameIn,fileNameOut);
	fclose(fileSH);
	sprintf(buffer,"\"%s\"",fileNameSH);
	system(buffer);
#endif
	if(getEnergyGeneric(fileNameOut,energy))
	{
		printf("Energy by Generic = %f\n", *energy);
		readGeometryFromGenericOutputFile(mol,fileNameOut);
	}
	else
	{
 		if(fileNameIn) free(fileNameIn);
 		if(fileNameOut) free(fileNameOut);
 		if(fileNameSH) free(fileNameSH);
		return FALSE;
	}

 	if(fileNameIn) free(fileNameIn);
 	if(fileNameOut) free(fileNameOut);
 	if(fileNameSH) free(fileNameSH);
	return TRUE;
}
/*****************************************************************************/
static gboolean runGenericFiles(int numberOfGeometries, SemiEmpiricalModel** geometries, double* energies, char* fileNamePrefix, char* keyWords, char* genericCommand)
{
	int i;
	int nG = 0;
	int nM = 0;
	char* str = NULL;
	for(i=0;i<numberOfGeometries;i++)
	{
		if(!geometries[i]) continue;
		nG++;
		if(str) free(str);
		printf("Minimization by Generic of geometry n = %d... Please wait\n", i+1);
		if(runOneOptGeneric(geometries[i], &energies[i], fileNamePrefix, keyWords, genericCommand)) 
		{
			nM++;
		}
	}
	if(str) free(str);
	if(nM==nG) return TRUE;
	return FALSE;

}
/*****************************************************************************/
static gboolean runOneOptFireFly(SemiEmpiricalModel* geom, gdouble* energy, gchar* fileNamePrefix, gchar* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gint j;
	gchar* fileNameIn = NULL;
	gchar* fileNameOut = NULL;
	gchar* fileNameSH = NULL;
	gchar multiplicityStr[100];
	gchar buffer[1024];
	*energy = 0;
#ifdef G_OS_WIN32
	gchar c='%';
#endif

	if(!geom) return FALSE;
	if(geom->molecule.nAtoms<1) return FALSE;
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%sPCGOne.sh",fileNamePrefix);
#else
	fileNameSH = g_strdup_printf("%sPCGOne.bat",fileNamePrefix);
#endif
 	fileSH = FOpen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"set PATH=%cPATH%c;\"%s\"\n",c,c,fireflyDirectory);
#endif

	getMultiplicityName(spinMultiplicity, multiplicityStr);

	fileNameIn = g_strdup_printf("%sOne.inp",fileNamePrefix);
 	file = FOpen(fileNameIn, "w");
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
	for(j=0;j<geom->molecule.nAtoms;j++)
	{
		gchar* symbol = geom->molecule.atoms[j].prop.symbol;
		SAtomsProp prop = prop_atom_get(symbol);
		fprintf(file,"%s %f %f %f %f\n", 
			symbol,
			(gdouble)prop.atomicNumber,
			geom->molecule.atoms[j].coordinates[0],
			geom->molecule.atoms[j].coordinates[1],
			geom->molecule.atoms[j].coordinates[2]
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
	{
		gchar* str = NULL;
		if(strstr(keyWords,"OPTIMIZE")) str = g_strdup_printf("Minimization by AM1/FireFly ... Please wait");
		else str = g_strdup_printf("Computing of energy by AM1/FireFly .... Please wait");
		set_text_to_draw(str);
		if(str) g_free(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
	}
#ifndef G_OS_WIN32
	sprintf(buffer,"chmod u+x %s",fileNameSH);
	{int ierr= system(buffer);}
	{int ierr = system(fileNameSH);}
#else
	sprintf(buffer,"\"%s\"",fileNameSH);
	{int ierr= system(buffer);}
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
static gboolean runFireFlyFiles(gint numberOfGeometries, SemiEmpiricalModel** geometries, gdouble* energies, gchar* fileNamePrefix, gchar* keyWords)
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
		if(runOneOptFireFly(geometries[i], &energies[i], fileNamePrefix, keyWords)) 
		{
			freeMoleculeSE(&geometries[i]->molecule);
			/*
			geometries[i]->molecule = createMoleculeSE(geometry0,Natoms,
			geometries[i]->molecule.totalCharge,
			geometries[i]->molecule.spinMultiplicity);
			*/
			geometries[i]->molecule = createFromGeomXYZMoleculeSE(
			geometries[i]->molecule.totalCharge,
			geometries[i]->molecule.spinMultiplicity,TRUE);
			nM++;
		}
		if(StopCalcul) break;
	}
	if(str) g_free(str);
	if(nM==nG) return TRUE;
	return FALSE;

}
/*************************************************************************************************************************************************/
static gboolean runOpenBabelFiles(gint numberOfGeometries, SemiEmpiricalModel** geometries, gdouble* energies, gchar* fileNamePrefix, gchar* keyWords)
{
        int i;
        int nG = 0;
        int nM = 0;
	gchar* str = NULL;
        if(!geometries) return FALSE;
        for(i=0;i<numberOfGeometries;i++)
        {
                if(!geometries[i]) continue;
                nG++;
		if(str) g_free(str);
		str = g_strdup_printf("Minimization by OpenBabel of geometry n = %d... Please wait", i+1);
		set_text_to_draw(str);
		drawGeom();
    		while( gtk_events_pending() ) gtk_main_iteration();
		printf("str runOpenBabelFiles = %s\n",str);
		//setConnectionsMoleculeSE(&geometries[i]->molecule);
		//printf("End setConnectionsMoleculeSE\n");
                if(runOneOpenBabel(&geometries[i]->molecule, fileNamePrefix, keyWords))
                {
                        energies[i] = geometries[i]->molecule.energy;
                        nM++;
                }
		if(StopCalcul) break;
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
static gdouble* getDistancesBetweenAtoms(SemiEmpiricalModel* seModel)
{
	gdouble* distances = NULL;
	gint i;
	gint j;
	gint n;
	gint k;
	if(seModel->molecule.nAtoms<1) return distances;
	n = seModel->molecule.nAtoms*(seModel->molecule.nAtoms-1)/2;
	distances = g_malloc(n*sizeof(gdouble));
	n = 0;
	for (  i = 0; i < seModel->molecule.nAtoms-1; i++ )
	for (  j = i+1; j < seModel->molecule.nAtoms; j++ )
	{
		gdouble x = seModel->molecule.atoms[i].coordinates[0]-seModel->molecule.atoms[j].coordinates[0];
		gdouble y = seModel->molecule.atoms[i].coordinates[1]-seModel->molecule.atoms[j].coordinates[1];
		gdouble z = seModel->molecule.atoms[i].coordinates[2]-seModel->molecule.atoms[j].coordinates[2];
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
static void removedsToEnd(gint numberOfGeometries, SemiEmpiricalModel** geometries, gdouble* energies, gboolean* removeds)
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
				SemiEmpiricalModel* g = geometries[i];

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
static void computeRemoveds(gint numberOfGeometries, SemiEmpiricalModel** geometries, gdouble* energies, gboolean *removeds, 
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
static void removeIdenticalGeometries(gint* nG, SemiEmpiricalModel*** geoms, gdouble** eners, gdouble tolEnergy, gdouble tolDistance)
{
	gint i;
	gint numberOfGeometries =*nG;
	SemiEmpiricalModel** geometries = *geoms; 
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
			if(geometries[i]) freeSemiEmpiricalModel(geometries[i]);
		}
		else newN++;
	}
	if(newN==0) newN = 1;
	if(newN==numberOfGeometries) return;
	*nG = newN;
	*eners = g_realloc(*eners,newN*sizeof(gdouble));
	*geoms = g_realloc(*geoms,newN*sizeof(SemiEmpiricalModel**));

	g_free(removeds);
}
/*****************************************************************************/
static void sortGeometries(gint numberOfGeometries, SemiEmpiricalModel** geometries, gdouble* energies)
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
				SemiEmpiricalModel* g = geometries[i];

				energies[i] = energies[k];
				energies[k] = energy;
				geometries[i] = geometries[k];
				geometries[k] = g;
			}
		}
	}
}
/*****************************************************************************/
static gboolean createMopacFiles(gint numberOfGeometries, SemiEmpiricalModel** geometries, gdouble* energies, gchar* fileNamePrefix, gchar* keyWords)
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
 	fileSH = FOpen(fileNameSH, "w");
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
 		file = FOpen(fileName, "w");
		if(!file) return FALSE;
		fprintf(file,"* ===============================\n");
		fprintf(file,"* Input file for Mopac\n");
		fprintf(file,"* Semi-Emperical Energy(kCal/mol) =%f\n",energies[i]);
		fprintf(file,"* ===============================\n");
		if(spinMultiplicity>1)
		fprintf(file,"%s UHF CHARGE=%d %s\n",keyWords,totalCharge,multiplicityStr);
		else
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
		{int ierr= system(buffer);}
	}
#endif
 	if(fileName) g_free(fileName);
 	if(fileNameSH) g_free(fileNameSH);
	return TRUE;

}
/*****************************************************************************/
static gboolean createGaussianFiles(gint numberOfGeometries, SemiEmpiricalModel** geometries, gdouble* energies, gchar* fileNamePrefix, gchar* keyWords)
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
 	fileSH = FOpen(fileNameSH, "w");
	if(!fileSH) return FALSE;


	for(i=0;i<numberOfGeometries;i++)
	{
		if(!geometries[i]) continue;
 		if(fileName) g_free(fileName);
		fileName = g_strdup_printf("%s_%d.com",fileNamePrefix,i+1);
 		file = FOpen(fileName, "w");
		if(!file) return FALSE;
		fprintf(file,"#P %s\n",keyWords);
		fprintf(file,"#  Units(Ang,Deg)\n");
		fprintf(file,"\n");
		fprintf(file,"File generated by Gabedit\n");
		fprintf(file,"Semi-emperical Energy(kCal/mol) = %f\n",energies[i]);
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
		{int ierr= system(buffer);}
	}
#endif
 	if(fileName) g_free(fileName);
 	if(fileNameSH) g_free(fileNameSH);
	return TRUE;

}
/*****************************************************************************/
static gboolean createFireFlyFiles(gint numberOfGeometries, SemiEmpiricalModel** geometries, gdouble* energies, gchar* fileNamePrefix, gchar* keyWords)
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
 	fileSH = FOpen(fileNameSH, "w");
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
 		file = FOpen(fileName, "w");
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
		{int ierr= system(buffer);}
	}
#endif
 	if(fileName) g_free(fileName);
 	if(fileNameSH) g_free(fileNameSH);
	return TRUE;

}
/*****************************************************************************/
static void semiEmpiricalMDConfo(GtkWidget* Win, gpointer data)
{
	SemiEmpiricalModel seModel; 
	SemiEmpiricalMD seMD;
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
	SemiEmpiricalModel** geometries = NULL; 
	gdouble* energies = NULL;
	gboolean optMopac = FALSE;
	gboolean optFireFly = FALSE;
	gboolean optOpenBabel = FALSE;
	gboolean optGeneric = FALSE;
	gchar* program = NULL;
	gchar* method = NULL;
	gdouble tolEnergy = -1;
	gdouble tolDistance = -1;
	SemiEmpiricalModelConstraints constraints = NOCONSTRAINTS;

	gint i;
	gchar message[BSIZE]="Created files :\n";
	gchar* dirName = NULL;

	constraints = NOCONSTRAINTS;
	if(GTK_TOGGLE_BUTTON (buttonConstraintsOptions[BONDSCONSTRAINTS])->active)
				constraints = BONDSCONSTRAINTS;
	if(GTK_TOGGLE_BUTTON (buttonConstraintsOptions[BONDSANGLESCONSTRAINTS])->active)
				constraints = BONDSANGLESCONSTRAINTS;


	if(GTK_TOGGLE_BUTTON (buttonTolerance[TOLE])->active)
		tolEnergy = atoi(gtk_entry_get_text(GTK_ENTRY(entryTolerance[TOLE])));
	if(GTK_TOGGLE_BUTTON (buttonTolerance[TOLD])->active)
		tolDistance = atoi(gtk_entry_get_text(GTK_ENTRY(entryTolerance[TOLD])));

	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	if(GTK_TOGGLE_BUTTON (buttonMopac)->active) 
	{
		program = g_strdup("Mopac");
		method = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryMopacMethod)));
	}
	else if(GTK_TOGGLE_BUTTON (buttonFireFly)->active) 
	{
		program = g_strdup("FireFly");
		method = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFireFlyMethod)));
	}
	else if(GTK_TOGGLE_BUTTON (buttonOpenBabel)->active) 
	{
		program = g_strdup("OpenBabel");
		method = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryOpenBabelMethod)));
	}
	else 
	{
		program = g_strdup("Generic");
		method = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryGenericMethod)));
		if(!genericProgName) g_free(genericProgName);
		genericProgName = g_strdup(method);
	}

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

	optMopac = GTK_TOGGLE_BUTTON (buttonPostOpt)->active && GTK_TOGGLE_BUTTON (buttonMopac)->active; 
	optFireFly = GTK_TOGGLE_BUTTON (buttonPostOpt)->active && GTK_TOGGLE_BUTTON (buttonFireFly)->active; 
	optOpenBabel = GTK_TOGGLE_BUTTON (buttonPostOpt)->active && GTK_TOGGLE_BUTTON (buttonOpenBabel)->active; 
	optGeneric = GTK_TOGGLE_BUTTON (buttonPostOpt)->active && GTK_TOGGLE_BUTTON (buttonGeneric)->active; 
	/* number for geometries */
	{
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryNumberOfGeom)));
		numberOfGeometries = atoi(tmp);
		if(numberOfGeometries<2) numberOfGeometries = 2;
		g_free(tmp);
	}
	/* fileName for geometries */
	{
		dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFileNameGeom)));
		if(dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
			fileNameGeom = g_strdup_printf("%s%s%s",dirName, G_DIR_SEPARATOR_S,tmp);
		else
			fileNameGeom = g_strdup_printf("%s%s",dirName, tmp);

		g_free(tmp);
	}
	if(GTK_TOGGLE_BUTTON (buttonCreateGaussian)->active)
		gaussianKeywords = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryGaussianKeywords)));
	if(GTK_TOGGLE_BUTTON (buttonCreateMopac)->active)
		mopacKeywords = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryMopacKeywords)));
	if(GTK_TOGGLE_BUTTON (buttonCreateFireFly)->active)
		fireflyKeywords = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFireFlyKeywords)));

	if(GTK_TOGGLE_BUTTON (buttonSaveTraj)->active)
	{
		if(!dirName) dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFileNameTraj)));
		if(dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
			fileNameTraj = g_strdup_printf("%s%s%s",dirName, G_DIR_SEPARATOR_S,tmp);
		else
			fileNameTraj = g_strdup_printf("%s%s",dirName, tmp);


		g_free(tmp);
	}
	if(GTK_TOGGLE_BUTTON (buttonSaveProp)->active)
	{
		if(!dirName) dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFileNameProp)));
		if(dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
			fileNameProp = g_strdup_printf("%s%s%s",dirName, G_DIR_SEPARATOR_S,tmp);
		else
			fileNameProp = g_strdup_printf("%s%s",dirName, tmp);
		g_free(tmp);
	}


	gtk_widget_destroy(Win);
    	while( gtk_events_pending() ) gtk_main_iteration();

	set_sensitive_stop_button( TRUE);
	StopCalcul = FALSE;

	if(!strcmp(program,"Mopac")) seModel = createMopacModel(geometry0,Natoms, totalCharge, spinMultiplicity,method,dirName, constraints);
	else if(!strcmp(program,"FireFly")) seModel = createFireFlyModel(geometry0,Natoms, totalCharge, spinMultiplicity,method,dirName,constraints);
	else if(!strcmp(program,"OpenBabel")) seModel = createOpenBabelModel(geometry0,Natoms, totalCharge, spinMultiplicity,method,dirName,constraints);
	else seModel = createGenericModel(geometry0,Natoms, totalCharge, spinMultiplicity,method,dirName,constraints);

	g_free(program);

	if(StopCalcul)
	{
		set_text_to_draw(" ");
		set_statubar_operation_str(_("Calculation Canceled "));
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

	geometries = runSemiEmpiricalMDConfo(&seMD, &seModel,
		updateFrequency, heatTime, equiTime, runTime, heatTemp, equiTemp, runTemp, stepSize, 
		integrator, thermostat, friction, collide, numberOfGeometries, fileNameTraj, fileNameProp);
	freeSemiEmpiricalModel(&seModel);
	if(!StopCalcul && geometries && numberOfGeometries>0)
	{
		gint i;
		energies = g_malloc(numberOfGeometries*sizeof(gdouble));
		for(i=0;i<numberOfGeometries;i++)
			energies[i] = geometries[i]->molecule.energy;
	}

	if(StopCalcul)
	{
		set_text_to_draw(" ");
		set_statubar_operation_str(_("Calculation canceled"));
		drawGeom();
	}
	set_sensitive_stop_button( FALSE);
	set_text_to_draw(" ");
	/* minimazation by mopac*/
	if(optMopac && !StopCalcul)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
		gchar* keys=g_strdup_printf("%s XYZ",method);
		if(runMopacFiles(numberOfGeometries, geometries, energies, fileNamePrefix, keys))
		{
			sortGeometries(numberOfGeometries, geometries, energies);
			removeIdenticalGeometries(&numberOfGeometries, &geometries, &energies, tolEnergy, tolDistance);
			if(saveConfoGeometries(numberOfGeometries, geometries, energies, fileNameGeom))
			{
				read_gabedit_file_add_list(fileNameGeom);
				strcat(message,fileNameGeom);
				strcat(message,_("\n\tGeometries after minimization by Mopac"));
				strcat(message,_("\n\tTo read this file : 'Read/Gabedit file'\n\n"));
			}
		}
		if(fileNamePrefix) g_free(fileNamePrefix);
		if(keys)g_free(keys);
	}
	/* minimazation by FireFly AM1*/
	if(optFireFly && !StopCalcul)
	{
		gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
		gchar* keys=g_strdup_printf("RUNTYP=Optimize GBASIS=%s",method);
		if(runFireFlyFiles(numberOfGeometries, geometries, energies, fileNamePrefix, keys))
		{
			sortGeometries(numberOfGeometries, geometries, energies);
			removeIdenticalGeometries(&numberOfGeometries, &geometries, &energies, tolEnergy, tolDistance);
			if(saveConfoGeometries(numberOfGeometries, geometries, energies, fileNameGeom))
			{
				read_gabedit_file_add_list(fileNameGeom);
				strcat(message,fileNameGeom);
				strcat(message,_("\n\tGeometries after minimization by FireFly"));
				strcat(message,_("\n\tTo read this file : 'Read/Gabedit file'\n\n"));
			}

		}
		if(fileNamePrefix) g_free(fileNamePrefix);
		if(keys)g_free(keys);
	}
        /* minimazation by OpenBabel*/
        if(optOpenBabel && !StopCalcul )
        {
                gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
		gchar* keys=g_strdup_printf("obopt -ff %s",method);
                if(runOpenBabelFiles(numberOfGeometries, geometries, energies, fileNamePrefix, keys))
                {
                        char* fileNameGeomOpenBabel =g_strdup_printf("%sOpenBabel.gab",fileNamePrefix);
                        sortGeometries(numberOfGeometries, geometries, energies);
                        removeIdenticalGeometries(&numberOfGeometries, &geometries, &energies, tolEnergy, tolDistance);
                        if(saveConfoGeometries(numberOfGeometries, geometries, energies, fileNameGeom))
                        {
                                strcat(message,fileNameGeom);
                                strcat(message,("\n\tGeometries after minimization by OpenBabel"));
                                strcat(message,("\n\tTo read this file through Gabedit : 'Read/Gabedit file'\n\n"));
                        }
                        free(fileNameGeomOpenBabel);

                }
                if(fileNamePrefix) free(fileNamePrefix);
                if(keys)free(keys);
        }
        /* minimazation by Generic*/
        if(optGeneric && !StopCalcul )
        {
                gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
		gchar* keys=g_strdup_printf("Opt");
                if(runGenericFiles(numberOfGeometries, geometries, energies, fileNamePrefix, keys, method))
                {
                        char* fileNameGeomGeneric =g_strdup_printf("%sGeneric.gab",fileNamePrefix);
                        sortGeometries(numberOfGeometries, geometries, energies);
                        removeIdenticalGeometries(&numberOfGeometries, &geometries, &energies, tolEnergy, tolDistance);
                        if(saveConfoGeometries(numberOfGeometries, geometries, energies, fileNameGeom))
                        {
                                strcat(message,fileNameGeom);
                                strcat(message,("\n\tGeometries after minimization by Generic"));
                                strcat(message,("\n\tTo read this file through Gabedit : 'Read/Gabedit file'\n\n"));
                        }
                        free(fileNameGeomGeneric);

                }
                if(fileNamePrefix) free(fileNamePrefix);
                if(keys)free(keys);
        }

	g_free(method);
	if(!optMopac && !optFireFly && !optGeneric && !optOpenBabel && !StopCalcul)
	{
		/*  sort by energies */
		sortGeometries(numberOfGeometries, geometries, energies);
		removeIdenticalGeometries(&numberOfGeometries, &geometries, &energies, tolEnergy, tolDistance);
		/* printf("fileNameGeom = %s\n",fileNameGeom);*/
		if(saveConfoGeometries(numberOfGeometries, geometries, energies, fileNameGeom))
		{
			read_gabedit_file_add_list(fileNameGeom);
			strcat(message,fileNameGeom);
			strcat(message,_("\n\tGeometries selected and optimized using your Semi-emperical potentials"));
			strcat(message,_("\n\tTo read this file : 'Read/Gabedit file'\n\n"));
		}
	}
	if(numberOfGeometries>0 && geometries && !StopCalcul)
	{
		if(mopacKeywords)
		{
			gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
			createMopacFiles(numberOfGeometries, geometries, energies, fileNamePrefix, mopacKeywords);
			strcat(message,fileNamePrefix);
			strcat(message,_("_*.mop\n\tFiles for a post processing by Mopac\n\n"));
			if(fileNamePrefix) g_free(fileNamePrefix);
		}
		if(gaussianKeywords)
		{
			gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
			createGaussianFiles(numberOfGeometries, geometries, energies, fileNamePrefix, gaussianKeywords);
			strcat(message,fileNamePrefix);
			strcat(message,_("_*.com\n\tFiles for a post processing by Gaussian\n\n"));
			if(fileNamePrefix) g_free(fileNamePrefix);
		}
		if(fireflyKeywords)
		{
			gchar* fileNamePrefix = get_suffix_name_file(fileNameGeom);
			createFireFlyFiles(numberOfGeometries, geometries, energies, fileNamePrefix, fireflyKeywords);
			strcat(message,fileNamePrefix);
			strcat(message,_("P_*.inp\n\tFiles for a post processing by FireFly\n\n"));
			if(fileNamePrefix) g_free(fileNamePrefix);
		}
	}
	if(geometries)
	{
		if(!StopCalcul)
		for(i=0;i<numberOfGeometries;i++)
			if(geometries[i]) freeSemiEmpiricalModel(geometries[i]);
		g_free(geometries);
	}
	if(energies) g_free(energies);
	if(strlen(message)>20) Message(message,_("Info"),TRUE);
	if(fileNameGeom)g_free(fileNameGeom);

}
/*****************************************************************************/
static void semiEmpiricalMD(GtkWidget* Win, gpointer data)
{
	SemiEmpiricalModel seModel; 
	SemiEmpiricalMD seMD;
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
	gchar* program = NULL;
	gchar* method = NULL;
	gchar* dirName = NULL;
	gint totalCharge = 0;
	gint spinMultiplicity = 1;
	SemiEmpiricalModelConstraints constraints = NOCONSTRAINTS;

	constraints = NOCONSTRAINTS;
	if(GTK_TOGGLE_BUTTON (buttonConstraintsOptions[BONDSCONSTRAINTS])->active)
				constraints = BONDSCONSTRAINTS;
	if(GTK_TOGGLE_BUTTON (buttonConstraintsOptions[BONDSANGLESCONSTRAINTS])->active)
				constraints = BONDSANGLESCONSTRAINTS;


	totalCharge = atoi(gtk_entry_get_text(GTK_ENTRY(entryCharge)));
	spinMultiplicity = atoi(gtk_entry_get_text(GTK_ENTRY(entrySpinMultiplicity)));
	TotalCharges[0] = totalCharge;
	SpinMultiplicities[0] = spinMultiplicity;
	if(GTK_TOGGLE_BUTTON (buttonMopac)->active) 
	{
		program = g_strdup("Mopac");
		method = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryMopacMethod)));
	}
	if(GTK_TOGGLE_BUTTON (buttonFireFly)->active) 
	{
		program = g_strdup("FireFly");
		method = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFireFlyMethod)));
	}
	else if(GTK_TOGGLE_BUTTON (buttonOpenBabel)->active) 
	{
		program = g_strdup("OpenBabel");
		method = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryOpenBabelMethod)));
	}
	else if(GTK_TOGGLE_BUTTON (buttonGeneric)->active) 
	{
		program = g_strdup("Generic");
		method = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryGenericMethod)));
		if(!genericProgName) g_free(genericProgName);
		genericProgName = g_strdup(method);
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

	dirName = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER(buttonDirSelector));
	if(GTK_TOGGLE_BUTTON (buttonSaveTraj)->active)
	{
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFileNameTraj)));
		if(dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
			fileNameTraj = g_strdup_printf("%s%s%s",dirName, G_DIR_SEPARATOR_S,tmp);
		else
			fileNameTraj = g_strdup_printf("%s%s",dirName, tmp);

		g_free(tmp);
	}
	if(GTK_TOGGLE_BUTTON (buttonSaveProp)->active)
	{
		gchar* tmp = g_strdup(gtk_entry_get_text(GTK_ENTRY(entryFileNameProp)));
		if(dirName[strlen(dirName)-1] != G_DIR_SEPARATOR)
			fileNameProp = g_strdup_printf("%s%s%s",dirName, G_DIR_SEPARATOR_S,tmp);
		else
			fileNameProp = g_strdup_printf("%s%s",dirName, tmp);
		g_free(tmp);
	}


	gtk_widget_destroy(Win);
    	while( gtk_events_pending() ) gtk_main_iteration();

	set_sensitive_stop_button( TRUE);
	StopCalcul = FALSE;

	if(!strcmp(program,"Mopac")) seModel = createMopacModel(geometry0,Natoms, totalCharge, spinMultiplicity,method,dirName, constraints);
	else if(!strcmp(program,"FireFly")) seModel = createFireFlyModel(geometry0,Natoms, totalCharge, spinMultiplicity,method,dirName, constraints);
	else if(!strcmp(program,"OpenBabel")) seModel = createOpenBabelModel(geometry0,Natoms, totalCharge, spinMultiplicity,method,dirName, constraints);
	else if(!strcmp(program,"Generic")) seModel = createGenericModel(geometry0,Natoms, totalCharge, spinMultiplicity,method,dirName, constraints);

	g_free(method);
	g_free(program);

	if(StopCalcul)
	{
		set_text_to_draw(" ");
		set_statubar_operation_str(_("Calculation Canceled "));
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

	runSemiEmpiricalMD(&seMD, &seModel,
		updateFrequency, heatTime, equiTime, runTime, coolTime, heatTemp, equiTemp, runTemp, coolTemp, stepSize, 
		integrator, thermostat, friction, collide, fileNameTraj, fileNameProp);

	if(StopCalcul)
	{
		set_text_to_draw(" ");
		set_statubar_operation_str(_("Calculation Canceled "));
		drawGeom();
	}
	set_sensitive_stop_button(FALSE);
	set_text_to_draw(" ");
	freeSemiEmpiricalModel(&seModel);
	g_free(dirName);
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

	LabelOnglet = gtk_label_new(_("Molecular Dynamics Options"));
	LabelMenu = gtk_label_new(_("Molecular Dynamics Options"));
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
	label = gtk_label_new(_("Heating"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 3;
	label = gtk_label_new(_("Equilibrium"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 4;
	label = gtk_label_new(_("Production"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 5;
	label = gtk_label_new(_("Cooling"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 0;
	label = gtk_label_new(_("Time (ps)"));
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
	label = gtk_label_new(_("Temperature (K)"));
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
	label = gtk_label_new(_("Step size (fs)"));
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
	label = gtk_label_new(_("Screen Rafresh after"));
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
	gtk_entry_set_text(GTK_ENTRY(entryMDRafresh),"1");
	gtk_table_attach(GTK_TABLE(table),entryMDRafresh, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 3;
	label = gtk_label_new(_("steps"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 5;
	j = 0;
	label = gtk_label_new(_("Collide(ps^-1)"));
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
	label = gtk_label_new(_("Friction(ps^-1)"));
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
	buttonMDOptions[VERLET]= gtk_radio_button_new_with_label( NULL, _("MD Trajectory via Verlet velocity Algorithm")); 
	gtk_table_attach(GTK_TABLE(table),buttonMDOptions[VERLET],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDOptions[VERLET]), TRUE);
/*----------------------------------------------------------------------------------*/
	i = 9;
	j = 0;
	buttonMDOptions[BEEMAN]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDOptions[VERLET])), _("MD Trajectory via Modified Beeman Algorithm")); 
	gtk_table_attach(GTK_TABLE(table),buttonMDOptions[BEEMAN],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDOptions[BEEMAN]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 10;
	j = 0;
	buttonMDOptions[STOCHASTIC]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDOptions[VERLET])), _("Stochastic Dynamics via Verlet velocity Algorithm")); 
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
	buttonConstraintsOptions[NOCONSTRAINTS]= gtk_radio_button_new_with_label( NULL, _("No constraints")); 
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
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonConstraintsOptions[NOCONSTRAINTS])), _("Bond constraints")); 
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
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonConstraintsOptions[NOCONSTRAINTS])), _("Bond & Angle constraints")); 
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
	buttonSaveTraj = gtk_check_button_new_with_label(_("Save Trajectory in ")); 
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
	buttonSaveProp = gtk_check_button_new_with_label(_("Save Properties in ")); 
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
	label = gtk_label_new(_("Folder"));
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
	_(
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
	)
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
	_(
	"During the production step, Gabedit selects a number geometries"
	"\nAt the end of the molecular dynamic simulation,"
	"\nthe geometries are sorted by energy and are saved in a file."
	"\nGabedit can also optimize these geometries, interactively."
	"\nThe very similar molecular structures can be removed."
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
	)
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
	GtkWidget *table = NULL;
	GtkWidget *label = NULL;
	GtkWidget *sep = NULL;
	gint i;
	gint j;

	totalCharge = TotalCharges[0];
	spinMultiplicity=SpinMultiplicities[0];

	table = gtk_table_new(10,5,FALSE);

	frame = gtk_frame_new (_("Post Processing"));
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
	buttonPostOpt = gtk_radio_button_new_with_label(
			NULL,
			_("Minimization of selected geometries"));
	gtk_table_attach(GTK_TABLE(table),buttonPostOpt,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPostOpt), TRUE);
	gtk_widget_show (buttonPostOpt);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonPostNone =gtk_radio_button_new_with_label( 
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonPostOpt)),
			_("No minimization of selected geometries")); 
	gtk_table_attach(GTK_TABLE(table),buttonPostNone,
			j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonPostNone), FALSE);
	gtk_widget_show (buttonPostNone);

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
	label = gtk_label_new(_("Remove identical molecular structures"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+6,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonTolerance[TOLE] = gtk_check_button_new_with_label(_("Energy tolerance(KCal/mol)")); 
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
	buttonTolerance[TOLD] = gtk_check_button_new_with_label(_("Distance tolerance(Angstrom)")); 
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
	buttonCreateMopac = gtk_check_button_new_with_label(_("Create Mopac files.          Keywords ")); 
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
	gtk_entry_set_text(GTK_ENTRY(entryMopacKeywords),"PM7 XYZ AUX");
	gtk_table_attach(GTK_TABLE(table),entryMopacKeywords, j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i++;
	j = 0;
	buttonCreateGaussian = gtk_check_button_new_with_label(_("Create Gaussian files.     Keywords ")); 
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
	buttonCreateFireFly = gtk_check_button_new_with_label(_("Create FireFly files.    Keywords ")); 
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
}
/********************************************************************************/
static void AddModelOptionsDlg(GtkWidget *NoteBook, GtkWidget *win)
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

	LabelOnglet = gtk_label_new(_("Model"));
	LabelMenu = gtk_label_new(_("Model"));
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook), frame,LabelOnglet, LabelMenu);
	g_object_set_data (G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	table = gtk_table_new(5,3,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

	i=-1;
/*==================================================================================*/
	i++;
	j = 0;
	buttonMopac = gtk_radio_button_new_with_label( NULL,_("Use Mopac with method"));
	gtk_table_attach(GTK_TABLE(table),buttonMopac,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMopac), TRUE);
	gtk_widget_show (buttonMopac);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryMopacMethod = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryMopacMethod),"PM7");
	gtk_table_attach(GTK_TABLE(table),entryMopacMethod, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*==================================================================================*/
	i++;
	j = 0;
	buttonFireFly = gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMopac)),
			_("Use FireFly with method"));
	gtk_table_attach(GTK_TABLE(table),buttonFireFly,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonFireFly), FALSE);
	gtk_widget_show (buttonFireFly);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryFireFlyMethod = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryFireFlyMethod),"AM1");
	gtk_table_attach(GTK_TABLE(table),entryFireFlyMethod, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*==================================================================================*/
	i++;
	j = 0;
	buttonOpenBabel = gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMopac)),
			_("Use OpenBabel with method"));
	gtk_table_attach(GTK_TABLE(table),buttonOpenBabel,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonOpenBabel), FALSE);
	gtk_widget_show (buttonOpenBabel);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryOpenBabelMethod = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(entryOpenBabelMethod),"MMFF94");
	gtk_table_attach(GTK_TABLE(table),entryOpenBabelMethod, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*==================================================================================*/
	i++;
	j = 0;
	buttonGeneric = gtk_radio_button_new_with_label(
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMopac)),
			_("Use Generic with command"));
	gtk_table_attach(GTK_TABLE(table),buttonGeneric,
			j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonGeneric), FALSE);
	gtk_widget_show (buttonGeneric);
/*----------------------------------------------------------------------------------*/
	j = 1;
	label = gtk_label_new(":");
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	j = 2;
	entryGenericMethod = gtk_entry_new();
	if(!genericProgName) genericProgName = g_strdup("myGenericProgram");
	gtk_entry_set_text(GTK_ENTRY(entryGenericMethod),genericProgName);
	gtk_table_attach(GTK_TABLE(table),entryGenericMethod, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*==================================================================================*/
	i++;
	j = 0;
	vbox = gtk_vbox_new (FALSE, 0);
	gtk_table_attach(GTK_TABLE(table),vbox,
			j,j+3,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	addChargeSpin(vbox);
/*==================================================================================*/
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

	LabelOnglet = gtk_label_new(_("Info"));
	LabelMenu = gtk_label_new(_("Info"));
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

	LabelOnglet = gtk_label_new(_("Info"));
	LabelMenu = gtk_label_new(_("Info"));
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

	LabelOnglet = gtk_label_new(_("General"));
	LabelMenu = gtk_label_new(_("General"));
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook), frame,LabelOnglet, LabelMenu);
	g_object_set_data (G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	table = gtk_table_new(8,5,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 0;
	add_label_table(table,_("Number of selected geometries"),(gushort)i,(gushort)j);
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
	add_label_table(table,_("Geometries saved in File"),(gushort)i,(gushort)j);
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
	add_label_table(table,_("Folder"),(gushort)i,(gushort)j);
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

	LabelOnglet = gtk_label_new(_("Molecular Dynamics"));
	LabelMenu = gtk_label_new(_("Molecular Dynamics"));
	gtk_notebook_append_page_menu(GTK_NOTEBOOK(NoteBook), frame,LabelOnglet, LabelMenu);
	g_object_set_data (G_OBJECT (frame), "frame", frame);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_widget_show (vbox);
	gtk_container_add (GTK_CONTAINER (frame), vbox);

	table = gtk_table_new(18,6,FALSE);
	gtk_box_pack_start (GTK_BOX (vbox), table, TRUE, TRUE, 0);

/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 2;
	label = gtk_label_new(_("Heating"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 3;
	label = gtk_label_new(_("Equilibrium"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 0;
	j = 4;
	label = gtk_label_new(_("Production"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_EXPAND) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 1;
	j = 0;
	label = gtk_label_new(_("Time (ps)"));
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
	gtk_entry_set_text(GTK_ENTRY(entryMDTimes[2]),"10.0");
	gtk_table_attach(GTK_TABLE(table),entryMDTimes[2], j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 2;
	j = 0;
	label = gtk_label_new(_("Temperature (K)"));
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
	label = gtk_label_new(_("Step size (fs)"));
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
	label = gtk_label_new(_("Screen Rafresh after"));
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
	gtk_entry_set_text(GTK_ENTRY(entryMDRafresh),"1");
	gtk_table_attach(GTK_TABLE(table),entryMDRafresh, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 4;
	j = 3;
	label = gtk_label_new(_("steps"));
	gtk_table_attach(GTK_TABLE(table),label, j,j+1,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
/*----------------------------------------------------------------------------------*/
	i = 5;
	j = 0;
	label = gtk_label_new(_("Collide(ps^-1)"));
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
	label = gtk_label_new(_("Friction(ps^-1)"));
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
	buttonMDOptions[VERLET]= gtk_radio_button_new_with_label( NULL, _("MD Trajectory via Verlet velocity Algorithm")); 
	gtk_table_attach(GTK_TABLE(table),buttonMDOptions[VERLET],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDOptions[VERLET]), TRUE);
/*----------------------------------------------------------------------------------*/
	i = 9;
	j = 0;
	buttonMDOptions[BEEMAN]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDOptions[VERLET])), _("MD Trajectory via Modified Beeman Algorithm")); 
	gtk_table_attach(GTK_TABLE(table),buttonMDOptions[BEEMAN],
			j,j+4,i,i+1,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK) ,
                  (GtkAttachOptions)(GTK_FILL|GTK_SHRINK),
                  1,1);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (buttonMDOptions[BEEMAN]), FALSE);
/*----------------------------------------------------------------------------------*/
	i = 10;
	j = 0;
	buttonMDOptions[STOCHASTIC]= gtk_radio_button_new_with_label(gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonMDOptions[VERLET])), _("Stochastic Dynamics via Verlet velocity Algorithm")); 
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
	buttonConstraintsOptions[NOCONSTRAINTS]= gtk_radio_button_new_with_label( NULL, _("No constraints")); 
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
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonConstraintsOptions[NOCONSTRAINTS])), _("Bond constraints")); 
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
			gtk_radio_button_get_group (GTK_RADIO_BUTTON (buttonConstraintsOptions[NOCONSTRAINTS])), _("Bond & Angle constraints")); 
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
	buttonSaveTraj = gtk_check_button_new_with_label(_("Save Trajectory in ")); 
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
	buttonSaveProp = gtk_check_button_new_with_label(_("Save Properties in ")); 
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
void semiEmpiricalMolecularDynamicsDlg()
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
	AddModelOptionsDlg(NoteBook, Win);
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
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)semiEmpiricalMD,GTK_OBJECT(Win));
	gtk_widget_show (button);

	gtk_widget_show_all(Win);
  
}
/***********************************************************************/
void semiEmpiricalMolecularDynamicsConfoDlg()
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
	AddModelOptionsDlg(NoteBook, Win);
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
	g_signal_connect_swapped(GTK_OBJECT(button), "clicked", (GCallback)semiEmpiricalMDConfo,GTK_OBJECT(Win));
	gtk_widget_show (button);

	gtk_widget_show_all(Win);
  
}
/***********************************************************************/
