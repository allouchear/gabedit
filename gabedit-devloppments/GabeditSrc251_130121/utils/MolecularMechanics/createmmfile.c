#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <glib.h>
#include <math.h>

typedef struct _AmberParameters  AmberParameters;
typedef struct _AmberAtomTypes  AmberAtomTypes;
typedef struct _AmberBondStretchTerms  AmberBondStretchTerms;
typedef struct _AmberAngleBendTerms  AmberAngleBendTerms;
typedef struct _AmberDihedralAngleTerms  AmberDihedralAngleTerms;
typedef struct _AmberImproperTorsionTerms  AmberImproperTorsionTerms;
typedef struct _AmberNonBondedTerms  AmberNonBondedTerms;
typedef struct _AmberHydrogenBondedTerms  AmberHydrogenBondedTerms;

/************************************/
struct _AmberAtomTypes
{
	gchar* name;
	gint number;
	gdouble masse;
	gdouble polarisability;
	gchar* c;
};
/************************************/
struct _AmberBondStretchTerms
{
	gint numbers[2];
	gdouble equilibriumDistance;
	gdouble forceConstant;
	gchar* c;
};
/************************************/
struct _AmberAngleBendTerms
{
	gint numbers[3];
	gdouble equilibriumAngle;
	gdouble forceConstant;
	gchar* c;
};
/************************************/
struct _AmberDihedralAngleTerms
{
	gint numbers[4];
	gint nSomme;
	gdouble* divisor;
	gdouble* barrier;
	gdouble* phase;
	gdouble* n;
	gchar* c;
};
/************************************/
struct _AmberImproperTorsionTerms
{
	gint numbers[4];
	gdouble barrier;
	gdouble phase;
	gdouble n;
	gchar* c;
};
/************************************/
struct _AmberNonBondedTerms
{
	gint number;
	gdouble r;
	gdouble epsilon;
	gchar* c;
};
/************************************/
struct _AmberHydrogenBondedTerms
{
	gint numbers[2];
	gdouble c;
	gdouble d;
	gchar* cc;
};
/************************************/
struct _AmberParameters
{
	gint numberOfTypes;
	AmberAtomTypes* atomTypes;

	gint numberOfStretchTerms;
	AmberBondStretchTerms* bondStretchTerms;

	gint numberOfBendTerms;
	AmberAngleBendTerms* angleBendTerms;

	gint numberOfDihedralTerms;
	AmberDihedralAngleTerms* dihedralAngleTerms;

	gint numberOfImproperTorsionTerms;
	AmberImproperTorsionTerms* improperTorsionTerms;

	gint numberOfNonBonded;
	AmberNonBondedTerms* nonBondedTerms;

	gint numberOfHydrogenBonded;
	AmberHydrogenBondedTerms* hydrogenBondedTerms;
};
/************************************/

static gchar atomTypesTitle[]       = "Begin  INPUT FOR ATOM TYPES, MASSE AND POLARISABILITIES";
static gchar bondStretchTitle[]     = "Begin INPUT FOR BOND LENGTH PARAMETERS";
static gchar angleBendTitle[]       = "Begin INPUT FOR BOND ANGLE PARAMETERS";
static gchar hydrogenBondedTitle[]  = "Begin INPUT FOR H-BOND 10-12 POTENTIAL PARAMETERS";
static gchar improperTorsionTitle[] ="Begin INPUT FOR IMPROPER DIHEDRAL PARAMETERS";
static gchar nonBondedTitle[]       ="Begin INPUT FOR THE NON-BONDED 6-12 POTENTIAL PARAMETERS";
static gchar nonBondedEquivTitle[]  ="Begin INPUT FOR EQUIVALENCING ATOM TYPES FOR THE NON-BONDED 6-12 POTENTIAL PARAMETERS";
static gchar dihedralAngleTitle[]   = "Begin INPUT FOR DIHEDRAL PARAMETERS";


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


	return amberParameters;
	
}
/**********************************************************************/
void freeAmberParameters(AmberParameters* amberParameters)
{
	gint i;

	for(i=0;i<amberParameters->numberOfTypes;i++)
		if(amberParameters->atomTypes[i].name)
			g_free(amberParameters->atomTypes[i].name);

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
gint getNumberType(AmberParameters* amberParameters, gchar* type)
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
gboolean readAmberTypes(AmberParameters* amberParameters, FILE* file)
{
	gchar t[1024];
	
	gchar dump[1024];
	gint len = 1024;
	gboolean Ok = FALSE;
	gint n = 0;
	AmberAtomTypes* types = NULL;
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



		sscanf(t,"%s %lf %lf",dump,&types[n].masse,&types[n].polarisability);
	      	types[n].name = g_strdup(dump);	
			types[n].c = g_strdup(t+37);
			types[n].c[strlen(types[n].c)-1]='\0';
	      	types[n].number = n;	

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
gboolean readAmberBondStretchTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[1024];
	gchar dump1[1024];
	gchar dump2[1024];
	gint len = 1024;
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



		dump1[2]='\0';
		dump2[2]='\0';
		sscanf(t,"%c%c-%c%c %lf %lf",&dump1[0],&dump1[1],&dump2[0],&dump2[1],
				&terms[n].forceConstant,
				&terms[n].equilibriumDistance);
		if(strlen(t)>28)
			terms[n].c = g_strdup(t+28);
		else
			terms[n].c = g_strdup("  ");

		terms[n].c[strlen(terms[n].c)-1]='\0';
		if(dump1[1]==' ') dump1[1]='\0';
		if(dump2[1]==' ') dump2[1]='\0';
	      	terms[n].numbers[0] = getNumberType(amberParameters,dump1);	
	      	terms[n].numbers[1] = getNumberType(amberParameters,dump2);	

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
gboolean readAmberAngleBendTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[1024];
	gchar dump1[10];
	gchar dump2[10];
	gchar dump3[10];
	gint len = 1024;
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



		dump1[2]='\0';
		dump2[2]='\0';
		dump3[2]='\0';
		sscanf(t,"%c%c-%c%c-%c%c %lf %lf",
				&dump1[0],&dump1[1],&dump2[0],&dump2[1],&dump3[0],&dump3[1],
				&terms[n].forceConstant,
				&terms[n].equilibriumAngle);

		if(strlen(t)>32)
			terms[n].c = g_strdup(t+32);
		else
			terms[n].c = g_strdup("  ");

		terms[n].c[strlen(terms[n].c)-1]='\0';

		if(dump1[1]==' ') dump1[1]='\0';
		if(dump2[1]==' ') dump2[1]='\0';
		if(dump3[1]==' ') dump3[1]='\0';

	      	terms[n].numbers[0] = getNumberType(amberParameters,dump1);	
	      	terms[n].numbers[1] = getNumberType(amberParameters,dump2);	
	      	terms[n].numbers[2] = getNumberType(amberParameters,dump3);	

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
gboolean readAmberDihedralAngleTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[1024];
	gchar dump1[10];
	gchar dump2[10];
	gchar dump3[10];
	gchar dump4[10];
	gint len = 1024;
	gboolean Ok = FALSE;
	gint n = 0;
	AmberDihedralAngleTerms* terms = NULL;
	gdouble divisor = 1;
	gdouble barrier = 0;
	gdouble phase = 0;
	gdouble nN = 0;

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

		dump1[2]='\0';
		dump2[2]='\0';
		dump3[2]='\0';
		dump4[2]='\0';
		sscanf(t,"%c%c-%c%c-%c%c-%c%c %lf %lf %lf %lf",
				&dump1[0],&dump1[1],
				&dump2[0],&dump2[1],
				&dump3[0],&dump3[1],
				&dump4[0],&dump4[1],
				&divisor,
				&barrier,
				&phase,
				&nN);
		terms[n].divisor[0] = divisor;
		terms[n].barrier[0] = barrier;
		terms[n].phase[0]   = phase;
		terms[n].n[0]       = fabs(nN);

		if(strlen(t)>60)
			terms[n].c = g_strdup(t+60);
		else
			terms[n].c = g_strdup("  ");

		terms[n].c[strlen(terms[n].c)-1]='\0';


		if(dump1[1]==' ') dump1[1]='\0';
		if(dump2[1]==' ') dump2[1]='\0';
		if(dump3[1]==' ') dump3[1]='\0';
		if(dump4[1]==' ') dump4[1]='\0';

	    terms[n].numbers[0] = getNumberType(amberParameters,dump1);
	    terms[n].numbers[1] = getNumberType(amberParameters,dump2);
	    terms[n].numbers[2] = getNumberType(amberParameters,dump3);
	    terms[n].numbers[3] = getNumberType(amberParameters,dump4);

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

				sscanf(t,"%c%c-%c%c-%c%c-%c%c %lf %lf %lf %lf",
						&dump1[0],&dump1[1],
						&dump2[0],&dump2[1],
						&dump3[0],&dump3[1],
						&dump4[0],&dump4[1],
						&divisor,
						&barrier,
						&phase,
						&nN);
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
gboolean readAmberImproperTorsionTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[1024];
	gchar dump1[10];
	gchar dump2[10];
	gchar dump3[10];
	gchar dump4[10];
	gint len = 1024;
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



		dump1[2]='\0';
		dump2[2]='\0';
		dump3[2]='\0';
		dump4[2]='\0';
		sscanf(t,"%c%c-%c%c-%c%c-%c%c %lf %lf %lf",
				&dump1[0],&dump1[1],
				&dump2[0],&dump2[1],
				&dump3[0],&dump3[1],
				&dump4[0],&dump4[1],
				&terms[n].barrier,
				&terms[n].phase,
				&terms[n].n);

		if(strlen(t)>60)
			terms[n].c = g_strdup(t+60);
		else
			terms[n].c = g_strdup("  ");

		terms[n].c[strlen(terms[n].c)-1]='\0';

		if(dump1[1]==' ') dump1[1]='\0';
		if(dump2[1]==' ') dump2[1]='\0';
		if(dump3[1]==' ') dump3[1]='\0';
		if(dump4[1]==' ') dump4[1]='\0';

	      	terms[n].numbers[0] = getNumberType(amberParameters,dump1);
	      	terms[n].numbers[1] = getNumberType(amberParameters,dump2);
	      	terms[n].numbers[2] = getNumberType(amberParameters,dump3);
	      	terms[n].numbers[3] = getNumberType(amberParameters,dump4);

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
gboolean readAmberHydrogenBondedTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[1024];
	gchar dump1[10];
	gchar dump2[10];
	gint len = 1024;
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



		dump1[2]='\0';
		dump2[2]='\0';
		sscanf(t,"%c%c-%c%c %lf %lf",&dump1[0],&dump1[1],&dump2[0],&dump2[1],
				&terms[n].c,
				&terms[n].d);

		if(strlen(t)>31)
			terms[n].cc = g_strdup(t+31);
		else
			terms[n].cc = g_strdup("  ");

		terms[n].cc[strlen(terms[n].cc)-1]='\0';

		if(dump1[1]==' ') dump1[1]='\0';
		if(dump2[1]==' ') dump2[1]='\0';

	      	terms[n].numbers[0] = getNumberType(amberParameters,dump1);	
	      	terms[n].numbers[1] = getNumberType(amberParameters,dump2);	

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
gboolean readAmberNonBondedTerms(AmberParameters* amberParameters,FILE* file)
{
	gchar t[1024];
	gchar dump1[1024];
	gchar dump2[1024];
	gint len = 1024;
	gboolean Ok = FALSE;
	gint n = 0;
	AmberNonBondedTerms* terms = NULL;
	gint nLigneEquiv = 0;
	gint* nTypes = NULL;
	gint **matrixNumbers = NULL;
	gchar** strLines = NULL;
	gint i;
	gint j;


	/* Search Equivalence */ 
	Ok = FALSE;
	while(!feof(file))
	{
		if(fgets(t,len,file))
		{
			if(strstr(t,nonBondedEquivTitle))
			{
				Ok = TRUE;
				break;
			}
		}
	}
	if(Ok)
	{
		/*
		*/
		strLines = g_malloc(sizeof(gint*));
		nLigneEquiv = 0;
		while(!feof(file))
		{
			if(fgets(t,len,file))
			{
				if(strstr(t,"End"))
				{
					Ok = TRUE;
					break;
				}

				strLines[nLigneEquiv] = g_strdup(t);
				nLigneEquiv++;
				strLines = g_realloc(strLines,(nLigneEquiv+1)*sizeof(gint*));
			}
			else 
			{
				Ok = FALSE;
				break;
			}
		}
	}
	if(nLigneEquiv>0)
	{
		gchar name[10];
		gchar* pos;

		strLines = g_realloc(strLines,(nLigneEquiv)*sizeof(gint*));
		nTypes = g_malloc(nLigneEquiv*sizeof(gint));
		matrixNumbers = g_malloc(sizeof(gint*));
		for(i=0;i<nLigneEquiv;i++)
		{
			nTypes[i] = (strlen(strLines[i])-2)/4+1;
			matrixNumbers[i] = g_malloc(nTypes[i]*sizeof(gint));
			pos = strLines[i];
			for(j=0;j<nTypes[i];j++)
			{
				sscanf(pos,"%s",name);
				matrixNumbers[i][j] = getNumberType(amberParameters,name);	
				pos += 4;

			}

		}
		for(i=0;i<nLigneEquiv;i++)
			if(strLines[i])
				g_free(strLines[i]);

		if(strLines)
			g_free(strLines);

	}
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



		dump1[2]='\0';
		dump2[2]='\0';
		sscanf(t,"%s %lf %lf",dump1,
				&terms[n].r,
				&terms[n].epsilon);

		if(strlen(t)>41)
			terms[n].c = g_strdup(t+41);
		else
			terms[n].c = g_strdup("  ");

		terms[n].c[strlen(terms[n].c)-1]='\0';

		if(dump1[1]==' ') dump1[1]='\0';
	      	terms[n].number = getNumberType(amberParameters,dump1);	
		n++;
		terms = g_realloc(terms,(n+1)*sizeof(AmberNonBondedTerms));
		/* equivalence */
		if(nLigneEquiv>0)
		{
			gint numberOld = terms[n-1].number;
			gint numberLine = -1;
			for(i=0;i<nLigneEquiv;i++)
			{
				for(j=0;j<nTypes[i];j++)
				{
					if(numberOld == matrixNumbers[i][j])
					{
						numberLine = i;
						break;
					}
				}
			}
			if(numberLine != -1)
			{
				for(j=0;j<nTypes[numberLine];j++)
				{
					if(
						numberOld !=  matrixNumbers[numberLine][j] &&
						matrixNumbers[numberLine][j]>-1
					)
					{
	      					terms[n].number = matrixNumbers[numberLine][j];
	      					terms[n].r = terms[n-1].r;
	      					terms[n].epsilon = terms[n-1].epsilon;
						terms[n].c = g_strdup(terms[n-1].c);
						n++;
						terms = 
						g_realloc(terms,(n+1)*sizeof(AmberNonBondedTerms));
					}

				}
			}
		}
	}
	if(nLigneEquiv>0)
	{
		for(i=0;i<nLigneEquiv;i++)
			if(matrixNumbers[i])
				g_free(matrixNumbers[i]);

	}
	if(matrixNumbers)
		g_free(matrixNumbers);
	if(nTypes)
		g_free(nTypes);

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
void readAmberParameters(AmberParameters* amberParameters)
{
	FILE* file;
	
	file = fopen("amber.prm","r");
	if(file == NULL)
		printf("file amber.prm not found\n");
	else
	{
		readAmberTypes(amberParameters,file);
		readAmberBondStretchTerms(amberParameters,file);
		readAmberAngleBendTerms(amberParameters,file);
		readAmberDihedralAngleTerms(amberParameters,file);
		readAmberImproperTorsionTerms(amberParameters,file);
		readAmberHydrogenBondedTerms(amberParameters,file);
		readAmberNonBondedTerms(amberParameters,file);
		fclose(file);
	}
}

/**********************************************************************/
gchar* getSymbol(gchar* name)
{
	gchar* t;
	if(strlen(name)==1)
		return name;
	if(strcmp(name,"C0")==0)
		return "Ca";
	if(strcmp(name,"CU")==0)
		return "Cu";
	if(strcmp(name,"FE")==0)
		return "Fe";
	if(strcmp(name,"MG")==0)
		return "Mg";
	if(strcmp(name,"IM")==0)
		return "Cl";
	if(strcmp(name,"IB")==0)
		return "I";

	if(strcmp(name,"Li")==0)
		return "Li";
	if(strcmp(name,"Na")==0)
		return "Na";
	if(strcmp(name,"IP")==0)
		return "Na";
	if(strcmp(name,"Cs")==0)
		return "Cs";
	if(strcmp(name,"Rb")==0)
		return "Rb";
	if(strcmp(name,"Zn")==0)
		return "Zn";
	if(strcmp(name,"Cl")==0)
		return "Cl";
	if(strcmp(name,"Br")==0)
		return "Br";

	t = g_strdup_printf("%c",name[0]);
	return t;
}
/**********************************************************************/
void printfMmCFileBond(FILE* file,AmberParameters* amberParameters)
{
	gint i;
	fprintf(file,"\tfprintf(fout,\"%s\\n\");\n",bondStretchTitle);

	for(i=0;i<amberParameters->numberOfStretchTerms;i++)
	{
		fprintf(file,"\tfprintf(fout,\"%d\t%d\t%6.1f\t%7.4f\t\t%s\\n\");\n",
			amberParameters->bondStretchTerms[i].numbers[0]+1,
			amberParameters->bondStretchTerms[i].numbers[1]+1,
			amberParameters->bondStretchTerms[i].forceConstant,
			amberParameters->bondStretchTerms[i].equilibriumDistance,
			amberParameters->bondStretchTerms[i].c
			);
	}
	fprintf(file,"\tfprintf(fout,\"End\\n\");\n");
}
/**********************************************************************/
void printfMmCFileBend(FILE* file,AmberParameters* amberParameters)
{
	gint n;
	fprintf(file,"\tfprintf(fout,\"%s\\n\");\n",angleBendTitle);

	for(n=0;n<amberParameters->numberOfBendTerms;n++)
	{
		fprintf(file,"\tfprintf(fout,\"%d\t%d\t%d\t%6.3f\t%6.2f\t\t%s\\n\");\n",
				amberParameters->angleBendTerms[n].numbers[0]+1,
				amberParameters->angleBendTerms[n].numbers[1]+1,
				amberParameters->angleBendTerms[n].numbers[2]+1,
				amberParameters->angleBendTerms[n].forceConstant,
				amberParameters->angleBendTerms[n].equilibriumAngle,
				amberParameters->angleBendTerms[n].c
				);
	}

	fprintf(file,"\tfprintf(fout,\"End\\n\");\n");
}
/**********************************************************************/
void printfMmCFileHydrogenBonded(FILE* file,AmberParameters* amberParameters)
{
	gint n;
	fprintf(file,"\tfprintf(fout,\"%s\\n\");\n",hydrogenBondedTitle);
	for(n=0;n<amberParameters->numberOfHydrogenBonded;n++)
	{
			fprintf(file,"\tfprintf(fout,\"%d\t%d\t%8.1f\t%8.1f\t\t%s\\n\");\n",
				amberParameters->hydrogenBondedTerms[n].numbers[0]+1,
				amberParameters->hydrogenBondedTerms[n].numbers[1]+1,
				amberParameters->hydrogenBondedTerms[n].c,
				amberParameters->hydrogenBondedTerms[n].d,
				amberParameters->hydrogenBondedTerms[n].cc
				);
	}

	fprintf(file,"\tfprintf(fout,\"End\\n\");\n");
}
/**********************************************************************/
void printfMmCFileimproperTorsion(FILE* file,AmberParameters* amberParameters)
{
	gint n;
	fprintf(file,"\tfprintf(fout,\"%s\\n\");\n",improperTorsionTitle);
	for(n=0;n<amberParameters->numberOfImproperTorsionTerms;n++)
	{
		fprintf(file,"\tfprintf(fout,\"%d\t%d\t%d\t%d\t%6.3f\t%6.2f\t%4.1f\t\t%s\\n\");\n",
				amberParameters->improperTorsionTerms[n].numbers[0]+1,
				amberParameters->improperTorsionTerms[n].numbers[1]+1,
				amberParameters->improperTorsionTerms[n].numbers[2]+1,
				amberParameters->improperTorsionTerms[n].numbers[3]+1,
				amberParameters->improperTorsionTerms[n].barrier,
				amberParameters->improperTorsionTerms[n].phase,
				amberParameters->improperTorsionTerms[n].n,
				amberParameters->improperTorsionTerms[n].c
				);
	}

	fprintf(file,"\tfprintf(fout,\"End\\n\");\n");
}
/**********************************************************************/
void printfMmCFileNonBonded(FILE* file,AmberParameters* amberParameters)
{
	gint n;
	fprintf(file,"\tfprintf(fout,\"%s\\n\");\n",nonBondedTitle);

	for(n=0;n<amberParameters->numberOfNonBonded;n++)
	{
		fprintf(file,"\tfprintf(fout,\"%d\t%8.4f\t%8.4f\t\t%s\\n\");\n",
				amberParameters->nonBondedTerms[n].number+1,
				amberParameters->nonBondedTerms[n].r,
				amberParameters->nonBondedTerms[n].epsilon,
				amberParameters->nonBondedTerms[n].c
				);
	}


	
		fprintf(file,"\tfprintf(fout,\"End\\n\");\n");
}


/**********************************************************************/
void printfMmCFileDihedralAngle(FILE* file,AmberParameters* amberParameters)
{
	gint n;
	gdouble nn;

	fprintf(file,"\tfprintf(fout,\"%s\\n\");\n",dihedralAngleTitle);
	for(n=0;n<amberParameters->numberOfDihedralTerms;n++)
	{
		gint j;
		for(j=0;j<amberParameters->dihedralAngleTerms[n].nSomme;j++)
		{
			if(j==amberParameters->dihedralAngleTerms[n].nSomme-1)
				nn = amberParameters->dihedralAngleTerms[n].n[j];
			else
				nn = -amberParameters->dihedralAngleTerms[n].n[j];
		
			fprintf(file,"\tfprintf(fout,\"%d\t%d\t%d\t%d\t%4.1f\t%6.3f\t%6.2f\t%4.1f\t\t%s\\n\");\n",
				amberParameters->dihedralAngleTerms[n].numbers[0]+1,
				amberParameters->dihedralAngleTerms[n].numbers[1]+1,
				amberParameters->dihedralAngleTerms[n].numbers[2]+1,
				amberParameters->dihedralAngleTerms[n].numbers[3]+1,
				amberParameters->dihedralAngleTerms[n].divisor[j],
				amberParameters->dihedralAngleTerms[n].barrier[j],
				amberParameters->dihedralAngleTerms[n].phase[j],
				nn,
				amberParameters->dihedralAngleTerms[n].c

				);
		}
	}
	fprintf(file,"\tfprintf(fout,\"End\\n\");\n");
}
/**********************************************************************/
void printfMmCFileAtomTypes(FILE* file,AmberParameters* amberParameters)
{
	gint i;
	fprintf(file,"\tfprintf(fout,\"%s\\n\");\n",atomTypesTitle);

	for(i=0;i<amberParameters->numberOfTypes;i++)
	{
		fprintf(file,"\tfprintf(fout,\"%s\t%s\t%d\t%6.3f\t%6.3f\t\t%s\\n\");\n",
			amberParameters->atomTypes[i].name,
			getSymbol(amberParameters->atomTypes[i].name),
			amberParameters->atomTypes[i].number+1,
			amberParameters->atomTypes[i].masse,
			amberParameters->atomTypes[i].polarisability,
			amberParameters->atomTypes[i].c
			);
	}

	fprintf(file,"\tfprintf(fout,\"End\\n\");\n");
}

/**********************************************************************/
void printfMmCFileTitle(FILE* file)
{
	fprintf(file,"\tfprintf(fout,\"Begin Title\\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\tAtom Types         : Ty(Type) Symbol Numero Masse(C12 UMA) Polarisablities(Ang**3) \\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\tBond Length        : N1-N2 Force(Kcal/mol/A**2) Re\\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\tBond Angle         : N1-N2-N3 Force(Kcal/mol/rad**2) Angle(Deg) \\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\tDihedral           : N1-N2-N3-N4 Idiv Pk Phase(Deg) Pn \\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\t                     E = Pk/Idiv*(1 + cos(P,*Phi - Phase)\\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\t                     Pk = Barrier/2 Kcal/mol\\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\t                     Idiv barrier is divised by Idiv\\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\t                     Pn = periodicity fo the torional barrier\\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\t                     if Pn<0 the tosional potential is \\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\t                     assumed to have more than one term\\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\t                     if Ni=0 => N is a number for any one Type\\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\tImproper Dihedral  : N1-N2-N3-N4 Idiv Pk Phase(Deg) Pn \\n\");\n");
	fprintf(file,"\tfprintf(fout,\"\tH-Bond             : N1-N2   A(coef. 1/r**12)  B(coef. -B/r**10)\\n\");\n");
	fprintf(file,"\tfprintf(fout,\"End\\n\");\n");
}
/**********************************************************************/
void printfMmCFile(AmberParameters* amberParameters)
{
	FILE* file;
	file = fopen("createMMFile.fc","w");
	if(!file)
	{
		printf("I can not open createMMFile.fc\n");
		return;
	}
	fprintf(file,"gboolean createMMFile(gchar* filename)\n");
	fprintf(file,"{\n");
	fprintf(file,"\tFILE* fout = fopen(filename,\"w\");\n\n");
	fprintf(file,"\tif(fout==NULL)\n");
	fprintf(file,"\t{\n");
	fprintf(file,"\t\treturn FALSE;\n");
	fprintf(file,"\t}\n");
//	fprintf(file,"\tfprintf(fout,\"Natoms = %d\\n\");\n",natoms);
	printfMmCFileTitle(file);
	printfMmCFileAtomTypes(file,amberParameters);
	printfMmCFileBond(file,amberParameters);
	printfMmCFileBend(file,amberParameters);
	printfMmCFileDihedralAngle(file,amberParameters);
	printfMmCFileimproperTorsion(file,amberParameters);
	printfMmCFileHydrogenBonded(file,amberParameters);
	printfMmCFileNonBonded(file,amberParameters);
	
	fprintf(file,"\tfclose(fout);\n");
	fprintf(file,"\treturn TRUE;\n");
	fprintf(file,"}\n");
	fclose(file);
}
int main()
{
	AmberParameters amberParameters;
	readAmberParameters(&amberParameters);
	printfMmCFile(&amberParameters);
 
 return 0;
	
}
