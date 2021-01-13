/* LoadMMParameters.c */
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
#include "../Utils/Constants.h"
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
static gboolean readAmberTypes(AmberParameters* amberParameters, FILE* file)
{
	gchar t[BSIZE];
	gchar dumpName[BSIZE];
	gchar dumpSymb[BSIZE];
	gchar dumpDesc[BSIZE];
	gint len = BSIZE;
	gboolean Ok = FALSE;
	gint n = 0;
	AmberAtomTypes* types = NULL;
	gint np;
	/* Search Begin INPUT FOR  ATOM TYPES */ 

	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,atomTypesTitle))
			{
				Ok = TRUE;
				break;
			}
		}
	}
	if(!Ok)
		return FALSE;

	types = g_malloc(sizeof(AmberAtomTypes));
	n = 0;
	Ok = FALSE;
	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,"End"))
			{
				Ok = TRUE;
				break;
			}
		}
		else 
		{
			Ok = FALSE;
			break;
		}



		np = sscanf(t,"%s %s %d %lf %lf %s",
			dumpName,
			dumpSymb,
			&types[n].number,
			&types[n].masse,
			&types[n].polarisability,dumpDesc);

	      	types[n].name = g_strdup(dumpName);	
	      	types[n].symbol = g_strdup(dumpSymb);
		types[n].number--;
		/*printf("t=%s\n",t);*/
		/*printf("dumpDesc=%s\n",dumpDesc);*/
		if(np>=6) 
		{
			gchar** strsplit;
			gint ls,l;
			np = 0;
			types[n].description = NULL;

			ls = strlen(t);
			for(l = 0;l<ls;l++) if(t[l]=='\t') t[l]=' ';
			strsplit = gab_split(t);

			while(strsplit[np])
			{
				/*printf("===> %s ", strsplit[np]);*/
				if( np == 5 ) { types[n].description = g_strdup(strsplit[np]);}
				if(np>5 && types[n].description) {
					gchar* tmp = types[n].description;
					types[n].description = g_strdup_printf("%s %s",tmp,strsplit[np]);
					g_free(tmp);
				}
				np++;
			}
			/*printf("===>\n");*/
			if(!types[n].description) types[n].description =  g_strdup(dumpSymb);
			ls = strlen(types[n].description);
			for(l = 0;l<ls;l++) if(types[n].description[l]=='\n') types[n].description[l]='\0';
			g_strfreev(strsplit);
			
		}
		else types[n].description =  g_strdup(dumpSymb);
		

		n++;
		types = g_realloc(types,(n+1)*sizeof(AmberAtomTypes));
	}
	if(n==0 || !Ok )
		g_free(types);
	else
	{
		amberParameters->numberOfTypes = n;
		amberParameters->atomTypes = types;
	}
	/* printing for test*/
	/*
	printf("umber of types = %d \n",amberParameters->numberOfTypes);
	for(n=0;n<amberParameters->numberOfTypes;n++)
	{
		printf("%s\t %d\t",
				amberParameters->atomTypes[n].name,
				amberParameters->atomTypes[n].number
				);
	}
	printf("\n");
	*/

	return TRUE;
			

}
/**********************************************************************/
static gboolean readAmberBondStretchTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[BSIZE];
	gint len = BSIZE;
	gboolean Ok = FALSE;
	gint n = 0;
	AmberBondStretchTerms* terms = NULL;

	/* Search Begin INPUT FOR  ATOM TYPES */ 

	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,bondStretchTitle))
			{
				Ok = TRUE;
				break;
			}
		}
	}
	if(!Ok)
		return FALSE;

	terms = g_malloc(sizeof(AmberBondStretchTerms));
	n = 0;
	Ok = FALSE;
	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,"End"))
			{
				Ok = TRUE;
				break;
			}
		}
		else 
		{
			Ok = FALSE;
			break;
		}

		sscanf(t,"%d %d %lf %lf",
			&terms[n].numbers[0],
			&terms[n].numbers[1],
			&terms[n].forceConstant,
			&terms[n].equilibriumDistance);

	      	terms[n].numbers[0]--;
	      	terms[n].numbers[1]--;
		if(terms[n].numbers[0]>terms[n].numbers[1])
		{
			gint t = terms[n].numbers[0];
			terms[n].numbers[0] = terms[n].numbers[1];
			terms[n].numbers[1] = t;
		}

		n++;
		terms = g_realloc(terms,(n+1)*sizeof(AmberBondStretchTerms));
	}
	if(n==0 || !Ok )
		g_free(terms);
	else
	{
		amberParameters->numberOfStretchTerms = n;
		amberParameters->bondStretchTerms = terms;
	}
	/* printing for test*/
	/*
	printf("number of bonds = %d \n",amberParameters->numberOfStretchTerms);
	for(n=0;n<amberParameters->numberOfStretchTerms;n++)
	{
		printf("%d %d %f %f\n",
				amberParameters->bondStretchTerms[n].numbers[0],
				amberParameters->bondStretchTerms[n].numbers[1],
				amberParameters->bondStretchTerms[n].forceConstant,
				amberParameters->bondStretchTerms[n].equilibriumDistance
				);
	}
	printf("\n");
	*/

	return TRUE;
			

}
/**********************************************************************/
static gboolean readAmberAngleBendTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[BSIZE];
	gint len = BSIZE;
	gboolean Ok = FALSE;
	gint n = 0;
	AmberAngleBendTerms* terms = NULL;

	/* Search Begin INPUT FOR  ATOM TYPES */ 

	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,angleBendTitle))
			{
				Ok = TRUE;
				break;
			}
		}
	}
	if(!Ok)
		return FALSE;

	terms = g_malloc(sizeof(AmberAngleBendTerms));
	n = 0;
	Ok = FALSE;
	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,"End"))
			{
				Ok = TRUE;
				break;
			}
		}
		else 
		{
			Ok = FALSE;
			break;
		}

		sscanf(t,"%d %d %d %lf %lf",
				&terms[n].numbers[0],
				&terms[n].numbers[1],
				&terms[n].numbers[2],
				&terms[n].forceConstant,
				&terms[n].equilibriumAngle);
		terms[n].numbers[0]--;
		terms[n].numbers[1]--;
		terms[n].numbers[2]--;
		if(terms[n].numbers[0]>terms[n].numbers[2])
		{
			gint t = terms[n].numbers[0];
			terms[n].numbers[0] = terms[n].numbers[2];
			terms[n].numbers[2] = t;
		}
		

		n++;
		terms = g_realloc(terms,(n+1)*sizeof(AmberAngleBendTerms));
	}
	if(n==0 || !Ok )
		g_free(terms);
	else
	{
		amberParameters->numberOfBendTerms = n;
		amberParameters->angleBendTerms = terms;
	}
	/* printing for test*/
	/*
	printf("number of bonds = %d \n",amberParameters->numberOfBendTerms);
	for(n=0;n<amberParameters->numberOfBendTerms;n++)
	{
		printf("%d %d %d %f %f\n",
				amberParameters->angleBendTerms[n].numbers[0],
				amberParameters->angleBendTerms[n].numbers[1],
				amberParameters->angleBendTerms[n].numbers[2],
				amberParameters->angleBendTerms[n].forceConstant,
				amberParameters->angleBendTerms[n].equilibriumAngle
				);
	}
	printf("\n");
	*/

	return TRUE;
			

}
/**********************************************************************/
static gboolean readAmberDihedralAngleTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[BSIZE];
	gint len = BSIZE;
	gboolean Ok = FALSE;
	gint n = 0;
	AmberDihedralAngleTerms* terms = NULL;
	gdouble divisor = 1;
	gdouble barrier = 0;
	gdouble phase = 0;
	gdouble nN = 0;
	gint d;

	/* Search Begin INPUT FOR  DIHEDRAL PARAMETERS */

	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,dihedralAngleTitle))
			{
				Ok = TRUE;
				break;
			}
		}
	}
	if(!Ok)
		return FALSE;

	terms = g_malloc(sizeof(AmberDihedralAngleTerms));

	n = 0;
	Ok = FALSE;
	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,"End"))
			{
				Ok = TRUE;
				break;
			}
		}
		else 
		{
			Ok = FALSE;
			break;
		}

		terms[n].nSomme = 1;
		terms[n].divisor = g_malloc(sizeof(gdouble));
		terms[n].barrier = g_malloc(sizeof(gdouble));
		terms[n].phase   = g_malloc(sizeof(gdouble));
		terms[n].n       = g_malloc(sizeof(gdouble));

		sscanf(t,"%d %d %d %d %lf %lf %lf %lf",
			&terms[n].numbers[0],
			&terms[n].numbers[1],
			&terms[n].numbers[2],
			&terms[n].numbers[3],
			&divisor,
			&barrier,
			&phase,
			&nN);

		terms[n].divisor[0] = divisor;
		terms[n].barrier[0] = barrier;
		terms[n].phase[0]   = phase;
		terms[n].n[0]       = fabs(nN);

	    terms[n].numbers[0]--;
	    terms[n].numbers[1]--;
	    terms[n].numbers[2]--;
	    terms[n].numbers[3]--;

	    if(terms[n].numbers[0]>terms[n].numbers[3])
	    {
		gint t = terms[n].numbers[0];
		terms[n].numbers[0] = terms[n].numbers[3];
		terms[n].numbers[3] = t;

		t =  terms[n].numbers[1];
		terms[n].numbers[1] = terms[n].numbers[2];
		terms[n].numbers[2] = t;
	     }

		Ok = TRUE;
		while(!feof(file) && nN<0)
		{
			if(!fgets(t,len,file))
			{
				Ok = FALSE;
				break;
			}

			terms[n].nSomme++;
			terms[n].divisor = g_realloc(terms[n].divisor,terms[n].nSomme*sizeof(gdouble));
			terms[n].barrier = g_realloc(terms[n].barrier,terms[n].nSomme*sizeof(gdouble));
			terms[n].phase   = g_realloc(terms[n].phase,terms[n].nSomme*sizeof(gdouble));
			terms[n].n       = g_realloc(terms[n].n,terms[n].nSomme*sizeof(gdouble));

			sscanf(t,"%d %d %d %d %lf %lf %lf %lf",
					  &d,&d,&d,&d,
					  &divisor,&barrier,&phase,&nN);

			terms[n].divisor[terms[n].nSomme-1] = divisor;
			terms[n].barrier[terms[n].nSomme-1] = barrier;
			terms[n].phase[terms[n].nSomme-1]   = phase;
			terms[n].n[terms[n].nSomme-1]       = fabs(nN);
		}
		if(!Ok)
			break;
		n++;
		terms = g_realloc(terms,(n+1)*sizeof(AmberDihedralAngleTerms));
	}
	if(n==0 || !Ok )
		g_free(terms);
	else
	{
		amberParameters->numberOfDihedralTerms = n;
		amberParameters->dihedralAngleTerms = terms;
	}
	/* printing for test*/
	/*	
	printf("number of dihedral torsion terms = %d \n",
			amberParameters->numberOfDihedralTerms);

	for(n=0;n<amberParameters->numberOfDihedralTerms;n++)
	{
		gint j;
		printf("%d %d %d %d \t",
				amberParameters->dihedralAngleTerms[n].numbers[0],
				amberParameters->dihedralAngleTerms[n].numbers[1],
				amberParameters->dihedralAngleTerms[n].numbers[2],
				amberParameters->dihedralAngleTerms[n].numbers[3]
			);
		for(j=0;j<amberParameters->dihedralAngleTerms[n].nSomme;j++)
		{
			printf("%f %f %f %f\t",
				amberParameters->dihedralAngleTerms[n].divisor[j],
				amberParameters->dihedralAngleTerms[n].barrier[j],
				amberParameters->dihedralAngleTerms[n].phase[j],
				amberParameters->dihedralAngleTerms[n].n[j]
				);
		}
		printf("\n");
	}
	printf("\n");
	*/	

	return TRUE;
			

}
/**********************************************************************/
static gboolean readAmberImproperTorsionTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[BSIZE];
	gint len = BSIZE;
	gboolean Ok = FALSE;
	gint n = 0;
	AmberImproperTorsionTerms* terms = NULL;

	/* Search Begin INPUT FOR  ATOM TYPES */ 

	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,improperTorsionTitle))
			{
				Ok = TRUE;
				break;
			}
		}
	}
	if(!Ok)
		return FALSE;

	terms = g_malloc(sizeof(AmberImproperTorsionTerms));
	n = 0;
	Ok = FALSE;
	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,"End"))
			{
				Ok = TRUE;
				break;
			}
		}
		else 
		{
			Ok = FALSE;
			break;
		}



		sscanf(t,"%d %d %d %d %lf %lf %lf",
			&terms[n].numbers[0],
			&terms[n].numbers[1],
			&terms[n].numbers[2],
			&terms[n].numbers[3],
			&terms[n].barrier,
			&terms[n].phase,
			&terms[n].n);

	    terms[n].numbers[0]--;
	    terms[n].numbers[1]--;
	    terms[n].numbers[2]--;
	    terms[n].numbers[3]--;
	    if(terms[n].numbers[0]>terms[n].numbers[3])
	    {
		gint t = terms[n].numbers[0];
		terms[n].numbers[0] = terms[n].numbers[3];
		terms[n].numbers[3] = t;

		t =  terms[n].numbers[1];
		terms[n].numbers[1] = terms[n].numbers[2];
		terms[n].numbers[2] = t;
	    }

		n++;
		terms = g_realloc(terms,(n+1)*sizeof(AmberImproperTorsionTerms));
	}
	if(n==0 || !Ok )
		g_free(terms);
	else
	{
		amberParameters->numberOfImproperTorsionTerms = n;
		amberParameters->improperTorsionTerms = terms;
	}
	/* printing for test*/
	/*
	printf("number of improper torsion terms = %d \n",
			amberParameters->numberOfImproperTorsionTerms);

	for(n=0;n<amberParameters->numberOfImproperTorsionTerms;n++)
	{
		printf("%d %d %d %d %f %f %f\n",
				amberParameters->improperTorsionTerms[n].numbers[0],
				amberParameters->improperTorsionTerms[n].numbers[1],
				amberParameters->improperTorsionTerms[n].numbers[2],
				amberParameters->improperTorsionTerms[n].numbers[3],
				amberParameters->improperTorsionTerms[n].barrier,
				amberParameters->improperTorsionTerms[n].phase,
				amberParameters->improperTorsionTerms[n].n
				);
	}
	printf("\n");
	*/

	return TRUE;
			

}
/**********************************************************************/
static gboolean readAmberHydrogenBondedTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[BSIZE];
	gint len = BSIZE;
	gboolean Ok = FALSE;
	gint n = 0;
	AmberHydrogenBondedTerms* terms = NULL;

	/* Search Begin INPUT FOR  ATOM TYPES */ 

	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,hydrogenBondedTitle))
			{
				Ok = TRUE;
				break;
			}
		}
	}
	if(!Ok)
		return FALSE;

	terms = g_malloc(sizeof(AmberHydrogenBondedTerms));
	n = 0;
	Ok = FALSE;
	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,"End"))
			{
				Ok = TRUE;
				break;
			}
		}
		else 
		{
			Ok = FALSE;
			break;
		}

		sscanf(t,"%d %d %lf %lf",
				&terms[n].numbers[0],
				&terms[n].numbers[1],
				&terms[n].c,
				&terms[n].d);

		terms[n].numbers[0]--;
		terms[n].numbers[1]--;
		if(terms[n].numbers[0]>terms[n].numbers[1])
		{
			gint t = terms[n].numbers[0];
			terms[n].numbers[0] = terms[n].numbers[1];
			terms[n].numbers[1] = t;
		}

		n++;
		terms = g_realloc(terms,(n+1)*sizeof(AmberHydrogenBondedTerms));
	}
	if(n==0 || !Ok )
		g_free(terms);
	else
	{
		amberParameters->numberOfHydrogenBonded = n;
		amberParameters->hydrogenBondedTerms = terms;
	}
	/* printing for test*/
	/*
	printf("number of hydrogen bonds terms = %d \n",amberParameters->numberOfHydrogenBonded);
	for(n=0;n<amberParameters->numberOfHydrogenBonded;n++)
	{
		printf("%d %d %f %f\n",
				amberParameters->hydrogenBondedTerms[n].numbers[0],
				amberParameters->hydrogenBondedTerms[n].numbers[1],
				amberParameters->hydrogenBondedTerms[n].c,
				amberParameters->hydrogenBondedTerms[n].d
				);
	}
	printf("\n");
	*/

	return TRUE;
			

}
/**********************************************************************/
static gboolean readAmberNonBondedTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[BSIZE];
	gint len = BSIZE;
	gboolean Ok = FALSE;
	gint n = 0;
	AmberNonBondedTerms* terms = NULL;

	/* Search Begin INPUT FOR  NON-BONDED  */ 
	Ok = FALSE;
	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,nonBondedTitle))
			{
				Ok = TRUE;
				break;
			}
		}
	}
	if(!Ok)
		return FALSE;

	terms = g_malloc(sizeof(AmberNonBondedTerms));
	n = 0;
	Ok = FALSE;
	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,"End"))
			{
				Ok = TRUE;
				break;
			}
		}
		else 
		{
			Ok = FALSE;
			break;
		}



		sscanf(t,"%d %lf %lf",
			&terms[n].number,
			&terms[n].r,
			&terms[n].epsilon);
		
		terms[n].number--;
		n++;
		terms = g_realloc(terms,(n+1)*sizeof(AmberNonBondedTerms));
	}

	if(n==0 || !Ok )
		g_free(terms);
	else
	{
		amberParameters->numberOfNonBonded = n;
		amberParameters->nonBondedTerms = terms;
	}
	/* printing for test*/
	/*
	printf("number of non bended terms = %d \n",amberParameters->numberOfNonBonded);
	for(n=0;n<amberParameters->numberOfNonBonded;n++)
	{
		printf("%d %f %f\n",
				amberParameters->nonBondedTerms[n].number,
				amberParameters->nonBondedTerms[n].r,
				amberParameters->nonBondedTerms[n].epsilon
				);
	}
	printf("\n");
	*/

	return TRUE;
			

}
/**********************************************************************/
static gboolean readAmberPairWiseTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[BSIZE];
	gint len = BSIZE;
	gboolean Ok = FALSE;
	gint n = 0;
	AmberPairWiseTerms* terms = NULL;

	/* Search Begin INPUT FOR PAIR WIZE  */ 
	Ok = FALSE;
	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,pairWiseTitle))
			{
				Ok = TRUE;
				break;
			}
		}
	}
	if(!Ok)
		return FALSE;

	terms = g_malloc(sizeof(AmberPairWiseTerms));
	n = 0;
	Ok = FALSE;
	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,"End"))
			{
				Ok = TRUE;
				break;
			}
		}
		else 
		{
			Ok = FALSE;
			break;
		}



		sscanf(t,"%d %d %lf %lf %lf %lf %lf %lf",
			&terms[n].numbers[0],
			&terms[n].numbers[1],
			&terms[n].a,
			&terms[n].beta,
			&terms[n].c6,
			&terms[n].c8,
			&terms[n].c10,
			&terms[n].b
			);
		
		terms[n].numbers[0]--;
		terms[n].numbers[1]--;
		n++;
		terms = g_realloc(terms,(n+1)*sizeof(AmberPairWiseTerms));
	}

	if(n==0 || !Ok )
		g_free(terms);
	else
	{
		amberParameters->numberOfPairWise = n;
		amberParameters->pairWiseTerms = terms;
	}
	/* printing for test*/
	/*
	printf("number of pair wise terms = %d \n",amberParameters->numberOfPairWise);
	for(n=0;n<amberParameters->numberOfPairWise;n++)
	{
		printf("%d %d %f %f %f %f %d\n",
				amberParameters->pairWiseTerms[n].numbers[0],
				amberParameters->pairWiseTerms[n].numbers[1],
				amberParameters->pairWiseTerms[n].a,
				amberParameters->pairWiseTerms[n].beta,
				amberParameters->pairWiseTerms[n].c6,
				amberParameters->pairWiseTerms[n].c8,
				amberParameters->pairWiseTerms[n].c10,
				amberParameters->pairWiseTerms[n].b,
				);
	}
	printf("\n");
	*/

	return TRUE;
			

}
/**********************************************************************/
gboolean readAmberParameters(AmberParameters* amberParameters,gchar* filename)
{
	FILE* file;
	file = FOpen(filename,"rb");

	/* printf("filename = %s\n",filename);*/
	if(file == NULL)
		return FALSE;
	else
	{
		readAmberTypes(amberParameters,file);
		readAmberBondStretchTerms(amberParameters,file);
		readAmberAngleBendTerms(amberParameters,file);
		readAmberDihedralAngleTerms(amberParameters,file);
		readAmberImproperTorsionTerms(amberParameters,file);
		readAmberHydrogenBondedTerms(amberParameters,file);
		readAmberNonBondedTerms(amberParameters,file);
		readAmberPairWiseTerms(amberParameters,file);
		fclose(file);
	}
	return TRUE;
}
/**********************************************************************/
