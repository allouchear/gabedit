/* OrbitalsGamess.c */
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
#include "GlobalOrb.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/Constants.h"
#include "../Geometry/GeomGlobal.h"
#include "GeomDraw.h"
#include "GLArea.h"
#include "UtilsOrb.h"
#include "Basis.h"
#include "GeomOrbXYZ.h"
#include "AtomicOrbitals.h"
#include "StatusOrb.h"
#include "Basis.h"
#include "Orbitals.h"
#include "GeomOrbXYZ.h"
#include "BondsOrb.h"

/********************************************************************************/
typedef enum
{
  GABEDIT_ORBLOCALTYPE_BOYS=0,
  GABEDIT_ORBLOCALTYPE_EDMISTON,
  GABEDIT_ORBLOCALTYPE_PIPEK,
  GABEDIT_ORBLOCALTYPE_UNKNOWN
} GabEditOrbLocalType;

static gchar* titlesLocalOrb[GABEDIT_ORBLOCALTYPE_PIPEK+1]=
{
	"BOYS ORBITAL LOCALIZATION",
	"EDMISTON-RUEDENBERG ENERGY LOCALIZATION",
	"MOLECULAR ORBITALS LOCALIZED BY THE POPULATION METHOD"
};

typedef enum
{
  GABEDIT_ORBTYPE_ALPHA = 0,
  GABEDIT_ORBTYPE_BETA,
  GABEDIT_ORBTYPE_MOLECULAR,
  GABEDIT_ORBTYPE_MCSCF,
  GABEDIT_ORBTYPE_EIGENVECTORS,
  GABEDIT_ORBTYPE_BOYS_ALPHA,
  GABEDIT_ORBTYPE_BOYS_BETA,
  GABEDIT_ORBTYPE_BOYS,
  GABEDIT_ORBTYPE_EDMISTON_ALPHA,
  GABEDIT_ORBTYPE_EDMISTON_BETA,
  GABEDIT_ORBTYPE_EDMISTON,
  GABEDIT_ORBTYPE_PIPEK_ALPHA,
  GABEDIT_ORBTYPE_PIPEK_BETA,
  GABEDIT_ORBTYPE_PIPEK,
} GabEditOrbType;
static gchar* titlesOrb[GABEDIT_ORBTYPE_PIPEK+1]=
{
	"----- ALPHA SET -----",
	"----- BETA SET -----",
	"MOLECULAR ORBITALS",
	"MCSCF OPTIMIZED ORBITALS",
        "EIGENVECTORS",
	"***** ALPHA ORBITAL LOCALIZATION *****",
	"****** BETA ORBITAL LOCALIZATION *****",
	"THE BOYS LOCALIZED ORBITALS ARE",
	"***** ALPHA ORBITAL LOCALIZATION *****",
	"****** BETA ORBITAL LOCALIZATION *****",
	"EDMISTON-RUEDENBERG ENERGY LOCALIZED ORBITALS",
	"***** ALPHA ORBITAL LOCALIZATION *****",
	"****** BETA ORBITAL LOCALIZATION *****",
	"THE PIPEK-MEZEY POPULATION LOCALIZED ORBITALS ARE"
};
/********************************************************************************/
static gboolean sphericalBasis = FALSE;
/********************************************************************************/
static void get_charges_from_gamess_output_file(FILE* fd,gint N)
{
 	guint taille=BSIZE;
  	gchar t[BSIZE];
  	gchar dump[BSIZE];
  	gchar d[BSIZE];
  	gchar* pdest;
	gint i;


  	while(!feof(fd) )
	{
    		pdest = NULL;
    		if(!fgets(t,taille,fd)) break;
    		pdest = strstr( t, "TOTAL MULLIKEN AND LOWDIN ATOMIC POPULATIONS");

		if(pdest)
		{
    			if(!fgets(t,taille,fd)) break;
			for(i=0;i<N;i++)
			{
    				if(!fgets(t,taille,fd)) break;
				if(sscanf(t,"%s %s %s %s %s %s",dump, dump ,dump, dump, dump, d)==6)
				{
					GeomOrb[i].partialCharge = atof(d);
				}
				else break;
			}
			break;
		}
	}
}
/********************************************************************************/
static gint* read_geomorb_gamess_file_geom(gchar *FileName)
{
 	gchar *t;
 	gchar *tmp = NULL;
 	gboolean OK;
 	gchar *AtomCoord[5];
 	FILE *fd;
 	guint taille=BSIZE;
 	guint i;
 	gint j=0;
 	guint numgeom;
	gchar dum[100];
	gint uni=1;
	gint* znuc=NULL;
	long geompos=0;

 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
    		return NULL;
 	}

 	t=g_malloc(taille);
 	fd = FOpen(FileName, "rb");
 	if(fd ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
 		g_free(t);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
  		return NULL;
 	}

  	init_dipole();
	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Gamess");
 	numgeom =1;
 	do 
 	{
		set_status_label_info(_("Geometry"),_("Reading"));
 		OK=FALSE;
 		while(!feof(fd))
		{
			if(!fgets(t,taille,fd))break;
			if ( strstr(t,"COORDINATES (BOHR)"))
			{
    				if(!feof(fd)) { char* e = fgets(t,taille,fd);}
 				numgeom++;
                		OK = TRUE;
				uni = 0;
	  			break;
	  		}
			if ( strstr(t,"COORDINATES OF ALL ATOMS ARE (ANGS)"))
			{
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
 				numgeom++;
				uni=1;
				OK = TRUE;
				break;
	  		}
        	}
 		if(!OK && (numgeom == 1) )
		{
  			Message(_("Sorry\nI can not read geometry from this file"),_("Error"),TRUE);
 			fclose(fd);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(AtomCoord[i]);
			set_status_label_info(_("File name"),_("Nothing"));
			set_status_label_info(_("File type"),_("Nothing"));
			set_status_label_info(_("Geometry"),_("Nothing"));
			return NULL;
    		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(fd) )
  		{
    			{ char* e = fgets(t,taille,fd);}
			if ( !strcmp(t,"\n"))
			{
				break;
			}
			if ( !strcmp(t,"\r\n")) break;
    			j++;
    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));
    			if(znuc==NULL) znuc=g_malloc(sizeof(gint));
    			else znuc=g_realloc(znuc,(j+1)*sizeof(gint));

			sscanf(t,"%s %s %s %s %s",AtomCoord[0],dum, AtomCoord[1], AtomCoord[2],AtomCoord[3]);

    			GeomOrb[j].Symb=g_strdup(AtomCoord[0]);
			znuc[j]=atoi(dum);
    			for(i=0;i<3;i++) 
			{
				GeomOrb[j].C[i]=atof((AtomCoord[i+1]));
				if(uni==1) GeomOrb[j].C[i] *= ANG_TO_BOHR;
			}

			/* GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);*/ /* not here*/
  		}
		geompos = ftell(fd);
			
 	}while(!feof(fd));

 	nCenters = j+1;
	if(nCenters>0)
	{
		fseek(fd, geompos, SEEK_SET);
		get_dipole_from_gamess_output_file(fd);
		fseek(fd, geompos, SEEK_SET);
		get_charges_from_gamess_output_file(fd,nCenters);
	}
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 )
	{
		g_free(GeomOrb);
		g_free(znuc);
		znuc = NULL;
	}
 	else
	{
  		DefineType();
  		/* PrintGeomOrb();*/
	}
	buildBondsOrb();
	RebuildGeomD = FALSE;
	return znuc;
}
/********************************************************************************/
static void DefineGamessCartBasis()
{
 gint i,j,k,n;
 gint l1,l2,l3;
 gint L;
 gint *l[3]={NULL,NULL,NULL};
 gint m;

 NAOrb = 0;
 for(i=0;i<nCenters;i++)
 {
	 for(j=0;j<Type[GeomOrb[i].NumType].Norb;j++)
	 {
		L=Type[GeomOrb[i].NumType].Ao[j].L;
		NAOrb += (L+1)*(L+2)/2;
	 }
 }

 AOrb = g_malloc(NAOrb*sizeof(CGTF));
 if(SAOrb) g_free(SAOrb);
 SAOrb = NULL;
 
 k=-1;
 for(i=0;i<nCenters;i++)
	 for(j=0;j<Type[GeomOrb[i].NumType].Norb;j++)
 {
	L = Type[GeomOrb[i].NumType].Ao[j].L;

	for(m=0;m<3;m++)
	{
		if(l[m])
		   g_free(l[m]);
		l[m] = g_malloc((L+1)*(L+2)/2*sizeof(gint));
	}
	switch(L)
	{
	  case 1 :
		 m=0;
		 l[0][m] = 1;l[1][m] = 0;l[2][m] = 0; /* X */
		 m++;
		 l[0][m] = 0;l[1][m] = 1;l[2][m] = 0; /* Y */
		 m++;
		 l[0][m] = 0;l[1][m] = 0;l[2][m] = 1; /* Z */
	  	 break;
	  case 2 :
		 m=0;
		 l[0][m] = 2;l[1][m] = 0;l[2][m] = 0; /* XX */
		 m++;
		 l[0][m] = 0;l[1][m] = 2;l[2][m] = 0; /* YY */
		 m++;
		 l[0][m] = 0;l[1][m] = 0;l[2][m] = 2; /* ZZ */
		 m++;
		 l[0][m] = 1;l[1][m] = 1;l[2][m] = 0; /* XY */
		 m++;
		 l[0][m] = 1;l[1][m] = 0;l[2][m] = 1; /* XZ */
		 m++;
		 l[0][m] = 0;l[1][m] = 1;l[2][m] = 1; /* YZ */
		 break;
	  case 3 :
		 m=0;
		 l[0][m] = 3;l[1][m] = 0;l[2][m] = 0; /* XXX */
		 m++;
		 l[0][m] = 0;l[1][m] = 3;l[2][m] = 0; /* YYY */
		 m++;
		 l[0][m] = 0;l[1][m] = 0;l[2][m] = 3; /* ZZZ */
		 m++;
		 l[0][m] = 2;l[1][m] = 1;l[2][m] = 0; /* XXY */
		 m++;
		 l[0][m] = 2;l[1][m] = 0;l[2][m] = 1; /* XXZ */
		 m++;
		 l[0][m] = 1;l[1][m] = 2;l[2][m] = 0; /* YYX */
		 m++;
		 l[0][m] = 0;l[1][m] = 2;l[2][m] = 1; /* YYZ */
		 m++;
		 l[0][m] = 1;l[1][m] = 0;l[2][m] = 2; /* ZZX */
		 m++;
		 l[0][m] = 0;l[1][m] = 1;l[2][m] = 2; /* ZZY */
		 m++;
		 l[0][m] = 1;l[1][m] = 1;l[2][m] = 1; /* XYZ */
		 break;
	  case 4 :
		 m=0; l[0][m] = 4;l[1][m] = 0;l[2][m] = 0; /* XXXX */
		 m++; l[0][m] = 0;l[1][m] = 4;l[2][m] = 0; /* YYYY */
		 m++; l[0][m] = 0;l[1][m] = 0;l[2][m] = 4; /* ZZZZ */
		 m++; l[0][m] = 3;l[1][m] = 1;l[2][m] = 0; /* XXXY */
		 m++; l[0][m] = 3;l[1][m] = 0;l[2][m] = 1; /* XXXZ */
		 m++; l[0][m] = 1;l[1][m] = 3;l[2][m] = 0; /* YYYX */
		 m++; l[0][m] = 0;l[1][m] = 3;l[2][m] = 1; /* YYYZ */
		 m++; l[0][m] = 1;l[1][m] = 0;l[2][m] = 3; /* ZZZX */
		 m++; l[0][m] = 0;l[1][m] = 1;l[2][m] = 3; /* ZZZY */
		 m++; l[0][m] = 2;l[1][m] = 2;l[2][m] = 0; /* XXYY */
		 m++; l[0][m] = 2;l[1][m] = 0;l[2][m] = 2; /* XXZZ */
		 m++; l[0][m] = 0;l[1][m] = 2;l[2][m] = 2; /* YYZZ */
		 m++; l[0][m] = 2;l[1][m] = 1;l[2][m] = 1; /* XXYZ */
		 m++; l[0][m] = 1;l[1][m] = 2;l[2][m] = 1; /* YYXZ */
		 m++; l[0][m] = 1;l[1][m] = 1;l[2][m] = 2; /* ZZXY */
		 break;
	  default :
		m=0;
		for(l3=Type[GeomOrb[i].NumType].Ao[j].L;l3>=0;l3--)
			for(l2=Type[GeomOrb[i].NumType].Ao[j].L-l3;l2>=0;l2--)
		{
	 		l1 = Type[GeomOrb[i].NumType].Ao[j].L-l2-l3;
			l[0][m] = l1;
			l[1][m] = l2;
			l[2][m] = l3;
			m++;
		}
	}
		for(m=0;m<(L+1)*(L+2)/2;m++)
 		{
			l1 = l[0][m];
			l2 = l[1][m];
	 		l3 = l[2][m];
	 		k++;
	 		AOrb[k].numberOfFunctions=Type[GeomOrb[i].NumType].Ao[j].N;
			AOrb[k].NumCenter = i;
	 		AOrb[k].Gtf =g_malloc(AOrb[k].numberOfFunctions*sizeof(GTF));
	 		for(n=0;n<AOrb[k].numberOfFunctions;n++)
	 		{
	   			AOrb[k].Gtf[n].Ex   = Type[GeomOrb[i].NumType].Ao[j].Ex[n];
	   			AOrb[k].Gtf[n].Coef = Type[GeomOrb[i].NumType].Ao[j].Coef[n];
	   			AOrb[k].Gtf[n].C[0] = GeomOrb[i].C[0];
	   			AOrb[k].Gtf[n].C[1] = GeomOrb[i].C[1];
	   			AOrb[k].Gtf[n].C[2] = GeomOrb[i].C[2];
	   			AOrb[k].Gtf[n].l[0] = l1;
	   			AOrb[k].Gtf[n].l[1] = l2;
	   			AOrb[k].Gtf[n].l[2] = l3;
	 		}
	 
 		}
}

NOrb = NAOrb;
DefineAtomicNumOrb();
/* DefineNorb();*/
}
/********************************************************************************/
static gchar** read_basis_from_a_gamess_output_file(gchar *FileName, gint* nrs)
{
 	gchar **strbasis;
 	gchar *t;
 	FILE *fd;
 	gint taille=BSIZE;
 	gint nrows=0;
	gboolean OK = FALSE;
	
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return NULL;
 	}

 	fd = FOpen(FileName, "rb");
 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
  		return NULL;
 	}

 	t=g_malloc(taille*sizeof(gchar));
 	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
		if(strstr(t,"ATOMIC BASIS SET"))
		{
			OK = TRUE;
			break;
		}
        }
	if(OK)
	{
		OK = FALSE;
 		while(!feof(fd))
		{
			if(!fgets(t,taille,fd))break;
			if(strstr(t,"SHELL TYPE"))
			if(strstr(t,"EXPONENT"))
			{
				OK = TRUE;
				break;
			}
		}
	}
	if(!OK)
	{
		g_free(t);
  		Message(_("Sorry I can read basis from this file\n"),_("Error"),TRUE);
  		return NULL;
	}
    	{ char* e = fgets(t,taille,fd);}

	strbasis=g_malloc(sizeof(gchar*));
 	while(!feof(fd))
	{
    		{ char* e = fgets(t,taille,fd);}
		if(strstr(t,"TOTAL NUMBER OF")) break;
		nrows++;
		strbasis = g_realloc(strbasis,nrows*sizeof(gchar*));
		strbasis[nrows-1] = g_strdup(t);
	}
	if(nrows == 0)
	{
		g_free(t);
		g_free(strbasis);
  		Message(_("Sorry I can read basis in this file\n"),_("Error"),TRUE);
  		return NULL;
	}
	
	/*
	Debug("End of read \n");
	Debug("Atomic basis nrows = %d \n",nrows);
	for(i=0;i<nrows;i++)
	{
		Debug("%s",strbasis[i]);
	}
	*/

 	fclose(fd);
 	g_free(t);
	*nrs = nrows;
	return strbasis;
}
/**********************************************/
static gint get_num_type_from_symbol(gchar* symbol)
{
	gint k;
	for(k=0;k<nCenters;k++)
	{
		if(strcmp(symbol,GeomOrb[k].Symb)==0)
			return (gint)GeomOrb[k].NumType;
	}
	return -1;
}
/**********************************************/
static gboolean addOneBasis(gint i,gint j,gchar *shell,gint ncont, gdouble* ex, gdouble* coef)
{
	gint jj;
       	Type[i].Ao[j].N = ncont;
	Type[i].Ao[j].Ex=g_malloc(Type[i].Ao[j].N*sizeof(gdouble));
	Type[i].Ao[j].Coef=g_malloc(Type[i].Ao[j].N*sizeof(gdouble));
	for(jj=0;jj<Type[i].Ao[j].N;jj++)
	{
		Type[i].Ao[j].Ex[jj] = ex[jj];
		Type[i].Ao[j].Coef[jj] = coef[jj];
	}
    	switch(shell[0])
	{
		/* L =SP with Gamess */
        	case 'l' : 
        	case 'L' : 
        	case 's' : 
        	case 'S' : Type[i].Ao[j].L=0;break;
        	case 'p' :
        	case 'P' : Type[i].Ao[j].L=1;break; 
        	case 'd' : 
        	case 'D' : Type[i].Ao[j].L=2;break;
        	case 'f' : 
        	case 'F' : Type[i].Ao[j].L=3;break;         
		case 'g' : 
        	case 'G' : Type[i].Ao[j].L=4;break;
		case 'h' : 
        	case 'H' : Type[i].Ao[j].L=5;break;
		case 'i' : 
        	case 'I' : Type[i].Ao[j].L=6;break;
		case 'j' : 
        	case 'J' : Type[i].Ao[j].L=7;break;
		case 'k' : 
        	case 'K' : Type[i].Ao[j].L=8;break;

        	default : return FALSE;
     	}
	return TRUE;
}
/**********************************************/
static gboolean DefineGamessBasisType(gchar** strbasis, gint nrows)
{
	gchar sym[50];
	gchar shell[10];
	gchar t[10];
	gint i;
	gint j;
	gint nconts;
	gint k;
	gdouble *ex=NULL;
	gdouble *coef1=NULL;
	gdouble *coef2=NULL;
	gchar* temp[10];
	gint ne;
	gboolean Ok;
	gint jj;

	if(Ntype<1) return FALSE;
	if(nrows<1) return FALSE;
	ex = g_malloc(nrows*sizeof(gdouble));
	coef1 = g_malloc(nrows*sizeof(gdouble));
	coef2 = g_malloc(nrows*sizeof(gdouble));
	for(i=0;i<10;i++) temp[i] = g_malloc(BSIZE*sizeof(gchar));

	/*
	for(k=0;k<nCenters;k++)
	{
		printf("%s %d\n",GeomOrb[k].Symb,GeomOrb[k].NumType);
	}
	*/

	Type = g_malloc(Ntype*sizeof(TYPE));
	for(i=0;i<Ntype;i++)
	{
		Type[i].Ao = NULL;
        	Type[i].Norb=0;
	}
	for(k=0;k<nCenters;k++)
	{
		sprintf(sym,"%s",GeomOrb[k].Symb);
		i = GeomOrb[k].NumType;
		/* printf("numType = %d\n",k);*/
     		Type[i].Symb=g_strdup(sym);
     		Type[i].N=GetNelectrons(sym);
	}
	/* set number of basis by type */
	i = -1;
	sprintf(shell,"S");
	nconts=-1;
	Ok = TRUE;
	for(k=0;k<nrows;k++)
	{
		if(this_is_a_backspace(strbasis[k])&&i==-1)continue;
		if(this_is_a_backspace(strbasis[k]))
		{
			gint j = Type[i].Norb;
			if(strcmp(shell,"L")==0) Type[i].Norb+=2;
			else Type[i].Norb++;
     			if(Type[i].Ao == NULL) 
				Type[i].Ao=g_malloc(Type[i].Norb*sizeof(AO));
     			else 
				Type[i].Ao=g_realloc(Type[i].Ao,Type[i].Norb*sizeof(AO));
			for(jj=j;jj< Type[i].Norb;jj++)
			{
				Type[i].Ao[jj].Ex = NULL;
				Type[i].Ao[jj].Coef = NULL;
			}


			if(!addOneBasis(i,j,shell,nconts, ex, coef1)) 
			{
				Ok = FALSE;
				break;
			}
			if(strcmp(shell,"L")==0)
			{
				if(!addOneBasis(i,j+1,"P",nconts, ex, coef2))
				{
					Ok = FALSE;
					break;
				}
			}
				
			/*
			printf("shell =%s ",shell);
			printf("nconts =%d\n",nconts);
			*/
			nconts=0;
			continue;
		}
		sscanf(strbasis[k],"%s",t);
		if(!isdigit(t[0])) /* symbol of atom*/
		{
			/*
			for(i=1;i<strlen(t);i++)
				if(isdigit(t[i])) t[i] = ' ';
			delete_last_spaces(t);
			*/

			i=get_num_type_from_symbol(t);
			if(i<0)
			{
				Ok = FALSE;
				break;
			}
			/* printf("symb =%s i = %d\n",t,i);*/
			Type[i].Norb=0;
			nconts=0;
			k++;
			continue;
		}
		sscanf(strbasis[k],"%s %s",t,shell);
		if(strstr(strbasis[k],"(") && strstr(strbasis[k],")"))
		{
			gchar dum1[BSIZE];
			gchar dum2[BSIZE];
			for(j=0;j<strlen(strbasis[k]);j++)
			{
				if(strbasis[k][j]==')') strbasis[k][j]=' ';
				if(strbasis[k][j]=='(') strbasis[k][j]=' ';
			}
			ne = sscanf(strbasis[k],"%s %s %s %s %s %s %s %s",t,shell,temp[3],temp[0],dum1, temp[1],dum2, temp[2]);
			if(ne==8) ne = 6;
			else if(ne==6) ne = 5;
		}
		else
		ne = sscanf(strbasis[k],"%s %s %s %s %s %s",t,shell,temp[3],temp[0],temp[1],temp[2]);
		if(ne<5)
		{
			Ok = FALSE;
			break;
		}
		for(j=0;j<ne-3;j++)
		{
			gchar* d=strstr(temp[j],"D");
			if(d) *d='e';
		}
		if(ne>=4) ex[nconts]=atof(temp[0]);
		if(ne>=5) coef1[nconts]=atof(temp[1]);
		if(ne==6) coef2[nconts]=atof(temp[2]);
		nconts++;

	}
	if(!Ok)
	{
		if(Type)
		for(i=0;i<Ntype;i++)
		{
			if(Type[i].Ao != NULL)
			{
				for(j=0;j<Type[i].Norb;j++)
				{
					if(Type[i].Ao[j].Ex != NULL) g_free(Type[i].Ao[j].Ex);
					if(Type[i].Ao[j].Coef != NULL) g_free(Type[i].Ao[j].Coef);
				}
				g_free(Type[i].Ao);
			}
		}

		if(Type) g_free(Type);
		if(ex) g_free(ex);
		if(coef1) g_free(coef1);
		if(coef2) g_free(coef2);
		for(j=0;j<10;j++) g_free(temp[j]);
		return FALSE;
	}
	if(ex) g_free(ex);
	if(coef1) g_free(coef1);
	if(coef2) g_free(coef2);
	for(j=0;j<10;j++) g_free(temp[j]);

    	return TRUE;
}
/********************************************************************************/
static gint get_number_of_ecp_core_electrons(gchar* FileName)
{
 	gchar *t;
 	FILE *file;
	gint ne = 0;

 	if ((!FileName) || (strcmp(FileName,"") == 0)) return ne;

 	t=g_malloc(BSIZE*sizeof(gchar));
 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return ne;

	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if(strstr(t,"ECP") && strstr(t,"REMOVES") &&strstr(t,"CORE") &&strstr(t,"ELECTRONS"))
		{
			gchar* equal=strstr(t,"REMOVES");
			if(equal) ne=atoi(equal+strlen("REMOVES"));
		}
		if(strstr(t,"NUMBER OF OCCUPIED ORBITALS") && strstr(t,"KEPT"))/* recent version of GAMESS-US */
		{
			ne = 0;
			break;
		}
	}
	if(t!=NULL) g_free(t);
	if(file) fclose(file);
	return ne;
}
/********************************************************************************/
static void get_number_of_occuped_orbitals(gchar* FileName, gint* nAlpha, gint* nBeta)
{
 	gchar *t;
 	FILE *file;
	gchar *tag=NULL;
	gint necore = 0;

	*nAlpha=-1;
	*nBeta=-1;
 	if ((!FileName) || (strcmp(FileName,"") == 0)) return;

 	t=g_malloc(BSIZE*sizeof(gchar));
 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return;

	tag=g_strdup_printf("NUMBER OF OCCUPIED ORBITALS ");
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if(strstr(t,tag))
		{
			gchar* equal=strstr(t,"=");
			if(equal) *nAlpha=atoi(equal+1);
			if(!fgets(t,BSIZE,file))break;
			equal=strstr(t,"=");
			if(equal) *nBeta=atoi(equal+1);
		}

	}
	if(tag!=NULL) g_free(tag);
	if(t!=NULL) g_free(t);
	if(file) fclose(file);
	necore = get_number_of_ecp_core_electrons(FileName);
	*nAlpha -= necore/2;
	*nBeta -= necore/2;
	if(*nAlpha<0) *nAlpha= 0;
	if(*nBeta<0) *nBeta= 0;
}
/********************************************************************************/
static GabEditOrbLocalType get_local_orbital_type(gchar *fileName)
{
 	gchar *t;
 	FILE *file;
 	gint taille=BSIZE;
	
 	if ((!fileName) || (strcmp(fileName,"") == 0)) return GABEDIT_ORBLOCALTYPE_UNKNOWN;

 	t=g_malloc(taille);
 	file = FOpen(fileName, "rb");
 	if(file ==NULL) return GABEDIT_ORBLOCALTYPE_UNKNOWN;
 	while(!feof(file))
	{
			GabEditOrbLocalType i;
	  		if(!fgets(t,taille,file))break;
			for(i=GABEDIT_ORBLOCALTYPE_BOYS;i<=GABEDIT_ORBLOCALTYPE_PIPEK;i++)
			{
          			if(strstr( t,titlesLocalOrb[i]))
				{
					g_free(t);
					return i;
				}
			}
	}
	g_free(t);
 	return GABEDIT_ORBLOCALTYPE_UNKNOWN;
}
/********************************************************************************/
static gboolean read_last_orbitals_in_gamess_file(gchar *fileName,GabEditOrbType itype, gint nAlpha, gint nBeta)
{
 	gchar *t;
 	gboolean OK;
 	gchar *dum[5];
 	FILE *fd;
 	gint taille=BSIZE;
 	gint i;
 	gint numorb;
 	gchar *pdest = NULL;
	gint NumOrb[5];
	gchar SymOrb[5][10];
	gdouble EnerOrb[5];
	gint ncart;
	gint n;
	gint k,k1,k2,k3;
	gint j;
	gdouble **CoefOrbitals;
	gdouble *EnerOrbitals;
	gchar **SymOrbitals;
	gchar* tmp = NULL;
	gint ibegin = 0;
	
 	if ((!fileName) || (strcmp(fileName,"") == 0))
 	{
		Message(_("Sorry No file selected\n"),_("Error"),TRUE);
    		return FALSE;
 	}

 	t=g_malloc(taille);
 	fd = FOpen(fileName, "rb");
 	if(fd ==NULL)
 	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not open '%s' file\n"),fileName);
  		Message(buffer,_("Error"),TRUE);
  		return FALSE;
 	}
 	for(i=0;i<5;i++) dum[i]=g_malloc(BSIZE*sizeof(gchar));
  
	/* Debug("Norb = %d\n",NOrb);*/
	CoefOrbitals = CreateTable2(NOrb);
	EnerOrbitals = g_malloc(NOrb*sizeof(gdouble));
	SymOrbitals = g_malloc(NOrb*sizeof(gchar*));

 	numorb =1;
 	do 
 	{
 		OK=FALSE;
 		while(!feof(fd))
		{
	  		if(!fgets(t,taille,fd))break;
			switch(itype)
			{
				case GABEDIT_ORBTYPE_ALPHA :
          				pdest = strstr( t, titlesOrb[itype]);
					break;
				case GABEDIT_ORBTYPE_BETA :
          				pdest = strstr( t, titlesOrb[itype]);
					break;
				case GABEDIT_ORBTYPE_MOLECULAR: 
          				pdest = strstr( t, titlesOrb[itype] ); 
					if(pdest)
					{
						gchar b1[100];
						if(sscanf(t,"%s %s %s",b1,b1,b1)!=2) pdest = NULL;
					}
					break;
				case GABEDIT_ORBTYPE_MCSCF: 
					pdest = strstr( t,titlesOrb[itype]); 
					break;
				case GABEDIT_ORBTYPE_EIGENVECTORS: 
          				pdest = strstr( t, titlesOrb[itype] ); 
					if(pdest)
					{
						gchar dump1[BSIZE];
						gchar dump2[BSIZE];
						gint k = sscanf(t,"%s %s",dump1,dump2);
						if(k!=1 || strcmp(dump1,titlesOrb[itype])!=0) pdest=NULL;
					}
					break;
				case GABEDIT_ORBTYPE_BOYS_ALPHA: 
          				pdest = strstr( t,titlesOrb[itype]); 
					break;
				case GABEDIT_ORBTYPE_BOYS_BETA: 
          				pdest = strstr( t,titlesOrb[itype]); 
					break;
				case GABEDIT_ORBTYPE_BOYS: 
          				pdest = strstr( t,titlesOrb[itype]); 
					break;
				case GABEDIT_ORBTYPE_EDMISTON_ALPHA: 
          				pdest = strstr( t,titlesOrb[itype]); 
					break;
				case GABEDIT_ORBTYPE_EDMISTON_BETA: 
          				pdest = strstr( t,titlesOrb[itype]); 
					break;
				case GABEDIT_ORBTYPE_EDMISTON: 
          				pdest = strstr( t,titlesOrb[itype]); 
					break;
				case GABEDIT_ORBTYPE_PIPEK_ALPHA: 
          				pdest = strstr( t,titlesOrb[itype]); 
					break;
				case GABEDIT_ORBTYPE_PIPEK_BETA: 
          				pdest = strstr( t,titlesOrb[itype]); 
					break;
				case GABEDIT_ORBTYPE_PIPEK: 
          				pdest = strstr( t,titlesOrb[itype]); 
					break;
			}
	 		if ( pdest != NULL )
	  		{
                		numorb++;
                		OK = TRUE;
	  			break;
	  		}
        	}
 		if(!OK && (numorb == 1) )
		{
  			if(
  				itype==GABEDIT_ORBTYPE_BETA || 
  				itype==GABEDIT_ORBTYPE_EIGENVECTORS ||
  				itype==GABEDIT_ORBTYPE_BOYS_BETA ||
  				itype==GABEDIT_ORBTYPE_EDMISTON_BETA ||
  				itype==GABEDIT_ORBTYPE_PIPEK_BETA
  			)
			{
				gchar buffer[BSIZE];
				sprintf(buffer,_("Sorry,  I can not read orbitals from '%s' file\n"),fileName);
  				Message(buffer,_("Error"),TRUE);
			}
			FreeTable2(CoefOrbitals,NOrb);
			g_free(EnerOrbitals);
			g_free(SymOrbitals);
 			fclose(fd);
 			g_free(t);
			return FALSE;
    		}
 		if(!OK)
		{
			/* Debug("End of read \n");*/
 			fclose(fd);
 			g_free(t);
 			for(i=0;i<5;i++) g_free(dum[i]);

			switch(itype)
			{
				case GABEDIT_ORBTYPE_ALPHA : 
				case GABEDIT_ORBTYPE_BOYS_ALPHA: 
				case GABEDIT_ORBTYPE_EDMISTON_ALPHA: 
				case GABEDIT_ORBTYPE_PIPEK_ALPHA: 
				CoefAlphaOrbitals = CoefOrbitals;
				EnerAlphaOrbitals = EnerOrbitals;
				
				SymAlphaOrbitals = SymOrbitals;

				OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
				for(i=0;i<nAlpha;i++) OccAlphaOrbitals[i] = 1.0;
				for(i=nAlpha;i<NOrb;i++) OccAlphaOrbitals[i] = 0.0;

				NAlphaOcc = nAlpha;
				NAlphaOrb = NOrb;
				break;

				case GABEDIT_ORBTYPE_BETA : 
				case GABEDIT_ORBTYPE_BOYS_BETA: 
				case GABEDIT_ORBTYPE_EDMISTON_BETA: 
				case GABEDIT_ORBTYPE_PIPEK_BETA: 
				CoefBetaOrbitals = CoefOrbitals;
				EnerBetaOrbitals = EnerOrbitals;
				SymBetaOrbitals = SymOrbitals;

				OccBetaOrbitals = g_malloc(NOrb*sizeof(gdouble));
				for(i=0;i<nBeta;i++) OccBetaOrbitals[i] = 1.0;
				for(i=nBeta;i<NOrb;i++) OccBetaOrbitals[i] = 0.0;

				NBetaOcc = nBeta;
				NBetaOrb = NOrb;
				break;

				case GABEDIT_ORBTYPE_MOLECULAR: 
				case GABEDIT_ORBTYPE_MCSCF: 
				case GABEDIT_ORBTYPE_EIGENVECTORS: 
				case GABEDIT_ORBTYPE_BOYS: 
				case GABEDIT_ORBTYPE_EDMISTON: 
				case GABEDIT_ORBTYPE_PIPEK: 
				CoefAlphaOrbitals = CoefOrbitals;
				EnerAlphaOrbitals = EnerOrbitals;
				SymAlphaOrbitals = SymOrbitals;
				OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
				for(i=0;i<nAlpha;i++) OccAlphaOrbitals[i] = 1.0;
				for(i=nAlpha;i<NOrb;i++) OccAlphaOrbitals[i] = 0.0;

				CoefBetaOrbitals = CoefOrbitals;
				EnerBetaOrbitals = EnerOrbitals;
				OccBetaOrbitals = OccAlphaOrbitals;
				SymBetaOrbitals = SymOrbitals;
				NAlphaOcc = nAlpha;
				NBetaOcc = nBeta;
				NAlphaOrb = NOrb;
				NBetaOrb = NOrb;
				break;
			}
			return TRUE;
    		}
		switch(itype)
		{
			case GABEDIT_ORBTYPE_BOYS_ALPHA: 
			case GABEDIT_ORBTYPE_BOYS_BETA: 
 				while(!feof(fd))
				{
	  				if(!fgets(t,taille,fd))break;
					if(strstr(t,titlesOrb[GABEDIT_ORBTYPE_BOYS]))
					{
    						{ char* e = fgets(t,taille,fd);}
						break;
					}
				}
				break;
			case GABEDIT_ORBTYPE_EDMISTON_ALPHA: 
			case GABEDIT_ORBTYPE_EDMISTON_BETA: 
 				while(!feof(fd))
				{
	  				if(!fgets(t,taille,fd))break;
					if(strstr(t,titlesOrb[GABEDIT_ORBTYPE_EDMISTON]))
					{
    						{ char* e = fgets(t,taille,fd);}
						break;
					}
				}
				break;
			case GABEDIT_ORBTYPE_PIPEK_ALPHA: 
			case GABEDIT_ORBTYPE_PIPEK_BETA: 
 				while(!feof(fd))
				{
	  				if(!fgets(t,taille,fd))break;
					if(strstr(t,titlesOrb[GABEDIT_ORBTYPE_PIPEK]))
					{
    						{ char* e = fgets(t,taille,fd);}
						break;
					}
				}
				break;
			case GABEDIT_ORBTYPE_ALPHA :
			case GABEDIT_ORBTYPE_BETA :
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
    				{ char* e = fgets(t,taille,fd);}
				break;
			case GABEDIT_ORBTYPE_MOLECULAR: 
			case GABEDIT_ORBTYPE_MCSCF: 
			case GABEDIT_ORBTYPE_EIGENVECTORS: 
				/* error message of version */
 				while(!feof(fd))
				{
	  				if(!fgets(t,taille,fd))break;
					{
						gint d;
						gint k = sscanf(t,"%d",&d);
						if(k==1 && d>0) {ibegin=1;break;}
					}
				}
				break;
			case GABEDIT_ORBTYPE_BOYS: 
			case GABEDIT_ORBTYPE_EDMISTON: 
			case GABEDIT_ORBTYPE_PIPEK: 
    				{ char* e = fgets(t,taille,fd);}
		}

  		ncart=NOrb/5;
		if(NOrb%5>0) ncart++;
		gint no=0;
		for(n=0;n<ncart;n++)
		{
			if(ibegin ==0) {if(!fgets(t,taille,fd))break;}
			else ibegin = 0;
			k1 = sscanf(t,"%d %d %d %d %d",&NumOrb[0],&NumOrb[1],&NumOrb[2],&NumOrb[3],&NumOrb[4]);
			for(i=0;i<k1;i++) NumOrb[i]--;
			for(i=0;i<k1;i++) 
			{
				if(NumOrb[i]<0 || NumOrb[i]>NOrb-1) break;
			}
			if(k1<1)
			{
				break;
			}

    			{ char* e = fgets(t,taille,fd);}
			k2 = sscanf(t,"%lf %lf %lf %lf %lf", &EnerOrb[0], &EnerOrb[1], &EnerOrb[2], &EnerOrb[3], &EnerOrb[4]);
			for(i=0;i<k2;i++) EnerOrbitals[NumOrb[i]] = EnerOrb[i];
			if(k2>0)
			{
    				{ char* e = fgets(t,taille,fd);}
				/* Debug("%d %d %d %d %d\n",NumOrb[0],NumOrb[1],NumOrb[2],NumOrb[3],NumOrb[4]);*/
				k3 = sscanf(t,"%s %s %s %s %s",SymOrb[0],SymOrb[1],SymOrb[2],SymOrb[3],SymOrb[4]);
				/* Debug("%s %s %s %s %s\n",SymOrb[0],SymOrb[1],SymOrb[2],SymOrb[3],SymOrb[4]);*/

				for(i=0;i<k3;i++) SymOrbitals[NumOrb[i]] = g_strdup(SymOrb[i]);
			}
			else
			{
				for(i=0;i<k1;i++) EnerOrbitals[NumOrb[i]] = 0.0;
				if(
					   itype==GABEDIT_ORBTYPE_BOYS_ALPHA 
					|| itype==GABEDIT_ORBTYPE_BOYS_BETA 
					|| itype==GABEDIT_ORBTYPE_BOYS 
				)
					for(i=0;i<k1;i++) SymOrbitals[NumOrb[i]] = g_strdup("BOYS");
				else 
				if(
					   itype==GABEDIT_ORBTYPE_EDMISTON_ALPHA 
					|| itype==GABEDIT_ORBTYPE_EDMISTON_BETA 
					|| itype==GABEDIT_ORBTYPE_EDMISTON 
				)
					for(i=0;i<k1;i++) SymOrbitals[NumOrb[i]] = g_strdup("EDMISTON-RUEDENBERG");

				else
				if(
					   itype==GABEDIT_ORBTYPE_PIPEK_ALPHA 
					|| itype==GABEDIT_ORBTYPE_PIPEK_BETA 
					|| itype==GABEDIT_ORBTYPE_PIPEK 
				)
					for(i=0;i<k1;i++) SymOrbitals[NumOrb[i]] = g_strdup("PIPEK-MEZEY");
				else for(i=0;i<k1;i++) SymOrbitals[NumOrb[i]] = g_strdup("UNK");
				k3 = k1;
			}
			for(i=0;i<NOrb;i++)
			{
    				{ char* e = fgets(t,taille,fd);}
				tmp = t + 17;
				k = sscanf(tmp,"%s %s %s %s %s",dum[0], dum[1], dum[2], dum[3], dum[4]);
				for(j=0;j<k;j++) CoefOrbitals[NumOrb[j]][i]=atof(dum[j]);
			}
    			{ char* e = fgets(t,taille,fd);}
			no+=k3;
			if(k3<5)break;
		}
		if(no<NOrb)
		{
			for(j=no;j<NOrb;j++)
			{
				EnerOrbitals[j]=0;
				SymOrbitals[j] = g_strdup("DELETED");
				for(i=0;i<NOrb;i++)
				{
					CoefOrbitals[j][i]=0.0;
				}
			}
		}
		/* Debug("End ncart\n"); */
		
 	}while(!feof(fd));

	/* Debug("End of read \n"); */
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(dum[i]);

	CoefAlphaOrbitals = CoefOrbitals;
	EnerAlphaOrbitals = EnerOrbitals;
	return TRUE;
}
/********************************************************************************/
void read_gamess_orbitals(gchar* FileName)
{
	gint typefile;
	/* gint typebasis=1;*/ /* Gamess print OM in cartezian presentation even ISPHER=0 or 1 or -1 */
	gchar *t = NULL;
	gint nrs;
	gchar** strbasis=NULL;
	gint i;
	gint* znuc;
	gboolean Ok;
	gint nAlpha;
	gint nBeta;
	GabEditOrbLocalType typeLocal;


	typefile =get_type_file_orb(FileName);
	if(typefile==GABEDIT_TYPEFILE_UNKNOWN) return;


	if(typefile != GABEDIT_TYPEFILE_GAMESS)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read this format from '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return ;
	}

	free_data_all();
	t = get_name_file(FileName);
	set_status_label_info(_("File name"),t);
	g_free(t);
	set_status_label_info(_("File type"),"Gamess");
	set_status_label_info(_("Mol. Orb."),_("Reading"));
	
	free_orbitals();	

	/* typebasis =get_type_basis_in_gamess_file(FileName);
	if(typebasis == -1)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,
				"Sorry, Gabedit does not support spherical basis with contaminant cartezian function\n\n"
				"Use ISPHER=-1 or ISPHER=1 in CONTROL block"
		       );
  		Message(buffer,_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}
	*/

 	znuc = read_geomorb_gamess_file_geom(FileName);
 	if(znuc==NULL)
	{
		free_geometry();
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}
	strbasis=read_basis_from_a_gamess_output_file(FileName, &nrs);
	if(strbasis==NULL)
	{
		printf("strbasis=NULL\n");
		if(GeomOrb)
		{
			init_atomic_orbitals();
			for(i=0;i<nCenters;i++) GeomOrb[i].Prop = prop_atom_get("H");
			free_geometry();
		}
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}

	set_status_label_info(_("Mol. Orb."),_("Reading"));
 	InitializeAll();
 	if(!DefineGamessBasisType(strbasis,nrs))
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read basis from '%s' file\n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}
	for(i=0;i<Ntype;i++)
	if(Type[i].Ao == NULL)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,_("Sorry, I can not read '%s' file, problem with basis set \n"),FileName);
  		Message(buffer,_("Error"),TRUE);
		return;
	}
	/* reset symbol of atoms  */
	if(GeomOrb)
	for(i=0;i<nCenters;i++)
	{
		if(GeomOrb[i].Symb) g_free(GeomOrb[i].Symb);
		GeomOrb[i].Symb=get_symbol_using_z(znuc[i]);
		GeomOrb[i].Prop = prop_atom_get(GeomOrb[i].Symb);
		GeomOrb[i].nuclearCharge = get_atomic_number_from_symbol(GeomOrb[i].Symb);
		GeomOrb[i].variable = TRUE;
		/* printf("%s %i %f\n",GeomOrb[i].Symb,i,GeomOrb[i].nuclearCharge);*/
	}
  	/*DefineType();*/
	buildBondsOrb();
	RebuildGeomD = TRUE;
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	glarea_rafresh(GLArea); /* for geometry*/


	/*
	if(typebasis == 0)
	{
 		DefineGamessSphericalBasis();
		sphericalBasis = TRUE;
	}
	else
	*/
	{
 		DefineGamessCartBasis();
		sphericalBasis = FALSE;
	}
 	
 	/* PrintAllBasis();*/
 	NormaliseAllBasis();
 	/* PrintAllBasis();*/
 	DefineNOccs();

	get_number_of_occuped_orbitals(FileName, &nAlpha, &nBeta);
	printf("Number of ALPHA occ = %d\n",nAlpha);
	printf("Number of BETA  occ = %d\n",nBeta);
	printf("NOrb = %d\n",NOrb);

	 typeLocal = get_local_orbital_type(FileName);
	if(typeLocal!=GABEDIT_ORBLOCALTYPE_UNKNOWN)
	{
		if(typeLocal==GABEDIT_ORBLOCALTYPE_BOYS)
		{
			Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_BOYS_ALPHA,nAlpha,nBeta);
			if(Ok) Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_BOYS_BETA, nAlpha, nBeta);
			else Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_BOYS, nAlpha, nBeta);

		}
		else 
		if(typeLocal==GABEDIT_ORBLOCALTYPE_EDMISTON)
		{
			Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_EDMISTON_ALPHA,nAlpha,nBeta);
			if(Ok) Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_EDMISTON_BETA, nAlpha, nBeta);
			else Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_EDMISTON, nAlpha, nBeta);
		}
		else
		{
			Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_PIPEK_ALPHA,nAlpha,nBeta);
			if(Ok) Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_PIPEK_BETA, nAlpha, nBeta);
			else Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_PIPEK, nAlpha, nBeta);

		}

	}
	else
	{
		Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_ALPHA,nAlpha,nBeta);
		if(Ok) Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_BETA, nAlpha, nBeta);
		else
		{
			if(!Ok) Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_MOLECULAR, nAlpha, nBeta);
			if(!Ok) Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_MCSCF, nAlpha, nBeta);
			if(!Ok) Ok = read_last_orbitals_in_gamess_file(FileName,GABEDIT_ORBTYPE_EIGENVECTORS, nAlpha, nBeta);
		}
	}

	if(Ok)
	{
		/*PrintAllOrb(CoefAlphaOrbitals);*/
		set_status_label_info(_("Mol. Orb."),_("Ok"));
		glarea_rafresh(GLArea); /* for geometry*/
		NumSelOrb = NAlphaOcc-1;
		create_list_orbitals();
	}
	else
	{
		free_orbitals();	
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
	}

} 
