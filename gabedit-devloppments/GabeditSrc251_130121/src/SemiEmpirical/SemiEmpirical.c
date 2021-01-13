/* SemiEmpirical.c */
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
#ifndef G_OS_WIN32
#include <unistd.h>
#endif

#include "../Common/Global.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../SemiEmpirical/AtomSE.h"
#include "../SemiEmpirical/MoleculeSE.h"
#include "../SemiEmpirical/SemiEmpiricalModel.h"
#include "../SemiEmpirical/SemiEmpirical.h"

static void calculateGradientMopac(SemiEmpiricalModel* seModel);
static void calculateEnergyMopac(SemiEmpiricalModel* seModel);
static void calculateGradientFireFly(SemiEmpiricalModel* seModel);
static void calculateEnergyFireFly(SemiEmpiricalModel* seModel);
static void calculateGradientOpenBabel(SemiEmpiricalModel* seModel);
static void calculateEnergyOpenBabel(SemiEmpiricalModel* seModel);
static void calculateGradientGeneric(SemiEmpiricalModel* seModel);
static void calculateEnergyGeneric(SemiEmpiricalModel* seModel);


/****************************************************************/
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
/**********************************************************************************************************************************/
static gboolean getGradientMopacOut(gchar* fileNameOut, SemiEmpiricalModel *seModel)
{
	FILE* file = NULL;
	gchar buffer[1024];
	gchar* pdest = NULL;
	gboolean Ok = FALSE;
	gdouble tmp;
	gint i;
	gint j;

 	file = FOpen(fileNameOut, "rb");
		/* fprintf(stderr,"DEBUG : Je suis dans getGradientMopac\n");*/
	if(!file) return FALSE;
	 while(!feof(file))
	 {
		if(!fgets(buffer,BSIZE,file))break;
		if(strstr(buffer, "FINAL  POINT  AND  DERIVATIVES"))break;
		if(strstr(buffer, "FINAL") && strstr(buffer, "POINT") && strstr(buffer,"AND") && strstr(buffer,"DERIVATIVES"))break;
	}
	if(!strstr(buffer,"FINAL")) rewind(file);// Old Mopac, beforee 2018
	 while(!feof(file))
	 {
		if(!fgets(buffer,BSIZE,file))break;
		pdest = strstr( buffer, "PARAMETER     ATOM    TYPE            VALUE       GRADIENT");
		if(pdest) 
		{
			/* fprintf(stderr,"DEBUG : %s\n",pdest);*/
			for(i=0;i<seModel->molecule.nAtoms;i++)
			for(j=0;j<3;j++)
			{
				if(!fgets(buffer,BSIZE,file))break;
				pdest = strstr( buffer, "CARTESIAN");
				if(pdest)
				{
					/* fprintf(stderr,"DEBUG : %s\n",pdest+12);*/
					if(sscanf(pdest+12,"%lf %lf",&tmp,&seModel->molecule.gradient[j][i])!=2)
					{
						fclose(file);
						return FALSE;
					}
				}
					/* fprintf(stderr,"DEBUG : %f\n",seModel->molecule.gradient[j][i]);*/
			}
			Ok = TRUE;
			break;
	 	}
		pdest = strstr( buffer, "Cartesian Gradients"); /* MOZYME Keyword */
		if(pdest) 
		{
			gchar td[100];
			gint d;
			if(!fgets(buffer,BSIZE,file))break; /*Atom       X  ....*/
			if(!fgets(buffer,BSIZE,file))break; /* backspace */
			for(i=0;i<seModel->molecule.nAtoms;i++)
			{
				if(!fgets(buffer,BSIZE,file)) /* 1  O    0.000   -4.566    0.027  */
				{
					fclose(file);
					return FALSE;
				}
				if(sscanf(buffer,"%d %s %lf %lf %lf",&d, td, 
						&seModel->molecule.gradient[0][i],
						&seModel->molecule.gradient[1][i],
						&seModel->molecule.gradient[2][i]
					 )
						!=5)
					{
						fclose(file);
						return FALSE;
					}
			}
			Ok = TRUE;
			break;
	 	}
	 }
	fclose(file);
	return Ok;
}
/*****************************************************************************/
static gboolean getGradientMopacAux(gchar* fileNameOut, SemiEmpiricalModel *seModel)
{
	FILE* file = NULL;
	gboolean Ok = FALSE;
	gchar* fileNamePrefix = get_suffix_name_file(fileNameOut);
	gchar* fileNameAux = g_strdup_printf("%s.aux", fileNamePrefix);
	gchar** strlist = NULL;
	gint n;

	/*
	fprintf(stderr,"fileNameOut %s\n",fileNameOut);
	fprintf(stderr,"fileNamePrefix %s\n",fileNamePrefix);
	fprintf(stderr,"fileNameAux %s\n",fileNameAux);
	*/
 	file = FOpen(fileNameAux, "rb");
	if(file)
	{
		strlist = get_one_block_from_aux_mopac_file(file, "GRADIENTS:KCAL",  &n);
		if(strlist && n== seModel->molecule.nAtoms*3)
		{
			gint i;
			gint j;
			gint k;
			k=0;
			for(i=0;i<seModel->molecule.nAtoms;i++)
			for(j=0;j<3;j++)
			{
				/* fprintf(stderr,"strlist %d %s\n",k,strlist[k]);*/
				seModel->molecule.gradient[j][i] = atof(strlist[k]);
				k++;
			}
			Ok = TRUE;
		}
	}
	fclose(file);
	if(strlist) strlist = free_one_string_table(strlist, n);
	if(fileNamePrefix) g_free(fileNamePrefix); 
	if(fileNameAux) g_free(fileNameAux); 
	return Ok;
}
/*****************************************************************************/
static gboolean getGradientMopac(gchar* fileNameOut, SemiEmpiricalModel *seModel)
{
	gboolean Ok = FALSE;
	/* try to read gradients from mopac out file : more digits than those of aux */
	Ok = getGradientMopacOut(fileNameOut,seModel);
	if(Ok) return Ok;
	Ok = getGradientMopacAux(fileNameOut,seModel);
	return Ok;
}
/*****************************************************************************/
static gchar* runOneMopac(SemiEmpiricalModel* seModel, gchar* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gint j;
	gchar* fileNameIn = NULL;
	gchar* fileNameOut = NULL;
	gchar* fileNameSH = NULL;
	gchar multiplicityStr[100];
	gchar buffer[1024];
	MoleculeSE m = seModel->molecule;
#ifdef G_OS_WIN32
	gchar c='%';
#endif

	if(m.nAtoms<1) return fileNameOut;
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%s%sMopacOne.sh",seModel->workDir,G_DIR_SEPARATOR_S);
#else
	fileNameSH = g_strdup_printf("%s%sMopacOne.bat",seModel->workDir,G_DIR_SEPARATOR_S);
#endif
 	fileSH = FOpen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"@echo off\n");
	fprintf(fileSH,"set PATH=%cPATH%c;\"%s\"\n",c,c,mopacDirectory);
#endif

	getMultiplicityName(seModel->molecule.spinMultiplicity, multiplicityStr);

	fileNameIn = g_strdup_printf("%s%sOne.mop",seModel->workDir,G_DIR_SEPARATOR_S);
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
	if(seModel->molecule.spinMultiplicity>1)
		fprintf(file,"%s UHF CHARGE=%d AUX %s\n",keyWords,seModel->molecule.totalCharge,multiplicityStr);
	else
		fprintf(file,"%s CHARGE=%d AUX %s\n",keyWords,seModel->molecule.totalCharge,multiplicityStr);
	fprintf(file,"\n");
	fprintf(file,"Mopac file generated by Gabedit\n");

	for(j=0;j<m.nAtoms;j++)
	{
	fprintf(file," %s %f %d %f %d %f %d\n", 
			m.atoms[j].prop.symbol,
			m.atoms[j].coordinates[0],
			1,
			m.atoms[j].coordinates[1],
			1,
			m.atoms[j].coordinates[2],
			1
			);
	}
	fclose(file);
#ifndef G_OS_WIN32
	fprintf(fileSH,"%s %s\n",NameCommandMopac,fileNameIn);
	fclose(fileSH);
	sprintf(buffer,"chmod u+x %s",fileNameSH);
	{int ierr = system(buffer);}
	{ int ierr = system(fileNameSH);}
#else
	fprintf(fileSH,"\"%s\" \"%s\"\n",NameCommandMopac,fileNameIn);
	fclose(fileSH);
	sprintf(buffer,"\"%s\"",fileNameSH);
	{int ierr = system(buffer);}
#endif

	unlink(fileNameIn);
	unlink(fileNameSH);
 	if(fileNameIn) g_free(fileNameIn);
 	if(fileNameSH) g_free(fileNameSH);
	fileNameOut = g_strdup_printf("%s%sOne.out",seModel->workDir,G_DIR_SEPARATOR_S);
	return fileNameOut;
}
/**********************************************************************/
static SemiEmpiricalModel newMopacModel(gchar* method, gchar* dirName, SemiEmpiricalModelConstraints constraints)
{
	SemiEmpiricalModel seModel = newSemiEmpiricalModel(method, dirName, constraints);

	seModel.klass->calculateGradient = calculateGradientMopac;
	seModel.klass->calculateEnergy = calculateEnergyMopac;

	return seModel;
}
/**********************************************************************/
static void calculateGradientMopac(SemiEmpiricalModel* seModel)
{
	gint i;
	gint j;
	MoleculeSE m = seModel->molecule;
	gchar* keyWords = NULL;
	gchar* fileOut = NULL;
	if(!seModel) return;
	if(seModel->molecule.nAtoms<1) return;
	if(!seModel->method) return;
	keyWords = g_strdup_printf("%s 1SCF GRAD",seModel->method);
	fileOut = runOneMopac(seModel, keyWords);

	if(fileOut)
	{
		for(j=0;j<3;j++)
			for( i=0; i<m.nAtoms;i++)
				m.gradient[j][i] = 0.0;
		if(!getGradientMopac(fileOut, seModel))
		{
			StopCalcul=TRUE;
			set_text_to_draw(_("Problem : I cannot calculate the Gradient... "));
			set_statubar_operation_str(_("Calculation Stopped "));
			drawGeom();
			gtk_widget_set_sensitive(StopButton, FALSE);
			Waiting(1);
			return;
		}
		getEnergyMopac(fileOut, &seModel->molecule.energy);
		g_free(fileOut);
	}

}
/**********************************************************************/
static void calculateEnergyMopac(SemiEmpiricalModel* seModel)
{
	gchar* keyWords = NULL;
	gchar* fileOut = NULL;
	if(!seModel) return;
	if(seModel->molecule.nAtoms<1) return;
	if(!seModel->method) return;
	keyWords = g_strdup_printf("%s 1SCF",seModel->method);
	fileOut = runOneMopac(seModel, keyWords);
	if(fileOut)
	{
		getEnergyMopac(fileOut, &seModel->molecule.energy);
		g_free(fileOut);
	}

}
/**********************************************************************/
SemiEmpiricalModel createMopacModel (GeomDef* geom,gint Natoms,gint charge, gint spin, gchar* method, gchar* dirName, SemiEmpiricalModelConstraints constraints)
{
	SemiEmpiricalModel seModel = newMopacModel(method, dirName, constraints);

	seModel.molecule = createMoleculeSE(geom,Natoms, charge, spin,TRUE);
	setRattleConstraintsParameters(&seModel);
	
	return seModel;
}
/**********************************************************************/
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
				{
					OK = TRUE;
					/* break;*/
				}
			}
		}
	 }
	fclose(file);
	return OK;
}
/*****************************************************************************/
static gboolean getGradientFireFly(gchar* fileNameOut, SemiEmpiricalModel *seModel)
{
	FILE* file = NULL;
	gchar buffer[1024];
	gchar stmp[1024];
	gchar* pdest = NULL;
	gboolean Ok = FALSE;
	gint itmp;
	gint i;
	gint j;

 	file = FOpen(fileNameOut, "rb");
	if(!file) return FALSE;
	 while(!feof(file))
	 {
		if(!fgets(buffer,BSIZE,file))break;
		pdest = strstr( buffer, "ATOM                 E'X               E'Y               E'Z");
		if(pdest) 
		{
			for(i=0;i<seModel->molecule.nAtoms;i++)
			{
				if(!fgets(buffer,BSIZE,file))break;
				if(sscanf(buffer,"%d %s %lf %lf %lf",&itmp, stmp,
							&seModel->molecule.gradient[0][i],
							&seModel->molecule.gradient[1][i],
							&seModel->molecule.gradient[2][i]
							)!=5)
				{
					fclose(file);
					return FALSE;
				}
				for(j=0;j<3;j++) seModel->molecule.gradient[j][i] *= 627.50944796/BOHR_TO_ANG;
			}
			Ok = TRUE;
			break;
	 	}
	 }
	fclose(file);
	return Ok;
}
/*****************************************************************************/
static gchar* runOneFireFly(SemiEmpiricalModel* seModel, gchar* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	gint j;
	gchar* fileNameIn = NULL;
	gchar* fileNameOut = NULL;
	gchar* fileNameSH = NULL;
	gchar multiplicityStr[100];
	gchar buffer[1024];
	MoleculeSE m = seModel->molecule;
	gchar* fileNamePrefix = NULL;
#ifdef G_OS_WIN32
	gchar c='%';
#endif

	if(m.nAtoms<1) return fileNameOut;
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%s%sFireFlyOne.sh",seModel->workDir,G_DIR_SEPARATOR_S);
#else
	fileNameSH = g_strdup_printf("%s%sFireFlyOne.bat",seModel->workDir,G_DIR_SEPARATOR_S);
#endif
 	fileSH = FOpen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"@echo off\n");
	fprintf(fileSH,"set PATH=%cPATH%c;\"%s\"\n",c,c,fireflyDirectory);
#endif

	getMultiplicityName(seModel->molecule.spinMultiplicity, multiplicityStr);

	fileNameIn = g_strdup_printf("%s%sOne.inp",seModel->workDir,G_DIR_SEPARATOR_S);
	fileNameOut = g_strdup_printf("%s%sOne.out",seModel->workDir,G_DIR_SEPARATOR_S);


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
		if(seModel->molecule.spinMultiplicity==1)
			fprintf(file," $CONTRL SCFTYP=RHF $END\n");
		else
			fprintf(file," $CONTRL SCFTYP=UHF $END\n");
	}

	fprintf(file," $CONTRL ICHARG=%d MULT=%d $END\n",seModel->molecule.totalCharge,seModel->molecule.spinMultiplicity);
	if(strstr(keyWords,"GBASIS"))
	{
		sscanf(strstr(keyWords,"GBASIS"),"%s",buffer);
		fprintf(file," $BASIS %s $END\n",buffer);
	}
	fprintf(file," $DATA\n");
	fprintf(file,"Molecule specification\n");
	fprintf(file,"C1\n");
	for(j=0;j<m.nAtoms;j++)
	{
		gchar* symbol = m.atoms[j].prop.symbol;
		SAtomsProp prop = prop_atom_get(symbol);
		fprintf(file,"%s %f %f %f %f\n", 
			symbol,
			(gdouble)prop.atomicNumber,
			m.atoms[j].coordinates[0],
			m.atoms[j].coordinates[1],
			m.atoms[j].coordinates[2]
			);
	}
	fprintf(file," $END\n");
	fclose(file);
	fileNamePrefix = g_strdup_printf("%s%sWorkFF",seModel->workDir,G_DIR_SEPARATOR_S);
#ifndef G_OS_WIN32
	if(!strcmp(NameCommandFireFly,"pcgamess") || !strcmp(NameCommandFireFly,"nohup pcgamess")
	|| !strcmp(NameCommandFireFly,"firefly") || !strcmp(NameCommandFireFly,"nohup firefly"))
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
         	fprintf(fileSH,"del PUNCH 2>nul\n");
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
	{ int ierr = system(fileNameSH);}
#else
	sprintf(buffer,"\"%s\"",fileNameSH);
	{int ierr = system(buffer);}
#endif
	unlink(fileNameIn);
	unlink(fileNameSH);
 	if(fileNamePrefix) g_free(fileNamePrefix);
 	if(fileNameIn) g_free(fileNameIn);
 	if(fileNameSH) g_free(fileNameSH);
	return fileNameOut;
}
/**********************************************************************/
static SemiEmpiricalModel newFireFlyModel(gchar* method, gchar* dirName, SemiEmpiricalModelConstraints constraints)
{
	SemiEmpiricalModel seModel = newSemiEmpiricalModel(method, dirName, constraints);

	seModel.klass->calculateGradient = calculateGradientFireFly;
	seModel.klass->calculateEnergy = calculateEnergyFireFly;

	return seModel;
}
/**********************************************************************/
static void calculateGradientFireFly(SemiEmpiricalModel* seModel)
{
	gint i;
	gint j;
	MoleculeSE m = seModel->molecule;
	gchar* keyWords = NULL;
	gchar* fileOut = NULL;
	if(!seModel) return;
	if(seModel->molecule.nAtoms<1) return;
	if(!seModel->method) return;
	keyWords = g_strdup_printf("RUNTYP=GRADIENT GBASIS=%s",seModel->method);
	fileOut = runOneFireFly(seModel, keyWords);

	if(fileOut)
	{
		for(j=0;j<3;j++)
			for( i=0; i<m.nAtoms;i++)
				m.gradient[j][i] = 0.0;
		if(!getGradientFireFly(fileOut, seModel))
		{
#ifdef G_OS_WIN32
			gchar* comm = g_strdup_printf("type %s",fileOut);
#else
			gchar* comm = g_strdup_printf("cat %s",fileOut);
#endif
			StopCalcul=TRUE;
			set_text_to_draw(_("Problem : I cannot calculate the Gradient... "));
			set_statubar_operation_str(_("Calculation Stopped "));
			drawGeom();
			gtk_widget_set_sensitive(StopButton, FALSE);
			Waiting(1);
			{int ierr = system(comm);}
			g_free(fileOut);
			g_free(comm);
			return;
		}
		getEnergyFireFly(fileOut, &seModel->molecule.energy);
		g_free(fileOut);
	}

}
/**********************************************************************/
static void calculateEnergyFireFly(SemiEmpiricalModel* seModel)
{
	gchar* keyWords = NULL;
	gchar* fileOut = NULL;
	if(!seModel) return;
	if(seModel->molecule.nAtoms<1) return;
	if(!seModel->method) return;
	keyWords = g_strdup_printf("RUNTYP=Energy GBASIS=%s",seModel->method);
	fileOut = runOneFireFly(seModel, keyWords);
	if(fileOut)
	{
		getEnergyFireFly(fileOut, &seModel->molecule.energy);
		g_free(fileOut);
	}

}
/**********************************************************************/
SemiEmpiricalModel createFireFlyModel (GeomDef* geom,gint Natoms,gint charge, gint spin, gchar* method, gchar* dirName, SemiEmpiricalModelConstraints constraints)
{
	SemiEmpiricalModel seModel = newFireFlyModel(method,dirName, constraints);

	seModel.molecule = createMoleculeSE(geom,Natoms, charge, spin,TRUE);
	setRattleConstraintsParameters(&seModel);
	
	return seModel;
}
/**********************************************************************/
static gboolean getEnergyOpenBabel(gchar* fileNameOut, gdouble* energy)
{
        FILE* file = NULL;
        char buffer[1024];
        char* pdest = NULL;
        //char* energyTag = "TOTAL ENERGY =";
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
static gboolean getGradientOpenBabel(gchar* fileNameOut, SemiEmpiricalModel *seModel)
{
	FILE* file = NULL;
	char buffer[1024];
	char* pdest = NULL;
	//char* energyTag = "TOTAL ENERGY =";
	char* energyTag = "FINAL ENERGY:";
	char* gradTag = "Gradients:";
	gboolean kj = FALSE;
	double conv = 1.0;
	int i;
	MoleculeSE* mol = &seModel->molecule;

 	file = fopen(fileNameOut, "r");
	if(!file) return FALSE;
	 while(!feof(file))
	 {
		if(!fgets(buffer,BSIZE,file))break;
		pdest = strstr( buffer, energyTag);
		if(pdest &&sscanf(pdest+strlen(energyTag)+1,"%lf",&mol->energy)==1)
		{
			if(strstr(pdest,"kJ")) { kj = TRUE;}
			break;
		}
	 }
	 while(!feof(file))
	 {
		if(!fgets(buffer,BSIZE,file))break;
		if(strstr(buffer, gradTag))
		{
			for(i=0;i<mol->nAtoms;i++)
			{
				if(!fgets(buffer,BSIZE,file))break;
				//printf("%s\n",buffer);
				if(sscanf(buffer,"%lf %lf %lf",
					&mol->gradient[0][i],
					&mol->gradient[1][i],
					&mol->gradient[2][i]
					)!=3) break;
			}
			break;
		}
	 }
	if(kj) conv /= KCALTOKJ;
	mol->energy *= conv;
	for(i=0;i<mol->nAtoms;i++)
	{
		mol->gradient[0][i] *= conv;
		mol->gradient[1][i] *= conv;
		mol->gradient[2][i] *= conv;
	}

	fclose(file);
	return FALSE;
}
/*****************************************************************************/
static gchar* runOneOpenBabel(SemiEmpiricalModel* seModel, gchar* keyWords)
{
	FILE* fileSH = NULL;
	char* fileNameIn = NULL;
	char* fileNameOut = NULL;
	char* fileNameSH = NULL;
	char buffer[1024];
	MoleculeSE* mol = &seModel->molecule;
	char* NameCommandOpenBabel = "obgradient";
	int rank = 0;
#ifdef ENABLE_MPI
	MPI_Comm_rank( MPI_COMM_WORLD,&rank);
#endif
#ifdef G_OS_WIN32
	char c='%';
#endif

	if(mol->nAtoms<1) return fileNameOut;
#ifndef G_OS_WIN32
	fileNameSH = g_strdup_printf("%s%sOpenBabelOne%d.sh",seModel->workDir,G_DIR_SEPARATOR_S,rank);
#else
	fileNameSH = g_strdup_printf("%s%sOpenBabelOne%d.bat",seModel->workDir,G_DIR_SEPARATOR_S,rank);
#endif
 	fileSH = fopen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef G_OS_WIN32
	fprintf(fileSH,"@echo off\n");
#endif

	fileNameIn = g_strdup_printf("%s%sOne%d.hin",seModel->workDir,G_DIR_SEPARATOR_S,rank);
	fileNameOut = g_strdup_printf("%s%sOne%d.out",seModel->workDir,G_DIR_SEPARATOR_S,rank);

	if(!saveMoleculeSEHIN(mol,fileNameIn))
	{
 		if(fileNameIn) free(fileNameIn);
 		if(fileNameOut) free(fileNameOut);
 		if(fileNameSH) free(fileNameSH);
		return FALSE;
	}
#ifndef G_OS_WIN32
	if(keyWords)
	{
		fprintf(fileSH,"#!/bin/bash\n");
		fprintf(fileSH,"export PATH=$PATH:%s\n",openbabelDirectory);
		fprintf(fileSH,"export BABEL_DATADIR=%s\n",openbabelDirectory);
		fprintf(fileSH,"%s %s %s > %s 2>/dev/null\n",NameCommandOpenBabel,keyWords,fileNameIn,fileNameOut);
		fprintf(fileSH,"exit\n");
	}
	else
		fprintf(fileSH,"%s %s >%s 2>/dev/null",NameCommandOpenBabel,fileNameIn, fileNameOut);
#else
	{
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

		fprintf(fileSH,"%s %s %s > %s\n",NameCommandOpenBabel,keyWords,fileNameIn,fileNameOut);
		fprintf(fileSH,"exit\n");
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

	/*
	sprintf(buffer,"cat %s",fileNameOut);
	system(buffer);
	*/
#else
	sprintf(buffer,"\"%s\"",fileNameSH);
	system(buffer);
#endif
	unlink(fileNameIn);
	unlink(fileNameSH);
 	if(fileNameIn) free(fileNameIn);
 	if(fileNameSH) free(fileNameSH);
	return fileNameOut;
}
/**********************************************************************/
static SemiEmpiricalModel newOpenBabelModel(gchar* method, gchar* dirName, SemiEmpiricalModelConstraints constraints)
{
	SemiEmpiricalModel seModel = newSemiEmpiricalModel(method, dirName, constraints);

	seModel.klass->calculateGradient = calculateGradientOpenBabel;
	seModel.klass->calculateEnergy = calculateEnergyOpenBabel;

	return seModel;
}
/**********************************************************************/
static void calculateGradientOpenBabel(SemiEmpiricalModel* seModel)
{
        char* keyWords = NULL;
        char* fileOut = NULL;
        if(!seModel) return;
        if(seModel->molecule.nAtoms<1) return;
        if(!seModel->method) return;
        keyWords = g_strdup_printf("-ff %s",seModel->method);
        fileOut = runOneOpenBabel(seModel, keyWords);
        if(fileOut)
        {
                getGradientOpenBabel(fileOut, seModel);
                //getDipoleOpenBabel(fileOut, &seModel->molecule, seModel->molecule.dipole);
                computeMoleculeSEDipole(&seModel->molecule);
                free(fileOut);
        }

}
/**********************************************************************/
static void calculateEnergyOpenBabel(SemiEmpiricalModel* seModel)
{
        char* keyWords = NULL;
        char* fileOut = NULL;
        if(!seModel) return;
        if(seModel->molecule.nAtoms<1) return;
        if(!seModel->method) return;
        keyWords = g_strdup_printf("%s ",seModel->method);
        fileOut = runOneOpenBabel(seModel, keyWords);
        if(fileOut)
        {
                getEnergyOpenBabel(fileOut, &seModel->molecule.energy);
                //getDipoleOpenBabel(fileOut, &seModel->molecule, seModel->molecule.dipole);
                computeMoleculeSEDipole(&seModel->molecule);
                free(fileOut);
        }


}
/**********************************************************************/
SemiEmpiricalModel createOpenBabelModel (GeomDef* geom,gint Natoms,gint charge, gint spin, gchar* method, gchar* dirName, SemiEmpiricalModelConstraints constraints)
{
	SemiEmpiricalModel seModel = newOpenBabelModel(method,dirName, constraints);

	seModel.molecule = createMoleculeSE(geom,Natoms, charge, spin,TRUE);
	setRattleConstraintsParameters(&seModel);
	
	return seModel;
}
/**********************************************************************/
static gboolean getDipoleGeneric(char* fileNameOut, double* dipole)
{
	FILE* file = NULL;
	char buffer[1024];
	int i;
 	file = fopen(fileNameOut, "r");
	if(!file) return FALSE;
	if(!fgets(buffer,BSIZE,file)) { fclose(file); return FALSE;}/* first line for energy in Hartree*/
	if(!fgets(buffer,BSIZE,file)) { fclose(file); return FALSE;}/* dipole in au */
	for(i=0;i<strlen(buffer);i++) if(buffer[i]=='D' || buffer[i]=='d') buffer[i] ='E';
	if(sscanf(buffer,"%lf %lf %lf",&dipole[0],&dipole[1],&dipole[2])==3)
	{
		for(i=0;i<3;i++) dipole[i] *= AUTODEB;
		fclose(file);
		return TRUE;
	}
	fclose(file);
	return FALSE;
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
		*energy *= AUTOKCAL;
		return TRUE;
	}
	fclose(file);
	return FALSE;
}
/*****************************************************************************/
gboolean getGradientGeneric(char* fileNameOut, SemiEmpiricalModel *seModel)
{
	FILE* file = NULL;
	char buffer[1024];
	gboolean Ok = FALSE;
	int i;
	int j;

 	file = fopen(fileNameOut, "r");
	if(!file) return FALSE;
	if(!fgets(buffer,BSIZE,file)) { fclose(file); return FALSE;}/* first line for energy in Hartree*/
	if(!fgets(buffer,BSIZE,file)) { fclose(file); return FALSE;}/* dipole in au */
	for(i=0;i<seModel->molecule.nAtoms;i++)
	{
		if(!fgets(buffer,BSIZE,file))break;
		for(j=0;j<strlen(buffer);j++) if(buffer[j]=='D' || buffer[j]=='d') buffer[j] ='E';
		if(sscanf(buffer,"%lf %lf %lf",
					&seModel->molecule.gradient[0][i],
					&seModel->molecule.gradient[1][i],
					&seModel->molecule.gradient[2][i]
					)!=3)
		{
			fclose(file);
			return FALSE;
		}
		for(j=0;j<3;j++) seModel->molecule.gradient[j][i] *= AUTOKCAL/BOHR_TO_ANG;
		for(j=0;j<3;j++) seModel->molecule.gradient[j][i] = - seModel->molecule.gradient[j][i];
	}
	Ok = TRUE;
	fclose(file);
	return Ok;
}
/*****************************************************************************/
char* runOneGeneric(SemiEmpiricalModel* seModel, char* keyWords)
{
	FILE* file = NULL;
	FILE* fileSH = NULL;
	char* fileNameIn = NULL;
	char* fileNameOut = NULL;
	char* fileNameSH = NULL;
	char buffer[1024];
	MoleculeSE* mol = &seModel->molecule;
	char* NameCommandGeneric = seModel->method;
	int rank = 0;
	int type = 0;
#ifdef ENABLE_MPI
	MPI_Comm_rank( MPI_COMM_WORLD,&rank);
#endif
#ifdef OS_WIN32
	char c='%';
#endif

	if(mol->nAtoms<1) return fileNameOut;
#ifndef OS_WIN32
	fileNameSH = g_strdup_printf("%s%sGenericOne%d.sh",seModel->workDir,G_DIR_SEPARATOR_S,rank);
#else
	fileNameSH = g_strdup_printf("%s%sGenericOne%d.bat",seModel->workDir,G_DIR_SEPARATOR_S,rank);
#endif
 	fileSH = fopen(fileNameSH, "w");
	if(!fileSH) return FALSE;
#ifdef OS_WIN32
	fprintf(fileSH,"@echo off\n");
#endif

	fileNameIn = g_strdup_printf("%s%sOne%d.inp",seModel->workDir,G_DIR_SEPARATOR_S,rank);
	fileNameOut = g_strdup_printf("%s%sOne%d.out",seModel->workDir,G_DIR_SEPARATOR_S,rank);

 	file = fopen(fileNameIn, "w");
	if(!file) 
	{
 		if(fileNameIn) free(fileNameIn);
 		if(fileNameOut) free(fileNameOut);
 		if(fileNameSH) free(fileNameSH);
		return FALSE;
	}
	/*
	fprintf(file,"# ======================================================\n");
	fprintf(file,"#  Generic input file made in CChemI\n"); 
	fprintf(file,"# ======================================================\n");
	*/
	if(strstr(keyWords,"ENGRAD")) type = 1;
	fprintf(file,"%d\n",type);
	addMoleculeSEToFile(mol,file);
	fclose(file);

#ifndef OS_WIN32
	fprintf(fileSH,"%s %s %s",NameCommandGeneric,fileNameIn,fileNameOut);
	fclose(fileSH);
	sprintf(buffer,"chmod u+x %s",fileNameSH);
	system(buffer);
	system(fileNameSH);
#else
	fprintf(fileSH,"\"%s\" \"%s\" \"%s\"",NameCommandGeneric,fileNameIn,fileNameOut);
	fclose(fileSH);
	sprintf(buffer,"\"%s\"",fileNameSH);
	system(buffer);
#endif
	unlink(fileNameIn);
	unlink(fileNameSH);
 	if(fileNameIn) free(fileNameIn);
 	if(fileNameSH) free(fileNameSH);
	return fileNameOut;
}
/**********************************************************************/
static SemiEmpiricalModel newGenericModel(char* method, char* dirName, SemiEmpiricalModelConstraints constraints)
{
	/* method = nameCommand */
	SemiEmpiricalModel seModel = newSemiEmpiricalModel(method, dirName, constraints);

	seModel.klass->calculateGradient = calculateGradientGeneric;
	seModel.klass->calculateEnergy = calculateEnergyGeneric;

	return seModel;
}
/**********************************************************************/
static void calculateGradientGeneric(SemiEmpiricalModel* seModel)
{
	int i;
	int j;
	MoleculeSE m = seModel->molecule;
	char* keyWords = NULL;
	char* fileOut = NULL;
	if(!seModel) return;
	if(seModel->molecule.nAtoms<1) return;
	if(!seModel->method) return;
	keyWords = g_strdup_printf("%s ENGRAD ",seModel->method);
	fileOut = runOneGeneric(seModel, keyWords);

	if(fileOut)
	{
		for(j=0;j<3;j++)
			for( i=0; i<m.nAtoms;i++)
				m.gradient[j][i] = 0.0;
		if(!getGradientGeneric(fileOut, seModel))
		{
#ifdef OS_WIN32
			char* comm = g_strdup_printf("type %s",fileOut);
#else
			char* comm = g_strdup_printf("cat %s",fileOut);
#endif
			printf(("Problem : I cannot calculate the Gradient... "));
			printf(("Calculation Stopped "));
			system(comm);
			free(fileOut);
			free(comm);
			exit(1);
			return;
		}
		getEnergyGeneric(fileOut, &seModel->molecule.energy);
		getDipoleGeneric(fileOut, seModel->molecule.dipole);
		free(fileOut);
	}

}
/**********************************************************************/
static void calculateEnergyGeneric(SemiEmpiricalModel* seModel)
{
	char* keyWords = NULL;
	char* fileOut = NULL;
	if(!seModel) return;
	if(seModel->molecule.nAtoms<1) return;
	if(!seModel->method) return;
	keyWords = g_strdup_printf("%s ",seModel->method);
	fileOut = runOneGeneric(seModel, keyWords);
	if(fileOut)
	{
		getEnergyGeneric(fileOut, &seModel->molecule.energy);
		getDipoleGeneric(fileOut, seModel->molecule.dipole);
		free(fileOut);
	}

}
/**********************************************************************/
SemiEmpiricalModel createGenericModel (GeomDef* geom,gint Natoms,gint charge, gint spin, gchar* method, gchar* dirName, SemiEmpiricalModelConstraints constraints)
{
	SemiEmpiricalModel seModel = newGenericModel(method,dirName, constraints);
	seModel.molecule = createMoleculeSE(geom,Natoms, charge, spin,TRUE);
	setRattleConstraintsParameters(&seModel);
	
	return seModel;
}
