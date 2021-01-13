/* OrbitalsQChem.c */
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
#include "../Utils/Zlm.h"
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
  GABEDIT_ORBTYPE_RESTRICTED,
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
	"ALPHA MOLECULAR ORBITAL COEFFICIENTS",
	"BETA  MOLECULAR ORBITAL COEFFICIENTS",
	"RESTRICTED (RHF) MOLECULAR ORBITAL COEFFICIENTS",
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
static gboolean read_geomorb_qchem_file_geom(gchar *FileName)
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
	long geompos=0;
	gint idummy;
	gint l;

 	for(i=0;i<5;i++) AtomCoord[i]=g_malloc(taille*sizeof(char));
  
 	if ((!FileName) || (strcmp(FileName,"") == 0))
 	{
		Message(_("Sorry\n No file selected"),_("Error"),TRUE);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
    		return FALSE;
 	}

 	t=g_malloc(taille);
 	fd = FOpen(FileName, "rb");
 	if(fd ==NULL)
 	{
  		Message(_("Sorry\nI can not open this file"),_("Error"),TRUE);
 		g_free(t);
 		for(i=0;i<5;i++) g_free(AtomCoord[i]);
  		return FALSE;
 	}

  	init_dipole();
	free_data_all();
	tmp = get_name_file(FileName);
	set_status_label_info(_("File name"),tmp);
	g_free(tmp);
	set_status_label_info(_("File type"),"Q-Chem");
 	numgeom =1;
 	do 
 	{
		set_status_label_info(_("Geometry"),"Reading");
 		OK=FALSE;
 		while(!feof(fd))
		{
			if(!fgets(t,taille,fd))break;
			if ( strstr( t,"Atom") && strstr( t,"X") && strstr( t,"Y") && strstr( t,"Z"))
			{
	  			if(!fgets(t,taille,fd))break;
				if(!strstr( t,"----------------------------------")) break;
 				numgeom++;
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
			return FALSE;
    		}
 		if(!OK)break;

  		j=-1;
  		while(!feof(fd) )
  		{
    			{ char* e = fgets(t,taille,fd);}
    			if (strstr( t, "----------------------------------" ))
    			{
				geompos = ftell(fd);
      				break;
    			}

    			j++;
    			if(GeomOrb==NULL) GeomOrb=g_malloc(sizeof(TypeGeomOrb));
    			else GeomOrb=g_realloc(GeomOrb,(j+1)*sizeof(TypeGeomOrb));

    			sscanf(t,"%d %s %s %s %s",&idummy,AtomCoord[0],AtomCoord[1],AtomCoord[2],AtomCoord[3]);

			AtomCoord[0][0]=toupper(AtomCoord[0][0]);
	 		l=strlen(AtomCoord[0]);
          		if (l==2) AtomCoord[0][1]=tolower(AtomCoord[0][1]);
			if(isdigit(AtomCoord[0][1]))l=1;
			if(l==1)sprintf(t,"%c",AtomCoord[0][0]);
		         else sprintf(t,"%c%c",AtomCoord[0][0],AtomCoord[0][1]);

    			GeomOrb[j].Symb=g_strdup(t);
    			for(i=0;i<3;i++) GeomOrb[j].C[i]=atof(ang_to_bohr(AtomCoord[i+1]));

			GeomOrb[j].Prop = prop_atom_get(GeomOrb[j].Symb);
  		}
		geompos = ftell(fd);
			
 	}while(!feof(fd));

 	nCenters = j+1;
	if(nCenters>0)
	{
		fseek(fd, geompos, SEEK_SET);
		get_dipole_from_qchem_output_file(fd);
	}
 	fclose(fd);
 	g_free(t);
 	for(i=0;i<5;i++) g_free(AtomCoord[i]);
 	if(nCenters == 0 )
	{
		g_free(GeomOrb);
	}
 	else
	{
  		DefineType();
  		/* PrintGeomOrb();*/
	}
	buildBondsOrb();
	RebuildGeomD = FALSE;
	return TRUE;
}
/********************************************************************************/
static void DefineQChemCartBasis()
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
		 m=0; l[0][m] = 2;l[1][m] = 0;l[2][m] = 0; /* XX */
		 m++; l[0][m] = 1;l[1][m] = 1;l[2][m] = 0; /* XY */
		 m++; l[0][m] = 0;l[1][m] = 2;l[2][m] = 0; /* YY */
		 m++; l[0][m] = 1;l[1][m] = 0;l[2][m] = 1; /* XZ */
		 m++; l[0][m] = 0;l[1][m] = 1;l[2][m] = 1; /* YZ */
		 m++; l[0][m] = 0;l[1][m] = 0;l[2][m] = 2; /* ZZ */
		 break;
	  case 3 :
		 m=0; l[0][m] = 3;l[1][m] = 0;l[2][m] = 0; /* XXX */
		 m++; l[0][m] = 2;l[1][m] = 1;l[2][m] = 0; /* XXY */
		 m++; l[0][m] = 1;l[1][m] = 2;l[2][m] = 0; /* YYX */
		 m++; l[0][m] = 0;l[1][m] = 3;l[2][m] = 0; /* YYY */
		 m++; l[0][m] = 2;l[1][m] = 0;l[2][m] = 1; /* XXZ */
		 m++; l[0][m] = 1;l[1][m] = 1;l[2][m] = 1; /* XYZ */
		 m++; l[0][m] = 0;l[1][m] = 2;l[2][m] = 1; /* YYZ */
		 m++; l[0][m] = 1;l[1][m] = 0;l[2][m] = 2; /* ZZX */
		 m++; l[0][m] = 0;l[1][m] = 1;l[2][m] = 2; /* ZZY */
		 m++; l[0][m] = 0;l[1][m] = 0;l[2][m] = 3; /* ZZZ */
		 break;
	  case 4 :
		 /* I think that Qchem can n ot print orb with G cartz */
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
/**********************************************/
static void DefineQChemSphericalBasis()
{
 gint i,j,k;
 gint c;
 gint kl;
 gint L,M;
 CGTF *temp;
 Zlm Stemp;
 gint N,Nc,n;
 gint  klbeg;
 gint  klend;
 gint  klinc;


 NOrb = 0;
 for(i=0;i<nCenters;i++)
 {
	 for(j=0;j<Type[GeomOrb[i].NumType].Norb;j++)
	 {
		L=Type[GeomOrb[i].NumType].Ao[j].L;
		NOrb += 2*L+1;
	 }
 }

 temp  = g_malloc(NOrb*sizeof(CGTF));

 k=-1;
 for(i=0;i<nCenters;i++)
 for(j=0;j<Type[GeomOrb[i].NumType].Norb;j++)
 {
	 	L =Type[GeomOrb[i].NumType].Ao[j].L;

		if(L==1)
		{
                	klbeg = L;
                	klend = -L;
                	klinc = -1;
		}
		else
		{
                	klbeg = -L;
                	klend = L;
                	klinc = +1;
		}
		for(kl = klbeg;(klbeg == -L && kl<=klend) || (klbeg == L && kl>=klend);kl +=klinc)
		{
			M = kl;
	 		k++;
	 	   	Stemp =  getZlm(L,M);

	 		temp[k].numberOfFunctions=Stemp.numberOfCoefficients*Type[GeomOrb[i].NumType].Ao[j].N;
		    	temp[k].NumCenter=i;
	 		temp[k].Gtf =g_malloc(temp[k].numberOfFunctions*sizeof(GTF));
          		Nc=-1;
	 		for(N=0;N<Type[GeomOrb[i].NumType].Ao[j].N;N++)
	 			 for(n=0;n<Stemp.numberOfCoefficients;n++)
	 			{
	 			   Nc++;
	 			
	   				temp[k].Gtf[Nc].Ex   = Type[GeomOrb[i].NumType].Ao[j].Ex[N];
	   				temp[k].Gtf[Nc].Coef = Type[GeomOrb[i].NumType].Ao[j].Coef[N]*Stemp.lxyz[n].Coef;
	   				for(c=0;c<3;c++)
	   				{
	   					temp[k].Gtf[Nc].C[c] = GeomOrb[i].C[c];
	   					temp[k].Gtf[Nc].l[c] = Stemp.lxyz[n].l[c];
	   				}
	 			}
			if(L==0) break;
			if(L==1 && kl == -1)
			{
				/* switch -1 and 0 */
 				CGTF dum = temp[k];
				temp[k] = temp[k-1];
				temp[k-1] = dum;
			}
	      }
 }
 for(i=0;i<NAOrb;i++) g_free(AOrb[i].Gtf);
 g_free(AOrb);
 NAOrb = NOrb;
 AOrb = temp;
 if(SAOrb) g_free(SAOrb);
 SAOrb = NULL;
 DefineAtomicNumOrb();
}
/********************************************************************************/
static gchar** read_basis_from_a_qchem_output_file(gchar *FileName, gint* nrs)
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
		if(strstr(t,"Basis set in general basis input format:"))
		{
			if(!fgets(t,taille,fd))break;
			if(!strstr(t,"----------------------------"))break;
			if(!fgets(t,taille,fd))break;
			if(!strstr(t,"$basis"))break;
			OK = TRUE;
			break;
		}
        }
	if(!OK)
	{
		g_free(t);
  		Message(_("Sorry I can read basis from this file. Add \nPRINT_GENERAL_BASIS     true\nPRINT_ORBITALS  true\n to your input file"),_("Error"),TRUE);
  		return NULL;
	}

	strbasis=g_malloc(sizeof(gchar*));;
 	while(!feof(fd))
	{
		if(!fgets(t,taille,fd))break;
		if(strstr(t,"$end") || strstr(t,"$END") ) break;
		nrows++;
		strbasis = g_realloc(strbasis,nrows*sizeof(gchar*));
		strbasis[nrows-1] = g_strdup(t);
	}
	if(nrows == 0)
	{
		g_free(t);
		g_free(strbasis);
  		Message(_("Sorry I can read basis from this file\n"),_("Error"),TRUE);
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
		/* L =SP with QChem */
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
static gboolean DefineQChemBasisType(gchar** strbasis, gint nrows)
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
	gint c;
	gboolean newAtom  = TRUE;

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
	nconts = 0;
	newAtom  = TRUE;
	for(k=0;k<nrows;k++)
	{
		if(strstr(strbasis[k],"**"))
		{
			newAtom = TRUE;
			k++;
			if(k>=nrows)break;
		}
		sscanf(strbasis[k],"%s",t);
		if(newAtom && !isdigit(t[0])) /* symbol of atom*/
		{
			newAtom = FALSE;
			i=get_num_type_from_symbol(t);
			if(i<0)
			{
				Ok = FALSE;
				break;
			}
			Type[i].Norb=0;
			nconts=0;
			continue;
		}
		sscanf(strbasis[k],"%s %d",shell, &nconts);
		for(c=0;c<nconts;c++)
		{
			k++;
			ne = sscanf(strbasis[k],"%s %s %s",temp[0],temp[1],temp[2]);
			if(ne<2)
			{
				Ok = FALSE;
				break;
			}
			for(j=0;j<ne;j++)
			{
				gchar* d=strstr(temp[j],"D");
				if(d) *d='e';
			}
			if(ne>=1) ex[c]=atof(temp[0]);
			if(ne>=2) coef1[c]=atof(temp[1]);
			if(ne==3) coef2[c]=atof(temp[2]);
		}
		if(nconts != 0)
		{
			gint j = Type[i].Norb;
			uppercase(shell);
			if(strcmp(shell,"SP")==0) Type[i].Norb+=2;
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
			if(strcmp(shell,"SP")==0)
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
		}
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
static gint get_number_of_occ(FILE* file, gchar* t)
{
	gint n = 0;
	gint nr;
	if(!fgets(t,BSIZE,file)) return 0;
	if(!strstr(t,"-- Occupied --")) return 0;
	n = 0;
	do{
		if(!fgets(t,BSIZE,file))break;
		nr = numb_of_reals_by_row(t);
		if(numb_of_string_by_row(t)!=nr) nr = 0;
		n += nr;

	}while(!strstr(t,"-- ") && !this_is_a_backspace(t));
	return n;
}
/********************************************************************************/
static void get_number_of_occuped_orbitals(gchar* FileName, gint* nAlpha, gint* nBeta)
{
 	gchar *t;
 	FILE *file;

	*nAlpha=0;
	*nBeta=0;
 	if ((!FileName) || (strcmp(FileName,"") == 0)) return;

 	t=g_malloc(BSIZE*sizeof(gchar));
 	file = FOpen(FileName, "rb");
 	if(file ==NULL) return;

	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if(strstr(t,"Orbital Energies (a.u.)")) break;
	}
	if(!strstr(t,"Orbital Energies (a.u.)")) 
	{
		if(t!=NULL) g_free(t);
		if(file) fclose(file);
		return;
	}
    	{ char* e = fgets(t,BSIZE,file);}
		
	while(!feof(file))
	{
		if(!fgets(t,BSIZE,file))break;
		if(strstr(t,"----")) break;
		if(strstr(t,"Alpha MOs"))
			*nAlpha  = get_number_of_occ(file, t);
		if(strstr(t,"Beta MOs"))
			*nBeta  = get_number_of_occ(file, t);

	}
	if(t!=NULL) g_free(t);
	if(file) fclose(file);
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
static gboolean read_last_orbitals_in_qchem_file(gchar *fileName,GabEditOrbType itype, gint nAlpha, gint nBeta)
{
#define NO   6
 	gchar *t;
 	gboolean OK;
 	gchar *dum[NO];
 	FILE *fd;
 	gint taille=BSIZE;
 	gint i;
 	gint numorb;
 	gchar *pdest = NULL;
	gint NumOrb[NO];
	gdouble EnerOrb[NO];
	gint ncart;
	gint n;
	gint k,k1,k2,k3;
	gint j;
	gdouble **CoefOrbitals;
	gdouble *EnerOrbitals;
	gchar **SymOrbitals;
	gchar* tmp = NULL;
	
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
 	for(i=0;i<NO;i++) dum[i]=g_malloc(BSIZE*sizeof(gchar));
  
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
				case GABEDIT_ORBTYPE_RESTRICTED: 
          				pdest = strstr( t, titlesOrb[itype] ); 
					break;
				case GABEDIT_ORBTYPE_MCSCF: 
					pdest = strstr( t,titlesOrb[itype]); 
					break;
				case GABEDIT_ORBTYPE_EIGENVECTORS: 
          				pdest = strstr( t, titlesOrb[itype] ); 
					{
						gchar dump1[50];
						gchar dump2[50];
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
 			for(i=0;i<NO;i++) g_free(dum[i]);

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

				case GABEDIT_ORBTYPE_RESTRICTED: 
				case GABEDIT_ORBTYPE_MCSCF: 
				case GABEDIT_ORBTYPE_EIGENVECTORS: 
				case GABEDIT_ORBTYPE_BOYS: 
				case GABEDIT_ORBTYPE_EDMISTON: 
				case GABEDIT_ORBTYPE_PIPEK: 
				CoefAlphaOrbitals = CoefOrbitals;
				EnerAlphaOrbitals = EnerOrbitals;
				SymAlphaOrbitals = SymOrbitals;
				OccAlphaOrbitals = g_malloc(NOrb*sizeof(gdouble));
				if(itype==GABEDIT_ORBTYPE_RESTRICTED && nBeta==0) nBeta = nAlpha;

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
				break;
			case GABEDIT_ORBTYPE_RESTRICTED: 
			case GABEDIT_ORBTYPE_MCSCF: 
			case GABEDIT_ORBTYPE_EIGENVECTORS: 
				break;
			case GABEDIT_ORBTYPE_BOYS: 
			case GABEDIT_ORBTYPE_EDMISTON: 
			case GABEDIT_ORBTYPE_PIPEK: 
    				{ char* e = fgets(t,taille,fd);}
		}

  		ncart=NOrb/NO;
		if(NOrb%NO>0) ncart++;
		gint no=0;
		for(n=0;n<ncart;n++)
		{
	  		if(!fgets(t,taille,fd))break;
			k1 = sscanf(t,"%d %d %d %d %d %d",&NumOrb[0],&NumOrb[1],&NumOrb[2],&NumOrb[3],&NumOrb[4],&NumOrb[5]);
			for(i=0;i<k1;i++) NumOrb[i]--;
			if(k1<1)
			{
				break;
			}

			
	  		if(!fgets(t,taille,fd))break;
			k2 = sscanf(t,"%s %lf %lf %lf %lf %lf %lf", dum[0], &EnerOrb[0], &EnerOrb[1], &EnerOrb[2], &EnerOrb[3], &EnerOrb[4], &EnerOrb[5]);
			k2--;
			for(i=0;i<k2;i++) EnerOrbitals[NumOrb[i]] = EnerOrb[i];
			if(k2>0)
			{
				for(i=0;i<k2;i++) SymOrbitals[NumOrb[i]] = g_strdup("UNK");
				k3 = k2;
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
				tmp = t + 19;
				k = sscanf(tmp,"%s %s %s %s %s %s",dum[0], dum[1], dum[2], dum[3], dum[4], dum[5]);
				for(j=0;j<k;j++) CoefOrbitals[NumOrb[j]][i]=atof(dum[j]);
			}
			no+=k3;
			if(k3<NO)break;
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
 	for(i=0;i<NO;i++) g_free(dum[i]);

	CoefAlphaOrbitals = CoefOrbitals;
	EnerAlphaOrbitals = EnerOrbitals;
	return TRUE;
}
/********************************************************************************/
void read_qchem_orbitals(gchar* FileName)
{
	gint typefile;
	gchar *t = NULL;
	gint nrs;
	gchar** strbasis=NULL;
	gint i;
	gboolean Ok;
	gint nAlpha;
	gint nBeta;
	GabEditOrbLocalType typeLocal;
	gint typebasis = -1;
	/* gint j,jj; */

	typefile =get_type_file_orb(FileName);
	if(typefile==GABEDIT_TYPEFILE_UNKNOWN) return;


	if(typefile != GABEDIT_TYPEFILE_QCHEM)
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
	set_status_label_info(_("File type"),"QChem");
	set_status_label_info(_("Mol. Orb."),"Reading");
	
	free_orbitals();	

	typebasis =get_type_basis_in_qchem_file(FileName);
	if(typebasis == -1)
	{
		gchar buffer[BSIZE];
		sprintf(buffer,
				_(
				"Sorry, Gabedit does not support mixed spherical  and contaminant cartezian basis functions\n\n"
				"Use 'purecart      11111' or 'purecart        22222' in $rem block"
				)
		       );
  		Message(buffer,_("Error"),TRUE);
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}

 	if(!read_geomorb_qchem_file_geom(FileName) )
	{
		free_geometry();
		set_status_label_info(_("File name"),_("Nothing"));
		set_status_label_info(_("File type"),_("Nothing"));
		set_status_label_info(_("Mol. Orb."),_("Nothing"));
		return;
	}
	strbasis=read_basis_from_a_qchem_output_file(FileName, &nrs);
	if(strbasis==NULL)
	{
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

	set_status_label_info(_("Mol. Orb."),"Reading");
 	InitializeAll();
 	if(!DefineQChemBasisType(strbasis,nrs))
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

	/*
	for(i=0;i<Ntype;i++)
	for(j=0;j<Type[i].Norb;j++)
	{
		printf(" L = %d\n",Type[i].Ao[j].L);
		for(jj=0;jj<Type[i].Ao[j].N;jj++)
		{
			printf(" %lf ",Type[i].Ao[j].Ex[jj]);
			printf(" %lf ",Type[i].Ao[j].Coef[jj]);
		}
		printf("\n");
	}
	*/
  	DefineType();
	buildBondsOrb();
	RebuildGeomD = TRUE;
	reset_grid_limits();
	init_atomic_orbitals();
	set_status_label_info(_("Geometry"),_("Ok"));
	glarea_rafresh(GLArea); /* for geometry*/


	if(typebasis == 1)
	{
 		DefineQChemSphericalBasis();
		sphericalBasis = TRUE;
	}
	else
	{
 		DefineQChemCartBasis();
		sphericalBasis = FALSE;
	}
 	
 	/*PrintAllBasis();*/
 	NormaliseAllBasis();
 	/*PrintAllBasis();*/
 	DefineNOccs();

	get_number_of_occuped_orbitals(FileName, &nAlpha, &nBeta);
	
	/*
	printf("Number of ALPHA occ = %d\n",nAlpha);
	printf("Number of BETA  occ = %d\n",nBeta);
	printf("NOrb = %d\n",NOrb);
	*/


	typeLocal = get_local_orbital_type(FileName);
	if(typeLocal!=GABEDIT_ORBLOCALTYPE_UNKNOWN)
	{
		if(typeLocal==GABEDIT_ORBLOCALTYPE_BOYS)
		{
			Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_BOYS_ALPHA,nAlpha,nBeta);
			if(Ok) Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_BOYS_BETA, nAlpha, nBeta);
			else Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_BOYS, nAlpha, nBeta);

		}
		else 
		if(typeLocal==GABEDIT_ORBLOCALTYPE_EDMISTON)
		{
			Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_EDMISTON_ALPHA,nAlpha,nBeta);
			if(Ok) Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_EDMISTON_BETA, nAlpha, nBeta);
			else Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_EDMISTON, nAlpha, nBeta);
		}
		else
		{
			Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_PIPEK_ALPHA,nAlpha,nBeta);
			if(Ok) Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_PIPEK_BETA, nAlpha, nBeta);
			else Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_PIPEK, nAlpha, nBeta);

		}

	}
	else
	{
		Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_ALPHA,nAlpha,nBeta);
		if(Ok) 
		{
			Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_BETA, nAlpha, nBeta);
		}
		else
		{
			if(!Ok) Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_RESTRICTED, nAlpha, nBeta);

			if(!Ok) Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_MCSCF, nAlpha, nBeta);
			if(!Ok) Ok = read_last_orbitals_in_qchem_file(FileName,GABEDIT_ORBTYPE_EIGENVECTORS, nAlpha, nBeta);
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
